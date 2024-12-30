/* $Id: iscsi_pdu.c 159663 2012-08-22 15:36:42Z marshall_midden $ */
/**
 ******************************************************************************
 **
 **  @file       iscsi_pdu.c
 **
 **  @brief      iSCSI PDU functions
 **
 **  This provides API's for processing/building iSCSI PDU(s)
 **
 **  Copyright (c) 2005-2010 Xiotech Corporation.  All rights reserved.
 **
 ******************************************************************************
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>

#include "XIO_Types.h"
#include "XIO_Std.h"
#include "iscsi_common.h"
#include "iscsi_pdu.h"
#include "iscsi_tsl.h"
#include "iscsi_digest.h"
#include "iscsi_tmf.h"

#include "iscsi_timer.h"

#include "def.h"
#include "fsl.h"
#include "ilt.h"
#include "isp.h"
#include "LOG_Defs.h"
#include "cdriver.h"
#include "magdrvr.h"
#include "misc.h"
#include "pm.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "xl.h"

#include "scsi.h"
#include "MR_Defs.h"                    /* MRP definitions                  */
#include "sgl.h"
#include "mem_pool.h"
#include "CT_defines.h"

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
#define ISCSI_SSN_CLOSE                     0x00
#define ISCSI_CONN_CLOSE                    0x01
#define ISCSI_CONN_RECOVERY                 0x02
#define ISCSI_RSVD_FF                       0xffffffff
#define ISCSI_TTT_MAX                       0xfffffffe
#define ISCSI_CDB_LEN                       16

#define ISCSI_VER_MAX                       0
#define ISCSI_VER_MIN                       0

#define BUFF_LEN                            8192

#define TIME2RETAIN                         20 /* in seconds */
#define TIME2WAIT                           2  /* in seconds */
#define TIME2WAIT_B4_NEW_LOGOUT             20 /* in seconds */
#define TIME2RETAIN_B4_NEW_LOGOUT           2 /* in seconds */

#define ISCSI_CONN_SSN_CLOSED               0x00 /* connection or session closed successfully */
#define ISCSI_CONN_NOT_FOUND                0x01
#define ISCSI_CONN_RECOVERY_NOT_SUPP        0x02 /* connection recovery not supported */
#define ISCSI_CLEANUP_FAILED                0x03 /* cleanup failed for various reasons */

#define AL_CHAP_WITH_MD5                    5  /* Algorithm CHAP with MD5 */
#define CHAP_MAX_BUFF_LEN                   512
#define CHAP_ENCODING_BASE                  16 /* it can be  16 or 64 */


/*
** status class for initiator and target
*/
#define STATUS_CLASS_SUCCESS        0x00
#define STATUS_CLASS_REDIRECT       0x01
#define STATUS_CLASS_INIERR         0x02 /* INItiator ERRor */
#define STATUS_CLASS_TARGERR        0x03 /* TARGet ERRor */
/*
** status details for initiator and target
*/
#define STATUS_DETAIL_SUCCESS       0x00
#define STATUS_DETAIL_ERR               0x00
#define STATUS_DETAIL_AUTH_FAIL         0x01
#define STATUS_DETAIL_AUTHORIZATION_FAIL   0x02
#define STATUS_OUT_OF_RES               0x02 /* Target has insufficient sesssion/conn or other resources */
#define STATUS_TARGET_DOESNT_EXIST      0x03
#define STATUS_UNSUPPORTED_VER          0x05 /* Unsupporte version */
#define STATUS_DETAIL_TOOMANY_CONN       0x06 /* too many conn on this SSID */
#define STATUS_SESSION_DOESNT_EXIST     0x0a /* session doesnt exist */
#define STATUS_DETAIL_INVALID_DURING_LOGIN   0x0b  /* Invalid during login */


#define ISCSI_SEND_ERR    0x01          /* used to report error status to PROC in iscsi_send */
#define ISCSI_CONNCLOSE   2

/*
******************************************************************************
** Private variables
******************************************************************************
*/
UINT8       respBuff[BUFF_LEN]  = {0}; /* for parsed response */

/*
******************************************************************************
** Public variables and functions - externed in the header file
******************************************************************************
*/
extern TGD     *T_tgdindx[MAX_TARGETS];
extern SDD     *S_sddindx[MAX_SERVERS];
extern UINT8 (*csmtActions[18])(CONNECTION *pConn, UINT8 trEvt);
extern void KernelDispatch (UINT32 returnCode, ILT *pILT,  MR_PKT *pMRP, UINT32 w0);

extern void printBuffer(UINT8*,INT32);

UINT32 sessionSize = sizeof(SESSION);
UINT32 connectionSize = sizeof(CONNECTION);

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
UINT8 iscsiProcLoginReq(ISCSI_LOGIN_REQ_HDR *pLoginReq, CONNECTION *pConn);
UINT8 iscsiProcScsiCmd(ISCSI_SCSI_CMD_HDR *pReq, CONNECTION *pConn, ISCSI_PDU *pPdu);
UINT8 iscsiProcTxtPdu(ISCSI_TEXT_REQ_HDR *pReq, CONNECTION *pConn, ISCSI_PDU *pPdu);
UINT8 iscsiProcLogoutPdu(ISCSI_LOGOUT_REQ_HDR*pLogoutReq, CONNECTION *pConn,ISCSI_PDU *pPdu);
UINT8 iscsiProcChkVer(ISCSI_LOGIN_REQ_HDR *pReq, ISCSI_LOGIN_RESP_HDR *pResp, CONNECTION *pConn);
UINT8 iscsiProcChkCsgNsg(ISCSI_LOGIN_REQ_HDR *pLoginReq,ISCSI_LOGIN_RESP_HDR *pLoginResp,CONNECTION *pConn);
UINT8 iscsiProcScsiDataOut(ISCSI_DATA_OUT_HDR *pRecvdHdr, CONNECTION *pConn, bool OutOrderPDU);
UINT32 getScsiReadSize(ISCSI_PDU *pPdu);
UINT32 getScsiWriteSize(ISCSI_PDU *pPdu);
UINT32 iscsiDoChap(CONNECTION *pConn,UINT8* buff, INT32 *validLen);

INT8 iscsiBuildAndSendLoginReject(CONNECTION *pConn,ISCSI_LOGIN_REQ_HDR *pReq, ISCSI_LOGIN_RESP_HDR *pRespPdu,
                                    UINT16 statClass, UINT16 statDetail);
UINT8 iscsiProcNopOutPdu(ISCSI_NOPOUT_HDR *pReq,CONNECTION *pConn,ISCSI_PDU *pPdu);
UINT8 iscsiBuildLoginResp(ISCSI_LOGIN_REQ_HDR *pReqHdr, ISCSI_LOGIN_RESP_HDR *pRespHdr, UINT16 length, UINT16 tid);
UINT8 iscsiSendDataIn(ISCSI_PDU *pPdu, ILT *pXLILT, CONNECTION *pConn, ILT* iscsi_ILT);
UINT8 iscsiBuildTextResp(ISCSI_TEXT_REQ_HDR *pReqHdr, ISCSI_TEXT_RESP_HDR *pRespHdr, CONNECTION *pConn, UINT16 length);
UINT8 iscsiBuildR2TAndSend(CONNECTION *pConn, ILT *r2tILT, ISCSI_SCSI_CMD_HDR *pCmd);

UINT32 generateTTT(void);
int iscsi_crSendResponse(ILT* pILT);
int iscsi_crSendR2T(ILT* pILT);
int iscsi_crSendLogin (ILT* pILT);
int iscsi_crLogout(ILT* pILT);
int iscsi_crCloseConn(ILT* pILT);

/*
 ******************************************************************************
 ** Code Start
 ******************************************************************************
 */

/**
 ******************************************************************************
 **
 **  @brief      iscsiProcRecvdPdu
 **
 **              Processes received PDU from Transport layer, based on the
 **              opcode type it calls corrsponding function to exucute.
 **
 **  @param      pRecvdHdr - Received iSCSI Header
 **  @param      pTPD      - Transport descriptro
 **  @param      OutOrderPDU   - Out of order PDU or not
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiProcRecvdPdu(UINT8 *pRecvdHdr, ISCSI_TPD *pTPD, ISCSI_PDU *pPdu)
{
    CONNECTION *pConn = NULL;
    ISCSI_LOGIN_RESP_HDR pLoginResp;

    UINT8   result = XIO_FAILURE;
    UINT8   opcode = 0;

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Entry: iscsiProcRecvdPdu()\n");
    /*
    ** get the connection for the recvd pdu
    */
    if (pTPD->pConn == NULL)
    {
        fprintf(stderr,"Conn pointer is null in pTPD for the recvd pdu\n");
        return XIO_FAILURE;
    }
    /*
    ** get the connection from TPD
    */
    pConn = pTPD->pConn;

    /*
    ** Get opcode
    */
    opcode = GET_ISCSI_OPCODE(pRecvdHdr[0]);
    /* To provide input for the nopin timer task, we will check the opcode,if it is not
     * INIOP_NOP_OUT, then some activity is going on, on the connection.  We need to ping
     * the initiator only if the connection is idle for the specified timeout time.
     */
    if (pConn->timer_created && opcode != INIOP_NOP_OUT)
    {
        ResetTimer(pConn->timerid);
    }
    if (pConn->timer_created && opcode == INIOP_LOGOUT_REQ)
    {
        StopTimer(pConn->timerid);
    }

    /*
    ** if it is non-immediate command
    */
    if (opcode != INIOP_SCSI_DATA_OUT )
    {
        if (pConn && pConn->pSession)
        {
            pConn->pSession->cmdSN = bswap_32(((ISCSI_GENERIC_HDR*)pRecvdHdr)->Sn);
        }
    }
    if (pConn->state == CONNST_XPT_UP)
    {
        /*
        ** only login request is allowed
        */
        if (opcode != INIOP_LOGIN_REQ)
        {
            /*
            ** Need to free the connection
            */
            pConn->state = CONNST_FREE;
        }
        else
        {
            /*
            ** got login request, process it
            */
            result = iscsiProcLoginReq((ISCSI_LOGIN_REQ_HDR *) pRecvdHdr, pConn);
            if (!result)
            {
                fprintf(stderr,"ISCSI_DEBUG: iscsiProcLoginReq() failed during pConn->state = CONNST_XPT_UP\n");
                return XIO_FAILURE;
            }
        }
    }

    else if (pConn->state == CONNST_IN_LOGIN)
    {
        /*
        ** it allows only login request, for all other requests,
        ** it MUST send a Login reject with Status "invalid during
        ** login" and then disconnect
        */
        if (opcode != INIOP_LOGIN_REQ)
        {
            if (opcode == 0)
            {
                fprintf(stderr,"ISCSI_DEBUG: Closing the connection as recvd 00 bytes\n");
                /*
                ** Need to free the connection
                */
                pConn->state = CONNST_FREE;
                return XIO_SUCCESS;
            }
            /*
            ** requests other than login are not allowed
            */
            result = iscsiBuildAndSendLoginReject(pConn,(ISCSI_LOGIN_REQ_HDR *)pRecvdHdr,&pLoginResp,
                        STATUS_CLASS_INIERR,STATUS_DETAIL_INVALID_DURING_LOGIN);
        }
        else
        {
            /*
            ** got login request, process it
            */
            result = iscsiProcLoginReq((ISCSI_LOGIN_REQ_HDR*)pRecvdHdr, pConn);
            if (!result)
            {
                fprintf(stderr,"ISCSI_DEBUG: iscsiProcLoginReq() failed in CONNST IN_LOGIN\n");
                return XIO_FAILURE;
            }
        }
    }

    else if (pConn->state == CONNST_LOGGED_IN)
    {
        /*
        ** Connection is in Full Feature Phase (FFP)
        */

        switch(opcode)
        {
            case INIOP_LOGIN_REQ:
                {
                    /*
                    ** Login phase is not allowed before tearing
                    ** down the connection, send reject
                    */
                    result = iscsiBuildAndSendReject(pConn,REJECT_PROTOCOL_ERROR,(ISCSI_GENERIC_HDR*)pRecvdHdr);
                    break;
                }

            case INIOP_TEXT_REQ:
                {
                    /*
                    ** each text pdu when comes will be allocated a place in pSession->commandQ
                    */
                    fprintf(stderr,"ISCSI_DEBUG: Received  Text Request\n");

                    /*
                    ** process Text PDU
                    */
                    result = iscsiProcTxtPdu((ISCSI_TEXT_REQ_HDR*)pRecvdHdr, pConn,pPdu);
                    if (!result)
                    {
                        fprintf(stderr,"ISCSI_DEBUG: iscsiProcTxtPdu() failed\n");
                        return XIO_FAILURE;
                    }
                    break;
                }
            case INIOP_LOGOUT_REQ:
                {
                    /*
                    ** Received a Logout request, process it
                    */
                    fprintf(stderr,"ISCSI_DEBUG: Received  Logout Request\n");
                    result = iscsiProcLogoutPdu((ISCSI_LOGOUT_REQ_HDR*)pRecvdHdr, pConn,pPdu);
                    if (!result)
                    {
                        fprintf(stderr,"ISCSI_DEBUG: iscsiProcLogoutPdu() failed\n");
                        return XIO_FAILURE;
                    }
                    break;
                }

            case INIOP_SCSI_CMD:
                {
                    /*
                    ** Received a SCSI cmd request, process it
                    */
                    /*
                    ** each scsi command pdu when comes will be allocated a place in pSession->commandQ
                    */

                    ISCSI_PDU *tmpPdu = (ISCSI_PDU *)s_MallocC(sizeof(ISCSI_PDU), __FILE__, __LINE__);
                    tmpPdu->position = -1;
                    memcpy(&(tmpPdu->bhs),pRecvdHdr,sizeof(ISCSI_SCSI_CMD_HDR));
                    tmpPdu->ahs = pPdu->ahs;
                    result = iscsiProcScsiCmd((ISCSI_SCSI_CMD_HDR *)pRecvdHdr, pConn,tmpPdu);
                    if (!result)
                    {
                        fprintf(stderr,"iscsiProcScsiCmd() failed\n");
                        return XIO_FAILURE;
                    }
                    break;
                }
            case INIOP_SCSI_DATA_OUT:
                {
                    /*
                    ** got SCSI data out request, process it
                    */
                    result = iscsiProcScsiDataOut((ISCSI_DATA_OUT_HDR *)pRecvdHdr, pConn,false);
                    if (!result)
                    {
                        fprintf(stderr,"iscsiProcScsiDataOutPdu() failed\n");
                        return XIO_FAILURE;
                    }
                    break;
                }
            case INIOP_TASK_MGMT_REQ:
                {
                    /*
                    ** got Task mgmt request, process it
                    ** This is temporary fix for command sequence handling as this ISCSI_PDU will be referenced through
                    ** callback
                    */
                    ISCSI_PDU *tmpPdu = (ISCSI_PDU *)s_MallocC(sizeof(ISCSI_PDU), __FILE__, __LINE__);
                    tmpPdu->position = -1;
                    memcpy(&(tmpPdu->bhs),pRecvdHdr,sizeof(ISCSI_TMF_REQ_HDR));
                    result = iscsiProcTMFPdu((ISCSI_TMF_REQ_HDR*)pRecvdHdr, pConn, tmpPdu);

                    if (!result)
                    {
                        fprintf(stderr,"iscsiProcTaskMgmtReq() failed\n");
                        return XIO_FAILURE;
                    }
                    break;
                }

            case INIOP_NOP_OUT:
                {
                    /*
                    ** got NOP out request process it add sudhakar code here (TBD)
                    */
                    result = iscsiProcNopOutPdu((ISCSI_NOPOUT_HDR *)pRecvdHdr, pConn,pPdu);
                    if (!result)
                    {
                        fprintf(stderr,"iscsiProcNopoutPdu() failed\n");
                        return XIO_FAILURE;
                    }
                    break;
                }

            case INIOP_SNACK_REQ:
                {
                    if (iscsiBuildAndSendReject(pConn, REJECT_SNACK,(ISCSI_GENERIC_HDR *)pRecvdHdr) != XIO_SUCCESS)
                    {
                        return XIO_FAILURE;
                    }
                    break;
                }
            case TARGOP_ASYNC_MSG:
            case TARGOP_LOGIN_RESP:
            case TARGOP_LOGOUT_RESP:
            case TARGOP_NOP_IN:
            case TARGOP_R2T:
            case TARGOP_REJECT:
            case TARGOP_SCSI_DATA_IN:
            case TARGOP_SCSI_RESP:
            case TARGOP_TASK_MGMT_RESP:
            case TARGOP_TEXT_RESP:
                {
                    /*fall down*/
                }

            default:
                {
                    fprintf(stderr,"Default Got Unknown PDU, opcode %d\n",opcode);
                    /*
                    ** Build reject and send it
                    */
                    if (iscsiBuildAndSendReject(pConn, REJECT_INVALID_PDU_FIELD,
                        (ISCSI_GENERIC_HDR *)pRecvdHdr) != XIO_SUCCESS)
                    {
                        return XIO_FAILURE;
                    }

                    return XIO_FAILURE;
                }
        }
    }
    else if (pConn->state == CONNST_IN_LOGOUT ||
            pConn->state == CONNST_LOGOUT_REQUESTED ||
            pConn->state == CONNST_CLEANUP_WAIT )
    {
        /*
        ** Must be in waiting for logout
        */
        if (iscsiBuildAndSendReject(pConn, REJECT_WAITING_FOR_LOGOUT,(ISCSI_GENERIC_HDR *)pRecvdHdr)
                        != XIO_SUCCESS)
        {
            return XIO_FAILURE;
        }
    }
    else
    {
        fprintf(stderr,"ERROR -- Connection state is not valid for this pdu. pTPD = 0x%x pConn = 0x%x pConn->state = %d\n",
                (UINT32)pTPD, (UINT32)pConn,pConn->state);
        return XIO_FAILURE;
    }
    /*
    ** recvd a pdu and processed it, increment recvd pdu
    */
    if (pConn != NULL)
        pConn->numPduRcvd += 1;
    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiProcRecvdPdu()\n");
    return XIO_SUCCESS;

}

/**
 ******************************************************************************
 **
 **  @brief      iscsiProcLogoutPdu
 **
 **              Processes received Logout pdu and updates connection and
 **              session params and closes connection or session.
 **
 **  @param      pLogoutReq- Logout request pointer
 **  @param      pConn     - Connection pointer
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiProcLogoutPdu(ISCSI_LOGOUT_REQ_HDR *pLogoutReq, CONNECTION *pConn,ISCSI_PDU *pPdu)
{
    ISCSI_LOGOUT_RESP_HDR pLogoutResp;
    CONNECTION *pConnNode = NULL;
    UINT8 reasonCode = 0;

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Entry: iscsiProcLogoutPdu()\n");

    memset(&pLogoutResp, XIO_ZERO, ISCSI_HDR_LEN);

    if (pPdu !=NULL)
    {
        RemovePduFromCommandQ(pConn->pSession, pPdu->position);
    }

    /*
    ** update the response
    */
    pLogoutResp.opcode        = TARGOP_LOGOUT_RESP;
    pLogoutResp.flags         = TARGOP_LOGOUT_FLAGS;
    pLogoutResp.initTaskTag   = pLogoutReq->initTaskTag;

    pLogoutReq->cidOrResvd = bswap_16(pLogoutReq->cidOrResvd);

    /*
    ** match for CID from the list
    */
    pConnNode = iscsiConnMatchCID(pConn->pSession->pCONN, pLogoutReq->cidOrResvd);
    if (pConnNode != NULL)
    {
        /*
        ** see whether its connection of session logout
        */
        reasonCode = pLogoutReq->flags & 0x7F; // Immediate Bit

        switch(reasonCode)
        {
            case ISCSI_SSN_CLOSE:
            case ISCSI_CONN_CLOSE:
                pLogoutResp.resp = ISCSI_CONN_SSN_CLOSED;
                pLogoutResp.time2Wait   = bswap_16(TIME2WAIT);
                pLogoutResp.time2Retain = bswap_16(TIME2RETAIN);
                break;

            case ISCSI_CONN_RECOVERY:
                /*
                ** Recovery is not supported in this release
                */
                pLogoutResp.resp = ISCSI_CONN_RECOVERY_NOT_SUPP;
                pLogoutResp.time2Wait   = bswap_16(TIME2WAIT_B4_NEW_LOGOUT);
                pLogoutResp.time2Retain = bswap_16(TIME2RETAIN_B4_NEW_LOGOUT);
                break;

            default :
                // Invalid field
                return XIO_FAILURE;
        }

        pConnNode->state = CONNST_IN_LOGOUT;

        pLogoutResp.statSn      = bswap_32(pConnNode->statSN++);
        pLogoutResp.expCmdSn    = bswap_32(pConnNode->pSession->expCmdSN);
        pLogoutResp.maxCmdSn    = bswap_32(pConnNode->pSession->maxCmdSN);

        iscsi_updateSend(pConnNode, NULL, (char*)&pLogoutResp, ISCSI_HDR_LEN,
                          NULL, 0, 0, iscsi_crLogout, ISCSI_MAX_LUN);
    }
    else
    {
        /*
        ** CID is not present
        */
        fprintf(stderr,"CID is not present, CID is(%d)\n",pLogoutReq->cidOrResvd);

        pLogoutResp.resp        = ISCSI_CONN_NOT_FOUND;
        pLogoutResp.time2Wait   = bswap_16(pConn->pSession->params.defaultTime2Wait);
        pLogoutResp.time2Retain = bswap_16(pConn->pSession->params.defaultTime2Retain);
        pLogoutResp.statSn      = bswap_32(pConn->statSN++);
        pLogoutResp.expCmdSn    = bswap_32(pConn->pSession->expCmdSN);
        pLogoutResp.maxCmdSn    = bswap_32(pConn->pSession->maxCmdSN);

        iscsi_updateSend(pConn, NULL, (char*)&pLogoutResp, ISCSI_HDR_LEN,
                          NULL, 0, 0, iscsi_crRelILT, ISCSI_MAX_LUN);
    }

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiProcLogoutPdu()\n");
    return XIO_SUCCESS;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiProcChkVer
 **
 **              check for the version supported, if not supported then build
 **              the reject response and send back
 **
 **  @param      pLogingReq - Loging request pointer
 **  @param      pResp      - Resp pointer
 **  @param      pConn      - Connection pointer
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiProcChkVer(ISCSI_LOGIN_REQ_HDR *pReq, ISCSI_LOGIN_RESP_HDR *pResp, CONNECTION *pConn)
{
    UINT8 result = XIO_SUCCESS;
    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Entry: iscsiProcChkVer()\n");

    if ((pReq->verMax != ISCSI_VER_MAX) || (pReq->verMin != ISCSI_VER_MIN))
    {
        /*
        ** version is not supported, build login reject and send back to the initiator
        */
        fprintf(stderr,"Version not supported\nSending login reject\n");
        iscsiBuildAndSendLoginReject(pConn,pReq,pResp,STATUS_CLASS_INIERR,STATUS_UNSUPPORTED_VER);
        result = XIO_FAILURE;
    }

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiProcChkVer()\n");
    return result;
}

/**
 ******************************************************************************
 **
 **  @brief      scsiProcChkCsgNsg
 **
 **              Check for the CSG(current stage) and NSG(next stage) params
 **
 **  @param      pLogingReq - Loging request pointer
 **  @param      pLoginResp - Resp pointer
 **  @param      pConn      - Connection pointer
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiProcChkCsgNsg(ISCSI_LOGIN_REQ_HDR *pLoginReq,ISCSI_LOGIN_RESP_HDR *pLoginResp,CONNECTION *pConn)
{
    bool sendLoginReject = XIO_FALSE; /* to see whether to send login reject or not */
    UINT8   result = XIO_SUCCESS;
    UINT8   imm_bit, csg, nsg;

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Entry: iscsiProcChkCsgNsg()\n");
    /*
    ** Check for the I bit, if not set then its protocol error
    */
    imm_bit = IS_IMMBIT_SET(pLoginReq->opcode);
    if (imm_bit != IMM_BIT)
    {
        sendLoginReject = XIO_TRUE;
    }
    /*
    ** check for the valid CSG
    */
    csg = GET_CSG(pLoginReq->flags);
    if (csg >= CSG_INVALID)
    {
        /*
        ** invalid CSG, it should start with Security Negotiation
        ** phase or Login Op Neg phase only
        */
        fprintf(stderr,"iscsiProcChkCsgNsg() Invalid CSG\n");
        sendLoginReject = XIO_TRUE;
    }
    pLoginResp->flags |= csg << CSG_SHIFT;
    /*
    ** check for valid NSG
    */
    nsg = GET_NSG(pLoginReq->flags);
    /*
    ** NSG is valid only when Transmit bit is set
    */
    if (pLoginReq->flags & TR_BIT)
    {
        /*
        ** update login resp flags TBIT
        */
        pLoginResp->flags |= TR_BIT;
        /*
        ** T bit is set here, validate the NSG also
        */
        if ((nsg == CSG_INVALID) || (nsg <= csg))
        {
            /*
            ** invalid NSG recived
            */
            fprintf(stderr,"Invalid NSG received\n");
            sendLoginReject = XIO_TRUE;
        }
        else
        {
            /*
            ** NSG is valid, based on table in RFC section 5.3
            */
            if (csg == CSG_SECURITY )
            {
                /*
                ** see the NSG and update response if its allowed
                */
                if (nsg == CSG_LOGIN_OP_NEG)
                {
                    pLoginResp->flags |= CSG_LOGIN_OP_NEG;
                    pConn->csg = nsg;
                }
                else if (nsg == CSG_FFP)
                {
                    pLoginResp->flags |= CSG_FFP;
                    pConn->csg = nsg;
                }
                else
                {
                    /*
                    ** this is invalid NSG
                    */
                    sendLoginReject = XIO_TRUE;
                }
            }
            else if (csg == CSG_LOGIN_OP_NEG)
            {
                if (nsg == CSG_FFP)
                {
                    /*
                    ** this is allowed, update the login response
                    */
                    pLoginResp->flags |= CSG_FFP;
                    pConn->csg = nsg;
                }
                else
                {
                    /*
                    ** this is not allowed so send login reject
                    */
                    sendLoginReject = XIO_TRUE;
                }
            }
        }
    }

    /*
    ** Now decide whether need to send login reject
    */
    if (sendLoginReject)
    {
        /*
        ** iscsi need to send login reject
        */
        fprintf(stderr,"iscsiProcChkCsgNsg() sending login reject\n");
        result  = iscsiBuildAndSendLoginReject(pConn,pLoginReq,pLoginResp,STATUS_CLASS_INIERR,STATUS_DETAIL_ERR);
        return (result);
    }
    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiProcChkCsgNsg()\n");
    return XIO_SUCCESS;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiProcTxtPdu
 **
 **              Processes received Text PDU, and sends response
 **
 **  @param      pReq       - Text request pointer
 **  @param      pConn      - Connection pointer
 **  @param      OutOrderPdu- out of order PDU
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiProcTxtPdu(ISCSI_TEXT_REQ_HDR *pReq, CONNECTION *pConn, ISCSI_PDU *pPdu)
{
    ISCSI_TEXT_RESP_HDR pResp; /* to build and send text response */
    UINT32  length          = XIO_ZERO; /* for data segment length */
    UINT8   result          = XIO_FAILURE;
    UINT8   opcode          = XIO_ZERO;

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Entry: iscsiProcTxtPdu()\n");

    memset(respBuff,0,BUFF_LEN);
    if (pPdu != NULL)
    {
        RemovePduFromCommandQ(pConn->pSession, pPdu->position);
    }
    /*
    ** do the byteswap
    */
    pReq->ahsDataLen  = bswap_32(pReq->ahsDataLen);
    pReq->LUN         = bswap_64(pReq->LUN);
    pReq->itt         = bswap_32(pReq->itt);
    pReq->ttt         = bswap_32(pReq->ttt);
    pReq->cmdSn       = bswap_32(pReq->cmdSn);
    pReq->expStatSn   = bswap_32(pReq->expStatSn);

    /*
    ** Memset the response Pdu
    */
    memset(&pResp, XIO_ZERO, ISCSI_HDR_LEN);
    /*
    ** Get the opcode and see F BIT and C Bit
    */
    opcode = IS_F_BIT_SET(pReq->flags);
    if (opcode != F_BIT)
    {
        /*
        ** More text request will follow (TBD)
        */
        fprintf(stderr,"F Bit is not set, Waiting for final PDU\n");
        opcode = IS_C_BIT_SET(pReq->flags);
        if (opcode == C_BIT)
        {
            /*
            ** Continue bit is set (TBD)
            */
            fprintf(stderr,"C Bit is not set, TBD\n");
        }
    }
    length = pConn->recvLen;
    /*
    ** This is in order PDU, do the text processing
    */
    result  = iscsiProcTxtReq(pConn->recvBuff, respBuff, &length, pConn->pSession, pConn);
    if (!result)
    {
        fprintf(stderr,"iscsiProcTxtReq() failed\n");
        return XIO_FAILURE;
    }
    /*
    ** Build Text Response PDU and send
    */
    iscsiBuildTextResp(pReq, &pResp, pConn, length);

    iscsi_updateSend(pConn, NULL, (char*)&pResp, ISCSI_HDR_LEN, (char*)&respBuff, length, 0, iscsi_crRelILT, ISCSI_MAX_LUN);

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiProcTxtPdu()\n");
    return XIO_SUCCESS;
}

int iscsi_crTmfWarm(ILT* pILT)
{
    CONNECTION  *pConn = NULL;
    pConn = ((ISCSI_TPD *)(pILT->ilt_normal.w1))->pConn;
    fprintf(stderr,"TARGET WARM RESET: response sent: conn = %x, tid = %d\n",(UINT32)pConn, pConn->pTPD->tid);
    if (pILT->ilt_normal.w2)
    {
        PM_RelSGLWithBuf((SGL*)(pILT->ilt_normal.w2));
        pILT->ilt_normal.w2 = 0;
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
        put_ilt(pILT);
    }

    pConn->state = CONNST_FREE;
    iscsiCloseConn(pConn);
    return ISCSI_CONNCLOSE;
}

int iscsi_crTmfCold(ILT* pILT)
{
    UINT8 port = 0;
    CONNECTION  *pConn = NULL;
    pConn = ((ISCSI_TPD *)(pILT->ilt_normal.w1))->pConn;
    port = ispPortAssignment[pConn->pTPD->tid];

    fprintf(stderr,"TARGET COLD RESET: response sent: conn = %x, tid = %d\n",(UINT32)pConn, pConn->pTPD->tid);
    if (port == 0xFF)
        fprintf(stderr,"******************************************* portassignment wrong\n");
    else
        BIT_SET(tar[port]->opt, TARGET_RESET);

    if (pILT->ilt_normal.w2)
    {
        PM_RelSGLWithBuf((SGL*)(pILT->ilt_normal.w2));
        pILT->ilt_normal.w2 = 0;
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
        put_ilt(pILT);
    }

    return ISCSI_CONNCLOSE;
}

int iscsi_crRelILT(ILT* pILT)
{
    if (pILT->ilt_normal.w2)
    {
        PM_RelSGLWithBuf((SGL*)(pILT->ilt_normal.w2));
        pILT->ilt_normal.w2 = 0;
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
        put_ilt(pILT);
    }
    return XIO_SUCCESS;
}

int iscsi_crCloseConn(ILT* pILT)
{
    CONNECTION  *pConn = NULL;
    pConn = ((ISCSI_TPD *)(pILT->ilt_normal.w1))->pConn;

    fprintf(stderr,"iscsi_crCloseConn: sending reject PDU\n");
    if (pILT->ilt_normal.w2)
    {
        PM_RelSGLWithBuf((SGL*)(pILT->ilt_normal.w2));
        pILT->ilt_normal.w2 = 0;
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
        put_ilt(pILT);
    }
    pConn->state = CONNST_FREE;
    iscsiCloseConn(pConn);
    return ISCSI_CONNCLOSE;
}

int iscsi_crLogout(ILT* pILT)
{
    CONNECTION  *pConn = NULL;
    pConn = ((ISCSI_TPD *)(pILT->ilt_normal.w1))->pConn;

    PM_RelSGLWithBuf((SGL*)(pILT->ilt_normal.w2));
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
    put_ilt(pILT);
    pConn->state = CONNST_FREE;
    iscsiCloseConn(pConn);
    return ISCSI_CONNCLOSE;
}
/**
 ******************************************************************************
 **
 **  @brief      iscsiProcLoginReq
 **
 **              Processes received Login PDU and updates state machine and
 **              sends the response back, handles of CHAP if initiator requests
 **
 **  @param      pLoginReq  - Login request pointer
 **  @param      pConn      - Connection pointer
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8  iscsiProcLoginReq(ISCSI_LOGIN_REQ_HDR *pLoginReq, CONNECTION *pConn)
{
    UINT8       result      = XIO_FAILURE;
    UINT32      chapResult  = 0;
    INT32       validLen    = BUFF_LEN;
    UINT8       *buff       = NULL;
    UINT32      length      = XIO_ZERO; /* for data segment length */
    SESSION     *pSsnNew    = NULL;     /* for new session creation */
    TAR         *pTar       = NULL;
    SESSION     *pSsn       = NULL;
    ISCSI_LOGIN_RESP_HDR    pLoginResp;     /* to build and send login response */
    ILT* pILT;
    TGD* pTGD = NULL;

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Entry: iscsiProcLoginReq()\n");

    /*
    ** memset
    */
    memset(&pLoginResp, XIO_ZERO, sizeof(ISCSI_LOGIN_RESP_HDR));
    memset(respBuff, XIO_ZERO, BUFF_LEN);

    /*
    ** do the byteswap
    */
    pLoginReq->ahsDataLen   = bswap_32(pLoginReq->ahsDataLen);
    /*
    ** tsih is only 16 bit
    */
    pLoginReq->sid.tsih     = bswap_16(pLoginReq->sid.tsih);
    pLoginReq->initTaskTag  = bswap_32(pLoginReq->initTaskTag);
    pLoginReq->cid          = bswap_16(pLoginReq->cid);
    pLoginReq->cmdSn        = bswap_32(pLoginReq->cmdSn);
    pLoginReq->expStatSnOrRsvd = bswap_32(pLoginReq->expStatSnOrRsvd);

    /*
    ** check version supported or not, if not supported then build a login reject
    ** resp and send it and close the connection TCP socket
    */
    result =  iscsiProcChkVer(pLoginReq,&pLoginResp,pConn);
    if (!result)
    {
        /*
        ** We need to return with success, as BuildAndSendLoginReject takes care of clossing
        ** connection
        */
        fprintf(stderr,"iscsiProcChkVer failed, while proc login req, Send login Reject(TBD)\n");
        return XIO_SUCCESS;
    }

    /*
    ** validate the CSG and NSG, if not valid build and send login reject. update the current NSG
    */
    result = iscsiProcChkCsgNsg(pLoginReq,&pLoginResp,pConn);
    if (!result)
    {
        fprintf(stderr,"iscsiProcChkCsgNsg failed, while proc login req, returning FAILURE\n");
        return XIO_SUCCESS;
    }
    /*
    ** check the connection state
    */
    if (CONNST_XPT_UP == pConn->state)
    {
         length = pConn->recvLen;
         iscsiSsnInit(&pSsnNew);
         /*
         ** create the session state machine
         */
         iscsiSsmtInitState(pSsnNew);
         if (pConn->pSession != NULL)
         {
             fprintf (stderr, "**************************************************************\n");
             fprintf (stderr, "**************************************************************\n");
             fprintf (stderr, "************ Memory leak - losing session struct *************\n");
             fprintf (stderr, "**************************************************************\n");
             fprintf (stderr, "**************************************************************\n");
             ((CONNECTION *)0)->pSession = NULL;  // Force crash
         }
         pConn->pSession = pSsnNew;
         pSsnNew->tid = pConn->pTPD->tid;
         /*
         ** initialize default session params
         */
         InitSessionParams (pSsnNew);
         /*
         ** this is the first connection (we support single connection)
         */
         pSsnNew->pCONN = pConn;
         /*
         ** update session in the session list
         */
         pTar  = fsl_get_tar(pConn->pTPD->tid);
         if (pTar == NULL)
         {
             fprintf(stderr,"fsl_get_tar() returned pTAR NULL for tid %d\n",pConn->pTPD->tid);
             return XIO_FAILURE;
         }
         if (pTar->portID == 0 || pTar->portID == 0xffff)
         {
             /*
             ** this is the first node in the list
             */
             pTar->portID  = (UINT32)pSsnNew;
             pTar->ssn_cnt = 1;
         }
         else
         {
             /*
              ** insert at the end
              */
             iscsi_ins_ssn((SESSION*)pTar->portID,pSsnNew);
             pTar->ssn_cnt += 1;
         }

         /*
         ** ProcTxtReq call is needed as we have to get initiatorName and TargetName before proceeding
         */
         result  = iscsiProcTxtReq(pConn->recvBuff, respBuff, &length, pSsnNew, pConn);
         if (!result)
         {

             /*
             ** param validation failed during negotiation, send login reject
             */
             iscsiBuildAndSendLoginReject(pConn,pLoginReq,&pLoginResp, STATUS_CLASS_INIERR,STATUS_DETAIL_ERR);
             return XIO_SUCCESS;
         }

         if (pConn->isChap)
         {
             /*
             ** We are negotiating for CHAP, if T bit is set in the
             ** login request (which is already copied into login response) then clear T bit
             */
             if (pLoginResp.flags & TR_BIT)
             {
                 pLoginResp.flags &= TR_BIT_UNSET;
             }
         }
         /*
         ** Allow Login processing only for xio-iscsi Initiators if gRegTargetsOK is FALSE
         ** otherwise reject login request for non-xio-iscsi Initiators
         */
         if (gRegTargetsOK == FALSE)
         {
             if (fsl_is_xioInit(pSsnNew->params.initiatorName) == FALSE)
             {
                 fprintf(stderr,"ISCSI_DEBUG: Initiator(%s) is NOT allowed to proceed, send Login Reject & closing connection\n",pSsnNew->params.initiatorName);
                 /*
                 ** send Login Reject
                 */
                 iscsiBuildAndSendLoginReject(pConn,pLoginReq,&pLoginResp, STATUS_CLASS_TARGERR,STATUS_DETAIL_ERR);
                 return XIO_SUCCESS;
             }
         }

         /*
         ** Find out if Chap is mandatory on target
         */
         pTGD = T_tgdindx[pConn->pTPD->tid];
         if (pTGD != NULL && pTGD->itgd != NULL && pTGD->itgd->authMethod == 0x81)
         {

             /*
             ** see if authmethod is mandatory and initiator has not sent authmethod for negotiation
             ** if the authMethod is None(initialized value on connection), which means initiator(qlogic)
             ** has not negotiated for AuthMethod, which is also an error
             ** Allow xio-iscsi initiators to login when authmethod is mandatory on target
             */
            if ((stringCompare(pConn->params.authMethod.strval,(UINT8 *) "AUTH_MANDATORY_ERR") == 0 ||
                stringCompare(pConn->params.authMethod.strval,(UINT8 *) "None") == 0) &&
                fsl_is_xioInit(pSsnNew->params.initiatorName) == FALSE)
             {

                 /*
                 ** send login reject with reason authorization failure
                 */
                 fprintf(stderr,"ISCSI_DEBUG: Authorization is Mandatory on target %d for Initiator %s\n",pConn->pTPD->tid,pSsnNew->params.initiatorName);
                 iscsiBuildAndSendLoginReject(pConn,pLoginReq,&pLoginResp, STATUS_CLASS_INIERR,STATUS_DETAIL_AUTHORIZATION_FAIL);
                 return XIO_SUCCESS;
             }
         }

         if (pSsnNew->params.sessionType == NORMAL_SESSION)
         {
             if (isTargetNameMatching(pSsnNew->params.targetName,pSsnNew->params.targetPortalGroupTag,pConn->pTPD->tid)== TRUE)
             {
                 pSsn = iscsiNormalSsnMatchISID(pLoginReq->sid.sid,pConn->pTPD->tid, pSsnNew->params.initiatorName);
             }
             else
             {
                 iscsiBuildAndSendLoginReject(pConn,pLoginReq,&pLoginResp, STATUS_CLASS_INIERR,STATUS_TARGET_DOESNT_EXIST);
                 return XIO_SUCCESS;
             }
         }
         if (pSsn != NULL && pSsn != pSsnNew)
         {

             /*
             ** isid is matching
             */
             if (!pLoginReq->sid.tsih)
             {
                 /*
                 ** TSIH is zero, we need to do session reinstatement
                 ** Try to release Session structure. if not successful send session_not_exist
                 ** if session is released successfully, new session instatement will take place
                 */
                 pSsn->ssnState = SSN_REINST;
                 fprintf(stderr," --- Session Reinstatement --for intiatorName %s Tid 0x%x\n",
                     pSsn->params.initiatorName,pSsn->tid);
                 /*
                 ** cleanup connection & session on pSsn and start new session
                 */
                 iscsiCloseSession(pSsn);
             }
             else
             {
                 /*
                 ** TSIH is non-zero, this is case for adding new connection in session
                 ** NOT SUPPORTED NOW.
                 */
                 iscsiBuildAndSendLoginReject(pConn,pLoginReq,&pLoginResp, STATUS_CLASS_INIERR,STATUS_SESSION_DOESNT_EXIST);
                 return XIO_SUCCESS;

              }
        }
        /*
        ** isid is new
        */
        if (!pLoginReq->sid.tsih)
        {

            /*
            ** update isid in the session
            */
            xioMemcpy(&pSsnNew->isid, pLoginReq->sid.isid,ISCSI_ISID_LEN);
            /*
            ** update command SN, for first req its the same
            */
            pSsnNew->cmdSN      = pLoginReq->cmdSn;
            pSsnNew->firstCmdSN = pLoginReq->cmdSn;
            pConn->statSN       = pLoginReq->expStatSnOrRsvd;
            pSsnNew->expCmdSN   = pLoginReq->cmdSn;

            /*
            ** update the maxCmdSN,itt,version
            */
            pSsnNew->maxCmdSN = pLoginReq->cmdSn + MAX_COMMANDS - 1;
            pSsnNew->itt = pLoginReq->initTaskTag;
            pSsnNew->version = pLoginReq->verMax;

            /*
            ** update the conn state, as we a iSCSI login
            */
            result = csmtActions[CONNEVT_INIT_LOGIN](pConn,CONNEVT_INIT_LOGIN);
            if (!result)
            {
                fprintf(stderr,"csmtActions() failed: state update failed\n");
                return result;
            }
            /*
            ** update the ssn state, as the conn state is in IN_LOGIN state
            */
            result =  iscsiSsmtUpdateState(pSsnNew, FIRST_CONN_INLOGIN);
            if (!result)
            {
                fprintf(stderr,"iscsiSsmtUpdateState failed\n");
                return result;
            }
        }/* isid new else */
    }/* if (CONNST_XPT_UP == pConn->state) */
    else if (CONNST_IN_LOGIN == pConn->state)
    {
        /*
        ** connection is in login phase
        */
        length = pConn->recvLen;
        /*
        ** see if we had sent Authmethod=CHAP
        */
        if (pConn->isChap)
        {
            /*
            ** Process chap
            */
            buff = (UINT8 *)s_MallocC(BUFF_LEN, __FILE__, __LINE__);
            chapResult = iscsiDoChap(pConn,buff,&validLen);
            if (chapResult > 0)
            {
                iscsiBuildAndSendLoginReject(pConn,(ISCSI_LOGIN_REQ_HDR *)pLoginReq,&pLoginResp,
                (chapResult & 0xffff0000)>> 16 ,(chapResult & 0xffff));
                s_Free((void *)buff, BUFF_LEN, __FILE__, __LINE__);
                return XIO_SUCCESS;
            }
        }
        else
        {
            /*
            ** Process the text portion
            */
            result  = iscsiProcTxtReq(pConn->recvBuff, respBuff, &length, pConn->pSession, pConn);
            if (!result || (stringCompare(pConn->params.dataDigest.strval,(UINT8 *) "DDIGEST_MANDATORY_ERR") == 0)
            || (stringCompare(pConn->params.headerDigest.strval,(UINT8 *) "HDIGEST_MANDATORY_ERR") == 0))

            {
                iscsiBuildAndSendLoginReject(pConn,(ISCSI_LOGIN_REQ_HDR *)pLoginReq,&pLoginResp,
                        STATUS_CLASS_INIERR,0);
                fprintf(stderr,"ISCSI_DEBUG: DIGEST (HDR/DATA) is Mandatory/Disabled on target %d for Initiator %s\n",pConn->pTPD->tid,pConn->pSession->params.initiatorName);
                return XIO_SUCCESS;
            }
            else if (result == UNSET_TR_BIT)
            {
                pLoginResp.flags = CLEAR_TR_BIT(pLoginResp.flags);
            }
        }/* End of else */
    }/* End of CONNST_IN_LOGIN  */
    else
    {
        fprintf(stderr,"WHAT STATE IS CONN %d\n",pConn->state);
    }

    /*
    ** update CID for connection from the req, for first req
    */
    pConn->cid = pLoginReq->cid;

    /* build the login response based on login req */
    if ((pConn->isChap) && (pConn->cc != NULL))
    {
        result = iscsiBuildLoginResp(pLoginReq, &pLoginResp, validLen, pConn->pTPD->tid);
    }
    else
    {
        result = iscsiBuildLoginResp(pLoginReq, &pLoginResp, length, pConn->pTPD->tid);
    }
    /*
    ** see if buld login resp success or failure
    */
    if (result != XIO_SUCCESS)
    {
        fprintf(stderr,"iscsiBuildLoginResp failed\n");
        if (buff != NULL)
            s_Free((void *)buff, BUFF_LEN, __FILE__, __LINE__);
        return result;
    }

    /*
    ** update the tsih, if its zero in request then create a new tsih
    */
    if (pLoginReq->sid.tsih == XIO_ZERO &&
            pLoginResp.flags & TR_BIT &&
            GET_NSG(pLoginResp.flags)== CSG_FFP &&
            CONNST_IN_LOGIN == pConn->state)
    {
        /*
        ** check if all mandatory params are negotiated properly
        ** if not then send login reject & close connection
        ** AuthMethod, DataDigest, HeaderDigest can be set Mandatory on a targets
        */
         if (((pTGD = T_tgdindx[pConn->pTPD->tid]) != NULL)
             && (pTGD->itgd != NULL)
             && (fsl_is_xioInit(pConn->pSession->params.initiatorName) == FALSE))
         {
             if (pTGD->itgd->authMethod == 0x81)
             {
                 if ((stringCompare(pConn->params.authMethod.strval,(UINT8 *) "AUTH_MANDATORY_ERR") == 0)
                    || (stringCompare(pConn->params.authMethod.strval,(UINT8 *) "None") == 0))
                 {
                     /*
                     ** send login reject with reason authorization failure
                     */
                     fprintf(stderr,"ISCSI_DEBUG: Authorization is Mandatory on target %d for Initiator %s\n",
                                    pConn->pTPD->tid,pConn->pSession->params.initiatorName);
                     iscsiBuildAndSendLoginReject(pConn,pLoginReq,&pLoginResp,
                                                 STATUS_CLASS_INIERR,STATUS_DETAIL_AUTHORIZATION_FAIL);
                     if (buff != NULL)
                     {
                         s_Free((void *)buff, BUFF_LEN, __FILE__, __LINE__);
                     }
                     return XIO_SUCCESS;
                 }
             }
             if (pTGD->itgd->dataDigest == 0x81)
             {
                 if ((stringCompare(pConn->params.dataDigest.strval,(UINT8 *) "DDIGEST_MANDATORY_ERR") == 0 ||
                             stringCompare(pConn->params.dataDigest.strval,(UINT8 *) "None") == 0))
                 {
                     fprintf(stderr,"ISCSI_DEBUG: Data Digest is Mandatory on target %d for Initiator %s\n",pConn->pTPD->tid,pConn->pSession->params.initiatorName);
                     iscsiBuildAndSendLoginReject(pConn,(ISCSI_LOGIN_REQ_HDR *)pLoginReq,&pLoginResp,STATUS_CLASS_INIERR,0);
                     if (buff != NULL)
                         s_Free((void *)buff, BUFF_LEN, __FILE__, __LINE__);
                     return XIO_SUCCESS;
                 }
             }
             if (pTGD->itgd->headerDigest == 0x81)
             {
                 if ((stringCompare(pConn->params.headerDigest.strval,(UINT8 *) "HDIGEST_MANDATORY_ERR") == 0) ||
                         stringCompare(pConn->params.headerDigest.strval,(UINT8 *) "None") == 0)
                 {
                     fprintf(stderr,"ISCSI_DEBUG: Header Digest is Mandatory on target %d for Initiator %s\n",pConn->pTPD->tid,pConn->pSession->params.initiatorName);
                     iscsiBuildAndSendLoginReject(pConn,(ISCSI_LOGIN_REQ_HDR *)pLoginReq,&pLoginResp,STATUS_CLASS_INIERR,0);
                     if (buff != NULL)
                         s_Free((void *)buff, BUFF_LEN, __FILE__, __LINE__);
                     return XIO_SUCCESS;
                 }
             }
         }
        /*
        ** increment by one and update in response
        */
        pLoginResp.sid.tsih = fsl_gen_tsih(pConn->pTPD->tid);
        /*
        ** check if target has resources or not
        */
        if (pLoginResp.sid.tsih == 0)
        {
            /*
            ** no tsih could be generated, maximum sessions reached on this target
            */
            iscsiBuildAndSendLoginReject(pConn,pLoginReq,&pLoginResp,STATUS_CLASS_TARGERR,STATUS_OUT_OF_RES);
            if (buff != NULL)
                s_Free((void *)buff, BUFF_LEN, __FILE__, __LINE__);
            return XIO_SUCCESS;
        }
        pConn->state = CONNST_LOGGED_IN;
        pConn->pSession->tsih = pLoginResp.sid.tsih;

    }
    else
    {
        /* copy the same as in request */
        pLoginResp.sid.tsih = pLoginReq->sid.tsih;
    }
    /*
    ** get the session from the connection
    */
    pSsnNew = pConn->pSession;

    /*
    ** update SN(s)
    */
    pLoginResp.statSn       = bswap_32(pConn->statSN++);
    pLoginResp.expCmdSn     = bswap_32(pSsnNew->expCmdSN);
    pLoginResp.maxCmdSn     = bswap_32(pSsnNew->maxCmdSN);

    pLoginResp.ahsDataLen   = bswap_32(pLoginResp.ahsDataLen);
    pLoginResp.sid.tsih     = bswap_16(pLoginResp.sid.tsih);
    pLoginResp.initTaskTag  = bswap_32(pLoginResp.initTaskTag);

    pILT = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
    pILT->misc  = (UINT32)0x0;
    pILT->ilt_normal.w0 = (UINT32)pConn;
    pILT->ilt_normal.w1 = (UINT32)pSsnNew;
    pILT->ilt_normal.w2 = (UINT32)pConn->pSession->tsih;
    /*
    ** send complete login response,using iovectors
    */
    if ((pConn->isChap) && (pConn->cc != NULL))
    {
        iscsi_updateSend(pConn, pILT, (char*)&pLoginResp, ISCSI_HDR_LEN, (char *)buff, validLen, 0, iscsi_crSendLogin, ISCSI_MAX_LUN);
        if (pConn->cc->state == CHAP_COMPLETE)
        {
            /*
            ** release the chap context cc
            */
            chap_release_context_st(pConn->cc);
            /*
            ** update isCHAP to false
            */
            pConn->cc = NULL;
            pConn->isChap = XIO_FALSE;
        }
    }
    else
    {
        iscsi_updateSend(pConn, pILT, (char*)&pLoginResp, ISCSI_HDR_LEN, (char*)&respBuff, length, 0, iscsi_crSendLogin, ISCSI_MAX_LUN);
    }
    if (buff != NULL)
        s_Free((void *)buff, BUFF_LEN, __FILE__, __LINE__);
    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiProcLoginReq()\n");
    return XIO_SUCCESS;
}

int iscsi_crSendLogin (ILT* pILT)
{
    CONNECTION *pConn = NULL;
    SESSION    *pSess = NULL;
//    UINT16 tsih = 0;

    PM_RelSGLWithBuf((SGL*)(pILT->pi_pdu.w2));
    pILT->pi_pdu.w2 = 0;

    pILT = pILT - 1;
    pConn = (CONNECTION*)pILT->pi_pdu.w0;
    pSess = (SESSION*)pILT->pi_pdu.w1;
//    tsih  = (UINT16)pILT->pi_pdu.w2;
    if (pILT->pi_pdu.flag & ISCSI_SEND_ERR)
    {
        pConn->state = CONNST_FREE;
        iscsiCloseConn(pConn);
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
        put_ilt(pILT);
        return ISCSI_CONNCLOSE;
    }

#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
    put_ilt(pILT);

    /*
    ** see if we have sent final response, otherwise dont update the connection state
    */
    if (pConn->state == CONNST_LOGGED_IN)
    {
        if (pSess->ssnState == SSN_ACTIVE)
        {

            pSess->ssnState = SSN_LOGGED_IN;
            /*
            ** update active connections, we support one connection
            */
            pSess->activeConns = 1;

            /*
            ** update the Proc with server information
            */
            if (pConn->pSession->params.sessionType == NORMAL_SESSION)
            {
                fprintf(stderr,"*** New Session b/w initiator %s sid %016llx and Tid 0x%x tsih %04x ****\n",
                        pSess->params.initiatorName, pConn->pSession->sid,
                        pSess->tid, pConn->pSession->tsih);
                fsl_SrvLogin(pConn->pTPD->tid,pConn->pSession->tsih,pConn->pSession->sid);
                fsl_logServer(pConn->pSession->sid, pConn->pTPD->tid, pConn->pSession->params.initiatorName,FSL_LOGIN);
            }
            else
            {
                fprintf(stderr,"*** New Discovery Session b/w initiator %s sid %016llx and Tid 0x%x tsih %04x ****\n",
                        pSess->params.initiatorName, pConn->pSession->sid,
                        pSess->tid, pConn->pSession->tsih);
            }
        }
    }
    return 0;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiGetLun
 **
 **              Extracts LUN value and return value
 **
 **  @param      pLun    - LUN pointer
 **
 **  @return     lun on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT32 iscsiGetLun(UINT8 *pLun)
{
    UINT32 lun  = XIO_FAILURE;
    UINT32 temp = XIO_ZERO;

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Entry: iscsiGetLun()\n");

    temp = ((*pLun) >> 6);

    lun = *(pLun + 1);

    switch (temp)
    {
        case XIO_ZERO:
        {
            if (XIO_ZERO != (*pLun))
            {
                fprintf(stderr,"Value 0 in LUN Byte 0\n");
            }
        break;
        }
        case XIO_ONE:
        {
            lun += ((*pLun) & LUN_MASK_0X3F) << 8;
        }
        break;

        default:
            break;
    }

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiGetLun()\n");
    return lun;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiProcScsiCmd
 **
 **              Processes received SCSI command PDU, creates ILT and passes
 **              to FE sublayer.
 **
 **  @param      pReq       - SCSI command pointer
 **  @param      pConn      - Connection pointer
 **  @param      OutOrderPDU   - Out of order PDU or not
 **  @param      position   - position
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiProcScsiCmd(ISCSI_SCSI_CMD_HDR *pReq, CONNECTION *pConn,  ISCSI_PDU *pPdu)
{
    ILT     *pILT       = NULL;
    ILT     *pPrevILT   = NULL;
    UINT8   flags       = XIO_ZERO;
    INT32 ahsLength = 0;

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Entry: iscsiProcScsiCmd()\n");
    /*
    ** allowed only in FFP connection
    */
    if (pConn->state != CONNST_LOGGED_IN)
    {
         fprintf(stderr,"Received SCSI Cmd in invalid connection state (TBD)\n");
        return XIO_FAILURE;
    }
    /*
    ** fill the ILT structure as cmd SN matched then call FE thin layer A
    */
    pILT = iscsiAddInboundPdu((ISCSI_GENERIC_HDR* )pReq, pConn,pPdu);
    if (pILT == NULL)
    {
        fprintf(stderr,"iscsiAddInboundPdu() failed\n");
        return XIO_FAILURE;
    }
    /*
    ** check if F bit is set or not
    */
    flags = (pILT->iscsi_def.pPdu->bhs.flags & ISCSI_SCSI_FIN);
    if (flags == ISCSI_SCSI_FIN)
    {
        /*
        ** If received Immediate data not supported in this release
        */
    }
    /* F bit is 0, chk if wbit is not set */
    else
    {
        /*
        ** check for write
        */
        flags = (pILT->iscsi_def.pPdu->bhs.flags & ISCSI_SCSI_WR);
        if (flags != ISCSI_SCSI_WR)
        {
            /*
            ** w bit not set, this is an error (TBD)
            */
            fprintf(stderr,"w bit not set, this is an error (TBD)\n");
            return XIO_FAILURE;
        }
    }
    pPrevILT = pILT;
    /*
    ** update the CDB pointer
    */
    ahsLength = (((bswap_32(pReq->ahsDataLen) & AHS_LEN_MASK) >> 24) & 0xFF) * 4;
    if (ahsLength == XIO_ZERO)
    {
        pILT->iscsi_def.pCdb    = pILT->iscsi_def.pPdu->bhs.cdb;
        pILT->iscsi_def.pPdu->extCdb = NULL;
        pILT->iscsi_def.pPdu->cdbLen = 0;
    }
    else
    {
        /* FIXME: TotalAHSLength is a count of dwords, not bytes, and may include padding. */
        /*
        ** malloc for the CDB and update cdb len accordingly, if AHS is present
        */
        fprintf(stderr,"AHS found cmdsn=0x%x tid=%d initiatorName=%s\n",pConn->pSession->cmdSN,pConn->pSession->tid,pConn->pSession->params.initiatorName);
        pILT->iscsi_def.pPdu->cdbLen = ISCSI_CDB_LEN + ahsLength;

        pILT->iscsi_def.pPdu->extCdb = (UINT8 *) s_MallocC(pILT->iscsi_def.pPdu->cdbLen, __FILE__, __LINE__);
        xioMemcpy(pILT->iscsi_def.pPdu->extCdb, &pILT->iscsi_def.pPdu->bhs.cdb,ISCSI_CDB_LEN);
        xioMemcpy((pILT->iscsi_def.pPdu->extCdb + ISCSI_CDB_LEN), (UINT8 *) (pILT->iscsi_def.pPdu->ahs),ahsLength);

        pILT->iscsi_def.pCdb  = pILT->iscsi_def.pPdu->extCdb;

        /*
        ** Free the ahs which has come to this function
        */
        s_Free(pILT->iscsi_def.pPdu->ahs,ahsLength, __FILE__, __LINE__);
    }
    /*
    ** zero values to be used
    */
    pILT->iscsi_def.pPdu->bhs.expStatSn = 0;   /* to store r2t buffer offset, incase of writebuffer */
    pILT->iscsi_def.pPdu->bhs.rsvd1     = 0;   /* to store number of r2t's sent in scsi resp        */
    /*
    ** store iSCSI related values in the ILT created
    */
    pILT->iscsi_def.pConn = pConn;
    pILT->iscsi_def.tsih  = pConn->pSession->tsih;
    pILT->iscsi_def.tid   = pConn->pTPD->tid;
    pILT->iscsi_def.tpgt  = (UINT16) pConn->pSession->params.targetPortalGroupTag;
    pILT->iscsi_def.lun   = (UINT16)iscsiGetLun((UINT8*)&(pReq->LUN));
    pILT->iscsi_def.flags = pReq->flags;
    pILT->iscsi_def.portId = ispPortAssignment[(UINT16)pConn->pTPD->tid];

    pILT->iscsi_def.pSgl  = NULL;
    pILT        = (pILT + 1);
    pILT->cr    = NULL;
    pILT->misc  = (UINT32)pPrevILT;
    fsl_iscsi_scsi(pILT);

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiProcScsiCmd()\n");
    return XIO_SUCCESS;
}
/**
 ******************************************************************************
 **
 **  @brief      iscsiProcNopOutPdu
 **
 **              Processes NOPOUT PDU, builds nopIn response and sends
 **
 **  @param      pReq       - NOPOUT PDU pointer
 **  @param      pConn      - Connection pointer
 **  @param      OutOrderPDU   - Out of order PDU or not
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiProcNopOutPdu(ISCSI_NOPOUT_HDR *pReq,CONNECTION *pConn,ISCSI_PDU *pPdu )
{
    UINT8       counter = XIO_ZERO;
    UINT32      length = XIO_ZERO;

    ISCSI_NOPIN_HDR pResp; /* to build and send noop in resp  */

    if (pPdu !=NULL)
    {
        RemovePduFromCommandQ(pConn->pSession, pPdu->position);
    }
    /*
    ** do the byteswap
    */
    pReq->ahsDataLen   = bswap_32(pReq->ahsDataLen);
    pReq->lunOrRsvd    = bswap_64(pReq->lunOrRsvd);
    pReq->initTaskTag  = bswap_32(pReq->initTaskTag);
    pReq->ttt          = bswap_32(pReq->ttt);
    pReq->cmdSn        = bswap_32(pReq->cmdSn);
    pReq->expStatSn    = bswap_32(pReq->expStatSn);
    /*
    ** target has not sent nop in, so ttt must be 0xffffffff
    */

    if (ISCSI_HDR_RSVDFF != pReq->ttt)
    {
      /*
      ** We sent a nopin  to the Initiator and we receive the response
      */
        for (counter=0; counter<MAX_OUTSTANDING_NOPINS; counter++)
        {
            if (pReq->ttt == pConn->nopin_ttt[counter])
            {
                pConn->nopin_ttt[counter]=MAX_INT;
                pConn->outstanding_nopins = 0;
                break;
            }
        }
        return XIO_SUCCESS;
    }
    memset(&pResp, XIO_ZERO, ISCSI_HDR_LEN);
    /*
    ** get the data length
    */
    length = pConn->recvLen;
    /*
    ** Build NOP In hdr
    */
    iscsiBuildNopInHdr(pReq,&pResp,ISCSI_TARG_NOPIN_RESP);
    if (pReq->initTaskTag == ISCSI_HDR_RSVDFF)
    {
        pResp.statSn       = bswap_32(pConn->statSN);
    }
    else
    {
        pResp.statSn       = bswap_32(pConn->statSN++);
    }
    pResp.expCmdSn         = bswap_32(pConn->pSession->expCmdSN);
    pResp.maxCmdSn         = bswap_32(pConn->pSession->maxCmdSN);
    pResp.ahsDataLen       = bswap_32(pResp.ahsDataLen);
    pResp.lunOrRsvd        = bswap_64(pResp.lunOrRsvd);
    pResp.initTaskTag      = bswap_32(pResp.initTaskTag);
    pResp.ttt              = bswap_32(pResp.ttt);
    /*
    ** send ping response only max pdu length bytes
    */
    if (length > pConn->params.maxSendDataSegmentLength)
    {
        length  = pConn->params.maxSendDataSegmentLength;
    }
    pResp.ahsDataLen = bswap_32(length & DATA_LEN_MASK);
    /*
    ** send to the initiator
    */
    if (length == XIO_ZERO)
        iscsi_updateSend(pConn, NULL, (char*)&pResp, ISCSI_HDR_LEN, NULL, 0, 0, iscsi_crRelILT, ISCSI_MAX_LUN);
    else
        iscsi_updateSend(pConn, NULL, (char*)&pResp, ISCSI_HDR_LEN, (char *)pConn->recvBuff, length, 0, iscsi_crRelILT, ISCSI_MAX_LUN);

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiProcNopOutPdu()\n");

    return XIO_SUCCESS;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiProcScsiDataOut
 **
 **              Processes received Dataout PDUs and updates SGL buffer and
 **              informs after completing command
 **
 **  @param      pHdr       - DataOut Header pointer
 **  @param      pConn      - Connection pointer
 **  @param      OutOrderPDU   - Out of order PDU or not
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 **  @attention  Few fields of SCSI command are used
 **
 ******************************************************************************
 **/
UINT8 iscsiProcScsiDataOut(ISCSI_DATA_OUT_HDR *pHdr, CONNECTION *pConn, bool OutOrderPDU UNUSED)
{
    UINT8 result    = XIO_FAILURE;
    UINT8 isLastDataOut = 0;
    ISCSI_PDU *pPdu = NULL;
    ISCSI_SCSI_CMD_HDR *pCmd = NULL;
    ILT *pSecILT = NULL;
    ILT *r2tILT = NULL;
    ILT *pILT = NULL;
    ILT *pPriILT = NULL;
    ILT *pXLILT = NULL;
    ILT* piSCSI_ILT = NULL;
    INT32 recvDataLen = 0;
    INT32 pad = 0;
//     UINT32 writeLen = 0;

    SGL_DESC    *pSglDesc       = NULL;

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Entry: iscsiProcScsiDataOut()\n");
    /*
    ** do byte swap
    */
    pHdr->ahsDataLen    = bswap_32(pHdr->ahsDataLen);
    pHdr->LUN           = bswap_64(pHdr->LUN);
    pHdr->itt           = bswap_32(pHdr->itt);
    pHdr->ttt           = bswap_32(pHdr->ttt);
    pHdr->expStatSn     = bswap_32(pHdr->expStatSn);
    pHdr->buffOffset    = bswap_32(pHdr->buffOffset);
    /*
    ** Mask for datasegment length
    */
    pHdr->ahsDataLen    = pHdr->ahsDataLen & DATA_LEN_MASK;

    /*
    ** retrieve secILT based on ttt
    */
    r2tILT = hash_find(pConn->hashTable,pHdr->itt,pHdr->ttt);
    if (r2tILT == NULL)
    {
        fprintf(stderr,"r2tILT stored is NULL for itt = %x, ttt = %x\n",pHdr->itt,pHdr->ttt);
        return XIO_FAILURE;
    }
    pSecILT = (ILT*)r2tILT->secondary_ilt.secILT;
    /*
    ** Get ILT and XLILT
    */
    pILT = (ILT*)pSecILT->misc;
    pXLILT = pILT;

    /*
    ** Get primary ILT and iSCSI ILT
    */
    pPriILT = (ILT *)(((ILT *)(pSecILT->misc))->ilt_normal.w5);
    piSCSI_ILT = ((ILT *)(pPriILT->misc));

    /*
    ** extract pdu and connection from ilt
    */
    pPdu  = piSCSI_ILT->iscsi_def.pPdu;
    pCmd = (ISCSI_SCSI_CMD_HDR *)&pPdu->bhs;

    /*
    ** get the sgl descriptor,pdu and command
    */
    pSglDesc = pXLILT->fc_xl.xl_pSGL;

    /*
    ** match for itt and ttt
    */
    if (r2tILT->secondary_ilt.ITT != pHdr->itt || r2tILT->secondary_ilt.TTT != pHdr->ttt)
    {
        fprintf(stderr,"Scsi Cmd mismatch\n");
        fprintf(stderr,"stored itt = %d, recvd itt = %d ##### stored ttt = %d, recvd ttt = %d\n",
                    r2tILT->secondary_ilt.ITT,pHdr->itt,r2tILT->secondary_ilt.TTT,pHdr->ttt);
        return XIO_FAILURE;
    }

//     writeLen = getScsiWriteSize(pPdu);
    if (pHdr->buffOffset != r2tILT->secondary_ilt.r2toffset)
    {
        /*
        ** this should not happen if all pdu's come in order
        */
        fprintf(stderr,"mismatch !! buffer offset=(0x%x) r2toffset=(0x%x) itt=(0x%x)\n", pHdr->buffOffset,r2tILT->secondary_ilt.r2toffset,pHdr->itt);
        return XIO_FAILURE;
    }

    /*
    ** in current implementation out of order pdu is not supported
    */
    if (r2tILT->secondary_ilt.dataTxLen == (r2tILT->secondary_ilt.r2toffset - pCmd->expStatSn) -(r2tILT->secondary_ilt.r2tCount - pCmd->rsvd1)*(pConn->pSession->params.maxBurstLength) + pHdr->ahsDataLen)
    {
        isLastDataOut = 1;
    }
    pad = -(pHdr->ahsDataLen) & 3;
    /*
    ** we got the required data-out for r2t, read into SGL buffer
    */
    pConn->offset = 0;
    pConn->recvLen   = pHdr->ahsDataLen  + pad;
    pConn->itt       = r2tILT->secondary_ilt.ITT;
    pConn->ttt       = r2tILT->secondary_ilt.TTT;
    pConn->recvState = IR_RECV_DATA;
    recvDataLen = tsl_recv_dataOut (pConn->pTPD, pSglDesc, pHdr->ahsDataLen  + pad, pXLILT->fc_xl.xl_sgllen, (r2tILT->secondary_ilt.r2toffset - pCmd->expStatSn));
    /*
    ** see if we recvd completely or not, if recvd partial, retry again for remaining bytes. If recvd
    ** -1 then return and cleanup
    */
    if (recvDataLen == -1)
    {
        fprintf(stderr,"&&&&&&&& tsl_recv_data error\n");
        KernelDispatch(1, pILT, NULL, 0);

        result  = hash_delete(pConn->hashTable,r2tILT->secondary_ilt.ITT,r2tILT->secondary_ilt.TTT);
        if (result == XIO_FAILURE)
        {
            fprintf(stderr,"hash_delete() failed\n");
        }
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)r2tILT);
#endif /* M4_DEBUG_ILT */
        put_ilt(r2tILT);
        pConn->state = CONNST_FREE;
        return XIO_FAILURE;
    }
    if (pConn->recvState == IR_COMP)
    {
        return iscsiProcScsiData(pConn, pSglDesc, pHdr, pILT, r2tILT, pCmd, isLastDataOut, pXLILT->fc_xl.xl_sgllen);
    }
    /*
    ** recvd partial data, retry
    */

    return XIO_SUCCESS;

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiProcScsiDataOut()\n");
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiProcScsiData
 **
 **              This verifies datadigest, and checks to send more r2t's
 **
 **  @param      pConn      - Connection pointer
 **  @param      pSgl       - SGL address pointer
 **  @param      pHdr       - dataout hdr ptr
 **  @param      pILT       - ILT pointer
 **  @param      r2tILT     - r2t ILT pointer
 **  @param      isLastDataOut- is this last dataout pdu
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 **
 ******************************************************************************
 **/
UINT8 iscsiProcScsiData(CONNECTION *pConn, SGL_DESC* pSglDesc, ISCSI_DATA_OUT_HDR *pHdr,
                        ILT *pILT, ILT *r2tILT, ISCSI_SCSI_CMD_HDR *pCmd, UINT8 isLastDataOut, UINT8 sglCount)
{
    hash_node *update_node = NULL;
    UINT32 prevTTT = 0;
    UINT8 result = XIO_FAILURE;
    UINT32 dataLen = 0;
    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Entry: iscsiProcScsiData()\n");

    /*
    ** no need to do byte swap  The caller of the function must give correct pHdr
    */
    dataLen = pHdr->ahsDataLen;

    if (ReadDataDigestAndCheck(pConn, pSglDesc, sglCount, pHdr->buffOffset, (pHdr->buffOffset + dataLen - 1)) == -1)
    {
        HandleDataDigestError(pConn,(ISCSI_GENERIC_HDR*)pHdr);
        /*
        ** data digest error
        */
        fprintf(stderr,"DataDigestError: cmdSN = 0x%x buffoffset=%d ExpectedDataXferLength=%d"
               " DataSegLen without padding = %d, r2TSN=%d\n",pConn->pSession->cmdSN,pHdr->buffOffset,
                    r2tILT->secondary_ilt.totalLen, dataLen, r2tILT->secondary_ilt.r2tCount);
        KernelDispatch(1, pILT, NULL, 0);
        result  = hash_delete(pConn->hashTable,r2tILT->secondary_ilt.ITT,r2tILT->secondary_ilt.TTT);
        if (result == XIO_FAILURE)
        {
            fprintf(stderr,"hash_delete() failed\n");
        }
        pConn->state = CONNST_FREE;
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)r2tILT);
#endif /* M4_DEBUG_ILT */
        put_ilt(r2tILT);
        return XIO_FAILURE;
    }
    /*
    ** free connection state
    */
    pConn->recvState = IR_FREE;
    pConn->recvLen = 0;
    pConn->offset = 0;

    r2tILT->secondary_ilt.r2toffset += dataLen;
    /*
    ** done with r2t
    ** R2T may be answered with one or more SCSI Data out PDUs with a matching TTT
    ** We will send scsi response only when it is the Last R2T and the received Data Out is the
    ** last Data Out for this R2T
    */
    if ((r2tILT->secondary_ilt.r2tDone == 1) && (isLastDataOut == 1))
    {
        /*
        ** clear the r2t ILT
        */
        pCmd->expStatSn = r2tILT->secondary_ilt.r2toffset;
        pCmd->rsvd1 = r2tILT->secondary_ilt.r2tCount;
        KernelDispatch(0, pILT, NULL, 0);
        pConn->itt = 0;
        pConn->ttt = 0;
        result  = hash_delete(pConn->hashTable,r2tILT->secondary_ilt.ITT,r2tILT->secondary_ilt.TTT);
        if (result == XIO_FAILURE)
        {
            fprintf(stderr,"hash_delete() failed\n");
        }
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)r2tILT);
#endif /* M4_DEBUG_ILT */
        put_ilt(r2tILT);
        return XIO_SUCCESS;
    }

    /*
    ** see if we need to send more r2t's
    */
    if ((r2tILT->secondary_ilt.r2tDone == 0) && (isLastDataOut == 1))
    {
        r2tILT->secondary_ilt.r2tCount += 1; /* increment the r2t count */
        /*
        ** update ITT,TTT in connection
        */
        pConn->itt       = r2tILT->secondary_ilt.ITT;
        prevTTT          = r2tILT->secondary_ilt.TTT;
        /*
        ** generate TTT for the r2t and store it
        */
        r2tILT->secondary_ilt.TTT       = generateTTT();
        /*
        ** update TTT in hash table node
        */

        if ((update_node = hash_lookup(pConn->hashTable,r2tILT->secondary_ilt.ITT,prevTTT)) != NULL)
        {
            update_node->ttt = r2tILT->secondary_ilt.TTT;
        }
        else
        {
            /*
            ** make sure this debug statement should not come
            */
            fprintf(stderr,"ISCSI_DEBUG: iscsiProcScsiData() hash_lookup failed for ITT %x TTT %x\n",r2tILT->secondary_ilt.ITT,prevTTT);
        }

        if ((r2tILT->secondary_ilt.totalLen - (r2tILT->secondary_ilt.r2toffset - pCmd->expStatSn)) <= pConn->pSession->params.maxBurstLength)
        {
            /*
            ** This is last R2T for this command
            */
            r2tILT->secondary_ilt.dataTxLen = r2tILT->secondary_ilt.totalLen - (r2tILT->secondary_ilt.r2toffset - pCmd->expStatSn);
            r2tILT->secondary_ilt.r2tDone  = 1;
        }
        else
        {
            r2tILT->secondary_ilt.dataTxLen = pConn->pSession->params.maxBurstLength;
        }

        /*
        ** pass connection and r2tILT
        */
        result = iscsiBuildR2TAndSend(pConn, r2tILT,pCmd);
        if (!result)
        {
            fprintf(stderr,"iscsiBuildR2TAndSend() failed\n");
            return XIO_FAILURE;
        }
        return XIO_SUCCESS;
    }

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiProcScsiData()\n");
    return XIO_SUCCESS;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiFeProcMsg
 **
 **              called by FE layer with secondary level ILT
 **
 **  @param      pSecILT    - Secondary level ILT pointer
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiFeProcMsg(ILT *pSecILT)
{
    UINT8 result    = XIO_FAILURE;
//    UINT8 opcode = XIO_ZERO;
    UINT32 cmdWrLen = XIO_ZERO;
    UINT32 dataTxLen = XIO_ZERO;

    ISCSI_SCSI_CMD_HDR *pCmd = NULL;
    ISCSI_SCSI_RESP_HDR pResp;
    ISCSI_PDU *pPdu     = NULL;
    CONNECTION *pConn     = NULL;
    ILT *pILT             = NULL;
    ILT *pILTNew          = NULL;
    ILT *pPriILT         = NULL;
    ILT *pXLILT         = NULL;
    ILT *piSCSI_ILT     = NULL;
    SGL_DESC* pSglDesc    = NULL;

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Entry: iscsiFeProcMsg()\n");
    /*
    ** extract ILT and XL ILT
    */
    pILT = (ILT*)pSecILT->misc;
    pXLILT = pILT; /* p XL */
    /*
    ** Extract iscsi ilt
    */
    pPriILT = (ILT *)(((ILT *)(pSecILT->misc))->ilt_normal.w5);
    piSCSI_ILT = ((ILT *)(pPriILT->misc));
    /*
    ** extract pdu and connection from ilt
    */
    pPdu  = piSCSI_ILT->iscsi_def.pPdu;
    pConn = piSCSI_ILT->iscsi_def.pConn;

    if ((pConn->pSession == NULL) || (pConn->state != CONNST_LOGGED_IN))
    {
        KernelDispatch(1,pILT,0,0);
        return XIO_FAILURE;
    }
    /*
    ** memset scsi resp
    */
    memset(&pResp, 0, sizeof (ISCSI_SCSI_RESP_HDR));
    if (pXLILT->fc_xl.xl_scsist == 0x02)
    {
       /*
       ** maxCmdSN is updated here so before creation of response PDU this should be done
       */
        RemovePduFromCommandQ(pConn->pSession, pPdu->position);
        /*
        ** sense response received
        */
        pCmd = (ISCSI_SCSI_CMD_HDR *)&pPdu->bhs;

        /*
        ** create SCSI response and send
        */
        pResp.opcode         = TARGOP_SCSI_RESP;
        pResp.flags          |= F_BIT;
        pResp.ahsDataLen     = ((pXLILT->fc_xl.xl_snslen + SENSE_DATA_LEN_FLD) & DATA_LEN_MASK);
        pResp.resp           = SCSI_CMD_COMPLETED;
        pResp.status         = SCSI_CMD_CHK_CONDITION;
        pResp.expDataOrRsvd  = 0;
        pResp.initTaskTag    = pCmd->initTaskTag;
        pResp.ahsDataLen     = bswap_32(pResp.ahsDataLen);
        pResp.statSn         = bswap_32(pConn->statSN++);
        pResp.expCmdSn       = bswap_32(pConn->pSession->expCmdSN);
        pResp.maxCmdSn       = bswap_32(pConn->pSession->maxCmdSN);

        iscsi_updateSend(pConn, pSecILT, (char*)&pResp, ISCSI_HDR_LEN,
                         (char *)pXLILT->fc_xl.xl_pSNS,
                         (pXLILT->fc_xl.xl_snslen + SENSE_DATA_LEN_FLD), SENSE_DATA_PRESENT,
                         iscsi_crSendResponse, piSCSI_ILT->iscsi_def.lun);
    }
    else
    {
        if (pXLILT->fc_xl.xl_cmd  == XL_NONE)
        {
            /*
            ** good scsi Response
            */
            pCmd = (ISCSI_SCSI_CMD_HDR *)&pPdu->bhs;
//            opcode = IS_IMMBIT_SET(pCmd->opcode);
            /*
            ** once we are sending response to a command we should remove that command from queue
            */
            RemovePduFromCommandQ(pConn->pSession, pPdu->position);
            /*
            ** Build SCSI response and send
            */
            pResp.opcode         = TARGOP_SCSI_RESP;
            /*
            ** check if overflow/underflow occurred, update flag bit & residual
            ** count
            */
            if (pXLILT->fc_xl.xl_reslen > 0)
            {
                /*
                ** update under flow bit
                */
                pResp.flags          = SCSI_RESP_FLAG_RU;
                pResp.residCntOrRsvd =  (UINT32)pXLILT->fc_xl.xl_reslen;
            }
            else if (pXLILT->fc_xl.xl_reslen < 0)
            {
                /*
                ** Update overflow bit
                */
                pResp.flags          = SCSI_RESP_FLAG_RO;
                pResp.residCntOrRsvd =  (UINT32)(0 - pXLILT->fc_xl.xl_reslen);
            }
            else
            {
                /*
                ** NO OF/UF
                */
                pResp.flags                 = 0;
                pResp.residCntOrRsvd        = 0;
            }
            pResp.flags          |= F_BIT;
            /*
            ** we only send a non-zero iSCSI response if we have an
            ** error that can't be reported via the SCSI status.
            ** The SCSI status field is undefined if the response is non-zero.
            **/
            pResp.resp           = SCSI_CMD_COMPLETED;
            pResp.status         = pXLILT->fc_xl.xl_scsist;
            pResp.initTaskTag    = pCmd->initTaskTag;
            /*
            ** Number of DataIn/ r2t Pdu's send to initiator is stored in pCmd->rsvd1 field
            */
            pResp.expDataOrRsvd  = bswap_32(pCmd->rsvd1);
            pResp.ahsDataLen     = bswap_32(pResp.ahsDataLen);
            pResp.residCntOrRsvd = bswap_32(pResp.residCntOrRsvd);
            pResp.statSn         = bswap_32(pConn->statSN++);
            pResp.expCmdSn       = bswap_32(pConn->pSession->expCmdSN);
            pResp.maxCmdSn       = bswap_32(pConn->pSession->maxCmdSN);
            iscsi_updateSend(pConn, pSecILT, (char*)&pResp, ISCSI_HDR_LEN, NULL, 0, 1,
                              iscsi_crSendResponse, piSCSI_ILT->iscsi_def.lun);
        }
        else if (pXLILT->fc_xl.xl_cmd == XL_DATA2INIT)
        {
            /*
            ** Data to Initiator, Read operation
            */
            pSglDesc = pXLILT->fc_xl.xl_pSGL;
            result =  iscsiSendDataIn(pPdu,pXLILT,pConn,pSecILT);
            if (!result)
            {
                fprintf(stderr,"ISCSI_DEBUG: FeProcMsg   iscsiSendDataIn() failed\n");
                pConn->state = CONNST_FREE;
                pConn->pSession->ssnState = SSN_FAILED;
                KernelDispatch(1,pILT,0,0);
                return XIO_FAILURE;
            }
        }
        else if (pXLILT->fc_xl.xl_cmd == XL_DATA2CTRL)
        {
            /*
            ** data to controller, Write operation
            */
            /*
            ** get the command
            */
            pCmd = (ISCSI_SCSI_CMD_HDR *)&pPdu->bhs;
            pSglDesc = pXLILT->fc_xl.xl_pSGL;

            /*
            ** get the length
            */
            cmdWrLen = getScsiWriteSize(pPdu);
            if (cmdWrLen == XIO_ZERO)
            {
                /*
                ** Write Length is zero, complete command back to PROC, it will send SCSI resp.
                */
                fprintf(stderr, "ISCSI_DEBUG: FeProcMsg  getScsiWriteSize()returned zero, received Write Cmd for 0 bytes ITT %x\n",
                        bswap_32(pCmd->initTaskTag));
                KernelDispatch(0,pILT,0,0);
                return XIO_SUCCESS;
            }
            /*
            ** check the sgl length
            */
            if (pSglDesc->len < cmdWrLen)
            {
                cmdWrLen = pSglDesc->len;
                if (pXLILT->fc_xl.xl_sgllen > 1)
                {
                    INT32 i = 0;
                    INT32 length = 0;
                    /*
                    ** Add sgl descriptor length and request those many number of bytest in R2T
                    */
                    for (i = 0; i<pXLILT->fc_xl.xl_sgllen;i++)
                    {
                        length += pSglDesc->len & 0x00ffffff;
                        pSglDesc++;
                    }
                    cmdWrLen = length;
                }
            }
            /*
            ** create ILT for sending out r2t, update
            */
            pILTNew = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILTNew);
#endif /* M4_DEBUG_ILT */
            /*
            ** update ITT TTT r2toffset, totalLen and r2t count
            */
            pILTNew->secondary_ilt.secILT = (UINT32)pSecILT;
            pILTNew->secondary_ilt.ITT    = bswap_32(pCmd->initTaskTag);

            pILTNew->secondary_ilt.TTT    = generateTTT();
            pILTNew->secondary_ilt.totalLen  = cmdWrLen;
            /*
            ** increment the r2t count, to send in final response
            */
            pCmd->rsvd1 += 1;
            pILTNew->secondary_ilt.r2tCount = pCmd->rsvd1;
            /*
            ** expStatSn field is not used while sending scsiresp,
            ** so reusing for writebuff process
            */
            pILTNew->secondary_ilt.r2toffset = pCmd->expStatSn;
            /*
            ** based on MaxBurstLength send r2t
            */
            if (cmdWrLen <= pConn->pSession->params.maxBurstLength)
            {
                /*
                ** single r2t is enough
                */
                dataTxLen = cmdWrLen;
                /*
                ** no need to send more r2ts
                */
                pILTNew->secondary_ilt.r2tDone = 1;
                pILTNew->secondary_ilt.dataTxLen = dataTxLen;
            }
            else
            {
                /*
                ** need to send more than one r2t(s)
                */
                dataTxLen = pConn->pSession->params.maxBurstLength;
                /*
                ** store sent size for sending next r2t
                */
                pILTNew->secondary_ilt.dataTxLen = dataTxLen; /* data transfer length in data-out
                                                ** corresponding to this r2Tincrement the r2t count,
                                                ** to send in final response
                                                */
                pILTNew->secondary_ilt.r2tDone = 0; /* need to send more r2ts */
            }

            /*
            ** add ILT into hash table
            */
            result = hash_insert(pConn->hashTable,pILTNew->secondary_ilt.ITT,pILTNew,pILTNew->secondary_ilt.TTT);
            if (result != XIO_SUCCESS)
            {
                fprintf(stderr," ISCSI_DEBUG: FeProcMsg - hash_inset() failed\n");
                pConn->state = CONNST_FREE;
                pConn->pSession->ssnState = SSN_FAILED;
                KernelDispatch(1,pILT,0,0);
                return XIO_FAILURE;
            }

            /*
            ** pass connection,offset,dataTxlen,ttt,itt
            */
            result  = iscsiBuildR2TAndSend(pConn,pILTNew,pCmd);
            if (!result)
            {
                pConn->state = CONNST_FREE;
                pConn->pSession->ssnState = SSN_FAILED;
                fprintf(stderr,"ISCSI_DEBUG: FeProcMsg  iscsiBuildR2TAndSend() failed\n");
                KernelDispatch(1,pILT,0,0);
                return XIO_FAILURE;
            }
        }
    }

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiFeProcMsg()\n");
    return XIO_SUCCESS;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsi_crSendResponse
 **
 **              Completion routine called after sending SCSI response
 **
 **  @param      pILT    - ILT pointer
 **
 ******************************************************************************
 **/
int iscsi_crSendResponse(ILT* pILT)
{
    if (pILT->pi_pdu.w2)
    {
        PM_RelSGLWithBuf((SGL*)(pILT->pi_pdu.w2));
        pILT->pi_pdu.w2 = 0;

        if (pILT->pi_pdu.flag & ISCSI_SEND_ERR)
        {
            KernelDispatch(1, (ILT*)(pILT->pi_pdu.w0), NULL, 0);
        }
        else if (pILT->pi_pdu.dataDone)
        {
            KernelDispatch(0, (ILT*)(pILT->pi_pdu.w0), NULL, 0);
        }
    }
    return 0;
}

static int iscsi_crDataInResponse(ILT *pILT)
{
    if (pILT->pi_pdu.w2)
    {
        PM_RelSGLWithBuf((SGL*)(pILT->pi_pdu.w2));
        pILT->pi_pdu.w2 = 0;

        /*
        ** check if read buffer SGL is allocated and Free it.
        */
        if ((pILT + 1)->ilt_normal.w6 != 0)
        {
            PM_RelSGLWithBuf((SGL*)((pILT+1)->ilt_normal.w6));
            (pILT + 1)->ilt_normal.w6 = 0;
        }
        if (pILT->pi_pdu.flag & ISCSI_SEND_ERR)
        {
            KernelDispatch(1, (ILT*)(pILT->pi_pdu.w0), NULL, 0);
        }
        else if (pILT->pi_pdu.dataDone)
        {
            KernelDispatch(0, (ILT*)(pILT->pi_pdu.w0), NULL, 0);
        }
    }
    return 0;
}
/**
 ******************************************************************************
 **
 **  @brief      iscsi_crSendR2T
 **
 **              Completion routine called after sending R2T
 **
 **  @param      pILT    - ILT pointer
 **
 ******************************************************************************
 **/
int iscsi_crSendR2T(ILT* pILT)
{
    if (pILT->ilt_normal.w2)
    {
        PM_RelSGLWithBuf((SGL*)(pILT->ilt_normal.w2));
        pILT->ilt_normal.w2 = 0;
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
        put_ilt(pILT);
    }
    return 0;
}

/**
 ******************************************************************************
 **
 **  @brief     iscsiBuildLoginReject
 **
 **             This is for building login response with status code and detail
 **             on error
 **
 **  @param     pReq    - Login request pointer
 **  @param     pRespPdu  - Response pointer
 **  @param     statClass   - Status Class
 **  @param     statDetail  - Status Detail
 **
 **  @return    XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
INT8 iscsiBuildAndSendLoginReject(CONNECTION *pConn,ISCSI_LOGIN_REQ_HDR *pReq,
                        ISCSI_LOGIN_RESP_HDR *pRespPdu, UINT16 statClass, UINT16 statDetail)
{
    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Entry: iscsiBuildLoginReject()\n");
    if (!pConn || !pReq )
    {
        return XIO_FAILURE;
    }
    fprintf(stderr,"ISCSI_DEBUG: Sending Login reject PDU status class=0x%x status details=0x%x conn %x\n",statClass,statDetail,(UINT32)pConn);
    pRespPdu->opcode         = TARGOP_LOGIN_RESP;
    pRespPdu->flags          = 0;
    pRespPdu->statClass      = statClass;
    pRespPdu->statDetail     = statDetail;
    pRespPdu->verActive      = ISCSI_VER_MIN;
    pRespPdu->verMax         = ISCSI_VER_MAX;
    pRespPdu->ahsDataLen     = XIO_ZERO;
    pRespPdu->initTaskTag    = bswap_32(pReq->initTaskTag);
    pRespPdu->sid.tsih       = bswap_16(pReq->sid.tsih);

    xioMemcpy(&pRespPdu->sid.sid, &pReq->sid.sid, ISCSI_ISID_LEN);

    iscsi_updateSend(pConn, NULL, (char*)pRespPdu, ISCSI_HDR_LEN, NULL, 0, 0, iscsi_crCloseConn, ISCSI_MAX_LUN);

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiBuildLoginReject()\n");
    return XIO_SUCCESS;
}
/**
 ******************************************************************************
 **
 **  @brief     iscsiBuildAndSendReject
 **
 **             This is for building Reject when theres a protocol error
 **
 **  @param     pReject - Reject header pointer
 **  @param     reason  - reason
 **  @param     pConn   - Connection pointer
 **
 **  @return    XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 **  @attention This is to be tested
 **
 ******************************************************************************
 **/
INT32 iscsiBuildAndSendReject(CONNECTION *pConn, REJECT_REASON_CODE reason, ISCSI_GENERIC_HDR *pRejectedHdr)
{
    ISCSI_REJECT_HDR rejectHdr;

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Entry: iscsiBuildReject()\n");

    if (pConn == NULL)
    {
       printf("iscsiBuildAndSendReject: CONNECTION ptr is null\n");
       return XIO_FAILURE;
    }
    rejectHdr.opcode = TARGOP_REJECT;
    rejectHdr.flags  = TARGOP_REJECT_FLAGS;
    rejectHdr.reason = reason;
    /*
    ** these will carry thier usual values
    */
    if (reason == REJECT_PROTOCOL_ERROR && (GET_ISCSI_OPCODE(pRejectedHdr->opcode)== INIOP_SNACK_REQ )
        &&  (GET_SNACK_TYPE(pRejectedHdr->opcode)) == DATA_R2T_SNACK)

    {
        rejectHdr.dataSNorR2tsnorRsvd =    0;
        printf("TODO:: Reject pdu should carry valid Data/R2T SN\n");
    }
    /*
    ** val must be all 1's
    */
    rejectHdr.val     = ISCSI_RSVD_FF;
    rejectHdr.statSn = bswap_32(pConn->statSN++);
    if (pConn->pSession)
    {

       rejectHdr.expCmdSn = bswap_32(pConn->pSession->expCmdSN);
       rejectHdr.maxCmdSn = bswap_32(pConn->pSession->maxCmdSN);
    }
    rejectHdr.ahsDataLen     = bswap_32(ISCSI_HDR_LEN & DATA_LEN_MASK);
    fprintf(stderr,"iscsiBuildRejectPdu sending reject pdu reason = 0x%x\n",reason);

    iscsi_updateSend(pConn, NULL, (char*)&rejectHdr, ISCSI_HDR_LEN, (char *)pRejectedHdr, ISCSI_HDR_LEN,0, iscsi_crRelILT, ISCSI_MAX_LUN);

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiBuildReject()\n");
    return XIO_SUCCESS;
}


/**
 ******************************************************************************
 **
 **  @brief      iscsiBuildLoginResp
 **
 **              this is for building login response PDU header
 **
 **  @param     pReqHdr    - Login request pointer
 **  @param     pRespHdr   - Login response pointer
 **  @param     length     - length
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiBuildLoginResp(ISCSI_LOGIN_REQ_HDR *pReqHdr, ISCSI_LOGIN_RESP_HDR *pRespHdr,\
                            UINT16 length, UINT16 tid UNUSED)
{

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Entry: iscsiBuildLoginResp()\n");
    /*
    ** update the response pdu flags based on the input PDU
    */
    pRespHdr->opcode     = TARGOP_LOGIN_RESP;
    pRespHdr->verActive  = ISCSI_VER_MIN;
    pRespHdr->verMax     = ISCSI_VER_MAX;
    pRespHdr->ahsDataLen = length & DATA_LEN_MASK;
    /*
    ** copy isid into response
    */
    xioMemcpy(&pRespHdr->sid.isid, &pReqHdr->sid.isid, ISCSI_ISID_LEN);
    /*
    ** set tsih only  when the NSG is FFP
    */
    /*
    ** Update itt, status
    */
    pRespHdr->initTaskTag    = pReqHdr->initTaskTag;
    pRespHdr->statClass      = STATUS_CLASS_SUCCESS;
    pRespHdr->statDetail     = STATUS_DETAIL_SUCCESS;

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiBuildLoginResp()\n");
    return XIO_SUCCESS;
}
/**
 ******************************************************************************
 **
 **  @brief      iscsiBuildTextResp
 **
 **              This builds text response PDU
 **
 **  @param      pReqHdr  - Text request header pointer
 **  @param      pRespHdr - Text response header pointer
 **  @param      pConn    - Connection pointer
 **  @param      length   - length
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiBuildTextResp(ISCSI_TEXT_REQ_HDR *pReqHdr, ISCSI_TEXT_RESP_HDR *pRespHdr, \
                            CONNECTION *pConn, UINT16 length)
{

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Entry: iscsiBuildTextResp()\n");

    /*
    ** Update the response PDU header
    */
    pRespHdr->opcode         = TARGOP_TEXT_RESP;
    pRespHdr->flags         |= F_BIT;
    pRespHdr->itt            = bswap_32(pReqHdr->itt);
    pRespHdr->ahsDataLen     = bswap_32(length & DATA_LEN_MASK);
    pRespHdr->ttt            = bswap_32(ISCSI_RSVD_FF);

    pRespHdr->statSn         = bswap_32(pConn->statSN++);
    pRespHdr->expCmdSn       = bswap_32(pConn->pSession->expCmdSN);
    pRespHdr->maxCmdSn       = bswap_32(pConn->pSession->maxCmdSN);

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiBuildTextResp()\n");
    return XIO_SUCCESS;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiSendDataIn
 **
 **              This builds data in pdu and sends to the initiator requested
 **
 **  @param      pPdu       - iscsi PDU pointer
 **  @param      pXLILT     - XL ILT pointer
 **  @param      pConn      - Connection pointer
 **  @param      piSCSI_ILT       - iSCSI ILT pointer
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiSendDataIn(ISCSI_PDU *pPdu, ILT *pXLILT, CONNECTION *pConn,ILT* pSecILT)
{
    UINT8      dataInDone     = XIO_ZERO; /* is the tx over or not */
    UINT32     length         = XIO_ZERO; /* length of buffer to be sent to initiator */
    UINT32     expDataTxLen   = XIO_ZERO; /* datalen requested in command */
    UINT32     dataLen        = XIO_ZERO; /* data length for the pdu */
    UINT32     sglLen         = XIO_ZERO; /* sgl buffer length */
    UINT32     offset         = XIO_ZERO; /* offset for sending more data in pdu's */
    UINT32     sn             = XIO_ZERO; /* data SN for the pdu's */
    UINT16     dataIncnt      = XIO_ZERO; /* total no of data in pdu's sent */
    UINT8*     pTmpBuff       = NULL;     /* for sending a portion of data */
    ISCSI_DATA_IN_HDR pHdr;
    SGL *pSGL = NULL;
    SGL_DESC *pSglDesc = pXLILT->fc_xl.xl_pSGL;
    UINT32 tmpLen = 0;
    UINT8 sglCount = pXLILT->fc_xl.xl_sgllen;
    UINT8 i=0;
    UINT32 addr = ISCSI_HDR_LEN;
    ILT* piSCSI_ILT = NULL;

    UINT8 scsiResp = (pXLILT->fc_xl.xl_fcflgs & XL_STATUSWIO);
    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Entry: iscsiSendDataIn()\n");
    ISCSI_SCSI_CMD_HDR *pCmd = (ISCSI_SCSI_CMD_HDR *) &(pPdu->bhs);


    piSCSI_ILT = (ILT *)(((ILT *)(pSecILT->misc))->ilt_normal.w5);
    piSCSI_ILT = ((ILT *)(piSCSI_ILT->misc));

    /*
    ** If sglCount >1 meaning, there are multiple sgl's for this command
    ** allocating a single sgl and copy all the recvd sgl's into it.
    ** And free when dataIn completes
    */
    sglLen = pSglDesc->len;
    length = sglLen;

    if (sglCount > 1 || (length & 3) != 0)
    {
        SGL_DESC    *desc;

        desc = pXLILT->fc_xl.xl_pSGL;
        for (i = 0; i < sglCount; ++i, ++desc)
        {
            tmpLen += desc->len;
        }
        sglLen = length = tmpLen;
        pSGL = (SGL *)m_asglbuf((tmpLen + 3) & ~3);
        pSGL->scnt = 1;
        pSglDesc = (SGL_DESC *)(pSGL + 1);

        tmpLen = 0;
        desc = pXLILT->fc_xl.xl_pSGL;
        for (i = 0; i < sglCount; ++i, ++desc)
        {
            memcpy((char *)pSglDesc->addr + tmpLen, desc->addr, desc->len);
            tmpLen += desc->len;
        }
       // fprintf(stderr,"dataIn: multiple SGL total length = %d, SGL pointer = 0x%x\n", length, (UINT32)pSGL);
        addr = (UINT32)pSGL;
    }

    /*
    ** get the length
    */
    expDataTxLen = getScsiReadSize(pPdu);
    if (expDataTxLen == XIO_ZERO)
    {
        /*
        ** nothing to be to be sent to initiator, We must honour this request so complete it back to
        ** proc
        */
        fprintf(stderr, "ISCSI_DEBUG: getScsiReadSize() returned zero ITT %x\n",
                bswap_32(pCmd->initTaskTag));
        KernelDispatch(0,pXLILT,0,0);
        return XIO_SUCCESS;
    }

    /*
    ** send all data In pdu's in one shot
    */
    do
    {
        /*
        ** memset for the data in pdu hdr
        */
        memset(&pHdr, 0, sizeof(ISCSI_DATA_IN_HDR));
        /*
        ** check if we can send data requested in single DataIn
        */
        if (length >= pConn->params.maxSendDataSegmentLength)
        {
            dataLen = pConn->params.maxSendDataSegmentLength;
        }
        else
        {
            dataLen = length;
        }
        /*
        ** update the length sglLen and expDataTxLen
        */
        length -= dataLen;
        sglLen -= dataLen;
        expDataTxLen -= dataLen;
        /*
        ** update data length in pdu hdr
        */
        pHdr.ahsDataLen = (dataLen & DATA_LEN_MASK);
        if (length == 0)
        {
            /*
            ** if length is zero then this is the final pdu
            */
            pHdr.flags  = F_BIT;
            dataInDone  = 1;
            /*
            ** check if this is PDU with status, then statSN should be incremented after
            ** sending this PDU with status
            */
            if (scsiResp == XL_STATUSWIO)
            {
                pHdr.flags    |=  DATA_IN_FLAG_S;
                pHdr.statSn   = bswap_32(pConn->statSN++);
                pHdr.status   = pXLILT->fc_xl.xl_scsist;
                /*
                ** check for overflow/under flow set bit & residual count
                */
                if (pXLILT->fc_xl.xl_reslen > 0)
                {
                    /*
                    ** update under flow bit
                    */
                    pHdr.flags  |= SCSI_RESP_FLAG_RU;
                    pHdr.resCnt  =  (UINT32)pXLILT->fc_xl.xl_reslen;
                }
                else if (pXLILT->fc_xl.xl_reslen < 0)
                {
                    /*
                    ** Update overflow bit
                    */
                    pHdr.flags  |= SCSI_RESP_FLAG_RO;
                    pHdr.resCnt  =  (UINT32)(0 - pXLILT->fc_xl.xl_reslen);
                }
            }
            else
            {
                /*
                ** update stat SN but do not increment as we are not
                ** sending status along with this PDU.
                */
                pHdr.statSn   = bswap_32(pConn->statSN);
            }
        }
        else
        {
            pHdr.statSn   = bswap_32(pConn->statSN);
        }
        pTmpBuff = (UINT8 *)(pSglDesc->addr) + offset;
        dataIncnt +=1;
        /*
        ** update pHdr
        */
        pHdr.opcode         = TARGOP_SCSI_DATA_IN;
        pHdr.itt            = pCmd->initTaskTag;
        pHdr.LUN            = 0;
        pHdr.ttt            = ISCSI_HDR_RSVDFF; /* ttt reserved must be ffffffff */
        pHdr.buffOffset     = bswap_32(offset);
        pHdr.ahsDataLen     = bswap_32(pHdr.ahsDataLen);
        pHdr.resCnt         = bswap_32(pHdr.resCnt);
        pHdr.expCmdSn       = bswap_32(pConn->pSession->expCmdSN);
        pHdr.maxCmdSn       = bswap_32(pConn->pSession->maxCmdSN);
        pHdr.dataSn         = bswap_32(sn);
        /*
        ** update number of read bytes on this connection
        */
        pConn->totalReads += dataLen;
        if (length != 0)
        {
            iscsi_updateSend(pConn, NULL, (char*)&pHdr, ISCSI_HDR_LEN, (char *)pTmpBuff, dataLen, dataInDone, iscsi_crRelILT, piSCSI_ILT->iscsi_def.lun);
        }
        else
        {
            iscsi_updateSend(pConn, pSecILT, (char*)&pHdr, addr, (char *)pTmpBuff, dataLen, dataInDone, iscsi_crDataInResponse, piSCSI_ILT->iscsi_def.lun);
        }

        /*
        ** update sn and offset and see if we are done sending read data
        */
        sn += 1;
        offset += dataLen;
    } while (length != 0);
    /*
    ** store the data-in count in the rsvd1 of the command
    ** as we need to send count in the SCSI Read response
    */
    pCmd->rsvd1 = dataIncnt;

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiSendDataIn()\n");
    return XIO_SUCCESS;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiBuildNopInHdr
 **
 **              This is for building NOP IN PDU's NOP-Out/In PDUs may be
 **              utilized to synchronize the command and status ordering
 **              counters of the target and initiator
 **
 **  @param      pReqHdr    - NOPOUT request pointer
 **  @param      pRespHdr   - NOPIN response pointer
 **  @param      isResp     - is it a response
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiBuildNopInHdr(ISCSI_NOPOUT_HDR *pReqHdr, ISCSI_NOPIN_HDR *pRespHdr, UINT8 isResp)
{

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Entry: iscsiBuildNopInHdr()\n");
    /*
    ** fill in opcode
    */
    pRespHdr->opcode = TARGOP_NOP_IN;
    pRespHdr->rsvd1 = TARGOP_RESERVE_BIT;

    if (isResp == ISCSI_TARG_NOPIN_RESP)
    {
        /*
        ** copy the initTaskTag in respons
        */
        pRespHdr->initTaskTag = pReqHdr->initTaskTag;
        /*
        ** set ttt to 0xffffffff
        */
        pRespHdr->ttt = ISCSI_HDR_RSVDFF;
    }
    else if (isResp == ISCSI_TARG_NOPIN_REQ)
    {
        /*
        ** this is sent as not response to nopout request
        */
        pRespHdr->ahsDataLen = XIO_ZERO;
        pRespHdr->initTaskTag = ISCSI_HDR_RSVDFF;
        /*
        ** We are sending a nopin to the initiator, set the ttt to
        ** a valid value other than reserved value.
        */
        pRespHdr->ttt = generateTTT();
        pRespHdr->lunOrRsvd = 0;
    }
    else if (isResp == ISCSI_TARG_NOPIN)
    {
        /*
        ** doesn't expect a response
        ** this is sent as not response to nopout request
        */
        pRespHdr->ahsDataLen = XIO_ZERO;
        pRespHdr->initTaskTag = ISCSI_HDR_RSVDFF;
        pRespHdr->ttt = ISCSI_HDR_RSVDFF;
    }
    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiBuildNopInHdr()\n");
    return XIO_SUCCESS;

}

/**
 ******************************************************************************
 **
 **  @brief      generateTTT
 **
 **              This is for generating a TTT
 **
 **  @param      none
 **
 **  @return     TTT
 **
 ******************************************************************************
 **/
UINT32 generateTTT(void)
{
    static  UINT32 ttt = XIO_ZERO;
    /*
    ** 0xffffffff is reserved value for TTT
    */
    if (ttt == ISCSI_TTT_MAX)
        ++ttt;
    return ++ttt;
}

/**
 ******************************************************************************
 **
 **  @brief      getScsiReadSize
 **
 **              This is to get the size of the SCSI read command
 **
 **  @param      pPdu   - iSCSI PDU
 **
 **  @return     size on success ZERO otherwise
 **
 ******************************************************************************
 **/
UINT32 getScsiReadSize(ISCSI_PDU *pPdu)
{
    /*
    ** get the scsi command from pdu
    */
    ISCSI_SCSI_CMD_HDR *pCmd = (ISCSI_SCSI_CMD_HDR *)&(pPdu->bhs);
    if (pCmd->flags & ISCSI_SCSI_RD)
    {
        if (!(pCmd->flags & ISCSI_SCSI_WR))
        {
            return bswap_32(pCmd->expDataTxLen);
        }
        if (pCmd->flags & ISCSI_SCSI_RD)
        {
            ISCSI_SCSIRD_DLEN_AHS_HDR *pAhs = (ISCSI_SCSIRD_DLEN_AHS_HDR *) pPdu->ahs;
            if (pAhs && pAhs->type == ISCSI_AHS_RD_LEN)
            {
                return bswap_32(pAhs->ahsLen);
            }
        }

    }
    return XIO_ZERO;
}

/**
 ******************************************************************************
 **
 **  @brief      getScsiWriteSize
 **
 **              This is to get the size of the SCSI write command
 **
 **  @param      pPdu   - iSCSI PDU
 **
 **  @return     size on success ZERO otherwise
 **
 ******************************************************************************
 **/
UINT32 getScsiWriteSize(ISCSI_PDU *pPdu)
{
    /*
    ** get the command from pdu and return size
    */
    ISCSI_SCSI_CMD_HDR *pCmd = (ISCSI_SCSI_CMD_HDR *)& pPdu->bhs;
    if (pCmd->flags & ISCSI_SCSI_WR)
    {
        return bswap_32(pCmd->expDataTxLen);

    }
    return XIO_ZERO;
}

/**
 ******************************************************************************
 **
 **  @brief      getScsiexpDataTxLen
 **
 **              This is to get expected data Tx length for scsi cmd
 **
 **  @param      pPdu   - iSCSI PDU
 **
 **  @return     size on success ZERO otherwise
 **
 ******************************************************************************
 **/
UINT32 getScsiexpDataTxLen(ISCSI_PDU *pPdu)
{
    /*
    ** get the command from pdu and return size
    */
    ISCSI_SCSI_CMD_HDR *pCmd = (ISCSI_SCSI_CMD_HDR *)&pPdu->bhs;
    /*
    ** return the command write size
    */
    if (pCmd->flags & ISCSI_SCSI_WR)
    {
        return bswap_32(pCmd->expDataTxLen);
    }
    else if (pCmd->flags & ISCSI_SCSI_RD)
    {
        return bswap_32(pCmd->expDataTxLen);
    }
    return XIO_ZERO;
}
/**
 ******************************************************************************
 **
 **  @brief      iscsiBuildR2TAndSend
 **
 **              This for building and sending R2Ts
 **
 **  @param      pConn      - Connection pointer
 **  @param      r2tILT     - ILT pointer
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 iscsiBuildR2TAndSend(CONNECTION *pConn, ILT *r2tILT, ISCSI_SCSI_CMD_HDR *pCmd)
{
    ISCSI_R2T_HDR pHdr;
    ILT* piSCSI_ILT = NULL;

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Entry: iscsiBuildR2TAndSend()\n");

    memset(&pHdr,XIO_ZERO,ISCSI_HDR_LEN);

    /*
    ** build the r2t header
    */
    pHdr.opcode     = TARGOP_R2T;
    pHdr.ahsDataLen = XIO_ZERO;
    pHdr.flags      = F_BIT;
    pHdr.itt        = bswap_32(r2tILT->secondary_ilt.ITT);
    pHdr.r2tSn      = bswap_32(r2tILT->secondary_ilt.r2tCount - 1);
    pHdr.buffOffset = bswap_32(r2tILT->secondary_ilt.r2toffset); /* this is zero for the first r2t */
    pHdr.dataTxLen  = bswap_32(r2tILT->secondary_ilt.dataTxLen);
    pHdr.ttt        = bswap_32(r2tILT->secondary_ilt.TTT);
    pHdr.LUN        = pCmd->LUN;
    pHdr.expCmdSn   = bswap_32(pConn->pSession->expCmdSN);
    pHdr.maxCmdSn   = bswap_32(pConn->pSession->maxCmdSN);
    pHdr.statSn     = bswap_32(pConn->statSN);

    piSCSI_ILT = (ILT *)r2tILT->secondary_ilt.secILT; /* secondary ILT */
    piSCSI_ILT = (ILT *)(((ILT *)(piSCSI_ILT->misc))->ilt_normal.w5);
    piSCSI_ILT = (ILT *)(piSCSI_ILT->misc);

    iscsi_updateSend(pConn, NULL, (char*)&pHdr, ISCSI_HDR_LEN, NULL, 0, 0, iscsi_crSendR2T, piSCSI_ILT->iscsi_def.lun);

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiBuildR2TAndSend()\n");
    return XIO_SUCCESS;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiCr
 **
 **              This is called after completing command for cleanin up
 **              ILT created at iSCSI level
 **
 **  @param      pILT   - ILT pointer
 **
 **  @return     none
 **
 ******************************************************************************
 **/
void iscsiCr(ILT *pILT)
{
    /*
    **A fix for cmd SN problem. For TMF and Scsi cmd ILT is allocated and PDU is malloced
    ** so it must be freed.
    */
    if (pILT->iscsi_def.pPdu)
    {
        if (pILT->iscsi_def.pPdu->extCdb)
        {
            s_Free(pILT->iscsi_def.pPdu->extCdb,pILT->iscsi_def.pPdu->cdbLen, __FILE__, __LINE__);
        }
        s_Free(pILT->iscsi_def.pPdu,sizeof(ISCSI_PDU), __FILE__, __LINE__);
    }

#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
    put_ilt(pILT);

   /*
   **  if the session state is Free and there is no outstanding command then we can free the session here
   */
   return;
}

/**
 ******************************************************************************
 **
 **  @brief      iscsi_cleanupILT
 **
 **              This function handles cleanup of ILTs in connection when
 **              MAX_LUN is passed. Otherwise it cleansup nodes only matching
 **              LUN
 **
 **  @param      pConn - connection pointer
 **  @param      lunn - LUN value
 **
 **  @return     none
 **
 ******************************************************************************
 **/
void iscsi_cleanupILT(CONNECTION* pConn, UINT16 lun)
{
    UINT8  result = 0;
    struct hash_node *hash_start = NULL;
    struct hash_node *curr_node = NULL;
    struct hash_node *tmp_node = NULL;
    ILT* pILT       = NULL;
    ILT* r2tILT     = NULL;
    ILT* pXL        = NULL;
    ILT* piSCSI_ILT = NULL;
    ILT* pPriILT    = NULL;
    void (*cb)(ILT*)= NULL;
    ILT* pTmpILT    = NULL;
    UINT16      i   = 0;

    for (i = 0; i < HASH_TABLE_SIZE; i++)
    {
        hash_start = pConn->hashTable[i];
        for (curr_node = hash_start;curr_node != NULL;curr_node = tmp_node)
        {
            tmp_node = curr_node->pNext;
            r2tILT = (ILT*)curr_node->rec;
            pXL = (ILT*)r2tILT->secondary_ilt.secILT;
            pXL = (ILT*)pXL->misc;
            pPriILT = (ILT*)pXL->ilt_normal.w5;
            piSCSI_ILT = ((ILT *)(pPriILT->misc));
            if ((lun == ISCSI_MAX_LUN) || (lun == piSCSI_ILT->iscsi_def.lun))
            {
                KernelDispatch(1, pXL, NULL, 0);
                result = hash_delete(pConn->hashTable,r2tILT->secondary_ilt.ITT,r2tILT->secondary_ilt.TTT);
                if (result == XIO_FAILURE)
                {
                    fprintf(stderr,"ISCSI_DEBUG: iscsi_cleanupILT() hash_delete() failed\n");
                }
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)r2tILT);
#endif /* M4_DEBUG_ILT */
                put_ilt(r2tILT);
            }
        }
    }

    pILT = pConn->pSendIltHead;
    while (pILT != NULL)
    {
        pTmpILT = pILT->fthd;

        if ((lun == ISCSI_MAX_LUN) || (lun == (pILT + 1)->ilt_normal.w7))
        {
            cb = pILT->cr;
            iscsi_unthreadILT(pConn, pILT);
            if (pILT->cr != NULL)
            {
                pILT->pi_pdu.flag |= ISCSI_SEND_ERR;
                (*cb)(pILT);
            }
        }
        pILT = pTmpILT;
    }

    if ((pConn->pSendIltHead == NULL)
           || (pConn->pSendIltTail == NULL)
           || (pConn->SendCount == 0))
    {
        pConn->pSendIltHead = NULL;
        pConn->pSendIltTail = NULL;
        pConn->SendCount   = 0;
    }
}

/**
 ******************************************************************************
 **
 **  @brief      iscsi_send
 **
 **              This is for sending messae to queue
 **
 **  @param      pTPD      - Transport descriptor pointer
 **
 **  @return     Status - XIO_CONN_CLOSE -- if connection is gone.
 **
 ******************************************************************************
 **/

int iscsi_send (ISCSI_TPD *pTPD)
{
    CONNECTION* pConn;
    int retVal = 0;
    int (*cb)(ILT*);
    ILT *pILT = NULL;

    if (pTPD == NULL)
    {
        return(XIO_CONN_CLOSE);
    }

    pConn = pTPD->pConn;

    if (pConn == NULL
        || pConn->send_state == ISCSI_PARTIAL_SEND)
    {
        return(XIO_SUCCESS);
    }

    while (pConn->SendCount)
    {
        pILT = pConn->pSendIltHead;
        if (pILT == NULL)
        {
            fprintf (stderr, "**************** pILT is NULL **************\n");
            return(XIO_SUCCESS);
        }
        if ((retVal = tsl_send(pILT)) > 0)
        {
            cb = pILT->cr;
            iscsi_unthreadILT(pConn,pILT);
            if (cb != NULL)
            {
                retVal = (*cb)(pILT);
                if (retVal == ISCSI_CONNCLOSE)
                {
                    return(XIO_CONN_CLOSE);
                }
            }
        }
        else if (retVal == TSL_ERROR_ON_CONN)
        {
            iscsi_cleanupILT(pConn,ISCSI_MAX_LUN);
            break;
        }
        else
        {
            pConn->send_state = ISCSI_PARTIAL_SEND;
            break;
        }
    }

    /*
     * If we have sent everything we have, clear the output event bit.
     */

    if (pConn->SendCount == 0)
    {
        if (pTPD->write_event_enabled)
        {
            tsl_remove_output_event (pTPD);
        }
    }
    else
    {
        if (!pTPD->write_event_enabled)
        {
            tsl_add_output_event (pTPD);
        }
    }
    return(XIO_SUCCESS);
}

/**
 ******************************************************************************
 **
 **  @brief      iscsiDoChap
 **
 **              This is for generating CHAP cheallenge and processing CHAP
 **              response received.
 **
 **  @param      pConn  - connection pointer
 **  @param      buff   - buffer
 **  @param      validLen  - valid length pointer
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT32 iscsiDoChap(CONNECTION *pConn,UINT8* buff, INT32 *validLen)
{
    UINT32   result              = 0;
    UINT32  length          = XIO_ZERO;     /* for data segment length */
    UINT32 status = 0;
    TGD* pTGD= NULL;

    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Entry:iscsiDoChap()\n");
    if (NULL == pConn)
    {
        fprintf(stderr,"NULL pointer passed to function\n");
        result = STATUS_CLASS_TARGERR << 16;
        result |= STATUS_DETAIL_ERR;
        return result;
    }
    memset(respBuff, XIO_ZERO, BUFF_LEN);

    /*
    ** need to negotiate for CHAP check whether chap is started or not
    */
    if (!pConn->cc)
    {
        length = pConn->recvLen;
        /*
        ** CHAP is not started parse CHAP_A for algorithm, if found then respond with algo
        */
        status = iscsiProcTxtReq(pConn->recvBuff, respBuff, &length, pConn->pSession, pConn);
        if (!status)
        {
            result = STATUS_CLASS_INIERR << 16;
            result |= 0;
            printf("CHAP_A failed\n");
        }
        else if (pConn->params.chap_A != 0)
        {
            fprintf(stderr,"Negotiating for CHAP: sending CHAP challenge\n");
            /*
            ** fill chap context structure with algo, name & id
            */
            pConn->cc = chap_create_context_st();
            if (!pConn->cc)
            {
                fprintf(stderr,"Malloc failed in CHAP module\n");
                result = STATUS_CLASS_TARGERR << 16;
                result |= STATUS_DETAIL_ERR;
            }
            else
            {
                /*
                ** ID is generated at CHAP, pass algorithm and name, name len
                */
                pConn->cc->tid       = pConn->pTPD->tid;
                pConn->cc->algorithm = AL_CHAP_WITH_MD5;
                pConn->cc->name_len  = XIO_ZERO;
                memset(pConn->cc->name,XIO_ZERO,CHAP_MAX_NAME_LEN);
                /*
                ** call CHAP for creating a CHAP request as required by iSCSI
                */
                if (XIO_SUCCESS != (UINT8)chap_create_chal (pConn->cc, buff,validLen,CHAP_ENCODING_BASE))
                {
                    /*
                    ** Chap challenge create failed
                    */
                    fprintf(stderr,"chap_create_chal() failed\n");
                    result = STATUS_CLASS_TARGERR << 16;
                    result |= STATUS_DETAIL_ERR;
                }
            }
        }
        else
        {
            /* Invalid algo, so send login reject */
            fprintf(stderr,"Recvd Invalid Chap Algorithm\n");
            result = STATUS_CLASS_INIERR << 16;
            result |= STATUS_DETAIL_ERR;
        }
    }
    else
    {
        UINT8 tmpChapName[256];
        /*
        ** got chap response
        */
        length = pConn->recvLen;
        /*
        ** extract CHAP_R value from the response and fill in pConn->cc->resp_recvd_encoded
        ** also update the length of it in len field
        */
        status = iscsiProcTxtReq(pConn->recvBuff, buff, &length, pConn->pSession, pConn);
        if (!status)
        {
            result = STATUS_CLASS_INIERR << 16;
            result |= 0;
            printf("CHAP_R failed\n");
        }
        else if (!chap_is_valid_resp(pConn->cc))
        {
            /*
            ** validating response failed
            */
            char tmp_buff[254] = {0};
            sprintf(tmp_buff, "CHAP Negotiation Failed for %s", pConn->pSession->params.initiatorName);
            iscsi_dprintf(tmp_buff);

            result = STATUS_CLASS_INIERR << 16;
            result |= STATUS_DETAIL_AUTH_FAIL;
        }
        else
        {
            char tmp_buff[254] = {0};
            sprintf(tmp_buff, "CHAP Negotiation SUCCESSFUL for %s", pConn->pSession->params.initiatorName);
            iscsi_dprintf(tmp_buff);

            /*
            ** Find if we have got challenge from initiator
            */
            if (pConn->cc->chal_recvd_len > 0)
            {
                getiscsiNameForTarget(tmpChapName,pConn->pTPD->tid);
                chap_create_response(pConn->cc,buff,validLen,CHAP_ENCODING_BASE,tmpChapName);
            }
            else
            {
                /*
                ** Find out if Chap is mandatory for 2 way
                */
                pTGD = T_tgdindx[pConn->cc->tid];

                if (pTGD && pTGD->itgd->authMethod == 0x81)
                {
                    UINT8 secret1[32] = {0};
                    UINT8 secret2[32] = {0};
                    /*
                    ** authMethod is set as mandatory
                    ** Find out if the initiator secret has been set for this user name
                    */
                    chapGetSecret(pConn->cc->tid,pConn->cc->name,secret1,secret2);
                    if (strlen((char *)secret2) > 0 )
                    {
                        /*
                        ** log message initiator must perform mutual chap
                        */
                        sprintf(tmp_buff, "Mutual CHAP Mandatory for %s on Target %d ",
                                          pConn->pSession->params.initiatorName, pConn->pTPD->tid);
                        iscsi_dprintf(tmp_buff);
                        result = STATUS_CLASS_INIERR << 16;
                        result |= STATUS_DETAIL_AUTHORIZATION_FAIL;
                    }

                }

                *validLen = 0;
            }
        }
    }
    TRACE_ENTRY_EXIT(FU_ENABLE,"Function Exit: iscsiDoChap()\n");
    return result;
}
/**
 ******************************************************************************
 **
 **  @brief     iscsi_updateSend
 **
 **              This is for queuing PDU(s) to be sent out
 **
 **  @param      pConn     - connection pointer
 **  @param      pSecILT   - ILT ptr
 **  @param      pHdr      - Header pointer
 **  @param      hdrLen    - Header length
 **  @param      pData     - Data pointer
 **  @param      dataLen   - Data length
 **  @param      dataDone  - Data Done flag
 **  @param      cr        - ILT completion routine pointer
 **
 **  @return     None
 **
 ******************************************************************************
 **/
void iscsi_updateSend(CONNECTION *pConn, ILT* pSecILT,
                      char* pHdr, UINT32 hdrLen, char* pData, UINT32 dataLen,
                      UINT8 dataDone, int (*cr)(ILT*), UINT16 lun)
{
    ILT         *pConnILT = NULL, *pTmpILT = NULL;
    SGL         *pSGL;
    SGL_DESC    *pSGL_DESC;
    UINT8       opcode;
    INT32       pad =0;
    UINT32      length = ISCSI_HDR_LEN;
    UINT32      digest = 0;

    if (pSecILT != NULL)
    {
        pConnILT = pSecILT + 1;
        memset(pConnILT, 0 ,sizeof(ILT));
        pConnILT->pi_pdu.w0 = (UINT32)pSecILT->misc;       /* ILT */
    }
    else
    {
        pConnILT = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pConnILT);
#endif /* M4_DEBUG_ILT */
        memset(pConnILT, 0 ,sizeof(ILT));
        pConnILT->pi_pdu.w0 = (UINT32)0x0;
    }

    opcode = GET_ISCSI_OPCODE(pHdr[0]);
    if (isDigestCheckValid(pConn->pTPD)== XIO_SUCCESS &&
            stringCompare(pConn->params.headerDigest.strval,(UINT8*) "CRC32C") == 0
            && opcode != TARGOP_LOGIN_RESP)
    {
        length += DIGEST_LENGTH;
    }

    if ((opcode != TARGOP_SCSI_DATA_IN) && (dataLen != 0) && (pData != NULL))
    {
        length += dataLen;

        pad = (-dataLen) & 3;
        length += pad;
    }

    pSGL = (SGL*)m_asglbuf(length);
    pSGL->scnt = 1;
    pSGL_DESC = (SGL_DESC*)((char*)pSGL + 8);

    memcpy((char*)pSGL_DESC->addr, pHdr, ISCSI_HDR_LEN);
    length = ISCSI_HDR_LEN;
    AppendHeaderDigest(pConn->pTPD, (char*)pSGL_DESC->addr, (int *)&length);

    pTmpILT = pConnILT + 1;
    memset(pTmpILT, 0 ,sizeof(ILT));
    pTmpILT->ilt_normal.w0 = (UINT32)pSGL_DESC->addr;
    pTmpILT->ilt_normal.w1 = (UINT32)length;
    pTmpILT->ilt_normal.w7 = lun;

    if (dataDone == SENSE_DATA_PRESENT)
    {
        /*
        ** update SENSE HDR at iSCSI level
        */
        ISCSI_SCSI_SENSE_SEG snsHdr;
        memset(&snsHdr, XIO_ZERO,ISCSI_SCSI_SENSE_SEG_SIZE);
        /*
        ** update sense length and do byte swap
        */
        snsHdr.snsLen = bswap_16(dataLen - SENSE_DATA_LEN_FLD);
        xioMemcpy(&(snsHdr.snsData),pData,(dataLen - SENSE_DATA_LEN_FLD));
        /*
        ** copy required sense bytes to SGL that is allocated
        ** And update total length in bytes to be sent out.
        */
        xioMemcpy((char*)pSGL_DESC->addr + length, &snsHdr, dataLen);
        length += dataLen;
        pad = (-dataLen) & 3;
        if (pad)
        {
            memset((char*)pSGL_DESC->addr + length, XIO_ZERO, pad);
        }
        length += pad;

        pTmpILT->ilt_normal.w1 = (UINT32)length;

        if (isDigestCheckValid(pConn->pTPD)== XIO_SUCCESS &&
                stringCompare(pConn->params.dataDigest.strval,(UINT8*) "CRC32C") == 0
                && opcode !=TARGOP_LOGIN_RESP)
        {
            length += DIGEST_LENGTH;
            digest = iscsi_CalculateCRC32((UINT8 *)&snsHdr, dataLen);
            pTmpILT->ilt_normal.w4 = digest;
            pTmpILT->ilt_normal.w5 = DIGEST_LENGTH;
        }

        pData    = NULL;
        dataLen  = 0;
        /*
        ** Need to call KernelDispatch after this PDU is sent out
        */
        dataDone = 1;
    }

    if ((pData != NULL) && (dataLen != 0))
    {
        if (opcode != TARGOP_SCSI_DATA_IN)
        {
            memcpy((char*)pSGL_DESC->addr + length, pData, dataLen);
            length += dataLen;
            pad = (-dataLen) & 3;
            if (pad)
            {
                memset((char*)pSGL_DESC->addr + length, 0, pad);
            }
            length += pad;
            pTmpILT->ilt_normal.w1 = (UINT32)length;
        }
        else
        {
            pConnILT->pi_pdu.dataPresent = 1;
            pad = (-dataLen) & 3;
            if (pad)
            {
                memset(pData+dataLen,0,pad);
            }
            dataLen += pad;

            pTmpILT->ilt_normal.w2 = (UINT32)pData;
            pTmpILT->ilt_normal.w3 = (UINT32)dataLen;
            length += dataLen;
        }

        if (isDigestCheckValid(pConn->pTPD)== XIO_SUCCESS &&
                stringCompare(pConn->params.dataDigest.strval,(UINT8*) "CRC32C") == 0
                && opcode !=TARGOP_LOGIN_RESP)
        {
            length += DIGEST_LENGTH;
            digest = iscsi_CalculateCRC32((UINT8 *)pData, dataLen);
            pTmpILT->ilt_normal.w4 = digest;
            pTmpILT->ilt_normal.w5 = DIGEST_LENGTH;
        }

        if (opcode != TARGOP_SCSI_DATA_IN)
        {
            pData = NULL;
        }
    }

    if (hdrLen != ISCSI_HDR_LEN)
    {
        pTmpILT->ilt_normal.w6 = hdrLen;
    }

    pConn->numPduSent += 1;
    pConnILT->cr = cr;                          /* CallBack function*/
    pConnILT->pi_pdu.w1 = (UINT32)pConn->pTPD;         /* TPD */
    pConnILT->pi_pdu.w2 = (UINT32)pSGL;                /* SGL */
    pConnILT->pi_pdu.w3 = (UINT32)pData;
    pConnILT->pi_pdu.dataDone  = dataDone;
    pConnILT->pi_pdu.len       = length;
    pConnILT->pi_pdu.flag      = 0;                   /* flag represents success(0) or failure(0x01) */
    pConnILT->fthd = NULL;
    pConnILT->bthd = NULL;

    if (pConn->pSendIltHead == NULL)
    {
        pConn->pSendIltHead = pConnILT;
        pConn->pSendIltTail = pConnILT;
        pConn->SendCount   = 0;
    }
    else
    {
        pConn->pSendIltTail->fthd = pConnILT;
        pConnILT->bthd = pConn->pSendIltTail;
        pConn->pSendIltTail = pConnILT;
    }

    /*
     * Update the count of queued messages, and set the indicator that we just added to
     * the queue. Also make sure the events are enabled for output.
     */

    pConn->SendCount++;
    if (pConn->pTPD == NULL)
    {
        fprintf (stderr, "**************** pConn->pTPD is NULL **************\n");
        return;
    }

    /*
     * If we are currently processing an event for this TPD (send_indicator is
     * zero or one), then set it to one to force processing the output. If not,
     * enable the output events for this TPD to generate an event which will cause
     * the message to be sent.
     */

    if (pConn->pTPD->send_indicator < 2)
    {
        pConn->pTPD->send_indicator = 1;
    }
    else
    {
        tsl_add_output_event (pConn->pTPD);
    }
}

/**
 ******************************************************************************
 **
 **  @brief     iscsi_unthreadILT
 **
 **              This is for unthreading ILT(s) from send queue
 **
 **  @param      pConn     - connection pointer
 **  @param      pILT      - ILT ptr
 **
 **  @return     None
 **
 ******************************************************************************
 **/
void iscsi_unthreadILT(CONNECTION *pConn, ILT *pILT)
{
    pConn->SendCount--;
    if (pConn->SendCount == 0)
    {
        pConn->pSendIltTail = NULL;
        pConn->pSendIltHead = NULL;
    }
    else if (pILT == pConn->pSendIltHead)
    {
        pConn->pSendIltHead = pILT->fthd;
        pConn->pSendIltHead->bthd = NULL;
    }
    else
    {
        pILT->bthd->fthd = pILT->fthd;
        if (pILT->fthd)
            pILT->fthd->bthd = pILT->bthd;
        else
            pConn->pSendIltTail = pILT->bthd;
    }
}
/**
 ******************************************************************************
 **
 **  @brief     iscsi_ins_conn
 **
 **              This is for inserting a connection in the list
 **
 **  @param      pStart    - connection head pointer
 **  @param      pIns      - connection to be inseted into list
 **
 **  @return     None
 **
 ******************************************************************************
 **/
void iscsi_ins_conn(CONNECTION *pStart, CONNECTION *pIns)
{
    CONNECTION *curr_conn = NULL;
    CONNECTION *prev_conn = NULL;

    if (iscsi_lookup_conn(pStart,pIns) != NULL)
    {
        fprintf(stderr,"ISCSI_DEBUG: Connection already exists %x\n",(UINT32)pIns);
    }
    else
    {
        for (curr_conn = pStart; curr_conn != NULL; prev_conn = curr_conn,curr_conn = curr_conn->pNext);
        prev_conn->pNext = pIns;
    }
}

/**
 ******************************************************************************
 **
 **  @brief     iscsi_del_conn
 **
 **              This is for deleting a connection from the list
 **
 **  @param      pStart    - connection head pointer
 **  @param      pDel      - connection to be deleted
 **
 **  @return     None
 **
 ******************************************************************************
 **/
void iscsi_del_conn(CONNECTION *pStart, CONNECTION *pDel)
{
    CONNECTION *curr_conn = NULL;
    CONNECTION *prev_conn = NULL;

    for (curr_conn = pStart; curr_conn != NULL; prev_conn = curr_conn,curr_conn = curr_conn->pNext)
    {
        if (curr_conn == pDel)
        {
            /*
             ** found node
             */
            if (prev_conn == NULL)
            {
                /*
                ** first node on chain, remove and free the node
                */
                pStart = curr_conn->pNext;
            }
            else
            {
                /*
                ** remove the node
                */
                fprintf(stderr,"ISCSI_DEBUG: This can not happen conn_del(), We support single connection\n");
                prev_conn->pNext = curr_conn->pNext;
            }
            /*
            ** free the node
            */
            s_Free(curr_conn,sizeof(CONNECTION), __FILE__, __LINE__);
            return;
        }
    }
    fprintf(stderr,"ISCSI_DEBUG: connection delete failed, no matching ssn\n");
}

/**
 ******************************************************************************
 **
 **  @brief     iscsi_lookup_conn
 **
 **              This is for checking whether a connection is already present in
 **              the list
 **
 **  @param      pStart    - connection head pointer
 **  @param      pLookup  - connection to be matched
 **
 **  @return     CONNECTION pointer if found NULL otherwise
 **
 ******************************************************************************
 **/
CONNECTION *iscsi_lookup_conn(CONNECTION *pStart, CONNECTION *pLookup)
{
    CONNECTION *curr_conn = NULL;

    for (curr_conn = pStart;curr_conn != NULL;curr_conn = curr_conn->pNext)
    {
        /*
         ** return ssn matching
         */
        if (curr_conn == pLookup)
        {
            return (curr_conn);
        }
    }
    return NULL;
}
/**
 ******************************************************************************
 **
 **  @brief     iscsi_ins_ssn
 **
 **             This is for inserting a session in the list
 **
 **  @param      pStart    - session head pointer
 **  @param      pIns      - session to be inserted
 **
 **  @return     None
 **
 ******************************************************************************
 **/

void iscsi_ins_ssn(SESSION *pStart, SESSION *pIns)
{
    SESSION *curr_ssn = NULL;
    SESSION *prev_ssn = NULL;

    if (iscsi_lookup_ssn(pStart,pIns) != NULL)
    {
        fprintf(stderr,"ISCSI_DEBUG: Session already exists %x\n",(UINT32)pIns);
    }
    else
    {
        for (curr_ssn = pStart; curr_ssn != NULL; prev_ssn = curr_ssn,curr_ssn = curr_ssn->pNext);
        prev_ssn->pNext = pIns;
    }
}
/**
 ******************************************************************************
 **
 **  @brief     iscsi_del_ssn
 **
 **             This is for deleting a session from the list
 **
 **  @param      head      - session head pointer
 **  @param      pDel      - session to be deleted
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/

UINT8 iscsi_del_ssn(UINT32 *head, SESSION *pDel)
{
    UINT8   result =  XIO_FAILURE;
    SESSION *curr_ssn = NULL;
    SESSION *prev_ssn = NULL;
    SESSION *pStart = (SESSION*)head;

    for (curr_ssn = pStart; curr_ssn != NULL; prev_ssn = curr_ssn,curr_ssn = curr_ssn->pNext)
    {
        if (curr_ssn == pDel)
        {
            /*
            ** found node
            */
            if (prev_ssn == NULL)
            {
                /*
                ** first node on chain, remove and free the node
                */
                head = (UINT32*)curr_ssn->pNext;
            }
            else
            {
                /*
                ** remove the node
                */
                prev_ssn->pNext = curr_ssn->pNext;
            }
            /*
            ** free the node
            */
            s_Free(curr_ssn, sizeof(SESSION), __FILE__, __LINE__);
            result = XIO_SUCCESS;
            break;
        }
    }
    return result;
}
/**
 ******************************************************************************
 **
 **  @brief     iscsi_lookup_ssn
 **
 **              This is for checking whether a session is already present in
 **              the list
 **
 **  @param      pStart    - session head pointer
 **  @param      pLookup  - session to be matched
 **
 **  @return     SESSION pointer if found NULL otherwise
 **
 ******************************************************************************
 **/
SESSION *iscsi_lookup_ssn(SESSION *pStart, SESSION *pLookup)
{
    SESSION *curr_ssn = NULL;

    for (curr_ssn = pStart;curr_ssn != NULL; curr_ssn = curr_ssn->pNext)
    {
        /*
         ** return ssn matching
         */
        if (curr_ssn == pLookup)
        {
            return (curr_ssn);
        }
    }
    return NULL;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
