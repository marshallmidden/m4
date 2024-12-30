/* $Id: nvr.h 161041 2013-05-08 15:16:49Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       nvr.h
**
**  @brief      Configuration NVRAM descriptions
**
**  To provide a common means of defining the format of the NVRAM to be
**  used for configuration saving.
**
**  Copyright (c) 1996-2010 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _NVR_H_
#define _NVR_H_

#include "DEF_Workset.h"
#include "DEF_iSCSI.h"
#include "globalOptions.h"
#include "XIO_Types.h"
#include "isns.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
/*
**  Record type field definitions
*/
#define NRT_EOF         0x03        /*  3 - End of file record                */
#define NRT_SERVER      0x08        /*  8 - server devices                    */
#define NRT_RAID        0x09        /*  9 - RAID devices                      */
#define NRT_VIRT        0x0A        /* 10 - virtual devices                   */
#define NRT_PHYS        0x0B        /* 11 - physical devices                  */
#define NRT_SES         0x0C        /* 12 - SES devices                       */
#define NRT_MISC        0x0D        /* 13 - misc devices                      */
#define NRT_TARGET      0x0E        /* 14 - targets                           */
#define NRT_XLDD        0x0F        /* 15 - XIOtech LDD                       */
#define NRT_FLDD        0x10        /* 16 - foreign LDD                       */
#define NRT_COPY        0x11        /* 17 - copy operation                    */
#define NRT_MIRROR      0x12        /* 18 - mirror partner                    */
#define NRT_WORKSET     0x13        /* 19 - workset definitions               */
/** UNUSED              0x14        can be reused                             */
#define NRT_DMCR        0x16        /* 22 - default copy configuration record */
#define NRT_COPYCFG     0x17        /* 23 - copy configuration record         */
#define NRT_ISNS        0x18        /* 24 - iSNS configuration record         */
#define NRT_PRES        0x19        /* 25 - Persistent LUN Reserve record     */
#define NRT_RAID_GT2TB  0x29        /* 41 - RAID devices - GT2TB              */

/*
** Record types for local images.
**
** Note that these do not have to be exclusive of the record types for the
** regular NVRAM image, but I am making them so for clarity.
*/
#define NRL_RAID        0x40        /* RAID records in local image Version 1 */
#define NRL_PHYS        0x41        /* Physical devices in local image      */
#define NRL_ENCL        0x42        /* Device enclosure in local image      */
#define NRL_RAID2       0x43        /* RAID records in local image Version 2 */

/*
**  Other miscellaneous constants.
*/
#define NR_MAGIC        0x37e1      /* Magic number and checksum seed       */
#define NR_REVISION     0x0c        /* NVRAM format revision, 0x0b=magnitude */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
/*
**  The part II NVRAM is constructed of a header followed by a series of
**  configuration records for each controller in the virtual controller
**  group.  The individual controller areas have their own header followed
**  by records for each device of interest in the controller such as
**  virtual devices, RAID devices, physical devices, etc.  The header
**  checksum covers the entire NVRAM.  The checksum in each controller
**  area is for the individual controller header.
*/

/* The NVRAM part II description        */
typedef struct NVRII
{
    UINT32      cSum2;              /* Checksum Part II                     */
    UINT8       rsvd4[4];           /* Reserved                             */
    UINT16      vers;               /* Version                              */
    UINT16      rev;                /* Revision                             */
    UINT32      length;             /* Length of entire structure           */
    UINT16      magic;              /* Magic number                         */
    UINT8       revision;           /* NVRAM PII revision                   */
    UINT8       defLabel;           /* Default label                        */
    UINT32      seq;                /* Sequence number                      */
    UINT8       gPri;               /* Global priority                      */
    UINT8       rsvd25;             /* Reserved                             */
    UINT8       ften;               /* Foreign target enable map            */
    UINT8       rsvd27;             /* Reserved                             */
    UINT32      vcgID;              /* Virtual contrller group ID           */
    UINT8       whql;               /* whql compliance enable (T/F)         */
    UINT8       rsvd33[11];         /* Reserved                             */
    UINT8       scrubOpt;           /* Scrubbing enable (T/F)               */
    UINT8       glCache;            /* Global cache enable (T/F)            */
    UINT8       glVdPriority;       /* Global VDisk Priority enable (T/F)   */
    UINT8       glPdAutoFailback;   /* Global VDisk Priority enable (T/F)   */
    UINT8       name[16];           /* VCG name                             */
} NVRII;

/* Physical disk description            */
typedef struct NVRP
{
    UINT16      pid;                /* Physical drive ID                    */
    UINT8       class;              /* Device class                         */
    UINT8       channel;            /* Channel device is installed in       */
    UINT32      fcID;               /* Fibre channel ID                     */
    UINT32      sSerial;            /* System serial number                 */
    UINT8       prodID[16];         /* Product ID                           */
    UINT8       vendID[8];          /* Vendor ID                            */
    UINT8       serial[12];         /* Serial number                        */
    UINT64      wwn;                /* World wide name                      */
    UINT32      lun;                /* Logical unit number                  */
    UINT8       miscStat;           /* Miscellaneous status from PDD        */
    UINT8       dName[4];           /* Positioning information              */
    UINT8       hsDName[4];         /* Position info if used hot spare      */
    UINT8       geoLocation;        /* GEO Location ID                      */
    UINT8       flags;
    UINT8       rsvd53[5];          /* Reserved                             */
} NVRP;

/* RAID device description              */
typedef struct NVRR
{
    UINT16      rid;                /* RAID device ID                       */
    UINT8       type;               /* RAID device type                     */
    UINT8       depth;              /* RAID device depth                    */
    UINT16      vid;                /* VDisk ID to which RAID belongs       */
    UINT16      devCount;           /* Number of physical devices in RAID   */
    UINT32      sps;                /* Sectors per stripe                   */
    UINT64      devCap;             /* Device capacity                      */
    UINT32      spu;                /* Sectors per unit                     */
    UINT32      sLen;               /* Segment length               - GT2TB */
    UINT32      rLen;               /* Rebuild len in progress     - unused */
    UINT8       aStatus;            /* Additional status                    */
    UINT8       rsvd[3];            /* Reserved                             */
    UINT32      notMirrorCSN;       /* Not Mirroring Controller Serial Number*/
    UINT32      owner;              /* Owning controller at time of save    */
} NVRR;

/* RAID device extension                */
typedef struct NVRRX
{
    UINT16      pid;                /* Physical device ID                   */
    UINT8       status;             /* PSD status                           */
    UINT8       aStatus;            /* Additional status                    */
    UINT32      sda;                /* Starting disk address of PSD - GT2TB */
} NVRRX;

/* RAID GT2TB physical device description */
typedef struct NVRR_GT2TB
{
    UINT16      rid;                /* RAID device ID                       */
    UINT8       type;               /* RAID device type                     */
    UINT8       depth;              /* RAID device depth                    */
    UINT16      vid;                /* VDisk ID to which RAID belongs       */
    UINT16      devCount;           /* Number of physical devices in RAID   */
    UINT32      sps;                /* Sectors per stripe                   */
    UINT64      devCap;             /* Device capacity                      */
    UINT32      spu;                /* Sectors per unit                     */
    UINT64      sLen;               /* Segment length                       */
    UINT8       aStatus;            /* Additional status                    */
    UINT8       rsvd[3];            /* Reserved                             */
    UINT32      notMirrorCSN;       /* Not Mirroring Controller Serial Number*/
    UINT32      owner;              /* Owning controller at time of save    */
} NVRR_GT2TB;

/* RAID GT2TB device extension */
typedef struct NVRRX_GT2TB
{
    UINT16      pid;                /* Physical device ID                   */
    UINT8       status;             /* PSD status                           */
    UINT8       aStatus;            /* Additional status                    */
    UINT64      sda;                /* Starting disk address of PSD         */
} NVRRX_GT2TB;

/* VDisk device description             */
/* GEORAID */
typedef struct GR_GeoRaidNvrInfo
{
    UINT8 vdOpState:3;
    UINT8 permFlags:4;
    UINT8 rsvd     :1;
} GR_GeoRaidNvrInfo;

typedef struct NVRV
{
    UINT16      vid;                /* VDisk device ID                      */
    UINT8       dRaidCnt;           /* VDisk deferred RAID count            */
    UINT8       raidCnt;            /* VDisk RAID count                     */
    UINT64      devCap;             /* Device capacity                      */
    UINT16      attr;               /* VDisk attribute                      */
    UINT8       vlarCnt;            /* VDisk VLAR count                     */
    GR_GeoRaidNvrInfo grInfo;
    UINT8       rsvd16[3];
    UINT32      breakTime;          /* Mirror Break Time                    */
    UINT32      createTime;         /* VDisk create time                    */
    UINT8       priority;           /* Priority of this particular Vdisk    */
    UINT8       name[16];           /* Vdisk name                           */
} NVRV;

/* VDisk extension 1 (RAIDs in VDisk)   */
/* VDisk extension 2 (deferred RAIDs)   */

typedef struct NVRVX1
{
    UINT16      rid;                /* RAID device ID                       */
} NVRVX1, NVRVX2;

/* VDisk extension 3 (VLARs)            */
typedef struct NVRVX3
{
    UINT32      srcSN;              /* Source controller serial number      */
    UINT8       srcCluster;         /* Source controller cluster number     */
    UINT8       srcVDisk;           /* Source controller vdisk number       */
    UINT8       attr;               /* Attributes                           */
    UINT8       poll;               /* VLink poll timer count               */
    UINT16      repVID;             /* Reported VDisk number                */
    UINT8       name[52];           /* Name                                 */
    UINT32      agnt;               /* Agent serial number                  */
} NVRVX3;

/* Server device records                */
typedef struct NVRS
{
    UINT16      sid;                /* Server ID                            */
    UINT16      nLuns;              /* Number of LUNs mapped in server      */
    UINT16      tid;                /* Target server is mapped to           */
    UINT8       stat;               /* Status                               */
    UINT8       pri;                /* Server priority                      */
    UINT32      owner;              /* Owning controller                    */
    UINT64      wwn;                /* World wide name                      */
    UINT32      attrib;             /* Server attributes                    */
    UINT16      linkedSID;          /* Linked Server ID                     */
    UINT8       rsvd30[2];          /* Reserved                             */
    UINT8       name[16];           /* Name                                 */
} NVRS;

/* Server device extensions             */
typedef struct NVRSX
{
    UINT16      vid;                /* Virtual device ID                    */
    UINT16      lun;                /* LUN the VID is mapped onto           */
} NVRSX;

/* Server device extensions             */
typedef struct NVRSX2
{
    UINT8       i_name[256];        /* iSCSI Server Name                    */
} NVRSX2;

/* Target device records                */
typedef struct NVRT
{
    UINT16      tid;                /* Target ID                            */
    UINT8       port;               /* Port mapped onto                     */
    UINT8       opt;                /* Options                              */
    UINT8       fcid;               /* Fibre channel ID                     */
    UINT8       rsvd9;              /* Reserved                             */
    UINT8       lock;               /* Locked target indicator              */
#if ISCSI_CODE
    UINT8       ipPrefix;           /* IP prefix for a classless IP address */
#else
    UINT8       rsvd11;             /* Reserved                             */
#endif
    UINT32      owner;              /* Owning controller                    */
    union {
    UINT64      portName;           /* Port world wide name                 */
#if ISCSI_CODE
        struct {
            UINT32  ipAddr;         /* Target IP Address                    */
            UINT32  ipGw;           /* Default Gateway IP Address           */
        };
#endif
    };
    UINT64      nodeName;           /* Node world wide name                 */
    UINT32      prefOwner;          /* Preferred owner                      */
    UINT16      cluster;            /* Cluster                              */
    UINT16      rsvd2;              /* Reserved                             */
    UINT8       prefPort;           /* Preferred port  */
    UINT8       altPort;            /* Alternate port                       */
    UINT8       rsvd38[2];          /* Reserved                             */
    UINT32      i_mask;             /* Reserved                             */
} NVRT;

/* Target device extensions             */
typedef struct NVRTX
{
    I_TGD       i_tgd;              /* Target param info                    */
} NVRTX;

/* Extension for CHAP User Info */
typedef struct NVRTX1
{
    UINT8                  sname[256];
    UINT8                  secret1[32];
    UINT8                  secret2[32];
} NVRTX1;

/* Persistent LUN Reservation records   */
typedef struct NVRPR
{
    UINT16 vid;      /* VID                                  */
    UINT16 sid;      /* sid of initiator holding reservation */
    UINT8  scope;    /* scope of reservation                 */
    UINT8  type;     /* type of reservation                  */
    UINT8  regCount; /* number of registrations              */
    UINT8  rsvd;
} NVRPR;

/* Persistent LUN Reservation extension records (registration information) */
typedef struct NVRPRX
{
    UINT16 sid;       /* sid of initiator  */
    UINT8  tid;       /* tid */
    UINT8  lun;       /* LUN */
    UINT8  key[8];    /* 8 Byte Registration key */
} NVRPRX;

/* iSNS Server records                  */
#define NVRISNS         ISD

/* XIOtech LDD records                  */
typedef struct NVRX
{
    UINT16      lid;                /* LDD ID                               */
    UINT8       pathMask;           /* Path mask                            */
    UINT8       pathPri;            /* Path priority                        */
    UINT64      devCap;             /* Device capacity                      */
    UINT8       serial[12];         /* Serial number                        */
    UINT16      baseVDisk;          /* Base virtual disk number             */
    UINT8       baseCluster;        /* Base cluster number                  */
    UINT8       state;              /* LDD sate                             */
    UINT64      baseNode;           /* Base node world wide name            */
    UINT32      baseSN;             /* Base serial number                   */
    UINT16      lun;                /* LUN                                  */
    UINT8       rsvd30[2];          /* Reserved                             */
    UINT8       baseName[16];       /* Base device name                     */
    UINT32      owner;              /* Owner of the LDD                     */
    UINT8       rsvd36[12];         /* Reserved                             */
} NVRX;

/* Foreign LDD records                  */
typedef struct NVRF
{
    UINT16      lid;                /* LDD ID                               */
    UINT8       pathMask;           /* Path mask                            */
    UINT8       pathPri;            /* Path priority                        */
    UINT64      devCap;             /* Device capacity                      */
    UINT8       serial[12];         /* Serial number                        */
    UINT8       vendID[8];          /* Vendor ID string                     */
    UINT8       prodID[16];         /* Product ID string                    */
    UINT32      rev;                /* Revision                             */
    UINT16      lun;                /* LUN                                  */
    UINT8       rsvd54[6];          /* Reserved                             */
    UINT32      owner;              /* Owner of the LDD                     */
    UINT8       rsvd60[12];         /* Reserved                             */
} NVRF;

/* Mirror partner records               */
typedef struct NVRM
{
    UINT32      mySerial;           /* Serial number of controller          */
    UINT32      myPartner;          /* Serial number of partner controller  */
    UINT8       rsvd12[4];          /* Reserved                             */
} NVRM;

/*
** Default Managment Configuration Record structure
*/
typedef struct NVDMCR
{
    UINT32      rid;                /* next rigistration ID of              */
    UINT8       cr_pri;             /* COR priority                         */
    UINT8       pr_pri;             /* proc priority                        */
    UINT8       rsvd06[10];         /* reserved                             */
} NVDMCR;

/*
** Copy Configuration Record structure
*/
typedef struct NVCOPY
{
    UINT16      svid;               /* source VID                           */
    UINT8       stype;              /* source SCD type                      */
    UINT8       shidx;              /* src phs1/2 upd hdlrs idx             */
    UINT16      dvid;               /* dest VID                             */
    UINT8       dtype;              /* desnssnt SCD type                    */
    UINT8       dhidx;              /* dest phs1/2 upd hdlrs idx            */
    UINT32      tsegs;              /* total segments                       */
    UINT32      rid;                /* copy registration ID                 */
                                    /*                   -------<0x10>---   */
    UINT32      rcsn;               /* CM MAG serial number                 */
    UINT8       rcscl;              /* CM source cl num                     */
    UINT8       rcsvd;              /* CM source Vdisk num                  */
    UINT8       rcdcl;              /* CM destination cl num                */
    UINT8       rcdvd;              /* CM destination Vdisk num             */
    UINT32      rssn;               /* Copy source serial num               */
    UINT32      rdsn;               /* Copy dest serial num                 */
                                    /*                   -------<0x20>---   */
    UINT8       rscl;               /* Copy source cl num                   */
    UINT8       rsvd;               /* Copy source Vdisk num                */
    UINT8       rdcl;               /* Copy dest cl num                     */
    UINT8       rdvd;               /* Copy dest Vdisk num                  */
    UINT8       gid;                /* user defined group ID                */
    UINT8       cr_crstate;         /* cor copy registration state          */
    UINT8       cr_cstate;          /* cor  cstate                          */
    UINT8       cm_type;            /* CM  type                             */
    UINT8       cm_pri;             /* CM  priority                         */
    UINT8       cm_mtype;           /* CM copy type/mirror type             */
    UINT8       rsvd2a[2];          /* reserve                              */
    UINT32      powner;             /* primary owning controller s/n        */
                                    /*                   -------<0x30>---   */
    UINT32      sowner;             /* secondary owning controller s/n      */
    UINT8       rsvd34[12];         /* reserve                              */
                                    /*                   -------<0x40>---   */
    UINT8       label[16];          /* copy label                           */
                                    /*                   -------<0x50>---   */
                                    /* This is the RCC storage area         */
    UINT32      nssn;               /*   new source sn                      */
    UINT32      ndsn;               /*   new destination sn                 */
    UINT32      cssn;               /*   current (old) source sn            */
    UINT32      cdsn;               /*   current (old) dest sn              */
} NVCOPY;

/* Workset records                      */
typedef struct NVRW
{
    DEF_WORKSET     workset[DEF_MAX_WORKSETS];
} NVRW;

/*
**  Local image structures...
**
**    Local images are not really NVRAM records that are stored in NVRAM,
**    but they are built in a similar fashion, transfered to the master
**    controller and integrated by the master controller into the strucures
**    in the master including the NVRAM that is saved to disk, to NVRAM and
**    to the slave controller NVRAMs.
*/

/* Local image records                  */
typedef struct NVRL
{
    UINT32      length;             /* Size of the local image              */
    UINT32      ctrlID;             /* Controller ID                        */
    UINT32      mirrorPartner;      /* Mirror partner ID                    */
    UINT32      seqNumber;          /* Sequence Number (for debug)          */
} NVRL;

/* Local image for RAID status          */
typedef struct NVRLRDD              /* Version 1                            */
{
    UINT16      rid;                /* RAID ID                              */
    UINT16      psdCnt;             /* Number of PSDs                       */
    UINT8       aStatus;            /* Alternate status                     */
    UINT8       rsvd[3];            /* Pad                                  */
} NVRLRDD;

typedef struct NVRLRDD2             /* Version 2                            */
{
    UINT16      rid;                /* RAID ID                              */
    UINT16      psdCnt;             /* Number of PSDs                       */
    UINT8       aStatus;            /* Alternate status                     */
    UINT8       rsvd13[3];          /* Pad                                  */
    UINT32      notMirroringCSN;    /* Not Mirroring Controller Serial Num  */
    UINT8       rsvd20[12];         /* Pad                                  */
} NVRLRDD2;

/* Local image extension for RAID stat  */
typedef struct NVRLXPSD
{
    UINT16      pid;                /* Physical ID                          */
    UINT8       status;             /* Status                               */
    UINT8       aStatus;            /* Alternate status                     */
} NVRLXPSD;

/* Local image for drive status         */
typedef struct NVRLXPDD
{
    UINT16      pid;                /* Physical ID                          */
    UINT8       status;             /* Status                               */
    UINT8       rsvd3;              /* Reserved                             */
} NVRLXPDD;

/* Fsys report structure                */
typedef struct NVRFSYS
{
    UINT16      firstPID;           /* First PID in this report             */
    UINT16      flags;              /* Flags                                */
    UINT16      lastIndex;          /* Index to the last entry in this map  */
    UINT8       rsvd[10];           /* Total blocks in defragmentation      */
    UINT32      map[8];             /* Bit map                              */
} NVRFSYS;

/* Header record type                   */
typedef struct NVRH
{
    UINT16      recLen;             /* Record length for this record        */
    UINT8       recType;            /* Record type for this record          */
    UINT8       status;             /* Record status for this record        */
} NVRH;

/* NVR record union                     */
typedef struct NVR
{
    NVRH        hdr;                /* Header for this record               */

    union
    {
        NVRS    serv;               /* Server record                        */
        NVRT    targ;               /* Target record                        */
        NVRV    virt;               /* Virtual device record                */
        NVRR    raid;               /* RAID device record                   */
        NVRR_GT2TB raidGT2TB;       /* RAID device record - GT2TB           */
        NVRP    phys;               /* Physical record                      */
        NVRF    lddf;               /* Foreign LDD record                   */
        NVRX    lddx;               /* XIOtech LDD record                   */
        NVRM    mirror;             /* Mirror partner record                */
        NVRW    workset;            /* Workset record                       */
        NVRW    geopool;            /* Geopool record                       */
        NVRLRDD lraid;              /* Local image RAID Version 1           */
        NVRLRDD2 lraid2;            /* Local image RAID Version 2           */
        NVDMCR  dmcr;               /* Default Misc Copy record             */
        NVCOPY  copycfg;            /* Copy configuration record            */
        NVRISNS isns;               /* iSNS configuration record            */
        NVRPR   prsv;               /* Persistent LUN reservation record    */
    } u;
} NVR;

/* NVR extension record union           */
typedef struct NVX
{
    union
    {
        NVRSX   serv;               /* Server device extension record       */
        NVRSX2  serv2;              /* Server device extension record       */
        NVRVX1  virt1;              /* Virtual device extension 1 record    */
        NVRVX2  virt2;              /* Virtual device extension 2 record    */
        NVRVX3  virt3;              /* Virtual device extension 3 record    */
        NVRRX   raid;               /* RAID device extension record         */
        NVRRX_GT2TB raidGT2TB;      /* RAID device extension record - GT2TB */
        NVRLXPSD lpsd;              /* Local image extension for PSD        */
        NVRLXPDD lphys;             /* Local image physical device          */
        NVRTX   targ;               /* Target Extension record              */
        NVRTX1  chapInfo;           /* CHAP User Info                       */
        NVRPRX  prsvx;              /* Persistent LUN Registration record   */
    } u;
} NVX;

/*      */
typedef struct MPMAP
{
    UINT32  source;
    UINT32  dest;
} MPMAP;

#endif /* _NVR_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
