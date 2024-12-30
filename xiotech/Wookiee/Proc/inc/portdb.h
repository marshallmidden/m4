/* $Id: portdb.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       portdb.h
**
**  @brief      Port Database
**
**  To define the data structure for the QLogic Get Port DB data.
**
**  Copyright (c) 2001-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef PORTDB_H
#define PORTDB_H

#include "XIO_Types.h"

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
typedef struct PDB
{
    UINT8  opt;                         /* Options                          */
    UINT8  cnt;                         /* Control                          */
    UINT8  mst;                         /* Master State                     */
    UINT8  sst;                         /* Slave State                      */
    UINT32 had;                         /* Hard address from ADISC          */
    UINT32 pid;                         /* Port ID                          */
    UINT64 ndn;                         /* Node Name                        */
    UINT64 pdn;                         /* Port Name                        */
    UINT16 exThrottle;                  /* Execution Throttle               */
    UINT16 exLimit;                     /* Execution Limit                  */
    UINT8  retryCount;                  /* Retry Count                      */
    UINT8  rsvd1;
    UINT16 resourceAllocation;          /* Resource Allocation              */
    UINT16 currentAllocation;           /* Current Allocation               */
    UINT16 queueHead;                   /* Queue Head                       */
    UINT16 queueTail;                   /* Queue Tail                       */
    UINT16 xmitExecListNext;            /* Transmit Exection List Next      */
    UINT16 xmitExecListPrev;            /* Transmit Exection List Previous  */
    UINT16 commonFeatures;              /* Common Features                  */
    UINT16 totalConcurrentSeq;          /* Total concurrent sequences       */
    UINT16 relativeOffset;              /* Relative offset by info cat flag */
    UINT16 receiptControlFlags;         /* receipt control flags            */
    UINT16 recvDataSize;                /* Receive data size                */
    UINT16 concurrentSeq;               /* Concurrent sequences             */
    UINT16 openSeq;                     /* Number of Port (retry) timer     */
    UINT16 rsvd2[4];
    UINT16 portTimer;                   /* Port (retry) timer               */
    UINT16 nextSeqID;                   /* Next sequence ID                 */
    UINT16 frameCount;                  /* Frame Count                      */
    UINT16 prliPayloadLen;              /* prli payload length              */
    UINT16 prliw0;                      /* prli ser parm word 0             */
    UINT16 prliw3;                      /* prli ser parm word 3             */
    UINT16 lid;                         /* Loop ID                          */
    UINT16 lunlist;                     /* Extended LUN List ptr            */
    UINT16 lunstop;                     /* Extended LUN Stop ptr            */
    UINT16 spare;
} PDB;

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern PDB  *portdb[MAX_PORTS];

#endif /* PORTDB_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
