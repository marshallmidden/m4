# $Id: magdrvr.as 162911 2014-03-20 22:45:34Z marshall_midden $
#******************************************************************************
#
#  NAME: magdrvr.as
#
#  PURPOSE:
#   To provide a device driver element to process host to MAGNITUDE
#   SCSI-FCP events and to manage their completion.
#
#  GLOBAL FUNCTIONS:
#       MAGD$init      - MAG driver initialization
#       MAGD$create_image - MAG driver create new image routine
#
#       This module employs the following processes:
#
#       mag$exec       - MAG driver executive
#       mag$fu         - FORMAT UNIT processing task
#
#  Copyright (c) 1998 - 2010 Xiotech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- local equates -----------------------------------------------------------
#
        .set    IMTMAX,24               # # IMTs to initialize
        .set    ILMTMAX,IMTMAX*3        # # ILMTs to initialize
        .set    mag_twait1,1000         # mag$exec timed wait (in msec.)
                                        #  when no virtual drives assigned
                                        #  to my interface
        .set    mystrategy,vrnorm       # VRP strategy to use
#
# --- global function declarations --------------------------------------------
#
        .globl  MAGD$init               # MAG driver initialization
        .globl  MAGD$stf                # set target flags
        .globl  MAGD$vdiskchange        # Virtual disk info changed by Define
        .globl  MAGD$serverchange       # Server tables change by Define
        .globl  MAG$submit_vrp          # Submit VRP to MAGNITUDE
        .globl  MAGD$create_image       # MAG driver create new image routine
        .globl  MAG$del_all_inact_imt   # Deletes all inactive IMT

.if     INITIATOR
#
# --- global function declarations for initiator support
#
        .globl  mag$get_ilmt
        .globl  mag$sepilmtvdmt

        .globl  mag$init_task
        .globl  mag$qtask2aq
        .globl  mag$ISP$receive_io
        .globl  mag$chknextask
        .globl  mag$remtask

        .globl  mag1_iocr
        .globl  mag2_iocr

        .globl  magd$nolun
        .globl  magd$inquiry
        .globl  magd$reqsns
        .globl  magd$repluns

        .globl  mag1$undef
        .globl  mag1$tur
        .globl  mag1$startstop
        .globl  mag1$modesns
        .globl  mag1$reqsns
        .globl  mag1$repluns
        .globl  mag1$read10
        .globl  mag1$write10
        .globl  mag1$writevfy
        .globl  mag1$vfymedia
        .globl  mag1$inquiry
        .globl  mag1_MAGcomp
        .globl  mag1$cmdcom
        .globl  mag1_srpreq
        .globl  mag1_srpcomp
        .globl  te6_srpreq
        .globl  te6_srpcomp

        .globl  mag2$rconflict

        .globl  mag$abtask              # Abort task
        .globl  mag$dabtaskset          # Abort task set
        .globl  mag$dreset              # Reset received
        .globl  mag$doffline            # Interface offline (not operational)
        .globl  mag$donline             # Interface online (becoming
                                        #  operational)
        .globl  mag$cmdrecv             # SCSI command received
        .globl  mag$abtaskset           # Abort task set
        .globl  mag$reset               # Reset received
        .globl  mag$offline             # Interface offline (not operational)
        .globl  mag$online              # Interface online (becoming
                                        #  operational)
        .globl  mag$clearaca            # Clear ACA received
#
# --- global data declarations
#
        .globl  modesns1
        .globl  sense_err1
        .globl  sense_invf1
        .globl  inquiry_tbl1
        .globl  inquiry_tbl2
        .globl  inquiry_tbl3
        .globl  task_etbl2
        .globl  teh_ignore
.endif  # INITIATOR
#
# --- global data declarations
#
        .globl  MAGD$sense_nolun        # LUN not supported SENSE data
        .globl  MAGD$sense_uninit       # LUN not ready - still initializing
                                        #  SENSE data
        .globl  MAGD$targetrdy          # target ready flag

        .globl  MAGD_SCSI_WHQL          # SCSI WHQL compliance
        .globl  MAG_VDMT_dir            # Virtual Device Mgmt Directory
        .globl  mag_imt_head            # Head of allocated IMT list
        .globl  magerr_events           # Number of mag events in last second
        .globl  host_events             # Number of host sense events in last sec
#
# --- Data Definitions --------------------------------------------------------
#
        .data
#
        .align  4                       # align just in case
#
# --- Local storage fields
#
MAGD$targetrdy:
        .byte   0                       # target ready flag
                                        # 00 = not ready
                                        # <>00 = ready
#
mmc_wkflgs:
        .byte   0                       # process work flags
                                        # Bit 7:
                                        #     6:
                                        #     5:
                                        #     4:
                                        #     3:
                                        #     2:
        .set    wk_svchg,1              #     1: 1=server changed processing
        .set    wk_vdchg,0              #     0: 1=VD changed processing
#
mag_cmdt_count:                         # Host sense error counts per port on MAG-2-MAG links
        .word   0
        .word   0
        .word   0
        .word   0
mag_busy_count:
        .byte   0                       # busy status count for filtering busy
                                        #  messages to the MMC

MAGD_SCSI_WHQL:
        .byte   0                       # WHQL compliance

#
# --- Logging event throttle constant and current counts
#     This limits the number of mag and host sense errors logged.
#     CDriver clears the event counters every n seconds.
#     MagDriver will only log the first err_throttle events each period for
#     each error type.
#
#     A more robust design in the future might log events that are different
#     than previous ones and include a count of the number of events that
#     were not reported. Maybe the XSSA should save the debug logs too.
#
        .set    err_throttle,2
        .align  2
magerr_events:                          # Mag error counts this second
        .word   0
host_events:                            # Host sense error counts this second
        .word   0
#
        .align  4                       # align
#
# --- Process PCBs
#
MAG_exec_pcb:
        .word   0                       # mag$exec PCB address
#
MAG_mmc_pcb:
        .word   0                       # mag$mmc PCB address
#
mag_cleanup_pcb:
        .word   0                       # IMT/TMT cleanup PCB

        .section        .shmem
        .globl  m_logbuffer
m_logbuffer:
        .space  ezilen,0                # Log event temporary buffer - largest
                                        #   possible size

        .data
#
# --- FORMAT UNIT process task variables
#
FU_pcb:
        .word   0                       # PCB address of mag$fu task
FU_head:
        .word   0                       # head pointer of mag$fu task FUPMT
                                        #  list
FU_tail:
        .word   0                       # tail pointer of mag$fu task FUPMT
                                        #  list
#
#
#******************************************************************************
#
# _________________ MAG DRIVER EVENT HANDLER TABLES ___________________________
#
#******************************************************************************
#
# --- Default MAG device driver event handler table (im_ehand)
#
MAG_d$event_tbl:                        # MAG default event table
        .word   mag$dcmdrecv            # SCSI command received
        .word   mag$abtask              # Abort task
        .word   mag$dabtaskset          # Abort task set
        .word   mag$dreset              # Reset received
        .word   mag$doffline            # Interface offline (not operational)
        .word   mag$donline             # Interface online (becoming
                                        #  operational)
#
# --- Normal MAG device driver event handler table (ilm_ehand)
#
MAG_event_tbl:                          # Normal MAG event table
        .word   mag$cmdrecv             # SCSI command received
        .word   mag$abtask              # Abort task
        .word   mag$abtaskset           # Abort task set
        .word   mag$reset               # Reset received
        .word   mag$offline             # Interface offline (not operational)
        .word   mag$online              # Interface online (becoming
                                        #  operational)
        .word   mag$clearaca            # Clear ACA received
#
# --- Dflt MAG device driver event handler table for a pending image (im_ehand)
#
MAG_p$event_tbl:                        # Pending image MAG event table
        .word   magp$cmdrecv            # SCSI command received
        .word   magp$abtask             # Abort task
        .word   magp$abtaskset          # Abort task set
        .word   magp$reset              # Reset received
        .word   magp$offline            # Interface offline (not operational)
        .word   magp$online             # Interface online (becoming
                                        #  operational)
#
# --- The following data fields must be initialized to zero in the code
#
        .section end,bss
#
# --- Virtual inQuiry Data from frontend
#     This includes a header and the virtual drive capacities indexed by VID.
#     See VQD.inc for the structure definition
#
temp_inq_data:                          # Temporary data copy area
        .space  vqsize,0
        .align  4
#
MAG_inq_data:                           # Current data copy area
        .space  vqsize,0
#
        .align  4
#
MAG_VDMT_dir:                           # Virtual device directory
        .space  MAXVIRTUALS*4,0         # array of VDMT pointers
#
# --- Shared memory data
#
        .section    .shmem
        .align  2
magd_sft:                               # Software Detected Fault Log area
        .space  mlemaxsiz,0

        .text
#
# --- beginning of code -------------------------------------------------------
#
#******************************************************************************
#
#  NAME: MAGD$init
#
#  PURPOSE:
#       Initializes the MAG device driver environment.
#
#  DESCRIPTION:
#       The executive process is established and made ready
#       for execution.
#
#  CALLING SEQUENCE:
#       call    MAGD$init
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
#******************************************************************************
#
MAGD$init:
#
# --- Clear local data structures
#
        movq    0,r4
        st      r4,mag_imt_head         # clear allocated IMT list head pointer
        st      r4,mag_imt_tail         # clear allocated IMT list tail pointer
        st      r4,MAG_mmc_pcb          # clear mag$mmc task pcb address
        stq     r4,tag_00               # clear task tag counts
        stq     r4,tag_04
        stl     r4,FU_head              # clear mag$fu FUPMT list fields
        st      r4,FU_pcb               # clear mag$fu pcb address
        stob    r4,mmc_wkflgs           # clear mmc task work flags
        stob    r4,MAGD$targetrdy       # clear target ready flag
        stob    r4,mag_busy_count       # clear busy status count
        stq     r4,mag_cmdt_count       # clear cmd terminate status count
#
        lda     MAG_VDMT_dir,r9         # r9 = VDMT directory pointer
c       memset((void*)r9, 0, MAXVIRTUALS*4);
        lda     MAG_inq_data,r9         # r9 = pointer into data structure
c       memset((void*)r9, 0, vqsize);
#
# --- Establish executive process
#
        movl    g0,r14                  # save g0-g1
        lda     mag$exec,g0             # establish executive process
        ldconst MAGEXECPRI,g1
c       CT_fork_tmp = (ulong)"mag$exec";
        call    K$fork
        st      g0,MAG_exec_pcb         # save mag$exec PCB address
        movl    r14,g0                  # restore g0-g1
        ret
#
#******************************************************************************
#
#  NAME: mag$exec
#
#  PURPOSE:
#       MAG device driver executive process.
#
#  DESCRIPTION:
#       This initializes IMTs, ILMTs, and other tables.
#       When this completes:
#       - inquiry data will be available for each virtual device
#       - a VDMT will be allocated for each entry in MAG_VDMT_dir.
#
#       The exec loop waits for a call from define for a vdisk or server change.
#       Those functions enable this task which checks flag bits to determine
#       which operations to perform.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers can be destroyed.
#
#******************************************************************************
#
mag$exec:
#
# --- Allocate and initialize spare IMTs
#
c       init_imt(IMTMAX);               # Get IMTMAX IMTs into pool to start.
                                        # NOTE: we are NOT initializing MAG_d$event_tbl in im_ehand!
#
# --- Allocate spare ILMTs, VDMTs
#
c       init_vdmt(LUNMAX);              # Get LUNMAX VDMTs into pool to start.
#
# --- Initialize spare ILMTs
#
c       init_ilmt(ILMTMAX);             # Initialize ILMTMAX entries in pool.
#
# --- Indicate target is ready
#
        ldconst 0x01,g0
        stob    g0,MAGD$targetrdy       # set target ready flag
#
# --- Initiate mag$fu task
#
        lda     mag$fu,g0               # establish FORMAT UNIT process task
        ldconst MAGFUPRI,g1
c       CT_fork_tmp = (ulong)"mag$fu";
        call    K$fork
        st      g0,FU_pcb               # save mag$fu PCB address
#
# --- Main Processing Loop ----------------------------------------------------
#     The functions called by this loop are allowed to destroy all the global
#     registers so any used here must be save/restored or reloaded.
#
# --- Wait for define messages for server or virtual disk change --------------
#
.magex_100:
c       TaskSetMyState(pcnrdy);         # Set to process not ready
#
# --- Virtual disk change
#
.magex_110:
        call    K$qxchang               # go to sleep
        ldob    mmc_wkflgs,r4           # r4 = MMC work flags byte
        bbc     wk_vdchg,r4,.magex_130  # Jif VD changed flag clear
#
        clrbit  wk_vdchg,r4,r4          # clear the bit
        stob    r4,mmc_wkflgs           # save MMC work flag byte
        call    mag$vdiskchange         # process VD changed message
        b       .magex_110              # loop back and recheck Vdisk Change
#
# --- Server change
#
.magex_130:
        bbc     wk_svchg,r4,.magex_100  # Jif image changed flag clear
        clrbit  wk_svchg,r4,r4          # clear the bit
        stob    r4,mmc_wkflgs           # save MMC work flag byte
        call    mag$serverchange        # process server changed message
#
# --- Recheck the bits before putting the task back to sleep
#
        b       .magex_110              # loop back and recheck the bits
#
#******************************************************************************
#
#  NAME: MAGD$vdiskchange
#
#  PURPOSE:
#       Wakeups up the magdriver exec to process a virtual disk change event.
#
#  DESCRIPTION:
#       This is called by the define layer when a virtual disk configuration
#       change has occurred. It will do these steps:
#       - wait for mag$exec process to be defined
#       - Sets bits in mmc_wkflgs to indicate which define event occurred
#       - Enables the magdriver task
#       - Returns
#
#  CALLING SEQUENCE:
#       call MAGD$vdiskchange
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
#******************************************************************************
#
MAGD$vdiskchange:
        ldconst pcrdy,r7                # set magdriver PCB status to ready
#
        ld      MAG_exec_pcb,r8         # get mag$exec PCB address
        ldob    mmc_wkflgs,r4           # r4 = work flags byte
        setbit  wk_vdchg,r4,r4          # set server changed work flag
        stob    r4,mmc_wkflgs           # save updated work flags byte
#
        ldob    pc_stat(r8),r6
        cmpobne r6,pcnrdy,.vdiskret1
.ifdef HISTORY_KEEP
c CT_history_pcb("MAGD$vdiskchange setting ready pcb", r8);
.endif  # HISTORY_KEEP
        stob    r7,pc_stat(r8)          # ready task
.vdiskret1:
        ret
#
#******************************************************************************
#
#  NAME: MAGD$serverchange
#
#  PURPOSE:
#       Wakeups up the magdriver exec to process a server change event.
#
#  DESCRIPTION:
#       This is called by the define layer when a server configuration
#       change has occurred. It will do these steps:
#       - Stops cache - mag$serverchange is responsible for restarting it
#       - Sets bits in mmc_wkflgs to indicate which define event occurred
#       - Enables the magdriver task
#       - Returns
#
#  CALLING SEQUENCE:
#       call MAGD$serverchange
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
#******************************************************************************
#
MAGD$serverchange:
        ldconst pcrdy,r7                # set magdriver PCB status to ready
#
        ld      MAG_exec_pcb,r8         # get mag$exec PCB address
        ldob    mmc_wkflgs,r4           # r4 = work flags byte
        setbit  wk_svchg,r4,r4          # set server changed work flag
        stob    r4,mmc_wkflgs           # save updated work flags byte
#
        ldob    pc_stat(r8),r6
        cmpobne r6,pcnrdy,.serverret1
.ifdef HISTORY_KEEP
c CT_history_pcb("MAGD$serverchange setting ready pcb", r8);
.endif  # HISTORY_KEEP
        stob    r7,pc_stat(r8)          # ready task
.serverret1:
        ret
#
#******************************************************************************
#
#  NAME: mag$fu
#
#  PURPOSE:
#       MAG device driver FORMAT UNIT process task.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers can be destroyed.
#
#******************************************************************************
#
mag$fu:
.magfu_100:
        ld      FU_head,r15             # r15 = next FUPMT on list to process
        cmpobne 0,r15,.magfu_200        # Jif FUPMT to process
c       TaskSetMyState(pcnrdy);         # Set to process not ready
        call    K$qxchang               # go to sleep
        b       .magfu_100              # and go check list again
#
# --- FUPMT on list to process
#
.magfu_200:
        ldob    fpmt_flag(r15),r14      # r14 = FUPMT flag byte
        bbs     7,r14,.magfu_1000       # Jif process has been aborted
#
# --- Allocate resources for FORMAT UNIT process
#       This includes:
#                       fmtcount ILTs
#                       fmtcount VRPs
#
                                        # r15 = FUPMT
        ld      fpmt_vdmt(r15),r14      # r14 = assoc. VDMT
        ldl     vdm_devcap(r14),r4      # r4 = device capacity (sectors)
        stl     r4,fpmt_endsda(r15)     # save ending SDA in FUPMT

        ldconst fmtcount,r14            # r14 = # ILT/VRPs to allocate

        ld      fpmt_priilt(r15),r13    # r13 = assoc. pri. task ILT at inl2
                                        #  nest level
        ld      inl2_FCAL(r13),r12      # r12 = assoc. pri. task ILT at inl1
                                        #  nest level
        mov     r13,g9                  # g9 = assoc. pri task ILT at inl2
        mov     0,r11                   # r11 = ILT/VRP list
.magfu_210:
        call    M$aivw                  # allocate an ILT/VRP combo
                                        # g1 = ILT
                                        # g2 = VRP
        ldq     il_w0(r12),r4           # r4-r7 = il_w0-il_w3 of FCAL parms.
        stq     r4,il_w0(g1)            # copy il_w0-il_w3 parms. into ILT
        ldq     il_w4(r12),r4           # r4-r7 = il_w4-il_w7 of FCAL parms.
        stq     r4,il_w4(g1)            # copy il_w4-il_w7 parms. into ILT
        st      g2,vrvrp(g1)            # save allocated VRP in ILT
        lda     ILTBIAS(g1),r4          # r4 = ILT at funl2 nest level
        st      g1,funl2_FCAL(r4)       # save FCAL parm. ptr.
        st      r15,funl2_fupmt(r4)     # save FUPMT in ILT
        st      g9,il_fthd(r4)          # link ILT onto list
        mov     r4,g9                   # g9 = new top ILT on list
#
# --- Assign ILT/VRPs secondary process ILT/VRPs
#
        ld      fpmt_ilmt(r15),r3       # r3 = assoc. ILMT address
        ld      il_fthd(g9),r11         # r11 = ILT/VRP list to process
        st      r3,fusl2_ilmt(g9)       # save ILMT address in ILT
        ldconst 0,r3
        st      r3,il_fthd(g9)          # clear forward thread in ILTs
        ld      fpmt_secvrp(r15),r7     # r7 = top sec. ILT on list
        st      r7,il_fthd(g9)          # link new sec. ILT on top of list
        st      g9,fpmt_secvrp(r15)     # save new sec. ILT head member
#
# --- determine length of format and allocate Write Buffer
#
        mov     g2,r8                   # Save the VRP address
        ldconst fmtlen,g2               # Length of the format (blocks)
        mov     r8,g0                   # g0 = VRP
        shlo    9,g2,g2                 # Length of the format (bytes)
        call    c$alrbuf                # Go get the Write Buffer
        ld      vr_sglptr(r8),r9        # Get the SGL Pointer (assume 1 entry)
c       if (r9 == 0xfeedf00d) {
c           fprintf(stderr,"%s%s:%u mag$fu sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__);
c           abort();
c       }
        ld      sg_desc0+sg_addr(r9),r10 # Get the BE Address as viewed from FE
#
# --- Transform the Buffer address to what the BE will see locally
#
        st      r10,sg_desc0+sg_addr(r9) # Save what the BE960 would see locally
        mov     r8,g2                   # restore the VRP address

#
# --- now build up the pattern and send out the VRP's
#
        call    mag$fu_build_pat        # build up pattern and send out vrp's
#
# -- determine if there is more ilt's to

        subo    1,r14,r14               # decrement loop count
        cmpobne 0,r14,.magfu_210        # Jif more ILTs to allocate

.magfu_300:
        ldob    fpmt_flag(r15),r4       # r4 = FUPMT flag byte
        bbs     0,r4,.magfu_1000        # Jif process completion flag set
c       TaskSetMyState(pcnrdy);         # Set to process not ready
        call    K$qxchang               # go to sleep
        b       .magfu_300              # check if completed
#
# --- FORMAT UNIT process has been terminated
#       Note: This routine assumes that all allocated resources
#               are available to be deallocated (i.e. they are done
#               being used!!!!!)
#
.magfu_1000:
        ld      fpmt_secvrp(r15),g1     # g1 = top sec. ILT on list
        cmpobe  0,g1,.magfu_1010        # Jif no more sec. ILTs on list
        ld      il_fthd(g1),r4          # r4 = next ILT on list
        st      r4,fpmt_secvrp(r15)     # save new top ILT on list
        ld      vr_sglptr(g1),g0        # Get the SGL Pointer (assume 1 entry)
c       if (g0 == 0xfeedf00d) {
c           fprintf(stderr,"%s%s:%u .magfu_1000 sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__);
c           abort();
c       }
        call    c$rlrbuf                # Release the Write Buffer
        ldconst 0,g13
        lda     -ILTBIAS(g1),g1         # g1 = ILT address at #1 nest
                                        #  level
        ld      vrvrp(g1),g2            # g2 = assoc. VRP from ILT
        st      0,vrvrp(g1)             # clear VRP field in ILT
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        b       .magfu_1000             # and go check for more sec.
                                        #  ILT/VRPs to release
.magfu_1010:
        ld      fpmt_priilt(r15),g1     # g1 = pri. task ILT at inl2
                                        #  nest level
        cmpobe  0,g1,.magfu_1030        # Jif task pri. ILT already
                                        #  disassociated with FUPMT
        ld      ILTBIAS+fu3_patsgl(g1),g2 # g2 = pattern SGL/buffer if one
                                          #  still assigned
        cmpobe  0,g2,.magfu_1025        # Jif no pattern SGL/buffer
                                        #  assigned
        lda     -sghdrsiz(g2),g0
        call    M$rsglbuf               # release SGL/buffer
        mov     0,g2
        st      g2,ILTBIAS+fu3_patsgl(g1) # clear pattern SGL in ILT
.magfu_1025:
        ldob    fpmt_status(r15),g0     # g0 = FORMAT UNIT process
                                        #  completion status
        ld      inl2_ilmt(g1),g6        # g6 = assoc. ILMT address
        ld      inl2_ehand(g1),r14      # r14 = task event handler table
        ld      inl2_eh_magcomp(r14),r13 # r13 = MAG completion event
                                         #  handler routine
        callx   (r13)                   # call task MAG completion
                                        #  event handler routine
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
        st      0,fpmt_priilt(r15)      # clear task pri. ILT from FUPMT
.magfu_1030:
        ld      fpmt_link(r15),r14      # r14 = next FUPMT on list
        st      r14,FU_head             # save new head pointer
        cmpobne 0,r14,.magfu_1040       # Jif more FUPMTs on list
        st      r14,FU_tail             # clear list tail pointer
.magfu_1040:
        mov     r15,g1                  # g1 = ILT/FUPMT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT (FUPMT)
        b       .magfu_100              # and check for any more FUPMTs on
                                        #  list
#
#************************************************************************
#
#  NAME: fus_MAGcomp
#
#  PURPOSE:
#       Provides general MAGNITUDE request completion processing
#       for a FORMAT UNIT process secondary ILT/VRP.
#
#  DESCRIPTION:
#       Checks for a VRP error and if true, saves the VRP status code in
#       the FUPMT and sets up to terminate the FORMAT UNIT process
#       abnormally. If no error occurred, sets the next sec. ILT/VRP
#       operation to be performed if more are needed or else if no more
#       sec. ILT/VRPs are needed, decrements the outstanding sec. ILT/VRP
#       count and then formats the buffer for the SRP operation to be
#       performed and sends the SRP back to the MAGNITUDE.
#
#  CALLING SEQUENCE:
#       call    fus_MAGcomp
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = FORMAT UNIT sec. ILT/VRP address of task at inl2 level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#************************************************************************
#
fus_MAGcomp:
.ifdef TRACES
        call    fu_tr$MAGcomp
.endif # TRACES
        movq    g0,r12                  # save g0-g3
        movq    g4,r4                   # save g4-g7
        movq    g8,r8                   # save g8-g11
        ld      fusl2_fupmt(g1),g3      # g3 = assoc. FUPMT
        ldob    fpmt_flag(g3),g4        # g4 = FUPMT flag byte
        bbs     7,g4,usMAGcomp_100      # Jif process has been aborted
#
# --- Process has not been aborted
#
        cmpobe  0,g0,usMAGcomp_100      # Jif no error reported
        stob    g0,fpmt_status(g3)      # save error code in FUPMT status
        setbit  7,g4,g4                 # set aborted flag
        stob    g4,fpmt_flag(g3)        # save updated flag byte
usMAGcomp_100:
        call    mag$FUsecop             # setup next format op. as appropriate
        movq    r12,g0                  # restore g0-g3
        movq    r4,g4                   # restore g4-g7
        movq    r8,g8                   # restore g8-g11
        ret
#
.ifdef TRACES
#************************************************************************
#
#  NAME: fu_tr$MAGcomp
#
#  PURPOSE:
#       Traces a FORMAT UNIT MAGNITUDE completion event.
#
#  CALLING SEQUENCE:
#       call    fus_MAGcomp
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = FORMAT UNIT sec. ILT/VRP address of task at inl2 level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#************************************************************************
#
fu_tr$MAGcomp:
        movq    g0,r12                  # save g0-g3
                                        # r12 = VRP request completion status
                                        # r13 = pri. ILT at inl2 nest level
        movq    g4,r4                   # save g4-g7
        ld      fusl2_ilmt(g1),g6       # g6 = assoc. ILMT address
        movq    g8,r8                   # save g8-g11
#
# --- Trace incoming event if appropriate
#
        cmpobe  0,g6,.MAGfu10z          # Jif no assoc. ILMT address
        ld      ilm_cimt(g6),g4         # g4 = assoc. CIMT address
        cmpobe  0,g4,.MAGfu10z          # Jif no assoc. CIMT address
        ldos    ci_tflg(g4),g2          # g2 = trace flags
        lda     C_temp_trace,g10        # g10 = trace record build pointer
        bbc     tflg_MAGcomp,g2,.MAGfu10z # Jif event trace disabled
        ld      inl2_FCAL(g1),g7        # g7 = ILT param. area
        ldob    ci_num(g4),g2           # g2 = chip instance
        ldconst trt_MAGcomp,g5          # g5 = trace record type code
        ld      idf_exid(g7),g8         # g8 = exchange ID
        stob    g5,trr_trt(g10)         # save trace record type code
        stob    g2,trr_ci(g10)          # save chip instance
        stos    g8,trr_exid(g10)        # save chip instance
        ldos    idf_lun(g7),g5          # g5 = assoc. LUN #
c       g8 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        st      g0,4(g10)               # save VRP status, clear unused byte
        ld      idf_init(g7),g3         # g3 = initiator ID
        stos    g5,6(g10)               # save LUN
        st      g8,12(g10)              # save timestamp
        st      g3,8(g10)               # save initiator ID
        ld      ci_curtr(g4),g7         # g7 = current trace record pointer
        ldq     (g10),g8                # g8-g11 = trace record
        ldl     ci_begtr(g4),g2         # g2 = trace area beginning pointer
                                        # g3 = trace area ending pointer
        lda     trr_recsize(g7),g5      # g5 = next trace record pointer
        stq     g8,(g7)                 # save trace record in CIMT trace area
        cmpoble g5,g3,.MAGfu10a         # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ci_tflg(g4),g3          # g3 = trace flags
        mov     g2,g5                   # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,g3,.MAGfu10a # Jif wrap off flag not set
        mov     0,g3                    # turn off traces due to trace area
                                        #  wrapped.
        stos    g3,ci_tflg(g4)
.MAGfu10a:
        st      g5,ci_curtr(g4)         # save new current trace record pointer
.MAGfu10z:
        movq    r12,g0                  # restore g0-g3
        movq    r4,g4                   # restore g4-g7
        movq    r8,g8                   # restore g8-g11
        ret
.endif # TRACES
#
#************************************************************************
#
#  NAME: mag$fu_build_pat
#
#  PURPOSE:
#       Provides a means to build up the FORMAT UNIT patterns in the
#       SGL's related to an ILT/VRP.
#
#  DESCRIPTION:
#       Sets up the initialization pattern in the buffers and then
#       initiates sending the process ILT/VRPs with these
#       SGL/buffers to the next layer as VRP write ops. until either
#       the process has been aborted or the entire Virtual Disk has
#       been formatted according to the initiator's specifications.
#
#  CALLING SEQUENCE:
#       call    mag$fu_build_pat
#
#  INPUT:
#       g9 = FORMAT UNIT process ILT/VRP at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. r3-r15/g0-g11 can be destroyed.
#
#************************************************************************
#
mag$fu_build_pat:
        ld      funl2_fupmt(g9),r15     # r15 = assoc. FUPMT address
        ld      fpmt_priilt(r15),r12    # r12 = assoc. task pri. ILT at inl2
        ld      ILTBIAS+fu3_patsgl(r12),r3 # r3 = initialization pattern SGL/
                                        #  buffer if not default
        cmpobne 0,r3,ubldpat_200        # Jif host initialization pattern specified
#
# --- Set up default initialization pattern
#
        mov     g9,r14                  # r14 = ILT/VRP being processed
        lda     -ILTBIAS(g9),r13        # r13 = associated primary ILT
        ld      vrvrp(r13),r13          # r13 = associated VRP
        ld      vr_sglptr(r13),r13      # r13 = associated SGL
c       if (r13 == 0xfeedf00d) {
c           fprintf(stderr,"%s%s:%u mag$fu_build_pat sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__);
c           abort();
c       }
        ld      sg_desc0+sg_addr(r13),r11 # r11 = buffer address
        ld      sg_desc0+sg_len(r13),r12 # r12 = buffer length
c   r4 = r5 = r6 = r7 = 0;              # Default pattern to match.
        ldconst 16,r8                   # r8 = size of pattern write/wack
ubldpat_120:
        stq     r4,(r11)                # write pattern to buffer
        addo    r8,r11,r11              # inc. buffer pointer
        subo    r8,r12,r12              # dec. pattern length
        cmpobne 0,r12,ubldpat_120       # Jif more pattern to initialize
        b       ubldpat_300             # and continue setting up ILT/VRPs
                                        #  to write pattern out with.
#
# --- Host initialization pattern specified
#
ubldpat_200:
        mov     0,r4
        st      r4,ILTBIAS+fu3_patsgl(r12) # clear pattern SGL/buffer field
        ld      sg_addr(r3),r8          # r8 = pattern buffer address
        ld      sg_len(r3),r9           # r9 = pattern length
        ldconst 512,r10                 # r10 = sector size
        mov     g9,r14                  # r14 = ILT/VRP being processed
        movl    r8,r6                   # r6 = working pattern buffer address
                                        # r7 = working pattern length
        mov     r10,r4                  # r4 = working sector size
        lda     -ILTBIAS(g9),r13        # r13 = associated primary ILT
        ld      vrvrp(r13),r13          # r13 = associated VRP
        ld      vr_sglptr(r13),r13      # r13 = associated SGL
c       if (r13 == 0xfeedf00d) {
c           fprintf(stderr,"%s%s:%u ubldpat_200 sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__);
c           abort();
c       }
        ld      sg_desc0+sg_addr(r13),r11 # r11 = buffer address
        ld      sg_desc0+sg_len(r13),r12 # r12 = buffer length
ubldpat_220:
        ldob    (r6),r5                 # r5 = pattern byte
        addo    1,r6,r6                 # inc. pattern address
        subo    1,r7,r7                 # dec. pattern length
        cmpobne 0,r7,ubldpat_230        # Jif pattern length <> 0
        movl    r8,r6                   # reset pattern address & length
ubldpat_230:
        stob    r5,(r11)                # save pattern byte
        addo    1,r11,r11               # inc. buffer address
        subo    1,r12,r12               # dec. buffer length
        subo    1,r4,r4                 # dec. sector byte length
        cmpobne 0,r4,ubldpat_240        # Jif sector length <> 0
        movl    r8,r6                   # reset pattern address & length
        mov     r10,r4                  # reset sector length
ubldpat_240:
        cmpobne 0,r12,ubldpat_220       # Jif more pattern to initialize
        lda     -sghdrsiz(r3),g0        # g0 = pattern SGL/buffer
        call    M$rsglbuf               # release SGL & buffer
                                        # and continue setting up ILT/VRPs
                                        #  to write pattern out with.
#
# --- Set up process ILT/VRPs and ready them to send out.
#
ubldpat_300:
        ld      fpmt_vdmt(r15),r14      # r14 = assoc. VDMT
        ldos    vdm_vid(r14),r12        # r12 = vid being formatted
        mov     g9,r14                  # r14 = ILT/VRP being processed
        ld      -ILTBIAS+vrvrp(r14),g2  # g2 = assoc. VRP for ILT/VRP
        ldconst vroutput,r4             # r4 = VRP function code
        st      r4,vr_func(g2)          # save request function code, clear
                                        #  strategy and status
        stos    r12,vr_vid(g2)          # save vid
        ldconst fmtlen,r5               # r5 = # sectors/format op.
        st      r5,vr_vlen(g2)          # save length in VRP
        setbit  sg_buffer_alloc,0,r4    # r4 = SGL flag byte indicating data
                                        #  has already been received
        stob    r4,vrpsiz+sg_flag(g2)   # save SGL flag byte
                                        # The SGL was set up when the buffer
                                        #  was allocated up front
        lda     fus_MAGcomp,r4          # r4 = ILT completion handler
        st      r4,il_cr(r14)           # save my completion handler in ILT
        lda     ILTBIAS(r14),g1         # g1 = ILT to be submitted to MAG
        ld      funl2_FCAL(r14),r5      # r5 = pointer to FCAL parms.
        st      r5,inl3_FCAL(g1)        # save FCAL parm. ptr. for next level
        lda     fus_srpreq,r4           # r4 = SRP request handler routine
        st      r4,inl2_rcvsrp(r14)     # save SRP request handler in ILT
#
# --- Send out process secondary ILT/VRPs.
#
        mov     r15,g3                  # g3 = FUPMT being processed
        ld      fpmt_secvrp(g3),g1      # g1 = sec. ILT/VRP being processed
        ldob    fpmt_secvrpcnt(g3),r4   # r4 = # sec. ILT/VRPs outstanding
        addo    1,r4,r4                 # inc. count
        stob    r4,fpmt_secvrpcnt(g3)   # save updated counter
        movq    g0,r4                   # save g0-g3
        movq    g4,r8                   # save g4-g7
        movq    g8,r12                  # save g8-g11
        call    mag$FUsecop             # set up and send format op.
        movq    r12,g8                  # restore g8-g11
        movq    r8,g4                   # restore g4-g7
        movq    r4,g0                   # restore g0-g3
        ret
#
#************************************************************************
#
#  NAME: fus_srpreq
#
#  PURPOSE:
#       Provides general SRP request processing for a FORMAT UNIT
#       SRP related to a secondary process ILT/VRP.
#       Note: This routine SHOULD NEVER be called!!!
#
#  DESCRIPTION:
#       This routine does nothing but return the ILT/SRP
#       back to the caller.
#
#  CALLING SEQUENCE:
#       call    fus_srpreq
#
#  INPUT:
#       g1 = ILT/SRP at the otl2 nest level
#       g2 = SRP address
#       g7 = FORMAT UNIT process pri. ILT/VRP at inl1 nest level
#       g9 = FORMAT UNIT process pri. ILT/VRP at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. r3-r15/g0-g11 can be destroyed.
#
#************************************************************************
#
fus_srpreq:
        lda     -ILTBIAS(g1),g1         # g1 = ILT/SRP at otl1 nest level
        ld      il_cr(g1),r4            # r4 = ILT completion routine
        mov     0,g0                    # g0 = SRP completion status code
        callx   (r4)                    # call ILT completion routine
        ret
#
#************************************************************************
#
#  NAME: mag$FUsecop
#
#  PURPOSE:
#       Processes a FORMAT UNIT operation ILT/VRP to
#       be setup to be forwarded on.
#
#  DESCRIPTION:
#       This routine takes an ILT/VRP and it's associated
#       FUPMT and determines whether this operation is complete or if it should
#       be updated and sent again. This routine also saves the
#       logical block address in the beginning of each sector if
#       specified to do so.
#
#  CALLING SEQUENCE:
#       call    mag$FUsecop
#
#  INPUT:
#       g1 = sec. ILT/VRP at the fusl2 nest level
#       g3 = assoc. FUPMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. r3-r15/g0-g11 can be destroyed.
#
#************************************************************************
#
mag$FUsecop:
        movt    g12,r12                 # save g12-g14
        ld      -ILTBIAS+vrvrp(g1),g2   # g2 = assoc. VRP for ILT/VRP
        ld      vr_sglptr(g2),r10       # r10 = associated SGL
c       if (r10 == 0xfeedf00d) {
c           fprintf(stderr,"%s%s:%u mag$FUsecop sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__);
c           abort();
c       }
        ld      sg_desc0+sg_addr(r10),r10 # r10 = buffer address
        ldob    fpmt_flag(g3),r3        # r3 = FUPMT flag byte
        bbc     7,r3,Usecop_30          # Jif process has not been aborted
#
# --- Process has been aborted. Determine if all the VRP's have been
#     returned.
#
Usecop_10:
        ldob    fpmt_secvrpcnt(g3),r4   # r4 = # ILT/VRPs outstanding
        subo    1,r4,r4                 # dec. count
        stob    r4,fpmt_secvrpcnt(g3)   # save updated count
        cmpobne 0,r4,Usecop_1000        # Jif more ILT/VRPs outstanding
#
# --- set format unit operation complete
#
        ldob    fpmt_flag(g3),r4        # r4 = FUPMT flag byte
        setbit  0,r4,r4                 # set process completed flag
        stob    r4,fpmt_flag(g3)        # save updated flag byte

        ld      FU_pcb,r4               # r4 = pcb of mag$fu task
        ldob    pc_stat(r4),r5          # r5 = PCB status byte
        cmpobne pcnrdy,r5,Usecop_1000   # Jif process not not ready
        ldconst pcrdy,r5                # r5 = new process status byte
.ifdef HISTORY_KEEP
c CT_history_pcb("Usecop_10 setting ready pcb", r4);
.endif  # HISTORY_KEEP
        stob    r5,pc_stat(r4)          # save new process status byte
        b       Usecop_1000             # exit
#
# --- Process has NOT been aborted.
#
Usecop_30:
!       ldl     fpmt_sda(g3),r4         # r4,r5 = next op. SDA
!       ldl     fpmt_endsda(g3),r6      # r6,r7 = ending SDA for the last
                                        #  ILT/VRP operation
        ldconst fmtlen,r11              # r11 = normal op. length
        cmpo    1,1                     # Set up to do the subtraction
        subc    r4,r6,r8                # r8,r9 = remaining number of sectors
        subc    r5,r7,r9                #  to format before returning
        cmpobne 0,r9,Usecop_100         # Jif more sectors to format (high word)
        cmpobe  0,r8,Usecop_10          # Jif no more sectors to format (low)
#
# --- More sectors to format before finishing the VRP
#
        cmpobg  r8,r11,Usecop_100       # Jif remaining sector count > normal
                                        #  format op. length
        mov     r8,r11                  # r11 = remaining sector count
        shlo    9,r8,r8                 # r8 = length of data in bytes
        st      r8,vrpsiz+sg_desc0+sg_len(g2) # save data length in SGL
Usecop_100:
        stl     r4,vr_vsda(g2)          # save SDA of format operation
        stl     r4,fusl2_sda(g1)        # save current SDA in ILT
        cmpo    0,1                     # Set up to do the addition
        addc    r11,r4,r8               # r8,r9 = next format operation SDA
        addc    0,r5,r9
        stl     r8,fpmt_sda(g3)         # save new SDA for next format op.
        bbc     5,r3,Usecop_150         # Jif not writing LBA/sector
        ldconst 512,r8                  # r8 = size of each sector
Usecop_140:
        bswap   r4,r7                   # r6,r7 = LBA in big-endian format
        bswap   r5,r6
        stl     r6,(r10)                # save LBA in buffer
        cmpo    0,1                     # Set up to do the addition
        addc    1,r4,r4                 # Increment the LBA
        addc    0,r5,r5
        subo    1,r11,r11               # dec. # sectors in buffer
        addo    r8,r10,r10              # inc. to next sector in buffer
        cmpobne 0,r11,Usecop_140        # Jif more sectors data to write LBA in
Usecop_150:
#
.ifdef TRACES
        mov     g2,r4
        mov     g7,r7
        mov     g9,r9
        mov     g1,g9
        lda     -ILTBIAS(g9),g7
        ld      vrvrp(g7),g2
        call    mag$tr_MAG$submit_vrp
        mov     r9,g9
        mov     r7,g7
        mov     r4,g2
.endif # TRACES
#
        lda     ILTBIAS(g1),g1          # g1 = ILT/VRP at inl3 nest level
        st      g2,vrvrp(g1)            # Store the VRP address at this level
        call    MAG$submit_vrp          # send VRP to next layer
Usecop_1000:
        movt    r12,g12                 # restore g12-g14
        ret
#
#**********************************************************************
#
#  NAME: MAGD$stf
#
#  PURPOSE:
#       Set Target Flags SRP request handler routine.
#
#  DESCRIPTION:
#       This routine takes the Set Target Flags SRP and processes it.
#
#  CALLING SEQUENCE:
#       call    MAGD$stf
#
#  INPUT:
#       g1 = ILT associated with SRP at nest level #2
#       g2 = SRP address to process
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g0-g12, g14 can be destroyed.
#
#**********************************************************************
#
MAGD$stf:
        ld      sr_stf_flags(g2),r4     # r4 = target flags
        bbc     0,r4,.stf_100           # Jif bit 0 not set
        ldob    mmc_wkflgs,r4           # r4 = work flags byte
        ldconst pcrdy,r7                # set magdriver PCB status to ready
        setbit  wk_vdchg,r4,r4          # set VD changed work flag
        ld      MAG_exec_pcb,r8         # get mag$exec PCB address
        stob    r4,mmc_wkflgs           # save updated work flags byte
        ldob    pc_stat(r8),r6
        cmpobne r6,pcnrdy,.stf_100
.ifdef HISTORY_KEEP
c CT_history_pcb("MAGD$stf setting ready pcb", r8);
.endif  # HISTORY_KEEP
        stob    r7,pc_stat(r8)          # ready task
#
.stf_100:
        ldconst srok,g0                 # return good status
        lda     -ILTBIAS(g1),g1         # g1 = ILT/SRP at nest level #1
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT/SRP completion handler routine
        ret
#
#******************************************************************************
#
#  NAME: MAGD$create_image
#
#  PURPOSE:
#       Called from the channel driver level when a new server appears to
#       create a new image from configuration information stored locally.
#
#  DESCRIPTION:
#       This allocates an IMT for the new server.
#
#       It calls mag$setup_zone to allocate any required ILMTs based
#       on the existing configuration data:
#
#       If the server has been configured, use the SDD/LVM/VCD structures
#       to setup the IMT correctly.
#
#       If the server has not been previously configured and a default server
#       (with WWN = 0) exists, then the new server will use the default server
#       configuration. A new server event is sent to the CCB.
#
#       If the server has not been previously configured and a default server
#       does not exist, then an new server event is sent to the CCB.
#
#  CALLING SEQUENCE:
#       call    MAGD$create_image
#
#  INPUT:
#       g0 = Target ID
#       g4 = assoc. CIMT address
#       g5 = initiator ID
#    g6-g7 = WWN of initiator
#
#  OUTPUT:
#       g5 = IMT address of image being created
#
#  REGS DESTROYED:
#       Reg. g5 destroyed.
#
#******************************************************************************
#
MAGD$create_image:
        movq    g0,r8                   # save g0-g3
        movq    g4,r12                  # save g4-g7
                                        # r12 = assoc. CIMT address
                                        # r13 = target ID
                                        # r14-r15 = initiator's MAC address
#
# --- Allocate and initialize IMT to use for new image
#
        call    mag$get_imt             # allocate an IMT to use for new image
                                        # g5 = IMT address to use
#
        st      g4,im_cimt(g5)          # save CIMT in IMT
        stos    r13,im_fcaddr(g5)       # save initiator ID in IMT
        stos    r8,im_tid(g5)           # save target ID
        stl     g6,im_mac(g5)           # save initiator's MAC address in IMT
.if FE_ISCSI_CODE
#
# --- if iSCSI Interfaces - set the iSCSI IF bitn the flags
#
        ldob    ci_num(g4),r4           # Get interface number
c       r5 = ICL_IsIclPort((UINT8)r4);  # r5 = (TRUE/FALSE)
        cmpobe  TRUE,r5,.magcrim_icl01  # Jif ICL port
        ld      iscsimap,r5             # Get iscsimap bitmap

        bbc     r4,r5,.magcrim_150      # Jif
.magcrim_icl01:
#
        ldob    im_flags(g5),r4         # r4 = imt flags byte
        setbit  im_flags_iscsi,r4,r4    # set iSCSI IF flag bit
        stob    r4,im_flags(g5)         # save updated flags byte

        PushRegs(r5)
        mov     g5,g0                   # IMT in g0
        call    fsl_UpdName
        PopRegsVoid(r5)
#
.magcrim_150:
.endif  # FE_ISCSI_CODE
#
#
.if     MAG2MAG
c       g6 = M_chk4XIO(*(UINT64*)&g6);  # is this a XIOtech Controller ???
        cmpobe  0,g6,.magcrim_500       # Jif not a XIOtech Controller
#
        ld      im_mac(g5),g6           # g6 = MSW WWN
        extract 12,4,g6                 # Get 'e' nibble from WWN
        ldconst (WWNBPort>>20)&0xF,g7   # Value for XIOtech BE port
        cmpobe  g6,g7,.magcrim_500      # Jif a XIOtech BE port
#
        ldob    im_flags(g5),r4         # r4 = imt flags byte
        setbit  im_flags_mtml,r4,r4     # set MAG-to-MAG flag bit
        stob    r4,im_flags(g5)         # save updated flags byte
        lda     LLD_d$event_tbl,r4      # r4 = default event handler routine
                                        #  table for MAGNITUDE link images
        st      r4,im_ehand(g5)         # save default event handler table
#
# --- Add an ILMT for LUN 0 and initialize
#
        ldconst 0,g0                    # g0 = assoc. VDMT (null)
        mov     g5,g1                   # g1 = assoc. IMT to add ILMT to
        setbit  31,00,g2                # set flag indicating ILMT online
        ldconst 0,g3                    # g3 = LUN # for ILMT
        call    lld$add_ilmt            # add ILMT to IMT
#
# --- The control port has a target ID above the maximum number of targets.
#     Do not notify the CCB of servers connecting to the control port.
#
        ldconst MAXTARGETS,r3
        cmpobl  r8,r3,.magcrim_370      # Jif this is a not control port
#
# --- Set the Server ID as invalid for the control port
#
        ldconst 0xffff,r3               # Set SID number for no server
        stos    r3,im_sid(g5)           # save the Server ID
        b       .magcrim_1000
#
.magcrim_370:
        movl    r14,g0                  # g0,g1 = WWN for this server
        mov     r8,g2                   # g2 = Target ID for this server
        ldconst FALSE,g3                # g3 = Ignore new servers

        PushRegs(r3)
        lda     im_iname(g5),g4         # g4 = iSCSI name
        call    DEF_WWNLookup
        mov     g0,r4
        PopRegsVoid(r3)
        mov     r4,g3

        ldconst 0xffffffff,r3           # Set SID number to not found
        stos    g3,im_sid(g5)           # save the Server ID (even if invalid)
        cmpobne g3,r3,.magcrim_1000     # Jif a matching Server was found
#
# --- Check if the server is a XIOtech control port
#
        mov     r14,g6                  # g6 = MSW WWN
        ld      im_mac(g5),g6           # g6 = MSW WWN
        extract 12,4,g6                 # Get 'e' nibble from WWN
        ldconst (WWNCPort>>20)&0xF,g7   # Value for XIOtech Control port
        cmpobe  g6,g7,.magcrim_1000     # Jif a XIOtech Control port
#
# --- Check if a new server record exists for this server
#
        movl    r14,g0                  # g0,g1 = WWN for this server
        mov     r8,g2                   # g2 = Target ID for this server
        ldconst TRUE,g3                 # g3 = Include new servers

        PushRegs(r3)
        lda     im_iname(g5),g4         # g4 = iSCSI name
        call    DEF_WWNLookup
        mov     g0,r4
        PopRegsVoid(r3)
        mov     r4,g3

        ldconst 0xffffffff,r3           # Set SID number to not found
        cmpobne g3,r3,.magcrim_1000     # Jif a matching Server was found
#
# --- Do not notify the CCB if we are talking to ourselves (FE QLogics on the
#     same loop or switch) or between Virtual Controller Group controllers.
#
                                        # g0-g1 = WWN for this server
        call    M$findtgt               # g2 = Target ID of a matching WWN
        cmpobne r3,g2,.magcrim_1000     # Jif the server is ourself
#
# --- Server not found - Notify CCB
#
        ldl     im_mac(g5),g0           # g0 = WWN for this server
        ldconst 0x00000F00,r3           # Mask for channel number
        andnot  r3,g0,g0                # Clear the channel number in WWN
        mov     g0,r4
        extract 12,4,r4                 # r4 = port type code
        cmpobe  0,r4,.magcrim_390       # Jif MAGNITUDE WWN
        ldconst 0x0F000000,r3           # Mask off least significant digit
                                        #  of controller serial number
        andnot  r3,g1,g1
.magcrim_390:
        mov     r8,g2                   # g2 = Target ID
        call    mag$mmczoneinquiry      # Send zoning inquiry MRP to CCB
        b       .magcrim_1000           # and we're out of here
#
# --- Not a MAGNITUDE initiator
#
.magcrim_500:
.endif  # MAG2MAG
#
# --- Create any ILMT's for new devices for this server
#
        call    mag$setup_zone
#
.magcrim_1000:
        movq    r8,g0                   # restore g0-g3
        movl    r14,g6                  # restore g6-g7
        ret
#
#******************************************************************************
#
#  NAME: mag$setup_zone
#
#  PURPOSE:
#       This routine creates any ILMTs that may be needed for VID/LUNs
#
#  DESCRIPTION:
#       This routine changes the event handler table in the IMT,
#       sets up ILMTs as specified for this particular server,
#       determines if the IMT is still on the active queue and if
#       so, calls the IMT online event routine and processes all of the
#       task/ILTs on the im_pendtask queue and sends through the normal
#       event handling logic.
#
#  CALLING SEQUENCE:
#       call    mag$setup_zone
#
#  INPUT:
#       g5 = associated IMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$setup_zone:
        PushRegs(r3)                    # Save all G registers

        ld      im_cimt(g5),g4          # g4 = assoc. CIMT address
        call    mag$bld_maci            # build image based on table information
        lda     MAG_d$event_tbl,r4      # r4 = normal default IMT event handler
                                        #  table
        st      r4,im_ehand(g5)         # save new event handler table in IMT
        mov     0,g13                   # always 0
.if 0   # pendVRP is dead code, always zero.
        st      0,im_pendvrp(g5)        # remove ILT/VRP pending just in case
.endif  # pendVRP is dead code, always zero.
#
# --- Check if IMT on active list
#
        ld      ci_imthead(g4),r5       # r5 = first IMT on active list
        cmpobe  0,r5,.znsetup_250       # Jif no IMTs on active list
.znsetup_210:
        cmpobe  r5,g5,.znsetup_220      # Jif IMT found on active list
        ld      im_link(r5),r5          # r5 = next IMT on active list
        cmpobne 0,r5,.znsetup_210       # Jif more IMTs to check on active list
        b       .znsetup_250            # IMT is not on active list
#
# --- IMT is on active list
#
.znsetup_220:
        ld      dd_online(r4),r5        # r5 = IMT online event routine
        callx   (r5)                    # call online event routine
        ld      im_pendtask(g5),r5      # r5 = first task/ILT on pending list
        cmpobe  0,r5,.znsetup_1000      # Jif no task/ILTs on list
        st      0,im_pendtask(g5)       # clear pending task/ILTs from IMT
        movq    g0,r8                   # save g0-g3
        movq    g4,r12                  # save g4-g7
.znsetup_230:
        mov     r5,g1                   # g1 = task ILT at inl2 nest level
        ld      il_fthd(r5),r5          # r5 = next ILT on list
        lda     -ILTBIAS(g1),g7         # g7 = ILT at inl1 nest level
        ldos    idf_lun(g7),r6          # r6 = LUN #
        ld      im_ilmtdir(g5)[r6*4],g6 # g6 = assoc. ILMT address
        mov     r4,r6                   # r6 = IMT event handler table
        cmpobe  0,g6,.znsetup_240       # Jif no ILMT assoc. with LUN
        ld      ilm_ehand(g6),r6        # r6 = ILMT event handler table
.znsetup_240:
        ld      dd_cmdrcv(r6),r6        # r6 = event handler routine to call
        callx   (r6)                    # go to event handler routine
        movq    r12,g4                  # restore g4-g7
        cmpobne 0,r5,.znsetup_230       # Jif more task/ILTs to process
        movq    r8,g0                   # restore g0-g3
        b       .znsetup_1000           # and we're out of here!
#
# --- IMT not on active list
#
.znsetup_250:
        ld      im_pendtask(g5),r5      # r5 = first task/ILT on pending list
        cmpobe  0,r5,.znsetup_1000      # Jif no task/ILTs on list
#
# --- Task/ILTs on pending list. Make sure IMT is on inactive IMT list.
#
        ld      C_imt_head,r6           # r6 = first IMT on inactive list
.znsetup_260:
        cmpobe  0,r6,.znsetup_1000      # Jif no IMTs on inactive list
        cmpobe  g5,r6,.znsetup_270      # Jif found on IMT inactive list
        ld      im_link(r6),r6          # get pointer to next inactive IMT
        b       .znsetup_260            # check if more IMTs on inactive list

.znsetup_270:
        call    magp$reset              # process as though a reset was received
.znsetup_1000:
        PopRegsVoid(r3)                 # Restore all G registers
        ret
#
#******************************************************************************
#
#  NAME: mag$bld_maci
#
#  PURPOSE:
#       Configures the specified IMT with the server configuration data
#       in the SDD/LVM/VCDs.
#
#  DESCRIPTION:
#       This includes allocating ILMTs for all the
#       virtual drives specified in the data and initializing these ILMTs
#       to place them into service.
#
#       If the server has been configured, use the SDD/LVM/VCD structures
#       to setup the IMT correctly.
#
#       If the server has not been previously configured and a default server
#       (with WWN = 0) exists, then the new server will use the default server
#       configuration. A new server event is sent to the CCB.
#
#       If the server has not been previously configured and a default server
#       does not exist, then an new server event is sent to the CCB.
#
#  CALLING SEQUENCE:
#       call    mag$bld_maci
#
#  INPUT:
#       g5 = IMT used to build image in
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$bld_maci:
        movq    g0,r12                  # save g0-g3
#
# --- Look for server with the same WWN in the SDD
#
        ldl     im_mac(g5),g0           # Get the WWN for this server
        ldos    im_tid(g5),g2           # Get the Target ID for this server
        ldconst FALSE,g3                # g3 = Ignore new servers

        PushRegs(r3)
        lda     im_iname(g5),g4         # g4 = iSCSI name
        call    DEF_WWNLookup
        mov     g0,r4
        PopRegsVoid(r3)
        mov     r4,g3

        ldconst 0xffffffff,r3           # Set SID number to not found
        cmpobne g3,r3,.bldmaci_50       # Jif a matching Server was found
#
.if FE_ISCSI_CODE
#
# --- if iSCSI Interfaces - skip the port check & portdb check
#
        ldob    im_flags(g5),r4         # r8 = IMT flags byte
        bbs     im_flags_iscsi,r4,.bldmaci_10 # Jif iSCSI interface
.endif  # FE_ISCSI_CODE
#
# --- Server not found.  Check if the server is a switch.
#
        ld      im_cimt(g5),r4          # Get CIMT
        ldob    ci_num(r4),r6           # Get interface number
#
# ---   check reserved nphandles, we cant tell fl_port from nport on the 2400
#       by the nphandle so we just go the fl_port path.
#
        ldos    im_fcaddr(g5),r5
        ldconst 0x7FC,r4                #  check low value
        cmpobl  r5,r4,.bldmaci_7        #  if less than or =
        ldconst 0x7FF,r4                #  check high val
        cmpobg  r5,r4,.bldmaci_7        #  if greater than
        b      .bldmaci_45              #  it was a reserved port jump to invalid

#
# --- Get ptr to port ID in Port DB structure for this LID under this chip
#
.bldmaci_7:
        ld      im_cimt(g5),r4          # Get CIMT
        ldob    ci_num(r4),r4           # Get interface number
#
        shlo    11,r4,r6                # multiply chip instance by 2048
        ldob    dbflags(r6)[r5*1],r6    # get flag byte
        cmpobe  FALSE,r6,.bldmaci_10    # Jif database not valid
#
        ldconst PORTDBSIZ,r6            # sizeof Port DB struct
        mulo    r6,r5,r6                # Index via LID into Port DB array
        ld      portdb[r4*4],r7         # Get anchor for chip's Port DB array
        ld      pdbpid(r7)[r6*1],r8     # Get the Port ID
        ldconst 0xFF0000FF,r9           # Name server port ID mask
        and     r8,r9,r8                # Ignore last byte of port ID
        ldconst 0xFC0000FF,r9           # Name server port ID
        cmpobe  r8,r9,.bldmaci_45
#
# --- Check if a new server record exists for this server
#
.bldmaci_10:
        ldl     im_mac(g5),g0           # Get the WWN for this server
        ldos    im_tid(g5),g2           # Get the Target ID for this server
        ldconst TRUE,g3                 # g3 = Include new servers

        PushRegs(r3)
        lda     im_iname(g5),g4         # g4 = iSCSI name
        call    DEF_WWNLookup
        mov     g0,r4
        PopRegsVoid(r3)
        mov     r4,g3

        ldconst 0xffffffff,r3           # Set SID number to not found
        cmpobne g3,r3,.bldmaci_20       # Jif a matching Server was found
#
#     Notify CCB of the new controller
#
        ldl     im_mac(g5),g0           # Get the WWN for this server
        ldos    im_tid(g5),g2           # Get the Target ID for this server
        call    mag$mmczoneinquiry      # Send zoning inquiry MRP to CCB
#
# --- Check for default server with WWN = 0
#
.bldmaci_20:
        movl    0,g0                    # Default Server WWN and original TID
        ldconst FALSE,g3                # g3 = Ignore new servers

        PushRegs(r3)
        ldconst 0,g4                    # g4 = iSCSI name
        call    DEF_WWNLookup
        mov     g0,r4
        PopRegsVoid(r3)
        mov     r4,g3

        ldconst 0xffffffff,r3           # Set SID number to not found
#
        cmpobne g3,r3,.bldmaci_50       # Jif a default server was found
#
#
# --- There is neither a defined server or a default server available
#
.bldmaci_45:
        mov     0,g1                    # g1 = Config record size
        stob    g1,im_pri(g5)           # set server priority to zero
        ldconst 0xFFFF,r4               # Invalid server ID
        stos    r4,im_sid(g5)           # Set invalid Server ID in IMT
        b       .bldmaci_1000           # return
#
# --- Setup pointers to the SDD and im_cfgrec
#
.bldmaci_50:
        ld      S_sddindx[g3*4],g3      # get pointer to SDD
#
        ldob    sd_pri(g3),r4           # get server priority
        stob    r4,im_pri(g5)           # copy it to IMT
        ldos    sd_sid(g3),r4           # get server ID
        stos    r4,im_sid(g5)           # copy it to IMT
#
        mov     0,r4                    # r4 = Device Attributes
        ldconst imtsize-im_cfgrec,r7    # r7 = Maximum size of Config Records
        lda     im_cfgrec(g5),g0        # g0 = IMT configuration record address
        mov     0,g1                    # g1 = Config record size
#
# --- Loop to handle each LVM in the SDD --------------------------------------
#     Add it to the im_cfgrec table and allocate an ILMT
#
        ld      sd_lvm(g3),r10          # r10 = pointer to first LVM
#
.bldmaci_130:
        cmpobe  0,r10,.bldmaci_1000     # Jif no LUN/VID found at this LVM
        cmpobge g1,r7,.bldmaci_1000     # Jif no more will fit in im_cfgrec
        ldos    lv_vid(r10),r3          # r3 = VID
        ldos    lv_lun(r10),r5          # r5 = LUN
        ld      vcdIndex[r3*4],g2       # g2 = VCD entry
        ld      lv_nlvm(r10),r10        # r10 = next LUN/VID entry
        ldob    vc_stat(g2),g2          # g2 = VCD status
        bbs     vc_copy_dest_ip,g2,.bldmaci_130 # Jif this VDisk is the
                                        #  destination of a Mirror/Copy = Hide
                                        #  from the server
        stos    r4,im_cfgattr(g0)       # store the device attributes
        stos    r5,im_cfglun(g0)        # store the assoc. LUN
        stos    r3,im_cfgvid(g0)        # store the device ID (VID)
        addo    im_cfgrecsiz,g1,g1      # Add 1 more config record to the size
        lda     im_cfgrecsiz(g0),g0     # point to next config record
#
# --- Determine if another ILMT needs to be created for this LUN/VID
#     We should always pass these sanity checks.
#
        ldconst MAXVIRTUALS,g2          # g2 = max. VID number supported
        cmpobge r3,g2,.bldmaci_400      # Jif specified VID not supported
        ldconst LUNMAX,g2               # g2 = max. LUN number supported
        cmpobge r5,g2,.bldmaci_401      # Jif specified assoc. LUN not supported
        shlo    2,r3,r3                 # r3 = specified device VID * 4
        shlo    2,r5,r5                 # r5 = specified assoc. LUN * 4
        lda     MAG_VDMT_dir(r3),r3     # r3 = pointer to assoc. VDMT
        ld      (r3),r3                 # r3 = assoc. VDMT
        cmpobe  0,r3,.bldmaci_402       # Jif no assoc. VDMT defined
        lda     im_ilmtdir(r5),r5       # r5 = index to assoc. ILMT in IMT
        addo    g5,r5,r5                # r5 = pointer to assoc. ILMT in IMT
        ld      (r5),g2                 # g2 = assoc. ILMT if LUN already
                                        #  defined
        cmpobne 0,g2,.bldmaci_403       # Jif LUN already defined for this
                                        #  image
#
# --- Get an ILMT for this LUN/VID and fill it in
#
        call    mag$get_ilmt            # allocate an ILMT to use for this LUN
                                        #  on this image
                                        # g2 = ILMT for this LUN
        st      g5,ilm_imt(g2)          # save assoc. IMT in ILMT
        st      r3,ilm_vdmt(g2)         # save assoc. VDMT in ILMT
        st      g2,(r5)                 # save ILMT in IMT ILMT directory
        stos    r4,ilm_attr(g2)         # save attributes in ILMT
        lda     MAG_event_tbl,r5        # r5 = ILMT event handler table
        st      r5,ilm_ehand(g2)        # save ILMT event handler table
        lda     normtbl1,r5             # r5 = ILMT command index table
        st      r5,ilm_cmdtbl(g2)       # save command index table
        lda     cmdtbl1,r5              # r5 = command handler table address
        st      r5,ilm_origcmdhand(g2)  # save as original command handler table
        st      r5,ilm_cmdhand(g2)      # save as command handler table
        ld      vdm_itail(r3),r5        # r5 = last ILMT on VDMT/ILMT list
        st      g2,vdm_itail(r3)        # save new list tail element
        cmpobne 0,r5,.bldmaci_240       # Jif elements on list
#
# --- First ILMT in the VDMT. Save it to the head and retrieve the PRR
#     config from the BE. Send an MRP request from the FE to BE to
#     retrieve the current PRR config. The updation of FE PR structs will
#     happen in the MRP completion callback context. This is to avoid locking
#     up the isp$monitor task untill the config update is done - the original
#     problem we tried to solve by rewriting the PRR code!!! - Raghu
#
        st      g2,vdm_ihead(r3)        # save ILMT as head element
        PushRegs(r5)
        ldos    vdm_vid(r3),g0          # g0 = vid
        call    pr_cfgRetrieve
        PopRegsVoid(r5)                 # Restore registers
        b       .bldmaci_250
#
.bldmaci_240:
        st      g2,ilm_link(r5)         # link new ILMT onto end of list
#
# --- If a persistent reservation is present, the command handler may have to
#     to be modified. Call the function to set the appropriate command handler.
#
        PushRegs(r5)
        mov     g2,g0
        call    pr_updCmdHandler
        PopRegsVoid(r5)                 # Restore registers
#
.bldmaci_250:
        lda     modesns1,r5             # r5 = default working environment
                                        #  table address
        st      r5,ilm_dfenv(g2)        # save default working environment
                                        #  table address
        b       .bldmaci_130            # process another LVM
#
# Sanity errors - Code bugs.  Log a firmware alert and error trap.
#
.bldmaci_400:
        ldconst magd_sft8,r11           # r11 = error code to log
        b       .bldmaci_410

.bldmaci_401:
        ldconst magd_sft9,r11           # r11 = error code to log
        b       .bldmaci_410

.bldmaci_402:
        ldconst magd_sft10,r11          # r11 = error code to log
        b       .bldmaci_410

.bldmaci_403:
        ldconst magd_sft11,r11          # r11 = error code to log
.bldmaci_410:
        lda     magd_sft,g0             # g0 = Software Fault Log Area
        st      r11,efa_ec(g0)          # Save the Error Code
        st      r3,efa_data(g0)         # Save selected registers
        st      r5,efa_data+4(g0)
        st      g2,efa_data+8(g0)
        st      g5,efa_data+12(g0)
        ldconst 20,r11                  # Number of bytes saved (ec + data)
        st      r11,mle_len(g0)         # Save the number of bytes to send
        call    M$soft_flt              # Log failure, possibly error trap
#
        ldconst 3000,g0                 # Wait so the log can get to the CCB
        call    K$twait
        b       .err26                  # Error trap
#
# --- All done
#
.bldmaci_1000:
        stos    g1,im_cfgsiz(g5)        # save configuration record size in IMT
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: mag$vdiskchange
#
#  PURPOSE:
#       Performs the processing necessary to service a virtual drive
#       changed message.
#
#  DESCRIPTION:
#       Issues any necessary inquiry request and when the
#       inquiry data is received checks if any changes were made
#       to my environment. If not, processing is done. If changes
#       were made, determines what the changes were and makes the
#       appropriate changes in the working environment.
#
#  CALLING SEQUENCE:
#       call    mag$vdiskchange
#
#       Note: This routine assumes that it runs at the process level!!!
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.  !!!
#
#******************************************************************************
#
mag$vdiskchange:
#
# --- Set up the non-device specific part of the inquiry data
#
        lda     MAG_inq_data,r12        # Inquiry data pointer
        ldconst VERS,r4                 # Set up version
        ldconst REV,r5                  # Set up revision
        stos    r4,vq_vers(r12)         # Store the version
        ldconst SECSIZE,r6              # Set up sector size
        stos    r5,vq_rev(r12)          # Store the revision
        ldconst MAXIO*8,r4              # Set up max sector count to 16MB
        st      r6,vq_secsize(r12)      # Store the sector size
        ldconst MAXVIRTUALS,r3          # Set up max virtual devices
        st      r4,vq_maxio(r12)        # Store the max sector count
        st      r3,vq_vcnt(r12)         # Store the max virtual devices
#
        call    mag$upinq1              # update INQUIRY data part 1
#
# --- Copy the capacities from the VCD to the temporary inquiry data buffer
#
        ldconst vq_capacity,r12         # Point to start of capacity area
                                        #   in the temporary buffer
        ldconst MAXVIRTUALS,r11         # Number of entries to check
.mvc10:
        subo    1,r11,r11               # Point to next entry
        movl    0,r4                    # Assume capacity = 0, also used if
                                        #   VCD does not exist
        ld      vcdIndex[r11*4],r10     # Get VCD pointer
        cmpobe  0,r10,.mvc20            # Jif VCD does not exist
#
#   Determine if this VDisk is a Destination of a Copy/Mirror op and if so,
#       set the capacity to zero so that any VDMTs built will be removed.
#       This will cause the Servers to no longer see this VDisk and CDriver will
#       report Unknown LUN errors while the Copy/Mirror is in progress.
#
        ldob    vc_stat(r10),r3         # r3 = VCD Status
        bbs     vc_copy_dest_ip,r3,.mvc20 # Jif the Destination of a Copy/Mirror
        ldl     vc_capacity(r10),r4     # Get capacity
.mvc20:
        stl     r4,temp_inq_data(r12)[r11*8]  # Save capacity
        cmpobne 0,r11,.mvc10            # Jif not done
#
# --- Process vrinquiry response data
#
        lda     temp_inq_data,g1        # g1 = INQUIRY data address
        call    mag$procinq             # process new inquiry data
        ldos    K_ii+ii_status,r3       # Get initialization status
        setbit  iivdmt,r3,r3            # Set VDMTs Built Flag
        stos    r3,K_ii+ii_status       # Save the new status
#
        ret
#
#******************************************************************************
#
#  NAME: mag$procinq
#
#  PURPOSE:
#       Processes new INQUIRY data.
#
#  DESCRIPTION:
#       Figures what differences are reflected in newly received
#       INQUIRY data from what has been stored in input xxx_inq_data
#       area and processes those changes appropriately. Two sections
#       of INQUIRY data are processed, non-drive INQUIRY data and
#       drive INQUIRY data. Non-drive INQUIRY data is processed by
#       mag$upinq1 routine after the new data is copied into the
#       MAG_inq_data area. Drive INQUIRY data is processed four
#       different ways:
#
#       1). Virtual drive changed capacities
#       2). Virtual drive deleted
#       3). Virtual drive added
#       4). Virtual drive unchanged
#
#  CALLING SEQUENCE:
#       call    mag$procinq
#
#  INPUT:
#       g1 = address where INQUIRY data resides.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$procinq:
        lda     MAG_inq_data,r10        # r10 = local inquiry data area
#
# --- Process INQUIRY data part 2 (drive related data)
#
        ld      vq_vcnt(r10),r7         # r7 = max. # virtual drives
        lda     vq_capacity(r10),r10    # r10 = pointer to drive capacity in
                                        #  active (old) INQUIRY data
        lda     vq_capacity(g1),r3      # r3 = pointer to drive capacity in
                                        #  new INQUIRY data
        mov     0,r11                   # r11 = index into the above tables
.procinq50:
        ldl     (r3)[r11*8],r4          # r4,r5 = new drive size value
        ldl     (r10)[r11*8],r8         # r8,r9 = old drive size value
        ld      MAG_VDMT_dir[r11*4],r6  # r6 = old assoc. VDMT (0=none)
        cmpobne r4,r8,.procinq52        # Jif sizes are different (lower word)
        cmpobe  r5,r9,.procinq100       # Jif sizes are the same (upper/lower)
#
# --- Drive sizes different
#
.procinq52:
        cmpobne 0,r8,.procinq70         # Jif drive was defined before
        cmpobne 0,r9,.procinq70         # Jif drive was defined before
#
# --- Drive has been added logic follows. -------------------------------------
#
        movq    g0,r12                  # save g0-g3
c       g0 = get_vdmt();                # Allocate a VDMT for new drive
.ifdef M4_DEBUG_VDMT
c fprintf(stderr, "%s%s:%u get_vdmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_VDMT
        st      g0,MAG_VDMT_dir[r11*4]  # save VDMT in VDMT directory
        cmpobe  0,r6,.procinq53
        ld      vdm_resv(r6),r6         # Load persistent reservation pointer
        st      r6,vdm_resv(g0)         # Store persistent reservation pointer
                                        # in the new VDMT
.procinq53:
        stl     r4,(r10)[r11*8]         # save drive capacity in active INQUIRY table
        stl     r4,vdm_devcap(g0)       # save new drive capacity in VDMT
        stos    r11,vdm_vid(g0)         # save VID in VDMT
        movq    r12,g0                  # restore g0-g3
        b       .procinq100             # and check next drive
#
.procinq70:
        cmpobne 0,r4,.procinq90         # Jif drive size has changed (lower)
        cmpobne 0,r5,.procinq90         # Jif drive size has chgd (upper/lower)
#
# --- Drive has been deleted logic follows. -----------------------------------
#
        stl     r4,(r10)[r11*8]         # clear drive capacity in active INQUIRY table
        cmpobe  0,r6,.procinq100        # Jif no VDMT defined. Note that this should never occur.
        st      r4,MAG_VDMT_dir[r11*4]  # clear VDMT from VDMT directory
        mov     g0,r4                   # save g0
        mov     r6,g0                   # g0 = assoc. VDMT being deleted
        call    mag$del_vdmt            # process VDMT deletion
        mov     r4,g0                   # restore g0
        b       .procinq100             # and check next drive
#
# --- Drive size has changed logic follows. -----------------------------------
#
.procinq90:
        stl     r4,(r10)[r11*8]         # save new drive size in table
        cmpobe  0,r6,.procinq100        # Jif no VDMT (this should NEVER occur)
        stl     r4,vdm_devcap(r6)       # save new drive capacity in VDMT
        ld      vdm_ihead(r6),r9        # r9 = first assoc. ILMT on list
        cmpobe  0,r9,.procinq100        # Jif no ILMTs assoc. with VDMT
# Set bit in ilm_flag2 to set a unit attention indicating "volume set modified"
        ldob    ilm_flag2(r9),r15
        setbit  6,r15,r15
        stob    r15,ilm_flag2(r9)
#
# NOTE! - mode page only supports 3 bytes of capacity.
#   Need to change to support 8 bytes when decided on
#
c       r14 = r4;                       # If too big, make as big as possible.
c       if (r5 != 0 || (r14 & 0xff000000) != 0) {
c           r14 = 0x00ffffff;
c       }                               # Top byte is "density".
        bswap   r14,r6                  # drive capacity in big endian format
#
c       *(UINT64*)&r4 = *(UINT64*)&r4 / 2000ULL; # Convert capacity into # of heads
c       if (r5 != 0 || (r4 & 0xff000000) != 0) {
c           r4 = 0x00ffffff;
c       }
c       r4 = (r4 << 8) | 0x14;          # Logical or in # of heads to # cylinders.
        bswap   r4,r4                   # make into big endian
.procinq95:
        ld      ilm_wkenv(r9),r15       # r15 = assoc. working env. table
        st      r6,modesns1_cap(r15)    # save new drive size in env. table
        st      r4,mspg4_cylnum(r15)    # save new # cylinders in env. table
# NOTE! - end of mode page only supports 3 bytes of capacity.
#
        ldob    ilm_flag2(r9),r15       # r15 = ILMT flag byte #2
        setbit  4,r15,r15               # set flag to give SENSE to initiator
                                        #  indicating MODE parameters changed
        stob    r15,ilm_flag2(r9)       # save updated flag byte #2
        ld      ilm_link(r9),r9         # r9 = next ILMT assoc. with VDMT
        cmpobne 0,r9,.procinq95         # Jif more ILMTs assoc. with VDMT
#
# --- Process next drive in list ----------------------------------------------
#
.procinq100:
        addo    1,r11,r11               # inc. drive index
        cmpobne r7,r11,.procinq50       # Jif more drives to check
        ret
#
#******************************************************************************
#
#  NAME: mag$upinq1
#
#  PURPOSE:
#       Processes INQUIRY data part 1 (non-drive inquiry data)
#
#  DESCRIPTION:
#       Saves components of the INQUIRY data in other fields for
#       use at a later time. Converts pieces of the INQUIRY data
#       into ASCII characters and stores where needed.
#
#  CALLING SEQUENCE:
#       call    mag$upinq1
#
#  INPUT:
#       INQUIRY data stored in MAG_inq_data area.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$upinq1:
        lda     MAG_inq_data,r4         # r4 = inquiry data pointer
        ldos    vq_vers(r4),r5          # r5 = version
        ldconst 0x30,r8
        mov     0,r10                   # r10 = rev. digit index/count
        lda     inquiry1_rev,r11        # r11 = inquiry revision field ptr.
        cmpobg  10,r5,.upinq1_42        # Jif version < 10
        divo    10,r5,r7                # r7 = version/10
        mulo    10,r7,r9                # r9 = first digit value * 10
        addo    r8,r7,r7                # r7 = first version digit
#        stob    r7,(r11)[r10*1]         # save version digit
        addo    1,r10,r10               # inc. digit index
        subo    r9,r5,r5                # r5 = second digit value
.upinq1_42:
        addo    r8,r5,r5                # r5 = second digit
#        stob    r5,(r11)[r10*1]         # save second digit
        addo    1,r10,r10               # inc. digit index
        ldconst '.',r5                  # put '.' into revision
#        stob    r5,(r11)[r10*1]         # save '.'
        addo    1,r10,r10               # inc. digit index
        ldos    vq_rev(r4),r5           # r5 = revision
        cmpobg  10,r5,.upinq1_43        # Jif revision < 10
        divo    10,r5,r7                # r7 = revision/10
        mulo    10,r7,r9                # r9 = first digit value * 10
        addo    r8,r7,r7                # r7 = first revision digit
#        stob    r7,(r11)[r10*1]         # save revision digit
        addo    1,r10,r10               # inc. digit index
        subo    r9,r5,r5                # r5 = second digit value
        cmpoble 4,r10,.upinq1_44        # Jif 4 digits already saved
.upinq1_43:
        addo    r8,r5,r5                # r5 = second digit
#        stob    r5,(r11)[r10*1]         # save second digit
        addo    1,r10,r10               # inc. digit index
        cmpoble 4,r10,.upinq1_44        # Jif 4 digits already saved
        ldconst ' ',r5                  # put ' ' in remainder of field
#        stob    r5,(r11)[r10*1]         # save ' '
.upinq1_44:
        ld      K_ficb,r5               # r5 = FICB
        ld      fi_vcgid(r5),r5         # r5 = Virtual controller group
        mov     8,r10                   # r10 = serial # digit count
        lda     inquiry1_sn,r11         # r11 = inquiry data serial # field ptr.
        lda     0x30,r8
        lda     0x39,r7                 # r7 = compare value for hex numbers
        st      r5,inqpg_83_sn1         # save system serial number in inquiry
        ldob    MAGD_SCSI_WHQL,r3       # is WHQL compliance active??
        cmpobe 0,r3,.upinq1_45          # Jif no
        st      r5,inqpg83_3bytesn      # save system serial number in inquiry(For WHQL descriptor)
.upinq1_45:
        and     0x0f,r5,r6              # r6 = serial # digit
        addo    r8,r6,r6                # calc. ASCII code
        cmpoble r6,r7,.upinq1_46        # Jif 0-9
        addo    7,r6,r6                 # A-F
.upinq1_46:
        cmpdeco 1,r10,r10               # dec. digit count
        stob    r6,(r11)[r10*1]         # save serial # digit
        shro    4,r5,r5                 # shift to next digit
        bne    .upinq1_45               # Jif loop count <> 0
        ret
#
#******************************************************************************
#
#  NAME: mag$del_vdmt
#
#  PURPOSE:
#       Performs the necessary processing to delete a VDMT.
#
#  DESCRIPTION:
#       Identifies all associated ILMTs for the specified VDMT,
#       aborts any active tasks associated with these ILMTs, removes
#       the ILMTs from service, deallocates the associated ILMTs,
#       removes the specified VDMT from service and deallocates the
#       VDMT.
#
#  CALLING SEQUENCE:
#       call    mag$del_vdmt
#
#  INPUT:
#       g0 = VDMT to delete
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$del_vdmt:
#
# --- Identify all associated ILMTs for the specified VDMT, abort
#       any active tasks associated with these ILMTs, remove the
#       ILMTs from service, deallocate the associated ILMTs.
#
        movt    g0,r12                  # save g0-g2
        mov     g6,r15                  # save g6
.delvdmt_10:
        ld      vdm_ihead(g0),g2        # g2 = first assoc. ILMT with VDMT
        cmpobe  0,g2,.delvdmt_80        # Jif no ILMTs assoc. with VDMT

        ld      ilm_link(g2),r4         # r4 = next ILMT on list
        st      r4,vdm_ihead(g0)        # remove ILMT from list
        cmpobne 0,r4,.delvdmt_20        # Jif list is empty
        st      r4,vdm_itail(g0)        # clear list tail pointer
.delvdmt_20:
        mov     0,r4
        st      r4,ilm_link(g2)         # clear ILMT link field
        ld      ilm_imt(g2),r5          # r5 = assoc. IMT of ILMT
        cmpobe  0,r5,.delvdmt_50        # Jif no IMT assoc. with ILMT
        mov     g2,g6                   # g6 = ILMT address
.delvdmt_30:
        ld      ilm_whead(g2),g1        # g1 = first task ILT on work queue
        cmpobe  0,g1,.delvdmt_40        # Jif no tasks on work queue
        ld      inl2_ehand(g1),r4       # r4 = task event handler table
        call    mag$remtask             # remove ILT from work queue
        ld      inl2_eh_abort(r4),r6    # r6 = task's abort handler routine
        callx   (r6)                    # call task's abort handler routine
        b       .delvdmt_30             # and check for any more tasks on
                                        #  work queue
.delvdmt_40:
#
# --- Set up to locate ILMT being processed in the IMT ILMT directory.
#
        mov     0,r6                    # r6 = LUN # being processed
        ldconst LUNMAX,r7               # r7 = max. # LUNs supported
.delvdmt_45:
        ld      im_ilmtdir(r5)[r6*4],r8 # r8 = ILMT in IMT directory
        cmpobe  g2,r8,.delvdmt_47       # Jif found the ILMT in IMT directory
        addo    1,r6,r6                 # inc. to next ILMT in IMT directory
        cmpobne.t r7,r6,.delvdmt_45     # Jif more ILMTs to check in IMT
                                        #  directory
        b       .delvdmt_50             # didn't find ILMT in IMT directory.
                                        # continue deleting ILMT from existence.
#
# --- Found ILMT in IMT directory
#
.delvdmt_47:
#
# --- Check if associated with MAG-to-MAG image
#
.if     MAG2MAG
        cmpobne.t 0,r6,.delvdmt_49      # Jif not LUN 0
        ldob    im_flags(r5),r8         # r8 = IMT flags byte
        bbc     im_flags_mtml,r8,.delvdmt_49 # Jif not a MAG-to-MAG image
#
# --- MAG-to-MAG image. Check for LUN 0 special processing.
#
        mov     0,r8
        st      r8,ilm_vdmt(g2)         # clear assoc. VDMT field in ILMT
        lda     LLD$cmdtbl1a,r8         # r8 = command handler table address
        st      r8,ilm_origcmdhand(g2)  # save as original command handler table
                                        #  address
        st      r8,ilm_cmdhand(g2)      # save command handler table address
        b       .delvdmt_60             # and don't remove ILMT from IMT
#
.delvdmt_49:
.endif  # MAG2MAG
#
        mov     0,r8                    # set up to clear ILMT from IMT
                                        #  directory
        st      r8,im_ilmtdir(r5)[r6*4] # clear ILMT from IMT directory
#
# --- Deallocate ILMT
#
.delvdmt_50:
.ifdef M4_DEBUG_ILMT
c fprintf(stderr, "%s%s:%u put_ilmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_ILMT
c       put_ilmt(g2);                   # Deallocate ILMT and working environment
.delvdmt_60:
        b       .delvdmt_10
#
.delvdmt_80:
#
# --- Remove the VDMT from service and deallocate the VDMT.
#
        ldos    vdm_vid(g0),r4          # r4 = assigned VID
        shlo    2,r4,r4                 # r4 = VID * 4
        lda     MAG_VDMT_dir(r4),r4     # r4 = address where VDMT resides
        ld      (r4),r5                 # r5 = VDMT in directory
        cmpobne g0,r5,.delvdmt_90       # Jif VDMT in directory does not match
        mov     0,r5
        st      r5,(r4)                 # clear VDMT from directory
.delvdmt_90:
        PushRegs(r5)
        call    pr_rmVID
        PopRegsVoid(r5)
.ifdef M4_DEBUG_VDMT
c fprintf(stderr, "%s%s:%u put_vdmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_VDMT
c       put_vdmt(g0);                   # Deallocate VDMT
        movt    r12,g0                  # restore g0-g2
        mov     r15,g6                  # restore g6
        ret
#
#******************************************************************************
#
#  NAME: mag$serverchange
#
#  PURPOSE:
#       This routine checks for zoning configuration changes for
#       all active and inactive images.
#
#  DESCRIPTION:
#       This routine transcends the IMT allocated list and compares the
#       IMT configuration records with the current SDD/LVM/VCD data.
#       It updates the current image configuration and makes the necessary
#       changes to the image to reflect any changes.
#
#  CALLING SEQUENCE:
#       call    mag$serverchange
#
#       Note: This routine assumes that it runs at the process level!!!
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.  !!!
#
#******************************************************************************
#
mag$serverchange:
        ld      mag_imt_head,g5         # g5 = first IMT on allocated list
        cmpobe  0,g5,.msc1000           # Jif no IMTs on allocated list
#
# --- Loop here to check each IMT on the allocated list -----------------------
#
.msc5:
#
# --- Don't delete the IMT if this has a pending vrp
#
.if 0   # pendVRP is dead code, always zero.
        ld      im_pendvrp(g5),r5       # r5 = check for pending image
        cmpobne 0,r5,.msc500            # Jif pending image
.endif  # pendVRP is dead code, always zero.
#
# --- Update IMTs with any changes in the server records that have occurred.
#
# --- Check for a WWN of zero.  If the IMT was previously attempted to be
#     deleted, but all ILTs were not yet completed, the WWN would have been
#     cleared but the IMT would have been left on the inactive queue.
#
        ldl     im_mac(g5),g0           # Get the WWN for this server
        cmpobne 0,g0,.msc7              # Jif WWN is not zero
        cmpobe  0,g1,.msc8              # Jif WWN is zero
#
# --- Look for a matching WWN in the SDD
#
.msc7:
#
#     Note: Since Vdisk are never mapped to control ports,
#           IMTs associated with control ports will be left alone.
#
        ldos    im_tid(g5),g2           # g2 = The Target ID for this server
        ldconst MAXTARGETS,r3           # r3 = Maximum Target ID
        cmpobge g2,r3,.msc500           # Jif IMT associated with a Control Port
        ldob    im_flags(g5),r5         # r5 = IMT flags byte
                                        # g0,g1 = The WWN for this server
                                        # g2 = The Target ID for this server
        ldconst TRUE,g3                 # g3 = get SID for any server found

        PushRegs(r3)
        lda     im_iname(g5),g4         # g4 = iSCSI name
        call    DEF_WWNLookup
        mov     g0,r4
        PopRegsVoid(r3)
        mov     r4,g3

        ldconst 0xffffffff,r3           # Set SID number to not found
        cmpobe  g3,r3,.msc8             # jif a SID was not found - delete imt
#
                                        # Valid SID was found:
        bbs     im_flags_mtml,r5,.msc10 # Jif MAG-to-MAG link - continue
        ld      S_sddindx+sx_sdd[g3*4],r6 # get pointer to SDD
        ld      sd_attrib(r6),r6        # get server attributes
        bbc     sdunmanaged,r6,.msc10   # Jif server managed - continue
#
# --- Look for the default server mapping since this
#     server exists and is unmanaged
#
        movl    0,g0                    # Look for default server w/ orig TID
                                        # g2 = The Target ID for this server
        ldconst FALSE,g3                # g3 = Ignore new servers

        PushRegs(r3)
        ldconst 0,g4                    # g4 = iSCSI name
        call    DEF_WWNLookup
        mov     g0,r4
        PopRegsVoid(r3)
        mov     r4,g3

        ldconst 0xffffffff,r3           # Set SID number to not found
        cmpobne g3,r3,.msc10            # Jif a default server was found
                                        # else drop through and delete imt
#
# --- Server no longer exists so delete the IMT and any LTMTs
#     This will delete all servers that are not defined to the system that
#     were managed before (unmanaged servers use the default server).
#     These will be reallocated the next time they send a command to Tbolt.
.msc8:
######### Patch/Fix for #Tbolt00011123
        ldob    im_flags(g5),r5         # r5 = IMT flags byte
        bbs     im_flags_mtml,r5,.msc500 # Jif MAG-to-MAG link - continue
######### Patch/Fix for #Tbolt00011123
#
#       Delete the unused server
#
        ld      im_link2(g5),r5         # r5 = next IMT on allocated list
        call    mag$del_imt             # Delete IMT
        mov     r5,g5                   # g5 = next IMT on allocated list
        b       .msc550                 # Go to next IMT
#
# --- Found a server ----------------------------------------------------------
#     Copy the LVM data into the IMTs im_cfgrec table
#
# --- Setup pointers to the SDD and im_cfgrec
#
.msc10:
        ld      S_sddindx[g3*4],g3      # get pointer to SDD
        ldos    sd_sid(g3),r3           # copy server ID from SDD to IMT.
        stos    r3,im_sid(g5)           #   this may change if the IMT used to
                                        #   be based off a default server and
                                        #   is now managed.
        bbs     im_flags_mtml,r5,.msc500 # Jif MAG-to-MAG link - all done
        ldob    sd_pri(g3),r3           # get server priority
        stob    r3,im_pri(g5)           # copy it to IMT
        mov     0,r4                    # r4 = Device Attributes
        ldconst FALSE,r14               # r14 = IMT change flag
        ldconst imtsize-im_cfgrec,r7    # r7 = Maximum size of Config Records
        lda     im_cfgrec(g5),g0        # g0 = IMT configuration record address
        mov     0,g1                    # g1 = Config record size
#
# --- Loop to handle each LVM in the SDD
#     Flag an IMT change if the LUN/VID/or attributes have changed
#     Save any changes in the im_cfgrec table
#
        ld      sd_lvm(g3),r10          # r10 = pointer to first LVM
#
.msc30:
        cmpobe  0,r10,.msc50            # Jif no more LUN/VID
#
# --- These sanity errors are ignored.
#
        cmpobge g1,r7,.msc50            # Jif no more will fit
        ldos    lv_lun(r10),r5          # r5 = potentially 'new' LUN
        ldos    lv_vid(r10),r3          # r3 = potentially 'new' VID
#
        ldos    im_cfgattr(g0),r12      # get current attributes
        cmpobe  r4,r12,.msc38           # compare 'new' and current attributes
                                        # jif same
        ldconst TRUE,r14                # indicate a config change
        stos    r4,im_cfgattr(g0)       # store the new device attributes
.msc38:
        ldos    im_cfglun(g0),r6        # get current LUN
        cmpobe  r5,r6,.msc40            # compare 'new' and current LUNs
                                        # jif same
        ldconst TRUE,r14                # indicate a config change
        stos    r5,im_cfglun(g0)        # store the assoc. LUN
.msc40:
        ldos    im_cfgvid(g0),r9        # get current VID
        cmpobe  r3,r9,.msc42            # compare 'new' and current VIDs
                                        # jif same
        ldconst TRUE,r14                # indicate a config change
        stos    r3,im_cfgvid(g0)        # store the device ID (VID)
.msc42:
        ld      lv_nlvm(r10),r10        # r10 = next LUN/VID entry
        addo    im_cfgrecsiz,g1,g1      # Add 1 more config record to the size
        lda     im_cfgrecsiz(g0),g0     # point to next config record
#
        b       .msc30                  # process another LVM
#
.msc50:                                 # done processing LVMs for this SDD
#
# --- Done looping through LVMs
#     Check total cfgsize to see if any added or removed
#
        ldos    im_cfgsiz(g5),r3
        cmpobe  r3,g1,.msc60

        ldconst TRUE,r14
        stos    g1,im_cfgsiz(g5)        # save configuration record size in IMT
.msc60:
#
#
# --- Changes in the image configuration were detected. The following
#       possibilities exist for each LUN/VID combination:
#
#       1). LUN/VID association remain unchanged.
#       2). LUN assigned to new VID.
#       3). LUN/VID terminated.
#
#       Set up to check each ILMT slot in the IMT ILMT directory. If
#       it exists in the current (old) configuration, determine if
#       it exists in the new configuration. If it does, check if the
#       associated VID is the same. If so, that ILMT/VDMT remains
#       unchanged. If the ILMT remains but is assigned to a different
#       VID, clear the association of the ILMT with the VDMT and
#       establish a new relationship with the new VDMT if it exists.
#       If it doesn't exist, terminate the ILMT. If the ILMT no longer
#       in the new configuration, clear the association of the ILMT
#       with the VDMT and then terminate the ILMT.
#
        ldconst LUNMAX,r4               # r4 = max. # LUNs supported
        lda     im_ilmtdir(g5),r5       # r5 = pointer into IMT ILMT directory
        lda     im_cfgrec(g5),r6        # r6 = pointer to image config. records
        ldos    im_cfgsiz(g5),r7        # r7 = size of image config. records
        mov     0,r8                    # r8 = current LUN being processed
.msc100:
        ld      (r5),r9                 # r9 = ILMT from directory
        movl    r6,r12                  # r12 = pointer to image config. records
                                        # r13 = size of image config. records
#
# --- Loop here on each config record
#
.msc110:
        cmpoble im_cfgrecsiz,r13,.msc120 # Jif more image config. records
                                        #  to check
        mov     0,r12                   # r12 = 0 denoting no image config.
                                        #  record in new config.
# r12 = 0 denotes no matching LUN in the new config
        b       .msc150                 # continue processing ILMT
#
.msc120:
        ldos    im_cfglun(r12),r10      # r10 = LUN # from image config. record
        cmpobe  r8,r10,.msc150          # Jif LUN being processed is in new
                                        #  config. records
        addo    im_cfgrecsiz,r12,r12    # inc. to next config. record
        subo    im_cfgrecsiz,r13,r13    # dec. remaining config. record size
        b       .msc110                 # and check next config. record
#
#       Register definitions: -----------------------------------------------
#
#       r4 = max. # LUNs supported
#       r5 = pointer to current ILMT being processed
#       r6 = pointer to image config. records
#       r7 = size of image config. records
#       r8 = LUN # being processed
#       r9 = ILMT from directory being processed (0=none defined)
#       r12 = pointer to new config. record assoc. with LUN being
#               processed (0=new image does not have config. record
#               associated with the LUN being processed)
#       g5 = IMT being processed
#
.msc150:
        cmpobe  0,r9,.msc300            # Jif no ILMT in directory slot
#
# --- ILMT was defined --------------------------------------------------------
#
        cmpobne 0,r12,.msc160           # Jif image config. record defined
#
# --- CASE: ILMT defined in IMT directory but no image configuration
#       record defined in new image configuration.
#       Terminate ILMT/VDMT association and terminate ILMT.
#
        mov     r9,g6                   # g6 = ILMT being processed
        ld      ilm_ehand(g6),r9        # r9 = ILMT event handler table
        ld      dd_offline(r9),r9       # r9 = ILMT offline event handler
                                        #  routine
        ld      im_cimt(g5),g4          # g4 = assoc. CIMT address
        callx   (r9)                    # call ILMT offline event handler
                                        #  routine
        mov     g6,g2                   # g2 = ILMT being processed
        call    mag$sepilmtvdmt         # separate ILMT, VDMT from each other
        ldconst 0,r15
        st      r15,(r5)                # clear ILMT from directory
.ifdef M4_DEBUG_ILMT
c fprintf(stderr, "%s%s:%u put_ilmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_ILMT
c       put_ilmt(g2);                   # Deallocate ILMT and working environment
        b       .msc400                 # and go to next LUN and process
#
# --- CASE: ILMT defined in IMT directory and an image configuration
#       record is defined in the new image configuration. Determine
#       if the associated VID is the same as it is now or has changed.
#
.msc160:
        ld      ilm_vdmt(r9),r10        # r10 = assoc. VDMT address
        cmpobe  0,r10,.msc165           # Jif VDMT not defined
        ldos    vdm_vid(r10),r11        # r11 = VID #
        ldos    im_cfgvid(r12),r13      # r13 = VID # assigned to this LUN in
                                        #  the image config. record
        cmpobe  r11,r13,.msc400         # Jif same VID as before
#
# --- CASE: ILMT was defined but for a different VDMT. Clear ILMT
#       association with the current VDMT, then check if the newly
#       defined VDMT is configured for this interface.
#
.msc165:
        mov     r9,g2                   # g2 = ILMT being processed
        call    mag$sepilmtvdmt         # separate ILMT, VDMT from each other
        ldos    im_cfgvid(r12),r13      # r13 = VID # assigned to this LUN in
                                        #  the image config. record
        shlo    2,r13,r13               # r13 = new VID * 4
        lda     MAG_VDMT_dir(r13),r13   # r13 = pointer to specified VDMT
        ld      (r13),r13               # r13 = newly specified VDMT to use
        cmpobne 0,r13,.msc170           # Jif new VDMT is defined
#
# --- CASE: ILMT was defined but for a different VDMT and the newly
#       defined VDMT is not configured for this interface. Simply
#       terminate the ILMT that was active for this LUN.
#
        mov     r9,g6                   # g6 = ILMT being processed
        ld      ilm_ehand(g6),r9        # r9 = ILMT event handler table
        ld      dd_offline(r9),r9       # r9 = ILMT offline event handler
                                        #  routine
        ld      im_cimt(g5),g4          # g4 = assoc. CIMT address
        callx   (r9)                    # call ILMT offline event handler
                                        #  routine
        mov     g6,g2                   # g2 = ILMT being processed
        mov     0,r9
        st      r9,(r5)                 # clear ILMT from directory
.ifdef M4_DEBUG_ILMT
c fprintf(stderr, "%s%s:%u put_ilmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_ILMT
c       put_ilmt(g2);                   # Deallocate ILMT and working environment
        b       .msc400                 # and go to next LUN and process
#
# --- CASE: ILMT was defined but for a different VDMT and the newly
#       defined VDT is configured for this interface. Associate ILMT
#       with new VDMT.
#
.msc170:
        ld      ilm_origcmdhand(r9),r14 # r14 = original command handler
                                        #  table
        st      r14,ilm_cmdhand(r9)     # restore original command handler
                                        #  table
        mov     0,r14
        st      r14,ilm_link(r9)        # clear link list field in ILMT
        st      r13,ilm_vdmt(r9)        # save new VDMT in ILMT
        ld      vdm_itail(r13),r14      # r14 = last ILMT on VDMT link list
        cmpobne 0,r14,.msc180           # Jif ILMTs on VDMT link list
        st      r9,vdm_ihead(r13)       # save ILMT as head of list
        b       .msc190
#
.msc180:
        st      r9,ilm_link(r14)        # link ILMT onto end of list
.msc190:
        st      r9,vdm_itail(r13)       # save ILMT as new tail pointer
        ld      vdm_rilmt(r13),r14      # r14 = ILMT of reserving initiator
        cmpobe  0,r14,.msc400           # Jif device not reserved
        lda     cmdtbl2,r14             # r14 = command handler table for
                                        #  other initiators for this device
                                        #  while device is reserved
        st      r14,ilm_cmdhand(r9)     # set command handler table to the
                                        #  device reserved one
        b       .msc400                 # and process next LUN for this image
#
# --- ILMT was not defined -----------------------------------------------------
#
#       No ILMT found in directory for current configuration. Check
#       if a new LUN/VID for the LUN being processed has been
#       defined.
#
.msc300:
        cmpobe  0,r12,.msc400           # Jif this LUN not defined in new image
        ldos    im_cfgvid(r12),r13      # r13 = VID # assigned to this LUN in
                                        #  the image config. record
        shlo    2,r13,r13               # r13 = new VID * 4
        lda     MAG_VDMT_dir(r13),r13   # r13 = pointer to specified VDMT
#
        ld      (r13),r13               # r13 = newly specified VDMT to use
        cmpobe  0,r13,.msc400           # Jif new VDMT is not defined
        mov     r13,g0                  # g0 = VDMT to assoc. LUN with
        mov     g5,g1                   # g1 = assoc. IMT address
        ldos    im_cfgattr(r12),g2      # g2 = attributes
        setbit  31,g2,g2                # indicate ILMT to be placed online
        call    mag$add_ilmt            # add ILMT to this image
#
# --- Go to next LUN and process -----------------------------------------------
#
.msc400:
        addo    4,r5,r5                 # inc. ILMT directory pointer
        addo    1,r8,r8                 # inc. LUN being processed
        cmpobl  r8,r4,.msc100           # Jif more ILMTs to process
#
.msc500:                                # Done with all LUNs
        ld      im_link2(g5),g5         # g5 = next IMT on allocated list
.msc550:
        cmpobne 0,g5,.msc5              # Jif more IMTs to process on allocated
                                        #  list
#
.msc1000:
        ret
#
#******************************************************************************
#
#  NAME: mag$add_ilmt
#
#  PURPOSE:
#       Adds and initializes an ILMT associated with a specified
#       VDMT for the specified IMT.
#
#  DESCRIPTION:
#       Allocates an ILMT, initializes the ILMT and links it with the
#       VDMT, finds the next available LUN for the specified IMT and
#       assigns the ILMT to the LUN.
#
#  CALLING SEQUENCE:
#       call    mag$add_ilmt
#
#  INPUT:
#       g0 = VDMT address to associate ILMT with
#       g1 = IMT address to add ILMT/LUN to
#       g2 = associated attributes for ILMT (LSS)
#            + online flag (MSbit=1)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$add_ilmt:
        movl    g2,r14                  # save g2-g3
                                        # r14 = associated attributes for ILMT
                                        #  + online flag if Bit 31 = 1
        ldos    im_cfgsiz(g1),r4        # r4 = configuration record size
        cmpobe  0,r4,.addilmt_1000      # Jif no configuration record
        lda     im_cfgrec(g1),r6        # r6 = pointer to image cfg. record
        ldconst im_cfgrecsiz,r7         # r7 = sub-record size
        ldos    vdm_vid(g0),r8          # r8 = VID #
.addilmt_01:
        cmpobg  r7,r4,.addilmt_1000     # Jif no match found
                                        #  Skip adding drive/LUN to image if
                                        #  no subrecord for the specified VID
                                        #  was found
        ldos    im_cfgvid(r6),r9        # r9 = subrecord VID #
        cmpobne r9,r8,.addilmt_03       # Jif not a match
#
# --- Match found for VID # being added
#
        ldos    im_cfglun(r6),r9        # r9 = assigned LUN #

.if     MAG2MAG
#
# --- Check if MAG-to-MAG image
#
        ldob    im_flags(g1),r3         # r3 = IMT flags byte
        bbc     im_flags_mtml,r3,.addilmt_02 # Jif not MAG-to-MAG image
#
# --- Transfer processing to lld$add_ilmt routine
#
        mov     r9,g3                   # g3 = assigned LUN #
        call    lld$add_ilmt            # and process through LLD routine
        b       .addilmt_1000           # and we're out of here!
#
.addilmt_02:
.endif  # MAG2MAG

        shlo    2,r9,r9                 # r9 = assigned LUN # * 4
        ldconst 0xffff0000,r3           # r3 = specified attributes mask
        and     r3,r14,r14
        ldos    im_cfgattr(r6),r10      # r10 = attributes from subrecord
        addo    r10,r14,r14             # add subrecord attributes to specified
                                        #  attributes
        lda     im_ilmtdir(g1),r5       # r5 = pointer to base of ILMT directory
        addo    r9,r5,r5                # r5 = pointer to LUN/ILMT directory
                                        #  field to define new ILMT for this IMT
        b       .addilmt_20             # and continue adding ILMT to IMT
#
.addilmt_03:
        addo    r7,r6,r6                # inc. to next subrecord
        subo    r7,r4,r4                # sub. from record size
        b       .addilmt_01             # and check next subrecord
#
# -----------------------------------------------------------------------------
#
#  INPUT:
#
#       r5 = pointer to LUN/ILMT directory field to define
#               new ILMT for this IMT
#       r14 = associated attributes for ILMT
#               + online flag if Bit 31 = 1
#       g0 = specified VDMT address
#       g1 = specified IMT address
#
# -----------------------------------------------------------------------------
#
.addilmt_20:
        call    mag$get_ilmt            # allocate an ILMT
                                        # g2 = ILMT for this LUN
        st      g1,ilm_imt(g2)          # save assoc. IMT in ILMT
        st      g0,ilm_vdmt(g2)         # save assoc. VDMT in ILMT
        st      g2,(r5)                 # save ILMT in IMT ILMT directory
        stos    r14,ilm_attr(g2)        # save attributes in ILMT
        lda     MAG_event_tbl,r3        # r3 = ILMT event handler table
        st      r3,ilm_ehand(g2)        # save ILMT event handler table
        lda     normtbl1,r5             # r5 = ILMT command index table
        st      r5,ilm_cmdtbl(g2)       # save command index table
        lda     cmdtbl1,r5              # r5 = command handler table address
        st      r5,ilm_origcmdhand(g2)  # save as original command handler table
        st      r5,ilm_cmdhand(g2)      # save as command handler table
        ld      vdm_itail(g0),r5        # r5 = last ILMT on VDMT/ILMT list
        st      g2,vdm_itail(g0)        # save new list tail element
        cmpobne 0,r5,.addilmt_40        # Jif elements on list
#
# --- First ILMT in the VDMT. Save it to the head and retrieve the PRR
#     config from the BE. Send an MRP request from the FE to BE to
#     retrieve the current PRR config. The updation of FE PR structs will
#     happen in the MRP completion callback context. This is to avoid locking
#     up the isp$monitor task untill the config update is done - the original
#     problem we tried to solve by rewriting the PRR code!!! - Raghu
#
        st      g2,vdm_ihead(g0)        # save ILMT as head element
        PushRegs(r5)
        ldos    vdm_vid(g0),g0          # g0 = vid
        call    pr_cfgRetrieve
        PopRegsVoid(r5)                 # Restore registers
        b       .addilmt_50
#
.addilmt_40:
        st      g2,ilm_link(r5)         # link new ILMT onto end of list
#
# --- If a persistent reservation is present, the command handler may have to
#     to be modified. Call the function to set the appropriate command handler.
#
        PushRegs(r5)
        mov     g2,g0
        call    pr_updCmdHandler
        PopRegsVoid(r5)                 # Restore registers
#
.addilmt_50:
        lda     modesns1,r5             # r5 = default working environment
                                        #  table address
        st      r5,ilm_dfenv(g2)        # save default working environment
                                        #  table address
#
# --- Handle online/offline status
#
        bbc     31,r14,.addilmt_1000    # Jif ILMT offline
        ld      dd_online(r3),r3        # r3 = ILMT online handler routine
        movt    g4,r4                   # save g4-g6
        ld      im_cimt(g1),g4          # g4 = assoc. CIMT address
        mov     g1,g5                   # g5 = IMT address
        mov     g2,g6                   # g6 = ILMT address
        mov     g2,r7
        cmpobe  0,g4,.addilmt_900       # Jif no CIMT associated with IMT
        callx   (r3)                    # call ILMT online event routine to setup
                                        #  ILMT to be placed online
# Set bit 5 in ilm_flag2 to put up a unit attention indicating "reported luns
# data changed"
        ldob    ilm_flag2(r7),r8
        setbit  5,r8,r8
        stob    r8,ilm_flag2(r7)
.addilmt_900:
        movt    r4,g4                   # restore g4-g6
.addilmt_1000:
        movl    r14,g2                  # restore g2-g3
        ret
#
#******************************************************************************
#
#  NAME: mag$imt_cleanup
#
#  PURPOSE:
#       This routine deletes all inactive IMT for the specified port.
#
#  DESCRIPTION:
#       This routine demolishes an image (IMT), sending an initiator
#       inactive MRP request to the CCB notifying it that the
#       image has been inactive for an extended period of time. It
#       deallocates all resources associated with the image including
#       ILMTs and IMT.
#
#  CALLING SEQUENCE:
#       call    mag$imt_cleanup
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0,g4,g5.
#
#******************************************************************************
#
mag$imt_cleanup:
#
# --- Start with the head of the inactive list
#
.mdic10:
        ld      C_imt_head,g5           # g5 = First IMT on inactive list
        cmpobe  0,g5,.mdic100           # Jif no more IMTs on this list
        ldconst FALSE,r13               # Clear 'IMT to delete' flag
#
# --- Check if the WWN for this IMT is zero.  This is an IMT that
#     was previously attempted to be deleted, but was still in use.
#
#
.mdic20:
        ldl     im_mac(g5),r4           # Get WWN
        ld      im_link(g5),r12         # r12 = next IMT on list
        cmpobne 0,r4,.mdic30            # Jif WWN not zero
        cmpobne 0,r5,.mdic30            # Jif WWN not zero
#
# --- Delete the inactive IMT
#
                                        # g5 = IMT
        call    mag$del_imt             # Delete IMT
        cmpobne FALSE,g0,.mdic30        # Jif IMT deleted
        ldconst TRUE,r13                # Indicate IMT was not deleted.
#
# --- Advance to next IMT on inactive list
#
.mdic30:
        mov     r12,g5                  # Next IMT
        cmpobne 0,g5,.mdic20            # Jif more IMTs on this list
#
# --- Check if any IMTs still need to be deleted.
#
        cmpobe  FALSE,r13,.mdic100
#
# --- Wait 1/4 seconds and try to delete IMTs again
#
        lda     250,g0                  # set up to wait 250 ms
        call    K$twait                 # delay task
        b       .mdic10
#
# --- Exit
#
.mdic100:
        ldconst 0,r3
        st      r3,mag_cleanup_pcb      # Clear PCB pointer
        ret
#
#******************************************************************************
#
#  NAME: MAG$del_all_inact_imt
#
#  PURPOSE:
#       This routine deletes all inactive IMT for the specified port.
#
#  DESCRIPTION:
#       This routine demolishes an image (IMT), sending an initiator
#       inactive MRP request to the CCB notifying it that the
#       image has been inactive for an extended period of time. It
#       deallocates all resources associated with the image including
#       ILMTs and IMT.
#
#  CALLING SEQUENCE:
#       call    MAG$del_all_inact_imt
#
#  INPUT:
#       g0 = Port for which all IMTs will be demolished
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
MAG$del_all_inact_imt:
        movl    g0,r12                  # Save g0,g1
        mov     g5,r15                  # Save g5
        ldconst FALSE,r11
#
# --- Start with the head of the inactive list
#
        ld      C_imt_head,g5           # g5 = First IMT on inactive list
        cmpobe  0,g5,.mdai100           # Jif no more IMTs on this list
#
# --- Check if this IMT is for the specified port
#
.mdai10:
        ld      im_link(g5),r14         # r14 = next IMT on list
        ld      im_cimt(g5),r4          # Get associated CIMT
        ldob    ci_num(r4),r3           # Get port number from CIMT
        cmpobne r3,r12,.mdai30          # Jif not specified port
#
# --- Delete the inactive IMT
#
        call    mag$del_imt             # Delete IMT
        cmpobne FALSE,g0,.mdai30        # Jif IMT deleted
        ldconst TRUE,r11                # Indicate IMT was not deleted.
#
# --- Advance to next IMT on inactive list
#
.mdai30:
        mov     r14,g5                  # Next IMT
        cmpobne 0,g5,.mdai10            # Jif more IMTs on this list
#
# --- Check if any IMT for this port remain on the inactive list
#
        cmpobe  FALSE,r11,.mdai100      # Jif no left over IMTs on this list
        ld      mag_cleanup_pcb,r3
        cmpobne 0,r3,.mdai100           # Jif no process running
c       g0 = -1;                        # Flag task being started.
        st      g0,mag_cleanup_pcb
        lda     mag$imt_cleanup,g0      # establish executive process
        ldconst MAGICPRI,g1
c       CT_fork_tmp = (ulong)"mag$imt_cleanup";
        call    K$fork
        st      g0,mag_cleanup_pcb
#
# --- Exit
#
.mdai100:
        movl    r12,g0                  # Restore g0,g1
        mov     r15,g5                  # Restore g5
        ret
#
#******************************************************************************
#
#  NAME: mag$del_imt
#
#  PURPOSE:
#       This routine deletes an image.
#
#  DESCRIPTION:
#       This routine demolishes an image (IMT), sending an initiator
#       inactive MRP request to the CCB notifying it that the
#       image has been inactive for an extended period of time. It
#       deallocates all resources associated with the image including
#       ILMTs and IMT.
#
#  CALLING SEQUENCE:
#       call    mag$del_imt
#
#       Note: This routine assumes that it runs at the process level!!!
#
#  INPUT:
#       g5 = IMT of image to demolish
#               Note: Before calling this routine, the caller must
#                       remove the IMT from all queues (except the
#                       allocated IMT list) and insured that all
#                       activity related to the image has completed.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$del_imt:

        movq    g0,r12                  # save g0-g3
#
# --- Check that all activity for the image has completed before removing IMT
#
        ldos    im_qdepth(g5),r3        # See if ILT has an IMT pointer.
c       if (r3 != 0) {
.ifndef PERF
c         fprintf(stderr, "%s%s:%u mag$del_imt 0x%08lx has qDepth %lx\n", FEBEMESSAGE, __FILE__, __LINE__, g5, r3);
.endif  # PERF
          b     .mdi30                  # IMT still in use by ILT(s)
c       }
        ldconst LUNMAX,r10              # r10 = max. # LUNs supported
        lda     im_ilmtdir(g5),r11      # r11 = pointer into ILMT directory
.mdi10:
        ld      (r11),r4                # get ILMT assoc. with IMT
        cmpobe  0,r4,.mdi20             # Jif no ILMT for this LUN
        ld      ilm_whead(r4),r3        # get first task/ILT on work queue
        cmpobne 0,r3,.mdi30             # Jif task/ILTs on work queue
        ld      ilm_ahead(r4),r3        # get first task/ILT on abort queue
        cmpobne 0,r3,.mdi30             # Jif task/ILTs on abort queue
        ld      ilm_bhead(r4),r3        # get first task/ILT on blocked queue
        cmpobne 0,r3,.mdi30             # Jif task/ILTs on blocked queue

.mdi20:
        addo    4,r11,r11               # inc. to next ILMT in directory
        subo    1,r10,r10               # dec. LUN count
        cmpobne 0,r10,.mdi10            # Jif more ILMTs to check for
        b       .mdi40
#
# --- Clear out the WWN so this IMT is not reused.  It will be left
#     in the inactive queue.
#
.mdi30:
        ldconst FALSE,r12               # Indicate IMT was not deleted
        movl    0,r6
        stl     r6,im_mac(g5)           # Clear WWN
        b       .mdi1000                # Leave IMT on the inactive queue
#
# --- Remove IMT from the CIMT active list
#
.mdi40:
        ldconst FALSE,r11               # r11 = Flag to show if IMT was found
                                        #   on the active or inactive queue
                                        #   (do not delete - in the process of
                                        #   being activated!)
        ld      im_cimt(g5),r8          # get assoc. CIMT address
        ld      ci_imthead(r8),r4       # get first IMT on list
        cmpobe  0,r4,.mdi90             # Jif no images on list
#
        ld      ci_imttail(r8),r7       # get last IMT on list
#
        ld      im_link(r4),r3          # get next IMT
        cmpobne r4,g5,.mdi70            # Jump if this IMT is not head
        cmpobne r7,g5,.mdi50            # Jump if this IMT is not tail
        ldconst 0,r3
        st      r3,ci_imttail(r8)       # clear tail pointer
.mdi50:
        st      r3,ci_imthead(r8)       # save new head pointer
        ldconst TRUE,r11                # Show the IMT was on the Active List
        b       .mdi90
#
.mdi60:
        mov     r3,r4                   # Increment to next IMT
        cmpobe  r7,r3,.mdi90            # Jif last IMT on the list
.mdi70:
        cmpobe  0,r3,.mdi90             # Jif no IMT on the list
        ld      im_link(r4),r3          # get next IMT
        cmpobne g5,r3,.mdi60            # Jump if not this IMT
#
        cmpobne r7,g5,.mdi80            # Jump if this IMT is not tail
        st      r4,ci_imttail(r8)       # set tail pointer
        ldconst 0,r3
        st      r3,im_link(r4)          # clear next pointer of last IMT
        ldconst TRUE,r11                # Show the IMT was on the Active List
        b       .mdi90
#
.mdi80:
        ld      im_link(g5),r3          # get next IMT
        st      r3,im_link(r4)          # set next IMT on list
        ldconst TRUE,r11                # Show the IMT was on the Active List
.mdi90:
#
# --- Remove IMT from inactive list
#     If found on active list then could skip this inactive check
#
        ld      C_imt_head,r4           # get first IMT on inactive image list
        cmpobe  0,r4,.mdi180            # Jif no images on inactive list
#
        ld      C_imt_tail,r7           # get last IMT on inactive image list
#
        ld      im_link(r4),r3          # get next inactive IMT
        cmpobne r4,g5,.mdi150           # Jump if this IMT is not head
        cmpobne r7,g5,.mdi130           # Jump if this IMT is not tail
        ldconst 0,r3
        st      r3,C_imt_tail           # clear inactive tail pointer
.mdi130:
        st      r3,C_imt_head           # save new inactive head pointer
        ldconst TRUE,r11                # Show the IMT was on the Inactive List
        b       .mdi180
#
.mdi140:
        mov     r3,r4                   # Increment to next IMT
        cmpobe  r7,r3,.mdi180           # Jif last IMT on the list
.mdi150:
        cmpobe  0,r3,.mdi180            # Jif no IMT on the list
        ld      im_link(r4),r3          # get next inactive IMT
        cmpobne g5,r3,.mdi140           # Jump if not this IMT
#
        cmpobne r7,g5,.mdi160           # Jump if this IMT is not tail
        st      r4,C_imt_tail           # set inactive tail pointer
        ldconst 0,r3
        st      r3,im_link(r4)          # clear next pointer of last IMT
        ldconst TRUE,r11                # Show the IMT was on the Inactive List
        b       .mdi180
#
.mdi160:
        ld      im_link(g5),r3          # get next inactive IMT
        st      r3,im_link(r4)          # set next IMT on inactive list
        ldconst TRUE,r11                # Show the IMT was on the Inactive List
.mdi180:
        cmpobne FALSE,r11,.mdi190       # Jif the IMT was removed from a list
        call    MAGD$serverchange       # Notify that an IMT is not being
                                        #  deleted because of the IMT being
                                        #  in the limbo state (in the process
                                        #  of being activated).  This will
                                        #  start the process to go through all
                                        #  the IMTs again to ensure all are
                                        #  valid.
        ldconst FALSE,r12               # Indicate IMT was not deleted
        b       .mdi1000                # Leave the IMT alone for now
#
# --- Remove ILMTs
#
.mdi190:
        mov     0,r11                   # r11 is always 0
        ldconst LUNMAX,r4               # r4 = # ILMTs to check for
        lda     im_ilmtdir(g5),r5       # r5 = pointer into ILMT directory
.mdi200:
        ld      (r5),g2                 # g2 = ILMT to remove from IMT
        cmpobe  0,g2,.mdi210            # Jif no ILMT defined for this LUN
        call    mag$sepilmtvdmt         # separate ILMT, VDMT from each other
        st      r11,(r5)                # clear ILMT out of directory
.ifdef M4_DEBUG_ILMT
c fprintf(stderr, "%s%s:%u put_ilmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_ILMT
c       put_ilmt(g2);                   # Deallocate ILMT and working environment
.mdi210:
        addo    4,r5,r5                 # inc. to next LUN in ILMT directory
        subo    1,r4,r4                 # dec. LUN count
        cmpobne 0,r4,.mdi200            # Jif more ILMTs to check for
#
# --- Check if LTMT exists
#
        ld      im_ltmt(g5),r4          # r4 = assoc. LTMT address
        cmpobe  0,r4,.mdi400            # Jif no LTMT assoc. with IMT
#
# --- Remove link to LTMT
#
        st      r11,im_ltmt(g5)         # clear assoc. LTMT address from IMT
        ld      ltmt_imt(r4),r5         # r5 = assoc. IMT from LTMT
        cmpobne r5,g5,.mdi400           # Jif not the IMT being processed
        st      r11,ltmt_imt(r4)        # clear IMT address from LTMT
        ld      ltmt_tmt(r4),r6         # r6 = assoc. TMT address
        cmpobne 0,r6,.mdi400            # Jif TMT assoc. with LTMT
        movl    g4,r6                   # save g4-g5
        ldconst 0,g5                    # g5 = null TMT address
        mov     r4,g4                   # g4 = LTMT to terminate
        ld      ltmt_ehand(g4),r3       # r3 = LTMT event handler table
        ld      ltmt_eh_tgone(r3),r5    # r5 = LTMT target gone event
                                        #  handler routine
        callx   (r5)                    # and call target gone event
                                        #  handler routine
#
#******************************************************************************
#
# --- Interface to Target Gone event handler routine
#
#   INPUT:
#
#       g4 = assoc. LTMT address
#       g5 = assoc. TMT address (null)
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#******************************************************************************
#
        movl    r6,g4                   # restore g4-g5
#
# --- Update the Number of Hosts count in the CIMT, Deallocate IMT, and return
#
.mdi400:
        ldos    ci_numhosts(r8),r6      # r6 = Count of the Number of Hosts
        cmpobe  0,r6,.mdi500            # Jif offline has zeroed everything.
        subo    1,r6,r6                 # Decrement the count
        stos    r6,ci_numhosts(r8)      # Save the new count
.mdi500:
        call    mag$put_imt             # deallocate IMT
        ldconst TRUE,r12                # Indicate IMT was deleted
#
.mdi1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: mag$sepilmtvdmt
#
#  PURPOSE:
#       This routine performs the processing to separate an ILMT
#       from it's associated VDMT.
#
#  DESCRIPTION:
#       This routine checks if the specified ILMT has the associated
#       device reserved and performs the processing to release the
#       reservation on the device. It unlinks the ILMT from the VDMT
#       link list.
#
#  CALLING SEQUENCE:
#       call    mag$sepilmtvdmt
#
#  INPUT:
#       g2 = ILMT to separate
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$sepilmtvdmt:
        movq    g0,r12                  # save g0-g3
        ld      ilm_vdmt(g2),r6         # r6 = associated VDMT address
        cmpobe  0,r6,.sepilvd_1000      # Jif no VDMT - REDI SAN Links builds
                                        #  an ILMT but may not associate a VDMT
                                        #  with it.
        ld      vdm_rilmt(r6),r7        # r7 = reserving ILMT
        cmpobne g2,r7,.sepilvd_30       # Jif this ILMT not the reserving one
#
# --- Virtual device is assigned to this ILMT. Perform the necessary
#       steps to terminate the assignment.
#       Note: This IMT should never be on the inactive list and
#               still have a virtual device reserved!!!
#
        mov     r6,g3                   # g3 = assoc. VDMT address
        mov     g6,r10                  # save g6
        mov     g9,r11                  # save g9
        mov     0,g9                    # g9 = 0
        mov     g2,g6                   # g6 = ILMT releasing device
        call    mag$locrelease          # process the release of the device
        mov     r11,g9                  # restore g9
        mov     r10,g6                  # restore g6
#
# --- Find and remove ILMT from VDMT link list
#
.sepilvd_30:
        ld      vdm_ihead(r6),r7        # r7 = first assoc. ILMT on list
        mov     0,r8                    # r8 = last ILMT checked on list
.sepilvd_35:
        cmpobe  0,r7,.sepilvd_1000      # Jif no more ILMTs assoc. with VDMT
        cmpobe  g2,r7,.sepilvd_40       # Jif ILMT found on list
        mov     r7,r8                   # r8 = last ILMT checked on list
        ld      ilm_link(r7),r7         # r7 = next ILMT on list
        b       .sepilvd_35             # check next ILMT on list if present
#
.sepilvd_40:
        ld      ilm_link(g2),r7         # r7 = next ILMT on list
        cmpobne 0,r8,.sepilvd_45        # Jif not the first ILMT on list
#
# --- ILMT is the first on the VDMT list.
#
        st      r7,vdm_ihead(r6)        # remove ILMT from top of list
        b       .sepilvd_47
#
# --- ILMT is NOT the first on the list
#
.sepilvd_45:
        st      r7,ilm_link(r8)         # link next ILMT onto previous ILMT
.sepilvd_47:
        cmpobne 0,r7,.sepilvd_1000      # Jif not the last on list
        st      r8,vdm_itail(r6)        # previous ILMT is new tail pointer
.sepilvd_1000:
        movq    r12,g0                  # restore g0-g3
        mov     0,r3                    # clear link
        st      r3,ilm_link(g2)
        ret
#
#******************************************************************************
#
# _______________ DEFAULT MAGNITUDE DEVICE DRIVER ROUTINES ____________________
#
#******************************************************************************
#
#  NAME: mag$dcmdrecv
#
#  PURPOSE:
#       Default SCSI command received event handler routine.
#
#  DESCRIPTION:
#       This routine is called by the channel driver when a SCSI
#       command is received associated with an IMT but specifying
#       a LUN which is not defined for use by the initiator. The
#       routine does the necessary response to tell the initiator
#       to buzz off!!!
#
#  CALLING SEQUENCE:
#       call    mag$dcmdrecv
#
#  INPUT:
#       g1 = pri. ILT at inl2 nest level
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = 0 indicating no assoc. ILMT
#       g7 = assoc. ILT param. structure
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. r3-r15/g4-g7 can be destroyed.
#
#******************************************************************************
#
mag$dcmdrecv:
        movq    g8,r8                   # save g8-g11
        ld      sccdb(g7),g8            # g8 = pointer to CDB
        call    mag$init_task           # initialize task inl2 level data structure
        lda     task_etbl2,r3           # r3 = task event handler table
        st      r3,inl2_ehand(g1)       # save task event handler table
        lda     dnormtbl,g11            # g11 = default cmd. norm. table
        movq    g0,r4                   # save g0-g3
        ldob    (g8),g9                 # g9 = SCSI command code
        lda     dcmdhand,g10            # g10 = default command handler table
        ldob    (g11)[g9*1],g9          # g9 = normalized command code
        movt    g12,r12                 # save g12-g14
        ld      (g10)[g9*4],g10         # g10= command handler routine
        mov     g1,g9                   # g9 = primary ILT address
        callx   (g10)                   # and go to handler routine
        movq    r4,g0                   # restore g0-g3
        movq    r8,g8                   # restore g8-g11
        movt    r12,g12                 # restore g12-g14
        ret
#
#*********************************************************************
#
# --- Input to command handler routines:
#
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = 0 indicating no assoc. ILMT
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#       All registers may be destroyed.
#
#*********************************************************************
#
# --- MAGNITUDE default command normalization table #1
#
# --- Undefined LUN command normalization table
#
        .data
dnormtbl:
        .byte   0,0,0,2,0,0,0,0         # 00-07     03
        .byte   0,0,0,0,0,0,0,0         # 08-0f
        .byte   0,0,1,0,0,0,0,0         # 10-17     12
        .byte   0,0,0,0,0,0,0,0         # 18-1f
        .byte   0,0,0,0,0,0,0,0         # 20-27
        .byte   0,0,0,0,0,0,0,0         # 28-2f
        .byte   0,0,0,0,0,0,0,0         # 30-37
        .byte   0,0,0,0,0,0,0,0         # 38-3f
        .byte   0,0,0,0,0,0,0,0         # 40-47
        .byte   0,0,0,0,0,0,0,0         # 48-4f
        .byte   0,0,0,0,0,0,0,0         # 50-57
        .byte   0,0,0,0,0,0,0,0         # 58-5f
        .byte   0,0,0,0,0,0,0,0         # 60-67
        .byte   0,0,0,0,0,0,0,0         # 68-6f
        .byte   0,0,0,0,0,0,0,0         # 70-77
        .byte   0,0,0,0,0,0,0,0         # 78-7f
        .byte   0,0,0,0,0,0,0,0         # 80-87
        .byte   0,0,0,0,0,0,0,0         # 88-8f
        .byte   0,0,0,0,0,0,0,0         # 90-97
        .byte   0,0,0,0,0,0,0,0         # 98-9f
        .byte   3,0,0,0,0,0,0,0         # a0-a7     a0
        .byte   0,0,0,0,0,0,0,0         # a8-af
        .byte   0,0,0,0,0,0,0,0         # b0-b7
        .byte   0,0,0,0,0,0,0,0         # b8-bf
        .byte   0,0,0,0,0,0,0,0         # c0-c7
        .byte   0,0,0,0,0,0,0,0         # c8-cf
        .byte   0,0,0,0,0,0,0,0         # d0-d7
        .byte   0,0,0,0,0,0,0,0         # d8-df
        .byte   0,0,0,0,0,0,0,0         # e0-e7
        .byte   0,0,0,0,0,0,0,0         # e8-ef
        .byte   0,0,0,0,0,0,0,0         # f0-f7
        .byte   0,0,0,0,0,0,0,0         # f8-ff
#
# --- MAGNITUDE default command handler table
#
# --- Normal operation command handler table
#     Index value gotten from dnormtbl.
#
dcmdhand:
        .word   magd$nolun      # All other commands for unsupported LUNs
        .word   magd$inquiry    # Inquiry
        .word   magd$reqsns     # Request sense
        .word   magd$repluns    # Report LUNs
#
#******************************************************************************
#
#  NAME: magd$nolun
#
#  PURPOSE:
#       Processes all commands received from a host for an
#       undefined LUN other than INQUIRY, REQUEST SENSE and
#       REPORT LUNs.
#
#  DESCRIPTION:
#       Returns appropriate status and sense data to indicate
#       to the issuing host to BUZZ OFF!!!
#
#  CALLING SEQUENCE:
#       call    magd$nolun
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = 0 indicating no assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
nolun_tbld:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   MAGD$sense_nolun        # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
        .text
#
magd$nolun:
        ldq     nolun_tbld,r4           # load op. values into regs.
        ld      nolun_tbld+16,r8
        b       mag1$cmdcom             # and finish processing command
#
#******************************************************************************
#
#  NAME: magd$inquiry
#
#  PURPOSE:
#       Processes an Inquiry command received from a host for an undefined LUN.
#
#  DESCRIPTION:
#       Returns inquiry data to the issuing host.
#
#  CALLING SEQUENCE:
#       call    magd$inquiry
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = 0 indicating no assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
inquiry_tbld:
        .byte   dtxferi,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # # SGL descriptors
        .short  0                       # sense length
#
inquiry_tbld2:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invf1             # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
inquiry_tbld3:
        .byte   dtxfern,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
magd$inquiry:
#
# --- Validate INQUIRY CDB
#
        ldob    1(g8),r4                # r4 = CDB byte with op. flags
        ldob    2(g8),r5                # r5 = Page/Operation code byte
        and     0x03,r4,r4              # mask off reserved bits
        cmpobne 0,r4,.inqd_100          # Jif flag bits set
        cmpobe  0,r5,.inqd_500          # Jif Page/Operation code 0
#
# --- Invalid INQUIRY CDB handler routine
#
.inqd_100:
        ldq     inquiry_tbld2,r4        # load op. values into regs.
        ld      inquiry_tbld2+16,r8
        b       .inqd_1000
#
# --- determine if inquiry CDB is non zero
#
.inqd_500:
        ldos    4(g8),r3                # r3 = alloc. length from CDB
        cmpobne 0,r3,.inqd_510          # Jif alloc. length non-zero

        ldq     inquiry_tbld3,r4        # load op. values into regs.
        ld      inquiry_tbld3+16,r8
        b       .inqd_1000              # and just return status
#
# --- CDB seems ok. Determine if WHQL compliance is active and set parameter accordantly
#
.inqd_510:
        lda     inquiry1_size,g0        # g0 = SGL/buffer combo memory size
        ldconst 02,r5                   # ISO value
        ldob    MAGD_SCSI_WHQL,r4       # is WHQL compliance active??
        cmpobe  0,r4,.inqd_520          # Jif no

        lda     inquiry1_WHQL_size,g0   # g0 = SGL/buffer combo memory size
        ldconst 04,r5                   # ISO value
#
# --- save parameter and allocate buffer
#
.inqd_520:
        mov     g0,r9                   # r9 = my inquiry data size
        stob    r5,inquiry1_ISO         # save iso value
        subo    5,r9,r4                 # calculate new inquiry additional length
        stob    r4,inquiry1_len         # save new length

c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        lda     inquiry1,r4             # r4 = inquiry data base address
        ld      sghdrsiz+sg_addr(g0),g1 # g1 = buffer address
        mov     g1,g2                   # g2 = buffer address
        shro    2,r9,r5                 # r5 = # words to copy
.inqd_590:
        ld      (r4),r6
        lda     4(r4),r4
        subo    1,r5,r5                 # dec. loop count
        st      r6,(g1)
        lda     4(g1),g1
        cmpobne 0,r5,.inqd_590          # Jif more data to copy
#
# --- If associated Virtual Drive does not exist for a LUN that
#       is normally supported, return a 0x20 as device type indicating
#       a disk type of device but that the device is not available
#       now. If associated with a LUN that is not normally supported
#       return a 0x7f indicating the device is not supported.
#
        ldconst 0x20,r6                 # r6 = INQUIRY device type
        ldos    sclun(g7),r5            # r5 = assoc. LUN
        ldconst LUNMAX,r4               # r4 = max. # LUNs supported
        cmpobg  r4,r5,.inqd_600         # Jif assoc. LUN < max. LUN #
        ldconst 0x7f,r6                 # r6 = device type

.inqd_600:
        stob    r6,(g2)                 # indicate LUN not supported
        cmpo    r3,r9                   # check if alloc. length < inquiry size
        sell    r9,r3,r9                # r9 = size of transfer to host
        st      r9,sghdrsiz+sg_len(g0)  # save size of data in SGL
        ldq     inquiry_tbld,r4         # load op. values into regs.
        ld      inquiry_tbld+16,r8
        mov     g0,r6                   # r6 = SGL pointer
.inqd_1000:
        b       mag1$cmdcom             # and finish processing command
#
#******************************************************************************
#
#  NAME: magd$reqsns
#
#  PURPOSE:
#       Processes a Request Sense command received from a host for an
#       undefined LUN.
#
#  DESCRIPTION:
#       Returns Sense data to the issuing host.
#
#  CALLING SEQUENCE:
#       call    magd$reqsns
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = 0 indicating no assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
sense_tbld:
        .byte   dtxferi,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # # SGL descriptors
        .short  0                       # sense length
#
sense_tbld1:
        .byte   dtxferi,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invf1             # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
sense_tbld2:
        .byte   dtxfern,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
magd$reqsns:
        ldob    MAGD_SCSI_WHQL,r5       # is WHQL compliance active??
        cmpobe  0,r5,.rsnsd10           # Jif no
        ldob    1(g8),r6                # r6 = To check the DESC bit
        cmpobe  0,r6,.rsnsd10           # Jif zero value
        ldq     sense_tbld1,r4          # load op. values into regs.
        ld      sense_tbld1+16,r8
        b       .rsnsd1000              # and just return status
#
.rsnsd10:
        ldob    4(g8),r3                # r3 = alloc. length from CDB
        lda     0,g0                    # allocate an SGL
        lda     sensesize,r9            # r9 = my sense data size
        cmpobne 0,r3,.rsnsd100          # Jif alloc. length in CDB non-zero
        ldq     sense_tbld2,r4          # load op. values into regs.
        ld      sense_tbld2+16,r8
        b       .rsnsd1000              # and just return status
#
.rsnsd100:
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        lda     MAGD$sense_nolun,r4     # r4 = inquiry data base address
        st      r4,sghdrsiz+sg_addr(g0) # save sense data address in SGL
        cmpo    r3,r9                   # check if alloc. length < inquiry size
        sell    r9,r3,r9                # r9 = size of transfer to host
        st      r9,sghdrsiz+sg_len(g0)  # save size of data in SGL
        ldq     sense_tbld,r4           # load op. values into regs.
        ld      sense_tbld+16,r8
        mov     g0,r6                   # r6 = SGL pointer
.rsnsd1000:
        b       mag1$cmdcom             # and finish processing command
#
#******************************************************************************
#
#  NAME: magd$repluns
#
#  PURPOSE:
#       Processes REPORT LUNs command received on an unsupported
#       LUN.
#
#  DESCRIPTION:
#       Processes the command normally (as if received on a supported
#       LUN) if received on LUN 0; otherwise rejects the command with
#       LUN NOT SUPPORTED check status sense data.
#
#  CALLING SEQUENCE:
#       call    magd$repluns
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = 0 indicating no assoc. ILMT address
#       g7 = primary ILT address at FCAL nest level
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at inl2 nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
magd$repluns:
        ldos    sclun(g7),r4            # r4 = LUN #
        cmpobe  0,r4,mag1$repluns       # Jif LUN 0 specified
        b       magd$nolun              # Process as any other command on an
                                        #  unsupported LUN
#
#******************************************************************************
#
#  NAME: mag$dabtaskset
#
#  PURPOSE:
#       Default abort task set event handler routine.
#
#  DESCRIPTION:
#       This routine is called by the channel driver when an abort
#       task set is received associated with an IMT but specifying
#       a LUN which is not defined for use by the initiator. The
#       routine ignores the event and performs the necessary
#       processing to complete the event.
#
#  CALLING SEQUENCE:
#       call    mag$dabtaskset
#
#  INPUT:
#       g1 = ILT of event
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = 0 indicating no assoc. ILMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. r3-r15/g6 can be destroyed.
#
#******************************************************************************
#
mag$dabtaskset:
        ret
#
#******************************************************************************
#
#  NAME: mag$dreset
#
#  PURPOSE:
#       Default reset received event handler routine.
#
#  DESCRIPTION:
#       This routine is called by the channel driver when a reset
#       event is received. The channel driver calls the reset
#       received event handler routine associated for each ILMT
#       defined for the IMT before calling the default reset received
#       handler routine. This routine simply does any general processing
#       of the event pertinent to the initiator session.
#
#  CALLING SEQUENCE:
#       call    mag$dreset
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$dreset:
        mov     g6,r15                  # save g6
        lda     im_ilmtdir(g5),r9       # r9 = pointer into ILMT directory
        ldconst LUNMAX,r5               # r5 = # LUNs to process
.dreset30:
        ld      (r9),g6                 # g6 = ILMT address
        subo    1,r5,r5                 # dec. LUN counter
        addo    4,r9,r9                 # inc. to next ILMT in directory
        cmpobe  0,g6,.dreset40          # Jif ILMT not defined
        ld      ilm_ehand(g6),r6        # r6 = default event handler table
        ld      dd_reset(r6),r6         # r6 = reset event handler routine
#
#*******************************************************************************
#
#       Interface to device driver routine:
#       -----------------------------------
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED.
#
#       None.
#
#*******************************************************************************
#
        callx   (r6)                    # and notify device driver of event
.dreset40:
        cmpobne 0,r5,.dreset30          # Jif more LUNs to check for this IMT
        mov     r15,g6                  # restore g6
        ret
#
#******************************************************************************
#
#  NAME: mag$doffline
#
#  PURPOSE:
#       Default interface offline event handler routine.
#
#  DESCRIPTION:
#       This routine is called by the channel driver when an interface
#       offline event occurs for the associated channel interface. This
#       routine does any general cleanup associated with the IMT including
#       processing each ILMT with the offline event as well as
#       re-registering the IMT for use at a later time.
#
#  CALLING SEQUENCE:
#       call    mag$doffline
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       Note: The IMT has to be removed from the active queue before
#               this routine is called!
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
# C access
# void MAG_SrvLogout(CIMT *pCIMT, IMT *pIMT);
        .globl  MAG_SrvLogout           # C access
MAG_SrvLogout:
        mov     g0,g4
        mov     g1,g5
#
mag$doffline:
        mov     g6,r15                  # save g6
        lda     im_ilmtdir(g5),r9       # r9 = pointer into ILMT directory
        ldconst LUNMAX,r5               # r5 = # LUNs to process
.doffl30:
        ld      (r9),g6                 # g6 = ILMT address
        subo    1,r5,r5                 # dec. LUN counter
        addo    4,r9,r9                 # inc. to next ILMT in directory
        cmpobe  0,g6,.doffl40           # Jif ILMT not defined
        ld      ilm_ehand(g6),r6        # r6 = default event handler table
        ld      dd_offline(r6),r6       # r6 = offline event handler routine
#
# -----------------------------------------------------------------------------
#
#       Interface to device driver routine:
#       -----------------------------------
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED.
#
#       None.
#
# -----------------------------------------------------------------------------
#
        callx   (r6)                    # and notify device driver of event
.doffl40:
        cmpobne 0,r5,.doffl30           # Jif more LUNs to check for this IMT
        mov     0,r3
        ld      im_ltmt(g5),r4          # r4 = assoc. LTMT address
        cmpobe  0,r4,.doffl100          # Jif no LTMT assoc. with IMT
        ld      ltmt_imt(r4),r5         # r5 = assoc. IMT for LTMT
        cmpobe  r5,g5,.doffl50          # Jif IMT/LTMT paired up
#
# --- IMT->LTMT orphaned; Just whack LTMT from IMT.
#
        st      r3,im_ltmt(g5)          # clear LTMT from IMT
        b       .doffl100
#
.doffl50:
        ld      ltmt_tmt(r4),r5         # r5 = assoc. TMT address
        cmpobne 0,r5,.doffl100          # Jif LTMT assoc. with TMT
#
# --- If LTMT is not assoc. with a TMT, the IMT/LTMT association can
#       be terminated and the LTMT cleaned up.
#
        movl    g0,r10                  # save g0-g1
        movl    g4,r8                   # save g4-g5
                                        # r9 = IMT address
        mov     r4,g4                   # g4 = LTMT address to clean up
        ldconst 0,g5                    # g5 = null TMT address
        call    LLD$pre_target_gone     # check if OK to terminate LTMT
        cmpobe  TRUE,g0,.doffl70        # Jif OK to terminate LTMT
        movl    0,r4
        stl     r4,im_mac(r9)           # clear MAC address from IMT to
                                        #  render it useless when all ops.
                                        #  have completed
        ld      mag_cleanup_pcb,r4      # check if IMT cleanup process running
        cmpobne 0,r4,.doffl90           # Jif process running
c       g0 = -1;                        # Flag task being started.
        st      g0,mag_cleanup_pcb
        lda     mag$imt_cleanup,g0      # establish IMT cleanup process
        ldconst MAGICPRI,g1
c       CT_fork_tmp = (ulong)"mag$imt_cleanup";
        call    K$fork
        st      g0,mag_cleanup_pcb
        b       .doffl90
#
.doffl70:
        ld      ltmt_ehand(g4),r7       # r7 = LTMT event handler table
        ld      ltmt_eh_tgone(r7),r6    # r6 = LTMT target gone event
                                        #  handler routine
        callx   (r6)                    # and call target gone event
                                        #  handler routine
#
#******************************************************************************
#
# --- Interface to Target Gone event handler routine
#
#   INPUT:
#
#       g4 = assoc. LTMT address
#       g5 = assoc. TMT address (null)
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#******************************************************************************
#
.doffl90:
        movl    r10,g0                  # restore g0-g1
        movl    r8,g4                   # restore g4-g5
.doffl100:
        st      r3,im_inacttmr(g5)      # clear inactive timer field
        call    C$regimt                # put IMT on inactive queue
        mov     r15,g6                  # restore g6
        ret
#
#******************************************************************************
#
#  NAME: mag$donline
#
#  PURPOSE:
#       Default interface online event handler routine.
#
#  DESCRIPTION:
#       This routine is called by the channel driver when an interface
#       online event occurs for the associated channel interface. This
#       routine initializes the IMT and then calls the online event
#       handler routine for each ILMT associated with the IMT. When this
#       routine returns to the caller, the IMT and all associated ILMTs
#       have been initialized and are ready to be placed into service.
#
#  CALLING SEQUENCE:
#       call    mag$donline
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$donline:
        mov     g6,r15                  # save g6
        lda     im_ilmtdir(g5),r9       # r9 = pointer into ILMT directory of
                                        #  IMT
        ldconst LUNMAX,r4               # r4 = max. # LUNs supported
.donline200:
        ld      (r9),g6                 # g6 = ILMT assoc. with IMT
        addo    4,r9,r9                 # inc. to next record in ILMT dir.
        subo    1,r4,r4                 # dec. LUN counter
        cmpobe  0,g6,.donline210        # Jif no ILMT for this LUN
        ld      ilm_ehand(g6),r14       # r14 = ILMT event handler table
        ld      dd_online(r14),r13      # r13 = ILMT online event handler
                                        #  routine
#
#****************************************************************************
#
# --- Interface to ILMT online event handler routine
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED.
#       None.
#
#****************************************************************************
#
        callx   (r13)                   # call ILMT online event handler
                                        #  routine
.donline210:
        cmpobne 0,r4,.donline200        # Jif more ILMTs to check for in IMT
        mov     r15,g6                  # restore g6
        ret
#
#******************************************************************************
#
# ____________ LUN SPECIFIC MAGNITUDE DEVICE DRIVER ROUTINES __________________
#
#******************************************************************************
#
#  NAME: mag$cmdrecv
#
#  PURPOSE:
#       LUN specific SCSI command received event handler routine.
#
#  DESCRIPTION:
#       This routine is called by the channel driver when a SCSI
#       command is received associated with a specific LUN supported
#       by the ILMT. The routine decodes the command and processes
#       it according to the command code and the state of the LUN
#       when received.
#
#  CALLING SEQUENCE:
#       call    mag$cmdrecv
#
#  INPUT:
#       g1 = primary ILT at inl2 nest level
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. r3-r15/g4-g7 can be destroyed.
#
#******************************************************************************
#
mag$cmdrecv:
        movq    g8,r8                   # save g8-g11
        ld      sccdb(g7),g8            # g8 = pointer to CDB
        ldq     (g8),r12                # r12-r15 = CDB
        movq    g0,r4                   # save g0-g3
        lda     inl2_cdb(g1),g8         # g8 = new pointer to CDB
        ldob    ilm_flag1(g6),g9        # g9 = ILMT flag byte #1
        ldob    ilm_flag2(g6),g10       # g10 = ILMT flag byte #2
        stq     r12,(g8)                # save CDB in temp area of ILT
        bbc     0,g9,.cmdr100           # Jif flushing commands flag not set
#
# *** FINISH processing flushing command flag set
#
# --- Flushing command flag not set; proceed with CDB processing
#
.cmdr100:
        call    mag$init_task           # initialize task structure
        cmpobe  0,g9,.cmdr110           # Jif flag byte #1 zero
        bbc     1,g9,.cmdr110           # Jif ACA active flag not set
#
# --- ACA active flag set
#
        ldob    inl2_ttype(g1),r12      # r12 = task type code of new task
        cmpobne inl2_tt_aca,r12,.cmdr101 # Jif not ACA type task
#
# --- ACA type task received
#
        ld      ilm_wtail(g6),r12       # r12 = last task ILT on work queue
        cmpobe  0,r12,.cmdr110          # Jif no tasks on work queue
        ldob    inl2_ttype(r12),r13     # r13 = task type code of last task
                                        #  on work queue
        cmpobne inl2_tt_aca,r13,.cmdr110 # Jif not ACA type task
.cmdr101:
        movt    g12,r12                 # save g12-g14
        mov     g1,g9                   # g9 = pri. ILT at inl2 nest level
        call    mag1$acaactive          # return ACA active status to initiator
        movt    r12,g12                 # restore g12-g14
        b       .cmdr1000               # and we're out of here!
#
.cmdr110:
        ldob    inl2_ttype(g1),r12      # r12 = task type code of new task
        call    mag$qtask2wq            # queue task to working queue
                                        #  in ILMT
        cmpobne inl2_tt_aca,r12,.cmdr111 # Jif not ACA type task
        bbs     1,g9,.cmdr300           # Jif ACA active
#
# --- ACA task received and not ACA active. Return error to initiator.
#
        movt    g12,r12                 # save g12-g14
        mov     g1,g9                   # g9 = pri. ILT at inl2 nest level
        call    mag1$acanotactive       # return error back to initiator
        movt    r12,g12                 # restore g12-g14
        b       .cmdr1000               # and we're out of here!
#
.cmdr111:
        cmpobe  TRUE,g0,.cmdr300        # Jif task placed on empty queue
        cmpobne 0,g9,.cmdr1000          # Jif flag byte #1 non-zero
        cmpobne 0,g10,.cmdr1000         # Jif flag byte #2 non-zero
        ld      il_bthd(g1),g10         # g10 = ILT address of previous
                                        #  task
        ldob    inl2_ttype(g1),g9       # g9 = task type code of new task
        ldob    inl2_ttype(g10),r12     # r12 = task type of previous task
        ld      inl2_ehand(g10),r3      # r3 = previous task event handler
                                        #  table
#
# --- Not mixed Untagged/Tagged tasks
#
        cmpobe  inl2_tt_ord,g9,.cmdr1000 # Jif ordered type task since we
                                        #  must wait for all older tasks to
                                        #  complete before processing an
                                        #  ordered task
        ldob    inl2_eh_tstate(r3),r13  # r13 = task state code of previous
                                        #  task
        cmpobne inl2_tt_sim,g9,.cmdr300 # Jif not a simple task type
                                        # Must be either an ACA or head-of-
                                        #  queue task.
        cmpobne inl2_tt_sim,r12,.cmdr1000 # Jif previous task not simple
                                        #  task type
        cmpobne inl2_ts_enbl,r13,.cmdr1000 # Jif previous task is not
                                        #  enabled
#
# --- Note: The only task types left are head-of-queue and ACA task types
#       which continue processing.
#
# --- Continue processing task
#
.cmdr300:
        ldob    ilm_flag3(g6),g11       # g11 = ILMT flag byte #3
        bbs     7,g11,.cmdr1000         # Jif PR config retrieval in progress
#
        movt    g12,r12                 # save g12-g14
        call    mag$enable_task         # enable task
        movt    r12,g12                 # restore g12-g14
.cmdr1000:
        movq    r4,g0                   # restore g0-g3
        movq    r8,g8                   # restore g8-g11
        ret
#
# --- MAGNITUDE command normalization table #1
#
# --- Normal operation normalization table
#
        .data
normtbl1:
        .byte   1,0,0,9,15,0,0,0        # 00-07
        .byte   10,0,11,0,0,0,0,0       # 08-0f
        .byte   0,0,2,0,0,12,13,14      # 10-17
        .byte   0,0,6,23,0,17,0,0       # 18-1f
        .byte   0,0,0,0,0,3,0,0         # 20-27
        .byte   4,0,5,0,0,0,16,8        # 28-2f
        .byte   0,0,0,0,0,26,0,0        # 30-37
        .byte   0,0,0,0,0,0,0,0         # 38-3f
        .byte   0,0,0,0,0,0,0,0         # 40-47
        .byte   0,0,0,0,0,0,0,0         # 48-4f
        .byte   0,0,0,0,0,25,21,22      # 50-57
        .byte   0,0,24,0,0,0,27,28      # 58-5f
        .byte   0,0,0,0,0,0,0,0         # 60-67
        .byte   0,0,0,0,0,0,0,0         # 68-6f
        .byte   0,0,0,0,0,0,0,0         # 70-77
        .byte   0,0,0,0,0,0,0,0         # 78-7f
        .byte   0,0,0,0,0,0,0,0         # 80-87
        .byte   30,0,31,0,0,0,33,32     # 88-8f
        .byte   0,0,0,0,0,0,0,0         # 90-97
        .byte   0,0,0,0,0,0,29,0        # 98-9f
        .byte   7,0,0,0,0,0,0,0         # a0-a7
        .byte   0,0,0,0,0,0,0,0         # a8-af
        .byte   0,0,0,0,0,0,0,0         # b0-b7
        .byte   0,0,0,0,0,0,0,0         # b8-bf
        .byte   0,0,0,0,0,0,0,0         # c0-c7
        .byte   0,0,0,0,0,0,0,0         # c8-cf
        .byte   0,0,0,0,0,0,0,0         # d0-d7
        .byte   0,0,0,0,0,0,0,0         # d8-df
        .byte   0,0,0,0,0,0,0,0         # e0-e7
        .byte   0,0,0,0,0,0,0,0         # e8-ef
        .byte   0,0,0,0,0,0,0,0         # f0-f7
        .byte   0,0,0,0,0,0,0,0         # f8-ff
#
# --- MAGNITUDE command handler table #1
#
# --- Normal operation command handler table
#
cmdtbl1:
        .word   mag1$undef      #  0 - undefined command
        .word   mag1$tur        #  1 - Test unit ready          0x00
        .word   mag1$inquiry    #  2 - Inquiry                  0x12
        .word   mag1$readcap    #  3 - Read capacity            0x25
        .word   mag1$read10     #  4 - Read (10) extended       0x28  -> 88
        .word   mag1$write10    #  5 - Write (10) extended      0x2A  -> 8A
        .word   mag1$modesns    #  6 - Mode sense (6)           0x1A
        .word   mag1$repluns    #  7 - Report LUNs              0xA0
        .word   mag1$vfymedia   #  8 - Verify media             0x2F  -> 8F
        .word   mag1$reqsns     #  9 - Request sense            0x03
        .word   mag1$read6      # 10 - Read (6)                 0x08
        .word   mag1$write6     # 11 - Write (6)                0x0A
        .word   mag1$modesel    # 12 - Mode select (6)          0x15
        .word   mag1$reserve6   # 13 - Reserve (6)              0x16
        .word   mag1$release6   # 14 - Release (6)              0x17
        .word   mag1$fmtunit    # 15 - Format unit              0x04
        .word   mag1$writevfy   # 16 - Write & verify           0x2E  -> 8E
        .word   mag1$snddiag    # 17 - Send diagnostics         0x1D
        .word   mag1$rcvdiag    # 18 - Receive diagnostics      Not in above table
        .word   mag1$logsns     # 19 - Log sense                Not in above table
        .word   mag1$logsel     # 20 - Log select               Not in above table
        .word   mag1$reserve10  # 21 - Reserve (10)             0x56
        .word   mag1$release10  # 22 - Release (10)             0x57
        .word   mag1$startstop  # 23 - Start Stop unit          0x1B
        .word   mag1$modesns10  # 24 - Mode sense (10)          0x5A
        .word   mag1$modesel10  # 25 - Mode select (10)         0x55
        .word   mag1$synccache  # 26 - Sync Cache               0x35
        .word   mag1$presv_in   # 27 - Persistent Reserve In    0x5E
        .word   mag1$presv_out  # 28 - Persistent Reserve Out   0x5F
        .word   mag1$readcap_16 # 29 - Read capacity (16)       0x9E
        .word   mag1$read_16    # 30 - Read (16)                0x88
        .word   mag1$write_16   # 31 - Write (16)               0x8A
        .word   mag1$verify_16  # 32 - Verify (16)              0x8F
        .word   mag1$writevfy_16 #33 - Write & Verify (16)      0x8E
#
# --- MAGNITUDE command handler table #2
#
# --- Device reserved by other initiator command handler table
#
cmdtbl2:
        .word   mag1$undef      #  0 - undefined command
        .word   mag2$rconflict  #  1 - Test unit ready
        .word   mag1$inquiry    #  2 - Inquiry
        .word   mag2$rconflict  #  3 - Read capacity
        .word   mag2$rconflict  #  4 - Read (10) extended
        .word   mag2$rconflict  #  5 - Write (10) extended
        .word   mag2$rconflict  #  6 - Mode sense (6)
        .word   mag1$repluns    #  7 - Report LUNs
        .word   mag2$rconflict  #  8 - Verify media
        .word   mag1$reqsns     #  9 - Request sense
        .word   mag2$rconflict  # 10 - Read (6)
        .word   mag2$rconflict  # 11 - Write (6)
        .word   mag2$rconflict  # 12 - Mode select (6)
        .word   mag2$rconflict  # 13 - Reserve (6)
        .word   mag1$release6   # 14 - Release (6)
        .word   mag2$rconflict  # 15 - Format unit
        .word   mag2$rconflict  # 16 - Write & verify
        .word   mag2$rconflict  # 17 - Send diagnostics
        .word   mag2$rconflict  # 18 - Receive diagnostics
        .word   mag2$rconflict  # 19 - Log sense
        .word   mag2$rconflict  # 20 - Log select
        .word   mag2$rconflict  # 21 - Reserve (10)
        .word   mag1$release10  # 22 - Release (10)
        .word   mag2$rconflict  # 23 - Start Stop unit
        .word   mag2$rconflict  # 24 - Mode sense (10)
        .word   mag2$rconflict  # 25 - Mode select (10)
        .word   mag2$rconflict  # 26 - Sync Cache
        .word   mag2$rconflict  # 27 - Persistent Reserve In
        .word   mag2$rconflict  # 28 - Persistent Reserve Out
        .word   mag2$rconflict  # 29 - Read capacity (16)
        .word   mag2$rconflict  # 30 - Read (16)
        .word   mag2$rconflict  # 31 - Write (16)
        .word   mag2$rconflict  # 32 - Verify (16)              0x8F
        .word   mag2$rconflict  # 33 - Write & Verify (16)      0x8E
#
# --- MAGNITUDE command handler table #3
#
# --- Device reserved by other initiator using Persistent Reservation
#     (exclusive access)
#
cmdtbl3:
        .word   mag1$undef      #  0 - undefined command
        .word   mag2$rconflict  #  1 - Test unit ready
        .word   mag1$inquiry    #  2 - Inquiry
        .word   mag1$readcap    #  3 - Read capacity
        .word   mag2$rconflict  #  4 - Read (10) extended
        .word   mag2$rconflict  #  5 - Write (10) extended
        .word   mag2$rconflict  #  6 - Mode sense (6)
        .word   mag1$repluns    #  7 - Report LUNs
        .word   mag2$rconflict  #  8 - Verify media
        .word   mag1$reqsns     #  9 - Request sense
        .word   mag2$rconflict  # 10 - Read (6)
        .word   mag2$rconflict  # 11 - Write (6)
        .word   mag2$rconflict  # 12 - Mode select (6)
        .word   mag2$rconflict  # 13 - Reserve (6)
        .word   mag2$rconflict  # 14 - Release (6)
        .word   mag2$rconflict  # 15 - Format unit
        .word   mag2$rconflict  # 16 - Write & verify
        .word   mag2$rconflict  # 17 - Send diagnostics
        .word   mag2$rconflict  # 18 - Receive diagnostics
        .word   mag1$logsns     # 19 - Log sense
        .word   mag2$rconflict  # 20 - Log select
        .word   mag2$rconflict  # 21 - Reserve (10)
        .word   mag2$rconflict  # 22 - Release (10)
        .word   mag1$startstop  # 23 - Start Stop unit
        .word   mag2$rconflict  # 24 - Mode sense (10)
        .word   mag2$rconflict  # 25 - Mode select (10)
        .word   mag2$rconflict  # 26 - Sync Cache
        .word   mag1$presv_in   # 27 - Persistent Reserve In
        .word   mag1$presv_out  # 28 - Persistent Reserve Out
        .word   mag1$readcap_16 # 29 - Read capacity (16)
        .word   mag2$rconflict  # 30 - Read (16)
        .word   mag2$rconflict  # 31 - Write (16)
        .word   mag2$rconflict  # 32 - Verify (16)              0x8F
        .word   mag2$rconflict  # 33 - Write & Verify (16)      0x8E
#
# --- MAGNITUDE command handler table #4
#
# --- Device reserved by other initiator using Persistent Reservation
#     (write exclusive)
cmdtbl4:
        .word   mag1$undef      #  0 - undefined command
        .word   mag2$rconflict  #  1 - Test unit ready
        .word   mag1$inquiry    #  2 - Inquiry
        .word   mag1$readcap    #  3 - Read capacity
        .word   mag1$read10     #  4 - Read (10) extended
        .word   mag2$rconflict  #  5 - Write (10) extended
        .word   mag2$rconflict  #  6 - Mode sense (6)
        .word   mag1$repluns    #  7 - Report LUNs
        .word   mag1$vfymedia   #  8 - Verify media
        .word   mag1$reqsns     #  9 - Request sense
        .word   mag1$read6      # 10 - Read (6)
        .word   mag2$rconflict  # 11 - Write (6)
        .word   mag2$rconflict  # 12 - Mode select (6)
        .word   mag2$rconflict  # 13 - Reserve (6)
        .word   mag2$rconflict  # 14 - Release (6)
        .word   mag2$rconflict  # 15 - Format unit
        .word   mag2$rconflict  # 16 - Write & verify
        .word   mag2$rconflict  # 17 - Send diagnostics
        .word   mag2$rconflict  # 18 - Receive diagnostics
        .word   mag1$logsns     # 19 - Log sense
        .word   mag2$rconflict  # 20 - Log select
        .word   mag2$rconflict  # 21 - Reserve (10)
        .word   mag2$rconflict  # 22 - Release (10)
        .word   mag1$startstop  # 23 - Start Stop unit
        .word   mag2$rconflict  # 24 - Mode sense (10)
        .word   mag2$rconflict  # 25 - Mode select (10)
        .word   mag2$rconflict  # 26 - Sync Cache
        .word   mag1$presv_in   # 27 - Persistent Reserve In
        .word   mag1$presv_out  # 28 - Persistent Reserve Out
        .word   mag1$readcap_16 # 29 - Read capacity (16)
        .word   mag1$read_16    # 30 - Read (16)
        .word   mag2$rconflict  # 31 - Write (16)
        .word   mag1$verify_16  # 32 - Verify (16)              0x8F
        .word   mag1$writevfy_16 #33 - Write & Verify (16)      0x8E
#
# --- MAGNITUDE command handler table #5
#
# --- Device reserved by other initiator using Persistent Reservation
#     (write exclusive - Registrants only) and this initiator has not
#     registered a key.
#
cmdtbl5:
        .word   mag1$undef      #  0 - undefined command
        .word   mag2$rconflict  #  1 - Test unit ready
        .word   mag1$inquiry    #  2 - Inquiry
        .word   mag1$readcap    #  3 - Read capacity
        .word   mag1$read10     #  4 - Read (10) extended
        .word   mag2$rconflict  #  5 - Write (10) extended
        .word   mag2$rconflict  #  6 - Mode sense (6)
        .word   mag1$repluns    #  7 - Report LUNs
        .word   mag1$vfymedia   #  8 - Verify media
        .word   mag1$reqsns     #  9 - Request sense
        .word   mag1$read6      # 10 - Read (6)
        .word   mag2$rconflict  # 11 - Write (6)
        .word   mag2$rconflict  # 12 - Mode select (6)
        .word   mag2$rconflict  # 13 - Reserve (6)
        .word   mag2$rconflict  # 14 - Release (6)
        .word   mag2$rconflict  # 15 - Format unit
        .word   mag2$rconflict  # 16 - Write & verify
        .word   mag2$rconflict  # 17 - Send diagnostics
        .word   mag2$rconflict  # 18 - Receive diagnostics
        .word   mag1$logsns     # 19 - Log sense
        .word   mag2$rconflict  # 20 - Log select
        .word   mag2$rconflict  # 21 - Reserve (10)
        .word   mag2$rconflict  # 22 - Release (10)
        .word   mag1$startstop  # 23 - Start Stop unit
        .word   mag2$rconflict  # 24 - Mode sense (10)
        .word   mag2$rconflict  # 25 - Mode select (10)
        .word   mag2$rconflict  # 26 - Sync Cache
        .word   mag1$presv_in   # 27 - Persistent Reserve In
        .word   mag1$presv_out  # 28 - Persistent Reserve Out
        .word   mag1$readcap_16 # 29 - Read capacity (16)
        .word   mag1$read_16    # 30 - Read (16)
        .word   mag2$rconflict  # 31 - Write (16)
        .word   mag1$verify_16  # 32 - Verify (16)              0x8F
        .word   mag1$writevfy_16 #33 - Write & Verify (16)      0x8E
#
# --- MAGNITUDE command handler table #6
#
# --- Device reserved by other initiator using Persistent Reservation
#     (exclusive access - Registrants only) and this initiator has
#     not registered a key
cmdtbl6:
        .word   mag1$undef      #  0 - undefined command
        .word   mag2$rconflict  #  1 - Test unit ready
        .word   mag1$inquiry    #  2 - Inquiry
        .word   mag2$rconflict  #  3 - Read capacity
        .word   mag2$rconflict  #  4 - Read (10) extended
        .word   mag2$rconflict  #  5 - Write (10) extended
        .word   mag2$rconflict  #  6 - Mode sense (6)
        .word   mag1$repluns    #  7 - Report LUNs
        .word   mag2$rconflict  #  8 - Verify media
        .word   mag1$reqsns     #  9 - Request sense
        .word   mag2$rconflict  # 10 - Read (6)
        .word   mag2$rconflict  # 11 - Write (6)
        .word   mag2$rconflict  # 12 - Mode select (6)
        .word   mag2$rconflict  # 13 - Reserve (6)
        .word   mag2$rconflict  # 14 - Release (6)
        .word   mag2$rconflict  # 15 - Format unit
        .word   mag2$rconflict  # 16 - Write & verify
        .word   mag2$rconflict  # 17 - Send diagnostics
        .word   mag2$rconflict  # 18 - Receive diagnostics
        .word   mag1$logsns     # 19 - Log sense
        .word   mag2$rconflict  # 20 - Log select
        .word   mag2$rconflict  # 21 - Reserve (10)
        .word   mag2$rconflict  # 22 - Release (10)
        .word   mag1$startstop  # 23 - Start Stop unit
        .word   mag2$rconflict  # 24 - Mode sense (10)
        .word   mag2$rconflict  # 25 - Mode select (10)
        .word   mag2$rconflict  # 26 - Sync Cache
        .word   mag1$presv_in   # 27 - Persistent Reserve In
        .word   mag1$presv_out  # 28 - Persistent Reserve Out
        .word   mag1$readcap_16 # 29 - Read capacity (16)
        .word   mag2$rconflict  # 30 - Read (16)
        .word   mag2$rconflict  # 31 - Write (16)
        .word   mag2$rconflict  # 32 - Verify (16)              0x8F
        .word   mag2$rconflict  # 33 - Write & Verify (16)      0x8E
#
        .text
#
#******************************************************************************
#
#  NAME: mag$abtask
#
#  PURPOSE:
#       LUN specific abort task event handler routine.
#
#  DESCRIPTION:
#       This routine is called by the channel driver when an abort
#       task is received associated with a specific IMT.
#       The routine locates the LUN/ILMT associated with the specified
#       exchange ID and does the appropriate processing to abort it if
#       found.
#
#  CALLING SEQUENCE:
#       call    mag$abtask
#
#  INPUT:
#       g1 = ILT of event at inl2 nest level
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g7 = ILT of event at inl1 nest level
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. r3-r15/g6 can be destroyed.
#
#******************************************************************************
#
mag$abtask:
        mov     g0,r15                  # save g0
        mov     g6,r14                  # save g6
        ld      idf_exid(g7),g0         # g0 = exchange ID
        lda     im_ilmtdir(g5),r13      # r13 = pointer to ILMT directory in IMT
        ldconst LUNMAX,r12              # r12 = max. # ILMTs to check
.abtask_20:
        ld      (r13),g6                # g6 = ILMT in directory
        cmpobe  0,g6,.abtask_50         # Jif no ILMT at this location
        ld      ilm_whead(g6),r11       # r11 = first task on work queue
        cmpobe  0,r11,.abtask_50        # Jif no tasks on work queue
.abtask_30:
        ld      inl2_FCAL(r11),r10      # r10 = pri. ILT of task
        ld      idf_exid(r10),r10       # r10 = exchange ID for task
        cmpobe  g0,r10,.abtask_100      # Jif exchange ID found in task set
                                        #  on work queue
        ld      il_fthd(r11),r11        # r11 = next task on work list
        cmpobne 0,r11,.abtask_30        # Jif more tasks to check on work list
.abtask_50:
        addo    4,r13,r13               # inc. to next ILMT in directory
        subo    1,r12,r12               # dec. ILMT count
        cmpobne 0,r12,.abtask_20        # Jif more ILMTs to check
        b       .abtask_1000            # Can't find exchange ID in all ILMT
                                        #  working task sets. Just return to
                                        #  caller.
.abtask_100:
        call    mag$abort_task          # abort specified task
.abtask_1000:
        mov     r14,g6                  # restore g6
        mov     r15,g0                  # restore g0
        ret
#
#******************************************************************************
#
#  NAME: mag$abtaskset
#
#  PURPOSE:
#       LUN specific abort task set event handler routine.
#
#  DESCRIPTION:
#       This routine is called by the channel driver when an abort
#       task set is received associated with a specific LUN related
#       to the specified IMTr. The routine performs the necessary
#       processing to abort the task set currently in operation.
#
#  CALLING SEQUENCE:
#       call    mag$abtaskset
#
#  INPUT:
#       g1 = ILT of event
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. r3-r15/g6 can be destroyed.
#
#******************************************************************************
#
mag$abtaskset:
        call    mag$abort_ilmt_tasks    # abort all ILMT tasks
        ret
#
#******************************************************************************
#
#  NAME: mag$reset
#
#  PURPOSE:
#       LUN specific reset received event handler routine.
#
#  DESCRIPTION:
#       This routine is called by the channel driver when a reset
#       event is received. The channel driver calls the reset
#       received event handler routine associated for each ILMT
#       defined for the IMT before calling the default reset received
#       handler routine. This routine does the appropriate processing
#       to clean up the ILMT and associated operations appropriate to
#       a reset event.
#
#  CALLING SEQUENCE:
#       call    mag$reset
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$reset:
        mov     g3,r14                  # save g3
        ld      ilm_vdmt(g6),g3         # g3 = assoc. VDMT
        ldob    ilm_flag2(g6),r6        # r6 = ILMT flag byte #2
        call    mag$reset_ilmt_tasks    # abort all ILMT tasks
        bbs     0,r6,.reset100          # Jif poweron reset notification still
                                        #  pending
        setbit  1,r6,r6                 # set flag indicating SCSI bus reset
                                        #  received
        stob    r6,ilm_flag2(g6)        # save updated flag byte
.reset100:
# explicitly clear ACA
        ldob    ilm_flag1(g6),r6        # r6 = ILMT flag byte #1
        clrbit  1,r6,r6                 # clear ACA active flag
        stob    r6,ilm_flag1(g6)        # save updated flag byte #1
# c fprintf(stderr, "%s%s:%u mag$reset: g4=%08lx, g5=%08lx, g6=%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g4, g5, g6);

        cmpobe  0,g3,.reset1000         #  Jif no VDMT was setup
#        ld      vdm_rilmt(g3),r4        # r4 = ILMT address of reserving
#                                        #  initiator
#        cmpobne r4,g6,.reset1000        # Jif not this initiator
        mov     g9,r15                  # save g9
        mov     0,g9                    # g9 = 0 indicating local device
                                        #  release is not associated with
                                        #  a task
        call    mag$locrelease          # release device from this initiator
        mov     r15,g9                  # restore g9
.reset1000:
        mov     r14,g3                  # restore g3
        ret
#
#******************************************************************************
#
#  NAME: mag$offline
#
#  PURPOSE:
#       LUN specific interface offline event handler routine.
#
#  DESCRIPTION:
#       This routine is called by the channel driver when an interface
#       offline event occurs for the associated channel interface. This
#       routine is called to perform the LUN specific processing for this
#       event and before the general purpose processing performed by the
#       default interface offline event handler routine.
#
#  CALLING SEQUENCE:
#       call    mag$offline
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#******************************************************************************
#
mag$offline:
        mov     g1,r15                  # save g1
.offl40:
        ld      ilm_whead(g6),g1        # g1 = task ILT address
        cmpobe  0,g1,.offl50            # Jif no task ILTs on work queue
        ld      inl2_ehand(g1),r4       # r4 = task event handler table
        ld      inl2_eh_offline(r4),r5  # r5 = task abort event handler
                                        #  routine
        call    mag$remtask             # remove task ILT from work queue
        callx   (r5)                    # call task's abort event handler
                                        #  routine
        b       .offl40                 # and check for more tasks to process
#
.offl50:
#
# --- Check if device reserve needs to be terminated
#
        mov     g3,r14                  # save g3
        ld      ilm_vdmt(g6),g3         # g3 = assoc. VDMT
        cmpobe  0,g3,.offl200           # Jif no associated VDMT
        ld      vdm_rilmt(g3),r4        # r4 = reserving ILMT of device
        cmpobne r4,g6,.offl200          # Jif this ILMT not reserving device
        mov     g9,r13                  # save g9
        mov     0,g9                    # g9 = 0 indicating release not
                                        #  associated with task
        call    mag$locrelease          # release reserve from device
        mov     r13,g9                  # restore g9
.offl200:
        mov     r14,g3                  # restore g3
#
# *** FINISH - Clear ilm_bhead/btail queue
#               Clear ilm_snshead/snstail queue
#
        mov     r15,g1                  # restore g1
        ret
#
#******************************************************************************
#
#  NAME: mag$online
#
#  PURPOSE:
#       LUN specific interface online event handler routine.
#
#  DESCRIPTION:
#       This routine is called by the IMT online event handler routine when
#       an interface online event occurs for the associated channel interface.
#       This routine is called to perform the LUN specific processing for this
#       event. It initializes the working environment table as well as other
#       ILMT fields needed when activating an ILMT.
#
#  CALLING SEQUENCE:
#       call    mag$online
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$online:
#
# --- Initialize working environment table
#
        ld      ilm_vdmt(g6),r10        # r10 = assoc. VDMT address
        ld      ilm_dfenv(g6),r12       # r12 = default environment table
        ld      ilm_wkenv(g6),r13       # r13 = working environment table
        ldconst mscopy_sz,r4            # r4 = # words to copy from default
                                        #  environment table to working
                                        #  environment table
        mov     r13,r14                 # set up to copy default table to
                                        #  working table
#
# NOTE! - Mode Pages Block size only allows 3 bytes of Device Cap
#
        cmpobne 0,r10,.onl50            # Jif a VDMT exists
        movl    1,r8                    # Show an invalid Capacity (zero results
                                        #  in DLM doing a Read Capacity for
                                        #  DLM communications which gets
                                        #  rejected)  (MAGNITUDE loads
                                        #  vdm_devcap even when no VDMT exists
                                        #  - resulting in garbage).
        b       .onl100                 # continue processing
#
.onl50:
        ldl     vdm_devcap(r10),r8      # r8,r9 = VD device capacity
#
.onl100:
        ld      (r12),r15               # r15 = word to copy
        addo    4,r12,r12               # inc. to next word in default table
        subo    1,r4,r4                 # dec. word counter
        st      r15,(r13)               # save word
        addo    4,r13,r13               # inc. to next word in working table
        cmpobne 0,r4,.onl100            # Jif more words to copy
#
c       r15 = r8;                       # If too big, make as big as possible.
c       if (r9 != 0 || (r15 & 0xff000000) != 0) {
c           r15 = 0x00ffffff;
# c fprintf(stderr, ".onl100 vdisk size too big for 3 bytes (0x%lx %lx) [%llu]\n", r9, r8, *(UINT64*)&r8);
c       }                               # Top byte is "density".
        bswap   r15,r15                 # drive capacity in big endian format
        st      r15,4(r14)              # save device capacity in MODE SENSE
#
c       *(UINT64*)&r8 = *(UINT64*)&r8 / 2000ULL;    # Convert capacity into # of heads
c       if (r9 != 0 || (r8 & 0xff000000) != 0) {
c           r8 = 0x00ffffff;
# c fprintf(stderr, ".onl100 number heads too big for 3 bytes (0x%lx %lx) [%llu]\n", r9, r8, *(UINT64*)&r8);
c       }
c       r8 = (r8 << 8) | 0x14;          # Logical or in # of heads to # cylinders.
        bswap   r8,r8                   # make into big endian
        st      r8,mspg4_cylnum(r14)    # save new # cylinders in env. table
# NOTE! - end of mode page only supports 3 bytes of capacity.
#
# --- Complete ILMT initialization
#
        mov     0,r15                   # r15 = 0
        ldconst 1,r14                   # r14 = ilm_flag2 starting value
        st      g4,ilm_cimt(g6)         # save assoc. CIMT address in ILMT
        stob    r15,ilm_flag1(g6)       # clear flag1 byte
        stob    r14,ilm_flag2(g6)       # indicate power-on reset occurred
                                        #  pending SENSE data
        ret
#
#******************************************************************************
#
#  NAME: mag$clearaca
#
#  PURPOSE:
#       LUN specific Clear ACA event handler routine.
#
#  DESCRIPTION:
#       This routine is called by the Channel Driver when a
#       Clear ACA task management function is received from
#       an initiator for a specific defined ILMT. This routine
#       checks if the associated ILMT is in an ACA active state
#       and if so clears the ACA active flag in the ILMT and
#       calls the ACA cleared event handler routine for any
#       tasks residing on the work queue.
#
#  CALLING SEQUENCE:
#       call    mag$clearaca
#
#  INPUT:
#       g1 = ILT address
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = ILT param structure
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$clearaca:
        ldob    ilm_flag1(g6),r14       # r14 = ILMT flag byte #1
        bbc     1,r14,.clraca_1000      # Jif ACA active flag not set
        mov     g1,r15                  # save g1
        clrbit  1,r14,r14               # clear ACA active flag
        stob    r14,ilm_flag1(g6)       # save updated flag byte #1
        ld      ilm_whead(g6),g1        # g1 = task ILTs on work queue
        cmpobe  0,g1,.clraca_900        # Jif no task ILTs on work queue
.clraca_100:
        ld      inl2_ehand(g1),r13      # r13 = task's event handler table
        ld      il_fthd(g1),r14         # r14 = next task ILT on queue
        ld      inl2_eh_acaclr(r13),r12 # r12 = ACA clear event handler routine
#
#****************************************************************************
#
# --- Interface to Clear ACA event handler routine:
#
#  INPUT:
#       g1 = pri. ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#*****************************************************************************
#
        callx   (r12)                   # call task's ACA clear event handler
                                        #  routine
        mov     r14,g1                  # g1 = next task ILT on queue
        cmpobne 0,g1,.clraca_100        # Jif more task ILTs to process
.clraca_900:
        mov     r15,g1                  # restore g1
.clraca_1000:
        ret
#
#******************************************************************************
#
#  NAME: magp$cmdrecv
#
#  PURPOSE:
#       Default SCSI command received event handler routine while
#       an image is pending.
#
#  DESCRIPTION:
#       This routine is called by the channel driver when a SCSI
#       command is received for an image that is pending. This
#       routine queues the associated task ILT to the IMT (im_pendtask)
#       and returns to the caller. The tasks on the im_pendtask
#       queue will be processed once the zoning inquiry VRP request
#       is returned and processed for this image.
#
#  CALLING SEQUENCE:
#       call    magp$cmdrecv
#
#  INPUT:
#       g1 = pri. ILT at inl2 nest level
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = 0 indicating no assoc. ILMT
#       g7 = assoc. ILT param. structure
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. r3-r15/g4-g7 can be destroyed.
#
#******************************************************************************
#
magp$cmdrecv:
        st      g6,il_fthd(g1)          # clear forward thread field in ILT
        ld      im_pendtask(g5),r4      # r4 = first task/ILT on list
        cmpobne 0,r4,.magpcr_100        # Jif list not empty
        st      g1,im_pendtask(g5)      # save task/ILT as head of list
        b       .magpcr_1000            # and we're out of here!
#
.magpcr_100:
        mov     r4,r5                   # r5 = last ILT on list
        ld      il_fthd(r4),r4          # r4 = next task/ILT on list
        cmpobne 0,r4,.magpcr_100        # Jif more task/ILTs on list
        st      g1,il_fthd(r5)          # link new task/ILT onto end of list
.magpcr_1000:
        ret
#
#******************************************************************************
#
#  NAME: magp$abtask
#
#  PURPOSE:
#       Default abort task event handler routine for a pending image.
#
#  DESCRIPTION:
#       This routine is called by the channel driver when an abort
#       task is received associated with a specific IMT while the IMT
#       is pending. This routine locates the associated task/ILT on the
#       im_pendtask queue and if found removes it and returns it to
#       the original caller's completion handler routine.
#
#  CALLING SEQUENCE:
#       call    magp$abtask
#
#  INPUT:
#       g1 = ILT of event
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = 0 indicating no assoc. ILMT
#       g7 = assoc. ILT param. structure
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. r3-r15/g6 can be destroyed.
#
#******************************************************************************
#
magp$abtask:
        PushRegs(r3)                    # Save all G registers
        ld      idf_exid(g7),g0         # g0 = exchange ID
        ld      im_pendtask(g5),r11     # r11 = first task/ILT on list
        cmpobe  0,r11,.magpabtask_1000  # Jif no task/ILTs on list
        mov     0,r12                   # r12 = previous task/ILT on list
.magpabtask_30:
        ld      inl2_FCAL(r11),r10      # r10 = pri. ILT of task
        ld      idf_exid(r10),r10       # r10 = exchange ID for task
        cmpobe  g0,r10,.magpabtask_100  # Jif exchange ID found in task set
                                        #  on pending task queue
        mov     r11,r12                 # r12 = last task/ILT on list
        ld      il_fthd(r11),r11        # r11 = next task on list
        cmpobne 0,r11,.magpabtask_30    # Jif more tasks to check on list
        b       .magpabtask_1000        # can't find task on list. Just return
                                        #  to caller.
.magpabtask_100:
        cmpobne 0,r12,.magpabtask_200   # Jif task/ILT not the first on list
        ld      il_fthd(r11),r12        # r12 = next task/ILT on list
        st      r12,im_pendtask(g5)     # save new list head pointer
        b       .magpabtask_300         # and return task/ILT to originator's
                                        #  completion handler routine.
.magpabtask_200:
        ld      il_fthd(r11),r10        # r10 = next task/ILT on list
        st      r10,il_fthd(r12)        # remove task/ILT from list
.magpabtask_300:
        lda     -ILTBIAS(r11),g1        # g1 = task/ILT at nest level #1
        ld      il_cr(g1),r4            # r4 = completion handler routine
        callx   (r4)                    # call completion handler routine
.magpabtask_1000:
        PopRegsVoid(r3)                 # Restore all G registers
        ret
#
#******************************************************************************
#
#  NAME: magp$abtaskset
#
#  PURPOSE:
#       Default abort task set event handler routine for a pending image.
#
#  DESCRIPTION:
#       This routine is called by the channel driver when an abort
#       task set is received associated with a pending image.
#       This routine removes all tasks on the im_pendtask queue
#       and returns them to the originator's completion handler
#       routine.
#
#  CALLING SEQUENCE:
#       call    magp$abtaskset
#
#  INPUT:
#       g1 = ILT of event at inl2 nest level
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g7 = ILT of event at inl1 nest level
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. r3-r15/g6 can be destroyed.
#
#******************************************************************************
#
magp$abtaskset:
        PushRegs(r3)                    # Save all G registers
        ldos    idf_lun(g7),r5          # r5 = assoc. LUN
        ld      im_pendtask(g5),r11     # r11 = first task/ILT on list
        cmpobe  0,r11,.magpabtaskset_1000 # Jif no task/ILTs on list
        mov     0,r12                   # r12 = previous task/ILT on list
.magpabtaskset_30:
        ld      inl2_FCAL(r11),r10      # r10 = pri. ILT of task
        ldos    idf_lun(r10),r10        # r10 = LUN for task
        cmpobe  r5,r10,.magpabtaskset_100 # Jif LUN match found in task set
                                        #  on pending task queue
        mov     r11,r12                 # r12 = last task/ILT on list
        ld      il_fthd(r11),r11        # r11 = next task on list
        cmpobne 0,r11,.magpabtaskset_30 # Jif more tasks to check on list
        b       .magpabtaskset_1000     # can't find task on list. Just return
                                        #  to caller.
.magpabtaskset_100:
        cmpobne 0,r12,.magpabtaskset_200 # Jif task/ILT not the first on list
        ld      il_fthd(r11),r10        # r10 = next task/ILT on list
        st      r10,im_pendtask(g5)     # save new list head pointer
        b       .magpabtaskset_300      # and return task/ILT to originator's
                                        #  completion handler routine.
.magpabtaskset_200:
        ld      il_fthd(r11),r10        # r10 = next task/ILT on list
        st      r10,il_fthd(r12)        # remove task/ILT from list
.magpabtaskset_300:
        lda     -ILTBIAS(r11),g1        # g1 = task/ILT at nest level #1
        ld      il_cr(g1),r4            # r4 = completion handler routine
        callx   (r4)                    # call completion handler routine
        ldq     16(r3),g4               # restore g4-g7
        mov     r10,r11                 # r11 = next task/ILT on list
        cmpobne 0,r11,.magpabtaskset_30 # Jif more task/ILTs to check
.magpabtaskset_1000:
        PopRegsVoid(r3)                 # Restore all G registers
        ret
#
#******************************************************************************
#
#  NAME: magp$reset
#
#  PURPOSE:
#       Default reset received event handler routine for a pending
#       image.
#
#  DESCRIPTION:
#       This routine is called by the channel driver when a reset
#       event is received for a pending image. This routine removes
#       all task/ILTs from the im_pendtask queue of the IMT and
#       returns them to the originator's completion handler routine.
#
#  CALLING SEQUENCE:
#       call    magp$reset
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
magp$reset:
        PushRegs(r3)                    # Save all G registers
.magpreset_30:
        ld      im_pendtask(g5),r11     # r11 = first task/ILT on list
        cmpobe  0,r11,.magpreset_1000   # Jif no more task/ILTs on list
        ld      il_fthd(r11),r10        # r10 = next task/ILT on list
        st      r10,im_pendtask(g5)     # save new list head pointer
        lda     -ILTBIAS(r11),g1        # g1 = task/ILT at nest level #1
        ld      il_cr(g1),r4            # r4 = completion handler routine
        callx   (r4)                    # call completion handler routine
        ldq     16(r3),g4               # restore g4-g7
        b       .magpreset_30           # and check for more task/ILTs on list
#
.magpreset_1000:
        PopRegsVoid(r3)                 # Restore all G registers
        ret
#
#******************************************************************************
#
#  NAME: magp$offline
#
#  PURPOSE:
#       Default interface offline event handler routine for a pending
#       image.
#
#  DESCRIPTION:
#       This routine is called by the channel driver when an interface
#       offline event occurs for the associated channel interface for the
#       specified IMT when the image is pending. This routine removes all
#       the task/ILTs from the im_pendtask queue and returns them to the
#       originator's completion handler routine. It then places the
#       IMT on the inactive (registered) image queue.
#
#  CALLING SEQUENCE:
#       call    magp$offline
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       Note: The IMT has to be removed from the active queue before
#               this routine is called!
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#******************************************************************************
#
magp$offline:
        call    magp$reset              # return any/all task/ILTs on pending
                                        #  list
        mov     0,r3
        ld      im_ltmt(g5),r4          # r4 = assoc. LTMT address
        cmpobe  0,r4,.poffl100          # Jif no LTMT assoc. with IMT
        ld      ltmt_imt(r4),r5         # r5 = assoc. IMT for LTMT
        cmpobe  r5,g5,.poffl50          # Jif IMT/LTMT paired up
#
# --- IMT->LTMT orphaned; Just whack LTMT from IMT.
#
        st      r3,im_ltmt(g5)          # clear LTMT from IMT
        b       .poffl100
#
.poffl50:
        ld      ltmt_tmt(r4),r5         # r5 = assoc. TMT address
        cmpobne 0,r5,.poffl100          # Jif LTMT assoc. with TMT
#
# --- If LTMT is not assoc. with a TMT, the IMT/LTMT association can
#       be terminated and the LTMT cleaned up.
#
        movl    g0,r10                  # save g0-g1
        movl    g4,r8                   # save g4-g5
                                        # r9 = IMT address
        mov     r4,g4                   # g4 = LTMT address to clean up
        ldconst 0,g5                    # g5 = null TMT address
        call    LLD$pre_target_gone     # check if OK to terminate LTMT
        cmpobe  TRUE,g0,.poffl70        # Jif OK to terminate LTMT
        movl    0,r4
        stl     r4,im_mac(r9)           # clear MAC address from IMT to
                                        #  render it useless when all ops.
                                        #  have completed
        ld      mag_cleanup_pcb,r4      # check if IMT cleanup process running
        cmpobne 0,r4,.poffl90           # Jif process running
c       g0 = -1;                        # Flag task being started.
        st      g0,mag_cleanup_pcb
        lda     mag$imt_cleanup,g0      # establish IMT cleanup process
        ldconst MAGICPRI,g1
c       CT_fork_tmp = (ulong)"mag$imt_cleanup";
        call    K$fork
        st      g0,mag_cleanup_pcb
        b       .poffl90
#
.poffl70:
        ld      ltmt_ehand(g4),r7       # r7 = LTMT event handler table
        ld      ltmt_eh_tgone(r7),r6    # r6 = LTMT target gone event
                                        #  handler routine
        callx   (r6)                    # and call target gone event
                                        #  handler routine
#
#******************************************************************************
#
# --- Interface to Target Gone event handler routine
#
#   INPUT:
#       g4 = assoc. LTMT address
#       g5 = assoc. TMT address (null)
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#******************************************************************************
#
.poffl90:
        movl    r10,g0                  # restore g0-g1
        movl    r8,g4                   # restore g4-g5
.poffl100:
        st      r3,im_inacttmr(g5)      # clear inactive timer field
        call    C$regimt                # put IMT on inactive queue
        ret
#
#******************************************************************************
#
#  NAME: magp$online
#
#  PURPOSE:
#       Default interface online event handler routine for a pending
#       image.
#
#  DESCRIPTION:
#       This routine is called by the channel driver when an online
#       event has occurred associated with the specified pending
#       image. This routine does nothing.
#
#  CALLING SEQUENCE:
#       call    magp$online
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
magp$online:
        ret
#
#******************************************************************************
#
# ____________________________ SUBROUTINES ____________________________________
#
#******************************************************************************
#
#******************************************************************************
#
#  NAME: mag$enable_task
#
#  PURPOSE:
#       Enables a task as appropriate.
#
#  DESCRIPTION:
#       Places a task into the enabled state, taking into consideration
#       any flags (ilm_flag1, ilm_flag2) being set, any pending SENSE
#       data or any other considerations.
#
#  CALLING SEQUENCE:
#       call    mag$enable_task
#
#  INPUT:
#       g1 = pri. ILT of task at the inl2 nest level
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = pri. ILT at inl1 nest level
#       g8 = pointer to CDB
#
#  OUTPUT:
#       g1 = next task on work queue if next task can be enabled.
#       g1 = 0 if no other tasks on work queue or next
#               task on work queue cannot be enabled.
#
#  REGS DESTROYED:
#       g0-g3/g8-g14 can be destroyed.
#
#******************************************************************************
#
        .data
entask_tbl1:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
entask_tbl2:
        .byte   dtxferi,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
mag$enable_task:
        movq    g4,r12                  # save g4-g7
        ldob    ci_num(g4),r4           # Get interface number
        ld      ispOnline,r5            # Get port online bitmap
        bbc     r4,r5,.entask900        # Jif the port is OFFLINE

        ldob    ilm_flag2(g6),r10       # r10 = ILMT flag byte #2
        ldob    (g8),g0                 # g0 = SCSI command code

        ld      ilm_cmdtbl(g6),g11      # g11 = Command index table address
# .ifndef PERF
#         ld      ilm_origcmdhand(g6),g9  # g9 = Original command handler table address
#         ld      ilm_cmdhand(g6),r9      # r9 = Command handler table address
# c fprintf(stderr, "%sscsi cmd=0x%02x cmdtbl=%8.8lx origcmdhand=%8.8lx cmbhand=%8.8lx ", FEBEMESSAGE, (int)g0, g11, g9, r9);
# c fprintf(stderr, "%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x ", *((UINT8*)(g8+0)), *((UINT8*)(g8+1)), *((UINT8*)(g8+2)), *((UINT8*)(g8+3)), *((UINT8*)(g8+4)), *((UINT8*)(g8+5)), *((UINT8*)(g8+6)), *((UINT8*)(g8+7)), *((UINT8*)(g8+8)), *((UINT8*)(g8+9)), *((UINT8*)(g8+10)), *((UINT8*)(g8+11)), *((UINT8*)(g8+12)), *((UINT8*)(g8+13)), *((UINT8*)(g8+14)), *((UINT8*)(g8+15)));
# c switch (g0) { case SCC_TESTUNR: fprintf(stderr, "Test Unit Ready\n"); c break;
# c               case SCC_REWIND: fprintf(stderr, "RECALIBRATE or REWIND\n"); c break;
# c               case SCC_REQSENSE: fprintf(stderr, "REQUEST SENSE\n"); c break;
# c               case SCC_FORMAT: fprintf(stderr, "FORMAT UNIT\n"); c break;
# c               case SCC_READBLKLMT: fprintf(stderr, "READ BLOCK LIMITS\n"); c break;
# c               case SCC_REASSIGNBK: fprintf(stderr, "REASSIGN BLOCKS\n"); c break;
# c               case SCC_READ_6: fprintf(stderr, "READ (6)\n"); c break;
# c               case SCC_WRITE_6: fprintf(stderr, "WRITE (6)\n"); c break;
# c               case SCC_SEEK_6: fprintf(stderr, "SEEK (6)\n"); c break;
# c               case SCC_READ_RVS_6: fprintf(stderr, "READ REVERSE (6)\n"); c break;
# c               case SCC_WRITEFLM_6: fprintf(stderr, "WRITE FILEMARKS (6)\n"); c break;
# c               case SCC_SPACE_6: fprintf(stderr, "SPACE (6)\n"); c break;
# c               case SCC_INQUIRY: fprintf(stderr, "INQUIRY\n"); c break;
# c               case SCC_VERIFY_6: fprintf(stderr, "VERIFY (6)\n"); c break;
# c               case SCC_RCVRBUFDAT: fprintf(stderr, "RECOVER BUFFERED DATA\n"); c break;
# c               case SCC_MODESLC: fprintf(stderr, "MODE SELECT\n"); c break;
# c               case SCC_RESERVE_6: fprintf(stderr, "RESERVE (6)\n"); c break;
# c               case SCC_RELEASE_6: fprintf(stderr, "RELEASE (6)\n"); c break;
# c               case SCC_COPY: fprintf(stderr, "COPY\n"); c break;
# c               case SCC_ERASE_6: fprintf(stderr, "ERASE (6)\n"); c break;
# c               case SCC_MODESNS: fprintf(stderr, "Mode Sense (6)\n"); c break;
# c               case SCC_SSU: fprintf(stderr, "Start/Stop Unit or Load/Unload\n"); c break;
# c               case SCC_RCVDIAG: fprintf(stderr, "RECEIVE DIAGNOSTIC RESULTS\n"); c break;
# c               case SCC_SNDDIAG: fprintf(stderr, "SEND DIAGNOSTIC\n"); c break;
# c               case SCC_MEDIUM_PAR: fprintf(stderr, "PREVENT/ALLOW MEDIUM REMOVAL\n"); c break;
# c               case SCC_READFORMAT: fprintf(stderr, "READ FORMAT CAPACITIES (MMC)\n"); c break;
# c               case SCC_SET_WINDOW: fprintf(stderr, "SET WINDOW\n"); c break;
# c               case SCC_READCAP: fprintf(stderr, "Read Capacity (10)\n"); c break;
# c               case SCC_READEXT: fprintf(stderr, "Read (10) lba=0x%x lth=0x%x\n", ((SCSI_READ_EXTENDED*)g8)->lba, ((SCSI_READ_EXTENDED*)g8)->numBlocks); c break;
# c               case SCC_READGEN: fprintf(stderr, "READ GENERATION\n"); c break;
# c               case SCC_WRITEXT: fprintf(stderr, "Write (10) lba=0x%x lth=0x%x\n", ((SCSI_WRITE_EXTENDED*)g8)->lba, ((SCSI_WRITE_EXTENDED*)g8)->numBlocks); c break;
# c               case SCC_SEEK_10: fprintf(stderr, "SEEK (10)\n"); c break;
# c               case SCC_ERASE_10: fprintf(stderr, "ERASE (10)\n"); c break;
# c               case SCC_READUPDBLK: fprintf(stderr, "READ UPDATED BLOCK\n"); c break;
# c               case SCC_WRTVRFY_10: fprintf(stderr, "WRITE AND VERIFY (10)\n"); c break;
# c               case SCC_VERIMED: fprintf(stderr, "VERIFY MEDIA (10)\n"); c break;
# c               case SCC_SCHHIGH_10: fprintf(stderr, "SEARCH DATA HIGH (10)\n"); c break;
# c               case SCC_SCHEQ_10: fprintf(stderr, "SEARCH DATA EQUAL (10)\n"); c break;
# c               case SCC_SCHLOW_10: fprintf(stderr, "SEARCH DATA LOW (10)\n"); c break;
# c               case SCC_SETLMTS_10: fprintf(stderr, "SET LIMITS (10)\n"); c break;
# c               case SCC_PRFETCH_10: fprintf(stderr, "PRE-FETCH (10)\n"); c break;
# c               case SCC_SYNCCAC_10: fprintf(stderr, "SYNCHRONIZE CACHE (10)\n"); c break;
# c               case SCC_LKCACHE_10: fprintf(stderr, "LOCK/UNLOCK CACHE (10)\n"); c break;
# c               case SCC_RDDFTDT_10: fprintf(stderr, "READ DEFECT DATA (10)\n"); c break;
# c               case SCC_MEDIUMSCAN: fprintf(stderr, "MEDIUM SCAN\n"); c break;
# c               case SCC_COMPARE: fprintf(stderr, "COMPARE\n"); c break;
# c               case SCC_COPYVERIFY: fprintf(stderr, "COPY AND VERIFY\n"); c break;
# c               case SCC_WRITEBUF: fprintf(stderr, "WRITE BUFFER\n"); c break;
# c               case SCC_READBUF: fprintf(stderr, "READ BUFFER\n"); c break;
# c               case SCC_UPDATEBLK: fprintf(stderr, "UPDATE BLOCK\n"); c break;
# c               case SCC_READLONG: fprintf(stderr, "READ LONG\n"); c break;
# c               case SCC_WRITELONG: fprintf(stderr, "WRITE LONG\n"); c break;
# c               case SCC_CHANGEDEF: fprintf(stderr, "CHANGE DEFINITION\n"); c break;
# c               case SCC_WRTSAME_10: fprintf(stderr, "WRITE SAME (10)\n"); c break;
# c               case SCC_GETDENSITY: fprintf(stderr, "REPORT DENSITY SUPPORT\n"); c break;
# c               case SCC_PLAYAUDIO: fprintf(stderr, "PLAY AUDIO (10)\n"); c break;
# c               case SCC_GETCONFIG: fprintf(stderr, "GET CONFIGURATION\n"); c break;
# c               case SCC_PLAYAUDMSF: fprintf(stderr, "PLAY AUDIO MSF\n"); c break;
# c               case SCC_EVENTNOTFY: fprintf(stderr, "GET EVENT STATUS NOTIFICATION\n"); c break;
# c               case SCC_PAUSERSUME: fprintf(stderr, "PAUSE / RESUME\n"); c break;
# c               case SCC_LOGSELECT: fprintf(stderr, "LOG SELECT\n"); c break;
# c               case SCC_LOGSENSE: fprintf(stderr, "LOG SENSE\n"); c break;
# c               case SCC_XDWRITE_10: fprintf(stderr, "XDWRITE (10)\n"); c break;
# c               case SCC_XPWRITE_10: fprintf(stderr, "XPWRITE (10)\n"); c break;
# c               case SCC_XDREAD_10: fprintf(stderr, "XDREAD (10)\n"); c break;
# c               case SCC_XDWRTRD_10: fprintf(stderr, "XDWRITEREAD (10)\n"); c break;
# c               case SCC_SNDOPCINFO: fprintf(stderr, "SEND OPC INFORMATION\n"); c break;
# c               case SCC_MDSELCT_10: fprintf(stderr, "MODE SELECT (10)\n"); c break;
# c               case SCC_RESERVE_10: fprintf(stderr, "RESERVE (10)\n"); c break;
# c               case SCC_RELEASE_10: fprintf(stderr, "RELEASE (10)\n"); c break;
# c               case SCC_REPAIRTRCK: fprintf(stderr, "REPAIR TRACK\n"); c break;
# c               case SCC_MODESNS_10: fprintf(stderr, "MODE SENSE (10)\n"); c break;
# c               case SCC_CLOSETRACK: fprintf(stderr, "CLOSE TRACK / SESSION\n"); c break;
# c               case SCC_RDBUFCAP: fprintf(stderr, "READ BUFFER CAPACITY\n"); c break;
# c               case SCC_SENDCUESHT: fprintf(stderr, "SEND CUE SHEET\n"); c break;
# c               case SCC_PRRSERVIN: fprintf(stderr, "PERSISTENT RESERVE IN\n"); c break;
# c               case SCC_PRSERVOUT: fprintf(stderr, "PERSISTENT RESERVE OUT\n"); c break;
# c               case SCC_EXTCDB: fprintf(stderr, "EXTENDED CDB\n"); c break;
# c               case SCC_VARLTHCDB: fprintf(stderr, "VARIABLE LENGTH CDB\n"); c break;
# c               case SCC_XDWRT_16: fprintf(stderr, "XDWRITE EXTENDED (16)\n"); c break;
# c               case SCC_REBUILD_16: fprintf(stderr, "REBUILD (16)\n"); c break;
# c               case SCC_REGENER_16: fprintf(stderr, "REGENERATE (16)\n"); c break;
# c               case SCC_EXTENDCOPY: fprintf(stderr, "EXTENDED COPY\n"); c break;
# c               case SCC_RECCOPYRLT: fprintf(stderr, "RECEIVE COPY RESULTS\n"); c break;
# c               case SCC_ATAPTHR_16: fprintf(stderr, "ATA COMMAND PASS THROUGH (16)\n"); c break;
# c               case SCC_A_CTL_IN: fprintf(stderr, "ACCESS CONTROL IN\n"); c break;
# c               case SCC_A_CTL_OUT: fprintf(stderr, "ACCESS CONTROL OUT\n"); c break;
# c               case SCC_READ_16: fprintf(stderr, "READ (16)\n"); c break;
# c               case SCC_WRITE_16: fprintf(stderr, "WRITE (16)\n"); c break;
# c               case SCC_ORWRITE: fprintf(stderr, "ORWRITE\n"); c break;
# c               case SCC_READ_ATTR: fprintf(stderr, "READ ATTRIBUTE\n"); c break;
# c               case SCC_WRITE_ATTR: fprintf(stderr, "WRITE ATTRIBUTE\n"); c break;
# c               case SCC_WRTVRFY_16: fprintf(stderr, "WRITE AND VERIFY (16)\n"); c break;
# c               case SCC_VERIFY_16: fprintf(stderr, "VERIFY (16)\n"); c break;
# c               case SCC_PRFETCH_16: fprintf(stderr, "PRE-FETCH (16)\n"); c break;
# c               case SCC_SYNCCAC_16: fprintf(stderr, "SYNCHRONIZE CACHE (16)\n"); c break;
# c               case SCC_SPACE_16: fprintf(stderr, "SPACE (16) or LOCK UNLOCK CACHE (16)\n"); c break;
# c               case SCC_WRTSAME_16: fprintf(stderr, "WRITE SAME (16)\n"); c break;
# c               case SCC_READCAP_16: fprintf(stderr, "READ CAPACITY (16) [Service action In (16)]\n"); c break;
# c               case SCC_SERVOUT_16: fprintf(stderr, "SERVICE ACTION OUT (16)\n"); c break;
# c               case SCC_REPLUNS: fprintf(stderr, "REPORT LUNS\n"); c break;
# c               case SCC_ATAPTHR_12: fprintf(stderr, "ATA COMMAND PASS THROUGH (12)\n"); c break;
# c               case SCC_SECPROTIN: fprintf(stderr, "SECURITY PROTOCOL IN\n"); c break;
# c               case SCC_REPORTOPCD: fprintf(stderr, "REPORT SUPPORTED OPCODES\n"); c break;
# c               case SCC_MAINTOUT: fprintf(stderr, "MAINTENANCE (OUT) (REPORT_KEY)\n"); c break;
# c               case SCC_MOVEMEDIUM: fprintf(stderr, "MOVE MEDIUM\n"); c break;
# c               case SCC_EXCHMEDIUM: fprintf(stderr, "EXCHANGE MEDIUM\n"); c break;
# c               case SCC_MVMEDATTCH: fprintf(stderr, "MOVE MEDIUM ATTACHED\n"); c break;
# c               case SCC_READ_12: fprintf(stderr, "READ (12)\n"); c break;
# c               case SCC_SERVICEOUT: fprintf(stderr, "SERVICE ACTION OUT (12)\n"); c break;
# c               case SCC_WRITE_12: fprintf(stderr, "WRITE (12)\n"); c break;
# c               case SCC_SERVICEIN: fprintf(stderr, "SERVICE ACTION IN (12)\n"); c break;
# c               case SCC_ERASE_12: fprintf(stderr, "ERASE (12)\n"); c break;
# c               case SCC_READDVDSTR: fprintf(stderr, "READ DVD STRUCTURE\n"); c break;
# c               case SCC_WRTVRFY_12: fprintf(stderr, "WRITE AND VERIFY (12)\n"); c break;
# c               case SCC_VERIFY_12: fprintf(stderr, "VERIFY (12)\n"); c break;
# c               case SCC_SCHHIGH_12: fprintf(stderr, "SEARCH DATA HIGH (12)\n"); c break;
# c               case SCC_SCHEQ_12: fprintf(stderr, "SEARCH DATA EQUAL (12)\n"); c break;
# c               case SCC_SCHLOW_12: fprintf(stderr, "SEARCH DATA LOW (12)\n"); c break;
# c               case SCC_SETLMTS_12: fprintf(stderr, "SET LIMITS (12)\n"); c break;
# c               case SCC_READSTATCH: fprintf(stderr, "READ ELEMENT STATUS ATTACHED\n"); c break;
# c               case SCC_SECPROTOUT: fprintf(stderr, "SECURITY PROTOCOL OUT\n"); c break;
# c               case SCC_SENDVOLTAG: fprintf(stderr, "SEND VOLUME TAG\n"); c break;
# c               case SCC_RDDFTDT_12: fprintf(stderr, "READ DEFECT DATA (12)\n"); c break;
# c               case SCC_READSTATUS: fprintf(stderr, "READ ELEMENT STATUS\n"); c break;
# c               case SCC_READCDMSF: fprintf(stderr, "READ CD MSF\n"); c break;
# c               case SCC_REDGRPIN: fprintf(stderr, "REDUNDANCY GROUP (IN)\n"); c break;
# c               case SCC_REDGRPOUT: fprintf(stderr, "REDUNDANCY GROUP (OUT)\n"); c break;
# c               case SCC_SPARE_IN: fprintf(stderr, "SPARE (IN)\n"); c break;
# c               case SCC_SPARE_OUT: fprintf(stderr, "SPARE (OUT)\n"); c break;
# c               case SCC_VOLUME_IN: fprintf(stderr, "VOLUME SET (IN)\n"); c break;
# c               case SCC_VOLUME_OUT: fprintf(stderr, "VOLUME SET (OUT)\n"); c break;
# c               default: fprintf(stderr, "not decoded\n");
# c            }
# c if (g11 == (UINT32)&lld$normtbl) {
# c   if (g0 != 0x3b && g0 != 0x3c) {     # Read buffer and Write buffer done all the time.
# c     fprintf(stderr, "%sLLD command=0x%02lx (%ld) origcmdhand=0x%8.8lx cmdhand=0x%8.8lx\n", FEBEMESSAGE, g0, g0, g9, r9);
# c   }
# c }
# .endif  # PERF
        mov     g1,g9                   # g9 = pri. ILT of task at inl2 nest
                                        #  level
        ld      il_fthd(g9),r9          # r9 = next task on work queue
#
# --- Process flag byte #2 being non-zero
#
# --- Check for commands that require special consideration
#
        ldconst inquiry,r11             # r11 = INQUIRY command code
        cmpobe  g0,r11,.entask150       # Jif INQUIRY command
# --- Process a RepLuns command also like the inquiry. The standard says
#     that rep luns command should never get a unit attention.
#     Fix for Tbolt00015132
        ldconst repluns,r11             # r11 = Report Luns command code
        cmpobe  g0,r11,.entask150       # Jif REPLUNS command
#
        cmpobe  0,r10,.entask103        # Jif flag byte #2 zero
#
#
# --- Command does not require special consideration
#
        lda     flag2_tbl,r11           # r11 = flag byte #2 SENSE table
        mov     0,r4                    # r4 = bit # being checked/processed
.entask101:
        bbc     r4,r10,.entask102       # Jif bit not set
        ld      (r11),r11               # r11 = pointer to SENSE data
        stob    r4,inl2_ecode(g9)       # save flag byte #2 bit # in task
        ldconst reqsense,r9             # r9 = REQUEST SENSE command code
        cmpobe  g0,r9,.entask120        # Jif REQUEST SENSE command
        ldq     entask_tbl1,r4          # load op. values into regs.
        ld      entask_tbl1+16,r8
        mov     r11,r7                  # r7 = pointer to SENSE data
        b       .entask140              # and finish up processing command
#
.entask102:
        addo    1,r4,r4                 # inc. bit # being processed
        addo    4,r11,r11               # inc. to next handler routine in
                                        #  table
        cmpobg  8,r4,.entask101         # Jif more bits to process
        b       .entask900              # Jif somehow we get here!!!
#
# --- Now do the above processing for ilm_flag3
#
.entask103:
        ldob    ilm_flag3(g6),r10       # r10 = ILMT flag byte #3
        cmpobe  0,r10,.entask200        # Jif flag byte #3 zero
        bbs     7,r10,.entask900        # Jif PR config retrieval in progress
        lda     flag3_tbl,r11           # r11 = flag byte #3 SENSE table
        mov     0,r4                    # r4 = bit # being checked/processed
.entask104:
        bbc     r4,r10,.entask105       # Jif bit not set
        ld      (r11),r11               # r11 = pointer to SENSE data
        stob    r4,inl2_ecode2(g9)      # save flag byte #2 bit # in task
        ldconst reqsense,r9             # r9 = REQUEST SENSE command code
        cmpobe  g0,r9,.entask120        # Jif REQUEST SENSE command
        ldq     entask_tbl1,r4          # load op. values into regs.
        ld      entask_tbl1+16,r8
        mov     r11,r7                  # r7 = pointer to SENSE data
        b       .entask139              # and finish up processing command
#
.entask105:
        addo    1,r4,r4                 # inc. bit # being processed
        addo    4,r11,r11               # inc. to next handler routine in
                                        #  table
        cmpobg  8,r4,.entask104         # Jif more bits to process
        b       .entask900              # Jif somehow we get here!!!
#
# --- Process REQUEST SENSE command while flag byte #2 non-zero
#
.entask120:
        ldob    4(g8),r3                # r3 = alloc. length from CDB
        mov     0,g0                    # allocate an SGL
        lda     sensesize,r9            # r9 = my sense data size
        cmpobne 0,r3,.entask130         # Jif alloc. length in CDB non-zero
        lda     task_etbl2,r3           # r3 = task event handler table
        ldq     sense_tbl2,r4           # load op. values into regs.
        ld      sense_tbl2+16,r8
        b       .entask141              # and complete command processing
#
.entask130:
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        st      r11,sghdrsiz+sg_addr(g0) # save sense data address in SGL
        cmpo    r3,r9                   # check if alloc. length < SENSE size
        sell    r9,r3,r9                # r9 = size of transfer to host
        st      r9,sghdrsiz+sg_len(g0)  # save size of data in SGL
        ldq     entask_tbl2,r4          # load op. values into regs.
        ld      entask_tbl2+16,r8
        mov     g0,r6                   # r6 = SGL pointer
.entask139:
        lda     task_etbl7a,r3          # r3 = task event handler table
        b       .entask141
#
.entask140:
        lda     task_etbl7,r3           # r3 = task event handler table
.entask141:
        mov     inl2_ps_finalio,r10     # r10 = new process state code
        lda     mag1_iocr,r11           # r11 = I/O completion routine
        st      r3,inl2_ehand(g9)       # save new event handler table
#
        stob    r10,inl2_pstate(g9)     # save task process state code
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        cmpobe  0,r6,.entask145         # Jif no SGL specified
        lda     sghdrsiz(r6),r6         # point to descriptor area
.entask145:
        stq     r4,xlcommand(g1)        # stuff op. req. record
        mov     g7,r9                   # r9 = assoc. ILT parameter structure
        mov     g9,r10                  # r10 = pri. ILT at inl2 nest level
        st      r11,otl2_cr(g1)         # save completion routine
        stt     r8,xlsgllen(g1)         # save sense length/SGL length/assoc.
                                        #  ILT param. structure/pri. ILT at
                                        #  inl2 nest level
        cmpobe  0,r6,.entask147         # Jif no SGL specified for this I/O
#
# --- SGL specified for this I/O.
#
        call    mag$updtlen             # update inl2_dtlen value and set
                                        #  appropriate xlreslen value in
                                        #  current I/O being processed.
        b       .entask149              # and continue processing I/O
#
# --- No SGL specified for this I/O processing.
#
.entask147:
        mov     0,r11                   # r11 = xlreslen for this I/O
        bbc     xlsndsc+16,r4,.entask148 # Jif ending status not being
                                        #  presented
        ldl     inl2_dtreq(g9),r8       # r8 = inl2_dtreq value
                                        # r9 = inl2_dtlen value
        subo    r9,r8,r11               # r11 = residual length for this I/O
.entask148:
        st      r11,xlreslen(g1)        # save xlreslen value for this I/O
.entask149:
        mov     g1,r11                  # r11 = my ILT nest level ptr.
#
.if     ERRLOG
        call    mag$chkenderr           # check if error ending status and
                                        #  log in error log if true
.endif  # ERRLOG
#
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        st      r11,otl3_OTL2(g1)       # save param. ptr. in next nest
        call    mag$ISP$receive_io      # issue channel directive
        b       .entask900              # return status indicating not to
                                        #  initiate another task.
#
# --- Process INQUIRY command while flag byte #2 non-zero
#
.entask150:
        mov     0,r9                    # r9 = 0 indicating not to process
                                        #  any more tasks
.entask200:
        ld      ilm_cmdhand(g6),g10     # g10 = command handler table
        addo    g0,g11,g11              # index into cmd. norm. table
        ldob    (g11),g0                # g0 = normalized command code
        lda     task_etbl2,r3           # r3 = enabled task event handler table
        st      r3,inl2_ehand(g9)       # save new task event handler table
#
        shlo    2,g0,g0                 # normalized command code * 4
        addo    g0,g10,g10              # index into command handler table
        ld      (g10),g10               # g10= command handler routine
#
#*********************************************************************
#
# --- Interface to command handler routines
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = pri. ILT address at inl1 nest level
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = pri. ILT address at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       All registers may be destroyed.
#
#*********************************************************************
#
        ldob    inl2_ttype(g9),r8       # r8 = current task type code
        callx   (g10)                   # and go to handler routine
        mov     r9,g1                   # g1 = next task on work queue
        movq    r12,g4                  # restore g4-g7
        cmpobe  0,g1,.entask1000        # Jif no more tasks on work queue
        cmpobne  inl2_tt_sim,r8,.entask900 # Jif previous task not simple task
        ld      inl2_ehand(g1),r7       # r7 = next task's event handler table
        ldob    inl2_ttype(g1),r6       # r6 = next task's task type code
        ldob    inl2_eh_tstate(r7),r5   # r5 = next task's task state code
        cmpobne inl2_tt_sim,r6,.entask900 # Jif next task not simple task
        cmpobe  inl2_ts_dorm,r5,.entask1000 # Jif next task in dormant state
.entask900:
        mov     0,g1                    # g1 = 0 indicating not to process
                                        #  next task on work queue
.entask1000:
        movq    r12,g4                  # restore g4-g7
        ret
#
#******************************************************************************
#
#  TABLE: flag2_tbl
#
#  PURPOSE:
#
#       Provide a table to index into based on which flag bits
#       are set in ilm2_flag2 byte. The values stored in this table
#       are the SENSE data pointers for the various check status
#       events which are represented in the ilm2_flag2 bits.
#
#******************************************************************************
#
        .data
flag2_tbl:
        .word   sense_poweron           # power-on reset occurred
        .word   sense_scsi_reset        # SCSI bus reset received
        .word   sense_bus_reset         # device bus reset occurred
        .word   sense_cmds_clr          # tasks cleared by another initiator
        .word   sense_MPchgd            # MODE parameters changed
        .word   sense_replunsdata_chgd  # Used to report addition/deletion of luns
        .word   sense_lunsize_chgd      # Vdisk size changed
        .word   sense_nosense           # unused
#
#******************************************************************************
#
#  TABLE: flag3_tbl
#
#  PURPOSE:
#
#       Provide a table to index into based on which flag bits
#       are set in ilm2_flag3 byte. The values stored in this table
#       are the SENSE data pointers for the various check status
#       events which are represented in the ilm2_flag3 bits.
#
#******************************************************************************
#
flag3_tbl:
        .word  sense_resv_preempted     # Reservations preempted
        .word  sense_resv_released      # Reservations released
        .word  sense_reg_preempted      # Registrations preempted
        .word  sense_nosense            # unused
        .word  sense_nosense            # unused
        .word  sense_nosense            # unused
        .word  sense_nosense            # unused
        .word  sense_nosense            # unused
#
        .text
#
#******************************************************************************
#
#  NAME: mag$abort_task
#
#  PURPOSE:
#       Find and abort the specified task if possible.
#
#  DESCRIPTION:
#       Locates the specified task (if possible) and if found
#       perform the appropriate operations to abort the task.
#
#  CALLING SEQUENCE:
#       call    mag$abort_task
#
#  INPUT:
#       g0 = exchange ID of task to abort
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$abort_task:
        movq    g0,r12                  # save g0-g3
        ld      ilm_whead(g6),g1        # g1 = pri. ILT of task to check
.abtask40:
        cmpobe  0,g1,.abtask1000        # Jif no tasks on work queue
        ld      -ILTBIAS+idf_exid(g1),g2 # g2 = exchange ID of task
        cmpobe  g0,g2,.abtask100        # Jif exchange ID matches
        ld      il_fthd(g1),g1          # inc. to next ILT on queue
        b       .abtask40               # and check next task ILT
#
.abtask100:
        ld      inl2_ehand(g1),g2       # g2 = task event handler table
        ld      inl2_eh_abort(g2),g3    # g3 = task's abort event handler
                                        #  routine
        call    mag$remtask             # remove task ILT from work queue
        callx   (g3)                    # and call abort event handler routine
.abtask1000:
#
# --- Added call to mag$chknexttask.  This resolves the issue where all I/O was
#     stuck and queued in a dormant state following a SCSI check condition.
#     This only occurred if a Vlink was on Lun 0 for this Target.
#
        call    mag$chknextask          # Activate another task that may be dormant
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: mag$abort_ilmt_tasks
#
#  PURPOSE:
#       Aborts all tasks associated with the specified ILMT.
#
#  DESCRIPTION:
#       Locates all tasks associated with the specified ILMT and
#       performs the appropriate operations to abort each task.
#
#  CALLING SEQUENCE:
#       call    mag$abort_ilmt_tasks
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$abort_ilmt_tasks:
        mov     g1,r12                  # save g1
.abiltask40:
        ld      ilm_whead(g6),g1        # g1 = task ILT address
        cmpobe  0,g1,.abiltask1000      # Jif no task ILTs on work queue
        ld      inl2_ehand(g1),r4       # r4 = task event handler table
        ld      inl2_eh_abort(r4),r5    # r5 = task abort event handler
                                        #  routine
        call    mag$remtask             # remove task ILT from work queue
        callx   (r5)                    # call task's abort event handler
                                        #  routine
        b       .abiltask40             # and check for more tasks to process
#
.abiltask1000:
        mov     r12,g1                  # restore g1
        ret
#
#******************************************************************************
#
#  NAME: mag$reset_ilmt_tasks
#
#  PURPOSE:
#       Aborts all tasks associated with the specified ILMT due
#       to a reset event.
#
#  DESCRIPTION:
#       Locates all tasks associated with the specified ILMT and
#       performs the appropriate operations to abort each task with
#       a reset event call.
#
#  CALLING SEQUENCE:
#       call    mag$reset_ilmt_tasks
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$reset_ilmt_tasks:
        mov     g1,r12                  # save g1
.rsiltask40:
        ld      ilm_whead(g6),g1        # g1 = task ILT address
        cmpobe  0,g1,.rsiltask1000      # Jif no task ILTs on work queue
        ld      inl2_ehand(g1),r4       # r4 = task event handler table
        ld      inl2_eh_reset(r4),r5    # r5 = task reset event handler
                                        #  routine
        call    mag$remtask             # remove task ILT from work queue
        callx   (r5)                    # call task's reset event handler
                                        #  routine
        b       .rsiltask40             # and check for more tasks to process
#
.rsiltask1000:
        mov     r12,g1                  # restore g1
        ret
#
#******************************************************************************
#
#  NAME: mag$init_task
#
#  PURPOSE:
#       Initializes the ILT of a task to be included in the ILMT's
#       task set. This routine is called when initially processing
#       a task received from an initiator.
#
#  DESCRIPTION:
#       Initializes the inl2 nest area of an ILT to facilitate
#       managing the task during it's various stages of processing.
#       Fields in the inl2 nest level that are initialized in this
#       routine include:
#
#       vrvrp (0)
#       inl2_pstate (inl2_ps_wt0)
#       inl2_cdbctl                     inl2_ttype
#       inl2_flag1                      inl2_ecode
#       inl2_ilmt                       inl2_ehand (task_etbl1)
#       inl2_dtreq (scdatalen)          inl2_dtlen (0)
#
#       inl2_dtreq field initialization contains logic that attempts
#       to determine whether the value in scdatalen is valid or not
#       and if not valid sets the value in inl2_dtreq to 0.
#
#  CALLING SEQUENCE:
#       call    mag$init_task
#
#  INPUT:
#       g1 = primary ILT at inl2 nest level
#       g6 = assoc. ILMT address (0 denotes no assoc. ILMT)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$init_task:
        ldob    inl2_cdb(g1),r9         # r9 = CDB group code/command code
        lda     ctl_norm,r11            # r11 = command control byte offset
                                        #  normalization table
        movl    0,r4                    # clear r4-r5
        shro    5,r9,r9                 # isolate group code bits
        lda     inl2_cdb(g1),r10        # r10 = pointer to temp CDB
        addo    r9,r11,r11              # r11 = location of control byte offset
                                        #  in CDB
        st      r4,-ILTBIAS+vrvrp(g1)   # clear vrvrp field
        ldconst inl2_ps_wt0,r8          # r8 = initial process state code
        ldob    (r11),r9                # r9 = control byte offset in CDB
        st      r4,inl2_dtlen(g1)       # clear inl2_dtlen
        ldob    -ILTBIAS+scexecc(g1),r6 # r6 = task execution codes
        ldob    -ILTBIAS+sctaskc(g1),r11 # r11 = task codes which include the
                                        #  task attributes field
        stob    r8,inl2_pstate(g1)      # initialize process state code
        addo    r9,r10,r10              # r10 = pointer to control byte
                                        #  in CDB
        and     3,r6,r6                 # r6 = task READ/WRITE bits from
                                        #  execution code field
        and     7,r11,r11               # r11 = task attribute code
        stos    r4,inl2_flag1(g1)       # clear flag byte #1 & error code
                                        #  field
        cmpobe  0,r6,.initask_50        # Jif neither READ/WRITE flags set
        ld      -ILTBIAS+scdatalen(g1),r5 # r5 = data length value from
                                        #  command frame
.initask_50:
        stob    r11,inl2_ttype(g1)      # save task type code
        st      r5,inl2_dtreq(g1)       # save inl2_dtreq value to use when
                                        #  processing this CDB.
        cmpobe  0,r9,.initask_100       # Jif CDB type not supported
        ldob    (r10),r9                # r9 = control byte from CDB
.initask_100:
        shlo    2,r11,r11               # task type code * 4
        lda     task_etbl1,r12          # r12 = task event handler table
        stob    r9,inl2_cdbctl(g1)      # save CDB control byte
        lda     tag_counts(r11),r11     # r11 = address of tag counter
        st      r12,inl2_ehand(g1)      # save event handler table
        ld      (r11),r12               # r12 = current tag counter
        st      g6,inl2_ilmt(g1)        # save assoc. ILMT address
        addo    1,r12,r12               # inc. tag counter
        st      r12,(r11)               # save updated tag counter
        ret
#
# --- Command control byte offset normalization table
#
# --- Use the group code from the CDB group code/command code
#       byte to index into this table to get the offset into
#       the CDB where the control byte resides.
#
        .data
ctl_norm:
        .byte   5                       # 6 byte command
        .byte   9                       # 10 byte command
        .byte   9                       # 10 byte command
        .byte   0                       # reserved
        .byte   15                      # 16 byte command
        .byte   11                      # 12 byte command
        .byte   0                       # vendor specific
        .byte   0                       # vendor specific
#
        .text
#
#******************************************************************************
#
#  NAME: mag$qtask2wq
#
#  PURPOSE:
#       Queues an ILT associated with an initiator task to the
#       working queue in the associated ILMT.
#
#  DESCRIPTION:
#       Places the specified ILT onto the end of the working list
#       of tasks associated with the specified ILMT.
#
#  CALLING SEQUENCE:
#       call    mag$qtask2wq
#
#  INPUT:
#       g1 = primary ILT of task to queue
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       g0 = TRUE if queue was empty.
#       g0 = FALSE if queue was NOT empty.
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#******************************************************************************
#
mag$qtask2wq:
        ldl     ilm_whead(g6),r14       # r14 = first element on list
                                        # r15 = last element on list
        lda     ilm_whead(g6),r13       # r13 = base address of queue
        mov     0,r4                    # r4 = 0
        lda     1,r5                    # r5 = inl2_flag1 byte value
        cmpobne 0,r14,.qwq100           # Jif queue not empty
#
# --- Case: Queue was empty.
#
        mov     r13,r15                 # set base of queue as backward thread
        stl     r14,il_fthd(g1)         # save forward/backward threads in
                                        #  ILT
        mov     g1,g0
        stl     g0,(r13)                # save ILT as head & tail pointer
        mov     TRUE,g0                 # indicate queue was empty to caller
        b       .qwq900                 # and we're out of here!
#
# --- Case: Queue was NOT empty. Place on end of queue.
#
.qwq100:
        st      g1,il_fthd(r15)         # link new ILT onto end of list
        st      g1,ilm_wtail(g6)        # save new ILT as new tail
        st      r4,il_fthd(g1)          # clear forward thread in new ILT
        st      r15,il_bthd(g1)         # save backward thread in new ILT
        mov     FALSE,g0                # indicate queue was NOT empty
                                        #  to caller
.qwq900:
        stob    r5,inl2_flag1(g1)       # set flag indicating ILT is on
                                        #  the working queue
        ret
#
#******************************************************************************
#
#  NAME: mag$qtask2aq
#
#  PURPOSE:
#       Queues an ILT associated with an initiator task to the
#       aborted queue in the associated ILMT.
#
#  DESCRIPTION:
#       Places the specified ILT onto the end of the aborted list
#       of tasks associated with the specified ILMT.
#
#  CALLING SEQUENCE:
#       call    mag$qtask2aq
#
#  INPUT:
#       g1 = primary ILT of task to queue
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$qtask2aq:
        ldl     ilm_ahead(g6),r14       # r14 = first element on list
                                        # r15 = last element on list
        lda     ilm_ahead(g6),r13       # r13 = base address of queue
        mov     0,r4                    # r4 = 0
        lda     2,r5                    # r5 = inl2_flag1 byte value
        cmpobne 0,r14,.qaq100           # Jif queue not empty
#
# --- Case: Queue was empty.
#
        mov     r13,r15                 # set base of queue as backward thread
        stl     r14,il_fthd(g1)         # save forward/backward threads in
                                        #  ILT
        st      g1,(r13)                # save ILT as head & tail pointer
        st      g1,4(r13)
        b       .qaq900                 # and we're out of here!
#
# --- Case: Queue was NOT empty. Place on end of queue.
#
.qaq100:
        st      g1,il_fthd(r15)         # link new ILT onto end of list
        st      g1,ilm_atail(g6)        # save new ILT as new tail
        st      r4,il_fthd(g1)          # clear forward thread in new ILT
        st      r15,il_bthd(g1)         # save backward thread in new ILT
.qaq900:
        stob    r5,inl2_flag1(g1)       # set flag indicating ILT is on
                                        #  the working queue
        ret
#
#******************************************************************************
#
#  NAME: mag$remtask
#
#  PURPOSE:
#       Removes the specified ILT associated with an initiator
#       task from a queue in the specified ILMT if appropriate.
#
#  DESCRIPTION:
#       Checks the flags in the specified ILT to see if it resides
#       on a queue maintained in the specified ILMT. If so,
#       removes the ILT from the appropriate queue and clears the
#       flag indicating the ILT is no longer on the queue. If the
#       ILT has no flags indicating it resides on a queue in the ILMT,
#       this routine simply returns to the caller.
#
#  CALLING SEQUENCE:
#       call    mag$remtask
#
#  INPUT:
#       g1 = primary ILT address of the task to process at the
#               inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$remtask:
        ldob    inl2_flag1(g1),r4       # r4 = flag byte indicating whether
                                        #  the ILT is on a queue.
        ldl     il_fthd(g1),r14         # r14 = forward thread of ILT
                                        # r15 = backward thread of ILT
        lda     ilm_whead(g6),r13       # r13 = base address of working queue
        cmpobe  0,r4,.remt1000          # Jif no flags set indicating on a
                                        #  queue in the ILMT
        mov     0,r5                    # r5 = 0
        bbs     0,r4,.remt500           # Jif on working queue
        bbc     1,r4,.remt900           # Jif NOT on aborted queue
#
# --- Remove task from aborted queue
#
        lda     ilm_ahead(g6),r13       # r13 = base address of aborted queue
#
# --- Remove task from working queue
#
.remt500:
        st      r14,il_fthd(r15)        # put forward thread from removed ILT
                                        #  as forward thread of previous ILT
        cmpobne 0,r14,.remt700          # Jif non-zero forward thread
        mov     r13,r14                 # make base of queue the forward
                                        #  thread
        cmpobne r13,r15,.remt700        # Jif backward thread <> base of
                                        #  queue
        mov     0,r15                   # queue is now empty!
.remt700:
        st      r15,il_bthd(r14)        # put backward thread from removed
                                        #  ILT as backward thread of previous
                                        #  ILT
#
.remt900:
        stob    r5,inl2_flag1(g1)       # clear flag byte #1
.remt1000:
        ret
#
#******************************************************************************
#
#  NAME: mag$chknextask
#
#  PURPOSE:
#       Checks ILMT to see if the next task on the working
#       queue needs to be enabled and if so enables the task
#       and calls the handler routine to begin processing the
#       task. Also, if the next task needs to be enabled, also
#       checks subsequent tasks to see if it's appropriate to
#       enable and begin processing these tasks.
#
#  DESCRIPTION:
#       Checks the next task on the working queue (if one is
#       present). If the task is enabled already, this routine
#       simply returns to the caller. If the task is dormant,
#       enables the task and sets up to begin processing the
#       task. In this case, this routine checks the subsequent
#       task on the working queue (if there are any) and enables
#       and begins processing any more tasks if appropriate.
#
#  CALLING SEQUENCE:
#       call    mag$chknextask
#
#  INPUT:
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#      None.
#
#******************************************************************************
#
# C access
# void MAG_CheckNextTask(ILMT *pILMT)
        .globl  MAG_CheckNextTask       # C access
MAG_CheckNextTask:
        PushRegs(r3)
        mov     g0,g6                   # g6 = ILMT
        call    mag$chknextask          # Activate the task that may be blocked
        PopRegsVoid(r3)                 # Restore registers
        ret
#
mag$chknextask:
        ld      ilm_whead(g6),r15       # r15 = next task on work queue
        cmpobe  0,r15,.chkt1000         # Jif no tasks on working queue
        ld      inl2_ehand(r15),r14     # r14 = task event handler table
        ldob    inl2_eh_tstate(r14),r13 # r13 = task state code
        cmpobne inl2_ts_dorm,r13,.chkt1000 # Jif task not dormant

        PushRegs(r3)                    # Save all G registers
        mov     r15,g1                  # g1 = pri. ILT at inl2 nest level
.chkt30:
        ld      ilm_imt(g6),g5          # g5 = assoc. IMT address
        lda     -ILTBIAS(g1),g7         # g7 = pri. ILT at inl1 nest level
        lda     inl2_cdb(g1),g8         # g8 = pointer to 16 byte SCSI CDB
        ld      im_cimt(g5),g4          # g4 = assoc. CIMT address
        call    mag$enable_task         # enable next task on work queue
        cmpobne 0,g1,.chkt30            # Jif more tasks to enable
        PopRegsVoid(r3)                 # Restore all G registers
#
.chkt1000:
        ret
#
#******************************************************************************
#
#  NAME: mag$ISP$receive_io
#
#  PURPOSE:
#       This is an indirect call to the ISP$receive_io routine
#       that performs some additional processing before calling
#       the ISP$receive_io routine.
#
#  DESCRIPTION:
#       This is an indirect call to the ISP$receive_io routine
#       that checks for completion status included with the I/O
#       request and if present checks for Check Condition status
#       and if present performs the ACA processing. This routine
#       will also trace the event if traces are enabled.
#
#  CALLING SEQUENCE:
#       call    mag$ISP$receive_io
#
#  INPUT:
#       g1 = ILT of I/O operation at the otl3 nest level
#       g7 = pri. ILT at inl1 nest level
#       g9 = pri. ILT at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#      Regs. g0-g14 can be destroyed.
#
#******************************************************************************
#
mag$ISP$receive_io:
        ld      inl2_ilmt(g9),g6        # g6 = assoc. ILMT address
        ld      otl3_OTL2(g1),r15       # r15 = ILT of I/O op. at otl2 nest
#
.ifdef TRACES
        cmpobe  0,g6,.magISP_00         # Jif no ILMT assoc. with I/O
        call    mag$tr_ISP$receive_io   # trace event
.magISP_00:
.endif # TRACES
#
# --- Check if Check Condition status being presented to initiator
#
        ldob    xlfcflgs(r15),r4        # r4 = FC-AL flags for I/O operation
        ldob    xlscsist(r15),r5        # r5 = SCSI status
        bbc     xlsndsc,r4,.magISP_1000 # Jif no status w/IO
        cmpobe  scnorm,r5,.magISP_1000  # Jif normal (successful) status
        cmpobe  scechk,r5,.magISP_09    # Jif check condition status
#
# --- Non-Check Condition (and non-successful) status being presented
#       to initiator
#
.magISP_05:
        movl    g0,r12                  # save g0-g1
        mov     r5,g0                   # g0 = SCSI status
        mov     r15,g1                  # g1 = ILT of I/O op. at otl2 nest
        call    mag$mmcnonSENSE         # send event message to MMC
        movl    r12,g0                  # restore g0-g1
        b       .magISP_1000            # and continue processing request
#
# --- Check Condition status being presented to initiator
#
.magISP_09:
        ld      xlsnsptr(r15),r12       # r12 = SENSE data pointer
        cmpobe  0,r12,.magISP_05        # Jif no SENSE data pointer
        ldos    xlsnslen(r15),r13       # r13 = SENSE data length
        cmpobe  0,r13,.magISP_05        # Jif SENSE data length = 0
        movl    g0,r14                  # save g0-g1
        movl    r12,g0                  # g0 = SENSE data pointer
                                        # g1 = SENSE data length
        call    mag$mmcSENSE            # send event message to MMC
        movl    r14,g0                  # restore g0-g1
        cmpobe  0,g6,.magISP_1000       # Jif no ILMT associated with task
#
# --- Process the associated ACA event appropriately.
#
        mov     g1,r15                  # save g1
        ld      ilm_wkenv(g6),r14       # r14 = assoc. working environment
                                        #  table
        lda     mspga_QErr(r14),r13     # r13 = address of QErr byte in working
                                        #  environment table
        ldob    (r13),r12               # r12 = QErr byte
        ldconst 0x06,r11                # r11 = mask for QErr field
        and     r11,r12,r12             # r12 = QErr field value
        cmpobe  0,r12,.magISP_100       # Jif blocked tasks to resume after ACA
                                        #  cleared
#
# --- Abort all tasks on work queue except the one at hand
#
        ld      ilm_whead(g6),g1        # g1 = ILTs on work queue
        cmpobe  0,g1,.magISP_100        # Jif no more ILTs to process on work
                                        #  queue
.magISP_10:
        ld      il_fthd(g1),r14         # r14 = next task ILT on work queue
        cmpobe  g9,g1,.magISP_20        # Jif ILT in error
        call    mag$remtask             # remove task from work queue
        ld      inl2_ehand(g1),r13      # r13 = task event handler table
        ld      inl2_eh_abort(r13),r11  # r11 = abort event handler routine
#
#*****************************************************************************
#
#  Interface to Abort event handler routines:
#
#  INPUT:
#
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#*****************************************************************************
#
        callx   (r11)                   # call abort event handler routine
.magISP_20:
        mov     r14,g1                  # g1 = next task ILT on work queue
        cmpobne 0,g1,.magISP_10         # Jif more task ILTs to process on
                                        #  work queue
#
# --- Check whether ACA condition based on SCSI-2 or SCSI-3 rules
#
# --- If SCSI-2 rules apply, we do not set the ACA active flag because
#       the ACA condition does not persist and does not require a
#       corresponding initiator event to clear the ACA condition.
# --- If SCSI-3 rules apply, if tasks are to resume after the ACA
#       condition has been cleared, each task's ACA event handler
#       routine needs to be called to perform the appropriate
#       processing for the ACA event. In this case, the ACA condition
#       persists until a CLEAR ACA task management function is
#       received from the initiator receiving the Check Condition
#       status.
#
.magISP_100:
        ldob    inl2_cdbctl(g9),r14     # r14 = control byte from CDB
                                        # Note: The NACA flag in the control
                                        #       byte of the associated CDB
                                        #       determines whether SCSI-2 or
                                        #       SCSI-3 rules apply.
        bbc     2,r14,.magISP_900       # Jif SCSI-2 ACA rules apply
        ldob    ilm_flag1(g6),r14       # r14 = ILMT flag byte #1
        setbit  1,r14,r14               # set ACA active flag
        stob    r14,ilm_flag1(g6)       # save update flag byte #1
        cmpobe  0,r12,.magISP_900       # Jif tasks on work queue were aborted
#
# --- Call ACA event handler routine for all tasks on work queue
#       except the one at hand. The tasks on the work queue will
#       resume after the ACA condition has been cleared.
#
        ld      ilm_whead(g6),g1        # g1 = ILTs on work queue
        cmpobe  0,g1,.magISP_900        # Jif no more ILTs to process on work
                                        #  queue
.magISP_120:
        ld      il_fthd(g1),r14         # r14 = next task ILT on work queue
        cmpobe  g9,g1,.magISP_130       # Jif task ILT in error
        ld      inl2_ehand(g1),r13      # r13 = task event handler table
        ld      inl2_eh_aca(r13),r11    # r11 = ACA event handler routine
#
#*****************************************************************************
#
#  Interface to ACA event handler routines:
#
#  INPUT:
#
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#*****************************************************************************
#
        callx   (r11)                   # call ACA event handler routine
.magISP_130:
        mov     r14,g1                  # g1 = next task ILT on work queue
        cmpobne 0,g1,.magISP_120        # Jif more task ILTs to process
.magISP_900:
        mov     r15,g1                  # restore g1
.magISP_1000:
        b       ISP$receive_io          # and go request the I/O operation
#
#******************************************************************************
#
#  NAME: mag$upmodesns
#
#  PURPOSE:
#       Updates MODE SENSE page data in the working environment
#       table from MODE SELECT data. This routine assumes that
#       the MODE SELECT data has been validated!
#
#  DESCRIPTION:
#       Saves MODE SENSE data in the working environment table
#       based on MODE SELECT data specified in initiator data
#       and mode page changeable mask.
#
#  CALLING SEQUENCE:
#       call    mag$upmodesns
#
#  INPUT:
#       g0 = MODE SELECT page data address
#       g1 = MODE SENSE page data address
#       g2 = MODE SENSE change mask
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#      None.
#
#******************************************************************************
#
mag$upmodesns:
        ldob    1(g1),r12               # r12 = page length in bytes
        lda     2(g2),r10               # r10 = working MODE SENSE change mask
                                        #  address
        lda     2(g0),r8                # r8 = working MODE SELECT page data
                                        #  address
        lda     2(g1),r9                # r9 = working MODE SENSE page data
                                        #  address
.upmsns_10:
        ldob    (r10),r6                # r6 = change mask byte
        ldob    (r8),r4                 # r4 = MODE SELECT data byte
        ldob    (r9),r5                 # r5 = current MODE SENSE data byte
        addo    1,r8,r8                 # inc. MODE SELECT data pointer
        lda     1(r10),r10              # inc. change mask pointer
        subo    1,r12,r12               # dec. page byte count
        cmpobe  0,r6,.upmsns_50         # Jif nothing can change in this byte
        and     r6,r4,r4                # r4 = changeable bits from MODE
                                        #  SELECT data
        andnot  r6,r5,r5                # r5 = MODE SENSE data byte with
                                        #  changeable field masked off
        or      r4,r5,r5                # add in MODE SELECT data
        stob    r5,(r9)                 # save back updated MODE SENSE data
.upmsns_50:
        lda     1(r9),r9                # inc. MODE SENSE data pointer
        cmpobne 0,r12,.upmsns_10        # Jif more page data bytes to process
        ret                             # and we're done!
#
#******************************************************************************
#
#  NAME: mag$locreserve
#
#  PURPOSE:
#       Processes a request to locally reserve a device and to
#       initiate a request to globally reserve the same device.
#
#  DESCRIPTION:
#       Saves the initiator's associated ILMT in the VDMT as the
#       locally reserving initiator and allocates a VRP(and ILT
#       if necessary) to issue a reserve request to the MAGNITUDE
#       to globally reserve the device.
#
#  CALLING SEQUENCE:
#       call    mag$locreserve
#
#  INPUT:
#       g3 = assoc. VDMT address
#       g6 = assoc. ILMT of reserving initiator
#       g9 = task pri. ILT address of reserve (0 if none)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#      None.
#
#******************************************************************************
#
mag$locreserve:
        movt    g0,r12                  # save g0-g2
        ld      ilm_origcmdhand(g6),r3  # r3 = original command handler routine
                                        #  table
        st      g6,vdm_rilmt(g3)        # save initiator's ILMT as local
                                        #  reserving initiator
        st      r3,ilm_cmdhand(g6)      # make sure requesting initiator's
                                        #  ILMT has original command handler
                                        #  routine table (just in case)
#
# --- Give any other local initiators a new command handler table
#       to handle the device being reserved.
#
        lda     cmdtbl2,r3              # r3 = command handler table for
                                        #  other initiators for this device
                                        #  while device is reserved
        ld      vdm_ihead(g3),r4        # r4 = first assoc. ILMT for VDMT
.locres_10:
        cmpobe  0,r4,.locres_50         # Jif no more ILMTs to process
        cmpobe  r4,g6,.locres_20        # Jif reserving initiator's ILMT
        st      r3,ilm_cmdhand(r4)      # save new command handler table in
                                        #  ILMT for this initiator
.locres_20:
        ld      ilm_link(r4),r4         # get next ILMT on list
        b       .locres_10              # and process next ILMT on list
#
.locres_50:
        cmpobe  0,g9,.locres_500        # Jif no asoc. task ILT specified
#
# --- Process request with task pri. ILT specified
#
        ld      inl2_FCAL(g9),r7        # r7 = pri. ILT at FCAL nest level
c       g2 = get_vrp();                 # Allocate a VRP
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u get_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
        st      g2,vrvrp(r7)            # save VRP address in ILT
        lda     task_etbl9,r5           # r5 = task event handler table
        mov     vrresv,r6               # r6 = VRP request function code
        lda     mag1_MAGcomp,r8         # r7 = completion handler routine
        st      r5,inl2_ehand(g9)       # save new task event handler table
        st      r6,vr_func(g2)          # save VRP request function code, clear
                                        #  strategy and status fields
        st      r8,inl2_cr(g9)          # save ILT completion routine
        ldconst mystrategy,r6           # r6 = VRP strategy
        ldos    vdm_vid(g3),r4          # r4 = vid
        mov     inl2_ps_req,r8          # r8 = new task process state code
        stob    r6,vr_strategy(g2)      # save VRP strategy
        lda     ILTBIAS(g9),g1          # g1 = pri. ILT at inl3 nest level
        stos    r4,vr_vid(g2)           # save vid in VRP
        st      g2,vrvrp(g1)            # save VRP address in ILT level 3
        stob    r8,inl2_pstate(g9)      # save new process state code
        st      r7,inl3_FCAL(g1)        # save FCAL pointer in inl3 data
                                        #  structure
        movl    0,r4                    # r4-r5 = 0
        st      r4,vr_sglptr(g2)        # clear SGL pointer field in VRP
        stl     r4,vrpsiz+sg_desc0+sg_addr(g2) # Clear SGL address & length
                                        #  fields in VRP
        st      r4,vr_sglsize(g2)       # clear SGL size field
        st      r4,vrpsiz+sg_size(g2)   # clear SGL size field
        st      r4,vrpsiz+sg_scnt(g2)   # clear descriptor count
#
.ifdef TRACES
        call    mag$tr_MAG$submit_vrp   # trace event
.endif # TRACES
#
        call    MAG$submit_vrp          # send VRP to MAGNITUDE
        b       .locres_1000            # and we're out of here!
#
# --- Process request with no task pri. ILT specified
#
.locres_500:
        mov     g9,r15                  # save g9
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        mov     g1,g9                   # g9 = ILT at my nest level (inl2)
c       g2 = get_vrp();                 # Allocate a VRP
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u get_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
        mov     vrresv,r6               # r6 = VRP request function code
        lda     mag2_MAGcomp,r8         # r7 = completion handler routine
        st      r6,vr_func(g2)          # save VRP request function code, clear
                                        #  strategy and status fields
        st      r8,inl2_cr(g9)          # save ILT completion routine
        ldconst mystrategy,r6           # r6 = VRP strategy
        st      g2,vrvrp(g9)            # save assoc. VRP in ILT
        stob    r6,vr_strategy(g2)      # save VRP strategy
        st      g6,il_w1(g9)            # save assoc. ILMT in ILT (w1)
        ldos    vdm_vid(g3),r4          # r4 = vid
        lda     ILTBIAS(g9),g1          # g1 = pri. ILT at inl3 nest level
        stos    r4,vr_vid(g2)           # save vid in VRP
        st      g2,vrvrp(g1)            # save the VRP at this nest level
        movl    0,r4                    # r4-r5 = 0
        st      r4,inl3_FCAL(g1)        # clear FCAL pointer in inl3 data
                                        #  structure
        st      r4,vr_sglptr(g2)        # clear SGL pointer field in VRP
        stl     r4,vrpsiz+sg_desc0+sg_addr(g2) # Clear SGL address & length
                                        #  fields in VRP
        st      r4,vr_sglsize(g2)       # clear SGL size field
        st      r4,vrpsiz+sg_size(g2)   # clear SGL size field
        st      r4,vrpsiz+sg_scnt(g2)   # clear descriptor count
        call    MAG$submit_vrp          # send VRP to MAGNITUDE
        mov     r15,g9                  # restore g9
.locres_1000:
        movt    r12,g0                  # restore g0-g2
        ret
#
#******************************************************************************
#
#  NAME: mag$locrelease
#
#  PURPOSE:
#       Processes a request to locally release a device and to
#       initiate a request to globally release the same device
#       if necessary. A global release is deemed necessary if
#       the vdm_rilmt field contains a non-zero value; if zero
#       this routine assumes that the global reserve did not
#       occur.
#
#  DESCRIPTION:
#       Restores the normal command handler table for all initiators
#       assigned to this device. If the vdm_rilmt field is non-zero,
#       clears the initiator's associated ILMT in the VDMT as the
#       locally reserving initiator and allocates a VRP(and ILT
#       if necessary) to issue a release request to the MAGNITUDE
#       to globally release the device.
#
#  CALLING SEQUENCE:
#       call    mag$locrelease
#
#  INPUT:
#       g3 = assoc. VDMT address
#       g6 = assoc. ILMT of releasing initiator
#       g9 = task pri. ILT address of release (0 if none)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#      None.
#
#******************************************************************************
#
mag$locrelease:
        movt    g0,r12                  # save g0-g2
        mov     0,r6
        ld      vdm_rilmt(g3),r7        # r7 = vdm_rilmt value upon input
        st      r6,vdm_rilmt(g3)        # "remove" initiator's ILMT as local
                                        #  reserving initiator
#
# --- Give any other local initiators a new command handler table
#       to handle the device no longer being reserved.
#
        ld      vdm_ihead(g3),r4        # r4 = first assoc. ILMT for VDMT
.locrel_10:
        cmpobe  0,r4,.locrel_50         # Jif no more ILMTs to process
        ld      ilm_origcmdhand(r4),r3  # r3 = original command handler table
        st      r3,ilm_cmdhand(r4)      # save new command handler table in
                                        #  ILMT for this initiator
        ld      ilm_link(r4),r4         # get next ILMT on list
        b       .locrel_10              # and process next ILMT on list
#
.locrel_50:
        cmpobe  0,r7,.locrel_1000       # Jif global release not necessary
        cmpobe  0,g9,.locrel_500        # Jif no asoc. task ILT specified
#
# --- Process request with task pri. ILT specified
#
        ld      inl2_FCAL(g9),r7        # r7 = pri. ILT at FCAL nest level
c       g2 = get_vrp();                 # Allocate a VRP
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u get_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
        st      g2,vrvrp(r7)            # save VRP address in ILT
        lda     task_etbl10,r5          # r5 = task event handler table
        mov     vrrelresv,r6            # r6 = VRP request function code
        lda     mag1_MAGcomp,r8         # r8 = completion handler routine
        st      r5,inl2_ehand(g9)       # save new task event handler table
        st      r6,vr_func(g2)          # save VRP request function code, clear
                                        #  strategy and status fields
        st      r8,inl2_cr(g9)          # save ILT completion routine
        ldconst mystrategy,r6           # r6 = VRP strategy
        ldos    vdm_vid(g3),r4          # r4 = vid
        mov     inl2_ps_req,r8          # r8 = new task process state code
        stob    r6,vr_strategy(g2)      # save VRP strategy
        lda     ILTBIAS(g9),g1          # g1 = pri. ILT at inl3 nest level
        st      g2,vrvrp(g1)            # save VRP address in ILT nest level 3
        stos    r4,vr_vid(g2)           # save vid in VRP
        stob    r8,inl2_pstate(g9)      # save new process state code
        st      r7,inl3_FCAL(g1)        # save FCAL pointer in inl3 data
                                        #  structure
        movl    0,r4                    # r4-r5 = 0
        st      r4,vr_sglptr(g2)        # clear SGL pointer field in VRP
        stl     r4,vrpsiz+sg_desc0+sg_addr(g2) # Clear SGL address & length
                                        #  fields in VRP
        st      r4,vr_sglsize(g2)       # clear SGL size field
        st      r4,vrpsiz+sg_size(g2)   # clear SGL size field
        st      r4,vrpsiz+sg_scnt(g2)   # clear descriptor count
#
.ifdef TRACES
        call    mag$tr_MAG$submit_vrp   # trace event
.endif # TRACES
#
        call    MAG$submit_vrp          # send VRP to MAGNITUDE
        b       .locrel_1000            # and we're out of here!
#
# --- Process request with no task pri. ILT specified
#
.locrel_500:
        mov     g9,r15                  # save g9
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        mov     g1,g9                   # g9 = ILT at my nest level (inl1)
c       g2 = get_vrp();                 # Allocate a VRP
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u get_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
        mov     0,r6
        st      g2,vrvrp(g9)            # save VRP address in ILT inl1 nest level
        stob    r6,vrchipi(g9)          # set chip instance to 0
        lda     .magex1000,r7           # r7 = inl1 level completion routine
        st      r7,inl1_cr(g9)          # save inl1 completion handler
        mov     vrrelresv,r6            # r6 = VRP request function code
        st      r6,vr_func(g2)          # save VRP request function code, clear
                                        #  strategy and status fields
        ldos    vdm_vid(g3),r4          # r4 = vid
        ldconst mystrategy,r6           # r6 = VRP strategy
        lda     ILTBIAS(g9),g1          # g1 = pri. ILT at inl2 nest level
        lda     mag3_MAGcomp,r8         # r8 = completion handler routine
        stob    r6,vr_strategy(g2)      # save VRP strategy
        st      r8,inl2_cr(g1)          # save ILT completion routine
        lda     ILTBIAS(g1),g1          # g1 = ILT at inl3 nest level
        stos    r4,vr_vid(g2)           # save vid in VRP
        movl    0,r4                    # r4-r5 = 0
        st      g2,vrvrp(g1)            # save VRP addr in ILT inl3 nest level
        st      g9,inl3_FCAL(g1)        # set FCAL pointer in inl3 data
                                        #  structure
        st      r4,vr_sglptr(g2)        # clear SGL pointer field in VRP
        stl     r4,vrpsiz+sg_desc0+sg_addr(g2) # Clear SGL address & length
                                        #  fields in VRP
        st      r4,vr_sglsize(g2)       # clear SGL size field
        st      r4,vrpsiz+sg_size(g2)   # clear SGL size field
        st      r4,vrpsiz+sg_scnt(g2)   # clear descriptor count
        call    MAG$submit_vrp          # send VRP to MAGNITUDE
        mov     r15,g9                  # restore g9
.locrel_1000:
        movt    r12,g0                  # restore g0-g2
        ret
#
#******************************************************************************
#
#  NAME: magex1000
#
#  PURPOSE:
#       Completion handler routine final nest.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       g1 = ILT.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
.magex1000:
        mov     g2,r8                   # save g2
        ld      vrvrp(g1),g2            # g2 = VRP address
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
        mov     r8,g2                   # restore g2
        ret
#
#******************************************************************************
#
#  NAME: mag$findblkilt
#
#  PURPOSE:
#       Locates and removes an associated SRP ILT from the
#       blocked queue.
#
#  DESCRIPTION:
#       Finds the associated SRP ILT on the blocked queue in
#       an ILMT and if found removes the ILT from the queue
#       and returns the address of it to the caller.
#
#  CALLING SEQUENCE:
#       call    mag$findblkilt
#
#  INPUT:
#       g6 = assoc. ILMT of task
#       g9 = pri. task ILT address at inl2 nest level
#
#  OUTPUT:
#       g1 = SRP ILT address if found on blocked queue
#       g1 = 0 if no associated SRP ILT found on blocked queue
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
mag$findblkilt:
        ld      ilm_bhead(g6),g1        # g1 = first SRP ILT on blocked queue
        cmpobe  0,g1,.blkilt_1000       # Jif no SRP ILTs on blocked queue
        ld      il_w2(g1),r15           # r15 = assoc. pri. ILT of assoc. task
        cmpobne g9,r15,.blkilt_100      # Jif first ILT not a match
#
# --- Case: First ILT on blocked queue is a match
#
        ld      il_fthd(g1),r15         # r15 = next ILT on blocked queue
        st      r15,ilm_bhead(g6)       # save next ILT address as new head
        cmpobne 0,r15,.blkilt_200       # Jif not the last one on list
        st      r15,ilm_btail(g6)       # clear tail pointer of queue
        b       .blkilt_200             # and return ILT to caller
#
.blkilt_100:
#
# --- First ILT on blocked queue not a match
#
        mov     g1,r13                  # r13 = previous ILT in list
        ld      il_fthd(g1),g1          # g1 = next ILT on list
        cmpobe  0,g1,.blkilt_1000       # Jif no more ILTs to check on list
        ld      il_w2(g1),r15           # r15 = assoc. pri. ILT of assoc. task
        cmpobne g9,r15,.blkilt_100      # Jif ILT not a match
#
# --- Case: Not the first ILT on blocked queue is a match
#
        ld      il_fthd(g1),r15         # r15 = next ILT on list
        st      r15,il_fthd(r13)        # remove ILT from list; put next ILT
                                        #  address in previous ILT's fwd. ptr.
        cmpobne 0,r15,.blkilt_200       # Jif not the last one on the list
        st      r13,ilm_btail(g6)       # save previous ILT as new tail element
                                        #  of list
.blkilt_200:
        mov     0,r15
        st      r15,il_fthd(g1)         # clear forward thread field in ILT
.blkilt_1000:
        ret
#
#******************************************************************************
#
#  NAME: mag$get_imt
#
#  PURPOSE:
#       Allocates an IMT for the caller.
#
#  DESCRIPTION:
#       Allocates an IMT from the spare IMT queue or allocates memory
#       from the free memory pool to create an IMT in none available.
#       This will put the IMT on the allocated link list, set the IMT
#       to use the default or pending event table, and clear numerous
#       fields including the ILMT directory.
#
#  CALLING SEQUENCE:
#       call    mag$get_imt
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g5 = allocated IMT address
#
#  REGS DESTROYED:
#       Reg. g5 destroyed.
#
#******************************************************************************
#
mag$get_imt:
        lda     MAG_p$event_tbl,r3      # r3 = IMT pending image event handler
                                        #  table address
c       g5 = get_imt();                 # Allocate an IMT, all zeroed.
.ifdef M4_DEBUG_IMT
c fprintf(stderr, "%s%s:%u get_imt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g5);
.endif # M4_DEBUG_IMT
        st      r3,im_ehand(g5)         # save IMT event handler table address
        ld      mag_imt_tail,r4         # r4 = last IMT on allocated IMT list
        cmpobne 0,r4,.getimt_1010       # Jif list not empty
        st      g5,mag_imt_head         # save IMT as new head pointer
        b       .getimt_1020

.getimt_1010:
        st      g5,im_link2(r4)         # link IMT onto end of allocated list
.getimt_1020:
        st      g5,mag_imt_tail         # save allocated IMT as new tail
        ret
#
#******************************************************************************
#
#  NAME: mag$put_imt
#
#  PURPOSE:
#       Deallocates an IMT back into the spare IMT pool.
#
#  DESCRIPTION:
#       Deallocates an IMT back into the spare IMT queue after removing
#       it from the allocated IMT list.
#
#  CALLING SEQUENCE:
#       call    mag$put_imt
#
#  INPUT:
#       g5 = IMT address to deallocate
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$put_imt:
        ld      mag_imt_head,r5         # r5 = first IMT on allocated list
        cmpobe  0,r5,.putimt_100        # Jif no IMTs on allocated list
        cmpobne g5,r5,.putimt_20        # Jif not a match
#
# --- IMT being deallocated is first on allocated list
#
        ld      im_link2(g5),r5         # r5 = next IMT on allocated list
        st      r5,mag_imt_head         # save as new list head
        cmpobne 0,r5,.putimt_100        # Jif not the last member of list
        st      r5,mag_imt_tail         # clear list tail pointer
        b       .putimt_100
#
# --- IMT being deallocated is not the first on allocated list
#
.putimt_20:
        mov     r5,r6                   # r6 = last IMT processed
        ld      im_link2(r5),r5         # r5 = next IMT to check
        cmpobe  0,r5,.putimt_100        # Jif no more IMTs to check
        cmpobne r5,g5,.putimt_20        # Jif not a match
        ld      im_link2(g5),r4         # r4 = next IMT after one being
                                        #  deallocated
        st      r4,im_link2(r6)         # remove IMT from list
        cmpobne 0,r4,.putimt_100        # Jif not the last on list
        st      r6,mag_imt_tail         # save new list tail pointer
#
# --- Put IMT being deallocated on the head of the spare IMT list
#
.putimt_100:
.ifdef M4_DEBUG_IMT
c fprintf(stderr, "%s%s:%u put_imt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g5);
.endif # M4_DEBUG_IMT
c       put_imt(g5);                    # Deallocate IMT.
        ret
#
#******************************************************************************
#
#  NAME: mag$get_ilmt
#
#  PURPOSE:
#       Allocates an ILMT for the caller.
#
#  DESCRIPTION:
#       Allocates an ILMT from the spare ILMT queue or allocates memory
#       from the free memory pool to create an ILMT in none available.
#
#  CALLING SEQUENCE:
#       call    mag$get_ilmt
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g2 = allocated ILMT address
#
#  REGS DESTROYED:
#       Reg. g2 destroyed.
#
#******************************************************************************
#
mag$get_ilmt:
c       g2 = get_ilmt();                # Get ILMT and working environment table
.ifdef M4_DEBUG_ILMT
c fprintf(stderr, "%s%s:%u get_ilmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_ILMT
        lda     ilmtsize(g2),r3         # r3 points to working environment table
        st      r3,ilm_wkenv(g2)        # save in ILMT
        ret
#
#******************************************************************************
#
#  NAME: mag$add2dtlen
#
#  PURPOSE:
#       Adds the SRP SGL buffer size(s) to the associated task's
#       inl2_dtlen field.
#
#  DESCRIPTION:
#       Adds up the SGL buffer size associated with a task and adds
#       this value to the task's inl2_dtlen field for use in checking
#       the amount of data transferred for a task. Also saves the
#       appropriate residual length value in the xlreslen field of
#       the secondary ILT.
#       Note: This routine assumes that an SGL is defined for the
#               I/O operation.
#
#  CALLING SEQUENCE:
#       call    mag$add2dtlen
#
#  INPUT:
#       g1 = sec. ILT at otl2 nest level with I/O request setup
#       g9 = pri. ILT at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$add2dtlen:
        ld      xlsglptr(g1),r14        # r14 = SGL pointer
        ldos    xlsgllen(g1),r13        # r13 = # SGL descriptors
        ld      inl2_dtlen(g9),r9       # r9 = length of buffers to this point
        ld      inl2_dtreq(g9),r7       # r7 = requested length of data
        ldob    xlcommand(g1),r6        # r6 = I/O request command byte
        mov     0,r10                   # r10 = xlreslen value to save
                                        # (initialize to 0)
.add2dtlen_10:
        ld      sg_len(r14),r8          # r8 = length of segment
        lda     sgdescsiz(r14),r14      # inc. to next segment in SGL
        subo    1,r13,r13               # dec. descriptor count
        addo    r8,r9,r9                # add to buffer length
        cmpobne 0,r13,.add2dtlen_10     # Jif more segments to size
        st      r9,inl2_dtlen(g9)       # save updated cumulative buffer size
        cmpobne r9,r7,.add2dtlen_1000   # Jif cumulative data length not what
                                        #  was requested
        cmpobne dtxferi,r6,.add2dtlen_1000 # Jif not READ type CDB
        ldob    inl2_ecode(g9),r5       # r5 = inl2 error code
        cmpobne.f 0,r5,.add2dtlen_1000  # Jif error has occurred
        ldob    xlfcflgs(g1),r5         # r5 = I/O request flags byte
        lda     task_etbl3a,r4          # r4 = new task event handler table
        st      r4,inl2_ehand(g9)       # save new task event handler table
        setbit  xlsndsc,r5,r5           # set flag to indicate present SCSI
                                        #  status w/IO
        stob    r5,xlfcflgs(g1)         # save updated flags
.add2dtlen_1000:
        st      r10,xlreslen(g1)        # save residual length in ILT
        ret
#
#******************************************************************************
#
#  NAME: mag$updtlen
#
#  PURPOSE:
#       Updates the inl2_dtlen field in the primary ILT and sets the
#       appropriate value in xlreslen field of the secondary ILT for
#       the I/O operation being initiated.
#
#  DESCRIPTION:
#       Adds up the SGL buffer size associated with a task and adds
#       this value to the task's inl2_dtlen field for use in checking
#       the amount of data transferred for a task. Also saves the
#       appropriate residual length value in the xlreslen field of
#       the secondary ILT.
#       Note: This routine assumes that an SGL is defined for the
#               I/O operation.
#
#  CALLING SEQUENCE:
#       call    mag$updtlen
#
#  INPUT:
#       g1 = sec. ILT at otl2 nest level with I/O request setup
#       g9 = pri. ILT at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$updtlen:
        ld      xlsglptr(g1),r14        # r14 = SGL pointer
        ldos    xlsgllen(g1),r13        # r13 = # SGL descriptors
        ld      inl2_dtlen(g9),r9       # r9 = length of buffers to this point
        ld      inl2_dtreq(g9),r7       # r7 = requested length of data
        mov     0,r10                   # r10 = xlreslen value to save
                                        # (initialize to 0)
.updtlen_10:
        ld      sg_len(r14),r8          # r8 = length of segment
        lda     sgdescsiz(r14),r14      # inc. to next segment in SGL
        subo    1,r13,r13               # dec. descriptor count
        addo    r8,r9,r9                # add to buffer length
        cmpobne 0,r13,.updtlen_10       # Jif more segments to size
        ldob    xlfcflgs(g1),r5         # r5 = I/O request flags byte
        st      r9,inl2_dtlen(g9)       # save updated cumulative buffer size
        bbc     xlsndsc,r5,.updtlen_1000 # Jif not presenting ending status
                                        # with I/O
        subo    r9,r7,r10               # r10 = residual length
.updtlen_1000:
        st      r10,xlreslen(g1)        # save residual length in ILT
        ret
#
#******************************************************************************
#
#  NAME: mag$mmcSENSE
#
#  PURPOSE:
#       Formats a message packet to be sent to the MMC indicating
#       SENSE data was sent to the server.
#
#  DESCRIPTION:
#       Formats the message packet (using the standard SENSE message
#       packet template mmc_SENSE) and then calls the routine to send
#       the packet to the MMC.
#
#  CALLING SEQUENCE:
#       call    mag$mmcSENSE
#
#  INPUT:
#       g0 = SENSE data pointer
#       g6 = assoc. ILMT address (0 if none)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$mmcSENSE:
#
# --- Just log the first n log events per time period
#
        ld      host_events,r3          # Increment the current host error count
        addo    1,r3,r3
        st      r3,host_events
        ldconst err_throttle,r4         # Compare to throttle
        cmpobg  r3,r4,.mmcMAGerr_1000   # Jif count > throttle - exit
#
# Don't send a message for resets or unsupported LUNs
#
        ldob    sscode(g0),r4           # r4 = ASC from SENSE data
        ldconst 0x29,r6                 # r6 = reset type event ASC
        ldconst 0x25,r7                 # r7 = LUN not supported type ASC
        cmpobe  r4,r6,.mmcMAGerr_1000   # Jif reset type event
        cmpobe  r4,r7,.mmcMAGerr_1000   # Jif LUN not supported type event
#
# Copy sense data into logging buffer
#
        lda     m_logbuffer,r14         # r14 = message packet template address
        ldq     (g0),r4                 # r4-r7 = 1st 16 SENSE data bytes
        ldos    16(g0),r8               # r8 = last 2 bytes of SENSE data
        stq     r4,ems_sense(r14)       # save 1st 16 bytes of SENSE data
        stos    r8,(ems_sense+16)(r14)  # save last 2 bytes of SENSE data
#
# Load registers for branch to common code
#
        mov     g0,r15                  # save g0
        ldconst mlehostsense,r3         # set MAGDriver sense event type
        ldconst emslen,r4               # set parm length
        ldconst 0,g0                    # zero out the unused 'error code'
        b       .mmcSenseEntry          # branch and return from common code
#
#******************************************************************************
#
#  NAME: mag$mmcQLerr
#
#  PURPOSE:
#       Formats a QLerr message packet to be sent to the MMC indicating
#       a QLogic error was reported.
#
#  DESCRIPTION:
#       Formats the message packet (using the standard QLerr message
#       packet template mmc_QLerr) and then calls the routine to send
#       the packet to the MMC.
#
#  CALLING SEQUENCE:
#       call    mag$mmcQLerr
#
#  INPUT:
#       g0 = error code from QLogic (FCAL driver)
#       g6 = assoc. ILMT address (0 if none)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
#******************************************************************************
#
#  NAME: mag$mmcnonSENSE
#
#  PURPOSE:
#
#       Formats a message packet to be sent to the MMC indicating
#       a non-SENSE error was sent to the server.
#
#  DESCRIPTION:
#
#       Formats the message packet (using the standard non-SENSE message
#       packet template mmc_nonSENSE) and then calls the routine to send
#       the packet to the MMC.
#
#  CALLING SEQUENCE:
#
#       call    mag$mmcnonSENSE
#
#  INPUT:
#
#       g0 = error status byte being sent to server
#       g6 = assoc. ILMT address (0 if none)
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
#******************************************************************************
#
#  NAME: mag$mmcMAGerr
#
#  PURPOSE:
#       Formats a message packet to be sent to the MMC indicating
#       an error was received from the MAGNITUDE.
#
#  DESCRIPTION:
#       Formats the message packet (using the standard MAG error message
#       packet template mmc_MAGerr) and then calls the routine to send
#       the packet to the MMC.
#
#  CALLING SEQUENCE:
#       call    mag$mmcMAGerr
#
#  INPUT:
#       g0 = MAGNITUDE error code (VRP status)
#       g6 = assoc. ILMT address (0 if none)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$mmcQLerr:
        ldconst mlehostqlogic,r3        # set qlogic event type
        b       .mmcMAGerr

mag$mmcnonSENSE:
        ldconst mlehostnonsense,r3      # set nonsense event type
        cmpobe.t  0x22,g0,.mmcnonSENSE_100 # Jif cmd terminate
        cmpobne.f 0x08,g0,.mmcMAGerr    # Jif not busy status
#
# --- Report every 100th SAN Links busy status
#
        ldob    mag_busy_count,r4       # r4 = busy status count
        addo    1,r4,r4                 # inc. busy status count
        ldconst 100,r5                  # r5 = busy count threshold value
        stob    r4,mag_busy_count       # save updated count
        cmpobg.t r5,r4,.mmcMAGerr_1000  # Jif busy count < threshold value
        mov     0,r4                    # clear the busy count
        stob    r4,mag_busy_count
        b       .mmcMAGerr
#
# --- Report every 128th SAN Links cmdt status and if the count exceeds 512, reset the port
#
.mmcnonSENSE_100:
        ld      ilm_imt(g6),r6          # r6 = assoc. IMT address
        cmpobe  0,r6,.mmcMAGerr         # Jif no IMT
        ld      im_cimt(r6),r7          # r7 =  cimt
        cmpobe  0,r7,.mmcMAGerr         # Jif no CIMT
        ldob    ci_num(r7),r8           # r8 = chip instance

        ld      mag_cmdt_count[r8*4],r4 # r4 = cmdt status count
        addo    1,r4,r4                 # inc. cmdt status count
        st      r4,mag_cmdt_count[r8*4] # save updated count
        ldconst 0x7f,r5                 # r5 = threshold valu is 128
        and     r4,r5,r5                # check the lower 7 bits
        cmpobne 0,r5,.mmcMAGerr_1000    # Jif cmdt count < log threshold value

#-- c fprintf(stderr,"%s%s:%u magNONSENSE[0x%02lX]:CIMT=0x%08lX IMT=0x%08lX ILMT=0x%08lX cnt = 0x%lX\n", FEBEMESSAGE, __FILE__, __LINE__, r8,r7,r6,g6,r4);
        ldconst 0x800,r5                # r5 = threshold value is 2048
        cmpobg.t r5,r4,.mmcMAGerr       # Jif cmdt count < reset threshold value

#-- c fprintf(stderr,"%s%s:%u magNONSENSE[0x%02lX] ResetChip!!!n", FEBEMESSAGE, __FILE__, __LINE__,r8);

        mov     0,r4                    # clear the cmdt count
        st      r4,mag_cmdt_count[r8*4] # clear cdmt count for the port

#        movl    g0,r4                   # save g0-g1
#        mov     r8,g0                   # g0 = chip instance
#        ldconst ecri,g1                 # g1 = reset reason code
#        call    ISP_ResetChip           # reset QLogic chip
#        movl    r4,g0                   # restore g0-g1
#
        b       .mmcMAGerr_1000
#
mag$mmcMAGerr:
#
# --- Just log the first n log events per time period
#
        ld      magerr_events,r3        # Increment the magerr reporting count
        addo    1,r3,r3
        st      r3,magerr_events
        ldconst err_throttle,r4         # Compare to throttle
        cmpobg  r3,r4,.mmcMAGerr_1000   # Jif count > throttle - exit
#
        ldconst mlemagdriver,r3         # set magdriver event type
#
.mmcMAGerr:                             # Common entry point
        ldconst emdlen,r4               # set parameter field length
        mov     g0,r15                  # r15 = error code, save g0
#
.mmcSenseEntry:                         # entry point from mag$mmcSense function
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
c       memcpy(&TmpStackMessage[0],&m_logbuffer,r4);
        stos    r3,mle_event(g0)        # save event type
        mov     r4,r14                  # save parm data length

        stob    r15,emd_errcode(g0)     # save error code in packet
#
# --- Set default values for the packet
#
        movl    0,r4                    # r4-r5 = default WWN
        ldconst 0xFFFF,r8               # r8 = default VID
        mov     0,r9                    # r9 = default channel #
        mov     0,r10                   # r10 = default target #
        cmpobe  0,g6,.mmcMAGerr_100     # Jif no ILMT specified
#
# --- ILMT specified by caller.
#
        ld      ilm_vdmt(g6),r13        # r13 = assoc. VDMT address
        cmpobe  0,r13,.mmcMAGerr_30     # Jif no VDMT
#
# --- VDMT specified by caller.
#
        ldos    vdm_vid(r13),r8         # r8 = VID of affected task

.mmcMAGerr_30:
        ld      ilm_imt(g6),r12         # r12 = assoc. IMT address
        cmpobe  0,r12,.mmcMAGerr_100    # Jif no IMT
        ldl     im_mac(r12),r4          # r4-r5 = WWN of server
        ld      im_cimt(r12),r9         # Get cimt
        ldob    ci_num(r9),r9           # r9 = channel #
        ldos    im_tid(r12),r10         # r10 = target ID
#
# --- Send it
#
.mmcMAGerr_100:
        stos    r8,emd_vid(g0)          # save VID
        stob    r9,emd_cnum(g0)         # save channel #
        stos    r10,emd_tid(g0)         # save target ID
        stl     r4,emd_wwn(g0)          # save World Wide Name
#   If there is a nonsense error of type reservation conflict, handle specially.
c       if (r3 == mlehostnonsense && (r15 == scresc || r15 == scacac || r15 == scbusy)) {
#                             (port, errcode, vid, wwn[0], wwn[1], tid)
c           MAG_add_delayed_message(r9, r15, r8, r4, r5, r10);
# ATS-415 Do not print out nonsense errors of type scechk.
c       } else if (!(r3 == mlehostnonsense && r15 == scechk)) {
c       # Message is on stack, copy it to MRP.
c           MSC_LogMessageStack(&TmpStackMessage[0],r14);
c       }
        mov     r15,g0                  # restore g0
.mmcMAGerr_1000:
        ret
#
#******************************************************************************
#
#  NAME: mag$mmczoneinquiry
#
#  PURPOSE:
#       Builds up a zoning inquiry MRP request for the caller.
#
#  DESCRIPTION:
#       Formats the message packet and then calls the routine to send
#       the packet to the MMC.
#
#  CALLING SEQUENCE:
#       call    mag$mmczoneinquiry
#
#  INPUT:
#       g0 = WWN of server
#       g2 = target ID
#       g5 = assoc. IMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$mmczoneinquiry:
        PushRegs(r3)                    # Save register contents
        mov     g5,g0                   # IMT
        call    fsl_LogZoneInquiry      # Send Zone Enq log to CCB
        PopRegsVoid(r3)                 # Restore registers
        ret
#
.ifdef TRACES
#
#******************************************************************************
#
#  NAME: mag$tr_ISP$receive_io
#
#  PURPOSE:
#       Trace call ISP$receive_io event & SENSE data if appropriate.
#
#  CALLING SEQUENCE:
#       call    mag$tr_ISP$receive_io
#
#  INPUT:
#       g1 = I/O request ILT address at otl3 nest level
#       g7 = pri. ILT at inl1 nest level
#       g9 = pri. ILT at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$tr_ISP$receive_io:
        ld      inl2_ilmt(g9),r3        # r3 = assoc. ILMT address
        ldconst trt_ISP$recv,r5         # r5 = trace record type code
        cmpobe  0,r3,rISP1000           # Jif no ILMT assoc. with task
        ld      ilm_cimt(r3),r4         # r4 = assoc. CIMT address
        lda     C_temp_trace,r10        # r10 = temp trace build area address
        ldos    ci_tflg(r4),r6          # r6 = trace flags
        ldob    idf_ci(g7),r7           # r7 = chip instance
        ld      idf_exid(g7),r8         # r8 = exchange ID
        bbc     tflg_ISP$recv,r6,rISP100 # Jif trace event disabled
        stob    r5,trr_trt(r10)         # save trace record type code
        ld      -ILTBIAS+xlcommand(g1),r9 # r9 = I/O command byte, SCSI status,
                                        #  FC-AL flags, reserved
        stob    r7,trr_ci(r10)          # save chip instance
        ldos    idf_lun(g7),r5          # r5 = LUN #
        stos    r8,trr_exid(r10)        # save exchange ID
        ld      idf_init(g7),r7         # r7 = initiator ID
        st      r9,4(r10)               # save I/O command byte, SCSI status,
                                        #  FC-AL flags, reserved
c       r8 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        stob    r5,7(r10)               # save LUN #
        st      r7,8(r10)               # save initiator ID
        st      r8,12(r10)              # save timestamp
        ld      ci_curtr(r4),r5         # r5 = current trace record pointer
        ldq     (r10),r12               # r12-r15 = trace record
        ldl     ci_begtr(r4),r6         # r6 = beginning trace record pointer
                                        # r7 = ending trace record pointer
        lda     trr_recsize(r5),r3      # r3 = next trace record pointer
        stq     r12,(r5)                # save trace record
        cmpoble r3,r7,rISP10            # Jif trace pointer not at end of
                                        #  trace area
        ldos    ci_tflg(r4),r5          # r5 = trace flags
        mov     r6,r3                   # set next trace record pointer to
                                        #  beginning of trace area
        bbc     tflg_wrapoff,r5,rISP10  # Jif wrap off flag not set
        mov     0,r5                    # turn off traces due to trace area
                                        #  wrapped.
        stos    r5,ci_tflg(r4)
rISP10:
        st      r3,ci_curtr(r4)         # save next trace record pointer
rISP100:
        ldos    ci_tflg(r4),r6          # r6 = trace flags
        ldconst trt_SENSE,r5            # r5 = trace record type code
        bbc     tflg_SENSE,r6,rISP1000  # Jif trace event disabled
        ld      -ILTBIAS+xlsnsptr(g1),r7 # r7 = SENSE data pointer
        ldos    -ILTBIAS+xlsnslen(g1),r8 # r8 = SENSE data length
        cmpobe  0,r7,rISP1000           # Jif SENSE data pointer = 0
        cmpobe  0,r8,rISP1000           # Jif SENSE data length = 0
        stob    r5,trr_trt(r10)         # save trace record type code
        ldconst 15,r8                   # r8 = # SENSE data bytes in trace
        ld      ci_curtr(r4),r5         # r5 = current trace record pointer
        lda     1(r10),r11              # r11 = pointer for SENSE data in
                                        #  trace record
rISP110:
        ldob    (r7),r12                # r12 = SENSE data byte
        subo    1,r8,r8                 # dec. SENSE data byte counter
        addo    1,r7,r7                 # inc. to next SENSE data byte
        stob    r12,(r11)               # save SENSE data byte in trace record
        addo    1,r11,r11               # inc. to next trace record byte
        cmpobne 0,r8,rISP110            # Jif more SENSE data bytes to copy
        ldq     (r10),r12               # r12-r15 = trace record
        ldl     ci_begtr(r4),r6         # r6 = beginning trace record pointer
                                        # r7 = ending trace record pointer
        lda     trr_recsize(r5),r3      # r3 = next trace record pointer
        stq     r12,(r5)                # save trace record
        cmpoble r3,r7,rISP120           # Jif trace pointer not at end of
                                        #  trace area
        ldos    ci_tflg(r4),r5          # r5 = trace flags
        mov     r6,r3                   # set next trace record pointer to
                                        #  beginning of trace area
        bbc     tflg_wrapoff,r5,rISP120 # Jif wrap off flag not set
        mov     0,r5                    # turn off traces due to trace area
                                        #  wrapped.
        stos    r5,ci_tflg(r4)
rISP120:
        st      r3,ci_curtr(r4)         # save next trace record pointer
rISP1000:
        ret
#
#******************************************************************************
#
#  NAME: mag$tr_iocomp
#
#  PURPOSE:
#       Trace I/O completion event & data if appropriate.
#
#  DESCRIPTION:
#       Traces a call to I/O completion handler event.
#
#  CALLING SEQUENCE:
#       call    mag$tr_iocomp
#
#  INPUT:
#       g0 = I/O completion status
#       g1 = pri. ILT address at inl2 nest level
#       g6 = assoc. ILMT (0 if none)
#       g9 = sec. ILT address at olt2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#      None.
#
#******************************************************************************
#
mag$tr_iocomp:
#
# --- Trace incoming event if appropriate
#
        cmpobe  0,g6,.iocrtr10z         # Jif no assoc. ILMT address
        ld      ilm_cimt(g6),r4         # r4 = assoc. CIMT address
        cmpobe  0,r4,.iocrtr10z         # Jif no assoc. CIMT address
        ldos    ci_tflg(r4),r12         # r12 = trace flags
        lda     C_temp_trace,r10        # r10 = trace record build pointer
        bbc     tflg_mag1iocr,r12,.iocrtr10e # Jif event trace disabled
        ld      xlFCAL(g9),r7           # r7 = ILT param. area
        ldob    ci_num(r4),r12          # r12 = chip instance
        ldconst trt_mag1iocr,r5         # r5 = trace record type code
        ld      inl2_ehand(g1),r3       # r3 = ILMT event handler table
        ld      idf_exid(r7),r8         # r8 = exchange ID
        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r12,trr_ci(r10)         # save chip instance
        st      g0,4(r10)               # save completion status code, clear
                                        #  unused byte
        ldob    inl2_eh_tstate(r3),r12  # r12 = current task state code
        stos    r8,trr_exid(r10)        # save chip instance
c       r8 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ldos    idf_lun(r7),r5          # r5 = assoc. LUN #
        stob    r12,5(r10)              # save current task state code
        ld      idf_init(r7),r3         # r3 = initiator ID
        stob    r5,7(r10)               # save LUN
        st      r8,12(r10)              # save timestamp
        st      r3,8(r10)               # save initiator ID
        ld      ci_curtr(r4),r3         # r3 = current trace record pointer
        ldq     (r10),r8                # r8-r11 = trace record
        ldl     ci_begtr(r4),r12        # r12 = trace area beginning pointer
                                        # r13 = trace area ending pointer
        lda     trr_recsize(r3),r5      # r5 = next trace record pointer
        stq     r8,(r3)                 # save trace record in CIMT trace area
        cmpoble r5,r13,.iocrtr10a       # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ci_tflg(r4),r3          # r3 = trace flags
        mov     r12,r5                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r3,.iocrtr10a # Jif wrap off flag not set
        mov     0,r3                    # turn off traces due to trace area
                                        #  wrapped.
        stos    r3,ci_tflg(r4)
.iocrtr10a:
        st      r5,ci_curtr(r4)         # save new current trace record pointer
.iocrtr10e:
        ldos    ci_tflg(r4),r12         # r12 = trace flags
        ld      xlsglptr(g9),r3         # r3 = SGL pointer
        bbc     tflg_data,r12,.iocrtr10z # Jif event trace disabled
        ld      ci_curtr(r4),r11        # r11 = current trace record pointer
        cmpobe  0,r3,.iocrtr10z         # Jif no SGL specified
        ld      sg_addr(r3),r7          # r7 = pointer to data
        ld      sg_len(r3),r5           # r5 = data transfer length
        ldconst trt_data,r3             # r3 = trace record type code
        bswap   r5,r5
        st      r5,trr_trt(r11)         # save length (3 bytes)
        stob    r3,trr_trt(r11)         # save trace record type code
        ldt     (r7),r8                 # r8-r10 = first 12 data bytes
        stt     r8,4(r11)               # save data
        lda     trr_recsize(r11),r5     # r5 = next trace record pointer
        ldl     ci_begtr(r4),r8         # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        cmpoble  r5,r9,.iocrtr10k       # Jif trace record pointer has not
                                        #  exceeded end of trace area
        mov     r8,r5                   # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r12,.iocrtr10k # Jif wrap off flag not set
        mov     0,r12                   # turn off traces due to trace area
                                        #  wrapped.
        stos    r12,ci_tflg(r4)
.iocrtr10k:
        st      r5,ci_curtr(r4)         # save new current trace record pointer
.iocrtr10z:
        ret
#
#******************************************************************************
#
#  NAME: mag$tr_MAG$submit_vrp
#
#  PURPOSE:
#       Trace call MAG$submit_vrp event.
#
#  DESCRIPTION:
#       Traces a call to MAG$submit_vrp if appropriate.
#
#  CALLING SEQUENCE:
#       call    mag$tr_MAG$submit_vrp
#
#  INPUT:
#       g2 = assoc. VRP address
#       g7 = pri. ILT at inl1 nest level
#       g9 = pri. ILT at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
mag$tr_MAG$submit_vrp:
        ld      inl2_ilmt(g9),r3        # r3 = assoc. ILMT address
        ldconst trt_MAGsubv,r5          # r5 = trace record type code
        ld      ilm_cimt(r3),r4         # r4 = assoc. CIMT address
        lda     C_temp_trace,r10        # r10 = temp trace build area address
        ldos    ci_tflg(r4),r6          # r6 = trace flags
        ldob    idf_ci(g7),r7           # r7 = chip instance
        ld      idf_exid(g7),r8         # r8 = exchange ID
        bbc     tflg_MAGsubv,r6,rMAG1000 # Jif trace event disabled
        stob    r5,trr_trt(r10)         # save trace record type code
        ldos    vr_func(g2),r9          # r9 = VRP request code
        stob    r7,trr_ci(r10)          # save chip instance
        stos    r9,4(r10)               # save VRP request code
        ldos    idf_lun(g7),r5          # r5 = LUN #
        ldos    vr_vid(g2),r9           # r9 = VID
        stos    r8,trr_exid(r10)        # save exchange ID
        stos    g9,6(r10)               # save the Virtual ID
c       r7 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ld      vr_vlen(g2),r8          # r8 = length in sectors
        stob    r5,7(r10)               # save LUN #
        st      r7,12(r10)              # save timestamp
        st      r8,8(r10)               # save length in sectors
        ld      ci_curtr(r4),r5         # r5 = current trace record pointer
        ldq     (r10),r12               # r12-r15 = trace record
        ldl     ci_begtr(r4),r6         # r6 = beginning trace record pointer
                                        # r7 = ending trace record pointer
        lda     trr_recsize(r5),r3      # r3 = next trace record pointer
        stq     r12,(r5)                # save trace record
        cmpoble r3,r7,rMAG10            # Jif trace pointer not at end of
                                        #  trace area
        ldos    ci_tflg(r4),r5          # r5 = trace flags
        mov     r6,r3                   # set next trace record pointer to
                                        #  beginning of trace area
        bbc     tflg_wrapoff,r5,rMAG10  # Jif wrap off flag not set
        mov     0,r5                    # turn off traces due to trace area
                                        #  wrapped.
        stos    r5,ci_tflg(r4)
rMAG10:
        st      r3,ci_curtr(r4)         # save next trace record pointer
rMAG1000:
        ret
#
.endif # TRACES
#
.if     ERRLOG
#
#******************************************************************************
#
#  NAME: mag$chkenderr
#
#  PURPOSE:
#       Checks for error ending status being issued and if true
#       logs the event in the error log if appropriate.
#
#  DESCRIPTION:
#       Checks if ending status included in I/O request. If true,
#       checks the status for non-successful status. If true, checks
#       if Check condition status or other error status and processes
#       each type of error status appropriately, checking if error
#       logging for the event is enabled and if so enters a record
#       in the error log.
#
#  CALLING SEQUENCE:
#       call    mag$chkenderr
#
#  INPUT:
#       g1 = ILT address of I/O request at otl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
SENS_elog:
        .ascii  "SENS"
ERST_elog:
        .ascii  "ERst"
#
mag$chkenderr:
        ldob    xlfcflgs(g1),r4         # r4 = FC-AL flags for I/O operation
        ldob    xlscsist(g1),r5         # r5 = SCSI status
        bbc     xlsndsc,r4,.chkenderr_1000 # Jif no status w/IO
        cmpobe  scnorm,r5,.chkenderr_1000 # Jif successful status
        ld      C_eflg,r14              # r14 = error log flags
        ld      xlFCAL(g1),r15          # r15 = pri. ILT at inl1 nest level
        cmpobne scechk,r5,.chkenderr_100 # Jif not check condition status
        ld      xlsnsptr(g1),r10        # r10 = SENSE data address
        ldos    xlsnslen(g1),r7         # r7 = SENSE data length
        cmpobe  0,r10,.chkenderr_100    # Jif SENSE data pointer = 0
        cmpobe  0,r7,.chkenderr_100     # Jif SENSE data length = 0
#
# --- Process SENSE ending status event
#
        bbc     eflg_SENSE,r14,.chkenderr_1000 # Jif event logging not enabled
        mov     g0,r14                  # save g0
        call    C$geterrrec             # allocate an error log record
        ld      SENS_elog,r4            # r4 = error record type ID
        ldob    scchipi(r15),r5         # r5 = chip instance
        ld      scrxid(r15),r7          # r7 = exchange ID
        ld      scinitiator(r15),r8     # r8 = initiator ID
        ldos    sclun(r15),r9           # r9 = LUN
        st      r4,err_rtc(g0)          # save error record type ID
        stos    r5,err_ci(g0)           # save chip instance
        stos    r7,err_exid(g0)         # save exchange ID
        st      r8,err_free(g0)         # save initiator ID
        ldq     (r10),r4                # r4-r7 = first 16 SENSE data bytes
        stq     r4,err_free+4(g0)       # save SENSE data bytes
        stos    r9,err_free+4(g0)       # save LUN
        b       .chkenderr_900          # and we're out of here
#
# --- Process non_SENSE ending status event
#
.chkenderr_100:
        bbc     eflg_staterr,r14,.chkenderr_1000 # Jif event logging not enabled
        mov     g0,r14                  # save g0
        call    C$geterrrec             # allocate an error log record
        ld      ERST_elog,r4            # r4 = error record type ID
        ldob    scchipi(r15),r5         # r5 = chip instance
        ld      scrxid(r15),r6          # r6 = exchange ID
        ldos    sclun(r15),r7           # r7 = LUN
        ld      scinitiator(r15),r8     # r8 = initiator ID
        ld      xlcommand(g1),r9        # r9 = command byte/SCSI status/FC-AL
                                        #  flags/unused
        st      r4,err_rtc(g0)          # save record type ID
        stos    r5,err_ci(g0)           # save chip instance
        stos    r6,err_exid(g0)         # save exchange ID
        st      r9,err_free(g0)         # save command byte/SCSI status/FC-AL
                                        #  flags/unused
        st      r8,err_free+4(g0)       # save initiator ID
        st      r7,err_free+8(g0)       # save LUN
        movl    0,r4
        stl     r4,err_free+12(g0)      # clear unused bytes
.chkenderr_900:
#
.ifdef TRACES
  .if TRACE_EL
        call    C$traceerr              # copy traces into error log record
  .endif # TRACE_EL
.endif # TRACES
#
        mov     r14,g0                  # restore g0
.chkenderr_1000:
        ret
#
#******************************************************************************
#
#  NAME: mag$chkioerr
#
#  PURPOSE:
#       Processes an I/O completion error (either I/O completion
#       or SRP completion) with respect to logging it in the
#       error log.
#
#  DESCRIPTION:
#       Checks if I/O completion error logging is enabled and
#       if so logs the I/O completion error event in the error
#       log.
#
#  CALLING SEQUENCE:
#       call    mag$chkioerr
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = pri. ILT address at inl2 nest level
#       g9 = sec. ILT address at otl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#      None.
#
#******************************************************************************
#
IOER_elog:
        .ascii  "IOer"
#
mag$chkioerr:
        ld      C_eflg,r14              # r14 = error log flags
        bbc     eflg_IOerr,r14,.chkioerr_1000 # Jif event logging disabled
        mov     g0,r15                  # save g0
        call    C$geterrrec             # allocate an error log record
        ld      xlFCAL(g9),r14          # r14 = pri. ILT at inl1 nest level
        ld      IOER_elog,r4            # r4 = error record type ID
        ldob    scchipi(r14),r5         # r5 = chip instance
        ld      scrxid(r14),r6          # r6 = exchange ID
        ldos    sclun(r14),r7           # r7 = LUN
        ld      scinitiator(r14),r8     # r8 = initiator ID
        st      r4,err_rtc(g0)          # save record type ID
        stos    r5,err_ci(g0)           # save chip instance
        stos    r6,err_exid(g0)         # save exchange ID
        ldt     inl2_cdb(g1),r4         # r4-r6 = first 12 bytes of CDB
        stos    r15,err_free(g0)        # save I/O completion status code
        stos    r7,err_free+2(g0)       # save LUN
        st      r8,err_free+4(g0)       # save initiator ID
        stt     r4,err_free+8(g0)       # save first 12 bytes of CDB
#
.ifdef TRACES
  .if TRACE_EL
        call    C$traceerr              # copy traces into error log record
  .endif # TRACE_EL
.endif # TRACES
#
        mov     r15,g0                  # restore g0
.chkioerr_1000:
        ret
#
.endif  # ERRLOG
#
#******************************************************************************
#
#  NAME: mag$startfu
#
#  PURPOSE:
#       Performs the necessary operations to start a FORMAT UNIT
#       process on the specified Virtual Drive.
#
#  DESCRIPTION:
#       This routine is called when the FORMAT UNIT command has
#       been processed to the point that we know how to proceed
#       with the FORMAT UNIT request. The specified primary ILT
#       contains an fu3 data structure in the pri. ILT nest level
#       #3 area the defines the parameters for the subsequent
#       format operation. This routine allocates an ILT to function
#       as the primary format process management table (FUPMT),
#       queues this ILT to the FORMAT UNIT task, wakes up the
#       FORMAT UNIT task and then returns to the caller.
#
#  CALLING SEQUENCE:
#       call    mag$startfu
#
#  INPUT:
#       g9 = pri. ILT of task containing the FORMAT UNIT CDB
#            at the inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#      None.
#
#******************************************************************************
#
mag$startfu:
        mov     0,r13                   # r13 is zero
        mov     g1,r15                  # save g1
c       g1 = get_ilt();                 # Allocate an ILT for the FUPMT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        ld      inl2_ilmt(g9),r4        # r4 = assoc. ILMT address
        ld      ilm_vdmt(r4),r5         # r5 = assoc. VDMT address
        st      g9,fpmt_priilt(g1)      # save assoc. pri. ILT in FUPMT
        st      r4,fpmt_ilmt(g1)        # save assoc. ILMT in FUPMT
        st      r5,fpmt_vdmt(g1)        # save assoc. VDMT in FUPMT
        st      r13,fpmt_sda(g1)        # clear SDA field (lower and upper
        st      r13,fpmt_sda+4(g1)      #  fields)
        st      r13,fpmt_flag(g1)       # clear flag byte #1, pri. VRP
                                        #  outstanding count, sec. VRP
                                        #  outstanding count, SRP
                                        #  outstanding count.
        stob    r13,fpmt_status(g1)     # clear status field
        st      g1,ILTBIAS+fu3_fupmt(g9) # save FUPMT address in pri. ILT
        ldob    ILTBIAS+fu3_flags(g9),r6 # r6 = process flags byte
        lda     task_etbl12d,r4         # r4 = pri. ILT task event handler table
                                        #  for FORMAT UNIT process.
        st      r4,inl2_ehand(g9)       # save task event handler table
        stob    r6,fpmt_flag(g1)        # save flags byte
        st      r13,fpmt_link(g1)       # clear forward thread
        st      r13,fpmt_secvrp(g1)     # clear sec. ILT/VRP list field
        ld      FU_tail,r4              # r4 = last FUPMT on list
        cmpobne 0,r4,.startfu_100       # Jif others on list
        st      g1,FU_head              # save ILT/FUPMT as new head element
        b       .startfu_120            # and continue processing request
#
.startfu_100:
        st      g1,fpmt_link(r4)        # link FUPMT onto end of list
.startfu_120:
        st      g1,FU_tail              # save new tail element
        ld      FU_pcb,r4               # r4 = pcb of FORMAT UNIT task
        cmpobe  0,r4,.startfu_1000      # Jif task not started
        ldob    pc_stat(r4),r5          # r5 = PCB status byte
        cmpobne pcnrdy,r5,.startfu_1000 # Jif process not not ready
        ldconst pcrdy,r5                # r5 = new process status byte
.ifdef HISTORY_KEEP
c CT_history_pcb(".startfu_120 setting ready pcb", r4);
.endif  # HISTORY_KEEP
        stob    r5,pc_stat(r4)          # save new process status byte
.startfu_1000:
        mov     r15,g1                  # restore g1
        ret
#
#******************************************************************************
#
# ____________________ TASK EVENT HANDLER TABLES ______________________________
#
#******************************************************************************
#
# --- Dormant task event handler table ----------------------------------------
#
        .data
task_etbl1:
        .byte   inl2_ts_dorm            # task state code
        .byte   0,0,0                   # reserved
        .word   teh_ignore              # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te1_abort               # abort task handler routine
        .word   te1_reset               # reset event handler routine
        .word   te1_offline             # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Enabled task event handler table for immediate type commands ------------
#
task_etbl2:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te2_iocomp              # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te2_abort               # abort task handler routine
        .word   te2_reset               # reset event handler routine
        .word   te2_offline             # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Enabled task event handler table for READ type commands -----------------
#
task_etbl3:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te3_iocomp              # task I/O completion
                                        #  handler routine
        .word   te3_MAGcomp             # MAGNITUDE request completion
                                        #  handler routine
        .word   te3_srpreq              # SRP request handler routine
        .word   te3_srpcomp             # SRP request completion handler
                                        #  routine
        .word   te3_abort               # abort task handler routine
        .word   te3_reset               # reset event handler routine
        .word   te3_offline             # offline event handler routine
        .word   te3_aca                 # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Enabled task event handler table for READ type commands where SCSI ------
#       status was presented at the completion of the data transfer.
#
task_etbl3a:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te3_iocomp              # task I/O completion
                                        #  handler routine
        .word   te3a_MAGcomp            # MAGNITUDE request completion
                                        #  handler routine
        .word   te3_srpreq              # SRP request handler routine
        .word   te3_srpcomp             # SRP request completion handler
                                        #  routine
        .word   te3_abort               # abort task handler routine
        .word   te3_reset               # reset event handler routine
        .word   te3_offline             # offline event handler routine
        .word   te3_aca                 # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Enabled task event handler table for WRITE type commands ----------------
#
task_etbl4:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te4_iocomp              # task I/O completion
                                        #  handler routine
        .word   te4_MAGcomp             # MAGNITUDE request completion
                                        #  handler routine
        .word   te4_srpreq              # SRP request handler routine
        .word   te4_srpcomp             # SRP request completion handler
                                        #  routine
        .word   te4_abort               # abort task handler routine
        .word   te4_reset               # reset event handler routine
        .word   te4_offline             # offline event handler routine
        .word   te4_aca                 # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Blocked basic task event handler table ----------------------------------------
#
task_etbl5:
        .byte   inl2_ts_blk             # task state code
        .byte   0,0,0                   # reserved
        .word   te5_iocomp              # task I/O completion
                                        #  handler routine
        .word   te5_MAGcomp             # MAGNITUDE request completion
                                        #  handler routine
        .word   te5_srpreq              # SRP request handler routine
        .word   te5_srpcomp             # SRP request completion handler
                                        #  routine
        .word   te5_abort               # abort task handler routine
        .word   te5_reset               # reset event handler routine
        .word   te5_offline             # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   te5_acaclr              # ACA cleared handler routine
#
# --- Blocked task with SRP request on blocked queue event handler table ------
#
task_etbl5a:
        .byte   inl2_ts_blk             # task state code
        .byte   0,0,0                   # reserved
        .word   teh_ignore              # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te5a_abort              # abort task handler routine
        .word   te5a_reset              # reset event handler routine
        .word   te5a_offline            # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   te5a_acaclr             # ACA cleared handler routine
#
# --- Blocked task with MAGNITUDE completion received event handler table ----
#
task_etbl5b:
        .byte   inl2_ts_blk             # task state code
        .byte   0,0,0                   # reserved
        .word   teh_ignore              # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te5_abort               # abort task handler routine
        .word   te5_reset               # reset event handler routine
        .word   te5_offline             # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   te5b_acaclr             # ACA cleared handler routine
#
# --- Aborted task event handler table ----------------------------------------
#
task_etbl6:
        .byte   inl2_ts_abt             # task state code
        .byte   0,0,0                   # reserved
        .word   te6_iocomp              # task I/O completion
                                        #  handler routine
        .word   te6_MAGcomp             # MAGNITUDE request completion
                                        #  handler routine
        .word   te6_srpreq              # SRP request handler routine
        .word   te6_srpcomp             # SRP request completion handler
                                        #  routine
        .word   teh_ignore              # abort task handler routine
        .word   teh_ignore              # reset event handler routine
        .word   teh_ignore              # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Enabled task event handler table for commands processed for -------------
#       bits set in ilm_flag2 byte
#
task_etbl7:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te7_iocomp              # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te2_abort               # abort task handler routine
        .word   te2_reset               # reset event handler routine
        .word   te2_offline             # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Enabled task event handler table for commands processed for -------------
#       bits set in ilm_flag3 byte
#
task_etbl7a:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te7a_iocomp             # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te2_abort               # abort task handler routine
        .word   te2_reset               # reset event handler routine
        .word   te2_offline             # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Enabled task event handler table for MODE SELECT command -------------
#
task_etbl8:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te8_iocomp              # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te8_abort               # abort task handler routine
        .word   te8_reset               # reset event handler routine
        .word   te8_offline             # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Enabled task event handler table for RESERVE type commands -----------
#
task_etbl9:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te2_iocomp              # task I/O completion
                                        #  handler routine
        .word   te9_MAGcomp             # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te9_abort               # abort task handler routine
        .word   te2_reset               # reset event handler routine
        .word   te2_offline             # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Enabled task event handler table for RELEASE type commands -----------
#
task_etbl10:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te2_iocomp              # task I/O completion
                                        #  handler routine
        .word   te10_MAGcomp            # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te2_abort               # abort task handler routine
        .word   te2_reset               # reset event handler routine
        .word   te2_offline             # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Enabled task event handler table for VERIFY type commands ----------------
#
task_etbl11:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te2_iocomp              # task I/O completion
                                        #  handler routine
        .word   te11_MAGcomp            # MAGNITUDE request completion
                                        #  handler routine
        .word   te4_srpreq              # SRP request handler routine
        .word   te4_srpcomp             # SRP request completion handler
                                        #  routine
        .word   te2_abort               # abort task handler routine
        .word   te2_reset               # reset event handler routine
        .word   te4_offline             # offline event handler routine
        .word   te3_aca                 # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Enabled task event handler table for FORMAT UNIT command -------------
#       PHASE 1 data transfer (Defect List Header)
#
task_etbl12a:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te12a_iocomp            # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te12_abort              # abort task handler routine
        .word   te12_reset              # reset event handler routine
        .word   te12_offline            # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Enabled task event handler table for FORMAT UNIT command -------------
#       PHASE 2 data transfer (Defect List + Initialization Pattern Desc.)
#
task_etbl12b:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te12b_iocomp            # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te12_abort              # abort task handler routine
        .word   te12_reset              # reset event handler routine
        .word   te12_offline            # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Enabled task event handler table for FORMAT UNIT command -------------
#       PHASE 3 data transfer (Initialization Pattern Descriptor)
#
task_etbl12c:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te12c_iocomp            # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te12_abort              # abort task handler routine
        .word   te12_reset              # reset event handler routine
        .word   te12_offline            # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Enabled task event handler table for FORMAT UNIT command -------------
#       Formatting Virtual Disk Process in progress.
#
task_etbl12d:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   teh_ignore              # task I/O completion
                                        #  handler routine
        .word   te12d_MAGcomp           # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te12_abort              # abort task handler routine
        .word   te12_reset              # reset event handler routine
        .word   te12_offline            # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Aborted task event handler table for commands that require a ------------
#       data transfer to be performed before continuing processing
#       the command.
#
task_etbl13:
        .byte   inl2_ts_abt             # task state code
        .byte   0,0,0                   # reserved
        .word   te13_iocomp             # task I/O completion
                                        #  handler routine
        .word   te6_MAGcomp             # MAGNITUDE request completion
                                        #  handler routine
        .word   te6_srpreq              # SRP request handler routine
        .word   te6_srpcomp             # SRP request completion handler
                                        #  routine
        .word   teh_ignore              # abort task handler routine
        .word   teh_ignore              # reset event handler routine
        .word   teh_ignore              # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Enabled task event handler table for SYNC CACHE command -----------------
#
task_etbl14:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te2_iocomp              # task I/O completion
                                        #  handler routine
        .word   te14_MAGcomp            # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te2_abort               # abort task handler routine
        .word   te2_reset               # reset event handler routine
        .word   te4_offline             # offline event handler routine
        .word   te4_aca                 # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Task event handler for persistent reserve out commands for
#     getting the parameter data which is not present in CDB
#
task_etbl16:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te16_iocomp             # task I/O completion
                                        #  handler routine
#        .word   mag1_MAGcomp            # MAGNITUDE request completion
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te8_abort               # abort task handler routine
        .word   te8_reset               # reset event handler routine
        .word   te8_offline             # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Task event handler for persistent reserve out commands for
#     waiting for the config update to complete
#
task_etbl17:
        .byte   inl2_ts_blk             # task state code
        .byte   0,0,0                   # reserved
        .word   teh_ignore              # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te17_abort              # abort task handler routine
        .word   te17_abort              # reset event handler routine
        .word   te17_abort              # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
        .text
#
#******************************************************************************
#
# ____________________ TASK EVENT HANDLER ROUTINES ____________________________
#
#******************************************************************************
#
# --- Generic (not specific to a process state) task event handler routines ---
#
#******************************************************************************
#
#******************************************************************************
#
#  NAME: teh_ignore
#
#  PURPOSE:
#       Ignores the event.
#
#  DESCRIPTION:
#       Simply returns to the caller effectively ignoring the event
#       being reported.
#
#  CALLING SEQUENCE:
#       call    teh_ignore
#
#  INPUT:
#       None. (actually it doesn't matter since we simply return
#               to the caller without doing any processing)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
teh_ignore:
        ret                             # and we're out of here!
#
#******************************************************************************
#
# --- State specific task event handler routines ------------------------------
#
#******************************************************************************
#
#
#******************************************************************************
#
#  NAME: te1_abort
#
#  PURPOSE:
#       Provide the appropriate processing for an abort event while
#       a task is waiting to begin processing (task_etbl1 event
#       handler table).
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue, returns
#       any resources if necessary and returns the associated ILT to
#       the originator.
#
#  CALLING SEQUENCE:
#       call    te1_abort
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#******************************************************************************
#
te1_abort:
        PushRegs(r3)                    # Save all G registers

        lda     -ILTBIAS(g1),g1         # back up to previous nest level
        ld      inl1_cr(g1),g0          # g0 = completion routine for
        callx   (g0)                    # go originator's completion routine
#
# --- Added call to mag$chknexttask.  This resolves the issue where all I/O was
#     stuck and queued in a dormant state following a SCSI check condition.
#     This only occurred if a Vlink was on Lun 0 for this Target.
#
        call    mag$chknextask          # Activate another task that may be dormant
#
        PopRegsVoid(r3)                 # Restore all G registers
        ret
#
#******************************************************************************
#
#  NAME: te1_reset
#
#  PURPOSE:
#       Provide the appropriate processing for a reset event while
#       a task is waiting to begin processing (task_etbl1 event
#       handler table).
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue, returns
#       any resources if necessary and returns the associated ILT to
#       the originator.
#
#  CALLING SEQUENCE:
#       call    te1_reset
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te1_reset:
        b       te1_abort               # handler the same as an abort task
                                        #  event
#
#******************************************************************************
#
#  NAME: te1_offline
#
#  PURPOSE:
#       Provide the appropriate processing for an offline event
#       while a task is waiting to begin processing (task_etbl1
#       event handler table).
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue, returns
#       any resources if necessary and returns the associated ILT to
#       the originator.
#
#  CALLING SEQUENCE:
#       call    te1_offline
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te1_offline:
        b       te1_abort               # handler the same as an abort task
                                        #  event
#
#******************************************************************************
#
#  NAME: te2_iocomp
#
#  PURPOSE:
#       Provide the processing to perform the task I/O
#       complete processing for immediate type commands.
#
#  DESCRIPTION:
#       Returns any resources and then returns the primary task
#       ILT back to the originator.
#
#  CALLING SEQUENCE:
#       call    te2_iocomp
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address ( 0 if no assoc. ILMT)
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te2_iocomp:
        movt    g12,r12                 # save g12-g14
        cmpobe  0,g6,e2_io100           # Jif no assoc. ILMT
        call    mag$remtask             # remove task ILT from working queue
e2_io100:
        lda     -ILTBIAS(g1),g1         # set up to return ILT to originator
        movq    g4,r4                   # save g4-g7
        ld      il_cr(g1),r3            # r3 = originator's completion
                                        #  handler routine
        callx   (r3)                    # call completion routine
        movq    r4,g4                   # restore g4-g7
        cmpobe  0,g6,e2_io200           # Jif no asoc. ILMT
        call    mag$chknextask          # check if next task needs to be
                                        #  enabled
e2_io200:
        movt    r12,g12                 # restore g12-g14
        ret
#
#******************************************************************************
#
#  NAME: te2_abort
#
#  PURPOSE:
#       Provide the appropriate processing for an abort event for
#       an immediate type command (task_etbl2 task event handler
#       table).
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te2_abort
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te2_abort:
        lda     task_etbl6,r3           # r3 = new task event handler table
        st      r3,inl2_ehand(g1)       # save new task event handler table
                                        #  in task ILT
        call    mag$qtask2aq            # queue task ILT to abort queue
        ret
#
#******************************************************************************
#
#  NAME: te2_reset
#
#  PURPOSE:
#       Provide the appropriate processing for a reset event for
#       an immediate type command (task_etbl2 task event handler
#       table).
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te2_reset
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te2_reset:
        b       te2_abort               # handler same as abort for now
#
#******************************************************************************
#
#  NAME: te2_offline
#
#  PURPOSE:
#       Provide the appropriate processing for an offline event for
#       an immediate type command (task_etbl2 task event handler
#       table).
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te2_offline
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te2_offline:
        b       te2_abort               # handler same as abort for now
#
#******************************************************************************
#
#  NAME: te3_iocomp
#
#  PURPOSE:
#       Provide the processing to perform the task I/O
#       complete processing for READ type commands.
#
#  DESCRIPTION:
#       Returns any resources and then returns the primary task
#       ILT back to the originator.
#
#  CALLING SEQUENCE:
#       call    te3_iocomp
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te3_iocomp:
        b       te2_iocomp
#
#******************************************************************************
#
#  NAME: te3_MAGcomp
#
#  PURPOSE:
#       Perform the processing for a MAGNITUDE request completion
#       event for a READ type command.
#
#  DESCRIPTION:
#       Checks for errors and if any are indicated, returns the
#       appropriate status and SENSE data to the initiator. If
#       no errors are indicated, returns successful status to the
#       initiator.
#
#  CALLING SEQUENCE:
#       call    te3_MAGcomp
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
        .data
rd10_tbl1:
        .byte   dtxfern,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
rd10_tbl2:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
rd10_tbl3:
        .byte   dtxfern,scbusy,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .text
#
te3_MAGcomp:
        lda     0,r3                    # r3 = 0
        mov     g1,g9                   # g9 = pri. ILT at inl2 nest level
        ldob    inl2_ecode(g1),r4       # r4 = local error code for task
        ld      -ILTBIAS+vrvrp(g1),g2   # g2 = assoc. VRP address
        cmpobe  0,g0,e3_MAG02           # Jif good completion status
        cmpobne ecbusy,g0,e3_MAG01
#        c       fprintf(stderr,"[%s:%d]: SCSI BUSY g0=%lX\n", __FILE__, __LINE__, g0);
        ldq     rd10_tbl3,r4            # load op. values into regs.
        ld      rd10_tbl3+16,r8
        b       e3_MAG20

e3_MAG01:
        cmpobe  0,r4,e3_MAG03           # Jif no local error indicated
        mov     r4,g0                   # g0 = error code to process. Local
        b       e3_MAG03

e3_MAG02:
        cmpobe  0,r4,e3_MAG10           # Jif no local error indicated
        mov     r4,g0                   # g0 = error code to process. Local

e3_MAG03:
#
# --- Process error status
#
        ldconst 0x100,r4                # r4 = max. error code value supported
        cmpobg  r4,g0,e3_MAG04          # Jif error code within range
        ldconst 0x00,g0                 # g0 = default error code value
e3_MAG04:
        lda     te3_MAGetbl,r9          # r9 = error code normalization table
        addo    g0,r9,r9                # r9 = pointer to normalized error code
        ldob    (r9),r9                 # r9 = normalized error code
        cmpobne 10,r9,e3_MAG05          # Jif not device reserved code
        ldq     rconflict_tbl1,r4       # load op. values into regs.
        ld      rconflict_tbl1+16,r8    # return reservation conflict status
        b       e3_MAG20                #  to initiator
e3_MAG05:
        shlo    2,r9,r9                 # normalized error code * 4
        lda     te3_MAGetbl2,r10        # r10 = normalized error table pointer
        addo    r9,r10,r10              # r10 = pointer to error table for
                                        #  reported error
        ld      (r10),r7                # r7 = SENSE data address
        ldt     rd10_tbl2,r4            # load op. values into regs.
        ld      rd10_tbl2+16,r8
        b       e3_MAG20
#
e3_MAG10:
        ldq     rd10_tbl1,r4            # load op. values into regs.
        ld      rd10_tbl1+16,r8
e3_MAG20:
        lda     -ILTBIAS(g1),g1         # back up to previous nest level in ILT
        st      r3,vrvrp(g1)            # clear vrvrp field in ILT
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
        mov     g1,g7                   # g7 = assoc. pri. ILT at FC-AL level
        b       mag1$cmdcom             # and send completion status to host
#
#*****************************************************************************
#
#  NAME: te3_MAGetbl
#
#  PURPOSE:
#       Used to normalize an error code returned on a READ type
#       command to determine the proper response to the initiator
#       for the reported error
#
#  DESCRIPTION:
#       Use the error code reported from the READ request to
#       index into this table. The value read from the indexed
#       location is multiplied by 4 and used to index into the
#       te3_MAGetbl2 table to get the address of the parameters
#       to set up to complete the I/O operation.
#       Note: The error code definitions are found in ecodes.inc.
#
#****************************************************************************
#
        .data
te3_MAGetbl:
        .byte   0,0,8,6,5,7,7,7         # 00-07
        .byte   4,4,4,5,5,4,10,0        # 08-0f
        .byte   0,0,0,0,0,0,0,0         # 10-17
        .byte   0,0,0,0,0,0,0,0         # 18-1f
        .byte   0,0,0,0,0,0,0,0         # 20-27
        .byte   0,0,0,0,0,0,0,0         # 28-2f
        .byte   0,0,0,0,0,0,0,0         # 30-37
        .byte   0,0,0,0,0,0,0,0         # 38-3f
        .byte   0,3,9,4,4,1,4,4         # 40-47
        .byte   1,1,1,1,4,2,1,1         # 48-4f
        .byte   4,4,3,4,0,0,0,0         # 50-57
        .byte   0,0,0,0,0,0,0,0         # 58-5f
        .byte   0,0,0,0,0,0,0,0         # 60-67
        .byte   0,0,0,0,0,0,0,0         # 68-6f
        .byte   0,0,0,0,0,0,0,0         # 70-77
        .byte   0,0,0,0,0,0,0,0         # 78-7f
        .byte   2,0,0,0,0,0,0,0         # 80-87
        .byte   0,0,0,0,0,0,0,0         # 88-8f
        .byte   0,0,0,0,0,0,0,0         # 90-97
        .byte   0,0,0,0,0,0,0,0         # 98-9f
        .byte   0,0,0,0,0,0,0,0         # a0-a7
        .byte   0,0,0,0,0,0,0,0         # a8-af
        .byte   0,0,0,0,0,0,0,0         # b0-b7
        .byte   0,0,0,0,0,0,0,0         # b8-bf
        .byte   0,0,0,0,0,0,0,0         # c0-c7
        .byte   0,0,0,0,0,0,0,0         # c8-cf
        .byte   0,0,0,0,0,0,0,0         # d0-d7
        .byte   0,0,0,0,0,0,0,0         # d8-df
        .byte   0,0,0,0,0,0,0,0         # e0-e7
        .byte   0,0,0,0,0,0,0,0         # e8-ef
        .byte   0,0,0,0,0,0,0,0         # f0-f7
        .byte   0,0,0,0,0,0,0,1         # f8-ff
#
#
#*****************************************************************************
#
#  NAME: te3_MAGetbl2
#
#  PURPOSE:
#
#       This table contains the addresses of parameter tables
#       associated with error codes reported for a READ type
#       command.
#
#  DESCRIPTION:
#
#       The normalized error code from the te3_MAGetbl is multiplied
#       by 4 and used to index into this table to extract the address
#       of the parameters table used to report the associated error
#       to the initiator.
#
#****************************************************************************
#
#                                        ASQ/ASCQ Description
te3_MAGetbl2:                           # -- -- -----------------------------
        .word   sense_rerr1             # 11/00-Unrecovered READ error
        .word   sense_err1              # 00/06-I/O process terminated
        .word   sense_err2              # 41/00-Data path failure
        .word   sense_err3              # 3E/02-Timeout on Logical Unit
        .word   sense_err4              # 44/00-Internal target failure
        .word   sense_err5              # 04/03-Logical unit not ready -
                                        #       Manual intervention required
        .word   sense_err6              # 25/00-Logical unit not supported
        .word   sense_err7              # 21/00-Logical block address out of
                                        #       range
        .word   sense_err8              # 22/00-Illegal function
        .word   sense_err9              # 21/01-Invalid element address
        .word   0                       # Reservation conflict
#
        .text
#
#******************************************************************************
#
#  NAME: te3a_MAGcomp
#
#  PURPOSE:
#
#       Perform the processing for a MAGNITUDE request completion
#       event for a READ type command that we've given completion
#       status for already.
#
#  DESCRIPTION:
#
#       Removes ILT from working queue and returns resources. Checks
#       working queue for ILMT to see if any tasks need to be enabled.
#
#  CALLING SEQUENCE:
#
#       call    te3a_MAGcomp
#
#  INPUT:
#
#       g0 = VRP request completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te3a_MAGcomp:
        ld      -ILTBIAS+vrvrp(g1),g2   # g2 = assoc. VRP
        cmpobe  0,g2,e3a_MAG100         # Jif no VRP associated with task
c       record_mag(FR_MAG_READ_COMPLETE, (void *)g2);
        mov     0,r3
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
        st      r3,-ILTBIAS+vrvrp(g1)   # clear vrvrp field in ILT
e3a_MAG100:
        b       te2_iocomp              # process same as if I/O completion
                                        #  for ending status.
#
#******************************************************************************
#
#  NAME: te3_srpreq
#
#  PURPOSE:
#       Perform the processing for a MAGNITUDE SRP request
#       event associated with a READ type command.
#
#  DESCRIPTION:
#       Sets up a request to the FC-AL level and issues the
#       I/O request to the FC-AL driver.
#
#  CALLING SEQUENCE:
#       call    te3_srpreq
#
#  INPUT:
#       g1 = sec. ILT at otl2 nest level
#       g2 = SRP address
#       g6 = assoc. ILMT address
#       g7 = pri. ILT at inl1 nest level
#       g9 = pri. ILT at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
        .data
read10_tbl5:
        .byte   dtxferi,0,0,0
#
        .text
#
te3_srpreq:
        movt    g12,r12                 # save g12-g14
        ldob    sr_func(g2),r4          # r4 = SRP function code
        cmpobe  src2h,r4,e3_srp10       # Jif function code OK
        ldconst ecinvfunc,g0            # return SRP with error
        ldconst inl2_ps_srpcomp,r3      # r3 = task process state code
        stob    g0,inl2_ecode(g9)       # save error code in inl2 data structure
        stob    r3,inl2_pstate(g9)      # save new task process state code
        lda     -ILTBIAS(g1),g1         # back up to previous nest level
        ld      otl1_cr(g1),r4          # r4 = MAG interface completion routine
        callx   (r4)                    # and call it.
        b       e3_srp1000              # and we're done
#
e3_srp10:
#
# --- Check for completion code error and process
#
        ldob    -ILTBIAS+otl1_cmpcode(g1),r4 # r4 = completion code
        ld      -ILTBIAS+otl1_relofset(g1),r5 # r5 = relative offset
        cmpobe  0,r4,e3_srp10a          # Jif no error indicated
        movl    g0,r6                   # save g0-g1
        mov     r4,g0                   # g0 = SRP error status
        mov     g9,g1                   # g1 = pri. ILT at inl2 nest level
        call    mag$mmcMAGerr           # report error to MMC
        movl    r6,g0                   # restore g0-g1
        ldob    inl2_ecode(g9),r8       # r8 = current inl2 error code
        cmpobne 0,r8,e3_srp10a          # Jif error code already saved
        stob    r4,inl2_ecode(g9)       # save completion code in error code field
#
e3_srp10a:
        ld      read10_tbl5,r4          # r4 = I/O op. setup
        ld      sr_count(g2),r8         # r8 = SGL descriptor count
        lda     sr_desc0(g2),g3         # g3 = SRP SGL pointer
        mov     0,r7                    # r7 = sense data pointer (null)
        mov     g3,r6                   # r6 = SGL pointer for I/O operation
        mov     r8,g5                   # g5 = SGL descriptor count
        mov     g3,g4                   # g4 = pointer to SRP SGL records
#
# --- Build I/O SGL list from SRP SGL
#
e3_srp40:
        ld      sr_dest(g4),r10         # r10 = source address of segment
        ld      sr_len(g4),r11          # r11 = segment length
        subo    1,g5,g5                 # dec. segment count
        lda     srpesiz(g4),g4          # inc. to next SRP SGL record
        stl     r10,sg_addr(g3)         # save in I/O SGL
        lda     sgdescsiz(g3),g3        # inc. to next I/O SGL record
        cmpobne 0,g5,e3_srp40           # Jif more segments to translate
        stq     r4,xlcommand(g1)        # save I/O param. block in ILT
        mov     g7,r9                   # r9 = pri. ILT param. structure
        mov     g9,r10                  # r10 = pri. ILT at inl2 nest level
        stt     r8,xlsgllen(g1)
        mov     inl2_ps_srpact,r3       # r3 = new task process state code
        lda     mag1_srpcomp,r5         # r5 = my completion handler routine
        mov     g1,r10                  # r10 = sec. ILT at otl2 nest level
        stob    r3,inl2_pstate(g9)      # save new process state code
        st      r5,otl2_cr(g1)          # save completion handler in ILT
        call    mag$add2dtlen           # add to data transfer length
#
        lda     ILTBIAS(g1),g1          # bump to next ILT nest level
        st      r10,otl3_OTL2(g1)       # save pointer to I/O param. block
#
.ifdef TRACES
        call    mag$tr_ISP$receive_io   # trace event
.endif # TRACES
#
        call    ISP$receive_io          # and issue I/O request to FC-AL driver
e3_srp1000:
        movt    r12,g12                 # restore g12-g14
        ret
#
#******************************************************************************
#
#  NAME: te3_srpcomp
#
#  PURPOSE:
#       Processes a FC-AL I/O request completion event associated
#       with a MAGNITUDE SRP request associated with a READ type
#       command.
#
#  DESCRIPTION:
#       Checks for errors and if any occurred returns the appropriate
#       error code to the requesting MAGNITUDE level. Else returns the
#       requested data to the MAGNITUDE driver.
#
#  CALLING SEQUENCE:
#       call    te3_srpcomp
#
#  INPUT:
#       g0 = SRP I/O completion status
#       g1 = sec. SRP ILT at olt2 nest level
#       g6 = assoc. ILMT address
#       g9 = pri. ILT at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te3_srpcomp:
#
# --- Check for I/O error reported from FC-AL driver
#
        cmpobe  0,g0,e3srpcomp_10       # Jif no error indicated
        ldconst ec_ioerr,r3             # r3 = my local error code
        stob    r3,inl2_ecode(g9)       # save local error code in inl2
#
# --- Simulate abort on task
#
        mov     g0,r15                  # save g0
        ld      -ILTBIAS+scrxid(g9),g0  # g0 = exchange ID of task to abort
        ld      ilm_cimt(g6),g4         # g4 = assoc. CIMT address
        ld      ilm_imt(g6),g5          # g5 = assoc. IMT address
        call    mag$abort_task          # abort task processing
        mov     r15,g0                  # restore g0
        b       e3srpcomp_30

e3srpcomp_10:
        ldob    inl2_ecode(g9),g0       # g0 = previous error code if error
                                        #  has occurred
e3srpcomp_30:
        movt    g12,r12                 # save g12-g14
        lda     -ILTBIAS(g1),g1         # back up to previous nest level in ILT
        ldconst inl2_ps_srpcomp,r3      # r3 = new task process state code
        ld      otl1_cr(g1),r4          # get completion handler routine from ILT
        stob    r3,inl2_pstate(g9)      # save new task process state
        callx   (r4)                    # and go to it
        movt    r12,g12                 # restore g12-g14
        ret
#
#******************************************************************************
#
#  NAME: te3_abort
#
#  PURPOSE:
#       Provide the appropriate processing for an abort event for
#       a READ type command to the MAGNITUDE (task_etbl3 task
#       event handler table).
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te3_abort
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te3_abort:
        b       te2_abort               # handle the same for now
#
#******************************************************************************
#
#  NAME: te3_reset
#
#  PURPOSE:
#       Provide the appropriate processing for a reset event for
#       a READ type command to the MAGNITUDE (task_etbl3 task
#       event handler table).
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te3_reset
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te3_reset:
        b       te2_abort               # handle the same for now
#
#******************************************************************************
#
#  NAME: te3_offline
#
#  PURPOSE:
#       Provide the appropriate processing for an offline event for
#       a READ type command to the MAGNITUDE (task_etbl3 task event
#       handler table).
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te3_offline
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te3_offline:
        b       te2_abort               # handle the same for now
#
#******************************************************************************
#
#  NAME: te3_aca
#
#  PURPOSE:
#       Provide the appropriate processing for an ACA event for
#       a READ type command to the MAGNITUDE (task_etbl3 task
#       event handler table).
#
#  DESCRIPTION:
#       Replaces the task event handler table to the one which
#       is to be used for an ACA event occurring. SRP requests
#       and completion responses are delayed until the ACA condition
#       has been cleared or the task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te3_aca
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te3_aca:
        ld      inl2_ehand(g1),r15      # r15 = current task event handler
                                        #  table
        lda     task_etbl5,r14          # r14 = new task event handler table
        st      r15,inl2_ehand2(g1)     # save current task event handler
                                        #  table in ILT
        st      r14,inl2_ehand(g1)      # save new current task event handler
                                        #  table
        ret
#
#******************************************************************************
#
#  NAME: te4_iocomp
#
#  PURPOSE:
#       Provide the processing to perform the task I/O
#       complete processing for WRITE type commands.
#
#  DESCRIPTION:
#       Returns any resources and then returns the primary task
#       ILT back to the originator.
#
#  CALLING SEQUENCE:
#       call    te4_iocomp
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te4_iocomp:
        b       te2_iocomp
#
#******************************************************************************
#
#  NAME: te4_MAGcomp
#
#  PURPOSE:
#       Perform the processing for a MAGNITUDE request completion
#       event for a WRITE type command.
#
#  DESCRIPTION:
#       Checks for errors and if any are indicated, returns the
#       appropriate status and SENSE data to the initiator. If
#       no errors are indicated, returns successful status to the
#       initiator.
#
#  CALLING SEQUENCE:
#       call    te4_MAGcomp
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
        .data
wrt10_tbl1:
        .byte   dtxfern,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
wrt10_tbl2:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
wrt10_tbl3:
        .byte   dtxfern,scbusy,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
#
        .text
#
te4_MAGcomp:
        lda     0,r3                    # r3 = 0
        mov     g1,g9                   # g9 = pri. ILT at inl2 nest level
        ldob    inl2_ecode(g1),r4       # r4 = local error code for task
        ld      -ILTBIAS+vrvrp(g1),g2   # g2 = assoc. VRP address
c       record_mag(FR_MAG_WRITE_COMPLETE, (void *)g2);
        cmpobe  0,g0,e4_MAG02           # Jif good completion status
        cmpobne ecbusy,g0,e4_MAG01
#        c       fprintf(stderr,"[%s:%d]: WRITE CMD SCSI BUSY g0=%lX\n", __FILE__, __LINE__, g0);
        ldq     wrt10_tbl3,r4           # load op. values into regs.
        ld      wrt10_tbl3+16,r8
        b       e4_MAG20

e4_MAG01:
        cmpobe  0,r4,e4_MAG03           # Jif no local error indicated
        mov     r4,g0                   # g0 = error code to process. Local
        b       e4_MAG03

e4_MAG02:
        cmpobe  0,r4,e4_MAG10           # Jif no local error indicated
        mov     r4,g0                   # g0 = error code to process. Local

e4_MAG03:
#
# --- Process error status
#
        ldconst 0x100,r4                # r4 = max. error code value supported
        cmpobg  r4,g0,e4_MAG04          # Jif error code within range
        ldconst 0x00,g0                 # g0 = default error code value
e4_MAG04:
        lda     te4_MAGetbl,r9          # r9 = error code normalization table
        addo    g0,r9,r9                # r9 = pointer to normalized error code
        ldob    (r9),r9                 # r9 = normalized error code
        cmpobne 10,r9,e4_MAG05          # Jif not device reserved code
        ldq     rconflict_tbl1,r4       # load op. values into regs.
        ld      rconflict_tbl1+16,r8    # return reservation conflict status
        b       e4_MAG20                #  to initiator
e4_MAG05:
        shlo    2,r9,r9                 # normalized error code * 4
        lda     te4_MAGetbl2,r10        # r10 = normalized error table pointer
        addo    r9,r10,r10              # r10 = pointer to error table for
                                        #  reported error
        ld      (r10),r7                # r7 = SENSE data address
        ldt     wrt10_tbl2,r4           # load op. values into regs.
        ld      wrt10_tbl2+16,r8
        b       e4_MAG20

e4_MAG10:
        ldq     wrt10_tbl1,r4           # load op. values into regs.
        ld      wrt10_tbl1+16,r8
e4_MAG20:
        lda     -ILTBIAS(g1),g1         # back up to previous nest level in ILT
        st      r3,vrvrp(g1)            # clear vrvrp field in ILT
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
        mov     g1,g7                   # g7 = assoc. pri. ILT at FC-AL level
        b       mag1$cmdcom             # and send completion status to host
#
#*****************************************************************************
#
#  NAME: te4_MAGetbl
#
#  PURPOSE:
#       Used to normalize an error code returned on a WRITE type
#       command to determine the proper response to the initiator
#       for the reported error.
#
#  DESCRIPTION:
#       Use the error code reported from the WRITE request to
#       index into this table. The value read from the indexed
#       location is multiplied by 4 and used to index into the
#       te4_MAGetbl2 table to get the address of the parameters
#       to set up to complete the I/O operation.
#       Note: The error code definitions are found in ecodes.inc.
#
#****************************************************************************
#
        .data
te4_MAGetbl:
        .byte   0,0,8,6,5,7,7,7         # 00-07
        .byte   4,4,4,5,5,4,10,0        # 08-0f
        .byte   0,0,0,0,0,0,0,0         # 10-17
        .byte   0,0,0,0,0,0,0,0         # 18-1f
        .byte   0,0,0,0,0,0,0,0         # 20-27
        .byte   0,0,0,0,0,0,0,0         # 28-2f
        .byte   0,0,0,0,0,0,0,0         # 30-37
        .byte   0,0,0,0,0,0,0,0         # 38-3f
        .byte   0,3,9,4,4,1,4,4         # 40-47
        .byte   1,1,1,1,4,2,1,1         # 48-4f
        .byte   4,4,3,4,0,0,0,0         # 50-57
        .byte   0,0,0,0,0,0,0,0         # 58-5f
        .byte   0,0,0,0,0,0,0,0         # 60-67
        .byte   0,0,0,0,0,0,0,0         # 68-6f
        .byte   0,0,0,0,0,0,0,0         # 70-77
        .byte   0,0,0,0,0,0,0,0         # 78-7f
        .byte   2,0,0,0,0,0,0,0         # 80-87
        .byte   0,0,0,0,0,0,0,0         # 88-8f
        .byte   0,0,0,0,0,0,0,0         # 90-97
        .byte   0,0,0,0,0,0,0,0         # 98-9f
        .byte   0,0,0,0,0,0,0,0         # a0-a7
        .byte   0,0,0,0,0,0,0,0         # a8-af
        .byte   0,0,0,0,0,0,0,0         # b0-b7
        .byte   0,0,0,0,0,0,0,0         # b8-bf
        .byte   0,0,0,0,0,0,0,0         # c0-c7
        .byte   0,0,0,0,0,0,0,0         # c8-cf
        .byte   0,0,0,0,0,0,0,0         # d0-d7
        .byte   0,0,0,0,0,0,0,0         # d8-df
        .byte   0,0,0,0,0,0,0,0         # e0-e7
        .byte   0,0,0,0,0,0,0,0         # e8-ef
        .byte   0,0,0,0,0,0,0,0         # f0-f7
        .byte   0,0,0,0,0,0,0,1         # f8-ff
#
#
#*****************************************************************************
#
#  NAME: te4_MAGetbl2
#
#  PURPOSE:
#       This table contains the addresses of parameter tables
#       associated with error codes reported for a WRITE type
#       command.
#
#  DESCRIPTION:
#       The normalized error code from the te4_MAGetbl is multiplied
#       by 4 and used to index into this table to extract the address
#       of the parameters table used to report the associated error
#       to the initiator.
#
#****************************************************************************
#
#                                        ASQ/ASCQ Description
te4_MAGetbl2:                           # -- -- -----------------------------
        .word   sense_werr1             # 0C/02-Write error-Auto reallocation
                                        #       failed
        .word   sense_err1              # 00/06-I/O process terminated
        .word   sense_err2              # 41/00-Data path failure
        .word   sense_err3              # 3E/02-Timeout on Logical Unit
        .word   sense_err4              # 44/00-Internal target failure
        .word   sense_err5              # 04/03-Logical unit not ready -
                                        #       Manual intervention required
        .word   sense_err6              # 25/00-Logical unit not supported
        .word   sense_err7              # 21/00-Logical block address out of
                                        #       range
        .word   sense_err8              # 22/00-Illegal function
        .word   sense_err9              # 21/01-Invalid element address
        .word   0                       # Reservation conflict
#
#******************************************************************************
#
#  NAME: te4_srpreq
#
#  PURPOSE:
#       Perform the processing for a MAGNITUDE SRP request
#       event associated with a WRITE type command.
#
#  DESCRIPTION:
#       Sets up a request to the FC-AL level and issues the
#       I/O request to the FC-AL driver.
#
#  CALLING SEQUENCE:
#       call    te4_srpreq
#
#  INPUT:
#       g1 = sec. ILT at otl2 nest level
#       g2 = SRP address
#       g6 = assoc. ILMT address
#       g7 = pri. ILT at inl1 nest level
#       g9 = pri. ILT at XL nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
write10_tbl5:
        .byte   dtxferc,scnorm,0,0
#
        .text
#
te4_srpreq:
        movt    g12,r12                 # save g12-g14
        ldob    sr_func(g2),r4          # r4 = SRP function code
        cmpobe  srh2c,r4,e4_srp10       # Jif function code OK
        ldconst ecinvfunc,g0            # return SRP with error
        ldconst inl2_ps_srpcomp,r3      # r3 = new task process state code
        stob    g0,inl2_ecode(g9)       # save error code in inl2 data structure
        stob    r3,inl2_pstate(g9)      # save new task process state code
        lda     -ILTBIAS(g1),g1         # back up to previous nest level
        ld      otl1_cr(g1),r4          # r4 = MAG interface completion routine
        callx   (r4)                    # and call it.
        b       e4_srp1000              # and we're out of here

e4_srp10:
        ld      write10_tbl5,r4         # r4 = I/O op. setup
        ld      sr_count(g2),r8         # r8 = SGL descriptor count
        lda     sr_desc0(g2),g3         # g3 = SRP SGL pointer
        mov     0,r7                    # r7 = sense data pointer (null)
        mov     g3,r6                   # r6 = SGL pointer for I/O operation
        ld      -ILTBIAS+otl1_relofset(g1),r5 # r5 = relative offset
        mov     r8,g5                   # g5 = SGL descriptor count
        mov     g3,g4                   # g4 = pointer to SRP SGL records
#
# --- Build I/O SGL list from SRP SGL
#
e4_srp40:
        ld      sr_dest(g4),r10         # r10 = destination address of segment
        ld      sr_len(g4),r11          # r11 = segment length
        subo    1,g5,g5                 # dec. segment count
        lda     srpesiz(g4),g4          # inc. to next SRP SGL record
        stl     r10,sg_addr(g3)         # save in I/O SGL
        lda     sgdescsiz(g3),g3        # inc. to next I/O SGL record
        cmpobne 0,g5,e4_srp40           # Jif more segments to translate
        stq     r4,xlcommand(g1)        # save I/O param. block in ILT
        mov     g7,r9                   # r9 = pri. ILT param. structure
        mov     g9,r10                  # r10 = pri. ILT at inl2 nest level
        stt     r8,xlsgllen(g1)
        mov     inl2_ps_srpact,r3       # r3 = new task process state code
        lda     mag1_srpcomp,r5         # r5 = my completion handler routine
        mov     g1,r10                  # r10 = sec. ILT at otl2 nest level
        stob    r3,inl2_pstate(g9)      # save new process state code
        st      r5,otl2_cr(g1)          # save completion handler in ILT
        call    mag$add2dtlen           # add to data transfer length
#
        lda     ILTBIAS(g1),g1          # bump to next ILT nest level
        st      r10,otl3_OTL2(g1)       # save pointer to I/O param. block
#
.ifdef TRACES
        call    mag$tr_ISP$receive_io   # trace event
.endif # TRACES
#
        call    ISP$receive_io          # and issue I/O request to FC-AL driver
e4_srp1000:
        movt    r12,g12                 # restore g12-g14
        ret
#
#******************************************************************************
#
#  NAME: te4_srpcomp
#
#  PURPOSE:
#       Processes a FC-AL I/O request completion event associated
#       with a MAGNITUDE SRP request associated with a WRITE type
#       command.
#
#  DESCRIPTION:
#       Checks for errors and if any occurred returns the appropriate
#       error code to the requesting MAGNITUDE level. Else returns the
#       requested data to the MAGNITUDE driver.
#
#  CALLING SEQUENCE:
#       call    te4_srpcomp
#
#  INPUT:
#       g0 = SRP I/O completion status
#       g1 = sec. SRP ILT address at OLT2 nest level
#       g6 = assoc. ILMT address
#       g9 = pri. ILT at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te4_srpcomp:
#
# --- Check for I/O error reported from FC-AL driver
#
        cmpobe  0,g0,e4srpcomp_10       # Jif no error indicated
        ldconst ec_ioerr,r3             # r3 = my local error code
        stob    r3,inl2_ecode(g9)       # save local error code in inl2
#
# --- Simulate abort on task
#
        mov     g0,r15                  # save g0
        ld      -ILTBIAS+scrxid(g9),g0  # g0 = exchange ID of task to abort
        ld      ilm_cimt(g6),g4         # g4 = assoc. CIMT address
        ld      ilm_imt(g6),g5          # g5 = assoc. IMT address
        call    mag$abort_task          # abort task processing
        mov     r15,g0                  # restore g0
e4srpcomp_10:
        ldconst inl2_ps_srpcomp,r3      # r3 = new task process state code
        lda     -ILTBIAS(g1),g1         # back up to previous nest level in ILT
        ld      otl1_cr(g1),r4          # get completion handler routine from ILT
        movt    g12,r12                 # save g12-g14
        stob    r3,inl2_pstate(g9)      # save new task process state
        callx   (r4)                    # and go to it
        movt    r12,g12                 # restore g12-g14
        ret
#
#******************************************************************************
#
#  NAME: te4_abort
#
#  PURPOSE:
#       Provide the appropriate processing for an abort event for
#       a WRITE type command to the MAGNITUDE (task_etbl4 task
#       event handler table).
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te4_abort
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te4_abort:
        b       te2_abort               # handle the same for now
#
#******************************************************************************
#
#  NAME: te4_reset
#
#  PURPOSE:
#       Provide the appropriate processing for a reset event for
#       a WRITE type command to the MAGNITUDE (task_etbl4 task
#       event handler table).
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te4_reset
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te4_reset:
        b       te2_abort               # handle the same for now
#
#******************************************************************************
#
#  NAME: te4_offline
#
#  PURPOSE:
#       Provide the appropriate processing for an offline event for
#       a WRITE type command to the MAGNITUDE (task_etbl4 task event
#       handler table).
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te4_offline
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te4_offline:
        b       te2_abort               # handle the same for now
#
#******************************************************************************
#
#  NAME: te4_aca
#
#  PURPOSE:
#       Provide the appropriate processing for an ACA event for
#       a WRITE type command to the MAGNITUDE (task_etbl4 task
#       event handler table).
#
#  DESCRIPTION:
#       Replaces the task event handler table to the one which
#       is to be used for an ACA event occurring. SRP requests
#       and completion responses are delayed until the ACA condition
#       has been cleared or the task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te4_aca
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te4_aca:
        b       te3_aca                 # handler same as for READ commands
#
#******************************************************************************
#
#  NAME: te5_iocomp
#
#  PURPOSE:
#       Provide the processing to perform the task I/O
#       complete processing for blocked commands.
#
#  DESCRIPTION:
#       Returns any resources and then returns the primary task
#       ILT back to the originator.
#
#  CALLING SEQUENCE:
#       call    te5_iocomp
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te5_iocomp:
        b       te2_iocomp
#
#******************************************************************************
#
#  NAME: te5_MAGcomp
#
#  PURPOSE:
#       Perform the processing for a MAGNITUDE request completion
#       event for a blocked task.
#
#  DESCRIPTION:
#       Saves the completion status in the inl2_ecode field of the
#       task's ILT and sets the task event handler table to one which
#       can resume sending the appropriate completion status to the
#       initiator when the ACA clear event occurs.
#
#  CALLING SEQUENCE:
#       call    te5_MAGcomp
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te5_MAGcomp:
        lda     task_etbl5b,r15         # r15 = new task event handler table
        ldob    inl2_ecode(g1),r14      # r14 = error code in inl2 data
                                        #  structure
        st      r15,inl2_ehand(g1)      # save new task event handler table
                                        #  in task's pri. ILT
        cmpobne 0,r14,e5_MAG10          # Jif error code already stored in
                                        # inl2_ecode field
        stob    g0,inl2_ecode(g1)       # save status in inl2_ecode for later
                                        #  completion processing
e5_MAG10:
        ret
#
#******************************************************************************
#
#  NAME: te5_srpreq
#
#  PURPOSE:
#       Perform the processing for a MAGNITUDE SRP request
#       event associated with a task that is in the blocked
#       state.
#
#  DESCRIPTION:
#       Queues the SRP request up to the blocked queue in the
#       associated ILMT and changes the task event handler table
#       to one which will process the queued SRP request at the
#       appropriate event occurrence.
#
#  CALLING SEQUENCE:
#       call    te5_srpreq
#
#  INPUT:
#       g1 = sec. ILT at otl2 nest level
#       g2 = SRP address
#       g6 = assoc. ILMT address
#       g7 = pri. ILT at inl1 nest level
#       g9 = pri. ILT at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te5_srpreq:
#
# --- Set up secondary ILT to be able to continue processing
#       at a later point in time and queue the secondary ILT
#       to the blocked ILT queue in the ILMT.
#
        st      g2,il_w0(g1)            # w0 = SRP address
        st      g7,il_w1(g1)            # w1 = pri. ILT at inl1 nest level
        st      g9,il_w2(g1)            # w2 = pri. ILT at inl2 nest level
        mov     0,r14                   # r14 = 0
        st      r14,il_fthd(g1)         # clear forward thread field in sec.
                                        #  ILT
        ld      ilm_btail(g6),r13       # r13 = last ILT on blocked queue
        cmpobe  0,r13,e5_srp10          # Jif queue empty
        st      g1,ilm_bhead(g6)        # save ILT as head element
        b       e5_srp20
#
e5_srp10:
        st      g1,il_fthd(r13)         # link new ILT onto end of list
e5_srp20:
        st      g1,ilm_btail(g6)        # save new ILT as new tail element
        lda     task_etbl5a,r15         # r15 = new task event handler table
        st      r15,inl2_ehand(g9)      # save new task event handler table
                                        #  in pri. task ILT
        ret
#
#******************************************************************************
#
#  NAME: te5_srpcomp
#
#  PURPOSE:
#       Processes a FC-AL I/O request completion event associated
#       with a MAGNITUDE SRP request associated with a blocked task.
#
#  DESCRIPTION:
#       Checks for errors and if any occurred returns the appropriate
#       error code to the requesting MAGNITUDE level. Else returns the
#       requested data to the MAGNITUDE driver. Note that if an SRP
#       completion event occurs on a blocked task, that the operation
#       proceeds for the time being.
#
#  CALLING SEQUENCE:
#       call    te5_srpcomp
#
#  INPUT:
#       g0 = SRP I/O completion status
#       g1 = sec. SRP ILT address at OLT2 nest level
#       g6 = assoc. ILMT address
#       g9 = pri. ILT at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te5_srpcomp:
        b       te4_srpcomp             # handler the same for now.
#
#******************************************************************************
#
#  NAME: te5_abort
#
#  PURPOSE:
#       Provide the appropriate processing for an abort event
#       when a task is in the blocked task state.
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted. It also checks the
#       blocked queue for any SRP operation that was blocked
#       and returns an error to the MAGNITUDE.
#
#  CALLING SEQUENCE:
#       call    te5_abort
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te5_abort:
        b       te2_abort               # handle the same for now
#
#******************************************************************************
#
#  NAME: te5a_abort
#
#  PURPOSE:
#       Provide the appropriate processing for an abort event
#       when a task is in the blocked task state and an associated
#       SRP request ILT is queued on the blocked ILT queue in the
#       associated ILMT.
#
#  DESCRIPTION:
#       Finds and removes the associated SRP ILT from the blocked
#       queue and returns it along with an error indication to
#       the MAGNITUDE indicating the SRP request has been aborted.
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te5a_abort
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te5a_abort:
        movq    g0,r12                  # save g0-g3
        ld      inl2_ehand2(g1),r3      # r3 = original task event handler
                                        #  table
        movq    g4,r4                   # save g4-g7
        movq    g8,r8                   # save g8-g11
        mov     g1,g9                   # g9 = pri. ILT at inl2 nest level
        call    mag$findblkilt          # find assoc. SRP ILT on blocked
                                        #  queue and remove it.
        ld      inl2_eh_abort(r3),r3    # r3 = abort event handler routine
                                        #  from original task event handler
                                        #  table
        cmpobe  0,g1,e5a_abt100         # Jif SRP ILT not found on queue
        ld      il_w0(g1),g2            # g2 = SRP address
        ld      il_w1(g1),g7            # g7 = pri. ILT at inl1 nest level
        call    te6_srpreq              # simulate a SRP request for an aborted
                                        #  task
e5a_abt100:
        movq    r12,g0                  # restore g0-g3
        movq    r4,g4                   # restore g4-g7
        movq    r8,g8                   # restore g8-g11
        bx      (r3)                    # and finish processing the abort event
#
#******************************************************************************
#
#  NAME: te5_reset
#
#  PURPOSE:
#       Provide the appropriate processing for a reset event when
#       a task is in the blocked state.
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te5_reset
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te5_reset:
        b       te2_abort               # handle the same for now
#
#******************************************************************************
#
#  NAME: te5a_reset
#
#  PURPOSE:
#       Provide the appropriate processing for a reset event when
#       a task is in the blocked state and an associated
#       SRP request ILT is queued on the blocked ILT queue in the
#       associated ILMT.
#
#  DESCRIPTION:
#       Finds and removes the associated SRP ILT from the blocked
#       queue and returns it along with an error indication to
#       the MAGNITUDE indicating the SRP request has been aborted.
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te5a_reset
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te5a_reset:
        b       te5a_abort              # handle the same for now
#
#******************************************************************************
#
#  NAME: te5_offline
#
#  PURPOSE:
#       Provide the appropriate processing for an offline event when
#       a task is in the blocked state.
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te5_offline
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te5_offline:
        b       te2_abort               # handle the same for now.
#
#******************************************************************************
#
#  NAME: te5a_offline
#
#  PURPOSE:
#       Provide the appropriate processing for an offline event when
#       a task is in the blocked state and an associated
#       SRP request ILT is queued on the blocked ILT queue in the
#       associated ILMT.
#
#  DESCRIPTION:
#       Finds and removes the associated SRP ILT from the blocked
#       queue and returns it along with an error indication to
#       the MAGNITUDE indicating the SRP request has been aborted.
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te5a_offline
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te5a_offline:
        b       te5a_abort              # handle the same for now.
#
#******************************************************************************
#
#  NAME: te5_acaclr
#
#  PURPOSE:
#       Provide the appropriate processing for an ACA cleared event
#       when a task is in the blocked state.
#
#  DESCRIPTION:
#       Replaces the original task event handler table as the
#       current task event handler table and then returns to
#       caller. If this routine is called, the associated task
#       was blocked and is now being re-enabled without any
#       events occurring which would have blocked the processing
#       of the associated task.
#
#  CALLING SEQUENCE:
#       call    te5_acaclr
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te5_acaclr:
        ld      inl2_ehand2(g1),r15     # r15 = original task event handler
                                        #  table
        st      r15,inl2_ehand(g1)      # save original task event handler
                                        #  table in task ILT
        ret                             # and simply return to caller.
#
#******************************************************************************
#
#  NAME: te5a_acaclr
#
#  PURPOSE:
#       Provide the appropriate processing for an ACA cleared event
#       when a task is in the blocked state and an associated
#       SRP request ILT is queued on the blocked ILT queue in the
#       associated ILMT.
#
#  DESCRIPTION:
#       Finds the associated SRP ILT on the blocked queue in the
#       associated ILMT and sets up to issue the I/O request to
#       the channel interface driver. It also puts the original
#       task event handler table back into effect.
#
#  CALLING SEQUENCE:
#       call    te5a_acaclr
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te5a_acaclr:
        movq    g0,r12                  # save g0-g3
        ld      inl2_ehand2(g1),r3      # r3 = original task event handler
                                        #  table
        movq    g4,r4                   # save g4-g7
        movq    g8,r8                   # save g8-g11
        mov     g1,g9                   # g9 = pri. ILT at inl2 nest level
        call    mag$findblkilt          # find assoc. SRP ILT on blocked
                                        #  queue and remove it.
        st      r3,inl2_ehand(g9)       # save original task event handler
                                        #  table in task ILT
        cmpobe  0,g1,e5a_acaclr100      # Jif SRP ILT not found on queue
        ld      inl2_eh_srpreq(r3),r3   # r3 = original SRP request event
                                        #  handler routine address
        ld      il_w0(g1),g2            # g2 = SRP address
        ld      il_w1(g1),g7            # g7 = pri. ILT at inl1 nest level
        callx   (r3)                    # simulate a SRP request for the task
e5a_acaclr100:
        movq    r12,g0                  # restore g0-g3
        movq    r4,g4                   # restore g4-g7
        movq    r8,g8                   # restore g8-g11
        ret                             # and we're done
#
#******************************************************************************
#
#  NAME: te5b_acaclr
#
#  PURPOSE:
#       Provide the appropriate processing for an ACA cleared event
#       when a task is in the blocked state and the MAGNITUDE completion
#       event has been received.
#
#  DESCRIPTION:
#       Sets up to issue the I/O completion request for the associated
#       task. It also places the original task event handler table
#       into effect.
#
#  CALLING SEQUENCE:
#       call    te5b_acaclr
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te5b_acaclr:
        movq    g0,r12                  # save g0-g3
        ld      inl2_ehand2(g1),r3      # r3 = original task event handler
                                        #  table
        movq    g4,r4                   # save g4-g7
        movq    g8,r8                   # save g8-g11
        st      r3,inl2_ehand(g1)       # save original task event handler
                                        #  table in task ILT
        ld      inl2_eh_magcomp(r3),r3  # r3 = original MAGNITUDE completion
                                        #  event handler routine address
        ldob    inl2_ecode(g1),g0       # g0 = MAGNITUDE completion status code
        callx   (r3)                    # simulate a MAGNITUDE completion event
                                        #  for the task
        movq    r12,g0                  # restore g0-g3
        movq    r4,g4                   # restore g4-g7
        movq    r8,g8                   # restore g8-g11
        ret                             # and we're done
#
#******************************************************************************
#
#  NAME: te6_iocomp
#
#  PURPOSE:
#       Perform the processing for an I/O request completion
#       event for an aborted task.
#
#  DESCRIPTION:
#       Returns any resources allocated for the task and then
#       returns the primary task ILT to the originator.
#
#  CALLING SEQUENCE:
#       call    te6_iocomp
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address ( 0 if no assoc. ILMT)
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te6_iocomp:
        call    mag$remtask             # remove task ILT from abort queue
        b       te1_abort               # and clean up task
#
#******************************************************************************
#
#  NAME: te6_MAGcomp
#
#  PURPOSE:
#       Perform the processing for a MAGNITUDE request completion
#       event for an aborted task.
#
#  DESCRIPTION:
#       Returns any resources allocated for the task and then
#       returns the primary task ILT to the originator.
#
#  CALLING SEQUENCE:
#       call    te6_MAGcomp
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te6_MAGcomp:
        ld      -ILTBIAS+vrvrp(g1),g2   # g2 = assoc. VRP
        cmpobe  0,g2,e6_MAG100          # Jif no VRP associated with task
        mov     0,r3
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
        st      r3,-ILTBIAS+vrvrp(g1)   # clear vrvrp field in ILT
e6_MAG100:
        call    mag$remtask             # remove task ILT from abort queue
        b       te1_abort               # and clean up task
#
#******************************************************************************
#
#  NAME: te6_srpreq
#
#  PURPOSE:
#       Performs the processing for a SRP request event occurring
#       for a task that has been aborted.
#
#  DESCRIPTION:
#       Returns an error back to the MAGNITUDE driver indicating
#       that the associated task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te6_srpreq
#
#  INPUT:
#       g1 = sec. ILT at otl2 nest level
#       g2 = SRP address
#       g6 = assoc. ILMT address
#       g7 = pri. ILT at inl1 nest level
#       g9 = pri. ILT at XL nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te6_srpreq:
        ldconst ec_abort,g0             # g0 = SRP status back to MAG driver
        movt    g12,r12                 # save g12-g14
        lda     -ILTBIAS(g1),g1         # g1 = sec. ILT at otl1 nest level
        ldconst inl2_ps_srpcomp,r4      # r4 = new task process state code
        ld      otl1_cr(g1),r3          # r3 = originator's completion routine
        stob    r4,inl2_pstate(g9)      # save new task process state code
        callx   (r3)                    # and go to it
        movt    r12,g12                 # restore g12-g14
        ret
#
#******************************************************************************
#
#  NAME: te6_srpcomp
#
#  PURPOSE:
#       Processes a FC-AL I/O request completion event for a task
#       that has been aborted.
#
#  DESCRIPTION:
#       Returns an error code to the MAGNITUDE driver indicating
#       the associated task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te6_srpcomp
#
#  INPUT:
#       g0 = SRP I/O completion status
#       g1 = sec. ILT of SRP at the otl2 nest level
#       g6 = assoc. ILMT address
#       g9 = pri. ILT at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te6_srpcomp:
        lda     -ILTBIAS(g1),g1         # back up to previous nest level in ILT
        ldconst inl2_ps_srpcomp,r3      # r3 = new task process state code
        ld      otl1_srp(g1),g2         # g2 = assoc. SRP address
        ld      otl1_cr(g1),r4          # get completion handler routine from ILT
        ldconst ec_ioerr,g0             # g0 = SRP status
        movt    g12,r12                 # save g12-g14
        stob    r3,inl2_pstate(g9)      # save new task process state
        callx   (r4)                    # and go to it
        movt    r12,g12                 # restore g12-g14
        ret
#
#******************************************************************************
#
#  NAME: te7_iocomp
#
#  PURPOSE:
#       Provide the processing to perform the task I/O
#       complete processing for commands which are processed
#       while flag bits are set in ilm_flag2 byte.
#
#  DESCRIPTION:
#       Returns any resources, clears the associated flag bit in
#       ilm_flag2 byte if the I/O operation completed successfully
#       and then returns the primary task ILT back to the originator.
#
#  CALLING SEQUENCE:
#       call    te7_iocomp
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te7_iocomp:
        movt    g12,r12                 # save g12-g14
        cmpobne 0,g0,e7_io100           # Jif I/O operation not successful
        ldob    inl2_ecode(g1),r4       # r4 = bit in ilm_flag2 byte to clear
        ldob    ilm_flag2(g6),r5        # r5 = ilm_flag2 byte
        clrbit  r4,r5,r5                # clear bit in ilm_flag2 byte
        stob    r5,ilm_flag2(g6)        # save updated ilm_flag2 byte
e7_io100:
        call    mag$remtask             # remove task ILT from working queue
        lda     -ILTBIAS(g1),g1         # set up to return ILT to originator
        movq    g4,r4                   # save g4-g7
        ld      il_cr(g1),r3            # r3 = originator's completion
                                        #  handler routine
        callx   (r3)                    # call completion routine
        movq    r4,g4                   # restore g4-g7
        call    mag$chknextask          # check if next task needs to be
                                        #  enabled
        movt    r12,g12                 # restore g12-g14
        ret
#
#******************************************************************************
#
#  NAME: te7a_iocomp
#
#  PURPOSE:
#       Provide the processing to perform the task I/O
#       complete processing for commands which are processed
#       while flag bits are set in ilm_flag3 byte.
#
#  DESCRIPTION:
#       Returns any resources, clears the associated flag bit in
#       ilm_flag3 byte if the I/O operation completed successfully
#       and then returns the primary task ILT back to the originator.
#
#  CALLING SEQUENCE:
#       call    te7a_iocomp
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te7a_iocomp:
        movt    g12,r12                 # save g12-g14
        cmpobne 0,g0,e7a_io100          # Jif I/O operation not successful
        ldob    inl2_ecode2(g1),r4      # r4 = bit in ilm_flag2 byte to clear
        ldob    ilm_flag3(g6),r5        # r5 = ilm_flag2 byte
        clrbit  r4,r5,r5                # clear bit in ilm_flag2 byte
        stob    r5,ilm_flag3(g6)        # save updated ilm_flag2 byte
e7a_io100:
        call    mag$remtask             # remove task ILT from working queue
        lda     -ILTBIAS(g1),g1         # set up to return ILT to originator
        movq    g4,r4                   # save g4-g7
        ld      il_cr(g1),r3            # r3 = originator's completion
                                        #  handler routine
        callx   (r3)                    # call completion routine
        movq    r4,g4                   # restore g4-g7
        call    mag$chknextask          # check if next task needs to be
                                        #  enabled
        movt    r12,g12                 # restore g12-g14
        ret
#
#******************************************************************************
#
#  NAME: te8_iocomp
#
#  PURPOSE:
#       Provide the processing to perform the task I/O
#       complete processing for MODE SELECT commands.
#
#  DESCRIPTION:
#       Checks for errors during the data transfer phase and processes
#       any errors appropriately. If no error have occurred, processes
#       the initiator's MODE SELECT data, validates it and either
#       returns the appropriate error to the initiator if one is
#       detected in the MODE SELECT data or makes the specified changes
#       in the MODE SENSE data for the initiator and returns good
#       completion status if no errors are detected.
#
#  CALLING SEQUENCE:
#       call    te8_iocomp
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = pri. ILT at inl2 nest level
#       g2 = assoc. SGL address
#       g6 = assoc. ILMT address
#       g9 = sec. ILT at otl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
        .data
te8iocomp_tbl1:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_commerr           # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
te8iocomp_tbl2:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invfpl1           # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
        .text
#
te8_iocomp:
        cmpobe  0,g0,e8_io100           # Jif I/O operation successful
        ldq     te8iocomp_tbl1,r4       # load op. values in regs.
        ld      te8iocomp_tbl1+16,r8
        b       e8_io1000
#
e8_io100:
        ldl     sg_addr(g2),r14         # r14 = base buffer address
                                        # r15 = base MODE SELECT length
        movl    r14,r12                 # r12 = working buffer address
                                        # r13 = working MODE SELECT length
        ld      ilm_wkenv(g6),r11       # r11 = working environment table addr.
#
# --- Validate MODE parameter header
#
        ldob    inl2_cdb(g1),r3         # r3 = command code byte from CDB
        ldconst 0x15,r4                 # r4 = MODE SELECT(6) command code
        cmpobne r3,r4,e8_io105          # Jif not MODE SELECT(6) code
        ldconst 0x04,r3                 # r3 = header length
        ldob    1(r14),r4               # r4 = medium type code
        ldob    2(r14),r5               # r5 = reserved byte
        ldob    3(r14),r6               # r6 = block descriptor length
        b        e8_io110
#
e8_io105:
        ldconst 0x55,r4                 # r4 = MODE SELECT(10) command code
        cmpobne r3,r4,e8_io900          # Jif not MODE SELECT(10) code
        ldconst 0x08,r3                 # r3 = header length
        ldob    2(r14),r4               # r4 = medium type code
        ldos    (r14),r5                # r5 = reserved bytes
        cmpobne 0,r5,e8_io900           # Jif reserved bytes <> 0
        ldob    3(r14),r5               # r5 = reserved byte
        cmpobne 0,r5,e8_io900           # Jif reserved byte <> 0
        ldos    4(r14),r5               # r5 = reserved bytes
        ldos    6(r14),r6
        bswap   r6,r6
        shro    16,r6,r6                # r6 = block descriptor length
e8_io110:
        cmpobg  r3,r13,e8_io900         # Jif data length < header size
        cmpobne 0,r4,e8_io900           # Jif medium type code <> 0
        cmpobne 0,r5,e8_io900           # Jif reserved field <> 0
        subo    r3,r13,r13              # r13 = updated length - header size
        addo    r3,r12,r12              # r12 = updated working data pointer
        cmpobe  0,r6,e8_io120           # Jif no block descriptor specified
#
# --- Validate block descriptor
#
        cmpobne 0x08,r6,e8_io900        # Jif block descriptor length <> 0
        cmpobg  0x08,r13,e8_io900       # Jif not enough data for block desc.
        ldl     (r12),r8                # r8-r9 = block descriptor data
        ldl     4(r11),r6               # r6-r7 = working block descriptor data
        subo    8,r13,r13               # r13 = updated data length - block
                                        #  descriptor
        lda     8(r12),r12              # r12 = updated data pointer to page
                                        #  descriptor data
        cmpobne r6,r8,e8_io900          # Jif block descriptor data not the
                                        #  same
        cmpobne r7,r9,e8_io900          # Jif block descriptor data not the
                                        #  same
e8_io120:
        cmpobe  0,r13,e8_io800          # Jif no more MODE SELECT data to
                                        #  process
        movl    r12,r14                 # r14 = MODE SELECT page data pointer
                                        # r15 = page data length
#
# --- Validate page records
#
e8_io130:
        cmpobg  0x04,r13,e8_io900       # Jif not at least min. length
        ldob    1(r12),r5               # r5 = page length
        ldob    (r12),r4                # r4 = page code
        addo    2,r5,r3                 # actual page record size
        cmpobg  r3,r13,e8_io900         # Jif page length > remaining data
                                        #  length
        cmpobg  0x04,r3,e8_io900        # Jif length not at least the min.
                                        #  record length
        ldconst 0x3f,r6                 # r6 = page code mask
        andnot  r6,r4,r7                # r7 = page code byte unused bits
        cmpobne 0,r7,e8_io900           # Jif unused bits in page code byte
                                        #  <> 0
        cmpobne 0x01,r4,e8_io140        # Jif not page 01
#
# --- Validate Page 01
#
        ldconst mspg1_sz-2,r6           # r6 = size of page 01
        cmpobne r6,r5,e8_io900          # Jif page length incorrect
        ldob    7(r12),r6               # r6 = byte #7 of page data
        ldob    9(r12),r7               # r7 = byte #9 of page data
        cmpobne 0,r6,e8_io900           # Jif reserved byte <> 0
        cmpobne 0,r7,e8_io900           # Jif reserved byte <> 0
        b       e8_io200                # continue processing MODE SELECT data
#
e8_io140:
        cmpobne  0x02,r4,e8_io140aa     # Jif not page 02
#
# --- Validate page 02
#
        ldconst mspg2_sz-2,r6           # r6 = size of page 02
        cmpobne r6,r5,e8_io900          # Jif page length incorrect
        b       e8_io200                # continue processing MODE SELECT data
#
e8_io140aa:
        cmpobne  0x03,r4,e8_io140a      # Jif not page 03
#
# --- Validate page 03
#
        ldconst mspg3_sz-2,r6           # r6 = size of page 03
        cmpobne r6,r5,e8_io900          # Jif page length incorrect
        ld      20(r12),r6              # r6 = byte #20-23 of page data
        ldconst 0x0fffffff,r7           # r7 = byte #20-23 reserved mask
        and     r7,r6,r6                # check if any reserved bits set
        cmpobne 0,r6,e8_io900           # Jif reserved bits set
        b       e8_io200                # continue processing MODE SELECT data
#
e8_io140a:
        cmpobne 0x04,r4,e8_io140b       # Jif not page 04
#
# --- Validate page 04
#
        ldconst mspg4_sz-2,r6           # r6 = size of page 04
        cmpobne r6,r5,e8_io900          # Jif page length incorrect
        ldob    17(r12),r6              # r6 = byte #17 of page data
        ldconst 0x000000fc,r7           # r7 = byte #17 reserved mask
        and     r7,r6,r6                # check if any reserved bits set
        cmpobne 0,r6,e8_io900           # Jif reserved bits set
        ldos    22(r12),r6              # r6 = byte #22-23 of page data
        cmpobne 0,r6,e8_io900           # Jif reserved bytes non-zero
        b       e8_io200                # continue processing MODE SELECT data
#
e8_io140b:
        cmpobne 0x08,r4,e8_io150        # Jif not page 08
#
# --- Validate Page 08
#
        ldconst mspg8_sz-2,r6           # r6 = size of page 08
        cmpobne r6,r5,e8_io900          # Jif page length incorrect
        ldob    12(r12),r6              # r6 = byte #12 of page data
        ldob    16(r12),r7              # r7 = byte #16 of page data
        ldob    3(r12),r9               # r9 = byte #3 of page data
        ldconst 0x1f,r8                 # r8 = byte #12 reserved mask
        and     r8,r6,r6                # check for reserved bits
        cmpobne 0,r6,e8_io900           # Jif reserved bits <> 0
        cmpobne 0,r7,e8_io900           # Jif reserved byte <> 0
        mov     r9,r6
        ldconst 0x0f,r8
        shro    4,r6,r6
        and     r8,r9,r9                # r9 = write retention priority
        and     r8,r6,r6                # r6 = demand read retention pri.
        mov     0,r7                    # r7 = valid match counter
        ldconst 0x00,r8                 # r8 = valid pri. value
        cmpobne r8,r6,e8_io142          # Jif not a match
        addo    1,r7,r7                 # inc. valid match count
e8_io142:
        cmpobne r8,r9,e8_io143          # Jif not a match
        addo    1,r7,r7                 # inc. valid match count
e8_io143:
        ldconst 0x01,r8                 # r8 = valid pri. value
        cmpobne r8,r6,e8_io144          # Jif not a match
        addo    1,r7,r7                 # inc. valid match count
e8_io144:
        cmpobne r8,r9,e8_io145          # Jif not a match
        addo    1,r7,r7                 # inc. valid match count
e8_io145:
        ldconst 0x0f,r8                 # r8 = valid pri. value
        cmpobne r8,r6,e8_io146          # Jif not a match
        addo    1,r7,r7                 # inc. valid match count
e8_io146:
        cmpobne r8,r9,e8_io147          # Jif not a match
        addo    1,r7,r7                 # inc. valid match count
e8_io147:
        cmpobne 0x02,r7,e8_io900        # Jif both retention priorities
                                        #  not valid
        b       e8_io200                # continue processing MODE SELECT data
#
e8_io150:
        cmpobne 0x0a,r4,e8_io160        # Jif not page 0a
#
# --- Validate Page 0a
#
        ldconst mspga_sz-2,r6           # r6 = size of page a
        cmpobne r6,r5,e8_io900          # Jif page length incorrect
        ldos    10(r12),r6              # r6 = byte #10 & #11 of page data
        ldob    5(r12),r7               # r7 = byte #5 of page data
        cmpobne 0,r6,e8_io900           # Jif reserved bytes <> 0
        cmpobne 0,r7,e8_io900           # Jif reserved byte <> 0
        ldob    4(r12),r6               # r6 = byte #4 of page data
        ldob    3(r12),r7               # r7 = byte #3 of page data
        ldob    2(r12),r8               # r8 = byte #2 of page data
        ldconst 0xa0,r9                 # r9 = byte #4 reserved bit mask
        and     r9,r6,r6                # check byte #4 for reserved bits set
        cmpobne 0,r6,e8_io900           # Jif reserved bits set
        mov     r7,r6                   # r6 = byte #3 of page data
        ldconst 0x08,r9                 # r9 = byte #3 reserved bit mask
        and     r9,r6,r6                # check for reserved bit set
        cmpobne 0,r6,e8_io900           # Jif reserved bit set
        shro    1,r7,r6
        and     0x03,r6,r6              # isolate QErr field
        cmpobe  0x02,r6,e8_io900        # Jif reserved QErr field value
        shro    4,r7,r6
        and     0x0f,r6,r6              # isolate Queue algorithm modifier
                                        #  field
        cmpobl  0x02,r6,e8_io900        # Jif invalid Queue algorithm modifier
        mov     r8,r6                   # r6 = byte #2 of page data
        ldconst 0x1c,r9                 # r9 = mask for reserved bits in byte
                                        #  #2 of page data
        and     r9,r6,r6                # check for reserved bits set
        cmpobne 0,r6,e8_io900           # Jif reserved bits set
        shro    5,r8,r6                 # set up to check TST field
        cmpobl  0x02,r6,e8_io900        # Jif reserved TST field value
        b       e8_io200                # continue processing MODE SELECT data
#
e8_io160:
        cmpobne 0x18,r4,e8_io170        # Jif not page 18
#
# --- Validate Page 18
#
        ldconst mspg18_sz-2,r6          # r6 = size of page 18
        cmpobne r6,r5,e8_io900          # Jif page length incorrect
        ld      4(r12),r6               # r6 = byte #4,5,6,7 of page data
        ldob    2(r12),r7               # r7 = byte #2 of page data
        cmpobne 0,r6,e8_io900           # Jif reserved bytes <> 0
        cmpobne 0,r7,e8_io900           # Jif reserved byte <> 0
        ldob    3(r12),r6               # r6 = byte #3 of page data
        andnot  1,r6,r6                 # check reserved bits
        cmpobne 0,r6,e8_io900           # Jif reserved bits <>0
        b       e8_io200                # continue processing MODE SELECT data
#
e8_io170:
        cmpobne 0x19,r4,e8_io180        # Jif not page 19
#
# --- Validate Page 19
#
        ldconst mspg19_sz-2,r6          # r6 = size of page 19
        cmpobne r6,r5,e8_io900          # Jif page length incorrect
        ld      4(r12),r6               # r6 = byte #4,5,6,7 of page data
        ldob    2(r12),r7               # r7 = byte #2 of page data
        cmpobne 0,r6,e8_io900           # Jif reserved bytes <> 0
        cmpobne 0,r7,e8_io900           # Jif reserved byte <> 0
        ldob    3(r12),r6               # r6 = byte #3 of page data
        ldconst 0x80,r7
        and     r7,r6,r6                # check reserved bits
        cmpobne 0,r6,e8_io900           # Jif reserved bits <>0
        b       e8_io200                # continue processing MODE SELECT data
#
e8_io180:
        cmpobne 0x1C,r4,e8_io190        # Jif not page 1C
#
# --- Validate Page 1C
#
        ldconst mspg1C_sz-2,r6          # r6 = size of page 1C
        cmpobne r6,r5,e8_io900          # Jif page length incorrect
        ldob    2(r12),r6               # r6 = byte #2 of page data
        ldconst 0x72,r7
        and     r7,r6,r6                # check reserved bits
        cmpobne 0,r6,e8_io900           # Jif reserved bits <>0
        b       e8_io200                # continue processing MODE SELECT data
#
e8_io190:
        cmpobne 0x00,r4,e8_io200        # Jif not page 00
#
# --- Validate Page 00
#
        ldconst mspg0_sz-2,r6           # r6 = size of page 0
        cmpobne r6,r5,e8_io900          # Jif page length incorrect
        ldob    2(r12),r6               # r6 = byte #2 of page data
        ldob    3(r12),r7               # r7 = byte #3 of page data
        ldconst 0x88,r8                 # r8 = reserved bit mask for byte #2
        and     r8,r6,r6                # check for reserved bits
        cmpobne 0,r6,e8_io900           # Jif reserved bits set
        cmpobne 0,r7,e8_io900           # Jif reserved byte <> 0
e8_io200:
        subo    r3,r13,r13              # r13 = remaining data length
        addo    r3,r12,r12              # r12 = pointer to remaining data
        cmpobne 0,r13,e8_io130          # Jif more data to check
#
# --- MODE SELECT data valid
#
        movt    g0,r8                   # save g0-g2
        movl    r14,r12                 # r12 = working page data pointer
                                        # r13 = working page data length
e8_io210:
        lda     ms1_sz(r11),g1          # g1 = pointer to first page data in
                                        #  working environment table
        ldob    (r12),r4                # r4 = page code
        ldob    1(r12),r5               # r5 = page length
        addo    2,r5,r5
        cmpobne 0x01,r4,e8_io230        # Jif not page 01
#
# --- Process page 01 data
#
        mov     r12,g0                  # g0 = MODE SELECT page data pointer
        lda     mschg_pg1,g2            # g2 = mode page change mask pointer
        call    mag$upmodesns           # update MODE SENSE data
        b       e8_io400                # continue processing page data
#
e8_io230:
        lda     mspg1_sz(g1),g1         # g1 = pointer to next page data in
                                        #  working environment table
        cmpobne 0x02,r4,e8_io235        # Jif not page 02

#
# --- Process page 02 data
#
        mov     r12,g0                  # g0 = MODE SELECT page data pointer
        lda     mschg_pg2,g2            # g2 = mode page change mask pointer
        call    mag$upmodesns           # update MODE SENSE data
        b       e8_io400                # continue processing page data
#
e8_io235:
        lda     mspg2_sz(g1),g1         # g1 = pointer to next page data in
                                        #  working environment table
        cmpobne 0x03,r4,e8_io240        # Jif not page 03
#
# --- Process page 03 data
#
        mov     r12,g0                  # g0 = MODE SELECT page data pointer
        lda     mschg_pg3,g2            # g2 = mode page change mask pointer
        call    mag$upmodesns           # update MODE SENSE data
        b       e8_io400                # continue processing page data
#
e8_io240:
        lda     mspg3_sz(g1),g1         # g1 = pointer to next page data in
                                        #  working environment table
        cmpobne 0x04,r4,e8_io240a       # Jif not page 04
#
# --- Process page 04 data
#
        mov     r12,g0                  # g0 = MODE SELECT page data pointer
        lda     mschg_pg4,g2            # g2 = mode page change mask pointer
        call    mag$upmodesns           # update MODE SENSE data
        b       e8_io400                # continue processing page data
#
e8_io240a:
        lda     mspg4_sz(g1),g1         # g1 = pointer to next page data in
                                        #  working environment table
        cmpobne 0x08,r4,e8_io250        # Jif not page 08
#
# --- Process page 08 data
#
#
        mov     r12,g0                  # g0 = MODE SELECT page data pointer
        lda     mschg_pg8,g2            # g2 = mode page change mask pointer
        call    mag$upmodesns           # update MODE SENSE data
        b       e8_io400                # continue processing page data
#
e8_io250:
        lda     mspg8_sz(g1),g1         # g1 = pointer to next page data in
                                        #  working environment table
        cmpobne 0x0a,r4,e8_io270        # Jif not page 0a
#
# --- Process page 0a data
#
#
        mov     r12,g0                  # g0 = MODE SELECT page data pointer
        lda     mschg_pga,g2            # g2 = mode page change mask pointer
        call    mag$upmodesns           # update MODE SENSE data
        b       e8_io400                # continue processing page data
#
e8_io270:
        lda     mspga_sz(g1),g1         # g1 = pointer to next page data in
                                        #  working environment table
#
        cmpobne 0x18,r4,e8_io275        # Jif not page 18
#
# --- Process page 18 data
#
#
        mov     r12,g0                  # g0 = MODE SELECT page data pointer
        lda     mschg_pg18,g2           # g2 = mode page change mask pointer
        call    mag$upmodesns           # update MODE SENSE data
        b       e8_io400                # continue processing page data
#
e8_io275:
        lda     mspg18_sz(g1),g1        # g1 = pointer to next page data in
                                        #  working environment table
        cmpobne 0x19,r4,e8_io280        # Jif not page 19
#
# --- Process page 19 data
#
#
        mov     r12,g0                  # g0 = MODE SELECT page data pointer
        lda     mschg_pg19,g2           # g2 = mode page change mask pointer
        call    mag$upmodesns           # update MODE SENSE data
        b       e8_io400                # continue processing page data
#
e8_io280:
        lda     mspg19_sz(g1),g1        # g1 = pointer to next page data in
                                        #  working environment table
        cmpobne 0x1C,r4,e8_io290        # Jif not page 1C
#
# --- Process page 1C data
#
#
        mov     r12,g0                  # g0 = MODE SELECT page data pointer
        lda     mschg_pg1C,g2           # g2 = mode page change mask pointer
        call    mag$upmodesns           # update MODE SENSE data
        b       e8_io400                # continue processing page data
#
e8_io290:
        lda     mspg1C_sz(g1),g1        # g1 = pointer to next page data in
                                        #  working environment table
        cmpobne 0x00,r4,e8_io400        # Jif not page 00
#
# --- Process page 00 data
#
#
        mov     r12,g0                  # g0 = MODE SELECT page data pointer
        lda     mschg_pg0,g2            # g2 = mode page change mask pointer
        call    mag$upmodesns           # update MODE SENSE data
        b       e8_io400                # continue processing page data
#
e8_io400:
        addo    r5,r12,r12              # r12 = pointer to next page record
        subo    r5,r13,r13              # r13 = remaining page data count
        cmpobne 0,r13,e8_io210          # Jif more page data to process
        movt    r8,g0                   # restore g0-g2
#
# --- Successful completion routine
#
e8_io800:
        ldq     tur_tbl1,r4             # just return good status
        ld      tur_tbl1+16,r8
        b       e8_io1000
#
# --- Invalid field in parameter list completion routine
#
e8_io900:
        ldq     te8iocomp_tbl2,r4       # load op. values in regs.
        ld      te8iocomp_tbl2+16,r8
e8_io1000:
        lda     -sghdrsiz(g2),g0        # set up to release SGL & buffer
        mov     g1,g8                   # g8 = pri. ILT at inl2 nest level
        call    M$rsglbuf               # release SGL & buffer
        mov     g9,g1                   # g1 = sec. ILT to deallocate
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate secondary ILT
        lda     task_etbl2,r3           # r3 = new task event handler table
        mov     g8,g9                   # g9 = pri. ILT at inl2 nest level
        ld      inl2_FCAL(g8),g7        # g7 = pri. ILT at inl1 nest level
        st      r3,inl2_ehand(g9)       # save new event handler table
        b       mag1$cmdcom             # send completion status to initiator
#
#******************************************************************************
#
#  NAME: te8_abort
#
#  PURPOSE:
#       Provide the appropriate processing for an abort event for
#       a command that requests data from the initiator before
#       proceeding with processing the command.(task_etbl8, task_etbl12x
#       event handler tables).
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te8_abort
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te8_abort:
        lda     task_etbl13,r3          # r3 = new task event handler table
        st      r3,inl2_ehand(g1)       # save new task event handler table
                                        #  in task ILT
        call    mag$qtask2aq            # queue task ILT to abort queue
        ret
#
#******************************************************************************
#
#  NAME: te8_reset
#
#  PURPOSE:
#       Provide the appropriate processing for a reset event for
#       a command that requests data from the initiator before
#       proceeding with processing the command.(task_etbl8, task_etbl12x
#       event handler tables).
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te8_reset
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te8_reset:
        b       te8_abort               # handler same as abort for now
#
#******************************************************************************
#
#  NAME: te8_offline
#
#  PURPOSE:
#       Provide the appropriate processing for an offline event for
#       a command that requests data from the initiator before
#       proceeding with processing the command.(task_etbl8, task_etbl12x
#       event handler tables).
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted.
#
#  CALLING SEQUENCE:
#       call    te8_offline
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te8_offline:
        b       te8_abort               # handler same as abort for now
#
#******************************************************************************
#
#  NAME: te9_MAGcomp
#
#  PURPOSE:
#       Perform the processing for a MAGNITUDE request completion
#       event for a RESERVE type command task.
#
#  DESCRIPTION:
#       Returns the VRP allocated for the global RESERVE request.
#       Checks for an error on the global RESERVE request and if
#       an error was indicated returns Reservation Conflict status
#       to the initiator and clears out the local device reserve.
#       If no error was indicated, returns successful status to
#       the initiator.
#
#  CALLING SEQUENCE:
#       call    te9_MAGcomp
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te9_MAGcomp:
        lda     0,r3                    # r3 = 0
        ld      ilm_vdmt(g6),g3         # g3 = assoc. VDMT address
        mov     g1,g9                   # g9 = pri. ILT at inl2 nest level
        ld      vdm_rilmt(g3),r4        # r4 = reserving ILMT address for VDMT
        ld      -ILTBIAS+vrvrp(g1),g2   # g2 = assoc. VRP address
        lda     -ILTBIAS(g1),g1         # back up to previous nest level in ILT
        cmpobne r4,g6,e9_MAG10          # Jif this ILMT not the current
                                        #  reserving ILMT
        cmpobe  0,g0,e9_MAG50           # Jif successful completion status
#
# --- Error reported on RESERVE request operation
#
e9_MAG10:
        mov     g9,r12                  # save g9
        st      r3,vdm_rilmt(g3)        # clear ILMT from vdm_rilmt field
        mov     r3,g9                   # indicate release not assoc. with
                                        #  a task
        call    mag$locrelease          # release local reserve on device
        mov     r12,g9                  # restore g9
        ldq     reserv6_tbl2,r4         # load op. values into regs.
        ld      reserv6_tbl2+16,r8
        b       e9_MAG100
#
# --- RESERVE request was successful
#
e9_MAG50:
        ldq     reserv6_tbl3,r4         # load op. values into regs.
        ld      reserv6_tbl3+16,r8
e9_MAG100:
        st      r3,vrvrp(g1)            # clear vrvrp field in ILT
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
        mov     g1,g7                   # g7 = assoc. pri. ILT at FC-AL level
        b       mag1$cmdcom             # and send completion status to host
#
#******************************************************************************
#
#  NAME: te9_abort
#
#  PURPOSE:
#       Provide the appropriate processing for an abort event for
#       a RESERVE type command.
#
#  DESCRIPTION:
#       Cancels the local reserve operation and sends a
#       release VRP to terminate any global device reservation.
#       It then branches to the te2_abort routine to perform
#       all common abort event handling processing.
#
#  CALLING SEQUENCE:
#       call    te9_abort
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te9_abort:
        mov     g3,r14                  # save g3
        ld      ilm_vdmt(g6),g3         # g3 = assoc. VDMT
        ld      vdm_rilmt(g3),r4        # r4 = reserving ILMT
        cmpobne r4,g6,e9_ab100          # Jif reserving ILMT not mine
        mov     g9,r15                  # save g9
        mov     0,g9                    # g9 = 0 indicating release not
                                        #  associated with task
        call    mag$locrelease          # release device reserve
        mov     r15,g9                  # restore g9
e9_ab100:
        mov     r14,g3                  # restore g3
        b       te2_abort               # and clean up the task
#
#******************************************************************************
#
#  NAME: te10_MAGcomp
#
#  PURPOSE:
#       Perform the processing for a MAGNITUDE request completion
#       event for a RELEASE type command task.
#
#  DESCRIPTION:
#       Returns the VRP allocated for the global RELEASE request.
#       Returns good status to the initiator.
#
#  CALLING SEQUENCE:
#       call    te10_MAGcomp
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te10_MAGcomp:
        lda     0,r3                    # r3 = 0
        mov     g1,g9                   # g9 = pri. ILT at inl2 nest level
        ld      -ILTBIAS+vrvrp(g1),g2   # g2 = assoc. VRP address
        lda     -ILTBIAS(g1),g1         # back up to previous nest level in ILT
        ldq     releas6_tbl2,r4         # load op. values into regs.
        ld      releas6_tbl2+16,r8
        st      r3,vrvrp(g1)            # clear vrvrp field in ILT
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
        mov     g1,g7                   # g7 = assoc. pri. ILT at FC-AL level
        b       mag1$cmdcom             # and send completion status to host
#
#******************************************************************************
#
#  NAME: te11_MAGcomp
#
#  PURPOSE:
#       Perform the processing for a MAGNITUDE request completion
#       event for a VERIFY type command.
#
#  DESCRIPTION:
#       Checks for errors and if any are indicated, returns the
#       appropriate status and SENSE data to the initiator. If
#       no errors are indicated, returns successful status to the
#       initiator.
#
#  CALLING SEQUENCE:
#       call    te11_MAGcomp
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
        .data
verify_tbl1:
        .byte   dtxfern,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
verify_tbl2:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
        .text
#
te11_MAGcomp:
        lda     0,r3                    # r3 = 0
        mov     g1,g9                   # g9 = pri. ILT at inl2 nest level
        ldob    inl2_ecode(g1),r4       # r4 = local error code for task
        ld      -ILTBIAS+vrvrp(g1),g2   # g2 = assoc. VRP address
        cmpobne 0,g0,e11_MAG02          # Jif error completion status
        cmpobe  0,r4,e11_MAG10          # Jif no local error indicated
e11_MAG02:
        cmpobe  0,r4,e11_MAG03          # Jif no local error code specified
        mov     r4,g0                   # g0 = error code to process. Local
                                        #  error code takes precedence.
e11_MAG03:
#
# --- Process error status
#
        ldconst 0x100,r4                # r4 = max. error code value supported
        cmpobg  r4,g0,e11_MAG04         # Jif error code within range
        ldconst 0x00,g0                 # g0 = default error code value
e11_MAG04:
        lda     te11_MAGetbl,r9         # r9 = error code normalization table
        addo    g0,r9,r9                # r9 = pointer to normalized error code
        ldob    (r9),r9                 # r9 = normalized error code
        cmpobne 10,r9,e11_MAG05         # Jif not device reserved code
        ldq     rconflict_tbl1,r4       # load op. values into regs.
        ld      rconflict_tbl1+16,r8    # return reservation conflict status
        b       e11_MAG20               #  to initiator
#
e11_MAG05:
        shlo    2,r9,r9                 # normalized error code * 4
        lda     te11_MAGetbl2,r10       # r10 = normalized error table pointer
        addo    r9,r10,r10              # r10 = pointer to error table for
                                        #  reported error
        ld      (r10),r7                # r7 = SENSE data address
        ldt     verify_tbl2,r4          # load op. values into regs.
        ld      verify_tbl2+16,r8
        b       e11_MAG20

e11_MAG10:
        ldq     verify_tbl1,r4          # load op. values into regs.
        ld      verify_tbl1+16,r8
e11_MAG20:
        lda     -ILTBIAS(g1),g1         # back up to previous nest level in ILT
        st      r3,vrvrp(g1)            # clear vrvrp field in ILT
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
        mov     g1,g7                   # g7 = assoc. pri. ILT at FC-AL level
        b       mag1$cmdcom             # and send completion status to host
#
#*****************************************************************************
#
#  NAME: te11_MAGetbl
#
#  PURPOSE:
#       Used to normalize an error code returned on a VERIFY type
#       command to determine the proper response to the initiator
#       for the reported error.
#
#  DESCRIPTION:
#       Use the error code reported from the VERIFY request to
#       index into this table. The value read from the indexed
#       location is multiplied by 4 and used to index into the
#       te11_MAGetbl2 table to get the address of the parameters
#       to set up to complete the I/O operation.
#       Note: The error code definitions are found in ecodes.inc.
#
#****************************************************************************
#
        .data
te11_MAGetbl:
        .byte   0,0,8,6,5,7,7,7         # 00-07
        .byte   4,4,4,5,5,4,10,11       # 08-0f
        .byte   0,0,0,0,0,0,0,0         # 10-17
        .byte   0,0,0,0,0,0,0,0         # 18-1f
        .byte   0,0,0,0,0,0,0,0         # 20-27
        .byte   0,0,0,0,0,0,0,0         # 28-2f
        .byte   0,0,0,0,0,0,0,0         # 30-37
        .byte   0,0,0,0,0,0,0,0         # 38-3f
        .byte   0,3,9,4,4,1,4,4         # 40-47
        .byte   1,1,1,1,4,2,1,1         # 48-4f
        .byte   4,4,3,4,0,0,0,0         # 50-57
        .byte   0,0,0,0,0,0,0,0         # 58-5f
        .byte   0,0,0,0,0,0,0,0         # 60-67
        .byte   0,0,0,0,0,0,0,0         # 68-6f
        .byte   0,0,0,0,0,0,0,0         # 70-77
        .byte   0,0,0,0,0,0,0,0         # 78-7f
        .byte   2,0,0,0,0,0,0,0         # 80-87
        .byte   0,0,0,0,0,0,0,0         # 88-8f
        .byte   0,0,0,0,0,0,0,0         # 90-97
        .byte   0,0,0,0,0,0,0,0         # 98-9f
        .byte   0,0,0,0,0,0,0,0         # a0-a7
        .byte   0,0,0,0,0,0,0,0         # a8-af
        .byte   0,0,0,0,0,0,0,0         # b0-b7
        .byte   0,0,0,0,0,0,0,0         # b8-bf
        .byte   0,0,0,0,0,0,0,0         # c0-c7
        .byte   0,0,0,0,0,0,0,0         # c8-cf
        .byte   0,0,0,0,0,0,0,0         # d0-d7
        .byte   0,0,0,0,0,0,0,0         # d8-df
        .byte   0,0,0,0,0,0,0,0         # e0-e7
        .byte   0,0,0,0,0,0,0,0         # e8-ef
        .byte   0,0,0,0,0,0,0,0         # f0-f7
        .byte   0,0,0,0,0,0,0,1         # f8-ff
#
#
#*****************************************************************************
#
#  NAME: te11_MAGetbl2
#
#  PURPOSE:
#       This table contains the addresses of parameter tables
#       associated with error codes reported for a VERIFY type
#       command.
#
#  DESCRIPTION:
#       The normalized error code from the te11_MAGetbl is multiplied
#       by 4 and used to index into this table to extract the address
#       of the parameters table used to report the associated error
#       to the initiator.
#
#****************************************************************************
#
#                                        ASQ/ASCQ Description
te11_MAGetbl2:                          # -- -- -----------------------------
        .word   sense_werr1             # 0C/02-Write error-Auto reallocation
                                        #       failed
        .word   sense_err1              # 00/06-I/O process terminated
        .word   sense_err2              # 41/00-Data path failure
        .word   sense_err3              # 3E/02-Timeout on Logical Unit
        .word   sense_err4              # 44/00-Internal target failure
        .word   sense_err5              # 04/03-Logical unit not ready -
                                        #       Manual intervention required
        .word   sense_err6              # 25/00-Logical unit not supported
        .word   sense_err7              # 21/00-Logical block address out of
                                        #       range
        .word   sense_err8              # 22/00-Illegal function
        .word   sense_err9              # 21/01-Invalid element address
        .word   0                       # Reservation conflict
        .word   sense_err10             # 1D/00-Miscompare during VERIFY op.
#
#******************************************************************************
#
#  NAME: te12a_iocomp
#
#  PURPOSE:
#       Provide the processing to perform the task I/O
#       complete processing for the first data phase of a
#       FORMAT UNIT command when we're getting the Defect List
#       Header data.
#
#  DESCRIPTION:
#       Checks for errors during the data transfer phase and processes
#       any errors appropriately. If no error have occurred, processes
#       the first 4 bytes of the initiator's FORMAT UNIT data,
#       validates it and either determines whether to get more data
#       from the initiator and if so how much. If no more data needs
#       to be received from the initiator and everything is good up
#       until this point, proceed on with processing the FORMAT UNIT
#       command.
#
#  CALLING SEQUENCE:
#       call    te12a_iocomp
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = pri. ILT at inl2 nest level
#       g2 = assoc. SGL address
#       g6 = assoc. ILMT address
#       g9 = sec. ILT at otl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te12aiocomp_tbl2:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invfpl1           # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
te12aiocomp_tbl3:
        .byte   dtxferc,scnorm,0,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
te12a_iocomp:
        cmpobe  0,g0,e12a_io100         # Jif I/O operation successful
        movl    g0,r4                   # save g0-g1
        lda     -sghdrsiz(g2),g0        # set up to release SGL & buffer
        call    M$rsglbuf               # release SGL & buffer
        mov     g9,g1                   # g1 = sec. ILT to deallocate
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate secondary ILT
        movl    r4,g0                   # restore g0-g1
        lda     task_etbl2,r3           # r3 = new task event handler table
        st      r3,inl2_ehand(g1)       # save new event handler table
        b       te2_iocomp              # and terminate task

e12a_io100:
        ld      sg_addr(g2),r14         # r14 = base buffer address
        ldob    (r14),r4                # r4 = data byte #0
        ldob    1(r14),r5               # r5 = data byte #1
        cmpobne 0,r4,e12a_io110         # Jif reserved byte not zero
        bbc     1,r5,e12a_io105         # Jif IMMED flag not set
        ldob    ILTBIAS+fu3_flags(g1),r6 # r6 = FORMAT UNIT flags byte
        setbit  6,r6,r6                 # set IMMED flag
        stob    r6,ILTBIAS+fu3_flags(g1) # save updated flags byte
e12a_io105:
        bbs     7,r5,e12a_io150         # Jif FOV bit set
        cmpobe  0,r5,e12a_io120         # Jif byte #1 zeros
#
# --- Invalid field in defect list header
#
e12a_io110:
        ldq     te12aiocomp_tbl2,r4     # load op. values in regs.
        ld      te12aiocomp_tbl2+16,r8
        b       e12a_io1000
#
# --- FOV flag clear. Proceed with default format.
#
#  INPUT:
#       r5 = data byte #1 of defect list header
#       g1 = pri. ILT at inl2 nest level
#       g2 = assoc. SGL address
#       g6 = assoc. ILMT address
#       g9 = sec. ILT at otl2 nest level
#
e12a_io120:
        lda     -sghdrsiz(g2),g0        # set up to release SGL & buffer
        mov     g1,g8                   # g8 = pri. ILT at inl2 nest level
        call    M$rsglbuf               # release SGL & buffer
        mov     g9,g1                   # g1 = sec. ILT to deallocate
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate secondary ILT
        mov     g8,g9                   # g9 = pri. ILT at inl2 nest level
        b       mag$startfu             # start the FORMAT UNIT process and exit
#
# --- FOV flag set.
#
e12a_io150:
        bbc     1,r5,e12a_io120         # Jif IP flag clear indicating to use
                                        #  default initialization pattern.
        ldob    2(r14),r6               # r6 = MSB of defect list length
        ldob    3(r14),r7               # r7 = LSB of defect list length
        shlo    8,r6,r6                 # shift MSB of defect list length
        or      r6,r7,r7                # r7 = defect list length
        ld      xlreloff(g9),r8         # r8 = relative offset of this data
                                        #  segment
        ld      sg_len(g2),r9           # r9 = length of this segment
        addo    r8,r9,r9                # r9 = relative offset of next segment
        lda     -sghdrsiz(g2),g0        # set up to release SGL & buffer
        mov     g1,g8                   # g8 = pri. ILT at inl2 nest level
        call    M$rsglbuf               # release SGL & buffer
        mov     g9,g1                   # g1 = sec. ILT to deallocate
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate secondary ILT
        addo    4,r7,g0                 # g0 = data xfer phase 2 length
                                        #  which includes the rest of the defect
                                        #  list as well as the Initialization
                                        #  Pattern Descriptor.
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        ldq     te12aiocomp_tbl3,r4     # load values into regs.
        mov     r9,r5                   # set rel. offset of transfer
        ld      te12aiocomp_tbl3+16,r8
        lda     task_etbl12b,r9         # r9 = new task event handler table
        mov     g0,r6                   # r6 = SGL address
        st      r9,inl2_ehand(g8)       # save new task event handler table
        mov     g8,g9                   # g9 = pri. ILT at inl2 nest level
        ld      inl2_FCAL(g8),g7        # g7 = pri. ILT at inl1 nest level
        b       mag2$cmdcom             # and get next portion of data from
                                        #  initiator
#
# --- Error detected in FORMAT UNIT data. Terminate command.
#
e12a_io1000:
        lda     -sghdrsiz(g2),g0        # set up to release SGL & buffer
        mov     g1,g8                   # g8 = pri. ILT at inl2 nest level
        call    M$rsglbuf               # release SGL & buffer
        mov     g9,g1                   # g1 = sec. ILT to deallocate
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate secondary ILT
        lda     task_etbl2,r3           # r3 = new task event handler table
        mov     g8,g9                   # g9 = pri. ILT at inl2 nest level
        ld      inl2_FCAL(g8),g7        # g7 = pri. ILT at inl1 nest level
        st      r3,inl2_ehand(g9)       # save new event handler table
        b       mag1$cmdcom             # send completion status to initiator
#
#******************************************************************************
#
#  NAME: te12b_iocomp
#
#  PURPOSE:
#       Provide the processing to perform the task I/O
#       complete processing for the second data phase of a
#       FORMAT UNIT command when we're getting the Defect List
#       data along with the Initialization Pattern Descriptor
#       header.
#
#  DESCRIPTION:
#       Checks for errors during the data transfer phase and processes
#       any errors appropriately. If no error have occurred, processes
#       the last 4 bytes of the initiator's FORMAT UNIT data received
#       in this data transfer segment which consists of the
#       Initialization Pattern Descriptor header,
#       validates it and either determines whether to get more data
#       from the initiator and if so how much. If no more data needs
#       to be received from the initiator and everything is good up
#       until this point, proceed on with processing the FORMAT UNIT
#       command.
#
#  CALLING SEQUENCE:
#       call    te12b_iocomp
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = pri. ILT at inl2 nest level
#       g2 = assoc. SGL address
#       g6 = assoc. ILMT address
#       g9 = sec. ILT at otl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
        .data
te12biocomp_tbl2:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invfpl1           # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
te12biocomp_tbl3:
        .byte   dtxferc,scnorm,0,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
te12b_iocomp:
        cmpobe  0,g0,e12b_io100         # Jif I/O operation successful
        movl    g0,r4                   # save g0-g1
        lda     -sghdrsiz(g2),g0        # set up to release SGL & buffer
        call    M$rsglbuf               # release SGL & buffer
        mov     g9,g1                   # g1 = sec. ILT to deallocate
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate secondary ILT
        movl    r4,g0                   # restore g0-g1
        lda     task_etbl2,r3           # r3 = new task event handler table
        st      r3,inl2_ehand(g1)       # save new event handler table
        b       te2_iocomp              # and terminate task

e12b_io100:
        ld      sg_addr(g2),r14         # r14 = base buffer address
        ld      sg_len(g2),r4           # r4 = length of data transfer
        subo    4,r4,r4                 # back up to initialization pattern
                                        #  descriptor header
        addo    r4,r14,r14              # r14 = pointer to initialization pattern
                                        #  descriptor header
        ldob    (r14),r4                # r4 = data byte #0
        ldob    1(r14),r5               # r5 = data byte #1
        ldconst 0x3f,r6                 # r6 = data byte #0 reserved bit mask
        and     r6,r4,r6                # r6 = reserved bits in data byte #0
        cmpobne 0,r4,e12b_io110         # Jif reserved byte not zero
        cmpobge 1,r5,e12b_io150         # Jif valid pattern type code
#
# --- Invalid field in initialization pattern descriptor header
#
e12b_io110:
        ldq     te12biocomp_tbl2,r4     # load op. values in regs.
        ld      te12biocomp_tbl2+16,r8
        b       e12b_io1000
#
# --- Default pattern type. Proceed with default format.
#
#  INPUT:
#       r5 = data byte #1 of initialization pattern descriptor header
#       r7 = initialization pattern length
#       g1 = pri. ILT at inl2 nest level
#       g2 = assoc. SGL address
#       g6 = assoc. ILMT address
#       g9 = sec. ILT at otl2 nest level
#
e12b_io120:
        cmpobne 0,r7,e12b_io110         # Jif pattern length non-zero
        lda     -sghdrsiz(g2),g0        # set up to release SGL & buffer
        mov     g1,g8                   # g8 = pri. ILT at inl2 nest level
        call    M$rsglbuf               # release SGL & buffer
        mov     g9,g1                   # g1 = sec. ILT to deallocate
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate secondary ILT
        mov     g8,g9                   # g9 = pri. ILT at inl2 nest level
        b       mag$startfu             # start the FORMAT UNIT process and exit
#
# --- Valid pattern type code.
#
#  INPUT:
#       r4 = data byte #0 of initialization pattern descriptor header
#       r5 = data byte #1 of initialization pattern descriptor header
#       r7 = initialization pattern length
#       g1 = pri. ILT at inl2 nest level
#       g2 = assoc. SGL address
#       g6 = assoc. ILMT address
#       g9 = sec. ILT at otl2 nest level
#
e12b_io150:
        shro    6,r4,r4                 # r4 = IP modifier code
        cmpobe  3,r4,e12b_io110         # Jif reserved IP modifier code
        cmpobe  0,r4,e12b_io160         # Jif no header specified in IP mod.
        ldob    ILTBIAS+fu3_flags(g1),r4 # r4 = FORMAT UNIT proc. flags byte
        setbit  5,r4,r4                 # set overwrite pattern with LBA flag
        stob    r4,ILTBIAS+fu3_flags(g1) # save update flags byte
e12b_io160:
        ldob    2(r14),r6               # r6 = MSB of init. pat. desc. length
        ldob    3(r14),r7               # r7 = LSB of init. pat. desc. length
        shlo    8,r6,r6                 # shift MSB of init. pat. desc. length
        or      r6,r7,r7                # r7 = init. pat. desc. length
        cmpobe  0,r5,e12b_io120         # Jif default pattern type code
        cmpobe  0,r7,e12b_io110         # Jif pattern length zero
        ldconst 512,r6                  # r6 = sector size
        cmpobg  r7,r6,e12b_io110        # Jif pattern length > sector size
        ld      xlreloff(g9),r8         # r8 = relative offset of this data
                                        #  segment
        ld      sg_len(g2),r9           # r9 = length of this segment
        addo    r8,r9,r9                # r9 = relative offset of next segment
        lda     -sghdrsiz(g2),g0        # set up to release SGL & buffer
        mov     g1,g8                   # g8 = pri. ILT at inl2 nest level
        call    M$rsglbuf               # release SGL & buffer
        mov     g9,g1                   # g1 = sec. ILT to deallocate
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate secondary ILT
        mov     r7,g0                   # g0 = data xfer phase 3 length
                                        #  which includes the Initialization
                                        #  Pattern Descriptor.
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        ldq     te12biocomp_tbl3,r4     # load values into regs.
        mov     r9,r5                   # set rel. offset of transfer
        ld      te12biocomp_tbl3+16,r8
        lda     task_etbl12c,r9         # r9 = new task event handler table
        mov     g0,r6                   # r6 = SGL address
        st      r9,inl2_ehand(g8)       # save new task event handler table
        mov     g8,g9                   # g9 = pri. ILT at inl2 nest level
        ld      inl2_FCAL(g8),g7        # g7 = pri. ILT at inl1 nest level
        b       mag2$cmdcom             # and get next portion of data from
                                        #  initiator
#
# --- Error detected in FORMAT UNIT data. Terminate command.
#
e12b_io1000:
        lda     -sghdrsiz(g2),g0        # set up to release SGL & buffer
        mov     g1,g8                   # g8 = pri. ILT at inl2 nest level
        call    M$rsglbuf               # release SGL & buffer
        mov     g9,g1                   # g1 = sec. ILT to deallocate
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate secondary ILT
        lda     task_etbl2,r3           # r3 = new task event handler table
        mov     g8,g9                   # g9 = pri. ILT at inl2 nest level
        ld      inl2_FCAL(g8),g7        # g7 = pri. ILT at inl1 nest level
        st      r3,inl2_ehand(g9)       # save new event handler table
        b       mag1$cmdcom             # send completion status to initiator
#
#******************************************************************************
#
#  NAME: te12c_iocomp
#
#  PURPOSE:
#       Provide the processing to perform the task I/O
#       complete processing for the third data phase of a
#       FORMAT UNIT command when we're getting the Initialization
#       Pattern Descriptor data.
#
#  DESCRIPTION:
#       Checks for errors during the data transfer phase and processes
#       any errors appropriately. If no error have occurred, stores the
#       SGL/buffer containing the initiator's initialization pattern
#       in the pri. ILT of the associated task and proceed with starting
#       the FORMAT UNIT process.
#
#  CALLING SEQUENCE:
#       call    te12c_iocomp
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = pri. ILT at inl2 nest level
#       g2 = assoc. SGL address
#       g6 = assoc. ILMT address
#       g9 = sec. ILT at otl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te12c_iocomp:
        cmpobe  0,g0,e12c_io100         # Jif I/O operation successful
        movl    g0,r4                   # save g0-g1
        lda     -sghdrsiz(g2),g0        # set up to release SGL & buffer
        call    M$rsglbuf               # release SGL & buffer
        mov     g9,g1                   # g1 = sec. ILT to deallocate
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate secondary ILT
        movl    r4,g0                   # restore g0-g1
        lda     task_etbl2,r3           # r3 = new task event handler table
        st      r3,inl2_ehand(g1)       # save new event handler table
        b       te2_iocomp              # and terminate task

e12c_io100:
        mov     g1,g8                   # g8 = pri. ILT at inl2 nest level
        mov     g9,g1                   # g1 = sec. ILT to deallocate
        st      g2,ILTBIAS+fu3_patsgl(g8) # save SGL/buffer with pattern in
                                        #  pri. ILT for later.
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate secondary ILT
        mov     g8,g9                   # g9 = pri. ILT at inl2 nest level
        call    mag$startfu             # start the FORMAT UNIT process
        ret                             # and we're out of here!
#
#******************************************************************************
#
#  NAME: te12_abort
#
#  PURPOSE:
#       Provide the appropriate processing for an abort event for
#       a FORMAT UNIT command.
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted. It sets the abort flag
#       in either the FUPMT or the FU3 area whichever is appropriate.
#
#  CALLING SEQUENCE:
#       call    te12_abort
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te12_abort:
        ld      ILTBIAS+fu3_fupmt(g1),r5 # r5 = assoc. FUPMT if defined
        cmpobne 0,r5,e12abort_100       # Jif FUPMT defined
        ldob    ILTBIAS+fu3_flags(g1),r6 # r6 = flag byte #1
        setbit  7,r6,r6                 # set process aborted flag
        stob    r6,ILTBIAS+fu3_flags(g1) # save updated flags byte
        lda     task_etbl13,r3          # r3 = new task event handler table
        b       e12abort_200
#
e12abort_100:
        ldob    fpmt_flag(r5),r6        # r6 = flags byte #1
        setbit  7,r6,r6                 # set process aborted flag
        stob    r6,fpmt_flag(r5)        # save updated flags byte
        ldob    fpmt_status(r5),r6      # r6 = process status
        lda     task_etbl6,r3           # r3 = new task event handler table
        cmpobne 0,r6,e12abort_200       # Jif error status already specified
        ldconst ec_abort,r6
        stob    r6,fpmt_status(r5)      # save aborted status code in FUPMT
e12abort_200:
        st      r3,inl2_ehand(g1)       # save new task event handler table
                                        #  in task ILT
        call    mag$qtask2aq            # queue task ILT to abort queue
        ret
#
#******************************************************************************
#
#  NAME: te12_reset
#
#  PURPOSE:
#       Provide the appropriate processing for a reset event for
#       a FORMAT UNIT command.
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted. It sets the abort flag
#       in either the FUPMT or the FU3 area whichever is appropriate.
#
#  CALLING SEQUENCE:
#       call    te12_reset
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te12_reset:
        b       te12_abort              # handler same as abort for now
#
#******************************************************************************
#
#  NAME: te12_offline
#
#  PURPOSE:
#       Provide the appropriate processing for an offline event for
#       a FORMAT UNIT command.
#
#  DESCRIPTION:
#       Removes the associated task ILT from the working queue
#       and places it onto the aborted queue. It replaces the
#       task event handler routine table to the one which is
#       used when a task has been aborted. It sets the abort flag
#       in either the FUPMT or the FU3 area whichever is appropriate.
#
#  CALLING SEQUENCE:
#       call    te12_offline
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te12_offline:
        b       te12_abort              # handler same as abort for now
#
#******************************************************************************
#
#  NAME: te12d_MAGcomp
#
#  PURPOSE:
#       Perform the processing for a MAGNITUDE request completion
#       event for a FORMAT UNIT command.
#
#  DESCRIPTION:
#       Checks for errors and if any are indicated, returns the
#       appropriate status and SENSE data to the initiator. If
#       no errors are indicated, returns successful status to the
#       initiator.
#
#  CALLING SEQUENCE:
#       call    te12d_MAGcomp
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
        .data
te12dMAGcomp_tbl1:
        .byte   dtxfern,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
te12dMAGcomp_tbl2:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_FUfail            # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
        .text
#
te12d_MAGcomp:
        lda     task_etbl2,r3           # r3 = new task event handler table
        st      r3,inl2_ehand(g1)       # save new task event handler table
        lda     0,r3                    # r3 = 0
        mov     g1,g9                   # g9 = pri. ILT at inl2 nest level
        ldob    inl2_ecode(g1),r4       # r4 = local error code for task
        cmpobne 0,g0,e12d_MAG02         # Jif error completion status
        cmpobe  0,r4,e12d_MAG10         # Jif no local error indicated
e12d_MAG02:
#
# --- Process error status
#
        ldq     te12dMAGcomp_tbl2,r4    # load op. values into regs.
        ld      te12dMAGcomp_tbl2+16,r8
        b       e12d_MAG20
#
e12d_MAG10:
        ldq     te12dMAGcomp_tbl1,r4    # load op. values into regs.
        ld      te12dMAGcomp_tbl1+16,r8
e12d_MAG20:
        lda     -ILTBIAS(g1),g1         # back up to previous nest level in ILT
        st      r3,vrvrp(g1)            # clear vrvrp field in ILT
        mov     g1,g7                   # g7 = assoc. pri. ILT at FC-AL level
        b       mag1$cmdcom             # and send completion status to host
#
#******************************************************************************
#
#  NAME: te13_iocomp
#
#  PURPOSE:
#       Provide the processing to perform the task I/O
#       complete processing for an aborted command that was
#       performing a data transfer before continuing to process
#       the command.
#
#  DESCRIPTION:
#       Deallocates the resources allocated for the data transfer
#       and then terminates the task.
#
#  CALLING SEQUENCE:
#       call    te13_iocomp
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = pri. ILT at inl2 nest level
#       g2 = assoc. SGL address
#       g6 = assoc. ILMT address
#       g9 = sec. ILT at otl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te13_iocomp:
        lda     -sghdrsiz(g2),g0        # set up to release SGL & buffer
        mov     g1,g8                   # g8 = pri. ILT at inl2 nest level
        call    M$rsglbuf               # release SGL & buffer
        mov     g9,g1                   # g1 = sec. ILT to deallocate
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate secondary ILT
        mov     g8,g1                   # g1 = pri. ILT at inl2 nest level
        call    mag$remtask             # remove task ILT from abort queue
        b       te1_abort               # and clean up task
#
#******************************************************************************
#
#  NAME: te14_MAGcomp
#
#  PURPOSE:
#       Perform the processing for a MAGNITUDE request completion
#       event for a Sync Cache type command.
#
#  DESCRIPTION:
#       Checks for errors and if any are indicated, returns the
#       appropriate status and SENSE data to the initiator. If
#       no errors are indicated, returns successful status to the
#       initiator.
#
#  CALLING SEQUENCE:
#       call    te14_MAGcomp
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
        .data
sync_tbl1:
        .byte   dtxfern,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
sync_tbl2:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
        .text
#
te14_MAGcomp:
        lda     0,r3                    # r3 = 0
        mov     g1,g9                   # g9 = pri. ILT at inl2 nest level
        ldob    inl2_ecode(g1),r4       # r4 = local error code for task
        ld      -ILTBIAS+vrvrp(g1),g2   # g2 = assoc. VRP address
        cmpobne 0,g0,e14_MAG02          # Jif error completion status
        cmpobe  0,r4,e14_MAG10          # Jif no local error indicated
e14_MAG02:
        cmpobe  0,r4,e14_MAG03          # Jif no local error code specified
        mov     r4,g0                   # g0 = error code to process. Local
                                        #  error code takes precedence.
e14_MAG03:
#
# --- Process error status
#
        ldconst 0x100,r4                # r4 = max. error code value supported
        cmpobg  r4,g0,e14_MAG04         # Jif error code within range
        ldconst 0x00,g0                 # g0 = default error code value
e14_MAG04:
        lda     te14_MAGetbl,r9         # r9 = error code normalization table
        addo    g0,r9,r9                # r9 = pointer to normalized error code
        ldob    (r9),r9                 # r9 = normalized error code
        cmpobne 10,r9,e14_MAG05         # Jif not device reserved code
        ldq     rconflict_tbl1,r4       # load op. values into regs.
        ld      rconflict_tbl1+16,r8    # return reservation conflict status
        b       e14_MAG20               #  to initiator
#
e14_MAG05:
        shlo    2,r9,r9                 # normalized error code * 4
        lda     te14_MAGetbl2,r10       # r10 = normalized error table pointer
        addo    r9,r10,r10              # r10 = pointer to error table for
                                        #  reported error
        ld      (r10),r7                # r7 = SENSE data address
        ldt     sync_tbl2,r4            # load op. values into regs.
        ld      sync_tbl2+16,r8
        b       e14_MAG20
#
e14_MAG10:
        ldq     sync_tbl1,r4            # load op. values into regs.
        ld      sync_tbl1+16,r8
e14_MAG20:
        lda     -ILTBIAS(g1),g1         # back up to previous nest level in ILT
        st      r3,vrvrp(g1)            # clear vrvrp field in ILT
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
        mov     g1,g7                   # g7 = assoc. pri. ILT at FC-AL level
        b       mag1$cmdcom             # and send completion status to host
#
#*****************************************************************************
#
#  NAME: te14_MAGetbl
#
#  PURPOSE:
#       Used to normalize an error code returned on a Sync Cache type
#       command to determine the proper response to the initiator
#       for the reported error.
#
#  DESCRIPTION:
#       Use the error code reported from the Sync Cache request to
#       index into this table. The value read from the indexed
#       location is multiplied by 4 and used to index into the
#       te14_MAGetbl2 table to get the address of the parameters
#       to set up to complete the I/O operation.
#       Note: The error code definitions are found in ecodes.inc.
#
#****************************************************************************
#
        .data
te14_MAGetbl:
        .byte   0,0,8,6,5,7,7,7         # 00-07
        .byte   4,4,4,5,5,4,10,4        # 08-0f
        .byte   4,0,0,0,0,0,0,0         # 10-17
        .byte   0,0,0,0,0,0,0,0         # 18-1f
        .byte   0,0,0,0,0,0,0,0         # 20-27
        .byte   0,0,0,0,0,0,0,0         # 28-2f
        .byte   0,0,0,0,0,0,0,0         # 30-37
        .byte   0,0,0,0,0,0,0,0         # 38-3f
        .byte   0,3,9,4,4,1,4,4         # 40-47
        .byte   1,1,1,1,4,2,1,1         # 48-4f
        .byte   4,4,3,4,8,0,0,0         # 50-57
        .byte   0,0,0,0,0,0,0,0         # 58-5f
        .byte   0,0,0,0,0,0,0,0         # 60-67
        .byte   0,0,0,0,0,0,0,0         # 68-6f
        .byte   0,0,0,0,0,0,0,0         # 70-77
        .byte   0,0,0,0,0,0,0,0         # 78-7f
        .byte   2,0,0,0,0,0,0,0         # 80-87
        .byte   0,0,0,0,0,0,0,0         # 88-8f
        .byte   0,0,0,0,0,0,0,0         # 90-97
        .byte   0,0,0,0,0,0,0,0         # 98-9f
        .byte   0,0,0,0,0,0,0,0         # a0-a7
        .byte   0,0,0,0,0,0,0,0         # a8-af
        .byte   0,0,0,0,0,0,0,0         # b0-b7
        .byte   0,0,0,0,0,0,0,0         # b8-bf
        .byte   0,0,0,0,0,0,0,0         # c0-c7
        .byte   0,0,0,0,0,0,0,0         # c8-cf
        .byte   0,0,0,0,0,0,0,0         # d0-d7
        .byte   0,0,0,0,0,0,0,0         # d8-df
        .byte   0,0,0,0,0,0,0,0         # e0-e7
        .byte   0,0,0,0,0,0,0,0         # e8-ef
        .byte   0,0,0,0,0,0,0,0         # f0-f7
        .byte   0,0,0,0,0,0,0,1         # f8-ff
#
#
#*****************************************************************************
#
#  NAME: te14_MAGetbl2
#
#  PURPOSE:
#       This table contains the addresses of parameter tables
#       associated with error codes reported for a VERIFY type
#       command.
#
#  DESCRIPTION:
#       The normalized error code from the te14_MAGetbl is multiplied
#       by 4 and used to index into this table to extract the address
#       of the parameters table used to report the associated error
#       to the initiator.
#
#****************************************************************************
#
#                                        ASQ/ASCQ Description
te14_MAGetbl2:                          # -- -- -----------------------------
        .word   sense_werr1             # 0C/02-Write error-Auto reallocation
                                        #       failed
        .word   sense_err1              # 00/06-I/O process terminated
        .word   sense_err2              # 41/00-Data path failure
        .word   sense_err3              # 3E/02-Timeout on Logical Unit
        .word   sense_err4              # 44/00-Internal target failure
        .word   sense_err5              # 04/03-Logical unit not ready -
                                        #       Manual intervention required
        .word   sense_err6              # 25/00-Logical unit not supported
        .word   sense_err7              # 21/00-Logical block address out of
                                        #       range
        .word   sense_err8              # 22/00-Illegal function
        .word   sense_err9              # 21/01-Invalid element address
        .word   0                       # Reservation conflict
#
#*****************************************************************************
#
#  NAME: te16_iocomp
#
#  PURPOSE:
#       To handle the second phase of persistent reserve out command (after
#       getting the parameter data for the cdb).
#
#  CALLING SEQUENCE:
#       call    te16_iocomp
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = pri. ILT at inl2 nest level
#       g2 = assoc. SGL address
#       g6 = assoc. ILMT address
#       g9 = sec. ILT at otl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#****************************************************************************
#
te16iocomp_tbl0:
        .byte   dtxfern,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length

te16iocomp_tbl1:
        .byte   dtxfern,scresc,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length

te16iocomp_tbl2:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_inv_list_len      # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length

te16iocomp_tbl3:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invf1             # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length

te16iocomp_tbl4:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_inv_release       # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length

te16iocomp_tbl5:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invfpl1           # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length

te16iocomp_tbl6:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_insuff_reg_res    # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
        .text
#
te16_iocomp:
        cmpobe  0,g0,.te16io_100       # Jif I/O operation successful
        ldq     te16iocomp_tbl1,r4      # load op. values in regs.
        ld      te16iocomp_tbl1+16,r8
        b       .te16io_900

.te16io_100:
        PushRegs(r3)
        mov     g1,g0                   # ilt at lvl-2 in g0
        ld      sg_addr(g2),g1          # g1 = base buffer address
        call    presv_out
        mov     g0,r4                   # r4 = return value
# c fprintf(stderr, "%s%s:%u presv_out returned 0x%x\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)g0);
        PopRegsVoid(r3)                 # Restore registers

        cmpobe  0xffff,r4,.te16io_1000  # Jif in progress
        cmpobe  0,r4,.te16io_200        # Jif success
        cmpobe  1,r4,.te16io_201        # Jif reservation conflict
        cmpobe  2,r4,.te16io_202        # Jif inv param list len
        cmpobe  4,r4,.te16io_204        # Jif inv release
        cmpobe  5,r4,.te16io_205        # Jif invf in param list
        cmpobe  6,r4,.te16io_206        # Jif insuf res
        b       .te16io_203             # invfl in cdb

.te16io_200:
        ldq     te16iocomp_tbl0,r4      # load op. values in regs.
        ld      te16iocomp_tbl0+16,r8
        b       .te16io_900

.te16io_201:
        ldq     te16iocomp_tbl1,r4      # load op. values in regs.
        ld      te16iocomp_tbl1+16,r8
        b       .te16io_900

.te16io_202:
        ldq     te16iocomp_tbl2,r4      # load op. values in regs.
        ld      te16iocomp_tbl2+16,r8
        b       .te16io_900

.te16io_203:
        ldq     te16iocomp_tbl3,r4      # load op. values in regs.
        ld      te16iocomp_tbl3+16,r8
        b       .te16io_900

.te16io_204:
        ldq     te16iocomp_tbl4,r4      # load op. values in regs.
        ld      te16iocomp_tbl4+16,r8
        b       .te16io_900

.te16io_205:
        ldq     te16iocomp_tbl5,r4      # load op. values in regs.
        ld      te16iocomp_tbl5+16,r8
        b       .te16io_900

.te16io_206:
        ldq     te16iocomp_tbl6,r4      # load op. values in regs.
        ld      te16iocomp_tbl6+16,r8
        b       .te16io_900

.te16io_900:
        lda     -sghdrsiz(g2),g0        # set up to release SGL & buffer
        mov     g1,g8                   # g8 = pri. ILT at inl2 nest level
        call    M$rsglbuf               # release SGL & buffer
        mov     g9,g1                   # g1 = sec. ILT to deallocate
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate secondary ILT
        lda     task_etbl2,r3           # r3 = new task event handler table
        mov     g8,g9                   # g9 = pri. ILT at inl2 nest level
        ld      inl2_FCAL(g8),g7        # g7 = pri. ILT at inl1 nest level
        st      r3,inl2_ehand(g9)       # save new event handler table
        b       mag1$cmdcom             # send completion status to initiator
#
.te16io_1000:
        lda     -sghdrsiz(g2),g0        # set up to release SGL & buffer
        mov     g1,r3                   # save g1
        call    M$rsglbuf               # release SGL & buffer
        mov     g9,g1                   # g1 = sec. ILT to deallocate
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate secondary ILT
        mov     r3,g1                   # restore g1 = .pri ILT at lvl-2
        lda     task_etbl17,r3          # r3 = new task event handler table
        st      r3,inl2_ehand(g1)       # save new event handler table
        ret
#
#******************************************************************************
#
#  NAME: MAG_prAbortTask
#
#  PURPOSE:
#       Aborts the task specified by the ILT
#
#  DESCRIPTION:
#       Aborts the task specified by the ILT. This function
#       is used in the processing of the PR OUT command with
#       service action PREEMPT AND ABORT - which aborts all the
#       outstanding cmds on different I_T nexus
#
#  CALLING SEQUENCE:
#       call    MAG_prAbortTask
#
#  INPUT:
#       g0 = primary ILT of task at the inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
# C access
# void MAG_prAbortTask(ILT *pILT)
        .globl  MAG_prAbortRask         # C access
MAG_prAbortTask:
        PushRegs(r3)
        mov     g0,g1                   # g1 = prim ILT at lvl-2
        ld      inl2_ilmt(g1),g6        # g6 = ILMT
        ld      ilm_imt(g6),g5          # g5 = IMT
        ld      im_cimt(g5),g4          # g4 = CIMT
        ldconst 0,g0                    # g0 = status
        ld      inl2_ehand(g1),g2       # g2 = task event handler table
        ld      inl2_eh_abort(g2),g3    # g3 = task's abort event handler
                                        #  routine
        call    mag$remtask             # remove task ILT from work queue
        callx   (g3)                    # and call abort event handler routine
        PopRegsVoid(r3)                 # Restore registers
        ret
#
#******************************************************************************
#
#  NAME: MAG_prComp
#
#  PURPOSE:
#       Perform the processing for a MAGNITUDE request completion
#       event for a PRR cmd after the config update
#
#  DESCRIPTION:
#       Checks for errors and if any are indicated, returns the
#       appropriate status and SENSE data to the initiator. If
#       no errors are indicated, returns successful status to the
#       initiator.
#
#  CALLING SEQUENCE:
#       call    MAG_prComp
#
#  INPUT:
#       g0 = primary ILT of task at the inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
# C access
# void MAG_prComp(ILT *pILT)
        .globl  MAG_prComp              # C access
MAG_prComp:
        mov     g0,g1                   # g1 = prim ILT at lvl-2
        ld      inl2_ilmt(g1),g6        # g6 = ILMT
        ld      ilm_imt(g6),g5          # g5 = IMT
        ld      im_cimt(g5),g4          # g4 = CIMT
        ldconst 0,g0                    # g0 = status
# Fall thru...
#te17_MAGcomp:                          # not used in the current version.
        ldq     te16iocomp_tbl0,r4      # load op. values in regs.
        ld      te16iocomp_tbl0+16,r8
        lda     task_etbl2,r3           # r3 = new task event handler table
        ld      inl2_FCAL(g1),g7        # g7 = pri. ILT at inl1 nest level
        mov     g1,g9                   # g9 = pri. ILT at inl2 nest level
        st      r3,inl2_ehand(g9)       # save new event handler table
        b       mag1$cmdcom             # send completion status to initiator
#
#******************************************************************************
#
#  NAME: te17_abort
#
#  PURPOSE:
#       Provide the appropriate processing for an abort event for
#       a PR type command.
#
#  DESCRIPTION:
#       CLeans up the request parameters and frees any memory
#       allocated for original request processing and
#       it then branches to the te2_abort routine to perform
#       all common abort event handling processing.
#
#  CALLING SEQUENCE:
#       call    te17_abort
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
te17_abort:
        PushRegs(r3)
        mov     g6,g0                  # g0=ilmt; g1=ilt at lvl-2 in g0
        call    presv_abort
c fprintf(stderr, "%s%s:%u presv_abort called\n", FEBEMESSAGE, __FILE__, __LINE__);
        PopRegsVoid(r3)                 # Restore registers
        b       te2_abort               # and clean up the task
#
#******************************************************************************
#
# _________________________ COMMAND HANDLER ROUTINES __________________________
#
#******************************************************************************
#
#******************************************************************************
#
#  NAME: mag1$cmdcom
#
#  PURPOSE:
#       Common routine to branch to set up and issue final
#       task I/O requests for tasks. This includes task I/O
#       requests for immediate type commands (either data
#       transfer w/ending status or just ending status) as well
#       as the final ending status associated with MAGNITUDE
#       related tasks.
#
#  DESCRIPTION:
#       Allocates a secondary ILT and sets it up to issue the final
#       I/O request to the FC-AL driver.
#
#  CALLING SEQUENCE:
#       b       mag1$cmdcom
#
#  INPUT:
#       r4 = xlcommand-xlscsist-xlfcflgs-00
#       r5 = rel. offset (xlreloff)
#       r6 = SGL pointer (xlsglptr)
#       r7 = Sense data pointer (xlsnsptr)
#       r8 = Sense length (xlsnslen)-SGL length (xlsgllen)
#       g7 = assoc. ILT param. structure
#       g9 = primary ILT at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       All registers may be destroyed.
#
#*************************************************************************
#
mag1$cmdcom:
        mov     inl2_ps_finalio,r10     # r10 = new process state code
        lda     mag1_iocr,r11           # r11 = I/O completion routine
mag_cmdcom:
        stob    r10,inl2_pstate(g9)     # save task process state code
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        cmpobe  0,r6,.cmdcom10          # Jif no SGL specified
        lda     sghdrsiz(r6),r6         # point to descriptor area
.cmdcom10:
        stq     r4,xlcommand(g1)        # stuff op. req. record
        mov     g7,r9                   # r9 = assoc. ILT parameter structure
        mov     g9,r10                  # r10 = pri. ILT at inl2 nest level
        st      r11,otl2_cr(g1)         # save completion routine
        stt     r8,xlsgllen(g1)         # save sense length/SGL length/assoc.
                                        #  ILT param. structure/pri. ILT at
                                        #  inl2 nest level
        cmpobe  0,r6,.cmdcom50          # Jif no SGL specified for this I/O
#
# --- SGL specified for this I/O.
#
        call    mag$updtlen             # update inl2_dtlen value and set
                                        #  appropriate xlreslen value in
                                        #  current I/O being processed.
        b       .cmdcom100              # and continue processing I/O
#
# --- No SGL specified for this I/O processing.
#
.cmdcom50:
        mov     0,r11                   # r11 = xlreslen for this I/O
        bbc     xlsndsc+16,r4,.cmdcom60 # Jif ending status not being
                                        #  presented
        ldl     inl2_dtreq(g9),r12      # r12 = inl2_dtreq value
                                        # r13 = inl2_dtlen value
        subo    r13,r12,r11             # r11 = residual length for this I/O
.cmdcom60:
        st      r11,xlreslen(g1)        # save xlreslen value for this I/O
.cmdcom100:
        mov     g1,r11                  # r11 = my ILT nest level ptr.
#
.if     ERRLOG
        call    mag$chkenderr           # check if error ending status and
                                        #  log in error log if true
.endif  # ERRLOG
#
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        movt    g12,r12                 # save g12-g14
        st      r11,otl3_OTL2(g1)       # save param. ptr. in next nest
        call    mag$ISP$receive_io      # issue channel directive
        movt    r12,g12                 # restore g12-g14
        ret
#
#*************************************************************************
#
#  NAME: mag2$cmdcom
#
#  PURPOSE:
#       Common routine to branch to set up and issue a data
#       transfer request for tasks which require processing of the
#       data received during the transfer. This includes task I/O
#       requests for immediate type commands (data transfer
#       w/o ending status). Similar to mag1$cmdcom except that
#       the I/O completion handler passes the received data back
#       to the task's I/O completion handler for processing.
#
#  DESCRIPTION:
#       Allocates a secondary ILT and sets it up to issue a data
#       transfer request to the FC-AL driver.
#
#  CALLING SEQUENCE:
#       b       mag2$cmdcom
#
#  INPUT:
#       r4 = xlcommand-xlscsist-xlfcflgs-00
#       r5 = rel. offset (xlreloff)
#       r6 = SGL pointer (xlsglptr)
#       r7 = Sense data pointer (xlsnsptr)
#       r8 = Sense length (xlsnslen)-SGL length (xlsgllen)
#       g7 = assoc. ILT param. structure
#       g9 = primary ILT at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       All registers may be destroyed.
#
#*************************************************************************
#
mag2$cmdcom:
        mov     inl2_ps_datatr,r10      # r10 = new process state code
        lda     mag2_iocr,r11           # r11 = I/O completion routine
        b       mag_cmdcom              # and finish up processing command
#
#************************************************************************
#
#  NAME: mag1_iocr
#
#  PURPOSE:
#       Provides general task I/O completion processing. Is
#       defined as the ILT completion routine for the final
#       task I/O request which includes immediate type command
#       I/O completion as well as the MAGNITUDE type command
#       final I/O completion.
#
#  DESCRIPTION:
#       Gets the associated primary ILT from the secondary
#       ILT and sets up to call the current task I/O
#       completion handler routine. It then releases any
#       resources associated with the secondary ILT, including
#       the secondary ILT as appropriate.
#
#  CALLING SEQUENCE:
#       call    mag1_iocr
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = sec. ILT address at my nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#************************************************************************
#
mag1_iocr:
        movq    g0,r12                  # save g0-g3
                                        # r13 = sec. ILT address
        ld      xl_INL2(g1),g1          # g1 = primary ILT at inl2 nest
        movq    g4,r4                   # save g4-g7
        ld      inl2_ehand(g1),g3       # g3 = task event handler table
        ld      inl2_ilmt(g1),g6        # g6 = assoc. ILMT address
        movq    g8,r8                   # save g8-g11
        ld      inl2_eh_iocomp(g3),g3   # g3 = task I/O completion handler
                                        #  routine address
#
.ifdef TRACES
        mov     r13,g9
        call    mag$tr_iocomp           # trace event
.endif # TRACES
#
        cmpobe  0,g0,.iocr1_05          # Jif no error reported
        call    mag$mmcQLerr            # send packet to MMC of the event
#
.if     ERRLOG
        mov     r13,g9                  # g9 = sec. ILT at my nest level
        call    mag$chkioerr            # process I/O error
.endif  # ERRLOG
#
.iocr1_05:
#
# --- Interface to task I/O completion handler routine -----------------------
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = primary ILT at inl2 nest level
#       g6 = assoc. ILMT address ( 0 if no assoc. ILMT)
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED.
#       All registers may be destroyed.
#
# ----------------------------------------------------------------------------
#
        mov     g6,r3                   # save g6
        callx   (g3)                    # call task I/O completion handler
        mov     r3,g6                   # restore g6
        ld      xlsglptr(r13),g0        # check if SGL/buffer assigned to ILT
        cmpobe  0,g0,.iocr100           # Jif no SGL/buffer assigned
#
# --- Calculate # bytes transfered
#
        cmpobe  0,g6,.iocr1_08          # Jif no ILMT assoc. with I/O
        ld      ilm_imt(g6),g9          # g9 = assoc. IMT
        mov     g0,g10                  # g10 = SGL pointer
        ldos    xlsgllen(r13),g1        # g1 = # SGL segments
        mov     0,g2                    # g2 = data transfer size calc. reg.
#
# --- Calculate # bytes transfered
#
.iocr1_06:
        ld      sg_len(g10),g3          # g3 = length of segment
        subo    1,g1,g1                 # dec. SGL segment count
        lda     sgdescsiz(g10),g10      # inc. to next SGL segment
        addo    g3,g2,g2                # add segment length to total
        cmpobne 0,g1,.iocr1_06          # Jif more SGL segments to process
#
# --- Update aggregate and in-progress periodic statistics
#
        lda     im_stagg(g9),g10        # point to aggregate stats area
?       ldl     imst_bytes(g10),g4      # get total # bytes
        cmpo    1,0                     # Clear carry
        addc    g2,g4,g4                # inc. aggregate total # cmds
        addc    0,g5,g5                 # inc. msw
        stl     g4,imst_bytes(g10)      # save updated counts
#
        lda     im_stinprog(g9),g10     # point to in-progress stats area
?       ldl     imst_bytes(g10),g4      # get total # bytes
        cmpo    1,0                     # Clear carry
        addc    g2,g4,g4                # inc. aggregate total # cmds
        addc    0,g5,g5                 # inc. msw
        stl     g4,imst_bytes(g10)      # save updated counts
#
# --- Release SGL/Buffer and ILT
#
.iocr1_08:
        lda     -sghdrsiz(g0),g0        # back up to start of SGL
        call    M$rsglbuf               # release SGL/buffer combo
.iocr100:
        movq    r12,g0                  # restore g0-g3
        movq    r4,g4                   # restore g4-g7
        movq    r8,g8                   # restore g8-g11
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate secondary ILT
        ret
#
#************************************************************************
#
#  NAME: mag2_iocr
#
#  PURPOSE:
#       Provides general task I/O completion processing for tasks
#       which perform a data transfer and need to process the data
#       received during the data transfer. Is defined as the ILT
#       completion routine for a data transfer I/O request for
#       immediate type commands.
#
#  DESCRIPTION:
#       Gets the associated primary ILT from the secondary
#       ILT and sets up to call the current task I/O
#       completion handler routine. This routine DOES NOT release
#       any resources. It requires the task I/O completion handler
#       routine to release any resources as appropriate.
#
#  CALLING SEQUENCE:
#       call    mag2_iocr
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = sec. ILT address at my nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#************************************************************************
#
mag2_iocr:
        movq    g0,r12                  # save g0-g3
                                        # r13 = sec. ILT address
        ld      xl_INL2(g1),g1          # g1 = primary ILT at inl2 nest
        movq    g4,r4                   # save g4-g7
        ld      xlsglptr(r13),g2        # g2 = assoc. SGL address
        ld      inl2_ehand(g1),r3       # r3 = task event handler table
        ld      inl2_ilmt(g1),g6        # g6 = assoc. ILMT address
        movq    g8,r8                   # save g8-g11
        ld      inl2_eh_iocomp(r3),r3   # r3 = task I/O completion handler
                                        #  routine address
        mov     r13,g9
#
.ifdef TRACES
        call    mag$tr_iocomp           # trace I/O completion
.endif # TRACES
#
        cmpobe  0,g0,.iocr2_05          # Jif no error reported
        call    mag$mmcQLerr            # send packet to MMC of the event
#
.if     ERRLOG
#
        mov     r13,g9                  # g9 = sec. ILT at my nest level
        call    mag$chkioerr            # process I/O error
#
.endif  # ERRLOG
                                        # g2 is not correct if EC_CMD_ABORT
        b       .iocr2_08               # If error, do not update statistics.
#
.iocr2_05:
#
# --- Update statistics if appropriate
#
        cmpobe  0,g2,.iocr2_08          # Jif no SGL specified
        cmpobe  0,g6,.iocr2_08          # Jif no ILMT assoc. with I/O
        ld      ilm_imt(g6),g3          # g3 = assoc. IMT
        mov     g2,g10                  # g10 = SGL pointer
        ldos    xlsgllen(r13),g4        # g4 = # SGL segments
        mov     0,g5                    # g5 = data transfer size calc. reg.
#
# --- Calculate # bytes transfered
#
.iocr2_06:
? # crash - cqt# 24595 - 2008-11-18 - FE ILT - failed @ magdrvr.as:13679 ld 4+g10,g7 -- bringing up vlink system, g2=g10-> level 5 of ILT, g7= 0xdaaddaad.
? # Happened on 7000 on 2010-06-12 and on 4000 about same-ish time.
        ld      sg_len(g10),g7          # g7 = length of segment
        subo    1,g4,g4                 # dec. SGL segment count
        lda     sgdescsiz(g10),g10      # inc. to next SGL segment
        addo    g7,g5,g5                # add segment length to total
        cmpobne 0,g4,.iocr2_06          # Jif more SGL segments to process
#
# --- Update aggregate and in-progress periodic statistics
#
        lda     im_stagg(g3),g7         # point to aggregate stats area
?       ldl     imst_bytes(g7),g10      # get total # bytes
        cmpo    1,0                     # Clear carry
        addc    g5,g10,g10              # inc. aggregate total # cmds
        addc    0,g11,g11               # inc. msw
        stl     g10,imst_bytes(g7)      # save updated counts
#
        lda     im_stinprog(g3),g7      # point to in-progress stats area
?       ldl     imst_bytes(g7),g10      # get total # bytes
        cmpo    1,0                     # Clear carry
        addc    g5,g10,g10              # inc. aggregate total # cmds
        addc    0,g11,g11               # inc. msw
        stl     g10,imst_bytes(g7)      # save updated counts
#
.iocr2_08:
#
#*****************************************************************************
#
# --- Interface to task I/O completion handler routine -----------------------
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = pri. ILT at inl2 nest level
#       g2 = assoc. SGL address
#       g6 = assoc. ILMT address
#       g9 = sec. ILT at otl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED.
#       All registers may be destroyed.
#
#*****************************************************************************
#
        callx   (r3)                    # call task I/O completion handler
        movq    r12,g0                  # restore g0-g3
        movq    r4,g4                   # restore g4-g7
        movq    r8,g8                   # restore g8-g11
        ret                             # and we're done
#
#************************************************************************
#
#  NAME: mag1_MAGcomp
#
#  PURPOSE:
#       Provides general task MAGNITUDE request completion
#       processing.
#
#  DESCRIPTION:
#       Sets up to call the MAGNITUDE completion event handler
#       routine currently specified for the associated task.
#
#  CALLING SEQUENCE:
#       call    mag1_MAGcomp
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = pri. ILT address of task at inl2 level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#************************************************************************
#
mag1_MAGcomp:
        movq    g0,r12                  # save g0-g3
                                        # r12 = VRP request completion status
                                        # r13 = pri. ILT at inl2 nest level
        ld      inl2_ehand(g1),g2       # g2 = task event handler table
        movq    g4,r4                   # save g4-g7
        ld      inl2_ilmt(g1),g6        # g6 = assoc. ILMT address
        ld      inl2_eh_magcomp(g2),r3  # r3 = event handler routine for this
                                        #  event
        movq    g8,r8                   # save g8-g11
#
.ifdef TRACES
#
# --- Trace incoming event if appropriate
#
        cmpobe  0,g6,.MAGc10z           # Jif no assoc. ILMT address
?# crash - JIRA SAN-1605 - 2009-06-18 FE ILMT inl2_ilmt(g1) points to freed ILMT.
        ld      ilm_cimt(g6),g4         # g4 = assoc. CIMT address
        cmpobe  0,g4,.MAGc10z           # Jif no assoc. CIMT address
        ldos    ci_tflg(g4),g2          # g2 = trace flags
        lda     C_temp_trace,g10        # g10 = trace record build pointer
        bbc     tflg_MAGcomp,g2,.MAGc10z # Jif event trace disabled
        ld      inl2_FCAL(g1),g7        # g7 = ILT param. area
        ldob    ci_num(g4),g2           # g2 = chip instance
        ldconst trt_MAGcomp,g5          # g5 = trace record type code
        ld      inl2_ehand(g1),g3       # g3 = ILMT event handler table
        ld      idf_exid(g7),g8         # g8 = exchange ID
        stob    g5,trr_trt(g10)         # save trace record type code
        stob    g2,trr_ci(g10)          # save chip instance
        ldob    inl2_eh_tstate(g3),g2   # g2 = current task state code
        stos    g8,trr_exid(g10)        # save chip instance
        ldos    idf_lun(g7),g5          # g5 = assoc. LUN #
c       g8 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        st      g0,4(g10)               # save VRP status, clear unused byte
        ld      idf_init(g7),g3         # g3 = initiator ID
        stob    g2,5(g10)               # save current task state code
        stos    g5,6(g10)               # save LUN
        st      g8,12(g10)              # save timestamp
        st      g3,8(g10)               # save initiator ID
        ld      ci_curtr(g4),g7         # g7 = current trace record pointer
        ldq     (g10),g8                # g8-g11 = trace record
        ldl     ci_begtr(g4),g2         # g2 = trace area beginning pointer
                                        # g3 = trace area ending pointer
        lda     trr_recsize(g7),g5      # g5 = next trace record pointer
        stq     g8,(g7)                 # save trace record in CIMT trace area
        cmpoble g5,g3,.MAGc10a          # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ci_tflg(g4),g3          # g3 = trace flags
        mov     g2,g5                   # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,g3,.MAGc10a # Jif wrap off flag not set
        mov     0,g3                    # turn off traces due to trace area
                                        #  wrapped.
        stos    g3,ci_tflg(g4)
.MAGc10a:
        st      g5,ci_curtr(g4)         # save new current trace record pointer
.MAGc10z:
.endif # TRACES
#
        cmpobe  0,g0,.MAGc100           # Jif successful completion status
        call    mag$mmcMAGerr           # report event to MMC
.MAGc100:
#
#*****************************************************************************
#
# --- Interface to task MAGNITUDE request completion handler routine ---------
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = primary ILT at inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED.
#       All registers may be destroyed.
#
#*****************************************************************************
#
        callx   (r3)                    # call event handler routine
        movq    r8,g8                   # restore g8-g11
        movq    r4,g4                   # restore g4-g7
        movq    r12,g0                  # restore g0-g3
        ret
#
#************************************************************************
#
#  NAME: mag2_MAGcomp
#
#  PURPOSE:
#       Provides general task MAGNITUDE request completion
#       processing for a RESERVE VRP request not associated
#       with a task.
#
#  DESCRIPTION:
#       Validates that the RESERVE request is still active locally.
#       Checks if RESERVE request was successful and if so maintains
#       the local reserve for the associated device. If the request
#       failed, terminates the local device reserve.
#
#  CALLING SEQUENCE:
#       call    mag2_MAGcomp
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = ILT address of RESERVE request at my nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#************************************************************************
#
mag2_MAGcomp:
        movq    g0,r12                  # save g0-g3
                                        # r12 = VRP request completion status
                                        # r13 = ILT at my nest level
        mov     g6,r11                  # save g6
        ld      il_w1(g1),g6            # g6 = assoc. ILMT address
        ld      il_w0(g1),g2            # g2 = assoc. VRP address
        ld      ilm_vdmt(g6),g3         # g3 = assoc. VDMT
        ld      vdm_rilmt(g3),r4        # r4 = current local reserving ILMT
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        cmpobne r4,g6,.mag2Mcomp_1000   # Jif not current local reserving ILMT
        cmpobe  ecok,g0,.mag2Mcomp_1000 # Jif global RESERVE was successful
        mov     0,r4                    # cancel local reserve
        mov     g9,r10                  # save g9
        st      r4,vdm_rilmt(g3)
        mov     r4,g9                   # indicate not related with task
        call    mag$locrelease          # release local reserve
        mov     r10,g9                  # restore g9
.mag2Mcomp_1000:
        mov     r11,g6                  # restore g6
        movq    r12,g0                  # restore g0-g3
        ret
#
#************************************************************************
#
#  NAME: mag3_MAGcomp
#
#  PURPOSE:
#       Provides general task MAGNITUDE request completion
#       processing for a RELEASE VRP request not associated
#       with a task.
#
#  DESCRIPTION:
#       Regardless of success of RELEASE request, returns the
#       allocated ILT and VRP and simply returns without further
#       complications.
#
#  CALLING SEQUENCE:
#       call    mag3_MAGcomp
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = ILT address of RELEASE request at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#************************************************************************
#
mag3_MAGcomp:
        lda     -ILTBIAS(g1),g1         # back ILT up to inl1 nest level
        ld      inl1_cr(g1),r4          # r4 = inl1 nest level completion routine
        callx   (r4)                    # and call completion routine
        ret
#
#************************************************************************
#
#  NAME: mag1_srpreq
#
#  PURPOSE:
#       Provides general SRP request processing for a task.
#
#  DESCRIPTION:
#       Sets up to call the current SRP request handler routine
#       for the associated task.
#
#  CALLING SEQUENCE:
#       call    mag1_srpreq
#
#  INPUT:
#       g1 = sec. ILT at the otl2 nest level
#       g2 = SRP address
#       g7 = pri. ILT at inl1 nest level
#       g9 = pri. ILT at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       All registers may be destroyed.
#
#************************************************************************
#
mag1_srpreq:
        ld      inl2_ehand(g9),r15      # r15 = task event handler table
        ld      inl2_ilmt(g9),g6        # g6 = assoc. ILMT address
        movt    g12,r12                 # save g12-g14
        ld      inl2_eh_srpreq(r15),r3  # r3 = SRP request event handler
                                        #  routine
#
.ifdef TRACES
#
# --- Trace incoming event if appropriate
#
        ld      ilm_cimt(g6),g4         # g4 = assoc. CIMT address
        lda     C_temp_trace,r10        # r10 = trace record build pointer
        ldos    ci_tflg(g4),r4          # r4 = trace flags
        ld      ci_curtr(g4),g3         # g3 = current trace record pointer
        bbc     tflg_srpreq,r4,.srpr100 # Jif event trace disabled
        ldob    ci_num(g4),r6           # r6 = chip instance
        ldconst trt_srpreq,r5           # r5 = trace record type code
        ld      idf_exid(g7),r7         # r7 = exchange ID
        stob    r5,trr_trt(r10)         # save trace record type code
        ldob    sr_func(g2),r8          # r8 = SRP request function code
        stob    r6,trr_ci(r10)          # save chip instance
        ldob    inl2_eh_tstate(r15),r9  # r9 = current task state code
        stos    r7,trr_exid(r10)        # save chip instance
        st      r8,4(r10)               # save SRP request function code,
                                        #  clear unused byte
c       r8 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ldos    idf_lun(g7),r6          # r6 = LUN
        stob    r9,5(r10)               # save current task state code
        ld      idf_init(g7),r7         # r7 = initiator ID
        stos    r6,6(r10)               # save LUN
        st      r8,12(r10)              # save timestamp
        st      r7,8(r10)               # save initiator ID
        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ci_begtr(g4),r8         # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(g3),r10     # r10 = next trace record pointer
        stq     r4,(g3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.srpr10a         # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ci_tflg(g4),r9          # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.srpr10a # Jif wrap off flag not set
        mov     0,r9                    # turn off traces due to trace area
                                        #  wrapped.
        stos    r9,ci_tflg(g4)
.srpr10a:
        st      r10,ci_curtr(g4)        # save new current trace record pointer
.srpr100:
#
.endif # TRACES
#
# NOTE: FALLS THROUGH!
#
#*****************************************************************************
#
# --- Interface to task SRP request handler routine --------------------------
#
#  INPUT:
#       g1 = sec. ILT at the otl2 nest level
#       g2 = SRP address
#       g6 = assoc. ILMT address
#       g7 = pri. ILT at inl1 nest level
#       g9 = pri. ILT at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED.
#       All registers may be destroyed.
#
#*****************************************************************************
#
# NOTE: FALLS THROUGH!
        callx   (r3)                    # call task's SRP request event
                                        #  handler routine
        movt    r12,g12                 # restore g12-g14
        ret
#
#************************************************************************
#
#  NAME: mag1_srpcomp
#
#  PURPOSE:
#       Provides general task SRP request completion
#       processing.
#
#  DESCRIPTION:
#       Sets up to call the SRP completion event handler
#       routine currently specified for the associated task.
#
#  CALLING SEQUENCE:
#       call    mag1_srpcomp
#
#  INPUT:
#       g0 = SRP I/O completion status code
#       g1 = sec. SRP ILT address of task at otl2 level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#************************************************************************
#
mag1_srpcomp:
        PushRegs                        # Save all g registers.
        ld      xl_INL2(g1),g9          # g9 = assoc. pri. ILT at inl2 nest level
        ld      inl2_ehand(g9),g11      # g11 = task event handler table
        ld      -ILTBIAS+otl1_srp(g1),g2 # g2 = assoc. SRP address
        ld      inl2_ilmt(g9),g6        # g6 = assoc. ILMT address
        ld      inl2_eh_srpcomp(g11),r3 # r3 = SRP completion handler routine
#
.ifdef TRACES
#
# --- Trace incoming event if appropriate
#
        ld      ilm_cimt(g6),g4         # g4 = assoc. CIMT address
        lda     C_temp_trace,g10        # g10 = trace record build pointer
        ldos    ci_tflg(g4),g3          # g3 = trace flags
        ldob    inl2_eh_tstate(g11),g11 # g11 = current task state code
        ldconst trt_srpcomp,g5          # g5 = trace record type code
        bbc     tflg_srpcomp,g3,.srpc10z # Jif event trace disabled
        ld      xlFCAL(g1),g7           # g7 = pri. ILT at inl1 nest level
        ldob    ci_num(g4),g2           # g2 = chip instance
        stob    g5,trr_trt(g10)         # save trace record type code
        ld      idf_exid(g7),g8         # g8 = exchange ID
        stob    g2,trr_ci(g10)          # save chip instance
        st      g0,4(g10)               # save SRP completion status, clear
                                        #  unused byte
c       g2 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        stob    g11,5(g10)              # save current task state code
        stos    g8,trr_exid(g10)        # save chip instance
        ldos    idf_lun(g7),g5          # g5 = assoc. LUN #
        ld      idf_init(g7),g3         # g3 = initiator ID
        stos    g5,6(g10)               # save LUN
        st      g2,12(g10)              # save timestamp
        st      g3,8(g10)               # save initiator ID
        ld      ci_curtr(g4),g7         # g7 = current trace record pointer
        mov     g9,g3                   # save g9
        ldq     (g10),g8                # g8-g11 = trace record
        lda     trr_recsize(g7),g5      # g5 = next trace record pointer
        stq     g8,(g7)                 # save trace record in CIMT trace area
        ldl     ci_begtr(g4),g8         # g8 = trace area beginning pointer
                                        # g9 = trace area ending pointer
        cmpoble g5,g9,.srpc10a          # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ci_tflg(g4),g9          # g9 = trace flags
        mov     g8,g5                   # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,g9,.srpc10a # Jif wrap off flag not set
        mov     0,g9                    # turn off traces due to trace area
                                        #  wrapped.
        stos    g9,ci_tflg(g4)
.srpc10a:
        st      g5,ci_curtr(g4)         # save new current trace record pointer
        mov     g3,g9                   # restore g9
.srpc10z:
        ldos    ci_tflg(g4),g3          # g3 = trace flags
        bbc     tflg_data,g3,.srpc10z2  # Jif event trace disabled
        ld      xlsglptr(g1),g3         # g3 = SGL pointer
        ld      ci_curtr(g4),g11        # g11 = current trace record pointer
        ld      sg_addr(g3),g7          # g7 = pointer to data
        ld      sg_len(g3),g5           # g5 = data transfer length
        ldconst trt_data,g3             # g3 = trace record type code
        bswap   g5,g5
        st      g5,trr_trt(g11)         # save length (3 bytes)
        stob    g3,trr_trt(g11)         # save trace record type code
        mov     g9,g3                   # save g9
        ldt     (g7),g8                 # g8-g10 = first 12 data bytes
        stt     g8,4(g11)               # save data
        lda     trr_recsize(g11),g5     # g5 = next trace record pointer
        ldl     ci_begtr(g4),g8         # g8 = trace area beginning pointer
                                        # g9 = trace area ending pointer
        cmpoble g5,g9,.srpc10a2         # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ci_tflg(g4),g9          # g9 = trace flags
        mov     g8,g5                   # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,g9,.srpc10a2 # Jif wrap off flag not set
        mov     0,g9                    # turn off traces due to trace area
                                        #  wrapped.
        stos    g9,ci_tflg(g4)
.srpc10a2:
        st      g5,ci_curtr(g4)         # save new current trace record pointer
        mov     g3,g9                   # restore g9
.srpc10z2:
.endif # TRACES
#
        cmpobe  0,g0,.srpc20            # Jif no error reported
        call    mag$mmcQLerr            # send packet to MMC of the event
#
.if     ERRLOG
        mov     g1,g10                  # save g1
        mov     g9,g11                  # save g9
        mov     g9,g1                   # g1 = pri. ILT at inl2 nest level
        mov     g10,g9                  # g9 = sec. ILT at otl2 nest level
        call    mag$chkioerr            # process I/O error
        mov     g10,g1                  # restore g1
        mov     g11,g9                  # restore g9
.endif  # ERRLOG
#
        b       .srpc50
#
.srpc20:
        ld      ilm_imt(g6),r4          # r4 = assoc. IMT address
        ld      xlsglptr(g1),r10        # r10 = assoc. SGL pointer
        ldos    xlsgllen(g1),r14        # r14 = SGL segment count
        ldob    xlcommand(g1),r5        # r5 = I/O op. command code
        mov     0,r9                    # r9 = total byte count of I/O op.
#
# --- Calculate # bytes transfered
#
.srpc30:
        ld      sg_len(r10),r8          # r8 = segment length
        lda     sgdescsiz(r10),r10      # inc. to next segment record
        subo    1,r14,r14               # dec. segment count
        addo    r8,r9,r9                # add to total byte count
        cmpobne 0,r14,.srpc30           # Jif more segments to process
#
# --- Update aggregate and in-progress periodic statistics
#
        lda     im_stagg(r4),r15        # point to aggregate stats area
?       ldl     imst_bytes(r15),r12     # get total # bytes
        cmpo    1,0                     # Clear carry
        addc    r9,r12,r12              # inc. aggregate total # cmds
        addc    0,r13,r13               # inc. msw
        stl     r12,imst_bytes(r15)     # save updated counts
#
        lda     im_stinprog(r4),r15     # point to in-progress stats area
?       ldl     imst_bytes(r15),r12     # get total # bytes
        cmpo    1,0                     # Clear carry
        addc    r9,r12,r12              # inc. aggregate total # cmds
        addc    0,r13,r13               # inc. msw
        stl     r12,imst_bytes(r15)     # save updated counts
#
        cmpobe  dtxferi,r5,.srpc40      # Jif read type operation
#
# --- Update WRITE byte counters in statistics
#
        lda     im_stagg(r4),r15        # point to aggregate stats area
?       ldl     imst_wbytes(r15),r12    # get total # bytes
        cmpo    1,0                     # Clear carry
        addc    r9,r12,r12              # inc. aggregate total # cmds
        addc    0,r13,r13               # inc. msw
        stl     r12,imst_wbytes(r15)    # save updated counts
#
        lda     im_stinprog(r4),r15     # point to aggregate stats area
?       ldl     imst_wbytes(r15),r12    # get total # bytes
        cmpo    1,0                     # Clear carry
        addc    r9,r12,r12              # inc. aggregate total # cmds
        addc    0,r13,r13               # inc. msw
        stl     r12,imst_wbytes(r15)    # save updated counts
#
        b       .srpc50                 # and continue processing event
#
# --- Update READ byte counters in statistics
#
.srpc40:
        lda     im_stagg(r4),r15        # point to aggregate stats area
?       ldl     imst_rbytes(r15),r12    # get total # bytes
        cmpo    1,0                     # Clear carry
        addc    r9,r12,r12              # inc. aggregate total # cmds
        addc    0,r13,r13               # inc. msw
        stl     r12,imst_rbytes(r15)    # save updated counts
#
        lda     im_stinprog(r4),r15     # point to aggregate stats area
?       ldl     imst_rbytes(r15),r12    # get total # bytes
        cmpo    1,0                     # Clear carry
        addc    r9,r12,r12              # inc. aggregate total # cmds
        addc    0,r13,r13               # inc. msw
        stl     r12,imst_rbytes(r15)    # save updated counts
#
.srpc50:
#
# NOTE: FALLS THROUGH!
#
#*****************************************************************************
#
# --- Interface to task SRP completion handler routine -----------------------
#
#  INPUT:
#       g0 = SRP request completion status code
#       g1 = sec. SRP ILT at the otl2 nest level
#       g2 = assoc. SRP address
#       g6 = assoc. ILMT address
#       g9 = pri. ILT at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED.
#       All registers may be destroyed.
#
#*****************************************************************************
#
# NOTE: FALLS THROUGH!
        callx   (r3)                    # call SRP completion handler routine
        PopRegsVoid                     # Restore all g registers.
        ret
#
#******************************************************************************
#
#  NAME: mag1$acaactive
#
#  PURPOSE:
#       Processes a task that is to have ACA active status returned
#       to the initiator.
#
#  DESCRIPTION:
#       This routine assumes that the task ILT has not been placed on
#       the work or aborted queue in the ILMT. It sets up to return
#       ACA active status to the initiator and when that I/O operation
#       completes to return the associated ILT back to the originator.
#
#  CALLING SEQUENCE:
#       call    mag1$acaactive
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
acaactive_tbl1:
        .byte   dtxfern,scacac,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
mag1$acaactive:
        lda     task_etbl2,r3           # r3 = task event handler table
        ldq     acaactive_tbl1,r4       # load op. values into regs.
        ld      acaactive_tbl1+16,r8
        st      r3,inl2_ehand(g9)       # save task event handler table
        b       mag1$cmdcom
#
#******************************************************************************
#
#  NAME: mag1$acanotactive
#
#  PURPOSE:
#       Processes an ACA type task when no ACA not active.
#
#  DESCRIPTION:
#       Removes the task ILT from the work queue and sets up to return
#       Check Condition status and SENSE data indicating to the
#       initiator that it #$@%&* UP!!!
#
#  CALLING SEQUENCE:
#       call    mag1$acanotactive
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
acanotactive_tbl1:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invm1             # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
        .text
#
mag1$acanotactive:
        mov     g9,g1                   # g1 = pri. ILT of failing task
        call    mag$remtask             # remove ILT for task from work queue
        lda     task_etbl2,r3           # r3 = task event handler table
        ldq     acanotactive_tbl1,r4    # load op. values into regs.
        ld      acanotactive_tbl1+16,r8
        st      r3,inl2_ehand(g9)       # save task event handler table
        b       mag1$cmdcom
#
#******************************************************************************
#
#  NAME: mag1$undef
#
#  PURPOSE: Pass on an undefined command received from a host.
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
mag1$undef:
c       mag1_undef(g8, g5, g7, g9);
        ret
#
#******************************************************************************
#
#  NAME: mag1$tur
#
#  PURPOSE:
#       Processes a Test Unit Ready command received from a host.
#
#  DESCRIPTION:
#       Returns appropriate status and sense data(if appropriate)
#       to indicate to the issuing host the current state of the
#       specified LUN.
#
#  CALLING SEQUENCE:
#       call    mag1$tur
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
tur_tbl1:
        .byte   dtxfern,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
mag1$tur:
        ldq     tur_tbl1,r4             # load op. values into regs.
        ld      tur_tbl1+16,r8
        b       mag1$cmdcom             # and finish processing command
#
#******************************************************************************
#
#  NAME: mag1$inquiry
#
#  PURPOSE:
#       Processes an Inquiry command received from a host.
#
#  DESCRIPTION:
#       Returns inquiry data to the issuing host.
#
#  CALLING SEQUENCE:
#       call    mag1$inquiry
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
inquiry_tbl1:
        .byte   dtxferi,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # # SGL descriptors
        .short  0                       # sense length
#
inquiry_tbl2:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invf1             # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
inquiry_tbl3:
        .byte   dtxfern,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
mag1$inquiry:
#
# --- Validate INQUIRY CDB
#
        ldob    1(g8),r4                # r4 = CDB byte with op. flags
        ldob    2(g8),r5                # r5 = Page/Operation code byte
        and     0x03,r4,r4              # mask off reserved bits
        cmpobne 0,r4,.inq1_50           # Jif flag bits set
        cmpobe  0,r5,.inq1_500          # Jif Page/Operation code 0
        b       .inq1_100               # Return error to requestor
#
# --- Flag bit(s) set. Check further for how to process CDB
#
.inq1_50:
        cmpobe  0x01,r4,.inq1_200       # Jif EVPD flag set
#
# --- Either CmdDt flag set or both EVPD & CmdDt flags set.
#       Treat these cases as invalid!
#
#
# --- Invalid INQUIRY CDB handler routine
#
.inq1_100:
        ldq     inquiry_tbl2,r4         # load op. values into regs.
        ld      inquiry_tbl2+16,r8
        b       .inq1_1000
#
# --- EVPD flag set in INQUIRY command handler routine
#
#       r5 = Page code byte from CDB
#
.inq1_200:
        cmpobe  0,r5,.inq1_400          # Jif Page 0 VPD
        ldconst 0x80,r4                 # r4 = page 80 VPD code
        cmpobe  r4,r5,.inq1_300         # Jif Page 0x80 VPD
        ldconst 0x83,r4                 # r4 = page 83 VPD code
        cmpobne r4,r5,.inq1_100         # Jif not Page 0x83 VPD
#
# --- Process INQUIRY Vital Product Data Page 0x83 request
#
        ldob    4(g8),r3                # r3 = alloc. length from CDB
        lda     inqpg_83_size,g0        # g0 = SGL/buffer combo memory size
# Determine if WHQL compliance is active and set parameter accordantly
        ldob    MAGD_SCSI_WHQL,r4       # is WHQL compliance active?
        cmpobe 0,r4,.inq1_210
        lda     inqpg_83_WHQLsize,g0    # g0 = SGL/buffer combo memory size
.inq1_210:
        mov     g0,r9                   # r9 = my inquiry data size
        subo    4,r9,r4                 # calculate new inquiry additional length
        stob    r4,inqpg_83_len         # save new length
        cmpobe  0,r3,.inq1_501          # Jif alloc. length zero
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        ld      ilm_vdmt(g6),r4         # r4 = assoc. VDMT address
        cmpobne 0,r4,.inq1_220          # Jif there is a VDMT address
        ldconst 0xFFFF,r5               # Show an invalid VID number
        b       .inq1_230               # Store the invalid VID number

.inq1_220:
        ldos    vdm_vid(r4),r5          # r5 = VDisk ID
.inq1_230:
        stos    r5,inqpg_83_vd          # save assigned VDisk ID
        stos    r5,inqpg_83_vd2         # save assigned VDisk ID(for WHQL)
        lda     inqpg_83,r4             # r4 = inquiry VPD base address
        ld      sghdrsiz+sg_addr(g0),g1 # g1 = buffer address
        mov     r9,r5                   # r5 = # bytes to copy
        mov     g1,r7                   # r7 = buffer address
.inq1_250:
        ldob    (r4),r6
        lda     1(r4),r4
        subo    1,r5,r5                 # dec. loop count
        stob    r6,(r7)
        lda     1(r7),r7
        cmpobne 0,r5,.inq1_250          # Jif more data to copy
        b       .inq1_900               # and finish setting up I/O operation
#
# --- Process INQUIRY Vital Product data Page 0x80 request
#
.inq1_300:
        ldob    4(g8),r3                # r3 = alloc. length from CDB
        lda     inqpg_80_size,g0        # g0 = SGL/buffer combo memory size
        mov     g0,r9                   # r9 = my inquiry data size
        cmpobe  0,r3,.inq1_501          # Jif alloc. length zero
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        ldt     inquiry1_sn,r4          # r4-r6=drive serial number (less VDisk
                                        #  number)
        stt     r4,inqpg_80_sn          # copy drive serial number from basic
                                        # INQUIRY data
        ld      ilm_vdmt(g6),r4         # r4 = assoc. VDMT address
        cmpobne 0,r4,.inq1_320          # Jif there is a VDMT address
        ldconst 0x20202020,r5           # No VDMT - Set VID to spaces
        st      r5,inqpg_80_sn+8        # Set the VID to spaces
        b       .inq1_350               # Finish the work

.inq1_320:
        ldos    vdm_vid(r4),r5          # r5 = VDisk ID
        mov     4,r4                    # r4 = VDisk ID # digit count
        lda     0x30,r6
        lda     0x39,r7
.inq1_330:
        and     0x0f,r5,r8              # r6 = VDisk ID # digit
        addo    r6,r8,r8                # calc. ASCII code
        cmpoble r8,r7,.inq1_340         # Jif 0-9
        addo    7,r8,r8                 # A-F
.inq1_340:
        cmpdeco 1,r4,r4                 # dec. digit count
        stob    r8,inqpg_80_sn+8[r4*1]  # save serial # digit
        shro    4,r5,r5                 # shift to next digit
        bne    .inq1_330                # Jif loop count <> 0
.inq1_350:
        lda     inqpg_80,r4             # r4 = inquiry VPD base address
        ld      sghdrsiz+sg_addr(g0),g1 # g1 = buffer address
        mov     r9,r5                   # r5 = # bytes to copy
        mov     g1,r7                   # r7 = buffer address
.inq1_360:
        ldob    (r4),r6
        lda     1(r4),r4
        subo    1,r5,r5                 # dec. loop count
        stob    r6,(r7)
        lda     1(r7),r7
        cmpobne 0,r5,.inq1_360          # Jif more data to copy
        b       .inq1_900               # and finish setting up I/O operation
#
# --- Process INQUIRY Vital Product Data Page 0x00 request
#
.inq1_400:
        ldob    4(g8),r3                # r3 = alloc. length from CDB
        lda     inqpg_00_size,g0        # g0 = SGL/buffer combo memory size
        mov     g0,r9                   # r9 = my inquiry data size
        cmpobe  0,r3,.inq1_501          # Jif alloc. length zero
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        lda     inqpg_00,r4             # r4 = inquiry VPD base address
        ld      sghdrsiz+sg_addr(g0),g1 # g1 = buffer address
        mov     r9,r5                   # r5 = # bytes to copy
        mov     g1,r7                   # r7 = buffer address
.inq1_410:
        ldob    (r4),r6
        lda     1(r4),r4
        subo    1,r5,r5                 # dec. loop count
        stob    r6,(r7)
        lda     1(r7),r7
        cmpobne 0,r5,.inq1_410          # Jif more data to copy
        b       .inq1_900               # and finish setting up I/O operation
#
# --- Basic INQUIRY command request handler routine. Determine if the
#     CDB is non zero
#
.inq1_500:
        ldob    4(g8),r3                # r3 = alloc. length from CDB
        lda     inquiry1_size,g0        # g0 = SGL/buffer combo memory size
        mov     g0,r9                   # r9 = my inquiry data size
        cmpobne 0,r3,.inq1_501          # Jif alloc. length non-zero

        ldq     inquiry_tbl3,r4         # load op. values into regs.
        ld      inquiry_tbl3+16,r8
        b       .inq1_1000              # and just return status
#
# --- CDB seems ok. Determine if WHQL compliance is active and set parameter accordantly
#
.inq1_501:
        lda     inquiry1_size,g0        # g0 = SGL/buffer combo memory size
        ldconst 02,r5                   # ISO value
        ldob    MAGD_SCSI_WHQL,r4       # is WHQL compliance active??
        cmpobe  0,r4,.inq1_505          # Jif no

        lda     inquiry1_WHQL_size,g0   # g0 = SGL/buffer combo memory size
        ldconst 04,r5                   # ISO value
#
# --- save parameter and allocate buffer
#
.inq1_505:
        mov     g0,r9                   # r9 = my inquiry data size
        stob    r5,inquiry1_ISO         # save ISO type
        subo    5,r9,r4                 # calculate new associated data length
        stob    r4,inquiry1_len         # save new length

c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        ld      sghdrsiz+sg_addr(g0),g1 # g1 = buffer address
c       memcpy((void *)g1, &inquiry1, r9);
#
# --- Add assigned VID to serial number in INQUIRY data
#
        ld      ilm_vdmt(g6),r7         # r7 = assoc. VDMT
        cmpobne 0,r7,.inq1_520          # Jif there is a VDMT
        ldconst 0x20202020,r5           # No VDMT - Set VID to spaces
        st      r5,44(g1)               # Set the VID to spaces
        b       .inq1_900

.inq1_520:
        ldos    vdm_vid(r7),r5          # r5 = VID
        mov     4,r4                    # r4 = VDisk ID # digit count
        lda     0x30,r6
        lda     0x39,r7
.inq1_530:
        and     0x0f,r5,r8              # r6 = VDisk ID # digit
        addo    r6,r8,r8                # calc. ASCII code
        cmpoble r8,r7,.inq1_540         # Jif 0-9
        addo    7,r8,r8                 # A-F
.inq1_540:
        cmpdeco 1,r4,r4                 # dec. digit count
        stob    r8,44(g1)[r4*1]         # save serial # digit
        shro    4,r5,r5                 # shift to next digit
        bne    .inq1_530                # Jif loop count <> 0
.inq1_900:
        cmpo    r3,r9                   # check if alloc. length < inquiry size
        sell    r9,r3,r9                # r9 = size of transfer to host
        st      r9,sghdrsiz+sg_len(g0)  # save size of data in SGL
        ldq     inquiry_tbl1,r4         # load op. values into regs.
        ld      inquiry_tbl1+16,r8
        mov     g0,r6                   # r6 = SGL pointer
.inq1_1000:
        b       mag1$cmdcom             # and finish processing command
#
#******************************************************************************
#
#  NAME: mag1$readcap
#
#  PURPOSE:
#       Processes a Read Capacity command received from a host.
#
#  DESCRIPTION:
#       Returns the device capacity data to the issuing host
#       for the specified LUN.
#
#  CALLING SEQUENCE:
#       call    mag1$readcap
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .set    readcap_size,8          # size of Read Capacity data to host
#
        .data
readcap_tbl1:
        .byte   dtxferi,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
mag1$readcap:
        lda     readcap_size,g0         # allocate an SGL/buffer combo
        mov     g0,r9                   # r9 = my read capacity data size
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        ld      sghdrsiz+sg_addr(g0),g1 # g1 = buffer address
        ld      ilm_vdmt(g6),g2         # g2 = assoc. VDMT address
# NOTE! - Read Capacity only allows 4 bytes of capacity
        ldl     vdm_devcap(g2),r4       # r4,r5 = device capacity
c       *((UINT64*)&r4) = *((UINT64*)&r4) - 1;
c       if (r5 != 0) {
c           r4 = 0xFFFFFFFFLLU;         # Flag that SCSI-3 Read Capacity (16) needs to be issued.
c       }
        ld      MAG_inq_data+vq_secsize,r5 # r5 = sector size
        st      r9,sghdrsiz+sg_len(g0)  # save size of data in SGL
        bswap   r4,r4
        bswap   r5,r5
        stl     r4,(g1)                 # save device capacity & sector size
                                        #  in data area
        ldq     readcap_tbl1,r4         # load op. values into regs.
        ld      readcap_tbl1+16,r8
        mov     g0,r6                   # r6 = SGL pointer
        b       mag1$cmdcom             # and finish processing command
#
#******************************************************************************
#
#  NAME: mag1$readcap_16
#
#  PURPOSE:
#       Processes a Read Capacity (16) command received from a host.
#
#  DESCRIPTION:
#       Returns the device capacity data to the issuing host
#       for the specified LUN.
#
#  CALLING SEQUENCE:
#       call    mag1$readcap_16
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .set    readcap_16_size,0x0C    # size of Read Capacity data to host
#
        .data
readcap_16_tbl1:
        .byte   dtxferi,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
mag1$readcap_16:
# Must be service action 0x10 of read_capacity, and buffer length of 12 bytes to return.
c       r8 = *((UINT8*)g8 + 13);        # Size of return for read capacity (16)
c       if (*((UINT8*)g8 + 1) != 0x10 || r8 > 128 || r8 < 12) {
            ldq     readcap_16_tbl1,r4  # load op. values into regs.
c fprintf(stderr,"%s%s:%u mag1$readcap_16 READ CAPACITY 16 -- can't handle\n", FEBEMESSAGE, __FILE__, __LINE__);
c           r8 = 0;                     # No SGL descriptors
            b       mag1$cmdcom             # and exit now.
c       }
#++ c       r8 = readcap_16_size;           # Size of return for read capacity (16)
c       g0 = m_asglbuf(r8);             # allocate a SGL and buffer
        ld      sghdrsiz+sg_addr(g0),g1 # g1 = buffer address
        ld      ilm_vdmt(g6),g2         # g2 = assoc. VDMT address
        ldl     vdm_devcap(g2),r4       # r4,r5 = device capacity
c       *((UINT64*)&r4) = *((UINT64*)&r4) - 1;  # Last block is one less than capacity.
        ld      MAG_inq_data+vq_secsize,r6 # r6 = sector size
        st      r8,sghdrsiz+sg_len(g0)  # save size of data in SGL
# The output is all byte swapped "interestingly".
        mov     r4,r7                   # move r4
        bswap   r5,r4                   # byte swap r5 into r4
        bswap   r7,r5                   # byte swap r4 (in r7) into r5.
        stl     r4,(g1)                 # save device capacity in data area
        bswap   r6,r6
        st      r6,8(g1)                # save sector size in data area
#
        ldq     readcap_16_tbl1,r4      # load op. values into regs.
        ld      readcap_16_tbl1+16,r8
        mov     g0,r6                   # r6 = SGL pointer
        b       mag1$cmdcom             # and finish processing command
#
#******************************************************************************
#
#  NAME: mag1$read10
#
#  PURPOSE:
#       Processes a Read (10) Extended command received from a host.
#
#  DESCRIPTION:
#       Sets up to forward the Read request to the MAGNITUDE.
#
#  CALLING SEQUENCE:
#       call    mag1$read10
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
read10_tbl1:
        .byte   dtxfern,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
mag1$read10:
        ld      6(g8),r8                # isolate transfer length from CDB
        lda     0x00ffff00,r4
        ld      2(g8),r6                # r6 = SDA of host request
        and     r4,r8,r8
        mov     0,r7                    # Init the upper SDA to zero
        bswap   r8,r8
        bswap   r6,r6
        shro    8,r8,r8                 # r8 = transfer length from CDB
#
# NOTE: FALLS THROUGH!
#
#*****************************************************************************
#
#  NAME: read_com
#
#  PURPOSE:
#       READ (xx) command common handler routine
#
#  DESCRIPTION:
#       Provides the command command processing to set up to
#       perform a READ command to the MAGNITUDE.
#
#  CALLING SEQUENCE:
#       b       read_com
#
#  INPUT:
#       r6,r7 = SDA of host request (little endian format)
#       r8 = transfer length from CDB (little endian format)
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
# NOTE: FALLS THROUGH!
#
read_com:
        cmpobne 0,r8,.read10_10         # Jif transfer length non-zero
#
        ldq     read10_tbl1,r4          # load op. values from table
        ld      read10_tbl1+16,r8
        b       mag1$cmdcom             # and finish up routine here and now

.read10_10:
#
# --- Update aggregate and in-progress periodic statistics
#
        lda     im_stagg(g5),r15        # point to aggregate stats area
?       ldl     imst_reads(r15),r12     # get total # bytes
        cmpo    1,0                     # Clear carry
        addc    1,r12,r12               # inc. aggregate total # cmds
        addc    0,r13,r13               # inc. msw
        stl     r12,imst_reads(r15)     # save updated counts
#
        lda     im_stinprog(g5),r15     # point to in-progress stats area
?       ldl     imst_reads(r15),r12     # get total # bytes
        cmpo    1,0                     # Clear carry
        addc    1,r12,r12               # inc. aggregate total # cmds
        addc    0,r13,r13               # inc. msw
        stl     r12,imst_reads(r15)     # save updated counts
#
# --- Update HBA Read Stats
        ldob    ci_num(g4),r9           # interface #
#
        ld      hbaPerRdCmds[r9*4],r12  # load periodic reads for this interface.
        addo    1,r12,r12               # inc. periodic # cmds
        st      r12,hbaPerRdCmds[r9*4]  # save new value
#
        ldl     hbaPerRdBlocks[r9*8],r12  # load periodic block count
        cmpo    1,0                     # Clear carry
        addc    r8,r12,r12              # add this command's block count in
        addc    0,r13,r13               # inc. msw
        stl     r12,hbaPerRdBlocks[r9*8]  # save new value
#
        shlo    9,r8,r9                 # r9 = byte count for CDB (sector count
                                        #  * 512)
c       g2 = get_vrp();                 # Allocate a VRP
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u get_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
        st      g2,vrvrp(g7)            # save VRP address in pri. ILT
        lda     mag1_MAGcomp,r5         # r5 = ILT completion routine
        mov     vrinput,r4              # r4 = VRP request code
        ld      ilm_vdmt(g6),g10        # g10 = assoc. VDMT
        stl     r6,vr_vsda(g2)          # save SDA
        st      r5,inl2_cr(g9)          # save my completion handler in pri. ILT
        st      r4,vr_func(g2)          # save request function code, clear
                                        #  strategy and status
        lda     mag1_srpreq,r5          # r5 = SRP received handler routine
        ldob    im_flags(g5),r7         # r7 = server flags
        ldos    vdm_vid(g10),r6         # r6 = vid
        ldob    im_pri(g5),r4           # r4 = server priority
        bbc     im_flags_mtml,r7,.read10_40 # Jif this is not a VLink Server
        setbit  vrvlinkop,0,r7          # Show this op is an incoming VLink Op
        stob    r7,vr_options(g2)
#
.read10_40:
        st      r8,vr_vlen(g2)          # save # sectors to read in VRP
        stos    r6,vr_vid(g2)           # save vid
        st      r5,inl2_rcvsrp(g9)      # save SRP handler routine in ILT
        lda     ILTBIAS(g9),g1          # g1 = ILT at MAG interface nest level
        stob    r4,vr_strategy(g2)      # save VRP strategy
        st      g7,inl3_FCAL(g1)        # save FC-AL pointer in ILT
        ldconst 0xfeedf00d,r4
        st      g2,vrvrp(g1)            # save the VRP at this ILT next level
        shlo    9,r8,r5                 # r5 = byte count of read data
        st      0,vr_sglptr(g2)         # initialize SGL ptr. in VRP
        lda     sghdrsiz+sgdescsiz,r6
        stl     r4,vrpsiz+sg_desc0+sg_addr(g2)
        st      r6,vr_sglsize(g2)       # initialize SGL size in VRP
        mov     1,r7                    # r7 = descriptor count
        st      r6,vrpsiz+sg_size(g2)
        mov     inl2_ps_req,r14         # r14 = new task process state code
        lda     task_etbl3,r15          # r15 = task event handler table
        stob    r14,inl2_pstate(g9)     # save new task process state code
        st      r15,inl2_ehand(g9)      # save new task event handler table
        st      0,vr_pptr(g2)           # clear packet physical address field
        st      r7,vrpsiz+sg_scnt(g2)   # save descriptor count
#
.ifdef TRACES
        call    mag$tr_MAG$submit_vrp   # trace event
.endif # TRACES
#
c       record_mag(FR_MAG_READ, (void *)g2);
        b       MAG$submit_vrp          # send VRP to MAGNITUDE
#
#******************************************************************************
#
#  NAME: mag1$read_16
#
#  PURPOSE:
#       Processes a Read (16) command received from a host.
#
#  DESCRIPTION:
#       Sets up to forward the Read request to the MAGNITUDE.
#
#  CALLING SEQUENCE:
#       call    mag1$read_16
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
mag1$read_16:
        ld      10(g8),r8               # transfer length from CDB
        bswap   r8,r8                   # Convert length to little endian
        ldl     2(g8),r6                # r6,r7 = SDA of host request
        mov     r6,r3                   # move r6 to r3
        bswap   r7,r6                   # byte swap r7 into r6
        bswap   r3,r7                   # byte swap r6 (in r3) into r7.
#       r6,r7 = SDA of host request (little endian format)
#       r8 = transfer length (little endian)
        b       read_com
#
#******************************************************************************
#
#  NAME: mag1$write_16
#
#  PURPOSE:
#       Processes a Write (16) command received from a host.
#
#  DESCRIPTION:
#       Sets up to forward the Write request to the MAGNITUDE.
#
#  CALLING SEQUENCE:
#       call    mag1$write_16
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
mag1$write_16:
        ld      10(g8),r8               # transfer length from CDB
        bswap   r8,r8                   # Convert length to little endian
        ldl     2(g8),r6                # r6,r7 = SDA of host request
        mov     vroutput,r4             # r4 = VRP request code
        mov     r6,r3                   # move r6 to r3
        bswap   r7,r6                   # byte swap r7 into r6
        bswap   r3,r7                   # byte swap r6 (in r3) into r7.
#       r4 = VRP request function code
#       r6,r7 = SDA of host request (little endian format)
#       r8 = transfer length (little endian)
        b       write_com
#
#******************************************************************************
#
#  NAME: mag1$write10
#
#  PURPOSE:
#       Processes a Write (10) Extended command received from a host.
#
#  DESCRIPTION:
#       Sets up to forward the Write request to the MAGNITUDE.
#
#  CALLING SEQUENCE:
#       call    mag1$write10
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
write10_tbl1:
        .byte   dtxfern,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
mag1$write10:
        ld      6(g8),r8                # isolate transfer length from CDB
        lda     0x00ffff00,r5
        ld      2(g8),r6                # r6 = SDA of host request
        mov     vroutput,r4             # r4 = VRP request code
        and     r5,r8,r8
        mov     0,r7                    # Zero out the upper SDA word
        bswap   r8,r8
        bswap   r6,r6
        shro    8,r8,r8                 # r8 = transfer length from CDB
#
# NOTE: FALLS THROUGH to the Write Common handler
#
#******************************************************************************
#
#  NAME: write_com
#
#  PURPOSE:
#       WRITE (xx) command common handler routine.
#
#  DESCRIPTION:
#       Performs the common processing necessary to set up
#       and issue a WRITE command request to the MAGNITUDE.
#
#  CALLING SEQUENCE:
#       b       write_com
#
#  INPUT:
#       r4 = VRP request function code
#       r6,r7 = SDA of host request (little endian format)
#       r8 = transfer length from CDB (little endian format)
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
# NOTE: FALLS THROUGH to the Write Common handler
#
write_com:
        cmpobne  0,r8,.write10_10       # Jif transfer length non-zero
#
        ldq     write10_tbl1,r4         # load op. values from table
        ld      write10_tbl1+16,r8
        b       mag1$cmdcom             # and finish up routine here and now
#
# --- Update aggregate and in-progress periodic statistics
#
.write10_10:
        lda     im_stagg(g5),r15        # point to aggregate stats area
?       ldl     imst_writes(r15),r12    # get total # bytes
        cmpo    1,0                     # Clear carry
        addc    1,r12,r12               # inc. aggregate total # cmds
        addc    0,r13,r13               # inc. msw
        stl     r12,imst_writes(r15)    # save updated counts
#
        lda     im_stinprog(g5),r15     # point to in-progress stats area
?       ldl     imst_writes(r15),r12    # get total # bytes
        cmpo    1,0                     # Clear carry
        addc    1,r12,r12               # inc. aggregate total # cmds
        addc    0,r13,r13               # inc. msw
        stl     r12,imst_writes(r15)    # save updated counts
#
# --- Update HBA Write Stats
        ldob    ci_num(g4),r9           # interface #
#
        ld      hbaPerWrCmds[r9*4],r12  # load periodic reads for this interface.
        addo    1,r12,r12               # inc. periodic # cmds
        st      r12,hbaPerWrCmds[r9*4]  # save new value
#
        ldl     hbaPerWrBlocks[r9*8],r12  # load periodic block count
        cmpo    1,0                     # Clear carry
        addc    r8,r12,r12              # add this command's block count in
        addc    0,r13,r13               # inc. msw
        stl     r12,hbaPerWrBlocks[r9*8] # save new value
#
        shlo    9,r8,r9                 # r9 = byte count for CDB (sector count
                                        #  * 512)
c       g2 = get_vrp();                 # Allocate a VRP
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u get_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
        st      g2,vrvrp(g7)            # save VRP address in pri. ILT
        lda     mag1_MAGcomp,r5         # r5 = ILT completion routine
        ld      ilm_vdmt(g6),g10        # g10 = assoc. VDMT
        stl     r6,vr_vsda(g2)          # save SDA
        st      r5,inl2_cr(g9)          # save my completion handler in pri. ILT
        st      r4,vr_func(g2)          # save request function code, clear
                                        #  strategy and status
        lda     mag1_srpreq,r5          # r5 = SRP received handler routine
        ldob    im_flags(g5),r7         # r7 = server flags
        ldos    vdm_vid(g10),r6         # r6 = vid
        ldob    im_pri(g5),r4           # r4 = server priority
        bbc     im_flags_mtml,r7,.write10_40 # Jif this is not a VLink Server
        setbit  vrvlinkop,0,r7          # Show this op is an incoming VLink Op
        stob    r7,vr_options(g2)
#
.write10_40:
        st      r8,vr_vlen(g2)          # save # sectors to write in VRP
        stos    r6,vr_vid(g2)           # save vid
        stob    r4,vr_strategy(g2)      # save VRP strategy
        st      r5,inl2_rcvsrp(g9)      # save SRP handler routine in ILT
        lda     ILTBIAS(g9),g1          # g1 = ILT at MAG interface nest level
        st      g7,inl3_FCAL(g1)        # save FC-AL pointer in ILT
        ldconst 0xfeedf00d,r4
        st      g2,vrvrp(g1)            # save the VRP at this ILT next level
        shlo    9,r8,r5                 # r5 = byte count of write data
        st      0,vr_sglptr(g2)         # initialize SGL ptr. in VRP
        lda     sghdrsiz+sgdescsiz,r6
        stl     r4,vrpsiz+sg_desc0+sg_addr(g2)
        st      r6,vr_sglsize(g2)       # initialize SGL size in VRP
        mov     1,r7                    # r7 = descriptor count
        st      r6,vrpsiz+sg_size(g2)
        mov     inl2_ps_req,r14         # r14 = new task process state code
        lda     task_etbl4,r15          # r15 = task event handler table
        stob    r14,inl2_pstate(g9)     # save new task process state code
        st      r15,inl2_ehand(g9)      # save new task event handler table
        st      0,vr_pptr(g2)           # clear packet physical address field
        st      r7,vrpsiz+sg_scnt(g2)   # save descriptor count
#
.ifdef TRACES
        call    mag$tr_MAG$submit_vrp   # trace event
.endif # TRACES
#
c       record_mag(FR_MAG_WRITE, (void *)g2);
        b       MAG$submit_vrp          # send VRP to MAGNITUDE
#
#******************************************************************************
#
#  NAME: mag1$read6
#
#  PURPOSE:
#       Processes a Read (6) command received from a host.
#
#  DESCRIPTION:
#       Sets up to forward the Read request to the MAGNITUDE.
#
#  CALLING SEQUENCE:
#       call    mag1$read6
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
mag1$read6:
        ld      (g8),r6
        ldconst 0x001fffff,r5
        ldob    4(g8),r8                # r8 = host transfer length
        bswap   r6,r6
        mov     0,r7                    # Init the upper SDA word
        and     r5,r6,r6                # r6 = SDA from host
        cmpobne 0,r8,.read6_100         # Jif transfer length not 0
        ldconst 256,r8                  # set transfer length to 256
.read6_100:
        b       read_com                # and process through common logic
#
#******************************************************************************
#
#  NAME: mag1$write6
#
#  PURPOSE:
#       Processes a Write (6) command received from a host.
#
#  DESCRIPTION:
#       Sets up to forward the Write request to the MAGNITUDE.
#
#  CALLING SEQUENCE:
#       call    mag1$write6
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
mag1$write6:
        ld      (g8),r6
        ldconst 0x001fffff,r5
        ldob    4(g8),r8                # r8 = host transfer length
        bswap   r6,r6
        mov     vroutput,r4             # r4 = VRP request function code
        and     r5,r6,r6                # r6 = SDA from host
        mov     0,r7                    # Zero out the upper SDA word
        cmpobne 0,r8,.write6_100        # Jif transfer length not 0
        ldconst 256,r8                  # set transfer length to 256
.write6_100:
        b       write_com               # and process through common logic
#
#******************************************************************************
#
#  NAME: mag1$modesns
#
#  PURPOSE:
#       Processes a MODE SENSE (6) command received from a host.
#
#  DESCRIPTION:
#       Returns the Mode Sense data to the issuing host for
#       the specified LUN.
#
#  CALLING SEQUENCE:
#       call    mag1$modesns
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
modesense_tbl1:
        .byte   dtxferi,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # # SGL descriptors
        .short  0                       # sense length
#
modesense_tbl2:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invf1             # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
modesense_tbl3:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_nosaveparm        # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
        .text
#
mag1$modesns:
        ldob    2(g8),r4                # r4 = mode page from CDB
        ldconst 0x3f,r5                 # r5 = page code mask
        ld      ilm_wkenv(g6),r15       # r15 = pointer to working environment
                                        #  table
        shro    6,r4,r3                 # isolate page control
        and     3,r3,r3                 # r3 = page control
        and     r5,r4,r4                # r4 = page code
        cmpobe  0x00,r3,.modesns1_500   # Jif current parameters
#
# NOTE: FALLS THROUGH!
#
#****************************************************************************
#
# --- Page control not current values
#
#  INPUT:
#       r3 = page control
#       r4 = page code
#       r5 = 0x3f
#       r15 = ilm_wkenv address
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#****************************************************************************
#
# NOTE: FALLS THROUGH!
#
        lda     modesns1,r15            # r15 = pointer to default MODE
                                        #  SENSE data
        cmpobe  0x02,r3,.modesns1_500   # Jif default parameters
        lda     chgmodesns1,r15         # r15 = pointer to changeable MODE
                                        #  SENSE data
        cmpobe  0x01,r3,.modesns1_500   # Jif changeable parameters
#
# --- Saved MODE SENSE parameters specified (not supported at this time)
#
        ldq     modesense_tbl3,r4       # load op. values into regs.
        ld      modesense_tbl3+16,r8
        b       .modesns1_1000          # and we're out of here
#
#****************************************************************************
#
# --- Process request for MODE SENSE data interface
#
#  INPUT:
#       r3 = page control
#       r4 = page code
#       r5 = 0x3f
#       r15 = base address of MODE SENSE data area
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#****************************************************************************
#
# --- process request for MODE SENSE data
#
.modesns1_500:
        lda     ms1_sz(r15),r13         # r13 = pointer to page data
        lda     ms_sz-ms1_sz,r14        # r14 = size of page data area
        cmpobe  r5,r4,.modesns1_com     # Jif all pages specified
.modesns1_600:
        ldob    1(r13),r14              # r14 = page length
        ldob    (r13),r5                # r5 = page code
        addo    2,r14,r14
        cmpobe  r5,r4,.modesns1_com     # Jif page # matches
        addo    r14,r13,r13             # inc. to next page
        cmpobne 0,r5,.modesns1_600      # Jif not last page supported
#
# --- specified page code not supported processing
#
        ldq     modesense_tbl2,r4       # load op. values into regs.
        ld      modesense_tbl2+16,r8
        b       .modesns1_1000
#
#***************************************************************************
#
# --- Common current page MODE SENSE data processing
#
#  INPUT:
#       r13 = pointer to page data in working environment table
#       r14 = size of page data
#       r15 = ilm_wkenv pointer
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#***************************************************************************
#
.modesns1_com:
        ldob    4(g8),r4                # r4 = CDB allocation length
        ldob    1(g8),r3                # r3 = MODE SENSE CDB byte #1
        ldconst ms1_sz,r9               # r9 = header + block descriptor
        cmpobne 0,r4,.modesns1_com_05   # Jif CDB alloc. length <> 0
        ldq     tur_tbl1,r4             # just return good status
        ld      tur_tbl1+16,r8
        b       .modesns1_1000

.modesns1_com_05:
        bbc     3,r3,.modesns1_com_10   # Jif DBD bit cleared
        ldconst 4,r9                    # send header only
.modesns1_com_10:
        addo    r9,r14,g0               # g0 = buffer size for MODE SENSE data
        mov     g0,r11                  # r11 = total size of MODE SENSE data
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        ld      sghdrsiz+sg_addr(g0),r12 # r12 = MODE SENSE data address
        st      r11,sghdrsiz+sg_len(g0) # save length of MODE SENSE data
        bbc     3,r3,.modesns1_com_20   # Jif DBD bit cleared
        ld      (r15),r8                # r8 = header data
        ldconst 0x00ffff00,r9           # r9 = mask for MODE SENSE length &
                                        #  block descriptor length
        and     r9,r8,r8                # block descriptor length = 00
        subo    1,r11,r9                # r9 = MODE SENSE length-1
        or      r9,r8,r8                # add in MODE SENSE length
        st      r8,(r12)                # save header data
        addo    4,r12,r12               # inc. to next data area
        b       .modesns1_com_30

.modesns1_com_20:
        ldt     (r15),r8                # r8-r10 = header + block desc. data
        ldconst 0xffffff00,r6           # r6 = mask for MODE SENSE length
        subo    1,r11,r7                # r7 = MODE SENSE length-1
        and     r6,r8,r8                # mask off MODE SENSE length
        or      r7,r8,r8                # add in MODE SENSE length-1
        stt     r8,(r12)                # save in MODE SENSE buffer
        addo    12,r12,r12              # inc. to next data area
.modesns1_com_30:
        ldob    (r13),r8                # copy page data into buffer
        addo    1,r13,r13               # inc. page data pointer
        subo    1,r14,r14               # dec. page data counter
        stob    r8,(r12)                # save page data in buffer
        addo    1,r12,r12               # inc. buffer pointer
        cmpobne 0,r14,.modesns1_com_30  # Jif more page data bytes to copy
        cmpo    r4,r11                  # check if alloc. length < mode sense
                                        #  size
        sell    r11,r4,r11              # r11 = size of transfer to host
        st      r11,sghdrsiz+sg_len(g0) # save size of data in SGL
        ldq     modesense_tbl1,r4       # load op. values into regs.
        ld      modesense_tbl1+16,r8
        mov     g0,r6                   # r6 = SGL pointer
.modesns1_1000:
        b       mag1$cmdcom
#
#******************************************************************************
#
#  NAME: mag1$modesns10
#
#  PURPOSE:
#       Processes a MODE SENSE (10) command received from a host.
#
#  DESCRIPTION:
#       Returns the Mode Sense data to the issuing host for
#       the specified LUN.
#
#  CALLING SEQUENCE:
#       call    mag1$modesns10
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
modesense1_tbl1:
        .byte   dtxferi,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # # SGL descriptors
        .short  0                       # sense length
#
modesense1_tbl2:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invf1             # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
modesense1_tbl3:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_nosaveparm        # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
        .text
#
mag1$modesns10:
        ldconst 0,g13                   # g13 = 0
        ldob    2(g8),r4                # r4 = mode page from CDB
        ldconst 0x3f,r5                 # r5 = page code mask
        ld      ilm_wkenv(g6),r15       # r15 = pointer to working environment
                                        #  table
        shro    6,r4,r3                 # isolate page control
        and     3,r3,r3                 # r3 = page control
        and     r5,r4,r4                # r4 = page code
        cmpobe  0x00,r3,.modesns10_500  # Jif current parameters
#
# NOTE: FALLS THROUGH!
#
#****************************************************************************
#
# --- Page control not current values
#
#  INPUT:
#
#       r3 = page control
#       r4 = page code
#       r5 = 0x3f
#       r15 = ilm_wkenv address
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#****************************************************************************
#
# NOTE: FALLS THROUGH!
#
        lda     modesns1,r15            # r15 = pointer to default MODE
                                        #  SENSE data
        cmpobe  0x02,r3,.modesns10_500  # Jif default parameters
        lda     chgmodesns1,r15         # r15 = pointer to changeable MODE
                                        #  SENSE data
        cmpobe  0x01,r3,.modesns10_500  # Jif changeable parameters
#
# --- Saved MODE SENSE parameters specified (not supported at this time)
#
        ldq     modesense1_tbl3,r4      # load op. values into regs.
        ld      modesense1_tbl3+16,r8
        b       .modesns10_1000         # and we're out of here
#
#****************************************************************************
#
# --- Process request for MODE SENSE data interface
#
#  INPUT:
#
#       r3 = page control
#       r4 = page code
#       r5 = 0x3f
#       r15 = base address of MODE SENSE data area
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#****************************************************************************
#
# --- process request for MODE SENSE data
#
.modesns10_500:
        lda     ms1_sz(r15),r13         # r13 = pointer to page data
        lda     ms_sz-ms1_sz,r14        # r14 = size of page data area
        cmpobe  r5,r4,.modesns10_com    # Jif all pages specified
.modesns10_600:
        ldob    1(r13),r14              # r14 = page length
        ldob    (r13),r5                # r5 = page code
        addo    2,r14,r14
        cmpobe  r5,r4,.modesns10_com    # Jif page # matches
        addo    r14,r13,r13             # inc. to next page
        cmpobne 0,r5,.modesns10_600     # Jif not last page supported
#
# --- specified page code not supported processing
#
        ldq     modesense1_tbl2,r4      # load op. values into regs.
        ld      modesense1_tbl2+16,r8
        b       .modesns10_1000
#
#***************************************************************************
#
# --- Common current page MODE SENSE data processing
#
#  INPUT:
#
#       r13 = pointer to page data in working environment table
#       r14 = size of page data
#       r15 = ilm_wkenv pointer
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#***************************************************************************
#
.modesns10_com:
        ldob    7(g8),r4                # r4 = CDB allocation length (MSB)
        shlo    8,r4,r4
        ldob    8(g8),r3                # r3 = CDB allocation length (LSB)
        or      r3,r4,r4                # r4 = CDB allocation length
        ldob    1(g8),r3                # r3 = MODE SENSE CDB byte #1
        ldconst ms10_sz,r9              # r9 = header + block descriptor
        cmpobne 0,r4,.modesns10_com_05  # Jif CDB alloc. length <> 0
        ldq     tur_tbl1,r4             # just return good status
        ld      tur_tbl1+16,r8
        b       .modesns10_1000
#
.modesns10_com_05:
        bbc     3,r3,.modesns10_com_10  # Jif DBD bit cleared
        ldconst 8,r9                    # send header only
#
.modesns10_com_10:
        addo    r9,r14,g0               # g0 = buffer size for MODE SENSE data
        mov     g0,r11                  # r11 = total size of MODE SENSE data
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        ld      sghdrsiz+sg_addr(g0),r12# r12 = MODE SENSE data address
        st      r11,sghdrsiz+sg_len(g0) # save length of MODE SENSE data
#
        stob    0,(r12)                 # clear MSB of sense length
        subo    2,r11,r6                # r6 = LSB of sense length
        stob    r6,1(r12)               # save LSB of sense length
        ldob    1(r15),r8               # r8 = medium type code
        stob    r8,2(r12)               # save medium type code
        ldob    2(r15),r8               # r8 = WP/DPO flag byte
        stob    r8,3(r12)               # save WP/DPO flag byte
        st      0,4(r12)                # clear remaining header bytes
        subo    8,r9,r8                 # r8 = remaining header length
        stob    r8,7(r12)               # save remaining header length
        addo    8,r12,r12               # inc. to next data area
        bbs     3,r3,.modesns10_com_30  # Jif DBD bit set
#
        ldl     4(r15),r8               # r8-r9 = block desc. data
        stl     r8,(r12)                # save in MODE SENSE buffer
        addo    8,r12,r12               # inc. to next data area
#
.modesns10_com_30:
        ldob    (r13),r8                # copy page data into buffer
        addo    1,r13,r13               # inc. page data pointer
        subo    1,r14,r14               # dec. page data counter
        stob    r8,(r12)                # save page data in buffer
        addo    1,r12,r12               # inc. buffer pointer
        cmpobne 0,r14,.modesns10_com_30 # Jif more page data bytes to copy
        cmpo    r4,r11                  # check if alloc. length < mode sense
                                        #  size
        sell    r11,r4,r11              # r11 = size of transfer to host
        st      r11,sghdrsiz+sg_len(g0) # save size of data in SGL
        ldq     modesense1_tbl1,r4      # load op. values into regs.
        ld      modesense1_tbl1+16,r8
        mov     g0,r6                   # r6 = SGL pointer
#
.modesns10_1000:
        b       mag1$cmdcom
#
#******************************************************************************
#
#  NAME: mag1$repluns
#
#  PURPOSE:
#       Processes a Report LUNs command received from a host.
#
#  DESCRIPTION:
#       Returns the appropriate LUNs data to the issuing host.
#
#  CALLING SEQUENCE:
#       call    mag1$repluns
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
# --- Invalid allocation length in CDB response op. value table
#
        .data
replun_tbl1:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invf1             # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
# --- Report LUNS response op. value table
#
replun_tbl2:
        .byte   dtxferi,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
mag1$repluns:
        ld      6(g8),r8                # r8 = allocation length from CDB
        bswap   r8,r8
        cmpoble 16,r8,.repl10           # Jif valid allocation length
        ldq     replun_tbl1,r4          # load op. values into regs.
        ld      replun_tbl1+16,r8
        b       .repl1000               # and finish processing command
#
# --- Valid allocation length in CDB
#       r8 = allocation length from CDB
# --- Set up to identify how many LUNs are supported for initiator
#
.repl10:
        ldconst LUNMAX,r4               # r4 = max. # LUNs supported
        lda     im_ilmtdir(g5),r5       # r5 = pointer into ILMT directory
        mov     0,r6                    # r6 = # LUNs supported for initiator
.repl20:
        ld      (r5),r7                 # r7 = ILMT
        lda     4(r5),r5                # inc. to next ILMT in directory
        subo    1,r4,r4                 # dec. # ILMTs to check for
        cmpobe  0,r7,.repl30            # Jif no ILMT assoc. with LUN
        addo    1,r6,r6                 # inc. # LUNs supported
.repl30:
        cmpobne 0,r4,.repl20            # Jif more ILMTs to check for
        mulo    8,r6,r9
        lda     8(r9),r9                # r9 = size of response data to initiator
        mov     r9,g0                   # g0 = size of buffer for response data
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        subo    8,r9,r10                # r10 = LUN list length
        bswap   r10,r10
        mov     0,r11                   # r11 = reserved
        ld      sghdrsiz+sg_addr(g0),g1 # g1 = buffer address
        stl     r10,(g1)                # save response header in buffer
        lda     8(g1),g1                # inc. past header area of data
        mov     0,r11                   # r10 = 4 LSB of LUN ids.
        ldconst LUNMAX,r4               # r4 = max. # LUNs supported
        lda     im_ilmtdir(g5),r5       # r5 = pointer into ILMT directory
        mov     0,r6                    # r6 = LUN #
.repl40:
        ld      (r5),r7                 # r7 = ILMT
        lda     4(r5),r5                # inc. to next ILMT in directory
        subo    1,r4,r4                 # dec. # ILMTs to check for
        cmpobe  0,r7,.repl50            # Jif no ILMT assoc. with LUN

# Unconditionally clear the "reported luns data changed" unit attention here.

        ldob    ilm_flag2(r7),r12       # r12= ilm_flag2 byte
        clrbit  5,r12,r12               # clear replunsdata_chgd bit in ilm_flag2 byte
        stob    r12,ilm_flag2(r7)       # save updated ilm_flag2 byte

        shlo    8,r6,r10                # r10 = LUN # to place in response data
        stl     r10,(g1)                # save LUN # in response data
        lda     8(g1),g1                # inc. to next record in response data
.repl50:
        addo    1,r6,r6                 # inc. LUN #
        cmpobne 0,r4,.repl40            # Jif more ILMTs to check for
        cmpo    r8,r9                   # check if alloc. length < my response
                                        #  data
        sell    r9,r8,r9                # r9 = size of transfer to initiator
        st      r9,sghdrsiz+sg_len(g0)  # save length in SGL
        ldq     replun_tbl2,r4          # load op. values into regs.
        ld      replun_tbl2+16,r8
        mov     g0,r6                   # r6 = SGL pointer
.repl1000:
        b       mag1$cmdcom             # and finish processing command
#
#******************************************************************************
#
#  NAME: mag1$vfymedia and mag1$verify_16
#
#  PURPOSE:
#       Processes a Verify Media (10) or (16)  command received from a host.
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
mag1$vfymedia:
c       mag1_vfymedia(g8, 10, g5, g6, g7, g9);
        ret

mag1$verify_16:
c       mag1_vfymedia(g8, 16, g5, g6, g7, g9);
        ret
#
#******************************************************************************
#
#  NAME: mag1$reqsns
#
#  PURPOSE:
#       Processes a Request Sense command received from a host.
#
#  DESCRIPTION:
#       Returns the appropriate sense data to the issuing host.
#
#  CALLING SEQUENCE:
#       call    mag1$reqsns
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
sense_tbl1:
        .byte   dtxferi,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # # SGL descriptors
        .short  0                       # sense length
#
sense_tbl2:
        .byte   dtxfern,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
sense_tbl3:
        .byte   dtxferi,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invf1             # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
mag1$reqsns:
        ldob    MAGD_SCSI_WHQL,r5       # is WHQL compliance active??
        cmpobe  0,r5,.rsns1_10          # Jif no
        ldob    1(g8),r6                # r6 = To check the DESC bit
        cmpobe  0,r6,.rsns1_10          # Jif  zero value
        ldq     sense_tbl3,r4           # load op. values into regs.
        ld      sense_tbl3+16,r8
        b       .rsns1_1000              # and just return status

.rsns1_10:
        ldob    4(g8),r3                # r3 = alloc. length from CDB
        mov     0,g0                    # allocate an SGL
        lda     sensesize,r9            # r9 = my sense data size
        cmpobne 0,r3,.rsns1_100         # Jif alloc. length in CDB non-zero
        ldq     sense_tbl2,r4           # load op. values into regs.
        ld      sense_tbl2+16,r8
        b       .rsns1_1000             # and complete command processing
#
.rsns1_100:
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        lda     sense_nosense,r4        # r4 = SENSE data base address
        st      r4,sghdrsiz+sg_addr(g0) # save sense data address in SGL
        cmpo    r3,r9                   # check if alloc. length < SENSE size
        sell    r9,r3,r9                # r9 = size of transfer to host
        st      r9,sghdrsiz+sg_len(g0)  # save size of data in SGL
        ldq     sense_tbl1,r4           # load op. values into regs.
        ld      sense_tbl1+16,r8
        mov     g0,r6                   # r6 = SGL pointer
.rsns1_1000:
        b       mag1$cmdcom             # and finish processing command
#
#******************************************************************************
#
#  NAME: mag1$modesel
#
#  PURPOSE:
#       Processes a MODE SELECT (6) command received from a host.
#
#  DESCRIPTION:
#       Validates the MODE SELECT data sent by the initiator and
#       if not valid returns the appropriate error to the initiator.
#       If valid, changes the MODE SENSE data for the initiator as
#       specified in the MODE SELECT data.
#
#  CALLING SEQUENCE:
#       call    mag1$modesel
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
modesel_tbl1:
        .byte   dtxferc,scnorm,0,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # # SGL descriptors
        .short  0                       # sense length
#
modesel_tbl2:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invf1             # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
        .text
#
mag1$modesel:
        ldob    1(g8),r5                # r5 = CDB byte with SP flag
        bbc     0,r5,.modesel10         # Jif SP flag 0
        ldq     modesel_tbl2,r4         # load op. values into regs.
        ld      modesel_tbl2+16,r8
        b       .modesel1000            # and finish processing command

.modesel10:
        ldob    4(g8),g0                # g0 = parameter list length from CDB
        cmpobne 0,g0,.modesel20         # Jif length <> 0
        ldq     tur_tbl1,r4             # just return good status
        ld      tur_tbl1+16,r8
        b       .modesel1000

.modesel20:
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        ldq     modesel_tbl1,r4         # load op. values into regs.
        ld      modesel_tbl1+16,r8
        lda     task_etbl8,r9           # r9 = task event handler table
        mov     g0,r6                   # r6 = SGL address
        st      r9,inl2_ehand(g9)       # save task event handler table in
                                        #  task
        b       mag2$cmdcom             # and finish processing command

.modesel1000:
        b       mag1$cmdcom             # and finish processing command
#
#******************************************************************************
#
#  NAME: mag1$modesel10
#
#  PURPOSE:
#       Processes a MODE SELECT (10) command received from a host.
#
#  DESCRIPTION:
#       Validates the MODE SELECT data sent by the initiator and
#       if not valid returns the appropriate error to the initiator.
#       If valid, changes the MODE SENSE data for the initiator as
#       specified in the MODE SELECT data.
#
#  CALLING SEQUENCE:
#       call    mag1$modesel10
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
mag1$modesel10:
        ldob    1(g8),r5                # r5 = CDB byte with SP flag
        bbc     0,r5,.modeselx10        # Jif SP flag 0
        ldq     modesel_tbl2,r4         # load op. values into regs.
        ld      modesel_tbl2+16,r8
        b       .modeselx1000           # and finish processing command

.modeselx10:
        ldos    7(g8),g0                # g0 = parameter list length from CDB
        bswap   g0,g0
        shro    16,g0,g0
        cmpobne 0,g0,.modeselx20        # Jif length <> 0
        ldq     tur_tbl1,r4             # just return good status
        ld      tur_tbl1+16,r8
        b       .modeselx1000

.modeselx20:
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        ldq     modesel_tbl1,r4         # load op. values into regs.
        ld      modesel_tbl1+16,r8
        lda     task_etbl8,r9           # r9 = task event handler table
        mov     g0,r6                   # r6 = SGL address
        st      r9,inl2_ehand(g9)       # save task event handler table in
                                        #  task
        b       mag2$cmdcom             # and finish processing command

.modeselx1000:
        b       mag1$cmdcom             # and finish processing command
#
#******************************************************************************
#
#  NAME: mag1$reserve6
#
#  PURPOSE:
#       Processes a Reserve(6) command received from a host.
#
#  DESCRIPTION:
#       Attempts to reserve the device (LUN) for the issuing
#       host and if successful returns good status to the issuing
#       host. If unsuccessful, returns check condition status to the
#       issuing host with appropriate sense data indicating the LUN
#       could not be reserved.
#
#  CALLING SEQUENCE:
#       call    mag1$reserve6
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
reserv6_tbl1:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invf1             # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
reserv6_tbl2:
        .byte   dtxfern,scresc,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
reserv6_tbl3:
        .byte   dtxfern,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
mag1$reserve6:
        ldob    1(g8),r4                # r4 = CDB byte #2
        ldob    2(g8),r5                # r5 = CDB byte #3
        ldconst 0x1f,r6                 # r6 = mask for extent flag, 3rd party
                                        #  reserve flags
        and     r6,r4,r6                # check if any unsupported flags set
        cmpobne 0,r6,.reserv6_10        # Jif unsupported flags set
        cmpobe  0,r5,.reserv6_100       # Jif reserved byte 0
.reserv6_10:
        ldq     reserv6_tbl1,r4         # load op. values into regs.
        ld      reserv6_tbl1+16,r8
        b       mag1$cmdcom             # and return error to initiator

.reserv6_100:
        ld      ilm_vdmt(g6),g3         # g3 = assoc. VDMT address
        ld      vdm_resv(g3),r4         # r4 = pointer to RESV (for persistent
                                        # reservations)
        cmpobne 0,r4,.reserv6_150       # Jif a RESV is present. Return Resv. Conf.
        ld      vdm_rilmt(g3),r4        # r4 = ILMT of local reserving
                                        #  initiator
        cmpobe  0,r4,.reserv6_300       # Jif no local reserving initiator
        cmpobe  g6,r4,.reserv6_200      # Jif this initiator has local
#
# --- Device reserved locally by another initiator
#
.reserv6_150:
        ldq     reserv6_tbl2,r4         # load op. values into regs.
        ld      reserv6_tbl2+16,r8
        b       mag1$cmdcom             # and return reservation conflict
                                        #  status
#
# --- Device reserved locally by requesting initiator
#
.reserv6_200:
        ldq     reserv6_tbl3,r4         # load op. values into regs.
        ld      reserv6_tbl3+16,r8
        b       mag1$cmdcom             # and return successful status
#
# --- Device not reserved locally by any initiator. Reserve device
#       locally and make request to reserve it globally.
#
.reserv6_300:
        call    mag$locreserve          # locally reserve device & make a
                                        #  VRP reserve request
        ret
#
#******************************************************************************
#
#  NAME: mag1$reserve10
#
#  PURPOSE:
#       Processes a Reserve(10) command received from a host.
#
#  DESCRIPTION:
#       Attempts to reserve the device (LUN) for the issuing
#       host and if successful returns good status to the issuing
#       host. If unsuccessful, returns check condition status to the
#       issuing host with appropriate sense data indicating the LUN
#       could not be reserved.
#
#  CALLING SEQUENCE:
#       call    mag1$reserve10
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
mag1$reserve10:
        ldob    1(g8),r4                # r4 = CDB byte #2
        ld      2(g8),r5                # r5 = CDB bytes #3-6
        ld      6(g8),r7                # r7 = CDB bytes #7-10
        ldconst 0x1f,r6                 # r6 = mask for extent flag, 3rd party
                                        #  reserve flags
        and     r6,r4,r6                # check if any unsupported flags set
        cmpobne 0,r6,.reserv10_10       # Jif unsupported flags set
        cmpobne 0,r5,.reserv10_10       # Jif reserved bytes not 0
        ldconst 0xffffffc0,r6           # r6 = mask for other reserved bytes
        and     r6,r7,r6                # check if other reserved bytes 0
        cmpobe  0,r6,.reserv10_100      # Jif no other reserved bytes set
.reserv10_10:
        ldq     reserv6_tbl1,r4         # load op. values into regs.
        ld      reserv6_tbl1+16,r8
        b       mag1$cmdcom             # and return error to initiator

.reserv10_100:
        ld      ilm_vdmt(g6),g3         # g3 = assoc. VDMT address
        ld      vdm_resv(g3),r4         # r4 = pointer to RESV (for persistent
                                        # reservations)
        cmpobne 0,r4,.reserv10_150      # Jif a RESV is present. Return Resv. Conf.
        ld      vdm_rilmt(g3),r4        # r4 = ILMT of local reserving
                                        #  initiator
        cmpobe  0,r4,.reserv10_300      # Jif no local reserving initiator
        cmpobe  g6,r4,.reserv10_200     # Jif this initiator has local
                                        #  reservation
#
# --- Device reserved locally by another initiator
#
.reserv10_150:
        ldq     reserv6_tbl2,r4         # load op. values into regs.
        ld      reserv6_tbl2+16,r8
        b       mag1$cmdcom             # and return reservation conflict
                                        #  status
#
# --- Device reserved locally by requesting initiator
#
.reserv10_200:
        ldq     reserv6_tbl3,r4         # load op. values into regs.
        ld      reserv6_tbl3+16,r8
        b       mag1$cmdcom             # and return successful status
#
# --- Device not reserved locally by any initiator. Reserve device
#       locally and make request to reserve it globally.
#
.reserv10_300:
        call    mag$locreserve          # locally reserve device & make a
                                        #  VRP reserve request
        ret
#
#******************************************************************************
#
#  NAME: mag1$release6
#
#  PURPOSE:
#       Processes a Release(6) command received from a host.
#
#  DESCRIPTION:
#       Releases the device if the device was reserved to the
#       issuing host, else returns check condition status to
#       the issuing host indicating the device was not reserved
#       to it.
#
#  CALLING SEQUENCE:
#       call    mag1$release6
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
releas6_tbl1:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invf1             # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
releas6_tbl2:
        .byte   dtxfern,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
releas6_tbl3:
        .byte   dtxfern,scresc,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
mag1$release6:
        ldob    1(g8),r4                # r4 = CDB byte #2
        ld      2(g8),r5                # r5 = CDB bytes #3-6
        ldconst 0x1f,r6                 # r6 = CDB byte #2 mask
        and     r6,r4,r6                # check if unsupported flags set
        cmpobne 0,r6,.releas6_10        # Jif unsupported flags set
        ldconst 0xfffffffc,r6           # r6 = reserved bytes mask
        and     r6,r5,r6                # check if reserved bytes set
        cmpobe  0,r6,.releas6_100       # Jif reserved bytes 0
.releas6_10:
        ldq     releas6_tbl1,r4         # load op. values into regs.
        ld      releas6_tbl1+16,r8
        b       mag1$cmdcom             # and return error to initiator

.releas6_100:
        ld      ilm_vdmt(g6),g3         # g3 = assoc. VDMT
        ld      vdm_resv(g3),r4         # r4 = pointer to RESV (for persistent
                                        # reservations)
        cmpobne 0,r4,.releas6_150       # Jif a RESV is present. Return Resv. Conf.
        ld      vdm_rilmt(g3),r4        # r4 = ILMT having local reserve
                                        #  Note: It better be ME!!!
        cmpobe  r4,g6,.releas6_200      # Jif if me
        ldq     releas6_tbl2,r4         # load op. values into regs.
        ld      releas6_tbl2+16,r8
        b       mag1$cmdcom             # and just return successful status

.releas6_150:
        ldq     releas6_tbl3,r4         # load op. values into regs.
        ld      releas6_tbl3+16,r8
        b       mag1$cmdcom             # and return reservation conflict

.releas6_200:
        call    mag$locrelease          # release local reserve & global
                                        #  reserve
        ret
#
#******************************************************************************
#
#  NAME: mag1$release10
#
#  PURPOSE:
#       Processes a Release(10) command received from a host.
#
#  DESCRIPTION:
#       Releases the device if the device was reserved to the
#       issuing host, else returns check condition status to
#       the issuing host indicating the device was not reserved
#       to it.
#
#  CALLING SEQUENCE:
#       call    mag1$release10
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
mag1$release10:
        ldob    1(g8),r4                # r4 = CDB byte #2
        ld      2(g8),r5                # r5 = CDB bytes #3-6
        ldconst 0x1f,r6                 # r6 = CDB byte #2 mask
        ld      6(g8),r7                # r7 = CDB bytes #7-10
        and     r6,r4,r6                # check if unsupported flags set
        cmpobne 0,r6,.releas10_10       # Jif unsupported flags set
        cmpobne 0,r5,.releas10_10       # Jif reserved bytes not 0
        ldconst 0xfffffffc,r6           # r6 = reserved bytes mask
        and     r6,r7,r6                # check if reserved bytes set
        cmpobe  0,r6,.releas10_100      # Jif reserved bytes 0
.releas10_10:
        ldq     releas6_tbl1,r4         # load op. values into regs.
        ld      releas6_tbl1+16,r8
        b       mag1$cmdcom             # and return error to initiator

.releas10_100:
        ld      ilm_vdmt(g6),g3         # g3 = assoc. VDMT
        ld      vdm_resv(g3),r4         # r4 = pointer to RESV (for persistent
                                        # reservations)
        cmpobne 0,r4,.releas10_150      # Jif a RESV is present. Return Resv. Conf.
        ld      vdm_rilmt(g3),r4        # r4 = ILMT having local reserve
                                        #  Note: It better be ME!!!
        cmpobe  r4,g6,.releas10_200     # Jif if me
        ldq     releas6_tbl2,r4         # load op. values into regs.
        ld      releas6_tbl2+16,r8
        b       mag1$cmdcom             # and just return successful status

.releas10_150:
        ldq     releas6_tbl3,r4         # load op. values into regs.
        ld      releas6_tbl3+16,r8
        b       mag1$cmdcom             # and return reservation conflict

.releas10_200:
        call    mag$locrelease          # release local reserve & global
                                        #  reserve
        ret
#
#******************************************************************************
#
#  NAME: mag1$fmtunit
#
#  PURPOSE:
#       Processes a Format Unit command received from a host.
#
#  DESCRIPTION:
#       Decodes the specifics of the Format Unit request and executes
#       them. We first determine whether a default format request is
#       specified. If so, the initiator does not send any additional
#       information specific to the request. If the initiator sends
#       additional information specific to the format, we receive the
#       data and process it. We disregard any defect specification data
#       since it doesn't pertain to our operation. If the initiator
#       specifies an initialization pattern, we conform to the requested
#       pattern. This command operates by issuing getting a write buffer from
#       Cache for the subsequent WRITE to the Virtual Disk. The buffer is then
#       filled with the appropriate data pattern and a VRP is sent to MAGNITUDE
#       to write the data pattern to the virtual disk. This continues until the
#       entire Virtual Disk has been formatted according to the initiator's
#       specification or until the command has been aborted.
#
#  CALLING SEQUENCE:
#       call    mag1$fmtunit
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
fmtunit_tbl1:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invf1             # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
fmtunit_tbl2:
        .byte   dtxferc,scnorm,0,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
mag1$fmtunit:
        ldob    2(g8),r5                # r5 = CDB byte #2
        ldob    5(g8),r6                # r6 = CDB byte #5
        ldob    1(g8),r4                # r4 = CDB byte #1
        cmpobne 0,r5,mtunit10           # Jif CDB byte #2 non-zero
        cmpobe  0,r6,mtunit20           # Jif CDB byte #5 zero
mtunit10:
        ldq     fmtunit_tbl1,r4         # load op. values into regs.
        ld      fmtunit_tbl1+16,r8
        b       mag1$cmdcom             # and finish up routine here and now!

mtunit20:
        lda     ILTBIAS(g9),r9          # r9 = pri. ILT at inl3 nest level. Need
                                        #  to set this level up to store FORMAT
                                        #  UNIT process info.
        ldconst fupat_dflt,r8           # r8 = default pattern type code
        mov     0,r10                   # r10 = 0
        stob    r8,fu3_pattype(r9)      # init. default pattern type
        stob    r10,fu3_flags(r9)       # clear flags byte
        st      r10,fu3_patsgl(r9)      # clear pattern SGL/buffer address
        st      r10,fu3_fupmt(r9)       # clear FUPMT address
        bbs     4,r4,mtunit50           # Jif FORMAT UNIT data specified
        b       mag$startfu             # start FORMAT UNIT processing and exit
#
# --- FORMAT UNIT data specified in CDB - go get it!
#
mtunit50:
        ldconst 4,g0                    # g0 = size of buffer for Defect
                                        #  List Header data
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        ldq     fmtunit_tbl2,r4         # load op. values into regs.
        ld      fmtunit_tbl2+16,r8
        lda     task_etbl12a,r9         # r9 = task event handler table
        mov     g0,r6                   # r6 = SGL address
        st      r9,inl2_ehand(g9)       # save task event handler table in
                                        #  task
        b       mag2$cmdcom             # and continue processing command
#
#******************************************************************************
#
#  NAME: mag1$writevfy
#
#  PURPOSE:
#       Processes a Write & Verify command received from a host.
#
#  DESCRIPTION:
#       Sets up to forward the Write & Verify request to the MAGNITUDE.
#
#  CALLING SEQUENCE:
#       call    mag1$writevfy
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
mag1$writevfy:
        ld      6(g8),r8                # isolate transfer length from CDB
        lda     0x00ffff00,r5
        ld      2(g8),r6                # r6 = SDA of host request
        mov     vroutputv,r4            # r4 = VRP request code
        and     r5,r8,r8
        mov     0,r7                    # Zero out the upper SDA word
        bswap   r8,r8
        bswap   r6,r6
        shro    8,r8,r8                 # r8 = transfer length from CDB
        b       write_com               # and process through common logic
#
#******************************************************************************
#
#  NAME: mag1$writevfy_16
#
#  PURPOSE:
#       Processes a Write & Verify (16) command received from a host.
#
#  DESCRIPTION:
#       Sets up to forward the Write & Verify request to the MAGNITUDE.
#
#  CALLING SEQUENCE:
#       call    mag1$writevfy
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
mag1$writevfy_16:
        ld      10(g8),r8               # transfer length from CDB
        bswap   r8,r8                   # Convert length to little endia
        ldl     2(g8),r6                # r6,r7 = SDA of host request
        mov     vroutputv,r4            # r4 = VRP request code
        mov     r6,r3                   # move r6 to r3
        bswap   r7,r6                   # byte swap r7 into r6
        bswap   r3,r7                   # byte swap r6 (in r3) into r7
#       r4 = VRP request function code
#       r6,r7 = SDA of host request (little endian format)
#       r8 = transfer length (little endian)
        b       write_com               # and process through common logic
#
#******************************************************************************
#
#  NAME: mag1$snddiag
#
#  PURPOSE:
#       Processes a Send Diagnostics command received from a host.
#
#  DESCRIPTION:
#       NOTE: This command verifies that the reserved bits in the CDB
#               are zero and then simply responds with successful status
#               back to the initiator for now.
#
#  CALLING SEQUENCE:
#       call    mag1$snddiag
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
snddiag_tbl1:
        .byte   dtxfern,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
snddiag_tbl2:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invf1             # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
        .text
#
mag1$snddiag:
        ld      (g8),r4                 # r4 = first 4 bytes of CDB
        ldconst 0x00e0ff00,r5           # r5 = reserved bit mask
        bswap   r4,r4                   # normalize first 4 bytes of CDB
        and     r5,r4,r4                # mask off non-reserved bits
        cmpobe  0,r4,.snddiag_100       # Jif no reserved bits set
        ldq     snddiag_tbl2,r4         # load op. values into regs.
        ld      snddiag_tbl2+16,r8
        b       .snddiag_1000

.snddiag_100:
        ldq     snddiag_tbl1,r4         # load op. values into regs.
        ld      snddiag_tbl1+16,r8
.snddiag_1000:
        b       mag1$cmdcom             # and finish processing command
#
#******************************************************************************
#
#  NAME: mag1$rcvdiag
#
#  PURPOSE:
#       Processes a Receive Diagnostics command received from a host.
#
#  DESCRIPTION:
#       NOTE: This command is NOT supported at this time!!!!!!!!!
#
#  CALLING SEQUENCE:
#       call    mag1$rcvdiag
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
mag1$rcvdiag:
        ldconst magd_sft5,r7            # r7 = error code to log
        mov     g0,r3                   # Save g0
        lda     magd_sft,g0             # g0 = Software Fault Log Area
        st      r7,efa_ec(g0)           # Save the Error Code
        st      g5,efa_data(g0)         # Save the IMT
        st      g6,efa_data+4(g0)       # Save the ILMT pointer
        ldq     (g8),r4                 # Load the CDB
        stq     r4,efa_data+8(g0)       # Save the CDB
        ldconst 28,r7                   # Number of bytes saved (ec + data)
        st      r7,mle_len(g0)          # Save the number of bytes to send
        call    M$soft_flt              # Error Trap or Log failure
        mov     r3,g0                   # Restore g0
        ret
#
#******************************************************************************
#
#  NAME: mag1$logsns
#
#  PURPOSE:
#       Processes a Log Sense command received from a host.
#
#  DESCRIPTION:
#       NOTE: This command is NOT supported at this time!!!!!!!!!
#
#  CALLING SEQUENCE:
#       call    mag1$logsns
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
mag1$logsns:
        ldconst magd_sft6,r7            # r7 = error code to log
        mov     g0,r3                   # Save g0
        lda     magd_sft,g0             # g0 = Software Fault Log Area
        st      r7,efa_ec(g0)           # Save the Error Code
        st      g5,efa_data(g0)         # Save the IMT
        st      g6,efa_data+4(g0)       # Save the ILMT pointer
        ldq     (g8),r4                 # Load the CDB
        stq     r4,efa_data+8(g0)       # Save the CDB
        ldconst 28,r7                   # Number of bytes saved (ec + data)
        st      r7,mle_len(g0)          # Save the number of bytes to send
        call    M$soft_flt              # Error Trap or Log failure
        mov     r3,g0                   # Restore g0
        ret
#
#******************************************************************************
#
#  NAME: mag1$logsel
#
#  PURPOSE:
#       Processes a Log Select command received from a host.
#
#  DESCRIPTION:
#       NOTE: This command is NOT supported at this time!!!!!!!!!
#
#  CALLING SEQUENCE:
#       call    mag1$logsel
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
mag1$logsel:
        ldconst magd_sft7,r7            # r7 = error code to log
        mov     g0,r3                   # Save g0
        lda     magd_sft,g0             # g0 = Software Fault Log Area
        st      r7,efa_ec(g0)           # Save the Error Code
        st      g5,efa_data(g0)         # Save the IMT
        st      g6,efa_data+4(g0)       # Save the ILMT pointer
        ldq     (g8),r4                 # Load the CDB
        stq     r4,efa_data+8(g0)       # Save the CDB
        ldconst 28,r7                   # Number of bytes saved (ec + data)
        st      r7,mle_len(g0)          # Save the number of bytes to send
        call    M$soft_flt              # Error Trap or Log failure
        mov     r3,g0                   # Restore g0
        ret
#
#******************************************************************************
#
#  NAME: mag1$startstop
#
#  PURPOSE:
#       Processes a Start Stop Unit command received from a host.
#
#  DESCRIPTION:
#       Returns appropriate status and sense data(if appropriate)
#       to indicate to the issuing host the current state of the
#       specified LUN for a start unit.  If the command is a stop unit, then
#       the cache will be flushed before returning status.
#
#  CALLING SEQUENCE:
#       call    mag1$startstop
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
startstop_tbl1:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invf1             # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
startstop_tbl2:
        .byte   dtxfern,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
mag1$startstop:
        ldob    1(g8),r4                # r4 = byte #2 of CDB
        ldconst 0x1e,r6                 # r6 = CDB byte #2 reserved bit mask
        ldob    6(g8),r5                # r5 = byte #7 of CDB
        and     r6,r4,r7                # r7 = reserved bit mask results
        cmpobne 0,r5,.startstop10       # Jif CDB byte #7 non-zero
        cmpobne 0,r7,.startstop10       # Jif reserved bits set in CDB byte #2
        ldos    2(g8),r8                # r8 = bytes #3 and 4 of CDB
        ldconst 0xfe,r6                 # r6 = CDB byte #5 bit mask
        ldob    4(g8),r7                # r7 = byte #5 of CDB
        cmpobne 0,r8,.startstop10       # Jif CDB bytes #3 and 4 are non-zero
        and     r6,r7,r8                # r8 = reserved bit mask results
        cmpobne 0,r8,.startstop10       # Jif reserved bits set in CDB byte #5
        bbc     0,r7,.startstop30       # Jif this is a Stop Unit command
#
# --- Start Unit Command Processing (handle same as Test Unit Ready)
#
        ldq     startstop_tbl2,r4       # Start Unit - load up the operational
        ld      startstop_tbl2+16,r8    #  values in the registers and
        b       mag1$cmdcom             #  finish processing command
#
# --- Error in CDB Processing
#
.startstop10:
        ldq     startstop_tbl1,r4       # load op. values into regs.
        ld      startstop_tbl1+16,r8
        b       mag1$cmdcom             # and finish up routine here and now!
#
# --- Stop Unit Command Processing (handle same as a Sync Cache over the entire
#       drive)
#
.startstop30:
        movl    0,r6                    # r6,r7 = begin at the start of the VD
        ldconst 0,r8                    # r8 = 0 (all of the VDisk)
        ldconst vrsync,r4               # r4 = sync cache VRP function code
        b       .synccache50            # Handle like a Sync Cache command
#
#******************************************************************************
#
#  NAME: mag1$synccache
#
#  PURPOSE:
#       Processes a Sync Cache Command
#
#  DESCRIPTION:
#       This function forwards the request to the cache routines to handle.
#
#  CALLING SEQUENCE:
#       call    mag1$synccache
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
synccache_tbl1:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invf1             # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
        .text
#
mag1$synccache:
        ldob    1(g8),r4                # r4 = byte #2 of CDB
        ldconst 0x1d,r6                 # r6 = CDB byte #2 reserved bit mask
        ldob    6(g8),r5                # r5 = byte #7 of CDB
        and     r6,r4,r7                # r7 = reserved bit mask results
        cmpobne 0,r5,.synccache10       # Jif CDB byte #7 non-zero
        cmpobe  0,r7,.synccache30       # Jif no reserved bits set in CDB
                                        #  byte #2
.synccache10:
        ldq     synccache_tbl1,r4       # load op. values into regs.
        ld      synccache_tbl1+16,r8
        b       mag1$cmdcom             # and finish up routine here and now!

.synccache30:
        ld      6(g8),r8                # isolate transfer length from CDB
        lda     0x00ffff00,r5
        ld      2(g8),r6                # r6 = SDA of host request
        and     r5,r8,r8
        mov     0,r7                    # Zero the upper SDA word
        bswap   r8,r8
        bswap   r6,r6
        shro    8,r8,r8                 # r8 = transfer length from CDB
        ldconst vrsync,r4               # r4 = sync cache VRP function code
#
# NOTE: FALLS THROUGH!
#
# The following code is also called from the Stop Unit Command
#
#  INPUT:
#       r4 = VRP request function code
#       r6,r7 = SDA of host request (little endian format)
#       r8 = transfer length from CDB (little endian format)
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
# NOTE: FALLS THROUGH!
#
.synccache50:
c       g2 = get_vrp();                 # Allocate a VRP
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u get_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
        st      g2,vrvrp(g7)            # save VRP address in pri. ILT
        lda     mag1_MAGcomp,r5         # r5 = ILT completion routine
        ld      ilm_vdmt(g6),g10        # g10 = assoc. VDMT
        stl     r6,vr_vsda(g2)          # save SDA
        st      r5,inl2_cr(g9)          # save my completion handler in pri. ILT
        st      r4,vr_func(g2)          # save request function code, clear
                                        #  strategy and status
        ldos    vdm_vid(g10),r6         # r6 = vid
        ldob    im_pri(g5),r4           # get server priority
#
        st      r8,vr_vlen(g2)          # save # sectors to write in VRP
        stob    r4,vr_strategy(g2)      # save VRP strategy
        stos    r6,vr_vid(g2)           # save vid
        lda     ILTBIAS(g9),g1          # g1 = ILT at MAG interface nest level
        st      g7,inl3_FCAL(g1)        # save FC-AL pointer in ILT
        st      g2,vrvrp(g1)            # save the VRP at this nest level
        mov     inl2_ps_req,r14         # r14 = new task process state code
        lda     task_etbl14,r15         # r15 = task event handler table
        stob    r14,inl2_pstate(g9)     # save new task process state code
        st      r15,inl2_ehand(g9)      # save new task event handler table
#
        movl    0,r4                    # r4-r5 = 0
        st      r4,vr_sglptr(g2)        # clear SGL pointer field in VRP
        stl     r4,vrpsiz+sg_desc0+sg_addr(g2) # Clear SGL address & length
                                        #  fields in VRP
        st      r4,vr_sglsize(g2)       # clear SGL size field
        st      r4,vrpsiz+sg_size(g2)   # clear SGL size field
        st      r4,vrpsiz+sg_scnt(g2)   # clear descriptor count
#
.ifdef TRACES
        call    mag$tr_MAG$submit_vrp   # trace event
.endif # TRACES
        b       MAG$submit_vrp          # send VRP to MAGNITUDE
#
#******************************************************************************
#
#  NAME: mag1$presv_in
#
#  PURPOSE:
#       Handle Persistent Reserve In command
#
#  CALLING SEQUENCE:
#       call    mag1$presv_in
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
presv_in_tbl1:
        .byte   dtxfern,scresc,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length

presv_in_tbl2:
        .byte   dtxferi,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # # SGL descriptors
        .short  0                       # sense length

presv_in_tbl3:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invf1             # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
        .text
#
mag1$presv_in:
        PushRegs(r5)
        mov    g9,g0                    # ILT at nest lvl2
        call   presv_in
        mov    g0,r4                    # return code in r4
        PopRegsVoid(r5)                 # Restore registers

        cmpobe 0,r4,.presvin50          # Jif presv_in succeeded
        cmpobe 1,r4,.presvin10          # Jif "Reservation Conflict"
        cmpobe 3,r4,.presvin20          # Jif "Invalid field in CDB"

.presvin10:
        ldq    presv_in_tbl1,r4         # load error op. values into regs.
        ld     presv_in_tbl1+16,r8      # load sensesize and # of SGL descriptors
        b      .presvin100

.presvin20:
        ldq    presv_in_tbl3,r4         # load error op. values into regs.
        ld     presv_in_tbl3+16,r8      # load sensesize and # of SGL descriptors
        b      .presvin100

.presvin50:
        ldq    presv_in_tbl2,r4         # load op. values into regs.
        ld     presv_in_tbl2+16,r8      # load sensesize and # of SGL descriptors
        ld     ILTBIAS+il_misc(g9),r6   # r6 = SGL pointer

.presvin100:
        b       mag1$cmdcom              # Complete the command
#
#******************************************************************************
#
#  NAME: mag1$presv_out
#
#  PURPOSE:
#       Handle the Persistent Reserve Out command
#
#  CALLING SEQUENCE:
#       call    mag1$presv_out
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
presv_out_tbl10:
        .byte   dtxferc,scnorm,0,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
mag1$presv_out:
        ld      inl2_ilmt(g9),r4
        ld      inl2_ilmt(g7),r5
        ld      5(g8),g0                # Get the parameter list length from CDB
        bswap   g0,g0
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        lda     task_etbl16,r9          # r9 = task event handler table
        st      r9,inl2_ehand(g9)       # save task event handler table in
        ldq     presv_out_tbl10,r4      # load op. values into regs.
        ld      presv_out_tbl10+16,r8   # load sensesize and # of SGL descriptors
        mov     g0,r6                   # r6 = SGL address
        b       mag2$cmdcom             # Go get the parameter data
#
#******************************************************************************
#
#  NAME: mag2$rconflict
#
#  PURPOSE:
#       Return Reservation Conflict status for a command received
#       from an initiator.
#
#  DESCRIPTION:
#       Automatically returns Reservation Conflict status to a CDB
#       received from an initiator.
#
#  CALLING SEQUENCE:
#       call    mag2$rconflict
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
        .data
rconflict_tbl1:
        .byte   dtxfern,scresc,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
        .text
#
mag2$rconflict:
        ldq     rconflict_tbl1,r4       # load op. values into regs.
        ld      rconflict_tbl1+16,r8
        b       mag1$cmdcom             # and return Reservation Conflict
                                        #  status back to initiator
#
#******************************************************************************
#
#  NAME: MAG$submit_vrp
#
#  PURPOSE:
#       To submit the VRP to the next next appropriate layer
#
#  DESCRIPTION:
#       This routine is a funnel point in transferring the ILT/VRP to the
#       next layer (Cache).
#
#  CALLING SEQUENCE:
#       call    MAG$submit_vrp
#
#  INPUT:
#       g1 = ILT
#
#  OUTPUT:
#       none.
#
#  REGS DESTROYED:
#       g1
#
#******************************************************************************
#
MAG$submit_vrp:
#
# --- Set up an in-between ILT Level so Cache can store stuff in it safely
#
        ld      il_misc(g1),r3          # Get the ILT Parms Pointer
        ld      vrvrp(r3),r4            # Get the VRP
        lda     K$comp,r5               # Use the Kernel to complete this layer
        st      r3,il_misc+ILTBIAS(g1)  # Save the ILT Parms Pointer
        st      g1,vr_ilt(r4)           # Save the ILT Pointer in the VRP
        st      r4,vrvrp(g1)            # Save the VRP at this level
        st      r4,vrvrp+ILTBIAS(g1)    # Save the VRP at the next level
        st      r5,il_cr(g1)            # Save the Kernel Completion routine
        lda     ILTBIAS(g1),g1          # Point to the next level
        mov     0,r3                    # Zero out the ILT Fwd thread
        st      r3,il_fthd(g1)          # Close link
#
# --- Determine who to queue the request to, queue the request, and return
#
.if     MAG2MAG
        ldos    vr_func(r4),r6          # r6 = function code
        ldconst vrmagst,r5              # r5 = first VRP function code for DLM functions
        cmpobg  r5,r6,.svrp_100         # Jif not a DLM function code
        ldconst vrmagend,r5             # r5 = last VRP function code for DLM functions
        cmpobg  r6,r5,.svrp_100         # Jif not a DLM function code
        bx      DLM$vque                # Queue the request to DLM
.svrp_100:
.endif  # MAG2MAG
#
        PushRegs(r5)
        mov     g1,g0                   # g0=ILT to que
        call    CA_Que
        PopRegsVoid(r5)
        ret
#
        .data
#******************************************************************************
#
# _____________________ Non-SENSE DATA DEFINITIONS ____________________________
#
#******************************************************************************
#
# ________________________ INQUIRY command data _______________________________
#
#******************************************************************************
#
        .align  4
#
# --- Basic Inquiry command data
#
inquiry1:
        .byte   00              # Peripheral identifier
                                # Bit 7-5 = peripheral qualifier
                                #     4-0 = device type
        .byte   00              # Bit 7 = (RMB) removable medium bit
                                #   6-0 = reserved
inquiry1_ISO:
        .byte   0x02            # Bit 7-6 = ISO version
                                #     5-3 = ECMA version
                                #     2-0 = ANSI version
        .byte   0x72            # Bit 7 = (AERC) asynchronous event reporting
                                #         capability bit
                                #     6 = (TrmTsk) terminate task bit
                                #     5 = (NormACA) normal ACA bit
                                #     4 = (HiSupport) hierarchical LUN bit
                                #   3-0 = response data format
inquiry1_len:
        .byte   inquiry1_size-5 # additional length (n-5)
        .byte   00              # Bit 7 = (SCCS) SCC supported bit
                                #   6-0 = reserved
        .byte   0x00            # Bit 7 = (BQue) basic queuing bit
                                #     6 = (EncServ) enclosure services bit
                                #     5 = (VS) reserved
                                #     4 = (MultiP) multi port bit
                                #     3 = (MChngr) medium changer bit
                                #     2 = (AckReqQ) ACKQ/REQQ bit
                                #     1 = (Addr32) wide SCSI address 32 bit
                                #     0 = (Addr16) wide SCSI address 16 bit
        .byte   0x02            # Bit 7 = (RelAdr) relative addressing bit
                                #     6 = (WBus32) wide SCSI address 32 bit
                                #     5 = (WBus16) wide SCSI address 16 bit
                                #     4 = (Sync) synchronous transfer bit
                                #     3 = (Linked) linked command bit
                                #     2 = (TranDis) transfer disable bit
                                #     1 = (Cmdque) command queuing bit
                                #     0 = (VS) soft reset
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .ascii  "Xiotech "      # Vendor ID
        .ascii  "Virtual-ISE     "  # Product ID
inquiry1_rev:
        .ascii  "1.00"          # Product revision
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .ascii  "XIOtech "      # Vendor ID
        .ascii  "Magnitude 3D    "  # Product ID
inquiry1_rev:
        .ascii  "3.00"          # Product revision
.endif  # MODEL_4700
.endif  # MODEL_7000

inquiry1_sn:
        .space  12,0x20         # drive serial number
                                # in ssssssssvvvv format where:
                                #    s = MAGNITUDE serial # in ASCII
                                #    v = assigned VDisk # in ASCII (blank padded)
inquiry1end:

        .byte   0x00,0x00       # reserved
                                # *****************************
                                # **** version descriptors ****
                                # *****************************
        .byte   0x00,0x20       # SAM (no version claim)
        .byte   0x0d,0x9c       # FC-PH-3 ANSI X3.303-1998
        .byte   0x09,0x01       # FCP-2 T10/1444 revision 4
        .byte   0x02,0x60       # SPC-2 (no version claim)
        .byte   0x01,0x80       # SBC (no version claimed)
        .byte   0x09,0x60       # iSCSI (no version claimed)
        .byte   0x00,0x00
        .byte   0x00,0x00
inquiry1WHQLend:

#
        .set    inquiry1_size,inquiry1end-inquiry1 # size of old inquiry1 data
        .set    inquiry1_WHQL_size,inquiry1WHQLend-inquiry1 # size of WHQL inquiry1 data
#
# --- INQUIRY Vital Product Data Pages
#
# --- Supported vital product data page 0
#
inqpg_00:
        .byte   00              # Bit 7-5 = peripheral qualifier
                                #     4-0 = peripheral device type code
        .byte   00              # page code
        .byte   00              # reserved
        .byte   inqpg_00_size-4 # page length (remaining byte count)
        .byte   00              # page code 00 supported
        .byte   0x80            # page code 0x80 supported
        .byte   0x83            # page code 0x83 supported
inqpg_00_end:
        .set    inqpg_00_size,inqpg_00_end-inqpg_00 # size of inquiry page 00
                                # data
#
        .align  4
#
# --- Unit serial number page 0x80
#
inqpg_80:
        .byte   00              # Bit 7-5 = peripheral qualifier
                                #     4-0 = peripheral device type code
        .byte   0x80            # page code
        .byte   00              # reserved
        .byte   inqpg_80_size-4 # page length (remaining byte count)
inqpg_80_sn:
        .space  12,0x20         # unit serial number in ASCII
inqpg_80_end:
        .set    inqpg_80_size,inqpg_80_end-inqpg_80 # size of inquiry page 80
                                # data
#
# --- Device identification page 0x83
#
inqpg_83:
        .byte   00              # Bit 7-5 = peripheral qualifier
                                #     4-0 = peripheral device type code
        .byte   0x83            # page code
        .byte   00              # reserved
inqpg_83_len:
        .byte   inqpg_83_size-4 # page length (remaining byte count)
#
# --- Logical device identifier descriptor
#
        .byte   01              # Bit 7-4 = reserved
                                #     3-0 = code set
                                #           0 = reserved
                                #           1 = the identifier field shall
                                #               contain binary values.
                                #           2 = the identifier field shall
                                #               contain ASCII chars.
                                #         3-f = reserved
        .byte   01              # Bit 7-6 = reserved
                                #     5-4 = association
                                #           0 = identifier assoc. with device
                                #           1 = identifier assoc. with path
                                #         2-3 = reserved
                                #     3-0 = identifier type
                                #           0 = no assignment authority
                                #           1 = first 8 bytes are vendor ID
                                #           2 = IEEE extended unique identifier
                                #           3 = FC-PH Name_Identifier
                                #         4-f = reserved
        .byte   00              # reserved
        .byte   inqpg_83a_size  # identifier length (remaining in ID field)

inqpg_83a:
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .ascii  "Xiotech "      # Vendor ID
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .ascii  "XIOtech "      # Vendor ID
.endif  # MODEL_4700
.endif  # MODEL_7000

inqpg_83_sn1:
        .word   0               # MAGNITUDE system serial number
inqpg_83_vd:
        .byte   0               # Upper and lower byte of VDisk Number
        .byte   0
inqpg_83b:
# For WHQL tests we need to send type-2 descriptor(EUI-64) value.
# Type-2 descriptor values that we sent are in the following format
# < 24bit Company ID> <40 bit Vendor Unique ID> (IEEE reqmt)
# <24bit Company ID> <First 3 bytes of Serial number + Upper and lower bytes of VDisk Number)
# CCCCCC SSSSSS VVVV         C= Company Identifier
#                            S = Serial Number
#                            V = Vdisk number
# Exp: 00D0B2 27B2 01 00
#

inqpg_83_end:
        .byte   0x01            # The identifier field Shall contain binary values
        .byte   0x02            # type 2 descriptor EUI-64
        .byte   0x00            # Reserved filed
        .byte   0x08            # Descriptor List length
inqpg83_compid:
        .byte   0x00,0xD0,0xB2  # Xiotech Company ID
inqpg83_3bytesn:
        .byte   0               # First 3 bytes of Serial Number
        .byte   0
        .byte   0
inqpg_83_vd2:
        .byte   0               # Upper and lower byte of VDisk Number
        .byte   0
inquiryvpd83_WHQLend:

        .set    inqpg_83_size,inqpg_83_end-inqpg_83             # size of inquiry page 83
        .set    inqpg_83_WHQLsize,inquiryvpd83_WHQLend-inqpg_83 # size of inquiry page 83(WHQL flag enabled)
        .set    inqpg_83a_size,inqpg_83b-inqpg_83a              # size of first identifier field data
#
#******************************************************************************
#
# ______________________ MODE SENSE command data ______________________________
#
#******************************************************************************
#
# ______________________ Default MODE SENSE data ______________________________
#
#******************************************************************************
#
# For C conversion, put set statements into structures
# then msalloc_sz doesn't have to be global
#
        .align  4
#
# --- 6 byte Mode Parameter Header + Block Descriptor
#
modesns1:
        .byte   ms_sz-1         # size of mode sense data-1
        .byte   0               # medium type
        .byte   0               # Flag bits
                                # Bit 7 = (WP) write protect bit
                                #   6-5 = reserved
                                #     4 = (DPOFUA) DPO & FUA flag support bit
                                #   3-0 = reserved
        .byte   8               # block descriptor length
#
modesns_1_cap:
        .byte   0               # density code
        .byte   0               # MSB number of blocks
        .byte   0               # number of blocks
        .byte   0               # LSB number of blocks
        .byte   0               # reserved
        .byte   0               # MSB block length
        .byte   2               # block length
        .byte   0               # LSB block length
modesns1end:
#
        .set    ms1_sz,modesns1end-modesns1 # size of modesns1 data
        .set    modesns1_cap,modesns_1_cap-modesns1 # location in MODE SENSE
#
#
# --- Mode Page 1 (Error Recovery page)
#
ms_pg1:
        .byte   0x01            # page code
        .byte   0x0a            # page length
        .byte   0xc0            # Flag bits
                                # Bit 7 = (AWRE) automatic write reallocation
                                #         enabled bit
                                #     6 = (ARRE) automatic read reallocation
                                #         enabled bit
                                #     5 = (TB) transfer block bit
                                #     4 = (RC) read continuous bit
                                #     3 = (EER) enable early recovery bit
                                #     2 = (PER) post error bit
                                #     1 = (DTE) disable transfer on error bit
                                #     0 = (DCR) disable correction bit
        .byte   0x10            # read retry count
        .byte   0x00            # correction span
        .byte   0x00            # head offset count
        .byte   0x00            # data strobe offset count
        .byte   0x00            # reserved
        .byte   0x10            # write retry count
        .byte   0x00            # reserved
        .byte   0xff,0xff       # recovery time limit (in millisec.)
ms_pg1end:
#
        .set    mspg1_sz,ms_pg1end-ms_pg1 # size of page 1 MODE SENSE data
        .set    mspg1_off,ms_pg1-modesns1 # offset into MODE SENSE data where
                                          #  Mode Page 1 begins
#
#
# --- Mode Page 2 (Disconnect/reconnect page (for WHQL compliance))
#     *** Why Microsoft(r) cares about this page is anyone guess  ***
#     *** The table is just fill with values that make some sense ***
#     *** They are copied from a Seagate(r) FC drive manual       ***
#
ms_pg2:
        .byte   0x02            # page code
        .byte   0x0e            # page length
        .byte   0x80,0x80
        .byte   0x00,0x0a
        .byte   0x00,0x00
        .byte   0x00,0x00
        .byte   0x00,0x00
        .byte   0x00,0x00
        .byte   0x00,0x00
ms_pg2end:
#
        .set    mspg2_sz,ms_pg2end-ms_pg2 # size of page 2 MODE SENSE data
        .set    mspg2_off,ms_pg2-modesns1 # offset into MODE SENSE data where
                                          #  Mode Page 2 begins
#
#
# --- Mode Page 3 (Format Parameters page)
#
ms_pg3:
        .byte   0x03            # page code
        .byte   0x16            # page length
        .byte   0x00,0x60       # tracks per zone (MSB)
        .byte   0x00,0x00       # alternate sectors per zone
        .byte   0x00,0x00       # alternate tracks per zone
        .byte   0x00,0x00       # alternate tracks per volume
        .byte   0x00,0xc8       # sectors per track
        .byte   0x02,0x00       # data bytes per physical sector
        .byte   0x00,0x01       # interleave
        .byte   0x00,0x1e       # track skew factor
        .byte   0x00,0x3c       # cylinder skew factor
        .byte   0x40            # Flag bits
                                # Bit 7 = SSEC (soft sectoring)
                                #     6 = HSEC (hard sectoring)
                                #     5 = RMB (removable flag)
                                #     4 = SURF (surface bit)
                                #     3 = reserved
                                #     2 = reserved
                                #     1 = reserved
                                #     0 = reserved
        .byte   0x00,0x00,0x00  # reserved
ms_pg3end:
#
        .set    mspg3_sz,ms_pg3end-ms_pg3 # size of page 3 MODE SENSE data
        .set    mspg3_off,ms_pg3-modesns1 # offset into MODE SENSE data where
                                          #  Mode Page 3 begins
#
#
# --- Mode Page 4 (Rigid Disk Drive Geometry Parameters page)
#
ms_pg4:
        .byte   0x04            # page code
        .byte   0x16            # page length
ms_pg4_cylnum:
        .byte   0x00            # number of cylinders (MSB)
        .byte   0x00            # number of cylinders
        .byte   0x00            # number of cylinders (LSB)
        .byte   0x14            # number of heads
        .byte   0x00,0x00,0x00  # starting cylinder - write precomp
        .byte   0x00,0x00,0x00  # starting cylinder - reduced write current
        .byte   0x00,0x00       # drive step rate
        .byte   0x00,0x00,0x00  # landing zone cylinder
        .byte   0x00            # Flags
                                # Bit 7-2 = reserved
                                #     1-0 = RPL (rotational position locking)
        .byte   0x00            # rotational offset
        .byte   0x00            # reserved
        .byte   0x27,0x10       # medium rotation rate
        .byte   0x00,0x00       # reserved
ms_pg4end:
#
        .set    mspg4_cylnum,ms_pg4_cylnum-modesns1 # location in MODE SENSE
                                #  data where number of cylinders field resides
        .set    mspg4_sz,ms_pg4end-ms_pg4 # size of page 4 MODE SENSE data
        .set    mspg4_off,ms_pg4-modesns1 # offset into MODE SENSE data where
                                          #  Mode Page 4 begins
#
#
# --- Mode Page 8 (Caching page)
#
ms_pg8:
        .byte   0x08            # page code
        .byte   0x12            # page length
        .byte   0x00            # Flag bits
                                # Bit 7 = (IC) initiator control enable bit
                                #     6 = (ABPF) abort pre-fetch bit
                                #     5 = (CAP) caching analysis permitted bit
                                #     4 = (DISC) discontinuity bit
                                #     3 = (SIZE) size enabled bit
                                #     2 = (WCE) write cache enabled bit
                                #     1 = (MF) multiplication factor bit
                                #     0 = (RCD) read cache disabled bit
        .byte   0x00            # Bit 7-4=demand read retention priority
                                #     3-0=write retention priority
        .byte   0x00,0x00       # disable pre-fetch transfer length
        .byte   0x00,0x00       # minimum pre-fetch
        .byte   0x00,0x00       # maximum pre-fetch
        .byte   0x00,0x00       # maximum pre-fetch ceiling
        .byte   0x20            # Flag bits
                                # Bit 7 = (FSW) force sequential write bit
                                #     6 = (LBCSS) logical block cache segment
                                #         size bit
                                #     5 = (DRA) disable read-ahead bit
                                #     4 = (VS) vendor-specific bit
                                #     3 = (VS) vendor-specific bit
                                #   2-0 = reserved
        .byte   0x00            # number of cache segments
        .byte   0x00,0x00       # cache segment size
        .byte   0x00            # reserved
        .byte   0x00,0x00,0x00  # non-cache segment size
ms_pg8end:
#
        .set    mspg8_sz,ms_pg8end-ms_pg8 # size of page 8 MODE SENSE data
        .set    mspg8_off,ms_pg8-modesns1 # offset into MODE SENSE data where
                                          #  Mode Page 8 begins
#
#
# --- Mode Page A (Control Mode page)
#
ms_pga:
        .byte   0x0a            # page code
        .byte   0x0a            # page length
        .byte   0x22            # Flag bits
                                # Bit 7-5 = (TST) task set type field
                                #           000b=task set per logical unit
                                #                for all initiators
                                #           001b=task set per initiator per
                                #                logical unit
                                #           010b-111b=reserved
                                #     4-2 = reserved
                                #       1 = (GLTSD) global logging target save
                                #           disable bit
                                #       0 = (RLEC) report log exception
                                #           condition bit
ms_pga_QErr:
        .byte   0x00            # Flag bits
                                # Bit 7-4 = Queue algorithm modifier
                                #           0h=restricted ordering
                                #           1h=unrestricted reordering allowed
                                #           2h-7h=reserved
                                #           8h-fh=vendor-specific
                                #       3 = reserved
                                #     2-1 = (QErr) queue error management
                                #            field
                                #           00b=blocked tasks in the task set
                                #               shall resume after an ACA or
                                #               CA condition is cleared.
                                #           01b=all blocked tasks in the task
                                #               set shall be aborted for all
                                #               initiators.
                                #           10b=reserved
                                #           11b=blocked tasks in the task set
                                #               belonging to the initiator
                                #               to which a CHECK CONDITION
                                #               status is sent shall be
                                #               aborted when the status is
                                #               sent.
                                #       0 = (DQue) disable queuing bit
        .byte   0x00            # Flag bits
                                # Bit 7 = reserved
                                #     6 = (RAC) report a check bit
                                #   5-4 = reserved
                                #     3 = (SWP) software write protect bit
                                #     2 = (RAERP) ready AER permission bit
                                #     1 = (UAAERP) unit attention AER
                                #         permission bit
                                #     0 = (EAERP) error AER permission bit
        .byte   0x00            # reserved
        .byte   0x00,0x00       # ready AER holdoff period (in millisecs.)
        .byte   0x00,0x00       # busy timeout period (in 100 millisecs.)
        .byte   0x00            # reserved
        .byte   0x00            # reserved
ms_pgaend:
#
        .set    mspga_QErr,ms_pga_QErr-modesns1 # location in MODE SENSE data
                                #  where QErr field resides
        .set    mspga_sz,ms_pgaend-ms_pga # size of page A MODE SENSE data
        .set    mspga_off,ms_pga-modesns1 # offset into MODE SENSE data where
                                          #  Mode Page a begins
#
# --- Mode Page 0x18 (Fibre Channel Logical Unit Control page)
#
ms_pg18:
        .byte   0x18            # page code
        .byte   0x06            # page length
        .byte   0x00            # reserved
        .byte   0x00            # Bit 7-1 = reserved
                                #       0 = enable precise delivery checking
                                #           (EPDC)
        .byte   0x00            # reserved
        .byte   0x00            # reserved
        .byte   0x00            # reserved
        .byte   0x00            # reserved
ms_pg18end:
#
        .set    mspg18_sz,ms_pg18end-ms_pg18 # size of page 18 MODE SENSE data
        .set    mspg18_off,ms_pg18-modesns1  # offset into MODE SENSE data where
                                             # Mode Page 18 begins
#
# --- Mode Page 0x19 (Fibre Channel Port Control page)
#
ms_pg19:
        .byte   0x19            # page code
        .byte   0x06            # page length
        .byte   0x00            # reserved
        .byte   0x00            # Bit 7 = reserved
                                #     6 = prevent loop port bypass (PLPB)
                                #     5 = disable discovery (DDIS)
                                #     4 = disable loop master (DLM)
                                #     3 = disable soft address (DSA)
                                #     2 = allow login without loop
                                #         initialization (ALWI)
                                #     1 = disable target initiated port enable
                                #         (DTIPE)
                                #     0 = disable target originated loop
                                #         initialization (DTOLI)
        .byte   0x00            # reserved
        .byte   0x00            # reserved
        .byte   0x00            # reserved
        .byte   0x00            # reserved
ms_pg19end:
#
        .set    mspg19_sz,ms_pg19end-ms_pg19 # size of page 19 MODE SENSE data
        .set    mspg19_off,ms_pg19-modesns1  # offset into MODE SENSE data where
                                             # Mode Page 19 begins
#
# --- Mode Page 0x1C (Informational exceptions control page)
#
ms_pg1C:
        .byte   0x1C            # page code
        .byte   0x0A            # page length
        .byte   0x08            # Bit 7 = Perf
                                #     6 = Reserved
                                #     5 = Reserved
                                #     4 = Reserved
                                #     3 = DExcpt
                                #     2 = Test
                                #     1 = Reserved
                                #     0 = LogErr
        .byte   0x00            # MRIE
        .word   0x00            # Interval timer
        .word   0x00            # Report count
ms_pg1Cend:
#
        .set    mspg1C_sz,ms_pg1Cend-ms_pg1C # size of page 1C MODE SENSE data
        .set    mspg1C_off,ms_pg1C-modesns1  # offset into MODE SENSE data where
                                             # Mode Page 1C begins
#
#
# --- Mode Page 0 (Unit Attention page)
#
ms_pg0:
        .byte   0x00            # page code
        .byte   0x02            # page length
        .byte   0x00            # Flag bits
                                # Bit 7 = reserved
                                #     6 = (SSM) synchronous select mode bit
                                #     5 = (IL) INQUIRY length bit
                                #     4 = (UnitAttn) Unit Attention bit
                                #     3 = reserved
                                #     2 = (Rnd) round bit
                                #     1 = (Strict) Strict bit
                                #     0 = (S2) SCSI-2 bit
        .byte   0x00            # reserved
ms_pg0end:
#
        .set    mspg0_sz,ms_pg0end-ms_pg0 # size of page 0 MODE SENSE data
        .set    mspg0_off,ms_pg0-modesns1 # offset into MODE SENSE data where
                                          #  Mode Page 0 begins
#
        .set    ms_sz,ms_pg0end-modesns1  # size for all MODE SENSE data
        .set    msalloc_sz,(ms_sz+7)&0xf8 # MODE SENSE data allocation size
MSAlloc_SZ: .word msalloc_sz    # NEED THIS value IN "c"
        .set    mscopy_sz,msalloc_sz/4  # MODE SENSE data copy size (words)
        .set    ms10_sz,16              # size of mode sense 10 header data
#
#******************************************************************************
#
# ____________________ Changeable MODE SENSE data _____________________________
#
#******************************************************************************
#
#
# --- 6 byte Mode Parameter Header + Block Descriptor
#
chgmodesns1:
        .byte   ms_sz-1         # size of mode sense data-1
        .byte   0               # medium type
        .byte   0x00            # Flag bits
                                # Bit 7 = (WP) write protect bit
                                #   6-5 = reserved
                                #     4 = (DPOFUA) DPO & FUA flag support bit
                                #   3-0 = reserved
        .byte   8               # block descriptor length
        .byte   0               # density code
        .byte   0               # MSB number of blocks
        .byte   0               # number of blocks
        .byte   0               # LSB number of blocks
        .byte   0               # reserved
        .byte   0               # MSB block length
        .byte   0x00            # block length
        .byte   0               # LSB block length
#
# --- Mode Page 1 (Error Recovery page)
#
mschg_pg1:
        .byte   0x01            # page code
        .byte   0x0a            # page length
        .byte   0xc0            # Flag bits
                                # Bit 7 = (AWRE) automatic write reallocation
                                #         enabled bit
                                #     6 = (ARRE) automatic read reallocation
                                #         enabled bit
                                #     5 = (TB) transfer block bit
                                #     4 = (RC) read continuous bit
                                #     3 = (EER) enable early recovery bit
                                #     2 = (PER) post error bit
                                #     1 = (DTE) disable transfer on error bit
                                #     0 = (DCR) disable correction bit
        .byte   0xff            # read retry count
        .byte   0x00            # correction span
        .byte   0x00            # head offset count
        .byte   0x00            # data strobe offset count
        .byte   0x00            # reserved
        .byte   0xff            # write retry count
        .byte   0x00            # reserved
        .byte   0xff,0xff       # recovery time limit (in millisec.)

#
#
# --- Mode Page 2 (Disconnect/reconnect page (for WHQL compliance))
#     *** Why Microsoft (r) cares about this page is anyone guess ***
#     *** No area of this page are modifiable                     ***
#
mschg_pg2:
        .byte   0x02            # page code
        .byte   0x0e            # page length
        .byte   0x00,0x00
        .byte   0x00,0x00
        .byte   0x00,0x00
        .byte   0x00,0x00
        .byte   0x00,0x00
        .byte   0x00,0x00
        .byte   0x00,0x00

#
# --- Mode Page 3 (Format Parameters page)
#
mschg_pg3:
        .byte   0x03            # page code
        .byte   0x16            # page length
        .byte   0x00,0x00       # tracks per zone (MSB)
        .byte   0x00,0x00       # alternate sectors per zone
        .byte   0x00,0x00       # alternate tracks per zone
        .byte   0x00,0x00       # alternate tracks per volume
        .byte   0x00,0x00       # sectors per track
        .byte   0x00,0x00       # data bytes per physical sector
        .byte   0x00,0x00       # interleave
        .byte   0x00,0x00       # track skew factor
        .byte   0x00,0x00       # cylinder skew factor
        .byte   0x00            # Flag bits
                                # Bit 7 = SSEC (soft sectoring)
                                #     6 = HSEC (hard sectoring)
                                #     5 = RMB (removable flag)
                                #     4 = SURF (surface bit)
                                #     3 = reserved
                                #     2 = reserved
                                #     1 = reserved
                                #     0 = reserved
        .byte   0x00,0x00,0x00  # reserved
#
# --- Mode Page 4 (Rigid Disk Drive Geometry Parameters page)
#
mschg_pg4:
        .byte   0x04            # page code
        .byte   0x16            # page length
        .byte   0x00            # number of cylinders (MSB)
        .byte   0x00            # number of cylinders
        .byte   0x00            # number of cylinders (LSB)
        .byte   0x00            # number of heads
        .byte   0x00,0x00,0x00  # starting cylinder - write precomp
        .byte   0x00,0x00,0x00  # starting cylinder - reduced write current
        .byte   0x00,0x00       # drive step rate
        .byte   0x00,0x00,0x00  # landing zone cylinder
        .byte   0x00            # Flags
                                # Bit 7-2 = reserved
                                #     1-0 = RPL (rotational position locking)
        .byte   0x00            # rotational offset
        .byte   0x00            # reserved
        .byte   0x00,0x00       # medium rotation rate
        .byte   0x00,0x00       # reserved
#
# --- Mode Page 8 (Caching page)
#
mschg_pg8:
        .byte   0x08            # page code
        .byte   0x12            # page length
        .byte   0x00            # Flag bits
                                # Bit 7 = (IC) initiator control enable bit
                                #     6 = (ABPF) abort pre-fetch bit
                                #     5 = (CAP) caching analysis permitted bit
                                #     4 = (DISC) discontinuity bit
                                #     3 = (SIZE) size enabled bit
                                #     2 = (WCE) write cache enabled bit
                                #     1 = (MF) multiplication factor bit
                                #     0 = (RCD) read cache disabled bit
        .byte   0x00            # Bit 7-4=demand read retention priority
                                #     3-0=write retention priority
        .byte   0x00,0x00       # disable pre-fetch transfer length
        .byte   0x00,0x00       # minimum pre-fetch
        .byte   0x00,0x00       # maximum pre-fetch
        .byte   0x00,0x00       # maximum pre-fetch ceiling
        .byte   0x20            # Flag bits
                                # Bit 7 = (FSW) force sequential write bit
                                #     6 = (LBCSS) logical block cache segment
                                #         size bit
                                #     5 = (DRA) disable read-ahead bit
                                #     4 = (VS) vendor-specific bit
                                #     3 = (VS) vendor-specific bit
                                #   2-0 = reserved
        .byte   0x00            # number of cache segments
        .byte   0x00,0x00       # cache segment size
        .byte   0x00            # reserved
        .byte   0x00,0x00,0x00  # non-cache segment size
#
# --- Mode Page A (Control Mode page)
#
mschg_pga:
        .byte   0x0a            # page code
        .byte   0x0a            # page length
        .byte   0x00            # Flag bits
                                # Bit 7-5 = (TST) task set type field
                                #           000b=task set per logical unit
                                #                for all initiators
                                #           001b=task set per initiator per
                                #                logical unit
                                #           010b-111b=reserved
                                #     4-2 = reserved
                                #       1 = (GLTSD) global logging target save
                                #           disable bit
                                #       0 = (RLEC) report log exception
                                #           condition bit
        .byte   0x06            # Flag bits
                                # Bit 7-4 = Queue algorithm modifier
                                #           0h=restricted ordering
                                #           1h=unrestricted reordering allowed
                                #           2h-7h=reserved
                                #           8h-fh=vendor-specific
                                #       3 = reserved
                                #     2-1 = (QErr) queue error management
                                #            field
                                #           00b=blocked tasks in the task set
                                #               shall resume after an ACA or
                                #               CA condition is cleared.
                                #           01b=all blocked tasks in the task
                                #               set shall be aborted for all
                                #               initiators.
                                #           10b=reserved
                                #           11b=blocked tasks in the task set
                                #               belonging to the initiator
                                #               to which a CHECK CONDITION
                                #               status is sent shall be
                                #               aborted when the status is
                                #               sent.
                                #       0 = (DQue) disable queuing bit
        .byte   0x00            # Flag bits
                                # Bit 7 = reserved
                                #     6 = (RAC) report a check bit
                                #   5-4 = reserved
                                #     3 = (SWP) software write protect bit
                                #     2 = (RAERP) ready AER permission bit
                                #     1 = (UAAERP) unit attention AER
                                #         permission bit
                                #     0 = (EAERP) error AER permission bit
        .byte   0x00            # reserved
        .byte   0x00,0x00       # ready AER holdoff period (in millisecs.)
        .byte   0x00,0x00       # busy timeout period (in 100 millisecs.)
        .byte   0x00            # reserved
        .byte   0x00            # reserved
#
# --- Mode Page 0x18 (Fibre Channel Logical Unit Control page)
#
mschg_pg18:
        .byte   0x18            # page code
        .byte   0x06            # page length
        .byte   0x00            # reserved
        .byte   0x00            # Bit 7-1 = reserved
                                #       0 = enable precise delivery checking
                                #           (EPDC)
        .byte   0x00            # reserved
        .byte   0x00            # reserved
        .byte   0x00            # reserved
        .byte   0x00            # reserved
#
# --- Mode Page 0x19 (Fibre Channel Port Control page)
#
mschg_pg19:
        .byte   0x19            # page code
        .byte   0x06            # page length
        .byte   0x00            # reserved
        .byte   0x7f            # Bit 7 = reserved
                                #     6 = prevent loop port bypass (PLPB)
                                #     5 = disable discovery (DDIS)
                                #     4 = disable loop master (DLM)
                                #     3 = disable soft address (DSA)
                                #     2 = allow login without loop
                                #         initialization (ALWI)
                                #     1 = disable target initiated port enable
                                #         (DTIPE)
                                #     0 = disable target originated loop
                                #         initialization (DTOLI)
        .byte   0x00            # reserved
        .byte   0x00            # reserved
        .byte   0x00            # reserved
        .byte   0x00            # reserved
#
# --- Mode Page 0x1C (Informational exceptions control page)
#
mschg_pg1C:
        .byte   0x1C            # page code
        .byte   0x0A            # page length
        .byte   0x80            # Bit 7 = Perf
                                #     6 = Reserved
                                #     5 = Reserved
                                #     4 = Reserved
                                #     3 = DExcpt
                                #     2 = Test
                                #     1 = Reserved
                                #     0 = LogErr
        .byte   0x00            # MRIE
        .word   0x00            # Interval timer
        .word   0x00            # Report count
#
#
# --- Mode Page 0 (Unit Attention page)
#
mschg_pg0:
        .byte   0x00            # page code
        .byte   0x02            # page length
        .byte   0x00            # Flag bits
                                # Bit 7 = reserved
                                #     6 = (SSM) synchronous select mode bit
                                #     5 = (IL) INQUIRY length bit
                                #     4 = (UnitAttn) Unit Attention bit
                                #     3 = reserved
                                #     2 = (Rnd) round bit
                                #     1 = (Strict) Strict bit
                                #     0 = (S2) SCSI-2 bit
        .byte   0x00            # reserved
#
#
# --- 10 byte Mode Parameter Header + Block Descriptor
#
chgmodesns10:
        .byte   0               # Mode data length (MSB)
        .byte   ms_sz-1         # Mode data length (LSB)
        .byte   0               # medium type
        .byte   0               # Flag bits
                                # Bit 7 = (WP) write protect bit
                                #   6-5 = reserved
                                #     4 = (DPOFUA) DPO & FUA flag support bit
                                #   3-0 = reserved
        .byte   0               # Reserved
        .byte   0               # Reserved
        .byte   0               # Block Descriptor Length (MSB)
        .byte   8               # Block Descriptor Length (LSB)
#
        .byte   0               # density code
        .byte   0               # MSB number of blocks
        .byte   0               # number of blocks
        .byte   0               # LSB number of blocks
        .byte   0               # reserved
        .byte   0               # MSB block length
        .byte   0               # block length
        .byte   0               # LSB block length
#
#******************************************************************************
#
# ______________________ SENSE DATA DEFINITIONS _______________________________
#
#******************************************************************************
#
#
# --- Undefined command sense data
#
sense_undef:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x05                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x20                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0xc0,0x00,0x00          # sense key specific
#
# --- Undefined LUN sense data
#
        START_SH_DATA_SECTION
MAGD$sense_nolun:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x05                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x25                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- LUN not ready - still initializing sense data
#
MAGD$sense_uninit:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x02                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x04                    # ASC
        .byte   0x01                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- No sense sense data
#
sense_nosense:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x00                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x00                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
        END_SH_DATA_SECTION
#
# --- Overlapped commands attempted sense data
#
sense_overlap:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x0b                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x4e                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Invalid field in CDB sense data format #1
#
sense_invf1:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x05                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x24                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Invalid field in parameter list sense data format #1
#
sense_invfpl1:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x05                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x26                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Invalid Release of Persistent Reservation sense data format
#
sense_inv_release:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x05                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x26                    # ASC
        .byte   0x04                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Insufficient Registration Resources sense data format
#
sense_insuff_reg_res:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x05                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x55                    # ASC
        .byte   0x04                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Parameter list length sense data format
#
sense_inv_list_len:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x05                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x1A                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Invalid message error sense data format #1
#
sense_invm1:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x05                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x49                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Saving parameters not supported sense data format
#
sense_nosaveparm:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x05                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x39                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Logical Unit Communications Failure sense data format
#
sense_commerr:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x0b                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x08                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Power-on reset occurred sense data format
#
        START_SH_DATA_SECTION
sense_poweron:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x06                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x29                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- SCSI bus reset received sense data format
#
sense_scsi_reset:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x06                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x29                    # ASC
        .byte   0x02                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Bus device reset function occurred sense data format
#
sense_bus_reset:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x06                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x29                    # ASC
        .byte   0x03                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Commands cleared by another initiator sense data format
#
sense_cmds_clr:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x0b                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x2f                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
        END_SH_DATA_SECTION
#
# --- Write error sense data format #1 (Write error-Auto reallocation failed)
#
sense_werr1:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x03                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x0c                    # ASC
        .byte   0x02                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Read error sense data format #1 (Unrecovered READ error)
#
sense_rerr1:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x03                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x11                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Error sense data format #1 (I/O process terminated)
#
sense_err1:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x0b                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x00                    # ASC
        .byte   0x06                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Error sense data format #2 (Data path failure)
#
sense_err2:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x0b                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x41                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Error sense data format #3 (Timeout on logical unit)
#
sense_err3:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x0b                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x3e                    # ASC
        .byte   0x02                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Error sense data format #4 (Internal target failure)
#
sense_err4:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x0b                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x44                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Error sense data format #5 (Logical unit not ready-manual intervention req.)
#
sense_err5:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x0b                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x04                    # ASC
        .byte   0x03                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Error sense data format #6 (Logical unit not supported)
#
sense_err6:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x03                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x25                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Error sense data format #7 (Logical block address out of range)
#
sense_err7:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x05                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x21                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Error sense data format #8 (Illegal function)
#
sense_err8:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x05                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x22                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Error sense data format #9 (Invalid element address)
#
sense_err9:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x05                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x21                    # ASC
        .byte   0x01                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Error sense data format #10 (Miscompare during VERIFY operation)
#
sense_err10:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x0e                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x1d                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- MODE parameters changed format
#
        START_SH_DATA_SECTION
sense_MPchgd:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x06                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x2a                    # ASC
        .byte   0x01                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- LUN Inventory changed
#
sense_replunsdata_chgd:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x06                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x3f                    # ASC
        .byte   0x0e                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- LUN size changed
#
sense_lunsize_chgd:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x06                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x3f                    # ASC
        .byte   0x0a                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Reservation Preempted
#
sense_resv_preempted:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x06                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x2A                    # ASC
        .byte   0x03                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Registrations Preempted
#
sense_reg_preempted:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x06                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x2A                    # ASC
        .byte   0x05                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- Reservation Released
#
sense_resv_released:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x06                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x2A                    # ASC
        .byte   0x04                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
        END_SH_DATA_SECTION
#
# --- FORMAT command failed format
#
sense_FUfail:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x03                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x31                    # ASC
        .byte   0x01                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
# --- EXTENDED COPY command failed format
#
sense_XCfail:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x0a                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x3e                    # ASC
        .byte   0x01                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific

sense_rdefect:
        .byte   0x70                    # response code
        .byte   0x00                    # segment number
        .byte   0x00                    # sense key + flags
        .byte   0x00,0x00,0x00,0x00     # information
        .byte   17-7                    # additional sense length
        .byte   0x00,0x00,0x00,0x00     # command-specific information
        .byte   0x1c                    # ASC
        .byte   0x00                    # ASCQ
        .byte   0x00                    # FRU
        .byte   0x00,0x00,0x00          # sense key specific
#
#******************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
