/* $Id: xk_kernel.h 159129 2012-05-12 06:25:16Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       xk_kernel.h
**
**  @brief      This is the header for the wrapper for the I960 assembler kernel
**
**  This module will define kernel functions to the CCB code running in the
**  Linux environment.  This will preserve our concept of a cooperative
**  multitasking codebase.
**
**  Copyright (c) 2004-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _XK_KERNEL_H_
#define _XK_KERNEL_H_

#include <stddef.h>
#include "mutex.h"
#include "pcb.h"

#ifndef LOG_SIMULATOR
#include <sys/socket.h>
#include <sys/select.h>
#endif /* LOG_SIMULATOR */
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define SYSTEM_SRV_SOCKET_PATH  "/tmp/SystemSrv.socket"
#define XK_CLOSE_FD             0
#define XK_CLOSE_FILE           1

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/
#define XK_CloseFd(cfd)     XK_CloseDescriptor(XK_CLOSE_FD,(void*)(cfd))
#define XK_CloseFile(cfd)   XK_CloseDescriptor(XK_CLOSE_FILE,(void*)(cfd))

#define get_esp()       ({ulong _s_;__asm__ __volatile__("movl %%esp,%0" : "=r" (_s_)); _s_;})
#define get_ebp()       ({ulong _s_;__asm__ __volatile__("movl %%ebp,%0" : "=r" (_s_)); _s_;})
#define get_edi()       ({ulong _s_;__asm__ __volatile__("movl %%edi,%0" : "=r" (_s_)); _s_;})
#define get_esi()       ({ulong _s_;__asm__ __volatile__("movl %%esi,%0" : "=r" (_s_)); _s_;})
#define get_edx()       ({ulong _s_;__asm__ __volatile__("movl %%edx,%0" : "=r" (_s_)); _s_;})
#define get_ecx()       ({ulong _s_;__asm__ __volatile__("movl %%ecx,%0" : "=r" (_s_)); _s_;})
#define get_ebx()       ({ulong _s_;__asm__ __volatile__("movl %%ebx,%0" : "=r" (_s_)); _s_;})
#define get_eax()       ({ulong _s_;__asm__ __volatile__("movl %%eax,%0" : "=r" (_s_)); _s_;})
#define get_cs()        ({ulong _s_;__asm__ __volatile__("movl %%cs,%0" : "=r" (_s_)); _s_;})
#define get_ds()        ({ulong _s_;__asm__ __volatile__("movl %%ds,%0" : "=r" (_s_)); _s_;})
#define get_ss()        ({ulong _s_;__asm__ __volatile__("movl %%ss,%0" : "=r" (_s_)); _s_;})
#define get_es()        ({ulong _s_;__asm__ __volatile__("movl %%es,%0" : "=r" (_s_)); _s_;})
#define get_fs()        ({ulong _s_;__asm__ __volatile__("movl %%fs,%0" : "=r" (_s_)); _s_;})
#define get_gs()        ({ulong _s_;__asm__ __volatile__("movl %%gs,%0" : "=r" (_s_)); _s_;})
#define get_eflags()    ({ulong _t_;                        \
                          __asm__ __volatile__("pushf");   \
                          _t_ = *((UINT32*)get_esp());      \
                          __asm__ __volatile__("popf");    \
                         _t_;})
#define get_rip()        (*((UINT32*)(get_ebp() + 4)))

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
typedef struct _TASK_PARMS
{
    UINT32      p1; /**< Parameter 1    */
    UINT32      p2; /**< Parameter 2    */
    UINT32      p3; /**< Parameter 3    */
    UINT32      p4; /**< Parameter 4    */
    UINT32      p5; /**< Parameter 5    */
    UINT32      p6; /**< Parameter 6    */
    UINT32      p7; /**< Parameter 7    */
    UINT32      p8; /**< Parameter 8    */
} TASK_PARMS;

struct sockaddr;

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern UINT32 kernel_sleep;
extern UINT32 kernel_up_counter;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern INT32 XK_InitKernel(void);
extern UINT32 XK_KernelReady(void);

extern PCB *XK_TaskCreate(void (*funcPtr) (TASK_PARMS *), TASK_PARMS *parms, UINT32 shared);
#ifdef M4_ABORT
extern void XK_TaskSwitch(const char *, const unsigned int, const char *);
#else   /* M4_ABORT */
extern void XK_TaskSwitch(void);
#endif  /* M4_ABORT */
extern void XK_TaskSleepMS(UINT32 msec);

extern void *XK_Malloc(UINT32 size, UINT32 type, UINT32 shared, const char *file,
                       const UINT32 line);
extern void XK_Free(void *pMem, const char *file, const UINT32 line);

extern PCB *XK_GetPcb(void);
extern UINT8 *XK_GetStack(PCB * pcb);
extern INT32 XK_GetStackSize(PCB * pcb);

#ifdef LOG_SIMWOOKIEE
#define XK_Select(a,b,c,d,e) ;
#else  /* LOG_SIMWOOKIEE */
extern INT32 XK_Select(INT32 maxSock, fd_set * pRead, fd_set * pWrite,
                       fd_set * pException, struct timeval *pTimeOut);
#endif /* LOG_SIMWOOKIEE */
extern INT32 XK_Accept(INT32 sockFd, struct sockaddr *pPeerAddr, size_t * pAddrLen);
extern INT32 XK_CloseDescriptor(UINT32 type, void *closeFile);

extern void XK_CheckSleepers(void);

extern INT32 XK_System(const char *pCommand);

#if defined(MODEL_7000) || defined(MODEL_4700)
extern INT32 writen(INT32 fd, char *ptr, INT32 nbytes);
#endif /* MODEL_7000 || MODEL_4700 */

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _XK_KERNEL_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
