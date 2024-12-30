# $Id: cachefe.as 159870 2012-09-20 12:59:51Z marshall_midden $
#**********************************************************************
#
#  NAME: cachefe.as
#
#  PURPOSE:
#       To provide a means of supplying the Cache functionality in a system
#       that has a Cache installed and also to provide the required logic
#       necessary when a Cache is not installed.
#
#       NOTE: Cache uses two layers of ILT communications to handle precedence
#           problems.
#
#  FUNCTIONS:
#       C$init     - Cache initialization
#       C$que      - Queue I/O request
#       C$quesrp   - Queue SRP request from the BEP
#
#       C$stop     - Stop I/O activity
#       C$setShutdownFlag - Indicate cache will be shutdown
#       C$resume   - Resume I/O activity
#
#       C$disable  - Disable Write Cache globally or by VDisk
#       C$enable   - Enable Write Cache globally or by VDisk
#
#       C$setMirrorPartner - Set the Mirror Partner that Cache will mirror to
#
#       This module employs 6 processes:
#
#       c$exec     - Executive (1 copy)
#       c$ioexec   - I/O Executive (issue I/O after precedence checking)
#       c$drpexec  - Executive to handle DRPs
#       c$error    - Executive to release Operations from the hold queue during
#                    mirroring errors (temporary)
#       c$VCDQueueCheck - Executive to handle VCDs with waiting Ops
#       CA_OpRetryTask - Handles retrying of Ops after a short delay
#
#  Copyright (c) 1996-2008 Xiotech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- local equates ---------------------------------------------------
#
        .set    WI_ISSUE_CNT,3          # Issue the command this many times max
        .set    WI_TIME_OUT,6           # Time out value in Seconds
#
        .set    ERROR_TIME_LIMIT,960    # 2 minutes at 125msec intervals
        .set    ERROR_TIME_WAIT,125     # Error Task timed wait amount in msec
        .set    STOP_TIME_WAIT,125      # C$Stop task timed wait amount in msec
        .set    STOP_HOST_OUTSTANDING,25000/STOP_TIME_WAIT # wait for 25
                                        #  seconds of continuous Host Op
                                        #  outstanding time before reset port
#
#   The following equates tune the preallocated buffer strategy for
#     non-cached read operations from the cache layer.  Non-cached reads of
#     less than <CFREETHR> in size use a preallocated buffer/SRP for the
#     operation if any exist on the freelist. A maximum of <CFREEMAX> buffers
#     will be allocated.
#
        .set    CFREEMAX,512            # Max free buff on freelist
        .set    CFREESIZ,128+8192       # Preallocated buff size
        .set    CFREETHR,64+8192        # Threshold for prealloc
#
        .set    CSRPMAX,512             # Maximum SRPs on freelist
#
# --- global function declarations ------------------------------------
#
        .globl  C$init                  # Cache initialization
        .globl  C$que                   # Queue I/O request
        .globl  C$quedrp                # Queue DRP
#
        .globl  C$stop                  # Stop I/O activity
        .globl  C$setShutdownFlag       # Indicate Cache shutdown
        .globl  C$resume                # Resume I/O activity
#
        .globl  C$disable               # Globally disable cache
        .globl  C$enable                # Caching on a VID basis is now allowed
#
        .globl  C$alwbuf                # Allocate a non-cached write buffer
        .globl  C$rlwbuf                # Release a non-cached write buffer
#
        .globl  C$getwdata              # Get Write Data into a cached buffer
#
        .globl  C$do_nc_op              # Go do a non-cached op
#
        .globl  C$queryCacheData        # Any Possibility of Data in Cache
#
        .globl  C$setMirrorPartner      # Set the new Mirror Partner
        .globl  CA_QueryMirrorPartnerChange  # Query Mirror Partner Change OK?
#
        .globl  CA_RemoveVIDRetryQueue  # Remove any Ops associated with the VID
                                        #  in the Retry Queue
#
        .globl  CA_LogMirrorFailure     # Log a Mirror Failure
#
# --- global data declarations ----------------------------------------
#
        .globl  vcdIndex                # Caching VCD index table
        .globl  C_ca                    # Cache Information
        .globl  C_numtags               # Calculated number of Cache Tags
#
# --- Shared memory data
#
        .section    .shmem
        .align  2
C_sft_flt:
        .space  mlemaxsiz,0             # Software Fault Log Area
#
# --- global usage data definitions -----------------------------------
#
        .data
#
#     Set up the Cache structures
#
        .align  2
C_numtags:
        .word   0                       # Calculated number of Cache Tags
C_TagStartFlushThresh:
        .word   0                       # Calculated Start Flush Tag ceiling
C_TagStopFlushThresh:
        .word   0                       # Calculated Stop Flush Tag floor
C_MaxDirtyTags:
        .word   0                       # Calculated Maximum Dirty Tag Count
C_TagFlushIncr:
        .word   0                       # Calculated Tag Flush Increment
        .globl  C_MaxTotalFlushBlocks
C_MaxTotalFlushBlocks:
        .word   0                       # Calculated Max Total Blocks to flush
        .globl  C_BlockStartFlushThresh
C_BlockStartFlushThresh:
        .word   0                       # Calculated Start Flush Block ceiling
        .globl  C_BlockStopFlushThresh
C_BlockStopFlushThresh:
        .word   0                       # Calculated Stop Flush Block floor
        .globl  C_BlockFlushIncr
C_BlockFlushIncr:
        .word   0                       # Calculated Dirty Block Flush Increment
        .globl  C_MaxDirtyBlocks
C_MaxDirtyBlocks:
        .word   0                       # Calculated Maximum Dirty Block Count
#
C_haltBackgroundFlag:
        .word   0                       # Halt background flag - bits to allow
                                        #  certain tasks to halt background
                                        #  flushes
        .set    hbginvfe,0              #   bit 0 = Invalidate FE process
        .set    hbginvbe,1              #   bit 1 = Invalidate BE process
        .set    hbgstop,2               #   bit 2 = Stop I/O process
                                        #   bits 3 - 31 = reserved
#
C_vcd_wait_active:
        .word   0                       # VCD op waiting queue task active
C_vcd_wait_head:
        .word   0                       # VCD op waiting queue head
C_vcd_wait_tail:
        .word   0                       # VCD op waiting queue tail
C_vcd_wait_pcb:
        .word   0                       # VCD op waiting Exec PCB
#
        .space  4,0                     # entry count field
vcdIndex:
        .space  MAXVIRTUALS*4,0         # Index to VCD structures
#
# --- local usage data definitions ------------------------------------
#
        .align  2
C_drpexec_pcb:
        .word   0                       # Cache DRP Executive
C_drpexec_cqd:
        .word   0                       # Cache DRP current queue depth
C_drpexec_qht:
        .space  8,0                     # Cache DRP Queue head/tail

        .globl   C_error_pcb
C_error_pcb:
        .word   0                       # Error Executive
C_error_cqd:
        .word   0                       # Error State current queue depth
C_error_qht:
        .space  8,0                     # Error State Queue head/tail
C_cwi_pcb:
        .word   0                       # Clear Write Info Send Exec
C_cwi_cqd:
        .word   0                       # Clear Write Info Current Queue Depth
C_cwi_qht:
        .space  8,0                     # Clear Write Info (CWI) Queue
C_cwi_cdrp:
        .word   0                       # Number of CWI DRPs outstanding
        .set    MAX_CWI_DRPS,ltmt_dgmax/4 # Max number of CWI outstanding DRPs

        .globl  CA_OpRetryQue
CA_OpRetryQue:
        .space  qusiz,0                 # Op Retry Task Queue Structure
                                        #  Queue Head, Tail, Count, PCB
        .set    EC_RETRY_COUNT,30       # Retry every second for 30 seconds
        .set    EC_RETRY_TIME,1000/QUANTUM # One Second in II Timer Units
C_swi_cqd:
        .word   0                       # Set Write Info Current Queue Depth
C_swi_qht:
        .space  8,0                     # Set Write Info (SWI) Queue
C_swi_cdrp:
        .word   0                       # Number of SWI DRPs outstanding
        .set    MAX_SWI_DRPS,ltmt_dgmax/4 # Max number of SWI outstanding DRPs
C_swi_kick_pcb:
        .word   0                       # SWI Kick task pcb
C_wi_seqnum:
        .word   0                       # Write Information Datagram Sequence #
C_recoveryFlush_pcb:
        .word   0                       # Cache recovery flush exec
C_Freehead:
        .word   0                       # SRP/buffer freelist head
C_Freecnt:
        .word   0                       # SRP/buffer freelist alloc count
C_SrpCnt:
        .word   0                       # Write SRP prealloc alloc count
C_SFreehead:
        .word   0                       # Write SRP Prealloc freelist head
C_error_loop_cnt:
        .word   0                       # Error Exec Loop Count (can be reset
                                        #  by another task back to zero)
#
# Set up area for saving a remote mirror error so that only one log event of
#   the same kind is sent to the CCB
#
        .globl    C_mirror_error_flag
C_mirror_error_flag:
        .word   0                       # Flag that a previous error has already
                                        #   been sent (could be a byte except
                                        #   for alignment purposes)
C_mirror_error_ilt:
        .word   0                       # ILT status word
C_mirror_error_drp:
        .byte   0                       # DRP status byte
C_mirror_error_dg:
        .byte   0                       # Datagram Status byte
C_mirror_error_ec1:
        .byte   0                       # Datagram Response Error Code #1
C_mirror_error_ec2:
        .byte   0                       # Datagram Response Error Code #2
#
        .align  4
C_user_stop_cnt:
        .space  256,0                   # 256 User Stop Counts (1 byte user ID)
C_wi_nvac:
        .space  nvacsize,0              # Write Information Control Structure
#
#   C$Stop Recovery Logging message layout
#
        .set    log_c_stop_time,mle_len+4   # Time of this recovery (From start
                                        # of C$Stop) in seconds         <w>
        .set    log_c_stop_function,log_c_stop_time+4 # Function that is called <w>
        .set    log_c_stop_corc,log_c_stop_function+4 # Copy of C_orc           <w>
        .set    log_c_stop_cowsrpc,log_c_stop_corc+4  # Copy of C_owsrpc        <w>
        .set    log_c_stop_corsrpc,log_c_stop_cowsrpc+4 # Copy of C_orsrpc      <w>
        .set    log_c_stop_dvlorc,log_c_stop_corsrpc+4 # Copy of D_vlorc        <w>
        .set    log_c_stop_linkorc,log_c_stop_dvlorc+4 # Copy of Outstanding VRPs <w>
        .set    log_c_stop_log_size,log_c_stop_linkorc+4 # Size of c_stop logging area
#
# --- executable code -------------------------------------------------
#
#**********************************************************************
#
#  NAME: C$init
#
#  PURPOSE:
#       To provide a means of initializing this module.
#
#  DESCRIPTION:
#       The executive process for this module is established and made
#       ready for execution.
#
#  CALLING SEQUENCE:
#       call    C$init
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
#
        .text
C$init:
#
# --- Initialize Variables
#
        mov     0,r12                   # Used to zero out fields
        st      r12,C_exec_cqd          # Zero out the Cache Queue Depth Number
        st      r12,C_exec_qht+qu_head  # Zero out the Cache Queue Head
        st      r12,C_exec_qht+qu_tail  # Zero out the Cache Queue Tail
        st      r12,C_ioexec_cqd        # Zero out the Cache I/O queue depth
        st      r12,C_ioexec_qht+qu_head # Zero out the Cache I/O Queue Head
        st      r12,C_ioexec_qht+qu_tail # Zero out the Cache I/O Queue Tail
        st      r12,C_drpexec_cqd       # Zero out the Cache DRP queue depth
        st      r12,C_drpexec_qht+qu_head # Zero out the Cache DRP Queue Head
        st      r12,C_drpexec_qht+qu_tail # Zero out the Cache DRP Queue Tail
        st      r12,c_rbifreec          # Zero out the RBI node free lists
        st      r12,c_rbfreec           # Zero out the RB node free lists
# C_numtags = (WctSize/tgsize)  ( Number of Cache Tags in Tag area )
        ld      WctSize,r5              # Calculate the number of Cache Tags by
        ldconst tgsize,r6               #   Getting the Cache Tag Area Size and
        divo    r6,r5,r7                #   dividing by the tag size
        st      r7,C_numtags            # Save the number of Cache Tags
# C_MaxDirtyTags = (C_numtags - 16)
        lda     -16(r7),r6              # Calculate the Maximum Dirty Tags
        st      r6,C_MaxDirtyTags       # Save the Maximum Dirty Tags allowed
# C_TagStartFlushThresh = (C_numtags*6/8) ( Dirty Tag Start Flush Thresh )
        shro    3,r7,r5                 # Calculate the Number of Dirty Tags to
        mulo    6,r5,r5                 #   start the flushing process
        st      r5,C_TagStartFlushThresh # Save the Start Flush Threshold
# C_TagStopFlushThresh = (C_numtags*6/8) ( Dirty Tag Stop Flush Threshold )
        mov     r5,r6                   # Calculate the Number of Dirty Tags
                                        #   to stop the flushing process
        st      r6,C_TagStopFlushThresh # Save the Stop Flush Threshold
# C_TagFlushIncr = ((C_TagStartFlushThresh-C_TagStopFlushThresh)/4)
        subo    r6,r5,r8                # Calculate the Cache Tag Flush
        shro    2,r8,r8                 #   Increment value
        st      r8,C_TagFlushIncr       # Save the Tag Flush Increment value
# C_MaxTotalFlushBlocks = (WcbSize/SECSIZE) ( Max blocks to flush )
        ld      WcbSize,r5              # Calculate the Maximum number of
        ldconst SECSIZE,r6              #   Blocks to flush in an op
        divo    r6,r5,r7
        st      r7,C_MaxTotalFlushBlocks # Save Max number of Blocks to flush
# C_MaxDirtyBlocks = ((WCBSIZE/SECSIZE)*31/32)
        shro    5,r7,r8                 # Calculate the Maximum Number of
        mulo    31,r8,r8                #   Dirty blocks allowed
        st      r8,C_MaxDirtyBlocks     # Save the Max Number of Dirty Blocks
# C_BlockStartFlushThresh = ((WcbSize/SECSIZE)*6/8) ( Dirty Blk Start Thresh )
        shro    3,r7,r8                 # Calculate the Number of Dirty Blocks
        mulo    6,r8,r8                 #   to start the background flush
        st      r8,C_BlockStartFlushThresh # Save Dirty Blk Flush Start Thresh
# C_BlockStopFlushThresh = ((WcbSize/SECSIZE)*6/8) ( Dirty Blk Stop Thresh )
        mov     r8,r9                   # Calculate the Number of Dirty Blocks
                                        #   to stop the flushing process
        st      r9,C_BlockStopFlushThresh # Save Dirty Block Flush Stop Thresh
# C_BlockFlushIncr = ((C_BlockStartFlushThresh-C_BlockStopFlushThresh)/4)
        subo    r9,r8,r10               # Calculate the Dirty Block Flushing
        shro    2,r10,r10               #   Increment value
        st      r10,C_BlockFlushIncr    # Save the Dirty Block Flush Increment
#
# --- Allocate and initialize the Cache Information Structure
#
        lda     C_ca,g0                 # Get the CA Info Pointer
        setbit  MAXBAT,0,r5             # The lower MAXBAT bits of the
        subo    1,r5,r5                 #  battery state are set to show they
                                        #  bad.  The CCB will set them good
                                        #  when they are.
        stob    r5,ca_status2(g0)       # Save the Battery State
        ld      WcbSize,r5              # r5 = Write Cache Buffer Size
        ldconst WCACHEMAX,r6            # r6 = Maximum blocks for cached write
        st      r5,ca_size(g0)          # Set the size of the Cache
        ldconst MAX_SGL_ENTRIES,r7      # r7 = Maximum SGLs to Flush
        st      r6,ca_maxcwr(g0)        # Maximum allowed cached write - blocks
        ld      C_numtags,r5            # r5 = Total number of Cache Tags
        st      r7,ca_maxsgl(g0)        # Maximum SGLs sent to physical device
        ld      WcbSize,r6              # Calculate the total number of
        ldconst SECSIZE,r8              #   Cache Blocks in the Cache Buffer
        divo    r8,r6,r6                #   by Size/Sector size
        st      r5,ca_numTags(g0)       # Total number of Cache Tags
        st      r6,ca_numBlks(g0)       # Total number of Cache Buffer Blocks
#
# --- Initialize the Cache Write Information NVAC structure (mirror of what the
#       P4 records look on the FE of the Mirror Partner)
#
c       r3 = NUM_OF_P4_NVA_WK;
        lda     C_wi_nvac,r15           # Get Cache WI NVAC control address
        st      r3,nc_cur(r15)          #  to current
        st      r3,nc_min(r15)          #  to minimum
#
#       Allocate SNVA assignment bitmap & setup NVA control structure
#
        ldconst p_localFENVRAM_BASE,r4
        lda     nv_csum(r4),r6          # Get address of checksum
        st      r6,nc_csum(r15)         # Set up checksum ptr
#
        ldconst nvabasesiz,r3           # Get header size
        addo    r3,r4,r4                # Increment past header
        st      r4,nc_nvarec(r15)       # Save ptr to first NVA record
#
# (num rec / 32 bits) * 4 bytes per word -- assign part 3 bitmap.
c       g0 = s_MallocC(NUM_OF_P4_NVA_WK/8, __FILE__, __LINE__);
        st      g0,nc_mapbase(r15)      # Set up bitmap base
        st      g0,nc_mapptr(r15)       # Set up bitmap ptr
#
# --- Establish VCD Queue Check  Executive process ----------------------------
#
        lda     c$VCDQueueCheck,g0      # Start VCD Queue Check Executive
        ldconst CVCDQUEUECHECKPRI,g1
c       CT_fork_tmp = (ulong)"c$VCDQueueCheck";
        call    K$fork
        st      g0,C_vcd_wait_pcb       # Save PCB
#
# --- Establish Mirror Executive process --------------------------------------
#
        ldconst CMIRRORPRI,g1
        lda     WC_MirrorExec,g0        # Start Mirror Executive
c       CT_fork_tmp = (ulong)"WC_MirrorExec";
        call    K$fork
#
# --- Establish Mirror BE Tags Executive process ------------------------------
#
        ldconst CMIRRORPRI,g1
        lda     WC_MirrorBETagExec,g0   # Start Mirror BE Tags Executive
c       CT_fork_tmp = (ulong)"WC_MirrorBETagExec";
        call    K$fork
#
# --- Establish DRP Executive process -----------------------------------------
#
        lda     c$drpexec,g0            # Start DRP Executive
        ldconst CDRPEXECPRI,g1
c       CT_fork_tmp = (ulong)"c$drpexec";
        call    K$fork
        st      g0,C_drpexec_pcb        # Save PCB
#
# --- Establish Clear Write Information Executive process ---------------------
#
        lda     c$clearWriteInfoTask,g0 # Start Clear Write Info Executive
        ldconst CDRPEXECPRI,g1
c       CT_fork_tmp = (ulong)"c$clearWriteInfoTask";
        call    K$fork
        st      g0,C_cwi_pcb            # Save PCB
#
# --- Establish Recovery Flush Task process -----------------------------------
#
        lda     WC$recoveryFlushTask,g0 # Start Recovery Flush Executive
        ldconst CFLUSHPRI,g1
c       CT_fork_tmp = (ulong)"WC$recoveryFlushTask";
        call    K$tfork
        st      g0,C_recoveryFlush_pcb  # Save PCB
#
# --- Establish Op Retry Task process -----------------------------------------
#
        lda     CA_OpRetryTask,g0       # Op Retry Executive
        ldconst COPRETRYPRI,g1
c       CT_fork_tmp = (ulong)"CA_OpRetryTask";
        call    K$fork
        st      g0,CA_OpRetryQue+qu_pcb
#
# --- Establish WRP Executive process -----------------------------------------
#
        lda     WC_WRPExec,g0           # WRP Executive
        ldconst CDRPEXECPRI,g1
c       CT_fork_tmp = (ulong)"WC_WRPExec";
        call    K$fork
#
# --- Establish Temporary VDisk Disable process -------------------------------
#
        lda     WC_TDisExec,g0          # VDisk Temporary Disable Executive
        ldconst CFLUSHPRI,g1
c       CT_fork_tmp = (ulong)"WC_TDisExec";
        call    K$fork
#
# --- Establish Write Cache Mark Signature process ----------------------------
#
        lda     wc_markWCache,g0        # Write Cache Mark Signature Executive
        ldconst CDRPEXECPRI,g1
c       CT_fork_tmp = (ulong)"wc_markWCache";
        call    K$fork
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: C$boot
#
#  PURPOSE:
#       To provide a means of initializing this module.
#
#  DESCRIPTION:
#       Allocate and initialize the non-cached Write Proxy Buffer
#
#  CALLING SEQUENCE:
#       call    C$boot
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
#       g2
#
#**********************************************************************
#
C$boot:
#
# --- Wait for the backend, because we used to wait for the BE write
#   buffer space here.
#
.cb10:
        ld      C_fego,g0               # Wait for BE to set this flag
        cmpobne 0,g0,.cb20              # Continue when non-zero
        ldconst 1,g0                    # Delay for minimum time
        call    K$twait
        b       .cb10
#
.cb20:
        ret
#
#**********************************************************************
#
#  NAME: C$stop
#
#  PURPOSE:
#       To provide a common means of stopping all I/O activity invoked
#       through the Cache layer and flushing any cached data.
#
#  DESCRIPTION:
#       The stop counter is incremented, cache flushed, and a check is made for
#       any outstanding I/O.  When all outstanding I/O completes, this
#       routine returns to the caller.  While the stop counter is
#       non-zero, the executive is effectively blocked.
#
#       If the input parameter says to wait (TRUE), the Flushing of cache will
#       be waited on until all data has been flushed and then return to the
#       caller will occur.  If the input parameter says to not wait (FALSE),
#       the Flushing of cache will be started and then return to the caller will
#       occur (before all the data has been flushed from the cache).
#
#       Multiple Stop Counters are set up so that one user may resume the stops
#       that user issued without interfering with other users stops.
#
#       This routine may only be called from the process level.
#
#  CALLING SEQUENCE:
#       call    C$stop
#
#  INPUT:
#       g0 - Options flag byte
#           Bit 0 = Wait (ignored if Bit 3 = 1 or Bit 1 = 1)
#                   0 = Do not wait for the Flush to complete
#                   1 = Wait for the Flush to complete (unless an error occurs
#                       and the flush could not complete)
#           Bit 1 = Flush (ignored if Bit 3 = 1)
#                   0 = Flush write cache
#                   1 = Do not flush the write cache
#           Bit 2 = Background Flush Allowed (ignored if Bit 3 = 1 or Bit 1 = 0)
#                   0 = Allow background flushes to occur as normal
#                   1 = Halt background flushes - this will cause the Cache
#                       to wait for outstanding write cache ops to the disk to
#                       complete before returning
#           Bit 3 = Wait for outstanding Host I/O to complete
#                   0 = Wait for all outstanding Host I/O to complete before
#                       returning
#                   1 = Do not wait for the outstanding Host I/O to complete
#                       before returning
#       g1 - User ID byte
#           0 = Generic User (Stop/Resumes must be paired)
#           1-3F = PROC Users
#           40-7F = CCB Users
#           80-BF = XSSA Users
#           C0-FF = Reserved
#
#  OUTPUT:
#       g0 = Completion Code
#               ecok    = Completed successfully
#               ecioerr = Stop did not complete successfully
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
C$stop:
        PushRegs                        # Save all G registers (stack relative)
        mov     g0,r15                  # Save the input parameters
        mov     g1,r14
#
# --- Bump user stop counter and possibly the overall stop counter.  The
#       overall stop counter is incremented when the user count goes from zero
#       to one.  Therefore, the overall stop counter shows how many users have
#       stops outstanding.
#
        ldob    C_user_stop_cnt[g1*1],r4 # Get the user stop counter
        ld      C_exec_pcb,r6           # See if the Cache is running yet
        lda     C_ca,r5                 # Get the CA Structure address
        cmpinco 0,r4,r4                 # Bump the user stop counter
        stob    r4,C_user_stop_cnt[g1*1]
        bne     .cstop10                # Jif the user counter has already been
                                        #  counted in the overall stop counter
        ldob    ca_stopcnt(r5),r3       # Bump the overall stop counter
        addo    1,r3,r3
        stob    r3,ca_stopcnt(r5)
.cstop10:
        cmpobe  0,r6,.cstop95           # Jif Cache is not running yet
#
# --- Determine if the requestor wants to wait for the outstanding Host I/O
#
        bbs     mxcnowaithost,r15,.cstop95 # Jif not waiting for Host I/O
#
# --- Stall until pending host I/Os complete or they cannot be completed
#       because cache went into an error state.  Some recovery actions if the
#       ops are not completing are to, let VLink Ops through occasionally, and
#       LIP the Bus if Server I/O is stuck.
#
        mov     0,r3                    # Keep track of time to determine what
                                        #  recovery step may be needed.
        mov     0,r11                   # r11 = timer of host ops outstanding
        ldconst 0,r12                   # r12 = Host Outstanding Write SRP cnt
        mov     0,r13                   # r13 = Host Outstanding Read SRP cnt
.cstop20:
        ldob    ca_stopcnt(r5),r4       # Ensure still in a stop state requested
        ld      C_cwi_cdrp,r6           # Get Current CWI DRP Count
        ldob    ca_status(r5),r7        # See if Cache can complete Host ops
        cmpobe  0,r4,.cstop95           # Jif a stop is no longer requested
c       g0 = CA_CheckOpsOutstanding();  # Check for Ops outstanding in BE or linklayer.
        cmpobne 0,g0,.cstop23           # Jif still host ops to wait on
        ld      C_cwi_cqd,r4            # Get Current Clear Write Info Queue cnt
        cmpobne 0,r6,.cstop23           # Jif still outstanding CWI DRPs
        cmpobe  0,r4,.cstop30           # Jif all clear - no ops left
#
#   Clear Write Info ops on queue but task not ready to send yet.  Ready the
#       task to allow those to complete
#
        ld      C_cwi_pcb,r6            # r6 = Clear Write Info Task pcb
        ldob    pc_stat(r6),r4          # r4 = Current process status
        cmpobne pcnrdy,r4,.cstop23      # Jif status is not Not Ready (already
                                        #   active)
        ldconst pcrdy,r4                # r4 = Process ready status
.ifdef HISTORY_KEEP
c CT_history_pcb("C$stop setting ready pcb", r6);
.endif  # HISTORY_KEEP
        stob    r4,pc_stat(r6)          # Ready process
#
.cstop23:
        bbs     ca_error,r7,.cstop90    # Jif the host ops will not finish
#
#   Determine if host ops have been outstanding too long and
#       handle appropriately
#
        ld      C_owsrpc,g4             # Get Server Outstanding Write SRPs
        ld      C_orsrpc,g5             # Get Server Outstanding Read SRPs
        addo    g4,g5,g6                # Determine if any ops are outstanding
## controller coming to this every time (at -ve value.i.e. g0=abnormal value) g5=g6=r12=r13=0
#c       fprintf(stderr,"<GR>Cstop-g0(ret value of CA_CheckOpsOutstanding)=%lx g4=%lx g5=%lx g6=%lx r12=%lx r13=%lx\n",g0,g4,g5,g6,r12,r13);
        cmpobe  0,g6,.cstop26           # Jif there are no outstanding Ops
        cmpobne r12,g4,.cstop24         # Jif Write SRPs does not match
        cmpobe  r13,g5,.cstop25         # Jif Read SRPs match and check timer
.cstop24:
        mov     g4,r12                  # Save the new Write SRP count
        mov     g5,r13                  # Save the new Read SRP count
        ldconst 0,r11                   # Clear the Timer count
#c       fprintf(stderr,"<GR>CStop-recovery steps\n");
        b       .cstop26                # Continue looking at the table for
                                        #  recovery steps
#
.cstop25:
        ldconst STOP_HOST_OUTSTANDING,g6 # Determine if time to reset FE
        cmpinco g6,r11,r11              # Ops outstanding, compare and bump cnt
        bg      .cstop26                # Jif more time still allowed to wait
#c       fprintf(stderr,"<GR>Cstop-g6=%lx r11=%lx\n",g6,r11);
#
        ld      L_stattbl,g3            # Get stat table pointer
        shro    3,r3,g1                 # Convert current count to seconds
        ld      D_vlorc,g2              # Get VLink outstanding requests
        ldos    ls_vrpocount(g3),g3     # Get VRP outstanding counter
        lda     c$stopLipServerOps,g6   # Call function to reset FE loops
        mov     g0,r6                   # Save the ops outstanding count
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mlecstoplog,r11         # get log code
        st      r11,mle_event(g0)
        st      g1,log_c_stop_time(g0)  # Save the Time of the recovery
        st      g6,log_c_stop_function(g0) # Save the function used during rcvry
        st      r6,log_c_stop_corc(g0)  # Save the Outstanding host Ops
        st      g4,log_c_stop_cowsrpc(g0) # Save the copy of C_owsrpc
        st      g5,log_c_stop_corsrpc(g0) # Save the copy of C_orsrpc
        st      g2,log_c_stop_dvlorc(g0) # Save the copy of D_vlorc
        st      g3,log_c_stop_linkorc(g0) # Save the copy of Link960 ORC
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], log_c_stop_log_size);
#
#c       fprintf(stderr,"<GR>Cstop-g3=%lx r6=%lx r3=%lx.. calling c$stopLipServerOps\n",g3,r6,r3);
        call    c$stopLipServerOps      # Reset the FE Ports with Ops outstanding
        ldconst 0,r11                   # Reset the FE host Ops time counter
        mov     r6,g0                   # Restore ops outstanding count
#
#   Determine if time to take a Recovery step and if so log a message
#
.cstop26:
        cmpobe  0,r3,.cstop27           # Jif first time through loop (no message).
c       r9 = r3 & 0x1f;                 # If 4*8 clock ticks, we have zero.
        cmpobne 0,r9,.cstop27           # Jif not time to do a recovery step
        ld      L_stattbl,g4            # Get stat table pointer
        ld      C_owsrpc,g1             # Get Server Outstanding Write SRPs
        ld      C_orsrpc,g2             # Get Server Outstanding Read SRPs
        ld      D_vlorc,g3              # Get VLink outstanding requests
        ldos    ls_vrpocount(g4),g4     # Get VRP outstanding counter
        shro    3,r3,r9                 # Convert back to seconds
        mov     g0,g5                   # save outstanding host ops
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        st      r9,log_c_stop_time(g0)  # Save the Time of the recovery
        lda     c$stopGoVLinkOps,r10    # r10 = Function to do rcvry step
        st      r10,log_c_stop_function(g0) # Save the function used during rcvry
        st      g5,log_c_stop_corc(g0)  # Save the Outstanding host Ops
        st      g1,log_c_stop_cowsrpc(g0) # Save the copy of C_owsrpc
        st      g2,log_c_stop_corsrpc(g0) # Save the copy of C_orsrpc
        st      g3,log_c_stop_dvlorc(g0) # Save the copy of D_vlorc
        st      g4,log_c_stop_linkorc(g0) # Save the copy of Link960 ORC
        ldconst mlecstoplog,r9          # get log code
        st      r9,mle_event(g0)
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], log_c_stop_log_size);
#
#   Do the recovery step and then set up for the next recovery step
#
#c       fprintf(stderr,"<GR>Cstop-vrp outstanding couter=%lx outstanding hostops=%lx r3=%lx.. calling c$stopGoVLinkOps\n",g4,g5,r3);
        call    c$stopGoVLinkOps        # Go do the recovery step
.cstop27:
        ldconst STOP_TIME_WAIT,g0       # Delay for time specified above
        call    K$twait                 # Wait for ops to complete
        addo    1,r3,r3                 # Increment the timeout counter
#c       fprintf(stderr,"<GR>CStop-Retrying for recovery..r3=%lx.\n",r3);
        b       .cstop20                # Keep checking for completion
#
# --- Now that all ops have been handled and data possibly put into cache,
#       handle as requested.
#
#       Determine if a Flush is required or not
#
.cstop30:
        bbs     mxcnoflush,r15,.cstop50   # Jif a flush is not requested
#
#       Flush required, go through all the VIDs and flush and invalidate
#       any cached data.
#
        chkbit  mxcwait,r15             # Determine if wait is requested or not
        sele    FALSE,TRUE,g0           # Set input appropriately
        call    wc$FlInvAll             # Flush all the VIDs and return as
                                        #   requested
        cmpobe  TRUE,g0,.cstop95        # Jif all was flushed OK. Return Good
        bbs     mxcwait,r15,.cstop80    # If wait and not flushed, return an
        b       .cstop95                #   error.  If no wait, then the
                                        #   flush could still be in progress and
                                        #   return good
#
#       No Flush is requested.  Determine if background flushes are allowed
#       or not.  If so, all done.  If not, wait for outstanding write cache
#       ops to complete and allow no more.
#
.cstop50:
        bbc     mxcnobackground,r15,.cstop95 # Jif background flushing is allowed
#c       fprintf(stderr,"<GR>cstop-allowing back ground flush..\n");
        ldob    ca_status(r5),r6        # Get the Cache Status byte
        ld      C_haltBackgroundFlag,r7 # Get the Background Halt Flag
        ldconst 1,g0                    # Delay for minimum time in K$twait
        setbit  ca_halt_background,r6,r6 # Turn off Background flushing
        setbit  hbgstop,r7,r7           # Show Stop I/O is halting BG flush
        stob    r6,ca_status(r5)        # Store the new status byte
        st      r7,C_haltBackgroundFlag # Save new Halt Background Flag
.cstop60:
        ld      C_flush_orc,r6          # Get the outstanding Flush Ops count
        ldob    ca_status(r5),r7        # See if Cache can complete Host ops
        cmpobe  0,r6,.cstop95           # Jif no more flush ops outstanding
        bbs     ca_error,r7,.cstop90    # Jif the Flush ops will not finish
        call    K$twait                 # More ops still, wait for them to
        b       .cstop60                #  complete
#
#       See if we should ignore error
#       Look in C_user_stop_cnt for Proc users.
#
.cstop80:
        ldconst 32,r7                   # BE user 0x20(32) - 0x3F(63)
.cstop82:
        ldob    C_user_stop_cnt[r7*1],r8 # Get the user stop counter
        cmpobne 0,r8,.cstop92           # BE cstop pending
        addo    1,r7,r7                 # Bump BE User
        cmpobe  64,r7,.cstop90          # No BE stops pending, return error
        b       .cstop82
#
# --- Did not successfully complete.  Return error status
#
.cstop90:
        ldconst ecioerr,g0              # Show the stop failed to complete OK
        b       .cstop100               # Return
#
.cstop92:
.ifndef PERF
c fprintf(stderr, "%s%s:%u cstop92: BE stop active, ignoring flush error\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # PERF
#
# --- Completed all OK.  Return Good status
#
.cstop95:
        ldconst ecok,g0                 # Show the stop completed good
#
# --- Exit
#
.cstop100:
        PopRegs                         # Restore g1 thru g14
        ret
#
#**********************************************************************
#
#  NAME: C$setShutdownFlag
#
#  PURPOSE:     To set the shutdown flag so the write cache is
#               marked as flushed after the write cache is flushed.
#
#  DESCRIPTION:
#       The flushed indicator is used on power up to indicate the write
#       cache does not contain data that needs to be flushed to disk.
#
#       This routine may only be called from the process level.
#
#  CALLING SEQUENCE:
#       call    C$setShutdownFlag
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
C$setShutdownFlag:
        lda     C_ca,r5                 # Get the Cache Information pointer
        ldob    ca_status(r5),r3        # Get the Cache Status
        setbit  ca_shutdown,r3,r3       # Clear the cache shutdown bit
        stob    r3,ca_status(r5)        # Set the Cache Status
        ret
#
#**********************************************************************
#
#  NAME: c$stopGoVLinkOps
#
#  PURPOSE:     Walk the C$Stop Queue of ops and let any VLink Ops continue
#               to the Backend.
#
#  DESCRIPTION:
#       The C$Stop Queue will be walked and any VLink Ops that are outstanding
#       will be let continue.  This is an attempt at not deadlocking the Stop
#       because both controller have outstanding VLink Ops.  This is
#       accomplished by setting a flag for c$exec to execute VLink ops and then
#       "Ready" the c$exec task.
#
#       This routine may only be called from the process level.
#
#  CALLING SEQUENCE:
#       call    c$stopGoVLinkOps
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
c$stopGoVLinkOps:
        ld      C_exec_pcb,r4           # Get the c$exec pcb
        ldconst pcrdy,r6                # Ready the c$exec task
        ldob    pc_stat(r4),r5          # Get the status of the c$exec pcb
        ldconst TRUE,r3                 # Show that the VLink Ops need to be
        st      r3,c_allowVLinkOps      #  allowed even though stopped (c$exec)
        cmpobe  pcsem2wait,r5,.csgovlinkop50 # Jif in a cache stop wait
        cmpobne pcnrdy,r5,.csgovlinkop100 # Jif not in a ready state (must be
                                        #    waiting for something else)
.csgovlinkop50:
.ifdef HISTORY_KEEP
c CT_history_pcb("c$stopGoVLinkOps setting ready pcb", r4);
.endif  # HISTORY_KEEP
        stob    r6,pc_stat(r4)          # Enable the c$exec
#
# --- Exit
#
.csgovlinkop100:
        ret
#
#**********************************************************************
#
#  NAME: c$stopLipServerOps
#
#  PURPOSE:     LIP any ports that have outstanding requests to the servers
#
#  DESCRIPTION:
#       Walk the VCDs to determine if there are any outstanding SRPs waiting
#       on the servers.  If there are, then LIP the FE port that the ops
#       are waiting on.  This is an attempt at not deadlocking the Stop
#       because the servers are not talking to us.
#
#       This routine may only be called from the process level.
#
#       Due to many problems with a LIP, the QLogic Cards will now be reset
#       to clear any I/O.
#
#  CALLING SEQUENCE:
#       call    c$stopLipServerOps
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
c$stopLipServerOps:
        ld      C_owsrpc,r3             # Get the outstanding Write SRPs
        ld      C_orsrpc,r4             # Get the outstanding Read SRPs
        PushRegs                        # Save all G registers (stack relative)
#
#   Determine if there are any outstanding SRPs in the first place
#
        cmpobne 0,r3,.cslso05           # Jif there are outstanding Write SRPs
        cmpobe  0,r4,.cslso90           # Jif there are no outstanding Read SRPs
#
#   Find out which ports they are on (unfortunately, there is no
#       list that differentiates between an Op to the BE or the FE so walk all
#       the VCDs with outstanding VRPs), and reset them.
#
.cslso05:
        ld      ispmap,r8               # r8 = Bit mask of all interfaces
        mov     0,r5                    # r5 = VID being tested
        ldconst MAXVIRTUALS-1,r6        # r6 = Maximum VID allowed
        mov     0,r7                    # r7 = port bitmap to reset
        mov     0,g2                    # g2/g3 = Minimum LBA to look for
        mov     0,g3
        ldconst 0xFFFFFFFF,g4           # g4/g5 = Maximum LBA to look for
        lda     0xFFFFFFFF,g5
.cslso20:
        ld      vcdIndex[r5*4],r9       # r9 = VCD Index
        cmpobe  0,r9,.cslso50           # Jif there is no valid VCD entry
        ld      vc_io(r9),g0            # g0 = VCD I/O Tree Pointer
        cmpobe  0,g0,.cslso50           # Jif no active Ops outstanding
        call    RBI$foverlap            # Get the first node to look at
.cslso30:
        ld      rbdpoint(g1),r10        # get ILT pointer
        ld      il_misc(r10),r11        # r11 = ILT Parms pointer
        ldob    idf_ci(r11),r12         # r12 = Path the VRP came from
        cmpobg  MAXISP,r12,.cslso40     # Jif path is valid
        mov     r8,r7                   # Path is invalid - reset all interfaces
        b       .cslso60                # Go do the resets
#
.cslso40:
        setbit  r12,r7,r7               # Turn on this paths bit in bit map
        cmpobe  r7,r8,.cslso60          # Jif all ports have been found
        call    RBI$noverlap            # Get the next node in the tree
        cmpobne FALSE,g1,.cslso30       # Jif there is another entry
.cslso50:
        cmpobe  r7,r8,.cslso60          # Jif all ports have been found
        cmpinco r6,r5,r5                # Increment the Counter and chk for done
        bne     .cslso20                # Jif not checked all VCDs yet
        cmpobe  0,r7,.cslso90           # No ports found, report none reset
#
#   Do all the resets
#
.cslso60:
        ldconst 0,r13                   # Set up a port number evaluator
.cslso65:
        bbc     0,r7,.cslso70           # Jif this bit is clear
        mov     r13,g0                  # g0 = Port to LIP Reset
        ldconst ecri,g1                 # g1 = Reason - Init and Go (Recovery)
        call    ISP_reset_chip          # Initiate the Reset (ignore ret code)
.cslso70:
        shro    1,r7,r7                 # Remove the last port just processed
        addo    1,r13,r13               # Point to the next port number
        cmpobne 0,r7,.cslso65           # Jif more work ports to be reset
        b       .cslso100               # All done - exit
#
#   Show that no outstanding SRPs existed (the absence of this message shows
#       that there were SRPs outstanding to the servers).  This is the same
#       as the C$Stop recovery message but the time = 0 and the function shows
#       who logged the message.
#
.cslso90:
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst 0,r5                    # Zeroing register
        st      r5,log_c_stop_time(g0)  # Zero to show special log
        lda     c$stopLipServerOps,r4   # Show this log coming from this exec
        st      r4,log_c_stop_function(g0) # Save the function logging the message
        st      r5,log_c_stop_corc(g0)  # Zero - not used
        st      r5,log_c_stop_cowsrpc(g0) # Zero - not used
        st      r5,log_c_stop_corsrpc(g0) # Zero - not used
        st      r5,log_c_stop_dvlorc(g0) # Zero - not used
        st      r5,log_c_stop_linkorc(g0) # Zero - not used
        ldconst mlecstoplog,r9          # get log code
        st      r9,mle_event(g0)
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], log_c_stop_log_size);
#
.cslso100:
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
#**********************************************************************
#
#  NAME: C$resume
#
#  PURPOSE:
#       To provide a common means of resuming all I/O activity invoked
#       through the Cache layer.
#
#  DESCRIPTION:
#       Multiple User Stop counters exist and the overall stop counter is the
#       number of User Stop counters that are non-zero.  The user stop count
#       is decremented as requested and if it goes to zero, the overall stop
#       counter is decremented.  When the overall counter has returned to zero,
#       the executive is unblocked and certain write cache flags handled.
#
#  CALLING SEQUENCE:
#       call    C$resume
#
#  INPUT:
#       g0 = Options byte
#           0 = Clear the last stop in this User ID Stop Counter
#           1 = Clear all the stops in this User ID Stop Counter (only valid
#               if the User ID > 0)
#           2-FF = Reserved (Treated as if the value was a zero)
#       g1 - User ID byte
#           0 = Generic User (Stop/Resumes must be paired)
#           1-3F = PROC Users
#           40-7F = CCB Users
#           80-BF = XSSA Users
#           C0-FF = Reserved
#
#  OUTPUT:
#       g0 = Completion Code
#               ecok    = Completed successfully
#               destopzero = Stop count already zero
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
C$resume:
        mov     g1,r15                  # Save g1
        ldconst deok,r14                # Preset Return Completion code to good
#
# --- Adjust user stop counter.  If the user stop counter goes to zero, then
#       also decrement the overall stop counter.
#
        lda     C_ca,r4                 # Get the CA Structure Address
        ldob    C_user_stop_cnt[g1*1],r5 # r5 = User stop counter
        ldob    ca_stopcnt(r4),r3       # r3 = Overall stop counter
        cmpobne 0,r5,.re10              # Jif the users count is not zero
        cmpobe  mriclearall,g0,.re100   # Jif this is a Clear All situation
                                        #  to show it completed successfully
                                        #  even though the count is already
                                        #  zero (used in Error Recovery and
                                        #  may be issued even if no stop is
                                        #  outstanding).
        ldconst destopzero,r14          # Decrement count - Set RC to Stop
                                        #  already zero (likely code bug).
        b       .re100                  # Exit
#
.re10:
        cmpobe  mxigenuser,g1,.re20     # Jif this is the generic user counter
        cmpobe  mriclearone,g0,.re20    # Jif the user request dec by one
        cmpobne mriclearall,g0,.re20    # Jif invalid option - decrement by one
        mov     1,r5                    # Set user stop counter to decrement to
                                        #  zero to simulate clearing all counts
.re20:
        cmpdeco 1,r5,r5                 # Decrement the users count
        stob    r5,C_user_stop_cnt[g1*1]
        bne     .re100                  # Jif additional stops still outstanding
#
        cmpobe  0,r3,.re40              # Jif overall stop count already zero
        subo    1,r3,r3                 # Decrement the overall stop counter
        stob    r3,ca_stopcnt(r4)
#c       fprintf(stderr,"<GR>c_resume--caStopCnt=%lx\n",r3);
        cmpobne 0,r3,.re100             # Jif additional stops
#
# --- Ready semaphore 2 wait
#
c       TaskReadyByState(pcsem2wait);   # Ready semaphore 2 wait
#
# --- Stop I/O may have halted background flushing.  Clear the flag and see if
#       there are others still wanting it halted. If not, re-enable
#       background flushing.
#
.re40:
        ld      C_haltBackgroundFlag,r5 # Get the Background Halt Flag
        ldob    ca_status(r4),r3        # Get the Cache Status
        clrbit  hbgstop,r5,r5           # Clear Stop I/O is halting BG flushes
        st      r5,C_haltBackgroundFlag # Save new Halt Background Flag
        cmpobne 0,r5,.re50              # Jif cannot let Background Flushes go
        clrbit  ca_halt_background,r3,r3 # Show Background Flushing can start
        stob    r3,ca_status(r4)        # Save changed Cache Status
#
# --- Check if write cache is disabled.
#
.re50:
        bbc     ca_ena,r3,.re100        # Jif the cache is disabled
        bbc     ca_shutdown,r3,.re100   # Jif the cache is not shutdown
#
# --- If cache is not disabled, mark in the DRAM that write cache is
#       enabled and a cache restore operation on power up is possible.
#
        ldconst FALSE,g0                # Do not change Signature if not Init'd
        call    WC$markWCacheEn         # Will swap and CStatus could change
        ldob    ca_status(r4),r3        # Get Cache Status (may have changed)
        cmpobne 0,g0,.re60              # Jif enable failed
        clrbit  ca_shutdown,r3,r3       # Clear the cache shutdown bit
        b       .re80
#
.re60:
        clrbit  ca_ena,r3,r3            # Clear the Cache Enabled Flag
        setbit  ca_ena_pend,r3,r3       # Set the enable pending bit
.re80:
        stob    r3,ca_status(r4)        # Save changed Cache Status
#
# --- Exit
#
.re100:
        movl    r14,g0                  # Restore g1 and set Completion Code
        ret
#
#**********************************************************************
#
#  NAME: C$disable
#
#  PURPOSE:
#       Disable cache for a specific VID or globally all cache, no matter
#       if a virtual device is enabled or not.
#
#  DESCRIPTION:
#       For a VID specific disable cache, the data flushing process is begun,
#       and then controller will be returned to the caller.  When the data
#       has been flushed to disk, the cache is disabled for the device and a
#       message is sent to the MMC that the Cache data for this VID has been
#       flushed to disk.
#
#       For a global cache disable, all cache ops are stopped and the data
#       flushing process begun. This function will then return to the caller.
#       When all the data is flushed the cache is globally disabled (ignores
#       all the device cache enabled flags) and a message is sent to the MMC
#       that all the Cache data has been flushed to disk.
#
#  CALLING SEQUENCE:
#       call    C$disable
#
#  INPUT:
#       g0 = VID to Disable or 0xFFFFFFFF for a global Disable
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
C$disable:
        movl    g0,r14                  # Save g0 and g1
#
# --- Determine which path to take, global or specific VID and then go handle
#       appropriately
#
        ldconst 0xFFFFFFFF,r13          # Flag showing Global command
        cmpobne r13,g0,.cdis50          # Jif this is a specific VID to disable
#
# --- Handle the global cache disable -----------------------------------------
#
        lda     C_ca,r5                 # Get the Cache Information pointer
        ldob    ca_status(r5),r3        # Get the Cache Status
#
# --- Clear the global enable pending bit.  This bit is set when the cache
#        is globally enabled and the battery health isn't too healthy.
#
        clrbit  ca_ena_pend,r3,r3       # Clear global enable pending
        stob    r3,ca_status(r5)        # Save the Cache Status
        bbs     ca_ena,r3,.cdis30       # Jif the cache is not disabled yet
#
#   The CCB needs to know the Cache completed flushing even when Write Cache
#       is already disabled.  Send the Flush complete message anyway.
#
        call    wc$MsgFlushComplete     # Report the completion (g0 already set)
        b       .cdis100                # All done!
#
# --- Disable the cache globally (if possible - could have an error on flush)
#       Set the Disable in Progress bit to prevent new data from getting
#        into cache during the Flush Process
#
.cdis30:
        setbit  ca_dis_ip,r3,r3         # Set the Disable In Progress flag
        stob    r3,ca_status(r5)        # Save the Cache Status
        ldconst FALSE,g0                # Do not wait for all data to be
                                        #  flushed before returning
        call    wc$FlInvAll             # Begin to flush all the VIDs - sets
                                        #  up for Background to complete if not
                                        #  done in the wc$FlInvAll
        b       .cdis100                # All done
#
# --- Handle the specific VID disable -----------------------------------------
#
.cdis50:
        ld      vcdIndex[r14*4],r10     # r10 = VCD Index
        ldob    vc_stat(r10),r7         # Get the current State
        bbc     vc_cached,r7,.cdis90    # Jif already disabled - inform the CCB
                                        #  that no data is in Cache for this VID
        setbit  vc_disable_ip,r7,r7     # Set the Disable in Progress bit
        stob    r7,vc_stat(r10)
        mov     r14,g0                  # g0 = VID to flush
        call    wc$FlInvVID             # Flush all cached data for this VID
        ld      vc_cache(r10),r7        # Get the Valid Data RB Root
        cmpobne 0,r7,.cdis100           # Jif there is more data - background
                                        #  task will handle the flush completion
        ld      vc_write_count(r10),r6  # r6 = # of outstanding host write cmds
        cmpobne 0,r6,.cdis100           # Jif possible to get more data
        ldob    vc_stat(r10),r7         # r7 = Cache Status (in case it changed)
        mov     r14,g0                  # g0 = restore to VID being processed
        clrbit  vc_cached,r7,r7         # Clear the Cached bit
        clrbit  vc_disable_ip,r7,r7     # Clear the Disable in progress bit
        bbc     vc_error,r7,.cdis90     # Jif the VID is in Error State
        ldconst 0,g1                    # g1 = No more Errors
        call    wc$MsgFlushRecovered    # Send message of Error State recovered
        clrbit  vc_error,r7,r7          # Clear the Error state
.cdis90:
        stob    r7,vc_stat(r10)         # Save the new status
#
# --- Report to the MMC that the VID flush has completed
#
                                        # g0 = VID
        call    wc$MsgFlushComplete     # Report the Flush complete
#
# --- Now that all the data for this VID is gone, handle the Global Disable
#       in Progress, if the cache is in that state
#
        lda     C_ca,r6                 # Get the Cache Info
        ldob    ca_status(r6),r7        # r6 = Cache Status
        bbc     ca_dis_ip,r7,.cdis100   # Jif there is no Global Disable in Prog
        call    wc$SetGlobalDisable     # Try and complete the Global Disable
#
# --- Exit
#
.cdis100:
        movl    r14,g0                  # Restore g0 and g1
        ret
#
#**********************************************************************
#
#  NAME: C$enable
#
#  PURPOSE:
#       Globally enable the usage of the individual VID cache flags.
#
#  DESCRIPTION:
#       The enable flag is set to enable the individual VID cache flags to
#       be used.  If a VID cache flag is set to not cache, this change will
#       have no effect.
#
#  CALLING SEQUENCE:
#       call    C$enable
#
#  INPUT:
#       g0 = VID to Enable or 0xFFFFFFFF for a Global Enable
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
        .globl C_Enable
C_Enable:
C$enable:
#
# --- Determine which path to take, global or specific VID and then go handle
#       appropriately
#
        mov     g0,r15                  # Save g0
        ldconst 0xFFFFFFFF,r13          # Flag showing Global command
        ldconst TRUE,r12                # r12 = Flag showing to clear counters
        cmpobne r13,g0,.cen50           # Jif this is a specific VID to enable
#
# --- Global Enable -----------------------------------------------------------
#       Determine first if the Cache is already enabled.  If so, turn off the
#       disable in progress and return (do not reset the counters - define
#       calls C$enable on Slave controllers all the time).  If not, check
#       to see if the Mirror Partner has been found before allowing
#       Cache to be enabled
#
        lda     C_ca,r5                 # Get the Cache Information pointer
        ldob    ca_status(r5),r3        # r3 = Cache Status Flag
        ldos    K_ii+ii_status,r14      # r14 = Initialization status
        bbc     ca_ena,r3,.cen05        # Jif Cache is not already Enabled
        ldconst FALSE,r12               # r12 = Flag to show not to clear cntrs
        ldob    ca_status2(r5),r4       # r4 = Status 2 (battery health) || ca_temp_disable
        cmpobne 0,r4,.cen25             # Jif Status 2 is not good
        b       .cen30                  # Go clear the disable in progress bit
#
.cen05:
        bbc     iimpfound,r14,.cen25    # Jif Mirror Partner is not ready yet
#
# --- Determine if the mirroring of data is to the BE or not.  If to another
#       controller, ensure that data can be mirrored before allowing the Write
#       Cache to be enabled.
#
        bbc     ca_nwaymirror,r3,.cen20 # Jif not mirroring to another controller
        bbs     ca_mirrorbroken,r3,.cen25 # Jif the Mirror Partner is broken
        bbs     ca_error,r3,.cen25      # Jif the Cache is in an error state
#
# --- Check the battery health and Temporary Disable bits.  If the battery
#        state is low or Temporary Disabled, only the global enable pending bit
#        is set.  The write cache is not enable at this time.
#
.cen20:
        ldob    ca_status2(r5),r4       # r4 = Status 2 (battery health) || ca_temp_disable
        cmpobne 0,r4,.cen25             # Jif Status 2 is not good
#
# --- Mark in the DRAM that write cache is enabled and
#       a cache restore operation on power up is possible.
#
        ldconst FALSE,g0                # Do not change Signature if not Init'd
        call    WC$markWCacheEn         # May swap and Cache Status Change
        ldob    ca_status(r5),r3        # r3 = Cache Status Flag (may have chgd)
        cmpobe  0,g0,.cen30             # Jif marking was successful
#
# --- Something is preventing caching at the moment, set the state to Enable
#       Pending until that item is cleared up.
#
.cen25:
        setbit  ca_ena_pend,r3,r3       # Set the enable pending bit
        stob    r3,ca_status(r5)        # Save the Cache Status Flag
#
# --- Mark in the DRAM that write cache is disabled.
#
        ldconst FALSE,g0                # Do not change Signature if not Init'd
        call    WC$markWCacheDis
        b       .cen60
#
# --- Enable the cache globally and start the Background Flush Task if in the
#       Not Ready state
#
.cen30:
        ld      c_bgflush,r8            # Get the BG Flush Task PCB address
        setbit  ca_ena,r3,r3            # Globally enable cache
        clrbit  ca_dis_ip,r3,r3         # Clear the Disable In Progress (if on)
        cmpobe  0,r8,.cen40             # Jif the BG Flush Task not running yet
        ldob    pc_stat(r8),r7          # Get the BG Flush Task Status
        cmpo    pcnrdy,r7               # Determine if BG Flush Task is Not Rdy
        sele    r7,pcrdy,r6             # Leave status alone or Ready if Not Rdy
.ifdef HISTORY_KEEP
c CT_history_pcb("C$enable setting ready pcb", r8);
.endif  # HISTORY_KEEP
        stob    r6,pc_stat(r8)          # Set the PCB to Ready or leave alone
#
.cen40:
        stob    r3,ca_status(r5)        # Save the Cache Status Flag
        b       .cen60                  # All done
#
# --- Handle the specific VID enable ------------------------------------------
#
.cen50:
        ld      vcdIndex[r15*4],r13     # r13 = VCD Index
        ldob    vc_stat(r13),r7         # Get the current state
        bbc     vc_cached,r7,.cen55     # Jif already enabled
        ldconst FALSE,r12               # Do not clear the Cache Counters (could
                                        #  be called multiple times in a row)
.cen55:
        setbit  vc_cached,r7,r7         # Set the VID Cached bit
        clrbit  vc_disable_ip,r7,r7     # Clear the Disable in Progress (if on)
        stob    r7,vc_stat(r13)         # Enable the cache for this VID
#
# --- Clear the Write Cache counters.  An input value of 0xFFFFFFFF
#     clears the cache counter for all VCD with cache enabled.
#
.cen60:
        cmpobe  FALSE,r12,.cen100       # Jif not to clear the counters
        mov     r13,g0                  # g0 = VCD pointer to clear
        call    c$clearCacheCounters
#
# --- Exit
#
.cen100:
        mov     r15,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: C$que
#
#  PURPOSE:
#       To provide a common means of queuing I/O requests for the incoming
#       c$exec module.
#
#  DESCRIPTION:
#       The ILT and associated request packet are queued for the c$exec
#       executive to process.  The executive is then activated to process
#       this request.  This routine may be called from either the process
#       or interrupt level.
#
#  CALLING SEQUENCE:
#       call    C$que
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
#  ILT USAGE:
#                           PARENT W/O            PARENT W/
#                            DEPENDENCY          DEPENDENCY
#       CALLER AREA         CALLEE AREA          CALLEE AREA
#       ___________         ___________          ___________
#                                                PCB  = Placeholder ILT
#       W0 = Hit Count      W0 = Hit Count       W0 = Hit Count
#                           W1 = Dependency Cnt  W1 = DEPENDENCY CNT
#
#                           W3 = ILT/SRP         W3 = ILT/SRP
#       W4(vrvrp) = VRP     W4 = VRP             W4 = VRP
#                           W5 = Ptr to Tree     W5 = Pointer to Tree Node
#       W6 = SGL
#
#**********************************************************************
#
# void C_que(ILT *pILT)
        .globl C_que
C_que:
.if     DEBUG_FLIGHTREC_C
        ldconst frt_c_que,r3            # Cache - C$que
        lda     -ILTBIAS(g1),r8         # r8 = ILT back one level to get values
        ld      vrvrp(r8),r4            # r4 = VRP
        ldos    vr_func(r4),r5          # r5 = VRP Function
        ldos    vr_vid(r4),r6           # r6 = VID
        ldl     vr_vsda(r4),r8          # r8/r9 = SDA, r9=lower
        shlo    8,r5,r5                 # Set up to have several values in parm0
        shlo    16,r6,r6
        or      r5,r3,r3
        or      r6,r3,r3                # r3 = VID, Function, Flight Recorder ID
        st      r3,fr_parm0             # VID, Function, Flight Recorder ID
        st      g1,fr_parm1             # ILT
        st      r4,fr_parm2             # VRP
        st      r9,fr_parm3             # Lower word of SDA
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_C
#
# --- Insert ILT into executive queue
#
        ld      C_exec_qht+qu_head,r4   # Get queue head
        ld      C_exec_qht+qu_tail,r5   # Get queue tail
        ld      C_exec_pcb,r6           # Get PCB
        ld      C_exec_cqd,r7           # Get current queue depth
        cmpobne 0,r4,.cq10              # Jif queue not empty
#
# --- Insert into empty queue
#
        mov     g1,r5                   # Update queue head/tail with
        mov     g1,r4                   #  single entry
        b       .cq20
#
# --- Insert into non-empty queue
#
.cq10:
        st      g1,il_fthd(r5)          # Append ILT to end of queue
        mov     g1,r5                   # Update queue tail
.cq20:
        st      r4,C_exec_qht+qu_head
        st      r5,C_exec_qht+qu_tail
        addo    1,r7,r7                 # Bump queue depth
        st      r7,C_exec_cqd
#
# --- Activate executive if necessary
#
        cmpobe  0,r6,.cq100             # Jif PCB is NULL
        ldob    pc_stat(r6),r3          # Get current process status
        mov     pcrdy,r8                # Get ready status
        cmpobne pcnrdy,r3,.cq100        # Jif status other than not ready
#
.ifdef HISTORY_KEEP
c CT_history_pcb("C$que setting ready pcb", r6);
.endif  # HISTORY_KEEP
        stob    r8,pc_stat(r6)          # Ready process
#
# --- Exit
#
.cq100:
        ret
#
#**********************************************************************
#
#  NAME: c$qio
#
#  PURPOSE:
#       To provide a common means of queuing I/O requests for the
#       c$ioexec module.
#
#  DESCRIPTION:
#       The ILT and associated request packet are queued for the executive
#       to process.  The executive is then activated to process this request.
#       This routine may be called from either the process or interrupt level.
#
#  CALLING SEQUENCE:
#       call    c$qio
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
#  ILT USAGE:
#       CALLER AREA
#       ___________
#       W4(vrvrp) = VRP
#       W6 = local SGL/buff
#       W7 = Block I/O ILT Level
#
#**********************************************************************
#
c$qio:
.if     DEBUG_FLIGHTREC_C
        ldconst frt_c_qio,r3            # Cache - C$qio
        lda     -ILTBIAS(g1),r8         # r8 = point back to the input parms
        ld      vrvrp(r8),r4            # r4 = VRP
        ldos    vr_func(r4),r5          # r5 = VRP Function
        ldos    vr_vid(r4),r6           # r6 = VID
        ldl     vr_vsda(r4),r8          # r8/r9 = SDA, r8 = upper
        shlo    8,r5,r5                 # Set up to have several values in parm0
        shlo    16,r6,r6
        or      r5,r3,r3
        or      r6,r3,r3                # r3 = VID, Function, Flight Recorder ID
        st      r3,fr_parm0             # VID, Function, Flight Recorder ID
        st      g1,fr_parm1             # ILT
        st      r4,fr_parm2             # VRP
        st      r8,fr_parm3             # Upper word of the SDA
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_C
#
# --- Insert ILT into executive queue
#
        ld      C_ioexec_qht+qu_head,r4 # Get queue head
        ld      C_ioexec_qht+qu_tail,r5 # Get queue tail
        ld      C_ioexec_pcb,r6         # Get PCB
        ld      C_ioexec_cqd,r7         # Get current queue depth
        cmpobne 0,r4,.cqi10             # Jif queue not empty
#
# --- Insert into empty queue
#
        mov     g1,r5                   # Update queue head/tail with
        mov     g1,r4                   #  single entry
        b       .cqi20
#
# --- Insert into non-empty queue
#
.cqi10:
        st      g1,il_fthd(r5)          # Append ILT to end of queue
        mov     g1,r5                   # Update queue tail
.cqi20:
        st      r4,C_ioexec_qht+qu_head
        st      r5,C_ioexec_qht+qu_tail
        addo    1,r7,r7                 # Bump queue depth
        st      r7,C_ioexec_cqd
#
# --- Activate executive if necessary
#
        ldob    pc_stat(r6),r3          # Get current process status
        mov     pcrdy,r8                # Get ready status
        cmpobne pcnrdy,r3,.cqi100       # Jif status other than not ready
#
.ifdef HISTORY_KEEP
c CT_history_pcb("c$qio setting ready pcb", r6);
.endif  # HISTORY_KEEP
        stob    r8,pc_stat(r6)          # Ready process
#
# --- Exit
#
.cqi100:
        ret
#
#**********************************************************************
#
#  NAME: C$quedrp
#
#  PURPOSE:
#       To provide a common means of queuing DRPs to the c$drpexec executive
#
#  DESCRIPTION:
#       The ILT and associated request packet are queued to the c$drpexec
#       executive to process.
#
#  CALLING SEQUENCE:
#       call    C$quedrp
#
#  INPUT:
#       g1 = ILT
#           W4 (vrvrp) = DRP at the previous ILT level
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
C$quedrp:
.if     DEBUG_FLIGHTREC_C
        ld      vrvrp-ILTBIAS(g1),r4    # r4 = DRP
        ld      dr_req_address(r4),r7   # r7 = Datagram Request Address
        ldob    dgrq_fc(r7),r6          # r6 = Datagram function
        ldconst frt_c_quedrp,r3         # Cache - C$quedrp
        ldos    dr_func(r4),r5          # r5 = DRP Function
        shlo    16,r5,r5                # Set up to have several values in parm0
        or      r5,r3,r3                # r3 = DRP Function, Flight Recorder ID
        shlo    8,r6,r8                 # Set up to have several values in parm0
        or      r8,r3,r3                # r3 = DRP func, DG func, Flight Rec ID
        st      r3,fr_parm0             # DRP func, DG func, Flight Recorder ID
        st      g1,fr_parm1             # ILT
        st      r4,fr_parm2             # DRP
        st      r7,fr_parm3             # Datagram Request Address
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_C
#
# --- Insert ILT into the drp executive queue
#
        lda     C_drpexec_qht,r15       # r15 = DRP Queue pointer
        lda     C_drpexec_cqd,r14       # r14 = DRP Current Queue Depth pointer
        ld      C_drpexec_pcb,r6        # r6 = DRP PCB pointer
        ld      qu_head(r15),r4         # r4 = Queue Head
        ld      qu_tail(r15),r5         # r5 = Queue Tail
        ld      (r14),r7                # r7 = Current Queue Depth
        cmpobne 0,r4,.cqd50             # Jif queue not empty
#
# --- Insert into empty queue
#
        mov     g1,r5                   # Update queue head/tail with
        mov     g1,r4                   #  single entry
        b       .cqd60
#
# --- Insert into non-empty queue
#
.cqd50:
        st      g1,il_fthd(r5)          # Append ILT to end of queue
        mov     g1,r5                   # Update queue tail
.cqd60:
        st      r4,qu_head(r15)
        st      r5,qu_tail(r15)
        addo    1,r7,r7                 # Bump queue depth
        st      r7,(r14)
#
# --- Activate drp executive if necessary
#
        ldob    pc_stat(r6),r3          # Get current process status
        mov     pcrdy,r8                # Get ready status
        cmpobne pcnrdy,r3,.cqd100       # Jif status other than not ready
#
.ifdef HISTORY_KEEP
c CT_history_pcb("C$quedrp setting ready pcb", r6);
.endif  # HISTORY_KEEP
        stob    r8,pc_stat(r6)          # Ready process
#
# --- Exit
#
.cqd100:
        ret
#
#**********************************************************************
#
#  NAME: c$qerror
#
#  PURPOSE:
#       To provide a common means of queuing operations that have been halted
#       or are stalled because of mirroring problems.
#
#  DESCRIPTION:
#       The ILT is queued to the Error Process to be restarted when the
#       current "Error State" problem is resolved.
#
#  CALLING SEQUENCE:
#       call    c$qerror
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
c$qerror:
#
# --- Insert ILT into executive queue
#
        ld      C_error_qht+qu_head,r4  # Get queue head
        ld      C_error_qht+qu_tail,r5  # Get queue tail
        ld      C_error_cqd,r7          # Get current queue depth
        cmpobne 0,r7,.cqerr10           # Jif queue not empty
#
# --- Insert into empty queue
#
        mov     g1,r5                   # Update queue head/tail with
        mov     g1,r4                   #  single entry
        b       .cqerr20
#
# --- Insert into non-empty queue
#
.cqerr10:
        st      g1,il_fthd(r5)          # Append ILT to end of queue
        mov     g1,r5                   # Update queue tail
.cqerr20:
        mov     0,r6                    # r6 = Clearing Register
        addo    1,r7,r7                 # Bump queue depth
        st      r4,C_error_qht+qu_head
        st      r5,C_error_qht+qu_tail
        st      r7,C_error_cqd
        st      r6,il_fthd(g1)          # Clear Fwd Pointer for this ILT
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: c$exec
#
#  PURPOSE:
#       To provide a means of processing requests at the process level
#       which have been previously queued to this module.
#
#  DESCRIPTION:
#       The queuing routine C$que deposits a request into the que and
#       activates this executive if necessary.  This executive extracts
#       the next request from the queue and initiates the appropriate
#       request(s).
#
#       Precedence will be checked for I/O routines and then queued to the
#       c$ioexec to be finished when no blocking conditions exist.
#
#       For non-precedence checking type operations (Reserve, Release, etc.),
#       this module will execute the request immediately without the necessary
#       extra overhead of precedence checking and queueing.
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
# --- Set this process to not ready
#
.ex10:
        ld      K_xpcb,r15              # Get this PCB
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
#
# --- Exchange processes ----------------------------------------------
#
c$exec:
.ex30:
        call    K$qxchang               # Exchange processes
        ldconst FALSE,r13               # Set flag that no VLink Ops Sent
#
# --- Stall when stop counter active (Indicates that a configuration
#     change is underway)
#
.ex40:
        lda     C_ca,r4                 # Get the CA Structure Address
        ldob    ca_stopcnt(r4),r3       # Get stop count
        cmpobe  0,r3,.ex60              # Jif not active
#
#       Determine if VLink type ops are allowed or not.  If so, search the
#       queue looking for these type of ops.  If one is found, remove it from
#       the queue, update the Current Queue Depth, and continue the op.
#
        ld      c_allowVLinkOps,r3      # Get the VLinks Allowed Flag
        cmpobe  FALSE,r3,.ex55          # Jif VLink Ops are treated like normal
        ld      C_exec_cqd,r3           # Get the current queue depth
        ld      C_exec_qht,r12          # Get the head of the queue
        lda     C_exec_qht,r8           # Save as the previous ILT
        cmpobe  0,r3,.ex53              # Jif there are no ops on the queue
        mov     r8,r9                   # Save to tell if only one entry on que
.ex43:
        ld      vrvrp-ILTBIAS(r12),r14  # Get the VRP
        ldob    vr_options(r14),r14     # Get the options flag
        ld      il_fthd(r12),r7         # Get the next ILT
        bbc     vrvlinkop,r14,.ex50     # Jif this is not a VLink Op
        st      r7,il_fthd(r8)          # Remove this ILT from the queue (Note:
                                        #  this assumes il_fthd = 0 and will
                                        #  work the same as C_exec_qht if this
                                        #  ILT is the head of the queue)
        cmpobne 0,r7,.ex47              # Jif this is not the last entry on que
        cmpo    r8,r9                   # Determine if this is the last entry
        sele    r8,r7,r10               # Select zero or previous entry as tail
        st      r10,C_exec_qht+4        # Set the new tail
.ex47:
        subo    1,r3,r3                 # Adjust the current queue depth
        st      r3,C_exec_cqd
        ldconst TRUE,r13                # Set flag showing a VLink Op was sent
        b       .ex70                   # Go execute this particular VLink op
#
.ex50:
        mov     r12,r8                  # Save the previous ILT
        mov     r7,r12                  # Make the next ILT the current ILT
        cmpobne 0,r7,.ex43              # Jif not the last ILT
.ex53:
        st      FALSE,c_allowVLinkOps   # Reset VLinks Allowed Flag (there
                                        #  were no VLink ops on queue)
        cmpobe  TRUE,r13,.ex55          # Jif a VLink Op was sent
#
#   Show that no VLinks Ops were found (the absence of this message shows that
#       a VLink was found and sent to the BE).  This is the same as the C$Stop
#       recovery message but the time = 0 and the function shows who logged the
#       message.
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst 0,r5                    # Zeroing register
        st      r5,log_c_stop_time(g0)  # Zero to show special log
        lda     c$exec,r4               # Show this log coming from this exec
        st      r4,log_c_stop_function(g0) # Save the function logging the message
        st      r3,log_c_stop_corc(g0)  # Save how many ops on the queue
        st      r5,log_c_stop_cowsrpc(g0) # Zero - not used
        st      r5,log_c_stop_corsrpc(g0) # Zero - not used
        st      r5,log_c_stop_dvlorc(g0) # Zero - not used
        st      r5,log_c_stop_linkorc(g0) # Zero - not used
        ldconst mlecstoplog,r5          # get log code
        st      r5,mle_event(g0)
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], log_c_stop_log_size);
.ex55:
#c       fprintf(stderr,"<GR>Putting c$exec in pcsem2 wait state...\n");
c       TaskSetMyState(pcsem2wait);     # Set wait on semaphore 2
        b       .ex30
#
# --- Get next queued request
#
.ex60:
        ld      C_exec_cqd,r3           # Get current queue depth
        st      FALSE,c_allowVLinkOps   # Reset the VLinks Allowed Flag (normal
                                        #  path at this time)
        cmpobe  0,r3,.ex10              # Jif no ops outstanding
#
        subo    1,r3,r3                 # Adjust current queue depth
        st      r3,C_exec_cqd
        ld      C_exec_qht+qu_head,r6   # Get queue head
        ld      C_exec_qht+qu_tail,r7   # Get queue tail
#
# --- Dequeue selected request (FIFO fashion)
#
        mov     r6,r12                  # Isolate queued ILT
#
        ld      il_fthd(r6),r6          # Dequeue ILT
        cmpo    0,r6                    # Update queue head/tail
        sele    r7,0,r7
        st      r6,C_exec_qht+qu_head
        st      r7,C_exec_qht+qu_tail
#
# --- Copy ILT parameters from caller to callee level and adjust the outstanding
#       request count
#
.ex70:
c       C_orc++;                        # Increment the outstanding request cnt
.ifdef M4_DEBUG_C_orc
c CT_history_printf("%s%s:%u: C_orc incremented to %lu\n", FEBEMESSAGE,__FILE__, __LINE__, C_orc);
.endif  # M4_DEBUG_C_orc
        ld      il_misc-ILTBIAS(r12),r6 # Copy the ILT Parms Pointer
        st      r6,il_misc(r12)         # Save the ILT Parms Pointer at this ILT
        ld      vrvrp-ILTBIAS(r12),r14  # Copy the VRP
        st      r14,vrvrp(r12)          # Save VRP at this level of ILT
        ldob    vr_options(r14),r8      # Get the VRP Options
        setbit  vrcorc,r8,r8            # Show this op as a C_orc type op
        stob    r8,vr_options(r14)      # Update VRP Options
#
# --- Validate some of the VRP fields
#
        mov     r14,g2                  # Set up the VRP
        mov     r12,g1                  # Set up the ILT
        call    c$vrpcheck              # Validate some VRP fields
        cmpobe  ecok,g2,.ex80           # Jif all is OK
        mov     g2,r3                   # Store the error code
        b       .ex1200                 # Go report the error
#
# --- Isolate VCD
#
.ex80:
        ldos    vr_func(r14),r6         # r6 = function code
        ldos    vr_vid(r14),r8          # Get virtual ID
c       record_cache(FR_CACHE_EXEC, (void *)r14);
.if     DEBUG_FLIGHTREC_C
        ld      vr_vlen(r14),r7         # Get the length of the op
        ldconst frt_c_execb,r3          # Cache - c$exec
        shlo    8,r6,r4                 # Set up to have several values in parm0
        or      r4,r3,r3
        shlo    16,r8,r4
        or      r4,r3,r3                # r3 = VID, Function, Flight Recorder ID
        st      r3,fr_parm0             # VID, Function, Flight Recorder ID
        st      r12,fr_parm1            # ILT
        st      r14,fr_parm2            # VRP
        st      r7,fr_parm3             # Op Length
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_C
        ldconst MAXVIRTUALS,r4          # Get the number of VIDs to cycle thru
        cmpobe  vrprrelresv,r6,.ex120   # Jif priority release reserve
        cmpobge r8,r4,.ex1100           # Jif invalid virtual ID (boundary)
#
        ld      vcdIndex[r8*4],r10      # Get VCD
        and     3,r6,r3                 # Check for control request
        cmpobe  0,r10,.ex1100           # Jif invalid virtual ID
#
# --- Check for control related requests other than verify checkword
#
        cmpobe  vrverifyc,r6,.ex200     # Jif verify checkword
        cmpobne 0,r3,.ex200             # Jif function not control related
        cmpobe  vrresv,r6,.ex100        # Jif reserve
        cmpobe  vrrelresv,r6,.ex110     # Jif release reservation
        cmpobe  vrtstresv,r6,.ex130     # Jif test reserve
        b       .ex1130                 # VRP function not supported!
#
# --- Process reserve request (just return and let Magdrvr handle reservations)
#
.ex100:
        b       .ex1160                 # Complete request
#
# --- Process release reserve request (just return - let Magdrvr handle)
#
.ex110:
        b       .ex1160
#
# --- Process priority release reserve request (let Magdrvr handle)
#
.ex120:
        b       .ex1160
#
# --- Process test reserve request (let Magdrvr handle)
#
.ex130:
        b       .ex1160
#
# --- Process I/O request ---------------------------------------------
#
.ex200:
#
# --- Calculate an Op Throttle Value (OTV) and save in VRP for later use.
#       The OTV = Op Type* Op Size (the Op Throttle Value will be used once
#       precedence checking also shows the op can continue).  And then
#       determine if the op can continue or if it needs to be queued because
#       of VDisk Throttling needs
#
        ld      vr_vlen(r14),r4         # Get length
        chkbit  1,r6                    # Determine if a "Write" Type Op
        sele    READ_THROTTLE_VALUE,WRITE_THROTTLE_VALUE,g2 # Select type of op
        mulo    r4,g2,g0                # g0 = OTV
        st      g0,vr_otv(r14)          # Save OTV in the VRP
#
        mov     r10,g0                  # g0 = VCD pointer
        ldconst TRUE,g1                 # g1 = Check queue to determine if OK
        call    c$queryOpSend           # Determine if throttling needs to occur
                                        # g0 = TRUE of OK to send, FALSE if not
        cmpobe  TRUE,g0,.ex220          # Jif OK to send the op on its way
        mov     r10,g0                  # Set up the VCD pointer to queue to
        mov     r12,g1                  # Set up the ILT that will be queued
        call    c$opQueue               # Queue the Op on the VCD waiting queue
        b       .ex40                   # Go handle the next request
#
# --- Handle the I/O by checking for precedence
#
.ex220:
#
#       Make sure there is enough Non-cached DRAM available
#
        ldconst iltsiz,g2               # Set the size needed for a placeholder
        call    c$rdchk                 # Make sure there is enough room
        mov     r8,g0                   # Set up VID
        mov     r12,g1                  # Set up the ILT
        call    WC$coverlap             # Check precedence - will call the
                                        #  correct routine to handle or queue
        b       .ex40
#
# --- Report invalid VID
#
.ex1100:
        ldconst ecinvvid,r3             # Set invalid VID
        b       .ex1200
#
# --- Report invalid function
#
.ex1130:
        ldconst ecinvfunc,r3            # Set invalid function
        b       .ex1200
#
# --- Report OK
#
.ex1160:
        ldconst ecok,r3                 # Set OK
#
# --- Complete request
#
.ex1200:
.if 0 #CSTOP_SAN1171,1416,1670
        ldob    vr_options(r14),r6
        clrbit  vrnotcompleted,r6,r6    # Indicate VRP is completed
        stob    r6,vr_options(r14)      # Update vrp options
.endif
        stob    r3,vr_status(r14)       # Set error code
        mov     r3,g0                   # Set up the Completion Code
        mov     r12,g1                  # Complete this request
        call    K$comp
#
        ld      C_orc,r3                # Load, Decrement, and Store the
        subo    1,r3,r3                 #  outstanding request count
.ifdef M4_DEBUG_C_orc
c CT_history_printf("%s%s:%u: C_orc starts at %lu, ends at %lu\n", FEBEMESSAGE,__FILE__, __LINE__, C_orc, r3);
.endif  # M4_DEBUG_C_orc
        st      r3,C_orc
        b       .ex40
#
#**********************************************************************
#
#  NAME: c$ioexec
#
#  PURPOSE:
#       Continue the process of setting up the I/O to the upper and lower
#       layers for the original request.
#
#  DESCRIPTION:
#       After the original operation has passed the precedence checking,
#       the operation will come to this routine to be passed to the upper
#       or lower layer.  Allocation of resources will be made, the SRP created
#       to handle the host side of the transfer, and the I/O queued to the
#       next appropriate layer.
#
#       If a cache is not installed in the system or a specific virtual
#       device is not cached, this module manages the local memory
#       assignments for I/O buffers and also controls the actual
#       movement of data to/from these buffers and the host memory.
#
#       When a cache is present in the system and a specific virtual
#       device is cached, the caching of writes is handled by this
#       module.  Reads are handled a little differently.  If the data is
#       resident within the cache, it is returned directly from the
#       cache to the host memory.  If the data is not resident within the
#       cache, the read is handled in the same manner as if a cache is
#       not installed.
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
# --- Set this process to not ready
#
.ci10:
        ld      K_xpcb,r15              # Get this PCB
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
#
# --- Exchange processes ----------------------------------------------
#
c$ioexec:
        call    K$qxchang               # Exchange processes
#
# --- Get next queued request
#
.ci50:
        ld      C_ioexec_cqd,r3         # Get current queue depth
        cmpobe  0,r3,.ci10              # Jif none
#
        subo    1,r3,r3                 # Adjust current queue depth
        st      r3,C_ioexec_cqd
        ld      C_ioexec_qht+qu_head,r6 # Get queue head
        ld      C_ioexec_qht+qu_tail,r7 # Get queue tail
#
# --- Dequeue selected request (FIFO fashion)
#
        mov     r6,g1                   # Isolate queued ILT
#
        ld      il_fthd(r6),r6          # Dequeue ILT
        cmpo    0,r6                    # Update queue head/tail
        sele    r7,0,r7
        st      r6,C_ioexec_qht+qu_head
        st      r7,C_ioexec_qht+qu_tail
#
# --- Copy the callers parameters from the ILT to this layers ILT
#
        ld      vrvrp-ILTBIAS(g1),g14   # Get the VRP
        ld      il_misc-ILTBIAS(g1),r5  # Get the ILT Parms Pointer
        st      g14,vrvrp(g1)           # Save the VRP
        st      r5,il_misc(g1)          # Save the ILT Parms Pointer
        ldos    vr_func(g14),r6         # Get the function code
#
# --- Isolate VCD
#
        lda     C_ca,r3                 # Point to the Cache Information
        ldob    ca_status(r3),r7        # Get the global caching flag
        ldos    vr_vid(g14),r3          # Set virtual ID
        ld      vcdIndex[r3*4],g0       # Retrieve VCD pointer for this VID
        ldob    vc_stat(g0),g2          # g2 = the VID Status
##         c       record_cache(FR_CACHE_IOEXEC, (void *)g14);
.if     DEBUG_FLIGHTREC_C
        ldconst frt_c_execio,r9         # Cache - c$ioexec
        shlo    8,r6,r8                 # Set up to have several values in parm0
        or      r8,r9,r9                # r9 = Function, Flight Recorder ID
        shlo    16,r3,r8
        or      r8,r9,r9                # r9 = VID, Function, Flight Recorder ID
        st      r9,fr_parm0             # VID, Function, Flight Recorder ID
        st      g1,fr_parm1             # ILT
        st      g14,fr_parm2            # VRP
        shlo    16,r7,r9                # Set up to save caching flags
        or      r9,g2,r9                # r9 = Global Caching & VID Status Flags
        st      r9,fr_parm3             # Global Caching and VID Status Flags
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_C
#
# --- If split request - bypass cache
#
        ld      vr_use0(g14),r4         # Get the parent VRP
        cmpobne 0,r4,.ci100             # Jif parent VRP present - SPLIT req
        bbc     ca_ena,r7,.ci100        # Jif if all caching is disabled
        bbc     vc_cached,g2,.ci100     # Jif VID is not cached
        mov     r3,g0                   # Set up for cached Op - g0 = VID
#
# --- Process request for cached virtual device -----------------------
#
# --- r6 = function code
#     g0 = VID
#     g1 = ILT
#
        cmpobe  vroutput,r6,.ci70       # Jif write request
#
# --- Process read, verify checksum, verify data, write/verify,
#     and synchronize cache I/O requests directed
#
        mov     g1,r3                   # Preserve ILT pointer
        call    WC$Rsubmit              # Perform cached read if possible
#
# --- g12 return value T/F; T = Read occurred from cache   F = no hit
#
        cmpobe  TRUE,g12,.ci50          # Jif read was handled
        mov     r3,g1                   # Restore the ILT pointer
        b       .ci100                  # Go handle the op as a non-cached read
#
# --- Process cached write request
#
.ci70:
        mov     g1,r3                   # Preserve ILT pointer
        call    WC$Wsubmit              # Perform cached write if possible
#
# --- g12 return value T/F; T = I/O was cached F = bypassed
#
        cmpobe  TRUE,g12,.ci50          # Jif request was cached; process
                                        #  next item on queue;
        mov     r3,g1                   # Not cached - Restore ILT
#
# --- Process request for non-cached virtual device -------------------
#
.ci100:
        call    C$do_nc_op              # Go handle the non-cached Op
        b       .ci50                   # Go get the next queued op
#
#**********************************************************************
#
#  NAME: c$drpexec
#
#  PURPOSE:
#       Handle most DRPs from other controllers
#
#  DESCRIPTION:
#       The DRP is verified for proper values and if all is good, the
#       request in the Datagram is executed.
#
#       Datagram requests handled are:
#           Write Memory
#           Read Memory
#           Flush Write Cache Mirror Data
#           Clear Write Information
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
# --- Set this process to not ready
#
.cdrp10:
        ld      K_xpcb,r15              # Get this PCB
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
#
# --- Exchange processes ----------------------------------------------
#
c$drpexec:
        call    K$qxchang               # Exchange processes
#
# --- Get next queued request
#
.cdrp30:
        ld      C_drpexec_cqd,r3        # Get current queue depth
        cmpobe  0,r3,.cdrp10            # Jif none
#
        subo    1,r3,r3                 # Adjust current queue depth
        st      r3,C_drpexec_cqd
        ld      C_drpexec_qht+qu_head,r6 # Get queue head
        ld      C_drpexec_qht+qu_tail,r7 # Get queue tail
#
# --- Dequeue selected request (FIFO fashion)
#
        mov     r6,g13                  # Isolate queued ILT
#
        ld      il_fthd(r6),r6          # Dequeue ILT
        cmpo    0,r6                    # Update queue head/tail
        sele    r7,0,r7
        st      r6,C_drpexec_qht+qu_head
        st      r7,C_drpexec_qht+qu_tail
#
# --- Copy the callers parameters from the ILT to this layers ILT
#
        ld      vrvrp-ILTBIAS(g13),g14  # Get the DRP
        st      g14,vrvrp(g13)          # Save the DRP
        ld      il_misc-ILTBIAS(g13),r7 # Copy the ILT Parms Pointer
        st      r7,il_misc(g13)
        ldos    dr_func(g14),r6         # r6 = DRP function code
        ld      dr_req_address(g14),r4  # r4 = Datagram Request Address
        ld      dr_rsp_address(g14),r5  # r5 = Datagram Response Address
.if     DEBUG_FLIGHTREC_C
        ldconst frt_c_drpexec,r9        # Cache - c$drpexec
        shlo    16,r6,r8                # Set up to have several values in parm0
        or      r8,r9,r9                # r9 = Function, Flight Recorder ID
        st      r9,fr_parm0             # Function, Flight Recorder ID
        st      g13,fr_parm1            # ILT
        st      g14,fr_parm2            # DRP
        ld      dgrq_fc(r4),r9          # Dg Function, path, g0, g1
        st      r9,fr_parm3
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_C
#
# --- Determine if the DRP and Datagram looks good
#
        mov     dg_st_srvr,r3           # r3 = Datagram return status - failed
        mov     0,r8                    # r8-r11 = 0 to set up the response
        mov     0,r9
        mov     0,r10
        mov     0,r11
        lda     (dgrs_size<<8)(r8),r8   # Add the response header length
        stq     r8,(r5)                 # Set up the response header
#
        ldconst drdlmtocache,r15        # r15 = expected DRP function
        cmpobne r6,r15,.cdrp900         # Jif the DRP Function is bad
        ldob    dgrq_fc(r4),r8          # r8 = datagram function code
.if WC_ENABLE
        cmpobe  CAC0_fc_wrtmem,r8,.cdrp100 # Jif this is a Write Memory request
.else # WC_ENABLE
        cmpobe  CAC0_fc_wrtmem,r8,.cdrp910 # Jif this is a Write Memory request
                                        #  and fail the request (WC not OK)
.endif # WC_ENABLE
        cmpobe  CAC0_fc_setwriteinfo,r8,.cdrp500 # Jif this is Set Write Info
        cmpobe  CAC0_fc_clearwriteinfo,r8,.cdrp400 # Jif this is a Clear Wrt Inf
        cmpobe  CAC0_fc_rdmem,r8,.cdrp200 # Jif this is a Read Memory request
        cmpobe  CAC0_fc_fldata,r8,.cdrp300 # Jif this is a Flush Data request
        b       .cdrp910                # This is an unexpected request
#
# --- Write Memory Request ----------------------------------------------------
#
.if WC_ENABLE
.cdrp100:
#
# Verify the data addresses and lengths to reduce the possibility of corrupting
#   other data
#
        ldconst CAC0_WRTMEM_MASK,g0     # g0 = Mask of allowed memory regions
        ldob    dgrq_g0(r4),g1          # g1 = Memory regions specified
        mov     r4,g2                   # g2 = Datagram Request Address
        call    c$drpmemchk             # Check the memory address in the Dg
        cmpobe  1,g0,.cdrp930           # Jif the Regions bit are invalid
        cmpobe  2,g0,.cdrp920           # Jif the memory addresses fail
#
# Set up the SRP to get the memory region(s) written
#
        mov     g13,g1                  # g1 = DRP ILT
        mov     r4,g2                   # g2 = Datagram Request Address
        mov     srh2c,g3                # g3 = Transfer data from requestor
        call    c$adrpsrp               # Allocate and Build and SRP
#
# Initiate data transfer from host to BE960 memory locations
#
        ld      il_w3(g13),g1           # Get ILT/SRP
        lda     c$drpsrpcomp,g2         # Get completion routine
        call    c$callupper
        b       .cdrp30                 # Go get the next request
.endif   # WC_ENABLE
#
# --- Read Memory Request -----------------------------------------------------
#
.cdrp200:
#
# Verify the data addresses and lengths to reduce the possibility of corrupting
#   other data
#
        ldconst CAC0_RDMEM_MASK,g0      # g0 = Mask of allowed memory regions
        ldob    dgrq_g0(r4),g1          # g1 = Memory regions specified
        mov     r4,g2                   # g2 = Datagram Request Address
        call    c$drpmemchk             # Check the memory address in the Dg
        cmpobe  1,g0,.cdrp930           # Jif the Regions bit are invalid
        cmpobe  2,g0,.cdrp920           # Jif the memory addresses fail
#
# Initiate data transfer to host from BE960 memory locations by
# calling the completion handler.  Down this path te16g_MAGcomp
# sends the read buffer datagram header and the requested data.
#
        mov     g13,g1                  # Restore the ILT
        call    K$comp                  # Complete this DRP
        b       .cdrp30                 # Go get the next request
#
# --- Flush Write Cache Mirror Data Request -----------------------------------
#
.cdrp300:
        b       .cdrp910                # FINISH - show bad code until Finished
#
# --- Clear Write Information Request -----------------------------------------
#
.cdrp400:
#
# Load up the number of records to clear and start clearing them
#
        ldconst dgrq_size,r8            # Point beyond the request header
        addo    r8,r4,r8
        ld      CAC0_rq_cwi_num(r8),r9  # r9 = Number of records to clear
        lda     CAC0_rq_cwi_nvaa(r8),r8 # Point to the NVA Address
        mov     r9,r7                   # Save the total number of records
.cdrp420:
        cmpobe  0,r9,.cdrp440           # Jif all done
        ld      (r8),g0                 # g0 = NVA Record address for freeing
# Convert the address to offset before passing it to M$rp4nva,if the MP is BF.
c       if (MP_IS_BIGFOOT) g0 = g0 - NVRAM_P4_START_BF;
.ifdef   NVA_DEBUG
c fprintf(stderr,"%s%s:%u NVA offset passed to M$rp4nva in Cache Layer: %x\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # NVA_DEBUG
        call    M$rp4nva                # Free the NVA Record
        cmpobne 0,g0,.cdrp490           # Jif there was a failure
        subo    1,r9,r9                 # Decrement the counter
        addo    4,r8,r8                 # Point to the next record
        b       .cdrp420                # Continue with the next record
#
.cdrp440:
        mov     dg_st_ok,r3             # Set good status
        b       .cdrp1000               # Complete the operation
#
#   Error Occurred, determine which record was in error and report in EC2
#
.cdrp490:
.ifdef NVA_DEBUG
c fprintf(stderr,"%s%s:%u ERROR returned from M$rp4nva:CACHE LAYER <value is %d>\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif  #  NVA_DEBUG
        subo    r9,r7,g0                # g0 = Record number that failed (zero
                                        #      based)
        b       .cdrp940                # Go report the error
#
# --- Set Write Information Request -------------------------------------------
#
.cdrp500:
        lda     dgrq_size(r4),r14       # r14 = Extended Request Info pointer
        lda     dgrs_size(r5),r13       # r13 = Extended Response Info pointer
        ld      CAC0_rq_swi_num(r14),r12 # r12 = Number of records that follow
        ldconst dgrs_size,r9            # r9 = Size of the response header
        mulo    CAC0_rs_swi_size,r12,r8 # r8 = Size of all response records
        st      r9,dgrs_hdrlen(r5)      # Save the length of the response header
        addo    16,sp,sp                # Bump Stack Pointer to save info
        st      r12,-16(sp)             # Save the total number of records
        addo    CAC0_rs_swi_hdrsz,r8,r8 # r8 = Total size of extended Resp data
        bswap   r8,r8                   # r8 = byte swapped ext. response len
        st      r12,CAC0_rs_swi_num(r13) # Save the number of records returned
        st      r8,dgrs_resplen(r5)     # Save the response length
        lda     CAC0_rs_swi_hdrsz(r13),r13 # r13 = pointer to response records
        addo    CAC0_rq_swi_hdrsz,r14,r14 # Point to the extended data records
        lda     P4_nvac,g1              # Get NVA control structure address
        mov     r5,r11                  # Save r5 (used below in call)
.cdrp550:
        ld      CAC0_rq_swi_nvaa(r14),g0 # g0 = NVAA Address to use
        ldos    CAC0_rq_swi_vid(r14),r4 # r4 = VID of the Op
        ld      CAC0_rq_swi_len(r14),r5 # r5 = Length of the Op
        ldl     CAC0_rq_swi_slba(r14),r6 # r6,r7 = Starting LBA of the Op
        ld      nc_nvarec(g1),r8        # Get addr of 1st record
        ldconst NVSRAMP4SIZ-nvabasesiz,r9 # Size of Part 4
#
# Checking for Invalid NVA Address
#
# nva Offset
# Convert address to offset if the MP is Bigfoot
c       if (MP_IS_BIGFOOT) g0 = g0-NVRAM_P4_START_BF;
        ldconst nvabasesiz,r9
        cmpobl  g0,r9,.cdrp590          # Jif g0 < NVA Header size : error
c       r9 = gMPNvramP4Size;
        cmpobge g0,r9,.cdrp590          # Jif g0 >= highest record + 1: error
.ifdef NVA_DEBUG
c fprintf(stderr,"%s%s:%u Received NVA offset from MP is:%x\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # NVA_DEBUG
        balx    M$wnva,r15              # Input: g0 = NVA Record Address
                                        #        g1 = NVA Controller Structure
                                        #        r4 = Record ID (VID)
                                        #        r5 = Number of Sectors
                                        #        r6,r7 = Starting LBA
                                        # Destroys: r3 - r9
# Convert the offset back to address
c       if (MP_IS_BIGFOOT)g0+=NVRAM_P4_START_BF;
.ifdef  NVA_DEBUG
c fprintf(stderr,"%s%s:%u NVA OFFSET after M$wnva executed:%x", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif #   NVA_DEBUG
        ld      CAC0_rq_swi_ilt(r14),r3 # r3 = ILT associated with this record
        subo    1,r12,r12               # Decrement the number of records left
        st      r3,CAC0_rs_swi_ilt(r13) # Save this ILT that matches the Rec ID
        st      g0,CAC0_rs_swi_nvaa(r13) # Save the NVA Address for freeing
                                        #           when the write completes
        lda     CAC0_rs_swi_size(r13),r13 # Point to the next output record
        addo    CAC0_rq_swi_size,r14,r14 # Point to the next input record
        cmpobne 0,r12,.cdrp550          # Jif more records to do
        mov     dg_st_ok,r3             # Set good status
        mov     r11,r5                  # Restore r5 (used below in call)
        subo    16,sp,sp                # Reset the stack pointer
        b       .cdrp1000               # Complete the request
#
#   An invalid address was passed to this routine
#
.cdrp590:
.ifdef NVA_DEBUG
c fprintf(stderr,"%s%s:%u @@@@...RECEIVED BAD NVA ADDRESS FROM MIRROR PARTNER...@@@@", FEBEMESSAGE, __FILE__, __LINE__);
.endif # NVA_DEBUG
        ld      -16(sp),r4              # r4 = Number of records
        mov     r11,r5                  # Restore r5 (used below in call)
        subo    16,sp,sp                # Reset the stack pointer
        mov     dg_st_srvr,r3           # r3 = Datagram return status - failed
        subo    r12,r4,g0               # Pointer to which record failed (zero
                                        #  based)
        b       .cdrp940                # Go report the error
#
# --- Something is wrong with the request, report the problem -----------------
#
.cdrp900:                               # ------- BAD DRP FUNCTION CODE ------
        ldconst CAC0_BAD_DRP_FC,g8      # Set EC #1 as bad DRP function code
        stob    g8,dgrs_ec1(r5)
        stos    r6,dgrs_g0(r5)          # Save the DRP Function code
        b       .cdrp1000               # Report the error
#
.cdrp910:                               # ---- BAD DATAGRAM FUNCTION CODE ----
        ldconst CAC0_BAD_DG_FC,g8       # Set EC #1 as bad Datagram Function
        stob    g8,dgrs_ec1(r5)
        stob    r8,dgrs_ec2(r5)         # Save the Datagram Function Code
        b       .cdrp1000               # Report the error
#
.cdrp920:                               # --- BAD MEMORY ADDRESS OR LENGTH ---
        ldconst CAC0_RANGE_ERROR,g8     # Set EC #1 to range error
        stob    g8,dgrs_ec1(r5)
        stob    g1,dgrs_ec2(r5)         # Save the entry number that failed
        b       .cdrp1000               # Report the error
#
.cdrp930:                               # --- BAD MEMORY ADDRESS OR LENGTH ---
        ldconst CAC0_BAD_MEM_REGION,g8  # Set EC #1 to region error
        stob    g8,dgrs_ec1(r5)
        stob    g1,dgrs_ec2(r5)         # Save the invalid region bits
        b       .cdrp1000               # Report the error
#
.cdrp940:                               # --- BAD NVA RECORD ID --------------
        ldconst CAC0_BAD_NVA_ID,g8      # Set EC #1 to Bad NVR Record ID
        stob    g8,dgrs_ec1(r5)
        stob    g0,dgrs_ec2(r5)         # Save number of the Record that failed
                                        #  to pass the Address checking
#
# --- Complete the Datagram and then get the next request
#
.cdrp1000:
        stob    r3,dgrs_status(r5)      # Save the Datagram Status
        mov     0,g0                    # g0 = Datagram Processed status
        stob    g0,dr_status(g14)       # Save the status in the DRP
        mov     g13,g1                  # Restore the ILT
        call    K$comp                  # Complete this DRP
        b       .cdrp30                 # Get the next request
#
#**********************************************************************
#
#  NAME: c$error
#
#  PURPOSE:
#       Handles the Error Queue to complete Operations held due to mirroring
#       failures to the Mirror Partner
#
#  DESCRIPTION:
#       This process should only be started when the operations in the Error
#       queue can now be completed.
#
#       This process will pull the stalled Operations from the Error queue and
#       call its completion routine.  The completion routine will then
#       do whatever is necessary to restart the failed operation.  Because ops
#       may be in queues at the time the command that started this task could
#       still get errors, this task will hang around until the Mirror Broken
#       status goes away or 30 minutes, whichever is shorter.
#
#  CALLING SEQUENCE:
#       process call (temporary fork)
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
c$error:
        mov     0,r15                   # r15 = Clearing register
        st      r15,C_error_loop_cnt    # Reset the loop counter (in case this
                                        #  task is already running, need to
                                        #  reset the counter to keep running
                                        #  since a new error occurred)
#
# --- Kick off the task to determine if it will be OK to restart mirroring
#       controller information to the mirror partner
#
        call    DLM$StartFECheckingMP
#
# --- Determine if this task is already running and if so return.  Else
#       continue the process.
#
        ld      C_error_pcb,r4          # r4 = Error PCB
        cmpobne 0,r4,.cerr1000          # Jif Error task is already running
#
        ld      K_xpcb,r3               # r3 = this processes PCB
        st      r3,C_error_pcb          # save to show this task is working
#
        lda     C_ca,r14                # r14 = pointer to Cache structure
        ldconst ERROR_TIME_LIMIT,r12    # r12 = Time amount to wait with no work
        ldconst ((3*STOP_TIME_WAIT)/ERROR_TIME_WAIT)+1,r11 # r11 = number
                                        #  of wait times before clearing
                                        #  error state when in a Stop condition
        mov     0,r10                   # r10 = Error & Stop State counter
#
# --- Get next queued request, if there is one.
#
.cerr20:
        ld      C_error_cqd,r3          # r3 = Current queue depth
        ld      C_error_loop_cnt,r13    # r13 = Current loop count
        ldob    ca_status(r14),r6       # r6 = Cache Status
        ldob    ca_stopcnt(r14),r4      # r4 = Stop count
        ld      C_orc,r5                # r5 = Outstanding request count
#
# --- Determine if this task can be ended yet or not
#
        addo    1,r13,r13               # increment the loop counter
        chkbit  ca_error,r6             # Test to see if in Error State again
        sele    0,r10,r10               # If not in Error - Clear r10
        bne     .cerr40                 # Jif error state flag is not on
        cmpobe  0,r4,.cerr30            # Jif no stop in progress
.ifdef M4_DEBUG_C_orc
c CT_history_printf("%s%s:%u: C_orc (%lu) checked for 0 (and branch on zero).\n", FEBEMESSAGE,__FILE__, __LINE__, C_orc);
.endif  # M4_DEBUG_C_orc
        cmpobe  0,r5,.cerr30            # Jif there are no outstanding ops
        cmpinco r11,r10,r10             # Compare against max wait count and Inc
        bg      .cerr40                 # Wait for C$Stop to see the error state
                                        #  before clearing it
.cerr30:
        clrbit  ca_error,r6,r6          # Clear the error state flag again. It
                                        #  may have been set because of retries.
        mov     0,r10                   # Clear the C$Stop and Error State cntr
        stob    r6,ca_status(r14)       # Save the new status
.cerr40:
        cmpobne 0,r3,.cerr100           # Jif more work to be done
        bbc     ca_mirrorbroken,r6,.cerr900 # Jif a new mirror partner (new
                                        #  errors will get new error task)
        st      r13,C_error_loop_cnt    # Save the latest loop counter
        cmpobe  r12,r13,.cerr900        # Jif looped long enough with no work
.cerr50:
        ldconst ERROR_TIME_WAIT,g0      # g0 = number of msec to wait for ops
        call    K$twait                 # go to sleep
        b       .cerr20                 # Determine if more work to do yet
#
# --- Work on the queued request.  Decrement the current queue depth, remove
#       from the queue, and then call the completion routine.
#
.cerr100:
        subo    1,r3,r3                 # Adjust current queue depth
        st      r15,C_error_loop_cnt    # Reset the loop counter
        st      r3,C_error_cqd          # Store the new queue depth
        ld      C_error_qht+qu_head,r4  # Get queue head
        ld      C_error_qht+qu_tail,r5  # Get queue tail
#
#   Dequeue selected request (FIFO fashion)
#
        mov     r4,g1                   # Isolate queued ILT
#
        ld      il_fthd(r4),r4          # Dequeue ILT
        cmpo    0,r4                    # Update queue head/tail
        sele    r5,0,r5
        st      r4,C_error_qht+qu_head
        st      r5,C_error_qht+qu_tail
#
#   Call the completion routine that will handle getting the operation
#       going again
#
        ld      il_cr(g1),r6            # r6 = Completion routine to call
                                        # g1 = ILT of completion routine
        st      r15,il_fthd(g1)         # Clear the forward thread
        callx   (r6)                    # Call the completion routine
        cmpobne 0,r10,.cerr50           # Jif C$Stop counting - delay for C$Stop
        b       .cerr20                 # Flush all the queue quickly
#
# --- All done, kill this process
#
.cerr900:
        st      r15,C_error_pcb         # Clear the PCB to show not running
.cerr1000:
        ret
#
#**********************************************************************
#
#  NAME: CA_OpRetryTask
#
#  PURPOSE:
#       To provide a means of Operations that need to be retried.
#
#  DESCRIPTION:
#       The Op is removed and determined what time length needs to occur before
#       this op needs to be reissued. Once it is time, the op is reissued to
#       the Lower Levels to try and complete again.
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
# --- Set this process to not ready
#
.cort10:
        ld      qu_pcb(r11),r15         # Get queue PCB
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
#
# --- Exchange processes ----------------------------------------------
#
CA_OpRetryTask:
        call    K$qxchang               # Exchange processes
.cort20:
#
# --- Get next queued request
#
        lda     CA_OpRetryQue,r11       # Get executive queue pointer
        ld      qu_head(r11),r12        # Get queue head
        cmpobe  0,r12,.cort10           # Jif nothing queued
#
# --- Determine if the Head still needs to wait before executing.  If the head
#       op still needs to wait, then do a Timed wait to get to the point where
#       the op can be issued again.
#
        ld      K_ii+ii_time,r4         # r4 = Current Time
        ld      il_w1(r12),r5           # r5 = Ops time to retry
        cmpobge r4,r5,.cort30           # Jif the op is ready to be retried
        subo    r4,r5,r5                # Difference in time for first op in
                                        # II Time Units
        ldconst QUANTUM,r6              # r6 = II Time Unit
        mulo    r5,r6,g0                # g0 = Time to wait
        call    K$twait                 # Wait the time requested
        b       .cort20                 # Check the new head again (could have
                                        #  been removed if a VDisk was deleted)
#
# --- If a C$Stop is outstanding, then have to walk the queue to let any
#       non-C_orc ops go unless it is a C_flush_orc op and Halt Background is
#       also on.  Else, continue to reissue the op.
#
.cort30:
        ldob    C_ca+ca_stopcnt,r3      # See if there is a Stop in progress
        ldob    C_ca+ca_status,r7       # r7 = Cache Status
        ld      vrvrp(r12),r5           # r5 = VRP Pointer
        ldob    vr_options(r5),r6       # r6 = VRP Options field
        cmpobe  0,r3,.cort60            # Jif there are no Stops in progress
#
#c       fprintf(stderr,"<GR>CA_OpRetryTask - vid=%lx vroptions=%lx lba=%llx r7=%lx cr=%0x\n",r3,r6,((VRP*)r5)->startDiskAddr,r7, (UINT32)((ILT*)r12)->cr);
        bbs     vrcorc,r6,.cort50       # Jif this is a C_orc op - check next
        bbc     vrforc,r6,.cort60       # Jif this is not a C_flush_orc - issue
        bbc     ca_halt_background,r7,.cort60 # Jif no Halt Background - issue
#
.cort50:
        ld      il_fthd(r12),r12        # r12 = next ILT on the list
        cmpobne 0,r12,.cort30           # Jif more ILTs to investigate
        ldconst 1000,g0                 # Nothing to try - delay 1 second and
        call    K$twait                 #  start all over
        b       .cort20
#
# --- Remove op from the queue (beginning, middle, or end) and issue it
#
.cort60:
        ld      il_fthd(r12),r8         # r8 = forward thread of ILT
        ld      il_bthd(r12),r9         # r9 = backward thread of ILT
        mov     r12,g1                  # Isolate next queued ILT
        st      r8,il_fthd(r9)          # put forward thread from removed ILT
                                        #  as forward thread of previous ILT
        cmpobne 0,r8,.cort70            # Jif non-zero forward thread
        mov     r11,r8                  # make base of queue the forward thread
        cmpo    r11,r9                  # Determine if backward thread <> base
        sele    r9,0,r9                 # If they are equal, then queue is empty
.cort70:
        st      r9,il_bthd(r8)          # put backward thread from removed ILT
                                        #  as backward thread of previous ILT
        ld      qu_qcnt(r11),r14        # Load queue count
        subo    1,r14,r14               # Adjust queue count
        st      r14,qu_qcnt(r11)        # Save updated queue count
#
# --- Clear out the VRP Status, increment the correct Outstanding Request
#       Count Queue, and increment the Op Throttle Values
#
        ldconst ecok,r3                 # Set the VRP status to good again
        stob    r3,vr_status(r5)
#
        bbc     vrcorc,r6,.cort80       # Jif this is not a C_orc type op
        ld      C_orc,r3                # Increment the C_orc counter
        addo    1,r3,r3
.ifdef M4_DEBUG_C_orc
c CT_history_printf("%s%s:%u: C_orc starts at %lu, ends at %lu\n", FEBEMESSAGE,__FILE__, __LINE__, C_orc, r3);
.endif  # M4_DEBUG_C_orc
        st      r3,C_orc
.if 0 #CSTOP_SAN1171,1416,1670
        setbit  vrnotcompleted,r6,r6    # Indicate VRP in progress
        stob    r6,vr_options(r5)       # Update vrp options
.endif
.cort80:
        bbc     vrforc,r6,.cort90       # Jif this is not a C_flush_orc type op
        ld      C_flush_orc,r3          # Increment the C_flush_orc counter
        addo    1,r3,r3
        st      r3,C_flush_orc
#
.cort90:
        ldos    vr_vid(r5),r9           # r9 = VID
        ld      vr_otv(r5),r3           # r3 = OTV from the VRP
        ld      vcdIndex[r9*4],r6       # r6 = VCD
        ld      C_ctv,r8                # r8 = CTV
        ld      vc_vtv(r6),r7           # r7 = VTV
        addo    r3,r8,r8                # Update the CTV
        addo    r3,r7,r7                # Update the VTV
.ifdef M4_DEBUG_C_ctv
c CT_history_printf("%s%s:%u: C_ctv starts=%lu ends=%lu vc_vtv[%ld]=%ld\n", FEBEMESSAGE,__FILE__, __LINE__, C_ctv, r8, r9,r7);
.endif  # M4_DEBUG_C_ctv
        st      r8,C_ctv
        st      r7,vc_vtv(r6)
#
# --- Send the request again to the lower layers
#
        call    CA_CallLower2           # Input: g1 = ILT
#
# --- Done with this op, go see if there is another one that needs to be issued
#
        b       .cort20                 # Go see if any more work to be done
#
#**********************************************************************
#
#  NAME: c$VCDQueueCheck
#
#  PURPOSE:
#       Determine which VCDs have ops waiting to be issued and issue any that
#       can be issued while still under the Throttling limits.
#
#  DESCRIPTION:
#       Walk the queue of VCDs looking for Ops that are waiting to see if they
#       can now be issued.  The VCDs are in order of time and so when an op is
#       issued from one VCD, it is removed from the list.  If there are more
#       ops waiting on the VCD, the VCD will be put on the tail of the list.
#       This allows a "Fairness" in allowing the oldest waiting VCD to have
#       a chance at starting an op.  This also allows the task to know it has
#       been completely through the list when the tail is reached and
#       nothing else can be issued.  The task will then go into "Not Ready"
#       state, waiting for another Op to complete and the process starts
#       all over again (if anything is on the queue).
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
c$VCDQueueCheck:
        ldconst FALSE,r4                # r4 = TRUE
        st      r4,C_vcd_wait_active    # The task is dormant
#
# --- Set this process to not ready
#
        ld      K_xpcb,r15              # Get this PCB
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
#
# --- Exchange processes ----------------------------------------------
#
        call    K$qxchang               # Exchange processes
#
        ldconst TRUE,r4                 # r4 = TRUE
        st      r4,C_vcd_wait_active    # The task is active
# --- Get the VCD queue and determine if anything is on it.  If so, start
#       walking it and starting any ops that can be started.
#
        ld      C_vcd_wait_head,r4      # r4 = VCD Head of Wait queue
        ld      C_vcd_wait_tail,r5      # r5 = VCD Tail of Wait queue
        cmpobe  0,r4,c$VCDQueueCheck    # Jif nothing on the queue
        mov     r4,r3                   # VCD to test at this time
#
#   Determine if the Op on the top of the VCD Op queue can be issued or not
#
.cvqc10:
        mov     r3,g0                   # g0 = VCD
        ldconst FALSE,g1                # g1 = Do not check queue to determine
                                        #  if it is OK to send the OP
        call    c$queryOpSend           # Determine if an op for this VCD can
                                        # be issued
                                        # g0 = TRUE if OK, FALSE if not
        cmpobne FALSE,g0,.cvqc20        # Jif this op can be issued for this VCD
        ld      vc_fwd_wait(r3),r3      # r3 = Next waiting VCD
        cmpobne r4,r5,.cvqc80           # Jif not the only VCD on list
        b       .cvqc90                 # Only this VCD on the list and could
                                        #  not issue an op, go wait for an op
                                        #  to complete.
#
#   The Op can be issued, remove it from the VCD Op Throttle queue, move this
#       VCD to the tail of the VCD Wait queue (if there are more ops still
#       outstanding), and then send the op.
#
.cvqc20:
        ld      vc_thead(r3),r6         # r6 = Op Throttle Queue head
        ld      vc_ttail(r3),r7         # r7 = Op Throttle Queue tail
        mov     r6,g1                   # g1 = current op going to be issued
        ld      il_fthd(r6),r6          # Dequeue ILT
        cmpo    0,r6                    # Update Op Throttle Queue head/tail
        sele    r7,0,r7
        st      r6,vc_thead(r3)
        st      r7,vc_ttail(r3)
#
#       Op removed from queue, now remove this VCD from the VCD Wait Queue
#
        ld      vc_fwd_wait(r3),r8      # r8 = VCD Wait Queue next VCD
        ld      vc_bwd_wait(r3),r9      # r9 = VCD Wait Queue previous VCD
        mov     r8,r10                  # r10 = Saved VCD forward pointer
        cmpo    0,r8                    # Determine if this is the last VCD
        sele    r5,r9,r5                # Set a possibly new VCD Wait Queue Tail
        cmpo    0,r9                    # Determine if this is the first VCD
        sele    r4,r8,r4                # Set a possibly new VCD Wait Queue Head
        be      .cvqc30                 # Jif Bkwd pointer is zero - no fix bck
        st      r8,vc_fwd_wait(r9)      # Set up the previous VCDs Fwd pointer
.cvqc30:
        cmpobe  0,r8,.cvqc40            # Jif Fwd pointer is zero - no fix fwd
        st      r9,vc_bwd_wait(r8)      # Set up the next VCDs backward pointer
.cvqc40:
        mov     0,r8                    # Clear out this VCD Wait Queue pointers
        mov     0,r9
#
#       VCD is removed from the VCD Wait Queue now.  Put it at the VCD wait
#           tail if there are any Ops still on the Op Wait Queue.
#
        cmpobe  0,r6,.cvqc60            # Jif there are no more ops on the queue
        mov     r5,r9                   # Set up the VCD Backward pointer
        mov     r3,r5                   # Set up the new VCD Wait Queue Tail
        cmpobe  0,r9,.cvqc50            # Jif there is no VCD backwards
        st      r3,vc_fwd_wait(r9)      # Set up the previous tails forward ptr
.cvqc50:
        cmpo    0,r4                    # Determine if nothing was on queue now
        sele    r4,r3,r4                # Set up new head if nothing on queue
.cvqc60:
        st      r8,vc_fwd_wait(r3)      # Save the new VCD wait fwd pointer
        st      r9,vc_bwd_wait(r3)      # Save the new VCD wait bwd pointer
#
#       Now ready to issue the op
#
        mov     0,r9                    # Clear the ILT Fwd/Bwd pointers
        st      r8,il_fthd(g1)
        st      r9,il_bthd(g1)
        ld      vrvrp(g1),g0            # Get the VRP
        ldos    vr_vid(g0),g0           # g0 = VID
                                        # g1 = ILT
        call    WC$coverlap             # Check precedence - will call the
                                        #  correct routine to handle or queue
        mov     r10,r3                  # Set up the next VCD pointer
#
#   Go to the next waiting VCD on the Wait Queue
#
.cvqc80:
        cmpobne 0,r3,.cvqc10            # Jif more VCDs waiting
        cmpobne r4,r5,.cvqc90           # Jif not just one VCD on list (went
                                        #  through entire list without sending
                                        #  any ops).
        mov     r4,r3                   # Set up this same VCD (single VCD list)
        cmpobne 0,r4,.cvqc10            # Jif there is this VCD on the queue
#
# --- All done, wait for more work to do
#
.cvqc90:
        st      r4,C_vcd_wait_head      # Save the new VCD Wait Queue head
        st      r5,C_vcd_wait_tail      # Save the new VCD Wait Queue tail
        b       c$VCDQueueCheck         # All Done - wait for more work
#
#**********************************************************************
#
#  NAME: c$clearWriteInfoTask
#
#  PURPOSE:
#       Handles processing the queued Clear Write Information ILTs and sending
#       the information to the Mirror Partner
#
#  DESCRIPTION:
#       This process will pull any queued Clear Write Information ILTs,
#       coalesce the entire queue into one packet, and then send to
#       the mirror partner.
#
#  CALLING SEQUENCE:
#       process call (permanent fork)
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
c$clearWriteInfoTask:
#
# --- Wait for the Mirror Partner to get set up before capturing who it is
#
        ldconst 1000,g0                 # g0 = Wait one second
.ccwit05:
        call    K$twait                 # Wait for Mirror Partner to get set up
        ldos    K_ii+ii_status,r11      # r11 = Initialization status
        bbc     iimpfound,r11,.ccwit05  # Jif Mirror Partner is not ready yet
#
# --- Register Set up to run the process
#
        ldconst 0,r15                   # r15 = Clearing register
        lda     C_ca,r12                # r12 = The Cache Information pointer
        b       .ccwit15                # Already waited, see if any work to do
#
# --- Exchange processes ----------------------------------------------
#
.ccwit10:
c       TaskSetMyState(pcnrdy);         # Set this process to not ready
        call    K$qxchang               # Exchange processes
#
# --- Determine if there is work to be done.  If not, wait some more.  If there
#       are queued items, and there is allowed another request to the Mirror
#       Partner, then send the whole lot to the Mirror Partner.
#
.ccwit15:
        ld      C_cwi_cqd,r14           # Get current queue depth
        ld      C_cwi_cdrp,r13          # Get the number of DRPs already sent
        cmpobe  0,r14,.ccwit10          # Jif there is no work to do
        cmpoble MAX_CWI_DRPS,r13,.ccwit10 # Jif no more DRPs are allowed (will
                                        #  be handled in the completion routine)
#
# --- All OK to send the queued requests.  Build up the ILT, DRP, Datagram, and
#       request packet to send the information to the Mirror Partner.
#
#   Ensure the Mirror Partner is ready to receive requests.  If there are no
#       outstanding DRPs, then go ahead and issue it (need one in the
#       error queue to get things going again).  If in Error State and there
#       are outstanding DRPs, leave items queued and wait until the error
#       state is fixed.  If in Mirror Broken state, free the queued ILT(s)
#       used in mirroring the original information.
#
        ldob    ca_status(r12),r3       # Get the Cache Status
        cmpobe  0,r13,.ccwit18          # Jif there are no outstanding DRPs
        bbs     ca_error,r3,.ccwit10    # Jif Mirroring is not OK - wait
#
.ccwit18:
        ld      C_cwi_qht,r10           # r10 = Queue Head at this time
        st      r15,C_cwi_cqd           # Clear the current queue depth
        st      r15,C_cwi_qht           # Clear the queue head and tail
        st      r15,C_cwi_qht+4
#
        bbc     ca_mirrorbroken,r3,.ccwit30 # Jif Mirror Partner is not broken
#
#   Mirror Partner is broken.  Clear out the NVA Addresses from the Cache mirror
#       of the P4 records.  This will leave the Cache copy in good shape for
#       when a Mirror Partner is assigned again.
#
        lda     C_wi_nvac,g1            # g1 = Cache mirror of P4 NVAC
        mov     r10,r8                  # r8 = Current record being worked on
.ccwit20:
        ld      il_wcdlmnvaa(r8),g0     # g0 = NVA Address
        ld      il_fthd(r8),r8          # r8 = The next ILT on the list
c        if (MP_IS_BIGFOOT) g0 = g0-NVRAM_P4_START_BF;
.ifdef NVA_DEBUG
c fprintf(stderr,"%s%s:%u 2.NVA offset passed to M$rnvaa: %x\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif   # NVA_DEBUG
        call    M$rnvaa                 # Free the NVA Address
        subo    1,r14,r14               # Decrement the number left to do
        cmpobne 0,r14,.ccwit20          # Jif more NVA Address's to free
#
#   Cleared some NVA records.  Call the routine to see if the Set Write Info
#       Engine needs kick started.
#
        call    c$setWriteInfoKick      # Go see if the SWI needs to be started
        b       .ccwit90
#
#   Mirror Partner OK.  Continue to build the ILT, DRP, Datagram, and request
#       packet to send to the Mirror Partner.
#
.ccwit30:
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        ldconst drpsiz+dgrq_size+dgrs_size,g0 # g0 = size of the DRP, Datagram
                                        #  Request, and Datagram Response
        shlo    2,r14,r3                # Size needed to send all queued ILT
                                        #  Write Information Record IDs
        addo    4,r3,r3                 # Add the header size (number of IDs)
        addo    r3,g0,g0                # g0 = total size needed
        lda     c$clearWriteInfoComp,r4 # r4 = DRP Completion routine
        st      g0,il_wcdlmreqsize(g1)  # Save the buffer size
c       g0 = s_MallocC(g0, __FILE__, __LINE__); # Get DRP
        st      r4,il_cr(g1)            # Save the completion routine
        st      g0,il_wcdlmdrp(g1)      # Save the DRP buffer address
#
#   Build the DRP to request FE DLM to transfer to the partner the Clear req.
#
        ldconst drcachetodlm,r4         # Cache to DLM DRP
        lda     drpsiz(g0),r5           # r5 = pointer to the datagram request
        stos    r4,dr_func(g0)          # Save the DRP Function (Cache to DLM)
        st      r5,dr_req_address(g0)   # Save the datagram request address
        ldconst dgrq_size,r4            # r4 = length of datagram request hdr
        st      r15,dr_pptr(g0)         # Clear the physical packet pointer
        addo    r3,r4,r6                # r6 = size of datagram request
        ldconst dgrs_size,r8            # r8 = datagram response size
        st      r6,dr_req_length(g0)    # Save the Request size
        addo    r6,r5,r7                # r7 = Datagram response address
        st      r8,dr_rsp_length(g0)    # Save the response size
        st      r15,dr_sglptr(g0)       # Clear the SGL pointer
        st      r7,dr_rsp_address(g0)   # Save the datagram response address
        ldconst WI_ISSUE_CNT,r6         # Set up max issue cnt due to errors
        ldconst WI_TIME_OUT,r7          # Set up max time before aborting op
        stob    r6,dr_issue_cnt(g0)     # Save the issue count
        stob    r7,dr_timeout(g0)       # Save the time out value (in seconds)
#
#   Set up the Datagram Request Header
#
        ld      K_ficb,r11              # r11 = K_ficb pointer
        ldconst dg_cpu_interface,r6     # r6 = Request server CPU as FE
        ldos    C_wi_seqnum,r7          # r7 = Datagram sequence number
        ld      fi_mirrorpartner(r11),r11 # r11 = Mirror Partners Serial Number
        stob    r6,dgrq_srvcpu(r5)      # Save the FE as the Server CPU
        stob    r4,dgrq_hdrlen(r5)      # Save the Header Length
        addo    1,r7,r7                 # Increment the Datagram Sequence Number
        ldconst CAC0_fc_clearwriteinfo,r6 # r6 = Write Memory Function Code
        stos    r7,C_wi_seqnum          # Save the new sequence number
        stos    r7,dgrq_seq(r5)         # Save the sequence number in Request
        stob    r6,dgrq_fc(r5)          # Save the function code
        bswap   r11,r11                 # Change Endian format of Dest Cntrl SN
        ldconst dg_path_any,r8          # r8 = Take any path available
        ldconst CAC0name,r6             # r6 = Show CAC0 is the server name
        st      r3,dgrq_reqlen(r5)      # Save extended request info length
        stob    r8,dgrq_path(r5)        # Any path to the Destination Controller
        st      r6,dgrq_srvname(r5)     # Save the server name (Cache)
        st      r11,dgrq_dstsn(r5)      # Save the Dest. Controller serial num.
#
#   Set up the Request Data (List of Write Information Record IDs)
#
        addo    r4,r5,r6                # r6 = Request Extended Data address
        st      r14,CAC0_rq_cwi_num(r6) # Save the number of Record IDs
        lda     CAC0_rq_cwi_nvaa(r6),r6 # r6 = The first NVA Address
        mov     r10,r8                  # r8 = ILT at the head
.ccwit60:
        ld      il_wcdlmnvaa(r8),r9     # r9 = NVA Address
        ld      il_fthd(r8),r8          # r8 = The next ILT on the list
        st      r9,(r6)                 # Save the Write Info Record ID
        addo    4,r6,r6                 # Point to the next record
        cmpobne 0,r8,.ccwit60           # Jif more ILTs on list
#
# --- Send the data to the partner and update the outstanding DRP counter
#
        ld      C_cwi_cdrp,r8           # r8 = current outstanding DRP counter
        lda     ILTBIAS(g1),g1          # Point to the next level of ILT
        addo    1,r8,r8                 # Bump the outstanding DRP counter
        st      r15,il_fthd(g1)         # Clear the ILT forward pointer
        call    DLM$quedrp              # Queue the DRP to DLM
        st      r8,C_cwi_cdrp           # Save the new outstanding DRP counter
#
# --- All done with these requests, clear out the queue and wait again for
#       more work
#
.ccwit90:
        mov     r10,g1                  # g1 = ILT to free
        ld      il_fthd(r10),r10        # r8 = The next ILT on the list
c       put_wc_plholder(g1);            # Release the placeholder ILT
        cmpobne 0,r10,.ccwit90          # Jif more ILTs to free
        b       .ccwit15                # Go see if more work to do
#
#**********************************************************************
#
#  NAME: C$do_nc_op
#
#  PURPOSE:
#       Continue the process of executing a non-cached Op
#
#  DESCRIPTION:
#       The original operation has passed all previous checking and been
#       determined this Op cannot be serviced by the write cache.  The
#       buffers for the op will be allocated and the op processed.
#
#  CALLING SEQUENCE:
#       call C$do_nc_op
#
#  INPUT:
#       g1 = ILT for the VRP
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
C$do_nc_op:
        mov     g0,r12                  # Save g0
#
# --- Set up and determine which type of Op it is
#
        ld      vrvrp(g1),g0            # g0 = VRP
        mov     g1,r13                  # Save g1
        mov     g2,r14                  # Save g2
        mov     g3,r15                  # Save g3
        ld      vr_vlen(g0),r7          # Get the length of the op
        ldos    vr_func(g0),r6          # Get the function code
        mov     g4,r11                  # Save g4
        shlo    9,r7,g2                 # g2 = byte count
        bbc     0,r6,.donc40            # Jif not a read
#
# --- Process read operation ------------------------------------------
#
# --- Allocate local read buffer and ILT/SRP
#
        call    c$rdchk                 # Stall if insufficient resources
        call    c$alrbuf                # Allocate read buffer
c       record_cache(FR_CACHE_NC_READ, (void *)g0);
        mov     src2h,g4                # Show that this is a Read Type Op
        ld      vr_sglptr(g0),r4        # Get the SGL Pointer
c       if (r4 == 0xfeedf00d) {
c           fprintf(stderr,"%s%s:%u C$do_nc_op -1 sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__);
c           abort();
c       }
        ld      sg_desc0+sg_addr(r4),g2 # Get the SGL Buffer Address
        ld      sg_desc0+sg_len(r4),g3  # Get the SGL Buffer Length
        mov     g2,r5                   # Save local Buffer Address
        call    c$asrp                  # Allocate read ILT/SRP using
                                        #  Local SGL Address
#
# --- Transform the SGL Address from Local Buffer to Remote Buffer for BE960
#
        st      r5,sg_desc0+sg_addr(r4) # Save the address in SGL
#
# --- Initiate read from virtual disk to local buffer
#
        ld      vr_use0(g0),r4          # Get the parent VRP
        cmpobe  0,r4,.donc10            # Jif g0 is parent VRP
c       g2 = CA_GetReadComp();
        b       .donc20
.donc10:
        lda     c$ncrcomp1,g2           # Get completion routine
.donc20:
        call    c$calllower
        b       .donc100
#
# --- Process output operation ----------------------------------------
#
.donc40:
#
        cmpobe  vrverifyc,r6,.donc50    # Jif verify checkword
        cmpobe  vrsync,r6,.donc50       # Jif sync cache command
        ld      vr_sglptr(g0),r4        # Get the SGL Pointer
c       if (r4 == 0xfeedf00d) {
c           fprintf(stderr,"%s%s:%u C$do_nc_op -2 sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__);
c           abort();
c       }
        cmpobe  0,r4,.donc70            # Jif there is no SGL Pointer
        ldob    sg_flag(r4),r4          # Get flag byte
        bbc.f   sg_buffer_alloc,r4,.donc70 # Jif a buffer is not already
                                        #       allocated
#
# --- The buffer has already been allocated and the data is in the buffer or
#       this is a Verify Checkword or Sync Cache request (No buffers needed!)
#
.donc50:
#
# --- Check for a synchronize cache request
#
        cmpobne vrsync,r6,.donc60       # Jif not synchronize cache request
.if 0 #CSTOP_SAN1171,1416,1670
        ldob    vr_options(g0),r6
        clrbit  vrnotcompleted,r6,r6    # Indicate VRP is completed
        stob    r6,vr_options(g0)       # Update vrp options
.endif
        ldconst ecok,r5                 # Set the Completion code to OK
        stob    r5,vr_status(g0)
        ldconst ecok,g0                 # Make g0 show the sync completed OK
        call    K$comp                  # Complete this request
        ld      C_orc,r5                # Load, Decrement, and Store the
        subo    1,r5,r5                 #  outstanding request count
.ifdef M4_DEBUG_C_orc
c CT_history_printf("%s%s:%u: C_orc starts at %lu, ends at %lu\n", FEBEMESSAGE,__FILE__, __LINE__, C_orc, r5);
.endif  # M4_DEBUG_C_orc
        st      r5,C_orc
        b       .donc100
#
# --- Queue request
#
.donc60:
c       record_cache(FR_CACHE_NC_WRITE_SGL, (void *)g0);
        lda     c$ndopcomp,g2           # Decrement the C_orc and complete op
        call    c$calllower             # Queue request
        b       .donc100
#
# --- Process a Write operation without the buffer already allocated
#
#
# --- Allocate buffers and ILT/SRP
#
.donc70:
        call    c$rdchk                 # Stall if insufficient resources
        call    c$alrbuf                # Allocate loc SGL & write buffer
        mov     srh2c,g4                # Show this SRP is a Write Type Op
        ld      vr_sglptr(g0),r3        # Get the SGL Pointer
        ld      sg_desc0+sg_addr(r3),g2 # Get the SGL Buffer Address
        ld      sg_desc0+sg_len(r3),g3  # Get the SGL Buffer Length
        mov     g2,r4                   # Save local Buffer Address
        call    c$asrp                  # Allocate write ILT/SRP
        st      r4,sg_desc0+sg_addr(r3)
c       record_cache(FR_CACHE_NC_WRITE_PROXY, (void *)g0);
#
# --- Initiate data transfer from host to BE960 write buffer
#
        ld      il_w3(g1),g1            # Get ILT/SRP
        lda     c$ncwcomp1,g2           # Get completion routine
        call    c$callupper
#
# --- Adjust outstanding write SRP count
#
        ld      C_owsrpc,g0             # Load, Increment, and Store the
        addo    1,g0,g0                 #  outstanding write SRP count
        st      g0,C_owsrpc
#
# All done
#
.donc100:
        mov     r11,g4                  # Restore g4
        mov     r12,g0                  # Restore g0-g3
        mov     r13,g1
        mov     r14,g2
        mov     r15,g3
        ret
#
#**********************************************************************
#
#  NAME: c$rdchk
#
#  PURPOSE:
#       To provide a common means of regulating incoming read activity
#       by examination of current cached and non-cached DRAM resources.
#
#  DESCRIPTION:
#       A check is made to determine if sufficient cached DRAM resources
#       are available.  If not, this process is placed into cached DRAM
#       wait.
#
#       An additional check is made to determine if sufficient non-cached
#       DRAM resources exits.  If not, this process is placed into non-cached
#       DRAM wait.
#
#  CALLING SEQUENCE:
#       call    c$rdchk
#
#  INPUT:
#       g2 = byte count
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
c$rdchk:
#
# --- Check for sufficient non-cached DRAM. If there are fewer than 6
# --- outstanding requests, then just check the MINNCDRAM limit. If
# --- 6 or more requests, check both the MINNCDRAM and MINCDRAM limits.
#
.ifndef REALLY_DELAY_MEMORY_FREES
        ld      C_orc,r5                # Get outstanding request count
.ifdef M4_DEBUG_C_orc
c CT_history_printf("%s%s:%u: C_orc (%lu) check >5\n", FEBEMESSAGE,__FILE__, __LINE__, C_orc);
.endif  # M4_DEBUG_C_orc
        cmpobg  5,r5,.cr10              # Jif limited outstanding requests
        ldconst (MINCDRAM/4)*3,r3       # Get minimum Cacheable DRAM req'd
        b       .cr20
#
.cr10:
        ldconst 0,r3                    # No MINCDRAM limit
.cr20:
        ld      K_ii+ii_nccur,r4        # Get current size of non-cached DRAM
        lda     MINNCDRAM(g2),r5        # Get non-cached memory required
        addo    r5,r3,r3                # Calculate total limit
        cmpobge r4,r3,.cr100            # Jif sufficient
#
# --- Place in non-cached DRAM wait
#
        lda     K_ii+ii_nccur,r4        # Get non-cached DRAM FMS
        mov     pcncdram,r15            # Get non-cached DRAM wait status
        balx    c$memwait,r7            # Set memory wait
        b       c$rdchk
#
# --- Exit
#
.cr100:
.endif  # !REALLY_DELAY_MEMORY_FREES
        ret
#
#**********************************************************************
#
#  NAME: c$drpmemchk
#
#  PURPOSE:
#       To provide a common means of checking for correct Datagram memory
#       locations to read or write.
#
#  DESCRIPTION:
#       Based on the input mask, the memory areas are verified that no incorrect
#       memory section can be accessed.  The Datagram addresses are then
#       verified to be in the stated memory areas.
#
#  CALLING SEQUENCE:
#       call    c$drpmemchk
#
#  INPUT:
#       g0 = Mask of allowed memory regions
#       g1 = Memory regions specified
#       g2 = Datagram Request Address
#
#  OUTPUT:
#       g0 = 0 - All memory addresses being touched are allowed
#                g1 = corrupted
#          = 1 - Regions asked to be accessed that are not allowed in mask
#                g1 = Regions specified that are not allowed
#          = 2 - Address not in specified regions
#                g1 = Entry number of the address/length that failed the test
#
#  REGS DESTROYED:
#       g0 and g1
#
#**********************************************************************
#
c$drpmemchk:
#
# --- Check for correctly requested memory regions for the request
#
        mov     g1,r15                  # r15 = Bit mask of valid regions
        not     g0,r8                   # Invert the Mask of allowed
        and     r8,g1,r9                # Determine if any un-allowed bits on
        cmpobe  0,r9,.cdrpmemchk10      # Jif all is well
        mov     1,g0                    # Report the allowed region failure
        mov     r9,g1                   # What invalid regions were allowed
        b       .cdrpmemchk1000         # Return with the error reported
#
# --- Check the addresses against the memory regions
#
.cdrpmemchk10:
        ldob    dgrq_hdrlen(g2),r8      # Get the header length
        mov     0,g1                    # Show the first entry as being checked
        addo    r8,g2,r3                # r3 = Extended Request Information ptr
        ld      dgrq_reqlen(g2),r9      # Get the Extended Req Info length
        divo    CAC0_Entry_len,r9,r4    # r4 = number of entries
.cdrpmemchk100:
        mov     2,g0                    # Show the memory address checks bad
        ld      CAC0_rq_mem_addr(r3),r6 # r6 = Beginning Memory Address
        ld      CAC0_rq_mem_len(r3),r7  # r7 = Length of transfer
        addo    r6,r7,r7                # r7 = End address + 1
#
        bbc     CAC0_WCCT,r15,.cdrpmemchk200 # Jif not Write Cache Control Table
        ld      WcctAddr,r8             # r8 = Beginning of region
        ld      WcctSize,r10            # r10 = length of region
        addo    r8,r10,r9               # r9 = End of region + 1
        cmpobl  r6,r8,.cdrpmemchk200    # Jif Memory Address before this region
        cmpobg  r7,r9,.cdrpmemchk200    # Jif Memory Address after this region
        mov     0,g0                    # Show Memory Location is Good
        b       .cdrpmemchk900          # Check the next entry if there is one
#
.cdrpmemchk200:
        bbc     CAC0_WCC,r15,.cdrpmemchk400 # Jif not Write Cache Configuration
        ld      WccAddr,r8              # r8 = Beginning of region
        ld      WccSize,r10             # r10 = length of region
        addo    r8,r10,r9               # r9 = End of region + 1
        cmpobl  r6,r8,.cdrpmemchk400    # Jif Memory Address before this region
        cmpobg  r7,r9,.cdrpmemchk400    # Jif Memory Address after this region
        mov     0,g0                    # Show Memory Location is Good
        b       .cdrpmemchk900          # Check the next entry if there is one
#
.cdrpmemchk400:
        bbc     CAC0_TAG,r15,.cdrpmemchk600 # Jif not Write Cache Tag region
        ld      WctAddr,r8              # r8 = Beginning of region
        ld      WctSize,r10             # r10 = length of region
        addo    r8,r10,r9               # r9 = End of region + 1
        cmpobl  r6,r8,.cdrpmemchk600    # Jif Memory Address before this region
        cmpobg  r7,r9,.cdrpmemchk600    # Jif Memory Address after this region
        mov     0,g0                    # Show Memory Location is Good
        b       .cdrpmemchk900          # Check the next entry if there is one
#
.cdrpmemchk600:
        bbc     CAC0_DATA,r15,.cdrpmemchk900 # Jif not Write Cache Data region
        ld      WcbAddr,r8              # r8 = Beginning of region
        ld      WcbSize,r10             # r10 = length of region
        addo    r8,r10,r9               # r9 = End of region + 1
        cmpobl  r6,r8,.cdrpmemchk900    # Jif Memory Address before this region
        cmpobg  r7,r9,.cdrpmemchk900    # Jif Memory Address after this region
        mov     0,g0                    # Show Memory Location is Good
#
.cdrpmemchk900:
        cmpobne 0,g0,.cdrpmemchk1000    # Jif the memory location is not valid
        addo    1,g1,g1                 # Increment the entry number to check
        addo    CAC0_Entry_len,r3,r3    # Point to the next entry
        cmpobl  g1,r4,.cdrpmemchk100    # Jif more entries to check
#
# --- Exit
#
.cdrpmemchk1000:
        ret
#
#**********************************************************************
#
#  NAME: c$memwait
#
#  PURPOSE:
#       To provide a common means of placing a process into the
#       specified memory wait status.
#
#  DESCRIPTION:
#       The memory wait count is incremented within the FMS and the
#       executing process is set to the specified memory wait.  Control
#       is relinquished until the memory wait condition is cleared.  At
#       that time this routine will exit back to the caller.
#
#  CALLING SEQUENCE:
#       balx    c$memwait,r7
#
#  INPUT:
#       r4  = FMS
#       r15 = memory wait status
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       r3
#
#**********************************************************************
#
.ifndef REALLY_DELAY_MEMORY_FREES
c$memwait:
#
# --- Bump memory wait count
#
        ld      fs_wait(r4),r3          # Bump wait count
        addo    1,r3,r3
        st      r3,fs_wait(r4)
#
# --- Place process in memory wait
#
c       TaskSetMyState(r15);            # memory wait status
        call    K$xchang                # Give up control
#
# --- Exit
#
        bx      (r7)
.endif  # !REALLY_DELAY_MEMORY_FREES
#
#**********************************************************************
#
#  NAME: c$ncrcomp1
#
#  PURPOSE:
#       To provide a means of handling the completion of a non-cached
#       read operation from a virtual disk to the read buffer.
#
#  DESCRIPTION:
#       The data transfer operation from the local read buffer back to the
#       host is queued without wait.  A completion routine is specified
#       to handle the completion of that data transfer.
#
#  CALLING SEQUENCE:
#       call    c$ncrcomp1
#
#  INPUT:
#       g0 = VRP Completion Status
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
# void C_ncrComp1(UINT32 status, ILT *pILT);
        .globl  C_ncrComp1
C_ncrComp1:
c$ncrcomp1:
#
# --- Initiate data transfer from local buffer to host
#
        mov     g0,r12                  # Save g0
        mov     g1,r13                  # Save g1
        mov     g2,r14                  # Save g2
        ld      il_w3(g1),g1            # g1 = ILT of SRP
.ifdef FLIGHTRECORDER
        ld      vrvrp(r13),r4           # r4 = VRP
c       record_cache(FR_CACHE_NC_READ_COMPLETE, (void *)r4);
.endif  # FLIGHTRECORDER
#
.if     DEBUG_FLIGHTREC_C
        ld      vrvrp(r13),r4           # r4 = VRP
        ldconst frt_c_ncr1,r3           # Cache - c$ncrcomp1
        ldos    vr_func(r4),r5          # r5 = VRP Function
        ldos    vr_vid(r4),r6           # r6 = VID
        shlo    24,r12,r7               # Set up to have several values in parm0
        shlo    8,r5,r5
        shlo    16,r6,r6
        or      r5,r3,r3                # r3 = Function, Flight Recorder ID
        or      r7,r3,r3                # r3 = VRP Status, Function, FRID
        or      r6,r3,r3                # r3 = VRP Status, VID, Function, FRID
        st      r13,fr_parm1            # ILT
        st      r4,fr_parm2             # VRP
        st      g1,fr_parm3             # ILT of the SRP
        st      r3,fr_parm0             # VRP Status, VID, Function, FRID
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_C
        lda     c$ncrcomp2,g2           # g2 = completion routine
#
# --- Call the layer above Cache to handle the SRP
#
        call    c$callupper
#
# --- Adjust outstanding read SRP count
#
        ld      C_orsrpc,r3             # Load, Increment, and Store the
        addo    1,r3,r3                 #  outstanding read SRP count
        st      r3,C_orsrpc
#
# --- Exit
#
        mov     r12,g0                  # Restore g0-g2
        mov     r13,g1
        mov     r14,g2
        ret
#
#**********************************************************************
#
#  NAME: c$ncrcomp2
#
#  PURPOSE:
#       To provide a means of handling the completion of a read data
#       transfer from the read buffer to the host.
#
#  DESCRIPTION:
#       The ILT/SRP combination is released back to the system.  The
#       local SGL and read buffer are released.  The parent ILT is
#       completed back to the originator.
#
#  CALLING SEQUENCE:
#       call    c$ncrcomp2
#
#  INPUT:
#       g1 = ILT for the read SRP
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
c$ncrcomp2:
        mov     g0,r14                  # Save g0-g1
        mov     g1,r15
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
# --- Release read buffer
#
        ld      il_w6-ILTBIAS(r13),g0   # Get SGL
        ld      sg_desc0+sg_addr(g0),r4 # Get the BE960 address
        mov     r13,g1                  # g1 = parent ILT
        st      r4,sg_desc0+sg_addr(g0) # Save the FE960 address to be released
        call    c$rlrbuf                # Release the read buffer
        ld      vrvrp(r13),r5           # Get the VRP
        ldob    vr_status(r5),g0        # Get the VRP completion status code
#
.if     DEBUG_FLIGHTREC_C
        ldconst frt_c_ncr2,r3           # Cache - c$ncrcomp2
        ldos    vr_func(r5),r8          # r5 = VRP Function
        ldos    vr_vid(r5),r9           # r9 = VID
        shlo    8,r8,r8                 # Set up to have several values in parm0
        shlo    16,r9,r9
        or      r8,r3,r3                # r3 = Function, Flight Recorder ID
        or      r9,r3,r3                # r3 = VID, Function, Flight Recorder ID
        st      r3,fr_parm0             # VID, Function, Flight Recorder ID
        st      r13,fr_parm1            # ILT
        st      r5,fr_parm2             # VRP
        st      r15,fr_parm3            # ILT of the SRP
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_C
#
.if 0 #CSTOP_SAN1171,1416,1670
        ldob    vr_options(r5),r6       # Get vrp options
        clrbit  vrnotcompleted,r6,r6    # Indicate VRP is completed
        stob    r6,vr_options(r5)       # Update vrp options
.endif
#
# --- Complete request
#
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
        mov     r14,g0                  # Restore g0-g1
        mov     r15,g1
        ret
#
#**********************************************************************
#
#  NAME: c$ncwcomp1
#
#  PURPOSE:
#       To provide a means of handling the completion of a write data
#       transfer (SRP) from the host to the write buffer.
#
#  DESCRIPTION:
#       The data transfer operation from the local write buffer to a
#       non-cacheable virtual disk is queued without wait.  Another
#       completion routine is specified to handle the completion of
#       the data transfer.
#
#       The parent ILT is removed from the pending write SRP queue
#       and any dependent ILT requests queued to that parent are then
#       queued to the designated routines.
#
#  CALLING SEQUENCE:
#       call    c$ncwcomp1
#
#  INPUT:
#       g1 = ILT (SRP)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
c$ncwcomp1:
#
# --- Get SRP completion status
#
        ld      vrsrp(g1),r3            # Get SRP
        mov     g0,r12                  # Save g0-g2
        mov     g1,r13
        mov     g2,r14
        ldob    sr_status(r3),r15       # Get SRP status
        ld      il_w3(g1),g1            # Get parent ILT
.ifdef FLIGHTRECORDER
        ld      vrvrp(g1),r4           # r4 = VRP
c       record_cache(FR_CACHE_NC_WRITE_PROXY_DATA, (void *)r4);
.endif  # FLIGHTRECORDER
#
.if     DEBUG_FLIGHTREC_C
        ldconst frt_c_ncw1,r3           # Cache - c$ncwcomp1
        ld      vrvrp(g1),r4            # r4 = VRP
        ldos    vr_func(r4),r5          # r5 = VRP Function
        ldos    vr_vid(r4),r6           # r6 = VID
        shlo    8,r5,r5                 # Set up to have several values in parm0
        shlo    24,r15,r7
        shlo    16,r6,r6
        or      r5,r3,r3                # r3 = Function, Flight Recorder ID
        or      r7,r3,r3                # r3 = SRP Status, Function, FRID
        or      r6,r3,r3                # r3 = SRP Status, VID, Function, FRID
        st      r3,fr_parm0             # SRP Status, VID, Function, FRID
        st      g1,fr_parm1             # ILT
        st      r4,fr_parm2             # VRP
        st      r13,fr_parm3            # ILT of the SRP
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_C
#
        cmpobne srok,r15,.cw25          # Jif SRP status is not OK
        lda     c$ncwcomp2,g2           # Pass completion routine
#
# --- Call the next lower layer to pass on the request
#
        call    c$calllower
        b       .cw110
#
# --- Return write request w/ error
#
.cw25:
        mov     r15,g0                  # g0 = VRP completion status
        call    c$ncwcomp2              # Complete write request
#
# --- All done with this SRP, decrement the outstanding Write SRP Count
#
.cw110:
        ld      C_owsrpc,r3             # Load, Decrement, and Store the
        subo    1,r3,r3                 #  outstanding write SRP count
        st      r3,C_owsrpc
#
# --- Release write ILT/SRP and return
#
        mov     r12,g0                  # Restore g0-g2
        mov     r13,g1
        mov     r14,g2
        b       c$rsrp                  # Release write ILT/SRP and return
#
#**********************************************************************
#
#  NAME: c$ncwcomp2
#
#  PURPOSE:
#       To provide a means of handling the completion of a write data
#       transfer from the write buffer to a non-cacheable virtual disk.
#
#  DESCRIPTION:
#       The local proxy write buffer is released back to the system.
#       The parent ILT is then completed back to the caller.
#
#  CALLING SEQUENCE:
#       call    c$ncwcomp2
#
#  INPUT:
#       g0 = VRP Completion Status
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
c$ncwcomp2:
        mov     g0,r12                  # Save g0
        mov     g1,r13                  # Save g1
.ifdef FLIGHTRECORDER
        ld      vrvrp(g1),r14           # r14 = VRP
c       record_cache(FR_CACHE_NC_WRITE_PROXY_COMPLETE, (void *)r14);
.endif  # FLIGHTRECORDER
#
# --- Release write buffer proxy
#
        ld      il_w6-ILTBIAS(g1),g0    # Get local proxy
        mov     g2,r14                  # Save g2
#
.if     DEBUG_FLIGHTREC_C
        ld      vrvrp(r13),r5           # Get the VRP
        ldconst frt_c_ncw2,r3           # Cache - c$ncwcomp2
        ldos    vr_func(r5),r4          # r4 = VRP Function
        ldos    vr_vid(r5),r6           # r6 = VID
        shlo    24,r12,r7               # Set up to have several values in parm0
        shlo    8,r4,r4
        shlo    16,r6,r6
        or      r4,r3,r3                # r3 = Function, Flight Recorder ID
        or      r7,r3,r3                # r3 = VRP Status, Function, FRID
        or      r6,r3,r3                # r3 = VRP Status, VID, Function, FRID
        st      r13,fr_parm1            # ILT
        st      r5,fr_parm2             # VRP
        st      g0,fr_parm3             # Proxy Write Buffer Address
        st      r3,fr_parm0             # VRP Status, VID, Function, FRID
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_C
#
.if 0 #CSTOP_SAN1171,1416,1670
        ldob    vr_options(r13),r6      # Get vrp options
        clrbit  vrnotcompleted,r6,r6    # Indicate VRP is completed
        stob    r6,vr_options(r13)      # Update vrp options
.endif
        call    c$rlrbuf                # Release the Write buffer
        mov     r12,g0                  # Restore the VRP Completion Status
#
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
        mov     r12,g0                  # Restore g0-g2
        mov     r13,g1
        mov     r14,g2
        ret
#
.if WC_ENABLE
#**********************************************************************
#
#  NAME: c$drpsrpcomp
#
#  PURPOSE:
#       To provide a means of handling the completion of a DRP transfer of
#       data to the BE memory and copying the data to the NV Card.
#
#  DESCRIPTION:
#       The ILT and SRP are freed and the SRP status is verified good.
#       If it is not good, the Datagram Response status will be set to
#       the appropriate failure with the SRP status sent back as part of the
#       Response data.  If it is good, the data will then be copied to the
#       NV Card.
#
#  CALLING SEQUENCE:
#       call    c$drpsrpcomp
#
#  INPUT:
#       g0 = ILT Return Status
#       g1 = ILT of the SRP transferring the data
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
c$drpsrpcomp:
#
# Get SRP completion status
#
        ld      vrsrp(g1),r5            # Get SRP
        ldob    sr_status(r5),r15       # Get SRP status
        ld      il_w3(g1),r4            # r4 = parent ILT (DRP)
#
.if     DEBUG_FLIGHTREC_C
        ldconst frt_c_drpcomp,r3        # Cache - c$drpcomp
        ldconst 0x0100,r10              # Show this is an DRPSRP Completion
        or      r10,r3,r3               # r3 = SRP Comp, Flight Recorder ID
        shlo    16,r15,r10              # Set up to save the SRP status
        or      r10,r3,r3               # r3 = SRP Status, SRP Comp, Flight ID
        shlo    24,g0,r10               # Set up to save the ILT status
        or      r10,r3,r3               # r3 = ILT St, SRP St, SRP Comp, Fl ID
        st      r3,fr_parm0             # ILT St, SRP St, SRP Comp, Flight ID
        st      g1,fr_parm1             # ILT(SRP)
        st      r5,fr_parm2             # SRP
        st      r4,fr_parm3             # ILT(DRP)
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_C
#
# Return from the ILT and SRP
#
        call    c$rsrp
#
# Check the SRP for good/bad completion and set up the response if bad
#
        mov     r4,g1                   # Set up the DRP ILT
        ld      vrvrp(r4),r14           # r14 = DRP address
        ld      dr_rsp_address(r14),r13 # r13 = Datagram Response Address
        mov     0,r3                    # Preset status as good
        cmpobe  srok,r15,.cdrpcomp50    # Jif all went good with SRP
        ldconst CAC0_DATA_XFER_FAILED_SRP,r5 # Show the Data transfer failed SRP
        stob    r15,dgrs_ec2(r13)       # Save the SRP status in EC #2
        b       .cdrpcomp60             # Log the rest of the status
#
.cdrpcomp50:
        cmpobe  ecok,g0,.cdrpcomp200    # Jif ILT status is good
        ldconst CAC0_DATA_XFER_FAILED_ILT,r5 # Show the Data transfer failed ILT
        st      g0,dgrs_ec2(r13)        # Save the ILT Status in g0
.cdrpcomp60:
        ldconst dg_st_srvr,r3           # Show in the response the failure
        stob    r5,dgrs_ec1(r13)        # Save the EC #1 Failure code
        mov     0,r6                    # Show the DRP Status as processed
        stob    r3,dgrs_status(r13)     # Save the Datagram Status
        stob    r6,dr_status(r14)       # Set the DRP Status
        b       K$comp                  # Complete the DRP
#
# The Data was transfered correctly, now copy the data into the NV Card
#
.cdrpcomp200:
        PushRegs(r3)                    # Save all the 'G' registers
        mov     g1,g0                   # Set up the ILT parameter
        mov     r14,g1                  # Set up the DRP parameter
        call    WC_MirrorBE             # Go set up the DMA and return
        PopRegsVoid(r3)
        ret
.endif   # WC_ENABLE
#
#**********************************************************************
#
#  NAME: c$srpcomp
#
#  PURPOSE:
#       To provide a means of handling the srp completion
#
#  DESCRIPTION:
#       Stores the completion code in the SRP and calls the next layer
#
#  CALLING SEQUENCE:
#       call    c$srpcomp
#
#  INPUT:
#       g0 = SRP status
#       g1 = ILT of SRP
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
c$srpcomp:
        ld      otl1_srp(g1),r4         # r4 = SRP
        stob    g0,sr_status(r4)        # Save the SRP status
.if     DEBUG_FLIGHTREC_C
        b       c$nlevelcomp_otl1_srp   # Log the event and complete the request
.else   # DEBUG_FLIGHTREC_C
        b       K$comp                  # Complete the request
.endif  # DEBUG_FLIGHTREC_C
#
#**********************************************************************
#
#  NAME: c$ndopcomp
#
#  PURPOSE:
#       To provide a means of handling a non-data operation completion
#
#  DESCRIPTION:
#       Logs the completion of the op (if in debug mode), completes the op to
#       the next layer, and then decrements the Cache outstanding Op counter.
#
#  CALLING SEQUENCE:
#       call    c$ndopcomp
#
#  INPUT:
#       g0 = VRP Completion Status
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
c$ndopcomp:
        ld      vrvrp(g1),r4            # r4 = VRP
.ifdef FLIGHTRECORDER
c       record_cache(FR_CACHE_NC_WRITE_SGL_COMPLETE, (void *)r4);
.endif  # FLIGHTRECORDER
#
.if 0 #CSTOP_SAN1171,1416,1670
        ldob    vr_options(r4),r6       # Get VRP options
        clrbit  vrnotcompleted,r6,r6    # Indicate VRP is completed
        stob    r6,vr_options(r4)       # Update vrp options
.endif
        movl    g0,r4                   # Save g0-g1
#
.if     DEBUG_FLIGHTREC_C
        call    c$nlevelcomp_vrvrp      # Log the event and complete the request
.else   # DEBUG_FLIGHTREC_C
        call    K$comp                  # Complete the request
.endif  # DEBUG_FLIGHTREC_C
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
        movl    r4,g0                   # Restore g0-g1
        ret
#
.if     DEBUG_FLIGHTREC_C
#**********************************************************************
#  NAME: c$nlevelcomp_vrvrp
#  NAME: c$nlevelcomp_otl1_srp
#
#  PURPOSE:
#       To log flight recorder, and then complete request.
#
#  DESCRIPTION:
#       The next layer of completion is called.
#
#  CALLING SEQUENCE:
#       call    c$nlevelcomp_vrvrp
#       b       c$nlevelcomp_otl1_srp
#
#  INPUT:
#       g0 = VRP/SRP status
#       g1 = ILT of VRP or SRP
#       r4 = VRP or SRP for c$nlevelcomp_otl1_srp only.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
c$nlevelcomp_vrvrp:
        ld      vrvrp(g1),r4            # r4 = VRP
#        b       c$nlevel_comp_common                INTENTIONALLY COMMENTED OUT
# NOTE: fall through!
#**********************************************************************
c$nlevelcomp_otl1_srp:
# NOTE: r4 already gotten before it branches here.
#       ld      otl1_srp(g1),r4         # r4 = SRP  INTENTIONALLY COMMENTED OUT
#       b       c$nlevel_comp_common                INTENTIONALLY COMMENTED OUT
# NOTE: fall through!
#**********************************************************************
# c$nlevelcomp_common:                              INTENTIONALLY COMMENTED OUT
        movq    g0,r12                  # Save g0-g3
        ldconst frt_c_nlc,r3            # Cache - c$nlevelcomp
        st      r3,fr_parm0             # Flight Recorder ID
        st      r13,fr_parm1            # ILT
        st      r4,fr_parm2             # VRP/SRP
        st      g0,fr_parm3             # Status
        call    M$flight_rec            # Record it
        call    K$comp                  # Complete this request
        movq    r12,g0                  # Restore g0-g3
        ret
.endif  # DEBUG_FLIGHTREC_C
#
#**********************************************************************
#
#  NAME: c$alrbuf
#
#  PURPOSE:
#       To provide a common means of assigning a read buffer to a VRP/SGL
#       combination.
#
#  DESCRIPTION:
#       A data buffer is assigned from Local DRAM and linked to the VRP/SGL.
#       This routine will block if memory resources are not available.
#
#       This routine can only be called from the process level.
#
#  CALLING SEQUENCE:
#       call    c$alrbuf                # read buffer
#
#  INPUT:
#       g0 = VRP
#       g1 = ILT
#       g2 = byte count (multiple of BGRAN+1)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
c$alrbuf:
        mov     g0,r12                  # Save VRP, ILT, and Byte Count
        mov     g1,r13
        mov     g2,r14
#
# --- Allocate read buffer from DRAM
#
        ld      C_Freehead,r5           # Get head of list
        mov     g2,g0                   # Set up call to allocate buffer
        ldconst CFREETHR,r4             # Get max size of prealloc buffer
        ldconst CFREESIZ,r8             # Get alloc size
        cmpobg  g2,r4,.calr30           # Jif too big
#
# --- Try to use buffer from freelist if available, if not maybe alloc one
#
        cmpobe  0,r5,.calr10            # Jif one not available
#
# --- Buffer available, get off freelist.
#
        ld      (r5),r6                 # Get next item from list
        mov     r5,g0                   # g0 = buffer to allocate
        st      r6,C_Freehead           # And unlink it
        b       .calr60                 # Done with allocate
#
# --- If max not allocated now, allocate another buffer of CFREEMAX size
#
.calr10:
        ld      C_Freecnt,r6            # Get alloc counter
        ldconst CFREEMAX,r7             # Get max alloc count
        cmpobge r6,r7,.calr30           # Jif max allocated
#
        addo    1,r6,r6                 # Increment alloc counter
        setbit  31,r8,g0                # Set permanent alloc
        st      r6,C_Freecnt            # Save the new allocated counter
        b       .calr50
#
.calr30:
        addo    16,g0,g0                # Adjust size
        cmpobne g0,r8,.calr40           # Jif not equal to prealloc size
        addo    16,g0,g0                # Adjust it if necessary
#
.calr40:
        mov     g0,r8                   # Save size of buffer
.calr50:
c       g0 = s_MallocW(g0, __FILE__, __LINE__); # Allocate the read buffer
#
.calr60:
        st      r8,(g0)                 # Store alloc size at beginning of buffer
        addo    16,g0,g0                # Adjust buffer beginning point
#
# --- Initialize the SGL in the VRP
#
        ldconst 1,r4                    # Set up descriptor count
        mov     g0,r6                   # Copy the buffer address
        ldconst sghdrsiz+sgdescsiz,r5   # Set up SGL size
        mov     r14,r7                  # Copy the buffer length
        st      r5,vr_sglsize(r12)      # Save the SGL size in the VRP
        lda     vr_sglhdr(r12),r8       # Get the SGL pointer
        stq     r4,vr_sglhdr(r12)       # Store the SGL in the VRP
        st      r8,vr_sglptr(r12)       # Save the SGL @ in the VRP
#
# --- Save the SGL
#
        st      r8,il_w6-ILTBIAS(r13)   # Save SGL in the caller's ILT
#
# --- Exit
#
        mov     r12,g0                  # Restore g0-g2
        mov     r13,g1
        mov     r14,g2
        ret
#
#**********************************************************************
#
#  NAME: c$rlrbuf
#
#  PURPOSE:
#       To provide a common means of releasing a read buffer associated with
#       a VRP.
#
#  DESCRIPTION:
#       The read buffer is released back to the local DRAM.
#
#       This routine may be called from either the process or interrupt
#       level.
#
#  CALLING SEQUENCE:
#       call    c$rlrbuf                # read buffer
#
#  INPUT:
#       g0 = SGL
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
c$rlrbuf:
        mov     g0,r14                  # Save g0-g1
        mov     g1,r15
#
# --- Release the data buffer
#
        ld      sg_desc0+sg_addr(g0),g0 # Get the address
        ld      C_Freehead,r3           # Get head of list
        subo    16,g0,g0                # Adjust back to beginning of buffer
        ld      (g0),g1                 # Get actual size of buffer
        ldconst CFREESIZ,r4             # Get prealloc buffer size
        cmpobne g1,r4,.crlr10           # Jif not preallocated
#
# --- Preallocated; return to buffer free pool
#
        st      r3,(g0)                 # link buffer to list
        st      g0,C_Freehead           #   and store head pointer
        b       .crlr20                 # Done!
#
.crlr10:
c       s_Free(g0, g1, __FILE__, __LINE__); # Free the read buffer
#
# --- Exit
#
.crlr20:
        mov     r14,g0                  # Restore g0-g1
        mov     r15,g1
        ret
#
#**********************************************************************
#
#  NAME: c$asrp
#
#  PURPOSE:
#       To provide a common means of assigning an ILT and SRP for
#       supporting a host read/write operation.
#
#  DESCRIPTION:
#       An ILT and SRP are dynamically assigned from the system with
#       wait.  The SRP is initialized and the S/G descriptors are
#       created from the input parameters.
#
#  CALLING SEQUENCE:
#       call    c$asrp
#
#  INPUT:
#       g1 = parent ILT
#       g2 = Buffer Address
#       g3 = Buffer Length (in bytes)
#       g4 = SRP Function
#
#  OUTPUT:
#       Parent ILT
#           il_w3 = ILT/SRP
#
#       SRP ILT
#           il_w3 = ILT/VRP
#           il_w4 = SRP
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
c$asrp:
        mov     g0,r12                  # Save g0-g3
        mov     g1,r13
        mov     g2,r14
        mov     g3,r15
        mov     g4,r4                   # Set up the SRP function
#
# --- Assign new ILT
#
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
#
# --- Assign SRP from non-cached DRAM
#
        ld      C_SFreehead,g0          # Get freelist head ptr
        ld      C_SrpCnt,r8             # r8 = Number of SRPs in permanent store
        ldconst (srpbsiz+srpesiz),r5    # Calculate the SRP length
        mov     g1,r7                   # Preserve new ILT address
        cmpobne 0,g0,.casrp10           # Jif one available on freelist
        ldconst CSRPMAX,r3              # r3 = Maximum allowed in perm storage
        cmpoble r3,r8,.casrp20          # Jif the limit was reached
        addo    1,r8,r8                 # Increment the number of SRPs in perm
        setbit  sr_perm_store,0,r6      # Set the Permanent Store Flag bit
        shlo    sr_flag_offset,r6,r6    # Set up the correct byte in word
        or      r6,r4,r4                # Turn on the Perm Store
        st      r8,C_SrpCnt             # Save the number allocated so far
        setbit  31,r5,g0                # Allocate Permanent Storage for SRP
        b       .casrp30
#
# --- Use SRP on freelist instead of allocating one
#
.casrp10:
        ld      (g0),r3                 # Unlink from list
        setbit  sr_perm_store,0,r6      # Set the Permanent Store Flag bit
        shlo    sr_flag_offset,r6,r6    # Set up the correct byte in word
        or      r6,r4,r4                # Turn on the Perm Store
        st      r3,C_SFreehead          # Store new freelist head ptr
        b        .casrp60
#
# --- Allocate a SRP from cacheable DRAM
#
.casrp20:
        mov     r5,g0                   # Set up for the call to get the memory
.casrp30:
c       g0 = s_MallocW(g0, __FILE__, __LINE__); # Assign memory
#
# --- Initialize SRP
#
.casrp60:
        ldconst 1,r6                    # Build SRP count
        mov     0,r8                    # Need to Clear out unused part of SRP
        mov     0,r9
        mov     0,r10
        mov     0,r11
        st      r13,il_w3(r7)           # Link parent ILT to new ILT
        st      g1,il_w3(r13)           # Link ILT to parent ILT
        st      g0,vrsrp(r7)            # Link SRP to ILT
        stq     r4,sr_func(g0)          # Save the SRP Header
        stq     r8,sr_vrpilt(g0)        # Clear the VRP ILT and General Use regs
#
# --- Initialize SRP descriptors
#
        mov     0,r4                    # Set up the 1st reserved field
        mov     r14,r5                  # Copy the passed Buffer Address
        mov     r15,r6                  # Copy the passed Length
        mov     0,r7                    # Set up the 2nd reserved field
        stq     r4,sr_desc0+sr_source(g0) # Store SRP descriptor
#
# --- Exit
#
        mov     r12,g0                  # Restore g0-g3
        mov     r13,g1
        mov     r14,g2
        mov     r15,g3
        ret
#
.if WC_ENABLE
#**********************************************************************
#
#  NAME: c$adrpsrp
#
#  PURPOSE:
#       To provide a common means of assigning an ILT and SRP for
#       supporting a DRP operation.
#
#  DESCRIPTION:
#       An ILT and SRP are dynamically assigned from the system with
#       wait.  The SRP is initialized and the S/G descriptors are
#       copied from the Datagram to the SRP.
#
#       NOTE:  Since DRPs are primarily used for the Mirror data in the
#           Back End, the addresses will be translated into BE PCI addresses
#           before being put into the SRP descriptors
#
#  CALLING SEQUENCE:
#       call    c$adrpsrp
#
#  INPUT:
#       g1 = parent ILT
#       g2 = Datagram Request Address
#       g3 = SRP Function
#
#  OUTPUT:
#       Parent ILT
#           il_w3 = ILT/SRP
#
#       SRP ILT
#           il_w3 = ILT/DRP
#           il_w4 (vrsrp) = SRP
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
c$adrpsrp:
        mov     g0,r12                  # Save g0-g3
        mov     g1,r13                  # r13 = Parent ILT
        mov     g2,r14                  # r14 = Datagram Request Address
        mov     g3,r15                  # r15 = SRP function
#
# --- Assign new ILT
#
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
#
# --- Assign SRP from non-cached DRAM
#
        mov     g1,r7                   # Save the new ILT address
        st      g1,il_w3(r13)           # Link ILT to parent ILT
        ldconst srpbsiz,r5              # Calculate the SRP length
        ld      dgrq_reqlen(r14),r6     # r6 = Length of Datagram descriptors
        ldob    dgrq_hdrlen(r14),r8     # r8 = Datagram Request Length
        ldconst srpesiz,r3              # r3 = size of one SRP descriptor
        addo    r8,r14,r8               # r8 = pointer to the descriptors
        divo    CAC0_Entry_len,r6,r6    # r6 = number of descriptors
        mulo    r6,r3,r3                # r3 = size of all SRP descriptors
        addo    r3,r5,r5                # r5 = SRP size
        st      r13,il_w3(r7)           # Link parent ILT to new ILT
c       g0 = s_MallocC(r5, __FILE__, __LINE__); # Assign memory
#
# --- Initialize SRP
#
        mov     r15,r4                  # r4 = SRP function
        st      g0,vrsrp(r7)            # Link SRP to ILT
        stq     r4,sr_func(g0)          # Save the SRP Header
                                        # r4 = SRP function
                                        # r5 = Size of the SRP
                                        # r6 = Number of SRP Descriptors
                                        # r7 = SRP ILT
        st      r13,sr_vrpilt(g0)       # Save the DRP/ILT in the SRP
#
# --- Initialize SRP descriptors
#
        lda     sr_desc0(g0),g0         # Advance to 1st SRP descriptor
        mov     r6,r9                   # r9 = number of descriptors
        mov     0,r4                    # Set up the 1st reserved field
        mov     0,r7                    # Set up the 2nd reserved field
#
# --- Construct SRP descriptors
#
.cadrpsrp40:
        ld      CAC0_rq_mem_addr(r8),r5 # Set up the Buffer Address
        lda     BE_ADDR_OFFSET(r5),r5   # Translate to a BE address
        ld      CAC0_rq_mem_len(r8),r6  # Set up the Buffer Length
        stq     r4,sr_source(g0)        # Store SRP descriptor
        subo    1,r9,r9                 # Decrement the number of descriptors
        lda     CAC0_Entry_len(r8),r8   # Increment to the next Dg descriptor
        lda     srpesiz(g0),g0          # Increment to the next SRP descriptor
        cmpobne 0,r9,.cadrpsrp40        # Jif there are more descriptors
#
# --- Exit
#
        mov     r12,g0                  # Restore g0-g3
        mov     r13,g1
        mov     r14,g2
        mov     r15,g3
        ret
.endif   # WC_ENABLE
#
#**********************************************************************
#
#  NAME: c$rsrp
#
#  PURPOSE:
#       To provide a common means of releasing an ILT and read/write
#       SRP back to the system.
#
#  DESCRIPTION:
#       The ILT is released back to the ILT pool.  The read/write SRP
#       is released back to the appropriate pool.
#
#       This routine may be called from either the process or interrupt
#       level.
#
#       This routine can handle SRPs of differing size from the preallocated
#       pool size.  If a SRP is not the preallocated size it is not placed
#       into the freelist; instead it is handled by the memory release
#       process.
#
#  CALLING SEQUENCE:
#       call    c$rsrp
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
c$rsrp:
        mov     g0,r14                  # Save g0-g1
        mov     g1,r15
        ld      vrsrp(g1),g0            # g0 = SRP
#
# --- Release ILT
#
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
#
        ldob    sr_flag(g0),r5          # Get the flag byte
        ld      sr_plen(g0),g1          # g1 = packet length
        ld      C_SFreehead,r4          # Get the head of the freelist
        ldconst (srpbsiz+srpesiz),r6    # Size of prealloc SRP
        cmpobne g1,r6,.crsr10           # Jif not standard size
        bbc     sr_perm_store,r5,.crsr10 # Jif not permanent storage
#
# --- Put this SRP on freelist
#
        st      g0,C_SFreehead          # Save new head pointer
        st      r4,(g0)                 # Link to head of list
        b       .crsr20
#
# --- Release SRP using a deferred release
#
.crsr10:
c       s_Free_and_zero(g0, g1, __FILE__, __LINE__); # SRP must be zeroed when freed.
#
# --- Exit
#
.crsr20:
        mov     r14,g0                  # Restore g0,g1
        mov     r15,g1
        ret
#
#**********************************************************************
#
#  NAME: c$calllower
#
#  PURPOSE:
#       To provide a common means of calling the next lower layer
#
#  DESCRIPTION:
#       The ILT completion routine is set up at this layer and the next (as
#       required by the Link layer) and set the VRP address in the proper
#       field as well.
#
#  CALLING SEQUENCE:
#       call    c$calllower
#
#  INPUT:
#       g1 = ILT
#       g2 = completion routine
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
c$calllower:
#
# --- Save completion routine in ILT and clear the ILT Forward Link
#
        mov     0,r3
        st      g2,il_cr(g1)            # Save completion routine at this level
        st      r3,il_fthd(g1)          # Close link
#
# --- Move the ILT down one layer, copy the VRP, clear the ILT forward link,
#       and set up the Retry Checker completion routine.
#
        mov     g1,r15                  # Save g1
        ld      vrvrp(g1),r6            # Get the VRP address
        lda     ILTBIAS(g1),g1          # Go the to next level
        ld      vr_sglptr(r6),r7        # r7 = SGL pointer
c       if (r7 == 0xfeedf00d) {
c           fprintf(stderr,"%s%s:%u c$calllower sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__);
c           abort();
c       }
        lda     CA_SendCompRetryCheck,r5  # Get the Completion routine
#
# --- If the VRP has no SGLs, then fill in an invalid SGL pointer to get around
#       code in the BE that is testing for NULL SGLs disregarding the type of
#       command.
#
        cmpobne 0,r7,.clower_50         # Jif there is a valid SGL pointer
        ldconst 0xfeedf00d,r7           # r7 = fictional pointer, if accessed
                                        #   gets an error
        st      r7,vr_sglptr(r6)        # Save the fictional SGL pointer
.clower_50:
        st      r6,vrvrp(g1)            # Save the VRP address at this level
        st      r5,il_cr(g1)            # Save the new completion routine
        st      r3,il_fthd(g1)          # Close link
        st      r3,il_w0(g1)            # Clear the Retry Counter
                                        # Input:  g1 = ILT
# Allow 4700 and 7000 to have same code.
.ifndef MODEL_3000
.ifndef MODEL_7400
        ldob     vr_options(r6),r7
        setbit   vrretryonce,r7,r7
        stob     r7,vr_options(r6)
.endif  # MODEL_7400
.endif  # MODEL_3000
        call    CA_CallLower2           # Go do the Rebuild Check and Mirroring
                                        #  if needed
#
# --- All Done
#
        mov     r15,g1                  # Restore g1
        ret
#
#**********************************************************************
#
#  NAME: CA_CallLower2
#
#  PURPOSE:
#       To provide the next level of checking before sending the op to the
#       next lower level.
#
#  DESCRIPTION:
#       This function implements the needed checks before sending the op to
#       the next lower level.  The checks are to see if a Rebuild Check Op
#       needs to be sent before this op and if any Mirror Information needs
#       to be sent as well.
#
#  CALLING SEQUENCE:
#       call    CA_CallLower2
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
CA_CallLower2:
#
# --- Determine if this is a write type of request that needs the write
#       information mirrored to the other controller to ease RAID
#       resync problems
#
        ld      vrvrp(g1),r6            # Get the VRP
        mov     g0,r12                  # Save g0
.if 1 #CSTOP_SAN1171,1416,1670
        ldob    vr_options(r6),r5
        setbit  vrinbackend,r5,r5       # Indicate VRP is in  BackEnd
        stob    r5,vr_options(r6)       # Update vrp options
.endif
        ldos    vr_vid(r6),r5           # r5 = VID
        ldos    vr_func(r6),r4          # r4 = Function
        ld      vcdIndex[r5*4],r8       # r8 = VCD pointer for this VID
        ldob    vc_stat(r8),r9          # r9 = VID Status
        bbc     vc_mirror_write_info,r9,.clower_200 # Jif no Write Info Mirror
        cmpobe  vroutput,r4,.clower_100 # Jif this is a Write request
        cmpobne vroutputv,r4,.clower_200 # Jif not a Write and Verify request
#
# --- Write type request with VID needing Write Info Mirroring
#
.clower_100:
.if 1 # VIJAY_MC
        ldconst 88278827,r7             # load reference magic number
# This value may not be set -- really a bad way to mark things.  2008-06-14
?       ld      il_w2(g1),r4            # Load magic number
        cmpobne r4,r7,.clower_149       # Jif not magic number(Raid not degraded)
#c       fprintf(stderr,"<VIJAY>CallLower2--exclusive write fail with special error- issue rebuild before write\n");
        ldconst 0,r4
        st      r4,il_w2(g1)
        b       .clower_150a
#
.clower_149:
        bbc     vc_rebuild_required,r9,.clower_150 # Jif Rebuild not required
#c       fprintf(stderr,"<VIJAY>CallLower2--issuing rebuild before write(vid=%lx)...\n",r5);
.clower_150a:
        call    c$issueRebuild          # Rebuild data area before writing
        b       .clower_1000            # Return while Rebuild is outstanding
#
.clower_150:
        ldconst 0,r4
        setbit  0,r4,r4
        st      r4,vr_use2(r6)          # set 0-bit indicating no rebuild before write.
        call    c$setWriteInfo          # Mirror the write information and then
                                        #   do the write
                                        # g1 = ILT (input)
                                        # g0 = True - Mirror Info In Progress
                                        #    = False - Mirror not needed
        cmpobe  TRUE,g0,.clower_1000    # Jif a mirror is in progress
.else # VIJAY_MC
.if 1
# ---    Miscompare fix for part-2 issue (write without rebuild- 19732)
#
# ---   This was commented out to force rebuilds before write even though the stripe has
#       been rebuilt. There is a timing issue because of which the FE can't  able to
#       get the raid rebuild information from BE leading FE to send writes without the
#       rebuild having been sent first that leads to stripe corruption when unknown
#       data from the hotpsare is used in parity calculations in a normal op. The
#       overhead of the change is not that much significant except in extreme bench
#       mark cases.
#
        call    c$issueRebuild          # Rebuild data area before writing
        b       .clower_1000            # Return while Rebuild is outstanding
.else   /* 1 */
        bbc     vc_rebuild_required,r9,.clower_150 # Jif Rebuild not required
        call    c$issueRebuild          # Rebuild data area before writing
        b       .clower_1000            # Return while Rebuild is outstanding
#
.clower_150:
        call    c$setWriteInfo          # Mirror the write information and then
                                        #   do the write
                                        # g1 = ILT (input)
                                        # g0 = True - Mirror Info In Progress
                                        #    = False - Mirror not needed
        cmpobe  TRUE,g0,.clower_1000    # Jif a mirror is in progress
.endif  # 1
.endif # VIJAY_MC
#
# --- Non-Write type Request or this VID does not require Write Info Mirroring
#
.clower_200:
        call    CA_CallLowerSend        # Send on the request to the Link Layer
#
# --- Exit
#
.clower_1000:
        mov     r12,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: CA_CallLowerSend
#
#  PURPOSE:
#       To send on the ILT requested (with the VRP) to the Link Layer
#
#  DESCRIPTION:
#       The ILT is dropped one layer, the Retry Checker Completion routine is
#       set up and then the ILT is queued to the Link Layer to forward on the
#       VRP to the Virtual Layer.
#
#  CALLING SEQUENCE:
#       call    CA_CallLowerSend
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
CA_CallLowerSend:
        movq    g0,r12                  # Save g0-g3, g14
        mov     g14,r11
.if     DEBUG_FLIGHTREC_C
        ld      vrvrp(g1),r4            # r4 = VRP
        ld      -ILTBIAS+il_cr(g1),r7   # r7 = Previous level Completion routine
        ldos    vr_func(r4),r6          # r6 = Function
        ldos    vr_vid(r4),r5           # r5 = VID
        ldconst frt_c_lower,r3          # Cache - c$calllower
        shlo    8,r6,r6
        shlo    16,r5,r5
        or      r6,r3,r3
        or      r5,r3,r3
        st      g1,fr_parm1             # ILT
        st      r4,fr_parm2             # VRP
        st      r7,fr_parm3             # Completion Routine to call
        st      r3,fr_parm0             # VID, Function, Flight Recorder ID
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_C
#
# --- Queue request
#
        call    L$que                   # Queue request
#
# --- Exit
#
        movq    r12,g0                  # Save g0-g3, g14
        mov     r11,g14
        ret
#
#**********************************************************************
#
#  NAME: CA_SendCompRetryCheck
#
#  PURPOSE:
#       Upon completion of the op at the lower level, determine if the Cache
#       Layer needs to retry the op or let it complete.
#
#  DESCRIPTION:
#       The completion handler will determine from the VRP status if the
#       Op needs to be retried after a delay.  If not, the ILT is completed
#       at this level and continues on.  Otherwise, the ILT is put on a queue
#       that will handle the retries.
#
#  CALLING SEQUENCE:
#       call    CA_SendCompRetryCheck
#
#  INPUT:
#       g0 = Completion Status
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
CA_SendCompRetryCheck:
        mov     g0,r15                  # Save g0
#
# --- Get the VRP and prepare to save the completion status in it.  Determine
#       if the Status is RETRY.
#
        ld      vrvrp(g1),r3            # r3 = VRP
.if 1 #CSTOP_SAN1171,1416,1670
        ldob    vr_options(r3),r12
        clrbit  vrinbackend,r12,r12     # Indicate VRP is returned from BE
                                        # Now in FE.
        stob    r12,vr_options(r3)      # Update vrp options
.endif
        ldconst ecspecial,r12
        cmpobne g0,r12,.csndcmprtrychk00
#        c   fprintf(stderr,"<VIJAY>CA_SendCompRetryCheck>special-error occured\n");
        ldconst 88278827,r4             # store magic number
        st      r4,il_w2(g1)
        ldconst ecretry,g0              # Now treat as ecretry
        b       .csndcmprtrychk02
#
.csndcmprtrychk00:
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ldconst ecbebusy,r12           # r12 = ISE BUSY
        cmpobne  r12,g0,.csndcmprtrychk0a # Jif not ISE BUSY
#
# --- We got ISE BUSY. Mark the VCD state to ISE_BUSY
#
        ldconst ecretry,g0              # Now treat as ecretry
        ldconst 0,r4                    # r4 = 0
        st      r4,il_w0(g1)            # Clear the Retry Counter
        stob    g0,vr_status(r3)        # Save the Return Status in the VRP
#
        ldos    vr_vid(r3),r12          # r12 = VID
        ld      vcdIndex[r12*4],r12     # r12 = VCD
        ldob    vc_stat(r12),r4         # Get the current State
        bbs     vc_ise_busy,r4,.csndcmprtrychk02 # Jif VCD not in busy state
        setbit  vc_ise_busy,r4,r4       # Set the ISE BUSY bit
        stob    r4,vc_stat(r12)
        b       .csndcmprtrychk02
#
.csndcmprtrychk0a:
#
# --- We got non ISE BUSY response. Clear the VCD ISE_BUSY state if set
#
        ldos    vr_vid(r3),r12          # r12 = VID
        ld      vcdIndex[r12*4],r12     # r12 = VCD
        ldob    vc_stat(r12),r4         # Get the current State
        bbc     vc_ise_busy,r4,.csndcmprtrychk0b # Jif VCD not in busy state
        clrbit  vc_ise_busy,r4,r4       # clear the ISE BUSY bit
        stob    r4,vc_stat(r12)
#
.csndcmprtrychk0b:
.endif  # MODEL_7400
.endif  # MODEL_3000
        ldconst ecretryrc,r12           # r12 = RETRY-Reset Count Status
        cmpobne  r12,g0,.csndcmprtrychk01 # Jif not RETRYRC
        ldconst ecretry,g0              # Now treat as ecretry
        ldconst 0,r4                    # r4 = 0
        st      r4,il_w0(g1)            # Clear the Retry Counter
        stob    g0,vr_status(r3)        # Save the Return Status in the VRP
        b       .csndcmprtrychk02
#
.csndcmprtrychk01:
        ldconst ecretry,r12             # r12 = RETRY Status
        cmpobne r12,g0,.csndcmprtrychk90 # Jif not RETRY status
#
# --- The op needs to be retried.  Determine if the retry count has been
#       exhausted.  If so, return Device Not Ready back to the next ILT Level.
#       If not, determine what Outstanding Request Count value is based on this
#       ops waiting list, increment the count, and queue it to the
#       Op Retry Task.
#
.csndcmprtrychk02:
        ld      il_w0(g1),r4            # r4 = Retry Count
        ldconst EC_RETRY_COUNT,r14      # r14 = Maximum Retry Counter
        ldconst ecinop,g0               # Prep Device Not Ready status
        cmpinco r14,r4,r4               # Compare to max and increment
.if 1 #VIJAY_MC
        ble     .csndcmprtrychk91       # Jif retries exhausted
.else   # 1
        ble     .csndcmprtrychk90       # Jif retries exhausted
.endif  # 1
#
# --- Determine if we are possibly stuck in c\$stop, rdalocalimageip contention
#
        lda     C_ca,r7                 # Get the CA Structure address
        ldob    ca_stopcnt(r7),r8       # Get the overall stop counter
        cmpobe  0,r8,.csndcmprtrychk15  # No stops pending
#
#     Look in C_user_stop_cnt for Proc users.
#
        ldconst 32,r7                   # BE user 0x20(32) - 0x3F(63)
.csndcmprtrychk03:
        ldob    C_user_stop_cnt[r7*1],r8 # Get the user stop counter
        cmpobne 0,r8,.csndcmprtrychk10  # BE cstop pending
        addo    1,r7,r7                 # Bump BE User
        cmpobe  64,r7,.csndcmprtrychk04 # No BE stops pending, check define
        b       .csndcmprtrychk03
#
#
#     Look in define queue.
.csndcmprtrychk04:
        ld      d_exec_qu+qu_head,r8    # Get queue head
        ld      d_exec_qu+qu_tail,r9    # Get queue tail
        ld      d_exec_qu+qu_qcnt,r10   # Get queue count
        ld      d_exec_qu+qu_pcb,r11    # Get queue PCB
#
.csndcmprtrychk05:
#
#
# - At this point of time the C_user stop count for proc users =0; when there are no requests in define queue(r8=0),
#   but the  Overall stop counter is not zero, means there are some stops still pending. Can we still retry the
#   the OP (OR) we need to set the retry count to MAX to stop the retries and complete the OP with ecinop. This
#   will faciliate the pending stops to be completed first.
#
# - Currently the code is retrying the OP,even though the overall stop count is not zero (branching to .csndcmprtrychk15)
#   (To Investigate further - VIJAY)
#
        cmpobe  0,r8,.csndcmprtrychk15  # Jif none
        ld      il_w0-ILTBIAS(r8),r7    # Get request ptr
        ldos    mr_func(r7),r9          # Get request function type
        cmpobe  0x202,r9,.csndcmprtrychk10 # Configure cache
        cmpobe  0x205,r9,.csndcmprtrychk10 # Cache Stop cache
        ld      il_fthd(r8),r8          # Next.
        b       .csndcmprtrychk05       # Loop
#
#     Change retries to one last one.
#
.csndcmprtrychk10:
        ldconst EC_RETRY_COUNT,r4       # Somethings fishy, one last retry.
#
.csndcmprtrychk15:
        ldob    vr_options(r3),r7       # r7 = VRP Options flag
        ld      C_orc,r8                # r8 = Cache Outstanding Host Op Count
        ld      C_flush_orc,r9          # r9 = Cache Flush Outstanding Op Cnt
        bbc     vrcorc,r7,.csndcmprtrychk20 # Jif not a C_orc type op
        subo    1,r8,r8                 # Decrement the count and save the new
.ifdef M4_DEBUG_C_orc
c CT_history_printf("%s%s:%u: C_orc starts at %lu, ends at %lu\n", FEBEMESSAGE,__FILE__, __LINE__, C_orc, r8);
.endif  # M4_DEBUG_C_orc
        st      r8,C_orc                #  count so a C$Stop will be able to
                                        #  complete
.if 0 #CSTOP_SAN1171,1416,1670
        clrbit  vrnotcompleted,r7,r7    # Indicate VRP is completed
        stob    r7,vr_options(r3)       # Update vrp options
.endif
.csndcmprtrychk20:
        bbc     vrforc,r7,.csndcmprtrychk30 # Jif not a C_flush_orc type op
        subo    1,r9,r9                 # Decrement the count and save the new
        st      r9,C_flush_orc          #  count so a C$Stop will be able to
                                        #  complete
#
#   Remove any Outstanding Throttle Values to allow other ops to go.  Also kick
#       off the Throttle Task to let anything go.
#
.csndcmprtrychk30:
        ldos    vr_vid(r3),r11          # r11 = VID
        ld      vr_otv(r3),r6           # r6 = Op Throttle Value
        ld      C_vcd_wait_head,r5      # r5 = The wait head queue
        ld      vcdIndex[r11*4],r11     # r11 = VCD
        ld      C_ctv,r9                # r9 = Controller Throttle Value
        ld      C_vcd_wait_pcb,r7       # r7 = VCD Throttle Wait Exec PCB
        ld      vc_vtv(r11),r8          # r8 = VDisk Throttle Value
        subo    r6,r9,r9                # Update the CTV
        subo    r6,r8,r8                # Update the VTV
.ifdef M4_DEBUG_C_ctv
c CT_history_printf("%s%s:%u: C_ctv starts=%lu ends=%lu vc_vtv[]=%ld\n", FEBEMESSAGE,__FILE__, __LINE__, C_ctv, r9, r8);
.endif  # M4_DEBUG_C_ctv
        st      r9,C_ctv
        st      r8,vc_vtv(r11)
#
        cmpobe  0,r5,.csndcmprtrychk40  # Jif nothing is waiting
        ldob    pc_stat(r7),r8          # r8 = Current process status
        mov     pcrdy,r5                # Set ready status
        cmpobne pcnrdy,r8,.csndcmprtrychk40 # Jif status other than not ready
.ifdef HISTORY_KEEP
c CT_history_pcb(".csndcmprtrychk30 setting ready pcb", r7);
.endif  # HISTORY_KEEP
        stob    r5,pc_stat(r7)          # Ready process
#
#   Queue this op
#
.csndcmprtrychk40:
        st      r4,il_w0(g1)            # Save the new Retry Counter
        ld      K_ii+ii_time,r5         # r5 = Current Time
        ldconst EC_RETRY_TIME,r13       # r13 = Retry time counter
        addo    r13,r5,r6               # r6 = Time to kick this op off again
        st      r6,il_w1(g1)            # Save the new time to kick off the op
        lda     CA_OpRetryQue,r11       # r11 = Queue to put this ILT on
        mov     r15,g0                  # Restore g0
        b       K$cque                  # Queue the op to the Op Retry Task
                                        #  and return
#
# --- No need to retry the operation, complete the op to the next ILT level
#
.if 1 #VIJAY_MC
.csndcmprtrychk91:
        ldconst 0,r4
        st      r4,il_w2(g1)
.endif  # 1
.csndcmprtrychk90:
        cmpobne ecinop,g0,.csndcmprtrychk99 # Jif status other than not ecinop
        ldconst ecbusy,g0               # Change status to ecbusy
.csndcmprtrychk99:
        stob    g0,vr_status(r3)        # Save the Return Status in the VRP
        call    K$comp                  # Done at this level complete it
#
# --- Exit
#
        mov     r15,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: CA_RemoveVIDRetryQueue
#
#  PURPOSE:
#       This function will remove any Ops for a specified VID from the RETRY
#       Queue and I/O Pending Queue.  It will return them to the host with a
#       Non-Existent Device Error.
#
#  DESCRIPTION:
#       This function is used to remove any host operations from the RETRY queue
#       that are associated with a specific VID that is being deleted.  Any ops
#       found for this VID are removed from the Queue and returned to the
#       Requester with a Non-Existent Device Error.  The Pending I/O tree is
#       also searched and any that are waiting on the RETRY queue op will also
#       be returned to the requester with Non-Existent Device Error.
#
#       NOTE: Assumes a C$Stop has completed successfully, if needed, before
#       calling this function
#
#  CALLING SEQUENCE:
#       call    CA_RemoveVIDRetryQueue
#
#  INPUT:
#       g0 = VID that needs any ops in the Retry Queue removed
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
CA_RemoveVIDRetryQueue:
        PushRegs(r15)                   # Save all "g" registers
#
# --- Walk the VCD I/O Tree and remove any Ops waiting for other ops (the
#       outstanding ops are assumed to be on the Op Retry Queue)
#
        mov     g0,r14                  # r14 = VID being passed in
        ld      vcdIndex[g0*4],r3       # r3 = VCD
        ld      vc_io(r3),g0            # g0 = I/O Tree Root Pointer
        cmpobe  0,g0,.crvidrq1000       # Jif nothing on the I/O Tree
        movl    0,g2                    # g2,g3 = Lower Key to look for
        ldconst 0xFFFFFFFF,g4           # g4,g5 = Maximum Key to look for -
        ldconst 0xFFFFFFFF,g5           #  go through entire tree
        call    RBI$foverlap            # g1 = Lowest Node in tree
.crvidrq020:
        cmpobe  FALSE,g1,.crvidrq200    # Jif all the Nodes have been walked
        mov     g1,r13                  # Save this Node for RBI$noverlap
        mov     g1,r8                   # Loop RBI Node pointer
.crvidrq030:
        ld      rbdpoint(r8),r12        # r12 = ILT this RBI Node points to
.crvidrq040:
        ld      il_fthd(r12),r11        # r11 = Placeholder waiting on this ILT
        cmpobe  0,r11,.crvidrq050       # Jif no other ops waiting on this ILT
        ld      il_misc(r11),r10        # r10 = Original ILT held by Placeholder
        ld      il_w1(r10),r9           # r9 = Blocked I/O counter
#
#   Unlink placeholder from list
#
        ld      il_fthd(r11),r4         # get forward pointer
        ld      il_bthd(r11),r5         # get backward pointer
        cmpo    r4,r5                   # Check for last blocked I/O on list
        sele    r4,0,r6                 # Set pointers
        sele    r5,0,r7                 #   to zero if true
        st      r6,il_fthd(r5)          # set forward pointer of previous block
#
#   Because the blocked list is circular, the backward thread pointer is
#       never null
#
        st      r7,il_bthd(r4)          # Unlink from list
#
#   Deallocate placeholder
#
        mov     r11,g1                  # g1 = Placeholder ILT to free
c       put_wc_plholder(g1);            # Release the placeholder ILT
#
#   Decrement blocked I/O count
#
        cmpdeci 1,r9,r9
        st      r9,il_w1(r10)           # Save blocked I/O counter
        bl      .crvidrq040             # Jif still waiting on other ops - check
                                        #  for more Ops waiting on this Op
#
#   This op is no longer waiting for other ops to complete but it was waiting,
#       so now it can just be completed with the Non-Existant Device Error Code.
#       Remove the RBI Node associated with this Op, decrement the ORC for
#       this Op, and complete it.
#
        ld      il_w5(r10),g1           # g1 = RBI Node to remove
        ld      vc_io(r3),g0            # g0 = Root of I/O tree
        call    WC$delete_node          # Remove the node from the tree
        st      g0,vc_io(r3)            # Save the new root pointer
        mov     r10,g1                  # Set up to complete this ILT
        ld      vrvrp(r10),r11          # r11 = VRP from ILT
        ld      C_orc,r4                # r4 = Outstanding Request Count
        subo    1,r4,r4                 # Decrement this Op
.ifdef M4_DEBUG_C_orc
c CT_history_printf("%s%s:%u: C_orc starts at %lu, ends at %lu\n", FEBEMESSAGE,__FILE__, __LINE__, C_orc, r4);
.endif  # M4_DEBUG_C_orc
        st      r4,C_orc                # Save the new ORC
        ldconst ecnonxdev,g0            # Set up Return Status as Non-Existent
        stob    g0,vr_status(r11)       # Save the status in the VRP
        call    K$comp                  # Complete this op - will go back to
                                        #  MAGDriver Level
        b       .crvidrq040             # Check for more ops waiting on this op
#
#   No more Placeholder ILTs (waiting Ops) on this RBI Node, see if other RBI
#       nodes have the same key and work with them.
#
.crvidrq050:
        ld      rbfthd(r8),r8           # Get the next node on this list
        cmpobne 0,r8,.crvidrq030        # Jif more nodes to deal with
#
#   No more ILTs to work with at this point, look for more in the I/O tree
#
        mov     r13,g1                  # g1 = New starting point to find next
        movl    0,g2                    # g2,g3 = Lower Key to look for
        ldconst 0xFFFFFFFF,g4           # g4,g5 = Maximum Key to look for -
        ldconst 0xFFFFFFFF,g5           #  go through entire tree
        call    RBI$noverlap            # Find the next overlap
        b       .crvidrq020             # Continue removing waiting Ops
#
# --- Get Queue Head, Tail, and Count, and prep for walking the OP Retry Queue
#
.crvidrq200:
        lda     CA_OpRetryQue,r11       # Get executive queue pointer
        ld      qu_head(r11),r8         # Get queue head
        ld      qu_tail(r11),r9         # Get queue tail
        ld      qu_qcnt(r11),r10        # Get queue count
#
#   Walk the queue to find any Ops that have the requested VID.  If found,
#       remove it and return a Non-existant error as the completion code.
#
.crvidrq210:
        cmpobe  0,r8,.crvidrq900        # Jif there are no more ops queued
#
        ld      vrvrp(r8),r3            # r3 = VRP Pointer
        ldconst ecnonxdev,g0            # Set up Return Status as Non-Existent
        ldos    vr_vid(r3),r6           # r6 = VRP VID
        cmpobne r14,r6,.crvidrq280      # Jif this is not the VID requested
#
#   Remove op from the queue (beginning, middle, or end)
#
        ld      il_fthd(r8),r4          # r4 = forward thread of ILT
        ld      il_bthd(r8),r5          # r5 = backward thread of ILT
                                        # r11 = base address of working queue
        mov     r8,g1                   # Isolate this queued ILT
        subo    1,r10,r10               # Adjust queue count
        st      r4,il_fthd(r5)          # put forward thread from removed ILT
                                        #  as forward thread of previous ILT
        cmpobne 0,r4,.crvidrq220        # Jif non-zero forward thread
        mov     r11,r4                  # make base of queue the forward thread
        cmpo    r11,r5                  # Determine if backward thread <> base
        sele    r5,0,r5                 # If they are equal, then queue is empty
.crvidrq220:
        st      r5,il_bthd(r4)          # put backward thread from removed ILT
                                        #  as backward thread of previous ILT
        st      r10,qu_qcnt(r11)        # Save updated queue count
#
#   Update the Outstanding Request Counter to what it should have been before
#   being removed, update the Throttle Values appropriately, and then return
#   Non-Existent Device to the original Requester.
#
        ldob    vr_options(r3),r4       # Get the Options byte
        bbc     vrcorc,r4,.crvidrq240   # Jif this is not a C_orc type op
        ld      C_orc,r5                # Increment the outstanding Request Cnt
        addo    1,r5,r5
.ifdef M4_DEBUG_C_orc
c CT_history_printf("%s%s:%u: C_orc starts at %lu, ends at %lu\n", FEBEMESSAGE,__FILE__, __LINE__, C_orc, r5);
.endif  # M4_DEBUG_C_orc
        st      r5,C_orc
.crvidrq240:
        bbc     vrforc,r4,.crvidrq250   # Jif this is not a C_flush_orc type op
        ld      C_flush_orc,r5          # Increment the outstanding Request Cnt
        addo    1,r5,r5
        st      r5,C_flush_orc
.crvidrq250:
        ld      vcdIndex[r6*4],r4       # r4 = VCD
        ld      vr_otv(r3),r5           # r5 = OTV from the VRP
        ld      C_ctv,g8                # g8 = CTV
        ld      vc_vtv(r4),g7           # g7 = VTV
        addo    r5,g8,g8                # Update the CTV
        addo    r5,g7,g7                # Update the VTV
.ifdef M4_DEBUG_C_ctv
c CT_history_printf("%s%s:%u: C_ctv starts=%lu ends=%lu vc_vtv[%ld]=%ld\n", FEBEMESSAGE,__FILE__, __LINE__, C_ctv, g8, r6, g7);
.endif  # M4_DEBUG_C_ctv
        st      g8,C_ctv
        st      g7,vc_vtv(r4)
#
        stob    g0,vr_status(r3)        # Save the status in the VRP
        call    K$comp                  # Complete this ILT
#
.crvidrq280:
        ld      il_fthd(r8),r8          # r8 = next ILT on the list
        b       .crvidrq210
#
# --- Ensure there are no more ops on the I/O Tree
#
.crvidrq900:
        ld      vcdIndex[r14*4],r3      # r3 = VCD address
        ld      vc_io(r3),r4            # r4 = I/O Tree Pointer
        cmpobe  0,r4,.crvidrq1000       # Jif No Ops still outstanding
        call    .err19                  # Ops outstanding still - error trap
#
# --- Exit
#
.crvidrq1000:
        PopRegsVoid(r15)                # Restore all "g" registers
        ret
#
#**********************************************************************
#
#  NAME: c$callupper
#
#  PURPOSE:
#       To provide a common means of calling the layer above Cache
#
#  DESCRIPTION:
#       The ILT completion routine is set up at this layer and the next (as
#       required by the Cdriver layer) and set the VRP address in the proper
#       field as well.
#
#  CALLING SEQUENCE:
#       call    c$callupper
#
#  INPUT:
#       g1 = ILT
#       g2 = completion routine
#
#       callers ILT
#           il_w3 = VRP ILT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
c$callupper:
#
# --- Save completion routine in ILT
#
        st      g2,il_cr(g1)            # Save completion routine at this level
        mov     g0,r12                  # Save g0
        mov     g1,r13                  # Save g1
        ld      il_w3(g1),r4            # Get the VRP ILT
        ld      vrsrp(g1),g0            # Get the SRP for this request
#
# --- Advance ILT to next level
#
        mov     g2,r14                  # Save g2
        ld      il_misc(r4),r8          # Get the ILT Parms Pointer
        ld      vrvrp(r4),r15           # r15 = VRP
.if     DEBUG_FLIGHTREC_C
        ldconst frt_c_upper,r3          # Cache - c$callupper
        ldos    vr_func(r15),r7         # r7 = Function
        ldos    vr_vid(r15),r5          # r5 = VID
        shlo    8,r7,r7
        shlo    16,r5,r5
        or      r7,r3,r3
        or      r5,r3,r3
        st      r3,fr_parm0             # VID, Function, Flight Recorder ID
        st      r4,fr_parm1             # ILT
        st      r15,fr_parm2            # VRP
        st      r13,fr_parm3            # ILT of SRP
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_C
        mov     0,r3
        st      r3,il_fthd(g1)          # Clear this link
        st      r3,ILTBIAS+il_fthd(g1)  # Close link
        lda     ILTBIAS(g1),g1          # Advance to next level
#
        mov     0,r6
        ld      vr_use0(r15),r5         # Get the parent VRP
        cmpobe  0,r5,.cupper10          # Jif g0 is parent VRP
        ld      -ILTBIAS-ILTBIAS+il_w0(r4),r6   # r6 = relative offset
        mulo    512,r6,r6
#
.cupper10:
#
# --- Retrieve and save the VRP Pointer in the ILT along with the SRP fields
#
        ldob    sr_func(g0),r5          # r5 = function code
        ldob    vr_status(r15),r9       # r9 = VRP Completion Status
        lda     c$srpcomp,r7            # setup completion routine
        st      r8,otl1_FCAL(g1)        # Store ILT parms pointer
        st      g0,otl1_srp(g1)         #   and SRP pointer
        stob    r5,otl1_cmd(g1)         # Store function code
        stob    r9,otl1_cmpcode(g1)     # Store the VRP Completion Status
        st      r6,otl1_relofset(g1)    # store relative offset
#
        st      r7,il_cr(g1)            # Store the completion routine
        mov     g1,r8                   # Preserve ILT pointer
#
        lda     ILTBIAS(g1),g1          # advance to next nesting level
        st      r8,il_misc(g1)          #  and point back to struct
        st      r3,il_cr(g1)            # Clear next nest completion routine
#
# --- Submit SRP to Translation Layer.
#
        call    C$receive_srp
#
# --- Exit
#
        mov     r12,g0                  # Restore g0-g2
        mov     r13,g1
        mov     r14,g2
        ret
#
#**********************************************************************
#
#  NAME: c$vrpcheck
#
#  PURPOSE:
#       To provide a means of validating a VRP received from the host.
#
#  DESCRIPTION:
#       This routine validates the function code, SDA, and EDA.
#
#  CALLING SEQUENCE:
#       call    c$vrpcheck
#
#  INPUT:
#       g1 = ILT associated with VRP
#       g2 = VRP
#
#  OUTPUT:
#       g2 = error code
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
c$vrpcheck:
#
# --- Validate function code
#
        ldos    vr_func(g2),r3          # r3 = function code
        lda     ecinvfunc,r15           # r15 = Prep Invalid Function error code
        cmpobl  vrmaxfunc,r3,.cvrp80    # Jif function out of range
        ld      vr_vlen(g2),r6          # r6 = length of transfer in sectors
        cmpobe  vrinput,r3,.cvrp10      # Jif this is a Read command
        cmpobe  vrverifyc,r3,.cvrp20    # Jif this is a Verify Checkword command
        ldconst vrsync,r5
        cmpobe  r3,r5,.cvrp20           # Jif this is a Cache SYNC command
        and     3,r3,r5                 # See if this is a Write type command
        cmpobne 2,r5,.cvrp90            # Jif this is not a Write type command
#
# --- Validate the Length (reads and writes only), VID, Starting Disk Address,
#       and Ending Disk Address
#
.cvrp10:
        ldconst MAXIO,r8                # See if the Transfer size is too big
        ldconst ecinvlen,r15            # Prep Invalid Length error code
        cmpobg  r6,r8,.cvrp80           # Jif the length is too big
.cvrp20:
        ldos    vr_vid(g2),r7           # r7 = VID
        ld      MAG_VDMT_dir[r7*4],r8   # r8 = pointer to assoc. VDMT
        ldconst ecinvvid,r15            # r15 = Prep Invalid VID error code
        cmpobe  0,r8,.cvrp80            # Jif this is not a valid VDMT
#
        ldconst ecinvsda,r15            # r15 = Prep Invalid SDA error code
c       if (((VRP*)g2)->startDiskAddr >= ((VDMT*)r8)->devCap) {
            b   .cvrp80                 # Jif the SDA is out of range
c       }
        ldconst ecinvda,r15             # r15 = Prep Invalid EDA error code
c       if (((VRP*)g2)->startDiskAddr + ((VRP*)g2)->length <= ((VDMT*)r8)->devCap) {
            b   .cvrp90                 # Jif the SDA+Length is ok
c       }
# c fprintf(stderr, "cvrp20-2, %llu > %llu\n", ((VRP*)g2)->startDiskAddr + ((VRP*)g2)->length, ((VDMT*)r8)->devCap);
#
# --- The VRP did not check out correctly
#
.cvrp80:
        mov     r15,g2                  # Set up error code
        b       .cvrp100
#
# --- Clear error code
#
.cvrp90:
        mov     ecok,g2                 # Clear error code
#
# --- Exit
#
.cvrp100:
        ret
#
#**********************************************************************
#
#  NAME: C$getwdata
#
#  PURPOSE:
#       To provide a means of getting write data into a buffer from the host.
#
#  DESCRIPTION:
#       This routine allocates an SRP and queues the request back to the
#       next upper layer to transfer the write data into the buffer requested.
#
#  CALLING SEQUENCE:
#       call    C$getwdata
#
#  INPUT:
#       g0 = Completion Routine to call when the data has been transfered
#       g1 = ILT associated with VRP
#       g2 = Buffer Address
#       g3 = Buffer Length (in bytes)
#
#  OUTPUT:
#       VRP ILT
#           il_w3 = SRP ILT
#
#       SRP ILT
#           il_w3 = VRP ILT
#           vrsrp(il_w4) = SRP
#           il_w5 = Callers completion routine
#
#       Filled out SRP
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
C$getwdata:
        mov     g0,r12                  # Save g0-g3
        mov     g1,r13
        mov     g2,r14
        mov     g3,r15
        mov     g4,r4                   # Preserve g4
#
        ldconst srh2c,g4                # Setup function call
        call    c$asrp                  # Get ILT/SRP for op
#
# --- Set up and call the routine to request the host data
#
        ld      il_w3(g1),g1            # Get ILT for this SRP
        lda     c$getwdcomp,g2          # Point to the completion routine
        st      r12,il_w5(g1)           # Store completion routine in SRP ILT
        call    c$callupper             # Issue the request to get the data
#
# --- Adjust outstanding write SRP count
#
        ld      C_owsrpc,g0             # Load, Increment, and Store the
        addo    1,g0,g0                 #  outstanding write SRP count
        st      g0,C_owsrpc
#
# --- Exit
#
        mov     r12,g0                  # Restore g0-g3
        mov     r13,g1
        mov     r14,g2
        mov     r15,g3
        mov     r4,g4                   # Restore g4
        ret
#
#**********************************************************************
#
#  NAME: c$getwdcomp
#
#  PURPOSE:
#       To provide a completion path once the write data is in the buffer
#
#  DESCRIPTION:
#       This routine sets up and then call the original callers completion
#       routine and then deletes the ILT/SRP combo
#
#  CALLING SEQUENCE:
#       call    c$getwdcomp
#
#  INPUT:
#       g1 = ILT associated with SRP
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
c$getwdcomp:
        mov     g0,r14                  # Save g0-g1
        mov     g1,r15
#
# --- Get SRP completion status and call the appropriate completion function
#
        ld      vrsrp(g1),r3            # r3 = SRP
        ld      il_w5(r15),r4           # r4 = original callers completion
        ldob    sr_status(r3),g0        # g0 = SRP status
        ld      il_w3(g1),g1            # g1 = parent ILT
.if     DEBUG_FLIGHTREC_C
        ldconst frt_c_wdcomp,r3         # Cache - c$getwdcomp
        ld      vrvrp(g1),r6            # r6 = VRP
        ldos    vr_func(r6),r7          # r4 = Function
        ldos    vr_vid(r6),r5           # r5 = VID
        shlo    8,r7,r7
        shlo    16,g0,r8
        shlo    24,r5,r5
        or      r7,r3,r3
        or      r8,r3,r3
        or      r5,r3,r3
        st      r3,fr_parm0             # VID, SRP Status, Function, FRID
        st      g1,fr_parm1             # ILT
        st      r6,fr_parm2             # VRP
        st      r4,fr_parm3             # Completion routine to call
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_C
        callx   (r4)                    # Call the original completion routine
#
# --- Decrement the outstanding Write SRP Count
#
        ld      C_owsrpc,r3             # Load, Decrement, and Store the
        subo    1,r3,r3                 #  outstanding write SRP count
        st      r3,C_owsrpc
#
# --- Release write ILT/SRP and return
#
        mov     r14,g0                  # Restore g0-g1
        mov     r15,g1
        b       c$rsrp                  # Release write ILT/SRP and return
#
#**********************************************************************
#
#  NAME: c$setWriteInfo
#
#  PURPOSE:
#       To set up and begin the transfer of Write Information to the
#           mirror partner
#
#  DESCRIPTION:
#       This routine sets up and then calls the FE DLM to transfer the
#       Write information (VID, LBA, Length) to the mirror partner to aid
#       in reducing the time to resync on failover.
#
#  CALLING SEQUENCE:
#       call    c$setWriteInfo
#
#  INPUT:
#       g1 = ILT associated with write
#
#  OUTPUT:
#       g0 = TRUE - Mirror of Write Information in Progress
#          = FALSE - Mirror of Write Information is not needed
#
#  REGS DESTROYED:
#       g0
#
#**********************************************************************
#
c$setWriteInfo:
#
# --- Determine if there are any problems that would prevent this list being
#       sent to the Mirror Partner.
#
        ld      K_ficb,r3               # r3 = K_ficb pointer
        ld      fi_mirrorpartner(r3),r4 # r4 = Mirror Partners Serial Number
        ld      fi_cserial(r3),r5       # r5 = This controllers Serial Number
        ldconst 0,r12                   # r12 = clearing register
        cmpobe  r4,r5,.c_swi_900        # Jif mirroring to the BE of this cntrl
#
# --- Ensure the Mirror Partner is there
#
        lda     C_ca,r3                 # Get the Cache Information pointer
        ldob    ca_status(r3),g0        # Get the Cache Status
        bbs     ca_mirrorbroken,g0,.c_swi_900 # Jif the Mirror Partner is gone
#
# --- Queue this item to the list that will be built later when the records
#       go to the Mirror Partner.
#
        ld      C_swi_cqd,r3            # r3 = current queue depth
        ld      C_swi_qht+qu_head,r6    # r6 = Queue Head
        ld      C_swi_qht+qu_tail,r7    # r7 = Queue Tail
        cmpobne 0,r3,.c_swi_120         # Jif queue not empty
#
#   Insert into empty queue
#
        mov     g1,r7                   # Update queue head/tail with
        mov     g1,r6                   #  single entry
        st      r12,il_bthd(g1)         # Clear the Backward thread pointer
        b       .c_swi_140
#
#   Insert into non-empty queue
#
.c_swi_120:
        st      g1,il_fthd(r7)          # Append ILT to end of queue
        st      r7,il_bthd(g1)          # Save the Backward thread pointer
        mov     g1,r7                   # Update queue tail
.c_swi_140:
        addo    1,r3,r3                 # Bump queue depth
        st      r6,C_swi_qht+qu_head    # Save the Queue Head
        st      r7,C_swi_qht+qu_tail    # Save the Queue Tail
        st      r3,C_swi_cqd            # Save the current queue depth
        st      r12,il_fthd(g1)         # Clear Fwd Pointer for this ILT
#
# --- Send the list of queued items to the Mirror Partner.  Need to have at
#       least one outstanding op to handle any error state situation.  So if
#       none are outstanding, go ahead and send the op - it will get an error
#       and be put on the error state queue which will be handled properly
#       once the error state is cleared
#
        cmpobe  1,r3,.c_swi_150         # Jif only this op is outstanding -
                                        #  issue it no matter if in error state
        bbs     ca_error,g0,.c_swi_160  # Jif unable to mirror yet
.c_swi_150:
        call    c$setWriteInfoSend      # Send the list to the Mirror Partner
.c_swi_160:
        ldconst TRUE,g0                 # Show the mirror is in progress
        b       .c_swi_1000             # All Done!
#
# --- Mirroring to the Back End of this controller ---------------------------
#
.c_swi_900:
        ldconst FALSE,g0                # Set up return - no mirror occurred
#
# --- All Done
#
.c_swi_1000:
        ret
#
#**********************************************************************
#
#  NAME: c$setWriteInfoSend
#
#  PURPOSE:
#       To set up and begin the transfer of Write Information to the
#           mirror partner
#
#  DESCRIPTION:
#       This routine determines if the Write Information can be sent to the
#       Mirror Partner.  If so, it sets up and then calls the FE DLM to
#       transfer the Write information (VID, LBA, Length) to the mirror
#       partner to aid in reducing the time to resync on failover.
#
#  CALLING SEQUENCE:
#       call    c$setWriteInfoSend
#
#  INPUT:
#       None
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None
#
#**********************************************************************
#
c$setWriteInfoSend:
#
# --- Determine if there is room to send another request to the Mirror Partner
#
        ld      C_swi_cdrp,r3           # r3 = outstanding DRP count
        ld      C_swi_cqd,r13           # r13 = current queue depth
        cmpoble MAX_SWI_DRPS,r3,.c_swisend100 # Jif not able to send another req
        cmpobe  0,r13,.c_swisend100     # Jif nothing is queued to be sent
#
# --- Register Set up for all the following
#
        PushRegs                        # Save all G registers (stack relative)
        ldconst 0,r12                   # Clearing Register
#
# --- Determine if there is at least one NVA address available to continue
#
        lda     C_wi_nvac,g1            # g1 = NVA control structure address
        call    M$anvanw                # g0 = Address
        ldconst 0xffffffff,r5           # r5 = Invalid NVA Address constant
        cmpobe  g0,r5,.c_swisend99      # Jif no NVA address is available
#
c       if (MP_IS_BIGFOOT) g0 = g0+NVRAM_P4_START_BF;
        mov     g0,r15                  # r15 = NVA address to put record in
        ldconst 1,g2                    # g2 = count of records in this DRP
#
# --- Enough room to send another request.  Allocate another ILT for the DRP
#       to mirror the information
#
        addo    1,r3,r3                 # Bump the outstanding DRP count
        ldconst drpsiz+dgrq_size+dgrs_size,g0 # g0 = size of the DRP, Datagram
                                        #  Request Header, and Datagram
                                        #  Response Header
        st      r3,C_swi_cdrp           # Save the new outstanding DRP count
                                        #  before any waits are possible
        mulo    CAC0_rq_swi_size,r13,r5 # r5 = Size needed for Request records
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        mov     g1,r14                  # r14 = new ILT for DRP processing
        mulo    CAC0_rs_swi_size,r13,r3 # r3 = Max size needed for Resp records
        addo    CAC0_rq_swi_hdrsz,r5,r10 # r10 = Size needed extended Req data
        addo    CAC0_rs_swi_hdrsz,r3,r9 # r9 = Size needed for extended Res data
        addo    r10,g0,g0               # g0 = Size of the DRP, Datagram
                                        #  Request Header, Datagram Response
                                        #  Header, and Size of extended Request
                                        #  data
        lda     c$setWriteInfoComp,r7   # r7 = DRP Completion routine
        addo    r9,g0,g0                # g0 = Size of the DRP, Datagram
                                        #  Request Header, Datagram Response
                                        #  Header, size of extended request
                                        #  data, and size of extended resp data
        st      r7,il_cr(r14)           # Save the completion routine
        st      g0,il_wcdlmreqsize(r14) # Save the buffer size
c       g0 = s_MallocC(g0, __FILE__, __LINE__); # Get DRP
        ld      C_swi_qht,r8            # r8 = Queue Head (after all possible
                                        #  waits that could change the list)
        st      g0,il_wcdlmdrp(r14)     # Save the DRP buffer address
        st      r8,il_wcdlmphilt(r14)   # Save the beginning of ILT list
#
# --- Build the DRP to request FE DLM to transfer to the partner
#
        ldconst drcachetodlm,r6         # Cache to DLM DRP
        lda     drpsiz(g0),r5           # r5 = pointer to the datagram request
        stos    r6,dr_func(g0)          # Save the DRP Function (Cache to DLM)
        st      r5,dr_req_address(g0)   # Save the datagram request address
        lda     dgrq_size(r10),r7       # r7 = length of datagram request
        st      r7,dr_req_length(g0)    # Save the Request size
        addo    r7,r5,r4                # r4 = Datagram response address
        lda     dgrs_size(r9),r6        # r6 = Datagram Response size
        st      r4,dr_rsp_address(g0)   # Save the datagram response address
        st      r6,dr_rsp_length(g0)    # Save the response size
        ldconst WI_ISSUE_CNT,r6         # Set up max issue cnt due to errors
        ldconst WI_TIME_OUT,r7          # Set up max time before aborting op
        stob    r6,dr_issue_cnt(g0)     # Save the issue count
        stob    r7,dr_timeout(g0)       # Save the time out value (in seconds)
#
#       Set up the Datagram Request Header
#
        ld      K_ficb,r3               # r3 = K_ficb pointer
        ldconst dgrq_size,r4            # r4 = Datagram request header size
        ldconst dg_cpu_interface,r6     # r6 = Request server CPU as FE
        ldos    C_wi_seqnum,r7          # r7 = Datagram sequence number
        ld      fi_mirrorpartner(r3),r11 # r11 = Mirror Partners Serial Number
        stob    r4,dgrq_hdrlen(r5)      # Save the request header size
        addo    1,r7,r7                 # Increment the Datagram Sequence Number
        stob    r6,dgrq_srvcpu(r5)      # Save the FE as the Server CPU
        stos    r7,C_wi_seqnum          # Save the new sequence number
        ldconst CAC0_fc_setwriteinfo,r6 # Write Memory Function Code
        stos    r7,dgrq_seq(r5)         # Save the sequence number in Request
        ldconst dg_path_any,r4          # Take any path available
        stob    r6,dgrq_fc(r5)          # Save the function code
        stob    r4,dgrq_path(r5)        # Any path to the Destination Controller
        ldconst CAC0name,r6             # Show CAC0 is the server name
        bswap   r11,r11                 # Change Endian format
        st      r10,dgrq_reqlen(r5)     # Save extended request info length
        st      r6,dgrq_srvname(r5)     # Save the server name (Cache)
        st      r11,dgrq_dstsn(r5)      # Save the Dest. Controller serial num.
#
#     Set up the extended request information
#
        lda     dgrq_size(r5),r5        # r5 = Extended Request Info pointer
        addo    CAC0_rq_swi_hdrsz,r5,g3 # g3 = Pointer to the record being added
        lda     C_wi_nvac,g1            # g1 = NVA control structure address
        ldconst 0xffffffff,r9           # r9 = Invalid NVA Address constant
        mov     r15,g0                  # g0 = NVA Address
.c_swisend40:
        ld      vrvrp(r8),r6            # r6 = VRP pointer
        ldos    vr_vid(r6),r3           # r3 = VID
        ld      vr_vlen(r6),r4          # r4 = Length in sectors
        ldl     vr_vsda(r6),r6          # r6-r7 = Starting LBA
        subo    1,r13,r13               # Decrement the number left to set up
        st      r8,CAC0_rq_swi_ilt(g3)  # Save the ILT associated with this rec
        st      g0,CAC0_rq_swi_nvaa(g3) # Save the NVA Address
        stos    r3,CAC0_rq_swi_vid(g3)  # Save the VID
        st      r4,CAC0_rq_swi_len(g3)  # Save the number of sectors written
        stl     r6,CAC0_rq_swi_slba(g3) # Save the Starting LBA
#
#       Determine if there is more work and more NVRAM space for the records
#
        cmpobe  0,r13,.c_swisend50      # Jif no more work to be done
        call    M$anvanw                # g0 = Address
c       if (MP_IS_BIGFOOT) g0 = g0+NVRAM_P4_START_BF;
        cmpobe  g0,r9,.c_swisend50      # Jif no NVA address is available
        addo    CAC0_rq_swi_size,g3,g3  # Point to the next record to fill in
        ld      il_fthd(r8),r8          # Point to the next ILT on the list
        addo    1,g2,g2                 # Increment the number of records in DRP
#c       fprintf(stderr, ".c_swisend40: No. of records in DRP = %d\n", (UINT32)g2);
        b       .c_swisend40            # Continue onto the next record
#
.c_swisend50:
# c       fprintf(stderr, ".c_swisend50: No NVA address available\n");
        ld      il_fthd(r8),r10         # r10 = Next possible ILT on the Queue
        ld      C_swi_qht+4,r11         # r11 = Tail of the queue
        lda     ILTBIAS(r14),g1         # Point to the next level of DRP ILT
#      c fprintf(stderr,"CACHEBUG:sending DRP ILT = 0x%X\n",(UINT32)g1);
        cmpo    0,r10                   # Is this the last ILT on the queue?
        sele    r11,0,r11               # Clear Tail pointer if no more on queue
        be      .c_swisend60            # Jif there are no more ILTs on list
        st      r12,il_bthd(r10)        # Clear the backward thread pointer for
                                        #  the new top of the ILT list
.c_swisend60:
        ld      C_swi_cqd,r3            # Get the current Queue Depth (may have
                                        #  changed because of waits above)
        subo    g2,r3,r3                # Subtract how many actually sent from
                                        #  current queue depth
        st      r12,il_fthd(r8)         # Clear last ILT's forward thread ptr
        st      r3,C_swi_cqd            # Save the new current queue depth
        st      g2,CAC0_rq_swi_num(r5)  # Save the number of records in list
        st      r10,C_swi_qht+qu_head   # Save the new queue head
        st      r11,C_swi_qht+qu_tail   # Save the new queue tail
#
# --- Send the data to the partner
#
        st      r12,il_fthd(g1)         # Clear the ILT Forward pointer
        call    DLM$quedrp              # Queue the DRP to DLM
#
# --- All done
#
.c_swisend99:
        PopRegsVoid                     # Restore all G registers (stack relative)
.c_swisend100:
        ret
#
#**********************************************************************
#
#  NAME: c$setWriteInfoErrorRestart
#
#  PURPOSE:
#       To set up and begin the transfer of Write Information to the
#           mirror partner
#
#  DESCRIPTION:
#       This routine sets up and then calls the FE DLM to transfer the
#       Write information (VID, LBA, Length) to the mirror partner to aid
#       in reducing the time to resync on failover.
#
#  CALLING SEQUENCE:
#       call    c$setWriteInfoErrorRestart
#
#  INPUT:
#       g1 = ILT associated with write
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0-g1
#
#**********************************************************************
#
c$setWriteInfoErrorRestart:
#
# --- Determine if the Mirror Partner is talking and if so, is it the same
#       Mirror Partner as this op requests.
#
        lda     C_ca,r3                 # r3 = The Cache Information pointer
        lda     -ILTBIAS(g1),r12        # g1 = ILT at the previous level
        ldob    ca_status(r3),r4        # r4 = The Cache Status
        ldconst 0,r15                   # r15 = Clearing Register
        ld      il_wcdlmdrp(r12),r7     # r7 = Address of the DRP
        bbs     ca_mirrorbroken,r4,.c_swierrrestart30 # Jif no Mirror Partner
#
        ld      K_ficb,r5               # r5 = K_ficb pointer
        ld      dr_req_address(r7),r8   # r8 = Address of the Datagram Request
        ld      fi_mirrorpartner(r5),r6 # r6 = Mirror Partners Serial Number
        ld      dgrq_dstsn(r8),r9       # r9 = Destination Controller Serial #
        bswap   r6,r6                   # Change to show the correct format
        cmpobne r6,r9,.c_swierrrestart30 # Jif it is a different Controller
#
# --- Able to retry the operation again
#
        st      r15,il_fthd(g1)         # Clear the ILT forward pointer
        call    DLM$quedrp              # Queue the DRP to DLM
        b       .c_swierrrestart100     # All done - return.
#
# --- Not able to mirror due to a broken link or different Mirror Partner.
#       Dequeue the ILTs in the list and forward to the lower layers to complete
#       like no Write Information was mirrored.  Then free the ILT, DRP, etc.
#
.c_swierrrestart30:
        ld      il_wcdlmphilt(r12),r4   # r4 = List of ILTs to forward
.c_swierrrestart40:
        mov     r4,g1                   # g1 = ILT to send to the next layer
        ld      il_fthd(r4),r4          # Get the next ILT in the list
        st      r15,il_fthd(g1)         # Clear the forward pointer
#
#   Send the ILT to the next layer
#
        call    CA_CallLowerSend
#
#   Continue looping if more ILTs are on the list.  If all done, free the
#       ILT, DRP, Datagram, etc.  Then update the outstanding DRP counter.
#
        cmpobne 0,r4,.c_swierrrestart40 # Jif more ILTs to work on
#
#       Need to clear out all the NVA Entries that were used on the Mirror
#       Partner (those will be taken care of when the Mirror Partner comes back
#       online).
#
        lda     drpsiz+dgrq_size(r7),r3 # Point to the extended request records
        ld      CAC0_rq_swi_num(r3),r4  # r4 = Number of records to free
        lda     CAC0_rq_swi_hdrsz(r3),r3 # r3 = Point to list of records
        lda     C_wi_nvac,g1            # Point to the Cache mirror of P4 NVAC
.c_swierrrestart45:
        ld      CAC0_rq_swi_nvaa(r3),g0 # g0 = NVA Address to free
c        if (MP_IS_BIGFOOT) g0 = g0-NVRAM_P4_START_BF;
.ifdef    NVA_DEBUG
c fprintf(stderr,"%s%s:%u 3.NVA offset passed to M$rnvaa: %x\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif   # NVA_DEBUG
        call    M$rnvaa                 # Free the address
        cmpdeco 1,r4,r4                 # Decrement the number left to do
        lda     CAC0_rq_swi_size(r3),r3 # r3 = Next SWI Record
        bne     .c_swierrrestart45      # Jif not done yet
#
        mov     r7,g0                   # g0 = Address of the DRP
        ld      il_wcdlmreqsize(r12),g1 # g1 = Size to free
        ld      C_swi_cdrp,r3           # r3 = Outstanding DRP count
c       s_Free(g0, g1, __FILE__, __LINE__); # Free the DRP and Datagram area
        mov     r12,g1                  # g1 = ILT to Free
        subo    1,r3,r3                 # Decrement the outstanding DRP count
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        st      r3,C_swi_cdrp           # Save the new outstanding DRP counter
#
# --- Now clear out any ILTs that are queued up to be mirrored
#
.c_swierrrestart50:
        ld      C_swi_cqd,r3            # Get current queue depth
        cmpobe  0,r3,.c_swierrrestart100 # Jif none
#
        ld      C_swi_qht+qu_head,r4    # Get next queue head
        ld      C_swi_qht+qu_tail,r5    # Get next queue tail
        subo    1,r3,r3                 # Adjust current queue depth
        st      r3,C_swi_cqd
#
# --- Dequeue selected request (FIFO fashion)
#
        mov     r4,g1                   # Isolate queued ILT
#
        ld      il_fthd(r4),r4          # Dequeue ILT
        cmpo    0,r4                    # Update queue head/tail
        sele    r5,0,r5
        st      r4,C_swi_qht+qu_head
        st      r5,C_swi_qht+qu_tail
        st      r15,il_fthd(g1)         # Clear the forward pointer
#
#   Send the ILT to the next layer
#
        call    CA_CallLowerSend
#
#   Continue looping to clear out all the ILTs on the list.
#
        b       .c_swierrrestart50      # Jif to test for more work
#
# --- All Done
#
.c_swierrrestart100:
        ret
#
#**********************************************************************
#
#  NAME: c$setWriteInfoComp
#
#  PURPOSE:
#       To handle the Set Write Information to the mirror partner completion
#
#  DESCRIPTION:
#       This routine handles the completion of the Set Write Information
#
#  CALLING SEQUENCE:
#       call    c$setWriteInfoComp
#
#  INPUT:
#       g0 = ILT Return Status
#       g1 = ILT associated with the mirror of write information
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
c$setWriteInfoComp:
        mov     g0,r14                  # Save g0-g1
        mov     g1,r15
#        lda ILTBIAS(g1), r3
#       c fprintf(stderr,"CACHEBUG:setWriteInfoCom received ILT= 0x%X\n", (UINT32)r3);
#
# --- Get the status of the Mirror Request.  If an error occurred, report it
#       to the CCB and put the original request on the Error Queue waiting for
#       instructions from the CCB.
#
        ld      il_wcdlmdrp(g1),r3      # r3 = Address of the DRP
        ldconst 0,r10                   # r10 = Clearing register
        ld      dr_req_address(r3),r5   # r5 = Address of the Datagram Request
        ld      dr_rsp_address(r3),r6   # r6 = Address of the Datagram Response
        ldob    dr_status(r3),r11       # r11 = DRP status
        ldob    dgrs_status(r6),r12     # r12 = Datagram Response status
        ld      il_wcdlmphilt(g1),r13   # r13 = original list of ILT(s)
        cmpobne 0,g0,.swidrpcomp200     # Jif there was an ILT error
        cmpobne 0,r11,.swidrpcomp200    # Jif there was a DRP error
        cmpobe  dg_st_ok,r12,.swidrpcomp900 # Jif all was good
#
# --- Send message to browser about the error being found, if it has not
#       already been reported.
#
.swidrpcomp200:
                                        # g0 = ILT Error Status
                                        # g1 = ILT of Datagram
        call    c$logMirrorFailure      # Log the mirror failure
#
# --- The mirror failed - set the error flag, free the mirror resources, and
#       put the original op on the Error Queue waiting for instructions
#       from the CCB.
#
.if WC_MIRROR_ERROR_DISABLE
        mov     r13,r11                 # r11 = Beginning of ILTs that were lost
        b       .swidrpcomp980          # Go send list of ILTs to lower layer
.else   # WC_MIRROR_ERROR_DISABLE
        ld      dgrq_dstsn(r5),r3       # r3 = Controller that Failed the DG
        ldob    C_ca+ca_status,g0       # g0 = The Cache Status
        bswap   r3,r3                   # Restore to normal Controller SN
        setbit  ca_error,g0,g0          # Set the flag saying in Error State
        st      r3,gWCErrorMP           # Save the failing Controller SN
        stob    g0,C_ca+ca_status       # Save the new status
        lda     ILTBIAS(g1),g1          # g1 = original ILT at the next ILT lvl
        lda     c$setWriteInfoErrorRestart,r3 # Routine to call once the error
        st      r3,il_cr(g1)            #        has been cleared up
        call    c$qerror                # Queue this op until the error is fixed
        b       .swidrpcomp1000         # All done
.endif  # WC_MIRROR_ERROR_DISABLE
#
# --- The mirror completed successfully, send the original request(s) to the
#       lower layers.
#
.swidrpcomp900:
        ldconst FALSE,r3                # Show that remote mirror worked
        st      r3,C_mirror_error_flag  # Save the remote mirror worked flag
        lda     dgrs_size(r6),r7        # r7 = extended response info pointer
        ld      CAC0_rs_swi_num(r7),r12 # r12 = number of response records
        lda     c$clearWriteInfo,r4     # r4 = Clear the Record when write done
        lda     CAC0_rs_swi_hdrsz(r7),r7 # Point to the response records
#
#   Search the list of ILTs in the list that matches for this record.  Remove
#       the match from the list.  If none is found, report the error and
#       continue with the next response record in the list.
#
.swidrpcomp910:
        ld      CAC0_rs_swi_ilt(r7),r9  # r9 = ILT WI Record is for
        cmpobe  0,r9,.swidrpcomp970     # record is empty goto next
        mov     r13,r11                 # r11 = Test ILT for Match
.swidrpcomp920:
        cmpobe  r11,r9,.swidrpcomp940   # Jif a matching ILT is found
        cmpobne 0,r11,.swidrpcomp930    # Jif more ILTs to search through
#
#   Something is wrong with the list of response data - an ILT could not be
#       found that should have been matched.  Free the local NVA Address, Log
#       the error and continue to the next item in the list.
#
        ld      CAC0_rs_swi_nvaa(r7),g0 # g0 = NVA Address
        lda     C_wi_nvac,g1            # g1 = The NVA Control Structure
c       if (MP_IS_BIGFOOT) g0 = g0 - NVRAM_P4_START_BF;
.ifdef NVA_DEBUG
c fprintf(stderr,"%s%s:%u 4.NVA offset passed to M$rnvaa: %x\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # NVA_DEBUG
        call    M$rnvaa                 # Remove the NVA Address
        mov     g0,r8                   # Save the NVA address
#
        ldconst cac_sft4,r3             # r11 = error code to log
        lda     C_sft_flt,g0            # g0 = Software Fault Log Area
        st      r3,efa_ec(g0)           # Save the Error Code
        st      r9,efa_data(g0)         # Save the Missing ILT
        st      r8,efa_data+4(g0)       # Save the WI Record ID on the Mirror
        ldconst 12,r3                   # Number of bytes saved (ec + data)
        st      r3,mle_len(g0)          # Save the number of bytes to send
        call    M$soft_flt              # Error Trap or Log failure
        b       .swidrpcomp970          # Continue with the next entry
#
#   Continue looking for the matching ILT
#
.swidrpcomp930:
        ld      il_fthd(r11),r11        # Get next ILT on the list
        b       .swidrpcomp920          # Keep looking for a match
#
#   Found a match, remove it from the list
#
.swidrpcomp940:
        ld      il_fthd(r11),r8         # r8 = Next ILT in the list
        ld      il_bthd(r11),r9         # r9 = Previous ILT in the list
        ld      vrvrp(r11),r6           # r6 = Original VRP
        cmpobe  0,r8,.swidrpcomp950     # Jif this is the tail of the list
        st      r9,il_bthd(r8)          # Set next ILTs previous pointer to what
                                        #  is in this ILTs previous pointer
.swidrpcomp950:
        cmpobe  0,r9,.swidrpcomp960     # Jif this is the head of the list
        st      r8,il_fthd(r9)          # Set previous ILTs next pointer to what
                                        #  is in this ILTs next pointer
.swidrpcomp960:
        sele    r13,r8,r13              # Reset head pointer if head was removed
#
#   Set up the ILT and then send on to the next layer to process
#
        ld      CAC0_rs_swi_nvaa(r7),r8 # r8 = NVA Address
        lda     ILTBIAS(r11),g1         # Move down one ILT layer
        st      r4,il_cr(g1)            # Save the completion routine
        st      r10,il_fthd(g1)         # Clear the forward pointer in this ILT
        st      r6,vrvrp(g1)            # Save the VRP in this ILT level
        st      r8,il_wcdlmnvaa(g1)     # Save the NVA Address
#
#   Send the original write VRP to the lower layer
#
        call    CA_CallLowerSend
#
#   Continue looping through all the response records.
#
.swidrpcomp970:
        subo    1,r12,r12               # Decrement the number of records left
        lda     CAC0_rs_swi_size(r7),r7 # Bump the Response Record Pointer
        cmpobne 0,r12,.swidrpcomp910    # Jif there is more work to be done
#
# --- Verify the list is down to the last.  If not, log a message and set up to
#       continue any ILTs that did not get a mirror record associated with them.
#
        cmpobe  0,r13,.swidrpcomp990    # Jif nothing is on the list (good)
#
#   Not all ILTs had an associated Record ID.  Forward the ILT(s) to the next
#       layer without any clearing of the write info and then log a message.
#
        mov     r13,r11                 # r11 = Beginning of ILTs that were lost
.swidrpcomp980:
        mov     r13,g1                  # g1 = ILT to forward
        ld      il_fthd(r13),r13        # Get the next ILT on the List
#
#   Send the original write VRP to the lower layer
#
        call    CA_CallLowerSend
#
        addo    1,r12,r12               # Bump the ILTs in Lost List count
        cmpobne 0,r13,.swidrpcomp980    # Jif more items are on the list
#
.if WC_MIRROR_ERROR_DISABLE
#   Do not log anything when hiding errors
.else   # WC_MIRROR_ERROR_DISABLE
        ldconst cac_sft5,r3             # r3 = error code to log
        lda     C_sft_flt,g0            # g0 = Software Fault Log Area
        st      r3,efa_ec(g0)           # Save the Error Code
        st      r12,efa_data(g0)        # Save the number on the list
        st      r11,efa_data+4(g0)      # Save the original lost ILT head
        ldconst 12,r3                   # Number of bytes saved (ec + data)
        st      r3,mle_len(g0)          # Save the number of bytes to send
        call    M$soft_flt              # Error Trap or Log failure
.endif  # WC_MIRROR_ERROR_DISABLE
#
# --- Done through the loop, now free the DRP, Datagram, and ILT used to mirror
#       the information.  Drop the count of outstanding DRPs and send off
#       another request if there are any.
#
.swidrpcomp990:
        ld      il_wcdlmdrp(r15),g0     # g0 = Address of the DRP
        ld      il_wcdlmreqsize(r15),g1 # g1 = Size to free
        ld      C_swi_cdrp,r3           # r3 = The Outstanding DRP counter
c       s_Free(g0, g1, __FILE__, __LINE__); # Free the DRP and Datagram area
        mov     r15,g1                  # Free the ILT
        subo    1,r3,r3                 # Decrement the count
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
#
        st      r3,C_swi_cdrp           # Save the outstanding DRP counter
        call    c$setWriteInfoSend      # Try and send any outstanding ops
#
# --- All Done
#
.swidrpcomp1000:
        mov     r14,g0                  # restore g0-g1
        mov     r15,g1
        ret
#
#**********************************************************************
#
#  NAME: c$setWriteInfoKick
#
#  PURPOSE:
#       Function to kick off a task that will kick start the Set Write
#       Info engine off.
#
#  DESCRIPTION:
#       This function will call kick off a task to start the Set Write Info
#       engine off.  Allows multiple callers to kick off the task without
#       worrying why or when it needs to happen.
#
#  CALLING SEQUENCE:
#       call    c$setWriteInfoKick
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
c$setWriteInfoKick:
        mov     g0,r14                  # Save g0 and g1
        mov     g1,r15
#
# --- If the Set Write Info has ops queued and there are no DRPs in progress,
#       we could have run into a case where the NVA was full and could not
#       send a DRP.  Kick off a task to start the Set Write Info ops (need
#       to start a task because the start could end up swapping due to
#       resource constraints which could hang since this could be a
#       completion routine).
#
        ld      C_swi_cqd,r3            # Get the current Set Write Info count
        ld      C_swi_cdrp,r4           # Get the current SWI DRP count
        ld      C_swi_kick_pcb,r5       # Determine if the task is already going
        cmpobe  0,r3,.c_swikick_100     # Jif there is no work to be done
        cmpobne 0,r4,.c_swikick_100     # Jif there are outstanding DRPs
        cmpobne 0,r5,.c_swikick_100     # Jif the task is running already
c       g0 = -1;                        # Flag process being started.
        st      g0,C_swi_kick_pcb       # Flag trying to be started.
        lda     c$setWriteInfoKickStart,g0 # Work needs to be done, kick off the
        ldconst CMIRRORPRI,g1           #  task to enable Set Write Info ops
c       CT_fork_tmp = (ulong)"c$setWriteInfoKickStart";
        call    K$tfork                 #  to continue
        st      g0,C_swi_kick_pcb       # Save the PCB to only start one task
.c_swikick_100:
        mov     r14,g0                  # Restore g0 and g1
        mov     r15,g1
        ret
#
#**********************************************************************
#
#  NAME: c$setWriteInfoKickStart
#
#  PURPOSE:
#       To kick start the Set Write Info engine off.
#
#  DESCRIPTION:
#       This task calls the Set Write Info engine to get mirroring of Write
#       Information back in progress.  This task is in place to be called by
#       completion routines to avoid hanging the system because the Set Write
#       Information engine can pause waiting for resources.
#
#  CALLING SEQUENCE:
#       Process call
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
c$setWriteInfoKickStart:
        ldconst 0,r3
        call    c$setWriteInfoSend      # Call the engine to start mirroring
                                        #  Write Information
        st      r3,C_swi_kick_pcb       # Clear the PCB to allow starting again
        ret
#
#**********************************************************************
#
#  NAME: c$clearWriteInfo
#
#  PURPOSE:
#       To set up and begin the clearing of Write Information to the
#           mirror partner
#
#  DESCRIPTION:
#       This routine sets up and then calls the FE DLM to clear the
#       Write information of a previous Set Write Information to the mirror
#       partner to aid in reducing the time to resync on failover. The original
#       Write ILT is completed before the mirror is done to improve performance
#       in this path.
#
#  CALLING SEQUENCE:
#       call    c$clearWriteInfo
#
#  INPUT:
#       g0 = Write Completion Status
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
c$clearWriteInfo:
        mov     g0,r14                  # Save g0-g1
        mov     g1,r15
#
# --- Get a Placeholder ILT to save the Record ID that needs to be cleared.
#       Queue this and the finish the original operation
#
        ld      il_wcdlmnvaa(g1),r3     # r3 = NVA Address
c       g1 = get_wc_plholder();         # Get Placeholder for the queue.
        st      r3,il_wcdlmnvaa(g1)     # Save the NVA Address in the
                                        #  Placeholder ILT
#
#   Insert the Placeholder ILT into executive queue
#
        mov     0,r13                   # r13 = Clearing register
        ld      C_cwi_cqd,r7            # r7 = Current queue depth
        ld      C_cwi_qht+qu_head,r4    # r4 = Queue head
        ld      C_cwi_qht+qu_tail,r5    # r5 = Queue tail
        cmpobne 0,r7,.c_cwi_120         # Jif queue not empty
#
#   Insert into empty queue
#
        mov     g1,r5                   # Update queue head/tail with
        mov     g1,r4                   #  single entry
        b       .c_cwi_140
#
#   Insert into non-empty queue
#
.c_cwi_120:
        st      g1,il_fthd(r5)          # Append ILT to end of queue
        mov     g1,r5                   # Update queue tail
.c_cwi_140:
        addo    1,r7,r7                 # Bump queue depth
        st      r4,C_cwi_qht+qu_head
        st      r5,C_cwi_qht+qu_tail
        st      r7,C_cwi_cqd
        st      r13,il_fthd(g1)         # Clear Fwd Pointer for this ILT
#
#   If at the point where the records need to be sent, then activate the task
#     to start the process
#
        ld      C_cwi_pcb,r6            # r6 = Clear Write Info Task pcb
        ldob    pc_stat(r6),r3          # r3 = Current process status
        ldconst pcrdy,r8                # r8 = Process ready status
        cmpobne pcnrdy,r3,.c_cwi_200    # Jif status is not Not Ready (already
                                        #  active)
.ifdef HISTORY_KEEP
c CT_history_pcb(".c_cwi_140 setting ready pcb", r6);
.endif  # HISTORY_KEEP
        stob    r8,pc_stat(r6)          # Ready process
#
# --- Return the original Write Return Code to the host (done here to improve
#       performance at the host when mirroring write information)
#
.c_cwi_200:
        mov     r14,g0                  # g0 = Write Return Code
        mov     r15,g1                  # g1 = the original ILT
        call    K$comp                  # Complete this op to the next level
        mov     r14,g0                  # g0 = Write Return Code
        mov     r15,g1                  # g1 = the original ILT
        ret
#
#**********************************************************************
#
#  NAME: c$clearWriteInfoComp
#
#  PURPOSE:
#       To handle the Clear Write Information completion from the
#           mirror partner
#
#  DESCRIPTION:
#       This routine handles the completion of the Clear Write Information
#       request to the mirror partner
#
#  CALLING SEQUENCE:
#       call    c$clearWriteInfoComp
#
#  INPUT:
#       g0 = ILT Completion Status
#       g1 = ILT associated with the mirror of write information
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
c$clearWriteInfoComp:
#
# --- Get the status of the Mirror Request.  If an error occurred, report it
#       to the CCB
#
        ld      il_wcdlmdrp(g1),r3      # r3 = Address of the DRP
        ld      dr_req_address(r3),r5   # r5 = Address of the Datagram Request
        ld      dr_rsp_address(r3),r6   # r6 = Address of the Datagram Response
        ldob    dr_status(r3),r11       # r11 = DRP status
        ldob    dgrs_status(r6),r12     # r12 = Datagram Response status
        cmpobne 0,g0,.cwidrpcomp200     # Jif there was an ILT error
        cmpobne 0,r11,.cwidrpcomp200    # Jif there was a DRP error
        cmpobe  dg_st_ok,r12,.cwidrpcomp500 # Jif all was good
#
# --- Send message to browser about the error being found, if it has not
#       already been reported.
#
.cwidrpcomp200:
                                        # g0 = ILT Error Status
                                        # g1 = ILT of Datagram
        call    c$logMirrorFailure      # Log the mirror failure
#
# --- Mirror Failed - Set the Error Flag and put the clearing info op on the
#       error queue waiting for CCB action
#
.if WC_MIRROR_ERROR_DISABLE
        mov     0,g0                    # Do not show any errors to next layers
        b       .cwidrpcomp500          # Go treat like it worked
.else   # WC_MIRROR_ERROR_DISABLE
        mov     g0,r14                  # save g0-g1
        mov     g1,r15
        ldob    C_ca+ca_status,g0       # Get the Cache Status
        ld      dgrq_dstsn(r5),r13      # Get the Controller that Failed the DG
        lda     ILTBIAS(r15),g1         # g1 = ILT at next level
        setbit  ca_error,g0,g0          # Set the flag saying in Error State
        lda     c$clearWriteInfoRestart,r3 # r3 = Restart routine when OK again
        bswap   r13,r13                 # Return to a normal Controller SN
        stob    g0,C_ca+ca_status       # Save the new status
        st      r3,il_cr(g1)            # Save the Restart Address
        st      r13,gWCErrorMP          # Save the Failing Controller SN
        call    c$qerror                # Queue the Op to the Error Queue
        b       .cwidrpcomp1000         # All Done!
.endif  # WC_MIRROR_ERROR_DISABLE
#
# --- Clear out the NVA Address's from the local copy of the Mirror Partner's
#       P4 NVA
#
.cwidrpcomp500:
        ldconst FALSE,r15               # Show that a remote mirror worked
        st      r15,C_mirror_error_flag
        mov     g0,r14                  # save g0
        lda     dgrq_size(r5),r3        # r3 = Extended Request data
        mov     g1,r15                  # save g1
        ld      CAC0_rq_cwi_num(r3),r4  # r4 = Number of records to free
        addo    4,r3,r3                 # r3 = Pointer to record
        lda     C_wi_nvac,g1            # g1 = Cache mirror of P4 NVAC
.cwidrpcomp520:
        ld      (r3),g0                 # g0 = NVA Address
        addo    4,r3,r3                 # r3 = Pointer to next record
c       if (MP_IS_BIGFOOT) g0 = g0 - NVRAM_P4_START_BF;
.ifdef    NVA_DEBUG
c fprintf(stderr,"%s%s:%u 5.NVA offset passed to M$rnvaa: %x\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif  #  NVA_DEBUG
        call    M$rnvaa                 # Free the NVA Address
        cmpdeco 1,r4,r4                 # Decrement the number left to do
        bne     .cwidrpcomp520          # Jif more NVA Address's to free
#
# --- Free the ILT and DRP
#
        ld      il_wcdlmdrp(r15),g0     # g0 = Address of the DRP
        ld      il_wcdlmreqsize(r15),g1 # Size to free
c       s_Free(g0, g1, __FILE__, __LINE__); # Free the DRP and Datagram area
        mov     r15,g1                  # Free the ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
#
# --- Decrement the outstanding DRP count and if the number waiting
#       is > cutoff, activate the task to send another DRP
#
        ld      C_cwi_cdrp,r3           # r3 = Outstanding DRP count
        ld      C_cwi_cqd,r4            # r4 = Current number of waiting ops
        subo    1,r3,r3                 # Decrement the outstanding DRP count
        ld      C_cwi_pcb,r6            # r6 = Clear Write Info Task pcb
        st      r3,C_cwi_cdrp           # Save the new outstanding DRP counter
        cmpobe  0,r4,.cwidrpcomp600     # Jif nothing left to send
        ldob    pc_stat(r6),r3          # r3 = Current process status
        ldconst pcrdy,r8                # r8 = Process ready status
        cmpobne pcnrdy,r3,.cwidrpcomp600 # Jif status is not Not Ready -
                                        #   already active
.ifdef HISTORY_KEEP
c CT_history_pcb(".cwidrpcomp520 setting ready pcb", r6);
.endif  # HISTORY_KEEP
        stob    r8,pc_stat(r6)          # Ready process
#
# --- Cleared some NVA records.  Call the routine to see if the Set Write Info
#       Engine needs kick started.
#
.cwidrpcomp600:
        call    c$setWriteInfoKick      # Go see if the SWI needs to be started
#
# --- All Done
#
.cwidrpcomp1000:
        mov     r14,g0                  # Restore g0
        mov     r15,g1                  # Restore g1
        ret
#
#**********************************************************************
#
#  NAME: c$clearWriteInfoRestart
#
#  PURPOSE:
#       To set up and begin the clearing of Write Information to the
#           mirror partner after an error
#
#  DESCRIPTION:
#       This routine checks to see if the Mirror Partner is talking or not.
#       If not, it throws away the Clearing information (unable to clear).
#       If so, re-issues the op to the Mirror Partner.
#
#  CALLING SEQUENCE:
#       call    c$clearWriteInfoRestart
#
#  INPUT:
#       g1 = ILT associated with Clear Write Infor DRP
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0-g1
#
#**********************************************************************
#
c$clearWriteInfoRestart:
#
# --- Determine if the Mirror Partner is talking and if so, is it the same
#       Mirror Partner as this op requests.
        lda     -ILTBIAS(g1),r12        # r12 = ILT at the previous level
        lda     C_ca,r3                 # r3 = The Cache Information pointer
        ld      il_wcdlmdrp(r12),r7     # r7 = Address of the DRP
        ldob    ca_status(r3),r4        # r4 = The Cache Status
        ldconst 0,r15                   # r15 = Clearing Register
        ld      dr_req_address(r7),r8   # r8 = Address of the Datagram Request
        bbs     ca_mirrorbroken,r4,.c_cwirestart50 # Jif no Mirror Partner
#
        ld      K_ficb,r5               # r5 = K_ficb pointer
        ld      fi_mirrorpartner(r5),r6 # r6 = Mirror Partners Serial Number
        ld      dgrq_dstsn(r8),r9       # r9 = Destination Controller Serial #
        bswap   r6,r6                   # Change to show the correct format
        cmpobne r6,r9,.c_cwirestart50   # Jif it is a different Controller
#
# --- Able to retry the operation again
#
        st      r15,il_fthd(g1)         # Clear the ILT forward pointer
        call    DLM$quedrp              # Queue the DRP to DLM
        b       .c_cwirestart100        # All done - return.
#
# --- Not able to mirror due to a broken link or different Mirror Partner.
#       Free the NVA Address from the Cache mirror of the Mirror Partners
#       P4 NVAC and then free the ILT, DRP, etc. and exit.
#
.c_cwirestart50:
        lda     dgrq_size(r8),r3        # r3 = Extended Request data
        ld      CAC0_rq_cwi_num(r3),r4  # r4 = Number of records to free
        addo    4,r3,r3                 # r3 = Pointer to record
        lda     C_wi_nvac,g1            # g1 = Cache mirror of P4 NVAC
.c_cwirestart60:
        ld      (r3),g0                 # g0 = NVA Address
        addo    4,r3,r3                 # r3 = Pointer to next record
c       if (MP_IS_BIGFOOT) g0 = g0-NVRAM_P4_START_BF;
.ifdef    NVA_DEBUG
c fprintf(stderr,"%s%s:%u 6.NVA offset passed to M$rnvaa: %x\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif   # NVA_DEBUG
        call    M$rnvaa                 # Free the NVA Address
        cmpdeco 1,r4,r4                 # Decrement the number left to do
        bne     .c_cwirestart60         # Jif more NVA Address's to free
#
        mov     r7,g0                   # g0 = DRP, DG, etc. area to free
        ld      il_wcdlmreqsize(r12),g1 # g1 = Size to free
        ld      C_cwi_cdrp,r3           # r3 = Outstanding DRP count
c       s_Free(g0, g1, __FILE__, __LINE__); # Free the DRP and Datagram area
        mov     r12,g1                  # g1 = The ILT to free
        subo    1,r3,r3                 # Decrement the outstanding DRP count
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        st      r3,C_cwi_cdrp           # Save the new outstanding DRP counter
#
# --- Kick off the Clear Write Info Task if items are queued up to process
#
        ld      C_cwi_cqd,r4            # r4 = Current number of waiting ops
        ld      C_cwi_pcb,r6            # r6 = Clear Write Info Task pcb
        cmpobe  0,r4,.c_cwirestart70    # Jif nothing left to send
        ldob    pc_stat(r6),r3          # r3 = Current process status
        ldconst pcrdy,r8                # r8 = Process ready status
        cmpobne pcnrdy,r3,.c_cwirestart70 # Jif status is not Not Ready -
                                        #   already active
.ifdef HISTORY_KEEP
c CT_history_pcb(".c_cwirestart60 setting ready pcb", r6);
.endif  # HISTORY_KEEP
        stob    r8,pc_stat(r6)          # Ready process
#
# --- Cleared some NVA records.  Call the routine to see if the Set Write Info
#       Engine needs kick started.
#
.c_cwirestart70:
        call    c$setWriteInfoKick      # Go see if the SWI needs to be started
#
# --- All Done
#
.c_cwirestart100:
        ret
#
#**********************************************************************
#
#  NAME: c$clearCacheCounters
#
#  PURPOSE:
#       Clears all the Write Cache Counters for the specified VCD or all the
#           VCDs that have cache enabled.
#
#  DESCRIPTION:
#       This routine will clear all the Write Cache counters for a specified
#       VCD or all the VCDs that have cache enabled.  This allows those that
#       are doing performance testing to know what the Hit ratios are for
#       specific tests.
#
#  CALLING SEQUENCE:
#       call    c$clearCacheCounters
#
#  INPUT:
#       g0 = VCD pointer or 0xFFFFFFFF if all VCDs with cache enabled are
#               to be reset
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#  REGISTERS USED:
#       r3  = Temporary register
#       r10 = Current VCD pointer
#       r11 = VCD index number
#       r12 - r15 = Zero (used in clearing the counters)
#
#**********************************************************************
#
c$clearCacheCounters:
        ldconst 0,r11                   # r11 = VCD Index pointer initialized
                                        #   for a single VCD
        mov     g0,r10                  # r10 = Current VCD being cleared
        ldconst 0xFFFFFFFF,r3           # r3 = All VCDs constant
        movq    0,r12                   # r12-r15 = Clear registers
        cmpobne r3,r10,.cccc200         # Jif this is a specific VCD
#
# --- Loop through all the VCDs looking for cache enabled status and clear
#       the counters
#
        ldconst MAXVIRTUALS,r11         # Set the index counter to one beyond
                                        #   the limit
.cccc100:
        subo    1,r11,r11               # Point to the previous VCD
        ld      vcdIndex[r11*4],r10     # r10 = VCD pointer
        cmpobe  0,r10,.cccc900          # Jif no VCD at this location
        ldob    vc_stat(r10),r3         # r3 = VCD Status
        bbc     vc_cached,r3,.cccc900   # Jif this VCD is not caching
#
# --- Clear all the Cache counters for a specified VID
#
.cccc200:
        stl     r12,vc_rdhits(r10)      # Clear the Read Hit Counter
        stl     r14,vc_rdpart(r10)      # Clear the Read Partial Hit Counter
        stl     r12,vc_rdmiss(r10)      # Clear the Read Miss Counter
        stl     r14,vc_wrhits(r10)      # Clear the Write Hit Counter
        stl     r12,vc_wrpart(r10)      # Clear the Write Partial Hit Counter
        stl     r14,vc_wrmiss(r10)      # Clear the Write Miss Counter
        stl     r12,vc_wrtbyres(r10)    # Clear the Bypass due to Resources Cntr
        stl     r14,vc_wrtbylen(r10)    # Clear the Bypass due to Length Counter
.cccc900:
        cmpobne 0,r11,.cccc100          # Jif more VCDs to do
        ret
#
#**********************************************************************
#
#  NAME: C$queryCacheData
#
#  PURPOSE:
#       Determines if any data exists in Cache.
#
#  DESCRIPTION:
#       This routine will check out the Write Cache to determine if any data
#       exists in the Write Cache.
#
#  CALLING SEQUENCE:
#       call    C$queryCacheData
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g0 = TRUE if data is going into or is in the Write Cache
#          = FALSE if no data exists in cache
#
#  REGS DESTROYED:
#       g0
#
#  REGISTERS USED:
#       r3  =   Cache Information pointer
#       r4  =   Current VID
#       r5  =   Maximum VID + 1
#       r6  =   VCD pointer
#       r7  =   Tag Address being checked
#       r8  =   End of the Tag Address Range
#       r9  =   Tag size
#       r10 =   Temporary
#       r11 =   Temporary
#       r13 =   VCD Status
#       r14 =   VCD Count of Writes going into cache
#       r15 =   VCD Cache Tree Root pointer
#
#**********************************************************************
#
C$queryCacheData:
.ifdef HISTORY_KEEP
c CT_HISTORY_OFF();
.endif  # HISTORY_KEEP
#
# --- Determine first if Cache is initialized yet for further testing
#
        ldos    K_ii+ii_status,r15      # r15 = Initialization status
        ldconst TRUE,g0                 # Set return to show data could exist
        ldconst MAXVIRTUALS,r5          # r5 = maximum VID + 1
        bbs     iicinit,r15,.cqcd_50    # Jif Cache is initialized
#
#   Cache is not initialized yet.  Walk through the Tags to determine if data
#       exists or not.
#
        ld      WctAddr,r7              # r7 = Pointer to cache tag
        ld      WctSize,r8              # r8 = Size of the cache tag area
        ldconst tgsize,r9               # r9 = Tag incrementer value
        addo    r7,r8,r8                # r8 = End of cache tag area
.cqcd_20:
        ldos    tg_vid(r7),r10          # r10 = VID associated with this tag
        ldos    tg_attrib(r7),r11       # r11 = Tag Attributes
        cmpobge r10,r5,.cqcd_30         # Jif VID is out of range - ignore
        bbs     TG_DIRTY,r11,.cqcd_90   # Jif the tag is dirty - report problem
#
#       Go to the next tag and continue processing until all done
#
.cqcd_30:
        addo    r9,r7,r7                # Point to the next Tag
        cmpobl  r7,r8,.cqcd_20          # Jif more tags to process
        ldconst FALSE,g0                # Show that no data exists in cache
        b       .cqcd_100               # Return
#
# --- Cache is initialized.  Test to determine if data is going into cache or
#       is already in cache.  Check all the VIDs for any with cache enabled.
#
.cqcd_50:
        ldconst FALSE,g0                # Set return to show no data could exist
        ldconst 0,r4                    # r4 = Current VID
.cqcd_70:
        cmpobe  r4,r5,.cqcd_100         # Jif no more VIDs to check
        ld      vcdIndex[r4*4],r6       # r6 = VCD pointer
        addo    1,r4,r4                 # increment the VID
        cmpobe  0,r6,.cqcd_70           # Jif undefined
        ldob    vc_stat(r6),r13         # r13 = VCD status
        ld      vc_write_count(r6),r14  # r14 = Count of data coming into cache
        bbc     vc_cached,r13,.cqcd_70  # Jif this VID is not caching data
        ld      vc_cache(r6),r15        # r15 = Data in Cache Tree Root
        cmpobne 0,r14,.cqcd_90          # Jif there is data going into cache
        cmpobe  0,r15,.cqcd_70          # Jif there is no data in cache
.cqcd_90:
        ldconst TRUE,g0                 # Data is going to or is in cache
#
# --- All Done
#
.cqcd_100:
.ifdef HISTORY_KEEP
c CT_HISTORY_ON();
.endif  # HISTORY_KEEP
        ret
#
#**********************************************************************
#
#  NAME: c$queryOpSend
#
#  PURPOSE:
#       Determines if an Op can be sent or needs to be queued depending on the
#       outstanding ops for the VDisk or Controller.
#
#  DESCRIPTION:
#       This routine will determine if an op can be sent on or if it needs to
#       be queued.  The determination is based on:
#           a) Utilization of VDisk (priority, size of op, type of op, number
#               of ops)
#           b) Utilization of the Controller (combined VDisks utilization)
#
#       Each op is assigned an "OTV" (Op Throttle Value) which is the
#       multiplication of the VDisk Priority, Size of Op, and Type of Op.
#       This OTV, when sent on, is added to the "VTV (VDisk Throttle Value)"
#       and the "CTV (Controller Throttle Value)".  If any ops are already
#       queued for the VDisk, then this ops must also be queued.  If the
#       current VTV is less than a prespecified value, the op can continue.
#       If the current CTV is less than a prespecified value, then
#       op can continue.  Otherwise, the op must be queued (too busy).
#
#  CALLING SEQUENCE:
#       call    c$queryOpSend
#
#  INPUT:
#       g0 = VCD pointer
#       g1 = TRUE if the queue should be used to determine if sending an
#               op is OK
#          = FALSE if the queue should not be used to determine if sending an
#               op is OK
#
#  OUTPUT:
#       g0 = TRUE if op can continue (not too busy)
#          = FALSE if the op cannot continue - must be queued (too busy)
#
#  REGS DESTROYED:
#       g0
#
#  REGISTERS USED:
#       r3  = Copy of VCD
#       r4  = VCD Op Queue Head
#       r5  = VDisk Throttle Value (VTV)
#       r6  = VTV Maximum value before checking CTV
#       r7  = Controller Throttle Value (CTV)
#       r8  = CTV Maximum value before need to queue op
#
#**********************************************************************
#
c$queryOpSend:
        mov     g0,r3                   # Save the VCD pointer
        ldconst FALSE,g0                # Set return code as Cannot Continue
.ifndef MODEL_3000
.ifndef  MODEL_7400
#
# --- Check for ISE_BUSY. If true, return false
#
        ldob    vc_stat(r3),r5          # Get the current State
        bbs     vc_ise_busy,r5,.cqos100 # Jif VCD in busy state
.endif  # MODEL_7400
.endif  # MODEL_3000
        cmpobe  FALSE,g1,.cqos40        # Jif not checking the queue
#
# Determine if there are ops waiting (on queue) for the VDisk already - if so
#   return as cannot continue
#
        ld      vc_thead(r3),r4         # r4 = Queue Head of ops waiting
        cmpobne 0,r4,.cqos100           # Jif ops already waiting to be sent
#
# Determine if the VTV will allow more ops to continue
#
.cqos40:
        ld      vc_vtv(r3),r5           # r5 = VDisk Throttle Value
        ldconst VTV_MAX,r6              # r6 = Maximum VDisk Throttle Value
        cmpoble r5,r6,.cqos90           # Jif current is less than or equal max
#
# Determine if the CTV will allow more ops to continue
#
        ld      C_ctv,r7                # r7 = Controller Throttle Value
        ldconst CTV_MAX,r8              # r8 = Maximum Controller Throttle Value
        cmpobe  r5,r7,.cqos90           # Jif only VDisk being used at this time
        cmpobg  r7,r8,.cqos100          # Jif current is greater than max
#
# The op can be sent on
#
.cqos90:
        ldconst TRUE,g0                 # Return code to allow op to continue
#
# All Done
#
.cqos100:
        ret
#
#**********************************************************************
#
#  NAME: c$opQueue
#
#  PURPOSE:
#       Queue the requested Op to the VCD Op Wait Queue
#
#  DESCRIPTION:
#       The ILT of the op will be added to the tail of the VCD Op waiting queue.
#       If no ops were waiting, also add the VCD to the VCD waiting queue to
#       get the queued ops sent when possible.
#
#  CALLING SEQUENCE:
#       call    c$opQueue
#
#  INPUT:
#       g0 = VCD pointer
#       g1 = ILT of op needing to be queued
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       None
#
#  REGISTERS USED:
#       r3  = Clearing register
#       r4  = VCD Op Waiting Queue Head
#       r5  = VCD Op Waiting Queue Tail
#       r6  = Controller VCD Waiting Queue Head
#       r7  = Controller VCD Waiting Queue Tail
#
#**********************************************************************
#
c$opQueue:
.ifndef MODEL_3000
.ifndef  MODEL_7400
        PushRegs(r3)                    # Save all the 'G' registers
        call    CA_Check4PAB
        PopRegsVoid(r3)
.endif  # MODEL_7400
.endif  # MODEL_3000
#
# --- Determine if this op is going onto a queue that is empty or not
#
        ld      vc_thead(g0),r4         # Get throttle queue head
        cmpo    0,r4                    # Determine if throttle queue empty
        sele    r4,g1,r4                # Set up the appropriate Head
        bne     .coq70                  # Jif queue is not empty
#
# --- Insert into empty queue, so need to add to the VCD waiting queue to get
#       these ops issued when possible
#
        ld      C_vcd_wait_head,r6      # Get the VCD Wait Queue Head
        ld      C_vcd_wait_tail,r7      # Get the VCD Wait Queue Tail
        st      r7,vc_bwd_wait(g0)      # Set up the VCD Wait backward pointer
        cmpo    0,r6                    # Determine if the queue is empty
        sele    r6,g0,r6                # Set up the appropriate Head
        be      .coq20                  # Jif VCD Wait Queue is empty
        st      g0,vc_fwd_wait(r7)      # Append Op to the end of the queue
.coq20:
        mov     g0,r7                   # Set up the new tail
        st      r6,C_vcd_wait_head      # Store the new VCD Wait Head
        st      r7,C_vcd_wait_tail      # Store the new VCD Wait Tail
        b       .coq80
#
# --- Insert the op into a non-empty queue
#
.coq70:
        ld      vc_ttail(g0),r5         # Get throttle queue tail
        st      g1,il_fthd(r5)          # Append ILT to end of queue
.coq80:
        mov     g1,r5                   # Update queue tail
        st      r4,vc_thead(g0)
        st      r5,vc_ttail(g0)
        st      0,il_fthd(g1)           # Clear input ILT forward pointer
        ret
#
#**********************************************************************
#
#  NAME: CA_QueryMirrorPartnerChange
#
#  PURPOSE:
#       Determines if the Mirror Partner can change.
#
#  DESCRIPTION:
#       This routine will determine if all I/O has been stopped, there are no
#       outstanding I/O, no data can be put into the Write Cache, and if there
#       is a communications path to the new Mirror Partner.
#
#  CALLING SEQUENCE:
#       call    CA_QueryMirrorPartnerChange
#
#  INPUT:
#       g0 = The requested new Mirror Partner
#
#  OUTPUT:
#       g0 = 0x00 - All OK to change Mirror Partner
#           otherwise the response is the problem that could hang up MP change
#          =  mrqmpcnostop      Bit 0 = I/O Is not stopped
#          =  mrqmpcioouts      Bit 1 = I/O are still outstanding
#          =  mrqmpccachedata   Bit 2 = Data Still Cached for old MP
#          =  mrqmpcnocomm      Bit 3 = No Comm Path to new MP (possible prob)
#
#**********************************************************************
#
CA_QueryMirrorPartnerChange:
#
# --- Ensure that a Stop I/O is active, there are no outstanding operations,
#       there is no cache data for the type of change, and there is a
#       communications path to the new Mirror Partner
#
        lda     C_ca,r4                 # r4 = Cache Information pointer
        ld      K_ficb,r3               # r3 = FICB
        mov     g0,r15                  # r15 = new Mirror Partner
        ldob    ca_stopcnt(r4),r11      # r11 = stop count to ensure all I/0
                                        #  has stopped
        ld      C_orc,r10               # r10 = outstanding I/O count
        ld      fi_mirrorpartner(r3),r5 # r5 = the current mirror partner S/N
        ld      fi_cserial(r3),r7       # r7 = This controllers Serial Number
#
        cmpo    0,r11                   # Determine if I/O is stopped
        alterbit mrqmpcnostop,0,r6      # Set bit if I/O is not stopped
#
.ifdef M4_DEBUG_C_orc
c CT_history_printf("%s%s:%u: C_orc mrqmpcioouts setting if 0 (%lu)\n", FEBEMESSAGE,__FILE__, __LINE__, C_orc);
.endif  # M4_DEBUG_C_orc
        cmpo    0,r10                   # Determine if I/O is still outstanding
        alterbit mrqmpcioouts,r6,r6     # Set bit if not (need to invert)
        notbit  mrqmpcioouts,r6,r6      # Now correct to show if I/O is outstanding
#
        cmpobe  0,r15,.cqmpc_100        # Jif the new is zero (use set MP) and
                                        #  ignore Communications Check
        cmpobe  r15,r5,.cqmpc_100       # Jif the new and old Mirror match
                                        #  and ignore Communications Check
        call    C$queryCacheData        # Determine if it is possible to have
                                        #  any data in cache
                                        # input: none
                                        # output: g0 = TRUE if data could be
                                        #            = FALSE if not possible
        cmpo    TRUE,g0                 # Determine if Data in Cache or Not
        alterbit mrqmpccachedata,r6,r6  # Set bit if data in cache
#
        cmpobe  r7,r15,.cqmpc_100       # Jif the New MP is this controller
                                        #  (Can always talk to ourself)
        mov     r15,g0                  # g0 = New Mirror Partner
        call    DLM$queryFEcomm         # g0 = ecok if comm path available
                                        #    != ecok if no comm path available
        cmpo    ecok,g0                 # Determine if path is available
        alterbit mrqmpcnocomm,r6,r6     # Set bit if a good path (need to invrt)
        notbit  mrqmpcnocomm,r6,r6      # Set bit if no good path (invert bit)
#
# --- All Done
#
.cqmpc_100:
        mov     r6,g0                   # Set up the return status
        ret
#
#**********************************************************************
#
#  NAME: c$sendMirrorPartnerChange
#
#  PURPOSE:
#       Sends a message to the back end processor to update
#       the Mirror Partner stored in NVRAM.
#
#  DESCRIPTION:
#       This function will create a message to send across the PCI bus
#       to the back end processor.  The new Mirror Partner serial number must
#       already be in the FICB.
#
#  CALLING SEQUENCE:
#       call    c$sendMirrorPartnerChange
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
#  REGISTERS USED:
#       r3 = K_ficb pointer
#       r8 = Mirror Partner
#       r10 - r15 = g0-g5 save registers
#
#**********************************************************************
        .section    .shmem
# This data area will be overwritten by the other processes define, but the
# information returned is never used -- and as such doesn't matter.
sendMirrorPartnerChangeReturn:
        .space  msmprsiz
#**********************************************************************
        .text

c$sendMirrorPartnerChange:
        movq    g0,r12                  # Save g0-g3
        movl    g4,r10                  # Save g4-g5
        mov     g6,r9                   # Save g6
#
# --- Create the message.
#
# NOTE: This message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ld      K_ficb,r3               # Get FICB
        ld      fi_mirrorpartner(r3),r8 # Get the mirror partner
        st      r8,msmp_mp(g0)          # Set the new Mirror Partner
        ldconst msmpsiz,g1              # Set Mirror Partner packet size
        ldconst mrfesetmp,g2            # Set Mirror Partner function code
        lda     sendMirrorPartnerChangeReturn,g3 # Return data address (ignored)
        ldconst msmprsiz,g4             # Return data size (ignored data)
        ldconst 0,g5                    # No completion function
        ldconst 0,g6                    # no user defined data
#
# --- Send the message
#
        call    L$send_packet           # Send the packet
#
# --- Exit
#
        movq    r12,g0                  # Restore g0-g3
        movl    r10,g4                  # Restore g4-g5
        mov     r9,g6                   # Restore g6
        ret
#
#**********************************************************************
#
#  NAME: c$logMirrorFailure
#
#  PURPOSE:
#       Logs a message to the CCB for a Mirror Failure (if not already reported)
#
#  DESCRIPTION:
#       This function will create a log message to send to the CCB if it has
#       not already been reported.
#
#  CALLING SEQUENCE:
#       call    c$logMirrorFailure
#
#  INPUT:
#       g0 = transport error code
#       g1 = ILT for this Datagram
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
CA_LogMirrorFailure:
c$logMirrorFailure:
        ld      C_mirror_error_flag,r9  # r9 = Reported any error since good
                                        #   mirror occurred
        ldconst TRUE,r15                # Show that a remote mirror failed
        st      r15,C_mirror_error_flag
        cmpobe  r9,r15,.clogmf100       # Jif an error has already been reported
                                        #  for this mirror partner.
#
# --- Send message to browser about the error being found, if it has not
#       already been reported
#
        ld      il_wcdlmdrp(g1),r3      # r3 = Address of the DRP
        ld      dr_req_address(r3),r5   # r5 = Address of the Datagram Request
        ld      dr_rsp_address(r3),r6   # r6 = Address of the Datagram Response
        ld      dgrq_dstsn(r5),r10      # r10 = Destination Controller Serial #
        ldob    dr_status(r3),r11       # r11 = DRP status
        ldob    dgrs_status(r6),r12     # r12 = Datagram Response status
        ldob    dgrs_ec1(r6),r13        # r13 = Datagram Response Error Code #1
        ldob    dgrs_ec2(r6),r14        # r14 = Datagram Response Error Code #2
#
        mov     g0,r4                   # Save g0
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
#
        ldconst mlecachemirrorfailed,r3 # Write cache sequence number bad
        st      r3,mle_event(g0)        # Store as word to clear other bytes
        bswap   r10,r10                 # Byte swap the Controller Serial Number
        st      r10,emf_controller_sn(g0) # Save the controller serial number
        st      r4,emf_ilt_status(g0)   # Save the ILT Status
        stob    r11,emf_drp_status(g0)  # Save the DRP Status
        stob    r12,emf_dg_status(g0)   # Save the Datagram Response status
        stob    r13,emf_dg_ec1(g0)      # Save Datagram Response Error Code #1
        stob    r14,emf_dg_ec2(g0)      # Save Datagram Response Error Code #2
        st      r4,C_mirror_error_ilt   # Save the ILT status for future checks
        st      r11,C_mirror_error_drp  # Save the DRP status for future checks
        st      r12,C_mirror_error_dg   # Save the Datagram Status for later
        st      r13,C_mirror_error_ec1  # Save the DG EC #1 for future checks
        st      r14,C_mirror_error_ec2  # Save the DG EC #2 for future checks
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], emflen);
        mov     r4,g0                   # Restore g0
#
.clogmf100:
        ret
#
#**********************************************************************
#
#  NAME: c$issueRebuild
#
#  PURPOSE:
#       This particular VDisk requires Rebuild ops to be issued for the Host
#       write to occur before mirroring the Write Information to the
#       Mirror Partner
#
#  DESCRIPTION:
#       This VDisk requires a Rebuild on the region of the Host write to
#       insure the Parity and Data match for the device that has been hotspared.
#       After the Rebuild completes, mirror the Write Information to the
#       Mirror Partner.
#
#  CALLING SEQUENCE:
#       call    c$issueRebuild
#
#  INPUT:
#       g1 = ILT associated with write
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None
#
#**********************************************************************
#
c$issueRebuild:
        movq    g0,r12                  # Save g0-g3
#
# --- Create the Rebuild request based on the Write Request
#
        call    M$aivw                  # g1 = new ILT
                                        # g2 = new VRP
        ld      vrvrp(r13),r3           # r3 = original write VRP
.if 1 # VIJAY_MC
        mov     0,r4
        setbit  1,r4,r4
        st      r4,vr_use2(r3)          # set 1-bit indicating a rebuild before write.
.endif  # 1
        ldconst vrrebuildchk,r4         # r4 = Rebuild Check operation
        ldconst vrhigh,r5               # r5 = Strategy = High
        ldos    vr_vid(r3),r6           # r6 = VID
        ld      vr_vlen(r3),r7          # r7 = Length in sectors
        ldl     vr_vsda(r3),r8          # r8-r9 = Starting LBA
        lda     c$issueRebldComp,r10    # r10 = Completion routine
        ldconst 0,r11                   # r11 = Clearing register
        stos    r4,vr_func(g2)          # Set new VRP function (Rebuild)
        stob    r5,vr_strategy(g2)      # Set new VRP strategy (High)
        stob    r11,vr_status(g2)       # Clear the Status field
        stos    r6,vr_vid(g2)           # Set new VRP VID
        stob    r11,vr_path(g2)         # Clear the Path field
        st      r11,vr_pptr(g2)         # Clear the Physical Pointer field
        st      r7,vr_vlen(g2)          # Set new VRP length
        stl     r8,vr_vsda(g2)          # Set new VRP Starting Logical Block @
        st      r11,vr_sglptr(g2)       # Clear the SGL Pointer
        st      r11,vr_sglsize(g2)      # Clear the SGL Size
        st      r13,il_w0(g1)           # Save old ILT in new ILT
        st      r11,il_w1(g1)           # Clear the Retry Counter
        st      g2,vrvrp(g1)            # Save new VRP in new ILT
        st      r10,il_cr(g1)           # Save completion routine to call
#
# --- Send the Rebuild request to the BE and wait for the completion
#
        call    CA_CallLowerSend
#
# --- All done
#
        movq    r12,g0                  # Restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: c$issueRebldComp
#
#  PURPOSE:
#       The Rebuild op completed, so the Write Mirroring can occur now.
#
#  DESCRIPTION:
#       The Rebuild op completed that was needed before sending the Write
#       Information to the Mirror Partner.  Now send the Write Information to
#       the Mirror Partner, if still wanted.
#
#       If the Rebuild op failed, return the error status to the host, after
#       appropriate retries.
#
#  CALLING SEQUENCE:
#       call    c$issueRebldComp
#
#  INPUT:
#       g0 = Completion status
#       g1 = ILT associated with rebuild
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None
#
#**********************************************************************
#
c$issueRebldComp:
        movq    g0,r12                  # Save g0-g3
#
# --- Determine if the Rebuild Check completed OK and if so, do the write.
#       Else, determine if a retry is needed or not and handle accordingly.
#
        cmpobe  ecok,g0,.cissuerebldcomp_40 # Jif Rebuild completed OK
        cmpobe  ecreserved,g0,.cissuerebldcomp_40 # Jif not in RB or in limageip
        ld      il_w1(g1),r4            # r4 = retry count
        cmpobne ecioerr,g0,.cissuerebldcomp_10 # Jif not an I/O Error
        cmpinco 4,r4,r4                 # 5 Retries yet? Also increment
        be      .cissuerebldcomp_20     # Jif retries exhausted - return error
        st      r4,il_w1(g1)            # Save the new retry count and retry
        b       .cissuerebldcomp_50     #  the Rebuild Check again.
#
.cissuerebldcomp_10:
        cmpobe  ecnonxdev,g0,.cissuerebldcomp_20 # Jif Non-existant
        cmpobe  ecinop,g0,.cissuerebldcomp_20 # Jif Inop
        cmpobe  ecuninit,g0,.cissuerebldcomp_20 # Jif Uninitialized
c       if(ecretry == g0)fprintf(stderr,"<GR>issueRebldComp - request in RETRY..\n");
        cmpobe  ecretry,g0,.cissuerebldcomp_20 # Jif ecretry
#
# --- Unexpected error code received.  Log the error code.
#
        ldconst cac_sft7,r3             # r3 = error code to log
        lda     C_sft_flt,g0            # g0 = Software Fault Log Area
        st      r3,efa_ec(g0)           # Save the Error Code
        st      r12,efa_data(g0)        # Save the Returned Status
        st      r4,efa_data+4(g0)       # Save the Retry Counter
        st      r13,efa_data+8(g0)      # Save the ILT
        ldconst 16,r3                   # Number of bytes saved (ec + data)
        st      r3,mle_len(g0)          # Save the number of bytes to send
        call    M$soft_flt              # Error Trap or Log failure
                                        # Return the error to the host
#
# --- Rebuild failed.  Return the error to the host.
#
.cissuerebldcomp_20:
        ld      il_w0(g1),r3            # r3 = Original ILT
        ld      vrvrp(g1),g2            # g2 = Rebuild VRP
        call    M$riv                   # Release the Rebuild ILT and VRP
#
        ld      vrvrp(r3),r4            # Get the original VRP
        mov     r3,g1                   # Set up original ILT
        mov     r12,g0                  # Set up the Return code
        stob    r12,vr_status(r4)       # Store the return code in the VRP
        call    K$comp                  # Complete the VRP back to the host
        b       .cissuerebldcomp_100    #  and return
#
# --- Recover the original Write and send it on.
#
.cissuerebldcomp_40:
        ld      il_w0(g1),r3            # r3 = Original ILT
#
        ld      il_w0(r3),r4            # r4 = Retry Count
        ldconst 1,r8                    # Add 1
        addo    r8,r4,r4                # Increment
        st      r4,il_w0(r3)            # Save the new Retry Counter
#
        ld      vrvrp(g1),g2            # g2 = Rebuild VRP
        call    M$riv                   # Release the Rebuild ILT and VRP
        mov     r3,g1                   # Set up original ILT
        call    c$setWriteInfo          # Mirror the write information and then
                                        #   do the write
                                        # g1 = ILT (input)
                                        # g0 = True - Mirror Info In Progress
                                        #    = False - Mirror not needed
        cmpobe  TRUE,g0,.cissuerebldcomp_100 # Jif a mirror is in progress
#
# --- VID no longer requires Write info Mirroring or the Rebuild needs tried
#       again.
#
.cissuerebldcomp_50:
        call    CA_CallLowerSend
#
# --- All done
#
.cissuerebldcomp_100:
        movq    r12,g0                  # Restore g0-g3
        ret
#
#**********************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
