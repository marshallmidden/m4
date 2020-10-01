#include <stdio.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>


void get_shared_memory(char *filename, int length, void *addr)
{
    void *start;
    int fd = -1;
    int i;
    struct stat st;
    int loop;

    length = (length + 4096-1) & ~(4096-1);       /* Page size it for i386 */
    fd = open(filename, O_RDWR | O_CREAT, S_IREAD | S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
    if (fd < 0) {
        perror("open get_shared_memory");
        exit(1);
    }
    loop = 0;
  stat_loop:
    loop++;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        exit(1);
    }
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "fstat file (%s) type not regular file\n", filename);
        exit(1);
    }
    if (st.st_size == 0) {                /* must create the file */
        /* Assume page size of 4096 */
        char buf[4096];

        memset (buf, '\0', sizeof(buf));
        if (lseek(fd, length-1, SEEK_SET) < 0) {
            perror("lseek");
            exit(1);
        }
        if (lseek(fd, 0, SEEK_SET) < 0) {
            perror("lseek 0");
            exit(1);
        }
        for (i= 0; i < length; i+=4096) {
            if (write(fd, buf, 4096) < 0) {
                perror("write");
                exit(1);
            }
        }
    } else if (st.st_size != length) {
        if (loop < 200) {
            sleep(1);
            goto stat_loop;
        }
        fprintf(stderr, "%s, wanted size %d, unexpected size (%ld)\n", filename, length, 
                (long)st.st_size);
        exit(1);
    }
    start = mmap(addr, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (start == NULL) {
        perror("mmap null");
        fprintf(stderr, "mmap returned null for address %p file %s\n", addr, filename);
        exit(1);
    }
    if (start == MAP_FAILED) {
        perror("mmap failed");
        fprintf(stderr, "mmap returned MAP_FAILED for address %p file %s\n", addr, filename);
        exit(1);
    }
    if (start != addr) {
        fprintf(stderr, "mmap moved address from %p to %p for file %s\n", addr, start, filename);
        exit(1);
    }
}   /* end of get_shared_memory */


int main()
{
    char buf[BUFSIZ];
    unsigned int where = 0x50000000;

    get_shared_memory("FILE", 1024*1024, (void *)where);

    *(unsigned int *)where = 0;
    *(unsigned int *)(where + 4096) = 0;
    *(unsigned int *)(where + 4096*10) = 0;

    fprintf(stderr, "file opened and mmap-ped ");
    fgets(buf, BUFSIZ, stdin);

    *(unsigned int *)where = 1;

    fprintf(stderr, "data set to 1 ");
    fgets(buf, BUFSIZ, stdin);

    msync((void *)0x50000000, 4, MS_SYNC);

    fprintf(stderr, "msync #1 done ");
    fgets(buf, BUFSIZ, stdin);

    *(unsigned int *)where = 2;
    *(unsigned int *)(where + 4096) = 2;
    *(unsigned int *)(where + 4096*10) = 2;

    fprintf(stderr, "3 different places have data set to 2 ");
    fgets(buf, BUFSIZ, stdin);

    msync((void *)0x50000000, 4, MS_SYNC);

    fprintf(stderr, "msync #2 done ");
    fgets(buf, BUFSIZ, stdin);

    msync((void *)(0x50000000 + 4096), 4, MS_SYNC);

    fprintf(stderr, "msync #3 done ");
    fgets(buf, BUFSIZ, stdin);

    msync((void *)(0x50000000 + 4096*10), 4, MS_SYNC);

    fprintf(stderr, "msync #4 done, return to exit program. ");
    fgets(buf, BUFSIZ, stdin);

    exit(0);

}
