/* $Id: XIO_Std.h 161041 2013-05-08 15:16:49Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       XIO_Std.h
**
**  @brief      Standard and kernel related C library functions macros.
**
**  Contains standard ("strcmp", "memset") and kernel ("TaskCreate",
**  "TaskSwitch") C library functions macros.
**
**  Copyright (c) 2003-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _XIO_STD_H_
#define _XIO_STD_H_

#include "XIO_Types.h"
#ifdef CCB_RUNTIME_CODE
    #include "xk_kernel.h"
#endif  /* CCB_RUNTIME_CODE */

#include "kernel.h"

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/

/**********************************************************************
**  MACROS DEFINED FOR ALL CODE BASES
**********************************************************************/

/**
******************************************************************************
**
**  @brief      ccb_assert() MACRO
**
**              Test an assertion, call failure handler if not true.
**
** @param       exp - exression to test. Must evaluate to TRUE for the
**                    normal case.
** @param       val - An INT32 sized value that will be passed to the
**                    _assert() error handler function. This value can
**                    be printed/logged as an additional debug data point.
**
**  @return     none
**
**  @attention  ASSERT MUST BE CALLED THROUGH THE MACRO!
**  @attention  "_assert()" must be defined for your codebase for this
**              to work in your codebase, it is not a shared function.
**
******************************************************************************
**/
/* JeffJ 2005-11-07 Added #ifndef XIO_LINUX to allow
 * this file to be used by XioWebService (Ewok) so it can share source files for the
 * PI interface XIO_LINUX is defined by XioWebService.  */
#ifndef XIO_LINUX

#ifdef NO_ASSERT
    #define ccb_assert(ignore1, ignore2)
#else   /* NO_ASSERT */
    #define ccb_assert(exp, val) if(exp) {} \
                             else {_assert(#exp, (INT32)(val), __FILE__, __LINE__);}
#endif /* NO_ASSERT */
void _assert(const char *, const INT32, const char *, const INT32);

#endif /* XIO_LINUX */

/**********************************************************************
**  CCB RUNTIME macros
**********************************************************************/
#ifdef CCB_RUNTIME_CODE

    struct ILT;

    /*
    ** Kernel related function mappings
    */
    #ifdef  TaskSleepMS
    #undef  TaskSleepMS
    #endif
    #define TaskSleepMS                     XK_TaskSleepMS

    #define TaskCreate(x,y)                 XK_TaskCreate(x,y,0)
    #define TaskCreate2(x,y)                XK_TaskCreate(x,y,1)      /* LinuxLinkLayer */
#ifdef M4_ABORT
    #define TaskSwitch()                    XK_TaskSwitch(__FILE__, __LINE__, __func__)
#else   /* M4_ABORT */
    #define TaskSwitch()                    XK_TaskSwitch()
#endif  /* M4_ABORT */
    #define TaskEnd()
    /* #define TaskKill(pPcb)                  K$kill(pPcb) */

    #define TaskSetMyState(state)           TaskSetState(XK_GetPcb(),state)
    #define TaskSetState(pPcb, state)       ((pPcb)->pc_stat = state)
    #define TaskGetState(pPcb)              ((pPcb)->pc_stat)
    extern void *print_functionPtr(void *);
#ifndef PERF
#define DO_MORE_PRINT_STATE(pPcb)           else { dprintf(DPRINTF_DEFAULT, "%s:%u:%s TaskReadyState(%p) pc_stat=%u [%p]\n", __FILE__, __LINE__, __func__,  pPcb, (pPcb)->pc_stat, print_functionPtr(pPcb)); }
#else   /* PERF */
#define DO_MORE_PRINT_STATE(pPcb)
#endif  /* PERF */
    #define TaskReadyState(pPcb)            if (pPcb != NULL && \
                                                ((pPcb)->pc_stat == PCB_NOT_READY || \
                                                 (pPcb)->pc_stat == PCB_TIMED_WAIT)) \
                                            { (pPcb)->pc_stat = PCB_READY; } \
                                            DO_MORE_PRINT_STATE(pPcb)

#ifndef LOG_SIMULATOR
    /*
    ** Wait, Don't Clear. Returns Non-Cleared memory segment.
    */
    #define MallocW(size)                   XK_Malloc((size), MEM_MALLOC, 0, __FILE__, __LINE__)
    #define MallocSharedW(size)             XK_Malloc((size), MEM_MALLOC, 1, __FILE__, __LINE__)
#if 0 /* doesn't work -- figure out why sometime */
    #define malloc(size)                    DontCallLowerCaseMalloc()
#endif  /* 0 */
    /*
    ** Wait and Clear. Returns Cleared memory segment.
    */
    #define MallocWC(size)                  XK_Malloc((size), MEM_CALLOC, 0, __FILE__, __LINE__)
    #define p_XK_MallocWC(size,file,line)   XK_Malloc((size), MEM_CALLOC, 0, file, line)
    #define MallocSharedWC(size)            XK_Malloc((size), MEM_CALLOC, 1, __FILE__, __LINE__)
    #define s_XK_MallocWC(size,file,line)   XK_Malloc((size), MEM_CALLOC, 1, file, line)
    #define calloc(num, size)               DontCallLowerCaseCalloc()

    /*
    ** Check for NULL before attempting to free memory.  NULL pointer after
    ** freeing memory.  Check the electric fence.
    */
    #define Free(ptr)                       XK_Free(&(ptr), __FILE__, __LINE__)
    #define free(ptr)                       DontCallLowerCaseFree()

    /* Wait, Don't Clear. Returns Non-Cleared memory segment. */
//    extern void *s_MallocW(UINT32, const char *, unsigned int); /* Shared memory */
//    extern void *p_MallocW(UINT32, const char *, unsigned int); /* Private memory */

    /* Wait and Clear. Returns Cleared memory segment. */
//    extern void *s_MallocC(UINT32, const char *, unsigned int); /* Shared memory */
//    extern void *p_MallocC(UINT32, const char *, unsigned int); /* Private memory */

    /* Don't wait, Don't Clear. Returns NULL or Non-Cleared memory segment. */
    extern void *p_Malloc(UINT32, const char *, unsigned int);  /* Private memory */
    extern void *s_Malloc(UINT32, const char *, unsigned int);  /* Private memory */

    /* Free memory. */
    extern void p_Free(void *, UINT32, const char *, unsigned int); /* Private memory */
    extern void s_Free(void *, UINT32, const char *, unsigned int); /* Shared memory */

    /*
    ** Old Kernel Interfaces.
    */
    #define K$xchang                        TaskSwitch
#endif /* LOG_SIMULATOR */


    #define InitMutex(m)                    XK_InitMutex(m, __FILE__, __LINE__)
    #define LockMutex(m, f)                 XK_LockMutex(m, f, __FILE__, __LINE__)
    #define UnlockMutex(m)                  XK_UnlockMutex(m)

    /* If compiling the XioWebService aka ICON Services aka EWOK then we can't use
     * these macros because they break code associated with the Sockets library
     * by redefining the Select method.
     * Jeff Johnson 01-26-2006
     */
    #ifndef XIO_XWS
        /*
        ** select() system call
        */
        #define Select(maxfd, read, write, exception, timeout) \
                XK_Select((maxfd), (read), (write), (exception), (timeout))
        #define select(maxfd, read, write, exception, timeout) DontCallLowerCaseSelect()


        /*
        ** accept() system call
        */
        #define Accept(sockFd, addr, len)       XK_Accept((sockFd), (addr), (len))
        #define accept(sockFd, addr, len)       DontCallLowerCaseAccept()

        /*
        ** Other
        */
#if defined(PAM) || defined(LOG_SIMULATOR)
        #define Close(sock)                     close(sock)
        #define Fclose(sock)                    fclose(sock)
#else   /* PAM || LOG_SIMULATOR */
        #define Close(sock)                     XK_CloseFd(sock)
        #define close(ptr)                      DontCallLowerCaseClose()
        #define Fclose(sock)                    XK_CloseFile(sock)
        #define fclose(ptr)                     DontCallLowerCaseFclose()
#endif /* PAM || LOG_SIMULATOR */
    #endif /* XIO_XWS */

    /*
    ** EnqueueILT called by the Link Layer
    */
    extern void EnqueueILT(void(*qr)(struct ILT *), struct ILT *, void(*cr)(UINT32, struct ILT *, ...)); /* Queue request w/o wait    */

    /*
    ** String/memory functions
    */
    #include "stdio.h"
    #include "string.h"
    #include "stdlib.h"
#endif  /* CCB_RUNTIME_CODE */

/**********************************************************************
**  PROC RUNTIME macros
**********************************************************************/
#if defined(FRONTEND) || defined(BACKEND)

    #include <stdlib.h>
    struct ILT;
    struct SSMS;
    struct SM;

    /* Task Control */
    extern void TaskSwitch(void);                             /* Task exchange             */
    extern void TaskSleepMS(UINT32 mSec);                     /* Task timed wait (mS)      */
    extern void TaskSleepNoSwap(UINT32 uSec);                 /* Task timed wait (uS)      */
    extern void TaskReadyByState(UINT8 pstate);               /* Ready PCBs in spec.  wait */
//    extern void TaskSetMyState(UINT32 pstate);                /* Set executing task status */
    extern void TaskKill(PCB *pPcb);                          /* Task Kill                 */
    extern void *TaskCreate2(void *, UINT8);                  /* Task fork - temporary     */
    extern void *TaskCreate3(void *, UINT8, UINT32);          /* Task fork - temporary     */
    extern void *TaskCreate4(void *, UINT8, UINT32, UINT32);  /* Task fork - temporary     */
    extern void *TaskCreate5(void *, UINT8, UINT32, UINT32, UINT32);  /* Task fork - temporary     */
    extern void *TaskCreate6(void *, UINT8, UINT32, UINT32, UINT32, UINT32); /* Task fork - temporary     */
    extern void *TaskCreate7(void *, UINT8, UINT32, UINT32, UINT32, UINT32, UINT32); /* Task fork - temporary     */
    extern void *TaskCreate8(void *, UINT8, UINT32, UINT32, UINT32, UINT32,
            UINT32, UINT32); /* Task fork - temporary     */
    extern void *TaskCreatePerm2(void *func, UINT8 prio);     /* Task fork - permanent     */
    extern void *TaskCreatePerm3(void *func, UINT8 prio, UINT32 param);    /* Task fork - permanent */
    extern void *TaskCreatePerm2Shared(void *func, UINT8 prio); /* Task fork - permanent shared memory */
    extern void TaskEnd(void);                                /* End a task                */
    #define TaskSetMyState(state)           (K_xpcb)->pc_stat = (state)
    #define TaskSetState(pPcb, state)       (pPcb)->pc_stat = (state)
    #define TaskGetState(pPcb)              ((pPcb)->pc_stat)

    /* Miscellaneous */
    extern void EnqueueILT(void (*qr)(struct ILT *), struct ILT *, void(*cr)(void)); /* Queue request w/o wait */
    extern void EnqueueILTW(void (*qr)(UINT32, struct ILT *), struct ILT *); /* Queue request w/ wait     */
    extern void QWComp(void);                                 /* Complete req, resume task */

    /* Memory Control */
    /* Don't wait, Don't Clear. Returns NULL or Non-Cleared memory segment. */
    extern void *s_Malloc(UINT32, const char *, unsigned int);  /* Shared memory */
    extern void *p_Malloc(UINT32, const char *, unsigned int);  /* Private memory */

    /* Wait, Don't Clear. Returns Non-Cleared memory segment. */
    extern void *s_MallocW(UINT32, const char *, unsigned int); /* Shared memory */
    extern void *p_MallocW(UINT32, const char *, unsigned int); /* Private memory */

    /* Wait and Clear. Returns Cleared memory segment. */
    extern void *s_MallocC(UINT32, const char *, unsigned int); /* Shared memory */
    extern void *p_MallocC(UINT32, const char *, unsigned int); /* Private memory */

    /*
    ** Free memory.
    */
    extern void p_Free(void *, UINT32, const char *, unsigned int); /* Private memory */
    extern void s_Free(void *, UINT32, const char *, unsigned int); /* Shared memory */

    /*
    ** Copy memory.
    */
    extern void *CopyMem(void *dst, void *src, UINT32 length);

#endif  /* FRONTEND || BACKEND */

/*
******************************************************************************
** Macro for interfacing between K$fork and C routines in auto convert environment
******************************************************************************
*/

#define C_label_referenced_in_i960asm(x) (&CT_LC_##x)

/* Macro to create a read/write barrier for the compiler. */
#define rwbarrier() __asm__ __volatile__(" " : : : "memory")

/****************************************************************************/
/* Routine to do a test for zero, and set to one if zero -- atomically. */
/* Zero returned means you got the lock, 1 means you don't have it. */
static inline UINT8 test_zero_and_set_one_uint8(volatile UINT8 *ptr_oldval)
{
    UINT8 result;
    UINT8 check = 0;
    UINT8 newval = 1;

    __asm__ __volatile__("lock ; cmpxchgb %b1,%2"
                         : "=a"(result)                              /* outputs */
                         : "q"(newval), "m"(*ptr_oldval), "0"(check) /* inputs */
                         : "memory");                                /* clobbered */
    return(result);
}   /* End of test_zero_and_set_one_uint8 */

/* Get direct memory lock, wait for 10 ms if you didn't get it, and try again. */
#define Wait_DMC_Lock(a)    while (test_zero_and_set_one_uint8(&a->atomic_lock) != 0) {TaskSleepMS(10);}
#define Free_DMC_Lock(a)    (a->atomic_lock = 0)

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern const char *L_MsgPrefix;

#endif /* _XIO_STD_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
