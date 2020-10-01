
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
        lseek(ifd, 0, SEEK_SET);
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
int main(int argc, char **argv)
{
    int             return_status = 0; /* program exit status (main loop). */
    time_t          first_time = 0;
//    int errcnt = 0;

    program_name = argv[0];            /* The name of the running program. */

    parse_args(argc, argv);

    initialize_buffer();

    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
    {
        (void)signal(SIGINT, terminate);
    }

    read_records = 0;
    /* The main reading loop. */
    while (1)
    {
        /* I know that the following will not work for network protocols. */
        if (read(ifd, buffer, block_size) != block_size)
        {
            perror("read");
            return_status = 1;
            break;
        }
        if (read_records == 0)
        {
            first_time = *buffer_time;
        }
        if (*bignumber != read_records + 1 || *buffer_time != first_time)
        {
            fprintf(stderr, "ERROR, read block %llu, value=%llu,  first_time=%ld, current_block_time=%ld\n",
                    read_records + 1, *bignumber, (long int)first_time, (long int)(*buffer_time));
            return_status = 2;
//            if (errcnt++ > 10)
//            {
//                break;
//            }
        }
        read_records++;                /* count did something */
    }                                  /* end of while forever */

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
