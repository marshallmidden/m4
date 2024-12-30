# $Id: wcread.as 143007 2010-06-22 14:48:58Z m4 $
#******************************************************************************
#
#  NAME: wcread.as
#
#  PURPOSE:  Module to contain all the Write Cache Read Processing
#
#  FUNCTIONS:
#       WC$Rsubmit  - Process new read requests to see if hit or not
#       wc$ReadFullCacheHit  - Handle a full cache read hit
#       wc$RdHitComp  - Handle completion of a full cache read hit data xfer
#
#  Copyright (c) 2000-2010 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
#  NAME: WC$Rsubmit
#
#  PURPOSE: For read, verify checksum, verify data, write/verify
#           and synchronize cache I/O requests, detects any
#           overlaps with cached data.
#
#  DESCRIPTION:
#       This routine is passed read, verify checksum, verify data,
#       write/verify, and synchronize cache I/O requests directed at
#       a particular Virtual ID (VID).  It determines the following:
#
#       1).  Overlap with Dirty and/or Resident data
#       2).  For read requests, determines a complete cache "hit".
#
#       If the incoming I/O does not overlap any previously cached or the
#       read cache hit is not full hit, the return value is set to
#       FALSE to indicate the request is allowed to be immediately
#       submitted as an I/O to the back-end processor.
#
#       If the incoming I/O is a read command that completely "hits"
#       a previously cached I/O, the request is submitted to the upper
#       layer to initiate a data transfer from the "hit" cache tag.
#       The buffer address and length is calculated and wc$ReadFullCacheHit
#       is called to initiate the data transfer.
#
#       If the incoming I/O does overlaps any previously cached,
#       the return value is set to TRUE to indicate the request is
#       being handled (full read hit case) or will be handled following
#       the flushing of dirty data.  The completion routine passed to
#       flush request submits the I/O request to the  back-end processor
#       following the completion of the flush request.
#
#  CALLING SEQUENCE:
#       call    WC$Rsubmit
#
#  INPUT:
#       g1 = ilt
#
#  OUTPUT:
#       g12 = True  - Request was handled or will be handled
#             False - Overlap with data in cache did not occur
#
#  REGS DESTROYED:
#       g0 g1 g2 g3 g4 g5
#
#******************************************************************************
#     r3 = VCD pointer
#     r4 = RB tree node pointer for this I/O
#     r5 = dirtyTags
#     r8 = LSB disk start address - incoming I/O
#     r9 = MSB disk start address - incoming I/O
#    r10 = LSB last address of incoming I/O
#    r11 = MSB last address of incoming I/O
#    r12 = length of request
#    r13 = ILT of incoming request
#    r14 = the VRP function code
#    r15 = VRP pointer
#**********************************************************************
#
        .text
        .globl  WC$Rsubmit
WC$Rsubmit:
        ldconst TRUE,g12                # Show the op was/will be handled
        ld      vrvrp(g1),r15           # Get pointer to VRP
        mov     g1,r13                  # Preserve ILT pointer
#
# --- Get root value for this VID
#
        ldos    vr_vid(r15),g0          # g0 = VID
        ld      vcdIndex[g0*4],r3       # Retrieve VCD pointer for this VID
        ld      vc_cache(r3),g0         # Get the cache RB tree root
        ldos    vr_func(r15),r14        # Get the function code
#
# --- Determine if this request hits any I/Os already cached.
#
        ldl     vr_vsda(r15),r8         # Get disk starting address
        ld      vr_vlen(r15),r12        # Get length of request
        cmpdeco 0,r12,g4                # Adjust size/clear carry bit
#
# --- Check for a non-zero length.  If this is a synchronize cache
#     request, a zero length specifies a range from the starting
#     disk address to the end of the drive.
#
        bne     .wcr10                  # Jif length != 0
        cmpobne vrsync,r14,.wcr110      # Jif not Sync Cache

        ldconst 0xffffffff,r10          # Set the end range of request
        ldconst 0xffffffff,r11          #  to the maximum possible value
        b       .wcr20
#
# --- Calculate the ending lba for the overlap check.  Note that
#     length (stored in g4) has already been decrement and the
#     carry bit has been cleared.
#
.wcr10:
        addc    g4,r8,r10               # Calculate end range
        addc    0,r9,r11                #   of request
#
# --- Determine if this request hits any I/Os already cached.
#
.wcr20:
        movl    r8,g2                   # g2/g3 = Min Key value
        movl    r10,g4                  # g4/g5 = Max Key value
        call    RB$foverlap             # Check for cache hit
        cmpobe  FALSE,g1,.wcr110        # Jif no cache hit
#
# --- Check if this is a read request.
#
        cmpobne vrinput,r14,.wcr60      # Jif not read request
#
# --- Test to see if this command can be processed as a Full Cache Hit.
#     To be processed as a full cache hit, the following must be true:
#     Both the Starting and Ending LBAs are within the cache tag range,
#     and either the tag is not locked or the tag is locked for a previous
#     read full cache hit or a flush (ie- can not be locked for an Invalidate).
#
        ldl     rbkey(g1),g4
        cmpobg  r9,g5,.wcr30            # Jif MSB key is greater
#       cmpobne r9,g5,.wcr50            # Jif MSB key is not equal
        bne     .wcr50                  # Jif MSB key is not equal
        cmpobl  r8,g4,.wcr50            # Jif LSB key is less
#
# --- Start address is greater than or equal, now check if end address
#     is greater than or equal
#
.wcr30:
        ldl     rbkeym(g1),g4
        cmpobl  r11,g5,.wcr40           # Jif MSB key is less
#       cmpobne r11,g5,.wcr50           # Jif MSB key is not equal
        bne     .wcr50                  # Jif MSB key is not equal
        cmpobg  r10,g4,.wcr50           # Jif LSB key is greater
#
# --- The data for new request is contained with this tag, check
#     that the tag is not locked for an Invalidate.
#
.wcr40:
        ld      rbdpoint(g1),g0         # Get cache tag address
        ldos    tg_state(g0),g4         # Get tag state
        bbs     TG_LOCKED_INVALIDATE,g4,.wcr50    # Jif locked for invalidate
#
# --- Ensure the cache is not being disabled and if so, treat like a partial hit
#
        ldob    vc_stat(r3),r6          # r6 = Cache Status
        and     VC_NO_MORE_DATA,r6,r7   # Get the Disable and Error Bits
        cmpobne 0,r7,.wcr50             # Jif in Error or Disable Pending states
                                        #  Treat like a partial to flush data
        lda     C_ca,r4                 # Get the Global Cache Pointer
        ldob    ca_status(r4),r4        # r4 = Global Cache Status
        bbs     ca_dis_ip,r4,.wcr50     # Jif if the Global Cache is being
                                        #  disabled (flush any data)
#
# --- A Full Cache Hit can be processed. Increment the Read Hits Counter.
#     Ensure that the cache tag is locked for a Read Cache Hit
#     to prevent invalidation until the cache transfer is completed.
#
        ldl     vc_rdhits(r3),r6        # Get the Read Hits Counter
        cmpo    0,1                     # Clear the carry bit
        addc    1,r6,r6                 # Increment the Read Hits Counter
        addc    0,r7,r7
        stl     r6,vc_rdhits(r3)        # Store the Read Hits Counter
        setbit  TG_LOCKED_READ,g4,g4
        stos    g4,tg_state(g0)
#
# --- Increment the Host Read In Progress count for this cache tag.
#
        ldos    tg_rdcnt(g0),g5
        addo    g5,1,g5
        stos    g5,tg_rdcnt(g0)
#
# --- Generate the Scatter Gather List for this cache tag
#     that contains a cache hit.
#     The SGL descriptor buffer address is the starting
#     buffer address for the cache tag plus any
#     additional offset for the cache hit offset.
#     The cache hit offset is 512 times the difference between the
#     the starting lba for the read and the start of the tag.
#
#     Note that 32 bit math is OK here since the offset into
#     the cache tag is at most 64K blocks
#
#     sg_addr = tg_bufptr + (512 * (vr_vsda - tg_vsda))
#
c       g2 = (*(UINT64*)&r8 - ((TG*)g0)->vsda) * 512; # (Start LBA - VSDA)*512
        ld      tg_bufptr(g0),g5        # get buffer pointer
        addo    g2,g5,g2                # add buffer pointer to results
#
# --- The byte transfer length is 512 bytes times the length
#     of the read I/O.
#
#     sg_len = 512 * vr_vlen
#
        shlo    9,r12,g3                 # multiply length by 512
#
# --- Indicate no Scatter Gather List in VRP.
#
#     vrvrp->vr_sglptr  = 0
#     vrvrp->vr_sglsize  = 0
#
        ldconst 0,g4
        st      g4,vr_sglsize(r15)      # Clear SGL size in VRP
        st      g4,vr_sglptr(r15)       # Clear SGL pointer in VRP
#
# --- Perform the Host data transfer for the full cache hit.
#     g0 = tag, g1 = ILT,  g2 = buffer addr, g3 = length in bytes
#
        mov     r13,g1                  # g1 = ILT pointer
        st      g0,ioctag(g1)           # Store cache tag address
c       record_cache_data(FR_CACHE_READ_HIT, (void *)r15, (void *)g2);
        b       wc$ReadFullCacheHit
#
# --- The overlap detected was not a Full Cache Hit or the cache
#     tag was locked for an invalidate or flush.  Increment the
#     Read Partial Hits Counter.
#
.wcr50:
        ldl     vc_rdpart(r3),r6        # Get the Read Partial Counter
        cmpo    1,0                     # Clear the Carry bit
        addc    1,r6,r6                 # Increment the Read Partial Counter
        addc    0,r7,r7
        stl     r6,vc_rdpart(r3)        # Store the Read Partial Counter
#
# --- An overlap was detected that may need to be flushed.  Each
#     dirty overlap will be removed by the Flush Process before
#     this new request will be processed further.  If this is not a
#     write and verify request, resident data is ignored.  Loop to
#     find each dirty overlapping cache tag and queue them to the
#     Flush Process.
#
# --- Initialize the count of overlapped tags to 0.
#
.wcr60:
        ldconst 0,r5                    # clear overlapped tags count
#
# --- Load up the completion routine early
#
        lda     .wcr200,g4              # Set completion routine to perform the
        st      g4,il_cr(r13)           #  I/O request if Flush Request needs
        st      r5,iointcr(r13)         # Clear intermediate completion routine
        stob    r5,ioccode(r13)         # Clear Completion Code of future ops
#
# --- Get the cache tag address
#
.wcr70:
        ld      rbdpoint(g1),r12        # get pointer to cache tag
#
# --- Since <wc$FlushRequest> may change the RB tree, we need to find the next
#     possible overlap before invalidating the tag.  The result of the search
#     will be checked after the invalidate to see if any more tags need to
#     be invalidated.
#
        movl    r10,g4                  # g4/g5 = Max Key value
        call    RB$noverlap             # Find next overlap, if any
        mov     g1,r4                   # Preserve results of search
#
# --- If this is a write and verify request, all overlapping
#     cache tags are invalidated.
#
        cmpobne vroutputv,r14,.wcr80    # Jif not write and verify data
        ldconst INVALIDATE_REQUEST,g2   # g2 = Request Type
        b       .wcr90
#
# --- Check to see if this cache tag is Dirty.
#
.wcr80:
        ldos    tg_attrib(r12),g4        # get cache tag attrib
        bbc     TG_DIRTY,g4,.wcr100     # Jif not dirty
        ldconst FLUSH_REQUEST,g2        # g2 = Request Type
#
# --- This cache tag is Dirty.
#     Increment the number of overlapped tags that have
#     been detected
#
.wcr90:
#
# --- Queue the flush request for this overlapping cache tag.
#     Flush Request returns TRUE if request is queued, or FALSE
#     if the request is handled immediately.  Only increment the
#     count if the request is queued.
#     g0 = Cache tag, g1 = ILT, g2 = Request Type
#
        movl    r12,g0                                  # g0 = tag, g1 = ILT
        callj   wc$FlushRequest
        cmpobe  FALSE,g0,.wcr100        # Jif the Flush is already done
        addo    r5,1,r5                 # increment overlapped tag count
#
# --- Test for another overlap and loop if more exist.
#
.wcr100:
        mov     r4,g1                   # g1 = next RB tree node
        cmpibne FALSE,g1,.wcr70         # Jif overlap detected
#
# --- Finished detecting all the overlaps.
#     Check to see an any overlaps were flush requests.  If so, then
#     store the number of overlaps that were detected in the ILT.
#     This count will be decremented as each overlapping tag is
#     flushed.  When the count reaches 0, the completion
#     routine will be called to resume processing of this
#     new I/O request.
#
        cmpibe  0,r5,.wcr120            # Jif dirty tag count is zero
        stos    r5,iocounter(r13)       # Store Overlapped Tag count in ILT
        ldconst TRUE,g12                # Show the op will be handled
        ret
#
# --- This new I/O Request did not overlap write cache data.
#     If this is a read command, increment the Read Miss Counter.
#
.wcr110:
        cmpobne vrinput,r14,.wcr120     # Jif not read request
        ldl     vc_rdmiss(r3),r6        # Get the Read Miss Counter
        cmpo    1,0                     # Clear the Carry Bit
        addc    1,r6,r6                 # Increment the Read Miss Counter
        addc    0,r7,r7
        stl     r6,vc_rdmiss(r3)        # Store the Read Miss Counter
#
# --- Restore the ILT pointer and return FALSE to indicate a cache
#     hit was not detected
#
.wcr120:
        mov     r13,g1                  # g1 = ILT
        mov     FALSE,g12               # g12 = No cache hit
        ret
#
# --- Flush Request Complete Handler ------------------------------------------
#
#       g0 = Completion Code
#       g1 = ILT
#
.wcr200:
        cmpobe  0,g0,.wcr210            # Jif there was no error
        b       wc$complete_io          # Error occurred during the flush -
                                        #   Report the error on this op
                                        #   (g0 and g1 already set up)
.wcr210:
        b       C$do_nc_op              # Flush Completed OK - Handle as a
                                        #  non-cached op
#
#******************************************************************************
#
#  NAME: wc$ReadFullCacheHit
#
#  PURPOSE: Perform the Host data transfer for a full cache hit.
#
#  DESCRIPTION:
#       This routine allocates an SRP and queues the request back to the
#       next upper layer to transfer the read data to the host.
#
#  CALLING SEQUENCE:
#       call   WC$ReadFullCacheHit
#
#  INPUT:
#       g0 = cache tag ptr
#       g1 = ilt
#       g2 = buffer address
#       g3 = buffer length
#
#  OUTPUT:
#       g12 = True  - Read was handled
#
#  REGS DESTROYED:
#       g4
#
#******************************************************************************
#
        .globl  wc$ReadFullCacheHit
wc$ReadFullCacheHit:
        movq    g0,r12                  # Save g0-g3
#
# --- Allocate ILT/SRP and initiate data transfer from local buffer to host
#
        ldconst src2h,g4                # Set up the SRP function (to host)
        call    c$asrp                  # Allocate read ILT/SRP
#
# --- If this is a Resident Tag, move the tag from its current position in the
#      LRU queue to the MRU position
#
        ldos    tg_attrib(r12),r4       # Get tag attribute
        bbc     TG_RESIDENT,r4,.wcrfch_10 # Jif this Tag is not resident
        mov     r12,g0                  # Set up the call to move to the MRU
                                        #  spot on the LRU queue
        call    wc$MoveToResidentMRU    # Move this tag to MRU position of LRU
.wcrfch_10:
#
# --- Initiate data transfer from local buffer to host
#
        ld      il_w3(g1),g1            # g1 = ILT of SRP
        lda     wc$RdHitComp,g2         # g2 = completion routine
#
# --- Call the layer above Cache to handle the SRP
#
        call    c$callupper             # Issue the request to send the data
#
# --- Adjust outstanding read SRP count
#
        ld      C_orsrpc,r3             # Load, Increment, and Store the
        addo    1,r3,r3                 #  outstanding read SRP count
        st      r3,C_orsrpc
#
# --- Exit
#
        movq    r12,g0                  # Restore g0-g3
        ldconst TRUE,g12                # Show the read op was handled
        ret
#
#******************************************************************************
#
#  NAME: wc$RdHitComp
#       To provide a means of handling the completion of a cached
#       read transfer from the read buffer to the host.
#
#  DESCRIPTION:
#       The ILT/SRP combination is released back to the system.
#       The parent ILT is completed back to the originator.
#
#  CALLING SEQUENCE:
#       call   wc$RdHitComp
#
#  INPUT:
#       g0 = SRP Completion Status
#       g1 = ILT
#
#  OUTPUT:
#       none
#
#  REGS DESTROYED:
#       none
#
#******************************************************************************
#
        .globl  wc$RdHitComp
wc$RdHitComp:
        movl    g0,r14                  # Save g0-g1
#
# --- Release read ILT/SRP
#
        ld      il_w3(g1),r13           # Get parent ILT
        call    c$rsrp                  # Release read ILT/SRP
#
# --- Adjust outstanding read SRP count
#
        ld      C_orsrpc,r3             # Load, Decrement, and Store the
        subo    1,r3,r3                 #  outstanding read SRP count
        st      r3,C_orsrpc
#
# --- Decrement the Host Read In Progress count for this cache tag.
#
        ld      ioctag(r13),g0          # Get cache tag address
        ldos    tg_rdcnt(g0),r5
        subo    1,r5,r5                 # Decrement in progress count
        stos    r5,tg_rdcnt(g0)

        cmpibne 0,r5,.rdhc10            # check if count is zero
#
# --- All reads on this cache tag have completed.  Unlock the tag.
#
        ldos    tg_state(g0),r12         # Get tag state
        clrbit  TG_LOCKED_READ,r12,r12   # Clear locked read bit
        stos    r12,tg_state(g0)
#
# -- Unlock any ops waiting for this op to complete
#
        call    wc$unlock_tag           # Start any pending ops

.rdhc10:
#
# --- Complete request
#
        mov     r14,g0                  # Get the VRP completion status
        ld      vrvrp(r13),r5           # Get the VRP
        stob    g0,vr_status(r5)        # Store the VRP completion status code
        mov     r13,g1                  # Restore the VRP ILT
        call    K$comp                  # Complete this request
#
# --- Adjust outstanding request count
#
        ld      C_orc,r3                # Load, Decrement, and Store the
        subo    1,r3,r3                 #  outstanding request count
.ifdef M4_DEBUG_C_orc
c CT_history_printf("%s%s:%u: C_orc starts at %lu, ends at %lu\n", FEBEMESSAGE,__FILE__, __LINE__, C_orc, r3);
.endif  # M4_DEBUG_C_orc
        st      r3,C_orc
#
# --- Exit
#
        movl    r14,g0                  # Restore g0-g1
        ret
#
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
