/* $Id: memory.c 159870 2012-09-20 12:59:51Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       memory.c
**
**  @name       General memory allocation/free.
**
**  Copyright (c) 2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "XIO_Const.h"
#include "XIO_Std.h"
#include "XIO_Macros.h"
#include <stdio.h>
#include <string.h>
#include "mem_pool.h"
#include "memory.h"
#ifndef CCB_RUNTIME_CODE
#include "CT_defines.h"
#else   /* CCB_RUNTIME_CODE */
#include "CT_history.h"
extern void k_init_mem(struct fmm *, void *, UINT32);
extern UINT32 SHMEM_END;
#endif  /* CCB_RUNTIME_CODE */

/* ------------------------------------------------------------------------ */
#ifndef PERF
// Probably want to define these in the Makefile -- this is FE and BE and CCB.
//  #define CHECK_MEMORY_ALL
//  #define MEMSET_PATTERN_FREE
//  #define REALLY_DELAY_MEMORY_FREES
#endif  /* PERF */

/* ------------------------------------------------------------------------ */
#ifdef CCB_RUNTIME_CODE
#define FEBEMESSAGE             "CCB "
#endif  /* CCB_RUNTIME_CODE */
/* ------------------------------------------------------------------------ */

#ifdef CHECK_MEMORY_ALL
static void check_memory_links(struct fmm *, void *, size_t);
void check_memory_all(void);
#endif  /* CHECK_MEMORY_ALL */

#ifdef FRONTEND
#define PRIVATE_SIZE    MAKE_DEFS_FE_ONLY_SIZE
#define check_shared_memory         check_memory_links(&K_ncdram, NcdrAddr, NcdrSize);
#define check_private_memory        check_memory_links(&P_ram, &local_memory_start, MAKE_DEFS_FE_ONLY_SIZE);
#define check_write_cache_memory    check_memory_links(&c_wcfmm, WcbAddr, WcbSize);
extern UINT8 *WcbAddr;
extern UINT32 WcbSize;
#endif  /* FRONTEND */

#ifdef BACKEND
#define PRIVATE_SIZE    MAKE_DEFS_BE_ONLY_SIZE
#define check_shared_memory         check_memory_links(&K_ncdram, NcdrAddr, NcdrSize);
#define check_private_memory        check_memory_links(&P_ram, &local_memory_start, MAKE_DEFS_BE_ONLY_SIZE);
#endif  /* BACKEND */

#ifdef CCB_RUNTIME_CODE
#define PRIVATE_SIZE    MAKE_DEFS_CCB_ONLY_SIZE
#define check_shared_memory         check_memory_links(&K_ncdram, &SHMEM_END, SIZE_CCB_LTH - ((UINT32)&SHMEM_END - CCB_PCI_START));
#define check_private_memory        check_memory_links(&P_ram, &local_memory_start, MAKE_DEFS_CCB_ONLY_SIZE);
#endif  /* CCB_RUNTIME_CODE */

/* ------------------------------------------------------------------------ */
extern char *NcdrAddr;                          /* Start of shared memory. */
extern UINT32 NcdrSize;                         /* Size of shared memory. */
extern char local_memory_start;                 /* Start of private memory. */
extern unsigned int local_memory_pool_start;    /* Size of private memory. */
extern char Out_of_local_memory_print;          /* Zero if we never ran out of private memory. */

/* ------------------------------------------------------------------------ */
/* Prototype functions. */
#ifdef FRONTEND
void s_mrel_fmm(void *, UINT32, const char *, unsigned int);
#endif  /* FRONTEND */
void *k_malloc(UINT32, struct fmm *, const char *, unsigned int);
void k_mrel(void *, UINT32, struct fmm *, const char *, unsigned int);
void release_deferred_memory(void);
void k_make_pre_post_headers(void *, UINT32);

/* ------------------------------------------------------------------------ */
/* Data structures defined outside this file. */
extern struct fmm K_ncdram;
#ifdef FRONTEND
extern struct fmm c_wcfmm;
#endif  /* FRONTEND */
#ifndef CCB_RUNTIME_CODE
extern PCB *K_delayed_pcb_free;
extern void free_pcb(struct PCB *);
#endif  /* CCB_RUNTIME_CODE */

/* ------------------------------------------------------------------------ */
/* Private data structure used only in this file. */
struct fmm  P_ram;              /* Free memory management structure. */
struct fms  P_cur;              /* Free memory statistics structure. */

/* ------------------------------------------------------------------------ */
#ifdef REALLY_DELAY_MEMORY_FREES
static struct before_after *shared_delayed_ptr_head = 0;
static struct before_after *shared_delayed_ptr_tail = 0;
static struct before_after *private_delayed_ptr_head = 0;
static struct before_after *private_delayed_ptr_tail = 0;

// The following macros are to ensure that private and shared are done the same way.
#define save_delayed_memory_free(delayed_ptr, address, t_bytes, file, line)         \
    {                                                                               \
        check_on_local_memory_address(address, t_bytes);                            \
        struct before_after *new_entry = (struct before_after *)address - 1;        \
        const char *last_slash = strrchr(file, '/');                                \
        last_slash = (last_slash == NULL) ? file : last_slash + 1;                  \
        strncpy(new_entry->str, last_slash, sizeof(new_entry->str));                \
        new_entry->line_number = line;                                              \
        if (new_entry->used_or_free != 1) { abort(); }                              \
        if (new_entry->length != t_bytes) { abort(); }                              \
        new_entry->used_or_free = 2;                                                \
        memset(address, 0xff, t_bytes);                                             \
        new_entry->next = 0;                                                        \
        if (delayed_ptr##_head == 0)                                                \
        {                                                                           \
            delayed_ptr##_head = new_entry;                                         \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            delayed_ptr##_tail->next = new_entry;                                   \
        }                                                                           \
        delayed_ptr##_tail = new_entry;                                             \
    }

#define get_tsc()       ({ unsigned long long __scr;                                \
        __asm__ __volatile__("rdtsc" : "=A" (__scr)); __scr;})
#ifdef CCB_RUNTIME_CODE
extern UINT64 xkCpuSpeed;           /* Speed of cpu (like 3200 = 3.2gHz) */
#define TSC_ONE_MSEC_DELAY             (xkCpuSpeed * 1000)
#else   /* CCB_RUNTIME_CODE */
#define TSC_ONE_MSEC_DELAY             (2333ULL * 1000)    /* xkCpuSpeed on 7000 */
#endif  /* CCB_RUNTIME_CODE */

#define init_tsc_check      UINT64 tsc_check = get_tsc()

#define put_shared_delayed_memory(need_bytes)                                       \
    {                                                                               \
      UINT32 free_count = 0;        /* Free a bunch at a time - faster, cluster? */ \
      while (shared_delayed_ptr_head != 0)                                          \
      {                                                                             \
          char *release_addr = (char *)(shared_delayed_ptr_head + 1);               \
          UINT32 delayed_lth = shared_delayed_ptr_head->length;                     \
          UINT32 delayed_lth_loop = (delayed_lth < 1024) ? delayed_lth : 1024;      \
                                                                                    \
          if (shared_delayed_ptr_head->used_or_free != 2) { abort(); }              \
          shared_delayed_ptr_head->used_or_free = 1;                                \
          shared_delayed_ptr_head = shared_delayed_ptr_head->next;                  \
          UINT8 *chk = (UINT8 *)release_addr;                                       \
          UINT32 ix;                                                                \
          for (ix = 0; ix < delayed_lth_loop; ix++)                                 \
          { if (*chk != 0xff) { abort(); } chk++; }                                 \
          k_mrel(release_addr, delayed_lth, &K_ncdram, file, line);                 \
          free_count += delayed_lth;                                                \
          if ((get_tsc() - tsc_check) > (TSC_ONE_MSEC_DELAY * 1000/8)) { TaskSwitch();  tsc_check = get_tsc();} \
          if (free_count < (need_bytes)) { continue; }                              \
          break;                                                                    \
      }                                                                             \
      if (free_count != 0) { continue; }                                            \
    }

#if !defined(PERF) && !defined(CCB_RUNTIME_CODE)
#define check_address_in_range                                                      \
                if ((char *)release_addr < NcdrAddr || ((char *)release_addr + delayed_lth) >= (NcdrAddr + NcdrSize)) \
                {                                                                   \
                    fprintf(stderr, "%sERROR %s -- release_addr (%p + %u) < %p  or >= %p\n", FEBEMESSAGE, __func__, release_addr, delayed_lth, NcdrAddr, NcdrAddr + NcdrSize); \
                    abort();                                                        \
                }
#else   /* !PERF && !CCB_RUNTIME_CODE */
#define check_address_in_range
#endif  /* !PERF && !CCB_RUNTIME_CODE */

#define put_private_delayed_memory(need_bytes)                                      \
    {                                                                               \
      UINT32 free_count = 0;        /* Free a bunch at a time - faster, cluster? */ \
      while (private_delayed_ptr_head != 0)                                         \
      {                                                                             \
          char *release_addr = (char *)(private_delayed_ptr_head + 1);              \
          UINT32 delayed_lth = private_delayed_ptr_head->length;                    \
          UINT32 delayed_lth_loop = (delayed_lth < 1024) ? delayed_lth : 1024;      \
                                                                                    \
          if (private_delayed_ptr_head->used_or_free != 2) { abort(); }             \
          private_delayed_ptr_head->used_or_free = 1;                               \
          private_delayed_ptr_head = private_delayed_ptr_head->next;                \
          UINT8 *chk = (UINT8 *)release_addr;                                       \
          UINT32 ix;                                                                \
          for (ix = 0; ix < delayed_lth_loop; ix++)                                 \
          { if (*chk != 0xff) { abort(); } chk++; }                                 \
          if ((char *)release_addr >= &local_memory_start && (char *)release_addr < ((char *)&local_memory_start + PRIVATE_SIZE)) \
          {                                                                         \
              k_mrel(release_addr, delayed_lth, &P_ram, file, line);                \
          }                                                                         \
          else                                                                      \
          {                                                                         \
              check_address_in_range;                                               \
              k_mrel(release_addr, delayed_lth, &K_ncdram, file, line);             \
          }                                                                         \
          free_count += delayed_lth;                                                \
          if ((get_tsc() - tsc_check) > (TSC_ONE_MSEC_DELAY * 1000/8)) { TaskSwitch();  tsc_check = get_tsc();} \
          if (free_count < (need_bytes)) { continue; }                              \
          break;                                                                    \
      }                                                                             \
      if (free_count != 0) { continue; }                                            \
    }
#endif  /* REALLY_DELAY_MEMORY_FREES */

/* ------------------------------------------------------------------------ */
/* Code starts here. */

#ifndef CCB_RUNTIME_CODE
/**
******************************************************************************
**
**  @name       s_MallocC
**
**  @brief      To provide a common means of assigning and clearing memory.
**
**              The memory chain is searched for a suitable assignment. If
**              assignment is made the address is returned. If not, the task
**              is put into memory wait state. The call is reissued when
**              memory is released.
**
**  @param      t_bytes - number of bytes, with upper bit set for permanent
**                        assignment.
**
**  @return     assigned address
**
******************************************************************************
**/

void       *s_MallocC(UINT32 t_bytes, const char *file, unsigned int line)
{
    void       *address;
#ifdef REALLY_DELAY_MEMORY_FREES
    init_tsc_check;
#endif  /* REALLY_DELAY_MEMORY_FREES */

#ifndef PERF
    if ((t_bytes & 0x7fffffff) == 0)
    {
        abort();
    }
#endif  /* PERF */

#ifdef CHECK_MEMORY_ALL
    check_shared_memory;
#endif  /* CHECK_MEMORY_ALL */

    do
    {
        address = k_malloc(t_bytes, &K_ncdram, file, line);     /* FMM is K_ncdram. */
        if (address != NULL)    /* Memory allocated, exit. */
        {
            memset(address, 0, t_bytes & 0x7fffffff);   /* Clear memory. */
            return (address);
        }

#ifdef REALLY_DELAY_MEMORY_FREES
        put_shared_delayed_memory(t_bytes & 0x7fffffff);             // Note continue in macro.
#endif  /* REALLY_DELAY_MEMORY_FREES */

        K_ncdram.fmm_fms->fms_Number_tasks_waiting++;   /* Increment memory wait. */
        TaskSetState(K_xpcb, K_ncdram.fmm_waitstat);    /* Place this process in wait state. */
        release_deferred_memory();      /* If any deferred memory to free, do it. */
        TaskSwitch();           /* Relinquish running to others, and then
                                 * continue if/when memory available. */
    } while (1);
}                               /* End of s_MallocC */


/**
******************************************************************************
**
**  @name       s_MallocW
**
**  @brief      To provide a common means of assigning memory.
**
**              The memory chain is searched for a suitable assignment. If
**              assignment is made the address is returned. If not, the task
**              is put into memory wait state. The call is reissued when
**              memory is released.
**
**  @param      t_bytes - number of bytes, with upper bit set for permanent
**                        assignment.
**
**  @return     assigned address
**
******************************************************************************
**/

void       *s_MallocW(UINT32 t_bytes, const char *file, unsigned int line)
{
    void       *address;
#ifdef REALLY_DELAY_MEMORY_FREES
    init_tsc_check;
#endif  /* REALLY_DELAY_MEMORY_FREES */

#ifndef PERF
    if ((t_bytes & 0x7fffffff) == 0)
    {
        abort();
    }
#endif  /* PERF */

#ifdef CHECK_MEMORY_ALL
    check_shared_memory;
#endif /* CHECK_MEMORY_ALL */

    do
    {
        address = k_malloc(t_bytes, &K_ncdram, file, line);     /* FMM is K_ncdram. */
        if (address != NULL)    /* Memory allocated, exit. */
        {
            return (address);
        }

#ifdef REALLY_DELAY_MEMORY_FREES
        put_shared_delayed_memory(t_bytes & 0x7fffffff);             // Note continue in macro.
#endif  /* REALLY_DELAY_MEMORY_FREES */

        K_ncdram.fmm_fms->fms_Number_tasks_waiting++;   /* Increment memory wait. */
        TaskSetState(K_xpcb, K_ncdram.fmm_waitstat);    /* Place this process in wait state. */
        release_deferred_memory();      /* If any deferred memory to free, do it. */
        TaskSwitch();           /* Relinquish running to others, and then
                                 * continue if/when memory available. */
    } while (1);
}                               /* End of s_MallocW */
#endif  /* CCB_RUNTIME_CODE */


/**
******************************************************************************
**
**  @name       s_Malloc
**
**  @brief      To provide a common means of assigning memory.
**
**              The memory chain is searched for a suitable assignment. If
**              assignment is made the address is returned. If not, a NULL
**              (zero, or 0) is returned.
**
**  @param      t_bytes - number of bytes, with upper bit set for permanent
**                        assignment.
**
**  @return     assigned address, or NULL (0) if none available.
**
******************************************************************************
**/

void       *s_Malloc(UINT32 t_bytes, const char *file, unsigned int line)
{
    void       *address;
#ifdef REALLY_DELAY_MEMORY_FREES
    init_tsc_check;
#endif  /* REALLY_DELAY_MEMORY_FREES */

#ifndef PERF
    if ((t_bytes & 0x7fffffff) == 0)
    {
        abort();
    }
#endif  /* PERF */

#ifdef CHECK_MEMORY_ALL
    check_shared_memory;
#endif  /* CHECK_MEMORY_ALL */

#ifdef REALLY_DELAY_MEMORY_FREES
    do
    {
#endif  /* REALLY_DELAY_MEMORY_FREES */
        address = k_malloc(t_bytes, &K_ncdram, file, line);     /* FMM is K_ncdram. */
#ifdef REALLY_DELAY_MEMORY_FREES
        if (address == NULL)
        {
            put_shared_delayed_memory(t_bytes & 0x7fffffff);                 // Note continue in macro.
        }
        break;
    } while (1);
#endif  /* REALLY_DELAY_MEMORY_FREES */

    return (address);
}                               /* End of s_Malloc */


/* ------------------------------------------------------------------------ */
#ifdef CHECK_MEMORY_ALL

/*
   There are three types of memory usage to check here:
   a->  All the memory, in order, to make sure that pre/post are right, an all is used.
   b-> Free memory list, to make sure pre/post and middle is all right.
   c-> Check almost freed memory has pattern 0xff set.
 */

static void check_memory_links(struct fmm *fmm, void *start_addr, size_t memory_size)
{
    char       *start = (char *)start_addr;     // Start of the next "pre" to check.
    size_t      lth = 0;                        // How far we have gone in the memory.
    size_t      in_size;
    size_t      size;
    size_t      round_up_size;
    size_t      total_size;
    struct before_after *pre;
    struct before_after *post;
    void       *address;
    size_t      total_counted = 0;

#ifdef HISTORY_KEEP
    if (CT_NO_HISTORY == 1)
    {
        return;
    }
#endif  /* HISTORY_KEEP */
// a-> All the memory, in order, to make sure that pre/post are right, an all is used.
    start = (char *)(((UINT32)start & ~0x3f) + EXTRA_SPACE);
    if (*(UINT32 *)start == 0 ||
        (fmm->fmm_first.thd == 0 && fmm->fmm_first.len == 0 && fmm->fmm_fms == 0))
    {
        return;                                 // If not initialized, don't check.
    }
    memory_size = (memory_size - (start - (char *)start_addr)) & ~(ROUND_UP - 1);

    /* Check all of memory section to see if it is somewhat reasonable. */
    while (lth < memory_size)
    {
        pre = (struct before_after *)start;
        in_size = pre->length;
        size = in_size < MIN_LTH ? MIN_LTH : in_size;
        if (in_size == 0 || size > memory_size - lth)
        {
            fprintf(stderr, "%s%s:%s:%u Address %p length %u is not reasonable.\n",
                    FEBEMESSAGE, __FILE__, __func__, __LINE__, start, in_size);
            abort();
        }
        address = (void *)(pre + 1);
        check_on_local_memory_address(address, in_size);
        round_up_size = (size + ROUND_UP - 1) & ~(ROUND_UP - 1);
        total_size = round_up_size + EXTRA_SPACE + EXTRA_SPACE;

        total_counted += total_size;

        start = start + total_size;
        lth = lth + total_size;
    }

    if (total_counted != memory_size)
    {
        fprintf(stderr, "%s%s:%s:%u ERROR - total_counted (%u) != passed in size (%u)\n",
                FEBEMESSAGE, __FILE__, __func__, __LINE__, total_counted, memory_size);
        abort();
    }

    /* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
// b-> Free memory list, to make sure pre/post and middle is all right.
    /* Check general purpose free memory section. */
    pre = (struct before_after *)fmm->fmm_first.thd;
    total_counted = 0;

    if (pre == 0)                               // Out of memory -- this is an error, really!
    {
        abort();
    }

    while (pre != 0)
    {
        in_size = pre->length;
        size = in_size < MIN_LTH ? MIN_LTH : in_size;
        address = (void *)(pre + 1);
        round_up_size = (size + ROUND_UP - 1) & ~(ROUND_UP - 1);
        total_size = round_up_size + EXTRA_SPACE + EXTRA_SPACE;

        total_counted += total_size;

        post = (struct before_after *)((unsigned char *)address + round_up_size);

        if (pre->used_or_free != 0 || post->used_or_free != 0)
        {
            fprintf(stderr, "%s%s:%s:%u address %p not free? (%d/%d)\n",
                    FEBEMESSAGE, __FILE__, __func__, __LINE__,
                    address, pre->used_or_free, post->used_or_free);
            abort();
        }

#ifdef MEMSET_PATTERN_FREE
        size_t      i;
        size_t      j;

        j = (round_up_size < 64 * 1024) ? round_up_size : 64 * 1024;
        for (i = 0; i < j; i++)
        {
            if (*((unsigned char *)address + i) != (unsigned char)0xFE)
            {
                fprintf(stderr, "%s%s:%s:%u ERROR - checking free memory 0xFE -- %p is not all 0xFE (+%d = 0x%02x)\n",
                        FEBEMESSAGE, __FILE__, __func__, __LINE__,
                        address, i, *((unsigned char *)address + i));
                abort();
            }
        }
#endif  /* MEMSET_PATTERN_FREE */

        address = pre->next;
        if (address != 0)
        {
            pre = (struct before_after *)address - 1;
        }
        else
        {
            pre = 0;
        }
    }
    if (total_counted != fmm->fmm_fms->fms_Available_memory)
    {
        fprintf(stderr, "%s%s:%s:%u ERROR - total_counted (%u) != fms_Available_memory (%u)\n",
                FEBEMESSAGE, __FILE__, __func__, __LINE__,
                total_counted, fmm->fmm_fms->fms_Available_memory);
        abort();
    }
}                               /* End of check_memory_links */

void check_memory_all(void)
{
//    static int only_so_often = 0;
//
//    if (only_so_often++ % 100 != 0)
//    {
//        return;
//    }

    check_shared_memory;
    check_private_memory;
#ifdef FRONTEND
    check_write_cache_memory;
#endif  /* FRONTEND */
}                               /* End of check_memory_all */

/* ------------------------------------------------------------------------ */
#endif  /* CHECK_MEMORY_ALL */

/**
******************************************************************************
**
**  @name       k_malloc
**
**  @brief      To provide a common means of assigning memory from a struct fmm.
**
**  The memory chain is searched for a suitable assignment. If assignment is
**  made the address is returned. If not, a NULL (zero, or 0) is returned.
**
**  The allocation algorithm takes into account whether the assignment is
**  designated permanent or temporary. If the assignment is temporary, the
**  assignment is made from the first fit at the lowest possible address. If
**  the assignment is permanent, the assignment is made from the highest
**  possible address within the last fit. This technique serves to minimize
**  the fragmentation of the memory chain.
**
**  @param      t_bytes - number of bytes, with upper bit set for permanent.
**  @param      fmm     - The struct fmm (free memory management structure).
**
**  @return     assigned address, or NULL (0) if none available.
**
******************************************************************************
**/

void       *k_malloc(UINT32 t_bytes, struct fmm *fmm, const char *file, unsigned int line)
{
    struct fms *fms = fmm->fmm_fms;
    UINT32      in_size = t_bytes & 0x7fffffff;
    struct before_after *previous_pre;
    struct before_after *current_pre;
    struct before_after *next_pre;
    void       *next_memory;
    void       *address;
    void       *candidate;          // Placed here so gdb can see it.
    struct before_after *candidate_pre;
    UINT32      current_length;
    struct before_after *tmp;

#ifdef HISTORY_KEEP
    CT_history1("entering k_malloc wanting memory", t_bytes);
#endif  /* HISTORY_KEEP */

#ifndef CCB_RUNTIME_CODE
    /* If permanent assignment, free deferred free memory now. */
    if (in_size != t_bytes)
    {
        /* Release all deferred free mem, before attempting assignment. */
        release_deferred_memory();      /* If any deferred memory to free, do it. */
    }
#endif  /* CCB_RUNTIME_CODE */

    size_t      size = in_size < MIN_LTH ? MIN_LTH : in_size;
    size_t      round_up_size = (size + ROUND_UP - 1) & ~(ROUND_UP - 1);
    size_t      total_size = round_up_size + EXTRA_SPACE + EXTRA_SPACE;

    /*
     * Get pointer to first memory pre -- special check this value (left-over
     * legacy) needs. This is a pre to start with, if not below value.
     */
    current_pre = (struct before_after *)&fmm->fmm_first;

    /* K_ncdram has a pointer to the first memory, and the size of itself
     * is zero. Thus, the real memory segment is pointed to by the next
     * one, so grab it. */
    next_pre = (void *)fmm->fmm_first.thd;
    if (next_pre == NULL)
    {
#ifdef HISTORY_KEEP
        CT_history1("exiting k_malloc, no memory available at all", 0);
#endif  /* HISTORY_KEEP */
        return (NULL);          /* Indicate no memory available. */
    }
    next_memory = (void *)(next_pre + 1);

    /* If top bit is clear, then temporary assignment (most common). */
    if (in_size == t_bytes)
    {
        /* Go through the linked list, finding the first section that exactly
         * matches in size, or chop big one into two and return first. */
        while (1)
        {
            if (next_memory == NULL)
            {
#ifdef HISTORY_KEEP
                CT_history1("exiting k_malloc without memory", 0);
#endif  /* HISTORY_KEEP */
                return (NULL);  /* Indicate no memory available. */
            }

            /* Save current into previous, next into current. */
            previous_pre = current_pre;
            current_pre = (struct before_after *)next_memory - 1;

            /* Get next memory segment, and current segment length. */
            current_length = current_pre->length;
            next_memory = current_pre->next;    // point to memory, not pre.

            /* NOTE: do not use if too small or would leave unuseable chunk. */
            if (current_length < round_up_size)
            {
                continue;
            }
            /* c == r okay, c <= r+32+32 bad */
            if ((round_up_size != current_length) &&
                (current_length - round_up_size <= EXTRA_SPACE + EXTRA_SPACE))
            {
                continue;
            }

            /* If too large a section, use first portion, re-link in rest. */
            if (current_length > total_size)
            {
                /* Chop into two sections. Form new pre. */
                tmp = (void *)((char *)current_pre + total_size);

                /* Update current segment length. */
                current_length -= total_size;

                /* Fill in the pre allocation pattern. */
                tmp->pre1 = tmp->pre2 = 0xdddddddd;
                memset(&tmp->str, 0, sizeof(tmp->str));
                tmp->line_number = 0;
                tmp->used_or_free = 0;      // flag that this is free.
                tmp->next = next_memory;
                tmp->length = current_length;

                /* Update segment. */
                next_memory = (void *)(tmp + 1);
                next_pre = tmp;

                /* Fill in the old post allocation changed length. */
                tmp = (struct before_after *)((char *)(tmp + 1) + current_length);
                tmp->length = current_length;
            }
            else
            {
                next_pre = (next_memory != 0) ? (struct before_after *)next_memory - 1 : 0;
            }

            /* Unlink this segment, previous will now point to next. */
            if (previous_pre == (struct before_after *)&fmm->fmm_first)
            {
                fmm->fmm_first.thd = (struct mc *)next_pre;
            }
            else
            {
                previous_pre->next = next_memory;
            }
            address = (void *)current_pre;      // Adjusted later in routine to point to right place.
            /* Save secondary origin -- faster free? (Quick malloc-use-free.) */
            fmm->fmm_sorg = (struct mc *)previous_pre;
            break;
        }
    }
    else                        /* Process permanent assignment (least common method). */
    {
        /* No previous segment, nor candidate segment yet. */
        candidate = NULL;
        previous_pre = NULL;

        /* Scan memory chain for last possible candidate (highest segment that is big enough). */
        while (1)
        {
            /* If end of memory chain. */
            if (next_memory == NULL || next_memory == (void *)EXTRA_SPACE)
            {
                /* If no candidate to get memory from. */
                if (candidate == NULL)
                {
#ifdef HISTORY_KEEP
                    CT_history1("exiting k_malloc, no segment big enough ", 0);
#endif  /* HISTORY_KEEP */
                    return (NULL);      /* Indicate no memory available. */
                }

                /* If exact match. */
                candidate_pre = (struct before_after *)candidate - 1;
                if (candidate_pre->length == round_up_size)
                {
                    /* Link previous segment to next segment. */
                    address = (void *)candidate_pre;    // Adjusted later in routine.

                    if (previous_pre == (struct before_after *)&fmm->fmm_first)
                    {
                        if (candidate_pre->next == 0)
                        {
                            fmm->fmm_first.thd = (struct mc *)0;
                        }
                        else
                        {
                            fmm->fmm_first.thd = (struct mc *)((struct before_after *)candidate_pre->next - 1);
                        }
                    }
                    else
                    {
                        previous_pre->next = candidate_pre->next;
                    }
                }
                else
                {
                    /* Calculate and set new segment length. */
                    candidate_pre->length -= total_size;

                    /* Perform assignment from highest portion of this segment. */
                    /* Start + length (without headers) + our header/trailer. */
                    /* Note: address adjusted later in routine. */
                    address = (void *)((char *)candidate_pre + candidate_pre->length + EXTRA_SPACE + EXTRA_SPACE);

                    /* Set post pattern. */
                    tmp = (struct before_after *)address - 1;

                    tmp->pre1 = tmp->pre2 = 0xdcdcdcdc;
                    memset(&tmp->str, 0, sizeof(tmp->str));
                    tmp->line_number = 0;
                    tmp->used_or_free = 0;      // flag that this is not used (i.e. free)
                    tmp->next = (void *)0xdcdcdcdc;
                    tmp->length = candidate_pre->length;
                }
                fmm->fmm_sorg = NULL;   /* No secondary origin. */
                break;
            }

            next_pre = (struct before_after *)next_memory - 1;

/* Do not leave a chunk unusable (pre/post header). */
            if (next_pre->length == round_up_size ||
                next_pre->length >= (round_up_size + EXTRA_SPACE + EXTRA_SPACE + EXTRA_SPACE))
            {
                /* Set possible candidate segment. */
                candidate = next_memory;        /* This is real address, not "pre". */
                /* Save previous segment. */
                previous_pre = current_pre;
            }
            /* To next segment. */
            current_pre = next_pre;
            /* Get next segment and current segment length. */
            next_memory = current_pre->next;
        }
    }

    /* Update K_II statistics */

    /* Adjust current memory by allocated. */
    fms->fms_Available_memory -= total_size;

    /* Make sure minimum since running gets set if needed. */
    if (fms->fms_Minimum_available > fms->fms_Available_memory)
    {
        fms->fms_Minimum_available = fms->fms_Available_memory;
    }

    struct before_after *pre;
    struct before_after *post;

    pre = address;
    address = (char *)(pre + 1);
    post = (struct before_after *)((char *)address + round_up_size);

    /* Fill in the pre and post allocation patterns. */
    pre->pre1 = pre->pre2 = 0xdddddddd;
    memset(&pre->str, 0, sizeof(pre->str));
    const char *last_slash = strrchr(file, '/');

    last_slash = (last_slash == NULL) ? file : last_slash + 1;
    strncpy(pre->str, last_slash, sizeof(pre->str));
    pre->line_number = line;
    pre->used_or_free = 1;          // flag that this is used (for immediate free).
    pre->next = (void *)0xdddddddd;
    pre->length = in_size;

    post->pre1 = post->pre2 = 0xdcdcdcdc;
    memset(&post->str, 0, sizeof(post->str));
    strncpy(post->str, last_slash, sizeof(post->str));
    post->used_or_free = 1;         // flag that this is used.
    post->line_number = line;
    post->next = (void *)0xdcdcdcdc;
    post->length = in_size;

    /* Set anything extra (64 byte minimum allocations) to E's. */
    if (round_up_size - in_size > 0)
    {
        memset((char *)address + in_size, 0x44, round_up_size - in_size);
    }

#ifdef CHECK_MEMORY_ALL
    check_shared_memory;
#endif  /* CHECK_MEMORY_ALL */
#ifdef HISTORY_KEEP
    CT_history1("exiting k_malloc with memory", (UINT32)address);
#endif  /* HISTORY_KEEP */

    return (address);
}                               /* End of k_malloc */


/**
******************************************************************************
**
**  @name       s_Free
**
**  @brief      To free shared memory.
**
**  @param      address - Start of memory segment to free.
**  @param      t_bytes - Number of bytes.
**
**  @return     none
**
******************************************************************************
**/

void s_Free(void *address, UINT32 t_bytes, const char *file, unsigned int line)
{
#if defined(HISTORY_KEEP) && defined(REALLY_DELAY_MEMORY_FREES)
    CT_history1("entering s_Free with memory", (UINT32)address);
#endif  /* HISTORY_KEEP && REALLY_DELAY_MEMORY_FREES */
#ifndef PERF
    if ((t_bytes & 0x7fffffff) == 0)
    {
        abort();
    }
#endif  /* PERF */
#if !defined(PERF) && !defined(CCB_RUNTIME_CODE)
    if ((char *)address < NcdrAddr ||
        ((char *)address + t_bytes) >= (NcdrAddr + NcdrSize))
    {
        fprintf(stderr, "%sERROR %s -- address (%p + %u) < %p  or >= %p\n",
                FEBEMESSAGE, __func__, address, t_bytes, NcdrAddr, NcdrAddr + NcdrSize);
        abort();
    }
#endif  /* !PERF && !CCB_RUNTIME_CODE */

#ifdef REALLY_DELAY_MEMORY_FREES
    save_delayed_memory_free(shared_delayed_ptr, address, t_bytes, file, line);
#else   /* REALLY_DELAY_MEMORY_FREES */
    k_mrel(address, t_bytes, &K_ncdram, file, line);
#endif  /* REALLY_DELAY_MEMORY_FREES */

#ifdef CHECK_MEMORY_ALL
    check_shared_memory;
#endif  /* CHECK_MEMORY_ALL */
}                               /* End of s_Free */


#ifndef CCB_RUNTIME_CODE

/**
******************************************************************************
**
**  @name       s_Free_and_zero
**
**  @brief      To free shared memory and zero on free.
**
**  @param      address - Start of memory segment to free.
**  @param      t_bytes - Number of bytes.
**
**  @return     none
**
******************************************************************************
**/

void s_Free_and_zero(void *address, UINT32 t_bytes, const char *file, unsigned int line)
{
    memset(address, 0, t_bytes);
    s_Free(address, t_bytes, file, line);
}                               /* End of s_Free_and_zero */


/**
******************************************************************************
**
**  @name       release_deferred_memory
**
**  @brief      Free possible last process control block in deferred release.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/

void release_deferred_memory(void)
{
    if (K_delayed_pcb_free)
    {
        free_pcb(K_delayed_pcb_free);
        K_delayed_pcb_free = NULL;
    }
}                               /* End of release_deferred_memory */
#endif  /* CCB_RUNTIME_CODE */

#ifdef FRONTEND
/**
******************************************************************************
**
**  @name       mrel_fmm
**
**  @brief      To provide a common means of freeing memory write cache memory.
**
**              The provided memory is inserted back into the free memory
**              chain, in the sorted order. It will consolidate segments that
**              this one is next to, and if in the middle of two.
**
**  @param      address - Start of memory segment to free.
**  @param      t_bytes - Number of bytes.
**
**  @return     none
**
******************************************************************************
**/

void s_mrel_fmm(void *address, UINT32 t_bytes, const char *file, unsigned int line)
{
    k_mrel(address, t_bytes, &c_wcfmm, file, line);
}                               /* End of s_mrel_fmm */
#endif  /* FRONTEND */


/**
******************************************************************************
**
**  @name       k_mrel
**
**  @brief      To provide a common means of freeing memory into an fmm structure.
**
**              The provided memory is inserted back into the free memory
**              chain, in the sorted order. It will consolidate segments that
**              this one is next to, and if in the middle of two.
**
**  @param      address - Start of memory segment to free.
**  @param      t_bytes - Number of bytes.
**  @param      fmm     - Free Memory Management structure to initialize.
**
**  @return     none
**
******************************************************************************
**/

void k_mrel(void *address, UINT32 t_bytes, struct fmm *fmm, const char *file UNUSED,
            unsigned int line UNUSED)
{
    struct fms *fms = fmm->fmm_fms;
    void       *last_plus_one;
    struct before_after *next_pre;
    void       *next_address;
    struct before_after *current_pre;
    UINT32      current_length;
    char       *current_lba_plus1;
    struct before_after *previous_pre;
    UINT32      previous_length;
    char       *previous_lba_plus1;

#ifdef HISTORY_KEEP
    CT_history_memory_size("entering k_mrel", (UINT32)address, t_bytes);
#endif  /* HISTORY_KEEP */

    size_t      size = t_bytes < MIN_LTH ? MIN_LTH : t_bytes;
    size_t      round_up_size = (size + ROUND_UP - 1) & ~(ROUND_UP - 1);
    size_t      total_size = round_up_size + EXTRA_SPACE + EXTRA_SPACE;
    struct before_after *pre = (struct before_after *)address - 1;
    struct before_after *post = (struct before_after *)((char *)address + round_up_size);

#ifndef PERF
    /* Check that pre and post areas are ok. */
    if (t_bytes != pre->length || t_bytes != post->length)
    {
        fprintf(stderr, "%s%s:%s:%u Length of %p being freed (%u) != allocated (pre/post %u/%u)\n",
                FEBEMESSAGE, __FILE__, __func__, __LINE__,
                address, t_bytes, pre->length, post->length);
        abort();
    }
    static const char *round_up_string = "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD";
    void       *end = (char *)address + t_bytes;

    if (memcmp(end, round_up_string, round_up_size - t_bytes) != 0)
    {
        fprintf(stderr, "%s%s:%s:%u round-up area after %p of length %u does not match 0x44 (0x%16.16llx)\n",
                FEBEMESSAGE, __FILE__, __func__, __LINE__,
                address, t_bytes, *(unsigned long long *)end);
        abort();
    }

    if (pre->used_or_free != 1 || post->used_or_free != 1)
    {
        fprintf(stderr, "%s%s:%s:%u double free of address %p\n",
                FEBEMESSAGE, __FILE__, __func__, __LINE__, address);
        abort();
    }
    if (pre->pre1 != 0xdddddddd || pre->length != t_bytes || pre->pre2 != 0xdddddddd)
    {
        fprintf(stderr, "%s%s:%s:%u pre-area %p of length %u does not match (%x&%x!=dddddddd)||(%u != %u)\n",
                FEBEMESSAGE, __FILE__, __func__, __LINE__,
                address, t_bytes, pre->pre1, pre->pre2, pre->length, t_bytes);
        abort();
    }
    if (post->pre1 != 0xdcdcdcdc || post->length != t_bytes || post->pre2 != 0xdcdcdcdc)
    {
        fprintf(stderr, "%s%s:%s:%u post-area %p of length %u does not match (%x&%x!=dcdcdcdc)||(%u != %u)\n",
                FEBEMESSAGE, __FILE__, __func__, __LINE__,
                address, t_bytes, post->pre1, post->pre2, post->length, t_bytes);
        abort();
    }
    if ((UINT32)post->next != 0xdcdcdcdc)
    {
        fprintf(stderr, "%s%s:%s:%u post-area %p of length %u does not have next (%p!=dcdcdcdc)\n",
                FEBEMESSAGE, __FILE__, __func__, __LINE__,
                address, t_bytes, post->next);
        abort();
    }
#ifndef REALLY_DELAY_MEMORY_FREES
    if (memcmp(pre->str, post->str, sizeof(pre->str)) != 0)
    {
        fprintf(stderr, "%s%s:%s:%u Filename of %p being freed (%u) do not match (pre/post %.11s/%.11s)\n",
                FEBEMESSAGE, __FILE__, __func__, __LINE__,
                address, t_bytes, pre->str, post->str);
        abort();
    }
    if (pre->line_number != post->line_number)
    {
        fprintf(stderr, "%s%s:%s:%u line_number of %p being freed (%u) do not match (pre/post %u/%u)\n",
                FEBEMESSAGE, __FILE__, __func__, __LINE__,
                address, t_bytes, pre->line_number, post->line_number);
        abort();
    }
    /* Put in freed file name and line number into post. */
    const char *last_slash = strrchr(file, '/');

    last_slash = (last_slash == NULL) ? file : last_slash + 1;
    strncpy(post->str, last_slash, sizeof(post->str));
    post->line_number = line;
#endif  /* REALLY_DELAY_MEMORY_FREES */

#ifdef MEMSET_PATTERN_FREE
    /* Set freed memory pattern. */
    memset(address, 0xFE, round_up_size);
#endif  /* MEMSET_PATTERN_FREE */
#endif  /* PERF */

    pre->used_or_free = 0;
    post->used_or_free = 0;

    last_plus_one = (void *)(post + 1);

    /*
     * The secondary origin (when valid) contains a pointer to a pre which
     * has been recently referenced. If the to-be-freed segment lies beyond
     * this point the secondary origin will be used as the starting point for
     * this free. This technique serves to minimize search time.
     */

    if ((char *)pre <= (char *)fmm->fmm_sorg || fmm->fmm_sorg == NULL)
    {
        /* Use start of memory free chain. */
        current_pre = (struct before_after *)&fmm->fmm_first;   /* NOTE: Oh my goodness. */
        next_pre = (struct before_after *)fmm->fmm_first.thd;
        next_address = (next_pre == NULL) ? NULL : (void *)(next_pre + 1);
        current_length = 0;             /* Current_segment->len is always zero. */
        current_lba_plus1 = NULL;       /* Should never use this. */
    }
    else
    {
        /* Use secondary origin. */
        current_pre = (struct before_after *)fmm->fmm_sorg;
        if (fmm->fmm_sorg == &fmm->fmm_first)
        {
            next_pre = (struct before_after *)fmm->fmm_first.thd;
            next_address = (next_pre == NULL) ? NULL : (void *)(next_pre + 1);
            current_length = 0;
            current_lba_plus1 = NULL;   /* Better not use this. */
        }
        else
        {
            next_address = current_pre->next;
            next_pre = (struct before_after *)next_address - 1;
            current_length = current_pre->length;
            current_lba_plus1 = (char *)current_pre + current_length + EXTRA_SPACE + EXTRA_SPACE;
        }
    }

/*
    The free memory chain is constructed of segments which are linked together
    in ascending sequence by starting address. This block of code locates the
    pair of segments which bracket the segment of memory being released.

    If p and c represent the pair of adjacent segments (also referred to as
    previous and current respectively) and r represents the segment being
    released, the following four scenarios are possible:

       lo              hi
      addr            addr
      ----            ----
      pppp    rrrr    cccc      Type 1 - Creation and insertion of new segment r
      pppprrrr        cccc      Type 2 - Upward expansion of p segment
      pppp        rrrrcccc      Type 3 - Downward expansion of c segment
          pppprrrrcccc          Type 4 - Concatenation of p, c and r segments
 */

    while (1)
    {
        /* Copy current pre, length, and lba+1 to previous. */
        previous_pre = current_pre;
        previous_length = current_length;
        previous_lba_plus1 = current_lba_plus1;

        /* If next segment is end of chain, stop. */
        current_pre = next_pre;
        if (next_address == NULL)
        {
            current_length = 0; /* ??? */
            break;
        }

        /* Get current segment length and next segment address. */
        next_address = current_pre->next;
        next_pre = next_address == NULL ? NULL : (struct before_after *)next_address - 1;
        current_length = current_pre->length;
        current_lba_plus1 = (char *)current_pre + current_length + EXTRA_SPACE + EXTRA_SPACE;

        if ((char *)current_pre >= (char *)pre)
        {
#ifndef PERF
            if ((char *)pre == (char *)current_pre)
            {
                fprintf(stderr, "%sMemory freed twice k_mrel(%p %u %p)\n",
                        FEBEMESSAGE, address, t_bytes, fmm);
                abort();
            }
#endif  /* PERF */
            break;
        }
        /* Continue until we get to a memory address above where we are. */
    }

    /* previous_pre is before where we put this address. */

    struct before_after *fix_post;

    /* Check if this address is contiguous with previous (Type 2 or Type 4). */
    if ((char *)pre == previous_lba_plus1)
    {
        /* Check if also contiguous with current (Type 4). */
        if (last_plus_one == current_pre)
        {
            /* Concatenate prev segment, freed segment, and next segment (Type 4) */
            previous_length += current_length + total_size + EXTRA_SPACE + EXTRA_SPACE;
#ifdef MEMSET_PATTERN_FREE
            /* Set our pre and post before it to freed memory pattern. */
            memset((void *)(pre - 1), 0xFE, EXTRA_SPACE + EXTRA_SPACE);
            /* Set our post, and following pre to freed memory pattern. */
            memset((void *)post, 0xFE, EXTRA_SPACE + EXTRA_SPACE);
#endif  /* MEMSET_PATTERN_FREE */
            /* Adjust current memory by released -- two headers freed. */
            fms->fms_Available_memory += total_size;
            /* Fix pre and post lengths. */
            previous_pre->length = previous_length;
            fix_post = (struct before_after *)current_lba_plus1 - 1;
            fix_post->length = previous_length;
            previous_pre->next = next_address;
        }
        else
        {
            /* Tack on to previous segment (Type 2). */
            previous_length += total_size;
#ifdef MEMSET_PATTERN_FREE
            /* Set previous post and our pre header to freed memory pattern. */
            memset((void *)(pre - 1), 0xFE, EXTRA_SPACE + EXTRA_SPACE);
#endif  /* MEMSET_PATTERN_FREE */
            /* Adjust current memory by released -- two headers freed. */
            fms->fms_Available_memory += total_size;
            /* Set our post header correctly. */
            post->length = previous_length;
            previous_pre->length = previous_length;
        }
    }
    else
    {
        /* Release not contiguous with previous segment (Type 1 or Type 3). */

        if (last_plus_one != (char *)current_pre)       /* Not match next segment. */
        {
            /* Insert new entry in middle (Type 1). */
            /* Make this point to one after it. */
            pre->next = current_pre == NULL ? NULL : (void *)(current_pre + 1);
            pre->length = round_up_size;
            post->length = round_up_size;
            /* Adjust current memory by released -- no headers freed. */
            fms->fms_Available_memory += total_size;
        }
        else
        {
            /* Concatenate freed segment and current segment together (Type 3) */
            current_length += total_size;
            pre->next = next_address;
#ifdef MEMSET_PATTERN_FREE
            /* Set our post and next pre header to freed memory pattern. */
            memset((void *)post, 0xFE, EXTRA_SPACE + EXTRA_SPACE);
#endif  /* MEMSET_PATTERN_FREE */
            /* Adjust current memory by released -- one pre and post header freed. */
            fms->fms_Available_memory += total_size;
            pre->length = current_length;
            fix_post = (struct before_after *)current_lba_plus1 - 1;
            fix_post->length = current_length;
        }
        /* Make one before point to this one. */
        if (previous_pre == (struct before_after *)&fmm->fmm_first)
        {
            fmm->fmm_first.thd = (void *)pre;
        }
        else
        {
            previous_pre->next = address;
        }
    }

    /* Make sure maximum hasn't changed. */
    if (fms->fms_Maximum_available < fms->fms_Available_memory)
    {
        fms->fms_Maximum_available = fms->fms_Available_memory;
    }

    /* Set up secondary origin for next time. */
    fmm->fmm_sorg = (struct mc *)previous_pre;

#ifndef CCB_RUNTIME_CODE
    /* Process possible tasks waiting for memory condition. */
    if (fms->fms_Number_tasks_waiting != 0)
    {
        TaskReadyByState(fmm->fmm_waitstat);
        /* Clear wait count. */
        fms->fms_Number_tasks_waiting = 0;
    }
#endif  /* CCB_RUNTIME_CODE */
}                               /* End of k_mrel */


/* ------------------------------------------------------------------------ */
#ifndef CCB_RUNTIME_CODE

/**
******************************************************************************
**
**  @name       p_MallocC
**
**  @brief      To assign and clear private memory, wait until available.
**
**  @param      t_bytes - bytes, with upper bit set for permanent assignment.
**
**  @return     assigned address
**
******************************************************************************
**/

void       *p_MallocC(UINT32 t_bytes, const char *file, unsigned int line)
{
    void       *address;
#ifdef REALLY_DELAY_MEMORY_FREES
    init_tsc_check;
#endif  /* REALLY_DELAY_MEMORY_FREES */

#ifndef PERF
    if ((t_bytes & 0x7fffffff) == 0)
    {
        abort();
    }
#endif  /* PERF */

#ifdef CHECK_MEMORY_ALL
    check_private_memory;
#endif  /* CHECK_MEMORY_ALL */

    do
    {
        address = k_malloc(t_bytes, &P_ram, file, line);    /* FMM is P_ram. */
        if (address != NULL)                                /* Memory allocated, exit. */
        {
            memset(address, 0, t_bytes & 0x7fffffff);       /* Clear memory. */
            return (address);
        }

#ifdef REALLY_DELAY_MEMORY_FREES
        put_private_delayed_memory(t_bytes & 0x7fffffff);   // Note continue in macro.
#endif  /* REALLY_DELAY_MEMORY_FREES */

        if (Out_of_local_memory_print == 0)
        {
            fprintf(stderr, "%sOut of local memory allocating %s:%u -- going to shared memory\n",
                    FEBEMESSAGE, file, line);
            Out_of_local_memory_print++;
        }

        /* No private memory, try shared memory. */
        address = k_malloc(t_bytes, &K_ncdram, file, line);     /* FMM is K_ncdram. */
        if (address != NULL)                                /* Memory allocated, exit. */
        {
            memset(address, 0, t_bytes & 0x7fffffff);       /* Clear memory. */
            return (address);
        }

        P_ram.fmm_fms->fms_Number_tasks_waiting++;          /* Increment memory wait. */
        TaskSetState(K_xpcb, P_ram.fmm_waitstat);           /* Place this process in wait state. */
        release_deferred_memory();                          /* If any deferred memory to free, do it. */
        TaskSwitch();                                       /* Relinquish running to others, and then
                                                             * continue if/when memory available. */
    } while (1);
}                               /* End of p_MallocC */


/**
******************************************************************************
**
**  @name       p_MallocW
**
**  @brief      To assign private memory, wait until available.
**
**  @param      t_bytes - bytes, with upper bit set for permanent assignment.
**
**  @return     assigned address
**
******************************************************************************
**/

void       *p_MallocW(UINT32 t_bytes, const char *file, unsigned int line)
{
    void       *address;

#ifndef PERF
    if ((t_bytes & 0x7fffffff) == 0)
    {
        abort();
    }
#endif  /* PERF */

#ifdef CHECK_MEMORY_ALL
    check_private_memory;
#endif  /* CHECK_MEMORY_ALL */

    do
    {
        address = k_malloc(t_bytes, &P_ram, file, line);    /* FMM is P_ram. */
        if (address != NULL)                                /* Memory allocated, exit. */
        {
            return (address);
        }

#ifdef REALLY_DELAY_MEMORY_FREES
        if (private_delayed_ptr_head != 0)
        {
            char       *release_addr = (char *)(private_delayed_ptr_head + 1);
            UINT32      delayed_lth = private_delayed_ptr_head->length;

            if (private_delayed_ptr_head->used_or_free != 2)
            {
                abort();
            }
            private_delayed_ptr_head->used_or_free = 1; // Mark used before release
            private_delayed_ptr_head = private_delayed_ptr_head->next;
            if ((char *)release_addr >= &local_memory_start &&
                (char *)release_addr < ((char *)&local_memory_start + PRIVATE_SIZE))
            {
                k_mrel(release_addr, delayed_lth, &P_ram, file, line);
            }
            else
            {
#if !defined(PERF) && !defined(CCB_RUNTIME_CODE)
                if ((char *)release_addr < NcdrAddr ||
                    ((char *)release_addr + delayed_lth) >= (NcdrAddr + NcdrSize))
                {
                    fprintf(stderr, "%sERROR %s -- release_addr (%p + %u) < %p  or >= %p\n",
                            FEBEMESSAGE, __func__,
                            release_addr, delayed_lth, NcdrAddr, NcdrAddr + NcdrSize);
                    abort();
                }
#endif  /* !PERF && !CCB_RUNTIME_CODE */
                k_mrel(release_addr, delayed_lth, &K_ncdram, file, line);
            }
            continue;
        }
#endif  /* REALLY_DELAY_MEMORY_FREES */

        if (Out_of_local_memory_print == 0)
        {
            fprintf(stderr, "%sOut of local memory allocating %s:%u -- going to shared memory\n",
                    FEBEMESSAGE, file, line);
            Out_of_local_memory_print++;
        }

        /* No private memory, try shared memory. */
        address = k_malloc(t_bytes, &K_ncdram, file, line);     /* FMM is K_ncdram. */
        if (address != NULL)                                /* Memory allocated, exit. */
        {
            memset(address, 0, t_bytes & 0x7fffffff);       /* Clear memory. */
            return (address);
        }

        P_ram.fmm_fms->fms_Number_tasks_waiting++;  /* Increment memory wait. */
        TaskSetState(K_xpcb, P_ram.fmm_waitstat);   /* Place this process in wait state. */
        release_deferred_memory();                  /* If any deferred memory to free, do it. */
        TaskSwitch();                               /* Relinquish running to others, and then
                                                     * continue if/when memory available. */
    } while (1);
}                               /* End of p_MallocW */
#endif  /* CCB_RUNTIME_CODE */


/**
******************************************************************************
**
**  @name       p_Malloc
**
**  @brief      To assign private memory, if available -- else NULL.
**
**  @param      t_bytes - bytes, with upper bit set for permanent assignment.
**
**  @return     assigned address, or NULL (0) if none available.
**
******************************************************************************
**/

void       *p_Malloc(UINT32 t_bytes, const char *file, unsigned int line)
{
    void       *address;
#ifdef REALLY_DELAY_MEMORY_FREES
    init_tsc_check;
#endif  /* REALLY_DELAY_MEMORY_FREES */

#ifndef PERF
    if ((t_bytes & 0x7fffffff) == 0)
    {
        abort();
    }
#endif  /* PERF */

#ifdef CHECK_MEMORY_ALL
    check_private_memory;
#endif  /* CHECK_MEMORY_ALL */


#ifdef REALLY_DELAY_MEMORY_FREES
    do
    {
#endif  /* REALLY_DELAY_MEMORY_FREES */
        address = k_malloc(t_bytes, &P_ram, file, line);        /* FMM is P_ram. */
#ifdef REALLY_DELAY_MEMORY_FREES
        if (address != NULL)
        {
            return (address);
        }
#ifdef REALLY_DELAY_MEMORY_FREES
        put_private_delayed_memory(t_bytes & 0x7fffffff);   // Note continue in macro.
#endif  /* REALLY_DELAY_MEMORY_FREES */
        break;                                              /* There is no memory. */
    } while (1);
#endif  /* REALLY_DELAY_MEMORY_FREES */
    return (address);
}                               /* End of p_Malloc */


#ifndef CCB_RUNTIME_CODE
/**
******************************************************************************
**
**  @name       p_Free_and_zero
**
**  @brief      To free private memory and zero on free.
**
**  @param      address - Start of memory segment to free.
**  @param      t_bytes - Number of bytes.
**
**  @return     none
**
******************************************************************************
**/

void p_Free_and_zero(void *address, UINT32 t_bytes, const char *file, unsigned int line)
{
    memset(address, 0, t_bytes);
    p_Free(address, t_bytes, file, line);
}                               /* End of p_Free_and_zero */
#endif  /* CCB_RUNTIME_CODE */


/**
******************************************************************************
**
**  @name       p_Free
**
**  @brief      To free private memory.
**
**  @param      address - Start of memory segment to free.
**  @param      t_bytes - Number of bytes.
**
**  @return     none
**
******************************************************************************
**/

void p_Free(void *address, UINT32 t_bytes, const char *file, unsigned int line)
{
#if defined(HISTORY_KEEP) && defined(REALLY_DELAY_MEMORY_FREES)
    CT_history1("entering p_Free with memory", (UINT32)address);
#endif  /* HISTORY_KEEP && REALLY_DELAY_MEMORY_FREES */
#ifndef PERF
    if ((t_bytes & 0x7fffffff) == 0)
    {
        abort();
    }

    if (Out_of_local_memory_print == 0)
    {
        if ((char *)address < &local_memory_start ||
            ((char *)address + t_bytes) >= ((char *)&local_memory_start + PRIVATE_SIZE))
        {
            fprintf(stderr, "%sERROR %s -- address (%p + %u) < %p  or >= %p\n",
                    FEBEMESSAGE, __func__,
                    address, t_bytes, &local_memory_start, (char *)&local_memory_start + PRIVATE_SIZE);
            abort();
        }
    }
#endif  /* PERF */

#ifdef REALLY_DELAY_MEMORY_FREES
    save_delayed_memory_free(private_delayed_ptr, address, t_bytes, file, line);
#else   /* REALLY_DELAY_MEMORY_FREES */
    if ((char *)address >= &local_memory_start &&
        (char *)address < ((char *)&local_memory_start + PRIVATE_SIZE))
    {
        k_mrel(address, t_bytes, &P_ram, file, line);
    }
    else
    {
#if !defined(PERF) && !defined(CCB_RUNTIME_CODE)
        if (Out_of_local_memory_print == 0 &&
            ((char *)address < NcdrAddr ||
             ((char *)address + t_bytes) >= (NcdrAddr + NcdrSize)))
        {
            fprintf(stderr, "%sERROR %s -- address (%p + %u) < %p  or >= %p\n",
                    FEBEMESSAGE, __func__,
                    address, t_bytes, NcdrAddr, NcdrAddr + NcdrSize);
            abort();
        }
#endif  /* !PERF && !CCB_RUNTIME_CODE */
        k_mrel(address, t_bytes, &K_ncdram, file, line);
    }
#endif  /* REALLY_DELAY_MEMORY_FREES */

#ifdef CHECK_MEMORY_ALL
    check_private_memory;
#endif  /* CHECK_MEMORY_ALL */
}                               /* End of p_Free */


/**
******************************************************************************
**
**  @name       k_init_mem
**
**  @brief      To initialize either shared or private memory.
**
**  @param      fmm     - Free Memory Management structure to initialize.
**  @param      address - Start of memory.
**  @param      t_bytes - Number of bytes of memory.
**
**  @return     none
**
******************************************************************************
**/

void k_init_mem(struct fmm *fmm, void *address, UINT32 t_bytes)
{
    /* Make align on 32 byte (but not 64 byte) boundary. */
    char       *new_address = (char *)(((UINT32)address + 0x3f) & ~0x3f) + EXTRA_SPACE;

    t_bytes = (t_bytes - (new_address - (char *)address)) & ~(ROUND_UP - 1);

    struct before_after *pre = (struct before_after *)new_address;
    struct before_after *post = (struct before_after *)(new_address + t_bytes) - 1;

    new_address = (void *)(pre + 1);

    /* Fill in the pre and post allocation patterns. */
    pre->pre1 = pre->pre2 = 0xdddddddd;
    memset(&pre->str, 0, sizeof(pre->str));
    strncpy(pre->str, __func__, sizeof(pre->str));
    pre->line_number = __LINE__;
    pre->used_or_free = 0;                          // flag that this is not used (i.e. free)
    pre->next = (void *)0;                          // We are at the end
    pre->length = t_bytes - EXTRA_SPACE - EXTRA_SPACE;

    post->pre1 = post->pre2 = 0xdcdcdcdc;
    memset(&post->str, 0, sizeof(post->str));
    strncpy(post->str, __func__, sizeof(post->str));
    post->line_number = __LINE__;
    post->used_or_free = 0;                         // flag that this is not used (i.e. free)
    post->next = (void *)0;                         // We are at the end
    post->length = pre->length;

    fmm->fmm_first.thd = (struct mc *)pre;          /* Point to first memory address in link. */
    fmm->fmm_first.len = 0;                         /* Nothing for this size. */
    /* fmm_fms set before this routine is called. */
    fmm->fmm_sorg = NULL;
#ifndef CCB_RUNTIME_CODE
    /* fmm_waitstat set before this routine is called. */
    fmm->fmm_options = 0;                           /* No memory options. */
#endif  /* CCB_RUNTIME_CODE */

    fmm->fmm_fms->fms_Available_memory = t_bytes;   /* Currently available. */
    fmm->fmm_fms->fms_Maximum_available = t_bytes;  /* Max available, ever. */
    fmm->fmm_fms->fms_Minimum_available = t_bytes;  /* Min available, ever. */
#ifndef CCB_RUNTIME_CODE
    fmm->fmm_fms->fms_Number_tasks_waiting = 0;     /* Number tasks waiting for memory. */
#endif  /* CCB_RUNTIME_CODE */
#ifdef MEMSET_PATTERN_FREE
    /* Set freed memory pattern. */
    memset(new_address, 0xFE, pre->length);
#else   /* MEMSET_PATTERN_FREE */
    memset(new_address, 0, pre->length);            // IT WILL BE ZERO UPON START.
#endif  /* MEMSET_PATTERN_FREE */

#ifdef CHECK_MEMORY_ALL
    check_private_memory;
    check_shared_memory;
#endif  /* CHECK_MEMORY_ALL */
}                               /* End of k_init_mem */

/**
******************************************************************************
**
**  @name       k_make_pre_post_headers
**
**  @brief      To create pre and post headers for write cache memory free/keep.
**
**  @param      address - Start of memory.
**  @param      t_bytes - Number of bytes of memory.
**
**  @return     none
**
******************************************************************************
**/

void k_make_pre_post_headers(void *address, UINT32 t_bytes)
{
    struct before_after *pre = (struct before_after *)address - 1;
    struct before_after *post = (struct before_after *)((char *)address + t_bytes) + 1;

    /* Fill in the pre and post allocation patterns. */
    pre->pre1 = pre->pre2 = 0xdddddddd;
    memset(&pre->str, 0, sizeof(pre->str));
    strncpy(pre->str, __func__, sizeof(pre->str));
    pre->line_number = __LINE__;
    pre->used_or_free = 1;                          // flag that this is used
    pre->next = (void *)0;                          // We are at the end
    pre->length = t_bytes;

    post->pre1 = post->pre2 = 0xdcdcdcdc;
    memset(&post->str, 0, sizeof(post->str));
    strncpy(post->str, __func__, sizeof(post->str));
    post->line_number = __LINE__;
    post->used_or_free = 1;                         // flag that this is used
    post->next = (void *)0;                         // We are at the end
    post->length = t_bytes;
}                               /* End of k_make_pre_post_headers */

/* ------------------------------------------------------------------------ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
