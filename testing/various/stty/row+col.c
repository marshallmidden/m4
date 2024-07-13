#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main(int argc, char **argv)
{
    struct winsize win;

    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &win) < 0)
    {
	perror("TIOCGWINSZ");
	exit(1);
    }
    fprintf(stderr, "row=%d, col=%d, xpixel=%d, ypixel=%d\n", 
	    win.ws_row, win.ws_col, win.ws_xpixel, win.ws_ypixel);
    exit(0);
}
