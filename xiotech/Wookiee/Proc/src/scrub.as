# $Id: scrub.as 161041 2013-05-08 15:16:49Z marshall_midden $
#**********************************************************************
#
#  NAME: scrub.as
#
#  PURPOSE:
#
#       To implement the RAID scrub and scan features.
#
#  FUNCTIONS:
#
#       R$stop      - Stop scrub activity
#       R$resume    - Resume scrub activity
#       R$chkstop   - Stop checker activity
#       R$chkresume - Resume checker activity
#
#       This module employs these processes:
#
#       r$scrub     - Scrubber
#       r$checker   - RAID checker (1 copy)
#
#  Copyright (c) 1996-2008 Xiotech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  d$setscrub              # Scrub MRP
        .globl  R$stop                  # Stop scrubber
        .globl  R$resume                # Resume scrubber
        .globl  R$chkstop               # Stop checker
        .globl  R$chkresume             # Resume checker
        .globl  r$scrub                 # Scrub process
        .globl  r$checker               # Scan process
#
# --- global data declarations ----------------------------------------
#
        .globl  R_scrubopt              # Scrub enable T/F
        .globl  gScrubOpt               # C global access
        .globl  R_pcmisc                # Total num of parity miscomp's
        .globl  R_pcctrl                # Parity check control word
        .globl  R_pcpass                # Num of passes through parity checker
        .globl  R_pccurraid             # Current raid under test
        .globl  R_pcstartraid           # Starting RID for specific test
        .globl  r_prevctrl              # Previous scan control value

        .globl  r_scrub_pcb             # Scrubbing PCB for debug
        .globl  r_checker_pcb           # Parity/mirror scan PCB for debug
#
# --- global usage data definitions -----------------------------------
#
        .data
#
# --- local usage data definitions ------------------------------------
#
        .align  2                       # Align to word address
#
# --- Scrub Data
#     These scrub variables are defined as a structure in debug.c so
#     the order and types must stay the same. r_scrub_pcb must be first.
#
r_scrub_pcb:
        .word   0                       # Scrubbing PCB
r_src:
        .word   0                       # Scrub operations outstanding
r_curraid:
        .word   0                       # RID currently being scrubbed
r_stopcnt:
        .byte   0                       # Scrub stop count
r_change:
        .byte   FALSE                   # Configuration change

gScrubOpt:
R_scrubopt:
        .byte   FALSE                   # Scrub enable
        .byte   0                       # unused
#
# --- Parity / Mirror Scan Data (Checker)
#     These checker variables are defined as a structure in debug.c so
#     the order and types must stay the same. r_checker_pcb must be first.
#
        .align  2                       # Align to word address
# quad 1
r_checker_pcb:
        .word   0                       # Checker PCB
R_pcmisc:
        .word   0                       # Total num of parity miscomp's
R_pcctrl:
        .word   0                       # Parity check control word
R_pcpass:
        .word   0                       # Num of passes through parity checker
# quad 2
R_pccurraid:
        .word   0                       # Current raid under test
R_pcstartraid:
        .word   0                       # Starting RAID for a single RAID test
r_ecinvfunc:
        .word   0                       # Checker invalid function error count
r_ecreserved:
        .word   0                       # Checker bad PSD error count
# quad 3
r_ecother:
        .word   0                       # Checker other error count
r_correct:                              # Parity correction enabled (T/F)
        .word   0
r_prevctrl:                             # Previous parity scan control input
        .word   0
r_chkstopcnt:
        .byte   0                       # Checker process stop count
# quad 4
r_chklock:
        .byte   0                       # Checker process lock
        .byte   0,0                     # unused
#
        .align  2                       # align to word address
#
# --- Scrubinfo data
#
        .align  2                       # align to word address
#
r_scrub_block:
        .long   0                       # Block of drive getting scrubbed
r_scan_block:
        .long   0                       # Block of raid getting scanned
r_scrub_pid:
        .short  0                       # PID of drive getting scrubbed
#
# --- executable code -------------------------------------------------
#
        .text
#
#**********************************************************************
#
#  NAME: d$setscrub
#
#  PURPOSE:
#
#       To provide a means of enabling or disabling the scrubber and
#       parity/mirror scanner for the mrscrubctrl MRP.
#
#  DESCRIPTION:
#
#       The packet size and enable/disable option parameters are
#       validated with the new setting being established should these
#       checks be successful.
#
#  CALLING SEQUENCE:
#
#       call    d$setscrub
#
#  INPUT:
#
#       g0 = MRP
#
#  OUTPUT:
#
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#
#       None.
#
#**********************************************************************
#
d$setscrub:
#
        ld      mr_rptr(g0),r15         # Return data pointer
        ld      mr_ptr(g0),g0           # Get the parm block pointer
#
# --- Handle scrubbing (master only)
#
        ldos    K_ii+ii_status,r3       # Get bit map status
        bbc     iimaster,r3,.dsc20      # Jif slave
#
        ld      med_scrub(g0),r9        # Get scrub control bits
        bbc     medchange,r9,.dsc20     # Jif no scrub change requested
        stob    r9,R_scrubopt           # Save new state - 1 byte (T/F)
        stob    r9,K_ii+ii_scrub
#
        call    D$p2updateconfig        # Update NVRAM part II
#
# --- Handle scanning
#
.dsc20:
        ld      med_scan(g0),r8         # Get scan control bits
        ldos    med_rid(g0),r7          # Get specific RAID ID
        bbc     medchange,r8,.dsc100    # Jif no scan change requested
        st      r8,R_pcctrl             # Set the control
#
        bbc     rdpcdefault,r8,.dsc30   # Default overrides all other options
        ldconst rdpcdefctl,r5
        st      r5,R_pcctrl             # Initialize the checker control
        b       .dsc100
#
.dsc30:
        bbc     rdpcspecific,r8,.dsc100 # Jif no specific RAID ID selected
        st      r7,R_pcstartraid        # Save requested RID as a word
#
# --- Always return current state
#
.dsc100:
        ldob    R_scrubopt,r14          # Get current scrub state (byte)
        ld      R_pcctrl,r13            # Get current scan state
        ld      R_pcpass,r12
!       st      r14,med_rscrub(r15)     # Return scrub state (word)
!       st      r13,med_rscan(r15)      # Scan state
!       st      r12,med_passes(r15)     # Pass count
#
        ldl      r_scan_block,r10        # r10/r11 Get current scanned block
# NOTE: PDISK>2TB problem
!       st      r11,med_scanb(r15)      # Return scan block
        ldl     r_scrub_block,r12       # r12/r13 Get current scrubbed block
# NOTE: PDISK>2TB problem
!       st      r13,med_scrubb(r15)     # Return scrub block
        ldos    R_pccurraid,r13         # Get current scanned RID
!       stos    r13,med_scanr(r15)      # Return scan RID
        ldos    r_scrub_pid,r14         # Get current scrubbed PID
!       stos    r14,med_scrubp(r15)     # Return scrub PID
#
# --- Exit
#
        mov     deok,g1                 # Return OK status
        ldconst medrsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: R$stop
#
#  PURPOSE:
#
#       To provide a common means of stopping all scrubbing activity
#       invoked within the RAID layer.
#
#  DESCRIPTION:
#
#       The stop counter is incremented and a check is made for any
#       outstanding I/O.  When all outstanding I/O completes, this
#       routine returns to the caller.  While the stop counter is
#       non-zero, the executive is effectively blocked.
#
#       This routine may only be called from the process level.
#
#  CALLING SEQUENCE:
#
#       call    R$stop
#
#  INPUT:
#
#       None.
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#**********************************************************************
#
R$stop:
#
        mov     g0,r15                  # Save g0
#
# --- Bump stop counter
#
        ldob    r_stopcnt,r3            # Bump stop counter
        addo    1,r3,r3
        stob    r3,r_stopcnt
#
# --- Stall until pending I/Os complete
#
.st10:
        ld      r_src,r4                # Get outstanding request count
        cmpobe  0,r4,.st20              # Jif none
#
        ldconst 1,g0                    # Delay for minimum time
        call    K$twait
        b       .st10
#
# --- Flag potential configuration change
#
.st20:
        mov     TRUE,r3                 # Set configuration change
        stob    r3,r_change
#
# --- Exit
#
        mov     r15,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: R$resume
#
#  PURPOSE:
#
#       To provide a common means of resuming all scrubbing activity
#       invoked through the RAID layer.
#
#  DESCRIPTION:
#
#       The stop counter is decremented and an immediate return is
#       made to the caller.  When this counter has returned to zero,
#       the executive is unblocked.
#
#  CALLING SEQUENCE:
#
#       call    R$resume
#
#  INPUT:
#
#       None.
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#**********************************************************************
#
R$resume:
#
# --- Adjust stop counter
#
        ldob    r_stopcnt,r3            # Bump stop counter
        subo    1,r3,r3
        stob    r3,r_stopcnt
        cmpobne 0,r3,.rm100             # Jif additional stops
#
c       TaskReadyByState(pcsem2wait);   # Ready semaphore 2 wait
#
.rm100:
        ret
#
#**********************************************************************
#
#  NAME: r$scrub
#
#  PURPOSE:
#       To provide an automated means of scrubbing all physical disks
#       associated with each RAID array in the background.
#
#  DESCRIPTION:
#       This process performs an initial delay to permit the running of
#       benchmarks at full performance immediately after a system boot.
#       Each RAID array is scanned with the storage defined by each
#       PSD.  A verify checkword command is used to validate the
#       integrity of the data.  Verify activity is regulated by the
#       amount of queue depth generated within the cache layer.  The size
#       of each operation is 1 allocation unit (1 MB).
#
#       This process runs continually and never exits.  When a complete
#       scrub cycle has completed the next cycle starts. This only scrubs
#       RAIDs owned by this controller.
#
#  CALLING SEQUENCE:
#       fork this process
#
#  INPUT:
#       None.
#
#**********************************************************************
#
r$scrub:
#
# --- Don't start scrubbing until the system is initialized & scrub is enabled
#
.rs10:
        ldconst 30*1000,g0              # Delay 30 seconds
        call    K$twait
#
        ldob    O_p2init,r3             # Get phase II inits complete
        cmpobne TRUE,r3,.rs10           # Jif not completed
#
        ldob    R_scrubopt,r3           # Get scrub enable
        cmpobe  FALSE,r3,.rs10          # Jif disabled
#
# --- Prepare to scan all RAID arrays
#
        mov     0,g14                   # Start with lowest array
        mov     FALSE,r15               # Indicate nothing scrubbed for now
        mov     0,g7                    # Clear previous queue depth
#
# --- Examine next RAID array
#
.rs20:
        st      g14,r_curraid           # Save current RID for debug
        mov     FALSE,r3                # Clear configuration change
        stob    r3,r_change
#
        ld      R_rddindx[g14*4],g13    # g13 = next RDD
        cmpobe  0,g13,.rs100            # Do next RDD if undefined
#
        ldconst rdlinkdev,r3            # Check for Vlink
        ldob    rd_type(g13),r4         # Get the type
        cmpobe  r3,r4,.rs100            # Skip over Vlinks.
        cmpobe  rdslinkdev,r4,.rs100    # and skip over snapshots
#
        ldos    rd_vid(g13),g0          # Get the VID
#
        PushRegs(r3)                    # Save regs
        call    DL_AmIOwner             # Determine if we are the owner or not
        PopRegs(r3)                     # Restore regs
#
        cmpobe  FALSE,g0,.rs100         # Do next RDD if not owned
#
        ldob    rd_status(g13),r3       # Get array status
        cmpobg  rdop,r3,.rs100          # Jif not operational
#
        ld      rd_psd(g13),g12         # Get 1st PSD
        mov     g12,g11
c       *(UINT64*)&g8 = ((PSD*)g12)->sLen; # Get segment length
#
# --- Process next PSD
#
.rs30:
c       *(UINT64*)&g4 = 0;              # Reset SDA g4/g5
#
# --- Prepare to scrub next segment
#
.rs40:
        ldob    R_scrubopt,r3           # Get scrub enable
        cmpobe  FALSE,r3,.rs10          # Jif disabled
#
        ldob    ps_status(g11),r3       # Get segment status
        cmpobne psop,r3,.rs80           # Jif not OK - next PSD
#
        ld      P_orc,r4                # Get outstanding queue depth
                                        #  from physical layer
        addo    r4,g7,r3                # Combine current and previous
        mov     r4,g7                   # Save previous queue depth
        cmpobe  0,r3,.rs50_s            # Jif no activity
#
        ldconst SCRUBSCALE,r4           # Calculate appropriate delay
        mulo    r3,r4,g0                #  based on cache layer activity
        ldconst SCRUBMIN,r5             # Set minimum delay
        ldconst SCRUBMAX,r6             # Set maximum delay
        cmpo    g0,r5                   # Check minimum delay
        sell    g0,r5,g0                # Set minimum if required
        cmpo    g0,r6                   # Check maximum delay
        selg    g0,r6,g0                # Set maximum if required
        call    K$twait
#
.rs50_s:
        ldob    r_stopcnt,r3            # Check for stop requested
        cmpobe  0,r3,.rs60_s            # Jif not
#
c       TaskSetMyState(pcsem2wait);     # Set this process to wait semaphore 2
        call    K$xchang
        b       .rs20
#
.rs60_s:
        ldob    r_change,r3             # Get configuration change
        cmpobne FALSE,r3,.rs20          # Jif so
#
        ld      r_src,r3                # Bump scrub count
        addo    1,r3,r3
        st      r3,r_src
#
# --- Scrub 1 allocation unit (1 MB)
#
        mov     TRUE,r15                # Indicate scrubbing has started
        ldos    ps_pid(g11),g3          # Get PID
        stos    g3,r_scrub_pid          # Store pid getting scrubbed
        ld      P_pddindx[g3*4],g3      # Lookup PDD to pass
c       *(UINT64*)&r4 = *(UINT64*)&g4 + ((PSD*)g11)->sda; # Base SDA + sector working on.
c   if ((*((UINT64*)&r4) & ~0xffffffffULL) != 0ULL) { # If 16 byte command
        lda     O_t_verify1_16,g0          # Pass verify template
c   } else {
        lda     O_t_verify1,g0          # Pass verify template
c   }
        call    O$genreq                # Build request
#
c   if ((*((UINT64*)&r4) & ~0xffffffffULL) != 0ULL) { # If 16 byte command
c       *((UINT64*)&r12) = bswap_64(*(UINT64*)&r4); # Set up SDA in SCSI command
        stq     r12,pr_cmd+2(g2)
c   } else {
        bswap   r4,r3                   # Set up SDA in SCSI command
        st      r3,pr_cmd+2(g2)
c   }
        ldconst prlow,r3                # Set strategy to low priority
        stob    r3,pr_strategy(g2)
        st      g11,il_w4(g1)           # Record PSD
#
        stl     r4,r_scrub_block        # r4/r5 Store block getting scrubbed
c       *(UINT64*)&r6 = *(UINT64*)&r4 + DSKSALLOC; # Set up EDA
        stl     r4,pr_sda(g2)
        stl     r6,pr_eda(g2)
        call    O$quereq                # Initiate verify
#
        ld      r_src,r3                # Decrement scrub count
        subo    1,r3,r3
        st      r3,r_src
#
        call    M$chkstat               # Check status
        call    O$relreq                # Release request
        cmpobe  ecok,g0,.rs70_s         # Jif OK
#
# --- Error handler
#     Flag the PDD as having an error and queue an ILT to the error handler.
#     The RAID error handler has the logic to determine if this is a single
#     drive failure that should be rebuilt or if the entire RAID has become
#     inoperable.
#
        ldob    rd_type(g13),r3         # Get type
        cmpobe  rdstd,r3,.rs100         # Jif standard device or
        cmpobe  rdlinkdev,r3,.rs100     # Jif linked or
        cmpobe  rdslinkdev,r3,.rs100    # Jif snapshot or
        cmpobe  rdraid0,r3,.rs100       # Jif RAID 0 - ignore error and
                                        #  go to the next RAID
#
        mov     g11,g0                  # Pass PSD
        mov     g13,g1                  # Pass RDD
        call    RB$rerror               # Flag error in this PDD
#
c       g1 = get_ilt();                 # Allocate an ILT.
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif  # M4_DEBUG_ILT
c       g2 = get_rrp();                 # Allocate an RRP.
.ifdef M4_DEBUG_RRP
c CT_history_printf("%s%s:%u get_rrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif  # M4_DEBUG_RRP
c       ((ILT *)g1)->ilt_normal.w0 = g2; # Link to RRP in ILT.
        lda     ILTBIAS(g1),g1          # Advance to next ILT level
        ldconst 0,r3
.if 1 #VIJAY_MC
        st      r3,rr_flags(g2)
.endif  # 1
        ldconst rrverify,r4
        st      r3,rr_sglptr(g2)        # Set SGL = 0 when we release ILT/RRP
        st      r3,rr_sglsize(g2)
        st      r3,il_w5(g1)            # Pass NVA entry = 0
        stos    r4,rr_func(g2)          # A verify op failed
        st      g13,il_w4(g1)           # Pass RDD
        lda     RB$rerror_comp,r3       # Pass completion routine
        st      r3,il_cr(g1)
        call    RB$rerror_que           # Queue this ILT to error handler
#
        b       .rs100                  # Jump to next RAID
#
# --- Advance to next allocation unit
#
.rs70_s:
c       *(UINT64*)&g4 += DSKSALLOC;     # Adjust SDA
c       if (*(UINT64*)&g4 != *(UINT64*)&g8) {
            b   .rs40                   # Jif more to do
c       }
#
# --- Advance to next PSD
#
.rs80:
        ld      ps_npsd(g11),g11        # Advance to next PSD
        cmpobne g11,g12,.rs30           # Jif more
#
# --- Advance to next RAID array
#
.rs100:
        addo    1,g14,g14               # Advance to next array
        ldconst MAXRAIDS,r3             # Check for complete scan
        cmpobne r3,g14,.rs20            # Jif not complete scan
#
        cmpobe  FALSE,r15,.rs10         # Jif nothing scrubbed
#
        ldconst mlescrubdone,g0         # Log message to CCB
        call    O_logerror
        b       .rs10
#
#**********************************************************************
#
#  NAME: r$initchecker
#
#  PURPOSE:
#
#       To provide a means of initializing all data
#       associated with the parity checking algorithm.
#
#  DESCRIPTION:
#
#       This process initializes all data necessary for using the parity
#       checker.  Members of the RDD as well as permanent memory locations
#       are cleared.
#
#  CALLING SEQUENCE:
#
#       process call
#
#  INPUT:
#
#       None.
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#**********************************************************************
#
r$initchecker:
        movl    0,r4                    # Set up a zero
#
        st      r4,R_pcpass             # Clear the pass count.
        st      r4,R_pcmisc             # Clear total number of miscounts.
        st      r4,r_ecinvfunc          # Clear the Invalid Function count
        st      r4,r_ecreserved         # Clear the Missing Drive count
        st      r4,r_ecother            # Clear all other Errors count
#
# --- Clear out data within any defined RDD's
#
        ldconst MAXRAIDS-1,r6           # Check all RAIDs
#
.pci10:
        ld      R_rddindx[r6*4],r7      # r7 = RDD
        cmpobe  0,r7,.pci20             # Jif undefined
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stl     r4,rd_miscomp(r7)       # Clear miscomp and missing drive
#
.pci20:
        subo    1,r6,r6                 # Advance to next array
        cmpible 0,r6,.pci10             # Jif not complete
#
# --- Clear out data within all PDIs
#
        ldconst MAXDRIVES-1,r6          # Check all drives
#
.pci30:
        ld      P_pddindx[r6*4],r7      # Get PDD
        cmpobe  0,r7,.pci40             # Jif undefined
        st      r4,pd_r10misc(r7)       # Clear raid 10 miscompare count
        stl     r4,pd_miscomp(r7)       # Clear miscomp and devmiscomp
#
.pci40:
        subo    1,r6,r6                 # Adjust index
        cmpible 0,r6,.pci30             # Jif more
        ret
#
#**********************************************************************
#
#  NAME: r$checker
#
#  PURPOSE:
#
#       To provide an automated means of checking redundant data
#       associated with each RAID 5 array in the background.
#
#  DESCRIPTION:
#
#       This process performs an initial delay to permit the running of
#       benchmarks at full performance immediately after a system boot.
#       It scans the RAIDs based on the user input into the R_pcctrl variable.
#
#       This process runs continually and never exits.  When a complete
#       checker cycle has completed the next cycle starts.
#
#  CALLING SEQUENCE:
#
#       process call
#
#  INPUT:
#
#       Variables to setup before calling:
#       - R_pcctrl input control bits should be set including rdpcnewcmd
#         precedence of the bits:
#         - enable
#         - specific RID
#         - marked RDDs in rd_astatus
#         - all RIDs are checked if specific not set
#
#         clearlog, correction, and 1pass are applied when a new command
#           is received
#
#       - R_pcstartraid is RID to be tested when rdpcspecific set in R_pcctrl
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#**********************************************************************
#
r$checker:
#
# --- Initial setup
#
        ldconst rdpcdefctl,r5
        st      r5,R_pcctrl             # Initialize the checker control
#
# --- Don't start scanning until the system is initialized
#
.pcd00:
        ldconst 10*1000,g0              # Wait a while
        call    K$twait
        ldob    O_p2init,r3             # Get phase II inits complete
        cmpobne TRUE,r3,.pcd00          # Jif not completed
#
c       g1 = get_ilt();                 # Allocate an ILT.
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif  # M4_DEBUG_ILT
c       g2 = get_rrp();                 # Allocate an RRP.
.ifdef M4_DEBUG_RRP
c CT_history_printf("%s%s:%u get_rrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif  # M4_DEBUG_RRP
c       ((ILT *)g1)->ilt_normal.w0 = g2; # Link to RRP in ILT.
#
# --- These (ILT/RRP) will never get released.
#
.if 1 #VIJAY_MC
        ldconst 0,g0
        st      g0,rr_flags(g2)
.endif  # 1

#
# --- Top of parity scan loop -----------------------------------------
#     Delay 10 seconds then check parity control for next action to be performed
#
.pcd10:
        ldconst 0,g0                    # Clear the current raid num
        st      g0,R_pccurraid
#
        ldconst 10*1000,g0              # Delay 10 seconds
        call    K$twait
#
# --- Get latest control input and process it
#
        ld      R_pcctrl,r3             # Get parity control input from user
        bbc     rdpcenable,r3,.pcd10    # Jif scan disabled - keep waiting
        bbs     rdpcnewcmd,r3,.pcd20    # Jif control word has been updated
                                        # Else use the previous command in r8
        bbs     rdpc1pass,r8,.pcd10     # Jif single passes - keep waiting
        b       .pcd100                 # Else loop forever - check options
#
# --- Handle a new incoming command
#
# Save latest command
#
.pcd20:
        st      r3,r_prevctrl           # Save the previous control word
        mov     r3,r8                   # Save for later reference
        clrbit  rdpcnewcmd,r3,r3        # Clear the new command bit and save it
        st      r3,R_pcctrl
#
# Clear logs
#
        bbc     rdpcclearlog,r8,.pcd30  # Jif if clear log bit is not set
        call    r$initchecker           # Clear the logs
#
# Apply correction
#
.pcd30:
        chkbit  rdpccorrect,r8          # Determine if correction wanted
        sele    FALSE,TRUE,r4           #  and set appropriately
        st      r4,r_correct            # Save the correction state
#
# --- Handle specific RID ---------------------------------------------
#
.pcd100:
        bbc     rdpcspecific,r8,.pcd200 # Jif no specific RID being requested
        ld      R_pcstartraid,g11       # Else load the RID
        ldconst FALSE,g12               # No tests performed in this pass
        call    r$checkrid              # Check it
#
        b       .pcd800                 # Pass is complete
#
# --- Handle all marked RDDs ------------------------------------------
#
.pcd200:
        bbc     rdpcmarked,r8,.pcd300   # Jif no marked RDDs being requested
        ldconst FALSE,g12               # No tests performed in this pass
        mov     0,g11                   # Start with RID 0
        ldconst MAXRAIDS,r12
.pcd210:
        ld      R_rddindx[g11*4],g13    # g13 = RDD
        cmpobe  0,g13,.pcd220           # Jif undefined and get next RID
#
        ldob    rd_astatus(g13),r3      # Get raid additional status
        bbc     rdaparity,r3,.pcd220    # Jif parity check not required
#
        call    r$checkrid              # Check it
        ld      R_pcctrl,r3             # Get parity control input from user
        bbs     rdpcnewcmd,r3,.pcd700   # Jif a new command has been received
#
.pcd220:
        addo    1,g11,g11               # Advance to next RDD
        cmpobne r12,g11,.pcd210         # Jif not complete scan
#
        b       .pcd800                 # Pass is complete
#
# --- Handle all RDDs -------------------------------------------------
#
.pcd300:
        ldconst FALSE,g12               # No tests performed in this pass
        mov     0,g11                   # Start with RID 0
        ldconst MAXRAIDS,r12
.pcd310:
        call    r$checkrid              # Check it
        ld      R_pcctrl,r3             # Get parity control input from user
        bbs     rdpcnewcmd,r3,.pcd700   # Jif a new command has been received
#
        addo    1,g11,g11               # Advance to next RDD
        cmpobne r12,g11,.pcd310         # Jif not complete scan
#
        b       .pcd800                 # Pass is complete
#
# --- Multiple RAIDs being Checked with a new command coming in.
#       Determine if a Parity Check Complete Log message should be sent
#       up or not.  Criteria for logging a message:
#           1)  The new command is disabling Parity Checking, or
#           2)  The new command is different than the old, or
#           3)  The new command is the same as the old, the old was marked, and
#               there are no more marked RAIDs owned by this controller
#           4)  The new command is the same as the old, the old was all, and
#               there are no more RAIDs (1, 5, and 10) owned by this controller
.pcd700:
        ld      r_prevctrl,r4           # Get the Previous Command
        bbc     rdpcenable,r3,.pcd900   # Jif Parity Checking has been disabled
        cmpobne r3,r4,.pcd900           # Jif new and old cmds are different
        bbc     rdpcmarked,r4,.pcd750   # Jif old cmd handling all RDDs
#
#   Handle Marked RAIDs - anymore owned by this controller?  If so, do not
#       log a Parity Check complete message and cycle around.
#
        ldconst 0,r3                    # RID being tested
        ldconst MAXRAIDS-1,r4           # End of RAIDs possible for loop compl
.pcd710:
        ld      R_rddindx[r3*4],r5      # r5 = RDD
        cmpobe  0,r5,.pcd740            # Return if undefined
#
        ldob    rd_astatus(r5),r6       # Get raid additional status
        bbc     rdaparity,r6,.pcd740    # Jif parity check not required
#
        ldob    rd_status(r5),r6        # r6 = raid status
        cmpobne rdop,r6,.pcd740         # Jif not operational
#
        ldos    rd_vid(r5),g0           # Get the VID
#
        PushRegs(r6)                    # Save regs
        call    DL_AmIOwner             # Determine if we are the owner or not
        PopRegs(r6)                     # Restore regs
#
        cmpobe  TRUE,g0,.pcd10          # Jif I am the owner - do not log msg
#
.pcd740:
        cmpinco r4,r3,r3                # Check for done and point to next RAID
        bne     .pcd710                 # Jif more RIDs to check
        b       .pcd900                 # None found, so log a completion msg
#
#   Handle all RAIDs being checked - anymore owned by this controller?  If so,
#       do not log a Parity Check complete message and cycle around.
#
.pcd750:
        ldconst 0,r3                    # RID being tested
        ldconst MAXRAIDS-1,r4           # End of RAIDs possible for loop compl
.pcd760:
        ld      R_rddindx[r3*4],r5      # r5 = RDD
        cmpobe  0,r5,.pcd790            # Return if undefined
#
        ldob    rd_status(r5),r6        # r6 = raid status
        cmpobne rdop,r6,.pcd790         # Jif not operational
#
        ldob    rd_type(r5),r6          # r6 = raid type code
        cmpobe  rdraid5,r6,.pcd770      # Jif RAID 5
        cmpobe  rdraid1,r6,.pcd770      # Jif RAID 1
        cmpobne rdraid10,r6,.pcd790     # Jif not RAID 10 to check next RAID
#
.pcd770:
        ldos    rd_vid(r5),g0           # Get the VID
#
        PushRegs(r6)                    # Save regs
        call    DL_AmIOwner             # Determine if we are the owner or not
        PopRegs(r6)                     # Restore regs
#
        cmpobe  TRUE,g0,.pcd10          # Jif I am the owner - do not log msg
#
.pcd790:
        cmpinco r4,r3,r3                # Check for done and point to next RAID
        bne     .pcd760                 # Jif more RIDs to check
        b       .pcd900                 # None found, so log a completion msg
#
# --- Pass completed --------------------------------------------------
#
.pcd800:
        ld      R_pcpass,r3             # Increment the tested pass count
        addo    1,r3,r3
        st      r3,R_pcpass
#
# --- Log that a pass has completed.
#
.pcd900:
        cmpobe  FALSE,g12,.pcd10        # Jif no tests took place - loop again
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleparitychkdone,r3     # Get log code
        st      r3,mle_event(g0)        # Store as word to clear other bytes
        ld      R_pcmisc,r3             # Miscompare count
        st      r3,epc_misc(g0)
        ld      R_pcpass,r3             # Scan pass
        st      r3,epc_pass(g0)
        ld      R_pcstartraid,r3        # Starting RID
        st      r3,epc_startraid(g0)
        ld      r_prevctrl,r3           # Control value
        st      r3,epc_prevctrl(g0)
        ld      r_ecinvfunc,r3          # Number of invalid function errors
        st      r3,epc_invfunc(g0)
        ld      r_ecreserved,r3         # Number of reserved errors
        st      r3,epc_reserved(g0)
        ld      r_ecother,r3            # Number of other errors
        st      r3,epc_other(g0)
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], epclen);
        b       .pcd10                  # Back to start
#
#**********************************************************************
#
#  NAME: r$checkrid
#
#  PURPOSE:
#
#       To parity scan the requested RAID ID.
#
#  DESCRIPTION:
#
#       This process performs an initial sanity check of the requested RID
#       to ensure that it is valid, belongs to this controller, is
#       operational, and is a redundant RAID device.
#
#       It performs the IO operations to the RAID and updates the error
#       counters for the scanner and for RDD.
#
#       This function is closely tied to r$checker.
#
#  CALLING SEQUENCE:
#
#       call    r$checkrid
#
#  INPUT:
#
#       g1      ILT address
#       g2      RRP address
#       g11     RID
#       g12     RAID ops performed (T/F)
#
#  OUTPUT:
#
#       g6      Error code - ecok if no test performed or test good
#       g12     RAID ops performed - either TRUE or not changed
#               call must initialize g12 to FALSE if need to know a test ran
#       Updates RDD error counts and astatus bits
#
#  REGS DESTROYED:
#
#       g0,g3-g7,g13
#
#       TODO - SMW this needs fixed for > 2TB support
#**********************************************************************
r$checkrid:
#
# --- Setup variables
#
        st      g11,R_pccurraid         # Save the current Raid ID
        mov     0,r12                   # r12-r13 = current SDA to check
        mov     0,r13                   # r12-r13 = current SDA to check
#        mov     0,g7                    # g7 = previous queue depth
        ldconst ecok,g6                 # Assume test will be good
        ldconst e_pcr_term,r14          # r14 = Completion type = terminated
        ldconst FALSE,r11               # r11 = Scan Started Flag -> Not
#
# --- Top of loop to check each segment of the RAID
#
#
.rcr100:
        ld      R_pcctrl,r15            # Get latest parity check control word
        bbc     rdpcenable,r15,.rcr1000 # Jif disabled - return
#
# --- Sanity checks
#
        call    r$set_chklock           # Set checker lock and wait if needed
#
        ld      R_rddindx[g11*4],g13    # g13 = RDD
        cmpobe  0,g13,.rcr1000          # Return if undefined
#
        ldos    rd_vid(g13),g0          # Get the VID
#
        PushRegs(r3)                    # Save regs
        call    DL_AmIOwner             # Determine if we are the owner or not
        PopRegs(r3)                     # Restore regs
#
        cmpobe  FALSE,g0,.rcr1000       # Return if not owner
#
        ldob    rd_status(g13),r3       # r3 = raid status
        cmpobne rdop,r3,.rcr1000        # Return if not operational
#
        ldob    rd_type(g13),r3         # r3 = raid type code
        cmpobe  rdraid5,r3,.rcr105      # Jif RAID 5
        cmpobe  rdraid1,r3,.rcr105      # Jif RAID 1
        cmpobne rdraid10,r3,.rcr1000    # Return if not a redundant RAID
#
.rcr105:
        ld      rd_sps(g13),r8          # Assume sectors per stripe
        cmpobe  rdraid10,r3,.rcr110     # If RAID 10, use sps, else big number
#
        ldconst 512,r8                  # Max # blocks to chk in each pass
                                        # This should be a multiple of stripe
                                        # sizes we allow.
#
# --- Setup for the next RAID request
#
.rcr110:
c       if (((RDD *)g13)->devCap <= *(UINT64 *)&r12) {
          b       .rcr200               # Jif no more blocks in RAID to check
c       }
c       if ( (((RDD *)g13)->devCap - *(UINT64 *)&r12) < r8) { # If less than max stripe size
c         r8 = ((RDD *)g13)->devCap - *(UINT64 *)&r12;  # Set less than stripe size to do
c       }
#
# Log a message the first time started
#
        cmpobe  TRUE,r11,.rcr120        # Jif already logged a start message
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleparitycheckraid,r3   # Get log code
        st      r3,mle_event(g0)        # Store as word to clear other bytes
        stos    g11,e_pcr_rid(g0)       # Save the RID
        ldconst e_pcr_start,r3          # Set the Type to Start
        stos    r3,e_pcr_type(g0)
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], e_pcrlen);
        ldconst TRUE,r11                # r13 = Scan Started Flag -> Yes
#
# --- Generate and issue parity check request
#
.rcr120:
        ldconst TRUE,g12                # We are doing a test
#
        mov     0,r5
        st      r5,rr_sglptr(g2)        # Clear SGL link
        st      r5,rr_sglsize(g2)
        stob    r5,rr_status(g2)        # Clear status
        ldconst rrconsistentchk,r5      # Assume R10 check
        ldob    rd_type(g13),r3         # Check type
        cmpobe  rdraid1,r3,.rcr125      # Jif RAID 1
        cmpobe  rdraid10,r3,.rcr125     # Jif RAID 10
        ldconst rrparitychk,r5          # Parity check function
        chkbit  rdpccorrect,r15         # Determine if Parity Correction req'd
        alterbit rrpchkcorrect,0,r3     # Set Parity Correction appropriately
        stob    r3,rr_options(g2)
#
.rcr125:
        stos    r5,rr_func(g2)
        ldconst rrhigh,r5               # High priority
        stob    r5,rr_strategy(g2)
        stos    g11,rr_rid(g2)          # This RAID id
        st      r8,rr_rlen(g2)          # Length in sectors (one stripe)
        stl     r12,rr_rsda(g2)         # Set starting disk address
        lda     R$que,g0
        call    K$qw                    # Queue the request with wait
        call    r$clr_chklock           # Clear checker lock
#
# --- Check status of parity check request
#
        ldob    rr_status(g2),r5        # r5 = command status
        mov     r5,g6                   # Save status for marked RDD scan
        cmpobe  ecok,r5,.rcr190         # Jif no error - do next segment
#
# --- Error occurred checking parity
#     Update error counts in RDD
#
        ldconst eccompare,r3            # r3 = parity check miscompare error
        cmpobe  r3,r5,.rcr150           # Jif miscompare error
#
        ldconst ecinvfunc,r3            # r3 = invalid function error code
        cmpobne r3,r5,.rcr130           # Jif not invalid function error
#
# Invalid function error occurred
#
        ld      r_ecinvfunc,r3
        addo    1,r3,r3
        st      r3,r_ecinvfunc
        b       .rcr1000                # Return
#
.rcr130:
        ldconst ecreserved,r3           # r3 = bad PSD error code
        cmpobne r3,r5,.rcr140           # Jif not bad PSD error
#
# Bad PSD error occurred
#
        ld      rd_pardrv(g13),r3       # Get the missing drive errors for RDD
        addo    1,r3,r3
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        st      r3,rd_pardrv(g13)       # Increment and store the count.
        ld      r_ecreserved,r3
        addo    1,r3,r3
        st      r3,r_ecreserved
        b       .rcr190                 # Go to next segment
#
# Some other error occurred
#
.rcr140:
        ld      r_ecother,r3
        addo    1,r3,r3
        st      r3,r_ecother
        b       .rcr1000                # Return
#
# --- Miscompare error occurred
#
.rcr150:
        ld      rd_miscomp(g13),r4      # Increment the num of miscomps for this
        addo    1,r4,r4                 # raid device.
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        st      r4,rd_miscomp(g13)
#
        ld      R_pcmisc,r4             # Increment miscompare counter
        addo    1,r4,r4
        st      r4,R_pcmisc
        ldconst ecok,g6                 # Miscomps ok for marked RDDs
#
.rcr190:
c       *(UINT64*)&r12 += r8;           # Increment to next SDA to check
        stl     r12,r_scan_block        # r12/r13 Store current SDA
        b       .rcr100                 # Do next pass on this RAID
#
# --- Parity scan completed so update the RDD additional status and NVRAM
#
.rcr200:
        ldconst e_pcr_end,r14           # Show the Scan completed
        cmpobne ecok,g6,.rcr1000        # Return if the op failed
        ldob    rd_astatus(g13),r3      # Get additional status
        bbc     rdaparity,r3,.rcr1000   # Don't update NVRAM if bit is clear
        clrbit  rdaparity,r3,r3         # Clear the parity check bit
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r3,rd_astatus(g13)
        call    D$p2update              # Update NVRAM
#
# Log completion
#
.rcr1000:
        cmpobe  FALSE,r11,.rcr1010      # Jif Scan never started
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleparitycheckraid,r3   # Get log code
        st      r3,mle_event(g0)        # Store as word to clear other bytes
        stos    g11,e_pcr_rid(g0)       # Save the RID
        stos    r14,e_pcr_type(g0)      # Save the termination status
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], e_pcrlen);
#
.rcr1010:
        call    r$clr_chklock           # Clear checker lock
        ret
#
#**********************************************************************
#
#  NAME: R$chkstop
#
#  PURPOSE:
#
#       To provide a common means of stopping all I/O activity invoked
#       through the checker process
#
#  DESCRIPTION:
#
#       The stop counter is incremented and a check is made for any
#       outstanding I/O.  When all outstanding I/O completes, this
#       routine returns to the caller.  While the stop counter is
#       non-zero, the executive is effectively blocked.
#
#       This routine may only be called from the process level.
#
#  CALLING SEQUENCE:
#
#       call    R$chkstop
#
#  INPUT:
#
#       None.
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#**********************************************************************
#
R$chkstop:
#
        mov     g0,r15                  # Save g0
#
# --- Bump stop counter
#
        ldob    r_chkstopcnt,r3         # Bump stop counter
        addo    1,r3,r3
        stob    r3,r_chkstopcnt
#
# --- Stall until pending I/Os complete
#
.pcs10_s:
        ldob    r_chklock,r4            # Get checker lock
        cmpobe  FALSE,r4,.pcs100        # Jif unlocked
#
        ldconst 1,g0                    # Delay for minimum time
        call    K$twait
        b       .pcs10_s
#
# --- Exit
#
.pcs100:
        mov     r15,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: R$chkresume
#
#  PURPOSE:
#
#       To provide a common means of resuming all I/O activity invoked
#       through the checker process.
#
#  DESCRIPTION:
#
#       The stop counter is decremented and an immediate return is
#       made to the caller.  When this counter has returned to zero,
#       the checker process is unblocked.
#
#  CALLING SEQUENCE:
#
#       call    R$chkresume
#
#  INPUT:
#
#       None.
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#**********************************************************************
#
R$chkresume:
#
# --- Adjust stop counter
#
        ldob    r_chkstopcnt,r3         # Decrement stop counter
        subo    1,r3,r3
        stob    r3,r_chkstopcnt
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: r$set_chklock
#
#  PURPOSE:
#
#       To provide a common means of setting the checker process lock and
#       synchronizing with the checker process.
#
#  DESCRIPTION:
#
#       The stop counter is examined to determine if synchronization
#       has been requested.  If so, an 8 second delay occurs with
#       the stop counter being reexamined.  When the stop counter
#       is zero, the checker lock is set and this routine exits.
#
#  CALLING SEQUENCE:
#
#       call    r$set_chklock
#
#  INPUT:
#
#       None.
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#**********************************************************************
#
r$set_chklock:
        mov     g0,r15                  # Save g0
#
.scl10:
        ldob    r_chkstopcnt,r3         # Get stop counter
        cmpobe  0,r3,.scl20             # Jif clear
#
        ldconst 8000,g0                 # Wait for 8 seconds
        call    K$twait
        b       .scl10
#
.scl20:
        mov     TRUE,r4                 # Set checker lock
        stob    r4,r_chklock
#
# --- Exit
#
        mov     r15,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: r$clr_chklock
#
#  PURPOSE:
#
#       To provide a common means of clearing the checker process lock.
#
#  DESCRIPTION:
#
#       The checker lock is unconditionally cleared and this routine
#       exits.
#
#  CALLING SEQUENCE:
#
#       call    r$clr_chklock
#
#  INPUT:
#
#       None.
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#****************************************************************************
#
r$clr_chklock:
        mov     FALSE,r3                # Clear checker lock
        stob    r3,r_chklock
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
