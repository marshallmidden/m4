/* $Id: kern.c 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       kern.c
**
**  @brief      Xiotech scheduler and kernel specific routines.
**
**  Copyright (c) 2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "CT_defines.h"
#include "pcb.h"
#include "kernel.h"
#include "memory.h"

/**
******************************************************************************
**
**  @name   TaskReadyByState
**
**  @brief  To ready all processes in a specific wait condition.
**
**          The entire PCB thread is searched for processes in the
**          specified wait condition. These processes are then set to
**          ready state.
**
**  @param  state - Current state that is to become ready state.
**
**  @return none
**
***********************************************************************
**/
void TaskReadyByState(UINT8 state)
{
    struct PCB *pPCB = K_pcborg;

    if (pPCB == NULL)                   /* If not started running yet, exit. */
    {
        return;
    }

    do
    {
        if (TaskGetState(pPCB) == state)     /* Is current state matching input state? */
        {
#ifdef HISTORY_KEEP
            CT_history_pcb("TaskReadyByState setting ready pcb", (UINT32)pPCB);
#endif  /* HISTORY_KEEP */
            TaskSetState(pPCB, PCB_READY);  /* Set state to ready. */
        }
#ifndef PERF
        if (pPCB->pc_thd == NULL)
        {
            abort();                    /* Corrupted circular linked list. */
        }
#endif  /* PERF */
        pPCB = pPCB->pc_thd;            /* To next PCB. */
    } while (pPCB != K_pcborg);
}   /* End of TaskReadyByState */

/* ------------------------------------------------------------------------ */

#if 0   /* This is a debug routine for when something was really messed up. */
#ifndef PERF
extern struct fmm K_ncdram;

/**
******************************************************************************
**
**  @name   CheckTasksOK
**
**  @brief  Debug routine to check task statuses are all ok.
**
**  @param  none
**
**  @return none
**
***********************************************************************
**/
void CheckTasksOK(void)
{
    struct PCB *pPCB = K_pcborg;

    if (pPCB == NULL)                   /* If not started running yet, exit. */
    {
        return;
    }

    do
    {
        if (TaskGetState(pPCB) > PCB_IOCB_WAIT) /* current maximum state. */
        {
            abort();
        }
        pPCB = pPCB->pc_thd;            /* To next PCB. */
    } while (pPCB != K_pcborg);

    if (K_ncdram.fmm_first.thd != NULL && K_ncdram.fmm_waitstat != PCB_WAIT_NON_CACHEABLE)
    {
        abort();
    }
}   /* End of CheckTasksOK */
#endif  /* PERF */
#endif  /* 0 */

/* ------------------------------------------------------------------------ */
/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
