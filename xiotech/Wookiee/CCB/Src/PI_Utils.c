/* $Id: PI_Utils.c 159129 2012-05-12 06:25:16Z marshall_midden $ */
/**
******************************************************************************
**
**  @file   PI_Utils.c
**
**  @brief  Packet Interface Utility Functions
**
**  This files implements the functions from cpsutils.c.
**
**  Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "PI_Utils.h"
#include "AsyncEventHandler.h"
#include "CmdLayers.h"
#include "CacheLoop.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "idr_structure.h"
#include "kernel.h"
#include "LL_LinuxLinkLayer.h"
#include "LL_Stats.h"
#include "logdef.h"
#include "logging.h"
#include "timer.h"
#include "MR_Defs.h"
#include "mutex.h"
#include "PacketInterface.h"
#include "PacketStats.h"
#include "PortServer.h"
#include "quorum_utils.h"
#include "rm_val.h"
#include "rtc.h"
#include "trace.h"
#include "XIO_Std.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"

extern void LL_SendPacket(void *pReq, UINT32 reqSize, UINT32 cmdCode,
                          void *pRsp, UINT32 rspLen, void *pCallback, UINT32 param);

/*****************************************************************************
** Private defines
*****************************************************************************/
#define INIT_PROC_COMM_TIMEOUT      (5*60)      /* 5 minutes (defined in seconds) */
#define QUICK_TEST_TIMEOUT          2000        /* 2 seconds (defined in milli-seconds) */

#define MRP_INIT_REQUEST_FLAG       0x80000000

#define MRP_TYPE_MASK               0x00000F00
#define CCB_TO_BE                   0x00000100
#define CCB_TO_FE                   0x00000500
#define CCB_TO_DLM                  0x00000700

#define BE_READY                    0x01
#define FE_READY                    0x02
#define INIT_COMPL                  0x04

#define DELAYED_FREE_LIST_SIZE      1000        /* number of saved pointers */
#define DELAYED_FREE_RETRY_TIME     (15*1000)   /* number of milli-secs between tries */
#define BLOCKED_TASK_RETRY_TIME     (5*1000)    /* number of milli-secs between tries */

#define COMMAND_RECORD_SIZE         sizeof(PI_COMMAND_RECORD)

#define CRT_SlotClear(a)            memset(&commandRecordTable[a], 0x00, COMMAND_RECORD_SIZE)
#define CRT_SlotInUse(a)            (commandRecordTable[a].fence[0] == 0xAA)
#define CRT_FenceSet(a)             memset(commandRecordTable[a].fence, 0xAA, 16)
#define CRT_GetCallerPCB(a)         commandRecordTable[a].callerPCB
#define CRT_SetCallerPCB(a, b)      commandRecordTable[a].callerPCB = (b)
#define CRT_GetInBuffer(a)          commandRecordTable[a].commandBufferIn
#define CRT_SetInBuffer(a,b)        commandRecordTable[a].commandBufferIn = (b)
#define CRT_GetOutBuffer(a)         commandRecordTable[a].commandBufferOut
#define CRT_SetOutBuffer(a,b)       commandRecordTable[a].commandBufferOut = (b)
#define CRT_GetState(a)             commandRecordTable[a].state
#define CRT_SetState(a,b)           commandRecordTable[a].state = (b)
#define CRT_SetStateTMO(a)          commandRecordTable[a].state = PI_COMMAND_RECORD_STATE_TIMEOUT
#define CRT_GetCompletion(a)        commandRecordTable[a].completion
#define CRT_SetCompletionOK(a)      commandRecordTable[a].completion = PI_COMMAND_RECORD_COMPLETION_COMPLETE
#define CRT_SetCompletionTMO(a)     commandRecordTable[a].completion = PI_COMMAND_RECORD_COMPLETION_TIMEOUT
#define CRT_TimeoutActionFree(a)    (commandRecordTable[a].timeoutAction == PI_COMMAND_RECORD_TIMEOUT_ACTION_OUTPUT_FREE)
#define CRT_SetTimeoutAction(a,b)   commandRecordTable[a].timeoutAction = (b)
#define CRT_GetCommandCode(a)       commandRecordTable[a].commandCode
#define CRT_SetCommandCode(a,b)     commandRecordTable[a].commandCode = (b)
#define CRT_GetTimeout(a)           commandRecordTable[a].timeout
#define CRT_SetTimeout(a,b)         commandRecordTable[a].timeout = (b)

#define SystemMS()                  ((K_t1cnt * 2083) / 100)

/*****************************************************************************
** Private variables
*****************************************************************************/
typedef struct
{
    UINT32      blocked;
    UINT32      count;
    void       *list[DELAYED_FREE_LIST_SIZE * 4];
} DELAYED_FREE_LIST;

#define DFL_BLOCKED_BE          0x1
#define DFL_BLOCKED_BE_FSYS     0x2
#define DFL_BLOCKED_FE          0x1

/*
 * Note: no mutex is used over these variables because no exchanges
 * are being done while they are being examined or modified.  Be careful
 * when modifying the code that uses these variables so that this rule
 * is not violated.
 */
static UINT32 cleanupTaskRunning = 0;
static DELAYED_FREE_LIST be;
static DELAYED_FREE_LIST fe;

static PCB *gpBlockedTask = NULL;

/*
** The command record table is used for passing PI_COMMAND_RECORD's between
** PI_ExecMRP(), and PI_MRPCallback(). Slot numbers are passed, not pointers.
*/

static UINT32 gMRPTimeout = MRP_STD_TIMEOUT;    /* Global timeout value */
static UINT32 gIPCTimeout = 0;  /* Global timeout value */

static PCB *gMRPTimeoutTaskPCB = NULL;  /* Global MRP Timeout Task PCB  */
static UINT8 gProcInitFlags = 0;        /* global flag indicating FE/BE readiness */

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
PI_COMMAND_RECORD commandRecordTable[COMMAND_RECORD_TABLE_SIZE];

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static INT32 MRPCmd2Processor(UINT32 mrpCmd);
static void BlockedTaskStart(void);
static void BlockedTask(TASK_PARMS *parms);
static void DelayedFreeCleanupTask(TASK_PARMS *parms);
static void EnqueueDelayedFreePtr(INT32 proc, void *ptr);
static INT32 GetCommandRecordSlot(void);
static void PI_MRPCallback(UINT32 errorCode, void *ILT, void *commandBufferInCopy,
                           INT32 commandRecordSlot);
static UINT8 GetTypeFromStats(PI_STATS_LOOPS_RSP *pStatsLoops);
static NORETURN void PI_MRPTimeoutTask(UNUSED TASK_PARMS *parms);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function Name:   GetCommandRecordSlot
**
** Comments:        Returns the next open slot number in the commandRecordTable,
**                  -1 if there are none.
**
**--------------------------------------------------------------------------*/
static INT32 GetCommandRecordSlot(void)
{
    static UINT32 count = 0;
    UINT32      start = count;

    while (TRUE)
    {
        if (!CRT_SlotInUse(start))
        {
            count = start;
            break;
        }
        else
        {
            start++;
        }

        if (start == COMMAND_RECORD_TABLE_SIZE)
        {
            start = 0;
        }

        if (start == count)
        {
            return -1;
        }
    }

    count++;
    if (count == COMMAND_RECORD_TABLE_SIZE)
    {
        count = 0;
    }

    return start;
}

/*----------------------------------------------------------------------------
** Function Name:   PI_MRPTimeoutTask
**
** Comments:        This is a task that will run and check for MRP
**                  timeouts in the command record table.  When run,
**                  it will timeout any MRP's with a time limit that
**                  has expired.  When an MRP has timed out we will
**                  set the state and completion flags to TIMEOUT
**                  and then wakes the caller process.  If there are
**                  outstanding MRP's that have not timed out, we
**                  will calculate which of them has the shortest
**                  time in which to timeout, and then go to sleep
**                  for that length of time.  If there are no
**                  outstanding MRP's we will simply go into a wait
**                  state.  In either case, PI_ExecuteMRP will wake
**                  us up whenever a new MRP is issued.
**
**--------------------------------------------------------------------------*/
NORETURN void PI_MRPTimeoutTask(UNUSED TASK_PARMS *parms)
{
    unsigned long commandCode = 0;
    void       *callerPCB = NULL;
    INT32       commandRecordSlot = 0;
    UINT32      timestamp = 0;
    UINT32      timeout = 0;
    UINT32      sleepTime = 0;

    dprintf(DPRINTF_EXECMRP, "PI_MRPTimeoutTask - Starting MRP Timeout task.\n");

    /* Run forever. */
    while (TRUE)
    {
        /*
         * ATOMIC OPERATION BEGIN
         */

        /* Get the current system milliseconds. */
        timestamp = SystemMS();

        /* Reset our sleepTime to 0. */
        sleepTime = 0;

        /*
         * dprintf(DPRINTF_EXECMRP, "PI_MRPTimeoutTask - Running Current"
         * " System ms: %lu.\n", timestamp);
         */

        /* Loop through command table and look for tasks that have timed out. */
        for (commandRecordSlot = 0;
             commandRecordSlot < COMMAND_RECORD_TABLE_SIZE; ++commandRecordSlot)
        {
            /* If the slot is in use, check for a timeout. */
            if ((CRT_SlotInUse(commandRecordSlot)) &&
                (CRT_GetState(commandRecordSlot) == PI_COMMAND_RECORD_STATE_EXECUTING))
            {
                /*
                 * Get the time in milliseconds in which
                 * this MRP is to be timed out.
                 */
                timeout = CRT_GetTimeout(commandRecordSlot);

                /*
                 * dprintf(DPRINTF_EXECMRP, "PI_MRPTimeoutTask - Found Record Slot %d, timeout %lu, timestamp %lu.\n",
                 *         commandRecordSlot, timeout, timestamp);
                 */

                if (timeout != TMO_NONE)
                {
                    /*
                     * If this MRP has timed out set it appropriately,
                     * and then wake the callerPCB.
                     */
                    if (timestamp >= timeout)
                    {
                        /*
                         * Now, if we fault, this will show what
                         * command got hammered.  This will be the
                         * PCB of the calling process that was woken.
                         */
                        commandCode = CRT_GetCommandCode(commandRecordSlot);
                        callerPCB = CRT_GetCallerPCB(commandRecordSlot);

                        dprintf(DPRINTF_EXECMRP, "PI_MRPTimeoutTask - Timeout occured, waking up task in slot %d.\n",
                                commandRecordSlot);

                        /* Set the command record state and completion to TIMEOUT. */
                        CRT_SetStateTMO(commandRecordSlot);
                        CRT_SetCompletionTMO(commandRecordSlot);

                        /* Wake the calling process -- we know that it is PCB_WAIT_MRP. */
#ifndef PERF
if (TaskGetState(CRT_GetCallerPCB(commandRecordSlot)) != PCB_WAIT_MRP)
{
  dprintf(DPRINTF_DEFAULT, "%s:%u:%s PCB_WAIT_MRP not state of %p -- it is %u\n", __FILE__, __LINE__, __func__, CRT_GetCallerPCB(commandRecordSlot), TaskGetState(CRT_GetCallerPCB(commandRecordSlot)));
}
#endif  /* PERF */
                        TaskSetState(CRT_GetCallerPCB(commandRecordSlot), PCB_READY);
                    }

                    /*
                     * Else we have outstanding MRP's that have not
                     * timed out.  Calculate the shortest time until
                     * the next MRP is set to time out.
                     */
                    else if ((sleepTime == 0) || ((timeout - timestamp) <= sleepTime))
                    {
                        sleepTime = timeout - timestamp;
                    }
                }
            }
        }

        /*
         * ATOMIC OPERATION END
         */

        /*
         * If sleepTime is 0, there are no outstanding tasks.
         * Put ourselves in a wait state.
         */
        if (sleepTime == 0)
        {
            /*
             * dprintf(DPRINTF_EXECMRP, "PI_MRPTimeoutTask - No outstanding MRP's, going into wait state.\n");
             */
            TaskSetMyState(PCB_NOT_READY);
            TaskSwitch();
        }

        /*
         * Else we will sleep the minimum time
         * until the next MRP timeout is due.
         */
        else
        {
            /*
             * dprintf(DPRINTF_EXECMRP, "PI_MRPTimeoutTask - Outstanding MRP's, "
             * "sleeping %dms.\n", sleepTime);
             */
            /*
             * Sleep for sleepTime milliseconds.
             */
            TaskSleepMS(sleepTime);
        }

        /*
         * dprintf(DPRINTF_EXECMRP, "PI_MRPTimeoutTask - Woke up.\n");
         */
    }
}

/**
******************************************************************************
**
**  @brief  Handle link layer completions
**
**  @param  errorCode           - errorCode from link layer
**  @param  ILT                 - Pointer to ILT that was completed
**  @param  commandBufferInCopy - Pointer to some buffer
**  @param  commandRecordSlot   - commandRecordTable index to complete
**
**  @return none
**
******************************************************************************
**/
static void PI_MRPCallback(UNUSED UINT32 errorCode, UNUSED void *ILT,
                   UNUSED void *commandBufferInCopy, INT32 commandRecordSlot)
{
    UINT8       status;
    UINT32     *pCmdBufferOut;

    /*
     * Range check the slot number, and verify still in use -- which is in
     * itself "wrong" by design -- but better than what was here before.
     */
    if (commandRecordSlot < 0 ||
        commandRecordSlot >= COMMAND_RECORD_TABLE_SIZE ||
        !CRT_SlotInUse(commandRecordSlot))
    {
        dprintf(DPRINTF_EXECMRP, "%s: \"commandRecordSlot\" invalid (%d)\n",
                __func__, commandRecordSlot);
        return;
    }

    switch (CRT_GetState(commandRecordSlot))
    {
    case PI_COMMAND_RECORD_STATE_EXECUTING:
        /*
         * The link layer called this callback function before the
         * command timed out.  Set the state to completed, wake the
         * calling process and set completion status.
         */
        CRT_SetState(commandRecordSlot, PI_COMMAND_RECORD_STATE_COMPLETE);
        /* We know that the process is in PCB_WAIT_MRP state. */
#ifndef PERF
if (TaskGetState(CRT_GetCallerPCB(commandRecordSlot)) != PCB_WAIT_MRP)
{
  dprintf(DPRINTF_DEFAULT, "%s:%u:%s PCB_WAIT_MRP not state of %p -- it is %u\n", __FILE__, __LINE__, __func__, CRT_GetCallerPCB(commandRecordSlot), TaskGetState(CRT_GetCallerPCB(commandRecordSlot)));
}
#endif  /* PERF */
        TaskSetState(CRT_GetCallerPCB(commandRecordSlot), PCB_READY);
        CRT_SetCompletionOK(commandRecordSlot);
        break;

    case PI_COMMAND_RECORD_STATE_TIMEOUT:
        /*
         * Pull out the status for the command so it can be displayed
         * in the log message.  For CCB_TO_DLM, the status is in the
         * first byte, for all others it is in the 3rd byte.
         */
        pCmdBufferOut = (UINT32 *)CRT_GetOutBuffer(commandRecordSlot);

        if ((CRT_GetCommandCode(commandRecordSlot) & MRP_TYPE_MASK) != CCB_TO_DLM)
        {
            status = (UINT8)((*pCmdBufferOut) >> 24);
        }
        else
        {
            status = (UINT8)((*pCmdBufferOut));
        }

        /*
         * This command has already been timed out.  Free the output
         * buffer and the command record so we don't leak memory.
         */
        dprintf(DPRINTF_DEFAULT, "MRP TIMEOUT CALLBACK! cmd: 0x%X, status: 0x%x\n",
                   CRT_GetCommandCode(commandRecordSlot), status);

        TraceEvent(TRACE_MRP_TIMEOUT_CALLBACK,
                   CRT_GetCommandCode(commandRecordSlot));

        /*
         * We want to check and make sure that the caller
         * wants us to free the memory on a timeout.
         * They could possibly be using a static allocation.
         */
        if (CRT_TimeoutActionFree(commandRecordSlot))
        {
            Free(CRT_GetOutBuffer(commandRecordSlot));
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "%s: Skipping free on static buffer!\n",
                    __func__);
        }

        CRT_SlotClear(commandRecordSlot);

        /*
         * Startup the blocked task to make sure the blocked flags
         * are cleared.
         */
        BlockedTaskStart();
        break;
    }
}


/**
******************************************************************************
**
**  @brief  Set blocked flag as appropriate
**
**  @param  cmd     - Command code
**  @param  proc    - Either BE_READY or FE_READY
**
**  @return none
**
******************************************************************************
**/
static void set_blocked_flag(UINT32 cmd, UINT8 proc)
{
    switch (proc)
    {
        /*
         * For file system operations, set the DFL_BLOCKED_BE_FSYS
         * bit.
         *
         * For operations other than BE heartbeats, set the
         * DFL_BLOCKED_BE bit.
         */
    case BE_READY:
        if (cmd == MRFSYSOP || cmd == MRNOPFSYS)
        {
            if (!(be.blocked & DFL_BLOCKED_BE_FSYS))
            {
                be.blocked |= DFL_BLOCKED_BE_FSYS;
                TraceEvent(TRACE_MRP_BE_Q_FREE + be.blocked, be.count);

                dprintf(DPRINTF_DEFAULT, "MRP BE FSYS BLOCKED (cmd: 0x%x, 0x%x)\n",
                           cmd, be.blocked);
            }
        }
        else if (cmd != MRBEHBEAT)
        {
            if (!(be.blocked & DFL_BLOCKED_BE))
            {
                be.blocked |= DFL_BLOCKED_BE;
                TraceEvent(TRACE_MRP_BE_Q_FREE + be.blocked, be.count);

                dprintf(DPRINTF_DEFAULT, "MRP BE MAIN BLOCKED (cmd: 0x%x, 0x%x)\n",
                           cmd, be.blocked);
            }
        }
        break;

        /*
         * For operations other than FE heartbeats, set the
         * DFL_BLOCKED_FE bit.
         */
    case FE_READY:
        if (cmd != MRFEHBEAT)
        {
            if (!(fe.blocked & DFL_BLOCKED_FE))
            {
                fe.blocked |= DFL_BLOCKED_FE;
                TraceEvent(TRACE_MRP_FE_Q_FREE + fe.blocked, fe.count);

                dprintf(DPRINTF_DEFAULT, "MRP FE MAIN BLOCKED (cmd: 0x%x, 0x%x)\n",
                           cmd, fe.blocked);
            }
        }
        break;
    }
}


/**
******************************************************************************
**
**  @brief  Clear blocked flag as appropriate
**
**  @param  cmd     - Command code
**  @param  proc    - Either BE_READY or FE_READY
**
**  @return none
**
******************************************************************************
**/
static void clear_blocked_flag(UINT32 cmd, UINT8 proc)
{
    switch (proc)
    {
        /*
         * For file system operations, clear the DFL_BLOCKED_BE_FSYS
         * bit.
         *
         * For operations other than BE heartbeats, clear the
         * DFL_BLOCKED_BE bit.
         */
    case BE_READY:
        if (cmd == MRFSYSOP || cmd == MRNOPFSYS)
        {
            if (be.blocked & DFL_BLOCKED_BE_FSYS)
            {
                be.blocked &= ~DFL_BLOCKED_BE_FSYS;
                TraceEvent(TRACE_MRP_BE_Q_FREE + be.blocked, be.count);

                dprintf(DPRINTF_DEFAULT, "MRP BE FSYS NOT BLOCKED (cmd: 0x%x, 0x%x)\n",
                           cmd, be.blocked);
            }
        }
        else if (cmd != MRBEHBEAT)
        {
            if (be.blocked & DFL_BLOCKED_BE)
            {
                be.blocked &= ~DFL_BLOCKED_BE;
                TraceEvent(TRACE_MRP_BE_Q_FREE + be.blocked, be.count);

                dprintf(DPRINTF_DEFAULT, "MRP BE MAIN NOT BLOCKED (cmd: 0x%x, 0x%x)\n",
                           cmd, be.blocked);
            }
        }
        break;

        /*
         * For operations other than FE heartbeats, clear the
         * DFL_BLOCKED_FE bit.
         */
    case FE_READY:
        if (cmd != MRFEHBEAT)
        {
            if (fe.blocked & DFL_BLOCKED_FE)
            {
                fe.blocked &= ~DFL_BLOCKED_FE;
                TraceEvent(TRACE_MRP_FE_Q_FREE + fe.blocked, fe.count);

                dprintf(DPRINTF_DEFAULT, "MRP FE MAIN NOT BLOCKED (cmd: 0x%x, 0x%x)\n",
                           cmd, be.blocked);
            }
        }
        break;
    }
}


/**
******************************************************************************
**
**  @brief  Send a command to the Proc (with a designated timeout).
**
**  The high bit on 'timeout_mS' indicates an initialization
**  request.  Initialization requests occur at power up to
**  determine when a processor finally comes ready.
**
**  The following rules are neither complete not accurate.
**  You have been warned!
**
**  PI_ExecuteMRP rules:
**    - INPUT PACKET
**       - When caller is done with the input packet
**         free using "free"
**       - If input packet contains a PCI address and
**         the MRP timed out the PCI buffer must be
**         freed using "DelayedFree".
**    - OUTPUT PACKET
**       - If the MRP output packet is statically allocated, then
**         timeoutAction must equal
**             PI_COMMAND_RECORD_TIMEOUT_ACTION_OUTPUT_NONE
**       - If the MRP output packet is dynamically allocated, then
**         timeoutAction must equal
**             PI_COMMAND_RECORD_TIMEOUT_ACTION_OUTPUT_FREE
**       - If the MRP return code is TIMEOUT the output
**         packet must not be freed.  The free will be
**         done by the MRP Timeout handling.
**       - If the MRP return code is not TIMEOUT and
**         the output packet was dynamically allocated,
**         then the caller must free the output packet using
**         "free".
**
**  @param  commandBufferIn         - Pointer to command buffer
**  @param  commandBufferInSize     - Size of command buffer
**  @param  commandCode             - Command code
**  @param  commandBufferOut        - Pointer to command response buffer
**  @param  commandBufferOutSize    - Size of command response buffer
**  @param  timeoutMS               - Timeout in milliseconds
**  @param  timeoutAction           - code for timeout action
**
**  @return PI_GOOD, PI_ERROR, PI_TIMEOUT or one of the other PI_ return codes.
**
**  @attention  There are rules that need to be followed for the buffers
**              passed into this function.  The rules are stated above
**              and must be followed to ensure system integrity.
**
**  @attention  These rules are neither complete nor completely accurate!
**
******************************************************************************
**/
INT32 PI_ExecuteMRP(void *commandBufferIn, UINT32 commandBufferInSize,
                    UINT32 commandCode, void *commandBufferOut,
                    UINT32 commandBufferOutSize, UINT32 timeoutMS, UINT8 timeoutAction)
{
    INT32       commandRecordSlot;
    UINT32     *ptrCmdBufferOut;
    UINT16      rc = PI_GOOD;
    UINT8       status;
    UINT8       testF;
    bool        supressErrorLog;

#ifdef FOLLOW_MRP_EXECUTION
if (commandCode != 0x11e && commandCode != 0x1ff && commandCode != 0x5ff) {
  dprintf(DPRINTF_DEFAULT, "%s:%u:%s commandCode=0x%x (%d)\n", __FILE__, __LINE__, __func__, commandCode, commandCode);
}
#endif  /* FOLLOW_MRP_EXECUTION */

    /* Check for the supressErrorLog flag (top bit in the command code). */
    supressErrorLog = (commandCode & MRP_SUPRESS_ERROR_LOG_FLAG) != 0;

    commandCode &= ~MRP_SUPRESS_ERROR_LOG_FLAG; /* Remove flag from command */

    /* Determine the destination of the MRP request. */
    switch (MRPCmd2Processor(commandCode))
    {
    case PROCESS_BE:
        testF = BE_READY;
        break;

    case PROCESS_FE:
        testF = FE_READY;
        break;

    default:
        return PI_ERROR;
    }

    /*
     * If this is an initialization packet, then blow through. Otherwise
     * hold up here waiting for InitProcessorComm() to complete.
     */
    if ((timeoutMS & MRP_INIT_REQUEST_FLAG) == 0)
    {
        /*
         * Wait here for the requested processor to come ready.
         * If it didn't come ready during the initialization phase,
         * its hozed -- exit with error.
         */
        if ((gProcInitFlags & testF) == 0)
        {
            if ((gProcInitFlags & INIT_COMPL))
            {
                return PI_ERROR;
            }

            while ((gProcInitFlags & INIT_COMPL) == 0)
            {
                TaskSleepMS(300);
            }
        }
    }
    else
    {
        timeoutMS &= ~MRP_INIT_REQUEST_FLAG;
    }

    /*
     * Allocate memory for a command record for this command.
     * Fill in the necessary information from the input parms.
     */
    commandRecordSlot = GetCommandRecordSlot();
    if (commandRecordSlot < 0)
    {
        dprintf(DPRINTF_DEFAULT, "%s: commandRecordTable is FULL. Aborting.\n",
                    __func__);

        return PI_ERROR;
    }

    /*
     * Setup the command record slot with the information for this
     * particular request.
     */
    CRT_FenceSet(commandRecordSlot);
    CRT_SetState(commandRecordSlot, PI_COMMAND_RECORD_STATE_EXECUTING);
    CRT_SetInBuffer(commandRecordSlot, commandBufferIn);
    CRT_SetOutBuffer(commandRecordSlot, commandBufferOut);
    CRT_SetCallerPCB(commandRecordSlot, XK_GetPcb());
    CRT_SetCommandCode(commandRecordSlot, commandCode);
    CRT_SetTimeoutAction(commandRecordSlot, timeoutAction);

    /*
     * The timeout field in the command record entry holds the system time
     * at which this request should timeout.  If a timeout value is specified
     * (not TMO_NONE) then calculate the system time for the timeout to
     * occur.  If the system time happens to result in TMO_NONE then simply
     * add one to make sure the timeout value does not become TMO_NONE by
     * accident.
     */
    if (timeoutMS != TMO_NONE)
    {
        /*
         * We have a timeout value, add the current system MS to it to
         * get the timeout system value.
         */
        timeoutMS += SystemMS();

        /*
         * If the calculation happens to result in TMO_NONE simply
         * add one to it to make sure they are not equal.
         */
        if (timeoutMS == TMO_NONE)
        {
            ++timeoutMS;
        }
    }

    /* Save the timeout value in the command record entry. */
    CRT_SetTimeout(commandRecordSlot, timeoutMS);

    /* Issue the request to TBolt. */
    TraceEvent(TRACE_MRP_START, commandCode);
    IncPacketCount(PACKET_TYPE_MRP, commandCode);

    LL_SendPacket(commandBufferIn, commandBufferInSize, commandCode,
                  commandBufferOut, commandBufferOutSize, PI_MRPCallback,
                  commandRecordSlot);

    /* Wake up the MRP Timeout Task */
    if (TaskGetState(gMRPTimeoutTaskPCB) != PCB_READY)
    {
        TaskReadyState(gMRPTimeoutTaskPCB);
    }

wait:
    /* Put this process to sleep. It will wake up on timeout or completion. */
    TaskSetMyState(PCB_WAIT_MRP);
    TaskSwitch();

    if (!CRT_SlotInUse(commandRecordSlot))
    {
        dprintf(DPRINTF_DEFAULT, "%s: Command record slot %d unexpectedly "
                "freed!\n", __func__, commandRecordSlot);

        TraceEvent(TRACE_MRP + PI_TIMEOUT, commandCode);

        return PI_TIMEOUT;
    }

    UINT8   completion = CRT_GetCompletion(commandRecordSlot);
    UINT8   state = CRT_GetState(commandRecordSlot);

    if ((state != PI_COMMAND_RECORD_STATE_TIMEOUT &&
        state != PI_COMMAND_RECORD_STATE_COMPLETE))
    {
        dprintf(DPRINTF_DEFAULT, "%s: Incomplete completion, slot=%d, "
                "state=%d, compl=%d, cmd=0x%04X, pcb=%p\n", __func__,
                commandRecordSlot, state, completion, commandCode, XK_GetPcb());
        goto wait;
    }

    /*
     * Release the command record. If it wasn't timed out, it's no longer
     * needed.  DO NOT release the output buffers here, they're needed
     * by the caller.
     */
    if (CRT_GetCompletion(commandRecordSlot) != PI_COMMAND_RECORD_COMPLETION_TIMEOUT)
    {
        CRT_SlotClear(commandRecordSlot);

        /*
         * Check return code from the MRP.  We have to play some tricks here
         * since commandBufferOut is a void pointer.  The status is always
         * in the high byte of the first UINT32.  So cast the pointer and
         * shift to get the status byte. For a DRP, the status is in the
         * low byte.
         */
        ptrCmdBufferOut = (UINT32 *)commandBufferOut;

        if ((commandCode & MRP_TYPE_MASK) != CCB_TO_DLM)
        {
            status = (UINT8)((*ptrCmdBufferOut) >> 24);
        }
        else
        {
            status = (UINT8)((*ptrCmdBufferOut));
        }

        if (status != GOOD)
        {
            rc = PI_ERROR;

            /* Check to see if error log is to be supressed. */
            if (!supressErrorLog)
            {
                dprintf(DPRINTF_DEFAULT, "MRP ERROR! cmd: 0x%X, status: 0x%X\n",
                           commandCode, status);
            }
        }

        /*
         * Make sure the corresponding 'blocked' flag is clear for the
         * garbage collection (delayed free) routines.
         */
        clear_blocked_flag(commandCode, testF);
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "MRP TIMEOUT! cmd: 0x%X, status: unknown\n",
                   commandCode);

        rc = PI_TIMEOUT;        /* command did time out */

        /*
         * Make sure the corresponding 'blocked' flag is set for the
         * gargbage collection (delayed free) routines.
         */
        set_blocked_flag(commandCode, testF);
    }

    TraceEvent(TRACE_MRP + rc, commandCode);

    return rc;
}


/*----------------------------------------------------------------------------
** Function:    MRPCmd2Processor()
**
** Description: Maps an MRP command code to the target processor.
**
** Inputs:      MRP command code
**
** Returns:     Target processor
**
**--------------------------------------------------------------------------*/
static INT32 MRPCmd2Processor(UINT32 mrpCmd)
{
    if ((mrpCmd & MRP_TYPE_MASK) == CCB_TO_BE)
    {
        return PROCESS_BE;
    }
    return PROCESS_FE;
}


/*
******************************************************************************
**  @brief  Start the blocked task.
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
static void BlockedTaskStart(void)
{
    if (gpBlockedTask == NULL)
    {
        gpBlockedTask = TaskCreate(BlockedTask, NULL);
    }
}


/*
******************************************************************************
**  @brief  This task runs when the FE or BE MRP queues have been blocked in
**          order to submit an MRP to clear the blocked flag.
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
static void BlockedTask(UNUSED TASK_PARMS *parms)
{
    INT32       rcBE;
    INT32       rcFE;

    while (BEBlocked() || FEBlocked())
    {
        TaskSleepMS(BLOCKED_TASK_RETRY_TIME);

        dprintf(DPRINTF_EXECMRP, "Blocked Task running (0x%x, 0x%x)\n",
                BEBlocked(), FEBlocked());

        /* Assume that everything is good until we realize it isn't. */
        rcBE = PI_GOOD;
        rcFE = PI_GOOD;

        /*
         * Check if the BE is still considered blocked and if it is send
         * the NOOP and FSYS NOOP requests to attempt to clear the blocked
         * state.
         */
        if (BEBlocked())
        {
            rcBE = ProcessorQuickTest(MRNOPBE);

            if (rcBE != PI_TIMEOUT)
            {
                rcBE = ProcessorQuickTest(MRNOPFSYS);
            }
        }

        /*
         * Check if the FE is still considered blocked and if it is send
         * the NOOP request to attempt to clear the blocked state.
         */
        if (FEBlocked())
        {
            rcFE = ProcessorQuickTest(MRNOPFE);
        }

        /* Check if any of the requests timed out. */
        if (rcBE == PI_TIMEOUT || rcFE == PI_TIMEOUT)
        {
            /*
             * In the timeout case we want to exit out of the loop
             * and shut down this task, it will be restarted when
             * the next timeout callback is received.
             */
            break;
        }
    }

    gpBlockedTask = NULL;
    dprintf(DPRINTF_EXECMRP, "Blocked Task finished\n");
}


/*----------------------------------------------------------------------------
** Function:    DelayedFreeCleanupTask()
**
** Description: This is the garbage collection task that frees up storage
**              left behind when MRP's timed out.
**
** Inputs:      list and count of pointers to free
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
static void DelayedFreeCleanupTask(UNUSED TASK_PARMS *parms)
{
    while (TRUE)
    {
        TaskSleepMS(DELAYED_FREE_RETRY_TIME);

        TraceEvent(TRACE_MRP_GC_TASK_START, 0);
        dprintf(DPRINTF_EXECMRP, "MRP garbage collection running\n");

        /*
         * If the processor is no longer blocked and there is still memory
         * to be garbage collected then perform a quick test of processor
         * communications path. If the path is good (the processor has
         * handled all outstanding requests), then garbage collect all
         * data associated with that processor.
         * Perform this operation on both FE and BE processors.
         */
        if ((be.blocked == 0) && (be.count > 0))
        {
            if (ProcessorQuickTest(MRNOPBE) == PI_GOOD &&
                ProcessorQuickTest(MRNOPFSYS) == PI_GOOD)
            {
                while (be.count)
                {
                    be.count--;
                    Free(be.list[be.count]);
                }
            }
        }

        if ((fe.blocked == 0) && (fe.count > 0))
        {
            if (ProcessorQuickTest(MRNOPFE) == PI_GOOD)
            {
                while (fe.count)
                {
                    fe.count--;
                    Free(fe.list[fe.count]);
                }
            }
        }

        if ((be.blocked == 0) && (fe.blocked == 0))
        {
            cleanupTaskRunning = 0;
            break;
        }
    }

    TraceEvent(TRACE_MRP_GC_TASK_END, 0);
    dprintf(DPRINTF_EXECMRP, "MRP garbage collection finished\n");
}


/*----------------------------------------------------------------------------
** Function:    EnqueueDelayedFreePtr()
**
** Description: This routine enqueues a pointer on to a list for later
**              garbage collection.  It is a private routine, called only
**              by CheckIfDelayedFreeNeeded().
**
** Inputs:      processor (BE or FE), and the pointer.
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
static void EnqueueDelayedFreePtr(INT32 proc, void *ptr)
{
    DELAYED_FREE_LIST *lP;

    switch (proc)
    {
        case PROCESS_BE:
            lP = &be;
            break;

        case PROCESS_FE:
            lP = &fe;
            break;

        default:
            return;
    }

    lP->list[lP->count] = ptr;

    /*
     * Don't let the list overflow.  If we run out of space,
     * simply overwrite the last one.  We're in a world of hurt
     * anyway if we have 1000 outstanding allocations...
     */
    if (lP->count < (DELAYED_FREE_LIST_SIZE - 1))
    {
        lP->count++;
    }

    if (cleanupTaskRunning == 0)
    {
        TaskCreate(DelayedFreeCleanupTask, NULL);
        cleanupTaskRunning = 1;
    }
}


/**
******************************************************************************
**
**  @brief  Queue pointer for later garbage collection as needed
**
**  Checks to see if the corresponding processor is blocked. If
**  so, queues the pointer for later garbage collection. If not,
**  it simply frees the pointer.
**
**  @param  mrpCmd  - Indicates the processor (BE or FE)
**  @param  ptr     - The pointer to be freed later
**
**  @return none
**
******************************************************************************
**/
void DelayedFree(UINT32 mrpCmd, void *ptr)
{
    INT32       proc;
    UINT32      enqueued = 0;

    if (!ptr)
    {
        return;
    }

    proc = MRPCmd2Processor(mrpCmd);

    if (proc == PROCESS_BE)
    {
        switch (mrpCmd)
        {
            /*
             * If its a file system op and the file system queue is
             * blocked, stick the pointer on the delayed free list.
             */
        case MRFSYSOP:
        case MRNOPFSYS:
            if (be.blocked & DFL_BLOCKED_BE_FSYS)
            {
                EnqueueDelayedFreePtr(proc, ptr);
                enqueued = 1;
            }
            break;

            /* BE heartbeats slide right on through */
        case MRBEHBEAT:
            break;

            /*
             * If its NOT a BE heartbeat or file sys op, and the BE
             * main queue is blocked, stick the pointer on the delayed
             * free list.
             */
        default:
            if (be.blocked & DFL_BLOCKED_BE)
            {
                EnqueueDelayedFreePtr(proc, ptr);
                enqueued = 1;
            }
            break;
        }
    }

    /*
     * If its an FE command and the FE main queue is blocked, stick the
     * pointer on the delayed free list.
     */
    else if (proc == PROCESS_FE)
    {
        if (fe.blocked > 0)
        {
            EnqueueDelayedFreePtr(proc, ptr);
            enqueued = 1;
        }
    }

    if (enqueued == 0)
    {
        Free(ptr);
    }
}


/*----------------------------------------------------------------------------
** Function:    numDevs_From_listType
**
** Description: Get number of devices for various list types.
**
** Inputs:      listType - type of list (PDisk, VDisk, etc.) See header file
**                         for valid types.
**
** Returns:     Maximum number of devices for type of list.
**
**--------------------------------------------------------------------------*/
UINT16 numDevs_From_listType(UINT16 listType)
{
    /* Get the number of elements that will be returned. */
    switch (listType & MRP_MASK)
    {
        case MRBEGETPORTLIST:
            return MAX_BE_PORTS;

        case MRFEGETPORTLIST:
            return MAX_FE_PORTS;

        case MRGETELIST:
            return MAX_DISK_BAYS;

        case MRGETPLIST:
            return MAX_PHYSICAL_DISKS;

        case MRGETRLIST:
            return MAX_RAIDS;

        case MRGETSLIST:
            return MAX_SERVERS;

        case MRGETTLIST:
            return MAX_TARGETS;

        case MRGETVLIST:
            return MAX_VIRTUAL_DISKS;

        default:
            return 2048;
    }
}   /* End of numDevs_From_listType */


/*----------------------------------------------------------------------------
** Function:    PI_GetList
**
** Description: Get list of objects
**
** Inputs:      startID - ID of first item in the list to retrieve
**              listType - type of list (PDisk, VDisk, etc.) See header file
**                         for valid types.
**
** Returns:     PI_GOOD or PI_ERROR
**
** WARNING:     It is the callers responsibility to free the memory used
**              for the MRP output data. Returns NULL pointer on error.
**
**--------------------------------------------------------------------------*/
MR_LIST_RSP *PI_GetList(UINT16 startID, UINT16 listType)
{
    MR_DEVID_REQ *ptrListIn = NULL;
    MR_LIST_RSP *ptrListOut = NULL;
    UINT32      listOutSize;
    UINT16      numDevs;
    UINT16      rc = PI_GOOD;

    numDevs = numDevs_From_listType(listType);

    do
    {
        /*
         * Allocate memory for the MRP input and output packets.
         * Fill in the input parm.
         */
        ptrListIn = MallocWC(sizeof(*ptrListIn));
        ptrListIn->id = startID;

        /* Note: The structure definitions have an array of one for the list element. */
        listOutSize = (sizeof(*ptrListOut) + ((numDevs - 1) * sizeof(UINT16)));

        /*
         * If an output list was previously allocated, free it before
         * allocating a new one.
         */
        if (ptrListOut != NULL)
        {
            Free(ptrListOut);
        }

        ptrListOut = MallocSharedWC(listOutSize);

        /*
         * Send the request to Thunderbolt.  This function handles timeout
         * conditions and task switches while waiting.
         */
        rc = PI_ExecMRP(ptrListIn, sizeof(*ptrListIn), (listType & MRP_MASK),
                        ptrListOut, listOutSize, GetGlobalMRPTimeout());

        if (ptrListIn != NULL)
        {
            Free(ptrListIn);
            ptrListIn = NULL;
        }

        /*
         * Save the number of devices in case we need to make the
         * request again.
         */
        numDevs = ptrListOut->ndevs;

    } while ((listType & GET_LIST) && (rc == PI_ERROR) &&
             (ptrListOut->header.status == DETOOMUCHDATA));
    /*
     * If this was a request for a complete list, the MRP must complete
     * without error.  However if all we wanted was the number of items in
     * a list, an MRP return of DETOOMUCHDATA is OK.
     */
    if ((rc == PI_GOOD) ||
        ((rc == PI_ERROR) &&
         (ptrListOut->header.status == DETOOMUCHDATA) && (!(listType & GET_LIST))))
    {
        return (ptrListOut);
    }
    else
    {
        /*
         * If we return NULL the caller can't free the memory so do it here.
         * Only free the memory if the request did NOT timeout.  On a timeout
         * the memory must remain available in case the request eventually
         * completes.
         */
        if (rc != PI_TIMEOUT)
        {
            Free(ptrListOut);
        }

        return (NULL);
    }
}


/**********************************************************************
*                                                                     *
*  Name:        BEBlocked()                                           *
*                                                                     *
*  Description: Determine if BE is blocked as a result of             *
*               MRPs that have timed out.                             *
*                                                                     *
*  Input:       none.                                                 *
*                                                                     *
*  Returns:     0 if not blocked; non-zero otherwise.                 *
*                                                                     *
**********************************************************************/
bool BEBlocked(void)
{
    /* Indicate if BE is blocked. */
    return (be.blocked & DFL_BLOCKED_BE) != 0;
}


/**
******************************************************************************
**
**  @brief      To provide a method of determining if the FE is blocked
**              as a result of MRPs that have timed out.
**              a lost mirror partner.
**
**  @param      none
**
**  @return     bool - 0 if not blocked, non-zero otherwise
**
******************************************************************************
**/
bool FEBlocked(void)
{
    /* Indicate if FE is blocked. */
    return (fe.blocked & DFL_BLOCKED_FE) != 0;
}


/**********************************************************************
*                                                                     *
*  Name:        ProcessorQuickTest()                                  *
*                                                                     *
*  Description: Make a "quick" request to TBolt just to determine if  *
*               its ready yet.                                        *
*                                                                     *
*  Input:       mrpCmd - MRP Command code to use for this test.       *
*                                                                     *
*  Returns:     0 on success; non-zero otherwise.                     *
*                                                                     *
*  Warning:     The MRP command code must one of the following:       *
*               - MRNOPBE                                             *
*               - MRNOPFSYS                                           *
*               - MRNOPFE                                             *
*                                                                     *
**********************************************************************/
INT32 ProcessorQuickTest(UINT32 mrpCmd)
{
    MR_GENERIC_RSP *ptrOutPkt;
    INT32       rc;

    /*
     * The MRP command code must be a BE no-op, FSYS no-op or
     * a FE no-op.  Anything else results in an error.
     */
    if (mrpCmd == MRNOPBE || mrpCmd == MRNOPFSYS || mrpCmd == MRNOPFE)
    {
        /* Allocate memory for the output structure. */
        ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

        /*
         * Send the request to Thunderbolt.  This function handles timeout
         * conditions and task switches while waiting.
         *
         * This uses a short timeout.
         */
        rc = PI_ExecMRP(NULL, 0, mrpCmd, ptrOutPkt, sizeof(*ptrOutPkt),
                        (QUICK_TEST_TIMEOUT | MRP_INIT_REQUEST_FLAG));

        /*
         * Only free the memory if the request did NOT timeout.  On a timeout
         * the memory must remain available in case the request eventually
         * completes.
         */
        if (rc != PI_TIMEOUT)
        {
            Free(ptrOutPkt);
        }
        return rc;
    }

    /* Invalid MRP command code specified. */
    return PI_ERROR;
}

/**********************************************************************
*                                                                     *
*  Name:        InitProcessorComm()                                   *
*                                                                     *
*  Description: Wait for the FE / BE MRP to come ready.               *
*                                                                     *
*  Input:       void                                                  *
*                                                                     *
*  Returns:     void                                                  *
*                                                                     *
**********************************************************************/
void InitProcessorComm(void)
{
    INT32       rc_be = PI_ERROR;
    INT32       rc_fe = PI_ERROR;
    TIMESTAMP   start;
    TIMESTAMP   now;
    LOG_PROC_NOT_READY_PKT msg;

    /* Initialize the command record table (set to all zeros). */
    memset(commandRecordTable, 0x00, COMMAND_RECORD_TABLE_SIZE * COMMAND_RECORD_SIZE);

    /* Initialized the delayed free lists. */
    memset(&be, 0x00, sizeof(DELAYED_FREE_LIST));
    memset(&fe, 0x00, sizeof(DELAYED_FREE_LIST));

    RTC_GetTimeStamp(&start);
    start.systemSeconds += INIT_PROC_COMM_TIMEOUT;

    /* Create the task to handle MRP Timeouts */
    gMRPTimeoutTaskPCB = TaskCreate(PI_MRPTimeoutTask, NULL);

    while (1)
    {
        RTC_GetTimeStamp(&now);

        /* Exit if timed out or if both processors ready */
        if (!(IDRData.bootStatus & IDR_COORDINATED_BOOT_GOOD) ||
            (now.systemSeconds > start.systemSeconds) ||
            ((rc_be == PI_GOOD) && (rc_fe == PI_GOOD)))
        {
            break;
        }

        /* Check BE processor */
        if (rc_be != PI_GOOD)
        {
            rc_be = ProcessorQuickTest(MRNOPBE);
            if (rc_be == PI_GOOD)
            {
                dprintf(DPRINTF_INITPROCCOMM, "InitProcessorComm: BE is Ready\n");

                gProcInitFlags |= BE_READY;
            }
        }

        /* Check FE processor */
        if (rc_fe != PI_GOOD)
        {
            rc_fe = ProcessorQuickTest(MRNOPFE);
            if (rc_fe == PI_GOOD)
            {
                dprintf(DPRINTF_INITPROCCOMM, "InitProcessorComm: FE is Ready\n");

                gProcInitFlags |= FE_READY;
            }
        }
    }

    /* Set the completion flag no matter what the outcome */
    gProcInitFlags |= INIT_COMPL;

    /* Log an error event if either processor failed */
    msg.beReady = gProcInitFlags & BE_READY;
    msg.feReady = gProcInitFlags & FE_READY;
    if (!(msg.beReady && msg.feReady))
    {
        SendAsyncEvent(LOG_PROC_NOT_READY, sizeof(LOG_PROC_NOT_READY_PKT), &msg);
    }

    /* Initialize the CCB <-> FE comm variables. */
#if 0
    InitFECommVars(gProcInitFlags & FE_READY);
#endif  /* 0 */

    if ((gProcInitFlags & BE_READY) == 0)
    {
        dprintf(DPRINTF_INITPROCCOMM, "InitProcessorComm: BE could NOT be contacted!\n");
    }
    if ((gProcInitFlags & FE_READY) == 0)
    {
        dprintf(DPRINTF_INITPROCCOMM, "InitProcessorComm: FE could NOT be contacted!\n");
    }
}


/*----------------------------------------------------------------------------
** Function:    ProcessorCommReady
**
** Description: Determines if the processor communications is ready for
**              a specific processor.
**
** Inputs:      processor - Which processor to check.
**
** Returns:     true if the processor communications is ready, false otherwise
**
**--------------------------------------------------------------------------*/
bool ProcessorCommReady(INT32 processor)
{
    /* Check the XX_READY bits for the specified processor (bitwise AND) */
    if ((processor == PROCESS_BE) && (gProcInitFlags & BE_READY))
    {
        return true;
    }
    else if ((processor == PROCESS_FE) && (gProcInitFlags & FE_READY))
    {
        return true;
    }

    return false;
}


/*----------------------------------------------------------------------------
** Function:    SetGlobalMRPTimeout /  GetGlobalMRPTimeout
**
** Description: Allow the MRP timeout to be altered by the user.
**
** Inputs:      timeout - timeout value in seconds
**
** Returns:     none
**
** WARNING:     This timeout value only applies to those MRPs that would
**              normally use the value MRP_STD_TIMEOUT.  All other timeout
**              values will remain unaffected by this function.
**
**--------------------------------------------------------------------------*/
void SetGlobalMRPTimeout(UINT32 timeout)
{
    /* An input value of 0 will cause the standard timeout to be set. */
    if (timeout == 0)
    {
        gMRPTimeout = MRP_STD_TIMEOUT;
    }
    else
    {
        gMRPTimeout = timeout * 1000;   /* Input timeout is in seconds */
    }
}


UINT32 GetGlobalMRPTimeout(void)
{
    /* Return the global timeout value. */
    return (gMRPTimeout);
}


/*----------------------------------------------------------------------------
** Function:    SetGlobalIPCTimeout /  GetGlobalIPCTimeout
**
** Description: Allow the IPC timeout to be altered by the user.
**
** Inputs:      timeout - timeout value in seconds
**
** Returns:     none
**
** WARNING:     This timeout value only applies when set to something other
**              than zero and is greater than the timeout parameter value.
**--------------------------------------------------------------------------*/
void SetGlobalIPCTimeout(UINT32 timeout)
{
    gIPCTimeout = timeout * 1000;       /* Input timeout is in seconds */
}



UINT32 GetGlobalIPCTimeout(void)
{
    /* Return the global timeout value. */
    return (gIPCTimeout);
}

/*----------------------------------------------------------------------------
** Function:    GetControllerType
**
** Description: Return the controller type
**
** Inputs:      none
**
** Returns:
**              CONTROLLER_TYPE_BIGFOOT     0
**              CONTROLLER_TYPE_WOOKIEE     1
**              DO NOT USE                  2
**              CONTROLLER_TYPE_3100        3
**              CONTROLLER_TYPE_4000        4
**              CONTROLLER_TYPE_7000        5
**              CONTROLLER_TYPE_4700        6
**              CONTROLLER_TYPE_7400        7
**
**--------------------------------------------------------------------------*/
UINT8 GetControllerType(void)
{
    static UINT8 ctype;
    static UINT8 ctypeknown = FALSE;
    PI_STATS_LOOPS_RSP *pStatsLoops;

#ifdef  MODEL_7000
    ctype = CONTROLLER_TYPE_7000;
    ctypeknown = TRUE;
#endif /* MODEL_7000 */
#ifdef  MODEL_4700
    ctype = CONTROLLER_TYPE_4700;
    ctypeknown = TRUE;
#endif /* MODEL_4700 */
#ifdef  MODEL_7400
    ctype = CONTROLLER_TYPE_7400;
    ctypeknown = TRUE;
#endif /* MODEL_7400 */

    if (ctypeknown == FALSE)
    {
        ctype = CONTROLLER_TYPE_WOOKIEE;

        /* Go through the FE port info to see if there is a 4GB card */
        pStatsLoops = StatsLoops(GetMyControllerSN(), PROCESS_FE);
        if (pStatsLoops != NULL)
        {
            ctype = GetTypeFromStats(pStatsLoops);
            if (ctype == CONTROLLER_TYPE_WOOKIEE)
            {
                /*
                 * There is no 4GB HBA in the FE.
                 * Go through the BE port info to see if there is a 4GB card
                 */
                pStatsLoops = StatsLoops(GetMyControllerSN(), PROCESS_BE);
                if (pStatsLoops != NULL)
                {
                    ctype = GetTypeFromStats(pStatsLoops);
                }
            }
            ctypeknown = TRUE;
        }
    }
    return (ctype);
}

/*----------------------------------------------------------------------------
** Function:    GetTypeFromStats
**
** Description: Return the controller type
**
** Inputs:      PI_STATS_LOOPS_RSP* pStatsLoops - response to statsloop
**
** Returns:
**              CONTROLLER_TYPE_WOOKIEE     1
**              CONTROLLER_TYPE_3100        3
**              CONTROLLER_TYPE_4000        4
**
**--------------------------------------------------------------------------*/
static UINT8 GetTypeFromStats(PI_STATS_LOOPS_RSP *pStatsLoops)
{
    UINT32      vendor;
    UINT32      model;
    INT8        i;
    UINT8       ctype = CONTROLLER_TYPE_WOOKIEE;
    PI_STATS_LOOP *pStats;

    pStats = pStatsLoops->stats;
    for (i = 0; i < pStatsLoops->count; i++)
    {
        model = pStats->stats[0].rev.model;
        vendor = pStats->stats[0].rev.vendid;
        if ((vendor == 0x1077) && (model > 0x2400) && (model != 0xffff))
        {
            ctype = CONTROLLER_TYPE_4000;
            break;
        }
        pStats = (PI_STATS_LOOP *)((UINT8 *)pStats + pStats->length + 2);
    }
    Free(pStatsLoops);
    return (ctype);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
