/* $Id: CT_defines.h 143007 2010-06-22 14:48:58Z m4 $ */
/* Define Data Types */

#ifndef _CT_defines_h_
/* Only do this once. */
#define _CT_defines_h_


#include <sys/types.h>
#include <stdarg.h>

#define long64 long long
#define ulong64 unsigned long long
#define uchar  unsigned char
#define byte   unsigned char
#define schar  signed char

/* ------------------------------------------------------------------------ */
extern ulong  greg[16];
extern ulong* rreg;             /* Cannot be accessed in -O2 programs! */
/* ------------------------------------------------------------------------ */
extern ulong CT_start;          /* defined in fe.ld and be.ld linker scripts. */
extern ulong CT_end;            /* defined in fe.ld and be.ld linker scripts. */
/* ------------------------------------------------------------------------ */
/* #include <time.h> */               /* For "time()" function. */
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include "xio3d.h"
#include "XIO_Macros.h"
#include "LL_LinuxLinkLayer.h"
#include "queue_control.h"
#include <errno.h>
#include "pcb.h"
#include "memory.h"

#ifdef BACKEND
  #include "sdd.h"
#endif

struct ILT;
struct PRP;
struct PSD;
struct SGL;
struct VRP;
struct VDD;
struct RDD;

/* ------------------------------------------------------------------------ */
#if 0
/* These are created in CT_analyze, where it creates the program that is entered. */
/* Define registers for debugger -- preserve order below */
unsigned long  greg[15] __attribute__((section(".data"), aligned(4)));
unsigned long* rreg     __attribute__((section(".data"), aligned(4)));
/* Preserve order above */
#endif

/* ------------------------------------------------------------------------ */
#define g0      greg[0]
#define g1      greg[1]
#define g2      greg[2]
#define g3      greg[3]
#define g4      greg[4]
#define g5      greg[5]
#define g6      greg[6]
#define g7      greg[7]
#define g8      greg[8]
#define g9      greg[9]
#define g10     greg[10]
#define g11     greg[11]
#define g12     greg[12]
#define g13     greg[13]
#define g14     greg[14]
#define g15     greg[15]
#define fp      g15
__asm__(".equ g,greg");                         /* Make gdb "print /x g@16" print nicely. */
__asm__(".equ g0,greg+0"); __asm__(".global g0");
__asm__(".equ g1,greg+4"); __asm__(".global g1");
__asm__(".equ g2,greg+8"); __asm__(".global g2");
__asm__(".equ g3,greg+12"); __asm__(".global g3");
__asm__(".equ g4,greg+16"); __asm__(".global g4");
__asm__(".equ g5,greg+20"); __asm__(".global g5");
__asm__(".equ g6,greg+24"); __asm__(".global g6");
__asm__(".equ g7,greg+28"); __asm__(".global g7");
__asm__(".equ g8,greg+32"); __asm__(".global g8");
__asm__(".equ g9,greg+36"); __asm__(".global g9");
__asm__(".equ g10,greg+40"); __asm__(".global g10");
__asm__(".equ g11,greg+44"); __asm__(".global g11");
__asm__(".equ g12,greg+48"); __asm__(".global g12");
__asm__(".equ g13,greg+52"); __asm__(".global g13");
__asm__(".equ g14,greg+56"); __asm__(".global g14");
__asm__(".equ g15,greg+60"); __asm__(".global g15");
__asm__(".equ fp,greg+60"); __asm__(".global fp");

/* Define Registers */
__asm__(".equ r,rreg"); __asm__(".global r");   /* Make gdb "x/16 r" print nicely. */
#define        r0      ((ulong*)greg[15])[0]
#define        r1      ((ulong*)greg[15])[1]
#define        r2      ((ulong*)greg[15])[2]
#define        r3      ((ulong*)greg[15])[3]
#define        r4      ((ulong*)greg[15])[4]
#define        r5      ((ulong*)greg[15])[5]
#define        r6      ((ulong*)greg[15])[6]
#define        r7      ((ulong*)greg[15])[7]
#define        r8      ((ulong*)greg[15])[8]
#define        r9      ((ulong*)greg[15])[9]
#define        r10     ((ulong*)greg[15])[10]
#define        r11     ((ulong*)greg[15])[11]
#define        r12     ((ulong*)greg[15])[12]
#define        r13     ((ulong*)greg[15])[13]
#define        r14     ((ulong*)greg[15])[14]
#define        r15     ((ulong*)greg[15])[15]

#define pfp     r0
#define sp      r1
#define rip     r2

/* ------------------------------------------------------------------------ */
/* Hint to compiler on branch methodology. */
/* #define likely(x)    __builtin_expect((x),1) */
/* #define unlikely(x)  __builtin_expect((x),0) */
#define likely(x)       x
#define unlikely(x)     x

/* ------------------------------------------------------------------------ */
/* Generated calls for special processing. */
/* cmp_tmp0, cmp_tmp1, 0 == | 1 != | 2 < | 3 <= | 4 > | 5 >= */
extern void i960_fault(const ulong,const ulong,const ulong, const char *, const int);
/* Generate fault, string passed says what it is. */
extern void i960_generate_fault(const char *, const char *, int);
extern void i960_intctl(ulong,ulong *);
extern void i960_intdis(void);
extern void i960_inten(void);
extern void i960_dcctl(ulong,ulong,ulong *);
extern void i960_flushreg(void);
extern void i960_icctl(ulong,ulong,ulong *);
extern void i960_sysctl(ulong,ulong,ulong *);
extern void i960_modpc(ulong,ulong,ulong *);

#ifdef GCOV
extern void SIGNAL_HUP(int);
#endif /* GCOV */

/* ------------------------------------------------------------------------ */
/* Free running bus clock timer. */
#define get_tsc()       ({ unsigned long long __scr; \
        __asm__ __volatile__("rdtsc" : "=A" (__scr)); __scr;})
/* get lower 32 bits of tsc register */
#define get_tsc_l()     (get_tsc() & 0xffffffff)

/* Following for "c" stack exchange on i386.     CXGH */
#define get_ebp() ({ulong _s_;__asm__ __volatile__("movl %%ebp,%0" : "=r" (_s_)); _s_;})
#define get_esp() ({ulong _s_;__asm__ __volatile__("movl %%esp,%0" : "=r" (_s_)); _s_;})

#define set_esp_ebp(_esp_,_ebp_)                        \
        __asm__ __volatile__(                           \
                "movl %0,%%esp;"                        \
                "movl %1,%%ebp"                         \
                :                                       \
                : "r"(_esp_), "r"(_ebp_)                \
                :"memory", "ebx", "esi", "edi");


/* ------------------------------------------------------------------------ */
static __inline__ void CT_atomic_add(unsigned long *v, int i)
{
    __asm__ __volatile__( "lock ; addl %1,%0" : "=m" (*v) : "ir" (i), "m" (*v));
}

static __inline__ void CT_atomic_xchg(unsigned long *addr, unsigned long *dst)
{
    __asm__ __volatile__( "lock; xchgl  %2,%1"
        : "=a" (*dst), "=m" (*addr) : "r" (*dst), "m" (addr) : "memory");
}

/* ------------------------------------------------------------------------ */
/* These variables are defined in CT_variables.h */
extern unsigned long CT_fork_tmp;               /* temp for fork process name */
extern ulong CT_main_esp;                       /* main program current stack location */
extern ulong CT_main_ebp;                       /* main program current frame location */
extern ulong CT_start_esp;                      /* K$start stack location */
extern ulong CT_start_ebp;                      /* K$start frame pointer */
extern ulong CT_stack_copy_size;                /* size of stack. */
extern ulong CT_ebp_diff;                       /* where is ebp pointing */

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
#ifdef GCOV
/* extern int CT_ioctl(int, unsigned long, void *); */
extern int CT_write(int, void *, unsigned int);
extern int CT_read(int, void *, unsigned int);
extern int CT_ioctl(int, unsigned long, unsigned int);
#else
#define CT_ioctl ioctl
#define CT_read read
#define CT_write write
#endif

#ifdef HISTORY_KEEP
extern unsigned int CT_NO_HISTORY;
#endif
/* ------------------------------------------------------------------------ */
/* These extern's for the i960 code translator running on front and back. */
extern void SIGNAL_ERRTRAP(int, UINT32, siginfo_t *, UINT32);
extern void SIGNAL_ALRM(int);
extern ulong sigUsr1Stack;
extern unsigned long    current_pcb_ptr;
extern ulong ct_cpu_speed;
extern UINT8 L_CalcIdle(void);
extern int LLMyEvent;
extern  ulong errGreg[16];
extern QUEUE_CONTROL LINK_QCS;
extern ulong _init;
extern ulong _end;
extern ulong errLinuxSignal;
extern ulong errTrapAddr;

/* These are for checking if an address contains memory behind it. */
extern ulong __bss_start;
extern ulong __executable_start;
extern ulong etext;
extern ulong edata;
extern ulong __preinit_array_start;
extern UINT32 pci_dev_micro_memory;

extern struct xio3d_drvinfo *ptr_xio3d_drvinfo;
extern int  CT_xiofd;

#ifdef CT2_DEBUG
extern void checkstackmemory(const char *, UINT32);
extern void initstackmemory (UINT32);
extern void check_c_i960_locations_ok(void);
extern void check_pcb_chain(void);
extern unsigned int CT_NOCHECK_STACK;
#endif  /* CT2_DEBUG */

/* ------------------------------------------------------------------------ */
/* I have a problem with nested definitions if I include "misc.h". Specifically,
   /f/m4/b/Proc/src/be_proto:CA void MSC_QueHBeat(void*);
   /f/m4/b/Proc/src/fe_proto:CA void MSC_QueHBeat(void*);
   /f/m4/b/Proc/inc/misc.h:void MSC_QueHBeat(ILT* pILT);
   cause conflicts in compilation. */
/* ------------------------------------------------------------------------ */
#ifdef FRONTEND
#define FEBEMESSAGE "FE "
#else
#define FEBEMESSAGE "BE "
#endif
/* ------------------------------------------------------------------------ */
extern void s_Free_and_zero(void *, UINT32, const char *, unsigned int);
extern void p_Free_and_zero(void *, UINT32, const char *, unsigned int);
extern void k_init_mem(struct fmm *, void *, UINT32);

/* ------------------------------------------------------------------------ */
/* Routines converted from assembler to "c", not defined in header files. */
extern void r_blklock(UINT16, UINT64, UINT64);
extern void r_blkunlock(UINT16);
extern void r_bldprp(struct PRP *, struct PSD *, UINT64, UINT32, UINT16, UINT8);
extern void r_setrrpec(UINT8 error_code, struct ILT *ilt, struct PRP *prp);
extern UINT32 Raid10BusyCorruptionCheck(struct PRP * prp, UINT32 errcode,UINT32 flags);
extern void r_setrrpec_r0rstd(UINT8 error_code, struct ILT *ilt, struct PRP *prp);
extern struct SGL *m_gensgl(UINT32 sectors, UINT32 sgl_offset, struct SGL *sgl);
extern UINT32 v_io_match_cnt(struct VRP *vrp, UINT32 IO_length, struct VDD *vdd);
extern void v_genmrrp(UINT32 strategy, UINT32 sectors, UINT64 rdd_sector_offset, struct RDD *rdd,
                      UINT32 byte_offset, UINT32 remaining_sectors UNUSED, struct VDD *vdd,
                      struct SGL *sgl, struct ILT *ilt);
extern void v_gensrrp(UINT32 strategy, UINT32 sectors UNUSED, UINT64 rdd_sector_offset,
                      struct RDD *rdd, UINT32 remaining_sectors UNUSED,
                      struct VDD *vdd, struct SGL *sgl UNUSED, struct ILT *ilt);
extern void v_gensscrrp(UINT32 strategy, UINT32 sectors UNUSED, UINT64 rdd_sector_offset,
                        struct RDD *rdd, UINT32 remaining_sectors UNUSED,
                        struct VDD *vdd, struct SGL *sgl UNUSED, struct ILT *ilt);
extern struct SGL *PM_MergeSGL(UINT32 *lth_new_sgl, struct SGL *pSGL1, struct SGL *pSGL2);

/* ------------------------------------------------------------------------ */
#endif /* _CT_defines_h_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
