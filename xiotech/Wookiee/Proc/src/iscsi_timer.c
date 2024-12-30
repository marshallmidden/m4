/* $Id: iscsi_timer.c 144139 2010-07-14 19:46:01Z m4 $ */
/**
 ******************************************************************************
 **
 **  @file       iscsi_timer.c
 **
 **  @brief      iSCSI Timer functionality implementation file
 **
 **  This provides API's for create/start/stop timers
 **
 **  Copyright (c) 2005-2010 XIOtech Corporation.  All rights reserved.
 **
 ******************************************************************************
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "XIO_Std.h"
#include "XIO_Types.h"
#include "XIO_Macros.h"
#include "iscsi_common.h"
#include "iscsi_pdu.h"
#include "iscsi_tsl.h"
#include "iscsi_digest.h"
#include "iscsi_timer.h"

LIST_HEAD(timer_list,ISCSI_TIMER) timer_head;

extern unsigned long CT_fork_tmp;
extern void  CT_LC_iscsiTimerTaskCallback(void);
PCB *pTimerTask;
void iscsiTimerTaskCallback(void);

/* Timer API functions implementation
 *
 */

 /**
  ******************************************************************************
  **  @brief     CreateTimerTask
  **
  **             This is the main function that creates the timer task, which is
  **             being called when iScsi is just about to initialize.(tsl_epoll).
  **             The timer task runs at the priority 150. It continuously monitors
  **             the active timer list(current granularity is one second) every second,
  **             and invokes the client callback functions.
  **
  **  @param      void
  **
  **  @return     void
  **
  ******************************************************************************
 **/


void CreateTimerTask(void)
{

    CT_fork_tmp = (unsigned long)"iscsiTimerTaskCallback";
    LIST_INIT(&timer_head);
    pTimerTask = TaskCreate2(C_label_referenced_in_i960asm(iscsiTimerTaskCallback),TIMER_TASK_PRI);

}


 /**
  ******************************************************************************
  **  @brief     CreateTimer
  **
  **             Clinets use this API to create the timer,by providing the timer value, and
  **             a callback function with the two arguments.  Once the timer elapses, the
  **             client call back function will be called with these two arguments.  This
  **             function returns a unique timer_id value for every timer it creates.
  **             Cliens must use this timerid for subsequent operations on that created timer.
  **
  **  @param      timer_value -- timervalue in sec for the timer
  **  @param      arg1 --        UINT32 value arg1
  **  @param      arg2 --        UINT32 value arg2
  **  @param      cbfunc --      A callback function to be called when timer elapses
  **
  **  @return     UINT32  timer_id
  **
  ******************************************************************************
 **/

UINT32 CreateTimer(UINT32 timer_value, UINT32 arg1, UINT32 arg2, FUNCPTR cbfunc)
{
    ISCSI_TIMER *pTimerNode = (ISCSI_TIMER*)s_MallocC(sizeof(ISCSI_TIMER), __FILE__, __LINE__);
    if(pTimerNode == NULL)
    {
        fprintf(stderr,"Malloc Failed\n");
        return XIO_FAILURE;
    }
    memset((pTimerNode),XIO_ZERO,sizeof(ISCSI_TIMER));


    pTimerNode->timer_value =  timer_value;
    pTimerNode->timer_ticks =  timer_value;
    pTimerNode->arg1        =  arg1;
    pTimerNode->arg2        =  arg2;
    pTimerNode->timer_active=  0;
    pTimerNode->user_callback = cbfunc;

    pTimerNode->timer_id    = GenerateTimerId();

    /* Add this timer to  timer list */
    AddNodeToList(pTimerNode);

    return (pTimerNode->timer_id);

} /* End func CreateTimer */



 /**
  ******************************************************************************
  **  @brief     StartTimer
  **
  **             Starts the timer in the timer list that matches with the timer_id
  **             provided by the client.
  **
  **  @param     timer_id- timerid value that is returned during the CreateTimer call
  **
  **  @return    XIO_SUCCESS on success  XIO_FAILIURE otherwise.
  **
  ******************************************************************************
 **/

int StartTimer(UINT32 timer_id)
{
    ISCSI_TIMER  *node;

    if (LIST_EMPTY(&timer_head))
    {

        fprintf(stderr, "Timerlist is empy add a timer first\n");
        return XIO_FAILURE;
    }
    LIST_FOREACH(node,&timer_head,ISCSI_TIMER,timer_link)
    {
        if (node->timer_id == timer_id)
        {
            node->timer_active = TRUE;
            break;
        }
    }
    if (node == NULL)
    {
        fprintf(stderr, "Timer not found in the timer list\n");
        return XIO_FAILURE;
    }
    return XIO_SUCCESS;

} /* End func StartTimer */


 /**
  ******************************************************************************
  **  @brief     ResetTimer
  **
  **             Resets the timer in the timer list that matches with the timer_id
  **             provided by the client.
  **
  **  @param     timer_id- timerid value that is returned during the CreateTimer call
  **
  **  @return    XIO_SUCCESS on success  XIO_FAILIURE otherwise.
  **
  ******************************************************************************
 **/

int ResetTimer(UINT32 timer_id)
{
    ISCSI_TIMER  *node;
    if (LIST_EMPTY(&timer_head))
    {

         fprintf(stderr, "Timerlist is empy add a timer first\n");
         return XIO_FAILURE;
    }
    LIST_FOREACH(node,&timer_head,ISCSI_TIMER,timer_link)
    {
        if (node->timer_id == timer_id)
        {
            node->timer_ticks = node->timer_value;
            break;
        }
    }

    if (node == NULL)
    {
        fprintf(stderr, "Timer not found in the timer list\n");
        return XIO_FAILURE;
    }

    return XIO_SUCCESS;

} /* End of func ResetTimer */


 /**
  ******************************************************************************
  **  @brief     DeleteTimer
  **
  **             Deletes the timer in the timer list that matches with the timer_id
  **             provided by the client.
  **
  **  @param     timer_id- timerid value that is returned during the CreateTimer call
  **
  **  @return    XIO_SUCCESS on success  XIO_FAILIURE otherwise.
  **
  ******************************************************************************
 **/

int DeleteTimer(UINT32 timer_id)
{
    ISCSI_TIMER  *node;

    if (LIST_EMPTY(&timer_head))
    {

        fprintf(stderr, "Timerlist is empy add a timer first\n");
        return XIO_FAILURE;
    }
    LIST_FOREACH(node,&timer_head,ISCSI_TIMER,timer_link)
    {
        if (node->timer_id == timer_id)
        {
            node->timer_ticks = -1;
            return XIO_SUCCESS;
        }
    }
    if (node == NULL)
    {
        //fprintf(stderr, "Timer not found in the timer list\n");
        return XIO_FAILURE;
    }

    return XIO_SUCCESS;

} /* End of func DeleteTimer */




 /**
  ******************************************************************************
  **  @brief     StopTimer
  **
  **             Stops the timer in the timer list that matches with the timer_id
  **             provided by the client.
  **
  **  @param     timer_id- timerid value that is returned during the CreateTimer call
  **
  **  @return    XIO_SUCCESS on success  XIO_FAILIURE otherwise.
  **
  ******************************************************************************
 **/

int StopTimer(UINT32 timer_id)
{
    ISCSI_TIMER  *node;

    if (LIST_EMPTY(&timer_head))
    {

        fprintf(stderr, "Timerlist is empy add a timer first\n");
        return XIO_FAILURE;
    }

    LIST_FOREACH(node,&timer_head,ISCSI_TIMER,timer_link)
    {
        if (node->timer_id == timer_id)
        {
            node->timer_active=0;
            node->timer_ticks = node->timer_value;
            break;
        }
    }
    if (node == NULL)
    {
        fprintf(stderr, "Timer not found in the timer list\n");
        return XIO_FAILURE;
    }

    return XIO_SUCCESS;

} /* End of StopTimer */




/*Internal function definitions
 */

 /**
  ******************************************************************************
  **  @brief     AddNodeToList
  **
  **             Adds a newly created timer node to the list
  **
  **  @param     node- Newly created ISCSI_TIMER node
  **
  **  @return    void
  **
  ******************************************************************************
 **/

void AddNodeToList(ISCSI_TIMER *node)
{
    if (LIST_EMPTY(&timer_head))
    {
        LIST_INSERT_HEAD(&timer_head,node,timer_link);
    }
    else
    {
        LIST_INSERT_ELEMENT(&timer_head,node,ISCSI_TIMER,timer_link);
    }

} /* End of fucn AddNodeToList */


/*  Generates a unique timerid.
 */
 /**
  ******************************************************************************
  **  @brief     GenerateTimerid
  **             Generates a unique timerid.
  **
  **  @param     void
  **
  **  @return    UINT32 value
  **
  ******************************************************************************
 **/

UINT32 GenerateTimerId(void)
{
    static UINT32 timerid=1;

    return timerid++;
} /* End of func GenerateTimerId */



 /******************************************************************************
  **  @brief     CheckTimerList
  **
  **             Checks the timer list and decrements the timer ticks
  **             by one for active timers. If the timer ticks is zero
  **             invokes the user call back function.
  **
  **  @param     void
  **
  **  @return    XIO_SUCCESS on success  XIO_FAILIURE otherwise.
  **
  ******************************************************************************
 **/

int CheckTimerList(void)
{
    ISCSI_TIMER  *node;
    ISCSI_TIMER *tmp;
    if (LIST_EMPTY(&timer_head))
    {
       /* no timers exist no probs  we will return */
       return XIO_SUCCESS;
    }

    LIST_FOREACH(node,&timer_head,ISCSI_TIMER,timer_link)
    {
        if (node->timer_active)
        {

            if (node->timer_ticks > 0 )
            {
                node->timer_ticks--;
            }
            if (node->timer_ticks == 0 )
            {
                node->timer_ticks=node->timer_value;
                /* Call the user callback function */
                (*node->user_callback)(node->arg1,node->arg2);
            }
            if(node->timer_ticks == -1)
            {
               tmp = node->timer_link.le_next;
               LIST_REMOVE(node,timer_link);
               s_Free(node,sizeof(ISCSI_TIMER), __FILE__, __LINE__);
               node= tmp;
            }

            if(node == NULL)
            {
                /*
                 ** It seems all timers got deleted
                 */
                return XIO_SUCCESS;
            }
        }
    } /* End for each */

    return XIO_SUCCESS;
} /* End of func CheckTimerList */


 /******************************************************************************
  **  @brief     CheckTimerList
  **             Timertask callback function, this function
  **             sleeps for one second and checks the active timer list
  **
  **  @param     void
  **
  **  @return    void
  **
  ******************************************************************************
 **/

NORETURN
void iscsiTimerTaskCallback(void)
{
    INT8 retVal  =  -1;

    while(TRUE)
    {
        TaskSleepMS(TIMER_TASK_SLEEP_VALUE);
        retVal = CheckTimerList();
    } /* End of while TRUE */
} /* End function iscsiTimerTaskCallback */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
