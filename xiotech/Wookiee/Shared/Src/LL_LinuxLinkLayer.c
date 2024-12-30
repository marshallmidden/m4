/* $Id: LL_LinuxLinkLayer.c 157458 2011-08-03 14:09:50Z m4 $ */

/**
******************************************************************************
**
**  @mainpage   Link Layer Specification for Wookiee Environment
**
**  @section    intro Introduction
**
**      The Wookiee Link Layer provides message passing between the different
**      platforms (FE, BE, and CCB). It replaces the Bigfoot assembly language
**      routines found in link960.as.
**
**  @section    struct  Structure
**
**      The Link Layer consists of four tasks. One of these tasks is a startup
**      task, the other three are permanent tasks. The tasks are:
**          - LL_BootInit:  Task to initialize Link Layer. This task exits
**                  after initialization is complete.
**          - LL_InitiatorTask: Task that initiates message passing by
**                  queueing a message to the target platform. There is
**                  one initiator task for each outgoing connection.
**          - LL_TargetTask: Task on the target platform that accepts the
**                  incoming message queued by the initiator task. There is
**                  one target task for each incoming connection.
**          - LL_CompletionTask: Task that accepts the message reply
**                  from the target platform. This task runs on the same
**                  platform as the initiator task. There is one completion
**                  task for each outgoing connection (i.e. each completion
**                  task is paired with an initiator task).
**
**      Messages are passed between the platforms via shared memory
**      areas. No data "transmission" occurs, everything happens as
**      simple memory access to the memory mapped shared memory. There are
**      two message/reply queues in the shared memory for each connection.
**          - LLTargetQueue is the queue that contains the messages sent
**                  from the initiator platform to the target platform. The
**                  LL_InitiatorTask places a message in the queue, and notifies
**                  the LL_TargetTask on the target platform that there is a
**                   message by putting it into the PCB_READY state.
**          - LLCompletionQueue contains the replies that are returned from
**                  the target platform to the initiator platform. The
**                  message processing task puts the reply into the
**                  queue (via calling LL_TargetTaskCompletion) and notifies
**                  the LL_CompletionTask on the initiator platform that there
**                  is a reply by putting it into the PCB_READY state.
**
**
**  @section    interface Interface
**
**      Interface routines are:
**          - LL_TargetTaskCompletion: The completion routine for all received
**                  messages on the target. This sends the message reply back
**                  to the initiator.
**          - LL_Init:  Initialize the link layer
**          - LL_QueueMessageToSend: Que a message to be transmitted to
**                  another platform.
**          - LL_SendPacket: Create a message, and queue it to be transmitted
**                  to another platform.
**          - LL_Errtrap: Send an error message to the CCB.
**
**
**  @section    flow Data Flow
**
**      Data flow is illustrated in the following diagram.
**
**  @image html LinkLayer-flow.jpg
**  @image latex LinkLayer-flow.eps
**
******************************************************************************/


/**
******************************************************************************
**
**  @file       LL_LinuxLinkLayer.c
**
**  @brief      This file implements the link layer for the Wookiee
**              platform running Linux.
**
**      The Link Layer passes messages between the platforms
**      (FE, BE, and CCB) in a DSC. It does this by the following
**      steps:
**          - A message is queued to the Link Layer for transmission. A completion
**                  function is also specified that is called with the message
**                  reply when the reply is available.
**          - Using the message's function code (also called its command code),
**                  the Link Layer determines the destination platform for
**                  the message. This, in turn, identifies which instance of
**                  the Initiator Task is handed the message.
**          - The Initiator Task places the message in the LLTargetQueue for
**                  the appropriate platform. If necessary, it also schedules
**                  the Target Task to run by putting into PCB_READY state.
**          - The Target Task retrieves the incoming message, uses the
**                  function code to determine which task should receive this
**                  message, and then queues the message to that task. If
**                  necessary, it also schedules that task to run by
**                  putting into PCB_READY state.
**          - When the task has processed the message and generated a reply,
**                  it calls LL_TargetTaskCompletion to return the reply to
**                  the initiating platform. The reply is returned by placing it
**                  in the LLCompletionQueue of the correct connection. If
**                  necessary, the Completion Task is scheduled to run by
**                  putting into PCB_READY state.
**          - The Completion Task retrieves the reply, associates it with the
**                  original message, and returns the reply to the completion
**                  function if one was specified when the message was queued.
**          - Finally, after calling the completion function, the Link Layer
**                  cleans up its entries associated with the message.
**
**      Interface routines are:
**          - LL_TargetTaskCompletion: The completion routine for all received
**                  messages on the target. This sends the message reply back
**                  to the initiator.
**          - LL_Init:  Initialize the link layer
**          - LL_QueueMessageToSend: Que a message to be transmitted to
**                  another platform.
**          - LL_SendPacket: Create a message, and que it to be transmitted
**                  to another platform.
**          - LL_Errtrap: Send an error message to the CCB.
**
**      Additional global functions are defined, but they are global only as
**      an artifact of the auto-conversion of code. These functions represent
**      tasks that are "forked". The fork routine is in kernel.as, so when the
**      new task is created, this entails an assembly language file calling
**      a C routine. Therefore these functions must appear in the prototype
**      file - hence their global declarations. Without the auto-convert
**      environment, these functions would be passed to fork as addresses,
**      and wouldn't need to be globalized. The functions affected by this are:
**          - LL_BootInit
**          - LL_InitiatorTask
**          - LL_TargetTask
**          - LL_CompletionTask
**
**      Finally, one other function needs to be globalized for the
**      auto-convert environment. That is because this "completion
**      routine" is called from assembly, so must be declared in the
**      prototype files. This function is:
**          - LL_TargetTaskCompletion: Accept a message reply and queue
**                  it to be returned to the initiating platform (where the
**                  completion task will process it).
**
**  Copyright (c) 2002-2007 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <locale.h>

#include "LL_LinuxLinkLayer.h"
#ifdef PROC_CODE
#include "CT_defines.h"
  #define MallocWC(xx)  s_MallocC(xx, __FILE__, __LINE__)
#else
#define CT_ioctl ioctl
extern int  CT_xiofd;
#endif /* PROC_CODE */

#include "ecodes.h"
#include "LL_Stats.h"
#include "misc.h"
#include "MR_Defs.h"
#include "XIO_Std.h"
#include "xio3d.h"

/*** Following for Geo-RAID **/
#include "globalOptions.h"
#include "XIO_Macros.h"
/***  Geo-RAID **/

#ifdef CCB_RUNTIME_CODE
#include "AsyncQueue.h"
#include "ccb_hw.h"
#include "error_handler.h"
#include "logdef.h"
#include "sgl.h"
#include "mem_pool.h"
#include "xk_kernel.h"
  #define DEBUG_FLT_REC_LINK  FALSE   /**< Link layer */
#else
/* Proc */
#include "system.h"
#include "def.h"
#include "dlmfe.h"
#include "fsys.h"
#include "fr.h"
#include "options.h"
#include "pm.h"
#include "virtual.h"
#include "WC_WRP.h"
#include "mem_pool.h"
#endif /* CCB_RUNTIME_CODE */
#include "queue_control.h"
#include "ilt.h"
#include "vrp.h"

/* The following are for created tasks. */
#ifndef CCB_RUNTIME_CODE
extern void CT_LC_LL_InitiatorTask(int);
extern void CT_LC_LL_CompletionTask(int);
extern void CT_LC_LL_TargetTask(int);
#endif  /* CCB_RUNTIME_CODE */

void LL_Init(void);
UINT32 LL_QueueMessageToSend(ILT *pILT);
void LL_SendPacket(void *pReq, UINT32 reqSize, UINT32 cmdCode,
                          void *pRsp, UINT32 rspLen, void *pCallback,
                          UINT32 param);
void LL_TargetTaskCompletion(UINT32 dummy, struct ILT *pTargILT, ...);
#ifdef PROC_CODE
void LL_Errtrap(void);
#endif  /* PROC_CODE */
void LL_InitiatorTask(void *);
void LL_CompletionTask(void *);
void LL_TargetTask(void *);
static void LL_BootInit(void);

#ifndef DOXYGEN
#ifndef KERNEL_DEBUG_5
#define KERNEL_DEBUG_5  FALSE
#endif

#undef LL_TRACE     /* Define this to generate more complete message trace prints */
#undef LL_TRACE_ALL /* Define this to generate most complete message trace prints */

#ifdef LL_TRACE_ALL
#ifndef LL_TRACE
#define LL_TRACE
#endif /* ! LL_TRACE */
#define FPRINTFALL6(x,y,z,zz,xx,yy) fprintf(x,y,z,zz,xx,yy)
#else /* LL_TRACE_ALL */
#define FPRINTFALL6(x,y,z,zz,xx,yy)
#endif /* LL_TRACE_ALL */

#ifdef LL_TRACE
#define FPRINTF4(x,y,z,zz)         fprintf(x,y,z,zz)
#define FPRINTF5(x,y,z,zz,xx)      fprintf(x,y,z,zz,xx)
#define FPRINTF6(x,y,z,zz,xx,yy)   fprintf(x,y,z,zz,xx,yy)
#define FPRINTF7(x,y,z,zz,xx,yy,aa) fprintf(x,y,z,zz,xx,yy,aa)
#else
#define FPRINTF4(x,y,z,zz)
#define FPRINTF5(x,y,z,zz,xx)
#define FPRINTF6(x,y,z,zz,xx,yy)
#define FPRINTF7(x,y,z,zz,xx,yy,aa)
#endif /* LL_TRACE */
#endif /* DOXYGEN */

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/

#define LARGEST_FUNC_CODE   0x701   /**< Largest legal function code */

/**
 ** @Shared Memory Access.
 **/
/* @{ */
#define FE_SHARED_MEM       ((SHM *)(FE_BASE_ADDR+0x2000))  /**< Pointer into FE
                                                              shared memory */
#define BE_SHARED_MEM       ((SHM *)(BE_BASE_ADDR+0x2000))  /**< Pointer into BE
                                                              shared memory */
#define CCB_SHARED_MEM      ((SHM *)(CCB_BASE_ADDR+0x2000)) /**< Pointer into CCB
                                                              shared memory */
#if KERNEL_DEBUG_5 || defined(LL_TRACE)
#define D5_UNUSED_PARAM
#else
#define D5_UNUSED_PARAM UNUSED
#endif

/* @} */



/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/

#ifndef DOXYGEN
#ifdef PROC_CODE
/* PROC */
#define MALLOC_PERM_CACHED_ZERO(s)      MallocWC ((s) | BIT31)

#define TARGETTASKCOMPLETION            CAWRAP_LL_TargetTaskCompletion

#define TASKCREATEPERM(x,y,z,zz,xx)                                           \
                            {                                                 \
                                void *hold_tp;                                \
                                g5 = (UINT32)z;                               \
                                g6 = (UINT32)zz;                              \
                                hold_tp = TaskCreatePerm2Shared               \
                                        (C_label_referenced_in_i960asm(x), y);\
                                if (xx != NULL) {*((void **)(xx)) = hold_tp;}; \
                            }


#elif defined(CCB_RUNTIME_CODE)
/* CCB */
#define MALLOC_PERM_CACHED_ZERO(s)      MallocSharedWC (s)

#define TARGETTASKCOMPLETION            LL_TargetTaskCompletion

#define TASKCREATEPERM(x,y,z,zz,xx)                                           \
                            {                                                 \
                                static TASK_PARMS tp_##x;                     \
                                void *hold_tp;                                \
                                tp_##x.p1 = (UINT32)z;                        \
                                tp_##x.p2 = (UINT32)zz;                       \
                                hold_tp = TaskCreate2                         \
                                        ((void (*)(TASK_PARMS*))x, &tp_##x);  \
                                if (xx != NULL) {*((void **)(xx)) = hold_tp;}; \
                            }

#define get_tsc()       ({ unsigned long long __scr; \
        __asm__ __volatile__("rdtsc" : "=A" (__scr)); __scr;})

#endif /* PROC_CODE or CCB_RUNTIME_CODE */
#endif /* !DOXYGEN */


/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/

/**
 ** String values corresponding to the different link types. Used for
 ** message formatting.
 */

static const char *pLinkNames[] =
{
    "FE2BE",
    "CCB2FE",
    "CCB2BE",
    "BE2FE",
    "FE2CCB",
    "BE2CCB"
} ;


/*
******************************************************************************
** Private variables
******************************************************************************
*/

/*
** The following two arrays are used to determine the communication link
** for message transmission, and the proper input queue for message
** reception. Both of them are indexed by the message's function code.
** So, when a message is presented to LL_Que, it uses the function code
** as an index into linkID to determine the communication link to send
** send the message. When an incoming message is received, the function
** code is used to index into linkQue to determine which queue (and task)
** needs to process the message.
**
** These arrays are populated in LL_BootInit(). They are sparse, but
** memory is cheap, and this will make accessing the information much
** more efficient!
*/

static LL_POSSIBLE_LINKS linkID[LARGEST_FUNC_CODE + 1];
static void *pLinkQueFunc[LARGEST_FUNC_CODE + 1];

/**
 ** QUEUE_BLOCK structures for the two initiator tasks.
 */

static QUEUE_BLOCK llQb0;
static QUEUE_BLOCK llQb1;

/*
** Pointers into shared memory.
*/
#ifdef FRONTEND
SHM *pMySharedMem = FE_SHARED_MEM;
int LLMyEvent = XIO3D_FE_EVT;       /**< Event to wake the FE platform */
#else
static SHM *pFESharedMem = FE_SHARED_MEM;
#endif /* FRONTEND */

#ifdef BACKEND
SHM *pMySharedMem = BE_SHARED_MEM;
int LLMyEvent = XIO3D_BE_EVT;       /**< Event to wake the BE platform */
#else
static SHM *pBESharedMem = BE_SHARED_MEM;
#endif /* BACKEND */

#ifdef CCB_RUNTIME_CODE
SHM *pMySharedMem = CCB_SHARED_MEM;
#else
static SHM *pCCBSharedMem = CCB_SHARED_MEM;
#endif /* CCB_RUNTIME_CODE */

UINT8 *pStartOfHeap;
UINT32 startOfMySharedMem, endOfMySharedMem;

UINT32 startOfBESharedMem, endOfBESharedMem;

/*
** Synchronization stages of the various startup tasks.
*/

static SYNC_STATE boot_sync;

/*
** Table to hold Link Layer statistics.
*/

static LL_STATS LL_stattbl;

/*
** Message ID count.
*/

static UINT32 llMessageCount;

#ifdef PROC_CODE
/*
** A task that handles call-backs for queue full conditions - modified CQT-13112.
*/
static PCB *LL_QueFullReqHandlingTask_PCB = NULL;
struct ILT *LL_QueFullReqHandlingTask_ILT = NULL;
struct ILT *LL_QueFullReqHandlingTask_ILT_tail = NULL;
#endif  /* PROC_CODE */

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/

/**
 ** Link Layer QUEUE_CONTROL
 */

QUEUE_CONTROL LINK_QCS;

#ifndef DOXYGEN
#ifdef FRONTEND
extern UINT32 C_fego;
#endif /* FRONTEND */
#endif /* DOYGEN */


#if KERNEL_DEBUG_5
long long message_filter_time;
UINT32 kernel_pri[20];
UINT32 kernel_pri_index;
INT32 kernel_sec[20],kernel_usec[20];
char *kernel_name[20];
#endif /* KERNEL_DEBUG_5 */

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/

static void LL_NormalInitiatorProcessing(LLI *pMyLLI, QUEUE_BLOCK *pMyQB);
static void LL_ForwardTheReply (ILT *pILT, LL_POSSIBLE_LINKS myLink);
static void LL_FlushOutstandingMessageList(LLI *pMyLLI);
static void LL_NormalCompletionProcessing(LLI *pMyLLI);
static void LL_NormalTargetProcessing(LLI *pMyLLI);
static void LL_SendPacketCompletion (UINT32 returnCode, ILT *pILT);

#ifdef CCB_RUNTIME_CODE
#define TRAPADDR_OFFSET 0x00010000
static void LL_LogtrapComp(UINT32 returnCode, ILT *pILT);
static void LL_Logtrap( LLI *pMyLLI );
#endif /* CCB_RUNTIME_CODE */


#ifndef DOXYGEN
#ifdef PROC_CODE
extern void error08(void);
extern void error11(void);

extern void CAWRAP_LL_TargetTaskCompletion(void);
extern void KernelDispatch (UINT32 returnCode, ILT *pILT,  MR_PKT *pMRP,
                        UINT32 w0);
#if 1  /*Fix for CQT-13112 */
extern void CT_LC_LL_QueFullReqHandlingTask(int);
void LL_QueFullReqHandlingTask(UINT32);
#endif  /* 1 */
#endif /* PROC_CODE */

extern int change_my_priority(LL_POSSIBLE_LINKS myLink, UINT32 connection);
#endif /* !DOXYGEN */

#ifdef BACKEND
extern void  GR_HandleSpecialMirrorFlags(UINT32 ,VRP *, ILT *);
#endif


/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Provide a common means of dequeueing an item from an
**              interprocess queue.
**
**              This routine will normally be called from a task that is a
**              designated "handler" for a particular queue (a QB within a
**              QCS, see <queue_contro.h>). It is the responsibility of the
**              calling task to ascertain if entries exist on the queue before
**              calling this routine; a NULL address is returned if the queue
**              is empty. If the queue falls below the programmed low water
**              mark any waiting tasks are awakened.
**
**              The count of queued ILTs is adjusted to reflect the ILT being
**              delinked from the chain and the head/tail pointers are updated.
**              The queue full flag is cleared.
**
**  @param      pQCS    - Pointer to QUEUE_CONTROL structure
**              pQB     - Pointer to QUEUE_BLOCK within the QUEUE_CONTROL that
**                        contains the item to dequeue.
**
**  @return     Pointer to ILT that was dequeued.
**
******************************************************************************
**/

static ILT *DeQueueMessage (QUEUE_CONTROL *pQCS, QUEUE_BLOCK *pQB)
{
    ILT             *pILT;

    /*
     * If the queue is empty, return a null pointer.
     */
    pILT = (ILT *)pQB->pqb_First;
    if (pILT == 0)
    {
        return pILT;
    }

    /*
     * Update the chain head, the count of entries in the QB, the total count
     * of entries in the QCB, and the QB flags.
     */
    pQB->pqb_First = pILT->fthd;
    pQB->qb_size--;
    pQCS->qc_numEntries--;
    pQB->qb_flags &= ~QB_FULL;

    /*
     * If the QB count has falled below the low water mark, wake waiting tasks.
     */
    if (pQB->qb_size < pQB->qb_lowWater && (pQB->qb_flags & QB_PBLOCK) != 0)
    {
#ifdef PROC_CODE
        TaskReadyByState(pQB->qb_pstat);
#endif  /* PROC_CODE */
        pQB->qb_flags &= ~QB_PBLOCK;
    }

    /*
     * If the queue is now empty, clear the pointers, set the QB flags
     * to empty, and set the QCS status to show the queue has no entries.
     */
    if (pQB->qb_size == 0)
    {
        pQB->pqb_First = 0;
        pQB->pqb_Last = 0;
        pQB->qb_flags |= QB_EMPTY;
        pQCS->qc_status &= ~(1 << pQB->qb_ord);
    }
    return(pILT);
}   /* End of DeQueueMessage */

/**
******************************************************************************
**
**  @brief      Provide a common means of queueing ILTs between processes.
**
**              The ILT is queued to the indicated QCS/QB and the handler task
**              is awakened if not already awake. This routine can also block
**              the calling task if the queue is full (and the WAIT option is
**              selected). In this case the blocked task is reawakened when
**              the queue size drops below the low water mark previously
**              programmed into the QB.
**
**              The indicated ILT is attempted to be placed into the indicated
**              queue.
**
**              The ILT is queued by linking it to the tail of the linked list
**              already present; if the queue was empty the ILT becomes both
**              the head and tail. In this case the handler task is awakened
**              if it is not flagged as busy in the QCS.
**
**              The count of entries in the associated QCS is updated to show
**              the new entry and the empty flag is cleared.
**
**              If queue wait is indicated and the queue is full the task is
**              suspended until the queue drops below the low water mark as
**              programmed into the QB. Wait status is then indicated in the
**              QCS.
**
**  @param      pQCS     - Pointer to QUEUE_CONTROL structure
**              pILT     - Pointer to ILT structure to add to the queue.
**              queIndex - Index number into the QUEUE_CONTROL
**              wait     - Suspend if queue is full (TRUE/FALSE)
**
**  @return     TRUE     - if the ILT is placed into the queue
**              FALSE    - if the queue was full and we didn't wait
**
**  @attention  This function may suspend the calling task if the queue is
**              full, and the wait option is selected.
**
******************************************************************************
**/

static UINT32 QueueILT(QUEUE_CONTROL *pQCS, ILT *pILT, UINT32 queIndex, UINT32 wait)
{
    QUEUE_BLOCK     *pQB = pQCS->pqc_QB[queIndex];

#ifndef PERF
    if (queIndex >= 7)
    {
        fprintf(stderr, "%s:%s:%d queue index (%d) >= 7\n", __FILE__, __func__, __LINE__, queIndex);
        abort();
    }
    if (pQB == NULL)
    {
        fprintf(stderr, "%s:%s:%d No pqc_QB entry for queue index (%d)\n", __FILE__, __func__, __LINE__, queIndex);
        abort();
    }
#endif  /* !PERF */

    /*
     * Clear forward thread pointer.
     */
    pILT->fthd = NULL;

    /*
     * Check to see if there is room in the queue.
     */
    while ((pQB->qb_flags & QB_FULL) != 0)
    {
        /*
         * The queue is full. If we don't want to wait, then return an error.
         */
        if (wait != TRUE)
        {
            return (FALSE);
        }

        /*
         * Set QUEUE_BLOCK qb_flags to show there are waiting processes.
         */
        pQB->qb_flags |= QB_PBLOCK;

        /*
         * Set our PCB status to show that we are waiting for room in the
         * queue, then suspend.
         */
        TaskSetMyState (pQB->qb_pstat);
        TaskSwitch();
    }

    /*
     * Space is available in the queue. Add the ILT. If the queue is empty,
     * set the head pointer to point to the ILT. If the queue is non-empty,
     * set the last ILT in the queue to point to the ILT (i.e. put the ILT
     * at the end of the queue). Then set the tail pointer to this ILT.
     */
    if (pQB->pqb_Last == NULL)
    {
        pQB->pqb_First = pILT;
    }
    else
    {
        pQB->pqb_Last->fthd = pILT;
    }
    pQB->pqb_Last = pILT;

    /*
     * Update the counts and flags.
     */
    pQB->qb_size++;
    pQCS->qc_numEntries++;
    pQB->qb_flags &= ~QB_EMPTY;
    pQCS->qc_status |= (1 << queIndex);

    /*
     * If the queue is full, set the full indicator
     */
    if (pQB->qb_size >= pQB->qb_max)
    {
        pQB->qb_flags |= QB_FULL;
    }

    /*
     * If the task associated with this queue is not currently processing the
     * queue, set the task to the ready state (so it runs).
     */
    if ((pQCS->qc_flags & (1 << queIndex)) == 0)
    {
        TaskSetState(pQCS->pqc_PCB[queIndex], PCB_READY);
    }
    return(TRUE);
}   /* End of QueueILT */

/**
******************************************************************************
**
**  @brief      Normal operation of outgoing message processing task.
**
**              This routine handles the normal operations of sending
**              messages across its link. It will wait for a message to
**              be queued by LL_QueueMessageToSend, then send it across the
**              link to the waiting Target task.
**
**  @param      pMyLLI  - Pointer to LLI structure for this link.
**  @param      pMyQB   - Pointer to message queue (from LL_QueueMessageToSend)
**
**  @return     none
**
******************************************************************************
**/

static void LL_NormalInitiatorProcessing(LLI *pMyLLI, QUEUE_BLOCK *pMyQB)
{
    UINT8       myBitMask = 1 << pMyLLI->LLInitiatorQCSIndex;
    UINT8       myClearMask = ~myBitMask;
    ILT         *pILT, **ppNextEntry;
    int         return_status;

#if 1  /*Fix for CQT-13112 */
    int        reqsProcessed = 0;
#endif

#if KERNEL_DEBUG_5
    long long   tsc;
    UINT32      tsc_h, tsc_l;
#endif /* KERNEL_DEBUG_5 */

    /*
    ** Loop forever, processing messages. Loss of synchronization will cause
    ** a break out of the loop, returning to LL_InitiatorTask.
    */

    for(;;)
    {
        /*
        ** Go into a not-ready state, and rely on the
        ** queueing task to start up this task.
        */

        TaskSetMyState(PCB_NOT_READY);
        if ((LINK_QCS.qc_status & myBitMask) == 0)
        {
            TaskSwitch();
        }
        else
        {
            TaskSetMyState(PCB_READY);
        }

        /*
        ** Check synchronization.
        */

        if (pMyLLI->LLInitiatorSync != RUNNING)
        {
            FPRINTF5 (stderr, "%sInitiator Task %s resetting %d^^^^^^^^^^\n", L_MsgPrefix, pLinkNames[pMyLLI->LLmyLink], 1);
            break;
        }

        /*
        ** Indicated that we are busy processing the input queue.
        */

        LINK_QCS.qc_flags |= myBitMask;

        /*
        ** Process the message queue.
        */

        while ((LINK_QCS.qc_status & myBitMask) != 0)
        {
            /*
            ** Make sure there is room in the Target Input queue
            */

            ppNextEntry = pMyLLI->pLLTargetQueueInPtr + 1;
            if (ppNextEntry == pMyLLI->pLLTargetQueueEnd)
            {
                ppNextEntry = pMyLLI->pLLTargetQueueStart;
            }
            if (ppNextEntry == pMyLLI->pLLTargetQueueOutPtr)
            {
                /*
                ** The queue is full. Make sure the target task
                ** knows there is something waiting for it.
                */

                if (TaskGetState (pMyLLI->pLLTargetPCB) == PCB_WAIT_LINUX_TARG_IN)
                {
                    TaskSetState(pMyLLI->pLLTargetPCB, PCB_READY);
                    (*pMyLLI->pLLTargetUpCounter)++;
                    FORCE_WRITE_BARRIER;
                    if (*pMyLLI->pLLTargetSleep)
                    {
                        FPRINTFALL6 (stderr, "%s%d: ******************************** Kick the target for link %s with %d\n", L_MsgPrefix, __LINE__, pLinkNames[pMyLLI->LLmyLink], pMyLLI->LLTargetEvent);
                        return_status = CT_ioctl(CT_xiofd, XIO3D_SENDEVT, pMyLLI->LLTargetEvent);
                        if (return_status != 0)
                        {
                            fprintf(stderr, "%s%d: send event ioctl returned %d, errno=%d for link %s\n", L_MsgPrefix,
                                    __LINE__, return_status, errno, pLinkNames[pMyLLI->LLmyLink]);
                            perror ("");
                        }
                    }
                }

                /*
                ** Suspend to allow other tasks
                ** to run while we wait for the Target to free up
                ** some entries. Note that we do not set ourselves
                ** as not ready, so we will return without the need
                ** of interaction with another task.
                */
#if 1  /*Fix for CQT-13112 */
                if(++reqsProcessed >= 3)
                {
                     reqsProcessed = 0;
                     TaskSwitch();
                }
#else
                TaskSwitch();
#endif

                /*
                ** Check synchronization.
                */

                if (pMyLLI->LLInitiatorSync != RUNNING)
                {
                    FPRINTF5 (stderr, "%sInitiator Task %s resetting %d^^^^^^^^^^\n", L_MsgPrefix, pLinkNames[pMyLLI->LLmyLink], 2);
                    return;
                }
                continue;
            }

            /*
            ** Get the message's ILT.
            */

            pILT = DeQueueMessage (&LINK_QCS, pMyQB);

            /*
            ** If no ILT is returned, we are through.
            */

            if (pILT == 0)
            {
                break;
            }

#ifdef LL_TRACE
            {
                VRPCMD              *pVRPCMD;
                VRP                 *pVRP;

                pVRPCMD = (VRPCMD *)&pILT->ilt_normal.w0;
                pVRP = pVRPCMD->pvrVRP;
                FPRINTF6 (stderr,"%sIT: %'14d[0x%x] => %s\n", L_MsgPrefix,
                          pILT[1].linux_val, pVRP->function, pLinkNames[pMyLLI->LLmyLink]);
            }
#endif /* LL_TRACE */

#if KERNEL_DEBUG_5
            tsc = get_tsc() >> 21;
            tsc_h = (UINT32)(tsc >> 32);
            tsc_l = (UINT32)tsc;
            pILT[1].w2 = tsc_h;
            pILT[1].w3 = tsc_l;
            {
                INT32 timediff;
                long long stime, svtime;
                UINT32 kpri_i = kernel_pri_index;

                stime = ((long long)pILT[1].w0 << 32) | ((long long)pILT[1].w1);
                timediff = (INT32)((tsc - stime) & 0x7fffffff);
                if (timediff > 340 && timediff < 2670 && tsc > message_filter_time)
                {
                    int ind;
                    timediff /= 1.335;
                    fprintf (stderr,"\n*********************************%s%d:LL_NormalInitiatorProcessing found excessive time %d\n", L_MsgPrefix,__LINE__,
                             timediff);
                    for (ind = 20; ind > 0; ind--)
                    {
                        svtime = ((long long)kernel_sec[kpri_i] << 32) | ((long long)kernel_usec[kpri_i]);
                        timediff = (INT32)((svtime - stime) & 0x7fffffff) / 1.335;
                        fprintf (stderr,"\t%s%dth oldest schedule: task %s,  pri 0x%x, time before/after message queueing %d\n", L_MsgPrefix,
                                 ind, kernel_name[kpri_i], kernel_pri[kpri_i], timediff);
                        if (++kpri_i >= 20) kpri_i = 0;
                    }
                    message_filter_time = tsc + 2000;
                }
            }
#endif /* KERNEL_DEBUG_5 */

#if DEBUG_FLT_REC_LINK
            MSC_FlightRec (FR_L_EXECO, (UINT32)pILT,
                           (UINT32)(((VRPCMD *)&pILT->ilt_normal.w0)->pvrVRP),
                           (UINT32)pMyLLI->pLLTargetQueueInPtr);
#endif /* DEBUG_FLT_REC_LINK */

            /*
            ** Put the message in the Target Input queue.
            */

            *pMyLLI->pLLTargetQueueInPtr = pILT;
            FORCE_WRITE_BARRIER;
            pMyLLI->pLLTargetQueueInPtr = ppNextEntry;
            FORCE_WRITE_BARRIER;

            /*
            ** Start up the target task, so it can process the message.
            */

            if (TaskGetState (pMyLLI->pLLTargetPCB) == PCB_WAIT_LINUX_TARG_IN)
            {
                TaskSetState(pMyLLI->pLLTargetPCB, PCB_READY);
                (*pMyLLI->pLLTargetUpCounter)++;
                FORCE_WRITE_BARRIER;
                if (*pMyLLI->pLLTargetSleep)
                {
                    FPRINTFALL6 (stderr, "%s%d: ******************************** Kick the target for link %s with %d\n", L_MsgPrefix, __LINE__, pLinkNames[pMyLLI->LLmyLink], pMyLLI->LLTargetEvent);
                    return_status = CT_ioctl(CT_xiofd, XIO3D_SENDEVT, pMyLLI->LLTargetEvent);
                    if (return_status != 0)
                    {
                        fprintf(stderr, "%s%d: send event ioctl returned %d, errno=%d for link %s\n", L_MsgPrefix,
                                __LINE__, return_status, errno, pLinkNames[pMyLLI->LLmyLink]);
                        perror ("");
                    }
                }
            }

            /*
            ** Add the ILT into the outstanding message list. Use the
            ** next nest of the ILT to maintain the pointers.
            */

            pILT++;
            pILT->fthd = pMyLLI->pLLActiveILTHead;
            pMyLLI->pLLActiveILTHead->bthd = pILT;
            pILT->bthd = (ILT *)&pMyLLI->pLLActiveILTHead;
            pMyLLI->pLLActiveILTHead = pILT;

            /*
            ** Suspend, in case there are any higher priority tasks that
            ** want to queue another message to send. Note that we do not
            ** set ourselves as not ready, so we will return without the
            ** need of interaction with another task.
            */

            TaskSwitch();

            /*
            ** Check synchronization.
            */

            if (pMyLLI->LLInitiatorSync != RUNNING)
            {
                FPRINTF5 (stderr, "%sInitiator Task %s resetting %d^^^^^^^^^^\n", L_MsgPrefix, pLinkNames[pMyLLI->LLmyLink], 3);
                return;
            }
        }

        /*
        ** Show we are no longer processing the input queue.
        */

        LINK_QCS.qc_flags &= myClearMask;
    }
}


/**
******************************************************************************
**
**  @brief      Task to process outgoing messages.
**
**              This routine is called as a new task. There is one
**              such task for each initiator (outgoing) link.
**
**              After initialization and synchronization, this task
**              waits to be handed an outgoing message packet. It then
**              "sends" the packet to the destination, and notifies the
**              destination platform that it has a new message.
**
**  @param      ptr - Pointer to task param block (CCB ONLY). The same value
**                  is passed in the g5 register for the FE & BE.
**
**  @return     Never exits.
**
******************************************************************************
**/

NORETURN
void LL_InitiatorTask(UNUSED void *ptr)
{
#ifdef CCB_RUNTIME_CODE
    LL_POSSIBLE_LINKS   myLink = ((TASK_PARMS *)ptr)->p1;
#else
    LL_POSSIBLE_LINKS   myLink = g5;
#endif /* CCB_RUNTIME_CODE */
    LLI                 *pMyLLI = pMySharedMem->pSHMLLIDir[myLink];
    int                 return_status;
#if DEBUG_FLT_REC_LINK
    int                 log_filter = 0;
#endif /* DEBUG_FLT_REC_LINK */

    /* Lower priority */
    change_my_priority (myLink, FALSE);

    /*
    ** Initialize the Completion Response queue.
    */

    pMyLLI->pLLCompletionQueueStart =
        (ILT **)MALLOC_PERM_CACHED_ZERO (VRMAX *
                                         sizeof (*pMyLLI->pLLCompletionQueueStart));
    pMyLLI->pLLCompletionQueueEnd = pMyLLI->pLLCompletionQueueStart + VRMAX;
    pMyLLI->pLLCompletionQueueInPtr = pMyLLI->pLLCompletionQueueStart;
    pMyLLI->pLLCompletionQueueOutPtr = pMyLLI->pLLCompletionQueueInPtr;

    /*
    ** Save our PCB
    */

#ifdef CCB_RUNTIME_CODE
    pMyLLI->pLLInitiatorPCB = XK_GetPcb();
#else   /* CCB_RUNTIME_CODE */
    pMyLLI->pLLInitiatorPCB = K_xpcb;
#endif  /* CCB_RUNTIME_CODE */

    /*
    ** Wait for the boot task to complete its initialization, and for the
    ** target at the other end of the link to initialize.
    */

#if DEBUG_FLT_REC_LINK
    MSC_FlightRec (FR_L_SYNC, (UINT32)pMyLLI->LLTargetSync,
                   (UINT32)pMyLLI->LLInitiatorSync,
                   (UINT32)pMyLLI->LLmyLink);
#endif /* DEBUG_FLT_REC_LINK */

    while (  boot_sync < SYNCHRONIZE
             || pMyLLI->LLTargetSync != READY
             || pMyLLI->LLCompletionSync != SYNCHRONIZE)
    {
        TaskSleepMS (1);
    }

    /*
    ** Loop forever, processing incoming message packets.
    */

    for (;;)
    {
        FPRINTF4 (stderr, "%sInitiator task %s at resync point.\n", L_MsgPrefix, pLinkNames[myLink]);

        /*
        ** Make sure that we are not running at high priority. If we are,
        ** then it is possible for the two high priority tasks
        ** (Front.t & Back.t) to starve everything else
        ** on the Linux box. When this happens, it's power cycle time.
        */

        if (change_my_priority (myLink, FALSE))
        {
            boot_sync = SYNCHRONIZE;
        }

        /*
        ** We come here, too, if we lose synchronization later. Wait for the
        ** other two tasks to come to a stop. This is to get everyone to a
        ** known state.
        */

#ifdef PROC_CODE
        K_ii.status &= ~II_STATUS_LINKPROC;
#endif /* PROC_CODE */
        pMyLLI->LLInitiatorSync = SYNCHRONIZE;

#if DEBUG_FLT_REC_LINK
        if (log_filter == 0)
        {
            MSC_FlightRec (FR_L_SYNC, (UINT32)pMyLLI->LLTargetSync,
                           (UINT32)pMyLLI->LLInitiatorSync,
                           (UINT32)pMyLLI->LLmyLink);
            log_filter = 1;
        }
#endif /* DEBUG_FLT_REC_LINK */

        while ((TaskGetState(pMyLLI->pLLTargetPCB)     != PCB_NOT_READY) ||
               (TaskGetState(pMyLLI->pLLCompletionPCB) != PCB_NOT_READY))
        {
            if(TaskGetState(pMyLLI->pLLTargetPCB) != PCB_NOT_READY)
            {
                TaskSetState(pMyLLI->pLLTargetPCB, PCB_READY);
                (*pMyLLI->pLLTargetUpCounter)++;
                FORCE_WRITE_BARRIER;
                if (*pMyLLI->pLLTargetSleep)
                {
                    FPRINTFALL6 (stderr, "%s%d: ******************************** Kick the target for link %s with %d\n", L_MsgPrefix, __LINE__, pLinkNames[pMyLLI->LLmyLink], pMyLLI->LLTargetEvent);
                    /*
                    ** No error checking is done on this ioctl since the other processor is
                    ** just starting up. An error would not be unexpected.
                    */
                    (void)CT_ioctl(CT_xiofd, XIO3D_SENDEVT, pMyLLI->LLTargetEvent);
                }
            }
            if(TaskGetState(pMyLLI->pLLCompletionPCB) != PCB_NOT_READY)
            {
                TaskSetState(pMyLLI->pLLCompletionPCB, PCB_READY);
            }
            /*
            ** If we have been running, and we are trying to re-initiailze a link
            ** between the CCB and either FE or BE, then we don't need to be
            ** a fanatic about it: we are running fine and the FE & BE are continueing
            ** to handle disk requests. So, suspend the task for a while.
            */

            if (boot_sync == RUNNING)
            {
                TaskSleepMS(100);
            }
            else
            {
                TaskSleepMS(20);
            }
        }
        FPRINTF4 (stderr,
                  "%sInitiator task %s: Target task & Completion task stopped\n", L_MsgPrefix,
                  pLinkNames[myLink]);

        /*
        ** If either other task is not in SYNCHRONIZE state, keep waiting.
        */

        if (pMyLLI->LLTargetSync != SYNCHRONIZE)
        {
            FPRINTF4 (stderr, "%sInitiator task %s, TargetTask not at SYNC\n", L_MsgPrefix,
                      pLinkNames[myLink]);
            TaskSetState(pMyLLI->pLLTargetPCB, PCB_READY);
            (*pMyLLI->pLLTargetUpCounter)++;
            FORCE_WRITE_BARRIER;
            if (*pMyLLI->pLLTargetSleep)
            {
                FPRINTFALL6 (stderr, "%s%d: ******************************** Kick the target for link %s with %d\n", L_MsgPrefix, __LINE__, pLinkNames[pMyLLI->LLmyLink], pMyLLI->LLTargetEvent);
                return_status = CT_ioctl(CT_xiofd, XIO3D_SENDEVT, pMyLLI->LLTargetEvent);
                if (return_status != 0)
                {
                    fprintf(stderr, "%s%d: send event ioctl returned %d, errno=%d for link %s\n", L_MsgPrefix,
                            __LINE__, return_status, errno, pLinkNames[pMyLLI->LLmyLink]);
                    perror ("");
                }
            }
            continue;
        }
        if (pMyLLI->LLCompletionSync != SYNCHRONIZE)
        {
            FPRINTF4 (stderr, "%sInitiator task %s, CompletionTask not at SYNC\n", L_MsgPrefix,
                      pLinkNames[myLink]);
            TaskSetState(pMyLLI->pLLCompletionPCB, PCB_READY);
            continue;
        }

#ifdef FRONTEND
        /*
        ** Frontend only: Set flag to let another task run (it used
        ** to wait for write buffer assignment here).
        */

        C_fego = 1;
#endif /* FRONTEND */


        /*
        ** We are ready to run. Set our indicator, and make the Target Task (on
        ** the other end of the link) and the Completion Task ready to run.
        */

#if DEBUG_FLT_REC_LINK
        MSC_FlightRec (FR_L_SYNC, (UINT32)pMyLLI->LLTargetSync,
                       (UINT32)pMyLLI->LLInitiatorSync,
                       (UINT32)pMyLLI->LLmyLink);
        log_filter = 0;
#endif /* DEBUG_FLT_REC_LINK */

        pMyLLI->LLInitiatorSync = RUNNING;
        TaskSetState(pMyLLI->pLLTargetPCB, PCB_READY);
        (*pMyLLI->pLLTargetUpCounter)++;
        FORCE_WRITE_BARRIER;
        if (*pMyLLI->pLLTargetSleep)
        {
            FPRINTFALL6 (stderr, "%s%d: ******************************** Kick the target for link %s with %d\n", L_MsgPrefix, __LINE__, pLinkNames[pMyLLI->LLmyLink], pMyLLI->LLTargetEvent);
            return_status = CT_ioctl(CT_xiofd, XIO3D_SENDEVT, pMyLLI->LLTargetEvent);
            if (return_status != 0)
            {
                fprintf(stderr, "%s%d: send event ioctl returned %d, errno=%d for link %s\n", L_MsgPrefix,
                        __LINE__, return_status, errno, pLinkNames[pMyLLI->LLmyLink]);
                    perror ("");
            }
        }
        TaskSetState(pMyLLI->pLLCompletionPCB, PCB_READY);
        TaskSwitch();
        while (pMyLLI->LLTargetSync != RUNNING)
        {
            if (pMyLLI->pLLTargetPCB->pc_stat != PCB_READY)
            {
                FPRINTF5 (stderr, "%sWaiting for %s Target Task. Currently %d @@@@@@@@@@\n", L_MsgPrefix,
                          pLinkNames[myLink], pMyLLI->LLTargetSync);
            }
            TaskSetState(pMyLLI->pLLTargetPCB, PCB_READY);
            (*pMyLLI->pLLTargetUpCounter)++;
            FORCE_WRITE_BARRIER;
            if (*pMyLLI->pLLTargetSleep)
            {

                FPRINTFALL6 (stderr, "%s%d: ******************************** Kick the target for link %s with %d\n", L_MsgPrefix, __LINE__, pLinkNames[pMyLLI->LLmyLink], pMyLLI->LLTargetEvent);
                return_status = CT_ioctl(CT_xiofd, XIO3D_SENDEVT, pMyLLI->LLTargetEvent);
                if (return_status != 0)
                {
                    fprintf(stderr, "%s%d: send event ioctl returned %d, errno=%d for link %s\n", L_MsgPrefix,
                            __LINE__, return_status, errno, pLinkNames[pMyLLI->LLmyLink]);
                    perror ("");
                }
            }
            TaskSleepMS(20);
        }
        while (pMyLLI->LLCompletionSync != RUNNING)
        {
            if (pMyLLI->pLLCompletionPCB->pc_stat != PCB_READY)
            {
                FPRINTF5 (stderr, "%sWaiting for %s Completion Task. Currently %d @@@@@@@@@@\n", L_MsgPrefix,
                          pLinkNames[myLink], pMyLLI->LLCompletionSync);
            }
            TaskSetState(pMyLLI->pLLCompletionPCB, PCB_READY);
            TaskSleepMS(20);
        }

        /*
        ** Indicate the link layer is running.
        */

#ifdef PROC_CODE
        K_ii.status |= II_STATUS_LINKPROC;
#endif /* PROC_CODE */

        fprintf (stderr, "%sInitiator task %s is running! *************************\n", L_MsgPrefix,
                 pLinkNames[myLink]);

#if DEBUG_FLT_REC_LINK
        MSC_FlightRec (FR_L_SYNC, (UINT32)pMyLLI->LLTargetSync,
                       (UINT32)pMyLLI->LLInitiatorSync,
                       (UINT32)pMyLLI->LLmyLink);
#endif /* DEBUG_FLT_REC_LINK */

        /*
        ** Now that this connection is complete, check to see if it is time to
        ** raise the task's priority. If all of the connections have been
        ** established, then the priority will be raised, and we need to set
        ** the boot_sync variable to show we are up and running.
        */

        if (change_my_priority(myLink, TRUE) != 0)
        {
            boot_sync = RUNNING;
        }

        /*
        ** Enter normal processing mode. Process all outgoing message requests
        ** (as queued by LL_QueueMessageToSend).
        */

        LL_NormalInitiatorProcessing(pMyLLI, LINK_QCS.pqc_QB[pMyLLI->LLInitiatorQCSIndex]);

    }
}



/**
******************************************************************************
**
**  @brief      Callback function for a message reply generated on
**              the Target platform.
**
**              This function is called by the task processing a message
**              on the Target platform after it has generated a reply
**              to the message. The message reply is then queued into
**              LLCompletionQueue for the Completion Task.
**
**              This function may block the calling task if the queue
**              has filled.
**
**  @param      dummy   - Place holder param - not used.
**  @param      pTargILT- Pointer to the ILT for this message.
**
**  @return     none
**
******************************************************************************
**/

void LL_TargetTaskCompletion (UNUSED UINT32 dummy, ILT *pTargILT, ...)
{
    ILT                 *pILT, **ppNextEntry;
    VRP                 *pTargVRP;
    LLI                 *pMyLLI;
    UINT32              returnCode;
    int                 return_status;
#if KERNEL_DEBUG_5
    long long           tsc;
    UINT32              tsc_h, tsc_l;
#endif /* KERNEL_DEBUG_5 */

    /*
    ** Get the MRPs pointed to by both the Target ILT and the original ILT.
    */

    pILT = (ILT *)pTargILT->linux_val;
#if KERNEL_DEBUG_5
    tsc = get_tsc() >> 21;
    tsc_h = (UINT32)(tsc >> 32);
    tsc_l = (UINT32)tsc;
    pILT[1].w6 = tsc_h;
    pILT[1].w7 = tsc_l;
    {
        INT32 timediff;
        long long stime, svtime;
        UINT32 kpri_i = kernel_pri_index;

        stime = ((long long)pILT[1].w4 << 32) | ((long long)pILT[1].w5);
        timediff = (INT32)((tsc - stime) & 0x7fffffff);
        if (timediff > 340 && timediff < 2670 && tsc > message_filter_time)
        {
            int ind;
            timediff /= 1.335;
            fprintf (stderr,"\n%s%d:LL_TargetTaskCompletion found excessive time %d\n", L_MsgPrefix,__LINE__,
                     timediff);
            for (ind = 20; ind > 0; ind--)
            {
                svtime = ((long long)kernel_sec[kpri_i] << 32) | ((long long)kernel_usec[kpri_i]);
                timediff = (INT32)((svtime - stime) & 0x7fffffff) / 1.335;
                fprintf (stderr,"\t%s%dth oldest schedule: task %s,  pri 0x%x, time before/after message queueing %d\n", L_MsgPrefix,
                         ind, kernel_name[kpri_i], kernel_pri[kpri_i], timediff);
                if (++kpri_i >= 20) kpri_i = 0;
            }
            message_filter_time = tsc + 2000;
        }
    }
#endif /* KERNEL_DEBUG_5 */

    /*
    ** Get the return code.
    */

    pTargVRP = (VRP *)pTargILT->ilt_normal.w0;
    returnCode = (pTargVRP == NULL) ? 0 : pTargVRP->status;
#ifdef BACKEND
    if(pTargVRP != NULL)
    {

        GR_HandleSpecialMirrorFlags(returnCode, pTargVRP, pTargILT);
        pTargVRP->options = 0;
    }
#endif

    /*
    ** Save the return code into the original ILT.
    */

    pILT->linux_val = returnCode;

    /*
    ** Retrieve the LLI pointer.
    */

    pMyLLI = (LLI *)pTargILT->ilt_normal.w2;

    /*
    ** Check the sync count. If it has changed since the start of processing
    ** this message, then bitbucket this response. Similarly, check the current
    ** synchronization state of the link.
    */

    if (pMyLLI->LLTargetSyncCount == pTargILT->ilt_normal.w1 && pMyLLI->LLInitiatorSync == RUNNING)
    {
        /*
        ** Send the response. First, check for room in the Completion
        ** Response queue.
        */

        ppNextEntry = (ILT **)pMyLLI->pLLCompletionQueueInPtr + 1;
        if (ppNextEntry == pMyLLI->pLLCompletionQueueEnd)
        {
            ppNextEntry = pMyLLI->pLLCompletionQueueStart;
        }
        while (ppNextEntry == pMyLLI->pLLCompletionQueueOutPtr)
        {
            /*
            ** The queue is full. Make sure the completion task
            ** knows there is something waiting for it.
            */

            if (TaskGetState (pMyLLI->pLLCompletionPCB) == PCB_WAIT_LINUX_COMP_IN)
            {
                TaskSetState(pMyLLI->pLLCompletionPCB, PCB_READY);
                (*pMyLLI->pLLCompletionUpCounter)++;
                FORCE_WRITE_BARRIER;
                if (*pMyLLI->pLLCompletionSleep)
                {
                    FPRINTFALL6 (stderr, "%s%d: ******************************** Kick the completion for link %s with %d\n", L_MsgPrefix, __LINE__, pLinkNames[pMyLLI->LLmyLink], pMyLLI->LLCompletionEvent);
                    return_status = CT_ioctl(CT_xiofd, XIO3D_SENDEVT, pMyLLI->LLCompletionEvent);
                    if (return_status != 0)
                    {
                        fprintf(stderr, "%s%d: send event ioctl returned %d, errno=%d for link %s\n", L_MsgPrefix,
                                __LINE__, return_status, errno, pLinkNames[pMyLLI->LLmyLink]);
                        perror ("");
                    }
                }
            }

            /*
            ** Suspend to allow other tasks
            ** to run while we wait for the Completion Task to free up
            ** some entries. Note that we do not set ourselves
            ** as not ready, so we will return without the need
            ** of interaction with another task.
            */

            TaskSwitch();

            /*
            ** Check synchronization.
            */

            if (pMyLLI->LLInitiatorSync != RUNNING)
            {
                FPRINTF4 (stderr, "%sTarget Task resetting %d^^^^^^^^^^\n", L_MsgPrefix, 1);
                break;
            }
        }

#ifdef LL_TRACE
        {
            VRPCMD              *pVRPCMD;
            VRP                 *pVRP;

            pVRPCMD = (VRPCMD *)&pILT->ilt_normal.w0;
            pVRP = pVRPCMD->pvrVRP;
            FPRINTF7 (stderr,"%sTC: %'14d[0x%x]: %d => %s\n", L_MsgPrefix, pILT[1].linux_val,
                      pVRP->function, returnCode, pLinkNames[pMyLLI->LLmyLink]);
        }
#endif /* LL_TRACE */

        /*
        ** Check current synchronization. Only insert the response if we
        ** are still in synchronization.
        */

        if (pMyLLI->LLInitiatorSync == RUNNING)
        {

            /*
            ** Insert the response in the queue.
            */

            *pMyLLI->pLLCompletionQueueInPtr = pILT;
            FORCE_WRITE_BARRIER;
            pMyLLI->pLLCompletionQueueInPtr = ppNextEntry;
            FORCE_WRITE_BARRIER;



            /*
            ** Start up the completion task, so it can process the message.
            */

            if (TaskGetState (pMyLLI->pLLCompletionPCB) == PCB_WAIT_LINUX_COMP_IN)

            {
                TaskSetState(pMyLLI->pLLCompletionPCB, PCB_READY);
                (*pMyLLI->pLLCompletionUpCounter)++;
                FORCE_WRITE_BARRIER;
                if (*pMyLLI->pLLCompletionSleep)
                {
                    FPRINTFALL6 (stderr, "%s%d: ******************************** Kick the completion for link %s with %d\n", L_MsgPrefix, __LINE__, pLinkNames[pMyLLI->LLmyLink], pMyLLI->LLCompletionEvent);
                    return_status = CT_ioctl(CT_xiofd, XIO3D_SENDEVT, pMyLLI->LLCompletionEvent);
                    if (return_status != 0)
                    {
                        fprintf(stderr, "%s%d: send event ioctl returned %d, errno=%d for link %s\n", L_MsgPrefix,
                                __LINE__, return_status, errno, pLinkNames[pMyLLI->LLmyLink]);
                    perror ("");
                    }
                }
            }
        }

#if DEBUG_FLT_REC_LINK
    MSC_FlightRec (FR_L_COMP, (UINT32)pTargILT, (UINT32)pTargVRP, returnCode);
#endif /* DEBUG_FLT_REC_LINK */

    }

    /*
    ** Update the stats.
    */

    L_stattbl->LL_InCount--;

    /*
    ** Release the Target ILT & MRP.
    */

    /*
    ** Fix for bug #TBolt00010764 - Raghu
    ** The VRP on the target side must be distinct from the one
    ** on the initiator side.
    **  0x0000 - 0x00ff     FE -> BE    V$que         Cache to Virtual VRP/MRP
    */

    if (pTargVRP != NULL)
    {
        put_vrp(pTargVRP);
    }
    put_ilt(pTargILT);
}



/**
******************************************************************************
**
**  @brief      Normal operation of incoming message processing task.
**
**              This routine handles the normal operations of receiving
**              messages across its link. It will wait for a message to
**              be sent by the Initiator task on the other end of the link,
**              then it uses the function code to determine the message's
**              destination, and adds the message to the appropriate
**              task's message queue.
**
**  @param      pMyLLI  - Pointer to LLI structure for this link.
**
**  @return     none
**
******************************************************************************
**/

static void LL_NormalTargetProcessing(LLI *pMyLLI)
{
    ILT                 *pILT, *pTargILT, **ppNextILT;
    VRPCMD              *pVRPCMD;
    VRP                 *pTargVRP;
    UINT32              cmdCode;
    void                *pQueFunc;
#if KERNEL_DEBUG_5
    long long           tsc;
    UINT32              tsc_h, tsc_l;
#endif /* KERNEL_DEBUG_5 */

    /*
    ** Loop forever, processing messages. Loss of synchronization will cause
    ** a break out of the loop, returning to LL_TargetTask.
    */

    for(;;)
    {
        /*
        ** Put the task into a wait-for-message state, and rely on
        ** the Initiator Task to start up this task. Note that the
        ** queue is re-checked for messages after changing the task's
        ** state. This is necessary to avoid the race condition where
        ** the Initiator added a message to the queue after our last check,
        ** but sees that we are still in PCB_READY state, so we just
        ** go away ...
        */


        TaskSetMyState(PCB_WAIT_LINUX_TARG_IN);
        FORCE_WRITE_BARRIER;
        if (pMyLLI->pLLTargetQueueInPtr == pMyLLI->pLLTargetQueueOutPtr)
        {
                TaskSwitch();
        }
        else
        {
            TaskSetMyState(PCB_READY);
        }

        /*
        ** Check synchronization.
        */

        if (pMyLLI->LLInitiatorSync != RUNNING)
        {
            FPRINTF5 (stderr, "%sTarget Task %s resetting %d^^^^^^^^^^\n", L_MsgPrefix, pLinkNames[pMyLLI->LLmyLink], 2);
            break;
        }

        /*
        ** Process the message queue.
        */

        while (pMyLLI->pLLTargetQueueInPtr != pMyLLI->pLLTargetQueueOutPtr)
        {
            /*
            ** Get the next message's ILT and increment the pointer.
            */

            ppNextILT = pMyLLI->pLLTargetQueueOutPtr;
            pILT = *ppNextILT++;
            if (ppNextILT == pMyLLI->pLLTargetQueueEnd)
            {
                ppNextILT = pMyLLI->pLLTargetQueueStart;
            }
            pMyLLI->pLLTargetQueueOutPtr = ppNextILT;
#if KERNEL_DEBUG_5
            tsc = get_tsc() >> 21;
            tsc_h = (UINT32)(tsc >> 32);
            tsc_l = (UINT32)tsc;
            pILT[1].w4 = tsc_h;
            pILT[1].w5 = tsc_l;
            {
                INT32 timediff;
                long long stime, svtime;
                UINT32 kpri_i = kernel_pri_index;

                stime = ((long long)pILT[1].w2 << 32) | ((long long)pILT[1].w3);
                timediff = (INT32)((tsc - stime) & 0x7fffffff);
                if (timediff > 340 && timediff < 2670 && tsc > message_filter_time)
                {
                    int ind;
                    timediff /= 1.335;
                    fprintf (stderr,"\n%s%d:LL_NormalTargetProcessing found excessive time %d\n", L_MsgPrefix,__LINE__,
                             timediff);
                    for (ind = 20; ind > 0; ind--)
                    {
                        svtime = ((long long)kernel_sec[kpri_i] << 32) | ((long long)kernel_usec[kpri_i]);
                        timediff = (INT32)((svtime - stime) & 0x7fffffff) / 1.335;
                        fprintf (stderr,"\t%s%dth oldest schedule: task %s,  pri 0x%x, time before/after message queueing %d\n", L_MsgPrefix,
                                 ind, kernel_name[kpri_i], kernel_pri[kpri_i], timediff);
                        if (++kpri_i >= 20) kpri_i = 0;
                    }
                    message_filter_time = tsc + 2000;
                }
            }
#endif /* KERNEL_DEBUG_5 */

            /*
            ** Now allocate an ILT on the Target to use while processing
            ** the message and generating the response. Tie the MRP
            ** to the new ILT, and remember the original ILT address.
            */

            pVRPCMD = (VRPCMD *)&pILT->ilt_normal.w0;

            /*
            ** Fix for bug #TBolt00010764 - Raghu
            ** The VRP on the target side must be distinct from the one
            ** on the initiator side.
            */

            if (pVRPCMD->pvrVRP != NULL)
            {

                /*
                ** Allocate the ILT and VRP, and copy the entire 128 bytes allocated
                ** for the VRP. Then, it may be necessary to modify the VRP SGL pointer
                ** (aka the MRP request buffer pointer). If this pointer is pointing
                ** right behind the original VRP, then the message information was
                ** stuffed in the back of the VRP, and we have copied it. In this
                ** case, adjust the value. If the pointer is pointing somewhere else,
                ** then it is pointing into the Initiator's memory, and we need to
                ** leave it unchanged.
                */

                pTargILT = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, pTargILT);
#endif /* M4_DEBUG_ILT */
                pTargVRP = get_vrp();
#ifdef M4_DEBUG_VRP
CT_history_printf("%s%s:%u get_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, pTargVRP);
#endif /* M4_DEBUG_VRP */
                memcpy(pTargVRP, pVRPCMD->pvrVRP, VRPALLOC);
#if 1//CSTOP_SAN1171,1416,1670
                /*
                ** Clear VRP_IN_BACKEND bit in BE VRP that is copied from  FE VRP.
                ** This is necessary because this bit has different meaning/purpose
                ** and  is differently used in FE VRP and BE VRP.
                */
                BIT_CLEAR(pTargVRP->options,VRP_IN_BACKEND);
#endif
                if ((UINT32)pTargVRP->pSGL == (UINT32)pVRPCMD->pvrVRP + sizeof (VRP))
                {
                    pTargVRP->pSGL = (SGL *) ((UINT32)pTargVRP + sizeof (VRP));
                }


                /*
                ** Save the target VRP address in the Initiator VRP for debugging.
                */

                pVRPCMD->pvrVRP->pktAddr = pTargVRP;
            }
            else
            {

                /*
                ** No VRP. Just allocate the ILT, and set the VRP pointer to NULL.
                */

                pTargILT = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, pTargILT);
#endif /* M4_DEBUG_ILT */
                pTargVRP = pVRPCMD->pvrVRP;
            }
            pTargILT->ilt_normal.w0 = (UINT32)pTargVRP;
/* This is because of a problem where it failed in dlm.as routine dlm$MRC where
   it moves il_misc from nest level 2 to nest level 3. This is because of
   uninitialized memory. This does not fix it, it masks the symptoms.

   The 2nd level ([1]) is needed for dlm.as: dlm$MRC fetching il_misc. */
            pTargILT->misc = 0;
            pTargILT[1].misc = 0;

            pTargILT->linux_val = (UINT32)pILT;

            /*
            ** For easier debugging, save the Target ILT and VRP in the initiator
            ** ILT. Use the "extra" ILT instance for Linux code. Since the
            ** KERNEL_DEBUG_5 stuff uses w0 - w7, save the Target ILT in the
            ** misc value, and the Target VRP in the cr.
            */

            pILT[1].misc = (UINT32)pTargILT;
            pILT[1].cr = (void *)pTargVRP;

            /*
            ** Save the current sync count and LLI (they are used in
            **  LL_TargetTaskCompletion)
            */

            pTargILT->ilt_normal.w1 = pMyLLI->LLTargetSyncCount;
            pTargILT->ilt_normal.w2 = (UINT32)pMyLLI;

            /*
            ** Use the function code to determine the destination queue & task.
            */

            cmdCode = pTargVRP->function;
            if (cmdCode > LARGEST_FUNC_CODE)
            {
                fprintf(stderr, "%s: Bad cmdCode 0x%04X, pTargVRP=%p, pILT=%p, "
                        "pTargILT=%p, pMyLLI=%p\n",
                        __func__, cmdCode, pTargVRP, pILT, pTargILT, pMyLLI);
                abort();        /* Die right here */
                //error11();
            }
            pQueFunc = pLinkQueFunc[cmdCode];

#ifdef BACKEND
            /*
            ** Save the Target ILT in the Target VRP for virtual layer debug only.
            */

            if (pQueFunc == V$que && cmdCode != VRP_LINK_DRV_OK)
            {
                pTargVRP->gen0 = (UINT32)pTargILT;
            }
#endif /* BACKEND */

            /*
            ** Send the message to the correct routine.
            */

#ifdef LL_TRACE
            FPRINTF6 (stderr,"%sTT: %'14d[0x%x] <= %s\n", L_MsgPrefix, pILT[1].linux_val,
                      cmdCode, pLinkNames[pMyLLI->LLmyLink]);
#endif /* LL_TRACE */
            if (pQueFunc != 0)
            {
#ifdef BACKEND
                /*
                ** In case of V$que, indicate that the link layer is originator.
                ** for this VRP.
                */
                if(pQueFunc == V$que)
                {
                    if( BIT_TEST(pTargVRP->options,VRP_VLINK_OPT) == FALSE)
                    {
                        BIT_SET(pTargVRP->options,VRP_SERVER_ORIGINATOR);
                    }
                    pTargILT->misc = 0;   /* Going to store source VDD or vid  */
                }
#endif
                EnqueueILT (pQueFunc, pTargILT, TARGETTASKCOMPLETION);
            }

            /*
            ** Update statistics
            */

            L_stattbl->LL_InCount++;
            L_stattbl->LL_TotInCount++;

#if DEBUG_FLT_REC_LINK
            MSC_FlightRec (FR_L_EXECI, (UINT32)pTargILT, (UINT32)pTargVRP, cmdCode);
#endif /* DEBUG_FLT_REC_LINK */

        }
    }
}


/**
******************************************************************************
**
**  @brief      Task to process incoming messages.
**
**              This routine is called as a new task. There is one
**              such task for each target (incoming) link.
**
**               After initialization, this tasks waits for any
**              incoming message to arrive. When it does arrive,
**              it uses the message function code to determine
**              the destination task. It then puts the message
**              into that task's queue, and enables the task if
**              necessary.
**
**  @param      ptr - Pointer to task param block (CCB ONLY). The same value
**                  is passed in the g5 and g6 registers for the FE & BE.
**
**  @return     Never exits.
**
******************************************************************************
**/

NORETURN void LL_TargetTask(UNUSED void *ptr)
{
#ifdef CCB_RUNTIME_CODE
    LL_POSSIBLE_LINKS   myLink = ((TASK_PARMS *)ptr)->p1;
    SHM                 *pHisSharedMem = (SHM *)((TASK_PARMS *)ptr)->p2;
#else
    LL_POSSIBLE_LINKS   myLink = g5;
    SHM                 *pHisSharedMem = (SHM *)g6;
#endif /* CCB_RUNTIME_CODE */
    LLI                 *pMyLLI;
    int                 return_status;

    /* Lower priority */
    change_my_priority (myLink, FALSE);

    /*
    ** Wait for our partner to set up his LLI Directory. Then, set
    ** up our information. The value at startup may be trash (since we are using
    ** "random" kernel memory, so first do a range check on it.
    */

    while (pHisSharedMem->SHMStructVersionID != SM_STRUCT_VERSION)
    {
        /*
        ** Sleep for a millisecond, and try again.
        */

        TaskSleepMS (1);
    }
    if (pHisSharedMem->SHMSync > RUNNING)
    {
        pHisSharedMem->SHMSync = POWERUP;
    }
    while (pHisSharedMem->SHMSync != SYNCHRONIZE)
    {
        /*
        ** Sleep for a millisecond, and try again.
        */

        TaskSleepMS (1);
    }
    pMySharedMem->pSHMLLIDir[myLink] = pHisSharedMem->pSHMLLIDir[myLink];
    pMyLLI = pMySharedMem->pSHMLLIDir[myLink];
    pMyLLI->LLTargetSync = POWERUP;
    pMyLLI->LLTargetSyncCount = 0;

#if DEBUG_FLT_REC_LINK
    MSC_FlightRec (FR_L_SYNC, (UINT32)pMyLLI->LLTargetSync,
                   (UINT32)pMyLLI->LLInitiatorSync,
                   (UINT32)pMyLLI->LLmyLink);
#endif /* DEBUG_FLT_REC_LINK */

    if (pMyLLI->LLInitiatorSync == RUNNING)
    {
        pMyLLI->LLInitiatorSync = MUST_RESYNC;
        TaskSetState(pMyLLI->pLLInitiatorPCB, PCB_READY);
        TaskSetState(pMyLLI->pLLCompletionPCB, PCB_READY);
        (*pMyLLI->pLLCompletionUpCounter)++;
        FORCE_WRITE_BARRIER;
        if (*pMyLLI->pLLCompletionSleep)
        {
            FPRINTFALL6 (stderr, "%s%d: ******************************** Kick the completion for link %s with %d\n", L_MsgPrefix, __LINE__, pLinkNames[pMyLLI->LLmyLink], pMyLLI->LLCompletionEvent);
            return_status = CT_ioctl(CT_xiofd, XIO3D_SENDEVT, pMyLLI->LLCompletionEvent);
            if (return_status != 0)
            {
                fprintf(stderr, "%s%d: send event ioctl returned %d, errno=%d for link %s\n", L_MsgPrefix,
                        __LINE__, return_status, errno, pLinkNames[pMyLLI->LLmyLink]);
                perror ("");
            }
        }
    }

    /*
    ** Save our PCB and kernel information in the LLI
    */

#ifdef CCB_RUNTIME_CODE
    pMyLLI->pLLTargetPCB = XK_GetPcb();
#else   /* CCB_RUNTIME_CODE */
    pMyLLI->pLLTargetPCB = K_xpcb;
#endif  /* CCB_RUNTIME_CODE */
    pMyLLI->pLLTargetUpCounter = &kernel_up_counter;
    pMyLLI->pLLTargetSleep = &kernel_sleep;
    switch (myLink)
    {
        case BE2FE:
        case CCB2FE:
            pMyLLI->LLTargetEvent = XIO3D_FE_EVT;
            break;
        case FE2BE:
        case CCB2BE:
            pMyLLI->LLTargetEvent = XIO3D_BE_EVT;
            break;
        case FE2CCB:
        case BE2CCB:
            pMyLLI->LLTargetEvent = XIO3D_CCB_EVT;
            break;
        default:        /* stop compiler warning only. */
            break;
    }

    /*
    ** Initialize the Target Input queue.
    */

    pMyLLI->pLLTargetQueueStart =
        (ILT **)MALLOC_PERM_CACHED_ZERO (VRMAX *
                                         sizeof (*pMyLLI->pLLTargetQueueStart));
    pMyLLI->pLLTargetQueueEnd = pMyLLI->pLLTargetQueueStart + VRMAX;
    pMyLLI->pLLTargetQueueInPtr = pMyLLI->pLLTargetQueueStart;
    pMyLLI->pLLTargetQueueOutPtr = pMyLLI->pLLTargetQueueInPtr;

    /*
    ** Loop forever, processing incoming message packets.
    */

    for (;;)
    {
        FPRINTF4 (stderr, "%sTarget task %s at resync point.\n", L_MsgPrefix, pLinkNames[myLink]);

        /*
        ** Make sure that we are not running at high priority. If we are,
        ** then it is possible for the two high priority tasks
        ** (Front.t & Back.t) to starve everything else
        ** on the Linux box. When this happens, it's power cycle time.
        */

        if (change_my_priority (myLink, FALSE))
        {
            boot_sync = SYNCHRONIZE;
        }

        /*
        ** We come here, too, if we lose synchronization later. Indicate that
        ** we are here, and suspend until the Initiator Task is ready for us.
        */

        /*
        ** First, wait for the Initiator Task to reach the synchronization point.
        */

        for (;;)
        {
#ifdef CCB_RUNTIME_CODE
            /*
            ** Check whether the processer error trapped.
            */
            if (pMyLLI->LLInitiatorSync == ERR_TRAP)
            {
                fprintf(stderr, "%s+-+-+-+- %s ERROR TRAP -+-+-+-+\n", L_MsgPrefix,
                        (myLink == FE2CCB) ? "FRONTEND" : "BACKEND");
                LL_Logtrap( pMyLLI );
            }
#endif /* CCB_RUNTIME_CODE */
            pMyLLI->LLTargetSync = READY;
            TaskSetMyState(PCB_NOT_READY);
            TaskSwitch();
            if (pMyLLI->LLInitiatorSync == SYNCHRONIZE)
            {
                break;
            }
        }

#if DEBUG_FLT_REC_LINK
        MSC_FlightRec (FR_L_SYNC, (UINT32)pMyLLI->LLTargetSync,
                       (UINT32)pMyLLI->LLInitiatorSync,
                       (UINT32)pMyLLI->LLmyLink);
#endif /* DEBUG_FLT_REC_LINK */

        /*
        ** Now, wait for the Initiator Task to tell us to run.
        */


        for (;;)
        {
#ifdef CCB_RUNTIME_CODE
            /*
            ** Check whether the processer error trapped.
            */
            if (pMyLLI->LLInitiatorSync == ERR_TRAP)
            {
                fprintf(stderr, "%s+-+-+-+- %s ERROR TRAP -+-+-+-+\n", L_MsgPrefix,
                        (myLink == FE2CCB) ? "FRONTEND" : "BACKEND");
                LL_Logtrap( pMyLLI );
            }
#endif /* CCB_RUNTIME_CODE */
            pMyLLI->LLTargetSync = SYNCHRONIZE;
            TaskSetMyState(PCB_NOT_READY);
            TaskSwitch();

            if (pMyLLI->LLInitiatorSync == RUNNING)
            {
                break;
            }
        }

#if DEBUG_FLT_REC_LINK
        MSC_FlightRec (FR_L_SYNC, (UINT32)pMyLLI->LLTargetSync,
                       (UINT32)pMyLLI->LLInitiatorSync,
                       (UINT32)pMyLLI->LLmyLink);
#endif /* DEBUG_FLT_REC_LINK */

        /*
        ** We are ready to run.
        */

        /*
        ** Now that this connection is complete, check to see if it is time to
        ** raise the task's priority. If all of the connections have been
        ** established, then the priority will be raised, and we need to set
        ** the boot_sync variable to show we are up and running.
        */

        if (change_my_priority(myLink, TRUE) != 0)
        {
            boot_sync = RUNNING;
        }

        pMyLLI->LLTargetSync = RUNNING;
        pMyLLI->LLTargetSyncCount++;

#if DEBUG_FLT_REC_LINK
        MSC_FlightRec (FR_L_SYNC, (UINT32)pMyLLI->LLTargetSync,
                       (UINT32)pMyLLI->LLInitiatorSync,
                       (UINT32)pMyLLI->LLmyLink);
#endif /* DEBUG_FLT_REC_LINK */

        fprintf (stderr, "%sTarget task %s is running! ****************************\n", L_MsgPrefix,
                 pLinkNames[myLink]);

        /*
        ** Normal operation.
        */

        LL_NormalTargetProcessing (pMyLLI);
    }
}



/**
******************************************************************************
**
**  @brief      Return the message reply to the calling task.
**
**              When a message reply has been received (or communication
**              has been lost and an error must be returned), pass
**              the response to the calling task via the callback
**              routine specified when the message was queued.
**
**  @param      pILT    - Pointer to ILT of the reply
**  @param      myLink  - Link identification (FE2BE, BE2CCB, etc)
**
**  @return     none
**
******************************************************************************
**/

static void LL_ForwardTheReply (ILT *pILT, D5_UNUSED_PARAM LL_POSSIBLE_LINKS myLink)
{
    UINT32              returnCode;
    void                (*callback)(UINT32, struct ILT *, ...);
    UINT32              asmCallbackFunc;
    VRPCMD              *pVRPCMD;
    VRP                 *pVRP;
#if KERNEL_DEBUG_5
    long long           tsc;
    UINT32              tsc_h, tsc_l;
    long long           stime, svtime;
    UINT32              timediff;
#endif /* KERNEL_DEBUG_5 */

    /*
    ** Remove the ILT from the outstanding message list.
    */

    pILT++;
#if KERNEL_DEBUG_5
    tsc = get_tsc() >> 21;
    tsc_h = (UINT32)(tsc >> 32);
    tsc_l = (UINT32)tsc;
    {
        UINT32 kpri_i = kernel_pri_index;

        stime = ((long long)pILT[0].w6 << 32) | ((long long)pILT[0].w7);
        timediff = (INT32)((tsc - stime) & 0x7fffffff);
        if (timediff > 340 && timediff < 2670 && tsc > message_filter_time)
        {
            int ind;
            timediff /= 1.335;
            fprintf (stderr,"\n%s%d:LL_ForwardTheReply found excessive time %d\n", L_MsgPrefix,__LINE__,
                     timediff);
            for (ind = 20; ind > 0; ind--)
            {
                svtime = ((long long)kernel_sec[kpri_i] << 32) | ((long long)kernel_usec[kpri_i]);
                timediff = (INT32)((svtime - stime) & 0x7fffffff) / 1.335;
                fprintf (stderr,"\t%s%dth oldest schedule: task %s,  pri 0x%x, time before/after message queueing %d\n", L_MsgPrefix,
                         ind, kernel_name[kpri_i], kernel_pri[kpri_i], timediff);
                if (++kpri_i >= 20) kpri_i = 0;
            }
            message_filter_time = tsc + 2000;
        }
    }

    {
        stime = ((long long)pILT[0].w0 << 32) | ((long long)pILT[0].w1);
        timediff = (INT32)((tsc - stime) & 0x7fffffff);
        if (timediff > 2000 && tsc > message_filter_time)
        {

            timediff /= 1.335;
            fprintf (stderr,"\n%s%d:LL_ForwardTheReply found excessive time %d at time %.2f ms\n",
                     L_MsgPrefix,__LINE__, timediff, (tsc_l & 0xFFFF) / 1.335);
            fprintf (stderr,"\t%s Message number %d, link %s, type 0x%x\n", L_MsgPrefix,
                     pILT->linux_val, pLinkNames[myLink],
                     ((VRPCMD *)&pILT[-1].w0)->pvrVRP->function);
            svtime = ((long long)pILT[0].w2 << 32) | ((long long)pILT[0].w3);
            timediff = (INT32)((svtime - stime) & 0x7fffffff) / 1.335;
            fprintf (stderr,"\t%s Time from Que to Initiator = %d\n",
                     L_MsgPrefix, timediff);
            stime = svtime;
            svtime = ((long long)pILT[0].w4 << 32) | ((long long)pILT[0].w5);
            timediff = (INT32)((svtime - stime) & 0x7fffffff) / 1.335;
            fprintf (stderr,"\t%s Time from Initiator to Target = %d\n",
                     L_MsgPrefix, timediff);
            stime = svtime;
            svtime = ((long long)pILT[0].w6 << 32) | ((long long)pILT[0].w7);
            timediff = (INT32)((svtime - stime) & 0x7fffffff) / 1.335;
            fprintf (stderr,"\t%s Time from Target to Target Completion = %d\n",
                     L_MsgPrefix, timediff);
            stime = svtime;
            timediff = (INT32)((tsc - stime) & 0x7fffffff) / 1.335;
            fprintf (stderr,"\t%s Time from Target Completion to Completion = %d\n",
                     L_MsgPrefix, timediff);
        }
        stime = ((long long)pILT[0].w0 << 32) | ((long long)pILT[0].w1);
        timediff = (INT32)((tsc - stime) & 0x7fffffff);
    }
#endif /* KERNEL_DEBUG_5 */

    pILT->fthd->bthd = pILT->bthd;
    pILT->bthd->fthd = pILT->fthd;
    pILT--;

    /*
    ** Extract the return code from the ILT, and put it into the VRP/SRP/MRP/DRP/....
    */

    returnCode = pILT->linux_val;
    pVRPCMD = (VRPCMD *)&pILT->ilt_normal.w0;
    pVRP = pVRPCMD->pvrVRP;
    pVRP->status = returnCode;

    /*
    ** Call the completion function. The callback variable in the ILT has the
    ** address of the callback routine, possibly OR'ed with BIT31. BIT31 is
    ** set if the callback function is in assembly language; it is clear if
    ** the callback function is in C. This distinction is important in the
    ** auto-converted code environment, because "magic sauce" must be applied
    ** to any calls into assembly language routines from C code.
    */

    callback = pILT->cr;
    asmCallbackFunc = (UINT32)callback & BIT31;
    callback = (void (*)(UINT32, struct ILT *, ...))((UINT32)callback & ~BIT31);

#ifdef LL_TRACE
    {
        VRPCMD              *pTraceVRPCMD;
        MR_PKT              *pTraceMRP;

        pTraceVRPCMD = (VRPCMD *)&pILT->ilt_normal.w0;
        pTraceMRP = (MR_PKT *)pTraceVRPCMD->pvrVRP;
        FPRINTF7 (stderr,"%sCT: %'14d[0x%x]: %d <= %s\n", L_MsgPrefix, pILT[1].linux_val,
                  ((VRP *)pTraceMRP)->function, returnCode, pLinkNames[myLink]);
    }
#endif /* LL_TRACE */

    if (callback != 0)
    {
#ifdef PROC_CODE
        if (asmCallbackFunc)
        {
            /*
            ** The callback variable had BIT31 set, so it is an assembly
            ** routine. Replace the callback address in the ILT with the
            ** actual address, and call the assembly language routine KernelDispatch.
            ** This will ensure the "magic sauce" is present.
            */

            pILT->cr = callback;
            KernelDispatch (returnCode, pILT, NULL, 0);
        }
        else
#endif /* PROC_CODE */
        {
            void (*ck)(UINT32, struct ILT *);
            /*
            ** The callback is written in C, or we are on the CCB. Call the
            ** function directly.
            */

            ck = pILT->cr;
            (*ck)(returnCode, pILT);
        }
    }
}

/**
******************************************************************************
**
**  @brief      Return errors for all outstanding messages when communication
**              has been lost.
**
**              If communication has been lost with a Target, then
**              all of the outstanding messages (sent messages that
**              have not received a response) must be handled.
**
**              The sending task is sent an error of EC_LINK_FAIL
**              for each of the messages.
**
**              This routine is an error recovery routine. It is only
**              called when communication is lost.
**
**  @param      pMyLLI  - Pointer to LLI structure for this link.
**
**  @return     none
**
******************************************************************************
**/

static void LL_FlushOutstandingMessageList(LLI *pMyLLI)
{
    ILT                 *pILT;

    /*
    ** Process all of the ILTs in the outstanding message list.
    */

    while (pMyLLI->pLLActiveILTHead != (ILT *)&pMyLLI->pLLActiveILTHead)
    {
        /*
        ** Get the first message in the list.
        */

        pILT = pMyLLI->pLLActiveILTHead;

        /*
        ** Adjust the ILT pointer to point to the message ILT.
        */

        pILT--;

        /*
        ** Store the error return into the ILT.
        */

        pILT->linux_val = EC_LINK_FAIL;

        /*
        ** Send the error.
        */

#if KERNEL_DEBUG_5
        if (pMyLLI->LLmyLink != FE2BE && pMyLLI->LLmyLink != BE2FE)
        {
            long long           tsc;
            UINT32              tsc_h, tsc_l;

            tsc = get_tsc() >> 21;
            tsc_h = (UINT32)(tsc >> 32);
            tsc_l = (UINT32)tsc;
            pILT[1].w0 = pILT[1].w2 = pILT[1].w4 = pILT[1].w6 = tsc_h;
            pILT[1].w1 = pILT[1].w3 = pILT[1].w5 = pILT[1].w7 = tsc_l;
        }
#endif /* KERNEL_DEBUG_5 */

        LL_ForwardTheReply (pILT, pMyLLI->LLmyLink);
    }
}



/**
******************************************************************************
**
**  @brief      Normal operation of incoming response processing task.
**
**              This routine handles the normal operations of receiving
**              replies across its link. It will wait for a reply to
**              be sent by the Target task on the other end of the link,
**              then it forwards the reply by calling the message
**              completion routine specified when the message was queued.
**
**  @param      pMyLLI  - Pointer to LLI structure for this link.
**
**  @return     none
**
******************************************************************************
**/

static void LL_NormalCompletionProcessing(LLI *pMyLLI)
{
    ILT                 *pILT, **ppNextILT;


    /*
    ** Loop forever, processing responses. Loss of synchronization will cause
    ** a break out of the loop, returning to LL_CompletionTask.
    */

    for(;;)
    {
        /*
        ** Put the task into a wait-for-message state, and rely on
        ** the Target to start up this task. Note that the
        ** queue is re-checked for messages after changing the task's
        ** state. This is necessary to avoid the race condition where
        ** the Target added a message to the queue after our last check,
        ** but sees that we are still in PCB_READY state, so we just
        ** go away ...
        */

        TaskSetMyState(PCB_WAIT_LINUX_COMP_IN);
        FORCE_WRITE_BARRIER;
        if (pMyLLI->pLLCompletionQueueInPtr == pMyLLI->pLLCompletionQueueOutPtr)
        {
            TaskSwitch();
        }
        else
        {
            TaskSetMyState(PCB_READY);
        }

        /*
        ** Check synchronization.
        */

        if (pMyLLI->LLInitiatorSync != RUNNING)
        {
            /*
            ** Flush the queue of outstanding messages. Send an error return
            ** to the sender for each message that we have not received an
            ** answer.
            */

            LL_FlushOutstandingMessageList(pMyLLI);
            FPRINTF5 (stderr, "%sCompletion Task %s resetting %d^^^^^^^^^^\n", L_MsgPrefix, pLinkNames[pMyLLI->LLmyLink], 1);
            break;
        }

        /*
        ** Process the message queue.
        */

        while (pMyLLI->pLLCompletionQueueInPtr != pMyLLI->pLLCompletionQueueOutPtr)
        {
            /*
            ** Get the next responses's ILT and increment the pointer.
            */

            ppNextILT = (ILT **)pMyLLI->pLLCompletionQueueOutPtr;
            pILT = *ppNextILT++;
            if (ppNextILT == pMyLLI->pLLCompletionQueueEnd)
            {
                ppNextILT = (ILT **)pMyLLI->pLLCompletionQueueStart;
            }
            pMyLLI->pLLCompletionQueueOutPtr = ppNextILT;
            L_stattbl->LL_OutCount--;

#if DEBUG_FLT_REC_LINK
            {
                VRP *pVRP = ((VRPCMD *)&pILT->ilt_normal.w0)->pvrVRP;
                MSC_FlightRec (FR_L_EXECC, (UINT32)pILT, (UINT32)pVRP,
                               pVRP == NULL ? 0 : (UINT32)pVRP->pSGL);
            }
#endif /* DEBUG_FLT_REC_LINK */

            LL_ForwardTheReply (pILT, pMyLLI->LLmyLink);

        }
    }
}


/**
******************************************************************************
**
**  @brief      Task to handle message replies generated by the target.
**
**              This routine is called as a new task. There is one
**              such task for each initiator (outgoing) link.
**
**              After initialization, this task waits to be notified
**              that an outgoing message has been processed by
**              the target and a reply generated. It then handles
**              the completion tasks for that message.
**
**  @param      ptr - Pointer to task param block (CCB ONLY). The same value
**                  is passed in the g5 register for the FE & BE.
**
**  @return     Never exits.
**
******************************************************************************
**/

NORETURN
void LL_CompletionTask(UNUSED void *ptr)
{
#ifdef CCB_RUNTIME_CODE
    LL_POSSIBLE_LINKS   myLink = ((TASK_PARMS *)ptr)->p1;
#else
    LL_POSSIBLE_LINKS   myLink = g5;
#endif /* CCB_RUNTIME_CODE */
    LLI                 *pMyLLI = pMySharedMem->pSHMLLIDir[myLink];

    /*
    ** Save our information in the LLI.
    */

#ifdef CCB_RUNTIME_CODE
    pMyLLI->pLLCompletionPCB = XK_GetPcb();
#else   /* CCB_RUNTIME_CODE */
    pMyLLI->pLLCompletionPCB = K_xpcb;
#endif  /* CCB_RUNTIME_CODE */
    pMyLLI->pLLCompletionUpCounter = &kernel_up_counter;

    /*
    ** Loop forever, processing incoming message packets.
    */

    for (;;)
    {

        FPRINTF4 (stderr, "%sCompletion task %s at resync point.\n", L_MsgPrefix, pLinkNames[myLink]);

        /*
        ** We come here, too, if we lost synchronization later. Indicate we are
        ** here, and suspend until the Initiator Task is ready for us.
        */

        pMyLLI->LLCompletionSync = SYNCHRONIZE;
        TaskSetMyState(PCB_NOT_READY);
        TaskSwitch();
        if (pMyLLI->LLInitiatorSync != RUNNING)
        {
            FPRINTF4 (stderr,
                      "%sCompletion task %s, InitiatorTask not at RUNNING\n", L_MsgPrefix,
                      pLinkNames[myLink]);
            continue;
        }

        pMyLLI->LLCompletionSync = RUNNING;
        fprintf (stderr, "%sCompletion task %s is running! ************************\n", L_MsgPrefix,
                 pLinkNames[myLink]);

        /*
        ** Normal operation.
        */

        LL_NormalCompletionProcessing (pMyLLI);
    }
}


/**
******************************************************************************
**
**  @brief      Task to perform startup initialization of Link Layer
**
**              This routine is called as a new task to initialize
**              the Link Layer during system start. After initialization
**              is complete, this task exits.
**
**  @return     none
**
******************************************************************************
**/


static void LL_BootInit (void)
{
    int     ii, initiator[2], target[2];
    void    *queptr;
    SHM     *pHisSharedMem[4];
    LLI     *pMyLLI;
#if KERNEL_DEBUG_5
    long long   tsc;

    /*
    ** Initialize message filter to ignore everything until we are settled down.
    */

    tsc = get_tsc() >> 21;
    message_filter_time = tsc + 15000;
#endif /* KERNEL_DEBUG_5 */

    /*
    ** Pick up local formatting (numbers, money, etc.), and colations, ...
    */

    setlocale(LC_ALL, "");

    /*
    ** Initialize the QB pointers in LINK_QCS
    */

    LINK_QCS.pqc_QB[0] = &llQb0;
    llQb0.qb_flags = QB_EMPTY;
    llQb0.qb_ord = 0;
    llQb0.qb_pstat = PCB_WAIT_LINK;

    LINK_QCS.pqc_QB[1] = &llQb1;
    llQb1.qb_flags = QB_EMPTY;
    llQb1.qb_ord = 1;
    llQb1.qb_pstat = PCB_WAIT_LINK + 1;

    /*
    ** Initialize the LLI directory.
    */

#ifdef FRONTEND

    pMyLLI = &pMySharedMem->SHMLLI[0];
    pMyLLI->LLInitiatorQCSIndex = 0;
    pMyLLI->LLmyLink = FE2BE;
    pMyLLI->pLLCompletionSleep = &kernel_sleep;
    pMyLLI->LLCompletionEvent = XIO3D_FE_EVT;
    llQb0.qb_max = LL_INIT_Q_DEPTH;
    llQb0.qb_lowWater = LL_INIT_Q_LOWATER;
    pMySharedMem->pSHMLLIDir[FE2BE] = pMyLLI;
    initiator[0] = FE2BE;
    pHisSharedMem[0] = pBESharedMem;
    pMyLLI->LLInitiatorSync = POWERUP;
    pMyLLI->LLCompletionSync = POWERUP;
    if (pMyLLI->LLTargetSync == RUNNING)
    {
        pMyLLI->LLTargetSync = MUST_RESYNC;
    }

    target[0] = CCB2FE;
    pHisSharedMem[2] = pCCBSharedMem;

    target[1] = BE2FE;
    pHisSharedMem[3] = pBESharedMem;

    pMyLLI = &pMySharedMem->SHMLLI[1];
    pMyLLI->LLInitiatorQCSIndex = 1;
    pMyLLI->LLmyLink = FE2CCB;
    pMyLLI->pLLCompletionSleep = &kernel_sleep;
    pMyLLI->LLCompletionEvent = XIO3D_FE_EVT;
    llQb1.qb_max = LL_INIT_Q_DEPTH_CCB;
    llQb1.qb_lowWater = LL_INIT_Q_LOWATER_CCB;
    pMySharedMem->pSHMLLIDir[FE2CCB] = pMyLLI;
    initiator[1] = FE2CCB;
    pHisSharedMem[1] = pCCBSharedMem;
    pMyLLI->LLInitiatorSync = POWERUP;
    pMyLLI->LLCompletionSync = POWERUP;
    if (pMyLLI->LLTargetSync == RUNNING)
    {
        pMyLLI->LLTargetSync = MUST_RESYNC;
    }

#elif defined (BACKEND)

    target[0] = FE2BE;
    pHisSharedMem[2] = pFESharedMem;

    target[1] = CCB2BE;
    pHisSharedMem[3] = pCCBSharedMem;

    pMyLLI = &pMySharedMem->SHMLLI[0];
    pMyLLI->LLInitiatorQCSIndex = 0;
    pMyLLI->LLmyLink = BE2FE;
    pMyLLI->pLLCompletionSleep = &kernel_sleep;
    pMyLLI->LLCompletionEvent = XIO3D_BE_EVT;
    llQb0.qb_max = LL_INIT_Q_DEPTH;
    llQb0.qb_lowWater = LL_INIT_Q_LOWATER;
    pMySharedMem->pSHMLLIDir[BE2FE] = pMyLLI;
    initiator[0] = BE2FE;
    pHisSharedMem[0] = pFESharedMem;
    pMyLLI->LLInitiatorSync = POWERUP;
    pMyLLI->LLCompletionSync = POWERUP;
    if (pMyLLI->LLTargetSync == RUNNING)
    {
        pMyLLI->LLTargetSync = MUST_RESYNC;
    }

    pMyLLI = &pMySharedMem->SHMLLI[1];
    pMyLLI->LLInitiatorQCSIndex = 1;
    pMyLLI->LLmyLink = BE2CCB;
    pMyLLI->pLLCompletionSleep = &kernel_sleep;
    pMyLLI->LLCompletionEvent = XIO3D_BE_EVT;
    llQb1.qb_max = LL_INIT_Q_DEPTH_CCB;
    llQb1.qb_lowWater = LL_INIT_Q_LOWATER_CCB;
    pMySharedMem->pSHMLLIDir[BE2CCB] = pMyLLI;
    initiator[1] = BE2CCB;
    pHisSharedMem[1] = pCCBSharedMem;
    pMyLLI->LLInitiatorSync = POWERUP;
    pMyLLI->LLCompletionSync = POWERUP;
    if (pMyLLI->LLTargetSync == RUNNING)
    {
        pMyLLI->LLTargetSync = MUST_RESYNC;
    }

#elif defined (CCB_RUNTIME_CODE)

    pMyLLI = &pMySharedMem->SHMLLI[0];
    pMyLLI->LLInitiatorQCSIndex = 0;
    pMyLLI->LLmyLink = CCB2FE;
    pMyLLI->pLLCompletionSleep = &kernel_sleep;
    pMyLLI->LLCompletionEvent = XIO3D_CCB_EVT;
    llQb0.qb_max = LL_INIT_Q_DEPTH;
    llQb0.qb_lowWater = LL_INIT_Q_LOWATER;
    pMySharedMem->pSHMLLIDir[CCB2FE] = pMyLLI;
    initiator[0] = CCB2FE;
    pHisSharedMem[0] = pFESharedMem;
    pMyLLI->LLInitiatorSync = POWERUP;
    pMyLLI->LLCompletionSync = POWERUP;
    if (pMyLLI->LLTargetSync == RUNNING)
    {
        pMyLLI->LLTargetSync = MUST_RESYNC;
    }

    pMyLLI = &pMySharedMem->SHMLLI[1];
    pMyLLI->LLInitiatorQCSIndex = 1;
    pMyLLI->LLmyLink = CCB2BE;
    pMyLLI->pLLCompletionSleep = &kernel_sleep;
    pMyLLI->LLCompletionEvent = XIO3D_CCB_EVT;
    llQb1.qb_max = LL_INIT_Q_DEPTH;
    llQb1.qb_lowWater = LL_INIT_Q_LOWATER;
    pMySharedMem->pSHMLLIDir[CCB2BE] = pMyLLI;
    initiator[1] = CCB2BE;
    pHisSharedMem[1] = pBESharedMem;
    pMyLLI->LLInitiatorSync = POWERUP;
    pMyLLI->LLCompletionSync = POWERUP;
    if (pMyLLI->LLTargetSync == RUNNING)
    {
        pMyLLI->LLTargetSync = MUST_RESYNC;
    }

    target[0] = FE2CCB;
    pHisSharedMem[2] = pFESharedMem;

    target[1] = BE2CCB;
    pHisSharedMem[3] = pBESharedMem;

#endif

    /*
    ** Initialize the Outstanding ILT list pointers.
    */

    pMySharedMem->SHMLLI[0].pLLActiveILTHead = (ILT *)&pMySharedMem->SHMLLI[0].pLLActiveILTHead;
    pMySharedMem->SHMLLI[0].pLLActiveILTTail = (ILT *)&pMySharedMem->SHMLLI[0].pLLActiveILTHead;
    pMySharedMem->SHMLLI[1].pLLActiveILTHead = (ILT *)&pMySharedMem->SHMLLI[1].pLLActiveILTHead;
    pMySharedMem->SHMLLI[1].pLLActiveILTTail = (ILT *)&pMySharedMem->SHMLLI[1].pLLActiveILTHead;

    /*
    ** For the two initiators, fork off an initiator task, and a
    ** completion task. For the two targets, fork off a target
    ** task.
    */

    for (ii = 0; ii < 2; ii++)
    {
        /*
        ** Pass the offset into lliDir to the new tasks.
        **/

#ifdef PROC_CODE
        CT_fork_tmp = (ii == 0) ? (ulong)"LL_InitiatorTask 1":
            (ulong)"LL_InitiatorTask 2";
#endif /* PROC_CODE */
        TASKCREATEPERM (LL_InitiatorTask, LL_INITIATOR_PRI, initiator[ii],
                        pHisSharedMem[ii], &LINK_QCS.pqc_PCB[ii]);

#ifdef PROC_CODE
        CT_fork_tmp = (ii == 0) ? (ulong)"LL_CompletionTask 1":
            (ulong)"LL_CompletionTask 2";
#endif /* PROC_CODE */
        TASKCREATEPERM (LL_CompletionTask, LL_COMPLETION_PRI, initiator[ii], 0, NULL);
    }

    for (ii = 0; ii < 2; ii++)
    {
        /*
        ** Pass the offset into lliDir to the new task.
        */

#ifdef PROC_CODE
        CT_fork_tmp = (ii == 0) ? (ulong)"LL_TargetTask 1":
            (ulong)"LL_TargetTask 2";
#endif /* PROC_CODE */
        TASKCREATEPERM (LL_TargetTask, LL_TARGET_PRI, target[ii],
                        pHisSharedMem[ii+2], NULL);
    }

    /*
    ** Indicate that the initiator LLI Directory entries are set.
    */

    pMySharedMem->SHMSync = SYNCHRONIZE;


    /*
    ** Populate the two arrays, linkID and pLinkQueFunc, that are indexed by
    ** message function code. The function code, which defines the
    ** message type, determines the communication link to use to send
    ** the message, and determines the receiving que/task when receiving
    ** the message. The function codes are divided into ranges, and all
    ** codes within a range are handled the same. This could be checked on
    ** a per message basis with a series of 'if' statements, or by looping
    ** through an array (as link960.as does), but by populating thise
    ** two arrays now, we are trading off a little execution time during
    ** initialization for a more efficient running environment.
    **
    ** The function code ranges, and their respective communication links
    ** and reception queues, are:
    **  0x0000 - 0x00ff     FE -> BE    V$que         Cache to Virtual VRP/MRP
    **  0x0100 - 0x01fd     CCB -> BE   D$que         Define MRP
    **  0x01fe              CCB -> BE   FS$que        File System MRP
    **  0x01ff              CCB -> BE   M$que_hbeat   CCB to BE heartbeat
    **  0x0200 - 0x02ff     BE -> FE    D$que         Define MRP
    **  0x0300              FE -> CCB   EnqueLogEvent Logging MRP
    **  0x0301              BE -> CCB   EnqueLogEvent Logging MRP
    **  0x0302 - 0x04ff     FE -> BE    D$que         Define MRP
    **  0x0500 - 0x05fe     CCB -> FE   D$que         Define MRP
    **  0x05ff              CCB -> FE   M$que_hbeat   CCB to FE heartbeat
    **  0x0600 - 0x067f     BE -> FE    DLM$quesrp    BE SRPs to the FE
    **  0x0680 - 0x06ff     BE -> FE    WC_WRPQueue   BE WRP to the FE
    **  0x0700              CCB -> FE   DLM$quedrp    CCB to FE DLM DRP
    **  0x0701              FE -> CCB   EnqueDLMEvent FE DLM to CCB DRP
    */

    ii = 0;

    /* cache to Virtual VRP/MRP */
#ifdef BACKEND
    queptr = V$que;
#else
    queptr = 0;
#endif

    for ( ; ii <= 0xff; ii++)
    {
        linkID[ii] = FE2BE;
        pLinkQueFunc[ii] = queptr;
    }

    /* Define MRP */
#ifdef BACKEND
    queptr = D$que;
#else
    queptr = 0;
#endif

    for ( ; ii <= 0x1fd; ii++)
    {
        linkID[ii] = CCB2BE;
        pLinkQueFunc[ii] = queptr;
    }

    /* File System MRP */
    linkID[ii] = CCB2BE;
#ifdef BACKEND
    pLinkQueFunc[ii] = FS$que;
#endif
    ii++;

    /* CCB to BE heartbeat */
    linkID[ii] = CCB2BE;
#ifdef BACKEND
    pLinkQueFunc[ii] = M$que_hbeat;
#endif
    ii++;

    /* Define MRP */
#ifdef FRONTEND
    queptr = D$que;
#else
    queptr = 0;
#endif

    for ( ; ii <= 0x2ff; ii++)
    {
        linkID[ii] = BE2FE;
        pLinkQueFunc[ii] = queptr;
    }

    /* Logging MRP */
    linkID[ii] = FE2CCB;
#ifdef CCB_RUNTIME_CODE
    pLinkQueFunc[ii] = EnqueLogEvent;
#endif
    ii++;

    /* Logging MRP */
    linkID[ii] = BE2CCB;
#ifdef CCB_RUNTIME_CODE
    pLinkQueFunc[ii] = EnqueLogEvent;
#endif
    ii++;

    /* Define MRP */
#ifdef BACKEND
    queptr = D$que;
#else
    queptr = 0;
#endif

    for ( ; ii <= 0x4ff; ii++)
    {
        linkID[ii] = FE2BE;
        pLinkQueFunc[ii] = queptr;
    }

    /* Define MRP */
#ifdef FRONTEND
    queptr = D$que;
#else
    queptr = 0;
#endif

    for ( ; ii <= 0x5fe; ii++)
    {
        linkID[ii] = CCB2FE;
        pLinkQueFunc[ii] = queptr;
    }

    /* CCB to FE heartbeat */
    linkID[ii] = CCB2FE;
#ifdef FRONTEND
    pLinkQueFunc[ii] = M$que_hbeat;
#endif
    ii++;

    /* BE SRPs to the FE */
#ifdef FRONTEND
    queptr = DLM$quesrp;
#else
    queptr = 0;
#endif

    for ( ; ii <= 0x67f; ii++)
    {
        linkID[ii] = BE2FE;
        pLinkQueFunc[ii] = queptr;
    }

    /* BE WRPs to the FE */
#ifdef FRONTEND
    queptr = C_label_referenced_in_i960asm(WC_WRPQueue);
#else
    queptr = 0;
#endif

    for ( ; ii <= 0x6ff; ii++)
    {
        linkID[ii] = BE2FE;
        pLinkQueFunc[ii] = queptr;
    }

    /* CCB to FE DLM DRP */
    linkID[ii] = CCB2FE;
#ifdef FRONTEND
    pLinkQueFunc[ii] = DLM$quedrp;
#endif
    ii++;

    /* FE DLM to CCB DRP */
    linkID[ii] = FE2CCB;
#ifdef CCB_RUNTIME_CODE
    pLinkQueFunc[ii] = EnqueDlmEvent;
#endif

    if (ii != LARGEST_FUNC_CODE)
    {
        abort();
    }

    /*
    ** The boot task has completed initialization.
    */
    boot_sync = SYNCHRONIZE;
} /* LL_BootInit */


/**
******************************************************************************
**
**  @brief      Perform startup initialization of the Link Layer
**
**              This routine is called to initialize the Link Layer
**              during system start. A separate task (LL_BootInit) is started to
**              do the initialization itself so that other initializations
**              can also be progressing in parallel.
**
**  @return     none
**
******************************************************************************
**/

void LL_Init (void)
{
    /*
    ** Make sure the shared memory represents the correct structure version.
    */
    switch (pMySharedMem->SHMStructVersionID)
    {
        case SM_STRUCT_VERSION:
            /*
            ** Current! Everything is fine.
            */
            break;

        default:
            /*
            ** Unknown version number. Blow away the entire shared memory,
            ** and re-initialize.
            */
            memset (pMySharedMem, 0, sizeof(*pMySharedMem));
            pMySharedMem->SHMStructVersionID = SM_STRUCT_VERSION;
    }

    /*
    ** Initialize the synchronization states.
    */
    pMySharedMem->SHMSync = POWERUP;
    boot_sync = POWERUP;

    /*
    ** Initialize L_stattbl
    */
    L_stattbl = &LL_stattbl;

    /*
    ** Continue boot initialization (formerly a separate task).
    */
    LL_BootInit();
}


/**
******************************************************************************
**
**  @brief      Callback function from an LL_SendPacket call.
**
**              The callback function requested by the user call to
**              LL_SendPacket is made. Parameters passed to the user
**              callback function are return code, ILT ptr, MRP ptr,
**              and user supplied data. That is, the callback function
**              should be:
**                  void CallbackFunction (UINT32 returnCode, ILT *pILT,
**                                   MRP *pMRP, UINT32 userData).
**
**              This function then takes the ILT that was generated by the
**              LL_SendPacket call and releases it and the associated MRP.
**
**  @param      returnCode - Return code of the message
**  @param      pILT    - Pointer to the ILT for this message.
**
**  @return     none
**
******************************************************************************
**/

static void LL_SendPacketCompletion (UINT32 returnCode, ILT *pILT)
{
    VRPCMD              *pVRPCMD;
    MR_PKT              *pMRP;
    void                (*callback)(UINT32, struct ILT *, ...);
    UINT32              asmCallbackFunc;

    /*
    ** Get the MRP pointed to by the ILT
    */

    pVRPCMD = (VRPCMD *)&pILT->ilt_normal.w0;
    pMRP = (MR_PKT *)pVRPCMD->pvrVRP;

    /*
    ** Return to the previous ILT level. This is the top ILT level,
    ** and has the user values we need.
    */

    pILT--;

    /*
    ** If there is a user defined completion routine, call it.
    ** The callback variable in the ILT has the
    ** address of the callback routine, possibly OR'ed with BIT31. BIT31 is
    ** set if the callback function is in assembly language; it is clear if
    ** the callback function is in C. This distinction is important in the
    ** auto-converted code environment, because "magic sauce" must be applied
    ** to any calls into assembly language routines from C code.
    */

    callback = pILT->cr;
    asmCallbackFunc = (UINT32)callback & BIT31;
    callback = (void (*)(UINT32, struct ILT *, ...))((UINT32)callback & ~BIT31);
    if (callback != 0)
    {
#ifdef PROC_CODE
        if (asmCallbackFunc)
        {
            /*
            ** The callback variable had BIT31 set, so it is an assembly
            ** routine. Replace the callback address in the ILT with the
            ** actual address, and call the assembly language routine KernelDispatch.
            ** This will ensure the "magic sauce" is present.
            */

            pILT->cr = callback;
            KernelDispatch (returnCode, pILT, pMRP, pILT->ilt_normal.w0);
        }
        else
#endif /* PROC_CODE */
        {
            void (*ck)(UINT32, struct ILT *, MR_PKT *, unsigned int);
            /*
            ** The callback is written in C, or we are on the CCB. Call the
            ** function directly.
            */

            ck = pILT->cr;
            (*ck)(returnCode, pILT, pMRP, pILT->ilt_normal.w0);
        }
    }

    /*
    ** Release the MRP and ILT
    */

    put_vrp((VRP *)pMRP);
    put_ilt(pILT);
}


/**
******************************************************************************
**
**  @brief      Receive outbound message packets from a client and submit
**              them to the outbound queue.
**
**              This function is passed a pointer to an ILT to transmit
**              to another platform. The function code in the VRP/MRP is
**              used to determine which platform is the destination.
**
**              This function may block the calling task if the queue
**              has filled (see below).
**
**              The message is placed into an internal queue for the
**              LL_InitiatorTask. If the link to the requested platform is
**              not available, then the LL_InitiatorTask will not process
**              the messages in this queue, and the queue may fill up.
**              If the queue fills, then LL_QueueMessageToSend checks the
**              destination platform.
**                  - if the destination is the CCB, the message is put
**                      into the bit bucket, and if a completion routine
**                      was specified, LL_QueueMessageToSend calls it
**                      with an error condition.
**                  - if the destination is the FE or BE, then
**                      LL_QueueMessageToSend blocks the calling task
**                      until there is room in the queue for the
**                      message.
**
**  @param      pILT    - Pointer to the ILT to transmit.
**
**  @return     EC_OK           - Message successfully queued.
**  @return     EC_LINK_FAIL    - Message was not queued.
**
**  @attention  Note the difference between LL_QueueMessageToSend and
**              L$que in the auto-converted environment. If there is
**              a callback routine (completion routine) specified,
**              then if the client calls LL_QueueMessageToSend,
**              THE CALLBACK FUNCTION MUST BE IN C. If the client calls
**              L$que, then THE CALLBACK FUNCTION MUST BE IN ASSEMBLY.
**              Other than this, the two functions are identical.
**
******************************************************************************
**/

UINT32 LL_QueueMessageToSend (ILT *pILT)
{
    VRPCMD              *pVRPCMD;
    VRP                 *pVRP;
    UINT32              cmdCode, wait, retValue;
    LL_POSSIBLE_LINKS   llLink;
    LLI                 *pLLI;
#ifndef PROC_CODE
    void                (*callback)(UINT32, struct ILT *, ...);
#endif

#if KERNEL_DEBUG_5
    long long           tsc;
    UINT32              tsc_h, tsc_l;
#endif /* KERNEL_DEBUG_5 */

    /*
    ** Get the function code from the VRP (pointed to by the ILT)
    */

    pVRPCMD = (VRPCMD *)&pILT->ilt_normal.w0;
    pVRP = pVRPCMD->pvrVRP;
    cmdCode = pVRP->function;

    /*
    ** Find the LLI for the destination specified.
    */

    if (cmdCode > LARGEST_FUNC_CODE)
    {
        error08();
    }
    llLink = linkID[cmdCode];
    pLLI = pMySharedMem->pSHMLLIDir[llLink];

    /*
    ** Update stats outstanding and total VRP/MRP counter
    */

    L_stattbl->LL_OutCount++;
    L_stattbl->LL_TotOutCount++;

#if DEBUG_FLT_REC_LINK
    MSC_FlightRec (FR_L_QUE, (UINT32)pILT, (UINT32)pVRP, cmdCode);
#endif /* DEBUG_FLT_REC_LINK */

    /*
    ** If the target is the CCB, then we need to handle the case where the link
    ** is dead. We cannot suspend the task if the link is broken. If the
    ** QUEUE_CONTROL is full (and the target is CCB), we will fail the
    ** ILT and call its completion routine.
    **
    ** For a target of FE or BE, we wait until we can send the message.
    */

    switch (llLink)
    {
        case FE2CCB:
        case BE2CCB:
            wait = FALSE;
            break;
        case FE2BE:
        case CCB2FE:
        case CCB2BE:
        case BE2FE:
        default:
            wait = TRUE;
    }

    /*
    ** Put the request on the appropriate outbound queue.
    */

#if KERNEL_DEBUG_5
    tsc = get_tsc() >> 21;
    tsc_h = (UINT32)(tsc >> 32);
    tsc_l = (UINT32)tsc;
    pILT[1].w0 = tsc_h;
    pILT[1].w1 = tsc_l;
#endif /* KERNEL_DEBUG_5 */

    pILT[1].linux_val = llMessageCount++;
    if (llMessageCount == 0x7fffffff) { llMessageCount = 0;};

#ifdef LL_TRACE
    FPRINTF6 (stderr,"%sQ : %'14d[0x%x] => %s\n", L_MsgPrefix,
             pILT[1].linux_val, cmdCode, pLinkNames[llLink]);
#endif /* LL_TRACE */
    retValue = QueueILT (&LINK_QCS, pILT, pLLI->LLInitiatorQCSIndex, wait);

    /*
    ** A return failure will only happen if we didn't wait for queue space
    ** (i.e. only for a CCB target). If we got the failure, return an error,
    **  call the completion routine, and update the stats.
    */

    if (retValue == FALSE)
    {
        static time_t last_message_passing_failure_time = 0;
        time_t        now = time(0);        /* Current time in seconds. */

        /* Limit printed message to every 10 minutes. */
        if ((now - last_message_passing_failure_time) > 10*60)
        {
            last_message_passing_failure_time = now;
            fprintf(stderr, "%sMessage passing failed!!!\n", L_MsgPrefix);
        }
        pVRP->status = EC_LINK_FAIL;
#ifdef PROC_CODE
        /*
        ** Fix for CQT-13112 --
        ** In case of Q-Full event, the sendPacketCompletion (call back in the
        ** current ILT level is called in  separate I960 task context).  The
        ** SendPacketCompletion in turn calls the callback in its previous ILT
        ** level within this new stack.
        ** This is to avoid stack overflow that occur due to subsequent call
        ** back (the call back called by sendPacketCompletion) will in turn
        ** call many functions, in the current I960 task context that is trying
        ** to queue the packet.
        **
        ** Fix for CQT-21213 --
        ** Create a queue and task to empty queue for callback routines.
        ** Make sure that the ILT forward pointer is cleared.
        */
        pILT->fthd = NULL;
        if (LL_QueFullReqHandlingTask_PCB == NULL)
        {
           /*
            * Make sure a second task isn't created upon memory full condition.
            * Then set the queue head and tail pointers for the task to empty.
            */
           LL_QueFullReqHandlingTask_PCB = (PCB *)-1;
           LL_QueFullReqHandlingTask_ILT = pILT;
           LL_QueFullReqHandlingTask_ILT_tail = pILT;
           CT_fork_tmp = (unsigned long)"LL_QueFullReqHandlingTask";
           LL_QueFullReqHandlingTask_PCB = TaskCreate2(C_label_referenced_in_i960asm(LL_QueFullReqHandlingTask),
#ifdef CCB_RUNTIME_CODE
                      (UINT32)XK_GetPcb()->pc_pri
#else   /* CCB_RUNTIME_CODE */
                      (UINT32)K_xpcb->pc_pri
#endif  /* CCB_RUNTIME_CODE */
                                            );
        }
        else
        {
            /*
             * Task already exists, put new ILT on the queue.
             */
            if (LL_QueFullReqHandlingTask_ILT == NULL)
            {
                /*
                 * If task has taken everything off, but has not exited yet, set head.
                 */
                LL_QueFullReqHandlingTask_ILT = pILT;
            }
            else
            {
                /*
                 * Tack onto the tail of the list.
                 */
                LL_QueFullReqHandlingTask_ILT_tail->fthd = pILT;
            }
            LL_QueFullReqHandlingTask_ILT_tail = pILT;
        }
#else   /** CCB Case **/
        callback = pILT->cr;
        callback = (void (*)(UINT32, struct ILT *, ...))((UINT32)callback & ~BIT31);

        if (callback != 0)
        {
            {
                void (*ck)(UINT32, struct ILT *);
                /*
                ** The callback is written in C, and we are on the CCB. Call the
                ** function directly.
                */

                ck = pILT->cr;
                (*ck)(EC_LINK_FAIL, pILT);
            }
        }
#endif

        L_stattbl->LL_OutCount--;
        return EC_LINK_FAIL;
    }
    return EC_OK;
}

/**
*****************************************************************************/
/*Fix for CQT-13112 */
#ifdef PROC_CODE

void LL_QueFullReqHandlingTask(UNUSED UINT32 dummy1)
{
    void                (*callback)(UINT32, struct ILT *, ...);
    UINT32              asmCallbackFunc;
    ILT*                pILT1;

    while ((pILT1 = LL_QueFullReqHandlingTask_ILT) != NULL)
    {
        /*
         * Move the first pointer to the next ILT, and unlink ILT forward pointer.
         */
        LL_QueFullReqHandlingTask_ILT = pILT1->fthd;
        pILT1->fthd = NULL;

        /*
        ** The callback variable in the ILT has the
        ** address of the callback routine, possibly OR'ed with BIT31. BIT31 is
        ** set if the callback function is in assembly language; it is clear if
        ** the callback function is in C. This distinction is important in the
        ** auto-converted code environment, because "magic sauce" must be applied
        ** to any calls into assembly language routines from C code.
        */
        callback = pILT1->cr;
        asmCallbackFunc = (UINT32)callback & BIT31;
        callback = (void (*)(UINT32, struct ILT *, ...))((UINT32)callback & ~BIT31);
        if (callback != 0)
        {
            /*
            ** Note:- In the current implementation the following check infact
            **        is not needed, as the call back that is being called now
            **        is always sendPakcketCompletion() that is at current ILt
            **        level, and this routine is implmented in C, but not in
            **        assembly. But for future use (in case sendPacketCompletion
            **        or any other call back at this ILT, is implemented in asm)
            **        the following check is just kept like that.
            */
            if (asmCallbackFunc)
            {
                /*
                ** The callback variable had BIT31 set, so it is an assembly
                ** routine. Replace the callback address in the ILT with the
                ** actual address, and call the assembly language routine
                ** KernelDispatch. This will ensure the "magic sauce" is present.
                **
                ** Note:- Currently this case never exists. Because  this task is
                **        forked when Queue is FULL. The call back  that is being
                **        called now is always sendPacketCompletion() that is at
                **        current ILT level. And this routine is in C, not in asm.
                **        But for future use, this is just kept like that.
                */

                pILT1->cr = callback;
                KernelDispatch(EC_LINK_FAIL, pILT1, NULL, 0);
            }
            else
            {
                void (*ck)(UINT32, struct ILT *);
                /*
                ** The callback is written in C, Call the function directly.
                */

                ck = pILT1->cr;
                (*ck)(EC_LINK_FAIL, pILT1);
            }
        }
    }
    /*
     * Clear out the task pointers. Head of queue already NULL to be here. The
     * tail pointer should be nulled, as should the PCB address, so that this
     * task will be created again later, if needed.
     */
    LL_QueFullReqHandlingTask_ILT_tail = NULL;
    LL_QueFullReqHandlingTask_PCB = NULL;
}   /* End of LL_QueFullReqHandlingTask */
#endif /* PROC_CODE */

/*****************************************************************************/

/**
******************************************************************************
**
**  @brief      Create and send a message to another platform.
**
**              This function takes packet information from a user, allocates
**              an ILT and VRP/MRP and stuffs the user data into it.
**              A pointer to the input packet is passed along.
**
**              LL_QueueMessageToSend is called
**              to send the packet to the appropriate queue to go to the
**              appropriate platform.
**
**              Control returns to the calling function after the packet is
**              queued to be sent.
**
**              See LL_SendPacketCompletion for the parms passed to the users
**              callback function.
**
**  @param      pReq      - Pointer to message. This should NOT be on the stack!
**  @param      reqSize   - Message size in bytes.
**  @param      cmdCode   - Message function code.
**  @param      pRsp      - Pointer to buffer for response.
**  @param      rspLen    - Length of response buffer in bytes.
**  @param      pCallback - Pointer to callback function to execute on
**                          receipt of response (this may be NULL).
**  @param      param     - User defined value that is passed to the
**                          completion function as an argument when it
**                          is called.
**
**  @return     none
**
**  @attention  Note the difference between LL_SendPacket and
**              L$send_packet in the auto-converted environment. If there is
**              a callback routine (completion routine) specified,
**              then if the client calls LL_SendPacket,
**              THE CALLBACK FUNCTION MUST BE IN C. If the client calls
**              L$send_packet, then THE CALLBACK FUNCTION MUST BE IN ASSEMBLY.
**              Other than this, the two functions are identical.
**
******************************************************************************
**/

void LL_SendPacket(void *pReq, UINT32 reqSize, UINT32 cmdCode, void *pRsp,
                   UINT32 rspLen, void *pCallback, UINT32 param)
{
    ILT     *pILT;
    VRP     *pVRP;
    MR_PKT  *pMRP;
    VRPCMD  *pVRPCMD;
    UINT32  *pTo, *pFrom, *pEnd;

    /*
    ** If the link layer isn't set up yet, just drop this request.
    */

    if (pMySharedMem->SHMSync != SYNCHRONIZE)
    {
        fprintf(stderr, "=============================================\n"
                        "LL_SendPacket(%p,      /* request  */\n"
                        "              0x%08X,      /* req size */\n"
                        "              0x%08X,      /* cmd code */\n"
                        "              %p,      /* response */\n"
                        "              0x%08X,      /* rsp size */\n"
                        "              %p,      /* callback */\n"
                        "              0x%08X);     /* parm     */\n"
                        " SHARED MEM NOT SYNCHRONIZED!  DROPPING PKT!\n"
                        "=============================================\n",
                pReq, reqSize, cmdCode, pRsp, rspLen, pCallback, param);
        return;
    }

    /*
    ** Allocate an ILT and MRP. The routine allocates a VRP, which is large
    ** enough to hold any request (including an MRP). This allows the VRP to
    ** be saved and reused when we are through with it.
    */

    pILT = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, pILT);
#endif /* M4_DEBUG_ILT */
    pVRP = get_vrp();
#ifdef M4_DEBUG_VRP
CT_history_printf("%s%s:%u get_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, pVRP);
#endif /* M4_DEBUG_VRP */

    memset (pVRP, 0, VRPALLOC);

    /*
    ** Save the callback function, and bias the ILT forward.
    */

    pILT->cr = pCallback;
    pILT->ilt_normal.w0 = param;
    pILT++;

    /*
    ** Close the link and save a pointer to the common callback routine.
    */

    pILT->fthd = 0;
    pILT->cr = (void (*)(UINT32, struct ILT *, ...))LL_SendPacketCompletion;

    /*
    ** Link the MRP to the ILT, and fill in the MRP.
    */

    pVRPCMD = (VRPCMD *)&pILT->ilt_normal.w0;
    pVRPCMD->pvrVRP = pVRP;
    pMRP = (MR_PKT *)pVRP;
    pMRP->function = cmdCode;
    pMRP->version = 0;
    pMRP->pReqPCI = pMRP;
#ifndef PERF
        if (rspLen != 0 && pRsp!= NULL &&
            ((UINT32)pRsp < (UINT32)FRONT_END_PCI_START ||
             (UINT32)pRsp > (UINT32)BACK_END_PCI_START + (UINT32)SIZE_BE_LTH))
        {
            // Abort if not shared memory, and not a null pointer.
            abort();
        }
#endif /* PERF */
    pMRP->pRsp = pRsp;
    pMRP->rspLen = rspLen;
    pMRP->reqLen = reqSize;

    /*
    ** If the request is short enough to fit into the remaining space of the
    ** MRP, copy it into the MRP. NOTE THAT THIS IS NECESSARY BECAUSE THE CODE
    ** RELIES ON THE ABILITY TO REUSE THE BUFFERS FOR THESE SHORT MESSAGES!
    */

    if (reqSize > 0 && reqSize <= VRPAVAILABLE)
    {
        pTo = (UINT32 *)&pVRP[1];
        pMRP->pReq = pTo;
        pFrom = (UINT32 *)pReq;
        pEnd = pFrom + (reqSize + 3)/ 4;
        do
        {
            *pTo++ = *pFrom++;
        } while (pFrom < pEnd);
    }
    else
    {
#ifndef PERF
        if (reqSize != 0 && pReq!= NULL &&
            ((UINT32)pReq < (UINT32)FRONT_END_PCI_START ||
             (UINT32)pReq > (UINT32)BACK_END_PCI_START + (UINT32)SIZE_BE_LTH))
        {
            // Abort if not shared memory, and not a null pointer.
            abort();
        }
#endif /* PERF */
        pMRP->pReq = pReq;
    }

    /*
    ** Que the message for the Initiator Task to send.
    */

    LL_QueueMessageToSend (pILT);

}



#ifdef PROC_CODE

/**
******************************************************************************
**
**  @brief      Flag an errtrap condition to the CCB so it can create a
**              log entry.
**
**  @return     none
**
******************************************************************************
**/
void LL_Errtrap(void)
{
    LLI         *pMyLLI;

    /*
    ** Get addr of mailbox register that the proc writes to the CCB.
    */
#ifdef FRONTEND
    pMyLLI = pMySharedMem->pSHMLLIDir[FE2CCB];
#elif defined BACKEND
    pMyLLI = pMySharedMem->pSHMLLIDir[BE2CCB];
#else /* CCB_RUNTIME_CODE */
    return;
#endif /* FRONTEND / BACKEND */

    /*
    ** This is possible if the Link Layer has not been initialized.
    ** In this case, send a direct message to pam to deadloop.
    ** Reason for a deadloop, is that if this is hit during
    ** initialization, probably will hit again on a reboot.
    ** Possibly may not boot again...
    */
    if ( !pMyLLI )
    {
        errExit(ERR_EXIT_DEADLOOP);
        return;
    }

    /*
    ** Set the error trap sync state.
    */
    pMyLLI->LLInitiatorSync = ERR_TRAP;

    if (TaskGetState (pMyLLI->pLLTargetPCB) == PCB_WAIT_LINUX_TARG_IN)
    {
        TaskSetState(pMyLLI->pLLTargetPCB, PCB_READY);
        (*pMyLLI->pLLTargetUpCounter)++;
        FORCE_WRITE_BARRIER;
        if (*pMyLLI->pLLTargetSleep)
        {
            CT_ioctl(CT_xiofd, XIO3D_SENDEVT, pMyLLI->LLTargetEvent);
        }
    }

    return;
}
#endif /* PROC_CODE */

/*#ifdef CCB_RUNTIME_CODE*/
#if defined(CCB_RUNTIME_CODE) || defined (DOXYGEN)
/**
******************************************************************************
**
**  @brief      Completion handler for LL_Logtrap.
**
**              This releases the ILT and VRP allocated by LL_Logtrap.
**
**  @param      returnCode - Message return status (unused)
**  @param      pILT - ILT to be released
**
**  @return     none
**
******************************************************************************
**/
static void LL_LogtrapComp(UNUSED UINT32 returnCode, ILT *pILT)
{
    put_vrp((VRP *)pILT->ilt_normal.w0);
    put_ilt(pILT);
}

/**
******************************************************************************
**
**  @brief      Generate a log entry from an errtrap condition.
**              log entry.
**
**  @param      pMyLLI - LLI for the interface
**
**  @return     none
**
******************************************************************************
**/
static void LL_Logtrap( LLI *pMyLLI )
{
    ILT                *pILT;
    VRP                *pVRP;
    MR_PKT             *pMRP;
    LOG_MRP            *pERRDATA;
    UINT32              trapAddr    = TRAPADDR_OFFSET;

    /* --- Allocate ILT and MRP for a log entry */
    pILT = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, pILT);
#endif /* M4_DEBUG_ILT */
    pVRP = get_vrp();
#ifdef M4_DEBUG_VRP
CT_history_printf("%s%s:%u get_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, pVRP);
#endif /* M4_DEBUG_VRP */

    /* Save the callback function, and close the link. */
    pILT->fthd = 0;
    pILT->cr = (void (*)(UINT32, struct ILT *, ...))LL_LogtrapComp;

    /* Link the MRP to the ILT, and fill in the MRP. */
    pILT->ilt_normal.w0 = (UINT32)pVRP;
    pMRP = (MR_PKT *)pVRP;

//    pERRDATA = (LOG_MRP*)((UINT32)pMRP + sizeof(VRP));
// If run out of memory, dead lock might happen, but we are going down anyway.
    pERRDATA = (LOG_MRP *)p_XK_MallocWC(sizeof(*pERRDATA), __FILE__, __LINE__);
    pERRDATA->mleEvent = LOG_ERR_TRAP;
    pERRDATA->mleLength = sizeof(LOG_ERROR_TRAP_PKT);

    pMRP->pReq = pERRDATA;

    /* Get the error trap base address */
    if ( pMyLLI->LLmyLink == FE2CCB )
    {
        trapAddr += FE_BASE_ADDR;
    }
    else
    {
        trapAddr += BE_BASE_ADDR;
    }

    /* Copy the data from the error trapped process to local memory. */
    memcpy((UINT8*)pERRDATA->mleData, (UINT8*)trapAddr, sizeof(LOG_ERROR_TRAP_PKT));

    /* Send it to the asynchronous event queue */
    EnqueueILT (EnqueLogEvent, pILT, (void*)LL_LogtrapComp);

    /*
    ** Force the remote proc state to look
    ** like it's uninitialized. This stops
    ** the logs from being created forever.
    */
    pMyLLI->LLInitiatorSync = 0xDEAD0000;
}

#endif /* CCB_RUNTIME_CODE */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
