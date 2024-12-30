/* $Id: xk_init.c 146537 2010-08-27 18:09:08Z m4 $ */
/**
******************************************************************************
**
**  @file       xk_init.c
**
**  @brief      Module to initialize the CCB on Linux.
**
**  Copyright (c) 2004-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sched.h>
#include <signal.h>

#include "xk_init.h"

#include "ccb_hw.h"
#include "debug_files.h"
#include "errorCodes.h"
#include "error_handler.h"
#include "idr_structure.h"
#include "L_Signal.h"
#include "LL_LinuxLinkLayer.h"
#include "nvram.h"
#include "XIO_Std.h"
#include "xk_kernel.h"
#include "xk_mapmemfile.h"
#include "timer.h"
#include "trace.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "memory.h"

/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/
#define get_tsc()       ({ unsigned long long __scr; \
        __asm__ __volatile__("rdtsc" : "=A" (__scr)); __scr;})

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
INT32       pamPid = 0;
static struct fms K_ncdram_fms;     /* Memory management statistics. */

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static void XK_PamHeartbeatTask(TASK_PARMS *parms);

/*
******************************************************************************
** Not extern-ed in header files.
******************************************************************************
*/
extern void SETUP_linux(void);
extern UINT32 SHMEM_START;
extern UINT32 SHMEM_END;
extern struct fmm K_ncdram;         /* Free shared memory management structure  */
extern void k_init_mem(struct fmm *, void *, UINT32);
extern struct fms P_cur;            /* Memory management statistics. */
extern struct fmm P_ram;            /* Memory management structure. */
extern UINT64 xkCpuSpeed;           /* Speed of cpu (like 3200 = 3.2gHz) */

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Initialize the hypernode CCB on Linux.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void XK_Init(void)
{
    void       *mem = NULL;
    void       *localHeap = (void *)MAKE_DEFS_CCB_ONLY_MEMORY;
    UINT32      localHeapSize = MAKE_DEFS_CCB_ONLY_SIZE;
    void       *sharedHeap = (void *)&SHMEM_END;
    UINT32      sharedHeapSize = SIZE_CCB_LTH - ((UINT32)&SHMEM_END - CCB_PCI_START);
    UINT32      sigIndex = 0;
    struct sched_param schedparam;

    /*
     * Make sure we have the proper permissions to continue. We must have root
     * permissions at startup.  We will dummy down to wookiee user.
     */
    if ((getuid() != WOOKIEE_ADMIN_UID) || (getgid() != WOOKIEE_ADMIN_GID))
    {
        dprintf(DPRINTF_DEFAULT, "XK_Init FATAL ERROR (You must be running as root to start process)\n");
        DeadLoop(deadErrCode(ERR_EXIT_INVALID_USER), TRUE);
    }

    dprintf(DPRINTF_DEFAULT, "CCB runtime code starting...\n");

    /*
     * Set up default signals
     */
    for (sigIndex = 1; sigIndex < L_SIGNAL_MAX_SIGNALS; ++sigIndex)
    {
        switch (sigIndex)
        {
                /*
                 * Can not catch SIGKILL or SIGSTOP
                 */
            case SIGKILL:
            case SIGSTOP:
                continue;

                /*
                 * Linux threads uses the first 3 real-time signals
                 */
            case __SIGRTMIN:
            case (__SIGRTMIN + 1):
            case (__SIGRTMIN + 2):
                continue;

                /*
                 * These standard signals should not be caught, since
                 * they are used for standard communications.
                 */
            case SIGCHLD:      /* Child status has changed */
            case SIGCONT:      /* Continue */
            case SIGTSTP:      /* Keyboard stop */
            case SIGTTIN:      /* Background read from tty */
            case SIGTTOU:      /* Background write to tty */
            case SIGURG:       /* Urgent condition on socket */
            case SIGWINCH:     /* Window size change */
                continue;

                /*
                 * Cases to ignore.
                 */
            case SIGPIPE:      /* Broken pipe */
                signal(sigIndex, SIG_IGN);
                break;

                /*
                 * Trap the following signals.
                 */
            case SIGHUP:       /* Hangup */
            case SIGINT:       /* Interrupt */
            case SIGQUIT:      /* Quit */
            case SIGILL:       /* Illegal instruction */
            case SIGTRAP:      /* Trace/breakpoint trap */
            case SIGABRT:      /* Abort */
            case SIGBUS:       /* Bus error */
            case SIGFPE:       /* Floating point exception */
            case SIGUSR1:      /* User defined signal #1 */
            case SIGSEGV:      /* Segmentation violation */
            case SIGUSR2:      /* User defined signal #2 */
            case SIGALRM:      /* Alarm clock */
            case SIGTERM:      /* Termination */
            case SIGSTKFLT:    /* Stack fault */
            case SIGXCPU:      /* CPU time limit exceeded */
            case SIGXFSZ:      /* File size limit exceeded */
            case SIGVTALRM:    /* Virtual alarm clock */
            case SIGPROF:      /* Profiling alarm clock */
            case SIGIO:        /* I/O now possible */
            case SIGPWR:       /* Poiwer failure restart */
            case SIGSYS:       /* Bad system call */
            default:           /* All remaining real-time interrupts */

                /*
                 * Add the signal to our handler
                 */
                L_SignalHandlerAdd(sigIndex, DeadLoopInterrupt, true);

                break;
        }
    }

    /* Test Coordinated Boot Flag */
    if (TestCoordinatedBootGoodFlag() != TRUE)
    {
        dprintf(DPRINTF_DEFAULT, "ERROR - IDRData structure not working\n");
    }

    /* Display the firmware header information */
    {
        char        buffer[80] = { 0 };

        sprintf(buffer, "%c%c%c%c/%c%c%c%c built by '%c%c%c%c' on %02x/%02x/%04x",
                (UINT8)(fwHeader.revision >> 0),
                (UINT8)(fwHeader.revision >> 8),
                (UINT8)(fwHeader.revision >> 16),
                (UINT8)(fwHeader.revision >> 24),
                (UINT8)(fwHeader.revCount >> 0),
                (UINT8)(fwHeader.revCount >> 8),
                (UINT8)(fwHeader.revCount >> 16),
                (UINT8)(fwHeader.revCount >> 24),
                (UINT8)(fwHeader.buildID >> 0),
                (UINT8)(fwHeader.buildID >> 8),
                (UINT8)(fwHeader.buildID >> 16),
                (UINT8)(fwHeader.buildID >> 24),
                fwHeader.timeStamp.month,
                fwHeader.timeStamp.date, fwHeader.timeStamp.year);

        dprintf(DPRINTF_DEFAULT, "FW Header: %s\n", buffer);
    }

    /* Initialize the rest of the shared memory. */
    SETUP_linux();

    /* Configure the heap manager. */
    pStartOfHeap = (UINT8 *)localHeap;
    dprintf(DPRINTF_DEFAULT, "Configuring heap at %p, length = 0x%08X (%d)\n",
            localHeap, localHeapSize, localHeapSize);
    dprintf(DPRINTF_DEFAULT, "Configuring shared heap at %p, length = 0x%08X (%d)\n",
            sharedHeap, sharedHeapSize, sharedHeapSize);

    K_ncdram.fmm_fms = &K_ncdram_fms;                   // Initialize shared memory statistics area.
    k_init_mem(&K_ncdram, sharedHeap, sharedHeapSize);  // Initialize shared memory.

    P_ram.fmm_fms = &P_cur;                             // Initialize private memory statistics area.
    k_init_mem(&P_ram, localHeap, localHeapSize);       // Initialize private memory.

    /* Initialize NVRAM */
    dprintf(DPRINTF_DEFAULT, "Initializing NVRAM...\n");
    mem = MEM_InitMapFile("/opt/xiotech/ccbdata/CCB_NVRAM.mmf", SIZE_NVRAM, 0, NULL);
    if (!mem)
    {
        dprintf(DPRINTF_DEFAULT, "ERROR Initializing NVRAM\n");
        abort();
    }
    dprintf(DPRINTF_DEFAULT, "NVRAM at 0x%08X\n", (UINT32)mem);
    HW_InitNVRAM(mem);

    /* Initialize Flash */
    dprintf(DPRINTF_DEFAULT, "Initializing FLASH...\n");
    mem = MEM_InitMapFile("/opt/xiotech/ccbdata/CCB_FLASH.mmf", SIZE_FLASH, 0xFF, NULL);
    if (!mem)
    {
        dprintf(DPRINTF_DEFAULT, "ERROR Initializing FLASH\n");
        abort();
    }
    dprintf(DPRINTF_DEFAULT, "Flash at 0x%08X\n", (UINT32)mem);
    HW_InitFlash(mem);

    /* Set priority */
    schedparam.__sched_priority = XIO_LINUX_PRIORITY;
    dprintf(DPRINTF_DEFAULT, "Setting CCB priority to %d within scheduling policy SCHED_FIFO\n",
            schedparam.__sched_priority);
    if (sched_setscheduler(0, SCHED_FIFO, &schedparam) != 0)
    {
        dprintf(DPRINTF_DEFAULT, "FAILED to set CCB priority\n");
    }

    /* Initialize the kernel */
    dprintf(DPRINTF_DEFAULT, "Initializing Kernel...\n");
    XK_InitKernel();

    TimerEnable();          /* Start the 125ms timer */

    /* Fork PAM HB Task */
    dprintf(DPRINTF_DEFAULT, "Forking PAM HB Task...\n");
    TaskCreate(XK_PamHeartbeatTask, NULL);
}


/**
******************************************************************************
**
**  @brief      Heartbeat the Platform Application Monitor.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/

#define GetTodMS(a) ((((a)->tv_sec - startSec) * 1000) + ((a)->tv_usec / 1000))
#define XK_PAM_HB_TO    1000            /* One second. */

static void XK_PamHeartbeatTask(UNUSED TASK_PARMS *parms)
{
    UINT64      nowTime;                /* tsc register */
    UINT64      lastTime;               /* tsc register */
    UINT64      tmIntv = XK_PAM_HB_TO;  /* In milliseconds. */
    UINT64      waittime;

    if (pamPid == 0)
    {
        return;
    }

    /* Get initial data */
    nowTime = get_tsc();

    while (1)
    {
        /* Send HB to PAM */
        if (L_SignalProcess(pamPid, XIO_PLATFORM_SIGNAL, CCB_2_PAM_HB) != 0)
        {
            tmIntv = 250;
            TraceEvent(TRACE_PAM_HB, errno);
            LogMessage(LOG_TYPE_DEBUG, "PAM HB FAILED (%d-%s): RETRYING!",
                       errno, strerror(errno));
        }
        else
        {
            TraceEvent(TRACE_PAM_HB, 0);
            if (tmIntv != XK_PAM_HB_TO)
            {
                LogMessage(LOG_TYPE_DEBUG, "PAM heartbeat OK");
            }
            tmIntv = XK_PAM_HB_TO;
        }

        /* Set the last time. */
#ifdef M4_ABORT
        waittime = (tmIntv * 10) * (xkCpuSpeed * 1000); // Ten times longer is OK.
#else   /* M4_ABORT */
        waittime = tmIntv * (xkCpuSpeed * 1000);
#endif  /* M4_ABORT */
        lastTime = nowTime + waittime;

        /* Sleep for tmIntv ms */
        TaskSleepMS(tmIntv);

        /*
         * We slept for tmIntv. We set lastTime to be the time we were wanting
         * to be awakened. Make sure we are less than tmIntv in that range.
         */
        nowTime = get_tsc();
        if ((lastTime + waittime) < nowTime)
        {
            LogMessage(LOG_TYPE_DEBUG, "CCB %s - wakened (%llu ms) LATE!",
                       __func__, (nowTime - lastTime)/(xkCpuSpeed * 1000));
#ifdef M4_ABORT
            dprintf(DPRINTF_DEFAULT, "%s: CCB wakened %llu ms late\n", __func__, (nowTime - lastTime)/(xkCpuSpeed * 1000ULL));
            abort();
#endif  /* M4_ABORT */
        }
    }
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
