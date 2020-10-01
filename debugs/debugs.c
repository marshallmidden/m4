#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <ncurses.h>
#include <signal.h>
/* ------------------------------------------------------------------------ */
#include <termios.h>
static struct termios original_termios;
static char     original_termios_gotten = 0;

static int      terminal_col;			/* columns on terminal. */
static int      terminal_row;			/* rows on terminal. */
/* Where the words are printed. */
static int      wordstartrow;
static int	wordstartcol;
/* ------------------------------------------------------------------------ */
#define NORETURN __attribute__((noreturn))
#define UNUSED   __attribute__((unused)) /*@unused@*/
#define PACKED   __attribute__ ((packed))
/* ------------------------------------------------------------------------ */
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
// OS X does not have clock_gettime, use clock_get_time
#undef clock_gettime
#define clock_gettime(AAA, BBB)	{                       \
	clock_serv_t cclock;                            \
	mach_timespec_t mts;                            \
	                                                \
	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock); \
	clock_get_time(cclock, &mts);                   \
	mach_port_deallocate(mach_task_self(), cclock); \
	(BBB)->tv_sec = mts.tv_sec;                     \
	(BBB)->tv_nsec = mts.tv_nsec;                   \
    }
#endif	// __MACH__
/* ------------------------------------------------------------------------ */
#undef timeradd
#define timeradd(tvp, uvp, vvp)                                         \
        do {                                                            \
                (vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec;          \
                (vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec;       \
                if ((vvp)->tv_usec >= 1000000) {                        \
                        (vvp)->tv_sec++;                                \
                        (vvp)->tv_usec -= 1000000;                      \
                }                                                       \
        } while (0)
#undef timersub
#define timersub(tvp, uvp, vvp)                                         \
        do {                                                            \
                (vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec;          \
                (vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec;       \
                if ((vvp)->tv_usec < 0) {                               \
                        (vvp)->tv_sec--;                                \
                        (vvp)->tv_usec += 1000000;                      \
                }                                                       \
        } while (0)
/* ------------------------------------------------------------------------ */
#define	WORD_FILENAME	"words"
static FILE *word_file;
/* ------------------------------------------------------------------------ */
static int errors;
static int chars;
static int errflag;

#define NUMWORDS	54	/* Words per page. */
// #define NUMWORDS	2
#define MAXWORDLTH      50
/* The words to be typed. */
static char the_words[NUMWORDS][MAXWORDLTH];	

#define	strtx	3		/* space to leave on each side of printed line. */
#define	maxx	(terminal_col - strtx)

// lobit(zz)=10-int[bitcnt(zz$diff$(zz-1))/6]

/* ------------------------------------------------------------------------ */
/* forward. */
static void restore_original_termios(void);
/* ------------------------------------------------------------------------ */
NORETURN static void exit_perror(const char *str)
{
    perror(str);
    endwin();
    restore_original_termios();
    exit(1);
}    /* End of exit_perror */

/* ------------------------------------------------------------------------ */
static void restore_original_termios(void)
{
    if (original_termios_gotten == 0)
    {
        return;
    }
    original_termios.c_lflag |= ICANON;
    original_termios.c_lflag |= ECHO;
    original_termios_gotten = 0;	/* Make sure no fatal loop. */
    if (tcsetattr(0, TCSADRAIN, &original_termios) < 0)
    {
        exit_perror("tcsetattr ICANON");
    }
}                                       /* End of restore_original_termios */

/* ------------------------------------------------------------------------ */
static void get_original_termios(void)
{
    if (isatty(0))
    {
        if (tcgetattr(0, &original_termios) < 0)
        {
            perror("tcgetattr()");
	    return;
        }
        original_termios_gotten = 1;

        original_termios.c_lflag &= ~ICANON;
        original_termios.c_lflag &= ~ECHO;
        original_termios.c_cc[VMIN] = 1;
        original_termios.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &original_termios) < 0)
        {
            exit_perror("tcsetattr ICANON");
        }
    }
}                                       /* End of get_original_termios */

/* ------------------------------------------------------------------------ */
static char get_single_ch(void)
{
    char            buf = 0;

    if (read(0, &buf, 1) < 0)
    {
        exit_perror("read()");
    }
    return (buf);
}                                       /* End of get_single_ch */

/* ------------------------------------------------------------------------ */
static void purge_input(void)
{
    struct timeval limited;         	/* select timeout value */
    struct timeval *timelimit;
    fd_set          ibits;             	/* input select fd's */
    int maxfd;
    int n;
    struct timeval start_time;
    struct timeval end_time;

    if (original_termios_gotten == 0)
    {
        get_original_termios();
    }
    if (original_termios_gotten != 0)
    {
        original_termios.c_lflag &= ~ICANON;
        original_termios.c_lflag &= ~ECHO;
        original_termios.c_cc[VMIN] = 1;
        original_termios.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &original_termios) < 0)
        {
            exit_perror("tcsetattr ICANON");
        }
    }

    gettimeofday(&start_time, NULL);
    limited.tv_sec = 6;           	/* 6 second timeout */
    limited.tv_usec = 0;
    timeradd(&start_time, &limited, &end_time);

    do
    {
	FD_ZERO(&ibits);
	FD_SET(STDIN_FILENO, &ibits);
	maxfd = 1;

	gettimeofday(&start_time, NULL);
	timersub(&end_time, &start_time, &limited);
	timelimit = &limited;          	/* Some systems clobber this variable */
	n = select(maxfd, &ibits, NULL, (fd_set *) 0, timelimit);
        if (n < 0)
        {                              /* First check for error. */
            if (errno == EINTR)
            {
                continue;              /* if interrupted, loop */
            }
//             perror("select error, ignored");
            continue;                  /* ignore other errors(broken systems) */
        }
        if (n == 0)
        {                              /* select timeout */
	    // (void)fprintf(stderr, "select timeout\n");
            break;                  	/* Nothing to do upon a timeout. */
        }

	/* INPUT ---------- */
        if (FD_ISSET(STDIN_FILENO, &ibits))
        {
	    char ibuff[BUFSIZ];
            (void) read(STDIN_FILENO, ibuff, sizeof(ibuff));
	}
    } while(1);
}   /* End of purge_input */

/* ------------------------------------------------------------------------ */
NORETURN static void terminate(int code UNUSED)
{
    endwin();
    restore_original_termios();
    exit(0);
}                                       /* End of terminate */

/* ------------------------------------------------------------------------ */
static uint32_t random_state[64];

static void init_random(void)
{
    unsigned int i;
    unsigned seed;
    struct timespec time_now;

    for (i = 0; i < sizeof(random_state) / sizeof(random_state[0]); i +=2)
    {
        clock_gettime(CLOCK_REALTIME, &time_now);
        random_state[i+1] = time_now.tv_sec;
        random_state[i] = time_now.tv_nsec;
    }

    seed = 1;
    i = 256;
    initstate(seed, (char *)random_state, i);
    setstate((char *)random_state);
    srandom((unsigned int)time_now.tv_nsec);
}   /* End of init_random */

/* ------------------------------------------------------------------------ */
static void get_words(off_t word_file_size)
{
    unsigned int i;
    long         r;
    off_t        s;
    char	 line_file[MAXWORDLTH];
    int          l;
    int		 row;
    int		 col;

    chars = 0;
    for (i = 0; i < NUMWORDS; i++)
    {
	do
	{
	    r = random();
	    s = ((double)r / (double)0x7fffffff) * (double)word_file_size;
    /*  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
	    if (fseeko(word_file, s, SEEK_SET) != 0)
	    {
		exit_perror("fseeko of word_file failure");
	    }
	    memset(&line_file[0], 0, MAXWORDLTH);
	    /* The first could be a partial word. */
	    if (fgets(&line_file[0], MAXWORDLTH, word_file) == NULL)
	    {
		continue;                   	/* End Of File */
	    }
	    /*  The second will be a full word. */
	    if (fgets(&line_file[0], MAXWORDLTH, word_file) == NULL)
	    {
		continue;                   	/* End Of File */
	    }

	    /* Delete spaces and trailing new line. */
	    unsigned int j;
	    for (j = 0; j < MAXWORDLTH && line_file[j] != '\0'; j++)
	    {
		while (line_file[j] == ' ')    	/* delete spaces */
		{
		    memmove(&line_file[j], &line_file[j + 1], MAXWORDLTH - j - 1);
		}
		if (line_file[j] == '\n')      	/* delete end of line */
		{
		    line_file[j] = '\0';
		    break;
		}
	    }                               	/* End of for */
	    if (line_file[0] == '0')
	    {
		continue;			/* Nothing to add for this word. */
	    }
    /*  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
	    strncpy(&the_words[i][0], line_file, MAXWORDLTH);
	    l = strlen(line_file);
	    chars += l +1;			/* Count the spaces too. */
	    break;
	} while (1);
    }						/* End of for */

    /* Time to print out the words. */
    mvprintw(0, 0, "\n");
    mvprintw(1, 0, "This is a timed test. After a six (6) seconds pause,\n");
    mvprintw(2, 0, "start correctly to type in the following words.\n");
    mvprintw(3, 0, "\n");
    refresh();

    /* get the current curser position */
    getyx(stdscr, wordstartrow, wordstartcol);
    row = wordstartrow;
    col = wordstartcol;

    int x = col + strtx;			/* 3 leading spaces. */
    for (i = 0; i < NUMWORDS; i++)
    {
	l = strlen(&the_words[i][0]);
	if ((x + l) >= maxx)
	{
	    row += 3;				/* words, where to type, blank line. */
	    x = strtx;
	}
	mvprintw(row, x, "%s", &the_words[i][0]);
	x += l + strtx;
    }						/* End of for */
    refresh();
}   /* End of get_words */

/* ------------------------------------------------------------------------ */
static void debugp(void)
{
    char            key;
    unsigned int    j;				/* The word working on. */
    unsigned int    i;				/* The character in the word. */
    int   	    l;
    int             x = strtx;			/* 3 leading spaces. */
    int		    row;

    row = wordstartrow + 1;
    x = wordstartcol;

    mvprintw(row, x, " > ");
    refresh();

    for (j = 0; j < NUMWORDS; j++)
    {
	errflag = 1;
	for (i = 0; i < MAXWORDLTH; i++)
	{
	  get_key:
	    key = get_single_ch();
	    if (key == the_words[j][i])
	    {
	        printw("%c", key);
		refresh();
		continue;
	    }
	    if ((key == ' ' || key == '\n') && the_words[j][i] == '\0')
	    {
		/* Erase the last '>'. */
		mvprintw(row, x, "   ");
		refresh();

		l = strlen(&the_words[j][0]);
		x += strtx + l;			/* Where are we now. */
		if (j < (NUMWORDS-1))
		{
		    l = strlen(&the_words[j+1][0]);
		    if ((x + strtx + l) >= maxx)
		    {
			x = wordstartcol;
			row += 3;
		    }
		    mvprintw(row, x, " > ");
		    refresh();
		}
		break;
	    }
	    if (key == '?')
	    {
	        fprintf(stderr, "the_words[%d][%d]=0x%02x ('%c')\n", j, i, the_words[j][i], the_words[j][i]);
	    }
	    /* one error per word -- try to get right key */
	    errors += errflag;
	    errflag = 0;
	    goto get_key;
	}				/* End of for */
    }					/* End of for */
    printw("\n");
    refresh();
}   /* End of debugp */

/* ------------------------------------------------------------------------ */
static double print_results(double wpm, double old)
{
    printw("\n");
    printw("You did %3.2f words per minute.\n", wpm);
    if (wpm > old && old > 0.0)
    {
	printw("And you beat your best for today of %3.2f wpm.\n", old);
    }
    printw("Number of errors=%d. Corrected wpm=%3.2f.\n", errors, wpm - errors);
    refresh();
    return(old);
}   /* End of print_results */

/* ------------------------------------------------------------------------ */
int main(int argc UNUSED, char **argv UNUSED)
{
    double          wpm;
    double          old = 0.0;	/* Zero maximum while running program score. */
    char            key;
    struct stat     stats;
    off_t           word_file_size;
    clock_t         the_time;

    word_file = fopen(WORD_FILENAME, "r");
    if (word_file == NULL)
    {
	exit_perror("fopen of word file failed:");
    }
    if (stat(WORD_FILENAME, &stats) != 0)
    {
	exit_perror("stat of word file failed:");
    }
    word_file_size = stats.st_size - 1;

    /* Put into single character input mode. */
    get_original_termios();

    /* Initialize curses mode. */
    initscr();

    /* get the number of rows and columns */
    getmaxyx(stdscr,terminal_row,terminal_col);

    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
    {
        (void)signal(SIGINT, terminate);
    }

    init_random();			/* Initialize the random number generator. */

    do
    {
	erase();
	get_words(word_file_size);	/* Select words. */
	purge_input();

	the_time = time(0);
	errors = 0;
// j=1
	errflag = 1;			/* only one error per word. */

	debugp();			/* Type the words. */

	/* Determine words per minute. */
	wpm = (60 * chars) / ((0.00001 + (time(0) - the_time)) * 5);

	/* Print speed of typing, update maximum value while running program. */
	old = print_results(wpm, old);

	printw("\n");
	printw("Press return to do it again.\n");
	refresh();
	key = get_single_ch();
    } while (key == '\n' || key == ' ');

    terminate(0);
}	/* End of main */

/*
 * vi: sw=4 ts=8 expandtab
 */
