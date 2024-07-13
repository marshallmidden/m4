#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// #include <string.h>
// #include <time.h>
// #include <limits.h>
// #include <ctype.h>
// #include <errno.h>
// #include <sys/ioctl.h>
// #include <sys/select.h>
// #include <sys/stat.h>
// #include <sys/time.h>
// #include <ncurses.h>
// #include <signal.h>
/* ------------------------------------------------------------------------ */
#include <termios.h>
static struct termios original_termios;
static char     original_termios_gotten = 0;

/* ------------------------------------------------------------------------ */
#define NORETURN __attribute__((noreturn))
#define UNUSED   __attribute__((unused)) /*@unused@*/

/* ------------------------------------------------------------------------ */
NORETURN static void exit_perror(const char *str);

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
NORETURN static void exit_perror(const char *str)
{
    perror(str);
    restore_original_termios();
    exit(1);
}

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
int main(int argc UNUSED, char **argv UNUSED)
{
    char key;

    /* Put into single character input mode. */
    get_original_termios();

    do
    {
	key = get_single_ch();
    } while (key == '\n' || key == ' ');

    restore_original_termios();
    exit(0);
}	/* End of main */

/* ------------------------------------------------------------------------ */
/*
 * vi: sw=4 ts=8 expandtab
 */
