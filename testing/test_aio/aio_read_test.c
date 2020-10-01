#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <aio.h>

#define tmpfname        "aio_read_test_file"
#define BUF_SIZE        111

int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
    int             err;
    int             ret;
    unsigned char   buf[BUF_SIZE];
    unsigned char   check[BUF_SIZE + 1];
    int             fd;
    struct aiocb    aiocb;
    int             i;

    unlink(tmpfname);
    fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        perror("open");
        exit(1);
    }

//    unlink(tmpfname);

    for (i = 0; i < BUF_SIZE; i++)
    {
        buf[i] = i;
    }
    check[BUF_SIZE] = 1;

    memset(&aiocb, 0, sizeof(struct aiocb));
    aiocb.aio_fildes = fd;
    aiocb.aio_buf = check;
    aiocb.aio_nbytes = BUF_SIZE;

    if (write(fd, buf, BUF_SIZE) != BUF_SIZE)
    {
        perror("write");
        close(fd);
        exit(2);
    }

    memset(check, 0xaa, BUF_SIZE + 1);

    if (aio_read(&aiocb) == -1)
    {
        perror("aio_read");
        close(fd);
        exit(3);
    }

    /* Wait until end of transaction */
    while ((err = aio_error(&aiocb)) == EINPROGRESS) ;

    err = aio_error(&aiocb);
    ret = aio_return(&aiocb);

    if (err != 0)
    {
        fprintf(stderr, "Error at aio_error() : %s\n", strerror(err));
        close(fd);
        exit(4);
    }

    if (ret != BUF_SIZE)
    {
        fprintf(stderr, "Error at aio_return() - ret (%d) != (%d)\n", ret, BUF_SIZE);
        close(fd);
        exit(5);
    }

    /* check it */
    for (i = 0; i < BUF_SIZE; i++)
    {
        if (buf[i] != check[i])
        {
            fprintf(stderr, "Read values are corrupted at %d (0x%02x != 0x%02x)\n", i, buf[i], check[i]);
            exit(6);
        }
    }

    close(fd);
    printf("Test PASSED\n");
    return 0;
}
