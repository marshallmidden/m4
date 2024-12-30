#include <stdio.h>
#include <errno.h>

#define FRONTEND
/* #define BACKEND */
/* #define CCB_RUNTIME_CODE */


#include "../../Shared/Src/L_XIO3D.c"

#define LENGTH 40
static unsigned long offset = 0;

/* ------------------------------------------------------------------------ */

static void dumpmemory(unsigned long offset)
{
  int i;
  fprintf(stderr, "         Frontend  Backend    CCB\n");
  fprintf(stderr, "         %-8.8x %-8.8x %-8.8x\n", FE_BASE_ADDR+offset, BE_BASE_ADDR+offset, CCB_BASE_ADDR+offset);
  for (i = 0; i< LENGTH; i++) {
    fprintf(stderr, "%-8.8x %-8.8x %-8.8x %-8.8x\n",
        offset+i*4, *(int*)(FE_BASE_ADDR+offset+4*i), *(int*)(BE_BASE_ADDR+offset+4*i), *(int*)(CCB_BASE_ADDR+offset+4*i));
  }
}	/* end of dumpmemory */

/* ------------------------------------------------------------------------ */
static void process_args(int argc, char *argv[])
{
  int i;

  for (i = 1; i < argc; ++i)
  {
    offset = strtoul(argv[i], 0, 0);
    if (errno) {
      fprintf(stderr, "conversion of [%s] failed, errno=%d\n", argv[i], errno);
      exit(1);
    }
    dumpmemory(offset);
  }
}	/* end of process_args */

/* ------------------------------------------------------------------------ */


int main(int argc, char *argv[])
{
  char *p;
  char buf[BUFSIZ];
  int i;

  fprintf(stderr, "in main, setting up shared memory\n");

  SETUP_linux();

  process_args(argc, argv);

  while (1) {
    p = fgets(buf, BUFSIZ, stdin);
    if (p == NULL) {
      exit(0);
    }
    i = strlen(buf);
    if (i == 0) {
      exit(0);
    }
    i--;
    buf[i] = '\0';
    if (i == 0) {	/* carriage return alone */
      offset += LENGTH;
    } else if (i == 1) {
      switch (buf[0]) {
        case 'a':
	  *(int *)(FRONT_END_PCI_START+0x50) = 1;
	  break;
        case 'b':
	  *(int *)(FRONT_END_PCI_START+0x50) = 0;
	  break;
        case '+':
	  offset += LENGTH;
	  break;
        case '-':
	  offset -= LENGTH;
	  break;
        case '=':
        case '.':
	  break;
	default:
	  offset = strtoul(buf, 0, 0);
	  if (errno) {
	    fprintf(stderr, "conversion of [%s] failed, errno=%d\n", buf, errno);
	    continue;
	  }
	  break;
      }
    } else {
      if (strcmp("quit",buf) == 0 ||
          strcmp("exit",buf) == 0) {
        exit(0);
      }
      offset = strtoul(buf, 0, 0);
      if (errno) {
	fprintf(stderr, "conversion of [%s] failed, errno=%d\n", buf, errno);
	continue;
      }
    }
    dumpmemory(offset);
  }
}	/* end of main */

/* ------------------------------------------------------------------------ */
