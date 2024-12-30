/* $Id: L_CrashDump.c 157933 2011-09-07 18:08:39Z m4 $ */
/**
******************************************************************************
**
**  @file       L_CrashDump.c
**
**  @brief      Provide crash information.
**
**  This routine provides information about crashes. This avoids the need
**  to have symbolic information in the executables that we ship by avoiding
**  the need to summarize core dumps in the controller. X86 stack-walking
**  code is in Shared/Src.
**
**  Copyright (c) 2005-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "CT_defines.h"
#include "L_Signal.h"
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

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/

#ifdef  STANDALONE
#define SUMMARY_DIR     "."
#else
#define SUMMARY_DIR     "/var/log/xiotech"
#endif /* STANDALONE */

#define SUMMARY_SUFFIX ".txt"
#define HISTORY_SUFFIX ".hist"

#if defined(STANDALONE)
#define APP_NAME    "log"
#define TRAP_STR    "STANDALONE TRAPADDR"
#define TRAP_NAME   "errtrapSA"
#elif defined(FRONTEND)
#define APP_NAME    "Front.t"
#define TRAP_STR    "FE TRAPADDR"
#define TRAP_NAME   "errtrapF"
#elif defined(BACKEND)
#define APP_NAME    "Back.t"
#define TRAP_STR    "BE TRAPADDR"
#define TRAP_NAME   "errtrapB"
#else
#error "Neither FRONTEND nor BACKEND defined!"
#endif

#define SUMMARY_FILE    SUMMARY_DIR "/" APP_NAME SUMMARY_SUFFIX
#define HISTORY_FILE    SUMMARY_DIR "/" APP_NAME HISTORY_SUFFIX

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/

struct ErrtrapArgs
{
    unsigned long   rr;
    const char  *str;
};

/*
******************************************************************************
** Private variables
******************************************************************************
*/

static FILE *sfp;   /* File pointer for crash log file */

static sigjmp_buf   sigrtn;
static int  sigsig;
static unsigned long    *sigpc;
static unsigned long    *sigaddr;
static struct sigaction sigact;
static struct sigaction oldsegv;
static struct sigaction oldill;
static struct sigaction oldfpe;
static struct sigaction oldbus;
static sigset_t sigmask;
static struct StackDumpArgs sdargs;
static struct ErrtrapArgs   etargs;

/*
******************************************************************************
** Public functions - used only in i960 assembler, thus no extern in header file.
******************************************************************************
*/
void L_CrashDump(int sig, siginfo_t *sinfo, UINT32 ebp);
void DumpMemory(UINT32 *, UINT32, UINT32);

/*
******************************************************************************
** Private function prototypes
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
**  @brief      gl - Get long
**
**              Access unsigned long value from address as an unsigned long.
**
**  @param      addr - address in unsigned long form
**
**  @return     Unsigned long value
**
******************************************************************************
**/

static unsigned long    gl(unsigned long addr)
{
    return *(unsigned long *)addr;
}


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
**
**  @param      sinfo - Pointer to siginfo_t structure from signal
**
**  @param      contxt - Unused pointer
**
**  @return     None
**
******************************************************************************
**/

NORETURN
static void crash_catcher(int sig, siginfo_t *sinfo, void *contxt UNUSED)
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
**
**  @param      data - pointer to data to pass to function
**
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
**  @brief      rdump - Dump r-registers.
**
**              This function prints the i960 r-registers, given the
**              address of them.
**
**  @param      rr - address of r-registers as an unsigned long
**
**  @return     None
**
******************************************************************************
**/

static void rdump(unsigned long rr)
{
    unsigned long   *rptr;

    if ((long)rr == -1)
    {
        rr = fp;
    }
    if (rr == 0)
    {
        fprintf(sfp, "fp == NULL, r registers not available\n");
    }
    rptr = (unsigned long *)rr;
    fprintf(sfp, " r0 %08lx %08lx %08lx %08lx   %08lx %08lx %08lx %08lx\n",
        rptr[0], rptr[1], rptr[2], rptr[3],
        rptr[4], rptr[5], rptr[6], rptr[7]);
    fprintf(sfp, " r8 %08lx %08lx %08lx %08lx   %08lx %08lx %08lx %08lx\n",
        rptr[8], rptr[9], rptr[10], rptr[11],
        rptr[12], rptr[13], rptr[14], rptr[15]);
}


/**
******************************************************************************
**
**  @brief      i_stack - Print i960 stack.
**
**              This function prints the i960 stack given the stack
**              pointer and the address of the r-registers.
**
**  @param      isp - address of i960 stack as an unsigned long
**
**  @param      rr - address of r-registers as an unsigned long
**
**  @return     None
**
******************************************************************************
**/

static void i_stack(unsigned long isp, unsigned long rr)
{
    unsigned long   st;
    unsigned long   ct;

    if (isp == 0xffffffff)
    {
        isp = gl(fp + 4);
        rr = fp;
    }
    st = rr + 64;
    ct = 0;
    if (st < isp)
    {
        fprintf(sfp, "-- i960 Stack from %08lx to %08lx\n", st, isp);
    }
    L_memdump(sfp, (void *)st, (void *)isp);
}


/**
******************************************************************************
**
**  @brief      printi960stack - Display i960 stack.
**
**              This function prints the i960 stack with identification
**              of return addresses and r-register sets.
**
**  @param      args - Address of r-registers as a void *.
**
**  @return     None
**
******************************************************************************
**/

static void printi960stack(void *args)
{
    unsigned long   rr;
    unsigned long   Pfp;
    int llimit = 200;

    rr = (unsigned long)args;
    if (rr == 0)
    {
        return;
    }
    rdump(rr);
    Pfp = *(unsigned long *)rr;
    while (--llimit > 0 && Pfp != 0)
    {
        i_stack(gl(rr + 4), rr);
        fprintf(sfp, "routine @ 0x%08lx\n", gl(Pfp + 8));
        rr = Pfp;
        Pfp = gl(Pfp);
        if (rr)
        {
            rdump(rr);
        }
    }
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

static void forkname(void *ptr UNUSED)
{
    PCB *pcb = *(PCB **)current_pcb_ptr;

    fprintf(sfp, "Current task name %.32s\n", pcb->pc_fork_name);
}


/**
******************************************************************************
**
**  @brief      tochar - Return printable character.
**
**              This function returns a printable character for the byte
**              pointed to by the parameter.
**
**  @param      addr - address of the byte as an unsigned long
**
**  @return     Character or '?' if unprintable
**
******************************************************************************
**/

static char tochar(unsigned long addr)
{
    char    ch;

    ch = *(char *)addr;
    if (! isprint(ch))
    {
        ch = '?';
    }
    return ch;
}


/**
******************************************************************************
**
**  @brief      printerrtrapdata - Print error trap information.
**
**              This function prints information about an error trap that
**              took place.
**
**  @param      args - Address of ErrtrapArgs structure.
**
**  @return     None
**
******************************************************************************
**/

static void printerrtrapdata(struct ErrtrapArgs *args)
{
    unsigned long   rr;

    rr = args->rr;
    fprintf(sfp, "%s\n", args->str);
    fprintf(sfp, "error code=%08lx (%lu)  base ATU=%08lx  ",
        gl(rr), gl(rr), gl(rr + 4));
    fprintf(sfp, "Firmware revision=%c%c%c%c  revcount=%c%c%c%c\n",
        tochar(rr + 8), tochar(rr + 9), tochar(rr + 10), tochar(rr + 11),
        tochar(rr + 12), tochar(rr + 13), tochar(rr + 14), tochar(rr + 15));
    fprintf(sfp, "pfp=%08lx sp=%08lx rip=%08lx  r3=%08lx  r4=%08lx  r5=%08lx"
        "  r6=%08lx  r7=%08lx\n", gl(rr + 0x10), gl(rr + 0x14), gl(rr + 0x18),
        gl(rr + 0x1c), gl(rr + 0x20), gl(rr + 0x24), gl(rr + 0x28),
        gl(rr + 0x2c));
    fprintf(sfp, " r8=%08lx r9=%08lx r10=%08lx r11=%08lx r12=%08lx r13=%08lx"
        " r14=%08lx r15=%08lx\n", gl(rr + 0x30), gl(rr + 0x34), gl(rr + 0x38),
        gl(rr + 0x3c), gl(rr + 0x40), gl(rr + 0x44), gl(rr + 0x48),
        gl(rr + 0x4c));
    fprintf(sfp, " g0=%08lx g1=%08lx  g2=%08lx  g3=%08lx  g4=%08lx  g5=%08lx"
        "  g6=%08lx  g7=%08lx\n", gl(rr + 0x50), gl(rr + 0x54), gl(rr + 0x58),
        gl(rr + 0x5c), gl(rr + 0x60), gl(rr + 0x64), gl(rr + 0x68),
        gl(rr + 0x6c));
    fprintf(sfp, " g8=%08lx g9=%08lx g10=%08lx g11=%08lx g12=%08lx g13=%08lx"
        " g14=%08lx g15=%08lx\n", gl(rr + 0x70), gl(rr + 0x74), gl(rr + 0x78),
        gl(rr + 0x7c), gl(rr + 0x80), gl(rr + 0x84), gl(rr + 0x88),
        gl(rr + 0x8c));
    fprintf(sfp, "Last CCB heartbeat: 0x%08lx,%08lx,%08lx\n",
        gl(rr + 0x94), gl(rr + 0x98), gl(rr + 0x9c));
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
    unsigned int    stacksize;

    fprintf(sfp, "\nDecode of PCB@%p\n", pcb);
    fprintf(sfp, " pc_thd=%p, pc_global=%d, pc_pri=%d (0x%x), "
        "pc_stat=%d (0x%x)\n", pcb->pc_thd, pcb->pc_global,
        pcb->pc_pri, pcb->pc_pri, pcb->pc_stat, pcb->pc_stat);
    fprintf(sfp, " pc_rsreg=%d (0x%x), pc_time=%d (0x%x), pc_pfp=%p\n",
        pcb->pc_rsreg, pcb->pc_rsreg, pcb->pc_time, pcb->pc_time,
        pcb->pc_pfp);
    fprintf(sfp, " g0 %08x %08x %08x %08x   %08x %08x %08x %08x\n",
        pcb->pc_gRegs[0], pcb->pc_gRegs[1], pcb->pc_gRegs[2], pcb->pc_gRegs[3],
        pcb->pc_gRegs[4], pcb->pc_gRegs[5], pcb->pc_gRegs[6], pcb->pc_gRegs[7]);
    fprintf(sfp, " g8 %08x %08x %08x %08x   %08x %08x %08x %08x\n",
        pcb->pc_gRegs[8], pcb->pc_gRegs[9], pcb->pc_gRegs[10],
        pcb->pc_gRegs[11], pcb->pc_gRegs[12], pcb->pc_gRegs[13],
        pcb->pc_gRegs[14], pcb->pc_gRegs[15]);
    fprintf(sfp, " pc_fork_name=%.32s\n", pcb->pc_fork_name);
    printi960stack((void *)pcb->pc_pfp);
    stacksize = (unsigned int)&pcb->pc_esp - (unsigned int)&pcb->pc_c_stack;
    fprintf(sfp, " pc_c_stack=%p, for %d (0x%x)\n", pcb->pc_c_stack,
        stacksize, stacksize);
    fprintf(sfp, " pc_esp=%p\n", (void *)pcb->pc_esp);
    if (pcb->pc_CT_fork_tmp_exch != 0)
    {
        fprintf(sfp, " pc_CT_fork_tmp_exch=%d (0x%x)\n",
            pcb->pc_CT_fork_tmp_exch, pcb->pc_CT_fork_tmp_exch);
    }
    fprintf(sfp, "c backtrace (pc_ebp %p)\n", (void *)pcb->pc_ebp);
    sdargs.sfp = sfp;
    sdargs.ebp = pcb->pc_ebp;
    sdargs.pc = 0;
    sdargs.fsp = (unsigned long *)pcb->pc_esp;
    sdargs.saddr = 0;
    L_StackDump(&sdargs);
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
    PCB *pcb = *ppcb;

    *ppcb = 0;      /* Set to zero in case of failure */
    rwbarrier();    /* Memory barrier */
    *ppcb = (PCB *)pcb->pc_thd;
}


/**
******************************************************************************
**
**  @brief      L_CrashDump - Provide crash dump for later analysis.
**
**              This function rotates the appropriate log file and
**              generates a crash log for later analysis.
**
**  @param      sig - Signal causing crash
**
**  @param      sinfo - Pointer to siginfo_t structure for signal
**
**  @param      ebp - ebp register value for crash.
**
**  @return     None
**
******************************************************************************
**/

void L_CrashDump(int sig, siginfo_t *sinfo, UINT32 ebp)
{
    static siginfo_t    nosiginfo;
    time_t  now;

    if (!sinfo)
    {
        sinfo = &nosiginfo;
    }
    L_rotate(SUMMARY_FILE, HISTORY_FILE, HISTORY_FILE ".tmp", LOG_PREPEND);
    sfp = fopen(SUMMARY_FILE, "w");
    if (!sfp)
    {
        fprintf(stderr, "Unable to open summary file %s, errno=%d\n",
            SUMMARY_FILE, errno);
        exit(1);
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
        void *tmp_addr;

        sdargs.pc = __builtin_return_address(0);
        sdargs.fsp = __builtin_frame_address(0);
        tmp_addr = __builtin_frame_address(0);
        sdargs.ebp = (UINT32)tmp_addr;
        sdargs.saddr = 0;
    }
    insure((InsuredFunc)&L_StackDump, &sdargs, "L_StackDump");
    fprintf(sfp, "\n** ibt **\n");
    insure((InsuredFunc)&printi960stack, (void *)fp, "ibt printi960stack");
    fprintf(sfp, "\n** ebt **\n");
    insure((InsuredFunc)&printi960stack, (void *)sigUsr1Stack,
        "ebt printi960stack");
    fprintf(sfp, "\n** forkname **\n");
    insure((InsuredFunc)&forkname, 0, "forkname");
    fprintf(sfp, "\n** " TRAP_NAME " **\n");
    etargs.rr = errTrapAddr;
    etargs.str = TRAP_STR;
    insure((InsuredFunc)&printerrtrapdata, &etargs, TRAP_NAME);
    fprintf(sfp, "\n** r **\n");
    rdump(fp);
    fprintf(sfp, "\n** pcbstate **\n");
    {
        PCB *pcb = *(PCB **)current_pcb_ptr;
        int llimit = 2000;

        do
        {
            insure((InsuredFunc)&decode_pcb, pcb, "decodepcb");
            insure((InsuredFunc)&update_pcb, &pcb, "updatepcb");
        } while (--llimit > 0 && pcb && pcb != *(PCB **)current_pcb_ptr);
    }
    fprintf(sfp, "\n** Done **\n");
    fclose(sfp);
}


#ifdef  STANDALONE

/*****
 * The following code is for standalone testing of the functions in
 * this file. It may no longer work, since changes may have been
 * made since I began testing it integrated with the application. I
 * leave the code here in case it is some hellp in the future.
 */

const char  *L_MsgPrefix = "crashtest: ";
unsigned long   rrr[16] =
    { 0x11111111, 0x12121212, 0x13131313, 0x14141414,
      0x15151515, 0x16161616, 0x17171717, 0x18181818,
      0x19191919, 0x1a1a1a1a, 0x1b1b1b1b, 0x1c1c1c1c,
      0x1d1d1d1d, 0x1e1e1e1e, 0x1f1f1f1f, 0x22222222 };

unsigned long   greg[15] =
    { 0x01010101, 0x02020202, 0x03030303, 0x04040404,
      0x05050505, 0x06060606, 0x07070707, 0x08080808,
      0x09090909, 0x0a0a0a0a, 0x0b0b0b0b, 0x0c0c0c0c,
      0x0d0d0d0d, 0x0e0e0e0e, 0x0f0f0f0f };
unsigned long   *rreg = rrr;

ulong   sigUsr1Stack = 0;
ulong   errTrapAddr = 0;

static PCB  cpcb = { .pc_fork_name = "crashtester" };

unsigned long   current_pcb_ptr = (unsigned long) &cpcb;

NORETURN
static void sig_handler(int sig, siginfo_t *sinfo, void *p)
{
    UINT32 ebp;

    asm("movl %%ebp, %0" : "=r" (ebp) : );
    L_CrashDump(sig, sinfo, ebp);
    exit(1);
}


static void crash(int *v)
{
    fprintf(stderr, "*v=%d\n", *v);
}


int main(int argc, char *argv[])
{
    struct sigaction act;

    fprintf(stderr, "sum=%s, hist=%s\n", SUMMARY_FILE, HISTORY_FILE);
    act.sa_sigaction = &sig_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &act, 0);
    crash(0);
    return 0;
}
#endif /* STANDALONE */

/* ------------------------------------------------------------------------ */
void DumpMemory(UINT32 *ptr, UINT32 length, UINT32 additional)
{
    int start = additional;
    int flag = 0;           /* Flag if we are doing a bunch of zeros */
    int flagstart = -1;

    while (length >= 32)
    {
        if (*ptr == 0 && *(ptr+1) == 0 && *(ptr+2) == 0 && *(ptr+3) == 0 &&
            *(ptr+4) == 0 && *(ptr+5) == 0 && *(ptr+6) == 0 && *(ptr+7) == 0)
        {
            if (flag == 0)
            {
                flag = 1;
                flagstart = start;
            }
        }
        else
        {
            if (flag == 1)
            {
                fprintf(stderr, "%6.6x ... zeros ...\n", flagstart);
                flag = 0;
            }
            fprintf(stderr, "%6.6x %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x\n", start,
                    *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5), *(ptr+6), *(ptr+7));
        }
        length -= 32;
        start += 32;
        ptr += 8;
    }
    if (flag == 1)
    {
        fprintf(stderr, "%6.6x ... zeros ... %6.6x\n", flagstart, start - 1);
        flag = 0;
    }
    if (length > 0)
    {
        UINT32 i;
        char dump_buffer[8 + 9*8 + 2];
        int j;

        j = sprintf(&dump_buffer[0], "%6.6x", start);
        for (i = 0; i < length/4; i++)
        {
            j += sprintf(&dump_buffer[j], " %8.8x", *(ptr + i));
        }
        j += sprintf(&dump_buffer[j], "\n");
        fwrite(&dump_buffer[0], j, 1, stderr);
    }
}   /* End of DumpMemory */

/* ------------------------------------------------------------------------ */

/* End of L_CrashDump.c */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
