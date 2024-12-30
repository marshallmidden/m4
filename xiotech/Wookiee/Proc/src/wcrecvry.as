# $Id: wcrecvry.as 147915 2010-09-21 22:16:43Z m4 $
#**********************************************************************
#
#  NAME: wcrecvry.as
#
#  PURPOSE: This module provides the functions to support the
#           flushing of cache write data on power-on. Data is
#           retained in the write cache if the processor is
#           power cycled or reset instead of the normal shutdown.
#           To prevent the loss of customer data, the write cache
#           data is flushed to disk.
#
#  FUNCTIONS:
#           WC$markWCacheEn
#           WC$markWCacheDis
#           WC$recoveryInit
#           WC$recoveryFlushTask
#
#  Copyright (c) 2000-2010 Xiotech Corporation. All rights reserved.
#
#**********************************************************************
#
# --- Global Functions
#
        .globl  WC$batHealth
        .globl  WC$resumeCacheInit
        .globl  WC$continueWithoutMirrorPartner
#
# --- Local equates -----------------------------------------------------------
#
.set    USE_NEITHER,        0
.set    USE_LOCAL,          1
.set    USE_REMOTE,         2
.set    USE_BOTH,           3

#.set    DIRTY,              0
#.set    INVALID_VID,        1
#.set    LBA_OUT_OF_RANGE,   2
#.set    INVALID_TAG,        3
#.set    TAG_MISMATCH,       4
.set    DATA_MISMATCH,      5
.set    ECC_TAG_ERR,        6
#.set    ECC_DATA_ERR,       7
#.set    NOT_DIRTY,          8

.set    ERROR_LIMT,         5
#
# --- Local data --------------------------------------------------------------
#
        .data
#
c_recoveryMode:
        .word   0                       # recovery mode
c_bbdFail:
        .word   0                       # Battery backup data failure
c_rcDirtyCount:
        .word   0                       # Dirty tag to recover count
c_invalidTagCount:
        .word   0                       # invalid tag count
c_flushErrorCount:
        .word   0                       # Flush error count
c_eccErrorCount:
        .word   0                       # ECC error count
c_bufRoot:
        .word   0                       # Buffer tree root
c_vidFailed:
        .word   0                       # VID Failed Count
#
# --- Shared memory data
#
        .section    .shmem
        .align  2
c_logevent:
        .space  mlemaxsiz               # Space for max sized log event
#
# --- More regular data
#
        .data
c_resumeInit:
        .short  0xFFFF

        .globl  c_recoveryMode
        .globl  c_bbdFail
        .globl  c_rcDirtyCount
        .globl  c_flushErrorCount
        .globl  c_vidFailed
        .globl  c_logevent
        .globl  c_resumeInit
#
# --- executable code -------------------------------------------------
#
        .text
#
#**********************************************************************
#
#  NAME: WC$markWCacheEn / WC$markWCacheDis
#
#  PURPOSE:  Indicates whether the write cache may contain valid data
#            or it the write cache is empty. When the write cache is
#            stopped or disabled and all data data has been flushed,
#            markWCacheDis is called to indicate the cache is empty.
#            When is write cache is enabled, markWCacheEn is called
#            to indicate the cache may contain write data.
#
#            This indication is used on power up to determine if the
#            write cache contains valid data that needs to be written
#            to disk prior to initializing the write cache.
#
#  DESCRIPTION:
#            A "magic" number is written to two address to indicating
#            write data is cached or flushed. The system serial number,
#            controller serial number, Dram usage (local or remote),
#            and sequence number are written between the two writes
#            of the "magic" number.
#
#  CALLING SEQUENCE:
#           call    WC$markWCacheEn
#           call    WC$markWCacheDis
#
#  INPUT:
#       g0 - Allow Marking before Write Cache is Initialized (TRUE)
#          - Do not allow Marking before Write Cache is Initialized (FALSE)
#
#  OUTPUT:
#       g0 - completion status.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
WC$markWCacheEn:
        ldconst DATA_CACHED,r15         # Show as Enabling Cache
        b       .wcmwc20
#
WC$markWCacheDis:
        ldconst DATA_FLUSHED,r15        # Show as Disabling Cache
#
.wcmwc20:
        mov     g1,r14                  # Save g1
#
# ---Check if cache initialization is complete or if it is being ignored.
#    Do not alter the content of the firmware control block until initialized.
#
        ldos    K_ii+ii_status,r3       # r3 = Initialization status
        cmpobe  TRUE,g0,.wcmwc30        # Jif Marking no matter what
        bbs     iicinit,r3,.wcmwc30     # Jif Cache is initialized
        ldconst ecok,g0                 # Cache not initialized - return OK
        b       .wcmwc100
#
# --- Change the Table to show cache may or may not contain dirty data
#
.wcmwc30:
        ld      K_xpcb,r4               # r4 = Current PCB
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        st      r15,il_w0(g1)           # Save to show Enabled or Disabled
        st      r4,il_misc(g1)          # Set this Task as waiting
        lda     gWCMarkCacheQueue,g0    # g0 = Queue to put ILT on
                                        # g1 = ILT
        call    wc$q                    # Queue the Request to the WC Mark exec
        ldconst pciowait,r5             # Put this task to sleep until done
        stob    r5,pc_stat(r4)
        call    K$xchang                # Give up control
#
        ld      il_w1(g1),g0            # g0 = completion status
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
#
.wcmwc100:
        mov     r14,g1                  # Restore g1
        ret                             # Exit
#
#**********************************************************************
#
#  NAME: WC$recoveryFlushTask
#
#  PURPOSE: Examines the write cache and initialize the flushing of the data
#           for each dirty tag.
#
#  DESCRIPTION: The DRAM is scanned for dirty tags. For each dirty tag,
#           the flushing of the data is initiated with a call to
#           the recovery flush request function. After the entire
#           cache tag area has been examined, the function waits until
#           all the dirty tags are flushed by monitoring the dirty count.
#           When the count is zero which indicates all tags have been flushed,
#           the write cache is initialized, resumed, and started.
#
#  CALLING SEQUENCE:
#       call WC$recoveryFlushTask
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All G registers!
#
#**********************************************************************
#
#     r4 = limit of tag area
#     r5 = size of individual tag
#     r10 =  cache recovery flag
#     r14 = tag
#     r15 = ilt
#
#**********************************************************************
#
WC$recoveryFlushTask:
#
# --- Wait for the define to send over the system information.
#
.wcrft05:
        ldos    K_ii+ii_status,r4       # Get initialization status
        bbc     iisn,r4,.wcrft10        # Jif Serial number not defined
        bbc     iivdmt,r4,.wcrft10      # Jif VDMTs are not defined
        bbs     iivdd,r4,.wcrft20       # Jif virtual devices defined
.wcrft10:
        ldconst 1,g0                    # Wait for 1 time amount (125 msec)
        call    K$twait
        b       .wcrft05
#
# --- Ensure a Mirror Partner is set up by now. If not, log a message and set
#       to local mirroring.
#
.wcrft20:
        ld      K_ficb,r4               # r4 = FICB address
.if     MAG2MAG
        ld      fi_mirrorpartner(r4),r5 # r5 = Mirror Partner
        ld      fi_cserial(r4),r6       # r6 = This controllers serial number
        cmpobne 0,r5,.wcrft30           # Jif a Mirror Partner has been set
#
#   Log the No Mirror Partner error
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mlenomirrorpartner,r3   # No Mirror Partner Set Up Properly
        st      r3,mle_event(g0)        # Store as word to clear other bytes
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], mle_bstream);
#
        st      r6,fi_mirrorpartner(r4) # Save this controller as the mirror
        mov     r6,r5                   #   partner and continue
#
#   Send the new mirror parter information to the BE to be put in NVRAM
#
        call    c$sendMirrorPartnerChange
#
.wcrft30:
        lda     C_ca,r3                 # r3 = Cache Status block
        cmpobne r5,r6,.wcrft40          # Jif mirror is not to self
#
#   The mirror partner is ourself. Turn off the Mirror Broken Bit if it was
#   set before the FICB was set up and did not know who the mirror partner
#   was at that time.
#
        ldob    ca_status(r3),r7        # r7 = Status
        clrbit  ca_mirrorbroken,r7,r7   # Turn off the Mirror Broken Bit if set
        stob    r7,ca_status(r3)        # Save the new status
        b       .wcrft50                # Continue initialization
#
#   Mirroring Data to another controller. Determine if controller is
#   available to continue. If the controller is not available, wait
#   forever until it does
.wcrft40:
        ld      fi_mirrorpartner(r4),g0 # g0 = Mirror Partner
        mov     g0,r5                   # Save the new Mirror Partner if Changed
        cmpobe  r6,g0,.wcrft50          # MP is now ourself, continue init
        call    DLM$find_controller     # Check the Mirror Partners availability
                                        # g0 = 0 if not found
        cmpobe  0,g0,.wcrft45           # Jif the MP has not been seen
#
#   Find a good path to the Mirror Partner before continuing (can take several
#   seconds before the paths are fully established between the two controllers).
#
        ld      mlmt_dtmthd(g0),r7      # r7 = DTMT (path established to cntrl)
.wcrft43:
        cmpobe  0,r7,.wcrft45           # Jif paths have not been established
        ldob    dtmt_state(r7),r8       # r8 = State of the path
        cmpobne dtmt_st_init,r8,.wcrft50 # Jif the path is not initializing -
                                        #   either working or broken
        ld      dml_mllist(r7),r7       # r7 = next DTMT (path)
        b       .wcrft43                # Check next path that is operational
#
.wcrft45:
        ldob    ca_status(r3),r7        # r7 = Status
        bbs     ca_mirrorbroken,r7,.wcrft50 # Jif Mirror Partner is broken
        ldconst 125,g0                  # Wait for 125 msec
        call    K$twait
        b       .wcrft40                # Check to see if the partner is now
                                        #   available to continue
#
#   Continue setting up Cache
#
.wcrft50:
        ldos    K_ii+ii_status,r4       # r4 = Initialization status
        ldob    ca_status(r3),r7        # r7 = Cache Status
        setbit  iimpfound,r4,r4         # Set Mirror Partner Found status
        stos    r4,K_ii+ii_status       # Save the new status
        cmpo    r5,r6                   # Determine if mirroring to self
        alterbit ca_nwaymirror,r7,r7    # Set bit inverted
        notbit  ca_nwaymirror,r7,r7     # Correct bit meaning: If Mirroring to
                                        #  self, clear bit, else set bit
        stob    r7,ca_status(r3)        # Save the new value
        bbc     ca_ena_pend,r7,.wcrft60 # Jif Cache Enable is not pending
        bbs     ca_mirrorbroken,r7,.wcrft60 # Jif Mirror Partner is broken and
                                        #       do not allow cache to enable
        clrbit  ca_ena_pend,r7,r7       # Clear the Enable Pending
        stob    r7,ca_status(r3)        # Save the new status
.else   # MAG2MAG
#
# --- Set mirror partner to ourself, use the BE DRAM for the mirror copy. Set
#     the II Status to show that Cache Init has begun but not completed yet.
#
        ldos    K_ii+ii_status,r5       # r5 = Initialization status
        ld      fi_cserial(r4),r6       # r6 = This controllers serial number
        ldob    ca_status(r3),r7        # r7 = Cache Status
        setbit  iimpfound,r5,r5         # Set Mirror Partner Found status
        stos    r5,K_ii+ii_status       # Save the new II status
        st      r6,fi_mirrorpartner(r4) # Store Mirror Partner
        clrbit  ca_nwaymirror,r7,r7     # Clear the N-Way mirroring bit
        stob    r7,ca_status(r3)        # Save the new value
        bbc     ca_ena_pend,r7,.wcrft60 # Jif Cache Enable is not pending
        clrbit  ca_ena_pend,r7,r7       # Clear the Enable Pending
        stob    r7,ca_status(r3)        # Save the new status
.endif  # MAG2MAG
        ldconst 0xFFFFFFFF,g0           # g0 = Global Enable
        call    C$enable                # Enable the cache if possible
#
# --- Ensure the FE and BE memory sizes are the same before continuing
#
.wcrft60:
# --- Call the function to set up the Data in DRAM from the NV Memory
#       (may task switch to get job done)
#
        PushRegs(r8)                    # Save registers around C routine
        call    WC_RestoreData
        PopRegsVoid(r8)                 # Restore the registers
#
# --- Complete Initialization of the cache
#
        call    C$boot
#
# --- Setup LRU --------------------------------------------------------------
#
        ldconst 0,r12                   # Zero
        lda     c_hlruq,r3              # Get addr of head sentinel
        lda     c_tlruq,r4              # Get addr of tail sentinel
        st      r3,tg_bthd(r4)          # Save backward pointer of tail
        st      r12,tg_fthd(r4)         # Clear forward pointer of tail
        st      r4,tg_fthd(r3)          # Set forward pointer of head
        st      r12,tg_bthd(r3)         # Clear backward pointer of head
#
# --- Perform Write Cache recovery
#
        PushRegs(r8)                    # Save registers around C routine
        call    wc_recoveryInit
        PopRegsVoid(r8)                 # Restore the registers
#
# --- Now that any write data has been flushed to disk,
#       initialize the write cache.
#
        call    wc$initCache
#
# --- Start Write Cache executive processes
#
        call    WC$start
#
# --- Check to see if the BE needs to be flushed or not (Failover at Power Up
#     needs the BE Flushed but is done before Cache is initialized)
#
        ld      gWCPowerUpFlushBEFlag,r3 # Get the Flush BE at Power Up flag
        cmpobne TRUE,r3,.wcrft65        # Jif Not needed at this power up
        lda     c_wflushq,r3            # Get the Flush Task Queue to see if
                                        #  the Task has been started yet
.wcrft63:
        call    K$xchang                # Give up control to let Flush Task
                                        #  get going
        ld      qu_pcb(r3),g0           # Get the PCB
        cmpobe  0,g0,.wcrft63           # Jif the Flush Task is not operational
#
        mov     sp,g0                   # Get a pointer to MRP parameters
        addo    16,sp,sp                # Make room for the MRP stuff
        ldconst mfboglobalinval,r3      # Set up to do a Global Invalidate
        ldconst 0,g1                    # Clear all the other fields
        stob    r3,mfb_op(g0)
        stos    g1,mfb_nvids(g0)
        st      g1,mfb_vidlist(g0)
        call    WC$FlushBE              # Kick off the Flush of the BE process
        subo    16,sp,sp                # Reset the Stack Pointer
        cmpobe  deok,g0,.wcrft65        # Jif Flush is OK
c fprintf(stderr, "%s%s:%u WC$recoveryFlushTask: Bad Return Code from WC$FlushBE = 0x%x\n", FEBEMESSAGE, __FILE__, __LINE__, (UINT32)g0);
c       abort();
#
# --- Turn caching layer on
#
.wcrft65:
        ldconst mriclearone,g0          # g0 = Options (clear only one stop)
        ldconst mriwcrecvry,g1          # g1 = WC Recovery User ID
        call    C$resume                # Turn caching layer on
#
# --- Increment the sequence number
#
        ld      K_ficb,r3               # Get FICB
        ld      fi_seq(r3),r8           # Get sequence number
        addo    1,r8,r8                 # increment sequence number
        st      r8,fi_seq(r3)           # Store sequence number
#
# --- Mark the cache as enable or disabled. Also update the sequence
#     number stored in the DRAM.
#
        lda     C_ca,r5                 # Get the Cache Information pointer
        ldob    ca_status(r5),r3        # Get the Cache Status
        bbc     ca_ena,r3,.wcrft70      # Jif the cache is disabled
#
# --- Indicate the cache may contain dirty data
#
        ldconst TRUE,g0                 # Change Signature even if not Init'd
        call    WC$markWCacheEn         # Set value of signature
        b       .wcrft80
#
# --- Indicate the cache is empty, all data has been flushed
#
.wcrft70:
        ldconst TRUE,g0                 # Change Signature even if not Init'd
        call    WC$markWCacheDis        # Mark the cache
#
# --- Send the new sequence number to the back end processor so the
#       value gets updated in the NVRAM.
#
.wcrft80:
        call    wc$setSeqNo             # Set Sequence Number
#
# --- Try and flush any remaining data out of cache (if there is any)
#
        ldconst TRUE,g0                 # Wait for Flush to complete before
        call    wc$FlInvAll             #   Returning from wc$FlInvAll
#
# --- Indicate the cache is initialized.
#
        ldos    K_ii+ii_status,r3       # Get initialization status
        setbit  iicinit,r3,r3           # Set cache initialized bit
        stos    r3,K_ii+ii_status       # Set initialization status
#
# --- Exit
#
        ret                             # Exit

#**********************************************************************
#
#  NAME: wc$recoveryFlushRequest
#
#  PURPOSE: This function sends a request to the lower layer to
#           flush the data for a specific tag.
#
#  DESCRIPTION:
#           If the FE only DRAM is being used, the data is mirrored to
#           the BE DRAM. If both DRAMs are being used, they are
#           compared. The completion routine allocates a ILT and VRP
#           and submits a request to the lower layer.
#
#  CALLING SEQUENCE:
#       call wc$recoveryFlushRequest
#
#  INPUT:
#       g0 = cache tag.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g1, g2.
#
#**********************************************************************
#     r14 = tag
#     r15 = ilt
#**********************************************************************
#
.globl  wc_recoveryFlushRequest
wc_recoveryFlushRequest:
#
# --- Save value of cache tag
#
        mov     g0,r14                  # Save value of cache tag
#
# --- Allocate an ILT and VRP for this I/O.
#     Outputs: g1 = Ptr to ITL
#              g2 = Ptr to VRP
#
        call    M$aivw
        mov     0,r9
        st      r9,vr_use2(g2)         # Zero out vr_use2 so as to VRP not needed to be
                                       # tracked in Back End.
#
# --- Initialize the contents of the ILT.
#
        st      g2,il_vrp(g1)           # ILT_vrp = Ptr to VRP
        st      r14,il_tag(g1)          # ILT_tag = Ptr to cache tag
#
#     Load the cache tag entry into r8-r11.
#
!       ldl     tg_vsda(r14),r8         # r8/r9 = Starting LBA
        ld      tg_vlen(r14),r10        # r10 = Transfer Length (block count)
        ld      tg_bufptr(r14),r11      # r11 = Buffer Pointer
#
# --- Initialize the contents of the VRP.
#
        ldconst vroutput,g0             # VRP function code = Output (Write)
        stos    g0,vr_func(g2)          # Store VRP function code
        ldconst vrhigh,g0               # VRP strategy = High priority
        stos    g0,vr_strategy(g2)      # Store VRP strategy/status
        ldos    tg_vid(r14),g0          # Get VID from cache tag
        stos    g0,vr_vid(g2)           # Store Virtual ID
        stl     r8,vr_vsda(g2)          # Store VRP starting disk addr
        st      r10,vr_vlen(g2)         # Store VRP transfer length (blk cnt)
        lda     vrpsiz(g2),g0           # Get Ptr to SGL header
        st      g0,vr_sglptr(g2)        # Store SGL pointer in VRP
        ldconst sghdrsiz+sgdescsiz,g0   # Calculate size of SGL
        st      g0,vr_sglsize(g2)       # Store SGL size in VRP
#
# --- Initialize the Scatter Gather List Header. The count is one
#     and the size is the size of the header plus one descriptor.
#
        st      g0,vrpsiz+sg_size(g2)   # Store SGL size in SGL Header
        ldconst 1,g0                    # One SGL descriptors
        st      g0,vrpsiz+sg_scnt(g2)   # Store count of one
#
# --- Initialize the contents of the SGL entry for this cache tag.
#     This includes the starting physical buffer address and byte
#     transfer length.
#
                                        # Store phys buf addr for SGL
        st      r11,vrpsiz+sg_desc0+sg_addr(g2)
        shlo    9,r10,g0                # Calc byte xfer length=(512 * BlkCnt)
                                        # Store byte xfer length in SGL entry
        st      g0,vrpsiz+sg_desc0+sg_len(g2)
#
# --- Send this I/O request to the Virtual Layer Queue
#     Inputs: g0 = Ptr to VRP, g1 = Ptr to ILT, g2 = completion routine
#
        mov     g2,g0                   # g0 = VRP
        lda     wc$completeRecoveryFlush,g2
        b       c$calllower             # Queue request

#**********************************************************************
#
#  NAME: wc$completeRecoveryFlush
#
#  PURPOSE: This is the completion routine for the recovery flush request
#
#  DESCRIPTION:
#           The data buffer is released. The cache tag fields are
#           initialized (both local and remote). The cache tag is
#           added to the free list. The ILT and VRP are freed.
#           Finally the dirty count is decremented by one.
#
#  CALLING SEQUENCE:
#       call    wc$completeRecoveryFlush
#
#  INPUT:
#       g0 = completion code
#       g1 = ILT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g2
#
#**********************************************************************
#     r3 = Used to set tag attribute
#     r4 = 0
#     r12 = tag
#     r12 = tag in other DRAM
#     r14 = completion status
#     r15 = ilt
#**********************************************************************
#
wc$completeRecoveryFlush:
        movl    g0,r14                  # Save the input registers
#
# --- This tag has been flushed to disk and is no longer dirty.
#     Load the Cache Tag pointer from the ILT.
#
        ld      il_tag(g1),r12          # r12 = tag
#
# --- Check the completion status
#
        cmpobe  ecok,g0,.wcrfr100       # Jif completion status good
#
# --- If status is not good, don't release, this tag.
# --- Add buftag to RB tree with the buffer ptr as the key.
#
        mov     r12,g0                  # g0 = tag
        call    wc$insert_fail_node
        b       .wcrfr130
#
# --- Update tag fields
#
.wcrfr100:
#
# --- Link tag to freelist
#     (g0 = cache tag to free)
#
        mov     r12,g0                  # g0 = tag
        call    wc$mrel_ctag            # Release it to free pool/return
#
# --- Save the G registers.
#
        PushRegs(r3)
        mov     r12,g0                  # g0 = tag
        call   WC_initTag
        PopRegsVoid(r3)
#
.wcrfr130:
#
# --- Free the memory used for the VRP and the ILT.
#     (g1 = ILT to free, g2 = VRP to free)
#
        mov     r15,g1                  # Get ILT
        ld      il_vrp(g1),g2           # Get VRP from ILT
        call    M$riv
#
# --- Decrement the Dirty Tags count and check to see if it becomes zero.
#
        ld      c_rcDirtyCount,r3       # Load Dirty Tag count
        subo    1,r3,r3                 # Decrement Overlapped Tag count
        st      r3,c_rcDirtyCount       # Store Overlapped Tag count
#
# --- Exit
#
        movl    r14,g0                  # Restore the input registers
        ret                             # Exit

#**********************************************************************
#
#  NAME: wc$msgCacheRecoverFail
#
#  PURPOSE:
#       To provide a means of logging cache recovery failures.
#
#  DESCRIPTION:
#       An error log message code is constructed with the supplied
#       error code and that message is sent to the CCB.
#
#  CALLING SEQUENCE:
#       call    wc$msgCacheRecoverFail
#
#  INPUT:
#       g0 = error status
#       g1 = cache tag
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
        .globl    wc$msgCacheRecoverFail
wc$msgCacheRecoverFail:
#
        movq    g0,r12                  # Save g0-g3
        movl    g4,r10                  # Save g4-g5
#
        lda     c_logevent,g0           # Log error to CCB
        cmpobne ECC_TAG_ERR,r12,.mcr10  # Jif not ECC error
#
# --- Increment the counter that this site had an ECC error.
#
        ld      c_eccErrorCount,r5
        addo    1,r5,r5
        st      r5,c_eccErrorCount      # Increment count
        b       .mcr20
#
# --- Check for a valid VID
#
.mcr10:
        ldos    tg_vid(r13),r8          # Get vid in error
        ldconst MAXVIRTUALS,r9          # Get maximum number of vdisks
        cmpobg  r9,r8,.mcr40            # Jif valid VID
#
# --- Check for more than 5 log messages
#
        ld      c_invalidTagCount,r5    # Increment error count
        addo    1,r5,r5
        st      r5,c_invalidTagCount
.mcr20:
        cmpobl  ERROR_LIMT,r5,.mcr60    # Jif more than five
#
# --- Set log entry event code.
#
        ldconst mleinvalidtag,r8        # Event code
        ldconst tgsize,r9               # Get parameter length
        stos    r12,edf_status(g0)      # Set error code
        ldos    c_bbdFail,r3
        stos    r3,edf_status+2(g0)     # Set ECC block in error
#
# --- Copy tag into packet.
#
        ldconst tgsize,r4               # Get parameter length
.mcr30:
        subo    1,r4,r4                 # decrement
        ldob    (r13)[r4*1],r5          # Get byte
        stob    r5,mle_bstream+4(g0)[r4*1] # Store byte
        cmpobne 0,r4,.mcr30             # jif some bytes left
        b       .mcr50
#
.mcr40:
        ld      c_vidFailed,r3
        ldos    (r3)[r8*2],r5           # Get Failed count
        addo    1,r5,r5                 # Increment failed count
        stos    r5,(r3)[r8*2]           # Save Failed count
#
# --- Check for more than 5 log messages
#
        cmpobl  ERROR_LIMT,r5,.mcr60    # Jif more than five
#
# --- Create log error data.
#
!       ldl     tg_vsda(r13),r4         # Get lba in error
        ld      tg_vlen(r13),r6         # Get length
        stt     r4,erf_lba(g0)          # Set lba and length
#
        stos    r12,erf_status(g0)      # Set error code
#
        ld      c_recoveryMode,r7       # Set recovery mode
        stob    r7,erf_mode(g0)
#
        ldos    tg_vid(r13),r3          # Set vid in error
        stos    r3,erf_vid(g0)
#
        ldconst mlecacherecoverfail,r8  # Event code
        ldconst erflen,g1               # Get parameter length
#
.mcr50:
        st      r8,mle_event(g0)        # Store as word to clear other bytes
#
# --- Send log message for initialization failed
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        lda     c_logevent,r4
c       memcpy(&TmpStackMessage[0],(void*)r4,g1);
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], g1);
#
# --- Exit
#
.mcr60:
        movq    r12,g0                  # Restore g0-g3
        movl    r10,g4                  # Restore g4-g5
        ret
#
#**********************************************************************
#
#  NAME: wc$insert_fail_node
#
#  PURPOSE: To create a tree of failed node sorted by buffer address.
#           This tree is used to initialize the cache buffer, freeing
#           only the unused area of cache
#
#  DESCRIPTION:  Temporary memory is allocated for a tree node. The
#           buffer starting address is used as the key and
#           the buffer ending address is used as the keym. The node
#           is stored into a Red-Black tree. The root of tree is
#           maintained by being stored in a variable.
#
#  CALLING SEQUENCE:
#       call    wc$insert_fail_node
#
#  INPUT:
#       g0 = cache tag
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
wc$insert_fail_node:
        mov     g0,r15                  # save cache tag
#
# --- Increment the flush error count
#
        ld      c_flushErrorCount,r12   # Load Flush Error count
        addo    1,r12,r12               # Increment count
        st      r12,c_flushErrorCount   # Store Flush Error count
#
# --- Set the Error bit for this virtual ID
#
        ldos    tg_vid(r15),r12         # Get Virtual ID
        ld      vcdIndex[r12*4],r13     # Get Virtual Cache Definition ptr
        cmpo    0,r13                   # Check for valid virtual ID
        faulte
        ldob    vc_stat(r13),r14        # Get VID cache status
        setbit  vc_error,r14,r14        # Set Error state bit
        stob    r14,vc_stat(r13)        # Set VID cache status
#
# --- Allocate memory for a RB tree node.
#
c       g0 = s_MallocW(rbesize, __FILE__, __LINE__); # Allocate RB node
#
# --- Fill in RB node parameters. The starting buffer address is used
#       as the key and the ending buffer address is used as the keym.
#       This results in a tree sorted by buffer address. This tree is
#       later used to free memory to initialize the cache buffer.
#
        movq    0,r4                    # Clear r4-r7
        ld      tg_bufptr(r15),r4       # Get starting buffer address
        ld      tg_vlen(r15),r6         # Get length of request (blocks)
        shlo    9,r6,r6                 # convert from blocks to bytes
        stl     r4,rbkey(g0)            # Set key (start buffer address)
        addo    r6,r4,r6                # Add byte length to get end address
        stl     r6,rbkeym(g0)           # Set keym (end buffer address)
        st      r15,rbdpoint(g0)        # Set location of cache tag into node
#
# --- Get root for this tree and insert node.
#
        mov     g0,g1                   # g1 = node to insert in tree
        ld      c_bufRoot,g0            # g0 = root of failed tree
        call    RB$insert               # Insert node into tree
        st      g0,c_bufRoot
#
# --- Exit
#
        mov     r15,g0                  # restore g0
        ret
#
#**********************************************************************
#
#  NAME: wc$initCache
#
#  PURPOSE:
#       To perform initialization of Write Cache structures.
#
#  DESCRIPTION:
#       This routine performs the necessary initialization of Write Cache
#       structures before the Write Cache executive processes are started.
#       No requests may be submitted until structures have been initialized.
#
#       The following initialization actions are performed:
#       1) The free cache tag and block counts are initialized.
#       2) The FMM for the buffer area is initialized.
#       3) Any tags that the flush attempt at power failed are added to the
#          appropriate dirty and io trees.
#       4) The LRU queue sentinels are initialized.
#       5) The entire cache tag area is mirrored to the backend processor
#           cache tag area.
#
#       This routine is intended for use after power-on or after recovery of
#       an existing cache memory. It must not be called until recovery is
#       completed!
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0, g1, g2.
#
#**********************************************************************
# r5 = next node
# r6 = current node
# r7 = root of tree
# r8 = end of current node
# r9 = start of next node
#**********************************************************************
#
wc$initCache:
#
# --- Setup FMM for cache buffer area. Assumes start and size are multiples
#     of memory granularity.
#
        ld      WcbAddr,g0              # Get beginning of cache buffer area
        ld      WcbSize,g1              # Get sizeof cache buffer area
#
# --- Initialize the Free Cache Block Count by calculating how many blocks are
#     in the Cache by Size/Sector size.
#
        divo    (SECSIZE+EXTRA_SPACE+EXTRA_SPACE),g1,r3
        st      r3,C_ca+ca_blocksFree   # Save the number free in Counter structure
#
# --- Initialize the Free Cache Tag Count
#
        ld      C_numtags,r3            # r3 = the number of free tags
        st      r3,C_ca+ca_tagsFree     # Store the number of free tags
#
# -- Point the fmm to the fms.
#
c       ((struct fmm *)&c_wcfmm)->fmm_fms = (struct fms *)&c_wcfms; # Save FMS pointer of wc buffer addr
c       ((struct fmm *)&c_wcfmm)->fmm_waitstat = pccwb; # Set memory wait status
.if     KDEBUG4
c       ((struct fmm *)&c_wcfmm)->fmm_options = 0;      # Set to check Free memory linked list.
.else   # KDEBUG4
c       ((struct fmm *)&c_wcfmm)->fmm_options = 1 << fm_op_nofmcheck; # Set "No Free Memory Checking Option".
.endif  # KDEBUG4
#
# --- Check if cache recovery failed on any tags. Failed tags are stored in a
#     RB tree sorted by starting buffer address. If failed tags exist, cache
#     buffer space for it is not freed. Tag is added to dirty and io trees.
#
        ld      c_bufRoot,r7            # Get root of failed tree
        cmpobne 0,r7,.wcic20            # Jif failed tags exist.
#
# --- No failed tags exist, the entire cache buffer space is freed.
#     g0 = address start
#     g1 = size of area
c       k_init_mem((struct fmm *)&c_wcfmm, (void *)g0, g1);
        b       .wcic80
#
# ---- Failed tags exist, start at the beginning of the cache buffer area.
#
.wcic20:
# --- Set up the fmm and fms structures (as best we can).
c       ((struct fmm *)&c_wcfmm)->fmm_first.thd = NULL; # Clear the first free link
c       ((struct fmm *)&c_wcfmm)->fmm_first.len = 0;    # Clear the segment length
c       ((struct fmm *)&c_wcfmm)->fmm_sorg = 0;         # Clear the secondary origin field
# --- Set min/max/available memory.
c       ((struct fms *)&c_wcfms)->fms_Available_memory = 0; # Nothing yet.
c       ((struct fms *)&c_wcfms)->fms_Maximum_available = g1 - (3 * EXTRA_SPACE);   # Good luck!
c       ((struct fms *)&c_wcfms)->fms_Minimum_available = 0; # Nothing yet.
c       ((struct fms *)&c_wcfms)->fms_Number_tasks_waiting = 0; # Clear the number of tasks waiting
#
.set EXTRA_SPACE,32
c       r8 = g0 - EXTRA_SPACE;          # Get end of previous possible buffer (argh)
# --- Get the first node (lowest key) from the tree.
#     g0 = root element, g2 = Key LSB, g3 = Key MSB
#
        mov     r7,g0                   # g0 = root of failed tree
        mov     r8,g2                   # g2,g3 = key
        mov     0,g3
        call    RB$locateStart          # Returns g1 pointing to node
        mov     g1,r6                   # Save current node found
#
# --- Check for free space at the beginning of the cache buffer
#
        ld      rbkey(r6),r9            # Get key (next buffer address start)
c       if (r9 < r8) {                  # Disaster -- old write cache, new methodology.
c           abort();
c       }
        cmpobe r8,r9,.wcic30            # Jif equal to start of cache buffer
#
# --- There is free space ahead of this node, treat the node as the next node.
#
        mov     r6,r5                   # Set next node
        b       .wcic60
#
# --- Get the end of the current used chunk of memory. This is also
#     potentially the start of a free block.
#
.wcic30:
        ld      rbkeym(r6),r8           # Get keym (end buffer address)
#
# --- Get the next node (next lowest key) from the tree using current node.
#     g1 = address of current node element of tree.
        mov     r6,g1
        call    RB$locateNext
        mov     g1,r5                   # Set next node
#
# --- Check if the end of the tree was reached.
#
        cmpobne 0,g1,.wcic40            # Jif not at the end of the tree
        ld      WcbAddr,r9              # r9 = end of the cache buffer area
        ld      WcbSize,g0
        addo    g0,r9,r9                # Put next node at end of area.
c       r9 += EXTRA_SPACE;              # Put next node after pre-header.
        b       .wcic50
#
# --- Get the start of the next used chunk of memory. This is also
#       potentially the end of a free block.
#
.wcic40:
        ld      rbkey(r5),r9            # Get key (next node start buffer address)
#
# --- Need to allocate a RB node for this tag and put it into the cache tag
#     tree. Also need to assign another RB node to place it into the dirty
#     tree.
#
.wcic50:
        ld      rbdpoint(r6),g0         # Get cache tag address
        call    wc$insert_io_node       # Put into RB tag tree
        call    wc$insert_dirty_node    # Put into dirty tree
#
# --- Increment the number of dirty blocks and Decrement the Free Blocks
#
        ld      tg_vlen(g0),g0          # g0 = Number of blocks for this op
        call    wc$IncDirtyCount        # Increment the Dirty Counts
        call    wc$DecFreeCount         # Decrement the Free Counts
#
# --- Delete the node from the tree
#
        mov     r7,g0                   # g0 = root of failed tree
        mov     r6,g1                   # g1 = pointer to element to delete.
        call    RB$delete               # delete node
        mov     g0,r7                   # g0 = new root of failed tree
        st      r7,c_bufRoot            # save new root of failed tree
.wcic60:
        mov     r8,g0                   # g0 = starting address of data
        subo    r8,r9,g1                # g1 = size of data
c       g1 -= (EXTRA_SPACE+EXTRA_SPACE); # Subtract post-header and pre-header
#
c       k_make_pre_post_headers(g0, g1);
#
        cmpible 0,g1,.wcic70            # Jif size less than or equal 0
#
# --- Release the unused area of the cache buffer area
#       g0 = address to release
#       g1 = size of area to release
c       k_mrel((void *)g0, g1, (struct fmm *)&c_wcfmm, __FILE__, __LINE__); # Free memory
#
# --- The next node becomes current node. Loop if new current node is not zero.
#
.wcic70:
        mov     r5,r6                   # Copy next node into current node
        cmpobne 0,r6,.wcic30            # loop if node is non-zero
#
# --- Setup LRU ---------------------------------------------------------------
#
.wcic80:
        lda     c_hlruq,r3              # Get addr of head sentinel
        lda     c_tlruq,r4              # Get addr of tail sentinel
        st      r3,tg_bthd(r4)          # Save backward pointer of tail
        st      0,tg_fthd(r4)           # Clear forward pointer of tail
        st      r4,tg_fthd(r3)          # Set forward pointer of head
        st      0,tg_bthd(r3)           # Clear backward pointer of head
        ret
#
#**********************************************************************
#
#  NAME: wc$setSeqNo
#
#  PURPOSE:
#       Sends a message to the back end processor to update
#       the sequence number stored in NVRAM.
#
#  DESCRIPTION:
#       This function will create a message to send across the PCI bus
#       to the back end processor.
#
#  CALLING SEQUENCE:
#       call    wc$setSeqNo
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
# r3 = K_ficb pointer
# r8 = sequence number
#**********************************************************************
#
        .section    .shmem
# This data area will be overwritten by the other processes define, but the
# information returned is never used -- and as such doesn't matter.
setSeqNoReturn:
        .space  msqrsiz
        .text
#
wc$setSeqNo:
        movq    g0,r12                  # Save g0-g3
        movt    g4,r4                   # Save g4-g6
#
# --- Create the message.
#
# NOTE: This message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ld      K_ficb,r3               # Get FICB
        ld      fi_seq(r3),r8           # Get sequence number
        st      r8,msq_seq(g0)          # Store the sequence number in packet
        ldconst msqsiz,g1               # Length of packet
        ldconst mrfesetseq,g2           # MRP function code
        lda     setSeqNoReturn,g3       # Return data that is ignored
        ldconst msqrsiz,g4              # Length of return data (ignored data)
        ldconst 0,g5                    # No completion function
        ldconst 0,g6                    # No user defined data
#
# --- Send the message
#
        call    L$send_packet           # Send the packet
#
# --- Exit
#
#        c       fprintf(stderr, "wc$setSeqNo:  Sequence number set to 0x%lx\n", r8);
        movq    r12,g0                  # Restore g0-g3
        movt    r4,g4                   # Restore g4-g6
        ret
#
#**********************************************************************
#
#  NAME: wc$batHealth
#
#  CALLING SEQUENCE:
#       call    wc$batHealth
#
#  INPUT:
#       g0 = battery board 0 = FE 1 = BE.
#       g1 = battery state 0 = okay 1 = battery low
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# void WC_batHealth(UINT8 board, UINT8 state);
        .globl  WC_batHealth
WC_batHealth:
WC$batHealth:
.if WC_ENABLE
#
# --- Wait until C$init has initialize the Cache Information Structure
#
        lda     C_ca,r3                 # Get the Cache Information pointer
        ldob    ca_status(r3),r4        # Get the cache status
        ldob    ca_status2(r3),r5       # Get the Battery State and Temp Disable
#
# --- Sanity check the value in g0 (should be either 0 or 1)
#
        cmpo    2,g0
        faultle
#
# --- Check the battery state
#
        cmpobne ca_bat_good,g1,.wcbh10
#
# --- Battery is good, clear the battery low indicator
#
        clrbit  g0,r5,r5
#
# --- Update the cache state. If the cache enable was pending, then
#     clear the disable in progress and set the global enable if the mirror
#     partner has been set up.
#
        cmpobne 0,r5,.wcbh30            # Jif either battery is low
        bbc     ca_ena_pend,r4,.wcbh30  # Jif enable not pending
#
        stob    r5,ca_status2(r3)       # Save the Battery State
        clrbit  ca_ena_pend,r4,r4       # Clear the Enable Pending
        stob    r4,ca_status(r3)        # Save the new status
        mov     g0,r15                  # Save g0
        ldconst 0xFFFFFFFF,g0           # g0 = Global Enable
        call    C$enable                # Enable the cache if possible
        mov     r15,g0                  # Restore g0
        b       .wcbh40
#
# --- Battery is low, set the battery low indicator
#
.wcbh10:
        setbit  g0,r5,r5
#
# --- Update the cache state. If the cache is enabled, then
#     set the disable in progress.
#
        bbc     ca_ena,r4,.wcbh30       # Jif cache is disabled
        bbs     ca_dis_ip,r4,.wcbh30    # Jif disable is already pending
#
# --- Disable the cache globally (the background flush task will start the
#       flushing process without holding up customer ops).
#
        call    wc$SetGlobalDisable     # Set Global Cache to disabled
#
# --- Set the global write cache enable pending bit
#
        ldob    ca_status(r3),r4        # Get the cache status
        setbit  ca_ena_pend,r4,r4       # Set global enable pending
#
# --- Update the global cache status
#
        stob    r4,ca_status(r3)        # Save the global cache status
#
# --- Update the battery status
#
.wcbh30:
        stob    r5,ca_status2(r3)       # Save the Battery State
#
# --- Exit
#
.wcbh40:
.else # WC_ENABLE
#  Do not allow Write Cache to be enabled by not allowing the batteries to go
#  to a GOOD state
.endif # WC_ENABLE
        ret
#
#**********************************************************************
#
#  NAME: WC$continueWithoutMirrorPartner
#
#  PURPOSE:
#       Lets the Write Cache initialization continue when the remote
#       Mirror Partner is not available to test the mirrored data.
#
#  DESCRIPTION:
#       This function verifies the Write Cache is in the proper state to
#       continue initialization, and if so ignores the mirrored data in the
#       remote Mirror Partner's Back End DRAM.
#
#  CALLING SEQUENCE:
#       call    WC$continueWithoutMirrorPartner
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g0 = return code.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
WC$continueWithoutMirrorPartner:
        ldos    K_ii+ii_status,r4       # Get initialization status
        bbs     iimpfound,r4,.wccwmp_90 # Jif Mirror Partner already found
#
# --- Set the Mirror Broken Status to let the initialization continue and
#     Kick off the task to determine if it will be OK to restart mirroring
#     controller information to the mirror partner
#
        lda     C_ca,r3                 # r3 =Cache Information pointer
        ldob    ca_status(r3),r4        # r4 = Cache status
        setbit  ca_mirrorbroken,r4,r4   # Set the Mirror Broken Bit
        stob    r4,ca_status(r3)        # Save the new Cache status
        call    DLM$StartFECheckingMP   # Start checking the MP Status
        ldconst deok,g0                 # g0 = Return Good status
        b       .wccwmp_100
#
# --- Cache is already initialized. Report the error
#
.wccwmp_90:
        ldconst deinvop,g0
#
# --- All Done
#
.wccwmp_100:
        ret
#
#**********************************************************************
#
#  NAME: wc$processMirroredTags
#
#  PURPOSE:
#       Processes the mirrored cache tags that exist in the back-end of
#       of this controller and constructs the necessary Dirty and Cache
#       Valid RB trees based on the cache tag information for those VIDs
#       that are requested.
#
#  DESCRIPTION:
#       When another controller fails over to this controller, this function
#       is called to process the mirrored cache tags that exist in the back
#       end of this controller. These cache tags describe all the dirty data
#       for virtual disks that were previously owned by the failing controller
#       that have been mirrored to this controller.
#
#       This function scans the back end DRAM for dirty cache tags.
#       For each dirty cache tag that is found and requested, a corresponding
#       node is added to both the Dirty and Cache Valid RB trees for the VID
#       indicated by the cache tag. After the fail-over occurs, this
#       controller will check for overlaps with any dirty data for these
#       VIDs using the Cache Valid RB tree. The Dirty RB tree will be used
#       to flush all the dirty data from the cache.
#
#       Several fields in the cache tag are also cleared/updated to
#       eliminate any extraneous information from  the other controller
#       (such as from a flush in progress). These fields include:
#           - Forward/Backward Thread pointers = Cleared
#           - Read in Progress Count = Cleared
#           - Tag State = Cleared
#           - Next Dirty = Cleared
#           - Head/Tail Unlock Queue = Cleared
#           - Tag Attribute = clear all bits except Dirty bit
#           - Buffer Pointer = convert from FE addr to BE addr
#           - Cache Valid and Dirty RB Node pointers = point to new RB nodes
#
#  CALLING SEQUENCE:
#       call wc$processMirroredTags
#
#  INPUT:
#       g0 = Global or VID List Flag
#               0 = VID List provided (only restore VIDs with TRUE Flag in list)
#               1 = Global (create trees for all tags found with valid VIDs
#                       and set TRUE Flag in list)
#       g1 = VID List Pointer -> This is a byte array for each possible
#                   valid VID. Based on the g0 flag:
#               0 = Only create trees for VIDs that have a TRUE in the byte
#                   array provided (assumes the caller has set to TRUE only
#                   those VIDs that need to be created and all others have
#                   been set to FALSE).
#               1 = For each found VID, the byte will be set to TRUE (assumes
#                   caller has set to FALSE the entire array before calling
#                   this function).
#
#  OUTPUT:
#       g1 = VID List Pointer (only updated if Global was requested)
#
#  REGS DESTROYED:
#       None
#
#  REGISTER USAGE:
#        r3 = Number of tags found dirty with an invalid VID associated with it
#        r4 = End address for cache tag area
#        r5 = Size of individual tag
#        r6 = Maximum number of VIDs allowed
#        r7 = TRUE flag
#        r8 = Clearing register (0)
#       r10 = Tag Attributes
#       r11 = VID being processed
#       r12 = Temporary
#       r13 = Address of cache tag to process
#       r14 = Global or VID List flag
#       r15 = VID List Pointer
#
#        g0 = Parameter passing / Invalid Tags count
#        g1 = Parameter passing
#
#**********************************************************************
#
wc$processMirroredTags:
        PushRegs                        # Save all G registers (stack relative)
        movl    g0,r14                  # Set r14 and r15 for usage
#
# --- Initialize the starting address for the BE mirrored cache tags
#     and the size of each cache tag to process.
#
        ld      WctAddr,r13             # r13 = Address of beginning of tag area
        lda     BE_ADDR_OFFSET(r13),r13 # Translate to BE Address
        ldconst 0,r3                    # r3 = Number of invalid tags
        ldconst tgsize,r5               # r5 = Size of individual tag
        ldconst MAXVIRTUALS,r6          # r6 = Maximum number of VIDs
        ldconst TRUE,r7                 # r7 = TRUE value
        ldconst 0,r8                    # r8 = clearing register
#
# --- Get ending address of cache tags to terminate loop
#
        ld      WctSize,r4              # r4 = Size of cache tag area
        addo    r13,r4,r4               # r4 = Set limit of tag area
#
# --- Start of loop to process each cache tag
#
.wcpmt10:
#
# --- Check if this tag is dirty.
#
!       ldos    tg_attrib(r13),r10
        bbc     TG_DIRTY,r10,.wcpmt90   # Jif tag is not dirty
#
# --- This cache tag is dirty.
#     Determine if VID is valid and if not add to invalid count and go to next
#       cache tag. If it is valid, determine if it should be used based on
#       the flags and VID list. Ignore any Tags that show BE already (in case
#       the CCB issued the request twice).
#
!       ldos    tg_vid(r13),r11         # r11 = VID associated with the tag
        cmpobl  r11,r6,.wcpmt30         # Jif the VID is in the valid range
.wcpmt20:
#
#   An invalid VID was found, report the first one and then total count (if more
#   than one.
#
        addo    1,r3,r3                 # Increment the invalid Tag count
        cmpobne 1,r3,.wcpmt90           # Jif already logged one error
#
        ldconst cac_sft2,r12            # r12 = error code to log
        lda     C_sft_flt,g0            # g0 = Software Fault Log Area
        st      r12,efa_ec(g0)          # Save the Error Code
        st      r13,efa_data(g0)        # Save the Tag Address
        st      r10,efa_data+4(g0)      # Save the Tag Attribute
        st      r11,efa_data+8(g0)      # Save the Tag VID
!       ld      tg_vsda(r13),r12        # Get the 1st half of LBA
        st      r12,efa_data+12(g0)     # Save the 1st half of LBA
!       ld      tg_vsda+4(r13),r12      # Get the 2nd half of LBA
        st      r12,efa_data+16(g0)     # Save the 2nd half of LBA
        ld      tg_vlen(r13),r12        # Get the Number of Blocks
        st      r12,efa_data+20(g0)     # Save the number of Blocks
        ld      tg_bufptr(r13),r12      # Get the Buffer Pointer
        st      r12,efa_data+24(g0)     # Save the Buffer Pointer
        ldconst 32,r12                  # Number of bytes saved (ec + data)
        st      r12,mle_len(g0)         # Save the number of bytes to send
        call    M$soft_flt              # Error Trap or Log failure
        b       .wcpmt90                # Go look at the next tag
#
.wcpmt30:
        ld      vcdIndex[r11*4],r12     # r12 = VCD pointer
        cmpobe  0,r12,.wcpmt20          # Jif VCD is not valid
#
        bbc     TG_BE,r10,.wcpmt40      # Jif tag has not been processed yet
#
#   This tag has been processed before. Determine if it is in the tree already
#       or if this is being reprocessed after a Power cycle. If in the tree
#       already, ensure it is the same tag. If not the same tag, report a
#       Software Fault and ignore it. If the tag is not in the tree, continue
#       as not processed before.
#
        ld      tg_vlen(r13),g4         # g4 = Length of data in Write Cache
!       ldl     tg_vsda(r13),g2         # g2/g3 = Starting Disk Address
        ld      vc_cache(r12),g0        # g0 = The cache RB tree root
        cmpdeco 0,g4,g4                 # Adjust size/clear carry bit
                                        #   (assumes size != 0)
        addc    g4,g2,g4                # g4/g5 = Calculate end range
        addc    0,g3,g5                 #   of request
        call    RB$foverlap             # Check for cache hit
        cmpobe  FALSE,g1,.wcpmt40       # Jif this tag has not been processed
        ld      rbdpoint(g1),g2         # g2 = Cache Tag pointer
        cmpobe  g2,r13,.wcpmt90         # Jif this Cache Tag has been processed
#
#   This tag overlaps with other data already in the tree. Report the problem
#       and then ignore the tag.
#
        ldconst cac_sft6,r12            # r12 = error code to log
        lda     C_sft_flt,g0            # g0 = Software Fault Log Area
        st      r12,efa_ec(g0)          # Save the Error Code
        st      r13,efa_data(g0)        # Save the Tag Address
        st      r10,efa_data+4(g0)      # Save the Tag Attribute
        st      r11,efa_data+8(g0)      # Save the Tag VID
!       ld      tg_vsda(r13),r12        # Get the 1st half of LBA
        st      r12,efa_data+12(g0)     # Save the 1st half of LBA
!       ld      tg_vsda+4(r13),r12      # Get the 2nd half of LBA
        st      r12,efa_data+16(g0)     # Save the 2nd half of LBA
        ld      tg_vlen(r13),r12        # Get the Number of Blocks
        st      r12,efa_data+20(g0)     # Save the number of Blocks
        ld      tg_bufptr(r13),r12      # Get the Buffer Pointer
        st      r12,efa_data+24(g0)     # Save the Buffer Pointer
        st      g2,efa_data+28(g0)      # Save the other tag address
        ldos    tg_attrib(g2),r12       # Get the other tag attribute
        st      r12,efa_data+32(g0)     # Save the other tag attribute
!       ld      tg_vsda(g2),r12         # Get the 1st half of LBA
        st      r12,efa_data+36(g0)     # Save the 1st half of LBA
!       ld      tg_vsda+4(g2),r12       # Get the 2nd half of LBA
        st      r12,efa_data+40(g0)     # Save the 2nd half of LBA
        ld      tg_vlen(g2),r12         # Get the Number of Blocks
        st      r12,efa_data+44(g0)     # Save the number of Blocks
        ld      tg_bufptr(g2),r12       # Get the Buffer Pointer
        st      r12,efa_data+48(g0)     # Save the Buffer Pointer
        ldconst 56,r12                  # Number of bytes saved (ec + data)
        st      r12,mle_len(g0)         # Save the number of bytes to send
        call    M$soft_flt              # Error Trap or Log failure
        b       .wcpmt90                # Go look at the next tag
#
#   This tag has not been processed before. Continue operating on.
#
.wcpmt40:
        cmpobe  0,r14,.wcpmt50          # Jif it is a VID List
        stob    r7,(r15)[r11*1]         # Global - Set VID value to TRUE
        b       .wcpmt60                # Go process the tag
#
.wcpmt50:
        ldob    (r15)[r11*1],r12        # VID List, get Flag
        cmpobe  FALSE,r12,.wcpmt90      # Jif not a VID to be processed
#
# --- Tag has a valid VID.
#     Set only the TG_DIRTY bit in the attribute field to ensure that
#     all other attribute bits are cleared. Then call wc$initTag
#     to set the attribute and clear the other desired fields in
#     the cache tag.
#
.wcpmt60:
        setbit  TG_DIRTY,0,r12          # Setup initial tag attrib = DIRTY
        setbit  TG_BE,r12,r12           #  and BE
!       stos    r12,tg_attrib(r13)      # Set tag attributes
!       st      r8,tg_fthd(r13)         # Clear forward thread pointer
!       st      r8,tg_bthd(r13)         # Clear backward thread pointer
!       stos    r8,tg_rdcnt(r13)        # Clear read In progress count
!       stos    r8,tg_state(r13)        # Clear tag state
!       st      r8,tg_ioptr(r13)        # Clear cache tree ptr
!       st      r8,tg_dirtyptr(r13)     # Clear dirty tree node pointer
!       st      r8,tg_nextdirty(r13)    # Clear next dirty block
!       st      r8,tg_hqueue(r13)       # Clear the Unlock Queue Forward Ptr
!       st      r8,tg_tqueue(r13)       # Clear the Unlock Queue Backward Ptr
#
# --- Allocate a RB node for this tag and put it into the Cache Valid RB
#     tree. Also need to assign another RB node to place it into the Dirty
#     RB tree.
#
        mov     r13,g0                  # g0 = cache tag address
        call    wc$insert_io_node       # Put into Cache Valid RB tree
        call    wc$insert_dirty_node    # Put into Dirty RB tree
#
# --- Convert the buffer address in the cache tag from a FE to a BE
#     PCI address. This address will get used directly by the cache
#     flush routines to flush the data from cache.
#
!       ld      tg_bufptr(r13),r12      # Get buffer pointer from tag
        lda     BE_ADDR_OFFSET(r12),r12 # Translate to BE address
!       st      r12,tg_bufptr(r13)      # save buffer pointer in tag
#
# --- Increment to the next cache tag
#
.wcpmt90:
        addo    r13,r5,r13              # Get next tag address
        cmpobl  r13,r4,.wcpmt10         # Jif not past end of tag area
#
# --- Determine if there was more than one tag that was invalid. If there were
#       more than one, Software Fault with the total count.
#
        cmpobge 1,r3,.wcpmt99           # Jif zero or 1 bad tag found
#
        ldconst cac_sft3,r12            # r12 = error code to log
        lda     C_sft_flt,g0            # g0 = Software Fault Log Area
        st      r12,efa_ec(g0)          # Save the Error Code
        st      r3,efa_data(g0)         # Save the Number of invalid BE tags
        ldconst 8,r12                   # Number of bytes saved (ec + data)
        st      r12,mle_len(g0)         # Save the number of bytes to send
        call    M$soft_flt              # Error Trap or Log failure
#
# --- Exit
#
.wcpmt99:
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret                             # Exit
#
#**********************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
