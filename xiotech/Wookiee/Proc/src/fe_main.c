#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "CT_defines.h"

#include "system.h"

/* ------------------------------------------------------------------------ */
/* A starting place for the stack.  Program may change it. */
/* FRAME=64 (bytes to save 16 'r' registers. */
/* PLEVELS=16 (16 procedure levels allowed). */
/* PSTACK=512 (512 extra bytes for "stack" usage. */
char ct_stack[64*16 + 512];

/* CPU speed, environment CPU_SPEED passed in. */
ulong ct_cpu_speed = 3200;               /* default to 3200 MHz */

/* Get shared memory for all processes. */
extern unsigned long SETUP_linux(void);

extern unsigned long CT_start_shinittable;
extern unsigned long CT_end_shinittable;
#include <string.h>

/* Routine to enter. */
extern void K$start(void);

/* confusion reigns. */
extern unsigned long K_xpcb;
unsigned long current_pcb_ptr = (unsigned long)&K_xpcb;

/* ------------------------------------------------------------------------ */
int main(void)
{
  char *p;
  long i;
  unsigned long *w;

  close(0);          /* No stdin needed. */
  close(3);          /* Extra file descriptors left over from "commands" */
  close(4);
  close(5);
  close(6);
  close(7);
  close(8);
  close(9);
  close(10);

  /* Set up the frame location, and the stack 64 bytes after that. */
  fp = (ulong) &ct_stack[0];
  sp = (ulong)fp + 64;
  pfp = fp;

  p = getenv("CPU_SPEED");
  if (p != NULL) {
    i = strtol(p, (char **)NULL, 10);
    if (i > 500 && i < 100000) {
      ct_cpu_speed = i;
    }
  }
  fprintf(stderr, "CPU_SPEED=%ld\n", ct_cpu_speed);

  /* CXGH */
  CT_main_esp = get_esp();      /* save stack before i960 main program */
  CT_main_ebp = get_ebp();      /* save frame before i960 main program */

  SETUP_linux();                /* setup shared memory with everyone. */

/* Setup the .shinit data section to match the .shdata section, via the .shinittable. */
  w = (unsigned long *)&CT_start_shinittable;
  while (w < (unsigned long *)&CT_end_shinittable) {
      unsigned long from = *w;
      unsigned long to = *(w+1);
      unsigned long count = *(w+2);
      memmove((void *)to, (void *)from, count);
      w += 3;
  }

  K$start();                    /* Start the program */

  abort();                      /* must not get here */
}

/* ------------------------------------------------------------------------ */
/* End of file fe_main.c */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
