# $Id: dlmbe.as 161089 2013-05-15 21:18:36Z marshall_midden $
#**********************************************************************
#
#  NAME: dlmbe.as
#
#  PURPOSE:
#       To provide support for the Data-link Manager logic which
#       supports XIOtech Controller-to-XIOtech Controller functions
#       and services for Virtual Links.
#
#  FUNCTIONS:
#       DLM$init       - Data-link Manager initialization
#       DLM$upsul      - Update storage unit list
#       DLM$upsud      - Update storage unit data
#       DLM$est_vl     - Establish virtual linked device
#       DLM$term_vl    - Terminate virtual linked device
#       DLM$chk_ldd    - Check use of LDD and terminate if no longer needed
#       DLM$chk4vlink  - Check if VDisk is a VLink
#       DLM$add_mlpath - Add path to XIOtech Controller linked device
#       DLM$add_ftpath - Add path to Foreign Target linked device
#       DLM$VLraid     - VLink RAID I/O routine
#       DLM$proc_ldd   - Initiate LDD scan process
#       DLM$get_ldd    - Allocate a LDD structure
#       DLM$VDnamechg  - Process VDisk/VLink name change event
#       DLM$MAGnamechg - Process MAGNITUDE node name change event
#       DLM$VLopen     - Schedule VLink open process
#       DLM$chk_vlconflt - Check VLink access conflict validity routine
#       DLM$chk_vlock  - Check VDisk/VLink lock
#       DLM$VLmove     - Schedule VLink move process
#       DLM$VLreopen   - Schedule VLink open processfor all VLinks
#                        associated with a specified LDD
#       DLM$dem_path   - Demolish path (generic)
#       DLM$chg_size   - Change VDisk size on peer XIOtech Controller
#       DLM$reg_size   - Change VDisk size on peer MAGNITUDE (completion
#                        status returned to caller)
#       DLM$app_vdsize - Approve VDisk size change
#       DLM$clrldd_indx- Deallocates all LDDs from DLM_lddindx area
#                        and clears out all LDD records
#       DLM_clr_ldd    - Clears out a specific LDD record
#       DLM_put_ldd    - Returns a specific LDD record to pool
#       DLM$def_master - Notify all XIOtech controllers of the group master
#       DLM$pk_master  - Pack a Group Master Controller Definition datagram
#       DLM$chk_master - Check if I'm the current group master controller.
#                        Return the current group master controller serial
#                         number if not.
#
#       This module employs 6+ process:
#       dlm$lrpio      - LRP I/O request process (1 copy)
#       dlm$lrprty     - LRP I/O retry process (1 copy)
#       dlm$lrpcr      - LRP I/O completion process (1 copy)
#       dlm$lddx       - LDD scan process (multiple copies)
#       dlm$tpmtec     - TPMT error count reset process (1 copy)
#       dlm$vlopen     - VLink open process (multiple copies)
#       dlm$vlmove     - VLink move process (multiple copies)
#       dlm$vlock      - VDisk/VLink lock CCB info. update process (1 copy)
#       dlm$vlchk      - VLink check process (1 copy)
#
#  Copyright (c) 1999-2010 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
.ifdef TASK_STRESS_TEST
c static int Task_Torture_initialized;
.endif  # TASK_STRESS_TEST
#
# --- local equates ---------------------------------------------------
#
        .set    lrprty_to,100           # LRP retry task timeout period in msec.
        .set    lrpage_to,1000          # LRP aging task timeout period in msec.
        .set    tpmtec_to,10000         # TPMT error count reset task timeout period in msec.
#
        .set    DEBUG_APOOL,FALSE        # Apool debug
.if     MAG2MAG
#
# --- global function declarations ------------------------------------
#
        .globl  DLM$init                # Module initialization
        .globl  DLM$upsul               # Update storage unit list
        .globl  DLM$upsud               # Update storage unit data
        .globl  DLM$est_vl              # Establish virtual linked device
        .globl  DLM$term_vl             # Terminate virtual linked device
        .globl  DLM$chk_ldd             # Check use of LDD and terminate if no longer needed
        .globl  DLM$add_mlpath          # Add path to XIOtech Cntr link device
        .globl  DLM$add_ftpath          # Add path to Foreign Target link device
        .globl  DLM$VLraid              # VLink RAID I/O routine
        .globl  DLM$proc_ldd            # Initiate LDD scan process
        .globl  DLM$get_ldd             # Allocate a LDD structure
        .globl  DLM_GetLDD              # Allocate a LDD structure (C code access)
        .globl  DLM$VDnamechg           # Process VDisk/VLink name change event
        .globl  DLM$MAGnamechg          # Process MAGNITUDE node name change event
        .globl  DLM$VLopen              # Schedule VLink open process
        .globl  DLM$chk_vlconflt        # Check VLink access conflict validity routine
        .globl  DLM$chk_vlock           # Check VDisk/VLink lock
        .globl  DLM$VLmove              # Schedule VLink move process
        .globl  DLM$dem_path            # Demolish path (generic)
        .globl  DLM$reg_size            # Change VDisk size on peer MAGNITUDE
                                        #  (completion status returned to caller)
        .globl  DLM$chg_size            # Change VDisk size on peer XIOtech Cntr
        .globl  DLM$app_vdsize          # Approve VDisk size change DLM_lddindx area
        .globl  DLM_clr_ldd             # Clears out a specific LDD record
        .globl  DLM_ClrLDD              # C access
        .globl  DLM_put_ldd             # Returns a specific LDD record to pool
        .globl  DLM_PutLDD              # C access
        .globl  DLM$def_master          # Notify all XIOtech controllers of the group master
        .globl  DLM$pk_master           # Pack a Group Master Controller Definition datagram
        .globl  DLM$chk_master          # Check if I'm the current group master controller.
                                        #  Return the current group master controller serial number if not.
#async
        .globl DLM_send_async_nva       # C access
#
# --- copy manager resident routines ----------------------------------
#
        .globl  CM$qsp                  # Queue copy manager service provider request message
        .globl  CM$pkop_dmove           # Pack a Copy Device Moved datagram message
        .globl  CM$wp2_suspend          # Phase 2 write update handler routine when suspended.
        .globl  CM$wp2_null             # Null phase 2 write update handler routine.
        .globl  CM$whack_rcor           # Whack remote CORs associated with VLAR
        .globl  CMsp$srvr_inuse         # Specified copy device in use response pack/send routine.
        .globl  CM$pksnd_local_poll     # Pack and send a local poll request for the specified copy operation.
#
# --- Define Module global definition usage ---------------------------
#
        .globl  D$SendRefreshNV         # refresh NVRAM on slave controllers
#
# --- global usage data definitions -----------------------------------
#
#
# --- DLM resident data definitions
#
        .globl  DLM_servers             # Datagram services handler table
        .globl  DLM_channel_op_flags    # Channel operational flags table
.endif  # MAG2MAG
        .globl  DLM_lddindx             # Index into LDD structures
        .globl  vl_sulst                # Storage Unit List
        .globl  vl_sudata               # Storage Unit Data
        .globl  DLM_master_sn           # Group Master Controller Serial #
        .globl  DLM_vlchk_flag          # disable VLink check flag
#
# --- local usage data definitions ------------------------------------
#
        .data
vl_sulst:
        .word   0                       # Storage Unit List address
vl_sudata:
        .word   0                       # Storage Unit Data address
#
tpmt_banner:
        .ascii  "TPMT"                  # TPMT banner pattern
#
DLM_master_sn:
        .word   0                       # Group Master Controller serial #
#
# --- LRP I/O Request task queue data structure
#
dlm_lrpio_qu:
        .word   0                       # queue head pointer        <w>
        .word   0                       # queue tail pointer        <w>
        .word   0                       # queue count               <w>
        .word   0                       # associated pcb            <w>
#
# --- LRP I/O Request completion queue data structure
#
dlm_lrpcr_qu:
        .word   0                       # queue head pointer        <w>
        .word   0                       # queue tail pointer        <w>
        .word   0                       # queue count               <w>
        .word   0                       # associated pcb            <w>
#
dlm_rtyflg:
        .byte   FALSE                   # LRP I/O retry flag
#
dlm_tpmtecflg:
        .byte   FALSE                   # TPMT error count flag
#
DLM_vlchk_flag:
        .byte   0                       # disable VLink check flag
        .byte   0                       # spare
#
# --- CCB packet to notify of VLink established/terminated/swapped/size changed.
#
#- dlm_ccb_vl_type:
# <s> Type of message
#   mleestvlink = 0x0045 (est. vlink)
#   mletermvlink = 0x0046 (term. vlink)
#   mleswpvlink = 0x0047 (swap vlink)
#   mlevlszchg = 0x004D (VL Size Chg)
# <s> Reserved
# <w> Length of data following
# <w> Source MAG serial number
        .set    log_dlm_ccb_vl_srcsn,mle_event+8
# <b> Source cluster number
        .set    log_dlm_ccb_vl_srccl,log_dlm_ccb_vl_srcsn+4
# <1> Reserved
# <s> Source VLink number
        .set    log_dlm_ccb_vl_srcvd,log_dlm_ccb_vl_srccl+2
# <s> Destination VDisk number
        .set    log_dlm_ccb_vl_dstvd,log_dlm_ccb_vl_srcvd+2
# <s> Reserved
        .set    log_dlm_ccb_vl_size,log_dlm_ccb_vl_dstvd+4         # Size = 20 bytes
#
# --- Datagram services handler table
#
.if MAG2MAG
DLM_servers:
        .ascii  "DLM0"
        .word   dlm$DLM0                # DLM services #0
#
        .ascii  "CMsp"
        .word   CM$qsp                  # Copy manager service provider
#
        .word   0                       # end of table indication
.endif  # MAG2MAG
#
#
# --- MAGNITUDE Linked Device UID Template
#
DLM_magld_temp:
        .ascii  "TBLD"                  # Banner field
DLM_magld_vid:
        .short  0                       # VDisk #
DLM_magld_cl:
        .byte   0                       # Cluster #
        .byte   0                       # Reserved
DLM_magld_sn:
        .word   0                       # MAGNITUDE serial #
#
# --- Channel operational flags
#
#       Bit 7 =
#           6 =
#           5 =
#           4 =
#           3 =
#           2 =
#           1 =
#           0 = 1=Foreign Targets enabled
#
DLM_channel_op_flags:
        .space  MAXISP*1,0
#
        .align  2
.if MAG2MAG
#
#**********************************************************************
#
# --- executable code -------------------------------------------------
        .text
#
#**********************************************************************
#
#  NAME: DLM$init
#
#  PURPOSE:
#       To provide a means of initializing this module.
#
#  DESCRIPTION:
#       The executive process for this module is
#       established and made ready for execution.
#
#  INPUT:
#       g3 = FICB
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0
#       g1
#
#**********************************************************************
#
DLM$init:
#
# --- Create the Data areas needed by the code
#
c       g0 = s_MallocC(BIT31|vl_sulsiz, __FILE__, __LINE__); # Unit List allocated and cleared
        st      g0,vl_sulst             # g0 = memory address
#
c       g0 = s_MallocC(BIT31|vl_sudsiz, __FILE__, __LINE__); # Unit Data area allocated and cleared
        st      g0,vl_sudata            # g0 = memory address
#
# --- Initialize TPMT pool
#
c       init_tpmt(16);                  # Initialize TPMT memory pool.
#
# --- Establish DLM VRP executive process
#
        lda     dlm$vrpx,g0             # Establish VRP executive process
        ldconst DLMVEXECPRI,g1
c       CT_fork_tmp = (ulong)"dlm$vrpx";
        call    K$fork
        st      g0,DLM_vrp_qu+qu_pcb    # Save PCB
#
# --- Establish DLM LRP I/O request process
#
        lda     dlm$lrpio,g0            # Establish LRP I/O request process
        ldconst DLMLRPIOPRI,g1
c       CT_fork_tmp = (ulong)"dlm$lrpio";
        call    K$fork
        st      g0,dlm_lrpio_qu+qu_pcb  # Save PCB
#
# --- Establish DLM LRP I/O retry process
#
        lda     dlm$lrprty,g0           # Establish LRP I/O retry process
        ldconst DLMLRPRTYPRI,g1
c       CT_fork_tmp = (ulong)"dlm$lrprty";
        call    K$fork
#
# --- Establish DLM LRP I/O aging process
#
        lda     dlm$lrpage,g0           # Establish LRP I/O retry process
        ldconst DLMLRPAGEPRI,g1
c       CT_fork_tmp = (ulong)"dlm$lrpage";
        call    K$fork
#
# --- Establish DLM LRP I/O completion process
#
        lda     dlm$lrpcr,g0            # Establish LRP I/O completion process
        ldconst DLMLRPCRPRI,g1
c       CT_fork_tmp = (ulong)"dlm$lrpcr";
        call    K$fork
        st      g0,dlm_lrpcr_qu+qu_pcb  # Save PCB
#
# --- Establish DLM TPMT error count reset process
#
        lda     dlm$tpmtec,g0           # Establish TPMT error count reset process
        ldconst DLMTPMTECPRI,g1
c       CT_fork_tmp = (ulong)"dlm$tpmtec";
        call    K$fork
#
# --- Establish DLM datagram retry process
#
        lda     dlm$retrydg,g0          # Establish datagram retry process
        ldconst DLMRETRYDGPRI,g1
c       CT_fork_tmp = (ulong)"dlm$retrydg";
        call    K$fork
#
# --- Establish DLM VDisk/VLink lock CCB information update process
#
        lda     dlm$vlock,g0            # Establish VDisk/VLink lock update process
        ldconst DLMVLOCKPRI,g1
c       CT_fork_tmp = (ulong)"dlm$vlock";
        call    K$fork
#
# --- Establish DLM VLink poll process
#
        lda     dlm$vlchk,g0            # Establish VLink poll process
        ldconst DLMVLCHKPRI,g1
c       CT_fork_tmp = (ulong)"dlm$vlchk";
        call    K$fork
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: DLM$upsul
#
#  PURPOSE:
#       Update storage unit list.
#
#  DESCRIPTION:
#       Processes all LLDMTs found in the directory and saves the
#       DTMT info. in the storage unit list area for the CCB.
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g1 = Return Code
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
DLM$upsul:
        mov     g4,r12                  # save g4
        ld      vl_sudata,r4            # r4 = base address of storage unit data area
        ldconst deinitinprog,g1         # Report initialization in progress
        cmpobe  0,r4,.UPSUL_1000        # Jif the SUdata ptr has not been setup
        ldconst 0,r3
        stos    r3,vl_sudh_indx(r4)     # Clear index in data area
        stob    r3,vl_sudh_cnt(r4)      # clear LUN count in data area
        ld      vl_sulst,r4             # r4 = base address of storage unit list
        cmpobe  0,r4,.UPSUL_1000        # Jif the SUlist ptr has not been setup
        stos    r3,vl_sulh_cnt(r4)      # clear storage unit count
        lda     dlm_lldmt_dir,r4        # r4 = base of LLDMT directory
        ldconst MAXISP,r5               # r5 = # LLDMTs to check
.UPSUL_100:
        ld      (r4),r6                 # r6 = LLDMT being processed
        cmpobe  0,r6,.UPSUL_200         # Jif no LLDMT defined for this path
        ld      lldmt_dtmthd(r6),g4     # g4 = first DTMT on list
        cmpobe  0,g4,.UPSUL_200         # Jif no DTMTs assoc. with LLDMT
.UPSUL_150:
        stos    r3,dtmt_sulindx(g4)     # clear SUL index
        call    dlm$upsul               # put DTMT info. into storage unit list
        ld      dtmt_link(g4),g4        # g4 = next DTMT on list
        cmpobne 0,g4,.UPSUL_150         # Jif more DTMTs to process for this LLDMT
.UPSUL_200:
        addo    4,r4,r4                 # inc. to next LLDMT in directory
        subo    1,r5,r5                 # dec. LLDMT count
        cmpobne 0,r5,.UPSUL_100         # Jif more LLDMTs to process
        ldconst deok,g1                 # Set the Return Code as good
.UPSUL_1000:
        mov     r12,g4
        ret
#
#**********************************************************************
#
#  NAME: DLM$upsud
#
#  PURPOSE:
#       Update storage unit data area for the specified storage unit.
#
#  DESCRIPTION:
#       Validates the specified storage unit exists and if not returns
#       an error. If it exists, it determines whether it is a Foreign
#       Target or XIOtech Controller and processes the request as needed. If
#       an error occurs during the processing, an error is returned to
#       the caller. Otherwise, the storage unit data is stored in the
#       storage unit data area and a good status is returned to the
#       caller.
#
#  INPUT:
#       g1 = storage unit index into storage unit list
#       g2 = 32 or 64 for type of request to make.
#
#  OUTPUT:
#       g1 = completion status
#            0 = successful
#          <>0 = error occurred
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#**********************************************************************
#
DLM$upsud:
        movq    g0,r12                  # save g0-g3
#                                         Note: g2 is in r14 for checks later.
        ld      vl_sudata,r4            # r4 = base address of storage unit data area
        ldconst deinitinprog,r13        # Prep return code - initializing
        cmpobe  0,r4,.UPSUD_1000        # Jif not initialized yet
        ldconst 0,r5
        stos    g1,vl_sudh_indx(r4)     # save index in data area
        stob    r5,vl_sudh_cnt(r4)      # clear LUN count in data area
        ld      vl_sulst,r4             # r4 = base address of storage unit list
        cmpobe  0,r4,.UPSUD_1000        # Jif not initialized yet
        ldos    vl_sulh_cnt(r4),r5      # r5 = # storage units in list
        ldconst deinopdev,r13           # r13 = error code if error encountered during processing of the request
        cmpobge g1,r5,.UPSUD_1000       # Jif invalid index specified
        ldconst vl_sulr_siz,r6          # r6 = size of storage unit records
        mulo    g1,r6,r6
        addo    vl_sulh_siz,r6,r6       # r6 = index into storage unit record
        addo    r6,r4,r4                # r4 = address of storage unit record
        ld      vl_sulr_dtmt(r4),r7     # r7 = assoc. DTMT address
        cmpobe  0,r7,.UPSUD_1000        # Jif no DTMT assoc. with unit
        ldos    dtmt_sulindx(r7),r8     # r8 = SUL index from DTMT
        cmpobne r6,r8,.UPSUD_1000       # Jif index in DTMT doesn't match
        ldob    dtmt_type(r7),r8        # r8 = target type code
        cmpobne dtmt_ty_MAG,r8,.UPSUD_500 # Jif not a MAGNITUDE target
#
# --- XIOtech Controller link type target --------------------------------------------
#
#
# --- Allocate, build and send Get XIOtech Cntr Device Database Datagram Request
#
                                        # r7 = DTMT address
        movl    g10,g2                  # save g10-g11
        mov     0,g10                   # g10 = request message length not including header
c   if (r14 == 32) {
        ldconst magdd_alloc,g11         # g11 = response message length not including header
c   } else {
        ldconst magdd_alloc_GT2TB,g11   # g11 = response message length not including header
c   }
        call    DLM$get_dg              # setup datagram resources
                                        # g1 = datagram ILT at nest level 1
        movl    g2,g10                  # restore g10-g11
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r6   # r6 = local request header address
c   if (r14 == 32) {
        ldt     dlm_getmdd_hdr,r8       # r8-r11 = bytes 0-15 of request message header
c   } else {
        ldt     dlm_getmdd_hdr_GT2TB,r8 # r8-r11 = bytes 0-15 of request message header
c   }
        ld      dml_sn(r7),r11          # r11 = dest. serial #
        bswap   r11,r11                 #  in big-endian
        stq     r8,(r6)                 # save bytes 0-15 of header
c   if (r14 == 32) {
        ldq     dlm_getmdd_hdr+16,r8    # r8-r11 = bytes 16-31 of request message header
c   } else {
        ldq     dlm_getmdd_hdr_GT2TB+16,r8 # r8-r11 = bytes 16-31 of request message header
c   }
        stq     r8,16(r6)               # save bytes 16-31 of header
        ldob    dml_cl(r7),r8           # r8 = cluster number to get database for
        stob    r8,dgrq_g0(r6)          # save specified cluster #
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r6   # r6 = local response header address
        ldob    dgrs_status(r6),r11     # r11 = request completion status
        cmpobne dg_st_ok,r11,.UPSUD_300 # Jif error reported on request
        ldconst deok,r13                # r13 = good completion status
        ld      dsc2_rsbuf(g1),g3       # g3 = response message header
        lda     dgrs_size(g3),g3        # g3 = MAGDD data address
        ldob    magdd_vdrecs(g3),r8     # r8 = # VDisks defined
        cmpobe  0,r8,.UPSUD_300         # Jif no VDisks specified
        ld      vl_sudata,r10           # r10 = storage unit data area header
        stob    r8,vl_sudh_cnt(r10)     # save number of VDisks supported on this storage unit
        lda     magdd_hdr(g3),g0        # g0 = VDisk record pointer
        lda     vl_sudh_siz(r10),r10    # r10 = base address of storage unit data record area
        ldob    magdd_rsize(g3),r11     # r11 = VDisk record size
.UPSUD_250:
        ldob    magdd_vdnum(g0),r4      # r4 = VDisk #
        stos    r4,vl_sudr_lun(r10)     # save VDisk # as LUN #
        stos    r4,vl_sudr_vcnt(r10)    # save as VDisk #
        ldob    magdd_type(g0),r4       # r4 = RAID type code
        stob    r4,vl_sudr_raid(r10)    # save RAID type code
# NOTE: sector size ignored.
c   if (r14 == 32) {
        ld      magdd_sn(g0),r4         # r4 = base MAG serial #
c   } else {
        ld      magdd_sn_GT2TB(g0),r4   # r4 = base MAG serial #
c   }
        bswap   r4,r4
        st      r4,vl_sudr_bsn(r10)     # save base MAG serial #
c   if (r14 == 32) {
        ldob    magdd_cl(g0),r4         # r4 = base MAG cluster #
c   } else {
        ldob    magdd_cl_GT2TB(g0),r4   # r4 = base MAG cluster #
c   }
        stob    r4,vl_sudr_bcl(r10)     # save base MAG cluster #
c   if (r14 == 32) {
        ldob    magdd_vd(g0),r4         # r4 = base MAG VDisk #
c   } else {
        ldob    magdd_vd_GT2TB(g0),r4   # r4 = base MAG VDisk #
c   }
        stos    r4,vl_sudr_vid(r10)     # save base MAG VDisk #
c   if (r14 == 32) {
        ldob    magdd_attr(g0),r4       # r4 = VDisk attributes
c   } else {
        ldob    magdd_attr_GT2TB(g0),r4 # r4 = VDisk attributes
c   }
        stob    r4,vl_sudr_attr(r10)    # save VDisk attributes
c   if (r14 == 32) {
        ldos    magdd_snum(g0),r4       # r4 = assigned server count
c   } else {
        ldos    magdd_snum_GT2TB(g0),r4 # r4 = assigned server count
c   }
        bswap   r4,r4
        shro    16,r4,r4
        stos    r4,vl_sudr_srvcnt(r10)  # save assigned server count
c   if (r14 == 32) {
        ldos    magdd_vlnum(g0),r4      # r4 = assigned VLink count
c   } else {
        ldos    magdd_vlnum_GT2TB(g0),r4 # r4 = assigned VLink count
c   }
        bswap   r4,r4
        shro    16,r4,r4
        stos    r4,vl_sudr_vlcnt(r10)   # save assigned VLink count
c   if (r14 == 32) {
        ld      magdd_size(g0),r4       # r4-r5 = disk size in sectors
        ldconst 0,r5                    # Upper of 64 bit number.
        bswap   r4,r4
c   } else {
        ldl     magdd_size_GT2TB(g0),r4 # r4-r5 = disk size in sectors
c   }
c       *(UINT64*)&r4 = (*(UINT64*)&r4) >> 11; # r4/r5 = disk size in MBytes
        stl     r4,vl_sudr_vsiz(r10)    # save disk size
c   if (r14 == 32) {
        ldl     magdd_vdname(g0),r4     # r4-r5 = device name
c   } else {
        ldl     magdd_vdname_GT2TB(g0),r4 # r4-r5 = device name
c   }
        movl    0,r6                    # clear r6-r7
        stq     r4,vl_sudr_devsn(r10)   # save device name in device serial
        movl    0,r4                    #  number field and clear out the
        stq     r4,vl_sudr_devsn+16(r10) #  rest of the bytes
        stq     r4,vl_sudr_devsn+32(r10)
        st      r4,vl_sudr_devsn+48(r10)
        lda     vl_sudr_siz(r10),r10    # inc. to next LUN record in SUD
        addo    r11,g0,g0               # inc. to next VDisk record in MAGDD
        subo    1,r8,r8                 # dec. # VDisk records returned
        cmpobne 0,r8,.UPSUD_250         # Jif more VDisk records to process in MAGDD
.UPSUD_300:
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # return datagram resources
        b       .UPSUD_1000             # and return to caller!
#
# --- Foreign Target type ---------------------------------------------------
#
.UPSUD_500:
#
# --- Allocate, build and send Get Foreign Target Device Database SRP
#
# -- VRP header + SRP + FTDD + 2 so SRP isn't immediately after VRP.
c       r11 = vrpsiz + sr_fdd_size + ftdd_alloc + 2;
c       g3 = s_MallocC(r11, __FILE__, __LINE__); # Get VRP/SRP & FTDD
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        lda     vr_sglhdr(g3),g0        # g0 = SRP
        st      g3,vrvrp(g1)            # save the VRP address in the ILT
        st      g3,ILTBIAS+vrvrp(g1)    # save the VRP in the next level also
        ldconst vrbefesrp,r8            # set the function as a BE to FE SRP
        stos    r8,vr_func(g3)
        # If the SRP address immediately follows the VRP, then the LL_LinuxLinkLayer.c
        # code will adjust the addresses so that the Target will see the SRP in it's
        # local memory, rather than the SRP in the Initiator memory. Skip 2 bytes
        # between the VRP and SRP to force the address to be in Initiator memory
        # (the equivalent of changing the address to a PCI address for Bigfoot).
c       g0 += 2;                        # Add 2 bytes for "PCI Address translation"
        mov     g0,r8                   # Translate to global address
        st      r8,vr_sglptr(g3)        # save the SRP address in the VRP
        st      r11,vr_blen(g3)         # save the size in the VRP
        addo    r9,r10,r8               # get the size of the SRP and FTDD
        st      r8,vr_sglsize(g3)       # save the size of the SRP and FTDD
#
        ldconst srgfdd,r8               # r8 = SRP function code
c       r10 = ftdd_alloc;               # r10 = size of FTDD
        ldconst srerr,r11
        shlo    24,r11,r11
        or      r11,r8,r8
        mov     g1,r11                  # r11 = ILT/SRP address
        stq     r8,sr_func(g0)          # r8 = sr_func/sr_flag/rsvd1/sr_status
                                        # r9 = sr_plen
                                        # r10 = ftdd_alloc
                                        # r11 = sr_ilt
#
        ld      dtmt_lldid(r7),r8       # r8 = link-level driver ID
        st      r8,sr_fdd_lldid(g0)     #   and save in the SRP (sr_fdd_lldid)
        st      r7,sr_fdd_dlmid(g0)     # save the DLM session ID (sr_fdd_dlmid)
        movq    0,r8                    # clear the reserved fields
        lda     sr_fdd_size(g0),r10     # r10 = FTDD (global addr)
        stq     r8,srpbsiz+sr_source(g0) # save sr_source (cleared)
                                        #  + sr_destination (cleared)
                                        #  + sr_fdd_ftdd
                                        #  + sr_rsvd (cleared)

        ld      dtmt_lldmt(r7),r11      # r11 = assoc. LLDMT (ILT/VRP)
        ldob    dlmi_path-ILTBIAS(r11),r10 # r10 = Path #
        st      g0,dlmi_srp(g1)         # save SRP address in ILT
        st      r11,il_w3(g1)           # save assoc. ILT/VRP address in ILT
        ld      lldmt_vrp(r11),r11      # get the vrp
        ld      vr_ilt(r11),r11         # get the ILT/VRP on the FE
        st      r11,sr_vrpilt(g0)       # save the ILT/VRP in the srp
        st      r10,dlmi_path(g1)       # save Path # in ILT
        mov     g0,g2                   # g2 = SRP address
        lda     L$que,g0                # g0 = Link 960 Queue (wipes g0-g3,g14)
        movq    g0,r8                   # save g0-g3
        mov     g14,r3                  # save g14
        call    K$qwlink                # Queue request w/wait
        movq    r8,g0                   # restore g0-g3
        mov     r3,g14                  # restore g14
#
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        ldob    vr_status(g3),r11       # r11 = VRP completion status (really the SRP status)
        cmpobne srok,r11,.UPSUD_900     # Jif error reported on SRP
        ldconst deok,r13                # r13 = good completion status
        lda     sr_fdd_size(g2),r3      # r3 = FTDD address
        ldob    ftdd_lunrecs(r3),r8     # r8 = # LUNs defined
        cmpobe  0,r8,.UPSUD_900         # Jif no LUNs specified
        mov     0,r9                    # r9 = # disk LUNs defined
        lda     ftdd_hdr(r3),g0         # g0 = LUN record pointer
        ld      vl_sudata,r10           # r10 = base address of the storage
        lda     vl_sudh_siz(r10),r10    #   unit data record area
        ldob    ftdd_rsize(r3),r11      # r11 = LUN record size
        ldconst SECSIZE,r3              # r3 = valid sector size supported
.UPSUD_550:
        ldob    ftdd_dtype(g0),r4       # r4 = device type code
        cmpobne 0,r4,.UPSUD_560         # Jif not a disk type device
        ldos    ftdd_secsz(g0),r4       # r4 = sector size
        cmpobne r3,r4,.UPSUD_560        # Jif not the supported sector size
        ldob    ftdd_lnum(g0),r4        # r4 = LUN #
        stos    r4,vl_sudr_lun(r10)     # save LUN #
        stos    r4,vl_sudr_vcnt(r10)    # save as VDisk #
        mov     0,r4
        stob    r4,vl_sudr_raid(r10)    # clear RAID type
        st      r4,vl_sudr_bsn(r10)     # clear base MAG serial #
        st      r4,vl_sudr_bcl(r10)     # clear base MAG cluster #
                                        # + base MAG VDisk #
                                        # + assigned server count
                                        # + assigned VLink count
        ldq     ftdd_sn(g0),r4          # r4-r7 = device serial #
        stq     r4,vl_sudr_devsn(r10)   # save device serial # and clear
        movq    0,r4                    #   out the rest of the field
        stq     r4,vl_sudr_devsn+16(r10)
        stq     r4,vl_sudr_devsn+32(r10)
        st      r4,vl_sudr_devsn+48(r10)
        ldl     ftdd_size(g0),r4        # r4-r5 = disk size in sectors
c       *(UINT64*)&r4 = *(UINT64*)&r4 / (2 * 1024); # Convert to mb for display.
        stl     r4,vl_sudr_vsiz(r10)    # save disk size
        addo    1,r9,r9                 # inc. LUN counter
        lda     vl_sudr_siz(r10),r10    # inc. to next LUN record in SUD
.UPSUD_560:
        addo    r11,g0,g0               # inc. to next LUN record in FTDD
        subo    1,r8,r8                 # dec. # LUN records returned
        cmpobne 0,r8,.UPSUD_550         # Jif more LUN records to process in FTDD
        ld      vl_sudata,r10           # r10 = storage unit data area header
        stob    r9,vl_sudh_cnt(r10)     # save number of LUNs supported on this storage unit
.UPSUD_900:
        ld      vr_blen(g3),g1          # g1 = size of VRP, SRP, and FTDD
c       s_Free_and_zero(g3, g1, __FILE__, __LINE__); # release memory for VRP, SRP, and FTDD
.UPSUD_1000:
                                        # r13 = completion status
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: DLM$est_vl
#
#  PURPOSE:
#       Performs the processing to establish a virtual linked device.
#
#  DESCRIPTION:
#       Validates the input parameters and returns an error if
#       there are either bad parameters or duplicate elements
#       of the requested virtual link. If not, performs the
#       processing necessary to establish the elements of the
#       linked device.
#
#  INPUT:
#       g0 = LUN oridinal # of storage unit data (i.e. might be lower missing)
#       g1 = storage unit index into storage unit list (cid)
#       g3 = 16-bit VDisk # of VLink being created
#       g4 = establish VLink attributes
#            Bit 7 = 1=exclusive access
#                6 = 1=forced access
#                5 =
#                4 =
#                3 =
#                2 =
#                1 =
#                0 =
#
#  OUTPUT:
#       g0 = LDD address of linked device if successful
#       g1 = completion status
#            0 = successful
#          <>0 = error occurred
#
#  REGS DESTROYED:
#       Reg. g0, g1 destroyed.
#
#**********************************************************************
#
DLM$est_vl:
        movq    g0,r12                  # save g0-g3
                                        # r12 = LUN original # of storage unit data
                                        # r13 = storage unit index into storage unit list
                                        # r15 = 16-bit VDisk #
        ld      vl_sudata,r4            # r4 = base address of storage unit data area
        ldos    vl_sudh_indx(r4),r5     # r5 = index of unit data
        ldconst deinopdev,r13           # r13 = error code if error encountered during processing of the request
        cmpobne g1,r5,.ESTVL_1000       # Jif not the specified index
        ldob    vl_sudh_cnt(r4),r5      # r5 = # LUNs in data area
        cmpoble r5,g0,.ESTVL_1000       # Jif specified LUN # > LUN count in data area
        lda     vl_sudh_siz(r4),r4      # r4 = pointer to data records
        ldconst vl_sudr_siz,g3          # g3 = size of data records
        mulo    g0,g3,g3                # g3 = index into data records
        addo    r4,g3,g3                # g3 = pointer to specified LUN data record
        ld      vl_sulst,r4             # r4 = base address of storage unit list area
        ldconst vl_sulr_siz,r6          # r6 = size of storage unit records
        mulo    g1,r6,r6
        addo    vl_sulh_siz,r6,r6       # r6 = index into storage unit record
        addo    r6,r4,r4                # r4 = address of storage unit record
        ld      vl_sulr_dtmt(r4),r7     # r7 = assoc. DTMT address
        cmpobe  0,r7,.ESTVL_1000        # Jif no DTMT assoc. with unit
        ldos    dtmt_sulindx(r7),r8     # r8 = SUL index from DTMT
        cmpobne r6,r8,.ESTVL_1000       # Jif index in DTMT doesn't match
        ldob    dtmt_type(r7),r8        # r8 = target type code
        cmpobne dtmt_ty_MAG,r8,.ESTVL_500 # Jif not a MAGNITUDE target
#
# --- XIOtech Controller link type target -------------------------------------
#
        ldos    vl_sudr_vcnt(g3),r12    # r12 = VDisk # from specified VDisk record
        stos    r12,DLM_magld_vid       # save VDisk # in UID template
        ldob    dml_cl(r7),r5           # r5 = XIOtech Controller cluster #
        ld      dml_sn(r7),r6           # r6 = XIOtech Controller serial #
        stob    r5,DLM_magld_cl         # save cluster # in UID template
        st      r6,DLM_magld_sn         # save serial # in UID template
        ldt     DLM_magld_temp,g0       # g0-g2 = UID template to check for
        movt    g0,r4                   # r4-r6 = UID template of linked device
        call    dlm$chk4ldd             # check if LDD already defined for XIOtech Cntr/cluster/VDisk combo
                                        # g0 = LDD address if match found
        mov     g0,r3                   # r3 = existing LDD address
        cmpobne 0,g0,.ESTVL_264         # Jif LDD already exists
        call    dlm$get_ldd             # allocate a LDD combo and register them in the LDD table
                                        # g0 = LDD address if available
        cmpobne 0,g0,.ESTVL_260         # Jif LDD allocated
        ldconst demaxsegs,r13           # Can't add any more LDDs.
        b       .ESTVL_1000             # Return error to caller.
#
.ESTVL_260:
        ld      K_ficb,r9               # r9 = FICB address
        ld      fi_vcgid(r9),r9         # r9 = this controllers serial #
        st      r9,ld_owner(g0)         # save the owning controller number
        stt     r4,ld_serial(g0)        # save UID as serial #
        st      r6,ld_basesn(g0)        # save the other XIOtech Controller S/N
        ldob    DLM_magld_cl,r5         # r5 = Cluster number to work with
        stob    r5,ld_basecl(g0)        # save the other XIOtech Cluster number
        stos    r12,ld_basevd(g0)       # save the other XIOtech VDisk number
        ldl     dtmt_nwwn(r7),r4        # r4-r5 = node WWN
        stl     r4,ld_basenode(g0)      # save the other XIOtech Node WWN
        ldconst ldmld,r4                # r4 = LDD class
        stob    r4,ld_class(g0)         # save LDD class
        ldconst ldd_st_uninit,r4
        stob    r4,ld_state(g0)         # set LDD state to uninitialized
.ESTVL_264:
        mov     r15,g3                  # g3 = 16-bit VDisk #
        ldconst 3,g3                    # g3 = error retry count
.ESTVL_265:
c       g1 = 64;                        # size of DLM packet request.
c       r8 = 32;                        # Flag for other request to do if fails.
.ESTVL_266:
        call    dlm$pkest_vl            # pack an establish VLink datagram to register VDisk being created
                                        # g1 = datagram ILT at nest level 2
        mov     g0,r6                   # r6 = assoc. LDD address
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait

        mov     r6,g0                   # g0 = assoc. LDD address
        ld      dsc2_rshdr_ptr(g1),g2   # g2 = local response header address
        ldob    dgrs_status(g2),r11     # r11 = request completion status
        cmpobe  dg_st_ok,r11,.ESTVL_280 # Jif no error reported on request
c   if (r8 == 32) {
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # return datagram resources
c       g1 = 32;
c       r8 = 0;                         # Flag that we don't have 32 to try on next failure.
        b       .ESTVL_266
c   }
        cmpobe  0,g3,.ESTVL_270         # Jif error retry count expired
        mov     g1,r11                  # r11 = datagram ILT at nest level 2
        call    DLM$chk_vlconflt        # check if VLink conflict not valid
        cmpobne g1,r11,.ESTVL_268       # VLink conflict is not valid. Swap VLink lock.
        subo    1,g3,g3                 # dec. error retry count
        ldconst 1000,g0                 # delay for awhile
        call    K$twait
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # return datagram resources
        mov     r6,g0                   # g0 = assoc. LDD address
        b       .ESTVL_265              # Try 64 byte version of DLM again
#
#---  Vlink conflict is not valid, Swap VLink lock (generated in DLM$chk_vlconflt).
#
.ESTVL_268:
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait

        mov     r6,g0                   # g0 = assoc. LDD address
        ld      dsc2_rshdr_ptr(g1),g2   # g2 = local response header address
        ldob    dgrs_status(g2),r11     # r11 = request completion status
        cmpobe  dg_st_ok,r11,.ESTVL_280 # Jif no error reported on request
# Know that 64 bit dlm$pkswp_vl failed, try 32 bit version.
        lda     -dsc1_ulvl(g1),g1       # g1 = specified datagram ILT at nest level #1
        call    DLM$put_dg              # deallocate specified datagram resources
c       g1 = 32;                        # Try 32 bit version.
        call    dlm$pkswp_vl            # pack a Swap VLink Lock datagram
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait

        mov     r6,g0                   # g0 = assoc. LDD address
        ld      dsc2_rshdr_ptr(g1),g2   # g2 = local response header address
        ldob    dgrs_status(g2),r11     # r11 = request completion status
        cmpobe  dg_st_ok,r11,.ESTVL_280 # Jif no error reported on request
# Try sleeping for a second and trying again (unless retry count expired).
        lda     -dsc1_ulvl(g1),g1       # g1 = specified datagram ILT at nest level #1
        call    DLM$put_dg              # deallocate specified datagram resources
        cmpobe  0,g3,.ESTVL_340         # Jif error retry count expired
        subo    1,g3,g3                 # dec. error retry count
        ldconst 1000,g0                 # delay for awhile
        call    K$twait
        mov     r6,g0                   # g0 = assoc. LDD address
c       g1 = 64;                        # Try 64 bit version.
        call    dlm$pkswp_vl            # pack a Swap VLink Lock datagram
        b       .ESTVL_268              # and try it again
#
# --- Datagram not successfully sent, retry count exceeded.
#
.ESTVL_270:
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # return datagram resources
        b       .ESTVL_340              # and return error to caller
#
# --- Datagram successfully sent -- don't care which type, they return the same structure.
#
.ESTVL_280:
        ld      dsc2_rsbuf(g1),g3       # g3 = response message header
        ld      dgrs_resplen(g2),r11    # r11 = remaining response length
        lda     dgrs_size(g3),g3        # g3 = response data address
        bswap   r11,r11
c   if (r11 != DLM0_rs_estvl_size && r11 != DLM0_rs_estvl_size_GT2TB) {
        b       .ESTVL_270              # Jif not right amount of data in datagram response
c   }
        ld      DLM0_rs_estvl_basesn(g3),r6 # r6 = base MAG serial #
        bswap   r6,r6                   #  in little-endian format
        st      r6,ld_basesn(g0)        # save base MAG serial # in LDD
        ldob    DLM0_rs_estvl_basecl(g3),r6 # r6 = base MAG cluster #
        stob    r6,ld_basecl(g0)        # save base MAG cluster # in LDD
        ldob    DLM0_rs_estvl_basevd(g3),r6 # r6 = base MAG VDisk #
        stos    r6,ld_basevd(g0)        # save base MAG VDisk #
        ldob    DLM0_rs_estvl_altid(g2),r6 # r6 = MSB of alternate ID
        stob    r6,ld_altid+1(g0)       # save MSB of alternate ID
        ldob    DLM0_rs_estvl_altid+1(g2),r6 # r6 = LSB of alternate ID
        stob    r6,ld_altid(g0)         # save LSB of alternate ID
#
        ldl     ld_basename(g0),r8      # r8-r9 = current device name
c   if (r11 == DLM0_rs_estvl_size) {
        ldl     DLM0_rs_estvl_name(g3),r4 # r4-r5 = device name
c   } else {
        ldl     DLM0_rs_estvl_name_GT2TB(g3),r4 # r4-r5 = device name
c   }
        cmpobne r8,r4,.ESTVL_290        # Jif name is not the same
        cmpobe  r9,r5,.ESTVL_295        # Jif name is the same
.ESTVL_290:
        stl     r4,ld_basename(g0)      # Save the name
#
        movl    g0,r4
        ldos    ld_ord(g0),g0           # Set input parm (VID)
        ldconst ecnvlink,g1
        call    D$changename            # Post it
        movl    r4,g0                   # Restore g0/g1
#
.ESTVL_295:
c   if (r11 == DLM0_rs_estvl_size) {
        ld      DLM0_rs_estvl_dsiz(g3),r8 # r8,r9 = VDisk size in sectors
        ldconst 0,r9
        bswap   r8,r8
c   } else {
        ldl     DLM0_rs_estvl_dsiz_GT2TB(g3),r8 # r8,r9 = VDisk size in sectors
c   }
        stl     r8,ld_devcap(g0)        # save size in LDD
#
# --- Save the LDD to NVRAM
#
        ldos    K_ii+ii_status,r4       # Get current initialization status
        bbs     iimaster,r4,.ESTVL_297  # Jif already master
        PushRegs                        # Save all G registers (stack relative)
                                        # g0 = LDD Address
        ldconst lddsiz,g1               # g1 = Size of the LDD
        ldconst ebiseLDD,g2             # g2 = Event type is "LDD"
        ldconst ebibtmaster,g3          # g3 = Broadcast event to the master
        mov     0,g4                    # g4 = Serial Number (not used)
        call    D$reportEvent           # Send the updated LDD to the master
        PopRegsVoid                     # Restore all G registers (stack relative)
.ESTVL_297:
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # return datagram resources
                                        # g0 = assoc. LDD address
        call    DLM$proc_ldd            # start LDD scan process
        ld      ld_pcb(g0),r4           # r4 = LDD scan process PCB address
        mov     g0,g1                   # save g1
        ldconst 50,r5                   # r5 = max. timeout delay count
.ESTVL_300a:
        ldconst 100,g0                  # g0 = timeout value (in msec.)
        call    K$twait                 # delay to allow scan process to complete
        ld      ld_pcb(g1),g0           # g0 = LDD scan process PCB
        cmpobne g0,r4,.ESTVL_300b       # Jif scan process complete
        subo    1,r5,r5                 # dec. timeout delay count
        cmpobne 0,r5,.ESTVL_300a        # Jif max. timeout count not expired
#
# --- LDD scan process wait has expired
#
        mov     g1,g0                   # restore g0
        b       .ESTVL_330
#
.ESTVL_300b:
        mov     g1,g0                   # restore g0
        ldob    ld_state(g0),r4         # r4 = LDD state
        cmpobne ldd_st_op,r4,.ESTVL_330 # Jif LDD state not operational
#
# --- Path to linked device was successful
#
        ldconst deok,r13                # r13 = completion status
        mov     g0,r12                  # r12 = LDD address of linked device
        b       .ESTVL_1000             # and we're out of here!
#
# --- No paths available to target. Unregister VLink.
#
.ESTVL_330:
        mov     r15,g3                  # g3 = 16-bit VDisk #
        call    DLM$term_vl             # unregister VLink
.ESTVL_340:
        cmpobne 0,r3,.ESTVL_1000        # Jif LDD existed prior to being called
        call    DLM$chk_ldd             # terminate LDD if no one using it
        b       .ESTVL_1000             # and we're out of here!
#
# --- Foreign Target type ---------------------------------------------------
#
# g0 = LUN oridinal # of storage unit data (i.e. might be lower missing)
# g3 = pointer to specified LUN data record.
# r7 = assoc. DTMT address
# r8 = target type code
# r12 = LUN oridinal # of storage unit data
# r13 = storage unit index into storage unit list
# r15 = 16-bit VDisk #
#
.ESTVL_500:
#
# --- Scan TPMT list to see if LDD already defined for specified LUN
#
        ldt     vl_sudr_devsn(g3),g0    # g0-g2 = device serial #
c       r14 = g3;                       # save g3
        movt    g0,r4                   # r4-r6 = UID template of linked device
        ldos    vl_sudr_lun(r14),g3     # g3 = real LUN number
        call    dlm$chk4lddlun          # check if LDD already defined for this Foreign Target LUN
                                        # g0 = LDD address if match found
c       g3 = r14;                       # restore g3
        mov     g0,r3                   # r3 = existing LDD address
        cmpobne 0,g0,.ESTVL_600         # Jif LDD already exists
        call    dlm$get_ldd             # allocate a LDD combo and register them in the LDD table
                                        # g0 = LDD address if available
        cmpobne 0,g0,.ESTVL_560         # Jif LDD allocated
        ldconst demaxsegs,r13           # Can't add any more LDDs.
        b       .ESTVL_1000             # Return error to caller.
#
.ESTVL_560:
        ld      K_ficb,r9               # r9 = FICB address
        ld      fi_vcgid(r9),r9         # r9 = this controllers serial #
        st      r9,ld_owner(g0)         # save the owning controller number
        stt     r4,ld_serial(g0)        # save UID as serial #
c       r6 = (r4 & 0xffff) | 0x8000;    # Use WWN in serial number to generate big serial number.
        st      r6,ld_basesn(g0)        # save generated serial number from wwn
        ldos    vl_sudr_lun(g3),r4      # r4 = LUN # from specified LUN record
        stos    r4,ld_lun(g0)           # save LUN # in LDD
        ld      dft_rev(r7),r4          # r4 = product revision number
        st      r4,ld_rev(g0)           # save product revision number in LDD
        ldl     dft_venid(r7),r4        # r4-r5 = vendor ID
        stl     r4,ld_vendid(g0)        # save vendor ID in LDD
        ldt     dft_prid(r7),r4         # r4-r6 = product ID
        stt     r4,ld_prodid(g0)        # save product ID in LDD
        ld      dft_prid+12(r7),r4
        st      r4,ld_prodid+12(g0)
        ldl     dtmt_nwwn(r7),r4        # r4-r5 = node WWN
        stl     r4,ld_basenode(g0)      # save the Node WWN
        ldconst ldftd,r4                # r4 = LDD class
        stob    r4,ld_class(g0)         # save LDD class
        ldconst ldd_st_uninit,r4
        stob    r4,ld_state(g0)         # set LDD state to uninitialized
#
# --- Save the LDD to NVRAM
#
        ldos    K_ii+ii_status,r4       # Get current initialization status
        bbs     iimaster,r4,.ESTVL_570  # Jif already master
        PushRegs                        # Save all G registers (stack relative)
                                        # g0 = LDD Address
        ldconst lddsiz,g1               # g1 = Size of the LDD
        ldconst ebiseLDD,g2             # g2 = Event type is "LDD"
        ldconst ebibtmaster,g3          # g3 = Broadcast event to the master
        mov     0,g4                    # g4 = Serial Number (not used)
        call    D$reportEvent           # Send the updated LDD to the master
        PopRegsVoid                     # Restore all G registers (stack relative)
.ESTVL_570:
#
.ESTVL_600:
                                        # g0 = assoc. LDD address
        call    DLM$proc_ldd            # start LDD scan process
        ld      ld_pcb(g0),r4           # r4 = LDD scan process PCB address
        mov     g0,g1                   # save g1
        ldconst 50,r5                   # r5 = max. timeout delay count
.ESTVL_600a:
        ldconst 100,g0                  # g0 = timeout value (in msec.)
        call    K$twait                 # delay to allow scan process to complete
        ld      ld_pcb(g1),g0           # g0 = LDD scan process PCB
        cmpobne g0,r4,.ESTVL_600b       # Jif scan process complete
        subo    1,r5,r5                 # dec. timeout delay count
        cmpobne 0,r5,.ESTVL_600a        # Jif max. timeout count not expired
#
# --- LDD scan process wait has expired
#
        mov     g1,g0                   # restore g0
        b       .ESTVL_630
#
.ESTVL_600b:
        mov     g1,g0                   # restore g0
        ldob    ld_state(g0),r4         # r4 = LDD state
        cmpobne ldd_st_op,r4,.ESTVL_630 # Jif LDD state not operational
#
# --- Path to linked device was successful
#
        ldconst deok,r13                # r13 = completion status
        mov     g0,r12                  # r12 = LDD address of linked device
        b       .ESTVL_1000             # and we're out of here!
#
.ESTVL_630:
        cmpobne 0,r3,.ESTVL_1000        # Jif LDD existed prior to being called
        call    DLM$chk_ldd             # terminate LDD if no one using it
.ESTVL_1000:
                                        # r12 = LDD address of linked device if successful
                                        # r13 = completion status
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: DLM$term_vl
#
#  PURPOSE:
#       Performs the processing to terminate a virtual linked device.
#
#  DESCRIPTION:
#       Determines if the associated LDD is a XIOtech Controller Link target
#       and if not just returns to the caller. If it is, it builds
#       and sends a Terminate VLink datagram to terminate the VLink
#       session to the destination XIOtech Controller.
#
#  INPUT:
#       g0 = assoc. LDD address being terminated
#       g3 = 16-bit VLink # being terminated
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
DLM$term_vl:
        ldob    ld_class(g0),r5         # r5 = linked device type code
        cmpobne ldmld,r5,.termvl_1000   # Jif not MAGNITUDE link type target
        movq    g0,r12                  # save g0-g3
        call    dlm$pktrm_vl            # pack a Terminate VLink datagram
                                        # g1 = datagram ILT at nest level 2
        ldconst 16,g0                   # g0 = datagram error retry count
        call    DLM$just_senddg         # just send out datagram w/error retry
        movq    r12,g0                  # restore g0-g3
        ld      V_vddindx[g3*4],r5      # r5 = VDD
        cmpobe  0,r5,.termvl_1000       # Jif no VDD defined
        ld      vd_rdd(r5),r5           # r5 = RDD address of first RAID
        cmpobe  0,r5,.termvl_1000       # Jif no RDD defined
        mov     g5,r12                  # save g5
        ld      rd_vlop(r5),g5          # g5 = assoc. VLOP
        cmpobe  0,g5,.termvl_500        # Jif no VLOP assoc. with RDD
        ld      vlop_ehand(g5),r4       # r4 = VLOP event handler table
        ld      vlop_eh_abort(r4),r5    # r5 = abort event handler routine
        callx   (r5)                    # call abort event handler routine
.termvl_500:
        mov     r12,g5                  # restore g5
.termvl_1000:
        ret
#
#**********************************************************************
#
#  NAME: DLM$chk_ldd
#
#  PURPOSE:
#       Terminates a linked device (LDD) if no RAIDs assigned
#       to it.
#
#  DESCRIPTION:
#       Checks all RAIDs to see if they are linked devices and if
#       assigned to the specified linked device. If any RAIDs are
#       assigned to the specified linked device, no further processing
#       is performed. If no RAIDs are assigned to the specified linked
#       device, the linked device is terminated, the resources used
#       for the linked device are returned and the NVRAM is updated
#       to save off the configuration changes.
#
#  INPUT:
#       g0 = LDD address to process
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
DLM$chk_ldd:
        movq    g0,r12                  # save g0-g3
                                        # r12 = LDD address to process
        ldconst 0,r11                   # r11 = RAID # being checked
        ldconst MAXRAIDS,r10            # r10 = max. # RAIDs supported
.chkldd_100:
        ld      R_rddindx[r11*4],r3     # r3 = RDD address
        cmpobe  0,r3,.chkldd_200        # Jif RAID doesn't exist
        ldob    rd_type(r3),r4          # r4 = RAID type code
        cmpobne rdlinkdev,r4,.chkldd_200 # Jif not linked device RAID type
        ld      rd_psd(r3),r4           # r4 = first PSD address
        ldos    ps_pid(r4),r5           # r5 = assoc. LDD Index
        ld      DLM_lddindx[r5*4],r5    # r5 = LDD address
        cmpobe  r5,g0,.chkldd_1000      # Jif assoc. with specified LDD
.chkldd_200:
        addo    1,r11,r11               # inc. RAID # being checked
        cmpobl r11,r10,.chkldd_100      # Jif more RAIDs to check
#
# --- No VLinks exist to specified LDD. Terminate linked device.
#
        ldconst ldd_st_pterm,r4         # r4 = pending terminate state code
        stob    r4,ld_state(g0)         # set LDD state to pending terminate
        call    DLM$proc_ldd            # start LDD scan to terminate LDD
.chkldd_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: DLM$add_mlpath
#
#  PURPOSE:
#       Attempts to add a path to the XIOtech Controller linked device.
#
#  DESCRIPTION:
#       Allocates a TPMT to use for a new path to the specified
#       XIOtech Controller linked device and performs the processing to
#       establish a new path to the linked device.
#
#  INPUT:
#       g0 = LDD address of linked device to add path to
#       g1 = assoc. DTMT address of target to add path to
#
#  OUTPUT:
#       g2 = completion status
#            0          = successful
#            deunasspath = path blocked from use
#            deinopdev  = path failed to open successfully
#
#  REGS DESTROYED:
#       Reg. g2 destroyed.
#
#**********************************************************************
#
DLM$add_mlpath:
        movq    g0,r12                  # save g0-g3
                                        # r12 = LDD address
                                        # r13 = DTMT address
        ld      dtmt_lldmt(g1),r6       # r6 = assoc. LLDMT address
        ldob    ld_pmask(g0),r4         # r4 = path mask byte
        ldob    dlmi_path-ILTBIAS(r6),r7 # r7 = Path # of requested path
        bbs     r7,r4,.addmlpath_20     # Jif path open for use
        ldconst deunasspath,r14         # return unassigned path error code
        b       .addmlpath_1000
#
.addmlpath_20:
        mov     deinopdev,r14           # r14 = error completion status
c       g3 = get_tpmt();                # Allocate a TPMT to open session with
.ifdef M4_DEBUG_TPMT
c fprintf(stderr, "%s%s:%u get_tpmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g3);
.endif # M4_DEBUG_TPMT
        st      g0,tpm_ldd(g3)          # save assoc. LDD address in TPMT
        st      g1,tpm_dtmt(g3)         # save assoc. DTMT address in TPMT
        ldob    ld_ppri(g0),r4          # r4 = path priority byte
        ldconst tpm_ty_pri,r5           # r5 = primary type code
        bbs     r7,r4,.addmlpath_40     # Jif primary path type
        ldconst tpm_ty_sec,r5           # r5 = secondary type code
.addmlpath_40:
        stob    r5,tpm_type(g3)         # save path type code in TPMT
        ldconst tpm_st_popen,r5         # r5 = TPMT state
        stob    r5,tpm_state(g3)        # set TPMT state
#
# --- Link TPMT onto TPMT list in DTMT & LDD
#
        call    dlm$add_tpmt            # add TPMT to LDD list and DTMT list
#
# --- Allocate, build and send Open Session to XIOtech Controller SRP
#
        ldconst vrpsiz,r8               # r8 = size of VRP header
        ldconst sr_oml_size,r9          # r9 = size of SRP
        lda     mlossiz,r10             # r10 = size of MLOSPB
        addo    r8,r9,r8                # r8 = size of VRP and SRP
        addo    r8,r10,g0               # g0 = size of VRP, SRP, & MLOSPB
c       g0 += 2;                        # Add 2 extra bytes so SRP isn't immediately after VRP
        mov     g0,r11                  # r11 = save size of VRP, SRP, & MLOSPB
c       g0 = s_MallocC(g0, __FILE__, __LINE__); # Get VRP/SRP & MLOSPB
                                        # g0 = memory address
        mov     g0,r3                   # r3 = VRP (save for later use)
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        lda     vr_sglhdr(g0),g0        # g0 = SRP
        st      r3,vrvrp(g1)            # save the VRP address in the ILT
        st      r3,ILTBIAS+vrvrp(g1)    # save the VRP in the next level also
        ldconst vrbefesrp,r8            # set the function as a BE to FE SRP
        stos    r8,vr_func(r3)
        # If the SRP address immediately follows the VRP, then the LL_LinuxLinkLayer.c
        # code will adjust the addresses so that the Target will see the SRP in it's
        # local memory, rather than the SRP in the Initiator memory. Skip 2 bytes
        # between the VRP and SRP to force the address to be in Initiator memory
        # (the equivalent of changing the address to a PCI address for Bigfoot).
c       g0 += 2;                        # Add 2 bytes for "PCI Address translation"
        mov     g0,r8                   # Translate to global address
        st      r8,vr_sglptr(r3)        # save the SRP address in the VRP
        st      r11,vr_blen(r3)         # save the size in the VRP
        addo    r9,r10,r8               # get the size of the SRP and MLOSPB
        st      r8,vr_sglsize(r3)       # save the size of the SRP and MLOSPB
#
        mov     0,r5                    # r5 = zero register
        ldconst sropml,r8               # r8 = SRP function code
        ldconst srerr,r11
        shlo    24,r11,r11
        or      r11,r8,r8
        mov     g1,r11                  # r11 = ILT/SRP address
        ld      dml_sn(r13),r10         # r10 = XIOtech Controller serial number
        stq     r8,sr_func(g0)          # save sr_func
                                        #  + sr_plen
                                        #  + sr_oml_sn
                                        #  + sr_ilt
        ldob    ld_basecl(r12),r8       # r8 = XIOtech Controller cluster #
        ldos    ld_basevd(r12),r11      # r11 = VDisk # to add path to
        stob    r8,sr_oml_cl(g0)        # save cluster # in SRP
        stos    r11,sr_oml_vid(g0)      # save VDisk # in SRP
        ld      dtmt_lldid(r13),r8      # r8 = link-level driver ID
        st      r8,sr_oml_lldid(g0)     #   and save in srp (sr_oml_lldid)
        st      g3,sr_oml_dlmid(g0)     # save the DLM session ID (sr_oml_dlmid)
        movq    0,r8                    # Clear the sr descriptor
        lda     sr_oml_size(g0),r10     # r10 = MLOSPB (global)
        stq     r8,srpbsiz+sr_source(g0) # save sr_source (cleared)
                                        #  + sr_destination (cleared)
                                        #  + sr_oml_ospb
                                        #  + sr_rsvd (cleared)
        lda     sr_oml_size(g0),r10     # get a local address of the MLOSPB
        st      r5,mlos_lldid(r10)      # clear link-level driver session ID field in MLOSPB
        ldl     ld_devcap(r12),r4       # r4-r5 = device capacity from LDD
#
# To be compatible with Magnitude, the Device capacity will be limited to four bytes by the CCB
#
# NOTE: mlos_dsize is not used in lld$OML -- so this 32 bit value doesn't matter.
c if (r5 != 0) {
c       r4 = 0xffffffffUL;              # Flag that the vdisk is too big for this DLM call.
c       r5 = 0;
c }
        st      r4,mlos_dsize(r10)      # save device capacity in MLOSPB
#
        ld      dtmt_lldmt(r13),r11     # r11 = assoc. LLDMT (ILT/VRP)
        st      r4,mlos_dsize(r10)      # save device capacity in MLOSPB
        ldob    dlmi_path-ILTBIAS(r11),r10 # r10 = path #
        st      g0,dlmi_srp(g1)         # save SRP address in ILT
        st      r11,il_w3(g1)           # save assoc. ILT/VRP address in ILT
        st      r10,dlmi_path(g1)       # save path # in ILT
        ld      lldmt_vrp(r11),r11      # get the vrp
        ld      vr_ilt(r11),r11         # get the ILT/VRP on the FE
        st      r11,sr_vrpilt(g0)       # save the ILT/VRP in the srp
        mov     g0,g2                   # g2 = SRP address
        lda     L$que,g0                # g0 = Link 960 Queue (wipes g0-g3,g14)
        movq    g0,r4                   # save g0-g3
        mov     g14,r11                 # save r11
        call    K$qwlink                # Queue request w/wait
        movq    r4,g0                   # restore g0-g3
        mov     r11,g14                 # restore g14
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        ldob    tpm_state(g3),r11       # r11 = TPMT state
        cmpobne tpm_st_popen,r11,.addmlpath_200 # Jif TPMT state changed
        ldob    vr_status(r3),r11       # r11 = SRP completion status (really in the VRP)
        cmpobne srok,r11,.addmlpath_200 # Jif error reported on SRP
        ldconst deok,r14                # r14 = good completion status
        lda     sr_oml_size(g2),g0      # g0 = MLOSPB address
        ld      mlos_lldid(g0),r4       # r4 = link-level driver session ID
        ldconst tpm_st_op,r5            # r5 = path state code (operational)
        st      r4,tpm_lldid(g3)        # save link-level driver session ID in TPMT
        stob    r5,tpm_state(g3)        # save path state code
        b       .addmlpath_400          # and we're done!
#
.addmlpath_200:
        ldob    tpm_state(g3),r11       # r11 = current TPMT state
        cmpobe  tpm_st_popen,r11,.addmlpath_230 # Jif pending open
        cmpobne tpm_st_abort,r11,.addmlpath_300 # Jif not abort state
.addmlpath_230:
        ldconst tpm_st_notop,r11        # r11 = new TPMT state
        stob    r11,tpm_state(g3)       # save new TPMT state
.addmlpath_300:
        movl    r12,g0                  # restore g0-g1
        call    DLM$dem_mlpath          # demolish this path
.addmlpath_400:
        ld      vr_blen(r3),g1          # g1 = size of VRP, SRP, and MLOSPB
c       s_Free_and_zero(r3, g1, __FILE__, __LINE__); # release memory for VRP, SRP, and MLOSPB
.addmlpath_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: DLM$dem_path
#
#  PURPOSE:
#       Demolishes a path to either a XIOtech Controller linked device or a
#       Foreign Target.
#
#  DESCRIPTION:
#       Determines what type of linked device path was specified and
#       branches off to the appropriate routine to demolish the path.
#
#  INPUT:
#       g0 = LDD address of linked device to demolish path to
#       g3 = assoc. TPMT to demolish
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
DLM$dem_path:
        ld      tpm_dtmt(g3),r4         # r4 = assoc. DTMT address
        ldob    dtmt_type(r4),r5        # r5 = target type from DTMT
        cmpobe  dtmt_ty_MAG,r5,DLM$dem_mlpath # Jif MAGNITUDE link type device
        b       DLM$dem_ftpath          # Foreign Target type device
#
#**********************************************************************
#
#  NAME: DLM$dem_mlpath
#
#  PURPOSE:
#       Demolishes a path to a XIOtech Controller linked device.
#
#  DESCRIPTION:
#       Sends a close XIOtech Controller link session to the link-level driver
#       if necessary to terminate it, unlinks TPMT from LDD and DTMT as needed,
#       returns any/all ILT ops. from queues and processes them as needed,
#       deallocates the TPMT.
#
#  INPUT:
#       g0 = LDD address of linked device to demolish path to
#       g3 = assoc. TPMT to demolish
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
DLM$dem_mlpath:
        movq    g0,r12                  # save g0-g3
                                        # r12 = LDD address
                                        # r15 = TPMT address being demolished
        ldob    tpm_state(g3),r4        # r4 = path state code
        cmpobne tpm_st_op,r4,.demmlpath_500 # Jif path not operational
#
# --- If operational, need to close session.
#
        ldconst tpm_st_pclose,r4        # r4 = new TPMT state
        stob    r4,tpm_state(g3)        # save new TPMT state
        ld      tpm_dtmt(g3),r4         # r4 = assoc. DTMT address
#
# --- Allocate, build and send Close Session to XIOtech Controller SRP
#
        ldconst vrpsiz,r8               # r8 = size of VRP header
        ldconst sr_cml_size,r9          # r9 = size of SRP
        addo    r8,r9,g0                # g0 = size of the VRP and SRP
c       g0 += 2;                        # Add 2 extra bytes so SRP isn't immediately after VRP
        mov     g0,r11                  # r11 = size of the VRP and SRP
c       g0 = s_MallocC(g0, __FILE__, __LINE__); # Get VRP/SRP
                                        # g0 = memory address
        mov     g0,r3                   # r3 = VRP address
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        lda     vr_sglhdr(g0),g0        # g0 = SRP
        st      r3,vrvrp(g1)            # save the VRP address in the ILT
        st      r3,ILTBIAS+vrvrp(g1)    # save the VRP in the next level also
        ldconst vrbefesrp,r8            # set the function as a BE to FE SRP
        stos    r8,vr_func(r3)
        # If the SRP address immediately follows the VRP, then the LL_LinuxLinkLayer.c
        # code will adjust the addresses so that the Target will see the SRP in it's
        # local memory, rather than the SRP in the Initiator memory. Skip 2 bytes
        # between the VRP and SRP to force the address to be in Initiator memory
        # (the equivalent of changing the address to a PCI address for Bigfoot).
c       g0 += 2;                        # Add 2 bytes for "PCI Address translation"
        mov     g0,r8                   # Translate to global address

        st      r8,vr_sglptr(r3)        # save the SRP address in the VRP
        st      r11,vr_blen(r3)         # save the size in the VRP
        st      r9,vr_sglsize(r3)       # save the size of the SRP
#
        ldconst srclml,r8               # r8 = SRP function code
        ldconst srerr,r11
        shlo    24,r11,r11
        or      r11,r8,r8
        mov     g1,r11                  # r11 = ILT/SRP address
        ld      dml_sn(r4),r10          # r10 = XIOtech Controller serial number
        stq     r8,sr_func(g0)          # save sr_func
                                        #  + sr_plen
                                        #  + sr_cml_sn
                                        #  + sr_ilt
        ldob    ld_basecl(r12),r8       # r8 = XIOtech Controller cluster #
        ldos    ld_basevd(r12),r11      # r11 = VDisk # to close path to
        stob    r8,sr_cml_cl(g0)        # save cluster # in SRP
        stos    r11,sr_cml_vid(g0)      # save VDisk # in SRP
        ld      tpm_lldid(r15),r8       # r8 = link-level driver ID
        st      r8,sr_cml_lldid(g0)     #  and save in SRP (sr_cml_lldid)
        st      r15,sr_cml_dlmid(g0)    # save DLM session ID (sr_cml_dlmid)
        ld      dtmt_lldmt(r4),r11      # r11 = assoc. LLDMT (ILT/VRP)
        ldob    dlmi_path-ILTBIAS(r11),r10 # r10 = path #
        st      g0,dlmi_srp(g1)         # save SRP address in ILT
        st      r11,il_w3(g1)           # save assoc. ILT/VRP address in ILT
        ld      lldmt_vrp(r11),r11      # get the vrp
        st      r10,dlmi_path(g1)       # save path # in ILT
        ld      vr_ilt(r11),r11         # get the ILT/VRP on the FE
        st      r11,sr_vrpilt(g0)       # save the ILT/VRP in the srp
        mov     g0,g2                   # g2 = SRP address
        mov     g1,r4                   # save g1
        mov     g14,r5                  # save g14
        lda     L$que,g0                # g0 = Link 960 Queue (wipes g0-g3,g14)
        call    K$qwlink                # Queue request w/wait
        mov     r4,g1                   # restore g1
        mov     r5,g14                  # restore g14
#
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        ld      vr_blen(r3),g1          # g1 = size of the VRP and SRP
c       s_Free_and_zero(r3, g1, __FILE__, __LINE__); # release memory for VRP & SRP
        movq    r12,g0                  # restore g0-g3
        ldob    tpm_state(g3),r4        # r4 = TPMT state
        ldconst tpm_st_notop,r5
        cmpobe  tpm_st_pclose,r4,.demmlpath_400 # Jif TPMT state pending close
        cmpobe  tpm_st_abort,r4,.demmlpath_400 # Jif TPMT state aborted
        b       .demmlpath_500          # all other TPMT states
#
.demmlpath_400:
        stob    r5,tpm_state(g3)        # save not operational TPMT state
.demmlpath_500:
        ldob    tpm_state(g3),r5        # r5 = current TPMT state
        ld      tpm_dtmt(g3),g1         # g1 = assoc. DTMT address
        cmpobe  tpm_st_dealloc,r5,.demmlpath_1000 # Jif TPMT is in deallocated state
        call    dlm$rem_tpmt            # remove TPMT from LDD & DTMT
        cmpobne tpm_st_notop,r5,.demmlpath_1000 # Jif TPMT not in not operational state
.ifdef M4_DEBUG_TPMT
c fprintf(stderr, "%s%s:%u put_tpmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g3);
.endif # M4_DEBUG_TPMT
c       put_tpmt(g3);                   # Deallocate TPMT
.demmlpath_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: DLM$chk_mlpath
#
#  PURPOSE:
#       Validates a path to the XIOtech Controller linked device.
#
#  DESCRIPTION:
#       Checks the TPMT associated with the specified LDD/DTMT to
#       make sure that all the components are still valid to use
#       the path.
#
#  INPUT:
#       g0 = LDD address of linked device to check path for
#       g1 = assoc. DTMT address of target to check path for
#       g3 = assoc. TPMT of path to check
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
DLM$chk_mlpath:
#*** FINISH
        ret
#
#**********************************************************************
#
#  NAME: dlm$create_oft
#
#  PURPOSE:
#       Allocate, build and send Open Session to Foreign Target SRP
#
#  INPUT:
#       g0 = Memory address of VRP/SRP and FTOSPB.
#       g1 = assoc. DTMT address of target to add path to
#       g2 = LDD address of linked device to add path to
#       g3 = TPMT to open session with
#       g4 = 32 or 64 bit SRP to generate
#       g5 = size of memory allocation.
#
#  OUTPUT:
#       g0 = SRP
#       g1 = FTOS
#
#  REGS DESTROYED:
#       g0, g1, g2
#
#**********************************************************************
#
dlm$create_oft:
c       r3 = g0;                        # Save VRP/SRP and FTOS
c       r13 = g1;                       # Save DTMT
c       r12 = g2;                       # Save LDD
c       r11 = g5;                       # Set up memory allocation size for VRP/SRC and FTOSPB.
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        lda     vr_sglhdr(g0),g0        # g0 = SRP
        st      r3,vrvrp(g1)            # save the VRP address in the ILT
        st      r3,ILTBIAS+vrvrp(g1)    # save the VRP in the next level also
        ldconst vrbefesrp,r8            # set the function as a BE to FE SRP
        stos    r8,vr_func(r3)
        # If the SRP address immediately follows the VRP, then the LL_LinuxLinkLayer.c
        # code will adjust the addresses so that the Target will see the SRP in it's
        # local memory, rather than the SRP in the Initiator memory. Skip 2 bytes
        # between the VRP and SRP to force the address to be in Initiator memory
        # (the equivalent of changing the address to a PCI address for Bigfoot).
c       g0 += 2;                        # Add 2 bytes for "PCI Address translation"
        mov     g0,r8                   # Translate to global address
        st      r8,vr_sglptr(r3)        # save the SRP address in the VRP
        st      r11,vr_blen(r3)         # save the size in the VRP
c       r8 = sr_oft_size + ftossiz_GT2TB;
        st      r8,vr_sglsize(r3)       # save the size of the SRP and FTOSPB
c   if (g4 == 32) {
        stob    sropft,sr_func(g0)      # Save SRP function code
c   } else {
        stob    sropft_GT2TB,sr_func(g0) # Save SRP function code
c   }
        stob    0,sr_flag(g0)           # Clear SRP flag
        stob    0,sr_rsvd1(g0)          # Clear reserved byte
        stob    srerr,sr_rsvd1(g0)      # Initialize SRP status to error
        st      r9,sr_plen(g0)          # Packet length of zero
        st      0,sr_count(g0)          # Descriptor count
        st      g1,sr_ilt(g0)           # ILT/SRP address

        ldos    ld_lun(r12),r8          # r8 = LUN # to open session to
        stos    r8,sr_oft_lun(g0)       # save LUN # in SRP
        ld      dtmt_lldid(r13),r8      # r8 = link-level driver ID and
        st      r8,sr_oft_lldid(g0)     #  save in the srp (sr_oft_lldid)
        st      g3,sr_oft_dlmid(g0)     # save the DLM session ID (sr_oft_dlmid)
        movq    0,r8                    # clear the not used fields
        lda     sr_oft_size(g0),r10     # r10 = FTOSPB (global and local)
        stq     r8,srpbsiz+sr_source(g0) # save sr_source (cleared)
                                        #  + sr_destination (cleared)
                                        #  + sr_oft_ospb
                                        #  + sr_rsvd (cleared)
        ldl     ld_vendid(r12),r4       # r4-r5 = vendor ID
c   if (g4 == 32) {
        stl     r4,ftos_venid(r10)      # save vendor ID in FTOSPB
c   } else {
        stl     r4,ftos_venid_GT2TB(r10) # save vendor ID in FTOSPB
c   }
        ldq     ld_prodid(r12),r4       # r4-r7 = product ID
c   if (g4 == 32) {
        stq     r4,ftos_prid(r10)       # save product ID in FTOSPB
c   } else {
        stq     r4,ftos_prid_GT2TB(r10) # save product ID in FTOSPB
c   }
        ldt     ld_serial(r12),r4       # r4-r6 = device serial #
        ldconst 0,r7                    # pad with zeros
        stq     r4,ftos_devsn(r10)      # save device serial # in FTOSPB
        st      r7,ftos_lldid(r10)      # clear link-level driver session ID field in FTOSPB
        ldl     ld_devcap(r12),r4       # r4-r5 = disk size in sectors
#
# To be compatible with Magnitude, the Device capacity will be limited to
#   four bytes by the CCB
#
c   if (g4 == 32) {
c       if (r5 != 0) {
c           r4 = 0xffffffffUL;          # Flag that the vdisk is too big for this DLM call.
c           r5 = 0;
c       }
        st      r4,ftos_dsize(r10)      # save disk size in FTOSPB
c   } else {
        stl     r4,ftos_dsize_GT2TB(r10) # save disk size in FTOSPB
c   }
#
        ld      dtmt_lldmt(r13),r11     # r11 = assoc. LLDMT (ILT/VRP)
        ldob    dlmi_path-ILTBIAS(r11),r5 # r5 = path #
        st      g0,dlmi_srp(g1)         # save SRP address in ILT
        st      r11,il_w3(g1)           # save assoc. ILT/VRP address in ILT
        ld      lldmt_vrp(r11),r11      # get the vrp
        ld      vr_ilt(r11),r11         # get the ILT/VRP on the FE
        st      r11,sr_vrpilt(g0)       # save the ILT/VRP in the srp
        st      r5,dlmi_path(g1)        # save path # in ILT
        mov     g0,g2                   # g2 = SRP address
        lda     L$que,g0                # g0 = Link 960 Queue (wipes g0-g3, g14)
        mov     g1,r4                   # save g1
        mov     g3,r5                   # save g3
        mov     g14,r9                  # save g14
        call    K$qwlink                # Queue request w/wait
        mov     r4,g1                   # restore g1
        mov     r5,g3                   # restore g3
        mov     r9,g14                  # restore g14
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
c       g0 = r3;                        # Return SRP address
c       g1 = r10;                       # Return FTOS address
        ret
#
#**********************************************************************
#
#  NAME: DLM$add_ftpath
#
#  PURPOSE:
#       Attempts to add a path to the Foreign Target linked device.
#
#  DESCRIPTION:
#       Allocates a TPMT to use for a new path to the specified
#       Foreign Target linked device and performs the processing to
#       establish a new path to the linked device.
#
#  INPUT:
#       g0 = LDD address of linked device to add path to
#       g1 = assoc. DTMT address of target to add path to
#
#  OUTPUT:
#       g2 = completion status
#            0          = successful
#            deunasspath = path blocked from use
#            deinopdev  = path failed to open successfully
#
#  REGS DESTROYED:
#       Reg. g2 destroyed.
#
#**********************************************************************
#
DLM$add_ftpath:
        movq    g0,r12                  # save g0-g3
                                        # r12 = LDD address
                                        # r13 = DTMT address
        ld      dtmt_lldmt(g1),r6       # r6 = assoc. LLDMT address
        ldob    ld_pmask(g0),r4         # r4 = path mask byte
        ldob    dlmi_path-ILTBIAS(r6),r7 # r7 = # of requested path
        bbs     r7,r4,.addftpath_20     # Jif path open for use
        ldconst deunasspath,r14         # return unassigned path error code
        b       .addftpath_1000
#
.addftpath_20:
        mov     deinopdev,r14           # r14 = error completion status
c       g3 = get_tpmt();                # Allocate a TPMT to open session with
.ifdef M4_DEBUG_TPMT
c fprintf(stderr, "%s%s:%u get_tpmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g3);
.endif # M4_DEBUG_TPMT
        st      g0,tpm_ldd(g3)          # save assoc. LDD address in TPMT
        st      g1,tpm_dtmt(g3)         # save assoc. DTMT address in TPMT
        ldob    ld_ppri(g0),r4          # r4 = path priority byte
        ldconst tpm_ty_pri,r5           # r5 = primary type code
        bbs     r7,r4,.addftpath_40     # Jif primary path type
        ldconst tpm_ty_sec,r5           # r5 = secondary type code
.addftpath_40:
        stob    r5,tpm_type(g3)         # save path type code in TPMT
        ldconst tpm_st_popen,r5         # r5 = TPMT state
        stob    r5,tpm_state(g3)        # set TPMT state
#
# --- Link TPMT onto TPMT list in DTMT & LDD
#
        call    dlm$add_tpmt            # add TPMT to LDD list and DTMT list
        ldconst vrpsiz,r8               # r8 = size of VRP header
        ldconst sr_oft_size,r9          # r9 = size of SRP
# Note: ftossiz_GT2TB is larger than ftossiz -- and vr_blen saves what was allocated for the Free.
        ldconst ftossiz_GT2TB,r10       # r10 = size of FTOSPB
        addo    r8,r9,r8                # r8 = size of VRP and SRP
        addo    r8,r10,g0               # g0 = size of VRP, SRP, & FTOSPB
c       g0 += 2;                        # Add 2 extra bytes so SRP isn't immediately after VRP
        mov     g0,r11                  # r11 = size of VRP, SRP, and FTOSPB
c       g0 = s_MallocC(g0, __FILE__, __LINE__); # Get VRP/SRP and FTOSPB
c       r9 = g4;                        # Save g4
c       r15 = g5;                       # Save g5
#
# --- Create 64 bit DG here
#       g0 = Memory address of VRP/SRP and FTOSPB.
#       g1 = assoc. DTMT address of target to add path to
#       g2 = LDD
#       g3 = TPMT to open session with
#       g4 = 32 or 64 bit SRP to generate
#       g5 = size of memory allocated for VRP/SRP and FTOSPB
c       r6 = g1;                        # Save DTMT in case needed for second call.
c       g5 = r11;                       # Set size of allocation.
c       r4 = r11;                       # Save for possible second call.
c       g4 = 64;                        # Try 64 bit version first.
c       g2 = r12;                       # LDD
        call    dlm$create_oft          # Create Open Foreign Target
#       g0 = SRP
#       g1 = FTOS
# --- Set up for following code to work as it used to.
c       g5 = r15;                       # restore g5
c       g4 = r9;                        # Restore g4
c       r3 = g0;                        # r3 = SRP
c       r10 = g1;                       # r10 = FTOS
c       r11 = 64;                       # Flag 64 bit SRP
#
# --- Process 64 bit response here
#
        ldob    tpm_state(g3),r9        # r9 = TPMT state
        cmpobne tpm_st_popen,r9,.addftpath_200 # Jif TPMT state changed
        ldob    vr_status(r3),r9        # r9 = SRP completion status (really the VRP status)
c   if (r9 != srok) {
# --- Create 32 bit DG here
#       g0 = Memory address of VRP/SRP and FTOSPB.
#       g1 = assoc. DTMT address of target to add path to
#       g2 = LDD
#       g3 = TPMT to open session with
#       g4 = 32 or 64 bit SRP to generate
#       g5 = size of memory allocated for VRP/SRP and FTOSPB (still set from above).
c       g5 = r4;                        # Saved above.
c       r9 = g4;                        # Save g4
c       g4 = 32;                        # Try 32 bit version next.
c       g2 = r12;                       # LDD
c       g1 = r6;                        # Set DTMT from saved above.
        call    dlm$create_oft          # Create Open Foreign Target
#       g0 = SRP
#       g1 = FTOS
# --- Set up for following code to work as it used to.
c       g5 = r15;                       # restore g5
c       g4 = r9;                        # Restore g4
c       r3 = g0;                        # r3 = SRP
c       r10 = g1;                       # r10 = FTOS
c       r11 = 32;                       # Flag 32 bit SRP
        ldob    tpm_state(g3),r9        # r9 = TPMT state
        cmpobne tpm_st_popen,r9,.addftpath_200 # Jif TPMT state changed
        ldob    vr_status(r3),r9        # r9 = SRP completion status (really the VRP status)
        cmpobne srok,r9,.addftpath_200  # Jif error reported on SRP
c   }
#
        ldconst deok,r14                # r14 = good completion status
        ld      ftos_lldid(r10),r4      # r4 = link-level driver session ID
        st      r4,tpm_lldid(g3)        # save link-level driver session ID in TPMT
        ldconst tpm_st_op,r4            # r4 = path state code (operational)
        stob    r4,tpm_state(g3)        # save path state code
c   if (r11 == 32) {                    # If 32 bit SRP
        ld      ftos_dsize(r10),r4      # r4,r5 = disk size in sectors
        ldconst 0,r5
c   } else {
        ldl     ftos_dsize_GT2TB(r10),r4 # r4,r5 = disk size in sectors
c   }
        stl     r4,ld_devcap(r12)       # save disk size in LDD
#
# --- Save the LDD to NVRAM
#
        ldos    K_ii+ii_status,r4       # Get current initialization status
        bbs     iimaster,r4,.addftpath_150  # Jif already master
        PushRegs                        # Save all G registers (stack relative)
        mov     r12,g0                  # g0 = LDD Address
        ldconst lddsiz,g1               # g1 = Size of the LDD
        ldconst ebiseLDD,g2             # g2 = Event type is "LDD"
        ldconst ebibtmaster,g3          # g3 = Broadcast event to the master
        mov     0,g4                    # g4 = Serial Number (not used)
        call    D$reportEvent           # Send the updated LDD to the master
        PopRegsVoid                     # Restore all G registers (stack relative)
.addftpath_150:
        b       .addftpath_400          # Release memory and we're done!
#
.addftpath_200:
        ldob    tpm_state(g3),r9        # r9 = current TPMT state
        cmpobe  tpm_st_popen,r9,.addftpath_230 # Jif pending open
        cmpobne tpm_st_abort,r9,.addftpath_300 # Jif not abort state
.addftpath_230:
        ldconst tpm_st_notop,r9         # r9 = new TPMT state
        stob    r9,tpm_state(g3)        # save new TPMT state
.addftpath_300:
        movl    r12,g0                  # restore g0-g1
        call    DLM$dem_ftpath          # demolish this path
.addftpath_400:
        ld      vr_blen(r3),g1          # g1 = size of VRP, SRP, and FTOSPB
c       s_Free_and_zero(r3, g1, __FILE__, __LINE__); # release memory for VRP, SRP, & FTOSPB
.addftpath_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: DLM$dem_ftpath
#
#  PURPOSE:
#       Demolishes a path to a Foreign Target linked device.
#
#  DESCRIPTION:
#       Sends a close Foreign Target session to the link-level driver if
#       necessary to terminate it, unlinks TPMT from LDD and DTMT as needed,
#       returns any/all ILT ops. from queues and processes them as needed,
#       deallocates the TPMT.
#
#  INPUT:
#       g0 = LDD address of linked device to demolish path to
#       g3 = assoc. TPMT to demolish
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
DLM$dem_ftpath:
        movq    g0,r12                  # save g0-g3
                                        # r12 = LDD address
                                        # r15 = TPMT address being demolished
        ldob    tpm_state(g3),r4        # r4 = path state code
        cmpobne tpm_st_op,r4,.demftpath_500 # Jif path not operational
#
# --- If operational, need to close session.
#
        ldconst tpm_st_pclose,r4        # r4 = new TPMT state
        stob    r4,tpm_state(g3)        # save new TPMT state
        ld      tpm_dtmt(g3),r4         # r4 = assoc. DTMT address
#
# --- Allocate, build and send Close Session to Foreign Target SRP
#
        ldconst vrpsiz,r8               # r8 = size of VRP header
        ldconst sr_cft_size,r9          # r9 = size of SRP
        addo    r8,r9,g0                # g0 = size of the VRP and SRP
c       g0 += 2;                        # Add 2 extra bytes so SRP isn't immediately after VRP
        mov     g0,r11                  # r11 = size of the VRP and SRP
c       g0 = s_MallocC(g0, __FILE__, __LINE__); # Get VRP
                                        # g0 = memory address
        mov     g0,r3                   # r3 = VRP (save for later use)
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        lda     vr_sglhdr(g0),g0        # g0 = SRP
        st      r3,vrvrp(g1)            # save the VRP address in the ILT
        st      r3,ILTBIAS+vrvrp(g1)    # save the VRP in the next level also
        ldconst vrbefesrp,r8            # set the function as a BE to FE SRP
        stos    r8,vr_func(r3)
        # If the SRP address immediately follows the VRP, then the LL_LinuxLinkLayer.c
        # code will adjust the addresses so that the Target will see the SRP in it's
        # local memory, rather than the SRP in the Initiator memory. Skip 2 bytes
        # between the VRP and SRP to force the address to be in Initiator memory
        # (the equivalent of changing the address to a PCI address for Bigfoot).
c       g0 += 2;                        # Add 2 bytes for "PCI Address translation"
        mov     g0,r8                   # Translate to global address
        st      r8,vr_sglptr(r3)        # save the SRP address in the VRP
        st      r11,vr_blen(r3)         # save the size in the VRP and SRP
        st      r9,vr_sglsize(r3)       # save the size of the SRP
#
        ldconst srclft,r8               # r8 = SRP function code
        ldconst srerr,r11
        shlo    24,r11,r11
        or      r11,r8,r8
        mov     g1,r11                  # r11 = ILT/SRP address
        mov     0,r10
        stq     r8,sr_func(g0)          # save sr_func
                                        #  + sr_plen
                                        #  + sr_count (0)
                                        #  + sr_ilt
        ldos    ld_lun(r12),r8          # r8 = LUN # to close session to
        stos    r8,sr_cft_lun(g0)       # save LUN # in SRP
        ld      tpm_lldid(r15),r8       # r8 = link-level driver ID
        st      r8,sr_cft_lldid(g0)     # and save in the srp (sr_cft_lldid)
        st      r15,sr_cft_dlmid(g0)    # save the DLM Session ID in the SRP
        ld      dtmt_lldmt(r4),r11      # r11 = assoc. LLDMT (ILT/VRP)
        ldob    dlmi_path-ILTBIAS(r11),r10 # r10 = path #
        st      g0,dlmi_srp(g1)         # save SRP address in ILT
        st      r11,il_w3(g1)           # save assoc. ILT/VRP address in ILT
        ld      lldmt_vrp(r11),r11      # get the vrp
        ld      vr_ilt(r11),r11         # get the ILT/VRP on the FE
        st      r11,sr_vrpilt(g0)       # save the ILT/VRP in the srp
        st      r10,dlmi_path(g1)       # save path # in ILT
        mov     g0,g2                   # g2 = SRP address
        lda     L$que,g0                # g0 = Link 960 Queue (wipes g0-g3, g14)
        mov     g1,r4                   # save g1
        mov     g14,r5                  # save g14
        call    K$qwlink                # Queue request w/wait
        mov     r4,g1                   # restore g1
        mov     r5,g14                  # restore g14
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        ld      vr_blen(r3),g1          # g1 = size of the VRP and SRP
c       s_Free_and_zero(r3, g1, __FILE__, __LINE__); # release memory for VRP and SRP
        movq    r12,g0                  # restore g0-g3
        ldob    tpm_state(g3),r4        # r4 = TPMT state
        ldconst tpm_st_notop,r5
        cmpobe  tpm_st_pclose,r4,.demftpath_400 # Jif TPMT state pending close
        cmpobe  tpm_st_abort,r4,.demftpath_400 # Jif TPMT state aborted
        b       .demftpath_500          # all other TPMT states
#
.demftpath_400:
        stob    r5,tpm_state(g3)        # save not operational TPMT state
.demftpath_500:
        ldob    tpm_state(g3),r5        # r5 = current TPMT state
        ld      tpm_dtmt(g3),g1         # g1 = assoc. DTMT address
        cmpobe  tpm_st_dealloc,r5,.demftpath_1000 # Jif TPMT is in deallocated state
        call    dlm$rem_tpmt            # remove TPMT from LDD & DTMT
        cmpobne tpm_st_notop,r5,.demftpath_1000 # Jif TPMT not in not operational state
.ifdef M4_DEBUG_TPMT
c fprintf(stderr, "%s%s:%u put_tpmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g3);
.endif # M4_DEBUG_TPMT
c       put_tpmt(g3);                   # Deallocate TPMT
.demftpath_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: DLM$VLraid
#
#  PURPOSE:
#       To provide a means of performing a VLink device I/O operation
#       requested via an RRP.
#
#  DESCRIPTION:
#       This procedure initiates the physical I/O for a RAID device that
#       is configured by the system as a "VLink device".  Completion
#       routines are used to handle the completion of the physical I/O
#       and to complete the RRP request back to the originator (virtual
#       layer).
#
#  INPUT:
#       g6/g7 = SDA
#       g8    = RRP function code
#       g9    = SGL
#       g10   = sector count
#       g11   = RDD (saved in dlmio_p_rdd field prior to calling)
#       g13   = RRP
#       g14   = primary ILT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#**********************************************************************
#
# void DLM$VLraid(g6/g7=UINT64, g8=UINT32, g9=UINT32, g10=UINT32, g11=UINT32, g13=UINT32, g14=UINT32);
DLM$VLraid:
        mov     g14,g1                  # g1 = primary ILT
        lda     dlm$VLraid_cr,r4        # r4 = ILT completion routine
        st      r4,dlmio_p_cr(g1)       # save completion routine in ILT
#
.if     DEBUG_FLIGHTREC_DLM
        ldconst frt_dlm_rrp,r3          # DLM - vrp received
        shlo    16,g8,r4                 # shift the Function code
        or      r4,r3,r3                # r3 = Function, Flight Recorder ID
        st      r3,fr_parm0             # Function, Flight Recorder ID
        st      g14,fr_parm1            # ILT
        st      g13,fr_parm2            # RRP
        st      g11,fr_parm3            # RDD
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_DLM
#
# --- determine if there are any paths to the device
#
        ld      rd_psd(g11),r4          # r4 = PSD address
        ldob    ps_pid(r4),r5           # r5 = LDD ordinal
        ld      DLM_lddindx[r5*4],r6    # r6 = assoc. LDD address
        cmpobe.f 0,r6,.VLraid_900       # Jif no LDD defined for RDD
#
        st      r6,dlmio_p_ldd(g1)      # save assoc. LDD address in ILT
#
# --- there is at least 1 path to device
#
# --- Convert any Local BE Buffer Addresses to PCI BE Buffer Addresses if there
#       is a valid SGL.
#
        cmpobe  0,g9,.VLraid_5          # Jif there is no SGL
# -- The address 0xfeedf00d setting in cachefe.as, checking in raid5.as and dlmbe.as.
        ldconst 0xfeedf00d,r3           # r3 = invalid SGL pointer
        cmpobne r3,g9,.VLraid_5         # Jif if it is not THE invalid SGL ptr
        ldconst 0,g9                    # Clear the invalid SGL pointer
#
.VLraid_5:
        ld      rd_vlop(g11),r4         # r4 = assoc. VLOP if one defined
        cmpobe  0,r4,.VLraid_70         # Jif no VLOP defined
        ldconst ecinop,r4               # dflt err "Inoperative virtual device"
        b       .VLraid_910
#
# --- No VLOP defined.
#
.VLraid_70:
        st      g10,dlmio_p_len(g1)     # save I/O length in ILT
.if     DEBUG_FLIGHTREC_DLM
        ldconst frt_dlm_novlop,r3       # Type
        st      r3,fr_parm0             # Virtual - v$exec
        st      r15,fr_parm1            # ILT
        st      g6,fr_parm2             # SDA LS
        st      g7,fr_parm3             # SDA MS
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_DLM
        stl     g6,dlmio_p_sda(g1)      # save SDA in ILT
        stl     g6,dlmio_p_psda(g1)     # save precedence SDA in ILT
        cmpo    0,1                     # Clear the Carry Bit
        addc    g6,g10,r6               # Get the End Disk Address + 1
        addc    0,g7,r7
        stl     r6,dlmio_p_peda(g1)     # save the precedence EDA in ILT
        st      g9,dlmio_p_sgl(g1)      # save SGL address in ILT
        lda     VLraid_rrpfc,r4         # r4 = raid f.c. xlate table
        ldconst 0,r5                    # r5 = table index
        st      r5,dlmio_p_ilt(g1)      # clear assoc. ILT address field
        st      r5,dlmio_p_status(g1)   # clear status fields
        st      r5,dlmio_p_extstat(g1)  # clear extended status field
        st      r5,dlmio_p_deplist(g1)  # clear dependent list field
.VLraid_100:
        ldob    (r4),r7                 # r7 = raid f.c. from table
        cmpobe  g8,r7,.VLraid_150       # Jif match found
        addo    2,r4,r4                 # Inc. table address
        cmpobne 0,r7,.VLraid_100        # Jif end of table not found
#
# --- RRP function code not found in table
#
        ldconst ecinvfunc,r4            # r4 = error status
        b       .VLraid_910
#
# --- RRP function code found in table
#
.VLraid_150:
        ldob    1(r4),r7                # r7 = LRP function code
        ldconst diop_rtycnt,r8          # r8 = retry count
        st      r7,dlmio_p_fc(g1)       # save LRP function code in ILT and clear state, flag bytes
        ldconst diop_timeout,r9         # r9 = I/O timeout value
        stob    r8,dlmio_p_rtycnt(g1)   # save retry count in ILT
        stos    r9,dlmio_p_timeout(g1)  # save I/O timeout value
        lda     dlm_lrpio_qu,r11        # r11 = QU for LRP I/O requests
        b       K$cque                  # and queue request to task

.VLraid_900:
        ldconst ecnonxdev,r4            # r4 = error status
.VLraid_910:
        ld      dlmio_p_cr(g1),r5       # r5 = ILT completion handler routine
        stob    r4,dlmio_p_status(g1)   # save error status in ILT
        callx   (r5)                    # call completion handler routine
        ret

#**********************************************************************
#
#  NAME: VLraid_rrpfc
#
#  PURPOSE:
#       Maps supported RRP function codes to LRP function codes.
#
#**********************************************************************
        .data
VLraid_rrpfc:
        .byte   rroutput,  sr_lrpfc_write      # write
        .byte   rrinput,   sr_lrpfc_read       # read
        .byte   rroutputv, sr_lrpfc_writevfy   # write and verify
        .byte   rrverifyc, sr_lrpfc_vfychkword # verify checkword
        .byte   rrverify,  sr_lrpfc_vfydata    # verify data
        .byte   0, 0                           # end of table

        .text
#
#**********************************************************************
#
#  NAME: dlm$VLraid_cr
#
#  PURPOSE:
#       Provide the LRP I/O request completion handling logic for
#       DLM$VLraid routine requests.
#
#  DESCRIPTION:
#       Moves the completion status into the associated RRP and returns
#       the ILT/RRP back to the original requestor.
#
#  INPUT:
#       g1 = ILT at dlmio_p nest level
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
dlm$VLraid_cr:
        movl    g0,r12                  # save g0-g1
        ld      dlmio_p_rdd(g1),r3      # r3 = assoc. RDD address
        ld      il_w0-ILTBIAS(g1),r4    # r4 = assoc. RRP address
        ldob    dlmio_p_status(g1),r5   # r5 = LRP I/O completion status
        stob    r5,rr_status(r4)        # save completion status in RRP
        balx    r$comp,r6               # complete RRP request
        movl    r12,g0                  # restore g0-g1
        ret
#
#**********************************************************************
#
#  NAME: DLM$proc_ldd
#
#  PURPOSE:
#       To initiate a means of performing a scan process on the
#       specified LDD.
#
#  DESCRIPTION:
#       This routine determines what to do to initiate a scan process
#       for the specified LDD. If an LDD scan task is not active, it
#       activates one and sets it up to perform. If an LDD scan is
#       already active, it sets the flag to restart the scan process
#       at the next appropriate opportunity.
#
#  INPUT:
#       g0 = LDD address to perform scan process on
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
DLM$proc_ldd:
        ldconst TRUE,r6                 # r6 = LDD scan process event flag #1
        ld      ld_pcb(g0),r4           # r4 = active LDD scan process PCB
                                        # r4 = 0 if process not active
        cmpobne 0,r4,.procldd_200       # Jif scan process already active
        movq    g0,r12                  # save g0-g3
        ldos    ld_ord(g0),r4           # r4 = LDD ordinal
        lda     DLM_lddindx[r4*4],g3    # g3 = DLM_lddindx pointer for specified LDD
        mov     g0,g2                   # g2 = LDD address
c       g0 = -1;                        # flag task in process of being created.
        st      g0,ld_pcb(r12)
        lda     dlm$lddx,g0             # g0 = LDD scan process address
        ldconst DLMLDDXPRI,g1           # g1 = LDD scan process priority
c       CT_fork_tmp = (ulong)"dlm$lddx";
        call    K$fork                  # fork a LDD scan process for the specified LDD
                                        # g0 = LDD scan process PCB address
        st      g0,ld_pcb(r12)          # save LDD scan process PCB address in assoc. LDD
        movq    r12,g0                  # restore g0-g3
.procldd_200:
        stob    r6,ld_flag1(g0)         # set process event flag #1
        ret
#
#******************************************************************************
#
#  NAME:  DLM$VDnamechg
#
#  PURPOSE:
#       Processes a VDisk/VLink name change event.
#
#  DESCRIPTION:
#       Determines if other XIOtech Controller nodes need to be notified
#       of a VDisk/VLink name change event and if so notifies
#       them as appropriately.
#
#  INPUT:
#       g1 = VDD address of VDisk/VLink changing names
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
DLM$VDnamechg:
        movq    g0,r12                  # save g0-g3
                                        # r13 = VDD address of name change
#
# --- Check if VLink named and if so notify source MAG.
#
        ldos    vd_attr(g1),r4          # r4 = attribute byte
        bbc     vdbvlink,r4,.namechg_100# Jif not a VLink
        ld      vd_rdd(g1),r5           # r5 = assoc. RDD address
        ld      rd_psd(r5),r6           # r6 = assoc. PSD address
        ldos    ps_pid(r6),r7           # r7 = assoc. LDD index
        ld      DLM_lddindx[r7*4],r7    # r7 = assoc. LDD address
        ld      ld_basesn(r7),g2        # g2 = MAG serial # to notify of name change
        call    DLM$send_namechg        # send VDisk name changed notification
        b       .namechg_1000           # and we're done!
#
# --- Check if any VLinks have been established to the VLink/VDisk
#       just named and if so notify them of the change.
#
.namechg_100:
        ld      vd_vlinks(g1),r10       # r10 = first VLAR on list
.namechg_150:
        cmpobe  0,r10,.namechg_1000     # Jif no more VLARs to process
        ld      vlar_srcsn(r10),g2      # g2 = MAG serial # linked to this VDisk/VLink
        call    DLM$send_namechg        # send VDisk name changed notification
        ld      vlar_link(r10),r10      # r10 = next VLAR on list
        b       .namechg_150
#
.namechg_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME:  DLM$send_namechg
#
#  PURPOSE:
#       Sends a VDisk/VLink name changed notification message
#       to the specified XIOtech Controller node.
#
#  DESCRIPTION:
#       Allocates the resources and builds up a VDisk/VLink name
#       changed notification message and sends it to the specified
#       XIOtech Controller node.
#
#  INPUT:
#       g1 = VDD address of VDisk/VLink changing names
#       g2 = XIOtech Controller node serial # to send notification to
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
DLM$send_namechg:
        movl    g0,r12                  # save g0-g1
                                        # r13 = VDD address of name change
        movl    g10,r14                 # save g10-g11
        ldconst DLM0_rq_vdchg_size,g10  # g10 = request message size
        ldconst 0,g11                   # g11 = response message size
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        mov     g1,r11                  # r11 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     dlm_vdchg_hdr,r4        # r4-r7 = bytes 0-15 of req. header
        lda     dgrq_size(r10),r10      # r10 = pointer to remaining req.  message
        bswap   g2,r7                   # r7 = dest. serial # in big-endian format
        stq     r4,(r8)                 # save bytes 0-15 of req. header
        ldq     dlm_vdchg_hdr+16,r4     # r4-r7 = bytes 16-31 of req. header
        bswap   r5,r5                   # swap remaining length value
        stq     r4,16(r8)               # save bytes 16-31 of req. header
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my VCG serial #
        ldl     vd_name(r13),r6         # Get the vdisk name (8 chars of 16)
        bswap   r3,r3
        stl     r6,DLM0_rq_vdchg_srcname(r10) # save source device name
        st      r3,DLM0_rq_vdchg_srcsn(r10) # save source node serial #
        ldos    vd_attr(r13),g10        # g10 = attribute byte
        bbc     vdbvlink,g10,.sendnmchg_300 # Jif device is not a VLink
#
# --- Device is a VLink. Pack the identification we have defined locally.
#
        ldob    vd_vid(r13),r4          # r4 = LSB VDisk #
        ldob    vd_vid+1(r13),r5        # r5 = MSB VDisk #
        st      r5,DLM0_rq_vdchg_srccl(r10) # save MSB VDisk # and clear reserved byte #0, #1
        stob    r4,DLM0_rq_vdchg_srcvd(r10) # save LSB VDisk #
        ld      vd_rdd(r13),r5          # r5 = assoc. RDD address
        ld      rd_psd(r5),r6           # r6 = assoc. PSD address
        ldos    ps_pid(r6),r4           # r4 = assoc. LDD index
        ld      DLM_lddindx[r4*4],r9    # r9 = assoc. LDD address
        ld      ld_basesn(r9),g10       # g10 = base MAG serial #
        ldob    ld_basecl(r9),r5        # r5 = base MAG cluster #
        ldos    ld_basevd(r9),r4        # r4 = base MAG VDisk #
        bswap   g10,g10
        st      r5,DLM0_rq_vdchg_basecl(r10) # save base MAG cluster # and clear reserved byte #2
        st      g10,DLM0_rq_vdchg_basesn(r10) # save base MAG serial #
        stob    r4,DLM0_rq_vdchg_basevd(r10) # save base MAG VDisk #
        ldl     ld_basename(r9),r6      # Get the device name
        stl     r6,DLM0_rq_vdchg_basename(r10) # save base MAG device name
        ldconst 0x01,r4                 # r4 = name change type code
        stob    r4,DLM0_rq_vdchg_type(r10) # save name change type code
        b       .sendnmchg_500
#
# --- Device is not a VLink. Find a VLAR record associated with the
#       virtual device and send the identity we have for the specified
#       server based on the VLAR information.
#
.sendnmchg_300:
        st      r3,DLM0_rq_vdchg_basesn(r10) # save base MAG serial #
        stl     r6,DLM0_rq_vdchg_basename(r10) # save base device name
        ld      vd_vlinks(r13),r4       # r4 = first VLAR assoc. with VDD
.sendnmchg_350:
        cmpobne 0,r4,.sendnmchg_400     # Jif VLAR assoc. with VDD
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram ILT
        b       .sendnmchg_1000         # and we're out of here!
#
.sendnmchg_400:
        ld      vlar_srcsn(r4),r5       # r5 = source node serial #
        cmpobe  r5,g2,.sendnmchg_450    # Jif matching VLAR found
        ld      vlar_link(r4),r4        # r4 = next VLAR assoc. with VDD
        b       .sendnmchg_350          # and check next VLAR if present
#
.sendnmchg_450:
        ldob    vlar_repvd+1(r4),r5     # r5 = reported target #
        ldob    vlar_repvd(r4),r6       # r6 = reported LUN #
        st      r5,DLM0_rq_vdchg_srccl(r10) # save reported target # and clear reserved byte #0, #1
        stob    r6,DLM0_rq_vdchg_srcvd(r10) # save reported LUN #
        st      r5,DLM0_rq_vdchg_basecl(r10) # save reported target # and clear reserved byte #2
        stob    r6,DLM0_rq_vdchg_basevd(r10) # save reported LUN #
        ldconst 0x02,r4                 # r4 = name change type code
        stob    r4,DLM0_rq_vdchg_type(r10) # save name change type code
#
.sendnmchg_500:
        ldconst 16,g0                   # g0 = datagram retry count
        mov     r11,g1                  # g1 = ILT to send
        call    DLM$just_senddg         # just send it out
.sendnmchg_1000:
        movl    r14,g10                 # restore g10-g11
        movl    r12,g0                  # restore g0-g1
        ret
#
#******************************************************************************
#
#  NAME:  DLM$MAGnamechg
#
#  PURPOSE:
#       Processes a XIOtech Controller node name change event.
#
#  DESCRIPTION:
#       Sends a XIOtech Controller Node Name changed datagram to all
#       identified XIOtech Controller.
#
#  INPUT:
#       g0-g1 = new XIOtech Controller node name
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
DLM$MAGnamechg:
        movq    g0,r12                  # save g0-g3
                                        # r12-r13 = XIOtech Controller node name
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r10        # r10 = my VCG serial #
        ld      fi_cserial(r3),r11      # r11 = my controller serial #
#
# --- Scan the MLMT list and send a XIOtech Controller node name changed
#       datagram to all DLM0 servers on all nodes identified.
#
        ld      dlm_mlmthd,r4           # r4 = first MLMT on list
        cmpobe  0,r4,.MAGchg_1000       # Jif no MLMTs on list
.MAGchg_200:
        ld      mlmt_sn(r4),g2          # g2 = assoc. MAG serial #
        cmpobe  g2,r10,.MAGchg_300      # Jif my VCG serial #
        cmpobe  g2,r11,.MAGchg_300      # Jif my controller serial #
        call    DLM$send_magnnchg       # send MAGNITUDE node name changed notification
.MAGchg_300:
        ld      mlmt_link(r4),r4        # r4 = next MLMT on list
        cmpobne 0,r4,.MAGchg_200        # Jif more MLMTs to process
.MAGchg_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME:  DLM$send_magnnchg
#
#  PURPOSE:
#       Sends a MAGNITUDE node name changed notification message
#       to the specified MAGNITUDE node.
#
#  DESCRIPTION:
#       Allocates the resources and builds up a XIOtech Controller node name
#       changed notification message and sends it to the specified
#       XIOtech Controller node.
#
#  INPUT:
#       g0-g1 = new XIOtech Controller node name
#       g2 = XIOtech Controller node serial # to send notification to
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
DLM$send_magnnchg:
        movl    g0,r12                  # save g0-g1
                                        # r12-r13 = XIOtech Controller node name
        movl    g10,r14                 # save g10-g11
        ldconst DLM0_rq_nnchg_size,g10  # g10 = request message size
        ldconst 0,g11                   # g11 = response message size
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     dlm_nnchg_hdr,r4        # r4-r7 = bytes 0-15 of req. header
        lda     dgrq_size(r10),r10      # r10 = pointer to remaining req. message
        bswap   g2,r7                   # g11 = dest. serial # in big-endian format
        stq     r4,(r8)                 # save bytes 0-15 of req. header
        ldq     dlm_nnchg_hdr+16,r4     # r4-r7 = bytes 16-31 of req. header
        bswap   r5,r5                   # swap remaining length value
        stq     r4,16(r8)               # save bytes 16-31 of req. header
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my VCG serial #
        bswap   r3,r3
        st      r3,DLM0_rq_nnchg_sn(r10) # save MAG serial #
        stl     r12,DLM0_rq_nnchg_name(r10) # save MAG node name
        ldconst 16,g0                   # g0 = datagram retry count
        call    DLM$just_senddg         # just send it out
        movl    r14,g10                 # restore g10-g11
        movl    r12,g0                  # restore g0-g1
        ret
#
#******************************************************************************
#
#  NAME:  DLM$VLopen
#
#  PURPOSE:
#       Schedules the VLink open process as specified.
#
#  DESCRIPTION:
#       Allocates a VLOP, builds it up and registers it in the
#       associated RDD. It then schedules the VLink open process
#       for the specified VLink.
#
#  INPUT:
#       g3 = 16-bit VDisk #
#       g4 = RDD address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
DLM$VLopen:
        ld      rd_vlop(g4),r4          # r4 = current VLOP
        cmpobne 0,r4,.VLopen_1000       # Jif VLink open process started
        ld      rd_psd(g4),r6           # r6 = assoc. PSD address
        ldos    ps_pid(r6),r7           # r7 = assoc. LDD index
        ld      DLM_lddindx[r7*4],r7    # r7 = LDD address
        ldob    ld_class(r7),r8         # r8 = linked device class
        cmpobne ldmld,r8,.VLopen_1000   # Jif not MAGNITUDE link
        movl    g0,r12                  # save g0-g1
        mov     g5,r14                  # save g5
c       g5 = s_MallocC(vlopsiz, __FILE__, __LINE__); # Get VLOP
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        st      g5,rd_vlop(g4)          # save VLOP in RDD
        st      g4,vlop_rdd(g5)         # save RDD in VLOP
        stos    g3,vlop_r0(g5)          # vlop_r0/vlop_r1 = 16-bit VLink #
        ldconst 0x80,r4                 # r4 = VLink attributes
        stob    r4,vlop_r2(g5)          # vlop_r2 = VLink attributes
        ldconst vlop_st_op,r4           # r4 = process state
        lda     dlm$vlopen_etbl,r5      # r5 = VLink open process event handler table
        stob    r4,vlop_state(g5)       # save process state
        st      r5,vlop_ehand(g5)       # save event handler table in VLOP
#
# --- Send message to CCB about the start of the open process.  Using an
#       available memory segment but putting the data in the format found in
#       LOG_Defs.h.
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       r4 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mlevlinkopenbegin,r3    # VLink Open Process Starting
        stos    r3,mle_event(r4)        # Type of message
        ldos    rd_vid(g4),r3           # Get the RAID VDisk Number
        ldos    rd_rid(g4),r15          # Get the RAID ID
        stos    g3,log_dlm_ccb_vl_srcsn(r4) # Save the Input VDisk Number
        stos    r3,log_dlm_ccb_vl_srcsn+2(r4) # Save the RAID VDisk Number
        stos    r15,log_dlm_ccb_vl_srcsn+4(r4) # Save the RAID ID
        st      g5,log_dlm_ccb_vl_srcsn+8(r4) # Save the VLOP Address
c       r3 = 0;
        stos    r3,log_dlm_ccb_vl_srcsn+6(r4) # Clear this short
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], log_dlm_ccb_vl_size);
#
        lda     dlm$vlopen,g0           # g0 = VLink open process address
        ldconst DLMVLOPENPRI,g1         # g1 = VLink open process priority
c       CT_fork_tmp = (ulong)"dlm$vlopen";
        call    K$fork                  # fork a VLink open process for the specified VLink
                                        # g0 = VLink open process PCB address
        st      g0,vlop_pcb(g5)         # save VLink open process PCB address in assoc. VLOP
        movl    r12,g0                  # restore g0-g1
        mov     r14,g5                  # restore g5
.VLopen_1000:
        ret
#
#******************************************************************************
#
#  NAME:  DLM$chk_vlconflt
#
#  PURPOSE:
#       Checks the validity of a VLink access conflict response
#       to either an establish VLink or Swap VLink lock datagram
#       request.
#
#  DESCRIPTION:
#       This routine checks the owner of a VLink lock indicated
#       in the response message of an establish VLink or Swap VLink
#       lock DLM0 request. If the owner is this requesting XIOtech Controller,
#       checks the owning cluster #/VLink # to see if it still is
#       defined to the specified VDisk and if it no longer defined
#       to the VDisk will deallocate the specified datagram ILT and
#       build up a new Swap VLink Lock datagram request and return
#       it to the caller.
#
#  INPUT:
#       g0 = assoc. LDD address
#       g1 = Establish VLink/Swap VLink lock datagram ILT at nest level #2
#
#  OUTPUT:
#       g1 = datagram ILT at nest level #2 to use when reissuing the request.
#            Will return the same ILT as specified on INPUT if the VLink access conflict
#            is valid or cannot be overcome by the issuing XIOtech Controller node.
#       Note: If a new datagram ILT is returned to the caller, the datagram specified
#             by the caller and all it's resources have been deallocated by this routine!
#
#  REGS DESTROYED:
#       Reg. g1 can be destroyed.
#
#******************************************************************************
#
DLM$chk_vlconflt:
        movq    g0,r12                  # save g0-g3
                                        # r12 = assoc. LDD address
                                        # r13 = datagram ILT in error
        ld      dsc2_rshdr_ptr(g1),g2   # g2 = local response header address
        ldob    dgrs_status(g2),r11     # r11 = request completion status
        cmpobne dg_st_srvr,r11,.chkvlcon_1000 # Jif not dest. server level error
        ldob    dgrs_ec1(g2),r11        # r11 = error code byte #1
        cmpobne dgec1_srvr_vlconflt,r11,.chkvlcon_1000 # Jif not VLink access conflict type error
        ld      dgrs_resplen(g2),r11    # r11 = remaining resp. length
        bswap   r11,r11
c   if (r11 != DLM0_rs_estvl_size && r11 != DLM0_rs_estvl_size_GT2TB) {
        b       .chkvlcon_1000          # Jif not right amount of data in datagram response
c   }
        ld      dsc2_rsbuf(g1),g3       # g3 = response buffer address
        lda     dgrs_size(g3),g3        # g3 = response data address
        ld      DLM0_rs_estvl_basesn(g3),r6 # r6 = lock owner MAG serial #
        bswap   r6,r6                   #  in little-endian format
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my MAG serial #
        cmpobne r3,r6,.chkvlcon_1000    # Jif my MAG not the current owner
        ldob    DLM0_rs_estvl_basecl(g3),r6 # r6 = lock owner MSB VDisk #
        ldob    DLM0_rs_estvl_basevd(g3),r7 # r7 = lock owner LSB VDisk #
        shlo    8,r6,r6
        or      r6,r7,r7                # r7 = lock owner VDisk #
        ld      V_vddindx[r7*4],r8      # r8 = corresponding VDD
        cmpobe  0,r8,.chkvlcon_300      # Jif VLink not defined anymore
        ld      vd_rdd(r8),r5           # r5 = RDD address of first RAID segment
        cmpobe  0,r5,.chkvlcon_300      # Jif no RDD defined
        ldob    rd_type(r5),r6          # r6 = RAID type code of first RAID segment
        cmpobne rdlinkdev,r6,.chkvlcon_300 # Jif not linked device RAID type
        ld      rd_psd(r5),r6           # r6 = assoc. PSD address
        ldos    ps_pid(r6),r7           # r7 = assoc. LDD ordinal
        ldos    ld_ord(g0),r6           # r6 = assoc. specified LDD ordinal
        cmpobne r6,r7,.chkvlcon_300     # Jif not the same LDD combo
        b       .chkvlcon_1000          # VLink is still valid. Return with original ILT.
#
# --- VLink not valid anymore.
#
.chkvlcon_300:
        movq    g4,r8                   # save g4-g7
        ld      dsc2_rqbuf(g1),r3       # r3 = request buffer address
        lda     dgrq_size(r3),r3        # r3 = request data address
        mov     g1,r7                   # r7 = specified datagram ILT
        ldob    DLM0_rq_estvl_srccl(r3),g2 # g2 = source MSB VDisk #
        ldob    DLM0_rq_estvl_srcvd(r3),g3 # g3 = source LSB VDisk #
        shlo    8,g2,g2
        or      g2,g3,g3                # g3 = 16-bit VDisk #
        ldob    DLM0_rq_estvl_attr(r3),g4  # g4 = establish VLink attributes
        ld      dsc2_rsbuf(g1),r3       # r3 = response buffer address
        lda     dgrs_size(r3),r3        # r3 = response data address
        ld      DLM0_rs_estvl_basesn(r3),g7 # g7 = lock owner MAG serial #
        bswap   g7,g7                   # in little-endian format
        ldob    DLM0_rs_estvl_basecl(r3),g5 # g5 = lock owner MSB VLink #
        ldob    DLM0_rs_estvl_basevd(r3),g6 # g6 = lock owner LSB VLink #
        shlo    8,g5,g5
        or      g5,g6,g6                # g6 = lock owner 16-bit VLink #
c       g1 = 64;                        # Flag a 64 bit version of the call.
        call    dlm$pkswp_vl            # pack a Swap VLink Lock datagram
        mov     g1,r13                  # r13 = new datagram ILT to return to caller with
        lda     -dsc1_ulvl(r7),g1       # g1 = specified datagram ILT at nest level #1
        call    DLM$put_dg              # deallocate specified datagram resources
        movq    r8,g4                   # restore g4-g7
.chkvlcon_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME:  DLM$chk_vlock
#
#  PURPOSE:
#       Checks the validity of a VDisk/VLink lock.
#
#  DESCRIPTION:
#       Builds and sends a VDisk/VLink Query datagram to the
#       lock owner. If the response is successful, determines
#       from the information returned in the datagram response
#       whether the lock is still needed or not. Returns a
#       completion status to the caller indicating:
#       1: If the lock is still needed.
#       2: If the lock is no longer needed.
#       3: If the lock owner could not be contacted successfully.
#
#  INPUT:
#       g0 = VLAR address of lock owner to check
#       g1 = assoc. VDD address
#       g2 = cluster # of locked VDisk/VLink
#       g3 = VDisk/VLink #
#
#  OUTPUT:
#       g0 = completion status
#            00 = lock still needed
#            01 = lock not needed
#            02 = could not validate lock with lock owner
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#******************************************************************************
#
DLM$chk_vlock:
        movq    g0,r12                  # save g0-g3
                                        # r12 = VLAR address
                                        # r13 = VDD address
                                        # r14 = cluster #
                                        # r15 = VDisk #
        mov     g0,r10                  # r10 = VLAR address
        ld      vlar_srcsn(r12),g0      # g0 = source MAG serial #
        ldob    vlar_srccl(r12),g1      # g1 = source MAG cluster #
        ldob    vlar_srcvd(r12),g2      # g2 = source MAG VDisk #
        ldconst 0,r3                    # r3 = error retry count
        movt    g0,r4                   # Save arguments to dlm$pk_vquery
.chkvlk_105:
c       g3 = 64;                        # Do 64 bit call first.
.chkvlk_106:
        call    dlm$pk_vquery           # pack a VDisk/VLink query datagram
                                        # g1 = datagram ILT at nest level 2
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),g2   # g2 = local response header address
        ldob    dgrs_status(g2),r11     # r11 = request completion status
        cmpobe  dg_st_ok,r11,.chkvlk_150 # Jif no error reported on request
c   if (g3 == 64) {
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # return datagram resources
c       g3 = 32;
        movt    r4,g0                   # restore arguments for dlm$pk_vquery
        b   .chkvlk_106                 # Try 32 bit DLM request
c   }
#
.chkvlk_130:
        cmpobe  0,r3,.chkvlk_140        # Jif error retry count expired
        subo    1,r3,r3                 # dec. error retry count
        ldconst 1000,g0                 # delay for awhile
        call    K$twait
        b       .chkvlk_105             # and try it again
#
.chkvlk_140:
#JNS Constant
        mov     0x02,r12                # r12 = completion status to caller
        b       .chkvlk_900             # and return status to caller
#
.chkvlk_150:
        ldconst 0x01,r12                # r12 = completion status to caller
        ld      dsc2_rsbuf(g1),g3       # g3 = response message header
# NOTE: can tell 32/64 by dgrs_resplen!
        ld      dgrs_resplen(g2),r11    # r11 = remaining response length
        lda     dgrs_size(g3),g3        # g3 = response data address
        bswap   r11,r11
c   if (r11 != DLM0_rs_vqury_size && r11 != DLM0_rs_vqury_size_GT2TB) {
        b       .chkvlk_140             # Jif not right amount of data in datagram response
c   }
        ldob    DLM0_rs_vqury_type(g3),r6 # r6 = VDisk/VLink type code
        cmpobe  vqury_ty_undef,r6,.chkvlk_900 # Jif undefined
        cmpobe  vqury_ty_vdisk,r6,.chkvlk_900 # Jif VDisk
        cmpobe  vqury_ty_busy,r6,.chkvlk_130 # Jif busy
#
# --- We have a VLink defined.
#
c   if (r11 == DLM0_rs_vqury_size) {
        ld      DLM0_rs_vqury_magsn(g3),r4 # r4 = MAG serial #
c   } else {
        ld      DLM0_rs_vqury_magsn_GT2TB(g3),r4 # r4 = MAG serial #
c   }
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my MAG serial #
        bswap   r4,r4                   # in little-endian format
        cmpobne r3,r4,.chkvlk_900       # Jif different MAG serial #
c   if (r11 == DLM0_rs_vqury_size) {
        ldob    DLM0_rs_vqury_magcl(g3),r4 # r4 = MAG cluster #
c   } else {
        ldob    DLM0_rs_vqury_magcl_GT2TB(g3),r4 # r4 = MAG cluster #
c   }
        ldob    vlar_repvd+1(r10),r3    # r3 = expected target #
        cmpobne r4,r3,.chkvlk_900       # Jif different MAG cluster #
c   if (r11 == DLM0_rs_vqury_size) {
        ldob    DLM0_rs_vqury_magvd(g3),r4 # r4 = MAG VDisk #
c   } else {
        ldob    DLM0_rs_vqury_magvd_GT2TB(g3),r4 # r4 = MAG VDisk #
c   }
        ldob    vlar_repvd(r10),r3      # r3 = expected LUN #
        cmpobne r4,r3,.chkvlk_900       # Jif different MAG VDisk #
        ldconst 0x00,r12                # r12 = completion status to caller
.chkvlk_900:
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # return datagram resources
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME:  DLM$VLmove
#
#  PURPOSE:
#       Schedules the VLink move process as specified.
#
#  DESCRIPTION:
#       Checks if a VLink process is active for the specified
#       RDD and if so calls the MOVE event handler routine of
#       the active process. If no VLink process is active,
#       allocates a VLOP, builds it up and registers it in the
#       specified RDD. It then schedules the VLink move process
#       for the specified RDD.
#
#  INPUT:
#       g3 = new 16-bit VLink #
#       g4 = RDD address
#       g6 = current owner 16-bit VLink #
#       g7 = current owner MAG serial #
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
DLM$VLmove:
        mov     g9,r14                  # save g9
        ld      rd_psd(g4),r6           # r6 = assoc. PSD address
        ldos    ps_pid(r6),r7           # r7 = assoc. LDD index
        ld      K_ficb,r3               # r3 = FICB address
        ld      DLM_lddindx[r7*4],r7    # r7 = assoc. LDD address
        ld      fi_vcgid(r3),r3         # r3 = my VCG serial number
        ldob    ld_class(r7),r8         # r8 = linked device class
        st      r3,ld_owner(r7)         # Save the new owner
        cmpobne ldmld,r8,.VLmove_1000   # Jif not MAGNITUDE link
        ld      rd_vlop(g4),g9          # g9 = current VLOP
        cmpobe  0,g9,.VLmove_200        # Jif VLink process not started
#
# --- VLink process already active.
#
        ld      vlop_ehand(g9),r4       # r4 = VLink process event handler table
        ld      vlop_eh_move(r4),r5     # r5 = move event handler routine
        callx   (r5)                    # call move event handler routine
        b       .VLmove_1000            # and we're done!
#
# --- Schedule move VLink process for requestor.
#
.VLmove_200:
        movl    g0,r12                  # save g0-g1
c       g9 = s_MallocC(vlopsiz, __FILE__, __LINE__); # Get VLOP
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        st      g9,rd_vlop(g4)          # save VLOP in RDD
        st      g4,vlop_rdd(g9)         # save RDD in VLOP
        stos    g3,vlop_r0(g9)          # vlop_r0/vlop_r1 = new 16-bit VLink #
        ldconst 0x80,r4                 # r4 = VLink attributes
        stob    r4,vlop_r2(g9)          # vlop_r2 = VLink attributes
        stos    g6,vlop_r3(g9)          # vlop_r3/vlop_r4 = current owner 16-bit VLink #
        st      g7,vlop_g0(g9)          # vlop_g0 = current owner MAG serial #
        ldconst vlop_st_op,r4           # r4 = process state
        lda     dlm$vlmove_etbl,r5      # r5 = VLink move process event handler table
        stob    r4,vlop_state(g9)       # save process state
        st      r5,vlop_ehand(g9)       # save event handler table in VLOP
        lda     dlm$vlmove,g0           # g0 = VLink move process address
        ldconst DLMVLMOVEPRI,g1         # g1 = VLink move process priority
c       CT_fork_tmp = (ulong)"dlm$vlmove";
        call    K$fork                  # fork a VLink move process for the specified VLink
                                        # g0 = VLink move process PCB address
        st      g0,vlop_pcb(g9)         # save VLink move process PCB address in assoc. VLOP
        movl    r12,g0                  # restore g0-g1
.VLmove_1000:
        mov     r14,g9                  # restore g9
        ret
#
#******************************************************************************
#
#  NAME:  DLM$VLreopen
#
#  PURPOSE:
#       Schedules the VLink re-establish open process for the VLinks
#       associated with the specified LDD if no paths established.
#
#  DESCRIPTION:
#       Allocates a VLOP, builds it up and registers it in the
#       associated RDD. It then schedules the VLink open process
#       for all associated VLinks.
#
#  INPUT:
#       g0 = LDD address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
DLM$VLreopen:
        ld      ld_tpmthd(g0),r4        # r4 = first TPMT on list
        cmpobne 0,r4,.VLreopen_1000     # Jif a path is still established
        ldos    ld_ord(g0),r7           # r7 = assoc. LDD index
        ldob    ld_class(g0),r4         # r4 = linked device class
        cmpobne ldmld,r4,.VLreopen_1000 # Jif not a MAGNITUDE link type of linked device
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ldconst 0xFFFFFFFF,r9           # g2 = VID not Found value
        mov     r7,g0                   # g0 = LDD Ordinal
        call    dlm$ldd_vid             # Get the VID associated with this LDD
                                        # g0 = VID or 0xFFFFFFFF if not found
        cmpobe  r9,g0,.VLreopen_900     # Jif no VID was found
#
# --- Found a VLink associated with the specified LDD.
#       Schedule VLopen process.
#
        ld      V_vddindx[g0*4],r9      # r9 = VDD address
        mov     g0,g3                   # g3 = VLink #
        ld      vd_rdd(r9),g4           # g4 = RDD address
        call    DLM$VLopen              # schedule VLopen process
.VLreopen_900:
        movq    r12,g0                  # restore g0-g3
        mov     r11,g4                  # restore g4
.VLreopen_1000:
        ret
#******************************************************************************
#
#  NAME:  DLM$chg_size
#
#  PURPOSE:
#       Packs and sends a Change VDisk Size datagram
#       if appropriate and necessary for the specified
#       virtual device.
#
#  DESCRIPTION:
#       Packs and sends a Change VDisk Size datagram for the specified
#       virtual device if appropriate and necessary. This routine
#       performs a best effort attempt to change the VDisk size
#       on a remote XIOtech Controller.
#
#  INPUT:
#       g4 = VDD address of VLink to process
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
DLM$chg_size:
        ld      vd_rdd(g4),r5           # r5 = RDD address of the first RAID segment
        ldob    rd_type(r5),r6          # r6 = RAID type code of first RAID segment
        cmpobne rdlinkdev,r6,.chgsize_1000 # Jif not linked device RAID type
        ld      rd_psd(r5),r4           # r4 = first PSD address
        ldos    ps_pid(r4),r6           # r6 = assoc. LDD index
        ld      DLM_lddindx[r6*4],r6    # r6 = assoc. LDD address
        ldob    ld_class(r6),r7         # r7 = linked device class
        cmpobne ldmld,r7,.chgsize_1000  # Jif not MAGNITUDE link class device
        ldl     vd_devcap(g4),r8        # r8,r9 = VLink size
        ldl     ld_devcap(r6),r10       # r10,r11 = linked device size
        cmpobne r11,r9,.chgsize_200     # Jif VLink size NE linked device size
        cmpobe  r10,r8,.chgsize_1000    # Jif VLink size EQ linked device size
.chgsize_200:
        stl     r8,ld_devcap(r6)        # update size in LDD
        movq    g0,r12                  # save g0-g5
        movl    g4,r4                   # r4/r5 = g4/g5
        ldos    vd_vid(g4),g1           # g1 = 16-bit VDisk #
        movl    r8,g2                   # g2,g3 = new VDisk size
        mov     r6,g4                   # g4 = assoc. LDD address
c   if (r9 == 0) {
c       g5 = 32;
c   } else {
c       g5 = 64;                        # Must send 64 bit version of command.
c   }
        call    dlm$pkchg_siz           # pack a Change VDisk Size datagram
                                        # g1 = datagram ILT at nest level 2
        ldconst 4,g0                    # g0 = datagram error retry count
        call    DLM$just_senddg         # just send out datagram w/error retry
        call    D$p2updateconfig        # update NVRAM
        movq    r12,g0                  # restore g0-g5
        mov     r3,g4
        movl    r4,g4                   # g4/r5 = r4/g5
.chgsize_1000:
        ret
#
#******************************************************************************
#
#  NAME:  DLM$reg_size
#
#  PURPOSE:
#       Packs and sends a Change VDisk Size datagram
#       if appropriate and necessary for the specified
#       virtual device.
#
#  DESCRIPTION:
#       Packs and sends a Change VDisk Size datagram for the specified
#       virtual device if appropriate and necessary. This routine
#       waits for the response message from the destination MAGNITUDE,
#       checks for errors, retries certain errors, and returns the
#       completion status of this operation back to the caller.
#       Note: This routine will stall the calling routines process
#               while waiting for the response to the datagram message.
#
#  INPUT:
#       g4 = VDD address of VLink to process
#
#  OUTPUT:
#       g0 = TRUE if successful
#            FALSE if unsuccessful
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#******************************************************************************
#
DLM$reg_size:
        ldconst TRUE,g0                 # g0 = successful completion status
        ld      vd_rdd(g4),r5           # r5 = RDD address of the first RAID segment
        ldob    rd_type(r5),r6          # r6 = RAID type code of first RAID segment
        cmpobne rdlinkdev,r6,.regsize_1000 # Jif not linked device RAID type

        ld      rd_psd(r5),r4           # r4 = first PSD address
        ldos    ps_pid(r4),r6           # r6 = assoc. LDD index
        ld      DLM_lddindx[r6*4],r6    # r6 = assoc. LDD address
        ldob    ld_class(r6),r7         # r7 = linked device class
        cmpobne ldmld,r7,.regsize_1000  # Jif not MAGNITUDE link class device

        ldl     vd_devcap(g4),r4        # r4/r5 = VLink size
        ldconst 4,r3                    # r3 = error retry count
        movq    g0,r12                  # save g0-g3

.regsize_100:
        ldos    vd_vid(g4),g1           # g1 = VDisk #
        movl    g4,r10                  # r10/r11 = saved g4/g5
        movl    r4,g2                   # g2/g3 = new VDisk size
        mov     r6,g4                   # g4 = assoc. LDD address
c   if (g3 == 0) {
c       g5 = 32;
c   } else {
c       g5 = 64;                        # Must send 64 bit version of command.
c   }
        call    dlm$pkchg_siz           # pack a Change VDisk Size datagram
        movl    r10,g4                  # g4/r5 restored
                                        # g1 = datagram ILT at nest level 2
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        ldob    dgrs_status(r7),r11     # r11 = request completion status
        ldob    dgrs_ec1(r7),r10        # r10 = error code byte #1
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # return datagram resources
        cmpobe.t dg_st_ok,r11,.regsize_300 # Jif no error reported on request
        cmpobe.t dg_st_srvr,r11,.regsize_200 # Jif dest. server level error

.regsize_120:
        subo    1,r3,r3                 # dec. error retry count
        cmpobne.t 0,r3,.regsize_100     # Jif error retry count not expired

.regsize_150:
        ldconst FALSE,r12               # r12 = unsuccessful completion status to return to caller
        b       .regsize_900

.regsize_200:
        cmpobne.f dgec1_srvr_inuse,r10,.regsize_120 # Jif not device in use
                                        #  error code byte #1
        b       .regsize_150            # device in use error is cause for immediate termination
.regsize_300:
        stl     r4,ld_devcap(r6)        # update size in LDI
#@@@ FINISH @@@
#        st      r4,ld_tas(r6)
        call    D$p2update              # update NVRAM with changes

.regsize_900:
        movq    r12,g0                  # restore g0-g3

.regsize_1000:
        ret
#
#******************************************************************************
#
#  NAME:  DLM_send_async_nva
#
#  PURPOSE:
#       Packs and sends a NVA record datagram
#
#  DESCRIPTION:
#       This routine waits for the response message from the destination MAGNITUDE,
#       checks for errors, retries certain errors, and returns the
#       completion status of this operation back to the caller.
#       Note: This routine will stall the calling routines process
#               while waiting for the response to the datagram message.
#
#  INPUT: None
#
#  OUTPUT: g0  -- status (GOOD or ERROR)
#
#******************************************************************************
#
DLM_send_async_nva:
#
#     Check whether the communication to MP is available, return error if not.
#
        ld      K_ficb,r3
        ld      fi_mirrorpartner(r3),r4 # MP serial number
        ld      fi_cserial(r3),r6       # this controller's serial number
        cmpobe  r4,r6,.async_send_250   # return error
        mov     g0,r7                   # Update type
        ldconst 4,r5                    # retry count
.async_send_10:
.ifdef ASYNC_DLM_DEBUG
c fprintf (stderr,"%s%s:%u ASYNC: Send Datagram - %lx\n", FEBEMESSAGE, __FILE__, __LINE__,r5);
.endif  # ASYNC_DLM_DEBUG
        movq    g8,r12                  # save g8-g11
        ldconst DLM0_rq_apool_nv_size,g10  # g10 = request message size
        ldconst 0,g11                   # extended response message size = 0
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1

        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     dlm_async_nva_hdr,g8    # g8-g11 = bytes 0-15 of req. header
        lda     dgrq_size(r10),r10      # r10 = pointer to remaining req.  message

        bswap   r4,g11                  # g11 = dest. serial # in big-endian format
        stq     g8,(r8)                 # save bytes 0-15 of req. header

        st      g11,DLM0_rq_apool_nv_destsn(r10) # save dest. MAG serial #

        ldq     dlm_async_nva_hdr+16,g8 # g8-g11 = bytes 16-31 of req. header
        bswap   g9,g9                   # swap remaining length value
        stq     g8,16(r8)               # save bytes 16-31 of req. header


        ld      fi_cserial(r3),r4
        st      r4,DLM0_rq_apool_nv_srcsn(r10) #save the src serial #
        st      r7,DLM0_rq_apool_nv_update_type(r10)  # save update type, Normal update OR implicit update

#
# --- Copy the APOOL_NV Packet to the DLM packet
#
        lda     DLM0_rq_apool_nvh(r10),r6            # Point to the start of APOOL NV HEADER
        lda     gApoolnvImage,r4
c       memcpy((void*)r6, (void*)r4, (sizeof(APOOL_NV_IMAGE)));
        movq    r12,g8                  # restore g8-g11


        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        ldob    dgrs_status(r7),r11     # r11 = request completion status
        ldob    dgrs_ec1(r7),r10        # r10 = error code byte #1
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # return datagram resources
        cmpobe.t dg_st_ok,r11,.async_send_200 # Jif no error reported on request
        cmpobe.t dg_st_srvr,r11,.async_send_300 # Jif dest. server level error
        subo    1,r5,r5                 # dec. error retry count
        cmpobne 0,r5,.async_send_10
.ifdef ASYNC_DLM_DEBUG
c fprintf(stderr,"%s%s:%u ASYNC: Failed to send packet after 4 retries\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # ASYNC_DLM_DEBUG
        ldconst ERROR,g0
        b .async_send_400

.async_send_200:
.ifdef ASYNC_DLM_DEBUG
c fprintf (stderr,"%s%s:%u ASYNC: Datagram sent successfully\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # ASYNC_DLM_DEBUG
        ldconst GOOD,g0
        b .async_send_400

.async_send_250:
.ifdef ASYNC_DLM_DEBUG
c fprintf (stderr,"%s%s:%u ASYNC: Mirror partner not established, can not send pkt\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # ASYNC_DLM_DEBUG
        ldconst ERROR,g0
        b .async_send_400
#
.async_send_300:
.ifdef ASYNC_DLM_DEBUG
c fprintf (stderr,"%s%s:%u ASYNC: Datagram server level error\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # ASYNC_DLM_DEBUG
        ldconst ERROR,g0
.async_send_400:
        ret
#
#******************************************************************************
#
#  NAME:  DLM$app_vdsize
#
#  PURPOSE:
#       Gets the approval for a VDisk size change for the
#       VLink owner(s) of the specified VDD.
#
#  DESCRIPTION:
#       Sends VDisk size changed datagrams to all VLink owner(s)
#       of the specified VDD and if successful, commits the change
#       to all owners and returns approved completion status to
#       the caller.
#
#  INPUT:
#       g4-g5 = proposed new VDisk size
#       g8 = VDD address of VDisk being changed
#
#  OUTPUT:
#       g0 = approval completion status
#            00 = approved
#            01 = denied
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#******************************************************************************
#
DLM$app_vdsize:
        ldconst 0,g0                    # preload approved completion status
        movq    g0,r12                  # save g0-g3
.appvdsize_100:
        ld      vd_vlinks(g8),r11       # r11 = first VLAR on list
.appvdsize_200:
        cmpobe  0,r11,.appvdsize_1000   # Jif no more VLARS on list
        ld      vlar_srcsn(r11),g0      # g0 = Controller serial # to notify
        ldob    vlar_srccl(r11),g1      # g1 = Controller cluster # affected
        ldob    vlar_srcvd(r11),g2      # g2 = Controller VLink # affected
        ldos    vlar_repvd(r11),g3      # g3 = Reported target/LUN # affected
        call    dlm$pk_vdsize           # pack a VDisk size changed datagram
                                        # g1 = datagram ILT at nest level #2
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r6   # r6 = local response header address
        ldob    dgrs_status(r6),r7      # r7 = request completion status
        ldob    DLM0_rs_vdsize_stat(r6),r8 # r8 = change acceptance code
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level #1
        call    DLM$put_dg              # release datagram resources
        ld      vd_vlinks(g8),r9        # find VLAR used on list
        cmpobe  0,r9,.appvdsize_1000    # Jif no VLARS on list
.appvdsize_310:
        cmpobe  r9,r11,.appvdsize_320   # Jif VLAR still on list
        ld      vlar_link(r9),r9        # r9 = next VLAR on list
        cmpobne 0,r9,.appvdsize_310     # Jif more VLARS to check
        b       .appvdsize_100          # can't find VLAR used on list. start over again at start of list
#
.appvdsize_320:
        cmpobne dg_st_ok,r7,.appvdsize_400 # Jif error reported on request
        cmpobe  0,r8,.appvdsize_350     # Jif change was accepted
        cmpobne 3,r8,.appvdsize_400     # Jif reject was for other then no VLink assoc.
.appvdsize_350:
        ld      vlar_link(r11),r11      # r11 = next VLAR on list
        b       .appvdsize_200          # and check next VLAR if defined
#
.appvdsize_400:
        movl    g4,r4                   # save g4-g5
        ldconst 0x01,r12                # r12 = denied completion status
        mov     r11,r9                  # r9 = VLAR that denied approval
        ldl     vd_devcap(g8),g4        # g4-g5 = original size of VDisk
        ld      vd_vlinks(g8),r11       # r11 = first VLAR on list
.appvdsize_500:
        cmpobe  0,r11,.appvdsize_900    # Jif end of VLAR list
        cmpobe  r9,r11,.appvdsize_900   # Jif no more VLARS ahead of VLAR that failed
        ld      vlar_srcsn(r11),g0      # g0 = Controller serial # to notify
        ldob    vlar_srccl(r11),g1      # g1 = Controller cluster # affected
        ldob    vlar_srcvd(r11),g2      # g2 = Controller VLink # affected
        ldos    vlar_repvd(r11),g3      # g3 = Reported target/LUN # affected
        call    dlm$pk_vdsize           # pack a VDisk size changed datagram
                                        # g1 = datagram ILT at nest level #2
        ldconst 16,g0                   # g0 = datagram error retry count
        call    DLM$just_senddg         # just send out datagram w/error retry
        ld      vlar_link(r11),r11      # r11 = next VLAR on list
        b       .appvdsize_500          # and process next VLAR if defined
#
.appvdsize_900:
        movl    r4,g4                   # restore g4-g5
.appvdsize_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME:  DLM$clrldd_indx
#
#  PURPOSE:
#       Deallocates all LDD records in DLM_lddindx area and clears
#       all LDD fields.
#
#  DESCRIPTION:
#       Goes through the entire DLM_lddindx area and checks for LDD
#       records and all those found are deallocated and their LDD
#       records cleared.
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#******************************************************************************
#
    .globl DLM_ClearLDDIndx
#
DLM_ClearLDDIndx:
        mov     g0,r12                  # save g0
        ldconst MAXLDDS,r14             # r14 = max. # LDD supported
#
.clrldd_indx_50:
        subo    1,r14,r14               # dec. LDD index
        ld      DLM_lddindx[r14*4],g0   # g0 = LDD address from directory
        cmpobe  0,g0,.clrldd_indx_200   # Jif LDD not defined in this slot
#
        call    DLM_clr_ldd             # clear LDD to ready for demolition
        call    DLM_put_ldd             # demolish LDD/LDI
#
.clrldd_indx_200:
        cmpobne 0,r14,.clrldd_indx_50   # Jif more LDDs to process
#
        mov     r12,g0                  # restore g0
        ret
#
#******************************************************************************
#
#  NAME:  DLM$def_master
#
#  PURPOSE:
#       Packs and sends Group Master Controller Definition datagrams
#       to all XIOtech controllers that have been identified.
#
#  DESCRIPTION:
#       Goes through the MLMT list and packs/sends a Group Master Controller
#       Definition datagram to each one that is not a primary node.
#
#  INPUT:
#       g0 = group serial #
#       g2 = master controller serial #
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#******************************************************************************
#
DLM$def_master:
        ld      dlm_mlmthd,r4           # r4 = first MLMT on list
        movl    g0,r14                  # save g0-g1
                                        # r14 = group serial #
.defmaster_100:
        cmpobe.f  0,r4,.defmaster_1000  # Jif no more MLMTs to process
        ld      mlmt_dtmthd(r4),r5      # r5 = first DTMT assoc. with MLMT
        cmpobe.f 0,r5,.defmaster_300    # Jif no DTMTs assoc. with MLMT
        ld      dtmt_alias_dtmt(r5),r6  # r6 = assoc. alias DTMT
        cmpobne.f 0,r6,.defmaster_300   # Jif alias DTMT assoc. with DTMT indicating
                                        #  a primary DTMT (assoc. with the VCG)
        ld      mlmt_sn(r4),g1          # g1 = controller serial # to send group
                                        #  master controller definition datagram to
        mov     r14,g0                  # g0 = group serial #
        call    DLM$pk_master           # pack the datagram
                                        # g1 = datagram ILT at nest level 2
        ldconst 4,g0                    # g0 = datagram error retry count
        call    DLM$just_senddg         # just send out datagram w/error retry
.defmaster_300:
        ld      mlmt_link(r4),r4        # r4 = next MLMT on list
        b       .defmaster_100          # and go process the next MLMT on the list
#
.defmaster_1000:
        movl    r14,g0                  # restore g0-g1
        ret
#
#**********************************************************************
#
# ----------------- Data-link Manager Processes -----------------------
#
#**********************************************************************
#
#  NAME: dlm$vrphand
#
#  PURPOSE:
#       Data-link Manager VRP request function handler routine table.
#
#  DESCRIPTION:
#       This table contains the VRP request handler routines used by
#       the data-link manager.
#
#  INPUT:
#       g13 = VRP address to process
#       g14 = ILT associated with VRP at nest level 2
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
        .data
dlm$vrphand:
        .word   dlm$LOP                 # 0x40 vrlldop - link-level driver operational
        .word   dlm$MLE                 # 0x41 vrmlest - MAGNITUDE link established
        .word   dlm$MLT                 # 0x42 vrmlterm - MAGNITUDE link terminated
        .word   dlm$FTI                 # 0x43 vrftid - Foreign Target identified
        .word   dlm$FTT                 # 0x44 vrftterm - Foreign Target terminated
        .word   dlm$MRC                 # 0x45 vrmsgrcv - message received
        .word   dlm$LST                 # 0x46 vrsterm - link session terminated
#
        .text
#
#**********************************************************************
#
#  NAME: dlm$FTI
#
#  PURPOSE:
#       Processes a Foreign Target identified VRP request.
#
#  DESCRIPTION:
#       If the DLM session ID field is zero, scans the DTMT list
#       associated with the LLDMT to see if the target has already
#       been identified and if so uses the DTMT associated with the
#       previously identified target. If not, allocates a new DTMT
#       and saves the target information from the VRP in the DTMT.
#       It updates the storage controller list for the CCB if
#       necessary. It schedules an LDD scan to determine if the target
#       is associated with any VLinks that should open a path to this
#       target. It then returns completion status to the requesting path
#       with the results of the VRP processing.
#
#  INPUT:
#       g13 = VRP address to process
#       g14 = ILT associated with VRP at nest level 2
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
dlm$FTI:
        movq    g4,r12                  # save g4-g7
        ldob    -ILTBIAS+dlmi_path(g14),r4 # r4 = path #
        lda     dlm_lldmt_dir,r5        # r5 = base of LLDMT directory
        ld      (r5)[r4*4],g6           # g6 = LLDMT for this interface
        cmpobe  0,g6,.dlmFTI_900        # Jif no LLDMT defined for this interface
        ld      vr_fti_dlmid(g13),g4    # g4 = DLM session ID from VRP
        cmpobe  0,g4,.dlmFTI_100        # Jif DLM session ID not specified
        ld      dtmt_bnr(g4),r7         # r7 = banner value from DTMT
        ld      dtmt_banner,r8          # r8 = DTMT banner pattern
        cmpobne r7,r8,.dlmFTI_100       # Jif specified DTMT not in use
        ldob    dtmt_type(g4),r7        # r7 = DTMT type code
        cmpobne dtmt_ty_FT,r7,.dlmFTI_100 # Jif DTMT not Foreign Target type
        ld      dtmt_lldmt(g4),r7       # r7 = assoc. LLDMT address
        ldob    -ILTBIAS+dlmi_path(r7),r7  # r7 = path # that DTMT is assoc. with
        cmpobne r4,r7,.dlmFTI_100       # Jif specified DTMT in use with a different path
        b       .dlmFTI_150             # use the specified DTMT
#
# --- Scan DTMT list for a match on the MAC address
#
.dlmFTI_100:
        ld      vr_fti_ftdt(g13),r5     # r5 = FTDT address (FE local address)
        movl    g8,r6                   # save g8-g9
        ldl     ftdt_pwwn(r5),g4        # g4-g5 = port WWN of target
        ldl     ftdt_nwwn(r5),g8        # g8-g9 = node WWN of target
        call    dlm$chk4dtmt            # check for match on port WWN
        movl    r6,g8                   # restore g8-g9
        cmpobe  0,g4,.dlmFTI_120        # Jif no match found
        ldob    dtmt_type(g4),r7        # r7 = DTMT type code
        cmpobne dtmt_ty_FT,r7,.dlmFTI_120 # Jif DTMT not Foreign Target type
        b       .dlmFTI_150             # use the identified DTMT
#
.dlmFTI_120:
c       g4 = get_dtmt();                # allocate a DTMT for this target
.ifdef M4_DEBUG_DTMT
c fprintf(stderr, "%s%s:%u get_dtmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g4);
.endif # M4_DEBUG_DTMT
        st      g6,dtmt_lldmt(g4)       # save assoc. LLDMT in DTMT
        lda     dtmt_etbl2,r7           # r7 = DTMT event handler table
        st      r7,dtmt_ehand(g4)       # save DTMT event handler table
        ldconst dtmt_ty_FT,r7           # r7 = DTMT type code
        stob    r7,dtmt_type(g4)        # save DTMT type code
        ld      lldmt_dtmttl(g6),r7     # r7 = last DTMT on list
        cmpobne 0,r7,.dlmFTI_130        # Jif list not empty
        st      g4,lldmt_dtmthd(g6)     # save DTMT as new head member
        b       .dlmFTI_140
#
.dlmFTI_130:
        st      g4,dtmt_link(r7)        # link DTMT onto end of list
.dlmFTI_140:
        st      g4,lldmt_dtmttl(g6)     # put DTMT as new tail member
#
# --- Update DTMT info. from FTDT
#
.dlmFTI_150:
        ld      vr_fti_lldid(g13),r4    # r4 = link-level driver session ID
        st      r4,dtmt_lldid(g4)       # save link-level driver session ID in DTMT
        ld      vr_fti_ftdt(g13),r4     # r4 = FTDT address (FE local address)
        mov     r4,r5                   # Translate to global address
        ld      ftdt_alpa(r5),r7        # r7 = AL-PA address
        st      r7,dtmt_alpa(g4)        # save AL-PA address in DTMT
        ldl     ftdt_nwwn(r5),r8        # r8-r9 = node WWN
        stl     r8,dtmt_nwwn(g4)        # save node WWN in DTMT
        ldl     ftdt_pwwn(r5),r8        # r8-r9 = port WWN
        stl     r8,dtmt_pwwn(g4)        # save port WWN in DTMT
        ldl     ftdt_venid(r5),r8       # r8-r9 = vendor ID
        stl     r8,dft_venid(g4)        # save vendor ID
        ldq     ftdt_prid(r5),r8        # r8-r11 = product ID
        stq     r8,dft_prid(g4)         # save product ID
        ld      ftdt_version(r5),r8     # r8 = product revision number
        st      r8,dft_rev(g4)          # save product revision number
        ldob    ftdt_snlen(r5),r8       # r8 = device serial length
        stob    r8,dft_snlen(g4)        # save serial number length
        ldob    ftdt_luns(r5),r8        # r8 = # LUNs
        stob    r8,dft_luns(g4)         # save # LUNs
        ldob    ftdt_dtype(r5),r8       # r8 = device type code
        stob    r8,dft_dtype(g4)        # save device type code
        ldq     ftdt_sn(r5),r8          # r8-r11 = device serial number
        stq     r8,dft_sn(g4)           # save serial number
        st      g4,ftdt_dlmid(r5)       # save DTMT address in FTDT
        call    dlm$upsul               # update storage unit list info.
        call    dlm$sched_lddx          # schedule the LDD scan processes to add paths
        ldconst 0,r5                    # r5 = VRP status
        b       .dlmFTI_910             # and we're out of here!
#
.dlmFTI_900:
        ldconst eccancel,r5             # r5 = VRP error status to return
.dlmFTI_910:
        stob    r5,vr_status(g13)       # Save error status
        mov     g1,r11                  # save g1
        mov     g14,g1                  # g1 = ILT being completed
        call    K$comp                  # Complete this request
        mov     r11,g1                  # restore g1
        movq    r12,g4                  # restore g4-g7
        ret
#
#**********************************************************************
#
#  NAME: dlm$FTT
#
#  PURPOSE:
#       Processes a Foreign Target terminated VRP request.
#
#  DESCRIPTION:
#       Validates the target has been registered with the DLM and if
#       not valid simply ignores the request. If valid, calls the
#       target disappeared event handler routine associated with the
#       DTMT. It then returns completion status for the VRP request
#       to the requestor.
#
#  INPUT:
#       g13 = VRP address to process
#       g14 = ILT associated with VRP at nest level 2
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
dlm$FTT:
        movq    g4,r12                  # save g4-g7
        ldob    -ILTBIAS+dlmi_path(g14),r4 # r4 = path #
        lda     dlm_lldmt_dir,r5        # r5 = base of LLDMT directory
        ld      (r5)[r4*4],g6           # g6 = LLDMT for this interface
        cmpobe  0,g6,.dlmFTT_900        # Jif no LLDMT defined for this interface
        ld      vr_ftt_dlmid(g13),g4    # g4 = DLM session ID from VRP
        cmpobe  0,g4,.dlmFTT_90         # Jif DLM session ID not specified
#
# --- Validate association of DTMT with LLD session ID
#
        ld      dtmt_bnr(g4),r7         # r7 = DTMT banner value
        ld      dtmt_banner,r8          # r8 = DTMT banner pattern
        cmpobne r7,r8,.dlmFTT_900       # Jif DTMT not active
        ld      dtmt_lldmt(g4),r7       # r7 = DTMT assoc. LLDMT address
        ldob    dlmi_path-ILTBIAS(r7),r8 # r8 = path # assoc. with DTMT
        cmpobne r4,r8,.dlmFTT_900       # Jif VRP came from different path # then assoc. with DTMT
        ld      vr_ftt_lldid(g13),r7    # r7 = link-level driver ID
        ld      dtmt_lldid(g4),r8       # r8 = LLDID from DTMT
        cmpobe  r7,r8,.dlmFTT_150       # Jif match found
        b       .dlmFTT_900             # ignore VRP request.
#
# --- Scan DTMT list for a match on the link-level driver ID
#
.dlmFTT_90:
        ld      vr_ftt_lldid(g13),r7    # r7 = link-level driver ID
        ld      lldmt_dtmthd(g6),g4     # g4 = first DTMT on list
        cmpobe  0,g4,.dlmFTT_900        # Jif no DTMTs on active list
.dlmFTT_100:
        ld      dtmt_lldid(g4),r8       # r8 = LLDID from DTMT
        cmpobe  r7,r8,.dlmFTT_150       # Jif match found
        ld      dtmt_link(g4),g4        # g4 = next DTMT on active list
        cmpobne 0,g4,.dlmFTT_100        # Jif more DTMTs to check
        b       .dlmFTT_900             # ignore. no match found.
#
.dlmFTT_150:
        ld      dtmt_lldmt(g4),g6       # g6 = assoc. LLDMT address
#
# --- Check if DTMT is active. If not, ignore request.
#
        ld      lldmt_dtmthd(g6),r4     # r4 = first DTMT on list
.dlmFTT_200:
        cmpobe  0,r4,.dlmFTT_900        # Jif DTMT not found on list
        cmpobe  g4,r4,.dlmFTT_300       # Jif DTMT found on list
        ld      dtmt_link(r4),r4        # r4 = next DTMT on list
        b       .dlmFTT_200             # and go check next DTMT on list
#
.dlmFTT_300:
        ld      dtmt_ehand(g4),r4       # r4 = DTMT event handler table
        ld      dtmt_eh_tgone(r4),r4    # r4 = target disappeared event handler routine
        callx   (r4)                    # call target disappeared event handler routine
.dlmFTT_900:
        ldconst 0,r5                    # r5 = VRP status to return
        stob    r5,vr_status(g13)       # Save error status
        mov     g1,r11                  # save g1
        mov     g14,g1                  # g1 = ILT being completed
        call    K$comp                  # Complete this request
        mov     r11,g1                  # restore g1
        movq    r12,g4                  # restore g4-g7
        ret
#
#**********************************************************************
#
#  NAME: dlm$LST
#
#  PURPOSE:
#       Processes a link-level driver link session terminated VRP request.
#
#  DESCRIPTION:
#       Check that the LDD session and DLM session is valid.
#       If so, delink TPMT from LDD and DTMT and update links for CCB
#
#  INPUT:
#       g13 = VRP address to process
#       g14 = ILT associated with VRP at nest level 2
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
dlm$LST:
        movq    g0,r4                   # save g0-g3
#
#       get the link level session ID and TPMT
#
        ldconst ecinvlld,r11            # r11 = invalid LLD session id
        ld      vr_st_dlmid(g13),g3     # g3 = address of TPMT - DLM session ID
        cmpobe  0,g3,.dlmLST_100        # Jif DLM session ID zero
        ld      tpm_bnr(g3),r12         # r12 = banner from TPMT
        ld      tpmt_banner,r13         # r13 = TPMT banner pattern
        cmpobne r12,r13,.dlmLST_100     # Jif TPMT banner not correct
        ld      vr_st_lldid(g13),r12    # r12 = LLD driver session ID
        ld      tpm_lldid(g3),r13       # r13 = LLD driver session ID in TPMT
        cmpobne r12,r13,.dlmLST_100     # Jif not the correct LLD session
        ld      tpm_dtmt(g3),r12        # r12 = assoc. DTMT address
        ld      dtmt_lldmt(r12),r13     # r13 = DTMT assoc. LLDMT address
        ldob    dlmi_path-ILTBIAS(r13),r10 # r10 = path # assoc. with DTMT
        ldob    -ILTBIAS+dlmi_path(g14),r9 # r9 = path # where VRP came from
        cmpobne r9,r10,.dlmLST_100      # Jif VRP came from different path # then assoc. with TPMT
#
#       delink from LDD chain and DTMT chain
#
        ldob    tpm_state(g3),r11       # r11 = TPMT state
        ld      tpm_ldd(g3),g0          # g0 = associated ldd address
        cmpobne tpm_st_op,r11,.dlmLST_50 # Jif path not in operational state
        ldconst tpm_st_notop,r11        # r11 = path not operational state code
        stob    r11,tpm_state(g3)       # save new TPMT state
.dlmLST_50:
        call    DLM$dem_path            # demolish this path
#
#       update device links in LDD
#
        ldconst ecok,r11                # r11 = ok status
.dlmLST_100:
        stob    r11,vr_status(g13)      # return status
        mov     g14,g1                  # g1 = ILT being completed
        call    K$comp                  # Complete this request
        movq    r4,g0                   # restore global registers
        ret
#
#**********************************************************************
#
#  NAME: dlm$lrpio
#
#  PURPOSE:
#       To provide a means of processing LRP I/O requests which have been
#       previously queued to the data-link manager.
#
#  DESCRIPTION:
#       DLM$VLraid (and others???) queue LRP I/O requests to the
#       dlm_lrpio_qu and wake up this task to process them. This
#       process removes the LRP I/O requests from the queue and
#       schedules them to be completed. When processing is complete
#       it returns the request to the completion routine in the
#       dlmio_p_cr field of the LRP I/O request.
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
dlm$lrpio:
#
# --- Exchange processes ----------------------------------------------
#
.lrpio20:
        call    K$qxchang               # Exchange processes
#
# --- Get next queued request
#
        lda     dlm_lrpio_qu,r11        # Get LRP I/O queue pointer
        ld      qu_head(r11),r12        # r12 = queue head pointer
        ld      qu_tail(r11),r13        # r13 = queue tail pointer
        ld      qu_qcnt(r11),r14        # r14 = queue count
        ld      qu_pcb(r11),r15         # r15 = PCB address
        mov     r12,g14                 # g14 = LRP I/O being processed
        cmpobne 0,r12,.lrpio25          # Jif one found
#
# --- Set this process to not ready
#
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
        b       .lrpio20
#
.lrpio25:
#
# --- Remove this request from queue ----------------------------------
#
        ld      il_fthd(r12),r12        # r12 = next LRP I/O on queue
        cmpo    0,r12                   # Check for queue now empty
        subo    1,r14,r14               # Adjust queue count
        sele    r13,r12,r13             # Set up queue tail
        stt     r12,qu_head(r11)        # Update queue head, tail and count
        be      .lrpio30                # Jif queue now empty
#
        st      r11,il_bthd(r12)        # Update backward thread
#
.lrpio30:
        ld      dlmio_p_len(g14),r4     # r4 = LRP I/O length
        ld      dlmio_p_ldd(g14),g4     # g4 = assoc. LDD address
        cmpobe  0,r4,.lrpio40           # Jif no data transferred
        call    dlm$precedence          # check for I/O precedence
        cmpobe  TRUE,g0,.lrpio20        # Jif precedence occurred
.lrpio40:
        call    dlm$q_LRP               # queue LRP I/O request to LDD
        call    dlm$gen_LRP             # generate LRP I/O request
        b       .lrpio20                # and check for more LRP I/Os to process
#
#**********************************************************************
#
#  NAME: dlm$lrprty
#
#  PURPOSE:
#       To provide a means of processing LRP I/O requests which need
#       to be retried.
#
#  DESCRIPTION:
#       Scans all LDDs for LRP I/Os that are waiting to be retried
#       and will retry them.
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
#       None.
#
#**********************************************************************
#
dlm$lrprty:
        ldconst 10,r3                   # r3 = 1 sec. timer count
.lrprty_10:
        cmpobe  0,r3,dlm$lrprty         # Jif 1 sec. timer count expired
        ldconst lrprty_to,g0            # g0 = time delay period (in msec.)
        call    K$twait                 # wait for time delay
        subo    1,r3,r3                 # dec. 1 sec. timer count
        ldob    dlm_rtyflg,r4           # r4 = retry flag
        cmpobe  FALSE,r4,.lrprty_10     # Jif retry flag FALSE
        ldconst FALSE,r4
        stob    r4,dlm_rtyflg           # reset retry flag
        lda     DLM_lddindx,r15         # r15 = base address for LDD directory
        ldconst MAXLDDS,r14             # r14 = max. # LDDs supported
.lrprty_50:
        ld      (r15),g4                # g4 = LDD address from directory
        addo    4,r15,r15               # inc. to next LDD in directory
        subo    1,r14,r14               # dec. LDD count
        cmpobe  0,g4,.lrprty_200        # Jif LDD not defined in this slot
        ld      ld_tpmthd(g4),r12       # r12 = top TPMT on list
        ldos    ld_rtycnt(g4),r13       # r13 = retry count for this LDD
        cmpobe  0,r13,.lrprty_200       # Jif no LRPs waiting to be retried
        cmpobe  0,r12,.lrprty_60        # Jif no paths defined for this LDD
        cmpobe  0,r3,.lrprty_60         # Jif 1 sec. timer count expired
        ldconst TRUE,r4
        stob    r4,dlm_rtyflg           # set retry flag
        b       .lrprty_200
#
.lrprty_60:
        ldconst 0,r13
        stos    r13,ld_rtycnt(g4)       # clear retry count
        ld      ld_ailthd(g4),g14       # g14 = first LRP ILT on list
        cmpobe  0,g14,.lrprty_200       # Jif no LRP ILTs on list
        cmpobe  0,r12,.lrprty_75        # Jif no paths defined for this LDD
.lrprty_70:
        ldob    dlmio_p_state(g14),r4   # r4 = LRP I/O state code
        cmpobne diopst_rty,r4,.lrprty_180 # Jif not waiting to retry I/O
        ldob    dlmio_p_rtycnt(g14),r4  # r4 = retry count
        cmpobne 0,r4,.lrprty_100        # Jif retry count not expired
.lrprty_75:
        ld      il_fthd(g14),r10        # r10 = next LRP ILT on list
        ldob    dlmio_p_status(g14),r4  # r4 = status byte
        cmpobne 0,r4,.lrprty_80         # Jif error status already saved
        ldconst ecinop,r4               # r4 = error status code
        stob    r4,dlmio_p_status(g14)  # save error status in LRP
.lrprty_80:
        call    dlm$term_LRP            # terminate LRP
        mov     r10,g14                 # g14 = next LRP on list
        b       .lrprty_190
#
.lrprty_100:
        subo    1,r4,r4                 # dec. retry count
        stob    r4,dlmio_p_rtycnt(g14)  # save updated retry count
        call    dlm$gen_LRP             # generate an I/O request for this LRP
.lrprty_180:
        ld      il_fthd(g14),g14        # g14 = next LRP on list
.lrprty_190:
        cmpobne 0,g14,.lrprty_70        # Jif more LRPs on list
.lrprty_200:
        cmpobne 0,r14,.lrprty_50        # Jif more LDDs to process
        b       .lrprty_10              # and wait to check again
#
#**********************************************************************
#
#  NAME: dlm$lrpage
#
#  PURPOSE:
#       To provide a means of aging LRP I/O requests.
#
#  DESCRIPTION:
#       Scans all LDDs for LRP I/Os and decrement the retry count.
#       This is done once a second such that the retry count will
#       go to 0 at about 40 seconds.
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
#       None.
#
#**********************************************************************
#
dlm$lrpage:
        ldconst lrpage_to,g0            # g0 = time delay period (in msec.)
        call    K$twait                 # wait for time delay
        lda     DLM_lddindx,r15         # r15 = base address for LDD directory
        ldconst MAXLDDS,r14             # r14 = max. # LDDs supported
.lrpage_50:
        ld      (r15),g4                # g4 = LDD address from directory
        addo    4,r15,r15               # inc. to next LDD in directory
        subo    1,r14,r14               # dec. LDD count
        cmpobe  0,g4,.lrpage_200        # Jif LDD not defined in this slot
        ld      ld_ailthd(g4),g14       # g14 = first LRP ILT on list
        cmpobe  0,g14,.lrpage_200       # Jif no LRP ILTs on list
.lrpage_70:
        ldob    dlmio_p_rtycnt(g14),r4  # r4 = retry count
        cmpobe  0,r4,.lrpage_180        # Jif retry count expired
        subo    1,r4,r4                 # dec. retry count
        stob    r4,dlmio_p_rtycnt(g14)  # save updated retry count
.lrpage_180:
        ld      il_fthd(g14),g14        # g14 = next LRP on list
        cmpobne 0,g14,.lrpage_70        # Jif more LRPs on list
.lrpage_200:
        cmpobne 0,r14,.lrpage_50        # Jif more LDDs to process
        b       dlm$lrpage              # and wait to check again
#
#**********************************************************************
#
#  NAME: dlm$lrpcr
#
#  PURPOSE:
#       To provide a means of processing LRP I/O request completions.
#
#  DESCRIPTION:
#       If the LRP I/O request was successful, returns the associated
#       ILT/RRP to the requestor with successful completion status.
#       If an error was reported, checks if the LRP I/O can be retried
#       and if so sets it up to be retried by the dlm$lrprty task. If
#       the LRP I/O cannot be retried, it returns the ILT/RRP back to
#       the original requestor with the error. If the LRP I/O is returned
#       any dependent LRP I/Os will be processed appropriately.
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
#       None.
#
#**********************************************************************
#
dlm$lrpcr:
#
# --- Exchange processes ----------------------------------------------
#
.lrpcr_20:
        call    K$qxchang               # Exchange processes
#
# --- Get next queued request
#
        lda     dlm_lrpcr_qu,r11        # Get LRP I/O completion queue pointer
        ld      qu_head(r11),r12        # r12 = queue head pointer
        ld      qu_tail(r11),r13        # r13 = queue tail pointer
        ld      qu_qcnt(r11),r14        # r14 = queue count
        ld      qu_pcb(r11),r15         # r15 = PCB address
        mov     r12,g1                  # g1 = LRP I/O being processed
        cmpobne 0,r12,.lrpcr_25         # Jif one found
#
# --- Set this process to not ready
#
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
        b       .lrpcr_20
#
.lrpcr_25:
#
# --- Remove this request from queue ----------------------------------
#
        ld      il_fthd(r12),r12        # r12 = next LRP I/O on queue
        cmpo    0,r12                   # Check for queue now empty
        subo    1,r14,r14               # Adjust queue count
        sele    r13,r12,r13             # Set up queue tail
        stt     r12,qu_head(r11)        # Update queue head, tail and count
        be      .lrpcr_30               # Jif queue now empty
#
        st      r11,il_bthd(r12)        # Update backward thread
#
.lrpcr_30:
        ld      dlmio_s2_srp(g1),r8     # r8 = assoc. SRP address
        lda     -ILTBIAS(g1),g1         # g1 = sec. LRP I/O ILT at nest level 1
        ld      dlmio_s1_ilt(g1),g14    # g14 = primary LRP I/O ILT
        ld      dlmio_p_ilt(g14),r9     # r9 = sec. ILT from primary ILT
        cmpobne g1,r9,.lrpcr_900        # Jif ILTs not matched
        ld      dlmio_p_ldd(g14),g4     # g4 = assoc. LDD address
        ldconst 0,r6
        ldob    sr_status(r8),r4        # r4 = SRP completion status
        st      r6,dlmio_p_ilt(g14)     # clear sec. ILT from pri. ILT
        cmpobe  srok,r4,.lrpcr_800      # Jif SRP successful
#
# --- SRP completion error indicated
#
        ld      dlmio_s1_tpmt(g1),r5    # r5 = TPMT used for this LRP I/O
        cmpobe  0,r5,.lrpcr_50          # Jif TPMT has disappeared
        ld      tpm_ecnt(r5),r6         # r6 = TPMT error count
        addo    1,r6,r6                 # inc. error count
        st      r6,tpm_ecnt(r5)         # save updated TPMT error count
        ldconst TRUE,r6
        stob    r6,dlm_tpmtecflg        # set TPMT error count flag
.lrpcr_50:
        ldconst diopst_rty,r5           # r5 = waiting for retry state
        stob    r5,dlmio_p_state(g14)   # save new LRP state
        cmpobne srerr,r4,.lrpcr_100     # Jif not general error reported
        ldconst ectimeout,r4            # r4 = timeout error status
.lrpcr_xxx:
        ldos    ld_rtycnt(g4),r6        # r6 = LRP retry count
        ldconst TRUE,r5
        stob    r4,dlmio_p_status(g14)  # save error status in LRP
        addo    1,r6,r6                 # inc. LRP retry count
        stob    r5,dlm_rtyflg           # set LRP retry flag
        stos    r6,ld_rtycnt(g4)        # save updated retry count
        b       .lrpcr_900
#
.lrpcr_100:
        ld      sr_lrp_extstat(r8),r9   # r9 = extended status area address
        cmpobne sr_lrpst_cse,r4,.lrpcr_200 # Jif not check status error
#
# --- SCSI check status error handling routine ------------------------------
#
#
# --- Check for LUN not supported SENSE data (ASC value)
#       or undefined command SENSE data (ASC value)
#
        ldob    (r9),r4                 # r4 = ASC byte from SENSE data
        ldconst 0x25,r6                 # r6 = LUN not supported ASC value
        cmpobe  r4,r6,.lrpcr_110        # Jif LUN not supported ASC value
        ldconst 0x20,r6                 # r6 = undefined command ASC value
        cmpobne r4,r6,.lrpcr_190        # Jif not command not supported ASC value
#
# --- Set up logic to terminate session associated with LUN not
#       supported or undefined command SENSE data, reestablish a
#       new session to the device and then wait to retry I/O.
#
.lrpcr_110:
        ldob    ld_state(g4),r6         # r6 = current LDD state
        ldconst ecinop,r4               # r4 = possible error code
        cmpobe  ldd_st_pterm,r6,.lrpcr_800 # Jif LDD in pending terminate state
        cmpobe  ldd_st_term,r6,.lrpcr_800 # Jif LDD in terminated state
        ld      dlmio_s1_tpmt(g1),g3    # g3 = assoc. TPMT address
        mov     g4,g0                   # g0 = assoc. LDD address
        cmpobe  0,g3,.lrpcr_112         # Jif no TPMT assoc. with ILT
        call    DLM$dem_path            # demolish the path used for this I/O
.lrpcr_112:
        call    DLM$proc_ldd            # schedule LDD scan process for this linked device
.lrpcr_190:
        ldconst 0x41,r6                 # r6 = data path failure ASC value
        cmpobe.f r4,r6,.lrpcr_190a      # Jif data path failure ASC value
#*** TEMPORARY
        ldconst ectimeout,r4            # r4 = timeout error status
        b       .lrpcr_xxx              # and retry later
#*** END TEMPORARY
#
.lrpcr_190a:
        ldconst ecdataflt,r4            # r4 = data fault error status
        ldconst 0,r6
        stob    r6,dlmio_p_rtycnt(g14)  # clear retry count
        b       .lrpcr_xxx
#
.lrpcr_200:
        cmpobne sr_lrpst_pe,r4,.lrpcr_300 # Jif not process error
#
# --- Process error handling routine ----------------------------------------
#
        ldob    (r9),r5                 # r5 = process error type code
        ldconst lrpcr_PEmax,r6          # r6 = max. PE type code supported
        cmpobge r5,r6,.lrpcr_220        # Jif PE type code not supported
        ld      lrpcr_PEtbl[r5*4],r6    # r6 = error handler routine
        bx      (r6)                    # and jump to error handler routine
#
#
# --- Specify timeout error status and wait to retry I/O.
#
.lrpcr_220:
        ldconst ectimeout,r4            # r4 = timeout error status
        b       .lrpcr_xxx              # and retry later
#
# --- Set up logic to reestablish affected session to device and then
#       wait to retry I/O.
#
.lrpcr_230:
        ldob    ld_state(g4),r6         # r6 = current LDD state
        ldconst ecinop,r4               # r4 = possible error code
        cmpobe  ldd_st_pterm,r6,.lrpcr_800 # Jif LDD in pending terminate state
        cmpobe  ldd_st_term,r6,.lrpcr_800 # Jif LDD in terminated state
        ld      dlmio_s1_tpmt(g1),r5    # r5 = assoc. TPMT address
        mov     g4,g0                   # g0 = assoc. LDD address
        cmpobe  0,r5,.lrpcr_233         # Jif no TPMT assoc. with ILT
        ldob    tpm_state(r5),r4        # r4 = TPMT state
        cmpobne tpm_st_op,r4,.lrpcr_231 # Jif TPMT not in operational state
        ldconst tpm_st_notop,r4         # r4 = not operational state code
        stob    r4,tpm_state(r5)        # set TPMT state to not operational
.lrpcr_231:
        mov     r5,g3                   # g3 = TPMT that needs to be demolished
        call    DLM$dem_path            # demolish the path used for this I/O
.lrpcr_233:
        call    DLM$proc_ldd            # schedule LDD scan process for this linked device
        b       .lrpcr_220              # and retry operation in awhile
#
# --- Return null SGL error to requestor.
#
.lrpcr_240:
        ldconst ecnulsgl,r4             # r4 = null SGL list error status
.ifndef PERF
c fprintf(stderr,"%s%s:%u .lrpcr_240 sgl null\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # PERF
        b       .lrpcr_800              # and terminate I/O
#
# --- Return INOP
#
.lrpcr_250:
        ldconst ecinop,r4             # r4 = null SGL list error status
        b       .lrpcr_800              # and terminate I/O
#
# --- Process error type code handler routine table
#
        .data
lrpcr_PEtbl:
        .word   .lrpcr_230              # unused
        .word   .lrpcr_220              # invalid request function code
        .word   .lrpcr_230              # invalid provider ID
        .word   .lrpcr_230              # invalid requestor ID
        .word   .lrpcr_220              # interface not supported
        .word   .lrpcr_220              # interface out of service
        .word   .lrpcr_220              # invalid session type code
        .word   .lrpcr_220              # invalid persistent rec. ses. type
        .word   .lrpcr_220              # invalid error recovery mode
        .word   .lrpcr_220              # invalid task type code
        .word   .lrpcr_220              # invalid data transfer attribute
        .word   .lrpcr_220              # no SENSE data area specified
        .word   .lrpcr_220              # SENSE data area length zero
        .word   .lrpcr_240              # No SGL pointer defined
        .word   .lrpcr_250              # no Path
        .word   .lrpcr_250              # target not found
        .word   .lrpcr_250              # target not open
        .word   .lrpcr_220              # invalid task attribute
        .word   .lrpcr_220              # invalid tag sequence
        .word   .lrpcr_230              # session is active & non-shareable
        .word   .lrpcr_230              # no more LIDs available
        .word   .lrpcr_250              # could not logon to port
#
endlrpcr_PEtbl:
        .set    lrpcr_PEmax,(endlrpcr_PEtbl-lrpcr_PEtbl)/4 # max. support PE type code
#
        .text
#
.lrpcr_300:
        cmpobne sr_lrpst_ioe,r4,.lrpcr_400 # Jif not I/O error
#
# --- I/O error handling routine --------------------------------------------
#
        ldob    (r9),r5                 # r5 = I/O error type code
        cmpobne ioe_lprst,r5,.lrpcr_310 # Jif not loop reset occurred
        ldconst ecliprs,r4              # r4 = lip reset status
        b       .lrpcr_xxx              # and retry later
#
.lrpcr_310:
        cmpobne ioe_Punvl,r5,.lrpcr_320 # Jif not port unavailable or logged out
        ldconst eclgoff,r4              # r4 =  port logoff status
        b       .lrpcr_xxx              # and retry later
#
.lrpcr_320:
        cmpobne ioe_cmdabrtd,r5,.lrpcr_330 # Jif not command aborted
        ldconst ecabtrq,r4              # r4 = command aborted status
        b       .lrpcr_xxx              # and retry later
#
.lrpcr_330:
        cmpobne ioe_timeout,r5,.lrpcr_340 # Jif not timeout
        ldconst ectimeout,r4            # r4 = timeout error status
        b       .lrpcr_xxx              # and retry later
#
.lrpcr_340:
        cmpobne ioe_DMAerr,r5,.lrpcr_350 # Jif not DMA error
        ldconst ecdma,r4                # r4 = dma error status
        b       .lrpcr_xxx              # and retry later
#
.lrpcr_350:
        ldconst ectimeout,r4            # r4 = timeout error status
        b       .lrpcr_xxx              # and retry later
#
.lrpcr_400:
        cmpobne sr_lrpst_noncse,r4,.lrpcr_500 # Jif not non-check status
                                              #  error
#
# --- SCSI non-check status error handling routine --------------------------
#
        ldob    (r9),r5                 # r5 = SCSI status byte
        cmpobne scresc,r5,.lrpcr_410    # Jif not reservation conflict status
        ldconst ecreserved,r4           # r4 = error status for reservation conflict status
        b       .lrpcr_800              # and terminate I/O NOW!
#
.lrpcr_410:
        ldconst ectimeout,r4            # r4 = error status
        ldconst 0x30,r6                 # r6 = ACA active status
        cmpobe  r5,r6,.lrpcr_xxx        # Jif ACA active status
        ldconst 0x28,r6                 # r6 = task set full status
        cmpobe  r5,r6,.lrpcr_xxx        # Jif task set full status
        ldconst 0x22,r6                 # r6 = command terminated status
        cmpobe  r5,r6,.lrpcr_xxx        # Jif command terminated status
        ldconst ecundsstat,r4           # r4 = undetermined SCSI status error status
        b       .lrpcr_xxx              # all other status combinations
#
.lrpcr_500:
        cmpobe  sr_lrpst_me,r4,.lrpcr_600 # Jif not misc. error
#
# --- Miscellaneous error handling routine ----------------------------------
#
        ldconst ectimeout,r4            # r4 = error status for this event
        b       .lrpcr_xxx              # and retry later
#
# --- Not defined error code handling routine -------------------------------
#
.lrpcr_600:
        ldconst ectimeout,r4            # r4 = error status for this event
        b       .lrpcr_xxx              # and retry later
#
# --- SRP completed successfully
#
.lrpcr_800:
        stob    r4,dlmio_p_status(g14)  # save LRP completion status in LRP ILT
        call    dlm$term_LRP            # terminate LRP I/O
#
# --- Primary LRP ILT does not point to this sec. LRP ILT. Just deallocate
#       sec. ILT and SRP and get out!
#
.lrpcr_900:
        ld      ILTBIAS+dlmio_s2_srpvrp(g1),g0 # g0 = VRP address to release
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        ld      vr_blen(g0),g1          # g1 = size of the VRP/SRP combo
c       s_Free_and_zero(g0, g1, __FILE__, __LINE__); # release memory for VRP/SRP combo
        b       .lrpcr_20               # and check for more LRP completion requests to process
#
#**********************************************************************
#
#  NAME: dlm$lddx
#
#  PURPOSE:
#       To provide a means of scanning for available paths to a
#       specific linked device that have not been established.
#
#  DESCRIPTION:
#       Scans all targets checking for paths that can be used to
#       connect linked devices over.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       g2 = assoc. LDD address to scan
#       g3 = DLM_lddindx pointer where LDD is registered at
#            (used to validate that the LDD is still registered)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.
#
#**********************************************************************
#
dlm$lddx:
        movl    g2,r12                  # r12 = assoc. LDD address to scan
                                        # r13 = DLM_lddindx pointer
        ld      K_xpcb,r14              # r14 = my PCB address
        movt    r12,g0
        call    dlm$chk4lddx            # check if LDD scan process still necessary
        cmpobe  FALSE,g3,.lddx_10000    # Jif LDD scan process no longer necessary
        ldconst TRUE,r4
.lddx_50:
        ldob    ld_lock(r12),r5         # r5 = LDD lock
        cmpobe FALSE,r5,.lddx_70        # Jif LDD not locked
        ldconst 100,g0                  # g0 = time delay period (in msec.)
        call    K$twait                 # wait for time delay
        b       .lddx_50                # and try to obtain lock again
#
.lddx_70:
        stob    r4,ld_lock(r12)         # lock LDD
.lddx_100:
        ldconst FALSE,r4
        stob    r4,ld_flag1(r12)        # set process event flag #1 to FALSE
#
#*****************************************************************************
#
# --- Basic VLink processing
#
#        Checks if VLink is being terminated
#
        ldob    ld_state(r12),r4        # r4 = LDD state code
        cmpobe  ldd_st_op,r4,.lddx_999  # Jif LDD is in operational state
        cmpobe  ldd_st_uninit,r4,.lddx_999 # Jif LDD is uninitialized
        cmpobne ldd_st_pterm,r4,.lddx_999 # Jif not pending terminate state
#
# --- Terminate linked device.
#
        mov     r12,g0                  # g0 = LDD address being processed
        ldconst 0,r5
        ld      ld_tpmthd(g0),r10       # r10 = TPMT list head member
        st      r5,ld_lasttpmt(g0)      # clear last TPMT used field in LDD
        st      r5,ld_tpmthd(g0)        # clear TPMT list head member field in LDD
        cmpobe  0,r10,.lddx_MLvl300     # Jif no TPMTs assoc. with LDD
#
# --- LDD TPMT list is a circular list. Find the end of the list
#       and clear forward pointer.
#
        mov     r10,r9                  # r9 = TPMT being checked
.lddx_MLvl220:
        mov     r9,r8                   # r8 = last TPMT checked
        ld      tpm_ntpmt(r9),r9        # r9 = next TPMT on LDD list
        cmpobne r9,r10,.lddx_MLvl220    # Jif end of list not found
        st      r5,tpm_ntpmt(r8)        # clear link field in last TPMT
        lda     DLM$dem_mlpath,r11      # r11 = path demolition routine address for MAG link
        ldob    ld_class(r12),r4        # r4 = linked device class
        cmpobe  ldmld,r4,.lddx_MLvl250  # Jif MAG link device class
        lda     DLM$dem_ftpath,r11      # r11 = path demolition routine address for Foreign Target
.lddx_MLvl250:
        cmpobe  0,r10,.lddx_MLvl300     # Jif no more TPMTs to terminate
        mov     r10,g3                  # g3 = TPMT to terminate
        ld      tpm_ntpmt(g3),r10       # r10 = next TPMT on LDD/TPMT list
        st      r5,tpm_ntpmt(g3)        # clear link list field
        callx   (r11)                   # call path demolition routine
        b       .lddx_MLvl250           # and check next TPMT if more
#
.lddx_MLvl300:
        call    DLM_clr_ldd             # clear out LDD to ready it for demolition
        call    DLM_put_ldd             # demolish LDD
        call    D$p2updateconfig        # update NVRAM changes
        b       .lddx_10000             # and terminate LDD scan process
#
# --- End check for basic VLink processing
#
#*****************************************************************************
#
#
# --- LDD state either operational or uninitialized
#
.lddx_999:
        ldob    ld_class(r12),r7        # r7 = linked device class code
        cmpobe  ldmld,r7,.lddx_1000     # Jif MAGNITUDE linked device
        cmpobe  ldftd,r7,.lddx_2000     # Jif Foreign Target device
        b       .lddx_9000              # unsupported class. Terminate LDD scan process
#
##############################################################################
#
# --- LDD scan process for XIOtech Controller linked devices ------------------
#
.lddx_1000:
#
# --- Find associated MLMT for specified XIOtech Controller
#
        ld      ld_basesn(r12),r10      # r10 = specified MAG serial #
        ldob    ld_basecl(r12),r3       # r3 = specified cluster #
        ld      dlm_mlmthd,g7           # g7 = first MLMT on list
.lddx_1020:
        cmpobe  0,g7,.lddx_MLa1000      # Jif no more MLMTs to process
        ld      mlmt_sn(g7),r5          # r5 = MAG serial # from MLMT
        cmpobe  r10,r5,.lddx_1050       # Jif match on serial #
        ld      mlmt_link(g7),g7        # g7 = next MLMT on list
        b       .lddx_1020              # and check next MLMT
#
# --- Associated MLMT identified for specified XIOtech Controller
#
.lddx_1050:
                                        # g7 = MLMT for specified MAG link
#
# --- Check to add paths to specified XIOtech Controller Link device
#
.lddx_MLa100:
        ld      mlmt_dtmthd(g7),g1      # g1 = first DTMT on MLMT list
        cmpobe  0,g1,.lddx_MLa1000      # Jif no DTMTs assoc. with MLMT
.lddx_MLa120:
        ldob    dml_cl(g1),r9           # r9 = assoc. cluster # assoc. with this DTMT
        cmpobne r9,r3,.lddx_MLa300      # Jif wrong cluster #
#
# --- Check if path already exists for this DTMT/LDD combo
#
        ld      dtmt_tpmthd(g1),g3      # g3 = first TPMT on DTMT list
.lddx_MLa130:
        cmpobe  0,g3,.lddx_MLa200       # Jif no TPMTs assoc. with DTMT
        ld      tpm_ldd(g3),r4          # r4 = assoc. LDD path is for
        cmpobe  r12,r4,.lddx_MLa150     # Jif TPMT is for this LDD
        ld      tpm_link(g3),g3         # g3 = next TPMT on DTMT list
        b       .lddx_MLa130            # and check next TPMT on list
#
.lddx_MLa150:
        call    DLM$chk_mlpath          # validate this path
        b       .lddx_MLa300            # and process next DTMT on MLMT list
#
# --- Add path to specified XIOtech Controller Link device
#
.lddx_MLa200:
        mov     r12,g0                  # g0 = LDD to add path to
        call    DLM$add_mlpath          # add path for LDD using this DTMT
.lddx_MLa300:
        call    dlm$getnextdtmt         # get next DTMT on MLMT list
                                        # g0 = FALSE if DTMT not on list
                                        # g0 = TRUE if DTMT still on list
                                        # g1 = next DTMT on list if g0=TRUE
        cmpobe  FALSE,g0,.lddx_MLa100   # Jif need to restart from top of list
        cmpobne 0,g1,.lddx_MLa120       # Jif more DTMTs to process
.lddx_MLa1000:
        ld      ld_tpmthd(r12),r4       # r4 = first TPMT on list
        cmpobe  0,r4,.lddx_MLa1100      # Jif no TPMTs assoc. with LDD
        mov     r4,r5                   # r5 = first TPMT on list
.lddx_MLa1050:
        ldob    tpm_state(r4),r6        # r6 = TPMT state code
        cmpobe  tpm_st_op,r6,.lddx_MLa1200 # Jif at least one path operational
        ld      tpm_ntpmt(r4),r4        # r4 = next TPMT on list
        cmpobne r4,r5,.lddx_MLa1050     # Jif not the end of list
.lddx_MLa1100:
        ldconst ldd_st_uninit,r4        # r4 = LDD uninitialized state code
        b       .lddx_MLa1300
#
.lddx_MLa1200:
        ldconst ldd_st_op,r4            # r4 = LDD operational state code
.lddx_MLa1300:
        stob    r4,ld_state(r12)        # save uninitialized state code in LDD
        mov     r12,g0                  # g0 = LDD address being processed
        b       .lddx_9000
#
# --- End check to add paths to specified XIOtech Controller Link device
#
#*****************************************************************************
#
#
# --- End LDD scan process for XIOtech Controller linked devices -------------
#
##############################################################################
#
# --- LDD scan process for Foreign Target devices ----------------------------
#
.lddx_2000:
#
# --- Check to add paths to specified Foreign Target device
#
        ldconst 0,r11                   # r11 = LLDMT index
.lddx_FTa100:
        ld      dlm_lldmt_dir[r11*4],r10 # r10 = LLDMT address
        cmpobe  0,r10,.lddx_FTa500      # Jif no LLDMT defined for this path
        ld      lldmt_dtmthd(r10),g1    # g1 = first DTMT on LLDMT list
        cmpobe  0,g1,.lddx_FTa500       # Jif no DTMTs assoc. with LLDMT
.lddx_FTa120:
        ldob    dtmt_type(g1),r8        # r8 = target type code
        cmpobne dtmt_ty_FT,r8,.lddx_FTa300 # Jif not Foreign Target type
#
# --- Check if device type, product ID, vendor ID match
#
        ldob    dft_dtype(g1),r4        # r4 = device type code
        ldl     dft_venid(g1),r4        # r4-r5 = vendor ID from DTMT
        ldl     ld_vendid(r12),r6       # r6-r7 = desired vendor ID
        cmpobne r4,r6,.lddx_FTa300      # Jif vendor ID does not match
        cmpobne r5,r7,.lddx_FTa300      # Jif vendor ID does not match
        ldl     dft_prid(g1),r4         # r4-r5 = bytes 1-8 of DTMT product ID
        ldl     ld_prodid(r12),r6       # r6-r7 = bytes 1-8 of desired ID
        cmpobne r4,r6,.lddx_FTa300      # Jif product ID does not match
        cmpobne r5,r7,.lddx_FTa300      # Jif product ID does not match
        ldl     dft_prid+8(g1),r4       # r4-r5 = bytes 9-16 of DTMT prod. ID
        ldl     ld_prodid+8(r12),r6     # r6-r7 = bytes 9-16 of desired ID
        cmpobne r4,r6,.lddx_FTa300      # Jif product ID does not match
        cmpobne r5,r7,.lddx_FTa300      # Jif product ID does not match
#
# --- Check if path already exists for this DTMT/LDD combo
#
        ld      dtmt_tpmthd(g1),g3      # g3 = first TPMT on DTMT list
.lddx_FTa130:
        cmpobe  0,g3,.lddx_FTa200       # Jif no TPMTs assoc. with DTMT
        ld      tpm_ldd(g3),r4          # r4 = assoc. LDD path is for
        cmpobe  r12,r4,.lddx_FTa300     # Jif TPMT is for this LDD
        ld      tpm_link(g3),g3         # g3 = next TPMT on DTMT list
        b       .lddx_FTa130            # and check next TPMT on list
#
# --- Path does not already exist to this Foreign Target device
#
.lddx_FTa200:
        mov     r12,g0                  # g0 = LDD to add path to
        call    DLM$add_ftpath          # add path for LDD using this DTMT
.lddx_FTa300:
        ld      dlm_lldmt_dir[r11*4],r9 # r9 = LLDMT address
        cmpobne r9,r10,.lddx_FTa100     # Jif different LLDMT for this path
        ld      dtmt_link(g1),g1        # g1 = next DTMT on list
        cmpobne 0,g1,.lddx_FTa120       # Jif more DTMTs to process
.lddx_FTa500:
        addo    1,r11,r11               # inc. LLDMT index
#
        cmpobg  MAXISP,r11,.lddx_FTa100 # Jif more paths to process
#
        ld      ld_tpmthd(r12),r4       # r4 = first TPMT on list
        cmpobe  0,r4,.lddx_FTa1100      # Jif no TPMTs assoc. with LDD
        mov     r4,r5                   # r5 = first TPMT on list
.lddx_FTa1050:
        ldob    tpm_state(r4),r6        # r6 = TPMT state code
        cmpobe  tpm_st_op,r6,.lddx_FTa1200 # Jif at least one path operational
        ld      tpm_ntpmt(r4),r4        # r4 = next TPMT on list
        cmpobne r4,r5,.lddx_FTa1050     # Jif not the end of list
.lddx_FTa1100:
        ldconst ldd_st_uninit,r4        # r4 = LDD uninitialized state code
        b       .lddx_FTa1300
#
.lddx_FTa1200:
        ldconst ldd_st_op,r4            # r4 = LDD operational state code
.lddx_FTa1300:
        stob    r4,ld_state(r12)        # save uninitialized state code in LDD
        mov     r12,g0                  # g0 = LDD address being processed
#
# --- End check to add paths to specified Foreign Target device
#
#
# --- End LDD scan process for Foreign Target devices ------------------------
#
##############################################################################
#
# --- Unregister LDD scan process from LDD and terminate LDD scan process
#
.lddx_9000:
        ldob    ld_flag1(r12),r4        # r4 = LDD process event flag
        cmpobe  TRUE,r4,.lddx_100       # Jif process event flag TRUE
        ldconst 0,r4
        ldconst FALSE,r5
        st      r4,ld_pcb(r12)          # clear LDD scan process PCB from LDD
        stob    r5,ld_lock(r12)         # unlock LDD
#
# --- Terminate LDD scan process
#
.lddx_10000:
        ret                             # terminate this LDD scan process!
#
#**********************************************************************
#
#  NAME: dlm$tpmtec
#
#  PURPOSE:
#       To provide a means of reseting TPMT error counts periodically.
#
#  DESCRIPTION:
#       Scans all LDDs for TPMTs that have non-zero error counts
#       and will reset them.
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
#       None.
#
#**********************************************************************
#
dlm$tpmtec:
.tpmtec_10:
        ldconst tpmtec_to,g0            # g0 = time delay period (in msec.)
        call    K$twait                 # wait for time delay
        ldob    dlm_tpmtecflg,r4        # r4 = TPMT error count flag
        cmpobe  FALSE,r4,.tpmtec_10     # Jif TPMT error count flag FALSE
        ldconst FALSE,r4
        stob    r4,dlm_tpmtecflg        # reset TPMT error count flag
        lda     DLM_lddindx,r15         # r15 = base address for LDD directory
        ldconst MAXLDDS,r14             # r14 = max. # LDDs supported
        ldconst 0,r3                    # r3 = 0
.tpmtec_50:
        ld      (r15),g4                # g4 = LDD address from directory
        addo    4,r15,r15               # inc. to next LDD in directory
        subo    1,r14,r14               # dec. LDD count
        cmpobe  0,g4,.tpmtec_200        # Jif LDD not defined in this slot
        ld      ld_tpmthd(g4),r4        # r4 = first TPMT on list
        cmpobe  0,r4,.tpmtec_200        # Jif no TPMTs assoc. with LDD
        mov     r4,r5                   # r5 = first TPMT on list
.tpmtec_70:
        st      r3,tpm_ecnt(r4)         # clear TPMT error count
        ld      tpm_ntpmt(r4),r4        # r4 = next TPMT on list
        cmpobne r4,r5,.tpmtec_70        # Jif more TPMTs on list
.tpmtec_200:
        cmpobne 0,r14,.tpmtec_50        # Jif more LDDs to process
        b       .tpmtec_10              # and wait to check again
#
#**********************************************************************
#
#  NAME: dlm$vlopen
#
#  PURPOSE:
#       This process reestablishes a VLink session.
#
#  DESCRIPTION:
#       This process opens a session for an established VLink and
#       when successful, brings up the LDD paths and releases any
#       I/O operations to the destination VDisk.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       g5 = assoc. VLOP address of process
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.
##
#**********************************************************************
#
dlm$vlopen:
        ldob    vlop_state(g5),r4       # r4 = process state code
        cmpobe  vlop_st_op,r4,.vlopen_100 # Jif process operational
#
# --- Process has been aborted
#
.vlopen_50:
#
# --- Send message to CCB about the end of the open process. Using an
#       available memory segment but putting the data in the format found in
#       LOG_Defs.h.
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       r4 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mlevlinkopenend,r3      # VLink Open Process Ending
        stos    r3,mle_event(r4)        # Type of message
        ld      vlop_rdd(g5),r3         # Get the RDD
        ldos    rd_rid(r3),r15          # Get the RAID ID
        ldos    rd_vid(r3),r14          # Get the VDisk ID
        ldos    vlop_r0(g5),r13         # Get the VLOP VDisk ID
        ldob    vlop_state(g5),r12      # Get the VLOP State
        stos    r13,log_dlm_ccb_vl_srcsn(r4) # Save the VLOP VDisk ID
        stos    r14,log_dlm_ccb_vl_srcsn+2(r4) # Save the RAID VDisk ID
        stos    r15,log_dlm_ccb_vl_srcsn+4(r4) # Save the RAID ID
        stos    r12,log_dlm_ccb_vl_srcsn+6(r4) # Save the VLOP State and clear byte
        st      g5,log_dlm_ccb_vl_srcsn+8(r4) # Save the VLOP being ended
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], log_dlm_ccb_vl_size);
#
c       s_Free(g5, vlopsiz, __FILE__, __LINE__); # Free VLOP memory
        ret                             # and terminate process
#
# --- Re-establish VLink with destination XIOtech Controller if this is the
#       owning controller ID
#
.vlopen_100:
        ld      vlop_rdd(g5),r7         # r7 = RDD address
        ld      rd_psd(r7),r4           # r4 = PSD address
        ldos    ps_pid(r4),r5           # r5 = LDD ordinal
        ld      DLM_lddindx[r5*4],g0    # g0 = assoc. LDD address
        cmpobe  0,g0,.vlopen_190        # Jif LDD not defined for RDD
        ld      K_ficb,r13              # r13 = FICB address
        ld      ld_owner(g0),r12        # r12 = owning controller ID
        ld      fi_vcgid(r13),r13       # r13 = current owner serial #
        cmpobne r12,r13,.vlopen_50      # Jif not owner - do not establish VLink
        ldos    vlop_r0(g5),r15         # r15 = 16-bit VLink #
        ldob    vlop_r2(g5),g4          # g4 = establish VLink attributes
        mov     r15,g3                  # g3 = new 16-bit VLink #
c       g1 = 64;                        # size of DLM packet request.
c       r8 = 32;                        # Flag for other request to do if fails.
.vlopen_126:
        call    dlm$pkest_vl            # pack an establish VLink datagram to re-register VLink
                                        # g1 = datagram ILT at nest level 2
        mov     g0,r6                   # r6 = assoc. LDD address
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait

        mov     r6,g0                   # g0 = assoc. LDD address
        ld      dsc2_rshdr_ptr(g1),g2   # g2 = local response header address
        ldob    dgrs_status(g2),r11     # r11 = request completion status
        cmpobe  dg_st_ok,r11,.vlopen_200 # Jif no error reported on request
c   if (r8 == 32) {
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # return datagram resources
c       g1 = 32;
c       r8 = 0;                         # Flag that we don't have 32 to try on next failure.
        b       .vlopen_126
c   }
        mov     g1,r11                  # r11 = datagram ILT at nest level 2
        call    DLM$chk_vlconflt        # check if VLink conflict not valid
        cmpobne g1,r11,.vlopen_198      # Jif VLink conflict is not valid. Swap VLink lock.
.vlopen_160:
        call    dlm$vlop_validate       # validate this VLOP
.vlopen_180:
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram resources
        ldob    vlop_state(g5),r4       # r4 = process state code
        cmpobne vlop_st_op,r4,.vlopen_50 # Jif process not operational
.vlopen_190:
        ldconst 1000,g0                 # delay for awhile
        call    K$twait
        b       dlm$vlopen              # and try it again
#
# --- Send 64 bit of Swap Vlink Lock.
.vlopen_198:
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait

        mov     r6,g0                   # g0 = assoc. LDD address
        ld      dsc2_rshdr_ptr(g1),g2   # g2 = local response header address
        ldob    dgrs_status(g2),r11     # r11 = request completion status
        cmpobe  dg_st_ok,r11,.vlopen_200 # Jif no error reported on request
# Know that 64 bit dlm$pkswp_vl failed, try 32 bit version.
        lda     -dsc1_ulvl(g1),g1       # g1 = specified datagram ILT at nest level #1
        call    DLM$put_dg              # deallocate specified datagram resources
c       g1 = 32;                        # Try 32 bit version.
        call    dlm$pkswp_vl            # pack a Swap VLink Lock datagram
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait

        mov     r6,g0                   # g0 = assoc. LDD address
        ld      dsc2_rshdr_ptr(g1),g2   # g2 = local response header address
        ldob    dgrs_status(g2),r11     # r11 = request completion status
        cmpobe  dg_st_ok,r11,.vlopen_200 # Jif no error reported on request
        b       .vlopen_160
#
# --- VLink has been re-established successfully.
#
.vlopen_200:
        ldob    vlop_state(g5),r4       # r4 = process state code
        cmpobe  vlop_st_op,r4,.vlopen_210 # Jif process operational
#
# --- Process has been aborted
#
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram resources
        mov     r15,g3                  # g3 = 16-bit VLink #
        call    DLM$term_vl             # terminate VLink at destination XIOtech Controller
        b       .vlopen_50              # and terminate process
#
.vlopen_210:
#
# --- Check if VLink has been moved
#
        ldos    vlop_r0(g5),r14         # r14 = new 16-bit VLink #
        cmpobe  r15,r14,.vlopen_230     # Jif new cluster # the same
#
# --- VLink has moved. Set up new move process parameters and
#       schedule move VLink process.
#
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # return datagram resources
                                        # g0 = assoc. LDD address
.vlopen_225:
        mov     g5,r5                   # r5 = open process VLOP address
        ld      vlop_rdd(g5),g4         # g4 = assoc. RDD address
        ldconst 0,r3
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        st      r3,rd_vlop(g4)          # remove VLOP from RDD
        mov     r14,g3                  # g3 = new 16-bit VLink #
        mov     r15,g6                  # g6 = current owner 16-bit VLink #
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),g7         # r3 = current owner MAG serial #
        call    DLM$VLmove              # schedule VLink move process
        mov     r5,g5                   # g5 = VLink open process VLOP
        b       .vlopen_50              # and terminate VLink open process
#
.vlopen_230:
        ld      dsc2_rsbuf(g1),g3       # g3 = response message header
        ld      dgrs_resplen(g2),r11    # r11 = remaining response length
        lda     dgrs_size(g3),g3        # g3 = response data address
        bswap   r11,r11
c   if (r11 != DLM0_rs_estvl_size && r11 != DLM0_rs_estvl_size_GT2TB) {
        b       .vlopen_180             # Jif not right amount of data in datagram response
c   }
        ld      DLM0_rs_estvl_basesn(g3),r6 # r6 = base MAG serial #
        bswap   r6,r6                   #  in little-endian format
        ld      ld_basesn(g0),r4        # r4 = base MAG serial # from LDD
        cmpobne r6,r4,.vlopen_180       # Jif not the same
        ldob    DLM0_rs_estvl_basecl(g3),r6 # r6 = base MAG cluster #
        ldob    ld_basecl(g0),r4        # r4 = base MAG cluster # from LDD
        cmpobne r6,r4,.vlopen_180       # Jif not the same
        ldob    DLM0_rs_estvl_basevd(g3),r6 # r6 = base MAG VDisk #
        ldos    ld_basevd(g0),r4        # r4 = base MAG VDisk # from LDD
        cmpobne r6,r4,.vlopen_180       # Jif not the same
        ldl     ld_devcap(g0),r4        # r4 = size from LDD
c   if (r11 == DLM0_rs_estvl_size) {
        ld      DLM0_rs_estvl_dsiz(g3),r6 # r6,r7 = VDisk size in sectors
        ldconst 0,r7
        bswap   r6,r6
c       if (r5 != 0) {
c           r4 = 0xffffffffUL;          # Flag that the vdisk is too big for this DLM call.
c           r5 = 0;
c       }
c   } else {
        ldl     DLM0_rs_estvl_dsiz_GT2TB(g3),r6 # r6,r7 = VDisk size in sectors
c   }
        ldconst FALSE,r3                # Flag to save LDD to NVRAM
c   if ((*(UINT64 *)&r6) < (*(UINT64 *)&r4)) {
        b       .vlopen_180             # If new size < than original
c   }
c   if ((*(UINT64 *)&r6) == (*(UINT64 *)&r4)) {
        b       .vlopen_236             # Jif the size is the same
c   }
        stl     r6,ld_devcap(g0)        # save VDisk size in LDD
        ldconst TRUE,r3                 # Show NVRAM needs to be saved
.vlopen_236:
        ldob    DLM0_rs_estvl_altid(g2),r6 # r6 = MSB of alternate ID
        stob    r6,ld_altid+1(g0)       # save MSB of alternate ID
        ldob    DLM0_rs_estvl_altid+1(g2),r6 # r6 = LSB of alternate ID
        stob    r6,ld_altid(g0)         # save LSB of alternate ID
c   if (r11 == DLM0_rs_estvl_size) {
        ldl     DLM0_rs_estvl_name(g3),r4 # r4-r5 = device name
c   } else {
        ldl     DLM0_rs_estvl_name_GT2TB(g3),r4 # r4-r5 = device name
c   }
        ldl     ld_basename(g0),r6      # r6-r7 = current device name
        cmpobne r4,r6,.vlopen_240       # Jif names are not the same
        cmpobe  r5,r7,.vlopen_250       # Jif names are the same
.vlopen_240:
        stl     r4,ld_basename(g0)      # Save the value
        ldconst TRUE,r3                 # Show NVRAM needs to be saved
#
        movl    g0,r4
        ldos    ld_ord(g0),g0           # Set input parm (LDD ordinal)
        ldconst ecnvlink,g1
        call    D$changename            # Post it
        movl    r4,g0                   # Restore g0/g1
#
.vlopen_250:
        cmpobe  FALSE,r3,.vlopen_260    # Jif LDD does not need to be saved
#
# --- Save the LDD to NVRAM
#
        ldos    K_ii+ii_status,r4       # Get current initialization status
        bbs     iimaster,r4,.vlopen_255 # Jif already master
        PushRegs                        # Save all G registers (stack relative)
                                        # g0 = LDD Address
        ldconst lddsiz,g1               # g1 = Size of the LDD
        ldconst ebiseLDD,g2             # g2 = Event type is "LDD"
        ldconst ebibtmaster,g3          # g3 = Broadcast event to the master
        mov     0,g4                    # g4 = Serial Number (not used)
        call    D$reportEvent           # Send the updated LDD to the master
        PopRegsVoid                     # Restore all G registers (stack relative)
.vlopen_255:
.vlopen_260:
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # return datagram resources
                                        # g0 = assoc. LDD address
        call    DLM$proc_ldd            # start LDD scan process
#
# -------------------------------------------------------------------------
#       This section of code performs the following operations:
#       1: Waits for the LDD scan process to complete.
#       2: As it's waiting, it checks if the process has been aborted
#          and if so terminates the process.
#       3: As it's waiting, it ages the I/O ILTs queued to the VLOP.
#       4: When the LDD scan process completes, checks if any paths
#          have been established and if so terminates the VLOPEN process.
#       5: If no paths have been established, checks if the associated
#          VLink has been moved and if so schedules the VLMOVE process.
#          If the VLink has not been moved, starts the VLOPEN process again.
# -------------------------------------------------------------------------
#       g0 = assoc. LDD address
#       g5 = VLOPEN process VLOP address
#       r15 = 16-bit VLink # of established VLink
#
        mov     g0,g2                   # g2 = assoc. LDD address
        ld      ld_pcb(g2),g1           # g1 = LDD scan process PCB address
.vlopen_280:
        ldconst 10,r3                   # r3 = timer loop count
.vlopen_281:
        ldconst 100,g0                  # delay for awhile
        call    K$twait
        ldob    vlop_state(g5),r4       # r4 = process state code
        cmpobne vlop_st_op,r4,.vlopen_50 # Jif process not operational
#
# --- Check if VLink has been moved
#
        ldos    vlop_r0(g5),r14         # r14 = new 16-bit VLink #
        cmpobne r15,r14,.vlopen_225     # Jif VLink # different
        ld      ld_pcb(g2),r4           # r4 = LDD scan process PCB
        cmpobne r4,g1,.vlopen_290       # Jif LDD scan process has completed
        subo    1,r3,r3                 # dec. timer wait count
        cmpobne 0,r3,.vlopen_281        # Jif timer wait count non-zero
        b       .vlopen_280             # wait somemore
#
# --- LDD scan process has completed.
#
.vlopen_290:
        ld      ld_tpmthd(g2),g3        # check if any paths established to linked device
        cmpobne 0,g3,.vlopen_293        # Jif paths have been established
#
# --- No paths have been established. Wait 1 sec. and try again.
#
        ldconst 1000,g0                 # delay for awhile
        call    K$twait
        b       dlm$vlopen              # and try it again
#
.vlopen_293:
#
# --- VLink has been established and path(s) to it have been established.
#       Release any I/O ILTs to be processed.
#
#
# --- Change peer VDisk size just in case.
#
        ld      V_vddindx[r15*4],g4     # g4 = corresponding VDD
        cmpobe  0,g4,.vlopen_295        # Jif no VDD defined
        call    DLM$chg_size            # change peer VDisk size if necessary
.vlopen_295:
        ld      vlop_rdd(g5),r7         # r7 = assoc. RDD address
        ldconst 0,r4
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        st      r4,rd_vlop(r7)          # clear VLOP from RDD
#
# --- VLink re-establsihed and paths made to VLink. Check if any associated
#       copy operations are in auto-suspend state and if so kick them to
#       have them attempt to get going again.
#
#       input - g4 = assoc. VDD address of VLink being processed
#               g5 = assoc. VLOP address of process
#
        cmpobe.f 0,g4,.vlopen_50        # Jif no VDD defined
        mov     g4,r15                  # r15 = assoc. VDD address
        ld      vd_scdhead(r15),r4      # r4 = first SCD assoc. with VDD
        cmpobe.t 0,r4,.vlopen_550       # Jif no SCDs assoc. with VDD
.vlopen_510:
        ld      scd_cor(r4),g3          # g3 = assoc. COR address
        ld      cor_cm(g3),g4           # g4 = assoc. CM address
        cmpobe.f 0,g4,.vlopen_540       # Jif no CM assoc. with COR indicating this is a remote COR
        ldob    cor_crstate(g3),r5      # r5 = current registration state
        cmpobne.t corcrst_autosusp,r5,.vlopen_540 # Jif registration state not auto-suspended
        ldob    cor_flags(g3),r6        # r6 = cor_flags byte
        bbs.f   CFLG_B_POLL_REQ,r6,.vlopen_540  # Jif a local poll request is pending
        setbit  CFLG_B_POLL_REQ,r6,r6   # Set outstanding poll request flag
        stob    r6,cor_flags(g3)        # save updated flags byte
        call    CM$pksnd_local_poll     # generate a local poll request to kick start the copy operation
.vlopen_540:
        ld      scd_link(r4),r4         # r4 = next SCD assoc. with VDD
        cmpobne.t 0,r4,.vlopen_510      # Jif more SCDs assoc. with VDD
.vlopen_550:
        ld      vd_dcd(r15),r4          # r4 = DCD assoc. with VDD
        cmpobe.t 0,r4,.vlopen_590       # Jif no DCD assoc. with VDD
        ld      dcd_cor(r4),g3          # g3 = assoc. COR address
        ld      cor_cm(g3),g4           # g4 = assoc. CM address
        cmpobe.f 0,g4,.vlopen_590       # Jif no CM assoc. with COR indicating this is a remote COR
        ldob    cor_crstate(g3),r5      # r5 = current registration state
        cmpobne.t corcrst_autosusp,r5,.vlopen_590 # Jif registration state not auto-suspended
        ldob    cor_flags(g3),r6        # r6 = cor_flags byte
        bbs.f   CFLG_B_POLL_REQ,r6,.vlopen_590  # Jif a local poll request is pending
        setbit  CFLG_B_POLL_REQ,r6,r6   # Set outstanding poll request flag
        stob    r6,cor_flags(g3)        # save updated flags byte
        call    CM$pksnd_local_poll     # generate a local poll request to kick start the copy operation
.vlopen_590:
        b       .vlopen_50              # terminate open process
##
#**********************************************************************
#
#  NAME: dlm$vlopen_etbl
#
#  PURPOSE:
#       VLink open process event handler table.
#
#  DESCRIPTION:
#       Provides the routines to handle events while processing a
#       VLink open.
#
#**********************************************************************
#
        .data
dlm$vlopen_etbl:
        .word   dlm$vlop_abort          # abort event handler routine
        .word   dlm$vlop_move           # move event handler routine
#
        .text
#
#**********************************************************************
#
#  NAME: dlm$vlop_abort
#
#  PURPOSE:
#       Processes an abort event for the VLink open process.
#
#  DESCRIPTION:
#       Aborts the VLink open process.
#
#  INPUT:
#       g5 = VLOP address being aborted
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
dlm$vlop_abort:
        movq    g0,r12                  # save g0-g3
        ldob    vlop_state(g5),r4       # r4 = current process state
        cmpobne vlop_st_op,r4,.vlopabort_1000 # Jif process not in operational state
        ldconst vlop_st_term,r4         # r4 = process terminated state code
        stob    r4,vlop_state(g5)       # save terminated state code
        ld      vlop_rdd(g5),r5         # r5 = assoc. RDD address
        ldconst 0,r4
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        st      r4,rd_vlop(r5)          # remove VLOP from RDD
#
.vlopabort_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: dlm$vlop_move
#
#  PURPOSE:
#       Processes a move event for the VLink open process.
#
#  DESCRIPTION:
#       Saves the new VLink cluster # and VLink # in the associated
#       VLink open VLOP for the VLink open process to recognize.
#
#  INPUT:
#       g3 = new 16-bit VLink #
#       g4 = RDD address
#       g5 = current owner VLink cluster #
#       g6 = current owner VLink #
#       g7 = current owner MAG serial #
#       g9 = VLOP address of active process
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
dlm$vlop_move:
        ldob    vlop_state(g9),r4       # r4 = process state code
        cmpobne vlop_st_op,r4,.vlopmove_1000 # Jif process not operational
        stos    g3,vlop_r0(g9)          # save new 16-bit VLink #
.vlopmove_1000:
        ret
#
#**********************************************************************
#
#  NAME: dlm$vlmove
#
#  PURPOSE:
#       This process moves a VLink session.
#
#  DESCRIPTION:
#       This process moves a session for an established VLink to
#       another VLink and when successful, releases any
#       I/O operations to the destination VDisk.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       g3 = new 16-bit VLink #
#       g4 = assoc. RDD address
#       g6 = current owner 16-bit VLink #
#       g7 = current owner MAG serial #
#       g9 = assoc. VLOP address of process
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.
#
#**********************************************************************
#
dlm$vlmove:
        ldob    vlop_state(g9),r4       # r4 = process state code
        cmpobe  vlop_st_op,r4,.vlmove_100 # Jif process operational
#
# --- Process has been aborted
#
.vlmove_50:
c       s_Free(g9, vlopsiz, __FILE__, __LINE__); # Free VLOP memory
        ret                             # and terminate process
#
# --- Move VLink with destination XIOtech Controller
#
.vlmove_100:
        ld      vlop_rdd(g9),r7         # r7 = RDD address
        ld      rd_psd(r7),r4           # r4 = PSD address
        ldos    ps_pid(r4),r5           # r5 = LDD ordinal
        ld      DLM_lddindx[r5*4],g0    # g0 = assoc. LDD address
        cmpobe  0,g0,.vlmove_190        # Jif LDD not defined for RDD
        ldos    vlop_r0(g9),g3          # g3 = new 16-bit VLink #
        mov     g3,r15                  # r15 = new 16-bit VLink #
        ldob    vlop_r2(g9),g4          # g4 = establish VLink attributes
        ldos    vlop_r3(g9),g6          # g6 = current owner 16-bit VLink #
        ld      vlop_g0(g9),g7          # g7 = current owner MAG serial #
c       g1 = 64;                        # Flag a 64 bit version of the call.
        call    dlm$pkswp_vl            # pack a Swap VLink datagram to move VLink
                                        # g1 = datagram ILT at nest level 2
        mov     g0,r6                   # r6 = assoc. LDD address
.vlmove_160:
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait
        mov     r6,g0                   # g0 = assoc. LDD address
        ld      dsc2_rshdr_ptr(g1),g4   # g4 = local response header address
        ldob    dgrs_status(g4),r11     # r11 = request completion status
        cmpobe  dg_st_ok,r11,.vlmove_200 # Jif no error reported on request
# Try 32 bit version here.
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram resources
c       g1 = 32;                        # Try 32 bit version.
        call    dlm$pkswp_vl            # pack a Swap VLink Lock datagram
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait
        mov     r6,g0                   # g0 = assoc. LDD address
        mov     g1,r11                  # r11 = datagram ILT at nest level 2
        call    DLM$chk_vlconflt        # check if VLink conflict not valid
        cmpobne g1,r11,.vlmove_160      # Jif VLink conflict is not valid. Swap VLink lock (64 bit version).
.vlmove_180:
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram resources
.vlmove_190:
        ldconst 1000,g0                 # delay for awhile
        call    K$twait
        mov     g9,g5                   # g5 = assoc. VLOP address of process
        b       dlm$vlmove              # and try it again
#
# --- VLink has been moved successfully.
#
.vlmove_200:
        ldob    vlop_state(g9),r4       # r4 = process state code
        cmpobe  vlop_st_op,r4,.vlmove_210 # Jif process operational
#
# --- Process has been aborted
#
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram resources
        mov     r15,g3                  # g3 = 16-bit VLink #
        call    DLM$term_vl             # terminate VLink at destination XIOtech Controller
        b       .vlmove_50              # and terminate process
#
.vlmove_210:
#
# --- Check if VLink has moved again
#
        ldos    vlop_r0(g9),r14         # r14 = new 16-bit VLink #
        cmpobe  r15,r14,.vlmove_230     # Jif new VDisk # the same
#
# --- VLink has moved again. Set up new move process parameters and
#       do it again.
#
.vlmove_220:
        stos    r15,vlop_r3(g9)         # save new owner VLink #
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my VCG serial #
        st      r3,vlop_g0(g9)          # save new owner MAG serial #
        b       dlm$vlmove              # and start new process
#
.vlmove_230:
        ld      dsc2_rsbuf(g1),g3       # g3 = response message header
        ld      dgrs_resplen(g4),r11    # r11 = remaining response length
        lda     dgrs_size(g3),g3        # g3 = response data address
        bswap   r11,r11
c   if (r11 != DLM0_rs_swpvl_size && r11 != DLM0_rs_swpvl_size_GT2TB) {
        b       .vlmove_180
c   }
        ld      DLM0_rs_swpvl_basesn(g3),r6 # r6 = base MAG serial #
        bswap   r6,r6                   #  in little-endian format
        ld      ld_basesn(g0),r4        # r4 = base MAG serial # from LDD
        cmpobne r6,r4,.vlmove_180       # Jif not the same
        ldob    DLM0_rs_swpvl_basecl(g3),r6 # r6 = base MAG cluster #
        ldob    ld_basecl(g0),r4        # r4 = base MAG cluster # from LDD
        cmpobne r6,r4,.vlmove_180       # Jif not the same
        ldob    DLM0_rs_swpvl_basevd(g3),r6 # r6 = base MAG VDisk #
        ldos    ld_basevd(g0),r4        # r4 = base MAG VDisk # from LDD
        cmpobne r6,r4,.vlmove_180       # Jif not the same
c   if (r11 != DLM0_rs_swpvl_size && r11 != DLM0_rs_swpvl_size_GT2TB) {
        ldconst 0,r7
        ld      DLM0_rs_swpvl_dsiz(g3),r6 # r6,r7 = VDisk size in sectors
        bswap   r6,r6
c   } else {
        ldl     DLM0_rs_swpvl_dsiz_GT2TB(g3),r6 # r6,r7 = VDisk size in sectors
c   }
        ldl     ld_devcap(g0),r4        # r4,r5 = size from LDD
        ldconst FALSE,r3                # Show LDD does not need to be saved
        cmpobl  r7,r5,.vlmove_180       # Jif size < what was established
        cmpobl  r6,r4,.vlmove_180       # Jif size < what was established
        cmpobne r6,r4,.vlmove_233       # Jif the size is different
        cmpobe  r7,r5,.vlmove_236       # Jif the size is the same
.vlmove_233:
        stl     r6,ld_devcap(g0)        # save VDisk size in LDD
        ldconst TRUE,r3                 # Show LDD needs to be saved to NVRAM
#
.vlmove_236:
        ldob    DLM0_rs_swpvl_altid(g4),r6 # r6 = MSB of alternate ID
        stob    r6,ld_altid+1(g0)       # save MSB of alternate ID
        ldob    DLM0_rs_swpvl_altid+1(g4),r6 # r6 = LSB of alternate ID
        stob    r6,ld_altid(g0)         # save LSB of alternate ID
c   if (r11 != DLM0_rs_swpvl_size && r11 != DLM0_rs_swpvl_size_GT2TB) {
        ldl     DLM0_rs_swpvl_name(g3),r4 # r4-r5 = device name
c   } else {
        ldl     DLM0_rs_swpvl_name_GT2TB(g3),r4 # r4-r5 = device name
c   }
        ldl     ld_basename(g0),r6      # r6-r7 = current device name
        cmpobne r4,r6,.vlmove_240       # Jif the names are not the same
        cmpobe  r5,r7,.vlmove_250       # Jif the names are the same
.vlmove_240:
        stl     r4,ld_basename(g0)      # Save the value
        ldconst TRUE,r3                 # Show LDD needs to be saved to NVRAM
#
        movl    g0,r4
        ldos    ld_ord(g0),g0           # Set input parm (LDD ordinal)
        ldconst ecnvlink,g1
        call    D$changename            # Post it
        movl    r4,g0                   # Restore g0/g1
#
.vlmove_250:
        cmpobe  FALSE,r3,.vlmove_260    # Jif LDD does not need to be saved
#
# --- Save the LDD to NVRAM
#
        ldos    K_ii+ii_status,r4       # Get current initialization status
        bbs     iimaster,r4,.vlmove_255 # Jif already master
        PushRegs                        # Save all G registers (stack relative)
                                        # g0 = LDD Address
        ldconst lddsiz,g1               # g1 = Size of the LDD
        ldconst ebiseLDD,g2             # g2 = Event type is "LDD"
        ldconst ebibtmaster,g3          # g3 = Broadcast event to the master
        mov     0,g4                    # g4 = Serial Number (not used)
        call    D$reportEvent           # Send the updated LDD to the master
        PopRegsVoid                     # Restore all G registers (stack relative)
.vlmove_255:
#
.vlmove_260:
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # return datagram resources
                                        # g0 = assoc. LDD address
        call    DLM$proc_ldd            # start LDD scan process
#
# -------------------------------------------------------------------------
#       This section of code performs the following operations:
#       1: Waits for the LDD scan process to complete.
#       2: As it's waiting, it checks if the process has been aborted
#          and if so terminates the process.
#       3: As it's waiting, it ages the I/O ILTs queued to the VLOP.
#       4: When the LDD scan process completes, checks if any paths
#          have been established and if so terminates the VLMOVE process.
#       5: If no paths have been established, checks if the associated
#          VLink has been moved and if so starts the VLMOVE process again.
#          If the VLink has not been moved, schedules the VLOPEN process
#          to execute.
# -------------------------------------------------------------------------
#       g0 = assoc. LDD address
#       g9 = VLOPEN process VLOP address
#       r15 = 16-bit VLink # of established VLink
#
        mov     g0,g2                   # g2 = assoc. LDD address
        ld      ld_pcb(g2),g1           # g1 = LDD scan process PCB address
.vlmove_280:
        ldconst 10,r3                   # r3 = timer loop count
.vlmove_281:
        ldconst 100,g0                  # delay for awhile
        call    K$twait
        ldob    vlop_state(g9),r4       # r4 = process state code
        cmpobne vlop_st_op,r4,.vlmove_50 # Jif process not operational
#
# --- Check if VLink has been moved
#
        ldos    vlop_r0(g9),r14         # r14 = new 16-bit VLink #
        cmpobne r15,r14,.vlmove_220     # Jif VLink # different
        ld      ld_pcb(g2),r4           # r4 = LDD scan process PCB
        cmpobne r4,g1,.vlmove_290       # Jif LDD scan process has completed
        subo    1,r3,r3                 # dec. timer wait count
        cmpobne 0,r3,.vlmove_281        # Jif timer wait count non-zero
        b       .vlmove_280             # and wait somemore
#
# --- LDD scan process has completed.
#
.vlmove_290:
        ld      ld_tpmthd(g2),g3        # check if any paths established to linked device
        cmpobne 0,g3,.vlmove_295        # Jif paths established
        ld      vlop_rdd(g9),g4         # g4 = assoc. RDD address
        ldconst 0,r3
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        st      r3,rd_vlop(g4)          # remove VLOP from RDD
        mov     r15,g3                  # g3 = 16-bit VLink #
        call    DLM$VLopen              # schedule VLink open process
        b       .vlmove_50              # and terminate VLink move process
#
# --- VLink has been established and path(s) to it have been established.
#       Release any I/O ILTs to be processed.
#
.vlmove_295:
#
# --- Change peer VDisk size just in case.
#
        ld      V_vddindx[r15*4],g4     # g4 = corresponding VDD
        cmpobe  0,g4,.vlmove_298        # Jif no VDD defined
        call    DLM$chg_size            # change peer VDisk size if necessary
.vlmove_298:
#
# --- Release any I/O ILTs to be processed
#
        ld      vlop_rdd(g9),r7         # r7 = assoc. RDD address
        ldconst 0,r4
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        st      r4,rd_vlop(r7)          # clear VLOP from RDD
        b       .vlmove_50
#
#**********************************************************************
#
#  NAME: dlm$vlmove_etbl
#
#  PURPOSE:
#       VLink move process event handler table.
#
#  DESCRIPTION:
#       Provides the routines to handle events while processing a
#       VLink move.
#
#**********************************************************************
#
        .data
dlm$vlmove_etbl:
        .word   dlm$vlop_abort          # abort event handler routine
        .word   dlm$vlop_move           # move event handler routine
#
        .text
#**********************************************************************
#
#  NAME: dlm$vlock
#
#  PURPOSE:
#       To provide a means of periodically updating the VDisk/VLink
#       lock information for use by the CCB.
#
#  DESCRIPTION:
#       Every second, goes through the VDisk/VLink tables and saves
#       off the VDisk/VLink lock information for the CCB.
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
#       None.
#
#**********************************************************************
#
dlm$vlock:
        ldos    K_ii+ii_status,r4       # r4 = current initialization status
        bbc     iimaster,r4,.vlock_900  # Jif not the master controller
        ldconst 30,g4                   # g4 = # secs. without a VLink Poll before checking VLock
        ldconst FALSE,g5                # g5 = NVRAM update required flag
        ldconst MAXVIRTUALS,r3          # r3 = max. # VDisks/cluster
        ldconst 0,g3                    # g3 = VDisk #
.vlock_200:
        ld      V_vddindx[g3*4],g1      # g1 = VDD address
        cmpobe.t 0,g1,.vlock_450        # Jif VDD not defined
        ld      vd_vlinks(g1),g0        # g0 = assoc. VLAR address
        cmpobe.t 0,g0,.vlock_450        # Jif no VDisk/VLink lock applied
#
# --- Check if need to validate lock first
#
        ldob    vlar_poll(g0),r10       # r10 = VLAR poll count
        addo    1,r10,r10               # inc. VLAR poll count
        stob    r10,vlar_poll(g0)       # save updated poll count
        cmpobl.t r10,g4,.vlock_450      # Jif lock validation not indicated
        ldconst 0,r10
        stob    r10,vlar_poll(g0)       # clear poll count in VLAR
#
# --- Validate lock
#
        mov     g0,r10                  # save g0
        call    DLM$chk_vlock           # check if lock is still needed
                                        # g0 = completion status
                                        #      00 = lock still needed
                                        #      01 = lock not needed
                                        #      02 = could not validate lock with lock owner
        ld      V_vddindx[g3*4],r11     # r11 = VDD address
        cmpobne r11,g1,.vlock_200       # Jif VDD has changed
        ld      vd_vlinks(g1),r11       # r11 = top VLAR on list
        cmpobne r10,r11,.vlock_200      # Jif top VLAR has changed
        cmpobne 0x01,g0,.vlock_280      # Jif lock still needed or could not validate lock with lock owner
        mov     g7,r11                  # save g7
        mov     r10,g7                  # g7 = VLAR being disposed of
        mov     g1,g6                   # g6 = VDD assoc. with VLAR
        call    CM$whack_rcor           # suspend any copy ops. assoc.  with this VLAR
        ld      vlar_link(r10),r10      # r10 = next VLAR on list
        st      r10,vd_vlinks(g1)       # save remaining list in VDD
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
.ifdef M4_DEBUG_VLAR
c fprintf(stderr, "%s%s:%u put_vlar 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_VLAR
c       put_vlar(g7);                   # Deallocate VLAR
        ldconst TRUE,g5                 # set flag indicating NVRAM update needed
        mov     r11,g7                  # restore g7
        cmpobne 0,r10,.vlock_200        # Jif more VLARs assoc. with VDisk
        ldos    vd_attr(g1),r11         # r11 = attribute byte
        clrbit  vdbvdlock,r11,r11       # clear VDisk/VLink lock flag
        stos    r11,vd_attr(g1)         # save updated attribute flag
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        b       .vlock_200              # and start over again on this VDisk

.vlock_280:
        cmpobe.t 0,g0,.vlock_285        # Jif lock still needed
        ld      vd_dcd(g1),r5           # r5 = assoc. DCD address
        cmpobe.t 0,r5,.vlock_285        # Jif no DCD assoc. with VDD
        ld      dcd_cor(r5),r6          # r6 = assoc. COR address
        cmpobe.f 0,r6,.vlock_285        # Jif no COR assoc. with DCD
        ldob    cor_crstate(r6),r7      # r7 = cor_crstate value
        cmpobne.t corcrst_active,r7,.vlock_285 # Jif not active
        ld      cor_cm(r6),r7           # r7 = assoc. CM address
        cmpobne.f 0,r7,.vlock_285       # Jif local COR
        ld      cor_rcsn(r6),r7         # r7 = copy MAG serial #
        ld      vlar_srcsn(r10),r8      # r8 = VLAR MAG serial #
        cmpobne.f r7,r8,.vlock_285      # Jif serial #'s different
        ldconst corcrst_autosusp,r8     # r8 = new cor_crstate value
        stob    r8,cor_crstate(r6)      # save new cor_crstate in COR
        mov     g3,r7                   # save g3
        mov     r6,g3                   # g3 = COR address
        call    CM$mmc_sflag            # indicate copy operation is not active to MMC
        mov     r7,g3                   # restore g3
.vlock_285:
        mov     r10,g0                  # restore g0
#
# --- Go to the next VDisk Record
#
.vlock_450:
        addo    1,g3,g3                 # inc. VDisk #
        cmpobg  r3,g3,.vlock_200        # Jif more VDisks to check
#
# --- Check if NVRAM needs updating
#
        cmpobne TRUE,g5,.vlock_900      # Jif NVRAM does not need updating
        call    D$p2updateconfig        # update NVRAM
.vlock_900:
#
# --- Delay before updating this information again.
#
        ldconst 1000,g0                 # g0 = time delay period (in msec.)
        call    K$twait                 # wait for time delay
        b       dlm$vlock               # and update the info. again
#
#**********************************************************************
#
#  NAME: dlm$vlchk
#
#  PURPOSE:
#       To provide a means of periodically validating a VLink
#       registration on the peer XIOtech Controller.
#
#  DESCRIPTION:
#       For VLinks defined to XIOtech Controller links, sends a VLink Poll
#       datagram request to validate the VLink registration and
#       to reset the peer XIOtech Controller VLink lock poll logic.
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
#       None.
#
#**********************************************************************
#
dlm$vlchk:
        ldconst MAXVIRTUALS,r3          # r3 = max. # VDisks/cluster
        ldconst 0,g3                    # g3 = VDisk #
.vlchk_200:
        ldob    DLM_vlchk_flag,g5       # r5 = disable VLink check flag
        cmpobne 0,g5,.vlchk_1000        # Jif VLink check disabled
        ld      V_vddindx[g3*4],g5      # g5 = VDD address
        cmpobe  0,g5,.vlchk_400         # Jif VDD is no longer defined
        ld      vd_rdd(g5),g6           # g6 = RDD of the first RAID segment
        cmpobe  0,g6,.vlchk_400         # Jif RDD not there
        ldob    rd_type(g6),r6          # r6 = RAID type code of first RAID segment
        cmpobne rdlinkdev,r6,.vlchk_400 # Jif not linked device RAID type
        ld      rd_psd(g6),r6           # r6 = first PSD address
        ldos    ps_pid(r6),g8           # g8 = assoc. LDD index
        ld      DLM_lddindx[g8*4],g8    # g8 = LDD address
        cmpobne 0,g8,.vlchk_220         # Jif LDD defined
#
# No LDD defined for VID - something is wrong
#
        ldconst dlm_sft12,r11           # r11 = error code to log
        lda     dlm_sft,g0              # g0 = Software Fault Log Area
        st      r11,efa_ec(g0)          # Save the Error Code
        st      g3,efa_data(g0)         # Save the VID
        st      g5,efa_data+4(g0)       # Save the VDD pointer
        st      g6,efa_data+8(g0)       # Save the RDD pointer
        ld      rd_psd(g6),r11          # Get the PSD pointer
        st      r11,efa_data+12(g0)     # Save the PSD pointer
        ldconst 20,r11                  # Number of bytes saved (ec + data)
        st      r11,mle_len(g0)         # Save the number of bytes to send
        call    M$soft_flt              # Error Trap or Log failure
        b       .vlchk_400              # Ignore this VDisk
#
.vlchk_220:
        ldob    ld_class(g8),r6         # r6 = linked device class
        cmpobne ldmld,r6,.vlchk_400     # Jif not a MAGNITUDE link class device
        mov     g8,g4                   # g4 = LDD address
        call    dlm$pkvl_poll           # pack a VLink poll datagram
                                        # Input g3 = 16-bit VLink #
                                        #       g4 = LDD
                                        # Output g1 = dg at ILT nest level #2
        ld      ld_tpmthd(g8),r6        # r6 = first TPMT on list
        cmpobe  0,r6,.vlchk_240         # Jif no TPMTs associated with LDD
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait
        ld      V_vddindx[g3*4],r11     # r11 = VDD address
        cmpobe  r11,g5,.vlchk_230       # Jif VDD the same
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram resources
        b       .vlchk_200              # and check it again
#
.vlchk_230:
        ld      dsc2_rshdr_ptr(g1),g4   # g4 = local response header address
        ldob    dgrs_status(g4),r11     # r11 = request completion status
        ldob    DLM0_rs_vlpol_stat(g4),r12 # r12 = VLink status code if successful completion
        cmpobne dg_st_ok,r11,.vlchk_300 # Jif error reported on request
        cmpobne 0,r12,.vlchk_240        # Jif VLink not locked by me
        ldob    DLM0_rs_vlpol_altid(g4),r6 # r6 = MSB of alternate ID
        stob    r6,ld_altid+1(g8)       # save MSB of alternate ID
        ldob    DLM0_rs_vlpol_altid+1(g4),r6 # r6 = LSB of alternate ID
        stob    r6,ld_altid(g8)         # save LSB of alternate ID
        call    dlm$chk_paths           # check if all possible paths have been established
                                        # Input g8 = LDD address
                                        # Output g0 = TRUE (all paths est.)
                                        #           = FALSE (not all paths est.)
        cmpobe  TRUE,g0,.vlchk_300      # Jif all possible paths have been established
.vlchk_240:
        mov     g6,g4                   # g4 = associated RDD address
        call    DLM$VLopen              # schedule VLink open process
.vlchk_300:
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram resources
.vlchk_400:
        addo    1,g3,g3                 # inc. VDisk #
        cmpobg  r3,g3,.vlchk_200        # Jif more VDisks to check for this cluster
#
# --- Delay before updating this information again.
#
.vlchk_1000:
        ldconst 10000,g0                # g0 = time delay period (in msec.)
        call    K$twait                 # wait for time delay
        b       dlm$vlchk               # and do it again!
#
#******************************************************************************
#
# ____________________ DTMT EVENT HANDLER TABLES ______________________________
#
#******************************************************************************
#
# --- Foreign target event handler table ------------------------------------
#
        .data
dtmt_etbl2:
        .word   dlm$term_FT             # Target disappeared event
#
        .text
#
#******************************************************************************
#
# ____________________ DTMT EVENT HANDLER ROUTINES ____________________________
#
#******************************************************************************
#
# --- Foreign Target disappeared event handler routine ------------------------
#
#
#******************************************************************************
#
#  NAME:  dlm$term_FT
#
#  PURPOSE:
#       Processes a Foreign Target disappeared event.
#
#  DESCRIPTION:
#       Demolishes all associated TPMTs and then terminates the DTMT
#       associated with the target.
#
#  INPUT:
#       g4 = assoc. DTMT of target being terminated
#       g6 = assoc. LLDMT address for DTMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$term_FT:
        b       dlm$term_ML             # handle the same for now!
#
#******************************************************************************
# _______________________ DLM Server Handler Routines _________________________
#
#******************************************************************************
#
#  NAME:  dlm$DLM0
#
#  PURPOSE:
#       Handle datagram services for server DLM0.
#
#  DESCRIPTION:
#       Decodes the request function code in the datagram message
#       and vectors to the proper routine to process the request.
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
dlm$DLM0:
        ldob    dgrq_fc(g2),r4          # r4 = request function code
        ldconst DLM0$maxfc,r5           # r5 = max. valid function code
        cmpobge r5,r4,.DLM0_100         # Jif valid request function code
#
# --- Invalid request function code. Return error to requestor.
#
        call    DLM$srvr_invfc          # pack and return invalid function code response to requestor
        b       .DLM0_1000              # and we're out of here!
#
# --- Valid request function code.
#
.DLM0_100:
.ifndef PERF
#-----------------------------------------------------------------------------
c r3 = 1;
c switch (r4) {
c   case 0: r3 = 0; c break;
c case 1: r3 = 0; c break;
c case 2: r3 = 0; c break;
c case 3: r3 = 0; c break;
c case 4: r3 = 0; c break;
c   case 5: r5 = (UINT32)"dlm0$swpvl          # 5 dLM0_fc_swpvl - swap VLink lock"; c break;
c case 6: r3 = 0; c break;
c   case 7: r5 = (UINT32)"dlm0$chgsiz         # 7 dLM0_fc_chgsiz - Change VDisk size"; c break;
c case 8: r3 = 0; c break;
c   case 9: r5 = (UINT32)"dlm0$vdsize         # 9 dLM0_fc_vdsize - VDisk size changed"; c break;
c case 10: r3 = 0; c break;
c case 11: r3 = 0; c break;
#-----------------------------------------------------------------------------
c   case 12: r5 = (UINT32)"dlm0$chgsiz         # 12 dLM0_fc_chgsiz_GT2TB - Change VDisk size  uses same routine"; c break;
c case 13: r3 = 0; c break;
c   case 14: r5 = (UINT32)"dlm0$vdsize         # 14 dLM0_fc_vdsize_GT2TB - VDisk size changed uses same routine"; c break;
c case 15: r3 = 0; c break;
c   case 16: r5 = (UINT32)"dlm0$swpvl          # 16 dLM0_fc_swpvl_GT2TB - VDisk size changed uses same routine"; c break;
c case 17: r3 = 0; c break;
c   default: r5 = (UINT32)"unknown";
c }
c if (r3 != 0) {
c fprintf(stderr, "%s%s:%u dlm$DLM0 fc=%ld %s\n", FEBEMESSAGE, __FILE__, __LINE__, r4, (char *)r5);
c }
#-----------------------------------------------------------------------------
.endif  # PERF
        shlo    2,r4,r4                 # r4 = function code * 4
        ld      DLM0$hand(r4),r4        # r4 = request handler routine
        callx   (r4)                    # and go to request handler routine
.DLM0_1000:
        ret
#
#******************************************************************************
#
#  NAME:  DLM0$hand
#
#  PURPOSE:
#       This table contains the valid request handler routines for
#       the DLM0 datagram server.
#
#  DESCRIPTION:
#       This table contains the request handler routines for the DLM0
#       datagram server in order based on the request function code.
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
        .data
DLM0$hand:
        .word   dlm0$getmagdd       # 0 DLM0_fc_magdd - get Magnitude device database
        .word   dlm0$estvl          # 1 DLM0_fc_estvl - establish VLink
        .word   dlm0$trmvl          # 2 DLM0_fc_trmvl - terminate VLink
        .word   dlm0$vdnamechg      # 3 DLM0_fc_vdname - VDisk/VLink name changed
        .word   dlm0$magnamechg     # 4 DLM0_fc_magname - Magnitude node name changed
        .word   dlm0$swpvl          # 5 DLM0_fc_swpvl - swap VLink lock
        .word   dlm0$vquery         # 6 DLM0_fc_vquery - VDisk/VLink query
        .word   dlm0$chgsiz         # 7 DLM0_fc_chgsiz - Change VDisk size
        .word   dlm0$vlpoll         # 8 DLM0_fc_vlpoll - VLink Poll
        .word   dlm0$vdsize         # 9 DLM0_fc_vdsize - VDisk size changed
        .word   dlm0$master         # 10 DLM0_fc_master - Group Master Controller Definition
        .word   dlm0$async_nva      # 11 DLM0_fc_async_nva - Async nva received
# GT2TB VLINKs below here.
        .word   dlm0$chgsiz         # 12 DLM0_fc_chgsiz_GT2TB - Change VDisk size  GT2TB uses same routine
        .word   dlm0$getmagdd       # 13 DLM0_fc_magdd_GT2TB - get Magnitude device database GT2TB request
        .word   dlm0$vdsize         # 14 DLM0_fc_vdsize_GT2TB - VDisk size changed GT2TB uses same routine
        .word   dlm0$estvl          # 15 DLM0_fc_estvl_GT2TB - VDisk size changed GT2TB uses same routine
        .word   dlm0$swpvl          # 16 DLM0_fc_swpvl_GT2TB - VDisk size changed GT2TB uses same routine
        .word   dlm0$vquery         # 17 DLM0_fc_vquery_GT2TB - VDisk/VLink query
#
endDLM0$hand:
        .set    DLM0$maxfc,((endDLM0$hand-DLM0$hand)/4)-1 # maximum valid function code
#
        .text
#
#******************************************************************************
#  NAME: dlm0$async_nva
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = remaining request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#******************************************************************************
#
dlm0$async_nva:
        ldconst DLM0_rq_apool_nv_size,r8 # r4 = expected remaining request message length
        cmpobge g5,r8,.asnva_100        # Jif size >= expected
.asnva_50:
.ifndef PERF
c fprintf(stderr,"%s%s:%u dlm0$async_nva calling DLM srvr_invparm \n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # PERF
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .asnva_1000             # and get out of here!
#
.asnva_100:
!       ld      DLM0_rq_apool_nv_destsn(g4),r8 # r8 = destination MAG serial #
        bswap   r8,r8                   # in little-endian format
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_cserial(r3),r3       # r3 = my VCG serial #
        cmpobne r3,r8,.asnva_50         # Jif serial # not me

!       ld   DLM0_rq_apool_nvetbl+DLM0_rq_apool_nve_head_pointer(g4),r8 # apool head
!       ld   DLM0_rq_apool_nvetbl+DLM0_rq_apool_nve_tail_pointer(g4),r5 # apool tail
!       ldos DLM0_rq_apool_nv_update_type(g4),r6  # save update type, Normal update OR implicit update

.ifdef ASYNC_DLM_DEBUG
c fprintf(stderr,"%s%s:%u ASYNC:-MESSAGE RCVD - apool head:%lx, apool tail:%lx, flags:%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r8,r5,r6);
.endif  # ASYNC_DLM_DEBUG
        lda     DLM0_rq_apool_nvh(g4),r5 # Pointer to the NV header
        PushRegs(r3)                    # Push the 'g' regs on to stack
c       r4 =  AR_ReceiveNVUpdate ((APOOL_NV_IMAGE*)r5, (UINT16)r6); # All update types are handled in this function
        PopRegsVoid(r3)                 # Pop the 'g' regs from stack
        cmpobe  FALSE,r4,.asnva_950     # Jif bad seq count

        ldq     dlm$srvr_ok,r4          # r4-r7 = good response header
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
        b       .asnva_1000

.asnva_950:
        ldq     dlm$srvr_invparm,r4     # r4-r7 = bad response header
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.asnva_1000:
        ret
#
#******************************************************************************
#
#  NAME:  dlm0$getmagdd
#
#  PURPOSE:
#       Processes a "Get MAGNITUDE Device Database" datagram request.
#
#  DESCRIPTION:
#       Validates that the response buffer is adequate to return the device
#       database in and if so builds the response message in the response
#       buffer and returns the request to the caller.
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = remaining request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
dlm0$getmagdd:
        mov     0,r3                    # r3 = remaining response message length
        cmpobe  0,g5,.getmagdd_100      # Jif remaining request message length = 0
.getmagdd_50:
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .getmagdd_1000          # and get out of here!
#
.getmagdd_100:
        ldob    DLM0_rq_magdd_cl(g2),r4 # r4 = requested cluster/target #
        ldconst MAXTARGETS,r13          # r13 = maximum # targets supported
        cmpobge r4,r13,.getmagdd_50     # Jif invalid target # specified
        ld      dss1_vrp-ILTBIAS-ILTBIAS-ILTBIAS(g1),r12 # r12 = Requested VRP
        stos    r4,vr_tid(r12)          # save target ID in VRP
        call    DLM$chk_master          # check if I'm the group master controller
                                        # g0 = 0 if I'm the master
        cmpobe  0,g0,.getmagdd_120      # Jif I'm the group master
        call    DLM$srvr_reroute        # return re-route datagram response
        b       .getmagdd_1000          # and get out of here!
#
.getmagdd_120:
        mov     sp,r4                   # Allocate stack frame
        addo    16,sp,sp
        stq     g8,(r4)                 # Save g8-g11 on the stack
        movq    g0,g8                   # Save g0-g3
#
c   if (((DATAGRAM_REQ*)g10)->fc == DLM0_fc_magdd) {
        ldconst magdd_recsz,r11         # r11 = MAGDD record size
c   } else {
        ldconst magdd_recsz_GT2TB,r11   # r11 = MAGDD record size
c   }
!       stos    r11,magdd_rsize(g6)     # save record size in MAGDD
!       stos    r3,magdd_vds(g6)        # indicate no VDisks defined
        mov     magdd_hdr,r3            # r3 = remaining response message length (MAGDD header size)
        ldconst 0,r12                   # r12 = # VDisks defined
        ldconst 0,r13                   # r13 = # VDisk records returned to requestor
        mov     0,g3                    # g3 = LUN #
        ldconst LUNMAX,r15              # Number of LUNs possible
        ldconst 0,r10                   # Clear the cluster number
.getmagdd_200:
        mov     g3,g0                   # g0 = LUN being found
                                        # g1 = ILT at nest level 4
        call    dlm$find_vid            # g0 = VID # or 0xFFFFFFFF
        ldconst 0xFFFFFFFF,g2           # g2 = VID not found value
        cmpobe  g2,g0,.getmagdd_240     # Jif no VID found for this LUN
#
# Return all VDISKs (not hidden or VLinks) associated with SDD using LUN # as
#   the VDISK number
#
        ld      V_vddindx[g0*4],r8      # r8 = corresponding VDD
        cmpobe  0,r8,.getmagdd_240      # Jif no VDisk defined
        addo    1,r12,r12               # inc. # VDisks defined
        cmpobl  g7,r11,.getmagdd_240    # Jif no room for more VDisk records
        ldos    vd_attr(r8),r6          # r6 = VDisk attribute
        and     vdvdmask,r6,r4          # r4 = isolated VDisk attribute field
        cmpobe  vdhidden,r4,.getmagdd_240 # Jif hidden VDisk
        ldconst vdvlink,r4
#
# --- Check for VLink type device and don't identify them to requestor.
#
        and     r4,r6,r4
        cmpobne 0,r4,.getmagdd_240      # Jif VLink type device
#
# --- Pack MAGDD VDisk record
#
        ld      vd_rdd(r8),r9           # r9 = RDD address
        ldob    rd_type(r9),r9          # r9 = RAID type code
        cmpobe  rdslinkdev,r9,.getmagdd_240 # don't support vlink to snapshot yet
!       stob    r9,magdd_type(g6)[r3*1] # save RAID type in response record
c   if (((DATAGRAM_REQ*)g10)->fc == DLM0_fc_magdd) {
!       stob    r10,magdd_cl(g6)[r3*1]  # save base MAG cluster #
c   } else {
!       stob    r10,magdd_cl_GT2TB(g6)[r3*1] # save base MAG cluster #
c   }
#
# For Magnitude compatibility, return the LUN # as the VDisk # since
#   Magnitude cannot support an ID greater than 32 VDisks
#
!       stob    g3,magdd_vdnum(g6)[r3*1] # save VDisk #
c   if (((DATAGRAM_REQ*)g10)->fc == DLM0_fc_magdd) {
!       stob    g3,magdd_vd(g6)[r3*1]   # save base MAG VDisk #
!       stob    r6,magdd_attr(g6)[r3*1] # save VDisk attribute
c   } else {
!       stob    g3,magdd_vd_GT2TB(g6)[r3*1]   # save base MAG VDisk #
!       stob    r6,magdd_attr_GT2TB(g6)[r3*1] # save VDisk attribute
c   }
#
# -- Count number of servers assigned to this VDISK
#
c       g4 = dlm_cnt_servers(g0);
        bswap   g4,g4
        shro    16,g4,g4                # g4 = server count in big-endian format
c   if (((DATAGRAM_REQ*)g10)->fc == DLM0_fc_magdd) {
!       stos    g4,magdd_snum(g6)[r3*1] # save server count
c   } else {
!       stos    g4,magdd_snum_GT2TB(g6)[r3*1] # save server count
c   }
        ldconst 0,g4                    # g4 = VLink count
        ld      vd_vlinks(r8),g5        # g5 = first VLAR on list
.getmagdd_220:
        cmpobe  0,g5,.getmagdd_230      # Jif no more VLARs on list
        ld      vlar_link(g5),g5        # g5 = next VLAR on list
        addo    1,g4,g4                 # inc. VLink count
        b       .getmagdd_220           # and count VLARs on list
#
.getmagdd_230:
        bswap   g4,g4
        shro    16,g4,g4                # g4 = VLink count in big-endian format
c   if (((DATAGRAM_REQ*)g10)->fc == DLM0_fc_magdd) {
!       stos    g4,magdd_vlnum(g6)[r3*1] # save VLink count
c   } else {
!       stos    g4,magdd_vlnum_GT2TB(g6)[r3*1] # save VLink count
c   }
        ldconst SECSIZE,r4              # r4 = sector size
        bswap   r4,r4
        shro    16,r4,r4
!       stos    r4,magdd_secsz(g6)[r3*1] # save sector size
        ldl     vd_devcap(r8),r4        # r4,r5 = VDisk capacity
c   if (((DATAGRAM_REQ*)g10)->fc == DLM0_fc_magdd) {
# Limit VDisk capacity to 4 bytes for compatibility with Magnitude
c       if (r5 != 0) {
c           r4 = 0xffffffffUL;              # Flag that the vdisk is too big for this DLM call.
c           r5 = 0;
c       }
        bswap   r4,r4
!       st      r4,magdd_size(g6)[r3*1] # save VDisk capacity
c   } else {
!       stl     r4,magdd_size_GT2TB(g6)[r3*1] # save VDisk capacity
c   }
        ld      dgrq_dstsn(g10),r4      # r4 = base MAG serial #
        ldl     vd_name(r8),r6          # Get the name
c   if (((DATAGRAM_REQ*)g10)->fc == DLM0_fc_magdd) {
!       st      r4,magdd_sn(g6)[r3*1]   # save base MAG serial #
!       stl     r6,magdd_vdname(g6)[r3*1]
c   } else {
!       st      r4,magdd_sn_GT2TB(g6)[r3*1] # save base MAG serial #
!       stl     r6,magdd_vdname_GT2TB(g6)[r3*1]
c   }
#
# --- END pack MAGDD VDisk record
#
        addo    1,r13,r13               # inc. VDisk record count
        addo    r11,r3,r3               # inc. remaining response message length
        subo    r11,g7,g7               # sub. from remaining response buffer length
.getmagdd_240:
        addo    1,g3,g3                 # Point to the next LUN
        cmpobne g3,r15,.getmagdd_200    # Jif more LUNs to check
!       stob    r12,magdd_vds(g6)       # save # VDisks defined count
!       stob    r13,magdd_vdrecs(g6)    # save # VDisk records defined count
        ldq     dlm$srvr_ok,r4          # r4-r7 = good response header
        bswap   r3,r6                   # r6 = remaining response message length
        stq     r4,(g11)                # save response message header

        movq    g8,g0                   # Restore g0-g3
        subo    16,sp,r4
        ldq     (r4),g8                 # Restore g8-g11 from the stack
        mov     r4,sp                   # Reset the Stack Pointer

        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.getmagdd_1000:
        ret
#
#******************************************************************************
#
#  NAME:  dlm0$estvl
#
#  PURPOSE:
#       Processes an "Establish VLink" datagram request.
#
#  DESCRIPTION:
#       Determines if the request is a duplicate that was sent to
#       change the operating parameters of the original request
#       and if so updates the original operating parameters if
#       possible or returns the appropriate error if not possible.
#       Determines if a new request can be honoured and if not returns
#       the appropriate error in the response message. If the
#       request can be honoured, allocates a VLAR for the specified
#       VDisk and associates it with the VDisk.
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = remaining request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
dlm0$estvl:
        ldq     dlm$srvr_ok,r4          # r4-r7 = good response header
c   if (((DATAGRAM_REQ*)g2)->fc == DLM0_fc_estvl) {
        ldconst DLM0_rs_estvl_size,r6   # r6 = remaining response message length
c   } else {
        ldconst DLM0_rs_estvl_size_GT2TB,r6 # r6 = remaining response message length
c   }
        bswap   r6,r6
        stq     r4,(g3)                 # save response message header
        ldconst DLM0_rq_estvl_size,r4   # r4 = expected remaining request message length
        cmpobge g5,r4,.estvl_100        # Jif size >= expected
.estvl_50:
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .estvl_1000             # and get out of here!
#
.estvl_100:
c   if (((DATAGRAM_REQ*)g2)->fc == DLM0_fc_estvl) {
        ldconst DLM0_rs_estvl_size,r4   # r4 = remaining response message length
c   } else {
        ldconst DLM0_rs_estvl_size_GT2TB,r4 # r4 = remaining response message length
c   }
        cmpobl  g7,r4,.estvl_50         # Jif size < expected
!       ld      DLM0_rq_estvl_dstsn(g4),r4 # r4 = specified dest. MAG serial #
!       st      r4,DLM0_rs_estvl_basesn(g6) # save base MAG serial # in response message
        bswap   r4,r4                   # in little-endian format
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my MAG serial #
        cmpobne r3,r4,.estvl_50         # Jif serial # not me
        call    DLM$chk_master          # check if I'm the group master controller
                                        # g0 = 0 if I'm the group master
        cmpobe  0,g0,.estvl_101         # Jif I'm the group master
                                        # g0 = group master controller serial #
        call    DLM$srvr_reroute        # return re-route datagram response
        b       .estvl_1000             # and get out of here!
#
.estvl_101:
!       ldob    DLM0_rq_estvl_dstcl(g4),r5 # r5 = specified cluster #
        ldconst MAXTARGETS,r12          # r12 = maximum # targets supported
        cmpobge r5,r12,.estvl_50        # Jif invalid target # specified
!       stob    r5,DLM0_rs_estvl_basecl(g6) # save base MAG cluster # in response message
        ld      T_tgdindx+tgx_tgd[r5*4],r12  # r12 = TGD address
        cmpobe  0,r12,.estvl_50         # Jif target not defined
        ld      dss1_vrp-ILTBIAS-ILTBIAS-ILTBIAS(g1),r12 # r12 = Requested VRP
        stos    r5,vr_tid(r12)          # save target ID in VRP
#
# For Magnitude compatibility, the VDisk # sent to Magnitude was really
#   the LUN #.  Keep as a LUN for communications purposes but use the VDisk #
#   for structure lookup.
#
!       ldob    DLM0_rq_estvl_dstvd(g4),r4 # r4 = specified VDisk #
!       stob    r4,DLM0_rs_estvl_basevd(g6) # save base MAG VDisk # in response message
        ldconst 0xFFFFFFFF,r15          # VID not found value
        mov     r4,g0                   # g0 = LUN #
                                        # g1 = ILT at nest level 4
        call    dlm$find_vid            # g0 = VID # or 0xFFFFFFFF
        cmpobe  g0,r15,.estvl_50        # Jif there was no VID found
        mov     g0,r4                   # Replace r4 with the VID (was LUN)
# NOTE: g0 remains valid (as VID) till label .estvl_350 reached!
#
        ldconst MAXVIRTUALS,r15         # r15 = max. VDisk #
        cmpobge r4,r15,.estvl_50        # Jif VDisk # invalid
        ld      V_vddindx[r4*4],r15     # r15 = corresponding VDD
        cmpobe  0,r15,.estvl_50         # Jif no VDisk defined
!       ld      DLM0_rq_estvl_srcsn(g4),r14 # r14 = source MAG serial #
        bswap   r14,r14                 # in little-endian format
#
# --- Check if VDisk is a destination of a copy operation.
#       If so, prohibit VLink from being established.
#       If so, allow the copy MAG to establish a VLink regardless
#       of the copy registration state. All other MAGs must
#       check if copy operation is user/remote suspended and
#       prohibit VLink from being established if copy is not suspended.
#
        ld      vd_dcd(r15),r10         # r10 = assoc. DCD if dest. of copy operation
        cmpobe.t 0,r10,.estvl_102       # Jif no DCD assoc. with VDD
        ld      dcd_cor(r10),r11        # r11 = assoc. COR address
        ld      cor_rcsn(r11),r12       # r12 = copy MAG serial #
        cmpobe.t r14,r12,.estvl_102     # Jif requesting MAG is copy op. copy MAG
        ldob    cor_crstate(r11),r12    # r12 = COR registration state
        cmpobe.t corcrst_usersusp,r12,.estvl_102# Jif copy operation is user suspended
        cmpobne.f corcrst_remsusp,r12,.estvl_50 # Jif copy operation is not remote suspended
.estvl_102:
#
# --- Check if destination VDD is a VLink and prohibit VLink
#       from being established if so.
#
        ldos    vd_attr(r15),r11        # r11 = VDisk attribute
        bbs     vdbvlink,r11,.estvl_50  # Jif VLink
#
# --- Check if VDisk is the source of a copy&swap copy operation.
#       If so, check if the swap is to a VLink and if so prohibit
#       the VLink from being established.
#
        ld      vd_scdhead(r15),r10     # r10 = first SCD assoc. with VDD

.estvl_103:
        cmpobe  0,r10,.estvl_105        # Jif not the source of any copy ops.
        ld      scd_cor(r10),r3         # r3 = assoc. COR address
        ld      cor_cm(r3),r12          # r12 = assoc. CM address if defined
        cmpobe.t 0,r12,.estvl_104       # Jif no CM assoc. with COR indicating this copy operation is a remote one
        ld      cor_destvdd(r3),r11     # r11 = dest. VDD address
        cmpobe.f 0,r11,.estvl_50        # Jif no dest. VDD assoc. with this copy operation
                                        #  (this implies that the dest. copy device was a VLink)
        ldob    cm_type(r12),r12        # r12 = copy type code
        cmpobe.f cmty_copyswap,r12,.estvl_103d # Jif copy&swap&break copy type
        cmpobne.t cmty_mirrorswap,r12,.estvl_104 # Jif not copy&swap&mirror copy type
.estvl_103d:
        ldos    vd_attr(r11),r11        # r11 = VDisk attribute
        bbs     vdbvlink,r11,.estvl_50  # Jif VLink
.estvl_104:
        ld      scd_link(r10),r10       # r10 = next SCD on list
        b       .estvl_103
#
.estvl_105:
        ldconst FALSE,r3                # r3 = D$p2update required flag [T/F]
!       ldob    DLM0_rq_estvl_srccl(g4),r13 # r13 = source MAG cluster #
!       ldob    DLM0_rq_estvl_srcvd(g4),r12 # r12 = source MAG VDisk #
!       ldob    DLM0_rq_estvl_attr(g4),r11 # r11 = VLink attributes
!       ld      DLM0_rq_estvl_agnt(g4),r10 # r10 = transfer agent MAG serial #
        bswap   r10,r10                 #  in little-endian format
        ld      vd_vlinks(r15),r9       # r9 = first VLAR on list
#
# --- Check if VLAR already exists that's associated with this request
#
.estvl_110:
        cmpobe  0,r9,.estvl_200         # Jif no more VLARs on list
        ld      vlar_srcsn(r9),r8       # r8 = source MAG serial # in VLAR
        cmpobne r14,r8,.estvl_170       # Jif serial # does not match
        ldob    vlar_srccl(r9),r8       # r8 = source MAG cluster # in VLAR
        cmpobne r13,r8,.estvl_170       # Jif cluster # does not match
        ldob    vlar_srcvd(r9),r8       # r8 = source MAG VDisk # in VLAR
        cmpobne r12,r8,.estvl_170       # Jif VDisk # does not match
#
# --- Found a matching VLAR with requestor
#
        ld      vlar_agnt(r9),r6        # r6 = current transfer agent
        cmpobe  r10,r6,.estvl_140       # Jif transfer agent the same
        st      r10,vlar_agnt(r9)       # save transfer agent in VLAR
.estvl_140:
        ldob    vlar_attr(r9),r6        # r6 = current attributes
        cmpobe  r11,r6,.estvl_141       # Jif attributes the same
        stob    r11,vlar_attr(r9)       # save attributes
        ldconst TRUE,r3                 # set flag indicating D$p2update required
.estvl_141:
!       ldl     DLM0_rq_estvl_name(g4),r6 # r6-r7 = source device name
        ldl     vlar_name(r9),r10       # r10-r11 = current source MAG device name
        cmpobne r6,r10,.estvl_142       # Jif device name has changed
        cmpobe  r7,r11,.estvl_500       # Jif device name the same
.estvl_142:
        stl     r6,vlar_name(r9)        # save source device name in VLAR
        ldconst TRUE,r3                 # set flag indicating D$p2update required
        b       .estvl_500              # and return good response
#
.estvl_170:
        ld      vlar_link(r9),r9        # r9 = next VLAR on list
        b       .estvl_110              # check next VLAR on list if any
#
# --- No existing VLAR associated with this request
#
.estvl_200:
        ldos    vd_attr(r15),r7         # r7 = VDisk attribute
        and     vdvdmask,r7,r7          # isolate VDisk attribute field
        cmpobe  vdhidden,r7,.estvl_50   # Jif VDisk is hidden
        ld      vd_vlinks(r15),r9       # r9 = first VLAR on list
        cmpobe  0,r9,.estvl_300         # Jif no VLARs on list
        bbc     vl_excl_access,r11,.estvl_290 # Jif request is not for exclusive access
.estvl_220:
        ld      vlar_srcsn(r9),r4       # r4 = lock owner MAG serial #
        bswap   r4,r4
        st      r4,DLM0_rs_estvl_basesn(g6) # save lock owner MAG serial #
        ldob    vlar_srccl(r9),r4       # r4 = lock owner MAG serial #
        stob    r4,DLM0_rs_estvl_basecl(g6) # save lock owner MAG cluster #
        ldob    vlar_srcvd(r9),r4       # r4 = lock owner MAG VDisk #
        stob    r4,DLM0_rs_estvl_basevd(g6) # save lock owner MAG VDisk #
        ldconst SECSIZE,r4              # r4 = sector size
        bswap   r4,r4
        shro    16,r4,r4
        stos    r4,DLM0_rs_estvl_secsz(g6) # save sector size
        ldl     vd_devcap(r15),r4       # r4-r5 = VDisk capacity
c   if (((DATAGRAM_REQ*)g2)->fc == DLM0_fc_estvl) {
c       if (r5 != 0) {
c           r4 = 0xffffffffUL;              # Flag that the vdisk is too big for this DLM call.
c           r5 = 0;
c       }
        bswap   r4,r4
        st      r4,DLM0_rs_estvl_dsiz(g6) # save VDisk capacity
        ldl     vd_name(r15),r6         # Get vdisk name
        stl     r6,DLM0_rs_estvl_name(g6)
c   } else {
        stl     r4,DLM0_rs_estvl_dsiz(g6) # save VDisk capacity
        ldl     vd_name(r15),r6         # Get vdisk name
        stl     r6,DLM0_rs_estvl_name_GT2TB(g6)
c   }
#
        ldq     dlm$srvr_vlconflt,r4    # r4-r7 = VLink access conflict response header
c   if (((DATAGRAM_REQ*)g2)->fc == DLM0_fc_estvl) {
        ldconst DLM0_rs_estvl_size,r6   # r6 = remaining response message length
c   } else {
        ldconst DLM0_rs_estvl_size_GT2TB,r6 # r6 = remaining response message length
c   }
        bswap   r6,r6
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
        b       .estvl_1000             # and we're out of here!

.estvl_290:
        ldob    vlar_attr(r9),r6        # r6 = VLAR attributes
        bbs     vl_excl_access,r6,.estvl_220 # Jif VLAR specifies exclusive
.estvl_300:
        bbs     vl_excl_access,r11,.estvl_310 # Jif request is exclusive access
        cmpobe  vdnormal,r7,.estvl_350  # Jif VDisk attr. normal
.estvl_310:
#
# -- Count number of servers assigned to this VDISK
#
c       r4 = dlm_cnt_servers(r4);       # r4 = server count
        cmpobe  0,r4,.estvl_350         # Jif no servers assigned to this VDisk
        call    DLM$srvr_srconflt       # return server access conflict response
        b       .estvl_1000             # and we're out of here!
#
# --- No conflicting VLAR to this request. Allocate and initialize VLAR.
#
# NOTE: g0 still is vid at this point (and beyond);.
.estvl_350:
c       g7 = get_vlar();                # Allocate a VLAR for this request
.ifdef M4_DEBUG_VLAR
c fprintf(stderr, "%s%s:%u get_vlar 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_VLAR
# NOTE: keep r12, r13, r14 for message below (unless we change it below!).
        st      r14,vlar_srcsn(g7)      # save source MAG serial #
        stob    r13,vlar_srccl(g7)      # save source MAG cluster #
        stob    r12,vlar_srcvd(g7)      # save source MAG VDisk #
        stob    r11,vlar_attr(g7)       # save attributes
        bbc     vl_vd_format,r11,.estvl_360 # Jif MAGNITUDE format
#
# --- Bigfoot format specified. Convert 16-bit VID to VBlock/vid format.
#
        shlo    8,r13,r13
        or      r13,r12,r12             # r12 = 16-bit VID format
        mov     r12,r13
        extract 5,10,r13                # r13 = VBlock #
        and     0x1f,r12,r12            # r12 = vid #
.estvl_360:
# NOTE: keep r12 and r13 for message below.
        st      r10,vlar_agnt(g7)       # save transfer agent MAG serial #
        ldl     DLM0_rq_estvl_name(g4),r6 # r6-r7 = source device name
        stl     r6,vlar_name(g7)        # save source device name in VLAR
        ldob    DLM0_rq_estvl_dstcl(g4),r9 # r9 = dest. target #
        ldob    DLM0_rq_estvl_dstvd(g4),r6 # r6 = dest. LUN #
        shlo    8,r9,r9
        or      r6,r9,r9                # r9 = target/LUN combo
        stos    r9,vlar_repvd(g7)       # Save target/LUN combo in VLAR
        ld      vd_vlinks(r15),r9       # r9 = first VLAR on list
        st      r9,vlar_link(g7)        # link VLAR onto top of list
        st      g7,vd_vlinks(r15)       # save new list head member
        ldos    vd_attr(r15),r6         # r6 = VDisk attribute byte
        setbit  vdbvdlock,r6,r6         # set VLink/VDisk lock flag
        stos    r6,vd_attr(r15)         # save updated attribute byte
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
# --- Send message to CCB of the good news!
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       r8 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        stos    g0,log_dlm_ccb_vl_dstvd(r8) # Save vid
        st      r14,log_dlm_ccb_vl_srcsn(r8) # source MAG serial number
        stob    r13,log_dlm_ccb_vl_srccl(r8)
        stos    r12,log_dlm_ccb_vl_srcvd(r8)
        ldconst mleestvlink,r3          # Established a VLink message
        stos    r3,mle_event(r8)        # Type of message
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], log_dlm_ccb_vl_size);
        ldconst TRUE,r3                 # indicate D$p2update required
#
# ---
#
        movq    g0,r4                   # save g0-g3
        ld      vd_scdhead(r15),r8      # r8 = first SCD assoc. with VDD
        cmpobe.t 0,r8,.estvl_450        # Jif no SCDs assoc. with VDD
.estvl_400:
        ld      scd_cor(r8),g3          # g3 = assoc. COR address
        ld      cor_cm(g3),r9           # r9 = assoc. CM address
        cmpobne.f 0,r9,.estvl_420       # Jif a local COR defined
        ld      cor_rcsn(g3),g0         # g0 = copy MAG serial #
        call    CM$pkop_dmove           # pack a copy device moved datagram
                                        # g1 = Copy Device Moved datagram ILT at nest level 2
        ldconst 4,g0                    # g0 = error retry count
        call    DLM$just_senddg         # send datagram to copy MAG
.estvl_420:
        ld      scd_link(r8),r8         # r8 = next SCD assoc. with VDD
        cmpobne.t 0,r8,.estvl_400       # Jif more SCDs assoc. with VDD
.estvl_450:
        ld      vd_dcd(r15),r8          # r8 = DCD assoc. with VDD
        cmpobe.t 0,r8,.estvl_490        # Jif no DCDs assoc. with VDD
        ld      dcd_cor(r8),g3          # g3 = assoc. COR address
        ld      cor_cm(g3),r8           # r8 = assoc. CM address
        cmpobne.f 0,r8,.estvl_490       # Jif a local COR defined
        ld      cor_rcsn(g3),g0         # g0 = copy MAG serial #
        call    CM$pkop_dmove           # pack a copy device moved datagram
                                        # g1 = Copy Device Moved datagram ILT at nest level 2
        ldconst 4,g0                    # g0 = error retry count
        call    DLM$just_senddg         # send datagram to copy MAG
.estvl_490:
        movq    r4,g0                   # restore g0-g3

.estvl_500:
        ldconst SECSIZE,r4              # r4 = sector size
        bswap   r4,r4
        shro    16,r4,r4
!       stos    r4,DLM0_rs_estvl_secsz(g6) # save sector size
        ldl     vd_name(r15),r6         # Get the vdisk name
#
# Limit capacity to 4 bytes for Magnitude compatibility
#
        ldl     vd_devcap(r15),r4       # r4-r5 = VDisk capacity
c   if (((DATAGRAM_REQ*)g2)->fc == DLM0_fc_estvl) {
c       if (r5 != 0) {
c           r4 = 0xffffffffUL;          # Flag that the vdisk is too big for this DLM call.
c           r5 = 0;
c       }
        bswap   r4,r4
!       st      r4,DLM0_rs_estvl_dsiz(g6) # save VDisk capacity
!       stl     r6,DLM0_rs_estvl_name(g6)
c   } else {
!       stl     r4,DLM0_rs_estvl_dsiz_GT2TB(g6) # save VDisk capacity
!       stl     r6,DLM0_rs_estvl_name_GT2TB(g6)
c   }
        ldos    vd_vid(r15),r6          # r6 = VID of VDisk
        stob    r6,DLM0_rs_estvl_altid+1(g3) # save LSB of VID in response
        shro    8,r6,r7                 # r7 = MSB of VID
        setbit  7,r7,r7                 # set flag indicating valid alternate ID
        stob    r7,DLM0_rs_estvl_altid(g3) # save MSB of VID in response
#
# End limit of capacity to 4 bytes
#
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
        cmpobne TRUE,r3,.estvl_1000     # Jif D$p2update not required
        call    D$p2updateconfig        # update NVRAM
#
        call    D$SendRefreshNV         # refresh NVRAM on slave controllers
#
.estvl_1000:
        ret
#
#******************************************************************************
#
#  NAME:  dlm0$trmvl
#
#  PURPOSE:
#       Processes a "Terminate VLink" datagram request.
#
#  DESCRIPTION:
#       Locates the associated VLAR and terminates it.
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = remaining request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
dlm0$trmvl:
        ldconst DLM0_rq_trmvl_size,r4   # r4 = expected remaining request message length
        cmpobge g5,r4,.trmvl_100        # Jif size >= expected
.trmvl_50:
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .trmvl_1000             # and get out of here!
#
.trmvl_100:
!       ld      DLM0_rq_trmvl_dstsn(g4),r4 # r4 = specified dest. MAG serial #
        bswap   r4,r4                   # in little-endian format
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my VCG serial #
        cmpobne r3,r4,.trmvl_50         # Jif serial # not me
        call    DLM$chk_master          # check if I'm the group master controller
                                        # g0 = 0 if I'm the group master
        cmpobe  0,g0,.trmvl_101         # Jif I'm the group master
                                        # g0 = group master controller serial #
        call    DLM$srvr_reroute        # return re-route datagram response
        b       .trmvl_1000             # and get out of here!
#
.trmvl_101:
!       ldob    DLM0_rq_trmvl_dstcl(g4),r5 # r5 = specified cluster #
        ldconst MAXTARGETS,r12          # r12 = maximum # targets supported
        cmpobge r5,r12,.trmvl_50        # Jif invalid target # specified
        ld      dss1_vrp-ILTBIAS-ILTBIAS-ILTBIAS(g1),r12 # r12 = Requested VRP
        stos    r5,vr_tid(r12)          # save target ID in VRP
#
# For Magnitude compatibility, the VDisk # sent to Magnitude was really
#   the LUN #.  Keep as a LUN for communications purposes but use the VDisk #
#   for structure lookup.
#
!       ldob    DLM0_rq_trmvl_dstvd(g4),r4 # r4 = specified VDisk #
        ldconst 0xFFFFFFFF,r15          # VID not found constant
        mov     r4,g0                   # g0 = LUN #
                                        # g1 = ILT at nest level 4
        call    dlm$find_vid            # g0 = VID # or 0xFFFFFFFF
        cmpobe  g0,r15,.trmvl_50        # Jif there was no VID found
        mov     g0,r4                   # Replace r4 with the VID (was LUN)
# NOTE: g0 and r4 remain intact until message is created.
        ldconst MAXVIRTUALS,r15         # r15 = max. VDisk #
        cmpobge r4,r15,.trmvl_50        # Jif VDisk # invalid
        ld      V_vddindx[r4*4],r15     # r15 = corresponding VDD
        cmpobe  0,r15,.trmvl_50         # Jif no VDisk defined
!       ld      DLM0_rq_trmvl_srcsn(g4),r14 # r14 = source MAG serial #
        bswap   r14,r14                 # in little-endian format
!       ldob    DLM0_rq_trmvl_srccl(g4),r13 # r13 = source MAG cluster #
!       ldob    DLM0_rq_trmvl_srcvd(g4),r12 # r12 = source MAG VDisk #
# NOTE: r12, 13, 14 remain intact until message (unless changed below!).
        ld      vd_vlinks(r15),r9       # r9 = first VLAR on list
        lda     vd_vlinks(r15),r7       # r7 = "last VLAR" processed
        b       .trmvl_110              # Check if VLAR exists for this request

.trmvl_109:
        mov     r9,r7                   # r7 = previous VLAR processed
        ld      vlar_link(r9),r9        # r9 = next VLAR on list
#
# --- Check if VLAR already exists that's associated with this request
#
.trmvl_110:
        cmpobe  0,r9,.trmvl_200         # Jif no more VLARs on list
        ld      vlar_srcsn(r9),r8       # r8 = source MAG serial # in VLAR
        cmpobne r14,r8,.trmvl_109       # Jif serial # does not match
        ldob    vlar_srccl(r9),r8       # r8 = source MAG cluster # in VLAR
        cmpobne r13,r8,.trmvl_109       # Jif cluster # does not match
        ldob    vlar_srcvd(r9),r8       # r8 = source MAG VDisk # in VLAR
        cmpobne r12,r8,.trmvl_109       # Jif VDisk # does not match
#
# --- Found a matching VLAR with requestor
#
# --- Check if any copy operations associated with virtual device
#       and if so force them into a suspended mode of operation.
#
        mov     r9,g7                   # g7 = VLAR being terminated
        mov     r15,g6                  # g6 = VDD assoc. with VLAR
        call    CM$whack_rcor           # suspend any copy ops. assoc.

        ld      vlar_link(r9),r8        # r8 = next VLAR on list
        st      r8,vlar_link(r7)        # remove VLAR from list
        cmpobne 0,r8,.trmvl_120         # Jif at least one VLAR still associated with VDisk
        ldos    vd_attr(r15),r8         # r8 = attribute byte
        clrbit  vdbvdlock,r8,r8         # clear VDisk/VLink lock flag
        stos    r8,vd_attr(r15)         # save updated attribute byte
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
.trmvl_120:
        mov     r9,g7                   # g7 = VLAR being terminated
        ldob    vlar_attr(g7),r8        # r8 = attribute byte
.ifdef M4_DEBUG_VLAR
c fprintf(stderr, "%s%s:%u put_vlar 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_VLAR
c       put_vlar(g7);                   # Deallocate VLAR
        call    D$p2updateconfig        # update NVRAM
#
# --- Send message to CCB of the bad news!
#
        bbc     vl_vd_format,r8,.trmvl_160 # Jif MAGNITUDE format
#
# --- Bigfoot format specified. Convert 16-bit VID to VBlock/vid format.
#
        shlo    8,r13,r13
        or      r13,r12,r12             # r12 = 16-bit VID format
        mov     r12,r13
        extract 5,10,r13                # r13 = VBlock #
        and     0x1f,r12,r12            # r12 = vid #
.trmvl_160:
# NOTE: g0, r4, r12,r13,r14 are 100+ lines above or r12 r13 r14 are just above.
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       r5 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        stos    r4,log_dlm_ccb_vl_dstvd(r5) # save vid number
        st      r14,log_dlm_ccb_vl_srcsn(r5) # source MAG serial number
        stob    r13,log_dlm_ccb_vl_srccl(r5) # save cluster # for CCB
        stos    r12,log_dlm_ccb_vl_srcvd(r5) # save VDisk # for CCB
        ldconst mletermvlink,r3         # Terminate a VLink message
        stos    r3,mle_event(r5)        # Type of message
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], log_dlm_ccb_vl_size);
#
        call    D$SendRefreshNV         # refresh NVRAM on slave controllers
#
.trmvl_200:
        ldq     dlm$srvr_ok,r4          # r4-r7 = good response header
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.trmvl_1000:
        ret
#
#******************************************************************************
#
#  NAME:  dlm0$vdnamechg
#
#  PURPOSE:
#       Processes a "VDisk Name Changed" datagram request.
#
#  DESCRIPTION:
#       Searches all LDDs associated with specified VDisk and
#       changes the device name where appropriate. Searches all
#       VDisk/VLinks for associated VLink matching the specified
#       VDisk/VLink and changes the device name where necessary.
#
#  CALLING SEQUENCE:
#       call    dlm0$vdnamechg
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = remaining request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
dlm0$vdnamechg:
        ldconst DLM0_rq_vdchg_size,r4   # r4 = expected remaining request message length
        cmpobge g5,r4,.vdchg_100        # Jif size >= expected
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .vdchg_1000             # and get out of here!
#
.vdchg_100:
        call    DLM$chk_master          # check if I'm the group master controller
                                        # g0 = 0 if I'm the group master
        cmpobe  0,g0,.vdchg_120         # Jif I'm the group master
                                        # g0 = group master controller serial #
        call    DLM$srvr_reroute        # return re-route datagram response
        b       .vdchg_1000             # and get out of here!
#
.vdchg_120:
        ldconst FALSE,r3                # r3 = update NVRAM flag
#
# --- Scan all LDDs to see if they are associated with the
#       specified VDisk/VLink name change.
#
        ld      DLM0_rq_vdchg_srcsn(g4),r4 # r4 = source MAG serial #
        ldob    DLM0_rq_vdchg_srccl(g4),r5 # r5 = source MAG cluster #
        ldob    DLM0_rq_vdchg_srcvd(g4),r6 # r6 = source MAG VDisk #
        bswap   r4,r4
        ldl     DLM0_rq_vdchg_basename(g4),r8 # r8-r9 = base MAG device name
        ldob    DLM0_rq_vdchg_type(g4),g0 # g0 = name change type code
        cmpobe  0,g0,.vdchg_150         # Jif all types
        cmpobne 2,g0,.vdchg_310         # Jif not VDisk-to-VLink type
.vdchg_150:
        lda     DLM_lddindx,r10         # r10 = pointer to LDD table
        ldconst MAXLDDS,r11             # r11 = max. # LDDs supported
.vdchg_200:
        ld      (r10),r7                # r7 = LDD address
        subo    1,r11,r11               # dec. LDD count
        cmpobe  0,r7,.vdchg_300         # Jif no LDD defined
        ldob    ld_class(r7),r15        # r15 = Linked device class
        cmpobne ldmld,r15,.vdchg_300    # Jif not MAGNITUDE link device class
        ld      ld_basesn(r7),r15       # r15 = source MAG serial #
        cmpobne r15,r4,.vdchg_300       # Jif wrong MAG serial #
        ldob    ld_basecl(r7),r15       # r15 = source MAG cluster #
        cmpobne r15,r5,.vdchg_300       # Jif wrong MAG cluster #
        ldos    ld_basevd(r7),r15       # r15 = source MAG VDisk #
        cmpobne r15,r6,.vdchg_300       # Jif wrong VDisk #
#
# --- Found a matching LDD
#
        ldos    ld_ord(r7),g0           # g0 = LDD ordinal #
        ldconst 0xFFFFFFFF,r12          # r12 = VID not Found Flag
        call    dlm$ldd_vid             # Get the VID associated with this LDD
                                        # g0 = VID or 0xFFFFFFFF if not found
        cmpobne r12,g0,.vdchg_210       # Jif a VID was found
        ldconst dlm_sft9,r14            # r14 = error code to log
        lda     dlm_sft,g0              # g0 = Software Fault Log Area
        st      r14,efa_ec(g0)          # Save the Error Code
        st      r7,efa_data(g0)         # Save the LDD Pointer
        st      r4,efa_data+4(g0)       # Save the Source Controller Serial #
        st      r5,efa_data+8(g0)       # Save the Source Cluster Number
        st      r6,efa_data+12(g0)      # Save the Source VDisk Number
        ldconst 20,r14                  # Number of bytes saved (ec + data)
        st      r14,mle_len(g0)         # Save the number of bytes to send
        call    M$soft_flt              # Error Trap or Log failure
        b       .vdchg_300              # VID not found - ignore LDD
#
.vdchg_210:
        ldl     ld_basename(r7),r12     # Get the current name
        cmpobne r8,r12,.vdchg_220       # Jif names are not the same
        cmpobe  r9,r13,.vdchg_230       # Jif the names are the same
.vdchg_220:
        ldconst TRUE,r3                 # set NVRAM update flag
        stl     r8,ld_basename(r7)      # Save the base MAG name
#
        movl    g0,r8
        ldos    ld_ord(r7),g0           # Set input parm (LDD ordinal)
        ldconst ecnvlink,g1
        call    D$changename            # Post it
        movl    r8,g0                   # Restore g0/g1
#
.vdchg_230:
        ld      DLM0_rq_vdchg_basesn(g4),r12 # r12 = base MAG serial #
        ld      ld_basesn(r7),r13       # get current base MAG serial number
        bswap   r12,r12                 #  in little-endian format
        cmpobe  r12,r13,.vdchg_240      # Jif there is no change
        ldconst TRUE,r3                 # set NVRAM update flag
        st      r12,ld_basesn(r7)       # save base MAG serial #
.vdchg_240:
        ldob    DLM0_rq_vdchg_basecl(g4),r12 # r12 = base MAG cluster #
        ldob    ld_basecl(r7),r13       # get current MAG cluster number
        cmpobe  r12,r13,.vdchg_250      # Jif there is no change
        ldconst TRUE,r3                 # set NVRAM update flag
        stob    r12,ld_basecl(r7)       # save base MAG cluster #
.vdchg_250:
        ldob    DLM0_rq_vdchg_basevd(g4),r12 # r12 = base MAG VDisk #
        ldob    ld_basevd(r7),r13       # get current MAG VDisk number
        cmpobe  r12,r13,.vdchg_300      # Jif there is no change
        ldconst TRUE,r3                 # set NVRAM update flag
        stos    r12,ld_basevd(r7)       # save base MAG VDisk #
.vdchg_300:
        addo    4,r10,r10               # inc. to next LDD in table
        cmpobne 0,r11,.vdchg_200        # Jif more LDDs to check for
.vdchg_310:
        ldob    DLM0_rq_vdchg_type(g4),g0 # g0 = name change type code
        cmpobe  2,g0,.vdchg_900         # Jif VDisk-to-VLink type
#
# --- Scan all VDisk/VLinks to see if the specified VDisk/VLink
#       is linked to them.
#
        ldl     DLM0_rq_vdchg_srcname(g4),r8 # r8-r9 = source MAG device name
        ldconst MAXVIRTUALS,r10         # r10 = max. # virtuals to process for each cluster
        mov     0,r12                   # r12 = vid
.vdchg_330:
        ld      V_vddindx[r12*4],r11    # r11 = VDD for vid
        addo    1,r12,r12               # inc. vid #
        cmpobe  0,r11,.vdchg_400        # Jif no VDD defined
        ld      vd_vlinks(r11),r14      # r14 = first VLAR on list
.vdchg_340:
        cmpobe  0,r14,.vdchg_400        # Jif no VLARs on list
        ld      vlar_srcsn(r14),r15     # r15 = source MAG serial #
        cmpobne r4,r15,.vdchg_350       # Jif wrong MAG serial #
        ldob    vlar_srccl(r14),r15     # r15 = source MAG cluster #
        cmpobne r5,r15,.vdchg_350       # Jif wrong MAG cluster #
        ldob    vlar_srcvd(r14),r15     # r15 = source MAG VDisk #
        cmpobne r6,r15,.vdchg_350       # Jif wrong MAG VDisk #
        stl     r8,vlar_name(r14)       # save device name in VLAR
        ldconst TRUE,r3                 # set NVRAM update flag
.vdchg_350:
        ld      vlar_link(r14),r14      # r14 = next VLAR on list
        b       .vdchg_340              # and check for match on next VLAR
#
.vdchg_400:
        cmpobne r12,r10,.vdchg_330      # Jif more VDDs to check & process
.vdchg_900:
        ldq     dlm$srvr_ok,r4          # r4-r7 = good response header
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
        cmpobe  FALSE,r3,.vdchg_1000    # Jif nothing changed to update NVRAM for
        call    D$p2updateconfig        # update NVRAM
#
        call    D$SendRefreshNV         # refresh NVRAM on slave controllers
#
.vdchg_1000:
        ret
#
#******************************************************************************
#
#  NAME:  dlm0$magnamechg
#
#  PURPOSE:
#       Processes a "MAGNITUDE Node Name Changed" datagram request.
#
#  DESCRIPTION:
#       Identifies the data structures affected and updates them
#       accordingly.
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = remaining request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
dlm0$magnamechg:
        ldconst DLM0_rq_nnchg_size,r4   # r4 = expected remaining request message length
        cmpobge g5,r4,.magchg_100       # Jif size >= expected
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .magchg_1000            # and get out of here!
#
.magchg_100:
!       ld      DLM0_rq_nnchg_sn(g4),r13 # r13 = MAG serial #
        ld      dlm_mlmthd,r4           # r4 = first MLMT on list
!       ldl     DLM0_rq_nnchg_name(g4),r14 # r14-r15 = MAG node name
        ld      vl_sulst,r11            # r11 = the pointer to the SUL area
        ldconst 0xFFFFFFFF,r12          # r12 = Send D$changename flag => FALSE
        bswap   r13,r13                 # Make serial number usable
#
# --- Update associated DTMTs & Storage Unit List
#
.magchg_140:
        cmpobe  0,r4,.magchg_300        # Jif no MLMTs on list
        mov     r4,r5                   # r5 = MLMT being processed
        ld      mlmt_sn(r4),r6          # r6 = MAG serial # for MLMT
        ld      mlmt_link(r4),r4        # r4 = next MLMT on list
        cmpobne r6,r13,.magchg_140      # Jif MAG serial # does not match
#
# --- Found the MLMT matching the incoming request.
#
        ld      mlmt_dtmthd(r5),r4      # r4 = first DTMT on list
        movt    0,r8                    # Clearing registers
        cmpobe  0,r4,.magchg_300        # Jif no DTMTs on list
.magchg_180:
        ldos    dtmt_sulindx(r4),r6     # r6 = SUL index value
        stl     r14,dml_pname(r4)       # save MAG node name in DTMT
        cmpobe  0,r6,.magchg_200        # Jif DTMT not registered in SUL area
        stl     r14,vl_sulr_name(r11)[r6*1] # save MAG node name in SUL
        stt     r8,vl_sulr_name+8(r11)[r6*1] #  record and clear rest of bytes
        mov     r6,r12                  # Save SUL index to send D$changename
#
.magchg_200:
        ld      dtmt_alias_dtmt(r4),r3  # r3 = assoc. alias DTMT address
        cmpobe  0,r3,.magchg_250        # Jif no alias DTMT assoc. with this primary DTMT
        ldos    dtmt_sulindx(r3),r6     # r6 = SUL index value
        stl     r14,dml_pname(r3)       # save MAG node name in DTMT
        cmpobe  0,r6,.magchg_250        # Jif DTMT not registered in SUL area
        stl     r14,vl_sulr_name(r11)[r6*1] # save MAG node name in SUL
        stt     r8,vl_sulr_name+8(r11)[r6*1] #  record and clear rest of bytes
        mov     r6,r12                  # Save SUL index to send D$changename
#
.magchg_250:
        ld      dml_mllist(r4),r4       # r4 = next DTMT on list
        cmpobne 0,r4,.magchg_180        # Jif more DTMTs to process
#
# --- If a name change needs to be sent to the CCB about the controller name
#       then let it know.  The CCB will then come down and get all the updated
#       information.
#
.magchg_300:
        ldconst 0xFFFFFFFF,r8           # Determine if a valid SUL index to send
        cmpobe  r8,r12,.magchg_320      # Jif CCB does not need to be informed
        movl    g0,r8
        mov     r12,g0                  # Last SUL index that was changed
        ldconst ecnctrl,g1              # Controller Name Changed
        call    D$changename            # Post it
        movl    r8,g0                   # Restore g0/g1
#
.magchg_320:
        ldq     dlm$srvr_ok,r4          # r4-r7 = good response header
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.magchg_1000:
        ret
#
#******************************************************************************
#
#  NAME:  dlm0$swpvl
#
#  PURPOSE:
#       Processes a "Swap VLink Lock" datagram request.
#
#  DESCRIPTION:
#       Validates if the specified current VLink lock owner still
#       exists and if so swaps the specified source MAG VLink
#       in as the new VLink lock owner. If the VLink lock no longer
#       exists, treats the request the same as an Establish VLink
#       request. If some other VLink owns the lock, rejects it the
#       same as for an Establish VLink request.
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = remaining request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
dlm0$swpvl:
        ldconst DLM0_rq_swpvl_size,r4   # r4 = expected remaining request message length
        cmpobge g5,r4,.swpvl_100        # Jif size >= expected
.swpvl_50:
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .swpvl_1000             # and get out of here!
#
.swpvl_100:
c if (((DATAGRAM_REQ*)g2)->fc == DLM0_fc_swpvl) {
        ldconst DLM0_rs_swpvl_size,r4   # r4 = expected remaining response message length
c } else {
        ldconst DLM0_rs_swpvl_size_GT2TB,r4 # r4 = expected remaining response message length
c }
        cmpobl  g7,r4,.swpvl_50         # Jif size < expected
!       ld      DLM0_rq_swpvl_dstsn(g4),r4 # r4 = specified dest. MAG serial #
!       st      r4,DLM0_rs_swpvl_basesn(g6) # save base MAG serial # in response message
        bswap   r4,r4                   # in little-endian format
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my VCG serial #
        cmpobne r3,r4,.swpvl_50         # Jif serial # not me
        call    DLM$chk_master          # check if I'm the group master controller
                                        # g0 = 0 if I'm the group master
        cmpobe  0,g0,.swpvl_120         # Jif I'm the group master
                                        # g0 = group master controller serial #
        call    DLM$srvr_reroute        # return re-route datagram response
        b       .swpvl_1000             # and get out of here!
#
.swpvl_120:
!       ldob    DLM0_rq_swpvl_dstcl(g4),r5 # r5 = specified cluster #
        ldconst MAXTARGETS,r12          # r12 = maximum # targets supported
        cmpobge r5,r12,.swpvl_50        # Jif invalid target # specified
!       stob    r5,DLM0_rs_swpvl_basecl(g6) # save base MAG cluster # in response message
        ld      dss1_vrp-ILTBIAS-ILTBIAS-ILTBIAS(g1),r12 # r12 = Requested VRP
        stos    r5,vr_tid(r12)          # save target ID in VRP
#
# For Magnitude compatibility, the VDisk # sent to Magnitude was really
#   the LUN #.  Keep as a LUN for communications purposes but use the VDisk #
#   for structure lookup.
#
!       ldob    DLM0_rq_swpvl_dstvd(g4),r4 # r4 = specified VDisk #
!       stob    r4,DLM0_rs_swpvl_basevd(g6) # save base MAG VDisk # in response message
        ldconst 0xFFFFFFFF,r15          # VID not found value
        mov     r4,g0                   # g0 = LUN #
                                        # g1 = ILT at nest level 4
        call    dlm$find_vid            # g0 = VID # or 0xFFFFFFFF
        cmpobe  g0,r15,.swpvl_50        # Jif no VID found for this LUN
        mov     g0,r4                   # Replace r4 with the VID (was LUN)
# NOTE: g0 is VID, and used in a message way below.
        ldconst MAXVIRTUALS,r15         # r15 = max. VDisk #
        cmpobge r4,r15,.swpvl_50        # Jif VDisk # invalid
        ld      V_vddindx[r4*4],r15     # r15 = corresponding VDD
        cmpobe  0,r15,.swpvl_50         # Jif no VDisk defined
        ld      vd_vlinks(r15),r9       # r9 = first VLAR on list
#
# --- Check if VLAR already exists that's associated with this request
#
        cmpobne 0,r9,.swpvl_300         # Jif VLARs on list
#
# --- No VLARs associated with VDisk
#
.swpvl_200:
        b       dlm0$estvl              # treat the same as establish VLink request
#
# --- Check top VLAR for owner match.
#
.swpvl_300:
!       ld      DLM0_rq_swpvl_orgsn(g4),r4 # r4 = specified owner serial #
        ld      vlar_srcsn(r9),r5       # r5 = owner MAG serial # from VLAR
        bswap   r4,r4
        cmpobne r4,r5,.swpvl_200        # Jif serial #'s don't match
!       ldob    DLM0_rq_swpvl_orgcl(g4),r4 # r4 = specified owner cluster #
        ldob    vlar_srccl(r9),r5       # r5 = owner MAG cluster # from VLAR
        cmpobne r4,r5,.swpvl_200        # Jif cluster #'s don't match
!       ldob    DLM0_rq_swpvl_orgvd(g4),r4 # r4 = specified owner VDisk #
        ldob    vlar_srcvd(r9),r5       # r5 = owner MAG VDisk # from VLAR
        cmpobne r4,r5,.swpvl_200        # Jif VDisk #'s don't match
#
# --- Swap VLink lock ownership.
#
!       ld      DLM0_rq_swpvl_srcsn(g4),r14 # r14 = source MAG serial #
        bswap   r14,r14                 # in little-endian format
!       ldob    DLM0_rq_swpvl_srccl(g4),r13 # r13 = source MAG cluster #
!       ldob    DLM0_rq_swpvl_srcvd(g4),r12 # r12 = source MAG VDisk #
!       ldob    DLM0_rq_swpvl_attr(g4),r11 # r11 = VLink attributes
!       ld      DLM0_rq_swpvl_agnt(g4),r10 # r10 = transfer agent MAG serial #
        bswap   r10,r10                 #  in little-endian format
        st      r14,vlar_srcsn(r9)      # save new owner serial # in VLAR
# NOTE: r14 saved in message a ways below (so may r12 and r13, if not changed).
        stob    r13,vlar_srccl(r9)      # save new owner cluster # in VLAR
        stob    r12,vlar_srcvd(r9)      # save new owner VDisk # in VLAR
        stob    r11,vlar_attr(r9)       # save new owner attributes in VLAR
        bbc     vl_vd_format,r11,.swpvl_360 # Jif MAGNITUDE format
#
# --- Bigfoot format specified. Convert 16-bit VID to VBlock/vid format.
#
        shlo    8,r13,r13
        or      r13,r12,r12             # r12 = 16-bit VID format
        mov     r12,r13
        extract 5,10,r13                # r13 = VBlock #
        and     0x1f,r12,r12            # r12 = vid #
.swpvl_360:
# NOTE: r12 and r13 used in message a ways below.
        st      r10,vlar_agnt(r9)       # save transfer agent in VLAR
!       ldl     DLM0_rq_swpvl_name(g4),r6 # r6-r7 = source device name
        stl     r6,vlar_name(r9)        # save source device name in VLAR
        ldconst SECSIZE,r4              # r4 = sector size
        bswap   r4,r4
        shro    16,r4,r4
!       stos    r4,DLM0_rs_swpvl_secsz(g6) # save sector size
        ldl     vd_devcap(r15),r4       # r4-r5 = VDisk capacity
c   if (((DATAGRAM_REQ*)g2)->fc == DLM0_fc_swpvl) {
c       if (r5 != 0) {
c           r5 = 0;
c           r4 = 0xffffffff;            # Limit to 32 bits.
c       }
        bswap   r4,r4
!       st      r4,DLM0_rs_swpvl_dsiz(g6) # save VDisk capacity
        ldl     vd_name(r15),r6         # Get the name
!       stl     r6,DLM0_rs_swpvl_name(g6)
        ldq     dlm$srvr_ok,r4          # r4-r7 = good response header
        ldconst DLM0_rs_swpvl_size,r6   # r6 = remaining response message length
c   } else {
        stl     r4,DLM0_rs_swpvl_dsiz_GT2TB(g6) # Save VDisk capacity
        ldl     vd_name(r15),r6         # Get the name
!       stl     r6,DLM0_rs_swpvl_name_GT2TB(g6)
        ldq     dlm$srvr_ok,r4          # r4-r7 = good response header
        ldconst DLM0_rs_swpvl_size_GT2TB,r6 # r6 = remaining response message length
c   }
        bswap   r6,r6
        ldos    vd_vid(r15),r5          # r5 = alternate ID
        setbit  15,r5,r5                # set alternate ID flag
        bswap   r5,r5
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
# NOTE: g0 is VID quite a way above.
        mov     g0,r5                   # Save g0 (VID) in r5 for message below.
        callx   (r4)                    # call ILT completion handler routine
        call    D$p2updateconfig        # update NVRAM
#
# --- Send message to CCB of the good news!
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       r4 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        stos    r5,log_dlm_ccb_vl_dstvd(r4) # save vid for message
        st      r14,log_dlm_ccb_vl_srcsn(r4) # save serial number for message
        stob    r13,log_dlm_ccb_vl_srccl(r4) # save cluster number for CCB
        stos    r12,log_dlm_ccb_vl_srcvd(r4) # save VDisk number for CCB
        ldconst mleswpvlink,r3          # Swap a VLink message
        stos    r3,mle_event(r4)        # Type of message
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], log_dlm_ccb_vl_size);
#
        call    D$SendRefreshNV         # refresh NVRAM on slave controllers
#
.swpvl_1000:
        ret
#
#******************************************************************************
#
#  NAME:  dlm0$vquery
#
#  PURPOSE:
#       Processes a "VDisk/VLink Query" datagram request.
#
#  DESCRIPTION:
#       Validates the incoming request parameters and returns the
#       type of VDisk/VLink device (or undefined if none) along
#       with information about the virtual device. It also indicates
#       if the device has a lock applied and if it does the owner
#       of the lock.
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = remaining request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
dlm0$vquery:
        ldconst DLM0_rq_vqury_size,r4   # r4 = expected remaining request message length
        cmpobge g5,r4,.vquery_100       # Jif size >= expected
.vquery_50:
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .vquery_1000            # and get out of here!
#
.vquery_100:
c if (((DATAGRAM_REQ*)g2)->fc == DLM0_fc_vquery) {
        ldconst DLM0_rs_vqury_size,r4   # r4 = expected remaining response message length
c } else {
        ldconst DLM0_rs_vqury_size_GT2TB,r4 # r4 = expected remaining response message length
c }
        cmpobl  g7,r4,.vquery_50        # Jif size < expected
!       ld      DLM0_rq_vqury_sn(g4),r4 # r4 = specified MAG serial #
        bswap   r4,r4                   # in little-endian format
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my VCG serial #
        cmpobne r3,r4,.vquery_50        # Jif serial # not me
        call    DLM$chk_master          # check if I'm the group master controller
                                        # g0 = 0 if I'm the group master
        cmpobe  0,g0,.vquery_110        # Jif I'm the group master
                                        # g0 = group master controller serial #
        call    DLM$srvr_reroute        # return re-route datagram response
        b       .vquery_1000            # and get out of here!
#
.vquery_110:
!       ldob    DLM0_rq_vqury_cl(g4),r5 # r5 = MSB VDisk #
!       ldob    DLM0_rq_vqury_vd(g4),r4 # r4 = LSB VDisk #
        shlo    8,r5,r5
        or      r5,r4,r4                # r4 = specified VDisk #
        ldconst MAXVIRTUALS,r15         # r15 = max. VDisk #
        cmpobge r4,r15,.vquery_50       # Jif VDisk # invalid
        ld      V_vddindx[r4*4],r15     # r15 = corresponding VDD
        cmpobne 0,r15,.vquery_150       # Jif VDisk defined
#
# --- VDisk/VLink not defined response handler
#
        ldconst vqury_ty_undef,r4       # r4 = undefined device type code
.vquery_130:
        st      r4,DLM0_rs_vqury_type(g6) # save device type code and clear lock, attributes, reserved byte #0
        b       .vquery_900             # and complete response processing
#
.vquery_150:
        ldob    D_Vflag,r14             # r14 = VDisk/VLink busy flag
        cmpobe  FALSE,r14,.vquery_155   # Jif VDisk/VLink not busy
        ldos    D_Vvid,r14              # r14 = VDisk # that's being worked on
        cmpobne r14,r4,.vquery_155      # Jif different VDisk #
        ldconst vqury_ty_busy,r4        # r4 = VDisk/VLink busy type code
        b       .vquery_130             # and return the response to the requestor
#
.vquery_155:
        ld      vd_rdd(r15),r5          # r5 = RDD of first RAID segment
        ldob    rd_type(r5),r6          # r6 = RAID type code of first RAID segment
        mov     0,r3                    # r3 = null LDD address
        ldconst vqury_ty_vdisk,r4       # r4 = VDisk type code
        cmpobne rdlinkdev,r6,.vquery_160 # Jif not linked device RAID type
        ldconst vqury_ty_vlink,r4       # r4 = VLink type code
        ld      rd_psd(r5),r6           # r6 = first PSD address
        ldos    ps_pid(r6),r3           # r3 = assoc. LDD index
        ld      DLM_lddindx[r3*4],r3    # r3 = assoc. LDD address
.vquery_160:
!       st      r4,DLM0_rs_vqury_type(g6) # save device type code and clear lock, attributes, reserved byte #0
        ldconst SECSIZE,r4              # r4 = sector size
        bswap   r4,r4
        shro    16,r4,r4
!       st      r4,DLM0_rs_vqury_secsz(g6) # save sector size and clear reserved bytes #1 & #2
        ldl     vd_devcap(r15),r4       # r4-r5 = VDisk capacity
c   if (((DATAGRAM_REQ*)g2)->fc == DLM0_fc_vquery) {
# Limit the capacity to 4 bytes for Magnitude compatibility
c       if (r5 != 0) {
c           r5 = 0;
c           r4 = 0xffffffff;            # Limit to 32 bits.
c       }
        bswap   r4,r4
!       st      r4,DLM0_rs_vqury_dsiz(g6) # save VDisk capacity
c   } else {
!       stl     r4,DLM0_rs_vqury_dsiz_GT2TB(g6) # save VDisk capacity
c   }
        movq    g0,r8                   # save g0-g3
        ldos    vd_attr(r15),r4         # r4 = attributes
!       stos    r4,DLM0_rs_vqury_attr(g6) # save attributes
        ldl     vd_name(r15),r6         # Get the VDisk Name
c   if (((DATAGRAM_REQ*)g2)->fc == DLM0_fc_vquery) {
!       stl     r6,DLM0_rs_vqury_name(g6)
c   } else {
!       stl     r6,DLM0_rs_vqury_name_GT2TB(g6)
c   }
        cmpobe  0,r3,.vquery_200        # Jif no LDD (no link device)
#
# --- Pack VLink related information
#
        ld      ld_basesn(r3),r4        # r4 = MAG serial #
        bswap   r4,r4                   # in big-endian format
c   if (((DATAGRAM_REQ*)g2)->fc == DLM0_fc_vquery) {
!       st      r4,DLM0_rs_vqury_magsn(g6) # save source MAG serial #
!       st      r4,DLM0_rs_vqury_basesn(g6) # save base MAG serial #
        ldob    ld_basecl(r3),r4        # r4 = MAG cluster #
!       stob    r4,DLM0_rs_vqury_magcl(g6) # save source MAG cluster #
!       stob    r4,DLM0_rs_vqury_basecl(g6) # save base MAG cluster #
        ldos    ld_basevd(r3),r4        # r4 = MAG VDisk #
!       stob    r4,DLM0_rs_vqury_magvd(g6) # save source MAG VDisk #
!       stob    r4,DLM0_rs_vqury_basevd(g6) # save base MAG VDisk #
        ldl     ld_basename(r3),r4      # Get the name
!       stl     r4,DLM0_rs_vqury_basename(g6) # save base MAG device name
c   } else {
!       st      r4,DLM0_rs_vqury_magsn_GT2TB(g6) # save source MAG serial #
!       st      r4,DLM0_rs_vqury_basesn_GT2TB(g6) # save base MAG serial #
        ldob    ld_basecl(r3),r4        # r4 = MAG cluster #
!       stob    r4,DLM0_rs_vqury_magcl_GT2TB(g6) # save source MAG cluster #
!       stob    r4,DLM0_rs_vqury_basecl_GT2TB(g6) # save base MAG cluster #
        ldos    ld_basevd(r3),r4        # r4 = MAG VDisk #
!       stob    r4,DLM0_rs_vqury_magvd_GT2TB(g6) # save source MAG VDisk #
!       stob    r4,DLM0_rs_vqury_basevd_GT2TB(g6) # save base MAG VDisk #
        ldl     ld_basename(r3),r4      # Get the name
!       stl     r4,DLM0_rs_vqury_basename_GT2TB(g6) # save base MAG device name
c   }
.vquery_200:
        movq    r8,g0                   # restore g0-g3
        ld      vd_vlinks(r15),r9       # r9 = first VLAR on list
        cmpobe  0,r9,.vquery_900        # Jif no VLARs assoc. with device
#
# --- Pack VDisk/VLink lock related information
#
        ldconst TRUE,r4
        stob    r4,DLM0_rs_vqury_lock(g6) # indicate lock exists
        ld      vlar_srcsn(r9),r4       # r4 = source MAG serial #
        bswap   r4,r4                   # in big-endian format
c   if (((DATAGRAM_REQ*)g2)->fc == DLM0_fc_vquery) {
        st      r4,DLM0_rs_vqury_vlsn(g6) # save source MAG serial #
        ldob    vlar_srccl(r9),r4       # r4 = source MAG cluster #
        st      r4,DLM0_rs_vqury_vlcl(g6) # save source MAG cluster # and clear reserved byte #3
        ldob    vlar_srcvd(r9),r4       # r4 = source MAG VDisk #
        stob    r4,DLM0_rs_vqury_vlvd(g6) # save source MAG VDisk #
        ldob    vlar_attr(r9),r4        # r4 = lock attributes byte
        stob    r4,DLM0_rs_vqury_vlattr(g6) # save lock attributes byte
        ldl     vlar_name(r9),r4        # r4-r5 = source MAG device name
        stl     r4,DLM0_rs_vqury_vlname(g6) # save source MAG device name
        ld      vlar_agnt(r9),r4        # r4 = transfer agent MAG serial #
        st      r4,DLM0_rs_vqury_vlagnt(g6) # save transfer agent serial #
c   } else {
        st      r4,DLM0_rs_vqury_vlsn_GT2TB(g6) # save source MAG serial #
        ldob    vlar_srccl(r9),r4       # r4 = source MAG cluster #
        st      r4,DLM0_rs_vqury_vlcl_GT2TB(g6) # save source MAG cluster # and clear reserved byte #3
        ldob    vlar_srcvd(r9),r4       # r4 = source MAG VDisk #
        stob    r4,DLM0_rs_vqury_vlvd_GT2TB(g6) # save source MAG VDisk #
        ldob    vlar_attr(r9),r4        # r4 = lock attributes byte
        stob    r4,DLM0_rs_vqury_vlattr_GT2TB(g6) # save lock attributes byte
        ldl     vlar_name(r9),r4        # r4-r5 = source MAG device name
        stl     r4,DLM0_rs_vqury_vlname_GT2TB(g6) # save source MAG device name
        ld      vlar_agnt(r9),r4        # r4 = transfer agent MAG serial #
        st      r4,DLM0_rs_vqury_vlagnt_GT2TB(g6) # save transfer agent serial #
c   }
.vquery_900:
        ldq     dlm$srvr_ok,r4          # r4-r7 = good response header
c   if (((DATAGRAM_REQ*)g2)->fc == DLM0_fc_vquery) {
        ldconst DLM0_rs_vqury_size,r6   # r6 = remaining response message length
c   } else {
        ldconst DLM0_rs_vqury_size_GT2TB,r6 # r6 = remaining response message length
c   }
        bswap   r6,r6
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.vquery_1000:
        ret
#
#******************************************************************************
#
#  NAME:  dlm0$chgsiz
#
#  PURPOSE:
#       Processes a "Change VDisk Size" datagram request.
#
#  DESCRIPTION:
#       Validates that the specified destination VDisk exists and
#       has the specified source XIOtech Controller lock applied. If not,
#       rejects the request back to the requestor. If everything
#       specified is valid, changes the size of the VDisk and saves
#       the changes off to NVRAM.
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = remaining request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
dlm0$chgsiz:
c if (((DATAGRAM_REQ*)g2)->fc == DLM0_fc_chgsiz) {
        ldconst DLM0_rq_chgsz_size,r4   # r4 = expected remaining request lth
c } else {
        ldconst DLM0_rq_chgsz_size_GT2TB,r4 # r4 = expected remaining request lth
c }
        cmpobge g5,r4,.chgsiz_100       # Jif size >= expected
.chgsiz_50:
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .chgsiz_1000            # and get out of here!
#
.chgsiz_100:
        ld      DLM0_rq_chgsz_dstsn(g4),r4 # r4 = destination MAG serial #
        bswap   r4,r4                   # in little-endian format
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my VCG serial #
        cmpobne r3,r4,.chgsiz_50        # Jif serial # not me
        call    DLM$chk_master          # check if I'm the group master controller
                                        # g0 = 0 if I'm the group master
        cmpobe  0,g0,.chgsiz_120        # Jif I'm the group master
                                        # g0 = group master controller serial #
        call    DLM$srvr_reroute        # return re-route datagram response
        b       .chgsiz_1000            # and get out of here!
#
.chgsiz_120:
        ldob    DLM0_rq_chgsz_dstcl(g4),r5 # r5 = specified cluster #
        ldconst MAXTARGETS,r12          # r12 = maximum # targets supported
        cmpobge r5,r12,.chgsiz_50       # Jif invalid target # specified
        ld      dss1_vrp-ILTBIAS-ILTBIAS-ILTBIAS(g1),r12 # r12 = Requested VRP
        stos    r5,vr_tid(r12)          # save target ID in VRP
#
# For Magnitude compatibility, the VDisk # sent to Magnitude was really
#   the LUN #.  Keep as a LUN for communications purposes but use the VDisk #
#   for structure lookup.
#
        ldob    DLM0_rq_chgsz_dstvd(g4),r4 # r4 = destination VDisk #
        ldconst 0xFFFFFFFF,r15          # VID not found value
        mov     r4,g0                   # g0 = LUN #
                                        # g1 = ILT at nest level 4
        call    dlm$find_vid            # g0 = VID # or 0xFFFFFFFF
        cmpobe  g0,r15,.chgsiz_50       # Jif no VID found for this LUN
        mov     g0,r4                   # Replace r4 with the VID (was LUN)
#
        ldconst MAXVIRTUALS,r15         # r15 = max. VDisk #
        cmpobge r4,r15,.chgsiz_50       # Jif VDisk # invalid
        ld      V_vddindx[r4*4],r15     # r15 = corresponding VDD
        cmpobe  0,r15,.chgsiz_50        # Jif no VDisk defined
        ld      vd_vlinks(r15),r14      # r14 = assoc. VLAR address
        cmpobe  0,r14,.chgsiz_50        # Jif no VLink assoc. with VDisk
        ld      DLM0_rq_chgsz_srcsn(g4),r4 # r4 = specified source MAG serial #
        bswap   r4,r4                   # in little-endian format
        ld      vlar_srcsn(r14),r5      # r5 = MAG serial # of lock owner
        cmpobne r4,r5,.chgsiz_50        # Jif wrong lock owner specified
        ldob    DLM0_rq_chgsz_srccl(g4),r4 # r4 = specified source MAG cluster #
        ldob    vlar_srccl(r14),r5      # r5 = MAG cluster # of lock owner
        cmpobne r4,r5,.chgsiz_50        # Jif wrong lock owner specified
        ldob    DLM0_rq_chgsz_srcvd(g4),r4 # r4 = specified source MAG Vlink #
        ldob    vlar_srcvd(r14),r5      # r5 = MAG VLink # of lock owner
        cmpobne r4,r5,.chgsiz_50        # Jif wrong lock owner specified
c if (((DATAGRAM_REQ*)g2)->fc == DLM0_fc_chgsiz) {
        ld      DLM0_rq_chgsz_dsiz(g4),r4 # r4-r5 = specified new VDisk size
        bswap   r4,r4                   # in little-endian format
        ldconst 0,r5                    # Upper 32 bits of capacity is zero.
c } else {
        ldl     DLM0_rq_chgsz_dsiz_GT2TB(g4),r4 # r4-r5 = specified new VDisk size
c }
        ldl     vd_devcap(r15),r6       # r6-r7 = current VDisk size
c if ((*(UINT64 *)&r4) == (*(UINT64 *)&r6)) {
            b   .chgsiz_900             # Jif same size specified
c }
c if ((*(UINT64 *)&r4) > (*(UINT64 *)&r6)) {
            b   .chgsiz_50              # Jif new size > current size
c }
#
# --- Lock Owner has been validated and specified new size < current size
#
# --- Check if specified virtual device is involved in any copy
#       operations and if so reject the request.
#
        ld      vd_scdhead(r15),r13     # r13 = first SCD assoc. with VDD
        cmpobe.t 0,r13,.chgsiz_300      # Jif no SCDs assoc. with VDD
.chgsiz_200:
        call    CMsp$srvr_inuse         # pack and send device in use response message and return it to requestor
        b       .chgsiz_1000
#
.chgsiz_300:
        ld      vd_dcd(r15),r13         # r13 = DCD assoc. with VDD
        cmpobne.f 0,r13,.chgsiz_200     # Jif DCD assoc. with VDD
        stl     r4,vd_devcap(r15)       # save new capacity in VDD
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        call    D$p2updateconfig        # update NVRAM with changes
#
# --- Notify associated path(s) that a VDisk change has occurred.
#
        movl    g0,r8                   # save g0-g1
        ldconst 0,g1                    # g1 = working path #
        ldconst 0x01,g0                 # g0 = target flags byte for SRP
        call    dlm$send_stf            # pack and send a Set Target Flags SRP
        movl    r8,g0                   # restore g0-g1
#
        call    D$SendRefreshNV         # refresh NVRAM on slave controllers
#
.chgsiz_900:
        ldq     dlm$srvr_ok,r4          # r4-r7 = good response header
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.chgsiz_1000:
        ret
#
#******************************************************************************
#
#  NAME:  dlm0$vlpoll
#
#  PURPOSE:
#       Processes a "VLink Poll" datagram request.
#
#  DESCRIPTION:
#       Validates that the specified destination VDisk exists and
#       has the specified source XIOtech Controller lock applied. If not,
#       returns a status back indicating the state of the virtual
#       device.
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = remaining request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
dlm0$vlpoll:
        ldconst DLM0_rq_vlpol_size,r4   # r4 = expected remaining request message length
        cmpobge g5,r4,.vlpoll_100       # Jif size >= expected
.vlpoll_50:
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .vlpoll_1000            # and get out of here!
#
.vlpoll_100:
!       ld      DLM0_rq_vlpol_dstsn(g4),r4 # r4 = destination MAG serial #
        bswap   r4,r4                   # in little-endian format
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my VCG serial #
        cmpobne r3,r4,.vlpoll_50        # Jif serial # not me
        call    DLM$chk_master          # check if I'm the group master controller
                                        # g0 = 0 if I'm the group master
        cmpobe  0,g0,.vlpoll_120        # Jif I'm the group master
                                        # g0 = group master controller serial #
        call    DLM$srvr_reroute        # return re-route datagram response
        b       .vlpoll_1000            # and get out of here!
#
.vlpoll_120:
        ldconst 0,r8                    # r8 = alternate ID register
!       ldob    DLM0_rq_vlpol_dstcl(g4),r5 # r5 = specified cluster #
        ldconst MAXTARGETS,r12          # r12 = maximum # targets supported
        cmpobge r5,r12,.vlpoll_50       # Jif invalid target # specified
        ld      dss1_vrp-ILTBIAS-ILTBIAS-ILTBIAS(g1),r12 # r12 = Requested VRP
        stos    r5,vr_tid(r12)          # save target ID in VRP
        ldconst 0x01,r3                 # r3 = VDisk does not exist return status code
#
# For Magnitude compatibility, the VDisk # sent to Magnitude was really
#   the LUN #.  Keep as a LUN for communications purposes but use the VDisk #
#   for structure lookup.
#
!       ldob    DLM0_rq_vlpol_dstvd(g4),r4 # r4 = destination VDisk #
        ldconst 0xFFFFFFFF,r15          # VID not found value
        mov     r4,g0                   # g0 = LUN #
                                        # g1 = ILT at nest level 4
        call    dlm$find_vid            # g0 = VID # or 0xFFFFFFFF
        cmpobe  g0,r15,.vlpoll_50       # Jif no VID found for this LUN
        mov     g0,r4                   # Replace r4 with the VID (was LUN)
        mov     g0,r8                   # r8 = alternate ID
#
        ldconst MAXVIRTUALS,r15         # r15 = max. VDisk #
        cmpobge r4,r15,.vlpoll_50       # Jif VDisk # invalid
        ld      V_vddindx[r4*4],r15     # r15 = corresponding VDD
        cmpobe  0,r15,.vlpoll_900       # Jif no VDisk defined
        ldconst 0x03,r3                 # r3 = VDisk exists, not locked return status code
        ld      vd_vlinks(r15),r14      # r14 = assoc. VLAR address
        cmpobe  0,r14,.vlpoll_900       # Jif no VLink assoc. with VDisk
        ldconst 0x02,r3                 # r3 = VDisk locked by other VLink return status code
.vlpoll_200:
!       ld      DLM0_rq_vlpol_srcsn(g4),r4 # r4 = specified source MAG serial #
        bswap   r4,r4                   # in little-endian format
        ld      vlar_srcsn(r14),r5      # r5 = MAG serial # of lock owner
        cmpobne r4,r5,.vlpoll_500       # Jif wrong lock owner specified
!       ldob    DLM0_rq_vlpol_srccl(g4),r4 # r4 = specified source MAG cluster #
        ldob    vlar_srccl(r14),r5      # r5 = MAG cluster # of lock owner
        cmpobne r4,r5,.vlpoll_500       # Jif wrong lock owner specified
!       ldob    DLM0_rq_vlpol_srcvd(g4),r4 # r4 = specified source MAG Vlink #
        ldob    vlar_srcvd(r14),r5      # r5 = MAG VLink # of lock owner
        cmpobne r4,r5,.vlpoll_500       # Jif wrong lock owner specified
#
# --- Lock Owner has been validated as the requestor.
#
        ldconst 0x00,r3                 # r3 = VLink locked by requestor return status code
        stob    r3,vlar_poll(r14)       # clear VLink poll count in VLAR
        b       .vlpoll_900             # and return response to requestor
#
.vlpoll_500:
        ld      vlar_link(r14),r14      # r14 = next VLAR on list
        cmpobne 0,r14,.vlpoll_200       # Jif more VLARs on list
.vlpoll_900:
        ldq     dlm$srvr_ok,r4          # r4-r7 = good response header
        cmpobne 0,r3,.vlpoll_920        # Jif VLink not locked by requestor
        setbit  15,r8,r8                # set alternate ID flag
        bswap   r8,r8
        or      r8,r3,r3                # add alternate ID to status byte
.vlpoll_920:
        mov     r3,r5                   # r5 = VLink status code
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.vlpoll_1000:
        ret
#
#******************************************************************************
#
#  NAME:  dlm0$vdsize
#
#  PURPOSE:
#       Processes a "VDisk Size Changed" datagram request.
#
#  DESCRIPTION:
#       Validates that the specified destination VLink exists and
#       is associated with the specified source VDisk. If not,
#       returns a status back indicating the specified source VDisk
#       not associated with the specified destination VLink. If it
#       is associated with the VDisk, checks if the VLink is the
#       source of a copy operation and if so rejects the request.
#       It then checks if the VLink is the destination of a copy
#       operation and if it is checks if the new VDisk size is less
#       then the current size of the VLink. If it is, it rejects the
#       request. If not, it returns good completion status to the
#       requestor.
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = remaining request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
dlm0$vdsize:
c if (((DATAGRAM_REQ*)g2)->fc == DLM0_fc_vdsize) {
        ldconst DLM0_rq_vdsize_size,r4  # r4 = expected remaining request length
c } else {
        ldconst DLM0_rq_vdsize_size_GT2TB,r4 # r4 = expected remaining request lth GT2TB
c }
        cmpobge g5,r4,.vdsize_100       # Jif size >= expected
.vdsize_50:
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .vdsize_1000            # and get out of here!
#
.vdsize_100:
        ld      DLM0_rq_vdsize_dstsn(g4),r4 # r4 = destination Controller s/n
        bswap   r4,r4                   # in little-endian format
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my VCGserial #
        cmpobne r3,r4,.vdsize_50        # Jif serial # not me
        call    DLM$chk_master          # check if I'm the group master controller
                                        # g0 = 0 if I'm the group master
        cmpobe  0,g0,.vdsize_120        # Jif I'm the group master
                                        # g0 = group master controller serial #
        call    DLM$srvr_reroute        # return re-route datagram response
        b       .vdsize_1000            # and get out of here!
#
.vdsize_120:
        ldob    DLM0_rq_vdsize_dstcl(g4),r5 # r5 = MSB destination VDisk #
        ldconst 0x03,r3                 # r3 = no VLink assoc. with the specified VDisk status code
        shlo    8,r5,r5
        ldob    DLM0_rq_vdsize_dstvd(g4),r4 # r4 = LSB destination VDisk #
        ldconst MAXVIRTUALS,r15         # r15 = max. VDisk #
        or      r5,r4,r4                # r4 = destination VDisk #
        cmpobge r4,r15,.vdsize_50       # Jif VDisk # invalid
#
        mov     r4,r13                  # Save vid for message later.
        ld      V_vddindx[r4*4],r15     # r15 = corresponding VDD
        cmpobe  0,r15,.vdsize_900       # Jif no VDisk defined
        ld      vd_rdd(r15),r8          # r8 = RDD address of the first RAID segment
        ldob    rd_type(r8),r6          # r6 = RAID type code of first RAID segment
        cmpobne rdlinkdev,r6,.vdsize_900 # Jif not linked device RAID type
        ld      rd_psd(r8),r6           # r6 = assoc. PSD address
        ldos    ps_pid(r6),r9           # r9 = assoc. LDD index
        ld      DLM_lddindx[r9*4],r9    # r9 = assoc. LDD address
        cmpobe  0,r9,.vdsize_900        # Jif no LDD assoc. in table
        ldob    ld_class(r9),r4         # r4 = device class
        cmpobne ldmld,r4,.vdsize_900    # Jif not XIOtech Link device
        ld      DLM0_rq_vdsize_srcsn(g4),r4 # r4 = specified source XIOtech Controller serial #
        bswap   r4,r4                   # in little-endian format
        ld      ld_basesn(r9),r5        # r5 = XIOtech controller serial # of VDisk for VLink
        cmpobne r4,r5,.vdsize_900       # Jif wrong Controller s/n specified
        mov     r4,r12                  # save serial number for message later.
        ldob    DLM0_rq_vdsize_srccl(g4),r4 # r4 = specified source cluster #
        ldob    ld_basecl(r9),r5        # r5 = XIOtech controller cluster # of VDisk for VLink
        cmpobne r4,r5,.vdsize_900       # Jif wrong cluster # specified
        mov     r4,r11                  # Save source cluster # for message later.
        ldob    DLM0_rq_vdsize_srcvd(g4),r4 # r4 = specified source XIOtech controller VDisk #
        ldob    ld_basevd(r9),r5        # r5 = MAG VDisk # for VLink
        cmpobne r4,r5,.vdsize_900       # Jif wrong VDisk # specified
        mov     r4,r10                  # Save source vid for message later.
#
# --- VLink/VDisk association has been validated as the requestor.
#
        ldconst 0x01,r3                 # r3 = VLink the source of copy op.  acceptance code
        ld      vd_scdhead(r15),r4      # r4 = first SCD of copy list
        cmpobne 0,r4,.vdsize_900        # Jif VLink is the source of a copy operation
        ldconst 0x02,r3                 # r3 = VLink the dest. of copy op.  acceptance code
        ldl     vd_devcap(r15),r6       # r6-r7 = current device capacity
c   if (((DATAGRAM_REQ*)g2)->fc == DLM0_fc_vdsize) {
        cmpobne 0,r7,.vdsize_900        # Jif current size > 4 byte field (2tb)
        ld      DLM0_rq_vdsize_devcap(g4),r4 # r4 = new VDisk size in
c       r5 = 0;                         # Upper value of VDisk size
        bswap   r4,r4                   # r5 = new VDisk size in little-endian format
c } else {
        ldl     DLM0_rq_vdsize_devcap_GT2TB(g4),r4 # r4/r5 = new VDisk size
c }
c if (((VDD *)r15)->pDCD != 0) {        # if destination of a copy op.
c    if (*((UINT64 *)&r4) < *((UINT64 *)&r6)) {
        b       .vdsize_900             # Jif new VDisk size < current size
c    }
c }
c if (*((UINT64 *)&r4) == *((UINT64 *)&r6)) {
        b       .vdsize_490             # Jif device capacity unchanged
c }
# Vdisk size increased.
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
c       ((VDD *)r15)->devCap = *(UINT64 *)&r4;  # Save less than 2tb capacity into VDD
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
c       ((RDD *)r8)->devCap = *(UINT64 *)&r4;   # Save capcity in RDD
c       ((RDD *)r8)->extension.pPSD[0]->sLen = *(UINT64 *)&r4; # Save segment length of PSD
c       ((LDD *)r9)->devCap = *(UINT64 *)&r4;   # Save new device capacity in LDD
        call    D$p2updateconfig        # update NVRAM
#
# Notify each path about the change
#
        movl    g0,r4                   # save g0 and g1
        ldconst 0x01,g0                 # g0 = target set flags word
        ldconst 0,g1                    # Path to send the Set Target Flags
.vdsize_450:
        call    dlm$send_stf            # Set the Target Flags word
        addo    1,g1,g1                 # Point to the next path
        cmpobne MAXISP,g1,.vdsize_450   # Jif not all done
        movl    r4,g0                   # restore g0 and g1
#
# --- Send message to the CCB of the good news!
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       r4 = (UINT32)&TmpStackMessage[0];   # Address of temporary message.
        stob    r10,log_dlm_ccb_vl_srcvd(r4) # save source vid in message
        stob    r11,log_dlm_ccb_vl_srccl(r4) # save source cluster # in message
        st      r12,log_dlm_ccb_vl_srcsn(r4) # save serial number for CCB
        stos    r13,log_dlm_ccb_vl_dstvd(r4) # save VDisk number for CCB
        ldconst mlevlszchg,r5           # VLink Size Changed
        stos    r5,mle_event(r4)        # Type of message
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], log_dlm_ccb_vl_size);
#
#- no g register changes in D$SendRefreshNV
        call    D$SendRefreshNV         # refresh NVRAM on slave controllers
#
.vdsize_490:
        ldconst 0,r3                    # r3 = change accepted by recipient acceptance code
.vdsize_900:
        ldq     dlm$srvr_ok,r4          # r4-r7 = good response header
        mov     r3,r5                   # r5 = change acceptance code
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.vdsize_1000:
        ret
#
#******************************************************************************
#
#  NAME:  dlm0$master
#
#  PURPOSE:
#       Processes a "Group Master Controller Definition" datagram request.
#
#  DESCRIPTION:
#       Validates whether it is a member of the specified group. If not, it
#       finds the associated MLMT of the specified group and saves the specified
#       group master controller in it. If it is a member of the specified group,
#       it checks if it was the group master controller and if it was will clear
#       the I am master flag. It saves the specified master controller as the
#       current master controller in the DLM_master_sn field.
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = remaining request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
dlm0$master:
        ldconst DLM0_rq_master_size,r4  # r4 = expected remaining request message length
        cmpobge g5,r4,.master_100       # Jif size >= expected
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .master_1000            # and get out of here!
#
.master_100:
        ld      K_ficb,r3               # r3 = FICB address
!       ld      DLM0_rq_master_gpsn(g4),r4 # r4 = group serial #
!       ld      DLM0_rq_master_mastsn(g4),r5 # r5 = master controller serial #
        bswap   r4,r4                   # in little-endian format
        ld      fi_vcgid(r3),r6         # r6 = my group #
        bswap   r5,r5                   # in little-endian format
        cmpobe  r6,r4,.master_500       # Jif group serial # is me
#
# --- Specified group serial # is not my group.
#
        mov     r4,g0                   # g0 = group serial # to locate MLMT for
        call    DLM$find_controller     # find assoc. MLMT
        cmpobe  0,g0,.master_900        # Jif no MLMT found for the group
        st      r5,mlmt_master(g0)      # save master controller serial # for the group in group's MLMT
        b       .master_900
#
# --- Specified group serial # is my group.
#
.master_500:
        st      r5,DLM_master_sn        # save my group master controller serial #
.master_900:
        ldq     dlm$srvr_ok,r4          # r4-r7 = good response header
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.master_1000:
        ret
#
#******************************************************************************
# ____________________________ SUBROUTINES ____________________________________
#
#******************************************************************************
#
#  NAME:  dlm$find_vid
#
#  PURPOSE:
#       Finds the local VID based on the Remotes LUN # in the Request
#
#  DESCRIPTION:
#       Based on the LUN and DTMT associated with this request, the Server ID
#       is found and then the LUN is converted to the VID.
#
#  INPUT:
#       g0 = LUN # from the Datagram Request
#       g1 = datagram ILT at nest level #4 for Datagram Messages only
#
#  OUTPUT:
#       g0 = VID Number or 0xFFFFFFFF if not found or invalid
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#******************************************************************************
#
dlm$find_vid:
        movq    g0,r8                   # Save g0-g3
                                        # r8 = LUN #
                                        # r9 = ILT
        ldconst 0xFFFFFFFF,r4           # Invalid SID/VID constant
        mov     r4,g3                   # Set up as VID not found
#
# --- Find the DTMT associated with this datagram request
#
        ld      dss3_rqhdr_ptr-ILTBIAS(r9),r15 # Point to the Dg Req Header
        ld      dgrq_srcsn(r15),r13     # r13 = Source XIOtech serial number
        bswap   r13,r13                 # convert to correct endian
        ld      dlm_mlmthd,r12          # r12 = first MLMT on list
.findvid_100:
        cmpobe  0,r12,.findvid_1000     # Jif no MLMTs on list
        ld      mlmt_sn(r12),r14        # r14 = MLMT XIOtech serial #
        cmpobe  r13,r14,.findvid_200    # Jif MLMT for source XIOtech identified
        ld      mlmt_link(r12),r12      # r12 = next MLMT on list
        b       .findvid_100            # and check next MLMT for match
#
.findvid_200:
        ld      dss1_vrp-ILTBIAS-ILTBIAS-ILTBIAS(r9),r3 # r3 = Requested VRP
        ldos    vr_tid(r3),g2           # g2 = Target ID to search
        ld      vr_mrm_dlmid(r3),r3     # r3 = DTMT assoc with this path
        cmpobne 0,r3,.findvid_400       # Jif DTMT defined
        ld      mlmt_dtmthd(r12),r3     # r3 = alternate DTMT to use
        cmpobe  0,r3,.findvid_1000      # Jif no DTMTs assoc. with MLMT
#
# --- Get requestors WWN and Path to determine which SDD to look at
#
.findvid_400:
        ldl     dtmt_pwwn(r3),g0        # g0-g1 = WWN of Requester
                                        # g2 = Path #
        ldconst FALSE,g3                # g3 = Ignore new servers

        PushRegs(r3)
        ldconst 0,g4                    # g4 = iSCSI name
        call    DEF_WWNLookup
        mov     g0,r4
        PopRegsVoid(r3)
        mov     r4,g3

        ldconst 0xFFFFFFFF,r4           # Invalid SID/VID constant
        cmpobe  r4,g3,.findvid_1000     # Jif there was no server found
#
# --- SDD found, get the information about the VDisk associated with the LUN
#
        ld      S_sddindx[g3*4],g0      # g0 = SDD address
        mov     r8,g1                   # g1 = LUN #
        call    D$lunlookup             # g3 = VID # or 0xFFFFFFFF
.findvid_1000:
        mov     g3,r8                   # Replace g0 with the VID (was LUN)
        movq    r8,g0                   # Restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME:  dlm$ldd_vid
#
#  PURPOSE:
#       Finds the local VID based on the LDD ordinal passed in
#
#  DESCRIPTION:
#       Based on the LDD ordinal passed in, the VID is found that is associated
#       with this LDD.
#
#  INPUT:
#       g0 = LDD Ordinal
#
#  OUTPUT:
#       g0 = VID Number or 0xFFFFFFFF if not found or invalid
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#******************************************************************************
#
dlm$ldd_vid:
        ldconst MAXVIRTUALS,r15         # r15 = total number of VIDs to search
        mov     g0,r14                  # r14 = the LDD Ordinal
        ldconst 0,g0                    # g0 = VID being investigated
.lddvid_100:
        ld      V_vddindx[g0*4],r3      # r3 = VDD address
        cmpobe  0,r3,.lddvid_900        # Jif there is no VDD
        ld      vd_rdd(r3),r4           # r4 = assoc. RDD address
        ldob    rd_type(r4),r5          # r5 = RAID type code
        cmpobne rdlinkdev,r5,.lddvid_900 # Jif not a linked device type RAID
        ld      rd_psd(r4),r5           # r5 = assoc. PSD address
        ldos    ps_pid(r5),r6           # r6 = assoc. LDD index
        cmpobe  r14,r6,.lddvid_1000     # Jif the LDD Ordinals match - Done
.lddvid_900:
        addo    1,g0,g0                 # g0 = next VID to investigate
        cmpobne g0,r15,.lddvid_100      # Jif not all the VIDs have been checked
        ldconst 0xFFFFFFFF,g0           # No matching VID found
.lddvid_1000:
        ret
#
#******************************************************************************
#
#  NAME:  dlm$upsul
#
#  PURPOSE:
#       Updates the Storage Unit List maintained for the CCB.
#
#  DESCRIPTION:
#       Checks if a Storage Unit List record associated with the
#       specified DTMT. If not, one is allocated. Updates the
#       information in the Storage Unit List record for the CCB.
#
#  INPUT:
#       g4 = DTMT to update
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#******************************************************************************
#
dlm$upsul:
#
# --- Identify if target is ME!
#
        ldob    dtmt_type(g4),r4        # r4 = target type code
        cmpobne dtmt_ty_MAG,r4,.upsul_70 # Jif not MAGNITUDE target
        ld      K_ficb,r3               # r3 = FICB address
        ld      dml_sn(g4),r4           # r4 = XIOtech Cntr target serial #
        ld      fi_cserial(r3),r5       # r5 = my controller serial #
        cmpobe  r5,r4,.upsul_50         # Jif me
        ld      fi_vcgid(r3),r5         # r5 = my VCG serial #
        cmpobne r5,r4,.upsul_60         # Jif not me
#
# --- Target is me. Don't show target in Storage Unit List area.
#
.upsul_50:
        ldconst 0,r4
        stos    r4,dtmt_sulindx(g4)     # clear SUL index
        b       .upsul_1000             # and we're out of here!
#
.upsul_60:
        ldob    dml_cl(g4),r4           # r4 = assigned cluster #
        cmpobe  0x0f,r4,.upsul_50       # Jif unassigned cluster #
        ldconst 0xff,r5
        cmpobe.f r4,r5,.upsul_50        # Jif unassigned cluster #
.upsul_70:
        ld      vl_sulst,r4             # r4 = base address of Storage Unit List
        ldos    dtmt_sulindx(g4),r5     # r5 = SUL index assigned to DTMT
        cmpobne 0,r5,.upsul_100         # Jif SUL record assigned to DTMT
#
# --- Check if storage controller already registered in table
#
        ldos    vl_sulh_cnt(r4),r5      # r5 = SUL record count
        lda     vl_sulh_siz(r4),r6      # r6 = record pointer
        ldconst vl_sulr_siz,r7          # r7 = record size
        ldl     dtmt_pwwn(g4),r10       # r10-r11 = port WWN of specified DTMT
.upsul_80:
        cmpobe  0,r5,.upsul_90          # Jif no more records to check
        ld      vl_sulr_dtmt(r6),r8     # r8 = record assoc. DTMT address
        cmpobne 0,r8,.upsul_83          # Jif a DTMT assoc. with record
        ldl     vl_sulr_mac(r6),r12     # r12-r13 = SUL WWN
        cmpobne r10,r12,.upsul_85       # Jif the SUL and DTMT WWNs differ
        cmpobne r11,r13,.upsul_85       # Jif the SUL and DTMT WWNs differ
        subo    r4,r6,r5                # r5 = offset into SUL for this record
        stos    r5,dtmt_sulindx(g4)     # Save this SUL index in the DTMT
        b       .upsul_100              # Update the record.
#
.upsul_83:
? # crash - cqt# 24593 - 2008-06-25 - BE DTMT - failed @ dlmbe.as:8776  ldl 32+r8,r12 with dafedafe - workaround?
        ldl     dtmt_pwwn(r8),r12       # r12-r13 = port WWN of registered DTMT
        cmpobne r10,r12,.upsul_85       # Jif DTMT WWNs different
        cmpobe  r11,r13,.upsul_50       # Jif DTMT WWNs the same
.upsul_85:
        subo    1,r5,r5                 # dec. record count
        addo    r7,r6,r6                # inc. to next record
        b       .upsul_80               # and check next record if more
#
.upsul_90:
#
# --- Assign SUL record to DTMT if possible
#
        ldos    vl_sulh_cnt(r4),r5      # r5 = SUL record count
        ldconst vl_maxsur,r6            # r6 = max. # SUL records
        cmpobe  r5,r6,.upsul_1000       # Jif no more records can be stored
        addo    1,r5,r6                 # inc. SUL record count
        stos    r6,vl_sulh_cnt(r4)      # save updated SUL record count
        ldconst vl_sulh_siz,r7          # r7 = SUL header size
        ldconst vl_sulr_siz,r6          # r6 = size of SUL record
        mulo    r5,r6,r5                # r5 = SUL index for DTMT
        addo    r5,r7,r5                # add in header size
        stos    r5,dtmt_sulindx(g4)     # save index in DTMT
        addo    r4,r5,r6                # r6 = pointer to SUL record to update
        mov     0,r7
        st      r7,vl_sulr_beat(r6)     # reset heartbeat
.upsul_100:
        addo    r4,r5,r6                # r6 = pointer to SUL record to update
        st      g4,vl_sulr_dtmt(r6)     # save DTMT address in SUL record
        ldl     dtmt_pwwn(g4),r8        # r8-r9 = port WWN
        stl     r8,vl_sulr_mac(r6)      # save port WWN
        ldob    dtmt_type(g4),r7        # r7 = target type code
        stob    r7,vl_sulr_type(r6)     # save target type code
        movt    0,r12                   # prepare to zero out the name field
        cmpobe  dtmt_ty_FT,r7,.upsul_200 # Jif not MAGNITUDE node
        ldl     dml_pname(g4),r8        # r8-r9 = node name
        stl     r8,vl_sulr_name(r6)     # save name and clear out the rest
        stt     r12,vl_sulr_name+8(r6)  # of the field
        ldob    dml_vdcnt(g4),r7        # r7 = # VDisks
        stob    r7,vl_sulr_luns(r6)     # save VDisk count
        ldob    dml_cl(g4),r7           # r7 = assigned cluster #
        stob    r7,vl_sulr_cl(r6)       # save cluster #
        b       .upsul_1000
#
.upsul_200:
        ldl     dft_venid(g4),r8        # r8-r9 = vendor ID
        stl     r8,vl_sulr_name(r6)     # save vendor ID and clear out the
        stt     r12,vl_sulr_name+8(r6)  #  rest of the name field
        ldob    dft_luns(g4),r7         # r7 = # LUNs
        stob    r7,vl_sulr_luns(r6)     # save LUN count
        stob    r12,vl_sulr_cl(r6)      # clear cluster #
.upsul_1000:
        ret
#
#******************************************************************************
#
#  NAME: dlm$chk4ldd
#
#  PURPOSE:
#       Checks for a specified linked device and if found returns the
#       associated LDD address.
#
#  DESCRIPTION:
#       Scans all active LDDs for a specified UID match and returns
#       the associated LDD address if a match is found. Otherwise, it
#       returns an indication that a match was not found.
#
#  INPUT:
#       g0-g2 = UID value to match
#
#  OUTPUT:
#       g0 = 0 if no match was found
#       g0 = LDD address if match was found
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#******************************************************************************
#
dlm$chk4ldd:
        lda     DLM_lddindx,r15         # r15 = LDD table pointer
        ldconst MAXLDDS,r14             # r14 = # LDDs to check for
.chk4ldd_100:
        ld      (r15),r12               # r12 = LDD address from table
        cmpobe  0,r12,.chk4ldd_200      # Jif no LDD in this slot
#
# --- Only check LDDs that are operational or uninitialized
#
        ldob    ld_state(r12),r4        # r4 = LDD state code
        cmpobe  ldd_st_op,r4,.chk4ldd_130 # Jif LDD operational
        cmpobne ldd_st_uninit,r4,.chk4ldd_200 # Jif LDD not uninitialized
.chk4ldd_130:
        ldt     ld_serial(r12),r4       # r4-r6 = UID from LDD
        cmpobne g0,r4,.chk4ldd_200      # Jif word 1 does not match
        cmpobne g1,r5,.chk4ldd_200      # Jif word 2 does not match
        cmpobe  g2,r6,.chk4ldd_1000     # Jif word 3 matches 'cause we found a matching LDD
.chk4ldd_200:
        subo    1,r14,r14               # dec. LDD counter
        addo    4,r15,r15               # inc. to next LDD slot in table
        cmpobne 0,r14,.chk4ldd_100      # Jif more LDDs to check
        mov     0,r12                   # return no match indication to caller
.chk4ldd_1000:
        mov     r12,g0                  # put returned value in g0
        ret
#
#******************************************************************************
#
#  NAME: dlm$chk4lddlun
#
#  PURPOSE:
#       Checks for a specified linked device UID and LUN and if found returns
#       associated LDD address.
#
#  DESCRIPTION:
#       Scans all active LDDs for a specified UID and LUN match and returns
#       associated LDD address if a match is found. Otherwise, it returns an
#       indication that a match was not found.
#
#  INPUT:
#       g0-g2 = UID value to match
#       g3    = Real LUN to match
#
#  OUTPUT:
#       g0 = 0 if no match was found
#       g0 = LDD address if match was found
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#******************************************************************************
#
dlm$chk4lddlun:
        lda     DLM_lddindx,r15         # r15 = LDD table pointer
        ldconst MAXLDDS,r14             # r14 = # LDDs to check for
.chk4lddlun_100:
        ld      (r15),r12               # r12 = LDD address from table
        cmpobe  0,r12,.chk4lddlun_200   # Jif no LDD in this slot
#
# --- Only check LDDs that are operational or uninitialized
#
        ldob    ld_state(r12),r4        # r4 = LDD state code
        cmpobe  ldd_st_op,r4,.chk4lddlun_130 # Jif LDD operational
        cmpobne ldd_st_uninit,r4,.chk4lddlun_200 # Jif LDD not uninitialized
.chk4lddlun_130:
        ldt     ld_serial(r12),r4       # r4-r6 = UID from LDD
        cmpobne g0,r4,.chk4lddlun_200   # Jif word 1 does not match
        cmpobne g1,r5,.chk4lddlun_200   # Jif word 2 does not match
        cmpobne g2,r6,.chk4lddlun_200   # Jif word 3 does not match
        ldos    ld_lun(r12),r7          # Get LUN associated with LDD
        cmpobe  g3,r7,.chk4lddlun_1000  # Jif real LUN does not match
.chk4lddlun_200:
        subo    1,r14,r14               # dec. LDD counter
        addo    4,r15,r15               # inc. to next LDD slot in table
        cmpobne 0,r14,.chk4lddlun_100   # Jif more LDDs to check
        mov     0,r12                   # return no match indication to caller
.chk4lddlun_1000:
        mov     r12,g0                  # put returned value in g0
        ret
#
#******************************************************************************
#
#  NAME: dlm$get_ldd
#
#  PURPOSE:
#       Allocates an LDD and initializes it.
#
#  DESCRIPTION:
#       Checks for the first available slot in the LDD table and if
#       none found returns an error to the caller indicating no
#       more LDDs can be supported. If an available slot found,
#       allocates memory to build an LDD and initializes
#       the following fields:
#           ld_ord, ld_pmask, ld_ppri
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g0 = 0 if no more LDD supported
#       g0 = LDD address if more LDDs supported
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#******************************************************************************
#
#
dlm$get_ldd:
        lda     DLM_lddindx,r15         # r15 = LDD table pointer
        ldconst MAXLDDS,r14             # r14 = max. # LDDs supported
        mov     0,r13                   # r13 = LDD ordinal
.getldd_100:
        ld      (r15),r12               # r12 = LDD address from table
        cmpobe  0,r12,.getldd_200       # Jif no LDD in this slot
        addo    1,r13,r13               # inc. LDD ordinal
        addo    4,r15,r15               # inc. to next LDD slot in table
        cmpobne r13,r14,.getldd_100     # Jif more LDDs to check
        mov     0,g0                    # g0 = failed return value
        b       .getldd_1000            # and we're done.

.getldd_200:
        call    DLM$get_ldd             # allocate the memory for the LDD
                                        # g0 = LDD address
        stos    r13,ld_ord(g0)          # save LDD ordinal in LDD
        st      g0,(r15)                # save LDD address in table
        ldconst 0xff,r5
        stob    r5,ld_pmask(g0)         # save path mask byte
        stob    r5,ld_ppri(g0)          # save path priority byte
.getldd_1000:
        ret
#
#******************************************************************************
#
#  NAME: DLM$get_ldd
#
#  PURPOSE:
#       Allocates the memory for an LDD.
#
#  DESCRIPTION:
#       Allocates the memory to be used for an LDD
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g0 = LDD address
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#******************************************************************************
#
#
DLM_GetLDD:
DLM$get_ldd:
c       g0 = s_MallocC(lddsiz|BIT31, __FILE__, __LINE__); # Get LDD
        ret
#
#******************************************************************************
#
#  NAME: DLM_put_ldd
#
#  PURPOSE:
#       Deallocates an LDD
#
#  DESCRIPTION:
#       Removes the LDD from the LDD table and deallocates the
#       memory used.
#
#  INPUT:
#       g0 = LDD address to deallocate
#       Note: It's assumed that all resources linked to the LDD
#             have been removed before calling this routine!
#
#  OUTPUT:
#       None.
#
#******************************************************************************
#
#
DLM_PutLDD:
DLM_put_ldd:
        movl    g0,r12                  # save g0-g1
        ldos    ld_ord(g0),r4           # r4 = LDD table ordinal
        ld      DLM_lddindx[r4*4],r5    # r5 = LDD in table
        cmpobne g0,r5,.putldd_100       # Jif not the same
        mov     0,r5                    # remove LDD from table
        st      r5,DLM_lddindx[r4*4]
.putldd_100:
c       s_Free(g0, lddsiz, __FILE__, __LINE__); # Free memory
        movl    r12,g0                  # restore g0-g1
        ret
#
#******************************************************************************
#
#  NAME: DLM_clr_ldd
#
#  PURPOSE:
#       Clears out an LDD
#
#  DESCRIPTION:
#       Returns pending ILTs queued to LDD with errors.  Performs the
#       necessary processing to "remove" any TPMTs attached to the LDD.
#       Waits for all active LRP ILTs associated with the specified
#       LDD to complete processing before returning to the caller.
#
#  INPUT:
#       g0 = LDD address to clear out
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
DLM_ClrLDD:
DLM_clr_ldd:
        movq    g0,r12                  # save g0-g3
#
# --- "Remove" TPMTs attached to LDD
#
.clrldd_100:
        ld      ld_tpmthd(g0),g3        # g3 = first TPMT on list
        cmpobe  0,g3,.clrldd_300        # Jif no TPMTs on list
        ld      tpm_dtmt(g3),g1         # g1 = assoc. DTMT address
        call    DLM$dem_path            # demolish this path
        b       .clrldd_100             # and check for more TPMTs on list
#
.clrldd_300:
        ld      ld_ailthd(g0),g1        # g1 = first active ILT on list
        cmpobe  0,g1,.clrldd_400        # Jif no active ILTs on list
        mov     g0,g1                   # save g0
        ldconst 1000,g0                 # g0 = task delay period (msec.)
        call    K$twait                 # wait for all active ILTs to complete
        mov     g1,g0                   # restore g0
        b       .clrldd_300             # and check if active ILTs have completed
#
.clrldd_400:
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: dlm$add_tpmt
#
#  PURPOSE:
#       Adds a TPMT to the active TPMT list in both the specified
#       LDD and DTMT.
#
#  DESCRIPTION:
#       Links the specified TPMT onto the active DTMT/TPMT list
#       and the LDD/TPMT list.
#
#  INPUT:
#       g0 = LDD to link TPMT onto
#       g1 = DTMT to link TPMT onto
#       g3 = TPMT address to link
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$add_tpmt:
#
# --- Increment the number of paths that are associated with this LDD
#
        ldob    ld_numpath(g0),r4       # Get the current path count
        addo    1,r4,r4                 # Increment the count
        stob    r4,ld_numpath(g0)       # Save the new path count
#
# --- Link TPMT onto TPMT list in DTMT
#
        mov     0,r4
        st      r4,tpm_link(g3)         # clear link list field in TPMT just in case
        ld      dtmt_tpmttl(g1),r4      # r4 = last TPMT on DTMT list
        cmpobne 0,r4,.addtpmt_310       # Jif list not empty
        st      g3,dtmt_tpmthd(g1)      # save TPMT as head list element
        b       .addtpmt_320
#
.addtpmt_310:
        st      g3,tpm_link(r4)         # link new TPMT onto end of list
.addtpmt_320:
        st      g3,dtmt_tpmttl(g1)      # save new list tail element
#
# --- Link TPMT onto TPMT list in LLD
#
        ld      ld_tpmthd(g0),r4        # r4 = first TPMT on LDD list
        cmpobne 0,r4,.addtpmt_330       # Jif list not empty
#
# --- Case: List was empty
#
        st      g3,ld_tpmthd(g0)        # save TPMT as new head element
        st      g3,tpm_ntpmt(g3)        # link TPMT to itself on LDD list
        b       .addtpmt_340
#
.addtpmt_330:
#
# --- Case: List was not empty. Find end of list and add new TPMT.
#
                                        # r4 = first TPMT on list
        mov     r4,r5                   # r5 = previous TPMT on list
.addtpmt_333:
        ld      tpm_ntpmt(r5),r6        # r6 = next TPMT address on list
        cmpobe  r4,r6,.addtpmt_335      # Jif end of list found
        mov     r6,r5                   # r5 = new previous TPMT on list
        b       .addtpmt_333            # and check next one
#
.addtpmt_335:
        st      g3,tpm_ntpmt(r5)        # link TPMT on end of list
        st      r4,tpm_ntpmt(g3)        # point new TPMT to first TPMT on list
.addtpmt_340:
        ret
#
#******************************************************************************
#
#  NAME: dlm$rem_tpmt
#
#  PURPOSE:
#       Clears the dlmio_s1_tpmt field in any active LRP I/Os using
#       this TPMT.
#       Removes a TPMT from the active TPMT list in both the specified
#       LDD and DTMT.
#
#  DESCRIPTION:
#       Unlinks the specified TPMT from the active DTMT/TPMT list
#       and the LDD/TPMT list. Sets the TPMT state to tom_st_abort.
#
#  INPUT:
#       g0 = LDD to remove TPMT from
#       g1 = DTMT to remove TPMT from
#       g3 = TPMT address to remove
#
#  OUTPUT:
#       tpm_state = tpm_st_abort
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$rem_tpmt:
#
# --- Clear dlmio_s1_tpmt field as needed.
#
        ld      ld_ailthd(g0),r4        # r4 = first LRP I/O on list
        cmpobe  0,r4,.remtpmt_40        # Jif no LRP I/Os on list
        ldconst 0,r3                    # r3 = 0
.remtpmt_10:
        ld      dlmio_p_ilt(r4),r5      # r5 = assoc. LRP I/O ILT address
        cmpobe  0,r5,.remtpmt_20        # Jif no LRP I/O ILT assoc. with LRP
        ld      dlmio_s1_tpmt(r5),r6    # r6 = assoc. TPMT used for I/O
        cmpobne r6,g3,.remtpmt_20       # Jif not using this TPMT
        st      r3,dlmio_s1_tpmt(r5)    # "remove" TPMT from LRP I/O ILT
.remtpmt_20:
        ld      il_fthd(r4),r4          # r4 = next LRP ILT on list
        cmpobne 0,r4,.remtpmt_10        # Jif more ILTs on list
.remtpmt_40:
#
# --- Find and remove TPMT from DTMT list
#
? # crash - cqt# 24330 - 2008-11-19 -- BE DTMT - dlmbe.as:9147  ld 40+g1,r4 with dafedafe - workaround?
        ld      dtmt_tpmthd(g1),r4      # r4 = first TPMT on list
        cmpobe  0,r4,.remtpmt_100       # Jif no TPMTs on list
        cmpobne g3,r4,.remtpmt_50       # Jif not the first on list
        ld      tpm_link(g3),r4         # r4 = next TPMT on DTMT list
        st      r4,dtmt_tpmthd(g1)      # save new list head element
        cmpobne 0,r4,.remtpmt_100       # Jif not the last on list
        st      r4,dtmt_tpmttl(g1)      # clear list tail element
        b       .remtpmt_100
#
.remtpmt_50:
        mov     r4,r5                   # r5 = previous TPMT on list
        ld      tpm_link(r4),r4         # r4 = next TPMT on list
        cmpobe  0,r4,.remtpmt_100       # Jif end of list encountered
        cmpobne g3,r4,.remtpmt_50       # Jif not this TPMT
        ld      tpm_link(g3),r4         # r4 = next TPMT on list
        st      r4,tpm_link(r5)         # remove TPMT from list
        cmpobne 0,r4,.remtpmt_100       # Jif not the last TPMT on list
        st      r5,dtmt_tpmttl(g1)      # save new list tail element
.remtpmt_100:
#
# --- Find and remove TPMT from LDD list
#
        ld      ld_lasttpmt(g0),r4      # r4 = last path TPMT used
        cmpobne g3,r4,.remtpmt_110      # Jif not the last path used
        mov     0,r4                    # clear last path used field in LDD
        st      r4,ld_lasttpmt(g0)
.remtpmt_110:
        ld      ld_tpmthd(g0),r4        # r4 = first TPMT on list
        cmpobe  0,r4,.remtpmt_1000      # Jif no TPMTs on list
#
# --- Decrement the path count for this LDD
#
        ldob    ld_numpath(g0),r5       # r5 = number of paths for this LDD
        subo    1,r5,r5                 # Decrement the current number of paths
        stob    r5,ld_numpath(g0)       # Save the current number of paths
#
        cmpobne g3,r4,.remtpmt_200      # Jif not the first on list
#
# --- TPMT is first on LDD list
#
        ld      tpm_ntpmt(g3),r4        # r4 = next TPMT on list
        cmpobne g3,r4,.remtpmt_150      # Jif not the only TPMT on list
        mov     0,r4                    # remove TPMT from list
        st      r4,ld_tpmthd(g0)        # clear list head element
        b       .remtpmt_1000           # and we're done!
#
.remtpmt_150:
        st      r4,ld_tpmthd(g0)        # save new list head element
        mov     r4,r5                   # r5 = list thread
.remtpmt_160:
        ld      tpm_ntpmt(r5),r6        # r6 = next TPMT on list
        cmpobe  g3,r6,.remtpmt_170      # Jif last on list found
        mov     r6,r5                   # r5 = next TPMT on list
        b       .remtpmt_160            # check next TPMT on list
#
.remtpmt_170:
        st      r4,tpm_ntpmt(r5)        # link last TPMT to new head element
        b       .remtpmt_1000           # and we're out of here!
#
# --- TPMT not the first on LDD list
#
.remtpmt_200:
                                        # r4 = first TPMT on list
        mov     r4,r5                   # r5 = previous TPMT on list
.remtpmt_210:
        ld      tpm_ntpmt(r5),r6        # r6 = next TPMT on list
        cmpobe  r4,r6,.remtpmt_1000     # Jif back to beginning of list
        cmpobe  g3,r6,.remtpmt_250      # Jif next TPMT on list is it
        mov     r6,r5                   # r5 = new previous TPMT on list
        b       .remtpmt_210            # check next TPMT on list
#
.remtpmt_250:
        ld      tpm_ntpmt(g3),r4        # r4 = next TPMT after one being removed
        st      r4,tpm_ntpmt(r5)        # remove TPMT from list
.remtpmt_1000:
        ldconst tpm_st_abort,r4         # r4 = TPMT abort state code
        stob    r4,tpm_state(g3)        # save TPMT state code
        ret
#
#******************************************************************************
#
#  NAME: dlm$pkest_vl
#
#  PURPOSE:
#       Packs an Establish VLink datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send an Establish VLink datagram to the specified XIOtech Controller
#       link.
#
#  INPUT:
#       g0 = assoc. LDD address
#       g1 = 32 or 64 for type of request to make.
#       g3 = 16-bit VDisk #
#       g4 = establish VLink attributes
#
#  OUTPUT:
#       g1 = Establish VLink datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
dlm$pkest_vl:
        movq    g8,r12                  # save g8-g11
        ldconst DLM0_rq_estvl_size,g10  # g10 = request message size
c       r6 = g1;                        # save input g1 register (32 or 64)
c   if (r6 == 32) {
        ldconst DLM0_rs_estvl_size,g11  # g11 = response message size
c   } else {
        ldconst DLM0_rs_estvl_size_GT2TB,g11 # g11 = response message size
c   }
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        ld      ld_basesn(g0),r9        # r9 = destination MAG serial # in little-endian format
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
c   if (r6 == 32) {
        ldq     dlm_estvl_hdr,g8        # g8-g11 = bytes 0-15 of req. header
c   } else {
        ldq     dlm_estvl_hdr_GT2TB,g8  # g8-g11 = bytes 0-15 of req. header
c   }
        lda     dgrq_size(r10),r10      # r10 = pointer to remaining req.  message
        bswap   r9,g11                  # g11 = dest. serial # in big-endian format
        stq     g8,(r8)                 # save bytes 0-15 of req. header
        st      g11,DLM0_rq_estvl_dstsn(r10) # save dest. MAG serial #
c   if (r6 == 32) {
        ldq     dlm_estvl_hdr+16,g8     # g8-g11 = bytes 16-31 of req. header
c   } else {
        ldq     dlm_estvl_hdr_GT2TB+16,g8 # g8-g11 = bytes 16-31 of req. header
c   }
        bswap   g9,g9                   # swap remaining length value
        stq     g8,16(r8)               # save bytes 16-31 of req. header
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r4         # r4 = my VCG serial #
        bswap   r4,r4
        st      r4,DLM0_rq_estvl_srcsn(r10) # save source MAG serial #
        ld      fi_cserial(r3),r4       # r4 = my controller serial #
        mov     g3,r5                   # r5 = VDisk #
        bswap   r4,r4
        extract 8,8,r5                  # r5 = MSB of VDisk #
        st      r4,DLM0_rq_estvl_agnt(r10) # save transfer agent MAG serial #
        stob    r5,DLM0_rq_estvl_srccl(r10) # save MSB source VDisk #
        stob    g3,DLM0_rq_estvl_srcvd(r10) # save LSB source VDisk #
        setbit  0,g4,r5                 # set 16-bit VDisk format attribute
        stos    r5,DLM0_rq_estvl_attr(r10) # save VLink attributes and clear reserved byte field
        ld      V_vddindx[g3*4],r4      # Get VDD
        cmpobe  0,r4,.pkestvl_400       # Jif no VDD defined yet
        ldl     vd_name(r4),r4          # Get the name
        b       .pkestvl_500
#
.pkestvl_400:
        movl    0,r4                    # r4-r5 = null VDisk name
.pkestvl_500:
        stl     r4,DLM0_rq_estvl_name(r10) # save device name
        ldob    ld_basecl(g0),r4        # r4 = dest. MAG cluster #
        ldos    ld_basevd(g0),r5        # r5 = dest. MAG VDisk #
        st      r4,DLM0_rq_estvl_dstcl(r10) # save dest. MAG cluster # and clear reserved bytes
        stob    r5,DLM0_rq_estvl_dstvd(r10) # save dest. MAG VDisk #
        movq    r12,g8                  # restore g8-g11
        ret
#
#******************************************************************************
#
#  NAME: dlm$pkswp_vl
#
#  PURPOSE:
#       Packs a Swap VLink Lock datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send a Swap VLink Lock datagram to the specified XIOtech Controller
#       link.
#
#  INPUT:
#       g0 = assoc. LDD address
#       g1 = 32 or 64 for type of DLM call to make.
#       g3 = 16-bit VDisk #
#       g4 = establish VLink attributes
#       g6 = original owner 16-bit VLink #
#       g7 = original owner MAG serial #
#
#  OUTPUT:
#       g1 = Swap VLink Lock datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
dlm$pkswp_vl:
        movq    g8,r12                  # save g8-g11
        ldconst DLM0_rq_swpvl_size,g10  # g10 = request message size
c   if (g1 == 64) {
        lda     dlm_swpvl_hdr_GT2TB,r7  # Save 64 bit template for below copying.
        ldconst DLM0_rs_swpvl_size_GT2TB,g11 # g11 = response message size
c   } else {
        lda     dlm_swpvl_hdr,r7        # Save 32 bit template for below copying.
        ldconst DLM0_rs_swpvl_size,g11  # g11 = response message size
c   }
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        ld      ld_basesn(g0),r9        # r9 = destination MAG serial # in little-endian format
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     (r7),g8                 # g8-g11 = bytes 0-15 of req. header
        lda     dgrq_size(r10),r10      # r10 = pointer to remaining req.  message
        bswap   r9,g11                  # g11 = dest. serial # in big-endian format
        stq     g8,(r8)                 # save bytes 0-15 of req. header
        st      g11,DLM0_rq_swpvl_dstsn(r10) # save dest. MAG serial #
        ldq     16(r7),g8               # g8-g11 = bytes 16-31 of req. header
        bswap   g9,g9                   # swap remaining length value
        stq     g8,16(r8)               # save bytes 16-31 of req. header
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r4         # r4 = my VCG serial #
        bswap   r4,r4
        st      r4,DLM0_rq_swpvl_srcsn(r10) # save source MAG serial #
        mov     g3,r5                   # r5 = 16-bit VDisk #
        ld      fi_cserial(r3),r4       # r4 = my controller serial #
        extract 8,8,r5                  # r5 = MSB VDisk #
        bswap   r4,r4
        st      r4,DLM0_rq_swpvl_agnt(r10) # save transfer agent MAG serial #
        stob    r5,DLM0_rq_swpvl_srccl(r10) # save MSB VDisk #
        setbit  0,g4,r4                 # set format attribute flag
        stob    g3,DLM0_rq_swpvl_srcvd(r10) # save LSB VDisk #
        stos    r4,DLM0_rq_swpvl_attr(r10) # save VLink attributes and clear reserved byte field
        ldconst 0x20202020,r4
        mov     r4,r5
        ld      V_vddindx[g3*4],r7      # r7 = VDD address
        cmpobe  0,r7,.pkswpvl_300       # Jif no VDD defined
#
        ldl     vd_name(r7),r4          # Get the name
#
.pkswpvl_300:
        stl     r4,DLM0_rq_swpvl_name(r10) # save device name
        ldob    ld_basecl(g0),r4        # r4 = dest. MAG cluster #
        ldos    ld_basevd(g0),r5        # r5 = dest. MAG VDisk #
        st      r4,DLM0_rq_swpvl_dstcl(r10) # save dest. MAG cluster # and clear reserved bytes
        stob    r5,DLM0_rq_swpvl_dstvd(r10) # save dest. MAG VDisk #
        bswap   g7,r4
        mov     g6,r5                   # r5 = original owner 16-bit VLink #
        st      r4,DLM0_rq_swpvl_orgsn(r10) # save orig. MAG serial #
        extract 8,8,r5                  # r5 = MSB orig. owner 16-bit VLink #
        st      r5,DLM0_rq_swpvl_orgcl(r10) # save MSB orig. 16-bit VLink # clear reserved bytes
        stob    g6,DLM0_rq_swpvl_orgvd(r10) # save LSB orig. 16-bit VLink #
        movq    r12,g8                  # restore g8-g11
        ret
#
#******************************************************************************
#
#  NAME: dlm$pktrm_vl
#
#  PURPOSE:
#       Packs a Terminate VLink datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send a Terminate VLink datagram to the specified XIOtech Controller
#       link.
#
#  INPUT:
#       g0 = assoc. LDD address
#       g3 = 16-bit VLink #
#
#  OUTPUT:
#       g1 = Terminate VLink datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
dlm$pktrm_vl:
        movq    g8,r12                  # save g8-g11
        ldconst DLM0_rq_trmvl_size,g10  # g10 = request message size
        ldconst 0,g11                   # g11 = response message size
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        ld      ld_basesn(g0),r9        # r9 = destination MAG serial # in little-endian format
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     dlm_trmvl_hdr,g8        # g8-g11 = bytes 0-15 of req. header
        lda     dgrq_size(r10),r10      # r10 = pointer to remaining req. message
        bswap   r9,g11                  # g11 = dest. serial # in big-endian format
        stq     g8,(r8)                 # save bytes 0-15 of req. header
        st      g11,DLM0_rq_trmvl_dstsn(r10) # save dest. MAG serial #
        ldq     dlm_trmvl_hdr+16,g8     # g8-g11 = bytes 16-31 of req. header
        bswap   g9,g9                   # swap remaining length value
        stq     g8,16(r8)               # save bytes 16-31 of req. header
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r4         # r4 = my VCG serial #
        bswap   r4,r4
        st      r4,DLM0_rq_trmvl_srcsn(r10) # save source MAG serial #
        mov     g3,r5                   # r5 = 16-bit VLink #
        ld      fi_cserial(r3),r4       # r4 = my MAG serial #
        extract 8,8,r5                  # r5 = MSB VLink #
        bswap   r4,r4
        st      r4,DLM0_rq_trmvl_agnt(r10) # save transfer agent MAG serial #
        st      r5,DLM0_rq_trmvl_srccl(r10) # save MSB VLink # and clear reserved bytes
        stob    g3,DLM0_rq_trmvl_srcvd(r10) # save LSB VLink #
        ldob    ld_basecl(g0),r4        # r4 = dest. MAG cluster #
        ldos    ld_basevd(g0),r5        # r5 = dest. MAG VDisk #
        st      r4,DLM0_rq_trmvl_dstcl(r10) # save dest. MAG cluster # and clear reserved bytes
        stob    r5,DLM0_rq_trmvl_dstvd(r10) # save dest. MAG VDisk #
        movq    r12,g8                  # restore g8-g11
        ret
#
#******************************************************************************
#
#  NAME: dlm$pk_vquery
#
#  PURPOSE:
#       Packs a VDisk/VLink Query datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send a VDisk/VLink Query datagram to the specified XIOtech
#       Controller link, cluster, VDisk # combo.
#
#  INPUT:
#       g0 = MAG serial # to query
#       g1 = MAG cluster # to query
#       g2 = MAG VDisk # to query
#       g3 = 32 or 64 for DLM request type.
#
#  OUTPUT:
#       g1 = VDisk/VLink Query datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
dlm$pk_vquery:
        movq    g8,r12                  # save g8-g11
        mov     g1,r3                   # r3 = MAG cluster # to query
        ldconst DLM0_rq_vqury_size,g10  # g10 = request message size
c   if (g3 == 32) {
        ldconst DLM0_rs_vqury_size,g11  # g11 = response message size
        lda     dlm_vqury_hdr,r4        # r4 is req. header
c   } else {
        ldconst DLM0_rs_vqury_size_GT2TB,g11 # g11 = response message size
        lda     dlm_vqury_hdr_GT2TB,r4  # r4 is req. header
c   }
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     (r4),g8                 # g8-g11 = bytes 0-15 of req. header
        lda     dgrq_size(r10),r10      # r10 = pointer to remaining req. message
        bswap   g0,g11                  # g11 = dest. serial # in big-endian format
        stq     g8,(r8)                 # save bytes 0-15 of req. header
        st      g11,DLM0_rq_vqury_sn(r10) # save MAG serial #
        ldq     16(r4),g8               # g8-g11 = bytes 16-31 of req. header
        bswap   g9,g9                   # swap remaining length value
        stq     g8,16(r8)               # save bytes 16-31 of req. header
        st      r3,DLM0_rq_vqury_cl(r10) # save cluster # and clear reserved bytes #0 & #1
        stob    g2,DLM0_rq_vqury_vd(r10) # save VDisk #
        movq    r12,g8                  # restore g8-g11
        ret
#
#******************************************************************************
#
#  NAME: dlm$pkchg_siz
#
#  PURPOSE:
#       Packs a Change VDisk Size datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send a Change VDisk Size datagram to the specified XIOtech
#       Controller link relative to the specified VLink.
#
#  INPUT:
#       g1 = 16-bit VLink #
#       g2,g3 = new VDisk size
#       g4 = assoc. LDD address of linked device
#       g5 = 32 or 64 for type of DLM request
#
#  OUTPUT:
#       g1 = Change VDisk Size datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
dlm$pkchg_siz:
        movq    g8,r12                  # save g8-g11
        mov     g1,r11                  # r11 = 16-bit VLink #
c if (g5 == 32) {
        ldconst DLM0_rq_chgsz_size,g10  # g10 = request message size
c } else {
        ldconst DLM0_rq_chgsz_size_GT2TB,g10 # g10 = request message size
c }
        ldconst 0,g11                   # g11 = response message size
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        ld      ld_basesn(g4),r9        # r9 = destination MAG serial # in little-endian format
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
c if (g5 == 32) {
        lda     dlm_chgsz_hdr,r3        # r3 is req. header
c } else {
        lda     dlm_chgsz_hdr_GT2TB,r3  # r3 is req. header
c }
        ldq     (r3),g8                 # g8-g11 = bytes 0-15 of req. header
        lda     dgrq_size(r10),r10      # r10 = pointer to remaining req.  message
        bswap   r9,g11                  # g11 = dest. serial # in big-endian format
        stq     g8,(r8)                 # save bytes 0-15 of req. header
        st      g11,DLM0_rq_chgsz_dstsn(r10) # save dest. MAG serial #
        ldq     16(r3),g8               # g8-g11 = bytes 16-31 of req. header
        bswap   g9,g9                   # swap remaining length value
        stq     g8,16(r8)               # save bytes 16-31 of req. header
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my VCG serial #
        mov     r11,r4                  # r4 = 16-bit VLink #
        bswap   r3,r3
        extract 8,8,r4                  # r4 = MSB VLink #
        st      r3,DLM0_rq_chgsz_srcsn(r10) # save source MAG serial #
        st      r4,DLM0_rq_chgsz_srccl(r10) # save MSB VLink # and clear reserved bytes
        stob    r11,DLM0_rq_chgsz_srcvd(r10) # save LSB VLink #
        ldob    ld_basecl(g4),r4        # r4 = dest. MAG cluster #
        ldos    ld_basevd(g4),r5        # r5 = dest. MAG VDisk #
        st      r4,DLM0_rq_chgsz_dstcl(r10) # save dest. MAG cluster # and clear reserved bytes
        stob    r5,DLM0_rq_chgsz_dstvd(r10) # save dest. MAG VDisk #
c if (g5 == 32) {
c       if (g3 != 0) {
c           g2 = 0xffffffffUL;          # Flag that the vdisk is too big for this DLM call.
c           g3 = 0;
c       }
        bswap   g2,r6                   # r6 = new size in big-endian format
        st      r6,DLM0_rq_chgsz_dsiz(r10) # save new VDisk size
c   } else {
        stl     g2,DLM0_rq_chgsz_dsiz_GT2TB(r10) # save new VDisk size
c   }
        movq    r12,g8                  # restore g8-g11
        ret
#
#******************************************************************************
#
#  NAME: dlm$pkvl_poll
#
#  PURPOSE:
#       Packs a VLink Poll datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send a VLink Poll datagram to the specified XIOtech Controller
#       link relative to the specified VLink.
#
#  INPUT:
#       g3 = 16-bit VLink #
#       g4 = assoc. LDD address of linked device
#
#  OUTPUT:
#       g1 = VLink Poll datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
dlm$pkvl_poll:
        movq    g8,r12                  # save g8-g11
        ldconst DLM0_rq_vlpol_size,g10  # g10 = request message size
        ldconst 0,g11                   # g11 = response message size
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        ld      ld_basesn(g4),r9        # r9 = destination MAG serial # in little-endian format
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     dlm_vlpol_hdr,g8        # g8-g11 = bytes 0-15 of req. header
        lda     dgrq_size(r10),r10      # r10 = pointer to remaining req.  message
        bswap   r9,g11                  # g11 = dest. serial # in big-endian format
        stq     g8,(r8)                 # save bytes 0-15 of req. header
        st      g11,DLM0_rq_vlpol_dstsn(r10) # save dest. MAG serial #
        ldq     dlm_vlpol_hdr+16,g8     # g8-g11 = bytes 16-31 of req. header
        bswap   g9,g9                   # swap remaining length value
        stq     g8,16(r8)               # save bytes 16-31 of req. header
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my VCG serial #
        mov     g3,r4                   # r4 = 16-bit VLink #
        bswap   r3,r3
        extract 8,8,r4                  # r4 = MSB VLink #
        st      r3,DLM0_rq_vlpol_srcsn(r10) # save source MAG serial #
        st      r4,DLM0_rq_vlpol_srccl(r10) # save MSB VLink # and clear reserved bytes
        stob    g3,DLM0_rq_vlpol_srcvd(r10) # save LSB VLink #
        ldob    ld_basecl(g4),r4        # r4 = dest. MAG cluster #
        ldos    ld_basevd(g4),r5        # r5 = dest. MAG VDisk #
        st      r4,DLM0_rq_vlpol_dstcl(r10) # save dest. MAG cluster # and clear reserved bytes
        stob    r5,DLM0_rq_vlpol_dstvd(r10) # save dest. MAG VDisk #
        movq    r12,g8                  # restore g8-g11
        ret
#
#******************************************************************************
#
#  NAME: dlm$pk_vdsize
#
#  PURPOSE:
#       Packs a VDisk Size Changed datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send a VDisk Size Changed datagram to the specified XIOtech
#       Controller link, cluster, VDisk # combo.
#
#  INPUT:
#       g0 = Controller serial # to notify
#       g1 = Controller cluster # that is affected
#       g2 = Controller VLink # that is affected
#       g3 = Reported target/LUN # affected
#       g4-g5 = proposed new VDisk size
#
#  OUTPUT:
#       g1 = VDisk Size Changed datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
dlm$pk_vdsize:
        mov     g1,r12                  # save g1
                                        # r12 = cluster # that is affected
        movl    g10,r14                 # save g10-g11
c   if (g5 == 0) {                      # Vdisk size small enough for 32 bit DLM version
        ldconst DLM0_rq_vdsize_size,g10 # g10 = request message size
c   } else {
        ldconst DLM0_rq_vdsize_size_GT2TB,g10 # request message size for 64 bit DLM version
c }
        ldconst 0,g11                   # g11 = response message size
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
c   if (g5 == 0) {                      # Vdisk size small enough for 32 bit DLM version
        lda     dlm_vdsize_hdr,r3       # r3 is req. header
c   } else {
        lda     dlm_vdsize_hdr_GT2TB,r3 # r3 is req. header
c   }
        ldq     (r3),r4                 # r4-r7 = bytes 0-15 of req. header
        lda     dgrq_size(r10),r10      # r10 = pointer to remaining req.  message
        bswap   g0,r7                   # r7 = dest. serial # in big-endian format
        stq     r4,(r8)                 # save bytes 0-15 of req. header
        st      r7,DLM0_rq_vdsize_dstsn(r10) # save dest. Controller serial # in datagram data
        ldq     16(r3),r4               # r4-r7 = bytes 16-31 of req. header
        bswap   r5,r5                   # swap remaining length value
        stq     r4,16(r8)               # save bytes 16-31 of req. header
        st      r12,DLM0_rq_vdsize_dstcl(r10) # save dest. cluster # and clear 2 reserved bytes
        stob    g2,DLM0_rq_vdsize_dstvd(r10) # save dest. vid
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my VCG serial #
        mov     g3,g11                  # g11 = reported target/LUN # affected
        bswap   r3,r3
        extract 8,8,g11                 # g11 = reported target # affected
        st      r3,DLM0_rq_vdsize_srcsn(r10) # save MAG serial #
c   if (g5 == 0) {
        bswap   g4,r5                   # r5 = proposed new VDisk size in big-endian format
        st      r5,DLM0_rq_vdsize_devcap(r10) # save proposed new size
c   } else {
        stl     g4,DLM0_rq_vdsize_devcap_GT2TB(r10) # save proposed new size
c   }
        st      g11,DLM0_rq_vdsize_srccl(r10) # save reported target affected and clear 2 reserved bytes
        stob    g3,DLM0_rq_vdsize_srcvd(r10) # Save reported LUN # affected
        movl    r14,g10                 # restore g10-g11
        ret
#
#******************************************************************************
#
#  NAME: DLM$pk_master
#
#  PURPOSE:
#       Packs a Group Master Controller Definition datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send a Group Master Controller Definition datagram to the
#       specified XIOtech Controller.
#
#  INPUT:
#       g0 = Group serial #
#       g1 = Controller serial # to notify
#       g2 = Master controller serial #
#
#  OUTPUT:
#       g1 = Group Master Controller Definition datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
DLM$pk_master:
        mov     g1,r12                  # save g1
                                        # r12 = controller serial # to notify
        movl    g10,r14                 # save g10-g11
        ldconst DLM0_rq_master_size,g10 # g10 = request message size
        ldconst 0,g11                   # g11 = response message size
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     dlm_master_hdr,r4       # r4-r7 = bytes 0-15 of req. header
        lda     dgrq_size(r10),r10      # r10 = pointer to remaining req.  message
        bswap   r12,r7                  # r7 = dest. serial # in big-endian format
        stq     r4,(r8)                 # save bytes 0-15 of req. header
        ldq     dlm_master_hdr+16,r4    # r4-r7 = bytes 16-31 of req. header
        bswap   r5,r5                   # swap remaining length value
        stq     r4,16(r8)               # save bytes 16-31 of req. header
        bswap   g0,r4
        bswap   g2,r5
        st      r4,DLM0_rq_master_gpsn(r10) # save group serial #
        st      r5,DLM0_rq_master_mastsn(r10) # save master controller serial #
        movl    r14,g10                 # restore g10-g11
        ret
#
#**********************************************************************
#
#  NAME: dlm$precedence
#
#  PURPOSE:
#       To provide a common means of performing a precedence check
#       for a LRP I/O operation.
#
#  DESCRIPTION:
#       A complete search is made of the LRP I/O queue.  For
#       each dependency found, the parent ILT is queued to that
#       dependency thread.
#
#  INPUT:
#       g4  = assoc. LDD address
#       g14 = incoming LRP I/O parent ILT
#
#  OUTPUT:
#       g0  = T/F (existing precedence)
#
#  REGS DESTROYED:
#       Reg. g0,g8,g9 destroyed.
#
#**********************************************************************
#
dlm$precedence:
#
# --- Check for pending LRP I/O precedence
#
        mov     0,r15                   # r15 = dependency count
        ld      dlmio_p_len(g14),r3     # r3 = Incoming I/O Length
!       ldl     dlmio_p_sda(g14),g8     # g8-g9 = Incoming SDA
.if     DEBUG_FLIGHTREC_DLM
# @@@ FINISH
        ldconst frt_dlm_prec,r4         # Type
        st      r4,fr_parm0             # Virtual - v$exec
        st      g14,fr_parm1            # ILT
        st      g8,fr_parm2             # SDA LS
        st      g9,fr_parm3             # SDA MS
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_DLM
        stl     g8,dlmio_p_psda(g14)    # Save the SDA as precedence in ILT
        cmpo    0,1                     # Clear the Carry Bit
        addc    r3,g8,r12               # r12-r13 = Incoming EDA
        addc    0,g9,r13
        stl     r12,dlmio_p_peda(g14)   # Save the EDA as precedence in ILT
        lda     ld_ailthd(g4),r3        # r3 = active LRP/ILT list pointer
        mov     FALSE,g0                # g0 = precedence flag for now
        lda     FALSE,r14               # r14 = indirect dependency flag
#
# --- Examine next LRP I/O ILT
#
.prec10:
        ld      il_fthd(r3),r3          # r3 = next active LRP ILT
        cmpobe  0,r3,.prec40            # Jif no more LRP/ILTs on list
        ld      dlmio_p_len(r3),r6      # r6 = I/O length
!       ldl     dlmio_p_psda(r3),r4     # r4-r5 = precedence SDA
!       ldl     dlmio_p_peda(r3),r10    # r10-r11 = precedence EDA
        cmpobe  0,r6,.prec10            # Jif no data transferred
        cmpobl  r13,r5,.prec10          # Jif incoming EDA below precedence SDA
        bg      .prec15
        cmpoble r12,r4,.prec10
.prec15:
        cmpobg  g9,r11,.prec10          # Jif imcoming SDA above precedence EDA
        bl      .prec18
        cmpobge g8,r10,.prec10
#
# --- Link new ILT to end of this dependency queue
#
.prec18:
        lda     dlmio_p_deplist(r3),r7  # r7 = ptr to head of dependency list
        mov     TRUE,g0                 # Set precedence flag
.prec20:
        mov     r7,r8                   # r8 = previous dependent LRP/ILT
        ld      il_fthd(r7),r7          # r7 = next LRP/ILT on list
        cmpobe  g14,r7,.prec30          # Jif new LRP/ILT already linked to this list
        cmpobe  0,r7,.prec25            # Jif end of list
#
        ld      dlmio_p_depcnt(r7),r9   # r9 = dependency count
        cmpobe  1,r9,.prec20            # Jif single dependency
#
        mov     TRUE,r14                # Set indirect dependency
        b       .prec20
#
.prec25:
#
        st      g14,il_fthd(r8)         # Link new ILT to end of queue
        addo    1,r15,r15               # Bump dependency count
#
# --- Update dependency addresses in parent ILT - If the addresses of this
#     request go beyond the addresses in the parent ILT, then the addresses
#     in the parent ILT must be expanded.
#
.prec30:
                                        # Check for lower SDA
        cmpobg  g9,r5,.prec35           # Jif in SDA(h) > precedence SDA(h)
        bne     .prec32                 # Jif in SDA(h) != precedence SDA(h)
        cmpobge g8,r4,.prec35           # Jif in SDA(l) >= precedence SDA(l)
.prec32:
        stl     g8,dlmio_p_psda(r3)     # In SDA < prec SDA, save in the ILT
.prec35:
                                        # Check for greater EDA
        cmpobl  r13,r11,.prec10         # Jif in EDA(h) < precedence EDA(h)
        bne     .prec38                 # Jif in EDA(h) != precedence EDA(h)
        cmpoble r12,r10,.prec10         # Jif in EDA(l) <= precedence EDA(l)
.prec38:
        stl     r12,dlmio_p_peda(r3)    # In EDA > prec EDA, save in the ILT
        b       .prec10                 # Continue searching
#
# --- Check for precedence found
#
.prec40:
        cmpobe  FALSE,g0,.prec100       # Jif precedence not found
#
        mov     0,r4                    # r4 = 0
        st      r15,dlmio_p_depcnt(g14) # Save dependency count
        ldconst diopst_dep,r5           # r5 = LRP I/O state code
        st      r4,il_fthd(g14)         # Clear forward thread
        stob    r5,dlmio_p_state(g14)   # save I/O state code
        cmpobe  FALSE,r14,.prec100      # Jif no indirect dependencies
#
        call    dlm$resync              # Resync LRP I/O queue
#
# --- Exit
#
.prec100:
        ret
#
#**********************************************************************
#
#  NAME: dlm$resync
#
#  PURPOSE:
#       To provide a means of resyncing dependencies within the active
#       LRP I/O queue.
#
#  DESCRIPTION:
#       The active LRP I/O queue for the specified LDD is rescanned
#       with the precedence SDA/EDA fields being reconstructed from
#       scratch.
#
#  INPUT:
#       g4 = LDD address to resync
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
dlm$resync:
        lda     ld_ailthd(g4),r15       # r15 = ptr to 1st active LRP/ILT
#
# --- Get next LRP I/O ILT
#
.rsync10:
        ld      il_fthd(r15),r15        # r15 = next LRP/ILT
        cmpobe  0,r15,.rsync100         # Jif none
#
# --- Update precedence SDA/EDA in LRP/ILT
#
        ld      dlmio_p_deplist(r15),r14 # r14 = dependency list
        cmpobe  0,r14,.rsync10          # Jif none
#
!       ldl     dlmio_p_psda(r15),r8    # r8-r9 = current parent precedence SDA
!       ldl     dlmio_p_peda(r15),r10   # r10-r11 = current parent prec. EDA
#
# --- Examine next dependent ILT
#
.rsync20:
        ld      dlmio_p_len(r14),r7     # r7 = Length of the I/O request
!       ldl     dlmio_p_sda(r14),r4     # r4-r5 = LRP I/O SDA
.if     DEBUG_FLIGHTREC_DLM
# @@@ FINISH
        ldconst frt_dlm_resync,r3       # Type
        st      r3,fr_parm0             # Virtual - v$exec
        st      r14,fr_parm1            # ILT
        st      r4,fr_parm2             # SDA LS
        st      r5,fr_parm3             # SDA MS
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_DLM
        cmpo    0,1                     # Clear the carry bit
        addc    r4,r7,r6                # Get the End Disk Address + 1
        addc    0,r5,r7                 # r6-r7 = LRP I/O EDA
        ld      il_fthd(r14),r14        # r14 = next dependent LRP/ILT
#
# --- Check for lower SDA
#
        cmpobg  r5,r9,.rsync40          # Jif in SDA(h) > precedence SDA(h)
        bne     .rsync30                # Jif in SDA(h) != precedence SDA(h)
        cmpobge r4,r8,.rsync40          # Jif in SDA(l) >= precedence SDA(l)
.rsync30:
        movl    r4,r8                   # Update the current precedence SDA
#
# --- Check for greater EDA
#
.rsync40:
        cmpobl  r7,r11,.rsync60         # Jif in EDA(h) < precedence EDA(h)
        bne     .rsync50                # Jif in EDA(h) != precedence EDA(h)
        cmpoble r6,r10,.rsync60         # Jif in EDA(l) <= precedence EDA(l)
.rsync50:
        movl    r6,r10                  # Update the current precedence EDA
.rsync60:
        cmpobne 0,r14,.rsync20          # Jif more dependent LRP/ILTs present
#
        stl     r8,dlmio_p_psda(r15)    # Save the smallest SDA in the ILT
        stl     r10,dlmio_p_peda(r15)   # Save the largest EDA in the ILT
        b       .rsync10                # and go check next ILT on list
#
# --- Exit
#
.rsync100:
        ret
#
#******************************************************************************
#
#  NAME: dlm$q_LRP
#
#  PURPOSE:
#       Queues a LRP I/O ILT to the active LRP queue associated with
#       the specified LDD.
#
#  DESCRIPTION:
#       Places the specified ILT onto the end of the active list
#       of LRPs associated with the specified LDD.
#
#  INPUT:
#       g4  = assoc. LDD address
#       g14 = LRP I/O ILT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$q_LRP:
        ldl     ld_ailthd(g4),r14       # r14 = first element on list
                                        # r15 = last element on list
        lda     ld_ailthd(g4),r13       # r13 = base address of queue
        mov     0,r4                    # r4 = 0
        ldob    dlmio_p_flag(g14),r6    # r6 = current flag byte
        cmpobne 0,r14,.qLRP100          # Jif queue not empty
#
# --- Case: Queue was empty.
#
        mov     r13,r15                 # set base of queue as backward thread
        stl     r14,il_fthd(g14)        # save forward/backward threads in ILT
        mov     g14,r8                  # r8 = LRP I/O ILT
        mov     g14,r9                  # r9 = LRP I/O ILT
        stl     r8,(r13)                # save ILT as head & tail pointer
        b       .qLRP900                # and we're out of here!
#
# --- Case: Queue was NOT empty. Place on end of queue.
#
.qLRP100:
        st      g14,il_fthd(r15)        # link new ILT onto end of list
        st      g14,ld_ailttl(g4)       # save new ILT as new tail
        st      r4,il_fthd(g14)         # clear forward thread in new ILT
        st      r15,il_bthd(g14)        # save backward thread in new ILT
.qLRP900:
        setbit  7,r6,r6                 # set flag indicating ILT on active list
        stob    r6,dlmio_p_flag(g14)    # set flag indicating ILT is on the working queue
        ret
#
#******************************************************************************
#
#  NAME: dlm$rem_LRP
#
#  PURPOSE:
#       Removes the specified ILT associated with a LRP I/O from
#       the active LRP queue in the specified LDD if appropriate.
#
#  DESCRIPTION:
#       Checks the flags in the specified ILT to see if it resides
#       on the active queue maintained in the specified LDD. If so,
#       removes the ILT from the appropriate queue and clears the
#       flag indicating the ILT is no longer on the queue. If the
#       ILT has no flags indicating it resides on the active queue
#       in the, this routine simply returns to the caller.
#
#  INPUT:
#       g4  = assoc. LDD address
#       g14 = LRP I/O ILT address to remove
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$rem_LRP:
        ldob    dlmio_p_flag(g14),r4    # r4 = flag byte indicating whether the ILT is on active queue.
        ldl     il_fthd(g14),r14        # r14 = forward thread of ILT
                                        # r15 = backward thread of ILT
        lda     ld_ailthd(g4),r13       # r13 = base address of active queue
        bbc     7,r4,.remLRP1000        # Jif on LDD active list flag is not set
        clrbit  7,r4,r4                 # clear on LDD active list flag
        stob    r4,dlmio_p_flag(g14)    # save updated flag byte
#
# --- Remove LRP I/O ILT from active queue
#
        st      r14,il_fthd(r15)        # put forward thread from removed ILT as forward thread of previous ILT
        cmpobne 0,r14,.remLRP700        # Jif non-zero forward thread
        mov     r13,r14                 # make base of queue the forward thread
        cmpobne r13,r15,.remLRP700      # Jif backward thread <> base of queue
        mov     0,r15                   # queue is now empty!
.remLRP700:
        st      r15,il_bthd(r14)        # put backward thread from removed ILT as backward thread of previous ILT
.remLRP1000:
        ret
#
#******************************************************************************
#
#  NAME: dlm$gen_LRP
#
#  PURPOSE:
#       This routine builds and sends an I/O request associated with
#       the specified LRP I/O ILT.
#
#  DESCRIPTION:
#       This routine determines the path to send a LRP I/O request
#       over. If none exists, it sets the LRP state to waiting to
#       retry I/O. If one exists, it allocates an ILT and SRP and
#       builds up a ILT/SRP I/O request associated with the specified
#       LRP. It sets the LRP state code the appropriate value and sends
#       out the request.
#
#  INPUT:
#       g4  = assoc. LDD address
#       g14 = LRP I/O ILT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$gen_LRP:
        movq    g0,r12                  # save g0-g3
        call    dlm$find_path           # find a path to generate request over
                                        # g3 = TPMT to use (0 if none)
        cmpobne 0,g3,.genLRP_100        # Jif path identified
        ldconst diopst_rty,r4           # r4 = waiting to retry I/O state
        ldos    ld_rtycnt(g4),r5        # r5 = error retry count
        stob    r4,dlmio_p_state(g14)   # save state code in LRP
        ldconst TRUE,r4
        addo    1,r5,r5                 # inc. error retry count
        stob    r4,dlm_rtyflg           # set LRP retry flag
        stos    r5,ld_rtycnt(g4)        # save updated retry count
        b       .genLRP_1000            # and get out of here!
#
.genLRP_100:
        ldconst diopst_act,r4           # r4 = I/O active state
        ld      dlmio_p_sgl(g14),r11    # r11 = assoc. SGL address
        ldconst 0,r10                   # r10 = SGL segment count
        stob    r4,dlmio_p_state(g14)   # save state code in LRP
        cmpobe  0,r11,.genLRP_120       # Jif no SGL defined
        ldos    sg_scnt(r11),r10        # r10 = SGL segment count from SGL
.genLRP_120:
        ldconst sr_lrpsgl_recsize,r9    # r9 = size of each SGL record
        mulo    r10,r9,r9               # r9 = SRP size for SGL records
        lda     sr_lrp_size(r9),r9      # r9 = SRP allocation size
        mov     r9,g0                   # g0 = memory allocation size
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        st      g14,dlmio_s1_ilt(g1)    # save LRP I/O ILT in ILT/SRP
        ld      tpm_dtmt(g3),g2         # g2 = assoc. DTMT address of path
        st      g0,dlmio_s1_srpsize(g1) # save SRP allocation size in ILT
        st      g3,dlmio_s1_tpmt(g1)    # save TPMT used in ILT
        st      g1,dlmio_p_ilt(g14)     # save ILT/SRP in LRP/ILT
        ld      dtmt_lldmt(g2),r8       # r8 = assoc. LLDMT (ILT/VRP)
        lda     ILTBIAS(g1),g1          # g1 = ILT at nest level 2
        st      g3,ld_lasttpmt(g4)      # save last TPMT used
        ldob    dlmi_path-ILTBIAS(r8),r6 # r6 = path #
        st      r8,dlmio_s2_iltvrp(g1)  # save assoc. ILT/VRP address in ILT
        st      r6,dlmio_s2_path(g1)    # save path # in ILT
        ldconst vrpsiz,r4               # r4 = size of VRP header
        addo    r4,g0,r4                # r4 = size of the VRP/SRP combo
c       r4 += 2;                        # Add 2 extra bytes so SRP isn't immediately after VRP
c       g0 = s_MallocC(r4, __FILE__, __LINE__); # allocate memory for VRP/SRP combo
                                        # g0 = memory address
        mov     g0,r3                   # r3 = VRP address
        lda     vr_sglhdr(g0),g0        # g0 = SRP
        st      r4,vr_blen(r3)          # save the size in the VRP
        # If the SRP address immediately follows the VRP, then the LL_LinuxLinkLayer.c
        # code will adjust the addresses so that the Target will see the SRP in it's
        # local memory, rather than the SRP in the Initiator memory. Skip 2 bytes
        # between the VRP and SRP to force the address to be in Initiator memory
        # (the equivalent of changing the address to a PCI address for Bigfoot).
c       g0 += 2;                        # Add 2 bytes for "PCI Address translation"
        mov     g0,r4                   # Translate to global address
        st      r4,vr_sglptr(r3)        # save the SRP address in the VRP
        ldconst vrbefesrp,r4            # set the function as a BE to FE SRP
        stos    r4,vr_func(r3)
        st      r9,vr_sglsize(r3)       # save the size of the SRP
        st      r3,dlmio_s2_srpvrp(g1)  # save the VRP address in the ILT
        st      r3,ILTBIAS+vrvrp(g1)    # save the VRP in the next level also
#
        st      g0,dlmio_s2_srp(g1)     # save SRP address in ILT
        ld      lldmt_vrp(r8),r8        # r8 = VRP assoc. with SRP
        ld      vr_ilt(r8),r8           # r8 = ILT of VRP assoc. with SRP
        st      r8,sr_vrpilt(g0)        # Save the ILT/VRP assoc. in the SRP
#
        ldob    dlmio_p_fc(g14),r8      # r7 = I/O function code
        stob    r8,sr_lrp_fc(g0)        # and save in the SRP
#
        stob    srxlrp,sr_func(g0)      # Save SRP function code
        stob    0,sr_flag(g0)           # Clear flag byte
        stob    0,sr_rsvd1(g0)          # Clear reserved byte
        stob    srerr,sr_status(g0)     # Set initial SRP status
        subo    sr_lrp_extstat_size,r9,r5 # r5 = SRP packet length
        st      r5,sr_plen(g0)          # SRP packet length
        ldl     dlmio_p_sda(g14),r6     # r6-r7 = SDA
        st      r6,sr_lrp_sda(g0)       # Save lower 32 bits of SDA address
# Breaking r7 (upper 32 bits of SDA) into sr_lrp_sda_upper3[3] and sr_lrp_sda_upper4th.
        stob    r7,sr_lrp_sda_upper4th(g0);
c       r7 = r7 >> 8;
        stob    r7,sr_lrp_sda_upper3(g0); # NOTE: located after sr_lrp_fc.
c       r7 = r7 >> 8;
        stob    r7,sr_lrp_sda_upper3+1(g0);
c       r7 = r7 >> 8;
        stob    r7,sr_lrp_sda_upper3+2(g0);
        st      g1,sr_ilt(g0)           # ILT address
#
        ld      tpm_lldid(g3),r4        # r4 = LLD session ID
        st      r4,sr_lrp_lldid(g0)     # save the LLD session ID in the SRP
        st      g3,sr_lrp_dlmid(g0)     # save the DLM session ID in the SRP
# --- Clear the unused fields, and set the length.
        st      0,sr_lrp_rsvd2(g0)      # sr_source (cleared)       sr_lrp_rsvd2
        st      0,sr_lrp_rsvd3(g0)      # sr_destination (cleared)  sr_lrp_rsvd3
        ld      dlmio_p_len(g14),r6     # length
        st      r6,sr_lrp_len(g0)       #                           sr_lrp_len
        st      0,sr_lrp_rsvd4(g0)      # sr_rsvd (cleared)         sr_lrp_rsvd4
#
        mulo    sr_lrpsgl_recsize,r10,r8 # r8 = size of SGL segment records
        lda     sr_lrp_sglseg(g0)[r8*1],r4 # r4 = ext. status area address (pci)
        st      r4,sr_lrp_extstat(g0)   # save sr_lrp_extstat
        stob    10,sr_lrp_timeout(g0)   # I/O timeout (secs.) to LLD
# Note: sr_lrp_sda_upper4th saved above with SDA.
        stob    2,sr_lrp_retry(g0)      # I/O retry count to LLD
        st      r10,sr_lrp_scnt(g0)     # SGL segment count
        st      0,sr_lrp_rsvd6(g0)      # clear reserved fields
        st      0,sr_lrp_rsvd6+1(g0)    # clear reserved fields
        addo    sghdrsiz,r8,r8          # r8 = SGL size
        st      r8,sr_lrp_sglsize(g0)   # size of SGL
#
        lda     sr_lrp_sglseg(g0),r8    # r8 = pointer to LRP SGL desc.  records
        lda     sghdrsiz(r11),r11       # r11 = pointer to requestor's SGL desc. records
        cmpobe  0,r10,.genLRP_200       # Jif no more SGL desc. records to build
.genLRP_150:
        movl    0,r6
        ldl     (r11),r4                # r4-r5 = first desc. record in pair
        subo    1,r10,r10               # dec. SGL desc. record count
        lda     sgdescsiz(r11),r11      # inc. to next requestor desc. record
        cmpobe  0,r10,.genLRP_160       # Jif no more SGL desc. records to copy
        ldl     (r11),r6                # r6-r7 = second desc. record in pair
        lda     sgdescsiz(r11),r11      # inc. to next requestor desc. record
        subo    1,r10,r10               # dec. SGL desc. record count
.genLRP_160:
        stq     r4,(r8)                 # save SGL desc. records in SRP
        lda     16(r8),r8               # inc. to next SGL desc. record set
        cmpobne 0,r10,.genLRP_150       # Jif more SGL desc. records to copy
.genLRP_200:
        lda     dlm$LRPcr,r4            # r4 = ILT completion routine
        mov     0,r3
        st      r4,il_cr(g1)            # Save the completion routine
        st      r3,il_fthd(g1)          # Clear the ILT Forward thread
        mov     g14,r5                  # save g14
        call    L$que                   # Queue the ILT to the Link 960 Queue
                                        #   (wipes g0-g3, g14)
        mov     r5,g14                  # restore g14
.genLRP_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: dlm$LRPcr
#
#  PURPOSE:
#       Processes LRP I/O completion events.
#
#  DESCRIPTION:
#       This routine queues the LRP I/O ILT to the dlm$lrpcr task queue
#       and wakes it up if necessary.
#
#  INPUT:
#       g1 = secondary LRP I/O ILT address at nest level 2
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$LRPcr:
        lda     dlm_lrpcr_qu,r11        # Get queue origin
        b       K$cque
#
#******************************************************************************
#
#  NAME: dlm$find_path
#
#  PURPOSE:
#       Finds the path to use to send a LRP I/O over.
#
#  DESCRIPTION:
#       Scans the specified LDD for the best path to send the next
#       LRP I/O over. If the ld_lasttpmt is non-zero, it uses this
#       TPMT to look for the best path. If this value is zero, it
#       starts with the first TPMT on the TPMT list in the LDD.
#
#  INPUT:
#       g4 = LDD address to chose path for
#
#  OUTPUT:
#       g3 = TPMT of path to use
#       g3 = 0 if no paths available to use
#
#  REGS DESTROYED:
#       Reg. g3 destroyed.
#
#******************************************************************************
#
dlm$find_path:
        ldconst 0,g3                    # g3 = TPMT to use (null to start)
        ld      ld_lasttpmt(g4),r4      # r4 = last TPMT used
        cmpobne 0,r4,.findpath_100      # Jif last TPMT defined
        ld      ld_tpmthd(g4),r4        # r4 = first TPMT on list
        cmpobne 0,r4,.findpath_120      # Jif TPMTs defined for LDD
        b       .findpath_1000          # no TPMTs defined for LDD
#
.findpath_100:
        ld      tpm_ntpmt(r4),r4        # r4 = next TPMT on list to start with
.findpath_120:
        ldconst 0xffffffff,r6           # r6 = error count to beat
        mov     r4,r5                   # r5 = starting TPMT address
                                        # r4 = TPMT being checked
        mov     0,r7                    # r7 = best operational path with errors
.findpath_150:
        ldob    tpm_state(r4),r10       # r10 = TPMT state code
        ld      tpm_ecnt(r4),r9         # r9 = TPMT error count
        cmpobne tpm_st_op,r10,.findpath_300 # Jif path not operational
        cmpobne 0,r9,.findpath_200      # Jif error count non-zero
        mov     r4,g3                   # g3 = TPMT of path to use
        b       .findpath_1000          # and we're out of here!
#
.findpath_200:
        cmpobg  r9,r6,.findpath_300     # Jif this path has more errors then the previous operational path tested.
        mov     r4,r7                   # r7 = TPMT with lowest non-zero error count
.findpath_300:
        ld      tpm_ntpmt(r4),r4        # r4 = next TPMT on list
        cmpobne r4,r5,.findpath_150     # Jif more TPMTs to check
#
# --- No operational paths found with no errors reported.
#
        mov     r7,g3                   # g3 = TPMT of path to use
.findpath_1000:
        ret
#
#******************************************************************************
#
#  NAME: dlm$term_LRP
#
#  PURPOSE:
#       Terminates the specified LRP, returning it to the original
#       requestor.
#
#  DESCRIPTION:
#       Removes the LRP ILT from the LDD active queue if necessary,
#       saves off the dependent LRP ILTs and returns the LRP ILT
#       back to the original requestor. If any dependent LRP ILTs
#       were found, it deals with them appropriately.
#
#  INPUT:
#       g4  = assoc. LDD address
#       g14 = LRP I/O ILT address to remove
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$term_LRP:
        mov     g14,r14                 # save g14
        mov     g1,r15                  # save g1
        ld      dlmio_p_deplist(g14),r13 # r13 = dependent LRP ILT list
        mov     g14,g1                  # g1 = LRP I/O ILT
        ld      dlmio_p_cr(g14),r3      # r3 = LRP completion routine
        call    dlm$rem_LRP             # remove LRP ILT from active list
        callx   (r3)                    # and call requestor's completion routine
        cmpobe  0,r13,.termLRP_1000     # Jif no dependent LRP ILTs
        ld      dlmio_p_depcnt(r13),r3  # r3 = dependency count in top LRP ILT
        subo    1,r3,r3                 # dec. dependency count
        st      r3,dlmio_p_depcnt(r13)  # save updated dependency count
        cmpobne 0,r3,.termLRP_1000      # Jif more dependencies exist
        mov     r13,g14                 # g14 = first LRP ILT on dependency list
        ld      il_fthd(r13),r3         # r3 = next LRP ILT on dependency list
!       ldl     dlmio_p_sda(g14),r4     # r4-r5 = LRP I/O SDA
        ld      dlmio_p_len(g14),r7     # r7 = LRP I/O Length
        cmpo    0,1                     # Clear the Carry Bit
        addc    r7,r4,r6                # r6-r7 = LRP I/O EDA
        addc    0,r5,r7
        st      r3,dlmio_p_deplist(g14) # save remainder of dependency list as dependent on this LRP ILT
        cmpobe  0,r3,.termLRP_200       # Jif no dependent LRPs
.termLRP_150:
!       ldl     dlmio_p_sda(r3),r8      # r8-r9 = LRP I/O SDA from dep. ILT
        ld      dlmio_p_len(r3),r11     # r11 = LRP I/O Length from dep. ILT
        cmpo    0,1                     # Clear the Carry Bit
        addc    r11,r8,r10              # r10-r11 = LRP I/O EDA from dep. ILT
        addc    0,r9,r11
        ld      il_fthd(r3),r3          # r3 = next LRP ILT on dependency list
#
# --- Check for lower SDA
#
        cmpobg  r9,r5,.termLRP_170      # Jif dependency SDA(h) > LRP SDA(h)
        bne     .termLRP_160            # Jif dependency SDA(h) != LRP SDA(h)
        cmpobge r8,r4,.termLRP_170      # Jif dependency SDA(l) >= LRP SDA(l)
.termLRP_160:
        movl    r8,r4                   # Update the current precedence SDA
#
# --- Check for greater EDA
#
.termLRP_170:
        cmpobl  r11,r7,.termLRP_190     # Jif dependency EDA(h) < LRP EDA(h)
        bne     .termLRP_180            # Jif dependency EDA(h) != LRP EDA(h)
        cmpoble r10,r6,.termLRP_190     # Jif dependency EDA(l) <= LRP EDA(l)
.termLRP_180:
        movl    r10,r6                  # Update the current precedence EDA
.termLRP_190:
        cmpobne 0,r3,.termLRP_150       # Jif more dependent LRP/ILTs present
.termLRP_200:
        stl     r4,dlmio_p_psda(g14)    # save precedence SDA in ILT
        stl     r6,dlmio_p_peda(g14)    # save precedence EDA in ILT
        call    dlm$q_LRP               # queue LRP I/O request to LDD
        call    dlm$gen_LRP             # generate LRP I/O request
.termLRP_1000:
        mov     r14,g14                 # restore g14
        mov     r15,g1                  # restore g1
        ret
#
#******************************************************************************
#
#  NAME: dlm$chk4lddx
#
#  PURPOSE:
#       Checks if LDD scan process still appropriate.
#
#  DESCRIPTION:
#       Checks if the specified LDD is still registered in the DLM_lddindx
#       area and if the LDD scan process PCB still registered in the
#       associated LDD. If either are false, indicates the LDD scan
#       process is no longer appropriate. If both are true, indicates the
#       LDD scan process is still appropriate.
#
#  INPUT:
#       g0 = assoc. LDD address
#       g1 = DLM_lddindx pointer where LDD should be registered
#       g2 = LDD scan process PCB address
#
#  OUTPUT:
#       g3 = TRUE if LDD scan process still appropriate
#       g3 = FALSE if LDD scan process no longer appropriate
#
#  REGS DESTROYED:
#       Reg. g3 destroyed.
#
#******************************************************************************
#
dlm$chk4lddx:
        ldconst FALSE,g3                # g3 = FALSE return value
        ld      (g1),r4                 # r4 = LDD from DLM_lddindx
        cmpobne g0,r4,.chk4lddx_1000    # Jif LDD no longer registered in DLM_lddindx area
        ld      ld_pcb(g0),r5           # r5 = LDD scan process PCB from LDD
        cmpobne g2,r5,.chk4lddx_1000    # Jif LDD scan process PCB no longer registered in LDD
        ldconst TRUE,g3                 # g3 = TRUE return value
.chk4lddx_1000:
        ret
#
#******************************************************************************
#
#  NAME: dlm$sched_lddx
#
#  PURPOSE:
#       Checks all defined LDDs for a match to the specified DTMT
#       and if matched schedules the LDD scan process for the associated
#       linked device.
#
#  DESCRIPTION:
#       Determines the target type of the specified DTMT and scans for
#       linked devices that relate to the DTMT and when found schedules
#       the LDD scan process for that linked device.
#
#  INPUT:
#       g4 = DTMT to find linked devices associated to
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$sched_lddx:
        movq    g0,r12                  # save g0-g3
        ldconst MAXLDDS,r11             # r11 = max. # LDDs supported
        ldob    dtmt_type(g4),r10       # r10 = target type code from DTMT
        mov     0,r9                    # r9 = linked device #
.schedlddx_100:
        ld      DLM_lddindx[r9*4],g0    # g0 = LDD from table
        cmpobe  0,g0,.schedlddx_300     # Jif no LDD defined
        ldob    ld_state(g0),r4         # r4 = LDD state
        cmpobe  ldd_st_op,r4,.schedlddx_150 # Jif LDD state is operational
        cmpobne ldd_st_uninit,r4,.schedlddx_300 # Jif LDD state not uninitialized
#
# --- Linked device is either operational or uninitialized
#
.schedlddx_150:
        ldob    ld_class(g0),r5         # r5 = LDD class code
        cmpobne dtmt_ty_MAG,r10,.schedlddx_200 # Jif DTMT type not MAGNITUDE link
#
# --- DTMT type is XIOtech Controller link
#
        cmpobne ldmld,r5,.schedlddx_300 # Jif LDD not MAGNITUDE link
        ld      dml_sn(g4),r4           # r4 = DTMT MAG serial #
        ldob    dml_cl(g4),r6           # r6 = DTMT cluster #
        ld      ld_basesn(g0),r5        # r5 = LDD MAG serial #
        ldob    ld_basecl(g0),r7        # r7 = LDD cluster #
        cmpobne r4,r5,.schedlddx_300    # Jif wrong MAG serial #
        cmpobne r6,r7,.schedlddx_300    # Jif wrong cluster #
        call    DLM$proc_ldd            # schedule LDD scan process for this linked device
        b       .schedlddx_300          # and check next LDD in table
#
# --- DTMT type is Foreign Target
#
.schedlddx_200:
        cmpobne ldftd,r5,.schedlddx_300 # Jif LDD not Foreign Target
        ldl     ld_vendid(g0),r4        # r4-r5 = vendor ID from LDD
        ldl     dft_venid(g4),r6        # r6-r7 = vendor ID from DTMT
        cmpobne r4,r6,.schedlddx_300    # Jif vendor IDs don't match
        cmpobne r5,r7,.schedlddx_300    # Jif vendor IDs don't match
        ldl     ld_prodid(g0),r4        # r4-r5 = bytes 1-8 of product ID from LDD
        ldl     dft_prid(g4),r6         # r6-r7 = bytes 1-8 of product ID from DTMT
        cmpobne r4,r6,.schedlddx_300    # Jif product IDs don't match
        cmpobne r5,r7,.schedlddx_300    # Jif product IDs don't match
        ldl     ld_prodid+8(g0),r4      # r4-r5 = bytes 9-16 of product ID from LDD
        ldl     dft_prid+8(g4),r6       # r6-r7 = bytes 9-16 of product ID from DTMT
        cmpobne r4,r6,.schedlddx_300    # Jif product IDs don't match
        cmpobne r5,r7,.schedlddx_300    # Jif product IDs don't match
        call    DLM$proc_ldd            # schedule LDD scan process for this linked device
.schedlddx_300:
        addo    1,r9,r9                 # inc. LDD number
        cmpobne r9,r11,.schedlddx_100   # Jif more LDDs to check
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: dlm$send_sif
#
#  PURPOSE:
#       Sends a Set Initiator Flags SRP.
#
#  DESCRIPTION:
#       Checks if the specified Path has an associated LLDMT and if
#       not simply returns to the caller. If it does have an
#       associated LLDMT, allocates resources, builds and sends a
#       Set Initiator Flags SRP to the specified LLD.
#
#  INPUT:
#       g0 = initiator flags word to send
#       g1 = path # to send SRP to
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$send_sif:
        movq    g0,r12                  # save g0-g3
                                        # r12 = initiator flags word
                                        # r13 = path #
        ld      dlm_lldmt_dir[g1*4],g3  # g3 = assoc. LLDMT for specified path #
        cmpobe  0,g3,.sendsif_1000      # Jif no LLDMT associated with specified path #
        ld      lldmt_vrp(g3),r11       # get the vrp associated with this SRP
        cmpobe  0,r11,.sendsif_1000     # Jif no VRP associated with this SRP
#
# --- Allocate, build and send Set Initiator Flags SRP
#
        ldconst vrpsiz,r3               # r3 = size of VRP header
        ldconst sr_sif_size,r9          # r9 = size of SRP
        addo    r3,r9,g0                # g0 = size of the VRP/SRP combo
c       g0 += 2;                        # Add 2 extra bytes so SRP isn't immediately after VRP
        mov     g0,r4                   # r4 = size of the VRP/SRP combo
c       g0 = s_MallocC(g0, __FILE__, __LINE__); # allocate memory for SRP
                                        # g0 = memory address
        mov     g0,r3                   # r3 = VRP (save for later use)
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        lda     vr_sglhdr(g0),g0        # g0 = SRP
        # If the SRP address immediately follows the VRP, then the LL_LinuxLinkLayer.c
        # code will adjust the addresses so that the Target will see the SRP in it's
        # local memory, rather than the SRP in the Initiator memory. Skip 2 bytes
        # between the VRP and SRP to force the address to be in Initiator memory
        # (the equivalent of changing the address to a PCI address for Bigfoot).
c       g0 += 2;                        # Add 2 bytes for "PCI Address translation"
        mov     g0,r8                   # Translate local to global for FE
        st      r8,vr_sglptr(r3)        # save the SRP address in the VRP
        ldconst vrbefesrp,r8            # set the function as a BE to FE SRP
        stos    r8,vr_func(r3)
        st      r4,vr_blen(r3)          # save the size in the VRP/SRP
        st      r9,vr_sglsize(r3)       # save the size of the SRP
#
        ld      lldmt_vrp(g3),r11       # get the vrp associated with this SRP
        ld      vr_ilt(r11),r11         # get the ILT/VRP on the FE
        st      r11,sr_vrpilt(g0)       # save the ILT/VRP in the srp
#
        mov     g1,r11                  # r11 = ILT/SRP address
        ldconst sriflgs,r8              # r8 = SRP function code
        mov     0,r10                   # r10 = count of descriptors (zero)
        stq     r8,sr_func(g0)          # save sr_func
                                        #  + sr_plen
                                        #  + count
                                        #  + sr_ilt
        st      r12,sr_sif_flags(g0)    # save the sr_sif_flags in the SRP
        st      r13,sr_sif_channel(g0)  # save the channel to apply it to
        st      g0,dlmi_srp(g1)         # save SRP address in ILT
        st      g3,il_w3(g1)            # save assoc. ILT/VRP address in ILT
        st      r3,dlmi_srpvrp(g1)      # save the VRP address in the ILT
        st      r3,ILTBIAS+vrvrp(g1)    # save the VRP in the next level also
        st      r13,dlmi_path(g1)       # save path # in ILT
        lda     dlm$sif_cr,r4           # r4 = ILT completion handler routine
        st      r4,il_cr(g1)            # Save the completion routine
        st      r10,il_fthd(g1)         # Clear the ILT Forward thread
        mov     g14,r3                  # save g14
        call    L$que                   # Queue the ILT to the Link 960 Queue
                                        #   (wipes g0-g3, g14)
        mov     r3,g14                  # restore g14
.sendsif_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: dlm$sif_cr
#
#  PURPOSE:
#       Set Initiator Flags SRP ILT completion handler routine.
#
#  DESCRIPTION:
#       Deallocates the resources used for the Set Initiator Flags
#       SRP and returns to the caller.
#
#  INPUT:
#       g1 = ILT/SRP at nest level #1
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$sif_cr:
        movl    g0,r12                  # save g0-g1
        ld      dlmi_srpvrp(g1),g0      # g0 = memory used for the VRP/SRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        ld      vr_blen(g0),g1          # g1 = size of the VRP/SRP combo
c       s_Free_and_zero(g0, g1, __FILE__, __LINE__); # Free SRP memory
        movl    r12,g0                  # restore g0-g1
        ret
#
#******************************************************************************
#
#  NAME: dlm$send_stf
#
#  PURPOSE:
#       Sends a Set Target Flags SRP.
#
#  DESCRIPTION:
#       Checks if the specified Path has an associated LLDMT and if
#       not simply returns to the caller. If it does have an
#       associated LLDMT, allocates resources, builds and sends a
#       Set Target Flags SRP to the specified LLD.
#
#  INPUT:
#       g0 = target flags word to send
#       g1 = path # to send SRP to
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$send_stf:
        movq    g0,r12                  # save g0-g3
                                        # r12 = target flags word
                                        # r13 = path #
        ld      dlm_lldmt_dir[g1*4],g3  # g3 = assoc. LLDMT for specified path #
        cmpobe  0,g3,.sendstf_1000      # Jif no LLDMT associated with specified path #
#
# --- Allocate, build and send Set Target Flags SRP
#
        ldconst vrpsiz,r3               # r3 = size of VRP header
        ldconst sr_stf_size,r9          # r9 = size of SRP
        addo    r3,r9,g0                # g0 = size of the VRP/SRP combo
c       g0 += 2;                        # Add 2 extra bytes so SRP isn't immediately after VRP
        mov     g0,r4                   # r4 = size of the VRP/SRP combo
c       g0 = s_MallocC(g0, __FILE__, __LINE__); # allocate memory for SRP
                                        # g0 = memory address
        mov     g0,r3                   # r3 = VRP (save for later use)
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        lda     vr_sglhdr(g0),g0        # g0 = SRP
        # If the SRP address immediately follows the VRP, then the LL_LinuxLinkLayer.c
        # code will adjust the addresses so that the Target will see the SRP in it's
        # local memory, rather than the SRP in the Initiator memory. Skip 2 bytes
        # between the VRP and SRP to force the address to be in Initiator memory
        # (the equivalent of changing the address to a PCI address for Bigfoot).
c       g0 += 2;                        # Add 2 bytes for "PCI Address translation"
        mov     g0,r8                   # Translate to global address
        st      r8,vr_sglptr(r3)        # save the SRP address in the VRP
        ldconst vrbefesrp,r8            # set the function as a BE to FE SRP
        stos    r8,vr_func(r3)
        st      r4,vr_blen(r3)          # save the size in the VRP/SRP
        st      r9,vr_sglsize(r3)       # save the size of the SRP
#
        ld      lldmt_vrp(g3),r11       # get the vrp associated with this SRP
        ld      vr_ilt(r11),r11         # get the ILT/VRP on the FE
        st      r11,sr_vrpilt(g0)       # save the ILT/VRP in the srp
#
        mov     g1,r11                  # r11 = ILT/SRP address
        ldconst srtflgs,r8              # r8 = SRP function code
        mov     0,r10                   # r10 = count of descriptors (zero)
        stq     r8,sr_func(g0)          # save sr_func
                                        #  + sr_plen
                                        #  + sr_count
                                        #  + sr_ilt
        st      r12,sr_stf_flags(g0)    # save the STF Flag in the SRP
        st      g0,dlmi_srp(g1)         # save SRP address in ILT
        st      g3,il_w3(g1)            # save assoc. ILT/VRP address in ILT
        st      r3,dlmi_srpvrp(g1)      # save the VRP address in the ILT
        st      r3,ILTBIAS+vrvrp(g1)    # save the VRP in the next level also
        st      r13,dlmi_path(g1)       # save path # in ILT
        lda     dlm$stf_cr,g2           # g2 = ILT completion handler routine
        st      g2,il_cr(g1)            # save the ILT completion routine
        st      r10,il_fthd(g1)         # Clear the ILT Forward thread
        mov     g14,r3                  # save g14
        call    L$que                   # Queue the ILT to the Link 960 Queue
                                        #   (wipes g0-g3, g14)
        mov     r3,g14                  # restore g14
.sendstf_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: dlm$stf_cr
#
#  PURPOSE:
#       Set Target Flags SRP ILT completion handler routine.
#
#  DESCRIPTION:
#       Deallocates the resources used for the Set Target Flags
#       SRP and returns to the caller.
#
#  INPUT:
#       g1 = ILT/SRP at nest level #1
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$stf_cr:
        movl    g0,r12                  # save g0-g1
        ld      dlmi_srpvrp(g1),g0      # g0 = memory used for the VRP/SRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        ld      vr_blen(g0),g1          # g1 = size of the VRP/SRP combo
c       s_Free_and_zero(g0, g1, __FILE__, __LINE__); # Free SRP memory
        movl    r12,g0                  # restore g0-g1
        ret
#
#******************************************************************************
#
#  NAME: dlm$chk_paths
#
#  PURPOSE:
#       Checks the specified LDD if all possible paths have been
#       established.
#
#  DESCRIPTION:
#       For LDDs associated with XIOtech Controller links, checks if the
#       current number of paths is >= the maximum possible number
#       of paths and if not returns a FALSE indication back to the
#       calling routine.
#
#  INPUT:
#       g8 = assoc. LDD address of VLink
#
#  OUTPUT:
#       g0 = FALSE if not all possible paths have been established
#       g0 = TRUE if all possible paths have been established
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#******************************************************************************
#
dlm$chk_paths:
        ldconst TRUE,g0                 # g0 = return value
        ldob    ld_class(g8),r4         # r4 = device class
        cmpobne ldmld,r4,.chkpaths_1000 # Jif not link class of device
        ld      ld_basesn(g8),r14       # r14 = Controller serial #
        ldob    ld_basecl(g8),r13       # r13 = Controller cluster #
        ldconst 0,r4                    # r4 = current path count
        ld      ld_tpmthd(g8),r7        # r7 = first TPMT on list
        cmpobe  0,r7,.chkpaths_200      # Jif no paths established
        mov     r7,r8                   # r8 = current TPMT being processed
.chkpaths_100:
        addo    1,r4,r4                 # inc. current path count
        ld      tpm_ntpmt(r8),r8        # r8 = next TPMT on list
        cmpobne r8,r7,.chkpaths_100     # Jif not the first TPMT on list
.chkpaths_200:
        ld      dlm_mlmthd,r12          # r12 = first MLMT on list
.chkpaths_300:
        cmpobe  0,r12,.chkpaths_1000    # Jif no MLMTs on list
        ld      mlmt_sn(r12),r11        # r11 = MLMT Controller serial #
        cmpobe  r11,r14,.chkpaths_400   # Jif MLMT for assoc. Controller
        ld      mlmt_link(r12),r12      # r12 = next MLMT on list
        b       .chkpaths_300           # and check next MLMT for match
#
.chkpaths_400:
        ld      mlmt_dtmthd(r12),r11    # r11 = first DTMT assoc. with MLMT
        cmpobe  0,r11,.chkpaths_1000    # Jif no DTMTs assoc. with MLMT
        ldconst 0,r5                    # r5 = max. possible paths for VLink
.chkpaths_450:
        ldob    dml_cl(r11),r7          # r7 = assoc. cluster # for DTMT
        cmpobne r7,r13,.chkpaths_500    # Jif not the correct cluster #
        addo    1,r5,r5                 # inc. possible path count
.chkpaths_500:
        ld      dml_mllist(r11),r11     # r11 = next DTMT on list
        cmpobne 0,r11,.chkpaths_450     # Jif more DTMTs on list to check
        cmpobge r4,r5,.chkpaths_1000    # Jif current path count >= possible path count
        ldconst FALSE,g0                # indicate less possible paths then are possible
.chkpaths_1000:
        ret
#
#******************************************************************************
#
#  NAME: DLM$chk_master
#
#  PURPOSE:
#       Checks if the controller is the current master controller of
#       the group and if not will return the current master controller
#       serial number.
#
#  DESCRIPTION:
#       Checks if the controller is the current master controller of
#       the group. If it is, it returns 0 in the response return value.
#       If it is not the current master controller, if the master controller
#       is known, it will return the current master controller serial number.
#       If the current master controller of the group is not known, it will
#       return the group serial number.
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g0 = 0 if the current group master controller
#       g0 = group serial number if not the master and the master is not
#            currently known
#       g0 = current group master controller serial number if not me and
#            the current master controller is known
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#******************************************************************************
#
DLM$chk_master:
        ldconst 0,g0                    # g0 = return value if we're the current group master controller
        ldos    K_ii+ii_status,r4       # Get initialization status
        bbs     iimaster,r4,.chkmaster_1000 # Jif I'm the current master
        ld      DLM_master_sn,g0        # g0 = current group master controller serial number if known
        cmpobne 0,g0,.chkmaster_1000    # Jif group master controller serial number is known
        ld      K_ficb,r3               # FICB
        ld      fi_vcgid(r3),g0         # g0 = group serial number
.chkmaster_1000:
        ret
#
#******************************************************************************
#
#  NAME: dlm$vlop_validate
#
#  PURPOSE:
#       Validates a VLOP.
#
#  DESCRIPTION:
#       Checks that a VLOP that is registered for a VLink OPEN process
#       is still valid. It uses the 16-bit VLink # specified in vlop_r0
#       and validates that the RDD address specified in vlop_rdd is
#       owned by the associated VDD address. If not, it aborts the
#       VLOP.
#
#  INPUT:
#       g5 = VLOP address to validate
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#******************************************************************************
#
dlm$vlop_validate:
        ldos    vlop_r0(g5),r4          # r4 = 16-bit VLink #
        ld      V_vddindx[r4*4],r5      # r5 = VDD address of specified VLink
        cmpobe  0,r5,.vlopval_500       # Jif VDD address not specified
        ld      vd_rdd(r5),r6           # r6 = RDD address assoc. with VDD
        ld      vlop_rdd(g5),r7         # r7 = RDD address assoc. with VLOP
        cmpobe  r6,r7,.vlopval_1000     # Jif RDD address assoc. with VDD
#
# --- Abort VLOP process.
#
.vlopval_500:
        ld      vlop_ehand(g5),r4       # r4 = VLOP event handler table
        ld      vlop_eh_abort(r4),r5    # r5 = abort event handler routine
        callx   (r5)                    # call abort event handler routine
.vlopval_1000:
        ret
#
.endif  # MAG2MAG
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
