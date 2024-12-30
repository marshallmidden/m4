/* $Id: xk_kernel.c 159129 2012-05-12 06:25:16Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       xk_kernel.c
**
**  @brief      This is a wrapper for the I960 assembler kernel
**
**  This module will define kernel functions to the CCB code running in the
**  Linux environment. This will preserve our concept of a cooperative
**  multitasking codebase.
**
**  Copyright (c) 2004-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

/* Linux includes */
#include <bits/local_lim.h>
#include <errno.h>
#include <execinfo.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>

/* Xiotech includes */
#include "xk_kernel.h"

#include "debug_files.h"
#include "errorCodes.h"
#include "error_handler.h"
#include "heap.h"
#include "idr_structure.h"
#include "logging.h"
#include "LL_LinuxLinkLayer.h"
#include "li_evt.h"
#include "mutex.h"
#include "pcb.h"
#include "XIO_Const.h"
#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "XIO_Types.h"
#include "xio3d.h"
#include "xk_mapmemfile.h"
#include "hw_common.h"
#include "memory.h"
#ifdef HISTORY_KEEP
#include "CT_history.h"
#endif  /* HISTORY_KEEP */

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/

//#define KERNEL_DEBUG
#define XK_KERNEL_PROFILE TRUE

/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/

#define  XK_MUTEX_MAX           100
#define  XK_MAX_THREADS         200
#define  XK_MAX_SHARED_THREADS  6
#define  XK_KERNEL_ACCESS       0
#define  XK_KERNEL_STACK_SIZE   SIZE_64K
#define  XK_HIGH_PRI_TASK       50

#define  XK_TryKMutex(slot)             pthread_mutex_trylock(&xkMutexList[slot].mutex)
#define  XK_LockKMutex(slot)            pthread_mutex_lock(&xkMutexList[slot].mutex)
#define  XK_UnlockKMutex(slot)          pthread_mutex_unlock(&xkMutexList[slot].mutex)

#define  XK_Signal(pTask)               pthread_cond_signal(&pTask->signal)

#define  XK_TaskYield(pTask)            XK_TaskWaitSig(&pTask->signal)
#define  XK_TaskWaitSig(pCnd)           pthread_cond_wait(pCnd, \
                                                          &xkMutexList[XK_KERNEL_ACCESS].mutex)

#define  XK_TaskMoveQueue(qFrom,qTo,pTask) XK_TaskEnqueue(&qTo, \
                                        XK_TaskDequeueTask(&qFrom, pTask))

#define get_tsc()       ({ unsigned long long __scr; \
        __asm__ __volatile__("rdtsc" : "=A" (__scr)); __scr;})

#define save_regs(xkpcb) do { (xkpcb)->pcb.pc_pfp = (SF *)get_ebp(); \
    (xkpcb)->pcb.pc_esp = get_esp(); } while (0)

#if defined(MODEL_3000) || defined(MODEL_4700)   /* usec * msec */
#define CPUSPEED    3200
#else                       /* usec * msec */
#define CPUSPEED    2333
#endif                      /* usec * msec */
#define TSC_ONE_MSEC_DELAY             (xkCpuSpeed * 1000)
#define TSC_ONE_SEC_DELAY              (xkCpuSpeed * (1000 * 1000))
#ifndef M4_ABORT
#define XK_KERNEL_TASK_MAX_LIMIT       (TSC_ONE_SEC_DELAY * 2)
#else   /* M4_ABORT */
#define XK_KERNEL_TASK_MAX_LIMIT       (TSC_ONE_SEC_DELAY * 10)
#endif  /* M4_ABORT */

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/

typedef struct XK_PCB
{
    PCB         pcb;            /* Old CCB pcb                          */
    TASK_PARMS  parms;          /* parameters fo tasks                  */
    UINT32      shared;         /* This PCB is in shared memory         */
    pthread_t   threadId;       /* ID of corresponding pthread          */
    struct XK_PCB *next;        /* Pointer to next pcb                  */
    struct XK_PCB *prev;        /* Pointer to previous pcb              */
    void        (*functionPtr)(TASK_PARMS *);     /* Function pointer   */
    UINT8      *stack;          /* Our stack for the pthread            */
    pthread_cond_t signal;      /* Signal used to wake up pthread       */
} XK_PCB;

typedef struct _XK_TASK_QUEUE
{
    UINT32      taskCount;      /* count of tasks in the queue          */
    XK_PCB     *tskPtr;         /* pointer to front of queue            */
} XK_TASK_QUEUE;

typedef struct _XK_MUTEX
{
    UINT8       used;           /* mutex is taken                   */
    UINT8       rsvd[3];        /* Reserved                         */
    char        init_file[128]; /* File where mutex init was done.  */
    UINT32      init_line;      /* Line where mutex init was done.  */
    char        lock_file[128]; /* File where mutex lock was done.  */
    UINT32      lock_line;      /* Line where mutex lock was done.  */
    XK_PCB     *lock_xk_pcb;    /* XK_PCB where mutex lock gotten.  */
    void (*lock_func)(TASK_PARMS *); /* Locked PCB function pointer */
    pthread_mutex_t mutex;      /* pthread mutex                    */
} XK_MUTEX;

/*
******************************************************************************
** Private variables
******************************************************************************
*/
static pthread_cond_t cpuMemChg = PTHREAD_COND_INITIALIZER;
pthread_key_t threadKeys;
static pthread_mutexattr_t xkMutexAttr;
static sem_t xkWakeTasksSem;
static sem_t xkKernelTaskSem;
static pthread_t xkWakeTasksId;
static pthread_t xkKernelTasksId;

static XK_MUTEX xkMutexList[XK_MUTEX_MAX];

static XK_TASK_QUEUE xkRunQueue;
static XK_TASK_QUEUE xkPoolQueue;
static XK_TASK_QUEUE xkSharedPoolQueue;

static volatile UINT32 xkTasksStarted = 0;
static volatile UINT32 xkMallocWaitCount = 0;
static volatile UINT64 xkTaskCurrentTime = 0;
static volatile UINT64 xkTaskLastTime = 0;
static UINT64 xkInitalTime = 0;
static UINT64 xkInitalTsc = 0;

#ifdef M4_ABORT
static volatile XK_PCB      *xkTaskLastRun = 0;
static volatile UINT32       xkTaskLastRunCount = 0;
#define xk_MAX_SEEN         2000
static volatile int          xk_TaskLastSeen = 0;
static volatile char const  *xk_TaskLastFILE[xk_MAX_SEEN];
static volatile unsigned int xk_TaskLastLINE[xk_MAX_SEEN];
static volatile char const  *xk_TaskLastFUNC[xk_MAX_SEEN];
static volatile unsigned int xk_TaskLastCount[xk_MAX_SEEN];
#endif  /* M4_ABORT */

UINT64 xkCpuSpeed = CPUSPEED;

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
UINT32      kernel_sleep LOCATE_IN_SHMEM;
UINT32      kernel_up_counter LOCATE_IN_SHMEM;
XK_PCB     *XK_pcb = NULL;
PCB        *K_xpcb = NULL;

#ifdef PERF
#define NumberLastPcbs  64
#else   /* PERF */
#ifndef M4_ABORT
#define NumberLastPcbs  (512)
#else   /* M4_ABORT */
#define NumberLastPcbs  (5120)
#endif  /* M4_ABORT */
#endif  /* PERF */
UINT32      number_last_pcbs = NumberLastPcbs;
XK_PCB     *k_last_pcbs[NumberLastPcbs];
char       *last_pc_fork_name[NumberLastPcbs][XIONAME_MAX];
void       *last_functionPtr[NumberLastPcbs];
UINT64      last_runtime[NumberLastPcbs];

#ifdef HISTORY_KEEP
#define update_K_xpcb_history                                                           \
    CT_history_printf("%s:%u:%s switching to %p '%s'@%p\n", __FILE__, __LINE__, __func__, \
                      XK_pcb, XK_pcb->pcb.pc_fork_name, XK_pcb->functionPtr);
#else   /* HISTORY_KEEP */
#define update_K_xpcb_history
#endif  /* HISTORY_KEEP */

#define update_K_xpcb_etc()                                                             \
    {                                                                                   \
        XK_pcb = XK_GetCurrentTask();                                                   \
        if (XK_pcb != NULL) {                                                           \
            K_xpcb = &XK_pcb->pcb;                                                      \
        } else {                                                                        \
            K_xpcb = NULL;                                                              \
        }                                                                               \
        memmove(&k_last_pcbs[1], &k_last_pcbs[0],                                       \
                (NumberLastPcbs-1) * sizeof(k_last_pcbs[0]));                           \
        k_last_pcbs[0] = XK_pcb;                                                        \
        memmove(&last_pc_fork_name[1][0], &last_pc_fork_name[0][0],                     \
                (NumberLastPcbs-1) * sizeof(last_pc_fork_name[0][0]) * XIONAME_MAX);    \
        memmove(&last_pc_fork_name[0][0], &XK_pcb->pcb.pc_fork_name[0], XIONAME_MAX);   \
        memmove(&last_functionPtr[1], &last_functionPtr[0],                             \
                (NumberLastPcbs-1) * sizeof(last_functionPtr[0]));                      \
        last_functionPtr[0] = XK_pcb->functionPtr;                                      \
        memmove(&last_runtime[1], &last_runtime[0],                                     \
                (NumberLastPcbs-1) * sizeof(last_runtime[0]));                          \
        last_runtime[0] = get_tsc();                                                    \
        update_K_xpcb_history;                                                          \
    }

/*
******************************************************************************
** Public routines - NOT externed in any header file
******************************************************************************
*/
extern void XK_KernelInitTime(void);
extern UINT64 XK_KernelGetTime(void);

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static INT32 XK_InitPCB(XK_PCB *pcb, pthread_attr_t *attr);
static void XK_TaskEnqueue(XK_TASK_QUEUE *taskQueue, XK_PCB *task);
static XK_PCB *XK_TaskDequeue(XK_TASK_QUEUE *taskQueue);
static XK_PCB *XK_TaskDequeueTask(XK_TASK_QUEUE *taskQueue, XK_PCB *task);
static INT32 XK_InitializeMutex(MUTEX mutex, const char *, const UINT32);
static void XK_PreBlockingCall(UINT8 state);
static void XK_PostBlockingCall(UINT8 state);
static NORETURN void XK_Task(void *parms);
XK_PCB *XK_GetCurrentTask(void);

static NORETURN void XK_KernelTask(void *parms);
static NORETURN void XK_TimerTask(void *parms);

static void XK_KernelError(const char *fmt, ...) __attribute__ ((__format__(__printf__, 1, 2)));
static void XK_KernelPrintStack(void);

/*
******************************************************************************
** Functions not defined in any header file
******************************************************************************
*/
extern int  pthread_mutexattr_settype(pthread_mutexattr_t *, int);
extern int  pthread_attr_setstack(pthread_attr_t *, void *, size_t);
extern int  pthread_attr_getstack(pthread_attr_t *, void **, size_t *);

/*
******************************************************************************
** Variables not externed in any header file
******************************************************************************
*/
extern int  CT_xiofd;

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Initialize XioKernel
**
**              Initialize XioKernel
**
**  @param      none    - use if there are no params
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
INT32 XK_InitKernel(void)
{
    INT32       rc = ERROR;
    MUTEX       index1;
    UINT32      index2;
    XK_PCB     *pTask1 = NULL;
    pthread_attr_t xkAttr;

    do
    {
        /*
         * Initialize task queues to NULL
         * pointers, and a count of zero.
         */
        xkRunQueue.taskCount = 0;
        xkRunQueue.tskPtr = NULL;
        xkPoolQueue.taskCount = 0;
        xkPoolQueue.tskPtr = NULL;
        xkSharedPoolQueue.taskCount = 0;
        xkSharedPoolQueue.tskPtr = NULL;

        /* Initialize our pthread attributes. */
        if ((rc = pthread_attr_init(&xkAttr)) != GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "Unable to create pthread_attr_init\n");
            break;
        }

        /*
         * Create thread Keys.  This will be used to set thread specific
         * data.  We will use this to have each thread save away its
         * pointer to it's pcb.  When we need to know the currently
         * running task is we have get the threads specific data and
         * look at the pointer to their pcb.
         */
        if ((rc = pthread_key_create(&threadKeys, NULL)) != GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "Unable to create threadKeys\n");
            break;
        }

        /* Initialize our mutex attributes. */
        if ((rc = pthread_mutexattr_init(&xkMutexAttr)) != GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "Unable to create pthread_mutexattr_init\n");
            break;
        }

        if ((rc = pthread_mutexattr_settype(&xkMutexAttr, PTHREAD_MUTEX_RECURSIVE_NP))
            != GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "Unable to create pthread_mutexattr_init\n");
            break;
        }

        /* Initialize the mutex list. */
        for (index1 = 0; index1 < XK_MUTEX_MAX; ++index1)
        {
            xkMutexList[index1].used = FALSE;
        }

        /* Initialize the Kernel mutex. */
        if ((rc = XK_InitializeMutex(XK_KERNEL_ACCESS, __FILE__, __LINE__)) != GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "XK_InitializeMutex(XK_KERNEL_ACCESS) failed\n");
            break;
        }

        /* Initialize the Kernel Task semaphore. */
        if ((rc = sem_init(&xkKernelTaskSem, 0, 0)) != GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "sem_init(&xkKernelTaskSem, 0, 0) failed\n");
            break;
        }

        /* Create the Kernel task. */
        if ((rc = pthread_create(&xkKernelTasksId, &xkAttr, (void *)XK_KernelTask, NULL))
            != GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "XK_Init Error creating thread XK_KernelTask\n");
            break;
        }

        /* Initialize the Wake Tasks semaphore. */
        if ((rc = sem_init(&xkWakeTasksSem, 0, 0)) != GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "sem_init(&xkWakeTasksSem, 0, 0) failed\n");
            break;
        }

        /* Create the Wake Tasks task. */
        if ((rc = pthread_create(&xkWakeTasksId, &xkAttr, (void *)XK_TimerTask, NULL))
            != GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "XK_Init Error creating thread XK_TimerTask\n");
            break;
        }

        /* Create our pool of threads */
        for (index2 = 0; index2 < XK_MAX_THREADS; ++index2)
        {
            UINT32 sharedThread = (index2 < XK_MAX_SHARED_THREADS);

            /*
             * Allocate the memory for the pcb and initialize it. The first XK_MAX_SHARED_THREADS
             * have their PCB in shared memory, the rest have PCBs on the local heap.
             */
            if (sharedThread)
            {
                pTask1 = s_Malloc(sizeof(*pTask1), __FILE__, __LINE__);
            }
            else
            {
                pTask1 = p_Malloc(sizeof(*pTask1), __FILE__, __LINE__);
                if (pTask1 == NULL)
                {
                    pTask1 = s_Malloc(sizeof(*pTask1), __FILE__, __LINE__);
                }
            }
            if (pTask1 == NULL)
            {
                dprintf(DPRINTF_DEFAULT, "malloc(pFreeThr) failed\n");
                break;
            }

            /* Clear memory -- just in case. */
            memset(pTask1, 0, sizeof(*pTask1));

            if(!XK_InitPCB(pTask1, &xkAttr))
            {
                break;
            }
            pTask1->shared = sharedThread;


            /* Create the thread. */
            if ((rc = pthread_create(&pTask1->threadId, &xkAttr, (void *)XK_Task, pTask1))
                != GOOD)
            {
                dprintf(DPRINTF_DEFAULT, "XK_Init Error creating thread %d, rc = %d\n",
                        index2, rc);
                break;
            }
#ifdef KERNEL_DEBUG
            else
            {
                dprintf(DPRINTF_DEFAULT, "Thread %u created (0x%08X)\n",
                        (UINT32)pTask1->threadId, (UINT32)pTask1);
            }
#endif  /* KERNEL_DEBUG */

            /* If we failed somewhere break out of this loop. */
            if (rc != GOOD)
            {
                break;
            }

            /*
             * Enqueue the task to the Pool queue of threads
             * available for use.
             */
            if(pTask1->shared)
            {
                XK_TaskEnqueue(&xkSharedPoolQueue, pTask1);
            }
            else
            {
                XK_TaskEnqueue(&xkPoolQueue, pTask1);
            }

            /* Null our pointer and do it all over again. */
            pTask1 = NULL;
        }

        /*
         * Check that we did not have any errors and that
         * we created all the threads we said we would.
         */
        if ((index2 < XK_MAX_THREADS) || (rc != GOOD))
        {
            dprintf(DPRINTF_DEFAULT, "XK_Init Error creating threads\n");
            break;
        }

        /* Wait for all of our children to come alive!. */
        while (XK_KernelReady() == FALSE)
        {
            usleep(10000);
        }

        /* We made it!  Set our return value to reflect our success. */
        rc = GOOD;
    } while (0);

    /* Check that we did not have any errors. */
    if (rc != GOOD)
    {
        rc = ERROR;
        XK_KernelError("XK_Init FATAL ERROR");
    }

#ifdef KERNEL_DEBUG
    dprintf(DPRINTF_DEFAULT, "Initializing complete\n");
#endif  /* KERNEL_DEBUG */

    return rc;
}


/**
******************************************************************************
**
**  @brief      Initialize an XK_PCB
**
**              Initialize an XK_PCB
**
**  @param      pcb  - Pointer to XK_PCB to initialize
**  @param      attr - Attribute block
**
**  @return     TRUE is initialization successful, FALSE otherwise
**
******************************************************************************
**/
INT32 XK_InitPCB(XK_PCB *pcb, pthread_attr_t *attr)
{
    INT32       rc = ERROR;

    /* Allocate the memory for the stack. */

    pcb->stack = p_Malloc(XK_KERNEL_STACK_SIZE + 31, __FILE__, __LINE__);
    if (pcb->stack == NULL)
    {
        dprintf(DPRINTF_DEFAULT, "malloc(XK_InitPCB) failed\n");
        return(FALSE);
    }
    /* Clear memory -- just in case. */
    memset(pcb->stack, 0, sizeof(*pcb->stack));

    /* Make aligned. */
    pcb->stack = (UINT8 *)(((UINT32)pcb->stack + 31) & 0xFFFFFFE0);

    /* Set the function pointer to NULL and clear the parameters. */
    pcb->functionPtr = NULL;
    memset((UINT8 *)&pcb->parms, 0x00, sizeof(TASK_PARMS));

    /*
     * Set the task status to not ready, and the name to reflect its
     * residing in the pool queue.
     */
    pcb->pcb.pc_stat = PCB_NOT_READY;
    strncpy(pcb->pcb.pc_fork_name, "POOL TASK", XIONAME_MAX);

    /* Initialize the thread's condition variable (Signal). */
    if ((rc = pthread_cond_init(&pcb->signal, NULL)) != GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "pthread_cond_init failed, rc = %d\n", rc);
        return(FALSE);
    }

    /* Set the threads stack and size. */
    if ((rc = pthread_attr_setstack(attr, (void *)pcb->stack,
                                    XK_KERNEL_STACK_SIZE)) != GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "pthread_attr_setstack (0x%08X) failed: %s\n",
                (UINT32)pcb->stack, strerror(rc));
        return(FALSE);
    }
    return(TRUE);
}


/**
******************************************************************************
**
**  @brief      Keep time internal to prevent all the system calls.
**
**  @return     None.
**
******************************************************************************
**/
void XK_KernelInitTime(void)
{
    if (getenv("CPU_SPEED"))
    {
        char       *strCpuSpeed = getenv("CPU_SPEED");

        xkCpuSpeed = atoll(strCpuSpeed);
        dprintf(DPRINTF_DEFAULT, "XK_Kernel, setting CPUSPEED to %llu usec\n",
                xkCpuSpeed);
    }

    xkInitalTime = (UINT64)time(NULL);
    xkInitalTsc = get_tsc();
}


/**
******************************************************************************
**
**  @brief      Retrieve the internally held time.
**
**  @return     None.
**
******************************************************************************
**/
UINT64 XK_KernelGetTime(void)
{
    UINT64      cTime = get_tsc();

    /* Update every 24 hours. */
    if ((xkInitalTime + ((cTime - xkInitalTsc) / TSC_ONE_SEC_DELAY)) >
        (xkInitalTime + (60 * 60 * 24)))
    {
        XK_KernelInitTime();
        cTime = get_tsc();

        xkTaskCurrentTime = get_tsc();      /* Not exactly right, but stop problems. */
        xkTaskLastTime = xkTaskCurrentTime;
    }

    return (xkInitalTime + ((cTime - xkInitalTsc) / TSC_ONE_SEC_DELAY));
}


/**
******************************************************************************
**
**  @brief      Test if the kernel is ready to create threads.
**
**  @return     TRUE if the kernel is ready, FALSE otherwise.
**
******************************************************************************
**/
UINT32 XK_KernelReady(void)
{
    return (xkTasksStarted < XK_MAX_THREADS) ? FALSE : TRUE;
}


/**
******************************************************************************
**
**  @brief      Notify the kernel that there is a task to schedule.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void XK_Schedule(void)
{
    INT32       return_status;

    kernel_up_counter++;
    sem_post(&xkKernelTaskSem);
    if (kernel_sleep)
    {
#ifdef M4_ABORT
        xk_TaskLastSeen = 0;
#endif  /* M4_ABORT */
        return_status = ioctl(CT_xiofd, XIO3D_SENDEVT, XIO3D_CCB_EVT);
        if (return_status != 0)
        {
            LogMessage(LOG_TYPE_DEBUG, "XK_Kernel - Send kernel wake event ioctl returned %d, errno=%d",
                       return_status, errno);
            perror("Send kernel wake event");
        }
    }
}


/**
******************************************************************************
**
**  @brief      Create a new task and schedule the task to run.
**
**  @param      funcPtr - pointer to the function to execute when scheduled.
**  @param      parms   - parameters* passed to above function.
**  @param      shared  - if true, use PCB shared pool, else use local pool
**
**  @return     pointer to pcb of newly created task
**
******************************************************************************
**/
PCB        *XK_TaskCreate(void (funcPtr)(TASK_PARMS *), TASK_PARMS *parms, UINT32 shared)
{
    XK_PCB     *pTask = NULL;
    static unsigned int task_count = 0;

    /* Dequeue a task from the pool queue. */
    if (shared)
    {
        pTask = XK_TaskDequeue(&xkSharedPoolQueue);
    }
    else
    {
        pTask = XK_TaskDequeue(&xkPoolQueue);
    }

    /* If we successfully retrieved a task from the pool queue, carry on. */
    if (pTask)
    {
        /* Set the function pointer in the pcb. */
        pTask->functionPtr = funcPtr;

        /* Copy the parameters. */
        if (parms != NULL)
        {
            memcpy(((UINT8 *)&pTask->parms), (UINT8 *)parms, sizeof(TASK_PARMS));
        }

        /* Set the task state to ready in the pcb. */
        pTask->pcb.pc_stat = PCB_READY;

        /* Set the task name. Maybe someday this will be more meaningful. */

        sprintf(pTask->pcb.pc_fork_name, "CCB Task %d", ++task_count);

        /* Place the task on the run queue to be scheduled to run. */
        XK_TaskEnqueue(&xkRunQueue, pTask);

#ifdef KERNEL_DEBUG
        dprintf(DPRINTF_DEFAULT, "XK_TaskCreate 0x%08X, Active Tasks %d\n",
                (UINT32)pTask, xkRunQueue.taskCount);
#endif  /* KERNEL_DEBUG */

/*
** TODO: Maybe add some sort of start
**       function instead of automatically
**       starting first task.
*/
        /* If this is the first task, start it. */
        if (xkRunQueue.taskCount == 1)
        {
            if (XK_Signal(pTask) != 0)
            {
                XK_KernelError("XK_TaskCreate Failed pthread_cond_signal");
            }
        }
    }

/*
** TODO: Maybe add some sort of way to dynamically
**       add tasks to pool queue so we don't die here.
*/
    /* If we cannot get a task to create, DIE. */
    else
    {
        XK_KernelError("XK_TaskCreate - No Threads in pool");
    }

    /* Return the pcb of the new task. */
    return (PCB *)pTask;
}


/**
******************************************************************************
**
**  @brief      Give up control of the CPU and allow another task to run.
**
**  @param      none
**
**  @return     none
**
**  @attention  This function gives up control of the CPU, and will
**              cause the callers task to be rescheduled to be run.
**              When the task is rescheduled by this pseudokernel and
**              woken up to run, this function will return.  This allows
**              for a cooperative multitasking environment in Linux.
**
******************************************************************************
**/

#ifdef M4_ABORT
void XK_TaskSwitch(const char *file, const unsigned int line, const char *func)
#else   /* M4_ABORT */
void XK_TaskSwitch(void)
#endif  /* M4_ABORT */
{
    XK_PCB     *pTask = XK_GetCurrentTask();

#ifdef KERNEL_DEBUG
    dprintf(DPRINTF_DEFAULT, "XK_TaskSwitch pTask\n");
#endif  /* KERNEL_DEBUG */

    /* Get our current Frame pointer. */
    save_regs(pTask);

    /* Check timing. */
    xkTaskCurrentTime = get_tsc();
    if (xkTaskLastTime &&
        ((xkTaskCurrentTime - xkTaskLastTime) > XK_KERNEL_TASK_MAX_LIMIT))
    {
        dprintf(DPRINTF_DEFAULT, "XK_TaskSwitch: Task took too long (%llu msec)\n",
                ((xkTaskCurrentTime - xkTaskLastTime) / TSC_ONE_MSEC_DELAY));
        XK_KernelPrintStack();
#ifdef M4_ABORT
abort();
#endif  /* M4_ABORT */
    }

    xkTaskLastTime = get_tsc();

#ifdef M4_ABORT
    xkTaskLastRun = pTask;

  {
    int i;
    int j = -1;

    for (i = 0; i < xk_TaskLastSeen; i++)
    {
        if (strcmp((char *)xk_TaskLastFUNC[i], func) == 0 &&
            strcmp((char *)xk_TaskLastFILE[i], file) == 0 &&
            xk_TaskLastLINE[i] == line)
        {
            j = 0;
            xk_TaskLastCount[i]++;
            break;
        }
    }
    if (j != 0)
    {
        xk_TaskLastFILE[xk_TaskLastSeen] = file;
        xk_TaskLastLINE[xk_TaskLastSeen] = line;
        xk_TaskLastFUNC[xk_TaskLastSeen] = func;
        xk_TaskLastCount[xk_TaskLastSeen] = 1;
        if (xk_TaskLastSeen < xk_MAX_SEEN )
        {
            xk_TaskLastSeen++;
        }
    }
  }
#endif  /* M4_ABORT */

    /*
     * Signal the next task to run, if we have successfully scheduled
     * another task other than ourselves, give control to the scheduler.
     */
    XK_Schedule();
    XK_TaskYield(pTask);

    xkTaskLastTime = get_tsc();

#ifdef M4_ABORT
    /* Find out if we were the last task to run? */
    if (xkTaskLastRun == pTask)
    {
        xkTaskLastRunCount++;
    }
    else
    {
        int i;
        int j = 0;

        for (i = 0; i < xk_TaskLastSeen; i++)
        {
            if (strcmp((char *)xk_TaskLastFUNC[i], "XK_PostBlockingCall") != 0 &&
                strcmp((char *)xk_TaskLastFUNC[i], "XK_TaskSleepMS") != 0)
            {
                j += xk_TaskLastCount[i];
            }
        }

        if (j >= 20)
        {
            dprintf(DPRINTF_DEFAULT, "%s: called %u times in a row by %p\n",
                    __func__, xkTaskLastRunCount, xkTaskLastRun);
            for (i = 0; i < xk_TaskLastSeen; i++)
            {
                dprintf(DPRINTF_DEFAULT, "%s: %u from %s:%u:%s\n", __func__,
                        xk_TaskLastCount[i], xk_TaskLastFILE[i], xk_TaskLastLINE[i], xk_TaskLastFUNC[i]);
            }
        }
        xkTaskLastRunCount = 0;
        xk_TaskLastSeen = 0;
    }
#endif  /* M4_ABORT */

    update_K_xpcb_etc();
}


/**
******************************************************************************
**
**  @brief      Put a task to sleep for msec milliseconds.
**
**  @param      msec  - milliseconds to sleep for
**
**  @return     none
**
******************************************************************************
**/
void XK_TaskSleepMS(UINT32 msec)
{
    XK_PCB     *pTask = NULL;

#ifdef KERNEL_DEBUG
    dprintf(DPRINTF_DEFAULT, "%lu XK_TaskSleepMS %d enter %d(msec)\n", time(NULL),
            (UINT32)pthread_self(), msec);
#endif  /* KERNEL_DEBUG */
#ifdef HISTORY_KEEP
    CT_history_printf("%s:%u:%s %p '%s'@%p sleep for %u\n", __FILE__, __LINE__, __func__, \
                      XK_pcb, XK_pcb->pcb.pc_fork_name, XK_pcb->functionPtr, msec);
#endif  /* HISTORY_KEEP */

    /* If the caller is asking for a msec sleep of 0, just task switch. */
    if (msec == 0)
    {
#ifdef M4_ABORT
        XK_TaskSwitch(__FILE__, __LINE__, __func__);
#else   /* M4_ABORT */
        XK_TaskSwitch();
#endif  /* M4_ABORT */
    }

    /* If the caller is asking for a msec sleep > 0, go to sleep. */
    else
    {
        /* Retrieve a pointer to the currently running task. */
        pTask = XK_GetCurrentTask();

        /* Set our task state to timer wait and time to sleep. */
        TaskSetState((PCB *)pTask, PCB_TIMED_WAIT);

        /* Set our sleep time. */
        pTask->pcb.pc_time = msec;

        /* TaskSwitch. */
#ifdef M4_ABORT
        XK_TaskSwitch(__FILE__, __LINE__, __func__);
#else   /* M4_ABORT */
        XK_TaskSwitch();
#endif  /* M4_ABORT */
    }

#ifdef KERNEL_DEBUG
    dprintf(DPRINTF_DEFAULT, "%lu XK_TaskSleepMS %d exit %d(msec)\n", time(NULL),
            (UINT32)pthread_self(), msec);
#endif  /* KERNEL_DEBUG */
}


/**
******************************************************************************
**
**  @brief      Allocates Memory with Wait (and clear).
**
**  @param      size   - Size of memory to allocate
**  @param      type   - Type of allocation to do.
**                       MEM_MALLOC - Malloc with wait
**                       MEM_CALLOC - Malloc with wait and clear
**  @param      shared - If true, allocate from shared memory. Else allocate from local memory.
**  @param      file   - pass the __FILE__ macro
**  @param      line   - pass the __LINE__ macro
**
**  @return     pointer to allocated memory.
**
**  @attention  This function will not return until it succeeds.  If it
**              is unable to allocate memory, it will not block while waiting.
**
**              NOTE: DO NOT CALL THIS FUNCTION DIRECTLY... USE THE DEFINES
**              MALLOCW AND MALLOCWC DEFINED IN XIO_STD.H
**
******************************************************************************
**/
void       *XK_Malloc(UINT32 size, UINT32 type, UINT32 shared, const char *file, const UINT32 line)
{
    XK_PCB     *pTask = NULL;
    void       *pMem = NULL;

    /* Use correct malloc to allocate the memory. */
    if (shared)
    {
        pMem = s_Malloc(size, file, line);
    }
    else
    {
        pMem = p_Malloc(size, file, line);
        if (pMem == NULL)
        {
            pMem = s_Malloc(size, file, line);
        }
    }

    /*
     * If the allocation failed, we need to wait
     * for another task to free some memory.
     */
    if (pMem == NULL)
    {
        /* Get our current task. */
        pTask = XK_GetCurrentTask();

        /* Set our state to waiting for memory. */
        TaskSetState((PCB *)pTask, PCB_WAIT_SRAM);

        /* Signal the next task to run. */
        XK_Schedule();

        /* Get our current Frame pointer. */
        save_regs(pTask);

        /*
         * Bump the Malloc wait count, so our free knows
         * to signal us when some memory has been cleared.
         */
        ++xkMallocWaitCount;

        /*
         * While we have not successfully allocated memory,
         * keep trying until we are able to get it.
         */
        while (pMem == NULL)
        {
            dprintf(DPRINTF_DEFAULT, "%s: MEM_WAIT_STATE: 0x%08X size=0x%x (%d)\n",
                        __func__, (UINT32)pTask, size, size & 0x7fffffff);
            LogMessage(LOG_TYPE_DEBUG, "XK_Kernel - MEM_WAIT_STATE: 0x%08X size: %u in %s heap",
                       (UINT32)pTask, size & 0x7fffffff, (shared ? "shared" : "local"));

            /*
             * Wait for the signal from another task signalling
             * that some memory has been freed, and we can try again.
             * This will automatically unlock the kernel mutex,
             * and reacquire it again.
             */
            XK_TaskWaitSig(&cpuMemChg);

            LogMessage(LOG_TYPE_DEBUG, "XK_Kernel - MEM_CHANGE: 0x%08X size: %u in %s heap",
                       (UINT32)pTask, size & 0x7fffffff, (shared ? "shared" : "local"));

            /* Use malloc to allocate the cleared memory. */
            if (shared)
            {
                pMem = s_Malloc(size, file, line);
            }
            else
            {
                pMem = p_Malloc(size, file, line);
                if (pMem == NULL)
                {
                    pMem = s_Malloc(size, file, line);
                }
            }
        }

        /*
         * Decrement the Malloc wait count, so our free knows
         * not to signal us when some memory has been cleared.
         */
        --xkMallocWaitCount;

        /*
         * We successfully (finally) were able to allocate
         * some memory.  Set our state to ready.
         */
        TaskSetState((PCB *)pTask, PCB_READY);

        /* TaskSwitch */
#ifdef M4_ABORT
        XK_TaskSwitch(__FILE__, __LINE__, __func__);
#else   /* M4_ABORT */
        XK_TaskSwitch();
#endif  /* M4_ABORT */

        LogMessage(LOG_TYPE_DEBUG, "XK_Kernel - MEM_ALLOCED: 0x%08X size: %u",
                   (UINT32)pTask, size & 0x7fffffff);
    }

    /* Clear memory if requested. */
    if (type == MEM_CALLOC)
    {
        memset(pMem, 0, size & 0x7fffffff);
    }

    /* Return a pointer to the memory we allocated. */
    return pMem;
}

/**
******************************************************************************
**
**  @brief      Free's Memory
**
**              Free's Memory, checks for NULL.
**
**  @param      pMem    - Pointer to pointer to memory to Free
**  @param      file    - File name where memory is freed
**  @param      line    - Line number in file where memory is freed
**
**
**  @return     none
**
******************************************************************************
**/
void XK_Free(void *pMem, const char *file, const UINT32 line)
{
    /* Free the memory */
    FreeDebugWithNullSet(pMem, file, line);

    /* If there are tasks waiting on memory, wake them up. */
    if (xkMallocWaitCount)
    {
        pthread_cond_broadcast(&cpuMemChg);
    }
}


/**
******************************************************************************
**
**  @brief      Initialize mutex.
**
**  @param      mutex   - pointer to MUTEX to initialize
**  @param      file    - File name where mutex is initialized
**  @param      line    - Line number in file where mutex is initialized
**
**  @return     GOOD on success.
**  @return     ERROR on failure.
**
******************************************************************************
**/
INT32 XK_InitMutex(MUTEX *mutex, const char *file, const UINT32 line)
{
    MUTEX       index1;

#ifndef PERF
    /* Note: kernel mutex is 0, so this check kind of works. */
    if (*mutex != 0)
    {
        dprintf(DPRINTF_DEFAULT, "mutex not zero to start with (already initialized?\n");
        abort();
    }
#endif  /* PERF */

    /*
     * We are able to have XK_MUTEX_MAX mutex's in the system. Look for a free
     * slot and then initialize the pthread mutex. Set the MUTEX to point to
     * the slot that references the pthread mutex. If we are unable to find a
     * free slot, we have over allocated our mutex's... flag the error.
     */
    for (index1 = 0; index1 < XK_MUTEX_MAX; ++index1)
    {
        /* If this slot is not used, set up the mutex here. */
        if (xkMutexList[index1].used == FALSE)
        {
            /* Initialize the underlying system mutex. */
            if (XK_InitializeMutex(index1, file, line) == GOOD)
            {
                *mutex = index1;
                return GOOD;
            }

            /*
             * If it fails, there is something wrong with
             * the system.  Flag the error.
             */
            *mutex = XK_MUTEX_MAX;
            return ERROR;
        }
    }

    /* Too many mutexes. */
    *mutex = XK_MUTEX_MAX;

    return ERROR;
}


/**
******************************************************************************
**
**  @brief      Lock a MUTEX.
**
**  @param      mutex       - pointer to MUTEX to lock
**  @param      waitFlag    - mutex wait flag (MUTEX_WAIT to wait for mutex)
**  @param      file        - File name where lock initiated
**  @param      line        - Line number in file where lock initiated
**
**  @return     1 on success.
**  @return     0 on failure.
**
******************************************************************************
**/
INT32 XK_LockMutex(MUTEX *mutex, INT32 waitFlag, const char *file, const UINT32 line)
{

    /* Give it one shot to start with to see if we can acquire the lock. */
    if (XK_TryKMutex(*mutex) != GOOD)
    {
        /* We couldn't get the lock. */
        if (xkMutexList[*mutex].lock_xk_pcb == XK_pcb &&
            xkMutexList[*mutex].lock_func == XK_pcb->functionPtr)
        {
            dprintf(DPRINTF_DEFAULT, "Nested mutex %d, locked by ourselves -- won't work!\n", *mutex);
            abort();                /* Nested lock by ourselves -- won't work! */
        }

        /* See whether we should wait for the MUTEX to come available. */
        if (waitFlag == MUTEX_WAIT)
        {
            /* Set up our blocking call. */
            XK_PreBlockingCall(PCB_NOT_READY);

            /* Get our current Frame pointer. */
            save_regs(XK_GetCurrentTask());

            /* Lock with wait the user mutex. */
            XK_LockKMutex(*mutex);

            /* Set up our blocking call. */
            XK_PostBlockingCall(PCB_READY);
        }
        else
        {
            return 0;               /* Failure */
        }
    }
    strncpy(xkMutexList[*mutex].lock_file, file, 128);
    xkMutexList[*mutex].lock_line = line;
    xkMutexList[*mutex].lock_xk_pcb = XK_pcb;
    xkMutexList[*mutex].lock_func = XK_pcb->functionPtr;

    return 1;                       /* Success */
}

/**
******************************************************************************
**
**  @brief      Do a select() system call.
**
**  @param      mutex       - pointer to MUTEX to lock
**  @param      waitFlag    - mutex wait flag (MUTEX_WAIT to wait for mutex)
**
**  @return     1 on success.
**  @return     0 on failure.
**
******************************************************************************
**/
#ifdef select
#undef select
#endif  /* select */
int XK_Select(int maxSock, fd_set *pRead, fd_set *pWrite, fd_set *pException,
              struct timeval *pTimeOut)
{
    int         rc;

    /* Set up our blocking call. */
    XK_PreBlockingCall(PCB_NOT_READY);

    /* Get our current Frame pointer. */
    save_regs(XK_GetCurrentTask());

    /* Go onto the select */
    rc = select(maxSock, pRead, pWrite, pException, pTimeOut);

    /* Set up our blocking call. */
    XK_PostBlockingCall(PCB_READY);

    return rc;
}


/**
******************************************************************************
**
**  @brief      Close a Descriptors.
**
**              Close a Descriptors.
**
**  @param      type        - Type of descriptor.
**  @param      closeFile   - Descriptor to close.
**
**  @return     none.
**
******************************************************************************
**/
#ifdef close
#undef close
#endif  /* close */
#ifdef fclose
#undef fclose
#endif  /* fclose */
INT32 XK_CloseDescriptor(UINT32 type, void *closeFile)
{
    INT32       rc = 0;
    time_t      t1 = 0;
    time_t      t2 = 0;

    /* Don't hang on crash. */
    if (GetProcessorState() == IDR_PROCESSOR_STATE_NORMAL)
    {
        /* Set up our blocking call. */
        XK_PreBlockingCall(PCB_NOT_READY);

        /* Get our current Frame pointer. */
        save_regs(XK_GetCurrentTask());

        /* Get Start time. */
        t1 = XK_KernelGetTime();

        /* Close the descriptor */
        switch (type)
        {
            case XK_CLOSE_FD:
                if (close((INT32)closeFile) != 0)
                {
                    dprintf(DPRINTF_DEFAULT, "XK_CloseDescriptor - close failed fd %d errno (%d - %s)\n",
                            (INT32)closeFile, errno, strerror(errno));
                }
                break;

            case XK_CLOSE_FILE:
                if (closeFile)
                {
                    if (fclose((FILE *)closeFile) != 0)
                    {
                        dprintf(DPRINTF_DEFAULT, "XK_CloseDescriptor - fclose failed FILE %p errno (%d - %s)\n",
                                (FILE *)closeFile, errno, strerror(errno));
                    }
                }
                else
                {
                    dprintf(DPRINTF_DEFAULT, "XK_CloseDescriptor - XK_CLOSE_FILE NULL FILE pointer\n");
                }
                break;

            default:
                dprintf(DPRINTF_DEFAULT, "XK_CloseDescriptor - Invalid type (%u)\n", type);
                break;
        }

        /* Get End time. */
        t2 = XK_KernelGetTime();

        /* Check time (2 seconds). */
        if ((t2 - t1) > 2)
        {
            dprintf(DPRINTF_DEFAULT, "XK_CloseDescriptor - closing type %u, descriptor %p Took %u seconds\n",
                    type, closeFile, (UINT32)(t2 - t1));
        }

        /* Set up our blocking call.  */
        XK_PostBlockingCall(PCB_READY);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      Do an accept() system call.
**
**  @param      mutex       - pointer to MUTEX to lock
**  @param      waitFlag    - mutex wait flag (MUTEX_WAIT to wait for mutex)
**
**  @return     1 on success.
**  @return     0 on failure.
**
******************************************************************************
**/
#ifdef accept
#undef accept
#endif  /* accept */
INT32 XK_Accept(INT32 sockFd, struct sockaddr *pPeerAddr, size_t *pAddrLen)
{
    INT32       rc;

    /* Set up our blocking call. */
    XK_PreBlockingCall(PCB_NOT_READY);

    /* Get our current Frame pointer. */
    save_regs(XK_GetCurrentTask());

    /* Go onto the accept */
    rc = accept(sockFd, pPeerAddr, pAddrLen);

    /* Set up our blocking call. */
    XK_PostBlockingCall(PCB_READY);

    return rc;
}


/**
******************************************************************************
**
**  @brief      Unlock a MUTEX.
**
**  @param      mutex       - pointer to MUTEX to unlock
**
**  @return     none.
**
******************************************************************************
**/
void XK_UnlockMutex(MUTEX *mutex)
{
    xkMutexList[*mutex].lock_xk_pcb = 0;
    xkMutexList[*mutex].lock_func = 0;
    /* Unlock the mutex */
    XK_UnlockKMutex(*mutex);
}


/**
******************************************************************************
**
**  @brief      Get a pointer to the current tasks pcb.
**
**  @param      none    - use if there are no params
**
**  @return     pointer to the current tasks pcb
**
******************************************************************************
**/
PCB        *XK_GetPcb(void)
{
    /* Return the pointer to the current tasks pcb. */
    return (PCB *)XK_GetCurrentTask();
}


/**
******************************************************************************
**
**  @brief      XK_GetStack - Get stack pointer.
**
**              Get a pointer to the given tasks stack.
**
**  @param      pcb - Pointer to PCB of task stack to get.
**
**  @return     Pointer to the tasks stack.
**
******************************************************************************
**/
UINT8      *XK_GetStack(PCB *pcb)
{
    XK_PCB     *xpcb = (XK_PCB *)pcb;

    return xpcb->stack;         /* Return the pointer to the tasks stack */
}


/**
******************************************************************************
**
**  @brief      XK_GetStackSize - Get stack size.
**
**              Get size of a tasks stack.
**
**  @param      pcb - Pointer to PCB of task stack size to get.
**
**  @return     Size of the tasks stack.
**
******************************************************************************
**/
int XK_GetStackSize(UNUSED PCB *pcb)
{
    return XK_KERNEL_STACK_SIZE;        /* Return the size of the tasks stack */
}


/**
******************************************************************************
**
**  @brief      Issue a system() "like" call.
**
**  @param      command - the command string to execute
**
**  @return     rc of called program.
**
******************************************************************************
**/

/*
** Write "n" bytes to a descriptor.
*/
#if defined(MODEL_3000) || defined(MODEL_7400)
static
#endif /* MODEL_3000 || MODEL_7400 */
INT32 writen(INT32 fd, char *ptr, INT32 nbytes)
{
    INT32       nleft,
                nwritten;

    nleft = nbytes;
    while (nleft > 0)
    {
        nwritten = write(fd, ptr, nleft);
        if (nwritten <= 0)
        {
            return (-1);        /* error */
        }

        nleft -= nwritten;
        ptr += nwritten;
    }
    return (0);
}


INT32 XK_System(const char *pCommand)
{
    INT32       sockfd,
                servlen;
    struct sockaddr_un serv_addr;
    INT32       length;
    INT32       rc;
    UINT8       sysRC;
    INT32       selectRC = 0;
    fd_set      readSet;        /* Uninitialized */

    /*
     * Fill in the structure "serv_addr" with the address of the
     * server that we want to send to.
     */
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, SYSTEM_SRV_SOCKET_PATH);
    servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

    /* Open a socket (an UNIX domain stream socket). */
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        LogMessage(LOG_TYPE_DEBUG, "%s: can't open stream socket, errno %d (%s)",
                   __func__, errno, pCommand);
        return ERROR;
    }

    do
    {
        /* Connect to the server. */
        if (connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0)
        {
            LogMessage(LOG_TYPE_DEBUG, "%s: can't connect to server, errno %d (%s)",
                       __func__, errno, pCommand);
            rc = ERROR;
            break;
        }

        /* Get command length */
        length = strlen(pCommand) + 1;

        /* Send it across, followed by the command */
        /* dprintf(DPRINTF_DEFAULT, "%s: sending length: %d\n", __func__, length); */
        rc = writen(sockfd, (char *)&length, 4);
        if (rc < 0)
        {
            LogMessage(LOG_TYPE_DEBUG, "%s: write error 1, errno %d (%s)", __func__, errno, pCommand);
            rc = ERROR;
            break;
        }

        /* dprintf(DPRINTF_DEFAULT, "%s: sending command: %s\n", __func__, pCommand); */
        rc = writen(sockfd, (char *)pCommand, length);
        if (rc < 0)
        {
            LogMessage(LOG_TYPE_DEBUG, "%s: write error 2, errno %d (%s)", __func__, errno, pCommand);
            rc = ERROR;
            break;
        }

        /* Set up the select and timeout period */
        FD_ZERO(&readSet);
        FD_SET(sockfd, &readSet);

        /* Wait for the select() to fire - blocking */
        selectRC = Select(sockfd + 1, &readSet, NULL, NULL, NULL);
        if (selectRC <= 0)
        {
            dprintf(DPRINTF_DEFAULT, "%s: select failed %u (%s)\n", __func__,
                    (UINT32)selectRC, pCommand);
            rc = ERROR;
            break;
        }

        rc = read(sockfd, &sysRC, 1);
        if (rc < 0)
        {
            LogMessage(LOG_TYPE_DEBUG, "%s: read error, errno %d (%s)", __func__, errno, pCommand);
            break;
        }

        /* dprintf(DPRINTF_DEFAULT, "%s: system() returned %u\n", __func__, (UINT32)sysRC); */
        rc = (INT32)sysRC;
    } while (0);

    if (sockfd >= 0)
    {
        Close(sockfd);
    }
    return rc;
}


/**
******************************************************************************
**
**  @brief      Readies the task to chack the sleeping tasks.
**
**              Readies the task to chack the sleeping tasks.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static volatile UINT64 xkSleeperCurrentTime = 0;
static volatile UINT64 xkSleeperLastTime = 0;
void XK_CheckSleepers(void)
{
    UINT64      tmCntr;

    /* Should be called every 125ms check for every 500ms */
    xkSleeperCurrentTime = get_tsc();
    if (xkSleeperLastTime &&
        ((xkSleeperCurrentTime - xkSleeperLastTime) > (TSC_ONE_SEC_DELAY / 2)))
    {
        dprintf(DPRINTF_DEFAULT, "XK_CheckSleepers: Pre sem_post (%llu msec)\n",
                ((xkSleeperCurrentTime - xkSleeperLastTime) / TSC_ONE_MSEC_DELAY));
    }

    if (xkSleeperLastTime)
    {
        tmCntr = ((xkSleeperCurrentTime - xkSleeperLastTime) / TSC_ONE_MSEC_DELAY);
    }
    else
    {
        tmCntr = 125;
    }

    xkSleeperLastTime = xkSleeperCurrentTime;

    do
    {
        sem_post(&xkWakeTasksSem);
        if (tmCntr >= 125)
        {
            tmCntr -= 125;
        }
    } while ((tmCntr / 125) > 0);

    /* Check sys_call time */
    xkSleeperCurrentTime = get_tsc();
    if (xkSleeperLastTime &&
        ((xkSleeperCurrentTime - xkSleeperLastTime) > (TSC_ONE_SEC_DELAY / 2)))
    {
        dprintf(DPRINTF_DEFAULT, "XK_CheckSleepers: Post sem_post (%llu msec)\n",
                ((xkSleeperCurrentTime - xkSleeperLastTime) / TSC_ONE_MSEC_DELAY));
    }
}

/*
******************************************************************************
**              PRIVATE FUNCTIONS (PROTOTYPES ABOVE)                        **
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Enqueue a task to a queue
**
**              Enqueue a task to a queue
**
**  @param      taskQueue   - queue where to place the task
**  @param      task        - task to place on queue
**
**  @return     none
**
******************************************************************************
**/
static void XK_TaskEnqueue(XK_TASK_QUEUE *taskQueue, XK_PCB *task)
{
    /* Check that we have valid input. */
    if ((taskQueue == NULL) || (task == NULL))
    {
        XK_KernelError("XK_TaskEnqueue - Invalid Parameters");
    }
    /* We have valid input. */
    else
    {
        /*
         * If this is the first item in the queue,
         * set the queue ptr to point to the task
         * and the tasks prev pointer to point to
         * itself.
         */
        if (taskQueue->taskCount == 0)
        {
            taskQueue->tskPtr = task;
            task->prev = task;
        }

        /* Increment the queues task count. */
        ++taskQueue->taskCount;

        /* Set the tasks next ptr to point to the first item in the queue. */
        task->next = taskQueue->tskPtr;

        /* Set the tasks prev ptr to point to the last item in the queue. */
        task->prev = taskQueue->tskPtr->prev;

        /* Set the first tasks prev ptr to point to the inserted task. */
        task->next->prev = task;

        /* Set the last tasks next ptr to point to the inserted task. */
        task->prev->next = task;

        /* Set the CCB PCB pointer correctly. */
        task->pcb.pc_thd = (PCB *)taskQueue->tskPtr;
        task->prev->pcb.pc_thd = (PCB *)task;
    }
#ifdef KERNEL_DEBUG
    dprintf(DPRINTF_DEFAULT, "XK_TaskEnqueue q: 0x%08X, task: 0x%08X, count: %d\n",
            (UINT32)taskQueue, (UINT32)task, taskQueue->taskCount);
#endif  /* KERNEL_DEBUG */
}


/**
******************************************************************************
**
**  @brief      Dequeue a task from a queue
**
**              Dequeue the first task from a queue
**
**  @param      taskQueue   - queue from where to remove the task
**
**  @return     pointer to the task removed on success.
**  @return     NULL on failure.
**
******************************************************************************
**/
static XK_PCB *XK_TaskDequeue(XK_TASK_QUEUE *taskQueue)
{
    XK_PCB     *task = NULL;

    /* Check that we have valid input. */
    if ((taskQueue != NULL) && (taskQueue->taskCount > 0))
    {
        /* Get the pointer to the first item in the queue. */
        task = taskQueue->tskPtr;

        /* Decrement the queues task count. */
        --taskQueue->taskCount;

        /* If there are still tasks in the queue, fix up the pointers. */
        if (taskQueue->taskCount > 0)
        {
            /*
             * Set the new first tasks prev ptr to
             * point the dequeued tasks prev.
             */
            task->next->prev = task->prev;

            /*
             * Set the last tasks next ptr to
             * point the dequeued tasks next.
             */
            task->prev->next = task->next;

            /*
             * Set the queues first task pointer to
             * point to the new first task.
             */
            taskQueue->tskPtr = task->next;

            /* Set the CCB PCB pointer correctly. */
            task->pcb.pc_thd = NULL;
            task->prev->pcb.pc_thd = (PCB *)task->next;
        }
        /*
         * If there are not tasks in the queue,
         * set the queues first task pointer to NULL.
         */
        else
        {
            taskQueue->tskPtr = NULL;
        }
    }
    else
    {
        LogMessage(LOG_TYPE_DEBUG, "XK_TaskDequeue - Queue 0x%08X is empty",
                   (UINT32)taskQueue);
    }

#ifdef KERNEL_DEBUG
    dprintf(DPRINTF_DEFAULT, "XK_TaskDequeue q: 0x%08X, task: 0x%08X, count: %d\n",
            (UINT32)taskQueue, (UINT32)task, taskQueue->taskCount);
#endif  /* KERNEL_DEBUG */

    /* Return pointer to the task removed. */
    return task;
}


/**
******************************************************************************
**
**  @brief      Dequeue a task from a queue
**
**              Dequeue a task from a queue
**
**  @param      taskQueue   - queue from where to dequeue the task
**  @param      task        - task to remove from the queue
**
**  @return     pointer to the task removed on success.
**  @return     NULL on failure.
**
******************************************************************************
**/
static XK_PCB *XK_TaskDequeueTask(XK_TASK_QUEUE *taskQueue, XK_PCB *task)
{
    XK_PCB     *rc = NULL;

    /* Check that we have valid input. */
    if ((taskQueue != NULL) && (task != NULL) && (taskQueue->taskCount > 0))
    {
        /* Get the pointer to the first item in the queue. */
        rc = taskQueue->tskPtr;

        /*
         * Iterate through the queue and
         * look for the task to be removed.
         */
        while (TRUE)
        {
            /* We found the task we are looking for.  break out of the loop. */
            if (task == rc)
            {
                break;
            }
            /*
             * We did not find the task we are looking for,
             * get the pointer to the next item in the queue.
             */
            else
            {
                rc = rc->next;

                /*
                 * If the next item in the queue is the same
                 * as the queues pointer, we have wrapped the
                 * queue.  Set the task pointer to NULL,
                 * indicating our failure to find it.
                 * break out of the loop.
                 */
                if (rc == taskQueue->tskPtr)
                {
                    rc = NULL;
                    break;
                }
            }
        }

        /* If we found the task in the queue, carry on. */
        if (rc)
        {
            /* Decrement the queues task count. */
            --taskQueue->taskCount;

            /*
             * If there are still tasks in the queue, fix up the pointers.
             */
            if (taskQueue->taskCount > 0)
            {
                /*
                 * Set the new first tasks prev ptr to
                 * point the dequeued tasks prev.
                 */
                task->next->prev = task->prev;

                /*
                 * Set the last tasks next ptr to
                 * point the dequeued tasks next.
                 */
                task->prev->next = task->next;

                /*
                 * If the task removed is the first item in the
                 * queue, Set the queues first task pointer to
                 * point to the prev entry of the current first
                 * entry (Make it point to the last item in the
                 * queue).  We do this, so if the scheduler uses
                 * this queue, it will first go to the next item
                 * in the queue, and then schedule it.  This would
                 * be the next item in the queue in respect to the
                 * task being removed now.  So essentially the next
                 * task in the queue will be the next task scheduled
                 * to run.
                 */
                if (task == taskQueue->tskPtr)
                {
                    taskQueue->tskPtr = task->prev;
                }

                /* Set the CCB PCB pointer correctly. */
                task->pcb.pc_thd = NULL;
                task->prev->pcb.pc_thd = (PCB *)task->next;
            }
            /*
             * If there are not tasks in the queue,
             * set the queues first task pointer to NULL.
             */
            else
            {
                taskQueue->tskPtr = NULL;
            }
        }
        /* We did not find the task in the queue. */
        else
        {
            LogMessage(LOG_TYPE_DEBUG, "XK_TaskDequeueTask - Task 0x%08X is not in queue 0x%08X",
                       (UINT32)task, (UINT32)taskQueue);
        }
    }
    /* We did not find the task in the queue (Queue was empty). */
    else
    {
        LogMessage(LOG_TYPE_DEBUG, "XK_TaskDequeueTask - Queue 0x%08X is empty",
                   (UINT32)taskQueue);
    }

#ifdef KERNEL_DEBUG
    dprintf(DPRINTF_DEFAULT, "XK_TaskDequeueTask q: 0x%08X, task: 0x%08X, count: %d\n",
            (UINT32)taskQueue, (UINT32)task, taskQueue->taskCount);
#endif  /* KERNEL_DEBUG */

    /* Return pointer to the task removed. */
    return rc;
}

/**
******************************************************************************
**
**  @brief      Initialize a pthread mutex.
**
**  @param      mutex   - MUTEX to initialize
**  @param      file    - File name where mutex is initialized
**  @param      line    - Line number in file where mutex is initialized
**
**  @return     0 on success (pthread documentation).
**
**  @attention  No boundary or error checking happens here, use at own risk.
**
******************************************************************************
**/
static INT32 XK_InitializeMutex(MUTEX mutex, const char *file, const UINT32 line)
{
    /* Set the mutex to used. */
    xkMutexList[mutex].used = TRUE;
    strncpy(xkMutexList[mutex].init_file, file, 128);
    xkMutexList[mutex].init_line = line;
    memset(xkMutexList[mutex].lock_file, 0, 128);
    xkMutexList[mutex].lock_line = 0;

    /* Initialize the pthread mutex and return this call to the caller. */
    return pthread_mutex_init(&xkMutexList[mutex].mutex, &xkMutexAttr);
}

/**
******************************************************************************
**
**  @brief      Pre blocking call setup
**
**              Call this with the state you want to set the current
**              task to, before you have done a blocking call.
**
**  @param      state   - state to set the calling task to
**
**  @return     none
**
**  @attention  You MUST hold the kernel mutex to call this function.
**
******************************************************************************
**/
static void XK_PreBlockingCall(UINT8 state)
{
    XK_PCB     *pTask = XK_GetCurrentTask();

    /*
     * We need to set our own task state to state.
     * This will ensure we will not be scheduled to run.
     */
    TaskSetState((PCB *)pTask, state);

    /* Check timing. */
    xkTaskCurrentTime = get_tsc();
    if (xkTaskLastTime &&
        ((xkTaskCurrentTime - xkTaskLastTime) > XK_KERNEL_TASK_MAX_LIMIT))
    {
        dprintf(DPRINTF_DEFAULT, "XK_PreBlockingCall: Task took too long (%llu msec)\n",
                ((xkTaskCurrentTime - xkTaskLastTime) / TSC_ONE_MSEC_DELAY));
        XK_KernelPrintStack();
#ifdef M4_ABORT
abort();
#endif  /* M4_ABORT */
    }
    xkTaskLastTime = xkTaskCurrentTime;

    /* Schedule the next task to run. */
    XK_Schedule();
    /* XK_TaskSwitch(__FILE__, __LINE__, __func__); */

    /*
     * We now need to unlock the kernel mutex so while we wait
     * for this user mutex, other tasks can run.
     */
    xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb = 0;
    xkMutexList[XK_KERNEL_ACCESS].lock_func = 0;
    XK_UnlockKMutex(XK_KERNEL_ACCESS);
}

/**
******************************************************************************
**
**  @brief      Post blocking call setup
**
**              Call this with the state you want to set the current
**              task to, after you are done with a blocking call.
**
**  @param      state   - state to set the calling task to
**
**  @return     none
**
**  @attention  You MUST NOT hold the kernel mutex to call this function.
**
******************************************************************************
**/
static void XK_PostBlockingCall(UINT8 state)
{
    XK_PCB     *pTask = XK_GetCurrentTask();

    /* The accept returned so reacquire the kernel mutex. */
    XK_LockKMutex(XK_KERNEL_ACCESS);
    strncpy(xkMutexList[XK_KERNEL_ACCESS].lock_file, __FILE__, 128);
    xkMutexList[XK_KERNEL_ACCESS].lock_line = __LINE__;
    xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb = pTask;
    if (xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb != 0)
    {
        xkMutexList[XK_KERNEL_ACCESS].lock_func = xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb->functionPtr;
    }
    else
    {
        xkMutexList[XK_KERNEL_ACCESS].lock_func = (void *)&__func__;
    }

    /* Set our task state back to state. */
    TaskSetState((PCB *)pTask, state);

    /* TaskSwitch */
#ifdef M4_ABORT
    XK_TaskSwitch(__FILE__, __LINE__, __func__);
#else   /* M4_ABORT */
    XK_TaskSwitch();
#endif  /* M4_ABORT */
}

/**
******************************************************************************
**
**  @brief      Encapsulates our creation of tasks.
**
**  @param      parms   - This is a pointer to this particular tasks pcb.
**
**  @return     none
**
**  @attention  This is only called by the kernel initialization.  This is
**              our basic thread task.  When the pool is created, it will
**              consist of pthreads running this routine.  This wa we will
**              be able to signal already existing pthreads when a task is
**              created, instead of going through the process of creating
**              a new pthread for every task create.  When a new task runs
**              and is finished it will simply wait to be signalled again
**              instead of exiting.
**
******************************************************************************
**/
static NORETURN void XK_Task(void *parms)
{
    UINT32      stackTestDummy = 0;
    XK_PCB     *pTask = (XK_PCB *)parms;
    sigset_t    mask;

#ifdef KERNEL_DEBUG
    dprintf(DPRINTF_DEFAULT, "XK_Task - 0x%08X Enter\n", (UINT32)pTask);
    dprintf(DPRINTF_DEFAULT, "Stack start = 0x%08X, Stack end = 0x%08X, Stack variable = 0x%08X\n",
            (UINT32)pTask->stack, (UINT32)((UINT32)pTask->stack + XK_KERNEL_STACK_SIZE),
            (UINT32)&stackTestDummy);
#endif  /* KERNEL_DEBUG */

    if (((UINT32)&stackTestDummy < (UINT32)pTask->stack) ||
        ((UINT32)&stackTestDummy >= (UINT32)((UINT32)pTask->stack +
                                             XK_KERNEL_STACK_SIZE)))
    {
        XK_KernelError
            ("Stack start = 0x%08X, Stack end = 0x%08X, Stack variable = 0x%08X\n",
             (UINT32)pTask->stack, (UINT32)((UINT32)pTask->stack + XK_KERNEL_STACK_SIZE),
             (UINT32)&stackTestDummy);
    }

    /*
     * We have just been created in the kernel initialization.
     * Give up control to let the kernel finish initializing.
     */
    sched_yield();

    /* Get our current Frame pointer. */
    pTask->pcb.pc_sf0 = pTask->pcb.pc_pfp = (SF *)get_ebp();

#ifdef KERNEL_DEBUG
    dprintf(DPRINTF_DEFAULT, "XK_Task - 0x%08X Got Frame Pointer 0x%08X\n",
            (UINT32)pTask, (UINT32)pTask->pcb.pc_sf0);
#endif  /* KERNEL_DEBUG */

    /* Detach ourselves from the current process. */
    pthread_detach(pTask->threadId);

    /* Retrieve the kernel mutex. */
    XK_LockKMutex(XK_KERNEL_ACCESS);
    strncpy(xkMutexList[XK_KERNEL_ACCESS].lock_file, __FILE__, 128);
    xkMutexList[XK_KERNEL_ACCESS].lock_line = __LINE__;
    xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb = XK_GetCurrentTask();
    if (xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb != 0)
    {
        xkMutexList[XK_KERNEL_ACCESS].lock_func = xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb->functionPtr;
    }
    else
    {
        xkMutexList[XK_KERNEL_ACCESS].lock_func = (void *)&__func__;
    }

    /* We need to ignore certain signals sent to the main process. */
    sigemptyset(&mask);

    /* Add to ignore here */
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGALRM);
    /* sigaddset(&mask, SIGINT); */

    /* Set to ignore */
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    /*
     * Set our thread specific data to point to our pcb.
     * This is how we know who is running at any given
     * point in time throughout the usage of this kernel.
     */
    if (pthread_setspecific(threadKeys, (void *)pTask))
    {
        XK_KernelError("XK_Task - pthread_setspecific");
    }

    /* Say we are ready to go. */
    ++xkTasksStarted;

    /* Enter the main task loop.  We will never break this loop. */
    while (1)
    {
#ifdef KERNEL_DEBUG
        dprintf(DPRINTF_DEFAULT, "XK_Task - 0x%08X Waiting for signal\n", (UINT32)pTask);
#endif  /* KERNEL_DEBUG */

        /*
         * Suspend this task unconditionally.  When the scheduler is ready to
         * run us, it will signal us and we will pick it up from here.
         */
        XK_TaskYield(pTask);

        update_K_xpcb_etc();

#ifdef KERNEL_DEBUG
        dprintf(DPRINTF_DEFAULT, "XK_Task - 0x%08X - %d Signaled\n", (UINT32)pTask,
                (UINT32)pthread_self());
#endif  /* KERNEL_DEBUG */

        /*
         * Make sure that our function pointer is not NULL.
         * This is set up in XK_TaskCreate.
         */
        if (!pTask->functionPtr)
        {
            XK_KernelError("XK_Task - NULL function pointer");
        }

        /* Call the task function and pass in the parameters. */
        pTask->functionPtr(&pTask->parms);

        /*
         * This task is finished.  Set the status to not ready,
         * the function pointer and parm pointer to NULL, and
         * the task name.
         */
        pTask->pcb.pc_stat = PCB_NOT_READY;
        pTask->functionPtr = NULL;
        memset(((UINT8 *)&pTask->parms), 0x00, sizeof(TASK_PARMS));
        strncpy(pTask->pcb.pc_fork_name, "POOL TASK", XIONAME_MAX);

        /* Move this tasks pcb from the run queue to the pool queue. */
        if(pTask->shared)
        {
            XK_TaskMoveQueue(xkRunQueue, xkSharedPoolQueue, pTask);
        }
        else
        {
            XK_TaskMoveQueue(xkRunQueue, xkPoolQueue, pTask);
        }

#ifdef KERNEL_DEBUG
        dprintf(DPRINTF_DEFAULT, "XK_Task - 0x%08X - %d\n",
                (UINT32)pTask, (UINT32)pthread_self());
#endif  /* KERNEL_DEBUG */

        /* Signal the next task to run. */
        XK_Schedule();
    }

    /*
     * If we ever need it we can exit here.  Currently the
     * above loop will never fail, and we will not hit this case.
     */
//    pthread_exit(NULL);
}


/**
******************************************************************************
**
**  @brief      Get a pointer to the current tasks pcb.
**
**              Get a pointer to the current tasks pcb.
**
**  @param      none    - use if there are no params
**
**  @return     pointer to the current tasks pcb
**
******************************************************************************
**/
XK_PCB *XK_GetCurrentTask(void)
{
    /*
     * In function XK_Task, we saved away the pointer to each tasks pcb
     * in their own thread specific data.  We can now pull up the
     * contents of this data to get the pcb pointer to the currently
     * running pthread.
     */
    XK_PCB     *pTask = (XK_PCB *)pthread_getspecific(threadKeys);

    /* If we cannot retrieve this data, we are toast... DIE!. */
    if (pTask == NULL)
    {
        if (xkRunQueue.tskPtr != NULL)
        {
            pTask = xkRunQueue.tskPtr;
        }
        else
        {
//            XK_KernelError("XK_GetCurrentTask Task not found");
            pTask = NULL;
        }
    }

    /* Return the pointer to the current tasks pcb. */
    return pTask;
}


/**
******************************************************************************
**
**  @brief      KernelTask that does kernel scheduling.
**
**              Task that checks for tasks that are sleeping and wakes them up.
**
**  @param      parms   - Unused.
**
**  @return     none.
**
******************************************************************************
**/
#if XK_KERNEL_PROFILE
static volatile UINT64 xkKernelCurrentTime = 0;
static volatile UINT64 xkKernelLastTime = 0;
#endif  /* XK_KERNEL_PROFILE */
static NORETURN void XK_KernelTask(UNUSED void *parms)
{
    sigset_t    mask;
    UINT32      hold_counter;
    XK_PCB     *tmpTask = NULL;
    struct sched_param schedPri;

    /*
     * We have just been created in the kernel initialization.
     * Give up control to let the kernel finish initializing.
     */
    sched_yield();

    /* Detach ourselves from the current process. */
    pthread_detach(pthread_self());
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    /* We need to ignore certain signals sent to the main process. */
    sigemptyset(&mask);

    /* Add to ignore here */
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGALRM);
    /* sigaddset(&mask, SIGINT); */

    /* Set to ignore */
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    /* Set my priority HIGH */
    schedPri.sched_priority = XK_HIGH_PRI_TASK;

/*     pthread_setschedparam(pthread_self(), SCHED_FIFO, &schedPri); */

    while (1)
    {
        /*
         * Wait on the semaphore.  This wiil be signaled.
         * by Timer0_Routine in timer.c.
         */
        sem_wait(&xkKernelTaskSem);

        /*
         * Lock the kernel mutex.
         * We don't want another task to be messing
         * with our pcb pointers.
         */
        XK_LockKMutex(XK_KERNEL_ACCESS);
        strncpy(xkMutexList[XK_KERNEL_ACCESS].lock_file, __FILE__, 128);
        xkMutexList[XK_KERNEL_ACCESS].lock_line = __LINE__;
        xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb = XK_GetCurrentTask();
        if (xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb != 0)
        {
            xkMutexList[XK_KERNEL_ACCESS].lock_func = xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb->functionPtr;
        }
        else
        {
            xkMutexList[XK_KERNEL_ACCESS].lock_func = (void *)&__func__;
        }

#ifdef KERNEL_DEBUG
        dprintf(DPRINTF_DEFAULT, "XK_KernelTask\n");
#endif  /* KERNEL_DEBUG */

        /* Update the task pointer. */
        tmpTask = xkRunQueue.tskPtr;

        /*
         * We need to keep the kernel from going to sleep for now,
         * so loop here if there is nothing to do.  This will change
         * when we get a signal to wake us up
         */
        while (1)
        {
            /*
             * Save the up count value before we start looking for
             * someone to schedule.
             */

            hold_counter = kernel_up_counter;

            /* If there are tasks to schedule, well get to it. */
            if (tmpTask)
            {
                /* Go to the next task in the queue. */
                xkRunQueue.tskPtr = xkRunQueue.tskPtr->next;

                /* If this task is ready to run, break outa here. */
                if (xkRunQueue.tskPtr->pcb.pc_stat == PCB_READY)
                {
                    break;
                }
            }

            /*
             * If we have prcessed all the tasks, give up the mutex,
             * to allow other tasks to grab it that may be waiting on it.
             * sleep for 1ms, and reobtain the mutex.  reacquire the task
             * count.
             */
            if (tmpTask == xkRunQueue.tskPtr)
            {
#if 0
                /* Unlock the kernel mutex to give others a chance */
                xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb = 0;
                xkMutexList[XK_KERNEL_ACCESS].lock_func = 0;
                XK_UnlockKMutex(XK_KERNEL_ACCESS);

                /* Sleep for 10ms (10000us) */
                usleep(10000);

                XK_LockKMutex(XK_KERNEL_ACCESS);

                /* Lock the kernel mutex. */
                strncpy(xkMutexList[XK_KERNEL_ACCESS].lock_file, __FILE__, 128);
                xkMutexList[XK_KERNEL_ACCESS].lock_line = __LINE__;
                xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb = XK_GetCurrentTask();
                if (xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb != 0)
                {
                    xkMutexList[XK_KERNEL_ACCESS].lock_func = xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb->functionPtr;
                }
                else
                {
                    xkMutexList[XK_KERNEL_ACCESS].lock_func = (void *)&__func__;
                }
#else   /* 0 */
                UINT32      pending;
                int         return_code;

                /*
                 * Suspend until there is something to do. Indicate
                 * that we are suspending, then check to see if
                 * the status has changed since we started looking
                 * for a task to schedule. If it has, rather than
                 * suspending, loop again.
                 */
                kernel_sleep = 1;
                FORCE_WRITE_BARRIER;
                if (kernel_up_counter == hold_counter)
                {
                    /* Unlock the kernel mutex to give others a chance */
                    xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb = 0;
                    xkMutexList[XK_KERNEL_ACCESS].lock_func = 0;
                    XK_UnlockKMutex(XK_KERNEL_ACCESS);
#if XK_KERNEL_PROFILE
                    xkKernelLastTime = get_tsc();
#endif  /* XK_KERNEL_PROFILE */

                    return_code = read(CT_xiofd, &pending, sizeof(pending));

#if XK_KERNEL_PROFILE
                    /* Check for every 1500ms */
                    xkKernelCurrentTime = get_tsc();
#ifndef HISTORY_KEEP
#define DELAY_CHECK_TSC (TSC_ONE_SEC_DELAY + (TSC_ONE_SEC_DELAY / 2))
#else   /* HISTORY_KEEP */
#define DELAY_CHECK_TSC (2 * TSC_ONE_SEC_DELAY)
#endif  /* HISTORY_KEEP */
                    if (xkKernelLastTime &&
                        ((xkKernelCurrentTime - xkKernelLastTime) > DELAY_CHECK_TSC))
                    {
                        dprintf(DPRINTF_DEFAULT, "XK_Kernel: CT_xiofd (%llu msec)\n",
                                ((xkKernelCurrentTime - xkKernelLastTime) / TSC_ONE_MSEC_DELAY));
                        XK_KernelPrintStack();
                    }
#endif  /* XK_KERNEL_PROFILE */
                    if (return_code == -1 && errno == EINTR)
                    {
                        /* OK - interrupt received */ ;
                    }
                    else if (return_code != sizeof(pending))
                    {
                        LogMessage(LOG_TYPE_DEBUG, "XIO3D read returned %d, errno %d",
                                   return_code, errno);
                        perror("XIO3D read error");
                    }

                    /* Lock the kernel mutex. */
                    XK_LockKMutex(XK_KERNEL_ACCESS);
                    strncpy(xkMutexList[XK_KERNEL_ACCESS].lock_file, __FILE__, 128);
                    xkMutexList[XK_KERNEL_ACCESS].lock_line = __LINE__;
                    xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb = XK_GetCurrentTask();
                    if (xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb != 0)
                    {
                        xkMutexList[XK_KERNEL_ACCESS].lock_func = xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb->functionPtr;
                    }
                    else
                    {
                        xkMutexList[XK_KERNEL_ACCESS].lock_func = (void *)&__func__;
                    }
                }
                kernel_sleep = 0;
#endif  /* 0 */

                /* Refresh the task pointer. */
                tmpTask = xkRunQueue.tskPtr;
            }
        }

        /*
         * If we scheduled a new task signal it to run.
         *
         * NOTE: The new task will not start until the
         *       kernel mutex is unlocked.
         */
        if (XK_Signal(xkRunQueue.tskPtr) != GOOD)
        {
            XK_KernelError("XK_KernelTask Failed XK_Signal");
        }

        /*
         * Clear out any calls we may have picked up
         * between the sem lock and the kernel lock.
         */
        while (sem_trywait(&xkKernelTaskSem) == GOOD)
        {
        }

        /* Unlock the kernel mutex and give others a chance. */
        xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb = 0;
        xkMutexList[XK_KERNEL_ACCESS].lock_func = 0;
        XK_UnlockKMutex(XK_KERNEL_ACCESS);
    }

    /*
     * If we ever need it we can exit here.  Currently the
     * above loop will never fail, and we will not hit this case.
     */
//    pthread_exit(NULL);
}


/**
******************************************************************************
**
**  @brief      Task that checks for tasks that are sleeping and wakes them up.
**
**              Task that checks for tasks that are sleeping and wakes them up.
**
**  @param      parms   - Unused.
**
**  @return     none.
**
******************************************************************************
**/
static NORETURN void XK_TimerTask(UNUSED void *parms)
{
    XK_PCB     *tmpTask = NULL;
    UINT8       maxCount = 0;
    UINT32      rdyTskCount = 0;
    sigset_t    mask;
    struct sched_param schedPri;

    /*
     * We have just been created in the kernel initialization.
     * Give up control to let the kernel finish initializing.
     */
    sched_yield();

    /* Detach ourselves from the current process. */
    pthread_detach(pthread_self());
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    /* We need to ignore certain signals sent to the main process. */
    sigemptyset(&mask);

    /* Add to ignore here */
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGALRM);
    /* sigaddset(&mask, SIGINT); */

    /* Set to ignore */
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
    dprintf(DPRINTF_DEFAULT, "Executing XK_TimerTask()\n");

    /* Set my priority HIGH */
    schedPri.sched_priority = XK_HIGH_PRI_TASK;

/*     pthread_setschedparam(pthread_self(), SCHED_FIFO, &schedPri); */

    while (1)
    {
        /*
         * Wait on the semaphore.  This wiil be signaled.
         * by Timer0_Routine in timer.c.
         */
        sem_wait(&xkWakeTasksSem);

        /*
         * Reset the max tries while hanging on to the mutex.
         * Currently we will do 5 passes while we have the mutex,
         * in case we get behind.
         */
        maxCount = 0;

        /*
         * Lock the kernel mutex.
         * We don't want another task to be messing
         * with our pcb pointers.
         */
        XK_LockKMutex(XK_KERNEL_ACCESS);
        strncpy(xkMutexList[XK_KERNEL_ACCESS].lock_file, __FILE__, 128);
        xkMutexList[XK_KERNEL_ACCESS].lock_line = __LINE__;
        xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb = XK_GetCurrentTask();
        if (xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb != 0)
        {
            xkMutexList[XK_KERNEL_ACCESS].lock_func = xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb->functionPtr;
        }
        else
        {
            xkMutexList[XK_KERNEL_ACCESS].lock_func = (void *)&__func__;
        }

        /* Reset the ready task count. */
        rdyTskCount = 0;

        /* 5 try loop while having the kernel mutex locked. */
        do
        {
            /* Set the temp pointer to the start of the queue. */
            tmpTask = xkRunQueue.tskPtr;

            /* Make sure there are items on the run queue. */
            if (tmpTask)
            {

                /* Run through all the items in the wait queue. */
                do
                {
                    /*
                     * If the task state is PCB_TIMED_WAIT, decrement the time
                     * and check if it is < 0.  If it is, wake up the task.
                     */
                    if (tmpTask->pcb.pc_stat == PCB_TIMED_WAIT &&
                        (tmpTask->pcb.pc_time -= 125) <= 0)
                    {
                        tmpTask->pcb.pc_time = 0;
#ifdef KERNEL_DEBUG
                        dprintf(DPRINTF_DEFAULT, "XK_TimerTask 0x%08X, setting ready\n",
                                (UINT32)tmpTask);
#endif  /* KERNEL_DEBUG */
                        /* Wake up the task. */
                        TaskSetState((PCB *)tmpTask, PCB_READY);

                        /* Increment the ready task count. */
                        ++rdyTskCount;
                    }

                    /* Go to the next task in the list. */
                    tmpTask = tmpTask->next;

                    /* When we have wrapped around the runqueue we are done. */
                } while (xkRunQueue.tskPtr != tmpTask);

                /* If we readied a task, schedule it. */
                if (rdyTskCount)
                {
                    XK_Schedule();
                }
            }

            /* Increment our count. */
            ++maxCount;

            /*
             * If we have gone through this loop 5 times, we are done.
             * We need to unlock the kernel mutex and let other tasks run.
             * We will also peek in our semaphore to see if we have anthing
             * to process.  If we don't,  exit this loop.
             */
        } while ((maxCount < 5) && (sem_trywait(&xkWakeTasksSem) == GOOD));

        /* Unlock the kernel mutex and give others a chance. */
        xkMutexList[XK_KERNEL_ACCESS].lock_xk_pcb = 0;
        xkMutexList[XK_KERNEL_ACCESS].lock_func = 0;
        XK_UnlockKMutex(XK_KERNEL_ACCESS);
    }

    /*
     * If we ever need it we can exit here.  Currently the
     * above loop will never fail, and we will not hit this case.
     */
//    pthread_exit(NULL);
}


/**
******************************************************************************
**
******************************************************************************
**/
NORETURN
void XK_KernelError(const char* fmt, ...)
{
    char        buf[256];
    char       *bufP = buf;
    va_list     args;

    va_start(args, fmt);
    bufP += vsprintf(bufP, fmt, args);
    va_end(args);

    if (XK_KernelReady() == TRUE)
    {
        LogMessage(LOG_TYPE_DEBUG, "! FATAL KERNEL ERROR !\n  ----> %s", buf);
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "! FATAL KERNEL ERROR !\n  ----> %s\n", buf);
    }

    DeadLoop(deadErrCode(ERR_EXIT_KERNEL), TRUE);
}


/**
******************************************************************************
**
**  @brief      Print Stack Trace
**
**              Print Stack Trace.
**
**  @return     none.
**
******************************************************************************
**/

#define SYS_MAX_BACKTRACE          64  /**< Max Backtrace depth            */
static void XK_KernelPrintStack(void)
{
    void       *array[SYS_MAX_BACKTRACE];
    size_t      size;
    char      **strings;
    size_t      i;
    pthread_t   tmp = pthread_self();

    /* Get the backtrace. */
    size = backtrace(array, SYS_MAX_BACKTRACE);

    dprintf(DPRINTF_DEFAULT, "+++ Call Stack for thread %p - Found %zd stack frames.\n",
            (void *)tmp, size);

    /* Get the backtrace strings. */
    if ((strings = backtrace_symbols(array, size)))
    {
        /* Print the backtrace strings. */
        for (i = 0; i < size; i++)
        {
            dprintf(DPRINTF_DEFAULT, "+++ %s\n", strings[i]);
        }

        /*
         * The strings are allocated in the backtrace_symbols
         * call, so make sure we free the data.
         */
#ifndef DOXYGEN
#undef free
#define free free
#endif  /* DOXYGEN */
        free(strings);
#ifndef DOXYGEN
#undef free
#define free DONT_USE_FREE
#endif  /* DOXYGEN */
    }
}


/**
******************************************************************************
**
**  @brief      Read buffer bytes from a file to readbuf.
**
**  @param      readbuf  - where to store read value.
**  @param      bytes    - number of bytes to read.
**  @param      filename - filename of device to read.
**
**  @return     0 on Success
**  @return     -1 on Failure
**
******************************************************************************
**/
int32_t hw_read_buffer(uint8_t *readbuf, uint32_t bytes, const char *filename)
{
    int32_t     rc;
    int32_t     fd;

    if (!readbuf || !bytes || !filename)
    {
        fprintf(stderr, "%s: invalid parm buf(%p), bytes(%u), fname(%p)\n",
                __func__, readbuf, bytes, filename);
        return -1;
    }

    XK_PreBlockingCall(PCB_NOT_READY);
    save_regs(XK_GetCurrentTask());

    fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        /* fprintf(stderr, "%s: open err (%d - %s)\n",
         * __func__, errno, strerror(errno)); */
        rc = -1;
        goto out;
    }

    rc = read(fd, readbuf, bytes);
    if (rc <= 0)
    {
        if (rc < 0)
        {
            fprintf(stderr, "%s: read err (%d - %s)\n",
                    __func__, (int32_t)errno, (char *)strerror(errno));
        }
        else
        {
            fprintf(stderr, "%s: read err 0 read\n", __func__);
        }
        rc = -1;
    }
    else
    {
        rc = 0;
    }

    close(fd);

  out:
    XK_PostBlockingCall(PCB_READY);

    return rc;
}


/**
******************************************************************************
**
**  @brief      Write buffer bytes to a file from writebuf.
**
**  @param      writebuf - buffer to write.
**  @param      bytes    - number of bytes to write.
**  @param      filename - filename of device to write.
**
**  @return     0 on Success
**  @return     -1 on Failure
**
******************************************************************************
**/
int32_t hw_write_buffer(const uint8_t *writebuf, uint32_t bytes, const char *filename)
{
    int32_t     rc;
    int32_t     fd = 0;

    if (!writebuf || !bytes || !filename)
    {
        fprintf(stderr, "%s: invalid parm buf(%p), bytes(%u), fname(%p)\n",
                __func__, writebuf, bytes, filename);
        return -1;
    }

    XK_PreBlockingCall(PCB_NOT_READY);
    save_regs(XK_GetCurrentTask());

    fd = open(filename, O_WRONLY);
    if (fd < 0)
    {
        /* fprintf(stderr, "%s: open err (%d - %s)\n",
         * __func__, errno, strerror(errno)); */
        rc = -1;
        goto out;
    }

    rc = write(fd, writebuf, bytes);
    if (rc <= 0)
    {
        if (rc < 0)
        {
            fprintf(stderr, "%s: write err (%d - %s)\n",
                    __func__, errno, strerror(errno));
        }
        else
        {
            fprintf(stderr, "%s: write err 0 write\n", __func__);
        }
        rc = -1;
    }
    else
    {
        if ((uint32_t)rc != bytes)
        {
            fprintf(stderr, "%s: wrote %u bytes, should have been %u bytes\n",
                    __func__, rc, bytes);
        }
        rc = 0;
    }

    close(fd);

  out:
    XK_PostBlockingCall(PCB_READY);

    return rc;
}


/**
******************************************************************************
**
**  @brief      Return the function that the task was started on.
**
**  @param      pcb      - Process Control Block to get function from.
**
**  @return     Address of function that task was started on.
**
******************************************************************************
**/
void *print_functionPtr(void *pcb)
{
    return( (void *)((XK_PCB*)pcb)->functionPtr );
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
