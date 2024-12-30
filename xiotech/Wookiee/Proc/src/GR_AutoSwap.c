/* $Id: GR_AutoSwap.c 159663 2012-08-22 15:36:42Z marshall_midden $ */
/**
*******************************************************************************
**
** @file:        GR_AutoSwap.c
**
** @brief:       This file contains the functions to do Auto RAID-Swap and Swap
**               back routines for GeoRAID feature. These functions are called
**               from VdiskErrorHandler module.
**
** @auther:      Boddukuri Vishweshwar
**
** Copyright (c) 2005-2010 XIOtech Corporation.  All rights reserved.
**
*******************************************************************************
**/

#include "globalOptions.h"
#include "defbe.h"
#include "GR_AutoSwap.h"
#include "GR_Error.h"
#include "cm.h"
#include "ilt.h"
#include "kernel.h"
#include "cor.h"
#include "ccsm.h"
#include "MR_Defs.h"
#include "nvram.h"
#include "scd.h"
#include "system.h"
#include "string.h"

#include "dcd.h"
#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "XIO_Types.h"
#include "pcb.h"
#include "stdio.h"
#include "stdlib.h"
#include "ddr.h"
#include "misc.h"

#ifdef HISTORY_KEEP
extern void CT_history_pcb(const char *str, UINT32 pcb);
#endif
/*
******************************************************************************
** Private defines
******************************************************************************
*/
#define SWAP_MIRROR 0
#define SWAP_BREAK  1

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
extern unsigned long CT_fork_tmp;
PCB* gAutoSwapTaskTable[GR_MAX_ASWAPS];
GR_SEND_SWAP_PKT g_swap_req_pkt;
GR_SWAP_COMP_PKT g_swap_comp_pkt;
extern UINT8 g_ccsm_op_state;

/*
********************************************************************************
** Private function prototypes
********************************************************************************
*/
void GR_ResumeUserPausedMirrors(VDD *pSrcVDD);
extern void CT_LC_GR_AutoSwapTask(int);
void GR_AutoSwapTask(UINT32, UINT32, UINT32, UINT32);
UINT32 GR_IsSwapAllowed(UINT32 srcOpState, UINT32 destOpstate);
UINT32 GR_IsSwapBackAllowed(UINT32 srcOpState, UINT32 destOpstate);
void GR_SendSwapEventToMaster(COR *);
void GR_SendAutoswapCompEventToOwner(COR *pCOR, UINT8 status);
UINT32 GR_findFreeAswapPCBIdx(void);

/*
********************************************************************************
** Public function prototypes - defined in other files
********************************************************************************
*/
extern COR  *CM_find_cor_rid(UINT32 , UINT32);
extern void CCSM_swap_raids(UINT32, COR *);
extern void CCSM_resume_copy(COR*);
extern void GR_RaidSwapEventHandler(void *p_req, UINT32 len);
extern void GR_NotifySwapComp(COR *pCOR);

/*
********************************************************************************
** Code Start
********************************************************************************
*/

/**
******************************************************************************
**
**  @brief      This function provides a common means of Creating Task for
**              Auto Raid Swap of mirrored virtual devices.
**
**  @param     VDD, opType (Auto Swap or Swapback) and IOType (Write/ read)
**
**  @return    status
**
******************************************************************************
**/
UINT32 GR_AutoSwap(VDD *pVDD, UINT8 opType, UINT8 IOType)
{
    UINT32 i;
#if GR_MYDEBUG1
    fprintf(stderr,"<GR_AutoSwap>: SourceVDD:%x, Op Type:%x, IO Type:%x\n",
            (UINT32)pVDD,opType,IOType);
#endif
    if (opType == GR_VD_ASWAP_OP)
    {
        i = GR_findFreeAswapPCBIdx();

        if (i == GR_MAX_ASWAPS)
        {
            return ERROR;
        }
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        pVDD->grInfo.aswapProcessIdx = (UINT8)i;
        CT_fork_tmp = (unsigned long)"GR_AutoSwapTask";
// Flag that this entry is in use -- TaskCreate may task switch.
        gAutoSwapTaskTable[i] = (PCB*)(-1);
        gAutoSwapTaskTable[i] = TaskCreate4(
                                C_label_referenced_in_i960asm(GR_AutoSwapTask),
                                GR_AUTOSWAP_PRIORITY,
                                (UINT32)pVDD,
                                (UINT32)IOType
                                );
#if GR_MYDEBUG1
        fprintf(stderr,"<GR_AutoSwap>: AutoSwap Task Created, PCB:%x\n",
            (UINT32)gAutoSwapTaskTable[i]);
#endif
        return GOOD;
    }
    else
    {
        return(ERROR); /* Invalid Op type */
    }
}

/**
******************************************************************************
**
**  @brief      This function provides a common means of Auto Raid swap of
**              mirrored virtual devices. It calls the functions provided by
**              copy manager. This function performs auto swap.
**
**  @param     VDD pointer and IOType
**
**  @return    status
**
******************************************************************************
**/
void GR_AutoSwapTask(UINT32 dummy1 UNUSED, UINT32 dummy2 UNUSED,
                     UINT32 pVDD, UINT32 IOType UNUSED)
{
    UINT8 vdmap[(MAX_VIRTUAL_DISKS+7)/8];
    UINT8 iSelections = 0;
    UINT8 done = 0;
    VDD *pSrcVDD;
    VDD *pDestVDD;
    COR *pCOR;
    SCD *pSCD;
    DCD *pDCD;
    UINT32 dopst;
    UINT32 decide;

    memset(vdmap, 0, (MAX_VIRTUAL_DISKS+7)/8); /* Initialize the vdisk bit map to 0 */

    pSrcVDD = (VDD*)pVDD;

#if GR_MYDEBUG1
    fprintf(stderr,"<GR_AutoSwapTask>:Source VDD:%x\n",(UINT32)pSrcVDD);
#endif
    while ((iSelections <= 1) && (!done))
    {
        for (pSCD = pSrcVDD->pSCDHead; pSCD != NULL; pSCD = pSCD->link)
        {
            pCOR    = pSCD->cor;
            pDestVDD = pCOR->destvdd;
#if GR_MYDEBUG1
            fprintf(stderr,"<GR_AutoSwapTask>:SCD:%x, COR:%x, DestVDD:%x\n",
                    (UINT32)pSCD,(UINT32)pCOR,(UINT32)pDestVDD);
            if (pCOR->rmaptbl != NULL)
            {
               fprintf(stderr,"<GR_AutoSwapTask>: Src=%u Dest=%u RMAP exists\n",
                       pSrcVDD->vid, pDestVDD->vid);
            }
#endif
            /*
            ** Make sure the destination device has not been already tried
            */
            if (BIT_TEST(vdmap[(pDestVDD->vid)/8],(pDestVDD->vid)%8) == 0)
            {
                /*
                ** Make sure the destination VDD is Georaid vdisk and it is in sync with
                ** Src at the time of source vdisk failure.
                */
                pDCD = (DCD*)(pCOR->dcd);

#if 0 // Enable this piece of code to fix SAN-126/SAN-735
                if (GR_IS_GEORAID_VDISK(pDestVDD) && (GR_IS_VD_INSYNC_AT_SRCFAIL(pDestVDD)))
#else
                /*
                ** Also ensure that the GeoRaid vdisks are not of vlink type
                */
                if (GR_IS_GEORAID_VDISK(pDestVDD) && (GR_IS_VD_INSYNC_AT_SRCFAIL(pDestVDD)) &&
                    ((pSCD->type == SCDT_BOTH) && (pDCD->type == DCDT_BOTH)))

#endif
                {
                    if (iSelections == 0)
                    {
                       if (GR_IS_PARNTER_AT_DIFFERENT_LOCATION(pDestVDD) == FALSE)
                       {
                           continue;    /* take next destination VDD */
                       }
                    }
                    dopst = GR_GetVdiskOpState(pDestVDD, GR_VDOPSTATE_BY_RDDSTAT);
                    decide = GR_IsSwapAllowed(pSrcVDD->grInfo.vdOpState, dopst);
                    if (decide == GR_ASWAP_ALLOWED)
                    {
                        if (pCOR == CM_find_cor_rid (pCOR->rid, pCOR->rcsn))
                        {

#if GR_MYDEBUG1
                            fprintf(stderr,"<GR_AutoSwapTask>:Swap is allowed COR found \
                                    on active queue COR:%x\n",(UINT32)pCOR);
#endif
                            if (g_ccsm_op_state == CCSM_ST_MASTER)
                            {
#if GR_MYDEBUG1
                                fprintf(stderr,"<GR_AutoSwapTask>:CCSM is master:%d.. \
                                        going to generate swap_raids event.\n",g_ccsm_op_state);
#endif
                                CCSM_swap_raids(SWAP_MIRROR, pCOR);
                            }
                            else
                            {
#if GR_MYDEBUG1
                                fprintf(stderr,"<GR_AutoSwapTask>:CCSM is Slave:%d, \
                                        going to send swap event to Master.\n",g_ccsm_op_state);
#endif
                                GR_SendSwapEventToMaster(pCOR);
                            }
                            /* This sets my task state. */
                            TaskSetState(gAutoSwapTaskTable[pSrcVDD->grInfo.aswapProcessIdx],
                                             PCB_NOT_READY);
                            TaskSwitch();
                            /*
                            ** AutoSwap Completed
                            */
                            if (!GR_IS_ASWAP_FAILED(pSrcVDD))
                            {
                                done = 1;
                                break;
                            }

                            /*
                            ** Mark that the destination vdd as tried
                            */
                            BIT_SET(vdmap[(pDestVDD->vid)/8], (pDestVDD->vid)%8);

                            GR_CLEAR_ASWAP_FAILED(pSrcVDD);
                            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                        }
                    }
                }
            }
        }
        iSelections++;
    }
    if (done == 0)
    {
        /*
        **  Auto swap failed with all it's partners, Inform vdisk error handling completer
        */
        GR_VdiskErrorCompleter(pSrcVDD, NULL, GR_ASWAP_FAILED);
    }

    /* Done with this PCB. Hence make this slot free */
    gAutoSwapTaskTable[pSrcVDD->grInfo.aswapProcessIdx] = NULL;
    pSrcVDD->grInfo.aswapProcessIdx = 0;
}

/**
******************************************************************************
**
**  @brief      This function provides a common means of Auto Raid swapback of
**              mirrored virtual devices. It calls the functions provided by
**              copy manager.
**
**  @param      Dest. VDD pointer, IOType
**
**  @return    status
**
******************************************************************************
**/
void GR_AutoSwapBack(VDD *pSrcVDD, VDD *pDestVDD)
{
    COR    *pCOR;
    UINT32  decide;
    SCD    *pSCD;

    pCOR = ((DCD *)(pDestVDD->pDCD))->cor;
    pSCD = (SCD*)(pCOR->scd);
    decide = GR_IsSwapBackAllowed(pSrcVDD->grInfo.vdOpState, pDestVDD->grInfo.vdOpState);
#if GR_MYDEBUG1
    fprintf(stderr,"<GR_AutoSwapBack> decide=%u svid=%x dvid=%x s-opstate=%x d-opstate=%x\n",
            decide, (UINT32)(pSrcVDD->vid),(UINT32)(pDestVDD->vid),pSrcVDD->grInfo.vdOpState,
             pDestVDD->grInfo.vdOpState);
#endif
    /*
     * Check whether the swap back is allowed based on the vd opstates and
     * the cor is found on the active cor queue
     */
    if ((decide == GR_ASWAPBACK_ALLOWED) && (pCOR == CM_find_cor_rid(pCOR->rid, pCOR->rcsn)))
    {
#if GR_MYDEBUG1
    fprintf(stderr,"<GR_AutoSwapBack>: Autoswapback process begins...\n");
#endif
        if (g_ccsm_op_state == CCSM_ST_MASTER)
        {
            CCSM_swap_raids(SWAP_MIRROR, pCOR);
        }
        else
        {
            GR_SendSwapEventToMaster(pCOR);
        }
    }
    else
    {
#if 1 /* GR_MYDEBUG1 */
        fprintf(stderr,"<GR_AutoSwapBack>COR not found  OR opState Mismatch(svid/opstate=%x/%x dvid/opstate=%x/%x)\n",
                   pSrcVDD->vid,pSrcVDD->grInfo.vdOpState, pDestVDD->vid,pDestVDD->grInfo.vdOpState);
#endif
        GR_AutoSwapComp(pSrcVDD, pDestVDD, GR_ASWAP_FAILED);
    }
}

/**
******************************************************************************
**
**  @brief  This function is the completion routine for Auto raid swap.
**
**  @param  VDD pointer, IOType and completion status
**
**  @return None
**
******************************************************************************
**/
void GR_AutoSwapComp(VDD *pSrcVDD, VDD *pDestVDD, UINT8 status)
{
    if (GR_VD_ASWAP_OPTYPE(pSrcVDD) == GR_VD_ASWAP_OP)
    {
        if (status == GR_ASWAP_SUCCESS || IS_GR_VD_SWAP_DONE_AT_OWNER(pSrcVDD))
        {
            GR_VdiskErrorCompleter(pSrcVDD, pDestVDD, GR_ASWAP_SUCCESS);
        }
        else
        {
            GR_SET_ASWAP_FAILED(pSrcVDD);   /* CCB_DMC_vdiskcache bit set below. */
        }

        CLEAR_GR_VD_SWAP_DONE_AT_OWNER(pSrcVDD);

        /* Set the autoswap task state to ready */
        if (TaskGetState(gAutoSwapTaskTable[pSrcVDD->grInfo.aswapProcessIdx]) == PCB_NOT_READY)
        {
#ifdef HISTORY_KEEP
CT_history_pcb("GR_AutoSwapComp setting ready pcb", (UINT32)gAutoSwapTaskTable[pSrcVDD->grInfo.aswapProcessIdx]);
#endif
            TaskSetState(gAutoSwapTaskTable[pSrcVDD->grInfo.aswapProcessIdx], PCB_READY);
        }
    }
    else if (GR_VD_ASWAP_OPTYPE(pSrcVDD) == GR_VD_ASWAPBACK_OP)
    {
        if (status == GR_ASWAP_SUCCESS || IS_GR_VD_SWAP_DONE_AT_OWNER(pSrcVDD))
        {
             GR_VdiskErrorCompleter(pSrcVDD, pDestVDD, GR_ASWAP_SUCCESS);
        }
        else
        {
             GR_VdiskErrorCompleter(pSrcVDD, pDestVDD, GR_ASWAP_FAILED);
        }
        CLEAR_GR_VD_SWAP_DONE_AT_OWNER(pSrcVDD);
    }
    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
}

/**
******************************************************************************
**
**  @brief     This function sets the Swap done at Owner bit on Src VDD when
**             the actual RAID Pointers swapped in Copy Manager module.
**
**  @param     Source VDD pointer
**
**  @return    none
**
******************************************************************************
**/
void GR_SetSwapDoneFlagOnSrcVDD(VDD *pSrcVDD)
{
    if (GR_IS_VD_ASWAP_INPROGRESS(pSrcVDD) || GR_IS_VD_ASWAPBACK_INPROGRESS(pSrcVDD))
    {
        SET_GR_VD_SWAP_DONE_AT_OWNER(pSrcVDD);
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
    }
}

/**
******************************************************************************
**
**  @brief    This function  issues Resume Copy event to the CCSM to resume copies
**            which went into user-pause state due to AutoSwap Operation.
**
**  @param    Source VDD pointer
**
**  @return    none
**
******************************************************************************
**/
void GR_ResumeUserPausedMirrors(VDD *pSrcVDD)
{
    SCD *pSCD;
    COR *pCOR;
    VDD *pDestVDD;

    for (pSCD = pSrcVDD->pSCDHead; pSCD != NULL; pSCD = pSCD->link)
    {
        pCOR = pSCD->cor;
        pDestVDD = pCOR->destvdd;
#if GR_MYDEBUG1
        fprintf(stderr,"<ResumeUserPausedMirrors>srcvid =%u, destvid=%u crstate = %u..",
                    (UINT32)(pSrcVDD->vid), (UINT32)(pDestVDD->vid) ,(UINT32)(pCOR->crstate));
        if (GR_IS_VD_INSYNC_AT_SRCFAIL(pDestVDD))
        {
           fprintf(stderr,"SRCFAIL BIT is SET\n");
        }
        else
        {
           fprintf(stderr,"SRCFAIL Bit is not SET\n");
        }
#endif
        if (GR_IS_VD_INSYNC_AT_SRCFAIL(pDestVDD) && (pCOR->crstate == CRST_USERSUSP) )
        {
#if GR_MYDEBUG1
            fprintf(stderr,"<ResumeUserPausedMirrors>srcvid =%u, destvid=%u in USRPAUSE..\n",
                    (UINT32)(pSrcVDD->vid), (UINT32)(pDestVDD->vid) );
#endif
            CCSM_resume_copy(pCOR);
        }
    }
}

/**
******************************************************************************
**
**  @brief      This function checks whether the swap is allowed between the
**              given Src and Dest Virtual Disks based on their operation state,
**              returns either GR_SWAP_ALLOWED or GR_SWAP_NOT_ALLOWED.
**
**  @param      srcOpState, destOpstate
**
**  @return     status
**
******************************************************************************
**/
UINT32 GR_IsSwapAllowed(UINT32 srcOpState, UINT32 destOpState)
{
    UINT32 status = GR_ASWAP_NOT_ALLOWED;

    if (srcOpState == GR_VD_DEGRADED)
    {
            status = GR_ASWAP_NOT_ALLOWED;
        }
    else if (srcOpState == GR_VD_INOP)
    {
        if (destOpState == GR_VD_OP || destOpState == GR_VD_DEGRADED)
        {
            status = GR_ASWAP_ALLOWED;
        }
        else
        {
            status = GR_ASWAP_NOT_ALLOWED;
        }
    }
    return(status);
}

/**
******************************************************************************
**
**  @brief     This function checks whether the swapback is allowed between the
**             given Src and Dest Virtual Disks based on their operation state.
**             This function returns either GR_SWAP_BACK_ALLOWED or
**             GR_SWAP_BACK_NOT_ALLOWED.
**
**  @param     srcOpstate, destOpstate
**
**  @return    status
**
******************************************************************************
**/
UINT32 GR_IsSwapBackAllowed(UINT32 srcOpState, UINT32 destOpState)
{
    UINT32 status = GR_ASWAPBACK_NOT_ALLOWED;

    if ((srcOpState == GR_VD_DEGRADED)|| (srcOpState == GR_VD_OP))
        {
        if (destOpState == GR_VD_OP)
    {
        status = GR_ASWAPBACK_ALLOWED;
    }
    }

    return(status);
}

/**
******************************************************************************
**
**  @brief    This function packs and sends swap raids request to ccsm on master
**            controller. A new request is formed with new event type and
**            function code.
**
**  @param    Pointer to COR
**
**  @return
**
******************************************************************************
**/
void GR_SendSwapEventToMaster(COR *pCOR)
{
#if GR_MYDEBUG1
    fprintf(stderr,"<GR_SendSwapEventToMaster>:Pack and send to Master, COR:%x\n",(UINT32)pCOR);
#endif
    g_swap_req_pkt.ccsm_event.len = sizeof(g_swap_req_pkt);
    g_swap_req_pkt.ccsm_event.type = CCSM_ET_GRS; /*Event type code*/
    g_swap_req_pkt.ccsm_event.fc   = CCSM_GRS_SWAP;  /* Event function code*/
    g_swap_req_pkt.ccsm_event.sendsn = K_ficb->cSerial;
    g_swap_req_pkt.swap_info.copy_reg_id = pCOR->rid; /* copy registration id*/
    g_swap_req_pkt.swap_info.cmsn = pCOR->rcsn;  /* cm serial number*/
    g_swap_req_pkt.swap_info.type = SWAP_MIRROR; /* SWAP_MIRROR=0 and SWAP_BREAK=1*/

    DEF_ReportEvent(
                &g_swap_req_pkt,
                sizeof(g_swap_req_pkt),
                PUTDG,
                DGBTMASTER,
                0
                );
}

/**
******************************************************************************
**
**  @brief    This function is handler for: 1. Raid swap request from slave
**            controller(Copy owner). This function generates a swap raids
**            define event on master controller.
**            2. Raid swap completed event from master controller, it calls
**            the auto swap completer routine.
**
**  @param    Pointer to request packet, length of the packet
**
**  @return   none
**
******************************************************************************
**/
void GR_RaidSwapEventHandler(void *p_req, UINT32 len UNUSED)
{
    GR_SEND_SWAP_PKT *p_pkt;
    GR_SWAP_COMP_PKT *p_cpkt;
    COR              *pCOR;

    p_pkt = (GR_SEND_SWAP_PKT *)p_req;

    if (p_pkt->ccsm_event.fc == CCSM_GRS_SWAP)
    {
        /*
        ** Raid swap request received from slave ccsm
        */
        pCOR = CM_find_cor_rid(p_pkt->swap_info.copy_reg_id, p_pkt->swap_info.cmsn);
        if (pCOR != NULL)
        {
            CCSM_swap_raids(p_pkt->swap_info.type, pCOR);
        }
        else
        {
            /*
            ** COR not found on active queue, swap can not be performed !!!
            ** Need to inform the autoswap completer that it is failed !!!
            */
            fprintf(stderr,"GR_AutoSwap: Master received AutoSwap event,\
                   but the cor is not found on the active COR queue\n");
        }
    }
    else if (p_pkt->ccsm_event.fc == CCSM_GRS_SWAP_COMP)
    {
        /*
        ** Raid Swap completed event received from Master ccsm
        */
        p_cpkt = (GR_SWAP_COMP_PKT*)p_req;
        pCOR = CM_find_cor_rid(p_cpkt->swap_comp_info.copy_reg_id, p_cpkt->swap_comp_info.cmsn);

        if ( pCOR != NULL)
        {
            if ((pCOR->srcvdd != NULL) && (pCOR->destvdd != NULL))
            {
            if (GR_IS_VD_WAITING_FOR_ASWAP_COMP(pCOR->srcvdd)||
                GR_IS_VD_WAITING_FOR_ASWAPBACK_COMP(pCOR->srcvdd))
            {
#if GR_MYDEBUG1
                    fprintf(stderr,"AutoSwap/Swapback Comp event received from master\n");
                    fprintf(stderr,"Calling AutoSwap Completer\n");
#endif
                GR_AutoSwapComp(pCOR->srcvdd, pCOR->destvdd, p_cpkt->swap_comp_info.status);
            }
            else
            {
#if GR_MYDEBUG1
          fprintf(stderr,"<GR_RaidSwapEventHandler>user swap operation-- srvid=%u destvid=%u\n",
             (UINT32)(pCOR->srcvdd->vid),(UINT32)(pCOR->destvdd->vid));
          fprintf(stderr,"<GR_RaidSwapEventHandler> Calling GR_UpdateGeoInfoAtCmdMgrReq()..owner=slave\n");
#endif
                if (pCOR->swapcompstat == GR_ASWAP_SUCCESS)
                {
                    GR_UpdateVdiskOpState(pCOR->srcvdd, GR_VDOPSTATE_BY_RDDSTAT, 0);
                    GR_UpdateVdiskOpState(pCOR->destvdd, GR_VDOPSTATE_BY_RDDSTAT, 0);
                    GR_SaveToNvram (pCOR->srcvdd, pCOR->destvdd);
                    GR_UpdateGeoInfoAtCmdMgrReq(pCOR, MVCSWAPVD);
            }
        }
        }
        else
        {
               fprintf(stderr,"There is no SrcVdd or DestVdd associated with this Copy\n");
           }
        }
        else
        {
            fprintf(stderr,"GR_AutoSwap: Received CCSM_GRS_SWAP_COMP, but there\
                     is no COR associated with this copy\n");
        }
    }
    else if (p_pkt->ccsm_event.fc == CCSM_GRS_NVRAM)
    {
        /*
        ** NVRAM update event from slave controller
        */
        GR_NvramUpdatePkt((GR_NVRAM_PKT*)p_req);
    }

}

/**
******************************************************************************
**
**  @brief    This function notifies the Autoswap component that the swap is
**            completed (either succeful or fail). If the controller is owner
**            of this perticular copy, it will directly call the Completer of
**            Autoswap component, otherwise it will send event to the copy
**            owner controller. This function is called from the copy manager
**            where the swap completion/failure is known.
**
**  @param    Pointer to COR
**
**  @return
**
******************************************************************************
**/
void GR_NotifySwapComp(COR *pCOR)
{
    SCD *pSCD;
    DCD *pDCD;

    /*
    ** User swaps will also enter this routine.. Hence ensure that vlinked vdisks
    ** will not enter this logic
    */
    pSCD = (SCD*)(pCOR->scd);
    pDCD = (DCD*)(pCOR->dcd);
    if (  (pSCD == NULL) || (pDCD == NULL)                       ||
         (pSCD->type != SCDT_BOTH) || (pDCD->type != DCDT_BOTH) ||
         (pCOR->destvdd == NULL)                                ||
         (pCOR->srcvdd == NULL)
      )
    {
#if 1//GR_MYDEBUG1
          fprintf(stderr,"<%s>Not Georaid swap--SCD=%p DCD=%p\n", __func__,pSCD,pDCD);
#endif
        return;
    }

    /*
    ** Check if swap is successful - If so, update operating states.
    ** Though seems to be checking for autoswap, it is in fact checks
    ** for manual swap also.
    */

    if ((K_ficb->cSerial == pCOR->powner) &&
        (pCOR->srcvdd) &&
        (pCOR->destvdd))
    {
        if ( (GR_IS_VD_WAITING_FOR_ASWAP_COMP(pCOR->srcvdd)) ||
           (GR_IS_VD_WAITING_FOR_ASWAPBACK_COMP(pCOR->srcvdd)))

        {
            GR_AutoSwapComp(pCOR->srcvdd,pCOR->destvdd, pCOR->swapcompstat);
        }
        else
        {
#if GR_MYDEBUG1
          fprintf(stderr,"<GR_NotifySwapComp>user swap operation-- srvid=%u destvid=%u\n",
             (UINT32)(pCOR->srcvdd->vid),(UINT32)(pCOR->destvdd->vid));
          fprintf(stderr,"<GR_NotifySwapComp> Calling GR_UpdateGeoInfoAtCmdMgrReq().owner=master\n");
#endif
            if (pCOR->swapcompstat == GR_ASWAP_SUCCESS)
            {
                GR_UpdateVdiskOpState(pCOR->srcvdd, GR_VDOPSTATE_BY_RDDSTAT, 0);
                GR_UpdateVdiskOpState(pCOR->destvdd, GR_VDOPSTATE_BY_RDDSTAT, 0);
                GR_SaveToNvram (pCOR->srcvdd, pCOR->destvdd);
                GR_UpdateGeoInfoAtCmdMgrReq(pCOR, MVCSWAPVD);
            }
         }
    }
    else
    {
        /*
        ** Send event to the Primary owner of copy by using the send broadcast.
        */
        GR_SendAutoswapCompEventToOwner(pCOR, pCOR->swapcompstat);

    }

    /* Clear the Swap completion status in COR. */
    pCOR->swapcompstat = 0;
}

/**
******************************************************************************
**
**  @brief  This function packs and sends Autoswap raids completion info to
**          ccsm on copy owner controller. A new request is formed with new
**          event type and function code.
**
**  @param  Pointer to COR and status
**
**  @return None
**
******************************************************************************
**/
void GR_SendAutoswapCompEventToOwner(COR *pCOR, UINT8 status)
{
    if (pCOR != NULL)
    {
        if (pCOR->powner != 0)
        {
    g_swap_comp_pkt.ccsm_event.len = sizeof(g_swap_comp_pkt);
    g_swap_comp_pkt.ccsm_event.type = CCSM_ET_GRS; /*Event type code*/
    g_swap_comp_pkt.ccsm_event.fc   = CCSM_GRS_SWAP_COMP;  /* Event function code*/
    g_swap_comp_pkt.ccsm_event.sendsn = K_ficb->cSerial;
    g_swap_comp_pkt.swap_comp_info.copy_reg_id = pCOR->rid; /* copy registration id*/
    g_swap_comp_pkt.swap_comp_info.cmsn = pCOR->rcsn;  /* cm serial number*/
    g_swap_comp_pkt.swap_comp_info.status = status;

    DEF_ReportEvent(
                &g_swap_comp_pkt,
                sizeof(g_swap_comp_pkt),
                PUTDG,
                DGBTSPEC,
                     pCOR->powner           /* pCOR->powner    Message receiver's serial number*/
                );
}
        else
        {
            /*
            ** Something wrong happened, Copy Primary owner's serial number is not correct.
            */
            fprintf(stderr, "GR_AutoSwap.c: Copy PrimaryOwner is 0, not sending SwapCompEvent to Owner\n");
        }
    }
    else
    {
        /*
        ** Something wrong happened, COR is NULL, We don't proceed to send the datagram
        */
        fprintf(stderr, "GR_AutoSwap.c: pCOR is NULL, not sending SwapCompEvent to Owner\n");
    }
}

/**
******************************************************************************
**
**  @brief    This function marks the destination VDD of a georaid copy, as
**            it is in sync with it's src when the src fails (if the dest is
**            in sync with it's src just before the src failure).
**
**  @param    pointer to destVDD
**
**  @return
**
******************************************************************************
**/
void GR_SetSyncFlagOnMirrors(VDD *pSrcVDD)
{
    SCD *pSCD;
    COR *pCOR;

    for (pSCD = pSrcVDD->pSCDHead; pSCD != NULL; pSCD = pSCD->link)
    {
        pCOR = pSCD->cor;
        if (pCOR->copystate == CST_MIRROR && pCOR->crstate == CRST_ACTIVE)
        {
            if (pSCD->cor->destvdd)
            {
                GR_SET_VD_INSYNC_AT_SRCFAIL(pSCD->cor->destvdd);
                BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            }
        }
    }

}

/**
******************************************************************************
**
**  @brief      This function clears the GR_VD_INSYNC_AT_SRCFAIL bit on all the
**              mirrors of a Source copy device
**
**  @param      Source VDD address
**
**  @return     none
**
******************************************************************************
**/
void GR_ClearSyncFlagOnMirrors(VDD *pSrcVDD)
{
    SCD *pSCD;

    /* For all the members in scdlist, clear the In Sync at Src failure bit */
    for (pSCD = pSrcVDD->pSCDHead; pSCD != NULL; pSCD = pSCD->link)
    {
        if (pSCD->cor->destvdd)
        {
            GR_CLEAR_VD_INSYNC_AT_SRCFAIL(pSCD->cor->destvdd);
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        }
    }
}

/**
******************************************************************************
**
**  @brief   This function searches for any mirrors exist for this vdisk and
**           it is in sync at present or at the time of source fail.
**
**  @param   SrcVDD
**
**  @return  TRUE if mirror exist, FALSE if no mirror exists
**
******************************************************************************
**/
UINT32 GR_AnyMirrorsExist(VDD *pSrcVDD)
{
    SCD   *pSCD;
    UINT32 status = FALSE;

    for (pSCD = pSrcVDD->pSCDHead; pSCD != NULL; pSCD = pSCD->link)
    {
        if ((pSCD->cor != NULL) && pSCD->cor->destvdd)
        {
            if ((GR_IS_VD_INSYNC_AT_SRCFAIL(pSCD->cor->destvdd)) ||
               ((pSCD->cor->copystate == CST_MIRROR) && (pSCD->cor->crstate == CRST_ACTIVE)))
            {
                status = TRUE;
                break;
            }
        }
    }
#if GR_MYDEBUG1
    fprintf(stderr,"GR_AnyMirror --ret status = %u\n",status);
#endif
    return(status);

}

/**
******************************************************************************
**
**  @brief      This function provides a common means finding a free slot in
**              global PCB pointer table to store the PCB pointer of AutoSwap
**              Task.
**
**  @param     none
**
**  @return    Index
**
******************************************************************************
**/
UINT32 GR_findFreeAswapPCBIdx(void)
{
    UINT32 i;

    for (i=0; i < GR_MAX_ASWAPS; i++)
    {
        if (gAutoSwapTaskTable[i] == NULL)
        {
            return(i);
        }
    }
    return (i);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
