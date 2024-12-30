/* ------------------------------------------------------------------------- */
#define BLOCKSIZE    512        /* 512 bytes in a block */

#define num_blocks  2048        /* one mb in blocks of 512 bytes */
#define blk_offset     0        /* Start at block 0 */

/* ------------------------------------------------------------------------- */
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#define _XOPEN_SOURCE 500
#define __USE_UNIX98
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <inttypes.h>

/* ------------------------------------------------------------------------- */
// #define NO_TSC                         // No tsc register usable for timer on AMD development machine.
#ifndef NO_TSC
/* Free running bus clock timer on Intel X86 (Xeon) processors -- not AMD! */
//-- #define get_tsc()       ({ unsigned long long __scr; __asm__ __volatile__("rdtsc" : "=A" (__scr)); __scr;})

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
static inline unsigned long long rdtsc(void)
{
    unsigned int upper;
    unsigned int lower;
    __asm__ __volatile__( "rdtsc" : "=a" (lower), "=d" (upper));
    return (((unsigned long long)upper << 32) | lower);
}

static inline unsigned long long get_tsc(void)
{
    unsigned long long __scr;
    __scr = rdtsc();
    return __scr;
}
//----------------------------------------------------------------------------

#else  /* NO_TSC */
#define get_tsc()   123456789ULL
#endif /* NO_TSC */
#define FORCE_WRITE_BARRIER __asm__ __volatile__ ("mfence": : : "memory")

/* ------------------------------------------------------------------------- */
/* Arguments are decoded into these variables. */
static const char *file1;                       /* argument 1, output file name/device */
static unsigned long long file_size;            /* argument 2, size of file */
static unsigned long long value;                /* argument 3, input unique value for write run */
static unsigned int sleepmicroseconds =1000;    /* argument 4, microseconds to sleep between reads/writes. */
/*  0: Get character from input line -- */
/*     w = write pattern, then get next input line */
/*     r = read and verify pattern, then get next input line */
/*     a = write, then read -- repeat both forever. */
/*     W = write pattern, repeat forever. */
/*     R = read and verify pattern, repeat forever. */
/*     A = write, then read forever (only read). */
/*     q/Q/EOF/empty-line = quit. */
/*     Number = Sleep for Number seconds and get next input line. */
/*  1: Write, then continue to 2 next time. */
/*  2: Read, then continue to 3 next time. */
/*  3: Sleep 60 seconds, read, then continue to 4 next time. */
/*  4: Sleep 60 seconds, read, then continue to 5 next time. */
/*  5: quit -- exit program. */
static int TEST = 0;                            /* argument 5, Run in TEST mode (from above list). */
static unsigned long long print_blocks = 1022;  /* argument 6, how many blocks to write/read before printing a period. */

/* ------------------------------------------------------------------------- */
static int      fd1;
static int      flag_written = 0;
static unsigned long long blocks_in_file;
static unsigned long long blks_from_front;

/* ------------------------------------------------------------------------- */
/* 32 bytes of pattern that goes to the block. */
struct blk_pattern
{
    unsigned long long block_number;   /* 1-8  The block (512 byte) number. */
    unsigned long long time_started;   /* 8-16 The tsc register when program starts. */
    unsigned long long value;          /* 17-24 Value increments each time blocks written. */
    unsigned long long tilde_block_number;      /* 25-32 Complement of block numner. */
};

/* There are 16*32=512 bytes in a block. */
#define DEF_LIMIT 16
struct blk_array
{
    struct blk_pattern p[DEF_LIMIT];
};

static struct blk_array *blk_write = NULL;      /* The blocks to write (malloc-ed) from here. */
static struct blk_array *blk_read = NULL;       /* The blocks read (malloc-ed) here. */

/* Time values, set during initialization. */
static volatile unsigned long long time_start;  /* When program starts -- for multiple runs. */
static volatile unsigned long long time_diff_one_sec;    /* Approx. how long a second is. */
static volatile unsigned long long time_diff_big;        /* Crash if too long ... > 90 seconds */

/* ------------------------------------------------------------------------- */
/* Set time values (approximately). */
static void time_init(void)
{
    volatile unsigned long long time_first;
    volatile unsigned long long time_second;

    FORCE_WRITE_BARRIER;
    time_start = get_tsc();
    FORCE_WRITE_BARRIER;

    system("sleep 1");

    FORCE_WRITE_BARRIER;
    time_first = get_tsc();
    FORCE_WRITE_BARRIER;
    time_diff_one_sec = time_first - time_start;
    FORCE_WRITE_BARRIER;

    /* Check again. */
    system("sleep 1");

    FORCE_WRITE_BARRIER;
    time_second = get_tsc();
    FORCE_WRITE_BARRIER;
    if (time_diff_one_sec > (time_second - time_first)) {
        time_diff_one_sec = time_second - time_first;
        FORCE_WRITE_BARRIER;
        time_start = time_first;
        FORCE_WRITE_BARRIER;
    }

    FORCE_WRITE_BARRIER;
//    time_diff_big = time_diff_one_sec * 90;
    time_diff_big = time_diff_one_sec * 180;
    FORCE_WRITE_BARRIER;

    fprintf(stdout, "time_start=0x%16.16llx time_first=0x%16.16llx time_second=0x%16.16llx time_diff_one_sec=0x%16.16llx\n",
            time_start, time_first, time_second, time_diff_one_sec);
    fprintf(stdout, "time_start=%18llu time_first=%18llu time_second=%18llu time_diff_one_sec=%18llu\n",
            time_start, time_first, time_second, time_diff_one_sec);
    fflush(stdout);
}   /* end of time_init */

/* ------------------------------------------------------------------------- */
static unsigned long long get_ull_sized(char *argon, int n)
{
    char *start = argon;
    unsigned long long number = 0;

    /* Get number, possibly followed by one of '[kKbBmMgB]'. */
    while (*argon >= '0' && *argon <= '9')
    {
        /* Shift old number by 10 (decimal), and new number added. */
        number = (number * 10) + (unsigned long long)(*argon++ - '0');
    }
    if (*argon == 't' || *argon == 'T')
    {
        number *= 1024ULL * 1024 * 1024 * 1024;        /* multiply by one terabyte. */
        argon++;
    }
    else if (*argon == 'g' || *argon == 'G')
    {
        number *= 1024 * 1024 * 1024;        /* multiply by one gigabyte. */
        argon++;
    }
    else if (*argon == 'm' || *argon == 'M')
    {
        number *= 1024 * 1024;      /* multiply by one megabyte. */
        argon++;
    }
    else if (*argon == 'k' || *argon == 'K')
    {
        number *= 1024;             /* multiply by one kilobyte. */
        argon++;
    }
    else if (*argon == 'b' || *argon == 'B')
    {
        number *= BLOCKSIZE;        /* multiple by one disk block. */
        argon++;
    }
    if (*argon != '\0')
    {                                  /* If not at end of number... */
        (void)fprintf(stdout, "Bad char in length number parsing: '%c' in argument %d '%s'\n", *argon, n, start);
        fflush(stdout);
        exit(1);
    }
    return(number);
}   /* end of get_ull_sized */

/* ------------------------------------------------------------------------- */
/* Parse possible arguments. */
static void parse_arguments(int argc, char **argv)
{
    int             sleeptime = 0;

    if (argc < (2 + 1) || argc > (6 + 1))
    {
        (void)fprintf(stdout, "Need 2 arguments: file, size(bytes)\n");
        (void)fprintf(stdout, "  Optional argument 3: Value to put in block.\n");
        (void)fprintf(stdout, "  Optional argument 4: microseconds to sleep between reads/writes.\n");
        (void)fprintf(stdout, "  Optional argument 5: automated test to run (default 0, line input)\n");
        (void)fprintf(stdout, "     0: Get character from input line --\n");
        (void)fprintf(stdout, "        w = write pattern, then get next input line\n");
        (void)fprintf(stdout, "        r = read and verify pattern, then get next input line\n");
        (void)fprintf(stdout, "        a = write, then read -- repeat both forever.\n");
        (void)fprintf(stdout, "        W = write pattern, repeat forever.\n");
        (void)fprintf(stdout, "        R = read and verify pattern, repeat forever.\n");
        (void)fprintf(stdout, "        A = write, then read forever (only read).\n");
        (void)fprintf(stdout, "        q/Q/EOF/empty-line = quit.\n");
        (void)fprintf(stdout, "        Number = Sleep for Number seconds and get next input line.\n");
        (void)fprintf(stdout, "     1: Write, then continue to 2 next time.\n");
        (void)fprintf(stdout, "     2: Read, then continue to 3 next time.\n");
        (void)fprintf(stdout, "     3: Sleep 60 seconds, read, then continue to 4 next time.\n");
        (void)fprintf(stdout, "     4: Sleep 60 seconds, read, then continue to 5 next time.\n");
        (void)fprintf(stdout, "     5: quit -- exit program.\n");
        (void)fprintf(stdout, "  Optional argument 6: how many blocks to write/read before printing a period.\n");
        fflush(stdout);
        exit(1);
    }

    /* Get first argument, file name. */
    file1 = argv[1];
    if (file1[0] == '\0')
    {
        (void)fprintf(stdout, "File name (path) must be provided\n");
        fflush(stdout);
        exit(1);
    }
    if ((bcmp(file1, "/dev/sda", 9) == 0) ||
        (bcmp(file1, "/dev/sda1", 10) == 0) ||
        (bcmp(file1, "/dev/sda2", 10) == 0) ||
        (bcmp(file1, "/dev/sda3", 10) == 0) ||
        (bcmp(file1, "/dev/sda4", 10) == 0) ||
        (bcmp(file1, "/dev/sdb", 9) == 0) ||
        (bcmp(file1, "/dev/sdb1", 10) == 0) ||
        (bcmp(file1, "/dev/sdb2", 10) == 0) ||
        (bcmp(file1, "/dev/sdb3", 10) == 0) ||
        (bcmp(file1, "/dev/sdb4", 10) == 0))
    {
        (void)fprintf(stdout, "I caught you trying to overwrite the operating system!\n");
        fflush(stdout);
        exit(1);
    }

    /* Get second argument, file size */
    file_size = get_ull_sized(argv[2], 2);
    if (file_size == 0)
    {
        (void)fprintf(stdout, "file size must be greater than 0 (%llu)\n", file_size);
        fflush(stdout);
        exit(1);
    }

    /* Get third argument, unique value for block. */
    if (argc == 3 + 1)
    {
        value = get_ull_sized(argv[3], 3);
    }

    /* Get fourth argument, microseconds to sleep between reads/writes. */
    if (argc == 4 + 1)
    {
        sleeptime = atoi(argv[4]);
        if (sleeptime > 0)
        {
            sleepmicroseconds = (unsigned int)sleeptime;
        }
    }

    /* Get fifth argument, TEST -- which to do. */
    if (argc == 5 + 1)
    {
        /*  0: Get character from input line -- */
        /*     w = write pattern, then get next input line */
        /*     r = read and verify pattern, then get next input line */
        /*     a = write, then read -- repeat both forever. */
        /*     W = write pattern, repeat forever. */
        /*     R = read and verify pattern, repeat forever. */
        /*     A = write, then read forever (only read). */
        /*     q/Q/EOF/empty-line = quit. */
        /*     Number = Sleep for Number seconds and get next input line. */
        /*  1: Write, then continue to 2 next time. */
        /*  2: Read, then continue to 3 next time. */
        /*  3: Sleep 60 seconds, read, then continue to 4 next time. */
        /*  4: Sleep 60 seconds, read, then continue to 5 next time. */
        /*  5: quit -- exit program. */
        TEST = atoi(argv[5]);
    }

    /* Get sixth argument, number of blocks to write/read before printing a period. */
    if (argc == 6 + 1)
    {
        print_blocks = get_ull_sized(argv[6], 6);
        print_blocks = (print_blocks < 1) ? 1 : print_blocks;
    }
}   /* end of parse_arguments */

/* ------------------------------------------------------------------------- */
static void set_block_pattern(struct blk_array *s, unsigned long long i)
{
    int             j;

    for (j = 0; j < DEF_LIMIT; j++)
    {
        s->p[j].block_number = i;
        s->p[j].time_started = time_start;
        s->p[j].value = value;
        s->p[j].tilde_block_number = ~i;
    }
}   /* end of set_block_pattern */

/* ------------------------------------------------------------------------ */
static void blk_pwrite(void *buffer, unsigned long long nblks, unsigned long long offsetblk)
{
    volatile unsigned long long time_before;
    volatile unsigned long long time_next;
    ssize_t         lth;

    FORCE_WRITE_BARRIER;
    time_before = get_tsc();
    FORCE_WRITE_BARRIER;

    nblks = 1;
    lth = pwrite64(fd1, buffer, (size_t)(nblks * BLOCKSIZE), (off_t) (offsetblk * BLOCKSIZE));      /* WRITE */
    if (lth != (ssize_t) (BLOCKSIZE * nblks))
    {
        perror("write");
        fprintf(stdout, "write wanted to write %lld bytes, did %zd @ block %lld (file byte %lld)\n",
                BLOCKSIZE * nblks, lth, offsetblk, offsetblk * BLOCKSIZE);
        fflush(stdout);
        exit(1);
    }

    FORCE_WRITE_BARRIER;
    time_next = get_tsc();
    FORCE_WRITE_BARRIER;
    if (time_before + 2*time_diff_one_sec < time_next)
    {
        fprintf(stdout, "\nwrite %lld bytes @ block %lld (file byte %lld)\n",
                BLOCKSIZE * nblks, offsetblk, offsetblk * BLOCKSIZE);
        fflush(stdout);
        if (time_before + time_diff_big < time_next)
        {
            fprintf(stdout, "write time difference super large, %.9f seconds = (%llu - %llu)/%llu\n",
                    (double)(time_next - time_before) / (double)time_diff_one_sec,
                    time_next, time_before, time_diff_one_sec);
            fflush(stdout);
//            exit(1);
        }
        fprintf(stdout, "write time difference large, %.9f seconds = (%llu - %llu)/%llu\n",
                (double)(time_next - time_before) / (double)time_diff_one_sec,
                time_next, time_before, time_diff_one_sec);
        fflush(stdout);
    }

    blks_from_front += nblks;
    while (blks_from_front >= print_blocks)
    {
        fprintf(stdout, ".");
        fflush(stdout);
        blks_from_front -= print_blocks;
    }

    if (sleepmicroseconds > 1)
    {
        usleep(sleepmicroseconds);
    }
// fprintf(stdout, " %lld[%lld]", offsetblk, nblks);
// fflush(stdout);
}   /* end of blk_pwrite */

/* ------------------------------------------------------------------------- */
void write_blocks(unsigned long long boff, unsigned long long nblks);
void write_blocks(unsigned long long boff, unsigned long long nblks)
{
    unsigned long long i;
    unsigned long long blkon;
    unsigned long long lstblkwrtn = 0;

fprintf(stdout, "write_blocks\n");
fflush(stdout);

    fd1 = open(file1, O_TRUNC | O_CREAT | O_RDWR, 0666);
    if (fd1 < 0)
    {                                  /* if error, quit */
        perror(file1);
        exit(1);
    }
fprintf(stdout, "open done\n");
fflush(stdout);

    flag_written = 1;                  /* Flag if we wrote a pattern. */
    blks_from_front = 0;               /* Used in blk_pwrite for period printing. */

    /* Write blocks before multi-block writes. */
    if (boff > 0)
    {
// fprintf(stdout, "write before blocks\n");
// fflush(stdout);
        /* for (i = 0; i < boff; i++) */
        i = 0;
        {
            set_block_pattern(blk_write + i, i);
        }
        blk_pwrite(blk_write, boff, (unsigned long long)0);
// fprintf(stdout, "\n");
// fflush(stdout);
    }

    /* Write multi-blocks. */
// fprintf(stdout, "write multi-blocks\n");
// fflush(stdout);
    for (blkon = boff; blkon < (blocks_in_file - (nblks - 1)); blkon += nblks)
    {
        /* for (i = 0; i < nblks; i++) */
        i = 0;
        {
            set_block_pattern(blk_write + i, blkon + i);
        }
        blk_pwrite(blk_write, nblks, blkon);
        lstblkwrtn = blkon;            /* Set for after writing, no subtract nor rely on for storing increment. */
    }
// fprintf(stdout, "\n");
// fflush(stdout);

    /* Write blocks after multi-block writes. */
    if ((lstblkwrtn + nblks) < blocks_in_file)
    {
// fprintf(stdout, "write after blocks\n");
// fflush(stdout);
        /* for (i = 0; i < (blocks_in_file - (lstblkwrtn + nblks)); i++) */
        i = 0;
        {
            set_block_pattern(blk_write + i, lstblkwrtn + nblks + i);
        }
        blk_pwrite(blk_write, blocks_in_file - (lstblkwrtn + nblks), lstblkwrtn + nblks);
    }
fprintf(stdout, "\n");
fflush(stdout);
    close(fd1);
fprintf(stdout, "close done\n");
fflush(stdout);
}   /* end of write blocks */

/* ------------------------------------------------------------------------ */
static void check_block_pattern(struct blk_array *s, unsigned long long i)
{
    int             j;
    int             stop = 0;

    /* First time, if we do a read before write -- need to set values for checking. */
    if (flag_written == 0)
    {
        /* Use what we read from disk. */
        time_start = blk_read->p[0].time_started;
        value = blk_read->p[0].value;
        flag_written = 1;               /* Only set once. */
    }

    for (j = 0; j < DEF_LIMIT; j++)
    {
        if (s->p[j].block_number != i || s->p[j].time_started != time_start ||
            s->p[j].value != value || s->p[j].tilde_block_number != ~i)
        {
            fprintf(stdout, "Block %llu entry %d (0x%016llx/0x%016llx/0x%016llx/0x%016llx != 0x%016llx/0x%016llx/0x%016llx/0x%016llx)\n",
                    i, j, 
                    blk_read->p[j].block_number, blk_read->p[j].time_started, blk_read->p[j].value, blk_read->p[j].tilde_block_number,
                    i, time_start, value, ~i);
            fflush(stdout);
            stop = 1;
        }
    }
    if (stop == 1)
    {
        exit(1);
    }
}   /* end of check_block_pattern */

/* ------------------------------------------------------------------------ */
static void blk_pread(void *buffer, unsigned long long nblks, unsigned long long offsetblk)
{
    volatile unsigned long long time_before;
    volatile unsigned long long time_next;
    ssize_t         lth;

    nblks = 1;
    FORCE_WRITE_BARRIER;
    time_before = get_tsc();
    FORCE_WRITE_BARRIER;

    lth = pread64(fd1, buffer, (size_t)(nblks * BLOCKSIZE), (off_t) (offsetblk * BLOCKSIZE));      /* READ */
    if (lth != (ssize_t) (BLOCKSIZE * nblks))
    {
        perror("read");
        fprintf(stdout, "read wanted to read %lld bytes, did %zd @ block %lld (file byte %lld)\n",
                BLOCKSIZE * nblks, lth, offsetblk, offsetblk * BLOCKSIZE);
        fflush(stdout);
        exit(1);
    }

    FORCE_WRITE_BARRIER;
    time_next = get_tsc();
    FORCE_WRITE_BARRIER;
    if (time_before + 2*time_diff_one_sec < time_next)
    {
        fprintf(stdout, "\nread %lld bytes @ block %lld (file byte %lld)\n",
                BLOCKSIZE * nblks, offsetblk, offsetblk * BLOCKSIZE);
        fflush(stdout);
        if (time_before + time_diff_big < time_next)
        {
            fprintf(stdout, "read time difference super large, %llu seconds\n",
                    (time_next - time_before) / time_diff_one_sec);
            fflush(stdout);
//            exit(1);
        }
        fprintf(stdout, "read time difference large, %llu seconds\n",
                (time_next - time_before) / time_diff_one_sec);
        fflush(stdout);
    }

    blks_from_front += nblks;
    while (blks_from_front >= print_blocks)
    {
        fprintf(stdout, ".");
        fflush(stdout);
        blks_from_front -= print_blocks;
    }

    if (sleepmicroseconds > 1)
    {
        usleep(sleepmicroseconds);
    }
// fprintf(stdout, " %lld[%lld]", offsetblk, nblks);
// fflush(stdout);
}   /* end of blk_pread */

/* ------------------------------------------------------------------------ */
void check_written(unsigned long long boff, unsigned long long nblks);
void check_written(unsigned long long boff, unsigned long long nblks)
{
    unsigned long long i;
    unsigned long long blkon;
    unsigned long long lstblkread = 0;

fprintf(stdout, "check_written\n");
fflush(stdout);

    fd1 = open(file1, O_RDWR, 0666);
    if (fd1 < 0)
    {                                  /* if error, quit */
        perror(file1);
        exit(1);
    }
fprintf(stdout, "open done\n");
fflush(stdout);

    blks_from_front = 0;               /* Used in blk_pread for period printing. */

    /* Read blocks before multi-block reads. */
    if (boff > 0)
    {   
// fprintf(stdout, "read before blocks\n");
// fflush(stdout);
        blk_pread(blk_read, boff, (unsigned long long)0);
        /* for (i = 0; i < boff; i++) */
        i = 0;
        {
            check_block_pattern(blk_read + i, i);
        }
// fprintf(stdout, "\n");
// fflush(stdout);
    }

    /* Read multi-blocks. */
// fprintf(stdout, "read multi-blocks\n");
// fflush(stdout);
    for (blkon = boff; blkon < (blocks_in_file - (nblks - 1)); blkon += nblks)
    {
        blk_pread(blk_read, nblks, blkon);
        /* for (i = 0; i < nblks; i++) */
        i = 0;
        {
            check_block_pattern(blk_read + i, blkon + i);
        }
        lstblkread = blkon;            /* Set for after reading, no subtract nor rely on for storing increment. */
    }
// fprintf(stdout, "\n");
// fflush(stdout);

    /* Read blocks left over after multi-block reads. */
    if ((lstblkread + nblks) < blocks_in_file)
    {
// fprintf(stdout, "read after blocks\n");
// fflush(stdout);
        blk_pread(blk_read, blocks_in_file - (lstblkread + nblks), lstblkread + nblks);
        /* for (i = 0; i < (blocks_in_file - (lstblkread + nblks)); i++) */
        i = 0;
        {
            check_block_pattern(blk_read + i, lstblkread + nblks + i);
        }
    }
fprintf(stdout, "\n");
fflush(stdout);
    close(fd1);
fprintf(stdout, "close done\n");
fflush(stdout);
}   /* end of check_written */

/* ------------------------------------------------------------------------- */
static char GET_NEXT(void)
{
    char            line[BUFSIZ];

    switch (TEST)
    {
        case 0:
            while (1)
            {
                fwrite("[r/w/a/q/R/W/A] > ", 18, 1, stdout);
                line[0] = '\0';
                fgets(line, sizeof(line), stdin);
                if (line[0] > '0' && line[0] <= '9')
                {
                    unsigned long int sleeptime = strtoul(line, NULL, 10);
                    sleep((unsigned int)sleeptime);
                }
                else
                {
                    return(line[0]);
                }
            }

        case 1:
            TEST++;
            return('w');

        case 2:
            TEST++;
            return('r');

        case 3:
            sleep(60);
            TEST++;
            return('r');

        case 4:
            sleep(60);
            TEST++;
            return('r');

        case 5:
            TEST++;
            return('q');

        case 6:
            exit(1);
    }
    return('q');
}   /* end of GET_NEXT */

/* ------------------------------------------------------------------------- */
int main(int argc, char **argv)
{
    char            ch = 'q';

    parse_arguments(argc, argv);

    time_init();

    blocks_in_file = (unsigned long long)(file_size / BLOCKSIZE);

    blk_write = malloc((size_t)(BLOCKSIZE));
    blk_read = malloc((size_t)(BLOCKSIZE));
    if (blk_write == NULL || blk_read == NULL)
    {
        perror("malloc");
        exit(1);
    }

    while (1)
    {
        fprintf(stdout, "value=%lld (0x%016llx) ", value, value);
        fflush(stdout);
        ch = GET_NEXT();
        if (ch == '\0' || ch == 'q' || ch == 'Q' || ch == '\n')
        {
            fprintf(stdout, "quit\n");
            fflush(stdout);
            break;
        }
        do
        {
            /* Do write. */
            if (ch == 'a' || ch == 'A' || ch == 'w' || ch == 'W')
            {
                value++;
                write_blocks((unsigned long long)blk_offset, (unsigned long long)num_blocks);

                if (ch == 'A')
                {
                    ch = 'R';       /* Only one write for capital A. */
                }
                /* Do a slight pause. */
                if (ch == 'a' || ch == 'W')
                {
                    system("sleep 1");
                }
            }

            /* Do read. */
            if (ch == 'a' || ch == 'r' || ch == 'R')
            {
                check_written((unsigned long long)blk_offset, (unsigned long long)num_blocks);

                /* Do a slight pause. */
                if (ch == 'a' || ch == 'R')
                {
                    system("sleep 1");
                }
            }
        } while (ch == 'a' || ch == 'W' || ch == 'R');
    }

    exit(0);
}   /* end of main */

/* ------------------------------------------------------------------------- */
/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
