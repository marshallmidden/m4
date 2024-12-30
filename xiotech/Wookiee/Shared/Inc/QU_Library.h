/* $Id: QU_Library.h 145021 2010-08-03 14:16:38Z m4 $ */
/**
********************************************************************************
**
**  @file       QU_Library.c
**
**  @brief      Exectutive Queueing Library
**
**  This provides common means of queuing operations on the executive queues
**  maintained by various modules. The functions includes putting and extra-
**  cting the ILT requests into/from the executive queues by various modules,
**  and  other  miscelleneous operations on the queues. All  these functions
**  operate on the qu structure defined in qu.h
**
**  Currently the code(in asm)  for dequeuing (extracting) the  ILTs from the
**  executive queues  is scattered throughout all the files. In the executive
**  processes(of various modules like raid, virtual, define, filsys and raid5
**  etc.), the code is replicated to extract the next ILT from the queue. And
**  the enqueing related operations for putting the request ILTs in the queue
**  (K$cque) is coupled with the kernel code and some other enqueuing opera-
**  tions are embedded in the respective modules.
**
**  This library is proposed to bring all the queuing related operations with
**  respect to the executive processes to a single(central) point that enable
**  one to maintain easily i.e., if any changes/modifications are proposed in
**  queuing  related operations  or on  its elements (like structures etc..),
**  or if the kernel itself is proposed to be changed (i.e. RTOS),  it  is
**  enough to make those changes in this  library  only,  instead of  walking
**  through all the files. And also this provides the transparency  in  terms
**  of  its elements (structures and  members etc.) and  implementation.  The
**  important thing  here is, dequeuing routine (relevant to  the asm code in
**  all the exec processes) and the enqueing routine (K$cque) and other stuff
**  are  implemented  as __inline, so  that  these will  run in  the caller's
**  current context / stack, thereby  not deviated  away  from  the  existing
**  assembly implementation. Hence this will  not reduce current performance.
**
**
**  NOTE
**  ----
**  - All  the  functions (containing executive queue related operations) use
**    this library during convertion to C from asm.
**  - The header file to be included is QU_Library.h
**  - Though now  this  library  is  meant  for  executive queues'/ processes
**    related operations,  these are  generic in nature and  can  be enhanced
**    to be used for any other queuing related operations in future.
**
**
**  Copyright (c) 2002-2009 Xiotech Corporation. All rights reserved.
**
********************************************************************************
**/
#ifndef _QULIBRARY_H_
#define _QULIBRARY_H_

#include "ilt.h"
#include "pcb.h"
#include "qu.h"
#include "XIO_Const.h"
#include "XIO_Std.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
static __inline ILT *QU_DequeReqILT(QU *pExecQueue);
static __inline void QU_EnqueReqILT(ILT *pILT, QU *pExecQueue);
static __inline UINT8 QU_IsExecProcessActive(QU *pExecQueue);
static __inline void QU_MakeExecProcessActive(QU *pExecQueue);
#ifndef CCB_RUNTIME_CODE
static __inline ILT *QU_DequeThisILT(ILT *pILT, QU *pExecQueue);
static __inline void QU_EnqueReqILT_2(ILT *pILT, QU *pExecQueue);
static __inline UINT8 QU_IsExecQEmpty(QU *pExecQueue);
static __inline void QU_MakeExecProcessInactive(QU *pExecQueue);
#endif  /* CCB_RUNTIME_CODE */

/**
********************************************************************************
**
**  @brief      Gets and dequeues(removes) the next ILT from the queue
**
**              This function gets the  next queued  ILT  from  the specified
**              executive queue. It also removes this request from the queue.
**              It  adjusts the  queue pointers  and the request  count.
**              This function replaces  the (assembly) code  in the executive
**              processes of various modules like raid, virtual, definebe etc.
**
**  @param      pExecQueue  - Pointer to the executive queue
**
**  @return     Pointer to ILT having next queued request (or) NULL in case of
**              empty queue.
**
**  @attention  The disable/enable of interrupts are added in this function. So
**              developers converting asm code section related to this dequeing
**              mechanism need not invoke interrupt disable/enable. Though the
**              disable/enable of interrupts mechanism is not mandatory for all
**              the modules as per the current design(since the queues are not
**              modified at the interrupt level), it is best to be consistent
**              and to have them located in one place.
**
********************************************************************************
**/
static __inline ILT *QU_DequeReqILT(QU *pExecQueue)
{
    ILT *pNextReq;
    ILT *pReqILT;

    /*
    ** Get the first request ILT and check if the queue is empty or not.
    */
    pReqILT = pExecQueue->head;
    if (NULL != pReqILT)
    {
        /*
        ** Dequeue the first request(at head) i.e. first ILT in the queue.
        */
        pNextReq = pReqILT ->fthd;

        /*
        ** Update the head pointer of the queue.
        */
        pExecQueue->head = pNextReq;

        if (NULL == pNextReq)
        {
            /*
            ** Queue is now empty-- i.e. only one node  existing is dequeued.
               Update the tail pointer.
            */
            pExecQueue->tail = NULL;
        }
        else
        {
            /*
            ** Update the backward thread. First node should be backward pointed
            ** the queue itself.
            */
            pNextReq->bthd = pExecQueue->head;
        }

        /*
        ** Update the requests count in the queue.
        */
        pExecQueue->qcnt -= 1;

        /*
        ** Just ensure that the node extracted does not have any nodes linked
        ** to it.
        */
        pReqILT->fthd = NULL;
        pReqILT->bthd = NULL;
    }

    return (pReqILT);
}

#ifndef CCB_RUNTIME_CODE
/**
********************************************************************************
**
**  @brief      QU_DequeThisILT - Dequeues(removes) the ILT from the queue
**
**              This function removes the requested ILT from the specified
**              executive queue. It adjusts the  queue pointers and the
**              request count.
**
**  @param      pILT        - Pointer to the ILT to remove
**  @param      pExecQueue  - Pointer to the executive queue
**
**  @return     Pointer to ILT removed (or) NULL in case the ILT was not on
**              the queue.
**
**  @attention  The disable/enable of interrupts are added in this function. So
**              developers converting asm code section related to this dequeing
**              mechanism need not invoke interrupt disable/enable. Though the
**              disable/enable of interrupts mechanism is not mandatory for all
**              the modules as per the current design(since the queues are not
**              modified at the interrupt level), it is best to be consistent
**              and to have them located in one place.
**
********************************************************************************
**/
static __inline ILT *QU_DequeThisILT(ILT *pILT, QU *pExecQueue)
{
    ILT *pThisReq = NULL;               /* Current ILT in the List          */

    /*
    ** Find the ILT on the list
    */
    for (pThisReq = pExecQueue->head;   /* Get the head of the queue        */
         ((pThisReq != pILT) && (pThisReq != NULL)); /* While not Found or end*/
         pThisReq = pThisReq->fthd)     /* Keep looking the list            */
    {
        /*
        ** Do nothing, just keep looking
        */
    }

    /*
    ** If the Request was found, remove it from the queue, decrement the count,
    ** and return the requested ILT.  Else return NULL to the caller.
    */
    if (pThisReq != NULL)
    {
        /*
        ** Remove the ILT from the doubly linked list
        */
        if ((pThisReq == pExecQueue->head) &&
            (pThisReq == pExecQueue->tail))
        {
            /*
            ** Only one item on the list, set the queue head and tail to NULL
            */
            pExecQueue->head = NULL;
            pExecQueue->tail = NULL;
        }
        else if (pThisReq == pExecQueue->head)
        {
            /*
            ** At the head of many, remove it from the beginning of the list.
            */
            pExecQueue->head = pThisReq->fthd;  /* Set new Head             */
            pThisReq->fthd->bthd = pExecQueue->head; /* Set Next ILTs bwd ptr */
        }
        else if (pThisReq == pExecQueue->tail)
        {
            /*
            ** At the tail of many requests, remove it from the end.
            */
            pExecQueue->tail = pThisReq->bthd; /* Set the new queue tail    */
            pThisReq->bthd->fthd = NULL; /* Null previous ILTs forward ptr  */
        }
        else
        {
            /*
            ** Not at head or tail, so must be in middle.
            */
            pThisReq->bthd->fthd = pThisReq->fthd; /* Remove the Fwd Links  */
            pThisReq->fthd->bthd = pThisReq->bthd; /* Remove the Bwd Links  */
        }

        /*
        ** Update the requests count in the queue.
        */
        --pExecQueue->qcnt;

        /*
        ** Just ensure that the node extracted does not have any nodes linked
        ** to it.
        */
        pThisReq->fthd = NULL;
        pThisReq->bthd = NULL;
    }

    return (pThisReq);
}
#endif  /* CCB_RUNTIME_CODE */

/**
********************************************************************************
**
**  @brief      Enqueues(puts) the requests(ILT) in the executive queue
**
**              This function provides a common means of queuing I/O requests
**              for all the modules.The ILT and the associated request packet
**              are queued to the tail of the specified executive  queue. The
**              corresponding executive process is then activated to  process
**              this request. The arguments for this function  are pointer to
**              to ILT that is to be put in the queue specified by the second
**              argument.
**              This  function is the  C equivalent for the assembly function
**              K$cque.
**              The C equivalent queuing functions  of the  D$que(define.as),
**              The  queuing  functions  D$que(define.as),    DLM$que(dlm.as)
**              DLM$Vlraid   and  DLM$LrPcr (dlmbe.a),   Lld$rcvsrp (lld.as),
**              M$que_hbeat (misc.as),  P$que and  P$qcomp_ilt (physical.as),
**              R$que  (raid.as),    r$xque    and        r$cque  (raid5.as),
**              D$rip_que(raidinit.as),              RB$rerror_que(rebld.as),
**              V$que(virtual.as),  which branches their  control to  the asm
**              function K$cque  for putting the  request in the queue should
**              now use this function instead.
**
**  @param      pILT        - Pointer to ILT, containing the request
**  @param      pExecQueue  - Pointer to the executive queue
**
**  @return     none
**
**  @attention  none
**
********************************************************************************
**/
static __inline void QU_EnqueReqILT(ILT *pILT, QU *pExecQueue)
{
    ILT *pPrevTail;

    /*
    ** Get previous(existing) Q tail.
    */
    pPrevTail = pExecQueue->tail;

    /*
    ** Set new queue tail.
    */
    pExecQueue->tail = pILT;

    /*
    ** Check if the queue is empty(before this req. addition).
    */
    if (pPrevTail == NULL)
    {
        /*
        ** Queue is empty..so this is the first node(request) in the queue. So,
        ** set up the new head and backward thread.
        */
        pExecQueue->head = pILT;
        pILT->bthd = pExecQueue->head;
    }
    else
    {
        /*
        ** Few requests(nodes) already existing. So add this request to the last
        ** entry and also link the backward thread of this new request to the
        ** previous node.
        */
         pPrevTail->fthd = pILT;
         pILT->bthd = pPrevTail;
    }

    /*
    ** Close the forward thread.
    */
    pILT->fthd = NULL;

    /*
    ** Adjust the queue new count.. added one more request.
    */
    pExecQueue->qcnt += 1;

    /*
    ** Set executive process in ready state, if not in ready state
    */
    if (FALSE == QU_IsExecProcessActive(pExecQueue))
    {
        QU_MakeExecProcessActive(pExecQueue);
    }
}

#ifndef CCB_RUNTIME_CODE
/**
********************************************************************************
**
**  @brief      Enqueues(puts) the requests(ILT) in the executive queue
**
**              This function provides a common means of queuing I/O requests
**              for all the modules.The ILT and the associated request packet
**              are queued to the tail of the specified executive  queue. The
**              corresponding executive process is then activated to  process
**              this request. The arguments for this function  are pointer to
**              to ILT that is to be put in the queue specified by the second
**              argument.
**
**  @param      pILT        - Pointer to ILT, containing the request
**  @param      pExecQueue  - Pointer to the executive queue
**
**  @return     none
**
**  @attention  This routine is same as QU_EnqueReqILT, the only difference is
**              that it activates the task only when the task is in NOT_READY
**              state.
**
********************************************************************************
**/
static __inline void QU_EnqueReqILT_2(ILT *pILT, QU *pExecQueue)
{
    ILT *pPrevTail;

    /*
    ** Get previous(existing) Q tail.
    */
    pPrevTail = pExecQueue->tail;

    /*
    ** Set new queue tail.
    */
    pExecQueue->tail = pILT;

    /*
    ** Check if the queue is empty(before this req. addition).
    */
    if (pPrevTail == NULL)
    {
        /*
        ** Queue is empty..so this is the first node(request) in the queue. So,
        ** set up the new head and backward thread.
        */
        pExecQueue->head = pILT;
        pILT->bthd = pExecQueue->head;
    }
    else
    {
        /*
        ** Few requests(nodes) already existing. So add this request to the last
        ** entry and also link the backward thread of this new request to the
        ** previous node.
        */
        pPrevTail->fthd = pILT;
        pILT->bthd = pPrevTail;
    }

    /*
    ** Close the forward thread.
    */
    pILT->fthd = NULL;

    /*
    ** Adjust the queue new count.. added one more request.
    */
    pExecQueue->qcnt += 1;

    /*
    ** Set executive process in ready state, if not in ready state
    */
    if (TaskGetState(pExecQueue->pcb) == PCB_NOT_READY)
    {
       QU_MakeExecProcessActive(pExecQueue);
    }
}
#endif  /* CCB_RUNTIME_CODE */

#ifndef CCB_RUNTIME_CODE
/**
********************************************************************************
**
**  @brief      Verifies whether the queue is empty or not
**
**              This function verifies the whether the executive queue specified
**              by the argument is empty or containing any requests to be proc-
**              essed. It checks the head pointer in the que and if it is NULL
**              returns  TRUE or returns FALSE.
**
**  @param      pExecQueue  - Pointer to the executive queue
**
**  @return     TRUE or FALSE
**
**  @attention  exec processes of various modules uses the function before
**              performing the dequing of the request..See QU_DequeReqILT()
**
********************************************************************************
**/
static __inline UINT8 QU_IsExecQEmpty(QU *pExecQueue)
{
    return (pExecQueue->head == NULL ?TRUE :FALSE);
}
#endif  /* CCB_RUNTIME_CODE */

/**
********************************************************************************
**
**  @brief      Verifies whether the exec process is ready or not
**
**              This function verifies the whether the executive process is
**              ready (active) or not. This  function checks the stat field
**              process control block  member contained in  the executive Q
**              of the caller's executive process.
**              The enquing routines  uses this routine before making the exec
**              process active.
**
**  @param      pExecQueue  - Pointer to the executive queue
**
**  @return     TRUE or FALSE
**
**  @attention  See QU_IsExecProcessActive & QU_EnqueReqILT()
**
********************************************************************************
**/
static __inline UINT8 QU_IsExecProcessActive(QU *pExecQueue)
{
    return ((TaskGetState(pExecQueue->pcb) != PCB_NOT_READY) ? TRUE : FALSE);
}

/**
********************************************************************************
**
**  @brief      Makes the specified executive process ready.
**
**              This function makes the specified executive process active,by
**              setting  the  stat  field  of  process control  block  member
**              contained in the executive Q  of   caller's executive process
**              to ready state.
**              The enquing routines should use this routine to activate the
**              the process if it is idle, after putting the  request in the
**              queue.
**
**  @param      pExecQueue  - Pointer to the executive queue
**
**  @return     none
**
**  @attention  See QU_EnqueReqILT()
**              This is equivalent to K$qwcomp
**
********************************************************************************
**/
static __inline void QU_MakeExecProcessActive(QU *pExecQueue)
{
    TaskSetState(pExecQueue->pcb,PCB_READY);
}

#ifndef CCB_RUNTIME_CODE
/**
********************************************************************************
**
**  @brief      Makes the specified executive process not-ready.
**
**              This function makes the specified executive  process inactive,
**              by setting the  stat field  of process control  block  member
**              contained in the executive Q  of   caller's executive process
**              to not-ready state.
**
**  @param      pExecQueue  - Pointer to the executive queue
**
**  @return     none
**
**  @attention  none
**
********************************************************************************
**/
static __inline void QU_MakeExecProcessInactive(QU *pExecQueue)
{
    TaskSetState(pExecQueue->pcb, PCB_NOT_READY);
}
#endif  /* CCB_RUNTIME_CODE */

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _QULIBRARY_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
