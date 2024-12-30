/* $Id: isns.c 144139 2010-07-14 19:46:01Z m4 $ */
/**
******************************************************************************
**
**  @file       isns.c
**
**  @brief      iSNS Protocol functionality implementation
**
**  Copyright (c) 2006-2010 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <linux/sockios.h>
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Types.h"
#include "XIO_Std.h"
#include "iscsi_common.h"
#include "iscsi_pdu.h"
#include "iscsi_tsl.h"
#include "target.h"
#include "ecodes.h"
#include "icl.h"
#include "ficb.h"
#include "def.h"
#include "LOG_Defs.h"
#include "isp.h"
#include "isns.h"

#ifdef HISTORY_KEEP
extern void CT_history_pcb(const char *str, UINT32 pcb);
#endif

extern ISD gISD;
extern TGD *T_tgdindx[MAX_TARGETS];
extern unsigned long CT_fork_tmp;
extern void CT_LC_t_iSNS(void *);

extern void tsl_cleanupEvents(void * pIDD);
extern void convertIntoReadableData(UINT64 nodeName,UINT8 *readableFormat);

UINT32 isns_retrieveIP(ISNS_RSP *pRsp);
INT32 isns_txrx(UINT32 ip, UINT16 port, UINT32 flags, ISNS_REQ *pReq, ISNS_RSP *pRsp);
UINT32 isns_build_req(UINT16 type, UINT16 tid, ISNS_REQ *pReq);
void isnsCB(UINT32 events, void *pRef);
void t_iSNS(UINT32 a, UINT32 b, UINT32 indx);

/**
******************************************************************************
**
**  @brief      iSNS_Init - initializes the structures to zero
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void iSNS_Init(void)
{
    memset((void *)&gISD, 0, sizeof(ISD));
}/* iSNS_Init */

/**
******************************************************************************
**
**  @brief      DEF_iSNSConfig
**              This starts a task which registers all targets to the isns server
**
**  @param      MR_PKT *pMRP
**
**  @return     UINT8       DEOK or error
**
******************************************************************************
**/
UINT8 DEF_iSNSConfig(MR_PKT *pMRP)
{
    UINT32 i = 0;
    UINT32 gflags = 0x0;
    UINT32 ip     = 0x0;
    UINT32 sflags = 0x0;
    UINT16 port   = 0x0;
    MRSETISNSINFO_REQ *pReq;
    MRSETISNSINFO_RSP *pRsp;

    /*
    ** Get pointer to Parm block address
    */
    pReq = (MRSETISNSINFO_REQ *)(pMRP->pReq);
    pRsp = (MRSETISNSINFO_RSP *)(pMRP->pRsp);

    if(iscsimap != 0x0)
    {
        gflags      = gISD.gflags & 0x7fffffff;
        gISD.gflags = pReq->flags;

        for(i = 0; i < MAX_ISNS_SERVERS; i++)
        {
            ip                  = gISD.srv[i].ip;
            port                = gISD.srv[i].port;
            sflags              = gISD.srv[i].sflags;
            gISD.srv[i].ip      = pReq->serverdata[i].ip;
            gISD.srv[i].port    = pReq->serverdata[i].port;
            gISD.srv[i].sflags  = pReq->serverdata[i].flags;

            if((gISD.srv[i].port == 0x0) && (gISD.srv[i].ip != 0x0))
                gISD.srv[i].port = ISNS_PORT;

            if((gflags != (gISD.gflags & 0x7fffffff))
                || (ip != gISD.srv[i].ip)
                || (port != gISD.srv[i].port)
                || (sflags != gISD.srv[i].sflags))
            {
                struct sockaddr_in old, new;

                old.sin_family         = AF_INET;
                old.sin_port           = htons(port);
                old.sin_addr.s_addr    = ip;

                new.sin_family         = AF_INET;
                new.sin_port           = htons(port);
                new.sin_addr.s_addr    = gISD.srv[i].ip;

                fprintf(stderr, "ISNS_DEBUG: iSNS Configuration changed from [%s %s:%d (%s)] to [%s %s:%d (%s)]\n",
                                    BIT_TEST(gflags, ISNS_CF_ENABLE) ? "Enabled" : "Disabled",
                                    inet_ntoa((struct in_addr)old.sin_addr),
                                    port,
                                    BIT_TEST(sflags, ISNS_SF_UDPTCP) ? "UDP" : "TCP",
                                    BIT_TEST(gISD.gflags, ISNS_CF_ENABLE) ? "Enabled" : "Disabled",
                                    inet_ntoa((struct in_addr)new.sin_addr),
                                    gISD.srv[i].port,
                                    BIT_TEST(gISD.srv[i].sflags, ISNS_SF_UDPTCP) ? "UDP" : "TCP");
            }
            /*
             * If the iSNS task is in the process of being created, wait till it exists.
             */
            while (gISD.srv[i].pt_iSNS == (PCB *)-1)
            {
                TaskSleepMS(50);       /* Sleep for 100 ms */
            }

            /*
            ** If the iSNS task exists, wake it up - it has got work to do
            ** If not, then create the iSNS task
            */
            if(gISD.srv[i].pt_iSNS != NULL)
            {
                if (TaskGetState(gISD.srv[i].pt_iSNS) == PCB_NOT_READY)
                {
#ifdef HISTORY_KEEP
CT_history_pcb("DEF_iSNSConfig setting ready pcb", (UINT32)(gISD.srv[i].pt_iSNS));
#endif
                    TaskSetState(gISD.srv[i].pt_iSNS, PCB_READY);
                }
            }
            else if(BIT_TEST(gISD.gflags, ISNS_GF_MASTER) && (gISD.srv[i].ip != 0x0))
            {
                CT_fork_tmp = (unsigned long)"t_iSNS";
                gISD.srv[i].pt_iSNS = (PCB *)-1;    // Flag task being created.
                gISD.srv[i].pt_iSNS = (PCB *)TaskCreate3(C_label_referenced_in_i960asm(t_iSNS), 135, i);
            }
        }
    }
    pRsp->header.status = DEOK;
    return (pRsp->header.status);
}/* DEF_iSNSConfig */

/**
******************************************************************************
**
**  @brief      Handles the target configuration changes
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void iSNS_Update(void)
{
    UINT32 i;

    for(i = 0; i < MAX_ISNS_SERVERS; i++)
    {
        if(gISD.srv[i].pt_iSNS != NULL)
        {
            if (TaskGetState(gISD.srv[i].pt_iSNS) == PCB_NOT_READY)
            {
#ifdef HISTORY_KEEP
CT_history_pcb("iSNS_Update setting ready pcb", (UINT32)(gISD.srv[i].pt_iSNS));
#endif
                TaskSetState(gISD.srv[i].pt_iSNS, PCB_READY);
            }
        }
    }
}/* iSNS_Update */

/**
******************************************************************************
**
**  @brief      Callback function for iSNS_txrx
**
**  @param      UINT32 - events
**              void * - pRef
**
**  @return     none
**
******************************************************************************
**/
void isnsCB(UINT32 events UNUSED, void *pRef)
{
    if(pRef != NULL)
    {
        if (TaskGetState((PCB*)pRef) == PCB_NOT_READY)
        {
#ifdef HISTORY_KEEP
CT_history_pcb("isnsCB setting ready pcb", (UINT32)pRef);
#endif
            TaskSetState((PCB *)pRef, PCB_READY);
        }
    }
}/* isnsCB */

/**
******************************************************************************
**
**  @brief      t_iSNS
**              iSNS task
**
**  @param      UINT32 indx   -   iSNS Srv indx
**
**  @return     none
**
******************************************************************************
**/
void t_iSNS(UINT32 a UNUSED, UINT32 b UNUSED, UINT32 indx)
{
    UINT32 i;
    UINT16 port;
    UINT32 srvIP;
    UINT32 flags;
    ISNS_REQ *pReq = NULL;
    ISNS_RSP *pRsp = NULL;

    /*
    ** Initialize the task variables and structs
    */
    srvIP   = gISD.srv[indx].ip;
    port    = gISD.srv[indx].port;
    flags   = gISD.srv[indx].sflags;

    /*
    ** Task Loop
    */
    while(BIT_TEST(gISD.gflags, ISNS_GF_MASTER))
    {
        /*
        ** set the socket as non blocking
        */
        pReq = (ISNS_REQ *)s_MallocC(sizeof(ISNS_REQ), __FILE__, __LINE__);
        pRsp = (ISNS_RSP *)s_MallocC(sizeof(ISNS_RSP), __FILE__, __LINE__);
        for(i = 0; i < MAX_TARGETS; i++)
        {
            if((T_tgdindx[i] == NULL)
                            || !BIT_TEST(T_tgdindx[i]->opt, TARGET_ISCSI)
                            || BIT_TEST(T_tgdindx[i]->opt, TARGET_ICL))
                continue;
            isns_build_req(ISNS_REQ_QRY, i, pReq);
            if(isns_txrx(srvIP, port, flags, pReq, pRsp) == EC_OK)
            {
                if(pRsp->error == 0)
                {
                    /*
                    ** Query Success! Now check the following:
                    **  1. iSNS disabled            - Deregister
                    **  2. iSNS Server IP change    - deregister with the current server
                    **                                and register with the new server
                    **  3. Target IP changed        - Deregister & reregister
                    */
                    if(!BIT_TEST(gISD.gflags, ISNS_CF_ENABLE))
                    {
                        isns_build_req(ISNS_REQ_DEREG, i, pReq);
                        isns_txrx(srvIP, port, flags, pReq, pRsp);
                    }
                    else if((srvIP != gISD.srv[indx].ip)
                            || (port != gISD.srv[indx].port)
                            || (flags != gISD.srv[indx].sflags)
                            || (T_tgdindx[i]->ipAddr != isns_retrieveIP(pRsp)))
                    {
                        isns_build_req(ISNS_REQ_DEREG, i, pReq);
                        isns_txrx(srvIP, port, flags, pReq, pRsp);
                        if(gISD.srv[indx].ip != 0x0)
                        {
                            isns_build_req(ISNS_REQ_REG, i, pReq);
                            isns_txrx(gISD.srv[indx].ip, gISD.srv[indx].port, gISD.srv[indx].sflags, pReq, pRsp);
                        }
                    }
                }
                else {
                    /*
                    ** Query Failed! Now check the following:
                    **  1. iSNS enabled             - register with latest configuration
                    */
                    if((BIT_TEST(gISD.gflags, ISNS_CF_ENABLE)) && (gISD.srv[indx].ip != 0x0))
                    {
                        isns_build_req(ISNS_REQ_REG, i, pReq);
                        isns_txrx(gISD.srv[indx].ip, gISD.srv[indx].port, gISD.srv[indx].sflags, pReq, pRsp);
                    }
                }
            }
            else
            {
                /*
                ** Could not communicate with the iSNS server! Check if
                ** the configuration changed and do the needful
                */
                if((BIT_TEST(gISD.gflags, ISNS_CF_ENABLE))
                                && (gISD.srv[indx].ip != 0x0)
                                && (srvIP != gISD.srv[indx].ip))
                {
                    isns_build_req(ISNS_REQ_REG, i, pReq);
                    isns_txrx(gISD.srv[indx].ip, gISD.srv[indx].port, gISD.srv[indx].sflags, pReq, pRsp);
                }
            }
        }
        s_Free((void *)pReq, sizeof(ISNS_REQ), __FILE__, __LINE__);
        s_Free((void *)pRsp, sizeof(ISNS_RSP), __FILE__, __LINE__);

        /*
        ** If the old IP is used by another task, there will be a rase condition
        ** wherein, the other task would have sent query requests and could have
        ** found that the targets are already registered before the current
        ** task de-registered them. When this happens, there will be a 15min hole
        ** before the keepalive time outs and the targets are registered again
        ** with the old IP by the other task. To avoid this situation, check to
        ** see if the old IP is being currently used by other active tasks. If so,
        ** wake those tasks so that they do the needful.
        */
        for(i = 0; i < MAX_ISNS_SERVERS; i++)
        {
            if((gISD.srv[i].ip == srvIP)
                    && (gISD.srv[i].pt_iSNS != NULL))
            {
                if (TaskGetState(gISD.srv[i].pt_iSNS) == PCB_NOT_READY)
                {
#ifdef HISTORY_KEEP
CT_history_pcb("t_iSNS setting ready pcb", (UINT32)(gISD.srv[i].pt_iSNS));
#endif
                    TaskSetState(gISD.srv[i].pt_iSNS, PCB_READY);
                }
            }
        }
        srvIP   = gISD.srv[indx].ip;
        port    = gISD.srv[indx].port;
        flags   = gISD.srv[indx].sflags;

        /*
        ** If iSNS is enabled, Sleep until Keep Alive timeout and if
        ** iSNS is disabled, Sleep until next Config Update
        */
        if(!(BIT_TEST(gISD.gflags, ISNS_CF_ENABLE)) || (gISD.srv[indx].ip == 0x0))
        {
            break;
        }
        TaskSleepMS(ISNS_KEEPALIVE_TO);
    }
    gISD.srv[indx].pt_iSNS = NULL;
}/* t_iSNS */

/**
******************************************************************************
**
**  @brief      Retrieve IP address from Query Response
**
**  @param      ISNS_RSP *
**
**  @return     UINT32 - IP address
**
******************************************************************************
**/
UINT32 isns_retrieveIP(ISNS_RSP *pRsp)
{
    struct attr {
        UINT32  tag;
        UINT32  len;
        UINT32  data[4];
    } *at;

    for(at = (struct attr *)pRsp->data;
        (void *)at < (void *)(pRsp->data + ntohs(pRsp->length));
        at = (struct attr *)(((UINT32)at) + ntohl(at->len) + 2*sizeof(UINT32)))
    {
        if(ntohl(at->tag) == ISNS_TAG_PORTALIP)
        {
            return (at->data[3]);
        }
    }
    return 0;
}/* isns_retrieveIP */

/**
******************************************************************************
**
**  @brief      Built the iSNS PDU
**
**  @param      UINT16 - Request Type
**              UINT16 - tid
**              ISNS_REQ *
**
**  @return     UINT32 - length of the Request PDU built
**
******************************************************************************
**/
UINT32 isns_build_req(UINT16 func, UINT16 tid, ISNS_REQ *pReq)
{
    static UINT16 trxID = 1;

    memset((void *)pReq, 0, sizeof(ISNS_REQ));
    if(trxID == 0)
        trxID++;

    pReq->version       = htons(ISNS_VERSION);
    pReq->function      = htons(func);
    pReq->flags         = (func == ISNS_REQ_REG) ? (1 << ISNS_F_REPLACE) : 0;
    pReq->flags         |= ((1 << ISNS_F_CLIENT) | (1 << ISNS_F_FIRST) | (1 << ISNS_F_LAST));
    pReq->flags         = htons(pReq->flags);
    pReq->transactionID = htons(trxID++);
    pReq->seqID         = 0;

    pReq->src_T         = htonl(ISNS_TAG_ISCSINAME);
    pReq->src_L         = htonl(sizeof(pReq->src_V));
    getiscsiNameForTarget((UINT8 *)pReq->src_V, tid);

    switch (func)
    {
        case ISNS_REQ_QRY:
        {
            pReq->qkey_T            = htonl(ISNS_TAG_EID);
            pReq->qkey_L            = htonl(sizeof(pReq->qkey_V));
            convertIntoReadableData(T_tgdindx[tid]->nodeName,pReq->qkey_V);
            pReq->qkey_V[16]        = '\0';
            pReq->q_limit           = 0x0;
            pReq->qip_T             = htonl(ISNS_TAG_PORTALIP);
            pReq->qip_L             = 0;
            pReq->length            = htons(PDU_QRY_LEN);
        }
        return (PDU_QRY_LEN);
        case ISNS_REQ_REG:
        {
            pReq->rkey_T            = htonl(ISNS_TAG_EID);
            pReq->rkey_L            = htonl(sizeof(pReq->qkey_V));
            convertIntoReadableData(T_tgdindx[tid]->nodeName,pReq->rkey_V);
            pReq->rkey_V[16]        = '\0';
            pReq->r_limit           = 0x0;
            pReq->reid_T            = htonl(ISNS_TAG_EID);
            pReq->reid_L            = htonl(sizeof(pReq->qkey_V));
            convertIntoReadableData(T_tgdindx[tid]->nodeName,pReq->reid_V);
            pReq->reid_V[16]        = '\0';
            pReq->rep_T             = htonl(ISNS_TAG_PROTO);
            pReq->rep_L             = htonl(4);
            pReq->rep_V             = htonl(ISNS_EP_ISCSI);
            pReq->rip_T             = htonl(ISNS_TAG_PORTALIP);
            pReq->rip_L             = htonl(16);
            pReq->rip_V[0]          = 0x0;
            pReq->rip_V[1]          = 0x0;
            pReq->rip_V[2]          = htonl(0xffff);
            pReq->rip_V[3]          = T_tgdindx[tid]->ipAddr;
            pReq->rpp_T             = htonl(ISNS_TAG_PORTALPORT);
            pReq->rpp_L             = htonl(4);
            pReq->rpp_Port          = htons(3260);
            pReq->rpp_PortType      = 0x0;
            pReq->rtn_T             = htonl(ISNS_TAG_ISCSINAME);
            pReq->rtn_L             = htonl(sizeof(pReq->rtn_V));
            getiscsiNameForTarget((UINT8 *)pReq->rtn_V, tid);
            pReq->rnt_T             = htonl(ISNS_TAG_NODETYPE);
            pReq->rnt_L             = htonl(4);
            pReq->rnt_V             = htonl(1 << ISNS_N_TARGET);
            pReq->length            = htons(PDU_REG_LEN);
        }
        return (PDU_REG_LEN);
        case ISNS_REQ_DEREG:
        {
            pReq->d_limit           = 0x0;
            pReq->deid_T            = htonl(ISNS_TAG_EID);
            pReq->deid_L            = htonl(sizeof(pReq->qkey_V));
            convertIntoReadableData(T_tgdindx[tid]->nodeName,pReq->deid_V);
            pReq->deid_V[16]        = '\0';
            pReq->length            = htons(PDU_DEREG_LEN);
        }
        return (PDU_DEREG_LEN);

        default:
            break;
    }
    return (0);
}/* isns_build_pdu */

/**
******************************************************************************
**
**  @brief      Send the Request PDU and get the response PDU
**
**  @param      INT32       - Socket fd to operate on
**              ISNS_REQ *  - Contains the request PDU to send
**              ISNS_RSP *  - expects the response in this on return
**
**  @return     INT32 - success of failure
**
******************************************************************************
**/
INT32 isns_txrx(UINT32 ip, UINT16 port, UINT32 flags, ISNS_REQ *pReq, ISNS_RSP *pRsp)
{
    INT32  fd;
    INT32 rVal = EC_RETRY;
    struct sockaddr_in addr;
    INT32 current, balance;

    memset((void *)pRsp, 0, sizeof(ISNS_RSP));

    if(BIT_TEST(flags, ISNS_SF_UDPTCP))
        fd = socket (AF_INET, SOCK_DGRAM, 0);
    else
        fd = socket (AF_INET, SOCK_STREAM, 0);

    if(fd > 0)
    {
        bzero (&addr, sizeof(addr));

        addr.sin_family         = AF_INET;
        addr.sin_port           = htons(port);
        addr.sin_addr.s_addr    = ip;

        TSL_SetNonBlocking (fd);

        /*
        ** Wait until the connection is established before proceeding
        */
        while(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
        {
            int save_errno = errno;

            if((save_errno == EINPROGRESS) || (save_errno == EALREADY))
            {
                TaskSleepMS(500);       /* Sleep for 500 ms */
            }
            else if(save_errno == EISCONN)
            {
                break;
            }
            else
            {
                fprintf(stderr, "ISNS_DEBUG: Connecting to iSNS Server [%s:%d] FAILED\n", inet_ntoa((struct in_addr)addr.sin_addr), port);
                fprintf(stderr,"ISNS_DEBUG: !!!! !!!! !!! isns_txrx: connect ERROR: errno(%d, %s)\n", save_errno,strerror(save_errno));
                close(fd);
                return (rVal);
            }
        }
        /*
        ** Connection established, Now send the PDU
        */
        tsl_ev_add(fd, EPOLLIN, isnsCB, (void *)K_xpcb);
        if(write(fd, (char *)pReq, ntohs(pReq->length) + 12) == ntohs(pReq->length) + 12)
        {
            /*
            ** PDU sent. Not wait for the response. First read the header - which is 12 bytes.
            ** From the header retrieve the payload size and read the rest of the PDU.
            */
            current = 0;
            balance = 12;
            while (1)
            {
                TaskSetMyState(PCB_NOT_READY);
                TaskSwitch();
                if((rVal = read(fd, (char*)pRsp + current, balance)) < 0)
                {
                    if((errno != EINTR) && (errno != EAGAIN))
                    {
                        fprintf(stderr,"ISNS_DEBUG: !!!! !!!! !!! isns_txrx: read ERROR: errno(%d, %s)\n", errno,strerror(errno));
                        break;
                    }
                    continue;
                }
                current += rVal;
                balance -= rVal;
                if((current == 12) && (balance == 0))
                {
                    balance = ntohs(pRsp->length);
                }
                if(balance == 0)
                {

                    /*
                    ** Log an appropriate msg
                    */
                    switch(ntohs(pReq->function))
                    {
                        case ISNS_REQ_QRY:
                        {
                            if(pRsp->error != 0)
                            {
                                fprintf(stderr, "ISNS_DEBUG: iSNS Server [%s:%d] Target %s not Registered\n",
                                                    inet_ntoa((struct in_addr)addr.sin_addr), port, pReq->src_V);
                            }
                        }
                        break;
                        case ISNS_REQ_REG:
                        {
                            fprintf(stderr, "ISNS_DEBUG: iSNS Server [%s:%d] REGISTER Target %s %s\n",
                                                    inet_ntoa((struct in_addr)addr.sin_addr), port,
                                                    pReq->src_V, (pRsp->error == 0) ? "Successful" : "Unsuccessful");

                        }
                        break;
                        case ISNS_REQ_DEREG:
                        {
                            fprintf(stderr, "ISNS_DEBUG: iSNS Server [%s:%d] DEREGISTER Target %s %s\n",
                                                    inet_ntoa((struct in_addr)addr.sin_addr), port,
                                                    pReq->src_V, (pRsp->error == 0) ? "Successful" : "Unsuccessful");
                        }
                        break;

                        default:
                            break;
                    }
                    rVal = EC_OK;
                    break;
                }
            }
        }
        else {
            fprintf(stderr,"ISNS_DEBUG: !!!! !!!! !!! isns_txrx: write ERROR: errno(%d, %s)\n", errno,strerror(errno));
        }
        /*
        ** We are done. Clean up the epoll & socket handles.
        */
        tsl_ev_del(fd, EPOLLIN, isnsCB, NULL);
        tsl_cleanupEvents((void *)K_xpcb);
        close(fd);
    }
    else
    {
        fprintf(stderr,"ISNS_DEBUG: !!!! !!!! !!! isns_txrx: socket ERROR: errno(%d, %s)\n", errno,strerror(errno));
    }
    return (rVal);
}/* isns_txrx */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
