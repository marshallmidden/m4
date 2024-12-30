/* $Id: CT_history.h 143007 2010-06-22 14:48:58Z m4 $ */

#ifndef _CT_history_h_
/* Only do this once. */
#define _CT_history_h_

#include "pcb.h"
#include <sys/types.h>
extern ulong edata;
extern ulong __executable_start;

/* ------------------------------------------------------------------------ */
#ifdef HISTORY_KEEP
extern void CT_history(const char *);           /* string and length for trace log. */
extern void CT_history_store(const char *, int, int); /* string, value, @ location for trace log. */
extern void CT_history1(const char *, int);     /* string and argument for trace log. */
extern void CT_history_memory_size(const char *, int, int);
extern void CT_history_task_name(const char *str, const char *fork_name, PCB* pcb);
extern void CT_history_printf(const char *format, ...) __attribute__((__format__(__printf__,1,2)));
extern void CT_history_pcb(const char *str, UINT32 pcb);
extern void CT_HISTORY_OFF(void);
extern void CT_HISTORY_OFF_NOW(void);
extern void CT_HISTORY_ON(void);
extern void CT_history_disable_task(const char *);
#endif

/* ------------------------------------------------------------------------ */
#ifdef HISTORY_KEEP
extern unsigned int CT_NO_HISTORY;
#endif
/* ------------------------------------------------------------------------ */
#endif /* _CT_history_h_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
