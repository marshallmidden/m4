/* $Id: GR_Misc.c 159664 2012-08-22 16:15:47Z marshall_midden $ */
/**
*******************************************************************************
**
** @file:      GR_Misc.c
**
** @brief:
**             This file contain the misc.functions related  to Georaid vdisk
**             failover/failback module.
**
**  Copyright (c) 2005-2010 XIOtech Corporation.  All rights reserved.
**
********************************************************************************
**/
#include "GR_Error.h"
#include "GR_LocationManager.h"
#include "CT_defines.h"
#include "cor.h"
#include "cm.h"
#include "scd.h"
#include "dcd.h"
#include "defbe.h"
#include "LOG_Defs.h"
#include "misc.h"
#include "MR_Defs.h"
#include "nvram.h"
#include "vdd.h"
#include "ddr.h"
#include "misc.h"

#include "XIO_Types.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include <stdio.h>

/*
******************************************************************************
** Private variables
******************************************************************************
*/
UINT8 aswapState;  // Used for event logging...

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static UINT32 GR_UpdateAbsLocationBit(VDD *pSrcVDD, VDD *pDestVDD);

/*
******************************************************************************
** Public function prototypes - defined in other files
******************************************************************************
*/
extern void CCSM_savenrefresh (void);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/*
******************************************************************************
**
**  @brief      This function updates the geo-Raid related information in
**              all the vdisks when the change of location is happened.
**
**              The bay location management module calls this function when
**              location change is happening (through configuration) for a
**              a disk or bay.
**
**              If the option is all-location-clear, means user has cleared all
**              geolocations. Means everything becomes non-geo Raid. In such
**              case, entire geo raid related information is to be updated in
**              all the vdisks.
**
**              If the option is location-change, geo_raid information is
**              updated based on the location value on each PDD, comprising
**              of a vdisk.
**
**  @param      locationChgOpt (all-location-clear / location-change)
**
**
**  @return     None
**
**  @attention
**  Since caller updates the nvram, nvram updation is avoided here.
******************************************************************************
*/

void GR_UpdateAllVddGeoInfo  (UINT8 locationChgOpt)
{
    VDD    *pVDD;
    UINT16 myIndex;

    /* For all VDDs Reset GeoRaid Bit. */
    if(locationChgOpt == GR_CLEAR_ALL_GEO_LOCATIONS)
    {
#if GR_MYDEBUG1
     fprintf(stderr,"<GR_UpdateAllVddGeoInfo>Location clearing -- update geoRaid bit in all vdds\n");
#endif
        for(myIndex = 0; myIndex < MAX_VIRTUAL_DISKS; myIndex++)
        {
            pVDD = gVDX.vdd[myIndex];

            if (pVDD != NULL)
            {
                /* Clear all geo-RAID information related bits in all the vdisks */
                pVDD->grInfo.permFlags = 0;
                pVDD->grInfo.tempFlags = 0;
            }
        }
    }
    else
    {
#if GR_MYDEBUG1
     fprintf(stderr,"<GR_UpdateAllVddGeoInfo>Location setting -- update geoRaid bit in all vdds\n");
#endif

        for(myIndex = 0; myIndex < MAX_VIRTUAL_DISKS; myIndex++)
        {
            pVDD = gVDX.vdd[myIndex];

            if(pVDD != NULL)
            {
#if 0 // Enable this piece of code to fix SAN-126/SAN-735
                /* Update Georaid info only when the VDISK is not of vlink. */
                if (BIT_TEST(pVDD->attr,VD_BVLINK) || BIT_TEST(pVDD->attr,VD_BVDLOCK))
                {
                    continue;
                }
#endif
                GR_UpdateVddGeoInfo(pVDD);
            }
        }
        /*
         * Now update the absolute location partner information in all the
         * partners.
         */
#if GR_MYDEBUG1
     fprintf(stderr,"<GR_UpdateAllVddGeoInfo>update abs location bit in all partner vdds\n");
#endif
        GR_UpdateAllVddPartners();
    }
}

/*
******************************************************************************
**
**  @brief      This function updates the geo-Raid related information in
**              specified vdisk.
**
**              This is called when any change occrus in bay location (by
**              bay location management module) and also when a vdisk  is
**              created or expanded.
**
**  @param      pVDD - VDD
**
**  @return     None
**
**  @attention
**  Since caller updates the nvram, nvram updation is avoided here.
******************************************************************************
*/

void GR_UpdateVddGeoInfo(VDD *pVDD)
{
#if GR_MYDEBUG1
     fprintf(stderr,"<GR_UpdateVddGeoInfo>Entering..updating geoRaid bit\n");
#endif
    if (GR_IsVDiskAtGeoLocations(pVDD))
    {
        /* Set Geo-RAID attributes, previously if this is not of Geo-RAID type */
        if(!GR_IS_GEORAID_VDISK(pVDD))
        {
#if GR_MYDEBUG1
     fprintf(stderr,"<GR_UpdateVddGeoInfo>Entering..setting geoRaid bit for vid=%u\n",
                (UINT32)(pVDD->vid));
#endif
            BIT_SET(pVDD->grInfo.permFlags, GR_VD_GEORAID_BIT);
            pVDD->grInfo.tempFlags = 0;
        }
    }
    else
    {
#if GR_MYDEBUG1
     fprintf(stderr,"<GR_UpdateVddGeoInfo>Entering..clearing geoRaid bit for vid=%u\n",
                (UINT32)(pVDD->vid));
#endif
        pVDD->grInfo.permFlags = 0;
        pVDD->grInfo.tempFlags = 0;
    }
    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
}

/*
******************************************************************************
**
**  @brief      This function updates the geo-Raid related in all the partners
**              of copy pairs.
**
**  @return     None
**
**  @attention
**  Since caller updates the nvram, nvram updation is avoided here.
******************************************************************************
*/

void GR_UpdateAllVddPartners(void)
{
    VDD   *pVDD;
    UINT16 iindex;

#if GR_MYDEBUG1
     fprintf(stderr,"<GR_UpdateAllVddPartners>Entering..\n");
#endif
    for (iindex = 0; iindex < MAX_VIRTUAL_DISKS; iindex++)
    {
        pVDD = gVDX.vdd[iindex];

        if(pVDD != NULL)
        {
            GR_UpdateVddPartners(pVDD);
        }
    }
}

/*
******************************************************************************
**
**  @brief      This function updates the geo-Raid  related information in the
**              destination partners of a source vdisk.
**
**  @param      pVDD - VDD
**
**  @return     None
**
**  @attention  Since caller updates the nvram, nvram updation is avoided here.
**
******************************************************************************
*/

void GR_UpdateVddPartners(VDD *pVDD)
{
    SCD  *pSCD;
    COR  *pCOR;
    VDD*  pDestVDD;

#if GR_MYDEBUG1
    fprintf(stderr,"<GR_UpdateVddPartners>Entering..for  vid=%u\n",
                (UINT32)(pVDD->vid));
#endif
    /* If is source device, compute diff location bit on its partners. */
    if (BIT_TEST(pVDD->attr, VD_BSCD))
    {
        pSCD = pVDD->pSCDHead;

        while (pSCD)
        {
            pCOR = pSCD->cor;
            pDestVDD = (pCOR != NULL) ? pCOR->destvdd:NULL;

            if (pDestVDD != NULL)
            {
#if GR_MYDEBUG1
     fprintf(stderr,"<GR_UpdateVddPartners>Calling UpdateAbsLocationBit.vid=%u.\n",
                (UINT32)(pVDD->vid));
#endif
                GR_UpdateAbsLocationBit(pVDD, pDestVDD);
            }
            pSCD = pSCD->link;
        }
    }
    /*
     * If this is destination vdisk and if does not have any source clear
     * diff location bit.
     */
    else if (!BIT_TEST(pVDD->attr, VD_BDCD))
    {
        BIT_CLEAR(pVDD->grInfo.permFlags, GR_VD_ABSOLUTE_GEOPARNTER_BIT);
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
    }
}

/*
******************************************************************************
**
**  @brief      This function updates the geo-Raid related information in
**              specified vdisk pair related to copy/mirror operation at
**              copy/manager configuration change initiated by user.
**
**              This is called when any user issues  copy/mirror or any
**              other command. Currently this is called when any mirror comes
**              to sync  with its source partner and when user issues  manual
**              Since this situation occurs in many instances, a flag is set
**              in its corresponding COR indicating that this is the request
*               issued by the user.
**
**  @param      pCOR - COR
**              operation - command (swap or break etc...)
**
**  @return     None
**
******************************************************************************
*/
void GR_UpdateGeoInfoAtCmdMgrReq(COR *pCOR, UINT8 operation)
{
    VDD  *pDestVDD;
    VDD  *pSrcVDD;
    VDD  *pDestVDD1;
    VDD  *pSrcVDD1;
    DCD  *pDCD;
    SCD  *pSCD;
    COR  *pCOR1;
    UINT8 configChange = FALSE;

    pDestVDD = pCOR->destvdd;
    pSrcVDD  = pCOR->srcvdd;

    if (pSrcVDD == NULL || pDestVDD == NULL)
    {
        return;
    }

    /*
     * Check whether this command can be executed, in the event any
     * autoswap bits are set on source destination VDDs.
     */
    switch(operation)
    {
        case MVCUPDATE:
        case MVCMOVEVD:     /* Move vdisk */
            break;

        case MVCCOPYCONT:   /* Copy and mirror command */
            break;

        case MVCXSPECCOPY:  /* Break off specified copy */
#if GR_MYDEBUG1
            fprintf(stderr,"<GR_UpdateGeoInfoOnCmdMgrReq>copy specific break command srvid=%u destvid=%u\n",(UINT32)(pSrcVDD->vid),(UINT32)(pDestVDD->vid));
#endif
        case MVCABORTCOPY:  /* abort copy */
#if GR_MYDEBUG1
            fprintf(stderr,"<GR_UpdateGeoInfoOnCmdMgrReq>copy abort command srvid=%u destvid=%u\n",(UINT32)(pSrcVDD->vid),(UINT32)(pDestVDD->vid));
#endif
            /*
             * Check if the source is the one that is undergoing autoswap
             * or autoswapback or already swapped
             */
            if (GR_IS_VD_ASWAP_INPROGRESS(pSrcVDD) ||
                GR_IS_VD_ASWAPBACK_INPROGRESS(pSrcVDD) ||
                GR_IS_VD_HYSTERESIS_ENABLED(pSrcVDD) ||
                GR_IS_VD_ALREADY_ASWAPPED(pSrcVDD))
            {
                /*
                 * Ensure that this the destination participated in
                 * autoswap with the above source VDD.
                 * This check is needed, because user may break any
                 * partner that eihter participated in autoswap or not.
                 */
                if (GR_IS_VD_INSYNC_AT_SRCFAIL(pDestVDD) ||
                     !GR_IS_VD_ASWAPBACK_IN_READY_STATE(pDestVDD) ||
                     GR_IS_VD_ASWAPBACK_PENDING(pDestVDD))
                {
#if GR_MYDEBUG1
                    fprintf(stderr,"<GR_UpdateGeoInfoOnCmdMgrReq>3.Clearing swap bits srcvid=%u destvid=%u\n",
                                 (UINT32)(pSrcVDD->vid),(UINT32)(pDestVDD->vid));
#endif

                    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                    pSrcVDD->grInfo.tempFlags = 0;
                    pDestVDD->grInfo.tempFlags = 0;

                    /* Aswap just started -- not yet completed autoswap */
                    BIT_CLEAR(pSrcVDD->grInfo.permFlags,GR_VD_ASWAP_BIT);
                    BIT_CLEAR(pDestVDD->grInfo.permFlags,GR_VD_ASWAPBACK_PENDING_BIT);
                    GR_SendAswapLogEvent( pSrcVDD, pDestVDD, GR_ASWAPBACK_CANCELLED);
                    configChange = TRUE;
                }
            }
            break;

        case MVCCOPYBRK:    /* Copy and break */
        case MVCPAUSECOPY:
        case MVCRESUMECOPY:
            break;

        case MVCCOPYSWAP:   /* Copy and swap command */
        case MVCSWAPVD:     /* swap vdisks */
#if GR_MYDEBUG1
            fprintf(stderr,"<GR_UpdateGeoInfoOnCmdMgrReq>userswap srcvid=%u destvid=%u\n",(UINT32)(pSrcVDD->vid),(UINT32)(pDestVDD->vid));
#endif
            /* Check if it is the source which was already autoswapped */
            if (GR_IS_VD_ALREADY_ASWAPPED(pSrcVDD))
            {
                /* Clear the AUTOSWAP bit in the source vdisk. */
                BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                BIT_CLEAR(pSrcVDD->grInfo.permFlags,GR_VD_ASWAP_BIT);
#if GR_MYDEBUG1
                fprintf(stderr,"<GR_UpdateGeoInfoOnCmdMgrReq>Clearing aswap bit on src=%u\n",
                              (UINT16)(pSrcVDD->vid));
#endif
                configChange = TRUE;
                pSrcVDD->grInfo.tempFlags = 0;

                /*
                 * Search for destination on which wait-for-sync bit is set,
                 * and clear that bit. If not available something wrong !!!
                 */
                for (pSCD = pSrcVDD->pSCDHead; pSCD != NULL; pSCD = pSCD->link)
                {
                    pCOR1    = pSCD->cor;
                    pDestVDD1 = pCOR1->destvdd;
                    if (pDestVDD1 && GR_IS_VD_ASWAPBACK_PENDING(pDestVDD1))
                    {
                        BIT_CLEAR(pDestVDD1->grInfo.permFlags,GR_VD_ASWAPBACK_PENDING_BIT);
                        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                        GR_SendAswapLogEvent( pSrcVDD, pDestVDD1, GR_ASWAPBACK_CANCELLED);

                        pDestVDD1->grInfo.tempFlags = 0;
#if GR_MYDEBUG1
                        fprintf(stderr,"<GR_UpdateGeoInfoOnCmdMgrReq>Clearing aswapback wait bit on dest=%u\n",
                                         (UINT16)(pDestVDD1->vid));
#endif
                        break;
                    }
                }
            }
            /*
             * ELSE,check if this source is waiting for auto swapback.
             * Means this is the destination ,and in turn source for
             * the another copy,  and  was already autoswapped with its
             * source -- 2nd cascade copy/mirror-pair
             */
            else if (GR_IS_VD_ASWAPBACK_PENDING(pSrcVDD))
            {
                /* Clear WAIT_FOR_SYNC bit on this destination cum source VDD */
                BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                BIT_CLEAR(pSrcVDD->grInfo.permFlags,GR_VD_ASWAPBACK_PENDING_BIT);
                pSrcVDD->grInfo.tempFlags = 0;
#if GR_MYDEBUG1
                fprintf(stderr,"<GR_UpdateGeoInfoOnCmdMgrReq>Clearing aswapback wait bit on dest-src=%u\n",(UINT16)(pSrcVDD->vid));
#endif
                configChange = TRUE;

                /*
                 * Get its source VDD.
                 * Clear the AUTOSWAP bit on this source VDD, if this bit
                 * is not set,something wrong  !!!
                 */
                if ((pDCD = pSrcVDD->pDCD) != NULL && (pCOR1 = pDCD->cor) != NULL)
                {
                    pSrcVDD1 = pCOR1->srcvdd;
                    if (pSrcVDD1 != NULL)
                    {
                        BIT_CLEAR(pSrcVDD1->grInfo.permFlags,GR_VD_ASWAP_BIT);
                        pSrcVDD1->grInfo.tempFlags = 0;
                        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                        GR_SendAswapLogEvent( pSrcVDD1, pSrcVDD, GR_ASWAPBACK_CANCELLED);
#if GR_MYDEBUG1
                         fprintf(stderr,"<GR_UpdateGeoInfoOnCmdMgrReq>Clearing aswap bit on src=%u\n",
                                         (UINT16)(pSrcVDD1->vid));
#endif
                    }
                }
            }
            break;

        default:
           break;
    }
    if (configChange)
    {
        GR_SaveToNvram(pSrcVDD, pDestVDD);
    }
}

/*
******************************************************************************
**
**  @brief      This function updates the absolute location in the destination
**              partner of a source vdisk.
**
**              It calls the bay location management provided routine to check
**              whether the source and destination partners are entirely at
**              different location. If so, set the absolute location bit.
**
**  @param      pSrcVDD  - Source VDD
**              pDestVDD - Destination VDD
**
**  @attention  Since caller updates the nvram, nvram updation is avoided here.
**
******************************************************************************
*/
static UINT32 GR_UpdateAbsLocationBit(VDD *pSrcVDD, VDD *pDestVDD)
{
#if GR_MYDEBUG1
     fprintf(stderr,"<GR_UpdateAbsLocationBit>Entering.. srcvid=%u destvdd=%u\n",
                (UINT32)(pSrcVDD->vid), (UINT32)(pDestVDD->vid));
#endif
    if (GR_IS_GEORAID_VDISK(pSrcVDD) && GR_IS_GEORAID_VDISK(pDestVDD))
    {
        if (GR_IsGeoLocationIntersected(pSrcVDD, pDestVDD) == FALSE)
        {
#if GR_MYDEBUG1
     fprintf(stderr,"<GR_UpdateAbsLocationBit>setting abs georaid bit.. srcvid=%u destvdd=%u\n",
                (UINT32)(pSrcVDD->vid), (UINT32)(pDestVDD->vid));
#endif
            /*
             * The locations of source and its partner are not overlapped -
             * Hence both are at different locations
             */
            BIT_SET(pDestVDD->grInfo.permFlags,GR_VD_ABSOLUTE_GEOPARNTER_BIT);
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            return TRUE;
        }
#if GR_MYDEBUG1
        fprintf(stderr,"<GR_UpdateAbsLocationBit>Clearing AbS partner bit in vid=%u srcvid=%u\n",
                 (UINT32)(pDestVDD->vid),(UINT32)(pSrcVDD->vid));
#endif
        BIT_CLEAR(pDestVDD->grInfo.permFlags,GR_VD_ABSOLUTE_GEOPARNTER_BIT);
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
    }
    return FALSE;
}

/*
******************************************************************************
**
**  @brief      This function  verifies whether any autoswap is inprogrees on
**              any of the vdisk.
**
**              This is used by the bay location management to ensure that no-
**              autoswap is in progress when changing the bay location.
**
**  @param     None
**
**  @return    TRUE  - Autoswap/autoswap back is in progress on atleast one
**                     vdisk.
**             FALSE - No autoswap/autoswap back on any of the vdisk.
**
******************************************************************************
*/

UINT8 GR_IsAnyAutoSwapInProgress(void)
{
    UINT16 myIndex;
    VDD   *pVDD=NULL;

#if GR_MYDEBUG1
    fprintf(stderr,"<GR_IsAnyAutoSwapInProgress>Entering..\n");
#endif
    for (myIndex = 0; myIndex < MAX_VIRTUAL_DISKS; myIndex++)
    {
        pVDD = gVDX.vdd[myIndex];

        /*
         * Don't bay location change as long as autoswap / autoswapback is in progress
         * In other cases it is OK
         */
        if (pVDD != NULL &&
            (GR_IS_VD_ASWAP_INPROGRESS(pVDD) || GR_IS_VD_ASWAPBACK_INPROGRESS(pVDD)))
        {
            return TRUE;
        }
    }
#if GR_MYDEBUG1
     fprintf(stderr,"<GR_IsAnyAutoSwapInProgress>out return val=%u..\n",(UINT32)ret);
#endif
    return FALSE;
}
/*
******************************************************************************
**
**  @brief      This function  verifies whether  autoswap or autoswap back is
**              is in progrees on a specified Vdisk.
**
**  @param      pSrcVDD - source vdisk.
**
**  @return     TRUE  - Autoswap/autoswap back in progress
**              FALSE - No autoswap/autoswap back is in progress
**
******************************************************************************
*/

UINT32 GR_IsAutoSwapInProgress(VDD *pSrcVDD)
{
    if (GR_IS_VD_ASWAP_INPROGRESS(pSrcVDD)  || GR_IS_VD_ASWAPBACK_INPROGRESS(pSrcVDD))
    {
        return TRUE;
    }
    return FALSE;
}
/*
******************************************************************************
**
**  @brief      This function  verifies whether the operating state of the
**              specified vdisk is valid for performing the autoswap.
**
**              This is used by the  virtual layer completion routine when I/O
**              error is found on GeoRaid source vdisk having mirrors.
**
**  @param      PSrcVDD - Source Vdisk.
**
**  @return     TRUE  - Valid
**              FALSE - Not valid
**
******************************************************************************
*/

UINT32 GR_IsValidOpStateForAutoSwap(VDD *pSrcVDD)
{
    if (pSrcVDD->grInfo.vdOpState == GR_VD_INOP)
    {
        return TRUE;
    }
#if GR_MYDEBUG1
    fprintf(stderr,"<GR_IsValidOpStateForAutoSwap>vid=%x - opstate=%x\n",
          (UINT32)(pSrcVDD->vid), (UINT32)(pSrcVDD->grInfo.vdOpState));
#endif
    return FALSE;
}

/**
******************************************************************************
**
******************************************************************************
**/
void GR_SendAswapLogEvent(VDD *pSrcVDD, VDD *pDestVDD, UINT8 state)
{
    aswapState = state;
    GR_LogEvent(GR_ASWAP_STATE_EVENT, pSrcVDD, pDestVDD);
}

/**
******************************************************************************
**
**  @brief      This function returns and sets up the mirror type of a copy-
**              mirror vdisk pair. The mirror types are as follows
**
**              If the source or destination is not GeoRaid vdisks, the mirror
**              is normal (non-GeoRaid type)
**              If both source and destination VDDs are of GeoRaid type, and
**              both the vdisks are created at different geolocations, the
**              mirror type is absolute GeoRaid mirror.
**
**              If both source and destination VDDs are GeoRaid type, but the
**              both source and destination VDDs having overlapping locations,
**              the mirror type is GeoRaid mirror.
**
**  @param      pMRP -- MPR packet passed during copy-mirror setup.
**
**  @return     Mirror type
******************************************************************************
**/
UINT8 GR_SetGeoMirrorSetType(MR_PKT *pMRP)
{
    MRVDISKCONTROL_REQ *pVdiskControlInfo;
    VDD                *pSrcVDD;
    VDD                *pDestVDD;
    UINT8               mirrorSetType = 0;

    /* get the mirror partner configuration from MRP. */
    pVdiskControlInfo = (MRVDISKCONTROL_REQ*)pMRP->pReq;

    pSrcVDD = gVDX.vdd[pVdiskControlInfo->svid];
    pDestVDD = gVDX.vdd[pVdiskControlInfo->dvid];

    if (GR_UpdateAbsLocationBit(pSrcVDD, pDestVDD) == TRUE)
    {
        GR_SaveToNvram(pSrcVDD, pDestVDD);
    }

    if (!GR_IS_GEORAID_VDISK(pSrcVDD) || !GR_IS_GEORAID_VDISK(pDestVDD))
    {
        mirrorSetType = GR_MIRRORSET_NON_GEORAID;
    }
    else if(GR_IS_PARNTER_AT_DIFFERENT_LOCATION(pDestVDD))
    {
        mirrorSetType = GR_MIRRORSET_ABSOLUTE_GEORAID;
    }
    else
    {
        mirrorSetType = GR_MIRRORSET_GEORAID;
    }

    return (mirrorSetType);
}

/**
******************************************************************************
**
**  @brief      This function verifies whether any I/O is pending on the src
**              vdisk  as well as any of  the  destination  vdisk associated
**              with the source vdisk.
**
**  @param      pSrcVDD -- Pointer to source vdisk.
**
**  @return     FALSE -- No I/O is pending
**              TRUE  -- I/O is pending.
******************************************************************************
**/
UINT32 GR_AnyIosArePending(VDD *pSrcVDD)
{
    SCD *pSCD;
    COR *pCOR;

    if (pSrcVDD->qd == 0)
    {
        /* No I/O is pending on source..now search all destinations */
        for (pSCD = pSrcVDD->pSCDHead; pSCD != NULL; pSCD = pSCD->link)
        {
            if( (pCOR=pSCD->cor) != NULL)
            {
                if ((pCOR->destvdd) && (pCOR->destvdd->qd != 0))
                {
                    /* I/O is pending on atleast one destination vdisk. */
                    fprintf(stderr,"<VIJAY>IOs pending=%x vid=%x\n",pCOR->destvdd->qd, pCOR->destvdd->vid);
                    break;
                }
            }
        }
        if (pSCD == NULL)
        {
            /* No I/O is pending on any of the destination vdisk */
            return FALSE;
        }
    }
    return TRUE;
}

/**
******************************************************************************
**
**  @brief      This function  updates  the part-II  NVRAM of the  Master DCN
**              with GeoRaid changed configuration. It sends the message with
**              configuration information to the other DCN to enable updating
**              GeoRaided   changed information  of the  vdisks  owned by the
**              sending DCN.
**
**  @param      pSrcVDD  -- Pointer to source vdisk.
**              pDestVDD -- Pointer to destination vdisk.
**
**  @return     None
******************************************************************************
**/

void GR_SaveToNvram(VDD *pSrcVDD, VDD *pDestVDD)
{
    GR_NVRAM_PKT nvramPkt;
    UINT32 save_g0;
    UINT32 save_g1;
    UINT32 save_g2;
    UINT32 save_g3;
    UINT32 save_g4;

    if (BIT_TEST(K_ii.status, II_MASTER) == TRUE)
    {
        /*
         * Save into Master's NVRAM
         * Slave NVram refresh is done at its own time.
         */
        NV_P2UpdateConfig();
        //NV_SendRefresh();
    }

    /* See if running n-way (>1). */
    if (BIT_TEST(K_ii.status, II_CCBREQ))
    {
        /*
         * Both DCNs are alive. Send a broadcast message to other DCN, informing to
         * update the configuration of VDDs owned by this controller.
         */
        nvramPkt.nvram_event.len  = sizeof(GR_NVRAM_PKT);
        nvramPkt.nvram_event.type = CCSM_ET_GRS;            /*Event type code*/
        nvramPkt.nvram_event.fc   = CCSM_GRS_NVRAM;         /* Event function code*/
        nvramPkt.nvram_event.sendsn     = K_ficb->cSerial;

        if (pSrcVDD != NULL)
        {
            nvramPkt.srcVid =  pSrcVDD->vid;
            nvramPkt.srcVal.vdOpState = pSrcVDD->grInfo.vdOpState;
            nvramPkt.srcVal.permFlags = pSrcVDD->grInfo.permFlags;
        }
        else
        {
            nvramPkt.srcVid =  0xffff;
        }
        if (pDestVDD != NULL)
        {
            nvramPkt.destVid =  pDestVDD->vid;
            nvramPkt.destVal.vdOpState = pDestVDD->grInfo.vdOpState;
            nvramPkt.destVal.permFlags = pDestVDD->grInfo.permFlags;
        }
        else
        {
            nvramPkt.srcVid =  0xffff;
        }

        /* Save the global registers used by report event function */
        save_g0 = g0;            /* used for record address */
        save_g1 = g1;            /* used for extension size */
        save_g2 = g2;            /* used to store event type */
        save_g3 = g3;            /* used to store broadcast type*/
        save_g4 = g4;            /* used to store serial number*/

        if (BIT_TEST(K_ii.status, II_MASTER) == TRUE)
        {
            DEF_ReportEvent(&nvramPkt, sizeof(GR_NVRAM_PKT), PUTDG, DGBTSLAVE, 0);
        }
        else
        {
            DEF_ReportEvent(&nvramPkt, sizeof(GR_NVRAM_PKT), PUTDG, DGBTMASTER, 0);
        }

        /* Restore back the global registers. */
        g0 = save_g0;
        g1 = save_g1;
        g2 = save_g2;
        g3 = save_g3;
        g4 = save_g4;
    }
}

/**
******************************************************************************
**
**  @brief      This function receives and processes the Georaid configuration
**              updation message sent from other controller(configuration cha-
**              nges related to vdisks owned by the other DCN.
**
**  @param      pNvramPkt  -- Pointer to nvram configuration data.
**
**  @return     None
******************************************************************************
**/
void GR_NvramUpdatePkt(GR_NVRAM_PKT *pNvramPkt)
{
    VDD *pSrcVDD;
    VDD *pDestVDD;

    /* Message arrived from  the other DCN */

    /*
     * Check, if any config change in source VDD geoRaid information
     * owned by other controller.
     */
    if (pNvramPkt->srcVid != 0xffff)
    {
        /* Update geoRaid config on this DCN. */
        pSrcVDD = gVDX.vdd[pNvramPkt->srcVid];
        pSrcVDD->grInfo.vdOpState = pNvramPkt->srcVal.vdOpState;
        pSrcVDD->grInfo.permFlags = pNvramPkt->srcVal.permFlags;
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
    }

    /*
     * Check, if any config change in dest VDD geoRaid information
     * owned by other controller.
     */
    if (pNvramPkt->destVid != 0xffff)
    {
        /* Update geoRaid config on this DCN */
        pDestVDD = gVDX.vdd[pNvramPkt->destVid];
        pDestVDD->grInfo.vdOpState = pNvramPkt->destVal.vdOpState;
        pDestVDD->grInfo.permFlags = pNvramPkt->destVal.permFlags;
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
    }

    /*
     * Save in Master NVRAM.
     * Slave NVram refresh is done at its own time.
     */
    if (BIT_TEST(K_ii.status, II_MASTER) == TRUE)
    {
        NV_P2UpdateConfig();
    }
}

void GR_LogEvent(UINT8 eventType, VDD *pSrcVDD, VDD *pDestVDD)
{
    LOG_GR_EVENT_PKT grLog;

    grLog.header.event = LOG_GR_EVENT;
    grLog.data.eventType = eventType;

    switch(eventType)
    {
        case GR_VLINK_ASSOCIATE:
             grLog.data.svid = pSrcVDD->vid;
             break;

        case GR_VLINK_CPYMIRROR:
             grLog.data.svid = pSrcVDD->vid;
             grLog.data.dvid = pDestVDD->vid;
             break;

        case GR_ASWAP_STATE_EVENT:
             grLog.data.aswapState = aswapState;

             if (pSrcVDD != NULL)
             {
                 grLog.data.svid  = pSrcVDD->vid;
             }
             else
             {
                 grLog.data.svid = -1;
             }
             if (pDestVDD != NULL)
             {
                 grLog.data.dvid = pDestVDD->vid;
             }
             else
             {
                 grLog.data.dvid = -1;
             }
             break;

        default:
             break;
    }

    /* Note: message is short, and L$send_packet copies into the MRP. */
    MSC_LogMessageStack(&grLog, sizeof(LOG_GR_EVENT_PKT));
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
