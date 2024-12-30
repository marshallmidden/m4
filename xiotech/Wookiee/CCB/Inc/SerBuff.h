/* $Id: SerBuff.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       SerBuff.h
** MODULE TITLE:    Header file for SerBuff.c
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _SERBUFF_H_
#define _SERBUFF_H_

#include "XIO_Types.h"
#include "xk_kernel.h"

#ifdef __cplusplus
#pragma pack(push, 1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
typedef UINT32 QUEUE_RETURN_TYPE;

#define QUEUE_RETURN_GOOD   0
#define QUEUE_RETURN_ERROR  1
#define QUEUE_RETURN_BUSY   2
#define QUEUE_RETURN_FULL   3
#define QUEUE_RETURN_EMPTY  4

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern UINT32 SerialBufferedReadChar(char *pData);
extern UINT32 SerialBufferedWriteChar(const char *pData);
extern UINT32 SerialBufferedWriteString(const char *pBuffer, UINT32 length);
extern UINT32 SerialBufferedWriteFlush(UINT8 doExchange);
extern UINT32 SerialBufferedGetQueueDepth(UINT8 whichDirection, UINT32 *queueDepthPtr);
extern void SerialUserPortReceiveTask(TASK_PARMS *parms);
extern void SerialUserPortTransmitTask(TASK_PARMS *parms);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _SERBUFF_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
