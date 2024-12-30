/* $Id: nvabe.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       nvabe.h
**
**  @brief      To provide a means of handling NVA records.
**
**              Define common routines and information for handling NVA
**              records. This file also defines an NVA entry structure.
**
**  Copyright (c) 2000 - 2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _NVABE_H_
#define _NVABE_H_

#include "ilt.h"
#include "nva.h"
#include "RL_RDD.h"
#include "rrp.h"
#include "system.h"
#include "XIO_Types.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/*     Note: Px_NVA_NUM must be a multiple of 32 !!!                        */
#define P3_NVA_NUM   ((NVRAM_P3_SIZE - sizeof(NVA_HEADER)) / sizeof(NVA))
                                        /* Number of P3 NVA entries         */

/*  Mark RAID Scan input parameters                                         */
#define SINGLE_RDD          0           /* Single RDD to mark               */
#define ALL_RDDS            1           /* All RDDs owned by controller     */
#define ALL_NOT_MIRROR_RDDS 2           /* All RDDs Marked "Not Mirroring"  */
#define LIST_RDDS           3           /* List of RDDs to mark             */

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void NVA_ClearP3(void);
extern void NVA_SetReSyncAStatus(NVA* pNVA, UINT32 numEntries);
extern void NVA_ClearReSyncAStatus(NVA* pNVA, UINT32 numEntries);
extern bool NVA_CheckReSyncInProgress(void);

#endif /* _NVABE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
