/* $Id: heap.c 145032 2010-08-03 17:01:18Z m4 $ */
/**
******************************************************************************
**
**  @file   heap.c
**
**  @brief  Misc Kernel Services
**
**  Contains the Heap Debug functions.
**
**  Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "heap.h"

#include "idr_structure.h"
#include "error_handler.h"
#include "misc.h"
#include "mode.h"
#include "debug_files.h"
#include "error_handler.h"
#include "XIO_Std.h"
#include "mem_pool.h"

#include <stdlib.h>


HEAP_STATS  heapStats;                      // No longer set reasonably.
extern char local_memory_start;

/*****************************************************************************
** Code Start
*****************************************************************************/

/**********************************************************************
*                                                                     *
*  Name:        FreeDebugWithNullSet()                                *
*                                                                     *
*  Description: In addition to what FreeDebug() does, this function   *
*               checks the incoming pointer for NULL before actually  *
*               trying to free it and sets it to NULL in the user's   *
*               space after freeing it.                               *
*                                                                     *
*  Input:       Pointer to MemoryBlock pointer                        *
*                                                                     *
*  Returns:     void                                                  *
*                                                                     *
**********************************************************************/
void FreeDebugWithNullSet(void *ptrToMemoryBlockPtr, const char *file, const UINT32 line)
{
    void *ptr = (void *)(*(UINT32 *)ptrToMemoryBlockPtr);

    /* Check to see if the pointer is NULL */
    if (ptr)
    {
        struct before_after *b = (struct before_after *)ptr - 1;

        if ((char *)b < &local_memory_start ||
            ((char *)(b + 2)+ b->length) >= ((char *)&local_memory_start + MAKE_DEFS_CCB_ONLY_SIZE))
        {
            s_Free(ptr, b->length, file, line);
        }
        else
        {
            p_Free(ptr, b->length, file, line);
        }

        /* Then set it to NULL. */
        (*(UINT32 *)ptrToMemoryBlockPtr) = 0;
    }
#if 0
    else
    {
        dprintf(DPRINTF_DEFAULT, "Free(): Tried to free a NULL pointer (non-fatal). %s(%u)\n", file, line);
    }
#endif  /* 0 */
}


/**********************************************************************
*
*  Name:        Free_memory_ptr()
*
*  Description: Free either type of memory.
*
*  Input:       MemoryBlockPtr  - Pointer to MemoryBlock
*               length          - length of memory to free (can check if ok)
*               file            - file name
*               line            - line number in file
*
*  Returns:     none
*
**********************************************************************/
void Free_Memory_Ptr(void *MemoryBlockPtr, UINT32 length, const char *file, const UINT32 line)
{
    char *ptr = (char *)MemoryBlockPtr;

    if (ptr < &local_memory_start ||
        ptr + length >= ((char *)&local_memory_start + MAKE_DEFS_CCB_ONLY_SIZE))
    {
        s_Free(ptr, length, file, line);
    }
    else
    {
        p_Free(ptr, length, file, line);
    }
}   /* End of Free_Memory_Ptr */


/**********************************************************************
*                                                                     *
*  Name:        CompileHeapStats()                                    *
*                                                                     *
*  Description: Fake up request for Test scripts.                     *
*                                                                     *
*  Returns:     void                                                  *
*                                                                     *
**********************************************************************/
void CompileHeapStats(void)
{
    /* Get initial stuff */
    memcpy(heapStats.eyecatcher, "HEAPSTATS 01", 12);
    RTC_GetTimeStamp(&heapStats.ts);

    heapStats.heapCB.fm_s0base = 0;
    heapStats.heapCB.fm_s0len = 0;
    heapStats.heapCB.fm_origin = 0;
    heapStats.heapCB.fm_cur_avl = 0;
    heapStats.heapCB.fm_max_avl = 0;
    heapStats.heapCB.fm_min_avl = 0;
    heapStats.heapCB.fm_waits = 0;
    heapStats.heapCB.fm_chain_len = 0;
    heapStats.heapCB.fm_count = 0;

    heapStats.numEntries = 0;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
