/* ------------------------------------------------------------------------ */
// #define PRINTING
#define PRINTING_END
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

/* ------------------------------------------------------------------------ */
/* The idea is to come up with something that marks that a variable is being
   read XXX times, and written once.  */

/* Thinking of 20 threads that read %i (thread number) times, then
   increments an array[%i] then goes back to reading.
   	thread 0 does all reads, no writes.
   	thread 1 increments read, then increments write.
   	thread 2 increments read, then increments read, then increments write.
	...
*/

/* ------------------------------------------------------------------------ */
#define	NUM_PTHREADS	90
#define	count	9000
/* #define	count	1000 */

/*                           Y   X  var  r  w	=9+5+5+1=20 chars */
#define ATWIDTH 20
#ifdef PRINTING
static const char            clearscreen[] = "\033[;H\033[J";
static const char at[] = "\033[%lld;%dH%9d%5d%5d ";
#endif	/* PRINTING */
#define EOPROW	NUM_PTHREADS+1
static const char            eop[] = "\033[%d;0H";

volatile static int32_t		    num_threads = 0;
volatile static int32_t             variable = 0;
volatile static int32_t             array_r[NUM_PTHREADS];
volatile static int32_t             array_w[NUM_PTHREADS];

/* ------------------------------------------------------------------------ */
/* Implement a read->many, and write->one locking method. */
/* If a write is requested, wait for reads to finish, but allow no new reads in the mean-time. */

/* State_mutex covers: state_read_count, state_write_wanted, state_read_wanted, state_cond. */

static pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
/* if reads outstanding, and write is needed, a condition variable is used. */
static pthread_cond_t  state_cond = PTHREAD_COND_INITIALIZER;
/* Number of reads in progress (multiple allowed). */
volatile static int32_t             state_read_count = 0;
/* Flag for if a write is needed, but reads are outstanding. */
volatile static int32_t             state_write_wanted = 0;	/* NOTE: NOT "us" doing read and write. */
volatile static int32_t             state_read_waiting = 0;	/* we want to write, but can't */
volatile static int32_t             state_write_waiting = 0;	/* number waiting to write. */

/* ------------------------------------------------------------------------ */
static void     IC_read_lock(pthread_mutex_t * mutex, pthread_cond_t * cond)
{
  int32_t             ret;

  ret = pthread_mutex_lock(mutex);
  if (ret != 0) {			/* if lock succeeded */
    fprintf(stderr, "%s(%s):%d error from pthread_mutex_lock()=%d\n", __FILE__, __FUNCTION__, __LINE__, ret);
    abort();
  }
  state_read_waiting++;
  while (state_write_wanted != 0) {	/* If outstanding "write" */
    ret = pthread_cond_wait(cond, mutex);
    if (ret != 0) {
      fprintf(stderr, "%s(%s):%d pthread_cond_wait returned %d\n", __FILE__, __FUNCTION__, __LINE__, ret);
      abort();
    }
  }
  state_read_count++;			/* read is now allowed. */
  state_read_waiting--;
  ret = pthread_mutex_unlock(mutex);
  if (ret != 0) {
    fprintf(stderr, "%s(%s):%d pthread_mutex_unlock returned %d\n", __FILE__, __FUNCTION__, __LINE__, ret);
    abort();
  }
}

/* ------------------------------------------------------------------------ */
/* NOTE: mutex is still locked upon exit. */
static void     IC_write_lock(pthread_mutex_t * mutex, pthread_cond_t * cond)
{
  int32_t             ret;

  ret = pthread_mutex_lock(mutex);
  if (ret == 0) {			/* if lock succeeded */
 /* If someone else wanted to write before us, allow that. */
    state_write_waiting++;
    while (state_write_wanted != 0) {	/* If outstanding "write" */
      ret = pthread_cond_wait(cond, mutex);
      if (ret != 0) {
	fprintf(stderr, "%s(%s):%d pthread_cond_wait #1 returned %d\n", __FILE__, __FUNCTION__, __LINE__, ret);
	abort();
      }
    }
    state_write_wanted++;		/* I got the write. */
    if (state_write_wanted != 1) {
      fprintf(stderr, "%s(%s):%d failed logic state_write_wanted=%d\n", __FILE__, __FUNCTION__, __LINE__, state_write_wanted);
      abort();
    }
    while (state_read_count != 0) {	/* If there are reads outstanding */
      ret = pthread_cond_wait(cond, mutex);
      if (ret != 0) {
	fprintf(stderr, "%s(%s):%d pthread_cond_wait #2 returned %d\n", __FILE__, __FUNCTION__, __LINE__, ret);
	abort();
      }
    }
    return;
  }
  fprintf(stderr, "%s(%s):%d error from pthread_mutex_lock()=%d\n", __FILE__, __FUNCTION__, __LINE__, ret);
  abort();
}

/* ------------------------------------------------------------------------ */
static void     IC_read_unlock(pthread_mutex_t * mutex, pthread_cond_t * cond)
{
  int32_t             ret;

  ret = pthread_mutex_lock(mutex);
  if (ret != 0) {
    fprintf(stderr, "%s(%s):%d pthread_mutex_lock returned %d\n", __FILE__, __FUNCTION__, __LINE__, ret);
    abort();
  }
  if (state_read_count <= 0) {
    fprintf(stderr, "%s(%s):%d state_read_count <= 0 ?? (%d)\n", __FILE__, __FUNCTION__, __LINE__, state_read_count);
    abort();
  }
  state_read_count--;
  if (state_read_count == 0 && (state_write_wanted != 0 || state_write_waiting != 0 || state_read_waiting != 0)) {
    ret = pthread_mutex_unlock(mutex);
    if (ret != 0) {
      fprintf(stderr, "%s(%s):%d pthread_mutex_unlock #1 returned %d\n", __FILE__, __FUNCTION__, __LINE__, ret);
      abort();
    }
    ret = pthread_cond_broadcast(cond);
    if (ret != 0) {
      fprintf(stderr, "%s(%s):%d pthread_cond_broadcast returned %d\n", __FILE__, __FUNCTION__, __LINE__, ret);
      abort();
    }
  } else {
    ret = pthread_mutex_unlock(mutex);
    if (ret != 0) {
      fprintf(stderr, "%s(%s):%d pthread_mutex_unlock #2 returned %d\n", __FILE__, __FUNCTION__, __LINE__, ret);
      abort();
    }
  }
}

/* ------------------------------------------------------------------------ */
/* NOTE: mutex is locked when entering. */
static void     IC_write_unlock(pthread_mutex_t * mutex, pthread_cond_t * cond)
{					/* (read = 0, write = 1) */
  int32_t             ret;

  if (state_write_wanted != 1 || state_read_count != 0) {
    fprintf(stderr, "%s(%s):%d state_write_wanted(%d) != 1 || state_read_count(%d) != 0\n", __FILE__, __FUNCTION__, __LINE__, state_write_wanted, state_read_count);
    abort();
  }
  state_write_wanted--;
  state_write_waiting--;
  if (state_write_waiting != 0 || state_read_waiting != 0) {
    ret = pthread_mutex_unlock(mutex);
    if (ret != 0) {
      fprintf(stderr, "%s(%s):%d pthread_mutex_unlock #1 returned %d\n", __FILE__, __FUNCTION__, __LINE__, ret);
      abort();
    }
    ret = pthread_cond_broadcast(cond);
    if (ret != 0) {
      fprintf(stderr, "%s(%s):%d pthread_cond_broadcast returned %d\n", __FILE__, __FUNCTION__, __LINE__, ret);
      abort();
    }
  } else {
    ret = pthread_mutex_unlock(mutex);
    if (ret != 0) {
      fprintf(stderr, "%s(%s):%d pthread_mutex_unlock #2 returned %d\n", __FILE__, __FUNCTION__, __LINE__, ret);
      abort();
    }
  }
}

/* ------------------------------------------------------------------------ */
static void    *new_thread(void *args)
{
  int64_t             arg = (int64_t) args;
  int32_t             i;
  int32_t             j;
#ifdef PRINTING
  int32_t             value;
#endif /* PRINTING */

  IC_write_lock(&state_mutex, &state_cond);
  num_threads++;
  fprintf(stderr, "num_threads=%d\n", num_threads);
  usleep(1000000 / 1000);
  IC_write_unlock(&state_mutex, &state_cond);

 /* Do readlocks and increments first. */
  for (i = 0; i < count; i++) {
#ifdef PRINTING
    value = 0;
#endif /* PRINTING */
    if (arg == 0) {
      j = 1;
    } else {
      j = i % (arg + 1);
    }
    if (j != 0) {			/* if doing a read */
      IC_read_lock(&state_mutex, &state_cond);
#ifdef PRINTING
      value = variable;
#endif /* PRINTING */
      array_r[arg]++;
      IC_read_unlock(&state_mutex, &state_cond);
    } else {				/* if doing a write */
      IC_write_lock(&state_mutex, &state_cond);
      variable++;
#ifdef PRINTING
      value = variable;
#endif /* PRINTING */
      array_w[arg]++;
      IC_write_unlock(&state_mutex, &state_cond);
    }
#ifdef PRINTING
    fprintf(stdout, at, 1 + arg, ATWIDTH * (i % 10) + 1, value, array_r[arg], array_w[arg]);
/*    usleep(1000000 / 1000); */		/* 1/10 of a second. */
#endif	/* PRINTING */
  }
  IC_write_lock(&state_mutex, &state_cond);
  num_threads--;
  if (num_threads == 0) {
#ifdef PRINTING_END
    fprintf(stdout, eop, EOPROW);
    fprintf(stdout, "variables:\n");
    fprintf(stdout, "\tvariable=%d  state_read_count=%d  state_write_wanted=%d  state_read_waiting=%d state_write_waiting=%d\n", variable, state_read_count, state_write_wanted, state_read_waiting, state_write_waiting);
    for (i = 0; i < NUM_PTHREADS; i++) {
      fprintf(stdout, "%5d %5d ", array_r[i], array_w[i]);
      if (i%10 == 9) {
	fprintf(stdout, "\n");
      }
    }
    fprintf(stdout, "\n");
#endif	/* PRINTING_END */
  }
  IC_write_unlock(&state_mutex, &state_cond);
 /* Do readlocks and increments first. */
  return (NULL);
}

/* ------------------------------------------------------------------------ */
int                 main(void)
{
  pthread_t       thread;
  int64_t             i;

#ifdef PRINTING
  fprintf(stdout, "%s", clearscreen);
#endif	/* PRINTING */
  for (i = 0; i < NUM_PTHREADS; i++)
/*   for (i = NUM_PTHREADS - 1; i >= 0; i--) */
  {
    if (pthread_create(&thread, NULL, new_thread, (void *) i)) {
      fprintf(stderr, "error creating a new thread \n");
      exit(1);
    }
    pthread_detach(thread);
  }
  pthread_exit(NULL);
}

/* ------------------------------------------------------------------------ */
