#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include "get_single_ch.h"

char get_single_ch(void)
{
    char            buf = 0;
    struct termios  old = { 0 };

    if (tcgetattr(0, &old) < 0)
    {
        perror("tcsetattr()");
    }
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
    {
        perror("tcsetattr ICANON");
        exit(1);
    }
    if (read(0, &buf, 1) < 0)
    {
        perror("read()");
        exit(1);
    }
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
    {
        perror("tcsetattr ~ICANON");
        exit(1);
    }
    return (buf);
}                                      /* End of get_single_ch */
