
#include "xthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define NUM_THREADS	100
#define NUM_WAITERS	50

/* ------------------------------------------------------------------------ */
static pthread_cond_t       xm_cvar;
static pthread_mutex_t      xm_mutx;
static pthread_mutex_t      xm_mutx2;

static uint32_t value = 0;
/* ------------------------------------------------------------------------ */
void           *new_thread(void *args)
{
    int64_t             arg = (int64_t) args;
    int rc;
    int i = 0;

    /*
     * Lock the mutex.
     */
    rc = xthread_mutexlock(&xm_mutx, 1);
    if (rc != 0 ) {
	fprintf(stderr, "xthread_mutexlock (%s)\n", strerror(rc));
	abort();
    }

    rc = xthread_mutexlock(&xm_mutx2, 1);
    if (rc != 0 ) {
	fprintf(stderr, "xthread_mutexlock (%s)\n", strerror(rc));
	abort();
    }
    value++;

    if (arg < NUM_WAITERS) {	/* first threads. */
	while(1) {
	    i++;
            /*
	     * Wait on the condition variable.
	     */
	    fprintf(stderr, "thread %d, value=%d condition variable waiting %d\n", arg, value, i);
	    xthread_mutexunlock(&xm_mutx2);
	    rc = pthread_cond_wait(&xm_cvar, &xm_mutx);

	    if (rc != 0 ) {
		fprintf(stderr, "pthread_cond_wait (%s)\n", strerror(rc));
		abort();
	    }
	    rc = xthread_mutexlock(&xm_mutx2, 1);
	    if (rc != 0 ) {
		fprintf(stderr, "xthread_mutexlock (%s)\n", strerror(rc));
		abort();
	    }
	    value++;
	}
    } else {
	fprintf(stderr, "thread %d, value=%d initial entering %d\n", arg, value, i);
	xthread_mutexunlock(&xm_mutx2);
	/*
	 * Unlock the mutex.
	 */
	xthread_mutexunlock(&xm_mutx);

	while(1) {

	    sleep(arg-NUM_WAITERS+1);
	    i++;

	    /*
	    ** Lock the mutex.
	    */
	    rc = xthread_mutexlock(&xm_mutx, 1);
	    if (rc != 0 ) {
		fprintf(stderr, "xthread_mutexlock (%s)\n", strerror(rc));
		abort();
	    }

	    rc = xthread_mutexlock(&xm_mutx2, 1);
	    if (rc != 0 ) {
		fprintf(stderr, "xthread_mutexlock (%s)\n", strerror(rc));
		abort();
	    }
	    value++;
	    fprintf(stderr, "thread %d, value=%d condition variable signal %d\n", arg, value, i);
	    xthread_mutexunlock(&xm_mutx2);

	    /*
	     * Unlock the mutex.
	     */
	    xthread_mutexunlock(&xm_mutx);

	    pthread_cond_signal(&xm_cvar);
	}
    }
}

/* ------------------------------------------------------------------------ */
int32_t             main()
{
    int64_t             i;
    int rc;
    pthread_condattr_t  condinit;

    xthread_init();

    /*
     * Initialize our conditional variable attributes.
     */
    rc = pthread_condattr_init(&condinit);
    if (rc != 0 ) {
        fprintf(stderr, "pthread_condattr_init (%s)\n", strerror(rc));
	abort();
    }

    /*
     * Set the condition variable process shared attribute.
     */
    rc = pthread_condattr_setpshared(&condinit, PTHREAD_PROCESS_PRIVATE);
    if (rc != 0) {
        fprintf(stderr, "pthread_condattr_setpshared (%s)\n", strerror(rc));
	abort();
    }

    /*
     * Initialize the condition variable.
     */
    rc = pthread_cond_init(&xm_cvar, &condinit);
    if (rc != 0 ) {
        fprintf(stderr, "pthread_cond_init (%s)\n", strerror(rc));
	abort();
    }

    /*
    ** Initialize the condition variable mutex.
    */
    rc = xthread_mutexinit(&xm_mutx);
    if (rc != 0 ) {
        fprintf(stderr, "xthread_mutexinit (%s)\n", strerror(rc));
	abort();
    }

    /*
    ** Initialize the value mutex.
    */
    rc = xthread_mutexinit(&xm_mutx2);
    if (rc != 0 ) {
        fprintf(stderr, "xthread_mutexinit (%s)\n", strerror(rc));
	abort();
    }

    /*
     * Destroy memory allocated with pthread_condattr_init.
     */
    rc = pthread_condattr_destroy(&condinit);
    if (rc != 0) {
        fprintf(stderr, "pthread_condattr_destroy (%s)\n", strerror(rc));
	abort();
    }

    for (i = 0; i < NUM_THREADS; i++)
    {
      if (xthread_cr(new_thread, (void *) i) == NULL) {
        fprintf(stderr, "error creating a new thread \n");
        exit(1);
      }
    }
    pthread_exit(NULL);
}

/* ------------------------------------------------------------------------ */
