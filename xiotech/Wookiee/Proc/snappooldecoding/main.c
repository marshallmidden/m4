#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* ------------------------------------------------------------------------ */
int fd;
blkcnt_t g_capacity;
/* ------------------------------------------------------------------------ */
/* In other files. */
extern void restore_ss_nv_task(void);

struct stat stat_buf;
/* ------------------------------------------------------------------------ */
static void initialize(int argc, char *argv[])
{
    unsigned long long siz;
    stat_buf.st_blksize = 512;  /* Initialize to 512 byte blocks. */

    if (argc < 2) {
        fprintf(stderr, "One argument, the file name is required. argc=%d\n", argc);
        exit(1);
    }
    fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        perror("open failed");
        exit(1);
    }
    if (argc == 3) {
        siz = strtoll(argv[2], (char **)NULL, 10);
    } else {
        if (fstat(fd, &stat_buf) < 0) {
            perror("fstat failed");
            exit(1);
        }
        siz = stat_buf.st_size;
        if (siz == 0) {
            fprintf(stderr, "File size is zero, try adding a second argument\n");
            exit(1);
        }
    }
    g_capacity = siz / 512;
    fprintf(stdout, "File %s g_capacity (blocks) is %lld, blksize=%ld, size=%lld\n", argv[1], g_capacity, stat_buf.st_blksize, siz);
    g_capacity = 21037056;

}   /* End of initialize() */

/* ------------------------------------------------------------------------ */
int main(int argc, char *argv[])
{
    initialize(argc, argv);

    restore_ss_nv_task();

    exit(0);
}   /* End of main() */

/* ------------------------------------------------------------------------ */
/* End of file snapshottest.c */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
