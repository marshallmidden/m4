/* $Id: heap.h 143007 2010-06-22 14:48:58Z m4 $ */
/*============================================================================
** FILE NAME:       heap.h
** MODULE TITLE:    Header file for heap.c
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _HEAP_H_
#define _HEAP_H_

#include "XIO_Types.h"
#include "mutex.h"
#include "rtc.h"

#ifdef __cplusplus
#pragma pack(push, 1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
#define MEM_CALLOC 1
#define MEM_MALLOC 0

// The Heap statistics are no longer used, but they are in CCB NVRAM and thus
// the space must continue to exist in the data structure. *boo hiss*

#define NUM_HEAP_STATS_ENTRIES 256
#define NUM_HEAP_STATS_ENTRIES_IN_NVRAM 20

/* The heap control structure definition. */
typedef struct KERNEL_HEAP_STRUCT                               // No longer used
{                                                               // No longer used
    UINT32      fm_s0base;                                      // No longer used
    UINT32      fm_s0len;                                       // No longer used
    UINT32      fm_origin;                                      // No longer used
    UINT32      fm_cur_avl;                                     // No longer used
    UINT32      fm_max_avl;                                     // No longer used
    UINT32      fm_min_avl;                                     // No longer used
    UINT32      fm_waits;                                       // No longer used
    UINT32      fm_chain_len;                                   // No longer used
    UINT32      fm_count;                                       // No longer used
} KERNEL_HEAP;                                                  // No longer used

typedef struct                                                  // No longer used
{                                                               // No longer used
    UINT32      name0;                                          // No longer used
    UINT32      name1;                                          // No longer used
    UINT32      lineNum;                                        // No longer used
    UINT32      count;                                          // No longer used
    UINT32      total;                                          // No longer used
} HEAP_STATS_ENTRY;                                             // No longer used

typedef struct                                                  // No longer used
{                                                               // No longer used
    CHAR        eyecatcher[12];                                 // No longer used
    KERNEL_HEAP heapCB;                                         // No longer used
    TIMESTAMP   ts;                                             // No longer used
    UINT32      numEntries;                                     // No longer used
    HEAP_STATS_ENTRY entry[NUM_HEAP_STATS_ENTRIES];             // No longer used
} HEAP_STATS;                                                   // No longer used

typedef struct                                                  // No longer used
{                                                               // No longer used
    CHAR        eyecatcher[12];                                 // No longer used
    KERNEL_HEAP heapCB;                                         // No longer used
    TIMESTAMP   ts;                                             // No longer used
    UINT32      numEntries;                                     // No longer used
    HEAP_STATS_ENTRY entry[NUM_HEAP_STATS_ENTRIES_IN_NVRAM];    // No longer used
} HEAP_STATS_IN_NVRAM;                                          // No longer used


/*****************************************************************************
** Public variables
*****************************************************************************/
extern HEAP_STATS heapStats;                                    // No longer set reasonably.

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void InitHeaps(void);
extern void FreeDebugWithNullSet(void *, const char *, const UINT32);
extern void Free_Memory_Ptr(void *, UINT32, const char *, const UINT32);
extern void CompileHeapStats(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _HEAP_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
