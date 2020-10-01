/*
   vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <aio.h>
#include <signal.h>

#define BUF_SIZE 20                             /* Size of buffers for read operations */
#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)
#define errMsg(msg)  do { perror(msg); } while (0)

/* ------------------------------------------------------------------------ */
/* Application-defined structure for tracking I/O requests. */
struct ioRequest
{
    size_t          reqNum;
    int             status;
    struct aiocb   *aiocbp;
};

/* ------------------------------------------------------------------------ */
/* On delivery of SIGQUIT, we attempt to cancel all outstanding I/O requests */
static volatile sig_atomic_t gotSIGQUIT = 0;

/* Handler for SIGQUIT */
static void quitHandler(int sig __attribute__ ((unused)))
{
    gotSIGQUIT = 1;
}   /* End of quitHandler */

/* ------------------------------------------------------------------------ */
/* Signal used to notify I/O completion */
#define IO_SIGNAL SIGUSR1

/* ------------------------------------------------------------------------ */
/* Handler for I/O completion signal */
static void aioSigHandler(int sig __attribute__ ((unused)), siginfo_t * si,
                          void *ucontext __attribute__ ((unused)))
{
    if (si->si_code == SI_ASYNCIO)
    {
        write(STDOUT_FILENO, "I/O completion signal received\n", 31);

        /* The corresponding ioRequest structure would be available as
	 * struct ioRequest *ioReq = si->si_value.sival_ptr; and the file
	 * descriptor would then be available via ioReq->aiocbp->aio_fildes */
    }
}   /* End of aioSigHandler */

/* ------------------------------------------------------------------------ */
static struct ioRequest *ioList;
static struct aiocb   *aiocbList;
static size_t          numReqs;                    /* Total number of queued I/O requests */

/* ------------------------------------------------------------------------ */
static void process_args(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <pathname> <pathname>...\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    numReqs = (size_t) (argc - 1);

    /* Allocate our arrays */
    ioList = (struct ioRequest *)calloc(numReqs, sizeof(struct ioRequest));
    if (ioList == NULL)
    {
        errExit("calloc");
    }
    aiocbList = (struct aiocb *)calloc(numReqs, sizeof(struct aiocb));
    if (aiocbList == NULL)
    {
        errExit("calloc");
    }
}    /* End of process_args */

/* ------------------------------------------------------------------------ */
static void setup_signal_handlers(void)
{
    struct sigaction sa;

    /* Establish handlers for SIGQUIT and the I/O completion signal */
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = quitHandler;
    if (sigaction(SIGQUIT, &sa, NULL) == -1)
        errExit("sigaction");
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sa.sa_sigaction = aioSigHandler;
    if (sigaction(IO_SIGNAL, &sa, NULL) == -1)
    {
        errExit("sigaction");
    }
}   /* End of setup_signal_handlers */

/* ------------------------------------------------------------------------ */
static void open_fd_for_read(char *argv[])
{
    size_t          j;
    int             s;

    /* Open each file specified on the command line, and queue a read request
       on the resulting file descriptor */
    for (j = 0; j < numReqs; j++)
    {
        ioList[j].reqNum = j;
        ioList[j].status = EINPROGRESS;
        ioList[j].aiocbp = &aiocbList[j];

        ioList[j].aiocbp->aio_fildes = open(argv[j + 1], O_RDONLY);
        if (ioList[j].aiocbp->aio_fildes == -1)
	{
            errExit("open");
	}
        printf("opened %s on descriptor %d\n", argv[j + 1], ioList[j].aiocbp->aio_fildes);

        ioList[j].aiocbp->aio_buf = malloc(BUF_SIZE);
        if (ioList[j].aiocbp->aio_buf == NULL)
	{
            errExit("malloc");
	}

        ioList[j].aiocbp->aio_nbytes = BUF_SIZE;
        ioList[j].aiocbp->aio_reqprio = 0;
        ioList[j].aiocbp->aio_offset = 0;
        ioList[j].aiocbp->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
        ioList[j].aiocbp->aio_sigevent.sigev_signo = IO_SIGNAL;
        ioList[j].aiocbp->aio_sigevent.sigev_value.sival_ptr = &ioList[j];

        s = aio_read(ioList[j].aiocbp);
        if (s == -1)
	{
            errExit("aio_read");
	}
    }
}   /* End of open_fd_for_read */

/* ------------------------------------------------------------------------ */
int main(int argc, char *argv[])
{
    size_t          j;
    size_t          openReqs;                   /* Number of I/O requests still in progress */
    int             s;

    process_args(argc, argv);

    setup_signal_handlers();

    open_fd_for_read(argv);
    openReqs = numReqs;

    /* Loop, monitoring status of I/O requests */
    while (openReqs > 0)
    {
        /* Delay between each monitoring step */
        sleep(5);

        if (gotSIGQUIT)
        {
            /* On receipt of SIGQUIT, attempt to cancel each of the outstanding I/O requests, and display status returned from the cancellation requests */
            printf("got SIGQUIT; canceling I/O requests: \n");

            for (j = 0; j < numReqs; j++)
            {
                if (ioList[j].status == EINPROGRESS)
                {
                    printf("    Request %ld on descriptor %d:", j,
                           ioList[j].aiocbp->aio_fildes);
                    s = aio_cancel(ioList[j].aiocbp->aio_fildes, ioList[j].aiocbp);
                    if (s == AIO_CANCELED)
		    {
                        printf("I/O canceled\n");
		    }
                    else if (s == AIO_NOTCANCELED)
		    {
                        printf("I/O not canceled\n");
		    }
                    else if (s == AIO_ALLDONE)
		    {
                        printf("I/O all done\n");
		    }
                    else
		    {
                        errMsg("aio_cancel");
		    }
                }
            }
            gotSIGQUIT = 0;
        }

        /* Check the status of each I/O request that is still in progress */
        printf("aio_error():\n");
        for (j = 0; j < numReqs; j++)
        {
            if (ioList[j].status == EINPROGRESS)
            {
                printf("    for request %ld (descriptor %d): ", j,
                       ioList[j].aiocbp->aio_fildes);
                ioList[j].status = aio_error(ioList[j].aiocbp);
                switch (ioList[j].status)
                {
                    case 0:
                        printf("I/O succeeded\n");
                        break;
                    case EINPROGRESS:
                        printf("In progress\n");
                        break;
                    case ECANCELED:
                        printf("Canceled\n");
                        break;
                    default:
                        errMsg("aio_error");
                        break;
                }
                if (ioList[j].status != EINPROGRESS)
		{
                    openReqs--;
		}
            }
        }
    }
    printf("All I/O requests completed\n");

    /* Check status return of all I/O requests */
    printf("aio_return():\n");
    for (j = 0; j < numReqs; j++)
    {
        ssize_t         s;

        s = aio_return(ioList[j].aiocbp);
        printf("    for request %ld (descriptor %d): %zd\n", j, ioList[j].aiocbp->aio_fildes, s);
    }

    exit(EXIT_SUCCESS);
}

/* ------------------------------------------------------------------------ */
