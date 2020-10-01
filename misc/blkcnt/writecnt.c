
/* Exit status of 1 means that it failed for reason -- see printed message. */
#define _GNU_SOURCE
#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <string.h>


static char    *program_name;

static unsigned long long write_records;        /* number full records written. */

static int      block_size = 512;      /* buffer size to use, default 512 */
static int      ofd;                   /* output file descriptor */

static char    *buffer;                /* for buffer writing */
static unsigned long long *bignumber;  /* The buffer we are writing. */
static time_t  *buffer_time;           /* Time put in buffer here. */

/* ------------------------------------------------------------------------- */
static void stats(void)
{
    (void)fprintf(stderr, "%s %llu buffers\n", program_name, write_records);
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
    char           *output_file = NULL; /* output file name/device */

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
        else if (strncmp(argv[tmp], "of=", 3) == 0)
        {                              /* output file */
            output_file = argv[tmp] + 3;
        }
        else
        {
            (void)fprintf(stderr, "%s: bad argument: %s\n", program_name, argv[tmp]);
            exit(1);
        }
    }                                  /* end of for() all arguments */

    /* Arguments parsed, open and check file names. */
    if (output_file != NULL && strcmp(output_file, "-") != 0)
    {                                  /* open output */
        ofd = open(output_file, 0666, O_WRONLY | O_DIRECT | O_LARGEFILE);
        if (ofd < 0)
        {                              /* if error, quit */
            perror(output_file);
            exit(1);
        }
        lseek(ofd, 0, SEEK_SET);
    }
    else
    {
        ofd = dup(1);                  /* or stdout */
        if (ofd < 0)
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
    time(buffer_time);
}                                      /* end of initialize_buffer */

/* ------------------------------------------------------------------------- */
int main(int argc, char **argv)
{
    int             ret;
    int             return_status = 0; /* program exit status (main loop). */
    unsigned long long cnt;

    program_name = argv[0];            /* The name of the running program. */

    parse_args(argc, argv);

    initialize_buffer();

    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
    {
        (void)signal(SIGINT, terminate);
    }

    write_records = 0;
    cnt = 0;
    /* The main writing loop. */
    while (1)
    {
        *bignumber = write_records + 1;
        /* I know that the following will not work for network protocols. */
        ret = write(ofd, buffer, block_size);
        if (ret == 0)
        {
            fprintf(stderr, "write - disk full\n");
            return_status = 1;
            break;
        }
        else if (ret != block_size)
        {
            perror("write");
            return_status = 1;
            break;
        }
        write_records++;               /* count did something */
//        if ((write_records % 2*1024*1024) == 0)
//        {
//          fprintf(stderr, ".");
//        }
        cnt++;
        if (cnt > 1024*1024)
        {
            fprintf(stderr, ".");
            cnt = 0;
        }
    }                                  /* end of while forever */

    ret = fsync(ofd);
    if (ret < 0)
    {
        perror("fsync");
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
