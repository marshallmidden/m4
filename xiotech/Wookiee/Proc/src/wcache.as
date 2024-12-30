# $Id: wcache.as 159870 2012-09-20 12:59:51Z marshall_midden $
#******************************************************************************
#
#  NAME: wcache
#
#  PURPOSE:
#       To provide Wookiee Cache support.
#
#  Copyright (c) 2000-2010 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- Global Functions
#
        .globl  WC$deleteVIDdata
        .globl  WC$InvalidateFE
        .globl  WC$InvalidateBE
        .globl  WC$delete_node
        .globl  WC_SetGlobalDisable
#
# --- Global Data
#
        .globl  c_wcfmm
        .globl  c_wcfms
        .globl  wc_mem_seqnum
        .globl  gWCErrorMP
        .globl  gWCMarkCacheQueue
        .globl  gWCMirrorBETagQueue
#
# --- Local data definitions
#
        .data
        .align  2
#
# --- Write/Read Memory data needs
#
wc_mem_seqnum:
        .short  0                       # Datagram Message Sequence Number
#
        .align  4
gWCMarkCacheQueue:
        .space  qusiz,0                 # Mark Cache exec queue control block
gWCMirrorBETagQueue:
        .space  qusiz,0                 # Mirror to BE NVRAM Queue
gWCErrorMP:
        .word   0                       # Mirror Failure Mirror Partner
c_wcfmm:
        .space  fmsiz,0                 # Write Cache FMM structure
c_wcfms:
        .space  fssiz,0                 # Write Cache FMS structure
#
        .text
#
#******************************************************************************
#
#  NAME: WC$start
#
#  PURPOSE:
#       To start Write Cache executive processes.
#
#  DESCRIPTION:
#       This routine forks the necessary executive processes for Write Cache.
#       It forks the following processes:
#
#       Mirror Executive
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Possibly all G registers.
#
#******************************************************************************
#
WC$start:
#
# --- Establish executive process -------------------------------------
#
        lda     c$exec,g0               # Establish executive process
        ldconst CEXECPRI,g1
c       CT_fork_tmp = (ulong)"c$exec";
        call    K$fork
        st      g0,C_exec_pcb           # Save PCB
#
# --- Establish the I/O executive process -----------------------------
#
        lda     c$ioexec,g0             # Establish the I/O executive process
        ldconst CIOEXECPRI,g1
c       CT_fork_tmp = (ulong)"c$ioexec";
        call    K$fork
        st      g0,C_ioexec_pcb         # Save the PCB
#
        lda     WC$FlushTask,g0         # Start the Flush Task Executive
        ldconst CFLUSHPRI,g1
c       CT_fork_tmp = (ulong)"WC$FlushTask";
        call    K$fork
#
        lda     WC$BackgroundFlushTask,g0 # Start the Background Flush Task
        ldconst CBFTPRI,g1
c       CT_fork_tmp = (ulong)"WC$BackgroundFlushTask";
        b       K$fork                  # Start and exit

#******************************************************************************
#
#  NAME: WC$Wsubmit
#
#  PURPOSE:
#       To submit an Write I/O to the write cache.
#
#  DESCRIPTION:
#       This routine is passed write I/Os directed at a particular
#       Virtual ID (VID).  It determines the following:
#
#       1).  Complete or partial cache "hit".
#       2).  Candidacy for caching.
#
#       If the incoming I/O does not "hit" any previously cached I/Os
#       and is not a candidate for caching, it is immediately submitted
#       as an I/O to the back-end processor.
#
#       If the incoming I/O partially "hits" a previously cached I/O
#       the hit I/O(s) are invalidated (flushed if necessary).
#
#       If the I/O "hits" an entire previously cached I/O that is in the BE
#       mirrored data, the BE tag will be invalidated first before handling.
#
#       If the I/O "hits" an entire previously cached I/O the previous
#       I/O is replaced by the new one.  This is essentially done in
#       these steps:
#         a) The cache tag is locked.
#         b) The buffer is overwritten with the new data.
#         c) The tag is marked dirty, then buffer/tag mirrored.
#         d) The tag is unlocked.
#
#       If the tag/buffer are marked as dirty, then the following
#       steps are performed instead of the above steps:
#         a) A new cache buffer is assigned for the incoming I/O and
#            filled with data.
#         b) The new cache buffer is mirrored.
#         c) The cache tag for the previous I/O is reassigned to
#            the new buffer; the tag is then mirrored.
#         d) The old buffer is deassigned.
#
#       If insufficient resources exist for the new cache buffer the
#       previous I/O is invalidated (and flushed if necessary), then
#       steps a) through c) are performed.
#
#       For an I/O that does not "hit" any cached I/Os and is itself
#       a cache candidate, steps a). through c). are performed.
#
#       It is assumed that the I/O being submitted does not overlap any
#       other pending I/Os.
#
#  INPUT:
#        g1 = ILT for this write I/O; <vrvrp> points to VRP
#
#  OUTPUT:
#       g12 = T/F; TRUE implies this I/O is being cached;
#                  FALSE implies that this I/O must be submitted to disk.
#
#  REGS DESTROYED:
#       g0-g7
#
#**********************************************************************
#
WC$Wsubmit:
#
# --- Check if enough write cache resource are available to proceed (removed -
#       caused hang in C$Stop and also holds up Reads until WC is available)
#
#        call    wc$chkres               # CQ#12759 removed this call
#
        mov     TRUE,g12                # Set I/O cached
        ld      vrvrp(g1),r15           # Get pointer to VRP
        mov     g1,r8                   # Preserve ILT pointer
#
# --- Get root value for this VID
#
        ldos    vr_vid(r15),r14         # r14 = VID
        ld      vcdIndex[r14*4],r11     # r11 = VCD pointer for this VID
        ld      vc_cache(r11),r12       # Get the cache RB tree root
#
# --- Determine if this request hits any I/Os already cached.
#
        ldl     vr_vsda(r15),g2         # Get disk starting address
        ld      vr_vlen(r15),r13        # Get length of request
        cmpdeco 0,r13,r6                # Adjust size/clear carry bit
                                        #   (assumes size != 0)
        addc    r6,g2,g4                # Calculate end range
        addc    0,g3,g5                 #   of request
        mov     r12,g0                  # g0 = root of RB tree
        call    RB$foverlap             # Check for cache hit
        cmpobne FALSE,g1,.wcw05         # Jif cache hit
        ldl     vc_wrmiss(r11),r4       # Get the Write Miss Counter
        cmpo    0,1                     # Clear the Carry Bit
        addc    1,r4,r4                 # Increment the Write Miss Counter
        addc    0,r5,r5
        stl     r4,vc_wrmiss(r11)       # Save the Write Miss Counter
        b       .wcw80                  # Put the data in cache
#
.wcw05:
        mov     0,g6                    # Init counter for cache hits
        ld      rbdpoint(g1),r9         # get pointer to cache tag
#
# --- If the Tag is for a BE Mirrored Data, treat like a partial hit and
#       invalidate it.
#
        ldos    tg_attrib(r9),r6        # Get the tag attribute
        bbs     TG_BE,r6,.wcw07         # Jif this is a BE Mirrored tag
#
# --- Determine if this hit is a complete hit or partial
#     (start addresses and size equal)
#
!       ldl     tg_vsda(r9),r6          # Get starting address
        ld      tg_vlen(r9),r12         # get length of request
        cmpobne r12,r13,.wcw07          # Jif length not equal; partial hit
#
# --- Length of request is identical, now check if start address is
#     same.
#
        cmpobne r7,g3,.wcw07            # jif MSBs differ; partial hit
        cmpobne r6,g2,.wcw07            # jif LSBs differ; partial hit
#
# --- The start address is the same, now ensure we are not in error state,
#       trying to disable caching globally or for this device
#
        ldob    vc_stat(r11),r4         # r4 = VID Cache Status
        and     VC_NO_MORE_DATA,r4,r4   # r4 = Bits to check (error & disable)
        cmpobne 0,r4,.wcw07             # Jif any bit is on (error or disable)
                                        #  and do not let more data in cache
        lda     C_ca,r4                 # Get the Global Cache Pointer
        ldob    ca_status(r4),r4        # r4 = Global Cache Status
        bbs     ca_dis_ip,r4,.wcw07     # Jif if the Global Cache is being
                                        #  disabled (do not let more data in)
#
# --- If the operation already has write buffers allocated (such as when
#       a Format Unit gets transformed to a write with the data pattern already
#       set up in the non-cached write buffers), handle like a partial hit
#       and let process I/O route to the non-cached routines
#
        ld      vr_sglptr(r15),r4       # Get the SGL Pointer
c       if (r4 == 0xfeedf00d) {
c           fprintf(stderr,"%s%s:%u WC$Wsubmit sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__);
c           abort();
c       }
        cmpobe  0,r4,.wcw70             # Jif no SGL Pointer - exact hit
        ldob    sg_flag(r4),r4          # Get flag byte
        bbc     sg_buffer_alloc,r4,.wcw70 # Jif a buffer is not already
                                        #    allocated - exact hit
#
# --- Incoming I/O is a partial cache hit.  Now find all segments affected
#     and queue them to be invalidated.
#
.wcw07:
        ldconst 0,r10                   # r10 = Zero register
        ldl     vc_wrpart(r11),r4       # Get the Write Partial Counter
        cmpo    0,1                     # Clear the Carry Bit
        addc    1,r4,r4                 # Increment the Write Partial Counter
        addc    0,r5,r5
        stl     r4,vc_wrpart(r11)       # Save the Write Partial Counter
        stob    r10,ioccode(r8)         # Clear Completion Code of future ops
        st      r10,iointcr(r8)         # No intermediate completion routine
#
#       Determine if the cache is in an Error State.  If in error state, only
#       let the op continue if all the overlapping tags are resident.  If a
#       non-resident tag is found that is overlapping, put the op on the Error
#       task queue to be handled later when the error state is resolved
#
        lda     C_ca,r4                 # r4 = The Global Cache Pointer
        ldob    ca_status(r4),r5        # r5 = Global Cache Status
        bbc     ca_error,r5,.wcw30      # Jif if the Global Cache is not in the
                                        #  error state (handle like normal)
        mov     g1,r12                  # Save the original first RB Tree node
.wcw10:
        ld      rbdpoint(g1),r9         # r9 = The cache tag address
        ldos    tg_attrib(r9),r6        # r6 = The tag attribute
        bbs     TG_RESIDENT,r6,.wcw20   # Jif this is a resident tag - check nxt
#
#       A non-resident tag was found when the Write Cache is in error state.
#       Queue the op to the Error Task to come back into routine that will call
#       this function again.
#
        lda     wc$Wsubmit_restart,r4   # r4 = Function to call when error state
                                        #  is cleared up.
        mov     r8,g1                   # Restore the ILT pointer
        st      r4,il_cr(r8)            # Save away the restart routine address
        call    c$qerror                # Queue the ILT to the error task
        b       .wcw65                  # g12 = TRUE - ILT is handled to be
                                        #   cached - return to caller
#
#       Only resident tags found so far.  Check next overlapped tag for this
#       operation.
#
.wcw20:
                                        # g1 = RB pointer for last overlap
                                        # g4/g5 = Ending LBA
        call    RB$noverlap             # Find next overlap, if any
                                        # g1 = Null if none, else next RB ptr
        cmpobne FALSE,g1,.wcw10         # Continue check all the overlaps
#
#       No more overlaps and all were resident.  Let the op continue.
#
        mov     r12,g1                  # Restore the first RB tree node
#
#       Normal operations for partial hits continue at this point
#
.wcw30:
        ld      rbdpoint(g1),r9         # Get cache tag address
#
#     r8 = ILT of incoming request
#     r9 = current cache tag
#    r13 = length of request
#    r14 = VID
#    r15 = VRP pointer
#
#    g1 = RB tree pointer for this I/O
#    g2 = LSB disk start address - incoming I/O
#    g3 = MSB disk start address - incoming I/O
#    g4 = LSB last address of incoming I/O
#    g5 = MSB last address of incoming I/O
#
# --- Perform necessary processing to invalidate tag.
#
        cmpobne 0,g6,.wcw40             # Jif more than one hit
        lda     .wcw90,r6               # Completion routine when
                                        #  all segments invalidated
        st      r6,il_cr(r8)            # Set ILT completion routine
#
.wcw40:
#
# --- Since <wc$FlushRequest> may change the RB tree, we need to find the next
#     possible overlap before invalidating the tag.  The result of the search
#     will be checked after the invalidate to see if any more tags need to
#     be invalidated.
#
        call    RB$noverlap             # Find next overlap, if any
        mov     g1,r3                   # Preserve results of search
#
        mov     r9,g0                   # g0 = cache tag to invalidate
        ldconst INVALIDATE_REQUEST,g2   # Set intent to invalidate
        mov     r8,g1                   # g1 = ILT for request
        call    wc$FlushRequest         # Queue for invalidate
        cmpobne TRUE,g0,.wcw60          # Jif tag already invalidated
        addo    1,g6,g6                 # Increment hit count if queued
        stos    g6,iocounter(r8)        # Save hit count
#
.wcw60:
#
        mov     r3,g1                   # Restore results of next overlap
        cmpobne FALSE,r3,.wcw30         # Jif overlap found
#
# --- No more overlaps found. If outstanding counter > 0, return.
#
        cmpobe  0,g6,.wcw80             # Jif all tags invalidated
#
# --- All done
#
.wcw65:
        ret
#
# --- Cache segment hit was exact match for incoming I/O ----------------------
#
.wcw70:
#
#     r8 = ILT of incoming request
#     r9 = current cache tag
#    r12 = Root pointer for this VID
#    r13 = length of request
#    r14 = VID
#    r15 = VRP pointer
#
#    g1 = RB tree pointer for this I/O
#    g2 = LSB disk start address - incoming I/O
#    g3 = MSB disk start address - incoming I/O
#    g4 = LSB last address of incoming I/O
#    g5 = MSB last address of incoming I/O
#
        ldl     vc_wrhits(r11),r4       # Get the Write Hits Counter
        cmpo    0,1                     # Clear the Carry Bit
        addc    1,r4,r4                 # Increment the Write Hits Counter
        addc    0,r5,r5
        stl     r4,vc_wrhits(r11)       # Save the Write Hits Counter
        mov     r9,g0                   # g0 = Cache tag hit by request
        mov     r8,g1                   # g1 = I/O ILT that hits cache seg
        b       wc$wcache_hit           # Process write cache hit/return
#
# --- No cache segment hit for this I/O ---------------------------------------
#     or all overlaps invalidated
#
#     Continue with I/O process; process I/O will attempt to cache this I/O
#       if caching is not being disabled or in error state
#
.wcw80:
#
#     r8 = ILT pointer for this I/O
#    r11 = VCD pointer for this VID
#    r15 = VRP for this request
#
        mov     r15,g0                  # g0 = VRP
        mov     r8,g1                   # g1 = ILT
#
        b       wc$process_io           # Process ILT/return
                                        # Op may or may not have been handled -
                                        #  g12 shows response to caller who will
                                        #  handle appropriately
#
.wcw90:
#
# --- Entry point after all queued invalidate requests complete.
#
#     g0 = error code
#     g1 = ILT for request
#
        cmpobne 0,g0,.wcw95             # Jif there was an error invalidating
        ld      vrvrp(g1),g0            # Get pointer to VRP
        call    wc$process_io           # Process ILT/return
        cmpobe  FALSE,g12,C$do_nc_op    # Jif Op was not handled & return
        ret                             # return - op was handled
#
# --- An error occurred with the invalidating of the cache, report the error
#       on the original request
#
.wcw95:
        b       wc$complete_io          # g0 and g1 already set up to complete
#
#******************************************************************************
#
#  NAME: wc$process_io
#
#  PURPOSE:
#       To process a write request that possibly could be cached.
#
#  DESCRIPTION:
#       This routine is passed a write I/O that does not hit any cache segment
#       for the particular VID.  The request is then checked for eligibility
#       with the following steps:
#
#       1). The I/O must be <= the maximum allowable size, <WCACHEMAX>.
#       2). A cache tag must be available.
#       3). A buffer must be available.
#
#       If all of the above can be satisfied, the request is cached and
#       mirrored, then completed to the host.  Else, the request is
#       submitted to disk in the normal manner.
#
#  INPUT:
#       g0 = VRP for this request
#       g1 = ILT for this write I/O
#
#  OUTPUT:
#       g12 = Return Status
#               TRUE  = Op was handled (cacheing of data begun)
#               FALSE = Op was not handled (due to resources or other states)
#
#  REGS DESTROYED:
#       g0-g7
#
#******************************************************************************
#
wc$process_io:
        mov     g0,r15                  # Preserve g0
        mov     g1,r14                  #  and g1
        ldos    vr_vid(r15),r6          # Get VID
        ld      vcdIndex[r6*4],r7       # Retrieve VCD pointer for this VID
#
# --- Check to see if a host op slipped in before a disable cache occurred
#
        ldob    vc_stat(r7),r3          # r3 = Cache Status
        and     VC_NO_MORE_DATA,r3,r3   # Check to see if more data is allowed
        cmpobne 0,r3,.wcpio90           # Jif if no more data is allowed in
                                        #  cache due to disable or error state
        lda     C_ca,r3                 # Get the Global Cache Pointer
        ldob    ca_status(r3),r3        # r3 = Global Cache Status
        ldconst CA_NO_MORE_DATA,r4      # r4 = Status where no more data is
                                        #  allowed into the Write Cache
        and     r4,r3,r3                # r3 = See if no more Write Cache
        cmpobne 0,r3,.wcpio90           # Jif if the Global Cache is in a state
                                        #  to not let more data in
#
# --- Check if write caching is allowed based on the size of the request.
#     Any write request that is greater than  WCACHEMAX will always
#     bypass the write cache, independent of the current queue depth.
#
        ld      vr_vlen(g0),r3          # Get write request size
        ldconst WCACHEMAX,r4
        cmpobg  r3,r4,.wcpio02          # Jif size > WCACHEMAX, bypass write cache
#
# --- The sizeof the request is less than the maximum size. Examine the qdepth
#     at the cache layer. If the qdepth is low, let the command through the
#     write cache. If the qdepth is "large", then look at the performance
#     bypass flag, to determine if the cache should be bypassed. This flag is
#     set periodically (once a second), based on the workload through the
#     controller (HBA qdepth and HBA MB/S).
#
        lda     C_ca,r4                 # r4 = Cache Information pointer
        ldob    ca_status(r4),r5        # r5 = Cache Status
        bbc     ca_nwaymirror,r5,.wcpio03 # Jif it is 1-Way system
#
# --- Check the state of the bypass flag
#
        call    WC_CheckBypass
        cmpobe  0,g0,.wcpio03           # Jif if skipping bypass

.wcpio02:
#
# --- Bypass the write cache since the transfer size is too large or large
#     enough for a nominal queue depth.  Increment the Bypass Write too large
#     counter and then go process the op as a normal I/O.
#
        ldl     vc_wrtbylen(r7),r10     # Get the Bypass Write too large counter
        cmpo    1,0                     # Clear carry
        addc    1,r10,r10               # Add one to the Bypass Write too large
        addc    0,r11,r11               #   counter
        stl     r10,vc_wrtbylen(r7)     # Save the counter
        b       .wcpio90

.wcpio03:
#
# --- If the operation already has write buffers allocated (such as when
#       a Format Unit gets transformed to a write with the data pattern already
#       set up in the non-cached write buffers), handle like a non-cached write
#
        ld      vr_sglptr(r15),r4       # Get the SGL Pointer
c       if (r4 == 0xfeedf00d) {
c           fprintf(stderr,"%s%s:%u wc$process_io sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__);
c           abort();
c       }
        cmpobe  0,r4,.wcpio05           # Jif no SGL Pointer - normal op
        ldob    sg_flag(r4),r4          # Get flag byte
        bbs     sg_buffer_alloc,r4,.wcpio90 # Jif a buffer is already
                                        #    allocated - non-cached op
.wcpio05:
#
# --- Size is <= maximum allowable.  Attempt to assign a cache tag/buffer.
#
        call    wc$masg_ctag            # Get a tag
        cmpobne 0,g0,.wcpio06           # Jif tag available
        ldl     vc_wrtbyres(r7),r10     # Get the Bypass Write out of resources
        cmpo    1,0                     # Clear carry
        addc    1,r10,r10               # Add one to the Bypass Write because
        addc    0,r11,r11               #   out of resources counter
        stl     r10,vc_wrtbyres(r7)     # Save the counter
        b       .wcpio90
#
.wcpio06:
        mov     g0,r13                  # Preserve cache tag
#
# --- Attempt to assign a buffer.
#
        mov     r3,g0                   # g0 = size in sectors
        call    wc$masg_cbuffer         # Attempt to assign buffer
        cmpobne 0,g0,.wcpio08           # Jif buffer available
        ldl     vc_wrtbyres(r7),r10     # Get the Bypass Write out of resources
        cmpo    1,0                     # Clear carry
        addc    1,r10,r10               # Add one to the Bypass Write because
        addc    0,r11,r11               #   out of resources counter
        stl     r10,vc_wrtbyres(r7)     # Save the counter
        b       .wcpio80
#
.wcpio08:
        mov     g0,r12                  # Preserve buffer pointer
#
# --- Keep track of the number of writes to the cache in progress until
#       the Cache I/O tree is updated
#
        ld      vc_write_count(r7),r4   # Get the 0utstanding Write Counter
        addo    1,r4,r4                 # Increment the Outstanding Write Cntr
        st      r4,vc_write_count(r7)   # Save the Outstanding Write Counter
#
# --- Tag and buffer available.  Fill the tag/buffer with data.
#
        ldl     vr_vsda(r15),r4         # Get LBA start address
#
        stos    r6,tg_vid(r13)          # Set VID
        stl     r4,tg_vsda(r13)         # Set disk starting addr
        st      r3,tg_vlen(r13)         # Set length of buffer
        st      g0,tg_bufptr(r13)       # Set buffer pointer
        ldconst 0,r8                    # Initialize the Read Count to zero
        stos    r8,tg_rdcnt(r13)
        st      r13,ioctag(r14)         # Set cache tag address
        mov     g0,g2                   # Set buffer pointer
        mov     r14,g1                  # Restore the ILT pointer
        shlo    9,r3,g3                 # Set I/O length
        lda     .wcpio10,g0             # Set completion routine
        ld      il_misc(g1),r3          # Preserve ILT address
        ld      vrvrp(g1),r4            #  And VRP address
        lda     ILTBIAS(g1),g1          # Advance to next level
        st      r3,il_misc(g1)          # Store ILT address
        st      r4,vrvrp(g1)            #  and VRP
        ldconst TRUE,g12                # Show the op was handled!
c       record_cache_data(FR_CACHE_WRITE_CACHEABLE, (void *)r4, (void *)g2);
        b       C$getwdata              # Fill buffer with data/return
#
# ------------------------------------------------------------------------------
# --- Execution continues here when buffer filled
#
#       g0 = error code (zero = normal)
#       g1 = ILT
#
.wcpio10:
        lda     -ILTBIAS(g1),g1         # Restore nest level of ILT
        cmpobne 0,g0,.wcpio50           # Jif an error occurred getting the data
#
# --- setup cache tag for mirror operation.
#
        ld      ioctag(g1),g0           # Get cache tag address
.ifdef FLIGHTRECORDER
        ld      vrvrp(g1), r3
        ld      tg_bufptr(g0),r4        # Get buffer pointer for request
c       record_cache_data(FR_CACHE_WRITE_DATA, (void *)r3, (void *)r4);
.endif  # FLIGHTRECORDER
        setbit  TG_DIRTY,0,r3           # Set attribute to dirty
        lda     .wcpio20,r4             # Setup completion routine
        stos    r3,tg_attrib(g0)
        st      r4,il_cr(g1)
        b       wc$qm_buffer_and_tag    # Mirror buffer, then tag/return
#
# ------------------------------------------------------------------------------
# --- Execution continues here after mirror operation.
#
#       g0 = error code
#       g1 = ILT
#
.wcpio20:
#
# --- Keep track of the number of writes to the cache in progress until
#       the Cache I/O tree is updated
#
        ld      vrvrp(g1),r7            # Get the VRP for this ILT
        ldos    vr_vid(r7),r7           # Get the VID number
        ld      vcdIndex[r7*4],r7       # Retrieve VCD pointer for this VID
        ld      vc_write_count(r7),r4   # Get the 0utstanding Write Counter
        subo    1,r4,r4                 # Decrement the Outstanding Write Cntr
        st      r4,vc_write_count(r7)   # Save the Outstanding Write Counter
#
# --- Determine if the mirror completed successfully.  If so, continue, else
#       send the data to the disk using the Write Cache buffer.
#
        cmpobe  0,g0,.wcpio30           # Jif the Mirror was successful
        mov     g2,r4                   # Save g2
        lda     wc$ctnc_wm_comp,g2      # g2 = Completion routine for write miss
        call    wc$convert_ctnc_op      # Go handle as non-cached write op
        mov     r4,g2                   # Restore g2
        b       .wcpio95                # return
#
# --- Need to allocate an RB node for this tag and put it into the cache tag
#     tree.  Also need to assign another RB node to place it into the dirty
#     tree.
#
.wcpio30:
        ld      ioctag(g1),g0           # Get cache tag address
        call    wc$insert_io_node       # Put into RB tag tree
        call    wc$insert_dirty_node    # Put into dirty tree
#
# --- Increment the number of dirty blocks and Decrement the Free Blocks
#
        ld      tg_vlen(g0),g0          # g0 = Number of blocks for this op
        call    wc$IncDirtyCount        # Increment the Dirty Counts
        call    wc$DecFreeCount         # Decrement the Free Counts
#
# --- Show the op completed successfully
#
        ldconst 0,g0                    # Put a good return in g0
        b       wc$complete_io          # Complete the I/O operation
                                        #  to the host
#
# ------------------------------------------------------------------------------
# --- Error occurred during the transfer of data from the host ----------------
#
#     Free the Cache Buffer, Tag, and complete the I/O command with the error
#
.wcpio50:
        movq    g0,r12                  # r12 = Error Code
                                        # r13 = ILT
        ld      ioctag(g1),r11          # r11 = cache tag address
        ld      tg_bufptr(r11),g0       # g0 = Buffer address
        ld      tg_vlen(r11),g1         # g1 = Buffer size
        call    wc$mrel_cbuffer         # Release the Cache buffer
        mov     r11,g0                  # g0 = Tag address
        call    wc$mrel_ctag            # Release the Tag
        ld      vrvrp(r13),r10          # r10 = VRP
        stob    r12,vr_status(r10)      # Save the Completion Code
#
# --- Keep track of the number of writes to the cache in progress until
#       the Cache I/O tree is updated
#
        ldos    vr_vid(r10),r7          # Get the VID number
        ld      vcdIndex[r7*4],r7       # Retrieve VCD pointer for this VID
        ld      vc_write_count(r7),r4   # Get the 0utstanding Write Counter
        subo    1,r4,r4                 # Decrement the Outstanding Write Cntr
        st      r4,vc_write_count(r7)   # Save the Outstanding Write Counter
        movq    r12,g0                  # Restore saved registers
        b       wc$complete_io          # Report and complete the error
#
# ------------------------------------------------------------------------------
# --- Process I/O as "normal" request -----------------------------------------
#
.wcpio80:
#
# --- Return cache tag to freelist; could not get buffer.
#
        mov     r13,g0                  # g0 = cache tag address
        call    wc$mrel_ctag            # Place on freelist
#
.wcpio90:
#
# --- Process I/O as "normal" I/O: directly to disk.
#
        mov     r14,g1                  # g1 = ILT for this I/O
        mov     FALSE,g12               # notify caller - unable to handle
.wcpio95:
        ret
#
#******************************************************************************
#
#  NAME: wc$convert_ctnc_op
#
#  PURPOSE:
#       Converts a Write Cache op to a non-cached write op.
#
#  DESCRIPTION:
#       This routine converts a Write Cache op (one that has a Write Cache
#       tag and buffer) into a non-cached write op.  It will set up the VRP
#       to use the Write Cache buffers and then forward this op to the lower
#       layers to do the write to disk.
#
#  INPUT:
#       g1 = ILT of write cache op
#       g2 = Completion routine to call
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
wc$convert_ctnc_op:
#
# --- Create the SGL list for this op using the Write Cache buffer as the
#       source of the data.
#
        ld      ioctag(g1),r14          # r14 = The cache tag address
        ld      vrvrp(g1),r15           # r15 = The VRP for this ILT
        ld      tg_bufptr(r14),r13      # r13 = Buffer pointer
        ld      tg_vlen(r14),r12        # r12 = Buffer length in blocks
#
        lda     vr_sglhdr(r15),r11      # r11 = The SGL address in the VRP
        mov     1,r4                    # r4 = Set up descriptor count
        lda     sghdrsiz+sgdescsiz,r5   # r5 = Set up SGL size
        mov     r13,r6                  # Translate to global address
        shlo    9,r12,r7                # r7 = byte count
        st      r5,vr_sglsize(r15)      # Set size of SGL
        st      r11,vr_sglptr(r15)      # Link SGL to VRP
        stq     r4,sg_scnt(r11)         # Store the SGL just created in the VRP
#
# --- Set up the ILT for completing the op and then send to the next layer to
#       complete the operation.
#
                                        # g1 = ILT
                                        # g2 = Completion routine to call
        call    c$calllower             # Go issue the op to the lower layer
#
# --- All Done
#
        ret
#
#**********************************************************************
#
#  NAME: wc$Wsubmit_restart
#
#  PURPOSE:
#       Restart the operation that was queued due to an error state condition.
#
#  DESCRIPTION:
#       This routine restarts the operation that was queued due to an error
#       state condition.  It calls the original routine, WC$Wsubmit, and if
#       the op was not handled, turn it into a non-cached operation.
#
#  INPUT:
#       g1 = ILT associated with write
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
wc$Wsubmit_restart:
        movq    g0,r8                   # Save g0-g3
        movq    g4,r12                  # Save g4-g7
        mov     g12,r7                  # Save g12
#
# --- Re-issue the operation to WC$Wsubmit
#
        call    WC$Wsubmit              # Restart the process
#
# --- Determine if the operation was handled.  If not, turn the operation into
#       a non-cached op.  If it was handled, return.
#
        cmpobe  TRUE,g12,.wcwsubmit_restart100 # Jif the op was handled
        mov     r9,g1                   # Restore the ILT
        call    C$do_nc_op              # Turn into a non-cached write operation
#
# --- All Done
#
.wcwsubmit_restart100:
        movq    r8,g0                   # Restore g0-g3
        movq    r12,g4                  # Restore g4-g7
        mov     r7,g12                  # Restore g12
        ret
#
#******************************************************************************
#
#  NAME: wc$insert_io_node
#
#  PURPOSE:
#       To allocate a RB tree tag and add a cache tag to the tree.
#
#  DESCRIPTION:
#       This routine is called with the address of a particular cache tag.
#       It then allocates an RB node, fills in the appropriate information,
#       and inserts the node into the cache tag tree for the VID.  The tag
#       is then updated to reflect the address of the node (<tg_ioptr> field).
#
#  INPUT:
#       g0 = Cache tag address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
wc$insert_io_node:
        mov     g0,r15                  # Preserve g0
        mov     g1,r14                  #  and g1
!       ldos    tg_vid(g0),g0           # Get VID for node
c       g1 = get_wc_rbnode();           # Assign a RB node
#
!       st      g1,tg_ioptr(r15)        # Save node location in tag
#
# --- Fill in RB node parameters.
#
!       ldl     tg_vsda(r15),r4         # Get starting disk address
!       ld      tg_vlen(r15),r6         # Get length of request
        stl     r4,rbkey(g1)            # Set key
        cmpdeco 0,r6,r6                 # Adjust length of request
        addc    r6,r4,r6
        addc    0,r5,r7                 # Generate endpoint for req
        stl     r6,rbkeym(g1)
        st      r15,rbdpoint(g1)        # Set location of cache tag into node
#
# --- Get root for this tree and insert node.
#
        ld      vcdIndex[g0*4],r12      # Retrieve VCD pointer for this VID
        ld      vc_cache(r12),g0        # g0 = root of resident tree
        call    RB$insert               # Insert node into tree
        st      g0,vc_cache(r12)        # Save (possibly) modified root
#
# --- Done with insertion.
#
        mov     r14,g1                  # Restore g1
        mov     r15,g0                  #  and g0
        ret
#
#******************************************************************************
#
#  NAME: wc$insert_dirty_node
#
#  PURPOSE:
#       To allocate a RB tree tag and add a cache tag to the dirty tree.
#
#  DESCRIPTION:
#       This routine is called with the address of a particular cache tag.
#       It then allocates an RB node, fills in the appropriate information,
#       and inserts the node into the dirty tag tree for the VID.  The tag
#       is then updated to reflect the address of the node (<tg_dirtyptr> field).
#
#  INPUT:
#       g0 = Cache tag address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
wc$insert_dirty_node:
        mov     g0,r15                  # Preserve g0
        mov     g1,r14                  #  and g1
c       g1 = get_wc_rbnode();           # Assign a RB node
#
!       st      g1,tg_dirtyptr(r15)     # Save node location in tag
#
# --- Fill in RB node parameters.
#
!       ldl     tg_vsda(r15),r4         # Get starting disk address
!       ld      tg_vlen(r15),r6         # Get length of request
        mov     r6,g0                   # Save the request length
        stl     r4,rbkey(g1)            # Set key
        cmpdeco 0,r6,r6                 # Adjust length of request
        addc    r6,r4,r6
        addc    0,r5,r7                 # Generate endpoint for req
        stl     r6,rbkeym(g1)
        st      r15,rbdpoint(g1)        # Set location of cache tag into node
#
# --- Get root for this tree and insert node.
#
!       ldos    tg_vid(r15),g0          # Get VID for node
        ld      vcdIndex[g0*4],r12      # Retrieve VCD pointer for this VID
        ld      vc_dirty(r12),g0        # g0 = root of dirty cache tag tree
        call    RB$insert               # Insert node into tree
        st      g0,vc_dirty(r12)        # Save (possibly) modified root
#
# --- Done with insertion.
#
        mov     r14,g1                  # Restore g1
        mov     r15,g0                  #  and g0
        ret
#
#******************************************************************************
#
#  NAME: wc$qm_tag / wc$qm_buffer_and_tag
#
#  PURPOSE:
#       To queue an ILT/Placeholder to the mirror process with a command
#       to perform the requested mirror operation.
#
#  DESCRIPTION:
#       This routine is called with the address of an ILT/Placeholder and
#       a cache tag.  The ILT is queued to the mirror executive (in a N=1
#       environment) or DRP executive (in an N=2 or more environment) with
#       command bits telling it to perform the requested mirror operation.
#       After the mirror operation is completed the completion routine in
#       the ILT/placeholder is called.
#
#       This routine returns immediately after queueing the ILT.
#
#       The following entry points exist:
#
#       wc$qm_tag                       Queue to mirror tag only
#       wc$qm_buffer_and_tag            Queue to mirror buffer first, then tag.
#
#  INPUT:
#       g0 = Cache tag address
#       g1 = Placeholder for this cache tag
#
#       <plmctag> and <plmcmd> in the ILT/placeholder are altered by this routine.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
wc$qm_tag:
#
# --- Only mirror tag if the tag is not a BE tag
#
!       ldos    tg_attrib(g0),r14       # Get the Attribute
        setbit  MIRROR_TAG,0,r3         # Mirror tag only
        lda     gMirrorQueue,r4         # Use Regular Mirror Routine
        bbc     TG_BE,r14,.wcqm10       # Jif this is not a BE Tag
#
#   BE Tag that is being requested to be mirrored.  Copy the data to the BE MMC.
#
        lda     gWCMirrorBETagQueue,r4  # Use the BE MMC Mirror Routine
        b       .wcqm10
#
wc$qm_buffer_and_tag:
#
# --- Set both mirror tag and mirror buffer bits
#
        ldconst ( (1 << MIRROR_TAG) | (1 << MIRROR_BUFFER) ),r3
        lda     gMirrorQueue,r4         # Use Regular Mirror Routine
#
.wcqm10:
        mov     g0,r14                  # r14 = Cache Tag Address
        mov     g1,r15                  # r15 = placeholder ILT
        st      g0,plctag(g1)           # Set cache tag
        st      r3,plmcmd(g1)           # Set command bits
        mov     r4,g0                   # g0 = queue control block
                                        # g1 = ILT
        call    wc$q                    # Queue request/return
        mov     r14,g0                  # Restore g0-g1
        mov     r15,g1
        ret
#
#******************************************************************************
#
#  NAME: wc$qtag_flush
#
#  PURPOSE:
#       To queue an ILT/Placeholder to the flush process.
#
#  DESCRIPTION:
#       This routine is called with the address of an ILT/Placeholder and
#       a cache tag.  The ILT is queued to the mirror executive.  After the
#       flush operation is completed the completion routine in the ILT/placeholder
#       is called.
#
#       This routine returns immediately after queueing the ILT.
#
#  INPUT:
#       g0 = Cache tag address
#       g1 = Placeholder for this cache tag
#
#       <plmctag>  in the ILT/placeholder is altered by this routine.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Possibly all G registers; completion routine in ILT may be
#       called.
#
#******************************************************************************
#
        .globl  wc$qtag_flush
wc$qtag_flush:
        st      g0,plctag(g1)           # Set cache tag
        lda     c_wflushq,g0            # g0 = queue control block
        b       wc$q                    # Queue request/return
#
#******************************************************************************
#
#  NAME: wc$wcache_hit
#
#  PURPOSE:
#       To process an I/O that exactly hits an already existing cache
#       segment.
#
#  DESCRIPTION:
#       This routine determines the course of action that must be taken
#       to handle an exact cache hit for an incoming write request.
#       If the cache segment is resident (already flushed) the tag/buffer
#       is reused and the buffer marked dirty.  If the buffer is dirty,
#       the following actions are taken:
#         1) A new buffer is allocated and filled with the incoming data.
#         2) The new buffer is mirrored.
#         3) The tag for the old buffer is changed to point to the new buffer.
#         4) The tag is mirrored.
#         5) The old buffer is deallocated.
#         6) The I/O is completed to the host.
#
#       If at step 1 insufficient memory exists to allocate the new buffer,
#       the old cache segment is flushed and invalidated.  Then, the
#       buffer is reused for the incoming I/O exactly in the fashion
#       of a cache fill.
#
#       If the given tag is locked, the I/O becomes blocked until the tag
#       is unlocked.  Once unblocked, operation proceeds as normal with
#       one exception: the tag is first checked for validity.  It is possible
#       that the tag became invalidated by another I/O while the current
#       I/O was blocked.  If the tag is now invalid the I/O is submitted
#       to disk, bypassing the write cache.
#
#       All of the above functions do not execute in the caller's task context.
#       Instead, certain steps require queueing of "placeholder" ILTs that
#       when the operation completes, executes a particular completion routine
#       that proceeds to the next step.  As a result the operations may
#       be completed in several task contexts and the caller of this routine
#       will not be blocked under any circumstances.
#
#       Note:  This routine depends upon precedence checking of concurrent I/O
#              requests against the cache.  In particular, overlapping writes
#              must not be allowed.  (See <WC$coverlap>)
#
#  INPUT:
#       g0 = Cache tag hit by incoming I/O
#       g1 = I/O ILT for incoming I/O
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g2-g7
#
#******************************************************************************
#
wc$wcache_hit:
        mov     g0,r15                  # Preserve cache tag address
        mov     g1,r14                  # Preserve I/O ILT address
#
# --- Determine state of cache tag.
#
#       Check for tag locked condition. The pending I/O will be blocked if the
#       tag is locked for any reason (read, write, fill, flush, invalidate).
#
#       Note:  Since this routine depends upon precedence checking of
#              all I/Os, overlapping write cache hits should not occur
#              and are not detected.  Corrupted data could result
#              if concurrent write cache hits are allowed to the same
#              tag.
#
        ldos    tg_attrib(r15),r3       # Get tag attribute bits
        ldos    tg_state(r15),r4        # Get tag state bits
        cmpobe  0,r4,.wcwc20            # Jif tag is not locked
#
# --- Tag is locked; queue placeholder to wait for tag unlock, then
#     continue processing.
#
c       g1 = get_wc_plholder();         # Get a Placeholder.
        stl     r14,plilt(g1)           # Save ILT address
                                        #  and cache tag
        lda     .wcwc10,r4              # Setup completion routine
        setbit  WRITE_REQUEST,0,r5      # Set intent to write
        st      r4,il_cr(g1)            # Store completion routine
        st      r5,plintent(g1)         # Store intent
        mov     r15,g0                  # g0 = tag address
                                        # g1 = placeholder address
        b       wc$qtag_unlock          # Queue/wait for unlock/return
#
# --- Control returns here in different context when tag unlocked
#
#     g1 = placeholder address
#
.wcwc10:
#
        ldl     plilt(g1),r14           # Restore ILT/cache tag address
c       put_wc_plholder(g1);            # Release the placeholder ILT
#
        ldos    tg_attrib(r15),r3       # Get tag attribute bits
#
.wcwc20:
        mov     r15,g0
        mov     r14,g1
#
# --- Cache tag is not locked.  Check attribute of tag.
#     Note:  This routine could be called after a tag unlock.  Therefore,
#            its validity must always be checked.
#
        bbs     TG_FREE,r3,.wcwc30      # Jif tag is no longer valid
                                        #  Submit I/O uncached
        bbs     TG_FREE_PENDING,r3,.wcwc30 # Jif tag will be free soon
#
# --- Check for dirty status
#
        bbs     TG_DIRTY,r3,.wcwc40     # Jif tag is marked dirty
#
# --- Tag is resident and unlocked.  Generate I/O for the associated buffer.
#
#       r15/g0 = cache tag address
#       r14/g1 = I/O ILT address
#
        b       wc$resident_write       # Generate cache I/O; return
#
.wcwc30:
#
# --- Cache tag was invalid
#
        b       C$do_nc_op              # Submit I/O w/o cache
#
.wcwc40:
#
# --- Tag marked dirty; perform dirty-to-dirty I/O.  If a new buffer cannot
#     be allocated, flush and then attempt resident I/O.
#
#       r15/g0 = cache tag address
#       r14/g1 = I/O ILT address
#
        ld      tg_vlen(r15),g0         # Get sizeof current buffer
        call    wc$masg_cbuffer         # Attempt assignment of cache buffer
        mov     r14,g1                  # Restore ILT address
        cmpobne 0,g0,.wcwc45            # Jif buffer assignment completed
#
# --- Could not assign buffer or the tag is for a BE tag.  Queue for flush
#       and perform resident I/O.
#
        ldconst .wcwc42,r3              # Set completion routine address
        st      r3,il_cr(r14)           #   and store
        mov     r15,g0                  # Restore the correct Cache Tag Pointer
        b       wc$qtag_flush           # Queue this tag for flush/return
#
# --- Execution continues here in another context after completion of flush
#
#     ASSUMPTION: Tag and I/O are still set to the same LBA and size; this should
#                 be true because of the front-end precedence checking.
#                 However, the tag could have been invalidated, which the
#                 beginning of this routine checks.
#
#     g0 = Error code
#     g1 = ILT address
#
.wcwc42:
        cmpobne 0,g0,.wcwc60            # Jif there was an error on the flush
        ld      ioctag(g1),g0           # Get cache tag address
        b       wc$wcache_hit           # Try I/O again
#
.wcwc45:
        mov     g0,g2                   # g2 = new buffer address
        mov     r15,g0                  # Restore tag address
#
        b       wc$dirty_io             # Submit dirty-to-dirty I/O/return
#
# --- An error occurred during the Flush of the data to disk.  The data has
#       already been set back to its proper state and the only item left to
#       do is to report the error up the ILT chain.
#
.wcwc60:
        b       wc$complete_io          # g0 and g1 are already set up.
#
#******************************************************************************
#
#  NAME: wc$resident_write
#
#  PURPOSE:
#       To perform a write to a resident cache buffer.
#
#  DESCRIPTION:
#       This routine is called when a incoming write I/O exactly "hits" an
#       existing cache segment.
#
#       The following assumptions are made:
#         1) The I/O and cache tag match on the LBA start address and the
#            size of the I/O.
#         2) The cache tag is unlocked.
#         3) The cache tag is not marked dirty.
#
#       This routine performs the following functions:
#         1) The tag is locked.
#         2) The buffer is overwritten with the new data.
#         3) The tag is marked dirty, then buffer/tag mirrored.
#         4) The tag is unlocked.
#         5) If all the above steps completed normally, then the I/O
#            is completed to the host.
#
#       This process results in the cache tag being removed from the
#       LRU queue.
#
#  INPUT:
#       g0 = Cache tag for the "hit"
#       g1 = ILT for this I/O
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Possibly all G registers.
#
#******************************************************************************
#
wc$resident_write:
        call    wc$remove_lru_queue     # Remove cache tag from LRU queue
#
# --- Lock the tag.
#
        ldos    tg_attrib(g0),r3        # Get tag attributes
        ldos    tg_state(g0),r4         # Get tag state
        cmpobne 0,r4,.wcrw30            # Jif tag locked (ERROR!!)
#
        setbit  TG_WRITEIP,r3,r3        # Set write in progress
        setbit  TG_LOCKED_WRITE,r4,r4   # Set tag locked for write
        stos    r3,tg_attrib(g0)        # Store tag attributes
        stos    r4,tg_state(g0)         # Store tag state
#
# --- Tag locked and set to write-in-progress.  Now initiate buffer fill.
#
        ld      tg_vlen(g0),g3          # Get length of request
        ld      tg_bufptr(g0),g2        # Get buffer pointer for request
#
        st      g0,ioctag(g1)           # Store cache tag address
        lda     .wcrw10,g0              # Completion routine for fill
        shlo    9,g3,g3                 # Set I/O length in bytes
        ld      il_misc(g1),r3          # Preserve ILT address
        ld      vrvrp(g1),r4            #  And VRP address
        lda     ILTBIAS(g1),g1          # Advance to next level
        st      r3,il_misc(g1)          # Store ILT address
        st      r4,vrvrp(g1)            #  and VRP
c       record_cache_data(FR_CACHE_WRITE_HIT_RESIDENT, (void *)r4, (void *)g2);
        b       C$getwdata              # Initiate buffer fill/return
#
# --- Execution continues here in another context after buffer fill
#
#     g0 = Error code (0 = no error)
#     g1 = ILT address
#
#     Leave tag locked, clear write-in-progress and set mirror-in-progress;
#     then queue to mirror process.
#
.wcrw10:
        lda     -ILTBIAS(g1),g1         # Restore nest level of ILT
        cmpobne 0,g0,.wcrw50            # Jif an error occurred getting the data
        ld      ioctag(g1),g0           # Get cache tag address
.ifdef FLIGHTRECORDER
        ld      vrvrp(g1), r3           # reload VRP
        ld      tg_bufptr(g0),r5        # Get buffer pointer for request
c       record_cache_data(FR_CACHE_WRITE_RESIDENT_DATA, (void *)r3, (void *)r5);
.endif  # FLIGHTRECORDER
        ldos    tg_attrib(g0),r3        # Get tag attributes
        lda     .wcrw20,r5              # Setup completion routine for next
                                        #  step
        clrbit  TG_WRITEIP,r3,r3        # Clear write in progress
        setbit  TG_DIRTY,r3,r3          # Set dirty buffer
        clrbit  TG_RESIDENT,r3,r3       # Clear the Resident Flag
#
        st      r5,il_cr(g1)            # Store completion routine
        stos    r3,tg_attrib(g0)        # Store new tag attributes
        call    wc$insert_dirty_node    # Insert into dirty tree
#
# --- Increment the Dirty Counters and Decrement the Resident Counters
#
        mov     g0,r4                   # Save the tag
        ld      tg_vlen(g0),g0          # g0 = Number of blocks for this op
        call    wc$IncDirtyCount        # Increment the Dirty Counters
        call    wc$DecResidentCount     # Decrement the Resident Counters
        mov     r4,g0                   # Restore the Tag
        b       wc$qm_buffer_and_tag    # Queue to mirror buffer first, then
                                        #    tag/return
#
# --- Execution continues here after mirror of buffer/tag
#
#     g0 = Error code (0 = no error)
#     g1 = ILT address
#
# --- Unlock tag and complete I/O to host.
#
.wcrw20:
        cmpobe  0,g0,.wcrw23            # Jif the mirror completed OK
        mov     g2,r3                   # Save g2
        lda     wc$ctnc_rw_comp,g2      # g2 = Completion routine to call
                                        # g1 = ILT
        call    wc$convert_ctnc_op      # Go treat this as a non-cached write
                                        #  op that is using cache buffers
        mov     r3,g2                   # Restore g2
        ret                             # Return to caller
#
.wcrw23:
        mov     g1,r15                  # Preserve ILT address
        ld      ioctag(g1),g0           # Get cache tag address
        ldos    tg_state(g0),r3         # Get tag state bits
        clrbit  TG_LOCKED_WRITE,r3,r3   # Clear tag locked for write
        stos    r3,tg_state(g0)         # Store cache tag status
        call    wc$unlock_tag           # Perform tag unlock postprocessing
#
# --- Tag must not be touched after unlock.  Some other process could now
#     have taken control of it.
#
# --- Complete processing to host.  g0 = error code (0 = normal completion).
#
        ldconst 0,g0                    # Set normal completion
        mov     r15,g1                  # Restore ILT address
        b       wc$complete_io          # Complete the I/O to host/return
#
.wcrw30:
        cmpo    0,0                     # //// Generate error ////
        faulte
#
# --- Error occurred during the transfer of data from the host ----------------
#
#     Return the attributes and states back to what they were, Lock for and
#       Invalidate, Invalidate the tag, process any waiting for the tag,
#       free the cache buffer and tag, and respond to the request with the
#       error.
#
.wcrw50:
        ld      ioctag(g1),r11          # r11 = cache tag address
        movq    g0,r12                  # r12 = Error Code
                                        # r13 = ILT
#
# --- Unlock the tag, set the attributes and state, and free the tag.
#
        ldos    tg_attrib(r11),r3       # Get tag attributes
        ldos    tg_state(r11),r4        # Get tag state
#
        clrbit  TG_WRITEIP,r3,r3        # Remove write in progress
        clrbit  TG_LOCKED_WRITE,r4,r4   # Remove tag locked for write
        setbit  TG_LOCKED_INVALIDATE,r4,r4 # Flag it as locked for invalidate
#
        stos    r3,tg_attrib(r11)       # Store tag attributes
        stos    r4,tg_state(r11)        # Store tag state
        mov     r11,g0                  # g0 = Tag address
        call    wc$invalidate_tag       # Invalidate tag
        ldos    tg_state(r11),r4        # Load State bits
        clrbit  TG_LOCKED_INVALIDATE,r4,r4  # Unlock tag for Invalidate
        stos    r4,tg_state(r11)        # Store State bits
        call    wc$unlock_tag           # Tag unlock postprocessing & free tag
        ld      vrvrp(r13),r10          # r10 = VRP
        movq    r12,g0                  # Restore saved registers
        stob    r12,vr_status(r10)      # Save the Completion Code
        b       wc$complete_io          # Report and complete the error
#
#******************************************************************************
#
#  NAME: wc$dirty_io
#
#  PURPOSE:
#       To perform a write to a resident but marked dirty cache buffer.
#
#  DESCRIPTION:
#       This routine is called when a incoming write I/O exactly "hits" an
#       existing cache segment that is marked dirty.
#
#       The following assumptions are made:
#         1) The I/O and cache tag match on the LBA start address and the
#            size of the I/O.
#         2) The cache tag is unlocked.
#         3) The cache tag is marked dirty.
#         4) A cache buffer has been assigned prior to the calling of
#            this routine.
#
#       This routine performs the following functions:
#         1) The tag is locked.
#         2) The new buffer is written with the data.
#         3) The buffer is then mirrored.
#         4) The original buffer is deallocated.
#         5) The tag is then modified to point to the new buffer.
#         6) The tag is mirrored.
#         7) The tag is unlocked.
#         8) If all the above steps completed normally the I/O
#            is then completed to the host.
#
#  INPUT:
#       g0 = Cache tag for the "hit"
#       g1 = ILT for this I/O
#       g2 = New buffer for the data
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Possibly all G registers.
#
#******************************************************************************
#
wc$dirty_io:
#
# --- Lock the tag.
#
        ldos    tg_attrib(g0),r3        # Get tag attributes
        ldos    tg_state(g0),r4         # Get tag state
        cmpobne 0,r4,.wcdi30            # Jif tag locked (ERROR!!)
#
        setbit  TG_WRITEIP,r3,r3        # Set write in progress
        setbit  TG_LOCKED_WRITE,r4,r4   # Set tag locked
#
        stos    r3,tg_attrib(g0)        # Store tag attributes
        stos    r4,tg_state(g0)         # Store tag state
#
# --- Tag locked and set to write-in-progress.  Now initiate buffer fill.
#
        ld      tg_vlen(g0),g3          # Get length pointer for request
#
        st      g0,ioctag(g1)           # Store cache tag address
        st      g2,iobufptr(g1)         #  and buffer address
        ld      il_misc(g1),r3          # Preserve ILT address
        ld      vrvrp(g1),r4            #  And VRP address
        lda     ILTBIAS(g1),g1          # Advance to next level
        st      r3,il_misc(g1)          # Store ILT address
        st      r4,vrvrp(g1)            #  and VRP
        lda     .wcdi10,g0              # Completion routine for fill
        shlo    9,g3,g3                 # Set I/O length in bytes
c       record_cache_data(FR_CACHE_WRITE_HIT_DIRTY, (void *)r4, (void *)g2);
        b       C$getwdata              # Initiate buffer fill/return
#
# --- Execution continues here in another context after buffer fill
#
#     g0 = Error code (0 = no error)
#     g1 = ILT address
#
#     Leave tag locked, clear write-in-progress and set mirror-in-progress;
#     then queue to mirror process.
#
.wcdi10:
        lda     -ILTBIAS(g1),g1         # Move to last ILT level
        cmpobne 0,g0,.wcdi50            # Jif an error occurred getting the data
        ld      ioctag(g1),g0           # Get cache tag address
        ld      iobufptr(g1),r4         # Get new buffer address
.ifdef FLIGHTRECORDER
        ld      vrvrp(g1),r3            # reload VRP address
c       record_cache_data(FR_CACHE_WRITE_DIRTY_DATA, (void *)r3, (void *)r4);
.endif  # FLIGHTRECORDER
        ldos    tg_attrib(g0),r3        # Get tag attributes
        ld      tg_bufptr(g0),r6        # Get old buffer pointer
        lda     .wcdi20,r5              # Setup completion routine for next
                                        #  step
        ld      tg_vlen(g0),r7          # Get old buffer length
        clrbit  TG_WRITEIP,r3,r3        # Clear write in progress
        setbit  TG_DIRTY,r3,r3          # Set dirty buffer
#
        stos    r3,tg_attrib(g0)        # Store new tag attributes
        st      r5,il_cr(g1)            # Store completion routine
        st      r4,tg_bufptr(g0)        # Store new buffer pointer
        st      r6,iobufptr(g1)         #  and store old buffer pointer
        st      r7,iovlen(g1)           # Save old buffer length
#
        b       wc$qm_buffer_and_tag    # Queue to mirror new buffer
#
# --- Execution continues here after mirror of buffer/tag
#
#     g0 = Error code (0 = no error)
#     g1 = ILT address
#
.wcdi20:
#
#   Determine if the Mirror completed successfully.  If so, continue as normal.
#       Else, clean up what needs to be cleaned up and put on the error queue
#       waiting for the CCB to handle the error situation.
#
        cmpobe  0,g0,.wcdi23            # Jif if the mirror completed OK
#
        lda     wc$dirty_io_restart,r3  # Point to the routine to restart
        st      r3,il_cr(g1)            # Save the restart routine
        b       c$qerror                # Queue the ILT to the Error Task & ret
#
#   Mirror completed successfully.  Deallocate the old buffer, unlock the tag,
#       and complete the I/O to the host.
#
.wcdi23:
        mov     g1,r15                  # Preserve ILT address
        ld      iovlen(g1),g1           # Get sizeof buffer
        ld      iobufptr(r15),g0        # Get address of old buffer
        call    wc$mrel_cbuffer         # Deallocate old buffer
#
        mov     r15,g1
        ld      ioctag(g1),g0           # Get cache tag address
#
        ldos    tg_state(g0),r4         # Get tag state bits
        clrbit  TG_LOCKED_WRITE,r4,r4   # Clear tag locked for write
#
        stos    r4,tg_state(g0)         # Store cache tag status
        call    wc$unlock_tag           # Perform tag unlock postprocessing
#
# --- Tag must not be touched after unlock.  Some other process could now
#     have taken control of it.
#
# --- Complete processing to host.  g0 = error code (0 = normal completion).
#
        ldconst 0,g0                    # Set normal completion
        mov     r15,g1                  # Restore ILT address
        b       wc$complete_io          # Complete the I/O to host/return
#
.wcdi30:
        cmpo    0,0                     # //// Generate error ////
        faulte
#
# --- Error occurred during the transfer of data from the host ----------------
#
#     Free the newly allocated buffer, clear the write lock and pending flag,
#       unlock the tag, and respond to the request with the error.
#
.wcdi50:
        ld      ioctag(g1),r11          # r11 = cache tag address
        movl    g0,r12                  # r12 = Error Code
                                        # r13 = ILT
        ld      tg_vlen(r11),g1         # g1 = Buffer length
        ld      iobufptr(r13),g0        # g0 = address of new buffer buffer
        movl    g2,r14                  # Save registers
        call    wc$mrel_cbuffer         # Deallocate new buffer
#
# --- Unlock the tag, set the attributes and state, and unlock the tag.
#
        ldos    tg_attrib(r11),r3       # Get tag attributes
        ldos    tg_state(r11),r4        # Get tag state
#
        clrbit  TG_WRITEIP,r3,r3        # Remove write in progress
        clrbit  TG_LOCKED_WRITE,r4,r4   # Remove tag locked for write
#
        stos    r3,tg_attrib(r11)       # Store tag attributes
        stos    r4,tg_state(r11)        # Store tag state
        mov     r11,g0                  # g0 = Tag address
        call    wc$unlock_tag           # Tag unlock postprocessing
        ld      vrvrp(r13),r10          # r10 = VRP
        movq    r12,g0                  # Restore saved registers
        stob    r12,vr_status(r10)      # Save the Completion Code
        b       wc$complete_io          # Report and complete the error
#
#**********************************************************************
#
#  NAME: wc$dirty_io_restart
#
#  PURPOSE:
#       Restart the mirror operation that was queued due to an error
#       state condition.
#
#  DESCRIPTION:
#       This routine restarts the mirror operation that was queued due to an
#       error state condition.  It restores the ILT to the correct state
#       and reissues the op to wc$qm_buffer_and_tag.
#
#  INPUT:
#       g1 = ILT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
wc$dirty_io_restart:
        movl    g0,r14                  # Save g0-g1
#
# --- Set up and reissue the mirror operation
#
        lda     .wcdi20,r3              # r3 = Completion routine when done
        ld      ioctag(g1),g0           # g0 = The cache tag to process
        st      r3,il_cr(g1)            # Save the completion routine
        call    wc$qm_buffer_and_tag    # Restart the process
        movl    r14,g0                  # Restore g0-g2
        ret
#
#******************************************************************************
#
#  NAME: wc$complete_io
#
#  PURPOSE:
#       To complete an I/O back to the previous layer.
#
#  DESCRIPTION:
#       This routine is called when an I/O has completed.  This routine
#       then rolls the ILT back to the previous nesting level and calls
#       the supplied completion routine.
#
#  INPUT:
#       g0 = Error code (0 = normal completion)
#       g1 = ILT for this I/O
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Possibly all G registers.
#
#*******************************************************************************
#
wc$complete_io:
#
# --- Decrement the Outstanding request count
#
        ld      C_orc,r3                # Load, Decrement, and Store the
        subo    1,r3,r3                 #  outstanding request count
.ifdef M4_DEBUG_C_orc
c CT_history_printf("%s%s:%u: C_orc starts at %lu, ends at %lu\n", FEBEMESSAGE,__FILE__, __LINE__, C_orc, r3);
.endif  # M4_DEBUG_C_orc
        st      r3,C_orc
#
        lda     -ILTBIAS(g1),g1         # Back up to previous nesting level
        ld      il_cr(g1),r6            # Get completion routine
        bx      (r6)                    # Call completion routine/return
#
#******************************************************************************
#
#  NAME: WC$coverlap
#
#  PURPOSE:
#       To prevent submission of write and read I/O requests to a
#       particular Virtual ID if the requests overlap.
#
#  DESCRIPTION:
#       This routine is passed I/Os directed at a particular
#       Virtual ID (VID).  It blocks requests that overlap previously
#       submitted requests where the result could be corrupted data.
#
#       These overlaps are not permitted:
#         1) Write to disk address when a previous submitted write is pending.
#         2) Write to disk address when a previous submitted read is pending.
#         3) Read to disk address when a previous submitted write is pending.
#
#       The blocked I/Os remain blocked until pending conflicting I/O is done.
#
#  ASSUMPTIONS:
#       Only valid I/O commands will be received by this function
#
#  INPUT:
#       g0 = VID
#       g1 = ILT for this I/O
#
#       ILT:
#           vrvrp  =  VRP
#
#  OUTPUT:
#       The VID Interval Tree Node Pointer may change
#
#  REGS DESTROYED:
#       g2-g7
#
#  NOTE:
#       Minimum memory requirements for this routine are:
#       (Max QLogic queue depth) * (Number of QLogic chips) * (placeholder size)
#
# //// TO DO:
#       Assign <iltdefs> equates for ILT variables used in this routine,
#               currently:
#               il_misc:  Address of original ILT (only stored in ILT )
#               il_w1:  Current number of I/Os that have blocked the I/O
#                       referred to by this ILT/VRP
#               il_w5:  Pointer to the tree node for this pending I/O
# ////
#
#******************************************************************************
#
WC$coverlap:
        mov     g0,r15                  # Save the VID
        mov     g1,r14                  # Preserve ILT pointer
#
        ld      vcdIndex[g0*4],r9       # Retrieve VCD pointer for this VID
        ld      vc_io(r9),r11           # Get the Pending IO Tree Pointer Root
        mov     0,g6                    # Initialize blocked count
#
# --- Input ILT assumed to have <vrvrp> pointing to the VRP associated
#     with the I/O request.
#
        ld      vrvrp(r14),r13          # Get VRP pointer
#
# --- Check I/O type; must be read or write request
#
        ldos    vr_func(r13),r3         # Get function code
        st      g6,il_w1(r14)           # Set blocked I/O counter
#
# --- Retrieve LBA and size of I/O request
#
        ldl     vr_vsda(r13),g2         # Get starting disk address
        ld      vr_vlen(r13),r4         # Get length
#
# --- Determine ending address for interval search
#
        cmpdeco 0,r4,r4                 # Adjust endpoint of request
                                        #  and clear carry bit
#
# --- Check for a non-zero length.  If this is a synchronize cache
#     request, a zero length specifies a range from the starting
#     disk address to the end of the drive.
#
        bne     .wcc10                  # Jif length != 0
        cmpobne vrsync,r3,.wcc05        # Jif not Sync Cache

        ldconst 0xffffffff,g4           # Set the end range of request
        ldconst 0xffffffff,g5           #  to the maximum possible value
        b       .wcc15
#
.wcc05:
        faultne                         # Command (other than Sync Cache) with
                                        #  zero length - should have been
                                        #  handled earlier
.wcc10:
        addc    r4,g2,g4                # Add length-1 to LSB
        addc    0,g3,g5                 # Add remainder to MSB
.wcc15:
        cmpobe  0,r11,.wcc120           # Jif no root
        mov     r11,g0                  # Set up the root pointer
        call    RBI$foverlap            # Check for overlap
        cmpobe  FALSE,g1,.wcc120        # Jif no overlap
#
.wcc20:
#
# --- Locate overlapped I/O if node is chain of tree nodes.
#
        mov     g1,r12                  # Preserve node pointer
        ld      rbfthd(g1),r8           # Get forward thread pointer
        cmpobe  0,r8,.wcc60             # Jif no forward thread exists; this
                                        #  node must overlap
#
# --- This I/O is first in chain of duplicates.  Therefore, we must reconstruct
#     the <rbkeym> field.
#
        ld      rbdpoint(g1),r4         # Get ILT pointer
        ldl     rbkey(g1),r6            # Get starting LBA
        ld      vrvrp(r4),r4            # Get pointer to VRP
        ld      vr_vlen(r4),r4          # Get length of request
        cmpdeco 0,r4,r4                 # Adjust size and clear carry bit
        bne     .wcc25                  # Jif original length is not zero
        ldconst 0xffffffff,r6           # Sync Cache command - set the end
        ldconst 0xffffffff,r7           #  range of request to the maximum
        b       .wcc30
#
.wcc25:
        addc    r4,r6,r6                # Generate
        addc    0,r7,r7                 #   end of request
#
# --- Loop until overlapped I/O found in chain of tree nodes.
#
.wcc30:
#
# --- Check ending LBA >= min key
#
        cmpobne r7,g3,.wcc40            # Jif MSBs differ
        cmpo    r6,g2                   # Check LSBs
#
.wcc40:
        bge     .wcc60                  # Jif overlap
#
# --- This node does not overlap.  Check next.
#
.wcc50:
        cmpobe  0,r8,.wcc110            # Jif end of nodes in chain
        mov     r8,g1                   # Move to next node in thread
        ldl     rbkeym(r8),r6           # Get ending LBA key for node
        ld      rbfthd(r8),r8           # Get next pointer
        b       .wcc30                  # Check next node
#
.wcc60:
#
# --- This request overlaps a previous request.  If current I/O is a write,
#     block this request.
#
#       r3 = function code for I/O request
#
        ld      rbdpoint(g1),r10        # get ILT pointer
        bbs     1,r3,.wcc80             # Jif write request; block
#
# --- I/O is a read; check for write in overlapped command.
#
        ld      vrvrp(r10),r5           # get VRP pointer
        ldos    vr_func(r5),r5          # Get function code
        bbc     1,r5,.wcc50             # Jif not write; check more overlaps
#
.wcc80:
#
# --- Block this I/O
#
        cmpinco 1,g6,g6                 # Increment blocked I/O counter
        st      g6,il_w1(r14)           # Save blocked I/O counter
#
# --- Allocate "placeholder" for this I/O.
#
c       g1 = get_wc_plholder();         # Get Placeholder ILT.
#
# --- Setup completion routine for the placeholder
#
        lda     wc$ph_compl,r4          # Placeholder completion routine
        st      r14,il_misc(g1)         # Store ILT address
        st      r4,il_cr(g1)            # Store completion routine
#
# --- Link this block to end of chain.
#
# --- <il_bthd> of first ILT in chain points to tail of chain.  Use this
#               to place the new ILT or placeholder at the end.
#               If no other ILTs on the chain this and forward thread
#               pointers are zero.
#
#       r10 = First ILT in chain to link to
#       g1  = placeholder
#
        ldl     il_fthd(r10),r4         # Get forward/backward (tail) thread
        st      g1,il_bthd(r10)         # Set tail pointer
        cmpo    0,r4                    # Check for empty list
        mov     r10,r4                  # Setup for long store
        sele    r5,r10,r5               # Select correct ILT to link to
#
        stl     r4,il_fthd(g1)          # Set forward/back pointer in placeholder
        st      g1,il_fthd(r5)          # Set forward pointer in last ILT on list
#
        cmpobne 0,r8,.wcc50             # Jif more nodes in chain to check
#
.wcc110:
#
# --- Check for next overlap.
#
        mov     r12,g1                  # Setup last block overlapped
        call    RBI$noverlap
        cmpobne FALSE,g1,.wcc20         # Loop for next overlapped I/O
#
# --- Done with overlap checking/blocking
#
.wcc120:
#
# --- Insert this node into interval tree.
#
c       g1 = get_wc_rbinode();          # Allocate tree element w/wait
#
        stl     g2,rbkey(g1)            # Set key value
        stl     g4,rbkeym(g1)           # Set key max value
        st      r14,rbdpoint(g1)        # Set data payload (ILT) pointer
        st      g1,il_w5(r14)           # Save tree node address for I/O
        mov     0,r3
        st      r3,il_fthd(r14)         # Clear forward
        st      r3,il_bthd(r14)         #  and backward pointers
#
        mov     r11,g0                  # Get root element
        call    RBI$insert              # Insert element into tree
        st      g0,vc_io(r9)            # Store the Interval tree root pointer
        cmpobne 0,g6,.wcc140            # Jif I/O blocked; do not submit.
#
# --- Submit this I/O; ILT pointer is in r14.
#
        ldconst wc$io_compl,g2          # Setup completion routine for I/O
c       wc_submit(r14, g2);             # Queue this request
#
.wcc140:
        mov     r15,g0                  # Restore the VID
        mov     r14,g1                  # Restore ILT pointer
        ret                             # Done!
#
#******************************************************************************
#
#  NAME: wc$ctnc_wm_comp
#
#  PURPOSE:
#       To complete the write operation that has Write Cache buffers and a tag
#       allocated that was a Write Miss.
#
#  DESCRIPTION:
#       This routine completes the write operation that has a Write Cache
#       buffer and a tag associated with it that was a write miss.  It will
#       free the buffer and tag and completes the operation.
#
#  INPUT:
#       g0 = return status
#       g1 = ILT for the operation
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
wc$ctnc_wm_comp:
        ld      ioctag(g1),r11          # r11 = cache tag address
        movl    g0,r12                  # r12 = Return status
                                        # r13 = ILT
#
# --- Free the Cache Buffer, Tag, and complete the I/O command with the status
#       returned from the lower layers.
#
        ld      tg_bufptr(r11),g0       # g0 = Buffer address
        ld      tg_vlen(r11),g1         # g1 = Buffer size
        movl    g2,r14                  # r14 = g2
                                        # r15 = g3
        call    wc$mrel_cbuffer         # Release the Cache buffer
        mov     r11,g0                  # g0 = Tag address
        call    wc$mrel_ctag            # Release the Tag
        ld      vrvrp(r13),r10          # r10 = VRP
        movq    r12,g0                  # Restore saved registers
        stob    r12,vr_status(r10)      # Save the Completion Code
        b       wc$complete_io          # Complete the operation
#
#******************************************************************************
#
#  NAME: wc$ctnc_rw_comp
#
#  PURPOSE:
#       To complete the write operation that has Write Cache buffers and a tag
#       allocated that was a Resident Write Hit.
#
#  DESCRIPTION:
#       This routine completes the write operation that has a Write Cache
#       buffer and a tag associated with it that was a resident write hit.
#       It will clear the locks, set the tag to resident, invalidate the tag,
#       and complete the operation.
#
#  INPUT:
#       g0 = return status
#       g1 = ILT for the operation
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
wc$ctnc_rw_comp:
        ld      ioctag(g1),r11          # r11 = cache tag address
        movl    g0,r12                  # r12 = Return status
                                        # r13 = ILT
#
# --- Clear the old locks, set the lock to Invalidate, and set the tag
#       to Resident.
#
        mov     0,r6                    # r6 = Clearing register
        ldos    tg_attrib(r11),r3       # r3 = Tag attributes
        ldos    tg_state(r11),r4        # r4 = Tag state
        movl    g2,r14                  # Save registers g2-g3

        clrbit  TG_LOCKED_WRITE,r4,r4   # Remove tag locked for write
        setbit  TG_LOCKED_INVALIDATE,r4,r4 # Flag it as locked for invalidate
#
        clrbit  TG_DIRTY,r3,r3          # Clear the Dirty Flag
        setbit  TG_RESIDENT,r3,r3       # Set the Resident Flag
#
        stos    r3,tg_attrib(r11)       # Store tag attributes
        stos    r4,tg_state(r11)        # Store tag state
#
# --- Remove the tag from the dirty tree
#
        ldos    tg_vid(r11),g0          # g0 = VID for this tag
        ld      tg_dirtyptr(r11),r5     # r5 = Dirty tree node to delete
        ld      vcdIndex[g0*4],r10      # r10 = VCD pointer for this VID
        mov     r5,g1                   # g1 = Dirty tree node to delete
        ld      vc_dirty(r10),g0        # g0 = Dirty root of the tree
        call    RB$delete               # Remove it from tree
        st      g0,vc_dirty(r10)        # Save root ptr
c       put_wc_rbnode(r5);              # Deallocate tree element
        st      r6,tg_dirtyptr(r11)     # Clear out the Dirty Node pointer
#
# --- Update the counts to show a Dirty Tag went to Resident status
#
        ld      tg_vlen(r11),g0         # g0 = Number of blocks for this op
        call    wc$DecDirtyCount        # Decrement the Dirty Counters
        call    wc$IncResidentCount     # Increment the Resident Counters
#
# --- Invalidate the tag and post any ops waiting for this tag
#
        mov     r11,g0                  # g0 = Tag address
        call    wc$invalidate_tag       # Invalidate tag - Clears the tag state
        call    wc$unlock_tag           # Tag unlock postprocessing & free tag
#
# --- Complete the operation
#
        ld      vrvrp(r13),r10          # r10 = VRP
        movq    r12,g0                  # Restore saved registers
        stob    r12,vr_status(r10)      # Save the Completion Code
        b       wc$complete_io          # Complete the operation and return
#
#******************************************************************************
#
#  NAME: wc$ph_compl
#
#  PURPOSE:
#       To provide completion services for blocked I/Os that used
#       a "placeholder" ILT on the blocked list.
#
#  DESCRIPTION:
#       This routine is passed the address of a "placeholder" ILT that
#       is linked onto a blocked I/O list for an I/O that has just
#       completed.  It performs the following actions:
#         1) Unlinks the placeholder from the blocked I/O list.
#         2) Deallocates the placeholder.
#         3) Locates the original ILT and decrements the blocked
#            count.
#         4) If the blocked count has reached zero, the I/O is submitted.
#
#  INPUT:
#       g1 = placeholder for this I/O
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g1; also see <wc$submit>.
#
#******************************************************************************
#
wc$ph_compl:
#
# --- Get ILT for this request
#
        ld      il_misc(g1),r15         # ILT address
        ld      il_w1(r15),r8           # Get blocked I/O counter
#
# --- Unlink placeholder from list.
#
        ldl     il_fthd(g1),r4          # get forward/backward pointers
        cmpo    r4,r5                   # Check for last blocked I/O on list
        sele    r4,0,r6                 # Set pointers
        sele    r5,0,r7                 #   to zero if true
        st      r6,il_fthd(r5)          # set forward pointer of previous block
#
# --- Because the blocked list is circular, the backward thread pointer is
#     never null.
#
        st      r7,il_bthd(r4)          # Unlink from list
#
# --- Deallocate placeholder.
#
c       put_wc_plholder(g1);            # Release the placeholder ILT
#
# --- Decrement blocked I/O count.
#
        cmpdeci 1,r8,r8
        st      r8,il_w1(r15)           # Save blocked I/O counter
        bl      .wcph10                 # Jif blocked by other I/Os
#
# --- This was the last I/O blocking; submit this I/O
#
        ldconst wc$io_compl,g2          # Setup completion routine for I/O
c       wc_submit(r15, g2);             # Queue this request
#
.wcph10:
        ret                             # Done!
#
#******************************************************************************
#
#  NAME: wc$io_compl
#
#  PURPOSE:
#       To provide completion services for I/Os that have completed or where
#       the write data has been received and the blocked I/Os can be released.
#
#  DESCRIPTION:
#       This routine is passed the address of an ILT that has an associated
#       VRP that has completed.  It performs the following functions:
#         1) Calls the completion routine for any blocked I/Os chained
#            to this ILT.
#         2) Removes the I/O from the interval tree.
#         3) And if the call is to wc$io_compl, moves the ILT up one level and
#            calls the associated completion routine.
#
#  INPUT:
#       g1 = ILT for this I/O
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Possibly all g registers.
#
#******************************************************************************
#
wc$io_compl:
        ld      vrvrp(g1),r13           # Get VRP pointer
        mov     g0,r14                  # Preserve g0
        ldos    vr_vid(r13),r12         # r6 = VID
        mov     g1,r15                  # Preserve ILT pointer
        ld      vcdIndex[r12*4],r11     # Retrieve VCD pointer for this VID

c       record_cache(FR_CACHE_EXEC_COMPLETE, (void *)r13);
.if     DEBUG_FLIGHTREC_C
        ldconst frt_c_iocomp,r3         # Cache - wc$io_compl
        ldos    vr_func(r13),r4         # r4 = VRP Function
        shlo    8,r4,r4                 # Set up to have several values in parm0
        shlo    16,r12,r6
        or      r4,r3,r3                # r3 = Function, Flight Recorder ID
        or      r6,r3,r3                # r3 = VID, Function, FRID
        st      r3,fr_parm0             # VID, Function, Flight Recorder ID
        st      r15,fr_parm1            # ILT
        st      r13,fr_parm2            # VRP
        ld      il_fthd(r15),r4         # Blocked I/Os Forward Thread
        st      r4,fr_parm3
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_C
#
# --- Check for blocked I/Os linked to this ILT.
#
.wcioc10:
        ld      il_fthd(r15),g1         # Get first blocked I/O
        cmpobe  0,g1,.wcioc20           # Jif no blocked I/Os
#
# --- Because the completion routine for a blocked I/O could result in
#     the process calling this routine to block, we always start with
#     the original ILT address as the starting point for each iteration.
#
        ld      il_cr(g1),r6            # Get completion routine address
        callx   (r6)                    # Invoke completion routine
#
# --- The completion routines for blocked I/Os are expected to remove the
#     ILT from the blocked list before any action that could cause process
#     block.  Therefore, it is safe to reload the original ILT and look
#     at the thread pointers to see if more I/Os are blocked after returning
#     from the completion routine.
#
        b       .wcioc10
#
# --- Done with blocked I/O list.
#
.wcioc20:
#
# --- Remove this node from the interval tree.
#
# --- If the interval tree node consisted
#     of a linked list of nodes with the same LBA start disk address,
#     we need to remove the node from the linked list
#     and update the maximum node value.  If this node is the only one
#     it is deleted from the tree.
#
        ld      il_w5(r15),g1           # Get tree node pointer
        cmpobe  0,g1,.wcioc30           # Only delete if not already done
        ld      vc_io(r11),g0           # Get the Pending I/O Tree Root
        call    WC$delete_node          # Delete node from tree
#
# --- Update root value
#
        st      g0,vc_io(r11)           # Save the new root pointer
        mov     0,r3                    # Zero out the Tree node pointer
        st      r3,il_w5(r15)           #  to show it has already been deleted
#
# --- Done with update - remove the OTV from the VTV and CTV, start the VCD wait
#       process (if anything is waiting), and then call the completion routine.
#
.wcioc30:
        ld      vr_otv(r13),r6          # Get the OTV
        ld      C_ctv,r4                # Get the current CTV
        ld      vc_vtv(r11),r3          # Get the current VTV
        subo    r6,r4,r4                # Update the CTV
        subo    r6,r3,r3                # Update the VTV
.ifdef M4_DEBUG_C_ctv
c CT_history_printf("%s%s:%u: C_ctv starts=%lu ends=%lu vc_vtv[%ld]=%ld\n", FEBEMESSAGE,__FILE__, __LINE__, C_ctv, r4, r12, r3);
.endif  # M4_DEBUG_C_ctv
        st      r4,C_ctv
        st      r3,vc_vtv(r11)
#
        ld      C_vcd_wait_head,r6      # Get the wait head queue
        ld      C_vcd_wait_pcb,r4       # Get the VCD Wait Exec PCB
        cmpobe  0,r6,.wcioc50           # Jif nothing is waiting
        ldob    pc_stat(r4),r3          # Get current process status
        cmpobne pcnrdy,r3,.wcioc50      # Jif status other than not ready
        mov     pcrdy,r5                # Get ready status
.ifdef HISTORY_KEEP
c CT_history_pcb(".wcioc30 setting ready pcb", r4);
.endif  # HISTORY_KEEP
        stob    r5,pc_stat(r4)          # Ready process
#
.wcioc50:
        mov     r14,g0                  # Restore the registers saved
        mov     r15,g1
        b       K$comp                  # Call the completion routine & exit
#
#******************************************************************************
#
#  NAME: WC$delete_node
#
#  PURPOSE:
#       Delete a node from the blocked I/O tree.
#
#  DESCRIPTION:
#       This routine is used to delete a node from an existing blocked I/O tree.
#       The node may be linked onto a list of nodes that have the same
#       key address; in this case the node may only be removed from
#       the list; no tree deletion is done.  The maximum node key value
#       is recomputed and propagated up the tree if necessary.
#
#  INPUT:
#       g0 = address of root element of tree.
#       g1 = pointer to element to delete.
#
#  OUTPUT:
#       g0 = possible new root pointer; zero if tree empty.
#
#  REGS DESTROYED:
#       g0-g8
#
#******************************************************************************
#
WC$delete_node:
.if rbidebug_NodeMax
        PushRegs(r3)
        call    CheckNodeMaximum        # Check the NodeMax values in the tree
        PopRegsVoid(r3)
.endif  # rbidebug_NodeMax
        mov     g1,r14                  # Preserve node pointer
#
# --- Determine if this element is linked onto a duplicate node list.
#     if true, locate the head of the list.
#
        ldl     rbfthd(g1),r4           # Check forward/backward pointer
        cmpobne 0,r5,.wcidn80           # Jif on linked list but not at head
        cmpobne 0,r4,.wcidn10           # Jif on head of linked list
        call    RBI$delete              # Not on linked list;
                                        #  remove this node from tree
        b       .wcidn160
#
# --- This node is on a linked list of duplicate nodes.
#     If not head of list <rbparent> points to it.
#
.wcidn10:
#
# --- Node to be removed is at head of list.
#     Need to make the next node on the list the head.
#
#   g0 = root of tree
#   g1 = pointer to node
#
#   r4/r5 = forward/backward pointers
#
# --- This node has other duplicate nodes linked to it.
#     Remove this node from the list and then recalculate the node
#     maximums.  The parent pointers of the children of this node
#     must be updated as well as the pointer from the parent.
#
#     g1 = node
#     r4/r5 = forward/backward pointers
#
        ldl     rbcleft(g1),r6          # get left/right children ptrs
        lda     ._nil,r15               # get NIL pointer
        cmpobe  r15,r6,.wcidn40         # jif NIL left child
        st      r4,rbparent(r6)         # set parent of left child
.wcidn40:
        cmpobe  r15,r7,.wcidn50         # Jif NIL right child
        st      r4,rbparent(r7)         # set parent of right child
#
.wcidn50:
#
# --- Set pointer from parent to next node; if at root ignore but
#     set new value of root (g0)
#
        ld      rbparent(g1),r3         # get parent of node
        cmpobne 0,r3,.wcidn60           # jif not at root
#
        mov     r4,g0                   # set root as next node
        b       .wcidn70
#
.wcidn60:
        ld      rbcleft(r3),r8          # get left child pointer from parent
#
# --- Determine which child we are of parent and store correct child pointer.
#
        cmpo    r8,g1                   # check for left child
        sele    rbcright,rbcleft,r8     # Select correct offset
        addo    r8,r3,r8                # Gen address to child of parent
        st      r4,(r8)                 # Set correct child pointer
#
.wcidn70:
#
# --- Store parent and right/left child pointers into new node.
#
        stl     r6,rbcleft(r4)
        st      r3,rbparent(r4)
#
# --- Indicate that this node is at head of list by clearing backward
#     pointer.
#
        ldconst 0,r3
        st      r3,rbbthd(r4)
#
# --- Copy the color of the node to be deleted into the node replacing it
#
        ldob    rbcolor(g1),r3          # Get the original nodes color
        stob    r3,rbcolor(r4)          # Store in the new node
#
# --- Calculate new maximum for this node, pointer is in r4
#
        mov     r4,r11                  # Preserve node pointer
        ldconst TRUE,r12                # Clear changed counter
        ldconst TRUE,r13                # Set new parent flag
        mov     r4,r3
        ldl     rbkeym(r4),r6           # Get key max values
        b       .wcidn120               # Recalc max/parent values
#
.wcidn80:
#
# --- The node to be deleted is in the middle of the doubly linked list.
#     We need to remove the node from the list and then recalculate the
#     key/node maximum values.
#
        cmpobe  0,r4,.wcidn90           # Jif at end of list
#
        st      r5,rbbthd(r4)           # Set backward pointer of next node
#
.wcidn90:
        st      r4,rbfthd(r5)           # Set forward pointer of prev node
#
# --- Lookup key value of head of list and then recalculate maximum
#     for node. g1 = node just deleted
#
        ld      rbparent(g1),r3         # Get head of list
        ld      rbdpoint(r3),r4         # Get ILT pointer
        ldl     rbkey(r3),r6            # Get starting address
        ld      vrvrp(r4),r4            # Get VRP pointer
        ld      vr_vlen(r4),r4          # Get request size
        cmpdeco 0,r4,r4                 # Adjust endpoint and clear carry bit
        bne     .wcidn95                # Jif original length is not zero
        ldconst 0xffffffff,r6           # Sync Cache command - set the end
        ldconst 0xffffffff,r7           #  range of request to the maximum
        b       .wcidn110
#
.wcidn95:
        addc    r4,r6,r6                # Calc
        addc    0,r7,r7                 #   key endpoint
#
.wcidn110:
        mov     r3,r11                  # Preserve head of list
        ldconst FALSE,r12               # set no change made
        ldconst FALSE,r13               # set no change parent
#
# --- Go through linked list and calculate node maximum based on
#     key values.  If r13 = TRUE, set the parent pointer of each
#     node in the list to the head.
#
#       r3  = Current position in list
#       r11 = Head of list
#
.wcidn120:
        ld      rbfthd(r3),r4           # get forward pointer
        cmpobe  0,r4,.wcidn150          # Jif end of list
#
        ldl     rbkeym(r4),r8           # get next node key max
        cmpobe  FALSE,r13,.wcidn130     # Jif no set parent
#
        st      r11,rbparent(r4)
#
.wcidn130:
        cmpobne r9,r7,.wcidn140         # jif MSBs differ
        cmpo    r8,r6                   # Compare LSBs
#
.wcidn140:
        selg    r6,r8,r6                # Select greater
        selg    r7,r9,r7                #   of both keys
        selg    r12,TRUE,r12            # Set changed to TRUE
                                        #  if change in keys
        mov     r4,r3
        b       .wcidn120               # Loop for next key
#
.wcidn150:
        cmpobe  FALSE,r12,.wcidn160     # Jif no change; done
        mov     0,r12                   # Set up to zero nodem
        mov     0,r13
        stl     r6,rbkeym(r11)          # Store new key max
        stl     r12,rbnodem(r11)        # Zero the NodeMax value
        mov     r11,g1                  # Setup for <rbi$maxprop>
        call    rbi$maxprop             # recalculate node maximum
                                        #  from node up to root/return
#
# --- Release node into memory free pool.
#
.wcidn160:
c       put_wc_rbinode(r14);            # Release node memory
.if rbidebug_NodeMax
        PushRegs(r3)
        call    CheckNodeMaximum        # Check the NodeMax values in the tree
        PopRegsVoid(r3)
.endif  # rbidebug_NodeMax
        ret
#
#******************************************************************************
#
#  NAME: WC$deleteVIDdata
#
#  PURPOSE:
#       Delete any data in Cache for this VID
#
#  DESCRIPTION:
#       This routine will delete all data in the cache, resident or dirty,
#       for a particular VID (used during Delete VID processing and Write Cache
#       invalidation).  This routine requires no device activity to the VID.
#
#       An Input parameter is used in determining if all Tags or Tags that do
#       not have a certain Locked state will be invalidated.
#
#  INPUT:
#       g0 = VID
#       g1 = Tag Locked State Mask to prohibit deleting
#            0 = Delete all
#            TG_LOCKED_FLUSH = Delete all except Locked for Flush Tags
#            TG_LOCKED_FLUSH and TG_LOCKED_WRITE = Delete all except Locked for
#               Flush and Locked for Write tags
#            etc.
#
#  OUTPUT:
#       g0 = Return Status
#            TRUE - all Tags were deleted
#            FALSE - some tags that met the Mask state were not deleted
#
#******************************************************************************
#
WC$deleteVIDdata:
#
        PushRegs                        # Save all G registers (stack relative)
        ldconst TRUE,r12                # Prep return status to show all deleted
        mov     g1,r13                  # Save Mask
#
# --- Get root value for this VID
#
        ld      vcdIndex[g0*4],r11      # r11 = VCD pointer for this VID
        ld      vc_cache(r11),r10       # r10 = the cache RB tree root
        cmpobe  0,r10,.wcdvd900         # Jif there is nothing in the tree
#
# --- Get the first entry in the Valid Data tree.
#
        movl    0,g2                    # g2,g3 = Start of drive
        mov     r10,g0                  # g0 = root of RB tree
        call    RB$locateStart          # Get the first entry
        mov     g1,r9                   # r9 = Node detected
#
# --- Loop through several before waiting to allow other tasks to complete
#
        ld      K_ii+ii_time,r14        # r14 = current time
        ldconst MAX_FLUSH_INV_TIME,r15  # r15 = maximum time to delete before
                                        #  swapping tasks
        addo    r14,r15,r15             # r15 = time where swap required
#
.wcdvd10:
        mov     r9,g1                   # g1 = RB pointer for last overlap
        call    RB$locateNext           # Find next tag in the tree, if any
        mov     g1,r3                   # Preserve results of search
#
        ld      rbdpoint(r9),g0         # g0 = cache tag to delete
#
# --- Determine if it is OK to delete this tag via the mask passed in and the
#       tag state
#
        cmpobe  0,r13,.wcdvd20          # Jif no mask
        ld      tg_hqueue(g0),r6        # Determine if something is on the queue
        cmpobne 0,r6,.wcdvd15           # Jif something is queued to this tag
        ldos    tg_state(g0),r7         # r7 = Tag State
        and     r13,r7,r7               # Get masked state
        cmpobe  0,r7,.wcdvd20           # Jif ok to delete this tag
.wcdvd15:
        ldconst FALSE,r12               # Show all the tags were not deleted
        b       .wcdvd800               # Go check the next tag
#
# --- Get sizeof request and buffer pointer.
#
.wcdvd20:
        ldos    tg_attrib(g0),r4        # r4 = Tag attributes
        ldl     tg_vlen(g0),r6          # r6 = length r7 = buffer ptr
        mov     g0,r5                   # r5 = Cache Tag being worked on
        bbc     TG_DIRTY,r4,.wcdvd100   # Jif not dirty data
#
# --- Tag is dirty.  Remove from the Dirty Tree, and then treat like valid
#
        ld      vc_dirty(r11),g0        # g0 = Dirty Root Pointer
        ld      tg_dirtyptr(r5),g1      # g1 = Dirty Node to delete
        mov     g1,r8                   # r8 = Dirty Node to deallocate
        call    RB$delete               # Delete the node
        st      g0,vc_dirty(r11)        # Save the new dirty root pointer
#
# --- Deallocate tree element
#
c       put_wc_rbnode(r8);              # Deallocate tree element
#
# --- Decrement the Dirty count if it is not a BE tag (do not keep track of
#       BE tag count)
#
        bbs     TG_BE,r4,.wcdvd130      # Jif the Tag is for the BE
        mov     r6,g0                   # g0 = Block Count
        call    wc$DecDirtyCount        # Decrement the Dirty Count
        b       .wcdvd120               # Free the Cache Node, tag, and buffer
#
# --- This tag is only in the Cache Valid tree
#
.wcdvd100:
#
# --- Remove from LRU list.
#
        call    wc$remove_lru_queue
#
# --- Decrement the Resident count if it is not a BE Tag (do not keep track
#       of BE tag count)
#
        bbs     TG_BE,r4,.wcdvd130      # Jif the tag is for the BE
        mov     r6,g0                   # g0 = Block count
        call    wc$DecResidentCount     # Dec the Resident tag and block count
#
# --- Increment the Free Count
#
.wcdvd120:
        call    wc$IncFreeCount         # Increment the Free tag and block count
                                        # NOTE: Free Pending is considered FREE
#
# --- Set invalid bits and save in NV Memory (local and maybe remote)
#
.wcdvd130:
        PushRegs(r8)
        mov     r5,g0                   # Init the Tag
        lda     -BE_ADDR_OFFSET(r5),r14 # r14=Destination (if a BE Tag)
        call    WC_initTag
        mov     r5,g1                   # g1 = Source
        chkbit  TG_BE,r4                # Is this a BE Tag or FE Tag
        sele    r5,r14,g0               # g0 = Destination BE/FE Tag
        sele    MIRROR_MP,MIRROR_BE,g2  # g2 = Mirror Destination (BE or MP)
        ldconst tgsize,g3               # g3 = Length to transfer
        call    WC_CopyNVwWait          # Copy to NV Memory (local and remote)
        PopRegsVoid(r8)
#
# --- Remove from cache tag tree.
#
        mov     r9,g1                   # g1 = node to delete
        mov     r10,g0                  # g0 = root of tree
        call    RB$delete               # Remove it from tree
        mov     g0,r10                  # r10 = new root pointer
        st      g0,vc_cache(r11)        # Save root ptr
#
# --- Deallocate tree element
#
c       put_wc_rbnode(r9);              # Deallocate tree element
#
# --- Deallocate buffer. r6 = length r7 = buffer pointer
#
        mov     r7,g0
        mov     r6,g1
        call    wc$mrel_cbuffer         # Release buffer
#
# --- Link tag to freelist
#
        mov     r5,g0                   # g0 = Cache Tag to free
        call    wc$mrel_ctag            # Release it to free pool/return
#
# --- Continue working the tree until they are all gone
#
.wcdvd800:
        ld      K_ii+ii_time,r14        # r14 = current time
        cmpobl  r14,r15,.wcdvd810       # Determine if time to let other work
        mov     1,g0                    # Wait minimal amount of time
        call    K$twait
        ld      K_ii+ii_time,r14        # r14 = current time
        ldconst MAX_FLUSH_INV_TIME,r15  # r15 = maximum time to delete before
                                        #  swapping tasks
        addo    r14,r15,r15             # r15 = time where swap required
.wcdvd810:
        mov     r3,r9                   # Restore results of next tree node
        cmpobne FALSE,r3,.wcdvd10       # Jif another tag was found
#
# --- All Done
#
.wcdvd900:
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
#******************************************************************************
#
#  NAME: WC$InvalidateFE
#
#  PURPOSE:
#       Invalidates the FE Write Cache for a VID, several VIDs, or all VIDs
#
#  DESCRIPTION:
#       This routine will delete all data in the cache, resident or dirty,
#       for a particular VID, several VIDs, or all VIDs.  If a VID or list of
#       VIDs is passed, the VID(s) will be verified to see if they exist.  If
#       one does not exist an error will be returned to the caller.
#
#       If Cache has not begun initialization, then the FE Write Cache is
#       is cleared without the need to remove from trees and freeing of memory.
#
#       NOTE: If any tag is in the locked state, this function will wait
#               on the lock to be removed before continuing unless Cache has
#               not been initialized yet.
#
#  INPUT:
#       g0 = Parameter pointer for MRP
#
#  OUTPUT:
#       g0 = MRP Return Code
#               deok (0x00) - Function completed successfully
#               deinitinprog (0x0A) - Cache initialization in progress
#               deinvvid (0x030) - At least one VID was not found that
#                   was requested
#       g1 = Invalid VID if g0 = deinvvid otherwise trashed
#
#  REGISTER USAGE:
#        r3 = MRP Options flag / Temporary
#        r4 = Index into VID List / Temporary
#        r5 = VCD pointer for VID / Temporary
#        r6 = Cache Statistics pointer / Temporary
#        r7 = temporary register
#        r8 = temporary register
#        r9 = TRUE constant
#       r10 = VID array pointer
#       r11 = Tags not deleted Flag (some tag was locked and could not be
#               deleted yet) / Temporary
#       r12 = VID List pointer
#       r13 = Number of VIDs to process
#       r14 = Maximum allowed VID + 1
#       r15 = MRP Parameters pointer
#
#        g0 = Parameter passing / return status
#        g1 = VID being processed / failing VID
#
#******************************************************************************
#
WC$InvalidateFE:
        mov     g0,r15                  # Save Parameter pointer
        ldconst MAXVIRTUALS,r14         # r14 = maximum VID + 1
        ldconst TRUE,r9                 # r9 = TRUE constant
#
# --- Turn off the Background Flush Task before starting so that no new flush
#       ops are generated while the FE Write cache is being invalidated.
#
        lda     C_ca,r6                 # r6 = The Cache Statistics Pointer
        ld      C_haltBackgroundFlag,r5 # r5 = Background Halt Flag
        ldob    ca_status(r6),r4        # r4 = Global Cache Status
        setbit  hbginvfe,r5,r5          # Show Invalidate FE is halting BG flush
        setbit  ca_halt_background,r4,r4 # Halt the Background Flush Task
        st      r5,C_haltBackgroundFlag # Save new Halt Background Flag
        stob    r4,ca_status(r6)        # Save new status
#
# --- Determine if this is a global invalidate or list of VIDs invalidate
#       and handle with the appropriate code.
#
        ldob    mif_op(g0),r3           # r3 = Options flag
        cmpobne mifoinvalvid,r3,.wc_invfe500 # Jif it is not a VID list
#
# --- List of VIDs to invalidate ----------------------------------------------
#
#   If the cache has not been initialized yet, need to delete directly from
#       the cache.  If cache is in the process of being initialized, return the
#       error saying that Cache is being initialized (cannot wait due to log
#       events in init that can require user input (another MRP))
#
        ldos    mif_nvids(r15),r13      # r13 = Number of VIDs to process
        lda     mif_vidlist(r15),r12    # r12 = Pointer to VID list
        ldconst 0,r4                    # r4 = Offset into VID list
        ldos    K_ii+ii_status,r7       # r7 = Initialization status
        cmpobe  0,r13,.wc_invfe800      # Jif no VIDs requested - good status
        bbc     iimpfound,r7,.wc_invfe300 # Jif Cache init has not begun
        bbs     iicinit,r7,.wc_invfe100 # Jif Cache init has completed
#
        ldconst deinitinprog,g0         # g0 = Return code - Init in progress
        ldconst 0xFFFF,g1               # Show invalid VID number
        b       .wc_invfe900            # Report the problem
#
#   Verify all the VIDs in the list are valid
#
.wc_invfe100:
        cmpobe  r4,r13,.wc_invfe200     # Jif all VIDs tested good
        ldos    (r12)[r4*2],g1          # g1 = VID being tested
        cmpobge g1,r14,.wc_invfe190     # Jif the VID is too big
        ld      vcdIndex[g1*4],r5       # r5 = VCD pointer
        cmpobe  0,r5,.wc_invfe190       # Jif undefined
        addo    1,r4,r4                 # Point to the next VID in the VID list
        b       .wc_invfe100            # Check the next VID in the VID list
#
#   An invalid VID was found, report the error to the requester and return
#
.wc_invfe190:
        ldconst deinvvid,g0             # Show that an Invalid VID was found
                                        # g1 = VID that failed the test
        b       .wc_invfe900            # return the status
#
#   VID List tested good, now go delete any data for those specified VIDs
#
.wc_invfe200:
        ldconst 0,r4                    # r4 = Offset into VID list
        ldconst FALSE,r11               # r11 = Flag showing no wait is needed
                                        #  for locked tags
.wc_invfe220:
        cmpobe  r4,r13,.wc_invfe260     # Jif all VIDs have been processed
        ldos    (r12)[r4*2],g0          # g0 = VID to invalidate data for
        ldconst 0xFFFFFFFF,g1           # g1 = State Mask = Do not delete any
                                        #  locked tags (any type of lock)
        call    WC$deleteVIDdata        # Do the delete (if any exists)
        cmpobe  TRUE,g0,.wc_invfe240    # Jif all was invalidated
        ldconst TRUE,r11                # r11 = Flag showing need to try again
.wc_invfe240:
        addo    1,r4,r4                 # Point to the next VID
        b       .wc_invfe220            # Go process the next VID
#
#   Determine if all were the tags were deleted yet.  If so, all done.  If not,
#       wait a small amount of time and retry.
#
.wc_invfe260:
        cmpobe  FALSE,r11,.wc_invfe800  # Jif all done
        ldconst 1,g0                    # Not done, wait the minimum
        call    K$twait
        b       .wc_invfe200            # Try again to delete all the tags
#
#   Cache is not initialized yet, nor in the process of being initialized.  Go
#       through the cache and delete any data for the VIDs noted.  Do this by
#       building a byte field list for each possible VID and then setting to
#       TRUE each requested VID.  Then walk through all the tags and any that
#       have a matching VID will have the tag cleared.
#
.wc_invfe300:
c       r10 = s_MallocC(r14, __FILE__, __LINE__); # Allocate space for one byte / possible VID array
.wc_invfe320:
        cmpobe  r4,r13,.wc_invfe340     # Jif all VIDs have been processed
        ldos    (r12)[r4*2],g1          # g1 = VID to invalidate data for
        cmpobge g1,r14,.wc_invfe330     # Jif the VID is too large
        stob    r9,(r10)[g1*1]          # Show the VID is valid to delete
        addo    1,r4,r4                 # Point to the next VID
        b       .wc_invfe320            # Go process the next VID
#
#   VID is too large.  Free the VID array and return the error.
#
.wc_invfe330:
c       s_Free(r10, r14, __FILE__, __LINE__);
        b       .wc_invfe190            # Report the error
#
#   VID flags have been created for each possible VID, now clear any tags that
#       are being requested to be deleted.
#
.wc_invfe340:
        PushRegs(r4)                    # Save 'g' registers
        ld      WctAddr,r5              # r5 = Pointer to cache tag
        ldconst tgsize,r7               # r7 = Tag incrementer value
        ld      WctSize,r6              # r6 = Size of the cache tag area
        addo    r5,r6,r6                # r6 = End of cache tag area
.wc_invfe360:
        ldos    tg_vid(r5),r8           # r8 = VID associated with this tag
        cmpobge r8,r14,.wc_invfe370     # Jif VID is too big - ignore.  It will
                                        #  be cleaned up later in the Cache
                                        #  initialization routines
        ldob    (r10)[r8*1],r3          # r3 = Flag for this byte
        cmpobne r9,r3,.wc_invfe370      # Jif this VID is not being invalidated
#
#       Tag found that matches the VID, now clear it out and mirror it
#
        mov     r5,g0                   # g0 = Tag being worked on
        call    WC_initTag              # Initialize the Tag
        mov     r5,g0                   # g0 = Destination (same as Destination)
        mov     r5,g1                   # g1 = Source
        ldconst MIRROR_FE,g2            # g2 = Mirror local
        mov     r7,g3                   # g3 = Length to transfer
        call    WC_CopyNVwWait          # Copy to local NV Memory
#
#       Go to the next tag and continue processing until all done
#
.wc_invfe370:
        addo    r7,r5,r5                # Point to the next Tag
        cmpobl  r5,r6,.wc_invfe360      # Jif more tags to process
        PopRegsVoid(r4)                 # Restore 'g' registers
#
#   All done with invalidating the FE VID List.  Free the VID array and
#       complete the command
#
c       s_Free(r10, r14, __FILE__, __LINE__);
        b       .wc_invfe800            # Show completed good
#
# --- Global Invalidation of Write Cache --------------------------------------
#
#   If the cache has not been initialized yet, need to delete directly from
#       the cache.  If cache is in the process of being initialized, return the
#       error saying that Cache is being initialized (cannot wait due to log
#       events in init that can require user input (another MRP))
#
.wc_invfe500:
        ldos    K_ii+ii_status,r7       # r7 = Initialization status
        bbc     iimpfound,r7,.wc_invfe600 # Jif Cache init has not begun
        bbs     iicinit,r7,.wc_invfe520 # Jif Cache init has completed
#
        ldconst deinitinprog,g0         # g0 = Return code - Init in progress
        ldconst 0xFFFF,g1               # Show invalid VID number
        b       .wc_invfe900            # Report the problem
#
.wc_invfe520:
        ldconst FALSE,r11               # r11 = Flag showing no wait is needed
                                        #  for locked tags
        ldconst -1,r4                   # r4 = VID being processed (init to -1
                                        #  because next loop pre-increments)
.wc_invfe540:
        addo    1,r4,r4                 # increment the VID
        cmpobe  r4,r14,.wc_invfe580     # Jif no more VIDs to check
        ld      vcdIndex[r4*4],r5       # r5 = VCD pointer
        cmpobe  0,r5,.wc_invfe540       # Jif undefined
#
#   VID found, delete any data for it.
#
        mov     r4,g0                   # Input g0 = VID to delete data for
        ldconst 0xFFFFFFFF,g1           # g1 = State Mask = Do not delete any
                                        #  locked tags (any type of lock)
        call    WC$deleteVIDdata        # Do the delete (if any exists)
        cmpobe  TRUE,g0,.wc_invfe560    # Jif all was invalidated
        ldconst TRUE,r11                # r11 = Flag showing need to try again
.wc_invfe560:
        b       .wc_invfe540            # Continue deleting data
#
#   Determine if all were the tags were deleted yet.  If so, all done.  If not,
#       wait a small amount of time and retry.
#
.wc_invfe580:
        cmpobe  FALSE,r11,.wc_invfe800  # Jif all done
        ldconst 1,g0                    # Not done, wait the minimum
        call    K$twait
        b       .wc_invfe520            # Try again to delete all the tags
#
#   Cache is not initialized yet.  Need to delete the tags directly from the tag
#       area (VCDs are not set up yet).
#
.wc_invfe600:
        PushRegs(r4)
        ld      WctAddr,r5              # r5 = Pointer to cache tag
        ldconst tgsize,r7               # r7 = Tag incrementer value
        ld      WctSize,r6              # r6 = Size of the cache tag area
        addo    r5,r6,r6                # r6 = End of cache tag area
#
#       Clear this tag
#
.ifdef HISTORY_KEEP
c CT_HISTORY_OFF();
.endif  # HISTORY_KEEP
.wc_invfe660:
        mov     r5,g0                   # g0 = Tag to initialize
        call    WC_initTag
#
#       Go to the next tag and continue processing until all done
#
        addo    r7,r5,r5                # Point to the next Tag
        cmpobl  r5,r6,.wc_invfe660      # Jif more tags to process
.ifdef HISTORY_KEEP
c CT_HISTORY_ON();
.endif  # HISTORY_KEEP
#
#       Mirror all the tags to the NV Memory
#
        ld      WctAddr,g0              # g0 = Destination (same as Destination)
        mov     g0,g1                   # g1 = Source
        ldconst MIRROR_FE,g2            # g2 = Mirror local
        ld      WctSize,g3              # g3 = Length to transfer
        call    WC_CopyNVwWait          # Copy to Local NV Memory
        PopRegsVoid(r4)
#
# --- All done deleting all the VID data, now set up the return parameters.
#
.wc_invfe800:
        ldconst deok,g0                 # Show the Invalidate completed OK
        ldconst 0xFFFF,g1               # Show invalid VID number
#
# --- All Done.  Allow the Background Flush task to continue as normal if others
#       do not need it stopped still.
#
.wc_invfe900:
        ld      C_haltBackgroundFlag,r5 # r5 = Background Halt Flag
        lda     C_ca,r6                 # r6 = The Cache Statistics Pointer
        clrbit  hbginvfe,r5,r5          # Clear Invalidate FE is halting BG
        st      r5,C_haltBackgroundFlag # Save new Halt Background Flag
        cmpobne 0,r5,.wc_invfe910       # Jif cannot let Background Flushes go
        ldob    ca_status(r6),r4        # r4 = Global Cache Status
        clrbit  ca_halt_background,r4,r4 # Allow the Background Flush Task to
        stob    r4,ca_status(r6)        #   continue as normal
.wc_invfe910:
        ret
#
#******************************************************************************
#
#  NAME: WC$InvalidateBE
#
#  PURPOSE:
#       Invalidates the BE Write Cache for a VID, several VIDs, or all VIDs
#
#  DESCRIPTION:
#       This routine will delete all data in the cache, resident or dirty,
#       for a particular VID, several VIDs, or all VIDs that have data in the
#       BE Mirrored Write Cache.  If a VID or list of VIDs is passed, the
#       VID(s) will be verified to see if they exist.  If one does not exist
#       an error will be returned to the caller.
#
#       If Cache has not begun initialization, then the FE Write Cache is
#       is cleared without the need to remove from trees and freeing of memory.
#
#       NOTE: This function requires no device activity to the VID(s).
#
#  INPUT:
#       g0 = Parameter pointer for MRP
#
#  OUTPUT:
#       g0 = MRP Return Code
#               deok (0x00) - Function completed successfully
#               deinitinprog (0x0A) - Cache initialization in progress
#               deinvvid (0x030) - At least one VID was not found that
#                   was requested
#       g1 = Invalid VID if g0 = deinvvid otherwise trashed
#
#  REGISTER USAGE:
#        r3 = MRP Options flag / Temporary
#        r4 = Index into VID List / Temporary
#        r5 = VCD pointer for VID / Temporary
#        r6 = Cache Statistics pointer / Temporary
#        r7 = Temporary register
#        r8 = Temporary register
#        r9 = Temporary register
#       r10 = VID array pointer
#       r11 = TRUE (constant for valid VID is VID array)
#       r12 = VID List pointer
#       r13 = Number of VIDs to process
#       r14 = Maximum allowed VID + 1
#       r15 = MRP Parameters pointer
#
#        g0 = Parameter passing / return status
#        g1 = VID being processed / failing VID
#
#******************************************************************************
#
WC$InvalidateBE:
        mov     g0,r15                  # Save Parameter pointer
        ldconst MAXVIRTUALS,r14         # r14 = maximum VID + 1
        ldconst TRUE,r11                # VID Valid in VID array
#
# --- Turn off the Background Flush Task before starting so that no new flush
#       ops are generated while the BE Write cache is being invalidated.
#
        lda     C_ca,r6                 # r6 = The Cache Statistics Pointer
        ld      C_haltBackgroundFlag,r5 # r5 = Background Halt Flag
        ldob    ca_status(r6),r4        # r4 = Global Cache Status
        setbit  hbginvbe,r5,r5          # Show Invalidate BE is halting BG flush
        setbit  ca_halt_background,r4,r4 # Halt the Background Flush Task
        st      r5,C_haltBackgroundFlag # Save new Halt Background Flag
        stob    r4,ca_status(r6)        # Save new status
#
# --- Determine if this is a global invalidate or list of VIDs invalidate
#       and handle with the appropriate code.
#
        ldob    mib_op(g0),r3           # r3 = Options flag
        cmpobne miboinvalvid,r3,.wc_invbe500 # Jif it is not a VID list
#
# --- List of VIDs to invalidate ----------------------------------------------
#
#   If the cache has not been initialized yet, need to delete directly from
#       the cache.  If cache is in the process of being initialized, wait for
#       the initialization to complete before continuing as normal.
#
        ldos    mib_nvids(r15),r13      # r13 = Number of VIDs to process
        lda     mib_vidlist(r15),r12    # r12 = Pointer to VID list
        ldconst 0,r4                    # r4 = Offset into VID list
        ldos    K_ii+ii_status,r7       # r7 = Initialization status
        cmpobe  0,r13,.wc_invbe810      # Jif no VIDs requested - good status
        bbc     iimpfound,r7,.wc_invbe400 # Jif Cache init has not begun
        bbs     iicinit,r7,.wc_invbe100 # Jif Cache init has completed
#
        ldconst deinitinprog,g0         # g0 = Return code - Init in progress
        ldconst 0xFFFF,g1               # Show invalid VID number
        b       .wc_invbe900            # Report the problem
#
#   Verify all the VIDs in the list are valid
#
.wc_invbe100:
        cmpobe  r4,r13,.wc_invbe200     # Jif all VIDs tested good
        ldos    (r12)[r4*2],g1          # g1 = VID being tested
        cmpobge g1,r14,.wc_invbe190     # Jif the VID is too big
        ld      vcdIndex[g1*4],r5       # r5 = VCD pointer
        cmpobe  0,r5,.wc_invbe190       # Jif undefined
        addo    1,r4,r4                 # Point to the next VID in the VID list
        b       .wc_invbe100            # Check the next VID in the VID list
#
#   An invalid VID was found, report the error to the requester and return
#
.wc_invbe190:
        ldconst deinvvid,g0             # Show that an Invalid VID was found
                                        # g1 = VID that failed the test
        b       .wc_invbe900            # return the status
#
#   VID List tested good, create the VID array for the BE to build the trees
#       for, create the BE VID Write Cache Trees, and then delete any
#       data for those specified VIDs
#
.wc_invbe200:
c       r10 = s_MallocC(r14, __FILE__, __LINE__); # Allocate space for one byte / possible VID array
        ldconst 0,r4                    # r4 = Reset offset into VID list
.wc_invbe220:
        cmpobe  r4,r13,.wc_invbe240     # Jif all VIDs have been processed
        ldos    (r12)[r4*2],g0          # g0 = VID to invalidate data for
        stob    r11,(r10)[g0*1]         # Show the VID is valid to restore tree
        addo    1,r4,r4                 # Point to the next VID
        b       .wc_invbe220            # Go process the next VID
#
.wc_invbe240:
        ldconst 0,g0                    # g0 = VID List provided
        mov     r10,g1                  # g1 = Pointer to VID array
        call    wc$processMirroredTags  # Go build the VID Write Cache Trees
#
        ldconst 0,r4                    # r4 = Reset offset into VID list
.wc_invbe300:
        cmpobe  r4,r13,.wc_invbe800     # Jif all VIDs have been processed
        ldos    (r12)[r4*2],g0          # g0 = VID to invalidate data for
        ldconst 0,g1                    # g1 = Del all Tags no matter the state
        call    WC$deleteVIDdata        # Do the delete (if any exists)
                                        # Output g0 = TRUE (all deleted)
        addo    1,r4,r4                 # Point to the next VID
        b       .wc_invbe300            # Go process the next VID
#
#   Cache is not initialized yet, nor in the process of being initialized.  Go
#       through the cache and delete any data for the VIDs noted.  Do this by
#       building a byte field list for each possible VID and then setting to
#       TRUE each requested VID.  Then walk through all the tags and any that
#       have a matching VID will have the tag cleared.
#
.wc_invbe400:
c       r10 = s_MallocC(r14, __FILE__, __LINE__); # Allocate space for one byte / possible VID array
.wc_invbe420:
        cmpobe  r4,r13,.wc_invbe440     # Jif all VIDs have been processed
        ldos    (r12)[r4*2],g1          # g1 = VID to invalidate data for
        cmpobge g1,r14,.wc_invbe430     # Jif the VID is too large
        stob    r11,(r10)[g1*1]         # Show the VID is valid to delete
        addo    1,r4,r4                 # Point to the next VID
        b       .wc_invbe420            # Go process the next VID
#
#   VID is too large.  Free the VID array and return the error.
#
.wc_invbe430:
c       s_Free(r10, r14, __FILE__, __LINE__);
        b       .wc_invbe190            # Report the error
#
#   VID flags have been created for each possible VID, now clear any tags that
#       are being requested to be deleted.
#
.wc_invbe440:
        PushRegs(r4)                    # Save 'g' registers
        ld      WctAddr,r5              # r5 = Pointer to cache tag
        ldconst tgsize,r7               # r7 = Tag incrementer value
        ld      WctSize,r6              # r6 = Size of the cache tag area
        lda     BE_ADDR_OFFSET(r5),r5   # Point to the BE memory
        addo    r5,r6,r6                # r6 = End of cache tag area
.wc_invbe460:
        ldos    tg_vid(r5),r8           # r8 = VID associated with this tag
        cmpobge r8,r14,.wc_invbe470     # Jif VID is too big - ignore.  It will
                                        #  be cleaned up later in the Cache
                                        #  initialization routines
        ldob    (r10)[r8*1],r3          # r3 = Flag for this byte
        cmpobne r11,r3,.wc_invbe470     # Jif this VID is not being invalidated
#
#       Tag found that matches the VID, now clear it out and mirror it
#
        mov     r5,g0                   # g0 = Tag to initialize
        call    WC_initTag
        lda     -BE_ADDR_OFFSET(r5),g0  # g0 = Destination (local NV)
        mov     r5,g1                   # g1 = Source (BE)
        ldconst MIRROR_BE,g2            # g2 = Mirror local BE
        mov     r7,g3                   # g3 = Length to transfer
        call    WC_CopyNVwWait          # Copy to local NV Memory
#
#       Go to the next tag and continue processing until all done
#
.wc_invbe470:
        addo    r7,r5,r5                # Point to the next Tag
        cmpobl  r5,r6,.wc_invbe460      # Jif more tags to process
        PopRegsVoid(r4)                 # Restore 'g' registers
#
#   All done with invalidating the FE VID List.  Free the VID array and
#       complete the command
#
        b       .wc_invbe800            # Show completed good
#
# --- Global Invalidation of Write Cache --------------------------------------
#
#   If the cache has not been initialized yet, need to delete directly from
#       the cache.  If cache is in the process of being initialized, return the
#       error saying that Cache is being initialized (cannot wait due to log
#       events in init that can require user input (another MRP))
#
.wc_invbe500:
        ldos    K_ii+ii_status,r7       # r7 = Initialization status
        bbc     iimpfound,r7,.wc_invbe600 # Jif Cache init has not begun
        bbs     iicinit,r7,.wc_invbe520 # Jif Cache init has completed
#
        ldconst deinitinprog,g0         # g0 = Return code - Init in progress
        ldconst 0xFFFF,g1               # Show invalid VID number
        b       .wc_invbe900            # Report the problem
#
# --- Build the Write Cache Trees for all valid VIDs in the BE and then delete
#       any data found in them.
#
.wc_invbe520:
c       r10 = s_MallocC(r14, __FILE__, __LINE__); # Allocate space for one byte / possible VID array
#
        mov     1,g0                    # g0 = Global Invalidate (all BE VIDs)
        mov     r10,g1                  # g1 = Pointer to VID array
        call    wc$processMirroredTags  # Go build the VID Write Cache Trees
                                        #  and flag in the Array those found
#
        ldconst -1,r4                   # r4 = Reset offset into VID array
                                        #  (init to -1 because next loop
                                        #  pre-increments)
.wc_invbe540:
        addo    1,r4,r4                 # increment the VID array pointer (VID)
        cmpobe  r4,r14,.wc_invbe800     # Jif no more VIDs to check
        ldob    (r10)[r4*1],r5          # r5 = Valid VID flag
        cmpobne r11,r5,.wc_invbe540     # Jif not a valid VID
#
#   VID found, delete any data for it.
#
        mov     r4,g0                   # Input g0 = VID to delete data for
        ldconst 0,g1                    # g1 = Del all Tags no matter the state
        call    WC$deleteVIDdata        # Do the delete (if any exists)
                                        # Output g0 = TRUE (all deleted)
        b       .wc_invbe540            # Continue deleting data
#
#   Cache is not initialized yet.  Need to delete the tags directly from the tag
#       area (VCDs are not set up yet).
#
.wc_invbe600:
        PushRegs(r4)                    # Save the 'g' registers
        ld      WctAddr,r5              # g0 = Pointer to cache tag
        ldconst tgsize,r7               # r7 = Tag incrementer value
        ld      WctSize,r6              # g1 = Size of the cache tag area
        lda     BE_ADDR_OFFSET(r5),r5   # Point to the BE memory
        addo    r5,r6,r6                # g1 = End of cache tag area
#
#       Clear this tag
#
.ifdef HISTORY_KEEP
c CT_HISTORY_OFF();
.endif  # HISTORY_KEEP
.wc_invbe660:
        mov     r5,g0                   # g0 = Tag to initialize
        call    WC_initTag
#
#       Go to the next tag and continue processing until all done
#
        addo    r7,r5,r5                # Point to the next Tag
        cmpobl  r5,r6,.wc_invbe660      # Jif more tags to process
.ifdef HISTORY_KEEP
c CT_HISTORY_ON();
.endif  # HISTORY_KEEP
#
#       Copy all of the tags to local BE NV Memory
#
        ld      WctAddr,g0              # g0 = Destination (Need FE Address)
        lda     BE_ADDR_OFFSET(g0),g1   # g1 = Source (Point to the BE)
        ldconst MIRROR_BE,g2            # g2 = Mirror local BE
        ld      WctSize,g3              # g3 = Length to transfer
        call    WC_CopyNVwWait          # Copy to Local NV Memory
        PopRegsVoid(r4)
        b       .wc_invbe810            # All done
#
#   All done deleting all the VID data, Free the VID array, and set up the
#       return parameters.
#
.wc_invbe800:
c       s_Free(r10, r14, __FILE__, __LINE__);
#
.wc_invbe810:
        ldconst deok,g0                 # Show the Invalidate completed OK
        ldconst 0xFFFF,g1               # Show invalid VID number
#
# --- All Done.  Allow the Background Flush task to continue as normal.
#
.wc_invbe900:

        ld      C_haltBackgroundFlag,r5 # r5 = Background Halt Flag
        lda     C_ca,r6                 # r6 = The Cache Statistics Pointer
        clrbit  hbginvbe,r5,r5          # Clear Invalidate BE is halting BG
        st      r5,C_haltBackgroundFlag # Save new Halt Background Flag
        cmpobne 0,r5,.wc_invbe910       # Jif cannot let Background Flushes go
        ldob    ca_status(r6),r4        # r4 = Global Cache Status
        clrbit  ca_halt_background,r4,r4 # Allow the Background Flush Task to
        stob    r4,ca_status(r6)        #   continue as normal
.wc_invbe910:
        ret
#
#******************************************************************************
#
#  NAME: wc$SetGlobalDisable
#
#  PURPOSE:
#       Goes through the VIDs and sets the Cache to Global Disables if no
#       data exists in any VID
#
#  DESCRIPTION:
#       The function will walk through each VID and determine if any data
#       exists in cache.  If a VID has had an error flushing the data from
#       cache to the device and the VID is in error state, also set the
#       Global Disable Pending bit in every VID and leave the Cache Global
#       Enabled.  If all data is out of cache, complete the global disable.
#
#  INPUT:
#       None
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#  REGISTER USAGE:
#        r7 = Global Cache Information
#        r8 = Global Cache Status
#       r15 = Save g0
#
#******************************************************************************
#
WC_SetGlobalDisable:
wc$SetGlobalDisable:
        mov     g0,r15                  # save g0
#
# --- Determine if Cache is enabled or not.  If not enabled - done.
#
        lda     C_ca,r7                 # r7 = the Cache Information pointer
        ldob    ca_status(r7),r8        # r8 = Cache Status
        bbc     ca_ena,r8,.wcsgd90      # Jif the cache is already disabled
        call    C$queryCacheData        # Determine if any data in or going
                                        #  to cache
                                        # Input = none
                                        # Output = g0 TRUE if data in cache
                                        #             FALSE if no data in cache
        ldob    ca_status(r7),r8        # r8 = Cache Status (may have changed)
        cmpobe  TRUE,g0,.wcsgd50        # Jif there is data in cache
#
# --- No data exists in cache, set the Global Disable Flag, and clear the
#       Global Disable In Progress
#
        clrbit  ca_ena,r8,r8            # Clear the Cache Enabled Flag
        clrbit  ca_dis_ip,r8,r8         # Clear the Cache Disable In Progress
        stob    r8,ca_status(r7)        # Save the Cache Status
#
# --- Now that all data has been flushed, mark in the DRAM that the
#       write cache has been shutdown.
#
        ldconst FALSE,g0                # Do not change Signature if not Init'd
        call    WC$markWCacheDis
#
# Report the Global Flush and Disable has completed to the MMC
#
        ldconst 0xFFFFFFFF,g0           # Show the Global Cache Complete
        call    wc$MsgFlushComplete     # Report the completion
        b       .wcsgd90                # All Done!
#
# --- Data exists in cache (or could), Set the Global Disable in Progress Flag
#
.wcsgd50:
        setbit  ca_dis_ip,r8,r8         # Set the Global Cache Disable In Prog.
        stob    r8,ca_status(r7)        # Save the new status
#
# --- If data is still in cache, the Background Flush task will call this
#       function again to see if the transition is complete yet
#
.wcsgd90:
        mov     r15,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: wc$chkrdy
#
#  PURPOSE:
#       To provide a common means of restarting incoming write cache processing
#       after stalling, when sufficient resources are again available.
#
#  DESCRIPTION:
#       If write cache ops have been previously stalled due to resources, then
#       a check is made to determine if sufficient cache tags and write
#       cache buffers are now available to perform a cached write.
#       If so, the stalled process is made ready again.
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
wc$chkrdy:
#
# --- Check if write cache processing is currently stalled due to
#     insufficient resources.  If not, then exit.
#
        ld      c_wcresourc,r8          # Load WC resources flag
        cmpobne TRUE,r8,.wcchkrdy20
#
# --- Check to see if the number of Dirty Tags exceeds the maximum allowed.
#     Note that the total number of dirty tags is the sum of the tags
#     dirty count and the tags flush in progress count.
#
        lda     C_ca,r3                 # r3 = Pointer to the counters
        ld      C_MaxDirtyTags,r4       # r4 = max total of dirty tags
        ld      ca_tagsDirty(r3),r5     # r5 = Dirty tags count
        ld      ca_tagsFlushIP(r3),r6   # r6 = Flush In Progress tags count
        addo    r5,r6,r7                # r7 = Total Dirty Tags
        cmpobge r7,r4,.wcchkrdy20       # Jif to many dirty tags, exit
#
# --- Check to see if the number of Dirty Blocks exceeds the maximum allowed.
#     Note that the total number of dirty blocks is the sum of the blocks
#     dirty count and the blocks flush in progress count.
#
        ld      C_MaxDirtyBlocks,r4     # r4 = max total number of dirty blocks
        ld      ca_blocksDirty(r3),r5   # r5 = Dirty blocks count
        ld      ca_blocksFlushIP(r3),r6 # r6 = Flush In Progress blocks count
        addo    r5,r6,r7                # r7 = Total Dirty blocks count
        cmpobge r7,r4,.wcchkrdy20       # Jif to many dirty blocks, exit
#
# --- Enough resources are now available to restart a stalled write cache op.
#     Clear a flag to indicate not stalled due to insufficient write cache resources.
#     Put the executing process in a state to wait for write cache resources and
#     exchange processes to allow a different process to execute.
#
        ldconst FALSE,r8
        st      r8,c_wcresourc          # Indicate not stalled for WC resources
#
c       TaskReadyByState(pcwcreswait);  # Ready processes waiting for wc resources
#
.wcchkrdy20:
        ret
#
#******************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
