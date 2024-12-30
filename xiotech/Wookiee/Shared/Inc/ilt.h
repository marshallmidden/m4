/* $Id: ilt.h 161068 2013-05-14 19:05:20Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       ilt.h
**
**  @brief      InterLayer Transport
**
**      To provide a common means of defining the InterLayer Transport
**      (ILT) definitions.
**
**  Copyright (c) 1996-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _ILT_H_
#define _ILT_H_

#include "vrp.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/* Pointers to structures that are typically not needed. */
struct DSEG_DESC;
struct DEV;
struct FCP_CMD_IU;
struct IMT;

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#ifdef PAM
#define ILTNEST1  0
#else  /* PAM */
#ifdef FRONTEND
#define ILTNEST1 10
#endif /* FRONTEND */

#ifdef BACKEND
#define ILTNEST1 6
#endif /* BACKEND */

#ifdef CCB_RUNTIME_CODE
#define ILTNEST1 3
#endif /* CCB_RUNTIME_CODE */
#endif /* PAM */

#define ILTNEST     (ILTNEST1+1)

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/
#define ILT_SIZE    (sizeof(struct ILT) *ILTNEST)   /* Size of an ILT       */
#define ILT_BIAS    (sizeof(struct ILT))            /* Bias to next nest    */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/* (nested ILTNEST levels deep) */
typedef struct ILT
{
    struct ILT *fthd;           /* Forward thread                       <w> */
    struct ILT *bthd;           /* Backward thread                      <w> */
    UINT32      misc;           /* Misc param 1                         <w> */
    void       *cr;             /* Completion routine                   <w> */
    /*
     * NOTE: this union must be 16 bytes offset from ILT. There are .def files
     * that depend upon il_w0 being 16, so that the "c" structure puts the
     * defined structure into the location of the following union.
     */
    union
    {
        /* Non-Descript Layers */
        struct ilt_normal
        {
            UINT32      w0;     /* Parameter 0                          <w> */
            UINT32      w1;     /* Parameter 1                          <w> */
            UINT32      w2;     /* Parameter 2                          <w> */
            UINT32      w3;     /* Parameter 3                          <w> */
            UINT32      w4;     /* Parameter 4                          <w> */
            UINT32      w5;     /* Parameter 5                          <w> */
            UINT32      w6;     /* Parameter 6                          <w> */
            UINT32      w7;     /* Parameter 7                          <w> */
        } ilt_normal;

        /* idriver scan layer */
        struct ilt_scan
        {
            UINT32    scan_status;   /* status                        [w0] */
            UINT32    scan_w1;
            UINT32    scan_nerr;     /* Error Count                   [w2] */
            UINT8     scan_state;    /* state                         [w3] */
            UINT16    scan_tagID;    /* tag type code                 [w3] */
            UINT8     scan_retry;    /* retry count                   [w3] */
            UINT32    scan_reqID;    /* requestor ID                  [w4] */
            UINT16    scan_tmr1;     /* Timer 1                       [w5] */
            UINT16    scan_tmr2;     /* Timer 1                       [w5] */
            UINT32    scan_tmt;      /* pointer to TMT                [w6] */
            UINT32    scan_xli;      /* pointer to XLI                [w7] */
        } ilt_scan;

        /* 8MB specific Primary ILT struct */
        #define MAX_SPLIT   8
        struct ilt_prim
        {
            struct ILT  *split[MAX_SPLIT];  /* [w0] to [w7] - SPLIT ILTs    */
        } ilt_prim;

        /* Cache to Virtual Layer Definitions */
        struct cache_2_virtual
        {
            UINT32      cvw0;   /* Parameter 0                          <w> */
            UINT32      cvw1;   /* Parameter 1                          <w> */
            struct TG  *pcvTag; /* Cache tag pointer               [w2] <w> */
            struct ILT *pcvPhILT; /* Placeholder pointer           [w3] <w> */
            struct VRP *pcvVRP; /* VRP pointer                     [w4] <w> */
            UINT32      cvw5;   /* Parameter 5                          <w> */
            UINT32      cvw6;   /* Parameter 6                          <w> */
            UINT32      cvw7;   /* Parameter 7                          <w> */
        } cache_2_virtual;

        /* Placeholder ILT definitions */
        struct wc_mirror
        {
            union
            {
                UINT32      plCmd;              /* Mirror Command  [w0] <w> */
                struct wc_mirror_tags
                {
                    UINT32      plMirrorTag:1;  /* Mirror Tags          <b> */
                    UINT32      plMirrorBuffer:1; /* Mirror Buffers     <b> */
                    /* NOTE: Both bits means mirror Buffer first */
                    UINT32      :30;            /* Reserved           30<b> */
                } wc_mirror_tags;
            }
#if defined(S_SPLINT_S) || defined(FRTVIEW)
             uaha1
#endif  /* S_SPLINT_S */
             ;
            UINT32      plw1;   /* Parameter 1                     [w1] <w> */
            UINT32      plw2;   /* Parameter 2                     [w2] <w> */
            UINT32      plw3;   /* Parameter 3                     [w3] <w> */
            struct ILT *pplILT; /* ILT pointer                     [w4] <w> */
            struct TG  *pplTag; /* Cache Tag Pointer               [w5] <w> */
            union
            {
                UINT32      plIntent;   /* Placeholder intent      [w6] <w> */
                struct pl_type
                {
                    UINT32   plInvalidate:1;    /* Queued for Invalidation <b> */
                    UINT32   plFlush:1;         /* Queued for Flushing  <b> */
                    UINT32   plWrite:1;         /* Queued for Writing   <b> */
                    UINT32   plRead:1;          /* Queued for Reading   <b> */
                    UINT32   :28;               /* Reserved           28<b> */
                } pl_type;
            }
#if defined(S_SPLINT_S) || defined(FRTVIEW)
             uaha2
#endif  /* S_SPLINT_S */
             ;
            void       *pplSavedCR;     /* Saved Completion Routine [w7] <w> */
        } wc_mirror;

        /* Cache to DLM Definitions */
        struct cache_2_dlm
        {
            struct DLM_REQ *pcdDRP;     /* DRP Pointer             [w0] <w> */
            UINT32      cdDgReqSize;    /* Datagram Request Size   [w1] <w> */
            UINT32      cdw2;           /* Parameter 2             [w2] <w> */
            struct ILT *pcdPhILT;       /* Placeholder pointer     [w3] <w> */
            struct TG  *pcdTag;         /* Tag Chain Pointer       [w4] <w> */
            union
            {
                UINT32      cdNumTags;  /* Number tags in chain    [w5] <w> */
                UINT32      cdNumDRPs;  /* Number DRPs in chain    [w5] <w> */
            }
#if defined(S_SPLINT_S) || defined(FRTVIEW)
             uaha3
#endif  /* S_SPLINT_S */
             ;
            UINT32      cdDRPec;        /* DRPs Error Code         [w6] <w> */
            union
            {
                UINT32      cdNVA;      /* NVA Address             [w7] <w> */
                struct NV_DMA_DD *pcdDMAList; /* DMA List pointer  [w7] <w> */
            }
#if defined(S_SPLINT_S) || defined(FRTVIEW)
             uaha4
#endif  /* S_SPLINT_S */
             ;
        } cache_2_dlm;

        /* SCSI CDB cmd structure */
        struct scsi_cdm
        {
            UINT8       sccommand;      /* Command byte          [w0] <b> */
            UINT8       scchipi;        /* Chip Instance         [w0] <b> */
            UINT16      sclun;          /* LUN                   [w0] <s> */
            UINT16      scinit;         /* Initiator ID          [w1] <s> */
            UINT16      scvpid;         /* Virtual Port ID       [w1] <s> */
            UINT32      scrxid;         /* RX_ID                 [w2] <w> */
            struct IMT *imt;            /* IMT pointer           [w3] <w> */
            UINT32      cdb;            /* CDB pointer           [w4] <w> */
            UINT8       sctaskc;        /* Task Codes            [w5] <b> */
            UINT8       scexecc;        /* Execution Codes       [w5] <b> */
            UINT8       sctaskf;        /* Task Flags            [w5] <b> */
            UINT8       scrcn;          /* Request Not Complete  [w5] <b> */
            UINT32      scdatalen;      /* Data length           [w6] <w> */
            union
            {
                UINT32      pdu;        /* ISCSI ILT pointer     [w7] <w> */
                struct scsi_cdm_attr
                {
                    UINT8   attribute2400; /* attribute field     [w7] <w> */
                    UINT8   isp2400flags;
                    UINT16  scox_id;
                } scsi_cdm_attr;
            }
#if defined(S_SPLINT_S) || defined(FRTVIEW)
             uaha5
#endif  /* S_SPLINT_S */
             ;
        } scsi_cdm;

        struct scsi_2400_dsd
        {
            void       *isp2400_dsdlist;        /* w0 */
            UINT32      isp2400_sgdesc_count;   /* w1 */
            UINT32      ts  ;                   /* w2 */
            UINT32      ts2  ;                  /* w3 */
            UINT8       cdb[16];                /* w4-7 */
        } scsi_2400_dsd;

        struct inotify_2400
        {
            UINT8       incommand;      /* Command byte          [w0] <b> */
            UINT8       inchipi;        /* Chip Instance         [w0] <b> */
            UINT16      inlun;          /* LUN                   [w0] <s> */
            UINT16      ininit;         /* Initiator ID          [w1] <s> */
            UINT16      invpid;         /* Virtual Port ID       [w1] <s> */
            UINT32      inseqid;        /* exchange addr         [w2] <w> */
            UINT32      inrsv0;         /* this gets overwritten somewhere w3 */
            UINT16      inflags;        /* Flags                 [w4] <s> */
            UINT16      instatus;       /* Status                [w4] <s> */
            UINT16      inox_id;        /* rsev                  [w5] <s> */
            UINT8       intaskf;        /* Task Flags            [w5] <b> */
            UINT8       inrcn;          /* Request Not Complete  [w5] <b> */
            UINT32      inrsv1;         /*                       [w6] <w> */
            UINT8       instatsubcode;  /*                       [w7] <b> */
            UINT8       fakeinotify;    /*                       [w7] <b> */
            UINT8       inrsv2[2];      /*                       [w7] <s> */
        } inotify_2400;

        /* iSCSI Definitions */
        struct iscsi_def
        {
            struct ISCSI_PDU *pPdu;     /* w0 */
            struct CONNECTION *pConn;   /* w1 */
            struct SGL *pSgl;           /* w2 */
            UINT8      *pCdb;           /* w3 */
            UINT16      tsih;           /* w4 */
            UINT16      tid;            /* w4 */
            UINT16      lun;            /* w5 */
            UINT16      tpgt;           /* w5 */
            UINT8       cdbLen;         /* w6 */
            UINT8       flags;          /* w6 */
            UINT16      cmdType;        /* w6 */
            UINT16      portId;         /* w7 */
            UINT16      isrsvd2;        /* w7 */
        } iscsi_def;

        /* physical_io.c definitions */
#define MAX_PHY_PRIORITY    0x3fff
        struct phy_io
        {
            UINT8  phy_scsiopcode;      /* SCSI opcode          w0   <b> */
            UINT8  phy_tag;             /* Queue tag number     w0+1 <b>  */
            UINT16 phy_overlap:1;       /* Overlap indicator    w0+2 <s> bit-0 */
            UINT16 phy_writeflag:1;     /* Write indicator      w0+2 <s> bit-1 */
            UINT16 phy_priority:14;     /* Priority (aging)     w0+2 <s> bit 2-15 */

            UINT64 phy_sda;             /* SDA                  w1,w2<l> */
            UINT64 phy_eda;             /* EDA                  w3,w4<l> */
            UINT32 phy_jointhread;      /* join thread          w5   <w> */
            UINT32 phy_joincount;       /* join count           w6   <w> */
                                        /* Also SCB pointer     w6 */
            union {
                struct QU *phy_queuehead; /* Exec queue head    w7   <w> */
                struct DEV *phy_pDev;     /* or DEV pointer     w7  <w> */
            };
        } phy_io;

        /* BackEnd Raid Layer parameter structure. */
        struct be_raid
        {
            struct PRP *r_prp;          /* PRP pointer w0 */
            struct ILT *r_mir;          /* Mirrored ILT w1 */
            // UINT32 r_sn; /* Session node, no longer used. */
        } be_raid;

        struct phys_io
        {
            UINT32      phflags;        /* flags */
            struct PCB *phinitpcb;      /* pcb of PH_init_drive thread */
            void       *phio_hdr;       /* io_hdr */
            UINT32      pherrqueto;     /* timeout for error queue.  */
            struct ILT *phdonenext;     /* done list pointer */
            struct ILT *mergediolist;   /* Parameter 5  */
            struct ILT *mergedmasterilt; /* Parameter 6  */
            UINT32      phw7;           /* Parameter 7  */
        } phys_io;

        /* isp layer defs */
        struct isp_defs
        {
            UINT32      isp_iocb_type;  /* Parameter 0                  <w> */
            struct DSEG_DESC *isp_dsd_list; /* Parameter 1              <w> */
            struct DEV       *isp_dev;      /* Parameter 2              <w> */
            struct FCP_CMD_IU *isp_fcp_cmd_iu; /* Parameter 3           <w> */
            void       *cmio7cpy;       /* Parameter 4                  <w> */
            UINT32      isp_timestamp;  /* Parameter 5                  <w> */
            UINT32      isp_timeout;    /* Parameter 6                  <w> */
            UINT32      isp_w7;         /* Parameter 7                  <w> */
        } isp_defs;

#if !defined(XIO_XWS) && !defined(LOG_SIMULATOR)
/* KLUDGE KLUDGE KLUDGE KLUDE KLUDGE.
 * The XIO Web Services resuses platform code associated with the PI interface.
 * As a result a large number of platform headers, including this one are sucked in.
 * This structure isn't used in any of the PI messages. It just comes along for the ride
 * XIO Web Service uses C++ and the following anonymous structure conflicts with a
 * previous declaration resulting in a compile error.
 * To compile XIO Web Service, we will just remove the offending code.
 */
        struct pi_pdu
        {
            UINT32      w0;             /* ILT */
            UINT32      w1;             /* TPD */
            UINT32      w2;             /* SGL */
            UINT32      w3;             /* */
            UINT32      w4;             /* */
            UINT32      w5;             /* */
            UINT32      len;            /* Send Length */
            UINT8       flag;           /* w6 */
            UINT8       dataDone;       /* w6 */
            UINT8       sglCount;       /* w6 */
            UINT8       dataPresent;    /* w6 */
        } pi_pdu;
#endif  /* !defined(XIO_XWS) && !defined(LOG_SIMULATOR) */

        struct secondary_ilt
        {
            UINT32      secILT;         /* w0 */
            UINT32      ITT;            /* w1 */
            UINT32      TTT;            /* w2 */
            UINT32      r2toffset;      /* w3 */
            UINT32      dataTxLen;      /* w4 */
            UINT16      r2tCount;       /* w5 */
            UINT8       r2tDone;        /* w5 */
            UINT8       r2tRsvd1;       /* w5 */
            UINT32      totalLen;       /* w6 */
            UINT32      r2tRsvd3;       /* w7 */
        } secondary_ilt;

        /* --- Translation Layer to FC-AL structure */
#define XL_NONE         0x0
#define XL_DATA2INIT    0x10
#define XL_DATA2CTRL    0x11

#define XL_STATUSWIO    0x01
        struct fc_xl
        {
            UINT8       xl_cmd;         /* Command byte               [w0] */
            UINT8       xl_scsist;      /* SCSI status                [w0] */
            UINT8       xl_fcflgs;      /* FC-AL flags                [w0] */
            UINT8       xl_rsvd1;       /* Reserverd byte             [w0] */
            UINT32      xl_reloff;      /* Relative offset            [w1] */
            struct SGL_DESC *xl_pSGL;   /* SGL descriptor pointer     [w2] */
            UINT8      *xl_pSNS;        /* Sense data pointer         [w3] */
            UINT16      xl_sgllen;      /* SGL length in bytes        [w4] */
            UINT16      xl_snslen;      /* Sense length in bytes      [w4] */
            struct ILT *xl_pILT;        /* Primary ILT at INL1 level  [w5] */
            struct ILT *xl_pILT2;       /* Primary ILT at INL2 level  [w6] */
            INT32       xl_reslen;      /* Residual length            [w7] */
        } fc_xl;

        /* Initiator outbound level 1 */
#define  OIFLG_LID             1        /* scan only this LID bit 1 */
#define  OIFLG_LUN             2        /* scan only this LUN bit 2  */
#define  OIFLG_STID            3        /* Suppress identification of target ID to LLD bit 3 */
#define  OIFLG_ABRT            6        /* Abort the process */
#define  OIFLG_PDBC            7        /* Port DB changed */

#define  OIMODE_PARENT         0        /* discovery task is the parent */
#define  OIMODE_CHILD          2        /* discovery task is a child */

        struct out_initiator
        {
            UINT8       oil1_chpid;     /* Chip ID            [w0] <b> */
            UINT8       oil1_tmode;     /* discovery task mode[w0] <b> */
            UINT8       oil1_lun;       /* current LUN        [w0] <b> */
            UINT8       oil1_lpmapidx;  /* ALPA map index     [w0] <b> */
            UINT32      oil1_pid;       /* port ID (ALPA)     [w1] <w> */
            UINT8       oil1_flag;      /* flag byte          [w2] <b> */
            UINT8       oil1_retry;     /* retry counter      [w2] <b> */
            UINT16      oil1_lid;       /* current LID        [w2] <s> */
            UINT32      oil1_dslink;    /* discovery queue lnk[w3] <w> */
            UINT32      oil1_tmt;       /* address of TMT     [w4] <w> */
            UINT32      oil1_ILT;       /* possible pri ILT   [w5] <w> */
            UINT32      oil1_snsdata;   /* sensedata(sg)      [w6] <w> */
            UINT32      oil1_reslen;    /* residual length(sg)[w7] <w> */
        } out_initiator;
    }
#if defined(S_SPLINT_S) || defined(FRTVIEW)
     uaha6
#endif  /* S_SPLINT_S */
     ;

    UINT32      linux_val;      /* Linux value: on Target, used to hold ptr
                                   to Initiator ILT; on Initiator, used to
                                   hold the return code            <w> */
} ILT;

/* ------------------------------------------------------------------------ */
typedef struct ILT_ALL_LEVELS
{
    struct ILT  ilt[ILTNEST];
} ILT_ALL_LEVELS;

/* ------------------------------------------------------------------------ */
/* Defines for pstate in following structure. */
#define INL2_PS_WT0     0       /* Waiting to begin processing */
#define INL2_PS_REQ     1       /* Initial request to MAGNITUDE */
#define INL2_PS_SRPBLK  2       /* Srp blocked wait */
#define INL2_PS_SRPACT  3       /* Srp active on channel */
#define INL2_PS_SRPCBLK 4       /* Srp completed & blocked */
#define INL2_PS_SRPCOMP 5       /* Srp completed & returned to MAG */
#define INL2_PS_RESPBLK 6       /* Response received from MAG and task blocked */
#define INL2_PS_FINALIO 7       /* Final I/O request (either data transfer
                                   w/ending status or just ending status) */
#define INL2_PS_DATATR  8       /* Data transfer for immediate type commands
                                   (w/o ending status) */

/* Process state code (inl2_pstate) definitions */
#define INL2_PS_WT0     0       /* waiting to begin processing */
#define INL2_PS_REQ     1       /* initial request to MAGNITUDE */
#define INL2_PS_SRPBLK  2       /* srp blocked wait */
#define INL2_PS_SRPACT  3       /* srp active on channel */
#define INL2_PS_SRPCBLK 4       /* srp completed & blocked */
#define INL2_PS_SRPCOMP 5       /* srp completed & returned to MAG */
#define INL2_PS_RESPBLK 6       /* response received from MAG and task blocked */
#define INL2_PS_FINALIO 7       /* final I/O request (either data transfer
                                   w/ending status or just ending status) */
#define INL2_PS_DATATR  8       /* data transfer for immediate type commands
                                   (w/o ending status) */

/* Task type code (inl2_ttype) definitions */
#define INL2_TT_SIM     0       /* simple task */
#define INL2_TT_HOQ     1       /* head-of-queue task */
#define INL2_TT_ORD     2       /* ordered task */
                                /* 3 = (reserved) */
#define INL2_TT_ACA     4       /* ACA task */
#define INL2_TT_UNTAG   5       /* untagged task */

/* This overlays an ILT. It is used in mag.c to replace magdrvr.as code. */
typedef struct INL2
{
    struct ILT  *fthd;          /* Forward thread                       <w> */
    struct ILT  *bthd;          /* Backward thread                      <w> */
    UINT32       FCAL;          /* FC-AL param. pointer          [misc] <w> */
    void        *cr;            /* Inbound level 2 completion routine   <w> */

    UINT8        pstate;        /* Process state code              [w0] <b> */
    UINT8        cdbctl;        /* CDB control byte                [w0] <b> */
                 /* Bit 7=, 6=, 5=, 4=, 3=, 2=NACA, 1=LINK, 0=FLAG */
    UINT8        ttype;         /* Task type code                  [w0] <b> */
    UINT8        rsvd1;         /* Unused                          [w0] <b> */

    UINT8        flag1;         /* flag byte #1                    [w1] <b> */
                 /* Bit 7=, 6=, 5=, 4=, 3=, 2=, 1=on aborted queue, 0=on work queue */
    UINT8        ecode;         /* error code                      [w1] <b> */
    UINT8        lldID;         /* lld exchange ID                 [w1] <b> */
    UINT8        ecode2;        /* error code part 2               [w1] <b> */

    struct ILMT *ilmt;          /* assoc. ILMT address             [w2] <w> */
    void        *ehand;         /* Task event handler tbl          [w3] <w> */
    void        *ehand2;        /* Old Task event hdlr tbl         [w4] <w> */
    UINT32       dtreq;         /* Data transfer req size          [w5] <w> */
    UINT32       dtlen;         /* Data transfer req length        [w6] <w> */
    void        *rcvsrp;        /* Recv SRP handler routine        [w7] <w> */
    UINT32       rsvd_lv;       /* unused                   [linux_val] <w> */
} INL2;

/* ------------------------------------------------------------------------ */
/* Command codes to <isp$receive_io> (xlcommand) */
#define DTXFERN 0x00            /* No data transfer */
#define DTXFERI 0x10            /* Data transfer - back to initiator */
#define DTXFERC 0x11            /* Data transfer - initiator to controller */

/* Bit definitions for FC-AL flags (xlfcflgs) */
#define XLSNDSC 0               /* Send SCSI status w/ IO */
#define XLTRADR 1               /* Convert SGL addr to PCI for local HAB memory */

/* Value definitions for FC-AL flags (xlfcflgs) */
#define XL_SNDSC (1 << XLSNDSC) /* Send SCSI status w/ IO */
#define XL_TRADR (1 << XLTRADR) /* Convert SGL addr to PCI for local HAB memory */

/* Translation Layer to FC-AL structure */
typedef struct XLFCAL
{
    struct ILT  *rsv_f;         /*                               [fthd] <w> */
    struct ILT  *rsv_b;         /*                               [bthd] <w> */
    UINT32       rsv_m;         /*                               [misc] <w> */
    void        *otl2_cr;       /* Outbound level 2 completion routine  <w> */

    UINT8        xlcommand;     /* Command byte                    [w0] <b> */
    UINT8        xlscsist;      /* SCSI status                     [w0] <b> */
    UINT8        xlfcflgs;      /* FC-AL flags                     [w0] <b> */
    UINT8        rsvd1;         /* Unused                          [w0] <b> */

    UINT32       xlreloff;      /* Relative offset                 [w1] <w> */
    struct SGL  *xlsglptr;      /* SGL pointer                     [w2] <w> */
    void        *xlsnsptr;      /* Sense data pointer              [w3] <w> */

    UINT16       xlsgllen;      /* SGL length in bytes             [w4] <s> */
    UINT16       xlsnslen;      /* Sense length in bytes           [w4] <s> */

    struct ILT  *xlFCAL;        /* Primary ILT at INL1 levl        [w5] <w> */
    struct INL2 *xl_INL2;       /* Primary ILT at INL2 levl        [w6] <w> */
    UINT32       xlreslen;      /* Residual length                 [w7] <w> */
    UINT32       rsvd_lv;       /* unused                   [linux_val] <w> */
} XLFCAL;

/* ------------------------------------------------------------------------ */
/* OUTBOUND LEVEL 3 parameter structures */
/* Used by the FC-AL driver to manage a FC-AL operation. */
typedef struct OLT3
{
    struct ILT *rsv_f;          /*                          [fthd] <w> */
    struct ILT *rsv_b;          /*                          [bthd] <w> */
    struct ILT *otl3_OTL2;      /* Ptr outbound level 2 ilt [misc] <w> */
    void       *otl3_cr;        /* Outbound level 3 cr        [cr] <w> */
    UINT32      otl3_type;      /* IOCB type                  [w0] <w> */
    UINT32      otl3_qst;       /* Qlogic status              [w1] <w> */
    UINT32      rsvd2;          /*                            [w2] <w> */
    UINT32      rsvd3;          /*                            [w3] <w> */
    UINT32      rsvd4;          /*                            [w4] <w> */
    UINT32      rsvd5;          /*                            [w5] <w> */
    UINT32      rsvd6;          /*                            [w6] <w> */
    UINT32      rsvd7;          /*                            [w7] <w> */
    UINT32      rsvd_lv;        /* unused              [linux_val] <w> */
} OLT3;

/* ------------------------------------------------------------------------ */
/* VRP Command structure. This structure overlays w0 - w6 of the ILT structure */
typedef struct VRPCMD
{
    UINT8       vrCommand;      /* Command byte                    [w0] <b> */
    UINT8       vrChipInt;      /* Chip Interface                  [w0] <b> */
    UINT16      vrLUN;          /* LUN                             [w0] <s> */
    UINT16      vrInitiator;    /* Initiator ID                    [w1] <s> */
    UINT8       vrNotUsed[2];   /* Unused                          [w1]<2b> */
    UINT16      vrRXID;         /* RX_ID                           [w2] <s> */
    UINT8       vrTaskFlags;    /* Task Flags                      [w2] <b> */
    UINT8       vrNotUsed2;     /* Unused                          [w2] <b> */
    UINT32      vrNotUsed3;     /* Unused                          [w3] <w> */
    struct VRP *pvrVRP;         /* VRP (or SRP) Pointer            [w4] <w> */
    UINT8       vrTaskCodes;    /* Task Codes                      [w5] <b> */
    UINT8       vrExecCodes;    /* Execution Codes                 [w5] <b> */
    UINT16      vrNotUsed4;     /* Unused                          [w5] <s> */
    UINT32      vrDataLen;      /* Data Length                     [w6] <w> */
} VRPCMD;

/* ------------------------------------------------------------------------ */
#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _ILT_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
