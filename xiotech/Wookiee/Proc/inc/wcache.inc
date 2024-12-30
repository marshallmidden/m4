# $Id: wcache.inc 145407 2010-08-10 17:26:37Z m4 $
#******************************************************************************
#
#  NAME: wcache.inc
#
#  DESCRIPTION:
#
#       To provide write caching related definitions.
#
#  Copyright (c) 2000-2005 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- Equates -----------------------------------------------------------------
#
        .set    CACHE_ISSUE_CNT,3       # Allow up to 5 tries during errors
        .set    CACHE_TIME_OUT,8        # Give each Cache op 10 seconds to cmplt
#
        .set    MAX_SGL_ENTRIES,8       # Allow up to 8 SGLs/VRP
        .set    MAX_FLUSH_INV_TIME,375/QUANTUM # Maximum time Flushing and
                                        #  Invalidating tags before a wait
                                        #  is required
        .set    MAX_FLUSH_OPS,512       # Maximum number of ops to be flushed
                                        #  before a wait is required
        .set    FLUSH_OPS_WAIT,125      # Number of milliseconds to wait
                                        #  between max ops being flushed
#        .set    MAX_CDRAM_RB_NODES     # Maximum number of RB Nodes allowed in
                                        #  Cacheable DRAM is the number of
                                        #  cache tags (C_numtags).  This will
                                        #  force any Flush BE tags to use
                                        #  NCDRAM tags.
        .set    MAX_CDRAM_RBI_NODES,MAXCDRAMILTS # Maximum number of RBI Nodes
                                        #  allowed in Cacheable DRAM
        .set    MAX_CDRAM_PLACEHOLDERS,4096 # Maximum number of Placeholder ILTs
                                        #  allowed in Cacheable DRAM
#
# --- Copy memory to Non-Volatile memory locations definitions
#
        .set    MIRROR_FE,1             # Mirror only to the Front End NV
        .set    MIRROR_BE,2             # Mirror only to the Back End NV
        .set    MIRROR_MP,3             # Mirror to local FE NV and MP NV

#
# --- These next 3 constants define the maximum size of a write request
#     that is allowed to use the write cache. Any write request that is less
#     than or equal to WCACHEMAX will always use the write cache, independent
#     of the current queue depth.  Write requests that are less than or equal
#     to WCACHEMAXLOWQ and greater that WCACHEMAX will only use the write cache
#     if the queue depth for the Virtual ID is currently less than WCACHELOWQ.
#     Write requests that are larger than WCACHEMAXLOWQ will always bypass the
#     the write cache.
#
        .set    WCACHEMAX,512           # 256K (512 sectors) max cacheable
        .set    WCACHEMAXLOWQ,2048      # 1M max cacheable at low Q depth
        .set    WCACHELOWQ,4            # Low Queue Depth for max cacheable
#
        .set    MAX_VID_FLUSH_BLOCKS,64     # Max blocks to flush per VID
        .set    SLOW_FLUSH_WAIT_TIME,1000   # 1 Second Wait before flushing
        .set    MEDIUM_FLUSH_WAIT_TIME,750  # 750 msec Wait before flushing
        .set    FAST_FLUSH_WAIT_TIME,500    # 500 msec Wait before flushing
        .set    FASTEST_FLUSH_WAIT_TIME,250 # 250 msec Wait before flushing
        .set    NO_FLUSH_ALLOWED_WAIT_TIME,125 # 125 msec Wait when no flushing
                                            #       is allowed
#
# --- Tag attribute bits
#
        .set    TG_DIRTY,0              # Buffer dirty
        .set    TG_WRITEIP,1            # Buffer fill (write) in progress
        .set    TG_FLUSHIP,2            # Flush in progress
        .set    TG_MIRRIP,3             # Mirror in progress
        .set    TG_READP,4              # Read hit in progress
        .set    TG_BE,12                # Tag is in BE Memory
        .set    TG_FREE_PENDING,13      # Free pending
        .set    TG_RESIDENT,14          # Tag/buffer resident
        .set    TG_FREE,15              # Tag invalid/free
#
# --- Tag state bits
#
        .set    TG_LOCKED_FLUSH,0       # Tag locked for flush
        .set    TG_LOCKED_READ,1        # Tag locked for read hit
        .set    TG_LOCKED_WRITE,2       # Tag locked for write hit
        .set    TG_LOCKED_INVALIDATE,3  # Tag locked for invalidate
        .set    TGM_LOCKED_FLUSH,1      # Mask for locked for flush
        .set    TGM_LOCKED_INVALIDATE,8 # Mask for locked for invalidate
        .set    TGM_LOCKED_NOFLUSH,0xd  # Mask for Flush, Invalidate, Write
#
# --- Cache Tag Element Structure definition ----------------------------------
#
        .set    tg_fthd,0               # Forward thread             <w>
        .set    tg_bthd,tg_fthd+4       # Backward thread            <w>
        .set    tg_vid,tg_bthd+4        # VID                        <s>
        .set    tg_rdcnt,tg_vid+2       # Read In Progress Count     <s>
        .set    tg_attrib,tg_rdcnt+2    # Tag attribute              <s>
        .set    tg_state,tg_attrib+2    # Tag state                  <s>
#                                                                  *******
        .set    tg_vsda,tg_state+2      # LBA starting address       <l>
        .set    tg_vlen,tg_vsda+8       # Length of request          <w>
        .set    tg_bufptr,tg_vlen+4     # Buffer pointer             <w>
#                                                                  *******
        .set    tg_ioptr,tg_bufptr+4    # Cache tree ptr             <w>
        .set    tg_dirtyptr,tg_ioptr+4  # Dirty tree node pointer    <w>
        .set    tg_nextdirty,tg_dirtyptr+4 # Next dirty block        <w>
        .set    tg_hqueue,tg_nextdirty+4 # Head of Queue for tag unlock <w>
#                                                                  *******
        .set    tg_tqueue,tg_hqueue+4   # Tail of queue for tag unlock <w>
#
#     Reserved 12 bytes
#
# --- End structure
#     SMW - Tag size can not exceed 64 bytes without having to restucture the log message
#     in _wc$msgCacheRecoverFail which is size limited
#
        .set    tgsize,tg_tqueue+16     # Sizeof cache tag element
#
#
# --- Cache Recovery F/W Initialization Ctl Block definition ----------------
#
        .set    DATA_FLUSHED,0x76125794 # signature values
        .set    DATA_CACHED,0x49752167
        .set    SIG_REFRESH,0

        .set    wt_signature1,0         # signature 1                <w>
        .set    wt_mirrorAttrib,wt_signature1+4 # (local or remote)  <w>
        .set    wt_vcgid,wt_mirrorAttrib+4 # Virtual Ctrl group ID   <w>
        .set    wt_cserial,wt_vcgid+4   # controller serial number   <w>
#                                                                  *******
        .set    wt_seq,wt_cserial+4     # sequence number            <w>
        .set    wt_mirrorpartner,wt_seq+4 # Mirror Partner           <w>
#
#     Reserved 8 bytes
#                                                                  *******
        .set    wt_battery_sig,wt_mirrorpartner+12 # Battery sign    <l>
#
#     Reserved 4 bytes
#
        .set    wt_signature2,wt_battery_sig+12 # signature 2        <w>
#                                                                  *******
#
# --- End structure
#
        .set    wtsize,wt_signature2+4  # Sizeof structure
#
# --- Battery Backup Data Area -----------------------------------------------
#
        .set    BB_MAX_ECC,             16      # Maximum ECC errors

        .set    bb_battery_sig,         0x00
        .set    bb_wrt_cache_start_adr, 0x10
        .set    bb_wrt_cache_end_adr,   0x14
        .set    bb_sngl_bit_ecc_current,0x20
        .set    bb_mult_bit_ecc_current,0x24
#
        .set    bb_sngl_bit_ecc_total,  0x28    # counts since last full scrub
        .set    bb_mult_bit_ecc_total,  0x2c
#
        .set    bb_sngl_elog_reg_off,   0x30    # offset into 16 word storage-sngl
        .set    bb_mult_elog_reg_off,   0x34    # offset into 16 word storage-mult
#
        .set    bb_sngl_elog_reg_16,    0x40    # up to 16 words
        .set    bb_sngl_ecar_reg_16,    0x80    # up to 16 words
#
        .set    bb_mult_elog_reg_16,    0xc0    # up to 16 words
        .set    bb_mult_ecar_reg_16,    0x100   # up to 16 words
        .set    bbsize,                 0x140   # size
#
# --- End structure
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
