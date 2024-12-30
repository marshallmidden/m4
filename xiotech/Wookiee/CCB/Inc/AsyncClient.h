/* $Id: AsyncClient.h 143845 2010-07-07 20:51:58Z mdr $*/
/**
******************************************************************************
**
**  @file   AsyncClient.h
**
**  @brief  CCB Client for Async Events
**
**  Client to transfer async events from the CCB to the UMC
**
**  Copyright (c) 2001-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _ASYNCCLIENT_H_
#define _ASYNCCLIENT_H_

#include "logging.h"
#include "pcb.h"
#include "XIO_Types.h"
#include "XIO_Std.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/

/* This structure is put into the NVRAM_STRUCTURE, and thus needs to be present. */
typedef struct _ASYNC_EVENT_QUEUE
{
    SEQ32   seqNum;         /* Sequence # of first event in the "Queue"      */
    UINT32  count;          /* Number of events remaining on the "Queue"     */
    UINT32  rsvd;           /* Resevered                                     */
    UINT32  crc;            /* crc                                           */
} ASYNC_EVENT_QUEUE;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _ASYNCCLIENT_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
