/* $Id: isns.h 144191 2010-07-15 20:23:53Z steve_wirtz $ */
/**
 ******************************************************************************
 **
 **  @file       isns.h
 **
 **  @brief      iSNS header file
 **
 **  Copyright (c) 2006-2010 XIOtech Corporation.  All rights reserved.
 **
 ******************************************************************************
 **/
#ifndef ISNS_H
#define ISNS_H

#include "MR_Defs.h"
#include "pcb.h"

#define ISNS_PORT               3205

#define ISNS_GF_MASTER          31

typedef struct _isns_server_descriptor {
    UINT32      gflags;                     /* Global Flags                     */
    UINT32      rsvd;                       /* Create a QUAD boundary           */
    struct {
        UINT32      ip;                     /* Server IP Address                */
        UINT16      port;                   /* Server TCP/UDP Port              */
        UINT16      sflags;                 /* Server Flags                     */
#ifdef FRONTEND
        PCB         *pt_iSNS;               /* iSNS Server task PCB             */
#endif /* FRONTEND */
    } srv[MAX_ISNS_SERVERS];
} ISD;

#ifdef FRONTEND

#define MAX_FE_TARGETS          8

#define ISNS_KEEPALIVE_TO       900*1000    /* 15 mins                      */


#define ISNS_TAG_EID            1
#define ISNS_TAG_PROTO          2
#define ISNS_TAG_PORTALIP       16
#define ISNS_TAG_PORTALPORT     17
#define ISNS_TAG_ISCSINAME      32
#define ISNS_TAG_NODETYPE       33

typedef struct isns_request {
    UINT16          version;                /* iSNS Version                     */
#define ISNS_VERSION        0x0001
    UINT16          function;               /* Request Function ID              */
#define ISNS_REQ_REG        0x0001          /* Device Attr Reg Request          */
#define ISNS_REQ_QRY        0x0002          /* Device Attr Query Request        */
#define ISNS_REQ_DEREG      0x0004          /* Device Dereg Request             */
    UINT16          length;                 /* PDU payload length               */
    UINT16          flags;                  /* iSNS Flags                       */
#define ISNS_F_CLIENT   15
#define ISNS_F_REPLACE  12
#define ISNS_F_LAST     11
#define ISNS_F_FIRST    10
    UINT16          transactionID;          /* iSNS Transaction ID              */
    UINT16          seqID;                  /* iSNS Sequence ID                 */
    struct {
        UINT32          src_T;              /* Source Attribute Tag             */
        UINT32          src_L;              /* Source Attribute Length          */
        UINT8           src_V[24];          /* Source Attribute Value           */
        union{
            struct {
                UINT32  qkey_T;             /* Msg Key Tag - we use EID         */
                UINT32  qkey_L;             /* Msg Key Length                   */
                UINT8   qkey_V[20];         /* Msg Key Value                    */
                UINT64  q_limit;            /* Delimitet Attribute              */
                UINT32  qip_T;              /* Op Attr Portal IP Tag            */
                UINT32  qip_L;              /* Op Attr Portal IP Length         */
#define PDU_QRY_LEN         76              /* 88 - 12 */
            };
            struct {
                UINT32  rkey_T;             /* Msg Key Tag - we use EID         */
                UINT32  rkey_L;             /* Msg Key Length                   */
                UINT8   rkey_V[20];         /* Msg Key Value                    */
                UINT64  r_limit;            /* Delimitet Attribute              */
                UINT32  reid_T;             /* Op Attr EID Tag                  */
                UINT32  reid_L;             /* Op Attr EID Length               */
                UINT8   reid_V[20];         /* Op Attr EID Value                */
                UINT32  rep_T;              /* Op Attr Entity Protocol Tag      */
                UINT32  rep_L;              /* Op Attr Entity Protocol Length   */
                UINT32  rep_V;              /* Op Attr Entity Protocol Value    */
#define ISNS_EP_ISCSI       2               /* Entity Protocol = iSCSI          */
                UINT32  rip_T;              /* Op Attr Portal IP Tag            */
                UINT32  rip_L;              /* Op Attr Portal IP Length         */
                UINT32  rip_V[4];           /* Op Attr Portal IP Value          */
                UINT32  rpp_T;              /* Op Attr Portal Port Tag          */
                UINT32  rpp_L;              /* Op Attr Portal Port Length       */
                UINT16  rpp_PortType;       /* Op Attr Portal TCP/UDP Type      */
                UINT16  rpp_Port;           /* Op Attr Portal Port Value        */
                UINT32  rtn_T;              /* Op Attr iSCSI Name Tag           */
                UINT32  rtn_L;              /* Op Attr iSCSI Name Length        */
                UINT8   rtn_V[24];          /* Op Attr iSCSI Name Value         */
                UINT32  rnt_T;              /* Op Attr iSCSI Node Type Tag      */
                UINT32  rnt_L;              /* Op Attr iSCSI Node Type Length   */
                UINT32  rnt_V;              /* Op Attr iSCSI Node Type Value    */
#define ISNS_N_TARGET       0
#define ISNS_N_INITIATOR    1
#define ISNS_N_CONTROLE     2
#define PDU_REG_LEN         188             /* 200 -12 */
            };
            struct {
                UINT64  d_limit;            /* Delimitet Attribute              */
                UINT32  deid_T;             /* Op Attr EID tag                  */
                UINT32  deid_L;             /* Op Attr EID length               */
                UINT8   deid_V[20];         /* Op Attr EID value                */
#define PDU_DEREG_LEN       68              /* 80 - 12  */
            };
        };
    };
} ISNS_REQ;

typedef struct isns_response {
    UINT16          version;
    UINT16          function;
#define ISNS_RSP_REG        0x8001          /* Device Attr Reg Response         */
#define ISNS_RSP_QRY        0x8002          /* Device Attr Query Response       */
#define ISNS_RSP_DEREG      0x8004          /* Device Dereg Response            */
    UINT16          length;
    UINT16          flags;
    UINT16          transactionID;
    UINT16          seqID;
    UINT32          error;
    UINT8           data[1024];
}ISNS_RSP;


extern void iSNS_Init(void);

#endif /* FRONTEND */

#endif /* ISNS_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
