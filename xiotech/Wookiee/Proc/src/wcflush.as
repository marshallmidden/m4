# $Id: wcflush.as 162911 2014-03-20 22:45:34Z marshall_midden $
#******************************************************************************
#
#  name: Write Cache Flush
#
#  PURPOSE:
#       To provide Write Cache Flush related functions.
#
#  EXTERNAL FUNCTIONS:
#       WC$FlushWOMP       - Flush FE Write Cache WithOut Mirror Partner
#       WC$FlushBE         - Flush BE Write Cache for a VID, several VIDs, or
#                            all the VIDs
#
#  FUNCTIONS:
#       wc$FlushRequest    - Interface to receive Flush or Invalidate Requests
#       wc$FlushTask       - Task for flushing dirty overlap data to disk
#       wc$BgFlushTask     - Task for background flush of dirty cache data
#       wc$FlushCoalesce   - Utility to coalesce dirty cache tags during flush
#
#  Copyright (c) 2000-2010 Xiotech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  WC$FlushWOMP            # Flush FE WithOut Mirror Partner
        .globl  WC$FlushBE              # Flush BE Write Cache
#
# --- global data declarations ----------------------------------------
#
        .globl  gWCPowerUpFlushBEFlag   # Flush BE during Power Up before CINIT
#
# --- Local data definitions ------------------------------------------
#
        .data
        .align  2
wc_flush_inv_cnt:
        .word   0                       # Flush and invalidate counter
#
gWCPowerUpFlushBEFlag:
        .word   0                       # Flush BE during Power up before CINIT
#
# --- Executable code ---------------------------------------------------------
#
        .text
#
#******************************************************************************
#
#  NAME: wc$FlushRequest
#
#  PURPOSE:
#       To handle a request to either Flush or Invalidate the input cache tag.
#
#  DESCRIPTION:
#       This routine is called by either of the functions that handle a new
#       read or write request for the write cache after an overlap with a dirty
#       cache tag has been detected.  This routine is called with the
#       address of the original ILT, the address of the overlapping cache
#       tag, and the type of request (Invalidate or Flush).  The caller is
#       expected to have stored the number of overlaps that exists for this cache
#       tag in the ILT.  The count will be decremented when the flush/invalidate
#       of this cache tag is completed.  If the number of overlaps is decremented
#       to zero, the ILT completion routine will be invoked.  It will handle all
#       aspects of the Flush or Invalidate request including allocation of a
#       ILT/Placeholder, dealing with cache tag locked conditions, and invoking
#       a mirror of the cache tag if it is invalidated.
#
#  CALLING SEQUENCE:
#       call  wc$FlushRequest
#
#  INPUT:
#       g0 = Cache tag address
#       g1 = ILT address that overlap the cache tag
#       g2 = Request Type (INVALIDATE_REQUEST or FLUSH_REQUEST)
#
#  OUTPUT:
#       g0 = Return Code, TRUE  = Request has been queued up, need ILT CR
#                         FALSE = Request has been handled immediately, done
#
#  REGS DESTROYED:
#       g0 - g2
#
#******************************************************************************
#
# unsigned int FlushRequest(CACHE_TAG_PTR tag, ILT_PTR ilt, unsigned int reqType)
#
#   PH_ILT_PTR      philt;
#
        .globl  wc$FlushRequest
wc$FlushRequest:
#
# --- Save ILT in r8.
#     Load tag state locked bits into r12.
#
        mov     g1,r8                   # r8 = ILT
!       ldos    tg_state(g0),r12        # Load tag state
#
# --- Check if this is an Invalidate Request and the tag is Resident and is not
#     locked.  For this case the invalidation can occur immediately to allow
#     the overlapping command to proceed immediately.
#
#   if (reqType == INVALIDATE_REQUEST)
#       if ((tag->attrib & TG_RESIDENT) &&
#           (!(tag->state & TG_LOCKED)))
#
        cmpobne INVALIDATE_REQUEST,g2,.L878 # Jif reqType != INVALIDATE_REQUEST
!       ldos    tg_attrib(g0),r13       # Load attribute for cache tag
        bbc     TG_RESIDENT,r13,.L878   # Jif tag attribute is not Resident
        cmpobne 0,r12,.L878             # Jif locked for any reason
#
        call    wc$invalidate_tag       # Invalidate this Resident Tag
        ldconst FALSE,g0                # Return FALSE
        ret                             # Exit
#
.L878:
# --- Can not handle this request immediately.
#     Allocate an ILT/Placeholder and initialize its contents.
#
#   philt = AllocatePlaceholder();
#   philt->ilt = ilt;
#   philt->tag = tag;
#   philt->type = reqType;
#
c       g1 = get_wc_plholder();         # Get Placeholder ILT.
        st      r8,plilt(g1)            # philt->ilt = ilt  plilt
        st      g0,plctag(g1)           # philt->tag = tag  plctag
        st      g2,plintent(g1)         # philt->type = reqType  plintent

#
# --- If the tag is locked for either an invalidate, flush, or Write than
#     nether an invalidate request or a flush request can be queue
#     for flush, so add this request to the locked queue.
#     Also add this request to the locked queue if this is an invalidate
#     request and the tag is locked for a read cache hit.
#
#   if ((tag->state & TG_LOCKED_NOFLUSH ) ||
#      ((reqType == INVALIDATE_REQUEST) && (tag->state & TG_LOCKED_READHIT)) )
#
#       ldos    tg_state(g0),r12    (above)
        cmpobe  0,r12,.L884             # Jif tag not locked, continue flush
        and     r12,TGM_LOCKED_NOFLUSH,r15 # Test locked for No Flush bits
        cmpobne 0,r15,.L885             # Jif locked for No Flush
        cmpobne INVALIDATE_REQUEST,g2,.L884 # Jif reqType != Invalidate Req
#
.L885:
#
# --- Tag is locked with respect to this request so add an entry to the
#     Locked Queue for this cache tag and exit.
#
#        philt->completionRoutine = ContFlushRequest; /* ENTRY 1 */
#        QueueLocked(tag, philt);
#        return(TRUE);
#
        lda     wc$ContFlushRequest,r15#  Address to resume execution at
        st      r15,plcr(g1)            # Save completion routine address
        call    wc$qtag_unlock          # Add the Placeholder to the Locked Q
        ldconst TRUE,g0                 # return TRUE
        ret                             # Exit
#
# --- The tag is not locked (or no longer locked ) with respect to this request.
#     Continue with the Flush or Invalidate Request.
#
#     g0 = philt when invoked by completion routine
#
wc$ContFlushRequest:
.L884:
#
# --- Set g0 (completion Code) to 0 (completed OK) in case the tag is no longer
#       dirty or not locked for this type of request
#
        ldconst 0,g0
#
# --- Load the cache tag pointer for this Placeholder
#
#   tag = philt->tag;
#
        ldl     plctag(g1),r4           # r4 = philt->tag  (plctag)
                                        # r5 = philt->type (plintent)
#
# --- Check if the cache tag is still dirty.  If not, go to the
#     the completion portion of this routine.
#
#   if (tag->attrib & TG_DIRTY)
#
!       ldos    tg_attrib(r4),r15       # Load attribute bits
        bbc     TG_DIRTY,r15,.L928      # Jif not dirty
#
# --- The cache tag is still Dirty.
#     Lock the tag for either an Invalidate or a Flush depending
#     on the request type.
#
#       if (philt->type == INVALIDATE_REQUEST)
#           tag->state |= TG_LOCKED_INVALIDATE;
#       else
#           tag->state |= TG_LOCKED_FLUSH;
#
!       ldos    tg_state(r4),r6         # Load tag->state
        cmpo    INVALIDATE_REQUEST,r5   # Check if tag->state = Invalidate
        sele    TGM_LOCKED_FLUSH,TGM_LOCKED_INVALIDATE,r15 # Sel lock bit
        or      r15,r6,r6               # Or in Flush or Invalidate Lock bit
!       stos    r6,tg_state(r4)         # Store tag->state
#
# --- Initialize the Placeholder completion routine address to the step
#     to resume at.  The Queue the request to the Flush Task and exit.
#
#       philt->completionRoutine = CompleteFlushRequest; /* ENTRY 2 */
#       QueueFlush(tag, philt);
#
        lda     wc$CompleteFlushRequest,r15
        st      r15,il_cr(g1)           # philt->cr = wc$CompleteFlushRequest
        mov     r4,g0                   # g0 = tag
        call    wc$qtag_flush           # Queue the Flush Request
        ldconst TRUE,g0                 # return TRUE
        ret                             # Exit
#
wc$CompleteFlushRequest:
.L928:
# --- This tag has been flushed to disk or is no longer dirty.
#     Load the ILT pointer, Cache Tag pointer, and request type
#     from the Placeholder
#
#   ilt = philt->ilt;
#
#   g0 = completion code
#   g1 = placeholder ilt
#


        ldt     plilt(g1),r4            # r4 = philt->ilt  (plilt)
                                        # r5 = philt->tag  (plctag)
                                        # r6 = philt->type (plintent)
        # is tag still dirty - if so flush probably failed
!       ldos    tg_attrib(r5),r15       # Load attribute bits
        bbc     TG_DIRTY,r15,.wcfr290   # Jif not dirty
        ldconst ecbusy,g0               # set error code to busy
.wcfr290:
        mov     g0,r13                  # r13 = completion code for this op
#
# --- If there was an error returned from the flush, save away the completion
#       code if there was not a previous error already logged.
#
        cmpobe  0,g0,.wcfr300           # Jif this op completed OK
        ldob    ioccode(r4),r8          # r8 = previous completion code
        cmpobne 0,r8,.wcfr300           # Jif there was a previous error
        stob    g0,ioccode(r4)          # Save this ops completion code
#
.wcfr300:
        cmpobne INVALIDATE_REQUEST,r6,.L965 # Jif not Invalidate Request
        cmpobne 0,g0,.wcfr320           # Jif if there was an error on this op
                                        #  and clear the locked bit
        mov     r5,g0                   # g0 = tag (input to functions below)
!       ldos    tg_attrib(r5),r15       # Load Attribute bits
        bbc     TG_RESIDENT,r15,.L965   # Jif not Resident
        call    wc$invalidate_tag       # Still Resident, Invalidate tag
.wcfr320:
        mov     r5,g0                   # g0 = tag to unlock
        mov     g1,r8                   # Save g1 (unloack_tag may call CR)
!       ldos    tg_state(r5),r15        # Load State bits
        clrbit  TG_LOCKED_INVALIDATE,r15,r15  # Unlock tag for Invalidate
!       stos    r15,tg_state(r5)        # Store State bits
        call    wc$unlock_tag           # Process the Tag Locked Queue
        mov     r8,g1                   # Restore g1
#
.L965:
# --- Release the Placeholder ILT
#
#   ReleasePlaceholder(philt);
#
c       put_wc_plholder(g1);            # Release the placeholder ILT
#
# --- Call the Intermediate completion routine if there is one.  Then decrement
#     the Overlapped Tags count in the ILT and check to see if it becomes zero.
#     If it is zero, then call the ILT completion routine.
#
        ld      iointcr(r4),r7          # r7 = Intermediate completion routine
        mov     r4,g1                   # g1 = ILT Address
        cmpobe  0,r7,.wcfr360           # Jif no intermediate completion routine
        mov     r13,g0                  # restore Completion Code for this op
        callx   (r7)                    # Call intermediate completion routine
#
.wcfr360:
        ldos    iocounter(r4),r15       # Load Overlapped Tag count from ILT
        cmpdeco 1,r15,r15               # Decrement Overlapped Tag count
        stos    r15,iocounter(r4)       # Store Overlapped Tag count in ILT
        bne     .L969                   # Jif overlapped tag count not zero
        ld      il_cr(r4),r7            # Load completion routine address
        ldob    ioccode(r4),g0          # g0 = completion code
        callx   (r7)                    # Call completion routine
#
.L969:
# --- Exit with a TRUE return code
#
        ldconst TRUE,g0                 # return TRUE
        ret                             # Exit
#
#******************************************************************************
#
#  NAME: WC$FlushTask
#
#  PURPOSE:
#       To process entries in the Flush Queue and submit ILTs to the Virtual
#       layer to write these dirty cache tags to disk.
#
#  DESCRIPTION:
#       This task removes entries from the head of the Flush Queue and submits
#       ILTs to the Virtual layer to flush the requested dirty cache tags.
#       This queue only contains cache tags that were requested for a flush
#       by wc$FlushRequest due to an overlap condition.  The write operation
#       for this type of flush is priority high.  However, this task
#       does still attempt to coalesce these tags with any other sequential
#       dirty cache tags which may or may not have already been requested for
#       a flush.
#
#  CALLING SEQUENCE:
#       Not called, invoked by Kernel
#
#  INPUT:
#       Entries from FlushQueue
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       Possibly all G registers.
#
#******************************************************************************
#
# void wc$FlushTask(void)
#
#   PH_ILT_PTR      philt;
#   ILT_PTR         ilt;
#   VRP_PTR         vrp;
#   SGL_PTR         sglPtr;
#   unsigned int    sglCount;
#   unsigned int    ioLength;
#   unsigned int    endLba;
#   CACHE_TAG_PTR   tag, lastTag;
#   RB_NODE_PTR     node;
#
        .globl  WC$FlushTask
WC$FlushTask:
#
# --- Setup queue control block
#
        ld      K_xpcb,r14
        lda     c_wflushq,r15           # Setup queue address
        st      r14,qu_pcb(r15)         # Set pcb (leave Queue head and tail
                                        #  alone since Flush Requests could
                                        #  already be queued up)
        b       .wcft20
#
.wcft10:
        ldconst pcnrdy,r3
        stob    r3,pc_stat(r14)         # Set task to not ready
#
.wcft20:
        call    K$qxchang
#
# --- Branch to start of while loop test.
#
        b       .L996                   # jump to while loop test
#
.L997:
# --- Start of while loop to process each entry in the Flush Queue
#
#     Get the next entry from the Head of the Flush Queue
#     and unlink it from the queue.
#
#       philt = flushQueueHead;
#       flushQueueHead = philt->fthd;
#       if (flushQueueHead == NULL)
#           flushQueueTail = NULL;
#
                                        # g1 = philt = flushQueueHead
        ld      qu_qcnt(r15),r6         # get the Queue Count
        ld      il_fthd(g1),r5          # flushQueueHead = philt->fthd
        subo    1,r6,r6                 # decrement number left
        st      r5,qu_head(r15)         # Store into Flush Head
        st      r6,qu_qcnt(r15)         # save the number of items left
        cmpobne 0,r5,.L998              # if (flushQueueHead == NULL)
        st      r5,qu_tail(r15)         # Flush Tail = NULL
#
.L998:
# --- Get the pointers to the cache tag and RB node for this placeholder
#
#   tag = philt->tag
#   node = tag->dirtyRB
#
        ld      plctag(g1),g0           # g0 = tag
!       ld      tg_dirtyptr(g0),g2      # g2 = node
#
# --- If a cache flush has already been queued to the virtual layer
#     for this cache tag, then put the Placeholder back on the
#     Locked Queue to wait for the flush to be completed.
#     If this placeholder was for an invalidate request, then clear
#     the lock for the invalidate (it remains locked for the flush).
#     Note that the wc$unlock_tag routine does not need to be called
#     for this case. Also note that the completion routine for the
#     placeholder is still set correctly.
#
#       if (tag->attrib & TG_FLUSHIP)
#           if (philt->type == INVALIDATE_REQUEST)
#               tag->state &= ~TG_LOCKED_INVALIDATE;
#           QueueLocked(tag, philt);
#
!       ldos    tg_attrib(g0),r12       # load tag attribute bits
        bbc     TG_FLUSHIP,r12,.L1000   # Jif not flush in progress
        ld      plintent(g1),r12        # load placeholder request type
        cmpobne INVALIDATE_REQUEST,r12,.L1002 # Jif not invalidate request
!       ldos    tg_state(g0),r12        # load tag state locked bits
        clrbit  TG_LOCKED_INVALIDATE,r12,r12 # clear locked for invalidate
!       stos    r12,tg_state(g0)        # store tag state locked bits
.L1002:
                                        # input tag in g0
                                        # input placeholder in g1
        call    wc$qtag_unlock          # Add the placeholder to the locked Q
        b   .L996                       # jump to while loop test
#
.L1000:
# --- This tag is not actively being flushed to disk
#     If the cache tag is not dirty (i.e - Resident or Free)
#     a flush is no longer needed for this tag.
#     Then call the completion routine for the Placeholder ILT.
#
#       else if (!(tag->attrib & TG_DIRTY))
#           philt->completionRoutine(philt)
#
        bbs     TG_DIRTY,r12,.L1005     # Jif tag is dirty
        ld      plcr(g1),r12            # load the completion routine address
        ldconst 0,g0                    # g0 = Good Completion Code
        callx   (r12)                   # call the completion routine
        b       .L996                   # jump to while loop test
#
.L1005:
# --- Else the Cache Tag is still dirty and a flush is not started yet.
#     Attempt to coalesce other dirty cache tags to this tag and build
#     and submit an ILT to the Virtual layer.
#
#       else
#           FlushCoalesce(tag, philt, node, CR)
#           Outputs (node, iolength)
#
                                        # Input g0 = tag
                                        # Input g1 = philt
                                        # Input g2 = node
        lda     wc$FlushIOComplete,g3   # Input g3 = Completion Routine
        call    wc$FlushCoalesce        # Attempt to coalesce to this dirty tag
#
.L996:
# --- Loop to process each entry in the Flush Queue
#     Check to see if there are any entries to process.
#     If so, go to the start of the loop.
#
#   while (flushQueueHead != NULL)
#
        ld      qu_head(r15),g1         # g1 = Head of the Flush Requests
        cmpobne 0,g1,.L997              # Jif flushQueueHead != NULL
#
        b       .wcft10
#
#******************************************************************************
#
#  NAME: wc$FlushCoalesce
#
#  CALLING SEQUENCE:
#       Call    wc$FlushCoalesce
#
#  INPUT:
#       g0 = tag (First tag to attempt coalesce to)
#       g1 = ILT/Placeholder (If called from FlushTask)
#       g2 = node (RB Tree node corresponding to tag)
#       g3 = Desired Completion Routine address
#
#  OUTPUT:
#       g0 = I/O Transfer length for ILT submitted to Virtual Layer
#       g2 = node (next node to continue from for Background Flush)
#       g4/g5 = Ending LBA + 1 for flush submitted to Virtual Layer
#
#  REGS DESTROYED:
#       g0 - g7
#
#       r3 = ILT/Placeholder (If called from FlushTask)
#       r4 = Desired Completion Routine address
#       r5 = Virtual ID
#       r6 = Pointer to current RB-tree entry (current entry to process)
#       r7 = Pointer to current cache tag data structure
#       r8 = Global Cache Status
#       r9 = Number of SGL entries
#       r10 = Transfer Length of I/O
#       r11 = Pointer to ILT
#       r12 = LSB of Ending LBA+1 for I/O
#       r13 = MSB of Ending LBA+1 for I/O
#       r14 = lastTag
#       r15 = Buffer pointer
#
#******************************************************************************
#
# void FlushCoalesce(CACHE_TAG_PTR tag, PH_ILT_PTR philt, RB_NODE_PTR node, CR)
#
#   ILT_PTR         ilt;
#   VRP_PTR         vrp;
#   SGL_PTR         sglPtr;
#   unsigned int    sglCount;
#   unsigned int    ioLength;
#   unsigned int    endLba;
#   CACHE_TAG_PTR   tag, lastTag;
#   RB_NODE_PTR     node;
#
wc$FlushCoalesce:
#
# --- Save the inputs in local registers
#
        mov     g0,r7                   # r7 = tag
        mov     g1,r3                   # r3 = Placeholder
        mov     g2,r6                   # r6 = node
        mov     g3,r4                   # r4 = Completion Routine address
#
# --- Increment the Flush Op Counter to keep track of outstanding Ops
#
        ld      C_flush_orc,r11         # r11 = Flush Op Counter
        addo    1,r11,r11               # Increment the counter and save
        st      r11,C_flush_orc
#
# --- Allocate an ILT and VRP for this I/O.
#     Outputs: g1 = Ptr to ITL
#              g2 = Ptr to VRP
#   ilt = AllocILT();
#   vrp = AllocVRP();
#
        call    M$aivw
        mov     g1,r11                  # r11 = Ptr to ILT
#
#     Load the first cache tag entry into r12-r15.
#     This will also initialize the Ending LBA which is stored in r12/r13.
#     The transfer length for each cache tag will be added to the Ending LBA
#     in the loop below while coalescing tags together.
#
#   endLba = tag->vsda;
#
!       ldl     tg_vsda(r7),r12         # r12/r13 = Starting LBA
!       ld      tg_vlen(r7),r14         # r14 = Transfer Length (block count)
!       ld      tg_bufptr(r7),r15       # r15 = Buffer Pointer
#
# --- Initialize the contents of the ILT.
#
#   ilt->vrp = vrp;
#   ilt->philt = philt;  /* ??? Link back to philt */
#   ilt->tag =  tag;
#
        st      g2,il_vrp(g1)           # ILT_vrp = Ptr to VRP
        st      r3,il_philt(g1)         # ILT_philt = Store Pointer to placeholder
        st      r7,il_tag(g1)           # ILT_tag = Ptr to 1st cache tag for I/O
#
# --- Initialize the contents of most of the VRP.  (The I/O transfer length and
#     size of the SGL are initialized later after completing any coalescence.)
#
#   vrp->func = VROUTPUT;
#   vrp->strategy = VRHIGH;
#   vrp->vid = tag->vid;
#   vrp->vsda = tag->vsda;
#   vrp->options |= Flush Outstanding Request Count type op
#
!       ldos    tg_vid(r7),g4           # Get VID from cache tag
        setbit  vrforc,0,r9             # Set the VRP Options = Flush ORC op
        ldconst vroutput,g0             # VRP function code = Output (Write)
        ldconst vrhigh,g6               # VRP strategy = High priority
        stob    r9,vr_options(g2)       # Show this as a Flush ORC type op
        stos    g0,vr_func(g2)          # Store VRP function code
        stos    g6,vr_strategy(g2)      # Store VRP strategy/status
        stos    g4,vr_vid(g2)           # Store Virtual ID
        stl     r12,vr_vsda(g2)         # Store VRP starting disk addr
#
# --- Initialize a pointer to the start of the scatter gather list header
#     and store it in the ILT SGL pointer.  The SGL will use the extra
#     memory space allocated with the VRP that immediately follows it.
#
#   sglPtr = &vrp->sgl;
#   vrp->sglPtr = sglPtr;
#
        lda     vrpsiz(g2),g0           # Get Ptr to SGL header
        st      g0,vr_sglptr(g2)        # Store SGL pointer in VRP
#
# --- Initialize the count of the number of SGL entries to 0.
#     This count will be used as an index from the start of the scatter
#     gather list to store each entry into the list.
#
#   sglCount = 0;
#
        mov     0,r9                   # r9 = Number of SGL entries = 0
.if 1 # VIJAY_MC
        st      r9,vr_use2(g2)         # Zero out vr_use2 so as to VRP not needed to be
                                       # tracked in Back End.
.endif  # 1
#
# --- Initialize the I/O length to zero.  The length of each cache tag
#     that is coalesced will be added to the I/O length in the loop below.
#
#   ioLength = 0;
#
        mov     0,r10                  # r10 = I/O length = 0
#
# --- Move the length and buffer pointer for the first cache tag
#     into g6/g7.  The registers g4-g7 will be used in the loop
#     below to load the lba,length, and bufPtr for each cache
#     tag that is coalesced.
#
        mov     r14,g6
        mov     r15,g7
#
# --- Initialize the Last Tag to NULL to indicate that this is the
#     the first cache tag being processed.  The last tag is used
#     to update the nextDirty linked list in the cache tag when
#     creating the linked list of tags which have been coalesced.
#
#   lastTag = NULL;
#
        mov      0,r14
#
# --- Load the attribute and state locked bits for this tag into g0/g1.
#
!       ldos     tg_attrib(r7),g0       # Load attribute bits for next tag
!       ldos     tg_state(r7),g1        # Load state locked bits for next tag
#
.fc10:
# --- Start of loop to select multiple cache tag entries for coalescing
#     sequential I/Os together into a single disk I/O.
#   do
#
#
# --- The next cache tag entry is sequential with the current I/O.
#     Set the disk flush in progress and locked for flush bits
#     for this cache tag.
#
#       tag->attrib |= TG_FLUSHIP;
#       tag->state |= TG_LOCKED_FLUSH;
#
        setbit  TG_FLUSHIP,g0,g0
!       stos     g0,tg_attrib(r7)
#
        setbit  TG_LOCKED_FLUSH,g1,g1
!       stos     g1,tg_state(r7)
#
# --- Coalesce this tag with the current I/O by adding it's transfer length
#     to the current I/O.  Also update the Ending LBA
#
#       ioLength += tag->vlen;
#       endLba += tag->vlen;
#
        addo    g6,r10,r10              # add xlen to I/O transfer length
#
        addc    0,0,g1                  # clear carry bit
        addc    g6,r12,r12              # add xlen to Ending LBA+1
        addc    0,r13,r13
#
# --- Decrement the Count of Dirty Cache Tags and Dirty Blocks (if it is not
#       a BE tag - do not keep track of BE Tag count)
#
        bbs     TG_BE,g0,.fc15          # Jif this is a BE Tag
#
#       c_tagsDirty--;
#       c_blocksDirty -= tag->vlen;
#
        mov     g6,g0
        call    wc$DecDirtyCount
#
# --- Increment the Flush In Progress Tag and Block counts
#
        call    wc$IncFlushCount
#
# --- Initialize the contents of the next SGL entry for this cache tag.
#     This includes the starting physical buffer address and byte
#     transfer length.
#
#       sglPtr->desc[sglCount].addr = tag->bufPtr;
#       sglPtr->desc[sglCount].len = 512 * tag->vlen;
#
.fc15:
                                        # Store phys buf addr for SGL
        st      g7,vrpsiz+sg_desc0+sg_addr(g2)[r9*8]

        shlo    9,g6,g6                 # Calc byte xfer length=(512 * BlkCnt)

                                        # Store byte xfer length in SGL entry
        st      g6,vrpsiz+sg_desc0+sg_len(g2)[r9*8]
#
# --- Link the previous cache tag to this current cache tag.
#     This linked list of dirty cache tags that have been coalesced together
#     will be used by the completion routine.
#
#       if (lastTag != NULL)
#           lastTag->nextDirty = tag;
#
        cmpobe  0,r14,.fc20             # Jif lastTag == NULL
!       st      r7,tg_nextdirty(r14)    # lastTag->nextDirty = tag
#
.fc20:
# --- Advance the last cache tag to point to the current tag.
#
#       lastTag = tag;
#
        mov     r7,r14                  # lastTag = tag
#
# --- Increment the count of the number of SGL entries that have been
#     created for the I/O.
#
#       sglCount++;
#
        addo    1,r9,r9                 # increment count of SGL entries
#
# --- Completed while loop to process this cache tag.
#     Determine if more coalesce with another tag is possible.
#
#   while ( ( node = RB$locateNextCIntf(node) ) &&
#           (sglCount < MAX_SGL_ENTRIES) &&
#           (tag = node->dataPtr)->attrib & TG_DIRTY) &&
#           (!(tag->attrib & TG_FLUSHIP)) &&
#           (!(tag->state & TG_LOCKED_WRITE)) &&
#           (tag->vsda == endLba)  );
#
# --- Advance to the next cache tag entry.
#     Get the pointer to the next node in the RB-tree with an LBA greater
#     than or equal to the current node.
#
#     Inputs:   g1 = Ptr to current node in RB-tree
#     Outputs:  g1 = Ptr to next node in RB-tree with a greater LBA
#
#   while   ( node = RB$locateNextCIntf(node) ) &&
#
        mov     r6,g1                   # g1 = pointer to current RB-tree node
        call    RB$locateNext           # get pointer to next node in RB-tree
        mov     g1,r6                   # r6 = pointer to next node in RB-tree
        cmpobe  0,r6,.fc30              # Jif reached end of tree, done I/O
#
#     Check to see if sequential concatenation limits have been reached.
#     The size of the memory space available for the SGL limits how many
#     cache tags may be coalesced together for this I/O. This is limited
#     to a fixed number of SGL descriptor entries.
#
#   while ( (sglCount < MAX_SGL_ENTRIES) &&
#
        cmpoble MAX_SGL_ENTRIES,r9,.fc30 # Jif more coalescing not allowed
#
# --- Get the pointer to the corresponding next cache tag entry.
#     Determine if the next cache tag entry is a candidate for flushing.
#     It must be Dirty, without a disk flush in progress, and without
#     a locked for a write.
#
#   while   ( (tag = node->dataPtr)->attrib & TG_DIRTY) &&
#           (!(tag->attrib & TG_FLUSHIP)) &&
#           (!(tag->state & TG_LOCKED_WRITE)) &&
#
        ld      rbdpoint(r6),r7         # r7 = pointer to next cache tag
!       ldos    tg_attrib(r7),g0        # Load attribute bits for next tag
        bbc     TG_DIRTY,g0,.fc30       # Jif not Dirty
        bbs     TG_FLUSHIP,g0,.fc30     # Jif flush in progress
!       ldos    tg_state(r7),g1         # Load state locked bits for next tag
        bbs     TG_LOCKED_WRITE,g1,.fc30 # Jif locked for write
#
# --- The next cache tag entry is a valid candidate for flushing.
#     Load the next cache tag entry into g4-g7.
#     Test to see if the next cache tag entry is sequential with the current
#     I/O being processed.
#
#   while   (tag->vsda == endLba)  );
#
!       ldl     tg_vsda(r7),g4          # g4/g5 = Starting LBA
!       ld      tg_vlen(r7),g6          # g6 = Transfer Length (block count)
!       ld      tg_bufptr(r7),g7        # g7 = Buffer Pointer
        cmpobne g5,r13,.fc30            # Jif MSB not equal, not sequential
        cmpobe  g4,r12,.fc10            # Jif LSB equal, is sequential
#
.fc30:
# --- Finished coalescing for this I/O.
#
# --- Indicate the end of the linked list of dirty cache tags for this I/O.
#     Store a NULL (zero) value in the last entry of the linked list to
#     indicate the end of the list.
#
#   lastTag->nextDirty = NULL;
#
!       st      0,tg_nextdirty(r14)     # Store in last entry of linked list
#
# --- Complete initialization of VRP.  Store the I/O transfer length and
#     and the size of the scatter gather list in the VRP.
#
#   vrp->vlen = ioLength;
#   vrp->sglSize = (sglCount * sizeof(SGL_DESC)) + sizeof(SGL_HEADER);
#
        st      r10,vr_vlen(g2)         # Store VRP transfer length (blk cnt)
        lda     sghdrsiz[r9*8],g0       # Calculate size of SGL
        st      g0,vr_sglsize(g2)       # Store SGL size in VRP
#
# --- Initialize the Scatter Gather List Header.
#
#   sglPtr->header.size = vrp->sglSize;
#   sglPtr->header.count = sglCount;
#
        st      r9,vrpsiz+sg_scnt(g2)   # Store count of SGL descriptors
        st      g0,vrpsiz+sg_size(g2)   # Store SGL size in SGL Header
#
# --- Calculate an Op Throttle Value (OTV) and save in VRP.  Also update the
#       Controller Throttle Value and the VCD Throttle Values.
#       The OTV = Op Type * Op Size
#
        mulo    WRITE_THROTTLE_VALUE,r10,g5 # g5 = Size * Op Type (write)
        ldos    vr_vid(g2),r5           # r5 = VID
        ld      C_ctv,r8                # r8 = CTV
        ld      vcdIndex[r5*4],r9       # r9 = VCD
        ld      vc_vtv(r9),r7           # r7 = VTV
        addo    g5,r8,r8                # Update the CTV
        addo    g5,r7,r7                # Update the VTV
.ifdef M4_DEBUG_C_ctv
c CT_history_printf("%s%s:%u: C_ctv starts=%lu ends=%lu vc_vtv[%ld]=%ld\n", FEBEMESSAGE,__FILE__, __LINE__, C_ctv, r8, r5, r7);
.endif  # M4_DEBUG_C_ctv
        st      g5,vr_otv(g2)           # Save OTV in the VRP
        st      r8,C_ctv
        st      r7,vc_vtv(r9)
.ifdef FLIGHTRECORDER
        lda     wc$FlushIOComplete, g1
        cmpobe  g1,r4,.fc65
c       record_cache(FR_CACHE_BACKGROUND_FLUSH, (void *)g2);
        b       .fc70
#
.fc65:
c       record_cache(FR_CACHE_FLUSH, (void *)g2);
.fc70:
.endif  # FLIGHTRECORDER

#
# --- Send this I/O request to the Virtual Layer Queue
#     Inputs: g1 = Ptr to ILT
#
#   V$Queue(ilt);
#
        mov     r11,g1                  # get Ptr to ILT
        mov     r4,g2                   # get the completion routine
        call    c$calllower             # queue the request
#
# --- Return outputs
#
        mov     r10,g0                  # g0 = I/O Transfer Length
        mov     r6,g2                   # g2 = node
        mov     r12,g4                  # g4/g5 = endLba
        mov     r13,g5
        ret                             # Exit
#
#******************************************************************************
#
#  NAME: wc$FlushIOComplete
#
#  PURPOSE:
#       Completion routine for cache flush ILTs that were submitted to the
#       Virtual layer by the FlushTask.
#
#  CALLING SEQUENCE:
#       Call    wc$FlushIOComplete
#
#  INPUT:
#       g0 = Completion status
#       g1 = ILT
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       g0
#       g1
#       g2
#       g14
#
#******************************************************************************
#
# void FlushIOComplete(WORD Comp_code, ILT_PTR ilt)
#
#   PH_ILT_PTR          philt;
#
        .globl  wc$FlushIOComplete
wc$FlushIOComplete:
#
# --- Get the pointer back to the Placeholder ILT that requested
#     this flush.
#     Then handle the Flush Completion.
#
#   philt = ilt->philt;
#   HandleFlushComplete(Comp_code, ilt, philt);
#
.ifdef FLIGHTRECORDER
        ld      il_vrp(g1),r4
c       record_cache(FR_CACHE_FLUSH_COMPLETE, (void *)r4);
.endif  # FLIGHTRECORDER
        ld      il_philt(g1),g2         # Load the pointer to the Placeholder
        b       wc$HandleFlushComplete  # Handle the flush completion
#
#******************************************************************************
#
#  NAME: wc$BgFlushComplete
#
#  PURPOSE:
#       Completion routine for cache flush ILTs that were submitted to the
#       Virtual layer by the BackgroundFlushTask.
#
#  CALLING SEQUENCE:
#       Call    wc$BgFlushComplete
#
#  INPUT:
#       g0 = Completion Code
#       g1 = ILT
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       g0
#       g1
#       g2
#       g14
#
#******************************************************************************
#
# void BgFlushComplete(WORD Comp_code, ILT_PTR ilt)
#
#   PH_ILT_PTR          philt;
#
        .globl  wc$BgFlushComplete
wc$BgFlushComplete:
#
# --- Allocate a placeholder which will be used by the
#     HandleFlushComplete routine when submitting a request to
#     mirror the cache tag.
#     Clear the completion routine address for the placeholder.
#     This will be used to indicate this is for a background flush
#     instead of an overlap flush from the Flush Task.
#     Then handle the Flush Completion.
#
#   philt = AllocatePlaceholder();
#   philt->completionRoutine = 0;
#
#   HandleFlushComplete(Comp_code, ilt, philt);
#
.ifdef FLIGHTRECORDER
        ld      il_vrp(g1),r4
c       record_cache(FR_CACHE_BACKGROUND_FLUSH_COMPLETE, (void *)r4);
.endif  # FLIGHTRECORDER
        mov     g0,r4                   # Save the completion code & ILT pointer
        mov     g1,r5
c       g1 = get_wc_plholder();         # Get Placeholder ILT.
        ldconst 0,r14
        st      r14,plcr(g1)            # Clear completion routine addr
        mov     g1,g2                   # g2 = Placeholder ILT
        mov     r4,g0                   # Restore the compl. code & ILT pointer
        mov     r5,g1
        b       wc$HandleFlushComplete  # Handle the flush completion
#
#******************************************************************************
#
#  NAME: wc$HandleFlushComplete
#
#  PURPOSE:
#       This utility routine is used to handle the completion routine for
#       cache flush ILTs that were submitted to the Virtual layer by
#       either the FlushTask or the BackgroundFlushTask.
#
#  CALLING SEQUENCE:
#       Call    wc$HandleFlushComplete
#
#  INPUT:
#       g0 = Completion Code of the Flush
#       g1 = ILT
#       g2 = Placeholder
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       g0
#       g1
#       g2
#       g14
#
#******************************************************************************
#
# void HandleFlushComplete(WORD Comp_code, ILT_PTR ilt, PH_ILT_PTR philt)
#
#   CACHE_TAG_PTR       compTag;
#
#_compTag   r3  local
#_ilt   r8  param
#_philt r9  param
        .globl  wc$HandleFlushComplete
wc$HandleFlushComplete:
#
# --- Load the pointer to the first cache tag for this ILT.
#
#   compTag = ilt->tag;
#
        mov     g2,r9                   # r9 = Placeholder ILT
        mov     g1,r8                   # r8 = ILT
        ld      il_tag(r8),r3           # r3 = Tag being processed
        cmpobne 0,g0,.wchfc200          # Jif an error occurred during the flush
!       ldos    tg_vid(r3),g0           # Get VID for this tag
        ld      vcdIndex[g0*4],r12      # Retrieve VCD pointer for this VID
#
.wchfc10:
# --- Loop to process each cache tag associated with this
#     completed cache flush I/O.
#
#   do
#
# --- Add this cache tag to the Resident list.
#
#       wc$AddToResidentList(compTag);
#
        mov     r3,g0
        call    wc$AddToResidentList
#
# --- Remove this cache tag's node from the Dirty RB Tree
#     and release it's memory.
#
#       dirtyRoot = RB$delete(dirtyRoot, compTag->dirtyRB);
#       DeallocRBNode(compTag->dirtyRB);
#
        ld      vc_dirty(r12),g0        # Get root of the tree
!       ld      tg_dirtyptr(r3),g1
        call    RB$delete
        st      g0,vc_dirty(r12)        # Save root ptr (possibly modified)
#
!       ld      tg_dirtyptr(r3),g1
c       put_wc_rbnode(g1);              # Deallocate tree element
#
# --- Clear the Dirty RB node pointer for this cache tag
#     since it is no longer in the Dirty RB Tree.
#
#       compTag->dirtyRB = NULL;
#
!       st      0,tg_dirtyptr(r3)
#
# --- Clear the Flush In Progress and Dirty attribute bits for this
#     cache tag and set the Resident attribute bit instead.
#
#       compTag->attrib &= ~(TG_FLUSHIP + TG_DIRTY);
#       compTag->attrib |= TG_RESIDENT;
#
!       ldos    tg_attrib(r3),r15
        clrbit  TG_DIRTY,r15,r15
        clrbit  TG_FLUSHIP,r15,r15
        setbit  TG_RESIDENT,r15,r15
!       stos    r15,tg_attrib(r3)
#
# --- If this is a BE Tag, do not adjust the counts (not keeping track of BE
#       tag count)
#
        bbs     TG_BE,r15,.wchfc20      # Jif this is a BE tag
#
# --- Decrement the count of Flush In Progress cache tags and blocks.
#     Increment the count of Resident cache tags and blocks.
#
!       ld      tg_vlen(r3),g0
        call    wc$DecFlushCount
        call    wc$IncResidentCount
#
# --- Advance to the next sequential cache tag for this I/O
#
#       compTag = compTag->nextDirty;
#
.wchfc20:
!       ld      tg_nextdirty(r3),r3
#
# --- Loop if the next dirty cache tag pointer is not NULL
#
#   while (compTag != NULL);
#
        cmpobne 0,r3,.wchfc10
#
# --- Finished processing all sequential cache tags associated with
#     this complete cache flush I/O.
#     Reload the pointer to the start of the linked list of
#     cache tags for this ILT.
#
#   compTag = ilt->tag;
#
        ld      il_tag(r8),r3
#
# --- Free the memory used for the VRP and the ILT.
#
#   DeallocVRP(ilt->vrp);
#   DeallocILT(ilt);
#
        ld      il_vrp(r8),g2
#
#       Remove the OTV from the VTV and CTV, start the VCD wait process
#           (if anything is waiting), and then delete the ILT ans VRP.
#
        ld      C_ctv,g1                # Get the current CTV
        ld      vr_otv(g2),g0           # Get the OTV
        ld      vc_vtv(r12),r5          # Get the current VTV
        subo    g0,g1,g1                # Update the CTV
        subo    g0,r5,r5                # Update the VTV
.ifdef M4_DEBUG_C_ctv
c CT_history_printf("%s%s:%u: C_ctv starts=%lu ends=%lu vc_vtv[]=%ld\n", FEBEMESSAGE,__FILE__, __LINE__, C_ctv, g1, r5);
.endif  # M4_DEBUG_C_ctv
        st      g1,C_ctv
        st      r5,vc_vtv(r12)
#
        ld      C_vcd_wait_head,r6      # Get the wait head queue
        ld      C_vcd_wait_pcb,r4       # Get the VCD Wait Exec PCB
        cmpobe  0,r6,.wchfc50           # Jif nothing is waiting
        ldob    pc_stat(r4),r5          # Get current process status
        cmpobne pcnrdy,r5,.wchfc50      # Jif status other than not ready
        mov     pcrdy,r7                # Get ready status
.ifdef HISTORY_KEEP
c CT_history_pcb(".wchfc20 setting ready pcb", r4);
.endif  # HISTORY_KEEP
        stob    r7,pc_stat(r4)          # Ready process
#
.wchfc50:
        mov     r8,g1
        call    M$riv
#
# --- Submit a request to mirror all the cache tags which have
#     changed from Dirty to Resident.  This uses the same
#     thread (nextDirty) in the cache tags for the linked list that was
#     used above for the Dirty tags that were flushed.
#     Pass in the pointer to the first cache tag in this linked list.
#     Save the original completion routine address for the placeholder
#     and change it to the completion routine for the completion of
#     the mirror of the tags.
#
#   philt->saveCompRoutine = philt->completionRoutine;
#   philt->completionRoutine = FlushIOMirrorComplete;
#   QueueMirrorTag(compTag, philt);
#
        ld      plcr(r9),g1
        lda     wc$FlushIOMirrorComplete,g0
        st      g0,plcr(r9)
        mov     r3,g0
        st      g1,plcrs(r9)
        mov     r9,g1
        call    wc$qm_tag
        b       .wchfc900               # All done, return
#
# --- An error occurred trying to flush the data to disk.  Leave the data
#       in cache, send a message to the Console stating the cache is going
#       into a temporary disabled state, and report the error to the caller.
#
.wchfc200:
        mov     g0,r14                  # Save the completion code
        ldos    tg_vid(r3),r4           # r4 = Virtual ID for this tag
#
# --- Set the VID to error state, if not already done
#
        ld      vcdIndex[r4*4],r12      # r12 = VCD pointer for this VID
        ldob    vc_stat(r12),r13        # r13 = Cache Status
        bbs     vc_error,r13,.wchfc210  # Jif error already reported
        setbit  vc_error,r13,r13        # Set the Error State bit
        stob    r13,vc_stat(r12)        # Save the New Cache Status
#
# --- Send a message to the console reporting the temporary disabled state
#
        mov     r4,g0                   # g0 = Virtual ID for this tag
        mov     r14,g1                  # g1 = Error Code
        call    wc$MsgFlushFailed
#
# --- Reverse the previous counter adjustments - Increment the dirty counters
#       and decrement the resident counters (if this is not a BE tag)
#
.wchfc210:
        ldos    tg_attrib(r3),r5        # Get the tag attribute
        bbs     TG_BE,r5,.wchfc220      # Jif this is a BE tag
#
        ld      il_vrp(r8),g0           # Get the VRP Pointer
        ld      vrpsiz+sg_scnt(g0),r5   # Get the number of tags to decrement
        ld      vr_vlen(g0),g0          # Get the Length of the Transfer
.wchfc215:
        call    wc$IncDirtyCount        # Increment the Dirty Counters
        call    wc$DecFlushCount        # Decrement the Flush In Progress Count
        cmpdeco 1,r5,r5                 # Decrement the number of tags to do
        be      .wchfc220               # Decrement tags only (no blocks)
        mov     0,g0                    # Only decrement tags now
        b       .wchfc215               # Go do the decrement/increment
#
# --- Clear the Flush In Progress, Locked for Flush, and unlock the tag
#
.wchfc220:
        ldos    tg_attrib(r3),r15       # Get the Tag attributes
        clrbit  TG_FLUSHIP,r15,r15      # Clear the Flush In Progress Bit
        stos    r15,tg_attrib(r3)       # Store the Tag attributes
        ldos    tg_state(r3),r15        # Load State bits
        clrbit  TG_LOCKED_FLUSH,r15,r15 # Unlock tag for Flush
        stos    r15,tg_state(r3)        # Store State bits
        mov     r3,g0                   # g0 = Cache tag to unlock
        ld      tg_nextdirty(r3),r3     # Get the next sequential tag
        call    wc$unlock_tag           # Process the cache tag locked queue
        cmpobne 0,r3,.wchfc220          # Jif there are more sequential tags
#
# --- Report the error to the original caller
#
        mov     r14,g0                  # input g0 = completion code
        mov     r9,g1                   # input g1 = placeholder
        ld      plcr(r9),r4             # load the completion routine address
        cmpobe  0,r4,.wchfc230          # Jif zero (Background Flush)
        callx   (r4)                    # Call placeholder completion routine
        b       .wchfc240               # Release the ILT/VRP pair
#
.wchfc230:
c       put_wc_plholder(g1);            # Release the placeholder ILT
#
# --- Free the memory used for the VRP and the ILT.
#
#   DeallocVRP(ilt->vrp);
#   DeallocILT(ilt);
#
.wchfc240:
        ld      il_vrp(r8),g2
#
#       Remove the OTV from the VTV and CTV, start the VCD wait process
#           (if anything is waiting), and then delete the ILT ans VRP.
#
        ld      C_ctv,g1                # Get the current CTV
        ld      vr_otv(g2),r3           # Get the OTV
        ld      vc_vtv(r12),r5          # Get the current VTV
        subo    r3,g1,g1                # Update the CTV
        subo    r3,r5,r5                # Update the VTV
.ifdef M4_DEBUG_C_ctv
c CT_history_printf("%s%s:%u: C_ctv starts=%lu ends=%lu vc_vtv[]=%ld\n", FEBEMESSAGE,__FILE__, __LINE__, C_ctv, g1, r5);
.endif  # M4_DEBUG_C_ctv
        st      g1,C_ctv
        st      r5,vc_vtv(r12)
#
        ld      C_vcd_wait_head,r6      # Get the wait head queue
        ld      C_vcd_wait_pcb,r4       # Get the VCD Wait Exec PCB
        cmpobe  0,r6,.wchfc250          # Jif nothing is waiting
        ldob    pc_stat(r4),r5          # Get current process status
        cmpobne pcnrdy,r5,.wchfc250     # Jif status other than not ready
        mov     pcrdy,r7                # Get ready status
.ifdef HISTORY_KEEP
c CT_history_pcb(".wchfc240 setting ready pcb", r4);
.endif  # HISTORY_KEEP
        stob    r7,pc_stat(r4)          # Ready process
#
.wchfc250:
        mov     r8,g1
        call    M$riv
#
# --- Decrement the number of outstanding Flush Ops
#
        ld      C_flush_orc,r3          # Get the Outstanding Flush Op counter
        subo    1,r3,r3                 # Decrement the counter and save
        st      r3,C_flush_orc
#
# --- All done, return
#
.wchfc900:
        ret
#
#******************************************************************************
#
#  NAME: wc$FlushIOMirrorComplete
#
#  PURPOSE:
#       Continuation of the completion routine for cache flush ILTs that were
#       submitted to the Virtual layer by either the FlushTask or the
#       BackgroundFlushTask after the mirror of the cache tag has been
#       completed.
#
#  CALLING SEQUENCE:
#       Call    wc$FlushIOMirrorComplete
#
#  INPUT:
#       g0 = Completion Code
#       g1 = Placeholder
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       Possibly all G registers.
#
#******************************************************************************
#
# void FlushIOMirrorComplete(PH_ILT_PTR philt)
#
#   CACHE_TAG_PTR       compTag;
#
        .globl  wc$FlushIOMirrorComplete
wc$FlushIOMirrorComplete:
#
# --- Determine if an error occurred during the Mirror.  If so, handle based
#       on the original requester.  Else, complete the operation.
#
        cmpobe  0,g0,.wcfiomc500        # Jif no error on the mirror
#
# --- An error occurred on the mirror.  The error has already been reported,
#       so clean up to the point that the op can be reissued once the error
#       status is cleared.  Then queue the op to the Error Task so it can be
#       restarted.
#
        lda     wc$FlushIOMirrorRestart,r3 # Point to the routine to restart
        st      r3,il_cr(g1)            # Save the restart routine
        call    c$qerror                # Queue the ILT to the Error Task
        ret                             # Return
#
# --- No error occurred on the mirror operation.  Continue to complete the op.
#
.wcfiomc500:
#
# --- Load the pointer to the start of the linked list of
#     cache tags for this placeholder ILT.
#     Also save the placeholder and cache tag in local registers
#     since all the gx registers may be corrupted when
#     the tag is unlocked (other completion routines may be called).
#
#   compTag = philt->tag;
#
        mov     g1,r3                   # r3 = placeholder
        ld      plctag(g1),r8           # r8 = load cache tag from placeholder
        mov     0,r4
#
.L1326:
# --- Loop to process each cache tag associated with this
#     completed mirror of the cache tags following the flush I/O.
#
#   do
#
# --- Unlock this Cache Tag for the Flush operation and process the
#     cache tag locked queue.
#
#       UnlockTagFlush(compTag);
#       compTag = compTag->nextDirty;
#
        mov     r8,g0                   # Get cache tag for input in g0
!       ldos    tg_state(g0),r15        # Load State bits
        clrbit  TG_LOCKED_FLUSH,r15,r15 # Unlock tag for Flush
!       stos    r15,tg_state(g0)        # Store State bits
!       ld      tg_nextdirty(r8),r8     # load next cache tag
!       st      r4,tg_nextdirty(g0)     # Clear the next cache tag field
        call    wc$unlock_tag           # Process the cache tag locked queue
#
# --- Advance to the next cache tag in the linked list.
#     Loop if the next dirty cache tag pointer is not NULL
#
#   while (compTag != NULL);
#
        cmpobne 0,r8,.L1326
#
#     Call the original completion routine for the Placeholder ILT
#     which was saved during the Flush I/O Completion routine.
#
#   if ( philt->saveCompRoutine )
#       philt->saveCompRoutine(philt);
#   else
#       ReleasePlaceholder(philt);
#
        ldconst 0,g0                    # Show the Op completed normal
        mov     r3,g1                   # input g1 = placeholder
        ld      plcrs(r3),r4            # load the saved complt routine addr
        cmpobe  0,r4,.L1327             # Jif zero (Background Flush)
        callx   (r4)                    # Call placeholder completion routine
        b       .L1329                  # Exit
#
.L1327:
c       put_wc_plholder(g1);            # Release the placeholder ILT
.L1329:
#
# --- Decrement the number of outstanding Flush Ops
#
        ld      C_flush_orc,r3          # Get the Outstanding Flush Op counter
        subo    1,r3,r3                 # Decrement the counter and save
        st      r3,C_flush_orc
#
# --- Check to see if write cache ops were previously blocked due to
#     insufficient resources and if so, whether a process needs to be
#     made ready.
#
        b       wc$chkrdy               # Do the check and return
#
#**********************************************************************
#
#  NAME: wc$FlushIOMirrorRestart
#
#  PURPOSE:
#       Restart the mirror operation that was queued due to an error
#       state condition.
#
#  DESCRIPTION:
#       This routine restarts the mirror operation that was queued due to an
#       error state condition.  It restores the ILT to the correct state
#       and reissues the op to wc$qm_tag.
#
#  CALLING SEQUENCE:
#       call    wc$FlushIOMirrorRestart
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
wc$FlushIOMirrorRestart:
        movl    g0,r14                  # Save g0-g2
#
# --- Set up and reissue the mirror operation
#
        lda     wc$FlushIOMirrorComplete,r3 # r3 = Completion routine when done
        ld      plctag(g1),g0           # g0 = The cache tag to process
        st      r3,il_cr(g1)            # Save the completion routine
        call    wc$qm_tag               # Restart the process
        movl    r14,g0                  # Restore g0-g2
        ret
#
#******************************************************************************
#
#  NAME: WC$BackgroundFlushTask
#
#  PURPOSE:
#       To select dirty data to be flushed from the write cache in the
#       background based on thresholds for the number of dirty cache tags
#       and dirty blocks.
#
#  CALLING SEQUENCE:
#       Not called, invoked by Kernel
#
#  INPUT:
#       Cache Tags
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       Possibly all G registers.
#
#  REGISTER USAGE:
#    g0 = Parameter Passing
#    g1 = Parameter Passing
#    g2 = Temp
#    g5 = Temp
#    g6 = Temp
#    g7 = Temp
#    g8 = Temp
#    g9 = Temp
#
#    r3 = Number of blocks flushed so far
#    r4 = Total number of VIDs (constant)
#    r5 = Cache Statistics Pointer (constant)
#    r6 = Timed(=TRUE)/Full(=FALSE) Flush Flag
#    r7 = Cached(=TRUE)/No Cache(=FALSE) Device Exists Flag
#    r8 = Start VID (to know when cycled back to beginning)
#   r10 = Number of Tags to increment to the next level of flush time
#   r11 = Number of Blocks to increment to the next level of flush time
#   r12 = Current VID to flush
#   r13 = Number of Tags to start flushing on (High water mark)
#   r14 = Number of Blocks to start flushing on (High water mark)
#   r15 = Time to wait before flushing again
#
#******************************************************************************
#
# void WC$BackgroundFlushTask(void)
#
#   static unsigned short int   flushVCD = 0;
#   unsigned int                blocksFlushed;
#
#_flushVCD  .L1443  static
#_blocksFlushed r3  local
#_startVID  r8  local
        .globl  WC$BackgroundFlushTask
WC$BackgroundFlushTask:
#
# --- Initialize a pointer to the PCB.
#     Initialize the flushVID to 0 and other constants
#
        lda     c_bgflush,r15           # Point to PCB ptr
        ldconst 0,r12                   # r12 = flushVID = 0
        ldconst MAXVIRTUALS,r4          # Initialize r4 = MAXVIRTUALS
        lda     C_ca,r5                 # Get the Cache Statistics Pointer
        ld      C_TagStartFlushThresh,r13 # Number of Tags to start flushing on
        ld      C_BlockStartFlushThresh,r14 # Number of Buffers to start flush
        ld      C_TagFlushIncr,r10      # Get the timed increment for tags
        ld      C_BlockFlushIncr,r11    # Get the timed increment for blocks
#
# --- Setup queue control block
#
        ld      K_xpcb,r3
        st      r3,(r15)                # Set pcb ptr
#
# --- Set up the nominal time to do flush checking
#
        ldconst SLOW_FLUSH_WAIT_TIME,r15 # Wait the default time amount (slow)
#
.wcbgft10:
        ldob    ca_status(r5),r3        # r3 = Global Cache Status
        ld      c_bgflush,r8            # r8 = This Tasks PCB
        ldconst pcnrdy,r6               # Set the task asleep until Write Cache
        bbs     ca_ena,r3,.wcbgft11     # Jif Cache is enabled
        stob    r6,pc_stat(r8)          # Set the PCB to Not Ready
        call    K$qxchang               # Go to sleep
        b       .wcbgft12               # Write Cache enabled, go do flush

.wcbgft11:
        mov     r15,g0                  # Wait for requested time
        call    K$twait
#
# --- Determine if Background Flushes are allowed.  If not, wait some more. If
#       so, continue as normal.
#
.wcbgft12:
        ldob    ca_status(r5),r3        # r3 = Global Cache Status
        ldconst NO_FLUSH_ALLOWED_WAIT_TIME,r15 # r15=No Flush allowed wait time
        bbs     ca_halt_background,r3,.wcbgft10 # Jif no flushes allowed
        bbs     ca_error,r3,.wcbgft10   # Jif cache in an error state
#
# --- Determine if there are any Disables in Progress and continue the process
#       in case some I/O slipped in.
#
        ldconst FLUSH_OPS_WAIT,r15      # r15 = Flush op wait time
        bbc     ca_dis_ip,r3,.wcbgft13  # Jif if the Global Cache is not being
                                        #  disabled
        ldconst FALSE,g0                # g0 = Do not wait for flush to complete
        call    wc$FlInvAll             # Try the Flush and Invalidate All again
        b       .wcbgft10
#
.wcbgft13:
        ldconst 0,r6                    # r6 = VID being processed
.wcbgft14:
        ld      vcdIndex[r6*4],g2       # Load pointer to VCD for this VID
        cmpobe  0,g2,.wcbgft18          # Jif there is not a valid device
        ldob    vc_stat(g2),g7          # Load VCD Status field
        bbc     vc_disable_ip,g7,.wcbgft18 # Jif not in Disable Pending state
        mov     r6,g0                   # g0 = input VID
        call    wc$FlInvVID             # Try and clear out any new ones again
        cmpobe  FALSE,g0,.wcbgft10      # Jif no more flushes allowed
.wcbgft18:
        addo    1,r6,r6                 # Point to the next VID
        cmpobne r6,r4,.wcbgft14         # Jif all VID not processed yet
#
# --- Initialized Total Blocks Flushed Count to zero.
#     Initialize the startVID to flushVID.
#     Initialize the maximum number of Virtual IDs to check.
#     Initialize the Timed Flush flag to show a timed flush.
#
#       blocksFlushed = 0;
#       startVID = flushVID;
#       timedFlush = TRUE;
#
        ldconst SLOW_FLUSH_WAIT_TIME,r15 # Set wait time to default (slow)
        mov     0,r3                    # blocksFlushed = 0
        mov     r12,r8                  # r8 = startVID = flushVID
        ldconst TRUE,r6                 # r6 = timedFlush = TRUE
        ldconst FALSE,r7                # r7 = No cached device exists
#
# --- Determine if this is a timed flush or a cache full flush
#
        ld      ca_tagsDirty(r5),g6
        cmpobge g6,r13,.wcbgft20        # Jif tagsDirty >= Start Threshold
        ld      ca_blocksDirty(r5),g5
        cmpobge g5,r14,.wcbgft20        # Jif blocksDirty >= Start Threshold
#
# --- Timed Flush.  Next Flush time depends on Host activity and how many
#       Cache resources are in use.
#
        ldconst FASTEST_FLUSH_WAIT_TIME,r15 # Flush Fastest time
        ld      C_orc,g7                # See if there is any host activity
.ifdef M4_DEBUG_C_orc
c CT_history_printf("%s%s:%u: C_orc check and branch if 0 (%lu)\n", FEBEMESSAGE,__FILE__, __LINE__, C_orc);
.endif  # M4_DEBUG_C_orc
        cmpobe  0,g7,.L1394             # Jif there is no activity
        subo    r10,r13,g8              # Almost at the High water mark of Tags?
        cmpobge g6,g8,.L1394            # Jif at the Flush Fastest time = tags
        subo    r11,r14,g9              # Almost at the High water of blocks?
        cmpobge g5,g9,.L1394            # Jif at the Flush Fastest time = blks
        ldconst FAST_FLUSH_WAIT_TIME,r15 # Flush Fast time
        subo    r10,g8,g8               # At the Midpoint for Tags?
        cmpobge g6,g8,.L1394            # Jif at the Flush Fast time = tags
        subo    r11,g9,g9               # At the Midpoint for Blocks?
        cmpobge g5,g9,.L1394            # Jif at the Flush Fast time = blocks
        ldconst MEDIUM_FLUSH_WAIT_TIME,r15 # Flush Medium time
        subo    r10,g8,g8               # Almost at the Low water mark for Tags?
        cmpobge g6,g8,.L1394            # Jif at the Flush Medium time = tags
        subo    r11,g9,g9               # Almost at the Low water mark for Blks?
        cmpobge g5,g9,.L1394            # Jif at the Flush Medium time = blocks
        ldconst SLOW_FLUSH_WAIT_TIME,r15 # Flush Slow time
        b       .L1394
#
.wcbgft20:
        mov     FALSE,r6                # This is a cache full situation
#
.L1394:
# --- Loop to flush dirty write cache data for each Virtual ID
#
#       do
#
# --- Initialize the startVID to flushVID.
#
#           startVID = flushVID;
#
        mov     r12,r8                  # r8 = startVID = flushVID
#
.wcbgft25:
# --- Loop to find a valid Cacheable Virtual ID
#
#           do
#
# --- Advance to the next Virtual ID.
#
#               flushVID++;
#               if (flushVID >= MAXVIRTUALS)
#                   flushVID = 0;
#
        addo    r12,1,r12               # Increment flushVID
        cmpobl  r12,r4,.L1393           # Jif flushVID < MAXVIRTUALS
        ldconst 0,r12                   # Wrap flushVID back to zero
.L1393:
#
# --- Check to see if the next Virtual ID is cacheable.  If not loop
#     back to advance to the next Virtual ID.  If the startVID is
#     been reached again without finding a valid cacheable Virtual ID,
#     then exit.
#          while ( ( (vcdIndex[flushVID] == NULL) ||
#                    (vcdIndex[flushVID]->vc_stat != VC_CACHED) ) &&
#                  (flushVID != startVID) );
#
        ld      vcdIndex[r12*4],g2      # Load pointer to VCD for this VID
        cmpobe  0,g2,.wcbgft30          # Jif there is not a valid device
        ldob    vc_stat(g2),g7          # Load VCD Status field
        bbs     vc_cached,g7,.L1395     # Jif VCD Status == VCD Cached
.wcbgft30:
        cmpobne r12,r8,.wcbgft25        # Jif flushVID != startVID
        cmpobe  TRUE,r7,.wcbgft90       # Jif there are cached devices
#
# --- None of the Virtual IDs are cacheable. Exit
#
        b       .wcbgft10
#
.L1395:
#
# --- This Virtual ID is a valid cacheable VID.
#
        ldconst TRUE,r7                 # Show a cached device does exist
#
# --- If in a Global Disable In Progress state, see if it can be completed
#
        ldob    ca_status(r5),g8        # g8 = Global Cache Status
        bbc     ca_dis_ip,g8,.wcbgft35  # Jif if the Global Cache is not being
                                        #  disabled
        call    wc$SetGlobalDisable     # See if this process can be completed

#
# ---If the VID is disable pending or Error state and there is no data in cache,
#       then finish the Disable in Process or Error Process handling.
#
.wcbgft35:
        and     VC_NO_MORE_DATA,g7,g8   # Check to see if more data is allowed
        cmpobe  0,g8,.wcbgft70          # Jif if not error or disable pending
        ld      vc_write_count(g2),g8   # Number of outstanding host write cmds
        cmpobne 0,g8,.wcbgft70          # Jif possible to get more data in cache
        bbc     vc_error,g7,.wcbgft40   # Jif the VID is not in error state
        ld      vc_dirty(g2),g8         # Get the Dirty Tree Root pointer
        cmpobne 0,g8,.wcbgft70          # Dirty data still exists
#
# --- There is no dirty data in cache, so the error state must have been
#       cleared. Report the error has been recovered and clear the error state.
#
        mov     r12,g0                  # g0 = VID to report the status on
        ldconst 0,g1                    # g1 = No error code
        call    wc$MsgFlushRecovered    # Send the message
        clrbit  vc_error,g7,g7          # Clear the Cache Error State bit
        stob    g7,vc_stat(g2)          # Save the new status
#
# --- If Disabled Pending - clear the Cached and Disable Pending flags if no
#       data is in cache
#
.wcbgft40:
        bbc     vc_disable_ip,g7,.wcbgft70 # Jif not in Disable Pending state
        ld      vc_cache(g2),g8         # See if any data is in cache
        cmpobne 0,g8,.wcbgft70          # Jif there is data in cache still
        clrbit  vc_disable_ip,g7,g7     # Clear the Disable in Progress bit
        clrbit  vc_cached,g7,g7         # Clear the VID cached Bit
        stob    g7,vc_stat(g2)          # Save the new status
        mov     r12,g0                  # g0 = VID to report flush complete
        call    wc$MsgFlushComplete     # Report to the MMC
        b       .wcbgft25               # Go to the next VID
#
# --- Flush some write cache data for this VID.
#
#           if (vcdIndex[flushVID]->vc_stat == VC_CACHED)
#               blocksFlushed += FlushVID(flushVID,maxBlocks);
#
.wcbgft70:
        mov     r12,g0                  # Input flushVID in g0
        ldconst MAX_VID_FLUSH_BLOCKS,g1 # g1 = maximum number of blocks to do
        call    wc$FlushVID             # Is cached, flush data for this VID
        addo    r3,g0,r3                # Update blocksFlushed count
#
# --- If nothing was flushed for this VID, try the next VID unless all the
#     VIDs have been processed in this loop.
#
        cmpobe  r12,r8,.wcbgft80        # Jif been around the loop already
        cmpobe  0,g0,.wcbgft25          # Jif nothing was flushed for this VID
#
# --- Swap Tasks before advancing to the next Virtual ID
#
.wcbgft80:
        call    K$qxchang
#
# --- Loop if this is a cache full situation and the number of dirty tags
#     or blocks still exceeds the stop threshold and the limit for the
#     number of dirty blocks to flush has not been exceeded.
#
#       while ( (timedFlush == FALSE) &&
#               (blocksFlushed < C_MaxTotalFlushBlocks) &&
#               ((c_tagsDirty >= C_TagStopFlushThresh) ||
#                (c_blocksDirty >= C_BlockStopFlushThresh)) );
#
.wcbgft90:
        cmpobne FALSE,r6,.L1390         # Jif this is a timed flush situation
        ld      C_MaxTotalFlushBlocks,g7 # g7 = maximum number of blocks to do
        cmpobge r3,g7,.L1390            # Jif blocksFlush >= Block Threshold
        ldconst FALSE,r7                # Reset the Cached Devices flag
        ld      ca_tagsDirty(r5),g6     # Get the number of dirty tags
        ld      C_TagStopFlushThresh,g7 # Number of Tags to stop flushing on
        cmpoble g7,g6,.L1394            # Jif tagsDirty >= Stop Threshold
        ld      ca_blocksDirty(r5),g5   # Get the number of dirty blocks
        ld      C_BlockStopFlushThresh,g7 # Number of buffers to stop flushing
        cmpoble g7,g5,.L1394            # Jif blocksDirty >= Stop Threshold
#
# --- The Stop threshold has been reached or a timed cache situation, Exit
#
.L1390:
        b       .wcbgft10
#
#******************************************************************************
#
#  NAME: wc$FlushVID
#
#  PURPOSE:
#       To flush dirty write cache data for the input Virtual ID.
#
#  CALLING SEQUENCE:
#       call    wc$FlushVID
#
#  INPUT:
#       g0 = Virtual ID
#       g1 = Maximum number of blocks to flush
#
#  OUTPUT:
#       g0 = Total blocks submitted for flush
#
#  REGS DESTROYED:
#       Possibly all G registers.
#
#******************************************************************************
#
# unsigned int FlushVID(unsigned short vid,unsigned int maxBlocks)
#
#    unsigned int    totalBlocks;
#    RB_NODE_PTR     rootRB, node, startNode;
#    unsigned int    ioLength;
#    unsigned int    flushLBA;
#    CACHE_TAG_PTR   tag
#
#_rootRB g0 param
#_maxBlocks g1 param
#_tag   r3  local
#_vid   r4  param
#_vcd   r5  local
#_MAX_VID_FLUSH_BLOCKS r7 local
#_node  r8  local
#_totalBlocks   r11 local
#_endLba    r12/r13 local
#
    .globl  wc$FlushVID
wc$FlushVID:
        mov     g0,r4                   # r4 = vid
        mov     g1,r7                   # r7 = Max number of blocks to flush
#
# --- Initialize the total blocks flushed count to zero.
#
#   totalBlocks = 0;
#
        mov     0,r11                   # totalBlocks = 0
#
# --- Get the Dirty RB Root and Flush LBA value from the VCD Table
#     for this Virtual ID.
#
#   rootRB = vcdIndex[vid]->vc_dirty;
#   flushLBA = vcdIndex[vid]->vc_flushLBA;
#
        ld      vcdIndex[r4*4],r5       # r5 = VCD Pointer
        ld      vc_dirty(r5),g0         # g0 = VCD[vid].dirtyRoot
        ldl     vc_flushLBA(r5),r12     # r12,r13 = VCD[vid].flushLBA
#
# --- Find the first node in the Dirty RB Tree for this Virtual ID
#     that is after the Flush LBA.  Flushing of dirty data will
#     resume from this Flush LBA.
#     If the RB Tree is empty, then return no blocks were flushed.
#     Jump into the middle of the loop below to accomplish these
#     steps.
#
#   node = RB$locateStart(rootRB, flushLBA);
#   if (node == NULL)
#       return(totalBlocks);
#
        b       .L1550                  # Jump to loop to find first node
#
.L1510:
# --- Loop to search for dirty data to be flushed to disk
#
#   do
#
# --- Initialize a pointer to the cache tag for this node.
#
#       tag = node->dataPtr;
#
        ld      rbdpoint(r8),r3         # r3 = tag = node->dataPtr
#
# --- Check to see if this cache tag is a valid candidate for flushing.
#     The tag must be Dirty, without a flush already in progress, and
#     not locked for a write.
#
#       if (!(tag->attrib & TG_DIRTY) ||
#          (tag->attrib & TG_FLUSHIP) ||
#          (tag->state & TG_LOCKED_WRITE) )
#
        ldos    tg_attrib(r3),g0        # Load tag attribute bits
        bbc     TG_DIRTY,g0,.L1560      # Jif not Dirty, exit (sanity error)
        bbs     TG_FLUSHIP,g0,.L1520    # Jif Flush In Progress
        ldos    tg_state(r3),g5         # Load tag locked state bits
        bbc     TG_LOCKED_WRITE,g5,.L1545 # Jif Locked for Write
#
.L1520:
# --- This cache tag was not a valid candidate to flush.
#     Advance to the next node in the Dirty RB Tree.
#           node = RB$locateNext(node);
#
        mov     r8,g1
        call    RB$locateNext
        mov     g1,r8
#
# --- Check to see if the end of the Dirty RB tree has been reached (NULL).
#     If it has, then restart at the beginning of the tree (LBA = 0).
#
#           if (node == NULL)
#
        cmpobne 0,r8,.L1510             # Jif node != NULL, (end of tree)
#
.L1540:
# --- The end of the RB tree was reached.
#     Reset the flushLBA to the beginning of the tree (LBA = 0) and exit.
#
#               flushLBA = 0;
#               break;
#
        mov     0,r12                   # r12/r13 = flushLBA = 0
        mov     0,r13
        b       .L1560
#
.L1545:
# --- This cache tag is a valid candidate to flush.
#     Attempt to coalesce other dirty cache tags to this tag and build
#     and submit an ILT to the Virtual layer.
#     Save the node output from FlushCoalesce.
#     Save the endLba output from FlushCoalesce
#
#       else
#           iolength,node,flushLba = FlushCoalesce(tag, philt, node, CR )
#               Outputs g0=iolength
#                       g2=node
#                       g4/g5=endLba
#
        mov     r3,g0                   # Input g0 = tag
                                        # Input g1 = philt (not used)
        mov     r8,g2                   # Input g2 = node
        lda     wc$BgFlushComplete,g3   # Input g3 = Completion Routine
        call    wc$FlushCoalesce        # Attempt to coalesce to this dirty tag
        mov     g4,r12                  # flushLBA = endLba
        mov     g5,r13
#
# --- Update the total blocks flushed count.
#
#           totalBLocks += ioLength;
#
        addo    r11,g0,r11              # totalBlocks += ioLength
#
# --- Check to see if the end of the Dirty RB tree was reached (NULL)
#     during the FlushCoalesce.
#
#           if (node == NULL)
#               flushLBA = 0;
#               break;
#
        cmpobe  0,g2,.L1540             # Jif at end of tree
#
.L1550:
# --- The end of the tree was not reached.
#     Find the first node in the Dirty RB Tree for this Virtual ID
#     that is after the specified LBA.  This is necessary since the tree
#     could have changed during a context exchange that may happen during
#     FlushCoalesce.
#
#           node = RB$locateStart(rootRB, flushLBA);
#
        ld      vc_dirty(r5),g0         # Input g0 = dirtyRoot
        mov     r12,g2                  # Input g2/g3 = flushLBA
        mov     r13,g3
        call    RB$locateStart          # Find the node at the beginning
        mov     g0,r8                   # Save the node found
        cmpobe  0,r8,.L1560             # Jif node == NULL, Exit
#
# --- Loop back to continue to process nodes in the Dirty RB Tree until
#     the maximum number of blocks for this Virtual ID have been submitted
#     for a cache flush (or the end of the tree is reached).
#
#   while ((totalBlocks < MAX_VID_FLUSH_BLOCKS) && (node != NULL));
#
        cmpobl  r11,r7,.L1510           # Jif totalBlocks < VID Max Flush Blks
#
.L1560:
# --- Done attempting to flush write cache data for this Virtual ID.
#     Save the Flush LBA value in the VCD Table.  The Flush will resume
#     at this LBA next time this function is invoked.
#
        stl     r12,vc_flushLBA(r5)     # Store flushLBA in VCD Table
#
# --- Return the number of blocks that were submitted for a cache flush.
#
#   return(totalBlocks);
#
        mov r11,g0                      # return(totalBlocks)
        ret
#
#******************************************************************************
#  NAME:  wc$FlInvAll
#
#  PURPOSE:
#       To flush and invalidate all write cache data.
#
#  DESCRIPTION:
#       This function will go through each of the cached VIDs and flush and
#       invalidate all data from the cache.  If the particular VID has the
#       Disable In Progress flag set and all the data has been invalidated,
#       then the cache will be disabled for the device.  The function
#       will return  when all data has successfully been flushed and
#       invalidated from the cache, if all the data could be flushed.
#       If the cache is globally being disabled and all data was flushed,
#       that process will be completed as well.
#
#  CALLING SEQUENCE:
#       call    wc$FlInvAll
#
#  INPUT:
#       g0 = TRUE - Wait for Cache to be flushed before returning
#            FALSE -  Do not wait for Cache to complete the flush
#
#  OUTPUT:
#       g0 = TRUE  - all data was successfully flushed and invalidated
#            FALSE - some data was left in cache due to a device error or
#                    resource constraint
#
#  REGS DESTROYED:
#       g0
#
#   REGISTER USAGE:
#        g0 = Parameter passing to called functions
#        g1 = Parameter passing to called functions
#        g2-g6 = Logging registers
#        g7 = 4 second constant
#        g8 = Starting time
#        g9 = K_ii.ii_time to log a message
#       g10-g12 = Logging register
#        r3 = Loop Counter
#        r4 = Last VID+1 (Constant)
#        r5 = temp register
#        r6 = temp register
#        r7 = Flushing VID (rotates so we do not get stuck on on VID)
#        r8 = Flushing counter to know when all the VIDs have been processed
#        r9 = VID in Error State flag
#       r10 = Maximum number of loops before detecting hang condition
#       r11 = VCD Pointer
#       r12 = All data has been Flushed Flag (TRUE = Done, FALSE = more data)
#       r13 = Current VID that is being processed
#       r14 = Save g0 (input parameter)
#       r15 = PushRegs - save all 'g' registers
#
#******************************************************************************
#
wc$FlInvAll:
        PushRegs(r15)                   # Save all the 'g' registers
        mov     g0,r14                  # Save the callers g0
        ld      K_ii+ii_time,g8         # Get the current time
        ldconst 5000/QUANTUM,g9         # Start logging after a 5 second delay
        ldconst 2000/QUANTUM,g7         # Wait 2 seconds between log entries
        addo    g9,g8,g9                # Next time to log a message
#
# --- Go through all the Valid VIDs looking for cached VIDs (in case
#     an op was pending between the calls)
#
        ldconst 0,r3                    # Loop counter (report failure at
                                        #   certain intervals)
        ldconst MAXVIRTUALS-1,r4        # r4 = Last valid VID
        ldconst 2000,r10                # r10 = about 4 minutes
        ldconst -1,r7                   # r7 = VID being used to Flush (-1 to
                                        #  get first time to VID = 0)
#
# If there is still dirty data,to ensure all data has been sent to the drives
#
.wcfa10:
        ldconst FALSE,r9                # r9 = No VIDs in Error Detected
        ldconst 0,r8                    # r8 = loop counter for flush terminate
#
# Loop - go through each VID to check if the device is cached or not
#   and flush the dirty data for all that are cached (if there is any)
#
.wcfa20:
        cmpinco r4,r7,r7                # Compare to max and increment VID
        sele    r7,0,r7                 # If beyond valid VID, set VID to zero
        ld      vcdIndex[r7*4],r11      # r11 = VCD pointer for this VID
        cmpobe  0,r11,.wcfa30           # Jif the VCD is not valid
        ldob    vc_stat(r11),r5         # r5 = Status of the VID
        bbc     vc_cached,r5,.wcfa30    # Jif this VID is not cached
        ld      vc_cache(r11),r6        # r6 = Valid Data Root Pointer
        cmpobe  0,r6,.wcfa30            # Jif there is no cached data
        mov     r7,g0                   # g0 = VID to flush and Invalidate
        call    wc$FlInvVID             # Go Invalidate the data for this VID
        cmpobe  FALSE,g0,.wcfa40        # Jif could not complete due to resource
                                        #  constraints
#
# End of Loop - going through each VID
#
.wcfa30:
        cmpinco r4,r8,r8                # Compare to done and increment cntr
        bg      .wcfa20                 # Jif not all VIDs checked yet
        cmpobe  FALSE,r14,.wcfa40       # Jif asked not to wait for flush
        call    K$xchang                # Let some other work be done
#
# Keep looping until all data has been flushed from the cache
#
.wcfa40:
        ldconst 0,r13                   # r13 = point to the first possible VID
        ldconst TRUE,r12                # r12 = all data has been Flushed flag
.wcfa45:
        ld      vcdIndex[r13*4],r11     # r11 = VCD pointer for this VID
        cmpobe  0,r11,.wcfa70           # Jif the VCD is not valid
        ldob    vc_stat(r11),r5         # r5 = Status of the VID
        bbc     vc_cached,r5,.wcfa70    # Jif this VID is not cached
        ld      vc_cache(r11),r6        # r6 = Cached Data Root Pointer
        cmpobe  0,r6,.wcfa55            # Jif there is no Cached data
        bbs     vc_error,r5,.wcfa50     # Jif this VID is in error state - do
                                        #   not include it in whether or not
                                        #   to continue looping (background
                                        #   task will handle Error State VIDs)
        ldconst FALSE,r12               # r12 = Cached data exists
        b       .wcfa70                 # Continue checking all the other VIDs
#
.wcfa50:
        ldconst TRUE,r9                 # r9 = VID in Error State found
        b       .wcfa70                 # Continue checking all other VIDs
#
.wcfa55:
        and     VC_NO_MORE_DATA,r5,r6   # Get the Disable or Error State Bits
        cmpobe  0,r6,.wcfa70            # Jif if not disable pending or Error
        ld      vc_write_count(r11),r6  # r6 = # of outstanding host write cmds
        cmpobe  0,r6,.wcfa60            # Jif not possible to get more data
        ldconst FALSE,r12               # r12 = cached data could exist
        b       .wcfa70                 # Continue checking all other VIDs
#
# --- There is no data in cache for this VID.
#
#       If the VID is in the Error state, report the error has been recovered
#       and clear the error state.
#
.wcfa60:
        bbc     vc_error,r5,.wcfa65     # Jif the VID is not in Error state
        mov     r13,g0                  # g0 = VID to report the status on
        ldconst 0,g1                    # g1 = No error code
        call    wc$MsgFlushRecovered    # Send the message
        clrbit  vc_error,r5,r5          # Clear the Error state
        stob    r5,vc_stat(r11)         # Save the new status
#
# --- If the VID is in the Disable in Progress state, finish disabling the VID.
#
.wcfa65:
        bbc     vc_disable_ip,r5,.wcfa70 # Jif the VID is not Disable In Progr
        clrbit  vc_disable_ip,r5,r5     # Clear the Disable in progress bit
        clrbit  vc_cached,r5,r5         # Clear the Cache Enabled bit
        stob    r5,vc_stat(r11)         # Save the new status
#
# Report to the MMC that the Flush and Disable is complete for this VID
#
        mov     r13,g0                  # Show the VID completing the Flush
        call    wc$MsgFlushComplete     # Send up the message
#
# End of loop to check for data being flushed - going through each VID
#
.wcfa70:
        cmpinco r4,r13,r13              # Compare to done and increment cntr
        bg      .wcfa45                 # Jif not all VIDs checked yet
        cmpobe  TRUE,r12,.wcfa100       # Jif all data has been flushed (or the
                                        #   only ones left are those in
                                        #   Error State)
        cmpobe  TRUE,r9,.wcfa75         # Jif any VCD is in error state - do
                                        #   not want to deadlock any C$Stop
                                        #   outstanding to flush all data
                                        #   (Background Flush will handle)
        ldob    C_ca+ca_status,r11      # r11 = Cache Status
        bbs     ca_error,r11,.wcfa75    # Jif Write Cache is in an error state
                                        #   - do not want to deadlock any
                                        #   C$Stop from define and doing the
                                        #   Flush without Mirror Partner.
        cmpobne FALSE,r14,.wcfa80       # Jif requested to wait for the flush
                                        #   to complete
.wcfa75:
        ldconst FALSE,g0                # Show not all the data was flushed
        b       .wcfa120                # Return to the caller
#
.wcfa80:
        cmpinco r10,r3,r3               # Compare to max loops and increment cnt
        bg      .wcfa90                 # Jif not at max count yet
#
# Time exceeded to flush all the data to disk.  Report the error and continue.
#   If the requestor asked to wait until all was flushed, keep trying until it
#   is successful.
#
        ldconst cac_sft1,r11            # r11 = error code to log
        lda     C_sft_flt,g0            # g0 = Software Fault Log Area
        st      r11,efa_ec(g0)          # Save the Error Code
        st      r9,efa_data(g0)         # Save the Error State Flag
        st      r12,efa_data+4(g0)      # Save the Flush Complete Flag
        st      r3,efa_data+8(g0)       # Save the 250msec Counter
        st      r10,efa_data+12(g0)     # Save the 250msec Timeout Value
        st      r14,efa_data+16(g0)     # Save the Input Parameter
        ldconst 24,r11                  # Number of bytes saved (ec + data)
        st      r11,mle_len(g0)         # Save the number of bytes to send
        call    M$soft_flt              # Error Trap or Log failure
        mov     0,r3                    # Reset the loop counter and keep trying
#
# Timeout has not been exceeded yet.  Keep waiting for the Flush to complete.
#   Log an event when the flush is taking a long time.
#
.wcfa90:
        ld      K_ii+ii_time,r9         # Get current time
        cmpobl  r9,g9,.wcfa95           # Jif not time to log a message yet
        subo    g8,r9,r9                # Get the number of time since beginning
        addo    g7,g9,g9                # Bump the counter to the next interval
        shro    3,r9,r9                 # Convert back to seconds (1/8sec timer)

        ld      L_stattbl,g4            # Get stat table pointer
        ld      C_ca+ca_tagsDirty,g12   # Get Tags Dirty Count
        ld      C_ca+ca_tagsResident,g5 # Get Tags Resident Count
        ld      C_ca+ca_tagsFlushIP,g1  # Get Tags Flush In Progress Count
        ld      C_ca+ca_blocksDirty,g2  # Get Blocks Dirty Count
        ld      C_ca+ca_blocksResident,g6 # Get Blocks Resident Count
        ld      C_ca+ca_blocksFlushIP,g3 # Get Blocks Flush In Progress Count
        ld      wc_flush_inv_cnt,r8     # Get the Flush and Invalidate Count
        ld      C_orc,g10               # Get the Cache Outstanding Req Cnt
        ld      C_ca+ca_status,g11      # Get the Cache Status, Status2, & Stops
        ldos    ls_vrpocount(g4),g4     # Get VRP outstanding counter
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        st      r9,e_wcflush_time(g0)   # Save the Time of the Flush
        st      g12,e_wcflush_tag_dirty(g0)  # Save the Tags Dirty Count
        st      g5,e_wcflush_tag_res(g0) # Save the Tags Resident Count
        st      g1,e_wcflush_tag_fip(g0) # Save the Tags Flush In Progress Cnt
        st      g2,e_wcflush_blks_dirty(g0) # Save the Blocks Dirty Count
        st      g6,e_wcflush_blks_res(g0) # Save the Blocks Resident Count
        st      g3,e_wcflush_blks_fip(g0) # Save Blocks Flush In Progress Cnt
        st      r8,e_wcflush_inv_cntr(g0) # Save the Flush and Inv Counter
        st      g10,e_wcflush_corc(g0)  # Save number of Outstanding Cache Reqs
        st      g11,e_wcflush_ca(g0)    # Save Cache Status, Status2, & Stops
        st      g4,e_wcflush_orc(g0)    # Save the copy of Link960 ORC
#
        ldconst mleWCFlushlog,r8        # Get the message code
        ldob    K_ii+ii_utzn,g12        # Get the Processor Utilization
        stos    r8,mle_event(g0)        # Set the Message code
        st      g12,e_wcflush_util(g0)  # Set the Processor Utilization
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], e_wcflush_len);

.wcfa95:
        ldconst FLUSH_OPS_WAIT,g0       # g0 = Wait time before flushing more
        call    K$twait                 # Wait for work to be done
        b       .wcfa10                 # Start over again
#
# All Done with the VIDs, determine if the Global Cache is being disabled and
#   handle
#
.wcfa100:
        cmpo    TRUE,r9                 # Was there a VID in Error State?
        sele    TRUE,FALSE,g0           # Return True if no VIDs in Error State
                                        #   - all data was flushed & invalidated
                                        # Return False if one or more VIDs are
                                        #   in Error State
        be      .wcfa120                # Jif devices in error state - Done
        ldob    C_ca+ca_status,r3       # Get the Cache Status
        bbc     ca_shutdown,r3,.wcfa110 # Jif not cache shutdown
        mov     g0,r14                  # Save g0
        ldconst FALSE,g0                # Do not change Signature if not Init'd
        call    WC$markWCacheDis        # Mark cache as flushed
        ldob    C_ca+ca_status,r3       # Get the Cache Status (may have chgd)
        mov     r14,g0                  # Restore g0
#
.wcfa110:
        bbc     ca_dis_ip,r3,.wcfa120   # Jif the Cache is not being disabled
        call    wc$SetGlobalDisable     # Try and finish the disable process
.wcfa120:
        PopRegs(r15)                    # Restore the callers g regs (except g0)
        ret
#
#******************************************************************************
#
#  NAME: wc$FlInvVID
#
#  PURPOSE:
#       To flush and invalidate all write cache data for the input Virtual ID.
#
#  CALLING SEQUENCE:
#       call    wc$FlInvVID
#
#  INPUT:
#       g0 = Virtual ID
#
#  OUTPUT:
#       g0 = Flush Flag
#               TRUE - Flush was able to complete without resource constraints
#               FALSE - Flush was halted due to resource constraints (Flush
#                       count or time)
#
#  REGS DESTROYED:
#       Possibly all G registers.
#
#   REGISTER USAGE:
#        g0 = Parameter passing to called functions
#        g1 = Parameter passing to called functions
#        r3 = Temp register
#        r4 = Temp register
#        r5 = Temp register
#        r6 = Temp register
#        r7 = Number of invalidates completed
#        r8 = Return Value
#        r9 = Number of tree nodes being invalidated
#       r10 = ILT used for the invalidation process
#       r11 = VCD Pointer
#       r12 = Valid Data Root Pointer
#       r13 = Tree Node being processed
#       r15 = Saved g1 register
#
#******************************************************************************
#
        .globl  WC_FlushInvalidateVID
WC_FlushInvalidateVID:
wc$FlInvVID:
        mov     g1,r15                  # Save g1
        ldconst TRUE,r8                 # r8 = Preset return as all done
#
# --- Get root value for this VID
#
        ld      vcdIndex[g0*4],r11      # r11 = VCD pointer for this VID
        ld      vc_cache(r11),r12       # Get the cache RB tree root
        cmpobe  0,r12,.wcfi100          # Jif there is nothing in the tree
#
# --- Get the first entry in the Valid Data tree.
#
        movl    0,g2                    # g2,g3 = Start of drive
        mov     r12,g0                  # g0 = root of RB tree
        call    RB$locateStart          # Get the first entry
#
# --- Perform necessary processing to invalidate tag.
#
        mov     g1,r13                  # r13 = the Tree Node to Invalidate
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        mov     0,r3                    # Clear out the forward and backward
        st      r3,il_fthd(g1)          #  thread pointers
        st      r3,il_bthd(g1)
        mov     g1,r10                  # r10 = ILT used to invalidate tags
#
# Initialize the tag count to 1 so that the ILT does not get released
#   before all the tags have had a chance to be invalidated
#
        ld      K_ii+ii_time,r9         # r9 = current time
        ldconst MAX_FLUSH_INV_TIME,r7   # r7 = maximum time to delete before
                                        #  swapping tasks
        addo    r9,r7,r7                # r7 = time where swap required
        ldconst 1,r9                    # r9 = Number of tags being invalidated
        stos    r9,iocounter(r10)       # Initialize the Number of tags = 1
        stob    r3,ioccode(r10)         # Clear Completion Code of future ops
        lda     .wcfi90,r3              # Completion routine when all tags invl.
        st      r3,il_cr(r10)           # Set ILT completion routine
        lda     .wcfi95,r3              # Intermediate completion routine to
        st      r3,iointcr(r10)         #  keep track of outstanding flushes
#
.wcfi40:
#
# --- Since <wc$FlushRequest> may change the RB tree, we need to find the next
#     possible overlap before invalidating the tag.  The result of the search
#     will be checked after the invalidate to see if any more tags need to
#     be invalidated.
#
        addo    1,r9,r9                 # Increment the number of tags found
        stos    r9,iocounter(r10)       # Save the tag count
        mov     r13,g1                  # g1 = RB pointer for last overlap
        call    RB$locateNext           # Find next tag in the tree, if any
        mov     g1,r3                   # Preserve results of search
#
        ld      rbdpoint(r13),g0        # g0 = cache tag to invalidate
        mov     r10,g1                  # g1 = ILT for request
!       ldos    tg_state(g0),r4         # r4 = State of tag
        ldconst INVALIDATE_REQUEST,g2   # g2 = intent to invalidate
        cmpobne 0,r4,.wcfi50            # Jif locked for anything (background
                                        #  task will pick up - do not want to
                                        #  chew up all the Cacheable DRAM
                                        #  getting Placeholders on a tag that
                                        #  is already being processed).
        call    wc$FlushRequest         # Queue for invalidate
        cmpobe  TRUE,g0,.wcfi60         # Jif tag is being invalidated
.wcfi50:
        subo    1,r9,r9                 # Decrement tag count if done
        stos    r9,iocounter(r10)       # Save tag count
#
.wcfi60:
#
# --- Determine if enough time has elapsed to require a swap (do this by
#       keeping track of how many tags have been invalidated)
#
        ld      K_ii+ii_time,r4         # r4 = current time
        cmpobge r4,r7,.wcfi70           # Jif enough time has elapsed
#
# --- Throttle the number of I/O requests to prevent processor hogging and
#       timing out other tasks
#
        ld      wc_flush_inv_cnt,r4     # r4 = Total Prev Flush & Inv cnt
        ldconst MAX_FLUSH_OPS,r6        # r6 = Max Flush and Invalidate Count
        addo    r9,r4,r5                # r5 = Total Flush & Invalidate Count
        cmpoble r5,r6,.wcfi75           # Jif not time to throttle ops
#
.wcfi70:
        ldconst FALSE,r8                # Show that the flush was terminated
                                        #  due to resource constraints (time,
                                        #  memory, I/O).
        b       .wcfi80                 # All done flushing
#
.wcfi75:
        mov     r3,r13                  # Restore results of next tree node
        cmpobne FALSE,r3,.wcfi40        # Jif another tag was found
#
# --- No more data in cache found, not enough memory to flush more data, or
#       need to throttle the number of ops to allow other tasks to run.
#       Decrement the count (was initialized to one to begin with).
#       If outstanding counter > 0, return.
#
.wcfi80:
        ld      wc_flush_inv_cnt,r7     # Get total Flush & Inv cnt in progress
        cmpdeco 1,r9,r9                 # Decrement the Counter
        addo    r7,r9,r7                # Increment by flush count
        stos    r9,iocounter(r10)       # Save the counter
        st      r7,wc_flush_inv_cnt     # Save the total Flush & Invalidate cnt
        bne     .wcfi100                # Jif tags were queued to be invalidated
        mov     r10,g1                  # g1 = ILT to be released
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        b       .wcfi100                # Return to the caller
#
#
# --- Entry point after all queued invalidate requests complete.
#
#     g0 = error code
#     g1 = ILT for request
#
#       If an error did occur (g0 != 0), the VID went into Error State and
#           reported the error to the CCB.  Nothing else to do here!
#
.wcfi90:
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        ret
#
#
# --- Intermediate Entry point after one queued op has complete.
#       Decrement the outstanding request count to determine if more can be
#       flushed to disk
#
#     g1 = ILT for request
#
#
.wcfi95:
        ld      wc_flush_inv_cnt,r7     # Get total Flush & Inv cnt in progress
        subo    1,r7,r7                 # Decrement the amount left (could go
                                        #  negative if an op completed before
                                        #  the above loop finishes - OK (add of
                                        #  negative to positive works))
        st      r7,wc_flush_inv_cnt     # Save new count
        ret
#
# --- All done
#
.wcfi100:
        mov     r15,g1                  # Restore g1
        mov     r8,g0                   # Return whether the flush was not
                                        #  completed due to resource constraints
        ret
#
#******************************************************************************
#
#  NAME: wc$MsgFlushFailed
#        wc$MsgFlushRecovered
#
#  PURPOSE:
#       To log Flush Failed and Flush Recovered events.
#
#  CALLING SEQUENCE:
#       call    wc$MsgFlushFailed
#       call    wc$MsgFlushRecovered
#
#  INPUT:
#       g0 = Virtual ID
#       g1 = Error code
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       None
#
#   REGISTER USAGE:
#        r4 = Saved g4
#        r5 = Saved g5
#        r6 = Saved g6
#        r9 = Type of message to log
#       r12 = VID reporting the message
#       r13 = Error Code
#       r14 = Saved g2
#       r15 = Saved g3
#
#******************************************************************************
#
wc$MsgFlushRecovered:
        ldconst mlecacheflushrec,r9     # Cache Flush Recovered message
        b       .wcmf10
#
wc$MsgFlushFailed:
        ldconst mlecacheflushfail,r9    # Cache Flush Failed message
.wcmf10:
        mov     g0,r12                  # Save g0
#
# --- Build the message to send
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        stos    r9,mle_event(g0)        # Set the Message code
        stos    r12,ecf_vid(g0)         # Set the VID reporting the message
        stob    g1,ecf_ecode(g0)        # Set the Error Code in the message
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], ecflen);
        mov     r12,g0                  # Restore g0
        ret
#
#
#******************************************************************************
#
#  NAME: wc$MsgFlushComplete
#
#  PURPOSE:
#       To log a Flush Complete message.
#
#  CALLING SEQUENCE:
#       call    wc$MsgFlushComplete
#
#  INPUT:
#       g0 = Virtual ID
#            (0xFFFFFFFF for Global Flush Complete)
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       Possibly all G registers.
#
#   REGISTER USAGE:
#        r4 = Saved g4
#        r5 = Saved g5
#        r6 = Saved g6
#        r9 = Type of message to log
#       r12 = VID reporting the message
#       r13 = Saved g1
#       r14 = Saved g2
#       r15 = Saved g3
#
#******************************************************************************
#
wc$MsgFlushComplete:
c  if (g0 == 0xFFFFFFFF) {
        ret
c }
        mov     g0,r12                  # Save g0
#
# --- Build the message to send
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleflushcomplete,r9     # Cache Flush Complete message
        stos    r9,mle_event(g0)        # Set the Message code
        st      r12,efc_vid(g0)         # Set the VID reporting the message
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], efclen);
        mov     r12,g0                  # Restore g0
        ret
#
#******************************************************************************
#
#  NAME: WC$FlushWOMP
#
#  PURPOSE:
#       Flush the FE Write Cache ignoring the Mirror Partner
#
#  DESCRIPTION:
#       This function will set the "Mirror Broken" status and clear the "Error"
#       status.  Then the task to handle any queued requests due to the error
#       state will be forked.  If cache is enabled, the status will be set to
#       "Disable in Progress" and "Enable Pending" (so it gets restarted when
#       everything is OK again), the Write Cache flushing begun, and the
#       Sequence Number will be incremented to ensure the mirror partners
#       data will not be used (if it comes back somehow).
#
#  CALLING SEQUENCE:
#       call    WC$FlushWOMP
#
#  INPUT:
#       g1 = FALSE  - Flush WOMP is not because of an error sent by the PROC
#            TRUE   - Flush WOMP is in response to an error sent by the PROC
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       None
#
#   REGISTER USAGE:
#        r3 = Failed to Mirror Error Mirror Partner
#        r4 = Global Cache Status
#        r5 = FICB pointer
#        r6 = Write Cache Sequence Number
#        r7 = Current Mirror Partner
#
#        g0 = Parameter passing
#        g1 = Parameter passing
#
#******************************************************************************
#
WC$FlushWOMP:
        movl    g0,r14                  # Save g0-g1
#
# --- Set the Mirror Broken status (if the MP has not been changed already),
#       clear the Error Status and then start the task that will handle all
#       the ops that are waiting to restart (if not already started).
#
        ld      K_ficb,r7               # r7 = K_ficb pointer
        ld      gWCErrorMP,r3           # Get the Error Mirror Partner SN
        ldob    C_ca+ca_status,r4       # r4 = The Cache Status
        ld      fi_mirrorpartner(r7),r7 # r7 = Current Mirror Partner SN
        cmpobe  0,r3,.wcfwomp05         # Jif No Error Mirror Partner SN saved
        cmpobne r3,r7,.wcfwomp07        # Jif another MP already Assigned
.wcfwomp05:
        setbit  ca_mirrorbroken,r4,r4   # Set the Mirror Broken status
.wcfwomp07:
        clrbit  ca_error,r4,r4          # Clear the Error Status
        cmpobe  FALSE,r15,.wcfwomp09    # Jif call not because of an Error Sent
        ldconst 0,r3                    # Prep to clear the Saved Error MP SN
        st      r3,gWCErrorMP           # Clear the Saved Error MP SN
.wcfwomp09:
        stob    r4,C_ca+ca_status       # Save the new Cache Status
        lda     c$error,g0              # Prepare to start the Error Task
        ldconst CMIRRORPRI,g1
c       CT_fork_tmp = (ulong)"c$error";
        call    K$tfork
#
# --- Kick off the Clear Write Info task to clear out any that could be
#       waiting for mirroring to start
#
        ld      C_cwi_pcb,r6            # r6 = Clear Write Info Task pcb
        ldob    pc_stat(r6),r5          # r5 = Current process status
        cmpobne pcnrdy,r5,.wcfwomp10    # Jif status is not Not Ready -
                                        #   already active
        ldconst pcrdy,r7                # r7 = Process ready status
.ifdef HISTORY_KEEP
c CT_history_pcb(".wcfwomp09 setting ready pcb", r6);
.endif  # HISTORY_KEEP
        stob    r7,pc_stat(r6)          # Ready process
.wcfwomp10:
#
        ldconst 8,r5                    # Wait up to 1 second for the host ops
                                        #  that were queued to get started again
        ldconst 125,g0                  # Wait 125 msec to let error task run
.wcfwomp20:
        call    K$twait                 # Wait to let ops complete that have
                                        #  been waiting a long time
        subo    1,r5,r5                 # Decrement the loop counter
        ld      C_error_cqd,r6          # r6 = Number of ops on error task queue
        cmpobe  0,r5,.wcfwomp40         # Jif waited 1 sec
        cmpobne 0,r6,.wcfwomp20         # Jif nothing is on the error task queue
#
# --- Determine if Write Cache is Enabled.  If so, set the "Disable in Progress"
#       and "Enable Pending" status and Flush the Cache.
#
.wcfwomp40:
        ldob    C_ca+ca_status,r4       # r4 = The Cache Status (may have chgd)
        bbc     ca_ena,r4,.wcfwomp80    # Jif Write Cache is not Enabled
        setbit  ca_dis_ip,r4,r4         # Set the Disable in Progress bit
        setbit  ca_ena_pend,r4,r4       # Set the Enable Pending
        stob    r4,C_ca+ca_status       # Save the new Cache Status
        ldconst FALSE,g0                # g0 = Do not wait for flush to complete
        call    wc$FlInvAll             # Begin the Flush process
#
# --- Increment the Write Cache Sequence Number to ensure no data is used from
#       the Mirror Partner
#
        ld      K_ficb,r5               # r5 = FICB
        ld      fi_seq(r5),r6           # r6 = Sequence number
        addo    1,r6,r6                 # Increment sequence number
        st      r6,fi_seq(r5)           # Store sequence number
        call    wc$setSeqNo             # Set Sequence Number
        b       .wcfwomp100             # All done - return
#
# --- Cache was not enabled, but the CCB needs to know that the Flush completed
#       and the Error State was taken care of and wants the flush complete
#       message sent.
#
.wcfwomp80:
        ldconst 0xFFFFFFFF,g0           # Show the Global Cache Complete
        call    wc$MsgFlushComplete     # Report the completion
#
# --- All Done!!
#
.wcfwomp100:
        movl    r14,g0                  # Restore g0-g1
        ret
#
#******************************************************************************
#
#  NAME: WC$FlushBE
#
#  PURPOSE:
#       Flush the BE Write Cache
#
#  DESCRIPTION:
#       If a VID List is provided, these are checked for validity before
#       continuing.  If all is OK (VIDs in VID List are valid or Global), then
#       this function will set up the Write Cache Trees for the Specified VID(s)
#       or all VIDs found in the BE Write Cache.  It will then set these VID(s)
#       to Disable In Progress and start the flushing for specified VID(s).
#
#  CALLING SEQUENCE:
#       call    WC$FlushBE
#
#  INPUT:
#       g0 = Parameter pointer for MRP
#
#  OUTPUT:
#       g0 = MRP Return Code
#               deok (0x00) - Function completed successfully
#               deinvvid (0x030) - At least one VID was not found that
#                   was requested
#       g1 = Invalid VID if g0 = deinvvid otherwise trashed
#
#  REGS DESTROYED:
#       g0-g1
#
#   REGISTER USAGE:
#        r3 = MRP Options flag
#        r4 = Index into VID List
#        r5 = VCD pointer for VID
#        r6 = VCD status for VID
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
WC$FlushBE:
        mov     g0,r15                  # Save Parameter pointer
        ldconst MAXVIRTUALS,r14         # r14 = maximum VID + 1
        ldconst TRUE,r11                # VID Valid in VID array
#
# --- Determine if this is a global BE flush or list of VIDs to flush
#       and handle with the appropriate code.
#
        ldob    mfb_op(g0),r3           # r3 = Options flag
        cmpobne mfboinvalvid,r3,.wc_flushbe500 # Jif it is not a VID list
#
# --- List of VIDs to flush ---------------------------------------------------
#
#   Verify all the VIDs in the list are valid
#
        ldos    mfb_nvids(r15),r13      # r13 = Number of VIDs to process
        lda     mfb_vidlist(r15),r12    # r12 = Pointer to VID list
        ldconst 0,r4                    # r4 = Offset into VID list
.wc_flushbe100:
        cmpobe  r4,r13,.wc_flushbe200   # Jif all VIDs tested good
        ldos    (r12)[r4*2],g1          # g1 = VID being tested
        cmpobge g1,r14,.wc_flushbe190   # Jif the VID is too big
        ld      vcdIndex[g1*4],r5       # r5 = VCD pointer
        cmpobe  0,r5,.wc_flushbe190     # Jif undefined
        addo    1,r4,r4                 # Point to the next VID in the VID list
        b       .wc_flushbe100          # Check the next VID in the VID list
#
#   An invalid VID was found, report the error to the requester and return
#
.wc_flushbe190:
        ldconst deinvvid,g0             # Show that an Invalid VID was found
                                        # g1 = VID that failed the test
        b       .wc_flushbe900          # return the status
#
#   VID List tested good, create the VID array for the BE to build the trees
#       for, create the BE VID Write Cache Trees, and then begin the Flush for
#       any data for those specified VIDs
#
.wc_flushbe200:
c       r10 = s_MallocC(r14, __FILE__, __LINE__); # Allocate space for one byte / possible VID array
        ldconst 0,r4                    # r4 = Reset offset into VID list
.wc_flushbe220:
        cmpobe  r4,r13,.wc_flushbe240   # Jif all VIDs have been processed
        ldos    (r12)[r4*2],g0          # g0 = VID to flush data for
        stob    r11,(r10)[g0*1]         # Show the VID is valid to restore tree
        addo    1,r4,r4                 # Point to the next VID
        b       .wc_flushbe220          # Go process the next VID
#

.wc_flushbe240:
        ldconst 0,g0                    # g0 = VID List provided
        mov     r10,g1                  # g1 = Pointer to VID array
        call    wc$processMirroredTags  # Go build the VID Write Cache Trees
#
        ldconst 0,r4                    # r4 = Reset offset into VID list
.wc_flushbe300:
        cmpobe  r4,r13,.wc_flushbe800   # Jif all VIDs have been processed
        ldos    (r12)[r4*2],g0          # g0 = VID to invalidate data for
        ld      vcdIndex[g0*4],r5       # r5 = VCD pointer
        ldob    vc_stat(r5),r6          # r6 = VCD status
        setbit  vc_cached,r6,r6         # Ensure it shows cached data
        setbit  vc_disable_ip,r6,r6     # Show that a Disable is in progress
        stob    r6,vc_stat(r5)          # Save the new status
        call    wc$FlInvVID             # Begin the Flush of data and let the
                                        #  background flush task send up the
                                        #  flush complete message
        addo    1,r4,r4                 # Point to the next VID
        b       .wc_flushbe300          # Go process the next VID
#
# --- Global Flush of BE Write Cache ------------------------------------------
#
.wc_flushbe500:
#
# --- Build the Write Cache Trees for all valid VIDs in the BE and then begin
#       to flush any data found in them.
#
c       r10 = s_MallocC(r14, __FILE__, __LINE__); # Allocate space for one byte / possible VID array
#
        mov     1,g0                    # g0 = Global Flush (all BE VIDs)
        mov     r10,g1                  # g1 = Pointer to VID array
        call    wc$processMirroredTags  # Go build the VID Write Cache Trees
                                        #  and flag in the Array those found
#
        ldconst -1,r4                   # r4 = Reset offset into VID array
                                        #  (init to -1 because next loop
                                        #  pre-increments)
.wc_flushbe510:
        addo    1,r4,r4                 # increment the VID array pointer (VID)
        cmpobe  r4,r14,.wc_flushbe590   # Jif no more VIDs to check
        ldob    (r10)[r4*1],r5          # r5 = Valid VID flag
        cmpobne r11,r5,.wc_flushbe510   # Jif not a valid VID
#
#   VID found, set the VID to Disable in progress and then begin the flush of
#       data for it.
#
        ld      vcdIndex[r4*4],r5       # r5 = VCD pointer
        ldob    vc_stat(r5),r6          # r6 = VCD status
        setbit  vc_cached,r6,r6         # Ensure it shows cached data
        setbit  vc_disable_ip,r6,r6     # Show that a Disable is in progress
        stob    r6,vc_stat(r5)          # Save the new status
        mov     r4,g0                   # Input g0 = VID to flush data for
        call    wc$FlInvVID             # Begin the flush of data
        b       .wc_flushbe510          # Continue flushing data
#
#   VID Flushing began.  Now make sure the two caches are out of sync.
#
.wc_flushbe590:
        ld      WcctAddr,r3             # r3 = Cache control table address
        lda     BE_ADDR_OFFSET(r3),r3   # Convert to the BE to read/write
!       ld      wt_seq(r3),r4           # r4 = Current Sequence Number
        subo    2,r4,r4                 # Decrement by two to ensure out of sync
!       st      r4,wt_seq(r3)           # Store the new BE Sequence Number
        PushRegs(r4)
        ld      WcctAddr,g0             # g0 = Destination Cache Control Table
        mov     r3,g1                   # g1 = Source
        ldconst MIRROR_BE,g2            # g2 = Mirror Destination - BE
        ldconst wtsize,g3               # g3 = Length to transfer
        call    WC_CopyNVwWait          # Copy to NV Memory (BE area)
        PopRegsVoid(r4)
#
# --- All done starting the flush for all the VID data, Free the VID array
#       and set up the return parameters.
#
.wc_flushbe800:
#
c       s_Free(r10, r14, __FILE__, __LINE__);
#
        ldconst deok,g0                 # Show the Flush started OK
        ldconst 0xFFFF,g1               # Show invalid VID number
#
# --- All Done
#
.wc_flushbe900:
        ret
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
