# $Id: lad_cm.as 159862 2012-09-19 20:49:57Z marshall_midden $
#**********************************************************************
#
#  NAME: lad_cm.as
#
#  PURPOSE:
#
#       To provide support for the Secondary Copy Manager logic which
#       supports MAGNITUDE-to-MAGNITUDE functions and services.
#
#  FUNCTIONS:
#
#       CM$init        - Copy Manager initialization
#       CM$que         - Queue Secondary Copy Requests
#       CM$rc_que      - Queue Read VRP Completions
#       CM$wc_que      - Queue Write VRP Completions
#
#       This module employs 3 process:
#
#       cm$exec        - SCR process
#       cm_rd_comp     - Read VRP completion
#       cm_wr_comp     - Write VRP completion
#
#  Copyright (c) 1999-2008 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- assembly options ------------------------------------------------
#
        .set    DEBUG,TRUE
        .data
#
# --- Copy Manager resident modules
#
        .globl  CM$init                 # Copy Manager initialization
        .globl  CM$que                  # Copy Manager queue routine
        .globl  CM$dealloc_RM           # deallocate main region map and
                                        #  segment maps
        .globl  CM$dealloc_transRM      # deallocate transfer ownership
                                        #  region map and segment maps
        .globl  CM_dealloc_transRM
        .globl  cm$Validate_Swap        # validate copy swap operation


        .globl  CM_Log_Completion       # Copy manager Log message routine

        .align  4                       # align just in case
#
# --- local usage data definitions ------------------------------------
#
        .globl  cur_ap_data
# --- SCR high priority queue definitions -------------------------------------- *****
#
        .set    CMEXECPRI,160
cm_exec_qu_high:
        .word   0                       # queue head pointer        <w>
        .word   0                       # queue tail pointer        <w>
        .word   0                       # queue count               <w>
        .word   0                       # associated pcb            <w>
#
# --- SCR normal priority queue definitions -------------------------------------- *****
#
        .globl  cm_exec_qu_norm
cm_exec_qu_norm:
        .word   0                       # queue head pointer        <w>
        .word   0                       # queue tail pointer        <w>
        .word   0                       # queue count               <w>
        .word   0                       # associated pcb            <w>
#
# --- SCR low priority queue definitions -------------------------------------- *****
#
cm_exec_qu_low:
        .word   0                       # queue head pointer        <w>
        .word   0                       # queue tail pointer        <w>
        .word   0                       # queue count               <w>
        .word   0                       # associated pcb            <w>
#
# --- Read VRP completion queue definitions -------------------------------------- *****
#
        .set    CMRCPRI,170
cm_rc_qu:
        .word   0                       # queue head pointer        <w>
        .word   0                       # queue tail pointer        <w>
        .word   0                       # queue count               <w>
        .word   0                       # associated pcb            <w>

#
# --- Write VRP completion queue definitions -------------------------------------- *****
#
        .set    CMWCPRI,170
cm_wc_qu:
        .word   0                       # queue head pointer        <w>
        .word   0                       # queue tail pointer        <w>
        .word   0                       # queue count               <w>
        .word   0                       # associated pcb            <w>
#
# --- v$exec synchronization process
#
        .set    CMSECCOPYPRI,152        # cm$seccopy priority
cmw_qu:
        .word   0                       # queue head pointer        <w>
        .word   0                       # queue tail pointer        <w>
        .word   0                       # queue count               <w>
        .word   0                       # associated pcb            <w>
#
# --- Retry Change Configuration process
#
        .set    CMRCCPRI,140            # cm$Retry_RCC priority
        .set    retryRCC_to,5000        # delay 5 seconds

cm_RCC_qu:
        .word   0                       # queue head pointer        <w>
        .word   0                       # queue tail pointer        <w>
        .word   0                       # queue count               <w>
        .word   0                       # associated pcb            <w>
#
# --- CM execution queue processing table definitions
#
cm_exec_qpt:
cm_exec_qpt_high:
        .word   cm_exec_qu_high         # queue address
        .byte   0                       # number of scr active
        .byte   255                     # active scr limit
        .byte   0                       # spare
        .byte   0                       # spare

cm_exec_qpt_norm:
        .word   cm_exec_qu_norm         # queue address
        .byte   0                       # number of scr active
        .byte   8                       # active scr limit
        .byte   0                       # spare
        .byte   0                       # spare

cm_exec_qpt_low:
        .word   cm_exec_qu_low          # queue address
        .byte   0                       # number of scr active
        .byte   1                       # active scr limit
        .byte   0                       # spare
        .byte   0                       # spare

        .word   0                       # queue address
        .byte   0                       # number of scr active
        .byte   0                       # active scr limit
        .byte   0                       # spare
        .byte   0                       # spare

       .ascii   "dbug"
cm_pollerr_0:
        .byte   0
cm_pollerr_1:
        .byte   0
cm_pollerr_2:
        .byte   0
cm_pollerr_3:
        .byte   0
cm_pollerr_4:
        .byte   0
cm_pollerr_5:
        .byte   0
cm_pollerr_6:
        .byte   0
cm_pollerr_7:
        .byte   0

#
# --- Storage area for CCB log message packet .
#
        .align  4                       # align just in case
cm_ccb_log:
cm_ccb_log_type:
        .short  0                       # Type of message
                                        #   0x11e0 - start activate process
                                        #   0x11e1 - end activate process
                                        #   0x11e2 - start suspend process
                                        #   0x11e3 - end suspend process
        .short  0                       # Reserved
        .word   0x010                    # Length of data following
cm_ccb_log_reqctlsn:
        .word   0                       # request Controller serial #
cm_ccb_log_ctlsn:
        .word   0                       # Controller serial #
cm_ccb_log_rid:
        .word   0                       # RID #
cm_ccb_log_owner:
        .word   0                       # owner #

        .set cm_ccb_log_size,24         # Size

        .text
#**********************************************************************
#
#  NAME: CM$init
#
#  PURPOSE:
#       To provide a means of initializing this module.
#
#  DESCRIPTION:
#       The executive and VRP completion processes for this module are
#       established and made ready for execution.
#
#  CALLING SEQUENCE:
#       call    CM$init
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0
#       g1
#
#**********************************************************************

CM$init:
        call    CM$init2                # call djk's initialization logic

# --- Allocate the SCMT's

c       init_scmt(MAXSCMT);             # Initialize SCMT pool (never increases)

# --- Allocate the SCIO's

c       init_scio(MAXSCMT*CPSEGNUM);    # Initialize SCIO pool.

# --- Establish executive process

        lda     cm_exec,g0              # Establish executive process
# .set NO_COPIES_AT_ALL,1
.ifdef NO_COPIES_AT_ALL
c if (g0 != 0) {
    ret
c }
.endif  # NO_COPIES_AT_ALL

        ldconst CMEXECPRI,g1
c       CT_fork_tmp = (ulong)"cm$exec";
        call    K$fork
        st      g0,cm_exec_qu_high+qu_pcb # Save PCB for high queue
        st      g0,cm_exec_qu_norm+qu_pcb # Save PCB for normal queue
        st      g0,cm_exec_qu_low+qu_pcb  # Save PCB for low queue

# --- Establish VRP read completion process

        lda     cm_rd_comp,g0
c       g1 = CMRCPRI;
c       CT_fork_tmp = (ulong)"cm_rd_completion";
        call    K$fork
        st      g0,cm_rc_qu+qu_pcb      # save PCB

# --- Establish VRP write completion process

        lda     cm_wr_comp,g0
c       g1 = CMWCPRI;
c       CT_fork_tmp = (ulong)"cm_wr_completion";
        call    K$fork
        st      g0,cm_wc_qu+qu_pcb      # save PCB
#
# --- Establish V$exec synchronization process
#
        lda     cm$Vsync,g0             # Establish V$exec synchronization process
        ldconst CMSECCOPYPRI,g1
c       CT_fork_tmp = (ulong)"cm$Vsync";
        call    K$fork
        st      g0,cmw_qu+qu_pcb        # save PCB
#
# --- Establish V$exec synchronization process
#
        lda     cm$Retry_RCC,g0         # Establish Retry Change Configuration process
        ldconst CMRCCPRI,g1
c       CT_fork_tmp = (ulong)"cm$Retry_RCC";
        call    K$fork
        st      g0,cm_RCC_qu+qu_pcb     # save PCB
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: CM$que
#
#  PURPOSE:
#       To provide a common method of queuing Segment Copy Requests to
#       the Copy Manager module.
#
#  DESCRIPTION:
#       The ILT and associated RRP are queued to the tail of the
#       executive queue.  The executive is activated to process this
#       request.  This routine may be called from either the process or
#       interrupt level.
#
#  CALLING SEQUENCE:
#       call    CM$que
#
#  INPUT:
#        g1 = SCR
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
CM$que:
        ldob    scr1_priority(g1),r4    # get priority
        lda     cm_exec_qu_norm,r11     # default normal queue origin
        cmpobe.t cmp_norm,r4,.que100    # Jif normal priority

        lda     cm_exec_qu_low,r11      # default low queue origin
        bg      .que100                 # Jif low priority

        lda     cm_exec_qu_high,r11     # set high queue origin

.que100:
        b       K$cque

#**********************************************************************
#
#  NAME: CM$rc_que
#
#  PURPOSE:
#       To provide a common method of queuing the completion of a
#       read VRP request.
#
#  DESCRIPTION:
#       The ILT and associated VRP are queued to the tail of the
#       completion queue.  The executive is activated to process this
#       request.  This routine may be called from either the process or
#       interrupt level.
#
#  CALLING SEQUENCE:
#       call    CM$rc_que
#
#  INPUT:
#       g1 = ILT for copy process at SCI level
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************

CM$rc_que:
        lda     cm_rc_qu,r11            # Read VRP completion queue address
        b       K$cque

#**********************************************************************
#
#  NAME: CM$wc_que
#
#  PURPOSE:
#       To provide a common method of queuing the completion of a
#       write VRP request.
#
#  DESCRIPTION:
#       The ILT and associated VRP are queued to the tail of the
#       completion queue.  The executive is activated to process this
#       request.  This routine may be called from either the process or
#       interrupt level.
#
#       A separate completion routine handles the completion of read
#       VRP requests.
#
#  CALLING SEQUENCE:
#       call    CM$wc_que
#
#  INPUT:
#       g1 = ILT for copy process at SCI1 level
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************

CM$wc_que:
        lda     cm_wc_qu,r11            # Write VRP completion queue address
        b       K$cque

#**********************************************************************
#**********************************************************************
#**********************************************************************
#
#       S U B R O U T I N E S
#
#**********************************************************************
#**********************************************************************
#**********************************************************************
#
#**********************************************************************
#
#  NAME: cm$cal_region
#
#  PURPOSE:
#       This routine provides a common means to calculate the region
#       number associated with a specific segment.
#
#  CALLING SEQUENCE:
#       call    cm$cal_region
#
#  INPUT:
#       g0 = segment number
#
#  OUTPUT:
#       g2 = region number associated with segment
#
#  REGS DESTROYED:
#       Reg. g2 destroyed.
#
#**********************************************************************

cm$cal_region:
        shro    SMbits2wrd_sf,g0,r5     # r5 = segment bit to word shift
        shro    SMwrd2reg_sf,r5,g2      # g2 = region map index
        ret
#
#**********************************************************************
#
#  NAME:cm$set_segment_bit
#
#  PURPOSE:
#       This routine provides a common means to set a segment
#       bit associated with a specific address of the Vdisk.
#
#  CALLING SEQUENCE:
#       call    cm$set_segment_bit
#
#  INPUT:
#       g0 = segment bit number
#       g1 = number of segment bits
#       g3 = COR address of sec. copy
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************

cm$set_segment_bit:
        cmpobe  0,g1,.ssb_1000          # Jif number of segment bits 0

        movq    g0,r12

        ld      cor_rmaptbl(g3),r10     # r10 = region map table address
        cmpobne.t 0,r10,.ssb_100        # Jif region map table specified

        cmpobe 0,g4,.ssb_50             # Jif primary VRP not available
        ldob    vr_options(g4),r7       # vr_options
#
# --- Check if the primary VRP is the one generated by server on geo-RAID source vdisk
#
        bbc     vrserveroriginator,r7,.ssb_50 # Jif not server generated.
#
# --- Set error on destination vdisk
#
        setbit  vrerrorondest,r7,r7     # Set destination errror
        stob    r7,vr_options(g4)       # Update vr option
        ld      cor_destvdd(g3),r4      # r4 = destination VDD
        cmpobe  0,r4,.ssb_50            # No destination VDD, continue processing.
c       GR_SetStillInSyncFlag((VDD*)r4); # Assume the destination is still in sync
.if GR_GEORAID15_DEBUG
        ld      cor_srcvdd(g3),r5
        ldos    vd_vid(r4),r4
        ldos    vd_vid(r5),r5
c fprintf(stderr,"%s%s:%u <GR15><set_segment_bit-ladcm>Allocating rmtable..srcvid=%lx, destvid=%lx vrpoptions=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r4,r7);
.endif  # GR_GEORAID15_DEBUG
.ssb_50:
c       g0 = get_rm();                  # Allocate a region table
.ifdef M4_DEBUG_RM
c fprintf(stderr, "%s%s:%u get_rm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_RM
        mov     g0,r10                  # r10 = region map table address
        st      g0,cor_rmaptbl(g3)      # save region map table in COR
        mov     r12,g0                  # restore g0
#
# --- determine the region, segment byte index, and segment
#     bit for this segment number.
#
.ssb_100:
        and     0x1f,g0,r4              # r4 = segment bit
        shro    SMbits2wrd_sf,g0,r5     # r5 = segment bit to word shift
        shro    SMwrd2reg_sf,r5,r6      # r6 = region map index
        ldconst SMwrdidx_mask,r3        # r3 = segment_word mask
        and     r3,r5,r5                # r5 = segment map byte ndex
#
# --- determine if there is a segment map for this region
#
        ld      RM_tbl(r10)[r6*4],r8    # r8 = region address
        cmpobne  0,r8,.ssb_200          # Jif segment is there
#
# --- there is no segment map associated with this region, so allocate
#     a segment map table and link it to the region map.
#
        mov     g1,r11                  # save g1
c       g1 = get_sm();                  # Allocate a cleared segment map table
.ifdef M4_DEBUG_SM
c fprintf(stderr, "%s%s:%u get_sm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_SM
        mov     g1,r8                   # r8 = segment map table address
        st      g1,RM_tbl(r10)[r6*4]    # save new segment table address
        mov     r11,g1                  # restore g1
        mov     g0,r11                  # save g0
        mov     r6,g0                   # g0 = new dirty region #
        call    CCSM$reg_dirty          # set dirty region # in state NVRAM
        mov     r11,g0                  # restore g0
#
# --- determine if the segment bit is already set
#
.ssb_200:
        subo    r4,31,r4                # adjust bit endiance
        ld      SM_tbl(r8)[r5*4],r3     # get segment byte
        bbs     r4,r3,.ssb_250          # Jif segment bit already set
#
# --- set the segment bit in the segment map table and increment
#     the segment counter
#
        ld      SM_cnt(r8),r7           # get segment count
        setbit  r4,r3,r3                # set segment bit
        addo    1,r7,r7                 # bump segment count
        st      r3,SM_tbl(r8)[r5*4]     # save the new segment
        st      r7,SM_cnt(r8)           # save the new segment count
#
# --- determine if there are more segment bits to set
#
.ssb_250:
        subo    1,g1,g1                 # decrement number of bits
        addo    1,g0,g0                 # bump segment number
        cmpobne 0,g1,.ssb_100           # Jif another bit

.if 0   # M4_SBCH_DELETE
# Update percentage "ok".
        ld      cor_cm(g3),g4           # Set up CM address
        ld      cor_totalsegs(g3),g0    # get total # segments from COR
        st      g0,cm_totalsegs(g4)     # save total # segments in CM
        call    cm$CntRemSegs           # Count segments left to copy.
        ldconst FALSE,g0                # Do not force update to other controller
        call    cm$PctCmplt             # Change percentage variable.
        ldob    cm_lastpct(g4),r5
.ifdef M4_DEBUG_HARD
c fprintf(stderr, "%s%s:%u ssb_200 - g0=%ld, g1=%ld, g3=%p, g4=%p, r5=%ld\n", FEBEMESSAGE, __FILE__, __LINE__, r12, r13, (void*)g3, (void*)g4, r5);
.endif # M4_DEBUG_HARD
.endif  # M4_SBCH_DELETE
        movq    r12,g0

.ssb_1000:
        ret                             # return to caller


#**********************************************************************
#
#  NAME: cm$chk_segment_bit
#
#  PURPOSE:
#       This routine provides a common means to determine if
#       a segment bit associated with a specific address of
#       the Vdisk is set.
#
#  CALLING SEQUENCE:
#       call    CM$chk_segment_bit
#       call    cm$chk_segment_bit
#
#  INPUT:
#       g0 = segment bit number
#       g3 = COR address
#
#  OUTPUT:
#       g2 = FALSE - Segment map bit clear
#            TRUE  - Segment map bit set
#  REGS DESTROYED:
#       None.
#
#**********************************************************************

cm$chk_segment_bit:
        ldconst FALSE,g2                # default to segment map bit clear

        ld      cor_rmaptbl(g3),r10     # r10 = region map table address
        cmpobe.f 0,r10,.cksb_300        # Jif no region map table defined
#
# --- determine the region, segment byte index, and segment
#     bit for this segment number.
#
        and     0x1f,g0,r4              # r4 = segment bit
        shro    SMbits2wrd_sf,g0,r5     # r5 = segment bit to word shift
        shro    SMwrd2reg_sf,r5,r6      # r6 = region map index
        ldconst SMwrdidx_mask,r3        # r3 = segment_word mask
        and     r3,r5,r5                # r5 = segment map byte index
#
# --- determine if there is a segment map for this region
#
        ld      RM_tbl(r10)[r6*4],r8    # r8 = region address
        cmpobe  0,r8,.cksb_300          # Jif no segment map
#
# --- determine if the segment bit is already clear
#
        subo    r4,31,r4                # adjust bit endiance
        ld      SM_tbl(r8)[r5*4],r3     # get segment byte
        bbc     r4,r3,.cksb_300         # Jif segment bit is clear

        ldconst TRUE,g2                 # segment map bit set

.cksb_300:
        ret                             # return to caller

#**********************************************************************
#
#  NAME: cm$find_segment_bit
#
#  PURPOSE:
#       This routine provides a common means to find a segment
#       bit associated with a specific address of the Vdisk.
#
#  CALLING SEQUENCE:
#       call    cm$find_segment_bit
#
#  INPUT:
#       g3 = COR address
#
#  OUTPUT:
#       g0 = segment bit number
#       g2 = number of segment bits
#               FALSE = no segment found
#               TRUE  = segment found
#
#  REGS DESTROYED:
#       g0, g2.
#
#**********************************************************************

cm$find_segment_bit:
        mov     FALSE,g2                # default to no segment found
        mov     0,r8                    # clear indexing registers
        ld      cor_rmaptbl(g3),r10     # r10 = region map table address
        cmpobe.f 0,r10,.fsb_1000        # Jif no region map table defined

.fsb_100:
        ld      RM_tbl(r10)[r8*4],r4    # r4 = segment table address
        cmpobe  0,r4,.fsb_400           # Jif no seg tbl for this region

        ld      SM_lastwrd(r4),r9       # r9 = last word checked
        ldconst SMTBLsize/4,r3
        cmpobl  r9,r3,.fsb_150          # JIf lastword within SM map

        mov     0,r9                    # r9 = default to start of table
        st      r9,SM_lastwrd(r4)       # and correct value in sm

.fsb_150:
        mov     r9,r11                  # save a copy of starting point

.fsb_200:
        ld      SM_tbl(r4)[r9*4],r5
        cmpobe  0,r5,.fsb_300           # JIf no bits set

        scanbit r5,g0                   # g0 = bit number
        st      r9,SM_lastwrd(r4)       # save last word checked
        subo    g0,31,g0                # adjust bit endiance
        shlo    SMwrd2reg_sf,r8,r8      # region idx * segment table size
        addo    r9,r8,r8                # find bit location in segment tbl
        shlo    SMbits2wrd_sf,r8,r8     # seg idx * word size
        addo    r8,g0,g0                # add in the correct region
        mov     TRUE,g2                 # set bit found
        b       .fsb_1000               # exit

.fsb_300:
        addo    1,r9,r9                 # bump to next word
        cmpobl  r9,r3,.fsb_350          # Jif more words in seg table

        mov     0,r9                    # r9 = start of SM table

.fsb_350:
        cmpobne  r9,r11,.fsb_200        # Jif not at starting point
#
# --- Deallocate segment map table and clear from region map table
#
#
        ldconst 0,r9
        mov     g1,r15                  # save g1
        st      r9,RM_tbl(r10)[r8*4]    # clear segment map table from
                                        #  region map table
        mov     r4,g1                   # g1 = segment map table to deallocate
.ifdef M4_DEBUG_SM
c fprintf(stderr, "%s%s:%u put_sm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_SM
c       put_sm(g1);                     # Deallocate segment map table
        mov     r15,g1                  # restore g1

        mov     r8,g0                   # g0 = region number
        call    CCSM$reg_sync           # inform CCSM of region synchronization
        b       .fsb_100                # continue

.fsb_400:
        addo    1,r8,r8                 # bump to next region
        ldconst maxRMcnt,r3
        cmpobl  r8,r3,.fsb_100          # Jif more regions to check

.fsb_1000:
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$CntRemSegs
#
#  PURPOSE:
#       Counts the remaining # segments that need to be copied
#       for a sec. copy operation.
#
#  DESCRIPTION:
#       Goes through the segment management tables associated
#       with a sec. copy operation and counts the remaining
#       number of segments that need to be copied. It then
#       stores that information in cm_remsegs.
#
#  CALLING SEQUENCE:
#       call    cm$CntRemSegs
#
#  INPUT:
#       g3 = COR address
#       g4 = CM address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************

cm$CntRemSegs:
        movl    0,r4                    # r4-r5 = 0
        ldconst maxRMcnt,r6             # get max number of regions
        ld      cor_rmaptbl(g3),r10     # r10 = region map table address
        cmpobe  0,r10,.crs_300          # Jif no region map

.crs_100:
        ld      RM_tbl(r10)[r4*4],r8    # get pointer to SM table
        cmpobe  0,r8,.crs_200           # JIf no SM table
        ld      SM_cnt(r8),r3           # get remaining segment count
        addo    r3,r5,r5                # add to total

.crs_200:
        addo    1,r4,r4                 # bump region index
        cmpobl  r4,r6,.crs_100          # JIf more regions

.crs_300:
        st      r5,cm_remsegs(g4)       # save remaining segment count
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME:  CM$dealloc_RM
#
#  PURPOSE:
#       Deallocates segment and region maps tables
#
#  CALLING SEQUENCE:
#       call    CM$dealloc_RM
#
#  INPUT:
#       g3 = COR address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
CM$dealloc_RM:
        movl    g0,r12                  # save g0-g1

        ld      cor_rmaptbl(g3),g0      # g0 = region map address
        cmpobe.f 0,g0,.deRM_1000        # Jif no region map table defined
        ldconst maxRMcnt-1,r9           # r9 = number of region tables - 1
        ldconst 0,r5                    # r5 = 0

.deRM_100:
        ld      RM_tbl(g0)[r9*4],g1     # g0 = address of segment table
        cmpobe.t 0,g1,.deRM_200         # Jif no table

.ifdef M4_DEBUG_SM
c fprintf(stderr, "%s%s:%u put_sm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_SM
c       put_sm(g1);                     # Deallocate segment map table
        st      r5,RM_tbl(g0)[r9*4]     # clear out address

.deRM_200:
        cmpobe.f 0,r9,.deRM_300         # Jif last region
        subo    1,r9,r9                 # decrement RM index
        b       .deRM_100               # continue

.deRM_300:
.ifdef M4_DEBUG_RM
c fprintf(stderr, "%s%s:%u put_rm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_RM
c       put_rm(g0);                     # Release region map table (RM)
        st      r5,cor_rmaptbl(g3)      # clear pointer to RMAp

.deRM_1000:
        PushRegs(r3)                    # Save all "g" registers and g14 = 0
        call    P6_ClrAllMainRM         # clear main region map bitmap
        PopRegsVoid(r3)                 # restore environment

        movl    r12,g0                  # restore g0-g1
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME:  CM$dealloc_transRM
#         CM_dealloc_transRM
#
#  PURPOSE:
#
#       Deallocates segment and region maps tables used for transfer
#       ownership.
#
#  CALLING SEQUENCE:
#       call    CM$dealloc_transRM
#
#  INPUT:
#       g3 = COR address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
CM$dealloc_transRM:
CM_dealloc_transRM:
        movl    g0,r12                  # save g0-g1

        ld      cor_transrmap(g3),g0    # g0 = region map address
        cmpobe.f 0,g0,.detrRM_1000      # Jif no region map table defined
        ldconst maxRMcnt-1,r9           # r9 = number of region tables - 1
        ldconst 0,r5                    # r5 = 0

.detrRM_100:
        ld      RM_tbl(g0)[r9*4],g1     # g0 = address of segment table
        cmpobe.t 0,g1,.detrRM_200       # Jif no table

.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR>ladcm-dealloc_transRM-Calling putsmtbl--dealloc segment map tbl\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
.ifdef M4_DEBUG_SM
c fprintf(stderr, "%s%s:%u put_sm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_SM
c       put_sm(g1);                     # Deallocate segment map table
        st      r5,RM_tbl(g0)[r9*4]     # clear out address

.detrRM_200:
        cmpobe.f 0,r9,.detrRM_300       # Jif last region
        subo    1,r9,r9                 # decrement RM index
        b       .detrRM_100             # continue

.detrRM_300:
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR>ladcm-dealloc_transRM-Calling deallcrmtbl\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
.ifdef M4_DEBUG_RM
c fprintf(stderr, "%s%s:%u put_rm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_RM
c       put_rm(g0);                     # Release region map table (RM)
        st      r5,cor_transrmap(g3)    # clear pointer to transrmap

.detrRM_1000:
        PushRegs(r3)                    # Save all "g" registers and g14 = 0
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR>ladcm-dealloc_transRM-Calling ClrAllTransRM\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
        call    P6_ClrAllTransRM        # clear transfer region map bitmap
        PopRegsVoid(r3)                 # restore environment

        movl    r12,g0                  # restore g0-g1
        ret                             # return to caller

#**********************************************************************
#
#  NAME: cm$riv
#
#  PURPOSE:
#       To provide a common means of releasing an ILT/VRP combination
#       back to the system.
#
#  DESCRIPTION:
#       The ILT must be at nest level 1
#
#  CALLING SEQUENCE:
#       call    cm$riv
#
#  INPUT:
#       g1 = ILT/VRP combination address to release
#             il_w0 = vrp address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
cm$riv:
        movq    g0,r12                  # Save g0-g3
        ld      il_w0(g1),g2            # g2 = VRP address
        call    M$riv                   # Release ILT
#
        movq    r12,g0                  # Restore g0-g3
#
# --- Exit
#
        ret
#
#******************************************************************************
#
#  NAME:  cm$get_scr
#
#  PURPOSE:
#       Allocates and sets up an SCR for a copy operation.
#
#  DESCRIPTION:
#       Allocates an ILT to be used as a SCR and sets up the 1st two levels.
#
#  CALLING SEQUENCE:
#       call    cm$get_src
#
#  INPUT:
#       g4= CM address
#
#  OUTPUT:
#       g1 = SCR address at scr1 level
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#******************************************************************************
#
cm$get_scr:
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT

        movq    0,r12                   # r12-15 = 0
        ld      cm_cor(g4),r3           # r3 = cor address
        ld      cm_pcb(g4),r6           # r6 = pcb address
#        ldconst vrnorm,r7               # r7 = copy strategy

        ld      cor_srcvdd(r3),r4       # r4 = source VDD address
        ld      cor_destvdd(r3),r5      # r5 = destination VDD address

        st      r4,scr1_srcvdd(g1)      # save source vdd
        st      r5,scr1_dstvdd(g1)      # save destination vdd
        st      r6,scr1_pcb(g1)         # save pcb address
#        stob    r7,scr1_strategy(g1)    # save strategy
        st      g4,scr1_cm(g1)          # save cm address

        stq     r12,il_w0+ILTBIAS(g1)   # clear out second level
        stq     r12,il_w4+ILTBIAS(g1)   # clear out second level
        st      g1,scr2_scr1+ILTBIAS(g1) # save pointer to scr1 in scr2
        st      r12,scr1_actlink(g1)     # clear active SCR link
#
# --- add this SCR to CM active SCR list
#
        ld      cm_scr(g4),r4           # r4 = head of scr act queue
        st      g1,cm_scr(g4)           # g1 = new head of scr act queue
        st      r4,scr1_actlink(g1)     # link previous to current head

        ret
#
#******************************************************************************
#
#  NAME:  cm$put_scr
#
#  PURPOSE:
#       Deallocates an SCR.
#
#  DESCRIPTION:
#       Deallocates an ILT used as a SCR and removes it from the active list.
#
#  CALLING SEQUENCE:
#       call    cm$put_src
#
#  INPUT:
#       g1 = SCR address at scr1 level
#       g4 = CM address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g1 is destroyed.
#
#******************************************************************************
#
cm$put_scr:
        ld      cm_scr(g4),r4           # r4 = head scr act list
        mov     0,r5                    # r5 = last scr
        lda     0,r7                    # r7 = 0

.pscr10:
        cmpobe.f 0,r4,.pscr100          # Jif queue empty
        cmpobe.t g1,r4,.pscr20          # Jif match
        mov     r4,r5                   # r5 = new last rec. ptr.
        ld      scr1_actlink(r4),r4     # get pointer to next record
        b       .pscr10                 # and check next record out

.pscr20:
        ld      scr1_actlink(g1),r6     # r6 = scmt_link to next record on list
        cmpobne.f 0,r5,.pscr30          # Jif middle of list
        st      r6,cm_scr(g4)           # put new head ptr.
        b       .pscr100

.pscr30:
        st      r6,scr1_actlink(r5)     # save next rec. in last rec.

.pscr100:
        st      r7,scr1_actlink(g1)     # clear scmt record ptr. field

.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT (SCR)

        ret

#**********************************************************************
#
#  NAME: cm$PctCmplt
#
#  PURPOSE:
#       Provide a common means of calculating the current percentage
#       complete of a copy/resync.
#
#  DESCRIPTION:
#       Calculates the percentage complete of a copy and stores
#       that byte value in vd_scpcomp.
#
#  CALLING SEQUENCE:
#       call    cm$calremseg
#
#  INPUT:
#       g0 = FALSE - Normal Processing
#            TRUE  - Force Update
#       g3 = COR address
#       g4 = CM address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
cm$PctCmplt:
        mov     g1,r15                  # save g1
# %complete = 100-((100*remaining_segments)/total_segments)
        ld      cm_remsegs(g4),r5       # decrement remaining segments
        ld      cor_destvdd(g3),r8      # r8 = destination vdd
        cmpobe  0,r8,.pctcmplt_100      # Jif dest. VDD undefined
        ld      cm_totalsegs(g4),r4     # r4 = total segments
        ldconst 100,r10                 # r10 = multiply factor
        emul    r10,r5,r6               # r6/r7 = 100 * r5
        ediv    r4,r6,r4                # r4/r5 = remainder/quotient
        cmpobe.f 0,r4,.pctcmplt_10      # Jif no remainder
        addo    1,r5,r5                 # round up percent complete (really down!)

.pctcmplt_10:
        subo    r5,r10,r5               # invert the percent complete
        stob    r5,vd_scpcomp(r8)       # save percent complete
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
# --- determine if a percent update is to be forced
#
        cmpobe  FALSE,g0,.pctcmplt_20   # Jif normal processing

#
# --- Force an Update, But first do the required paperwork
#
        stob    r5,cm_lastpct(g4)       # Save last update point

c       r4 = (UINT32)(get_tsc()/(1000000 * ct_cpu_speed)); # seconds
        st      r4,cm_lasttime(g4)      # save current time
        b       .pctcmplt_30            # Force the update

#
# --- Determine if the percentage has changed. If so, indicate that change to
#     the other controllers.
#
.pctcmplt_20:
        ldob    cm_lastpct(g4),r3       # Make sure we changed the percentage
        cmpobe  r3,r5,.pctcmplt_100     # Jif no change (already did this one)
#
        stob    r5,cm_lastpct(g4)       # Save last update point
#
# --- We don't want to sent percent updates to often. Determine if a
#     second has passed before we send an update.
#
c       r4 = (UINT32)(get_tsc()/(1000000 * ct_cpu_speed)); # seconds
        ld      cm_lasttime(g4),r7      # r7 = last percent update
c       r7 = r7 + 1;                    # make compare be for 1 full second.
        cmpobge r7,r4,.pctcmplt_100     # Jif not one full second
        st      r4,cm_lasttime(g4)      # save current time
#
# --- send the percent update
#
.pctcmplt_30:
        mov     r5,g1                   # place % complete in correct reg
        call    CCSM$update             # distribute % complete

.pctcmplt_100:
        mov     r15,g1                  # restore g1
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$Validate_Swap
#
#  PURPOSE:
#       Provide a common means of of determining if a swap operation
#       is valid.
#
#  DESCRIPTION:
#       This routine determines if a swap operation is valid. If any
#       of the following tests is true, the swap/copy swap request
#       is rejected.
#
#       1) If the DCD is a Vlink and the current SCD either has a
#          VLAR defined or is associated with a copy operation
#          that defined it as a remote SCD.
#
#       2) If either the SCD or the DCD is a Vlink that is not
#          currently defined.
#
#  CALLING SEQUENCE:
#       call    cm$Validate_Swap
#
#  INPUT:
#       g3 = COR address
#
#  OUTPUT:
#       g0 == 0 - swap is valid
#          <> 0 - swap is invalid
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
cm$Validate_Swap:
#
# --- determine if the source and destination are the same s/n but
#     not the s/n of the CM
#
        ldconst cmcc_RSD_2vl,g0         # default tp "2vl" error
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r4         # r4 = my s/n
        ld      cor_rssn(g3),r5         # r5 = source s/n
        ld      cor_rdsn(g3),r6         # r6 = dest s/n
        cmpobe.t r4,r5,.vs_050          # JIf source is my mag (ok so far)

        cmpobe.f r5,r6,.vs_900          # Jif source and dest the same (invalid)
#
# --- determine if the copy components are still there
#
.vs_050:
        ldconst cmcc_RSD_noDV,g0        # default tp "device not defined" error
        ld      cor_srcvdd(g3),r10      # r10 = source VDD
        ld      cor_destvdd(g3),r11     # r11 = destination VDD

        cmpobe  0,r10,.vs_900           # JIf source vdd not defined
        cmpobe  0,r11,.vs_900           # Jif destination vdd not defined
#
# --- Determine if the swap can occur.
#
        ld      vd_rdd(r11),r8          # r8 = first RAID ID of destination VDD
        ldob    rd_type(r8),r4          # r4 = RAID type code
        cmpobne.t rdlinkdev,r4,.vs_800  # Jif not linked device type RAID (valid)
#
# --- the DCD is a vlink. determine if the SCD has a VLAR, if
#     it does the swap is not valid
#
        ldconst cmcc_RSD_vl2vl,g0       # default to "vl2vl" error
        ld      vd_vlinks(r10),r4       # is there a VLAR
        cmpobne 0,r4,.vs_900            # Jif yes (invalid)
#
# --- there is no VLAR associated with the SCD. Determine if the
#     there is a SCD defined as a remote SCD. If it is, the swap
#     is invalid.
#
        ld      vd_scdhead(r10),r8      # get possible SCD
        cmpobe  0,r8,.vs_800            # Jif no SCD (valid)

.vs_100:
        ldob    scd_type(r8),r9         # get scd type
        cmpobe scdt_remote,r9,.vs_900   # JIf remote (invalid)

        ld      scd_link(r8),r8         # get next SCD
        cmpobne 0,r8,.vs_100            # Jif there is another SCD
#
# --- no more SCD, swap is valid
#
.vs_800:
        ldconst cmcc_ok,g0              # default to valid
        b       .vs_910                 # exit

#
# --- log the error
#
.vs_900:
        call    cm$Log_Completion       # *** log error message

#
# --- exit
#
.vs_910:
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: CM$Log_Completion
#
#  PURPOSE:
#       To provide a common means of logging a completion status to
#       the log file.
#
#  CALLING SEQUENCE:
#       call    CM$Log_Completion
#
#  INPUT:
#       g0 = completion status
#       g3 = COR address
#
#  OUTPUT:
#        None.
#
#  REGS DESTROYED:
#        None.
#
#**********************************************************************
#
CM_Log_Completion:
cm$Log_Completion:
        ld      cor_srcvdd(g3),r6       # r6 = source VDD
        cmpobe.f 0,r6,.logcomp_1000     # Jif source VDD not defined, could be Vlink
        ld      cor_destvdd(g3),r7      # r7 = destination VDD
        cmpobe.f 0,r7,.logcomp_1000     # Jif dest. VDD not defined, could be Vlink

        mov     g0,r8                   # save g0
# Use r3, r14, and r15 for temporary storage.
#
#       Build and send a copy completion message to the epc
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mlecopycomplt,r4        # Event code
        st      r4,mle_event(g0)        # Store as word to clear other bytes

        lda     eccopycomp,r4           # r4 = function code
        stob    r4,ecs_function(g0)     # save function code
        stob    r8,ecs_compstat(g0)     # save copy completion status code

        ldob    vd_scpcomp(r7),r4       # r4 = percentage complete from dest.
        stob    r4,ecs_percent(g0)      # save completion percentage
#
        ldos    vd_vid(r7),r4           # r4 = dest. VID
        stos    r4,ecs_destvid(g0)      # save dest. vid
        ldos    vd_vid(r6),r4           # r4 = source VID
        stos    r4,ecs_copyvid(g0)      # save source vid
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], ecslen);
        mov     r8,g0                   # restore g0
.logcomp_1000:
        ret
#
#**********************************************************************
#
#  NAME: cm$force_LP
#
#  PURPOSE:
#       To provide a common means to force a Local Poll.
#
#  CALLING SEQUENCE:
#       call    cm$force_LP
#
#  INPUT:
#       g3 = COR address
#       g4 = CM address
#
#  OUTPUT:
#        None.
#
#  REGS DESTROYED:
#        None.
#
#**********************************************************************
#
cm$force_LP:
        call    cm$pksnd_local_poll     # schedule a poll
        ret                             # return to caller
#
#**********************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
