#ifdef HISTORY_KEEP

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>

#include "XIO_Std.h"
#include "CT_history.h"
#include "mutex.h"
#define FEBEMESSAGE "CCB "
unsigned int CT_NO_HISTORY = 0;

#include "kernel.h"

extern pthread_key_t threadKeys;
/* ------------------------------------------------------------------------ */
#define MAX_CT_LOG    64000000
#define CT_TL_strings   800000          /* This number is also in .gdbinit tracelog macros. */
// #define MAX_CT_LOG   128000000
// #define CT_TL_strings  1600000          /* This number is also in .gdbinit tracelog macros. */
/* ------------------------------------------------------------------------ */
#define get_tsc()       ({ unsigned long long __scr; \
        __asm__ __volatile__("rdtsc" : "=A" (__scr)); __scr;})
/* ------------------------------------------------------------------------ */
static char CT_tracelog[MAX_CT_LOG];
static int CT_last_history_string;          /* Last string. */
static int CT_laststrings[CT_TL_strings];
static void *CT_last_functionPtr[CT_TL_strings];
static int CT_history_number = 0;
#ifdef HISTORY_TSC_KEEP
static unsigned long long CT_last_tsc[CT_TL_strings];
#endif /* HISTORY_TSC_KEEP */
static char CT_buf0[1024];              /* CT_history1 needs an area to work with. */
static char CT_buf1[1024];              /* CT_history_memory_size needs an area. */
static char CT_buf2[1024];              /* CT_history_store needs an area. */
static char CT_buf3[1024];              /* CT_history_pcb needs an area. */
static char CT_buf4[1024];              /* CT_history_task_name needs an area. */
static char CT_buf5[1024];              /* CT_history_printf needs an area. */
static char CT_hexdigit[16] = "0123456789abcdef";

/* ------------------------------------------------------------------------ */
pthread_mutex_t HistoryMutex = PTHREAD_MUTEX_INITIALIZER;
#define LOCKMUTEX   if (pthread_mutex_trylock(&HistoryMutex) != 0) {        \
                        pthread_mutex_lock(&HistoryMutex);                  \
                    }
#define UNLOCKMUTEX pthread_mutex_unlock(&HistoryMutex)

/* ------------------------------------------------------------------------ */
static void CT_history_always(const char *buf);

/* ------------------------------------------------------------------------ */
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

    LOCKMUTEX;

    bcopy(str, CT_buf0, len);
    CT_buf0[len + 0] = ' ';
    CT_buf0[len + 1] = '(';
    CT_buf0[len + 2] = '0';
    CT_buf0[len + 3] = 'x';
    CT_buf0[len + 4] = CT_hexdigit[(variable >> 28) & 0xf];  /* 1 */
    CT_buf0[len + 5] = CT_hexdigit[(variable >> 24) & 0xf];  /* 2 */
    CT_buf0[len + 6] = CT_hexdigit[(variable >> 20) & 0xf];  /* 3 */
    CT_buf0[len + 7] = CT_hexdigit[(variable >> 16) & 0xf];  /* 4 */
    CT_buf0[len + 8] = CT_hexdigit[(variable >> 12) & 0xf];  /* 5 */
    CT_buf0[len + 9] = CT_hexdigit[(variable >> 8) & 0xf];   /* 6 */
    CT_buf0[len + 10] = CT_hexdigit[(variable >> 4) & 0xf];  /* 7 */
    CT_buf0[len + 11] = CT_hexdigit[variable & 0xf];         /* 8 */
    CT_buf0[len + 12] = ')';
    CT_buf0[len + 13] = '\n';
    CT_buf0[len + 14] = '\0';
    CT_history_always(CT_buf0);
    UNLOCKMUTEX;
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
    PCB *pTask = (PCB *)pthread_getspecific(threadKeys);

    if (pTask != NULL)
    {
        fork_name = &(pTask->pc_fork_name[0]);
    }
    LOCKMUTEX;
    snprintf(CT_buf1, sizeof(CT_buf1), "%s with memory 0x%08x size 0x%x (%d) - %s\n",
             str, memory, size, size, fork_name);
    CT_buf1[sizeof(CT_buf1)-1] = '\0';
    CT_history_always(CT_buf1);
    UNLOCKMUTEX;
}   /* End of CT_history_memory_size */

/* ------------------------------------------------------------------------ */
/* This routine takes a string (file name and line number), and two variables
 * to print in hex. */
void CT_history_store(const char *str, int var1, int var2)
{
    if (CT_NO_HISTORY == 1) {           /* If not doing take "history", don't */
        return;
    }

    int     len = strlen(str);

    LOCKMUTEX;
    bcopy(str, CT_buf2, len);
    CT_buf2[len + 1] = ' ';              /* The null from terminating the string. */
    CT_buf2[len + 2] = CT_hexdigit[(var1 >> 28) & 0xf];  /* 1 */
    CT_buf2[len + 3] = CT_hexdigit[(var1 >> 24) & 0xf];  /* 2 */
    CT_buf2[len + 4] = CT_hexdigit[(var1 >> 20) & 0xf];  /* 3 */
    CT_buf2[len + 5] = CT_hexdigit[(var1 >> 16) & 0xf];  /* 4 */
    CT_buf2[len + 6] = CT_hexdigit[(var1 >> 12) & 0xf];  /* 5 */
    CT_buf2[len + 7] = CT_hexdigit[(var1 >> 8) & 0xf];  /* 6 */
    CT_buf2[len + 8] = CT_hexdigit[(var1 >> 4) & 0xf];  /* 7 */
    CT_buf2[len + 9] = CT_hexdigit[var1 & 0xf];         /* 8 */
    CT_buf2[len + 10] = '@';
    CT_buf2[len + 11] = CT_hexdigit[(var2 >> 28) & 0xf];  /* 1 */
    CT_buf2[len + 12] = CT_hexdigit[(var2 >> 24) & 0xf];  /* 2 */
    CT_buf2[len + 13] = CT_hexdigit[(var2 >> 20) & 0xf];  /* 3 */
    CT_buf2[len + 14] = CT_hexdigit[(var2 >> 16) & 0xf];  /* 4 */
    CT_buf2[len + 15] = CT_hexdigit[(var2 >> 12) & 0xf];  /* 5 */
    CT_buf2[len + 16] = CT_hexdigit[(var2 >> 8) & 0xf];  /* 6 */
    CT_buf2[len + 17] = CT_hexdigit[(var2 >> 4) & 0xf];  /* 7 */
    CT_buf2[len + 18] = CT_hexdigit[var2 & 0xf];         /* 8 */
    CT_buf2[len + 19] = '\n';
    CT_buf2[len + 20] = '\0';
    CT_history(CT_buf2);
    UNLOCKMUTEX;
}   /* End of CT_history_store */

/* ------------------------------------------------------------------------ */
/* This routine takes a string. */

void CT_history(const char *buf)
{
    if (CT_NO_HISTORY == 1) {           /* If not doing take "history", don't */
        return;
    }
    LOCKMUTEX;
    CT_history_always(buf);
    UNLOCKMUTEX;
}   /* End of CT_history */

/* ------------------------------------------------------------------------ */
/* This routine takes a string. */

static void CT_history_always(const char *buf)
{
    int i;
    int     len = strlen(buf) + 1;      /* Include terminating null. */

    CT_history_number++;
#ifdef HISTORY_TSC_KEEP
    CT_last_tsc[CT_last_history_string] = get_tsc();
#endif /* HISTORY_TSC_KEEP */
    CT_last_functionPtr[CT_last_history_string] = (void *)pthread_getspecific(threadKeys);
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
    CT_last_history_string++;
    if (CT_last_history_string >= CT_TL_strings)
    {
        CT_last_history_string = 0;
    }
    CT_laststrings[CT_last_history_string] = start + len;    /* put next one here. */
}   /* End of CT_history_always */

/* ------------------------------------------------------------------------ */
/* This routine takes a string, and a pcb address. */
void CT_history_pcb(const char *str, UINT32 pcb)
{
    PCB *p = (PCB *)pcb;

    if (CT_NO_HISTORY == 1)                 /* If not doing take "history", don't */
    {
        return;
    }

    LOCKMUTEX;
    if (p != NULL)
    {
        snprintf(CT_buf3, sizeof(CT_buf3), "%s %p - %s from 0x%2.2x\n", str, p, p->pc_fork_name, p->pc_stat);
    }
    else
    {
        snprintf(CT_buf3, sizeof(CT_buf3), "%s %p\n", str, p);
    }
    CT_buf3[sizeof(CT_buf3) - 1] = '\0';
    CT_history_always(CT_buf3);
    UNLOCKMUTEX;
}   /* End of CT_history_pcb */

/* ------------------------------------------------------------------------ */
/* This routine takes a string, a task name, and PCB. */
void CT_history_task_name(const char *str, const char *fork_name, PCB *pcb)
{
    LOCKMUTEX;
    snprintf(CT_buf4, sizeof(CT_buf4), "%s %s/%p\n", str, fork_name, pcb);
    CT_buf4[sizeof(CT_buf4)-1] = '\0';
    CT_history_always(CT_buf4);
    UNLOCKMUTEX;
}   /* End of CT_history_task_name */

/* ------------------------------------------------------------------------ */
/* This routine takes a format. */
void CT_history_printf(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    LOCKMUTEX;
    vsnprintf(CT_buf5, sizeof(CT_buf5), format, args);
    va_end(args);
    CT_buf5[sizeof(CT_buf5)-1] = '\0';
    CT_history_always(CT_buf5);
    UNLOCKMUTEX;
}   /* End of CT_history_printf */


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
#endif /* HISTORY_KEEP */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
/* End of file CT_history.c */
