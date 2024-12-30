/* $Id: GR_LocationManager.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       GR_LocationManager.h
**
**  @brief      To provide a common means of functon prototypes for Location
**              Manager
**
**  Copyright (c) 2005-2010 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/

#ifndef _GR_LocationManager_H
#define _GR_LocationManager_H

#include "cev.h"

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/
#define GR_SET_GEO_LOCATION           0
#define GR_CLEAR_ALL_GEO_LOCATIONS    1

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

struct MR_PKT;
struct VDD;
struct PDD;

/**
** The following structure is used to maintain the WWN and the location code of
** the Bay(s) and its associated drives. This structure aids us in maintainig
** the above
**/
typedef struct GRS_PDD
{
    UINT64     wwn;
    UINT8      locationCode;
} GRS_PDD;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void   GR_SetDefaultLocation (void);
extern UINT32 GR_SetGeoLocation(struct MR_PKT *pMRP);
extern UINT8  GR_IsCrossGeoLocation (CEV* pCEV);
extern void   GR_CheckForCrossLocationInsertion (void);
extern UINT8  GR_CheckForHotSparePresence (UINT8);
extern UINT8  GR_IsVDiskAtGeoLocations(struct VDD *);
extern UINT8  GR_IsGeoLocationIntersected(struct VDD *, struct VDD *);
extern UINT32 GR_ClearGeoLocation(struct MR_PKT *pMRP);
extern void GR_SetBayLocationCode (struct PDD* pBayPDD);
#endif /*_GR_LocationManager_H*/

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
