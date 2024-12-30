/* $Id: ipc_socket_io.c 156532 2011-06-24 21:09:44Z m4 $ */
/*============================================================================
** FILE NAME:       ipc_socket_io.c
** MODULE TITLE:    IPC socket data handling functions
**
** DESCRIPTION:     Implementation for the ipc socket io.  Provides an
**                  abstraction for the reading and writing of data to the
**                  sockets.
**
** Copyright (c) 2001-2010 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include "XIO_Std.h"
#include "ipc_socket_io.h"
#include "debug_files.h"
#include "fibreshim.h"
#include "ipc_common.h"
#include "ipc_packets.h"
#include "ipc_security.h"
#include "ipc_session_manager.h"
#include "PortServer.h"
#include "PortServerUtils.h"
#include "quorum_comm.h"
#include "CT_history.h"

/*****************************************************************************
** Private defines
*****************************************************************************/

/*
** Checks to make sure that the socket return code is ok
** will be removed when we go to non-blocking IO
*/
#define SOCKET_RC_OK(rc, dataTransfered)( ((rc) != -1) && \
                                        ((rc) == (dataTransfered)) ? 1 : 0)

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: ReadIpcPacket
**
**  Description:    Reads an IPC packet from the supplied socket
**
**  Inputs: SOCKECT     socket_number      Socket to read
**
**  Returns:    NULL on read error, else pointer to packet that was read
**--------------------------------------------------------------------------*/
IPC_PACKET *ReadIpcPacket(SOCKET socket_number)
{
    int         socketRC;
    IPC_PACKET *rc;
    UINT32      adaptiveTOSec;
    UINT32      len;

    if (SOCKET_ERROR == socket_number)
    {
        dprintf(DPRINTF_DEFAULT, "ReadIpcPacket: socket == SOCKET_ERROR\n");
        return NULL;
    }

    rc = MallocWC(sizeof(*rc));
    rc->header = MallocWC(sizeof(*rc->header));

    /* Read the header from the socket. */
    socketRC = ReceiveDataRaw(socket_number, (UINT8 *)rc->header,
                              sizeof(*rc->header), IPC_RX_TMO_MIN);

    if (!SOCKET_RC_OK(socketRC, sizeof(IPC_PACKET_HEADER)))
    {
        dprintf(DPRINTF_DEFAULT, "ReadIpcPacket: SOCKET_RC_OK Failed socketRC (%d)\n",
                socketRC);

        FreePacket(&rc, __FILE__, __LINE__);
        return NULL;
    }

    /* Check the header for a valid MD5 signature. */
    if (!CheckHeaderMD5(rc, Qm_GetCommKeyPtr()))
    {
        dprintf(DPRINTF_DEFAULT, "ReadIpcPacket: Header MD5 error\n");
        FreePacket(&rc, __FILE__, __LINE__);
        return NULL;
    }

    /* Check the header and see if we need to read more data. */
    len = GetPacketDataLength(rc->header);
    if (len)
    {
        rc->data = MallocSharedWC(len);

        /*
         * Calculate an adaptive overall timeout based upon the
         * length of the data being rx'd.  Increase timeout 1 sec
         * for each 8K additional data over 8K.
         */
        adaptiveTOSec = (len >> 13) + IPC_RX_TMO_MIN;
        if (adaptiveTOSec > IPC_RX_TMO_MAX)
        {
            adaptiveTOSec = IPC_RX_TMO_MAX;
        }

        socketRC = ReceiveDataRaw(socket_number, (UINT8 *)rc->data,
                                  len, adaptiveTOSec);

        if (!SOCKET_RC_OK(socketRC, (int)len))
        {
            FreePacket(&rc, __FILE__, __LINE__);
            return NULL;
        }
    }
    return rc;
}

/*----------------------------------------------------------------------------
**  Function Name: SendIpcPacket
**
**  Description:    Internal function used to send a packet.  It takes care
**                  of getting and freeing the locks
**
**  Inputs: SESSION    *session     Pointer to a session
**          SOCKET      socket_number      Socket to send data on
**          IPC_PACKET *packet      pointer to a packet to be sent
**
**  Returns:    TRUE if packet was sent ok
**              FALSE otherwise.
**--------------------------------------------------------------------------*/
static UINT8 SendIpcPacket(SESSION *session, SOCKET socket_number, IPC_PACKET *packet)
{
    INT32       len = 0;
    INT32       sent = 0;
    UINT32      adaptiveTOSec;

    if (packet && packet->header)
    {
        len = sizeof(*packet->header) + GetPacketDataLength(packet->header);
    }
    else
    {
        return FALSE;
    }

    GetSendLock(session);

    /*
     * Fill in the IPC Protocol version. This covers the case where the
     * header is being recycled (not created through CreateHeader).
     */
    SetPacketProtocolLevel(packet);

    /* Signing packet */
    if (CreateMD5Signature(packet, Qm_GetCommKeyPtr()))
    {
        /*
         * Calculate an adaptive overall timeout based upon the length of the
         * data being tx'd.  Increase timeout 1 sec for each 8K additional
         * data over 8K.
         */
        adaptiveTOSec = (len >> 13) + IPC_TX_TMO_MIN;
        if (adaptiveTOSec > IPC_TX_TMO_MAX)
        {
            adaptiveTOSec = IPC_TX_TMO_MAX;
        }

        /* Send packet */
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s Calling SendPacketRaw using packet of %p\n", __FILE__, __LINE__, __func__, packet);
#endif  /* HISTORY_KEEP */
        sent = SendPacketRaw(socket_number, (XIO_PACKET *)packet, adaptiveTOSec);
    }

    FreeSendLock(session);

    return (sent == len) ? TRUE : FALSE;
}

/*----------------------------------------------------------------------------
**  Function Name: SendPacketOverInterface
**
**  Description:    Send a packet over the specfied interface
**
**  Inputs: SESSION    *session         Pointer to a session
**          PATH_TYPE   requestedPath   Path the user wants
**          IPC_PACKET *txPacket        pointer to a packet to be sent
**
**  Returns:    Path that packet took when sent
**
** ATTENTION:   inProgressMutex must be locked before calling.
**
**--------------------------------------------------------------------------*/
#define ConvertInetAddrToString(A) (inet_ntoa(*(struct in_addr *)(&(A))))
PATH_TYPE SendPacketOverInterface(SESSION *session, PATH_TYPE requestedPath,
                                  IPC_PACKET *txPacket)
{
    bool        packetSent = FALSE;
    PATH_TYPE   rc = SENDPACKET_NO_PATH;

    if (session && txPacket && txPacket->header)
    {
        if (ETH_GOOD(session))
        {
            struct sockaddr_in sa;
            UINT32      len = sizeof(struct sockaddr);

            if (getpeername(session->eth, (struct sockaddr *)&sa, &len) < 0)
            {
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s using txPacket of %p\n", __FILE__, __LINE__, __func__, txPacket);
#endif  /* HISTORY_KEEP */
                dprintf(DPRINTF_IPC_MSG_DELIVERY, "Sending IPC cmd %u to crtlSn: %u, (no peer) (%d)\n",
                        txPacket->header->commandCode, session->sn, session->eth);
            }
            else
            {
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s using txPacket of %p\n", __FILE__, __LINE__, __func__, txPacket);
#endif  /* HISTORY_KEEP */
                dprintf(DPRINTF_IPC_MSG_DELIVERY, "Sending IPC cmd %u to crtlSn: %u, %s:%hu (%d)\n",
                        txPacket->header->commandCode, session->sn,
                        ConvertInetAddrToString(sa.sin_addr), ntohs(sa.sin_port),
                        session->eth);
            }
        }

        if ((!packetSent) && ETH_GOOD(session) &&
            ((requestedPath == SENDPACKET_ANY_PATH) ||
             (requestedPath == SENDPACKET_ETHERNET)))
        {
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s calling SendIpcPacket with txPacket of %p\n", __FILE__, __LINE__, __func__, txPacket);
#endif  /* HISTORY_KEEP */
            if (SendIpcPacket(session, session->eth, txPacket))
            {
                packetSent = TRUE;
                rc = SENDPACKET_ETHERNET;
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s return from using txPacket of %p\n", __FILE__, __LINE__, __func__, txPacket);
#endif  /* HISTORY_KEEP */
            }
            else
            {
                /* Mark this interface as down */
                dprintf(DPRINTF_DEFAULT, "Send IPC cmd %u to crtlSn: %u, (%d) Failed\n",
                        txPacket->header->commandCode, session->sn, session->eth);
                CloseEthernetSocket(session);
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s closing ethernet that was using txPacket of %p\n", __FILE__, __LINE__, __func__, txPacket);
#endif  /* HISTORY_KEEP */
            }
        }

        /*
         * This is an alternative way to send on the fibre channel
         * and we bypass the TCP/IP stack
         */
        if ((!packetSent) && FC_GOOD(session) &&
            ((requestedPath == SENDPACKET_ANY_PATH) ||
             (requestedPath == SENDPACKET_FIBRE)))
        {
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s Calling sendDLMDirect using txPacket of %p\n", __FILE__, __LINE__, __func__, txPacket);
#endif  /* HISTORY_KEEP */
            if (0 == SendDLMDirect(session->sn, txPacket))
            {
                packetSent = TRUE;
                rc = SENDPACKET_FIBRE;
            }
        }

        /*
         * We were unable to send the packet over the ethernet or
         * fibre channel so lets try the quorum if the user has
         * specfied that this is an OK thing to do
         */
        if ((!packetSent) &&
            QuorumSendablePacket(txPacket->header->commandCode) &&
            ((requestedPath == SENDPACKET_ANY_PATH) ||
             (requestedPath == SENDPACKET_QUORUM)))
        {
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s Calling SendQuorumPacket using txPacket of %p\n", __FILE__, __LINE__, __func__, txPacket);
#endif  /* HISTORY_KEEP */
            if (0 == SendQuorumPacket(session->sn, txPacket))
            {
                packetSent = TRUE;
                rc = SENDPACKET_QUORUM;

            }
        }

        /* Remove Session Bindings!! We have no paths!! */
        if ((packetSent == FALSE) && (session->status == GOOD))
        {
            if ((requestedPath == SENDPACKET_ANY_PATH) ||
                ((requestedPath == SENDPACKET_ETHERNET) && !FC_GOOD(session)) ||
                ((requestedPath == SENDPACKET_FIBRE) && !ETH_GOOD(session)) ||
                ((requestedPath == SENDPACKET_QUORUM) && !ETH_GOOD(session) &&
                 !FC_GOOD(session)))
            {
                session->status = ERROR;
                RemoveSessionBindings(session, FALSE);
            }
        }
        else if (packetSent == TRUE)
        {
            session->status = GOOD;
        }
    }

    return rc;
}

/*----------------------------------------------------------------------------
**  Function Name: bool SetIpcSocketOptions( SOCKET s)
**
**  Description:    Sets the socket up in accordance with the #define(s) in
**                  ipc_sommon.h
**
**  Inputs:     SOCKET  s   The socket we will be modifing
**
**  Returns:    TRUE if we completed without errors, otherwise FALSE
**--------------------------------------------------------------------------*/
bool SetIpcSocketOptions(SOCKET s)
{
#ifdef DISABLE_NAGLE
    INT32       tcpOption = 1;
#endif  /* DISABLE_NAGLE */

    /* Make the socket non-blocking */
    if (0 != SetBlockingState(s, BLOCKING_OFF))
    {
        return(FALSE);
    }

#ifdef DISABLE_NAGLE
    /* Disable Nagle. */
    if (0 != setsockopt(s, IPPROTO_TCP, TCP_NODELAY,
                        (void *)&tcpOption, sizeof(tcpOption)))
    {
        return(FALSE);
    }
#endif  /* DISABLE_NAGLE */

    return(TRUE);
}

/*----------------------------------------------------------------------------
**  Function Name:  int NonBlockingConnect( SOCKET sockfd,
**                                          const struct *sockaddr addressPtr,
**                                          socklen_t slen,
**                                          uint16 tmo_sec );
**
**  Description:    Does a non-blocking socket connect with a user specfied
**                  timeout in seconds
**
**  Inputs:     SOCKET sockfd                       socket for connect
**              const struct *sockaddr addressPtr   Where we are connecting to
**              int slen                            Len of data @ addressPtr
**              UINT16  tmo_sec                     Timeout in seconds
**
**  Returns:    0 if we completed without errors, else -1
**
**  Note:       Precondition is that we have a non-blocking socket as input
**--------------------------------------------------------------------------*/
int NonBlockingConnect(SOCKET sockfd, const struct sockaddr *addressPtr,
                       int slen, UINT16 tmo_sec)
{
    int         rc = 0;
    fd_set      rset;
    fd_set      wset;
    struct timeval tval;
    int         errorCode = 0;

    rc = connect(sockfd, (struct sockaddr *)addressPtr, slen);

    if (rc < 0)
    {
        /* If we got an error we are expecting EINPROGRESS */
        errorCode = errno;

        if (errorCode == TM_EINPROGRESS)
        {
            FD_ZERO(&rset);
            FD_SET(sockfd, &rset);
            wset = rset;
            tval.tv_sec = tmo_sec;
            tval.tv_usec = 0;

            rc = Select(sockfd + 1, &rset, &wset, NULL, tmo_sec ? &tval : NULL);

            if (rc == 0)
            {
                dprintf(DPRINTF_DEFAULT, "NonBlockingConnect: Timeout on select.\n");

                /* We had a timeout occur */
                rc = SOCKET_ERROR;
            }
            else
            {
                if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset))
                {
                    /*
                     * Do a connect again, on linux we should get an error
                     * EISCONN for the treck stack we should get zero?
                     */
                    rc = connect(sockfd, (struct sockaddr *)addressPtr, slen);

                    if (rc < 0)
                    {
                        errorCode = errno;

                        if (errorCode != TM_EISCONN)
                        {
                            dprintf(DPRINTF_DEFAULT, "NonBlockingConnect: Connect returned errno %u/%s (other than TM_EISCONN)\n",
                                    errorCode, GetErrnoString(errorCode));

                            rc = SOCKET_ERROR;
                        }
                    }
                    else if (rc > 0)
                    {
                        dprintf(DPRINTF_DEFAULT, "NonBlockingConnect: Connect non-zero (0x%x).\n", rc);

                        /*
                         * Treck documentation states that we should
                         * return 0 so this is an error
                         */
                        rc = SOCKET_ERROR;
                    }
                }
                else
                {
                    dprintf(DPRINTF_DEFAULT, "NonBlockingConnect: sockfd not set.\n");

                    /* Error: sockfd no set */
                    rc = SOCKET_ERROR;
                }
            }
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "NonBlockingConnect: Connect returned an error other than TM_EINPROGRESS (0x%x).\n",
                    errorCode);
        }
    }

    /* Close the socket if errors exist */
    if (rc == SOCKET_ERROR)
    {
        CloseSocket(&sockfd);
    }

    return rc;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
