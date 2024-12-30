/* $Id: magdt.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       magdt.h
**
**  @brief      MAGNITUDE Descriptor Table
**
**  The MAGNITUDE Descriptor Table is used to pass information about an
**  identified MAGNITUDE node between the Link-level driver and the
**  Data-link manager.  When a MLE or FTI VRP is sent from the LLD to
**  the DLM, this table is passed to the DLM via a SGL and buffer that
**  are associated with the VRP.  The MDT table is stored immediately after
**  the SGL memory where a data buffer would normally reside.  (The SGL
**  Address field points to the MDT.)
**
**  Copyright (c) 2003-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _MAGDT_H_
#define _MAGDT_H_

#include "XIO_Types.h"

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/*
** --- MAGDT structure
*/
typedef struct MAGDT
{
    UINT32  dlmSessionID;           /* SDM Session ID (magdt_dlmid)      */
    UINT8   nodeWWN[8];             /* MAG/Tbolt Node WWN (magdt_nwwn)   */
    UINT8   portWWN[8];             /* MAG/Tbolt Port WWN (magdt_nwwn)   */
    UINT8   alpa[4];                /* Assigned AL-PA                    */
    UINT32  sn;                     /* MAG serial number                 */
    UINT8   name[8];                /* Assigned MAG name                 */
    UINT8   ip[4];                  /* Assigned IP address               */
    UINT8   path;                   /* path number                       */
    UINT8   cl;                     /* Assigned cluster number           */
    UINT8   vds;                    /* Number of VDISKs                  */
    UINT8   flag1;                  /* Special Inquire flag byte #1      */
    UINT32  alias;                  /* Alias node serial number          */
} MAGDT;

#endif /* _MAGDT_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
