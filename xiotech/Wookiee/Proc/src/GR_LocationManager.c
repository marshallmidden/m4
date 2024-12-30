/* $Id: GR_LocationManager.c 160802 2013-03-22 16:39:33Z marshall_midden $ */
/**
*******************************************************************************
**
** @file:    GR_LocationManager.c
**
** @brief:   This file basically deals with the location code related issues
**
**  Copyright (c) 2005-2010 Xiotech Corporation.  All rights reserved.
**
********************************************************************************
**/
#include "GR_LocationManager.h"
#include "GR_Error.h"
#include "cev.h"
#include "defbe.h"
#include "nvram.h"
#include "LL_LinuxLinkLayer.h"
#include "LOG_Defs.h"
#include "online.h"
#include "pdd.h"
#include "XIO_Types.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/
#define NO_HS_PRESENT     0   /* No HotSpare present matching location code */
#define HS_PRESENT        1   /* HotSpare present matching location code    */
#define NON_GEO_VDISK     0   /* Non-Geo VDisk                              */
#define GEO_VDISK         1   /* Geo VDisk                                  */
#define MAX_GEO_LOCATIONS 255 /* Maximum GEO locations                      */
#define NON_GEORAID_VDISK 0   /* Non-Geo Raid Vdisk                         */

#define GEORAID_VDISK_ACROSS_MULTIPLE_LOCATIONS 1
                              /* Geo-Raid vdisk across multiple locations   */

#define ABSOULTE_GEORAID_VDISK 2
                             /* Single location Geo-Raid Vdisk              */

#define DEFAULT_NON_GEO_LOCATION  0 /* Default Non-Geo location             */


/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/**
** The following structure aids us in maintaining the WWN(s) and the location
** code(s) of the Disk Bay(s) and its associated drive(s). This is useful
** when a Bay is powercycled or when one drive is removed from one of the Bays
** and inserted into the same Bay or a different Bay. In this case, as per the
** existing functionality, we loose their location codes (if they don't have
** RAIDs on them). But, for GEORAID,  we need to maintain this information.
** The following structure does the same.
** This is internal only to the GEORAID feature.
**/
GRS_PDD                   gGRSpdd[MAX_PHYSICAL_DISKS];

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
static void GR_ProcessInsertion (PDD* , UINT8, UINT16);
static UINT8 GR_GetFreePlaceHolder(void);
static void GR_UpdateLocationCodeInLocalCopy(PDD*, UINT8);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      This function sets the default location of each PDD to zero
**
**  @param      NIL
**
**  @return     NIL
**
******************************************************************************
**/
void GR_SetDefaultLocation (void)
{
    UINT32    inner_indx;
    UINT16    enclIndex;          /* Index into enclosure list        */
    PDD       *pPDD = NULL;

    /*
    ** Set the default location code for drive bay.
    */
    for (enclIndex = 0; enclIndex < MAX_DISK_BAYS; enclIndex++)
    {
       pPDD = E_pddindx[enclIndex];

       if (pPDD != NULL)
       {
           pPDD->geoLocationId = DEFAULT_NON_GEO_LOCATION;
       }
    }

    /*
    ** Set the default Non-Geo location code for each physical disk
    */
    for (inner_indx = 0; inner_indx < MAX_PHYSICAL_DISKS; inner_indx++)
    {
        if (gPDX.pdd[inner_indx] != NULL)
        {
            gPDX.pdd[inner_indx]->geoLocationId = DEFAULT_NON_GEO_LOCATION;
        }
    }
}

/**
******************************************************************************
**
**  @brief      This function provides a standard means of setting the Geo
**              location Id for the given drive bay and the physical disks
**              owned by the given drive bay
**
**  @param      pMRP   : MRP packet
**
**  @return     UINT32 status
**
******************************************************************************
**/
UINT32 GR_SetGeoLocation(MR_PKT *pMRP)
{
    UINT32      status = DEOK;
    UINT32      indx;
    UINT16      bayId;
    UINT16      i;
    UINT8       locationCode;
    UINT8       found = FALSE;
    PDD         *pBayPDD = NULL;
    PDD         *pdd = NULL;
    MR_SET_GEO_LOCATION_REQ*    pReq = (MR_SET_GEO_LOCATION_REQ*)(pMRP->pReq);
    MR_SET_GEO_LOCATION_RSP*    pRsp = (MR_SET_GEO_LOCATION_RSP*)(pMRP->pRsp);

    /*
    ** Check if any Auto swap is in progress. This is just to avoid
    ** the possible changes in GEORAID properties of the vdisks when
    ** auto swap is in progress.
    ** We set the GEO location only when auto swap is NOT in progress
    */
    if (!GR_IsAnyAutoSwapInProgress())
    {
        /*
        ** Get the specified Bay Id and Location Code from the MRP
        */
        bayId = pReq->bayId;
        locationCode = pReq->locationId;

        /*
        ** Get the Bay PDD and check if it exists
        */
        if ((pBayPDD = gEDX.pdd[bayId]) != NULL)
        {
            /*
            ** Set the geo location Code for the Bay
            */
            pBayPDD->geoLocationId = locationCode;

            /*
            ** Set the Bay location code in the local copy
            */
            for (i = 0; i < MAX_PHYSICAL_DISKS; i++)
            {
                if (pBayPDD->wwn == gGRSpdd[i].wwn )
                {
                    gGRSpdd[i].locationCode = locationCode;
                    found = TRUE;
                    break;
                }
            }
            if (found == FALSE)
            {
                i = GR_GetFreePlaceHolder();

                if (i < MAX_PHYSICAL_DISKS)
                {
                    gGRSpdd[i].wwn = pBayPDD->wwn;
                    gGRSpdd[i].locationCode = locationCode;
                }
            }

            /*
            ** Loop through all the physical disks
            */
            for (indx = 0; indx < MAX_PHYSICAL_DISKS; indx++)
            {
                /*
                ** Check if the PDD exists
                */
                if ((pdd = gPDX.pdd[indx]) != NULL)
                {
                    /*
                    ** Set the location Code for the disk whose ses is
                    ** same as the given Bay Id.
                    **
                    ** Update the local copy with the WWN of the drive and location
                    ** code. This is useful  to log a message when a drive
                    ** is inserted into the Bay. If we don't maintain this as a
                    ** local copy, we loose the PDD of the drive once it is removed
                    ** and therefore loose its previous location code to know that
                    ** the insertion happend within the bay(s) with the same location
                    ** Code or into a bay with a different location Code.
                    */
                    if (pdd->ses == bayId)
                    {
                        pdd->geoLocationId = locationCode;
                        GR_UpdateLocationCodeInLocalCopy (pdd, locationCode);
                    }
                }
            }

            /*
            ** Find if any HotSpare exists with the same location code
            ** If not, update the response accordingly to log a message
            */
            pRsp->anyHotSpares = GR_CheckForHotSparePresence(locationCode);

            /*
            ** Update the geo-Raid related information in
            ** all the vdisks when the change of location is happened
            */
            GR_UpdateAllVddGeoInfo(GR_SET_GEO_LOCATION);

            /*
            ** Update the P2 NVRAM
            */
            NV_P2UpdateConfig();
        }
        else
        {
            /*
            ** No Bay exists with the given ID.
            ** So, set it as non-existent device
            */
            status = DENONXDEV;
        }
    }
    else
    {
         /*
         ** Set the status to Auto swap in progress
         */
         status = DEASWAPINPROGRESS;
    }

    /*
    ** Return the status
    */
    return (status);
}

/**
******************************************************************************
**
**  @brief     This function updates the location code of the local copy
**
**  @param     PDD*  : pPDD
**             UINT8 : locationCode
**
**  @return    NIL
**
******************************************************************************
**/
static void GR_UpdateLocationCodeInLocalCopy(PDD* pPDD, UINT8 locationCode)
{
    UINT16   i;

    /*
    ** Search for the matching entry in the local copy and update its
    ** location code
    */
    for (i = 0; i < MAX_PHYSICAL_DISKS; i++)
    {
         if (pPDD->wwn == gGRSpdd[i].wwn)
         {
             gGRSpdd[i].locationCode = locationCode;
             break;
         }
    }
}

/**
******************************************************************************
**
**  @brief      This function provides a standard means of clearing the Geo
**              location code for all the drive bays and the associated
**              physical disks
**
**  @param      pMRP   : MRP packet
**
**  @return     UINT32 status
**
******************************************************************************
**/
UINT32 GR_ClearGeoLocation(MR_PKT *pMRP UNUSED)
{
    UINT32      status = DEOK;
    UINT32      indx;
    UINT16      enclIndex;
    PDD         *pBayPDD = NULL;
    PDD         *pdd = NULL;

    /*
    ** Check if any Auto swap is in progress. This is just to avoid
    ** the possible changes in GEORAID properties of the vdisks when
    ** auto swap is in progress.
    ** We clear the GEO location only when auto swap is NOT in progress
    */
    if (!GR_IsAnyAutoSwapInProgress())
    {
        /*
        ** Clear the location code of each drive bay that exists.
        */
        for (enclIndex = 0; enclIndex < MAX_DISK_BAYS; enclIndex++)
        {
            pBayPDD = E_pddindx[enclIndex];

            /*
            ** Check if Bay PDD exists
            */
            if (pBayPDD != NULL)
            {
                /*
                ** Clear the geo location Code for the Bay
                */
                pBayPDD->geoLocationId = DEFAULT_NON_GEO_LOCATION;
            }
        }

        /*
        ** Loop through all the associated physical disks
        */
        for (indx = 0; indx < MAX_PHYSICAL_DISKS; indx++)
        {
            /*
            ** Check if the PDD exists
            */
            if ((pdd = gPDX.pdd[indx]) != NULL)
            {
                /*
                ** Clear the location Code of the disk.
                ** Update the local copy with the WWN of the drive and location
                ** code. This is useful  to log a message when a drive
                ** is inserted into the Bay. If we don't maintain this as a
                ** local copy, we loose the PDD of the drive once it is removed
                ** and therefore loose its previous location code to know that
                ** the insertion happend within the bay(s) with the same location
                ** Code or into a bay with a different location Code.
                */
                pdd->geoLocationId = DEFAULT_NON_GEO_LOCATION;
            }
        }

        /*
        ** Clear the location code of both bays and its associated devices
        ** from the local copy
        */
        for (indx = 0; indx < MAX_PHYSICAL_DISKS; indx++)
        {
            if (gGRSpdd[indx].wwn != 0)
            {
                gGRSpdd[indx].locationCode = DEFAULT_NON_GEO_LOCATION;
            }
        }

        /*
        ** Update the geo-Raid related information in
        ** all the vdisks when the change of location is happened
        */
        GR_UpdateAllVddGeoInfo(GR_CLEAR_ALL_GEO_LOCATIONS);

        /*
        ** Update the P2 NVRAM
        */
        NV_P2UpdateConfig();
    }
    else
    {
        status = DEASWAPINPROGRESS;
    }

    /*
    ** Return the status
    */
    return (status);
}

/**
******************************************************************************
**
**  @brief      This function finds if any HotSpare exists with the
**              the specified location code.
**
**  @param      locationCode
**
**  @return     status
**
******************************************************************************
**/
UINT8 GR_CheckForHotSparePresence (UINT8 locationCode)
{
    PDD*       pPDD = NULL;
    UINT8      status = NO_HS_PRESENT;
    UINT32     indx;

    /*
    ** Find if any HotSpare exists with the same location
    ** code as the failing PDD
    */
    for (indx = 0; indx < MAX_PHYSICAL_DISKS; indx++)
    {
        /*
        ** Check if the PDD exists and
        ** any HotSpare exists with the same location code
        */
        if (((pPDD = gPDX.pdd[indx]) != NULL) &&
             (pPDD->devClass == PD_HOTLAB) &&
             (pPDD->geoLocationId == locationCode))
        {
            status = HS_PRESENT;
            break;
        }
    }
    return (status);
}

/**
******************************************************************************
**
**  @brief      This function provides a standard means of verifying whether
**              the specified devices are in the same Geo location.
**
**  @param      pCEV
**
**  @return     status
**
******************************************************************************
**/
UINT8 GR_IsCrossGeoLocation (CEV *pCEV)
{
    UINT16       i;
    PDD         *pPDD;
    PDD         *pPDD1;

    if (gPDX.pdd[pCEV->dMap[0]] == NULL)
    {
        return NON_GEORAID_VDISK;
    }

    pPDD = gPDX.pdd[pCEV->dMap[0]];
    if (pPDD->geoLocationId == 0)
    {
        return NON_GEORAID_VDISK;
    }

    for (i = 1; i < pCEV->numDAML; ++i)
    {
        if (gPDX.pdd[pCEV->dMap[i]] != NULL)
        {
            pPDD1 = gPDX.pdd[pCEV->dMap[i]];
            if (pPDD1->geoLocationId == 0)
            {
                return NON_GEORAID_VDISK;
            }
        }
    }

    /* This is a geoRaid Vdisk, check whether it is across multi location. */
    for (i = 1; i < pCEV->numDAML; ++i)
    {
        if (gPDX.pdd[pCEV->dMap[i]] != NULL)
        {
           pPDD1 = gPDX.pdd[pCEV->dMap[i]];
           if (pPDD->geoLocationId != pPDD1->geoLocationId)
           {
               return GEORAID_VDISK_ACROSS_MULTIPLE_LOCATIONS;
           }
        }
    }
#if 0
    fprintf(stderr, " Geo location is matched for all the devices\n");
#endif
    return ABSOULTE_GEORAID_VDISK;
}

/**
******************************************************************************
**
**  @brief      This function finds if the drive is inserted across geo location.
**              If yes, sends a log message to CCB informing that the drive is
**              inserted across geo locations and modify the location of the
**              drive that is being inserted to the location of the drive bay
**              into which the insertion happend.
**
**  @param      NIL
**
**  @return     NIL
**
******************************************************************************
**/
void GR_CheckForCrossLocationInsertion(void)
{
    PDD        *pPDD = NULL;
    UINT16      i;
    UINT16      j;
    UINT8       previousLocationCode = 0;
    UINT16      placeHolder;
    UINT8       entryFound = FALSE;
    UINT64      wwn = 0;


    /*
    ** Get the previous location code of the drive that has
    ** been inserted. This is useful to know whether the
    ** insertion happend within the same bay or across
    ** locations codes.
    */
    for (i=0; i < MAX_PHYSICAL_DISKS; i++)
    {
        if ( (pPDD = gPDX.pdd[i]) &&
             ( BIT_TEST(pPDD->geoFlags, PD_DRIVE_INSERTED) ||
               BIT_TEST(pPDD->geoFlags, PD_DRIVE_REATTACHED))
           )
        {
            /*
            ** Get the WWN of the Drive that has been inserted
            */
            wwn = pPDD->wwn;

            /*
            ** Search the local copy for any of the local copy
            ** entries matches the WWN of the inserted drive.
            ** This aids us in findingout the previous location
            ** code of the inserted device. Need to know its
            ** previous location code, because if once the drive
            ** is removed from the slot, we loose its previous
            ** location code. We are taking care of this by
            ** maintaining the local copy
            */
            for (j=0; j < MAX_PHYSICAL_DISKS; j++)
            {
                if (gGRSpdd[j].wwn == wwn)
                {
                    previousLocationCode = gGRSpdd[j].locationCode;
                    entryFound = TRUE;
                    GR_ProcessInsertion (pPDD, previousLocationCode, j);
                    break;
                }
            }
            if (entryFound == FALSE)
            {
                /*
                ** This is the case when a new drive is being inserted.
                ** We don't find any entry in the local copy in this
                ** case. We get the place holder into the local copy
                ** and add the new entry into the local copy
                */
                placeHolder = GR_GetFreePlaceHolder();

                if (placeHolder < MAX_PHYSICAL_DISKS)
                {
                    gGRSpdd[placeHolder].wwn = pPDD->wwn;
                    GR_ProcessInsertion (pPDD, previousLocationCode, placeHolder);
                }
            }
        }
    }
}


/**
******************************************************************************
**
**  @brief       This function aids to find the empty slot in the local copy
**               that is maintained.
**
**  @param       NIL
**
**  @return      UINT8: PlaceHolder
**
******************************************************************************
**/
static UINT8 GR_GetFreePlaceHolder(void)
{
    UINT16  i;

    /*
    ** Search for the empty slot in the local copy.
    */
    for (i=0; i < MAX_PHYSICAL_DISKS; i++)
    {
        if (gGRSpdd[i].wwn == 0)
        {
            break;
        }
    }
    return (i);
}

/**
******************************************************************************
**
**  @brief       This function checks if the drive is inserted within the same
**               bay or not. If it is across the bays, updates the location
**               code of the inserted drive with the location code of the Bay
**               into which the drive is inserted. Updates its local copy of
**               location code with the current location code. Sets the bit
**               appropriately for the CCB to log the message.
**
**  @param       PDD*  :      pPDD
**               UINT8 :      previousLocationCode
**               UINT16:      placeHolder
**
**  @return      NIL
**
******************************************************************************
**/
static void GR_ProcessInsertion ( PDD* pPDD,
                                  UINT8 previousLocationCode,
                                  UINT16 placeHolder )
{
    UINT8       currentses;
    PDD        *pBayPDD = NULL;

    /*
    ** Get the current ses of the drive where it is inserted
    */
    currentses = pPDD->ses;

    /*
    ** Get the Bay PDD where the drive is inserted
    */
    pBayPDD = gEDX.pdd[currentses];

    /*
    ** Check if we got the Bay PDD. If not, just return
    ** We can not proceed further if we don't have the
    ** Bay PDD. We may get the Bay PDD next poll.
    */
    if (pBayPDD == NULL)
    {
        return;
    }

    /*
    ** Check if the location code of the Bay is zero. This is the case when a
    ** Bay is powercycled or a new Bay is added. We loose the location code of
    ** the Bay when the Bay is once powercycled. But, no need to panic. We can
    ** get its location code from the local copy we are maintaining.
    */
    if (pBayPDD->geoLocationId == 0)
    {
        GR_SetBayLocationCode (pBayPDD);
    }

    /*
    ** Check if the inserted drive is within the same bay or across the bays.
    ** If it is across the bays, update the location code of the drive with the
    ** location code of the Bay into which the drive is inserted. Update the
    ** local copy of the location code with the current location code.
    ** Set the insertion bit for the CCB to log the message accordingly.
    */

    if (previousLocationCode != pBayPDD->geoLocationId)
    {
        pPDD->geoLocationId = pBayPDD->geoLocationId;
        gGRSpdd[placeHolder].locationCode = pBayPDD->geoLocationId;

        /*
        ** Check if the previous location of the currently inserted
        ** drive is not zero. This is the case when a new drive or
        ** a drive from a non-geo bay is being inserted into a geo-bay.
        ** In this case, no need to log the cross geo-location message.
        ** because the insertion is not at all acorss geo locations.
        */
        if (previousLocationCode != 0)
        {
                BIT_SET(pPDD->geoFlags,PD_CROSS_LOCATION_INSERTION);
        }

        /*
        ** Log the drive inserted message if the drive inserted bit is set
        */
        if (BIT_TEST(pPDD->geoFlags, PD_DRIVE_INSERTED))
        {
                ON_LogDriveInserted (pPDD);
        }
        else if (BIT_TEST(pPDD->geoFlags, PD_DRIVE_REATTACHED))
        {
            /*
            ** Log the drive reattached message if the drive reattached is set
            */
            ON_LogDriveReattached (pPDD);
        }

        /*
        ** Clear the cross geo location insertion bit.
        */
        BIT_CLEAR(pPDD->geoFlags,PD_CROSS_LOCATION_INSERTION);
    }
    else
    {
        /*
        ** This is the case when a drive is re-inserted into the same bay
        ** from which it was removed
        **
        ** Set the location code of the inserted drive to the location
        ** code of the Bay into which the drive is inserted
        */
        pPDD->geoLocationId = pBayPDD->geoLocationId;

        /*
        ** Log the drive inserted message if the drive inserted bit is set
        */
        if (BIT_TEST(pPDD->geoFlags, PD_DRIVE_INSERTED))
        {
            ON_LogDriveInserted (pPDD);
        }

        else if (BIT_TEST(pPDD->geoFlags, PD_DRIVE_REATTACHED))
        {
            /*
            ** Log the drive reattached message if the drive reattached
            ** bit is set
            */
            ON_LogDriveReattached (pPDD);
        }

    }

    BIT_CLEAR(pPDD->geoFlags, PD_DRIVE_INSERTED);
    BIT_CLEAR(pPDD->geoFlags, PD_DRIVE_REATTACHED);

#if 0
    /*
    ** Update the P2 NVRAM
    */
    NV_P2UpdateConfig();
#endif
}

/**
******************************************************************************
**
**  @brief      This function sets the location code of the Bay. This handles
**              the two cases when a Bay is powercycled or when a new Bay is
**              added into the DSC.
**
**  @param      pBayPDD
**
**  @return     status
**
******************************************************************************
**/
void GR_SetBayLocationCode (PDD* pBayPDD)
{
    UINT32 i;
    UINT32 emptyslot;

    /*
    ** Serach for the matching entry in the local copy. We should have entry
    ** in the local copy, if any existing Bay is powercycled. Handel this case
    ** by copying the location code from the local copy.
    */
    for (i=0; i < MAX_PHYSICAL_DISKS; i++)
    {
        if (pBayPDD->wwn == gGRSpdd[i].wwn)
        {
            pBayPDD->geoLocationId = gGRSpdd[i].locationCode;
            return;
        }
    }
    /*
    * The bay may have been reinserted walk the pdisks, if there are pdisks
    * in the bay copy the location from the pdisks since gGRSpdd does not persist
    * through power cycles for missing bays. SMW.
    */
    for (i = 0; i < MAX_PHYSICAL_DISKS; i++)
    {
        if  (gPDX.pdd[i] == NULL || gPDX.pdd[i]->ses == 0xffff)
        {
            continue;
        }
        if ((pBayPDD->pid == gPDX.pdd[i]->devName[PD_DNAME_CSES])  &&
             (gPDX.pdd[i]->geoLocationId != DEFAULT_NON_GEO_LOCATION ))
        {
            pBayPDD->geoLocationId = gPDX.pdd[i]->geoLocationId;
            emptyslot = GR_GetFreePlaceHolder();
            gGRSpdd[emptyslot].wwn = pBayPDD->wwn;
            gGRSpdd[emptyslot].locationCode = pBayPDD->geoLocationId;
            return;
        }
    }
    /*
    ** This is the case when entirely new Bay is added with its associated
    ** associated drives in place. In this case, we don't find any matching
    ** entry in the local copy. Therefore, search for an empty slot and insert
    ** the new entry. Set the location code to zero, because it is a new Bay
    ** and it is the responsibility of the user to set the location code.
    */
    pBayPDD->geoLocationId = DEFAULT_NON_GEO_LOCATION;
    emptyslot = GR_GetFreePlaceHolder();
    gGRSpdd[emptyslot].wwn = pBayPDD->wwn;
    gGRSpdd[emptyslot].locationCode = 0;
}


/**
******************************************************************************
**
**  @brief      This function verifies if the given VDisk is a GEO VDisk or
**              Non_Geo.
**
**  @param      pVDD
**
**  @return     status
**
******************************************************************************
**/
UINT8 GR_IsVDiskAtGeoLocations(VDD *pVDD)
{
    RDD       *pRDD = NULL;
    PDD       *pPDD = NULL;
    PSD       *pPSD = NULL;
    PSD       *startpsd = NULL;
    UINT8      status = GEO_VDISK;

    /*
     * Make sure if the VDD is not NULL though this should not
     * be the case. Just for sanity.
     */
    if (pVDD != NULL)
    {
        /* Get the first RDD */
        pRDD = pVDD->pRDD;

        /* Search for all the RAIDS */
        while ((pRDD != NULL) && (status != NON_GEO_VDISK))
        {
            /*
             * The PSDs form a circular list.  Grab the first one
             * (it follows) the RDD structure in memory) and run
             * around the list.
             */
            startpsd = pPSD = *((PSD* *)(pRDD + 1));

            /*
             * Check for all the PSDs. Get the corresponding
             * PDD from PSD and check if the GEO location code
             * for any of the PDDs is 0 (DEFAULT_NON_GEO_LOCATION).
             * If atleast one if 0, no need to check for other
             * PDDs GEO location codes. It means that the given
             * VDisk is a non-geo VDisk. If none of the PDDs has
             * got default geo location code (0), it means that
             * the specified VDisk is a GEO-VDisk.
             */
            do
            {
                pPDD = gPDX.pdd[pPSD->pid];

                if ((pPDD != NULL) &&
                    (pPDD->geoLocationId == DEFAULT_NON_GEO_LOCATION))
                {
                    status = NON_GEO_VDISK;
                    break;
                }
                pPSD = pPSD->npsd;        /* Next PSD */
            }
            while (startpsd != pPSD);

            pRDD = pRDD->pNRDD;
        }
    }
    return (status);
}

/**
******************************************************************************
**
**  @brief      This function checks if there is any intersection of the
**              location codes among the two specified VDisks
**
**  @param     pVDD1, pVDD2
**
**  @return    status
**             returns TRUE or FALSE
**
******************************************************************************
**/
UINT8 GR_IsGeoLocationIntersected(VDD *pVDD1, VDD *pVDD2)
{
    RDD        *pRDD = NULL;
    PSD        *pPSD = NULL;
    PSD        *startpsd = NULL;
    PDD        *pPDD = NULL;
    UINT8       status = FALSE;
    UINT8       geoLocationMap[(MAX_GEO_LOCATIONS + 7)/8];

    /*
     * Make sure if the VDD is not NULL though this should not
     * be the case. Just for sanity.
     */
    if (pVDD1 != NULL)
    {
        memset(geoLocationMap, 0,((MAX_GEO_LOCATIONS + 7)/8));
        /* Get the first RDD */

        pRDD = pVDD1->pRDD;

        /* Search for all the RAIDS */
        while (pRDD != NULL)
        {
            /*
             * The PSDs form a circular list.  Grab the first one
             * (it follows) the RDD structure in memory) and run
             * around the list.
             */
            startpsd = pPSD = *((PSD* *)(pRDD + 1));

            do
            {
                /*
                 * Check for all the PSDs. Get the corresponding
                 * PDD from PSD and prepare the location map as
                 * per the location codes of the PDDs. This is
                 * used in the next part of the code to verify
                 * whether there is an intersection of the location
                 * codes among the two given VDDs.
                 */
                pPDD = gPDX.pdd[pPSD->pid];

                if (pPDD != NULL)
                {
                    BIT_SET (geoLocationMap[(pPDD->geoLocationId)/8], (pPDD->geoLocationId)%8);
                }
                pPSD = pPSD->npsd;
            } while (startpsd != pPSD);
            pRDD = pRDD->pNRDD;
        }
    }

    /*
     * Make sure if the VDD is not NULL though this should not
     * be the case. Just for sanity.
     */
    if (pVDD1 != NULL && pVDD2 != NULL)
    {
        /* Get the first RDD */
        pRDD = pVDD2->pRDD;

        /* Search for all the RAIDS */
        while (pRDD != NULL && status != TRUE)
        {
            /*
             * The PSDs form a circular list.  Grab the first one
             * (it follows) the RDD structure in memory) and run
             * around the list.
             */
            startpsd = pPSD = *((PSD* *)(pRDD + 1));

            do
            {
                pPDD = gPDX.pdd[pPSD->pid];

                /*
                 * Check for all the PSDs. Check if there is an
                 * intersection of the location codes among the
                 * two given VDDs by searching the corresponding
                 * bit value. If there is an intersection, break
                 * the search and return.
                 */
                if ((pPDD != NULL) &&
                    (BIT_TEST (geoLocationMap[(pPDD->geoLocationId)/8], (pPDD->geoLocationId)%8)))
                {
                    status = TRUE;
                    break;
                }
                pPSD = pPSD->npsd;
            } while (startpsd != pPSD);
            pRDD = pRDD->pNRDD;
        }
    }
    return (status);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
