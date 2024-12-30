/* $Id: AsyncQueue.h 143020 2010-06-22 18:35:56Z m4 $ */
/**
******************************************************************************
**
**  @file       AsyncQueue.h
**
**  @brief      Async Event Queue Tasks and Queueing functions.
**
**  These task s are started at system boot. One task services the DLM
**  "datagrams" coming in from the FE; the other handles asynchronous events
**  from both the FE and BE.  Queueing functions fro each queue are also
**  provided.
**
**  Copyright (c) 2004-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _ASYNC_QUEUE_H
#define _ASYNC_QUEUE_H

#include "qu.h"
#include "XIO_Std.h"

struct ILT;
struct _TASK_PARMS;

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
typedef void (*ASYNC_EVENT_HANDLER)(void *, void *);

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern QU   logEventQ;
extern QU   dlmEventQ;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void AsyncExecTask(struct _TASK_PARMS *parms);
extern void EnqueLogEvent(struct ILT *pILT);
extern void EnqueDlmEvent(struct ILT *pILT);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _ASYNC_QUEUE_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
