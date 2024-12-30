# $Id: kernel.as 159305 2012-06-16 08:00:46Z m4 $
#**********************************************************************
#
#  NAME: kernel
#
#  PURPOSE:
#       To provide the basic services normally associated with a light
#       weight multitasking kernel.  These services include process
#       controls, memory controls, queuing controls and interrupt
#       controls.
#
#  FUNCTIONS:
#    Initialization:
#       K$start         - Initial firmware entry from POSTs
#
#    Process control:
#       K$tfork         - Temporary process fork
#       K$fork          - Process fork
#       K$xchang        - Process exchange
#       K$qxchang       - Process quick exchange
#       K$twait         - Process timed wait
#       K$mrel_fmm      - Release a separate memory buffer
#
#    Message queuing control:
#       K$cque          - Common queuing routine
#       K$q             - Queue request
#       K$qw            - Queue request w/ wait
#       K$qwlink        - Queue request w/ wait without bumping ILT levels
#       K$comp          - Complete request
#       KernelDispatch  - Callback function dispatcher
#
#    This module employs 2 processes:
#       k$dfmexec       - Deferred free memory executive (1 copy)
#       k$timer         - K$twait timer executive (1 copy)
#
#  Copyright (c) 1996-2010 Xiotech Corporation.  All rights reserved.
#
#**********************************************************************
#
.ifdef FRONTEND
        .include "masterfe.inc"  # FRONTEND includes
.endif  # FRONTEND
        .include "master.inc"    # All include definitions
#
# --- local equates ---------------------------------------------------
#
        .set    prcbint,0x10            # Interrupt table offset in PRCB
        .set    timer0v,18              # Timer 0 interrupt vector
        .set    MAX_FILL_SIZE,16*1024*1024 # 16 MEG Memory Size Limit to Fill
                                        #   when freeing memory during Debug
#
# --- global function declarations ------------------------------------
#
        .globl  K$start                 # Initial entry
#
        .globl  K$tfork                 # Temporary process fork
        .globl  TaskCreate2             # Temporary process fork
        .globl  TaskCreate3             # Temporary process fork
        .globl  TaskCreate4             # Temporary process fork
        .globl  TaskCreate6             # Temporary process fork
        .globl  TaskCreate7             # Temporary process fork
        .globl  TaskCreate8             # Temporary process fork
        .globl  K$fork                  # Process fork
        .globl  TaskCreatePerm2         # Process fork, no parameters
        .globl  TaskCreatePerm2Shared   # Process fork, no parameters, shared memory
        .globl  TaskCreatePerm3         # Process fork, one parameter
        .globl  K$xchang                # Process exchange
        .globl  TaskSwitch              # Process exchange
        .globl  K$qxchang               # Process quick exchange
        .globl  K$twait                 # Process timed wait
        .globl  TaskSleepMS             # Process timed wait
        .globl  TaskSleepNoSwap         # Timed delay
#
.ifdef FRONTEND
        .globl  K$mrel_fmm              # Release a separate memory buffer
.endif  # FRONTEND
#
        .globl  K$cque                  # Common queuing routine
        .globl  EnqueueILT              # Queue request w/o wait
.ifdef BACKEND
        .globl  K$q                     # Queue request
        .globl  K$qw                    # Queue request w/ wait
.endif  # BACKEND
        .globl  EnqueueILTW             # Queue request w/ wait
        .globl  K$qwlink                # Queue request w/wait without ILT bump
        .globl  K$comp                  # Complete request
        .globl  QWComp                  # Complete request, resume task

        .globl  KernelDispatch          # Callback function dispatcher
#
# --- global data declarations ----------------------------------------
#
        .globl  K_ficb                  # FICB
        .globl  K_ii                    # II
        .globl  LimtAddr                # End address of Memory
        .globl  WcctSize                # Size of the Write Cache Control Table
        .globl  WcctAddr                # Address of the Write Cache Control Tbl
        .globl  WccSize                 # Size of the Write Cache Configuration
        .globl  WccAddr                 # Address of the Write Cache Config.
        .globl  WctSize                 # Size of the Write Cache Tag area
        .globl  WctAddr                 # Address of the Write Cache Tag area
        .globl  WcbSize                 # Size of the Write Cache Buffer area
        .globl  WcbAddr                 # Address of the Write Cache Buffer area
        .globl  NcdrAddr                # Beginning address = Non-cacheable DRAM
        .globl  NcdrSize                # Size of the Non-cacheable DRAM
        .globl  Bbd1Addr                # Address of Battery backup data 1 location
        .globl  Bbd2Addr                # Address of Battery backup data 2 location
        .globl  kernel_sleep            # Indicator of kernel suspending
        .globl  kernel_up_counter       # Indicator of status change
        .globl  kernel_switch_counter   # Indicator that kernel has done task switch
#
# --- global usage data definitions -----------------------------------
#
        .section critdata, lomem
        .globl  K_xpcb
        .globl  K_poffset
        .globl  K_rrstate
        .globl  K_rrtimer
        .globl  K_time
        .data
#
# --- kernel structures -----------------------------------------------
#
        .set    rr_idle,0               # RR not running
        .set    rr_start,1              # Start running RR on next switch
        .set    rr_running,2            # RR running
#
        .set    rr_delta,4              # One half second interval
#
# --- global usage data definitions -------------------------------------------
#
        .globl  gMMCFound
gMMCFound:
        .word   1                       # one is true.

        .section cds, data

#
# --- Memory Address Table
#
        .align  2                       # Word Alignment
#
.ifdef FRONTEND
        .set    LIMTADDR,FE_SHARE_LIM
.endif  # FRONTEND
.ifdef BACKEND
        .set    LIMTADDR,BE_SHARE_LIM
.endif  # BACKEND
#
        .set    WCCTSIZE,0x200          # Size of Write Cache Control Table
        .set    WCCTADDR,LIMTADDR-WCCTSIZE-0x200
        .set    WCCSIZE,0x10000         # 64K size
        .set    WCCADDR,WCCTADDR-WCCSIZE
        .set    WCTSIZE,0x48000         # 288K size
        .set    WCTADDR,WCCADDR-WCTSIZE
        .set    WCBSIZE,0x9000000       # 144Meg size
        .set    WCBADDR,WCTADDR-WCBSIZE
.ifdef FRONTEND
        .set    NCDRADDR,SHARELOC+0x147B00 # "Cacheable DRAM" must contain everything before heap
.endif # FRONTEND
.ifdef BACKEND
        .set    NCDRADDR,SHARELOC+0x187F80 # "Cacheable DRAM" must contain everything before heap
.endif # BACKEND
        .set    NCDRSIZE,WCBADDR-NCDRADDR

#
# --- Memory value locations
#
LimtAddr:
        .word   LIMTADDR                # End (Limit) address
WcctSize:
        .word   WCCTSIZE                # Write Cache Control Table size
WcctAddr:
        .word   WCCTADDR                # Write Cache Control Tabl addr
WccSize:
        .word   WCCSIZE                 # Write Cache Config Size
WccAddr:
        .word   WCCADDR                 # Write Cache Config address
WctSize:
        .word   WCTSIZE                 # Write Cache Tag size
WctAddr:
        .word   WCTADDR                 # Write Cache Tag address
WcbSize:
        .word   WCBSIZE                 # Write Cache Buffer size
WcbAddr:
        .word   WCBADDR                 # Write Cache Buffer address
NcdrAddr:
        .word   NCDRADDR                # Non-cacheable DRAM address
NcdrSize:
        .word   NCDRSIZE                # Non-cacheable DRAM size
Bbd1Addr:
        .word   BATTERY_BACKUP          # Battery backup data 1 addr
Bbd2Addr:
        .word   WCCTADDR+WCCTSIZE       # NOTE: they subtract 0x200 twice for WcctAddr.
                                        # i.e. x - WCCTSIZE-0x200.
#
# --- Configuration information
#
K_ficb:
        .word   0                       # FICB
#
.if     DEBUG_FLIGHTREC_XCHANG
        .align  4
.k_dbg_time:
        .word   0
        .set    K_MAX_PCB_TIME,4        # Maximum Task time between exchanges
                                        #   (in 128msec intervals)
.k_dbg_maxtime:
        .word   0
.k_dbg_caller:
        .word   0
.k_dbg_oldcaller:
        .word   0
.endif  # DEBUG_FLIGHTREC_XCHANG

        .section        .shmem
        .align  4
#
# --- Configuration information
#
#
K_ii:
        .space  iisiz,0
# k_dbg_logevent is no longer used anywhere, but due to being part of K_ii, it must remain.
.k_dbg_logevent:
        .space  32,0

kernel_sleep:   .word   0
kernel_up_counter:
                .word   0
kernel_switch_counter:
                .word   0

        .section cds, data
#
k_processed:
        .word   0                       # Bitmask of processed interrupts
k_upcount:
        .word   0                       # Save message count
k_prevrr:
        .word   0                       # Previous value of K_rrstate

        .word   CAWRAP_LL_TargetTaskCompletion # Needed to prevent C compiler from
                                               # optimizing out the routine!

# The number of entries in the "last" process to run list.
.ifndef PERF
        .set NumberLastPcbs,512*4
.else   # PERF
        .set NumberLastPcbs,64
.endif  # PERF
# This is for gdb figuring out how big the array is. (Old versions have CAWRAP_LL_TargetTaskCompletion)
        .word   NumberLastPcbs
k_last_pcbs:
        .space  4*NumberLastPcbs,0      # The last 64 tasks to run.
last_pc_fork_name:
        .space  32*NumberLastPcbs,0     # The name of the last 64 tasks to run.
last_runtime:
        .space  8*NumberLastPcbs,0      # The tsc at start time of this task run.
#
# --- BSS section
#
# --- Global data declarations ------------------------------------------------
#

#
# --- executable code -------------------------------------------------
#
        .text
#
#**********************************************************************
#
#  NAME: K$start
#
#  PURPOSE:
#       To provide a means of starting execution of the operational firmware.
#
#  DESCRIPTION:
#       The POSTs pass control to this routine after determining that
#       the hardware is operational. The entry is made in
#       supervisor mode with the supervisor stack.
#
#       The DRAM free memory pool(s) are initialized at this point.
#       Both cacheable and noncacheable regions are defined (see <PR$Tboltinit>)
#       and all PCI (both ATU and PCI bridge) registers are initialized.
#       Each module that contains an initialization routine is called prior
#       to starting up the multitasking kernel.
#
#  CALLING SEQUENCE:
#       b       K$start
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
K$start:
c       int ls = 0;
c       UINT64 gEpollUsecDelay = 1000;
c       UINT64 gEpollUsecDelay_SG = 1000;
#
# --- Initialize constant zero register  (g13)
#
        ldconst 0,g13                   # used throughout firmware (NOT REALLY)

# Set up signal handlers. Note that the signal handler for the timer interrupt
# (SIGALRM) can not be set up until the k$timer task has been forked, since
# the signal handler relies on K_tmrexec_pcb (the k$timer PCB address) being valid.

# L$int -> "inbound doorbell"   -- not used
# k$tint -> timer 0 interrupt   -- simulate with setitimer and SIG_ALRM
# k$tint1 -> timer 1 interrupt  -- simulate with i386 TSC register
# ISP$rqN -> Qlogic interrupts. -- handled via xio3d kernel module
# k$dmaint -> dma interrupt     -- not used

c       ls = 0;
c       while (++ls < L_SIGNAL_MAX_SIGNALS) {
c            switch (ls) { case SIGKILL: /* SIGKILL - line needed for CT_analyze */
c                /*
c                ** Can not catch SIGKILL or SIGSTOP
c                */
# c                case SIGKILL:
c                case SIGSTOP:
# c                    continue;
c                    /*
c                    ** Linux threads uses the first 3 real-time signals
c                    */
c                case __SIGRTMIN:
c                case (__SIGRTMIN + 1):
c                case (__SIGRTMIN + 2):
# c                    continue;

c                    /*
c                    ** These standard signals should not be caught, since
c                    ** they are used for standard communications.
c                    */
c                case SIGCHLD:   /* Child status has changed */
c                case SIGCONT:   /* Continue */
c                case SIGTSTP:   /* Keyboard stop */
c                case SIGTTIN:   /* Background read from tty */
c                case SIGTTOU:   /* Background write to tty */
c                case SIGURG:    /* Urgent condition on socket */
c                case SIGWINCH:  /* Window size change */
c                    continue;

c               case SIGPROF:   /* Profiling alarm clock */
.ifdef GCOV
c                    continue;  /* Don't catch when profiling - catch otherwise */
.endif # GCOV
.ifdef M4_GDB_RUNNING
c                    continue;  /* Don't catch when running in gdb. */
.endif # M4_GDB_RUNNING
c                    /*
c                    ** Trap the following signals.
c                    */
c                case SIGHUP:    /* Hangup */
c                case SIGINT:    /* Interrupt */
c                case SIGQUIT:   /* Quit */
c                case SIGILL:    /* Illegal instruction */
c                case SIGTRAP:   /* Trace/breakpoint trap */
c                case SIGABRT:   /* Abort */
c                case SIGBUS:    /* Bus error */
c                case SIGFPE:    /* Floating point exception */
c                case SIGUSR1:   /* User defined signal #1 */
c                case SIGSEGV:   /* Segmentation violation */
c                case SIGUSR2:   /* User defined signal #2 */
c                case SIGPIPE:   /* Broken pipe */
c                case SIGALRM:   /* Alarm clock */
c                case SIGTERM:   /* Termination */
c                case SIGSTKFLT: /* Stack fault */
c                case SIGXCPU:   /* CPU time limit exceeded */
c                case SIGXFSZ:   /* File size limit exceeded */
c                case SIGVTALRM: /* Virtual alarm clock */
c                case SIGIO:     /* I/O now possible */
c                case SIGPWR:    /* Poiwer failure restart */
c                case SIGSYS:    /* Bad system call */
c                default:        /* All remaining real-time interrupts */
c                    break;
c            }
c            L_SignalHandlerAdd(ls, SIGNAL_ERRTRAP, true);
c       }

.ifdef GCOV
# gcov processing needs to trap SIGHUP specially.
c       signal(SIGHUP, SIGNAL_HUP);         # 1
.endif # GCOV

.ifndef PERF
# In debugging (non-PERF) environment, do the default handling for SIGINT
c       signal(SIGINT, SIG_DFL);            # 2  default
.endif # PERF

# Save esp and ebp registers for forking setup. -- CXGH
c       CT_start_esp = get_esp();       /* save stack pointer */
c       CT_start_ebp = get_ebp();       /* save frame pointer */
c       CT_stack_copy_size = CT_main_esp - CT_start_esp;
c fprintf(stderr, "%sCT_main_esp =0x%08lx CT_main_ebp =0x%08lx\n", FEBEMESSAGE, CT_main_esp, CT_main_ebp);
c fprintf(stderr, "%sCT_start_esp=0x%08lx CT_start_ebp=0x%08lx\n", FEBEMESSAGE, CT_start_esp, CT_start_ebp);
c fprintf(stderr, "%s stackcopy = 0x%08lx\n", FEBEMESSAGE, CT_stack_copy_size);
c       if (CT_stack_copy_size >= pc_CT_C_stack_size) {
c         fprintf(stderr, "%sCT_stack_copy_size too big (%ld)\n", FEBEMESSAGE,CT_stack_copy_size);
c         abort();
c       }
c       CT_ebp_diff = CT_start_ebp - CT_start_esp;
# Get setting for epoll delay in microseconds.

c       if ( getenv("USECEPOLLDELAY") ) {
c           char* strEpollDelay = getenv("USECEPOLLDELAY");
c           gEpollUsecDelay = atoll(strEpollDelay);
c           fprintf(stderr, "%sSetting Global Epoll delay to %llu usec\n", FEBEMESSAGE, gEpollUsecDelay);
c       }
c       if ( getenv("USECEPOLLDELAY_SG") ) {
c           char* strEpollDelay = getenv("USECEPOLLDELAY_SG");
c           gEpollUsecDelay_SG = atoll(strEpollDelay);
c           fprintf(stderr, "%sSetting SG Epoll delay to %llu usec\n", FEBEMESSAGE, gEpollUsecDelay_SG);
c       }
#
        call    PR$Tboltinit            # Perform processor/architecture
                                        #  dependent initialization
#
# --- Initialize ii structure
#
        lda     K_ii,r15                # Get internal SRAM copy of II
#
        ldconst iipinit,r4              # Set up status
        stos    r4,ii_status(r15)
#
.ifdef BACKEND
        ldob    D_gpri,r4               # Set up global priority
        stob    r4,ii_gpri(r15)
.endif  # BACKEND
#
# --- Initialize the CCB Communications Area
#
c       memset((void *)CCBCOMM, 0, CCB_COMM_SIZE);
#
# --- Initialize Debug Data Retrieval (DDR) table K_ii entry
#
        lda     K_ii,g1                 # Load address of K_ii
c       M_addDDRentry(de_kii, g1, iisiz);
#
# --- Initialize Debug Data Retrieval (DDR) table NVRAM Diagnostic Data entry
#
        ldconst NVSRAMP5START,g1        # Load address of Part 5 NVRAM
        ld      nv5h_len(g1),g2         # Load length of Part 5 NVRAM
c       M_addDDRentry(de_nvram5, g1, g2);
#
# --- Initialize Debug Data Retrieval (DDR) table NVRAM Backtrace Data entry
#
        # Load address and length of Part 1 NVRAM
c       M_addDDRentry(de_nvram1, NVSRAM, NVSRAMRES);
#
# --- Establish deferred free memory executive
#
        lda     k$dfmexec,g0            # Establish deferred free mem exec
        ldconst DFMPRI,g1
c       CT_fork_tmp = (ulong)"k$dfmexec";
        call    K$fork
#
# --- Allocate memory for the FICB and load in the controller serial number
# --- from NVRAM.
#
c       g0 = s_MallocC(fisiz|BIT31, __FILE__, __LINE__); # Assign/clear memory for FICB
        st      g0,K_ficb               # Save FICB
#
# --- Add the FICB to the DDR table
#
c       M_addDDRentry(de_ficb, g0, fisiz);
#
# --- If the old controller serial number location is non-zero, copy to its
# --- new location and zero the old location.
#
c       r3 = getpagesize();
#
        ld      NVSRAM+NVSRAMOLDCSER,g1 # Get the s/n from the old location
        cmpobe  0,g1,.kst90             # Jif zero
        lda     NVSRAM+NVSRAMCSER,g0    # New controller s/n location
        ldconst 0,r4                    # Load zero into r4
        lda     NVSRAM+NVSRAMOLDCSER,r8 # Address of old s/n
#
# --- Copy old to new location
#
        stob    g1,(g0)                 # Copy old to new byte by byte
        shro    8,g1,g1                 # Shift right one byte
        stob    g1,1(g0)
        shro    8,g1,g1
        stob    g1,2(g0)
        shro    8,g1,g1
        stob    g1,3(g0)
#       No need to either push or pop g-regs, as the following is a system call
#       g0 --> Address of new controller s/n location
#       4 -->  Length in bytes
c       r9 = g0%r3;
c       r10 = msync((void*)(g0 - r9), 4 + r9, MS_SYNC);
c       if (r10 != 0) fprintf(stderr, "%sK$start:  msync failed, errno = %d\n", FEBEMESSAGE, errno);
#
# --- Zero old controller s/n
#
#
        stob    r4,(r8)                 # Zero old s/n byte by byte
        stob    r4,1(r8)
        stob    r4,2(r8)
        stob    r4,3(r8)
#       No need to either push or pop g-regs, as the following is a system call
c       r9 = r8%r3;
c       r10 = msync((void*)(r8 - r9), 4 + r9, MS_SYNC);
c       if (r10 != 0) fprintf(stderr, "%sK$start:  msync failed, errno = %d\n", FEBEMESSAGE, errno);
#
# --- Recalculate the NVRAM NMI area checksum
#
c       g0 = MSC_CRC32((void *)(NVSRAM+NVSRAMSTARTNMI+4),NVSRAMNMISIZ-4)               # Calculate the new checksum
        lda     NVSRAM+NVSRAMSTARTNMI,r8    # Load checksum location
#
# --- Store checksum in NVRAM, byte by byte
#
        stob    g0,(r8)
        shro    8,g0,g0
        stob    g0,1(r8)
        shro    8,g0,g0
        stob    g0,2(r8)
        shro    8,g0,g0
        stob    g0,3(r8)
#       No need to either push or pop g-regs, as the following is a system call
c       r9 = r8%r3;
c       r10 = msync((void*)(r8 - r9), 4 + r9, MS_SYNC);
c       if (r10 != 0) fprintf(stderr, "%sK$start:  msync failed, errno = %d\n", FEBEMESSAGE, errno);
#
# --- Recalculate the NVRAM serial number area checksum
#
c       g0 = MSC_CRC32((void *)(NVSRAM+NVSRAMSTARTSN+4),NVSRAMSNSIZ-4)               # Calculate the new checksum
        lda     NVSRAM+NVSRAMSTARTSN,r8     # Load checksum location
#
# --- Store checksum in NVRAM, byte by byte
#
        stob    g0,(r8)
        shro    8,g0,g0
        stob    g0,1(r8)
        shro    8,g0,g0
        stob    g0,2(r8)
        shro    8,g0,g0
        stob    g0,3(r8)
#       No need to either push or pop g-regs, as the following is a system call
c       r9 = r8%r3;
c       r10 = msync((void*)(r8 - r9), 4 + r9, MS_SYNC);
c       if (r10 != 0) fprintf(stderr, "%sK$start:  msync failed, errno = %d\n", FEBEMESSAGE, errno);
.kst90:
#
# --- Check if serial number area of NVRAM passes CRC check
#
c       g0 = MSC_CRC32((void *)(NVSRAM+NVSRAMSTARTSN+4),NVSRAMSNSIZ-4)           # Calculate the checksum
        ld      NVSRAM+NVSRAMSTARTSN,r4 # Get CRC from NVRAM
        cmpobe  r4,g0,.kst100           # Jif CRC is correct
        ldconst 0,g1                    # Load a controller sn of 0
        b       .kst110
.kst100:
        ld      NVSRAM+NVSRAMCSER,g1    # Get the controller serial number
        ldconst 0xffffff,r3             # Only use the lower 3 bytes
        and     r3,g1,g1
.kst110:
        ld      K_ficb,g0               # Point to ficb
        st      g1,fi_cserial(g0)       # Save it
#
# timer 1 simulated via i386 TSC register (64 bit value).
#
# --- Initialize any needed modules for this processor
#
        call    M$init                  # "misc" initialization
        PushRegs(r3)                    # Save register contents
        call    LL_Init                 # LinkLayer initialization
        PopRegsVoid(r3)                 # Restore registers
.ifdef FLIGHTRECORDER
c        flightrecorder_init();
.endif # FLIGHTRECORDER
.ifdef BACKEND
c       gMMCFound = TRUE;
        PushRegs(r3)                    # Save register contents
        call    MM_init                 # Initialize the Micro Memory Card
c       if (g0 == (unsigned long)(-1)) gMMCFound = FALSE;
        PopRegsVoid(r3)                 # Restore registers
.endif  # BACKEND
# NVA_Init call was commented from misc.as and added here.
        call    NVA_Init
        call    ISP$configure           # Configure any attached QLogic channels
.if FE_ICL
c       ICL_CheckIclExistence();
c       ICL_CreateIclTargets();
.endif  # FE_ICL
        call    ISP$start               # start QLogic tasks
#
.ifdef FRONTEND
#; LSW - Need to verify the correct order here...
        call    MAGD$init               # MAGNITUDE driver initialization
                                        #   must be before D$init
        call    CD$init                 # Channel Driver initialization
        call    D$init                  # "define" initialization
        call    C$init                  # "cache" initialization
c       tsl_init();
#
.if INITIATOR
        call    I$init                  # Initiator driver initialization
.endif # INITIATOR
#
.if     MAG2MAG                         # LLD and DLM initialization
        call    LLD$init
        call    DLM$init
.endif  # MAG2MAG
#
.else  # FRONTEND   i.e. BACKEND
        ldconst FALSE,r3                # Set Phase II inits NOT completed
        stob    r3,O_p2init             # This is NOT cleared when the jtag
                                        # debugger is reset.

        call    P$init                  # "physical" initialization
        call    O$init                  # "online" initialization
.endif  # FRONTEND
#
# --- Establish timer executive
#
        lda     k$timer,g0              # Establish timer exec
        ldconst TIMERPRI,g1
c       CT_fork_tmp = (ulong)"k$timer";
        call    K$fork
        st      g0,K_tmrexec_pcb        # Save PCB address

#
# --- Setting up the signal handler for SIGALRM must be done after
# --- the k$timer task fork, since the signal handling routine
# --- relies on the K_tmrexec_pcb variable being set.
#
#        c       extern void SIGNAL_ALRM(int);
c       signal(SIGALRM, SIGNAL_ALRM);       # 14    Use setitimer

# --- Set up real time clock interrupt.
c   {
c       static struct itimerval ct_real_time;
c       ct_real_time.it_value.tv_sec = 0;
c       ct_real_time.it_value.tv_usec = QUANTUM*1000;
c       ct_real_time.it_interval = ct_real_time.it_value;
c       setitimer(ITIMER_REAL, &ct_real_time, NULL);
c   }
#
# --- Exit
#
        ld      K_pcborg,r4             # Get 1st PCB
        mov     r4,r13

.ifdef TASK_STRESS_TEST
c       Task_Torture_initialized = 1;
.endif  # TASK_STRESS_TEST
        b       k$kernel                # Jumpstart multitasking kernel
#
#******************************************************************************
#
#  NAME: PR$Tboltinit
#
#  PURPOSE:
#       To perform processor and architecture dependent initialization for the
#       processor.
#
#  DESCRIPTION:
#       Setup FMM for cacheable dynamic memory.
#       Allocate PCI device table and perform scan of secondary PCI bus
#          forming device bitmask and device vendor/device IDs.
#       NOTE: assumes that the secondary PCI bus has been reset before
#             this routine is called.
#
#  CALLING SEQUENCE:
#       call    PR$Tboltinit
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#******************************************************************************
#
PR$Tboltinit:
        ldconst DIMMSIZEADDR,r5         # Get the Address of the size of DIMMs
.ifdef FRONTEND
c       r6 = SIZE_FE_LTH;               # Size of FE memory segment.
.endif # FRONTEND
.ifdef BACKEND
c       r6 = SIZE_BE_LTH;               # Size of BE memory segment.
.endif # BACKEND
        st      r6,(r5);                # emulate boot prom setting this address.
c       r3 = LI_GetDMAOffset();
        st      r3,K_poffset            # save cache offset value
#
# --- Validate start address of non-cacheable DRAM
#
c       g0 = (UINT32)pStartOfHeap;      # Get end of shared memory segments
        ld      NcdrAddr,g1             # Get noncache boundary
c       if (g0 != g1) {
c          fprintf (stderr, "%sNCDRADDR must be SHARELOC + exactly 0x%lx (currently 0x%lx)\n", FEBEMESSAGE, g0 - SHARELOC, g1 - SHARELOC);
c          abort();
c       }
#
# --- No concept of cacheable/non-cacheable in x86 world. Everything is considered
# --- non-cacheable.
#
# --- Initialize Noncacheable DRAM FMM structures
        ldconst MGRAN,r3                # Memory granularity
        ld      NcdrAddr,g0             # Get noncache boundary
        ld      NcdrSize,g1             # Get size of memory  //// TEMP ////
        andnot  r3,g1,g1                # Align size to granularity
#
        lda     pcncdram,r4             # Set up memory wait status
        stob    r4,K_ncdram+fm_waitstat
        lda     K_ii+ii_nccur,r4        # Set up FMS ptr
        st      r4,K_ncdram+fm_fms
c       k_init_mem((struct fmm *)&K_ncdram, (void *)g0, g1);  # Initialize shared memory.
#
# Private memory setup.
c       P_ram.fmm_waitstat = PCB_WAIT_NON_CACHEABLE;
c       P_ram.fmm_fms = &P_cur;         # Private memory statistics.
# Initialize Private memory statistics available memory value.
.ifdef FRONTEND
c       r5 = MAKE_DEFS_FE_ONLY_SIZE;
.else  # FRONTEND
c       r5 = MAKE_DEFS_BE_ONLY_SIZE;
.endif # FRONTEND
# Initialize shared memory.
c       k_init_mem(&P_ram, &local_memory_start, r5);
        ret
#
#**********************************************************************
#
#  NAME: K$fork/K$tfork
#
#  PURPOSE:
#       To provide a common means of calling and establishing a process.
#       The first entry point is used for establishing a permanent
#       process and the second entry point is used for establishing a
#       temporary process.
#
#  DESCRIPTION:
#       A Process Control Block is allocated and initialized.  The PCB
#       is then linked into the PCB thread and the status of this
#       process is set to ready.  The called process inherits the
#       global registers of the caller.  If this process executes a
#       return, the return is made to K$end which will end and release
#       this process.
#
#  CALLING SEQUENCE:
#       call    K$fork  - Used for permanent process
#        or
#       call    K$tfork - Used for temporary process
#
#  INPUT:
#       g0 = process entry point
#       g1 = process priority (0-255)
#
#  OUTPUT:
#       g0 = PCB
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
TaskCreate2:
TaskCreate3:
TaskCreate4:
TaskCreate5:
TaskCreate6:
TaskCreate7:
TaskCreate8:
K$tfork:
#
# --- Allocate temporary PCB ------------------------------------------
#
        mov     g0,r3                   # Save process entry point
c       r15 = malloc_pcb();             # Get a pcb from memory pool.
        lda     k$tend,r8               # Set RIP to k$tend in stack frame 0
        b       .fo10
#
# Create task in shared memory -- because other process changes it.
#
TaskCreatePerm2Shared:
        mov     g0,r3                   # Save process entry point
c       r15 = s_MallocC(pcbsiz|BIT31, __FILE__, __LINE__); # Allocate permanent PCB w/wait in shared memory
        lda     k$end,r8                # Set RIP to k$end in stack frame 0
        b       .fo10
#
TaskCreatePerm2:
TaskCreatePerm3:
K$fork:
#
# --- Allocate permanent PCB ------------------------------------------
#
        mov     g0,r3                   # Save process entry point
c       r15 = malloc_pcb();             # Get a pcb from memory pool.
        lda     k$end,r8                # Set RIP to k$end in stack frame 0
#
# --- Save global registers in PCB
#
.fo10:
.ifdef HISTORY_KEEP
c CT_history_task_name("creating task fork_name", (char*)CT_fork_tmp, (PCB*)r15);
.endif  # HISTORY_KEEP

.ifdef CT2_DEBUG
# Initialize for checking overflow.
c       initstackmemory(r15);
.endif  /* CT2_DEBUG */
        mov     r3,g0                   # g0 = entry point
        stq     g0,pc_g0(r15)           # Save g0-g3
        stq     g4,pc_g4(r15)           # Save g4-g7
        stq     g8,pc_g8(r15)           # Save g8-g11
        stt     g12,pc_g12(r15)         # Save g12-g14
#
# --- Initialize PCB
#
        mov     pcrdy,r3                # Set process to ready
        stob    g1,pc_pri(r15)          # Set process priority
        mov     TRUE,r4                 # Set global reg restore
        stob    r3,pc_stat(r15)
        stob    r4,pc_global(r15)
        lda     pc_sf1(r15),r3          # Set up PFP
        st      r3,pc_pfp(r15)
#
# --- Initialize stack frames in PCB ----------------------------------
#
        lda     pc_sf0(r15),r5          # r5 = frame 0
        lda     pc_sf1(r15),r6          # r6 = frame 1
        lda     pc_sf2(r15),r7          # r7 = frame 2
#
# --- Stack frame 0
#
        st      r6,sf_sp(r5)            # Set up SP (frame 1)
        st      r8,sf_rip(r5)
#
# --- Stack frame 1
#
        st      r5,sf_pfp(r6)           # Set up PFP (frame 0)
        st      r7,sf_sp(r6)            # Set up SP (frame 2)
        st      g0,sf_rip(r6)           # Set RIP to process entry point

.ifdef RREG_PATTERN_CHECK
#
# -- If r register initialization checking...
#
c       r8 = 0xBABEBABE;
?       st      r8,3*4(r5)              # r3 in frame 0
?       st      r8,4*4(r5)              # r4 in frame 0
?       st      r8,5*4(r5)              # r5 in frame 0
?       st      r8,6*4(r5)              # r6 in frame 0
?       st      r8,7*4(r5)              # r7 in frame 0
?       st      r8,8*4(r5)              # r8 in frame 0
?       st      r8,9*4(r5)              # r9 in frame 0
?       st      r8,10*4(r5)             # r10 in frame 0
?       st      r8,11*4(r5)             # r11 in frame 0
?       st      r8,12*4(r5)             # r12 in frame 0
?       st      r8,13*4(r5)             # r13 in frame 0
?       st      r8,14*4(r5)             # r14 in frame 0
?       st      r8,15*4(r5)             # r15 in frame 0

?       st      r8,3*4(r6)              # r3 in frame 1
?       st      r8,4*4(r6)              # r4 in frame 1
?       st      r8,5*4(r6)              # r5 in frame 1
?       st      r8,6*4(r6)              # r6 in frame 1
?       st      r8,7*4(r6)              # r7 in frame 1
?       st      r8,8*4(r6)              # r8 in frame 1
?       st      r8,9*4(r6)              # r9 in frame 1
?       st      r8,10*4(r6)             # r10 in frame 1
?       st      r8,11*4(r6)             # r11 in frame 1
?       st      r8,12*4(r6)             # r12 in frame 1
?       st      r8,13*4(r6)             # r13 in frame 1
?       st      r8,14*4(r6)             # r14 in frame 1
?       st      r8,15*4(r6)             # r15 in frame 1

?       st      r8,3*4(r7)              # r3 in frame 2
?       st      r8,4*4(r7)              # r4 in frame 2
?       st      r8,5*4(r7)              # r5 in frame 2
?       st      r8,6*4(r7)              # r6 in frame 2
?       st      r8,7*4(r7)              # r7 in frame 2
?       st      r8,8*4(r7)              # r8 in frame 2
?       st      r8,9*4(r7)              # r9 in frame 2
?       st      r8,10*4(r7)             # r10 in frame 2
?       st      r8,11*4(r7)             # r11 in frame 2
?       st      r8,12*4(r7)             # r12 in frame 2
?       st      r8,13*4(r7)             # r13 in frame 2
?       st      r8,14*4(r7)             # r14 in frame 2
?       st      r8,15*4(r7)             # r15 in frame 2
.endif  # RREG_PATTERN_CHECK

.ifdef CT2_DEBUG
# Check that the address we are going to is in the "i960" part of code.
#        c       fprintf(stderr, "%sCXGH pre-check_address_range(K$fork:%s) %s:%u between: 0x%-8.8lx < 0x%-8.8lx < 0x%-8.8lx\n", FEBEMESSAGE, (char *)CT_fork_tmp, __FILE__, __LINE__, (ulong)&CT_start, g0, (ulong)&CT_end);
c       if (g0 < (ulong)&CT_start || g0 >= (ulong)&CT_end) {
c               fprintf(stderr, "%sCXGH check_address_range(K$fork) %s:%u NOT: 0x%-8.8lx < 0x%-8.8lx < 0x%-8.8lx\n", FEBEMESSAGE, __FILE__, __LINE__, (ulong)&CT_start, g0, (ulong)&CT_end);
c               abort();
c       }
.endif  /* CT2_DEBUG */
.ifndef PERF
c       if (strncmp((void *)CT_fork_tmp, "UNSET_FORK_NAME_Please_FIX_THIS",32) == 0) {
c         fprintf(stderr, "%sUnset fork name, please set!\n", FEBEMESSAGE);
c         abort();
c       }
.endif # PERF
c       strncpy ((void *)(r15+pc_CT_fork_name), (void *)CT_fork_tmp, XIONAME_MAX);
c       CT_fork_tmp = (ulong)"UNSET_FORK_NAME_Please_FIX_THIS";

/* Initialize the saved "c" stack information */
c       r3 = r15+pc_c_sp+pc_CT_C_stack_size-CT_stack_copy_size;
c       r4 = r3+CT_ebp_diff;
        st      r3,pc_CT_esp(r15)
        st      r4,pc_CT_ebp(r15)
c       r6 = r15+pc_c_sp+pc_CT_C_stack_size-CT_stack_copy_size;
c       memcpy((char *)r6, (char *)CT_start_esp, CT_stack_copy_size);
.ifdef CT2_DEBUG
c       checkstackmemory("after initializing ", r15); # Check lower words still not changed.

# c       fprintf(stderr,"CXGH K$fork (%s)\n", (char*)(r15+pc_CT_fork_name));
# c       fprintf(stderr, "r15 (pcb)=0x%-8.8lx, stack[%lx - %lx], new esp=0x%-8.8lx, new bsp=0x%-8.8lx\n", r15, r15+pc_c_sp, r15+pc_c_sp+pc_CT_C_stack_size, r3, r4); */
.endif  /* CT2_DEBUG */

.ifdef HISTORY_KEEP
c CT_history_pcb("K$fork setting ready pcb", r15);
.endif  # HISTORY_KEEP

#
# --- Circularly link PCB into PCB thread -----------------------------
#
        ld      K_pcborg,r3             # Check for any processes
.ifdef HISTORY_KEEP
c CT_HISTORY_OFF();
.endif  # HISTORY_KEEP
        cmpobne 0,r3,.fo20              # If so, jump
#
# --- Only process
#
        st      r15,pc_thd(r15)         # New entry's forward points to ourself.
        st      r15,K_pcborg            # Set new entry as first process.
        b       .fo30
#
# --- Not the only process, insert in priority order.
#     Check if this new PCB has a higher priority than the
#     PCB currently at the head.  If so, the new PCB becomes the head.
#
.fo20:
        mov     r3,r6                   # Save 1st PCB
        ldob    pc_pri(r3),r5           # Get priority of next PCB
        cmpoble r5,g1,.fo25             # Jif priority below this process
#
# --- The new PCB will become the head of the list.  Find last PCB
#     so it can be linked to the new PCB.
#
.fo22:
        mov     r3,r4                   # Save current PCB
        ld      pc_thd(r4),r3           # Get next PCB
        cmpobne r3,r6,.fo22             # Jif PCB not found
#
# --- Set the new PCB as the head of the PCB list.
#
        st      r3,pc_thd(r15)          # new entry's forward points to start.
        st      r15,pc_thd(r4)          # last forward points to new entry.
        st      r15,K_pcborg            # Set new entry as first process.
        b       .fo30
#
# --- Find PCB that has a lower priority (higher number)
#
.fo25:
        mov     r3,r4
        ld      pc_thd(r4),r3           # Get next PCB
        cmpobe  r3,r6,.fo27             # Jif back to the 1st process
        ldob    pc_pri(r3),r5           # Get priority of next PCB
        cmpoble r5,g1,.fo25             # Jif priority below this process
#
# --- Insert process into PCB list
#
.fo27:
        st      r3,pc_thd(r15)          # new entry's forward points to entry.
        st      r15,pc_thd(r4)          # Insert process into PCB list forward.
#
.fo30:
#
# --- Update II statistics
#
        lda     K_ii,r3                 # Get II structure
        ldl     ii_pcbcur(r3),r4        # Get PCB counts
        addo    1,r4,r4                 # Bump current count
        st      r4,ii_pcbcur(r3)
        cmpoble r4,r5,.fo100            # Jif new max not met
#
        st      r4,ii_pcbmax(r3)        # Set new maximum
#
# --- Exit
#
.fo100:
        mov     r15,g0                  # Set g0 = PCB
.ifdef HISTORY_KEEP
c CT_HISTORY_ON();
.endif  # HISTORY_KEEP
        ret
#
#**********************************************************************
#
#  NAME: K$twait
#
#  PURPOSE:
#       To provide a common means for handling process timed wait
#       requests.
#
#  DESCRIPTION:
#       The calling process is placed into a timed wait status for the
#       duration of the requested wait.
#
#       The granularity of the time is 125ms.
#
#  CALLING SEQUENCE:
#       call    K$twait
#
#  INPUT:
#       g0 =  1 millisecond wait timeout, must be <= 0x7fffffff
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
#**********************************************************************
#
#  NAME: K$xchang
#
#  PURPOSE:
#       To provide a common means for a process to initiate a context
#       exchange to the next available process.
#
#  DESCRIPTION:
#       The global registers of the current process are saved within
#       the PCB.  Control is passed to the kernel in order to place the
#       next ready process into execution.
#
#  CALLING SEQUENCE:
#       call    K$xchang
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
#**********************************************************************
#
#  NAME: K$qxchang
#  NAME: K_qxchang
#
#  PURPOSE:
#       To provide a common means for a process to initiate a context
#       exchange to the next available process without incurring the
#       overhead of saving global registers.
#
#  DESCRIPTION:
#       The global registers of the current process are not saved within
#       the PCB.  Control is passed to the kernel in order to place the
#       next ready process into execution.
#
#  CALLING SEQUENCE:
#       call    K$qxchang
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All.
#
#**********************************************************************
#
#**********************************************************************
#
#  NAME: k$kernel
#
#  PURPOSE:
#       To provide a method of creating support for a multitasking
#       environment.
#
#  DESCRIPTION:
#       Control is initially passed to this routine in order to start
#       up the multitasking environment.  Each process is designed to
#       be nonpreemptive.  All processes execute in supervisor mode
#       with each process having its own supervisor stack.
#
#  CALLING SEQUENCE:
#       b       k$kernel                # Executes 1st process in thread
#
#  INPUT:
#       r4 = next PCB
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
TaskSleepMS:                            # -----------------------------
K$twait:                                # -----------------------------
        ld      K_xpcb,r13              # Get executing PCB
#
# --- Set process to timed wait
#
        bbs     31,g0,.kerr17           # Error trap if illegal timeout value
        lda     pctwait,r3              # Set status to timed wait
        st      g0,pc_time(r13)         # Set ms timeout counter
        stob    r3,pc_stat(r13)
#-        b       K$xchang
# fall-through
#
TaskSwitch:                             # -----------------------------
K$xchang:                               # -----------------------------
        ld      K_xpcb,r13              # Get executing PCB
#
# --- Save global registers in PCB
#
        stq     g0,pc_g0(r13)           # Save g0-g3
        stq     g4,pc_g4(r13)           # Save g4-g7
        stq     g8,pc_g8(r13)           # Save g8-g11
        mov     TRUE,r3                 # Set global reg restore
        stt     g12,pc_g12(r13)         # Save g12-g14
        b       .kr10
#
K$qxchang:                              # -----------------------------
        ld      K_xpcb,r13              # Get executing PCB
        mov     FALSE,r3                # Clear global reg restore
#
# --- Save previous frame pointer
#
.kr10:
c       memmove(((char *)&k_last_pcbs)+4, (char *)&k_last_pcbs, (NumberLastPcbs-1)*4);
c       memmove(((char *)&last_pc_fork_name)+32, (char *)&last_pc_fork_name, (NumberLastPcbs-1)*32);
c       memmove(((char *)&last_runtime)+8, (char *)&last_runtime, (NumberLastPcbs-1)*8);
c       *(UINT32 *)&k_last_pcbs = r13;
c       strncpy ((void *)&last_pc_fork_name, (char *)(r13+pc_CT_fork_name), XIONAME_MAX);
c       *(UINT64 *)&last_runtime = get_tsc();
.ifdef HISTORY_KEEP
c CT_history_task_name("leaving task fork_name", (char*)(r13+pc_CT_fork_name), (PCB*)r13);
# Turn off history during scheduler.
c CT_HISTORY_OFF();
.endif  # HISTORY_KEEP
.ifdef CT2_DEBUG
c check_pcb_chain();
.endif  # CT2_DEBUG
.if     DEBUG_FLIGHTREC_XCHANG
        ldl     .k_dbg_time,r4          # get time and maxtime
        ld      K_ii+ii_time,r6         # get system time
        ldconst K_MAX_PCB_TIME,r7       # determine when time is too long
        subo    r4,r6,r4                # calculate delta time
        st      r6,.k_dbg_time          # Store new time
        cmpobl  r4,r7,.kr11a            # Jif below the too long value
#
        lda     32(sp),sp               # allocate stack to hold g0 -> g7
        stq     g0,-32(sp)              # Save g0-g3
        stq     g4,-16(sp)              # Save g4-g7
# This is a nasty place to be to want to get memory, must not s_MallocW() here!
# The concept of this message is that a task is taking too long, and it is
# only active in non-PERF mode (i.e. DEBUG builds). A static buffer is ok.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mletasktoolong,g2       # g2 = Log Event - task too long
        ldconst etlplen,g3              # Set parameter length
        mov     rip,r7                  # Get caller
        ld      pc_g12+12(r13),r6
        st      g2,mle_event(g0)        # Store as word to clear other bytes
        stob    g3,mle_len(g0)          # Store the Parameter Length
        st      r4,etl_time(g0)         # Store the time
        st      r7,etl_caller(g0)       # Store the caller
        st      r6,etl_old_caller(g0)   # Store the old caller
        st      r13,etl_pcb(g0)         # Store the PCB
        ldconst etllen,g1               # Set total length
.ifdef FRONTEND
        ldconst mrlogfe,g2              # MRP function code
.else   # FRONTEND
        ldconst mrlogbe,g2              # MRP function code
.endif  # FRONTEND
        ldconst 0,g3                    # Return data address (no data)
        ldconst 0,g4                    # No data allowed
        ldconst 0,g5                    # No completion function
        ldconst 0,g6                    # No user defined data
#
        call    L$send_packet           # Send the packet
        ldq     -32(sp),g0              # Restore g0-g3
        ldq     -16(sp),g4              # Restore g4-g7
        lda     -32(sp),sp              # return sp to correct place
#
.kr11a:
        cmpoble r4,r5,.kr11b            # Jif below old maxtime
#
        mov     rip,r5                  # Get caller
        ld      pc_g12+12(r13),r6
        stt     r4,.k_dbg_maxtime       # Store maxtime, caller, oldcaller

        ldconst frt_k_xchang,r7         # Type = context switch
        st      r7,fr_parm0             # Kernel - xchang
        stt     r4,fr_parm1             # parm1 = time
                                        # parm2 = caller
                                        # parm3 = old caller
        call    M$flight_rec            # Record it
.kr11b:
        mov     rip,r5                  # Get caller
        st      r5,pc_g12+12(r13)
.endif  # DEBUG_FLIGHTREC_XCHANG

        ld      pc_thd(r13),r4          # Advance to next PCB
        stob    r3,pc_global(r13)       # Save global reg restore flag
        st      pfp,pc_pfp(r13)         # Save PFP
/* Following 2 lines indicate registers available below. */
/* not available: ??    r4             ??             r13         */
/*     available: ?? r3    r5 r6 r7 r8 ?? r10 r11 r12     r14 r15 */
/* Save current esp and ebp registers. */
c       r6 = get_esp();
c       r7 = get_ebp();
        st      r6,pc_CT_esp(r13)
        st      r7,pc_CT_ebp(r13)

.ifdef CT2_DEBUG
c       checkstackmemory("before switching from", K_xpcb); # Check lower words still not changed.
# c       fprintf(stderr,"CXGH - switching from (%s)\n", (char*)(r13+pc_CT_fork_name));
# c       fprintf(stderr, "r13 (pcb)=0x%-8.8lx, 960 stack[%lx - %lx], c stack[%lx - %lx], saved: esp=0x%-8.8lx, bsp=0x%-8.8lx\n", r13, r13+pc_SF_start, r13+pc_c_sp, r13+pc_c_sp, r13+pc_c_sp+pc_CT_C_stack_size, r6, r7);
.endif  /* CT2_DEBUG */

.ifndef CT_assumes_works
c       if (r7 <= r13+pc_c_sp) {
c         fprintf(stderr, "%s%s:%u c ebp stack grew too big (%lx <= %lx) for process (%s)\n", FEBEMESSAGE, __FILE__, __LINE__,r7, r13+pc_c_sp, (char*)(r13+pc_CT_fork_name));
c         abort();
c       }
c       if (r7 > r13+pc_c_sp+pc_CT_C_stack_size) {
c         fprintf(stderr, "%s%s:%u c ebp stack too small  (%lx > %lx) for process (%s)\n", FEBEMESSAGE, __FILE__, __LINE__,r7, r13+pc_c_sp+pc_CT_C_stack_size, (char*)(r13+pc_CT_fork_name));
c         abort();
c       }
c       if (r6 <= r13+pc_c_sp) {
c         fprintf(stderr, "%s%s:%u c esp stack grew too big (%lx <= %lx) for process (%s)\n", FEBEMESSAGE, __FILE__, __LINE__,r6, r13+pc_c_sp, (char*)(r13+pc_CT_fork_name));
c         abort();
c       }
c       if (r6 > r13+pc_c_sp+pc_CT_C_stack_size) {
c         fprintf(stderr, "%s%s:%u c esp stack too small  (%lx > %lx) for process (%s)\n", FEBEMESSAGE, __FILE__, __LINE__,r6, r13+pc_c_sp+pc_CT_C_stack_size, (char*)(r13+pc_CT_fork_name));
c         abort();
c       }

c       if (r0 <= r13+pc_SF_start) {
c         fprintf(stderr, "%s%s:%u i960 pfp stack too small (%lx <= %lx) for process (%s)\n", FEBEMESSAGE, __FILE__, __LINE__,r0, r13+pc_SF_start, (char*)(r13+pc_CT_fork_name));
c         abort();
c       }
c       if (r0 >= r13+pc_c_sp) {
c         fprintf(stderr, "%s%s:%u i960 pfp stack too big (%lx >= %lx) for process (%s)\n", FEBEMESSAGE, __FILE__, __LINE__,r0, r13+pc_c_sp, (char*)(r13+pc_CT_fork_name));
c         abort();
c       }
c       if (r1 <= r13+pc_SF_start) {
c         fprintf(stderr, "%s%s:%u i960 sp stack too small (%lx <= %lx) for process (%s)\n", FEBEMESSAGE, __FILE__, __LINE__,r1, r13+pc_SF_start, (char*)(r13+pc_CT_fork_name));
c         abort();
c       }
c       if (r1 >= r13+pc_c_sp) {
c         fprintf(stderr, "%s%s:%u i960 sp stack too big (%lx >= %lx) for process (%s)\n", FEBEMESSAGE, __FILE__, __LINE__,r1, r13+pc_c_sp, (char*)(r13+pc_CT_fork_name));
c         abort();
c       }
.endif  /* CT_assumes_works */
# A K${,t}fork might wait because it can not allocate memory.  Must save the fork name.
c       r6 = CT_fork_tmp;
        st      r6,pc_CT_fork_tmp_exch(r13)

#
k$kernel:                               # -----------------------------
#
# --- If there are any active IRQs or events, process them. Continue until there
# --- are no more to process.
#
c       k_processed = 0;
c       while (1) {
c               UINT32  current_active, new_irqs;
#
c               k_upcount = kernel_up_counter;
c               current_active = ptr_xio3d_drvinfo->active;
c               new_irqs = current_active & ((1 << XIO3D_MAX_IRQS) - 1);
c               new_irqs &= ~k_processed;
c               k_processed |= current_active;
c               if (new_irqs) {
.ifdef HISTORY_KEEP
c CT_history("k$kernel: calling LI_SchedIRQ\n");
.endif  # HISTORY_KEEP
c                       LI_SchedIRQ(new_irqs);
c                       continue;
c               }
c               if (k_processed) {
c                       int return_code;
.ifdef HISTORY_KEEP
c CT_history("k$kernel: calling CT_write\n");
.endif  # HISTORY_KEEP
c                       return_code = CT_write(CT_xiofd, &k_processed, sizeof (k_processed));
c                       if (return_code != sizeof (k_processed)) {
c                               fprintf(stderr, "%sXIO3D write returned %d, errno %d\n", FEBEMESSAGE, return_code, errno);
c                               perror ("XIO3D write error");
c                       }
c               }
.ifdef FRONTEND
c               if (iscsimap) {
c                       tsl_ev_check (0, 0);
c               }
.endif
c               break;
c       }
#
# --- Locate highest priority ready process
#
c       kernel_switch_counter++;
#
# --- If we are in a round robin window, we use the current PCB as the
# --- starting point for checking for active tasks.  If we are not in the
# --- round robin mode, or we are just starting the round robin mode, then
# --- use the first PCB.  In the case of starting round robin, also change
# --- the state to indicate we just restarted.
#
        ld      K_rrstate,r12           # Get the round robin state
        ldconst 0x10000,r14             # Initialize to look for ready
        ldconst 0,r15                   # Clear candidate PCB
        cmpobe  rr_running,r12,.kr20    # Round robin active, use current PCB (r4)
#
        ld      K_pcborg,r4             # Set 1st process link (start over)
#
        cmpobne rr_start,r12,.kr20      # Jif not starting round robin (if idle, or normal)
        ldconst rr_running,r12          # Set to running
        st      r12,K_rrstate           # Start it
#
.kr20:
        mov     r15,r11                 # Save previous PCB
        mov     r4,r15                  # Swap next to current PCB
        ldl     pc_thd(r4),r4           # Get next PCB and current status/pri (r5)
#
# --- Check for end of PCB list.  If we hit the end, turn off the round robin
# --- mode and reset the timer window.
#
        ld      K_pcborg,r8             # Get Top of PCB list
        cmpobne r8,r4,.kr25             # Jif not wrapped
#
        ld      K_rrstate,r12           # Get current RR state
        st      r12,k_prevrr            # Save it for later
        ld      K_ii+ii_time,r8         # Bump the window for next restart
        ldconst rr_idle,r12
        addo    rr_delta,r8,r8
        st      r8,K_rrtimer            # Update next time to switch to RR

        # Change of RR state must be done AFTER update to RR timer.
        st      r12,K_rrstate           # Set round robin to idle
        cmpobl  r5,r14,.kr50            # Jif last process is ready - start it
        cmpobe  0,r13,.kr23             # Jif there was no active task
        ldob    pc_stat(r13),r8         # Get the status of the original process
        cmpobne pcrdy,r8,.kr22          # Jif process is not ready
        mov     r13,r15                 # Set original process as one to run
        b       .kr50                   # Go run original process
#
# --- No process to run.
#
.kr22:
        ldconst 0,r13                   # Set original process as not ready
.kr23:
#
# --- If we have been processing in round robin mode, then we need to jump
# --- back to the top of the loop and check all of the tasks for ready state.
# --- That is because the scheduler, in round robin mode, starts with the
# --- next task (next lower priority task) after the task that just ended.
# --- This means that if we find no one to run at this point, we haven't
# --- checked any tasks with a higher priority than the previous task,
# --- so we don't want to suspend until we have checked everyone.
#
        ld      k_prevrr, r7            # Get previous RR state
        cmpobne rr_idle,r7,k$kernel     # jif if been doing RR scheduling
#
# --- Indicate that the scheduler is suspending. Then, check to see if there are
# --- active interrupts, or if any messages have been received, since we started
# --- searching for a task to schedule. If either has happened, then, rather than
# --- suspending, we need to restart the k$kernel logic.
#
c       kernel_sleep = 1;
c       FORCE_WRITE_BARRIER;
c       if (ptr_xio3d_drvinfo->active == 0 && k_upcount == kernel_up_counter) {
.ifdef BACKEND

c               int return_code;
c               UINT32 k_pending;
.endif  # BACKEND
#
# --- Suspend until (1) we get an HBA interrupt, (2) we get a xio3d event, or (3) we
# --- get a timer interrupt.
#
.ifdef FRONTEND
.ifdef HISTORY_KEEP
c CT_history("k$kernel: calling tsl_ev_check\n");
.endif  # HISTORY_KEEP
c               tsl_ev_check(1, 0);
.endif  # FRONTEND

.ifdef BACKEND
.ifdef HISTORY_KEEP
c CT_history("k$kernel: calling CT_read\n");
.endif  # HISTORY_KEEP
c               return_code = CT_read(CT_xiofd, &k_pending, sizeof (k_pending));
c               if (return_code == -1 && errno == EINTR) {
c                       /* OK - interrupt received */
c               } else if (return_code != sizeof (k_pending)) {
# c                       fprintf(stderr, "%sXIO3D read returned %d, errno %d\n", return_code, errno);
c                       perror ("XIO3D read error");
c               }
.endif  # BACKEND
c       }
c       kernel_sleep = 0;
        b       k$kernel
#
.kr25:
        cmpobge r5,r14,.kr20            # Jif process not ready
#
# --- If in round robin, just run this one.  Otherwise, look for priority
# --- criteria.
#
        cmpobe  rr_running,r12,.kr50
#
        cmpobe  r13,r15,.kr20           # Jif original process was selected, if
                                        #  nothing else is ready, this task will
                                        #  be run
#
# --- Move this process behind all process of same priority
#
        ldob    pc_pri(r4),r8           # Get priority of next PCB
        shro    8,r5,r7                 # Isolate current PCB's priority
        cmpobne r7,r8,.kr50             # Jif priority don't match
        cmpobne 0,r11,.kr37             # Jif PCB not first PCB in list
#
# --- Handle case of first PCB in list.
#
        mov     r15,r6                  # Start with first PCB
#
# --- Find last PCB
.kr33:
        mov     r6,r11                  # Save current PCB
        ld      pc_thd(r6),r6           # Get next PCB
        cmpobne r15,r6,.kr33            # Jif PCB not found
#
# --- Remove process into PCB list
#
.kr37:
        st      r4,pc_thd(r11)          # Remove process into PCB list
#
# --- insert in priority order
#
.kr40:
        mov     r4,r3                   # Save current PCB
        ld      pc_thd(r4),r4           # Get next PCB
        ldob    pc_pri(r4),r8           # Get priority of next PCB
        cmpobe  r7,r8,.kr40             # Jif priority is the same
#
# --- Insert process into PCB list
#
        st      r4,pc_thd(r15)
        st      r15,pc_thd(r3)          # Insert process into PCB list
#
# --- Place selected process into execution
#
.kr50:
        ld      pc_pfp(r15),pfp         # Load previous frame ptr
.ifdef CT2_DEBUG
c CT_NOCHECK_STACK = 1;
.endif  # CT2_DEBUG
        bbc     0,r5,.kr60              # Jif no global reg restore req'd
#
        ldq     pc_g0(r15),g0           # Restore g0-g3
        ldq     pc_g4(r15),g4           # Restore g4-g7
        ldq     pc_g8(r15),g8           # Restore g8-g11
        ldt     pc_g12(r15),g12         # Restore g12-g14
#
.kr60:
        st      r15,K_xpcb              # Set up new executing PCB
#
/* Get info for new process. */
# Possible out of memory during fork.  Restore fork name pointer.
        ld      pc_CT_fork_tmp_exch(r15),r3
        st      r3,CT_fork_tmp
        ld      pc_CT_esp(r15),r3
        ld      pc_CT_ebp(r15),r4

.ifdef CT2_DEBUG
c       checkstackmemory("before switch to", K_xpcb);
# c       fprintf(stderr,"CXGH switching to (%s)\n", (char*)(r15+pc_CT_fork_name));
# c       fprintf(stderr, "r15 (pcb)=0x%-8.8lx, 960 stack[%lx - %lx], c stack[%lx - %lx], saved: esp=0x%-8.8lx, bsp=0x%-8.8lx\n", r15, r15+pc_SF_start, r15+pc_c_sp, r15+pc_c_sp, r15+pc_c_sp+pc_CT_C_stack_size, r3, r4);
# c       checkstackmemory("does complicated fprintf do it?", K_xpcb);
.endif  /* CT2_DEBUG */

.ifndef CT_assumes_works
c       if (r4 <= r15+pc_c_sp) {
c         fprintf(stderr, "%s%s:%u c ebp stack grew too big (%lx <= %lx) for process (%s)\n", FEBEMESSAGE,__FILE__, __LINE__,r4, r15+pc_c_sp, (char*)(r15+pc_CT_fork_name));
c         abort();
c       }
c       if (r4 > r15+pc_c_sp+pc_CT_C_stack_size) {
c         fprintf(stderr, "%s%s:%u c ebp stack too small  (%lx > %lx) for process (%s)\n", FEBEMESSAGE,__FILE__, __LINE__,r4, r15+pc_c_sp+pc_CT_C_stack_size, (char*)(r15+pc_CT_fork_name));
c         abort();
c       }
c       if (r3 <= r15+pc_c_sp) {
c         fprintf(stderr, "%s%s:%u c esp stack grew too big (%lx <= %lx) for process (%s)\n", FEBEMESSAGE,__FILE__, __LINE__,r3, r15+pc_c_sp, (char*)(r15+pc_CT_fork_name));
c         abort();
c       }
c       if (r3 > r15+pc_c_sp+pc_CT_C_stack_size) {
c         fprintf(stderr, "%s%s:%u c esp stack too small  (%lx > %lx) for process (%s)\n", FEBEMESSAGE,__FILE__, __LINE__,r3, r15+pc_c_sp+pc_CT_C_stack_size, (char*)(r15+pc_CT_fork_name));
c         abort();
c       }

c       if (r0 <= r15+pc_SF_start) {
c         fprintf(stderr, "%s%s:%u i960 pfp stack too small (%lx <= %lx) for process (%s)\n", FEBEMESSAGE,__FILE__, __LINE__,r0, r15+pc_SF_start, (char*)(r15+pc_CT_fork_name));
c         abort();
c       }
c       if (r0 >= r15+pc_c_sp) {
c         fprintf(stderr, "%s%s:%u i960 pfp stack too big (%lx >= %lx) for process (%s)\n", FEBEMESSAGE,__FILE__, __LINE__,r0, r15+pc_c_sp, (char*)(r15+pc_CT_fork_name));
c         abort();
c       }
# r1 (sp) is not set at this point -- do not check it.
.endif  /* CT_assumes_works */

/* Set the new stack registers */
c       set_esp_ebp(r3,r4);

.ifdef CT2_DEBUG
c CT_NOCHECK_STACK = 1;
.endif  # CT2_DEBUG
.ifdef HISTORY_KEEP
# Turn on history after scheduler.
c CT_history_task_name("entering task fork_name", (char*)(r15+pc_CT_fork_name), (PCB*)r15);
.ifdef CHECK_MEMORY_ALL
c check_memory_all();
.endif  # CHECK_MEMORY_ALL
c CT_history_disable_task((char*)(r15+pc_CT_fork_name));
.endif  # HISTORY_KEEP
.ifdef CT2_DEBUG
c check_pcb_chain();
.endif  # CT2_DEBUG
#
# --- Exit
#
.ifdef HISTORY_KEEP
.ifdef CT2_DEBUG
c CT_NOCHECK_STACK = 1;
.endif  # CT2_DEBUG
.endif  # HISTORY_KEEP
        ret                             # Restart process
#
#**********************************************************************
#
#  NAME: k$end/k$tend
#
#  PURPOSE:
#       To provide a common means of ending a process.  The first entry
#       point is used for a process created by K$fork and the second
#       entry point is used for a process created by K$tfork.
#
#  DESCRIPTION:
#       The process branches to this routine in order to end this
#       process.  This can be done from any routine level within a
#       process.  The uppermost level routine may alternatively
#       execute a return instruction instead.
#
#  CALLING SEQUENCE:
#       b       k$end/k$tend
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
k$end:                                  # -----------------------------
k$tend:                                 # -----------------------------
# c       fprintf(stderr,"k$end (%s)\n", (char*)(K_xpcb + pc_CT_fork_name));
.ifdef CT2_DEBUG
c       checkstackmemory("before ending", K_xpcb); # Check lower words still not changed.
#        c       fprintf(stderr, "k$end processing %s\n",(char *)(K_xpcb + pc_CT_fork_name));
.endif  # CT2_DEBUG
        ldconst 0,r3                    # Clear executing PCB
        ld      K_xpcb,g0               # g0 = PCB
.ifdef HISTORY_KEEP
.ifdef CT2_DEBUG
c CT_NOCHECK_STACK = 1;
.endif  # CT2_DEBUG
c CT_history_task_name("ending task fork_name", (char *)(K_xpcb + pc_CT_fork_name), (PCB*)g0);
# Turn off history for rest, and during scheduler.
c CT_HISTORY_OFF_NOW();
.endif # HISTORY_KEEP
        st      r3,K_xpcb
#
# --- Update II statistics
#
        lda     K_ii,r15                # Get II structure
        ld      ii_pcbcur(r15),r3       # Adjust current PCB count
        subo    1,r3,r3
        st      r3,ii_pcbcur(r15)
#
# --- Remove PCB from PCB thread
#
        ld      K_pcborg,r4             # Get 1st PCB in thread
.ifdef CT2_DEBUG
c CT_NOCHECK_STACK = 1;
.endif  # CT2_DEBUG
        cmpobne g0,r4,.en20
#
        ld      pc_thd(r4),r5           # Link to next PCB
        st      r5,K_pcborg             # Update 1st PCB thread
.en20:
        mov     r4,r3                   # Save previous PCB
        ld      pc_thd(r4),r4           # Link to next PCB
.ifdef CT2_DEBUG
c CT_NOCHECK_STACK = 1;
.endif  # CT2_DEBUG
        cmpobne g0,r4,.en20             # Jif previous PCB link not found
#
        ld      pc_thd(g0),r5           # Unlink PCB (g0 is r4)
        st      r5,pc_thd(r3)           # Store our forward into previous forward
#
# --- Release memory for this PCB -------------------------------------
#
#     A deferred memory release is used at this time because the stack
#     associated with the terminating PCB is still in use until the
#     next PCB is placed into execution by the scheduler.
#
c       if (K_delayed_pcb_free) {
c           free_pcb((UINT32)K_delayed_pcb_free);
c       }
c       K_delayed_pcb_free = (UINT32 *)g0;
#
# --- Exit
#
        ld      K_pcborg,r4             # Get 1st PCB
        mov     r4,r13
.ifdef CT2_DEBUG
c CT_NOCHECK_STACK = 1;
.endif  # CT2_DEBUG
        b       k$kernel
#
#**********************************************************************
#
#  NAME: TaskSleepNoSwap
#
#  PURPOSE:
#       To provide a common means of delaying for a specified number
#       of microseconds/clocks.
#
#  DESCRIPTION:
#       Timer 1 is used manually without timer interrupts to guarantee
#       that the specified delay period in microseconds/clocks has been
#       met.
#       NOTE: Timer count counts down.
#
#  CALLING SEQUENCE:
#       call    TaskSleepNoSwap
#
#  INPUT:
#       g0 = usec/clock delay
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
TaskSleepNoSwap:
        cmpobe  0,g0,.fd100             # Jif invalid delay
#
# --- Set up timer 1 count (100*10ns)
#
# Wait for the specified time. We convert the delay time into clock cycles
# and add it to the current time (in clock cycles). Then we wait for the
# clock to reach our end value. We can use only the lower 32 bits of the
# timer because that will allow a maximum wait time of about 2.15 billion
# clock cycles (about .7 seconds on a 3GHz processor), which is far longer
# than we can afford to suspend all processing in a delay loop.
#
# NOTE:  The unsigned subtraction in the 'while' statement takes care of
# register wrap. It is *not* correct to use
# 'while (clock_end_value > clock_curr_value)' because it fails when the
# 32 bit field wraps (about every 1.4 seconds on a 3GHz processor).
#
c       {
c               static UINT32 clock_end_value, clock_curr_value;
c               clock_curr_value = get_tsc_l();
c               clock_end_value = clock_curr_value + g0 * ct_cpu_speed;
c               do {
c                       clock_curr_value = get_tsc_l();
c               } while ((INT32)(clock_end_value - clock_curr_value) > 0);
c       }
#
# --- Exit
#
.fd100:
        ret
#
#**********************************************************************
#
#  NAME: k$dfmexec
#
#  PURPOSE:
#       To provide an automated means of physically releasing any memory
#       which was scheduled for release through the deferred memory
#       release mechanism.  This mechanism is typically used to permit
#       the release of memory from interrupt level routines.
#
#  DESCRIPTION:
#       The deferred queue is snatched atomically using the atmod
#       instruction to close down the existing queue.  Each memory
#       segment within this closed down queue is released back to the
#       system.  This queue is organized with free memory blocks
#       containing ascending addresses.
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
k$dfmexec:
#
# --- Preload static constants
#
        ldconst 128,r14                 # Preload wait time
        b       .fm10
#
# --- Exchange processes
#
#     Give up control until any deferred free memory release actually
#     occurs.
#
.fm10:
        mov     r14,g0
        call    K$twait                 # Sleep for awhile
#
c       release_deferred_memory();      # Release all deferred free mem
        b       .fm10
#
#**********************************************************************
#
#  NAME: K$q
#
#  PURPOSE:
#       To provide a means of queuing a request without wait.
#
#  DESCRIPTION:
#       The ILT is directly passed to the specified queuing routine.
#       The queuing routine then returns directly to the caller.
#
#  CALLING SEQUENCE:
#       call    K$q
#
#  INPUT:
#       g0 = queuing routine
#       g1 = ILT
#       g2 = completion routine
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g1
#
#**********************************************************************
#
.ifdef BACKEND
K$q:
.endif  # BACKEND
EnqueueILT:
#
# --- Save completion routine in ILT
#
        st      g2,il_cr(g1)            # Save completion routine
#
# --- Advance ILT to next level
#
        mov     0,r3
        st      r3,ILTBIAS+il_fthd(g1)  # Close link
        lda     ILTBIAS(g1),g1          # Advance to next level
#
# --- Queue request
#
        bx      (g0)                    # Queue request
#
#**********************************************************************
#
#  NAME: K$cque
#
#  PURPOSE:
#       To provide a common means of queuing I/O requests for all
#       system modules.
#
#  DESCRIPTION:
#       The ILT and associated request packet are queued to the tail
#       of the specified executive queue.  The corresponding executive
#       is then activated to process this request.
#
#       K$cque may be called from either the process or interrupt level.
#
#  CALLING SEQUENCE:
#       b       K$cque
#
#  INPUT:
#       g1 = ILT
#       r11 = QU
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
K$cque:
#
# --- Get entire queue header
#
        ldconst 0,r8                    # Prepare to close forward thread
        ldq     qu_head(r11),r12        # Get queue head, tail, count
                                        #  and executive PCB
#
# --- Adjust queue count and set new queue tail
#
        mov     r13,r9                  # Get previous queue tail
        lda     1(r14),r14              # Adjust queue count
        mov     g1,r13                  # Set new queue tail
        cmpobne 0,r9,.qu10              # Jif queue not previously empty
#
# --- Insert ILT into empty queue
#
        mov     g1,r12                  # Set new queue head
        mov     r11,r9                  # Set up backward link
#
# --- Ready executive process if necessary
#
        cmpobe  0,r15,.qu20             # Jif the PCB isn't set up yet
        ldob    pc_stat(r15),r5         # Get current process status
        cmpobne pcnrdy,r5,.qu20         # Jump if executive is not not rdy
#
.ifdef HISTORY_KEEP
c CT_history_pcb("K$cque setting ready pcb", r15);
.endif  # HISTORY_KEEP
        mov     pcrdy,r4                # Get ready status
        stob    r4,pc_stat(r15)         # Ready process
        b       .qu20
#
# --- Insert ILT into queue that is not empty
#
.qu10:
        st      g1,il_fthd(r9)          # Link request to last entry
#
# --- Update queue header
#
.qu20:
        stt     r12,qu_head(r11)        # Update queue head, tail and
                                        #  count
        stl     r8,il_fthd(g1)          # Update ILT fwd/bwd threads
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: K$comp
#
#  PURPOSE:
#       To provide a common means of completing an ILT request back
#       to the caller.
#
#  DESCRIPTION:
#       The ILT is adjusted to the previous level, that of the caller.
#       The ILT is checked to see if the ILT was issued w/ wait.  If
#       so, the associated PCB is made ready.  Otherwise, the completion
#       routine is called.
#
#  CALLING SEQUENCE:
#       call    K$comp
#
#  INPUT:
#       g1 = ILT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g1
#
#**********************************************************************
#
.ifdef BACKEND
# void K_comp(ILT * ilt);
# C Access
        .globl  K_comp
K_comp:
.endif # BACKEND
K$comp:
        lda     -ILTBIAS(g1),g1         # Return to previous ILT level
        ld      il_cr(g1),r3            # Call completion routine
# --- Execute completion routine
        bx      (r3)

.ifdef BACKEND
# void complete_scr(void *, SCR *scr);
        .globl  complete_scr
complete_scr:
        bx      (g0)
.endif # BACKEND

#**********************************************************************
CAWRAP_LL_TargetTaskCompletion:
c       asm("   .globl  CAWRAP_LL_TargetTaskCompletion");
c       asm("CAWRAP_LL_TargetTaskCompletion:    ");
        PushRegs(r3)
        call    LL_TargetTaskCompletion
        PopRegsVoid(r3)
        ret

#**********************************************************************
        .globl  L$que
        .globl  LL_QueueMessageToSend
L$que:
        PushRegs(r3)

        # Callback functions specified in ILTs through this routine are
        # assembly language routines. If the callback routine is written
        # in C, then call LL_QueueMessageToSend, rather than L$que.
        #
        # Set BIT31 in the callback to indicated that this is an assembly
        # language callback routine.

c       g0 = g1;
c       ((ILT *)g0)->cr = (((ILT *)g0)->cr != NULL) ? (void *)((UINT32)((ILT *)g0)->cr | BIT31) : NULL;
        call    LL_QueueMessageToSend

        PopRegs(r3)                     # g0 is return.
        ret

#**********************************************************************
        .globl  L$send_packet
        .globl  LL_SendPacket
L$send_packet:
        PushRegs(r3)

        # Callback functions specified through this routine are
        # assembly language routines. If the callback routine is written
        # in C, then call LL_SendPacket, rather than L$send_packet.
        #
        # Set BIT31 in the callback to indicated that this is an assembly
        # language callback routine.

c       if (g5 != 0) g5 |= (UINT32)BIT31;
        call    LL_SendPacket

        PopRegsVoid(r3)
        ret
#
#**********************************************************************
#
#  NAME: K$qw
#
#  PURPOSE:
#       To provide a means of queuing a request with wait.
#
#  DESCRIPTION:
#       The ILT is directly passed to the specified queuing routine.
#       After the queuing routine returns, a return is made to the
#       caller.
#
#       This routine may only be called from the process level.
#
#  CALLING SEQUENCE:
#       call    K$qw
#
#  INPUT:
#       g0 = queuing routine
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
.ifdef BACKEND
K$qw:
.endif  /* BACKEND */
EnqueueILTW:
        mov     g1,r15                  # Save g1
#
# --- Save PCB in ILT
#
        ld      K_xpcb,r3               # Save PCB
        ldconst pciowait,r4             # Set I/O wait status
        st      r3,il_pcb(g1)
        lda     QWComp,r5               # Set up completion address
        stob    r4,pc_stat(r3)
        st      r5,il_cr(g1)
#
# --- Advance ILT to next level
#
        lda     ILTBIAS(g1),g1          # Advance to next level
        mov     0,r4                    # Close link
        st      r4,il_fthd(g1)
#
# --- Queue request
#
        callx   (g0)                    # Queue request
#
# --- Relinquish control until request completes
#
        mov     r15,g1                  # Restore g1
        b       K$xchang                # Give up control
#
#**********************************************************************
#
#  NAME: K$qwlink
#
#  PURPOSE:
#       To provide a means of queuing a request with wait without advancing
#       the ILT pointer
#
#  DESCRIPTION:
#       This function is exactly like K$qw except it does not advance the
#       ILT pointer before calling the queueing routine.
#
#       The ILT is directly passed to the specified queuing routine.
#       After the queuing routine returns, a return is made to the
#       caller.
#
#       This routine may only be called from the process level.
#
#  CALLING SEQUENCE:
#       call    K$qwlink
#
#  INPUT:
#       g0 = queuing routine
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
K$qwlink:
        mov     g1,r15                  # Save g1
#
# --- Save PCB in ILT
#
        ld      K_xpcb,r3               # Save PCB
        ldconst pciowait,r4             # Set I/O wait status
        st      r3,il_pcb(g1)
        lda     QWComp,r5               # Set up completion address
        stob    r4,pc_stat(r3)
        st      r5,il_cr(g1)
        mov     0,r4                    # Close link
        st      r4,il_fthd(g1)
#
# --- Queue request
#
        callx   (g0)                    # Queue request
#
# --- Relinquish control until request completes
#
        mov     r15,g1                  # Restore g1
        b       K$xchang                # Give up control
#
#**********************************************************************
#
#  NAME: QWComp
#
#  PURPOSE:
#       To provide a canned completion routine to ready a process that
#       has previously performed a queue with wait operation.
#
#  DESCRIPTION:
#       The PCB of the caller is extracted from the ILT and the status
#       of that process is set to ready.
#
#  CALLING SEQUENCE:
#       call    QWComp
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
QWComp:
#
# --- Ready calling process
#
        ld      il_pcb(g1),r3           # Get PCB
        mov     pcrdy,r4                # Set process to ready
.ifdef HISTORY_KEEP
c CT_history_pcb("QWComp setting ready pcb", r3);
.endif  # HISTORY_KEEP
        stob    r4,pc_stat(r3)
        ret
#
#**********************************************************************
#
#  NAME: k$timer
#
#  PURPOSE:
#       To provide a process level means of handling the K$twait timed
#       wait request by cooperative processing with the timer interrupt
#       routine k$tint.
#
#  DESCRIPTION:
#       This process is made ready by the timer 1 interrupt routine for
#       every timer interrupt.  The PCB thread is checked to see  if any
#       processes are in timer wait.  If so, the timeout count is
#       decremented.  When the timeout period has expired, the process
#       is made ready.
#
#       This process snatches the time period from the interrupt routine
#       by use of the atmod instruction.  This technique insures that
#       the time period is synchronized between these two routines.  The
#       atmod atomically retrieves/clears the time period without having
#       to disable interrupts.
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
k$timer:
#
# --- Preload constants
#
        ldconst pcrdy,r15               # Get process ready status
        ldconst pctwait,r14             # Timed wait status
        ld      K_xpcb,r13              # Get PCB
        ldconst pcnrdy,r12              # Get not ready status
        lda     K_time,r11              # Get address of time period
        ldconst ~0,r10
        ldconst 0,r3                    # Init counter to update idle time
                                        #  statistics once per second
#
# --- Exchange processes
#
.ti10:
        call    K$qxchang               # Exchange processes
#
# --- Process all pending pause requests
#
        ld      K_pcborg,r9             # Get PCB origin
        stob    r12,pc_stat(r13)        # Set this process not ready
        cmpobe  0,r9,.ti10              # Jif origin undefined
#
        mov     0,r8
c       CT_atomic_xchg((unsigned long *)r11,&r8)
        mov     r9,r7                   # Set terminal PCB
#
# --- Check next process for timeout
#
.ti20:
        ldt     pc_thd(r9),r4           # r4=pc_thd
                                        # r5=pc_global/pc_stat/pc_pri
                                        # r6=pc_time
        extract 16,8,r5                 # Isolate process status
        cmpobne r14,r5,.ti30            # Jif not timer wait
#
        subo    r8,r6,r6                # Decrement timeout
        st      r6,pc_time(r9)          # Update timeout
        cmpibl  0,r6,.ti30              # Jif not expired
#
# --- Set process ready
#
.ifdef HISTORY_KEEP
c CT_history_pcb("k$timer setting ready pcb", r9);
.endif  # HISTORY_KEEP
        stob    r15,pc_stat(r9)         # Set process ready
#
# --- Link to next process
#
.ti30:
        mov     r4,r9                   # Link to next entry
        cmpobne r4,r7,.ti20             # Jif more
#
# --- Idle Time Statistics follow (only do once a second)
#
        addo    1,r3,r3                 # Increment the idle counter
        cmpobg  (1000/QUANTUM),r3,.ti10 # Jif not time yet to do idle statistics
        ldconst 0,r3                    # Reset the counter
#
#   Update the percentage of processor utilization
#
c       r5 = L_CalcIdle();
        stob    r5,K_ii+ii_utzn         # Store the % to II for EPC
.ifdef BACKEND
        divo    10,r5,r5                # r5 should be 0-9
        ldconst 0,r4
        cmpobl  10,r5,.ti32             # Jif bogus percentage
        ldob    V_skptblpu(r5),r4       # Get the skip counter
.ti32:
        stob    r4,V_skipthrsh          # Update the skip counter
.endif  # BACKEND
#
# --- All done processing during this pass
#
        b       .ti10

#******************************************************************************
#
#  NAME: KernelDispatch
#
#  PURPOSE:
#       To provide a common means of calling assembly language routines
#       from C code in the auto-convert environment.
#
#  DESCRIPTION:
#       This function provides a convenient mechanism for enabling the
#       "magic sauce" necessary when calling an assembly language routine
#       from C code in the auto-convert environment. Since these routines
#       are called through an indirect function pointer, the normal
#       mechanisms for adding the sauce as bypassed. So, when the C routine
#       detects that it is calling an assembly routine through an indirect
#       pointer, it called KernelDispatch.
#
#       KernelDispatch simply branches to the function provided. This allows the
#       appropriate setup for the call (to KernelDispatch) and the return (from
#       the called function) to occur.
#
#  CALLING SEQUENCE:
#       KernelDispatch (p1, p2, p3, p4);
#
#  INPUT:
#       g0 - g3 = parameters used by the called function
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       None
#
#******************************************************************************
#
KernelDispatch:
        ld      il_cr(g1),r3            # get completion routine
        bx      (r3)
#
#******************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
