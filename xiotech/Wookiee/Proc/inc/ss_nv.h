/* $Id: ss_nv.h 159966 2012-10-01 23:20:49Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       ss_nv.h
**
**  @brief      To provide a means of handling Snapshot NV information.
**
**  Copyright (c) 2007 - 2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _SS_NV_H_
#define _SS_NV_H_

#include "system.h"
#include "XIO_Types.h"
#include "copymap.h"
#include "ssms.h"

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/
/* The header for version 2 and 3 is identical and in the same place. */
#define HEADER_OFFSET   0           /* Offset of header (SS_HEADER_NV) in first OGER */
// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#define SS_NV_VERSION   2           /* Version originally released to customers. */
#define FIRST_SSMS      2048        /* Sector offset to first SSMS (0=SS_HEADER_NV) */
#define SSMS_NV_ALLOC   256         /* Number of sectors for each SSMS in NV */
#define FIRST_OGER      (FIRST_SSMS + (MAX_SNAPSHOT_COUNT * SSMS_NV_ALLOC)) /* 2048+(256*256)=67584 */
                                    /* The offset to first OGER_NV structure */
#define OGER_NV_ALLOC   128         /* Sector size for each OGER_NV */
// #define END_OFFSET   (FIRST_OGER + (MAX_OGER_COUNT * OGER_NV_ALLOC)) /* 67584+(2048*128)=329728 */
// /* NOTE: The END_OFFSET cannot exceed 1GB in sectors (2097152), the size of an OGER (One Gig-ER). */
// NOTE: MAX_OGER_COUNT could be as large as 15856.

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#define SS_NV_VERSION_GT2TB   3     /* Version for GT2TB - 0920 first availability. */
#define FIRST_SSMS_GT2TB      1     /* Sector offset to first SSMS (0=SS_HEADER_NV) */
#define SSMS_NV_ALLOC_GT2TB   1     /* Number of sectors for each SSMS in NV */
#define FIRST_OGER_GT2TB      (FIRST_SSMS_GT2TB + (MAX_SNAPSHOT_COUNT * SSMS_NV_ALLOC_GT2TB)) /* 1+(256*1)=257 */
                                    /* The offset to first OGER_NV structure */
#define OGER_NV_ALLOC_GT2TB   10    /* Sector size for each OGER_NV */
// #define END_OFFSET_GT2TB   (FIRST_OGER_GT2TB + (MAX_OGER_COUNT * OGER_NV_ALLOC_GT2TB)) /* 257+(2048*10)=20737 */
// /* NOTE: The END_OFFSET cannot exceed 1GB in sectors (2097152), the size of an OGER (One Gig-ER. */
// NOTE: MAX_OGER_COUNT could be much larger.

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#define SS_MAGIC_NUM    0x12365478
#define SS_HEADER_MAGIC 0xBF        /* Snappool header magic number */
#define SS_RESTORE_PRI  205

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

typedef struct SSMS_NV
{
    UINT32          ordinal;                    /* Ordinal of this SSMS     */
    UINT16          srcvid;                     /* VID of the source        */
    UINT16          ssvid;                      /* VID of the snapshot      */
    UINT32          notused1;                   /* Not used                 */
    UINT16          firstoger;                  /* Ordinal of first OGER    */
    UINT16          tailoger;                   /* Ordinal of tail OGER     */
    UINT16          prev_tailoger;              /* Ordinal of previous tail OGER */
    UINT16          ogercnt;                    /* Number of OGERs          */
    UINT8           status;                     /* Status of the snapshot   */
    UINT8           version;                    /* Version of this record   */
    UINT8           prefowner;                  /* Preferred owner (0 or 1) */
    UINT8           reserved;                   /* Alignment */
    UINT32          resw[8];                    /* Extra space */
    UINT32          crc;
} SSMS_NV;                                      /* NOTE: 60 bytes long (1 sector) */

/* An OGER is a One Gig-er. */
typedef struct OGER_NV
{
    UINT16          vid;                        /* VID of the snappool      */
    UINT16          leftch;                     /* Ordinal of left child    */
    UINT16          rightch;                    /* Ordinal of right child   */
    UINT16          parent;                     /* Ordinal of the parent    */
    UINT16          ordinal;                    /* Ordinal of this OGER     */
    UINT16          maxprobe;                   /* Max probes done on insert*/
    UINT16          segcount;                   /* Number of segs stored    */
    UINT8           version;                    /* Version of this record   */
    UINT8           status;                     /* Status of this OGER      */
    UINT32          sda;                        /* SDA of Oger in snappool  */  /* GT2TB */
    UINT32          sdakey;                     /* SDA key value            */  /* GT2TB */
    UINT32          reserved0;                  /* UNUSED */
    UINT32          reserved1;                  /* UNUSED */
    UINT8           segfield[SEGSPEROGER/8];    /* Segment bitfield         */
    UINT32          sdamap[SEGSPEROGER];        /* SDA map                  */
    UINT16          next;                       /* Ordinal of next OGER     */
    UINT32          reserved[4];
    UINT32          crc;                        /* crc in same place for OGER_NV and OGER_NV_GT2TB */
} OGER_NV;                                      /* NOTE: 4278 bytes long (9 sectors) */

typedef struct OGER_NV_GT2TB
{
    UINT16          vid;                        /* VID of the snappool      */
    UINT16          leftch;                     /* Ordinal of left child    */
    UINT16          rightch;                    /* Ordinal of right child   */
    UINT16          parent;                     /* Ordinal of the parent    */
    UINT16          ordinal;                    /* Ordinal of this OGER     */
    UINT16          maxprobe;                   /* Max probes done on insert*/
    UINT16          segcount;                   /* Number of segs stored    */
    UINT8           version;                    /* Version of this record   */
    UINT8           status;                     /* Status of this OGER      */
    UINT64          sda;                        /* SDA of Oger in snappool  */
    UINT32          sdakey;                     /* SDA key value            */
    UINT8           segfield[SEGSPEROGER/8];    /* Segment bitfield         */
    UINT32          sdamap[SEGSPEROGER];        /* SDA map                  */
    UINT16          next;                       /* Ordinal of next OGER     */
    UINT32          reserved[5];
    UINT32          crc;                        /* crc in same place for OGER_NV and OGER_NV_GT2TB */
} OGER_NV_GT2TB;                                /* NOTE: 4278 bytes long (9 sectors) */

typedef struct SS_HEADER_NV
{
    UINT8           version;                    /* Version of this record   */
    UINT8           header_magic;               /* Magic number to indicate
                                                   initial condition of header NV */
    UINT16          spool_vid;                  /* Ordinal of this header   */
    UINT16          length;                     /* Length of this header    */
    UINT16          ssms_count;                 /* Number of ssms_nvs stored*/
    UINT32          ssms_offset;                /* First ssms_nv offset     */
    UINT32          oger_offset;                /* First oger_nv offset     */
    UINT8           ord_map[MAX_SNAPSHOT_COUNT];/* Bitmap of stored ssms_nvs*/
    UINT32          reserved[8];
    UINT32          crc;
} SS_HEADER_NV;                                 /* NOTE: 308 bytes long (1 sector) */

/*
******************************************************************************
** Public functions
******************************************************************************
*/
INT32           ss_check_for_duplicate_oger(INT32 ordinal, UINT16 spool_vid);

#endif /* _SS_NV_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
