/* $Id: iscsi_tmf.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
 ******************************************************************************
 **
 **  @file       iscsi_tmf.h
 **
 **  @brief      iscsi Task managemtn functions header file
 **
 **  Copyright (c) 2005-2010 XIOtech Corporation.  All rights reserved.
 **
 ******************************************************************************
 **/

#ifndef __ISCSI_TMF_h
#define __ISCSI_TMF_h
#include "iscsi_pdu.h"

#define GET_TMF_FUNCTION(x) ((x) & 0x7f)

/*
* @name        iscsiProcTMFPdu
* @params     ISCSI_TMF_HDR *pReq,
*            CONNECTION *pConn,
*            bool OutOrderPDU
* @brief    The function is main interface for TMF functions. The function responsibility is to
*            process the command and send response. This function assumes that the given pdu
*            is in order
*            In case of error it will return error code which calling function need to take care
* @return
*/

extern INT32 iscsiProcTMFPdu(ISCSI_TMF_REQ_HDR *pReq,CONNECTION *pConn,ISCSI_PDU *pPdu);
/*
* @name        CreateTMFResponseAndSend
* @params     CONNECTION *pConn,
*            UINT32 itt
*            UINT32 rtt
*            UINT8 status

* @brief    The function creates a TMF Repsonse PDU and send it on connection
*            Error codes are same as returned by tsl_send_data
* @return
*/

extern INT32 CreateTMFResponseAndSend(CONNECTION *pConn,ISCSI_TMF_REQ_HDR *pReq, UINT8 response, int (*cr)(ILT*));

/*
Following three function are interface to handle Abort task list.
Any module should always use these interfaces to make change in abortTaskList.

Before sending a response pdu one should check if the task has been already aborted.
    if(IsTaskAborted(pConn,rtt))
    {
        //don't send the pdu
        //free memory
        RemoveFromAbortedTaskList(pConn,rtt);

    }else
    {
        //send pdu
    }
*/
extern void RemovePduFromCommandQ(SESSION *pSession, INT32 position);
extern INT32 AllocatePlaceInCommandQ(SESSION *pSession,UINT32 cmdSN);

extern void InitializeCommandQueue(SESSION *pSession);
extern void ChangeCmdStateForLUNInCommandQ(SESSION *pSession,UINT64 lun,UINT32 refCmdSN);
extern INT32  getPositionInCommandQForCmdSN(SESSION *pSession,UINT32 CmdSN);

#endif  /* __ISCSI_TMF_h */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
