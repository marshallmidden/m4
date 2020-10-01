#include <stdio.h>
#include <stddef.h>
#include <objalloc.h>
// #include <setjmp.h>
// #include <sys/types.h>

#define __need_res_state
#include <resolv.h>
#include <sched.h>
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <thread_db.h>


typedef struct _pthread_descr_struct *pthread_descr;

/* Atomic counter made possible by compare_and_swap */
struct pthread_atomic {
  long p_count;
  int p_spinlock;
};

typedef struct _pthread_extricate_struct {
    void *pu_object;
    int (*pu_extricate_func)(void *, pthread_descr);
} pthread_extricate_if;


/* Fast thread-specific data internal to libc.  */
enum __libc_tsd_key_t { _LIBC_TSD_KEY_MALLOC = 0,
                        _LIBC_TSD_KEY_DL_ERROR,
                        _LIBC_TSD_KEY_RPC_VARS,
                        _LIBC_TSD_KEY_LOCALE,
                        _LIBC_TSD_KEY_CTYPE_B,
                        _LIBC_TSD_KEY_CTYPE_TOLOWER,
                        _LIBC_TSD_KEY_CTYPE_TOUPPER,
                        _LIBC_TSD_KEY_N };

#define PTHREAD_KEY_2NDLEVEL_SIZE       32

#define PTHREAD_KEY_1STLEVEL_SIZE \
  ((PTHREAD_KEYS_MAX + PTHREAD_KEY_2NDLEVEL_SIZE - 1) \
     / PTHREAD_KEY_2NDLEVEL_SIZE)


/* Arguments passed to thread creation routine */
struct pthread_start_args {
  void *(*start_routine)(void *); /* function to run */
  void *arg;                      /* its argument */
  sigset_t mask;                  /* initial signal mask for thread */
  int schedpolicy;                /* initial scheduling policy (if any) */
  struct sched_param schedparam;  /* initial scheduling parameters (if any) */
};

typedef struct _pthread_rwlock_info {
  struct _pthread_rwlock_info *pr_next;
  struct _pthread_rwlock_t *pr_lock;
  int pr_lock_count;
} pthread_readlock_info;

struct _pthread_descr_struct {
  /* XXX Remove this union for IA-64 style TLS module */
  union {
    struct {
      void *tcb;                /* Pointer to the TCB.  This is not always
                                   the address of this thread descriptor.  */
      union dtv *dtvp;
      pthread_descr self;       /* Pointer to this structure */
      int multiple_threads;
#ifdef NEED_DL_SYSINFO
      uintptr_t sysinfo;
#endif
    } data;
    void *__padding[16];
  } p_header;
  pthread_descr p_nextlive, p_prevlive;
                                /* Double chaining of active threads */
  pthread_descr p_nextwaiting;  /* Next element in the queue holding the thr */
  pthread_descr p_nextlock;     /* can be on a queue and waiting on a lock */
  pthread_t p_tid;              /* Thread identifier */
  int p_pid;                    /* PID of Unix process */
  int p_priority;               /* Thread priority (== 0 if not realtime) */
  struct _pthread_fastlock * p_lock; /* Spinlock for synchronized accesses */
  int p_signal;                 /* last signal received */
  sigjmp_buf * p_signal_jmp;    /* where to siglongjmp on a signal or NULL */
  sigjmp_buf * p_cancel_jmp;    /* where to siglongjmp on a cancel or NULL */
  char p_terminated;            /* true if terminated e.g. by pthread_exit */
  char p_detached;              /* true if detached */
  char p_exited;                /* true if the assoc. process terminated */
  void * p_retval;              /* placeholder for return value */
  int p_retcode;                /* placeholder for return code */
  pthread_descr p_joining;      /* thread joining on that thread or NULL */
  struct _pthread_cleanup_buffer * p_cleanup; /* cleanup functions */
  char p_cancelstate;           /* cancellation state */
  char p_canceltype;            /* cancellation type (deferred/async) */
  char p_canceled;              /* cancellation request pending */
  char * p_in_sighandler;       /* stack address of sighandler, or NULL */
  char p_sigwaiting;            /* true if a sigwait() is in progress */
  struct pthread_start_args p_start_args; /* arguments for thread creation */
  void ** p_specific[PTHREAD_KEY_1STLEVEL_SIZE]; /* thread-specific data */
#if !(USE_TLS && HAVE___THREAD)
  void * p_libc_specific[_LIBC_TSD_KEY_N]; /* thread-specific data for libc */
  int * p_errnop;               /* pointer to used errno variable */
  int p_errno;                  /* error returned by last system call */
  int * p_h_errnop;             /* pointer to used h_errno variable */
  int p_h_errno;                /* error returned by last netdb function */
  struct __res_state *p_resp;   /* Pointer to resolver state */
  struct __res_state p_res;     /* per-thread resolver state */
#endif
  int p_userstack;              /* nonzero if the user provided the stack */
  void *p_guardaddr;            /* address of guard area or NULL */
  size_t p_guardsize;           /* size of guard area */
  int p_nr;                     /* Index of descriptor in __pthread_handles */
  int p_report_events;          /* Nonzero if events must be reported.  */
  td_eventbuf_t p_eventbuf;     /* Data for event.  */
  struct pthread_atomic p_resume_count; /* number of times restart() was
                                           called on thread */
  char p_woken_by_cancel;       /* cancellation performed wakeup */
  char p_condvar_avail;         /* flag if conditional variable became avail */
  char p_sem_avail;             /* flag if semaphore became available */
  pthread_extricate_if *p_extricate; /* See above */
  pthread_readlock_info *p_readlock_list;  /* List of readlock info structs */
  pthread_readlock_info *p_readlock_free;  /* Free list of structs */
  int p_untracked_readlock_count;       /* Readlocks not tracked by list */
  int p_inheritsched;           /* copied from the thread attribute */
#if HP_TIMING_AVAIL
  hp_timing_t p_cpuclock_offset; /* Initial CPU clock for thread.  */
#endif
#ifdef USE_TLS
  char *p_stackaddr;            /* Stack address.  */
#endif
  size_t p_alloca_cutoff;       /* Maximum size which should be allocated
                                   using alloca() instead of malloc().  */
  /* New elements must be added at the end.  */
} __attribute__ ((aligned(32)));

/* Return the thread descriptor for the current thread.

   The contained asm must *not* be marked volatile since otherwise
   assignments like
        pthread_descr self = thread_self();
   do not get optimized away.  */
#define THREAD_SELF \
({                                                                            \
  register pthread_descr __self;                                              \
  __asm__ ("movl %%gs:%c1,%0" : "=r" (__self)                                 \
           : "i" (offsetof (struct _pthread_descr_struct,                     \
                            p_header.data.self)));                            \
  __self;                                                                     \
})


int abc = 5;

int testing()
{
	fprintf(stderr, "hi there, %p\n", abc);

	abc = THREAD_SELF;

	fprintf(stderr, "hi there, %p\n", abc);
}


int main(void)
{
	fprintf(stderr, "entering main\n");
	testing();
	fprintf(stderr, "exiting main\n");
	exit(0);
}
