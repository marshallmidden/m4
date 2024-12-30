# $Id: wcutils.as 143352 2010-06-28 13:05:33Z m4 $
#******************************************************************************
#
#  NAME: wcutils.as - Write Cache Utilities
#
#  PURPOSE:
#       To provide miscellaneous Write Cache utility functions.
#
#  Copyright (c) 2000-2008 Xiotech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- global data declarations ----------------------------------------
#
        .globl  c_tgfree                # Free cache tag list
#
# --- Local data definitions
#
        .text
#
#******************************************************************************
#
#  NAME: wc$masg_ctag
#
#  PURPOSE:
#       Assign a cache tag.
#
#  DESCRIPTION:
#       This routine returns a pointer to a free cache tag.  It either takes
#       a placeholder from the freelist or if none exist it attempts to
#       invalidate a resident tag from the tail of the LRU queue.  If this
#       cannot be accomplished zero (0) is returned as the cache tag address.
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g0 = pointer to cache tag, 0 if cannot find free tag.
#
#******************************************************************************
#
        .globl wc$masg_ctag
wc$masg_ctag:
        ld      c_tgfree,g0             # Check freelist
        cmpobne 0,g0,.wctg80            # Jif cache tags on freelist
#
# --- Attempt to invalidate a resident tag
#
        lda     c_hlruq,r15             # Get addr of head sentinel
        ld      c_tlruq+tg_bthd,g0      # Get cache tag on tail of list
.wctg10:
        cmpobe  g0,r15,.wctg90          # Jif nothing available on LRU queue
#
# --- Check tag state for any lock
#
        ldos    tg_state(g0),r4         # Get tag state
        cmpobe  0,r4,.wctg20            # Jif not locked
        ld      tg_bthd(g0),g0          # Get the Previous tag on the list
        b       .wctg10
#
# --- This tag should be OK to reclaim.  Invalidate it.
#
.wctg20:
        call    wc$invalidate_tag       # Perform invalidation
        b       wc$masg_ctag            # Take item from top of freelist,
                                        #  it should now have 1 item on it
#
.wctg80:
        ld      tg_fthd(g0),r3          # Unlink from chain
        movl    0,r4
        st      r3,c_tgfree
        stl     r4,tg_fthd(g0)          # Clear forward/backward ptrs
        stos    r4,tg_attrib(g0)        # Clear all tag attribute
        stos    r4,tg_state(g0)         #  and all tag state bits
        ret
#
.wctg90:
        ldconst 0,g0                    # Designate no free tag
        ret
#
#******************************************************************************
#
#  NAME: wc$mrel_ctag
#
#  PURPOSE:
#       Release a cache tag.
#
#  DESCRIPTION:
#       This routine links a cache tag to the freelist if it is not a BE tag.
#
#  INPUT:
#       g0 = pointer to cache tag.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
        .globl  wc$mrel_ctag
wc$mrel_ctag:
        ldconst FE_BASEADDR,r4          # Determine if this is a BE address
        ldconst FE_SHARE_LIM,r5
        cmpobl  g0,r4,.wc_mrelctag100   # Jif the addr is not in the FE range
        cmpoble r5,g0,.wc_mrelctag100
#
        setbit  TG_FREE,0,r6            # Show the Tag as free (could be left
                                        #  over garbage)
        stos    r6,tg_attrib(g0)        # Set Tag as free
        stos    0,tg_state(g0)          # Set Tag as "Not Locked"
        ld      c_tgfree,r3             # get freelist origin
        st      r3,tg_fthd(g0)          # Link cache tag to beginning of
        st      g0,c_tgfree             #  chain
.wc_mrelctag100:
        ret
#
#******************************************************************************
#
#  NAME: wc$q
#
#  PURPOSE:
#       Queue an ILT/placeholder to a particular write cache process.
#
#  DESCRIPTION:
#       This routine accepts a pointer to an ILT or placeholder and a
#       write cache control structure pointer.  The indicated ILT/placeholder
#       is then added to the queue for this process and the process is
#       awakened (if necessary).
#
#  INPUT:
#       g0 = queue control structure address.
#       g1 = ILT or placeholder to queue.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
        .globl  wc$q
wc$q:
        ldq     qu_head(g0),r4          # get head/tail/cnt/PCB of queue
        cmpobne 0,r4,.wcq10             # Jif head of queue already established
#
        st      g1,qu_head(g0)          # Set head of queue
        b       .wcq20
#
.wcq10:
        st      g1,il_fthd(r5)          # Set forward link of last ILT/placehldr
.wcq20:
        addo    1,r6,r6                 # Increment the number of items on queue
        st      g1,qu_tail(g0)          # Set new tail of queue
        st      r6,qu_qcnt(g0)          # Save the new count
#
        cmpobe  0,r7,.wcq30             # If queue not initialized (no task)
        ldob    pc_stat(r7),r9          # Get status of handler task
        cmpobne pcnrdy,r9,.wcq30        # Jif handler not asleep
        ldconst pcrdy,r8
.ifdef HISTORY_KEEP
c CT_history_pcb(".wcq20 setting ready pcb", r7);
.endif  # HISTORY_KEEP
        stob    r8,pc_stat(r7)          # Awaken task
.wcq30:
        st      0,il_fthd(g1)           # Clear forward thread
        ret                             # Done!
#
#******************************************************************************
#
#  NAME: wc$masg_cbuffer
#
#  PURPOSE:
#       Assign a cache buffer of the requested size.
#
#  DESCRIPTION:
#       This routine accepts a size in sectors for a cache buffer
#       to be allocated.  If the requested buffer cannot be assigned
#       the returned pointer is null (zero).
#
#  INPUT:
#       g0 = size in sectors.
#
#  OUTPUT:
#       g0 = Buffer address; zero if cannot allocate.
#
#  REGS DESTROYED:
#       g0
#       g1
#
#******************************************************************************
#
        .globl  wc$masg_cbuffer
wc$masg_cbuffer:
        shlo    9,g0,g0                 # Multiply by sector size
        mov     g0,r3                   # r3 = the byte count
c       g0 = k_malloc(g0, (struct fmm *)&c_wcfmm, __FILE__, __LINE__); # Allocate memory without waiting.
        cmpobne 0,g0,.wcgcb90           # Jif a buffer was allocated
#
# --- A buffer was not allocated.  Free up some Resident buffers to try and
#       get a buffer
        shlo    1,r3,r4                 # r4 = Free up twice the size requested
        lda     c_wcfms,r5              # r5 = Addr of the wc buffer FMS
        ldconst FALSE,r6                # r6 = Flag to show buffers released
#
# --- Attempt to invalidate a resident tag and its buffer
#
.wcgcb20:
        lda     c_hlruq,r15             # Get addr of head sentinel
        ld      c_tlruq+tg_bthd,r14     # Get cache tag on tail of list
.wcgcb30:
        cmpobe  r14,r15,.wcgcb80        # Jif nothing available on LRU queue
#
# --- Check tag state for any lock
#
        ldos    tg_state(r14),r7        # Get tag state
        cmpobe  0,r7,.wcgcb40           # Jif not locked
        ld      tg_bthd(r14),r14        # Get the Previous tag on the list
        b       .wcgcb30
#
# --- This tag should be OK to reclaim.  Invalidate it.
#
.wcgcb40:
        mov     r14,g0                  # g0 = Cache Tag to invalidate
        call    wc$invalidate_tag       # Perform invalidation
        ldconst TRUE,r6                 # r6 = Some buffers were released
        ld      fs_cur(r5),r7           # r7 = Cache Buffers Space available
        cmpobg  r4,r7,.wcgcb20          # Jif more needs to be released
#
# --- Try and get the buffer again
#
.wcgcb80:
        cmpobe  FALSE,r6,.wcgcb90       # Jif nothing was released
        mov     r3,g0                   # g0 = size of buffer to allocate
                                        # g1 = Write Cache Buffers FMM
c       g0 = k_malloc(g0, (struct fmm *)&c_wcfmm, __FILE__, __LINE__); # Allocate memory without waiting.
#
# Return with or without a buffer
#
.wcgcb90:
        ret
#
#******************************************************************************
#
#  NAME: wc$mrel_cbuffer
#
#  PURPOSE:
#       Release a cache buffer of the specified size.
#
#  DESCRIPTION:
#       This routine accepts a write cache buffer address and size, and puts
#       the indicated buffer into the write cache free pool.
#
#  INPUT:
#       g0 = Address of buffer to release
#       g1 = Size in sectors of the buffer
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
        .globl  wc$mrel_cbuffer
wc$mrel_cbuffer:
# Do not do anything with memory not in the FE. Specifically the BE passed in memory is to be ignored.
c       if (g0 >= FE_BASEADDR && g0 < FE_SHARE_LIM) {
# Free write cache memory.
c           k_mrel((void *)g0, g1 << 9, (struct fmm *)&c_wcfmm, __FILE__, __LINE__);
c       }
        ret
#
#******************************************************************************
#
#  NAME: wc$top_lru_queue
#
#  PURPOSE:
#       To add or move a cache tag to the top of the LRU queue.
#
#  DESCRIPTION:
#       This routine accepts the address of a cache tag and either adds
#       it to the top of the list or moves it from somewhere else in the
#       list to the top.  This is determined by inspecting the thread
#       pointers in the cache tag; if they are initialized it is assumed
#       that the tag is already linked onto the list.
#
#       The LRU queue is managed as a doubly linked list using the
#       <tg_fthd> and <tg_bthd> pointers in the cache tag.
#
#       The head of the queue is anchored by <tg_fthd> of <c_hlruq>;
#       the tail is anchored by <tg_bthd> of <c_tlruq>.  Note that
#       when the queue is empty <tg_fthd> of <c_hlruq> must point to
#       <c_tlruq>; <tg_bthd> of <c_tlruq> must also point to <c_hlruq>.
#
#       The above initialization must be complete before this routine is called!
#
#  INPUT:
#       g0 = cache tag to process.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
# --- Head and tail anchors must be linked together, denoting an empty queue,
#     before this routine is called for the first time.
#
        .globl  wc$MoveToResidentMRU
wc$MoveToResidentMRU:
        .globl  wc$AddToResidentList
wc$AddToResidentList:
!       ldl     tg_fthd(g0),r4          # Get cache tag fw/bw pointers
        lda     c_hlruq,r15             # Get current head of queue anch
        cmpobe  0,r4,.wctlq10           # Jif not on list
#
# --- Remove this item from list.
#
        st      r4,tg_fthd(r5)          # Set forward ptr of prev node
        st      r5,tg_bthd(r4)          # Set backward ptr of next node
.wctlq10:
        ld      tg_fthd(r15),r6         # Get current head ptr
#
# --- Add this item to top of list.
#
!       st      r6,tg_fthd(g0)          # Setup forward pointer
!       st      r15,tg_bthd(g0)         #   and backward ptr of ILT
!       st      g0,tg_bthd(r6)          # Setup backward pointer of next node
        st      g0,tg_fthd(r15)         # Link onto list
        ret                             # Done!
#
#******************************************************************************
#
#  NAME: wc$remove_lru_queue
#
#  PURPOSE:
#       To remove a cache tag from the LRU queue.
#
#  DESCRIPTION:
#       This routine accepts the address of a cache tag and removes it
#       from the doubly linked LRU list.  The forward and backward
#       pointers of the tag are then cleared.
#
#       The LRU queue is managed as a doubly linked list using the
#       <tg_fthd> and <tg_bthd> pointers in the cache tag.
#
#       This routine checks for initialized pointers in the cache tag.
#       If the forward or backward thread pointers are zero, no action
#       is taken.
#
#       Note that since the head and tail pointers are actually sentinels
#       for the the list, the forward and backward thread pointers for
#       a particular node on the list will never be zero (nodes at either
#       end of the list point to a sentinel).
#
#  INPUT:
#       g0 = cache tag to process.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
        .globl  wc$remove_lru_queue
wc$remove_lru_queue:
!       ldl     tg_fthd(g0),r4          # Get cache tag fw/bw pointers
        movl    0,r6
        cmpobe  0,r4,.wcrmq10           # Jif not on list
        cmpobe  0,r5,.wcrmq10           # Jif not on list
#
# --- Remove this item from list.
#
!       st      r4,tg_fthd(r5)          # Set forward ptr of prev node
!       st      r5,tg_bthd(r4)          # Set backward ptr of next node
#
.wcrmq10:
!       stl     r6,tg_fthd(g0)          # Clear forward/backward ptrs
        ret                             # Done!
#
#******************************************************************************
#
#  NAME: wc$qtag_unlock
#
#  PURPOSE:
#       To wait until a particular tag is unlocked.
#
#  DESCRIPTION:
#       This routine queues the supplied placeholder/ILT to the supplied
#       cache tag unlock queue.  When the tag is unlocked the placeholder
#       is unlinked from the queue and the completion routine within it
#       is then called.
#
#       Assumption:  Tag is locked when this routine is called; otherwise
#                    it might be a long wait before the placeholder is
#                    processed.
#
#  INPUT:
#       g0 = tag to queue placeholder.
#       g1 = placeholder.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
        .globl  wc$qtag_unlock
wc$qtag_unlock:
#
# --- Check to see if the Tag Locked Queue is currently empty.
#
!       ld      tg_tqueue(g0),r3        # Get tail of tag queue
        cmpobne 0,r3,.wcqu10            # Jif queue already established
#
# --- The Tag Locked Queue is currently empty.
#     Set both the head and tail pointers to this new Placeholder ILT
#     since it will now be the only entry in the queue.
#
!       st      g1,tg_hqueue(g0)        # Queue empty; setup as head of
        b       .wcqu20                 #  queue, branch to add to tail
#
.wcqu10:
# --- The Tag Locked Queue is not currently empty.
#     Add this new Placeholder ILT to the tail of the queue.
#     The forward thread pointer for the current tail of the list is changed
#     to point to the new entry which becomes the new tail of the queue.
#
        st      g1,il_fthd(r3)          # Link to queue (if queue exists)
#
.wcqu20:
# --- The tail is change to point to the new entry.
#     Initialize the forward thread pointer for this entry that was added to
#     the tail of the queue to NULL to indicate the end of the queue.
#
!       st      g1,tg_tqueue(g0)        # Add Placeholder to tail of queue
        ldconst 0,r15
        st      r15,il_fthd(g1)         # Clear forward thread for new entry
        ret
#
#******************************************************************************
#
#  NAME: wc$unlock_tag
#
#  PURPOSE:
#       To perform post-unlock processing for a cache tag.
#
#  DESCRIPTION:
#       This routine, given an already unlocked tag, calls the
#       completion routine of queued ILTs/placeholders that are waiting
#       for the tag to be unlocked (if any).  It processes each entry
#       on the list one item at a time until one of the following
#       events occur:
#         1) The end of the queue is reached.
#         2) The tag changes state from unlocked to locked and the type
#            of request (see <iltdefs.inc>) conflicts with the lock type.
#
#            Conflicts:
#              INVALIDATE_REQUEST and any type of lock
#              FLUSH_REQUEST      and locked for either flush or write
#
#  INPUT:
#       g0 = tag to process.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g1
#
#  ATTENTION: does callx to completion routine.
#
#******************************************************************************
#
        .globl  wc$unlock_tag
wc$unlock_tag:
#
# --- Check for queued placeholder/ILTs waiting for unlock
#
!       ld      tg_hqueue(g0),g1        # Get first item on queue
!       ldos    tg_state(g0),r3         # Get tag state
#
        mov     g0,r15                  # Preserve tag address
        lda     0,r14
#
.wcut10:
        cmpobe  0,g1,.wcut30            # Jif nothing on queue
#
        ld      il_fthd(g1),r5          # Get next placeholder/ILT on list
        ld      plintent(g1),r6         # Get intent of placeholder
        ld      il_cr(g1),r7            # Get completion routine

#
# --- If the tag is not locked, then this placeholder can be removed from
#     the locked queue and processed immediately.
#     If the tag is still locked for either an invalidate, flush, or Write
#     (TGM_LOCKED_NOFLUSH) then this placeholder can not be processed and
#     must remain on the locked queue.  Otherwise, the tag must still be
#     locked for a read hit.  If locked for a read hit, then check the intent
#     of the placeholder.  If the placeholder intent is an invalidate request,
#     then the placeholder can not be processed and must remain on the locked
#     queue.  Otherwise the placeholder intent must be for a flush request
#     (since a write request placeholder isn't possible since a write can not
#     overlap an outstanding read hit).  For this case the placeholder can be
#     removed from the locked queue and processed immediately.
#
#   if ((tag->state & TG_LOCKED_NOFLUSH ) ||
#      ((reqType == INVALIDATE_REQUEST) && (tag->state & TG_LOCKED_READHIT)) )
#           return();
#
        cmpobe  0,r3,.wcut20            # Jif tag not locked, process PH
        and     r3,TGM_LOCKED_NOFLUSH,r4    # Test locked for No Flush bits
        cmpobne 0,r4,.wcut40                # Jif locked for No Flush, exit
        cmpobe  INVALIDATE_REQUEST,r6,.wcut40 # Jif PHintent = Invalid Req,exit
#
# --- This placeholder can be processed.  Unlink it from queue and call its
#     completion routine.
#
.wcut20:
!       st      r5,tg_hqueue(r15)       # Remove placeholder from queue
        st      r14,il_fthd(g1)         # Clear forward link
        cmpobne 0,r5,.wcut25            # Jif this is not the last in the list
!       st      r14,tg_tqueue(r15)      # Clear the tail pointer
.wcut25:
        ldconst 0,g0                    # Show that this part of the op is OK
        callx   (r7)                    # Call completion routine
#
!       ld      tg_hqueue(r15),g1       # Get head of queue
!       ldos    tg_state(r15),r3        # Get tag state
        b       .wcut10                 # Process remaining items on queue
#
.wcut30:
# --- Done processing the tag locked queue.
#     Check to see if the state of the tag is Free Pending.  If it is,
#     change the attributes to Free and release the tag to the Free List.
#
!       ldos    tg_attrib(r15),r3       # Get tag attributes
        bbc     TG_FREE_PENDING,r3,.wcut40 # Jif not Free Pending
        setbit  TG_FREE,0,r3            # Set to free state
!       stos    r3,tg_attrib(r15)       # Store attribute bits.
        mov     r15,g0                  # g0 = cache tag input
        b       wc$mrel_ctag            # Release tag to free pool/return
#
.wcut40:
        ret                             # Done
#
#******************************************************************************
#
#  NAME: wc$invalidate_tag
#
#  PURPOSE:
#       To invalidate a cache tag.
#
#  DESCRIPTION:
#       This routine accepts the address of a particular cache tag and
#       invalidates it. The supplied tag must be unlocked and not dirty.
#       An error will occur if the above conditions do not hold.
#
#       The tag is removed from the cache tag tree upon invalidate.
#
#       Upon exit, the tag has been invalidated and added to the free list.
#
#  CALLING SEQUENCE:
#       call    wc$invalidate_tag
#
#  INPUT:
#       g0 = Cache tag to invalidate.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
        .globl  wc$invalidate_tag
wc$invalidate_tag:
        mov     g0,r15                  # Preserve cache tag
        mov     g1,r14                  # Preserve g1
#
# --- Sanity Check, ensure the tag is resident and not dirty.
#
!       ldos    tg_attrib(g0),r13       # Get tag attributes
#
        bbs     TG_DIRTY,r13,.wctierr   # Jif dirty
        bbc     TG_RESIDENT,r13,.wctierr # Jif not resident
#
# --- Perform some sanity checks against the cache tag.
#
!       ld      tg_ioptr(g0),r5         # Get cache tree pointer
!       ld      tg_dirtyptr(g0),r6      # Sanity check - get dirty tree ptr
!       ld      tg_hqueue(g0),r8        # Get head of locked queue
#
# --- If:
#       1). Cache tree ptr == 0
#       2). Dirty tree ptr != 0
#
#       Generate an error.  Tag should be in cache tree, not in dirty tree.
#
        cmpobe  0,r5,.wctierr           # Jif not in cache tree
        cmpobne 0,r6,.wctierr           # Jif still in dirty tree
#
# --- Get sizeof request and buffer pointer.
#
!       ldl     tg_vlen(g0),r6          # r6 = length r7 = buffer ptr
#
# --- Set invalid bits, remove from cache tree, and add to freelist.
#
        setbit  TG_FREE,0,r3            # Set to free state
        mov     0,r4
!       stos    r3,tg_attrib(g0)
!       stos    r4,tg_state(g0)         # Set state to zero
!       st      r4,tg_ioptr(g0)         # Clear pointer to RB node
!       st      r4,tg_vlen(g0)          # Clear vlen/buffer pointer
!       st      r4,tg_bufptr(g0)        # Clear buffer pointer
#
# --- Remove from LRU list.
#
        call    wc$remove_lru_queue
#
# --- Remove from cache tag tree.
#
!       ldos    tg_vid(g0),g0           # Get VID for this tag
        ld      vcdIndex[g0*4],r12      # Retrieve VCD pointer for this VID
        ld      vc_cache(r12),g0        # Get root of the tree
        mov     r5,g1                   # g1 = node to delete
                                        # g0 = root of tree
        call    RB$delete               # Remove it from tree
        st      g0,vc_cache(r12)        # Save root ptr
#
# --- Decrement the Resident count and Increment the Free counts (if this is
#       not a BE Tag)
#
        bbs     TG_BE,r13,.wcti10       # Jif this is a BE Tag (not keeping
                                        #  track of BE tags)
        mov     r6,g0                   # g0 = Block count
        call    wc$DecResidentCount     # Dec the Resident tag and block count
        call    wc$IncFreeCount         # Increment the Free tag and block count
                                        # NOTE: Free Pending is considered FREE
#
# --- Deallocate tree element
#
.wcti10:
c       put_wc_rbnode(r5);              # Deallocate tree element
#
# --- Deallocate buffer. r6 = length r7 = buffer pointer
#
        mov     r7,g0
        mov     r6,g1
        call    wc$mrel_cbuffer         # Release buffer
#
# --- Restore g0 and g1
#
        mov     r15,g0                  # g0 = cache tag to free
        mov     r14,g1                  # Restore g1
#
# --- Check to ensure that no placeholders are waiting for tag unlock
#     before release the tag to the free list.
#     may call this routine.
#
        cmpobne 0,r8,.wcti20            # Jif head ptr nonzero
#
# --- Link tag to freelist
#
        b       wc$mrel_ctag            # Release it to free pool/return
#
.wcti20:
# --- The Tag Locked Queue is not empty.  Set the free pending bit and finish
#     the release of the tag to the free list after the locked queue is empty.
#
        setbit  TG_FREE_PENDING,0,r3    # Set to Free Pending state
        stos    r3,tg_attrib(g0)
        ret
#
# --- Error trap point - someone is crunching memory
#
.wctierr:
        cmpo    0,0
        faulte
#
#******************************************************************************
#
#  NAME: wc$IncDirtyCount
#
#  PURPOSE:
#       To increment the count of dirty cache tags and blocks.
#
#  DESCRIPTION:
#       This routine accepts the block count for a new dirty cache tag
#       and update the dirty block count and increments the count of
#       dirty cache tags.  It then check to see if the flush threshold
#       has been exceeded for either of this counts and activates the
#       BackgroundFlushTask if necessary.
#
#  INPUT:
#       g0 = Block Count for new dirty cache tag.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
# void IncDirtyCount(unsigned int count)
        .globl  wc$IncDirtyCount
wc$IncDirtyCount:
#
# --- Increment the count of dirty cache tags and blocks.
#
#   c_tagsDirty++;
#   c_blocksDirty += count;
#
        lda     C_ca,r5
        ld      ca_tagsDirty(r5),r7
        ld      ca_blocksDirty(r5),r6
        addo    r7,1,r7
        st      r7,ca_tagsDirty(r5)
        addo    r6,g0,r6
        st      r6,ca_blocksDirty(r5)
#
# --- Check to see if either the dirty tags or block count threshold has
#     been exceeded.  If it has, then active the Background Flush Task.
#
#   if ((c_tagsDirty >= C_TagStartFlushThresh) ||
#       (c_blocksDirty >= C_BlockStartFlushThresh))
#   {
#       bgflush.pcb = TRUE;
#   }
        ld      C_TagStartFlushThresh,r9
        cmpobge r7,r9,.wcidc10          # Jif tagsDirty >= Start Threshold
        ld      C_BlockStartFlushThresh,r8
        cmpobl  r6,r8,.wcidc100         # Jif blocksDirty < Start Threshold
.wcidc10:
        ld      c_bgflush,r3            # Load pointer to PCB
        cmpobe  0,r3,.wcidc100          # Jif no PCB
        ldob    pc_stat(r3),r4          # Get status of handler task
        cmpobe  pcnrdy,r4,.wcidc20      # Jif handler is asleep
        cmpobne pctwait,r4,.wcidc100    # Jif handler is not in a Time Wait
.wcidc20:
        ldconst pcrdy,r5
.ifdef HISTORY_KEEP
c CT_history_pcb(".wcidc20 setting ready pcb", r3);
.endif  # HISTORY_KEEP
        stob    r5,pc_stat(r3)          # Awaken task
#
.wcidc100:
        ret
#
#******************************************************************************
#
#  NAME: wc$DecDirtyCount
#
#  PURPOSE:
#       To decrement the count of dirty cache tags and blocks.
#
#  DESCRIPTION:
#       This routine accepts the block count for a deleted dirty cache tag
#       and update the dirty block count and decrements the count of
#       dirty cache tags.
#
#  INPUT:
#       g0 = Block Count for deleted dirty cache tag.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
# void IncDirtyCount(unsigned int count)
#_count g0  param
        .globl  wc$DecDirtyCount
wc$DecDirtyCount:
        lda     C_ca,r5                 # r5 = Pointer to the counters
        ld      ca_tagsDirty(r5),r7     # r7 = Tags Dirty Counter
        ld      ca_blocksDirty(r5),r6   # r6 = Blocks Dirty Counter
        subo    1,r7,r7                 # Decrement the Tags Dirty Counter
        subo    g0,r6,r6                # Adjust the Dirty Block Counter
        st      r7,ca_tagsDirty(r5)     # Save the new Tags Dirty count
        st      r6,ca_blocksDirty(r5)   # Save the new Dirty Blocks count
        ret
#
#******************************************************************************
#
#  NAME: wc$IncResidentCount
#
#  PURPOSE:
#       To increment the count of resident cache tags and blocks.
#
#  DESCRIPTION:
#       This routine accepts the block count for inserting a resident cache tag
#       and update the resident block count and incrementing the count of
#       resident cache tags.
#
#  INPUT:
#       g0 = Block Count for the inserted resident cache tag.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
wc$IncResidentCount:
        lda     C_ca,r5                 # r5 = Pointer to the counters
        ld      ca_tagsResident(r5),r7  # r7 = Tags Resident Counter
        ld      ca_blocksResident(r5),r6 # r6 = Blocks Resident Counter
        addo    1,r7,r7                 # Increment the Tags Resident Counter
        addo    g0,r6,r6                # Adjust the Resident Block Counter
        st      r7,ca_tagsResident(r5)  # Save the new Tags Resident count
        st      r6,ca_blocksResident(r5) # Save the new Resident Blocks count
        ret
#
#******************************************************************************
#
#  NAME: wc$DecResidentCount
#
#  PURPOSE:
#       To decrement the count of resident cache tags and blocks.
#
#  DESCRIPTION:
#       This routine accepts the block count for removing a resident cache tag
#       and update the resident block count and decrementing the count of
#       resident cache tags.
#
#  INPUT:
#       g0 = Block Count for the removed resident cache tag.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
wc$DecResidentCount:
        lda     C_ca,r5                 # r5 = Pointer to the counters
        ld      ca_tagsResident(r5),r7  # r7 = Tags Resident Counter
        ld      ca_blocksResident(r5),r6 # r6 = Blocks Resident Counter
        subo    1,r7,r7                 # Decrement the Tags Resident Counter
        subo    g0,r6,r6                # Adjust the Resident Block Counter
        st      r7,ca_tagsResident(r5)  # Save the new Tags Resident count
        st      r6,ca_blocksResident(r5) # Save the new Resident Blocks count
        ret
#
#******************************************************************************
#
#  NAME: wc$IncFreeCount
#
#  PURPOSE:
#       To increment the count of free cache tags and blocks.
#
#  DESCRIPTION:
#       This routine accepts the block count for inserting a free cache tag
#       and updates the free block count and incrementing the count of
#       free cache tags.
#
#  INPUT:
#       g0 = Block Count for the inserted free cache tag.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
wc$IncFreeCount:
        lda     C_ca,r5                 # r5 = Pointer to the counters
        ld      ca_tagsFree(r5),r7      # r7 = Tags Free Counter
        ld      ca_blocksFree(r5),r6    # r6 = Blocks Free Counter
        addo    1,r7,r7                 # Increment the Tags Free Counter
        addo    g0,r6,r6                # Adjust the Free Block Counter
        st      r7,ca_tagsFree(r5)      # Save the new Tags Free count
        st      r6,ca_blocksFree(r5)    # Save the new Free Blocks count
        ret
#
#******************************************************************************
#
#  NAME: wc$DecFreeCount
#
#  PURPOSE:
#       To decrement the count of free cache tags and blocks.
#
#  DESCRIPTION:
#       This routine accepts the block count for removing a free cache tag
#       and update the free block count and decrementing the count of
#       free cache tags.
#
#  INPUT:
#       g0 = Block Count for the removed free cache tag.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
wc$DecFreeCount:
        lda     C_ca,r5                 # r5 = Pointer to the counters
        ld      ca_tagsFree(r5),r7      # r7 = Tags Free Counter
        ld      ca_blocksFree(r5),r6    # r6 = Blocks Free Counter
        subo    1,r7,r7                 # Decrement the Tags Free Counter
        subo    g0,r6,r6                # Adjust the Free Block Counter
        st      r7,ca_tagsFree(r5)      # Save the new Tags Free count
        st      r6,ca_blocksFree(r5)    # Save the new Free Blocks count
        ret
#
#******************************************************************************
#
#  NAME: wc$IncFlushCount
#
#  PURPOSE:
#       To increment the count of flush in progress cache tags and blocks.
#
#  DESCRIPTION:
#       This routine accepts the block count for the cache tag that is being
#       flushed and updates the flush in progress block count and increments
#       the count of flush in progress cache tags.
#
#  INPUT:
#       g0 = Block Count for the cache tag with flush in progress
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
wc$IncFlushCount:
        lda     C_ca,r5                 # r5 = Pointer to the counters
        ld      ca_tagsFlushIP(r5),r7   # r7 = Tags Flush Counter
        ld      ca_blocksFlushIP(r5),r6 # r6 = Blocks Flush Counter
        addo    1,r7,r7                 # Increment the Tags Flush Counter
        addo    g0,r6,r6                # Adjust the Flush Block Counter
        st      r7,ca_tagsFlushIP(r5)   # Save the new Tags Flush count
        st      r6,ca_blocksFlushIP(r5) # Save the new Flush Blocks count
        ret
#
#******************************************************************************
#
#  NAME: wc$DecFlushCount
#
#  PURPOSE:
#       To decrement the count of flush in progress cache tags and blocks.
#
#  DESCRIPTION:
#       This routine accepts the block count for the cache tag that completed a
#       flush and updates the flush in progress block count and decrements
#       the count of flush in progress cache tags.
#
#  INPUT:
#       g0 = Block Count for the cache tag that completed a flush.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
wc$DecFlushCount:
        lda     C_ca,r5                 # r5 = Pointer to the counters
        ld      ca_tagsFlushIP(r5),r7   # r7 = Tags Flush Counter
        ld      ca_blocksFlushIP(r5),r6 # r6 = Blocks Flush Counter
        subo    1,r7,r7                 # Decrement the Tags Flush Counter
        subo    g0,r6,r6                # Adjust the Flush Block Counter
        st      r7,ca_tagsFlushIP(r5)   # Save the new Tags Flush count
        st      r6,ca_blocksFlushIP(r5) # Save the new Flush Blocks count
        ret
#
#******************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
