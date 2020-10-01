/* This is the equivalent of "dd if= of= bs=", but it uses 3 circular buffers */
/* to read the data into.  This does not do any conversions whatsoever! */
/*   -v option lists the block number:characters read in block. */
/*   -fill option fills the input block. */
/* if "ef=" is specified, then stdout and stderr are written the same. */
/* if "-20' is given, then 20 circular buffers are use, specify nn. */
/* A file name of '-' means to use standard file descriptor. */
/* Exit status of 1 means that it failed for reason -- see printed message. */
/* ------------------------------------------------------------------------ */
#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <fcntl.h>
// #include "system.h"
#include <string.h>

/* ------------------------------------------------------------------------ */
/* extern int      errno; */
/* ------------------------------------------------------------------------ */
/* 3 appears to work best on a SPARC ELC. */

/* Default number of circular buffers. */
#define	DEFAULTBUFFERS	3
/* ------------------------------------------------------------------------ */
#define NORETURN __attribute__((noreturn))

static struct timeval limited;         /* select timeout value */
static struct timeval *timelimit;
static int      maxfd;                 /* max file descriptor for select */

static long long full_records = 0;     /* number full records read. */
static long long partial_records = 0;  /* number partial records read. */
static int      verbose = 0;           /* not verbose mode. */
static int      fillinput = 0;         /* do not fill input blocks. */
static int      nbuffers = DEFAULTBUFFERS;      /* number of buffers */
static int      usestderr = 0;         /* do not use stderr */

/* ------------------------------------------------------------------------- */
NORETURN static void usage(void)
{
    (void)fprintf(stderr, "Usage: fdd [bs=nn[kKbBgGmM]] [seek=nn[kKbBgGmM]] [if=?] [of=?] [ef=?] -v -nn -fill\n");
    (void)fprintf(stderr, "      nn          Is a number.\n");
    (void)fprintf(stderr, "      [kKbBgGmM]  b=512, k=1024, m=1024*1024, g=1024*1024*1024 (case no matter).\n");
    (void)fprintf(stderr, "    bs=nn         The size of the block/buffer to read.\n");
    (void)fprintf(stderr, "    seek=nn       The size to skip of the input file (if=).\n");
    (void)fprintf(stderr, "    if=?          Input file is ?. If not present, or '-', stdin is used.\n");
    (void)fprintf(stderr, "    of=?          Output file is ?. If not present, or '-', stdout is used.\n");
    (void)fprintf(stderr, "    ef=?          Error file is ?. If not present, or '-', stderr is used.\n");
    (void)fprintf(stderr, "    -v            Verbose output -- print number characters read each buffer.\n");
    (void)fprintf(stderr, "    -fill         If input is truncated before 'bs=', read til buffer filled.\n");
    (void)fprintf(stderr, "    -nn           Specify the number of circular buffers. Default is 3.\n");
    exit(1);
}
/* ------------------------------------------------------------------------- */
static void stats(void)
{
    if (usestderr == 0)
    {
        (void)fprintf(stderr, "%lld full, and %lld partial records processed\n",
                      full_records, partial_records);
    }
}   /* end of stats */

/* ------------------------------------------------------------------------- */
NORETURN static void terminate(int code)
{
    stats();
    exit(code);
}   /* end of terminate */

/* ------------------------------------------------------------------------- */
int main(int argc, char **argv)
{
    long long       block_size = 512;  /* buffer size to use, default 512 */
    long long       seek_size = 0;     /* seek size to use, default 0 */
    char          **buffer;            /* for multiple buffer reading */
    int            *read_chars;        /* number of characters in buffer */
    int            *buffer_empty;      /* anything in buffer */
    int            *stderr_empty;      /* anything in buffer */
    int             ifd;               /* input file descriptor */
    char           *input_file = NULL; /* input file name/device */
    int             iwhich;            /* current input buffer location */
    char           *ibuff;             /* used for input reading */
    int             ofd;               /* output file descriptor */
    char           *output_file = NULL; /* output file name/device */
    int             owhich;            /* current output buffer location */
    char           *obuff;             /* used for output writing */
    char           *stderr_file = NULL; /* output file name/device */
    int             ewhich;            /* current output buffer location */
    char           *ebuff;             /* used for output writing */
    int             efd;               /* stderr file descriptor */
    int             input_done = 0;    /* flag, non-zero means done. */
    int             output_done = 0;   /* flag, non-zero means done. */
    int             stderr_done = 0;   /* flag, non-zero means done. */
    fd_set          ibits;             /* input select fd's */
    fd_set          obits;             /* output select fd's */
    int             n;                 /* select return */
    int             tmp;
    int             temp1;
    int             temp2;
    int             cnt;               /* counter */
    char           *argon;             /* char on while processing argument */
    int             return_status = 0; /* program exit status (main loop). */
    int             block = 1;         /* blocks read (for verbose mode). */

    /* Parse the three possible arguments. */
    for (tmp = 1; tmp < argc; tmp++)
    {                                  /* process arguments */
        if (strncmp(argv[tmp], "bs=", 3) == 0)
        {                              /* buffer size */
            argon = argv[tmp] + 3;     /* move past the "bs=" characters. */
            block_size = 0;            /* zero the block size */

	    /* Get number, possibly followed by one of '[kKbBgGmM]'. */
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
                (void)fprintf(stderr, "%s: bad character on bs: %c\n", argv[0], *argon);
		usage();
            }
            if (*argon != '\0')
            {                          /* If not at end of number... */
                (void)fprintf(stderr, "Bad char in bs= number parsing: %c\n", *argon);
                usage();
            }
        }
        else if (strncmp(argv[tmp], "if=", 3) == 0)
        {                              /* input file */
            input_file = argv[tmp] + 3;
        }
        else if (strncmp(argv[tmp], "of=", 3) == 0)
        {                              /* output file */
            output_file = argv[tmp] + 3;
        }
        else if (strncmp(argv[tmp], "ef=", 3) == 0)
        {                              /* stderr file */
            stderr_file = argv[tmp] + 3;
        }
        else if (strncmp(argv[tmp], "-v", 2) == 0)
        {                              /* verbose mode */
            verbose++;
        }
        else if (strncmp(argv[tmp], "-fill", 5) == 0)
        {                              /* fill input mode */
            fillinput++;
        }
        else if (strncmp(argv[tmp], "-help", 5) == 0 ||
		 strncmp(argv[tmp], "--help", 6) == 0 ||
		 strncmp(argv[tmp], "-h", 2) == 0)
        {
            usage();
        }
        else if (strncmp(argv[tmp], "seek=", 5) == 0)
        {                              /* stderr file */
            argon = argv[tmp] + 5;     /* move past the "seek=" characters. */
            seek_size = 0;             /* zero the seek size */

	    /* Get number, possibly followed by one of '[kKbBgG]'. */
            while (*argon >= '0' && *argon <= '9')
            {
		/* Shift old number by 10 (decimal), and new number added. */
                seek_size = (seek_size * 10) + (*argon++ - '0');
            }
            if (*argon == 'g' || *argon == 'G')
            {
                seek_size *= 1024 * 1024 * 1024;        /* multiple by one disk block */
                argon++;
            }
            else if (*argon == 'm' || *argon == 'M')
            {
                seek_size *= 1024 * 1024;       /* multiple by one disk block */
                argon++;
            }
            else if (*argon == 'k' || *argon == 'K')
            {
                seek_size *= 1024;     /* multiply by one K. */
                argon++;
            }
            else if (*argon == 'b' || *argon == 'B')
            {
                seek_size *= 512;      /* multiple by one disk block */
                argon++;
            }
            else if (*argon != '\0')
            {
                (void)fprintf(stderr, "%s: bad character on seek: %c\n", argv[0], *argon);
                usage();
            }
            if (*argon != '\0')
            {                          /* If not at end of number... */
                (void)fprintf(stderr, "Bad char in bs= number parsing: %c\n", *argon);
                usage();
            }
        }
        else if (*argv[tmp] == '-')
        {                              /* possible number */
            argon = argv[tmp] + 1;     /* move past the '-' character. */
            nbuffers = 0;
            while (*argon >= '0' && *argon <= '9')
            {
		/* Shift old number by 10 (decimal), and new number added. */
                nbuffers = (nbuffers * 10) + (*argon++ - '0');
            }
            if (*argon != '\0')
            {
                (void)fprintf(stderr, "%s: bad character for number of buffers: %c\n",
                              argv[0], *argon);
                usage();
            }
        }
        else
        {
            (void)fprintf(stderr, "%s: bad argument: %s\n", argv[0], argv[tmp]);
            usage();
        }
    }                                  /* end of for() all arguments */

    /* Arguments parsed, open and check file names. */
    if (input_file != NULL && strcmp(input_file, "-") != 0)
    {                                  /* open input */
        ifd = open(input_file, O_RDONLY | O_NONBLOCK);
        if (ifd < 0)
        {                              /* if error, quit */
            perror(input_file);
            exit(1);
        }
    }
    else
    {
        ifd = dup(0);                  /* or stdin */
        if (ifd < 0)
        {                              /* if error, quit */
            perror(argv[0]);
            exit(1);
        }
    }
    if (output_file != NULL && strcmp(output_file, "-") != 0)
    {                                  /* open output */
        ofd = creat(output_file, 0666);
        if (ofd < 0)
        {                              /* if error, quit */
            perror(output_file);
            exit(1);
        }
        if (seek_size != 0)
        {
            lseek(ofd, seek_size, SEEK_SET);
        }
    }
    else
    {
        ofd = dup(1);                  /* or stdout */
        if (ofd < 0)
        {                              /* if error, quit */
            perror(argv[0]);
            exit(1);
        }
    }
    if (stderr_file != NULL && strcmp(stderr_file, "-") != 0)
    {                                  /* open stderr */
        efd = creat(stderr_file, 0666);
        if (efd < 0)
        {                              /* if error, quit */
            perror(stderr_file);
            exit(1);
        }
        usestderr = 1;
        stderr_done = 0;
    }
    else if (stderr_file != NULL && strcmp(stderr_file, "-") == 0)
    {
        efd = dup(2);                  /* or stderr */
        if (efd < 0)
        {                              /* if error, quit */
            perror(argv[0]);
            exit(1);
        }
        usestderr = 1;
        stderr_done = 0;
    }
    else
    {
        efd = 0;                       /* nope */
        usestderr = 0;
        stderr_done = 1;
    }

    /* Initialize the circular buffers. */
    buffer = (char **)malloc((unsigned)nbuffers * sizeof(char **));
    if (buffer == NULL)
    {
        (void)fprintf(stderr, "malloc of buffer (%d) failed.\n", nbuffers);
        exit(1);
    }
    read_chars = (int *)malloc((unsigned)nbuffers * (sizeof(int)));
    if (read_chars == NULL)
    {
        (void)fprintf(stderr, "malloc of read_chars (%lu) failed.\n",
                      nbuffers * sizeof(int));
        exit(1);
    }
    buffer_empty = (int *)malloc((unsigned)nbuffers * (sizeof(int)));
    if (buffer_empty == NULL)
    {
        (void)fprintf(stderr, "malloc of buffer_empty (%lu) failed.\n",
                      nbuffers * sizeof(int));
        exit(1);
    }
    stderr_empty = (int *)malloc((unsigned)nbuffers * (sizeof(int)));
    if (stderr_empty == NULL)
    {
        (void)fprintf(stderr, "malloc of stderr_empty (%lu) failed.\n",
                      nbuffers * sizeof(int));
        exit(1);
    }
    for (tmp = 0; tmp < nbuffers; tmp++)
    {                                  /* get the buffer */
        buffer_empty[tmp] = 0;         /* nothing in buffer */
        stderr_empty[tmp] = 0;         /* nothing in buffer */
        read_chars[tmp] = 0;           /* nothing in buffer */
        buffer[tmp] = valloc(block_size);       /* Get aligned memory block. */
        if (buffer[tmp] == (char *)NULL)
        {                              /* if error */
            (void)fprintf(stderr, "%s: valloc failed\n", argv[0]);
            exit(1);
        }
    }

    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
    {
        (void)signal(SIGINT, terminate);
    }

    /* The main reading and writing loop. */
    iwhich = 0;                        /* Starting buffer for reading. */
    owhich = 0;                        /* Starting buffer for writing. */
    ewhich = 0;                        /* Starting buffer for writing. */

    while (input_done == 0 || output_done == 0 || stderr_done == 0)
    {

/* (void) fprintf(stderr, "start of loop, %d %d %d whichs=%d %d %d\n", input_done,output_done,stderr_done,iwhich,owhich,ewhich); */
        FD_ZERO(&ibits);               /* nothing to read for select, yet */
        FD_ZERO(&obits);               /* nothing to write for select, yet */
        cnt = 0;                       /* count for possible error. */
        if (input_done == 0 &&
            buffer_empty[iwhich] == 0 &&
            ((usestderr != 0 && stderr_empty[iwhich] == 0) || usestderr == 0))
        {
            FD_SET(ifd, &ibits);       /* set to read */
            cnt++;

/* (void) fprintf(stderr, "read from stdin (%d)\n", ifd); */
        }
        if (buffer_empty[owhich] != 0 && output_done == 0)
        {                              /* if something in write buffer */
            FD_SET(ofd, &obits);       /* set to write buffer */
            cnt++;

/* (void) fprintf(stderr, "write to stdout (%d)\n", ofd); */
        }
        if (usestderr == 1)
        {
            if (stderr_empty[ewhich] != 0 && stderr_done == 0)
            {                          /* if something in write buffer */
                FD_SET(efd, &obits);   /* set to write buffer */
                cnt++;

/* (void) fprintf(stderr, "write to stderr (%d)\n", efd); */
            }
        }
        if (cnt == 0)
        {                              /* huh? */
            if (usestderr == 0)
            {
                (void)fprintf(stderr, "HUH, nothing to read or write???\n");
            }
        }
        maxfd = 1;
        if (ifd >= maxfd)
        {                              /* maximum file descriptor for select */
            maxfd = ifd + 1;
        }
        if (ofd >= maxfd)
        {                              /* maximum file descriptor for select */
            maxfd = ofd + 1;
        }
        if (efd >= maxfd)
        {                              /* maximum file descriptor for select */
            maxfd = efd + 1;
        }

        /*     limited.tv_sec = 20; *//* 20 second timeout (slow devices) */
        limited.tv_sec = 60;           /* 60 second timeout (slow devices) */
        limited.tv_usec = 0;
        timelimit = &limited;          /* Some systems clobber this variable */

        n = select(maxfd, &ibits, &obits, (fd_set *) 0, timelimit);

/* (void) fprintf(stderr, "%d = select(%d,%-8.8x,%-8.8x,...)\n", n, maxfd, (int)ibits.fds_bits[0],(int)obits.fds_bits[0]); */
        if (n < 0)
        {                              /* First check for error. */
            if (errno == EINTR)
            {
                continue;              /* if interrupted, loop */
            }
            perror("select error, ignored");
            continue;                  /* ignore other errors(broken systems) */
        }
        if (n == 0)
        {                              /* select timeout */
            if (usestderr == 0)
            {
                (void)fprintf(stderr, "select timeout\n");
            }
            continue;                  /* Nothing to do upon a timeout. */
        }
        cnt = 0;

/* INPUT ---------- */
        if (FD_ISSET(ifd, &ibits))
        {                              /* If correct input bit set. */
            cnt++;                     /* count did something */
            ibuff = buffer[iwhich] + read_chars[iwhich];
            temp1 = (int)block_size - read_chars[iwhich];
            temp2 = read(ifd, ibuff, temp1);

/* (void)fprintf(stderr,"%d = read(%d, ibuff, %d)\n", temp2, ifd, temp1); */
            if (temp2 <= -1)
            {                          /* if read error */
                if (usestderr == 0)
                {
                    (void)fprintf(stderr, "%s: ", argv[0]);
                    perror("read");
                }
                return_status = 1;
                break;
            }
            read_chars[iwhich] += temp2;
            if (verbose != 0)
            {                          /* print how many character read. */
                if (fillinput == 0)
                {
                    (void)fprintf(stderr, "%d:%d\n", block, temp2);
                }
                else
                {
                    (void)fprintf(stderr, "%d:%d -> %d\n",
                                  block, temp2, read_chars[iwhich]);
                }
            }
            if (temp2 == 0)
            {                          /* end of file reached */

/* (void)fprintf(stderr,"eof reached\n"); */
                input_done = 1;
            }
            if (fillinput == 0 || read_chars[iwhich] == (int)block_size || temp2 == 0)
            {
                if (read_chars[iwhich] != (int)block_size)
                {

/* (void)fprintf(stderr,"partial record increment\n"); */
                    partial_records++; /* number of partial records */
                }
                else
                {

/* (void)fprintf(stderr,"full record increment\n"); */
                    full_records++;    /* number of full records */
                }
                buffer_empty[iwhich] = 1;       /* something in buffer */
                stderr_empty[iwhich] = 1;       /* something in buffer */
                iwhich++;              /* to next buffer, circularly */
                block++;
                if (iwhich >= nbuffers)
                {                      /* wrap if necessary */
                    iwhich = 0;
                }
            }
            else
            {

/* (void)fprintf(stderr,"partial record read\n"); */
            }
        }                              /* end of reading section */

/* OUTPUT ---------- */
        if (FD_ISSET(ofd, &obits))
        {                              /* If correct output bit set. */
            cnt++;                     /* count did something */
            obuff = buffer[owhich];

/* I know that the following will not work for network protocols. */

/* (void)fprintf(stderr,"writing characters to stdout\n"); */
            if (write(ofd, obuff, (int)read_chars[owhich]) != read_chars[owhich])
            {
                perror("write stdout");
                return_status = 1;
                break;
            }
            if (usestderr == 1 && owhich == ewhich)
            {

/* (void)fprintf(stderr,"do not clear read count\n"); */
                ;                      /* Don't clear count. */
            }
            else
            {

/* (void)fprintf(stderr,"clear read count\n"); */
                read_chars[owhich] = 0; /* mark nothing in buffer */
            }
            buffer_empty[owhich] = 0;
            owhich++;                  /* to next buffer, circularly */
            if (owhich >= nbuffers)
            {                          /* wrap if necessary */
                owhich = 0;
            }
        }                              /* end of writing section */

/* STDERR ---------- */
        if (usestderr == 1 && FD_ISSET(efd, &obits))
        {
            cnt++;                     /* count did something */
            ebuff = buffer[ewhich];

	    /* I know that the following will not work for network protocols. */
/* (void)fprintf(stderr,"writing characters to stderr\n"); */
            if (write(efd, ebuff, (int)read_chars[ewhich]) != read_chars[ewhich])
            {
                perror("write stderr");
                return_status = 1;
                break;
            }
            if (owhich == ewhich)
            {

/* (void)fprintf(stderr,"do not clear read count\n"); */
                ;                      /* Don't clear count. */
            }
            else
            {

/* (void)fprintf(stderr,"clear read count\n"); */
                read_chars[ewhich] = 0; /* mark nothing in buffer */
            }
            stderr_empty[ewhich] = 0;
            ewhich++;                  /* to next buffer, circularly */
            if (ewhich >= nbuffers)
            {                          /* wrap if necessary */
                ewhich = 0;
            }
        }                              /* end of writing section */

	/* ------------------------------------------------------------------------ */
        if (cnt != n)
        {                              /* Check did what select told us. */
            if (usestderr == 0)
            {
                (void)fprintf(stderr, "bits set (%d) not equal to %d\n", cnt, n);
            }
        }

	/* ------------------------------------------------------------------------ */
        if (input_done == 1)
        {
            if (read_chars[owhich] == 0 && buffer_empty[owhich] == 0)
            {
                output_done = 1;
            }
            if (usestderr == 1 && read_chars[ewhich] == 0 && stderr_empty[ewhich] == 0)
            {
                stderr_done = 1;
            }
        }
    }                                  /* end of while forever */

    /* Exit from program, printing out statistics first. */
    stats();
    exit(return_status);               /* DONE! */
}   /* end of main */

/* End of file fdd.c */
/* ------------------------------------------------------------------------- */
