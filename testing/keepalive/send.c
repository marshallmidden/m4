//
// This runs on 10.64.100.93
//
#define KEEPALIVE

// #define KEEPIDLE    10
// #define KEEPINTVL   10
// #define KEEPCNT     2

// See listen.c.
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

static int             connect_fd = -1;
struct in_addr         toAddress;

/* ------------------------------------------------------------------------ */
static void setupsockettoconnect(void);
static void send_to(void);
/* ------------------------------------------------------------------------ */
int main(void)
{
    signal(SIGPIPE, SIG_IGN);

    inet_aton("10.64.100.92", &toAddress);

    setupsockettoconnect();

    send_to();

    close(connect_fd);

    exit(EXIT_SUCCESS);
}   /* End of main */

/* ------------------------------------------------------------------------ */
static void send_to(void)
{
    char  pCharBuffer[1024 + 1];
    int err;

    for (;;)
    {
        fd_set      writeSet;
        struct timeval selTO;

        (void)fgets(pCharBuffer, 1025, stdin);
        if (strlen(pCharBuffer) == 0)
        {
            return;
        }

//        err = write(connect_fd, pCharBuffer, strlen(pCharBuffer));
//        if (err < 0)
//        {
//            perror("write()");
//            return;
//        }
//        if (err == 0)
//        {
//            fprintf(stderr, "nothing written???\n");
//            return;
//        }
//        if ((size_t)err != strlen(pCharBuffer))
//        {
//            fprintf(stderr, "write returned %d, yet string is %d\n", err, strlen(pCharBuffer));
//            return;
//        }

        FD_ZERO(&writeSet);
        FD_SET(connect_fd, &writeSet);
        selTO.tv_sec = 0;
        selTO.tv_usec = 1000 * 1000;    /* 1 second */
        err = select(connect_fd + 1, NULL, &writeSet, NULL, &selTO);
        if (err > 0)
        {
            err = send(connect_fd, pCharBuffer, strlen(pCharBuffer), 0);
            if (err == -1)
            {
                if (errno != EWOULDBLOCK)
                {
                    perror("send()");
                    return;
                }
                fprintf(stderr, "send() returned EWOULDBLOCK -- error in test program.\n");
                return;
            }
            if ((size_t)err != strlen(pCharBuffer))
            {
                fprintf(stderr, "send returned %d, yet string is %d\n", err, strlen(pCharBuffer));
                return;
            }
        }
        else if (err == -1)
        {
            perror("select()");
            return;
        }
        else
        {
            fprintf(stderr, "select got a timeout -- error in test program\n");
            return;
        }

#define STRING1 "quit\n"
        if (strncmp(pCharBuffer, STRING1, strlen(STRING1)) == 0)
        {
            fprintf(stderr, "send quit\n");
            exit(0);
        }

#define STRING2 "abort\n"
        if (strncmp(pCharBuffer, STRING2, strlen(STRING2)) == 0)
        {
            fprintf(stderr, "send abort\n");
            abort();
        }

#define STRING3 "rebooting\n"
        if (strncmp(pCharBuffer, STRING3, strlen(STRING3)) == 0)
        {
            fprintf(stderr, "send reboot -f\n");
            system("reboot -f");
        }
    }
}   /* End of send_to */

/* ------------------------------------------------------------------------ */
static void setupsockettoconnect(void)
{
    int             err;
    int             on = 1;
    struct sockaddr_in their_in;
    fd_set      rset;
    fd_set      wset;
    struct timeval tval;
#ifdef KEEPALIVE
    int             optval;
#endif  /* KEEPALIVE */

    /* Create the socket */
    connect_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connect_fd < 0)
    {
        perror("socket()");
        close(connect_fd);
        exit(EXIT_FAILURE);
    }

    /* Set non-blocking. */
    err = ioctl(connect_fd, FIONBIO, &on); 
    if (err < 0)
    {
        perror("ioctl(FIONBIO)");
        close(connect_fd);
        exit(EXIT_FAILURE);
    }

    /* Disable Nagle. */
    err = setsockopt(connect_fd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));
    if (err < 0)
    {
        perror("setsockopt(TCP_NODELAY)");
        close(connect_fd);
        exit(EXIT_FAILURE);
    }

#ifdef KEEPALIVE
    /* Set the keepalive option active */
    err = setsockopt(connect_fd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));
    if (err < 0)
    {
        perror("setsockopt(SO_KEEPALIVE)");
        close(connect_fd);
        exit(EXIT_FAILURE);
    }

    optval = KEEPIDLE;
    err = setsockopt(connect_fd, SOL_TCP, TCP_KEEPIDLE, &optval, sizeof(optval));
    if (err < 0)
    {
        perror("setsockopt(TCP_KEEPIDLE)");
        close(connect_fd);
        exit(EXIT_FAILURE);
    }

    optval = KEEPCNT;
    err = setsockopt(connect_fd, SOL_TCP, TCP_KEEPCNT, &optval, sizeof(optval));
    if (err < 0)
    {
        perror("setsockopt(TCP_KEEPCNT)");
        close(connect_fd);
        exit(EXIT_FAILURE);
    }

    optval = KEEPINTVL;
    err = setsockopt(connect_fd, SOL_TCP, TCP_KEEPINTVL, &optval, sizeof(optval));
    if (err < 0)
    {
        perror("setsockopt(TCP_TCP_KEEPINTVL)");
        close(connect_fd);
        exit(EXIT_FAILURE);
    }
#endif  /* KEEPALIVE */

    /* Bind the socket to the port */
    memset(&their_in, 0, sizeof(their_in));
    their_in.sin_family = AF_INET;
    their_in.sin_addr.s_addr = toAddress.s_addr;
    their_in.sin_port = htons(4321);

    err = connect(connect_fd, (struct sockaddr *)&their_in, sizeof(their_in));
    if (err == 0)
    {
        goto resetkeepalive;
    }

    if (errno == EINPROGRESS)
    {
        fprintf(stderr, "connect non-blocking, got in progress\n");

        FD_ZERO(&rset);
        FD_SET(connect_fd, &rset);
        wset = rset;
        tval.tv_sec = 0;
        tval.tv_usec = 0;

        err = select(connect_fd + 1, &rset, &wset, NULL, NULL);

        if (err == 0)
        {
            fprintf(stderr, "NonBlockingConnect: Timeout on select.\n");
            close(connect_fd);
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(connect_fd, &rset) || FD_ISSET(connect_fd, &wset))
        {
            /* Do a connect again, on linux we should get an error EISCONN */
            err = connect(connect_fd, (struct sockaddr *)&their_in, sizeof(their_in));
            if (err < 0)
            {
                int errorCode = errno;

                if (errorCode != EISCONN)
                {
                    fprintf(stderr, "NonBlockingConnect: Connect returned errno %u (other than EISCONN)\n", errorCode);
                    close(connect_fd);
                    exit(EXIT_FAILURE);
                }
            }
            goto resetkeepalive;
        }
        fprintf(stderr, "NonBlockingConnect: connect_fd not set.\n");
        close(connect_fd);
        exit(EXIT_FAILURE);
    }
    perror("connect()");
    close(connect_fd);
    exit(EXIT_FAILURE);

resetkeepalive:
#ifdef KEEPALIVE
    /* Set the keepalive option active */
    err = setsockopt(connect_fd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));
    if (err < 0)
    {
        perror("setsockopt(SO_KEEPALIVE)");
        close(connect_fd);
        exit(EXIT_FAILURE);
    }

    optval = KEEPIDLE;
    err = setsockopt(connect_fd, SOL_TCP, TCP_KEEPIDLE, &optval, sizeof(optval));
    if (err < 0)
    {
        perror("setsockopt(TCP_KEEPIDLE)");
        close(connect_fd);
        exit(EXIT_FAILURE);
    }

    optval = KEEPCNT;
    err = setsockopt(connect_fd, SOL_TCP, TCP_KEEPCNT, &optval, sizeof(optval));
    if (err < 0)
    {
        perror("setsockopt(TCP_KEEPCNT)");
        close(connect_fd);
        exit(EXIT_FAILURE);
    }

    optval = KEEPINTVL;
    err = setsockopt(connect_fd, SOL_TCP, TCP_KEEPINTVL, &optval, sizeof(optval));
    if (err < 0)
    {
        perror("setsockopt(TCP_TCP_KEEPINTVL)");
        close(connect_fd);
        exit(EXIT_FAILURE);
    }
#endif  /* KEEPALIVE */
}   /* End of setupsockettoconnect */

/* ------------------------------------------------------------------------ */
/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
