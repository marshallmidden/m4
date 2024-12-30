/* $Id: ipc_sendpacket.c 156532 2011-06-24 21:09:44Z m4 $ */
/**
******************************************************************************
**
**  @file       ipc_sendpacket.c
**
**  @brief      Implementation for the ipc send packet
**
**  Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "ipc_sendpacket.h"

#include "debug_files.h"
#include "EL.h"
#include "ipc_common.h"
#include "ipc_socket_io.h"
#include "ipc_session_manager.h"
#include "ipc_packets.h"
#include "logging.h"
#include "misc.h"
#include "mode.h"
#include "PacketStats.h"
#include "PI_Utils.h"
#include "quorum_utils.h"
#include "trace.h"
#include "XIO_Std.h"
#include "CT_history.h"

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
/* Add in how many milliseconds the FE/BE discovery is allowed to take. */
#define TIMEFACTOR  10000

#define TMO_SEND_IPC_PING           (TIMEFACTOR + 1000)
#define TMO_SEND_IPC_RESYNC         (TIMEFACTOR + 1000)
#define TMO_SEND_IPC_OFFLINE        (TIMEFACTOR + 1000)
#define TMO_SEND_IPC_SIGNAL         120000

/*****************************************************************************
** Private functions
*****************************************************************************/
static INT32 IpcSendSignal(UINT32 serialNum, UINT16 signalEvent);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      IpcSendPacket
**
**              This function allows a CCB to send one packet to another
**              CCB based on CCB serial number and what path they would
**              like the packet to take.
**
**  @param      session         Pointer to a session, use GetSession
**  @param      requestedPath   Path you want the packet to go over
**  @param      txPacket        Pointer to a packet to transmit
**  @param      rxPacket        Pointer to the returning data packet
**  @param      callBack        Pointer to a function to call when completed
**  @param      callBackParameters  Pointer to the parameter to be
**                                  passed to the call back funtion
**  @param      asyncResult -   Result of async operation
**  @param      tmo             How long you want to wait in ms
**                              to the nearest rounded up 125 ms
**
**  @return     Actual path the packet was sent on.  Just because you get a
**              good return code for the send does not mean you will get the
**              response and rxPacket->header and rxPacket->data can be NULL
**
******************************************************************************
**/
PATH_TYPE IpcSendPacket(SESSION *session,
                        PATH_TYPE requestedPath,
                        IPC_PACKET *txPacket,
                        IPC_PACKET *rxPacket,
                        void (*callBack)(void *params),
                        void *callBackParameters,
                        IPC_CALL_BACK_RESULTS *asyncResult, UINT32 tmo)
{
    /*
     * Initialize the rc to not path initially
     */
    PATH_TYPE   rc = SENDPACKET_NO_PATH;
    UINT32      tmo_global = GetGlobalIPCTimeout();

    ccb_assert(session != NULL, session);
    ccb_assert(requestedPath >= SENDPACKET_ANY_PATH, requestedPath);
    ccb_assert(txPacket != NULL, txPacket);
    ccb_assert(txPacket->header != NULL, txPacket->header);
    ccb_assert(rxPacket != NULL, rxPacket);

    /*
     * If the global timeout is greater than zero and it is greater than the
     * timeout passed in, use the global timeout value.
     */
    if (tmo_global > 0 && tmo_global > tmo)
    {
        tmo = tmo_global;
    }

    if ((session && txPacket && txPacket->header) &&
        (!(callBack && (asyncResult == NULL))))
    {
        if (txPacket->header->commandCode == PACKET_IPC_TUNNEL)
        {
            TraceEvent(TRACE_IPC_TUNNEL_START,
                       ((PI_PACKET_HEADER *)(txPacket->data->tunnel.packet))->commandCode);
        }
        else
        {
            TraceEvent(TRACE_IPC_START, txPacket->header->commandCode);
        }

        IncPacketCount(PACKET_TYPE_IPC, txPacket->header->commandCode);

        /*
         * We have passed all the parameter checks to set the rc to what the
         * users wants for now.
         */
        rc = requestedPath;

        /*
         * Basically if we are trying to send a packet to someone as
         * a new request then we want a new sequence number
         * MAKE SURE THIS IS CORRECT
         */
        if (GetCCBSerialNumber(txPacket->header) == GetMyControllerSN())
        {
            /*
             * In the case that we are equal
             */
            SetSeqNum(txPacket->header, NextSequenceNumber());
        }

        /*
         * If the user is looking for a result packet make sure we initialize
         * the pointers
         */
        if (rxPacket != NULL)
        {
            /*
             * Make sure that we have an initialized packet here
             */
            rxPacket->header = NULL;
            rxPacket->data = NULL;
#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s set rxPacket header of %p to %p\n", __FILE__, __LINE__, __func__, rxPacket, rxPacket->header);
#endif  /* HISTORY_KEEP */
        }

        /*
         * Set the header to indicate which interface the user wanted
         * this information is a dupe, but what the heck (your a monster, shrek)
         *
         * Only sleep on blocking calls
         */
        SetSenderInterface(txPacket->header, requestedPath);

        if (session &&
            QueueSenderInformation(session, &rc, txPacket, rxPacket,
                                   callBack, callBackParameters,
                                   asyncResult, tmo, XK_GetPcb()) && (callBack == NULL))
        {
            /* Put this process to sleep -- PCB_IPC_WAIT status. */
            TakeNap();
        }
    }

    if (txPacket && txPacket->header)
    {
        TraceEvent(TRACE_IPC + rc, txPacket->header->commandCode);
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      IpcSendPacketBySN
**
**              This function is identical to IpcSendPacket except it
**              gets the session based on a serial number and uses it
**              when calling IpcSendPacket.
**
**  @param      serialNum       Serial number of the controller you
**                              want to send the packet to.
**  @param      session         Pointer to a session, use GetSession
**  @param      requestedPath   Path you want the packet to go over
**  @param      txPacket        Pointer to a packet to transmit
**  @param      rxPacket        Pointer to the returning data packet
**  @param      callBack        Pointer to a function to call when completed
**  @param      callBackParameters  Pointer to the parameter to be
**                                  passed to the call back funtion
**  @param      asyncResult -   Result of async operation
**  @param      tmo             How long you want to wait in ms
**                              to the nearest rounded up 125 ms
**
**  @return     Actual path the packet was sent on.  Just because you get a
**              good return code for the send does not mean you will get the
**              response and rxPacket->header and rxPacket->data can be NULL
**
******************************************************************************
**/
PATH_TYPE IpcSendPacketBySN(UINT32 serialNum,
                            PATH_TYPE requestedPath,
                            IPC_PACKET *txPacket,
                            IPC_PACKET *rxPacket,
                            void (*callBack)(void *params),
                            void *callBackParameters,
                            IPC_CALL_BACK_RESULTS *asyncResult, UINT32 tmo)
{
    SESSION    *pSession = NULL;
    PATH_TYPE   pathType = SENDPACKET_NO_PATH;

    ccb_assert(serialNum > 0, serialNum);
    ccb_assert(requestedPath >= SENDPACKET_ANY_PATH, requestedPath);
    ccb_assert(txPacket != NULL, txPacket);
    ccb_assert(txPacket->header != NULL, txPacket->header);
    ccb_assert(rxPacket != NULL, rxPacket);

    pSession = GetSession(serialNum);

    if (pSession != NULL)
    {

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call IpcSendPacket with rxPacket of %p\n", __FILE__, __LINE__, __func__, rxPacket);
#endif  /* HISTORY_KEEP */

        pathType = IpcSendPacket(pSession, requestedPath,
                                 txPacket, rxPacket,
                                 callBack, callBackParameters, asyncResult, tmo);
    }
    else
    {
        LogMessage(LOG_TYPE_DEBUG, "IpcSendPacketBySN: Failed to get a valid session");
    }

    return (pathType);
}

/**
******************************************************************************
**
**  @brief      IpcSendPingWithRetries
**
**              Sends the IPC_PING packet to the controller specified
**              by serialNum over the interface specified by pathType
**              and retry the operation if desired.
**
**  @param      serialNum   - serial number of the controller to send
**                            the ping packet to.
**  @param      desiredPath - Path to use when sending the ping packet.
**  @param      retries     - Number of times to retry sending the PING
**                            event to the controller.
**
**  @return     PI_GOOD or PI_ERROR
**
******************************************************************************
**/
INT32 IpcSendPingWithRetries(UINT32 serialNum, PATH_TYPE desiredPath, UINT8 retries)
{
    INT32       rc = PI_GOOD;
    UINT8       count = 0;

    dprintf(DPRINTF_IPC_SENDPACKET, "IpcSendPingWithRetries (serialNum: 0x%x, desiredpath: 0x%x, retries: 0x%x): ENTER\n",
            serialNum, desiredPath, retries);

    ccb_assert(serialNum > 0, serialNum);
    ccb_assert(desiredPath >= SENDPACKET_ANY_PATH, desiredPath);

    do
    {
        /*
         * Increment the retry counter, if the number of retries
         * was zero we will only attempt the ping once, otherwise
         * if the ping failed we will retry until we exhaust the
         * the retry counter.
         */
        count++;

        /*
         * Delay for 1 second(s) before sending the PING if the
         * previous PING failed (the first time through rc is
         * initialized to PI_GOOD so it won't delay).
         */
        if (rc != PI_GOOD)
        {
            TaskSleepMS(1000);
        }

        /*
         * Send the ping on the desired path.
         */
        rc = IpcSendPing(serialNum, desiredPath);

        /*
         * Loop until the ping returns a good status or the
         * retry count has been exhausted.
         */
    } while (rc != PI_GOOD && count <= retries);

    dprintf(DPRINTF_IPC_SENDPACKET, "IpcSendPingWithRetries: EXIT (rc: 0x%x)\n", rc);

    return rc;
}

/**
******************************************************************************
**
**  @brief      IpcSendPing
**
**              Sends the IPC_PING packet to the controller specified
**              by serialNum over the interface specified by pathType.
**
**  @param      serialNum   - serial number of the controller to send
**                            the ping packet to.
**  @param      desiredPath - Path to use when sending the ping packet.
**
**  @return     PI_GOOD or PI_ERROR
**
******************************************************************************
**/
INT32 IpcSendPing(UINT32 serialNum, PATH_TYPE desiredPath)
{
    INT32       rc = GOOD;
    SESSION    *pSession;
    IPC_PACKET  rx = { NULL, NULL };
    PATH_TYPE   pathType;
    IPC_PACKET *ptrPacket;
    UINT8      retries;

    dprintf(DPRINTF_IPC_SENDPACKET, "IpcSendPing (serialNum: 0x%x, desiredpath: 0x%x): ENTER\n",
            serialNum, desiredPath);

    ccb_assert(serialNum > 0, serialNum);
    ccb_assert(desiredPath >= SENDPACKET_ANY_PATH, desiredPath);

    pSession = GetSession(serialNum);

    if (pSession != NULL)
    {
        /* Bring up ethernet and fibre interfaces. */
        BringUpInterface(pSession, SENDPACKET_ETHERNET);
        BringUpInterface(pSession, SENDPACKET_FIBRE);

        /* Create the PING packet. */
        ptrPacket = CreatePacket(PACKET_IPC_PING, sizeof(IPC_PING), __FILE__, __LINE__);

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call IpcSendPacket with rxPacket of %p\n", __FILE__, __LINE__, __func__, &rx);
#endif  /* HISTORY_KEEP */

        retries = 2;                /* Ethernet, Fiber(1), Disk Quorum(2) */
        do
        {
            Free(rx.data);

            /* Send the request to the controller. */
            pathType = IpcSendPacket(pSession, desiredPath,
                                     ptrPacket, &rx, NULL, NULL, NULL, TMO_SEND_IPC_PING);
        } while (pathType == SENDPACKET_NO_PATH && (retries--) > 0);

        if (!IpcSuccessfulXfer(pathType))
        {
            LogMessage(LOG_TYPE_DEBUG, "IpcSendPing: Send packet failed (%u).", pathType);
            rc = ERROR;
        }
        FreePacket(&ptrPacket, __FILE__, __LINE__);
        FreePacketStaticPacketPointer(&rx, __FILE__, __LINE__);
    }
    else
    {
        LogMessage(LOG_TYPE_DEBUG, "IpcSendPing: Failed to get a valid session.");
        rc = ERROR;
    }

    dprintf(DPRINTF_IPC_SENDPACKET, "IpcSendPing: EXIT (rc: 0x%x)\n", rc);

    return (rc);
}

/**
******************************************************************************
**
**  @brief      TunnelRequest
**
**              Send a request to another CCB via an IPC tunnel packet.
**              This function will tunnel a request to a master or a slave
**              (i.e. no master checking is done).
**
**  @param      ctrlSN  - controller to send the request to
**  @param      pXIOReq - pointer to a request packet
**  @param      pXIORsp - pointer to a response packet
**
**  @return     PI_GOOD, PI_ERROR or PI_TUNNEL_ERROR
**
**  @attention  Memory for pXIORsp->packet is allocated in this function.
**              It must be freed by the caller.
**
******************************************************************************
**/
INT32 TunnelRequest(UINT32 ctrlSN, XIO_PACKET *pXIOReq, XIO_PACKET *pXIORsp)
{
    PATH_TYPE   pathType = SENDPACKET_ANY_PATH;
    IPC_PACKET *pIPCReq;
    IPC_PACKET  ipcRsp = { NULL, NULL };
    PI_PACKET_HEADER *pXioHdr;
    UINT8      *pXioData;
    UINT32      rc = PI_GOOD;
    UINT32      ipcReqSize;

    ccb_assert(ctrlSN > 0, ctrlSN);
    ccb_assert(pXIOReq != NULL, pXIOReq);
    ccb_assert(pXIORsp != NULL, pXIORsp);

    dprintf(DPRINTF_IPC_SENDPACKET, "TunnelRequest (0x%x, 0x%x): ENTER\n",
            ctrlSN, pXIOReq->pHeader->commandCode);

    /*
     * Create the IPC request packet.  CreatePacket() creates and fills
     * in the header portion of the IPC packet and allocates memory for
     * the data portion.  The tunneled packet (header + request) goes in
     * the data portion of the IPC packet.
     */
    ipcReqSize = sizeof(IPC_TUNNEL) + sizeof(*pXIOReq->pHeader) +
                 pXIOReq->pHeader->length;

    pIPCReq = CreatePacket(PACKET_IPC_TUNNEL, ipcReqSize, __FILE__, __LINE__);

    /*
     * Copy the XIO request header and request packet from the XIO_PACKET
     * request struct into the IPC request packet.
     */
    memcpy(pIPCReq->data->tunnel.packet, pXIOReq->pHeader, sizeof(*pXIOReq->pHeader));

    memcpy(pIPCReq->data->tunnel.packet + sizeof(*pXIOReq->pHeader),
           pXIOReq->pPacket, pXIOReq->pHeader->length);

    /*
     * If the request packet requested a specific path to be used copy
     * that value to pathType variable to be used in the send.
     */
    if (pXIOReq->pHeader->senderInterface > 0)
    {
        pathType = pXIOReq->pHeader->senderInterface;
    }

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call IpcSendPacketBySN with rxPacket of %p\n", __FILE__, __LINE__, __func__, &ipcRsp);
#endif  /* HISTORY_KEEP */

    /* Send the request to the other controller via IPC. */
    pathType = IpcSendPacketBySN(ctrlSN, pathType, pIPCReq,
                                 &ipcRsp, NULL, NULL, NULL, RMCMDHDL_IPC_SEND_TMO);

    /* Check that the IPC transfer was successful. */
    if (!IpcSuccessfulXfer(pathType))
    {
        LogMessage(LOG_TYPE_DEBUG, "TunnelRequest IPC ERROR! cmd: 0x%X pathType: 0x%X",
                   pXIOReq->pHeader->commandCode, pathType);

        rc = PI_TUNNEL_ERROR;
    }
    else
    {
        /*
         * Get pointers to the response header and data from the IPC
         * response data.
         */
        pXioHdr = (PI_PACKET_HEADER *)(ipcRsp.data->tunnel.packet);
        pXioData = ipcRsp.data->tunnel.packet + sizeof(*pXIORsp->pHeader);

        /*
         * The request was successfully sent and responded, copy the
         * response header into the XIO_PACKET response struct.
         */
        memcpy(pXIORsp->pHeader, pXioHdr, sizeof(*pXIORsp->pHeader));

        /*
         * If the tunneled request was successful copy the IPC response
         * packet into the XIO_PACKET response struct.
         */
        if (ipcRsp.data->tunnel.status == IPC_COMMAND_SUCCESSFUL)
        {
            if (pXioHdr->length == 0)
            {
                pXIORsp->pPacket = 0;
            }
            else
            {
                /* Allocate memory for the response data, and copy data */
                pXIORsp->pPacket = MallocWC(pXioHdr->length);
                memcpy(pXIORsp->pPacket, pXioData, pXioHdr->length);
            }
        }
        else
        {
            /* The tunneled request failed.  Generate a log message. */
            LogMessage(LOG_TYPE_DEBUG, "TunnelRequest ERROR! cmd: 0x%X  status: 0x%X  errorCode: 0x%X",
                       pXIOReq->pHeader->commandCode,
                       pXioHdr->status, pXioHdr->errorCode);
            rc = PI_ERROR;
        }
    }

    FreePacket(&pIPCReq, __FILE__, __LINE__);       /* From CreatePacket() */
    FreePacketStaticPacketPointer(&ipcRsp, __FILE__, __LINE__);

    dprintf(DPRINTF_IPC_SENDPACKET, "TunnelRequest (0x%x, 0x%x): EXIT\n",
            ctrlSN, pXIOReq->pHeader->commandCode);

    return (rc);
}

/**
******************************************************************************
**
**  @brief      IpcSendOffline
**
**              Sends the IPC_OFFLINE packet to the controller specified
**              by serialNum.
**
**  @param      serialNum - serial number of the controller.
**  @param      delay
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
INT32 IpcSendOffline(UINT32 serialNum, UINT32 delay)
{
    INT32       rc = GOOD;
    IPC_PACKET  rx = { NULL, NULL };
    PATH_TYPE   pathType;
    IPC_PACKET *ptrPacket;
    UINT8       retries = 2;                /* Ethernet, Fiber(1), Disk Quorum(2) */

    dprintf(DPRINTF_IPC_SENDPACKET, "IpcSendOffline: ENTER (sn: 0x%x, delay: %u)\n",
            serialNum, delay);

    ptrPacket = CreatePacket(PACKET_IPC_OFFLINE, sizeof(IPC_OFFLINE), __FILE__, __LINE__);

    ptrPacket->data->offline.WaitTime = delay;

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call IpcSendPacketBySN with rxPacket of %p\n", __FILE__, __LINE__, __func__, &rx);
#endif  /* HISTORY_KEEP */

    do
    {
        Free(rx.data);

        /* Sending packet to the other controller using any IPC path possible */
        pathType = IpcSendPacketBySN(serialNum, SENDPACKET_ANY_PATH,
                                     ptrPacket, &rx, NULL, NULL, NULL, TMO_SEND_IPC_OFFLINE);
    } while (pathType == SENDPACKET_NO_PATH && (retries--) > 0);

    if (!IpcSuccessfulXfer(pathType))
    {
        LogMessage(LOG_TYPE_DEBUG, "IPC_OFFLINE FAIL (0x%x, 0x%x)", serialNum, pathType);

        rc = ERROR;
    }
    else
    {
        LogMessage(LOG_TYPE_DEBUG, "IPC_OFFLINE OK (0x%x)", serialNum);
    }

    FreePacket(&ptrPacket, __FILE__, __LINE__);
    FreePacketStaticPacketPointer(&rx, __FILE__, __LINE__);

    dprintf(DPRINTF_IPC_SENDPACKET, "IpcSendOffline: EXIT\n");

    return (rc);
}

/**
******************************************************************************
**
**  @brief      IpcSignalSlaves
**
**              Send each of the slave controllers an IPC_SIGNAL message.
**
**  @param      signalEvent - Signal event to send to the slave controllers.
**
**  @return     none
**
******************************************************************************
**/
void IpcSignalSlaves(UINT16 signalEvent)
{
    UINT32      rc = GOOD;
    UINT32      count;
    UINT32      configIndex;
    UINT32      controllerSN;

    for (count = 0; count < Qm_GetNumControllersAllowed(); count++)
    {
        configIndex = Qm_GetActiveCntlMap(count);

        if (configIndex != ACM_NODE_UNDEFINED)
        {
            controllerSN = cntlConfigMap.cntlConfigInfo[configIndex].controllerSN;

            if (controllerSN != 0 && controllerSN != Qm_GetMasterControllerSN())
            {
                rc = IpcSendSignal(controllerSN, signalEvent);

                if (rc == ERROR)
                {
                    LogMessage(LOG_TYPE_DEBUG, "IpcSignalSlaves: Failed to send signal (%u, 0x%x).",
                               controllerSN, signalEvent);
                }
            }
        }
    }
}

/**
******************************************************************************
**
**  @brief      IpcSendSignal
**
**              Sends the IPC_SIGNAL packet to the controller specified
**              by serialNum.
**
**  @param      serialNum - serial number of the controller.
**  @param      signalEvent - Signal event to send.
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
INT32 IpcSendSignal(UINT32 serialNum, UINT16 signalEvent)
{
    INT32       rc = GOOD;
    IPC_PACKET  rx = { NULL, NULL };
    PATH_TYPE   pathType;
    IPC_PACKET *ptrPacket;

    ptrPacket = CreatePacket(PACKET_IPC_SIGNAL, sizeof(IPC_SIGNAL), __FILE__, __LINE__);

    ptrPacket->data->signal.signalEvent = signalEvent;

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call IpcSendPacketBySN with rxPacket of %p\n", __FILE__, __LINE__, __func__, &rx);
#endif  /* HISTORY_KEEP */

/* NOTE: This is not retried through other interfaces -- this is called during startup. */
    pathType = IpcSendPacketBySN(serialNum, SENDPACKET_ETHERNET,
                                 ptrPacket, &rx, NULL, NULL, NULL, TMO_SEND_IPC_SIGNAL);

    if (!IpcSuccessfulXfer(pathType))
    {
        LogMessage(LOG_TYPE_DEBUG, "IpcSendSignal: Send packet failed (%u).", pathType);
        rc = ERROR;
    }
    FreePacket(&ptrPacket, __FILE__, __LINE__);
    FreePacketStaticPacketPointer(&rx, __FILE__, __LINE__);

    return (rc);
}

/**
******************************************************************************
**
**  @brief      IpcSendResyncPersistentData
**
**              Sends the IPC_CLIENT_PERSISTENT packet to the controller specified
**              by serialNum.
**
**  @param      serialNum - serial number of the controller.
**  @param      desiredPath - path to be sent.
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/

INT32 IpcSendResyncPersistentData(UINT32 serialNum)
{
    IPC_PACKET *ptrPacket;
    IPC_PACKET  rx = { NULL, NULL };
    INT32       rc = GOOD;
    PATH_TYPE   pathType;
    UINT8       retries = 2;                /* Ethernet, Fiber(1), Disk Quorum(2) */

    ccb_assert(serialNum > 0, serialNum);

    /* Create the resyncpacket */
    ptrPacket = CreatePacket(PACKET_IPC_RESYNC_CLIENT_CMD, sizeof(IPC_RESYNC_CLIENT_CMD), __FILE__, __LINE__);

    /* store the controller serial number in the packet */
    ptrPacket->data->resyncClientCmd.slaveSN = GetMyControllerSN();

    dprintf(DPRINTF_DEFAULT, "IpcSendResyncPersistentData: sending RESYNC_CLIENT_CMD to %d\n",
            serialNum);

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call IpcSendPacketBySN with rxPacket of %p\n", __FILE__, __LINE__, __func__, &rx);
#endif  /* HISTORY_KEEP */

    do
    {
        Free(rx.data);

        /* Send the request to the controller. */
        pathType = IpcSendPacketBySN(serialNum, SENDPACKET_ANY_PATH,
                                     ptrPacket, &rx, NULL, NULL, NULL, TMO_SEND_IPC_RESYNC);
    } while (pathType == SENDPACKET_NO_PATH && (retries--) > 0);

    if (!IpcSuccessfulXfer(pathType))
    {
        LogMessage(LOG_TYPE_DEBUG, "IpcSendResyncPersistentData: Send packet failed (%u).",
                   pathType);
        rc = ERROR;
    }
    FreePacket(&ptrPacket, __FILE__, __LINE__);
    FreePacketStaticPacketPointer(&rx, __FILE__, __LINE__);

    dprintf(DPRINTF_IPC_SENDPACKET, "IpcSendResyncPersistentdata: EXIT (rc: 0x%x)\n", rc);

    return (rc);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
