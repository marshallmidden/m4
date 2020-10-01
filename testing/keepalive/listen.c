//
// This runs on 10.64.100.92
//
#define KEEPALIVE
// #define KEEPIDLE    10      /* 10 seconds after last data sent. */
// #define KEEPINTVL   10      /* each retry is 10 seconds apart, and each subsequent try is 10 seconds. */
// #define KEEPCNT     2       /* Try for 2 times. */

// Total time of 30 seconds.
// x = y        -- only makes sense to keep timers the same.
// x+y*5 = 30
//   6y = 30 => y=5.
// If y = 4, then 6*4=24 seconds, leaving 6 extra to do things.
#define KEEPIDLE    4       /* 4 seconds after last data sent. */
#define KEEPINTVL   4       /* each retry is 4 seconds apart, and each subsequent try is 4 seconds. */
#define KEEPCNT     5       /* Try for 5 times. */
/* ------------------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

static int             bind_fd = -1;
static int             accept_fd = -1;
struct in_addr         myAddress;

/* ------------------------------------------------------------------------ */
static void setupsockettolisten(void);
static void socketaccept(void);
static void read_accept(void);
/* ------------------------------------------------------------------------ */
int main(void)
{
    signal(SIGPIPE, SIG_IGN);

//    inet_aton("127.0.0.1", &myAddress);
    inet_aton("10.64.100.92", &myAddress);

    setupsockettolisten();

    socketaccept();

    read_accept();

    close(bind_fd);

    exit(EXIT_SUCCESS);
}   /* End of main */

/* ------------------------------------------------------------------------ */
static void read_accept(void)
{
    int       bytesRead = 0;
    unsigned char  pCharBuffer[1024 + 1];
    int       selectRC;
    fd_set    readSet;
    struct timeval timeout;
    int       count = 0;

    for (;;)
    {
        memset(pCharBuffer, 0 , sizeof(pCharBuffer));
        /* Set up the select and timeout period */
        FD_ZERO(&readSet);
        FD_SET(accept_fd, &readSet);
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000 * 1000;  /* 1 second */

        /* Wait for the select() to fire */
        selectRC = select(accept_fd + 1, &readSet, NULL, NULL, &timeout);

        if (selectRC > 0)
        {
            bytesRead = recv(accept_fd, pCharBuffer, 1024, 0);
            if (bytesRead == -1)
            {
                if (errno == EWOULDBLOCK)
                {
                    fprintf(stderr, "EWOULDBLOCK\n");
                    continue;
                }
                perror("recv");
                return;
            }
            else if (bytesRead == 0)
            {
                fprintf(stderr, "EOF on socket -- bytesRead == 0\n");
                return;
            }
            else
            {
                fprintf(stderr, "recv:(%s)\n", pCharBuffer);

#define STRING1 "exit\n"
                if (strncmp(pCharBuffer, STRING1, strlen(STRING1)) == 0)
                {
                    fprintf(stderr, "listen exit\n");
                    exit(0);
                }

#define STRING2 "die\n"
                if (strncmp(pCharBuffer, STRING2, strlen(STRING2)) == 0)
                {
                    fprintf(stderr, "listen abort\n");
                    abort();
                }

#define STRING3 "reboot\n"
                if (strncmp(pCharBuffer, STRING3, strlen(STRING3)) == 0)
                {
                    fprintf(stderr, "listen reboot -f\n");
                    system("reboot -f");
                }
            }
        }
        /* If the select() errored out, log it, then exit. */
        else if (selectRC < 0)
        {
            perror("select()");
            return;
        }
        else
        {
            /* We got a select() timeout, not a problem */
            count++;
            fprintf(stderr, "select timeout %d\n", count);
        }
    }
}   /* End of read_accept */

/* ------------------------------------------------------------------------ */
static void socketaccept(void)
{
    int             err;
    int             on = 1;
    struct sockaddr_in bin;
    socklen_t       sizeofStruct = sizeof(bin);
#ifdef KEEPALIVE
    int             optval;
#endif  /* KEEPALIVE */

    /* Get the socket */
fprintf(stderr, "before accept\n");
    accept_fd = accept(bind_fd, (struct sockaddr *)&bin, &sizeofStruct);
    if (accept_fd < 0)
    {
        perror("accept()");
        close(accept_fd);
        exit(EXIT_FAILURE);
    }

#ifdef KEEPALIVE
    /* Set the keepalive option active */
fprintf(stderr, "before SO_KEEPALIVE\n");
    err = setsockopt(accept_fd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));
    if (err < 0)
    {
        perror("setsockopt(SO_KEEPALIVE)");
        close(accept_fd);
        exit(EXIT_FAILURE);
    }

    optval = KEEPIDLE;  /* 10 seconds */
    err = setsockopt(accept_fd, SOL_TCP, TCP_KEEPIDLE, &optval, sizeof(optval));
    if (err < 0)
    {
        perror("setsockopt(TCP_KEEPIDLE)");
        close(accept_fd);
        exit(EXIT_FAILURE);
    }

    optval = KEEPCNT;
    err = setsockopt(accept_fd, SOL_TCP, TCP_KEEPCNT, &optval, sizeof(optval));
    if (err < 0)
    {
        perror("setsockopt(TCP_KEEPCNT)");
        close(accept_fd);
        exit(EXIT_FAILURE);
    }

    optval = KEEPINTVL;
    err = setsockopt(accept_fd, SOL_TCP, TCP_KEEPINTVL, &optval, sizeof(optval));
    if (err < 0)
    {
        perror("setsockopt(TCP_KEEPINTVL)");
        close(accept_fd);
        exit(EXIT_FAILURE);
    }
#endif  /* KEEPALIVE */

fprintf(stderr, "before FIONBIO\n");
    /* Set to non-blocking. */
    err = ioctl(accept_fd, FIONBIO, &on); 
    if (err < 0)
    {
        perror("ioctl(FIONBIO, &on)");
        close(accept_fd);
        exit(EXIT_FAILURE);
    }
fprintf(stderr, "exiting socketaccept\n");
}   /* End of socketaccept */

/* ------------------------------------------------------------------------ */
static void setupsockettolisten(void)
{
    int             err;
    int             on = 1;
    int             off = 0;
    struct linger lingerOption = { 1, 10 };
    struct sockaddr_in s_in;

    /* Create the socket */
    bind_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (bind_fd < 0)
    {
        perror("socket()");
        close(bind_fd);
        exit(EXIT_FAILURE);
    }

    /*
     * Enable local address reuse for the socket. This prevents bind() from
     * failing if the socket was closed following a failure in accept() below.
     */
    err = setsockopt(bind_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));
    if (err < 0)
    {
        perror("setsockopt(SO_REUSEADDR)");
        close(bind_fd);
        exit(EXIT_FAILURE);
    }

    /* Disable Nagle. */
    err = setsockopt(bind_fd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));
    if (err < 0)
    {
        perror("setsockopt(TCP_NODELAY)");
        close(bind_fd);
        exit(EXIT_FAILURE);
    }

    /* Set linger option for socket to ensure it flushes data when closed. */
    err = setsockopt(bind_fd, SOL_SOCKET, SO_LINGER, (void *)&lingerOption, sizeof(lingerOption));
    if (err < 0)
    {
        perror("setsockopt(SO_LINGER)");
        close(bind_fd);
        exit(EXIT_FAILURE);
    }

#ifdef KEEPALIVE
    /* Set the keepalive option active */
    err = setsockopt(bind_fd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));
    if (err < 0)
    {
        perror("setsockopt(SO_KEEPALIVE)");
        close(bind_fd);
        exit(EXIT_FAILURE);
    }
#endif  /* KEEPALIVE */

    /* Bind the socket to the port */
    memset(&s_in, 0, sizeof(s_in));
    s_in.sin_family = AF_INET;
    s_in.sin_addr.s_addr = myAddress.s_addr;
    s_in.sin_port = htons(4321);

    err = bind(bind_fd, (struct sockaddr *)&s_in, sizeof(s_in));
    if (err < 0)
    {
        perror("bind()");
        close(bind_fd);
        exit(EXIT_FAILURE);
    }

    /* Listen with a queue of 1 connections. */
    err = listen(bind_fd, 1);
    if (err < 0)
    {
        perror("listen()");
        close(bind_fd);
        exit(EXIT_FAILURE);
    }

    /* Set non-blocking off (i.e. BLOCKING). */
    err = ioctl(bind_fd, FIONBIO, &off); 
    if (err < 0)
    {
        perror("ioctl(FIONBIO)");
        close(bind_fd);
        exit(EXIT_FAILURE);
    }
}   /* End of setupsockettolisten */
/* ------------------------------------------------------------------------ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
