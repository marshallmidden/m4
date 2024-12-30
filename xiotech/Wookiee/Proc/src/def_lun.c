/* $Id: def_lun.c 162911 2014-03-20 22:45:34Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       def_lun.c
**
**  @brief      Define functions Pertaining to LUN mappings
**
**  Copyright (c) 2003-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <stdio.h>
#include <string.h>
#include <byteswap.h>

#include "def_lun.h"

#include "cm.h"
#include "ccsm.h"
#include "copymap.h"
#include "dcd.h"
#include "defbe.h"
#include "GR_Error.h"
#include "kernel.h"
#include "lvm.h"
#include "misc.h"
#include "MR_Defs.h"
#include "nvram.h"
#include "p6.h"
#include "rebuild.h"
#include "RL_RDD.h"
#include "scd.h"
#include "sdd.h"
#include "system.h"
#include "target.h"
#include "vdd.h"
#include "vlar.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "stdio.h"
#include "stdlib.h"
#include "apool.h"
#include "async_nv.h"
#include "ss_nv.h"
#include "ss_nv_funcs.h"
#include "online.h"
#include "CT_defines.h"
#include "ddr.h"
#include "misc.h"

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
extern UINT8  gApoolActive;
extern PCB *gApoolOwnshipPCB;
extern UINT8  gAsyncApoolOwner;
extern UINT32 gApoolOwner;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
UINT8 DL_CheckLousyLuns(UINT16 vid, UINT16 sid);
VDD *DL_GetCopyOwner(VDD *vdd);
void DL_ClearVIDLUNMap(UINT16);

/*
******************************************************************************
** Public function prototypes - in other files
******************************************************************************
*/
extern void CCSM_cco(void);
extern void DEF_RemoveKey(UINT16 vid, UINT16 sid);

/*
******************************************************************************
** Public variables - in other files
******************************************************************************
*/
extern COR  *CM_cor_act_que;
extern OGER *gnv_oger_array[2];

/*
******************************************************************************
** Code Start
******************************************************************************
*/


/**
******************************************************************************
**
**  @brief      Get ownership of a specific copy for a VDD.
**
**  @param      vdd - VDD in a copy for which we want the parent
**
**  @return     vdd - the parent or grandparent (case of a deleted vlink)
**
******************************************************************************
**/
VDD *DL_GetCopyOwner(VDD *vdd)
{
    COR    *pCOR;
    COR    *pVDDCOR = ((DCD*)(vdd->pDCD))->cor;

    if (COR_GetLSVID(pVDDCOR) != 0xFFFF)
    {
        return pVDDCOR->srcvdd;
    }
    else
    {
        for (pCOR = (COR*) CM_cor_act_que; pCOR != NULL; pCOR = pCOR->link)
        {
            if (pCOR != pVDDCOR)
            {
                if (COR_GetRSVID(pCOR) == COR_GetRSVID(pVDDCOR) &&
                    COR_GetRDVID(pCOR) == COR_GetRDVID(pVDDCOR))
                {
                    return pCOR->srcvdd;
                }
            }
        }
    }
    return NULL;
}

/**
******************************************************************************
**
**  @brief      Set ownership of all virtual disks.
**
**              This function will reset all of the vdisk owner fields
**              and then parse the LUN mappings and copy records to
**              create the new values for ownership.
**
**              A multiple pass method will be used.
**
**              Pass 1 - reset all values to no owner.
**              Pass 2 - set all explicit ownerships from LVM settings.
**              Pass 3 - repeat until no changes - set the owner of any
**                       disk that is the destination of a copy to that
**                       of the source. If the source is unmapped and
**                       the destination is, assign the source the owner
**                       of the destination.
**
**  @param      None
**
**  @return     None
**
**  @attention  Modifies all virtual disk owner fields
**
******************************************************************************
**/
void DL_SetVDiskOwnership(void)
{
    UINT16  i;
    LVM    *lvm;
    SDD    *sdd;
    UINT8   done = FALSE;
    VDD    *vdd;
    UINT8   tempOwner = VD_NOEXPOWNER;
    UINT16  apool_vid = ~0;
    SCD    *pSCD = NULL;
    char    copy_name[17];
    char   *offset;

    /* Pass 1 - Reset all owners to no owner. */
    for (i = 0; i < MAX_VIRTUAL_DISKS; ++i)
    {
        if (gVDX.vdd[i] != NULL)
        {
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            gVDX.vdd[i]->owner = VD_NOEXPOWNER;
        }
    }

    /*
     * Pass 2 - For all servers and all mappings within the servers
     *          assign explicit ownership of the vdisk to the owner DCN.
     */
    for (i = 0; i < MAX_SERVERS; ++i)
    {
        if (NULL != (sdd = gSDX.sdd[i]))
        {
            /* Check both the visible and invisible mappings. */
            for (lvm = sdd->lvm; lvm != NULL; lvm = lvm->nlvm)
            {
                if (gVDX.vdd[lvm->vid] != NULL)
                {
                    if (gVDX.vdd[lvm->vid]->vd_outssms == NULL && gVDX.vdd[lvm->vid]->vd_incssms == NULL)
                    {
                        if (T_tgdindx[sdd->tid])
                        {
                            gVDX.vdd[lvm->vid]->owner = DL_ExtractDCN(T_tgdindx[sdd->tid]->owner);
                            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                        }
                    }
                }
            }

            for (lvm = sdd->ilvm; lvm != NULL; lvm = lvm->nlvm)
            {
                if (gVDX.vdd[lvm->vid] != NULL)
                {
                    if (gVDX.vdd[lvm->vid]->vd_outssms == NULL && gVDX.vdd[lvm->vid]->vd_incssms == NULL)
                    {
                        if (T_tgdindx[sdd->tid])
                        {
                            gVDX.vdd[lvm->vid]->owner = DL_ExtractDCN(T_tgdindx[sdd->tid]->owner);
                            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                        }
                    }
                }
            }
        }
    }

    /*
     * This first section of code sets owner of snap pool vdisks.
     * The second section sets ownership of the snapshot vdisks.
     */
    int am_i_owner;
    int g_spool_owner[2] = {0,0};
    int got[2] = {0,0};

    for (i = 0; i < MAX_VIRTUAL_DISKS; ++i)
    {
        if (gVDX.vdd[i] != NULL)
        {
            if (BIT_TEST(gVDX.vdd[i]->attr, VD_BSNAPPOOL))
            {
                got[i & 0x1] = 1;
                if ((int)DL_ExtractDCN(K_ficb->cSerial) == (int)(i & 0x1))
                {
                    am_i_owner = i_should_own_sp(DL_ExtractDCN(K_ficb->cSerial),DL_ExtractDCN(K_ficb->cSerial));
                }
                else
                {
                    am_i_owner = i_should_own_sp(DL_ExtractDCN(K_ficb->cSerial),DL_ExtractDCN(K_ficb->cSerial) ^ 1);
                }

                if (am_i_owner)
                {
                    // This controller is the owner of this snappool.
                    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                    gVDX.vdd[i]->owner = DL_ExtractDCN(K_ficb->cSerial);
                    g_spool_owner[i & 0x1] = 1;
                }
                else
                {
                    // The other controller is the owner of this snappool
                    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                    gVDX.vdd[i]->owner = DL_ExtractDCN(K_ficb->cSerial) ^ 1;
                    g_spool_owner[i & 0x1] = 0;
                }
            }  // End if snappool
        }
    }  // End for each Vdisk

    if (got[1] || got[0])
    {
        /* Find all snapshots and set ownership based on ownership of its snappool */
        int             j;
        RDD             *rdd = NULL;

        for (j = 0; j < MAX_VIRTUAL_DISKS; ++j)
        {
            if (gVDX.vdd[j] != NULL)
            {
                rdd = gVDX.vdd[j]->pRDD;
                if (rdd != NULL)
                {
                    if (rdd->type == RD_SLINKDEV)
                    {
                        // If this controller owns the snappool which this snapshot is
                        // associated with then set the ownership of snapshot to self.
                        // NOTE: the rdd depth field is used to store the preferred
                        // owning controller.
                        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                        if (g_spool_owner[rdd->depth])
                        {
                            // Set owner to this controller
                            gVDX.vdd[j]->owner = DL_ExtractDCN(K_ficb->cSerial);
                        }
                        else
                        {
                            // Set ownership to the other controller
                            gVDX.vdd[j]->owner = DL_ExtractDCN(K_ficb->cSerial) ^ 1;
                        }
                        // Set the owner of the source to the same owner as the snapshot.
                        // Note the source VID is stashed in the rdd sps field.
                        gVDX.vdd[rdd->sps]->owner = gVDX.vdd[j]->owner;
                   }
               }
           }
       }
     }
     if (got[1] || got[0] || gnv_oger_array[0] || gnv_oger_array[1])
     {
       /*
        * Create Snapshot restore task only if there is at least one snap-pool
        * in the system.
        */
       restore_all_ss_nv();
     }

    /*
     * Pass 3 - traverse the VDD list looking for copies in progress as a.
     *          destination. If one is found, change the destination owner
     *          if it is not already set. If it is set and the source is
     *          not set, then set the source.
     */
    copy_name[16] = '\0';
    apool_set_max_usr_ap_data(30000);
    while (!done)
    {
        done = TRUE;
        for (i = 0; i < MAX_VIRTUAL_DISKS; ++i)
        {
            if (gVDX.vdd[i]==NULL)
            {
                continue;
            }

            if(gVDX.vdd[i]->name[0])
            {
                memcpy(copy_name, gVDX.vdd[i]->name, 16);
                offset = strstr(copy_name, "THROTTLE=");
                if (offset)
                {
                    apool_set_max_usr_ap_data((UINT32)atoi(offset+9));
                }
            }


            if (gVDX.vdd[i]->pDCD != NULL )
            {
                if (gVDX.vdd[i]->pDCD->cor != NULL )
                {
                    if (gVDX.vdd[i]->pDCD->cor->label[0])
                    {
                        /*
                          * The format for the label is overloaded to allow changing of
                          * async rep key parameters. In this case the string must
                          * take the form "MXD=xxxxMQD=xxxx". Where the numbers
                          * after each parameter are used to set the async values.
                          * The ccbcl command is RESYNCCTL.
                          */
                        memcpy(copy_name, gVDX.vdd[i]->pDCD->cor->label, 16);
                        offset = strstr(copy_name, "MQD=");
                        if (offset)
                        {
                            apool_set_qd((UINT32)atoi(offset+4));
                        }
                        copy_name[8] ='\0';
                        offset = strstr(copy_name, "MXD=");
                        if (offset)
                        {
                            apool_set_data_sz((UINT32)atoi(offset+4));
                        }
                    }
                }

                vdd = DL_GetCopyOwner(gVDX.vdd[i]);
                /*
                 * If the owner is NULL as would be the case in a deleted
                 * vlink at the root of a copy tree, then set the owner
                 * to no owner.
                 */
                if (vdd == NULL)
                {
                    tempOwner = VD_NOEXPOWNER;
                }
                else
                {
                    tempOwner = vdd->owner;
                }

                /*
                 * If the current owner value of this VDD is no owner, then
                 * assume the owner of the parent. If the owner is already
                 * set for this node, then set the parent to the same value
                 * if it is not set. Otherwise, do nothing.
                 */
                if (gVDX.vdd[i]->owner == VD_NOEXPOWNER)
                {
                    if (tempOwner != VD_NOEXPOWNER)
                    {
                        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                        gVDX.vdd[i]->owner = tempOwner;
                        done = FALSE;
                    }
                }
                else if ((tempOwner == VD_NOEXPOWNER) && (vdd != NULL))
                {
                    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                    vdd->owner = gVDX.vdd[i]->owner;
                    done = FALSE;
                }
            }
        }
    }

    /*
     * Pass - 4: By this time, the ownership of all the vdisks including that are
     * involved in copy/mirror operation also set. Now we must set the following:
     *   1) Ownership of the APool VDisk based on alink ownership
     *   2) Alink ownership of any Alinks
     *   3) Adjust ownership of all vdisks that are part of ALink copy chains
     * of course, skipping all of this section if we have no alinks or apools
     */

    /* First determine if we even have any ALinks or APools */
    for (i = 0; i < MAX_VIRTUAL_DISKS; ++i)
    {
        if (gVDX.vdd[i] != NULL)
        {
            if (BIT_TEST(gVDX.vdd[i]->attr, VD_BASYNCH))
            {
                break;
            }
        }
    }
    if (i >= MAX_VIRTUAL_DISKS)
    {
        return;
    }

    /* Find the owner of the first alink whose source is associated w/server */
    //NOTE: this means we won't allow a copy to alink from VDisk with dest bit set
    tempOwner = VD_NOEXPOWNER;
    gApoolOwner = 0xffffffff;
    for (i = 0; i < MAX_VIRTUAL_DISKS; ++i)
    {
        if (!gVDX.vdd[i] || !gVDX.vdd[i]->pDCD || !gVDX.vdd[i]->pDCD->cor)
        {
            continue;
        }
        if (!BIT_TEST(gVDX.vdd[i]->attr, VD_BASYNCH) ||
            !BIT_TEST(gVDX.vdd[i]->attr, VD_BVLINK))
        {
            continue;
        }

        /*
         * Only get ownership from an alink whose source
         * is associated with a server.
         */
        if (gVDX.vdd[i]->owner == VD_NOEXPOWNER)
        {
            continue;
        }

        UINT16 tid;

        tid = apool_get_tid(gVDX.vdd[i]->pDCD->cor->srcvdd);
        if (tid != 0xffff)
        {
            // Destination ownership is set in pass 2 above.
            if (T_tgdindx[tid])
            {
                tempOwner = gVDX.vdd[i]->owner;
                gApoolOwner = T_tgdindx[tid]->prefOwner;
                break;
            }
        }
    }

    /* If there are no ALinks that are 'owned', set ownership to the Master */
    if (i >= MAX_VIRTUAL_DISKS)
    {
        tempOwner = DL_ExtractDCN(K_ficb->cSerial);
        if (!BIT_TEST(K_ii.status,II_MASTER))
        {
            // If this is not the master then set ownership other controller
            tempOwner ^= 1;
        }
    }

    /* Now find the apool and set owner to the tempOwner just calculated */
    for (i = 0; i < MAX_VIRTUAL_DISKS; ++i)
    {
        /* IFF this is an APOOL (valid vid with async bit set AND is not a VLINK) */
        if ((gVDX.vdd[i] != NULL) && BIT_TEST(gVDX.vdd[i]->attr, VD_BASYNCH) &&
            !BIT_TEST(gVDX.vdd[i]->attr, VD_BVLINK) )
        {
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            gVDX.vdd[i]->owner = tempOwner;
            apool_vid = i;

            /*
             * The owner of apool may have changed, if there are any copies associated
             * with apool, change the owner of its destination vdisks.
             */
            for (pSCD = gVDX.vdd[i]->pSCDHead; pSCD != NULL; pSCD = pSCD->link)
            {
                if (pSCD->cor && pSCD->cor->destvdd)
                {
                    pSCD->cor->destvdd->owner = gVDX.vdd[i]->owner;
                }
            }

            break;
        }
    }

    /* Prep for treewalk */
    static INT16 VF[MAX_VIRTUAL_DISKS];

    for (i = 0; i < MAX_VIRTUAL_DISKS; ++i)
    {
        /*
         * If the vdd is valid AND has async bit set, then 'unset' VF[i] (i.e to owner)
         * else flag it for potential adjustment if it is just a plain vdisk or empty
         */
        if (gVDX.vdd[i] && BIT_TEST(gVDX.vdd[i]->attr, VD_BASYNCH))
        {
            VF[i] = tempOwner;
        }
        else
        {
            VF[i] = -1;
        }
    }

    /*
     * Update the owner field for all vdisks that are part of the copy tree
     * of an alink. This means going up and then back down in the tree.
     *
     * First, set ownership of all ALinks to that of the APOOL
     * NOTE: couldn't do this in the first loop while looking for the apool since apool
     *       might be found after alink
     */
    for (i = 0; i < MAX_VIRTUAL_DISKS; ++i)
    {
        /* If the vdd is valid and is an ALink, set ownership to that of apool */
        if (!gVDX.vdd[i] || !BIT_TEST(gVDX.vdd[i]->attr, VD_BASYNCH))
        {
            continue;                      /* If not an alink or apool, check next one */
        }

        if (!BIT_TEST(gVDX.vdd[i]->attr, VD_BVLINK))
        {
            continue;                      /* if not a vlink continue */
        }
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        gVDX.vdd[i]->owner = tempOwner;    /* got an ALink so set it to owner of APOOL */
    }


    /*
     * Now that the alinks and apools are all owned and everything else is 'flagged' scan
     * up and down copy trees assigning ownership to vdisks that have alinks in the chain
     */
    UINT8 still_looking = TRUE;
    while (still_looking)
    {
        int j;

        still_looking = FALSE;
        for (j = 0; j < MAX_VIRTUAL_DISKS; ++j)                 /* For each vdisk, look for: */
        {
            if (!gVDX.vdd[j])                                   /*   A valid vdisk */
            {
                continue;
            }

            if (!BIT_TEST(gVDX.vdd[j]->attr, VD_BDCD))          /*   and a dest. of a copy */
            {
                continue;
            }

            UINT16 svid = gVDX.vdd[j]->scorVID;                 /* Get the source VID and */

            if (svid >= MAX_VIRTUAL_DISKS || !gVDX.vdd[svid])   /* Make sure it is a vdisk */
            {
                continue;
            }

            if (VF[svid] >= 0 && VF[j] == -1)                   /* src owned, dest not? */
            {
                still_looking = TRUE;
                BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                gVDX.vdd[j]->owner = tempOwner;
                VF[j] = tempOwner;
            }
            if (VF[svid] == -1 && VF[j] >= 0)                   /* dest owned, source not? */
            {
                still_looking = TRUE;
                BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                gVDX.vdd[svid]->owner = tempOwner;
                VF[svid] = tempOwner;
            }
        }
    }

    /*
     * Is this controller is owning the apool? if it is, restore the apool nv elements
     * and start mover task on this controller
     */
    if ((DL_ExtractDCN(K_ficb->cSerial) == tempOwner))
    {

        /* This controller has ownership of the apool, set ownership flag and task. */
//        fprintf(stderr, "%s: This controller is owner of Apool\n", __func__);

        gAsyncApoolOwner = TRUE;
        mover_task_status &= ~(1 << MR_APOOL_KILL_MOVER);

        if (!gApoolActive && !gApoolOwnshipPCB)
        {
            AR_StartOwnershipTask(0,apool_vid);
        }

    }
    else if (gAsyncApoolOwner)
    {
        /* Stop the mover task here */
//        fprintf(stderr, "%s: This controller is NOT the owner of Apool\n", __func__);
        AR_DisownApool(0);
        gAsyncApoolOwner = FALSE;
    }
}

/**
******************************************************************************
**
**  @brief      Indicate if we are owner of a virtual disk.
**
**              This function will return a boolean value indicating
**              if the controller calling the function owns the vdisk
**              passed in.
**
**  @param      vid - virtual disk ID being checked for ownership
**
**  @return     T/F - indication of if we own the vdisk
**
******************************************************************************
**/
UINT32 DL_AmIOwner(UINT16 vid)
{

    UINT32  amIOwner = FALSE;

    if (gVDX.vdd[vid] != NULL)
    {
#if 1
        if (DL_ExtractDCN(K_ficb->cSerial) == gVDX.vdd[vid]->owner)
        {
            amIOwner = TRUE;
        }
        else if ((gVDX.vdd[vid]->owner == VD_NOEXPOWNER) && (BIT_TEST(K_ii.status,II_MASTER)))
        {
            amIOwner = TRUE;
        }
#else
        /*
         * The code below was the original version of DL_AmIOwner which had a problem
         * where ownership would return false when it wasn't. This is temporarily commented
         * out until the next release because of test time contraints.
         */

        /*
         * Check the virtual disk to see if there are any copies in
         * progress. If there are any, parse through them and determine
         * if any of them indicate this controller owns the vdisk. If
         * there are any transitional copies causing this controller to
         * own the vdisk, then we are the owner. If not, the copy and
         * therefor the VID are owned by another controller.
         */
       SCD    *pSCD;
       DCD    *pDCD;

       if (!amIOwner)
        {
            pDCD = (DCD*)(gVDX.vdd[vid]->pDCD);
            pSCD = (SCD*)(gVDX.vdd[vid]->pSCDHead);
            if ((pSCD != NULL) || (pDCD != NULL))
            {
                if (pDCD != NULL)
                {
                    if (pDCD->cor != NULL)
                    {
                        if (pDCD->cor->ocsecst == CCSM_ST_CpyOwned())
                        {
                            amIOwner = TRUE;
                        }
                    }
                }
                while ((pSCD != NULL) && !amIOwner)
                {
                    if (pSCD->cor->ocsecst == CCSM_ST_CpyOwned())
                    {
                        amIOwner = TRUE;
                    }
                    else
                    {
                        pSCD = pSCD->link;
                    }
                }
            }
            else
            {
                /*
                 * This VID is not part of a copy so process static ownership checking.
                 * There will be a few cases to check. First, if the controller
                 * ID (DCN number) matches the value in the VDD, then we own the vdisk.
                 * If it does not match and the owner if unowned, we will be the owner
                 * if we are the master controller.
                 */
                if (DL_ExtractDCN(K_ficb->cSerial) == gVDX.vdd[vid]->owner)
                {
                    amIOwner = TRUE;
                }

                else if ((gVDX.vdd[vid]->owner == VD_NOEXPOWNER) && (BIT_TEST(K_ii.status,II_MASTER)))
                {
                    amIOwner = TRUE;
                }

            }
        }
#endif
    }
    return(amIOwner);
}


/**
******************************************************************************
**
**  @brief      Checks for bad LUN mapping requests.
**
**              This function check the requested VID mapping to a SID and
**              see if it causes a virtual disk to be mapped to two
**              controllers.
**
**  @param      vid - VDisk ID being mapped
**  @param      sid - Server ID being used for mapping
**
**  @return     FALSE on OK (no lowsy luns), TRUE on bad mappings.
**
******************************************************************************
**/
UINT8 DL_CheckLousyLuns(UINT16 vid, UINT16 sid)
{
    UINT8   retCode = FALSE;

    /*
     * Check the owning controller of the server being requested and
     * the value of the owner field in the virtual disk being mapped.
     * If they are the same, it is OK. If they are different and the
     * owner of the virtual disk is not unassigned, then it is an error.
     */
    if (T_tgdindx[gSDX.sdd[sid]->tid] &&
        DL_ExtractDCN(T_tgdindx[gSDX.sdd[sid]->tid]->owner) != gVDX.vdd[vid]->owner)
    {
        if (gVDX.vdd[vid]->owner != VD_NOEXPOWNER)
        {
            retCode = TRUE;
        }
    }

    return(retCode);
}

/**
******************************************************************************
**
**  @brief      Remove a SDD from linked list of SDD.
**
**              This function check the requested VID mapping to a SID and
**              see if it causes a virtual disk to be mapped to two
**              controllers.
**
**  @param      sdd - Server record to be removed
**
**  @return     none.
**
******************************************************************************
**/
void DL_removeLnk(SDD *sdd)
{
    UINT32 linkedSID;
    SDD   *lsdd;

    /* Advance to next SDD in list. */
    linkedSID = sdd->linkedSID;
    lsdd = S_sddindx[linkedSID];

    /* Are there only two SDDs in the list? */
    if (lsdd->linkedSID == sdd->sid)
    {
        /* Now there is only one SDD in the list. */
        lsdd->linkedSID = 0xFFFF;
    }
    else
    {
        /*
         * Walk the linked SDD list until we arrive
         * at the SSD that points to the SDD being removed.
         */
        while (linkedSID != sdd->sid)
        {
            /* Advance to next SDD in list. */
            lsdd = S_sddindx[linkedSID];
            linkedSID = lsdd->linkedSID;
        }

        /* Remove the SSD from the linked list by pointing around the SDD. */
        lsdd->linkedSID = sdd->linkedSID;
    }

    /*
     * Update server record.
     * Set SID for input parm, False = do not delete.
     */
    DEF_UpdRmtServer(lsdd->sid, FALSE);

    /* Unlink the SDD being removed. */
    sdd->linkedSID = 0xFFFF;
}

/**
******************************************************************************
**
**  @brief      Links the source SDD into the linked list containing
**              the destination SDD.
**
**  @param      dsid - Id of source SDD
**  @param      ssid - Id of destination SDD
**
**  @return     Id of source SDD.
**
******************************************************************************
**/
UINT16 DL_insertLnk(UINT16 dsid, UINT16 ssid)
{
    UINT16 linkedSID;

    /* Get the source SDD link. */
    linkedSID = S_sddindx[ssid]->linkedSID;

    /* Check if source SDD already has a link. */
    if (linkedSID != 0xFFFF)
    {
        /* Find the SDD that points to the source SDD. */
        while (S_sddindx[linkedSID]->linkedSID != ssid)
        {
            linkedSID = S_sddindx[linkedSID]->linkedSID;
        }

        /* Check if destination SDD already has a link. */
        if (S_sddindx[dsid]->linkedSID != 0xFFFF)
        {
            /* Insert into link immediately after destination SDD. */
            S_sddindx[linkedSID]->linkedSID = S_sddindx[dsid]->linkedSID;
        }
        else
        {
            S_sddindx[linkedSID]->linkedSID = dsid;
        }
    }
    else if (S_sddindx[dsid]->linkedSID != 0xFFFF)
    {
        /* Insert into link immediately after destination SDD. */
        S_sddindx[ssid]->linkedSID = S_sddindx[dsid]->linkedSID;
    }
    else
    {
        /* This is the first link. Link source SDD to destination SDD. */
        S_sddindx[ssid]->linkedSID = dsid;
    }

    /* Link destination SSD to the source SDD. */
    S_sddindx[dsid]->linkedSID = ssid;

    return ssid;
}


/**
******************************************************************************
**
**  @brief      To delete all the LUN mappings for the specified server.
**
**              The LVM list and the invisible LVM is walked is
**              traversed and each entry is freed.
**
**  @param      SDD* - pointer to SDD.
**
**  @return     none
**
******************************************************************************
**/
void DL_DeleteLVM(SDD *sdd)
{
    LVM    *lvm;
    LVM    *nlvm;

    /* Get the LVM pointer. */
    lvm = sdd->lvm;

    /*
     * Process all LVM in the list. A NULL next pointer
     * inidcates the end of the list.
     */
    while (lvm != NULL)
    {
        /* Get next LVM prior to freeing current LVM */
        nlvm = lvm->nlvm;

        /* Release memory used by LVM */
        s_Free(lvm, sizeof(*lvm), __FILE__, __LINE__);

        /* Advance to next LVM */
        lvm = nlvm;
    }

    /* Set the LVM pointer to NULL. */
    sdd->lvm = NULL;

    /* Get the Invisible LVM pointer. */
    lvm = sdd->ilvm;

    /*
     * Process all LVM in the list. A NULL next pointer
     * inidcates the end of the list.
     */
    while (lvm != NULL)
    {
        /* Get next LVM prior to freeing current LVM */
        nlvm = lvm->nlvm;

        /* Release memory used by LVM */
        s_Free(lvm, sizeof(*lvm), __FILE__, __LINE__);

        /* Advance to next LVM */
        lvm = nlvm;
    }

    /* Set the Invisible LVM pointer to NULL. */
    sdd->ilvm = NULL;

    /* Set the number of LUNs to zero. */
    sdd->numLuns = 0;
}

/**
******************************************************************************
**
**  @brief      To provide a common means of associating a specific VDISK with
**              a given LUN.
**
**              The SID, VID and LUN parameters are validated with the new
**              LUN to virtual disk association is completed.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DL_SetLUNMap(MR_PKT *pMRP)
{
    MRMAPLUN_REQ   *mml;             /* MRP passed into the function */
    LVM            *lvm;             /* Pointer to LUN/VID map struct*/
    LVM            *ilvm;            /* Temp invisible LVM pointer   */
    LVM            *t0lvm;           /* Temp LVM pointer             */
    LVM            *t1lvm;           /* Temp LVM pointer             */
    RDD            *rdd;             /* RDD pointer                  */
    UINT16          dsid, ssid;      /* Source and destination SIDs  */
    UINT16          linkedSID;       /* Linked SID                   */
    UINT8           retStatus;

    /* Grab the MRP pointer into the appropriate type. */
    mml = (MRMAPLUN_REQ *) pMRP->pReq;

    /* Validate SID. */
    if ((mml->sid >=  MAX_SERVERS) || (S_sddindx[mml->sid] == NULL))
    {
        return DEINVSID;
    }

    /* Validate LUN. */
    if (mml->lun >=  MAX_LUNS)
    {
        return DEMAXLUNS;
    }

    if (mml->option != MMLMAP)
    {
        /* We are doing a copy, swap or move. */

        /* Validate destination SID. */
        if ((mml->dsid >=  MAX_SERVERS) || (S_sddindx[mml->dsid] == NULL))
        {
            return DEINVSID;
        }
        if (mml->option > MMLLINK)
        {
            return DEINVOPT;
        }

        /* Check SSD being linked together all are owned by same controller. */
        if (mml->option == MMLLINK &&
            T_tgdindx[S_sddindx[mml->sid]->tid] &&
            T_tgdindx[S_sddindx[mml->sid]->tid]->prefOwner !=
                 T_tgdindx[S_sddindx[mml->dsid]->tid]->prefOwner)
        {
            return DEINVCTRL;
        }

        /*
         * For all of the operations, we need to save the old associations
         * and clear the associations in the chain of servers in the
         * destination. We may or may not have to clear the source servers
         * later depending upon the operation type.
         */
        lvm   = S_sddindx[mml->dsid]->lvm;
        ilvm  = S_sddindx[mml->dsid]->ilvm;

        /*
         * Clear out the first server destination since we have grabbed
         * the associations for the swap that may happen. They will be
         * deleted later.
         */
        S_sddindx[mml->dsid]->numLuns = 0;
        S_sddindx[mml->dsid]->lvm = NULL;
        S_sddindx[mml->dsid]->ilvm = NULL;

        for (dsid = S_sddindx[mml->dsid]->linkedSID;
            dsid != mml->dsid && dsid != 0xFFFF;
            dsid = S_sddindx[dsid]->linkedSID)
        {
            /* Clear the mappings in this SDD. */
            while (S_sddindx[dsid]->lvm != NULL)
            {
                t0lvm = S_sddindx[dsid]->lvm;
                S_sddindx[dsid]->lvm = S_sddindx[dsid]->lvm->nlvm;
                s_Free(t0lvm, sizeof(struct LVM), __FILE__, __LINE__);
            }

            while (S_sddindx[dsid]->ilvm != NULL)
            {
                t0lvm = S_sddindx[dsid]->ilvm;
                S_sddindx[dsid]->ilvm = S_sddindx[dsid]->ilvm->nlvm;
                s_Free(t0lvm, sizeof(struct LVM), __FILE__, __LINE__);
            }

            S_sddindx[dsid]->numLuns = 0;
        }

        /*
         * For Link case, all SDDs with a different WWN must be
         * removed the linked list containing the destination SDD.
         * The remaining linked list containing the destination SDD
         * will contain SDDs that all have the same WWN. That allows
         * these SDDs to be added to the linked list containing
         * the source SDD.
         */
        if (mml->option == MMLLINK)
        {
            /* Get the SDD linked to destination SDD. */
            linkedSID = S_sddindx[mml->dsid]->linkedSID;

            /*
             * Check linked SDD exists. If not linked, there
             * is nothing to do. Otherwise, all SDDs with
             * a different WWN will be moved to a new list.
             */
            if (linkedSID != 0xFFFF)
            {
                /* Initialize to indicate no destination linked list. */
                dsid = 0xFFFF;

                do
                {
                    /* Advance to next SDD */
                    ssid = linkedSID;

                    /* Get next SDD prior to removing target SDD from linked list. */
                    linkedSID = S_sddindx[ssid]->linkedSID;

                    if (S_sddindx[linkedSID]->wwn != S_sddindx[mml->dsid]->wwn)
                    {
                        /* Remove this link. */
                        DL_removeLnk(S_sddindx[linkedSID]);

                        /*
                         * Add back into linked list of SDD
                         * removed from linked list containing
                         * the destination SDD.
                         */
                        if (dsid != 0xFFFF)
                        {
                            dsid = DL_insertLnk(dsid, linkedSID);
                        }
                        else
                        {
                            dsid = linkedSID;
                        }
                    }
                } while (linkedSID != mml->dsid);
            }
        }

        /*
         * For all cases, we need to copy the sources down to all of the
         * destinations. After this is done, we treat the sources for
         * each of the cases.
         */
        dsid = mml->dsid;

        do
        {
            t0lvm = S_sddindx[mml->sid]->lvm;

            /*
             * Process all LVM in the list. A NULL next pointer
             * inidcates the end of the list.
             */
            while (t0lvm != NULL)
            {
                /* Get the LUN and put it into the SDD hash table. */
                DEF_HashLUN(S_sddindx[dsid], t0lvm->lun, t0lvm->vid);

                /* Advance to next LVM */
                t0lvm = t0lvm->nlvm;
            }

            t0lvm = S_sddindx[mml->sid]->ilvm;

            while (t0lvm != NULL)
            {
                /* Get the LUN and put it into the SDD hash table. */
                DEF_HashLUN(S_sddindx[dsid], t0lvm->lun, t0lvm->vid);

                /* Advance to next LVM */
                t0lvm = t0lvm->nlvm;
            }

            /* Move to the next destination SID. */
            dsid = S_sddindx[dsid]->linkedSID;
        } while (dsid != mml->dsid && dsid != 0xFFFF);

        /*
         * The copy to the destinations is now complete. Handle the
         * specifics of the clean up for the different options now.
         */
        if (mml->option == MMLSWAP || mml->option == MMLMOVE)
        {
            ssid = mml->sid;

            do
            {
                /* Clear the mappings in this SDD. */
                while (S_sddindx[ssid]->lvm != NULL)
                {
                    t0lvm = S_sddindx[ssid]->lvm;
                    S_sddindx[ssid]->lvm = S_sddindx[ssid]->lvm->nlvm;
                    s_Free(t0lvm, sizeof(struct LVM), __FILE__, __LINE__);
                }

                while (S_sddindx[ssid]->ilvm != NULL)
                {
                    t0lvm = S_sddindx[ssid]->ilvm;
                    S_sddindx[ssid]->ilvm = S_sddindx[ssid]->ilvm->nlvm;
                    s_Free(t0lvm, sizeof(struct LVM), __FILE__, __LINE__);
                }

                S_sddindx[ssid]->numLuns = 0;

                /* Move to the next destination SID. */
                ssid = S_sddindx[ssid]->linkedSID;
            } while (ssid != mml->sid && ssid != 0xFFFF);
        }

        /*
         * For swap, we now have to replicate the mappings from the
         * destination to the source.
         */
        if (mml->option == MMLSWAP)
        {
            ssid = mml->sid;

            do
            {
                /* Copy the mappings from the destination to this SDD. */
                for (t0lvm = lvm; t0lvm != NULL; )
                {
                    DEF_HashLUN(S_sddindx[ssid], t0lvm->lun, t0lvm->vid);
                    t0lvm = t0lvm->nlvm;
                }

                for (t0lvm = ilvm; t0lvm != NULL; )
                {
                    DEF_HashLUN(S_sddindx[ssid], t0lvm->lun, t0lvm->vid);
                    t0lvm = t0lvm->nlvm;
                }

                /* Move to the next destination SID. */
                ssid = S_sddindx[ssid]->linkedSID;
            } while (ssid != mml->sid && ssid != 0xFFFF);
        }

        if (mml->option == MMLLINK)
        {
            /* Insert destination SDD into source list. */
            DL_insertLnk(mml->sid, mml->dsid);
        }

        /*
         * Now clean up the destination mapping structures since they
         * are no longer needed.
         */
        for ( ; lvm != NULL; )
        {
            t1lvm = lvm->nlvm;
            s_Free(lvm, sizeof(struct LVM), __FILE__, __LINE__);
            lvm = t1lvm;
        }

        for ( ; ilvm != NULL; )
        {
            t1lvm = ilvm->nlvm;
            s_Free(ilvm, sizeof(struct LVM), __FILE__, __LINE__);
            ilvm = t1lvm;
        }
    }
    else
    {
        /*
         * Regular mappings. Validate SID and VID, check for max lun mapping,
         * check for SAN Links, check for operable VDisk, and check for RAIDs
         * in the process of initializing.
         */
        if ((mml->vid >= MAX_VIRTUAL_DISKS) || (V_vddindx[mml->vid] == NULL))
        {
            return DEINVVIRTID;
        }
        if (S_sddindx[mml->sid]->numLuns >= MAX_LUNS)
        {
            return DEMAXLUNS;
        }
#if 0 // Enable this piece of code to fix SAN-126/SAN-735
        /* Deny vlink association if the vdisk is of GeoRaid type... */
        if ( (GR_IS_GEORAID_VDISK(gVDX.vdd[mml->vid])) &&
                  (BIT_TEST(S_sddindx[mml->sid]->attrib, SD_XIO) == TRUE))
        {
            fprintf(stderr,"<GR>GeoRaid vdisk can't be associated to Xiotech Server(no vlink allowed)\n");
            GR_LogEvent(GR_VLINK_ASSOCIATE,gVDX.vdd[mml->vid],NULL);
            return DEINVOP;
        }
#endif
        if (V_vddindx[mml->vid]->attr & VD_SLINK)
        {
            return DEDEVUSED;
        }
        if (V_vddindx[mml->vid]->attr & VD_ASYNCH)
        {
            return DEDEVUSED;
        }
        if (V_vddindx[mml->vid]->pVLinks != NULL)
        {
            return DEDEVUSED;
        }
        if (V_vddindx[mml->vid]->status == VD_INOP)
        {
            return DEINOPDEV;
        }
        if (V_vddindx[mml->vid]->pDRDD != 0)
        {
            return DEINITINPROG;
        }
        if ((V_vddindx[mml->vid]->pDCD != NULL)      &&
                (((DCD*) V_vddindx[mml->vid]->pDCD)->cor != NULL) &&
                (((DCD*) V_vddindx[mml->vid]->pDCD)->cor->crstate <= CRST_AUTOSUSP))
        {
            return DEDEVUSED;
        }

        rdd = V_vddindx[mml->vid]->pRDD;

        while (rdd != NULL)
        {
            if ((rdd->status == RD_INIT) ||
               (rdd->status == RD_UNINIT))
            {
                return DEINITINPROG;
            }

            rdd = rdd->pNRDD;
        }

        /* Check if the LUN is already mapped. If so, return an error. */
        lvm = S_sddindx[mml->sid]->lvm;

        while (lvm != NULL)
        {
            if ((lvm->lun == mml->lun) || (lvm->vid == mml->vid))
            {
                return DELUNMAPPED;
            }
            lvm = lvm->nlvm;
        }

        lvm = S_sddindx[mml->sid]->ilvm;

        while (lvm != NULL)
        {
            if (lvm->vid == mml->vid)
            {
                return DELUNMAPPED;
            }
            lvm = lvm->nlvm;
        }

        /* Check for really bad mappings (cross-controller). */
        if (DL_CheckLousyLuns(mml->vid, mml->sid) == TRUE)
        {
            return DE_VDISK_IN_USE;
        }

        /* Get the LUN and put it into the SDD hash table. */
        ssid = mml->sid;

        do
        {
            DEF_HashLUN(S_sddindx[ssid],  mml->lun, mml->vid);

            /* Move to the next destination SID. */
            ssid = S_sddindx[ssid]->linkedSID;
        } while (ssid != mml->sid && ssid != 0xFFFF);
    }

    /* Handling for Async mirrors */
    if (V_vddindx[mml->vid]->pSCDHead != NULL &&
        ((SCD*) V_vddindx[mml->vid]->pSCDHead)->cor != NULL)
    {
        retStatus = apool_validate_server_association(mml->vid, S_sddindx[mml->sid]->tid);
        if (retStatus != DEOK)
        {
            return retStatus;
        }
    }

    /* Update NVRAM part II. */
    NV_P2UpdateConfig();

    /* Start any rebuilds this controller might now own */
    RB_SearchForFailedPSDs();

    /* Update remote - Tell FEP that a vdisk changed */
    DEF_SignalVDiskUpdate();

    /* Update server record. Set SID for input parm, False = do not delete. */
    ssid = mml->sid;

    do
    {
        DEF_UpdRmtServer(ssid, FALSE);
        ssid = S_sddindx[ssid]->linkedSID;
    } while (ssid != mml->sid && ssid != 0xFFFF);

    if (mml->option != MMLMAP)
    {
        /*
         * Update destination server record.
         * Set SID for input parm, False = do not delete.
         */
        dsid = mml->dsid;

        do
        {
            DEF_UpdRmtServer(dsid, FALSE);
            dsid = S_sddindx[dsid]->linkedSID;
        } while (dsid != mml->dsid && dsid != 0xFFFF);
    }

    /* Indicate that the update occurred */
    DEF_SignalServerUpdate();

    /* Issue a Configuration Changed Occurred to CCSM */
    CCSM_cco();

    return DEOK;
}

/**
******************************************************************************
**
**  @brief      To provide a common means of disassociating a specific VDISK
**              from a given LUN.
**
**              The SID and LUN parameters are validated and the first LVM
**              found with that LUN is removed.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DL_ClearLUNMap(MR_PKT *pMRP)
{
    MRUNMAPLUN_REQ *mul;                /* MRP passed into the function */
    LVM            *lvm;                /* Pointer to LUN/VID map struct*/
    LVM            *plvm;               /* Pointer to previous LUN/VID  */
    UINT16          sid;                /* Server ID temp variable      */
    VLAR           *vlink;              /* VLinks assoc. records        */
    UINT8           retStatus = DEOK;
    UINT32          serial;             /* Serial number of server      */

    mul = (MRUNMAPLUN_REQ *) pMRP->pReq;

    /* Validate SID. */
    if (mul->sid >=  MAX_SERVERS || S_sddindx[mul->sid] == NULL)
    {
        retStatus = DEINVSID;
    }
    else
    {
        /*
         * The unmapping will look through each server in the server chain
         * and unmap any servers that it can. There should not be any
         * inconsistency between the servers in the chain, but they will all
         * be checked anyway and no error reported if an inconsistency is found.
         */
        sid = mul->sid;

        do
        {
            /* Check if this is a XIOtech Controller */
            if ((retStatus == DEOK) && (M_chk4XIO(S_sddindx[sid]->wwn) != 0))
            {
                /*
                 * Check if specified server has a VLink established to the
                 * specified virtual device and if so reject request.
                 */
                if (V_vddindx[mul->vid] != NULL)
                {
                    if (NULL != (vlink = V_vddindx[mul->vid]->pVLinks))
                    {
                        serial = (bswap_32(*(((UINT32 *)&S_sddindx[sid]->wwn) + 1)) & 0x000FFFF0) >> 4;

                        while (vlink != NULL)
                        {
                            if (serial == vlink->srcSN)
                            {
                                retStatus = DEDEVUSED;
                                break;
                            }

                            vlink = vlink->link;
                        }
                    }
                }
            }

            sid = S_sddindx[sid]->linkedSID;
        } while (sid != mul->sid && sid != 0xFFFF);

        if (retStatus == DEOK)
        {
            sid = mul->sid;

            do
            {
                if (mul->lun == 0xFF)
                {
                    lvm = S_sddindx[sid]->ilvm;
                }
                else
                {
                    lvm = S_sddindx[sid]->lvm;
                }

                /*
                 * Now search through the list, looking for the LVM that we wish
                 * to remove.
                 */
                plvm = NULL;

                while (lvm != NULL)
                {
                    if ((lvm->lun == mul->lun) && (lvm->vid == mul->vid))
                    {
                        if (plvm == NULL)
                        {
                            if (mul->lun == 0xFF)
                            {
                                S_sddindx[sid]->ilvm = lvm->nlvm;
                            }
                            else
                            {
                                S_sddindx[sid]->lvm = lvm->nlvm;
                            }
                        }
                        else
                        {
                            plvm->nlvm = lvm->nlvm;
                        }

                        S_sddindx[sid]->numLuns--;

                        /* If PRR update is required, take care of it here. */
                        DEF_RemoveKey(lvm->vid, sid);

                        s_Free(lvm, sizeof(struct LVM), __FILE__, __LINE__);
                        break;
                    }
                    else
                    {
                        plvm = lvm;
                        lvm = lvm->nlvm;
                    }
                }

                sid = S_sddindx[sid]->linkedSID;
            } while (sid != mul->sid && sid != 0xFFFF);

            /* Update NVRAM part II. */
            NV_P2UpdateConfig();

            /* Start any rebuilds this controller might now own */
            RB_SearchForFailedPSDs();

            /* Update remote - Tell FEP that a vdisk changed */
            DEF_SignalVDiskUpdate();

            /*
             * Update server record.
             * Set SID for input parm, False = do not delete.
             */
            sid = mul->sid;

            do
            {
                DEF_UpdRmtServer(sid, FALSE);
                sid = S_sddindx[sid]->linkedSID;
            } while (sid != mul->sid && sid != 0xFFFF);

            /* Indicate that the update occurred */
            DEF_SignalServerUpdate();

            /* Issue a Configuration Changed Occurred to CCSM */
            CCSM_cco();
        }
    }

    return retStatus;
}


/**
******************************************************************************
**
**  @brief      To provide a common means of disassociating a specific VDISK
**              from all LUNs.
**
**              All server records are examined to see if this VID is mapped
**              to any of them. All mappings found are removed and the
**              front end is updated to reflect the changes.
**
**  @param      VID - virtual disk ID of the disk having its mappings removed
**
**  @return     None.
**
******************************************************************************
**/
void DL_ClearVIDLUNMap(UINT16 vid)
{
    LVM       *lvm;
    LVM       *plvm;
    UINT16     sid;
    SDD       *sdd;
    UINT8      updateReq = FALSE;

    /*
     * The unmapping will look through each server in the server table
     * and unmap any servers that it finds using that VID.
     */
    for (sid = 0; sid < MAX_SERVERS; ++sid)
    {
        /*
         * Get the server struct and search the LVM chain and the invisisble
         * LVM chain to see if the VID is mapped.
         */
        sdd = S_sddindx[sid];

        if (sdd != NULL)
        {
            lvm = S_sddindx[sid]->lvm;
            plvm = NULL;

            while (lvm != NULL)
            {
                if (lvm->vid == vid)
                {
                    if (plvm == NULL)
                    {
                        S_sddindx[sid]->lvm = lvm->nlvm;
                    }
                    else
                    {
                        plvm->nlvm = lvm->nlvm;
                    }

                    S_sddindx[sid]->numLuns--;

                    s_Free(lvm, sizeof(struct LVM), __FILE__, __LINE__);
                    DEF_UpdRmtServer(sid, FALSE);
                    updateReq = TRUE;
                    break;
                }
                else
                {
                    plvm = lvm;
                    lvm = lvm->nlvm;
                }
            }

            lvm = S_sddindx[sid]->ilvm;
            plvm = NULL;

            while (lvm != NULL)
            {
                if (lvm->vid == vid)
                {
                    if (plvm == NULL)
                    {
                        S_sddindx[sid]->ilvm = lvm->nlvm;
                    }
                    else
                    {
                        plvm->nlvm = lvm->nlvm;
                    }

                    S_sddindx[sid]->numLuns--;

                    s_Free(lvm, sizeof(struct LVM), __FILE__, __LINE__);
                    DEF_UpdRmtServer(sid, FALSE);
                    updateReq = TRUE;
                    break;
                }
                else
                {
                    plvm = lvm;
                    lvm = lvm->nlvm;
                }
            }
        }

        /* Indicate that the update occurred if it did. */
        if (updateReq)
        {
            DEF_SignalServerUpdate();
        }
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
