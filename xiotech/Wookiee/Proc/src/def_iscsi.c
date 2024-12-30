/* $Id: def_iscsi.c 159663 2012-08-22 15:36:42Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       def_iscsi.c
**
**  @brief      Define functions Pertaining to iSCSI
**
**  Copyright (c) 2005-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "DEF_iSCSI.h"

#include "def.h"
#ifdef FRONTEND
#include "deffe.h"
#include "fsl.h"
#include "iscsi_common.h"
#include "iscsi_pdu.h"
#include "isp.h"
#endif
#ifdef BACKEND
#include "defbe.h"
#include "nvram.h"
#endif
#include "misc.h"
#include "MR_Defs.h"
#include "LL_LinuxLinkLayer.h"
#include "online.h"
#include "system.h"
#include "target.h"
#include "icl.h"
#include "isns.h"
#include "OS_II.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"

#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <byteswap.h>

ISD gISD;

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/

#define TWO_RAISE_TO_24 16777216
#define SET_BITS_7_AND_0 0x81

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/

extern void L$send_packet(void *pReq, UINT32 reqSize, UINT32 cmdCode,
                          void *pRsp, UINT32 rspLen, void *pCallback,
                          UINT32 param);
extern UINT8 iscsiGenerateParameters(I_TGD *pParamSrc);

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/

#ifdef FRONTEND
static void copySession(MRSESSION *pSessionDest, SESSION *pSession);
#endif /* FRONTEND */

/*
******************************************************************************
** Code Start
******************************************************************************
*/

#ifdef BACKEND

#include "ldd.h"

void dumpDLM(void);

void dumpDLM(void)
{
    int i = 0;
    LDD *pLDD = NULL;

    if (DLM_lddindx[0] == NULL)     /* do nothing if null entry. */
    {
        return;
    }
    fprintf(stderr,"\tIndx pLDD       cl st msk pri altID lun ord flg lck rty paths\n");
    fprintf(stderr,"\t-------------------------------------------------------------\n");
    for (pLDD = DLM_lddindx[i++]; pLDD != NULL; pLDD = DLM_lddindx[i++])
    {
        fprintf(stderr,"\t%04d 0x%08X %02d %02d %03d %03d %05d %03d %03d %03d %03d %03d %05d\n",
                            i, (UINT32)pLDD, pLDD->class, pLDD->state, pLDD->pathMask,
                            pLDD->pathPri, pLDD->altid, pLDD->lun, pLDD->ord,
                            pLDD->flag1, pLDD->lock, pLDD->retryCount, pLDD->numPaths);
    }
}   /* dumpDLM */


UINT32 isValidIP(UINT32 ip, UINT32 mask);
/*
** This function returns true if the IP address does not belong to following
** category
** 0.0.0.0 - local host
** 127.xxx.xxx.xxx.xxx  - local loopback address
** xxx.0.0.0, xxx.xxx.0.0, xxx.xxx.xxx.0  local host IP
** 255.255.255.255 - limited broadcast
** xxx.255.255.255, xxx.xxx.255.255, xxx.xxx.xxx.255 - directed broadcast
*/

UINT32 isValidIP(UINT32 ip, UINT32 mask)
{
    /*
    ** convert into network byte order
    */

    ip = htonl(ip);
    mask = htonl(mask);

    /*
    ** find if it is broadcast address
    */
    if((ip == 0xffffffff) ||(~mask | ip) == ip )
    {
        fprintf(stderr,"ERROR - BROADCAST address -- mask (%x) ip(%x) result(%x)\n",mask,ip,(~mask | ip));
        return 0;
    }
    /*
    **find out if it is loop back address
    */
    if(ip == 0 || ((ip & 0xff000000) >> 24) == 127)
    {
        fprintf(stderr,"ERROR Loop back address %x\n",ip);
        return 0;
    }
    /*
    **  binary host address with all 0s or all 1s are invalid
    */
    if((ip & ~mask) == 0 || (ip & ~mask) == ~mask)
    {
        fprintf(stderr,"Host address is either all 0s or all 1s\n");
        return 0;
    }
    return 1;
}
/**
******************************************************************************
**
**  @brief      Get ownership of a specific copy for a VDD.
**
**  @param      vdd - VDD in a copy for which we want the parent
**
**  @return     vdd - the parent or grandparent (case of a deleted vlink)
**
**  @attention  None
**
******************************************************************************
**/
void DEF_CreateTargetInfo(TGD *pTGD, I_TGD *iTGD)
{
    CHAPINFO   *pInfo;
    CHAPINFO   *pInfo1;
    if(pTGD->itgd == NULL)
    {
        pTGD->itgd = (I_TGD *)s_MallocC(sizeof(I_TGD), __FILE__, __LINE__);
    }
    /*
    ** Update iSCSI target (itgd)
    */
    if(iTGD == NULL)
    {
        /*
        ** if iTGD is NULL, then update with default values and this happens
        ** initiatlly when iSCSI targets are identified for the first time
        */
        pTGD->itgd->tid =  pTGD->tid;
        pTGD->itgd->ipAddr= bswap_32(0x0101010A + pTGD->tid);  /* 1.1.1.(10 + tid)   */
        pTGD->itgd->ipMask= bswap_32(0xffffff00);              /* 255.255.255.0 */
        pTGD->itgd->ipGw=0;
        pTGD->itgd->maxConnections=1;
        pTGD->itgd->initialR2T=1;
        pTGD->itgd->immediateData=0;
        pTGD->itgd->dataSequenceInOrder=1;
        pTGD->itgd->dataPDUInOrder=1;
        pTGD->itgd->ifMarker=0;
        pTGD->itgd->ofMarker=0;
        pTGD->itgd->errorRecoveryLevel=0;
        pTGD->itgd->targetPortalGroupTag = pTGD->tid;
        pTGD->itgd->maxBurstLength=262144;
        pTGD->itgd->firstBurstLength=65536;
        pTGD->itgd->defaultTime2Wait=2;
        pTGD->itgd->defaultTime2Retain=20;
        pTGD->itgd->maxOutstandingR2T=1;
        pTGD->itgd->maxRecvDataSegmentLength=65536;
        pTGD->itgd->maxSendDataSegmentLength=8192;
        pTGD->itgd->ifMarkInt=0;
        pTGD->itgd->ofMarkInt=0;
        pTGD->itgd->headerDigest=0;
        pTGD->itgd->dataDigest=0;
        pTGD->itgd->authMethod=0x81;              /* Making CHAP mandatory by default */
        pTGD->itgd->mtuSize=1500;                 /* Setting MTU to 1500 by default   */
        strcpy((char *)pTGD->itgd->tgtAlias,"");
        /*
        ** mask is to prevent user to configure parameter whose support is not built in STACK
        ** right now following are supported parameters
        **    KEY0_IP
        **    KEY1_SUBNET_MASK
        **    KEY2_GATEWAY
        **    KEY17_MAX_RECV_DATASEGMENT_LENGTH
        **    KEY20_HEADER_DIGEST
        **    KEY21_DATA_DIGEST
        **    KEY22_AUTHMETHOD
        **    KEY23_MTUSIZE
        **    KEY24_TGTALIAS
        **
        */
        pTGD->i_mask = 0x3f20007;

    }
    else
    {
        /*
        ** nvram restore happens here
        */
        pTGD->itgd->tid                 = iTGD->tid;
        pTGD->itgd->ipAddr              =  iTGD->ipAddr;
        pTGD->itgd->ipMask              =  iTGD->ipMask;
        pTGD->itgd->ipGw                =  iTGD->ipGw;
        pTGD->itgd->maxConnections      =  iTGD->maxConnections;
        pTGD->itgd->initialR2T          =  iTGD->initialR2T;
        pTGD->itgd->immediateData       =  iTGD->immediateData;
        pTGD->itgd->dataSequenceInOrder =  iTGD->dataSequenceInOrder;
        pTGD->itgd->dataPDUInOrder      =  iTGD->dataPDUInOrder;
        pTGD->itgd->ifMarker            =  iTGD->ifMarker;
        pTGD->itgd->ofMarker            =  iTGD->ofMarker;
        pTGD->itgd->errorRecoveryLevel  =  iTGD->errorRecoveryLevel;
        pTGD->itgd->targetPortalGroupTag= iTGD->targetPortalGroupTag;
        pTGD->itgd->maxBurstLength      =  iTGD->maxBurstLength;
        pTGD->itgd->firstBurstLength    =  iTGD->firstBurstLength;
        pTGD->itgd->defaultTime2Wait    =  iTGD->defaultTime2Wait;
        pTGD->itgd->defaultTime2Retain  =  iTGD->defaultTime2Retain;
        pTGD->itgd->maxOutstandingR2T   =  iTGD->maxOutstandingR2T;
        pTGD->itgd->maxRecvDataSegmentLength = iTGD->maxRecvDataSegmentLength;
        pTGD->itgd->maxSendDataSegmentLength = iTGD->maxSendDataSegmentLength;
        pTGD->itgd->ifMarkInt       =  iTGD->ifMarkInt;
        pTGD->itgd->ofMarkInt       =  iTGD->ofMarkInt;
        pTGD->itgd->headerDigest    =  iTGD->headerDigest;
        pTGD->itgd->dataDigest      =  iTGD->dataDigest;
        pTGD->itgd->authMethod      =  iTGD->authMethod;
        pTGD->itgd->mtuSize         =  iTGD->mtuSize;
        strcpy((char *)pTGD->itgd->tgtAlias, (char *)iTGD->tgtAlias);
        /*
        ** Delete all chapusers info because nvram restore will allocate anyway
        */
        pInfo = pTGD->itgd->chapInfo;
        while(pInfo != NULL)
        {
            /*
            ** Free existing userinfo
            */
            pInfo1 = pInfo->fthd;
            s_Free((void *)pInfo, sizeof(CHAPINFO), __FILE__, __LINE__);
            pInfo = pInfo1;
        }
    }
    pTGD->itgd->numUsers = 0;
    pTGD->itgd->chapInfo = NULL;
}/* DEF_CreateTargetInfo */

void DEF_CreateChapUserInfo(TGD *pTGD, CHAPINFO *pUserInfo)
{
    I_TGD      *pITGD = pTGD->itgd;
    CHAPINFO   *pInfo1;
    CHAPINFO   *pInfo2;

    if(pITGD == NULL)
    {
        return;
    }

    pInfo1 = (CHAPINFO *)s_MallocC(sizeof(CHAPINFO), __FILE__, __LINE__);
    pInfo1->fthd = NULL;

    memcpy(pInfo1->sname, pUserInfo->sname, sizeof(pUserInfo->sname));
    memcpy(pInfo1->secret1, pUserInfo->secret1, sizeof(pUserInfo->secret1));
    memcpy(pInfo1->secret2, pUserInfo->secret2, sizeof(pUserInfo->secret2));

    if (pITGD->chapInfo == NULL)
    {
        pITGD->chapInfo = pInfo1;
    }
    else
    {
        pInfo2 = pITGD->chapInfo;
        while (1)
        {
            if (pInfo2->fthd == NULL)
            {
                pInfo2->fthd = pInfo1;
                break;
            }
            pInfo2 = pInfo2->fthd;
        }
    }
    pITGD->numUsers++;
}/* DEF_CreateChapUserInfo */

void iSCSIAddUserBE(MRCHAPCONFIG userInfo)
{
    TGD       *tgd;
    I_TGD     *itgd;
    CHAPINFO  *pUserInfo;
    CHAPINFO  *pInfo1;

    tgd = T_tgdindx[userInfo.tid];
    if (tgd == NULL)
    {
        return;
    }
    itgd = tgd->itgd;
    if (itgd == NULL)
    {
        return;
    }
    pUserInfo = itgd->chapInfo;


    if(pUserInfo == NULL)
    {
        pInfo1 = (CHAPINFO *)s_MallocC(sizeof(CHAPINFO), __FILE__, __LINE__);
        pInfo1->fthd = NULL;
        itgd->chapInfo = pInfo1;
        itgd->numUsers = 1;
        memcpy(pInfo1->sname, userInfo.sname, sizeof(userInfo.sname));
        if(userInfo.opt == 0)
        {
            memcpy(pInfo1->secret1, userInfo.secret1, sizeof(userInfo.secret1));
        }
        else if (userInfo.opt == 1)
        {
            memcpy(pInfo1->secret2, userInfo.secret1, sizeof(userInfo.secret1));
        }
        else if (userInfo.opt == 2)
        {
            memcpy(pInfo1->secret1, userInfo.secret1, sizeof(userInfo.secret1));
            memcpy(pInfo1->secret2, userInfo.secret2, sizeof(userInfo.secret2));
        }
    }
    else
    {
        while(1)
        {
            if(strcmp((char *)pUserInfo->sname, (char *)userInfo.sname)==0)
            {
                if(userInfo.opt == 0)
                {
                    memcpy(pUserInfo->secret1, userInfo.secret1, sizeof(userInfo.secret1));
                }
                else if (userInfo.opt == 1)
                {
                    memcpy(pUserInfo->secret2, userInfo.secret1, sizeof(userInfo.secret1));
                }
                else if (userInfo.opt == 2)
                {
                    memcpy(pUserInfo->secret1, userInfo.secret1, sizeof(userInfo.secret1));
                    memcpy(pUserInfo->secret2, userInfo.secret2, sizeof(userInfo.secret2));
                }
                break;
            }
            if(pUserInfo->fthd == NULL)
            {
                pInfo1 = (CHAPINFO *)s_MallocC(sizeof(CHAPINFO), __FILE__, __LINE__);
                pInfo1->fthd = NULL;
                pUserInfo->fthd = pInfo1;
                itgd->numUsers++;
                memcpy(pInfo1->sname, userInfo.sname, sizeof(userInfo.sname));
                if(userInfo.opt == 0)
                {
                    memcpy(pInfo1->secret1, userInfo.secret1, sizeof(userInfo.secret1));
                }
                else if (userInfo.opt == 1)
                {
                    memcpy(pInfo1->secret2, userInfo.secret1, sizeof(userInfo.secret1));
                }
                else if (userInfo.opt == 2)
                {
                    memcpy(pInfo1->secret1, userInfo.secret1, sizeof(userInfo.secret1));
                    memcpy(pInfo1->secret2, userInfo.secret2, sizeof(userInfo.secret2));
                }
                break;
            }
            pUserInfo = pUserInfo->fthd;
        }
    }
}

void iSCSIRemoveUserBE(MRCHAPCONFIG userInfo)
{
    TGD       *tgd;
    I_TGD     *itgd;
    CHAPINFO  *pUserInfo;
    CHAPINFO  *pInfo1;

    tgd = T_tgdindx[userInfo.tid];
    if (tgd == NULL)
    {
        return;
    }
    itgd = tgd->itgd;
    if (itgd == NULL)
    {
        return;
    }
    pUserInfo = itgd->chapInfo;

    if(pUserInfo == NULL)
    {
        return;
    }
    if(strcmp((char *)pUserInfo->sname, (char *)userInfo.sname)==0)
    {
        pInfo1 = pUserInfo;

        if (pUserInfo->fthd == NULL)
        {
            itgd->chapInfo = NULL;
            itgd->numUsers = 0;
        }
        else
        {
            itgd->chapInfo = pUserInfo->fthd;
            itgd->numUsers--;
        }
        s_Free((void *)pInfo1, sizeof(CHAPINFO), __FILE__, __LINE__);
    }
    else
    {
        while(1)
        {
            if(pUserInfo->fthd == NULL)
            {
                break;
            }
            if(strcmp((char *)pUserInfo->fthd->sname, (char *)userInfo.sname)==0)
            {
                pInfo1 = pUserInfo->fthd;
                if (pInfo1->fthd == NULL)
                {
                    pUserInfo->fthd = NULL;
                }
                else
                {
                    pUserInfo->fthd = pInfo1->fthd;
                }
                itgd->numUsers--;
                s_Free((void *)pInfo1, sizeof(CHAPINFO), __FILE__, __LINE__);
                break;
            }
            pUserInfo = pUserInfo->fthd;
        }
    }
}

/**
******************************************************************************
**
**  @brief      Set iSCSI Target configuration
**
**  @param      MRP *pMRP
**
**  @return     UINT8 status
**
**  @attention
**
******************************************************************************
**/
UINT8 DEF_SetTgInfo(MR_PKT *pMRP)
{
    UINT8  retCode  = DEOK;
    UINT8  targUpd  = FALSE;
    UINT8  iscsiUpd = FALSE;
    I_TGD  *pITG;
    TGD    *pTGD;
    MRSETTGINFO_REQ   *pReq;
    MRSETTGINFO_RSP   *pRsp;
    INT32 invalidMask = 0;

    pReq = (MRSETTGINFO_REQ *)(pMRP->pReq);
    pRsp = (MRSETTGINFO_RSP *)(pMRP->pRsp);
    pRsp->header.len = sizeof(MRSETTGINFO_RSP);

    pITG = &(pReq->i_tgd);

    /*
    ** ATTN: If a parameter is allowed to be changed i.e. enabling it through the mask then
    ** please validate the possible values for the enabled parameter.
    ** if four parameters are being set and the code discovers that one of them does not have correct value
    ** it will set 3 other correct parameters and returns error. The parameter having wrong value will have the previous
    ** value
    **
    */
    if((pITG->tid < MAX_TARGETS)
       && ((pTGD = gTDX.tgd[pITG->tid]) != NULL)
       && (pTGD->itgd != NULL)
       && (pTGD->i_mask & pReq->setmap))
    {
        /*
        ** Process the IP Mask Setting
        */
        if((BIT_TEST(pReq->setmap, KEY1_SUBNET_MASK)) &&
                (pTGD->itgd->ipMask != pITG->ipMask))
        {
            /*
            ** Change only if valid subnet mask
            ** while modifying mask validate mask for the IP Address
            */
            if(pITG->ipMask != 0 && pITG->ipMask != 0xffffffff
                    && (( ~(ntohl(pITG->ipMask)) + 1) & ~(ntohl(pITG->ipMask))) == 0
                    && isValidIP(pTGD->itgd->ipAddr,pITG->ipMask) == 1)
            {
                if(BIT_TEST(pTGD->opt, TARGET_ISCSI))
                {
                    pTGD->ipPrefix = MSC_Mask2Prefix(bswap_32(pITG->ipMask));
                    targUpd = TRUE;
                }
                pTGD->itgd->ipMask = pITG->ipMask;
                iscsiUpd = TRUE;
            }else
            {
                /*
                ** so that IP address setting does not take effect
                ** set return code to invalid param
                */
                invalidMask = 1;
                fprintf(stderr,"Error: subnet mask is not set %x \n",pITG->ipMask);
                retCode = DEINVISCSIPARAM;
            }
        }
        /*
        ** Process the IP Address Setting
        */
        if((BIT_TEST(pReq->setmap, KEY0_IP)) &&
           (pTGD->itgd->ipAddr != pITG->ipAddr) && invalidMask == 0)
        {
            if(isValidIP(pITG->ipAddr,pTGD->itgd->ipMask) == 1)
            {
                if(BIT_TEST(pTGD->opt, TARGET_ISCSI))
                {
                    pTGD->ipAddr = pITG->ipAddr;
                    targUpd = TRUE;
                }
                pTGD->itgd->ipAddr = pITG->ipAddr;
                iscsiUpd = TRUE;
            }else
            {
                /*
                ** set return code to invalid param
                */
                fprintf(stderr,"Error: IP address not set %x\n",pITG->ipAddr);
                retCode = DEINVISCSIPARAM;
            }
        }
        /*
        ** Process the IP Gateway Setting
        */
        if((BIT_TEST(pReq->setmap, KEY2_GATEWAY)) &&
           (pTGD->itgd->ipGw != pITG->ipGw))
        {
            if(BIT_TEST(pTGD->opt, TARGET_ISCSI))
            {
                pTGD->ipGw = pITG->ipGw;
                targUpd = TRUE;
            }
            pTGD->itgd->ipGw = pITG->ipGw;
            iscsiUpd = TRUE;
        }
        /*
        ** Process the Max connections Setting
        */
        if((BIT_TEST(pReq->setmap, KEY3_MAX_CONNECTIONS))
           && (BIT_TEST(pTGD->i_mask, KEY3_MAX_CONNECTIONS))
           && (pTGD->itgd->maxConnections != pITG->maxConnections))
        {
            pTGD->itgd->maxConnections = pITG->maxConnections;
            iscsiUpd = TRUE;
        }
        /*
        ** Process the Initial R2T Setting
        */
        if((BIT_TEST(pReq->setmap, KEY4_INITIAL_R2T))
           && (BIT_TEST(pTGD->i_mask, KEY4_INITIAL_R2T))
           && (pTGD->itgd->initialR2T != pITG->initialR2T))
        {
            pTGD->itgd->initialR2T = pITG->initialR2T;
            iscsiUpd = TRUE;
        }
        /*
        ** Process the Immediate Data Setting
        */
        if((BIT_TEST(pReq->setmap, KEY5_IMMEDIATE_DATA))
           && (BIT_TEST(pTGD->i_mask, KEY5_IMMEDIATE_DATA))
           && (pTGD->itgd->immediateData != pITG->immediateData))
        {
            pTGD->itgd->immediateData = pITG->immediateData;
            iscsiUpd = TRUE;
        }
        /*
        ** Process the Data Seq in order Setting
        */
        if((BIT_TEST(pReq->setmap, KEY6_DATA_SEQUENCE_IN_ORDER))
           && (BIT_TEST(pTGD->i_mask, KEY6_DATA_SEQUENCE_IN_ORDER))
           && (pTGD->itgd->dataSequenceInOrder != pITG->dataSequenceInOrder))
        {
            pTGD->itgd->dataSequenceInOrder = pITG->dataSequenceInOrder;
            iscsiUpd = TRUE;
        }
        /*
        ** Process the Data PDU in order Setting
        */
        if((BIT_TEST(pReq->setmap, KEY7_DATA_PDU_IN_ORDER))
           && (BIT_TEST(pTGD->i_mask, KEY7_DATA_PDU_IN_ORDER))
           && (pTGD->itgd->dataPDUInOrder != pITG->dataPDUInOrder))
        {
            pTGD->itgd->dataPDUInOrder = pITG->dataPDUInOrder;
            iscsiUpd = TRUE;
        }
        /*
        ** Process the IF Marker Setting
        */
        if((BIT_TEST(pReq->setmap, KEY8_IF_MARKER))
           && (BIT_TEST(pTGD->i_mask, KEY8_IF_MARKER))
           && (pTGD->itgd->ifMarker != pITG->ifMarker))
        {
            pTGD->itgd->ifMarker = pITG->ifMarker;
            iscsiUpd = TRUE;
        }
        /*
        ** Process the OF Marker Setting
        */
        if((BIT_TEST(pReq->setmap, KEY9_OF_MARKER))
           && (BIT_TEST(pTGD->i_mask, KEY9_OF_MARKER))
           && (pTGD->itgd->ofMarker != pITG->ofMarker))
        {
            pTGD->itgd->ofMarker = pITG->ofMarker;
            iscsiUpd = TRUE;
        }
        /*
        ** Process the Error Recovery Level Setting
        */
        if((BIT_TEST(pReq->setmap, KEY10_ERROR_RECOVERY_LEVEL))
           && (BIT_TEST(pTGD->i_mask, KEY10_ERROR_RECOVERY_LEVEL))
           && (pTGD->itgd->errorRecoveryLevel != pITG->errorRecoveryLevel))
        {
            pTGD->itgd->errorRecoveryLevel = pITG->errorRecoveryLevel;
            iscsiUpd = TRUE;
        }
        /*
        ** Process the Target Portal Group Tag Setting
        */
        if((BIT_TEST(pReq->setmap, KEY11_TARGET_PORTAL_GROUP_TAG))
           && (BIT_TEST(pTGD->i_mask, KEY11_TARGET_PORTAL_GROUP_TAG))
           && (pTGD->itgd->targetPortalGroupTag != pITG->targetPortalGroupTag))
        {
            pTGD->itgd->targetPortalGroupTag = pITG->targetPortalGroupTag;
            iscsiUpd = TRUE;
        }
        /*
        ** Process the Max Burst Length Setting
        */
        if((BIT_TEST(pReq->setmap, KEY12_MAX_BURST_LENGTH))
           && (BIT_TEST(pTGD->i_mask, KEY12_MAX_BURST_LENGTH))
           && (pTGD->itgd->maxBurstLength != pITG->maxBurstLength))
        {
            pTGD->itgd->maxBurstLength = pITG->maxBurstLength;
            iscsiUpd = TRUE;
        }
        /*
        ** Process the First Burst Length Setting
        */
        if((BIT_TEST(pReq->setmap, KEY13_FIRST_BURST_LENGTH))
           && (BIT_TEST(pTGD->i_mask, KEY13_FIRST_BURST_LENGTH))
           && (pTGD->itgd->firstBurstLength != pITG->firstBurstLength))
        {
            pTGD->itgd->firstBurstLength = pITG->firstBurstLength;
            iscsiUpd = TRUE;
        }
        /*
        ** Process the Default Time2Wait Setting
        */
        if((BIT_TEST(pReq->setmap, KEY14_DEFAULT_TIME2_WAIT))
           && (BIT_TEST(pTGD->i_mask, KEY14_DEFAULT_TIME2_WAIT))
           && (pTGD->itgd->defaultTime2Wait != pITG->defaultTime2Wait))
        {
            pTGD->itgd->defaultTime2Wait = pITG->defaultTime2Wait;
            iscsiUpd = TRUE;
        }
        /*
        ** Process the Default Time2Retain Setting
        */
        if((BIT_TEST(pReq->setmap, KEY15_DEFAULT_TIME2_RETAIN))
           && (BIT_TEST(pTGD->i_mask, KEY15_DEFAULT_TIME2_RETAIN))
           && (pTGD->itgd->defaultTime2Retain != pITG->defaultTime2Retain))
        {
            pTGD->itgd->defaultTime2Retain = pITG->defaultTime2Retain;
            iscsiUpd = TRUE;
        }
        /*
        ** Process the Max Outstanding R2T Setting
        */
        if((BIT_TEST(pReq->setmap, KEY16_MAX_OUTSTANDING_R2T))
           && (BIT_TEST(pTGD->i_mask, KEY16_MAX_OUTSTANDING_R2T))
           && (pTGD->itgd->maxOutstandingR2T != pITG->maxOutstandingR2T))
        {
            pTGD->itgd->maxOutstandingR2T = pITG->maxOutstandingR2T;
            iscsiUpd = TRUE;
        }
        /*
        ** Process the Max Rx Data Segment Length Setting
        */
        if((BIT_TEST(pReq->setmap, KEY17_MAX_RECV_DATASEGMENT_LENGTH))
           && (BIT_TEST(pTGD->i_mask, KEY17_MAX_RECV_DATASEGMENT_LENGTH))
           && (pTGD->itgd->maxRecvDataSegmentLength != pITG->maxRecvDataSegmentLength))
        {
            if( pITG->maxRecvDataSegmentLength >= 512  && pITG->maxRecvDataSegmentLength <= TWO_RAISE_TO_24 -1)
            {
                pTGD->itgd->maxRecvDataSegmentLength = pITG->maxRecvDataSegmentLength;
                iscsiUpd = TRUE;
            }
            else
            {
                retCode = DEINVISCSIPARAM;
            }
        }
        /*
        ** Process the IF Mark INT Setting
        */
        if((BIT_TEST(pReq->setmap, KEY18_IFMARK_INT))
           && (BIT_TEST(pTGD->i_mask, KEY18_IFMARK_INT))
           && (pTGD->itgd->ifMarkInt != pITG->ifMarkInt))
        {
            pTGD->itgd->ifMarkInt = pITG->ifMarkInt;
            iscsiUpd = TRUE;
        }
        /*
        ** Process the OF Mark INT Setting
        */
        if((BIT_TEST(pReq->setmap, KEY19_OFMARK_INT))
           && (BIT_TEST(pTGD->i_mask, KEY19_OFMARK_INT))
           && (pTGD->itgd->ofMarkInt != pITG->ofMarkInt))
        {
            pTGD->itgd->ofMarkInt = pITG->ofMarkInt;
            iscsiUpd = TRUE;
        }
        /*
        ** Process the Header Digest Setting
        */
        if((BIT_TEST(pReq->setmap, KEY20_HEADER_DIGEST))
           && (BIT_TEST(pTGD->i_mask, KEY20_HEADER_DIGEST))
           && (pTGD->itgd->headerDigest != pITG->headerDigest))
        {
            if(pITG->headerDigest == 0 || pITG->headerDigest == 1 || pITG->headerDigest == SET_BITS_7_AND_0)
            {
                pTGD->itgd->headerDigest = pITG->headerDigest;
                iscsiUpd = TRUE;
            }
            else
            {
                retCode = DEINVISCSIPARAM;
            }
        }
        /*
        ** Process the Data Digest Setting
        */
        if((BIT_TEST(pReq->setmap, KEY21_DATA_DIGEST))
           && (BIT_TEST(pTGD->i_mask, KEY21_DATA_DIGEST))
           && (pTGD->itgd->dataDigest != pITG->dataDigest))
        {
            if(pITG->dataDigest == 0 || pITG->dataDigest == 1 || pITG->dataDigest == SET_BITS_7_AND_0 )
            {
                pTGD->itgd->dataDigest = pITG->dataDigest;
                iscsiUpd = TRUE;
            }
            else
            {
                retCode = DEINVISCSIPARAM;
            }
        }
        /*
        ** Process the Authentication Method Setting
        */
        if((BIT_TEST(pReq->setmap, KEY22_AUTHMETHOD))
           && (BIT_TEST(pTGD->i_mask, KEY22_AUTHMETHOD))
           && (pTGD->itgd->authMethod != pITG->authMethod))
        {
            if(pITG->authMethod == 0 || pITG->authMethod == 1 || pITG->authMethod == SET_BITS_7_AND_0)
            {
                pTGD->itgd->authMethod = pITG->authMethod;
                iscsiUpd = TRUE;
            }
            else
            {
                retCode = DEINVISCSIPARAM;
            }
        }
        /*
        ** Process the MTU Size Setting
        */
        if((BIT_TEST(pReq->setmap, KEY23_MTUSIZE))
           && (BIT_TEST(pTGD->i_mask, KEY23_MTUSIZE))
           && (pTGD->itgd->mtuSize != pITG->mtuSize))
        {
            /*
            ** This is for enabling jumbo frames, which is either enable (9K) or disable (1500).
            ** To check & finetune the performance, it will be nice to be able to set different
            ** MTU values & try it out. Thats why, we are only checking for the min allowed value.
            */
            if(pITG->mtuSize >= 1500)
            {
                pTGD->itgd->mtuSize = pITG->mtuSize;
                iscsiUpd = TRUE;
            }
            else
            {
                retCode = DEINVISCSIPARAM;
            }
        }
        /*
        ** Process the Target Alias
        */
        if((BIT_TEST(pReq->setmap, KEY24_TGTALIAS))
           && (BIT_TEST(pTGD->i_mask, KEY24_TGTALIAS))
           && (strcmp((char *)pTGD->itgd->tgtAlias, (char *)pITG->tgtAlias) != 0))
        {
            memset(pTGD->itgd->tgtAlias,0,32);
            strcpy((char *)pTGD->itgd->tgtAlias, (char *)pITG->tgtAlias);
            iscsiUpd = TRUE;
        }
        /*
        ** Validate the setting values in the above - TBD
        */

        if(iscsiUpd)
        {
            /*
            ** Update the front-end
            */
            DEF_UpdRmtTgInfo(pTGD);

            /*
            ** Save config params in NVRAM
            */
            NV_P2UpdateConfig ();
        }
        if(targUpd)
        {
            /*
            ** Update FE record.
            */
            DEF_UpdRmtTarg(pITG->tid, FALSE);
        }
    }
    else
    {
        retCode = DEINVISCSIPARAM;
    }

    return(retCode);
}/* DEF_SetTgInfo */

/**
******************************************************************************
**  DEF_StoreTargetConfigResponse
**  @brief      Store iSCSI Target configuration response
**
**  @param      pRsp - Pointer to MRGETTGINFO_RSP structure
**  @param      ptgd - Pointer to TGD structure
**
**  @return     none
**
**  @attention  Uses and increments response count
**
******************************************************************************
**/
static void
DEF_StoreTargetConfigResponse(MRGETTGINFO_RSP *pRsp, TGD *ptgd)
{
    I_TGD   *rtgdp = &pRsp->ITGInfo[pRsp->count].i_tgd;

    memcpy(rtgdp, ptgd->itgd, sizeof(*rtgdp));
    pRsp->ITGInfo[pRsp->count].configmap = ptgd->i_mask;
#if 0
    rtgdp->ipAddr = htonl(rtgdp->ipAddr);
    rtgdp->ipMask = htonl(rtgdp->ipMask);
    rtgdp->ipGw = htonl(rtgdp->ipGw);
    fprintf(stderr, "DEF_StoreTargetInfo: rtgdp=%p, configmap=%x, addr=%08x\n",
            rtgdp, pRsp->ITGInfo[pRsp->count].configmap, rtgdp->ipAddr);
#endif
    ++pRsp->count;
}

/**
******************************************************************************
**
**  @brief      Get iSCSI Target configuration
**
**  @param      MRP *pMRP
**
**  @return     UINT8 status
**
**  @attention
**
******************************************************************************
**/
UINT8 DEF_GetTgInfo(MR_PKT *pMRP)
{
    UINT8  retCode = DEOK;
    MRGETTGINFO_REQ   *pReq;
    MRGETTGINFO_RSP   *pRsp;

    /*
    ** Get pointer to Parm block address
    */
    pReq = (MRGETTGINFO_REQ *)pMRP->pReq;
    pRsp = (MRGETTGINFO_RSP *)pMRP->pRsp;
    pRsp->header.len = sizeof(MRGETTGINFO_RSP);

    if (pReq->tid == 0xFF)
    {
        int i;

        pRsp->count = 0;
        for (i = 0; i < MAX_TARGETS; i++)
        {
            if (gTDX.tgd[i] != NULL)
            {
                if (gTDX.tgd[i]->itgd != NULL)
                {
                    DEF_StoreTargetConfigResponse(pRsp, gTDX.tgd[i]);
                }
            }
        }
        return retCode;
    }

    /*
    ** Validate the input params
    */
    if ((pReq->tid < MAX_TARGETS)
        && (gTDX.tgd[pReq->tid] != NULL)
        && (gTDX.tgd[pReq->tid]->itgd != NULL)
        && BIT_TEST(gTDX.tgd[pReq->tid]->opt, TARGET_ISCSI))
    {
        pRsp->count = 0;
        DEF_StoreTargetConfigResponse(pRsp, gTDX.tgd[pReq->tid]);
    }
    else
    {
        dumpDLM();
        retCode = DEINVTID;
    }

    return retCode;
}/* DEF_GetTgInfo */

/**
******************************************************************************
**
**  @brief      To provide a standard means of sending MRP to FE to update
**              FE's iSCSI target info
**
**  @param      UINT16 tid
**
**  @return     none
**
**  @attention
**
**
******************************************************************************
**/
void DEF_UpdRmtTgInfo(TGD *pTGD)
{
    MRUPDTGINFO_REQ *pReq;
    MRUPDTGINFO_RSP *pRsp;

    /*
    ** Allocate memory required to send the iSCSI target info request &
    ** response
    */
    pReq = (MRUPDTGINFO_REQ *)s_MallocC(sizeof(MRUPDTGINFO_REQ) + sizeof(MRUPDTGINFO_RSP), __FILE__, __LINE__);
    pRsp = (MRUPDTGINFO_RSP *)(pReq + 1);

    /*
    ** Copy the updated target info to the request
    */
    memcpy(pReq, (void *)pTGD->itgd, sizeof(I_TGD));

    /*
    ** Send the request MRP to the FE
    */
    L$send_packet(pReq,
        sizeof(MRUPDTGINFO_REQ),
        MRUPDTGINFO,
        pRsp,
        sizeof(MRUPDTGINFO_RSP),
        (void *)&DEF_RmtWait,
        (UINT32)K_xpcb);
    /*
    ** Wait until the FE completes the MRP request
    */
    TaskSetMyState(PCB_WAIT_FE_BE_MRP);
    TaskSwitch();

    /*
    ** Free the memory allocated for the MRP request & response
    */
    s_Free((void *)pReq, (sizeof(MRUPDTGINFO_REQ) + sizeof(MRUPDTGINFO_RSP)), __FILE__, __LINE__);
}/* DEF_UpdRmtTgInfo */


/**
******************************************************************************
**
**  @brief      This function will modify the all the RAID 5 (RDD) "Not
**              Mirroring" State, for RAID 5s owned by this controller,
**              as requested.
**
**  @param      mrp - MRP structure
**
**  @return     return status.
**
**  @attention  none
**
******************************************************************************
**/
UINT8 DEF_UpdSID(MR_PKT *pMRP)
{
    MRUPDSID_REQ *pReq;
    MRUPDSID_RSP *pRsp;

    pReq = (MRUPDSID_REQ *)pMRP->pReq;
    pRsp = (MRUPDSID_RSP *)pMRP->pRsp;
    pRsp->header.len = sizeof(MRUPDSID_RSP);

    /*
    ** Update the Server record
    */
    if(gSDX.sdd[pReq->sid]->wwn != pReq->wwn)
    {
        gSDX.sdd[pReq->sid]->wwn = pReq->wwn;
        NV_P2UpdateConfig();
    }

    return(DEOK);
}

/**
******************************************************************************
**
**  @brief      To provide a standard means of sending MRP to FE to update
**              FE's CHAP user info
**
**  @param      UINT16 tid
**
**  @return     none
**
**  @attention
**
**
******************************************************************************
**/
UINT8 DEF_SetChap(MR_PKT *pMRP)
{
    UINT8           retCode = DEOK;
    MRSETCHAP_REQ   *pReqRcv;
    MRSETCHAP_RSP   *pRspRcv;

    MRSETCHAP_REQ   *pReq;
    MRSETCHAP_RSP   *pRsp;
    UINT32 len;
    UINT32 i;
    TGD             *pTGD;

    pReqRcv = (MRSETCHAP_REQ *)(pMRP->pReq);
    pRspRcv = (MRSETCHAP_RSP *)(pMRP->pRsp);

    if (pReqRcv->option == 0xff)
    {
        /*
        ** Process entire list
        */
        for(i = 0; i < pReqRcv->count; i++)
        {
            if((pReqRcv->userInfo[i].tid >= MAX_TARGETS)
               || ((pTGD = T_tgdindx[pReqRcv->userInfo[i].tid]) == NULL)
               || (BIT_TEST(pTGD->opt,TARGET_ISCSI) == 0))
            {
                retCode = DEINVTID;
                return (retCode);
            }
        }
        for(i = 0; i < pReqRcv->count; i++)
        {
            iSCSIAddUserBE(pReqRcv->userInfo[i]);
        }
    }
    else
    {
        /*
        ** Configure single User
        */
        if((pReqRcv->userInfo[0].tid >= MAX_TARGETS)
           || ((pTGD = T_tgdindx[pReqRcv->userInfo[0].tid]) == NULL)
           || (BIT_TEST(pTGD->opt,TARGET_ISCSI) == 0))
        {
            retCode = DEINVTID;
            return (retCode);
        }

        if(pReqRcv->option == 0)
        {
            /*
            ** Add/Modify user info
            */
            iSCSIAddUserBE(pReqRcv->userInfo[0]);
        }
        else if (pReqRcv->option == 1)
        {
            /*
            ** Remove user info
            */
            iSCSIRemoveUserBE(pReqRcv->userInfo[0]);
        }
    }

    /*
    ** Allocate memory required to send the iSCSI target info request &
    ** response
    */
    len = sizeof(MRSETCHAP_REQ) +
        sizeof(MRCHAPCONFIG)*(pReqRcv->count);

    pReq = (MRSETCHAP_REQ *)s_MallocC(len + sizeof(MRSETCHAP_RSP), __FILE__, __LINE__);
    pRsp = (MRSETCHAP_RSP *)((UINT8 *)pReq + len);

    /*
    ** Copy the updated target info to the request
    */
    pReq->count = pReqRcv->count;
    pReq->option = pReqRcv->option;

    for(i = 0; i < pReqRcv->count; i++)
    {
        pReq->userInfo[i].tid = pReqRcv->userInfo[i].tid;
        pReq->userInfo[i].opt = pReqRcv->userInfo[i].opt;
        memcpy(pReq->userInfo[i].sname, pReqRcv->userInfo[i].sname, 256);
        memcpy(pReq->userInfo[i].secret1, pReqRcv->userInfo[i].secret1, 32);
        memcpy(pReq->userInfo[i].secret2, pReqRcv->userInfo[i].secret2, 32);
    }


    /*
    ** Send the request MRP to the FE
    */
    L$send_packet(pReq,
                  len,
                  MRSETCHAPFE,
                  pRsp,
                  sizeof(MRSETCHAP_RSP),
                  (void *)&DEF_RmtWait,
                  (UINT32)K_xpcb);
    /*
    ** Wait until the FE completes the MRP request
    */
    TaskSetMyState(PCB_WAIT_FE_BE_MRP);
    TaskSwitch();


    if(pRsp->header.status == DEOK)
    {
        /*
        ** Save config params in NVRAM
        */
        NV_P2UpdateConfig ();
    }
    pRspRcv->header.status = pRsp->header.status;

    /*
    ** Free the memory allocated for the MRP request & response
    */
    s_Free((void *)pReq, (len + sizeof(MRSETCHAP_RSP)), __FILE__, __LINE__);

    return (retCode);

}/* DEF_SetChap */

UINT8 DEF_UpdChapInfo(I_TGD *pITGD)
{
    UINT8            retCode = DEOK;
    UINT32           len;
    UINT32           i = 0;
    MRSETCHAP_REQ   *pReq;
    MRSETCHAP_RSP   *pRsp;
    CHAPINFO        *pUserInfo;

    if(pITGD == NULL)
        return(retCode);

    if (pITGD->numUsers == 0)
        return(retCode);

    /*
    ** Allocate memory required to send the iSCSI target info request &
    ** response
    */
    len = sizeof(MRSETCHAP_REQ) +
        sizeof(MRCHAPCONFIG)*(pITGD->numUsers);

    pReq = (MRSETCHAP_REQ *)s_MallocC(len + sizeof(MRSETCHAP_RSP), __FILE__, __LINE__);
    pRsp = (MRSETCHAP_RSP *)((UINT8 *)pReq + len);

    /*
    ** Copy the updated target info to the request
    */
    pReq->count = pITGD->numUsers;
    pReq->option = 0xFF;
    pUserInfo = pITGD->chapInfo;
    while (1)
    {
        if (pUserInfo == NULL)
        {
            break;
        }
        pReq->userInfo[i].tid = pITGD->tid;
        pReq->userInfo[i].opt = 2;
        memcpy(pReq->userInfo[i].sname, pUserInfo->sname, 320);
        pUserInfo = pUserInfo->fthd;
        i++;
    }

    /*
    ** Send the request MRP to the FE
    */
    L$send_packet(pReq,
                  len + sizeof(MRSETCHAP_RSP),
                  MRSETCHAPFE,
                  pRsp,
                  sizeof(MRSETCHAP_RSP),
                  (void *)&DEF_RmtWait,
                  (UINT32)K_xpcb);
    /*
    ** Wait until the FE completes the MRP request
    */
    TaskSetMyState(PCB_WAIT_FE_BE_MRP);
    TaskSwitch();

    /*
    ** Free the memory allocated for the MRP request & response
    */
    s_Free((void *)pReq, (len + sizeof(MRSETCHAP_RSP)), __FILE__, __LINE__);

    return (retCode);

}/* DEF_UpdChapInfo */

/**
******************************************************************************
**
**  @brief      Get CHAP User Info for a Target
**
**  @param      MRP *pMRP
**
**  @return     UINT8 status
**
**  @attention
**
******************************************************************************
**/
UINT8 DEF_GetChap(MR_PKT *pMRP)
{
    UINT8  retCode = DEOK;
    MRGETCHAP_REQ   *pReq;
    MRGETCHAP_RSP   *pRsp;
    CHAPINFO        *pInfo;
    UINT32          i=0;

    /*
    ** Get pointer to Parm block address
    */
    pReq = (MRGETCHAP_REQ *)(pMRP->pReq);
    pRsp = (MRGETCHAP_RSP *)(pMRP->pRsp);
    pRsp->header.len = sizeof(MRGETCHAP_RSP);

    if(pReq->tid == 0xFF)
    {
        pRsp->count = 0;
        /*
        ** Get Info on all targets
        */
        for (i = 0; i < MAX_TARGETS; i++)
        {
            if(gTDX.tgd[i] != NULL)
            {
                if(gTDX.tgd[i]->itgd != NULL)
                {
                    pInfo = gTDX.tgd[i]->itgd->chapInfo;
                    /*
                    ** Copy the CHAP User info to the response buffer
                    */
                    while (pInfo != NULL)
                    {
                        pRsp->userInfo[(pRsp->count)].tid = gTDX.tgd[i]->tid;
                        strcpy((char *)pRsp->userInfo[(pRsp->count)].sname, (char *)pInfo->sname);
                        (pRsp->count)++;
                        pInfo = pInfo->fthd;
                        pRsp->header.len += sizeof(MRCHAPINFO);
                    }
                }
            }
        }
        return (retCode);
    }

    /*
    ** Validate the input params
    */
    if((pReq->tid < MAX_TARGETS)
       && (gTDX.tgd[pReq->tid] != NULL)
       && (gTDX.tgd[pReq->tid]->itgd != NULL)
#if 1   /* for DEBUG */
        )
#else
        && BIT_TEST(gTDX.tgd[pReq->tid]->opt, TARGET_ISCSI)
#endif
        {
            pInfo = gTDX.tgd[pReq->tid]->itgd->chapInfo;
            pRsp->count = 0;

            /*
            ** Copy the CHAP User info to the response buffer
            */
            while (pInfo != NULL)
            {
                pRsp->userInfo[(pRsp->count)].tid = pReq->tid;
//            memcpy(pRsp->userInfo[(pRsp->count)].sname , pInfo->sname, 256);
                strcpy((char *)pRsp->userInfo[(pRsp->count)].sname, (char *)pInfo->sname);
                (pRsp->count)++;
                pInfo = pInfo->fthd;
            }
            pRsp->header.len += (pRsp->count) * sizeof(MRCHAPINFO);
        }
    else
    {
        retCode = DEINVTID;
    }

    return(retCode);
}/* DEF_GetChap */

/**
******************************************************************************
**
**  @brief      Configuration of ISNS Server
**
**  @param      MRP *pMRP
**
**  @return     UINT8 status
**
**  @attention
**
******************************************************************************
**/

UINT8 DEF_iSNSSetInfo(MR_PKT *pMRP)
{
    UINT32 i = 0;
    UINT32 j = 0;
    UINT32 nset = 0;
    MRSETISNSINFO_REQ *pReq;
    MRSETISNSINFO_RSP *pRsp;
    /*
    ** Get the request and response pointers
    */
    pReq = (MRSETISNSINFO_REQ *)(pMRP->pReq);
    pRsp = (MRSETISNSINFO_RSP *)(pMRP->pRsp);

    nset = ((pMRP->reqLen - sizeof(MRSETISNSINFO_REQ)) / sizeof(MRISNS_SERVER_INFO));
    pRsp->header.status = DEOK;
    /*
    ** These checkings are necessarey for this release
    */
    if(BIT_TEST(pReq->flags, ISNS_CF_AUTO))
    {
        pRsp->header.status = DEINVOPT;
    }
    else if(nset > MAX_ISNS_SERVERS)
    {
        pRsp->header.status = DEINVOPT;
    }
    else
    {
        /*
        ** For now - return error if protocol is set to UDP or
        ** if IP is 255.255.255.255 or if the same IP is comfigured
        ** multiple times.
        */
        for(i = 0; ((i < nset) && (pRsp->header.status == DEOK)); i++)
        {
            if((BIT_TEST(pReq->serverdata[i].flags, ISNS_SF_UDPTCP))
                    || (pReq->serverdata[i].ip == 0xffffffff))
            {
                pRsp->header.status = DEINVOPT;
                continue;
            }
            for(j = 0; j < nset; j++)
            {
                if(i == j)
                    continue;
                if((pReq->serverdata[i].ip != 0x0)
                    && (pReq->serverdata[i].ip == pReq->serverdata[j].ip))
                {
                    pRsp->header.status = DEINVOPT;
                    break;
                }
            }
        }
        if(pRsp->header.status == DEOK)
        {
            gISD.gflags = (pReq->flags & ~(1 << ISNS_GF_MASTER));

            if(pMRP->reqLen > sizeof(MRSETISNSINFO_REQ))
            {
                for(i = 0; i < nset; i++)
                {
                    gISD.srv[i].ip      = pReq->serverdata[i].ip;
                    gISD.srv[i].port    = pReq->serverdata[i].port;
                    gISD.srv[i].sflags  = pReq->serverdata[i].flags;

                    if((gISD.srv[i].port == 0x0) && (gISD.srv[i].ip != 0x0))
                        gISD.srv[i].port = ISNS_PORT;
                }
                while(i < MAX_ISNS_SERVERS)
                {
                    gISD.srv[i++].ip    = 0x0;
                }
            }
            if((pRsp->header.status = DEF_iSNSUpdateFE()) == DEOK)
            {
                /*
                ** Save config params in NVRAM
                */
                NV_P2UpdateConfig ();
            }
        }
    }
    return (pRsp->header.status);
}/* DEF_iSNSSetInfo */


/**
******************************************************************************
**
**  @brief      Get ISNS info
**
**  @param      MRP *pMRP
**
**  @return     UINT8 status
**
**  @attention
**
******************************************************************************
**/

UINT8 DEF_GetIsnsInfo(MR_PKT *pMRP)
{

    MRGETISNSINFO_RSP *pRsp;
    /*
    ** Get pointer to Parm block address
    */
    pRsp = (MRGETISNSINFO_RSP *)(pMRP->pRsp);

    if(pMRP->rspLen > sizeof(MRGETISNSINFO_RSP))
    {
        UINT32 i = 0;

        pRsp->rspdata.nset = ((pMRP->rspLen - sizeof(MRGETISNSINFO_RSP)) / sizeof(MRISNS_SERVER_INFO));
        for(i = 0; i < pRsp->rspdata.nset; i++)
        {
            pRsp->rspdata.serverdata[i].ip = gISD.srv[i].ip;
            pRsp->rspdata.serverdata[i].port = gISD.srv[i].port;
            pRsp->rspdata.serverdata[i].flags = gISD.srv[i].sflags;
        }
    }

    pRsp->rspdata.flags = gISD.gflags;
    pRsp->header.len = sizeof(MRGETISNSINFO_RSP)
                                    + MAX_ISNS_SERVERS*sizeof(MRISNS_SERVER_INFO);
    pRsp->header.status = DEOK;
    return (pRsp->header.status);
}

/**
******************************************************************************
**
**  @brief      Restore isns variable at FE
**
**  @param      none
**
**  @return     UINT8 status
**
**  @attention
**
******************************************************************************
**/

UINT8 DEF_iSNSUpdateFE(void)
{
    UINT32 i = 0;
    UINT8  retCode = DEOK;
    MRSETISNSINFO_REQ *pReq;
    MRSETISNSINFO_RSP *pRsp;


    if(!BIT_TEST(K_ii.status, II_MASTER)
                && !BIT_TEST(K_ii.status, II_SLAVE))
    {
        return (retCode);
    }
    /*
    ** Allocate memory required to send the iSCSI target info request &
    ** response
    */

    pReq = (MRSETISNSINFO_REQ *)s_MallocC(sizeof(MRSETISNSINFO_REQ)
                                + MAX_ISNS_SERVERS*sizeof(MRISNS_SERVER_INFO)
                                + sizeof(MRSETISNSINFO_RSP), __FILE__, __LINE__);

    pRsp = (MRSETISNSINFO_RSP *)((UINT8 *)pReq + (sizeof(MRSETISNSINFO_REQ)
                                    + MAX_ISNS_SERVERS*sizeof(MRISNS_SERVER_INFO)));

    pReq->flags = gISD.gflags;
    if(BIT_TEST(K_ii.status, II_MASTER))
    {
        BIT_SET(pReq->flags, ISNS_GF_MASTER);
    }
    else {
        BIT_CLEAR(pReq->flags, ISNS_GF_MASTER);
    }
    for(i = 0; i < MAX_ISNS_SERVERS; i++)
    {
        pReq->serverdata[i].ip      = gISD.srv[i].ip;
        pReq->serverdata[i].port    = gISD.srv[i].port;
        pReq->serverdata[i].flags   = gISD.srv[i].sflags;
    }

    /*
    ** Send the request MRP to the FE
    */
    L$send_packet(pReq,
                  sizeof(MRSETISNSINFO_REQ) + MAX_ISNS_SERVERS*sizeof(MRISNS_SERVER_INFO),
                  MRSETISNSINFOFE,
                  pRsp,
                  sizeof(MRSETISNSINFO_RSP),
                  (void *)&DEF_RmtWait,
                  (UINT32)K_xpcb);
    /*
    ** Wait until the FE completes the MRP request
    */
    TaskSetMyState(PCB_WAIT_FE_BE_MRP);
    TaskSwitch();

    retCode = pRsp->header.status;
    /*
    ** Free the memory allocated for the MRP request & response
    */
    s_Free((void *)pReq, (sizeof(MRSETISNSINFO_REQ)
                        + MAX_ISNS_SERVERS*sizeof(MRISNS_SERVER_INFO)
                        + sizeof(MRSETISNSINFO_RSP)), __FILE__, __LINE__);

    return (retCode);
}/* DEF_iSNSUpdateFE */

#endif /* BACKEND */

#ifdef FRONTEND

extern IDD *gIDX[MAX_PORTS][MAX_DEV];

/**
******************************************************************************
**
**  @brief      To provide a standard means of processing the iSCSI target
**              info update from the BE
**
**  @param      mrp - MRP structure
**
**  @return     return status.
**
**  @attention  none
**
******************************************************************************
**/
UINT8 DEF_UpdateTgInfo(MR_PKT *pMRP)
{
    UINT8  retCode = DEOK;
    MRUPDTGINFO_REQ *pReq;
    MRUPDTGINFO_RSP *pRsp;

    /*
    ** Get pointer to Parm block address
    */
    pReq = (MRUPDTGINFO_REQ *)(pMRP->pReq);
    pRsp = (MRUPDTGINFO_RSP *)(pMRP->pRsp);
    pRsp->header.len = sizeof(MRUPDTGINFO_RSP);

    /*
    ** Call the function which will convert the given packed structure
    ** to a params struct which will be used in iSCSI text params
    ** negotiation
    */
    iscsiGenerateParameters(pReq);

    return (retCode);
}


/**
******************************************************************************
**
**  @brief      To provide a standard means of processing the CHAP user
**              update from the BE
**
**  @param      mrp - MRP structure
**
**  @return     return status.
**
**  @attention  none
**
******************************************************************************
**/
UINT8 DEF_SetChapInfo(MR_PKT *pMRP)
{
    UINT8         retCode = DEOK;
    UINT16        i=0;
    MRSETCHAP_REQ *pReq;
    MRSETCHAP_RSP *pRsp;

    /*
    ** Get pointer to Parm block address
    */
    pReq = (MRSETCHAP_REQ *)(pMRP->pReq);
    pRsp = (MRSETCHAP_RSP *)(pMRP->pRsp);
    pRsp->header.len = sizeof(MRSETCHAP_RSP);

    /*
    ** Call the function which will convert the given packed structure
    ** to a params struct which will be used in iSCSI text params
    ** negotiation
    */
//    iscsiGenerateParameters(pReq);
    if (pReq->option == 0xff)
    {
        /*
        ** Process entire list
        */
        for(i = 0; i < pReq->count; i++)
        {
            iSCSIAddUser(pReq->userInfo[i]);
        }
    }
    else
    {
        /*
        ** Configure single User
        */
        if(pReq->option == 0)
        {
            /*
            ** Add/Modify user info
            */
            iSCSIAddUser(pReq->userInfo[0]);
        }
        else if (pReq->option == 1)
        {
            /*
            ** Remove user info
            */
            iSCSIRemoveUser(pReq->userInfo[0]);
        }
    }
    return (retCode);
}

static void copySessions(MRGETSESSIONS_RSP *pRsp, TAR *pTar)
{
    SESSION            *pSession = NULL;
    CONNECTION         *pConn = NULL;
    UINT32              i=0;
    UINT32              j=0;

    for (i = pRsp->numSessions, pSession = (SESSION*)pTar->portID; pSession != NULL; pSession = pSession->pNext, i++)
    {
        pRsp->sessionInfo[i].tid = pSession->tid;
        pRsp->sessionInfo[i].sid = pSession->sid;
        pRsp->sessionInfo[i].state = pSession->ssnState;

        pRsp->sessionInfo[i].maxConnections = pSession->params.maxConnections;
        pRsp->sessionInfo[i].targetPortalGroupTag = pSession->params.targetPortalGroupTag;
        pRsp->sessionInfo[i].initialR2T = pSession->params.initialR2T;
        pRsp->sessionInfo[i].immediateData = pSession->params.immediateData;
        pRsp->sessionInfo[i].maxBurstLength = pSession->params.maxBurstLength;
        pRsp->sessionInfo[i].firstBurstLength = pSession->params.firstBurstLength;
        pRsp->sessionInfo[i].defaultTime2Wait = pSession->params.defaultTime2Wait;
        pRsp->sessionInfo[i].defaultTime2Retain = pSession->params.defaultTime2Retain;
        pRsp->sessionInfo[i].maxOutstandingR2T = pSession->params.maxOutstandingR2T;
        pRsp->sessionInfo[i].dataPDUInOrder = pSession->params.dataPDUInOrder;
        pRsp->sessionInfo[i].dataSequenceInOrder = pSession->params.dataSequenceInOrder;
        pRsp->sessionInfo[i].errorRecoveryLevel = pSession->params.errorRecoveryLevel;
        pRsp->sessionInfo[i].sessionType = pSession->params.sessionType;
        pRsp->sessionInfo[i].paramMap = pSession->params.paramMap;
        pRsp->sessionInfo[i].paramSentMap = pSession->params.paramSentMap;

        memset(pRsp->sessionInfo[i].targetName,0,256);
        memset(pRsp->sessionInfo[i].initiatorName,0,256);
        memset(pRsp->sessionInfo[i].targetAlias,0,256);
        memset(pRsp->sessionInfo[i].initiatorAlias,0,256);
        memset(pRsp->sessionInfo[i].targetAddress,0,256);

        strcpy((char *)pRsp->sessionInfo[i].targetName, (char *)pSession->params.targetName);
        strcpy((char *)pRsp->sessionInfo[i].initiatorName, (char *)pSession->params.initiatorName);
        strcpy((char *)pRsp->sessionInfo[i].targetAlias, (char *)pSession->params.targetAlias);
        strcpy((char *)pRsp->sessionInfo[i].initiatorAlias, (char *)pSession->params.initiatorAlias);
        strcpy((char *)pRsp->sessionInfo[i].targetAddress, (char *)pSession->params.targetAddress);

        for (j = 0, pConn = pSession->pCONN; pConn != NULL; pConn = pConn->pNext, j++)
        {
            pRsp->sessionInfo[i].connInfo[j].cid = pConn->cid;
            pRsp->sessionInfo[i].connInfo[j].state = pConn->state;

            pRsp->sessionInfo[i].connInfo[j].numPduRcvd = pConn->numPduRcvd;
            pRsp->sessionInfo[i].connInfo[j].numPduSent = pConn->numPduSent;
            pRsp->sessionInfo[i].connInfo[j].totalReads = pConn->totalReads;
            pRsp->sessionInfo[i].connInfo[j].totalWrites = pConn->totalWrites;

            pRsp->sessionInfo[i].connInfo[j].chap_A = pConn->params.chap_A;
            pRsp->sessionInfo[i].connInfo[j].ifMarker = pConn->params.ifMarker;
            pRsp->sessionInfo[i].connInfo[j].ofMarker = pConn->params.ofMarker;
            pRsp->sessionInfo[i].connInfo[j].maxRecvDataSegmentLength = pConn->params.maxRecvDataSegmentLength;
            pRsp->sessionInfo[i].connInfo[j].maxSendDataSegmentLength = pConn->params.maxSendDataSegmentLength;
            pRsp->sessionInfo[i].connInfo[j].paramMap = pConn->params.paramMap;
            pRsp->sessionInfo[i].connInfo[j].paramSentMap = pConn->params.paramSentMap;

            memset(pRsp->sessionInfo[i].connInfo[j].headerDigest,0,256);
            memset(pRsp->sessionInfo[i].connInfo[j].dataDigest,0,256);
            memset(pRsp->sessionInfo[i].connInfo[j].authMethod,0,256);

            strcpy((char *)pRsp->sessionInfo[i].connInfo[j].headerDigest, (char *)pConn->params.headerDigest.strval);
            strcpy((char *)pRsp->sessionInfo[i].connInfo[j].dataDigest, (char *)pConn->params.dataDigest.strval);
            strcpy((char *)pRsp->sessionInfo[i].connInfo[j].authMethod, (char *)pConn->params.authMethod.strval);
        }
        pRsp->sessionInfo[i].numConnections = j;
    }
    pRsp->numSessions = i;
}

UINT8 DEF_GetSessions(MR_PKT *pMRP)
{
    MRGETSESSIONS_REQ  *pReq;
    MRGETSESSIONS_RSP  *pRsp;
    UINT32              i;
    TAR                *pTar;

    pReq = (MRGETSESSIONS_REQ *)(pMRP->pReq);
    pRsp = (MRGETSESSIONS_RSP*)(pMRP->pRsp);
    pRsp->numSessions = 0;
    if (pReq->tid == 0xFF)
    {
        /* Retrieve Sessions on all Targets */
        for (i = 0; i < MAX_TARGETS; i++)
        {
            /* Dont' extract the session related to ICL */
            if (ICL_TARGET(i))
            {
                continue;
            }
            if (gTDX.tgd[i] != NULL && gTDX.tgd[i]->itgd != NULL)
            {
                if (BIT_TEST(gTDX.tgd[i]->opt, TARGET_ISCSI) != 0)
                {
                    pTar = fsl_get_tar(gTDX.tgd[i]->itgd->tid);
                    if (pTar != NULL)
                    {
                        copySessions(pRsp, pTar);
                    }
                }
            }
        }
    }
    else
    {
        if (pReq->tid >= MAX_TARGETS ||
            gTDX.tgd[pReq->tid] == NULL ||
            BIT_TEST(gTDX.tgd[pReq->tid]->opt, TARGET_ISCSI) == 0)
        {
            pRsp->header.len = sizeof(MRGETSESSIONS_RSP);
            return DEINVTID;
        }
        pTar = fsl_get_tar(pReq->tid);
        if (pTar != NULL)
        {
            copySessions(pRsp, pTar);
        }

    }
    pRsp->header.len = sizeof(MRGETSESSIONS_RSP) + ((pRsp->numSessions) * sizeof(MRSESSION));
    return DEOK;
}

UINT8 DEF_GetSessionsOnServer(MR_PKT *pMRP)
{
    MRGETSESSIONSPERSERVER_REQ  *pReq;
    MRGETSESSIONSPERSERVER_RSP  *pRsp;
    SESSION                     *pSession;
    UINT32                       i;
    UINT32                       nsession = 0;
    TAR                         *pTar;

    pReq = (MRGETSESSIONSPERSERVER_REQ *)(pMRP->pReq);
    pRsp = (MRGETSESSIONSPERSERVER_RSP*)(pMRP->pRsp);
    pRsp->numSessions = 0;

    /* Go over all Targets */
    for (i = 0; i < MAX_TARGETS; i++)
    {
        if (gTDX.tgd[i] != NULL && gTDX.tgd[i]->itgd != NULL)
        {
            if(BIT_TEST(gTDX.tgd[i]->opt, TARGET_ISCSI) != 0)
            {
                /* Get all sessions on the target */
                pTar = fsl_get_tar(gTDX.tgd[i]->itgd->tid);
                if (pTar != NULL)
                {

                    for (pSession = (SESSION*)pTar->portID; pSession != NULL; pSession = pSession->pNext)
                    {
                        if (strcmp((char *)pReq->sname, (char *)pSession->params.initiatorName)==0 ||
                            strcmp((char *)pReq->sname, (char *)pSession->params.initiatorAlias)==0)
                        {
                            /* Server name matched. Copy the session info to response. */
                            copySession(&(pRsp->sessionInfo[nsession]), pSession);
                            nsession++;
                        }
                    }
                }
            }
        }
    }
    pRsp->numSessions = nsession;
    fprintf(stderr,"STATS ON SERVER %d sessions",pRsp->numSessions);
    return DEOK;
}

static void copySession(MRSESSION *pSessionDest, SESSION *pSession)
{

    CONNECTION      *pConn = NULL;
    UINT32 j = 0;

    pSessionDest->tid = pSession->tid;

    pSessionDest->sid = pSession->sid;
    pSessionDest->state = pSession->ssnState;

    pSessionDest->maxConnections = pSession->params.maxConnections;
    pSessionDest->targetPortalGroupTag = pSession->params.targetPortalGroupTag;
    pSessionDest->initialR2T = pSession->params.initialR2T;
    pSessionDest->immediateData = pSession->params.immediateData;
    pSessionDest->maxBurstLength = pSession->params.maxBurstLength;
    pSessionDest->firstBurstLength = pSession->params.firstBurstLength;
    pSessionDest->defaultTime2Wait = pSession->params.defaultTime2Wait;
    pSessionDest->defaultTime2Retain = pSession->params.defaultTime2Retain;
    pSessionDest->maxOutstandingR2T = pSession->params.maxOutstandingR2T;
    pSessionDest->dataPDUInOrder = pSession->params.dataPDUInOrder;
    pSessionDest->dataSequenceInOrder = pSession->params.dataSequenceInOrder;
    pSessionDest->errorRecoveryLevel = pSession->params.errorRecoveryLevel;
    pSessionDest->sessionType = pSession->params.sessionType;
    pSessionDest->paramMap = pSession->params.paramMap;
    pSessionDest->paramSentMap = pSession->params.paramSentMap;

    memset(pSessionDest->targetName,0,256);
    memset(pSessionDest->initiatorName,0,256);
    memset(pSessionDest->targetAlias,0,256);
    memset(pSessionDest->initiatorAlias,0,256);
    memset(pSessionDest->targetAddress,0,256);

    strcpy((char *)pSessionDest->targetName, (char *)pSession->params.targetName);
    strcpy((char *)pSessionDest->initiatorName, (char *)pSession->params.initiatorName);
    strcpy((char *)pSessionDest->targetAlias, (char *)pSession->params.targetAlias);
    strcpy((char *)pSessionDest->initiatorAlias, (char *)pSession->params.initiatorAlias);
    strcpy((char *)pSessionDest->targetAddress, (char *)pSession->params.targetAddress);

    for (j = 0, pConn = pSession->pCONN; pConn != NULL; pConn = pConn->pNext, j++)
    {
        pSessionDest->connInfo[j].cid = pConn->cid;
        pSessionDest->connInfo[j].state = pConn->state;

        pSessionDest->connInfo[j].numPduRcvd = 0;
        pSessionDest->connInfo[j].numPduSent = 0;
        pSessionDest->connInfo[j].totalReads = 0;
        pSessionDest->connInfo[j].totalWrites = 0;

        pSessionDest->connInfo[j].chap_A = pConn->params.chap_A;
        pSessionDest->connInfo[j].ifMarker = pConn->params.ifMarker;
        pSessionDest->connInfo[j].ofMarker = pConn->params.ofMarker;
        pSessionDest->connInfo[j].maxRecvDataSegmentLength = pConn->params.maxRecvDataSegmentLength;
        pSessionDest->connInfo[j].maxSendDataSegmentLength = pConn->params.maxSendDataSegmentLength;
        pSessionDest->connInfo[j].paramMap = pConn->params.paramMap;
        pSessionDest->connInfo[j].paramSentMap = pConn->params.paramSentMap;

        memset(pSessionDest->connInfo[j].headerDigest,0,256);
        memset(pSessionDest->connInfo[j].dataDigest,0,256);
        memset(pSessionDest->connInfo[j].authMethod,0,256);

        strcpy((char *)pSessionDest->connInfo[j].headerDigest, (char *)pConn->params.headerDigest.strval);
        strcpy((char *)pSessionDest->connInfo[j].dataDigest, (char *)pConn->params.dataDigest.strval);
        strcpy((char *)pSessionDest->connInfo[j].authMethod, (char *)pConn->params.authMethod.strval);
    }
    pSessionDest->numConnections = j;
}

extern void dumpTMT(UINT16 port);

UINT8 DEF_GetIDDInfo(MR_PKT *pMRP)
{
    UINT16            i;
    MRGETIDDINFO_RSP *pRsp;
    UINT16            count = 0;

    pRsp = (MRGETIDDINFO_RSP *)(pMRP->pRsp);

    for (i = 0; i < MAX_PORTS; i++)
    {
        if(BIT_TEST(ispmap, i))
        {
            dumpTMT(i);
#if ISCSI_CODE
            if(BIT_TEST(iscsimap, i))
            {
                UINT16            j;
                IDD              *pIDD;

                for (j = 0; j < MAX_PORTS; j++)
                {
                    pIDD = getIdd(i,j);
                    if(pIDD != NULL)
                    {
                        /* Copy IDD */
                        memcpy(&(pRsp->iddInfo[count]), pIDD, sizeof(IDD));
                        count++;
                    }
                }
            }
#endif
        }
    }

    pRsp->count = count;
    pRsp->header.len = sizeof(MRGETIDDINFO_RSP) + (count * sizeof(MRIDD));
    return DEOK;
}


#endif /* FRONTEND */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
