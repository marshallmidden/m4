/* $Id: ipc_listener.c 149941 2010-11-03 21:38:18Z m4 $ */
/*============================================================================
** FILE NAME:       ipc_listner.c
** MODULE TITLE:    IPC listener daemon
**
** DESCRIPTION:     Implementation for the ipc port listener daemon.  This
**                  function creates a thread of execution that never exits.
**                  Sole purpose is to listen on all interfaces on port 3008.
**                  When a client connects the socket connection is
**                  registered with the session manager so that all can use
**                  it.
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "debug_files.h"
#include "ipc_listener.h"
#include "ipc_session_manager.h"
#include "ipc_socket_io.h"
#include "nvram.h"
#include "PortServerUtils.h"
#include "quorum_utils.h"

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: IPC_CreateListeningSocket
**
**  Description:    Internal function that is used to create a listening
**                  socket binded to whatever ip address and port the caller
**                  wants
**
**  Inputs: UINT32  inetAddress     ipaddress to bind to
**          UINT16  port            port number to bind to
**
**  Returns:    Returns a listening socket or -1 on error
**--------------------------------------------------------------------------*/
static SOCKET IPC_CreateListeningSocket(UINT32 inetAddress, UINT16 port)
{
    SOCKET      newSocket = SOCKET_ERROR;
    struct sockaddr_in s_in;
    INT32       reuseaddrOption = SO_REUSEADDR;
    UINT32      errorCode;
    INT32       rc = GOOD;
    const char *where = "unknown";

    memset(&s_in, 0, sizeof(s_in));

    do
    {
        if ((inetAddress == 0) || (port == 0))
        {
            rc = ERROR;
            where = "input parameter check";
            break;
        }

        newSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (newSocket < 0)
        {
            rc = ERROR;
            where = "socket()";
            break;
        }

        if (FALSE == SetIpcSocketOptions(newSocket))
        {
            rc = ERROR;
            where = "SetIpcSocketOptions()";
            break;
        }

        if (setsockopt(newSocket,
                       SOL_SOCKET, SO_REUSEADDR,
                       (void *)&reuseaddrOption, sizeof(reuseaddrOption)) == SOCKET_ERROR)
        {
            rc = ERROR;
            where = "setsockopt(SO_REUSEADDR)";
            break;
        }

        s_in.sin_family = AF_INET;
        s_in.sin_addr.s_addr = inetAddress;
        s_in.sin_port = htons(port);
        if (bind(newSocket, (struct sockaddr *)&s_in, sizeof(s_in)) == SOCKET_ERROR)
        {
            rc = ERROR;
            where = "bind()";
            break;
        }

        if (listen(newSocket, IPC_LISTENING_QUEUE_SIZE) == SOCKET_ERROR)
        {
            rc = ERROR;
            where = "listen()";
            break;
        }

    } while (0);

    if (rc == ERROR)
    {
        errorCode = (newSocket == SOCKET_ERROR) ? 0 : errno;
        LogMessage(LOG_TYPE_ERROR, "IPC_CreateListeningSocket(IPC): %s failed, socket %d, errno %d/%s.\n",
                   where, newSocket, errorCode, GetErrnoString(errorCode));
    }

    return newSocket;
}

/*----------------------------------------------------------------------------
**  Function Name: PortListener
**
**  Description:    Process that runs forever.  Basically this is the function
**                  that listens on port 3008 on all interfaces for a client
**                  connect.  When that happens is regisers the new client
**                  socket connection with the session manager.
**
**  Inputs: void *  Default signature so that we can do a pthread create in
**                  unix.  Paramter is not needed
**
**  Returns:    Never returns
**--------------------------------------------------------------------------*/
void PortListener(UNUSED TASK_PARMS *parms)
{
    SOCKET      listenSocketEth = -1;   /* Listening socket */
    SOCKET      newSocketEth = -1;      /* new socket when someone connects */
    struct timeval selectTmo;   /* select time out */
    fd_set      readSet;

    int         retval = 0;     /* Used for return value */

    size_t      sizeofStruct = 0;
    struct sockaddr_in clientEth;

    /* Keep looping if we can't open the listening socket */
    while (listenSocketEth < 0)
    {
        listenSocketEth = IPC_CreateListeningSocket(GetIPAddress(), IPC_PORT_ETH);

        if (listenSocketEth < 0)
        {
            TaskSleepMS(5000);
        }
    }

    /* Run until someone has asked us to exit */
    while (!IpcShutDownRequested())
    {
        FD_ZERO(&readSet);
        FD_SET(listenSocketEth, &readSet);

        /* Setup select timeout so we can check if we should shut down */
        selectTmo.tv_sec = 1;   /* 1 sec tmo */
        selectTmo.tv_usec = 0;

        /* Add checks for errors in select? Only needed for out of band data */
        retval = Select(listenSocketEth + 1, &readSet, NULL, NULL, &selectTmo);

        if (retval > 0)
        {
            /* Get the session lock */
            (void)LockMutex(&sessionListMutex, MUTEX_WAIT);

            if (FD_ISSET(listenSocketEth, &readSet))
            {
                /* Need to do an accept */
                sizeofStruct = sizeof(clientEth);
                newSocketEth = Accept(listenSocketEth, (struct sockaddr *)&clientEth, &sizeofStruct);

                if (newSocketEth != SOCKET_ERROR)
                {
                    if (FALSE == SetIpcSocketOptions(newSocketEth))
                    {
                        /* Error occured */
                        CloseSocket(&newSocketEth);
                    }
                    else
                    {
                        /* Set tcp keepalive options. */
                        setkeepalive(newSocketEth);

                        /*
                         * Check with the session manager and see if we
                         * are ok to register with the session manager
                         */
                        if (!ClientConnect(newSocketEth, clientEth.sin_addr.s_addr))
                        {
                            CloseSocket(&newSocketEth);
                        }
                    }
                }

                /* else
                 * {
                 * **
                 * ** See Unix network Programming 2nd edition
                 * ** Page 424 (Stevens)
                 * **
                 * ** Basically the select instructed us that someone was
                 * ** connecting so the accept should work, however if the
                 * ** client sends a RST before we accept we can get here
                 * ** in this case we are just going to loop again.
                 * **
                 * **
                 * }
                 */
            }

            /* Free the session lock */
            UnlockMutex(&sessionListMutex);
        }
        else
        {
            if (retval != 0)
            {
                dprintf(DPRINTF_DEFAULT, "PortListener(IPC): We got a value of %d from select.  This is unexpected and currently unhandled.\n", retval);
            }

            /*
             * Not sure what we should do here.  We could do a select on each
             * listening socket individually to see which one is in error and
             * then close that socket and try to re-bind another one?  This
             * might work, but I'm not sure at this time how to test it.
             * Could just shut down both and re-create listeners again?
             */
        }
    }

    /*
     * If we get here it means that someone has requested that we should stop
     * running, close the listening socket and exit thread
     */
    if (listenSocketEth >= 0)
    {
        /*
         * We are doing a simple socket close instead of a shutdown followed
         * by close.  This was shown as the correct way to close a listening
         * socket as we are not processing data on it
         */
        if (SOCKET_ERROR == Close(listenSocketEth))
        {
            UINT32      errorCode = errno;

            dprintf(DPRINTF_DEFAULT, "PortListener(IPC): Unexpected error while closing listening socket, errno %d/%s.\n",
                    errorCode, GetErrnoString(errorCode));
        }
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "PortListener(IPC): We expected our IPC listening socket to be valid!\n");
    }

    return;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
