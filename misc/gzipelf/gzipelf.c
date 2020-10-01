#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

int             main(int argc, char *argv[])
{
  int             fd;
  char           *fname;
  int             lth;
  struct stat     sb;
  char            cmd[BUFSIZ];
  FILE           *gf;

  fname = argv[1];

  if (stat(fname, &sb) != 0) {
    fprintf(stderr, "Can't stat file %s\n", fname);
    exit(1);
  }
  write(1, "ELF.mips32.gzip", 16);
  lth = sb.st_size;
  lth = htonl(lth);
  if (sizeof(lth) != 4) {
    fprintf(stderr, "error, unsigned int size not 4!\n");
    exit(1);
  }
  write(1, &lth, 4);
  snprintf(cmd, BUFSIZ, "gzip -c -9 %s", fname);
  if (system(cmd) < 0) {
    perror("system");
    exit(1);
  }
  exit(0);
}
