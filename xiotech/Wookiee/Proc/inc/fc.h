/* $Id: fc.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       fc.h
**
**  @brief      Fibre Channel definitions
**
**  Provides generic fibre channel definitions.
**
**  Copyright (c) 2007-2010 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/

#ifndef FC_H
#define FC_H

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/*
** Extended Link Service Requests
*/
#define FC_ELS_LS_RJT   0x01        /* Link Service Reject */
#define FC_ELS_LS_ACC   0x02        /* Link Service Accept */
#define FC_ELS_PLOGI    0x03        /* N_Port Login */
#define FC_ELS_FLOGI    0x04        /* F_Port Login */
#define FC_ELS_LOGO     0x05        /* Logout */
#define FC_ELS_ABTX     0x06        /* Abort Exchange - obsolete */
#define FC_ELS_RCS      0x07        /* Read Connection Status */
#define FC_ELS_RES      0x08        /* Read Exchange Status Back - obsolete */
#define FC_ELS_RSS      0x09        /* Read Sequence Status Back - obsolete */
#define FC_ELS_RSI      0x0A        /* Request Sequence Initiative */
#define FC_ELS_ESTS     0x0B        /* Establish Streaming */
#define FC_ELS_ESTC     0x0C        /* Estimate Credit */
#define FC_ELS_ADVC     0x0D        /* Advise Credit */
#define FC_ELS_RTV      0x0E        /* Read Timeout Value */
#define FC_ELS_RLS      0x0F        /* Read Link Error Status Block */
#define FC_ELS_ECHO     0x10        /* Echo */
#define FC_ELS_TEST     0x11        /* Test */
#define FC_ELS_RRQ      0x12        /* Reinstate Recovery Qualifier */
#define FC_ELS_REC      0x13        /* Read Exchange Concise */
#define FC_ELS_RESERVED 0x14        /* Reserved for legacy implementations */
#define FC_ELS_PRLI     0x20        /* Process Login */
#define FC_ELS_PRLO     0x21        /* Process Logout */
#define FC_ELS_SCN      0x22        /* State Change Notification - obsolete */
#define FC_ELS_TPLS     0x23        /* Test Process Login State */
#define FC_ELS_TPRLO    0x24        /* Third Party Process Logout */
#define FC_ELS_LCLM     0x25        /* Login Control List Management */
#define FC_ELS_GAID     0x30        /* Get Alias_ID */
#define FC_ELS_FACT     0x31        /* Fabric Activate Alias_ID */
#define FC_ELS_FDACT    0x32        /* Fabric Deactivate Alias_ID */
#define FC_ELS_NACT     0x33        /* N_Port Activate Alias_ID */
#define FC_ELS_NDACT    0x34        /* N_Port Deactivate Alias_ID */
#define FC_ELS_QOSR     0x40        /* Quality of Service Request - obsolete */
#define FC_ELS_RVCS     0x41        /* Read Virtual Circuit Status - obsolete */
#define FC_ELS_PDISC    0x50        /* Discover N_Port Service Parameters */
#define FC_ELS_FDISC    0x51        /* Discover F_Port Service Parameters */
#define FC_ELS_ADISC    0x52        /* Discover Address */
#define FC_ELS_RNC      0x53        /* Report Node Capability - obsolete */
#define FC_ELS_FARP_REQ 0x54        /* Address Resolution Protocol Request - obsolete */
#define FC_ELS_FARP_REPLY   0x55    /* Address Resolution Protocol Reply - obsolete */
#define FC_ELS_RPS      0x56        /* Read Port Status Block - obsolete */
#define FC_ELS_FAN      0x57        /* Fabric Address Notification */
#define FC_ELS_RSCN     0x61        /* Registered State Change Notification */
#define FC_ELS_SCR      0x62        /* State Change Registration */
#define FC_ELS_RNFT     0x63        /* Report Node FC-4 Types */
#define FC_ELS_CSR      0x68        /* Clock Synchronization Request */
#define FC_ELS_CSU      0x69        /* Clock Synchronization Update */
#define FC_ELS_LINIT    0x70        /* Loop Initialize */
#define FC_ELS_LPC      0x71        /* Loop Port Control - obsolete */
#define FC_ELS_LSTS     0x72        /* Loop Status */
#define FC_ELS_VENDOR   0x77        /* Vendor Specific */
#define FC_ELS_RNID     0x78        /* Request Node Identification Data */
#define FC_ELS_RLIR     0x79        /* Registered Link Incident Report */
#define FC_ELS_LIRR     0x7A        /* Link Incident Record Registration */
#define FC_ELS_SRL      0x7B        /* Scan Remote Loop */
#define FC_ELS_SBRP     0x7C        /* Set Bit-error Reporting Parameters */
#define FC_ELS_RPSC     0x7D        /* Report Port Speed Capabilities */
#define FC_ELS_QSA      0x7E        /* Query Security Attributes */
#define FC_ELS_EVFP     0x7F        /* Exchange Virtual Fabrics Parameters */
#define FC_ELS_LKA      0x80        /* Link Keep Alive */
#define FC_ELS_AUTH     0x90        /* Authentication ELS */
#define FC_ELS_RFCN     0x97        /* Request Fabric Change Notification */
#define FC_ELS_FFI_DTM  0xA0        /* Define FFI Domain Topology Map */
#define FC_ELS_FFI_RTM  0xA1        /* Request FFI Domain Topology Map */
#define FC_ELS_FFI_PSS  0xA2        /* FFI AE Principal Switch Selector */
#define FC_ELS_FFI_MUR  0xA3        /* FFI Map Update Registration */
#define FC_ELS_FFI_RMUN 0xA4        /* FFI Registered Map Update Notification */
#define FC_ELS_FFI_SMU  0xA5        /* FFI Suspend Map Updates */
#define FC_ELS_FFI_RMU  0xA6        /* FFI Resume Map Updates */

/*
******************************************************************************
** FC-Generic services
******************************************************************************
*/
/* GS defines*/

/* revision 1 indicates GS-2 not GS-3/4*/
#define FC_GS_REVISION  1

#define FC_GS_TYPE_KEY_SERVICE          0xF7
#define FC_GS_TYPE_ALIAS_SERVICE        0xF8
#define FC_GS_TYPE_MANAGMENT_SERVICE    0xFA
#define FC_GS_TYPE_TIME_SERVICE         0xFB
#define FC_GS_TYPE_DIRECTORY_SERVICE    0xFC
#define FC_GS_TYPE_FABRIC_CONTROLLER    0xFD

#define FC_GS_SUBTYPE_NAME_SERVICE      0x02

#define GAN_NXT                         0x100
#define GPN_ID                          0x112   /* Get Port Name                        */
#define GNN_ID                          0x113   /* Get Node Name                        */
#define GID_PN                          0x121   /* Get Port Identifier Port name        */
#define GID_NN                          0x131    /* Get Port Identifier Node Name        */
#define GID_FT                          0x171   /* Get list of port IDs                 */
#define GNN_FT                          0x173   /* Get list of port IDs and Node Names  */
#define GID_PT                          0x1A1   /* Get port ID                          */
#define RFT_ID                          0x217   /* Register FC-4 Types subcommand       */
#define DA_ID                           0x300   /* Remove All subcommand                */

/*note these are byte swapped so they work*/
#define FS_ACC                          0x0280
#define FS_RJT                          0x0180

/* CT Comman Header Structure */
typedef struct CTIU_PREAMBLE {
    UINT8  gsRevision;
    UINT8  Inid[3];
    UINT8  gsType;
    UINT8  gsSubType;
    UINT8  gsOptions;
    UINT8  rsvd;
    union {
        UINT16 ct_command;  /*common response code */
        UINT16 ct_response_code;
    };
    UINT16 ct_residual_size;
    UINT8  ct_fragmentid;
    UINT8  ct_reason_code;
    UINT8  ct_reason_code_expln;
    UINT8  ct_vend_specific;
} CTIU_PREAMBLE;

#define RFTID_REQ_LEN       52
#define RFTID_RESP_LEN      16

typedef struct CT_RFTID_REQ {
    CTIU_PREAMBLE  ctreq_hdr;
    UINT32 portid;
    UINT32 fc4Types[8];
} CT_RFTID_REQ;

typedef struct CT_RFTID_RESP {
     CTIU_PREAMBLE  ctrsp_hdr;
} CT_RFTID_RESP;

typedef struct CT_GPN_ID_REQ {
    CTIU_PREAMBLE   ctreq_hdr;
    UINT32          portid;
} CT_GPN_ID_REQ;

typedef struct CT_GPN_ID_RESP {
    CTIU_PREAMBLE  ctrsp_hdr;
    UINT64         portName;
} CT_GPN_ID_RESP;

typedef struct CT_GID_PN_REQ {
    CTIU_PREAMBLE   ctreq_hdr;
    UINT64         portName;
} CT_GID_PN_REQ;

typedef struct CT_GID_PN_RESP {
    CTIU_PREAMBLE   ctrsp_hdr;
    UINT32          portid;
} CT_GID_PN_RESP;

typedef struct CT_GID_NN_REQ {
    CTIU_PREAMBLE   ctreq_hdr;
    UINT64         NodeName;
} CT_GID_NN_REQ;

typedef struct CT_GID_NN_RESP {
    CTIU_PREAMBLE   ctrsp_hdr;
    UINT32          portid[16];
} CT_GID_NN_RESP;

typedef struct CT_GNN_ID_REQ {
    CTIU_PREAMBLE   ctreq_hdr;
    UINT32          portid;
} CT_GNN_ID_REQ;

typedef struct CT_GNN_ID_RESP {
    CTIU_PREAMBLE  ctrsp_hdr;
    UINT64         nodeName;
} CT_GNN_ID_RESP;

#define GNN_MAX             256     /* Maximum devices on GNN SNS request   */
typedef struct CT_GNN_FT_REQ {
    CTIU_PREAMBLE   ctreq_hdr;
    UINT8           rsvd1;
    UINT8           domain_id_scope;
    UINT8           area_id_scope;
    UINT8           fc4Protocol;
} CT_GNN_FT_REQ;

typedef struct CT_GNN_FT_RESP {
    CTIU_PREAMBLE   ctrsp_hdr;
    struct
    {
        UINT32  portId;
        UINT32  rsvd;
        UINT64  nodeName;
    } device[GNN_MAX];
} CT_GNN_FT_RESP;

typedef struct CT_GID_PT_REQ {
    CTIU_PREAMBLE   ctreq_hdr;
    UINT8           portType;
    UINT8           domain_id_scope;
    UINT8           area_id_scope;
    UINT8           rsvd1;
} CT_GID_PT_REQ;

typedef struct CT_GID_PT_RESP {
    CTIU_PREAMBLE   ctrsp_hdr;
    UINT32  portId[508];
} CT_GID_PT_RESP;

typedef struct CT_DA_ID_REQ {
    CTIU_PREAMBLE   ctreq_hdr;
    UINT32          portid;
} CT_DA_ID_REQ;

typedef struct CT_DA_ID_RESP {
    CTIU_PREAMBLE   ctrsp_hdr;
} CT_DA_ID_RESP;

#define GAN_NXT_REQ_LEN           20
#define GAN_NXT_RSP_LEN           636
typedef struct CT_GAN_REQ {
    CTIU_PREAMBLE  ctreq_hdr;
    UINT32 portid;
} CT_GAN_REQ;

typedef struct CT_GAN_RESP {
    CTIU_PREAMBLE  ctrsp_hdr;
    union {
        struct {
            UINT8  gan_ptype;
            UINT8  gan_pid16_23;
            UINT16 gan_pid0_15;
        };
        UINT32 gan_pid;
    };
    UINT64 gan_Pname;
    UINT8  gan_spnl;
    UINT8  gan_spn[255];
    UINT64 gan_Nname;
    UINT8  gan_snnl;
    UINT8  gan_snn[255];
    UINT8  init_proc_assoc[8];
    UINT8  gan_IPAddr[16];
    UINT8  gan_COS[4];
    UINT8  gan_fc4Types[32];
    UINT8  ip_address[16];
    UINT8  gan_FPname[8];
    UINT8  reserved;
    UINT8  gan_HA[3];
} CT_GAN_RESP;

#endif /* FC_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
***/
