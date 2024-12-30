/* $Id: CT_asm_defines.h 161678 2013-09-18 19:25:16Z marshall_midden $ */

#ifndef _CT_asm_defines_h_
/* Only do this once. */
#define _CT_asm_defines_h_


#include <sys/types.h>
#include <stdarg.h>

#define long64 long long
#define ulong64 unsigned long long
#define uchar  unsigned char
#define byte   unsigned char
#define schar  signed char

/* ------------------------------------------------------------------------ */
extern ulong  greg[16];
extern ulong *rreg;             /* Cannot be accessed in -O2 programs! */
/* ------------------------------------------------------------------------ */
extern ulong CT_start;          /* defined in fe.ld and be.ld linker scripts. */
extern ulong CT_end;            /* defined in fe.ld and be.ld linker scripts. */
/* ------------------------------------------------------------------------ */
/* #include <time.h> */               /* For "time()" function. */
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <byteswap.h>
#include <strings.h>
#define __USE_GNU
#include <string.h>

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

/* ------------------------------------------------------------------------ */
#if 0
/* These are created in CT_analyze, where it creates the program that is entered. */
/* Define registers for debugger -- preserve order below */
unsigned long  greg[15] __attribute__((section(".data"), aligned(4)));
unsigned long *rreg     __attribute__((section(".data"), aligned(4)));
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
extern ulong CT_main_esp;                       /* main program current stack location */
extern ulong CT_main_ebp;                       /* main program current frame location */
extern ulong CT_start_esp;                      /* K$start stack location */
extern ulong CT_start_ebp;                      /* K$start frame pointer */
extern ulong CT_stack_copy_size;                /* size of stack. */
extern ulong CT_ebp_diff;                       /* where is ebp pointing */
/* These variables are defined in CT_SHARED.c */
extern UINT32        trap_ebp;
extern siginfo_t    *trap_sinf;

/* ------------------------------------------------------------------------ */
extern struct fmm P_ram;                        /* Private Memory Management Structure. */
extern struct fms P_cur;                        /* Private Memory Statistics Structure. */

/* ------------------------------------------------------------------------ */
#ifdef HISTORY_KEEP
extern void CT_history(const char *);           /* string for trace log. */
extern void CT_history_store(const char *, int, int); /* string, value, @ location for trace log. */
extern void CT_history1(const char *, int);     /* string and argument for trace log. */
extern void CT_history_memory_size(const char *, int, int);
extern void CT_history_task_name(const char *str, const char *fork_name, PCB *pcb);
extern void CT_history_printf(const char *format, ...) __attribute__((__format__(__printf__,1,2)));
extern void CT_history_pcb(const char *str, UINT32 pcb);
extern void check_c_i960_locations_ok(void);
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
extern const char *L_MsgPrefix;
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
extern void checkstackmemory(const char *, unsigned long);
extern void initstackmemory (unsigned long);
extern unsigned int CT_NOCHECK_STACK;
extern void check_pcb_chain(void);
#endif  /* CT2_DEBUG */

/* These are for ispc.c needing PCB set before it runs. */
extern PCB *atiomonpcb[];
extern QU isp_RCVIO_queue[];

/* ------------------------------------------------------------------------ */
/* I have a problem with nested definitions if I include "misc.h". Specifically,
   /f/m4/b/Proc/src/be_proto:CA void MSC_QueHBeat(void *);
   /f/m4/b/Proc/src/fe_proto:CA void MSC_QueHBeat(void *);
   /f/m4/b/Proc/inc/misc.h:void MSC_QueHBeat(ILT *pILT);
   cause conflicts in compilation. */
extern void MSC_LogMessageStack(void *pLogEntry, UINT32 size);
extern void MSC_SoftFault(void *pData);
extern UINT32 MSC_CRC32(void *data, UINT32 length);
#ifdef DEBUG_FLIGHTREC
extern void MSC_FlightRec(UINT32 parm0, UINT32 parm1, UINT32 parm2, UINT32 parm3);
#endif  /* DEBUG_FLIGHTREC */
extern void  L_CrashDump(int, siginfo_t *, UINT32);
/* ------------------------------------------------------------------------ */
extern void create_snapshot_completers(void);
/* ------------------------------------------------------------------------ */
extern UINT32 apool_validate_async_copy(VDD *pSrcVDD, VDD *pDestVDD);
extern void apool_stop(void);
extern void apool_start(void);
extern void InitPR (void);
extern UINT16 *isp_get_iocb(UINT8 port, UINT32 *rqip);
#ifdef FRONTEND
extern void isp_abort_exchange_ilt(UINT16 port, struct ILT *ilt);
#endif
extern void ON_MigrateOldPDDtoNewPdd(PDD *oldPDD,PDD *NewPdd,PDD *GlobalArray[]);
#ifdef BACKEND
extern PCB    *fDiscoveryPcb;
extern PCB    *fPortMonitorPcb;
extern UINT32  FAB_ReValidateConnection(UINT8 port, struct DEV *device);
extern UINT32  FAB_IsDevInUse(struct DEV *device);
extern UINT32  PHY_CheckForRetry(struct ILT *ilt, struct DEV *dev, struct PRP *prp);
#endif
extern UINT32  dlm_cnt_servers(UINT16);
extern UINT32  check_vdisk_XIO_attached(UINT32);
extern UINT32  M_chk4XIO(UINT64);
/* ------------------------------------------------------------------------ */
/* This is copied from mem_pool.h */
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

/* ------------------------------------------------------------------------ */
/* New memory pool allocation methodology and usage. */
#define memory_externs(name)                                            \
    extern void init_##name(int, const char *, const unsigned int);     \
    extern void put_##name(UINT32, const char *, const unsigned int);   \
    extern UINT32 get_##name(const char *, const unsigned int);         \
    extern struct memory_pool pool_##name;

/* NOTE: see files mem_pool.h and mem_pool.c */

#if defined(FRONTEND)
  memory_externs(tmt);
  #define get_tmt() get_tmt(__FILE__, __LINE__);
  #define put_tmt(xxx) put_tmt(xxx, __FILE__, __LINE__);
  #define init_tmt(xxx) init_tmt(xxx, __FILE__, __LINE__);
  memory_externs(tlmt);
  #define get_tlmt() get_tlmt(__FILE__, __LINE__);
  #define put_tlmt(xxx) put_tlmt(xxx, __FILE__, __LINE__);
  #define init_tlmt(xxx) init_tlmt(xxx, __FILE__, __LINE__);
  memory_externs(ismt);
  #define get_ismt() get_ismt(__FILE__, __LINE__);
  #define put_ismt(xxx) put_ismt(xxx, __FILE__, __LINE__);
  #define init_ismt(xxx) init_ismt(xxx, __FILE__, __LINE__);
  memory_externs(xli);
  #define get_xli() get_xli(__FILE__, __LINE__);
  #define put_xli(xxx) put_xli(xxx, __FILE__, __LINE__);
  #define init_xli(xxx) init_xli(xxx, __FILE__, __LINE__);
  memory_externs(ltmt);
  #define get_ltmt() get_ltmt(__FILE__, __LINE__);
  #define put_ltmt(xxx) put_ltmt(xxx, __FILE__, __LINE__);
  #define init_ltmt(xxx) init_ltmt(xxx, __FILE__, __LINE__);
  memory_externs(irp);
  #define get_irp() get_irp(__FILE__, __LINE__);
  #define put_irp(xxx) put_irp(xxx, __FILE__, __LINE__);
  #define init_irp(xxx) init_irp(xxx, __FILE__, __LINE__);
  memory_externs(lsmt);
  #define get_lsmt() get_lsmt(__FILE__, __LINE__);
  #define put_lsmt(xxx) put_lsmt(xxx, __FILE__, __LINE__);
  #define init_lsmt(xxx) init_lsmt(xxx, __FILE__, __LINE__);
  memory_externs(imt);
  #define get_imt() get_imt(__FILE__, __LINE__);
  #define put_imt(xxx) put_imt(xxx, __FILE__, __LINE__);
  #define init_imt(xxx) init_imt(xxx, __FILE__, __LINE__);
  memory_externs(vdmt);
  #define get_vdmt() get_vdmt(__FILE__, __LINE__);
  #define put_vdmt(xxx) put_vdmt(xxx, __FILE__, __LINE__);
  #define init_vdmt(xxx) init_vdmt(xxx, __FILE__, __LINE__);
  memory_externs(ilmt);
  #define get_ilmt() get_ilmt(__FILE__, __LINE__);
  #define put_ilmt(xxx) put_ilmt(xxx, __FILE__, __LINE__);
  #define init_ilmt(xxx) init_ilmt(xxx, __FILE__, __LINE__);
  memory_externs(wc_plholder);
  #define get_wc_plholder() get_wc_plholder(__FILE__, __LINE__);
  #define put_wc_plholder(xxx) put_wc_plholder(xxx, __FILE__, __LINE__);
  #define init_wc_plholder(xxx) init_wc_plholder(xxx, __FILE__, __LINE__);
  memory_externs(wc_rbinode);
  #define get_wc_rbinode() get_wc_rbinode(__FILE__, __LINE__);
  #define put_wc_rbinode(xxx) put_wc_rbinode(xxx, __FILE__, __LINE__);
  #define init_wc_rbinode(xxx) init_wc_rbinode(xxx, __FILE__, __LINE__);
  memory_externs(wc_rbnode);
  #define get_wc_rbnode() get_wc_rbnode(__FILE__, __LINE__);
  #define put_wc_rbnode(xxx) put_wc_rbnode(xxx, __FILE__, __LINE__);
  #define init_wc_rbnode(xxx) init_wc_rbnode(xxx, __FILE__, __LINE__);
#endif  /* FRONTEND */

#if defined(BACKEND)
  memory_externs(scmte);
  #define get_scmte() get_scmte(__FILE__, __LINE__);
  #define put_scmte(xxx) put_scmte(xxx, __FILE__, __LINE__);
  #define init_scmte(xxx) init_scmte(xxx, __FILE__, __LINE__);
  memory_externs(prp);
  #define get_prp() get_prp(__FILE__, __LINE__);
  #define put_prp(xxx) put_prp(xxx, __FILE__, __LINE__);
  #define init_prp(xxx) init_prp(xxx, __FILE__, __LINE__);
  memory_externs(rrp);
  #define get_rrp() get_rrp(__FILE__, __LINE__);
  #define put_rrp(xxx) put_rrp(xxx, __FILE__, __LINE__);
  #define init_rrp(xxx) init_rrp(xxx, __FILE__, __LINE__);
  memory_externs(rpn);
  #define get_rpn() get_rpn(__FILE__, __LINE__);
  #define put_rpn(xxx) put_rpn(xxx, __FILE__, __LINE__);
  #define init_rpn(xxx) init_rpn(xxx, __FILE__, __LINE__);
  memory_externs(rrb);
  #define get_rrb() get_rrb(__FILE__, __LINE__);
  #define put_rrb(xxx) put_rrb(xxx, __FILE__, __LINE__);
  #define init_rrb(xxx) init_rrb(xxx, __FILE__, __LINE__);
  memory_externs(vlar);
  #define get_vlar() get_vlar(__FILE__, __LINE__);
  #define put_vlar(xxx) put_vlar(xxx, __FILE__, __LINE__);
  #define init_vlar(xxx) init_vlar(xxx, __FILE__, __LINE__);
  memory_externs(rm);
  #define get_rm() get_rm(__FILE__, __LINE__);
  #define put_rm(xxx) put_rm(xxx, __FILE__, __LINE__);
  #define init_rm(xxx) init_rm(xxx, __FILE__, __LINE__);
  memory_externs(sm);
  #define get_sm() get_sm(__FILE__, __LINE__);
  #define put_sm(xxx) put_sm(xxx, __FILE__, __LINE__);
  #define init_sm(xxx) init_sm(xxx, __FILE__, __LINE__);
  memory_externs(cm);
  #define get_cm() get_cm(__FILE__, __LINE__);
  #define put_cm(xxx) put_cm(xxx, __FILE__, __LINE__);
  #define init_cm(xxx) init_cm(xxx, __FILE__, __LINE__);
  memory_externs(cor);
  #define get_cor() get_cor(__FILE__, __LINE__);
  #define put_cor(xxx) put_cor(xxx, __FILE__, __LINE__);
  #define init_cor(xxx) init_cor(xxx, __FILE__, __LINE__);
  memory_externs(scd);
  #define get_scd() get_scd(__FILE__, __LINE__);
  #define put_scd(xxx) put_scd(xxx, __FILE__, __LINE__);
  #define init_scd(xxx) init_scd(xxx, __FILE__, __LINE__);
  memory_externs(dcd);
  #define get_dcd() get_dcd(__FILE__, __LINE__);
  #define put_dcd(xxx) put_dcd(xxx, __FILE__, __LINE__);
  #define init_dcd(xxx) init_dcd(xxx, __FILE__, __LINE__);
  memory_externs(tpmt);
  #define get_tpmt() get_tpmt(__FILE__, __LINE__);
  #define put_tpmt(xxx) put_tpmt(xxx, __FILE__, __LINE__);
  #define init_tpmt(xxx) init_tpmt(xxx, __FILE__, __LINE__);
  memory_externs(scmt);
  #define get_scmt() get_scmt(__FILE__, __LINE__);
  #define put_scmt(xxx) put_scmt(xxx, __FILE__, __LINE__);
  #define init_scmt(xxx) init_scmt(xxx, __FILE__, __LINE__);
  memory_externs(scio);
  #define get_scio() get_scio(__FILE__, __LINE__);
  #define put_scio(xxx) put_scio(xxx, __FILE__, __LINE__);
  #define init_scio(xxx) init_scio(xxx, __FILE__, __LINE__);
#endif  /* BACKEND */

memory_externs(qrp);
#define get_qrp() get_qrp(__FILE__, __LINE__);
#define put_qrp(xxx) put_qrp(xxx, __FILE__, __LINE__);
#define init_qrp(xxx) init_qrp(xxx, __FILE__, __LINE__);
memory_externs(dtmt);
#define get_dtmt() get_dtmt(__FILE__, __LINE__);
#define put_dtmt(xxx) put_dtmt(xxx, __FILE__, __LINE__);
#define init_dtmt(xxx) init_dtmt(xxx, __FILE__, __LINE__);
memory_externs(mlmt);
#define get_mlmt() get_mlmt(__FILE__, __LINE__);
#define put_mlmt(xxx) put_mlmt(xxx, __FILE__, __LINE__);
#define init_mlmt(xxx) init_mlmt(xxx, __FILE__, __LINE__);
memory_externs(pcb);
#define get_pcb() get_pcb(__FILE__, __LINE__);
#define put_pcb(xxx) put_pcb(xxx, __FILE__, __LINE__);
#define init_pcb(xxx) init_pcb(xxx, __FILE__, __LINE__);

memory_externs(ilt);
#define get_ilt() get_ilt(__FILE__, __LINE__);
#define put_ilt(xxx) put_ilt(xxx, __FILE__, __LINE__);
#define init_ilt(xxx) init_ilt(xxx, __FILE__, __LINE__);
memory_externs(vrp);
#define get_vrp() get_vrp(__FILE__, __LINE__);
#define put_vrp(xxx) put_vrp(xxx, __FILE__, __LINE__);
#define init_vrp(xxx) init_vrp(xxx, __FILE__, __LINE__);

/* ------------------------------------------------------------------------ */
/* The following allocates memory that is never returned (freed). */
extern UINT32 local_perm_malloc(size_t, const char *, int);
extern UINT32 local_memory_start;               /* start of local memory. */
extern UINT32 *K_delayed_pcb_free;              /* PCB's are delayed free. */
extern void free_local_memory(UINT32 address, UINT32 the_size);
extern void free_pcb(UINT32);
extern UINT32 malloc_pcb(void);
#ifdef FRONTEND
#define PRIVATE_SIZE    MAKE_DEFS_FE_ONLY_SIZE
#else   /* FRONTEND */
#define PRIVATE_SIZE    MAKE_DEFS_BE_ONLY_SIZE
#endif  /* FRONTEND */

extern void check_on_local_memory_address(UINT32 address, size_t in_size);

/* ------------------------------------------------------------------------ */
#ifdef FRONTEND
extern void DL_add_delayed_message(UINT32, UINT32, UINT8, UINT8, UINT8, UINT8);
extern void DL_check_delayed(void);
extern UINT32 DL_remove_delayed_message(UINT32, UINT8, UINT8, UINT8, UINT8);
extern void wc_submit(UINT32, UINT32);
#endif  /* FRONTEND */

extern void MAG_add_delayed_message(UINT8, UINT8, UINT16, UINT32, UINT32, UINT16);
extern void MAG_check_delayed(void);
extern UINT32 check_nvram_p2_available(void);
extern void WaitPortConfig(void);
extern void ISP_GetPositionMap(UINT8);
extern void isp_thread_ilt(UINT8, UINT32);
extern void print_scsi_cmd(DEV *, PRP *, const char *);
extern UINT32 isp_unthread_ilt_1(UINT32);
extern UINT32 isp_unthread_ilt(UINT8, UINT32);
extern UINT32 ISP_AbortTaskSet(UINT32, UINT32, UINT32);
extern void PHY_SetMaxTags(UINT32);
extern void StartOninitDrive(PDD *pddlist[], UINT32 pddlistcount, UINT8 TestType);
extern void ON_InquireAll(PDD *pddlist[], UINT32 pddlistcount, UINT8 TestType);
/* ------------------------------------------------------------------------ */
extern void M_addDDRentry(UINT32, UINT32, UINT32);
extern void Process_DMC_delayed(void);
extern void Process_DMC(void);
extern UINT64 DMC_bits;
extern UINT8 unknown_scsi_command[256];
extern UINT8 unknown_lld_command[256];
extern void mag1_undef(UINT32, UINT32, UINT32, UINT32);
extern void lld1_undef(UINT32, UINT32, UINT32, UINT32);
/* ------------------------------------------------------------------------ */
extern UINT32 s_MallocC(UINT32, const char *, unsigned int);
extern UINT32 s_MallocW(UINT32, const char *, unsigned int);
extern UINT32 s_Malloc(UINT32, const char *, unsigned int);
extern UINT32 p_MallocC(UINT32, const char *, unsigned int);
extern UINT32 p_MallocW(UINT32, const char *, unsigned int);
extern UINT32 p_Malloc(UINT32, const char *, unsigned int);
extern void s_Free(UINT32, UINT32, const char *, unsigned int);
extern void p_Free(UINT32, UINT32, const char *, unsigned int);
extern void s_Free_and_zero(UINT32, UINT32, const char *, unsigned int);
extern void s_mrel_fmm(void *, UINT32, const char *, unsigned int);
extern void release_deferred_memory(void);
extern UINT32 k_malloc(UINT32, struct fmm *, const char *, unsigned int);
extern void k_mrel(void *, UINT32, struct fmm *, const char *, unsigned int);
extern void k_init_mem(struct fmm *, void *, UINT32);
extern void k_make_pre_post_headers(UINT32, UINT32);

extern void TaskReadyByState(UINT8);
extern UINT32 m_asglbuf(UINT32);
extern void PM_RelSGLWithBuf(UINT32 *pSGL);

#ifdef BACKEND
extern void r_blklock(UINT16, UINT64, UINT64);
extern void r_blkunlock(UINT16);
extern void r_bldprp(UINT32, UINT32, UINT64, UINT32, UINT16, UINT8);
extern void r_setrrpec(UINT8, UINT32, UINT32);
extern UINT32 Raid10BusyCorruptionCheck(UINT32, UINT32,UINT32 );
extern void r_setrrpec_r0rstd(UINT8 , UINT32, UINT32);
extern UINT32 m_gensgl(UINT32, UINT32, UINT32);
extern UINT32 v_io_match_cnt(UINT32, UINT32, UINT32);
extern void v_genmrrp(UINT32, UINT32, UINT64, UINT32, UINT32, UINT32 UNUSED, UINT32, UINT32, UINT32);
extern void v_gensrrp(UINT32, UINT32 UNUSED, UINT64, UINT32, UINT32 UNUSED, UINT32, UINT32 UNUSED, UINT32);
extern void v_gensscrrp(UINT32, UINT32 UNUSED, UINT64, UINT32, UINT32 UNUSED, UINT32, UINT32 UNUSED, UINT32);
extern void v_exec_2(UINT32, UINT32, UINT32, UINT32, UINT32, UINT32, UINT32, UINT32);

extern void r_std(UINT64, UINT32, UINT16, UINT32, UINT32, UINT32, UINT32, UINT32, UINT32);
extern void r_VLraid(UINT64, UINT32, UINT16, UINT32, UINT32, UINT32, UINT32, UINT32, UINT32);
extern UINT32 PM_MergeSGL(long *, UINT32, UINT32);

extern void r_initrrbr5s(UINT32, UINT32, UINT32);
extern UINT32 r_initwrite(UINT32, UINT32, UINT32);
extern void r_initread(UINT32, UINT32, UINT32);
extern void r_initrbld(UINT32, UINT32, UINT32);
extern void r_initrpn(UINT32, UINT32);
extern UINT32 r_ulrrb(UINT32 *, UINT32);
extern void r_insrrb(UINT32, UINT64, UINT64, UINT32, UINT32, UINT32, UINT32 UNUSED, UINT32);
extern void r_actrpn(int, UINT32);

extern UINT32 r_splitrrb(UINT32, UINT32, UINT32);
extern UINT32 r_r5a6pwrite(UINT32, UINT32, UINT32, UINT32, UINT32);
extern UINT32 r_r5a6fwrite(UINT32, UINT32, UINT32, UINT32, UINT32, UINT32);
extern void r_r5a6paread(UINT32, UINT32, UINT32);
extern void r_r5a6pfread(UINT32, UINT32, UINT32);
extern void r_r5a6fread(UINT32, UINT32, UINT32, UINT32);
extern void r_r5a6parity(UINT32, UINT32, UINT32);
extern void r_preprpn(UINT32, UINT32, UINT32);
extern UINT32 r_joinn2n(UINT32, UINT32);
extern void r_joins2n(UINT32, UINT32);
extern void r_joins2s(UINT32, UINT32);

extern UINT32 build_sec_vrp(SCD *, UINT32 func, VRP *, ILT *);
extern UINT32 cm_wp2_copy(SCD *, UINT32 func, VRP *, ILT *);

extern UINT32 NV_GetLocalImageSize(UINT32);
extern void define_DIupdate(UINT32, UINT32, UINT32);
extern void access_snapshot(struct SSMS *, struct ILT *, UINT64 sda);
extern void D_init_slrm(UINT32, UINT32, SSMS *);
extern UINT8    gss_version[2];

extern void p$susp_rr(ILT *);

extern UINT32 generate_scsi_write(UINT32, UINT64, UINT32, UINT32*);
extern UINT32 generate_scsi_writesame(UINT32, UINT64, UINT32, UINT32*);
#endif /* BACKEND */

extern void check_memory_all(void);
/* ------------------------------------------------------------------------ */
extern void print_nonzero_ilt(UINT32);
extern void print_prp(UINT32);
extern void DumpMemory(UINT32, UINT32, UINT32);
/* ------------------------------------------------------------------------ */
extern UINT32 DEF_CheckDeviceType(UINT32);

/* ------------------------------------------------------------------------ */
/* This is to get around K_xpcb being an assembler definition. */
#define TaskSetMyState(state)           ((PCB*)K_xpcb)->pc_stat = (state)
/* ------------------------------------------------------------------------ */
#ifdef FRONTEND
#define FEBEMESSAGE "FE "
#else
#define FEBEMESSAGE "BE "
#endif
/* ------------------------------------------------------------------------ */
extern char *millitime_string(void);

#endif /* _CT_asm_defines_h_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
