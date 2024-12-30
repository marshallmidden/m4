/* $Id: L_CCBCrashDump.c 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       L_CCBCrashDump.c
**
**  @brief      Provide crash information for CCB.
**
**  This routine provides information about crashes. This avoids the need
**  to have symbolic information in the executables that we ship by avoiding
**  the need to summarize core dumps in the controller. X86 stack-walking
**  code is in Shared/Src.
**
**  Copyright (c) 2005-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "pcb.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include "L_StackDump.h"
#include "L_CCBCrashDump.h"

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/

#define SUMMARY_DIR     "/var/log/xiotech"

#define SUMMARY_SUFFIX ".txt"
#define HISTORY_SUFFIX ".hist"

#define APP_NAME    "ccbrun"
#define TRAP_STR    "CCB TRAPADDR"
#define TRAP_NAME   "errtrapC"

#define SUMMARY_FILE    SUMMARY_DIR "/" APP_NAME SUMMARY_SUFFIX
#define HISTORY_FILE    SUMMARY_DIR "/" APP_NAME HISTORY_SUFFIX

/*
******************************************************************************
** Private variables
******************************************************************************
*/

static FILE *sfp;               /* File pointer for crash log file */

static sigjmp_buf sigrtn;
static int  sigsig;
static unsigned long *sigpc;
static unsigned long *sigaddr;
static struct sigaction sigact;
static struct sigaction oldsegv;
static struct sigaction oldill;
static struct sigaction oldfpe;
static struct sigaction oldbus;
static sigset_t sigmask;
static struct StackDumpArgs sdargs;

/*
******************************************************************************
** Private function typedef prototypes
******************************************************************************
*/

typedef void (*InsuredFunc)(void *);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      crash_catcher - Catch signals caused by bad accesses
**
**              This function catches signals established by the ensure
**              function to avoid prematurely ending a crash dump due to
**              a bad pointer.
**
**  @param      sig - signal generated
**  @param      sinfo - Pointer to siginfo_t structure from signal
**  @param      contxt - Unused pointer
**
**  @return     None
**
******************************************************************************
**/

static NORETURN void crash_catcher(int sig, siginfo_t *sinfo, UNUSED void *contxt)
{
    sigsig = sig;
    sigpc = PC_FROM_SIGINFO(sinfo);
    sigaddr = sinfo->si_addr;
    siglongjmp(sigrtn, 1);
}


/**
******************************************************************************
**
**  @brief      insure - Call a function with crash protection.
**
**              This function calls a function, catching any signals that
**              it could generate from accessing a bad pointer.
**
**  @param      func - pointer to function to call
**  @param      data - pointer to data to pass to function
**  @param      str - string for crash reporting
**
**  @return     None
**
******************************************************************************
**/

static void insure(void (*func)(void *), void *data, const char str[])
{
    sigact.sa_sigaction = &crash_catcher;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = SA_SIGINFO;
    switch (sigsetjmp(sigrtn, 1))
    {
        case 0:
            sigaction(SIGSEGV, &sigact, &oldsegv);
            sigaction(SIGILL, &sigact, &oldill);
            sigaction(SIGFPE, &sigact, &oldfpe);
            sigaction(SIGBUS, &sigact, &oldbus);
            sigemptyset(&sigmask);
            sigaddset(&sigmask, SIGSEGV);
            sigaddset(&sigmask, SIGILL);
            sigaddset(&sigmask, SIGFPE);
            sigaddset(&sigmask, SIGBUS);
            sigprocmask(SIG_UNBLOCK, &sigmask, 0);
            (*func)(data);
            break;

        default:
            fprintf(sfp, "\n!!! %s: failed, signal %d @ %p, addr %p !!!\n",
                    str, sigsig, sigpc, sigaddr);
            break;
    }
    sigaction(SIGSEGV, &oldsegv, 0);
    sigaction(SIGILL, &oldill, 0);
    sigaction(SIGFPE, &oldfpe, 0);
    sigaction(SIGBUS, &oldbus, 0);
}


/**
******************************************************************************
**
**  @brief      forkname - Print forkname from PCB.
**
**              This function prints the forkname from the PCB pointed
**              to by the parameter.
**
**  @param      ptr - Pointer to a PCB as a void *.
**
**  @return     None
**
******************************************************************************
**/

static void forkname(UNUSED void *ptr)
{
    PCB        *pcb = XK_GetPcb();

    fprintf(sfp, "Current task name %.32s\n", pcb->pc_fork_name);
}


/**
******************************************************************************
**
**  @brief      decode_pcb - Decode PCB including stack dumps
**
**              This function prints information from a PCB including
**              both the C and i960 stacks that is contains.
**
**  @param      pcb - Pointer to PCB.
**
**  @return     None
**
******************************************************************************
**/

static void decode_pcb(PCB *pcb)
{
    UINT32      stacksize;
    UINT8      *stack;
    SF         *sf;

    fprintf(sfp, "\nDecode of PCB@%p\n", pcb);
    fprintf(sfp, " pc_thd=%p, pc_global=%d, pc_pri=%d (0x%x), "
            "pc_stat=%d (0x%x)\n", pcb->pc_thd, pcb->pc_global,
            pcb->pc_pri, pcb->pc_pri, pcb->pc_stat, pcb->pc_stat);
    fprintf(sfp, " pc_rsreg=%d (0x%x), pc_time=%d (0x%x), pc_pfp=%p\n",
            pcb->pc_rsreg, pcb->pc_rsreg, pcb->pc_time, pcb->pc_time, pcb->pc_pfp);
    sf = pcb->pc_pfp;
    fprintf(sfp, " pc_fork_name=%.32s\n", pcb->pc_fork_name);
    stack = XK_GetStack(pcb);
    stacksize = XK_GetStackSize(pcb);
    stacksize -= (unsigned int)pcb->pc_esp - (unsigned int)stack;
    fprintf(sfp, " pc_c_stack=%p, for %d (0x%x)\n", stack, stacksize, stacksize);
    fprintf(sfp, " pc_esp=%p\n", (void *)pcb->pc_esp);
    fprintf(sfp, "c backtrace (pc_ebp %p)\n", (void *)pcb->pc_pfp);
    fprintf(sfp, "---status ebp=0x%08x, sp=%p, pc=%p, si_addr=%p\n",
            (UINT32)pcb->pc_pfp, (unsigned long *)pcb->pc_esp, NULL, NULL);
    fprintf(sfp, "---stack from %p to %p\n",
            (unsigned char *)pcb->pc_esp, stack + XK_GetStackSize(pcb));
    L_memdump(sfp, (unsigned char *)pcb->pc_esp, stack + XK_GetStackSize(pcb));
}


/**
******************************************************************************
**
**  @brief      update_pcb - Update PCB pointer safely
**
**              This function advances the pcb pointer to the next PCB.
**
**  @param      ppcb - Pointer to PCB pointer to update.
**
**  @return     None
**
******************************************************************************
**/

static void update_pcb(PCB **ppcb)
{
    PCB        *pcb = *ppcb;

    *ppcb = 0;                  /* Set to zero in case of failure */
    rwbarrier();                /* Memory barrier */
    *ppcb = (PCB *)pcb->pc_thd;
}


/**
******************************************************************************
**
**  @brief      L_CCBCrashDump - Provide crash dump for later analysis.
**
**              This function rotates the appropriate log file and
**              generates a crash log for later analysis.
**
**  @param      sig - Signal causing crash
**  @param      sinfo - Pointer to siginfo_t structure for signal
**  @param      ebp - ebp register value for crash.
**
**  @return     None
**
******************************************************************************
**/

void L_CCBCrashDump(int sig, siginfo_t *sinfo, UINT32 ebp)
{
    time_t      now;

    L_rotate(SUMMARY_FILE, HISTORY_FILE, HISTORY_FILE ".tmp", LOG_PREPEND);
    sfp = fopen(SUMMARY_FILE, "w");
    if (!sfp)
    {
        fprintf(stderr, "Unable to open summary file %s, errno=%d\n",
                SUMMARY_FILE, errno);
        exit(EXIT_FAILURE);
    }
    setlinebuf(sfp);
    now = time(0);
    fprintf(stderr, "%sL_CrashDump called for signal %d at %s\n",
            L_MsgPrefix, sig, ctime(&now));
    fprintf(sfp, "--Signal %d\n** date **\n%s\n** bt **\n", sig, ctime(&now));
    sdargs.sfp = sfp;
    if (sinfo)
    {
        sdargs.pc = PC_FROM_SIGINFO(sinfo);
        sdargs.fsp = SP_FROM_SIGINFO(sinfo);
        sdargs.saddr = sinfo->si_addr;
        if (ebp)
        {
            sdargs.ebp = *(unsigned long *)ebp;
        }
        else
        {
            sdargs.ebp = 0;
        }
    }
    else
    {
        void       *tmp_ptr;    // gcc -Wbad-function-cast won't allow a direct typecast of a function return.

        sdargs.pc = __builtin_return_address(0);
        sdargs.fsp = __builtin_frame_address(0);
        tmp_ptr = __builtin_frame_address(0);
        sdargs.ebp = (UINT32)tmp_ptr;
        sdargs.saddr = 0;
    }
    insure((InsuredFunc)&L_StackDump, &sdargs, "L_StackDump");
    fprintf(sfp, "\n** forkname **\n");
    insure((InsuredFunc)&forkname, 0, "forkname");
    fprintf(sfp, "\n** pcbstate **\n");
    {
        PCB        *pcb = XK_GetPcb();
        int         llimit = 2000;

        do
        {
            insure((InsuredFunc)&decode_pcb, pcb, "decodepcb");
            insure((InsuredFunc)&update_pcb, &pcb, "updatepcb");
        } while (--llimit > 0 && pcb && pcb != XK_GetPcb());
    }
    fprintf(sfp, "\n** Done **\n");
    Fclose(sfp);
}

/* End of L_CCBCrashDump.c */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
