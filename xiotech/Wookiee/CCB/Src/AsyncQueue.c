/* $Id: AsyncQueue.c 156532 2011-06-24 21:09:44Z m4 $ */
/**
******************************************************************************
**
**  @file       AsyncQueue.c
**
**  @brief      Async Event Queue Tasks and Queueing functions.
**
**  These tasks are started at system boot. One task services the DLM
**  "datagrams" coming in from the FE; the other handles asynchronous events
**  from both the FE and BE.  Queueing functions fro each queue are also
**  provided.
**
**  Copyright (c) 2004-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "AsyncQueue.h"

#include "debug_files.h"
#include "MR_Defs.h"
#include "XIO_Macros.h"
#include "QU_Library.h"

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
QU          logEventQ = { NULL, NULL, 0, NULL };
QU          dlmEventQ = { NULL, NULL, 0, NULL };

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Asychronous Executive Task
**
**              Called at startup. Used to handle incoming asynchronous requests.
**
**  @param      pFunk    - this function's address
**  @param      pQue     - pointer to the queue to pull the requests from
**  @param      pHandler - address of the handler function
**
**  @return     none
**
******************************************************************************
**/
NORETURN void AsyncExecTask(TASK_PARMS *parms)
{
#define MAX_IN_A_ROW    20
    QU         *pQue = (QU *)parms->p1;
    ASYNC_EVENT_HANDLER pHandler = (ASYNC_EVENT_HANDLER)parms->p2;
    ILT        *pILT;
    void        (*ck)(UINT32, struct ILT *);
    int         in_a_row = 0;

    dprintf(DPRINTF_DEFAULT, "AsyncExecTask: Start. pQue = 0x%08X, Phand = 0x%08X\n",
            (UINT32)pQue, (UINT32)pHandler);

    while (1)
    {
        /*
         * Get next queued request
         */
        pILT = QU_DequeReqILT(pQue);

        if (pILT == NULL)
        {
            /*
             * Nothing to do, go to sleep.  Need to disable interrupts here
             * because this task can be made ready by the Link Layer interrupt.
             */
            TaskSetMyState(PCB_NOT_READY);
            TaskSwitch();
            in_a_row = 0;
        }
        else
        {
            /*
             * Back up to the previous ILT level
             */
            pILT--;

            /*
             * Call the handler
             */
            pHandler((void *)pILT->ilt_normal.w0, ((MR_PKT *)pILT->ilt_normal.w0)->pReq);

            /*
             * Now call the completion routine
             */
            ck = pILT->cr;
            (*ck)(0, pILT);

            if (in_a_row++ > 20)
            {
                TaskSwitch();
                in_a_row = 0;
            }
        }
    }
}


/**
******************************************************************************
**
**  @brief      Enqueue an ILT to the log event queue
**
**              Called from EnqueueILT() (see below) by the the Link Layer to
**              enque an asynchronous message (ILT) to the logging queue.
**
**  @param      pILT     - pointer to ILT to enqueue
**
**  @return     none
**
******************************************************************************
**/
void EnqueLogEvent(ILT *pILT)
{
    QU_EnqueReqILT(pILT, &logEventQ);
}


/**
******************************************************************************
**
**  @brief      Enqueue an ILT to the DLM queue
**
**              Called from EnqueueILT() (see below) by the the Link Layer to
**              enque an asynchronous message (ILT) to the fibreshim (dlm) queue.
**
**  @param      pILT     - pointer to ILT to enqueue
**
**  @return     none
**
******************************************************************************
**/
void EnqueDlmEvent(ILT *pILT)
{
    QU_EnqueReqILT(pILT, &dlmEventQ);
}


/**
******************************************************************************
**
**  @brief      Enque and ILT to the specified queue.
**
**  @param      quRoutine - queueing function pointer
**  @param      pILT      - pointer to ILT to enqueue
**  @param      compRoutine - completion function pointer
**
**  @return     none
**
******************************************************************************
**/

/* Note: prototype is found in Shared/Inc/XIO_Std.h */
void EnqueueILT(void (*qr)(struct ILT *), ILT *pILT,
                void (*cr)(UINT32, struct ILT *, ...))
{
    /*
     * Setup the completion routine
     */
    pILT->cr = cr;

    /*
     * Close the fwd link on the following ILT
     */
    pILT[1].fthd = NULL;

    /*
     * Call the queueing routine on the following ILT
     */
    qr(&pILT[1]);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
