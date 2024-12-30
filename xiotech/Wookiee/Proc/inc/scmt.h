/* $Id: scmt.h 145079 2010-08-04 19:09:12Z mdr $ */
/**
***********************************************************************
**
**  @file   scmt.h
**
**  @brief  Segment Copy Management Table
**
**  This file defines the data structures and definitions to support
**  segment copy process for virtual devices.
**
**  Copyright (c) 1998-2010 XIOtech Corporation. All rights reserved.
**
***********************************************************************
**/

#ifndef _SCMT_H_
#define _SCMT_H_
#include "XIO_Types.h"

struct SCRP2;
struct ILT;

/**
***********************************************************************
**  Definitions
***********************************************************************
**/

/*  pstate definitions */

#define PS_NOP     0       /* No operation in progress */
#define PS_DONE    1       /* Read/write sequence done */
#define PS_READ    2       /* Reading from original VD */
#define PS_ABREAD  3       /* Abort read from orig. VD */
#define PS_WRITE   4       /* Writing to copy VD */
#define PS_ABWRITE 5       /* Abort writing to copy VD */
#define PS_RDFAIL  6       /* Read failure */


/*  SCMT extended record (SCMTE) structure */

typedef struct SCMTE        /* Segment Copy Management Table */
{
    struct SCMTE *link;     /* Link list field */
    struct ILT   *ilt;      /* Associated SCIO (or ILT) */
    UINT64 vdsda;           /* Copy operation VD SDA */
    UINT64 vdeda;           /* Copy operation VD EDA */
    UINT32 numsec;          /* Sectors in copy segment */
    UINT8  pstate;          /* Copy process state */
    UINT8  retry;           /* Copy sequence Retry counter */
    UINT8  rsvd1[2];        /* Reserved 2 bytes */
 } SCMTE;


/*  SCMT main structure */

typedef struct SCMT         /* Segment Copy Management Table */
{
    struct SCMT  *link;     /* List of SCMT's on queue */
    UINT32        segnum;   /* Associated segment number */
    struct SCMTE *headscme; /* Head scme pointer */
    struct SCRP2 *scr2;     /* Assoc. SCR at lvl 2 address */
} SCMT;

#endif /* _SCMT_H_ */


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
