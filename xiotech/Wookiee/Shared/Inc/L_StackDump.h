/* $Id: L_StackDump.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       L_StackDump.h
**
**  @brief      Stack dumping function definition.
**
**  Stack-dumping function definition for crash summarization.
**
**  Copyright (c) 2005-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#ifndef _L_STACKDUMP_H_
#define _L_STACKDUMP_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

#define LOG_PREPEND "## HISTORY"
#define LOG_MAX_HISTORY_COUNT   10
#define SINF_PC_INDEX   51  /* Index to PC value into siginfo_t as long * */
#define SINF_SP_INDEX   54  /* Index to SP value into siginfo_t as long * */

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/

#define PC_FROM_SIGINFO(sinf)   (((unsigned long **)sinf)[SINF_PC_INDEX])
#define SP_FROM_SIGINFO(sinf)   (((unsigned long **)sinf)[SINF_SP_INDEX])

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

struct StackDumpArgs {
    FILE    *sfp;
    unsigned long   *pc;
    unsigned long   *fsp;
    void    *saddr;
    UINT32  ebp;
};

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void    L_memdump(FILE *ofp, void *start, void *end);
extern void L_StackDump(struct StackDumpArgs *);
extern void L_rotate(const char *summary, const char *history,
                const char *tempfn, const char marker[]);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _L_STACKDUMP_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
