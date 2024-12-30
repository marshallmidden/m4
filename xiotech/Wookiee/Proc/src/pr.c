/* $Id: pr.c 162911 2014-03-20 22:45:34Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       pr.c
**
**  @brief      Support for bringing the system online.
**
**  Copyright (c) 2003-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "LOG_Defs.h"
#include "MR_Defs.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <byteswap.h>

#include "ilmt.h"
#include "pr.h"
#include "sgl.h"
#include "pm.h"
#include "sdd.h"
#include "lvm.h"
#include "def.h"
#include "misc.h"
#include "vdmt.h"
#include "imt.h"

#include <netinet/in.h>

extern SDD*    S_sddindx[MAX_SERVERS];
extern ulong   cmdtbl1;
extern ulong   cmdtbl2;
extern ulong   cmdtbl3;
extern ulong   cmdtbl4;
extern ulong   cmdtbl5;
extern ulong   cmdtbl6;
extern VDMT* MAG_VDMT_dir[MAX_VIRTUALS];

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/

UINT32 presv_in(ILT *pILT);
UINT32 presv_out(ILT *pILT, UINT8 *data);
UINT32 prRegister(ILT *pILT, UINT8 *data);
UINT32 prRegisterAndIgnore(ILT *pILT, UINT8 *data);
UINT32 prReserve(ILT *pILT, UINT8 *data);
UINT32 prRelease(ILT *pILT, UINT8 *data);
UINT32 prClear(ILT *pILT, UINT8 *data);
UINT32 prPreempt(ILT *pILT, UINT8 *data);
UINT32 prPreemptAndAbort(ILT *pILT, UINT8 *data);
void   presv_abort(ILMT *pILMT, ILT *pILT);
UINT32 pr_cfgbkup(ILMT *pILMT);
void   pr_cfgUpdate (ILMT *pILMT);
void   pr_updCmdHandler(ILMT *pILMT);
void   pr_rmVID(VDMT *pVDMT);
void   pr_delAssociation(VDMT *pVDMT, UINT16 sid);
void   cbRegister1(ILT *pILT);
void   cbRegister2(ILT *pILT);
void   cbReserve(ILT *pILT);
void   cbRelease(ILT *pILT);
void   cbClear(ILT *pILT);
void   cbPreempt(ILT *pILT);
void   cbPreemptAndAbort(ILT *pILT);
INT32  findNexus(ILMT* pILMT, RESV* resv);
INT32  findKey(UINT8 *key, RESV *pRSV);
void   pr_cfgRetrieve(UINT16 vid);
void   cbRetrieve(UINT32 rc, ILT *pILT UNUSED, MR_PKT *pMRP, UINT32 param);

extern void MAG_prComp(ILT *pILT);
extern void MAG_prAbortTask(ILT *pILT);
extern void MAG_CheckNextTask(ILMT *pILMT);
extern void LL_SendPacket(void *pReq, UINT32 reqSize, UINT32 cmdCode,
                          void *pRsp, UINT32 rspLen, void *pCallback,
                          UINT32 param);

extern void prDump(UINT16 vid, const char *file, UINT32 line, const char *func);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/*
******************************************************************************
** PRR IN Processing Functions
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      persistent reserve in cmd processing main function
**
**  @param      pILT    - ILT pointer at 2nd lvl
**              ppSGL    - SGL pointer to return the response data
**
**  @return     return status GOOD or appropriate error.
**
******************************************************************************
**/
UINT32 presv_in(ILT *pILT)
{
    UINT16      i;
    UINT16      j;
    UINT16      allocLen;
    SGL_DESC    *desc;
    SGL         *pSGL = NULL;
    UINT32      *rsp = NULL;
    ILMT        *pILMT = (ILMT *)pILT->ilt_normal.w2;
    UINT8       *cdb = (UINT8 *)&(pILT + 1)->ilt_normal.w4;

   /*
   ** If this vdisk was registered using reserve(6)/reserve(10), return
   ** a reservation conflict.
   */
    if (pILMT->vdmt->rsvdILMT)
   {
        return(PRERR_RESV_CONF);
   }

   switch((cdb[1]) & 0x1f)
   {
      case PRESV_IN_READ_KEYS:
      {
          if((allocLen = (cdb[7] << 8) | cdb[8]) < 8)
          {
              return(PRERR_INVFL_CDB);
          }
          /*
          ** Allocate the response SGL
          */
          pSGL = m_asglbuf(allocLen);
          desc = (SGL_DESC *)(pSGL + 1);
          rsp  = (UINT32 *)desc->addr;
          rsp[0] = bswap_32(pILMT->vdmt->prGen);
          rsp[1] = 0;
          if(pILMT->vdmt->reservation != NULL)
          {
              for (i = 0, j = 0; i < MAX_KEYS; i++)
              {
                  if(pILMT->vdmt->reservation->keyset[i] != NULL)
                  {
                      if(allocLen >= (rsp[1] + 16))
                      {
                          memcpy((UINT8 *)&rsp[j+2], pILMT->vdmt->reservation->keyset[i]->key, 8);
                          j += 2;
                      }
                      rsp[1] += 8;
                  }
              }
          }
          desc->len = (allocLen < rsp[1] + 8) ? allocLen : rsp[1]+8;
          rsp[1] = bswap_32(rsp[1]);
          pILT++;
          pILT->misc = (UINT32)pSGL;
          return(PRERR_RESV_OK);
      }
      case PRESV_IN_READ_RESERVATION:
      {
          if((allocLen = (cdb[7] << 8) | cdb[8]) < 8)
          {
              return(PRERR_INVFL_CDB);
          }
          /*
          ** Allocate the response SGL
          */
          pSGL = m_asglbuf(allocLen);
          desc = (SGL_DESC *)(pSGL + 1);
          rsp  = (UINT32 *)desc->addr;
          rsp[0] = bswap_32(pILMT->vdmt->prGen);
          if((pILMT->vdmt->reservation != NULL)
                  && (pILMT->vdmt->reservation->rsvdIdx != -1))
          {
              rsp[1] = 16;
              if(allocLen > 15)
              {
                  memcpy((UINT8 *)&rsp[2], pILMT->vdmt->reservation->keyset[pILMT->vdmt->reservation->rsvdIdx]->key, 8);
              }
              if(allocLen > 21)
              {
                  ((UINT8 *)rsp)[21] = (pILMT->vdmt->reservation->scope << 4) | (pILMT->vdmt->reservation->resvType);
              }
          }
          else
          {
              rsp[1] = 0;
          }
          desc->len = (allocLen < rsp[1] + 8) ? allocLen : rsp[1]+8;
          rsp[1] = bswap_32(rsp[1]);
          pILT++;
          pILT->misc = (UINT32)pSGL;
          return(PRERR_RESV_OK);
      }
      case PRESV_IN_REPORT_CAPABILITIES:
      {
          fprintf (stderr, "[%s:%d\t%s]: PRESV_IN_REPORT_CAPABILITIES\n",__FILE__, __LINE__,__func__ );
          return PRERR_INVFL_CDB;
      }
      case PRESV_IN_READ_FULL_STATUS:
      {
          fprintf (stderr, "[%s:%d\t%s]: PRESV_IN_READ_FULL_STATUS\n",__FILE__, __LINE__,__func__ );
          return PRERR_INVFL_CDB;
      }
      default:
      {
          fprintf (stderr, "[%s:%d\t%s]: PRESV_IN_UNKNOWN\n",__FILE__, __LINE__,__func__ );
          return PRERR_INVFL_CDB;
      }
   }
}

/*
******************************************************************************
** PRR OUT Processing Functions
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      persistent reserve out cmd processing main function
**
**  @param      pILT    - ILT pointer at 2nd lvl
**              data    - Parameter list data for the PRR cmd
**
**  @return     return status GOOD or appropriate error.
**
******************************************************************************
**/
UINT32 presv_out(ILT *pILT, UINT8 *data)
{
    ILMT *pILMT = (ILMT *)pILT->ilt_normal.w2;
    UINT8 *cdb = (UINT8 *)&(pILT + 1)->ilt_normal.w4;
    VDMT *pVDMT = pILMT->vdmt;

    /*
    ** If this vdisk was reserved using reserve(6)/reserve(10), return
    ** a reservation conflict.
    */
    if(pVDMT->rsvdILMT)
    {
        return PRERR_RESV_CONF;
    }


//    fprintf (stderr, "PR_OUT %02d: %02d %02d %02d 0x%016llX 0x%016llX\n",
//                       pILMT->imt->sid,
//                       (cdb[1] & 0x1f),
//                       (cdb[2] & 0xf0) >> 4,
//                       (cdb[2] & 0x0f),
//                       *((UINT64 *)&data[0]),
//                       *((UINT64 *)&data[8]));
//    prDump(pILMT->vdmt->vid, __FILE__, __LINE__, __func__);

    switch(cdb[1] & 0x1f)
    {
        case PRESV_OUT_REGISTER:
        {
            return (prRegister(pILT, data));
        }
        case PRESV_OUT_RESERVE:
        {
            return (prReserve(pILT, data));
        }
        case PRESV_OUT_RELEASE:
        {
            return (prRelease(pILT, data));
        }
        case PRESV_OUT_CLEAR:
        {
            return (prClear(pILT, data));
        }
        case PRESV_OUT_PREEMPT:
        {
            return (prPreempt(pILT, data));
        }
        case PRESV_OUT_PREEMPT_AND_ABORT:
        {
            return (prPreemptAndAbort(pILT, data));
        }
        case PRESV_OUT_REGISTER_AND_IGNORE:
        {
            return (prRegisterAndIgnore(pILT, data));
        }
        case PRESV_OUT_REGISTER_AND_MOVE:
        {
            fprintf (stderr, "KM %s: PRESV_OUT_REGISTER_AND_MOVE command\n", __func__);
            return PRERR_INVFL_CDB;
        }
        default:
        {
            fprintf (stderr, "KM %s: PRESV_OUT_UNKNOWN command\n", __func__);
            return PRERR_INVFL_CDB;
        }
    }
}

/**
******************************************************************************
**
**  @brief      PR cmd processing for REGISTER service action
**
**  @param      pILT    - ILT pointer at 2nd lvl
**              data    - Parameter list data for the PRR cmd
**
**  @return     return status GOOD or appropriate error.
**
******************************************************************************
**/
UINT32 prRegister(ILT *pILT, UINT8 *data)
{
    UINT32 i;
    INT32 kIndx;
    UINT64 zKey = 0x0;
    REGKEYS *pKEY = NULL;
    LVM *pLVM = NULL;
    ILMT *pILMT = (ILMT *)pILT->ilt_normal.w2;
    UINT8 *cdb = (UINT8 *)&(pILT + 1)->ilt_normal.w4;
    VDMT *pVDMT = pILMT->vdmt;

    /*
    ** if flags other then APTPL is set or
    ** if additional param list is no 24, return error
    */
    if((data[20] & 0xfe) != 0)
    {
        return PRERR_INVFL_CDB;
    }
    if (bswap_32(*((UINT32 *)&cdb[5])) != 24)
    {
        return PRERR_INV_PARAM_LEN;
    }
    /*
    ** Initialize the callback & misc to NULL to start with
    */
    pILT++;
    pILT->cr = NULL;
    pILT->misc = 0;

    if((kIndx = findNexus(pILMT, pVDMT->reservation)) == -1)
    {
        /*
        ** Received on an unregistered I_T nexus. If the Reservation Key
        ** in the req is non-zero, return conflict.
        */
        if(memcmp(&data[0], (UINT8 *)&zKey, 8) != 0)
        {
            return PRERR_RESV_CONF;
        }
        /*
        ** if Service Action key is zero, then return Success
        */
        if(memcmp(&data[8], (UINT8 *)&zKey, 8) == 0)
        {
            pILMT->vdmt->prGen++;
            return PRERR_RESV_OK;
        }
        /*
        ** if pVDMT->registration is NULL, allocate it
        */
        if(pVDMT->reservation == NULL)
        {
            pVDMT->reservation = (RESV *)s_MallocC(sizeof(RESV), __FILE__, __LINE__);
            memset((void *)pVDMT->reservation, 0, sizeof(RESV));
            pVDMT->reservation->vid = pVDMT->vid;
            pVDMT->reservation->rsvdIdx = -1;
            i = 0;
        }
        else
        {
            /*
            ** Find a empty slot in the keysets and store this REGKEY
            */
            for (i = 0; i < MAX_KEYS; i++)
            {
                if(pVDMT->reservation->keyset[i] == NULL)
                {
                    break;
                }
            }
            if(i >= MAX_KEYS)
            {
                return PRERR_INSUFF_REG_RES;
            }
            /*
            ** backup the current config
            */
            pILT->misc = (UINT32)pr_cfgbkup(pILMT);
        }
        /*
        ** Register the Key specified in service action key
        */
        pKEY = s_MallocC(sizeof(REGKEYS), __FILE__, __LINE__);
        memcpy(pKEY->key, &data[8], 8);
        pKEY->sid = pILMT->imt->sid;
        pKEY->vid = pVDMT->vid;
        pKEY->tid = S_sddindx[pKEY->sid]->tid;
        for (pLVM = S_sddindx[pKEY->sid]->lvm; pLVM != NULL; pLVM = pLVM->nlvm)
        {
            if(pLVM->vid == pKEY->vid)
            {
                pKEY->lun = pLVM->lun;
                break;
            }
        }
        pVDMT->reservation->keyset[i] = pKEY;
    }
    else
    {
        /*
        ** Received req on a registered I_T nexus.
        ** If the reservasion KEY does not match, return conflict
        */
        if(memcmp(&data[0],  pVDMT->reservation->keyset[kIndx]->key, 8) != 0)
        {
            return PRERR_RESV_CONF;
        }
        /*
        ** if service action key is the registered key, there is nothing to do - return success
        */
        if(memcmp(&data[8],  pVDMT->reservation->keyset[kIndx]->key, 8) == 0)
        {
            pILMT->vdmt->prGen++;
            return PRERR_RESV_OK;
        }
        /*
        ** backup the current config
        */
        pILT->misc = (UINT32)pr_cfgbkup(pILMT);
        /*
        ** if Service action key is non-zero, change the reservation key
        */
        if(memcmp(&data[8], (UINT8 *)&zKey, 8) != 0)
        {
            memcpy(pVDMT->reservation->keyset[kIndx]->key, &data[8], 8);
        }
        else
        {
            /*
            ** The service action key is zero! We will have to unregister the reservation
            ** key and if that holds the reservation, we will have to remove the
            ** reservation
            */
            s_Free((void*)pVDMT->reservation->keyset[kIndx], sizeof(REGKEYS), __FILE__, __LINE__);
            pVDMT->reservation->keyset[kIndx] = NULL;
            if(pVDMT->reservation->rsvdIdx == kIndx)
            {
                /*
                ** The requesting host also holds thte reservation - need some special
                ** processing post-config update.
                */
                pILT->cr = cbRegister2;
                pVDMT->reservation->rsvdIdx = -1;
            }
        }
    }

    /*
    ** Setup the ILT for completion processing, call the Config update and
    ** return in progress so that the magdrvr will setup the proper task event handler.
    */
    if(pILT->cr == NULL)
    {
        pILT->cr = (void *)cbRegister1;
    }
    pILMT->prilt = pILT;
    pr_cfgUpdate(pILMT);
    return PRERR_IN_PROGRESS;
}

/**
******************************************************************************
**
**  @brief      Post config update callback routine used by REGISTER and
**              REGISTER AND IGNORE service actions when registering or
**              unregistering
**
**  @param      pILT    - ILT pointer at lvl-3
**
**  @return     return status GOOD or appropriate error.
**
******************************************************************************
**/
void cbRegister1(ILT *pILT)
{
    ILMT *pILMT = (ILMT *)(pILT - 1)->ilt_normal.w2;

    /*
    ** Incriment the PRGeneration counter
    */
    pILMT->vdmt->prGen++;
    /*
    ** Now that the config update is complete, update the ILMT
    ** cmd handlers to reflect the current reservation
    */
    if((pILMT->vdmt->reservation == NULL)
           || (pILMT->vdmt->reservation->rsvdIdx == -1))
    {
        pILMT->cmdhndl = (void *)&cmdtbl1;
    }
    else
    {
        switch(pILMT->vdmt->reservation->resvType)
        {
            case RESV_WR_EXCL:
                pILMT->cmdhndl = (void *)&cmdtbl4;
                break;
            case RESV_EXCL_ACC:
                pILMT->cmdhndl = (void *)&cmdtbl3;
                break;
            case RESV_WR_EXCL_RO:
                pILMT->cmdhndl = (findNexus(pILMT,pILMT->vdmt->reservation) != -1) ?
                                            (void *)&cmdtbl1 : (void *)&cmdtbl5;
                break;
            case RESV_EXCL_ACC_RO:
                pILMT->cmdhndl = (findNexus(pILMT,pILMT->vdmt->reservation) != -1) ?
                                            (void *)&cmdtbl1 : (void *)&cmdtbl6;
                break;
            default:
                break;
        }
    }
    /*
    ** Clean up the mess created for config update and call MAG layer to
    ** complete the PR cmd
    */
    pILMT->prilt = NULL;
    if(pILT->misc != 0)
    {
        s_Free((void *)pILT->misc, (sizeof(RESV) + (sizeof(REGKEYS) * MAX_KEYS)), __FILE__, __LINE__);
    }
    pILT--;
    MAG_prComp(pILT);
}

/**
******************************************************************************
**
**  @brief      Post config update callback routine used by REGISTER and
**              REGISTER AND IGNORE service actions when unregistering
**              involves removing an existing reservation
**
**  @param      pILT    - ILT pointer at lvl-3
**
**  @return     return status GOOD or appropriate error.
**
******************************************************************************
**/
void cbRegister2(ILT *pILT)
{
    ILMT *ilmt = NULL;
    ILMT *pILMT = (ILMT *)(pILT - 1)->ilt_normal.w2;
    VDMT *pVDMT = pILMT->vdmt;

    /*
    ** Incriment the PRGeneration counter
    */
    pILMT->vdmt->prGen++;
    /*
    ** Config update is complete for an unregister that caused
    ** releasing an existing configuration. Update the ILMTs' cmd
    ** handlers o reflect the config change. If the reservation released
    ** is of type 'registrants only', set UA of 'Reservations Released' for
    ** all the registered initiators.
    */

    for (ilmt = pVDMT->ilmtHead; ilmt != NULL; ilmt = ilmt->link)
    {
        if (ilmt != pILMT)
        {
            ilmt->cmdhndl = (void *)&cmdtbl1;
            if((ilmt != pILMT)
                    && ((((RESV *)pILT->misc)->resvType == RESV_WR_EXCL_RO)
                    || (((RESV *)pILT->misc)->resvType == RESV_EXCL_ACC_RO))
                    && (findNexus(ilmt, (RESV *)pILT->misc) != -1))
            {
                BIT_SET(ilmt->flag3, PR_UA_RESV_RELEASED);
            }
        }
    }
    /*
    ** Clean up the mess created for config update and call MAG layer to
    ** complete the PR cmd
    */
    pILMT->prilt = NULL;
    s_Free((void *)pILT->misc, (sizeof(RESV) + (sizeof(REGKEYS) * MAX_KEYS)), __FILE__, __LINE__);
    pILT--;
    MAG_prComp(pILT);
}

/**
******************************************************************************
**
**  @brief      PR cmd processing for REGISTER AND IGNORE service action
**
**  @param      pILT    - ILT pointer at 2nd lvl
**              data    - Parameter list data for the PRR cmd
**
**  @return     return status GOOD or appropriate error.
**
******************************************************************************
**/
UINT32 prRegisterAndIgnore(ILT *pILT, UINT8 *data)
{
    UINT32 i;
    INT32 kIndx;
    UINT64 zKey = 0x0;
    REGKEYS *pKEY = NULL;
    LVM *pLVM = NULL;
    ILMT *pILMT = (ILMT *)pILT->ilt_normal.w2;
    UINT8 *cdb = (UINT8 *)&((pILT + 1)->ilt_normal.w4);
    VDMT *pVDMT = pILMT->vdmt;

    /*
    ** if flags other then APTPL is set or
    ** if additional param list is no 24, return error
    */
    if((data[20] & 0xfe) != 0)
    {
        return PRERR_INVFL_CDB;
    }
    if (bswap_32(*((UINT32 *)&cdb[5])) != 24)
    {
        return PRERR_INV_PARAM_LEN;
    }
    /*
    ** Initialize the callback & misc to NULL to start with
    */
    pILT++;
    pILT->cr = NULL;
    pILT->misc = 0;

    if((kIndx = findNexus(pILMT, pVDMT->reservation)) == -1)
    {
        /*
        ** if Service Action key is zero, then return Success
        */
        if(memcmp(&data[8], (UINT8 *)&zKey, 8) == 0)
        {
            pILMT->vdmt->prGen++;
            return PRERR_RESV_OK;
        }
        /*
        ** if pVDMT->registration is NULL, allocate it
        */
        if(pVDMT->reservation == NULL)
        {
            pVDMT->reservation = (RESV *)s_MallocC(sizeof(RESV), __FILE__, __LINE__);
            memset((void *)pVDMT->reservation, 0, sizeof(RESV));
            pVDMT->reservation->vid = pVDMT->vid;
            pVDMT->reservation->rsvdIdx = -1;
            i = 0;
        }
        else
        {
            /*
            ** Find a empty slot in the keysets and store this REGKEY
            */
            for (i = 0; i < MAX_KEYS; i++)
            {
                if(pVDMT->reservation->keyset[i] == NULL)
                {
                    break;
                }
            }
            if(i >= MAX_KEYS)
            {
                return PRERR_INSUFF_REG_RES;
            }
            /*
            ** backup the current config
            */
            pILT->misc = (UINT32)pr_cfgbkup(pILMT);
        }
        /*
        ** Register the Key specified in service action key
        */
        pKEY = s_MallocC(sizeof(REGKEYS), __FILE__, __LINE__);
        memcpy(pKEY->key, &data[8], 8);
        pKEY->sid = pILMT->imt->sid;
        pKEY->vid = pVDMT->vid;
        pKEY->tid = S_sddindx[pKEY->sid]->tid;
        for (pLVM = S_sddindx[pKEY->sid]->lvm; pLVM != NULL; pLVM = pLVM->nlvm)
        {
            if(pLVM->vid == pKEY->vid)
            {
                pKEY->lun = pLVM->lun;
                break;
            }
        }
        pVDMT->reservation->keyset[i] = pKEY;
    }
    else
    {
        /*
        ** if Service action key is non-zero, change the reservation key
        */
        if(memcmp(&data[8], (UINT8 *)&zKey, 8) != 0)
        {
            /*
            ** backup the current config
            */
            pILT->misc = (UINT32)pr_cfgbkup(pILMT);
            memcpy(pVDMT->reservation->keyset[kIndx]->key, &data[8], 8);
        }
        else
        {
            /*
            ** if service action key is the registered key, there is nothing to do - return success
            */
            if(memcmp(&data[8],  pVDMT->reservation->keyset[kIndx]->key, 8) == 0)
            {
                return PRERR_RESV_OK;
            }
            /*
            ** backup the current config
            */
            pILT->misc = (UINT32)pr_cfgbkup(pILMT);
            /*
            ** The service action key is zero! We will have to unregister the reservation
            ** key and if that holds the reservation, we will have to remove the
            ** reservation
            */
            s_Free((void*)pVDMT->reservation->keyset[kIndx], sizeof(REGKEYS), __FILE__, __LINE__);
            pVDMT->reservation->keyset[kIndx] = NULL;
            if(pVDMT->reservation->rsvdIdx == kIndx)
            {
                /*
                ** The requesting host also holds thte reservation - need some special
                ** processing post-config update.
                */
                pILT->cr = cbRegister2;
                pVDMT->reservation->rsvdIdx = -1;
            }
        }
    }

    /*
    ** Setup the ILT for completion processing, call the Config update and
    ** return in progress so that the magdrvr will setup the proper task event handler.
    */
    if(pILT->cr == NULL)
    {
        pILT->cr = (void *)cbRegister1;
    }
    pILMT->prilt = pILT;
    pr_cfgUpdate(pILMT);
    return PRERR_IN_PROGRESS;
}

/**
******************************************************************************
**
**  @brief      PR cmd processing for RESERVE service action
**
**  @param      pILT    - ILT pointer at 2nd lvl
**              data    - Parameter list data for the PRR cmd
**
**  @return     return status GOOD or appropriate error.
**
******************************************************************************
**/
UINT32 prReserve(ILT *pILT, UINT8 *data)
{
    ILMT *pILMT = (ILMT *)pILT->ilt_normal.w2;
    UINT8 *cdb = (UINT8 *)&(pILT + 1)->ilt_normal.w4;
    VDMT *pVDMT = pILMT->vdmt;
    INT32 kIndx;
    /*
    ** if scope is non LU_SCOPE or if the reservation
    ** type is for all registrants or if additional
    ** param list is no 24, return error
    */
    if((cdb[2] & 0xf0)
            || ((cdb[2] & 0x0f) == RESV_WR_EXCL_AR)
            || ((cdb[2] & 0x0f) == RESV_EXCL_ACC_AR))
    {
        return PRERR_INVFL_CDB;
    }
    if (bswap_32(*((UINT32 *)&cdb[5])) != 24)
    {
        return PRERR_INV_PARAM_LEN;
    }
    /*
    **  if the host is not registered or if the host reservation key
    ** does not match with the configured key, return conflict
    */
    if(((kIndx = findNexus(pILMT,pVDMT->reservation)) == -1)
        || (memcmp(pVDMT->reservation->keyset[kIndx]->key, &data[0], 8) != 0))
    {
        return PRERR_RESV_CONF;
    }
    /*
    ** if the host is already registered with the same params, return success, else conflict
    */
    if(pVDMT->reservation->rsvdIdx != -1)
    {
        if((pVDMT->reservation->rsvdIdx == kIndx)
                && (pVDMT->reservation->resvType == (cdb[2] & 0x0f)))
        {
            return PRERR_RESV_OK;
        }
        else
        {
            return PRERR_RESV_CONF;
        }
    }

    /*
    ** Setup the ILT for completion processing, call the Config update and
    ** return in progress so that the magdrvr will setup the proper task event handler.
    */
    pILT += 1;
    pILT->cr = (void *)cbReserve;
    pILT->misc = (UINT32)pr_cfgbkup(pILMT);
    pVDMT->reservation->rsvdIdx = kIndx;
    pVDMT->reservation->resvType = (cdb[2] & 0x0f);
    pILMT->prilt = pILT;
    pr_cfgUpdate(pILMT);
    return PRERR_IN_PROGRESS;
}

/**
******************************************************************************
**
**  @brief      Post config update callback routine used by RESERVE
**              service action when unregistering
**              involves removing an existing reservation
**
**  @param      pILT    - ILT pointer at lvl-3
**
**  @return     return status GOOD or appropriate error.
**
******************************************************************************
**/
void cbReserve(ILT *pILT)
{
    ILMT *ilmt = NULL;
    ILMT *pILMT = (ILMT *)(pILT - 1)->ilt_normal.w2;
    VDMT *pVDMT = pILMT->vdmt;

    /*
    ** Now that the config update is complete, update the ILMT
    ** cmd handlers ro reflect the reservation change
    */
    for (ilmt = pVDMT->ilmtHead; ilmt != NULL; ilmt = ilmt->link)
    {
        if (ilmt != pILMT)
        {
            ilmt->origcmdhand = ilmt->cmdhndl;
            switch(pVDMT->reservation->resvType)
            {
                case RESV_WR_EXCL:
                    ilmt->cmdhndl = (void *)&cmdtbl4;
                    break;
                case RESV_EXCL_ACC:
                    ilmt->cmdhndl = (void *)&cmdtbl3;
                    break;
                case RESV_WR_EXCL_RO:
                    ilmt->cmdhndl = (findNexus(ilmt,pVDMT->reservation) != -1) ?
                                                (void *)&cmdtbl1 : (void *)&cmdtbl5;
                    break;
                case RESV_EXCL_ACC_RO:
                    ilmt->cmdhndl = (findNexus(ilmt,pVDMT->reservation) != -1) ?
                                                (void *)&cmdtbl1 : (void *)&cmdtbl6;
                    break;
                default:
                    break;
            }
        }
    }
    pILMT->cmdhndl = (void *)&cmdtbl1;
    /*
    ** Clean up the mess created for config update and call MAG layer to
    ** complete the PR cmd
    */
    pILMT->prilt = NULL;
    s_Free((void *)pILT->misc, (sizeof(RESV) + (sizeof(REGKEYS) * MAX_KEYS)), __FILE__, __LINE__);
    pILT--;
    MAG_prComp(pILT);
}

/**
******************************************************************************
**
**  @brief      PR cmd processing for RELEASE service action
**
**  @param      pILT    - ILT pointer at 2nd lvl
**              data    - Parameter list data for the PRR cmd
**
**  @return     return status GOOD or appropriate error.
**
******************************************************************************
**/
UINT32 prRelease(ILT *pILT, UINT8 *data)
{
    ILMT *pILMT = (ILMT *)pILT->ilt_normal.w2;
    UINT8 *cdb = (UINT8 *)&(pILT + 1)->ilt_normal.w4;
    VDMT *pVDMT = pILMT->vdmt;
    INT32 kIndx;
    /*
    ** if scope is non LU_SCOPE or if the reservation
    ** type is for all registrants or if additional
    ** param list is no 24, return error
    */
    if((cdb[2] & 0xf0)
            || ((cdb[2] & 0x0f) == RESV_WR_EXCL_AR)
            || ((cdb[2] & 0x0f) == RESV_EXCL_ACC_AR))
    {
        return PRERR_INVFL_CDB;
    }
    if (bswap_32(*((UINT32 *)&cdb[5])) != 24)
    {
        return PRERR_INV_PARAM_LEN;
    }
    /*
    **  if the host is not registered or if the host reservation key
    ** does not match with the configured key, return success
    */
    if(((kIndx = findNexus(pILMT, pVDMT->reservation)) == -1)
        || (memcmp(pVDMT->reservation->keyset[kIndx]->key, &data[0], 8) != 0))
    {
        return PRERR_RESV_OK;
    }
    /*
    ** if the reservation type do not match, return error
    */
    if((cdb[2] & 0x0f) != pVDMT->reservation->resvType)
    {
        return PRERR_INV_RELEASE;
    }
    /*
    ** Setup the ILT for completion processing, call the Config update and
    ** return in progress so that the magdrvr will setup the proper task event handler.
    */
    pILT += 1;
    pILT->cr = (void *)cbRelease;
    pILT->misc = (UINT32)pr_cfgbkup(pILMT);
    pVDMT->reservation->rsvdIdx = -1;
    pILMT->prilt = pILT;
    pr_cfgUpdate(pILMT);
    return PRERR_IN_PROGRESS;
}

/**
******************************************************************************
**
**  @brief      Post config update callback routine used by RELEASE
**              service action
**
**  @param      pILT    - ILT pointer at lvl-3
**
**  @return     return status GOOD or appropriate error.
**
******************************************************************************
**/
void cbRelease(ILT *pILT)
{
    ILMT *ilmt = NULL;
    ILMT *pILMT = (ILMT *)(pILT - 1)->ilt_normal.w2;
    VDMT *pVDMT = pILMT->vdmt;

    /*
    ** Update the ILMT cmd handlers to reflect the config change.
    ** If the reservation released is of type 'registrants only',
    ** set UA of 'Reservations Released' for all the registered initiators.
    */
    for (ilmt = pVDMT->ilmtHead; ilmt != NULL; ilmt = ilmt->link)
    {
        if (ilmt != pILMT)
        {
            ilmt->cmdhndl = (void *)&cmdtbl1;
            if((ilmt != pILMT)
                    && ((((RESV *)pILT->misc)->resvType == RESV_WR_EXCL_RO)
                    || (((RESV *)pILT->misc)->resvType == RESV_EXCL_ACC_RO))
                    && (findNexus(ilmt, (RESV *)pILT->misc) != -1))
            {
                BIT_SET(ilmt->flag3, PR_UA_RESV_RELEASED);
            }
        }
    }
    /*
    ** Clean up the mess created for config update and call MAG layer to
    ** complete the PR cmd
    */
    pILMT->prilt = NULL;
    s_Free((void *)pILT->misc, (sizeof(RESV) + (sizeof(REGKEYS) * MAX_KEYS)), __FILE__, __LINE__);
    pILT--;
    MAG_prComp(pILT);
}

/**
******************************************************************************
**
**  @brief      PR cmd processing for CLEAR service action
**
**  @param      pILT    - ILT pointer at 2nd lvl
**              data    - Parameter list data for the PRR cmd
**
**  @return     return status GOOD or appropriate error.
**
******************************************************************************
**/
UINT32 prClear(ILT *pILT, UINT8 *data)
{
    ILMT *pILMT = (ILMT *)pILT->ilt_normal.w2;
    UINT8 *cdb = (UINT8 *)&(pILT + 1)->ilt_normal.w4;
    VDMT *pVDMT = pILMT->vdmt;
    INT32 kIndx;
    UINT32 i = 0;
    /*
    ** if additional param list is no 24, return error
    */
    if (bswap_32(*((UINT32 *)&cdb[5])) != 24)
    {
        return PRERR_INV_PARAM_LEN;
    }
    /*
    **  if the host is not registered or if the host reservation key
    ** does not match with the configured key, return conflict
    */
    if(((kIndx = findNexus(pILMT, pVDMT->reservation)) == -1)
        || (memcmp(pVDMT->reservation->keyset[kIndx]->key, &data[0], 8) != 0))
    {
        return PRERR_RESV_CONF;
    }
    pILT += 1;
    /*
    ** Free all the memory allcoated for the keysets and RESV structs
    */
    if(pVDMT->reservation != NULL)
    {
        pILT->misc = (UINT32)pr_cfgbkup(pILMT);
        for (i = 0; i < MAX_KEYS; i++)
        {
            if(pVDMT->reservation->keyset[i] != NULL)
            {
                s_Free((void *)pVDMT->reservation->keyset[i], sizeof(REGKEYS), __FILE__, __LINE__);
                pVDMT->reservation->keyset[i] = NULL;
            }
        }
        s_Free((void *)pVDMT->reservation, sizeof(RESV), __FILE__, __LINE__);
        pVDMT->reservation = NULL;
    }
    /*
    ** Setup the ILT for completion processing, call the Config update and
    ** return in progress so that the magdrvr will setup the proper task event handler.
    */
    pILT->cr = (void *)cbClear;
    pILMT->prilt = pILT;
    pr_cfgUpdate(pILMT);
    return PRERR_IN_PROGRESS;
}

/**
******************************************************************************
**
**  @brief      Post config update callback routine used by CLEAR
**              service action
**
**  @param      pILT    - ILT pointer at lvl-3
**
**  @return     return status GOOD or appropriate error.
**
******************************************************************************
**/
void cbClear(ILT *pILT)
{
    ILMT *ilmt = NULL;
    ILMT *pILMT = (ILMT *)(pILT - 1)->ilt_normal.w2;
    VDMT *pVDMT = pILMT->vdmt;

    /*
    ** Incriment the PRGeneration counter
    */
    pILMT->vdmt->prGen++;
    /*
    ** Update the ILMT cmd handlers to reflect the config change.
    ** Set the unit attention of "Reservation Preempted", if this
    ** initiator has registered a key and is not the one which
    ** sent down this command.
    */
    for (ilmt = pVDMT->ilmtHead; ilmt != NULL; ilmt = ilmt->link)
    {
        ilmt->cmdhndl = (void *)&cmdtbl1;
        if((ilmt != pILMT) && (findNexus(ilmt, (RESV *)pILT->misc) != -1))
        {
            BIT_SET(ilmt->flag3, PR_UA_RESV_PREEMPTED);
        }
    }
    /*
    ** Clean up the mess created for config update and call MAG layer to
    ** complete the PR cmd
    */
    pILMT->prilt = NULL;
    if(pILT->misc != 0)
    {
        s_Free((void *)pILT->misc, (sizeof(RESV) + (sizeof(REGKEYS) * MAX_KEYS)), __FILE__, __LINE__);
    }
    pILT--;
    MAG_prComp(pILT);
}

/**
******************************************************************************
**
**  @brief      PR cmd processing for PREEMPT service action
**
**  @param      pILT    - ILT pointer at 2nd lvl
**              data    - Parameter list data for the PRR cmd
**
**  @return     return status GOOD or appropriate error.
**
******************************************************************************
**/
UINT32 prPreempt(ILT *pILT, UINT8 *data)
{
    ILMT *pILMT = (ILMT *)pILT->ilt_normal.w2;
    UINT8 *cdb = (UINT8 *)&(pILT + 1)->ilt_normal.w4;
    VDMT *pVDMT = pILMT->vdmt;
    UINT64 zKey = 0x0;
    INT32 rIndx;
    INT32 sIndx;
    ILMT *ilmt;
    /*
    ** if scope is non LU_SCOPE or if the reservation
    ** type is for all registrants or if additional
    ** param list is no 24, return error
    */
    if((cdb[2] & 0xf0)
            || ((cdb[2] & 0x0f) == RESV_WR_EXCL_AR)
            || ((cdb[2] & 0x0f) == RESV_EXCL_ACC_AR))
    {
        return PRERR_INVFL_CDB;
    }
    if (bswap_32(*((UINT32 *)&cdb[5])) != 24)
    {
        return PRERR_INV_PARAM_LEN;
    }
    /*
    ** if the host is not registered or if the host reservation key
    ** does not match with the configured key, return conflict
    */
    if(((rIndx = findNexus(pILMT,pVDMT->reservation)) == -1)
        || (memcmp(pVDMT->reservation->keyset[rIndx]->key, &data[0], 8) != 0))
    {
        return PRERR_RESV_CONF;
    }
    /*
    ** if Service Action key is zero,  or if the service action
    ** key is not registered, return CHECK CONDITION
    */
    if((memcmp(&data[8], (UINT8 *)&zKey, 8) == 0)
        || ((sIndx = findKey(&data[8],pVDMT->reservation)) == -1))
    {
        return PRERR_INVFL_PARAM_LIST;
    }

    /*
    ** backup the current config
    */
    pILT++;
    pILT->ilt_normal.w0 = 0x0;
    pILT->misc = (UINT32)pr_cfgbkup(pILMT);

    /*
    ** Check if this is an attempt to self-preempt.
    */
    if((memcmp(&data[8], &data[0], 8) == 0)
            && (rIndx == pVDMT->reservation->rsvdIdx))
    {
        if((cdb[2] & 0x0f) == pVDMT->reservation->resvType)
        {
            /*
            ** Incriment the PRGeneration counter and return success
            */
            pVDMT->prGen++;
            s_Free((void *)pILT->misc, (sizeof(RESV) + (sizeof(REGKEYS) * MAX_KEYS)), __FILE__, __LINE__);
            return PRERR_RESV_OK;
        }
        pVDMT->reservation->resvType = (cdb[2] & 0x0f);
    }
    else
    {
        /*
        ** Ok, we are done with all our checks. Unregister the
        ** key specified in service action key and save the
        ** corresponding ILMT in the ILT for callback processing
        */
        for (ilmt = pVDMT->ilmtHead; ilmt != NULL; ilmt = ilmt->link)
        {
            if(ilmt->imt->sid == pVDMT->reservation->keyset[sIndx]->sid)
            {
                pILT->ilt_normal.w0 = (UINT32)ilmt;
                break;
            }
        }
        s_Free((void*)pVDMT->reservation->keyset[sIndx], sizeof(REGKEYS), __FILE__, __LINE__);
        pVDMT->reservation->keyset[sIndx] = NULL;

        /*
        ** If the unregistered key holds the reservation, move the
        ** reservation to the I_T nexus refered to the key in
        ** reservation key
        */
        if(pVDMT->reservation->rsvdIdx == sIndx)
        {
            pVDMT->reservation->rsvdIdx = rIndx;
            pVDMT->reservation->resvType = (cdb[2] & 0x0f);
        }

    }
    /*
    ** Setup the ILT for completion processing, call the Config update and
    ** return in progress so that the magdrvr will setup the proper task event handler.
    */
    pILT->cr = (void *)cbPreempt;
    pILMT->prilt = pILT;
    pr_cfgUpdate(pILMT);
    return PRERR_IN_PROGRESS;
}

/**
******************************************************************************
**
**  @brief      Post config update callback routine used by PREEMPT
**              service action
**
**  @param      pILT    - ILT pointer at lvl-3
**
**  @return     return status GOOD or appropriate error.
**
******************************************************************************
**/
void cbPreempt(ILT *pILT)
{
    ILMT *pILMT = (ILMT *)(pILT - 1)->ilt_normal.w2;
    ILMT *sILMT = (ILMT *)pILT->ilt_normal.w0;
    RESV *pRSV = (RESV *)pILT->misc;
    VDMT *pVDMT = pILMT->vdmt;

    /*
    ** Incriment the PRGeneration counter
    */
    pVDMT->prGen++;

    /*
    ** If sILMT was preempted, change the cmd handlers and
    ** set UA to 'Reservations Preempted'. Also change the
    ** cmd handlers for pILMT - the new reservation holder.
    */
    if(sILMT != NULL)
    {
        if(pRSV->rsvdIdx == findNexus(sILMT, pRSV))
        {
             switch(pVDMT->reservation->resvType)
             {
                 case RESV_WR_EXCL:
                     sILMT->cmdhndl = (void *)&cmdtbl4;
                     break;
                 case RESV_EXCL_ACC:
                     sILMT->cmdhndl = (void *)&cmdtbl3;
                     break;
                 case RESV_WR_EXCL_RO:
                     sILMT->cmdhndl = (void *)&cmdtbl5;
                     break;
                 case RESV_EXCL_ACC_RO:
                     sILMT->cmdhndl = (void *)&cmdtbl6;
                     break;
                 default:
                     break;
             }
             BIT_SET(sILMT->flag3, PR_UA_RESV_PREEMPTED);
             pILMT->cmdhndl = (void *)&cmdtbl1;
        }
        else
        {
             /*
             ** set UA of 'Reservations Released' for sILMT
             */
              BIT_SET(sILMT->flag3, PR_UA_RESV_RELEASED);
        }
    }

    /*
    ** Clean up the mess created for config update and call MAG layer to
    ** complete the PR cmd
    */
    pILMT->prilt = NULL;
    s_Free((void *)pILT->misc, (sizeof(RESV) + (sizeof(REGKEYS) * MAX_KEYS)), __FILE__, __LINE__);
    pILT--;
    MAG_prComp(pILT);
}

/**
******************************************************************************
**
**  @brief      PR cmd processing for PREEMPT AND ABORT service action
**
**  @param      pILT    - ILT pointer at 2nd lvl
**              data    - Parameter list data for the PRR cmd
**
**  @return     return status GOOD or appropriate error.
**
******************************************************************************
**/
UINT32 prPreemptAndAbort(ILT *pILT, UINT8 *data)
{
    ILMT *pILMT = (ILMT *)pILT->ilt_normal.w2;
    UINT8 *cdb = (UINT8 *)&(pILT + 1)->ilt_normal.w4;
    VDMT *pVDMT = pILMT->vdmt;
    UINT64 zKey = 0x0;
    INT32 rIndx;
    INT32 sIndx;
    ILMT *ilmt;
    /*
    ** if scope is non LU_SCOPE or if the reservation
    ** type is for all registrants or if additional
    ** param list is no 24, return error
    */
    if((cdb[2] & 0xf0)
            || ((cdb[2] & 0x0f) == RESV_WR_EXCL_AR)
            || ((cdb[2] & 0x0f) == RESV_EXCL_ACC_AR))
    {
        return PRERR_INVFL_CDB;
    }
    if (bswap_32(*((UINT32 *)&cdb[5])) != 24)
    {
        return PRERR_INV_PARAM_LEN;
    }
    /*
    ** if the host is not registered or if the host reservation key
    ** does not match with the configured key, return conflict
    */
    if(((rIndx = findNexus(pILMT,pVDMT->reservation)) == -1)
        || (memcmp(pVDMT->reservation->keyset[rIndx]->key, &data[0], 8) != 0))
    {
        return PRERR_RESV_CONF;
    }
    /*
    ** if Service Action key is zero,  or if the service action
    ** key is not registered, return CHECK CONDITION
    */
    if((memcmp(&data[8], (UINT8 *)&zKey, 8) == 0)
        || ((sIndx = findKey(&data[8],pVDMT->reservation)) == -1))
    {
        return PRERR_INVFL_PARAM_LIST;
    }

    /*
    ** backup the current config
    */
    pILT++;
    pILT->ilt_normal.w0 = 0x0;
    pILT->misc = (UINT32)pr_cfgbkup(pILMT);

    /*
    ** Check if this is an attempt to self-preempt.
    */
    if((memcmp(&data[8], &data[0], 8) == 0)
            && (rIndx == pVDMT->reservation->rsvdIdx))
    {
        if((cdb[2] & 0x0f) == pVDMT->reservation->resvType)
        {
            /*
            ** Incriment the PRGeneration counter and return success
            */
            pVDMT->prGen++;
            s_Free((void *)pILT->misc, (sizeof(RESV) + (sizeof(REGKEYS) * MAX_KEYS)), __FILE__, __LINE__);
            return PRERR_RESV_OK;
        }
        pVDMT->reservation->resvType = (cdb[2] & 0x0f);
    }
    else
    {
        /*
        ** Ok, we are done with all our checks. Unregister the
        ** key specified in service action key and save the
        ** corresponding ILMT in the ILT for callback processing
        */
        for (ilmt = pVDMT->ilmtHead; ilmt != NULL; ilmt = ilmt->link)
        {
            if(ilmt->imt->sid == pVDMT->reservation->keyset[sIndx]->sid)
            {
                pILT->ilt_normal.w0 = (UINT32)ilmt;
                break;
            }
        }
        s_Free((void*)pVDMT->reservation->keyset[sIndx], sizeof(REGKEYS), __FILE__, __LINE__);
        pVDMT->reservation->keyset[sIndx] = NULL;

        /*
        ** If the unregistered key holds the reservation, move the
        ** reservation to the I_T nexus refered to the key in
        ** reservation key
        */
        if(pVDMT->reservation->rsvdIdx == sIndx)
        {
            pVDMT->reservation->rsvdIdx = rIndx;
            pVDMT->reservation->resvType = (cdb[2] & 0x0f);
        }

    }
    /*
    ** Setup the ILT for completion processing, call the Config update and
    ** return in progress so that the magdrvr will setup the proper task event handler.
    */
    pILT->cr = (void *)cbPreemptAndAbort;
    pILMT->prilt = pILT;
    pr_cfgUpdate(pILMT);
    return PRERR_IN_PROGRESS;
}

/**
******************************************************************************
**
**  @brief      Post config update callback routine used by PREEMPT
**              service action
**
**  @param      pILT    - ILT pointer at lvl-3
**
**  @return     return status GOOD or appropriate error.
**
******************************************************************************
**/
void cbPreemptAndAbort(ILT *pILT)
{
    ILMT *pILMT = (ILMT *)(pILT - 1)->ilt_normal.w2;
    ILMT *sILMT = (ILMT *)pILT->ilt_normal.w0;
    RESV *pRSV = (RESV *)pILT->misc;
    VDMT *pVDMT = pILMT->vdmt;
    ILT *ilt;

    /*
    ** Incriment the PRGeneration counter
    */
    pVDMT->prGen++;

    /*
    ** If sILMT was preempted, change the cmd handlers and
    ** set UA to 'Reservations Preempted'. Also change the
    ** cmd handlers for pILMT - the new reservation holder.
    */
    if(sILMT != NULL)
    {
        if(pRSV->rsvdIdx == findNexus(sILMT, pRSV))
        {
             switch(pVDMT->reservation->resvType)
             {
                 case RESV_WR_EXCL:
                     sILMT->cmdhndl = (void *)&cmdtbl4;
                     break;
                 case RESV_EXCL_ACC:
                     sILMT->cmdhndl = (void *)&cmdtbl3;
                     break;
                 case RESV_WR_EXCL_RO:
                     sILMT->cmdhndl = (void *)&cmdtbl5;
                     break;
                 case RESV_EXCL_ACC_RO:
                     sILMT->cmdhndl = (void *)&cmdtbl6;
                     break;
                 default:
                     break;
             }
             BIT_SET(sILMT->flag3, PR_UA_RESV_PREEMPTED);
             pILMT->cmdhndl = (void *)&cmdtbl1;
        }
        else
        {
             /*
             ** set UA of 'Reservations Released' for sILMT
             */
              BIT_SET(sILMT->flag3, PR_UA_RESV_RELEASED);
        }
        /*
        ** Abort all the tasks outstanding on the preempted I_T nexus
        */
        while((ilt = sILMT->whead) != NULL)
        {
            MAG_prAbortTask(ilt);
        }
    }

    /*
    ** Clean up the mess created for config update and call MAG layer to
    ** complete the PR cmd
    */
    pILMT->prilt = NULL;
    s_Free((void *)pILT->misc, (sizeof(RESV) + (sizeof(REGKEYS) * MAX_KEYS)), __FILE__, __LINE__);
    pILT--;
    /*
    ** Abort all the tasks outstanding on the preempting I_T nexus
    ** except for the PR task being processed
    */
    while((ilt = pILMT->whead) != NULL)
    {
        if(ilt == pILT)
        {
            if((ilt = pILT->fthd) == NULL)
            {
                break;
            }
        }
        MAG_prAbortTask(ilt);
    }
    MAG_prComp(pILT);
}

/*
******************************************************************************
** PRR util and config Functions
******************************************************************************
*/
/**
******************************************************************************
**
**  @brief      This is a stub function called as part of abort/reset/offline
**              processing of a task that is pending on config update complete.
**
**
**  @param      pILMT       - ILMT ptr <=> I_T nexus
**              pILT        - pointer to the task
**
**  @return     index or -1
**
******************************************************************************
**/
void presv_abort(ILMT *pILMT, ILT *pILT)
{
    if((pILMT != NULL) && (pILT == pILMT->prilt))
    {
        if(pILMT->prilt->misc != 0)
        {
            s_Free((void *)pILMT->prilt->misc, (sizeof(RESV) + (sizeof(REGKEYS) * MAX_KEYS)), __FILE__, __LINE__);
            pILMT->prilt->misc = 0x0;
        }
        pILMT->prilt = NULL;
    }
}

/**
******************************************************************************
**
**  @brief      This is a util function that  finds the I_T nexus in the given
**              RESV struct and returns the index to the keyset, or -1 if
**              not found
**
**  @param      pILMT       - ILMT ptr <=> I_T nexus
**              pRSV        - pointer to the resigtered keys
**
**  @return     index or -1
**
******************************************************************************
**/
INT32 findNexus(ILMT *pILMT, RESV *pRSV)
{
    INT32 indx;

    if(pRSV == NULL)
    {
        return (-1);
    }

    for (indx = 0; indx < MAX_KEYS; indx++)
    {
        if((pRSV->keyset[indx] != NULL)
                && (pRSV->keyset[indx]->sid == pILMT->imt->sid))
        {
            return (indx);
        }
    }
    return (-1);
}

/**
******************************************************************************
**
**  @brief      This is a util function that finds the KEY in the given
**              RESV struct and returns the index to the keyset, or -1 if
**              not found
**
**  @param      key         - key to find
**              pRSV        - pointer to the resigtered keys
**
**  @return     index or -1
**
******************************************************************************
**/
INT32 findKey(UINT8 *key, RESV *pRSV)
{
    INT32 indx;

    if(pRSV == NULL)
    {
        return (-1);
    }

    for (indx = 0; indx < MAX_KEYS; indx++)
    {
        if((pRSV->keyset[indx] != NULL)
                && (memcmp(pRSV->keyset[indx]->key, key, 8) == 0))
        {
            return (indx);
        }
    }
    return (-1);
}

/**
******************************************************************************
**
**  @brief      persistent reserve config update completion handler
**
**  @param      UINT16 vid
**              UINT16 sid
**              INT32  rc    - status
**
**  @return     return status GOOD.
**
******************************************************************************
**/
UINT8 pr_cfgcomp(UINT16 vid, UINT16 sid, INT32 rc)
{
    ILMT *pILMT = NULL;
    ILMT *prILMT = NULL;                        /* ILMT on which the pr request is being processed */
    VDMT *pVDMT = NULL;
    void (*cb)(ILT *);

    /*
    ** Find the vdmt from the vid.  Bail if anything goes a-wry.
    */
    if(vid >= MAX_VIRTUALS)
    {
        fprintf (stderr, "[%s:%d\t%s]: pr_cfgcomp ERROR\n",__FILE__, __LINE__,__func__ );
        return(DEINVVID);
    }
    if(!(pVDMT = MAG_VDMT_dir[vid]))
    {
        fprintf (stderr, "[%s:%d\t%s]: pr_cfgcomp ERROR\n",__FILE__, __LINE__,__func__ );
        return(DEINVVID);
    }
    if(rc != DEOK)
    {
        fprintf (stderr, "[%s:%d\t%s]: pr_cfgcomp ERROR rc = %d\n",__FILE__, __LINE__,__func__, rc );
    }

    /*
    ** It is being assumed that there will be only one outstanding PR request for a
    ** given vid at any time. Based on this, we will walk thru all the ILMTs in the VDMT
    ** to find the ILMT which has a valid (non NULL) prILT. That will be our prILT. This
    ** will be the original request ILT at the next level with all the information required
    ** to continue the processing from where it left.
    **
    ** If this assumption proves to be wrong, we will have to implement some means of blocking
    ** addition PR requests until the outstanding request is completed (requirement as per the
    ** PR standards spec)
    */
    for (pILMT = pVDMT->ilmtHead; pILMT != NULL; pILMT = pILMT->link)
    {
        if((pILMT->prilt != NULL) && (pILMT->imt->sid == sid))
        {
            prILMT = pILMT;
            break;
        }
    }
//    fprintf (stderr, "[%s:%d\t%s]: vid=%d sid=%d\n",__FILE__, __LINE__,__func__, vid, sid);
//    prDump(vid, __FILE__, __LINE__, __func__);

    /*
    ** The ILMT could have been knocked out from beneath us due to port reset
    ** or other so many reasons. In this case, there is nothing to complete. Just
    ** return success back the the CCB!
    */
    if((prILMT != NULL) && ((cb = prILMT->prilt->cr) != NULL))
    {
        (*cb)(prILMT->prilt);
    }
    return (DEOK);

}

/**
******************************************************************************
**
**  @brief      Create a backup copy of persistent reserve configuration
**
**  @param      ILMT *pILMT
**
**  @return     RESV * - newly created copy of the exsiting configuration
**
******************************************************************************
**/
UINT32 pr_cfgbkup(ILMT *pILMT)
{
    UINT32 i = 0;
    RESV *pRSV = NULL;
    REGKEYS *pKEY = NULL;
    VDMT *pVDMT = pILMT->vdmt;

    pRSV = (RESV *)s_MallocC(sizeof(RESV) + (sizeof(REGKEYS) * MAX_KEYS), __FILE__, __LINE__);

    memcpy(pRSV, pVDMT->reservation, sizeof(RESV));

    pKEY = (REGKEYS  *)(pRSV + 1);
    for (i = 0; i < MAX_KEYS; i++, pKEY++)
    {
        if(pVDMT->reservation->keyset[i] != NULL)
        {
            memcpy(pKEY, pVDMT->reservation->keyset[i], sizeof(REGKEYS));
            pRSV->keyset[i] = pKEY;
        }
        else
        {
            pRSV->keyset[i] = 0;
        }
    }
    return ((UINT32)pRSV);
}

/**
******************************************************************************
**
**  @brief      Sets the appropriate command handler in the ILMT passed in,
**              depending on the type of reservation present and whether this
**              initiator had registered previously.
**
**  @param      ILMT *pILMT
**
**  @return     none
**
******************************************************************************
**/
void pr_updCmdHandler(ILMT *pILMT)
{
    INT32 rIndx;
    ILMT *ilmt = NULL;
    VDMT *pVDMT = pILMT->vdmt;

    /*
    ** if legacy RESERVE/RELEASE is in place, jusr update the cmd handler accordingly
    */
    if(pVDMT->rsvdILMT != 0)
    {
        pILMT->cmdhndl = (void *)&cmdtbl2;
        return;
    }
    /*
    ** Check if the config retrieval is in progress. If so, set the cmd handler to
    ** block any incoming reqs. The config retrieval processing will take care of
    ** unblocking the ILMTs.
    */
    for (ilmt = pVDMT->ilmtHead; ilmt != NULL; ilmt = ilmt->link)
    {
        if(BIT_TEST(ilmt->flag3, PR_CFGRETRIEVE))
        {
            pILMT->cmdhndl = (void *)&cmdtbl1;
            BIT_SET(pILMT->flag3, PR_CFGRETRIEVE);
            fprintf (stderr, "[%s:%d\t%s]:\n",__FILE__, __LINE__,__func__ );
            return;
        }
    }
    BIT_CLEAR(pILMT->flag3, PR_CFGRETRIEVE);
    /*
    ** Check if any PR reservation exists.
    */
    if((pVDMT->reservation == NULL) || (pVDMT->reservation->rsvdIdx == -1))
    {
        /*
        ** There's either no registration or no reservation.
        */
        pILMT->cmdhndl = (void *)&cmdtbl1;
        return;
    }
    /*
    ** We have PR reservation. Set the cmd handler accordingly
    */
    if((rIndx = findNexus(pILMT,pVDMT->reservation)) == pVDMT->reservation->rsvdIdx)
    {
        /*
        ** pILMT owns the RESERVATION!!!
        */
        pILMT->cmdhndl = (void *)&cmdtbl1;
    }
    else
    {
        pILMT->origcmdhand = pILMT->cmdhndl;
        switch(pVDMT->reservation->resvType)
        {
            case RESV_WR_EXCL:
                pILMT->cmdhndl = (void *)&cmdtbl4;
                break;
            case RESV_EXCL_ACC:
                pILMT->cmdhndl = (void *)&cmdtbl3;
                break;
            case RESV_WR_EXCL_RO:
                pILMT->cmdhndl = (rIndx != -1) ? (void *)&cmdtbl1 : (void *)&cmdtbl5;
                break;
            case RESV_EXCL_ACC_RO:
                pILMT->cmdhndl = (rIndx != -1) ? (void *)&cmdtbl1 : (void *)&cmdtbl6;
                break;
            default:
                break;
        }
    }
}

/**
******************************************************************************
**
**  @brief      Clears all the reservations and registrations for a given VID
**
**  @param      UINT16 VID
**
**  @return     DEOK or appropriate status
**
******************************************************************************
**/
UINT8 pr_cfgClear(UINT16 vid)
{
    UINT32 i;
    ILMT *pILMT = NULL;
    VDMT *pVDMT = NULL;

   /*
   ** Find the vdmt from the vid.  Bail if anything goes a-wry.
   */
    if((vid >= MAX_VIRTUALS) || ((pVDMT = MAG_VDMT_dir[vid]) == NULL))
    {
        return(DEINVVID);
    }
    if(pVDMT->reservation == NULL)
    {
        /*
        ** Nothing to clear - return success
        */
        return(DEOK);
    }
    /*
    ** Update all the ILMTs with proper cmdhandlers
    */
    for(pILMT = pVDMT->ilmtHead; pILMT != NULL; pILMT = pILMT->link)
    {
        pILMT->cmdhndl = (void *)&cmdtbl1;
        if (findNexus(pILMT, pVDMT->reservation) != -1)
        {
            /*
            ** Set the unit attention of "Reservation Preempted". This will
            ** give the initiators a clue that they need to re-register.
            */
            BIT_SET(pILMT->flag3, PR_UA_RESV_PREEMPTED);
        }
        /*
        ** If there is an outstanding config update, clean up the memory
        ** allocated (?)and complete the pending req back to the mag layer
        */
        if(pILMT->prilt != NULL)
        {
            if(pILMT->prilt->misc != 0)
            {
                s_Free((void *)pILMT->prilt->misc, (sizeof(RESV) + (sizeof(REGKEYS) * MAX_KEYS)), __FILE__, __LINE__);
                pILMT->prilt->misc = 0x0;
            }
            MAG_prComp(pILMT->prilt - 1);
            pILMT->prilt = NULL;
        }
    }
    /*
    ** Free all the memory used for Keys and RESV struct
    */
    for (i = 0; i < MAX_KEYS; i++)
    {
        if(pVDMT->reservation->keyset[i] != NULL)
        {
            s_Free(pVDMT->reservation->keyset[i], sizeof(REGKEYS), __FILE__, __LINE__);
        }
    }
    s_Free(pVDMT->reservation, sizeof(RESV), __FILE__, __LINE__);
    pVDMT->reservation = NULL;
    pVDMT->prGen++;

    /*
    ** Need to communicate this info to the other controller and also store
    ** it in NVRAM.
    */
    if(pVDMT->ilmtHead)
    {
        pr_cfgUpdate(pVDMT->ilmtHead);
    }
    return(DEOK);
}

/**
******************************************************************************
**
**  @brief      Cleans up the local PRR config structs for VDisk removal
**
**  @param      VDMT *pVDMT
**
**  @return     none
**
******************************************************************************
**/
void pr_rmVID(VDMT *pVDMT)
{
    UINT32 i;

    if (!pVDMT || !pVDMT->reservation)
    {
        return;
    }
    /*
    ** Free all the memory used for Keys and RESV struct
    */
    for (i = 0; i < MAX_KEYS; i++)
    {
        if(pVDMT->reservation->keyset[i] != NULL)
        {
            s_Free(pVDMT->reservation->keyset[i], sizeof(REGKEYS), __FILE__, __LINE__);
        }
    }
    s_Free(pVDMT->reservation, sizeof(RESV), __FILE__, __LINE__);
    pVDMT->reservation = NULL;
}

/**
******************************************************************************
**
**  @brief      Updates up the local PRR config structs for Server-VDisk
**              association removal and Server removal cases. This function
**              can be extended for any future PRR specific config updates
**              using the flags field.
**
**  @param      UINT16 sid
**              UINT16 vid
**              UINT16 flags
**
**  @return     DEOK or appropriate status
**
******************************************************************************
**/
UINT8 pr_cfgChange(UINT16 vid, UINT16 sid, UINT16 flags)
{
    UINT16 i;
    UINT8  rVal = DEOK;
    VDMT   *pVDMT = NULL;

    if(BIT_TEST(flags, MRPRR_SERVERDEL))
    {
        /*
        ** This is the server delete case. Find all the VDisk assiciations
        ** for this server and delete the corresponding PRR Key(s) if exists
        */
        for(i = 0; i < MAX_VIRTUALS; i++)
        {
            if(((pVDMT = MAG_VDMT_dir[i]) != NULL)
                     && (pVDMT->reservation != NULL))
            {
                pr_delAssociation(pVDMT, sid);
            }
        }
    }
    else if(BIT_TEST(flags, MRPRR_ASSOCDEL))
    {
        /*
        ** This is the server-vdisk assiciation removal case. Find the
        ** corresponding Key and remove it.
        */
        if((vid < MAX_VIRTUALS)
                 && ((pVDMT = MAG_VDMT_dir[vid]) != NULL)
                 && (pVDMT->reservation != NULL))
        {
            pr_delAssociation(pVDMT, sid);
        }
    }
    else
    {
        rVal = DEFAILED;
    }
    return (rVal);
}

/**
******************************************************************************
**
**  @brief      Updates up the local PRR config structs for Server-VDisk
**              association removal
**
**  @param      VDMT *pVDMT
**              UINT16 sid
**
**  @return     none
**
******************************************************************************
**/
void pr_delAssociation(VDMT *pVDMT, UINT16 sid)
{
    UINT32 i;
    INT16  indx;
    UINT32 cnt = 0;
    ILMT   *pILMT = NULL;

    for (indx = 0; indx < MAX_KEYS; indx++)
    {
        if((pVDMT->reservation->keyset[indx] != NULL)
                    && (pVDMT->reservation->keyset[indx]->sid == sid)
                    && (pVDMT->reservation->keyset[indx]->vid == pVDMT->vid))
        {
            /*
            ** Free the corresponding keyset. If this is the only key in the
            ** reservation struct, free the reservation struct too
            */
            s_Free((void *)pVDMT->reservation->keyset[indx], sizeof(REGKEYS), __FILE__, __LINE__);
            pVDMT->reservation->keyset[indx] = NULL;

            /*
            ** Found the key. Now check if this the the key that holds the
            ** reservation. If it does, update the cmd handlers of
            ** other ILMTs if any
            */
            for(pILMT = pVDMT->ilmtHead; pILMT != NULL; pILMT = pILMT->link)
            {
                /*
                ** If there is an outstanding config update, clean up the memory
                ** allocated.
                */
                if(pILMT->imt->sid == sid)
                {
                    if(pILMT->prilt != NULL)
                    {
                        if(pILMT->prilt->misc != 0)
                        {
                            s_Free((void *)pILMT->prilt->misc, (sizeof(RESV) + (sizeof(REGKEYS) * MAX_KEYS)), __FILE__, __LINE__);
                            pILMT->prilt->misc = 0x0;
                        }
                        pILMT->prilt = NULL;
                    }
                }
                if(pVDMT->reservation->rsvdIdx == indx)
                {
                    pILMT->cmdhndl = (void *)&cmdtbl1;
                    if(((pVDMT->reservation->resvType == RESV_WR_EXCL_RO)
                            || (pVDMT->reservation->resvType == RESV_EXCL_ACC_RO))
                            && (findNexus(pILMT, pVDMT->reservation) != -1))
                    {
                        BIT_SET(pILMT->flag3, PR_UA_RESV_RELEASED);
                    }
                }
            }
            if(pVDMT->reservation->rsvdIdx == indx)
            {
                pVDMT->reservation->rsvdIdx = -1;
            }
            /*
            ** If the registrations are empty, free the reservation struct.
            */
            for (i = 0; i < MAX_KEYS; i++)
            {
                cnt += (pVDMT->reservation->keyset[i] != NULL) ? 1 : 0;
            }
            if(cnt == 0)
            {
                s_Free(pVDMT->reservation, sizeof(RESV), __FILE__, __LINE__);
                pVDMT->reservation = NULL;
            }
            prDump(pVDMT->vid, __FILE__, __LINE__, __func__);
            break;
        }
    }
}

/**
******************************************************************************
**
**  @brief
**
******************************************************************************
**/
UINT8 get_pr_data(UINT16 vid, MRPRGET_RSP* p_resp)
{
   VDMT        *p_vdmt;
   INT32       keyIndx;
   REGKEYS     *p_resvHolder;
   RESV        *p_resv;

   /*
   ** Find the vdmt from the vid.  Bail if anything goes a-wry.
   */
   if(vid >= MAX_VIRTUALS)
   {
      return(DEINVVID);
   }
   if(!(p_vdmt = MAG_VDMT_dir[vid]))
   {
      return(DEINVVID);
   }

   /*
   ** Fill in the blanks.
   */
   p_resp->vid = vid;

   if(!(p_resv = p_vdmt->reservation))
   {
      return(DEOK);
   }

   if (p_resv->rsvdIdx == -1)
   {
      /*
      ** There is no reservation, just put in stuff to indicate that.
      */
      p_resp->sid = 0xFFFF;
      p_resp->tid = 0xFF;
      p_resp->lun = 0xFF;
      p_resp->scope = 0xFF;
   }
   else
   {
      /*
      ** There is a valid reservation. Fill up the right values.
      */
      p_resvHolder = p_resv->keyset[p_resv->rsvdIdx];

      p_resp->sid = p_resvHolder->sid;
      p_resp->tid = p_resvHolder->tid;
      p_resp->lun = p_resvHolder->lun;
      p_resp->scope = (p_resv->scope << 4) | p_resv->resvType;
   }

   p_resp->keyCnt = 0;
   for (keyIndx = 0; keyIndx < MAX_KEYS; keyIndx++)
   {
      if(p_resv->keyset[keyIndx] != NULL)
      {
         memcpy(&p_resp->keyList[p_resp->keyCnt*PR_KEY_SIZE], p_resv->keyset[keyIndx]->key, 8);
         p_resp->keyCnt++;
      }
   }
   return(DEOK);
}

/**
******************************************************************************
**
**  @brief      Send log msg to update BE persistent reservation info.
**              Create a log msg from the RESV structure and send it to BE.
**
**  @param      ILMT *pILMT
**
**  @return     none
**
******************************************************************************
**/
void pr_cfgUpdate (ILMT *pILMT)
{
    VDMT *pVDMT = pILMT->vdmt;
    RESV *pRSV  = pVDMT->reservation;
    MRSETPRES_REQ* pReq;
    LOG_PRES_EVENT_PKT* pLOG;
    UINT32 cnt = 0;
    INT16  i = 0;
    UINT32 j = 0;


    /*
    ** Find the number of keys to determine the data size
    */
    if(pRSV != NULL)
    {
        for (i = 0; i < MAX_KEYS; i++)
        {
            cnt += (pRSV->keyset[i] != NULL) ? 1 : 0;
        }
    }

    /*
    ** Allocate the log msg and initialize the fields
    */
    pLOG = (LOG_PRES_EVENT_PKT *)s_MallocC(sizeof(LOG_PRES_EVENT_PKT), __FILE__, __LINE__);

    pLOG->vid      = pVDMT->vid;
    pLOG->sid      = pILMT->imt->sid;
    pLOG->dataSize = sizeof(MRSETPRES_REQ) + (cnt * sizeof(MRREGKEY));

    /*
    ** Allocalte the data buffer. This buffer will be freed in deffe.c
    ** in the completion MRP handler from the CCB.
    */
    pReq = (MRSETPRES_REQ *)s_MallocC(pLOG->dataSize, __FILE__, __LINE__);
    pLOG->data     = pReq;

    pReq->vid      = pVDMT->vid;
    pReq->sid      = 0xFFFF;
    pReq->regCount = cnt;

    /*
    ** Prepare the struct with the config update info
    */
    if(cnt > 0)
    {
        pReq->scope = pRSV->scope;
        pReq->type = pRSV->resvType;

        j = 0;
        for (i = 0; (i < MAX_KEYS) && (j < cnt); i++)
        {
            if(pRSV->keyset[i] != NULL)
            {
                if (i == pRSV->rsvdIdx)
                {
                    pReq->sid = pRSV->keyset[i]->sid;
                }
                pReq->keyList[j].sid = pRSV->keyset[i]->sid;
                pReq->keyList[j].tid = pRSV->keyset[i]->tid;
                pReq->keyList[j].lun = pRSV->keyset[i]->lun;
                memcpy(pReq->keyList[j].key, pRSV->keyset[i]->key, 8);
                j++;
            }
        }
    }
    pLOG->header.event = LOG_PRES_EVENT;
    /*
    ** Completion routine frees memory later.
    */
    MSC_LogMessageRel(pLOG, sizeof(LOG_PRES_EVENT_PKT));

}

/**
******************************************************************************
**
**  @brief      Send an MRP req to the BE requesting the PRR config.
**              Create a MRP request and sent it to the BE. The response is
**              handled in a callback where the FE PR structs are updated.
**
**  @param      UINT16 VID
**
**  @return     none
**
******************************************************************************
**/
void pr_cfgRetrieve(UINT16 vid)
{
    MRRETRIEVEPR_REQ *pReq;
    MRRETRIEVEPR_RSP *pRsp;
    ILMT *pILMT = MAG_VDMT_dir[vid]->ilmtHead;


    pReq = (MRRETRIEVEPR_REQ *)s_MallocC(sizeof(MRRETRIEVEPR_REQ), __FILE__, __LINE__);
    pRsp = (MRRETRIEVEPR_RSP *)s_MallocC(sizeof(MRRETRIEVEPR_RSP) + (sizeof(MRREGKEY) * MAX_KEYS),
    __FILE__, __LINE__);

    pReq->vid = vid;

    LL_SendPacket( pReq, sizeof(MRRETRIEVEPR_REQ), MRRETRIEVEPR, pRsp,
                (sizeof(MRRETRIEVEPR_RSP) + (sizeof(MRREGKEY) * MAX_KEYS)),
                (void*)&cbRetrieve, (UINT32)vid);
    /*
    ** For now, set the ILMT cmd handler to default and block any SCSI reqs processing
    ** for this VID until the config retrieval is complete!
    */
    pILMT->cmdhndl = (void *)&cmdtbl1;
    BIT_SET(pILMT->flag3, PR_CFGRETRIEVE);

    /*
    ** Req size being small, is copied to the request while sending across
    ** in the link layer. So, the pReq is not needed once the above call returns.
    ** Moreover, the pReq in the MRP struct will not be what we allocated - in
    ** the callback - so we have to free the pReq here!!! Figured this out
    ** after running into crash while freeing the pReq in the callback!!!
    */
    s_Free((void *)pReq, sizeof(MRRETRIEVEPR_REQ), __FILE__, __LINE__);
}

/**
******************************************************************************
**
**  @brief      Callback function that deals with PRR config retrieved from BE
**
**  @param      retCode, Pointer to ILT, Pointer MRP, param if any
**
**  @return     none
**
******************************************************************************
**/
void cbRetrieve(UINT32 rc, ILT *pILT UNUSED, MR_PKT *pMRP, UINT32 param)
{
    UINT32 i;
    INT32 rIndx = -1;
    ILMT *pILMT = NULL;
    UINT16 vid  = (UINT16)param;
    VDMT *pVDMT = MAG_VDMT_dir[vid];
    MRRETRIEVEPR_RSP *pRsp = (MRRETRIEVEPR_RSP *)pMRP->pRsp;

    fprintf (stderr, "[%s:%d\t%s]: cbRetrieve vid=%d rc = %d\n",__FILE__, __LINE__,__func__, vid, rc);

    if(pVDMT != NULL)
    {
        /*
        ** If the PRR structs exist, clean them up
        */
        if(pVDMT->reservation != NULL)
        {
            for (i = 0; i < MAX_KEYS; i++)
            {
               if(pVDMT->reservation->keyset[i] != NULL)
               {
                   s_Free(pVDMT->reservation->keyset[i], sizeof(REGKEYS), __FILE__, __LINE__);
               }
            }
            s_Free(pVDMT->reservation, sizeof(RESV), __FILE__, __LINE__);
            pVDMT->reservation = NULL;
        }

        pVDMT->prGen = 0;
        /*
        ** Create and update the new structs from the response data
        */
        if(pRsp->regCount > 0)
        {
            pVDMT->reservation = (RESV *)s_MallocC(sizeof(RESV), __FILE__, __LINE__);
            memset((void *)pVDMT->reservation, 0, sizeof(RESV));
            pVDMT->reservation->vid      = vid;
            pVDMT->reservation->scope    = pRsp->scope;
            pVDMT->reservation->resvType = pRsp->type;
            pVDMT->reservation->maxIdx   = pRsp->regCount;
            pVDMT->reservation->rsvdIdx  = -1;

            for (i = 0; i < pRsp->regCount; i++)
            {
                pVDMT->reservation->keyset[i] = (REGKEYS *)s_MallocC(sizeof(REGKEYS), __FILE__, __LINE__);

                pVDMT->reservation->keyset[i]->vid = vid;
                pVDMT->reservation->keyset[i]->sid = pRsp->keyList[i].sid;
                pVDMT->reservation->keyset[i]->tid = pRsp->keyList[i].tid;
                pVDMT->reservation->keyset[i]->lun = pRsp->keyList[i].lun;
                memcpy(pVDMT->reservation->keyset[i]->key, pRsp->keyList[i].key, 8);

                if (pRsp->keyList[i].sid == pRsp->sid)
                {
                    pVDMT->reservation->rsvdIdx = i;
                }
            }
            /*
            ** Now that the config update is complete, update the ILMT
            ** cmd handlers ro reflect the reservation change
            */
            if(pVDMT->reservation->rsvdIdx != -1)
            {
                /*
                ** Reservation exists. Walk thru all the ILMTs and update the
                ** cmd handlers accordingly. NOTE: If the server tries to access
                ** the volumes before the this asynchronous completion is complete,
                ** and if a reservation exists, then we could potentially run into
                ** where the hosts could have wrong cmd handlers causing improper
                ** access control.
                */
                for (pILMT = pVDMT->ilmtHead; pILMT != NULL; pILMT = pILMT->link)
                {
                    if((rIndx = findNexus(pILMT,pVDMT->reservation)) == pVDMT->reservation->rsvdIdx)
                    {
                        pILMT->cmdhndl = (void *)&cmdtbl1;
                    }
                    else
                    {
                        pILMT->origcmdhand = pILMT->cmdhndl;
                        switch(pVDMT->reservation->resvType)
                        {
                            case RESV_WR_EXCL:
                                pILMT->cmdhndl = (void *)&cmdtbl4;
                                break;
                            case RESV_EXCL_ACC:
                                pILMT->cmdhndl = (void *)&cmdtbl3;
                                break;
                            case RESV_WR_EXCL_RO:
                                pILMT->cmdhndl = (rIndx != -1) ?
                                                (void *)&cmdtbl1 : (void *)&cmdtbl5;
                                break;
                            case RESV_EXCL_ACC_RO:
                                pILMT->cmdhndl = (rIndx != -1) ?
                                                (void *)&cmdtbl1 : (void *)&cmdtbl6;
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
        }
        /*
        ** If there are any IO tasks that were blocked due to config restore,
        ** unblock and enable them.
        */
        for (pILMT = pVDMT->ilmtHead; pILMT != NULL; pILMT = pILMT->link)
        {
            if(BIT_TEST(pILMT->flag3, PR_CFGRETRIEVE))
            {
                BIT_CLEAR(pILMT->flag3, PR_CFGRETRIEVE);
                MAG_CheckNextTask(pILMT);
            }
        }
        prDump(vid, __FILE__, __LINE__, __func__);
    }
    s_Free((void *)pRsp, (sizeof(MRRETRIEVEPR_RSP) + (sizeof(MRREGKEY) * MAX_KEYS)), __FILE__, __LINE__);
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
