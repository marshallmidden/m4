/* $Id: defbe.c 161678 2013-09-18 19:25:16Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       defbe.c
**
**  @brief      Define C functions
**
**  To provide support of configuration definition requests.
**
**  Copyright (c) 2002-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "defbe.h"

#include "ccsm.h"
#include "chn.h"
#include "cor.h"
#include "daml.h"
#include "DEF_BEGetInfo.h"
#include "def_con.h"
#include "def_isp.h"
#include "def_lun.h"
#include "defrag.h"
#include "dev.h"
#include "dlmbe.h"

#include "ficb.h"
#include "fsys.h"
#include "globalOptions.h"
#include "GR_LocationManager.h"
#include "GR_Error.h"
#include "fabric.h"
#include "isp.h"
#include "ses.h"
#include "kernel.h"
#include "LL_LinuxLinkLayer.h"
#include "loop.h"
#include "lvm.h"
#include "misc.h"
#include "miscbe.h"
#include "MR_Defs.h"
#include "nva.h"
#include "nvabe.h"
#include "nvr.h"
#include "nvram.h"
#include "online.h"
#include "pcb.h"
#include "p6.h"
#include "pdd.h"
#include "pm.h"
#include "pr.h"
#include "rebuild.h"
#include "rbr.h"
#include "RL_PSD.h"
#include "RL_RDD.h"
#include "scrub.h"
#include "scd.h"
#include "sdd.h"

#include "system.h"
#include "target.h"
#include "virtual.h"
#include "vdd.h"
#include "vlop.h"
#include "xdl.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"

#include <stdio.h>
#include <string.h>
#include <byteswap.h>

#include "CT_defines.h"
#include "CT_change_routines.h"

#include "apool.h"
#include "async_nv.h"
#include "ssms.h"
#include "ss_nv.h"
#include "ss_nv_funcs.h"
#include "ddr.h"
#include "misc.h"

#include "mem_pool.h"           /* Needed for VLAR memory pool. */

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
UINT8 D_p2writefail_flag = FALSE;   /* p2 NVRAN write failed flag */
UINT32      failBack = FALSE;
extern struct CHN* P_chn_ind[MAX_PORTS];
/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
UINT8       DEF_CheckRaidStates(void);
UINT8       DEF_CreateCtrl(MR_PKT *pMRP);
UINT8       DEF_CreateServer(MR_PKT *pMRP);
UINT8       DEF_DegradePort(MR_PKT *pMRP);
void        DEF_DeleteSDD(UINT32 sid);
UINT8       DEF_DeleteServer(MR_PKT *pMRP);
UINT8       DEF_FailController(MR_PKT *pMRP);
UINT8       DEF_Generic(MR_PKT *pMRP);
UINT8       DEF_GetDevicePaths(MR_PKT *pMRP);
UINT8       DEF_GetMirrorPartnerList(MR_PKT *pMRP);
UINT8       DEF_GetVDiskOwner(MR_PKT *pMRP);
UINT8       DEF_ConfigReset(MR_PKT *pMRP);
UINT8       DEF_ModifyRDDAStatus(MR_PKT *pMRP);
UINT8       DEF_PutDeviceConfig(MR_PKT *pMRP);
UINT8       DEF_SetAttr(MR_PKT *pMRP);
UINT8       DEF_ChgRAIDNotMirroringState(MR_PKT *);
UINT8       DEF_LabelDevice(MR_PKT *);
UINT8       DEF_CBridge(MR_PKT *);
UINT8       DEF_ConfigRestore(MR_PKT *pMRP);
void        DEF_LogCopyLabelEvent(COR *pCOR);

/*
******************************************************************************
** Forward defined routines - not in header file
******************************************************************************
*/
UINT8       ap_check_set_unset_asynch(UINT16 vid, int oper);
UINT8       sp_check_set_unset_snappool(UINT16 vid, int oper);

/*
******************************************************************************
** Public routines -- defined in other files
******************************************************************************
*/
extern UINT8 DEF_CfgRetrieve(MR_PKT *pMRP);
extern void DEF_ChgRAIDNotMirroringState_2(COR *);

extern void CCSM_term_copy(COR *cor);
extern void DEF_RemoveSrvKeys(UINT16 sid);
extern void L$send_packet(void *pReq, UINT32 reqSize, UINT32 cmdCode,
                          void *pRsp, UINT32 rspLen, void *pCallback,
                          UINT32 param);
extern void RB_CheckPDDForFailBack(PDD *);
extern void NVA_ClearP4(void);
extern void ON_BEClear(UINT16 pid, UINT8 justthisonepid);
extern void ON_BEBusy(UINT16 pid, UINT32 TimetoFail, UINT8 justthisonepid);

/*
******************************************************************************
** Public variables -- defined in other files
******************************************************************************
*/
extern COR *CM_cor_act_que;
extern UINT8 NV_scsi_whql;
extern UINT8    gss_version[2];

/* I have no idea why these routines have been defined the way they are. */
void        _apool_init(UINT16 wh_vid);
void        _apool_delete(UINT16 wh_vid);
void        _alink_init(UINT16 wh_vid);
void        _alink_delete(UINT16 wh_vid);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Returns the first server mapped to this VID.
**
**              This function looks through the server mappings to see if
**              the VID passed in is mapped to any server.
**
**              Included in this check is a check for a copy in progress.
**              If the vdisk is the destination of a copy, then the source
**              of the copy is used as the VID for the check.  This puts
**              the mapping on the proper controller.
**
**  @param      vid - VID being checked
**
**  @return     The first SID found to be mapped to this VID or -1 if none.
**
******************************************************************************
**/
UINT16 DEF_CheckForMapped(UINT16 vid)
{
    SDD        *sdd;            /* Pointer to a SDD                 */
    UINT16      sid = -1;       /* Assume no mapping                */
    UINT16      i;              /* Index                            */
    LVM        *lvm;            /* Lun mapping                      */

    /*
     * Check all servers but the one being mapped and all mappings to
     * see if there is overlap.
     */
    for (i = 0; i < MAX_SERVERS; ++i)
    {
        if (NULL != (sdd = S_sddindx[i]))
        {
            /*
             * Check both the visible and invisible mappings.
             */
            for (lvm = sdd->lvm; lvm != NULL; lvm = lvm->nlvm)
            {
                if (lvm->vid == vid)
                {
                    /*
                     * Mapped.
                     */
                    sid = i;
                    break;
                }
            }

            for (lvm = sdd->ilvm; lvm != NULL; lvm = lvm->nlvm)
            {
                if (lvm->vid == vid)
                {
                    /*
                     * Mapped.
                     */
                    sid = i;
                    break;
                }
            }
        }
    }

    return (sid);
}

/**
******************************************************************************
**
**  @brief      Update the miscellaneous status field within each PDD to
**              properly reflect whether the current state of the PDD.
**
**              Each rebuild record is examined to see if there are rebuilds in
**              progress.  If there are, then the first one on the RBR list is
**              the active one, the rest are scheduled.  The defrag list is also
**              examined for defrags (they are treated like rebuilds).
**
**  @param      None.
**
**  @return     None.
**
**  @attention  Changes misc status of all PDDs.
**
******************************************************************************
**/
void DEF_UMiscStat(void)
{
    UINT16      pid;            /* Index into table                 */
    RBR        *rbr;            /* RBR pointer                      */

    /*
     * Set all the drive misc status to clear the rebuilding and the
     * scheduled rebuilding bits.
     */
    for (pid = 0; pid < MAX_PHYSICAL_DISKS; pid++)
    {
        if (gPDX.pdd[pid] != NULL)
        {
            BIT_CLEAR(gPDX.pdd[pid]->miscStat, PD_MB_REBUILDING);
            BIT_CLEAR(gPDX.pdd[pid]->miscStat, PD_MB_SCHEDREBUILD);
            BIT_CLEAR(gPDX.pdd[pid]->miscStat, PD_MB_DEFRAGGING);
        }
    }

    /*
     * Now search for rebuilds in the RBR list.  If there is a rebuild on the
     * PDD, set the rebuild status.  If it is scheduled, then set the bit
     * to indicate such.
     */
    if (gRBRHead != NULL)
    {
        BIT_SET(gRBRHead->pdd->miscStat, PD_MB_REBUILDING);

        /*
         * If the PSD being rebuilt is being done for a defrag, then
         * set the defragging bit also.
         */
        if (BIT_TEST(gRBRHead->psd->aStatus, PSDA_DEFRAG))
        {
            BIT_SET(gRBRHead->pdd->miscStat, PD_MB_DEFRAGGING);
        }

        for (rbr = gRBRHead->nextRBR; rbr != NULL; rbr = rbr->nextRBR)
        {
            BIT_SET(rbr->pdd->miscStat, PD_MB_SCHEDREBUILD);
        }
    }
}

/**
******************************************************************************
**
**  @brief      Start initializations
**
**              This function will scan through all the RDDs in the system
**              and start an initialization if needed.
**
**  @param      none
**
**  @return     GOOD (always, not sure why)
**
******************************************************************************
**/
UINT8 DEF_CheckRaidStates(void)
{

    RDD        *rdd;            /* Pointer to an RDD */
    UINT16      i;              /* General purpose counter */

    /* Check each valid RDD in the system */
    for (i = 0; i < MAX_RAIDS; ++i)
    {
        rdd = R_rddindx[i];

        if (rdd && (BIT_TEST(rdd->aStatus, RD_A_UNINIT)) && !D_moveinprogress)
        {
            if (failBack == FALSE)
            {
                BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                BIT_CLEAR(rdd->aStatus, RD_A_TERMBG);
            }
            DEF_QueRInit(rdd);
        }
    }

    return (GOOD);
}

/**
******************************************************************************
**
**  @brief      To provide a means for the CCB to create a new set of
**              default servers and targets for a controller being brought
**              into a VCG.
**
**              The controller serial number of the new controller being brought
**              into the VCG is passed to this function via the MRP.  The four
**              servers and four targets are created and placed in the
**              configuration.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_CreateCtrl(MR_PKT *pMRP)
{
    SDD        *sdd;            /* SDD pointer to move through lists    */
    UINT32      count = 0;      /* Count of target to create            */
    UINT32      currentTrgts;   /* Current number of targets            */
    UINT32      currentCtrls;   /* Current number of controllers        */
    UINT32      i;              /* Loop variables                       */
    UINT32      j;
    UINT32      cserial;        /* Controller serial number             */
    UINT32      limit;          /* Loop ending condition                */
    UINT64      cserialSwapped; /* Controller serial number (byte-swap) */
    MRCREATECTRLR_REQ *mmc;     /* Pointer to input MRP                */
    UINT8       retStatus = DEOK;       /* Return value, prep good status       */
    UINT8       deltaFlag = FALSE;      /* Indicator that changes were made     */

    /*
     * Get pointer to Parm block address
     */
    mmc = (MRCREATECTRLR_REQ *) pMRP->pReq;

    /*
     * Get count from the input packet.
     */
    count = mmc->numControllers;
    currentTrgts = (UINT32)T_tgdindx[-1];
    currentCtrls = currentTrgts / TARGETS_PER_CTRL;

    /*
     * Limit the count to the maximum number of controllers
     */
    if (count > MAX_CTRL)
    {
        count = MAX_CTRL;
    }

    /*
     * Check if targets already exist.
     */
    if (currentCtrls != 0)
    {
        /*
         * Limit the following loop to the smaller of the current
         * number of controller or the new number of controller.
         */
        if (currentCtrls < count)
        {
            /*
             * Limit to the current number of controllers
             */
            limit = currentTrgts;
        }
        else
        {
            /*
             * Limit to the new number of controllers
             */
            limit = (count * TARGETS_PER_CTRL);
        }

        /*
         * Examine each of the current targets
         */
        for (i = 0; i < limit; ++i)
        {
            /*
             * Calculate the controller serial number for the target
             */
            cserial = (K_ficb->vcgID << 4) | (i >> 2);

            /*
             * Check if the peferred owner of this target is correct.
             */
            if (T_tgdindx[i] != NULL && T_tgdindx[i]->prefOwner != cserial)
            {
                /*
                 * Indiate changes be made to configuration
                 */
                deltaFlag = TRUE;

                /*
                 * Check if the target is assigned to the perferred owner.
                 */
                if (T_tgdindx[i]->owner == T_tgdindx[i]->prefOwner)
                {
                    /*
                     * Update the owner field
                     */
                    T_tgdindx[i]->owner = cserial;
                }
                else
                {
                    /*
                     * Update the owner field.  Don't alter
                     * which controller instance owns this target.
                     */
                    T_tgdindx[i]->owner &= 3;
                    T_tgdindx[i]->owner |= (cserial & 0xFFFFFFF0);
                }

                /*
                 * Update the perferred owner.
                 */
                T_tgdindx[i]->prefOwner = cserial;

                /*
                 * Bytes swap the serial number for inserting into
                 * the node/port world wide names.
                 */
                cserialSwapped = bswap_64(cserial & 0x00FFFFFF);

                /*
                 * Update the node WWN.
                 */
                T_tgdindx[i]->nodeName &= 0x000000FFFFFFFF;
                T_tgdindx[i]->nodeName |= cserialSwapped;

                /*
                 * Update the port WWN.
                 */
                T_tgdindx[i]->portName &= 0x000000FFFFFFFF;
                T_tgdindx[i]->portName |= cserialSwapped;

                /*
                 * Process all the server records.
                 */
                for (j = 0; j < MAX_SERVERS; ++j)
                {
                    /*
                     * Get a pointer to the next server record.
                     */
                    sdd = S_sddindx[j];

                    /*
                     * Check if the server record exists and is
                     * assigned to this target.
                     */
                    if (sdd != NULL && sdd->tid == i)
                    {
                        /*
                         * Update the owner field.
                         */
                        sdd->owner = cserial;
                    }
                }
            }
        }
    }

    /*
     * Check if the number of controllers is increasing.
     */
    if (count > currentCtrls)
    {
        /*
         * Indiate changes be made to configuration
         */
        deltaFlag = TRUE;

        /*
         * Pretty simple.  Grab the controller serial number and call the
         * defaults generator function.  FEP will be updated accordingly
         * with the servers and targets.
         */
        for (i = currentCtrls; i < count; ++i)
        {
            cserial = (K_ficb->vcgID << 4) | i;
            ON_CreateDefaults(cserial);
        }
    }
    /*
     * Check if the number of controllers is decreasing.
     */
    else if (count < currentCtrls)
    {
        /*
         * Indiate changes be made to configuration
         */
        deltaFlag = TRUE;

        /*
         * Check that the target to be deleted do not own any vdisks.
         */
        for (i = 0; i < MAX_SERVERS; ++i)
        {
            /*
             * Get a pointer to the next server record.
             */
            sdd = S_sddindx[i];

            /*
             * Check if this server is associated with
             * the specified target.
             */
            if (sdd != NULL &&
                sdd->tid >= (count * TARGETS_PER_CTRL) &&
                sdd->numLuns != 0)
            {
                retStatus = DEINVTID;
                break;
            }
        }

        /*
         * Did this target not own any Vdisk?
         */
        if (retStatus != DEINVTID)
        {
            /*
             * Process the execess target records
             */
            for (i = (count * TARGETS_PER_CTRL); i < (currentTrgts); ++i)
            {
                /*
                 * Delete the target record.
                 */
                if (T_tgdindx[i] != NULL)
                {
                    s_Free(T_tgdindx[i], sizeof(TGD), __FILE__, __LINE__);
                    T_tgdindx[i] = NULL;
                }

                /*
                 * Process all the server records.
                 */
                for (j = 0; j < MAX_SERVERS; ++j)
                {
                    /*
                     * Get a pointer to the next server record.
                     */
                    sdd = S_sddindx[j];

                    /*
                     * Check if the server record exists and is
                     * assigned to this target.
                     */
                    if (sdd != NULL && sdd->tid == i)
                    {
                        /*
                         * Delete the server record.
                         */
                        s_Free(sdd, sizeof(SDD), __FILE__, __LINE__);
                        S_sddindx[j] = NULL;

                        /*
                         * Update the number of targets.
                         */
                        /* (UINT32) S_sddindx[-1] -= 1; */
                        S_sddindx[-1] = (SDD *) ((UINT32)S_sddindx[-1] - 1);
                    }
                }
            }

            /*
             * Update the number of targets.
             */
            T_tgdindx[-1] = (TGD *) (count * TARGETS_PER_CTRL);
        }
    }

    /*
     * Update NVRAM if changes were made.
     */
    if (deltaFlag != FALSE)
    {
        NV_P2UpdateConfig();
    }

    return (retStatus);
}

/**
******************************************************************************
**
**  @brief      To fail or unfail a controller.
**
**              Moves all the targets for the specified port
**              from one controller to another.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_FailController(MR_PKT *pMRP)
{
    UINT32      oldOwner;
    UINT32      newOwner;
    UINT32      ports;
    UINT32      i;              /* Loop variables                       */
    MRFAILCTRL_REQ *mtc;        /* Pointer to input MRP                 */
    UINT8       retStatus = DEOK;       /* Return value, prep good status       */
    UINT8       deltaFlag = FALSE;      /* Indicator that changes were made     */
    UINT32      maxCserial = 0;
    UINT32      lnkTarget;
    UINT8       srcVdMap[(MAX_VIRTUAL_DISKS+7)/8];

    /*
     * Get pointer to Parm block address
     */
    mtc = (MRFAILCTRL_REQ *) pMRP->pReq;
    oldOwner = mtc->oldOwner;
    newOwner = mtc->newOwner;
    failBack = FALSE;

    /*
     * Check the options.
     */
    if ((mtc->option & FC_PORTS) != 0)
    {
        /*
         * Restrict to targets on specified ports.
         */
        ports = mtc->qualifier;
    }
    else
    {
        /*
         * Default option is to allow targets on all ports to be modified.
         */
        ports = 0xFFFFFFFF;
    }

    if ((mtc->option & FC_FB) != 0)
    {
        /*
         * This is a "fail-back" operation.  Target will
         * only be moved back to the perferred owner.
         */
        failBack = TRUE;
    }

#if 1                           /*GR_GEORAID15_DEBUG */
    if (failBack)
    {
        fprintf(stderr, "<GR>Unfailing the DCN-old owner=%x new owner=%x\n", oldOwner, newOwner);
    }
    else
    {
        fprintf(stderr, "<GR>Failing the DCN-old owner=%x new owner=%x\n", oldOwner, newOwner);
        memset(&srcVdMap, 0x0, sizeof(srcVdMap));
        GR_PrepareVDMapOfFailedDCN(srcVdMap);
    }
#endif

    /*
     * Examine each of the current targets to find the maximum
     * controller ID.
     */
    for (i = 0; i < MAX_TARGETS; ++i)
    {
        /*
         * Check if this target exists.
         */
        if (T_tgdindx[i] != NULL)
        {
            /*
             * Compare the controller ID from the low nibble of the owner.
             */
            if ((T_tgdindx[i]->prefOwner & 0xF) > maxCserial)
            {
                /*
                 * Set the maximum controller ID found.
                 */
                maxCserial = (T_tgdindx[i]->prefOwner & 0xF);
            }
        }
    }

    /*
     * Check if specified owners are valid.  Check that the
     * Vcg ID in the controller serial number is correct. Check
     * that the controller ID does not exceed the maximum configured.
     */
    if ((oldOwner >> 4) != K_ficb->vcgID ||
        (newOwner >> 4) != K_ficb->vcgID ||
        (oldOwner & 0xF) > maxCserial ||
        (newOwner & 0xF) > maxCserial)
    {
        retStatus = DEINVCTRL;
    }
    else
    {
        /*
         * Examine each of the current targets
         */
        for (i = 0; i < MAX_TARGETS; ++i)
        {
            /*
             * Check if this is the desired owner of this target
             * and if this is a port being effected.
             */
            if (T_tgdindx[i] != NULL && T_tgdindx[i]->owner == oldOwner &&
                (1 << T_tgdindx[i]->prefPort) & ports)
            {
                /*
                 * If this is a failback operation, check the new
                 * owner is the preferred owner.
                 */
                if (failBack && T_tgdindx[i]->prefOwner != newOwner)
                {
                    /*
                     * Skip this target, since on an unfail targets
                     * move back only to the preferred owner.
                     */
                    continue;
                }

                /*
                 * Update to new owner.
                 */
                T_tgdindx[i]->owner = newOwner;

                /*
                 * Update FE record.
                 */
                DEF_UpdRmtTarg(i, FALSE);

                /*
                 * Get the cluster (linked target) field.
                 */
                lnkTarget = T_tgdindx[i]->cluster;

                /*
                 * Is this target linked to another other targets?
                 */
                if (lnkTarget < MAX_TARGETS && T_tgdindx[lnkTarget] != NULL)
                {
                    /*
                     * Is this target already owned by
                     * the new controller.
                     */
                    if (T_tgdindx[lnkTarget]->owner != newOwner)
                    {
                        /*
                         * Since this target is linked to a target
                         * being moved, this target must also be moved.
                         */
                        T_tgdindx[lnkTarget]->owner = newOwner;

                        /*
                         * Update FE record.
                         */
                        DEF_UpdRmtTarg(lnkTarget, FALSE);
                    }
                }

                /*
                 * Find targets linked to this target
                 */
                for (lnkTarget = 0; lnkTarget < MAX_TARGETS; ++lnkTarget)
                {
                    /*
                     * If a target exist in this slot, check if the
                     * target is linke to the target being moved.
                     */
                    if (T_tgdindx[lnkTarget] != NULL &&
                        T_tgdindx[lnkTarget]->cluster == i)
                    {
                        /*
                         * Is this target already owned by
                         * the new controller.
                         */
                        if (T_tgdindx[lnkTarget]->owner != newOwner)
                        {
                            /*
                             * Since this target is linked to a target
                             * being moved, this target must also be moved.
                             */
                            T_tgdindx[lnkTarget]->owner = newOwner;

                            /*
                             * Update FE record.
                             */
                            DEF_UpdRmtTarg(lnkTarget, FALSE);
                        }
                    }
                }

                /*
                 * Indiate changes are made to the configuration.
                 */
                deltaFlag = TRUE;
            }
        }

        /*
         * Were any changes made?
         */
        if (deltaFlag != FALSE)
        {
            /*
             * If changes were made and this is a fail-over, invalidate
             * the mirror partner of the failing controller.
             */
            if (failBack == FALSE)
            {
                for (i = 0; i < MAX_CTRL; ++i)
                {
                    /*
                     * Is this the desired controller record?
                     */
                    if (D_mpmaps[i].source == oldOwner)
                    {
                        /*
                         * Set the mirror partnet to itself.
                         */
                        D_mpmaps[i].dest = D_mpmaps[i].source;
                    }
                }
            }

            /*
             * Update NVRAM if changes were made.
             */
            NV_P2UpdateConfig();
        }
    }
    if (failBack != TRUE)
    {
        fprintf(stderr, "<GR> - Calling GR_SetCopyMirrorInfoDCNFail...\n");
        GR_SetCopyMirrorInfoDCNFail(srcVdMap);
    }
    return (retStatus);
}

/**
******************************************************************************
**
**  @brief      Gets a list of resource that are associated with the
**              specified target.
**
**              This function return a list of either target, port number,
**              server, and LUN that are associated with the specified
**              vdisk.  The vdisk mapping ID of server records are
**              examined to determine the target association.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_GetVDiskOwner(MR_PKT *pMRP)
{
    SDD        *sdd;            /* SDD pointer to move through lists    */
    LVM        *lvm;            /* LVM pointer to move through lists    */
    UINT16      vid;            /* Virtual ID                           */
    UINT16      count = 0;      /* Count of devices found               */
    UINT16      i;              /* Loop variables                       */
    MRGETVIDOWNER_REQ *miv;     /* Pointer to input MRP                 */
    UINT16      maxCount;       /* Maximum number of devices in list    */
    MRGETVIDOWNER_RSP_INFO *pList;      /* Returned list of owners              */
    UINT8       retStatus = DEOK;       /* Return value, prep good status       */
    LVM        *lvmPtr;         /* LUN to VID map pointer and invisible */
    LVM        *ilvmPtr;        /* LUN to VID map pointer and invisible */

    /* Get pointer to Parm block address */
    miv = (MRGETVIDOWNER_REQ *) pMRP->pReq;

    /* First, grab the return data address and length allowed. */
    pList = (MRGETVIDOWNER_RSP_INFO *) (pMRP->pRsp + 1);
    pMRP->pRsp->rspLen = pMRP->rspLen;
    maxCount = ((UINT16)pMRP->rspLen - sizeof(MR_RSP_PKT)) / sizeof(MRGETVIDOWNER_RSP_INFO);

    /* Get Vdisk ID from the input packet. */
    vid = miv->id;

    /* Check for a valid Virtual Disk ID */
    if (vid >= MAX_VIRTUAL_DISKS || V_vddindx[vid] == NULL)
    {
        /* Invalid Virtual Device ID. */
        retStatus = DEINVVID;
    }
    else
    {
        /* Get the list of servers for this target. */
        for (i = 0; i < MAX_SERVERS; ++i)
        {
            /* Get a pointer to the next server record. */
            sdd = S_sddindx[i];

            /* Check if this server is associated with the specified target. */
            if (sdd != NULL)
            {
                /*
                 * Get the list of all Vdisks mapped to this server and set
                 * the indicator in the vidMapped for each Vdisk.
                 */
                lvmPtr = sdd->lvm;
                ilvmPtr = sdd->ilvm;

                while ((lvmPtr != NULL) || (ilvmPtr != NULL))
                {
                    /*
                     * Set the pointer for the one we will examine in this
                     * iteration of the loop. Also move the ptr onto the next
                     * one for the list we used.
                     */
                    if (lvmPtr != NULL)
                    {
                        lvm = lvmPtr;
                        lvmPtr = lvmPtr->nlvm;
                    }
                    else
                    {
                        lvm = ilvmPtr;
                        ilvmPtr = ilvmPtr->nlvm;
                    }

                    /* If this is a valid Vdisk ID, set the flag. */
                    if (lvm->vid == vid)
                    {
                        /*
                         * Check if the count exceeds that amount of data
                         * is allowed to be returned.
                         */
                        if (count >= maxCount)
                        {
                            /* Set the return status to "too much data". */
                            retStatus = DETOOMUCHDATA;
                        }
                        else
                        {
                            /* Store the Target in the return packet. */
                            pList[count].tid = sdd->tid;

                            /* Check if the target exists. */
                            if (T_tgdindx[sdd->tid] == NULL)
                            {
                                /* Store invalid port number in the return packet. */
                                pList[count].port = 0xFFFF;
                            }
                            else
                            {
                                /* Store preferred port number in the return packet. */
                                pList[count].port = T_tgdindx[sdd->tid]->prefPort;
                            }

                            /* Store the Server and LUN in the return packet. */
                            pList[count].sid = sdd->sid;
                            pList[count].lun = lvm->lun;
                        }

                        /* Increment the vdisk count. */
                        ++count;
                    }
                }
            }
        }
    }

    /* Store the number of devices found in the return packet. */
    pMRP->pRsp->nDevs = count;
    pMRP->pRsp->status = retStatus;

    return (retStatus);
}

/**
******************************************************************************
**
**  @brief      Gets a list of resource that are associated with the
**              specified target.
**
**              This function return a list of either target, port number,
**              server, and LUN that are associated with the specified
**              vdisk.  The vdisk mapping ID of server records are
**              examined to determine the target association.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_GetDevicePaths(MR_PKT *pMRP)
{
    UINT16      count = 0;      /* Count of devices found               */
    UINT16      i;              /* Loop variables                       */
    UINT16      j;
    UINT16      pathCount;      /* Number of paths to device            */
    PDD       **pddindx = NULL; /* Pointer to PDD array                 */
    PDD        *pdd;            /* Pointer to PDD                       */
    MRGETBEDEVPATHS_REQ *mpp;   /* Pointer to input MRP                 */
    UINT16      maxCount;       /* Maximum number of devices in list    */
    union
    {
        struct MRGETBEDEVPATHS_RSP_BIT *l0;     /* Returned list of owners  */
        struct MRGETBEDEVPATHS_RSP_ARRAY *l1;   /* Returned list of owners  */
    } pList;
    UINT32      limit = 0;      /* Maximum number of devices            */
    UINT8       retStatus = DEOK;       /* Return value, prep good status       */

    /*
     * Get pointer to Parm block address
     */
    mpp = (MRGETBEDEVPATHS_REQ *) pMRP->pReq;

    /*
     * First, grab the return data address and length allowed.
     */
    pList.l0 = (MRGETBEDEVPATHS_RSP_BIT *) (pMRP->pRsp + 1);
    pMRP->pRsp->rspLen = pMRP->rspLen;

    if (mpp->format == FORMAT_PID_BITPATH)
    {
        /*
         * Set the size of each device structure.
         */
        pMRP->pRsp->size = sizeof(MRGETBEDEVPATHS_RSP_BIT);

        /*
         * Set the maximum number of devices data can be returned for.
         */
        maxCount = ((UINT16)pMRP->rspLen -
                    sizeof(MR_RSP_PKT)) / sizeof(MRGETBEDEVPATHS_RSP_BIT);
    }
    else
    {
        /*
         * Set the size of each device structure.
         */
        pMRP->pRsp->size = sizeof(MRGETBEDEVPATHS_RSP_ARRAY);

        /*
         * Set the maximum number of devices data can be returned for.
         */
        maxCount = ((UINT16)pMRP->rspLen -
                    sizeof(MR_RSP_PKT)) / sizeof(MRGETBEDEVPATHS_RSP_ARRAY);
    }

    if (mpp->type == MWLSES)
    {
        /*
         * The enclosure list is being examined.
         */
        pddindx = E_pddindx;
        limit = MAX_DISK_BAYS;
    }
    else if (mpp->type == MWLMISC)
    {
        /*
         * The miscellaneous list is being examined.
         */
        pddindx = M_pddindx;
        limit = MAX_MISC_DEVICES;
    }
    else if (mpp->type == MWLDISK)
    {
        /*
         * The physical disk list is being examined.
         */
        pddindx = P_pddindx;
        limit = MAX_PHYSICAL_DISKS;
    }
    else
    {
        retStatus = DEINVOPT;
    }

    /*
     * Check if the list option was valid
     */
    if (retStatus != DEINVOPT)
    {
        /*
         * Walk the list of devices
         */
        for (i = 0; i < limit; ++i)
        {
            /*
             * Get the pdd of this device
             */
            pdd = pddindx[i];

            /*
             * Check if a PDD exists for this PID.
             */
            if (pdd != NULL)
            {
                if (pdd->pDev != NULL)
                {
                    /*
                     * Check if the count exceeds that amount of data is
                     * is allowed to be returned.
                     */
                    if (count >= maxCount)
                    {
                        /*
                         * Set the return status to "too much data".
                         */
                        retStatus = DETOOMUCHDATA;
                    }
                    else if (mpp->format == FORMAT_PID_BITPATH)
                    {
                        /*
                         * Set the PID in the return packet.
                         */
                        pList.l0[count].pid = i;

                        /*
                         * Check each path.
                         */
                        for (j = 0; j < MAX_PORTS; ++j)
                        {
                            /*
                             * If path exists, set the corresponding
                             * path bit in the return packet.
                             */
                            if (pdd->pDev->pLid[j] != NO_CONNECT)
                            {
                                pList.l0[count].bitPath |= (1 << j);
                            }
                        }
                    }
                    else
                    {
                        /*
                         * Set the PID in the return packet.
                         */
                        pList.l1[count].pid = i;

                        /*
                         * Clear path count so a count of paths can be taken
                         */
                        pathCount = 0;

                        /*
                         * Check each path.
                         */
                        for (j = 0; j < MAX_PORTS; ++j)
                        {
                            if (mpp->format == FORMAT_PID_LID_ARRAY)
                            {
                                /*
                                 * Copy path to return packet
                                 */
                                pList.l1[count].path[j] = pdd->pDev->pLid[j];
                            }
                            else if (pdd->pDev->pLid[j] != NO_CONNECT)
                            {
                                /*
                                 * Copy drive port to return packet
                                 */
                                pList.l1[count].path[j] = pdd->pDev->dvPort[j];
                            }
                            else
                            {
                                /*
                                 * Indicate this path not connected.
                                 */
                                pList.l1[count].path[j] = 0xFF;
                            }

                            /*
                             * Increment the count of valid paths.
                             */
                            if (pdd->pDev->pLid[j] != NO_CONNECT)
                            {
                                ++pathCount;
                            }
                        }

                        /*
                         * Store the count of valid paths.
                         */
                        pList.l1[count].pathCount = pathCount;
                    }

                    /*
                     * Increment the pdisk count.
                     */
                    ++count;
                }
            }
        }
    }

    /*
     * Store the number of devices found in the return packet.
     */
    pMRP->pRsp->nDevs = count;
    pMRP->pRsp->status = retStatus;

    return (retStatus);
}


/**
******************************************************************************
**
**  @brief      To provide a means of putting the list of
**              drives and bays supported.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_PutDeviceConfig(MR_PKT *pMRP)
{
    MRPUTDEVCONFIG_REQ *pReq = (MRPUTDEVCONFIG_REQ *) pMRP->pReq;

    pMRP->pRsp->rspLen = pMRP->rspLen;
    pMRP->pRsp->status = DEOK;

    if (SES_DevInfoMaps != NULL)
    {
        s_Free(SES_DevInfoMaps, SES_DIMEntries * sizeof(SES_DEV_INFO_MAP), __FILE__, __LINE__);
    }

    /*
     * Allocate the memory for the new list.
     */
    SES_DevInfoMaps = s_Malloc(pReq->numEntries * sizeof(SES_DEV_INFO_MAP), __FILE__, __LINE__);
    SES_DIMEntries = pReq->numEntries;

    memcpy(SES_DevInfoMaps, pReq->pEntries, pReq->numEntries * sizeof(SES_DEV_INFO_MAP));

    /*
     * Turn on the bit in the status to allow online to continue.
     */
    BIT_SET(K_ii.status, II_DEV_CONFIG);

    return (DEOK);

}

/* I have no idea why these routines have been defined the way they are. */
void _apool_init(UINT16 wh_vid)
{
    apool_init_struct(0, wh_vid);
}

void _apool_delete(UINT16 wh_vid)
{
    apool_delete(wh_vid);
}

void _alink_init(UINT16 wh_vid)
{
    alink_init(wh_vid);
}

void _alink_delete(UINT16 wh_vid)
{
    alink_delete(wh_vid);
}


/**
******************************************************************************
**
**  @brief      Save async NV to file prior to shutdown
**
**  @param      pMRP - Pointer to MRP
**
**  @return     Error code
**
******************************************************************************
**/
static UINT8    DEF_SaveAsyncNV(MR_PKT *pMRP)
{
    pMRP->pRsp->rspLen = pMRP->rspLen;
    pMRP->pRsp->status = DEOK;

    AR_SaveNVtoFile();

    return DEOK;
}


/**
******************************************************************************
**
**  @brief      To provide a common means of setting the attribute for a
**              specific virtual device.
**
**              The packet size, VID and attribute parameters are validated
**              with the new attribute being established should these checks
**              be successful.
**
**  @param      pMRP    : MRP packet
**
**  @return     UINT8 status
**
******************************************************************************
**/
UINT8 DEF_SetAttr(MR_PKT *pMRP)
{
    UINT8       retCode;
    MRSETATTR_REQ *pReq = (MRSETATTR_REQ *) pMRP->pReq;
    UINT16      reqAttrib = pReq->attr;
    UINT16      vdAttrib = 0;
    UINT16      chgAttrib = 0;
    UINT16      async_set_flag = 0;
    UINT16      snappool_set_flag = 0;
    UINT16      snappool_unset_flag = 0;
    UINT16      is_vlink = 0;

    pMRP->pRsp->rspLen = sizeof(MRSETATTR_RSP);

    if (pReq->vid >= MAX_VIRTUAL_DISKS || gVDX.vdd[pReq->vid] == NULL)
    {
        /* The vid or VDD is not valid. */
        return (DEINVVIRTID);
    }
    if (BIT_TEST(reqAttrib, VD_BASYNCH))
    {
        /*
         * Determine whether a request has come in to set the Asynch attribute.
         * If so, call the ap_check_set_unset_asynch routine to check to see if this is OK.
         */
        retCode = ap_check_set_unset_asynch(pReq->vid, 1);
        if (retCode != DEOK)
        {
            return (retCode);
        }
        async_set_flag = 1;

        /*
         * If it is vlink, check for server mappings and active copy/mirrors.
         * If there are any, restrict the user to set this attr to vlinks that
         * are related to the global apool owning target list.
         */
        retCode = apool_validate_attr(pReq->vid);
        if (retCode != DEOK)
        {
            return (retCode);
        }
    }
    if (BIT_TEST(reqAttrib, VD_BSNAPPOOL))
    {
        /*
         * Determine whether a request has come in to set the snapshot attribute.
         * If so, call the sp_check_set_unset_snappool routine to check to see if this is OK.
         */
        retCode = sp_check_set_unset_snappool(pReq->vid, 1);
        if (retCode != DEOK)
        {
            return (retCode);
        }
#if defined(MODEL_3000) || defined(MODEL_7400)
        snappool_set_flag = 1;
#endif /* MODEL_3000 || MODEL_7400 */
    }
#if defined(MODEL_3000) || defined(MODEL_7400)
    if (BIT_TEST(reqAttrib, VD_BCACHEEN))
    {
        /*
         * Caching is requested to be enabled so check if the device does
         * support it being enabled (if it is not a vlink or part of a copy
         * tree that contains a vlink?). NOTE: a copy tree that contains an ALink is OK.
         * NOTE ALSO: this logic allows a source VDisk that has BOTH synch and async
         *            destinations to be set write cacheable.
         */

        /* 4000: prevent WC if vlink anywhere in copy chain */
        vdAttrib = DEF_GetVDiskAndCopyTreeAttrs(pReq->vid);

        /*
         * Also make sure we aren't enabling wc on source of a SS
         */
        if (gVDX.vdd[pReq->vid]->vd_outssms != NULL)
        {
            snappool_set_flag = 1;
        }

        /*
         * And make sure we aren't enabling wc on a SS itself
         */

        if (gVDX.vdd[pReq->vid]->vd_incssms != NULL)
        {
            snappool_set_flag = 1;
        }
        if (BIT_TEST(vdAttrib, VD_BVLINK) || snappool_set_flag)
        {
            return (DEINVOPT);
        }
    }
#else  /* MODEL_3000 || MODEL_7400 */
    /*
     * For the 7000 don't allow the disabling of WC.
     */
    if (!BIT_TEST(reqAttrib, VD_BCACHEEN) && BIT_TEST(gVDX.vdd[pReq->vid]->attr, VD_BCACHEEN))
    {
        return DEINVOPT;
    }
#endif /* MODEL_3000 || MODEL_7400 */

    /*
     * Get the current attribute
     */
    vdAttrib = gVDX.vdd[pReq->vid]->attr;

    if (BIT_TEST(vdAttrib, VD_BVLINK))
    {
        is_vlink = 1;
    }
    else
    {
        is_vlink = 0;
    }

    /*
     * Isolate bits to change in new
     */
    reqAttrib &= VD_VDCHGMSK;

    /*
     * Isolate the bits to change in current
     */
    chgAttrib = reqAttrib ^ (vdAttrib & VD_VDCHGMSK);

    /*
     * Remove change bits from current
     */
    vdAttrib &= ~(VD_VDCHGMSK);

    /*
     * Combine bit patterns
     */
    reqAttrib = vdAttrib | reqAttrib;

    /*
     * Before we change anything, get the async logic out of the way
     * NOTE: there is a very very tiny window here where it would be
     *       cleaner if the FW actually calculated this info each time
     *       (i.e. tell FW to init alink/apool but crash before attrib
     *       is set in NVRAM)
     */
    if (BIT_TEST(chgAttrib, VD_BASYNCH))
    {
        if (async_set_flag)     /* are we setting Asynch attrib */
        {
            if (!is_vlink)      /*   creating an APool? */
            {
                /* Quick check that MM board exists and works */
                if (AR_RecoverApoolNVImage(1) == GOOD)
                {
                    _apool_init(pReq->vid);
                }
                else
                {
                    return(DEINSNVRAM);     /* Insufficient NVRAM */
                }
            }
            else
            {
                _alink_init(pReq->vid); /*   creating an ALink? */
            }
        }
        else                    /* or are we unsetting asynch attrib */
        {
            /* last minute check for OK to unset */
            retCode = ap_check_set_unset_asynch(pReq->vid, 0);
            if (retCode != DEOK)
            {
                return (retCode);       /* kick out on error */
            }
            if (!is_vlink)
            {
                _apool_delete(pReq->vid);
            }
            else
            {
                _alink_delete(pReq->vid);
            }
        }
    }

    /*
     * Now check the snappool 'unset' logic...'set' has already been checked
     */
    if (BIT_TEST(chgAttrib, VD_BSNAPPOOL))
    {
        if (!snappool_set_flag)
        {
            retCode = sp_check_set_unset_snappool(pReq->vid, 0);        /* check for OK to unset */
            if (retCode != DEOK)
            {
                return (retCode);       /* kick out on error */
            }
            snappool_unset_flag = 1;
        }
    }

    /* Update VDD attribute. i.e. set up attribute.  */
    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
    gVDX.vdd[pReq->vid]->attr = reqAttrib;

    /*
     * Observe whether cache setting bit is same in old and new
     * attribute.  If change update front end (FE) that cache is
     * changed.
     */
    if (BIT_TEST(chgAttrib, VD_BCACHEEN))
    {
        DEF_UpdRmtCacheSingle(pReq->vid, FALSE);
    }

    /*
     *  Update NVRAM
     */
    NV_P2UpdateConfig();

    /*
     * Now log any SNAPPOOL set or unset actions
     */
    if (snappool_set_flag)
    {
        logSPOOLevent(SS_BUFFER_SET, LOG_AS_INFO, 0, (pReq->vid)&1, pReq->vid);
    }
    if (snappool_unset_flag)
    {
        logSPOOLevent(SS_BUFFER_UNSET, LOG_AS_INFO, 0, (pReq->vid)&1, pReq->vid);
    }
    return (DEOK);
}


/**
******************************************************************************
**
**  @brief      To provide a means of retrieving the attributes of a virtual
**              disk and the copy tree to which it belongs.  This checks for
**              the VID or the copy tree items having cache enabled or if
**              there is a VLink.
**
**  @param      UINT16 vid - Virtual identifier to check
**
**  @return     Return virtual disk attributes set for this vid and the copy
**              tree to which it belongs.
**
******************************************************************************
**/
UINT16 DEF_GetVDiskAndCopyTreeAttrs(UINT16 vid)
{
    UINT16      retAttr = 0;
    VDD        *pVDD = gVDX.vdd[vid];
    COR        *pCOR;
    SCD        *pListSCD = NULL;
    SCD        *pSCD;
    DCD        *pDCD;
    DCD        *pListDCD = NULL;
    VDD        *pHeadVDD;

    /*
     * Combine the return bits with the attributes
     * from this virtual disk.
     */
    retAttr |= pVDD->attr;
#if defined(MODEL_3000) || defined(MODEL_7400)
    // If this is an alink then clear the vlink and alink flaga
    if (BIT_TEST(pVDD->attr, VD_BASYNCH))
    {
        BIT_CLEAR(retAttr, VD_BVLINK);
        BIT_CLEAR(retAttr, VD_BASYNCH);
    }
#endif /* MODEL_3000 || MODEL_7400 */

    /* Check if the virtual disk is the source or destination of a copy. */
    if (BIT_TEST(pVDD->attr, VD_BSCD) || BIT_TEST(pVDD->attr, VD_BDCD))
    {
        /*
         * The vdisk is a copy source or destination so the copy tree
         * needs to be checked for VLinks.
         */

        /* Increment the pass key for this loop */
        d_resync_paskey++;

        /*
         * Save the starting VDD so we can recognize if we loop through
         * a circular copy tree.
         */
        pHeadVDD = pVDD;

        /*
         * Loop on the DCDs to find the top of the copy tree for which this
         * virtual disk is a destination.  If the copy tree is a circular
         * tree it will be recognized by the fact that the "passkey" value
         * will be the current value from d_resync_paskey in one of the DCDs.
         * If the copy is not a circular tree then we will find a VDD that
         * does not have a DCD pointer.
         *
         * The output from this loop will be that the pVDD value points
         * to a VDD which is the head of the copy tree or a part of the
         * circular portion of a copy tree.
         */
        while (pVDD != NULL && pVDD->pDCD != NULL)
        {
            /*
             * Save the starting VDD so we can recognize if we loop through
             * a circular copy tree.
             */
            pHeadVDD = pVDD;

            /* Get local copies of the DCD, COR and source VDD.  */
            pDCD = pVDD->pDCD;
            pCOR = pDCD->cor;
            pVDD = pCOR->srcvdd;

            /*
             * If this is the first item on the list, setup the DCD List
             * pointer so we have a starting point.
             *
             * If this DCD does not have the current passkey set it up to show
             * that we have already seen this DCD.  This helps in recognizing
             * circular copy trees.
             *
             * If the DCD does have the current passkey it means we have a
             * circular copy tree and have looped back to something we already
             * processed.
             */
            if (pListDCD == NULL)
            {
                pListDCD = pDCD;
                pCOR->r5PasKey = d_resync_paskey;
            }
            else if (pCOR->r5PasKey != d_resync_paskey)
            {
                pCOR->r5PasKey = d_resync_paskey;
                pDCD->rlink = pListDCD;
                pListDCD = pDCD;
            }
            else
            {
                break;
            }
        }

        /*
         * If we exit the above loop with a valid VDD we ran into the case
         * where there was a circular list or we found the top of the
         * copy tree and had no more DCDs to follow.  In that case we want
         * to save the current VDD as the new head VDD.
         *
         * If we exit the above loop with a NULL VDD it means we found a
         * DCD without a source VDD (which is most likely the storage side
         * of a vlink and it is the destination of a copy).  In that case
         * we want to back up to the previous head VDD.
         */
        if (pVDD != NULL)
        {
            /* Save the current VDD as the new HEAD.  */
            pHeadVDD = pVDD;
        }
        else
        {
            /* Make sure the VDD pointer is back at the head VDD. */
            pVDD = pHeadVDD;
        }

        /* Loop through and remove the rlinks. */
        while (pListDCD != NULL)
        {
            pDCD = pListDCD;
            pListDCD = (DCD *) pDCD->rlink;
            pDCD->rlink = NULL;
        }

        /*
         * Combine the return bits with the attributes
         * from this virtual disk.
         */

#if defined(MODEL_3000) || defined(MODEL_7400)
        // If this is an alink then clear the vlink and the alink flags
        if (BIT_TEST(pVDD->attr, VD_BASYNCH) && !BIT_TEST(retAttr, VD_BVLINK))
        {
            retAttr |= pVDD->attr;
            BIT_CLEAR(retAttr, VD_BVLINK);
            BIT_CLEAR(retAttr, VD_BASYNCH);
        }
        else
        {
            retAttr |= pVDD->attr;
        }
#else  /* MODEL_3000 || MODEL_7400 */
        retAttr |= pVDD->attr;
#endif /* MODEL_3000 || MODEL_7400 */

        /*
         * Given the VDD, get the start of the SCD list.  This will be
         * our starting point for copy tree interrogation.
         *
         * If we have an SCD list then make sure its "rlink" has been set
         * to NULL to indicate that it is not linked to anything (this is
         * just in case something existed in the link before this routine
         * was called).
         */
        pListSCD = (SCD *)pVDD->pSCDHead;

        if (pListSCD != NULL)
        {
            pListSCD->rlink = NULL;
        }

        /*
         * While we have items on our SCD list we will continue looping and
         * investigating them.  The code below will add items to this list
         * as it sees copy trees that need to be interrogated.  This logic
         * was taken from the assembly code in "d$updvidfeat" and modified
         * to suit this purpose.
         */
        while (pListSCD != NULL)
        {
            /*
             * Pop the first SCD off the list and update the head of the
             * list.
             */
            pSCD = pListSCD;
            pListSCD = pListSCD->rlink;
            pSCD->rlink = NULL;

            /* Continue looping while we have a current SCD to process. */
            while (pSCD != NULL)
            {
                /* Get the COR and destination VDD for this SCD. */
                pCOR = pSCD->cor;
                pVDD = pCOR->destvdd;

                /*
                 * If for some reason we encounter an SCD that has a COR
                 * but its destination VDD is NULL we will just continue
                 * to the next SCD in the current list via the "link".
                 */
                if (pVDD == NULL)
                {
                    pSCD = pSCD->link;
                    continue;
                }

                /*
                 * Check if we have wrapped back to the starting VDD, if
                 * so we can exit out.
                 */
                if (pVDD == pHeadVDD)
                {
                    break;
                }

                /*
                 * Combine the return bits with the attributes
                 * from this virtual disk.
                 */
#if defined(MODEL_3000) || defined(MODEL_7400)
                // If this is an alink then clear the vlink and the alink flags
                if (BIT_TEST(pVDD->attr, VD_BASYNCH) && !BIT_TEST(retAttr, VD_BVLINK))
                {
                    retAttr |= pVDD->attr;
                    BIT_CLEAR(retAttr, VD_BVLINK);
                    BIT_CLEAR(retAttr, VD_BASYNCH);
                }
                else
                {
                    retAttr |= pVDD->attr;
                }
#else  /* MODEL_3000 || MODEL_7400 */
                retAttr |= pVDD->attr;
#endif /* MODEL_3000 || MODEL_7400 */

                /*
                 * If this VDD is not the source of a copy then continue
                 * looping on the list of SCDs we are already checking.
                 *
                 * In this case we will just set our current SCD pointer
                 * to the next one in the current list via the "link" field.
                 * If that happens to be NULL it means we have completed this
                 * set of linked SCD and we will then look at the over all
                 * SCD list to see if there is more to process.
                 */
                if (pVDD->pSCDHead == NULL)
                {
                    pSCD = pSCD->link;
                    continue;
                }

                /*
                 * If this SCD has multiple destinations or links then we
                 * will add the link to the overall list of SCDs that need
                 * to be processed as we will not process it directly here
                 * and process that link as a tree of its own.  We do this
                 * by adding it to the SCD list and updating the "rlink"
                 * values so we have a linked list.
                 */
                if (pSCD->link != NULL)
                {
                    pSCD->link->rlink = pListSCD;
                    pListSCD = pSCD->link;
                }

                /*
                 * Update our current SCD to point to the current VDDs
                 * head SCD pointer.
                 */
                pSCD = pVDD->pSCDHead;
            }
        }
    }

    /* Return the attributes found. */
    return (retAttr);
}

/**
******************************************************************************
**
**  @brief      To provide a means of checking to see if it is OK to change the
**              asynch bit on either a VDisk or VLink. If it is OK to do so,
**              this logic will also call the appropriate logic in the asynch
**              code that needs to know about the change and then return TRUE
**              to the calling function.
**
**  @param      UINT16 vid - Virtual identifier to check
**              int oper   - set=1, unset=0
**
**  @return     Return DEOK if OK to set/unset attribute, error code otherwise
**
******************************************************************************
**/
UINT8 ap_check_set_unset_asynch(UINT16 vid, int oper)
{
    UINT16      index_D;
    VDD        *pVDD = gVDX.vdd[vid];
    VDD        *ptVDD;
    RDD        *pRDD;           /* Pointer to a Raid Entry */
    UINT16      apool_exists = 0;
    int         alink_count = 0;

    /* First, check to see if the vid is valid */
    if (pVDD == NULL)
    {
        return DEINOPDEV;
    }

    /* Now check to see if any servers are mapped to this VDisk */
    if (DEF_CheckForMapped(vid) != 0xffff)
    {
        return DEDEVUSED;
    }

    /* Now make sure we aren't making an apool out of a snappool */
    if (BIT_TEST(pVDD->attr, VD_BSNAPPOOL))
    {
        return DEDEVUSED;
    }

    /* Now check to see if this VDisk is part of any copy operations */
    if (BIT_TEST(pVDD->attr, VD_BSCD) || BIT_TEST(pVDD->attr, VD_BDCD))
    {
        return DEOUTOPS;
    }

    /* Now check to see if there already is an APool */
    apool_exists = 0;
    for (index_D = 0; index_D < MAX_VIRTUAL_DISKS; index_D++)
    {
        ptVDD = gVDX.vdd[index_D];
        if (ptVDD != NULL)
        {
            if (!BIT_TEST(ptVDD->attr, VD_BVLINK))
            {
                if (ptVDD->pVLinks == NULL)
                {
                    if ((ptVDD->attr) & VD_ASYNCH)
                    {
                        apool_exists = 1;
                    }
                }
            }
            else
            {
                if ((ptVDD->attr) & VD_ASYNCH)
                {
                    alink_count++;
                }
            }
        }
    }

    /* Now, determine if it is a VDisk or a VLink that we are dealing with */
    /* If we are a VDisk, then kick out if apool already exists */
    if (!BIT_TEST(pVDD->attr, VD_BVLINK))   /* we are a VDisk/APool */
    {
        if (oper)           /* are we setting asycn? */
        {
            if (apool_exists)
            {
                return DEINVOP;        /* make sure we are the only apool */
            }
            pRDD = pVDD->pRDD;
            if (pRDD)
            {
                if (pRDD->type == RD_RAID5)     /* and we must not be a raid 5 apool */
                {
                    return DEINVRTYPE;
                }
            }
        }
        else
        {                   /* unsetting the APool */
            if (alink_count)
            {
                return DEDEVUSED;      /* don't allow if alinks exist */
            }
        }
    }
    else                    /* we are a VLink/ALink */
    {
        if (oper)           /* are we setting asycn? */
        {
            if (!apool_exists)
            {
                return DEINVOP;        /* must have an apool before allowing ALinks */
            }
            if (alink_count >= MAX_ALINKS)
            {
                return DEINSTABLE;     /* Limit to 47 (MAX_ALINKS) allowed. */
            }
        }
    }
    return DEOK;         /* return overall status */
}

/**
******************************************************************************
**
**  @brief      To provide a means of checking to see if it is OK to change the
**              slink (snapshot buffer) bit on a VDisk. If it is OK to do so,
**              this logic will also call the appropriate logic in the asynch
**              code that needs to know about the change and then return TRUE
**              to the calling function.
**
**  @param      UINT16 vid - Virtual identifier to check
**              int oper   - set=1, unset=0
**
**  @return     Return DEOK if OK to set/unset attribute, error code otherwise
**
******************************************************************************
**/
UINT8 sp_check_set_unset_snappool(UINT16 vid, int oper)
{
    UINT16      index_D;
    VDD        *pVDD = gVDX.vdd[vid];
    VDD        *ptVDD;
    RDD        *pRDD;           /* Pointer to a Raid Entry */
    UINT16      spool_exists = 0;
    int         slink_count[2] = { 0, 0 };
    int         this_spool_owner;
    int         owner_of_this_snapshot = 0;

    /* First, check to see if the vid is valid */
    if (pVDD == NULL)
    {
        return (DEINOPDEV);
    }

    /* Now check to see if any servers are mapped to this VDisk */
    if (DEF_CheckForMapped(vid) != 0xffff)
    {
        return (DEDEVUSED);
    }

    /* Now check to see if this vdisk is already an apool or a vlink */
    if (BIT_TEST(pVDD->attr, VD_BASYNCH) || BIT_TEST(pVDD->attr, VD_BVLINK))
    {
        return (DEDEVUSED);
    }

    /* Now check to see if this VDisk is part of any copy operations */
    if (BIT_TEST(pVDD->attr, VD_BSCD) || BIT_TEST(pVDD->attr, VD_BDCD))
    {
        return (DEOUTOPS);
    }

    if (pVDD->status == VD_INOP)
    {
        return (DEINOPDEV);
    }

    for (pRDD = pVDD->pRDD; pRDD != NULL; pRDD = pRDD->pNRDD)
    {
        if(pRDD->status == RD_UNINIT || pRDD->status == RD_INIT)
        {
            return (DEINITINPROG);
        }
    }

    /* Check if snappool would be greater than or equal to 64TB. */
    if (pVDD->devCap >= (64ULL*1024*1024*1024*1024))
    {
        return (DE64TBLIMIT);
    }

    /* Now check to see if there already is a SnapPool */
    spool_exists = 0;
    this_spool_owner = vid & 0x1;
    for (index_D = 0; index_D < MAX_VIRTUAL_DISKS; index_D++)
    {
        ptVDD = gVDX.vdd[index_D];
        if (ptVDD != NULL)
        {
            if ((ptVDD->attr) & 0x2000 && this_spool_owner == (index_D & 0x1))
            {
                spool_exists = 1;
            }
            else
            {
                /* check to see if it is a snapshot */
                pRDD = ptVDD->pRDD;
                if (pRDD)
                {
                    if (pRDD->type == RD_SLINKDEV)  /*if a snapshot inc cnt */
                    {
                        owner_of_this_snapshot = pRDD->depth;       // was... find_owning_dcn(index_D);
                        if (owner_of_this_snapshot != 0xffff)
                        {
                            slink_count[owner_of_this_snapshot]++;
                        }
                    }
                }
            }
        }
    }

    /* Now, prevent multiple snappools OR removal of snappool when snapshots exist */
    if (oper)                   /* are we setting snappool? */
    {
        if (spool_exists)
        {
            return (DEINVOP);   /* already have an apool */
        }
        else                    /* make sure the vdisk is >=10GB in size */
        {
            if (pVDD->devCap < 20971520LLU) // 10gb or less invalid.
            {
                return (DEINSDEVCAP);
            }
        }
    }
    else
    {                           /* unsetting the snappool */
        if (slink_count[this_spool_owner])
        {
            return (DEDEVUSED); /* don't allow if slinks exist */
        }
    }

    if (oper)                   /* Write magic number in header nv if it is setting spool */
    {
        gss_version[vid & 1] = 0;       /* No current "version" of snappool. */
        update_header_nv_magic(vid);    /* Write the magic word 0xbf in the snappool header nv area */
    }
    return (DEOK);              /* return overall status */
}


/**
******************************************************************************
**
**  @brief      To provide a means of retrieving the list of
**              mirror partners which are defined.
**
**              This function retrieves the list of mirror partners.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_GetMirrorPartnerList(MR_PKT *pMRP)
{
    MRGETMPLIST_RSP_INFO *pList;        /* pointer to returned list             */
    UINT16      count = 0;      /* number of ports                      */
    UINT16      maxCount;       /* Maximum number of devices in list    */
    UINT32      i;
    UINT8       retStatus = DEOK;       /* Return value, prep good status       */

    /*
     * First, grab the return data address and length allowed.
     */
    pList = (MRGETMPLIST_RSP_INFO *) (pMRP->pRsp + 1);
    pMRP->pRsp->rspLen = sizeof(MR_RSP_PKT);
    maxCount = ((UINT16)pMRP->rspLen - sizeof(MR_RSP_PKT)) / sizeof(MRGETMPLIST_RSP_INFO);

    /*
     * Check that the starting controller ID is valid.
     */
    if (((MR_DEVID_REQ *) pMRP->pReq)->id >= MAX_CTRL)
    {
        retStatus = DEINVCTRL;
    }
    else
    {
        /*
         * Walk the list of mirror partners and copy
         * to the output MRP
         */
        for (i = ((MR_DEVID_REQ *) pMRP->pReq)->id; i < MAX_CTRL; ++i)
        {
            /*
             * Is there a controller record at this index?
             */
            if (D_mpmaps[i].source != 0)
            {
                /*
                 * Check if the count exceeds that amount of data is
                 * is allowed to be returned.
                 */
                if (count >= maxCount)
                {
                    /*
                     * Set the return status to "too much data".
                     */
                    retStatus = DETOOMUCHDATA;
                }
                else
                {
                    /*
                     * Fill in the return structure with the
                     * mirror partner information
                     */
                    pList[count].source = D_mpmaps[i].source;
                    pList[count].dest = D_mpmaps[i].dest;

                    /*
                     * Increment the size of the return data.
                     */
                    pMRP->pRsp->rspLen += sizeof(MRGETMPLIST_RSP_INFO);
                }

                /*
                 * Increment the count of mirror partners records.
                 */
                ++count;
            }
        }
    }

    /*
     * Store the number of devices found in the return packet.
     */
    pMRP->pRsp->nDevs = count;
    pMRP->pRsp->status = retStatus;

    return retStatus;
}

/**
******************************************************************************
**
**  @brief      Generic MRP for debug.
**
**              This function is for debug only.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_DegradePort(MR_PKT *pMRP)
{
    UINT8       retStatus = DEOK;
    UINT8       port;
    MRFAILPORT_REQ *mfp;

    /*
     * Set the return data length.
     */
    pMRP->pRsp->rspLen = pMRP->rspLen;

    /*
     * Get pointer to Parm block address
     */
    mfp = (MRFAILPORT_REQ *) pMRP->pReq;

    /*
     * Get the port number from the input packet.
     */
    port = mfp->port;

    /*
     * Check if the port is valid.
     */
    if (port >= MAX_PORTS || P_chn_ind[port] == NULL)
    {
        /*
         * Invalid port.
         */
        retStatus = DEINVCHAN;
    }
    else
    {
        /*
         * Is the port being set degraded or good?
         */
        if (mfp->fail == FALSE)
        {
            P_chn_ind[port]->degraded = FALSE;
        }
        else
        {
            P_chn_ind[port]->degraded = TRUE;
        }

        /*
         * Rebalance the devices among the ports.
         */
        FAB_BalanceLoad();
    }

    return retStatus;
}


/**
******************************************************************************
**
**  @brief      To provide a standard means of creating a new server
**              definition.  This is initiated by the mrcreateserver MRP.
**
**              This function will allocate the space for a new server and
**              register it into the SDX table.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_CreateServer(MR_PKT *pMRP)
{
    UINT32      sid;
    UINT32      linkedSID;
    SDD        *sdd;
    LVM        *lvm;
    UINT16      targetId;
    MRCREATESERVER_REQ *mms;
    MRCREATESERVER_RSP *mmsr;
    UINT8       retStatus = DEOK;

    /*
     * Get pointer to Parm block address
     */
    mms = (MRCREATESERVER_REQ *) pMRP->pReq;
    mmsr = (MRCREATESERVER_RSP *) pMRP->pRsp;

    /*
     * Initialize flag for server does not exist.
     */
    mmsr->flags = FALSE;

    /*
     * Get the Target ID from the MRP
     */
    targetId = mms->targetId;

    /*
     * Check that the specified target record exists.
     */
    if (targetId >= MAX_TARGETS || T_tgdindx[targetId] == NULL)
    {
        /*
         * Invalid target ID.
         */
        retStatus = DEINVTID;
    }
    else
    {
        /*
         * Attempt to find this server by performing a WWN lookup.
         */
        sid = DEF_WWNLookup(mms->wwn, targetId, TRUE, mms->i_name);

        /*
         * Check if the server record already exists.
         */
        if (sid != 0xFFFFFFFF)
        {
            /*
             * Set flag for server already exists.
             */
            mmsr->flags = TRUE;
        }
        else
        {
            /*
             * Find an available server ID.  If none exist, return error.
             */
            for (sid = 0; sid < MAX_SERVERS; ++sid)
            {
                if (S_sddindx[sid] == NULL)
                {
                    /*
                     * Available SSD slot found.
                     */
                    break;
                }
            }

            /*
             * Was an available SSD slot found?
             */
            if (sid >= MAX_SERVERS)
            {
                sid = 0xFFFF;
                retStatus = DEINSTABLE;
            }
            else
            {
                /*
                 * Allocate a server structure
                 */
                S_sddindx[sid] = sdd = DEF_AllocServer();

                /*
                 * Increment the server count.
                 */
                /* ++((UINT32) (S_sddindx[-1])); */
                S_sddindx[-1] = (SDD *) ((UINT32)(S_sddindx[-1]) + 1);

                /*
                 * Fill in the structure
                 */
                sdd->sid = sid;
                sdd->tid = targetId;
                sdd->owner = mms->owner;

                /*
                 * Put server ID into return parms
                 */
                mmsr->sid = sid;
                mmsr->flags = FALSE;

                /*
                 * Check if this is a SANlinks XIOtech Controller.
                 */
                if (M_chk4XIO(mms->wwn) != 0)
                {
                    /*
                     * Check for back end XIOtech Controllers.  The
                     * XIO bit is not set for front end XIO Controllers.
                     */
                    if (bswap_32((UINT32)(mms->wwn & 0xFFFFF0FF)) != WWN_B_PORT)
                    {
                        /*
                         * Set XIOtech controller bit.
                         */
                        S_sddindx[sid]->attrib = (1 << SD_XIO);
                    }

                    /*
                     * If the WWN indicates a XIOtech controller, ignore the
                     * nibble of the WWN where the port number is stored and
                     * and nibble where the controller ID is store (the least
                     * significant digit of the serial number).
                     */
                    sdd->wwn = mms->wwn & 0xF0FFFFFFFFFFF0FFLL;
                }
                else
                {
                    /*
                     * Store the server World Wide Port name.
                     */
                    sdd->wwn = mms->wwn;
                }
                if ((T_tgdindx[targetId] != NULL)
                    && BIT_TEST(T_tgdindx[targetId]->opt, TARGET_ISCSI))
                {
                    memcpy(sdd->i_name, mms->i_name, 254);
                }

                /*
                 * Check if a clustered target record exists.
                 */
                if (T_tgdindx[targetId] &&
                    (targetId  = T_tgdindx[targetId]->cluster) < MAX_TARGETS &&
                    T_tgdindx[targetId] != NULL)
                {
                    /*
                     * Determine if the server has already created
                     * a SDD on another target.
                     */
                    linkedSID = DEF_WWNLookup(mms->wwn, targetId, TRUE, mms->i_name);
                }
                else
                {
                    /*
                     * Initialize for linked SDD not found.
                     */
                    linkedSID = 0xFFFFFFFF;

                    /*
                     * Find targets linked to this target
                     */
                    for (targetId = 0; targetId < MAX_TARGETS; ++targetId)
                    {
                        /*
                         * If a target exist in this slot, check if the
                         * target is linked to the target being moved.
                         */
                        if (T_tgdindx[targetId] != NULL &&
                            T_tgdindx[targetId]->cluster == sdd->tid)
                        {
                            /*
                             * Determine if the server has already created
                             * a SDD on another target.
                             */
                            linkedSID = DEF_WWNLookup(mms->wwn, targetId, TRUE, mms->i_name);

                            /*
                             * Was a SDDs on another target found?
                             */
                            if (linkedSID != 0xFFFFFFFF)
                            {
                                break;
                            }
                        }
                    }
                }

                /*
                 * Was a SDDs with this WWN found on another target?
                 */
#if ISCSI_CODE
                if ((linkedSID != 0xFFFFFFFF) &&
                    T_tgdindx[S_sddindx[linkedSID]->tid] &&
                    T_tgdindx[sdd->tid] &&
                    (BIT_TEST(T_tgdindx[S_sddindx[linkedSID]->tid]->opt, TARGET_ISCSI)
                        == BIT_TEST(T_tgdindx[sdd->tid]->opt, TARGET_ISCSI)))
#else
                if (linkedSID != 0xFFFFFFFF)
#endif
                {
                    /*
                     * Is this SDD current linked?
                     */
                    if (S_sddindx[linkedSID]->linkedSID == 0xFFFF)
                    {
                        /*
                         * Indicate a Linked SID
                         */
                        sdd->linkedSID = linkedSID;
                    }
                    else
                    {
                        /*
                         * Indicate a Linked SID
                         */
                        sdd->linkedSID = S_sddindx[linkedSID]->linkedSID;
                    }

                    /*
                     * Indicate a Linked SDD
                     */
                    S_sddindx[linkedSID]->linkedSID = sid;

                    /*
                     * Update server record.
                     * Set SID for input parm, False = do not delete.
                     */
                    DEF_UpdRmtServer(linkedSID, FALSE);

                    /*
                     * Clear the LUN mapping from the linked SDD (will be
                     * updated by the DEF_HashLUN if there are any LUN Mappings)
                     */
                    sdd->numLuns = 0;

                    lvm = S_sddindx[linkedSID]->lvm;

                    /*
                     * Process all LVM in the list.  A NULL next pointer
                     * inidcates the end of the list.
                     */
                    while (lvm != NULL)
                    {
                        /*
                         * Get the LUN and put it into the SDD hash table.
                         */
                        DEF_HashLUN(sdd, lvm->lun, lvm->vid);

                        /*
                         * Advance to next LVM
                         */
                        lvm = lvm->nlvm;
                    }

                    lvm = S_sddindx[linkedSID]->ilvm;

                    while (lvm != NULL)
                    {
                        /*
                         * Get the LUN and put it into the SDD hash table.
                         */
                        DEF_HashLUN(sdd, lvm->lun, lvm->vid);

                        /*
                         * Advance to next LVM
                         */
                        lvm = lvm->nlvm;
                    }
                }
                else
                {
                    /*
                     * This SDD is not linked to any other SDDs.
                     */
                    S_sddindx[sid]->linkedSID = 0xFFFF;
                }

                /*
                 * Update NVRAM part II.
                 */
                NV_P2UpdateConfig();

                /*
                 * Update server record.
                 * Set SID for input parm, False = do not delete.
                 */
                DEF_UpdRmtServer(sid, FALSE);

                /*
                 * Indicate that the update occurred
                 */
                DEF_SignalServerUpdate();
            }
        }

        /*
         * Put server ID into return parms
         */
        mmsr->sid = sid;
    }

    return retStatus;
}

/**
******************************************************************************
**
**  @brief      To delete all the specified server record.
**
**              The memory for the specified SDD record is freed.
**              The SDD table is updated
**
**  @param      sid - server ID.
**
**  @return     none
**
******************************************************************************
**/
void DEF_DeleteSDD(UINT32 sid)
{
    /*
     * Release memory used by SDD
     */
    s_Free(S_sddindx[sid], sizeof(SDD), __FILE__, __LINE__);

    /*
     * NULL the SDD pointer in SDX
     */
    S_sddindx[sid] = NULL;

    /*
     * Decrement the server count.
     */
    /* --((UINT32) (S_sddindx[-1])); */
    S_sddindx[-1] = (SDD *) ((UINT32)(S_sddindx[-1]) - 1);
}

/**
******************************************************************************
**
**  @brief      Delete a server definition.
**              This is initiated by the mrdeleteserver MRP.
**
**              This function will deallocate the space used for a server and
**              remove it from the SDX table.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_DeleteServer(MR_PKT *pMRP)
{
    UINT32      sid;
    UINT32      dsid;
    UINT32      linkedSID;
    SDD        *sdd;
    MRDELETESERVER_REQ *mds;
    UINT8       retStatus = DEOK;
    UINT32      count = 0;
    UINT64      wwn;

    /*
     * Get pointer to Parm block address
     */
    mds = (MRDELETESERVER_REQ *) pMRP->pReq;

    /*
     * Get server ID from the MRP
     */
    sid = mds->id;

    /*
     * Check that the specified server record exists.
     */
    if (sid >= MAX_SERVERS || S_sddindx[sid] == NULL)
    {
        retStatus = DEINVSID;
    }
    /*
     * Check that the specified option is valid.
     */
    else if (mds->option > DS_LINKS)
    {
        retStatus = DEINVOPT;
    }
    else
    {
        /*
         * Get a pointer to the SDD.
         */
        sdd = S_sddindx[sid];
        wwn = sdd->wwn;

        /*
         * Get the first linked SDD.
         */
        linkedSID = sdd->linkedSID;

        /*
         * If Remove Links option, count the number of SDD in
         * the linked list with a different WWN.
         */
        if (mds->option == DS_LINKS && linkedSID != 0xFFFF)
        {
            while (sid != linkedSID)
            {
                /*
                 * Check if the WWN is different.
                 */
                if (wwn != S_sddindx[linkedSID]->wwn)
                {
                    /*
                     * Increment the count of server records
                     * with different WWN.
                     */
                    ++count;
                }

                /*
                 * Advance to the next SDD
                 */
                linkedSID = S_sddindx[linkedSID]->linkedSID;
            }
        }

        /*
         * A SDD with different WWN must exist for the 'Remove Links' option.
         */
        if (mds->option == DS_LINKS && count == 0)
        {
            retStatus = DEINVOPT;
        }
        else
        {
            /*
             * Delete all the LVM for this SDD.
             */
            DL_DeleteLVM(sdd);

            /*
             * Get the first linked SDD.
             */
            linkedSID = sdd->linkedSID;

            /*
             * Are any SDD linked to this SDD?
             */
            if (linkedSID != 0xFFFF)
            {
                if (mds->option == DS_MAPPINGS ||
                    mds->option == DS_DELETEALL ||
                    mds->option == DS_LINKS)
                {
                    if (mds->option == DS_LINKS)
                    {
                        /*
                         * Remove this link.
                         */
                        DL_removeLnk(sdd);
                    }

                    /*
                     * Walk the linked SDD list and process each SDD.
                     */
                    while (linkedSID != sid)
                    {
                        /*
                         * Get the Server ID and the pointer to the SDD.
                         */
                        dsid = linkedSID;
                        sdd = S_sddindx[dsid];

                        /*
                         * Get next SDD in list prior to deleting the
                         * current SDD.
                         */
                        linkedSID = sdd->linkedSID;

                        if (mds->option != DS_LINKS || sdd->wwn == wwn)
                        {
                            /*
                             * Delete all the LVM for this SDD.
                             */
                            DL_DeleteLVM(sdd);

                            if (mds->option == DS_DELETEALL)
                            {
                                /*
                                 * Release memory used by SDD and delete SDD.
                                 */
                                DEF_DeleteSDD(dsid);

                                /*
                                 * Update server record.
                                 * Set SID for input parm, True = delete.
                                 */
                                DEF_UpdRmtServer(dsid, TRUE);
                            }
                            else
                            {
                                if (mds->option == DS_LINKS)
                                {
                                    /*
                                     * Remove this link.
                                     */
                                    DL_removeLnk(sdd);

                                    /*
                                     * Add back into linked list of SDD
                                     * removed from linked list containing
                                     * the SDD specified in the MRP.
                                     */
                                    DL_insertLnk(sid, dsid);
                                }

                                /*
                                 * Update server record.
                                 * Set SID for input parm, False = do not delete.
                                 */
                                DEF_UpdRmtServer(dsid, FALSE);
                            }
                        }
                        else
                        {
                            /*
                             * Check and then decrement count.  This
                             * assure we scan all SDD in the linked list.
                             */
                            if (count-- == 0)
                            {
                                break;
                            }
                        }
                    }
                }
                else
                {
                    /*
                     * Get a pointer to the SDD.
                     */
                    sdd = S_sddindx[linkedSID];

                    /*
                     * Advance to next SDD in list.
                     */
                    linkedSID = sdd->linkedSID;

                    /*
                     * Were there only two SDDs in the list?
                     *
                     */
                    if (linkedSID == sid)
                    {
                        /*
                         * Now there is only one SDD in the list.
                         */
                        sdd->linkedSID = 0xFFFF;
                    }
                    else
                    {
                        /*
                         * Walk the linked SDD list until we arrive
                         * at the SSD that points to the sdd being delted.
                         */
                        while (linkedSID != sid)
                        {
                            /*
                             * Advance to next SDD in list.
                             */
                            sdd = S_sddindx[linkedSID];
                            linkedSID = sdd->linkedSID;
                        }

                        /*
                         * Remove the SSD from the linked list
                         * by pointing around the SDD.
                         */
                        sdd->linkedSID = S_sddindx[sid]->linkedSID;
                    }

                    /*
                     * Update server record.
                     * Set SID for input parm, False = do not delete.
                     */
                    DEF_UpdRmtServer(sdd->sid, FALSE);
                }
            }

            if (mds->option == DS_DELETE || mds->option == DS_DELETEALL)
            {
                /*
                 * Release memory used by SDD and delete SDD.
                 */
                DEF_DeleteSDD(sid);

                /*
                 * Update server record.
                 * Set SID for input parm, True = delete.
                 */
                DEF_UpdRmtServer(sid, TRUE);
            }
            else
            {
                /*
                 * Update server record.
                 * Set SID for input parm, False = do not delete.
                 */
                DEF_UpdRmtServer(sid, FALSE);
            }

            /*
             * Indicate that the update occurred
             */
            DEF_RemoveSrvKeys(sid);
            DEF_SignalServerUpdate();

            /*
             * Update NVRAM part II.
             */
            NV_P2UpdateConfig();
        }
    }

    return retStatus;
}

/**
******************************************************************************
**
**  @brief      Generic MRP for debug.
**
**              This function is for debug only.
**              There is no code in this function - it is provided as a
**              spot to put debug code.
**
**  @param      pMRP - MRP structure
**
**  @return     DEOK (always)
**
******************************************************************************
**/
UINT8 DEF_Generic(MR_PKT *pMRP UNUSED)
{
    /* Call your favorite function */
    return DEOK;
}

/**
******************************************************************************
**
**  @brief      Reset configuration.
**
**  @param      None
**
**  @return     None
**
******************************************************************************
**/
void DEF_ResetConfig(void)
{
    UINT16      index_D;
    SDD        *pSDD;
    VDD        *pVDD;
    RDD        *pRDD;
    PDD        *pPDD;
    TGD        *pTGD;
    VLAR       *pVLAR;
    COR        *pCOR;
    UINT8       checkCount;

    /* Turn off scrubbing.  */
    gScrubOpt = FALSE;
    K_ii.scrub = FALSE;

    DEF_TerminateBackground();

    /* Turn off WHQL compliance */
    NV_scsi_whql = FALSE;

    /* Kill all defrags.  These are only verification steps. */
    DF_CancelDefrag();
    DF_ClearLastResp();

    /* If there are copies in progress, terminate them. */
    while (CM_cor_act_que != NULL)
    {
        for (pCOR = CM_cor_act_que; pCOR != NULL; pCOR = pCOR->link)
        {
// NOTE: get_ilt can task switch, thus above loop is technically wrong.
            CCSM_term_copy(pCOR);
        }

        /*
         * Now sleep for a 125ms interval to allow the copies to terminate.
         * Do this for up to 4 attempts (1/2 second total) if the copies
         * have not stopped.  After the timeout, retry the terminates if
         * they are still alive.
         */
        for (checkCount = 0; (checkCount < 4) && (CM_cor_act_que != NULL); ++checkCount)
        {
            TaskSleepMS(125);
        }
    }

    /* Delete all server records. */
    for (index_D = 0; index_D < MAX_SERVERS; index_D++)
    {
        pSDD = gSDX.sdd[index_D];

        if (pSDD != NULL)
        {
            gSDX.count--;
            gSDX.sdd[index_D] = NULL;
            DEF_RelSDDLVM(pSDD);
            DEF_UpdRmtServer(index_D, TRUE);
        }
    }

    /* Delete all targets. */
    for (index_D = 0; index_D < MAX_TARGETS; index_D++)
    {
        pTGD = gTDX.tgd[index_D];

        if (pTGD != NULL)
        {
            gTDX.count--;
            gTDX.tgd[index_D] = NULL;
            s_Free(pTGD, sizeof(TGD), __FILE__, __LINE__);
            DEF_UpdRmtTarg(index_D, TRUE);
        }
    }

    /* Delete all virtual devices. */
    for (index_D = 0; index_D < MAX_VIRTUAL_DISKS; index_D++)
    {
        pVDD = gVDX.vdd[index_D];

        if (pVDD != NULL)
        {
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            gVDX.count--;
            gVDX.vdd[index_D] = NULL;

            /* Check for VLinks. */
            while (pVDD->pVLinks != NULL)
            {
                pVLAR = pVDD->pVLinks->link;
#ifdef M4_DEBUG_VLAR
fprintf(stderr, "put_vlar 0x%08x\n", (UINT32)pVDD->pVLinks);
#endif  /* M4_DEBUG_VLAR */
                put_vlar(pVDD->pVLinks);
                pVDD->pVLinks = pVLAR;
            }

            s_Free(pVDD, sizeof(VDD), __FILE__, __LINE__);
            DEF_UpdRmtCacheSingle(index_D, TRUE);
        }
    }

    /* Clear the RAID records. */
    for (index_D = 0; index_D < MAX_RAIDS; index_D++)
    {
        pRDD = gRDX.rdd[index_D];

        if (pRDD != NULL)
        {
            /* BIT_SET(DMC_bits, CCB_DMC_raidcache); -- done in DC_RelRDDPSD */
            gRDX.count--;
            gRDX.rdd[index_D] = NULL;

            /* Handle the VLOP thingy */
            if (pRDD->vlop != NULL)
            {
                pRDD->vlop->pEventTable->func[VLOP_EVENT_ABORT] ();
            }

            DC_RelRDDPSD(pRDD);
        }
    }

    DLM_ClearLDDIndx();

    /*
     * Clear out the SES devices.  We will not worry about cleaning up the
     * devices themselves since we talk to the devices though a drive.
     */
    gEDX.count = 0;

    for (index_D = 0; index_D < MAX_DISK_BAYS; index_D++)
    {
        if ((gEDX.pdd[index_D] != NULL) &&
            (!SES_DirectlyAddressable(gEDX.pdd[index_D])))
        {
            DC_RelPDD(gEDX.pdd[index_D]);
            gEDX.pdd[index_D] = NULL;
        }
    }

    /* Clear out the Misc devices. */
    for (index_D = 0; index_D < MAX_MISC_DEVICES; index_D++)
    {
        pPDD = gMDX.pdd[index_D];

        if ((pPDD != NULL) && !FAB_IsDevInUse(pPDD->pDev))
        {
            if (pPDD->pDev != NULL)
            {
                s_Free(pPDD->pDev, sizeof(DEV), __FILE__, __LINE__);
            }

            DC_RelPDD(pPDD);
            gMDX.pdd[index_D] = NULL;
            gMDX.count--;
        }
    }

    /*
     * Walk through the drives and see if they are in use.  If not, then
     * delete them.
     */
    for (index_D = 0; index_D < MAX_PHYSICAL_DISKS; index_D++)
    {
        pPDD = gPDX.pdd[index_D];

        if ((pPDD != NULL) && !FAB_IsDevInUse(pPDD->pDev))
        {
            if (pPDD->pDev != NULL)
            {
                s_Free(pPDD->pDev, sizeof(DEV), __FILE__, __LINE__);
            }

            DC_RelPDD(pPDD);
            gPDX.pdd[index_D] = NULL;
            gPDX.count--;
        }
    }

    /* Clean out the workset information. */
    memset(gWorksetTable, 0, DEF_MAX_WORKSETS * sizeof(DEF_WORKSET));

    /* Rescan to get back any missing devices. */
    if (O_stopcnt != 0)
    {
        ON_Resume();
        F_rescanDevice(MRDREDISCOVER);
        ON_Stop();
    }
    else
    {
        F_rescanDevice(MRDREDISCOVER);
    }

    /* Clear mirror partners */
    for (index_D = 0; index_D < D_mpcnt; index_D++)
    {
        D_mpmaps[index_D].source = 0;
        D_mpmaps[index_D].dest = 0;
    }
    D_mpcnt = 0;
}

/**
******************************************************************************
**
**  @brief      To provide a means of processing the configuration restore
**              request issued by the CCB.
**
**  @param      pMRP - MRP structure
**
**  @return     error code indicating status of function
**
******************************************************************************
**/
UINT8 DEF_ConfigRestore(MR_PKT *pMRP)
{
    UINT8       type;
    PDD        *pPDD;
    PDX        *pPDX;
    UINT16      index_D;
    UINT16      tempIndex;
    UINT32      FID;

    type = ((MRRESTORE_REQ *) pMRP->pReq)->opt;

    /*
     * Check the type and act accordingly.  We first want to fetch the NVRAM
     * image we will use to restore, refresh or clear and reload.  After this,
     * we process the load type.
     */
    if (BIT_TEST(type, MRNOBDISK) || BIT_TEST(type, MRNOBWWN))
    {
        /*
         * We are reading from a specific disk identified by either the
         * world wide name or by the bus/ID/LUN.  Get the PDD pointer and
         * perform the read operation.
         */
        if (BIT_TEST(type, MRNOBDISK))
        {
            pPDD = DEF_FindPDDWWN(((MRRESTORE_REQ *) pMRP->pReq)->wwn,
                                  ((MRRESTORE_REQ *) pMRP->pReq)->lun);
        }
        else
        {
            pPDD = DEF_FindPDD(((MRRESTORE_REQ *) pMRP->pReq)->channel,
                               (UINT32)((MRRESTORE_REQ *) pMRP->pReq)->addr,
                               ((MRRESTORE_REQ *) pMRP->pReq)->lun);
        }

        /* Check for an operable device. */
        if (pPDD == NULL)
        {
            return DENONXDEV;
        }
        if (pPDD->devStat != PD_OP)
        {
            return DEINOPDEV;
        }

        /* Perform the read. */
        if (FS_ReadFile(FID_BE_NVRAM, O_temp_nvram, NVRAM_P2_SIZE_SECTORS, pPDD, 1) != DEOK)
        {
            return DEIOERR;
        }
    }
    else if (BIT_TEST(type, MRNOBFSYS) || BIT_TEST(type, MRNOBFID))
    {
        /*
         * Get the FID to read.  This will either be the FID from the
         * input block or the standard FID for the NVRAM.
         */
        if (BIT_TEST(type, MRNOBFID))
        {
            FID = (UINT32)((MRRESTORE_REQ *) pMRP->pReq)->addr;
        }
        else
        {
            FID = FID_BE_NVRAM;
        }

        /*
         * Now do the file system operation to read up the data.  First, read
         * up one block, then read up the right amount of data.
         */
        if (FS_MultiRead(FID, O_temp_nvram, 1, 1, 1, 0, 0) == DEOK)
        {
            /* make sure the length is reasonable. */
            UINT32      nvram_len = MIN(NVRAM_P2_SIZE, O_temp_nvram->length);

            if (FS_MultiRead(FID, O_temp_nvram,
                             (nvram_len + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR,
                             1, 1, 0, 0) != DEOK)
            {
                return DEIOERR;
            }
        }
        else
        {
            return DEIOERR;
        }
    }
    else if (BIT_TEST(type, MRNOBPCI))
    {
        /* Just copy the data from the source to the temp buffer. */
        memcpy((void *)O_temp_nvram,
               (void *)((MRRESTORE_REQ *) pMRP->pReq)->addr,
               MIN(NVRAM_P2_SIZE, ((NVRII *) ((MRRESTORE_REQ *) pMRP->pReq)->addr)->length));
    }
    else
    {
        /* Invalid since no fetch type was set. */
        return DEINVOPT;
    }

    /*
     * Now that we have the NVRAM image, process it according to the rest
     * of the input paramters.  This will either be a refresh (update the
     * structures based upon other controllers status), an overlay (get any
     * changes in configuration) or a restore (reset all internal structures
     * and load up the image).
     */
    if (NV_P2ChkSumChk(O_temp_nvram))
    {
        /* Act according to the load type. */
        if (BIT_TEST(type, MRNOBREFRESH))
        {
            NV_RefreshNvram(O_temp_nvram);
        }
        else
        {
            /*
             * We are doing a restore or an overlay.  In the case of a
             * restore, we need to reset the configuration first.  After
             * that, a restore or an overlay are identical.
             */
            if (BIT_TEST(type, MRNOBRESTORE))
            {
                DEF_ResetConfig();
            }

            pPDX = s_MallocC(sizeof(PDX) + sizeof(EDX) + sizeof(MDX), __FILE__, __LINE__);
            tempIndex = 0;

            for (index_D = 0; index_D < MAX_PHYSICAL_DISKS; index_D++)
            {
                if (gPDX.pdd[index_D] != NULL)
                {
                    pPDX->pdd[tempIndex++] = gPDX.pdd[index_D];
                    gPDX.pdd[index_D] = NULL;
                }
            }

            for (index_D = 0; index_D < MAX_MISC_DEVICES; index_D++)
            {
                if (gMDX.pdd[index_D] != NULL)
                {
                    pPDX->pdd[tempIndex++] = gMDX.pdd[index_D];
                    gMDX.pdd[index_D] = 0;
                }
            }

            for (index_D = 0; index_D < MAX_DISK_BAYS; index_D++)
            {
                if (gEDX.pdd[index_D] != NULL)
                {
                    pPDX->pdd[tempIndex++] = gEDX.pdd[index_D];
                    gEDX.pdd[index_D] = 0;
                }
            }

            gPDX.count = 0;
            gEDX.count = 0;
            gMDX.count = 0;
            pPDX->count = index_D;

            /*
             * On a restore, do not restart any copies.  On all others,
             * we want to restart them.
             */
            NV_RestoreNvram(O_temp_nvram, pPDX, FALSE,
                            (UINT8)!BIT_TEST(type, MRNOBRESTORE), 0);

            s_Free(pPDX, sizeof(PDX) + sizeof(EDX) + sizeof(MDX), __FILE__, __LINE__);
        }
    }
    else
    {
        /*
         * For some reason  NVRAM is not loaded properly(checksum error),
         * we reflect the default whql flag to the front end.
         */
        DEF_SndConfigOpt();
        return DEBADNVREC;
    }

    /* If there were no errors, propagate to the front end processor. */
    TaskReadyByState(PCB_ONLINE_WAIT);
    DEF_UpdRmtCache();
    DEF_UpdRmtSysInfo();
    DEF_UpdRmtCacheGlobal();
    DEF_SignalServerUpdate();
    DEF_SignalVDiskUpdate();
    DEF_SndConfigOpt();

    return DEOK;
}


/**
******************************************************************************
**
**  @brief      To provide a means of processing the configuration reset
**              request issued by the CCB.
**
**  @param      pMRP - MRP structure
**
**  @return     none
**
**  @attention  This replaces the d$creset assembly code.  It also indirectly
**              replaces the d$rmtcreset assembly by adding the send packet
**              code directly in this function.
**
******************************************************************************
**/
UINT8 DEF_ConfigReset(MR_PKT *pMRP)
{
    UINT32      type;
    void       *pBuf;
    UINT32      i;
    PDD        *pPDD;

    type = ((MRRESET_REQ *) pMRP->pReq)->type;

    /*
     * The MXNBENVA is a special case, the only work that needs to be
     * done is to clear the P3 and P4 sections of NVRAM.  The rest of
     * the cases do more with the FE processor and/or BE configuration.
     */
    if (type == MXNBENVA)
    {
        /*
         * Clear the P3 and P4 sections of NVA records
         */
        NVA_ClearP3();
        NVA_ClearP4();
    }
    else
    {
        UINT8       CR_ipcomm[sizeof(MRRESETCONFIG_REQ)];
        UINT8      *CR_ipretptr = s_MallocC(sizeof(MRRESETCONFIG_RSP), __FILE__, __LINE__);

        /*
         * For the rest of the cases (MXNNMI, MXNFENVA, MXNALL), send the
         * reset configuration request to the FE processor.  After that
         * the MXNFENVA is completed, the MXNNMI needs to clear the NMI
         * information and the MXNALL needs to complete the configuration
         * reset (BE side of the reset).
         */

        ((MRRESETCONFIG_REQ *) & CR_ipcomm[0])->type = type;

        L$send_packet(&CR_ipcomm[0],
                      sizeof(MRRESETCONFIG_REQ),
                      MRRESETCONFIG,
                      CR_ipretptr,
                      sizeof(MRRESETCONFIG_RSP),
                      &DEF_RmtWait,
                      (UINT32)K_xpcb);

        TaskSetMyState(PCB_WAIT_FE_BE_MRP);
        TaskSwitch();

        s_Free(CR_ipretptr, sizeof(MRRESETCONFIG_RSP), __FILE__, __LINE__);

        if (type == MXNNMI)
        {
            /*
             * Clear the diagnostic NMI counts
             */
            MSC_NMIClear();
        }
        else if (type == MXNALL)
        {
            /*
             * Allocate a buffer to use for writing the label on the
             * physical disk.
             */
            pBuf = s_MallocC(FS_SIZE_LABEL * BYTES_PER_SECTOR, __FILE__, __LINE__);

            /*
             * Search out and clear all device labels
             */
            for (i = 0; i < MAX_PHYSICAL_DISKS; ++i)
            {
                if (gPDX.pdd[i] != NULL)
                {
                    pPDD = gPDX.pdd[i];

                    /*
                     * Write the label file with the empty buffer
                     */
                    FS_WriteFile(FID_LABEL, pBuf, FS_SIZE_LABEL, pPDD, 1);

                    pPDD->devClass = PD_UNLAB;
                    pPDD->miscStat = 0;
                    pPDD->failedLED = PD_LED_OFF;

                    ON_LedChanged(pPDD);
                }
            }

            /*
             * Reset the NVRAM configuration
             */
            DEF_ResetConfig();

            /*
             * Clear the P3 and P4 sections of NVA records
             */
            NVA_ClearP3();
            NVA_ClearP4();

            /*
             * Clear the diagnostic NMI counts
             */
            MSC_NMIClear();

            /*
             * Update P2 NVRAM (clears)
             */
            NV_P2UpdateConfig();

            /*
             * Tell the FEP that servers changed
             */
            DEF_SignalServerUpdate();

            /*
             * Tell the FEP that vdisks changed
             */
            DEF_SignalVDiskUpdate();

            /*
             * Free the buffer used for writing the label on the
             * physical disk.
             */
            s_Free(pBuf, FS_SIZE_LABEL * BYTES_PER_SECTOR, __FILE__, __LINE__);
        }
    }

    return DEOK;
}


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
******************************************************************************
**/
UINT8 DEF_ChgRAIDNotMirroringState(MR_PKT *pMRP)
{
    UINT8       type;           /* Set or Clear the Not Mirroring State     */
    RDD        *pRDD;           /* Pointer to an RDD                        */
    UINT16      rid;            /* RAID ID                                  */
    UINT8       updateNVRAM = FALSE;    /* Flag to show NVRAM needs updated         */
    UINT32      controllerSN = 0;       /* Controller Serial Number set in RDD      */

    /* Get the Type field from the request */
    type = ((MRCHGRAIDNOTMIRRORING_REQ *) pMRP->pReq)->type;

    /*
     * Set the Serial Number to put into the RDD based on the type of request.
     * If Set, put in this controllers serial number.  If Clear, then put zeros
     * in the field.
     */
    if (type == MRCCLEARNOTMIRRORING)       /* Clear the Not Mirroring State? */
    {
        /* Show clear by removing Serial Number (already done in init) */
    }
    else if (type == MRCSETNOTMIRRORING)    /* Set the Not Mirroring State? */
    {
        controllerSN = K_ficb->cSerial;     /* Show set by using this cntrls SN */
    }
    else                                    /* Invalid parameter */
    {
        return DEINVOPT;                    /* Return - Invalid Option */
    }

    /*
     * Check each valid RDD in the system, looking for a RAID 5 owned by
     * this controller.
     */
    for (rid = 0; rid < MAX_RAIDS; ++rid)
    {
        pRDD = gRDX.rdd[rid];

        if (pRDD &&                         /* RDD is a valid pointer       */
            pRDD->type == RD_RAID5 &&       /* RDD is a RAID 5              */
            DL_AmIOwner(pRDD->vid))         /* This controller owns the VID */
        {
            BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */

            /* Modify the Not Mirroring State as requested */
            pRDD->notMirroringCSN = controllerSN;

            /* Set flag to show the NVRAM needs to be updated */
            updateNVRAM = TRUE;
        }
    }

    /* Update NVRAM if it needs to be updated */
    if (updateNVRAM == TRUE)
    {
        NV_P2Update();
    }

    return DEOK;
}

/**
******************************************************************************
**
**  @biref      This function will modify the "NotMirroring" state of
**              all the raid5 raids (of source/destination vdisk pair)
**              when the copy ownership is acquired by this controller.
**
**              If the mirror partner is available the NotMirroring
**              serial number is set to zero , otherwise it is set to
**              self serial number.
**
**  @param      pCOR - Pointer to Copy Operation Record
**
**  @return     NIL.
**
******************************************************************************
**/
void DEF_ChgRAIDNotMirroringState_2(COR *pCOR)
{
    RDD        *pRDD;           /* Pointer to an RDD                      */
    UINT32      controllerSN = 0;       /* Controller Serial Number set in RDD    */
    VDD        *pSrcVDD;
    VDD        *pDestVDD;

    /* Proceed further only if the COR is not NULL */
    if (pCOR)
    {
        /*
         * Get the Destination and Source VDDs
         * Make sure that if any of Destination and Source VDD is not NULL
         */
        pDestVDD = pCOR->destvdd;
        pSrcVDD = pCOR->srcvdd;

        if ((pSrcVDD == NULL) || (pDestVDD == NULL))
        {
            return;
        }

        /*
         * Check if self is the Mirror Partner
         * If it is, get the serial number
         */
        if (K_ficb->cSerial == K_ficb->mirrorPartner)
        {
            controllerSN = K_ficb->cSerial;
        }

        /*
         * Parse through all the raids of source vdisk and set the notMirroring field
         * in all its raids
         */
        pRDD = pSrcVDD->pRDD;
        while (pRDD)
        {
            if (pRDD->type == RD_RAID5) /* RDD is a RAID 5   */
            {
                BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                pRDD->notMirroringCSN = controllerSN;
            }
            pRDD = pRDD->pNRDD;
        }

        /*
         * Parse through all the raids of destination vdisk and set the
         * notMirroring field in all its raids
         */
        pRDD = pDestVDD->pRDD;
        while (pRDD)
        {
            if (pRDD->type == RD_RAID5) /* RDD is a RAID 5              */
            {
                BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                pRDD->notMirroringCSN = controllerSN;
            }
            pRDD = pRDD->pNRDD;
        }
    }
}

/**
******************************************************************************
**
**  @brief      This function will label a physical drive.
**
**              If the drive is being labelled as any data drive type and
**              there is either a drive from the current slot and enclosure
**              that is missing or the drive from the current slot and
**              enclosure was previously hot spared to somewhere else, the
**              drive will only be allowed to be labelled as a hot spare and
**              the hot spare operation will start.
**
**  @param      mrp - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_LabelDevice(MR_PKT *pMRP)
{
    PDD        *pPDD = NULL;
    MRLABEL_REQ *pReq;
    ILT        *pILT = NULL;
    void       *pBuffer;
    PRP        *pPRP;
    UINT8       retCode = DEOK;
    UINT8       labelType = MLDNOLABEL;
    UINT8       activeDefinitions = TRUE;
    UINT8       inOpCheck;
    UINT32      LBA;
    PDD        *pConflict;

    /* Get the Data and the Mask from the request */
    pReq = ((MRLABEL_REQ *) pMRP->pReq);
    pMRP->pRsp->rspLen = pMRP->rspLen;

    if ((pReq->pid > MAX_PHYSICAL_DISKS) || (gPDX.pdd[pReq->pid] == NULL))
    {
        retCode = DEINVPID;
    }
    else
    {
        pPDD = gPDX.pdd[pReq->pid];

        /*
         * Check the state of the drive being labelled.  If it is not
         * operable then fail out indicating such.  Note that the drive
         * being non-operable with a missing file system is considered a
         * drive that can be labelled since that's what a fresh-out-of-the-box
         * drive looks like.
         */
        if ((pPDD->postStat != PD_FDIR) && (pPDD->devStat == PD_INOP))
        {
            retCode = DEINOPDEV;
        }
        else if (pReq->labtype > MLDNDATALABEL)
        {
            retCode = DEINVLABTYP;
        }
        else
        {
            /*
             * Get the inoperable check value for this drive.  This is
             * an indicator of whether or not there is a drive that should
             * be in this slot but is missing or set to inoperable.
             */
            inOpCheck = RB_InOpCheck(pReq->dname[PD_DNAME_CSES],
                                     pReq->dname[PD_DNAME_CSLOT],
                                     pPDD,
                                     FALSE,
                                     &pConflict);

            /*
             * Get the active definitions check value for this drive.  This
             * is an indicator of whether or not there are RAID devices
             * already defined for this drive.
             */
            activeDefinitions = RB_ActDefCheck(pPDD);

            /*
             * Now determine which label operation to do.  This can change
             * from what was passed in if there are RAID definitions that
             * already exist on the drive or in the slot the drive to be
             * labelled is in.
             */
            if (pReq->labtype == MLDNOLABEL)
            {
                if (activeDefinitions)
                {
                    /*
                     * There are RAID devices defined on this drive.
                     * Do not allow the drive to be relabelled.  Exit
                     * with an error indicating active definitions.
                     */
                    retCode = DEDEVUSED;
                }
                else if (inOpCheck && (pConflict->devType == pPDD->devType))
                {
                    /*
                     * If there is a drive that belongs in this slot
                     * and is hot spared to some other location, then
                     * do not allow this drive to be relabelled as no
                     * label.  Force it to a hot spare.
                     */
                    labelType = MLDSPARELABEL;
                }
                else
                {
                    labelType = MLDNOLABEL;
                }
            }
            else
            {
                /* Default the label to the current request. */
                labelType = pReq->labtype;

                /*
                 * If there was a drive in the system that was
                 * failed in this spot, label this drive as a
                 * hot spare and fail that drive over to here.
                 */
                if (inOpCheck && !activeDefinitions &&
                    (pConflict->devType == pPDD->devType))
                {
                    labelType = MLDSPARELABEL;
                }
                else if (pPDD->devClass != labelType)
                {
                    /*
                     * The class changed, check for other items.
                     *
                     * If the device was not labelled, then all changes
                     * are OK.  If the type is hot spare or data, then
                     * additional checks are still required.
                     */
                    memset(pPDD->hsDevName, 0, 4);

                    if (pPDD->devClass != MLDNOLABEL)
                    {
                        if (pPDD->devClass != MLDSPARELABEL)
                        {
                            if (activeDefinitions)
                            {
                                retCode = DEDEVUSED;
                            }
                        }
                    }
                }
            }
        }
    }

    /* Now do the actual label operation based upon the derived type. */
    if (retCode == DEOK)
    {
        if (labelType == MLDNOLABEL)
        {
            /* Unlabel the device. */
            pBuffer = s_MallocC(BYTES_PER_SECTOR * FS_SIZE_LABEL, __FILE__, __LINE__);

            FS_WriteFile(FID_LABEL, pBuffer, FS_SIZE_LABEL, pPDD, 1);

            s_Free(pBuffer, BYTES_PER_SECTOR * FS_SIZE_LABEL, __FILE__, __LINE__);

            BIT_CLEAR(pPDD->miscStat, PD_MB_REPLACE);
        }
        else
        {
            /*
             * We are going to label the drive.  In this case, we will
             * start by setting all of the mode parameters on the drives
             * to ensure that the drive behaves as we want it to.  Then
             * we will look at the options to see what additional items
             * have to be done to the drive before laying down the label.
             */
            ON_ModeSenseSelect(&gTemplateMSCache, pPDD,
                               MODE_CACHE_SET_MASK, MODE_CACHE_CLR_MASK);

            ON_ModeSenseSelect(&gTemplateMSRWErr, pPDD,
                               MODE_WRER_SET_MASK, MODE_WRER_CLR_MASK);

            ON_ModeSenseSelect(&gTemplateMSVErr, pPDD,
                               MODE_VER_SET_MASK, MODE_VER_CLR_MASK);

            ON_ModeSenseSelect(&gTemplateMSPower, pPDD,
                               MODE_PCNEW_SET_MASK, MODE_PCNEW_CLR_MASK);

            ON_ModeSenseSelect(&gTemplateMSException, pPDD,
                               MODE_IEC_SET_MASK, MODE_IEC_CLR_MASK);

            ON_ModeSenseSelect(&gTemplateMSFC, pPDD,
                               MODE_FCIC_SET_MASK, MODE_FCIC_CLR_MASK);
            /*
             * If a full init was requested, first clear the entire reserved
             * area by writing out zeros to it.
             */
            if (BIT_TEST(pReq->option, MLDFULL))
            {
                UINT32      write_size;

#ifndef DISABLE_WRITE_SAME
                /* Issue write same commands to fill it in.  */
                pILT = ON_GenReq(&gTemplateWriteSame, pPDD, &pBuffer, &pPRP);
                write_size = RESERVED_AREA_SIZE / 8;

                /* Set the length of each write same. */
                pPRP->cmd[7] = write_size >> 8;
                pPRP->cmd[8] = write_size & 0xFF;
                for (LBA = 0; LBA < RESERVED_AREA_SIZE; LBA += write_size)
                {
                    *(UINT32 *)(&pPRP->cmd[2]) = bswap_32(LBA);

                    /* Now issue the command. */
                    ON_QueReq(pILT);

                    if (MSC_ChkStat(pPRP) != DEOK)
                    {
                        retCode = DEIOERR;
                    }
                }

                ON_RelReq(pILT);
#else
                pILT = ON_GenReq(&gTemplateWrite, pPDD, &pBuffer, &pPRP);
                /* this should be 1 mB */
                write_size = RESERVED_AREA_SIZE / 128;

                /* Set the length of each write. */
                pPRP->pSGL = m_asglbuf(write_size * 512);
                PM_ClearSGL(pPRP->pSGL);
                pPRP->rqBytes = write_size * 512;
                pPRP->cmd[7] = write_size >> 8;
                pPRP->cmd[8] = write_size & 0xFF;
                for (LBA = 0; LBA < RESERVED_AREA_SIZE; LBA += write_size)
                {
                    *(UINT32 *)(&pPRP->cmd[2]) = bswap_32(LBA);

                    /* Now issue the command. */
                    ON_QueReq(pILT);
                    if (MSC_ChkStat(pPRP) != DEOK)
                    {
                        retCode = DEIOERR;
                        break;
                    }
                }
                PM_RelSGLWithBuf(pPRP->pSGL);
                pPRP->pSGL = NULL;
                ON_RelReq(pILT);
#endif /* !DISABLE_WRITE_SAME */
            }

            if (BIT_TEST(pReq->option, MLDDUPLICATE))
            {
                /*
                 * Duplicate the FSys off a known good disk.  If we already
                 * own the device, we do not have to update the file system.
                 */
                if (pPDD->ssn != K_ficb->vcgID)
                {
                    pPDD->devClass = labelType;

                    /* Pass 1 to not allow fs stop to interrupt process. */
                    if (FS_UpdateFS(pPDD, 1) != PASS)
                    {
                        retCode = DEIOERR;
                    }
                }
            }

            if (BIT_TEST(pReq->option, MLDFSYS))
            {
                /* Create a file system. */
                FS_InitDirectory(pPDD);
            }

            /* Now write the label. */
            if (retCode == DEOK)
            {
                XDL        *pxdl;

                pBuffer = s_MallocC(BYTES_PER_SECTOR * FS_SIZE_LABEL, __FILE__, __LINE__);
                pxdl = (XDL *) pBuffer;

                /* Copy the default label text into the buffer. */
                memcpy(pxdl->text, &O_devlab, sizeof(pxdl->text));
                memcpy(pxdl->devName, pReq->dname, 4);

                pxdl->sSerial = K_ficb->vcgID;
                pxdl->devClass = labelType;
                pxdl->wwn = pPDD->wwn;

                retCode = FS_WriteFile(FID_LABEL, pBuffer, FS_SIZE_LABEL, pPDD, 1);

                s_Free(pBuffer, BYTES_PER_SECTOR * FS_SIZE_LABEL, __FILE__, __LINE__);

                if (retCode != DEOK)
                {
                    retCode = DEIOERR;
                }
                else
                {
                    pPDD->devStat = PD_OP;
                    BIT_CLEAR(pPDD->miscStat, PD_MB_SERIALCHANGE);
                    BIT_CLEAR(pPDD->miscStat, PD_MB_FSERROR);
                    BIT_CLEAR(pPDD->miscStat, PD_MB_SCHEDREBUILD);
                    BIT_CLEAR(pPDD->miscStat, PD_MB_REBUILDING);
                    BIT_CLEAR(pPDD->miscStat, PD_MB_REPLACE);
                }
            }
        }
    }

    /*
     * Final clean up.  If the drive was unlabelled or there was an IO error
     * in the processing of the drive, clear out the information in the PDD.
     * Otherwise, set the values based upon the input parms which also should
     * have been written to the drives label.
     */
    if (((labelType == MLDNOLABEL) && (retCode == DEOK)) || (retCode == DEIOERR))
    {
        pPDD->devClass = MLDNOLABEL;
        memset(pPDD->devName, 0, 4);
        pPDD->ssn = 0;
    }
    else if (retCode == DEOK)
    {
        pPDD->devClass = labelType;
        memcpy(pPDD->devName, pReq->dname, 4);
        pPDD->ssn = K_ficb->vcgID;
    }

    if (pPDD != NULL)
    {
        DA_CalcSpace(pPDD->pid, TRUE);
    }

    RB_CheckHSDepletion(NULL);
    RB_setpsdstat();
    RB_SearchForFailedPSDs();
    NV_P2UpdateConfig();
    NV_SendFSys(TRUE);
    RB_CheckHSCapacity(NULL);
    RB_CheckHSDepletion(NULL);

    /*
     * This case is to handle when a drive from some other system is inserted
     * into the slot from where the previous drive got failed/removed.
     *
     * In this case, the serviceability feature log a message to the user and
     * auto fail back does not happen.
     * Auto fail back happens only when this drive gets labelled.
     * If autofailback flag is set and the drive is being labelled as HOTSPARE,
     *
     * check if any failback needs to be done.
     */
    if ((retCode == DEOK) && (pPDD->devClass == PD_HOTLAB) && gAutoFailBackEnable)
    {
        RB_CheckPDDForFailBack(pPDD);
    }

    return (retCode);
}

/**
******************************************************************************
**
**  @brief      To provide a standard means of sending a message to the
**              front end processor to update a single server record.
**
**              This function will create a MRP to send across the Link Layer
**              to the front end processor and will wait until the front end
**              processor has informed us that it was processed.
**
**  @param      UINT16 sid
**              UINT16 option - TRUE to delete the server
**
**  @return     none
**
******************************************************************************
**/
void DEF_UpdRmtServer(UINT16 sid, UINT16 options)
{
    UINT32      i = 0;
    UINT32      size = 0;
    SDD        *pSDD = NULL;
    MRLVM      *pMRL = NULL;
    struct LVM *pLVM = NULL;
    MRREPORTSCONFIG_REQ *pReq;
    MRREPORTSCONFIG_RSP *pRsp;

    pSDD = gSDX.sdd[sid];

    size = sizeof(MRREPORTSCONFIG_REQ) + sizeof(MRREPORTSCONFIG_RSP);

    if ((options == TRUE) || (pSDD == NULL))
    {
        /*
         * Delete Server Request
         */
        pRsp = (MRREPORTSCONFIG_RSP *) s_MallocC(size, __FILE__, __LINE__);
        pReq = (MRREPORTSCONFIG_REQ *) (pRsp + 1);

        pReq->sid = sid;
        pReq->del = TRUE;
    }
    else
    {
        /*
         * Update Server Record
         */
        size += (pSDD->numLuns * sizeof(MRLVM));

        pRsp = (MRREPORTSCONFIG_RSP *) s_MallocC(size, __FILE__, __LINE__);
        pReq = (MRREPORTSCONFIG_REQ *) (pRsp + 1);

        pReq->sid = pSDD->sid;
        pReq->del = FALSE;
        pReq->lsid = pSDD->linkedSID;
        pReq->nluns = pSDD->numLuns;
        pReq->tid = pSDD->tid;
        pReq->status = pSDD->status;
        pReq->pri = pSDD->pri;
        pReq->attrib = pSDD->attrib;
        pReq->owner = pSDD->owner;
        pReq->wwn = pSDD->wwn;
        memcpy(pReq->name, pSDD->name, 16);
        memcpy(pReq->i_name, pSDD->i_name, 254);

        pMRL = (MRLVM *) (pReq + 1);
        pLVM = pSDD->lvm;

        for (i = 0; i < pReq->nluns; i++)
        {
            if ((pMRL == NULL) || (pLVM == NULL))
            {
                break;
            }

            pMRL->lun = pLVM->lun;
            pMRL->vid = pLVM->vid;

            pLVM = pLVM->nlvm;
            pMRL = pMRL + 1;
        }
    }
    L$send_packet(pReq, (size - sizeof(MRREPORTSCONFIG_RSP)),
                  MRREPORTSCONFIG,
                  pRsp, sizeof(MRREPORTSCONFIG_RSP),
                  (void *)&DEF_RmtWait, (UINT32)K_xpcb);

    TaskSetMyState(PCB_WAIT_FE_BE_MRP);
    TaskSwitch();

    s_Free((void *)pRsp, size, __FILE__, __LINE__);
}

/**
******************************************************************************
**
**  @brief      Log Event to CCB when an iSCSI Target comes up/goes down
**
**  @param      sid
**              tid
**              tsih
**              initiator Name
**              SessionType
**
**  @return     none
**
******************************************************************************
**/

void logTargetUp(TGD *pTgd, UINT8 type, UINT8 state);
void logTargetUp(TGD *pTgd, UINT8 type, UINT8 state)
{
    LOG_TARGET_UP_OP_PKT elzi;  /* Log event        */

    /*
     * Send a log message
     */
    elzi.header.event = LOG_TARGET_UP_OP;
    elzi.tid = pTgd->tid;
    elzi.state = state;
    elzi.type = type;
    if (type == 0)
    {
        elzi.wwn = pTgd->portName;
    }
    else
    {
        if (pTgd->itgd == NULL)
        {
            elzi.ip = 0;
            elzi.mask = 0;
            elzi.gateway = 0;
        }
        else
        {
            elzi.ip = pTgd->itgd->ipAddr;
            elzi.mask = pTgd->itgd->ipMask;
            elzi.gateway = pTgd->itgd->ipGw;
        }
    }

    /*
     * Note: message is short, and L$send_packet copies into the MRP.
     */
    MSC_LogMessageStack(&elzi, sizeof(LOG_TARGET_UP_OP_PKT));
}                               /* logTargetUp */

/**
******************************************************************************
**
**  @brief      To provide a standard means of sending MRP to FE to get
**              the port type
**
**  @param      UINT16  tid
**              UINT8   opt - as defined in target.h
**
**  @return     none
**
******************************************************************************
**/
void DEF_ValidateTarget(TGD *pTGD, UINT8 status)
{
    MRGETPORTTYPE_REQ *pReq;
    MRGETPORTTYPE_RSP *pRsp;

    /*
     * Allocate memory required to send the Validate Target request &
     * response
     */
    pReq = (MRGETPORTTYPE_REQ *) s_MallocC(sizeof(MRGETPORTTYPE_REQ)
                                          + sizeof(MRGETPORTTYPE_RSP), __FILE__, __LINE__);
    pRsp = (MRGETPORTTYPE_RSP *) (pReq + 1);

    /*
     * Set the request params
     */
    pReq->pport = pTGD->prefPort;
    pReq->aport = pTGD->altPort;

    /*
     * Send the request MRP to the FE
     */
    L$send_packet(pReq,
                  sizeof(MRGETPORTTYPE_REQ),
                  MRGETPORTTYPE,
                  pRsp,
                  sizeof(MRGETPORTTYPE_RSP),
                  (void *)&DEF_RmtWait,
                  (UINT32)K_xpcb);

    /*
     * Wait until the FE completes the MRP request
     */
    TaskSetMyState(PCB_WAIT_FE_BE_MRP);
    TaskSwitch();

    /*
     * Check the return status. If successful, process the result
     */
    if (pRsp->header.status == DEOK)
    {
        switch (pRsp->type)
        {
            case PT_FC:
                {
                    fprintf(stderr, "PORTTYPE: tid(%d) = PT_FC\n", pTGD->tid);
                    logTargetUp(pTGD, 0, 0);

                    if (BIT_TEST(pTGD->opt, TARGET_ISCSI))
                    {
                        if (BIT_TEST(K_ii.status, status))
                        {
                            /*
                             * The TGD type is iSCSI & the corresponding FE ports
                             * are FC. Change the TGD to FC and save to the NVRAM.
                             */
                            ON_CreateTargetWWN(pTGD);
                            BIT_CLEAR(pTGD->opt, TARGET_ISCSI);

                            /*
                             * Update NVRAM
                             */
                            NV_P2UpdateConfig();

                            /*
                             * Update the CCB with a log msg. - TBD
                             */
                        }
                        else
                        {
                            /*
                             * Update the CCB with a log msg. - TBD
                             */
                        }
                    }
                }
                break;

            case PT_ISCSI:
                {
                    fprintf(stderr, "PORTTYPE: tid(%d) = PT_ISCSI\n", pTGD->tid);
                    logTargetUp(pTGD, 1, 0);

                    if (!BIT_TEST(pTGD->opt, TARGET_ISCSI))
                    {
                        if (BIT_TEST(K_ii.status, status))
                        {
                            /*
                             * The TGD type is FC & the corresponding FE ports are
                             * iSCSI. Change the TGD to iSCSC and save to the NVRAM.
                             */
                            BIT_SET(pTGD->opt, TARGET_ISCSI);

                            if (pTGD->itgd == NULL)
                            {
                                DEF_CreateTargetInfo(pTGD, NULL);
                            }

                            pTGD->ipPrefix = MSC_Mask2Prefix(bswap_32(pTGD->itgd->ipMask));
                            pTGD->ipAddr = pTGD->itgd->ipAddr;
                            pTGD->ipGw = pTGD->itgd->ipGw;

                            /*
                             * Update the iSCSI code in the FE with the target info
                             */
                            DEF_UpdRmtTgInfo(pTGD);

                            /*
                             * Update NVRAM
                             */
                            NV_P2UpdateConfig();

                            /*
                             * Update the CCB with a log msg. - TBD
                             */
                        }
                        else
                        {
                            /*
                             * Update the CCB with a log msg. - TBD
                             */
                        }
                    }
                }
                break;

            case PT_INVAL:
                {
                    /*
                     * The preffered port & the alt port are of different media
                     * type. This is an invalid hardware configuration. Log a msg
                     * to the CCB and let the CCB take appropriate action.
                     */
                    fprintf(stderr, "PORTTYPE: tid(%d) = PT_INVAL\n", pTGD->tid);
                }
                break;

            default:
                break;
        }
    }
    /*
     * Free the memory allocated for the MRP request & response
     */
    s_Free((void *)pReq, (sizeof(MRGETPORTTYPE_REQ) + sizeof(MRGETPORTTYPE_RSP)), __FILE__, __LINE__);
}                               /* DEF_ValidateTarget */

/**
******************************************************************************
**
**  @brief      Swap two PIDs, making sure no outstanding I/O exists.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
static UINT8 DEF_SwapPIDs(MR_PKT *pMRP UNUSED)
{
    MRSWAPPIDS_REQ   *req = (MRSWAPPIDS_REQ *)pMRP->pReq;
    UINT16            pid1 = req->pid1;
    UINT16            pid2 = req->pid2;

    if (pid1 >=MAX_PHYSICAL_DISKS || pid2 >=MAX_PHYSICAL_DISKS)
    {
        return(DENONXDEV);
    }

    PDD              *pdd1 = P_pddindx[pid1];
    PDD              *pdd2 = P_pddindx[pid2];

    if (pdd1 == NULL && pdd2 == NULL)
    {
        return(DENONXDEV);
    }

    if (pdd1 == NULL)
    {
fprintf(stderr, "swap PID %d into currently NULL %d\n", pid2, pid1);
        P_pddindx[pid1] = pdd2;
        P_pddindx[pid2] = NULL;
        pdd2->pid = pid1;
    }
    else if (pdd2 == NULL)
    {
fprintf(stderr, "swap PID %d into currently NULL %d\n", pid1, pid2);
        P_pddindx[pid2] = pdd1;
        P_pddindx[pid1] = NULL;
        pdd1->pid = pid2;
    }
    else
    {
fprintf(stderr, "swap PID %d with PID %d\n", pid1, pid2);
        P_pddindx[pid1] = pdd2;
        P_pddindx[pid2] = pdd1;
        pdd2->pid = pid1;
        pdd1->pid = pid2;
    }

/* Update NVRAM */
    NV_P2UpdateConfig();            /* Update NVRAM part II */

    return(DEOK);
}   /* End of DEF_SwapPIDs */

/**
******************************************************************************
**
**  @brief      Emulate a PAB (ProActive Busy) to a PID, turn on/off.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
static UINT8 DEF_EmulatePAB(MR_PKT *pMRP)
{
    MREMULATEPAB_REQ *req = (MREMULATEPAB_REQ *)pMRP->pReq;
    UINT16            ise;
    UINT16            option;
    MREMULATEPAB_RSP *rsp = (MREMULATEPAB_RSP *)pMRP->pRsp;
    int               i;
    PDD              *pdd;
    int               count = 0;

    rsp->cnt = 0;                               /* Set response "count" to zero for errors. */

    /* Get ISE number and option from the MRP. */
    ise = req->id;
    option = req->option;

    /* Check that the specified Bay (ISE) exists. */
    if (ise >=MAX_DISK_BAYS || E_pddindx[ise] == NULL)
    {
        return(DENONXDEV);
    }

    if (option > 2)                             /* If option out of range. */
    {
        return(DEINVOP);
    }

    /* Check all PIDs to see if they are for ISE(bay). */
    for (i = 0; i < MAX_PHYSICAL_DISKS; i++)
    {
        pdd = P_pddindx[i];
        if ((pdd != NULL) && (pdd->ses == ise))
        {
            if (option == 0)                    /* Clear busy if set */
            {
                /* If busy, clear. */
                if (BIT_TEST(pdd->flags, PD_BEBUSY) == TRUE)
                {
                    fprintf(stderr, "DEF_EmulatePAB: Clear Busy of PID %d\n", i);
                    ON_BEClear(i, TRUE);
                }
            }

            if (option == 1)                    /* Set busy if clear */
            {
                /* If not busy, set busy. */
                if (BIT_TEST(pdd->flags, PD_BEBUSY) != TRUE)
                {
                    fprintf(stderr, "DEF_EmulatePAB: Set Busy of PID %d\n", i);
                    ON_BEBusy(i, 300, TRUE);    /* SET PAB for just this PID 300 seconds till TUR happens */
                }                               /* See online.as, just before .pd35 */
            }

            if (pdd->qd > 0)
            {
                fprintf(stderr, "DEF_EmulatePAB pid=%d has Outstanding Request Count=%d\n", i, pdd->qd);
                count += pdd->qd;            /* Outstanding requests */
            }
        }
    }
    rsp->cnt = count;
    return(DEOK);
}   /* End of DEF_EmulatePAB */

/**
******************************************************************************
**
**  @brief      Decode the function code to the appropriate function.
**
**              The function code is extracted from the MRP and
**              used to determine which function to execute.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_CBridge(MR_PKT *pMRP)
{
    UINT8       retStatus = DEINVPKTTYP;        /* Return value, prep status    */

    /*
     * Check if the MRP was from the CCB.
     */
    if ((pMRP->function & 0xFF00) == MREBFUNCBASE)
    {
        /*
         * Decode the function
         */
        switch (pMRP->function)
        {
            case MRGETPINFO:
                {
                    retStatus = DEF_GetPData(pMRP);
                }
                break;

            case MRGETEINFO:
                {
                    retStatus = DEF_GetEData(pMRP);
                }
                break;

            case MRGETMINFO:
                {
                    retStatus = DEF_GetMData(pMRP);
                }
                break;

            case MRGETRINFO:
                {
                    retStatus = DEF_GetRAIDDeviceData(pMRP);
                }
                break;

            case MRGETVINFO:
                {
                    retStatus = DEF_GetVirtualData(pMRP);
                }
                break;

            case MRGETPLIST:
                {
                    retStatus = DEF_GetPList(pMRP);
                }
                break;

            case MRGETELIST:
                {
                    retStatus = DEF_GetEList(pMRP);
                }
                break;

            case MRGETMLIST:
                {
                    retStatus = DEF_GetMList(pMRP);
                }
                break;

            case MRGETRLIST:
                {
                    retStatus = DEF_GetRList(pMRP);
                }
                break;

            case MRGETSLIST:
                {
                    retStatus = DEF_GetSList(pMRP);
                }
                break;

            case MRGETTLIST:
                {
                    retStatus = DEF_GetTList(pMRP);
                }
                break;

            case MRGETVLIST:
                {
                    retStatus = DEF_GetVList(pMRP);
                }
                break;

            case MRRESET:
                {
                    retStatus = DEF_ConfigReset(pMRP);
                }
                break;

            case MRRESTORE:
            case MRREFRESH:
                {
                    retStatus = DEF_ConfigRestore(pMRP);
                }
                break;

            case MRRESETBEPORT:
                {
                    retStatus = DI_ResetPort(pMRP);
                }
                break;

            case MRLABEL:
                {
                    retStatus = DEF_LabelDevice(pMRP);
                }
                break;

            case MRBELOOP:
                {
                    retStatus = DI_PortStats(pMRP);
                }
                break;

            case MRGETVIDOWNER:
                {
                    retStatus = DEF_GetVDiskOwner(pMRP);
                }
                break;

            case MRBEGETDVLIST:
                {
                    retStatus = DI_GetDeviceList(pMRP);
                }
                break;

            case MRBEGETPORTLIST:
                {
                    retStatus = DI_GetPortList(pMRP);
                }
                break;

            case MRDEFRAGMENT:
                {
                    retStatus = DF_Defragment(pMRP);
                }
                break;

            case MRGETBEDEVPATHS:
                {
                    retStatus = DEF_GetDevicePaths(pMRP);
                }
                break;

            case MRBEGENERIC:
                {
                    retStatus = DEF_Generic(pMRP);
                }
                break;

            case MRGETMPLIST:
                {
                    retStatus = DEF_GetMirrorPartnerList(pMRP);
                }
                break;

            case MRBELOOPPRIMITIVE:
                {
                    retStatus = DI_LoopPrimitive(pMRP);
                }
                break;

            case MRCREATECTRLR:
                {
                    retStatus = DEF_CreateCtrl(pMRP);
                }
                break;

            case MRFAILCTRL:
                {
                    retStatus = DEF_FailController(pMRP);
                }
                break;

            case MRDEGRADEPORT:
                {
                    retStatus = DEF_DegradePort(pMRP);
                }
                break;

            case MRMAPLUN:
                {
                    retStatus = DL_SetLUNMap(pMRP);
                }
                break;

            case MRUNMAPLUN:
                {
                    retStatus = DL_ClearLUNMap(pMRP);
                }
                break;

            case MRCREATESERVER:
                {
                    retStatus = DEF_CreateServer(pMRP);
                }
                break;

            case MRDELETESERVER:
                {
                    retStatus = DEF_DeleteServer(pMRP);
                }
                break;

            case MRGETWSINFO:
                {
                    retStatus = DEF_GetWorkset(pMRP);
                }
                break;

            case MRSETWSINFO:
                {
                    retStatus = DEF_SetWorkset(pMRP);
                }
                break;

            case MRPDISKQLTIMEOUT:
                {
                    retStatus = DEF_BEQlogicTimeout(pMRP);
                }
                break;

            case MRCHGRAIDNOTMIRRORING:
                {
                    retStatus = DEF_ChgRAIDNotMirroringState(pMRP);
                }
                break;

            case MRPUTDEVCONFIG:
                {
                    retStatus = DEF_PutDeviceConfig(pMRP);
                }
                break;

            case MRSETATTR:
                {
                    retStatus = DEF_SetAttr(pMRP);
                }
                break;
            case MRGETEXTENDVINFO:
                {
                    retStatus = DEF_GetExtendVirtualData(pMRP);
                }
                break;
            case MRSETGLINFO:
                {
                    retStatus = GR_SetGeoLocation(pMRP);
                }
                break;

            case MRCLEARGLINFO:
                {
                    retStatus = GR_ClearGeoLocation(pMRP);
                }
                break;

            case MRALLDEVMISS:
                {
                    retStatus = GR_HandleAllDevMissAtOtherDCN(pMRP);
                }
                break;

            case MRSETVPRI:
                {
                    retStatus = DEF_SetVPri(pMRP);
                }
                break;

            case MRVPRI_ENABLE:
                {
                    retStatus = DEF_VPriEnable(pMRP);
                }
                break;

            case MRGETTGINFO:
                {
                    retStatus = DEF_GetTgInfo(pMRP);
                }
                break;
            case MRSETTGINFO:
                {
                    retStatus = DEF_SetTgInfo(pMRP);
                }
                break;
            case MRUPDSID:
                {
                    retStatus = DEF_UpdSID(pMRP);
                }
                break;
            case MRSETCHAP:
                {
                    retStatus = DEF_SetChap(pMRP);
                }
                break;
            case MRGETCHAP:
                {
                    retStatus = DEF_GetChap(pMRP);
                }
                break;
            case MRGETISNSINFO:
                {
                    retStatus = DEF_GetIsnsInfo(pMRP);
                }
                break;
            case MRSETISNSINFO:
                {
                    retStatus = DEF_iSNSSetInfo(pMRP);
                }
                break;
            case MRSETPR:
                {
                    retStatus = DEF_UpdatePres(pMRP);
                }
                break;
            case MRGETASYNC:
                {
                    retStatus = DEF_AsyncRep(pMRP);
                }
                break;
            case MRVIRTREDUNDANCY:
                {
                    retStatus = DEF_GetVdiskRedundancy(pMRP);
                }
                break;

            case MRSETBEPORTCONFIG:
                retStatus = DI_SetPortConfig(pMRP);
                break;

            case MRSAVEASYNCNV:
                retStatus = DEF_SaveAsyncNV(pMRP);
                break;

#if defined(MODEL_7000) || defined(MODEL_4700)
            case MRGETISEIP:
                {
                    retStatus = DEF_GetISEIP(pMRP);
                }
                break;
#endif /* MODEL_7000 || MODEL_4700 */

            case MRSWAPPIDS:
                retStatus = DEF_SwapPIDs(pMRP);
                break;

            case MREMULATEPAB:
                retStatus = DEF_EmulatePAB(pMRP);
                break;

            default:
                break;
        }
    }
    else
    {
        /*
         * Decode the function
         */
        switch (pMRP->function)
        {
            case MRRETRIEVEPR:
                {
                    retStatus = DEF_CfgRetrieve(pMRP);
                }
                break;
            case MRFEGETVINFO:
                {
                    retStatus = DEF_GetVirtualData(pMRP);
                }
                break;

            default:
                break;
        }
    }

    return retStatus;
}

void DEF_LogCopyLabelEvent(COR *pCOR)
{
    LOG_COPY_LABEL_EVENT_PKT copyLabelLog;

    /*
     * Send a log message
     */
    copyLabelLog.header.event = LOG_COPY_LABEL;

    if (pCOR->srcvdd != NULL)
    {
        copyLabelLog.data.svid = pCOR->srcvdd->vid;
    }
    if (pCOR->destvdd != NULL)
    {
        copyLabelLog.data.dvid = pCOR->destvdd->vid;
    }
    MSC_LogMessageStack(&copyLabelLog, sizeof(LOG_COPY_LABEL_EVENT_PKT));
}

/****************************************************************************/
/*                                                                          */
/* Formats the DLink Information for the CCB for the specified LDD and      */
/* places the information in the specified response buffer.                 */
/*                                                                          */
/* Inputs:                                                                  */
/*       LDD address to get DI information from.                            */
/*       rsp = CCB response buffer.                                         */
/*                                                                          */
/****************************************************************************/
extern void define_DIupdate(MR_PKT *mrp, LDD *ldd, MR_RSP_PKT *rsp);
void define_DIupdate(MR_PKT *mrp, LDD *ldd, MR_RSP_PKT *rsp)
{
    MRGETDLINK_RSP *old = (MRGETDLINK_RSP *)rsp;
    MRGETDLINK_GT2TB_RSP *new = (MRGETDLINK_GT2TB_RSP *)rsp;

    UINT16 gt2tb = (mrp->function != MRGETDLINK);   /* True if Greater than 2TB version of command */

    /* If terminated state, mark as INOP. */
    UINT8   ldd_state = (LDD_ST_PTERM > ldd->state) ? LDD_ST_INOP : ldd->state;

/* Define a conditional store depending upon gt2tb ... */
#define cs(dlink)    (*(gt2tb != TRUE ? &old->dlink : &new->dlink))

    UINT64  devcap = ldd->devCap;
    if (gt2tb != TRUE) {                        /* If old < 2TB function, limit it */
        if (devcap > 0xffffffffULL) {
// fprintf(stderr, "defbe.c: define_DIupdate, gt2tb != TRUE and devcap > 0xffffffffULL then make it 0xffffffff\n");
            devcap = 0xffffffff;
        }
    }

    if (ldd->class == LD_MLD)
    {
        /* XIOtech Link type device processing */
        cs(type) = 0;
        cs(linkStatus) = ldd_state;
// if (devcap == 0xffffffffULL) {
//   fprintf(stderr, "%s%s:%u define_DIupdate LD_MLD devcap=0xffffffffUL\n", FEBEMESSAGE, __FILE__, __LINE__);
// }
        if (gt2tb != TRUE) {
            old->devCap = devcap;
        }
        else
        {
            new->devCap = devcap;
        }
        cs(srcSerial) = (ldd->serial[8] << 24) | (ldd->serial[9] << 16) | (ldd->serial[10] << 8) | ldd->serial[11];
        cs(srcVBlock) = ldd->serial[6];         /* Source cluster or vblock */
        cs(srcVid) = (ldd->serial[4] << 8) | ldd->serial[5];    /* Vdisk ID of linked to remote device */
        cs(baseSerial) = ldd->baseSN;           /* Save base serial number */
        cs(baseVBlock) = ldd->baseCluster;      /* Save base cluster or vblock */
        cs(baseVid) = ldd->baseVDisk;           /* Save base VID */
        memcpy(cs(srcVName), &ldd->baseName, 8); /* Save VID name */
        bzero(cs(prodId), 16);                  /* Clear product ID field */
        bzero(cs(vendId), 8);                   /* Clear vendor ID */
        bzero(cs(rev), 4);                      /* Clear vendor ID */
        bzero(cs(serial), 12);
        if (ldd->pTPMTHead == 0)                /* If no first associated TPMT with LDD */
        {
            cs(srcVPort) = 0;                   /* Vport ID */
            cs(srcNumConn) = 0;                 /* Number of connections (paths) */
            cs(srcNodeWwn) = 0;                 /* Node WWN */
            cs(srcPortWwn1) = 0;                /* Port WWN 1 */
            cs(srcPortWwn2) = 0;                /* Port WWN 2 */
            bzero(cs(srcName), 8);              /* Clear node name field */
        }
        else
        {
            struct TPMT *t = ldd->pTPMTHead;
            int i = 1;

            for (;;)
            {
                t = t->pLDDNextTPMT;
                if (t == ldd->pTPMTHead)
                {
                    break;
                }
                i++;
            }
            cs(srcNumConn) = i;
//            t = ldd->pTPMTHead;
            cs(srcVPort) = t->pDTMT->td.xio.path;
            memcpy(cs(srcName), &t->pDTMT->td.xio.pName, 8);
            UINT64 srcPortWwn1 = t->pDTMT->nodeWWN;
            UINT64 srcPortWwn2 = 0;         /* Clear */
            cs(srcNodeWwn) = srcPortWwn1;
            for (;;)
            {
                t = t->pLDDNextTPMT;
                if (t == ldd->pTPMTHead)
                {
                    break;
                }
                if (srcPortWwn1 != t->pDTMT->nodeWWN)
                {
                    srcPortWwn2 = t->pDTMT->nodeWWN;
                    break;
                }
            }
            cs(srcPortWwn1) = srcPortWwn1;
            cs(srcPortWwn2) = srcPortWwn2;
        }
    }
    else if (ldd->class == LD_FTD)
    {
        /* Foreign Target type device processing */
        cs(type) = ldd->class - LD_MLD;     /* Normalize device class for MMC */
        cs(linkStatus) = ldd_state;         /* Save state */
// if (devcap == 0xffffffffULL) {
//   fprintf(stderr, "%s%s:%u define_DIupdate LD_FTD devcap=0xffffffffUL\n", FEBEMESSAGE, __FILE__, __LINE__);
// }
        if (gt2tb != TRUE) {
            old->devCap = devcap;
        }
        else
        {
            new->devCap = devcap;
        }
        bzero(cs(srcVName), 8);             /* Save VID name */
        cs(srcSerial) = 0;                  /* Clear serial number field */
        cs(srcVBlock) = 0;                  /* Source cluster or vblock */
        cs(srcVid) = 0;                     /* Vdisk ID of linked to remote device */
        cs(srcVPort) = 0;                   /* Vport ID */
        cs(srcNumConn) = 0;                 /* Number of connections (paths) */
        cs(srcNodeWwn) = 0;                 /* Node WWN */
        cs(srcPortWwn1) = 0;                /* Port WWN 1 */
        cs(srcPortWwn2) = 0;                /* Port WWN 2 */
        cs(baseSerial) = 0;                 /* Save base serial number */
        cs(baseVBlock) = 0;                 /* Save base cluster or vblock */
        cs(baseVid) = 0;                    /* Save base VID */
        bzero(cs(srcName), 8);              /* Clear node name field */
        memcpy(cs(prodId), &ldd->prodID, 16); /* Clear product ID field */
        memcpy(cs(vendId), &ldd->vendID, 8); /* Save vendor ID */
        memcpy(cs(rev), &ldd->rev, 4);      /* Save revision */
        memcpy(cs(serial), &ldd->serial, 12); /* Save serial number */
        if (ldd->pTPMTHead != 0)            /* If first associated TPMT with LDD */
        {
            struct TPMT *t = ldd->pTPMTHead;
            int i = 1;

            for (;;)
            {
                t = t->pLDDNextTPMT;
                if (t == ldd->pTPMTHead)
                {
                    break;
                }
                i++;
            }
            cs(srcNumConn) = i;
//            t = ldd->pTPMTHead;
            UINT64 srcPortWwn1 = t->pDTMT->nodeWWN;
            UINT64 srcPortWwn2 = 0;         /* Clear */
            cs(srcNodeWwn) = srcPortWwn1;
            for (;;)
            {
                t = t->pLDDNextTPMT;
                if (t == ldd->pTPMTHead)
                {
                    break;
                }
                if (srcPortWwn1 != t->pDTMT->nodeWWN)
                {
                    srcPortWwn2 = t->pDTMT->nodeWWN;
                    break;
                }
            }
            cs(srcPortWwn1) = srcPortWwn1;
            cs(srcPortWwn2) = srcPortWwn2;
        }
    }
}   /* End of define_DIupdate */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
