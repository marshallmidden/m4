/* $Id: ipc_server.c 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       ipc_server.c
** MODULE TITLE:    IPC server process
**
** DESCRIPTION:     Implementation for the ipc server process.  This is the
**                  process that monitors a set of incoming sockets for data
**                  to read.  When it has data to read is reads the data in
**                  and if it is all ok, enqueues it to the session manager
**
** Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "ipc_server.h"

#include "XIO_Std.h"
#include "debug_files.h"
#include "ipc_packets.h"
#include "ipc_socket_io.h"
#include "ipc_session_manager.h"
#include "ipc_common.h"
#include "ipc_security.h"
#include "ipc_sendpacket.h"
#include "logging.h"
#include "quorum_utils.h"

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: IpcServer
**
**  Description:    Function that is forked and never returns.  Monitors up to
**                  two sockets for incoming data.  When it arives it is
**                  validated and enqueued into the session manager for
**                  processing.
**
**  Inputs: void *  parm    Pointer to a session.  Set as a void * to work
**                  with pthreads library
**
**  Returns:    Never if we wrote it correctly
**--------------------------------------------------------------------------*/
void IpcServer(TASK_PARMS *parms)
{
    SESSION    *session = NULL;
    SOCKET     *eth = NULL;
    SOCKET      maxFD = 0;
    struct timeval selectTmo;
    IPC_PACKET *packetIn = NULL;
    fd_set      readSet;
    int         retval = 0;
    void       *parm = (void *)parms->p1;

    LogMessage(LOG_TYPE_DEBUG, "Starting IPC Server.");

    if (parm == NULL)
    {
        LogMessage(LOG_TYPE_DEBUG, "Stopping IPC Server, no parameter.");
        return;
    }

    session = (SESSION *)parm;

    eth = GetEthernetSocket(session);

    if (eth == NULL)
    {
        dprintf(DPRINTF_IPC_SERVER, "IpcServer: Error while getting socket handles\n");

        LogMessage(LOG_TYPE_DEBUG, "Stopping IPC Server, no ethernet socket.");
        return;
    }

    while (!IpcShutDownRequested())
    {
        /*
         * Check to see which sockets still have a valid value
         */
        while (*eth < 0)
        {
            TaskSleepMS(337);   /* Yield the CPU for a while */

            /*
             * We have no socket and someone has asked us to shut down so
             * we need to bail
             */
            if (IpcShutDownRequested())
            {
                return;
            }
        }

        /*
         * Setup the select timeout so that we
         * can examine the sockets to make sure that they
         * are ok
         */
        selectTmo.tv_sec = 0;
        selectTmo.tv_usec = 500000;     /* 500 msec tmo */

        /*
         * Zero out the read set
         */
        FD_ZERO(&readSet);

        if (*eth >= 0)
        {
            FD_SET(*eth, &readSet);
        }

        maxFD = *eth;

        /*
         * Need to add select for socket errors? Add timeout on select so we
         * can pick up both sockets
         */
        retval = Select(maxFD + 1, &readSet, NULL, NULL, &selectTmo);

        if (retval > 0)
        {
            if ((*eth >= 0) && FD_ISSET(*eth, &readSet))
            {
                /*
                 * Read the packet
                 */
                packetIn = ReadIpcPacket(*eth);

                if (packetIn == NULL)
                {
                    /*
                     * Need to mark this interface as down to the session manager
                     * and go on
                     */
                    CloseEthernetSocket(session);
                }
                else
                {
                    TaskSwitch();

                    /*
                     * Check the md5 on the data portion of the packet
                     */
                    if (CheckDataMD5(packetIn))
                    {
                        /*
                         * Save away the protocol level for this session
                         */
                        IPC_SetSessionProtocolLevel(session, packetIn);

                        /*
                         * Need to check time signature
                         */

                        SendResponseQueueDataPacketToSm(packetIn, SENDPACKET_ETHERNET);
                    }
                    else
                    {
                        dprintf(DPRINTF_IPC_SERVER, "IpcServer: Data MD5 error\n");

                        /*
                         * Need to mark this interface as down to the session manager
                         * and go on
                         */
                        CloseEthernetSocket(session);
                    }
                }
            }
        }
        else if (retval == SOCKET_ERROR)
        {
            /*
             * Error during select
             */
            dprintf(DPRINTF_IPC_SERVER, "IpcServer: We got an error returned on the select\n");

            /* Do a select on each socket to see which one died ? */
        }
    }

    /*
     * We were asked to shut down
     */
    if (*eth >= 0)
    {
        CloseSocket(eth);
    }

    LogMessage(LOG_TYPE_DEBUG, "Stopping IPC Server, IPC shutdown.");
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
