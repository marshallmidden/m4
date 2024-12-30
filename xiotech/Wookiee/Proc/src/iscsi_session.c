/* $Id: iscsi_session.c 159663 2012-08-22 15:36:42Z marshall_midden $ */
/**
 ******************************************************************************
 **
 **  @file       iscsi_session.c
 **
 **  @brief      iSCSI session/connection related functions
 **
 **  This provides API's for session and connection related state machines
 **  and initialization of params
 **
 **  Copyright (c) 2005-2010 XIOtech Corporation.  All rights reserved.
 **
 ******************************************************************************
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>

#include <netinet/in.h>
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"

#include "iscsi_common.h"
#include "iscsi_pdu.h"
#include "iscsi_tsl.h"
#include "iscsi_digest.h"
#include "fsl.h"
#include "iscsi_timer.h"
#include "iscsi_tmf.h"

#include "pm.h"
#include "ilt.h"
#include "sgl.h"
#include "mem_pool.h"
#include "CT_defines.h"

/*
******************************************************************************
** Private functions
******************************************************************************
*/

extern INT32 convertTargetNameInTo64bit(UINT8 *string, INT32 size, UINT64 *pTargetName);
int         serial_bit_compare(UINT32 s1, UINT32 s2);
bool        IscmdsnInWindow(UINT32 cmdSN, UINT32 maxCmdSN, UINT32 expCmdSN);
UINT8       iscsiCsmtInitConnState(CONNECTION *pConn);
UINT8       iscsiCsmtIllegal(CONNECTION *pConn, UINT8 trEvt);
UINT8       iscsiCsmtRcvdValidConn(CONNECTION *pConn, UINT8 trEvt);
UINT8       iscsiCsmtInitLoginReqRecvd(CONNECTION *pConn, UINT8 trEvt);
UINT8       iscsiCsmtFinLoginReqRecvd(CONNECTION *pConn, UINT8 trEvt);
UINT8       iscsiCsmtLoginTimedOut(CONNECTION *pConn, UINT8 trEvt);
UINT8       iscsiCsmtConnCloseEvts(CONNECTION *pConn, UINT8 trEvt);
UINT8       iscsiCsmtConnClean(CONNECTION *pConn, UINT8 trEvt);
UINT8       iscsiCsmtLogoutRecvd(CONNECTION *pConn, UINT8 trEvt);
UINT8       iscsiCsmtLogoutRecvd1(CONNECTION *pConn, UINT8 trEvt);
UINT8       iscsiCsmtAsyncEvt2Send(CONNECTION *pConn, UINT8 trEvt);
UINT8       iscsiCsmtAsyncEvt2Send1(CONNECTION *pConn, UINT8 trEvt);
UINT8       iscsiCsmtTpConnClose(CONNECTION *pConn, UINT8 trEvt);
UINT8       iscsiCsmtTpEvts(CONNECTION *pConn, UINT8 trEvt);
UINT8       iscsiCsmtTpEvts1(CONNECTION *pConn, UINT8 trEvt);
UINT8       iscsiCsmtLogoutProcFailure(CONNECTION *pConn, UINT8 trEvt);
UINT8       iscsiCsmtLogoutRespSucces(CONNECTION *pConn, UINT8 trEvt);

UINT8       (*csmtActions[18]) (CONNECTION *pConn, UINT8 trEvt) =
{
    iscsiCsmtIllegal,
    iscsiCsmtIllegal,
    iscsiCsmtRcvdValidConn,
    iscsiCsmtInitLoginReqRecvd,
    iscsiCsmtFinLoginReqRecvd,
    iscsiCsmtLoginTimedOut,
    iscsiCsmtConnCloseEvts,
    iscsiCsmtConnClean,
    iscsiCsmtLogoutRecvd,
    iscsiCsmtLogoutRecvd1,
    iscsiCsmtAsyncEvt2Send,
    iscsiCsmtAsyncEvt2Send1,
    iscsiCsmtTpConnClose,
    iscsiCsmtIllegal,
    iscsiCsmtTpEvts,
    iscsiCsmtTpEvts1,
    iscsiCsmtLogoutProcFailure,
    iscsiCsmtLogoutRespSucces
};

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      iscsiSsnInit
**
**              Allocate for session and initialize default values
**
**  @param      pSession - Session double pointer
**
**  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
**
******************************************************************************
**/
UINT8 iscsiSsnInit(SESSION **pSession)
{

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Entry: iscsiSsnInit()\n");

    /* Allocate and memset session */
    (*pSession) = (SESSION *) s_MallocC(sizeof(SESSION), __FILE__, __LINE__);

    if (*pSession == NULL)
    {
        fprintf(stderr, "Malloc Failed\n");
        return XIO_FAILURE;
    }
    memset((*pSession), XIO_ZERO, sizeof(SESSION));
//    (*pSession)->tid = 0xffff; /* NO_PORTID */
    InitializeCommandQueue(*pSession);

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Exit: iscsiSsnInit()\n");
    return XIO_SUCCESS;

}

/**
 ******************************************************************************
 **
 **  @brief      iscsiNormalSsnMatchISID
 **
 **              match for isid in the session list
 **
 **  @param      isid      - ISID
 **  @param      tid       - Target Id
 **
 **  @return    SESSION pointer on success NULL otherwise
 **
 ******************************************************************************
 **/
SESSION    *iscsiNormalSsnMatchISID(UINT64 sid, UINT16 tid, UINT8 *initName)
{
    TAR        *pTar = NULL;
    ISCSI_SID   ssnSid;
    ISCSI_SID   LoginReqSid;
    SESSION    *pSsnNode = NULL;
    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Entry: iscsiNormalSsnMatchISID()\n");

    /* Mask for TSIH in sid received */
    LoginReqSid.sid = sid;
    LoginReqSid.tsih = 0;

    /* Get the tar corresponding to tid */
    pTar = fsl_get_tar(tid);
    if (pTar == NULL)
    {
        fprintf(stderr, "iscsiNormalSsnMatchISID() pTar is null for tid(%d)\n", tid);
        return NULL;
    }

    /* search for the ISID through the list */
    for (pSsnNode = (SESSION *)pTar->portID; pSsnNode != NULL; pSsnNode = pSsnNode->pNext)
    {
        /* Compare the isid part */
        ssnSid.sid = pSsnNode->sid;
        ssnSid.tsih = 0;
        if (LoginReqSid.sid == ssnSid.sid &&
            strcmp((char *)initName, (char *)pSsnNode->params.initiatorName) == 0 &&
            pSsnNode->params.sessionType == NORMAL_SESSION)
        {
            return pSsnNode;
        }
    }
    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Exit: iscsiNormalSsnMatchISID()\n");
    return NULL;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiSsnMatchTSIH
 **
 **              match for TSIH in the session list
 **
 **  @param      tsih      - TSIH
 **  @param      tid       - Target Id
 **
 **  @return    SESSION pointer on success NULL otherwise
 **
 ******************************************************************************
 **/
SESSION    *iscsiSsnMatchTSIH(UINT16 tsih, UINT16 tid)
{
    SESSION    *pSsnNode = NULL;
    TAR        *pTar = NULL;

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Entry: iscsiSsnMatchTSIH()\n");
    pTar = fsl_get_tar(tid);
    if (pTar == NULL)
    {
        fprintf(stderr, "iscsiSsnMatchTSIH() pTar is null for tid(%d)\n", tid);
        return NULL;
    }

    /* Match the tsih in the existing session list */
    for (pSsnNode = (SESSION *) pTar->portID; pSsnNode != NULL; pSsnNode = pSsnNode->pNext)
    {
        /* match for the TSIH and return session node on success */
        if (pSsnNode->tsih == tsih)
        {
            return pSsnNode;
        }
    }
    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Exit: iscsiSsnMatchTSIH()\n");
    return NULL;
}

/**
 ******************************************************************************
 **
 **  @brief      iSCSI_GetISID
 **
 **              Get the iSCSI Initiator isid for a given tid and tsih
 **
 **  @param      tid       - Target Id
 **  @param      tsih      - tsih
 **
 **  @return    UINT64 - isid is returned
 **
 ******************************************************************************
 **/
UINT64 iSCSI_GetISID(UINT16 tid, UINT16 tsih)
{
    SESSION    *pSsn = NULL;
    UINT64      wwn = 0;

    /*
      * Get the tar and go through session list
      * match for tsih and return isid else zero
      */
    pSsn = iscsiSsnMatchTSIH(tsih, tid);
    if (pSsn == NULL)
    {
        fprintf(stderr, "iscsi_GetISID(), session not found\n");
        return 0;
    }
    if (strncmp((char *)pSsn->params.initiatorName, "eui", 3) == 0 ||
        strncmp((char *)pSsn->params.initiatorName, "naa", 3) == 0)
    {
        convertTargetNameInTo64bit(pSsn->params.initiatorName, 20, &wwn);
        //    return(fsl_a2x(pSsn->params.initiatorName+4));
        return wwn;
    }
    return (pSsn->sid);
}                               /* iSCSI_GetISID */

/**
 ******************************************************************************
 **
 **  @brief      iscsiGetSrvName
 **
 **              Get the iSCSI Initiator for a given tid and isid
 **
 **  @param      isid      - ISID
 **  @param      tid       - Target Id
 **
 **  @return    SESSION pointer on success NULL otherwise
 **
 ******************************************************************************
 **/
UINT8      *iSCSI_GetSrvName(UINT16 tsih, UINT16 tid)
{
    SESSION    *pSsnNode = NULL;
    TAR        *pTar = NULL;

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Entry: iscsiGetSrvName()\n");
    pTar = fsl_get_tar(tid);
    if (pTar == NULL)
    {
        fprintf(stderr, "iSCSI_GetSrvName() pTar is null for tid(%d)\n", tid);
        return NULL;
    }

    /* Match the isid in the existing session list */
    for (pSsnNode = (SESSION *) pTar->portID; pSsnNode != NULL; pSsnNode = pSsnNode->pNext)
    {
        /* Get the session and compare isid */
        if (pSsnNode->tsih == tsih)
        {
            return (pSsnNode->params.initiatorName);
        }
    }
    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Exit: iscsiGetSrvName()\n");
    fprintf(stderr, "iSCSI_GetSrvName() returning NULL tid(%d) tsih (%d)\n", tid, tsih);
    return NULL;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiSsnGetCurrState
 **
 **              Returns current state of session
 **
 **  @param      SESSION   - session pointer
 **
 **  @return    session state
 **
 ******************************************************************************
 **/
UINT8 iscsiSsnGetCurrState(SESSION *pSsn)
{
    return pSsn->ssnState;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiSsnRemove
 **
 **  @return    none
 **
 ******************************************************************************
 **/
void iscsiSsnRemove(SESSION *pSsn)
{
    TAR        *pTar = NULL;

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Entry: iscsiSsnRemove()\n");

    /* Get the ssn head node */
    pTar = fsl_get_tar(pSsn->tid);
    if (pTar == NULL)
    {
        fprintf(stderr, "fsl_get_tar() pTar is null for tid(%d)\n", pSsn->tid);
        return;
    }
    if ((iscsi_del_ssn(&pTar->portID, pSsn)) != XIO_SUCCESS)
    {
        fprintf(stderr, "ISCSI_DEBUG: BIG PROBLEM !!!! Why is session del failed ssn (%x) tid (%d)\n",
                (UINT32)pSsn, pSsn->tid);
    }
    pTar->ssn_cnt -= 1;
    if (pTar->ssn_cnt == 0 || pTar->portID == 0)
    {
        /* This was the last session on this */
        pTar->ssn_cnt = 0;
        pTar->portID = 0;
    }
    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Exit: iscsiSsnRemove()\n");
}

#define TWO_RAISE_TO_31_MIN_1 0x7fffffff
#define SERIAL_EQUAL 0
#define SERIAL_LESS_THAN 1
#define SERIAL_GREATER_THAN 2
#define SERIAL_UNDEFINED 3

/*
******************************************************************
**
** @name serial_bit_compare
** @brief serial bit comparison according to RFC 1982
**
** @param UINT32 s1
** @param UINT32 s2
**
** @return EQUAL, LESS_THAN, GREATER_THAN, UNDEFINED
**
**
******************************************************************
*/
int serial_bit_compare(UINT32 s1, UINT32 s2)
{
    if (s1 == s2)
    {
        return SERIAL_EQUAL;
    }
    if ((s1 < s2 && (s2 - s1) < TWO_RAISE_TO_31_MIN_1) ||
        (s1 > s2 && (s1 - s2) > TWO_RAISE_TO_31_MIN_1))
    {
        /* Condition for s1 is said to be less than s2 */
        return SERIAL_LESS_THAN;
    }
    if ((s1 < s2 && (s2 - s1) > TWO_RAISE_TO_31_MIN_1) ||
        (s1 > s2 && (s1 - s2) < TWO_RAISE_TO_31_MIN_1))
    {
        /* Condition for s1 is said to be greater than s2 */
        return SERIAL_GREATER_THAN;
    }
    return SERIAL_UNDEFINED;
}

bool IscmdsnInWindow(UINT32 cmdSN, UINT32 maxCmdSN, UINT32 expCmdSN)
{
    INT32       result;

    result = serial_bit_compare(cmdSN, maxCmdSN);
    if (result == SERIAL_EQUAL || result == SERIAL_LESS_THAN)
    {
        result = serial_bit_compare(cmdSN, expCmdSN);
        if (result == SERIAL_EQUAL || result == SERIAL_GREATER_THAN)
        {
            return true;
        }
    }
    return false;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiSeqChkCmdSn
 **
 **              This checks for the command SN, if it matches with expCmdSN
 **              in the session also increments the expCmdSN & maxCmdSN if the
 **              recvd command is non-immediate
 **
 **  @param      pCmd      - Received command
 **  @param      pSsn      - session pointer
 **  @param      update    - update
 **
 **  @return
 **           EXPECTED_CMD, OUT_OF_ORDER_CMD, OUT_OF_WINDOW_CMD, IMMEDIATE_CMD
 **
 ******************************************************************************
 **/
INT32 iscsiSeqChkCmdSn(ISCSI_GENERIC_HDR *pCmd, SESSION *pSsn, UINT8 update UNUSED)
{
    INT32       result = OUT_OF_WINDOW_CMD;
    UINT32      cmdSN;

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Entry: iscsiSeqChkCmdSn()\n");

    /* Check if this pdu is a command pdu */
    if (GET_ISCSI_OPCODE(pCmd->opcode) == INIOP_SCSI_DATA_OUT)
    {
        return NO_CMD;
    }

    /* Check whether the PDU is immediate or not */
    if (IS_IMMBIT_SET(pCmd->opcode) || GET_ISCSI_OPCODE(pCmd->opcode) == INIOP_LOGIN_REQ)
    {
        result = IMMEDIATE_CMD;
        pCmd->opcode |= IMM_BIT;
        return result;
    }
    if (pSsn == NULL)
    {
        printf("iscsiSeqChkCmdSn: Session pointer is null\n");
        return -1;
    }

    /* Do a byteswap for sn and we are sending recvd command directly */
    cmdSN = bswap_32(pCmd->Sn);

    if (IscmdsnInWindow(cmdSN, pSsn->maxCmdSN, pSsn->expCmdSN) == true)
    {
        /* cmdSN will be incremented when this command is processed in iscsiProcRecvPdu */
        if (cmdSN == pSsn->expCmdSN)
        {
            /* This is expected command so we can increment expCmdSN */
            result = EXPECTED_CMD;
            pSsn->expCmdSN++;
            pSsn->maxCmdSN++;
        }
        else
        {
            result = OUT_OF_ORDER_CMD;
        }
    }
    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Exit: iscsiSeqChkCmdSn()\n");
    return result;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiAddInboundPdu
 **
 **              This allocates an ILT and stores the request in that
 **
 **  @param      pReq      - Request pointer
 **  @param      pConn     - Connection pointer
 **  @param      dontQ     - flag to queue or dont queue request
 **
 **  @return    ILT pointer on success NULL otherwise
 **
 ******************************************************************************
 **/
ILT        *iscsiAddInboundPdu(ISCSI_GENERIC_HDR *pReq, CONNECTION *pConn,
                               ISCSI_PDU *pPdu)
{
    ILT        *pILT = NULL;
    INT8        retVal;
    UINT32      length;

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Entry: iscsiAddInboundPdu()\n");

    /* Allocate ILT */
    pILT = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
    pPdu->ahsLen = bswap_32((pReq->ahsDataLen)) & AHS_LEN_MASK;

    if (pPdu->ahsLen != XIO_ZERO)
    {
        /* AHS is present, copy it in the pdu */
        retVal = ISCSI_TSL_Event_Notify(pConn->pTPD, TSL_ISCSI_EVENT_READ, (char *)pPdu->ahs, pPdu->ahsLen);
        if (retVal == -1)
        {
            fprintf(stderr, "ISCSI_TSL_Event_Notify failed\n");
            return NULL;
        }
    }

    /* Add pdu to the IlLT */
    pILT->iscsi_def.pPdu = pPdu;
    length = bswap_32(pReq->ahsDataLen) & DATA_LEN_MASK;
    if (length != XIO_ZERO)
    {
        /*
         * data segment is present, We do not expect data in SCSI command
         * as we do not support immediate data
         */
        fprintf(stderr, "Immediate data is present, not supported in this release\n");
        return NULL;
    }
    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Exit: iscsiAddInboundPdu()\n");
    return pILT;
}

/**
 ******************************************************************************
 **
 **  @brief      getPosition
 **
 **              Get the position where command to be stored
 **
 **  @param      seqNo     - Sequence number
 **
 **  @return    position
 **
 ******************************************************************************
 **/

UINT32 getPosition(UINT32 seqNo)
{
    UINT32      pos;

    /* Get the position using modulo operation */
    pos = seqNo % MAX_COMMANDS;

    return pos;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiSsmtInitState
 **
 **              Initialize session state
 **
 **  @param      pSsn      - session pointer
 **
 **  @return     none
 **
 ******************************************************************************
 **/
void iscsiSsmtInitState(SESSION *pSsn)
{
    /* Session state initialized with FREE */
    pSsn->ssnState = SSN_FREE;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiSsmtUpdateState
 **
 **              Update session state based on event
 **
 **  @param      pSsn      - session pointer
 **  @param      trEvent   - transition event
 **
 **  @return     XIO_SUCCESS on success, XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiSsmtUpdateState(SESSION *pSsn, UINT8 trEvent)
{
    UINT8       result = XIO_SUCCESS;

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Exit: iscsiSsmtUpdateState()\n");
    switch (trEvent)
    {
        case FIRST_CONN_INLOGIN:
            if (pSsn->ssnState == SSN_FREE)
            {
                /* Valid event, update the state */
                pSsn->ssnState = SSN_ACTIVE;
            }
            else
            {
                result = XIO_FAILURE;
            }
            break;

        case ONE_CONN_INLOGIN:
            if (pSsn->ssnState == SSN_ACTIVE)
            {
                /* Valid event, update the state */
                pSsn->ssnState = SSN_LOGGED_IN;
            }
            else
            {
                result = XIO_FAILURE;
            }
            break;

        case GRACEFUL_CLOSE:
            if (pSsn->ssnState == SSN_LOGGED_IN)
            {
                /* Valid event, update the state */
                pSsn->ssnState = SSN_FREE;
            }
            else
            {
                result = XIO_FAILURE;
            }
            break;

        case SSN_FAILURE:
            if (pSsn->ssnState == SSN_LOGGED_IN)
            {
                /* Valid event, update the state */
                pSsn->ssnState = SSN_FAILED;
            }
            else
            {
                result = XIO_FAILURE;
            }
            break;

        case SSN_TIMEOUT:
            if (pSsn->ssnState == SSN_FAILED)
            {
                /* valid event, update the state */
                pSsn->ssnState = SSN_FREE;
            }
            else
            {
                result = XIO_FAILURE;
            }
            break;

        case SSN_CONT:
            if (pSsn->ssnState == SSN_FAILED)
            {
                /* Valid event, update the state */
                pSsn->ssnState = SSN_IN_CONTINUE;
            }
            else
            {
                result = XIO_FAILURE;
            }
            break;

        case LAST_SSN_CONT:
            if (pSsn->ssnState == SSN_IN_CONTINUE)
            {
                /* Valid event, update the state */
                pSsn->ssnState = SSN_FAILED;
            }
            else
            {
                result = XIO_FAILURE;
            }
            break;

        case LOGIN_ON_LEADCONN:
            if (pSsn->ssnState == SSN_ACTIVE)
            {
                /* Valid event, update the state */
                pSsn->ssnState = SSN_FREE;
            }
            else
            {
                result = XIO_FAILURE;
            }
            break;

        case SSN_CONT_SUCCESS:
            if (pSsn->ssnState == SSN_IN_CONTINUE)
            {
                /* Valid event, update the state */
                pSsn->ssnState = SSN_LOGGED_IN;
            }
            else
            {
                result = XIO_FAILURE;
            }
            break;

        case SSN_CLOSE_SUCCESS:
            if (pSsn->ssnState == SSN_IN_CONTINUE)
            {
                /* Valid event, update the state */
                pSsn->ssnState = SSN_FREE;
            }
            else
            {
                result = XIO_FAILURE;
            }
            break;

        default:
            /* Unknown event received */
            fprintf(stderr, " Unknown event received\n");
            result = XIO_FAILURE;
    }
    if (result == XIO_FAILURE)
    {
        fprintf(stderr, "Event receive is not valid in current state\n");
    }
    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Exit: iscsiSsmtUpdateState()\n");
    return result;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiConnInit
 **
 **              Initialize a connection
 **
 **  @param      pConn     - connection double pointer
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiConnInit(CONNECTION **pConn)
{

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Entry: iscsiConnInit()\n");

    /* Initialize a connection and do memset */
    *pConn = (CONNECTION *) s_MallocC(sizeof(CONNECTION), __FILE__, __LINE__);
    if (*pConn == NULL)
    {
        fprintf(stderr, "Malloc Failed\n");
        return XIO_FAILURE;
    }
    memset((*pConn), XIO_ZERO, sizeof(CONNECTION));

    /* Initialize connection params */
    (*pConn)->recvState = IR_FREE;

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Exit: iscsiConnInit()\n");
    return XIO_SUCCESS;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiConnMatchCID
 **
 **              match for CID in the list
 **
 **  @param      isid      - ISID
 **  @param      tid       - Target Id
 **
 **  @return    SESSION pointer on success NULL otherwise
 **
 ******************************************************************************
 **/
CONNECTION *iscsiConnMatchCID(CONNECTION *pHead, UINT32 cid)
{
    CONNECTION *pConnNode = NULL;

    TRACE_ENTRY_EXIT(FU_ENABLE, "Function Entry: iscsiConnMatchCID()\n");

    /* Match the CID in the list */
    for (pConnNode = pHead; pConnNode != NULL; pConnNode = pConnNode->pNext)
    {
        /* Compare the CID part */
        if (pConnNode->cid == cid)
        {
            /* CID is matched */
            TRACE_ENTRY_EXIT(FU_ENABLE, "Function Exit: iscsiConnMatchCID()\n");
            return pConnNode;
        }
    }
    return NULL;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiConnGetCurrState
 **
 **              To get the current state of connection
 **
 **  @param      pConn     - connection pointer
 **
 **  @return    connection state
 **
 ******************************************************************************
 **/
UINT8 iscsiConnGetCurrState(CONNECTION *pConn)
{
    return pConn->state;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiCsmtInitConnState
 **
 **              Initialize connection state
 **
 **  @param      pConn     - connection pointer
 **
 **  @return    XIO_SUCCESS
 **
 ******************************************************************************
 **/
UINT8 iscsiCsmtInitConnState(CONNECTION *pConn)
{
    /* Initialize connection state to FREE */
    pConn->state = CONNST_FREE;
    return XIO_SUCCESS;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiCsmtIllegal
 **
 **              To see if the event is illegal
 **
 **  @param      pConn     - connection pointer
 **  @param      trEvt     - Transition event
 **
 **  @return    XIO_SUCCESS on success, XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/

UINT8 iscsiCsmtIllegal(CONNECTION *pConn UNUSED, UINT8 trEvt)
{
    /* Check the current state and event as per state machine */
    if ((trEvt == CONNEVT_ILL) || (trEvt == CONNEVT_ILL) || (trEvt == CONNEVT_ILL))
    {
        /* illegal event received */
        return XIO_FAILURE;
    }
    return XIO_SUCCESS;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiCsmtRcvdValidConn
 **
 **              This changes the state on receiving valid connection
 **
 **  @param      pConn     - connection pointer
 **  @param      trEvt     - Transition event
 **
 **  @return    XIO_SUCCESS on success, XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiCsmtRcvdValidConn(CONNECTION *pConn, UINT8 trEvt)
{
    /* Check the current state and event as per state machine */
    if (pConn->state == CONNST_FREE)
    {
        if (trEvt == CONNEVT_VALID_CONN)
        {
            /* Update the state */
            pConn->state = CONNST_XPT_UP;
            return XIO_SUCCESS;
        }
    }
    return XIO_FAILURE;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiCsmtInitLoginReqRecvd
 **
 **              This changes the state on receiving initial login request
 **
 **  @param      pConn     - connection pointer
 **  @param      trEvt     - Transition event
 **
 **  @return    XIO_SUCCESS on success, XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiCsmtInitLoginReqRecvd(CONNECTION *pConn, UINT8 trEvt)
{
    /* Check the current state and event as per state machine */
    if (pConn->state == CONNST_XPT_UP)
    {
        if (trEvt == CONNEVT_INIT_LOGIN)
        {
            /* Update the state */
            pConn->state = CONNST_IN_LOGIN;
            return XIO_SUCCESS;
        }
    }
    return XIO_FAILURE;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiCsmtFinLoginReqRecvd
 **
 **              This changes the state on receiving final login request
 **
 **  @param      pConn     - connection pointer
 **  @param      trEvt     - Transition event
 **
 **  @return    XIO_SUCCESS on success, XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiCsmtFinLoginReqRecvd(CONNECTION *pConn, UINT8 trEvt)
{
    /* Check the current state and event as per state machine */
    if (pConn->state == CONNST_IN_LOGIN)
    {
        if (trEvt == CONNEVT_FIN_LOGIN)
        {
            /* Update the state */
            pConn->state = CONNST_LOGGED_IN;
            return XIO_SUCCESS;
        }
    }
    return XIO_FAILURE;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiCsmtLoginTimedOut
 **
 **              This changes the state when login times out
 **
 **  @param      pConn     - connection pointer
 **  @param      trEvt     - Transition event
 **
 **  @return    XIO_SUCCESS on success, XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiCsmtLoginTimedOut(CONNECTION *pConn, UINT8 trEvt)
{
    /* Check the current state and event as per state machine */
    if (pConn->state == CONNST_XPT_UP)
    {
        if (trEvt == CONNEVT_LOGIN_TIMEOUT)
        {
            /* Update the state */
            pConn->state = CONNST_IN_LOGIN;
            return XIO_SUCCESS;
        }
    }
    return XIO_FAILURE;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiCsmtConnCloseEvts
 **
 **              This changes the state on connection close event
 **
 **  @param      pConn     - connection pointer
 **  @param      trEvt     - Transition event
 **
 **  @return    XIO_SUCCESS on success, XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiCsmtConnCloseEvts(CONNECTION *pConn, UINT8 trEvt)
{
    /* Check the current state and event as per state machine */
    if (pConn->state == CONNST_IN_LOGIN)
    {
        if (trEvt == CONNEVT_CONN_CLOSE)
        {
            /* Update the state */
            pConn->state = CONNST_FREE;
            return XIO_SUCCESS;
        }
    }
    return XIO_FAILURE;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiCsmtConnClean
 **
 **              To clean connection state
 **
 **  @param      pConn     - connection pointer
 **  @param      trEvt     - Transition event
 **
 **  @return    XIO_SUCCESS on success, XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiCsmtConnClean(CONNECTION *pConn, UINT8 trEvt)
{
    /* Check the current state and event as per state machine */
    if (pConn->state == CONNST_LOGGED_IN)
    {
        if (trEvt == CONNEVT_CONN_CLEAN)
        {
            /* Update the state */
            pConn->state = CONNST_FREE;
            return XIO_SUCCESS;
        }
    }
    return XIO_FAILURE;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiCsmtLogoutRecvd
 **
 **              Update connection state on reciving a logout event
 **
 **  @param      pConn     - connection pointer
 **  @param      trEvt     - Transition event
 **
 **  @return    XIO_SUCCESS on success, XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiCsmtLogoutRecvd(CONNECTION *pConn, UINT8 trEvt)
{
    /* Check the current state and event as per state machine */
    if (pConn->state == CONNST_LOGGED_IN)
    {
        if (trEvt == CONNEVT_LOGOUT_RECVD)
        {
            /* Update the state */
            pConn->state = CONNST_IN_LOGOUT;
            return XIO_SUCCESS;
        }
    }
    return XIO_FAILURE;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiCsmtLogoutRecvd1
 **
 **              Update connection state on reciving a logout event 1
 **
 **  @param      pConn     - connection pointer
 **  @param      trEvt     - Transition event
 **
 **  @return    XIO_SUCCESS on success, XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiCsmtLogoutRecvd1(CONNECTION *pConn, UINT8 trEvt)
{
    /* Check the current state and event as per state machine */
    if (pConn->state == CONNST_LOGOUT_REQUESTED)
    {
        if (trEvt == CONNEVT_LOGOUT_RECVD1)
        {
            /* Update the state */
            pConn->state = CONNST_IN_LOGOUT;
            return XIO_SUCCESS;
        }
    }
    return XIO_FAILURE;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiCsmtAsyncEvt2Send
 **
 **              Update connection on async event
 **
 **  @param      pConn     - connection pointer
 **  @param      trEvt     - Transition event
 **
 **  @return    XIO_SUCCESS on success, XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiCsmtAsyncEvt2Send(CONNECTION *pConn, UINT8 trEvt)
{
    /* Check the current state and event as per state machine */
    if (pConn->state == CONNST_LOGGED_IN)
    {
        if (trEvt == CONNEVT_ASYNC_EVT2SEND)
        {
            /* Update the state */
            pConn->state = CONNST_LOGOUT_REQUESTED;
            return XIO_SUCCESS;
        }
    }
    return XIO_FAILURE;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiCsmtAsyncEvt2Send1
 **
 **              Update connection state on async event 1
 **
 **  @param      pConn     - connection pointer
 **  @param      trEvt     - Transition event
 **
 **  @return    XIO_SUCCESS on success, XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiCsmtAsyncEvt2Send1(CONNECTION *pConn, UINT8 trEvt)
{
    /* Check the current state and event as per state machine */
    if (pConn->state == CONNST_LOGOUT_REQUESTED)
    {
        if (trEvt == CONNEVT_ASYNC_EVT2SEND1)
        {
            /* State is unchanged */
            return XIO_SUCCESS;
        }
    }
    return XIO_FAILURE;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiCsmtTpConnClose
 **
 **              Update connection state on receving a TP connection close
 **
 **  @param      pConn     - connection pointer
 **  @param      trEvt     - Transition event
 **
 **  @return    XIO_SUCCESS on success, XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/

UINT8 iscsiCsmtTpConnClose(CONNECTION *pConn, UINT8 trEvt)
{
    /* Check the current state and event as per state machine */
    if (pConn->state == CONNST_IN_LOGOUT)
    {
        if (trEvt == CONNEVT_TP_CLOSE)
        {
            /* Update the state */
            pConn->state = CONNST_FREE;
            return XIO_SUCCESS;
        }
    }
    return XIO_FAILURE;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiCsmtTpEvts
 **
 **              Update connection state on TPL events
 **
 **  @param      pConn     - connection pointer
 **  @param      trEvt     - Transition event
 **
 **  @return    XIO_SUCCESS on success, XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiCsmtTpEvts(CONNECTION *pConn, UINT8 trEvt)
{
    /* Check the current state and event as per state machine */
    if (pConn->state == CONNST_LOGGED_IN)
    {
        if (trEvt == CONN_TP_EVTS)
        {
            /* Update the state */
            pConn->state = CONNST_CLEANUP_WAIT;
            return XIO_SUCCESS;
        }
    }
    return XIO_FAILURE;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiCsmtTpEvts1
 **
 **              Update connection state on TPL events1
 **
 **  @param      pConn     - connection pointer
 **  @param      trEvt     - Transition event
 **
 **  @return    XIO_SUCCESS on success, XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiCsmtTpEvts1(CONNECTION *pConn, UINT8 trEvt)
{
    /* Check the current state and event as per state machine */
    if (pConn->state == CONNST_LOGOUT_REQUESTED)
    {
        if (trEvt == CONN_TP_EVTS1)
        {
            /* Update the state */
            pConn->state = CONNST_CLEANUP_WAIT;
            return XIO_SUCCESS;
        }
    }
    return XIO_FAILURE;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiCsmtLogoutProcFailure
 **
 **              Update connection state on Logout processing failure
 **
 **  @param      pConn     - connection pointer
 **  @param      trEvt     - Transition event
 **
 **  @return    XIO_SUCCESS on success, XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiCsmtLogoutProcFailure(CONNECTION *pConn, UINT8 trEvt)
{
    /* Check the current state and event as per state machine */
    if (pConn->state == CONNST_IN_LOGOUT)
    {
        if (trEvt == CONNEVT_LOGOUT_FAIL)
        {
            /* Update the state */
            pConn->state = CONNST_CLEANUP_WAIT;
            return XIO_SUCCESS;
        }
    }
    return XIO_FAILURE;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiCsmtLogoutRespSucces
 **
 **              Update connection state on Logout responose success
 **
 **  @param      pConn     - connection pointer
 **  @param      trEvt     - Transition event
 **
 **  @return    XIO_SUCCESS on success, XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiCsmtLogoutRespSucces(CONNECTION *pConn, UINT8 trEvt)
{
    /* Check the current state and event as per state machine */
    if (pConn->state == CONNST_IN_LOGOUT)
    {
        if (trEvt == CONNEVT_LOGOUT_SUCCESS)
        {
            /* Update the state */
            pConn->state = CONNST_FREE;
            return XIO_SUCCESS;
        }
    }
    return XIO_FAILURE;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
