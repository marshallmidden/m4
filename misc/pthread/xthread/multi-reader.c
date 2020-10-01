/* ------------------------------------------------------------------------ */
#define PRINTING
/* #define PRINTING_NUMTHREADS */
/* #define PRINTING_END */
#define PRINT_WRAP_NUM 10

/* #define LAST_WRITE_ONLY */

#include "xthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

/* ------------------------------------------------------------------------ */
/* The idea is to come up with something that marks that a variable is being
   read XXX times, and written somewhat randomly.  */

/* Thinking of 20 threads that read %i (thread number) times, then
   increments an array[%i] then goes back to reading.
   	thread 0 does all reads, no writes.
   	thread 1 increments read, then increments write.
   	thread 2 increments read, then increments read, then increments write.
	...
*/

/* ------------------------------------------------------------------------ */
char           *buf = "abcdefghijklmnopqrstuvwxyz";
#define	NUM_PTHREADS	90
/* #define	NUM_PTHREADS	900 */

/* ---------------------------------------------- */
/* #define	THREAD_FIRST	0 */
/* #define	THREAD_LAST	NUM_PTHREADS-1 */
/* #define	THREAD_CMP	<= THREAD_LAST */
/* #define	THREAD_INC	++ */
/* ---------------------------------------------- */
#define	THREAD_FIRST	NUM_PTHREADS-1
#define	THREAD_LAST	0
#define	THREAD_CMP	>= THREAD_LAST
#define	THREAD_INC	--
/* ---------------------------------------------- */

/* NUM_PTHREADS 900 @ 1000000=7:22.12 on dual 3.2 64 bit xeons. 198% CPU */
/* NUM_PTHREADS 900 @ 100000=0:42.36 on dual 3.2 64 bit xeons. 198% CPU */
/* #define	COUNT	1000 */
/* #define	COUNT	10000 */
#define	COUNT	100000
/* #define	COUNT	1000000 */

/* printf(at, 5, 10, 'a');   will go to 5th row down, 10th character across and put 'a' there. */
/* Note: 0 and 1 are the same for the or x direction. */
char            at[] = " \033[%d;%dH%9d%9d%9d";
/*                           Y   X  var  r  w	=9+9+9+1=28 chars */
#define ATWIDTH 28
/* #define AT_NUM	7 */	/* 200 wide screen / ATWIDTH = 7.14285 */
#define AT_NUM	1	/* Keep it in one column. */
char            clearscreen[] = "\033[;H\033[J";
#define EOPROW	NUM_PTHREADS+1
char            eop[] = "\033[%d;0H";

int64_t		    num_threads = 0;
int64_t		    num_threads_started = 0;
int64_t             array_r[NUM_PTHREADS];
int64_t             array_w[NUM_PTHREADS];
int64_t             write_locks = 0;
int64_t             read_locks = 0;

/* ------------------------------------------------------------------------ */
pthread_rwlock_t the_lock;
/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */
void           *new_thread(void *args)
{
  int64_t             arg = (int64_t) args;
  int64_t             i;
  char            c;
  int64_t             j;
  int64_t             value;
  int64_t             local_read_locks = 0;
  int64_t calcwrite;
  int64_t calcread;

  xthread_rw_wwait(&the_lock);
  num_threads++;
  num_threads_started++;
#ifdef PRINTING_NUMTHREADS
fprintf(stderr, "num_threads=%d started=%d\n", num_threads, num_threads_started);
#endif /* PRINTING_NUMTHREADS */
/*   value = num_threads; */
  xthread_rwlockunlock(&the_lock);
/*   if (value == 1) { */
/*     usleep(1); */
/*   } */

 /* Do readlocks and increments first. */
  for (i = 0; i < COUNT; i++) {
    value = 0;
    if (arg == 0) {
      j = 1;		/* Only read locks. */
#ifdef LAST_WRITE_ONLY
    } else if (arg == NUM_PTHREADS-1) {
      j = 0;		/* Only write locks. */
#endif /* LAST_WRITE_ONLY */
    } else {
      j = i % (arg + 1);
    }
    if (j != 0) {			/* if doing a read */
      xthread_rw_rwait(&the_lock);
      local_read_locks++;
      value = write_locks;
      array_r[arg]++;
      xthread_rwlockunlock(&the_lock);		/* They say this is right! */
    } else {				/* if doing a write */
      xthread_rw_wwait(&the_lock);
      write_locks++;
      value = write_locks;
      array_w[arg]++;
      xthread_rwlockunlock(&the_lock);
    }
#ifdef PRINTING
    fprintf(stderr, at, 1 + arg, ATWIDTH * (i % AT_NUM) + 1, value, array_r[arg], array_w[arg]);
/*    usleep(1000000 / 1000); */		/* 1/10 of a second. */
#endif	/* PRINTING */
  }
  xthread_rw_wwait(&the_lock);
  read_locks += local_read_locks;
  num_threads--;
#ifdef PRINTING_NUMTHREADS
fprintf(stderr, "num_threads=%d started=%d\n", num_threads, num_threads_started);
#endif /* PRINTING_NUMTHREADS */
  if (num_threads == 0 && num_threads_started == NUM_PTHREADS) {
#ifdef PRINTING
    fprintf(stderr, eop, EOPROW);
#endif /* PRINTING */
#ifdef PRINTING_END
    fprintf(stderr, "variables:\n");
#endif /* PRINTING_END */
    {
	calcwrite = 0;
	calcread = 0;
	for (i = 0; i < NUM_PTHREADS; i++) {
	    if (i == 0) {
		calcread += COUNT;
#ifdef LAST_WRITE_ONLY
	    } else if (i == NUM_PTHREADS-1) {
		calcwrite += COUNT;	/* Only write locks. */
#endif /* LAST_WRITE_ONLY */
	    } else {
		j = (COUNT+i) / (i+1);
		calcwrite += j;
		calcread += COUNT - j;
	    }
	}
	if (write_locks != calcwrite || read_locks != calcread) {
	    fprintf(stderr, "\twrite_locks=%d  calcwrite=%d  read_locks=%d  calcread=%d \n",
		    write_locks, calcwrite, read_locks, calcread);
	}
    }
    calcwrite = 0;
    calcread = 0;
    for (i = 0; i < NUM_PTHREADS; i++) {
#ifdef PRINTING_END
      fprintf(stderr, " %9d %9d", array_r[i], array_w[i]);
#endif /* PRINTING_END */
      calcread += array_r[i];
      calcwrite += array_w[i];
#ifdef PRINTING_END
      if (i%PRINT_WRAP_NUM == PRINT_WRAP_NUM-1)
      {
	fprintf(stderr, "\n");
      }
#endif /* PRINTING_END */
    }
#ifdef PRINTING_END
    fprintf(stderr, "\n");
#endif /* PRINTING_END */
    if (write_locks != calcwrite || read_locks != calcread) {
	fprintf(stderr, "\twrite_locks=%d  summedwrite=%d  read_locks=%d  summedwrite=%d \n",
		write_locks, calcwrite, read_locks, calcread);
    }
  }
  xthread_rwlockunlock(&the_lock);
  return (NULL);
}

/* ------------------------------------------------------------------------ */
int32_t             main()
{
  int64_t             i;
  int32_t ret;

  xthread_init();
#ifdef PRINTING
  fprintf(stderr, "%s", clearscreen);
#endif	/* PRINTING */
  if((ret = xthread_rwlockinit(&the_lock)) != 0) {
    fprintf(stderr, "xthread_rwlockinit returned non-zero (%d)\n", ret);
    abort();
  }
  for (i = THREAD_FIRST; i THREAD_CMP; i THREAD_INC)
  {
    if (xthread_cr(new_thread, (void *) i) == NULL) {
      fprintf(stderr, "error creating a new thread \n");
      exit(1);
    }
  }
  pthread_exit(NULL);
}

/* ------------------------------------------------------------------------ */
