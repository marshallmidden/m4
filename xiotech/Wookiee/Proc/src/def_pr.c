/* $Id: def_pr.c 159663 2012-08-22 15:36:42Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       def_pr.c
**
**  @brief      Define configuration
**
**  To provide a common means of handling the define persistant configuration.
**
**  Copyright (c) 1996-2010 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/

#include "pr.h"

#include <stdio.h>
#include <time.h>
#include "daml.h"
#include "def.h"
#include "defbe.h"
#include "DEF_Workset.h"
#include "error.h"
#include "globalOptions.h"
#include "LOG_Defs.h"
#include "LL_LinuxLinkLayer.h"
#include "MR_Defs.h"
#include "nvram.h"
#include "pdd.h"
#include "qu.h"
#include "RL_RDD.h"
#include "vdd.h"
#include "ses.h"
#include "string.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "misc.h"

#ifdef FRONTEND
#include "vdmt.h"
#endif

/*
******************************************************************************
** Private variables
******************************************************************************
*/
#ifdef BACKEND
MRSETPRES_REQ* gRsvData[MAX_VIRTUAL_DISKS];
#endif

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
extern void L$send_packet(void *pReq, UINT32 reqSize, UINT32 cmdCode,
                          void *pRsp, UINT32 rspLen, void *pCallback,
                          UINT32 param);
#ifdef FRONTEND
extern VDMT* MAG_VDMT_dir[MAX_VIRTUALS];
#endif

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
#ifdef BACKEND
void InitPR (void);
void DEF_NewRsvNode(MRSETPRES_REQ* nvrec);
void DEF_ClearRsvAll(void);
UINT8 DEF_CfgRetrieve(MR_PKT* pMRP);
void DEF_RemoveSrvKeys(UINT16 sid);
void DEF_RemoveKey(UINT16 vid, UINT16 sid);
UINT8 prDelKey(UINT16 vid, UINT16 sid);
void prUpdateFE(UINT16 sid, UINT16 vid, UINT16 flags);
#endif

void prDump(UINT16 vid, const char *file, UINT32 line, const char *func);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

#ifdef BACKEND
void InitPR (void)
{
    UINT32 i;

    for (i = 0; i < MAX_VIRTUAL_DISKS; i++)
    {
        gRsvData[i] = NULL;
    }
}

/**
******************************************************************************
**
**  @brief      Clears all persistent reservation info in BE. Called
**              before restoring NVRAM data.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void DEF_ClearRsvAll(void)
{
    UINT32 i;

    for (i = 0; i < MAX_VIRTUAL_DISKS; i++)
    {
        if(gRsvData[i])
        {
            s_Free(gRsvData[i], (sizeof(MRSETPRES_REQ) + gRsvData[i]->regCount * sizeof(MRREGKEY)), __FILE__, __LINE__);
        }
        gRsvData[i] = NULL;
    }
}

/**
******************************************************************************
**
**  @brief      Updates persistent reservation info in BE. Called
**              during restoring NVRAM data.
**
**  @param      nvrec - pointer to persistent reservation record.
**
**  @return     none
**
******************************************************************************
**/
void DEF_NewRsvNode(MRSETPRES_REQ* nvrec)
{
    if(gRsvData[nvrec->vid])
    {
        s_Free(gRsvData[nvrec->vid], (sizeof(MRSETPRES_REQ) +
             gRsvData[nvrec->vid]->regCount * sizeof(MRREGKEY)), __FILE__, __LINE__);
        gRsvData[nvrec->vid] = NULL;
    }

    if(nvrec->regCount > 0)
    {
        gRsvData[nvrec->vid] = (MRSETPRES_REQ *)s_MallocC(sizeof(MRSETPRES_REQ) +
                                     nvrec->regCount * sizeof(MRREGKEY), __FILE__, __LINE__);
        memcpy(gRsvData[nvrec->vid], nvrec, sizeof(MRSETPRES_REQ) + nvrec->regCount * sizeof(MRREGKEY));
    }
#ifndef PERF
    prDump(nvrec->vid, __FILE__, __LINE__, __func__);
#endif
}

/**
******************************************************************************
**
**  @brief      Process MRP to retrieve BE persistent reservation info.
**
**  @param      pMRP - MRP pointer
**
**  @return     DEOK
**
******************************************************************************
**/
UINT8 DEF_CfgRetrieve(MR_PKT* pMRP)
{
    MRRETRIEVEPR_REQ* pReq   = (MRRETRIEVEPR_REQ*)(pMRP->pReq);
    MRRETRIEVEPR_RSP* pRsp   = (MRRETRIEVEPR_RSP*)(pMRP->pRsp);

    if(gRsvData[pReq->vid] != NULL)
    {
        memcpy(((UINT8 *)pRsp) + sizeof(MR_HDR_RSP), gRsvData[pReq->vid],
                         sizeof(MRSETPRES_REQ) + gRsvData[pReq->vid]->regCount * sizeof(MRREGKEY));
    }
    else
    {
        pRsp->regCount = 0;
        pRsp->sid = 0xffff;
    }

#ifndef PERF
    prDump(pReq->vid, __FILE__, __LINE__, __func__);
#endif
    pRsp->header.status = DEOK;
    return (DEOK);
}

/**
******************************************************************************
**
**  @brief      Process MRP to update BE persistent reservation info.
**
**  @param      pMRP - MRP pointer
**
**  @return     DEOK
**
******************************************************************************
**/
UINT8 DEF_UpdatePres (MR_PKT* pMRP)
{
    MRSETPRES_REQ* pReq   = (MRSETPRES_REQ*)(pMRP->pReq);
    MRSETPRES_RSP* pRsp   = (MRSETPRES_RSP*)(pMRP->pRsp);

    if(gRsvData[pReq->vid])
    {
        s_Free(gRsvData[pReq->vid], (sizeof(MRSETPRES_REQ) +
             gRsvData[pReq->vid]->regCount * sizeof(MRREGKEY)), __FILE__, __LINE__);
        gRsvData[pReq->vid] = NULL;
    }

    if(pReq->regCount > 0)
    {
        gRsvData[pReq->vid]= (MRSETPRES_REQ *)s_MallocC(sizeof(MRSETPRES_REQ) +
                                         pReq->regCount * sizeof(MRREGKEY), __FILE__, __LINE__);
        memcpy(gRsvData[pReq->vid], pReq, sizeof(MRSETPRES_REQ) + pReq->regCount * sizeof(MRREGKEY));
    }

#ifndef PERF
    prDump(pReq->vid, __FILE__, __LINE__, __func__);
#endif
    /*
    ** Update NVRAM part II.
    */
    NV_P2UpdateConfig();

    pRsp->header.status = DEOK;
    return (DEOK);
}

/**
******************************************************************************
**
**  @brief      Searches for all the PRR keys associated with the given server
**              and removes them from the configuration. If any key is found and
**              removed, a broadcast msg is sent to CCB to update the FE
**              processors in the DSC.
**
**  @param      UINT16 sid
**
**  @return     none
**
******************************************************************************
**/
void DEF_RemoveSrvKeys(UINT16 sid)
{
    UINT16 vid;
    UINT8 updateFE = FALSE;
    /*
    ** Find if the association has PRR key registered. If yes, remove the key
    ** and inform the FE(s) to update their local structs. For this
    ** we send a broadcase msg which will result in a CCB-FE MRP on all DSNs.
    */
    for(vid = 0; vid < MAX_VIRTUAL_DISKS; vid++)
    {
        if(prDelKey(vid,sid))
        {
            updateFE = TRUE;
        }
    }
    if(updateFE == TRUE)
    {
        prUpdateFE(sid, vid, (1<<MRPRR_SERVERDEL));
    }
}

/**
******************************************************************************
**
**  @brief      Searches the PRR key with the given inputs and if found,
**              removes it from the configuration. If the key is found and
**              removed, a broadcast msg is sent to CCB to update the FE
**              processors in the DSC.
**
**  @param      UINT16 vid
**              UINT16 sid
**
**  @return     none
**
******************************************************************************
**/
void DEF_RemoveKey(UINT16 vid, UINT16 sid)
{
    /*
    ** Find if the association has PRR key registered. If yes, remove the key
    ** and inform the FE(s) to update their local structs. For this
    ** we send a broadcase msg which will result in a CCB-FE MRP on all DSNs.
    */
    if(prDelKey(vid,sid))
    {
        prUpdateFE(sid, vid, (1<<MRPRR_ASSOCDEL));
    }
}

/**
******************************************************************************
**
**  @brief      Searches the PRR key with the given inputs and if found,
**              removes if from the configuration
**
**  @param      UINT16 vid
**              UINT16 sid
**
**  @return     1 - if the key is found and deleted
**              0 - if no key exists for this association
**
******************************************************************************
**/
UINT8 prDelKey(UINT16 vid, UINT16 sid)
{
    UINT32 i = 0;
    UINT32 j = 0;
    MRSETPRES_REQ* pNew = NULL;
    MRSETPRES_REQ* pOld = NULL;

    if((pOld = gRsvData[vid]) != NULL)
    {
        /*
        ** Find the PR Key corresponding to the deleted association.
        */
        for(i = 0; i < pOld->regCount; i++)
        {
            if(pOld->keyList[i].sid == sid)
            {
                /*
                ** Found! Remove the key. For this, we allocate new memory
                ** fill it up with valid keys - leaving out the removed key.
                ** All other fields are updated accordingly. If this happens to
                ** be the only key in the config, just free up the existing memory.
                */
                gRsvData[vid] = NULL;
                if(pOld->regCount > 1)
                {
                    pNew = (MRSETPRES_REQ *)s_MallocC(sizeof(MRSETPRES_REQ) +
                                             (pOld->regCount - 1) * sizeof(MRREGKEY), __FILE__, __LINE__);
                    pNew->vid      = vid;
                    pNew->sid      = (pOld->sid == sid) ? 0xffff : pOld->sid;
                    pNew->scope    = pOld->scope;
                    pNew->type     = pOld->type;
                    pNew->regCount = 0;
                    for(j = 0; j < pOld->regCount; j++)
                    {
                        if(j != i)
                        {
                            memcpy(&(pNew->keyList[pNew->regCount++]), &(pOld->keyList[j]), sizeof(MRREGKEY));
                        }
                    }
                    gRsvData[vid] = pNew;
                }
                s_Free(pOld, (sizeof(MRSETPRES_REQ) + pOld->regCount * sizeof(MRREGKEY)), __FILE__, __LINE__);
                return (1);
            }
        }
    }
    return (0);
}

/**
******************************************************************************
**
**  @brief      Sends a broadcast msg to the CCB to update the config changes
**              to the FE processors on all controllers in the DSC.
**
**  @param      UINT16 vid
**              UINT16 sid
**              UINT16 flags
**
**  @return     none
**
******************************************************************************
**/
void prUpdateFE(UINT16 sid, UINT16 vid, UINT16 flags)
{
    MRUPDPRR_REQ* pReq = NULL;
    LOG_IPC_BROADCAST_PKT *eldn = NULL;

    eldn = (LOG_IPC_BROADCAST_PKT *)s_MallocC(sizeof(LOG_IPC_BROADCAST_PKT) + sizeof(MRUPDPRR_REQ), __FILE__, __LINE__);
    pReq = (MRUPDPRR_REQ *)(eldn->data);

    eldn->header.event = LOG_IPC_BROADCAST;
    eldn->subEvent  = EB_PRR_SUB;               /* Subevent type             */
    eldn->bcastType = EB_TO_ALL;                /* Broadcast Type            */
    eldn->serialNum = 0;                        /* Serial number to send to  */
    eldn->dataSize  = sizeof(MRUPDPRR_REQ);     /* Size of data to follow    */

    pReq->sid = sid;
    pReq->vid = vid;
    pReq->dummy = 0;
    pReq->flags = flags;

    MSC_LogMessageRel(eldn, sizeof(LOG_IPC_BROADCAST_PKT) + sizeof(MRUPDPRR_REQ));
}

#endif /* BACKEND */

void prDump(UINT16 vid, const char *file, UINT32 line, const char *func)
{
    UINT32 i;

    if(vid >= MAX_VIRTUAL_DISKS)
    {
        fprintf (stderr, "[%s:%d\t%s]: prDump ERROR\n", file, line, func);
        return;
    }
#ifdef FRONTEND
    if((MAG_VDMT_dir[vid] != NULL) && (MAG_VDMT_dir[vid]->reservation != NULL))
    {
        fprintf(stderr,"[%s:%d\t%s] prDump: Reservation: %04d %02d %02d %02d\n",
                file, line, func,
                MAG_VDMT_dir[vid]->reservation->vid,
                MAG_VDMT_dir[vid]->reservation->scope,
                MAG_VDMT_dir[vid]->reservation->resvType,
                MAG_VDMT_dir[vid]->reservation->rsvdIdx);
        for (i = 0; i < MAX_KEYS; i++)
        {
            if(MAG_VDMT_dir[vid]->reservation->keyset[i] != NULL)
            {
                fprintf(stderr,"\tKey %d: %04d %04d %02d %02d 0x%016llX\n",i,
                                         MAG_VDMT_dir[vid]->reservation->keyset[i]->vid,
                                         MAG_VDMT_dir[vid]->reservation->keyset[i]->sid,
                                         MAG_VDMT_dir[vid]->reservation->keyset[i]->tid,
                                         MAG_VDMT_dir[vid]->reservation->keyset[i]->lun,
                                         *((UINT64 *)MAG_VDMT_dir[vid]->reservation->keyset[i]->key));
            }
        }
    }
#else   /* BACKEND */
    if(gRsvData[vid] != NULL)
    {
        fprintf(stderr,"[%s:%d\t%s] prDump: Reservation: %04d %02d %02d %04d\n",
                file, line, func,
                gRsvData[vid]->vid,
                gRsvData[vid]->scope,
                gRsvData[vid]->type,
                gRsvData[vid]->sid);
        for (i = 0; i < gRsvData[vid]->regCount; i++)
        {
            fprintf(stderr,"\tKey %d: %04d %02d %02d 0x%016llX\n",i,
                                         gRsvData[vid]->keyList[i].sid,
                                         gRsvData[vid]->keyList[i].tid,
                                         gRsvData[vid]->keyList[i].lun,
                                         *((UINT64 *)gRsvData[vid]->keyList[i].key));
        }
    }
#endif   /* FRONTEND/BACKEND */
    else
    {
        fprintf(stderr,"[%s:%d\t%s] prDump: vid=%d\n", file, line, func, vid);
    }
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
