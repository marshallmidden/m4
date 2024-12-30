/* $Id: iscsi_tmf.c 126080 2010-02-03 22:59:28Z mdr $ */
/**
******************************************************************************
**
**  @file       iscsi_tmf.c
**
**  @brief      iSCSI ErrorRecovery functions
**
**  This file contains all functions to support Task Manangement PDU
**  handling in iSCSI.
**
**  Copyright (c) 2005-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>
#include <sys/time.h>

#include "XIO_Types.h"
#include "XIO_Macros.h"
#include "iscsi_common.h"
#include "iscsi_pdu.h"
#include "iscsi_digest.h"
#include "iscsi_tsl.h"
#include "fsl.h"

#include "iscsi_tmf.h"

#include "MR_Defs.h"                    /* MRP definitions                  */
#include "ilt.h"
#include "sgl.h"
#include "pm.h"
#include "isp.h"
#include "mem_pool.h"
#include "CT_defines.h"

#define DATA_LENGTH_NOT_ZERO -10
#define XIO_MEMORY_FAILURE   -11

static INT32 IsLunExistingForSession(CONNECTION *pConn, UINT64 lun);
static INT32 IsSupportedTMF(UINT8 function);

/*
* @name        iscsiProcTMFPdu
* @params     ISCSI_TMF_HDR *pReq,
*            CONNECTION *pConn,
* @brief    The function is main interface for TMF functions. The function responsibility is to
*            process the command and send response. This function assumes that the given pdu
*            is in order
*            In case of error it will return error code which calling function need to take care
* @return
*/

INT32 iscsiProcTMFPdu(ISCSI_TMF_REQ_HDR *pReq,CONNECTION *pConn,ISCSI_PDU *pPdu)
{
    char logbuf[300] = {0};
    TMF_REQUEST    function;
    ILT *pILT = NULL,*pPrevILT= NULL;

    if ((pConn == NULL) || (pReq == NULL) || (pConn->state != CONNST_LOGGED_IN))
    {
        fprintf(stderr,"iscsiProcTMFPdu: Invalid Input Params\n");
        return -1;
    }

    /*
    ** check for all kind of format errors
    ** This code should be moved to IsFormatError
    */
    if(pReq->ahsDataLen != 0)
    {
        return DATA_LENGTH_NOT_ZERO;
    }

    function = GET_TMF_FUNCTION(pReq->function);
    /*
    ** some function may not be supported
    */
    if(IsSupportedTMF(function)== XIO_FAILURE)
    {
        sprintf(logbuf,"Task Management Function is Not supported");
        iscsi_dprintf(logbuf);
        fprintf(stderr, "%s\n",logbuf);
        CreateTMFResponseAndSend(pConn, pReq, TMF_FUNCTION_NOT_SUPPORTED, iscsi_crRelILT);
        return XIO_SUCCESS;
    }

    if(function != TMF_WARM_RESET && function != TMF_COLD_RESET && function != TMF_TASK_REASSIGN)
    {
        /*
        ** First find out if the lun is existing or not
        ** other than these 3 function all TMF requires LUN
        */
        if(IsLunExistingForSession(pConn, iscsiGetLun((UINT8*)&(pReq->LUN)))== XIO_FAILURE)
        {
            /*
            ** ttt is at the place of rtt in geniric header
            */
            sprintf(logbuf,"Task Management Function: LUN doesn't exist");
            iscsi_dprintf(logbuf);
            fprintf(stderr, "%s\n",logbuf);
            CreateTMFResponseAndSend(pConn, pReq, TMF_LUN_NOT_EXIST, iscsi_crRelILT);
            return XIO_SUCCESS;
        }
    }

    pILT = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */

    pPrevILT = pILT;
    pILT->iscsi_def.pPdu  = pPdu;
    pILT->iscsi_def.pConn = pConn;

    /*
    ** update TSIH & TID which is used to find corresponding IMT
    */
    pILT->iscsi_def.tsih  = pConn->pSession->tsih;
    pILT->iscsi_def.tid   = pConn->pTPD->tid;
    pILT->iscsi_def.tpgt  = pConn->pSession->params.targetPortalGroupTag;
    pILT->iscsi_def.lun   = (UINT16)iscsiGetLun((UINT8*)&(pReq->LUN));
    pILT->iscsi_def.flags = pReq->function;
    pILT->iscsi_def.cmdType = INIOP_TASK_MGMT_REQ;

    /*
    ** fill the port id
    */
    pILT->iscsi_def.portId = ispPortAssignment[(UINT16)pConn->pTPD->tid];
    pILT->iscsi_def.pSgl = NULL;
    pILT = (pILT + 1);
    pILT->cr    = NULL;
    pILT->misc  = (UINT32)pPrevILT;

    switch(function)
    {
        case TMF_ABORT_TASK:
            {
                /*    0-8 reserved 9th bit set but we have only one byte to fill*
                **   This is to abort a already issued task.
                **   Find the task in session wide command queue and abort it
                **   if the task is in command queue it means that it is not delievered to
                **   execution unit. And in that case we can safely free the resources held by
                **   the command and abort it.
                */
                sprintf(logbuf, "TMF: --ABORT TASK-- from %s on Target %d, ITT %x RTT %x",
                        pConn->pSession->params.initiatorName,
                        pConn->pTPD->tid, bswap_32(pReq->itt), bswap_32(pReq->rtt));
                pPrevILT->iscsi_def.flags = 0x80;
                break;
            }
        case TMF_ABORT_TASK_SET:
            {
                sprintf(logbuf, "TMF: --ABORT TASK SET-- from %s on Target %d, ITT %x",
                        pConn->pSession->params.initiatorName,
                        pConn->pTPD->tid, bswap_32(pReq->itt));
                pPrevILT->iscsi_def.flags = 0x02; /*0-8 reserved 9th bit set but we have only one byte to fill*/
                break;
            }
        case TMF_CLEAR_ACA:
            {
                sprintf(logbuf, "TMF: --CLEAR ACA-- from %s on Target %d, ITT %x",
                        pConn->pSession->params.initiatorName,
                        pConn->pTPD->tid, bswap_32(pReq->itt));
                pPrevILT->iscsi_def.flags = 0x40; /*0-8 reserved 14th bit set but we have only one byte to fill*/
                break;
            }
        case TMF_CLEAR_TASK_SET:
            {
                sprintf(logbuf, "TMF: --CLEAR TASK SET-- from %s on Target %d, ITT %x",
                        pConn->pSession->params.initiatorName,
                        pConn->pTPD->tid, bswap_32(pReq->itt));
                pPrevILT->iscsi_def.flags = 0x04; /*0-8 reserved 10th bit set but we have only one byte to fill*/
                break;
            }
        case TMF_LOGICAL_UNIT_RESET:
            {
                sprintf(logbuf, "TMF: --LUN RESET-- from %s on Target %d, ITT %x, LUN %d",
                         pConn->pSession->params.initiatorName,
                         pConn->pTPD->tid, bswap_32(pReq->itt),
                         (UINT16)iscsiGetLun((UINT8 *)&pReq->LUN));
                iscsi_cleanupILT(pConn,pPrevILT->iscsi_def.lun);
                pPrevILT->iscsi_def.flags = 0x10; /*0-8 reserved 12th bit set but we have only one byte to fill*/
                break;
            }
        case TMF_WARM_RESET:
            {
                sprintf(logbuf, "TMF: --TARGET WARM RESET-- from %s on Target %d ",
                                                            pConn->pSession->params.initiatorName,
                                                            pConn->pTPD->tid);
                iscsi_cleanupILT(pConn, ISCSI_MAX_LUN);
                pPrevILT->iscsi_def.flags = 0x08; /*0-8 reserved 13th bit set but we have only one byte to fill*/
                break;
            }
        case TMF_COLD_RESET:
            {
                sprintf(logbuf, "TMF: --TARGET COLD RESET-- from %s on Target %d ",
                                                            pConn->pSession->params.initiatorName,
                                                            pConn->pTPD->tid);
                iscsi_cleanupILT(pConn,ISCSI_MAX_LUN);
                pPrevILT->iscsi_def.flags = 0x20; /*0-8 reserved 13th bit set but we have only one byte to fill*/
                break;
            }
        case TMF_TASK_REASSIGN:
            {
                sprintf(logbuf, "TMF: --TASK REASSIGN-- from %s on Target %d ITT %x",
                        pConn->pSession->params.initiatorName,
                        pConn->pTPD->tid, bswap_32(pReq->itt));
                break;
            }
        default :
            {
                sprintf(logbuf, "TMF: --Unknown-- from %s on Target %d function %d ",
                        pConn->pSession->params.initiatorName,
                        pConn->pTPD->tid, function);
            }
    }
    fprintf(stderr, "%s\n",logbuf);
    iscsi_dprintf(logbuf);
    /*
    ** send it to FE layer
    */
    fsl_iscsi_scsi(pILT);

    return XIO_SUCCESS;
}
/*
* @name     CreateTMFResponseAndSend
* @params   CONNECTION *pConn,
*           UINT32 itt
*           UINT8 status

* @brief    The function creates a TMF Repsonse PDU and send it on connection
*           Error codes are same as returned by tsl_send_data
* @return
*/

INT32 CreateTMFResponseAndSend(CONNECTION *pConn, ISCSI_TMF_REQ_HDR* pReq, UINT8 response, int (*cr)(ILT*))
{
    ISCSI_TMF_RES_HDR pTmfResp;
    INT32 retVal = XIO_SUCCESS;
    UINT16 lun = 0;

    pTmfResp.opcode     = TARGOP_TASK_MGMT_RESP;
    pTmfResp.rsvd1      = 0x80;
    pTmfResp.response   = response;
    pTmfResp.rsvd2      = 0;
    pTmfResp.ahsDataLen = 0;
    pTmfResp.rsvd3      = 0;
    pTmfResp.itt        = pReq->itt;
    pTmfResp.rsvd4      = 0;
    pTmfResp.statSn     = bswap_32(pConn->statSN++);
    pTmfResp.expCmdSn   = bswap_32(pConn->pSession->expCmdSN);
    pTmfResp.maxCmdSn   = bswap_32(pConn->pSession->maxCmdSN);
    pTmfResp.rsvd5      = 0;
    pTmfResp.rsvd6      = 0;
    pTmfResp.rsvd7      = 0;

    lun   = (UINT16)iscsiGetLun((UINT8*)&(pReq->LUN));
    iscsi_updateSend (pConn, NULL, (char*)&pTmfResp, ISCSI_HDR_LEN, NULL, 0, 0, cr, lun);
    return retVal;
}

/*
* @name        IsSupportedTMF
* @params     UINT8    function
*
* @brief    The function returns XIO_SUCCESS if the function is valid and supported
*
* @return
*        XIO_SUCCESS
*        XIO_FAILURE
*/
static INT32 IsSupportedTMF(UINT8 function)
{
    /* TMF_TASK_REASSIGN not supported. That comes with error recovery level 2.
    ** when we want to support it write (function >0 && function < 9)
    */
    if((function >0 && function < 8))
    {
        return XIO_SUCCESS;
    }

    return XIO_FAILURE;
}
/*
* @name        IsLunExistingForSession
* @params     CONNECTION *pConn
* @brief    The function returns XIO_SUCCESS if the Lun is valid
*
* @return
*        XIO_SUCCESS
*        XIO_FAILURE
*/


static INT32 IsLunExistingForSession(CONNECTION *pConn UNUSED, UINT64 lun UNUSED)
{
    return XIO_SUCCESS;
}

void InitializeCommandQueue(SESSION *pSession)
{
    int i=0;

    if (pSession == NULL)
    {
        fprintf(stderr,"InitializeCommandQueue: Invalid Input Params\n");
        return;
    }

    pSession->commandQ.currentIndex = 0;
    pSession->commandQ.submittedToProc = 0;
    for (i=0; i < MAX_COMMANDS; i++)
    {
        pSession->commandQ.cmdArray[i].data = NULL;
        pSession->commandQ.cmdArray[i].state = COMMAND_EMPTY;

    }
}

void RemovePduFromCommandQ(SESSION *pSession, INT32 position)
{
    UINT32 cmdSN;
    UINT8 immediateBit;
    if(position < 0 || position >= MAX_COMMANDS )
    {
        return;
    }
    cmdSN = bswap_32(pSession->commandQ.cmdArray[position].pdu.bhs.Sn);
    immediateBit = IS_IMMBIT_SET(pSession->commandQ.cmdArray[position].pdu.bhs.opcode);
    if(pSession->commandQ.cmdArray[position].state != COMMAND_EMPTY )
    {
        pSession->commandQ.cmdArray[position].data = NULL;
        pSession->commandQ.cmdArray[position].state = COMMAND_EMPTY;
        pSession->commandQ.cmdArray[position].pdu.position = -1;
    }
}

void ChangeCmdStateForLUNInCommandQ(SESSION *pSession,UINT64 lun,UINT32 refCmdSN)
{
    INT32 i=0;
    fprintf(stderr,"Please check if LUN comparison is proper\n");
    for(i=0; i<MAX_COMMANDS; i++)
    {
        if(pSession->commandQ.cmdArray[i].state != COMMAND_EMPTY)
        {
            if( pSession->commandQ.cmdArray[i].pdu.bhs.Sn <= refCmdSN
                && pSession->commandQ.cmdArray[i].pdu.bhs.lun == lun )
            {
                pSession->commandQ.cmdArray[i].state = COMMAND_ABORTED;
            }

        }
    }
}

INT32 AllocatePlaceInCommandQ(SESSION *pSession, UINT32 cmdSN UNUSED)
{
    int i=0;
    int indexInQ=0;

    if (pSession == NULL)
    {
        fprintf(stderr,"AllocatePlaceInCommandQ: Invalid Input Params\n");
        return -1;
    }

    indexInQ = pSession->commandQ.currentIndex;
    for(i=indexInQ; i< MAX_COMMANDS; i++)
    {
        if(pSession->commandQ.cmdArray[i].state == COMMAND_EMPTY)
        {
            indexInQ = i+1;
            if(indexInQ == MAX_COMMANDS)
            {
                indexInQ = 0;
            }
            pSession->commandQ.currentIndex = indexInQ;
            pSession->commandQ.cmdArray[i].state = COMMAND_ARRIVED;
            pSession->commandQ.cmdArray[i].pdu.position = i;
            return i;
        }
    }
    for(i = 0; i< indexInQ-1; i++)
    {
        if(pSession->commandQ.cmdArray[i].state == COMMAND_EMPTY)
        {
            indexInQ = i+1;
            pSession->commandQ.currentIndex = indexInQ;
            pSession->commandQ.cmdArray[i].state = COMMAND_ARRIVED;
            pSession->commandQ.cmdArray[i].pdu.position = i;
            return i;
        }
    }
    return -1;
}

INT32  getPositionInCommandQForCmdSN(SESSION *pSession UNUSED,UINT32 CmdSN UNUSED)
{
   printf("getPositionFromCommandQ TBD\n");
   return -1;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
