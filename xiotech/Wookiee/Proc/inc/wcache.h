/* $Id: wcache.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       wcache.h
**
**  @brief      To provide write caching related definitions.
**
**  To provide write caching related definitions.
**
**  Copyright (c) 2000 - 2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _WCACHE_H_
#define _WCACHE_H_

#include "drp.h"
#include "qu.h"
#include "XIO_Types.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define CACHE_ISSUE_CNT         3       /* Allow up to 5 tries during errors */
#define CACHE_TIME_OUT          8       /* Give each Cache op 10 seconds to
                                            cmplt before aborting & retrying */

#define MAX_SGL_ENTRIES         8       /*  Allow up to 8 SGLs/VRP          */

#define MAX_FLUSH_INV_TIME      375/QUANTUM /* Maximum time Flushing and
                                                Invalidating tags before a wait
                                                is required                 */
#define MAX_FLUSH_OPS           512     /* Maximum number of ops to be flushed
                                            before a wait is required       */
#define FLUSH_OPS_WAIT          125     /* Number of milliseconds to wait
                                            between max ops being flushed   */
/*      MAX_CDRAM_RB_NODES                 Maximum number of RB Nodes allowed
                                            in Cacheable DRAM is the number of
                                            cache tags (C_numtags).  This will
                                            force any Flush BE tags to use
                                            NCDRAM tags.                    */
#define MAX_CDRAM_RBI_NODES MAXCDRAMILTS /* Maximum number of RBI Nodes
                                            allowed in Cacheable DRAM       */
#define MAX_CDRAM_PLACEHOLDERS  4096    /* Maximum number of Placeholder ILTs
                                            allowed in Cacheable DRAM       */

/*
**  These next 3 constants define the maximum size of a write request
**  that is allowed to use the write cache. Any write request that is less
**  than or equal to WCACHEMAX will always use the write cache, independent
**  of the current queue depth.  Write requests that are less than or equal
**  to WCACHEMAXLOWQ and greater that WCACHEMAX will only use the write cache
**  if the queue depth for the Virtual ID is currently less than WCACHELOWQ.
**  Write requests that are larger than WCACHEMAXLOWQ will always bypass the
**  the write cache.
*/

#define WCACHE_MAX      512         /* 256K (512 sectors) max cacheable     */
#define WCACHE_MAX_LOWQ 2048        /* 1M max cacheable at low Q depth      */
#define WCACHE_LOWQ     4           /* Low Queue Depth for max cacheable    */

#define MAX_VID_FLUSH_BLOCKS    64      /*  Max blocks to flush per VID     */
#define SLOW_FLUSH_WAIT_TIME    1000    /* 1 Second Wait before flushing    */
#define MEDIUM_FLUSH_WAIT_TIME  750     /* 750 msec Wait before flushing    */
#define FAST_FLUSH_WAIT_TIME    500     /* 500 msec Wait before flushing    */
#define FASTEST_FLUSH_WAIT_TIME 250     /* 250 msec Wait before flushing    */
#define NO_FLUSH_ALLOWED_WAIT_TIME 125  /* 125 msec Wait when no flushing
                                            is allowed                      */

/*
**  Tag attribute bits
*/
#define TG_DIRTY            0       /* Buffer dirty                         */
#define TG_WRITEIP          1       /* Buffer fill (write) in progress      */
#define TG_FLUSHIP          2       /* Flush    in progress                 */
#define TG_MIRRIP           3       /* Mirror in    progress                */
#define TG_READP            4       /* Read hit in progress                 */
#define TG_BE               12      /* Back End Mirrored Tag                */
#define TG_FREE_PENDING     13      /* Free pending                         */
#define TG_RESIDENT         14      /* Tag/buffer resident                  */
#define TG_FREE             15      /* Tag invalid/free                     */

#define TGM_DIRTY           0x0001  /* Buffer dirty                         */
#define TGM_WRITEIP         0x0002  /* Buffer fill (write) in progress      */
#define TGM_FLUSHIP         0x0004  /* Flush    in progress                 */
#define TGM_MIRRIP          0x0008  /* Mirror in    progress                */
#define TGM_READP           0x0010  /* Read hit in progress                 */
#define TGM_BE              0x1000  /* Back End Tag                         */
#define TGM_FREE_PENDING    0x2000  /* Free pending                         */
#define TGM_RESIDENT        0x4000  /* Tag/buffer resident                  */
#define TGM_FREE            0x8000  /* Tag invalid/free                     */

/*
**  Tag state bits
*/
#define TG_LOCKED_FLUSH       0     /* Tag locked for flush                 */
#define TG_LOCKED_READ        1     /* Tag locked for read hit              */
#define TG_LOCKED_WRITE       2     /* Tag locked for write hit             */
#define TG_LOCKED_INVALIDATE  3     /* Tag locked for invalidate            */
#define TGM_LOCKED_FLUSH      1     /* Mask for locked for flush            */
#define TGM_LOCKED_INVALIDATE 8     /* Mask for locked for invalidate       */
#define TGM_LOCKED_NOFLUSH    0xD   /* Mask for Flush, Invalidate, Write    */

/*
** Write Cache Non-Volatile Mirror Information
*/
#define WCCT_FE_NAME    0x46454354  /* "FECT" Label - FE WC Control Table   */
#define WCC_FE_NAME     0x46454346  /* "FECF" Label - FE WC Config Table    */
#define WCT_FE_NAME     0x46455447  /* "FETG" Label - FE WC Control Table   */
#define WCB_FE_NAME     0x46454246  /* "FEBF" Label - FE WC Control Table   */
#define WCCT_BE_NAME    0x42454354  /* "BECT" Label - BE WC Control Table   */
#define WCC_BE_NAME     0x42454346  /* "BECF" Label - BE WC Config Table    */
#define WCT_BE_NAME     0x42455447  /* "BETG" Label - BE WC Control Table   */
#define WCB_BE_NAME     0x42454246  /* "BEBF" Label - BE WC Control Table   */

/*
** Copy memory to Non-Volatile memory locations definitions
*/
#define MIRROR_FE       1           /**< Mirror only to the Front End NV    */
#define MIRROR_BE       2           /**< Mirror only to the Back End NV     */
#define MIRROR_MP       3           /**< Mirror to local FE NV and MP NV    */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
typedef struct TG
{
    struct TG *         fthd;           /* LRU Valid list(ForwardThread)<w> */
    struct TG *         bthd;           /* MRU Valid list (BackThread)  <w> */
    UINT16              vid;            /* Virtual ID                   <s> */
    UINT16              rdCnt;          /* Read In Progress Count       <s> */
    UINT16              attrib;         /* Attribute Bits               <s> */
    UINT16              state;          /* State Bits                   <s> */
    UINT64              vsda;           /* Starting Disk Address        <l> */
    UINT32              vLen;           /* Transfer Length (sectors)    <w> */
    UINT8 *             bufPtr;         /* Physical Buffer Address Ptr  <w> */
    struct RB *         ioPtr;          /* Valid RB Tree Pointer        <w> */
    struct RB *         dirtyPtr;       /* Dirty RB Tree Pointer        <w> */
    struct TG *         nextDirty;      /* Dirty, Flush IP link list Ptr<w> */
    struct ILT *        hQueue;         /* Head of Locked Queue(PH ILTs)<w> */
    struct ILT *        tQueue;         /* Tail of Locked Queue(PH ILTs)<w> */
    UINT32              rsvd1;
    UINT32              rsvd2;
    UINT32              rsvd3;
} TG;

/*
**  Cache Recovery F/W Initialization Ctl Block definition
*/
#define DATA_FLUSHED    0x76125794      /* signature values                 */
#define DATA_CACHED     0x49752167
#define SIG_REFRESH     0

#define MIRROR_LOCAL    0x10            /* mirror attribute values          */
#define MIRROR_REMOTE   0x20

typedef struct WT
{
    UINT32 signature1;                  /* signature 1                  <w> */
    UINT32 mirrorAttrib;                /* (local or remote)            <w> */
    UINT32 vcgID;                       /* Virtual Ctrl group ID        <w> */
    UINT32 cSerial;                     /* controller serial number     <w> */
    UINT32 seq;                         /* sequence number              <w> */
    UINT32 mirrorPartner;               /* Cache Mirror Partner         <w> */
    UINT32 rsvd1;                       /* reserved                     <w> */
    UINT32 rsvd2;                       /* reserved                     <w> */
    UINT64 batterySig;                  /* Battery signature            <l> */
    UINT32 rsvd3;                       /* reserved                     <w> */
    UINT32 signature2;                  /* signature 2                  <w> */
} WT;

/*
** Battery Backup Data Area
*/
#define BB_MAX_ECC            16        /* Maximum ECC errors               */

typedef struct BB
{
    UINT64   batterySig;
    UINT32   rsvd1[2];
    UINT8  * wrtCacheStartAdr;
    UINT8  * wrtCacheEndAdr;
    UINT32   rsvd2[2];
    UINT32   snglBitEccCurrent;
    UINT32   multBitEccCurrent;
    UINT32   snglBitEccTotal;           /* counts since last full scrub     */
    UINT32   multBitEccTotal;
    UINT32   snglELogRegOff;            /* offset into 16 word storage-sngl */
    UINT32   multELogRegOff;            /* offset into 16 word storage-mult */
    UINT32 * snglELogReg16[16];         /* up to 16 words                   */
    UINT32 * snglECarReg16[16];         /* up to 16 words                   */
    UINT32 * multELogReg16[16];         /* up to 16 words                   */
    UINT32 * multECarReg16[16];         /* up to 16 words                   */
} BB;

/*
** Wookiee Write Cache non-Volatile Mirror Information
*/
typedef struct WC_NV
{
    UINT32  wcctFELabel;                /* WC Control Table FE Label        */
    UINT32  wcctFESize;                 /* WC Control Table FE Mirror Size  */
    UINT32  wcctFEHandle;               /* WC Control Table FE Mirror Handle */
    UINT32  wccFELabel;                 /* WC Configuration Table FE Label  */
    UINT32  wccFESize;                  /* WC Configuration Table FE Size   */
    UINT32  wccFEHandle;                /* WC Configuration Table FE Handle */
    UINT32  wctFELabel;                 /* WC Tag FE Label                  */
    UINT32  wctFESize;                  /* WC Tag FE Mirror Size            */
    UINT32  wctFEHandle;                /* WC Tag FE Mirror Handle          */
    UINT32  wcbFELabel;                 /* WC Buffer FE Label               */
    UINT32  wcbFESize;                  /* WC Buffer FE Mirror Size         */
    UINT32  wcbFEHandle;                /* WC Buffer FE Mirror Handle       */

    UINT32  wcctBELabel;                /* WC Control Table BE Label        */
    UINT32  wcctBESize;                 /* WC Control Table BE Mirror Size  */
    UINT32  wcctBEHandle;               /* WC Control Table BE Mirror Handle */
    UINT32  wccBELabel;                 /* WC Configuration Table BE Label  */
    UINT32  wccBESize;                  /* WC Configuration Table BE Size   */
    UINT32  wccBEHandle;                /* WC Configuration Table BE Handle */
    UINT32  wctBELabel;                 /* WC Tag BE Label                  */
    UINT32  wctBESize;                  /* WC Tag BE Mirror Size            */
    UINT32  wctBEHandle;                /* WC Tag BE Mirror Handle          */
    UINT32  wcbBELabel;                 /* WC Buffer BE Label               */
    UINT32  wcbBESize;                  /* WC Buffer BE Mirror Size         */
    UINT32  wcbBEHandle;                /* WC Buffer BE Mirror Handle       */
} WC_NV;

typedef struct HBA_PERF_STATS
{
    UINT16 activeHBAs;                  /* active HBAs  with Host ops       */
    UINT32 perQDepth;                   /* periodic Qdepth                  */
    UINT32 perRdCmds;                   /* periodic read cmd count          */
    UINT32 perWrCmds;                   /* periodic write cmd count         */
    UINT32 perRdBlocks;                 /* periodic read blocks             */
    UINT32 perWrBlocks;                 /* periodic write blocks            */
} HBA_PERF_STATS;

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern  WC_NV gWC_NV_Mirror;            /* Write Cache NV Mirror Information  */

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern UINT8       wc_PauseCacheInit(UINT32 event, UINT32 data);
extern UINT8       WC_resumeCacheInit(UINT8 response);
extern void        WC_ComputeWCBypassState(HBA_PERF_STATS* pStats);
extern UINT32      WC_RemoteData(void *dst, void *src, UINT32 length, UINT8 region,
                                                                UINT8 function);

#endif /* _WCACHE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
