#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>

char * buf = "abcdefghijklmnopqrstuvwxyz";
int32_t num_pthreads = 20;
int32_t count = 15;
int32_t fd = 1;

/* printf(at, 5, 10, 'a');   will go to 5th row down, 10th character across and put 'a' there. */
/* Note: 0 and 1 are the same for the "10", or x direction. */
char at[] = "\033[%d;%dH%2d%c";
char clearscreen[] = "\033[;H\033[J";
char eop[] = "\033[30;0H";

void * new_thread(void *args)
{
    int64_t arg = (int64_t)args;
    int32_t i;
    char c;

    for (i = 0; i < count; i++) {
	c = buf[arg];
	fprintf(stderr, at, 5+i, 3*arg+1, i, c);
	usleep(1000000/4);		/* 1/4 of a second. */
    }
    fprintf(stderr, eop);
    return(NULL);
}

main()
{
   pthread_t thread;
   int64_t i;

   fprintf(stderr, "%s", clearscreen);
   for (i = num_pthreads-1; i >= 0; i--) {
	if (pthread_create(&thread, NULL, new_thread, (void *)i)) {
	   fprintf(stderr, "error creating a new thread \n");
	   exit(1);
	}
	pthread_detach(thread);
   }
   pthread_exit(NULL);
}
