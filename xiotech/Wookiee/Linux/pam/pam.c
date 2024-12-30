/* $Id: pam.c 147396 2010-09-10 19:01:55Z m4 $ */
/**
******************************************************************************
**
**  @file       pam.c
**
**  @brief      Platform Application Monitor.
**
**  PAM is responsible of starting the platform code, and monitoring the
**  platform afterwards. PAM will take appropriate action based on the type
**  of failure. PAM will also be responsible for setting up and monitoring
**  the watchdog timer.
**
**  Copyright (c) 2004-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

// The following is to turn on date and ps appending to a file to debug pam task termination.
// #define  EXTRA_PAM_DEBUGGING

#include <pthread.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <time.h>
#include <asm/bitops.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>

#include "L_Misc.h"
#include "L_Signal.h"
#include "LKM_layout.h"
#include "li_pci.h"
#include <sys/pci.h>
#ifndef PCI_VENDOR_ID_QLOGIC
#define PCI_VENDOR_ID_QLOGIC          0x1077
#endif /* PCI_VENDOR_ID_QLOGIC */

#include "debug_files.h"
#include "errorCodes.h"
#include "XIO_Const.h"

#include "XIO_Std.h"
#include "mach.h"

#include <fcntl.h>
#include <stdint.h>
#define __u32 uint32_t
#define __u8 uint8_t
#include <linux/watchdog.h>

#ifdef ENABLE_NG_LED
#include "led_control.h"
#endif

#include "logdef.h"

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/

/*
 * This define allows a routine to be declared as not returning.
 */
#define NORETURN __attribute__((noreturn))

/**
**  @name   Constants can be grouped and a group name applied.
**/

#define TASK_ISCSI  0x00        /* iSCSI Task                                 */
#define TASK_CCB    0x01        /* CCB Task                                   */
#define TASK_FE     0x02        /* FE Task                                    */
#define TASK_BE     0x03        /* BE Task                                    */
#define TASK_SV     0x04        /* Sys Call Server Task                       */

#define ERROR_CCB   0x10        /* CCB Failed to start                        */
#define ERROR_FE    0x20        /* FE Failed to start                         */
#define ERROR_BE    0x30        /* BE failed to start                         */
#define ERROR_SV    0x40        /* Sys Call Server failed to start            */
#define ERROR_BVM   0x50        /* BVM failed to start                        */
#define ERROR_ISCSI 0x60        /* iscsid failed to start                     */

#define ERROR_SHM   0x20        /* Error Clearing Shared Memory               */
#define ERROR_SGN   0x21        /* Error Setting Signal Handler               */

#define MON_SLP_TM  1           /* Time for monitor to sleep between checks   */
#define CCB_HB_TMT  10          /* Length of time to allow for CCB to HB      */

#define PCIMAXDEV   21          /* Number of devices to use for scanbus       */

#define CN_DOWN_REBOOT        0x00      /* Controller reboot            */
#define CN_DOWN_SHUTDOWN      0x01      /* Controller shutdown          */
#define CN_DOWN_DEADLOOP      0x02      /* Controller deadloop          */
#define CN_DOWN_BVM           0x03      /* Controller quick restart     */

#define WD_ERRTRAP_TIMEOUT    255       /* Errtrap timeout 4.25 minutes */

#define DPRINTF_PAM           DPRINTF_DEFAULT
#define DPRINTF_PAM_ALLOWED   MD_DPRINTF_DEFAULT

#define PAM_LOGFILE     "/var/log/xiotech/pam.log"      /* PAM Log File          */
#define REBOOT_NEEDED_FILE "/opt/xiotech/reboot_required"       /* A signal from the
                                                                 * bvm that a reboot
                                                                 * is required.   */
#define SYS_WD          "/dev/watchdog" /* System watchdog */

/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/
#define clearCcbHbTimers()      ccbHB = ccbLastHB = ccbTmMsd = 0

#define ArraySize(a)            (sizeof(a)/sizeof(a[0]))
#define TASK_MAX                ArraySize(tasks)

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/

/**
** The information pam needs for controlling a task that it starts/stops.
**/
typedef struct _PAM_PROCESS_DATA
{
    pid_t       pid;            /* Pid of process                     */
    INT32       status;         /* Status returned from waitpid       */
    INT32       stat;           /* Internal PAM status                */
    INT32       errCd;          /* Error Code to exit with            */
    char       *execArgs[4];    /* Exec Args                          */
} PAM_PROCESS_DATA;

/**
** Structure for starting a thread.
**/
typedef struct _PAM_TASK_PARMS
{
    void        (*funcPtr) (TASK_PARMS *);
    TASK_PARMS  parms;
} PAM_TASK_PARMS;

/*
******************************************************************************
** Private variables
******************************************************************************
*/
static char pamProcBuf[32];     /* Ascii argument of pid for CCB. */

/**
** Information about tasks, and tasks to be run/controlled by pam.
**/
static PAM_PROCESS_DATA tasks[] = {
    {0, 0, 0, ERROR_ISCSI, {"iscsid", "-d2", 0, 0}},    /* iSCSId Task     */
    {0, 0, 0, ERROR_CCB, {"ccbrun", pamProcBuf, 0, 0}}, /* CCB Task        */
    {0, 0, 0, ERROR_FE, {"Front.t", 0, 0, 0}},          /* Front End Task  */
    {0, 0, 0, ERROR_BE, {"Back.t", 0, 0, 0}},           /* Back End Task   */
    {0, 0, 0, ERROR_SV, {"syssrv", 0, 0, 0}}            /* Sys Server Task */
};

/**
** BVM Task Info
**/
static PAM_PROCESS_DATA bvmTask = {
    0, 0, 0, ERROR_BVM, {"/opt/xiotech/bvm", 0, 0, 0}
};

static INT32 pamProcPid = 0;            /* PID of main task         */
static volatile UINT32 ccbHB = 0;       /* CCB HB                   */
static UINT32 ccbLastHB = 1;            /* CCB Last HB              */
static UINT16 ccbTmMsd = 0;             /* Sec since CCB Last HB    */
static INT32 kMemOpen = 0;              /* Kernel Memory Opened     */

static volatile INT32 pltErrStart = 0;  /* Non-zero=handling signal */
static volatile INT32 pltErrSig = 0;    /* Platform Error Signal    */
static volatile UINT32 pltErrData = 0;  /* Platform Error Trap      */
static volatile UINT32 pltErrForce = 0; /* Wait for cores to finish */
static volatile ulong errSignal = 0;    /* Platform err mask        */
static volatile UINT32 sleeper = 0;     /* Magic Sleeper            */
static FILE *pamLogFile = NULL;         /* Handle for Log File      */
static UINT32 wdEnabled = 0;            /* Watchdog enabled or not  */
static UINT32 suicideDisabled = 0;      /* Suicide Disabled         */
static UINT32 pamHeartbeatDisabled = 0; /* CCB-to-PAM heartbeat  */

static int  gNgSdFd = -1;               /* watchdog timer file descriptor */

#ifdef PAM_PRIORITY
static struct sched_param rt_schedparam;
#endif
static INT32 start_index = 1;           /* index = 0 if iscsid to be started */

static UINT32 reduce_output = 0;        /* Reduce repeated output from signals */

/*
******************************************************************************
** Routines not externed in header files.
******************************************************************************
*/
extern void SETUP_linux(void);
extern INT32 resetQlogic(INT32);

/****************************************************************************/
/*
** These need to be here for use of the kernel memory mapping.
*/
UINT32      SHMEM_START = 0;
UINT32      SHMEM_END = 0;
UINT32      startOfBESharedMem;
UINT32      endOfBESharedMem;
UINT8      *PSTARTOFHEAP;
UINT32      startOfMySharedMem;
UINT32      endOfMySharedMem;
UINT8      *pStartOfHeap;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static void PAM_CheckTasks(void);
NORETURN static void PAM_SetAndHandleError(INT32, UINT8);
NORETURN static void PAM_HandleError(void);
NORETURN static void PAM_PlatformExec(PAM_PROCESS_DATA *);
static INT32 PAM_ForkExecFunction(PAM_PROCESS_DATA *);
static INT32 PAM_RestartTask(PAM_PROCESS_DATA *);
static INT32 PAM_StartTask(PAM_PROCESS_DATA *);
static void PAM_KillTask(PAM_PROCESS_DATA *, INT32, UINT8);
NORETURN static void PAM_RestartBvm(void);
static INT32 PAM_StartAll(void);
static INT32 PAM_KillAll(INT32, INT32);
static void PAM_ZeroSharedMemory(void);
static INT32 PAM_ResetQlogics(void);
static INT32 PAM_ResetIscsiEthernet(void);
static void PAM_SigAll(UINT32);
static void PAM_SignalCatcher(INT32, UINT32);
NORETURN static void PAM_ControllerDown(UINT32);
static void PAM_WdEnable(void);
static void PAM_WdDisable(void);
static void PAM_WdReset(void);
void        PAM_Printf(UINT32 levelconst, char *format, ...);
static void PAM_OpenLogFile(void);
NORETURN static void PAM_ExitError(INT32);

#ifdef PAM_DEADLOOP
NORETURN static void PAM_DeadLoop(void);
#endif

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Main start of program
**
**  @param      argc    - number of args to prgram
**  @param      argv    - array of string arguments
**
**  @return     Does not return.
**
******************************************************************************
**/
NORETURN INT32 main(INT32 argc, char *argv[])
{
    INT32       sigIndex = 0;
    struct stat statInfo;

    /* Set the OS output buffering mode to 'line buffer' for stdout and stderr. */
    setvbuf(stdout, (char *)NULL, _IOLBF, 0);
    setvbuf(stderr, (char *)NULL, _IOLBF, 0);

    /* Open the Log File. We will continue to run, regardless file is opened. */
    PAM_OpenLogFile();

    /* Get our pid, and ready to pass to the CCB as input argument. */
    pamProcPid = getpid();
    sprintf(pamProcBuf, "%d", pamProcPid);

    /* Set new session leader and group ID. */
    if (setpgrp() == -1)
    {
        PAM_Printf(DPRINTF_PAM, "ERROR Setting PAM Group ID to %d", pamProcPid);
        perror("Session");
    }

    /* Get the system configuration flags from /opt/xiotech/ccbdata/ccb.cfg. */
    suicideDisabled = SuicideDisableSwitch();
    pamHeartbeatDisabled = PAMHeartbeatDisableSwitch();

    /* Set up signal catcher. TaskCreate changes these in minor ways. */
    for (sigIndex = 1; sigIndex < L_SIGNAL_MAX_SIGNALS; ++sigIndex)
    {
        switch (sigIndex)
        {
                /* Can not catch SIGKILL or SIGSTOP. */
            case SIGKILL:
            case SIGSTOP:
                continue;

                /* Linux threads uses first 3 real-time signals, do not catch. */
            case __SIGRTMIN:
            case (__SIGRTMIN + 1):
            case (__SIGRTMIN + 2):
                continue;

                /* These standard signals should not be caught, since they are
                 * used for standard unix stuff and communications. */
            case SIGCHLD:      /* Child status has changed */
            case SIGCONT:      /* Continue */
            case SIGTSTP:      /* Keyboard stop */
            case SIGTTIN:      /* Background read from tty */
            case SIGTTOU:      /* Background write to tty */
            case SIGURG:       /* Urgent condition on socket */
            case SIGWINCH:     /* Window size change */
                continue;

                /* Trap the following signals. */
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
            case SIGPIPE:      /* Broken pipe */
            case SIGALRM:      /* Alarm clock */
            case SIGTERM:      /* Termination */
            case SIGSTKFLT:    /* Stack fault */
            case SIGXCPU:      /* CPU time limit exceeded */
            case SIGXFSZ:      /* File size limit exceeded */
            case SIGVTALRM:    /* Virtual alarm clock */
            case SIGPROF:      /* Profiling alarm clock */
            case SIGIO:        /* I/O now possible */
            case SIGPWR:       /* Power failure restart */
            case SIGSYS:       /* Bad system call */
            default:           /* All remaining real-time interrupts */
                break;
        }
        /* Add the signal to our catching routine. */
        L_SignalHandlerAdd(sigIndex, (XKSIGHNDLR) PAM_SignalCatcher, true);
    }

    /* Enable the watchdog with a startup timeout value, that is long enough
     * to allow the platform to come up.
     *
     * If the system hangs when the applications are starting (i.e. link layer
     * lockup) then this will allow the watchdog to reset the system. */
//    PAM_Printf(DPRINTF_PAM, "Setting watchdog - startup");
    PAM_WdEnable();

    /* Check if we need to reboot before starting. */
    if (stat(REBOOT_NEEDED_FILE, &statInfo) == 0)
    {
        PAM_Printf(DPRINTF_PAM, "Reboot signaled from BVM");
        unlink(REBOOT_NEEDED_FILE);
        /* Does not return. */
        PAM_SetAndHandleError(ERR_EXIT_REBOOT, 1);      /* force == true */
    }
    else
    {
        PAM_Printf(DPRINTF_PAM, "NO reboot required at this time (%d)", errno);
    }

#ifdef MODEL_750
    /* Restore adaptec if 750. */
    if (PAM_ResetAdaptec9410(1) == 0)
    {
        PAM_Printf(DPRINTF_PAM, "Waiting 3 seconds for adapter to reinit");
        sleeper = 3;
        while ((sleeper = sleep(sleeper))) ;
    }
#endif

    /* Start all the processes. */
    if (PAM_StartAll() != 0)
    {
        /*
         * If failed to start, then go to older firmware version and restart.
         */
        PAM_SigAll(ERR_EXIT_FIRMWARE);
        /* Above handled in PAM_CheckTasks(). */
    }

#ifdef ENABLE_NG_LED
    lc_init();
#endif

    /* Sleep for 5 seconds, to allow things to come up (before checking CCB heartbeat). */
    PAM_Printf(DPRINTF_PAM, "Waiting for applications to start");

    sleeper = 5;
    while (1)
    {
        /* Note: a signal will break through a sleep. */
        sleeper = sleep(sleeper);

        PAM_WdReset();          /* Kick the watchdog. */

        /* If a signal has come in, handle it. */
        if (pltErrSig != 0)
        {
            /* Does not return. */
            PAM_HandleError();
        }

        /* If we have not slept long enough, continue. */
        if (sleeper == 0)
        {
            break;
        }
    }

    /* Enable the watchdog with the default timeout. This overrides the
     * previous timeout setting. If we are running with the suicides disabled,
     * then just make a log message is the watchdog trips. If suicides are
     * disabled, then force the action to only make a log entry. */
    if (suicideDisabled)
    {
        PAM_Printf(DPRINTF_PAM, "Suicide disabled - watchdog will take no action");
        PAM_WdDisable();
    }
    else
    {
//        PAM_Printf(DPRINTF_PAM, "Setting watchdog - default");
        PAM_WdEnable();
    }

    /* Monitor Tasks. */
    while (1)
    {
        sleeper = MON_SLP_TM;
        while (1)
        {
            /* The most a signal will have to wait for processing is 1 seconds.
             * Specifically if it comes in before the last check and the sleep. */

            /* Note: a signal will break through a sleep. */
            sleeper = sleep(sleeper);

            PAM_WdReset();      /* Kick the watchdog. */

            /* If a signal has come in, handle it. */
            if (pltErrSig != 0)
            {
                /* Does not return. */
                PAM_HandleError();
            }

            /* If we have not slept long enough, continue. */
            if (sleeper == 0)
            {
                break;
            }
        }

        /* Reset the watchdog timer to start over with countdown. */
        PAM_WdReset();

        /* Check the Tasks. This also checks for a signal. */
        PAM_CheckTasks();

        /* If ccb.cfg has not disabled heartbeats, make sure happening. */
        if (!pamHeartbeatDisabled)
        {
            /* Check CCB heartbeat. */
            if (tasks[TASK_CCB].stat != 0)
            {
                if (ccbHB != ccbLastHB)
                {
                    ccbLastHB = ccbHB;
                    ccbTmMsd = 0;
                }
                else
                {
                    ccbTmMsd += MON_SLP_TM;

                    if (ccbTmMsd > CCB_HB_TMT)
                    {
                        clearCcbHbTimers();
                        PAM_Printf(DPRINTF_PAM, "CCB HB Failed, Going Down");

                        /* Send an initial signal to the CCB. */
                        PAM_SigAll(ERR_PAM_DIRECTED_DOWN);
                        /* Above handled in PAM_CheckTasks(). */
                    }
                }
            }
            else
            {
                clearCcbHbTimers();
            }
        }
    }
}   /* End of main */

/**
******************************************************************************
**
**  @brief      Routine to check for Status of running processes.
**
**              This routine checks for second stage of signals that need to
**              be handled, and processes started by PAM are still running.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void PAM_CheckTasks(void)
{
    INT32       index = 0;

    /* Check for platform error trap, requiring delayed processing. */
    if (pltErrSig != 0)
    {
        /* Does not return. */
        PAM_HandleError();
    }

    /* Check the Tasks. */
    for (index = start_index; index < TASK_MAX; ++index)
    {
        /* If a task was started, and we get a termination signal from it,
         * log it, and proceed to handle it. */
        if (tasks[index].stat == 1 &&
            waitpid(tasks[index].pid, &tasks[index].status, WNOHANG) > 0)
        {
            if (WIFSIGNALED(tasks[index].status))
            {
                PAM_Printf(DPRINTF_PAM, "%s terminated with signal %d",
                           tasks[index].execArgs[0], WTERMSIG(tasks[index].status));
            }
            else
            {
                PAM_Printf(DPRINTF_PAM, "%s exited with status %d",
                           tasks[index].execArgs[0], WEXITSTATUS(tasks[index].status));
            }

            /* Make sure there is not already an error trap in progress. */
            if (pltErrData == 0)
            {
                /* If this is the syscall server restart it. */
                if (index == TASK_SV)
                {
                    PAM_RestartTask(&tasks[index]);
                }
                /* If this is iscsid task then controller should reboot as
                 * no communication between controllers is possible. */
                if (index == TASK_ISCSI)
                {
                    PAM_Printf(DPRINTF_PAM, "iscsid has exited CONTROLLER REBOOTING\n");
                    PAM_SigAll(ERR_PAM_DIRECTED_DOWN);
                }
                /* Else, ignore. Let the CCB handle it. If the CCB cannot, the
                 * HB will timeout and we will take action there. */
            }
        }
    }

    /* Check for platform error trap (that occurred during above processing). */
    if (pltErrSig != 0)
    {
        /* Does not return. */
        PAM_HandleError();
    }
}   /* End of PAM_CheckTasks */

/**
******************************************************************************
**
**  @brief      Run script to get compressed shared memory files.
**
******************************************************************************
**/
static void run_gzshm(void)
{
    pid_t       pid;
    pid_t       rc;
    int         status;

    PAM_WdReset();      /* Kick the watchdog */

    /* Fork off a process to dump the shared memory sections. */
    pid = fork();

    switch (pid)
    {
        case -1:
            PAM_Printf(DPRINTF_PAM, "Forking gzshm FAILED (%s)", strerror(errno));
            return;

        case 0:
            execl("/bin/sh", "-c", "./gzshm", (char *)NULL);
            PAM_Printf(DPRINTF_PAM, "Executing gzshm FAILED (%s)", strerror(errno));
            return;

        default:
            /* Parent normal case. */
            break;
    }

    /* Give the process time to finish. */
    int         print_count_limit = 0;
    UINT32      tmTkn = 0;

    for (;;)
    {
        /* Sleep for 1/10th of a second */
        usleep(1000 * 100);
        PAM_WdReset();      /* Kick the watchdog */

        rc = waitpid(pid, &status, WNOHANG);
        if (rc > 0 || (rc < 0 && errno == ECHILD))
        {
            PAM_WdReset();      /* Kick the watchdog */
            PAM_Printf(DPRINTF_PAM, "gzshm finished");
            break;
        }
        if ((print_count_limit++ % 10) == 0)
        {
            PAM_Printf(DPRINTF_PAM, "Task gzshm still running");
            tmTkn += 1;         /* Ever second increment. */
        }

        /* If gzshm takes longer than 4.25 minutes, exit. */
        if (tmTkn >= WD_ERRTRAP_TIMEOUT)
        {
            PAM_Printf(DPRINTF_PAM, "gshm taking too long to run (%u)", tmTkn);
            return;
        }
    }
}   /* End of run_gzshm */

/**
******************************************************************************
**
**  @brief      PAM Error Handler
**
**  @param      none.
**
**  @return     none
**
**  @attention  Does not return.
**  @attention  Uses pltErrSig, pltErrData. and pltErrForce from PAM_SignalCatcher.
**
******************************************************************************
**/

NORETURN static void PAM_HandleError(void)
{
    UINT32      tskCnt = 0;
    UINT32      tskIndx = 0;
    UINT32      tmTkn = 0;
    INT32       rc = 0;

    /* As soon as this is set, we can no longer over-ride a signal with a
     * more high priority signal. */
    pltErrStart = 1;

    switch (pltErrSig)
    {
            /* kill -s 2 pamPid (happened) */
        case SIGINT:
            PAM_ResetQlogics();
            PAM_Printf(DPRINTF_PAM, "Caught SIGINT - exiting");

            /* Does not return. */
            PAM_ExitError(0xf0 | pltErrSig);
            break;

        case SIGABRT:
            /* Should not get here, handled in catcher. */
            signal(SIGABRT, SIG_DFL);
            abort();

        case SIGTERM:
            /* System is going down, do a PAM_KillAll to be sure the qlogics
             * are reset, and try and disable the WD if we can. */
            PAM_KillAll(SIGKILL, 0);
            PAM_WdDisable();
            exit(1);

        case SIGILL:
        case SIGFPE:
        case SIGSEGV:
        case SIGPIPE:
        case SIGBUS:
            /* We got a second signal and it was one of these.
             * Reset the watchdog timer. */
            PAM_WdReset();

            /* Reboot, does not return. */
            PAM_ControllerDown(CN_DOWN_REBOOT);
            break;

        case XIO_PLATFORM_SIGNAL:
            switch (pltErrData)
            {
                    /* We need to go down, but not wait for core dumps. */
                case ERR_EXIT_SHUTDOWN:         // 0x10
                case ERR_EXIT_REBOOT:           // 0x16
                    break;

                    /* If we are to go down for some reason. */
//                case ERR_EXIT_BE_MISS_HB:     UNUSED  0x11
//                case ERR_EXIT_FE_MISS_HB:     UNUSED  0x12
                case ERR_EXIT_RESET_CCB:        // 0x13
                case ERR_EXIT_RESET_ALL:        // 0x14
                case ERR_EXIT_DEADLOOP:         // 0x15
                case ERR_EXIT_FIRMWARE:         // 0x17
                case ERR_EXIT_BVM_RESTART:      // 0x18
                case ERR_EXIT_BIOS_REBOOT:      // 0x19
                    /* Reset the qlogic cards. */
                    if (PAM_ResetQlogics() != GOOD)
                    {
                        PAM_Printf(DPRINTF_PAM, "%s(%s) - Could not reset qlogics, exiting.",
                                   __func__, L_LinuxSignalToString(pltErrSig, pltErrData));
                        if (suicideDisabled)
                        {
                            /* Wait for cores in debug mode. */
                        }
                        else
                        {
                            /* Set to force down, do not wait for core dumps. */
                            pltErrForce = 1;
                        }
                    }

                    PAM_Printf(DPRINTF_PAM, "Caught (%s)", L_LinuxSignalToString(pltErrSig, pltErrData));

                    clearCcbHbTimers(); /* Clear the CCB HB Timers */

                    /* Reset the watchdog timer. */
                    PAM_WdReset();
                    break;

                    /* If CCB sending heartbeat notification to PAM - cannot be here. */
/*         case CCB_2_PAM_HB: */
                case ERR_PAM_DIRECTED_DOWN:             // 0x20
                default:
                    PAM_Printf(DPRINTF_PAM, "%s (%s) data %u",
                               pltErrData == ERR_PAM_DIRECTED_DOWN ? "PAM" : "Unhandled",
                               L_LinuxSignalToString(pltErrSig, pltErrData), pltErrData);
                    PAM_SigAll(ERR_PAM_DIRECTED_DOWN);

                    /* Force variable change, even if already set. */
                    pltErrSig = XIO_PLATFORM_SIGNAL;
                    pltErrData = ERR_PAM_DIRECTED_DOWN;
                    break;
            }
            break;

        default:               /* Should not get here, handled in catcher. */
            /* We need to be harsh and ignore all other signals,
             * and let the laws of nature proceed. In system event
             * cases, the platform will receive the same signals,
             * and we need to be here to handle their reaction. */
            PAM_Printf(DPRINTF_PAM, "Unhandled Signal (%s) - Ignoring",
                       L_LinuxSignalToString(pltErrSig, 0));
            break;
    }

    PAM_Printf(DPRINTF_PAM, "%s - (%s) force = %s wait for cores", __func__,
               L_LinuxSignalToString(XIO_PLATFORM_SIGNAL, pltErrData),
               (pltErrForce) ? "YES, do not" : "NO, ");

    /* If we do not need to force down, wait for the platform apps to core. */
    if (!pltErrForce)
    {
        /* Enable the Watchdog */
        PAM_WdEnable();

        PAM_Printf(DPRINTF_PAM, "%s(%s) - Waiting for tasks to core",
                   __func__, L_LinuxSignalToString(XIO_PLATFORM_SIGNAL, pltErrData));
#ifdef EXTRA_PAM_DEBUGGING
// Truncate ps.pam and put out the first ps.
        system("(date;/bin/ps augxww) >/var/log/xiotech/ps.pam");
#endif /* EXTRA_PAM_DEBUGGING */

        /* Give the processes time to finish dumping core. */
        int         print_count_limit = 0;

        while (tskCnt < TASK_MAX)
        {
            /* Sleep for 1/10th of a second */
            usleep(1000 * 100);
            PAM_WdReset();      /* Kick the watchdog */

            /* Init tskCnt, if no iscsi, count it as already down. */
            if (start_index == 0)
            {
                tskCnt = 0;
            }
            else
            {
                tskCnt = 1;
            }

            /* Check all task statuses (possibly with/without iscsi). */
            for (tskIndx = start_index; tskIndx < TASK_MAX; ++tskIndx)
            {
                if (tasks[tskIndx].pid == 0)
                {
                    ++tskCnt;
                    continue;
                }

                rc = waitpid(tasks[tskIndx].pid, &tasks[tskIndx].status, WNOHANG);
                if (rc > 0 || (rc < 0 && errno == ECHILD))
                {
                    PAM_WdReset();      /* Kick the watchdog */
                    tmTkn = 0;
                    PAM_Printf(DPRINTF_PAM, "%u - Task finished coring (%s)", tskIndx,
                               tasks[tskIndx].execArgs[0]);
                    tasks[tskIndx].pid = 0;
                    print_count_limit = 0;
                }
                else
                {
                    if ((print_count_limit++ % 10) == 0)
                    {
                        tmTkn++;        /* Increment every second. */
                        PAM_Printf(DPRINTF_PAM, "%u - Task still coring (%s)", tskIndx,
                                   tasks[tskIndx].execArgs[0]);
                    }
                }
#ifdef EXTRA_PAM_DEBUGGING
                system("(date;/bin/ps augxww) >>/var/log/xiotech/ps.pam");
#endif /* EXTRA_PAM_DEBUGGING */
            }

            /* If last core dump takes longer than 4.25 minutes, exit the loop, because
             * the core dumping is taking too long. */
            if (tmTkn >= WD_ERRTRAP_TIMEOUT)
            {
                PAM_Printf(DPRINTF_PAM, "Task taking too long to core (%u)", tmTkn);
                break;
            }
        }

#ifdef EXTRA_PAM_DEBUGGING
        system("(echo done; date;/bin/ps augxww) >>/var/log/xiotech/ps.pam");
#endif /* EXTRA_PAM_DEBUGGING */
        PAM_WdReset();          /* Kick the watchdog */

        system("/bin/sync");

        if (tskCnt >= TASK_MAX)
        {
            PAM_Printf(DPRINTF_PAM, "%s(%s) - All %lu Tasks Exited", __func__,
                       L_LinuxSignalToString(XIO_PLATFORM_SIGNAL, pltErrData), tskCnt);
        }
        else
        {
            PAM_Printf(DPRINTF_PAM, "%s(%d) - Only %lu of %lu Tasks Exited", __func__,
                       L_LinuxSignalToString(XIO_PLATFORM_SIGNAL, pltErrData),
                       tskCnt, TASK_MAX);
        }
    }

    PAM_WdReset();              /* Kick the watchdog */

    if (!pltErrForce)
    {
        /* Capture the shared memory files. */
        PAM_Printf(DPRINTF_PAM, "%s(%s) - Capturing shared memory", __func__,
                   L_LinuxSignalToString(XIO_PLATFORM_SIGNAL, pltErrData));
        run_gzshm();
    }

    PAM_Printf(DPRINTF_PAM, "%s(%s) - syncing file systems first time", __func__,
               L_LinuxSignalToString(XIO_PLATFORM_SIGNAL, pltErrData));
    system("/bin/sync");
    system("/bin/sync");
    PAM_WdReset();              /* Kick the watchdog */
    usleep(1000000 / 16);       /* sleep for 1/16th of a second */
    PAM_WdReset();              /* Kick the watchdog */
    PAM_Printf(DPRINTF_PAM, "%s(%s) - syncing file systems second time", __func__,
               L_LinuxSignalToString(XIO_PLATFORM_SIGNAL, pltErrData));
    system("/bin/sync");
    system("/bin/sync");

#ifdef EXTRA_PAM_DEBUGGING
    system("(date;/bin/ps augxww) >>/var/log/xiotech/ps.pam");
#endif /* EXTRA_PAM_DEBUGGING */
    PAM_WdReset();              /* Kick the watchdog */

    /* Determine the problem. */
    switch (pltErrData)
    {
        case ERR_EXIT_SHUTDOWN:        /* Does not return. */
#ifdef EXTRA_PAM_DEBUGGING
            system("(date;echo ERR_EXIT_SHUTDOWN) >>/var/log/xiotech/ps.pam");
#endif /* EXTRA_PAM_DEBUGGING */
            PAM_ControllerDown(CN_DOWN_SHUTDOWN);
            break;

        case ERR_EXIT_DEADLOOP:        /* Does not return. */
#ifdef EXTRA_PAM_DEBUGGING
            system("(date;echo ERR_EXIT_DEADLOOP) >>/var/log/xiotech/ps.pam");
#endif /* EXTRA_PAM_DEBUGGING */
            PAM_ControllerDown(CN_DOWN_DEADLOOP);
            break;

//         case ERR_EXIT_BE_MISS_HB:      /* Does not return. */    UNUSED
//         case ERR_EXIT_FE_MISS_HB:      /* Does not return. */    UNUSED
        case ERR_EXIT_REBOOT:          /* Does not return. */
        case ERR_EXIT_RESET_CCB:       /* Does not return. */
        case ERR_EXIT_RESET_ALL:       /* Does not return. */
        case ERR_PAM_DIRECTED_DOWN:    /* Does not return. */
#ifdef EXTRA_PAM_DEBUGGING
            system("(date;echo CN_DOWN_REBOOT) >>/var/log/xiotech/ps.pam");
#endif /* EXTRA_PAM_DEBUGGING */
            PAM_ControllerDown(CN_DOWN_REBOOT);
            break;

        case ERR_EXIT_FIRMWARE:
            /* If we get the preceeding error code, we could not exec
             * the program, we need to downlevel the FWV and try again, if we
             * can not get the thing to go, take the controller down. */
            PAM_Printf(DPRINTF_PAM, "%s(%s) - Could not exec fw, TODO downlevel fwv",
                       __func__, L_LinuxSignalToString(XIO_PLATFORM_SIGNAL, pltErrData));
            /* Kill all the tasks and wait for five seconds, then start over. */
            PAM_KillAll(SIGKILL, 1);
            sleep(5);
            /* Note: PAM_RestartBvm kicks the watchdog. */
            /* Does not return. */
#ifdef EXTRA_PAM_DEBUGGING
            system("(date;echo ERR_EXIT_FIRMWARE) >>/var/log/xiotech/ps.pam");
#endif /* EXTRA_PAM_DEBUGGING */
            PAM_RestartBvm();
            break;

            /* We want to go through the BIOS to reset everything -- specifically qlogic card. */
        case ERR_EXIT_BIOS_REBOOT:
            PAM_ControllerDown(CN_DOWN_REBOOT);
            break;

        case ERR_EXIT_BVM_RESTART:
        default:               /* Does not return. */
#ifdef EXTRA_PAM_DEBUGGING
            system("(date;echo default) >>/var/log/xiotech/ps.pam");
#endif /* EXTRA_PAM_DEBUGGING */
// NOTDONEYET -- auto-activate   PAM_ControllerDown(CN_DOWN_BVM);
            PAM_ControllerDown(CN_DOWN_DEADLOOP);
            break;
    }
}   /* End of PAM_HandleError */

/**
******************************************************************************
**
**  @brief      Execute A Platform Task
**
**  @param      pData - pointer to the structure containing the arguments for
**                      the exec system call.
**
**  @return     Does not return.
**
******************************************************************************
**/
NORETURN static void PAM_PlatformExec(PAM_PROCESS_DATA *pData)
{
    /* Execute the CCB. */
    if (execv(pData->execArgs[0], pData->execArgs) == -1)
    {
        PAM_Printf(DPRINTF_PAM, "Execing (%s) FAILED (%s)",
                   pData->execArgs[0], strerror(errno));

        /* Send the signal. */
        PAM_SigAll(ERR_EXIT_FIRMWARE);
    }

    exit(pData->errCd);
}   /* End of PAM_PlatformExec */

/**
******************************************************************************
**
**  @brief      Fork off a task for execution.
**
**  @param      pData - pointer to the structure containing the arguments for
**                      the exec system call.
**
**  @return     The parent process (after fork) returns new child pid.
**
******************************************************************************
**/
static INT32 PAM_ForkExecFunction(PAM_PROCESS_DATA *pData)
{
    pid_t       pid;

    /* Fork off a process to start the platform application. */
    pid = fork();

    switch (pid)
    {
        case -1:
            PAM_Printf(DPRINTF_PAM, "Forking (%s) FAILED (%s)", pData->execArgs[0], strerror(errno));
            break;

        case 0:
            /* Set Group ID. */
            if (setpgid(0, pamProcPid) != 0)
            {
                PAM_Printf(DPRINTF_PAM, "ERROR Setting %s Group ID to %d", pData->execArgs[0], pamProcPid);
            }

            /* See if we have an open handle to the wd. */
            if (gNgSdFd != -1)
            {
                close(gNgSdFd);
            }

            PAM_PlatformExec(pData);
            break;

        default:
            /* Parent normal case. */
            break;
    }
    return (pid);
}   /* End of PAM_ForkExecFunction */

/**
******************************************************************************
**
**  @brief      Routine to clear shared memory
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void PAM_ZeroSharedMemory(void)
{
    /* Map the huge shared memory sections. */
    if (!kMemOpen)
    {
        SETUP_linux();
        kMemOpen = 1;
    }

    /* Clear shared memory. */
    memset((void *)FE_BASE_ADDR, 0, SIZE_FE_LTH);
    memset((void *)BE_BASE_ADDR, 0, SIZE_BE_LTH);
    memset((void *)CCB_BASE_ADDR, 0, SIZE_CCB_LTH);

    PAM_Printf(DPRINTF_PAM, "Clearing Shared Memory - Complete");

    /* Do not want this memory any more. */
    munmap((INT32 *)FE_BASE_ADDR, SIZE_FE_LTH);
    munmap((INT32 *)BE_BASE_ADDR, SIZE_BE_LTH);
    munmap((INT32 *)CCB_BASE_ADDR, SIZE_CCB_LTH);
}   /* End of PAM_ZeroSharedMemory */

/**
******************************************************************************
**
**  @brief      Restart a single process (syssrv)
**
**  @param      pData - pointer to task data
**
**  @return     zero if successful, non-zero if error
**
******************************************************************************
**/
static INT32 PAM_RestartTask(PAM_PROCESS_DATA *pData)
{
    PAM_KillTask(pData, SIGKILL, 1);
    return (PAM_StartTask(pData));
}   /* End of PAM_RestartTask */

/**
******************************************************************************
**
**  @brief      Start a single Task
**
**  @param      pData - pointer to task data
**
**  @return     zero if successful, non-zero if error
**
******************************************************************************
**/
static INT32 PAM_StartTask(PAM_PROCESS_DATA *pData)
{
    INT32       rc = 0;

    /* Try to start the task. */
    if (pData->stat == 0 && (pData->pid = PAM_ForkExecFunction(pData)) <= 0)
    {
        /* Failed to start the task. */
        pData->stat = 0;
        rc = pData->errCd;
    }
    else
    {
        /* Set the status appropriately. */
        pData->stat = 1;
    }

    if (rc != 0)
    {
        PAM_Printf(DPRINTF_PAM, "Platform Application %s start FAILED", pData->execArgs[0]);
    }
    return (rc);
}   /* End of PAM_StartTask */

/**
******************************************************************************
**
**  @brief      Kill a single task
**
**  @param      pData   - pointer to task data
**              signal  - signal to send the process
**              wait    - true, if to wait for process to terminate.
**
**  @return     none
**
******************************************************************************
**/
static void PAM_KillTask(PAM_PROCESS_DATA *pData, INT32 killSignal, UINT8 wait)
{
    if (pData->stat == 1)
    {
        if (pData->pid != 0)
        {
            kill(pData->pid, killSignal);
        }
        pData->stat = 0;
    }

    if (wait && (pData->pid != 0))
    {
        waitpid(pData->pid, &pData->status, WUNTRACED);
    }
}   /* End of PAM_KillTask */

/**
******************************************************************************
**
**  @brief      Restart the BVM
**
**              Kill all of the Platform Apps, and exec the BVM.
**
**  @param      none
**
**  @return     none
**
**  @attention  Does not return.
**
******************************************************************************
**/
NORETURN static void PAM_RestartBvm(void)
{
    PAM_Printf(DPRINTF_PAM, "Killing Platform");

    /* Kill all of the platform applications. */
    PAM_KillAll(SIGKILL, 1);

    PAM_Printf(DPRINTF_PAM, "Restarting BVM");

    /* Enable the Watchdog for reset. */
    PAM_WdEnable();

    /* Re - exec the BVM, does not return. */
    PAM_PlatformExec(&bvmTask);
}   /* End of PAM_RestartBvm */

/**
******************************************************************************
**
**  @brief      Start all the processes
**
**  @param      none
**
**  @return     zero if successful, non-zero if error
**
******************************************************************************
**/
static INT32 PAM_StartAll(void)
{
    INT32       index = 0;
    char       *value = NULL;

    /* Clear the shared memory. */
    PAM_ZeroSharedMemory();

#ifdef PAM_PRIORITY
    /* Get our initial priority. */
    rt_schedparam.__sched_priority = XIO_LL_LINUX_PRIORITY;

    /* Lower our priority so the forked tasks start out running at (real-time) priority. */
    PAM_Printf(DPRINTF_PAM, "Setting PAM priority to %d within scheduling policy SCHED_RR",
               rt_schedparam.__sched_priority);
    if (sched_setscheduler(0, SCHED_RR, &rt_schedparam) != 0)
    {
        PAM_Printf(DPRINTF_PAM, "FAILED to set PAM priority");
        return (1);
    }
#endif
    /* Find out if iscsi daemon need to be started.
     * If there is any iscsi card the daemon need to be started and
     * start_index = 0; otherwise start_index = 1; */
    value = getenv("FEDEVS");

    if (value == 0 || *value == 0)
    {
        fprintf(stderr, "setdevices: Bus environment not set FEDEVS\n");
        return (1);
    }

    if (strstr(value, "iscsi") != NULL || strstr(value, "icl") != NULL)
    {
        start_index = 0;
    }

    /* Fork the platform applications. */
    for (index = start_index; index < TASK_MAX; ++index)
    {
        PAM_StartTask(&tasks[index]);
    }

    /* Clear the CCB HB Timers. */
    clearCcbHbTimers();

    PAM_Printf(DPRINTF_PAM, "Platform Applications Started");

#ifdef PAM_PRIORITY
    /* Get our max priority. */
    rt_schedparam.__sched_priority = XIO_LINUX_PRIORITY;

    /* Set PAM to run with real-time scheduling with high (real-time) priority. */
    PAM_Printf(DPRINTF_PAM, "Setting PAM priority to %d within scheduling policy SCHED_RR",
               rt_schedparam.__sched_priority);
    if (sched_setscheduler(0, SCHED_RR, &rt_schedparam) != 0)
    {
        PAM_Printf(DPRINTF_PAM, "FAILED to set PAM priority");
        return (1);
    }
#endif

    return (0);
}   /* End of PAM_StartAll */

/**
******************************************************************************
**
**  @brief      Kill all the application processes
**
**  @param      signal  - signal to send the processes
**              wait    - true, if want to wait for processes to end
**
**  @return     none
**
******************************************************************************
**/
static INT32 PAM_KillAll(INT32 signal, INT32 wait)
{
    INT32       index = 0;

    /* Kill All the processes. */
    for (index = start_index; index < TASK_MAX; ++index)
    {
        if (index == TASK_ISCSI)
        {
            PAM_KillTask(&tasks[index], SIGINT, wait);
        }
        else
        {
            PAM_KillTask(&tasks[index], signal, wait);
        }
    }

    PAM_Printf(DPRINTF_PAM, "Platform Applications Killed");

    /* Reset the Qlogic Cards. */
    PAM_ResetQlogics();

    usleep(1000000 / 4);        /* sleep for 1/4 second */

    /* Reset the Qlogic Cards again, perhaps PAM_KillTask() caught FE or BE
     * in the middle of doing something with the QL card, and the FE/BE will
     * NOT stop processing [to do the signal] until a time-slice, or system
     * call is done.  I.e. allow 1/4 second for race condition. */

    return (PAM_ResetQlogics());
}   /* End of PAM_KillAll */

/**
******************************************************************************
**
**  @brief      Reset the FE/BE Qlogic cards
**
**  @param      none
**
**  @return     zero if successful, non-zero if error occurred.
**
******************************************************************************
**/
static INT32 PAM_ResetQlogics(void)
{
    unsigned long bitmap = 0;
    long        index = 0;
    pcidevtbl   devtbl[PCIMAXDEV];
    INT32       rc;

    /* Cleanup the iscsi Ethernet interfaces. */
    rc = PAM_ResetIscsiEthernet();

    /* Search for Qlogic cards. */
    bitmap = LI_ScanBus(devtbl, PCIMAXDEV);

    /* If bitmap is 0, we did not find any devices.
     * This is an error as far as we are concerned. */
    if (!bitmap)
    {
        return (1);
    }

    for (index = 0; index < XIO3D_NIO_MAX; ++index)
    {
        if ((bitmap & (1 << (PCIBASE + index))) &&
            (devtbl[index + PCIOFFSET].vendev & 0x0000FFFF) == PCI_VENDOR_ID_QLOGIC)
        {
            if (resetQlogic(index) != GOOD)
            {
                PAM_Printf(DPRINTF_PAM, "Unable to reset Qlogic card (%l) 0x%08X",
                           index, devtbl[index + PCIOFFSET].vendev);
                rc |= 1;
            }
        }
    }

    if (rc == 0)
    {
        PAM_Printf(DPRINTF_PAM, "QLogic cards reset");
    }
    else
    {
        PAM_Printf(DPRINTF_PAM, "Error occurred resetting FE ports (ethernet/Qlogic)");
    }
    return (rc);
}   /* End of PAM_ResetQlogics */

/**
******************************************************************************
**
**  @brief      Reset the iscsi Interfaces
**
**  @param      none
**
**  @return     zero if successful, non-zero if error occurred.
**
******************************************************************************
**/
static INT32 PAM_ResetIscsiEthernet(void)
{
    struct ifreq ifr;
    UINT32      flags = IFF_NOARP;
    UINT32      mask = IFF_NOARP | IFF_UP;
    struct pci_devs *dev = NULL;
    INT32       fd = -1;
    long        index = 0;
    INT32       rc = 0;
    pcidevtbl   devtbl[PCIMAXDEV];

    /* Need to run this to identify the iscsi interfaces. */
    LI_ScanBus(devtbl, PCIMAXDEV);

    /* Bring down all iscsi interfaces. */
    for (index = 0; index < XIO3D_NIO_MAX; ++index)
    {
        dev = LI_GetPCIdev(index);
        if (dev && (dev->busdevfn == ENET_BUSDEVFN))
        {
            strcpy(ifr.ifr_name, dev->enetname);
            fd = socket(AF_INET, SOCK_DGRAM, 0);

            if ((ioctl(fd, SIOCGIFFLAGS, &ifr)) == 0)
            {
                if ((ifr.ifr_flags ^ flags) & mask)
                {
                    ifr.ifr_flags &= ~mask;
                    ifr.ifr_flags |= mask & flags;

                    if (ioctl(fd, SIOCSIFFLAGS, &ifr) != 0)
                    {
                        PAM_Printf(DPRINTF_PAM, "Shutdown of ethernet dev %s: failed\n", dev->enetname);
                        rc = 1;
                    }
                    else
                    {
                        PAM_Printf(DPRINTF_PAM, "Shutdown of ethernet dev %s: success\n", dev->enetname);
                    }
                }
                else
                {
                    PAM_Printf(DPRINTF_PAM, "Shutdown of ethernet dev %s: already down\n", dev->enetname);
                }
            }
            else
            {
                PAM_Printf(DPRINTF_PAM, "Get ethernet dev %s flags failed\n", dev->enetname);
                rc = 1;
            }
            close(fd);
        }
    }

#ifdef MODEL_750
    /* Shutdown the icl0 interface. */
    strcpy(ifr.ifr_name, "icl0");
    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if ((ioctl(fd, SIOCGIFFLAGS, &ifr)) == 0)
    {
        if ((ifr.ifr_flags ^ flags) & mask)
        {
            ifr.ifr_flags &= ~mask;
            ifr.ifr_flags |= mask & flags;

            if (ioctl(fd, SIOCSIFFLAGS, &ifr) != 0)
            {
                PAM_Printf(DPRINTF_PAM, "Shutdown of ethernet dev %s: failed\n", ifr.ifr_name);
                rc = 1;
            }
            else
            {
                PAM_Printf(DPRINTF_PAM, "Shutdown of ethernet dev %s: success\n", ifr.ifr_name);
            }
        }
        else
        {
            PAM_Printf(DPRINTF_PAM, "Shutdown of ethernet dev %s: already down\n", ifr.ifr_name);
        }
    }
    else
    {
        PAM_Printf(DPRINTF_PAM, "Get ethernet dev %s: flags failed\n", ifr.ifr_name);
        rc = 1;
    }
    close(fd);
#endif
    PAM_Printf(DPRINTF_PAM, "Iscsi cards reset");

    return (rc);
}   /* End of PAM_ResetIscsiEthernet */


/**
******************************************************************************
**
**  @brief      Take down the controller.
**
**  @param      downType    - Action to take when taking the controller down.
**
**  @return     none
**
**  @attention  Does not return.
**
******************************************************************************
**/
NORETURN static void PAM_ControllerDown(UINT32 downType)
{
    INT32       qlResetStatus = 0;

    /* Kill off the platform tasks, remember qlogic error for later processing. */
    qlResetStatus = PAM_KillAll(SIGKILL, 0);

    /* Get the state of the Battery backed memory.
     * If it is not in low power mode, only reboot.
     * If it is in low power mode, we need to do a
     * power reset as well to bring it back into
     * high power mode. */

    /* Determine the action to be taken. */
    switch (downType)
    {
            /* Shutdown the controller. */
        case CN_DOWN_SHUTDOWN:
            PAM_Printf(DPRINTF_PAM, "HALTING Controller - Shutdown Poweroff");
            PAM_WdEnable();
            system("/sbin/halt -i -f -p");
            break;

            /* Reboot the controller. */
        case CN_DOWN_REBOOT:
            PAM_Printf(DPRINTF_PAM, "HALTING Controller - Reboot");
            PAM_WdEnable();
            system("/sbin/reboot -i -f");
            break;

        case CN_DOWN_BVM:
            /* Re - exec the BVM, does not return. */
            PAM_Printf(DPRINTF_PAM, "HALTING Controller - Restarting BVM");
            PAM_PlatformExec(&bvmTask);
            break;

        case CN_DOWN_DEADLOOP:
        default:
            /* If suicide is disabled, Go into deadloop, or exit. */
            if (suicideDisabled)
            {
#ifdef PAM_DEADLOOP
                PAM_Printf(DPRINTF_PAM, "Taking down controller(%d) - SUICIDE_DISABLED Deadloop",
                           downType);
                /* Does not return. */
                PAM_DeadLoop();
#else
                PAM_Printf(DPRINTF_PAM, "Taking down controller(%d) - SUICIDE_DISABLED", downType);
                /* Does not return. */
                PAM_ExitError(downType);
#endif
            }

            /* Setup the watchdog. */
#ifdef PAM_DEADLOOP
            /* If we successfully reset the qlogic cards, go into deadloop,
             * else shut the applications down. */

            /* Set the watchdog to shutdown on timeout.
             * We will increment the watchdog to the next increment in deadloop. */
            if (qlResetStatus == GOOD)
            {
                PAM_WdEnable();
                PAM_Printf(DPRINTF_PAM, "HALTING Controller - Shutdown Now Deadloop");
                /* Jump to pam deadloop. Does not return. */
                PAM_DeadLoop();
            }
            else
            {
                /* We were unsuccessful resetting the qlogic cards.
                 * Shut down very quickly.
                 * Set the Watchdog to shutdown. */
                PAM_WdEnable();
                PAM_Printf(DPRINTF_PAM, "HALTING Controller - Shutdown Now PowerOff");
                /* Power down. */
                system("/sbin/halt -i -f -p");
            }
#else
            PAM_WdEnable();
            PAM_Printf(DPRINTF_PAM, "HALTING Controller - Shutdown Now Poweroff");
            system("/sbin/halt -f -i -p");
#endif /* PAM_DEADLOOP */
            break;
    }

    PAM_Printf(DPRINTF_PAM, "%s, exiting program?\n", __func__);

    /* Exit program. */
    exit(0);
}   /* End of PAM_ControllerDown */

/**
******************************************************************************
**
**  @brief      Watchdog Enable
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void PAM_WdEnable(void)
{
    if (gNgSdFd == -1)
    {
        gNgSdFd = open(SYS_WD, O_WRONLY);
    }

    if (gNgSdFd == -1)
    {
        PAM_Printf(DPRINTF_PAM, "Watchdog Enable %s FAILED %s(%d)", SYS_WD, strerror(errno), errno);
        return;
    }

    PAM_Printf(DPRINTF_PAM, "Watchdog Enable %s", SYS_WD);
    wdEnabled = 1;              /* Set the enabled flag */
    PAM_WdReset();              /* Start the watchdog */
}   /* End of PAM_WdEnable */

/**
******************************************************************************
**
**  @brief      Watchdog Disable
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void PAM_WdDisable(void)
{
    /* If the watchdog is enabled, disable it. */
    if (!wdEnabled)
    {
        return;
    }

    if (gNgSdFd != -1 && (write(gNgSdFd, "V", 1) != 1 || close(gNgSdFd) != 0))
    {
        PAM_Printf(DPRINTF_PAM, "Watchdog Disable %s FAILED %s(%d)", SYS_WD, strerror(errno), errno);
    }

    gNgSdFd = -1;

    PAM_Printf(DPRINTF_PAM, "Watchdog Disabled");
    wdEnabled = 0;
}   /* End of PAM_WdDisable */

/**
******************************************************************************
**
**  @brief      Watchdog reset to start countdown again.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void PAM_WdReset(void)
{
    /* If the watchdog is enabled, disable it. */
    if (!wdEnabled)
    {
        return;
    }

    if (gNgSdFd != -1 && ioctl(gNgSdFd, WDIOC_KEEPALIVE, 0) == -1)
    {
        PAM_Printf(DPRINTF_PAM, "Watchdog reset to %s FAILED %s(%d)", SYS_WD, strerror(errno), errno);
    }
}   /* PAM_WdReset */

#ifdef PAM_DEADLOOP

/**
******************************************************************************
**
**  @brief      PAM Deadloop
**
**              Reset the Watchdog, and blink the floppy light, until we die.
**
**  @param      none
**
**  @return     none
**
**  @attention  Does not return.
**
******************************************************************************
**/
NORETURN static void PAM_DeadLoop(void)
{
#ifndef ENABLE_NG_LED
    INT32       fdMountStatus = -1;
#endif

    /* Setup watchdog for deadloop. */
    PAM_WdEnable();

#ifdef ENABLE_NG_LED
    lc_led_status(LED_STATE_ON_WARN);
    lc_led_beacon(LED_STATE_ON_OK);
#endif
    /* Loop forever, and ever, and ever, and ... . */
    while (1)
    {
        /* Reset the watchdog timer. */
        PAM_WdReset();

#ifndef ENABLE_NG_LED
        /* Blink the floppy light. */
        if (fdMountStatus != 0)
        {
            fdMountStatus = mount("/dev/fd0", "/mnt/floppy", "ext2", 0, NULL);
        }
        else
        {
            /* Oops, we must have actually mounted something last time,
             * unmount it and try again. */
            umount("/mnt/floppy");
            fdMountStatus = -1;
        }
#endif

        /* Sleep for 5s and try again.
         * Note that a signal breaks through the sleep system call. */
        sleep(5);

        /* Special check for a signal of a type that takes us down immediately. */
        switch (pltErrSig)
        {
            case SIGINT:
            case SIGABRT:
            case SIGTERM:
                /* Does not return. */
                PAM_HandleError();

            case SIGILL:
            case SIGFPE:
            case SIGSEGV:
            case SIGPIPE:
            case SIGBUS:
                /* Reboot, does not return. */
                PAM_ControllerDown(CN_DOWN_REBOOT);
        }
    }
}   /* End of PAM_DeadLoop */
#endif /* PAM_DEADLOOP */

/**
******************************************************************************
**
**  @brief      Debug print routine
**
**  @param      filename    - file where print command was located.
**  @param      lineNum     - line number of print command.
**  @param      levelconst  - Bit mask of allowed prints to process.
**  @param      format      - Format string.
**
**  @return     none
**
******************************************************************************
**/
void PAM_Printf(UINT32 levelconst, char *format, ...)
{
    unsigned int length = 0;
    char        buf[1000];
    char        buf2[64];
    char       *bufP = buf;
    va_list     args;
    time_t      now;
    struct tm   nowLocal;
    struct tm  *now2 = &nowLocal;

    /* Check if we should allow this print to be processed. */
    if (!(levelconst & DPRINTF_PAM_ALLOWED))
    {
        return;
    }

    /* Get the current time for print prefix. */
    time(&now);
    now2 = localtime(&now);
    strftime(buf2, sizeof(buf2) - 1, "%F_%T", now2);

    bufP += sprintf(bufP, "%s :: ", buf2);

    /* Process the variable number of arguments, into a string. */
    va_start(args, format);
    vsprintf(bufP, format, args);
    va_end(args);

    /* Remove the trailing newline, if one exists. This is in here so that we
     * can enable other DPRINTF bits and still have legible output. */
    length = strlen(buf);
    if (length > 1 && buf[length - 1] == '\n')
    {
        buf[(length - 1)] = '\0';
    }

#ifdef PAM_DEBUG
    printf("PAM - ++++ %s ++++\n", buf);
#endif

    if (pamLogFile != NULL)
    {
        fseek(pamLogFile, 0, SEEK_END);
        fprintf(pamLogFile, "%s\n", buf);
        fflush(pamLogFile);
    }
}   /* End of PAM_Printf */

/**
******************************************************************************
**
**  @brief      Send a signal to all platform apps, with extra data value.
**
**  @param      data     - 4 bytes of data to send (see XIO_Const.h)
**
**  @return     none
**
**  @attention  Sets pltErrData, stopping application via PAM_CheckTasks().
**
******************************************************************************
**/
static void PAM_SigAll(UINT32 data)
{
    INT32       index = 0;
    char        string1[128];
    char        string2[128];

    /* Send a kill process to all the processes, iscsi needs a SIGINT. */
    for (index = start_index; index < TASK_MAX; ++index)
    {
        if (index == TASK_ISCSI)
        {
            PAM_KillTask(&tasks[index], SIGINT, 0);
        }
        else
        {
            L_SignalProcess(tasks[index].pid, XIO_PLATFORM_SIGNAL, data);
        }
    }

    /* Only do this if signal handling is not already happening. */
    if (test_and_set_bit(0, &errSignal) == 0)
    {
        /* Set the global error.
         * Setting this variable, forces delayed processing to PAM_CheckTasks(). */
        pltErrSig = XIO_PLATFORM_SIGNAL;
        pltErrData = data;
        /* pltErrForce is not set. */
    }
    else
    {
        /* Must copy from the static structure to be able to use it differently. */
        strncpy(string1, (char *)L_LinuxSignalToString(XIO_PLATFORM_SIGNAL, data), sizeof(string1));
        strncpy(string2, (char *)L_LinuxSignalToString(pltErrSig, pltErrData), sizeof(string2));
        PAM_Printf(DPRINTF_PAM, "Cannot process new signal (%s) - Already in progress (%s)",
                   string1, string2);
    }
}   /* End of PAM_SigAll */

/**
******************************************************************************
**
**  @brief      Set as if a signal was sent, and process it now.
**
**  @param      sig     - Extra data value to set for signal.
**  @param      force   - Non-zero to not wait for core dumps to finish.
**
**  @return     none
**
**  @attention  Does not return.
**  @attention  Sets pltErrSig, pltErrData, pltErrForce, and errSignal.
**
******************************************************************************
**/
NORETURN static void PAM_SetAndHandleError(INT32 errCode, UINT8 force)
{
    test_and_set_bit(0, &errSignal);
    pltErrSig = XIO_PLATFORM_SIGNAL;
    pltErrData = errCode;
    pltErrForce = force;
    /* Does not return. */
    PAM_HandleError();
}   /* End of PAM_SetAndHandleError */

/**
******************************************************************************
**
**  @brief      Catch signals in pam, GOOD or BAD.
**
**  @param      sig     - Signal sent to pam.
**  @param      data    - Extra data value passed to pam.
**
**  @return     none
**
**  @attention  Sets pltErrSig, pltErrData and errSignal for PAM_HandleError().
**
******************************************************************************
**/
static void PAM_SignalCatcher(INT32 sig, UINT32 data)
{

/* PAM_Printf(DPRINTF_PAM, "sig=%d (0x%x), data=%d (0x%x), pltErrStart=%d, errSignal=%d, pltErrSig=%d, pltErrData=%d", sig, sig, data, data, pltErrStart, errSignal, pltErrSig, pltErrData); */
    if (pltErrStart != 0)
    {
        if (sig == SIGINT || sig == SIGILL || sig == SIGFPE ||
            sig == SIGSEGV || sig == SIGPIPE || sig == SIGBUS)
        {
            ;                   /* Let these pass. */
        }
        else
        {
            /* We are already processing signals, do not do any others. */
            return;
        }
    }

    /* Note: SIGINT override other signals, if we are not processing them yet. */
    switch (sig)
    {
        case SIGINT:
            /* Set the bit if it is not already set and force override. */
            test_and_set_bit(0, &errSignal);
            pltErrSig = sig;
            /* Do not need data value. */
            return;

        case SIGABRT:
            signal(SIGABRT, SIG_DFL);
            abort();

        case SIGTERM:
            /* Set the signal, handle in PAM_HandleError(). */
            pltErrSig = sig;
            /* Do not need data value. */
            return;

        case SIGILL:
        case SIGFPE:
        case SIGSEGV:
        case SIGPIPE:
        case SIGBUS:
            /* Set the interrupt bit.
             * If we hit this a second time around (double fault),
             * reboot the controller. */
            if (test_and_set_bit(1, &errSignal) != 0)
            {
                if (reduce_output != 0)
                {
                    return;
                }
                reduce_output = 1;
                PAM_Printf(DPRINTF_PAM, "Caught (%s) - Take 2, Reboot", L_LinuxSignalToString(sig, 0));
                /* Cause a reboot in PAM_HandleSignal() or PAM_DeadLoop(). */
                pltErrSig = sig;
                return;
            }

            PAM_Printf(DPRINTF_PAM, "Caught (%s) - Take 1, Restart BVM", L_LinuxSignalToString(sig, 0));
            /* Send the restart Signal.  This will terminate the CCB/FE/BE
             * (w/core), and jump to pam, up above. Hopefully, we can make it
             * through this, in the presence of the error. If we re-enter this
             * handler, we will try and reboot the controller. */
            PAM_SigAll(ERR_PAM_DIRECTED_DOWN);
            pltErrSig = XIO_PLATFORM_SIGNAL;
            pltErrData = ERR_PAM_DIRECTED_DOWN;
            return;

            /* Coming from one of our applications with extra data value. */
        case XIO_PLATFORM_SIGNAL:
            switch (data)
            {
                    /* If CCB sending heartbeat notification to PAM. */
                case CCB_2_PAM_HB:
                    ++ccbHB;
                    return;

                    /* If we are to go down for some reason. */
                case ERR_EXIT_SHUTDOWN:                 // 0x10
//                 case ERR_EXIT_BE_MISS_HB:        UNUSED // 0x11
//                 case ERR_EXIT_FE_MISS_HB:        UNUSED // 0x12
                case ERR_EXIT_RESET_CCB:                // 0x13
                case ERR_EXIT_RESET_ALL:                // 0x14
                case ERR_EXIT_DEADLOOP:                 // 0x15
                case ERR_EXIT_REBOOT:                   // 0x16
                case ERR_EXIT_FIRMWARE:                 // 0x17
                case ERR_EXIT_BVM_RESTART:              // 0x18
                case ERR_EXIT_BIOS_REBOOT:              // 0x19
                    /* Set the interrupt bit. */
                    if (test_and_set_bit(0, &errSignal) != 0)
                    {
                        char        string1[128];
                        char        string2[128];

                        strncpy(string1, (char *)L_LinuxSignalToString(XIO_PLATFORM_SIGNAL, data), sizeof(string1));
                        strncpy(string2, (char *)L_LinuxSignalToString(pltErrSig, pltErrData), sizeof(string2));
                        PAM_Printf(DPRINTF_PAM, "Caught (%s) - Already in progress (%s)",
                                   string1, string2);
                        return;
                    }

                    /* Set the signal. */
                    pltErrSig = sig;
                    pltErrData = data;

                    /* If Shutdown, or Reboot, do not wait for cores. */
                    if (data == ERR_EXIT_SHUTDOWN || data == ERR_EXIT_REBOOT ||
                        data == ERR_EXIT_BIOS_REBOOT)
                    {
                        pltErrForce = 1;
                        return;
                    }

                    /* Signal the platform.
                     * NOTE: The PAM_SigALL() sets same data as above, so it is ok. */
                    PAM_SigAll(data);
                    return;

                case ERR_PAM_DIRECTED_DOWN:             // 0x20
                default:
                    PAM_Printf(DPRINTF_PAM, "PAM signal -- %s (%s) data %u",
                               data == ERR_PAM_DIRECTED_DOWN ? "PAM" : "Unhandled",
                               L_LinuxSignalToString(sig, data), data);
                    PAM_SigAll(ERR_PAM_DIRECTED_DOWN);
                    /* Force how to go down to a different type. */
                    pltErrSig = XIO_PLATFORM_SIGNAL;
                    pltErrData = ERR_PAM_DIRECTED_DOWN;
                    return;
            }

        default:
            /* We need to be harsh and ignore all other signals,
             * and let the laws of nature proceed.  In system event
             * cases, the platform will receive the same signals,
             * and we need to be here to handle their reaction. */
            PAM_Printf(DPRINTF_PAM, "PAM signal -- Unhandled (%s) - Ignoring", L_LinuxSignalToString(sig, 0));
            return;
    }
}   /* End of PAM_SignalCatcher */

/**
******************************************************************************
**
**  @brief      Open the pam LogFile
**
**  @param      none
**
**  @return     none
**
**  @attention  If the logfile is already open, close it first.
**
******************************************************************************
**/
static void PAM_OpenLogFile(void)
{
    /* If the logfile is open, close it first. */
    if (pamLogFile)
    {
        PAM_Printf(DPRINTF_PAM, "Closing LogFile");
        fclose(pamLogFile);
    }

    /* Open the Log File. We will continue to run,
     * regardless if we can open this file. */
    pamLogFile = fopen(PAM_LOGFILE, "a");

    /* Print some eye catching information for pam.log, that the logfile is open. */
    PAM_Printf(DPRINTF_PAM, "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    PAM_Printf(DPRINTF_PAM, "Opened LogFile");
}   /* End of PAM_OpenLogFile */

/**
******************************************************************************
**
**  @brief      Exit with an error
**
**              Kill all the processes and exit with an error (pam goes down).
**
**  @param      error   - error code to exit pam with.
**
**  @return     Does not return.
**
**  @attention  Does not return.
**
******************************************************************************
**/
NORETURN static void PAM_ExitError(INT32 error)
{
    PAM_Printf(DPRINTF_PAM, "Exiting(%d), caught SIGINT or SUICIDE_DISABLED", error);

    /* Kill all the processes. */
    PAM_KillAll(SIGKILL, 1);

    /* This is our safe path out, disable the watchdog. */
    if (wdEnabled)
    {
        PAM_WdDisable();
    }

    /* Exit. */
    exit(error);
}   /* PAM_ExitError */

/***
** Modelines
** vi:sw=4 ts=4 expandtab
**/
