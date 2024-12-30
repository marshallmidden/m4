/* $Id: quorum_comm.c 143020 2010-06-22 18:35:56Z m4 $*/
/*============================================================================
** FILE NAME:       quorum_comm.c
** MODULE TITLE:    Quorum communications implementation
**
** DESCRIPTION:     The quorum manager is responsible for two main functions:
**                  1. Providing an interface to allow access to quorum data
**                  2. Providing a mechanism to transfer and monitor packets
**                     through the quorum communications area, slave/master.
**
** Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "XIO_Std.h"
#include "debug_files.h"
#include "EL.h"
#include "cps_init.h"
#include "ipc_cmd_dispatcher.h"
#include "ipc_packets.h"
#include "ipc_session_manager.h"
#include "kernel.h"
#include "LargeArrays.h"
#include "misc.h"
#include "quorum.h"
#include "quorum_comm.h"
#include "quorum_utils.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
#define QM_STATE_IDLE   0
#define QM_STATE_BUSY   1

/*****************************************************************************
** Private variables
*****************************************************************************/
static QM_COMM_PORT inBox[MAX_CONTROLLERS];
static QM_COMM_PORT outBox[MAX_CONTROLLERS] LOCATE_IN_SHMEM;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static UINT32 PollQuorumCommunications(void);
static UINT32 CheckCommunications(UINT32 controllerSN,
                                  QM_COMM_SECTOR *commSector, QM_COMM_PORT *commPortPtr,
                                  UINT16 slotID, MessageDirection direction);
static bool ProtocolCheck(MessageState prevState, MessageState currState);
static void HandleProtocolError(UINT32 controllerSN,
                                UINT16 slotID, MessageDirection direction,
                                QM_COMM_PORT *commPortPtr);
static UINT32 HandleQuorumCommand(QM_COMM_SECTOR *sectorPtr);
static void HandleQuorumPacket(TASK_PARMS *parms);
static UINT32 SendQuorumCommunications(UINT16 slotID, MessageDirection direction,
                                       QM_COMM_PORT *commPortPtr, MessageState state);
static UINT32 InitQuorumComm(void);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: QuorumManager
**
**  Description:
**      The quorum manager monitors quorum communications for the CCB controller.
**      1. It will poll the quorum communications sector for each of the available
**         controllers, once a second.
**      2. Updates the message state in the communications slot
**      3. If a communications sector is allocated, copies quorum sector to the
**         communications sector
**      4. It will perform protocol checking on both inbound and outbound
**         communications.
**      5. It will detect new inbound communications.
**
**--------------------------------------------------------------------------*/
NORETURN void QuorumManager(UNUSED TASK_PARMS *parms)
{
    UINT32      quorumState = QM_STATE_IDLE;

    /* Perform any initialization associated with the Quorum Communications */
    InitQuorumComm();

    /* Loop forever */
    while (1)
    {
        /* Exchange and wait of the next quorum poll cycle */
        TaskSleepMS(QUORUM_POLL_PERIOD);

        /*
         * If we are part of a Virtual controller group, then poll the
         * quorum communications area.
         */
        if (PartOfVCG())
        {
            /*
             * If one of our interfaces is down (Ethernet or Fibre) or the
             * quorum is not idle, then poll the communications slots
             */
            if (!CheckInterfaceStatuses() || quorumState != QM_STATE_IDLE)
            {
                /* Poll all commumincations slots. */
                quorumState = PollQuorumCommunications();
            }
        }
    }
}


/*----------------------------------------------------------------------------
**  Function Name: PollQuorumCommunications
**
**  Description:
**      The quorum manager monitors quorum communications for the CCB controller.
**      1. It will poll the quorum communications sector for each of the available
**         controllers, once a second.
**      2. Updates the message state in the communications slot
**      3. If a communications sector is allocated, copies quorum sector to the
**         communications sector
**      4. It will perform protocol checking on both inbound and outbound
**         communications.
**      5. It will detect new inbound communications.
**
**  Returns:    QM_IDLE - all communications slots are idle
**              QM_BUSY - communication in progress in one or more slots
**
**--------------------------------------------------------------------------*/
static UINT32 PollQuorumCommunications(void)
{
    UINT32      quorumState = QM_STATE_IDLE;
    UINT32      inState = QM_STATE_IDLE;
    UINT32      outState = QM_STATE_IDLE;
    UINT8       i;
    UINT16      mySlot = GetCommunicationsSlot(GetMyControllerSN());

    if (ReadAllMailboxes(gIPCMailbox) == 0)
    {
        /*
         * Loop through all of the available communications slots looking for
         * activity
         */
        for (i = 0; i < MAX_CONTROLLERS; ++i)
        {
            /* Check the state of the inbound mail */
            inState |= CheckCommunications(GetMyControllerSN(),
                                           &gIPCMailbox[mySlot].commSector[i],
                                           &inBox[i], i, INBOUND);
        }

        for (i = 0; i < MAX_CONTROLLERS; ++i)
        {
            /* Check the state of the outbound transfers */
            outState |= CheckCommunications(GetMyControllerSN(),
                                            &gIPCMailbox[i].commSector[mySlot],
                                            &outBox[i], i, OUTBOUND);
        }

        /*
         * If either the inbound or the outbound communications slot
         * has a transfer in progress, mark the quorum as busy
         */
        if ((inState != QM_STATE_IDLE) || (outState != QM_STATE_IDLE))
        {
            quorumState = QM_STATE_BUSY;
        }
    }
    return (quorumState);
}


/*----------------------------------------------------------------------------
**  Function Name: CommunicationsCheck
**
**  Description:
**      This function will perform a quorum area protocol check for the data
**      just read versus the previous state of the quorum port. If the
**      protocol was good, it will update the state of the port. If this is
**      a new meesage to an Inbound port, a communications sector will be
**      allocated and a task forked to handle the command. If a communications
**      sector exists, the newly read data will be copied into that sector.
**
**      If a protocol error occured, the communications port will be cleaned up.
**
**  Inputs:
**          commSector - pointer to communications sector just read from quorum
**          commPort   - communications sector within a controller slot
**          direction  - direction of the communications relative to this
**                       controller.
**
**--------------------------------------------------------------------------*/
static UINT32 CheckCommunications(UINT32 controllerSN,
                                  QM_COMM_SECTOR *commSector, QM_COMM_PORT *commPortPtr,
                                  UINT16 slotID, MessageDirection direction)
{
    UINT32      rc = QM_STATE_IDLE;
    TASK_PARMS  parms;

    /* Get the state of the inbound transfer. */
    MessageState currMsgState = commSector->messageState;
    MessageState prevMsgState = commPortPtr->state;

    /* Check the communications protocol for this slot */
    if (ProtocolCheck(prevMsgState, currMsgState) == 0)
    {
        /* Did the state of the communications slot change? */
        if (currMsgState != prevMsgState)
        {
            dprintf(DPRINTF_QUORUMCOMM, "Quorum State Read = %d, slot = %d, dir = %d\n",
                    currMsgState, slotID, direction);

            /* Change the message state in the communications slot */
            commPortPtr->state = currMsgState;

            /* If a new message was received, allocate a communications sector. */
            if ((currMsgState == QM_NEW_MESSAGE) && (direction == INBOUND))
            {
                /*
                 * Allocate a communications sector.
                 * If the current pointer is not Null, generate an assert.
                 */
                ccb_assert(commPortPtr->sectorPtr == NULL, commPortPtr->sectorPtr);

                commPortPtr->sectorPtr = MallocSharedWC(sizeof(*commPortPtr->sectorPtr));
            }

            /*
             * If the comm sector is allocated,
             * copy the communications data to the appropriate slot
             */
            if (commPortPtr->sectorPtr != NULL)
            {
                memcpy((char *)commPortPtr->sectorPtr,
                       (char *)commSector, sizeof(*commPortPtr->sectorPtr));
            }

            /*
             * If this is a new message, fork a task to handle the new
             * request.
             */
            if ((currMsgState == QM_NEW_MESSAGE) && (direction == INBOUND))
            {
                parms.p1 = (UINT32)slotID;
                parms.p2 = (UINT32)commPortPtr;
                TaskCreate(HandleQuorumPacket, &parms);
            }
        }
    }
    else
    {
        dprintf(DPRINTF_QUORUMCOMM, "Protocol Error - curr = %d, prev =  %d, Slot = %d, Dir = %d\n",
                currMsgState, prevMsgState, slotID, direction);

        /* A protocol error occurred, clean up the communications slot */
        HandleProtocolError(controllerSN, slotID, direction, commPortPtr);
    }

    if (currMsgState != QM_IDLE)
    {
        rc = QM_STATE_BUSY;
    }
    return (rc);
}


/*----------------------------------------------------------------------------
**  Function Name: SendQuorumPacket
**
**  Description:
**      The SendQuorumPacket routine is responsible for initiating packet
**      transfers through the quorum area. The function determines the
**      communications slot to use based on the controller ID. It will block,
**      if the communications sector is not available. After initiating the
**      transfer, by writing the packet to the communications slot, it will
**      block waiting for the packet command to complete or timeout. It will
**      return the communications slot to IDLE when finished.
**
**  Inputs:
**          controllerSN - controller s/n of target controller
**          txPacket     - transmit packet
**
**  Returns:  0  = good completion
**            !0 = error
**
**--------------------------------------------------------------------------*/
INT32 SendQuorumPacket(UINT32 controllerSN, IPC_PACKET *txPacket)
{
    QM_COMM_PORT *commPortPtr;
    QM_COMM_SECTOR *sectorPtr;
    IPC_PACKET *rxPacket;
    MessageDirection direction;
    UINT16      slot;
    UINT8       timeoutCount;
    UINT8       rc = GOOD;

    /*
     * Validity check the Inputs. The length of the transmit data (plus header)
     * should not be greater the size of the data portion of the quorum packet.
     * The controller ID should also be valid for this controller group. This
     * will be checked when we establish the communications slot.
     */
    if (txPacket->header->length + sizeof(*txPacket->header) > sizeof(commPortPtr->sectorPtr->data))
    {
        dprintf(DPRINTF_QUORUMCOMM, "SendQuorumPacket: txPacket->header->length (%d) too big\n",
                txPacket->header->length);
        return SQP_PARM_ERROR;
    }

    dprintf(DPRINTF_QUORUMCOMM, "SQT: (sn=0x%x cmd=%d)\n", controllerSN, txPacket->header->commandCode);

    /* Determine which communications port to use. */
    slot = GetCommunicationsSlot(controllerSN);
    commPortPtr = &outBox[slot];
    direction = OUTBOUND;

    /* Ensure we received a valid slot (i.e. the controller ID was valid). */
    if (slot >= MAX_CONTROLLERS)
    {
        dprintf(DPRINTF_QUORUMCOMM, "SQT: invalid slot\n");
        return SQP_PARM_ERROR;
    }

    /* Allocate a communications buffer. And copy in the packet header and data. */
    sectorPtr = MallocSharedWC(sizeof(*sectorPtr));

    /* Copy the packet to be transmitted into the communications buffer. */
    memcpy((char *)sectorPtr->data, (char *)txPacket->header, sizeof(*txPacket->header));
    memcpy((char *)sectorPtr->data + sizeof(*txPacket->header),
           (char *)txPacket->data, txPacket->header->length);

    /*
     * If the communications port is locked for sending by another task, wait for
     * it to become free.
     */
    while (commPortPtr->sendLock != UNLOCKED)
    {
        dprintf(DPRINTF_QUORUMCOMM, "SQT: waiting, port locked\n");
        TaskSleepMS(QUORUM_POLL_PERIOD / 2);
    }

    /*
     * If the port is not locked for sending, then it should be idle. If not,
     * then this is a violation of protocol. In this case, clean up the
     * communications port and return an error.
     */
    if (commPortPtr->state != QM_IDLE)
    {
        /* A protocol error occurred, clean up the communications slot */
        dprintf(DPRINTF_QUORUMCOMM, "SQT: Protocol Error\n");
        HandleProtocolError(controllerSN, slot, OUTBOUND, commPortPtr);
        return SQP_PROTOCOL_ERROR;
    }

    /* Lock the communications slot from other tasks */
    commPortPtr->sendLock = LOCKED;

    /* Point communications sector at the packet to transmit. */
    commPortPtr->sectorPtr = sectorPtr;

    /* Set the message state for a New Message and write it to the Quorum. */
    if (SendQuorumCommunications(slot, direction, commPortPtr, QM_NEW_MESSAGE) == 0)
    {
        /*
         * Loop here waiting for the operation to complete. Sample the status at
         * twice the rate that the Quorum manager will update it. Exit if one of
         * the following occurs:
         *  1. The state is idle after 8 samples (the Quorum Manager is not active
         *     or a protocol error occurred).
         *  2. The operation completes
         *  3. The timeout count is exceeded
         */
        timeoutCount = 0;
        while (1)
        {
            if ((commPortPtr->state == QM_IDLE) && (timeoutCount > 8))
            {
                dprintf(DPRINTF_QUORUMCOMM, "SQT: Inprogress timeout\n");
                rc = SQP_TIMEOUT_ERROR;
                break;
            }
            else if (commPortPtr->state == QM_MESSAGE_COMPLETE)
            {
                /* Create a response packet to copy response header and data. */
                rxPacket = CreatePacket(((IPC_PACKET_HEADER *)(commPortPtr->sectorPtr->data))->commandCode,
                                        ((IPC_PACKET_HEADER *)(commPortPtr->sectorPtr->data))->length,
                                        __FILE__, __LINE__);

                /* Copy response to response packet. */
                memcpy((char *)rxPacket->header,
                       (char *)commPortPtr->sectorPtr->data, sizeof(*rxPacket->header));
                memcpy((char *)rxPacket->data,
                       (char *)commPortPtr->sectorPtr->data + sizeof(*rxPacket->header),
                       rxPacket->header->length);

                /* Queue the packet to the IPC session manager */
                SendResponseQueueDataPacketToSm(rxPacket, SENDPACKET_QUORUM);
                break;
            }
            else if (timeoutCount > 30)
            {
                dprintf(DPRINTF_QUORUMCOMM, "SQT: Command timeout\n");
                rc = SQP_TIMEOUT_ERROR;
                break;
            }
            else
            {
                TaskSleepMS(QUORUM_POLL_PERIOD / 2);
                ++timeoutCount;
            }
        }
    }
    else
    {
        rc = SQP_WRITE_ERROR;
    }

    /* If the communications port is not IDLE, set it back to IDLE. */
    if (commPortPtr->state != QM_IDLE)
    {
        /*
         * Write the sector state to Idle and wait for the Quorum manager
         * to recognize it.
         */
        if (SendQuorumCommunications(slot, direction, commPortPtr, QM_IDLE) != 0)
        {
            rc = SQP_WRITE_ERROR;
        }
    }

    /*
     * Free the communications sector, set the pointer to Null, and unlock
     * the communications slot
     */
    Free(sectorPtr);
    commPortPtr->sectorPtr = NULL;
    commPortPtr->sendLock = UNLOCKED;

    dprintf(DPRINTF_QUORUMCOMM, "SQT: EXITED (sn= 0x%x)\n", controllerSN);
    return rc;
}

/*----------------------------------------------------------------------------
**  Function Name: HandleQuorumPacket
**
**  Inputs:
**          commPortPtr - pointer to communications port just read from quorum
**
**--------------------------------------------------------------------------*/
static void HandleQuorumPacket(TASK_PARMS *parms)
{
    UINT16      slot = (UINT16)parms->p1;
    QM_COMM_PORT *commPortPtr = (QM_COMM_PORT *)parms->p2;
    MessageDirection direction = INBOUND;

    /* Ensure we received a valid slot (i.e. the controller ID was valid). */
    if (slot >= MAX_CONTROLLERS)
    {
        return;
    }

    /* Write the sector state to received */
    if (SendQuorumCommunications(slot, direction, commPortPtr, QM_MESSAGE_RECEIVED) == 0)
    {
        /*
         * Strip the transmit packet from the quorum data and handle the
         * command. Return the response packet in the quorum sector.
         */
        HandleQuorumCommand(commPortPtr->sectorPtr);

        /* Send the response to the quorum, indicating message complete. */
        SendQuorumCommunications(slot, direction, commPortPtr, QM_MESSAGE_COMPLETE);
    }

    /* Free the communications sector buffer and set the pointer to NULL. */
    Free(commPortPtr->sectorPtr);
    commPortPtr->sectorPtr = NULL;

    return;
}


/*----------------------------------------------------------------------------
**  Function Name: ProtocolCheck
**
**  Description:
**      This function checks the protocol of the quorum communications. It
**      examines the current state of the communications sector and compares
**      it to the previous state of the sector.
**
**  Inputs:
**          prevState - previous state of the communications sector
**          currState - current state of the communications sector
**
**  Returns:  0  = NO protocol error
**            1  = protocol error detected
**
**--------------------------------------------------------------------------*/
static bool ProtocolCheck(MessageState prevState, MessageState currState)
{
    bool        rc = FALSE;

    /*
     * Examine the current state of the communications sector and compare it
     * to the previous state.
     */
    switch (currState)
    {
        case QM_IDLE:
            /*
             * The state is now IDLE. Make sure it was previously IDLE or
             * transitioned from MESSAGE COMPLETE.
             */
            if ((prevState != QM_MESSAGE_COMPLETE) && (prevState != QM_IDLE))
            {
                rc = TRUE;
            }
            break;

        case QM_NEW_MESSAGE:
            /*
             * The state is now NEW MESSAGE. Make sure it was previously NEW
             * MESSAGE  or  transitioned from IDLE.  Or MESSAGE_COMPLETE if
             * back to back messages were sent.
             */
            if ((prevState != QM_NEW_MESSAGE) &&
                (prevState != QM_MESSAGE_COMPLETE) && (prevState != QM_IDLE))
            {
                rc = TRUE;
            }
            break;

        case QM_MESSAGE_RECEIVED:
            /*
             * The state is now RECEIVED. Make sure it was previously RECEIVED
             * or transitioned from NEW MESSAGE.
             */
            if ((prevState != QM_NEW_MESSAGE) && (prevState != QM_MESSAGE_RECEIVED))
            {
                rc = TRUE;
            }
            break;

        case QM_MESSAGE_COMPLETE:
            /*
             * The state is now COMPLETE. Make sure it was previously COMPLETE
             * or transitioned from NEW MESSAGE or RECEIVED.
             */
            if ((prevState != QM_NEW_MESSAGE) &&
                (prevState != QM_MESSAGE_RECEIVED) && (prevState != QM_MESSAGE_COMPLETE))
            {
                rc = TRUE;
            }
            break;

        default:
            rc = TRUE;
            break;
    }
    return (rc);
}


/*----------------------------------------------------------------------------
**  Function Name: HandleProtocolError
**
**  Description:
**      This function checks the protocol of the quorum communications. It
**      examines the current state of the communications sector and compares
**      it to the previous state of the sector.
**
**  Inputs:
**          prevState - previous state of the communications sector
**          currState - current state of the communications sector
**
**  Returns:  0  = NO protocol error
**            1  = protocol error detected
**
**--------------------------------------------------------------------------*/
static void HandleProtocolError(UNUSED UINT32 controllerSN,
                                UINT16 slotID, MessageDirection direction,
                                QM_COMM_PORT *commPortPtr)
{
    QM_COMM_PORT *pSector = NULL;

    /* Generate assert */
    dprintf(DPRINTF_QUORUMCOMM, "HandleProtocolError: slotID = %d\n", slotID);

    if (commPortPtr->sectorPtr == NULL)
    {
        pSector = MallocSharedWC(sizeof(*pSector));
        commPortPtr->sectorPtr = (void *)pSector;
    }

    /*
     * Write the sector state to Idle and wait for the Quorum manager
     * to recognize it.
     */
    if (SendQuorumCommunications(slotID, direction, commPortPtr, QM_IDLE) != 0)
    {
        dprintf(DPRINTF_QUORUMCOMM, "HandleProtocolError: slotID = %d\n", slotID);
    }

    /* Free the memory we allocated */
    if (pSector)
    {
        Free(pSector);
        commPortPtr->sectorPtr = NULL;
    }
}


/*----------------------------------------------------------------------------
**  Function Name: HandleQuorumCommand
**
**  Description:
**      This function takes an IPC command recieved through the quorum, converts
**      the command to a IPC packet, calls the command dispatcher, and converts
**      the response packet back to a quorum sector buffer.
**
**  Inputs:
**          sectorPtr - pointer to communications sector
**
**  Returns:  0  = NO protocol error
**
**--------------------------------------------------------------------------*/
static UINT32 HandleQuorumCommand(QM_COMM_SECTOR *sectorPtr)
{
    IPC_PACKET *rxPacket;
    IPC_PACKET *txPacket;

    /* Set up the input packet from the sector received and call the command dispatcher. */
    txPacket = CreatePacket(((IPC_PACKET_HEADER *)(sectorPtr->data))->commandCode,
                            ((IPC_PACKET_HEADER *)(sectorPtr->data))->length, __FILE__, __LINE__);

    /* Copy rtransmit packet, first the header and then the data. */
    memcpy((char *)txPacket->header, (char *)sectorPtr->data, sizeof(*txPacket->header));

    memcpy((char *)txPacket->data, (char *)sectorPtr->data + sizeof(*txPacket->header),
           txPacket->header->length);

    /*
     * Call the command dispatcher. The dispatcher will handle freeing
     * memory for the transmit packet.
     */
    dprintf(DPRINTF_QUORUMCOMM, "Command Dispatched (cmd = %d)\n",
            txPacket->header->commandCode);

    rxPacket = IpcCommandDispatcher(txPacket);
    if (rxPacket)
    {
        dprintf(DPRINTF_QUORUMCOMM, "Command Completed\n");

        /* Copy response packet into the sector buffer */
        memcpy((char *)sectorPtr->data, (char *)rxPacket->header, sizeof(*rxPacket->header));

        memcpy((char *)sectorPtr->data + sizeof(*rxPacket->header), (char *)rxPacket->data,
               rxPacket->header->length);

        FreePacket(&rxPacket, __FILE__, __LINE__);
    }
    return 0;
}


/*----------------------------------------------------------------------------
**  Function Name:  InitQuorumComm
**
**  Description:    Initialize Quorum communitions data structures
**
**  Inputs:         None
**
**  Returns:        GOOD -  successful
**
**--------------------------------------------------------------------------*/
static UINT32 InitQuorumComm(void)
{
    QM_COMM_PORT commPort;
    UINT32      rc = GOOD;
    UINT16      slot;

    dprintf(DPRINTF_QUORUMCOMM, "Quorum communications initialized.\n");

    /* Initialize communications data structure */
    memset(&inBox, 0x00, sizeof(inBox));
    memset(&outBox, 0x00, sizeof(outBox));

    /*
     * Initialize the Quorum communications sectors, to ensure they start
     * in a valid state.
     * Wait for the backend processor to be ready and ensure we own drives.
     */
    while (!PowerUpComplete() || Qm_GetOwnedDriveCount() == 0)
    {
        TaskSleepMS(1000);
    }

    /* Initialize communications sector and set port state to IDLE */
    commPort.sectorPtr = MallocSharedWC(sizeof(*commPort.sectorPtr));

    commPort.state = QM_IDLE;

    /* Get the communications slot ID for this controller */
    slot = GetCommunicationsSlot(GetMyControllerSN());

    /* Ensure we received a valid slot (i.e. the controller ID was valid). */
    if (slot >= MAX_CONTROLLERS)
    {
        rc = SQP_PARM_ERROR;
    }

    /* Write the INBOUND and OUTBOUND communications ports to IDLE */
    if (rc == GOOD)
    {
        SendQuorumCommunications(slot, INBOUND, &commPort, QM_IDLE);
        SendQuorumCommunications(slot, OUTBOUND, &commPort, QM_IDLE);
    }

    Free(commPort.sectorPtr);
    return (rc);
}


/*----------------------------------------------------------------------------
**  Function Name: SendQuorumCommunications
**
**  Description:  Send communications change through the quorum.
**
**  Inputs: slotID       - controller slot ID in the Quorum
**          direction    - direction of the requested transfer
**          sectorPtr    - pointer location to retrieve the sector
**
**  Returns:  0  = good completion
**            !0 = file i/o error
**
**--------------------------------------------------------------------------*/
static UINT32 SendQuorumCommunications(UINT16 slotID, MessageDirection direction,
                                       QM_COMM_PORT *commPortPtr, MessageState state)
{
    /* Write the sector state to received */
    commPortPtr->sectorPtr->messageState = state;

    if (WriteMailbox(slotID, direction, commPortPtr->sectorPtr) == 0)
    {
        dprintf(DPRINTF_QUORUMCOMM, "Quorum State Written = %d, slot = %d, dir = %d\n",
                state, slotID, direction);

        /* Change the state of the port */
        commPortPtr->state = state;
        return (0);
    }
    else
    {
        /* Write failed - generate a log event */
        return (-1);
    }
}


/*----------------------------------------------------------------------------
**  Function Name:  QuorumSendablePacket(UINT16 commandCode)
**
**  Description:    Determines if a packet can be sent over the quorum
**
**  Inputs:         UINT16 Command code, Command code of the packet to be sent
**
**  Returns:        TRUE if the quorum can send it, otherwise FALSE
**
**--------------------------------------------------------------------------*/
bool QuorumSendablePacket(UINT16 commandCode)
{
    switch (commandCode)
    {
        case (PACKET_IPC_REPORT_CONTROLLER_FAILURE):
        case (PACKET_IPC_COMMAND_STATUS):
        case (PACKET_IPC_PING):
        case (PACKET_IPC_ONLINE):
        case (PACKET_IPC_OFFLINE):
        case (PACKET_IPC_ELECT_QUORUM):
            /* Yes we can   :-) */
            return TRUE;
    }
    return FALSE;
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
