/* $Id: iscsi_init.c 144139 2010-07-14 19:46:01Z m4 $ */
/**
 ******************************************************************************
 **
 **  @file       iscsi_init.c
 **
 **  @brief      iSCSI initialization functions
 **
 **  This provides API's for interfacing with TSL(transport sub layer) and
 **  helping functions for iSCSI processing
 **
 **  Copyright (c) 2005-2010 Xiotech Corporation.  All rights reserved.
 **
 ******************************************************************************
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <fcntl.h>
#include <unistd.h>
#include <byteswap.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include "XIO_Std.h"
#include "XIO_Types.h"
#include "XIO_Macros.h"
#include "iscsi_common.h"
#include "iscsi_pdu.h"
#include "iscsi_tsl.h"
#include "iscsi_digest.h"
#include "iscsi_tmf.h"
#include "fsl.h"

#include "iscsi_timer.h"

#include "MR_Defs.h"            /* MRP definitions                  */
#include "ilt.h"                /* ILT definitions                  */
#include "pm.h"
#include "mem_pool.h"
#include "CT_defines.h"

/*
******************************************************************************
** Private functions - externed in the header file
******************************************************************************
*/
int         iscsiSendNopin(UINT32, UINT32);
UINT8       iscsiReadData(CONNECTION * pConn, INT32 length);
UINT8       iscsiRRData(CONNECTION * pConn);
UINT8       iscsiReadBHS(CONNECTION * pConn);
UINT8       iscsiRRBHS(CONNECTION * pConn);
UINT8       iscsiRecvData(CONNECTION * pConn);
INT32       iscsiIsAhsPresent(CONNECTION * pConn);
INT32       iscsiIsDatasegPresent(CONNECTION * pConn);

#define READ_HDR_COMP       2
#define READ_HDR_INCOMP     3
#define READ_DATA_COMP      4
#define READ_DATA_INCOMP    5

#define DIGEST_ERROR        6

/*
******************************************************************************
** Public functions - externed in the header file
******************************************************************************
*/
extern UINT8 (*csmtActions[18])(CONNECTION * pConn, UINT8 trEvt);
extern UINT8 iscsiCsmtInitConnState(CONNECTION * pConn);
extern void KernelDispatch(UINT32 returnCode, ILT * pILT, MR_PKT * pMRP, UINT32 w0);
extern INT32 convertTargetNameInTo64bit(UINT8 *string, INT32 size, UINT64 *pTargetName);
void        printBuffer(UINT8 *ptr, INT32 length);


void printBuffer(UINT8 *ptr, INT32 length)
{
    int         i;

    if (ptr == NULL)
    {
        return;
    }
    fprintf(stderr, "\nPRINTING BEGINS\n");
    for (i = 0; i < length; i = i + 4)
    {
        fprintf(stderr, "%d  ----- 0x%x\t0x%x\t0x%x\t0x%x\n", i / 4, ptr[i], ptr[i + 1],
                ptr[i + 2], ptr[i + 3]);
    }
    fprintf(stderr, "\nPRINTING FINISH\n");
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiHandleTpEvt
 **
 **              This is to handle events from transport layer
 **
 **  @param      pTPD      - Transport descriptro
 **  @param      Event     - Event
 **
 **  @return     XIO_CONN_CLOSE if connection closed, else XIO_SUCCESS on
 **              success or XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiHandleTpEvt(ISCSI_TPD * pTPD, UINT8 Event)
{
    bool        recvdPdu = FALSE;
    UINT8       opcode = 0;
    UINT8       result = XIO_ZERO;
    INT32       dataLen = XIO_ZERO;
    ISCSI_TPD  *pTPDNew = NULL;
    CONNECTION *pConn = NULL;
    SESSION    *pSession = NULL;
    INT32       positionInQ = -1;
    ISCSI_PDU   tmpPdu;
    ISCSI_PDU  *pPdu = NULL;
    INT32       seqNum = 0;

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Entry: iscsiHandleTpEvt()\n");
    /*
     * check if the received pTPD is null
     */

    if (pTPD == NULL)
    {
        fprintf(stderr, "pTPD passed is NULL\n");
        return XIO_FAILURE;
    }
    else
    {
        /*
         * Get connection from tpd
         */
        pConn = pTPD->pConn;
    }
    switch (Event)
    {
        case TSL_ISCSI_EVENT_ACCEPT:
            {
                INT32       tmp_addr;

                /*
                 * create a connection in idle state and have back ptr in pTPD
                 */
                if (pConn != NULL)
                {
                    fprintf(stderr, "ISCSI_DEBUG: BIG problem tpd %x conn %x\n",
                            (UINT32)pTPD, (UINT32)pConn);
                }
                iscsiConnInit(&pConn);
                /*
                 * start connection state machine
                 */
                iscsiCsmtInitConnState(pConn);
                tmp_addr = ISCSI_TSL_Event_Notify(pTPD, TSL_ISCSI_EVENT_ACCEPT_PROCESS, NULL, XIO_ZERO);
                pTPDNew = (ISCSI_TPD *)tmp_addr;
                if (pTPDNew == NULL)
                {
                    fprintf(stderr, "failed ISCSI_TSL_Event_Notify()%x "
                            "while accepting a connection\n", (UINT32)pTPDNew);
                    return XIO_FAILURE;
                }
                /*
                 * update the tp descriptor
                 */
                pConn->pTPD = pTPDNew;
                pTPDNew->pConn = pConn;
                pConn->pSession = NULL;
                /*
                 * Initialize connection params
                 */
                InitConnectionParams(pConn);
                /*
                 * got a valid connection update the CSM(conn state machine)
                 */
                result = csmtActions[CONNEVT_VALID_CONN] (pConn, CONNEVT_VALID_CONN);
                if (!result)
                {
                    pTPD->pConn->state = CONNST_FREE;
                    fprintf(stderr, "csmtActions() failed: when valid connection rcvd\n");
                }
            }
            /*
             * After accepting a connection we do not have anything to send so return from here
             */
            return result;

        case TSL_ISCSI_EVENT_READ:
            {
                if (pConn->recvState == IR_RECV_DATA ||
                    (pConn->recvState > IR_RECV_DATA && pConn->recvState < IR_COMP))
                {
                    /*
                     * need to recv remaining data as it could not be read previously, read and return
                     */
                    result = iscsiRecvData(pConn);
                    if (result == READ_DATA_INCOMP)
                    {
                        return XIO_SUCCESS;
                    }
                    if (result == XIO_FAILURE || pConn->state == CONNST_FREE)
                    {
                        recvdPdu = FALSE;
                        break;
                    }
                }
                /*
                 * check the state to receive
                 */
                switch (pConn->recvState)
                {
                    case IR_FREE:
                    case IR_BHS:
                        {
                            result = iscsiReadBHS(pConn);
                            if (result == READ_HDR_COMP)
                            {
                                recvdPdu = TRUE;
                            }
                            else if (result == READ_HDR_INCOMP)
                            {
                                return XIO_SUCCESS;
                            }
                            else if (result == XIO_FAILURE || pConn->state == CONNST_FREE
                                     || result == DIGEST_ERROR)
                            {
                                recvdPdu = FALSE;
                                pConn->state = CONNST_FREE;
                                break;
                            }
                            /*
                             * Read data if present
                             */
                            opcode = pConn->pHdr[0];

                            if (opcode != INIOP_SCSI_CMD && opcode != INIOP_SCSI_DATA_OUT)
                            {
                                /*
                                 * read the data portion if any
                                 * Do not allocate recv buffer for scsi cmds
                                 */

                                dataLen = iscsiIsDatasegPresent(pConn);
                                if (dataLen != XIO_ZERO)
                                {
                                    /*
                                     * Read data
                                     */
                                    result = iscsiReadData(pConn, dataLen);
                                    if (result == READ_DATA_COMP)
                                    {
                                        recvdPdu = TRUE;
                                    }
                                    else if (result == READ_DATA_INCOMP)
                                    {
                                        return XIO_SUCCESS;
                                    }
                                    if (result == XIO_FAILURE ||
                                        pConn->state == CONNST_FREE)
                                    {
                                        recvdPdu = FALSE;
                                        pConn->state = CONNST_FREE;
                                        break;
                                    }
                                    else if (result == DIGEST_ERROR)
                                    {
                                        /*
                                         * in case of data digest error we should send the reject pdu
                                         */
                                        iscsiBuildAndSendReject(pConn, REJECT_DIGEST_ERROR,
                                                                (ISCSI_GENERIC_HDR *)pConn->pHdr);
                                        recvdPdu = FALSE;
                                        break;
                                    }
                                }
                            }
                        }
                        break;

                    case IR_DATA:
                        {
                            /*
                             * need to read remaining bytes in the data seg
                             * check if we need to read ahs or data
                             */
                            result = iscsiRRData(pConn);
                            if (result == READ_DATA_COMP)
                            {
                                recvdPdu = TRUE;
                            }
                            else if (result == READ_DATA_INCOMP)
                            {
                                return XIO_SUCCESS;
                            }
                            else if (result == XIO_FAILURE || pConn->state == CONNST_FREE)
                            {
                                recvdPdu = FALSE;
                                pConn->state = CONNST_FREE;
                                break;
                            }
                            else if (result == DIGEST_ERROR)
                            {
                                /*
                                 * in case of data digest error we should send the reject pdu
                                 */
                                iscsiBuildAndSendReject(pConn, REJECT_DIGEST_ERROR,
                                                        (ISCSI_GENERIC_HDR *)pConn->pHdr);
                                recvdPdu = FALSE;
                                break;

                            }
                        }
                        break;

                    default:
                        {
//                            TRACE(TRACE_ISCSI_ERR, " Unknown Conn recv state pTPD= 0x%x pConn= 0x%x\n", (UINT32)pTPD, (UINT32)pConn);
                            pConn->state = CONNST_FREE;
                        }
                }
                /*
                 * done with reading the ahs and data, now process it
                 */
                if (recvdPdu == TRUE)
                {
                    ISCSI_GENERIC_HDR *pHdr = (ISCSI_GENERIC_HDR *)pConn->pHdr;

                    /*
                     * There are some cases in which reject will be sent for a command.
                     * In that case cmdSn should not be advanced TBD
                     */

                    /*
                     * Check for sequence number errors
                     */
                    seqNum = iscsiSeqChkCmdSn(pHdr, pTPD->pConn->pSession, 1);

                    tmpPdu.position = -1;
                    pPdu = &tmpPdu;
                    pPdu->ahs = pConn->ahs;
                    switch (seqNum)
                    {
                        case EXPECTED_CMD:
                            break;

                        case OUT_OF_ORDER_CMD:
                            {
                                /*
                                 * put the Command in queue. The command will be processed later
                                 */

                                fprintf(stderr, "OUT_OF_ORDER_CMD: Sn=(0x%x), opcode (0x%x)\n",
                                        bswap_32(pHdr->Sn), GET_ISCSI_OPCODE(pHdr->opcode));
                                printBuffer((UINT8 *)pHdr, 48);
                                positionInQ = AllocatePlaceInCommandQ(pConn->pSession, bswap_32(pHdr->Sn));
                                if (positionInQ == -1)
                                {
                                    fprintf(stderr, "No place to keep commands should never happen\n");
                                    break;
                                }
                                pPdu = &(pConn->pSession->commandQ.cmdArray[positionInQ].pdu);
                                pConn->pSession->commandQ.cmdArray[positionInQ].pConn = pTPD->pConn;
                                memcpy(&(pPdu->bhs), pHdr, 48);
                                if (pTPD->pConn->recvLen > 0)
                                {
                                    pConn->pSession->commandQ.cmdArray[positionInQ].data = s_MallocC(pTPD->pConn->recvLen, __FILE__, __LINE__);
                                    if (pConn->pSession->commandQ.cmdArray[positionInQ].data == NULL)
                                    {
                                        fprintf(stderr, "Malloc Failure closing the connection\n");
                                        pTPD->pConn->state = CONNST_FREE;
                                    }
                                    else
                                    {
                                        memcpy(pConn->pSession->commandQ.cmdArray[positionInQ].data,
                                               pTPD->pConn->recvBuff, pTPD->pConn->recvLen);
                                        /*
                                         * Where are you freeing this memory ?? For immediate cmd also
                                         * we need to do the same
                                         */
                                    }
                                }
                            }
                            result = XIO_SUCCESS;
                            break;

                        case OUT_OF_WINDOW_CMD:
                            /*
                             * discard the command
                             */
                            fprintf(stderr, "OUT_OF_WINDOW_CMD: Sn=(0x%x), opcode (0x%x) maxCmdSN (0x%x) expCmdSN (0x%x)\n",
                                    bswap_32(pHdr->Sn), GET_ISCSI_OPCODE(pHdr->opcode),
                                    pConn->pSession->maxCmdSN, pConn->pSession->expCmdSN);
                            printBuffer((UINT8 *)pHdr, 48);
                            result = XIO_SUCCESS;
                            break;

                        case IMMEDIATE_CMD:
                            /*
                             * send to execution engin if local cmdSn is same as pdu's
                             */
                            if (pTPD->pConn->pSession &&
                                pTPD->pConn->pSession->ssnState == SSN_LOGGED_IN)
                            {
                                pPdu = &(pConn->pSession->immediatePdu);
                                memcpy(&(pPdu->bhs), pHdr, 48);
                                pPdu->position = -1;
                            }
//                            else
//                            {
//                                /*
//                                ** It is the case when first PDU has come so there is no session
//                                */
//                            }
                            break;

                        case NO_CMD:
                            break;

                        default:
                            break;
                    }

                    /*
                     * result is 1 means we should process the PDU
                     */
                    if (seqNum == EXPECTED_CMD || seqNum == IMMEDIATE_CMD || seqNum == NO_CMD)
                    {
                        result = iscsiProcRecvdPdu((UINT8 *)pHdr, pTPD, pPdu);
                        if (!result)
                        {
                            pConn->state = CONNST_FREE;
                            break;
                        }
                        /*
                         * Make the buffer empty
                         */
                    }
                }
            }
            break;

        case TSL_ISCSI_EVENT_CLOSE_CONNECTION:
            {
                /*
                 * update the connection state to free
                 */
                pConn->state = CONNST_FREE;
            }
            break;

        default:
            {
                fprintf(stderr, "Invalid event received, closing connection\n");
                pConn->state = CONNST_FREE;
            }
    }

    if (result != XIO_SUCCESS)
    {
        if (pConn)
        {
            pConn->state = CONNST_FREE;
        }
    }

    /*
     * Create and start the nop timer
     */
    if (!pConn->timer_created &&
        pConn->state == CONNST_LOGGED_IN &&
        pConn->pSession->params.sessionType == NORMAL_SESSION && result == XIO_SUCCESS)
    {
        pConn->timerid = CreateTimer(NOPIN_TIMER_VALUE, (UINT32)pTPD, (UINT32)NULL, iscsiSendNopin);
        StartTimer(pConn->timerid);
        pConn->timer_created = TRUE;
    }

    /*
     * process all pdu which are queued and could not be proccessed because of cmdSn gap
     */
    if ((pTPD->pConn) && (result == XIO_SUCCESS) && (pConn->state != CONNST_FREE))
    {
        pSession = pTPD->pConn->pSession;
        while (pSession && pSession->cmdSN != pSession->expCmdSN &&
               pSession->cmdSN < pSession->expCmdSN - 1)
        {
            fprintf(stderr, "processing pdus which were out of order and queued local cmdSN = 0x%x expCmdSN= 0x%x\n",
                    pSession->cmdSN, pSession->expCmdSN);
            positionInQ = getPositionInCommandQForCmdSN(pSession, pSession->cmdSN);
            if (positionInQ == -1)
            {
                /*
                 * This means there is a gap, we have not received this command yet
                 */
                break;
            }
            /*
             * Check if there is any immediate command for this cmdSN
             */
            //processImmediateCmdForCmdSn(pSession, pSession->cmdSN);
            /*
             * now process non-immediate command
             */
            pPdu = &(pSession->commandQ.cmdArray[positionInQ].pdu);
            result = iscsiProcRecvdPdu((UINT8 *)&(pPdu->bhs),
                                       pSession->commandQ.cmdArray[positionInQ].pConn->
                                       pTPD, pPdu);
            if (!result)
            {
                pTPD->pConn->state = CONNST_FREE;
                break;
            }
        }
    }

    /*
     * check if the session & connection states are FREE which
     * means, we close corresponding session & connection
     */
    if ((pConn) && (pConn->state == CONNST_FREE))
    {
        iscsiCloseConn(pConn);
        result = XIO_CONN_CLOSE;
    }
    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Exit: iscsiHandleTpEvt()\n");
    return result;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiCloseConn
 **
 **              This is for closing connection
 **
 **  @param      pConn  - connection pointer
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiCloseConn(CONNECTION * pConn)
{
    SESSION    *pSession = NULL;

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Entry: iscsiCloseConn()\n");

    /*
     * Free the connection ILTs and connection structure
     */
    iscsi_cleanupILT(pConn, ISCSI_MAX_LUN);
    tsl_CloseConnection(pConn->pTPD);

    /*
     * release the chap context cc, if we have already allocated and not freed
     */
    chap_release_context_st(pConn->cc);

    if (pConn->timer_created)
    {
        DeleteTimer(pConn->timerid);
    }

    /*
     * check if the head count is one which means only one connection
     */
    if (pConn->pSession == NULL)
    {
        /*
         * This is a possibility when a session is not created because of login failure
         */
        s_Free(pConn, sizeof(CONNECTION), __FILE__, __LINE__);
        return XIO_SUCCESS;
    }

    {
        char        tmp_buff[300] = { 0 };
        strcpy(tmp_buff, "Connection Close for ");
        sprintf(tmp_buff + strlen(tmp_buff), "%s", pConn->pSession->params.initiatorName);
        iscsi_dprintf(tmp_buff);
    }

    pSession = pConn->pSession;
    pConn->pSession = NULL;
    s_Free(pConn, sizeof(CONNECTION), __FILE__, __LINE__);
    pSession->pCONN = NULL;

    pSession->ssnState = SSN_FREE;
    iscsiCloseSession(pSession);

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Exit: iscsiCloseConn()\n");
    return XIO_SUCCESS;
}

INT32 iscsiCloseSession(SESSION * pSession)
{
    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Entry: iscsiCloseSession()");

    /*
     * find out if there is any connection
     * Remove and free it
     */
    if (pSession->pCONN != NULL)
    {
        /*
         * socket is not closed for this connection so close that.
         * For session reinstatement, we must terminate all tasks
         */
        pSession->ssnState = SSN_FREE;
        iscsi_cleanupILT(pSession->pCONN, ISCSI_MAX_LUN);
        tsl_CloseConnection(pSession->pCONN->pTPD);
        /*
         * release the chap context cc, if we have already allocated and not freed
         */
        chap_release_context_st(pSession->pCONN->cc);

        if (pSession->pCONN->timer_created)
        {
            DeleteTimer(pSession->pCONN->timerid);
        }
        /*
         * check the next connection; release current connection
         */
        s_Free(pSession->pCONN, sizeof(CONNECTION), __FILE__, __LINE__);
    }
    /*
     * inform the Proc for logout
     */
    if (pSession->params.sessionType == NORMAL_SESSION && pSession->ssnState != SSN_ACTIVE)
    {
        UINT64      wwn = 0;

        if (pSession->tsih != 0)
        {
            /*
             * if the initiator name starts with eui or naa then get the wwn
             * from the initiator name and send it to SrvLogout otherwise
             * send sid from pSession
             */
            if ((strncmp((char *)pSession->params.initiatorName, "eui", 3) == 0) ||
                (strncmp((char *)pSession->params.initiatorName, "naa", 3) == 0))
            {
                convertTargetNameInTo64bit(pSession->params.initiatorName, 20, &wwn);
                fsl_SrvLogout(pSession->tid, pSession->tsih, wwn);
            }
            else
            {
                fsl_SrvLogout(pSession->tid, pSession->tsih, pSession->sid);
            }
        }
    }

    /*
     * this session node need to be removed from the list too
     */
    iscsiSsnRemove(pSession);

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Exit: iscsiCloseSession()\n");
    return XIO_SUCCESS;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiReadBHS
 **
 **              This is for reading BHS portion of the received PDU
 **
 **  @param      pConn  - connection pointer
 **  @param      pHdr   - geniric header pointer
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/

UINT8 iscsiReadBHS(CONNECTION * pConn)
{
    INT32       totalToRead = 0;
    INT32       ahsLength = 0;
    INT32       count = 0;
    UINT8      *tmp = NULL;
    UINT8       opcode = 0;

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Entry: iscsiReadBHS()");
    if (pConn->recvState == IR_FREE)
    {
        /*
         * this is first call so initialize every thing
         */

        pConn->recvLen = ISCSI_HDR_LEN;
        totalToRead = ISCSI_HDR_LEN;
        pConn->offset = 0;
    }

    /*
     * need to recv first 48 byte hdr
     */
    if (pConn->offset < ISCSI_HDR_LEN)
    {
        /*
         * we haven't been able to read 48 byte
         */
        tmp = pConn->pHdr + pConn->offset;
        totalToRead = ISCSI_HDR_LEN - pConn->offset;
        count = tsl_recv_data(pConn->pTPD, (char *)tmp, totalToRead);
        if (count < 0)
        {
            pConn->state = CONNST_FREE;
            pConn->recvState = IR_FREE;
            return (XIO_FAILURE);
        }
        pConn->offset += count;
        if (count < totalToRead)
        {
            pConn->recvState = IR_BHS;
            return (READ_HDR_INCOMP);
        }
    }
    opcode = GET_ISCSI_OPCODE(pConn->pHdr[0]);

    /*
     * at this point we must have read the initial 48 bytes
     */
    ahsLength = (((bswap_32(((ISCSI_GENERIC_HDR *)pConn->pHdr)->ahsDataLen) & AHS_LEN_MASK) >> 24) & 0xFF) * 4;

    /*
     * a non-zero AHS length typically means we've gotten confused about PDU
     * boundaries, because nobody uses additional header segments yet.
     */
    if (ahsLength != 0)
    {
        fprintf(stderr, "!!!iscsiReadBHS ahs %d 0x%x pconn %p offset %d hdr %p raw ahs/dl 0x%x\n",
                ahsLength, ahsLength, pConn, pConn->offset, pConn->pHdr,
                (((ISCSI_GENERIC_HDR *)pConn->pHdr)->ahsDataLen));
    }
    totalToRead = 0;
    count = 0;
    if (ahsLength > 0)
    {
        if (pConn->offset == ISCSI_HDR_LEN)
        {
            /*
             * first time so allocate memory for ahs
             */
            pConn->ahs = s_MallocC(ahsLength, __FILE__, __LINE__);
            if (pConn->ahs == NULL)
            {
                return (XIO_FAILURE);
            }
            totalToRead = ahsLength;
            tmp = pConn->ahs;
        }
        else if (pConn->offset < ISCSI_HDR_LEN + ahsLength)
        {
            /*
             * we have read incomplete ahs
             */
            totalToRead = ahsLength - (pConn->offset - ISCSI_HDR_LEN);
            tmp = pConn->ahs + pConn->offset - ISCSI_HDR_LEN;
        }

        if (totalToRead)
        {
            count = tsl_recv_data(pConn->pTPD, (char *)tmp, totalToRead);
        }
        if (count < 0)
        {
            pConn->state = CONNST_FREE;
            pConn->recvState = IR_FREE;
            return (XIO_FAILURE);
        }

        pConn->offset += count;
        if (count < totalToRead)
        {
            pConn->recvState = IR_BHS;
            return (READ_HDR_INCOMP);
        }
    }
    /*
     *  we should read here digest
     */
    if (isDigestCheckValid(pConn->pTPD) == XIO_SUCCESS &&
        stringCompare(pConn->params.headerDigest.strval, (UINT8 *)"CRC32C") == 0 &&
        opcode != INIOP_LOGIN_REQ)
    {
        unsigned long headerDigest = 0xabababab;

        totalToRead = ISCSI_HDR_LEN + ahsLength + DIGEST_LENGTH - pConn->offset;

        tmp = ((ISCSI_GENERIC_HDR *)pConn->pHdr)->headerDigest + DIGEST_LENGTH - totalToRead;

        if (totalToRead <= 0)
        {
            fprintf(stderr, "!!!iscsiReadBHS invalid total %d ahs %d %x pconn %p offset %d hdr %p tmp %p\n",
                    totalToRead, ahsLength, ahsLength, pConn, pConn->offset, pConn->pHdr, tmp);
        }

        count = tsl_recv_data(pConn->pTPD, (char *)tmp, totalToRead);
        if (count < 0)
        {
            pConn->state = CONNST_FREE;
            pConn->recvState = IR_FREE;
            return (XIO_FAILURE);
        }

        if (count < totalToRead)
        {
            pConn->recvState = IR_BHS;
            pConn->offset += count;
            return (READ_HDR_INCOMP);
        }
        else if (count == totalToRead)
        {
            memcpy(&headerDigest, ((ISCSI_GENERIC_HDR *)pConn->pHdr)->headerDigest, DIGEST_LENGTH);
            if (iscsi_isDigestError(pConn->pHdr, ISCSI_HDR_LEN, headerDigest) == 1)
            {
                fprintf(stderr, "ERROR: Header Digest Error\n");
                printBuffer(pConn->pHdr, 52);
                pConn->recvState = IR_FREE;
                return (DIGEST_ERROR);
            }
        }
    }
    pConn->recvState = IR_FREE;
    pConn->offset = 0;
    pConn->recvLen = 0;
    return (READ_HDR_COMP);
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiRRBHS
 **
 **              This is for reading remaining BHS portion
 **
 **  @param      pConn  - connection pointer
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiRRBHS(CONNECTION * pConn)
{
    INT32       readLen = XIO_ZERO;
    UINT8       result = 0;

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Entry: iscsiRRBHS()");

    readLen = tsl_recv_data(pConn->pTPD, (char *)(pConn->pHdr + pConn->offset),
                            (pConn->recvLen - pConn->offset));
    if (readLen == (pConn->recvLen - pConn->offset))
    {
        /*
         * check digest here
         */
        if (pConn->recvLen > ISCSI_HDR_LEN)
        {
            unsigned long headerDigest;

            memcpy(&headerDigest, ((ISCSI_GENERIC_HDR *)(pConn->pHdr))->headerDigest, DIGEST_LENGTH);
            if (iscsi_isDigestError(pConn->pHdr, ISCSI_HDR_LEN, headerDigest) == 1)
            {
                fprintf(stderr, "ERROR: Header Digest Error\n");
                return DIGEST_ERROR;
            }
        }
        pConn->recvLen = XIO_ZERO;
        pConn->recvState = IR_FREE;
        result = READ_HDR_COMP;
    }
    else if (readLen == -1)
    {
        /*
         * close the connection
         */
        pConn->state = CONNST_FREE;
        pConn->recvState = IR_FREE;
        result = XIO_FAILURE;
    }
    else
    {
        /*
         * recvd bytes are less than expected
         */
        pConn->offset += readLen;
        pConn->recvState = IR_BHS;
        result = READ_HDR_INCOMP;
    }
    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Exit: iscsiRRBHS()");
    return result;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiReadData
 **
 **              This is for reading data portion of the PDU
 **
 **  @param      pConn  - connection pointer
 **  @param      length - length
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiReadData(CONNECTION * pConn, INT32 length)
{
    bool        isDigest = FALSE;
    UINT8       result = 0;
    INT32       readLen = XIO_ZERO;
    INT32       pad = (-length) & 3;
    INT32       totalToRead;
    UINT8       opcode = 0;

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Entry: iscsiReadData()");

    length = length + pad;
    totalToRead = length;
    opcode = GET_ISCSI_OPCODE(pConn->pHdr[0]);

    /*
     * check for the digest
     */
    if (isDigestCheckValid(pConn->pTPD) == XIO_SUCCESS &&
        stringCompare(pConn->params.dataDigest.strval, (UINT8 *)"CRC32C") == 0 &&
        opcode != INIOP_LOGIN_REQ)
    {
        totalToRead += DIGEST_LENGTH;
        isDigest = TRUE;
    }
    pConn->recvLen = totalToRead;
    pConn->offset = 0;

    /*
     * read data into recv buff
     */
    readLen = tsl_recv_data(pConn->pTPD, (char *)pConn->recvBuff, totalToRead);
    if (readLen == pConn->recvLen)
    {
        /*
         * update the length
         */
        if (isDigest == TRUE)
        {
            pConn->recvLen -= DIGEST_LENGTH;
        }
        pConn->recvLen -= pad;
        pConn->recvState = IR_FREE;
        result = READ_DATA_COMP;
        if (totalToRead > length)
        {
            unsigned long dataDigest;

            memcpy(&dataDigest, pConn->recvBuff + totalToRead - DIGEST_LENGTH, DIGEST_LENGTH);
            if (iscsi_isDigestError((UINT8 *)pConn->recvBuff, pConn->recvLen, dataDigest) == 1)
            {
                fprintf(stderr, "ERROR: Data Digest Error\n");
                result = DIGEST_ERROR;
            }
        }
    }
    else if (readLen == -1)
    {
        /*
         * close connection
         */
        pConn->state = CONNST_FREE;
        result = XIO_FAILURE;
    }
    else
    {
        /*
         * recvd bytes are less than required
         */
        pConn->offset += readLen;
        pConn->recvState = IR_DATA;
        result = READ_DATA_INCOMP;
    }
    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Exit: iscsiReadData()");
    return result;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiRRData
 **
 **              This is for reading remaining data portion of the PDU
 **
 **  @param      pConn  - connection pointer
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiRRData(CONNECTION * pConn)
{
    INT32       readLen = XIO_ZERO;
    INT32       totalToRead = 0;
    UINT8       result = 0;
    INT32       pad = 0;
    UINT8       opcode = 0;

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Entry: iscsiRRData()");

    opcode = GET_ISCSI_OPCODE(pConn->pHdr[0]);
    totalToRead = pConn->recvLen - pConn->offset;

    /*
     * check for the recvBuff
     */
    if (pConn->recvBuff == NULL)
    {
        return XIO_FAILURE;
    }

    /*
     * read data into recv buff
     */
    pad = (-(pConn->recvLen) & 3);

    readLen = tsl_recv_data(pConn->pTPD, (char *)(pConn->recvBuff + pConn->offset), totalToRead);
    if (readLen == totalToRead)
    {
        pConn->recvState = IR_FREE;
        result = READ_DATA_COMP;

        /*
         * data digest check
         */
        if (isDigestCheckValid(pConn->pTPD) == XIO_SUCCESS &&
            stringCompare(pConn->params.dataDigest.strval, (UINT8 *)"CRC32C") == 0 &&
            opcode != INIOP_LOGIN_REQ)
        {
            /*
             * This means we have read data as well as digest also
             */
            unsigned long dataDigest;

            memcpy(&dataDigest, pConn->recvBuff + pConn->recvLen - DIGEST_LENGTH, DIGEST_LENGTH);
            pConn->recvLen -= DIGEST_LENGTH;
            if (iscsi_isDigestError((UINT8 *)pConn->recvBuff, pConn->recvLen, dataDigest) == 1)
            {
                fprintf(stderr, "ERROR: Data Digest Error\n");
                result = DIGEST_ERROR;
            }
        }
        pConn->recvLen -= pad;
    }
    else if (readLen == -1)
    {
        /*
         * close connection
         */
        pConn->state = CONNST_FREE;
        result = XIO_FAILURE;
    }
    else
    {
        /*
         * recvd bytes are less than required
         */
        pConn->offset += readLen;
        pConn->recvState = IR_DATA;
        return READ_DATA_INCOMP;
    }

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Exit: iscsiRRData()");
    return result;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiRecvData
 **
 **              This is for retrying to receive data in case of failure
 **
 **  @param      pConn  -  Pointer to connection
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiRecvData(CONNECTION * pConn)
{
    UINT8       result = 0;
    ILT        *pSecILT = NULL;
    ILT        *r2tILT = NULL;
    ILT        *pILT = NULL;
    ILT        *pPriILT = NULL;
    ILT        *pXLILT = NULL;
    ILT        *piSCSI_ILT = NULL;
    INT32       tmpvalue = 0;
    ISCSI_PDU  *pPdu = NULL;
    ISCSI_SCSI_CMD_HDR *pCmd = NULL;
    SGL_DESC   *pSglDesc = NULL;
    bool        isLastDataOut = FALSE;
    ISCSI_DATA_OUT_HDR *pHdr = (ISCSI_DATA_OUT_HDR *)(pConn->pHdr);
    INT32       length;

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Entry: iscsiRecvData()\n");

    r2tILT = hash_find(pConn->hashTable, pConn->itt, pConn->ttt);
    if (r2tILT == NULL)
    {
        fprintf(stderr, "r2tILT stored is NULL in hashtable, for itt = %x, ttt %x\n",
                (UINT32)pConn->itt, (UINT32)pConn->ttt);
        return XIO_FAILURE;
    }
    pSecILT = (ILT *)r2tILT->secondary_ilt.secILT;
    if (!pSecILT)
    {
        /*
         * ILT is null
         */
        fprintf(stderr, "secILT stored in connection is NULL\n");
        return XIO_SUCCESS;
    }
    /*
     * Get ILT and XLILT
     */
    pILT = (ILT *)pSecILT->misc;
    pXLILT = pILT;

    /*
     * Get primary ILT and iSCSI ILT
     */
    pPriILT = (ILT *)(((ILT *)(pSecILT->misc))->ilt_normal.w5);
    piSCSI_ILT = ((ILT *)(pPriILT->misc));

    /*
     * do byte swap
     */

    /*
     * extract pdu and connection from ilt
     */
    pPdu = piSCSI_ILT->iscsi_def.pPdu;
    pCmd = (ISCSI_SCSI_CMD_HDR *) & pPdu->bhs;

    /*
     * get the sgl descriptor, pdu and command
     */
    pSglDesc = pXLILT->fc_xl.xl_pSGL;
    /*
     * in current implementation out of order pdu is not supported
     */
    if (r2tILT->secondary_ilt.dataTxLen ==
              (r2tILT->secondary_ilt.r2toffset - pCmd->expStatSn) -
              (r2tILT->secondary_ilt.r2tCount - pCmd->rsvd1) *
              (pConn->pSession->params.maxBurstLength) + pHdr->ahsDataLen)
    {
        isLastDataOut = TRUE;
    }
    if (pHdr->buffOffset != r2tILT->secondary_ilt.r2toffset)
    {
        /*
         * this should not happen if all pdu's come in order
         */
        fprintf(stderr, "mismatch !! buffer offset=(0x%x) r2toffset=(0x%x) itt=(0x%x)\n",
                pHdr->buffOffset, r2tILT->secondary_ilt.r2toffset, pHdr->itt);
        return XIO_FAILURE;
    }

    length = pConn->recvLen - pConn->offset;
    tmpvalue = tsl_recv_dataOut(pConn->pTPD, pSglDesc, length, pXLILT->fc_xl.xl_sgllen,
                                (r2tILT->secondary_ilt.r2toffset - pCmd->expStatSn));
    if (tmpvalue == -1)
    {
        KernelDispatch(1, pILT, NULL, 0);
        fprintf(stderr, "tsl_recv_data failed, closing connection\n");
        result = hash_delete(pConn->hashTable, r2tILT->secondary_ilt.ITT, r2tILT->secondary_ilt.TTT);
        if (result == XIO_FAILURE)
        {
            fprintf(stderr, "hash_delete() failed\n");
        }
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)r2tILT);
#endif /* M4_DEBUG_ILT */
        put_ilt(r2tILT);
        pConn->state = CONNST_FREE;
        result = XIO_FAILURE;
        return result;
    }
    if (pConn->recvState == IR_COMP)
    {
        return iscsiProcScsiData(pConn, pSglDesc, pHdr, pILT, r2tILT, pCmd, isLastDataOut,
                                 pXLILT->fc_xl.xl_sgllen);
    }

    result = READ_DATA_INCOMP;
    return result;
}

INT32 iscsiIsAhsPresent(CONNECTION * pConn)
{
    return (bswap_32(((ISCSI_GENERIC_HDR *)pConn->pHdr)->ahsDataLen) & AHS_LEN_MASK);
}

INT32 iscsiIsDatasegPresent(CONNECTION * pConn)
{
    return (bswap_32(((ISCSI_GENERIC_HDR *)pConn->pHdr)->ahsDataLen) & DATA_LEN_MASK);
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiSendNopin
 **
 **             Sends nopins to the Initiator at the specified
 **             periodic interval
 **
 **  @param      data   -  Pointer to TPD structure
 **  @param      a      -  A UINT32 value(currently not used)
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
int iscsiSendNopin(UINT32 data, UINT32 a UNUSED)
{
    INT8        counter;
    ISCSI_TPD  *pTPD = (ISCSI_TPD *)data;
    ISCSI_NOPIN_HDR pNopin_hdr;
    ISCSI_NOPOUT_HDR pNopout_hdr;

    CONNECTION *pConn = pTPD->pConn;

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Entry: iscsiSendNopin()\n");

    /*
     * check for the connection
     */
    if (pConn == NULL)
    {
        return XIO_FAILURE;
    }

    memset(&pNopin_hdr, XIO_ZERO, ISCSI_HDR_LEN);
    memset(&pNopout_hdr, XIO_ZERO, ISCSI_HDR_LEN);
    if (pConn->outstanding_nopins < MAX_OUTSTANDING_NOPINS)
    {
        /*
         * build nop in hdr
         */
        iscsiBuildNopInHdr(&pNopout_hdr, &pNopin_hdr, ISCSI_TARG_NOPIN_REQ);
        pConn->outstanding_nopins++;
        for (counter = 0; counter < MAX_OUTSTANDING_NOPINS; counter++)
        {
            pConn->nopin_ttt[counter] = pNopin_hdr.ttt;
            break;
        }

        pNopin_hdr.statSn = bswap_32(pConn->statSN);
        pNopin_hdr.expCmdSn = bswap_32(pConn->pSession->expCmdSN);
        pNopin_hdr.maxCmdSn = bswap_32(pConn->pSession->maxCmdSN);
        pNopin_hdr.ahsDataLen = bswap_32(pNopin_hdr.ahsDataLen);
        pNopin_hdr.lunOrRsvd = bswap_64(pNopin_hdr.lunOrRsvd);
        pNopin_hdr.initTaskTag = bswap_32(pNopin_hdr.initTaskTag);
        pNopin_hdr.ttt = bswap_32(pNopin_hdr.ttt);

        iscsi_updateSend(pConn, NULL, (char *)&pNopin_hdr, ISCSI_HDR_LEN, NULL, 0, 0,
                         iscsi_crRelILT, ISCSI_MAX_LUN);
    }                           /* end if MAX_OUTSTANDING_NOPINS */
    else
    {
        /*
         * tear down the connection
         */
        fprintf(stderr, "Connection timeout: deleting the timer for tid = %d initiatorName=%s\n",
                pConn->pTPD->tid, pConn->pSession->params.initiatorName);
        iscsiCloseConn(pConn);
        return XIO_FAILURE;
    }
    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Exit: iscsiSendNopin()\n");
    return XIO_SUCCESS;
}                               /* end func iscsiSendNopin */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
