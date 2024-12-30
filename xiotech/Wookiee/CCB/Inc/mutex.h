/* $Id: mutex.h 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:      mutex.h
** MODULE TITLE:   Mutex support functions
**
** DESCRIPTION:    Mutex (MUTex EXclusion) semaphore support.
**
** Copyright (c) 2001-2010 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#ifndef _MUTEX_H
#define _MUTEX_H

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
enum
{
    Unlocked,
    Locked
} ;

typedef unsigned int MUTEX;

/* Valid 'waitFlag' values for LockMutex() call. */
#define MUTEX_NOWAIT 0
#define MUTEX_WAIT   1

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern INT32 XK_InitMutex(MUTEX *mutex, const char *, const UINT32);
extern int   XK_LockMutex(MUTEX *mutex, int waitFlag, const char *, const UINT32);
extern void  XK_UnlockMutex(MUTEX *mutex);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _MUTEX_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
