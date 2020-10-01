
/* if "-20' is given, then 20 sectors are written, specify nn. */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <values.h>
#include <string.h>

extern int      errno;


/* ------------------------------------------------------------------------- */
int main(argc, argv)
int argc;
char          **argv;
{
    int             nsectors = MAXINT;  /* number of sectors to write. */
    int             nibble = 0;         /* which time is this. */
    size_t          sector_size = 512;  /* buffer size to use, default 512 */
    int             sector = 0;         /* first sector number. */
    char           *buffer;
    unsigned int   *record_number_pre;
    unsigned int   *record_number_post;
    struct timeval *current;           /* current microsecond time. */
    int             tmp;
    int             c;

/* Parse the arguments. */
    while ((c = getopt(argc, argv, "s:n:")) != -1)
    {
        switch (c)
        {
            case 's':
                nsectors = atoi(optarg);
		if (nsectors > (100*1024*1024/512))
		{
/*		    fprintf(stderr, "reducing %d sectors to %d\n", nsectors, 100*1024*1024/512); */
		    nsectors = 100*1024*1024/512;
		}
                break;

            case 'n':
                nibble = atoi(optarg);
		nibble &= 0xf;
                break;

            default:
                fprintf(stderr, "Unrecognized argument (%c)\n", c);
                exit(1);
        }
    }                                   /* end of while() for arguments */

/* Initialize the buffer. */
    buffer = (char *)valloc(sector_size);
    if (buffer == (char *)NULL)
    {                                   /* if error */
        perror("valloc");
        exit(1);
    }
    record_number_pre = (unsigned int *)buffer; /* where to put count. */
    current = (struct timeval *)(buffer + sizeof(unsigned int));  /* where to put time. */
    record_number_post = (unsigned int *)(buffer +
                                          sizeof(unsigned int) + sizeof(struct timeval));
    if (16 != (sizeof(unsigned int) + sizeof(struct timeval) + sizeof(unsigned int)))
    {
        fprintf(stderr, "timeval + 2 ints not equal 16 (%tu + %tu + %tu)\n", 
		sizeof(unsigned int), sizeof(struct timeval), sizeof(unsigned int));
        exit(1);
    }
    if (512 != sector_size)
    {
        fprintf(stderr, "Sector size must match memcpy below.\n");
        exit(1);
    }

    while (sector < nsectors)
    {
        *record_number_pre = sector++;
        *record_number_post = *record_number_pre | (nibble << (32-4));

        gettimeofday(current, NULL);

        /* Set buffer to pattern. */
        memcpy(buffer + 16, buffer, 16);
        memcpy(buffer + 32, buffer, 32);
        memcpy(buffer + 64, buffer, 64);
        memcpy(buffer + 128, buffer, 128);
        memcpy(buffer + 256, buffer, 256);
        tmp = write(1, buffer, sector_size);
        if (tmp != (int)sector_size)
        {
            perror("write stdout");
            exit(1);
        }
    }                                   /* end of while forever */
    exit(0);                            /* DONE! */
}                                       /* end of main */

/* End of file diskpattern.c */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
