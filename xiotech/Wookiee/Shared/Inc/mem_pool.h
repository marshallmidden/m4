/* $Id: mem_pool.h 157461 2011-08-03 15:21:36Z m4 $ */
/**
******************************************************************************
**
**  @file       mem_pool.h
**
**  @brief      memory pool header file
**
**  Copyright (c) 2008 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _MEM_POOL_H_
#define _MEM_POOL_H_

/* FE, BE, and CCB */
#include "vrp.h"
#include "ilt.h"

/* Both FE and BE need following includes. */
#if defined(FRONTEND) || defined(BACKEND)
#include "qrp.h"
#include "dtmt.h"
#include "mlmt.h"
#endif  /* FRONTEND || BACKEND */

#if defined(FRONTEND)
  #include "tmt.h"
  #include "tlmt.h"
  #include "ismt.h"
  #include "xli.h"
  #include "ltmt.h"
  #include "irp.h"
  #include "lsmt.h"
  #include "imt.h"
  #include "ilmt.h"
  #include "vdmt.h"
  #include "rb.h"
#endif  /* FRONTEND */

#if defined(BACKEND)
  #include "prp.h"
  #include "rrp.h"
  #include "rpn.h"
  #include "rrb.h"
  #include "vlar.h"
  #include "copymap.h"
  #include "scmt.h"
  #include "tpmt.h"
  #include "cm.h"
  #include "cor.h"
  #include "scd.h"
  #include "dcd.h"
  #include "qu.h"
  extern QU cm_exec_qu_norm;       // For SCMT put_scmt processing.
  #include "scr.h"
#endif  /* BACKEND */

// Check that an address has it's pre and post allocation structures ok.
extern void check_on_local_memory_address(void *, size_t);

/*
******************************************************************************
** Private defines - data structures -- used internal.
******************************************************************************
*/

// Minimum length of an allocation.
#define MIN_LTH         64      // Must be at least 64 for memory_list struct.
// Round allocations up to this alignment.
#define ROUND_UP        64      // Round to multiple of 64 for allocations.
// Fill pattern for extra bytes till alignment is reached.
// memory pattern filling extra unused allocation of the ROUND_UP area. (D = 0x44)
// Structure for pre and post pattern in debug mode.
/* Pattern before is
 *      0xdddddddd          4
 *      STRING11            11  __FILE__ (11), or memory pool name(5) and __FILE__(6).
 *      used/free           1   Flag, 0 if free, 1 if used. (0 if new)
 *      line_number         4   __LINE__
 *      LENGTH              4
 *      next                4   next free address (not before_after structure)
 *      0xdddddddd          4 = 32 bytes */
/* Pattern AFTER is
 *      0x44 memset to fill ROUND_UP byte rounding
 *      0xdcdcdcdc          4
 *      STRING11            11  __FILE__ (11), or memory pool name(5) and __FILE__(6).
 *                               NOTE: AFTER is where freed if used/free=0.
 *      used/free           1   Flag, 0 if free, 1 if used. (0 if new)
 *      line_number         4   __LINE__
 *                               NOTE: AFTER is where freed if used/free=0.
 *      LENGTH              4
 *      0xdcdcdcdc          4   "next" in pre pattern location.
 *      0xdcdcdcdc          4 = 32 bytes + ROUND_UP-1 possible bytes.
 */
struct before_after
{
    UINT32       pre1;          // Pre/post-pattern
    char         str[11];       // Type of memory pool -- 5 chars, +6 of filename.
    char         used_or_free;  // 0 if free, 1 if used, 2 if delayed free. (double free check)
    UINT32       line_number;   // Line memory was allocated on.
    UINT32       length;        // Length of memory section. (not counting PRE/POST)
    void        *next;          // Link for freed memory
    UINT32       pre2;          // Pre/post-pattern
};

// How much extra space is to be allocated before, and how much after.
#define EXTRA_SPACE   (sizeof(struct before_after))

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/*
 * This is to put the linked list at the end of the first allocated array.
 * It also happens to point to the head and tail of the linked list (below).
 */
struct memory_list {
    struct memory_list *memory_list_next[0]; /* Put at end of 64 byte allocated memory. */
};

/*
 * This is the structure of a memory pool.
 * A pointer to first (next element to be gotten).
 * A pointer to the last one free (last to be used).
 * Number ever allocated.
 * Number currently free. (Thus used = allocated - free.)
 */

/* This is copied into CT_asm_defines.h */
struct memory_pool {
    struct memory_list *first;  /* First free entry on list, next to use. */
    struct memory_list *tail;   /* Last free entry on list, last to use. */
    INT32  num_allocated;       /* Number that have been allocated. */
    INT32  num_free;            /* Number currently on free list. */
    /* NOTE: Following two point to (struct before_after *), and ->next is linked list. */
    /* last entry has ->next = 0xdcdcdcdc. */
    void *first_allocated;      /* The first ever allocated, post pointer. */
    void *last_allocated;       /* The last one allocated, post pointer. */
};

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

/* New memory pool allocation methodology and usage. */
#define memory_externs(name, NAME)                                      \
    extern struct memory_pool pool_##name;                              \
    extern void init_##name(int, const char *, const unsigned int);     \
    extern void put_##name(NAME *, const char *, const unsigned int);   \
    extern NAME *get_##name(const char *, const unsigned int);


/* NOTE: See files CT_asm_defines.h and mem_pool.c for more definitions. */
/* I.e. The lists should be the same in the 3 files. */

#if defined(FRONTEND)
  memory_externs(tmt, TMT);
  #define get_tmt() get_tmt(__FILE__, __LINE__);
  #define put_tmt(xxx) put_tmt(xxx, __FILE__, __LINE__);
  #define init_tmt(xxx) init_tmt(xxx, __FILE__, __LINE__);
  memory_externs(tlmt, TLMT);
  #define get_tlmt() get_tlmt(__FILE__, __LINE__);
  #define put_tlmt(xxx) put_tlmt(xxx, __FILE__, __LINE__);
  #define init_tlmt(xxx) init_tlmt(xxx, __FILE__, __LINE__);
  memory_externs(ismt, ISMT);
  #define get_ismt() get_ismt(__FILE__, __LINE__);
  #define put_ismt(xxx) put_ismt(xxx, __FILE__, __LINE__);
  #define init_ismt(xxx) init_ismt(xxx, __FILE__, __LINE__);
  memory_externs(xli, XLI);
  #define get_xli() get_xli(__FILE__, __LINE__);
  #define put_xli(xxx) put_xli(xxx, __FILE__, __LINE__);
  #define init_xli(xxx) init_xli(xxx, __FILE__, __LINE__);
  memory_externs(ltmt, LTMT);
  #define get_ltmt() get_ltmt(__FILE__, __LINE__);
  #define put_ltmt(xxx) put_ltmt(xxx, __FILE__, __LINE__);
  #define init_ltmt(xxx) init_ltmt(xxx, __FILE__, __LINE__);
  memory_externs(irp, IRP);
  #define get_irp() get_irp(__FILE__, __LINE__);
  #define put_irp(xxx) put_irp(xxx, __FILE__, __LINE__);
  #define init_irp(xxx) init_irp(xxx, __FILE__, __LINE__);
  memory_externs(lsmt, LSMT);
  #define get_lsmt() get_lsmt(__FILE__, __LINE__);
  #define put_lsmt(xxx) put_lsmt(xxx, __FILE__, __LINE__);
  #define init_lsmt(xxx) init_lsmt(xxx, __FILE__, __LINE__);
  memory_externs(imt, IMT);
  #define get_imt() get_imt(__FILE__, __LINE__);
  #define put_imt(xxx) put_imt(xxx, __FILE__, __LINE__);
  #define init_imt(xxx) init_imt(xxx, __FILE__, __LINE__);
  memory_externs(vdmt, VDMT);
  #define get_vdmt() get_vdmt(__FILE__, __LINE__);
  #define put_vdmt(xxx) put_vdmt(xxx, __FILE__, __LINE__);
  #define init_vdmt(xxx) init_vdmt(xxx, __FILE__, __LINE__);
  memory_externs(ilmt, ILMT);
  #define get_ilmt() get_ilmt(__FILE__, __LINE__);
  #define put_ilmt(xxx) put_ilmt(xxx, __FILE__, __LINE__);
  #define init_ilmt(xxx) init_ilmt(xxx, __FILE__, __LINE__);
  memory_externs(wc_plholder, ILT);
  #define get_wc_plholder() get_wc_plholder(__FILE__, __LINE__);
  #define put_wc_plholder(xxx) put_wc_plholder(xxx, __FILE__, __LINE__);
  #define init_wc_plholder(xxx) init_wc_plholder(xxx, __FILE__, __LINE__);
  memory_externs(wc_rbinode, RB);
  #define get_wc_rbinode() get_wc_rbinode(__FILE__, __LINE__);
  #define put_wc_rbinode(xxx) put_wc_rbinode(xxx, __FILE__, __LINE__);
  #define init_wc_rbinode(xxx) init_wc_rbinode(xxx, __FILE__, __LINE__);
  memory_externs(wc_rbnode, RB);
  #define get_wc_rbnode() get_wc_rbnode(__FILE__, __LINE__);
  #define put_wc_rbnode(xxx) put_wc_rbnode(xxx, __FILE__, __LINE__);
  #define init_wc_rbnode(xxx) init_wc_rbnode(xxx, __FILE__, __LINE__);
#endif  /* FRONTEND */

#if defined(BACKEND)
  memory_externs(scmte, SCMTE);
  #define get_scmte() get_scmte(__FILE__, __LINE__);
  #define put_scmte(xxx) put_scmte(xxx, __FILE__, __LINE__);
  #define init_scmte(xxx) init_scmte(xxx, __FILE__, __LINE__);
  memory_externs(prp, PRP);
  #define get_prp() get_prp(__FILE__, __LINE__);
  #define put_prp(xxx) put_prp(xxx, __FILE__, __LINE__);
  #define init_prp(xxx) init_prp(xxx, __FILE__, __LINE__);
  memory_externs(rrp, RRP);
  #define get_rrp() get_rrp(__FILE__, __LINE__);
  #define put_rrp(xxx) put_rrp(xxx, __FILE__, __LINE__);
  #define init_rrp(xxx) init_rrp(xxx, __FILE__, __LINE__);
  memory_externs(rpn, RPN);
  #define get_rpn() get_rpn(__FILE__, __LINE__);
  #define put_rpn(xxx) put_rpn(xxx, __FILE__, __LINE__);
  #define init_rpn(xxx) init_rpn(xxx, __FILE__, __LINE__);
  memory_externs(rrb, RRB);
  #define get_rrb() get_rrb(__FILE__, __LINE__);
  #define put_rrb(xxx) put_rrb(xxx, __FILE__, __LINE__);
  #define init_rrb(xxx) init_rrb(xxx, __FILE__, __LINE__);
  memory_externs(vlar, VLAR);
  #define get_vlar() get_vlar(__FILE__, __LINE__);
  #define put_vlar(xxx) put_vlar(xxx, __FILE__, __LINE__);
  #define init_vlar(xxx) init_vlar(xxx, __FILE__, __LINE__);
  memory_externs(rm, RM);
  #define get_rm() get_rm(__FILE__, __LINE__);
  #define put_rm(xxx) put_rm(xxx, __FILE__, __LINE__);
  #define init_rm(xxx) init_rm(xxx, __FILE__, __LINE__);
  memory_externs(sm, SM);
  #define get_sm() get_sm(__FILE__, __LINE__);
  #define put_sm(xxx) put_sm(xxx, __FILE__, __LINE__);
  #define init_sm(xxx) init_sm(xxx, __FILE__, __LINE__);
  memory_externs(cm, CM);
  #define get_cm() get_cm(__FILE__, __LINE__);
  #define put_cm(xxx) put_cm(xxx, __FILE__, __LINE__);
  #define init_cm(xxx) init_cm(xxx, __FILE__, __LINE__);
  memory_externs(cor, COR);
  #define get_cor() get_cor(__FILE__, __LINE__);
  #define put_cor(xxx) put_cor(xxx, __FILE__, __LINE__);
  #define init_cor(xxx) init_cor(xxx, __FILE__, __LINE__);
  memory_externs(scd, SCD);
  #define get_scd() get_scd(__FILE__, __LINE__);
  #define put_scd(xxx) put_scd(xxx, __FILE__, __LINE__);
  #define init_scd(xxx) init_scd(xxx, __FILE__, __LINE__);
  memory_externs(dcd, DCD);
  #define get_dcd() get_dcd(__FILE__, __LINE__);
  #define put_dcd(xxx) put_dcd(xxx, __FILE__, __LINE__);
  #define init_dcd(xxx) init_dcd(xxx, __FILE__, __LINE__);
  memory_externs(tpmt, TPMT);
  #define get_tpmt() get_tpmt(__FILE__, __LINE__);
  #define put_tpmt(xxx) put_tpmt(xxx, __FILE__, __LINE__);
  #define init_tpmt(xxx) init_tpmt(xxx, __FILE__, __LINE__);
  memory_externs(scmt, SCMT);
  #define get_scmt() get_scmt(__FILE__, __LINE__);
  #define put_scmt(xxx) put_scmt(xxx, __FILE__, __LINE__);
  #define init_scmt(xxx) init_scmt(xxx, __FILE__, __LINE__);
  memory_externs(scio, SCIO1);
  #define get_scio() get_scio(__FILE__, __LINE__);
  #define put_scio(xxx) put_scio(xxx, __FILE__, __LINE__);
  #define init_scio(xxx) init_scio(xxx, __FILE__, __LINE__);
#endif  /* BACKEND */

#if defined(BACKEND) || defined(FRONTEND)
  memory_externs(qrp, QRP);
  #define get_qrp() get_qrp(__FILE__, __LINE__);
  #define put_qrp(xxx) put_qrp(xxx, __FILE__, __LINE__);
  #define init_qrp(xxx) init_qrp(xxx, __FILE__, __LINE__);
  memory_externs(dtmt, DTMT);
  #define get_dtmt() get_dtmt(__FILE__, __LINE__);
  #define put_dtmt(xxx) put_dtmt(xxx, __FILE__, __LINE__);
  #define init_dtmt(xxx) init_dtmt(xxx, __FILE__, __LINE__);
  memory_externs(mlmt, MLMT);
  #define get_mlmt() get_mlmt(__FILE__, __LINE__);
  #define put_mlmt(xxx) put_mlmt(xxx, __FILE__, __LINE__);
  #define init_mlmt(xxx) init_mlmt(xxx, __FILE__, __LINE__);
  memory_externs(pcb, PCB);
  #define get_pcb() get_pcb(__FILE__, __LINE__);
  #define put_pcb(xxx) put_pcb(xxx, __FILE__, __LINE__);
  #define init_pcb(xxx) init_pcb(xxx, __FILE__, __LINE__);
#endif  /* BACKEND || FRONTEND */

/* BE, FE, or CCB. */
memory_externs(ilt, ILT);
#define get_ilt() get_ilt(__FILE__, __LINE__);
#define put_ilt(xxx) put_ilt(xxx, __FILE__, __LINE__);
#define init_ilt(xxx) init_ilt(xxx, __FILE__, __LINE__);
memory_externs(vrp, VRP);
#define get_vrp() get_vrp(__FILE__, __LINE__);
#define put_vrp(xxx) put_vrp(xxx, __FILE__, __LINE__);
#define init_vrp(xxx) init_vrp(xxx, __FILE__, __LINE__);

/* The following allocates local memory that is never released. */
extern void *local_perm_malloc(size_t, const char *, int);
/* The following is for non-shared pcb memory freeing. */
extern void free_local_memory(void *address, size_t the_size);

#endif /* _MEM_POOL_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
