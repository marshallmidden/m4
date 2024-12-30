#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include "CT_defines.h"
#include "kernel.h"

/* ------------------------------------------------------------------------ */
#define MAX_CT_LOG    64000000
#define CT_TL_strings   800000          /* This number is also in .gdbinit tracelog macros. */
// #define MAX_CT_LOG   128000000
// #define CT_TL_strings  1600000          /* This number is also in .gdbinit tracelog macros. */

/* ------------------------------------------------------------------------ */
/*
 * Tasks that we disable processing during.
 */
const char *disable_task_names[] =
{
    "k$timer",                          // Timer task.
    "m$exec_hbeatproc",                 // heart beat generator task.
    "m$exec_hbeatmon",                  // heart beat monitoring task.
    "LL_TargetTask 1",                  // Link layer tasks
    "LL_TargetTask 2",                  // Link layer tasks
    "LL_CompletionTask 1",              // Link layer tasks
    "LL_CompletionTask 2",              // Link layer tasks
    "LL_InitiatorTask 1",               // Link layer tasks
    "LL_InitiatorTask 2",               // Link layer tasks
    "MM_MonitorTask",                   // MicroMemory card status task
    "NV_DMACm",                         // DMA completor task for MicroMemory card.
    "NV_DMAEx",                         // DMA executor task for MicroMemory card.
    "NV_ScrubTask",                     // DMA scrub task for MicroMemory card.
    "iscsiTimerTaskCallback",           //
    "wc_markWCache",                    // Mark write cache enable/disable task.
};

/* ------------------------------------------------------------------------ */
#ifdef HISTORY_KEEP
static char CT_tracelog[MAX_CT_LOG];
static int CT_last_history_string;          /* Last string. */
#define CT_PREV_HISTORY_STRINGS 9          /* Save 9 previous histories for "count" */
static int CT_prev_history_string[CT_PREV_HISTORY_STRINGS]; /* Previous 9 back strings. */
static int CT_laststrings[CT_TL_strings];
static int CT_count_repeats[CT_TL_strings];
static int CT_history_number = 0;
#endif /* HISTORY_KEEP */
#ifdef HISTORY_TSC_KEEP
static long long CT_last_tsc[CT_TL_strings];
#endif /* HISTORY_TSC_KEEP */
#ifdef HISTORY_REGS_KEEP
static unsigned int CT_save_g[CT_TL_strings][16];
static unsigned int CT_save_r[CT_TL_strings][16];
#endif  /* HISTORY_REGS_KEEP */
#ifdef HISTORY_KEEP
static char CT_buf[1024];               /* If need to sprintf a format string, do it here. */
static char CT_hexdigit[16] = "0123456789abcdef";
#endif /* HISTORY_KEEP */

/* ------------------------------------------------------------------------ */
#ifdef HISTORY_KEEP
static void CT_history_always(const char *buf);
#endif /* HISTORY_KEEP */

/* ------------------------------------------------------------------------ */
int CHECK_RREG_PATTERN_first_case = 0;
int CHECK_LOCAL_MEMORY_first_case = 0;

/* ------------------------------------------------------------------------ */
void CHECK_RREG_PATTERN(const char *str, ulong arg);
void CHECK_RREG_PATTERN(const char *str, ulong arg)
{
  if (CHECK_RREG_PATTERN_first_case != 0)
  {
    return;
  }

  if (arg == 0xBABEBABE)
  {
    fprintf(stderr, "%sCHECK_RREG_PATTERN: failed @ %s with %08lx\n", FEBEMESSAGE, str, arg);
    CHECK_RREG_PATTERN_first_case = 1;
    abort();
  }
}   /* End of CHECK_RREG_PATTERN */

/* ------------------------------------------------------------------------ */
// extern unsigned int __executable_start;  /* In CT_defines.h */
extern unsigned int __fini_array_end;
extern unsigned int end;

void CHECK_LOCAL_MEMORY(const char *str, ulong arg);
/* Allowed memory: */
/*    FE    FE shared memory, private memory, data, bss. */
/*    BE    BE shared memory, private memory, data, bss. */
/* Not allowed memory: */
/*    FE    BE shared memory, CCB shared memory, executable section, ro-data. MM only from "c". */
/*    BE    FE shared memory, CCB shared memory, executable section, ro-data. MM only from "c". */
void CHECK_LOCAL_MEMORY(const char *str, ulong arg)
{
  if (CHECK_LOCAL_MEMORY_first_case != 0)
  {
    return;
  }

  /* Shared memory. */
  if (arg >= FRONT_END_PCI_START && arg <= FE_SHARE_LIM)
  {
#ifdef FRONTEND
    return;         /* OK */
#else   /* FRONTEND */
    fprintf(stderr, "%sCHECK_LOCAL_MEMORY: FE shared accessed @ %s from %08lx\n", FEBEMESSAGE, str, arg);
    return;
#endif  /* !FRONTEND */
  }

  if (arg >= CCB_PCI_START && arg <= (ulong)CCB_PCI_START + SIZE_CCB_LTH)
  {
    fprintf(stderr, "%sCHECK_LOCAL_MEMORY: CCB shared accessed @ %s from %08lx\n", FEBEMESSAGE, str, arg);
    return;
  }

  if (arg >= BACK_END_PCI_START && arg <= BE_SHARE_LIM)
  {
#ifdef FRONTEND
    fprintf(stderr, "%sCHECK_LOCAL_MEMORY: BE shared accessed @ %s from %08lx\n", FEBEMESSAGE, str, arg);
    return;
#else   /* FRONTEND */
    return;         /* OK */
#endif  /* FRONTEND */
  }

  /* Private memory. */
  if (arg >= MAKE_DEFS_BE_ONLY_MEMORY && arg <= (ulong)MAKE_DEFS_BE_ONLY_MEMORY + MAKE_DEFS_BE_ONLY_SIZE)
  {
#ifdef FRONTEND
    fprintf(stderr, "%sCHECK_LOCAL_MEMORY: BE private accessed @ %s from %08lx\n", FEBEMESSAGE, str, arg);
    return;
#else   /* FRONTEND */
    return;         /* OK for BE to access its private memory. */
#endif  /* !FRONTEND */
  }

  if (arg >= MAKE_DEFS_FE_ONLY_MEMORY && arg <= (ulong)MAKE_DEFS_FE_ONLY_MEMORY + MAKE_DEFS_FE_ONLY_SIZE)
  {
#ifdef FRONTEND
    return;         /* OK for FE to access its private memory. */
#else   /* FRONTEND */
    fprintf(stderr, "%sCHECK_LOCAL_MEMORY: FE private accessed @ %s from %08lx\n", FEBEMESSAGE, str, arg);
    return;
#endif  /* !FRONTEND */
  }

  /* __fini_array_end is after ro and init data, before .data */
  if (arg >= (ulong)&__fini_array_end && arg <= (ulong)&edata)
  {
    return;         /* OK to access data section. */
  }

  /* bss section is after edata and before end */
  if (arg >= (ulong)&edata && arg <= (ulong)&end)
  {
    return;         /* OK to access bss section. */
  }

/* Make sure load/store not in text area, nor read-only data sections, etc. */
  if (arg >= (ulong)&__executable_start && arg <= (ulong)&__fini_array_end)
  {
    fprintf(stderr, "%sCHECK_LOCAL_MEMORY: failed .text & ro @ %s with/from %08lx\n", FEBEMESSAGE, str, arg);
    abort();
  }

/* Make sure load/store not in before executable area. */
  if (arg <= (ulong)&__executable_start)
  {
    fprintf(stderr, "%sCHECK_LOCAL_MEMORY: failed before .text @ %s with/from %08lx\n", FEBEMESSAGE, str, arg);
    abort();
  }

/* Make sure load/store not after bss area (someone using system malloc!?). */
  if (arg <= (ulong)&end)
  {
    fprintf(stderr, "%sCHECK_LOCAL_MEMORY: failed before .text @ %s with/from %08lx\n", FEBEMESSAGE, str, arg);
    abort();
  }
}   /* End of CHECK_LOCAL_MEMORY */

/* ------------------------------------------------------------------------ */
void check_memory_patterns(const char *str, ulong arg);
void check_memory_patterns(const char *str, ulong arg)
{
  if (
      arg == 0xDAADDAAD || arg == 0xADDAADDA ||     // ILT
      arg == 0xDEEDDEED || arg == 0xEDDEEDDE ||     // VRP
      arg == 0xBEEFBEEF || arg == 0xEFBEEFBE ||     // SCMT
      arg == 0xBAADBAAD || arg == 0xADBAADBA ||     // SCIO
      arg == 0xBADABADA || arg == 0xDABADABA ||     // SCIO's VRP
      arg == 0xFADEFADE || arg == 0xDEFADEFA ||     // QRP
      arg == 0xFEEDFEED || arg == 0xEDFEEDFE ||     // PRP
      arg == 0xBEADBEAD || arg == 0xADBEADBE ||     // RRP
      arg == 0xBADEBADE || arg == 0xDEBADEBA ||     // RPN
      arg == 0xCEEDCEED || arg == 0xEDCEEDCE ||     // RRB
      arg == 0xDAFFDAFF || arg == 0xFFDAFFDA ||     // MLMT
      arg == 0xCAFECAFE || arg == 0xFECAFECA ||     // IRP
      arg == 0xABEDABED || arg == 0xEDABEDAB ||     // VLAR
      arg == 0xACEDACED || arg == 0xEDACEDAC ||     // TMT
      arg == 0xCAFFCAFF || arg == 0xFFCAFFCA ||     // TLMT
      arg == 0xDECDDECD || arg == 0xCDDECDDE ||     // ISMT
      arg == 0xFAFFFAFF || arg == 0xFFFAFFFA ||     // XLI
      arg == 0xBABEBABE || arg == 0xBEBABEBA ||     // R registers not initialized
      arg == 0xDAFEDAFE || arg == 0xFEDAFEDA ||     // RB - rbnode
//      arg == 0xDEADDEAD || arg == 0xADDEADDE ||                               UNUSED
//      arg == 0xBEEDBEED || arg == 0xEDBEEDBE ||                               UNUSED
//      arg == 0xFABAFABA || arg == 0xBAFABAFA ||                               UNUSED
//      arg == 0xBABABABA ||                                                    UNUSED
//      arg == 0xBBBBBBBB ||                                                    UNUSED
//      arg == 0xBCBCBCBC ||                                                    UNUSED
//      arg == 0xBDBDBDBD ||                                                    UNUSED
//      arg == 0xBEBEBEBE ||                                                    UNUSED
//      arg == 0xBFBFBFBF ||                                                    UNUSED
//      arg == 0xCACACACA ||                                                    UNUSED
//      arg == 0xDADADADA ||                                                    UNUSED
      arg == 0xDBDBDBDB ||                          // RB - rbinode
      arg == 0xDCDCDCDC ||                          // Post-private memory pool allocation pattern
      arg == 0xDDDDDDDD ||                          // Pre-private memory pool allocation pattern
      arg == 0xDEDEDEDE ||                          // IMT
      arg == 0xDFDFDFDF ||                          // SCMTE
      arg == 0xEAEAEAEA ||                          // CM
      arg == 0xEBEBEBEB ||                          // COR
      arg == 0xECECECEC ||                          // SCD
      arg == 0xEDEDEDED ||                          // DCD
      arg == 0xEEEEEEEE ||                          // PCB
      arg == 0xEFEFEFEF ||                          // wc_plholder free pattern
      arg == 0xFAFAFAFA ||                          // ILMT
      arg == 0xFBFBFBFB ||                          // VDMT
      arg == 0xFCFCFCFC ||                          // RM
      arg == 0xFDFDFDFD                             // SM
     )
  {
    fprintf(stderr, "%sCHECK_MEMORY_PATTERNS: failed @ %s with %08lx\n", FEBEMESSAGE, str, arg);
#ifdef HISTORY_KEEP
    if (CT_NO_HISTORY == 1) {           /* If not doing take "history", don't */
      return;
    }
    abort();
#endif /* HISTORY_KEEP */
  }
}   /* End of check_memory_patterns */

/* ------------------------------------------------------------------------ */
#ifdef HISTORY_KEEP
/*
 * This routine takes a string (file name and line number) and a variable to
 * print in hex.
 */
void CT_history1(const char *str, int variable)
{
    if (CT_NO_HISTORY == 1) {           /* If not doing take "history", don't */
        return;
    }

    int     len = strlen(str);

    bcopy(str, CT_buf, len);
    CT_buf[len + 0] = ' ';
    CT_buf[len + 1] = '(';
    CT_buf[len + 2] = '0';
    CT_buf[len + 3] = 'x';
    CT_buf[len + 4] = CT_hexdigit[(variable >> 28) & 0xf];  /* 1 */
    CT_buf[len + 5] = CT_hexdigit[(variable >> 24) & 0xf];  /* 2 */
    CT_buf[len + 6] = CT_hexdigit[(variable >> 20) & 0xf];  /* 3 */
    CT_buf[len + 7] = CT_hexdigit[(variable >> 16) & 0xf];  /* 4 */
    CT_buf[len + 8] = CT_hexdigit[(variable >> 12) & 0xf];  /* 5 */
    CT_buf[len + 9] = CT_hexdigit[(variable >> 8) & 0xf];  /* 6 */
    CT_buf[len + 10] = CT_hexdigit[(variable >> 4) & 0xf];  /* 7 */
    CT_buf[len + 11] = CT_hexdigit[variable & 0xf];         /* 8 */
    CT_buf[len + 12] = ')';
    CT_buf[len + 13] = '\n';
    CT_buf[len + 14] = '\0';
    CT_history_always(CT_buf);
}   /* End of CT_history1 */

/* ------------------------------------------------------------------------ */
/*
 * This routine takes a string, memory address, and memory size.
 */
void CT_history_memory_size(const char *str, int memory, int size)
{
    if (CT_NO_HISTORY == 1) {           /* If not doing take "history", don't */
        return;
    }

    char    *fork_name = NULL;

    if (K_xpcb != NULL)
    {
        fork_name = &(K_xpcb->pc_fork_name[0]);
    }
    snprintf(CT_buf, sizeof(CT_buf), "%s with memory 0x%08x size 0x%x (%d) - %s\n",
             str, memory, size, size, fork_name);
    CT_buf[sizeof(CT_buf)-1] = '\0';
    CT_history_always(CT_buf);
}   /* End of CT_history_memory_size */
#endif /* HISTORY_KEEP */

/* ------------------------------------------------------------------------ */
#ifdef HISTORY_KEEP
/* This routine takes a string (file name and line number), and two variables
 * to print in hex. */
void CT_history_store(const char *str, int var1, int var2)
{
#if 0
    if (var2 == 0x6a2fa20c){
      fprintf(stderr, "6a2fa20c %s %08x @ %08x\n", str, var1, var2);
    }
#endif /* 0 */
    if (CT_NO_HISTORY == 1) {           /* If not doing take "history", don't */
        return;
    }

    int     len = strlen(str);

    bcopy(str, CT_buf, len);
    CT_buf[len + 1] = ' ';              /* The null from terminating the string. */
    CT_buf[len + 2] = CT_hexdigit[(var1 >> 28) & 0xf];  /* 1 */
    CT_buf[len + 3] = CT_hexdigit[(var1 >> 24) & 0xf];  /* 2 */
    CT_buf[len + 4] = CT_hexdigit[(var1 >> 20) & 0xf];  /* 3 */
    CT_buf[len + 5] = CT_hexdigit[(var1 >> 16) & 0xf];  /* 4 */
    CT_buf[len + 6] = CT_hexdigit[(var1 >> 12) & 0xf];  /* 5 */
    CT_buf[len + 7] = CT_hexdigit[(var1 >> 8) & 0xf];  /* 6 */
    CT_buf[len + 8] = CT_hexdigit[(var1 >> 4) & 0xf];  /* 7 */
    CT_buf[len + 9] = CT_hexdigit[var1 & 0xf];         /* 8 */
    CT_buf[len + 10] = '@';
    CT_buf[len + 11] = CT_hexdigit[(var2 >> 28) & 0xf];  /* 1 */
    CT_buf[len + 12] = CT_hexdigit[(var2 >> 24) & 0xf];  /* 2 */
    CT_buf[len + 13] = CT_hexdigit[(var2 >> 20) & 0xf];  /* 3 */
    CT_buf[len + 14] = CT_hexdigit[(var2 >> 16) & 0xf];  /* 4 */
    CT_buf[len + 15] = CT_hexdigit[(var2 >> 12) & 0xf];  /* 5 */
    CT_buf[len + 16] = CT_hexdigit[(var2 >> 8) & 0xf];  /* 6 */
    CT_buf[len + 17] = CT_hexdigit[(var2 >> 4) & 0xf];  /* 7 */
    CT_buf[len + 18] = CT_hexdigit[var2 & 0xf];         /* 8 */
    CT_buf[len + 19] = '\n';
    CT_buf[len + 20] = '\0';
    CT_history(CT_buf);
}   /* End of CT_history_store */
#endif /* HISTORY_KEEP */

/* ------------------------------------------------------------------------ */
/* This routine takes a string (file name and line number). */

#ifdef HISTORY_KEEP
void CT_history(const char *buf)
{
#ifdef CT2_DEBUG
check_c_i960_locations_ok();
#endif  /* CT2_DEBUG */
    if (CT_NO_HISTORY == 1) {           /* If not doing take "history", don't */
        return;
    }
    CT_history_always(buf);
}   /* End of CT_history */
#endif /* HISTORY_KEEP */

/* ------------------------------------------------------------------------ */
/* This routine takes a string (file name and line number). */

#ifdef HISTORY_KEEP
static void CT_history_always(const char *buf)
{
    int i;
    int     len = strlen(buf) + 1;      /* Include terminating null. */

#ifndef HISTORY_REGS_KEEP
    for (i = 0; i < CT_PREV_HISTORY_STRINGS; i++)
    {
        if (strcmp(buf, &CT_tracelog[CT_laststrings[CT_prev_history_string[i]]]) == 0)
        {
            CT_count_repeats[CT_prev_history_string[i]]++;
            return;
        }
    }
#endif  /* !HISTORY_REGS_KEEP */
    CT_history_number++;
#ifdef HISTORY_TSC_KEEP
    CT_last_tsc[CT_last_history_string] = get_tsc();
#endif /* HISTORY_TSC_KEEP */
    for (i = CT_PREV_HISTORY_STRINGS-1; i > 0; i--)
    {
        CT_prev_history_string[i] = CT_prev_history_string[i-1];
    }
    CT_prev_history_string[0] = CT_last_history_string;
    CT_count_repeats[CT_last_history_string] = 0;
/* If won't fit at end of buffer, zero out the end of the buffer and start over again. */
    int start = CT_laststrings[CT_last_history_string];
    if ((start + len) > MAX_CT_LOG)
    {
        for (i = start; i < MAX_CT_LOG; i++)
        {
            CT_tracelog[i] = '\0';
        }
        start = 0;
    }
    bcopy(buf, &CT_tracelog[start], len);
#ifdef HISTORY_REGS_KEEP
    for (i = 0; i < 16; i++)
    {
      CT_save_g[CT_last_history_string][i] = greg[i];
      if (rreg != NULL)
      {
        CT_save_r[CT_last_history_string][i] = rreg[i];
      }
      else
      {
        CT_save_r[CT_last_history_string][i] = 0;
      }
    }
#endif  /* HISTORY_REGS_KEEP */
    CT_last_history_string++;
    if (CT_last_history_string >= CT_TL_strings)
    {
        CT_last_history_string = 0;
    }
    CT_laststrings[CT_last_history_string] = start + len;    /* put next one here. */
}   /* End of CT_history_always */
#endif /* HISTORY_KEEP */

/* ------------------------------------------------------------------------ */
#ifdef HISTORY_KEEP
/* This routine takes a string (file name and line number), and a pcb address. */
void CT_history_pcb(const char *str, UINT32 pcb)
{
    PCB *p = (PCB *)pcb;

    if (CT_NO_HISTORY == 1)                 /* If not doing take "history", don't */
    {
        return;
    }
    if (p != NULL)
    {
        snprintf(CT_buf, sizeof(CT_buf), "%s %p - %s from 0x%2.2x\n", str, p, p->pc_fork_name, p->pc_stat);
    }
    else
    {
        snprintf(CT_buf, sizeof(CT_buf), "%s %p\n", str, p);
    }
    CT_buf[sizeof(CT_buf) - 1] = '\0';
    CT_history_always(CT_buf);
}   /* End of CT_history_pcb */
#endif /* HISTORY_KEEP */

/* ------------------------------------------------------------------------ */
#ifdef HISTORY_KEEP
/* This routine takes a string (file name and line number) and a task name. */
void CT_history_task_name(const char *str, const char *fork_name, PCB *pcb)
{
    snprintf(CT_buf, sizeof(CT_buf), "%s %s/%p\n", str, fork_name, pcb);
    CT_buf[sizeof(CT_buf)-1] = '\0';
    CT_history_always(CT_buf);
}   /* End of CT_history_task_name */
#endif /* HISTORY_KEEP */

/* ------------------------------------------------------------------------ */
#if defined(HISTORY_KEEP)
/* This routine takes a format. */
void CT_history_printf(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vsnprintf(CT_buf, sizeof(CT_buf), format, args);
    va_end(args);
    CT_buf[sizeof(CT_buf)-1] = '\0';
    CT_history_always(CT_buf);
}   /* End of CT_history_printf */
#endif /* HISTORY_KEEP */

/* ------------------------------------------------------------------------ */
#if 0
NORETURN
void CT_print_history(void);

/* Call from within gdb, or similar. */

NORETURN
void CT_print_history(void)
{
  /* Start at CT_laststrings[0] but do not use any more of that structure for printing the information. */
  int i;
  int j;

  for (i = 1; i < CT_TL_strings; i++)
  {
    j = (CT_last_history_string - i + CT_TL_strings) % CT_TL_strings;
    if (CT_tracelog[CT_laststrings[j]] != '\0')
    {
#ifdef HISTORY_TSC_KEEP
      fprintf(stderr, "%4d -- %lld: %s",
                      CT_count_repeats[j]+1,
                      CT_last_tsc[j],
                      &CT_tracelog[CT_laststrings[j]]);
#else  /* HISTORY_TSC_KEEP */
      fprintf(stderr, "%4d -- %s", CT_count_repeats[j]+1, &CT_tracelog[CT_laststrings[j]]);
#endif /* HISTORY_TSC_KEEP */
    }
    else
    {
      fprintf(stderr, "ERROR string %d is null?\n", j);
    }
  }
  fprintf(stderr, "Done with list\n");
  sleep(30);
  abort();
}   /* End of CT_print_history */
#endif /* 0 */

#ifdef HISTORY_KEEP
/* ------------------------------------------------------------------------ */
/*
 * Turn history off. If already off, keep off, and increment off counter.
 */
static int history_off_counter = 0;

void CT_HISTORY_OFF(void)
{
    if (CT_NO_HISTORY == 1)
    {
        history_off_counter++;
        return;
    }
    CT_NO_HISTORY = 1;
    history_off_counter = 0;            /* No nesting. */
}   /* End of CT_HISTORY_OFF */

/* ------------------------------------------------------------------------ */
/*
 * Turn history on. If history off counter positive, decrement until zero.
 */

void CT_HISTORY_ON(void)
{
    if (CT_NO_HISTORY == 0)
    {
        return;                     /* Already on. */
    }
    if (history_off_counter > 1)
    {
        history_off_counter--;
        return;
    }
    history_off_counter = 0;
    CT_NO_HISTORY = 0;
}   /* End of CT_HISTORY_ON */

/* ------------------------------------------------------------------------ */
/*
 * Turn history off, no questions.
 */
void CT_HISTORY_OFF_NOW(void)
{
    if (CT_NO_HISTORY == 1)
    {
        return;
    }
    CT_NO_HISTORY = 1;
    history_off_counter = 0;            /* No nesting. */
}   /* End of CT_HISTORY_OFF */

/* ------------------------------------------------------------------------ */
/*
 * This routine always turns history on or off.
 */
void CT_history_disable_task(const char *str)
{
    int     array_dimension = sizeof(disable_task_names) / sizeof(char *);
    int     i;

    history_off_counter = 0;            /* No nesting. */
    CT_NO_HISTORY = 0;                  /* Assume on. */
    for (i = 0; i < array_dimension; i++)
    {
        if (strcmp(str, disable_task_names[i]) == 0)
        {
            CT_NO_HISTORY = 1;          /* TURN HISTORY COLLECTION OFF */
            return;
        }
    }
}   /* End of CT_history_disable_task */

#endif /* HISTORY_KEEP */

/* ------------------------------------------------------------------------ */
/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
/* End of file CT_history.c */
