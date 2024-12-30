/* $Id: PortSrv.c 149941 2010-11-03 21:38:18Z m4 $ */
/*===========================================================================
** FILE NAME:       PortServer.c
** MODULE TITLE:    CCB Port Server
**
** DESCRIPTION:     Server that handles packet communication between the
**                  CCB and the XMC.
**
** Copyright (c) 2001-2010 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "PortServer.h"

#include "AsyncClient.h"
#include "CmdLayers.h"
#include "cps_init.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "MR_Defs.h"
#include "ipc_security.h"
#include "kernel.h"
#include "led.h"
#include "logdef.h"
#include "logging.h"
#include "LogOperation.h"
#include "timer.h"
#include "md5.h"
#include "misc.h"
#include "nvram.h"
#include "PacketInterface.h"
#include "PacketStats.h"
#include "PI_CmdHandlers.h"
#include "PI_ClientPersistent.h"
#include "PI_Clients.h"
#include "PI_Utils.h"
#include "PortServerUtils.h"
#include "quorum_utils.h"
#include "realtime.h"
#include "trace.h"
#include "X1_AsyncEventHandler.h"
#include "X1_Packets.h"
#include "X1_Structs.h"
#include "XIO_Std.h"
#include "XIO_Const.h"

/*****************************************************************************
 ** Private defines
 *****************************************************************************/
#define ETH_FRAME_SIZE (1448)
#define X1S_RCV_BUF_SIZE    (ETH_FRAME_SIZE * 70)       /* 70 eth frames, ~100K */
#define X1S_RCV_BUF_DEFAULT (ETH_FRAME_SIZE * 6)        /* 6 ethernet frames */

#define MAX_PI_CLIENT_PORTS         30
#define MAX_TEST_PORTS              20

/*****************************************************************************
 ** Private variables
 *****************************************************************************/
static UINT8 portServerRunning = TRUE;
static UINT32 npiports = 0;     /* Number of PI port connections */
static UINT32 ndebugports = 0;  /* Number of diag port connections */
static UINT32 ntestports = 0;   /* Number of test port connections */

/*****************************************************************************
 ** Public variables - externed in the header file
 *****************************************************************************/
UINT32      diagPortTimeout = 0;
UINT32      gSysIP = 0;
MUTEX       gPICommandMutex;

/*****************************************************************************
 ** Private function prototypes
 *****************************************************************************/
static void GetPortNumberString(UINT16 port, char *str);
static INT32 GetPortDefaultTimeout(UINT16 port);
static INT32 PacketInterfaceServer(pi_client_info *pinfo);

static INT32 CreateListeningSocket(UINT32 inetAddress, UINT16 port);
static INT32 ValidateHeader(XIO_PACKET *reqPacket);
static void StartPIServer(INT32 port, INT32 socket_number, UINT32 ipaddr);
static void PIServerThread(TASK_PARMS *parms);

/*****************************************************************************
 ** Public variables - not externed in header files.
 *****************************************************************************/
extern time_t skip_login_logout_msg_until;

/*****************************************************************************
 ** Code Start
 *****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    ValidateHeader
**
** Description: Validate the packet header by checking that the command
**              code is in range, the security checks OK, etc.
**
** Inputs:      pHeader - pointer to a packet header
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ValidateHeader(XIO_PACKET *reqPacket)
{
    PI_PACKET_HEADER *pHeader = reqPacket->pHeader;

    if (CheckHeaderMD5((IPC_PACKET *)reqPacket, piMD5Key) == FALSE)
    {
        dprintf(DPRINTF_PI_PROTOCOL, "ValidateHeader: header MD5 incorrect\n");
        return(PI_ERROR);
    }

    if ((pHeader->headerLength != sizeof(*pHeader)))
    {
        dprintf(DPRINTF_DEFAULT, "ValidateHeader: header length incorrect (%u)\n",
                pHeader->headerLength);
        return(PI_ERROR);
    }

    if ((pHeader->protocolLevel != PKT_SERVER_PROTOCOL_VER))
    {
        dprintf(DPRINTF_DEFAULT, "ValidateHeader: protocol version incorrect (%u)\n",
                pHeader->protocolLevel);
        return(PI_ERROR);
    }

    return(PI_GOOD);
}

/**
******************************************************************************
**
**  @brief      Get the port number as a string.
**
**  @param      UINT16 port - port number
**  @param      UINT8* str - Buffer in which to copy the port number string.
**
**  @return     none
**
**  @attention  The output buffer must be 8 bytes.
**
******************************************************************************
**/
static void GetPortNumberString(UINT16 port, char *str)
{
    if (port == EWOK_PORT_NUMBER)
    {
        strcpy(str, "EWOK");
    }
    else if (port == TEST_PORT_NUMBER)
    {
        strcpy(str, "TEST");
    }
    else if (port == DEBUG_PORT_NUMBER)
    {
        strcpy(str, "DEBUG");
    }
    else
    {
        strcpy(str, "UNKNOWN");
    }
}


/**
******************************************************************************
**
**  @brief      Get the port default timeout.
**
**  @param      UINT16 port - port number
**
**  @return     INT32 - Default timeout value
**
******************************************************************************
**/
static INT32 GetPortDefaultTimeout(UINT16 port)
{
    switch (port)
    {
        case EWOK_PORT_NUMBER:
        case TEST_PORT_NUMBER:
        case DEBUG_PORT_NUMBER:
            return(PI_PORT_TIMEOUT_DEFAULT);

        default:
            return(UNKNOWN_PORT_TIMEOUT_DEFAULT);
    }
}


/*----------------------------------------------------------------------------
** Function:    DiagPortsDisabled()
**
** Description: Checks to see if the Diag Ports (3100/3200) are open
**              or if they should be timed out and closed.
**
** Inputs:      void
**
** Returns:     TRUE/FALSE
**
**--------------------------------------------------------------------------*/
static UINT32 DiagPortsDisabled(void)
{
    if (TestModeBit(MD_DIAG_PORTS_ENABLE) == 0)
    {
        return(TRUE);
    }
    if ((diagPortTimeout) && (K_timel > diagPortTimeout))
    {
        ClrModeBit(MD_DIAG_PORTS_ENABLE);
        diagPortTimeout = 0;
        return(TRUE);
    }
    return(FALSE);
}


/*----------------------------------------------------------------------------
** Function:    PacketInterfaceServer()
**
** Description: Performs the following
**              1. Read and validate header
**              2. Send "in progress" status
**              3. Read request packet (if required)
**              4. Call command handler to process the request
**              5. Send response data
**              6. Send completion status
**
** Inputs:      pi_client_info* pinfo - Pointer to the client information.
**
** Returns:     Number of bytes send on last socket operation or
**              PI_SOCKET_ERROR
**
** NOTE:        This function is CALLED ONLY FROM DebugServer().
**--------------------------------------------------------------------------*/
static INT32 PacketInterfaceServer(pi_client_info *pinfo)
{
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    INT32       socketRC = 0;
    INT32       rc = PI_GOOD;
    UINT32      sysIP = GetIPAddress();
    UINT16      port = pinfo->port;
    INT32       socket_number = pinfo->sockfd;
    UINT8       pi_lock_held = 0;

    /* This while loops forever unless a socket error occurs. */
    while ((socketRC != PI_SOCKET_ERROR) && (gSysIP == sysIP))
    {
        /* Check if the client is in an error state. */
        if (pinfo->berror == TRUE)
        {
            socketRC = PI_SOCKET_ERROR;
            break;
        }

        /*
         * Check to see if the diag ports have been disabled. We check it here
         * (after command completion) and immediately after the initial read,
         * as that is where we would have been blocked.
         */
        if (port != EWOK_PORT_NUMBER && DiagPortsDisabled())
        {
            socketRC = PI_SOCKET_ERROR;
            break;
        }

        /* Initialize header and data pointers. */
        reqPacket.pHeader = NULL;
        reqPacket.pPacket = NULL;
        rspPacket.pHeader = NULL;
        rspPacket.pPacket = NULL;

        /* Allocate memory for the headers. */
        reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
        rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));

        /* Read the header. If received correctly continue with protocol. */
        socketRC = ReceiveData(socket_number, (UINT8 *)reqPacket.pHeader,
                               sizeof(*reqPacket.pHeader), pinfo->tmo_receive);

        /*
         * Check to see if the diag ports have been disabled. We check it here
         * (after the initial read), and after command completion.
         */
        if (port != EWOK_PORT_NUMBER && DiagPortsDisabled())
        {
            socketRC = PI_SOCKET_ERROR;
            break;
        }

        if (socketRC != PI_SOCKET_ERROR)
        {
            /*
             * If we already have the lock, and do not want to process any
             * more PI commands, wait forever.
             */
            while (pi_lock_held != 0)
            {
                dprintf(DPRINTF_DEFAULT, "Socket already has PI lock, but must not execute PI commands\n");
                TaskSleepMS(SOCKET_RETRY_DELAY);  /* wait for one second. */
            }

            /*
             * Lock the PI command mutex for not allowing
             * other connections to give commands simultaneously
             */
            (void)LockMutex(&gPICommandMutex, MUTEX_WAIT);

            /* Set flag that PI lock is held by us (see above). */
            pi_lock_held = 1;

            /*
             * Validate the header - the return is checked below.
             * NOTE: this should be handled differently, as we should not
             * be copying and returning an invalid header. But this will
             * be re-evaluated when MD5 is added back in.
             */
            rc = ValidateHeader(&reqPacket);

            /* Fill in the socket after the MD5 has been validated */
            reqPacket.pHeader->socket = socket_number;

            /*
             * Copy the request header into the response header. Set the
             * length to 0 since no data will follow.
             */
            memcpy(rspPacket.pHeader, reqPacket.pHeader, sizeof(*reqPacket.pHeader));
            rspPacket.pHeader->length = 0;

            /*
             * If the header is valid indicate "in progress" status to
             * the requester. Otherwise send the return code from the
             * header validation.
             */
            if (rc == PI_GOOD)
            {
                TraceEvent(TRACE_PACKET_START, reqPacket.pHeader->commandCode);
                IncPacketCount(PACKET_TYPE_PI, reqPacket.pHeader->commandCode);
                rspPacket.pHeader->status = PI_IN_PROGRESS;
            }
            else
            {
                rspPacket.pHeader->status = rc;
            }

            /* Send an initial status header to the requester. */
            socketRC = SendPacket(socket_number, &rspPacket, TMO_PI_SEND);

            /*
             * If the header is invalid force a socket error after sending
             * initial status. This will cause PacketInterfaceServer()
             * to return.
             */
            if (rc != PI_GOOD)
            {
                socketRC = PI_SOCKET_ERROR;
            }

            /*
             * If SendHeader() was successful and there is
             * a request packet to read, go get it.
             */
            if ((socketRC != PI_SOCKET_ERROR) && (reqPacket.pHeader->length > 0))
            {
                switch (reqPacket.pHeader->commandCode)
                {
                        /*
                         * Data will be pulled down in the command handler.
                         * Pass the socket ID as the data payload.
                         */
                    case PI_FIRMWARE_DOWNLOAD_CMD:
                    case PI_WRITE_BUFFER_MODE5_CMD:
                    case PI_TRY_CCB_FW_CMD:
                        reqPacket.pPacket = MallocWC(sizeof(INT32));
                        *(INT32 *)reqPacket.pPacket = socket_number;
                        break;

                    default:
                        reqPacket.pPacket = MallocSharedWC(reqPacket.pHeader->length);

                        socketRC = ReceiveData(socket_number, reqPacket.pPacket,
                                               reqPacket.pHeader->length,
                                               TMO_PI_RECEIVE_DATA);

                        if (socketRC != PI_SOCKET_ERROR)
                        {
                            if (CheckDataMD5((IPC_PACKET *)&reqPacket) == FALSE)
                            {
                                dprintf(DPRINTF_PI_PROTOCOL, "PacketInterfaceServer: data MD5 incorrect\n");
                                socketRC = PI_SOCKET_ERROR;
                            }
                        }
                        else
                        {
                            socketRC = PI_SOCKET_ERROR;
                        }
                        break;
                }
            }

            /*
             * If the previous socket operations were successful call the
             * command handler to process the request.
             */
            if (socketRC != PI_SOCKET_ERROR)
            {
                /*
                 * Since we are handling the packet from somewhere externally
                 * we want to make sure that our last access time is updated.
                 */
                RefreshLastAccess();

                switch (reqPacket.pHeader->commandCode)
                {
                        /*
                         * Pass the socket to the generic2 command handler.
                         * It sends the response packet from there.
                         *
                         * The structure display command also sends the response
                         * so make sure it is handled separately.
                         */
                    case PI_GENERIC2_CMD:
                        ((PI_GENERIC_REQ *)(reqPacket.pPacket))->socket = socket_number;
                        /* Fall through after setting up the socket.  */
                    case PI_DEBUG_STRUCT_DISPLAY_CMD:
                        rc = PortServerCommandHandler(&reqPacket, &rspPacket);
                        break;

                        /*
                         * For all other commands, call the handler and send the
                         * response from here.
                         */
                    default:
                        rc = PortServerCommandHandler(&reqPacket, &rspPacket);

                        socketRC = SendPacket(socket_number, &rspPacket, TMO_PI_SEND);
                        break;
                }
            }

            /*
             * Unlock the PI command mutex as it is
             * serviced the command.
             */
            switch (reqPacket.pHeader->commandCode)
            {
                    /*
                     * Do not unlock the mutex. These commands should block
                     * the other connections
                     */
                case PI_VCG_SHUTDOWN_CMD:
                case PI_RESET_CMD:
                case PI_MFG_CTRL_CLEAN_CMD:
                    /*
                     * If the command failed, unlock mutex.
                     * Otherwise do not accept any more commands.
                     */
                    if (rc != PI_GOOD)
                    {
                        UnlockMutex(&gPICommandMutex);
                        /* Set flag that our PI lock is freed. */
                        pi_lock_held = 0;
                    }
                    break;

                    /*
                     * For the fail controller case there are a couple of options
                     * that need to be handled. If the fail command was issued
                     * to fail this controller then keep the mutex locked as the
                     * controller will be failed shortly. If fail command was
                     * issued to fail another controller then release the mutex.
                     */
                case PI_VCG_FAIL_CONTROLLER_CMD:
                    if (((PI_VCG_FAIL_CONTROLLER_REQ *)reqPacket.pPacket)->serialNumber == GetMyControllerSN())
                    {
                        /* Do nothing */
                        break;
                    }

                    /*
                     * When failing another controller fall through to unlock
                     * the mutex.
                     */

                default:
                    UnlockMutex(&gPICommandMutex);
                    /* Set flag that our PI lock is freed. */
                    pi_lock_held = 0;
                    break;
            }
        }

        TraceEvent(TRACE_PACKET + rspPacket.pHeader->status,
                   rspPacket.pHeader->commandCode);

        /*
         * Free memory before returning. In the case of the response data
         * packet be sure the request did not time out.
         */
        if (reqPacket.pHeader != NULL)
        {
            Free(reqPacket.pHeader);
        }

        if (rspPacket.pHeader != NULL)
        {
            Free(rspPacket.pHeader);
        }

        if ((reqPacket.pPacket != NULL) && (rc != PI_TIMEOUT))
        {
            Free(reqPacket.pPacket);
        }

        if ((rspPacket.pPacket != NULL) && (rc != PI_TIMEOUT))
        {
            Free(rspPacket.pPacket);
        }
    }

    return (socketRC);
}


/*----------------------------------------------------------------------------
** Function:    CreateListeningSocket()
**
** Description: Use calls in the Treck TCP/IP library to create a socket
**              and bind it to a port.
**
** Inputs:      inetAddress - IP address for the socket
**              port - port to create the socket on
**
** Returns:     INT32 - socket descriptor or -1 on error
**
**--------------------------------------------------------------------------*/
static INT32 CreateListeningSocket(UINT32 inetAddress, UINT16 port)
{
    INT32       newSocket = 0;
    struct sockaddr_in s_in;
    INT32       socketRC = PI_SOCKET_ERROR;
    INT32       errorLoc;       /* Error location in this function */
    INT32       reuseaddrOption = SO_REUSEADDR;
    struct linger lingerOption = { 1, 10 };
    char       *errString = NULL;

#ifdef DISABLE_NAGLE
    INT32       tcpOption = 1;
#endif  /* DISABLE_NAGLE */

    /*
     * Create a new socket. If this call is successful, continue to
     * the next step in the process.
     */
    errorLoc = 0;
    newSocket = socket(AF_INET, SOCK_STREAM, 0);
    socketRC = newSocket;

    if (socketRC != PI_SOCKET_ERROR)
    {
        /*
         * Enable local address reuse for the socket. This prevents
         * bind() from failing if the socket was closed following a
         * failure in accept() below.
         */
        socketRC = setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR,
                              (void *)&reuseaddrOption, sizeof(reuseaddrOption));
        errorLoc = 1;
    }

#ifdef DISABLE_NAGLE
    if ((socketRC != PI_SOCKET_ERROR))
    {
        /* Disable Nagle. */
        socketRC = setsockopt(newSocket, IPPROTO_TCP, TCP_NODELAY,
                              (void *)&tcpOption, sizeof(tcpOption));
        errorLoc = 4;
    }
#endif  /* DISABLE_NAGLE */

    if (socketRC != PI_SOCKET_ERROR)
    {
        /*
         * Set the linger option for the socket to ensure it
         * attempts to flush any data before it is closed.
         */
        socketRC = setsockopt(newSocket, SOL_SOCKET, SO_LINGER,
                              (void *)&lingerOption, sizeof(lingerOption));
        errorLoc = 5;
    }

    if (socketRC != PI_SOCKET_ERROR)
    {
        /* Bind the socket to the port */
        memset(&s_in, 0, sizeof(s_in));
        s_in.sin_family = AF_INET;
        s_in.sin_addr.s_addr = inetAddress;
        s_in.sin_port = htons(port);

        socketRC = bind(newSocket, (struct sockaddr *)&s_in, sizeof(s_in));
        errorLoc = 2;
    }

    if (socketRC != PI_SOCKET_ERROR)
    {
        /* Listen with a queue of up to LISTENING_QUEUE_SIZE connections. */
        socketRC = listen(newSocket, LISTENING_QUEUE_SIZE);
        errorLoc = 3;
    }

    /*
     * If any of the previous calls failed, log an error,
     * close the socket, wait a while and try again.
     */
    if (socketRC == PI_SOCKET_ERROR)
    {
        /* Save errno so that no system calls can change it. */
        int         save_errno = errno;

        errString = MallocWC(200);

        sprintf(errString, "CreateListeningSocket() errorLoc = %d", errorLoc);

        /* Restore errno for socket error logging. */
        errno = save_errno;
        LogSocketError(save_errno, errString);
        Free(errString);

        Close(newSocket);
        TaskSleepMS(SOCKET_RETRY_DELAY);
    }
    else
    {
        /* If the socket was properly set up, set the blocking state. */
        socketRC = SetBlockingState(newSocket, BLOCKING_ON);
    }

    return newSocket;
}


/*----------------------------------------------------------------------------
** Function:   PIServerThread()
**
** Description: This is the task responsible for each connection
**
** Inputs:      none
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
static void PIServerThread(TASK_PARMS *parms)
{
    INT32       rc = 0;
    pi_client_info *pinfo = (pi_client_info *)parms->p1;
    UINT8       maxportsreached = FALSE;

    switch (pinfo->port)
    {
        case EWOK_PORT_NUMBER:
            npiports++;
            if (npiports > MAX_PI_CLIENT_PORTS)
            {
                dprintf(DPRINTF_DEFAULT, "PIServerThread: pi ports are greater than 30\n");
                maxportsreached = TRUE;
            }
            break;

        case TEST_PORT_NUMBER:
            ntestports++;
            if (ntestports > MAX_TEST_PORTS)
            {
                dprintf(DPRINTF_DEFAULT, "PIServerThread: test ports are greater than 20\n");
                maxportsreached = TRUE;
            }
            break;

        case DEBUG_PORT_NUMBER:
            ndebugports++;
            if (ndebugports > MAX_TEST_PORTS)
            {
                dprintf(DPRINTF_DEFAULT, "PIServerThread: pi debug ports are greater than 20\n");
                maxportsreached = TRUE;
            }
            break;

        default:
            dprintf(DPRINTF_DEFAULT, "PIServerThread: Unknown port request %d\n",
                    pinfo->port);
    }

    if (maxportsreached == FALSE)
    {
        rc = PacketInterfaceServer(pinfo);
    }

    switch (pinfo->port)
    {
        case EWOK_PORT_NUMBER:
            npiports--;
            break;

        case TEST_PORT_NUMBER:
            ntestports--;
            break;

        case DEBUG_PORT_NUMBER:
            ndebugports--;
            break;

        default:
            dprintf(DPRINTF_DEFAULT, "PIServerThread: Unknown port  %d request terminating\n",
                    pinfo->port);
    }

    /* Remove all locks of the records belong to this socket */
    RemoveAllLocks(pinfo->sockfd);

    if (pinfo->sockfd != SOCKET_ERROR)
    {
        /*
         * unregister clientevents which are registered
         * through this socket
         */
        pi_clients_register_events_set(pinfo->sockfd, 0);
    }

    /*
     * Remove the client connection from the list since it is being closed.
     * This function returns a pointer to the client connection but we can
     * ignore the return since we still have a pointer to it.
     */
    pi_clients_remove(pinfo);

    /* Close the client socket if the file descriptor is still valid. */
    if (pinfo->sockfd != SOCKET_ERROR)
    {
        pi_clients_close(pinfo);
    }

    Free(pinfo);
}


/*----------------------------------------------------------------------------
** Function:    StartPIServer()
**
** Description: This is the function creates a task for every new connection
**
** Inputs:      port, socket, ipaddr
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
static void StartPIServer(INT32 port, INT32 socket_number, UINT32 ipaddr)
{
    TASK_PARMS  parms;
    pi_client_info *pinfo;

    dprintf(DPRINTF_DEFAULT, "StartPIServer-ENTER (port: %d, sockfd: %d, ipaddr: %08x)\n",
            port, socket_number, ipaddr);

    pinfo = pi_clients_add(ipaddr, port, socket_number, GetPortDefaultTimeout(port));

    parms.p1 = (UINT32)pinfo;
    TaskCreate(PIServerThread, &parms);

    dprintf(DPRINTF_DEFAULT, "StartPIServer-EXIT (sockfd: %d)\n", socket_number);
}


/*----------------------------------------------------------------------------
** Function:    DebugServer()
**
** Description: This is the main task that implements the TCP/IP port
**              between the CCB and the XMC
**
** Inputs:      none
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
void DebugServer(TASK_PARMS *parms)
{
    UINT16      port = (UINT16)parms->p1;
    INT32       serverSocket = PI_SOCKET_ERROR;
    INT32       clientSocket = PI_SOCKET_ERROR;
    struct sockaddr_in pin;
    UINT32      addrlen = sizeof(pin);
    INT32       serverSocketRC = PI_SOCKET_ERROR;
    INT32       clientSocketRC = PI_SOCKET_ERROR;
    char        ipAddrStr[16];
    LOG_MRP     opLog;          /* Size = 264 bytes */
    char        portStr[8];
    UINT8       firsttime = 1;
    UINT32      sysIP = 0;
    UINT8       killDebugServer = false;
    time_t      cur_time = 0;

    /* Get the port number string so it can be displayed in print messages. */
    GetPortNumberString(port, portStr);

    /*
     * Create a socket, accept a connection and process packets. If any
     * errors occur attempt to re-establish the server socket.
     */
    while (TRUE)
    {
        if (port != EWOK_PORT_NUMBER && DiagPortsDisabled())
        {
            TaskSleepMS(WAIT_FOR_DIAG_ENABLE);  /* check every 15 seconds */
            continue;
        }

        while (TRUE)
        {
            /* Open up a server socket */
            if ((gSysIP != sysIP) && !firsttime)
            {
                killDebugServer = true;
                dprintf(DPRINTF_DEFAULT, "IP Change: Kill The DebugServer.\n");
                break;
            }

            if (firsttime)
            {
                sysIP = GetIPAddress();
                gSysIP = sysIP;
                serverSocketRC = serverSocket = CreateListeningSocket(sysIP, port);

                /* Set the socket blocking before accepting it */
                serverSocketRC = SetBlockingState(serverSocket, BLOCKING_ON);
                if (serverSocketRC == PI_SOCKET_ERROR)
                {
                    break;
                }

                firsttime = 0;
            }

            /* Go do the accept */
            clientSocket = Accept(serverSocket, (struct sockaddr *)&pin, &addrlen);
            serverSocketRC = clientSocket;
            if (serverSocketRC == PI_SOCKET_ERROR)
            {
                break;
            }

            /*
             * If the modebit got disabled while we were on the accept,
             * bail out now.
             */
            if (port != EWOK_PORT_NUMBER && DiagPortsDisabled())
            {
                break;
            }

            /* Print a debug message to the serial console. */
            strncpy(ipAddrStr, inet_ntoa(pin.sin_addr), 15);
            ipAddrStr[15] = 0;

            dprintf(DPRINTF_DEFAULT, "PI: Client connection from IP %s to port %hu OPENED (%s) on socket %hu.\n",
                    ipAddrStr, port, portStr, clientSocket);

            /* Make a log entry */
            cur_time = time(NULL);
            if (cur_time > skip_login_logout_msg_until)
            {
                opLog.mleEvent = LOG_CONFIGURATION_SESSION_START;
            }
            else
            {
                opLog.mleEvent = LOG_CONFIGURATION_SESSION_START_DEBUG;
            }
            opLog.mleLength = 8;
            ((LOG_SESSION_START_STOP_PKT *)(opLog.mleData))->port = port;
            ((LOG_SESSION_START_STOP_PKT *)(opLog.mleData))->flags = 0;
            ((LOG_SESSION_START_STOP_PKT *)(opLog.mleData))->IPAddress = (UINT32)(pin.sin_addr.s_addr);
            AsyncEventHandler(NULL, &opLog);

            /* Set to non-blocking i/o on the client socket. */
            clientSocketRC = SetBlockingState(clientSocket, BLOCKING_OFF);
            if (clientSocketRC != PI_SOCKET_ERROR)
            {
                /* Set the tcp keepalive options. */
                setkeepalive(clientSocket);

                /* Call the packet interface server function inline */
                StartPIServer(port, clientSocket, pin.sin_addr.s_addr);
            }
        }

        if (killDebugServer == true)
        {
            return;
        }

        /*
         * Close the client socket here since we are going back on
         * an accept() and will get a new one on the next connect().
         * Set the Port Server Client address to 0 so that the
         * Async Client will not try to connect.
         */
        if (serverSocket != PI_SOCKET_ERROR)
        {
            Close(serverSocket);
            serverSocket = PI_SOCKET_ERROR;
            firsttime = 1;
        }

        if (clientSocket != PI_SOCKET_ERROR)
        {
            pi_clients_register_events_set(clientSocket, 0);
            Close(clientSocket);
            clientSocket = PI_SOCKET_ERROR;
        }

        /*
         * If a socket error occurred, close the socket before attempting
         * to create a new one via CreateListeningSocket().
         */
        TaskSleepMS(SOCKET_RETRY_DELAY);
    }
}


/*----------------------------------------------------------------------------
** Function:    ShutdownPortServer()
**
** Description: Shuts down the port server.
**
** Inputs:      NONE
**
** Returns:     NONE
**
** Warning:     This will not immediately shutdown the port server. But will
**              shut it down on the next request, when the select times out,
**              or when the client disconnects.
**--------------------------------------------------------------------------*/
void ShutdownPortServer(void)
{
    /* Set the flag to shut down the port server */
    portServerRunning = FALSE;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
