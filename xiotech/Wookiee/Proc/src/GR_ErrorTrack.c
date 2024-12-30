/* $Id: GR_ErrorTrack.c 159129 2012-05-12 06:25:16Z marshall_midden $ */
/**
*******************************************************************************
**
** @file   : GR_ErrorTrack.c
**
** @brief:
**          This file contain the functions related error tracking component of
**          vdisk failover module.
**
**  Copyright (c) 2005-2010 XIOtech Corporation.  All rights reserved.
**
********************************************************************************
**/

#include "GR_Error.h"
#include "GR_AutoSwap.h"
#include "cor.h"
#include "RL_RDD.h"
#include "rebuild.h"
#include "rrp.h"
#include "scd.h"
#include "dcd.h"
#include "vdd.h"
#include "nvram.h"
#include "ecodes.h"
#include "vrp.h"
#include "ddr.h"
#include "misc.h"

#if GR_MYDEBUG1 || GR_GEORAID15_DEBUG
#include <stdio.h>
#endif

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/
#if GR_MYDEBUG1 || GR_GEORAID15_DEBUG
static char vdOpStateString[5][10]=
         { "INOP","UNINIT","INIT","OP","DEGRADED"};
#endif

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
extern UINT8 O_P2Init;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static UINT8 GR_VdiskStatusToOpState (UINT8 vdStatus);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      This function  called from virtual layer executive process
**              for each of the VRP that is posted to virtual layer.
**
**              This clears all  VRP option  bits, other than server origi-
**              nator bit. This bit that is set in Link layer will be cleared
**              in link layer's completion routine only.
**
**  @param      pVRP -- VRP
**
**  @return     None
**
**  @attention
**              Assumption - VRP is non-null pointer.
******************************************************************************
**/

void GR_ResetVRPILT(ILT *pILT, VRP *pVRP,VDD *pVDD)
{
#if defined(MODEL_7000) || defined(MODEL_4700)
    UINT8 mask = (1 << VRP_RETRY_ONCE);

    if (GR_IS_GEORAID_VDISK(pVDD))
    {
        mask |= (1 << VRP_SERVER_ORIGINATOR);
        if (BIT_TEST(pVRP->options,VRP_SERVER_ORIGINATOR))
        {
            (pILT-1)->misc = (UINT32)pVDD;
        }
    }

    pVRP->options &= mask;

#else  /* MODEL_7000 || MODEL_4700 */
    //TODO fix like above code
    if (BIT_TEST(pVRP->options,VRP_SERVER_ORIGINATOR) == TRUE &&
        GR_IS_GEORAID_VDISK(pVDD))
    {
        /*
        ** Identify this VRP as originated from Server  and the VDD
        ** is the GeoRAID source. Store the VDD in server ILT at
        ** Link layer level.
        */
        pVRP->options = 0;
        BIT_SET(pVRP->options,VRP_SERVER_ORIGINATOR);
        (pILT-1)->misc = (UINT32)pVDD;
    }
    else
    {
        pVRP->options = 0;
    }

#endif /* MODEL_7000 || MODEL_4700 */
}

/**
******************************************************************************
**
**  @brief      This function  called from virtual layer executive process
**              for each of the VRP that is posted to virtual layer.
**
**              This first checks the operting state, and if it is either in
**              operating state or degraded state, IO will continue. If the
**              operating state is failed/inop , this submits the vdisk to the
**              vdisk error queue for auto swap handling, if the vdisk is
**              canddiate VDD on which autoswap is handled.
**
**  @param      pVDD -- VDD
**              pVRP -- VRP
**
**  @return     EC_OK        --> Status OK -- go ahead with I/O
**              EC_INOP_VDEV --> Error (Reject I/O)
**
**  @attention
**              Assumption - VDD and VRP are non-null pointers.
******************************************************************************
**/
UINT32 GR_VerifyVdiskOpState(VDD *pVDD, VRP *pVRP)
{
    UINT8 vdOpState;

    if (pVDD->status == SS_IOSUSPEND)
    {
       /*
       ** This code is added for snapshot purposes, during the controller failovers
       ** esp in unfail conditions, it is necessary to move the IO into suspended state
       ** to transient amount of time till the slave takes ownership of its own snappool
       ** so that server can retry the IO. This code is common for GeoRAID vdisks and
       ** normal vdisks.
       */
       return EC_RETRY;
    }

    if (! GR_IS_GEORAID_VDISK (pVDD))
    {
        /* It is not geo raid vdisk. Follow original logic */
        if(pVDD->status >= VD_OP)
        {
            return EC_OK;
        }
        return EC_INOP_VDEV;
    }

    vdOpState = pVDD->grInfo.vdOpState;

    switch (vdOpState)
    {
        case GR_VD_IOSUSPEND:
             return EC_RETRY;

        case GR_VD_OP:
        case GR_VD_DEGRADED:
             /*
              * Don't allow any I/O as long as auto- swap/swapback
              * operations are in progress.
              */
             if (GR_IsAutoSwapInProgress(pVDD))
             {
                 return EC_RETRY;
             }
             return EC_OK;

        case GR_VD_INOP:
#if GR_MYDEBUG1
             fprintf(stderr,"<GR_VerifyVdiskOpState> VID = %d  INOP state\n", (int)pVDD->vid);
#endif
             if (FALSE == GR_IsAutoSwapInProgress(pVDD))
             {
                 if (GR_IsCandidateVdisk(pVDD, pVRP) == TRUE)
                 {
#if GR_MYDEBUG1
       fprintf(stderr, "<GR_VerifyVdiskOpState>submitting error to vd error queue\n\n");
#endif
                     GR_SubmitVdiskError(pVDD, pVRP);
                     return EC_RETRY;
                 }
                return EC_INOP_VDEV;
             }
#if GR_MYDEBUG1
             fprintf(stderr,
             "<GR_VerifyVdiskOpState>VID = %x  is in INOP..Swap in progress please retry\n",pVDD->vid);
#endif
             return EC_RETRY;

        default:
            break;
    }
    return EC_INOP_VDEV;
}
/**
******************************************************************************
**
**  @brief      This function decides whetehr VDD is candidate for handling
**              AutoSwap process.
**
**              The vdisk considered for auto swap is selected based on the
**              following creteria.
**
**              >> The VRP should be originated from server/non-vlink type
**              >> The VDD is of Geo-RAID type.
**              >> The VDD has already been once auto-swapped -- Second auto-
**                 swap is not allowed, till its swapback is done.   I.e. if
**                 it again fails after auto-swap, currently we don't do any
**                  thing.  This is as per current design.
**              >> The VDD must have mirror partners.
**              >> The I/O request is only READ or WRITE
**
**              This function  is called from  virtual layer executive process
**              context, when VDD is found with  error before I/O is submitted,
**              as well as from  virtual layer completer's (vm$comp) context
**              when the error is found at I/O completion path.
**
**  @param      pVDD -- VDD
**              pVRP -- VRP
**
**  @return     TRUE -- Eligible for auto-swap
**              FALSE-- Not eligible for auto-swap
**
**  @attention
**              Assumption - VDD and VRP are non-null pointers.
******************************************************************************
**/
UINT32 GR_IsCandidateVdisk(VDD *pVDD, VRP *pVRP)
{
    UINT8  retVal     = FALSE;
    UINT8  vrpOptions = pVRP->options;
    UINT16 vrpFunc    = pVRP->function;
#if GR_MYDEBUG1
    int    i = 0;
#endif

    if(vrpFunc >= RRP_BASE)
    {
        vrpFunc = vrpFunc-RRP_BASE;
    }

    BIT_CLEAR(vrpFunc, VRP_SPECIAL);

#if GR_MYDEBUG1
    if (
          ( ++i && (pVDD != NULL ))                                        &&
          ( ++i && (! GR_IS_VD_ALREADY_ASWAPPED(pVDD)) )                   &&
          ( ++i && ( GR_IS_GEORAID_VDISK (pVDD)) )                         &&
          ( ++i && (BIT_TEST(vrpOptions,VRP_SERVER_ORIGINATOR) == TRUE) )  &&
          /* (BIT_TEST(pVDD->attr,VD_BSCD) == TRUE) && */
          ( ++i &&  ( (VRP_INPUT == vrpFunc) || (VRP_OUTPUT == vrpFunc)) ) &&
          ( ++i &&  (TRUE == GR_AnyMirrorsExist(pVDD)) )
       )
    {
        if (BIT_TEST(pVDD->attr,VD_BSCD) == TRUE && pVDD->grInfo.vdOpState != GR_VD_INOP)
        {
            fprintf(stderr, "<GR_IsCandidate> VID = %d  func=%d opstate=%d -- check success = %d\n",
                    (int)pVDD->vid, (int)vrpFunc, pVDD->grInfo.vdOpState, i);
        }
        retVal = TRUE;
    }
    else
    {
        if (BIT_TEST(pVDD->attr,VD_BSCD) == TRUE)
        {
            fprintf(stderr, "<GR_IsCandidate> VID = %d  func=%d opstate=%d -- failed at check  = %d\n",
                    (int)pVDD->vid, (int)vrpFunc, pVDD->grInfo.vdOpState, i);
        }
    }
#else
    if (
          ( pVDD != NULL )                    &&
          (! GR_IS_VD_ALREADY_ASWAPPED(pVDD)) &&
          (  GR_IS_GEORAID_VDISK (pVDD) )  &&
          (  BIT_TEST(vrpOptions,VRP_SERVER_ORIGINATOR) == TRUE) &&
          /* (BIT_TEST(pVDD->attr,VD_BSCD) == TRUE) && */
          (  (VRP_INPUT == vrpFunc) || (VRP_OUTPUT == vrpFunc) ) &&
          (  TRUE == GR_AnyMirrorsExist(pVDD))
       )
    {
        retVal = TRUE;
    }
#endif

    return (retVal);
}

/**
******************************************************************************
**
**  @brief      This function updates operating state of the specified vdisk.
**
**              This gets called from RB_SetVirtStat() which is called during
**              any raid i/o error handling  or when a pdisk is removed or
**              inserted. This updates the vdopstate based on the specified
**              option.
**
**              This function is also called when manual swap or autoswap of
**              raids occurred for a vdisk, as the vdisk operating state is
**              computed based on the status of its underlying raids.
**
**              This function first obtains the vdisk status (alogirthm as
**              is given in RB_SetVirtStat) based on the option provided.
**
**              If the option is 1
**
**                >> means raid statuses are already from PSD statuses and the
**                >> vdisk status is needed to be obtained from raid status.
**
**              If the option is 2
**
**                >> It first obtains the raid statues based on current PSD
**                   statuses and then get vdisk status from raid status.
**
**              If the option is 3
**
**                >> It first updates PSD statuses from PDD states and then
**                >> updates raid statuses and then vd status.
**
**              If the option is 0
**
**                >> Take vdstatus as specified in the argument 3 -- this is
**                >> used vdisk at creation/expansion from definebe.as,def_con.as
**
**              Finally it converts the vdisk status value to vdisk operating
**              state value and updates the same in the structures.
**
**  @param      pVDD   -- VDD
**              option --   0 --> Set to specified vdstate in the 3rd argument.
**                          1 --> Based on current raid status
**                          2 --> Based on current PSD status
**                          3 --> Based on current PDD status
**              vdstatus-- this is meaningful if option specified is  zero.
**
**  @return
**
**  @attention
**              Caller has to ensure that VDD is not null.
**              Caller is sure to issue NVram update after making this call.
***************************************************************************
**/
void GR_UpdateVdiskOpState(VDD *pVDD, UINT8 option, UINT8 vdStatus)
{
    /* Get the vdisk status. */
    if (option != GR_VDOPSTATE_GIVEN)
    {
        vdStatus = GR_GetVdiskStatus(pVDD, option);
    }

    /* Update the operating state of the vdisk. */

    /*
     * If by any chance, IOSUSPEND is never cleared and failed drive bays are UP
     * it will never go to OP state, instead always stay in IOSUSPEND state
     * TO do some thing to solve this issue.
     */
    if (GR_IsInIOSuspendState(pVDD) == FALSE)
    {
        pVDD->grInfo.vdOpState = GR_VdiskStatusToOpState (vdStatus);
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#if GR_MYDEBUG1
        fprintf(stderr,"<GR_UpdateVdiskOpstate> VID=%d  opState = %s\n",
        (int)(pVDD->vid), vdOpStateString[pVDD->grInfo.vdOpState] );
#endif

        if (O_P2Init == TRUE && pVDD->grInfo.vdOpState == GR_VD_OP)
        {
            GR_InformVdiskOpState(pVDD);
        }
    }
}

/**
******************************************************************************
**
**  @brief      This function returns the eoperating state of the specified
**              vdisk, based on the specified option.
**
**  @param      pVDD -- VDD
**              option --  0 --> Current value
**                         1 --> Based on current raid stauts
**                         2 --> Based on current PSD status
**                         3 --> Based on current PDD status
**
**  @return     Vdisk operating state
**
**  @attention
**              Caller has to ensure that VDD is not null.
******************************************************************************
**/
UINT8 GR_GetVdiskOpState (VDD *pVDD, UINT8 option)
{
    UINT8 vdOpState;
    UINT8 vdStatus;

    switch (option)
    {
        case GR_VDOPSTATE_CURRENT:

                 vdOpState = pVDD->grInfo.vdOpState;
                 break;

        case GR_VDOPSTATE_BY_RDDSTAT :
        case GR_VDOPSTATE_BY_PSDSTAT :
        case GR_VDOPSTATE_BY_PDDSTAT :

                 vdStatus = GR_GetVdiskStatus(pVDD, option);
                 vdOpState = GR_VdiskStatusToOpState(vdStatus);
                 break;
       default : /* Never comes here */
                 vdOpState = GR_VD_INOP;
    }
    return(vdOpState);
}

/**
******************************************************************************
**
**  @brief      This function gets the vdisk status based on the current
**              raid and/or  psd statuses.
**
**              If the option specified is 1, it takes current raid status
**              and computes the vdisk status.
**              If the option specified is 2, it computes the raid status based
**              on its current PSD status and computes the vdisk status.
**
**  @param      pVDD -- VDD
**              option --   1 --> Based current raid status
**                          2 --> Based on current PSD status
**                          3 --> Based on current PDD status
**
**  @return     Vdisk status
**
**  @attention
**              Caller has to ensure that VDD is not null.
******************************************************************************
**/
UINT8  GR_GetVdiskStatus(VDD *pVDD, UINT8 option)
{
    UINT8 raidStatus;
    UINT8 vdStatus = VD_OP;
    RDD  *pRDD;

    pRDD = pVDD->pRDD;

    /* For all the raid devices in this vdisk */
    while (pRDD)
    {
        switch(option)
        {
            case GR_VDOPSTATE_BY_PDDSTAT :
                /*
                ** Update the status of all PSDs based on their
                ** Corresponding PDD statuses. This call in turn
                ** updates the raid statuses.
                */
                 RB_setpsdstat();

            case GR_VDOPSTATE_BY_RDDSTAT :
                /* Consider existing raid status */
                raidStatus = pRDD->status;
                break;

            case GR_VDOPSTATE_BY_PSDSTAT :
                /*
                ** Get the status of this raid, by looking at its current
                ** PSD Statuses.
                */
                raidStatus = rb$getraiderrorstat(pRDD);
                break;

           default:
                raidStatus = RD_INOP;
                break;
        }

        switch(raidStatus)
        {
            case RD_INOP:
                 /* Any INOP raid in a vdisk makes the entire vdisk inoperable */
                 vdStatus = VD_INOP;
                 break;

            case RD_UNINIT:
                 /* INOP state overrides any other state that another raid is in */
                 if( (vdStatus != VD_INOP) && (vdStatus != VD_UNINIT))
                 {
                     vdStatus = VD_UNINIT;
                 }
                 break;

            case RD_INIT:
                 if((vdStatus != VD_INOP) &&
                    (vdStatus != RD_UNINIT) && (vdStatus != VD_INIT))
                 {
                     vdStatus = VD_INIT;
                 }
                 break;

            case RD_DEGRADED:
                 if(vdStatus == VD_OP)
                 {
                    vdStatus = VD_DEGRADED;
                 }
                 break;

            case RD_OP:
                 /* Just maintain the state computed till previous raid */
                 break;

            default:
                vdStatus = VD_INOP;
                break;
        }

        /* Go with the next raid in this vdisk. */
        pRDD = pRDD->pNRDD;
    }
    return (vdStatus);
}

/**
******************************************************************************
**
**  @brief      This function translates vdisk status to vdisk operating
**              state value used by geo-Raid related modules.
**
**  @return     Vdisk status
*****************************************************************************
**/

static UINT8 GR_VdiskStatusToOpState(UINT8 vdStatus)
{
    switch (vdStatus)
    {
        case VD_UNINIT:
            return GR_VD_UNINIT;

        case VD_INIT:
            return GR_VD_INIT;

        case VD_INOP:
            return GR_VD_INOP;

        case VD_OP:
            return GR_VD_OP;

        case VD_DEGRADED:
            return GR_VD_DEGRADED;

        default:
            return GR_VD_INOP;
    }
}


/*
******************************************************************************
**
**  @brief    This function sets  the vdopstate of the  specified source vdisk
**            to IOSUSPEND. The  failing DCN sends the  list of all the source
**            and  destination vdisks (in sync) owned by it to  the  surviving
**            controller.
**
**            The surviving  DCN sets  the opstate of these  source vdisks to
**            suspend state, so as not to allow any I/O on these vdisks during
**            the special processing of these copies by the cpy/manager during
**            ownership takeover mechanism.
**
**  @param    pSrcVDD -- Pointer to source vdisk.
**
**  @return   None
******************************************************************************
*/
void GR_SetIOSuspendState(VDD *pSrcVDD)
{
     if (pSrcVDD)
     {
         pSrcVDD->grInfo.vdOpState = GR_VD_IOSUSPEND;
         BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
     }
}

/*
******************************************************************************
**
******************************************************************************
*/

UINT32 GR_IsInIOSuspendState(VDD *pSrcVDD)
{
    if (pSrcVDD->grInfo.vdOpState == GR_VD_IOSUSPEND)
    {
        return TRUE;
    }
    return FALSE;
}

/*
******************************************************************************
**
******************************************************************************
*/

void GR_ResetIOSuspendState(VDD *pSrcVDD)
{
    /*
     *  Let us not clear the IO suspend state till all the mirror partners
     *  are processed), means we don't clear this untill allDevMissSyncFlag
     *  bit is cleared on all its destinations.
     *  This is really needed when a vdisk has more than one mirror
     */
    if (pSrcVDD && GR_IsInIOSuspendState(pSrcVDD))
    {
        pSrcVDD->grInfo.vdOpState = 0;
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        GR_UpdateVdiskOpState(pSrcVDD, 1,0);
#if GR_GEORAID15_DEBUG
        fprintf(stderr,"<GR>ResetIOSuspendState>VID=%x  opState = %s\n",
              (int)(pSrcVDD->vid), vdOpStateString[pSrcVDD->grInfo.vdOpState] );
#endif
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
