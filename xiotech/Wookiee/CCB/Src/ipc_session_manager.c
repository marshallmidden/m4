/* $Id: ipc_session_manager.c 156532 2011-06-24 21:09:44Z m4 $ */
/*============================================================================
** FILE NAME:       ipc_session_manager.c
** MODULE TITLE:    ipc session manager
**
** DESCRIPTION:     The session manager is the central place for managing all
**                  all the sockets to a CCB and for handling all the packets
**                  that arive to the CCB.  It also does the unblocking and
**                  callbacks for the IpcSendPacket.
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include "debug_files.h"
#include "ipc_cmd_dispatcher.h"
#include "ipc_common.h"
#include "ipc_heartbeat.h"
#include "ipc_packets.h"
#include "ipc_sendpacket.h"
#include "ipc_server.h"
#include "ipc_session_manager.h"
#include "ipc_socket_io.h"
#include "timer.h"
#include "misc.h"
#include "EL.h"
#include "EL_Strings.h"
#include "slink.h"
#include "PortServerUtils.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "quorum_comm.h"
#include "fibreshim.h"
#include "nvram.h"
#include "trace.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "CT_history.h"

extern PCB        *K_xpcb;
/*****************************************************************************
** Private defines
*****************************************************************************/

/*
** State machine values
*/
#define     SM_SEND_REQ         0
#define     SM_TMO              1
#define     SM_CMD_IN_PROGRESS  2
#define     SM_RESPONSE_REQ     3

#define     setRetry(a)             ((a)->numRetries++)
#define     retrySet(a)             ((a)->numRetries)
#define     clearRetry(a)           ((a)->numRetries = 0)

/*
** Size of the history queue, the number of packets we check before
** we process a new packet.  This is required, because are packets are not all
** idempotent.
*/
#define MAX_HISTORY 100

/*
** If a user does not supply a return packet and supplied a callback
** then we will not wait nor look for a packet for a response
*/
#define NO_VERIFICATION_REQUESTED(a)    ( ((a)->rxPacket == NULL) && \
                                        ((a)->callBack == NULL))

#define HLList(a)      Sm.historyList.list[a]
#define HLHead()       Sm.historyList.head
#define HLTail()       Sm.historyList.tail
#define HLElements()   Sm.historyList.elements
#define HLIsEmpty()    (HLElements() == 0)
#define HLIsFull()     (HLElements() == MAX_HISTORY)
#define HLAddElement() ++HLElements()
#define HLRemElement() --HLElements()
#define HLIncHead()    HLHead() = ((HLHead() + 1) % MAX_HISTORY)
#define HLIncTail()    HLTail() = ((HLTail() + 1) % MAX_HISTORY)

typedef struct _IPC_HISTORY_LIST
{
    UINT8       head;
    UINT8       tail;
    UINT8       elements;
    IPC_PACKET *list[MAX_HISTORY];
} IPC_HISTORY_LIST;

/*
** Structure used to store all the information that relates to the
** session manager
*/
typedef struct _SESSION_MANAGER
{
    S_LIST     *sessionList;
    S_LIST     *outStanding;
    S_LIST     *inProgress;
    S_LIST     *packetQueue;
    S_LIST     *runningTasks;
    IPC_HISTORY_LIST historyList;
    PCB        *pcbID;
    bool        okToAwaken;
    bool        shutdown;

    IPC_PACKET *debugIpcPacket;
    SENDER_INFORMATION *debugSenderInfo;
    SESSION    *debugSession;
} SESSION_MANAGER_T;

MUTEX   sessionListMutex;
MUTEX   outStandingMutex;
MUTEX   inProgressMutex;
MUTEX   packetQueueMutex;
MUTEX   runningTasksMutex;
MUTEX   historyMutex;
MUTEX   sequenceNumberMutex;

/*****************************************************************************
** Private variables
*****************************************************************************/

/*
 * Instance of the session structure
 * make static after we get it fully debugged
 */
static SESSION_MANAGER_T Sm;
static UINT32 ipcSequenceNumber = 0;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/

/*
** Forward declaration, see implementation for documentation
*/
static void CheckForCommandTimeOuts(void);
static void StateMachine(SENDER_INFORMATION *si, IPC_PACKET *rx);
static void RegisterNewSocket(SESSION *session, SOCKET socket_number, UINT32 ipAddress);
static SOCKET MakeClientConnection(UINT32 ipAddress, UINT16 port);

static void DeleteInProgressInformation(SENDER_INFORMATION **p);
static void WakeTheSender(SENDER_INFORMATION *pSendInfo);
static void ExecuteCallback(SENDER_INFORMATION *si);
static void CompleteCommand(SENDER_INFORMATION *si);

static void NotifyTransportChange(SESSION *pSession, PATH_TYPE pt, bool working);

static SESSION *CreateSessionEntry(UINT32 serialNumber);
static void RemoveSession(SESSION *pSession);
static SESSION *SessionExist(UINT32 ipAddress);

static void dumpSI(SENDER_INFORMATION *p);
static UINT8 EnqueuePacket(IPC_PACKET *packet);

static void SendCommandInProgress(IPC_PACKET *packet, PATH_TYPE interface);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name:  void WakeSessionManager( void )
**
**  Description:    Wakes up the session manager
**
**  Inputs:         None
**
**  Returns:        None
**
**  Note:           If no one enqueus a packet we will not run again        .
**--------------------------------------------------------------------------*/
static void WakeSessionManager(void)
{
    if (Sm.okToAwaken && !Sm.shutdown)
    {
        TaskReadyState(Sm.pcbID);
    }
}

/*----------------------------------------------------------------------------
**  Function Name:  void TakeNap(void);
**
**  Description:    Puts the caller asleep until someone else wakes them up
**
**  Inputs:         None
**
**  Returns:        When someone wakes us up
**
**  Note:           If no one is scheduled to wake you up you will never
**                  execute again       .
**--------------------------------------------------------------------------*/
void TakeNap(void)
{
    PCB        *pcbID = XK_GetPcb();

    TaskSetState(pcbID, PCB_IPC_WAIT);
    TaskSwitch();
}

/*----------------------------------------------------------------------------
**  Function Name:  void IpcShutDown(void);
**
**  Description:    Shuts down the IPC layer
**
**  Inputs:         None
**
**  Returns:        None
**
**  Note:           Once you call this you need to restart the controller
**                  to get IPC to work again
**--------------------------------------------------------------------------*/
void IpcShutDown(void)
{
    static bool shutDownPending = FALSE;

    /*
     * Theory of operation:  If session manager is already asleep, because
     * he put himself to sleep then make sure he will never get woken again.
     * If he is in the ready state put him to sleep.  If the session manager
     * is not ready to run and he didn't put himself to sleep then we need
     * to wait until he becomes ready to run,
     */

    /*
     * If we are already shut down great, otherwise we make sure no one else
     * is in the middle of a shutdown
     */
    if (FALSE == Sm.shutdown && FALSE == shutDownPending)
    {
        /* Do this to make sure we are thread safe on the CCB */
        shutDownPending = TRUE;

        if (Sm.okToAwaken == FALSE)
        {
            while (TaskGetState(Sm.pcbID) != PCB_READY)
            {
                // TaskSwitch();
                TaskSleepMS(20);
            }

            /* Set the process to a not ready state */
            TaskSetState(Sm.pcbID, PCB_NOT_READY);
        }

        /* Let everyone else know that the ipc session manager is down */
        Sm.shutdown = TRUE;
        shutDownPending = FALSE;
    }
}

/*----------------------------------------------------------------------------
**  Function Name:  bool IpcShutDown(void);
**
**  Description:    Returns true if someone has asked to shut us down
**
**  Inputs:         None
**
**  Returns:        bool
**
**--------------------------------------------------------------------------*/
bool IpcShutDownRequested(void)
{
    return Sm.shutdown;
}


/*----------------------------------------------------------------------------
**  Function Name:  bool BringUpInterface( SESSION *session, PATH_TYPE path);
**
**  Description:    Used to re-establish a connection with a given ccb
**
**  Inputs: SESSION        *session         Pointer to the session we
**                                          are using
**          PATH_TYPE       path            Which path we want to bring up
**
**  Returns:        TRUE if the socket for this interface has been
**                  re-established, else FALSE.  If you try to bring up
**                  an interface that is already good then you will get a
**                  return of FALSE
**
**  Note:           This function should only be used by health monitoring?
**--------------------------------------------------------------------------*/
bool BringUpInterface(SESSION *session, PATH_TYPE path)
{
    UINT32      ipAddress;
    SOCKET      tmpSocket;

    if (session && (path == SENDPACKET_ETHERNET || path == SENDPACKET_FIBRE))
    {
        if (path == SENDPACKET_ETHERNET)
        {
            if (session->eth == SOCKET_ERROR)
            {
                ipAddress = SerialNumberToIPAddress(session->sn, SENDPACKET_ETHERNET);

                if (ipAddress)
                {
                    tmpSocket = MakeClientConnection(ipAddress, IPC_PORT_ETH);

                    if (tmpSocket >= 0)
                    {
                        RegisterNewSocket(session, tmpSocket, ipAddress);
                        return TRUE;
                    }
                    LogMessage(LOG_TYPE_DEBUG, "BringUpInterface: Failed to make client connection (0x%x).",
                               tmpSocket);
                    return FALSE;
                }
                LogMessage(LOG_TYPE_DEBUG, "BringUpInterface: Invalid IP address.");
                return FALSE;
            }
            dprintf(DPRINTF_DEFAULT, "BringUpInterface: Ethernet already up to %u, socket %d\n",
                    session->sn, session->eth);
            return FALSE;
        }

        if (path == SENDPACKET_FIBRE && (!FC_GOOD(session)))
        {
            if (!FC_GOOD(session))
            {
                /* Check to see if we have a valid fibre path */
                if (CheckFibrePath(session->sn) == GOOD)
                {
                    session->fcDirect = TRUE;
                    NotifyTransportChange(session, SENDPACKET_FIBRE, TRUE);
                    return TRUE;
                }
                return FALSE;
            }
            dprintf(DPRINTF_DEFAULT,
                    "BringUpInterface: Fibre already up to %u.\n", session->sn);
        }
    }
    return FALSE;
}

/*----------------------------------------------------------------------------
**  Function Name:  bool CheckInterfaceStatus( PATH_TYPE path );
**
**  Description:    Checks to see if all the sessions of a given path type
**                  are functional, or working best we know
**
**  Inputs: PATH_TYPE       path Which path we want to bring up
**
**  Returns:        TRUE If all the sessions have a working path as defined
**                  by the passed parameter, otherwise returns FALSE
**
**  Note:           Not sure if we should provide such info.
**--------------------------------------------------------------------------*/
bool CheckInterfaceStatuses()
{
    SESSION    *p;

    ccb_assert(Sm.sessionList != NULL, Sm.sessionList);

    /* Make sure we have a valid list pointer */
    if (!Sm.sessionList)
    {
        return FALSE;
    }

    /* Walk the list looking for sessions with a bad communication path */
    (void)LockMutex(&sessionListMutex, MUTEX_WAIT);
    SetIterator(Sm.sessionList);

    while (NULL != (p = (SESSION *)Iterate(Sm.sessionList)))
    {
        if ((!ETH_GOOD(p)) || (!FC_GOOD(p)))
        {
            UnlockMutex(&sessionListMutex);
            return FALSE;
        }
    }
    UnlockMutex(&sessionListMutex);
    return TRUE;
}

/*----------------------------------------------------------------------------
**  Function Name:  static int PacketHistoryCmp( void const *a, void const *b )
**
**  Description:    Used ot compare our current packet to each item in the
**                  history list.
**
**  Inputs:         void const *a   pointer to a packet
**                  void const *b   pointer to a packet
**
**  Returns:        zero if they match, else returns 1
**--------------------------------------------------------------------------*/
static int PacketHistoryCmp(void const *a, void const *b)
{
    if ((((IPC_PACKET *)a)->header->ccbSerialNumber == ((IPC_PACKET *)b)->header->ccbSerialNumber) &&
        (((IPC_PACKET *)a)->header->commandCode == ((IPC_PACKET *)b)->header->commandCode) &&
        (((IPC_PACKET *)a)->header->sequenceNumber == ((IPC_PACKET *)b)->header->sequenceNumber))
    {
        return 0;
    }
    return 1;
}

/*----------------------------------------------------------------------------
**  Function Name:  bool PacketHistoryListAdd( IPC_PACKET *packet )
**
**  Description:    Adds a packet to the history list
**
**  Inputs:         IPC_PACKET *packet pointer to the packet to add
**
**  Returns:        TRUE if successful, else returns a FALSE
**--------------------------------------------------------------------------*/
static bool PacketHistoryListAdd(IPC_PACKET *packet)
{
    IPC_PACKET *packetToDel;

    if (!packet)
    {
        return FALSE;
    }

    (void)LockMutex(&historyMutex, MUTEX_WAIT);

    if (HLIsFull())
    {
        packetToDel = HLList(HLHead());
        FreePacket(&packetToDel, __FILE__, __LINE__);

        HLIncHead();
        HLRemElement();
    }

    if (!HLIsEmpty())
    {
        HLIncTail();
    }

    HLList(HLTail()) = packet;
    HLAddElement();

    UnlockMutex(&historyMutex);

    return TRUE;
}

/*----------------------------------------------------------------------------
**  Function Name:  bool RunningTasksListAdd( IPC_PACKET *packet )
**
**  Description:    Adds a packet to the running tasks list
**
**  Inputs:         IPC_PACKET *packet pointer to the packet to add
**
**  Returns:        TRUE if successful, else returns a FALSE
**--------------------------------------------------------------------------*/
static bool RunningTasksListAdd(IPC_PACKET *packet)
{
    ccb_assert(Sm.runningTasks != NULL, Sm.runningTasks);

    if (Sm.runningTasks && packet)
    {
        (void)LockMutex(&runningTasksMutex, MUTEX_WAIT);
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s RunningTasksListAdd of %p\n", __FILE__, __LINE__, __func__, packet);
#endif  /* HISTORY_KEEP */
        Enqueue(Sm.runningTasks, packet);
        UnlockMutex(&runningTasksMutex);
        return TRUE;
    }
    return FALSE;
}

/*----------------------------------------------------------------------------
**  Function Name:  bool RunningTasksListRemove( IPC_PACKET *packet )
**
**  Description:    Removes a packet from the running tasks list
**
**  Inputs:         IPC_PACKET *packet pointer to the packet to remove
**
**  Returns:        Returns numm if packet not founf else returns Pointer
**                  to packet
**--------------------------------------------------------------------------*/
static IPC_PACKET *RunningTasksListRemove(IPC_PACKET *packet)
{
    IPC_PACKET *packetToRemove = NULL;

    ccb_assert(Sm.runningTasks != NULL, Sm.runningTasks);

    if (Sm.runningTasks && packet)
    {
        (void)LockMutex(&runningTasksMutex, MUTEX_WAIT);

        packetToRemove = RemoveElement(Sm.runningTasks, packet, PacketHistoryCmp);

        UnlockMutex(&runningTasksMutex);
    }

    return packetToRemove;
}

/*----------------------------------------------------------------------------
**  Function Name:  IPC_PACKET *CheckForPreviousPacket( IPC_PACKET *packet )
**
**  Description:    Checks to see if we have seen this packet before
**
**  Inputs:         IPC_PACKET *packet pointer to the packet to check for
**
**  Returns:        Returns NULL if packet not being processed, else returns
**                  Pointer to the packet
**--------------------------------------------------------------------------*/
static IPC_PACKET *CheckForPreviousPacket(IPC_PACKET *packet)
{
    IPC_PACKET *rc = NULL;
    UINT16      index1;

    if (packet && !HLIsEmpty())
    {
        (void)LockMutex(&historyMutex, MUTEX_WAIT);

        for (index1 = HLHead(); index1 < (HLHead() + HLElements()); ++index1)
        {
            if (PacketHistoryCmp(packet, HLList(index1 % MAX_HISTORY)) == GOOD)
            {
                rc = HLList(index1 % MAX_HISTORY);
                break;
            }
        }

        UnlockMutex(&historyMutex);
    }

    return rc;
}

/*----------------------------------------------------------------------------
**  Function Name:  IPC_PACKET *CheckForRunningTask( IPC_PACKET *packet )
**
**  Description:    Checks to see if we have this packet running
**
**  Inputs:         IPC_PACKET *packet pointer to the packet to check for
**
**  Returns:        Returns NULL if packet not processed before, else returns
**                  Pointer to the packet
**--------------------------------------------------------------------------*/
static IPC_PACKET *CheckForRunningTask(IPC_PACKET *packet)
{
    IPC_PACKET *rc = NULL;

    ccb_assert(Sm.runningTasks != NULL, Sm.runningTasks);

    if (Sm.runningTasks && packet)
    {
        (void)LockMutex(&runningTasksMutex, MUTEX_WAIT);
        rc = FindElement(Sm.runningTasks, packet, PacketHistoryCmp);
        UnlockMutex(&runningTasksMutex);
    }

    return rc;
}

/*----------------------------------------------------------------------------
**  Function Name:  void ReIssueWaiting( UINT32 sn, PATH_TYPE tp)
**
**  Description:    And commands that are outstanding on for this ccb
**                  serial number and this interface need to be resent.
**
**  Inputs:         UINT32  sn              serial number of ccb to reissue
**                  PATH_TYPE tp            which interface to re-issue
**                                          packets for
**
**  Returns:        void
**--------------------------------------------------------------------------*/
static void ReIssueWaiting(UINT32 sn, PATH_TYPE tp)
{
    SENDER_INFORMATION *p;

    ccb_assert(Sm.inProgress != NULL, Sm.inProgress);

    /* Make sure we have a valid list pointer */
    if (!Sm.inProgress)
    {
        return;
    }

    (void)LockMutex(&inProgressMutex, MUTEX_WAIT);

    /* Walk the list looking for timeouts */
    SetIterator(Sm.inProgress);

    while (NULL != (p = (SENDER_INFORMATION *)Iterate(Sm.inProgress)))
    {
        if (p->session->sn == sn &&
            /* Physical transport enum type is one less than paths */
            p->actualPath == tp &&
            (p->stateMachine == SM_CMD_IN_PROGRESS ||
             p->stateMachine == SM_RESPONSE_REQ))
        {
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s set stateMachine of %p to SM_SEND_REQ\n", __FILE__, __LINE__, __func__, p);
#endif  /* HISTORY_KEEP */
            p->stateMachine = SM_SEND_REQ;
            StateMachine(p, NULL);
        }
    }

    UnlockMutex(&inProgressMutex);
}

/*----------------------------------------------------------------------------
**  Function Name:  static int CompareSessionSerialNumbers( void const *a,
**                                                          void const *b )
**
**  Description:    Compares two session serial numbers
**
**  Inputs:         void const *a   Pointer to the first element to compare
**                  void const *b   Ponter to the second element to compare
**
**  Returns:        -1 if a < b
**                   1 if a > b
**                   0 if a == b
**--------------------------------------------------------------------------*/
static int CompareSessionSerialNumbers(void const *a, void const *b)
{
    /*
     * Compare the serial numbers, the rc above set to zero assumes
     * that they are equal until proven otherwise.
     */
    if (((SESSION *)a)->sn < *((UINT32 *)b))
    {
        return -1;
    }
    else if (((SESSION *)a)->sn > *((UINT32 *)b))
    {
        return 1;
    }
    return 0;
}


/**
******************************************************************************
**
**  @brief          Compares two session pointers.
**
**  @param          void const *a   Pointer to the first element to compare
**  @param          void const *b   Ponter to the second element to compare
**
**  @return         -1 if a < b
**  @return          1 if a > b
**  @return          0 if a == b
**
******************************************************************************
**/
static int CompareClientRequestSession(void const *a, void const *b)
{
    if (((SENDER_INFORMATION *)a)->session < (SESSION *)b)
    {
        return -1;
    }
    else if (((SENDER_INFORMATION *)a)->session > (SESSION *)b)
    {
        return 1;
    }
    return 0;
}


/*----------------------------------------------------------------------------
**  Function Name:  void CompleteCommand( void );
**
**  Description:    Completes a command that is finished
**
**  Inputs:         SENDER_INFORMATION *si
**
**  Returns:        None
**
**  Side effects:   si will be freed and set to NULL
**
**  ATTENTION:      Needs inProgressMutex set before call (for DeleteInProgressInformation)
**
**--------------------------------------------------------------------------*/
static void CompleteCommand(SENDER_INFORMATION *si)
{
    if (si)
    {
        /* Wake the caller if needed */
        WakeTheSender(si);

        /* Call the callback if needed */
        ExecuteCallback(si);

        /* Free the memory */
        DeleteInProgressInformation(&si);
    }
}


/*----------------------------------------------------------------------------
**  Function Name:  static int CompareSeqId( void const *a, void const *b )
**
**  Description:    Compares two sequence id's
**
**  Inputs:         void const *a   Pointer to the first element to compare
**                  void const *b   Ponter to the second element to compare
**
**  Returns:        -1 if a < b
**                   1 if a > b
**                   0 if a == b
**--------------------------------------------------------------------------*/
static int CompareSeqId(void const *a, void const *b)
{
    /*
     * Compare the sequence IDs, the rc above set to zero assumes
     * that they are equal until proven otherwise.
     */
    if (((SENDER_INFORMATION *)a)->seqNum < *((UINT16 *)b))
    {
        return -1;
    }
    else if (((SENDER_INFORMATION *)a)->seqNum > *((UINT16 *)b))
    {
        return 1;
    }
    return 0;
}

/*----------------------------------------------------------------------------
**  Function Name:  void WakeTheSender(SENDER_INFORMATION *pSendInfo)
**
**  Description:    Informs the code that made the send IpcSendPacket call
**                  wakes them up
**
**  Inputs:         SENDER_INFORMATION *pSendInfo   Pointer to the sender info
**
**  Returns:        None
**--------------------------------------------------------------------------*/
static void WakeTheSender(SENDER_INFORMATION *pSendInfo)
{
    if ((pSendInfo) && (FALSE == pSendInfo->haveWokenSender))
    {
        if (SENDPACKET_ANY_PATH == pSendInfo->actualPath)
        {
            dprintf(DPRINTF_IPC_SESSION_MANAGER, "IPC logic error, we are returning SENDPACKET_ANY_PATH");

            dumpSI(pSendInfo);
        }

        /* We only want to wake the sender once!  */
        pSendInfo->haveWokenSender = TRUE;

        /*
         * Check to see if we need to unblock the caller
         * we will only do this if we have a pcb id and we are not doing
         * callbacks
         */
        if (pSendInfo->pcbID && (pSendInfo->callBack == NULL))
        {
            /*
             * Update the requested senders path, do this before we wake the
             * client!!!!!
             */
            *(pSendInfo->ipc_send_rc) = pSendInfo->actualPath;

            /*
             * The process asked us to wake them up, if they are not
             * asleep yet then we have a very bad logic error.
             */
            if (TaskGetState(pSendInfo->pcbID) == PCB_IPC_WAIT)
            {
                TaskSetState(pSendInfo->pcbID, PCB_READY);
            }
            else if (TaskGetState(pSendInfo->pcbID) != PCB_READY)
            {
                TaskSetState(pSendInfo->pcbID, PCB_READY);
                dprintf(DPRINTF_DEFAULT, "%s:%u:%s (we are %p:%p) The sender process %p that we were to wake up was had status (%u)",
                        __FILE__, __LINE__, __func__,
                        K_xpcb, print_functionPtr(K_xpcb), pSendInfo->pcbID, pSendInfo->pcbID->pc_stat);
            }
            else
            {
                dprintf(DPRINTF_DEFAULT, "%s:%u:%s (we are %p:%p) The sender process %p that we were to wake up was not asleep (%u)",
                        __FILE__, __LINE__, __func__,
                        K_xpcb, print_functionPtr(K_xpcb), pSendInfo->pcbID, pSendInfo->pcbID->pc_stat);
            }
        }
    }
}

/*----------------------------------------------------------------------------
**  Function Name:  void executeCallback( SENDER_INFORMATION *si )
**
**  Description:    Calls the callback for async calls
**
**  Inputs:         SENDER_INFORMATION *pSendInfo   Pointer to the sender info
**
**  Returns:        None
**--------------------------------------------------------------------------*/
static void ExecuteCallback(SENDER_INFORMATION *si)
{
    TASK_PARMS  parms;

    if (si && si->callBack)
    {
        if (si->callBackResult)
        {
            si->callBackResult->result = si->actualPath;
        }

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s si->txPacket of %p\n", __FILE__, __LINE__, __func__, si->txPacket);
#endif  /* HISTORY_KEEP */

        TraceEvent(TRACE_IPC_CALLBACK, si->txPacket->header->commandCode);

        parms.p1 = (UINT32)si->callBackParams;
        TaskCreate((void *)si->callBack, &parms);
    }
}

/*----------------------------------------------------------------------------
**  Function Name:  void GetSendLock( SESSION *session)
**
**  Description:    Mutex for sending data
**
**  Inputs:         None
**
**  Returns:        None
**--------------------------------------------------------------------------*/
void GetSendLock(SESSION *session)
{
    if (session)
    {
        while (session->sendLock != FALSE)
        {
            // TaskSwitch();
            TaskSleepMS(20);
        }
        session->sendLock = TRUE;
    }
}

/*----------------------------------------------------------------------------
**  Function Name:  void FreeSendLock( SESSION *session)
**
**  Description:    Free mutex for sending data
**
**  Inputs:         None
**
**  Returns:        None
**--------------------------------------------------------------------------*/
void FreeSendLock(SESSION *session)
{
    if (session)
    {
        session->sendLock = FALSE;
    }
}

/*----------------------------------------------------------------------------
**  Function Name:  SOCKET MakeClientConnection( UINT32 ipAddress,
**                                                      UINT16 port)
**
**  Description:    Makes a client connection to a given ip address and port
**
**  Inputs:         UINT32  ipAddress       IP address to connect to
**                  UNIT16  port            Port to connect to
**
**  Returns:        SOCKET      Valid socket if successful, else -1
**--------------------------------------------------------------------------*/
static SOCKET MakeClientConnection(UINT32 ipAddress, UINT16 port)
{
    SOCKET      tmpSocket;
    struct sockaddr_in destAddr;
    struct sockaddr_in localAddr;
    int         errorCode;
    PATH_TYPE   pt;
    UINT32      localIp;

    tmpSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (tmpSocket == -1)
    {
        LogMessage(LOG_TYPE_DEBUG, "MakeClientConnection: Failed to create socket.");
        return -1;
    }

    if (FALSE == SetIpcSocketOptions(tmpSocket))
    {
        errorCode = errno;
        LogMessage(LOG_TYPE_DEBUG, "MakeClientConnection: Failed to set socket options, errno %d/%s.",
                   errorCode, GetErrnoString(errorCode));
        CloseSocket(&tmpSocket);
        return -1;
    }

    /* Set tcp keepalive options. */
    setkeepalive(tmpSocket);

    /*
     * Because we have two possible interface cards we need to bind
     * the socket to one of them
     *
     * Need to figure out which type of interface the server IP is
     * and then figure out which interface on the client matches that type
     */
    pt = GetTransportType(ipAddress);
    localIp = SerialNumberToIPAddress(GetMyControllerSN(), pt);

    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = 0;
    localAddr.sin_addr.s_addr = localIp;

    errorCode = bind(tmpSocket, (struct sockaddr *)&localAddr, sizeof(localAddr));

    if (errorCode == -1)
    {
        errorCode = errno;
        LogMessage(LOG_TYPE_DEBUG, "MakeClientConnection: Failed to bind socket, errno %d/%s.",
                   errorCode, GetErrnoString(errorCode));
        CloseSocket(&tmpSocket);
        return -1;
    }

    /* Setup all the items in sockaddr_in */
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(port);
    destAddr.sin_addr.s_addr = ipAddress;

    errorCode = NonBlockingConnect(tmpSocket, (struct sockaddr *)&destAddr,
                                   sizeof(destAddr), CONNECT_TMO);
    if (errorCode < 0)
    {
        errorCode = errno;
        LogMessage(LOG_TYPE_DEBUG, "MakeClientConnection: Failed NonBlockingConnect, errno %d/%s.",
                   errorCode, GetErrnoString(errorCode));
        /* On error, the socket gets closed in NonBlockingConnect() */
        return -1;
    }

    /* Set tcp keepalive options (again, just in case). */
    setkeepalive(tmpSocket);

    return tmpSocket;
}

/*----------------------------------------------------------------------------
**  Function Name:  static bool ConnectToIPCServer(SESSION *session,
**                                                 UINT32 serialNumber)
**
**  Description:    Establishes a connection to an IPC port server and
**                  forks a ipc_server if needed.
**
**  Inputs:         SESSION    *session         Session pointer
**                  UINT32      serialNumber    Serial number
**
**  Returns:        bool    TRUE    - we connected OK
**                          FALSE   - unable to connect
**--------------------------------------------------------------------------*/
static bool ConnectToIPCServer(SESSION *session, UINT32 serialNumber)
{
    SOCKET      tmpSocket;
    UINT32      ipAddress;
    TASK_PARMS  parms;

    if (session)
    {
        bool        rc = FALSE;

        /* Create a client socket over eth and fibre channel. */
        ipAddress = SerialNumberToIPAddress(serialNumber, SENDPACKET_ETHERNET);

        if (ipAddress)
        {
            tmpSocket = MakeClientConnection(ipAddress, IPC_PORT_ETH);

            if (tmpSocket >= 0)
            {
                RegisterNewSocket(session, tmpSocket, ipAddress);

                /* We have a session, go ahead and create a server. */
                rc = TRUE;
            }
            else
            {
                LogMessage(LOG_TYPE_DEBUG, "ConnectToIPCServer: Failed to make client connection (0x%x).",
                           tmpSocket);
            }
        }
        else
        {
            LogMessage(LOG_TYPE_DEBUG, "ConnectToIPCServer: Invalid IP address.");
        }

        if (GOOD == CheckFibrePath(serialNumber))
        {
            /* If we are willing to give the user a session pointer, we need
             * to have the reader task running. */
            session->fcDirect = TRUE;
            NotifyTransportChange(session, SENDPACKET_FIBRE, TRUE);

            /* We have a session, go ahead and create a server. */
            rc = TRUE;
        }

        /* If we have a socket, then we need a thread to read data from it. */
        if (rc == TRUE)
        {
            parms.p1 = (UINT32)session;
            TaskCreate(IpcServer, &parms);
            return TRUE;
        }
    }
    else
    {
        LogMessage(LOG_TYPE_DEBUG, "ConnectToIPCServer: Invalid session.");
    }
    return FALSE;
}

/*----------------------------------------------------------------------------
**  Function Name:  SESSION *CreateSessionEntry(UINT32 serialNumber)
**
**  Description:    Creates a new session entry for the session manager
**
**  Inputs:         UINT32  ccb serial number
**
**  Returns:        Pointer to the new session that was created, NULL
**                  if we encountered errors
**
**  ATTENTION:      Requires sessionListMutex locked before calling.
**
**--------------------------------------------------------------------------*/
static SESSION *CreateSessionEntry(UINT32 serialNumber)
{
    SESSION    *session;

    session = MallocWC(sizeof(*session));

    /* Initialize session structure */
    session->sn = serialNumber;
    session->eth = SOCKET_ERROR;
    session->fcDirect = TRUE;   /* Good until proven otherwise */

    session->sendLock = FALSE;

    AddElement(Sm.sessionList, session);

    return session;
}

/*----------------------------------------------------------------------------
**  Function Name:  SESSION *GetSession(UINT32 serialNumber)
**
**  Description:    Gets a session handle that cooresponds to the serial
**                  number
**
**  Inputs:         UINT32  ccb serial number
**
**  Returns:        Pointer to the session that existed or that was newly
**                  created or NULL if we encounter some errors
**
**  Note:           At this time it is invalid to send a packet to yourself
**
**--------------------------------------------------------------------------*/
SESSION *GetSession(UINT32 serialNumber)
{
    SESSION    *session = NULL;

    if (serialNumber != GetMyControllerSN())
    {
        (void)LockMutex(&sessionListMutex, MUTEX_WAIT);

        session = FindElement(Sm.sessionList, &serialNumber, CompareSessionSerialNumbers);

        if (!session)
        {
            session = CreateSessionEntry(serialNumber);

            if (session)
            {
                if (!ConnectToIPCServer(session, serialNumber))
                {
                    LogMessage(LOG_TYPE_DEBUG, "Unable to connect to CCB (%u)",
                               serialNumber);

                    /*
                     * Because we were unable to connect to a ccb with this
                     * serial number we need to remove this session
                     */
                    RemoveSession(session);
                    session = NULL;
                }
                else
                {
                    LogMessage(LOG_TYPE_DEBUG, "GetSession: Connected to IPC server (0x%x).",
                               serialNumber);
                }
            }
            else
            {
                LogMessage(LOG_TYPE_DEBUG, "GetSession: Invalid session entry.");
            }
        }
        UnlockMutex(&sessionListMutex);
    }
    return session;
}

/*----------------------------------------------------------------------------
**  Function Name:  void RegisterNewSocket( SESSION *session,
**                                          SOCKET socket,
**                                          UINT32 ipAddress)
**
**  Description:    Registers a new socket for a given session
**
**  Inputs:         SESSION *session    session you want socket registered to
**                  SOCKET socket       new socket to register
**                  UINT32              ipAddress of client
**
**  Returns:        None
**--------------------------------------------------------------------------*/
static void RegisterNewSocket(SESSION *session, SOCKET socket_number, UINT32 ipAddress)
{
    UINT32      SN;
    PATH_TYPE   tp = GetTransportType(ipAddress);

    SN = IPAddressToSerialNumber(ipAddress);

    if (!session)
    {
        LogMessage(LOG_TYPE_DEBUG, "RegisterNewSocket: Invalid session.");
        return;
    }

    if (tp != SENDPACKET_ETHERNET)
    {
        LogMessage(LOG_TYPE_DEBUG, "RegisterNewSocket: Not ethernet transport (0x%x).", tp);
        return;
    }

    if (session->eth != -1)
    {
        if (GetMyControllerSN() > SN)
        {
            dprintf(DPRINTF_DEFAULT, "RegisterNewSocket: The logic is requesting that new socket %d be closed, but it IS NOT BEING CLOSED\n",
                    socket_number);
        }

        CloseSocket(&session->eth);
        session->eth = socket_number;

        /*
         * At this point we need to re-issue any packets that
         * we are waiting on for on this interface
         */
        ReIssueWaiting(SN, tp);
    }
    else
    {
        session->eth = socket_number;
        NotifyTransportChange(session, SENDPACKET_ETHERNET, TRUE);
    }
    LogMessage(LOG_TYPE_DEBUG, "Opening IPC socket %d to %u", socket_number, SN);
}

/*----------------------------------------------------------------------------
**  Function Name:  QueueSenderInformation
**
**  Description:    Queues senders request and information to session manager
**
**  Inputs: session        - Pointer to the session we are using
**          requestedPath  - Pointer to the requested path
**          txPacket       - Packet we want to send
**          rxPacket       - Pointer to the result packet
**          callBack       - Pointer to the users callback function
**          callBackParams - Pointer to the list of parameters for callback
**          tmo            - Timeout in milliseconds
**          pcb            - Pointer to the pcb of caller
**
**  Returns:        True if we completed without errors else returns False
**
**  Note:           This function should only be used by Ipc internal functions
**
**--------------------------------------------------------------------------*/
bool QueueSenderInformation(SESSION *session, PATH_TYPE *requestedPath,
                            IPC_PACKET *txPacket, IPC_PACKET *rxPacket,
                            void (*callBack)(void *parms), void *callBackParams,
                            IPC_CALL_BACK_RESULTS *asyncResult, UINT32 tmo, PCB *pcb)
{
    SENDER_INFORMATION *pSenderInfo;
    IPC_PACKET *myTransmitPacket = txPacket;

    /* Make sure we have enough to send */
    if (!session || !txPacket || ! txPacket->header)
    {
        return FALSE;
    }

    /* Allocate storage for sender information */
    pSenderInfo = MallocSharedWC(sizeof(*pSenderInfo));

    /* Make a copy of the transmit packet if this is a callback packet */
    if (callBack)
    {
        /*
         * Copy the packet to be sent.  This way, the sender can free it's
         * txPacket as soon as IpcSendPacket returns.  In the case where IPC
         * needs to retransmit the packet, it still has it's own copy.
         */
        myTransmitPacket = CopyPacket(txPacket);
    }

    /* Copy all the parameters */
    pSenderInfo->session = session;

    /*
     * Smashing the stack for a very difficult bug to find!
     * We need to copy the requested path because if we wake up
     * the caller and use the rc value on the stack when it no longer exists
     * things can get very ugly.  We still store the address of the
     * return variable on the stack, but will only use when we are doing
     * a blocking send, then the address is valid.
     */
    pSenderInfo->requestedPath = *requestedPath;
    pSenderInfo->ipc_send_rc = requestedPath;

    pSenderInfo->txPacket = myTransmitPacket;
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s pSenderInfo->txPacket of %p\n", __FILE__, __LINE__, __func__, pSenderInfo->txPacket);
#endif  /* HISTORY_KEEP */
    pSenderInfo->rxPacket = rxPacket;
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s set rxPacket of %p to %p\n", __FILE__, __LINE__, __func__, pSenderInfo, rxPacket);
#endif  /* HISTORY_KEEP */
    pSenderInfo->callBack = callBack;
    pSenderInfo->callBackParams = callBackParams;
    pSenderInfo->callBackResult = asyncResult;
    pSenderInfo->seqNum = GetPacketSeqNum(txPacket->header);

    /* Round and convert from microseconds into clock ticks (1/8th of a second) */
    pSenderInfo->timeout = (tmo + IPC_CLOCK_RES - 1) / IPC_CLOCK_RES;
    pSenderInfo->pcbID = pcb;

    /* Adding the sender info to the outstanding queue */
    (void)LockMutex(&outStandingMutex, MUTEX_WAIT);

    Enqueue(Sm.outStanding, pSenderInfo);

    /* Wake the session manager if he needs to be woken */
    WakeSessionManager();

    UnlockMutex(&outStandingMutex);

    return TRUE;
}

/*----------------------------------------------------------------------------
**  Function Name:  void DeleteInProgressInformation( SENDER_INFORMATION **p )
**
**  Description:    Removes an item from the in progress list
**
**  Inputs:         Sender info pointer
**
**  Returns:        none
**
**  Note:           This function is only to be used by Ipc internal functions
**
**  ATTENTION:      Needs inProgressMutex set before call (for DeleteInProgressInformation)
**
**--------------------------------------------------------------------------*/
static void DeleteInProgressInformation(SENDER_INFORMATION **p)
{
    ccb_assert(Sm.inProgress != NULL, Sm.inProgress);

    *p = RemoveElement(Sm.inProgress, &(*p)->seqNum, CompareSeqId);

    if (*p)
    {
        /*
         * If this packet was a callback, we need to free the transmit packet
         * memory.  For callbacks, we make a copy of the transmit packet so that
         * the sender of the packet can free the transmit packet immediately
         * after the IpcSendPacket call returns.  This way, sending a callback
         * packet can be treated like a non-callback packet.
         */
        if ((*p)->callBack)
        {
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s Calling FreePacket &((*p)->txPacket) of %p\n", __FILE__, __LINE__, __func__, (*p)->txPacket);
#endif  /* HISTORY_KEEP */
            FreePacket(&((*p)->txPacket), __FILE__, __LINE__);
        }

        Free(*p);
    }
}

/*----------------------------------------------------------------------------
**  Function Name:  UINT32 NextSequenceNumber(void);
**
**  Description:    Returns a unique sequence number for the session manager
**
**  Inputs:         None
**
**  Returns:        sequence number
**
**  Note:           This function should only be used by Ipc internal
**                  functions
**--------------------------------------------------------------------------*/
UINT32 NextSequenceNumber(void)
{
    UINT32      nextSeqNum;

    (void)LockMutex(&sequenceNumberMutex, MUTEX_WAIT);

    nextSeqNum = ++ipcSequenceNumber;

    UnlockMutex(&sequenceNumberMutex);

    return nextSeqNum;
}


/*----------------------------------------------------------------------------
**  Function Name:  SESSION *SessionExist(UINT32 ipAddress)
**
**  Description:    Checks to see if we have a session already has this
**                  ipAddress
**
**  Inputs:         UINT32 ipAddress        Ip address to check for
**
**  Returns:        Pointer to session if it exists, otherwise returns NULL
**
**  Note:           This function should only be used by Ipc internal
**                  functions
**
** ATTENTION:       sessionListMutex must be locked before calling.
**
**--------------------------------------------------------------------------*/
static SESSION *SessionExist(UINT32 ipAddress)
{
    UINT32      sn = IPAddressToSerialNumber(ipAddress);
    SESSION    *rc;

    ccb_assert(Sm.sessionList != NULL, Sm.sessionList);

    rc = FindElement(Sm.sessionList, &sn, CompareSessionSerialNumbers);

    return rc;
}

/*----------------------------------------------------------------------------
**  Function Name:  SOCKET *GetEthernetSocket( SESSION *session);
**
**  Description:    Returns a pointer to the ethernet socket tie'd to a
**                  session
**
**  Inputs: SESSION        *session         Pointer to the session we
**                                          are using
**
**  Returns:        Address of the socket used for ethernet, may return NULL
**
**  Note:           This function should only be used by Ipc internal
**                  functions
**
**--------------------------------------------------------------------------*/
SOCKET     *GetEthernetSocket(SESSION *session)
{
    return ((session != NULL) ? &session->eth : NULL);
}

/*----------------------------------------------------------------------------
**  Function Name:  UINT8 EnqueuePacket( IPC_PACKET *packet);
**
**  Description:    Enqueues a data packet to the session manager
**
**  Inputs:         IPC_PACKET *packet      Packet to enqueue
**
**  Returns:        1 if completed without errors, else zero
**
**  Note:           This function should only be used by Ipc internal
**                  functions
**
**--------------------------------------------------------------------------*/
static UINT8 EnqueuePacket(IPC_PACKET *packet)
{
    if (!packet)
    {
    return FALSE;
    }

    (void)LockMutex(&packetQueueMutex, MUTEX_WAIT);

    /* Enqueue the packet */
    Enqueue(Sm.packetQueue, packet);

    /* Alert the session manager */
    WakeSessionManager();

    UnlockMutex(&packetQueueMutex);
    return TRUE;
}

/*----------------------------------------------------------------------------
**  Function Name:  bool ClientConnect(SOCKET socket_number, UINT32 ipAddress);
**
**  Description:    Takes a new client connection and ipaddress and checks
**                  to make sure we allow this client to connect to the ipc
**                  server.
**
**  Inputs:         SOCKET socket_number       socket that new client connected on
**                  UINT32  ipAddress   IP address that client coming from
**
**  Returns:        True or False
**
**  Note:           This function should only be used by Ipc internal
**                  functions
**
** ATTENTION:       sessionListMutex must be locked before calling
**                  (SessionExist, CreateSessionEntry).
**
**--------------------------------------------------------------------------*/
bool ClientConnect(SOCKET socket_number, UINT32 ipAddress)
{
    SESSION    *session;
    UINT32      sn;
    TASK_PARMS  parms;

    /* Check to see if we have a session */
    session = SessionExist(ipAddress);

    if (session)
    {
        /* We already have a session for this so register this socket */
        RegisterNewSocket(session, socket_number, ipAddress);
        return TRUE;
    }

    /* Convert form IP address to serial number */
    sn = IPAddressToSerialNumber(ipAddress);

    if (sn != 0)
    {
        /* Add a session record */
        session = CreateSessionEntry(sn);

        if (session)
        {
            RegisterNewSocket(session, socket_number, ipAddress);

            /* Fork a new receiver */
            parms.p1 = (UINT32)session;
            TaskCreate(IpcServer, &parms);
            return TRUE;
        }
    }
    else
    {
        /*
         * Log some event here like someone other than a known CCB
         * Tried to connect
         */
    }

    return FALSE;
}

/*----------------------------------------------------------------------------
**  Function Name:  static void HandleResponse(IPC_PACKET *pPacket)
**
**  Description:    A new packet was read and we believe that it is a response
**                  to a previous request so handle it here
**
**  Inputs:         IPC_PACKET *pPacket     Pointer to incomming packet
**
**  Returns:        None
**
**  Note:           This function should only be used by Ipc internal
**                  functions
**--------------------------------------------------------------------------*/
static void HandleResponse(IPC_PACKET *pPacket)
{
    SENDER_INFORMATION *p;

    if (!pPacket || !pPacket->header)
    {
        return;
    }

    (void)LockMutex(&inProgressMutex, MUTEX_WAIT);

    /* Need to get a pointer to the correct senderinformation */
    p = FindElement(Sm.inProgress, &pPacket->header->sequenceNumber, CompareSeqId);

    if (p)
    {
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call StateMachine with rxPacket of %p\n", __FILE__, __LINE__, __func__, pPacket);
#endif  /* HISTORY_KEEP */
        StateMachine(p, pPacket);

        /*
         * Don't free the packet here as we will copy out the data in the
         * state machine
         */
    }
    else
    {
        /*
         * Ignore this packet as we do not have an entry for it.  This is
         * caused by a coding error or because of error recovery and we
         * get more than one response packet etc.
         */
        FreePacket(&pPacket, __FILE__, __LINE__);
    }
    UnlockMutex(&inProgressMutex);
}

/*----------------------------------------------------------------------------
**  Function Name:  void RemoveSession( SESSION *pSession)
**
**  Description:    Removes a sesson from the session manager
**
**  Inputs:         SESSION *pSession   Session to remove
**
**  Returns:        None
**
**  Note:           We should never need to remove a session, unless we could
**                  not connect to the client.  Never remove a session after
**                  we have given out a session pointer.
**
** ATTENTION:       sessionListMutex must be locked before calling.
**
**--------------------------------------------------------------------------*/
static void RemoveSession(SESSION *pSession)
{
    SESSION    *rc;

    if (pSession)
    {
        rc = RemoveElement(Sm.sessionList, &pSession->sn, CompareSessionSerialNumbers);

        if (rc)
        {
            Free(rc);
        }
    }
}

/*----------------------------------------------------------------------------
**  Function Name:  void RemoveSessionBindings( SESSION *pSession, bool historyOnly)
**
**  Description:    Removes sesson bindings from a session
**
**  Inputs:         SESSION *pSession   Session to remove
**                  historyOnly         Remove only history, or more?
**
**  Returns:        None
**
** ATTENTION:       inProgressMutex must be locked before calling.
**
**--------------------------------------------------------------------------*/
void RemoveSessionBindings(SESSION *pSession, bool historyOnly)
{
    SENDER_INFORMATION *si;
    IPC_PACKET *pPacket;
    UINT32      hindex;
    UINT32      kcounter;

    if (!pSession)
    {
        return;
    }

    dprintf(DPRINTF_DEFAULT, "RemoveSessionBindings - Removing Bindings to SN %u\n",
            pSession->sn);

    /* Remove History. */
    (void)LockMutex(&historyMutex, MUTEX_WAIT);
    kcounter = 0;
    for (hindex = HLHead(); hindex < ((UINT32)HLHead() + HLElements()); ++hindex)
    {
        if (GetCCBSerialNumber(HLList(hindex % MAX_HISTORY)->header) == pSession->sn)
        {
            HLList(hindex % MAX_HISTORY)->header->ccbSerialNumber = 0;
            HLList(hindex % MAX_HISTORY)->header->commandCode = 0;
            HLList(hindex % MAX_HISTORY)->header->sequenceNumber = 0;
            ++kcounter;
        }
    }
    dprintf(DPRINTF_DEFAULT, "RSB - Removing (%u) History entries for SN %u\n",
            kcounter, pSession->sn);
    UnlockMutex(&historyMutex);

    if (historyOnly == TRUE)
    {
        return;
    }

#ifdef FORKING_COMMAND_DISPATCHER
    /* Remove running tasks. */
    (void)LockMutex(&runningTasksMutex, MUTEX_WAIT);
    kcounter = 0;
    SetIterator(Sm.runningTasks);
    while (NULL != (pPacket = (IPC_PACKET *)Iterate(Sm.runningTasks)))
    {
        if (GetCCBSerialNumber(pPacket->header) == pSession->sn)
        {
            pPacket->header->flags |= PI_HDR_FLG_KILL_IPC_TASK;
            ++kcounter;
        }
    }
    dprintf(DPRINTF_DEFAULT, "RSB - Killing (%u) running tasks for SN %u\n",
            kcounter, pSession->sn);
    UnlockMutex(&runningTasksMutex);
#endif  /* FORKING_COMMAND_DISPATCHER */

    /* Remove packetQueue entries for the session. */
    (void)LockMutex(&packetQueueMutex, MUTEX_WAIT);
    kcounter = 0;
    SetIterator(Sm.packetQueue);
    while (NULL != (pPacket = (IPC_PACKET *)Iterate(Sm.packetQueue)))
    {
        if (GetCCBSerialNumber(pPacket->header) == pSession->sn)
        {
            if (NULL != (pPacket = RemoveElement(Sm.packetQueue, pPacket, PacketHistoryCmp)))
            {
                pPacket->header->flags |= PI_HDR_FLG_KILL_IPC_TASK;
                ++kcounter;
            }
        }
    }
    dprintf(DPRINTF_DEFAULT, "RSB - Killing (%u) packets from packetQueue for SN %u\n",
            kcounter, pSession->sn);
    UnlockMutex(&packetQueueMutex);

    /*
     * Remove outstanding entries for the session.  As a test first
     * iterate through the list to see if any should be removed.
     * The second loop attempts to remove the elements and complete
     * the commands.
     *
     * NOTE: The first loop that iterates is for DEBUG only, we are
     *       unsure if the remove element works since we are unable
     *       to reproduce the problem which gets us into this state.
     *       TBolt00016397 is the issue which caused the change to
     *       be made.
     */
    (void)LockMutex(&outStandingMutex, MUTEX_WAIT);
    kcounter = 0;
    SetIterator(Sm.outStanding);
    while (NULL != (si = (SENDER_INFORMATION *)Iterate(Sm.outStanding)))
    {
        if (si->session == pSession)
        {
            ++kcounter;
        }
    }
    dprintf(DPRINTF_DEFAULT, "RSB - Should need to complete (%u) outstanding entries to SN %u\n",
            kcounter, pSession->sn);

    kcounter = 0;
    while (NULL != (si = (SENDER_INFORMATION *)RemoveElement(Sm.outStanding, pSession, CompareClientRequestSession)))
    {
        si->actualPath = SENDPACKET_NO_PATH;
        CompleteCommand(si);
        ++kcounter;
    }
    dprintf(DPRINTF_DEFAULT, "RSB - Completing (%u) outstanding entries to SN %u\n",
            kcounter, pSession->sn);
    UnlockMutex(&outStandingMutex);

    /* Remove inProgress entries for the session. */
    kcounter = 0;
    SetIterator(Sm.inProgress);
    while (NULL != (si = (SENDER_INFORMATION *)Iterate(Sm.inProgress)))
    {
        if (si->session == pSession)
        {
            ++kcounter;
        }
    }
    dprintf(DPRINTF_DEFAULT, "RSB - Ignoring (%u) inProgress entries to SN %u\n",
            kcounter, pSession->sn);
}

/*----------------------------------------------------------------------------
**  Function Name:  INT8 IPC_ElectionNotify(UINT8 ElectionState)
**
**  Description:    Checks to see if any histories need to be removed for failed controllers
**
**  Inputs:         ElectionState       Current state of election
**
**  Returns:        None
**
**  Note:           None.
**--------------------------------------------------------------------------*/
UINT8 IPC_ElectionNotify(UINT8 ElectionState)
{
    UINT8       cindex;
    UINT32      sn;
    SESSION    *session;
    FAILURE_DATA_STATE fdstate;

    switch (ElectionState)
    {
        case ELECTION_FINISHED:
            break;
        default:
            return GOOD;
    }

    dprintf(DPRINTF_DEFAULT, "IPC_ElectionNotify - Looking for histories to clean up\n");

    for (cindex = 0; cindex < MAX_CONTROLLERS; ++cindex)
    {
        if ((sn = CCM_ControllerSN(cindex)) != 0)
        {
            switch ((fdstate = EL_GetFailureState(cindex)))
            {
                case FD_STATE_OPERATIONAL:
                case FD_STATE_POR:
                case FD_STATE_ADD_CONTROLLER_TO_VCG:
                case FD_STATE_FIRMWARE_UPDATE_ACTIVE:
                case FD_STATE_UNFAIL_CONTROLLER:
                case FD_STATE_ACTIVATE:
                    break;

                case FD_STATE_UNUSED:
                case FD_STATE_FAILED:
                case FD_STATE_STRANDED_CACHE_DATA:
                case FD_STATE_FIRMWARE_UPDATE_INACTIVE:
                case FD_STATE_VCG_SHUTDOWN:
                case FD_STATE_INACTIVATED:
                case FD_STATE_DISASTER_INACTIVE:
                default:
                    dprintf(DPRINTF_DEFAULT, "IPC_ElectionNotify - CN%d, sn %u, state %u checking for session\n",
                            cindex, sn, fdstate);

                    /* Do not need to lock sessionListMutex here. */
                    session = FindElement(Sm.sessionList, &sn, CompareSessionSerialNumbers);
                    if (session)
                    {
                        dprintf(DPRINTF_DEFAULT, "IPC_ElectionNotify - CN%d, sn %u, state %u removing history\n",
                                cindex, sn, fdstate);

                        RemoveSessionBindings(session, TRUE);
                        session = NULL;
                    }
                    break;
            }
        }
    }

    return GOOD;
}

/*----------------------------------------------------------------------------
**  Function Name:  void CloseSocket( SOCKET *socket_number );
**
**  Description:    Closes a TCP socket
**
**  Inputs:         SOCKET socket_number       socket that we want closed
**
**  Returns:        None
**
**  Note:           This function should only be used by Ipc internal
**                  functions
**--------------------------------------------------------------------------*/
void CloseSocket(SOCKET *socket_number)
{
    SOCKET      tmp;
    UINT32      errorNumber;

    if (*socket_number == SOCKET_ERROR)
    {
        return;
    }

    /*
     * Store a temp copy of socket
     * and write the real one to error, this way we should avoid another
     * thread from call CloseSocket yet agin and causing a EBADF error.
     * If this does not fix this error then we can use a mutex instead.
     *
     * Problem only seems to happen when using the Treck stack and when
     * it does happen it takes the stack a long time to recover.
     */
    tmp = *socket_number;
    *socket_number = SOCKET_ERROR;

    LogMessage(LOG_TYPE_DEBUG, "Closing IPC socket %d", tmp);

    if (SOCKET_ERROR == shutdown(tmp, 2))
    {
        errorNumber = errno;
        dprintf(DPRINTF_DEFAULT, "Error on socket shutdown, errno %d/%s.\n",
                errorNumber, GetErrnoString(errorNumber));
    }

    if (SOCKET_ERROR == Close(tmp))
    {
        errorNumber = errno;
        dprintf(DPRINTF_DEFAULT, "%s:%u:%s Error on socket close, errno %d/%s.\n",
                __FILE__, __LINE__, __func__, errorNumber, GetErrnoString(errorNumber));
    }
}

/*----------------------------------------------------------------------------
**  Function Name:  void CloseEthernetSocket( SESSION *pSession);
**
**  Description:    Closes a the ethernet TCP socket for a given session
**
**  Inputs:         SESSION *pSession       sesion that contains the socket
**                                          we are closing
**
**  Returns:        None
**
**  Note:           This function should only be used by Ipc internal
**                  functions
**--------------------------------------------------------------------------*/
void CloseEthernetSocket(SESSION *pSession)
{
    if (pSession)
    {
        if (pSession->eth != SOCKET_ERROR)
        {
            LogMessage(LOG_TYPE_DEBUG, "CloseEthernetSocket: Closing socket %d to %u.",
                       pSession->eth, pSession->sn);

            NotifyTransportChange(pSession, SENDPACKET_ETHERNET, FALSE);
        }
        CloseSocket(&pSession->eth);
    }
}

/*----------------------------------------------------------------------------
**  Function Name:  static bool InitSessionManager( void )
**
**  Description:    Initializes the session manager
**
**  Inputs:         None
**
**  Returns:        True if we completed without errors, else returns false
**
**  Note:           This function should only be used by Ipc internal
**                  functions
**--------------------------------------------------------------------------*/
static bool InitSessionManager(void)
{
    UINT8       index1;

    Sm.sessionList = CreateList();
    Sm.outStanding = CreateList();
    Sm.inProgress = CreateList();
    Sm.packetQueue = CreateList();
    Sm.runningTasks = CreateList();

    /* Initialize the historyList */
    HLHead() = 0;
    HLTail() = 0;
    HLElements() = 0;
    for (index1 = 0; index1 < MAX_HISTORY; ++index1)
    {
        HLList(index1) = NULL;
    }

    /* Store the pcb address */
    Sm.pcbID = XK_GetPcb();

    /* Set this flag */
    Sm.okToAwaken = FALSE;

    return TRUE;
}

/*----------------------------------------------------------------------------
**  Function Name:  static IPC_PACKET *DequeueDataPacket(void)
**
**  Description:    Removes a [acket from the incoming data queue
**
**  Inputs:         None
**
**  Returns:        Pointer to the next packet in the queue, NULL if we are
**                  empty
**
**  Note:           This function should only be used by Ipc internal
**                  functions
**--------------------------------------------------------------------------*/
static IPC_PACKET *DequeueDataPacket(void)
{
    IPC_PACKET *p;

    (void)LockMutex(&packetQueueMutex, MUTEX_WAIT);
    p = (IPC_PACKET *)Dequeue(Sm.packetQueue);
    UnlockMutex(&packetQueueMutex);

    return p;
}

/*----------------------------------------------------------------------------
**  Function Name:  static bool SendClientPacket( SENDER_INFORMATION *si)
**
**  Description:    Sends a packet to a given requester
**
**  Inputs:         SENDER_INFORMATION *si      Sender information structure
**
**  Returns:        True if packet was sent, else returns false
**
**  Note:           This function should only be used by Ipc internal
**                  functions
**
** ATTENTION:   inProgressMutex must be locked before calling.
**
**--------------------------------------------------------------------------*/
static bool SendClientPacket(SENDER_INFORMATION *si)
{
    PATH_TYPE   pathTaken;

    if (!si)
    {
        return FALSE;
    }

    /*
     * We are sending a command and we want the command timeout to be short
     * until we get the command in progress which at that point we want the
     * timeout to be what the user wanted.
     *
     * Set the timeout value
     */
    si->expirationTime = COMMAND_IN_PROGRESS_TMO_MS + K_timel;

    /* Send over the interface that the user wanted */
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s Calling SendPacketOverInterface(si->txPacket) of %p\n", __FILE__, __LINE__, __func__, si->txPacket);
#endif  /* HISTORY_KEEP */
    pathTaken = SendPacketOverInterface(si->session, si->requestedPath, si->txPacket);

    /* Set path to actual path sent */
    si->actualPath = pathTaken;

    return (pathTaken >= SENDPACKET_ANY_PATH ? TRUE : FALSE);
}

/*----------------------------------------------------------------------------
**  Function Name:  void StateMachine(SENDER_INFORMATION *si, IPC_PACKET *rx)
**
**  Description:    Session manager state machine
**
**  Inputs:         SENDER_INFORMATION *    si  Sender information structure
**                  IPC_PACKET *            rx  Incoming packet
**
**  Returns:        none
**
**  Note:           This function should only be used by Ipc internal
**                  functions
**
** ATTENTION:   inProgressMutex must be locked before calling.
**
**--------------------------------------------------------------------------*/
static void StateMachine(SENDER_INFORMATION *si, IPC_PACKET *rx)
{
    bool        done = false;

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s stateMachine of %p is %d\n", __FILE__, __LINE__, __func__, si, si->stateMachine);
#endif  /* HISTORY_KEEP */
    switch (si->stateMachine)
    {
        case (SM_TMO):
            /* Do not retry Quorum operations on timeouts */
            if (si->actualPath == SENDPACKET_QUORUM)
            {
                /* Flag that we done (no retries). */
                done = true;
            }
            /* NO BREAK - falling through */

        case (SM_SEND_REQ):
            if (!done && SendClientPacket(si))
            {
                if (si->actualPath != SENDPACKET_QUORUM)
                {
                    /* We set the state we are done for now */
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s set stateMachine of %p to SM_CMD_IN_PROGRESS\n", __FILE__, __LINE__, __func__, si);
#endif  /* HISTORY_KEEP */
                    si->stateMachine = SM_CMD_IN_PROGRESS;
                }
                else
                {
                    /*
                     * We do not expect an inprogress indicator from the quorum.
                     * We need to reset the expiration timeout to reflect what
                     * the user wanted and not the shorter cmd in progress timeout.
                     */
                    si->expirationTime = si->timeout + K_timel;

                    /* We now expect to get the response */
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s set stateMachine of %p to SM_RESPONSE_REQ\n", __FILE__, __LINE__, __func__, si);
#endif  /* HISTORY_KEEP */
                    si->stateMachine = SM_RESPONSE_REQ;
                }
            }
            else
            {
                /*
                 * If the current state is SM_TMO and we got here that
                 * means that we had a timeout and should note that
                 */
                if (si->stateMachine == SM_TMO)
                {
                    /* We have two different reporting types set both */
                    si->actualPath = SENDPACKET_TIME_OUT;
                }

                /* We are done inform and clean up */
                CompleteCommand(si);
            }
            break;

            /*
             * Because the fibre channel does not guarantee that a packet
             * sent before another packet will arrive in sequence we need
             * to be able to handle an out or order packet sequence
             */
        case (SM_CMD_IN_PROGRESS):
        case (SM_RESPONSE_REQ):
            if (GotCmdInProgress(rx))
            {
                /*
                 * In some cases just writing the packet to the socket is good
                 * enough, in this case just inform the client, or if we did a
                 * retry and got here implies that the we had a command timeout
                 * and then we got the command in progress
                 */
                if (NO_VERIFICATION_REQUESTED(si) || retrySet(si))
                {
                    if (retrySet(si))
                    {
                        /*
                         * We resent a packet and the inprogress came back so
                         * we can derive the fact that the command timed out,
                         * because the communication is working
                         */
                        si->actualPath = SENDPACKET_TIME_OUT;
                    }

                    /* Finish up */
                    CompleteCommand(si);
                }
                else
                {
                    /*
                     * We need to reset the expiration timeout to reflect what
                     * the user wanted and not the shorter cmd in progress timeout.
                     */
                    si->expirationTime = si->timeout + K_timel;

                    /* We now expect to get the response */
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s set stateMachine of %p to SM_RESPONSE_REQ\n", __FILE__, __LINE__, __func__, si);
#endif  /* HISTORY_KEEP */
                    si->stateMachine = SM_RESPONSE_REQ;
                }
            }

            /*
             * We didn't get the command in progress so assume we got the
             * command complete, GotCmdInProgress returns true if rx = false
             */
            else if (rx)
            {
                /* Setup return packet data */
                if (si->rxPacket)
                {
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s set rxPacket header of %p->%p to %p\n", __FILE__, __LINE__, __func__, si, si->rxPacket, rx->header);
#endif  /* HISTORY_KEEP */
                    si->rxPacket->header = rx->header;
                    si->rxPacket->data = rx->data;

                    /*
                     * To prevent the deletion of the memory they point to
                     * when we call FreePacket below
                     */
                    rx->header = NULL;
                    rx->data = NULL;
                }

                /* Finish up */
                CompleteCommand(si);
            }
            else
            {
                dprintf(DPRINTF_IPC_SESSION_MANAGER, "We were expecting the inprogress or the response packet and got something else instead (will most likely lead to IPC timeout)\n");
                dumpSI(si);
            }
            break;

        default:
            break;
    }

    /* Free the rx */
    FreePacket(&rx, __FILE__, __LINE__);
}

/*----------------------------------------------------------------------------
**  Function Name:  static SENDER_INFORMATION *DequeueClientRequest( void )
**
**  Description:    Removes a client request from the outstanding queue
**
**  Inputs:         None
**
**  Returns:        Pointer to a psender info structure, NULL if queue is
**                  empty
**
**  Note:           This function should only be used by Ipc internal
**                  functions
**--------------------------------------------------------------------------*/
static SENDER_INFORMATION *DequeueClientRequest(void)
{
    SENDER_INFORMATION *p;

    ccb_assert(Sm.outStanding != NULL, Sm.outStanding);

    (void)LockMutex(&outStandingMutex, MUTEX_WAIT);
    p = (SENDER_INFORMATION *)Dequeue(Sm.outStanding);
    UnlockMutex(&outStandingMutex);

    return p;
}

/*----------------------------------------------------------------------------
**  Function Name:  static void HandleNewClientRequest(SENDER_INFORMATION *p)
**
**  Description:    Code that handle a new request from a client calling
**                  IpcSendPacket
**
**  Inputs:         Pointer to a sender_information packet
**
**  Returns:        None
**
**  Note:           This function should only be used by Ipc internal
**                  functions
**--------------------------------------------------------------------------*/
static void HandleNewClientRequest(SENDER_INFORMATION *p)
{
    ccb_assert(Sm.inProgress != NULL, Sm.inProgress);

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s set stateMachine of %p to SM_SEND_REQ\n", __FILE__, __LINE__, __func__, p);
#endif  /* HISTORY_KEEP */
    p->stateMachine = SM_SEND_REQ;

    (void)LockMutex(&inProgressMutex, MUTEX_WAIT);
    Enqueue(Sm.inProgress, p);
    StateMachine(p, NULL);
    UnlockMutex(&inProgressMutex);
}

/*----------------------------------------------------------------------------
**  Function Name:  void *Dispatcher( void *pPacket);
**
**  Description:    Function that may get forked for handling the ipc packets.
**
**  Inputs:         Packet pointer
**
**  Returns:        NONE
**
**  Note:           This is all getting ugly
**--------------------------------------------------------------------------*/
static void Dispatcher(TASK_PARMS *parms)
{
    IPC_PACKET *packet = (IPC_PACKET *)parms->p1;
    SESSION    *pSession;
    UINT8       freePacket = 0;
    UINT32      original_command_code;
    UINT32      new_command_code;

    if (!packet)            /* Not likely, but we don't want to crash either */
    {
        return;
    }

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s Dispatcher using packet of %p\n", __FILE__, __LINE__, __func__, packet);
#endif  /* HISTORY_KEEP */

    original_command_code = packet->header->commandCode;

    /* Trace it */
    if (original_command_code == PACKET_IPC_TUNNEL)
    {
        TraceEvent(TRACE_IPC_DISPATCH_TUNNEL_START,
                   ((PI_PACKET_HEADER *)(packet->data->tunnel.packet))->commandCode);
    }
    else
    {
        TraceEvent(TRACE_IPC_DISPATCH_START, original_command_code);
    }

    /* Call the actual command dispactcher */
    packet = IpcCommandDispatcher(packet);
    if (!packet)
    {
        TraceEvent(TRACE_IPC_DISPATCH_NULL, original_command_code);
        return;
    }

    new_command_code = packet->header->commandCode;

    /* Check for cancellation. */
    if (packet->header->flags & PI_HDR_FLG_KILL_IPC_TASK)
    {
        dprintf(DPRINTF_DEFAULT, "IPC Dispatcher cmd %04X killed by RSB after dispatch\n",
                new_command_code);
        freePacket = 1;
    }
    else
    {
        /* What should we do if we cannot get a valid session at this point? */
        pSession = GetSession(GetCCBSerialNumber(packet->header));

        if (pSession != NULL && packet != NULL)
        {
            /* lock inProgressMutex before calling SendPacketOverInterface. */
            (void)LockMutex(&inProgressMutex, MUTEX_WAIT);
            if (SENDPACKET_NO_PATH == SendPacketOverInterface(pSession,
                                     GetReaderInterface(packet->header), packet))
            {
                /* We really cannot do much here and count on the client timing out. */
                dprintf(DPRINTF_IPC_SESSION_MANAGER, "Unable to send a response back to the other closing the sockets for this session\n");
            }
            UnlockMutex(&inProgressMutex);
        }

        /* Check for cancellation. */
        if (packet->header->flags & PI_HDR_FLG_KILL_IPC_TASK)
        {
            dprintf(DPRINTF_DEFAULT, "IPC Dispatcher cmd %04X killed by RSB after response:\n",
                    new_command_code);
            freePacket = 1;
        }
        /* Add packet to the history */
        else if (!PacketHistoryListAdd(packet))
        {
            dprintf(DPRINTF_IPC_SESSION_MANAGER, "Unable to place a packet on the history queue\n");
        }
    }

#ifdef FORKING_COMMAND_DISPATCHER
    /* Remove the task from the running task list after added to the history list */
    if (!RunningTasksListRemove(packet))
    {
        dprintf(DPRINTF_IPC_SESSION_MANAGER, "Unable to remove a task from the running tasks list\n");
    }
#endif  /* FORKING_COMMAND_DISPATCHER */

    /* Do we need to remove the packet? */
    if (freePacket)
    {
        FreePacket(&packet, __FILE__, __LINE__);
    }
    TraceEvent(TRACE_IPC_DISPATCH_DONE, original_command_code);
}


/*----------------------------------------------------------------------------
**  Function Name:  static void HandleIncoming( IPC_PACKET *packet )
**
**  Description:    Each time we get a new packet from one of our reader
**                  threads we process the request here
**
**  Inputs:         IPC_PACKET *packet      Pointer to the packet read
**
**  Returns:        None
**
**  Note:           This function should only be used by Ipc internal
**                  functions
**--------------------------------------------------------------------------*/
static void HandleIncoming(IPC_PACKET *packet)
{
    SESSION    *pSession;
    IPC_PACKET *dupeReq;
    TASK_PARMS  parms;

    if (GetCCBSerialNumber(packet->header) == GetMyControllerSN())
    {
        /* This packet orginated from this CCB so check for what we need to do */
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call HandleResponse with rxPacket of %p\n", __FILE__, __LINE__, __func__, packet);
#endif  /* HISTORY_KEEP */
        HandleResponse(packet);
    }
    else
    {

#ifdef FORKING_COMMAND_DISPATCHER
        /*
         * Check to see if this packet is currently executing, if it is
         * then place this packet back on the queue for incomming packets
         */
        if (NULL != (dupeReq = CheckForRunningTask(packet)))
        {
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s HandleIncoming dupeReq of %p\n", __FILE__, __LINE__, __func__, packet);
#endif  /* HISTORY_KEEP */
            EnqueuePacket(packet);
            return;
        }
#endif  /* FORKING_COMMAND_DISPATCHER */

        /*
         * Check to see if we have recently processed this packet
         * before, if so then send the previous response packet
         */
        if (NULL != (dupeReq = CheckForPreviousPacket(packet)))
        {
            /*
             * We have found a previous packet that matches this one so
             * we need to send the previous packet result
             */
            dprintf(DPRINTF_DEFAULT, "IPC cmd %d already handled, sending previous result\n",
                    packet->header->commandCode);

            pSession = GetSession(GetCCBSerialNumber(packet->header));

            if (pSession && dupeReq)
            {
                /*
                 * Even though we have seen this one before we need to
                 * send the reply over the interface the request came over
                 * and not the interface we sent the orginal reply on
                 */
                /* lock inProgressMutex before calling SendPacketOverInterface. */
                (void)LockMutex(&inProgressMutex, MUTEX_WAIT);
                if (SENDPACKET_NO_PATH == SendPacketOverInterface(pSession,
                                            GetReaderInterface(packet->header), dupeReq))
                {
                    dprintf(DPRINTF_DEFAULT, "We had a packet request that we just processed but we were unable to send the previous response back. This is not a crital error\n");
                }
                UnlockMutex(&inProgressMutex);
            }

            /*
             * Free the packet, since it is otherwise not being done
             * in this branch of code.
             */
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s HandleIncoming CheckForPreviousPacket of %p\n", __FILE__, __LINE__, __func__, packet);
#endif  /* HISTORY_KEEP */
            FreePacket(&packet, __FILE__, __LINE__);
        }
        else
        {
            /*
             * Dispatch the command to the command handler
             * Need to ifdef each type forking and non-forking
             * command dispatchers
             */
#ifndef FORKING_COMMAND_DISPATCHER
            Dispatcher(packet);
#else   /* FORKING_COMMAND_DISPATCHER */

            /* Add task to running list */
            if (!RunningTasksListAdd(packet))
            {
                dprintf(DPRINTF_IPC_SESSION_MANAGER, "Unable to place a packet on the running task list\n");
            }

            /*
             * Fork the command dispatcher so that we can handle
             * concurrancy, but now we risk running out of process space.
             */
            parms.p1 = (UINT32)packet;
            TaskCreate(Dispatcher, &parms);
#endif  /* FORKING_COMMAND_DISPATCHER */
        }
    }
}


/*----------------------------------------------------------------------------
**  Function Name:  void *CreateSessionManager( void *parms);
**
**  Description:    Function that gets forked for session manager
**
**  Inputs:         None, parameter it for the OS
**
**  Returns:        Only if we messed up
**
**  Note:           Should only be one of these at any time     .
**--------------------------------------------------------------------------*/
void CreateSessionManager(UNUSED TASK_PARMS *parms)
{
#define TMO_INTERVAL    1           /* Really a +2 due to less than check. */
#define MAX_IN_A_ROW    40

    IPC_PACKET *pPacket;
    SENDER_INFORMATION *pRequest;
    UINT32      timeInterval;
    UINT32      in_a_row = 0;

    /* Initialize the session manager */
    if (!InitSessionManager())
    {
        return;
    }

    /* Set the time interval */
    timeInterval = K_timel + TMO_INTERVAL;

    /* This task should never exit ever, if we have written it correctly */
    while (FOREVER)
    {
        /*
         * Check to see if we have any incoming packets from our sockets
         * to process
         */
        pPacket = DequeueDataPacket();

        if (pPacket)
        {
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call HandleIncoming with pPacket of %p\n", __FILE__, __LINE__, __func__, pPacket);
#endif  /* HISTORY_KEEP */
            HandleIncoming(pPacket);

            if (in_a_row++ > MAX_IN_A_ROW)
            {
                if (NumberOfItems(Sm.outStanding) != 0 ||
                    NumberOfItems(Sm.packetQueue) != 0)
                {
                    /* In the CCB give everyone else time to run */
                    TaskSwitch();
                    in_a_row = 0;
                }
            }
        }

        /* Check to see if we have received a new request from IpcSendpacket */
        pRequest = DequeueClientRequest();

        if (pRequest)
        {
            HandleNewClientRequest(pRequest);

            if (in_a_row++ > MAX_IN_A_ROW)
            {
                if (NumberOfItems(Sm.outStanding) != 0 ||
                    NumberOfItems(Sm.packetQueue) != 0)
                {
                    /* In the CCB give everyone else time to run */
                    TaskSwitch();
                    in_a_row = 0;
                }
            }
        }

        if (timeInterval < K_timel)
        {
            CheckForCommandTimeOuts();

            /* Set the new time interval */
            timeInterval = K_timel + TMO_INTERVAL;

            if (in_a_row++ > MAX_IN_A_ROW)
            {
                if (NumberOfItems(Sm.outStanding) != 0 ||
                    NumberOfItems(Sm.packetQueue) != 0)
                {
                    /* In the CCB give everyone else time to run */
                    TaskSwitch();
                    in_a_row = 0;
                }
            }
        }

        /* If we have absolutely nothing to do we will get suspended here */
        if (NumberOfItems(Sm.outStanding) == 0 &&
            NumberOfItems(Sm.packetQueue) == 0)
        {
            if (NumberOfItems(Sm.inProgress) == 0)
            {
                Sm.okToAwaken = TRUE;
                TaskSetState(Sm.pcbID, PCB_NOT_READY);

                TaskSwitch();
                Sm.okToAwaken = FALSE;
            }
            else
            {
                INT32 sleeptime = timeInterval - K_timel;
                if (sleeptime >= 0)
                {
                    TaskSleepMS((sleeptime + 1) * 125);   /* Convert to ms from alarm ticks */
                }
                else
                {
                    /* In the CCB give everyone else time to run */
                    TaskSwitch();
                }
            }
            in_a_row = 0;
        }
    }
}

/*----------------------------------------------------------------------------
**  Function Name:  void CheckForCommandTimeOuts(void)
**
**  Description:    Checks the inProgress list for any send requests that
**                  have taken too long
**
**  Inputs:         None
**
**  Returns:        None
**
**  Note:           This function should only be used by Ipc internal
**                  functions
**--------------------------------------------------------------------------*/
static void CheckForCommandTimeOuts(void)
{
    UINT32      currentTime;
    SENDER_INFORMATION *p;

    /* Make sure we have a valid list pointer */
    if (!Sm.inProgress)
    {
        return;
    }

    (void)LockMutex(&inProgressMutex, MUTEX_WAIT);

    /* Walk the list looking for timeouts */
    SetIterator(Sm.inProgress);

    /* Get and store the current time */
    currentTime = K_timel;

    while (NULL != (p = (SENDER_INFORMATION *)Iterate(Sm.inProgress)))
    {
        /*
         * If an actual timeout occurred or the request is on a
         * path that is not good a TMO will be signaled.
         */
        if ((p->timeout && (currentTime >= p->expirationTime)) ||
            (SENDPACKET_ETHERNET == p->actualPath && !ETH_GOOD(p->session)) ||
            (SENDPACKET_FIBRE == p->actualPath && !FC_GOOD(p->session)))
        {
            /*
             * Print the correct message for the reason for getting
             * in here, it was either a request on a failed interface
             * or it was an actual timeout.  Both of these cases are
             * handled the same way, close the socket and set the
             * state to be TMO.
             */
            if ((SENDPACKET_ETHERNET == p->actualPath && !ETH_GOOD(p->session)) ||
                (SENDPACKET_FIBRE == p->actualPath && !FC_GOOD(p->session)))
            {
                dprintf(DPRINTF_IPC_SESSION_MANAGER, "Request on failed interface\n");
            }
            else
            {
                dprintf(DPRINTF_IPC_SESSION_MANAGER, "Timeout occurred\n");
            }

            dumpSI(p);

            if ((p->stateMachine == SM_RESPONSE_REQ) && !retrySet(p))
            {
                dprintf(DPRINTF_IPC_SESSION_MANAGER, "Retry this command\n");

                /*
                 * We want to make sure the connection is not the
                 * problem before we close the connection
                 */
                setRetry(p);
            }
            else
            {
                /* Whatever socket we used was not good so mark it bad */
                if (SENDPACKET_ETHERNET == p->actualPath)
                {
                    CloseEthernetSocket(p->session);
                }
                else if (SENDPACKET_FIBRE == p->actualPath)
                {
                    dprintf(DPRINTF_IPC_SESSION_MANAGER,
                            "Timeout on fibre channel direct\n");
                    p->session->fcDirect = FALSE;
                    NotifyTransportChange(p->session, SENDPACKET_FIBRE, FALSE);
                }

                /* Clear the retry */
                clearRetry(p);
            }

            /*
             * We got a timeout so set the state to tmo and call
             * the state machine
             */
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s set stateMachine of %p to SM_TMO\n", __FILE__, __LINE__, __func__, p);
#endif  /* HISTORY_KEEP */
            p->stateMachine = SM_TMO;
            StateMachine(p, NULL);
        }
    }
    UnlockMutex(&inProgressMutex);
}

/*----------------------------------------------------------------------------
**  Function Name:  void SendResponseQueueDataPacketToSm(
**                                              IPC_PACKET *rxPacket,
**                                              PATH_TYPE interface);
**
**  Description:    Sends a command in progress packet if needed,
**                  Places a packet in the incoming data queue for the
**                  session manager
**
**  Inputs:         rxPacket    Packet that was just received
**                  interface   Which interfacce we read it on
**
**  Returns:        None
**
**  Note:           This function should only be used by Ipc internal
**                  functions
**--------------------------------------------------------------------------*/
void SendResponseQueueDataPacketToSm(IPC_PACKET *rxPacket, PATH_TYPE interface)
{
    SESSION    *pSession;

    if (!rxPacket)
    {
        return;
    }

    /* Set the packet so that we know which interface this packet came on */
    SetReaderInterface(rxPacket->header, interface);

    /* Get the session */
    pSession = GetSession(GetCCBSerialNumber(rxPacket->header));

    /*
     * The ugly story, by Tony Asleson
     * If a CCB is not sending packets he does not know when a network
     * connection has gone bad. Futhermore, we added code to fix a race
     * condition when two CCB's @ the same time try to connect we break the
     * deadlock by using the SSN. Well if a CCB with a winning SN does
     * not know that his network connection is bad and dumps all new
     * connections becuase he has a better SN we are stuck.  So now we
     * will check to make sure that the requested path if any did not
     * go over eth then we will mark it bad. Get it? got it? Good.
     *
     */
    if ((GetSenderInterface(rxPacket->header) == SENDPACKET_ANY_PATH) &&
        (interface != SENDPACKET_ETHERNET))
    {
        /* Close the ethernet */
        CloseEthernetSocket(pSession);
    }

    /*
     * Because FIBRE_CHANNEL_DIRECT does not have a good mechanism for
     * letting the other side know that it is operational we will mark
     * it as good when we read from it
     */
    if (SENDPACKET_FIBRE == interface)
    {
        if (pSession)
        {
            /*
             * If the fibre channel direct was down before then
             * notify whomever that we read from it
             */
            if (!pSession->fcDirect)
            {
                NotifyTransportChange(pSession, SENDPACKET_FIBRE, TRUE);
            }
            pSession->fcDirect = TRUE;
        }
    }

    if (GetCCBSerialNumber(rxPacket->header) != GetMyControllerSN())
    {
        /*
         * We have successfully received a packet from a
         * different system, let that system know that
         * we have it by sending a command in progress
         * packet
         */
        SendCommandInProgress(rxPacket, interface);

        TaskSwitch();
    }

    /* Enqueue to the session manager */
    EnqueuePacket(rxPacket);
}

/*----------------------------------------------------------------------------
**  Function Name: SendCommandInProgress
**
**  Description:    Function sends the command in progress back to the
**                  sender that sent the packet
**
**  Inputs:
**                  IPC_PACKET *packet      Pointer to the packet that we just
**                                          read from the socket
**                  PATH_TYPE  interface    Which interface we read it on
**
**  Returns:    None
**--------------------------------------------------------------------------*/
static void SendCommandInProgress(IPC_PACKET *packet, PATH_TYPE interface)
{
    SESSION    *mySession;
    IPC_PACKET *cmdInProgress;

    /* Get the session for this CCB */
    mySession = GetSession(GetCCBSerialNumber(packet->header));

    /* Create the command in progress packet */
    cmdInProgress = CreateResponsePacket(PACKET_IPC_COMMAND_STATUS, packet,
                                         sizeof(IPC_COMMAND_STATUS), __FILE__, __LINE__);

    if (mySession && cmdInProgress)
    {
        cmdInProgress->data->commandStatus.status = IPC_COMMAND_IN_PROGRESS;

        /* No Need to check return code as we are sending this in blind faith */
        /* lock inProgressMutex before calling SendPacketOverInterface. */
        (void)LockMutex(&inProgressMutex, MUTEX_WAIT);
        SendPacketOverInterface(mySession, interface, cmdInProgress);
        UnlockMutex(&inProgressMutex);
    }

    FreePacket(&cmdInProgress, __FILE__, __LINE__);
}

/*----------------------------------------------------------------------------
**  Function Name: NotifyTransportChange
**
**  Description:    Common function set the status of a given interface and
**                  allow the centalized reporting of a transport as up or
**                  down.
**                  Since we are in the middle of an IPC transport and the
**                  the mechanism to report failures requires the IPC transport,
**                  the failure reporting is forked from here.
**
**  Inputs:
**              SESSION *pSession       Session in which we are refering to
**              PATH_TYPE  pt           Which transport
**              bool working            Set to true if interface is working
**
**
**  Returns:    None
**--------------------------------------------------------------------------*/
static void NotifyTransportChange(SESSION *pSession, PATH_TYPE pt, bool working)
{
    TASK_PARMS  parms;

    if (pSession)
    {
        if (working)
        {
            /* Interface is working */
            if (pt == SENDPACKET_ETHERNET)
            {
                /* Add code to indicate that Ethernet is up for this session */
                parms.p1 = (UINT32)GetMyControllerSN();
                parms.p2 = (UINT32)pSession->sn;
                parms.p3 = (UINT32)IPC_LINK_TYPE_ETHERNET;
                parms.p4 = (UINT32)PROCESS_CCB;
                parms.p5 = (UINT32)IPC_LINK_ERROR_OK;
                TaskCreate(UpdateLinkStatus, &parms);

                dprintf(DPRINTF_IPC_SESSION_MANAGER, "Eth is up\n");
            }
            else if (pt == SENDPACKET_FIBRE)
            {
                /* Add code to indicate that Fibre is up for this session */
                parms.p1 = (UINT32)GetMyControllerSN();
                parms.p2 = (UINT32)pSession->sn;
                parms.p3 = (UINT32)IPC_LINK_TYPE_FIBRE;
                parms.p4 = (UINT32)PROCESS_CCB;
                parms.p5 = (UINT32)IPC_LINK_ERROR_OK;
                TaskCreate(UpdateLinkStatus, &parms);
                dprintf(DPRINTF_IPC_SESSION_MANAGER, "Fibre is up\n");
            }
        }
        else
        {
            /* Interface is down */
            if (pt == SENDPACKET_ETHERNET)
            {
                /* Add code to indicate that Ethernet is down for this session */
                parms.p1 = (UINT32)GetMyControllerSN();
                parms.p2 = (UINT32)pSession->sn;
                parms.p3 = (UINT32)IPC_LINK_TYPE_ETHERNET;
                parms.p4 = (UINT32)PROCESS_CCB;
                parms.p5 = (UINT32)IPC_LINK_ERROR_FAILED;
                TaskCreate(UpdateLinkStatus, &parms);

                dprintf(DPRINTF_IPC_SESSION_MANAGER, "Eth is down\n");
            }
            else if (pt == SENDPACKET_FIBRE)
            {
                /* Add code to indicate that Fibre is down for this session */
                parms.p1 = (UINT32)GetMyControllerSN();
                parms.p2 = (UINT32)pSession->sn;
                parms.p3 = (UINT32)IPC_LINK_TYPE_FIBRE;
                parms.p4 = (UINT32)PROCESS_CCB;
                parms.p5 = (UINT32)IPC_LINK_ERROR_FAILED;
                TaskCreate(UpdateLinkStatus, &parms);
                dprintf(DPRINTF_IPC_SESSION_MANAGER, "Fibre is down\n");
            }
        }
    }
}


static void dumpSI(SENDER_INFORMATION *p)
{
    char        buf[1024];
    char       *pBuf = buf;
    char        ap_tempString[10];
    char        rp_tempString[10];

    if (p)
    {
        EL_GetSendPacketResultString(ap_tempString, p->actualPath, sizeof(ap_tempString));
        EL_GetSendPacketResultString(rp_tempString, p->requestedPath, sizeof(rp_tempString));
        pBuf += sprintf(pBuf,
                        "\n**** Sender information dump ****\n"
                        "Actual path       = %d (%s)\n"
                        "Requested path    = %d (%s)\n"
                        "stateMachine      = %d\n"
                        "callback          = %p\n"
                        "callback parms    = %p\n"
                        "callback result   = %p\n"
                        "TMO               = %u\n"
                        "seqNum            = %u\n",
                        p->actualPath, ap_tempString,
                        p->requestedPath, rp_tempString,
                        p->stateMachine,
                        p->callBack,
                        p->callBackParams, p->callBackResult, p->timeout, p->seqNum);
        if (p->txPacket)
        {
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s printing (p->txPacket) of %p\n", __FILE__, __LINE__, __func__, p->txPacket);
#endif  /* HISTORY_KEEP */
            pBuf += sprintf(pBuf, "txPacket          = %p\n", p->txPacket);
            if (p->txPacket->header != NULL && (UINT32)p->txPacket->header != 0xfffffffe)
            {
                pBuf += sprintf(pBuf,
                                " txCommandCode    = %u\n"
                                " txsequenceNumber = %u\n",
                                p->txPacket->header->commandCode,
                                p->txPacket->header->sequenceNumber);
            }
            else
            {
                pBuf += sprintf(pBuf, " txPacket->header = 0x%08x\n", (UINT32)p->txPacket->header);
            }
        }
        else
        {
            pBuf += sprintf(pBuf, "txPacket          = NULL\n");
        }

        /*
         * A SENDPACKET_QUORUM type already has freed the rxPacket, but does not
         * clear the timeout rxPacket -- and it is thus invalid.
         */
        if (p->actualPath != SENDPACKET_QUORUM && p->rxPacket)
        {
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s dumpSI use rxPacket of %p->%p\n", __FILE__, __LINE__, __func__, p, p->rxPacket);
#endif  /* HISTORY_KEEP */
            pBuf += sprintf(pBuf, "rxPacket          = %p\n", p->rxPacket);
            if (p->rxPacket->header != NULL)
            {
                pBuf += sprintf(pBuf, " rxCommandCode    = %u\n"
                                      " rxsequenceNumber = %u\n",
                                      p->rxPacket->header->commandCode,
                                      p->rxPacket->header->sequenceNumber);
            }
            else
            {
                pBuf += sprintf(pBuf, " rxPacket->header = %p\n", p->rxPacket->header);
            }
        }
        else
        {
            pBuf += sprintf(pBuf, "rxPacket          = %p\n", p->rxPacket);
        }
    }
    dprintf(DPRINTF_DEFAULT, "%s", buf);
}                               /* End of dumpSI */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
