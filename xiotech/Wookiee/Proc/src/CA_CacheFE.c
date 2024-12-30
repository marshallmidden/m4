/* $Id: CA_CacheFE.c 159870 2012-09-20 12:59:51Z marshall_midden $ */
/**
*******************************************************************************
**
** @file: CA_CacheFE.c
**
** @brief:
**       To provide a means handling of cache module related functions.
**
**       Currently it covers the functionality that is required for the Mirror
**       Partner Information.
**
**       Replaces the following functions in cachefe.as
**
**       (1)  c$setmirrorpartner
**
**       Following three functions collectively are equivalent to
**       'C$setMirrorPartner',inorder to incorporate the new mirror partner
**       info, the function is split into 3 parts.
**
**       1.UINT32 CA_AcceptMPChange       (UINT32 serialNo)
**       2.void CA_SetMirrorPartnerFE_1   (UINT32 newMPSerialNo)
**       3.UINT32 CA_SetMirrorPartnerFE_2 (void)
**
**       These functions gets called from MP_Proc.c during MP handshaking.
**
**       Very Important: Whenever asm code is changed,this code has to be updated
**
**  Copyright (c) 1996-2010 Xiotech Corporation. All rights reserved.
**
*******************************************************************************
**/
#include "CA_CacheFE.h"

#include "cache.h"
#include "CA_CI.h"
#include "vcd.h"
#include "ecodes.h"             /* Error Code definitions           */
#include "ficb.h"
#include "ilt.h"
#include "kernel.h"
#include "MR_Defs.h"
#include "options.h"
#include "pcb.h"
#include "pm.h"
#include "QU_Library.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "wcache.h"
#include "CT_defines.h"
#include "mem_pool.h"
#include "system.h"

#include <stdio.h>
#include <string.h>

extern void C_que(ILT * pILT);
extern void CT_LC_CA_SplitComp(UINT32, void *);
extern void CT_LC_CA_ReadVRPComp(UINT32, void *);
extern void KernelDispatch(UINT32 returnCode, ILT * pILT, void *pPtr, UINT32 w0);
extern UINT32 CA_QueryMirrorPartnerChange(UINT32 serialNo);

extern void C_Enable(UINT32 VID);       /* Function definition in cachefe.as*/

extern void WC_SetGlobalDisable(void);  /* Function definition - wcache.as */

/*
******************************************************************************
** Private defines
******************************************************************************
*/

#define OPS_DISPLAY 0

/* Define it to 4096 to be able to support 16MB block IO. */
#define SPLIT_SIZE  4096

/*
******************************************************************************
** Private variables
******************************************************************************
*/

/* Number of Users that have a Temporary Write Cache Disable outstanding. */
static UINT32 gUsersTDisableReq = 0;

/* Array of user counts with a Temproary WC Disable Request. */
static UINT8 gUsersTDisableCnt[MAX_TEMP_DISABLE_USERS] = { 0 };

/*
****************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
extern CA   C_ca;                   /* The declaration of this is in iramfe.inc */
extern UINT32 C_mirror_error_flag;  /* The declaration of this is in cachefe.as */
extern PCB *C_error_pcb;            /* The declaration of this is in cachefe.as */
extern struct RB nil;

extern QU   CA_OpRetryQue;

extern UINT32 C_orc;
extern UINT32 C_ctv;
extern UINT32 C_vcd_wait_active;
extern UINT32 C_vcd_wait_head;
extern UINT32 C_vcd_wait_tail;
extern PCB *C_vcd_wait_pcb;

extern UINT32 C_flush_orc;

/*
********************************************************************************
** Public variables not in any header files
********************************************************************************
*/
extern QU   gWCMarkCacheQueue;      /* Mark Cache Queue */

/*
********************************************************************************
** Public function prototypes not in header files
********************************************************************************
*/
extern void CA_Que(ILT * pILT);
extern void C_ncrComp1(UINT32 status, ILT * pILT);
extern void CA_SplitComp(UINT32 status, ILT * sILT);
extern void CA_ReadVRPComp(UINT32 status, ILT * sILT);
extern UINT32 CA_GetReadComp(void);

#if defined(MODEL_7000) || defined(MODEL_4700)
extern void CT_LC_CA_DeferredBusyTask(void);
extern void CA_DeferredBusyTask(void);
extern void CA_Check4PAB(VCD * pVCD, ILT * pILT);
#endif /* MODEL_7000 || MODEL_4700 */

/*
********************************************************************************
** Code Start
********************************************************************************
*/

/**
******************************************************************************
**
**  @brief      To checkg whether the FE can accept mirror partner change when
**              the SET MPCONFIG MRP is received from CCB.
**
**  @param      Mirror partner's serial number.
**
**  @return     status
**
******************************************************************************
**/
UINT32 CA_AcceptMPChange(UINT32 serialNo)
{
    UINT32 status;

    /* Determine if the mirror partner can change. */
    status = CA_QueryMirrorPartnerChange(serialNo);

    if (BIT_TEST(status, MRQMPCNOSTOP) || BIT_TEST(status, MRQMPCIOOUTS))
    {
        /* No c$stop outstanding, return Outstanding Operations Error. */
        return (DEOUTOPS);
    }
    else if (BIT_TEST(status, MRQMPCCACHEDATA))
    {
        /* No outstanding IO, but there is cached data. Return Invalid Controller Error. */
        return (DEINVCTRL);
    }
    else if (BIT_TEST(status, MRQMPCNOCOMM))
    {
        return (DENOCOMM);
    }
    return (status);
}

/**
******************************************************************************
**
**  @brief      To setg the mirror partner serial number in the global FICB
**              structure and cache status (in CA structure)when the
**              SET MPCONFIG MRP is received from CCB.
**
**  @param      New mirror partner's serial number.
**
**  @return     none
**
******************************************************************************
**/
void CA_SetMirrorPartnerFE_1(UINT32 newMPSerialNo)
{
    UINT32      oldMPSerialNo;
    ILT        *pILT;

    /* Save the old mirror partner's serial number from FICB struct. */
    oldMPSerialNo = K_ficb->mirrorPartner;

    /*
     * If the new mirror partner serial number is not same as the old mirror
     * partner's serial num, update FICB. If the new mirror partner is self,
     * update cache status (Clear the N-way mirror bit).
     */
    if (oldMPSerialNo != newMPSerialNo)
    {
        if (K_ficb->cSerial == newMPSerialNo)
        {
            BIT_CLEAR(C_ca.status, CA_NWAYMIRROR);
        }
        else
        {
            BIT_SET(C_ca.status, CA_NWAYMIRROR);
        }

        /* Update FICB structure with new serial number. */
        K_ficb->mirrorPartner = newMPSerialNo;

        /* Refresh the Write Cache Signature with the updated value. */
        pILT = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
        pILT->fthd = NULL;
        pILT->bthd = NULL;
        pILT->ilt_normal.w0 = SIG_REFRESH;  /* Set Refresh WC Signature     */
        pILT->misc = (UINT32)K_xpcb;        /* Set PCB (for wakeup)         */

        /* Enqueue the ILT to the WC Mark executor. */
        QU_EnqueReqILT(pILT, &gWCMarkCacheQueue);

        /* Set process to wait for signal and Exchange process. */
        TaskSetMyState(PCB_WAIT_IO);
        TaskSwitch();

        /* Check completion status. */
        if (pILT->ilt_normal.w1 != EC_OK)
        {
            fprintf(stderr, "%s: Bad return value from signature refresh=%d\n",
                    __func__, pILT->ilt_normal.w1);
        }
        else
        {
            fprintf(stderr, "%s: Signature refresh successful!\n", __func__);
        }

        /* Release the ILT. */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
        put_ilt(pILT);
    }
}

/**
******************************************************************************
**
**  @brief      To update cache status and Cache error PCB state when the
**              SET MPCONFIG MRP is received from CCB.
**
**  @param      none
**
**  @return     status
**
******************************************************************************
**/
UINT32 CA_SetMirrorPartnerFE_2(void)
{
    UINT32      status;

    /*
     * Clear the mirror broken bit, because the new mirror partner is up and
     * has sent it's Serial num.
     */
    if (BIT_TEST(C_ca.status, CA_MIRRORBROKEN))
    {
        BIT_CLEAR(C_ca.status, CA_MIRRORBROKEN);
    }

    /* Clear the Enable pending bit and Enable Global cache if possible. */
    if (BIT_TEST(C_ca.status, CA_ENA_PEND))
    {
        BIT_CLEAR(C_ca.status, CA_ENA_PEND);

        /* Enable cache if possible. */
        C_Enable(GLOBAL_CACHE_ENABLE);
    }

    /*
     * Show that a mirror partner could get Mirror Failures that need to be
     * reported to CCB/User.
     */
    C_mirror_error_flag = FALSE;

    /* Set return status to GOOD. */
    status = DEOK;

    /*
     * If error task is existing and its current process status is timed
     * wait, make it ready, to end itself.
     */
    if ((C_error_pcb != NULL) && (TaskGetState(C_error_pcb) == PCB_TIMED_WAIT))
    {
#ifdef HISTORY_KEEP
CT_history_pcb("CA_SetMirrorPartnerFE_2 setting ready pcb", (UINT32)C_error_pcb);
#endif
        TaskSetState(C_error_pcb, PCB_READY);
    }

    return status;
}


/**
******************************************************************************
**
**  @brief      CA_SetTempDisableWC - Set WC to Temporary Disable State
**
**              This function will keep track of the user requests and will
**              only set the WC into a Temporary Disable State and kick off a
**              Flush and Invalidate of the Write Cache if this is the first
**              caller of the functon. Then it will return to the caller.
**
**  @param      pMRP    - MRP Packet which has a pointer to the response
**
**  @return     Status of the Set Temp Disable - always good
**
**  @attention  It is the callers responsibility to determine when the Flush
**              and Invalidate has completed.
**
******************************************************************************
**/
UINT8 CA_SetTempDisableWC(MR_PKT * pMRP)
{
    MRSETTDISCACHE_REQ *pMRPReq;        /* Request Information              */
    MRSETTDISCACHE_RSP *pMRPRsp;        /* Response to send back            */
    UINT32      prevNumUsers;   /* Number of Users wanting To Set   */

    /* Get pointer to Request and Response Packet and set up the return length. */
    pMRPReq = (MRSETTDISCACHE_REQ *) pMRP->pReq;
    pMRPRsp = (MRSETTDISCACHE_RSP *) pMRP->pRsp;
    pMRPRsp->header.len = sizeof(MRSETTDISCACHE_RSP);

    /*
     * Get the number of Users that already have a Temporary Disable in progress
     * outstanding for use later.
     */
    prevNumUsers = gUsersTDisableReq;

    /*
     * If this is the first request for this user, bump the total number of
     * requesters. Then bump the count for this user.
     */
    if (gUsersTDisableCnt[pMRPReq->user] == 0)
    {
        gUsersTDisableReq++;
    }
    gUsersTDisableCnt[pMRPReq->user]++;

    /*
     * If this is the first requester to temporarily disable Write Cache, then
     * set the Flag to show in the Temporary Disabled State and begin the
     * Flush and Invalidate Process, if needed. Then Return.
     */
    if (prevNumUsers == 0)
    {
        BIT_SET(C_ca.status2, CA_TEMP_DISABLE);
        if ((BIT_TEST(C_ca.status, CA_ENA)) && (!BIT_TEST(C_ca.status, CA_DIS_IP)))
        {
            /*
             * Write Cache is Enabled and not already Doing a Disable in
             * Progress. Put the Write Cache into a Global Disabled state and
             * then set Enable Pending.
             */
            WC_SetGlobalDisable();
            BIT_SET(C_ca.status, CA_ENA_PEND);
        }
    }

    return DEOK;
}


/**
******************************************************************************
**
**  @brief      CA_ClearTempDisableWC - Clear the Temporary Disabled State in WC
**
**              This function will keep track of the user requests and once all
**              users have cleared their requests, the WC Temporary Disabled
**              State will be cleared which will allow the Write Cache to
**              return to its former state.
**
**  @param      pMRP    - MRP Packet which has a pointer to the response
**
**  @return     Status of the Clear Temp Disable - good or invalid option
**
**  @attention  None
**
******************************************************************************
**/
UINT8 CA_ClearTempDisableWC(MR_PKT * pMRP)
{
    MRCLRTDISCACHE_REQ *pMRPReq;        /* Request Information              */
    MRCLRTDISCACHE_RSP *pMRPRsp;        /* Response to send back            */

    /* Get pointer to Request and Response Packet and set up the return length. */
    pMRPReq = (MRCLRTDISCACHE_REQ *) pMRP->pReq;
    pMRPRsp = (MRCLRTDISCACHE_RSP *) pMRP->pRsp;
    pMRPRsp->header.len = sizeof(MRSETTDISCACHE_RSP);

    /*
     * If there are no outstanding users that have a Set, or this users count
     * is already zero, then return Good.
     * Else determine if we can clear the Temporary Disable yet.
     */
    if ((gUsersTDisableReq != 0) && (gUsersTDisableCnt[pMRPReq->user] != 0))
    {
        /*
         * If the user is requesting to remove all Sets, clear the count, Else
         * decrement the users count (do not go negative). If the result is
         * zero also decrement the Number of users requesting a Set.
         */
        if (pMRPReq->option == T_DISABLE_CLEAR_ALL)
        {
            /* Clear the count for the user. */
            gUsersTDisableCnt[pMRPReq->user] = 0;
        }
        else if (pMRPReq->option == T_DISABLE_CLEAR_ONE)
        {
            /* Decrement the count for the user. */
            gUsersTDisableCnt[pMRPReq->user]--;
        }
        else
        {
            /* An invalid option was passed in, reject the request. */
            return DEINVOPT;
        }

        if (gUsersTDisableCnt[pMRPReq->user] == 0)
        {
            gUsersTDisableReq--;
        }

        /*
         * Once all the users have cleared the Sets, clear the Temporary Disable
         * Flag and if Global Cache Enable is pending, then try and complete the
         * Enable process
         */
        if (gUsersTDisableReq == 0)
        {
            BIT_CLEAR(C_ca.status2, CA_TEMP_DISABLE);
            if ((C_ca.status2 == 0) && (BIT_TEST(C_ca.status, CA_ENA_PEND)))
            {
                /* Clear the Enable Pending and do a Global Cache Enable. */
                BIT_CLEAR(C_ca.status, CA_ENA_PEND);
                C_Enable(GLOBAL_CACHE_ENABLE);
            }
        }
    }
    return DEOK;
}


/**
******************************************************************************
**
**  @brief      CA_QueryFlushDone - Query if the Write Cache Flush is Done
**
**              This function queries and responds to the requester with the
**              status of the Write Cache Flush.
**
**  @param      pMRP    - MRP Packet which has a pointer to the response
**
**  @return     Status of the WC Flush Done - Done, VDisk in Error State, or
**                  In Progress.
**
******************************************************************************
**/
UINT8 CA_QueryTDisableDone(MR_PKT * pMRP)
{
    MRQTDISABLEDONE_RSP *pMRPRsp;   /* Response to send back            */
    UINT32      i;                  /* VCD Index                        */

    /* Get pointer to Response Packet and set up the return length. */
    pMRPRsp = (MRQTDISABLEDONE_RSP *) pMRP->pRsp;
    pMRPRsp->header.len = sizeof(MRQTDISABLEDONE_RSP);

    /*
     * Determine if the Write Cache is flushed.
     *
     * If all the tags are free, then the Cache is Flushed.
     */
    if (C_ca.tagsFree == C_ca.numTags)
    {
        /* Tags are all free - Done. */
        pMRPRsp->data.status = MQTD_DONE;
    }
    else
    {
        /*
         * Not all free yet.
         *
         * If there are any VCDs with a Cache tree and not in in an error
         * state then the Flush is still in progress. Else there is at least
         * one VDisk in Error State with data that cannot be flushed.
         */
        pMRPRsp->data.status = MQTD_VDISK_IN_ERROR;     /* Preset to VDisk in Err */

        for (i = 0; i < MAX_VIRTUAL_DISKS; i++)
        {
            if ((vcdIndex[i] != NULL) &&
                (vcdIndex[i]->pCache != NULL) &&
                (!(BIT_TEST(vcdIndex[i]->stat, VC_ERROR))))
            {
                /*
                 * VCD has Data in Cache and is not in Error State. Reset
                 * the response to In Progress and exit
                 */
                pMRPRsp->data.status = MQTD_IN_PROGRESS;
                break;
            }
        }
    }
    return (DEOK);
}


/**
******************************************************************************
**
**  @brief  CA_CheckOpsOutstanding -- Count I/O operations in BE (or link layer).
**
**  Creates a count of the operations in the vdisk throttle queues, or in the
**  RB trees but blocked due to overlapping a previous write.
**
**  The difference between C_orc and this count, is the number of I/Os that
**  are in the BE that we must wait to return from the BE.
**
**  NOTE: the intention is that you are going to delete a vdisk that has
**  I/O outstanding, and you must get that out of the BE before the ILT/VRPs
**  are suddenly pointing at a non-existent raid/vdisk.
**
**  @param      none
**
**  @return     (C_orc - count) -- the number of I/Os left to finish.
**
******************************************************************************
**/
UINT32 CA_CheckOpsOutstanding(void)
{
    UINT32      i;
    UINT32      count = 0;
    struct RB  *pRB;
    struct RB  *pRBPrev;
    struct RB  *pRBFthd;
    ILT        *pILT;

    UINT32      vdisk_throttled = 0;
    UINT32      RB_blocked = 0;

    /* Loop through the VCD list. */
    for (i = 0; i < MAX_VIRTUAL_DISKS; i++)
    {
        /* Skip the VCD if it is not valid. */
        if (vcdIndex[i] == NULL)
        {
            continue;
        }

        /* Count the ILTs in this vdisk throttle queue. */
        for (pILT = vcdIndex[i]->pTHead; pILT != NULL; pILT = pILT->fthd)
        {
            vdisk_throttled++;
        }

        pRBPrev = &nil;

        /* Walk the RB Tree and look for operations that are blocked. */
        pRB = vcdIndex[i]->pIO;
        while (pRB != NULL)
        {
            /* Loop on all the duplicate IOs (fthd) and increment the count. */
            pRBFthd = pRB;
            while (pRBFthd != NULL)
            {
                pILT = (ILT *) pRBFthd->dPoint;
                if (pILT != 0 && pILT->ilt_normal.w1 > 0)
                {
                    RB_blocked++;
                }
                pRBFthd = pRBFthd->fthd;
            }

            /*
             * Make a quick check to see if this happens to be the only
             * node in the RB tree. The only time the bParent should be
             * "NULL" is at the root of the tree and this is just taking
             * a shortcut when both left and right children are also empty.
             */
            if (pRB->bParent == NULL && pRB->cLeft == &nil && pRB->cRight == &nil)
            {
                pRB = &nil;
                break;
            }

            /*
             * Walk the RB tree starting with the left children and then
             * keep proceeding through the rest of the tree.
             */
            if (pRB->cLeft != &nil)
            {
                pRB = pRB->cLeft;
                continue;
            }

            if (pRB->cRight != &nil)
            {
                pRB = pRB->cRight;
                continue;
            }

            pRBPrev = pRB;
            pRB = pRB->bParent;

            while (pRB != &nil && pRBPrev != &nil)
            {
                if (pRBPrev == pRB->cLeft)
                {
                    if (pRB->cRight != &nil)
                    {
                        pRBPrev = &nil;
                        pRB = pRB->cRight;
                        break;
                    }
                }
                if (pRBPrev == pRB->cLeft || pRBPrev == pRB->cRight)
                {
                    if (pRB->bParent == 0)
                    {
                        pRBPrev = &nil;
                        pRB = &nil;
                        break;
                    }
                    else
                    {
                        pRBPrev = pRB;
                        pRB = pRB->bParent;
                    }
                }
            }
        }
    }

    /* The number of operations we found that are not going to complete. */
    count = vdisk_throttled + RB_blocked;

    if (count > C_orc)
    {
        fprintf(stderr, "%s%s:%u %s -- PROBLEM: count=%d (vdisk_throttled=%d + RB_blocked=%d) > C_orc=%d  -- C_flush_orc=%d\n",
                FEBEMESSAGE, __FILE__, __LINE__, __func__, count, vdisk_throttled, RB_blocked, C_orc, C_flush_orc);
#ifndef PERF
        abort();
#endif  /* !PERF */
    }
#ifndef PERF
    else
    {
        fprintf(stderr, "%s%s:%u %s count=%d (vdisk_throttled=%d + RB_blocked=%d), C_orc=%d, C_flush_orc=%d\n",
                FEBEMESSAGE, __FILE__, __LINE__, __func__, count, vdisk_throttled, RB_blocked, C_orc, C_flush_orc);
    }
#endif  /* !PERF */

    return (UINT32)(C_orc - count);
}       /* End of CA_CheckOpsOutstanding */

/**
******************************************************************************
**
**  @brief      CA_Que
**
**              Stub function that will analyze the request and splits it
**              if required, into multiple smaller requests before actually
**              queuing to the cache layer
**
**  @param      ILT *pILT
**
**  @return     none
**
******************************************************************************
**/

void CA_Que(ILT * pILT)
{
    UINT8       i = 0;
    UINT8       count = 0;
    UINT32      last_len = 0;
    ILT        *sILT = NULL;
    VRP        *sVRP = NULL;
    VRP        *pVRP = (VRP *) ((pILT - 1)->ilt_normal.w4);

    /*
     * If the request is not a READ or a WRITE-related or if the size of the
     * READ/WRITE is less then or equal to the SPLIT_SIZE, follow the normal
     * process path.
     */
    if (((pVRP->function != VRP_INPUT)
         && ((pVRP->function & 0x03) != VRP_OUTPUT)) || (pVRP->length <= SPLIT_SIZE))
    {
        pVRP->gen0 = 0;
        C_que(pILT);
        return;
    }
    /*
     * OK, we have a large request to split. Calculate the number of
     * split ILTs required, and the length of the last split req.
     */
    last_len = (pVRP->length % SPLIT_SIZE);
    count = ((pVRP->length / SPLIT_SIZE) + ((last_len > 0) ? 1 : 0));

    /*
     * If the split count is greater than what we can accomodate, complete
     * the original request with error
     */
    if (count > MAX_SPLIT)
    {
        KernelDispatch(EC_INV_LEN, (pILT - 1), 0, 0);
        return;
    }

    /*
     * Create and setup the split ILTs & corresponding VRPs, save the split
     * ILTs in their appropriate slots in the original request - for tracking
     * purpose, mark all the split VRPs as incomplete and queue the split ILTs
     * to the cache layer.
     */
    memset((void *)(pILT + 1), 0, sizeof(ILT));
    for (i = 0; i < count; i++)
    {
        sILT = get_ilt();
        sVRP = get_vrp();
        memset((void *)sVRP, 0, sizeof(VRP));
        memset((void *)sILT, 0, 2 * sizeof(ILT));
        /*
         * Split ILT top level:
         * misc => primary ILT params pointer (ISP level-0)
         * w0   => split offset in segments
         * w1   => primary ILT at current level
         * w4   => split VRP (consistent with vrvrp)
         */
        sILT->cr = (void *)C_label_referenced_in_i960asm(CA_SplitComp);
        sILT->misc = (UINT32)(pILT - 1)->misc;
        sILT->ilt_normal.w0 = (SPLIT_SIZE * i);
        sILT->ilt_normal.w1 = (UINT32)pILT;
        sILT->ilt_normal.w4 = (UINT32)sVRP;

        sVRP->function = pVRP->function;
        sVRP->strategy = pVRP->strategy;
        sVRP->status = 0;
        sVRP->vid = pVRP->vid;
        sVRP->path = pVRP->path;
        sVRP->options = pVRP->options;
        sVRP->length = ((i + 1 == count) && (last_len > 0)) ? last_len : SPLIT_SIZE;
        sVRP->startDiskAddr = pVRP->startDiskAddr + sILT->ilt_normal.w0;
        sVRP->gen0 = (UINT32)pVRP;

        BIT_SET(sVRP->options, VRP_NOTCOMPLETED);
        (pILT + 1)->ilt_prim.split[i] = sILT;
        C_que(sILT + 1);
    }
}

/**
******************************************************************************
**
**  @brief      Split req callback function
**
**              Final completion callback function for the split req ILT
**
**  @param      UINT32 status
**              ILT *pILT
**
**  @return     none
**
******************************************************************************
**/
void CA_SplitComp(UINT32 status, ILT * sILT)
{
    UINT32      indx = sILT->ilt_normal.w0 / SPLIT_SIZE;
    ILT        *pILT = (ILT *) sILT->ilt_normal.w1;
    VRP        *pVRP = (VRP *) ((pILT - 1)->ilt_normal.w4);

    /* Update the status in the parent VRP. */
    if (pVRP->status == EC_OK)
    {
        pVRP->status = status;
    }
    /*
     * Clear the split ilt slot in the primary ILT and release the split
     * resources: ilt and vrp
     */
    (pILT + 1)->ilt_prim.split[indx] = NULL;
    put_ilt(sILT);
    put_vrp((VRP *) sILT->ilt_normal.w4);

    /*
     * If all the split reqs for this original request are accounted for,
     * complete the original req back to the mag layers - else, just return.
     */
    for (indx = 0; indx < MAX_SPLIT; indx++)
    {
        if ((pILT + 1)->ilt_prim.split[indx] != NULL)
        {
            return;
        }
    }
    KernelDispatch(pVRP->status, (pILT - 1), 0, 0);
}

/**
******************************************************************************
**
**  @brief      Split req READ VRP completion callback function
**
**              READ completion callback function called when the BE is done
**              processing the split read request. This function will take
**              care of re-ordering the READ data before returning it to
**              the host
**
**  @param      UINT32 status
**              ILT *pILT
**
**  @return     none
**
******************************************************************************
**/
void CA_ReadVRPComp(UINT32 status, ILT * sILT)
{
    UINT32      i = 0;
    ILT        *ilt = NULL;
    ILT        *pILT = (ILT *) (sILT - 2)->ilt_normal.w1;
    VRP        *sVRP = (VRP *) (sILT - 1)->ilt_normal.w4;

    /* Save the status in VRP and clear the NOTCOMPLETE option bit in the VRP. */
    BIT_CLEAR(sVRP->options, VRP_NOTCOMPLETED);
    sVRP->status = status;

    /*
     * Decrement the C_orc count since we may hold this req at this point
     * to prevent C$stop issues
     */
    C_orc -= 1;

    /*
     * Take care of the throttling logic that will take care of scheduling
     * the blocked requests in the wait queue
     */
#ifdef M4_DEBUG_C_ctv
CT_history_printf("%s%s:%u: C_ctv starts=%u ends=%u vc_vtv[%d]=%d\n", FEBEMESSAGE,__FILE__, __LINE__, C_ctv, C_ctv - sVRP->gen3,sVRP->vid, vcdIndex[sVRP->vid]->vtv - sVRP->gen3);
#endif  // M4_DEBUG_C_ctv
    C_ctv -= sVRP->gen3;
    vcdIndex[sVRP->vid]->vtv -= sVRP->gen3;
    sVRP->gen3 = 0;
    if ((C_vcd_wait_head != 0) && (TaskGetState(C_vcd_wait_pcb) == PCB_NOT_READY))
    {
#ifdef HISTORY_KEEP
CT_history_pcb("CA_ReadVRPComp setting ready pcb", (UINT32)C_vcd_wait_pcb);
#endif
        TaskSetState(C_vcd_wait_pcb, PCB_READY);
    }

    /* Check to see if all the split READ reqs have completed back from BE. */
    for (i = 0; i < MAX_SPLIT; i++)
    {
        if (((ilt = (pILT + 1)->ilt_prim.split[i]) != NULL)
            && BIT_TEST(((VRP *) ilt->ilt_normal.w4)->options, VRP_NOTCOMPLETED))
        {
            return;
        }
    }

    /*
     * OK, we have all the split reqs of the original READ req completed back
     * to us. Now send the READ data in order to the host
     */
    for (i = 0; i < MAX_SPLIT; i++)
    {
        if ((ilt = (pILT + 1)->ilt_prim.split[i]) != NULL)
        {
            /* Increment C_orc as we are pushing the req back to processing. */
            C_orc += 1;
            C_ncrComp1(((VRP *) ilt->ilt_normal.w4)->status, (ilt + 2));
        }
    }
}

/**
******************************************************************************
**
**  @brief      Stub function to get the pointer to READ completion cb
**
**  @param      none
**
**  @return     UINT32 - callback function pointer cast to UINT32
**
******************************************************************************
**/
UINT32 CA_GetReadComp(void)
{
    return (UINT32)(C_label_referenced_in_i960asm(CA_ReadVRPComp));
}


#if defined(MODEL_7000) || defined(MODEL_4700)  // ISE_BUSY
PCB        *pcbPAB = NULL;

/**
******************************************************************************
**
**  @brief      Task that takes care of deferring the BUSY response to
**              the hosts when the corresponding VDISK is in PAB state
**
**              Whenever an IO request is added to the Throttle list and
**              this task pcb is NULL, the task is created.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void CA_DeferredBusyTask(void)
{
    UINT32      hit = FALSE;
    VCD        *pVCD = NULL;
    VCD        *prevVCD = NULL;
    ILT        *pILT = NULL;
    ILT        *prevILT = NULL;

    /*
     * The main loop wakes up every second and walks thru all the VCDs and
     * their ILT lists and updates the timeout values. If any ILT times out,
     * it is completed back with BUSY. If the completed ILT is the last entry
     * in the VCD's wait list, the VCD is removed from the C_vcd_wait_head.
     * If there are no VCDs with VC_ISE_BUSY status set in the C_vcd_wait list,
     * the task exits.
     */
    while (1)
    {
        /*
         * Walk thru the VCD list in the wait queue and check for the VCDs
         * with VC_ISE_BUSY status set. If found, decrement the PAB timeout
         * for all the ILT tasks in its wait list. If the timeout reaches '0'
         * for any ILT, complete it to the upper layers with BUSY status.
         */
        hit = FALSE;
        prevVCD = NULL;
        pVCD = (VCD *) C_vcd_wait_head;
        while ((pVCD != NULL) && (C_vcd_wait_active == FALSE))
        {
            if (BIT_TEST(pVCD->stat, VC_ISE_BUSY))
            {
                /*
                 * Walk thru the throttle wait ILT list in the VCD and decrement
                 * the timeout value. If the ILT times out, complete it back
                 * to the mag layer with BUSY status
                 */
                prevILT = NULL;
                pILT = pVCD->pTHead;
                while ((pILT != NULL)
                       && (BIT_TEST(pVCD->stat, VC_ISE_BUSY))
                       && (C_vcd_wait_active == FALSE))
                {
                    if ((pILT->ilt_normal.w1 -= 1) == 0)
                    {
                        /*
                         * ILT timed out! Dequeue this and complete back with
                         * EC_BUSY response
                         */
                        if (prevILT == NULL)
                        {
                            pVCD->pTHead = pILT->fthd;
                            if (pVCD->pTHead == NULL)
                            {
                                pVCD->pTTail = NULL;
                            }
                            else
                            {
                                pILT->fthd->bthd = NULL;
                            }
                        }
                        else
                        {
                            prevILT->fthd = pILT->fthd;
                            if (prevILT->fthd == NULL)
                            {
                                pVCD->pTTail = prevILT;;
                            }
                            else
                            {
                                pILT->fthd->bthd = prevILT;
                            }
                        }
                        C_orc -= 1;
                        KernelDispatch(EC_BUSY, (pILT - 1), 0, 0);
                        pILT = (prevILT == NULL) ? pVCD->pTHead : prevILT->fthd;
                    }
                    else
                    {
                        prevILT = pILT;
                        pILT = pILT->fthd;
                    }
                }

                /*
                 * If the VCD's wait queue is empty, remove the VCD from the
                 * Throttle list
                 */
                if ((pVCD->pTHead == NULL) && (C_vcd_wait_active == FALSE))
                {
                    if (C_vcd_wait_head == (UINT32)pVCD)
                    {
                        C_vcd_wait_head = (UINT32)pVCD->pFwdWait;
                        if (C_vcd_wait_head == 0)
                        {
                            C_vcd_wait_tail = 0;
                        }
                        else
                        {
                            pVCD->pFwdWait->pBwdWait = NULL;
                        }
                    }
                    else
                    {
                        prevVCD->pFwdWait = pVCD->pFwdWait;
                        if (prevVCD->pFwdWait == NULL)
                        {
                            C_vcd_wait_tail = (UINT32)prevVCD;
                        }
                        else
                        {
                            prevVCD->pFwdWait->pBwdWait = prevVCD;
                        }
                    }
                    pVCD->pFwdWait = NULL;
                    pVCD->pBwdWait = NULL;
                    pVCD = (prevVCD == NULL) ? (VCD *) C_vcd_wait_head : prevVCD->pFwdWait;
                }
                else
                {
                    /*
                     * Set the 'hit' if there is atleast one VCD with VC_ISE_BUSY
                     * status set
                     */
                    hit = TRUE;
                    prevVCD = pVCD;
                    pVCD = pVCD->pFwdWait;
                }
            }
            else
            {
                prevVCD = pVCD;
                pVCD = pVCD->pFwdWait;
            }
        }
        if ((C_vcd_wait_head == 0) || ((hit == FALSE) && (C_vcd_wait_active == FALSE)))
        {
            /* There are no more VCDs throttled. Exit the task. */
            break;
        }

        /* Sleep for one sec. */
        TaskSleepMS(1000);
    }
    pcbPAB = NULL;
}

/**
******************************************************************************
**
**  @brief      Checks if the VCD state is BUSY and sets the timeout in the
**              ILT. This also spawns the CA_DeferredBusyTask if not present
**
**              According to inputs from Mike Walker (COS), the timeouts for
**              various OSs are:
**              Netware  - 60 secs
**              VMWare   - Don't know yet
**              Win2k8   - 60 sec
**              Win2k3R2 - 40 sec
**              Win2k3   - 60 sec
**              Linux    - 60 sec
**              Solaris  - 60 sec
**              MAC      - 45 sec
**              HPUX     - 30 sec
**              Based on the above facts, setting the BUSY timeout to 28 secs.
**
**  @param      VCD *
**              ILT *
**
**  @return     none
**
******************************************************************************
**/
#define DEFERRED_BUSY_TO        28

void CA_Check4PAB(VCD * pVCD, ILT * pILT)
{
    /*
     * If the VCD state is ISE_BUSY, set the timeout in the ILT and
     * create the Deferred busy task if it does not exist
     */
    if (BIT_TEST(pVCD->stat, VC_ISE_BUSY))
    {
        pILT->ilt_normal.w1 = DEFERRED_BUSY_TO;

        /* Check if CA_DeferredBusyTask in process of starting (out of memory problems). */
        while (pcbPAB == (PCB *) - 1)
        {
            TaskSleepMS(50);
        }
        if (pcbPAB == NULL)
        {
            CT_fork_tmp = (unsigned long)"CA_DeferredBusyTask";
            pcbPAB = (PCB *) - 1;
            pcbPAB = TaskCreate2(C_label_referenced_in_i960asm(CA_DeferredBusyTask), VCDTHROTTLE_PRIORITY);
        }
    }
}

#endif /* MODEL_7000 || MODEL_4700 */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
