/* $Id: mem_pool.c 157461 2011-08-03 15:21:36Z m4 $ */
/**
******************************************************************************
**
**  @file       mem_pool.c
**
**  @brief      Memory initialization, allocation and freeing of "pools".
**
** We know that shared memory is allocated in 64 byte chunks (with debug
** electric fence after that).
**
** We want this memory pool to work as follows:
** a) Minimum length of 64 bytes. They are allocated on 64 byte alignment.
**    They have 64 bytes between allocations. (32 before, 32 after.)
** b) The linked list is kept outside the allocated memory, so that typical
**    memory structure will have garbage in the first two words (if PATTERNS
**    are turned on - runs slower). Typically the forward and backwards
**    pointers for allocated lists are in the first two words of structure.
** c) Common data structure for each pool (struct memory_pool).
** d) ROUND_UP to 64 byte boundaries in PERF and DEBUG mode.
** e) There is a pre-pattern and post-pattern around allocation, link is in pre.
** f) Routine check_local_memory_before_after() verifies all pre/post are ok.
** g) In Debug mode, there should always be one element free (if any are
**    ever "init_" or "put_" before the first "get_"). This insures that
**    memory pattern fills the last item free, in case it is ever re-used.
** h) In PATTERN filling, fill upon "put_", and "get_" checks for pattern.
** i) The PRIVATE_MEMORY_ALLOCATION is in memory that does not overlap with
**    any others -- thus if they end up in a different process (heaven forbid),
**    then it will immediately crash with a segment fault.
** j) For now, PRIVATE_MEMORY_ALLOCATION is merely allocated -- never freed.
**    Thus it works for memory pools - because they currently never clean up.
**
**  Copyright (c) 2008 Xiotech Corporation. All rights reserved.
**
** if defined(FRONTEND)
**  get_tmt(), put_tmt(), init_tmt() defined in this file.
**  get_tlmt(), put_tlmt(), init_tlmt() defined in this file.
**  get_ismt(), put_ismt(), init_ismt() defined in this file.
**  get_xli(), put_xli(), init_xli() defined in this file.
**  get_ltmt(), put_ltmt(), init_ltmt() defined in this file.
**  get_irp(), put_irp(), init_irp() defined in this file.
**  get_imt(), put_imt(), init_imt() defined in this file.
**  get_lsmt(), put_lsmt(), init_lsmt() defined in this file.
**  get_imt(), put_imt(), init_imt() defined in this file.
**  get_vdmt(), put_vdmt(), init_vdmt() defined in this file.
**  get_ilmt(), put_ilmt(), init_ilmt() defined in this file.
**  get_wc_plholder(), put_wc_plholder(), init_wc_plholder() defined in this file,
**  get_wc_rbinode(), put_wc_rbinode(), init_wc_rbinode() defined in this file,
**  get_wc_rbnode(), put_wc_rbnode(), init_wc_rbnode() defined in this file,
** endif FRONTEND
** if defined(BACKEND)
**  get_scmte(), put_scmte(), init_scmte() defined in this file.
**  get_prp(), put_prp(), init_prp() defined in this file.
**  get_rrp(), put_rrp(), init_rrp() defined in this file.
**  get_rpn(), put_rpn(), init_rpn() defined in this file.
**  get_rrb(), put_rrb(), init_rrb() defined in this file.
**  get_vlar(), put_vlar(), init_vlar() defined in this file.
**  get_rm(), put_rm(), init_rm() defined in this file.
**  get_sm(), put_sm(), init_sm() defined in this file.
**  get_cm(), put_cm(), init_cm() defined in this file.
**  get_cor(), put_cor(), init_cor() defined in this file.
**  get_scd(), put_scd(), init_scd() defined in this file.
**  get_dcd(), put_dcd(), init_dcd() defined in this file.
**  get_tpmt(), put_tpmt(), init_tpmt() defined in this file.
**  get_scmt(), put_scmt(), init_scmt() defined in this file.
** endif BACKEND
** Both FRONTEND and BACKEND below
**  get_qrp(), put_qrp(), init_qrp() defined in this file.
**  get_dtmt(), put_dtmt(), init_dtmt() defined in this file.
**  get_mlmt(), put_mlmt(), init_mlmt() defined in this file.
**  get_pcb(), put_pcb(), init_pcb() defined in this file.
** FRONTEND, BACKEND, and CCB_RUNTIME_CODE
**  get_ilt(), put_ilt(), init_ilt() defined in this file.
**  get_vrp(), put_vrp(), init_vrp() defined in this file.
******************************************************************************
**/

#include "XIO_Const.h"
#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "mem_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(FRONTEND) || defined(BACKEND)
#include "CT_defines.h"
#else   /* !defined FRONTEND || BACKEND */
#include "CT_history.h"
#endif  /* FRONTEND || BACKEND */

#undef strncpy                  // CT_defines.h has own routine, we need the "const" on it.
#undef memcmp                   // We don't want any potential surprises.
#undef memset                   // We don't want any potential surprises.

#if defined(FRONTEND) || defined(BACKEND)
extern UINT32 dtmt_banner;
#endif  /* FRONTEND || BACKEND */
#if defined(BACKEND)
extern UINT32 tpmt_banner;
#endif  /* BACKEND */

#ifdef CCB_RUNTIME_CODE
#define FEBEMESSAGE "CCB "
#endif  /* CCB_RUNTIME_CODE */

#ifdef CHECK_MEMORY_ALL
extern void check_memory_all(void);
#else   /* CHECK_MEMORY_ALL */
#define check_memory_all()
#endif

/*
******************************************************************************
** Private defines
******************************************************************************
*/
#ifdef FRONTEND
#define PRIVATE_SIZE    MAKE_DEFS_FE_ONLY_SIZE
#endif  /* FRONTEND */
#ifdef BACKEND
#define PRIVATE_SIZE    MAKE_DEFS_BE_ONLY_SIZE
#endif  /* BACKEND */
#ifdef CCB_RUNTIME_CODE
#define PRIVATE_SIZE    MAKE_DEFS_CCB_ONLY_SIZE
#endif  /* CCB_RUNTIME_CODE */

/*
 * These two macros allow converting a macro argument into a printable string.
 */
#define realstringify(abc)  #abc
#define stringify(abc)  realstringify(abc)

/*
 * There are two types of memory pools -- those in shared memory, and those
 * that are in private (local only) memory.
 */
/* Argument local_mem of allocate_pool() macro. */
#define PRIVATE_MEMORY_ALLOCATION   0   /* Use private memory. */
#define SHARED_MEMORY_ALLOCATION    1   /* Use old shared memory stuff. */

/*
 * There are many special features required of the various memory pools.
 * These options allow for those special features to be selected.
 */
/* Argument memset_zero of allocate_pool macro. */
#define ZEROED                      0   /* Zero memory with get, possible PATTERN on put. */
#define ANYTHING                    1   /* Any value ok, possible PATTERN on put. --ALWAYS ZERO! */
#define DTMT_SET                    2   /* Special stuff for DTMT, as below: */
    /* put_special_values - memset 0. */
    /* get_name_pattern_still_set - check 0. */
    /* get_##name - Set dtmt_bnr. */
#define PRP_SET                     3   /* Special stuff for PRP, as below: */
    /* put_increment_statistics - K_ii.prpCur++ */
    /* init_initialize_statistics - K_ii.prpMax++ */
    /* get_special_set_stats - reqSenseBytes = SENSE_SIZE, K_ii.prpCur-- */
#define RPN_SET                     4   /* Special stuff for RPN, as below: */
    /* put_increment_statistics - K_ii.rpnCur++ */
    /* init_initialize_statistics - K_ii.rpnMax++ */
    /* get_special_set_stats - K_ii.prpCur-- */
#define RRB_SET                     5   /* Special stuff for RRB, as below: */
    /* put_increment_statistics - K_ii.rrbCur++ */
    /* init_initialize_statistics - K_ii.rrbMax++ */
    /* get_special_set_stats - K_ii.rrbCur-- */
#define RRP_SET                     6   /* Special stuff for RRP, as below: */
    /* put_increment_statistics - K_ii.rrpCur++ */
    /* init_initialize_statistics - K_ii.rrpMax++ */
    /* get_special_set_stats - K_ii.rrpCur-- */
#define MINMAXTBL_SET               7   /* Set i_mintbl/i_maxtbl on get, as below: */
    /* ISMT, XLI */
    /* i_minmaxtbl_set - i_mintbl and i_maxtbl setting. */
    /* get_special_set_stats - uses i_minmaxtbl_set. */
#define Z_MINMAXTBL_SET             8   /* Zero memory and set i_mintbl/i_maxtbl, below: */
    /* TMT, TLMT */
    /* i_minmaxtbl_set - i_mintbl and i_maxtbl setting. */
    /* get_special_set_stats - uses i_minmaxtbl_set. */
    /* get_##name - memset 0. */
// #define UNUSED_CURRENTLY            9    /* NOTE: unused currently.  2008-12-23 */
#define TPMT_SET                   10   /* Special processing for TPMT's as below: */
    /* set_memory_pattern - DO NOT SET PATTERN. */
    /* put_special_values - banner = 0, state = TPM_STATE_DEALLOC. */
    /* get_name_pattern_still_set - check banner == 0, state == TPM_STATE_DEALLOC. */
    /* get_set_values - memset 0, banner = tpmt_banner. */
#define SCMT_SET                   11   /* Special processing for SCMT's, as below: */
    /* put_special_values - must have 2 or more free, then start cm_exec_qu_norm.pcb. */
    /* if_get_name_free_pool_check - do not get more items into the memory pool. */
//     /* get_set_values - memset 0, link in SCMT's. QUITE STRANGE. */
    /* get_name_another_init - do not get more items into the memory pool. */
#define ILT_SET                    12   /* Special processing for ILT's, as below: */
    /* put_increment_statistics - K_ii.iltCur++ */
    /* init_initialize_statistics - K_ii.iltMax++ */
    /* get_set_values - memset 0 the first ilt level only (caution!). */
    /* get_special_set_stats - K_ii.iltCur-- */
    /* An ILT allocation is really ILT_ALL_LEVELS big, 7 or 11 of sizeof(ILT). */
    /* An ILT must be allocated in shared memory. */
#define VRP_SET                    13   /* Special processing for VRP's. */
    /* get_set_values - options = 0, gen2 = 0. */
    /* A VRP allocation is really VRPALLOC big, having extra space for link layer, etc. */
    /* A VRP must be allocated in shared memory. */
#define FREEZERO                   14   /* Must zero upon free (normal - PATTERNS!): */
    /* LTMT, LSMT */
    /* put_special_values - memset 0. */
    /* get_name_pattern_still_set - check still zero. */
    /* Please NOTE that DTMT_SET and TPMT_SET must be cleared on free. */

    /* Lots of places check that data loaded from LTMT is zeroed, then ignore it. */

/* NOT FIXED or WORKAROUND YET, BELOW. */
    /* apldrv.as has places where ISMT if zeroed, then assume closed (bad). */
    /* TLMT, TMT loaded in apldrv.as -- complicated. */
    /* TLMT, apldrv.as, checks for tlm_ahead zero. */
    /* TMT, idriver.as,
       i$fabric_DiscoverTMT was returned from, and g5 was ok before the call to this (load happened).
       Hmmm, i$fabric_continue called with bad TMT? Need more info. */
    /* IMT -- lld.as -- maybe need null checks? */


/*
******************************************************************************
** The following is a set of #define's that create the 3 routines get_, put_,
** init_, and all special handling cases, Perf verses Debug, PATTERNS, etc.
** It also defines the structure for the pool.
**
** This allows one line per memory pool to be done (although in 3 files).
******************************************************************************
*/

/*
 * put_name_history_keep - will add the "free" to HISTORY_KEEP buffer.
 */
#ifdef HISTORY_KEEP
  #define put_name_history_keep(name, address)                              \
    CT_history1("entering put_"#name" with memory", (UINT32)address);
#else   /* HISTORY_KEEP */
  #define put_name_history_keep(name, address)
#endif  /* HISTORY_KEEP */

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
/*
 * check_name_null - does a check for free of null pointer in Debug mode.
 */
#ifndef PERF
  #define check_name_null(STRUCTURE)                                        \
    if (working == NULL)                                                    \
    {                                                                       \
        fprintf(stderr, "%s%s:%d "#STRUCTURE" to be freed is NULL\n", FEBEMESSAGE, __func__, __LINE__); \
        abort();                                                            \
    }
#else   /* PERF */
  #define check_name_null(STRUCTURE)
#endif  /* PERF */

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
/*
 * set_memory_pattern - will fill memory with a pattern -- normally does nothing.
 */
#ifdef M4_DEBUG_MEMORY_WITH_PATTERNS
  #define set_memory_pattern(PATTERN, memset_zero, LENGTH)                  \
    int end_at = ((LENGTH)/sizeof(struct memory_list *)) - 1;               \
    if (memset_zero != TPMT_SET)                                            \
    {                                                                       \
        /* Set memory pattern if in debug mode. */                          \
        int i;                                                              \
        for (i = 0; i <= end_at; i++)                                       \
        {                                                                   \
            w->memory_list_next[i] = (struct memory_list *)0x##PATTERN##PATTERN;\
        }                                                                   \
    }
#else   /* M4_DEBUG_MEMORY_WITH_PATTERNS */
  #define set_memory_pattern(PATTERN, memset_zero, LENGTH)
#endif  /* M4_DEBUG_MEMORY_WITH_PATTERNS */

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
/*
 * check_pool_structure - Does a check to make sure that head/tail/count ok.
 */
#ifndef PERF
  #define check_pool_structure(name)                                        \
        if (pool_##name.num_free != 0 || pool_##name.first != NULL)         \
        {                                                                   \
            fprintf(stderr, "%s%s:%d pool_"#name".first(%p) and "#name"_pool.num_free(%d) should both be zero\n", \
                    FEBEMESSAGE, __func__, __LINE__, pool_##name.first, pool_##name.num_free); \
            abort();                                                        \
        }                                                                   \
        if (pool_##name.tail != NULL)                                       \
        {                                                                   \
            fprintf(stderr, "%s%s:%d pool_"#name".first is NULL, but "#name"_pool.tail is (%p)\n", \
                    FEBEMESSAGE, __func__, __LINE__, pool_##name.tail);     \
            abort();                                                        \
        }
#else   /* PERF */
  #define check_pool_structure(name)
#endif  /* PERF */

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
/*
 * put_special_values - Some pools need a free to do special things.
 */
#ifdef BACKEND
// Need special set values section for put -- How many ways can you do a memory pool?
  #define put_special_values(memset_zero)                                   \
    if (memset_zero == FREEZERO || memset_zero == DTMT_SET)                 \
    {                                                                       \
        memset(working, 0, sizeof(*working));                               \
    }                                                                       \
    else if (memset_zero == TPMT_SET)                                       \
    {                                                                       \
        ((TPMT*)working)->banner = 0;                                       \
        ((TPMT*)working)->state = TPM_STATE_DEALLOC;                        \
    }                                                                       \
    else if (memset_zero == SCMT_SET)                                       \
    {                                                                       \
        if (pool_scmt.num_free >= 2 &&                                      \
            cm_exec_qu_norm.pcb != NULL &&                                  \
            TaskGetState(cm_exec_qu_norm.pcb) == PCB_IPC_WAIT)              \
        {                                                                   \
            TaskSetState(cm_exec_qu_norm.pcb, PCB_READY);                   \
        }                                                                   \
    }
#endif  /* BACKEND */
#ifdef FRONTEND
  #define put_special_values(memset_zero)                                   \
    if (memset_zero == FREEZERO || memset_zero == DTMT_SET)                 \
    {                                                                       \
        memset(working, 0, sizeof(*working));                               \
    }
#endif  /* FRONTEND */
#ifdef CCB_RUNTIME_CODE
  #define put_special_values(memset_zero)                                   \
    if (memset_zero == FREEZERO)                                            \
    {                                                                       \
        memset(working, 0, sizeof(*working));                               \
    }
#endif  /* CCB_RUNTIME_CODE */

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
/*
 * put_increment_statistics - Some pools have statistics that need to be kept.
 */
#ifdef BACKEND
  #define put_increment_statistics(memset_zero)                             \
    if (memset_zero == PRP_SET)                                             \
    {                                                                       \
        K_ii.prpCur++;                                                      \
    }                                                                       \
    else if (memset_zero == RPN_SET)                                        \
    {                                                                       \
        K_ii.rpnCur++;                                                      \
    }                                                                       \
    else if (memset_zero == RRB_SET)                                        \
    {                                                                       \
        K_ii.rrbCur++;                                                      \
    }                                                                       \
    else if (memset_zero == RRP_SET)                                        \
    {                                                                       \
        K_ii.rrpCur++;                                                      \
    }                                                                       \
    else if (memset_zero == ILT_SET)                                        \
    {                                                                       \
        K_ii.iltCur++;                                                      \
    }
#endif  /* BACKEND */
#ifdef FRONTEND
  #define put_increment_statistics(memset_zero)                             \
    if (memset_zero == ILT_SET)                                             \
    {                                                                       \
        K_ii.iltCur++;                                                      \
    }
#endif  /* FRONTEND */
#ifdef CCB_RUNTIME_CODE
  #define put_increment_statistics(memset_zero)
#endif  /* CCB_RUNTIME_CODE */

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
/*
 * init_initialize_statistics - Must initialize stats for some pools.
 */
#ifdef BACKEND
  #define init_initialize_statistics(memset_zero)                           \
    if (memset_zero == PRP_SET)                                             \
    {                                                                       \
        K_ii.prpMax++;                                                      \
    }                                                                       \
    else if (memset_zero == RPN_SET)                                        \
    {                                                                       \
        K_ii.rpnMax++;                                                      \
    }                                                                       \
    else if (memset_zero == RRB_SET)                                        \
    {                                                                       \
        K_ii.rrbMax++;                                                      \
    }                                                                       \
    else if (memset_zero == RRP_SET)                                        \
    {                                                                       \
        K_ii.rrpMax++;                                                      \
    }                                                                       \
    else if (memset_zero == ILT_SET)                                        \
    {                                                                       \
        K_ii.iltMax++;                                                      \
    }
#endif  /* BACKEND */
#ifdef FRONTEND
  #define init_initialize_statistics(memset_zero)                           \
    if (memset_zero == ILT_SET)                                             \
    {                                                                       \
        K_ii.iltMax++;                                                      \
    }
#endif  /* FRONTEND */
#ifdef CCB_RUNTIME_CODE
  #define init_initialize_statistics(memset_zero)
#endif  /* CCB_RUNTIME_CODE */

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
/*
 * Fill in the post filename:linenumber.
 */
#define fill_post_file_line                                                 \
  const char *last_slash = strrchr(filename, '/');                          \
  last_slash = (last_slash == NULL) ? filename : last_slash + 1;            \
  memset(&post->str, 0, sizeof(post->str));                                 \
  strncpy(post->str, last_slash, sizeof(post->str));                        \
  post->line_number = linenumber;

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
/*
 * put_check_free_is_ok - Check that structure being free is ok. (Debug only.)
 */
#ifndef PERF
  #define put_check_free_is_ok(STRUCTURE) \
    pre = ((struct before_after *)w) - 1;                                   \
    if (pre->used_or_free != 1) {                                           \
      fprintf(stderr, "%s%s double free of address %p\n", FEBEMESSAGE, #STRUCTURE, w); \
      abort();                                                              \
    }                                                                       \
    if (strncmp(#STRUCTURE, pre->str, sizeof(pre->str)) != 0) {             \
      fprintf(stderr, "%s%s free does not match structure type %s\n", FEBEMESSAGE, #STRUCTURE, pre->str); \
      abort();                                                              \
    }
#else   /* PERF */
  #define put_check_free_is_ok(STRUCTURE)                                   \
    pre = ((struct before_after *)w) - 1;
#endif  /* PERF */

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
/*
 * put_put_on_pool_list - Common pool to put freed memory on pool free list.
 */
#define put_put_on_pool_list(name, STRUCTURE, PATTERN, memset_zero, LENGTH) \
  set_memory_pattern(PATTERN, memset_zero, LENGTH);                         \
  /* Put #STRUCTURE onto free list. */                                      \
  struct before_after *pre;                                                 \
  if (pool_##name.first == NULL)                                            \
  {                                                                         \
      check_pool_structure(name);                                           \
      pool_##name.first = pool_##name.tail = w;                             \
  }                                                                         \
  else                                                                      \
  {                                                                         \
      pre = ((struct before_after *)pool_##name.tail) - 1;                  \
      pre->next = w;                                                        \
      pool_##name.tail = w;                                                 \
  }                                                                         \
  put_check_free_is_ok(STRUCTURE);                                          \
  pre->used_or_free = 0;                                                    \
  pre->next = NULL;                                                         \
  size_t asize = LENGTH < MIN_LTH ? MIN_LTH : LENGTH;                       \
  size_t around = (asize + ROUND_UP - 1) & ~(ROUND_UP - 1);                 \
  struct before_after *post = (struct before_after *)((char *)(pre + 1) + around); \
  post->used_or_free = 0;                                                   \
  fill_post_file_line;                                                      \
  pool_##name.num_free++;

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
/*
 * if_get_name_free_pool_check - does the check for if should allocate a new
 *                               pool entry -- if we run out.
 * NOTE: Code implies that number_free not needed in PERF mode.
 */
#ifndef PERF
  #define if_get_name_free_pool_check(name, memset_zero, number_free)       \
    if (working == NULL ||                                                  \
        ((memset_zero == SCMT_SET) ? (pool_##name.num_free == 0) :          \
         (pool_##name.num_free <= number_free)))
#else   /* PERF */
  #define if_get_name_free_pool_check(name, memset_zero, number_free)       \
    if (working == NULL || pool_##name.num_free == 0)
#endif  /* PERF */

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
/*
 * If running with TASK_STRESS_TEST defined, do a taskswitch().
 */
#ifdef TASK_STRESS_TEST
  #define get_name_taskswitch                                               \
        TaskSwitch();
#else   /* TASK_STRESS_TEST */
  #define get_name_taskswitch
#endif  /* TASK_STRESS_TEST */

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
/*
 * get_name_pattern_still_set - Checks that the memory to be allocated still
 *                              has the "free" pattern set.
 */
#ifdef M4_DEBUG_MEMORY_WITH_PATTERNS
  #ifdef BACKEND
    #define back_tpmt_check(memset_zero)                                    \
    else if (memset_zero == TPMT_SET)                                       \
    {                                                                       \
        if (((TPMT*)working)->banner != 0 ||                                \
            ((TPMT*)working)->state != TPM_STATE_DEALLOC)                   \
        {  /* SHOULD DO MORE, BUT WE DON'T SET A PATTERN ... TODO */        \
            fprintf(stderr, "%s TPMT address -- 0x%08x banner(0x%x)!=0  || state(0x%x)!=0x%x\n", FEBEMESSAGE, (UINT32)working, ((TPMT*)working)->banner, ((TPMT*)working)->state, TPM_STATE_DEALLOC); \
            abort();                                                        \
        }                                                                   \
    }
  #else     /* BACKEND -- FRONTEND || CCB_RUNTIME_CODE */
    #define back_tpmt_check(memset_zero)
  #endif    /* BACKEND */
  #define get_name_pattern_still_set(STRUCTURE, PATTERN, memset_zero, LENGTH) \
    size = (LENGTH) < MIN_LTH ? MIN_LTH : (LENGTH);                         \
    end_at = ((LENGTH)/sizeof(struct memory_list *)) - 1;                   \
    if (memset_zero != TPMT_SET && memset_zero != DTMT_SET)                 \
    {                                                                       \
        /* Check memory pattern still set to #PATTERN. */                   \
        int i;                                                              \
                                                                            \
        /* Do not check the memory_list next pointer. */                    \
        for (i = 0; i <= end_at; i++)                                       \
        {                                                                   \
            if ((UINT32)working->memory_list_next[i] != 0x##PATTERN##PATTERN)\
            {                                                               \
                fprintf(stderr, "%s%s:%d "#STRUCTURE" memory not 0x"#PATTERN#PATTERN", "#STRUCTURE"=%p, bad at 0x%x (%d)\n", \
                        FEBEMESSAGE, __func__, __LINE__,                    \
                        working, i*sizeof(struct memory_list *),            \
                        i*sizeof(struct memory_list *));                    \
                abort();                                                    \
            }                                                               \
        }                                                                   \
    }                                                                       \
    back_tpmt_check(memset_zero)                                            \
    else if (memset_zero == DTMT_SET)                                       \
    {                                                                       \
        int i;                                                              \
        for (i = 0; i <= end_at; i++)                                       \
        {                                                                   \
            if (i != 12 && (UINT32)working->memory_list_next[i] != 0x##PATTERN##PATTERN)\
            {                                                               \
                fprintf(stderr, "%s%s:%d "#STRUCTURE" memory not 0x"#PATTERN#PATTERN", "#STRUCTURE"=%p, bad at 0x%x (%d)\n", \
                        FEBEMESSAGE, __func__, __LINE__,                    \
                        working, i*sizeof(struct memory_list *),            \
                        i*sizeof(struct memory_list *));                    \
                abort();                                                    \
            }                                                               \
        }                                                                   \
    }
  #define get_var_define                                                    \
    size_t size;                                                            \
    int end_at;
#else   /* M4_DEBUG_MEMORY_WITH_PATTERNS */
  #define get_name_pattern_still_set(STRUCTURE, PATTERN, memset_zero, LENGTH)
  #define get_var_define
#endif  /* M4_DEBUG_MEMORY_WITH_PATTERNS */

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
/*
 * get_name_history_keep - will add the "get" to HISTORY_KEEP buffer.
 */
#ifdef HISTORY_KEEP
  #define get_name_history_keep(name, address)                              \
    CT_history1("exiting get_"#name" with memory", (UINT32)address);
#else   /* HISTORY_KEEP */
  #define get_name_history_keep(name, address)
#endif  /* HISTORY_KEEP */

/* ........................................................................ */
/*
 * Some front-end allocations have special start/end of memory pool checks.
 * This is probably a left-over kludge check from some bug that was happening.
 */
#ifdef FRONTEND
  extern UINT32 i_mintbl;
  extern UINT32 i_maxtbl;

  #define i_minmaxtbl_set(memset_zero)                                      \
        if (memset_zero == MINMAXTBL_SET ||                                 \
            memset_zero == Z_MINMAXTBL_SET)                                 \
        {                                                                   \
            if (i_mintbl > (UINT32)working)                                 \
            {                                                               \
                i_mintbl = (UINT32)working;                                 \
            }                                                               \
            if (i_maxtbl < (UINT32)working)                                 \
            {                                                               \
                i_maxtbl = (UINT32)working;                                 \
            }                                                               \
        }
#else   /* FRONTEND -- BACKEND || CCB_RUNTIME_CODE */
  #define i_minmaxtbl_set(memset_zero)
#endif  /* FRONTEND -- BACKEND || CCB_RUNTIME_CODE */

/* ........................................................................ */
/*
 * get_set_values - Some pools have special "get" clearing needs.
 * NOTE: If zeroed when freed, no need to do anything with the get.
 */
#ifdef BACKEND
  #define get_set_values(memset_zero)                                       \
    if (memset_zero == TPMT_SET)                                            \
    {                                                                       \
        ((TPMT*)working)->banner = tpmt_banner;                             \
    }                                                                       \
    else if (memset_zero == DTMT_SET)                                       \
    {                                                                       \
        ((DTMT *)(working))->banner = dtmt_banner;                          \
    }
#endif  /* BACKEND */
#ifdef FRONTEND
  #define get_set_values(memset_zero)                                       \
    if (memset_zero == DTMT_SET)                                            \
    {                                                                       \
        ((DTMT *)(working))->banner = dtmt_banner;                          \
    }
#endif  /* FRONTEND */
#ifdef CCB_RUNTIME_CODE
  #define get_set_values(memset_zero)
#endif  /* CCB_RUNTIME_CODE */

/* ........................................................................ */
/*
 * get_special_set_stats - This sets the legacy counters for special pools.
 */
#ifdef BACKEND
  #define get_special_set_stats(memset_zero)                                \
    if (memset_zero == PRP_SET)                                             \
    {                                                                       \
        ((PRP *)(working))->reqSenseBytes = SENSE_SIZE;                     \
        K_ii.prpCur--;                                                      \
    }                                                                       \
    else if (memset_zero == RPN_SET)                                        \
    {                                                                       \
        K_ii.rpnCur--;                                                      \
    }                                                                       \
    else if (memset_zero == RRB_SET)                                        \
    {                                                                       \
        K_ii.rrbCur--;                                                      \
    }                                                                       \
    else if (memset_zero == RRP_SET)                                        \
    {                                                                       \
        K_ii.rrpCur--;                                                      \
    }                                                                       \
    else if (memset_zero == ILT_SET)                                        \
    {                                                                       \
        K_ii.iltCur--;                                                      \
    }
#endif  /* BACKEND */
#ifdef FRONTEND
  #define get_special_set_stats(memset_zero)                                \
    if (memset_zero == MINMAXTBL_SET || memset_zero == Z_MINMAXTBL_SET)     \
    {                                                                       \
        i_minmaxtbl_set(memset_zero);                                       \
    }                                                                       \
    else if (memset_zero == ILT_SET)                                        \
    {                                                                       \
        K_ii.iltCur--;                                                      \
    }
#endif  /* FRONTEND */
#ifdef CCB_RUNTIME_CODE
  #define get_special_set_stats(memset_zero)
#endif  /* CCB_RUNTIME_CODE */

/* ........................................................................ */
/*
 * get_name_another_init - SCMT's are initialized, but never increase the
 *                         number of them during running.
 */
#ifdef BACKEND
  #define get_name_another_init(name, memset_zero, filename, linenumber)    \
        if (memset_zero == SCMT_SET)                                        \
        {                                                                   \
            get_name_history_keep(name, NULL);                              \
            return (NULL);       /* Do not get another item for pool. */    \
        }                                                                   \
        /* Get one more #name and put onto free list. */                    \
        init_##name(1, filename, linenumber);                               \
        working = pool_##name.first;
#else   /* BACKEND (i.e. FRONTEND || CCB_RUNTIME_CODE) */
  #define get_name_another_init(name, memset_zero, filename, linenumber)    \
        /* Get one more #name and put onto free list. */                    \
        init_##name(1, filename, linenumber);                               \
        working = pool_##name.first;
#endif  /* BACKEND (i.e. FRONTEND || CCB_RUNTIME_CODE) */
/* ........................................................................ */
/*
 * check_address_reasonable -- checks the put_##NAME addres to see that it is
 *                             aligned, and that pre and post patterns are ok.
 */
#ifndef PERF
  #define check_address_reasonable(LENGTH)                                  \
        if (((UINT32)working & (64-1)) != 0) {                             \
            fprintf(stderr, "%s put address not aligned on 64 -- 0x%08x 0x%02x (%d)\n", FEBEMESSAGE, (UINT32)working, (UINT32)working & (64-1), (UINT32)working & (64-1)); \
            abort();                                                        \
        }                                                                   \
        check_on_local_memory_address((char *)working, LENGTH); \
        check_memory_all();
#else   /* PERF */
  #define check_address_reasonable(LENGTH)
#endif  /* PERF */
/* ........................................................................ */
/*
 * set_first_last_allocated -- sets the memory_pool structure first & last
 *                             allocated linked list pointers.
 * NOTE: pcb's can get freed (if in shared memory), do not do this with them.
 * NOTE: scio's are an ilt and a vrp, thus next check don't work with them.
 */
#if defined(FRONTEND) || defined(BACKEND)
#define set_first_last_allocated(name)                                      \
        if (&pool_##name != &pool_pcb && &pool_##name != &pool_scio) {      \
            struct before_after *post = (struct before_after *)((char *)working + around); \
            if (pool_##name.first_allocated == NULL) {                      \
                pool_##name.first_allocated = post;                         \
                pool_##name.last_allocated = post;                          \
            }                                                               \
            else                                                            \
            {                                                               \
                struct before_after *opost = pool_##name.last_allocated;    \
                if (opost->next != (void *)0xdcdcdcdc) {                    \
                    fprintf(stderr, "%s%s memory corrupted %p.next=%p\n", FEBEMESSAGE, #name, opost, opost->next); \
                    abort();                                                \
                }                                                           \
                opost->next = post;                                         \
                pool_##name.last_allocated = post;                          \
            }                                                               \
        }
#endif  /* FRONTEND || BACKEND */
#if defined(CCB_RUNTIME_CODE)
#define set_first_last_allocated(name)                                      \
        struct before_after *post = (struct before_after *)((char *)working + around); \
        if (pool_##name.first_allocated == NULL) {                          \
            pool_##name.first_allocated = post;                             \
            pool_##name.last_allocated = post;                              \
        }
#endif  /* CCB_RUNTIME_CODE */

/* ........................................................................ */
/* ............... The big do everything macro starts now. ................ */
/* ........................................................................ */
#define allocate_pool(name, STRUCTURE, LENGTH, PATTERN, memset_zero, local_mem, number_free) \
  struct memory_pool pool_##name = { NULL, NULL, 0, 0, NULL, NULL };            \
                                                                                \
/**                                                                             \
******************************************************************************  \
**                                                                              \
**  @brief      Free a STRUCTURE.                                               \
**                                                                              \
**  @param      Address of STRUCTURE                                            \
**                                                                              \
**  @return     none                                                            \
**                                                                              \
******************************************************************************  \
**/                                                                             \
void put_##name(STRUCTURE *working, const char *filename, const unsigned int linenumber) \
{                                                                               \
    struct memory_list *w = (struct memory_list *)working;                      \
    check_address_reasonable(LENGTH);                                           \
                                                                                \
    put_name_history_keep(name, working);                                       \
    check_name_null(STRUCTURE);                                                 \
                                                                                \
    put_put_on_pool_list(name, STRUCTURE, PATTERN, memset_zero, LENGTH);        \
    put_special_values(memset_zero);                                            \
    put_increment_statistics(memset_zero);                                      \
}   /* End of put_##name */                                                     \
                                                                                \
/**                                                                             \
******************************************************************************  \
**                                                                              \
**  @brief      Initially allocate an #STRUCTURE.                               \
**                                                                              \
**  @param      num_alloc - put this number in pool.                            \
**                                                                              \
**  @return     none                                                            \
**                                                                              \
******************************************************************************  \
**/                                                                             \
void init_##name(int num_alloc, const char *filename, const unsigned int linenumber) \
{                                                                               \
    STRUCTURE *working;                                                         \
    int        i;                                                               \
    size_t asize = LENGTH < MIN_LTH ? MIN_LTH : LENGTH;                         \
    size_t around = (asize + ROUND_UP - 1) & ~(ROUND_UP - 1);                   \
                                                                                \
    for (i = 0; i < num_alloc; i++)                                             \
    {                                                                           \
        /* Get a #STRUCTURE and put onto free list. */                          \
        working = (STRUCTURE *)local_perm_malloc(LENGTH, #STRUCTURE, local_mem); \
        pool_##name.num_allocated++;                                            \
        init_initialize_statistics(memset_zero);                                \
        i_minmaxtbl_set(memset_zero);                                           \
        set_first_last_allocated(name);                                         \
        put_##name(working, filename, linenumber);                              \
    }                                                                           \
}   /* End of init_##name */                                                    \
                                                                                \
/**                                                                             \
******************************************************************************  \
**                                                                              \
**  @brief      Allocate an #STRUCTURE from free list.                          \
**                                                                              \
**  @param      none                                                            \
**                                                                              \
**  @return     A pointer to an #STRUCTURE.                                     \
**                                                                              \
**  @attention  This routine may ultimately task switch on memory alloction.    \
**                                                                              \
******************************************************************************  \
**/                                                                             \
STRUCTURE *get_##name(const char *filename, const unsigned int linenumber)      \
{                                                                               \
    struct memory_list *working = pool_##name.first;                            \
    get_var_define;                                                             \
                                                                                \
    /* If no free name##'s, initially allocate another one. */                  \
    if_get_name_free_pool_check(name, memset_zero, number_free)                 \
    {                                                                           \
        get_name_another_init(name, memset_zero, filename, linenumber);         \
    }                                                                           \
    else                                                                        \
    {                                                                           \
        check_address_reasonable(LENGTH);                                       \
    }                                                                           \
    get_name_pattern_still_set(STRUCTURE, PATTERN, memset_zero, LENGTH);        \
    struct before_after *pre = ((struct before_after *)working) - 1;            \
    pool_##name.first = pre->next;                                              \
    pre->next = NULL;                                                           \
    pre->used_or_free = 1;                                                      \
    size_t asize = LENGTH < MIN_LTH ? MIN_LTH : LENGTH;                         \
    size_t around = (asize + ROUND_UP - 1) & ~(ROUND_UP - 1);                   \
    struct before_after *post = (struct before_after *)((char *)(pre + 1) + around); \
    post->used_or_free = 1;                                                     \
    get_name_taskswitch;                                                        \
    /* If there are no more entries, get rid of tail. */                        \
    if (pool_##name.tail == working)                                            \
    {                                                                           \
        pool_##name.tail = NULL;                                                \
    }                                                                           \
    /* Decrement number of free memory segments. */                             \
    pool_##name.num_free--;                                                     \
    /* NOTE: FREEZERO and DTMT_SET done on free. */                             \
    if (memset_zero == ZEROED || memset_zero == Z_MINMAXTBL_SET ||              \
        memset_zero == ANYTHING || memset_zero == TPMT_SET ||                   \
        memset_zero == SCMT_SET || memset_zero == VRP_SET ||                    \
        memset_zero == RRB_SET || memset_zero == ILT_SET ||                     \
        memset_zero == MINMAXTBL_SET || memset_zero == PRP_SET ||               \
        memset_zero == RPN_SET || memset_zero == RRP_SET)                       \
    {                                                                           \
        memset(working, 0, LENGTH);                                             \
    }                                                                           \
    get_set_values(memset_zero);                                                \
    get_special_set_stats(memset_zero);                                         \
    get_name_history_keep(name, working);                                       \
    fill_post_file_line;                                                        \
    return ((STRUCTURE *)working);                                              \
}   /* End of get_##name */

/* ------------------------------------------------------------------------ */
/*
 * Create a special memory section for BE only memory, FE only memory, etc.
 */

extern char local_memory_start;

#if defined(FRONTEND)
asm(".section .fe_only_mem,\"aw\",@nobits");
asm(".globl Check_fe_only_start"); asm("Check_fe_only_start:");
asm(".globl local_memory_start"); asm("local_memory_start:");
asm(".space " stringify(PRIVATE_SIZE) ",0");
asm(".globl Check_fe_only_end"); asm("Check_fe_only_end:");
#endif  /* FRONTEND */
#if defined(BACKEND)
asm(".section .be_only_mem,\"aw\",@nobits");
asm(".globl Check_be_only_start"); asm("Check_be_only_start:");
asm(".globl local_memory_start"); asm("local_memory_start:");
asm(".space " stringify(PRIVATE_SIZE) ",0");
asm(".globl Check_be_only_end"); asm("Check_be_only_end:");
#endif  /* BACKEND */
#if defined(CCB_RUNTIME_CODE)
asm(".section .ccb_only_mem,\"aw\",@nobits");
asm(".globl Check_ccb_only_start"); asm("Check_ccb_only_start:");
asm(".globl local_memory_start"); asm("local_memory_start:");
asm(".space " stringify(PRIVATE_SIZE) ",0");
asm(".globl Check_ccb_only_end"); asm("Check_ccb_only_end:");
#endif  /* CCB_RUNTIME_CODE */
asm(".text");                   /* Make sure .text section again. */

/* ------------------------------------------------------------------------ */
#define MEMORY_POOL_USABLE      PRIVATE_SIZE
/*
 * The amount of private memory we start with. Variable for easier use in gdb.
 */
unsigned int local_memory_pool_start = PRIVATE_SIZE;

/*
 * The following lets us know (once) that we ran out of local memory once.
 * Please note that a big allocation might fail, but then a small allocation
 * might work. The next failure after the first will not print a message.
 */
char        Out_of_local_memory_print = 0;

/*
 * The following is to allow .gdbinit to know the size of a full ILT. It just
 * needed to be someplace in some file.
 */
size_t      size_ILT_ALL_LEVELS = sizeof(ILT_ALL_LEVELS);

#ifndef CCB_RUNTIME_CODE
/*
 * The following is for delayed free of a PCB -- because we must run the
 * scheduler on this PCB before selecting the next process to run.
 */
UINT32     *K_delayed_pcb_free = NULL;

// Although only used in the BACKEND, macro uses it in both.
struct memory_pool pool_scio = { NULL, NULL, 0, 0, NULL, NULL };
#endif  /* CCB_RUNTIME_CODE */

/* ------------------------------------------------------------------------ */
/*
 * This grabs memory from the local memory pool, and it is never returned.
 * Except of course that when we run out of local memory, then we get it from
 * the shared pool, and is never returned -- except for PCBs.
 */
void       *local_perm_malloc(size_t in_size, const char *str, int local_mem)
{
// #if !defined(PERF) && defined(M4_DEBUG_MEMORY_WITH_PATTERNS)
//     check_local_memory_before_after();
// #endif /* !defined(PERF) && defined(M4_DEBUG_MEMORY_WITH_PATTERNS) */
    void       *ret;

    if (local_mem == SHARED_MEMORY_ALLOCATION)
    {
#ifndef CCB_RUNTIME_CODE
        ret = s_MallocW(in_size | BIT31, str, __LINE__);
#else   /* CCB_RUNTIME_CODE */
        ret = XK_Malloc(in_size | BIT31, MEM_MALLOC, 1, str, __LINE__);
#endif  /* CCB_RUNTIME_CODE */
    }
    else
    {
        ret = p_Malloc(in_size | BIT31, str, __LINE__);
        if (ret == NULL)
        {
            if (Out_of_local_memory_print == 0)
            {
                fprintf(stderr, "%sOut of local memory allocating %s -- going to shared memory\n",
                        FEBEMESSAGE, str);
                Out_of_local_memory_print++;
            }
#ifndef CCB_RUNTIME_CODE
            ret = s_MallocW(in_size | BIT31, str, __LINE__);
#else   /* CCB_RUNTIME_CODE */
            ret = XK_Malloc(in_size | BIT31, MEM_MALLOC, 1, str, __LINE__);
#endif  /* CCB_RUNTIME_CODE */
        }
    }
    return (ret);
}                               /* End of local_perm_malloc */

/* ------------------------------------------------------------------------ */
/*
 * This will yell and abort if local private memory is attempting to be freed.
 */
NORETURN void free_local_memory(void *address, size_t the_size)
{
    fprintf(stderr, "%sTrying to free local memory 0x%08x of size 0x%x (%d) but must never do that.\n",
            FEBEMESSAGE, (UINT32)address, the_size, the_size);
    abort();
}                               /* End of free_local_memory */

/* ------------------------------------------------------------------------ */
#if !defined(PERF) || defined(CHECK_MEMORY_ALL)
// A) Routine given malloc-ed address and length to check before and after areas.
// B) Routine to travel through the list.

// char *address; is input malloc-ed address.
// size_t size; is input size of memory.
void check_on_local_memory_address(void *address, size_t in_size)
{
    size_t      size = in_size < MIN_LTH ? MIN_LTH : in_size;
    size_t      round_up_size = (size + ROUND_UP - 1) & ~(ROUND_UP - 1);
    struct before_after *pre = (struct before_after *)((char *)address - EXTRA_SPACE);
    struct before_after *post = (struct before_after *)((char *)address + round_up_size);

    if (round_up_size - in_size > 0)
    {
// A way to memcmp area after size to the pattern.
        static const char *round_up_string = "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD";
        void       *end = (char *)address + in_size;

        if (memcmp(end, round_up_string, round_up_size - in_size) != 0)
        {
            fprintf(stderr, "%s%s:%s:%u round-up area after %p of length %u does not match 0x44 (0x%08x 0x%08x)\n",
                    FEBEMESSAGE, __FILE__, __func__, __LINE__,
                    address, in_size, *(UINT32 *)end, *((UINT32 *)end + 1));
            abort();
        }
    }
    if (pre->pre1 != 0xdddddddd || pre->pre2 != 0xdddddddd)
    {
        fprintf(stderr, "%s%s:%s:%u pre-area %p of length %u does not match (%x&%x!=dddddddd)\n",
                FEBEMESSAGE, __FILE__, __func__, __LINE__,
                address, in_size, pre->pre1, pre->pre2);
        abort();
    }
    if (post->pre1 != 0xdcdcdcdc || post->pre2 != 0xdcdcdcdc)
    {
        fprintf(stderr, "%s%s:%s:%u post-area %p of length %u does not match (%x&%x!=dcdcdcdc)\n",
                FEBEMESSAGE, __FILE__, __func__, __LINE__,
                address, in_size, post->pre1, post->pre2);
        abort();
    }
    if (pre->length != in_size || post->length != in_size)
    {
        fprintf(stderr, "%s%s:%s:%u post-area %p of length %u has length pre=%u not matchine post=%u\n",
                FEBEMESSAGE, __FILE__, __func__, __LINE__,
                address, in_size, pre->length, post->length);
        abort();
    }

    if (pre->used_or_free == 0)             // If free
    {
        if (post->used_or_free != 0)
        {
            fprintf(stderr, "%s%s:%s:%u area %p of length %u has pre-used/free=%d and post=%u\n",
                    FEBEMESSAGE, __FILE__, __func__, __LINE__,
                    address, in_size, pre->used_or_free, post->used_or_free);
            abort();
        }
#ifdef M4_DEBUG_MEMORY_WITH_PATTERNS
        /* 0x544d5054 = TPMT     0x544d5444 = DTMT */
        if (*(UINT32 *)pre->str != 0x544d5054 && *(UINT32 *)pre->str != 0x544d5444)
        {
            /* Check memory set to pattern (first byte matches rest). */
            unsigned short *ca = (unsigned short *)address;
            unsigned short first_value = *ca;
            size_t      i;
            size_t      j;

            j = ((in_size < 1024) ? in_size : 1024) / 2;
            for (i = 1; i < j; i++)
            {
                if (ca[i] != first_value)
                {
                    fprintf(stderr, "%s%s:%s:%u area %p of length %u does not have complete memory pattern 0x%04x - offset %d has 0x%04x\n",
                            FEBEMESSAGE, __FILE__, __func__, __LINE__,
                            address, in_size, first_value, i * 2, ca[i]);
                    abort();
                }
            }
        }
#endif  /* M4_DEBUG_MEMORY_WITH_PATTERNS */
    }
    else if (pre->used_or_free == 1)        // If used
    {
        if (post->used_or_free != 1)
        {
            fprintf(stderr, "%s%s:%s:%u area %p of length %u has pre-used/free=%d and post=%u\n",
                    FEBEMESSAGE, __FILE__, __func__, __LINE__,
                    address, in_size, pre->used_or_free, post->used_or_free);
            abort();
        }
    }
#ifdef REALLY_DELAY_MEMORY_FREES
    else if (pre->used_or_free == 2)        // Delayed memory free
    {
        if (post->used_or_free != 1)
        {
            fprintf(stderr, "%s%s:%s:%u area %p of length %u has pre-used/free=%d (want 2) and post=%u (want 1)\n",
                    FEBEMESSAGE, __FILE__, __func__, __LINE__,
                    address, in_size, pre->used_or_free, post->used_or_free);
            abort();
        }
        /* Check memory set to pattern 0xffff. */
        unsigned short *ca = (unsigned short *)address;
        size_t      i;
        size_t      j;

        j = ((in_size < 1024) ? in_size : 1024) / 2;    // Max of 1024.
        for (i = 0; i < j; i++)
        {
            if (ca[i] != 0xffff)
            {
                fprintf(stderr, "%s%s:%s:%u area %p of length %u does not have complete memory pattern 0xffff - offset %d has 0x%04x\n",
                        FEBEMESSAGE, __FILE__, __func__, __LINE__,
                        address, in_size, i * 2, ca[i]);
                abort();
            }
        }
    }
#endif  /* REALLY_DELAY_MEMORY_FREES */
    else
    {
        fprintf(stderr, "%s%s:%s:%u area %p of length %u has pre-used/free=%d and post=%u and not 0/1/2 correctly\n",
                FEBEMESSAGE, __FILE__, __func__, __LINE__,
                address, in_size, pre->used_or_free, post->used_or_free);
        abort();
    }
}                               /* End of check_on_local_memory_address */

#endif  /* !defined(PERF) */

/* ------------------------------------------------------------------------ */
/*
 * The following  define the various memory pools, and their "c" structure names,
 * and the memory pattern to use if debugging with memory poisoning.
 */

/* name         - Name of Pool. Creates get_name, put_name, init_name, pool_name.
 * STRUCTURE    - "C" structure name.
 * PATTERN      - Memory fill pattern when freed (short).
 * LENGTH       - Length of one memory pool entry (see ILT for an example of this mess).
 * memset_zero  - See #defines for memset_zero argument above. Look for ZEROED.
 * local_mem    - Either SHARED_MEMORY_ALLOCATION (old way), or the new way
 *                  PRIVATE_MEMORY_ALLOCATION.
 */
/* #define allocate_pool(name, STRUCTURE, LENGTH, PATTERN, memset_zero, local_mem, number_free) */


/* NOTE: see files CT_asm_defines.h and mem_pool.h, and define allocate_pool_check */

/* allocate_pool(name, structure, length, pattern, 0=memset to zero, PRIVATE_MEMORY_ALLOCATION, number_free); */

#if defined(FRONTEND)
  extern int  MSAlloc_SZ;

  #undef get_tmt
  #undef put_tmt
  #undef init_tmt
  allocate_pool(tmt, TMT, sizeof(TMT), ACED, Z_MINMAXTBL_SET, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_tlmt
  #undef put_tlmt
  #undef init_tlmt
  allocate_pool(tlmt, TLMT, sizeof(TLMT), CAFF, Z_MINMAXTBL_SET, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_ismt
  #undef put_ismt
  #undef init_ismt
  allocate_pool(ismt, ISMT, sizeof(ISMT), DECD, MINMAXTBL_SET, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_xli
  #undef put_xli
  #undef init_xli
  allocate_pool(xli, XLI, sizeof(XLI), FAFF, MINMAXTBL_SET, PRIVATE_MEMORY_ALLOCATION, 64);

  #undef get_ltmt
  #undef put_ltmt
  #undef init_ltmt
  allocate_pool(ltmt, LTMT, sizeof(LTMT), 0000, FREEZERO, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_irp
  #undef put_irp
  #undef init_irp
  allocate_pool(irp, IRP, sizeof(IRP), CAFE, ZEROED, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_lsmt
  #undef put_lsmt
  #undef init_lsmt
  allocate_pool(lsmt, LSMT, sizeof(LSMT), 0000, FREEZERO, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_imt
  #undef put_imt
  #undef init_imt
  allocate_pool(imt, IMT, sizeof(IMT), DEDE, ZEROED, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_vdmt
  #undef put_vdmt
  #undef init_vdmt
  allocate_pool(vdmt, VDMT, sizeof(VDMT), FBFB, ZEROED, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_ilmt
  #undef put_ilmt
  #undef init_ilmt
  allocate_pool(ilmt, ILMT, sizeof(ILMT) + MSAlloc_SZ, FAFA, ZEROED, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_wc_plholder
  #undef put_wc_plholder
  #undef init_wc_plholder
  allocate_pool(wc_plholder, ILT, sizeof(ILT), EFEF, ZEROED, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_wc_rbinode
  #undef put_wc_rbinode
  #undef init_wc_rbinode
  allocate_pool(wc_rbinode, RB, sizeof(RB), DBDB, ZEROED, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_wc_rbnode
  #undef put_wc_rbnode
  #undef init_wc_rbnode
  allocate_pool(wc_rbnode, RB, sizeof(RB), DAFE, ZEROED, PRIVATE_MEMORY_ALLOCATION, 64);
#endif  /* FRONTEND */

#if defined(BACKEND)
  #undef get_scmte
  #undef put_scmte
  #undef init_scmte
  allocate_pool(scmte, SCMTE, sizeof(SCMTE), DFDF, ZEROED, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_prp
  #undef put_prp
  #undef init_prp
  allocate_pool(prp, PRP, sizeof(PRP), FEED, PRP_SET, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_rrp
  #undef put_rrp
  #undef init_rrp
  allocate_pool(rrp, RRP, sizeof(RRP), BEAD, RRP_SET, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_rpn
  #undef put_rpn
  #undef init_rpn
  allocate_pool(rpn, RPN, sizeof(RPN), BADE, RPN_SET, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_rrb
  #undef put_rrb
  #undef init_rrb
  allocate_pool(rrb, RRB, sizeof(RRB), CEED, RRB_SET, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_vlar
  #undef put_vlar
  #undef init_vlar
  allocate_pool(vlar, VLAR, sizeof(VLAR), ABED, ZEROED, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_rm
  #undef put_rm
  #undef init_rm
  allocate_pool(rm, RM, sizeof(RM), FCFC, ZEROED, SHARED_MEMORY_ALLOCATION, 64);
  #undef get_sm
  #undef put_sm
  #undef init_sm
  allocate_pool(sm, SM, sizeof(SM), FDFD, ZEROED, SHARED_MEMORY_ALLOCATION, 64);
  #undef get_cm
  #undef put_cm
  #undef init_cm
  allocate_pool(cm, CM, sizeof(CM), EAEA, ZEROED, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_cor
  #undef put_cor
  #undef init_cor
  allocate_pool(cor, COR, sizeof(COR), EBEB, ZEROED, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_scd
  #undef put_scd
  #undef init_scd
  allocate_pool(scd, SCD, sizeof(SCD), ECEC, ZEROED, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_dcd
  #undef put_dcd
  #undef init_dcd
  allocate_pool(dcd, DCD, sizeof(DCD), EDED, ZEROED, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_tpmt
  #undef put_tpmt
  #undef init_tpmt
  allocate_pool(tpmt, TPMT, sizeof(TPMT), 0000, TPMT_SET, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_scmt
  #undef put_scmt
  #undef init_scmt
  allocate_pool(scmt, SCMT, sizeof(SCMT), BEEF, SCMT_SET, PRIVATE_MEMORY_ALLOCATION, 64);
#endif  /* BACKEND */

#if defined(FRONTEND) || defined(BACKEND)
  /* BOTH FE and BE below. */
  #undef get_qrp
  #undef put_qrp
  #undef init_qrp
  allocate_pool(qrp, QRP, sizeof(QRP), FADE, ANYTHING, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_dtmt
  #undef put_dtmt
  #undef init_dtmt
  allocate_pool(dtmt, DTMT, sizeof(DTMT), 0000, DTMT_SET, PRIVATE_MEMORY_ALLOCATION, 64);
  // NOTE: MLMT's are never freed. WOW.  2008-10-20
  #undef get_mlmt
  #undef put_mlmt
  #undef init_mlmt
  allocate_pool(mlmt, MLMT, sizeof(MLMT), DAFF, ZEROED, PRIVATE_MEMORY_ALLOCATION, 64);
  #undef get_pcb
  #undef put_pcb
  #undef init_pcb
  allocate_pool(pcb, PCB, sizeof(PCB), EEEE, ZEROED, PRIVATE_MEMORY_ALLOCATION, 0);
#endif  /* FRONTEND || BACKEND */

/* FE, BE, CCB below. */
#undef get_ilt
#undef put_ilt
#undef init_ilt
allocate_pool(ilt, ILT, sizeof(ILT_ALL_LEVELS), DAAD, ILT_SET, SHARED_MEMORY_ALLOCATION, 64);
#undef get_vrp
#undef put_vrp
#undef init_vrp
allocate_pool(vrp, VRP, VRPALLOC, DEED, VRP_SET, SHARED_MEMORY_ALLOCATION, 64);

/* ------------------------------------------------------------------------ */
#if defined(BACKEND) || defined(FRONTEND)
/* I am putting this declaration here - I want only one place to use this. */
void free_pcb(struct PCB *);
void free_pcb(struct PCB *pcb)
{
    if ((UINT32)pcb >= (UINT32)&local_memory_start &&
        (UINT32)pcb <= (UINT32)((UINT32)&local_memory_start + PRIVATE_SIZE))
    {
        put_pcb(pcb, __FILE__, __LINE__);
    }
    else
    {
#ifdef HISTORY_KEEP
        CT_history1("entering free_pcb with memory", (UINT32)pcb);
#endif  /* HISTORY_KEEP */
        s_Free((void *)pcb, sizeof(struct PCB), __FILE__, __LINE__);
        pool_pcb.num_allocated--;       /* Decrement permanent allocation number. */
    }
#ifdef HISTORY_KEEP
    CT_history1("exiting free_pcb num_allocated=", (UINT32)pool_pcb.num_allocated);
    CT_history1("exiting free_pcb num_free=", (UINT32)pool_pcb.num_free);
#endif  /* HISTORY_KEEP */
}                               /* End of free_pcb */


/* I am putting this declaration here - I want only one place to use this. */
struct PCB *malloc_pcb(void);
struct PCB *malloc_pcb(void)
{
#ifdef HISTORY_KEEP
    CT_history1("entering malloc_pcb num_allocated=", (UINT32)pool_pcb.num_allocated);
    CT_history1("entering malloc_pcb num_free=", (UINT32)pool_pcb.num_free);
#endif  /* HISTORY_KEEP */
    return (get_pcb(__FILE__, __LINE__));
}                               /* End of malloc_pcb */

#endif  /* BACKEND || FRONTEND */

/* ------------------------------------------------------------------------ */
/*
 * NOTE: THIS POOL HAS LINK AT END OF POOL, NOT IN SEPARATE MEMORY.
 */
#if defined(BACKEND)
// This is defined above, because a forward to it is required.
// struct memory_pool pool_scio = { NULL, NULL, 0, 0, NULL, NULL };

/*
 * Special put_scio() routine.
 */
#undef put_scio
void put_scio(SCIO1 *working, const char *filename, const unsigned int linenumber)
{
    struct memory_list *w = (struct memory_list *)working;

    check_address_reasonable(sizeof(ILT_ALL_LEVELS));
    put_name_history_keep(scio, working);
    check_name_null(ILT);

#ifdef M4_DEBUG_MEMORY_WITH_PATTERNS
    int         end_ilt = (sizeof(ILT_ALL_LEVELS) / sizeof(struct memory_list *)) - 1;
    // set_memory_pattern(BAAD, memset_zero);
    /* Set memory pattern if in debug mode. */
    int         i;
    VRP        *save_vrp = working->vrp;
    int         end_vrp = (VRPALLOC / sizeof(struct memory_list *)) - 1;
    struct memory_list *v = (struct memory_list *)save_vrp;

    for (i = 0; i <= end_ilt; i++)
    {
        w->memory_list_next[i] = (struct memory_list *)0xBAADBAAD;
    }
    working->vrp = save_vrp;    // Restore pointer to vrp

    for (i = 0; i <= end_vrp; i++)
    {
        v->memory_list_next[i] = (struct memory_list *)0xBADABADA;
    }
#endif  /* M4_DEBUG_MEMORY_WITH_PATTERNS */
    struct before_after *pre;

    /* Put SCIO1 onto free list. */
    if (pool_scio.first == NULL)
    {
        check_pool_structure(scio);
        pool_scio.first = pool_scio.tail = w;
    }
    else
    {
        pre = ((struct before_after *)pool_scio.tail) - 1;
        pre->next = w;
        pool_scio.tail = w;
    }
    pre = ((struct before_after *)w) - 1;
//    pre->used_or_free = 1;    Do not do this for scio's, they are allocated ilt's.
    pre->next = NULL;
    size_t asize = sizeof(ILT_ALL_LEVELS);
    size_t around = (asize + ROUND_UP - 1) & ~(ROUND_UP - 1);
    struct before_after *post = (struct before_after *)((char *)(pre + 1) + around);
//    post->used_or_free = 0;    Do not do this for scio's, they are allocated ilt's.
    fill_post_file_line;
    pool_scio.num_free++;
}                               /* End of put_scio */

/*
 * Special init_scio() routine.
 */
#undef init_scio
void init_scio(int num_alloc, const char *filename, const unsigned int linenumber)
{
    ILT        *working;
    VRP        *pvrp;
    int         i;
    size_t      asize = sizeof(ILT_ALL_LEVELS) < MIN_LTH ? MIN_LTH : sizeof(ILT_ALL_LEVELS);
    size_t      around = (asize + ROUND_UP - 1) & ~(ROUND_UP - 1);

    for (i = 0; i < num_alloc; i++)
    {
        /* Get an SCIO (ILT/VRP combo) and put onto free list. */
        working = get_ilt(__FILE__, __LINE__);
        pvrp = get_vrp(__FILE__, __LINE__);
        working->ilt_normal.w0 = (UINT32)pvrp;      // No structure set up for this usage.
#if 1 /* VIJAY_MC */
        pvrp->gen2 = 0;                             // Zero vr_use2 so VRP does not need to be tracked.
#endif  /* 1 - VIJAY_MC */
        pool_scio.num_allocated++;
        set_first_last_allocated(scio);
        put_scio((SCIO1 *)working, filename, linenumber);
    }
}                               /* End of init_scio */

/*
 * Special get_scio() routine.
 */
#undef get_scio
SCIO1      *get_scio(const char *filename, const unsigned int linenumber)
{
    struct memory_list *working = pool_scio.first;

    /* If no free SCIO's, initially allocate another one. */
    if_get_name_free_pool_check(scio, ZEROED, 0)
    {
        get_name_another_init(scio, ZEROED, filename, linenumber);
    }
    else
    {
        check_address_reasonable(sizeof(ILT_ALL_LEVELS));
    }

    ILT        *pilt = (ILT *)working;
    VRP        *pvrp = (VRP *)pilt->ilt_normal.w0;  // No structure set up for this usage.

#ifdef M4_DEBUG_MEMORY_WITH_PATTERNS
    size_t      size = (sizeof(ILT_ALL_LEVELS)) < MIN_LTH ? MIN_LTH : sizeof(ILT_ALL_LEVELS);
    int         end_at = (sizeof(ILT_ALL_LEVELS) / sizeof(struct memory_list *)) - 1;

    pilt->ilt_normal.w0 = 0xBAADBAAD;               // Fill in vrp pointer.
    pvrp->gen2 = 0xBADABADA;                        // Fill in vr_use2.
    get_name_pattern_still_set(scio - ilt, BAAD, ZEROED, sizeof(ILT_ALL_LEVELS));
    working = (struct memory_list *)pvrp;
    get_name_pattern_still_set(scio - vrp, BADA, ZEROED, VRPALLOC);
    working = (struct memory_list *)pilt;
#endif  /* M4_DEBUG_MEMORY_WITH_PATTERNS */

    struct before_after *pre = ((struct before_after *)working) - 1;

    pool_scio.first = pre->next;
//    pre->used_or_free = 1;    Do not do this for scio's, they are allocated ilt's.
    pre->next = NULL;
    get_name_taskswitch;
    /* If there are no more entries, get rid of tail. */
    if (pool_scio.tail == working)
    {
        pool_scio.tail = NULL;
    }
    /* Decrement number of free memory segments. */
    pool_scio.num_free--;
    memset(working, 0, sizeof(ILT));    // Clear first level of ILT.
    pilt->ilt_normal.w0 = (UINT32)pvrp; // No structure set up for this usage.
#if 1 /* VIJAY_MC */
    pvrp->gen2 = 0;             // Zero vr_use2 so VRP does not need to be tracked.
#endif  /* 1 - VIJAY_MC */
    get_name_history_keep(scio, working);
    return ((SCIO1 *)working);
}                               /* End of get_scio */

#endif  /* BACKEND */

/* ------------------------------------------------------------------------ */
/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
