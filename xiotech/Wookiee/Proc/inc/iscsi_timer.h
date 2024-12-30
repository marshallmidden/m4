/* $Id: iscsi_timer.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       iscsi_timer.h
**
**  @brief      iSCSI Timer functionality header file
**
**  This provides API's for timer functionality, required for iscsi Stack
**
**  Copyright (c) 2005-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/

#ifndef _ISCSI_TIMER_H_
#define _ISCSI_TIMER_H_

#include <sys/queue.h>

/* Useful macros for  us which are not defined in sys/queue.h */

#define LIST_INSERT_ELEMENT(head, elm, type, field) do {\
               struct type *curelm = (head)->lh_first;\
               while( curelm->field.le_next != NULL )\
                      curelm = curelm->field.le_next;\
                      LIST_INSERT_AFTER(curelm,elm,field);\
  } while (/*CONSTCOND*/0)

#ifdef LIST_EMPTY
  #undef LIST_EMPTY
#endif /* LIST_EMPTY */
# define LIST_EMPTY(head) ((head)->lh_first==NULL)
#ifdef LIST_FOREACH
  #undef LIST_FOREACH
#endif /* LIST_FOREACH */
# define LIST_FOREACH(np,head,type,field) for(np = (type*)((head)->lh_first); np != NULL; np = np->field.le_next)

# define TIMER_TASK_PRI             150
# define TIMER_TASK_SLEEP_VALUE    1000 /* 1 second */

typedef int  (*FUNCPTR)(UINT32 arg1,UINT32 arg2);
typedef struct ISCSI_TIMER ISCSI_TIMER;

struct ISCSI_TIMER {
    LIST_ENTRY(ISCSI_TIMER) timer_link;
    UINT32 timer_value;
    INT32 timer_ticks;
    UINT32 timer_id;
/* callback function args */
    UINT32 arg1;
    UINT32 arg2;
/* Timer control flag */
    UINT8 timer_active;

    FUNCPTR user_callback;
};

/* Local function declarations */
extern void AddNodeToList(ISCSI_TIMER *node);
extern UINT32 GenerateTimerId(void);
extern int CheckTimerList(void);

/* Timer API function delclarations used by client applications */
extern UINT32 CreateTimer(UINT32 timer_value, UINT32 arg1, UINT32 arg2, FUNCPTR cbfunc);
extern void   CreateTimerTask(void);
extern int    StartTimer(UINT32 timer_id);
extern int    ResetTimer(UINT32 timer_id);
extern int    DeleteTimer(UINT32 timer_id);
extern int    StopTimer(UINT32 timer_id);

#endif  /* _ISCSI_TIMER_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
