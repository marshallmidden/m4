/* $Id: NVA_BE.c 145021 2010-08-03 14:16:38Z m4 $ */
/**
******************************************************************************
**
**  @file       NVA_BE.c
**
**  @brief      Functions to handle the RAID Non-Volatile Activity Records
**
**  Constants, Macros, Structures, Variables, and Functions used in processing
**  the RAID Non-Volatile Activity (NVA) Records in NVRAM.
**
**  Copyright (c) 2003-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "nvabe.h"

#include "defbe.h"
#include "def_lun.h"
#include "nvram.h"
#include "RL_RDD.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "ddr.h"
#include "misc.h"

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      From the NVA Record List, set the AStatus bit on in every RDD
**              that is owned by this controller.
**
**              The NVA Record List passed in will be searched for any valid
**              entries for this controller. Once a valid entry is found, the
**              RDD AStatus bit showing that a Stripe Resync is in progress is
**              set. The RDD R5 Stripe Resync Outstanding Request count also
**              will be incremented. If any RDDs are updated, then the NVRAM
**              configuration for the controller is updated.
**
**  @param      pNVA - pointer to the beginning NVA Record in the list
**              (excluding the header)
**  @param      numEntries - number of entries in the list to search
**
**  @return     none
**
******************************************************************************
**/
void NVA_SetReSyncAStatus(NVA *pNVA, UINT32 numEntries)
{
    UINT32     i = 0;          /* Loop counter                     */
    UINT32     rddChanged = FALSE; /* RDD Changed Flag             */
    RDD       *rdd;            /* Pointer to an RDD                */
    UINT8     *pRUFlag;        /* RAID Update Flag array pointer   */

    /*
    ** Allocate an array of byte flags in determining if this RAID has already
    ** been updated (only want to increment the counter once for each call to
    ** this routine).
    */
    pRUFlag = (UINT8*) s_MallocC(MAX_RAIDS, __FILE__, __LINE__);   /* Get memory for list  */

    /*
    ** Go through each entry looking for RAIDs that this controller owns.
    ** Once found, increment the outstanding request count (if not already done
    ** during this call) and set the bit of the AStatus fields of the RDD.
    */
    for (i = 0; i < numEntries; ++i, ++pNVA)
    {
        if ((pNVA->length != 0) &&      /* The record is valid if length != 0*/
            (pNVA->id < MAX_RAIDS))     /* RID must be valid            */
        {
            rdd = R_rddindx[pNVA->id];
            if (((UINT32)rdd != 0) &&      /* RDD must be valid             */
                (rdd->type == RD_RAID5) && /* Must be a RAID 5 type         */
                (pRUFlag[rdd->rid] == 0) &&/* This RAID hasn't been updated */
                (DL_AmIOwner(rdd->vid)))   /* This controller owns the VID  */
            {
                pRUFlag[rdd->rid] = 1;     /* Show this RAID as being incr  */
                rddChanged = TRUE;         /* Need to update the config     */
                ++rdd->r5SROut;            /* Increment the outstanding cnt */
                BIT_SET(rdd->aStatus,RD_A_R5SRIP); /* Set the AStatus flag  */
            }
        }
    }

    /* Free the previously allocated RAID Update Flag array */
    s_Free((void*)pRUFlag, MAX_RAIDS, __FILE__, __LINE__);

    /* If any RDD was changed, then update the configuration */
    if (rddChanged == TRUE)
    {
        BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        NV_P2Update();
    }
}


/**
******************************************************************************
**
**  @brief      From the NVA Record List, clear the AStatus bit on in every RDD
**              that is owned by this controller, if all done.
**
**              The NVA Record List passed in will be searched for any valid
**              entries for this controller. Once a valid entry is found, the
**              RDD R5 Stripe Resync Request Count byte will be decremented.
**              Once all the Stripe Resync Request Count is zero, the AStatus
**              flag can also be cleared. If any AStatus fields is updated,
**              then the NVRAM config for the controller is updated.
**
**  @param      pNVA - pointer to the beginning NVA Record in the list
**              (excluding the header)
**  @param      numEntries - number of entries in the list to search
**
**  @return     none
**
******************************************************************************
**/
void NVA_ClearReSyncAStatus(NVA *pNVA, UINT32 numEntries)
{
    UINT32     i = 0;               /* Loop counter                     */
    UINT32     rddChanged = FALSE;  /* RDD Changed Flag                 */
    RDD       *rdd;                 /* Pointer to an RDD                */
    UINT8     *pRUFlag;             /* RAID Update Flag array pointer   */

    /*
    ** Allocate an array of byte flags in determining if this RAID has already
    ** been updated (only want to decrement the counter once for each call to
    ** this routine).
    */
    pRUFlag = (UINT8*) s_MallocC(MAX_RAIDS, __FILE__, __LINE__);   /* Get memory for list  */

    /*
    ** Go through each entry looking for RAIDs that this controller owns.
    ** Once found, decrement the Parity Resync Request Count and the
    ** AStatus fields of the RDD if no more requests are outstanding.
    */
    for (i = 0; i < numEntries; ++i, ++pNVA)
    {
        if ((pNVA->length != 0) &&      /* The record is valid if length != 0*/
            (pNVA->id < MAX_RAIDS))     /* RID must be valid                */
        {
            rdd = R_rddindx[pNVA->id];
            if (((UINT32)rdd != 0) &&       /* RDD must be valid            */
                (rdd->type == RD_RAID5) &&  /* Must be a RAID 5 type        */
                (pRUFlag[rdd->rid] == 0) && /* This RAID hasn't been updated*/
                (DL_AmIOwner(rdd->vid)))    /* This controller owns the VID */
            {
                pRUFlag[rdd->rid] = 1;      /* Show this RAID has been updated*/
                --rdd->r5SROut;             /* Decr the outstanding count   */
                if (rdd->r5SROut == 0)      /* If no more reqs, then RDD done*/
                {
                    rddChanged = TRUE;      /* Need to update the config    */
                    BIT_CLEAR(rdd->aStatus,RD_A_R5SRIP); /* Clear the AStatus*/
                }
            }
        }
    }

    /* Free the previously allocated RAID Update Flag array */
    s_Free((void*)pRUFlag, MAX_RAIDS, __FILE__, __LINE__);

    /* If any RDD was changed, then update the configuration */
    if (rddChanged == TRUE)
    {
        BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        NV_P2Update();
    }
}


/**
******************************************************************************
**
**  @brief      Determine if there are any RAID 5 Resync in Progress status
**
**              The RAIDs are checked to see if there are any in the RAID 5
**              Stripe Resync in Progress Status.
**
**  @param      none
**
**  @return     bool true = There is at least one RAID in the RAID 5 Stripe
**                      Resync In Progress Status
**              bool false = There are no RAIDs in the RAID 5 Stripe Resync In
**                      Progress Status
**
******************************************************************************
**/
bool NVA_CheckReSyncInProgress(void)
{
    UINT32 i;                        /* Loop counter                    */
    UINT8  resyncInProgress = false; /* RAID Stripe Resync In Prog?     */
    RDD   *pRDD;                     /* Pointer to an RDD               */


    for(i = 0; (i < MAX_RAIDS) && (resyncInProgress == false); i++)
    {
        pRDD = R_rddindx[i];             /* Get this RAID pointer            */

        if( (pRDD != 0) &&               /* Valid RAID Pointer?              */
            (BIT_TEST(pRDD->aStatus,RD_A_R5SRIP)) ) /* Stripe Resync in Prog? */
        {
            resyncInProgress = true;    /* Yes, set the flag and exit       */
        }
    }
    return resyncInProgress;
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
