#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>

/* ------------------------------------------------------------------------ */
char           *buf = "abcdefghijklmnopqrstuvwxyz";
#define NUM_PTHREADS 5
int32_t             count = 5;
int32_t             fd = 1;

/* ------------------------------------------------------------------------ */
int32_t    foo = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int32_t mult[NUM_PTHREADS] = {
	100,
	1000,
	10000,
	100000,
	1000000,
};

/* ------------------------------------------------------------------------ */
int32_t             foo_initializer(void)
{
  int32_t ret;

  pthread_mutex_lock(&mutex);
  if (foo == 0) {
    foo = 10;				/* This must only be done once. */
  } else {
    foo++;
  }
  ret = foo;
  pthread_mutex_unlock(&mutex);
  return(ret);
}

/* ------------------------------------------------------------------------ */
int32_t             foo_add(int32_t arg)
{
  int32_t ret;

  pthread_mutex_lock(&mutex);
  foo += arg;
  ret = foo;
  pthread_mutex_unlock(&mutex);
  return(ret);
}

/* ------------------------------------------------------------------------ */
void           *new_thread(void *args)
{
  int64_t		  arg = (int64_t)args;
  int32_t             i;
  int32_t		  ret;
  char		  print[2] = "x\n";

  print[0] = buf[arg];
  ret = foo_initializer();
  fprintf(stderr, "thread %d-%p, foo=%9d\n", arg, (void *)pthread_self(), ret);

  for (i = 0; i < count; i++) {
    write(fd, print, 2);
    ret = foo_add(mult[arg]);
    usleep(1000000/4);		/* 1/4 of a second. */
  }
  fprintf(stderr, "thread %d-%p, foo=%9d\n", arg, (void *)pthread_self(), ret);
  return (NULL);
}

/* ------------------------------------------------------------------------ */
int32_t main()
{
  pthread_t       thread;
  int64_t             i;

  for (i = 0; i < NUM_PTHREADS; i++) {
    if (pthread_create(&thread, NULL, new_thread, (void *)i)) {
      fprintf(stderr, "error creating a new thread \n");
      exit(1);
    }
    pthread_detach(thread);
  }
  pthread_exit(NULL);
}

/* ------------------------------------------------------------------------ */
