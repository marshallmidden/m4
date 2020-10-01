
/* Exit status of 1 means that it failed for reason -- see printed message. */

#define _GNU_SOURCE
#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>


static char    *program_name;

static unsigned long long read_records; /* Number full records read. */

static int      block_size = 512;      /* Buffer size to use, default 512 */
static int      ifd;                   /* Input file descriptor */

static char    *buffer;                /* For buffer reading */
static unsigned long long *bignumber;  /* The buffer we are reading. */
static time_t  *buffer_time;           /* Time put in buffer here. */
static time_t          first_time = 0;

/* ------------------------------------------------------------------------- */
static void stats(void)
{
    (void)fprintf(stderr, "%s %llu buffers\n", program_name, read_records);
}                                      /* end of stats */

/* ------------------------------------------------------------------------- */
__attribute__ ((noreturn))
static void terminate(int code)
{
    stats();
    exit(code);
}                                      /* end of terminate */

/* ------------------------------------------------------------------------- */
static void parse_args(int argc, char **argv)
{
    int             tmp;
    char           *argon;             /* char on while processing argument */
    char           *input_file = NULL; /* input file name/device */

    /* Parse the three possible arguments. */
    for (tmp = 1; tmp < argc; tmp++)
    {                                  /* process arguments */
        if (strncmp(argv[tmp], "bs=", 3) == 0)
        {                              /* buffer size */
            argon = argv[tmp] + 3;     /* move past the "bs=" characters. */
            block_size = 0;            /* zero the block size */

            /* Get number, possibly followed by one of '[kKbB]'. */
            while (*argon >= '0' && *argon <= '9')
            {
                /* Shift old number by 10 (decimal), and new number added. */
                block_size = (block_size * 10) + (*argon++ - '0');
            }
            if (*argon == 'g' || *argon == 'G')
            {
                block_size *= 1024 * 1024 * 1024;       /* multiple by one disk block */
                argon++;
            }
            else if (*argon == 'm' || *argon == 'M')
            {
                block_size *= 1024 * 1024;      /* multiple by one disk block */
                argon++;
            }
            else if (*argon == 'k' || *argon == 'K')
            {
                block_size *= 1024;    /* multiply by one K. */
                argon++;
            }
            else if (*argon == 'b' || *argon == 'B')
            {
                block_size *= 512;     /* multiple by one disk block */
                argon++;
            }
            else if (*argon != '\0')
            {
                (void)fprintf(stderr, "%s: bad character on bs: %c\n", program_name,
                              *argon);
                exit(1);
            }
            if (*argon != '\0')
            {                          /* If not at end of number... */
                (void)fprintf(stderr, "Bad char in bs= number parsing: %c\n", *argon);
                exit(1);
            }
        }
        else if (strncmp(argv[tmp], "if=", 3) == 0)
        {                              /* input file */
            input_file = argv[tmp] + 3;
        }
        else
        {
            (void)fprintf(stderr, "%s: bad argument: %s\n", program_name, argv[tmp]);
            exit(1);
        }
    }                                  /* end of for() all arguments */

    /* Arguments parsed, open and check file names. */
    if (input_file != NULL && strcmp(input_file, "-") != 0)
    {                                  /* open input */
        ifd = open(input_file, 0666, O_RDONLY | O_DIRECT | O_LARGEFILE);
        if (ifd < 0)
        {                              /* if error, quit */
            perror(input_file);
            exit(1);
        }
        lseek64(ifd, 0, SEEK_SET);
    }
    else
    {
        ifd = dup(0);                  /* or stdin */
        if (ifd < 0)
        {                              /* if error, quit */
            perror(program_name);
            exit(1);
        }
    }
}                                      /* end of parse_args */

/* ------------------------------------------------------------------------- */
static void initialize_buffer(void)
{
    /* Initialize the buffer. */
    buffer = valloc(block_size);       /* Get aligned memory block. */
    if (buffer == (char *)NULL)
    {                                  /* if error */
        (void)fprintf(stderr, "%s: valloc failed\n", program_name);
        exit(1);
    }
    bignumber = (unsigned long long *)buffer;   /* First word in buffer is buffer number. */
    buffer_time = (time_t *) (buffer + sizeof(*bignumber));
}                                      /* end of initialize_buffer */

/* ------------------------------------------------------------------------- */
static int readfrom(unsigned long long block)
{
    long long errchk;
    long long byte = block * 512;

//    fprintf(stderr, "block %llu, %llu\n", block, byte);
    errchk = lseek64(ifd, byte, SEEK_SET);
    if (errchk < 0)
    {
//        perror("lseek64");
        return(-1);
    }

    errchk = read(ifd, buffer, block_size);
    if (errchk == 0)
    {
//        fprintf(stderr, "read - nothing?\n");
        return(-1);
    }
    else if (errchk != block_size)
    {
        perror("read");
        return(-1);
    }
    if (read_records == 0)
    {
        first_time = *buffer_time;
    }
    if (*bignumber != read_records + 1 || *buffer_time != first_time)
    {
        fprintf(stderr, "ERROR, read block %llu, value=%llu,  first_time=%ld, current_block_time=%ld\n",
                read_records + 1, *bignumber, (long int)first_time, (long int)(*buffer_time));
        return(2);
    }
    read_records++;               /* count did something */
    return(0);
}                                      /* end of readfrom */

/* ------------------------------------------------------------------------- */
int main(int argc, char **argv)
{
    int             return_status = 0; /* program exit status (main loop). */
    long long cnt;

    program_name = argv[0];            /* The name of the running program. */

    parse_args(argc, argv);

    initialize_buffer();

    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
    {
        (void)signal(SIGINT, terminate);
    }

    read_records = 0;
/* Starting blocks. */
    if (return_status == 0)
    {
        return_status = readfrom(0);
    }
    if (return_status == 0)
    {
        return_status = readfrom(1);
    }
    if (return_status == 0)
    {
        return_status = readfrom(2);
    }
    /* Power of 2 blocks. */
    if (return_status == 0)
    {
        for (cnt = 4; cnt <= 16*(2*1024*1024*1024LL) ; cnt = cnt * 2)
        {
            return_status = readfrom(cnt - 1);
            if (return_status != 0) break;

            return_status = readfrom(cnt);
            if (return_status != 0) break;

            return_status = readfrom(cnt + 1);
            if (return_status != 0) break;
        }
    }
    /* Here we know that cnt failed, and that cnt/2 worked. Do last block. */
    cnt = lseek64(ifd, 0, SEEK_END);
    if (cnt < 0)
    {
        perror("lseek64 to find end");
    }
    else
    {
        fprintf(stderr, "lseek64 return %llu bytes, %llu blocks\n", cnt, cnt/512);
        cnt = cnt / 512;
        return_status = readfrom(cnt - 1);
        if (return_status == 0)
        {
            return_status = readfrom(cnt);
        }
        if (return_status == 0)
        {
            return_status = readfrom(cnt + 1);
        }
    }



    /* Exit from program, printing out statistics first. */
    stats();
    exit(return_status);               /* DONE! */
}                                      /* end of main */

/* ------------------------------------------------------------------------ */

/* End of file fdd.c */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
