/* $Id: PortSrvUtils.c 149941 2010-11-03 21:38:18Z m4 $*/
/*===========================================================================
** FILE NAME:       PortServerUtils.c
** MODULE TITLE:    CCB Port Server Utilities
**
** DESCRIPTION:     Utility functions to support CCB port server
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "AsyncEventHandler.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "ipc_security.h"
#include "ipc_socket_io.h"
#include "logging.h"
#include "misc.h"
#include "PacketInterface.h"
#include "PI_Clients.h"
#include "PortServerUtils.h"
#include "rtc.h"
#include "X1_Packets.h"
#include "X1_Structs.h"
#include "XIO_Std.h"
#include "timer.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

/*****************************************************************************
** Private defines
*****************************************************************************/
#define MAX_ETHERNET_DATA_SIZE 1460     /* Frame size minus IP and TCP headers */

/*****************************************************************************
** Private variables
*****************************************************************************/
static UINT32 portServerClientIPAddr = 0;

static INT32 portSrvSocket = PI_SOCKET_ERROR;

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
UINT8       piMD5Key[16] = {
    0x11, 0x9F, 0x4F, 0x53, 0xB6, 0x6D, 0x81, 0xD9,
    0xD7, 0x5A, 0xFE, 0x98, 0xC4, 0x98, 0x75, 0x0A
};
MUTEX       sendMutex;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    ReceiveData
**
** Description: Read data from the socket into a buffer
**
** Inputs:      socket_number - TCP/IP socket
**              pBuffer - ptr to read buffer
**              bytesToRead - Number of bytes to read from the socket
**
** Returns:     Total number of bytes read or PI_SOCKET_ERROR
**
**--------------------------------------------------------------------------*/
INT32 ReceiveData(INT32 socket_number, UINT8 *pBuffer, INT32 bytesToRead,
                  UINT32 timeOutSec)
{
    return ReceiveDataRaw(socket_number, pBuffer, bytesToRead, timeOutSec);
}

/*----------------------------------------------------------------------------
** Function:    ReceiveDataRaw
**
** Description: Read data from the socket into a buffer
**
** Inputs:      socket_number - TCP/IP socket
**              pBuffer - ptr to read buffer
**              bytesToRead - Number of bytes to read from the socket
**              timeOutSec - seconds to allow for all bytes to be rx'd
**
** Returns:     Total number of bytes read or PI_SOCKET_ERROR
**
**--------------------------------------------------------------------------*/
INT32 ReceiveDataRaw(INT32 socket_number, UINT8 *pBuffer, INT32 bytesToRead,
                     UINT32 timeOutSec)
{
    INT32       totalBytesRead = 0;
    INT32       bytesRead = 0;
    UINT8      *pCharBuffer;
    INT32       selectRC;
    fd_set      readSet;
    struct timeval timeout;
    UINT32      timeLimit = 0;
    UINT32      errorCode;

    /*
     * Determine if basic requirements met first. We don't fail if all
     * assertions are not good. Maybe we should?
     */
    ccb_assert(socket_number != SOCKET_ERROR, pBuffer);
    ccb_assert(pBuffer != NULL, pBuffer);
    ccb_assert(bytesRead >= 0, bytesRead);
    ccb_assert(timeOutSec > 0, timeOutSec);

    if (!(pBuffer && (socket_number != SOCKET_ERROR)))
    {
        LogMessage(LOG_TYPE_DEBUG, "ReceiveDataRaw: bad input: pBuffer %p, socket %d",
                   pBuffer, socket_number);

        return SOCKET_ERROR;
    }

    /*
     * Initialize the local buffer pointer to the buffer passed in.
     * The local pointer indexes through the buffer as the data is received.
     */
    pCharBuffer = pBuffer;

    /*
     * Setup time limit. The time passed in is in seconds. 'timeLimit' is
     * based upon K_timel, which is 1/8ths of a second. So to convert the
     * input seconds to something comparable to K_timel, multiply by 8.
     */
    timeLimit = K_timel + (timeOutSec * 8);

    /*
     * Loop reading bytes from the socket until the entire packet has
     * been read or a socket error occurs.
     */
    while ((bytesRead != PI_SOCKET_ERROR) && (totalBytesRead < bytesToRead) &&
           K_timel < timeLimit)
    {
        /* Set up the select and timeout period */
        FD_ZERO(&readSet);
        FD_SET(socket_number, &readSet);
        timeout.tv_sec = 0;
        timeout.tv_usec = SEL_TMO_USEC;

        /* Wait for the select() to fire */
        selectRC = Select(socket_number + 1, &readSet, NULL, NULL, &timeout);

        /*
         * We should never see selectRC>1 here, but if it is, it shouldn't
         * hurt either.
         */
        if (selectRC >= 1)
        {
            bytesRead = recv(socket_number, pCharBuffer, bytesToRead - totalBytesRead, 0);

            if (bytesRead == PI_SOCKET_ERROR)
            {
                /*
                 * Probably should never get EWOULDBLOCK here, but in case
                 * we do, its not fatal, just try again.
                 */
                errorCode = errno;
                if (errorCode == TM_EWOULDBLOCK)
                {
                    bytesRead = 0;
                    TaskSwitch();
                    continue;
                }

                LogMessage(LOG_TYPE_DEBUG, "ReceiveDataRaw: recv() error %d/%s, socket %d",
                           errorCode, GetErrnoString(errorCode), socket_number);

                totalBytesRead = PI_SOCKET_ERROR;
            }
            else if (bytesRead == 0)
            {
                LogMessage(LOG_TYPE_DEBUG, "ReceiveDataRaw: EOF on socket %d, bytesRead = %d, totalBytesRead = %d, bytesToRead = %d",
                           socket_number, bytesRead, totalBytesRead, bytesToRead);

                totalBytesRead = bytesRead = PI_SOCKET_ERROR;
            }
            else
            {
                totalBytesRead += bytesRead;
                pCharBuffer += bytesRead;
                bytesRead = 0;
            }
        }

        /* If the select() errored out, log it, then exit.  */
        else if (selectRC < 0)
        {
            errorCode = errno;

            LogMessage(LOG_TYPE_DEBUG, "ReceiveDataRaw: select() error %d/%s, socket %d",
                       errorCode, GetErrnoString(errorCode), socket_number);

            totalBytesRead = bytesRead = PI_SOCKET_ERROR;
        }
        else
        {
            /* We got a select() timeout, not a problem */
        }
    }

    /*
     * Check to make sure we were able to read all the data.
     * If all the data is not there, timeout!
     */
    if ((totalBytesRead != PI_SOCKET_ERROR) && (totalBytesRead != bytesToRead))
    {
        totalBytesRead = PI_SOCKET_ERROR;
    }

    return (totalBytesRead);
}

/*----------------------------------------------------------------------------
** Function:    SendPacketRaw
**
** Description: Writes an IPC packet to the supplied socket
**
** Inputs:      socket_number      Socket to write to
**              packet      Packet to write
**              timeOutSec  Seconds to allow for send to complete
**
** Returns:     Number of bytes written or SOCKET_ERROR
**
** Note:        This routine is shared with IPC.  There are seperate mutexes
**              for IPC and the X1/TEST/DEBUG (one mutex) ports -- the appropriate
**              one must be locked/unlocked around the call to this routine.
**--------------------------------------------------------------------------*/
INT32 SendPacketRaw(INT32 socket_number, XIO_PACKET *packet, UINT32 timeOutSec)
{
    INT32       rc = 0;
    UINT32      numBytesWritten = 0;
    UINT8      *buffPtr;
    fd_set      writeSet;
    struct timeval selTO;
    UINT32      timeLimit;
    INT32       errorCode;
    struct iovec iov[2];
    INT32       doWritev;
    UINT32      len;

    /* Determine if basic requirements met first */
    ccb_assert(packet != NULL, packet);
    ccb_assert(packet->pHeader != NULL, packet);
    ccb_assert(socket_number != SOCKET_ERROR, packet);

    if (!(packet && packet->pHeader && (socket_number != SOCKET_ERROR)))
    {
        return SOCKET_ERROR;
    }

    /* Figure out basics for use later */
    buffPtr = (UINT8 *)packet->pHeader;
    len = sizeof(*packet->pHeader);

    /* All packets start out with a writev() unless no data segment. */
    if (packet->pPacket)
    {
        doWritev = 1;
        iov[0].iov_base = (caddr_t)buffPtr;
        iov[0].iov_len = sizeof(*packet->pHeader);
        iov[1].iov_base = (caddr_t)packet->pPacket;
        iov[1].iov_len = GetPacketDataLength(packet->pHeader);

        /* Calculate total bytes to send */
        len += iov[1].iov_len;
    }
    else
    {
        doWritev = 0;
    }

    /*
     * Setup time limit. The time passed in is in seconds. 'timeLimit' is
     * based upon K_timel, which is 1/8ths of a second. So to convert the
     * input seconds to something comparable to K_timel, multiply by 8.
     */
    timeLimit = K_timel + (timeOutSec * 8);   /* IPC_TX_TMO; */

    while (K_timel < timeLimit)
    {
        /* Do a select with a timeout period */
        FD_ZERO(&writeSet);
        FD_SET(socket_number, &writeSet);
        selTO.tv_sec = 0;
        selTO.tv_usec = SEL_TMO_USEC;

        rc = Select(socket_number + 1, NULL, &writeSet, NULL, &selTO);

        if (rc > 0)
        {
            /* Determine if writev() or send() */
            if (doWritev)
            {
                /* If total to send > Ethernet frame size, reduce writev() size */
                if (iov[0].iov_len + iov[1].iov_len > MAX_ETHERNET_DATA_SIZE)
                {
                    iov[1].iov_len = MAX_ETHERNET_DATA_SIZE - iov[0].iov_len;
                }
                rc = writev(socket_number, iov, 2);
            }
            else
            {
                rc = send(socket_number, buffPtr, (len - numBytesWritten), 0);
            }

            if (SOCKET_ERROR == rc)
            {
                /*
                 * Check to see what the error is all about.  In the case that
                 * we get a EWOULDBLOCK we should then go back and try again.
                 */
                errorCode = errno;

                if (errorCode != TM_EWOULDBLOCK)
                {
                    LogMessage(LOG_TYPE_DEBUG, "SendPacketRaw: send() error %d/%s, socket %d",
                               errorCode, GetErrnoString(errorCode), socket_number);
                    break;
                }
            }
            else
            {
                /*
                 * Increment the number of bytes read and the the pointer
                 * into the users buffer space
                 */
                numBytesWritten += rc;
                buffPtr += rc;

                /* exit loop if all bytes written */
                if (numBytesWritten >= len)
                {
                    break;
                }

                /* Determine if we should call writev() again or not */
                if (doWritev)
                {
                    if (numBytesWritten < sizeof(*packet->pHeader))
                    {
                        iov[0].iov_base = (caddr_t)buffPtr;
                        iov[0].iov_len = sizeof(*packet->pHeader) - numBytesWritten;

                        /*
                         * This is an unusual case, so print out a message
                         * describing what is going on.
                         */
                        dprintf(DPRINTF_DEFAULT, "SendPacketRaw: Attempting writev() again (%d/%d)\n",
                                iov[0].iov_len, iov[1].iov_len);
                    }

                    /* After the header is sent, switch to using send() */
                    else        /* (numBytesWritten >= iov[0].iov_len) */
                    {
                        doWritev = 0;
                        buffPtr = (UINT8 *)packet->pPacket;
                        buffPtr += (numBytesWritten - iov[0].iov_len);
                    }
                }
            }
        }
        else if (SOCKET_ERROR == rc)
        {
            /* Select had an error */
            errorCode = errno;
            LogMessage(LOG_TYPE_DEBUG, "SendPacketRaw: select() error %d/%s, socket %d",
                       errorCode, GetErrnoString(errorCode), socket_number);
            break;
        }
        else
        {
            /* We got a select() timeout, not a problem */
        }
    }

    if (SOCKET_ERROR != rc)
    {
        rc = numBytesWritten;

        if (numBytesWritten != len)
        {
            LogMessage(LOG_TYPE_DEBUG, "SendPacketRaw: Timeout at %u sec, socket %d",
                       timeOutSec, socket_number);
        }
    }

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    SendPacket
**
** Description: Send the header and data portion of the input packet.
**              Both header and data are encrypted before being sent.
**
** Inputs:      socket_number - TCP/IP socket
**              pPacket - pointer to the packet to send
**
** Returns:     Total number of bytes sent or PI_SOCKET_ERROR.  If the
**              entire packet was not sent an error is returned.
**
**--------------------------------------------------------------------------*/
INT32 SendPacket(INT32 socket_number, XIO_PACKET *pPacket, UINT32 timeOutSec)
{
    INT32       rc;
    INT32       expected_len = sizeof(*pPacket->pHeader) + GetPacketDataLength(pPacket->pHeader);

    /* MD5 the data and header. */
    CreateMD5Signature((IPC_PACKET *)pPacket, piMD5Key);

    /* Lock the send mutex */
    (void)LockMutex(&sendMutex, MUTEX_WAIT);

    /* Send the packet */
    rc = SendPacketRaw(socket_number, pPacket, timeOutSec);

    if (rc == SOCKET_ERROR)
    {
        dprintf(DPRINTF_DEFAULT, "SendPacket: error on socket %d\n", socket_number);
    }
    else if (rc != expected_len)
    {
        dprintf(DPRINTF_DEFAULT, "SendPacket: incomplete send on socket %d (expected %d, sent: %d)\n",
                socket_number, expected_len, rc);

        rc = SOCKET_ERROR;
    }

    /* Unlock the send mutex */
    UnlockMutex(&sendMutex);

    return rc;
}


/*----------------------------------------------------------------------------
** Function:    LogSocketError
**
** Description: Make a log entry if a socket error occurs.
**
** Inputs:      socketError
**              errString
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
void LogSocketError(int socketError, const char *errString)
{
    LOG_MRP    *opLog;

    /* MRSDEBUG: For now don't log "connection refused" error. */
    if (socketError != TM_ECONNREFUSED)
    {
        /*
         * Allocate memory for a log entry and fill in the entry data.
         * Call AsyncEventHandler() to log the error then free the memory
         * used for the log entry.\
         */
        opLog = MallocWC(sizeof(*opLog));

        if (opLog != NULL)
        {
            opLog->mleEvent = LOG_SOCKET_ERROR;
            opLog->mleLength = 0x04;
            opLog->mleData[0] = socketError;

            AsyncEventHandler(NULL, opLog);
            Free(opLog);
        }

        /* Print a debug message to the serial console. */
        dprintf(DPRINTF_DEFAULT, "Socket Error %d/%s - %s\n",
                socketError, GetErrnoString(socketError), errString);
    }
}


/*----------------------------------------------------------------------------
** Function:    SetPortServerClientAddr
**
** Description: Set the pointer to the Port Server client address.
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
void SetPortServerClientAddr(UINT32 clientIPAddr)
{
    portServerClientIPAddr = clientIPAddr;
}

/*----------------------------------------------------------------------------
** Function:    GetPortServerClientAddr
**
** Description: Return the pointer to the Port Server client address.
**
** Inputs:      none
**
**--------------------------------------------------------------------------*/
UINT32 GetPortServerClientAddr(void)
{
    return (portServerClientIPAddr);
}

/*----------------------------------------------------------------------------
** Function:    GetSelectTimeoutBySocket()
**
** Description: Return the select timeout associated with this socket
**
** Inputs:      socket_number
**
** Returns:     the timeout in seconds or PI_SOCKET_ERROR
**
**--------------------------------------------------------------------------*/
UINT32 GetSelectTimeoutBySocket(INT32 socket_number)
{
    UINT16      port = 0;
    struct sockaddr_in sa;
    UINT32      len = sizeof(sa);
    UINT32      timeout = UNKNOWN_PORT_TIMEOUT_DEFAULT;
    INT32       rc;

    if ((rc = getsockname(socket_number, (struct sockaddr *)&sa, &len)) == 0)
    {
        port = ntohs(sa.sin_port);

        switch (port)
        {
            case EWOK_PORT_NUMBER:
            case TEST_PORT_NUMBER:
            case DEBUG_PORT_NUMBER:
                timeout = pi_clients_tmo_receive_get(socket_number);
                break;

            default:
                rc = ERROR;
                break;
        }
    }

    if (rc != GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "GetSelectTimeoutBySocket: Couldn't find port %d on socket %d\n",
                port, socket_number);
    }
#if 0
    else
    {
        dprintf(DPRINTF_DEFAULT, "GetSelectTimeoutBySocket: got port %d timeout of %d sec\n",
                port, timeout);
    }
#endif  /* 0 */

    return timeout;
}

/*----------------------------------------------------------------------------
** Function:    SetSelectTimeoutBySocket()
**
** Description: Set the select timeout associated with this socket
**
** Inputs:      socket_number
**              timeout - in seconds
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
void SetSelectTimeoutBySocket(INT32 socket_number, UINT32 timeout)
{
    UINT16      port = 0;
    struct sockaddr_in sa;
    UINT32      len = sizeof(sa);
    INT32       rc;

    if ((rc = getsockname(socket_number, (struct sockaddr *)&sa, &len)) == 0)
    {
        port = ntohs(sa.sin_port);

        switch (port)
        {
            case EWOK_PORT_NUMBER:
            case TEST_PORT_NUMBER:
            case DEBUG_PORT_NUMBER:
                pi_clients_tmo_receive_set(socket_number, timeout);
                break;

            default:
                rc = ERROR;
                break;
        }
    }

    if (rc != GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "SetSelectTimeoutBySocket: Couldn't find port %d on socket %d\n",
                port, socket_number);
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "Setting port %d timeout to %d seconds\n", port, timeout);
    }

    return;
}

/*----------------------------------------------------------------------------
** Function:    GetErrnoString()
**
** Description: Get the corresponding errno string given the numeric value.
**
** Inputs:      errorNumber
**
** Returns:     pointer to errno string
**
**--------------------------------------------------------------------------*/
const char *GetErrnoString(UINT32 errorNumber)
{
    const char *pStr;

    switch (errorNumber)
    {
        case TM_EPERM:
            pStr = "EPERM";
            break;

        case TM_ENOENT:
            pStr = "ENOENT";
            break;

        case TM_ESRCH:
            pStr = "ESRCH";
            break;

        case TM_EINTR:
            pStr = "EINTR";
            break;

        case TM_EIO:
            pStr = "EIO";
            break;

        case TM_ENXIO:
            pStr = "ENXIO";
            break;

        case TM_EBADF:
            pStr = "EBADF";
            break;

        case TM_ECHILD:
            pStr = "ECHILD";
            break;

        case TM_ENOMEM:
            pStr = "ENOMEM";
            break;

        case TM_EACCES:
            pStr = "EACCES";
            break;

        case TM_EFAULT:
            pStr = "EFAULT";
            break;

        case TM_EEXIST:
            pStr = "EEXIST";
            break;

        case TM_ENODEV:
            pStr = "ENODEV";
            break;

        case TM_ENOTDIR:
            pStr = "ENOTDIR";
            break;

        case TM_EISDIR:
            pStr = "EISDIR";
            break;

        case TM_EINVAL:
            pStr = "EINVAL";
            break;

        case TM_EMFILE:
            pStr = "EMFILE";
            break;

        case TM_ENOSPC:
            pStr = "ENOSPC";
            break;

        case TM_EWOULDBLOCK:
            pStr = "EWOULDBLOCK";
            break;

        case TM_EINPROGRESS:
            pStr = "EINPROGRESS";
            break;

        case TM_EALREADY:
            pStr = "EALREADY";
            break;

        case TM_ENOTSOCK:
            pStr = "ENOTSOCK";
            break;

        case TM_EDESTADDRREQ:
            pStr = "EDESTADDRREQ";
            break;

        case TM_EMSGSIZE:
            pStr = "EMSGSIZE";
            break;

        case TM_EPROTOTYPE:
            pStr = "EPROTOTYPE";
            break;

        case TM_ENOPROTOOPT:
            pStr = "ENOPROTOOPT";
            break;

        case TM_EPROTONOSUPPORT:
            pStr = "EPROTONOSUPPORT";
            break;

        case TM_ESOCKTNOSUPPORT:
            pStr = "ESOCKTNOSUPPORT";
            break;

        case TM_EOPNOTSUPP:
            pStr = "EOPNOTSUPP";
            break;

        case TM_EPFNOSUPPORT:
            pStr = "EPFNOSUPPORT";
            break;

        case TM_EAFNOSUPPORT:
            pStr = "EAFNOSUPPORT";
            break;

        case TM_EADDRINUSE:
            pStr = "EADDRINUSE";
            break;

        case TM_EADDRNOTAVAIL:
            pStr = "EADDRNOTAVAIL";
            break;

        case TM_ENETDOWN:
            pStr = "ENETDOWN";
            break;

        case TM_ENETUNREACH:
            pStr = "ENETUNREACH";
            break;

        case TM_ENETRESET:
            pStr = "ENETRESET";
            break;

        case TM_ECONNABORTED:
            pStr = "ECONNABORTED";
            break;

        case TM_ECONNRESET:
            pStr = "ECONNRESET";
            break;

        case TM_ENOBUFS:
            pStr = "ENOBUFS";
            break;

        case TM_EISCONN:
            pStr = "EISCONN";
            break;

        case TM_ENOTCONN:
            pStr = "ENOTCONN";
            break;

        case TM_ESHUTDOWN:
            pStr = "ESHUTDOWN";
            break;

        case TM_ETOOMANYREFS:
            pStr = "ETOOMANYREFS";
            break;

        case TM_ETIMEDOUT:
            pStr = "ETIMEDOUT";
            break;

        case TM_ECONNREFUSED:
            pStr = "ECONNREFUSED";
            break;

        case TM_EHOSTDOWN:
            pStr = "EHOSTDOWN";
            break;

        case TM_EHOSTUNREACH:
            pStr = "EHOSTUNREACH";
            break;

        default:
            pStr = "??";
            break;
    }

    return pStr;
}

/*----------------------------------------------------------------------------
** Function:    InetToAscii()
**
** Description: Convert a binary IP address to an ascii string.
**
** Inputs:      ipAddr - the ip address to convert
**              pOutputStr - pointer to the location to store the converted
**                           string.
**
** Returns:     void
**
**--------------------------------------------------------------------------*/
void InetToAscii(UINT32 ipAddr, char *pOutputStr)
{
    struct in_addr in = { ipAddr };

    if (pOutputStr)
    {
        strcpy(pOutputStr, inet_ntoa(in));
    }
}

/*----------------------------------------------------------------------------
** Function:    GetIpAddressFromInterface()
**
** Description: Get the configured IP address from the driver.
**
** Inputs:      intfHandle - pointer to the interface handle (BF only)
**                         - interface name (e.g. "eth0") (HN)
**
** Returns:     the IP address (or 0)
**
**--------------------------------------------------------------------------*/
UINT32 GetIpAddressFromInterface(void *intfHandle)
{
    UINT32      ipAddr = 0;

    INT32       rc;
    struct ifreq ifReq;

    if (portSrvSocket == PI_SOCKET_ERROR)
    {
        portSrvSocket = socket(AF_INET, SOCK_STREAM, 0);
    }

    if (portSrvSocket < 0)
    {
        dprintf(DPRINTF_DEFAULT, "GetIpAddressFromInterface: socket() failed with errno %u\n", errno);
    }

    memset(&ifReq, 0, sizeof(ifReq));
    strncpy(ifReq.ifr_name, intfHandle, IFNAMSIZ);

    rc = ioctl(portSrvSocket, SIOCGIFADDR, &ifReq);

    if (rc)
    {
        Close(portSrvSocket);
        portSrvSocket = PI_SOCKET_ERROR;

        dprintf(DPRINTF_DEFAULT, "GetIpAddressFromInterface: ioctl() failed with errno %u\n", errno);
    }
    else
    {
        ipAddr = ((struct sockaddr_in *)&ifReq.ifr_addr)->sin_addr.s_addr;
    }

    return ipAddr;
}

/*----------------------------------------------------------------------------
** Function:    GetSubnetFromInterface()
**
** Description: Get the configured subnet mask from the driver.
**
** Inputs:      intfHandle - pointer to the interface handle (BF only)
**                         - interface name (e.g. "eth0") (HN)
**
** Returns:     the subnet mask (or 0)
**
**--------------------------------------------------------------------------*/
UINT32 GetSubnetFromInterface(void *intfHandle)
{
    UINT32      ipAddr = 0;

    INT32       rc;
    struct ifreq ifReq;

    if (portSrvSocket == PI_SOCKET_ERROR)
    {
        portSrvSocket = socket(AF_INET, SOCK_STREAM, 0);
    }

    if (portSrvSocket < 0)
    {
        dprintf(DPRINTF_DEFAULT, "GetSubnetFromInterface: socket() failed with errno %u\n", errno);
    }

    memset(&ifReq, 0, sizeof(ifReq));
    strncpy(ifReq.ifr_name, intfHandle, IFNAMSIZ);

    rc = ioctl(portSrvSocket, SIOCGIFNETMASK, &ifReq);

    if (rc)
    {
        Close(portSrvSocket);
        portSrvSocket = PI_SOCKET_ERROR;

        dprintf(DPRINTF_DEFAULT, "GetSubnetFromInterface: ioctl() failed with errno %u\n", errno);
    }
    else
    {
        ipAddr = ((struct sockaddr_in *)&ifReq.ifr_netmask)->sin_addr.s_addr;
    }

    return ipAddr;
}

/*----------------------------------------------------------------------------
** Function:    GetMacAddrFromInterface()
**
** Description: Get the interface MAC address from the driver.
**
** Inputs:      intfHandle - pointer to the interface handle (BF only)
**                         - interface name (e.g. "eth0") (HN)
**
** Returns:     the MAC address (all 1's if failure)
**
**--------------------------------------------------------------------------*/
ETHERNET_MAC_ADDRESS GetMacAddrFromInterface(void *intfHandle)
{
    ETHERNET_MAC_ADDRESS mac = { {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} };

    INT32       rc;
    struct ifreq ifReq;

    if (portSrvSocket == PI_SOCKET_ERROR)
    {
        portSrvSocket = socket(AF_INET, SOCK_STREAM, 0);
    }

    if (portSrvSocket < 0)
    {
        dprintf(DPRINTF_DEFAULT, "GetMacAddrFromInterface: socket() failed with errno %u\n", errno);
    }

    memset(&ifReq, 0, sizeof(ifReq));
    strncpy(ifReq.ifr_name, intfHandle, IFNAMSIZ);

    rc = ioctl(portSrvSocket, SIOCGIFHWADDR, &ifReq);

    if (rc)
    {
        Close(portSrvSocket);
        portSrvSocket = PI_SOCKET_ERROR;

        dprintf(DPRINTF_DEFAULT, "GetMacAddrFromInterface: ioctl() failed with errno %u\n", errno);
    }
    else
    {
        memcpy(&mac, &ifReq.ifr_hwaddr.sa_data, sizeof(mac));
    }

    return mac;
}

/*----------------------------------------------------------------------------
** Function:    GetGatewayFromInterface()
**
** Description: Get the configured default gateway from the driver.
**
** Inputs:      intfHandle   - interface name (e.g. "eth0") (HN)
**
** Returns:     the default gateway (or 0).
**
**--------------------------------------------------------------------------*/
UINT32 GetGatewayFromInterface(void *intfHandle)
{
    UINT32      ipAddr = 0;
    INT32       rc;

    FILE       *pF;
    char        pLine[256];     /* this should be more than we need */
    char        pIntf[9];
    UINT32      dest;
    UINT32      gateway;

    /*
     * Read and parse /proc/net/route to get default gateway
     */
    /* dprintf(DPRINTF_DEFAULT, "GetGatewayFromInterface: parsing /proc/net/route\n"); */
    pF = fopen("/proc/net/route", "r");
    if (pF)
    {
        while (fgets(pLine, 256, pF))
        {
            rc = sscanf(pLine, "%8s %x %x", pIntf, &dest, &gateway);
            /* dprintf(DPRINTF_DEFAULT, "GetGatewayFromInterface: rc: %d %s %u %u\n",
             * rc, pIntf, dest, gateway); */

            if ((rc == 3) && (strcmp(pIntf, intfHandle) == 0) && (dest == 0))
            {
                ipAddr = gateway;
                break;
            }
        }

        Fclose(pF);
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "GetGatewayFromInterface: couldn't open /proc/net/route, errno %d\n",
                errno);
    }

    return ipAddr;
}

/*----------------------------------------------------------------------------
** Function:    GetLinkStatusFromInterface()
**
** Description: Get the link status flags from the driver.
**
** Inputs:      intfHandle   - interface name (e.g. "eth0") (HN)
**
** Returns:     'ifr_flags' as defined in net/if.h (or -1 on failure)
**
**--------------------------------------------------------------------------*/
INT32 GetLinkStatusFromInterface(void *intfHandle)
{
    INT32       flags = -1;

    INT32       rc;
    struct ifreq ifReq;

    if (portSrvSocket == PI_SOCKET_ERROR)
    {
        portSrvSocket = socket(AF_INET, SOCK_STREAM, 0);
    }

    if (portSrvSocket < 0)
    {
        dprintf(DPRINTF_DEFAULT, "GetLinkStatusFromInterface: socket() failed with errno %u\n", errno);
    }

    memset(&ifReq, 0, sizeof(ifReq));
    strncpy(ifReq.ifr_name, intfHandle, IFNAMSIZ);

    rc = ioctl(portSrvSocket, SIOCGIFFLAGS, &ifReq);

    if (rc)
    {
        Close(portSrvSocket);
        portSrvSocket = PI_SOCKET_ERROR;

        dprintf(DPRINTF_DEFAULT, "GetLinkStatusFromInterface: ioctl() failed with errno %u\n", errno);
    }
    else
    {
        flags = (INT16)ifReq.ifr_flags;
    }

    return (INT32)flags;
}

/*----------------------------------------------------------------------------
** Function:    SetBlockingState()
**
** Description: Set/Clr the socket I/O nonblocking flag.
**
** Inputs:      sockfd - the socket descriptor to modify
**              blockingState - BLOCKING_OFF or BLOCKING_ON
**
** Returns:     GOOD on success, PI_SOCKET_ERROR on failure
**
**--------------------------------------------------------------------------*/
INT32 SetBlockingState(INT32 sockFd, INT32 blockingState)
{
    INT32       rc;

    INT32       on = (blockingState == BLOCKING_OFF) ? 1 : 0;

    rc = ioctl(sockFd, FIONBIO, &on);

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    setkeepalive
**
** Description: Set the tcp keepalive options.
**
** Inputs:      sockfd - the socket descriptor to modify
**
** Returns:     None, errors are printed and ignored.
**
**--------------------------------------------------------------------------*/

#define KEEPIDLE    3       /* Check if alive, 3 seconds after last data sent. */
#define KEEPINTVL   3       /* Each retry is 3 seconds apart. */
#define KEEPCNT     4       /* Retry for 4 times. */
// 3 second from last data, plus 3 * 4  => 3+12 => 15 second timeout.

void setkeepalive(SOCKET sockFd)
{
    INT32           err;
    int             optval;
    int             on = 1;

    /* Set the keepalive option active */
    err = setsockopt(sockFd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));
    if (err < 0)
    {
        perror("setsockopt(SO_KEEPALIVE)");
        fprintf(stderr, "socket=%d\n", sockFd);
    }

    optval = KEEPIDLE;
    err = setsockopt(sockFd, SOL_TCP, TCP_KEEPIDLE, &optval, sizeof(optval));
    if (err < 0)
    {
        perror("setsockopt(TCP_KEEPIDLE)");
        fprintf(stderr, "socket=%d\n", sockFd);
    }

    optval = KEEPCNT;
    err = setsockopt(sockFd, SOL_TCP, TCP_KEEPCNT, &optval, sizeof(optval));
    if (err < 0)
    {
        perror("setsockopt(TCP_KEEPCNT)");
        fprintf(stderr, "socket=%d\n", sockFd);
    }

    optval = KEEPINTVL;
    err = setsockopt(sockFd, SOL_TCP, TCP_KEEPINTVL, &optval, sizeof(optval));
    if (err < 0)
    {
        perror("setsockopt(TCP_KEEPINTVL)");
        fprintf(stderr, "socket=%d\n", sockFd);
    }
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
