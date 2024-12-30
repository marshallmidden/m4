/* $Id: DEF_iSCSI.h 145021 2010-08-03 14:16:38Z m4 $ */
/**
******************************************************************************
**
**  @file       DEF_iSCSI.h
**
**  @brief      iSCSI Definitions
**
**  To provide a common means of defining the ISP common structures.
**
**  Copyright (c) 2005-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
/**
**  @defgroup ISP_DEFS_H ISP Definitions
**  @{
**/
#ifndef _DEF_iSCSI_H_
#define _DEF_iSCSI_H_

#include "XIO_Types.h"
#include "XIO_Const.h"

/* if c++ pack on one byte boundaries so EWOK code compiles correctly
 * to match Wookiee compiler command line options
 */
#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/**
    MRISCSITGTCONFIG_REQ and
    MRISCSITGTCONFIG_RSP structures keeps a bitmap whose bits
    indicate whether the parameter can be configured or not

    bit
     0  ip;
     1  subnetMask;
     2  gateway;
     3  maxConnections;

     4  initialR2T;
     5  immediateData;
     6  dataSequenceInOrder;
     7  dataPDUInOrder;
     8  ifMarker;
     9  ofMarker;
    10  errorRecoveryLevel;
    11  targetPortalGroupTag;
    12  maxBurstLength;
    13  firstBurstLength;
    14  defaultTime2Wait;
    15  defaultTime2Retain;
    16  maxOutstandingR2T;
    17  maxRecvDataSegmentLength;
    18  ifMarkInt;
    19  ofMarkInt;
    20  headerDigest;
    21  dataDigest;
    22  authMethod;

**/
#define KEY0_IP 0
#define KEY1_SUBNET_MASK 1
#define KEY2_GATEWAY 2
#define KEY3_MAX_CONNECTIONS 3
#define KEY4_INITIAL_R2T 4
#define KEY5_IMMEDIATE_DATA 5
#define KEY6_DATA_SEQUENCE_IN_ORDER 6
#define KEY7_DATA_PDU_IN_ORDER 7
#define KEY8_IF_MARKER 8
#define KEY9_OF_MARKER 9
#define KEY10_ERROR_RECOVERY_LEVEL 10
#define KEY11_TARGET_PORTAL_GROUP_TAG 11
#define KEY12_MAX_BURST_LENGTH 12
#define KEY13_FIRST_BURST_LENGTH 13
#define KEY14_DEFAULT_TIME2_WAIT 14
#define KEY15_DEFAULT_TIME2_RETAIN 15
#define KEY16_MAX_OUTSTANDING_R2T 16
#define KEY17_MAX_RECV_DATASEGMENT_LENGTH 17
#define KEY18_IFMARK_INT 18
#define KEY19_OFMARK_INT 19
#define KEY20_HEADER_DIGEST 20
#define KEY21_DATA_DIGEST 21
#define KEY22_AUTHMETHOD 22
#define KEY23_MTUSIZE                         23
#define KEY24_TGTALIAS                        24
#define KEY25_MAX_SEND_DATASEGMENT_LENGTH     25
/*
** initiator and server are synonym in iSCSI context
*/

typedef struct chapInfo_node
{
    UINT8                  sname[256];          /*initiator uses this name in for CHAP authentication*/
    UINT8                  secret1[32];         /*target uses this secret to authenticate initiator*/
    UINT8                  secret2[32];         /*initiator uses this secret to authenticate target*/
    struct chapInfo_node  *fthd;
} CHAPINFO;
/*
** Following parameters can be configured by user
** ipAddr
** ipMask
** ipGw
** maxRecvDataSegmentLength
** headerDigest
** dataDigest
** authMethod
** mtuSize
** tgtAlias
** For iSCSI parameters more details can be found in RFC 3720 section 12.
*/

typedef struct I_TGD
{
    UINT16  tid;                       /* target id*/
    UINT32  ipAddr;                    /* ip address*/
    UINT32  ipMask;                    /* ip subnet mask */
    UINT32  ipGw;                      /* ip gateway     */
    UINT16  maxConnections;            /* an iSCSI parameter, maximum number of connection an iSCSI session could have
                                       ** default 1
                                       ** valid (1 to 65535)
                                       */

    UINT8   initialR2T;                /* an iSCSI parameter, if yes then target does not support unsolicited data
                                       ** default - 1
                                       ** valid - 0,1
                                       */
    UINT8   immediateData;             /* an iSCSI parameter, which controls the unsolicited data
                                       ** default -0
                                       ** valid - 0,1
                                       */
    UINT8   dataSequenceInOrder;       /* an iSCSI parameter, if yes data offset must be in sequence
                                       ** default -1
                                       ** valid - 0,1
                                       */
    UINT8   dataPDUInOrder;            /* an iSCSI parameter, if yes data PDU sequence must be in non-decreasing sequence
                                       ** default -1
                                       ** valid - 0,1
                                       */
    UINT8   ifMarker;                  /* an iSCSI parameter, used to turn on/off marker from target to initiator
                                       ** default -0
                                       ** valid - 0,1
                                       */
    UINT8   ofMarker;                  /* an iSCSI parameter, used to turn on/off marker from initiator to target
                                       ** default -0
                                       ** valid - 0,1
                                       */
    UINT8   errorRecoveryLevel;        /* an iSCSI parameter, Error Recovery Level
                                       ** default 0
                                       ** valid - 0,1,2
                                       */
    UINT16  targetPortalGroupTag;      /* an iSCSI parameter, declared by target, by default it is same as tid
                                       ** default same as tid
                                       ** valid - 16 bit binary value
                                       */
    UINT32  maxBurstLength;            /* an iSCSI parameter, maximum SCSI data payload in bytes in a data sequence
                                       ** default 2^(24-1)
                                       ** valid - 512 to 2^(24-1)
                                       */
    UINT32  firstBurstLength;          /* an iSCSI parameter, max   imum amount in bytes for unsolicited data in
                                       ** single SCSI command
                                       ** default 2^(24-1)
                                       ** valid - 512 to 2^(24-1)
                                       */
    UINT16  defaultTime2Wait;          /* an iSCSI parameter, minimum time (in seconds) to wait before attempting a
                                       **  logout or active task reassignment after an unexpected connection termination
                                       **  has occurred
                                       ** default 2
                                       ** valid -0 t0 3600
                                       */
    UINT16  defaultTime2Retain;        /* an iSCSI parameter, maximum time (in seconds) after Time2Wait before which
                                       **    an active task reassignment is possible
                                       ** default 20
                                       ** valid -0 t0 3600
                                       */
    UINT16  maxOutstandingR2T;         /*  maximum number of outstannding R2T per task
                                       ** default 1
                                       ** valid - 1 to 65535
                                       */
    UINT32  maxRecvDataSegmentLength;  /* an iSCSI parameter, maximum Data to receive in a PDU
                                       ** default  65535
                                       ** valid - 512  to 2^(24-1)
                                       */
    UINT16  ifMarkInt;                 /* an iSCSI parameter, value of interval for ifMarker
                                       ** default  0
                                       ** valid - 1 to 65535
                                       */
    UINT16  ofMarkInt;                 /* an iSCSI parameter, value of interval for ofMarker
                                       ** default  0
                                       ** valid - 1 to 65535
                                       */

    UINT8   headerDigest;
                                        /*  an iSCSI parameter, digest type to use for Header
                                             Bit 0 - CRC32C
                                             Bit 7 - if set digest use will be mandatory
                                            remaining bits - for future usage
                                            default - CRC32C
                                        */
    UINT8  dataDigest;
                                        /* an iSCSI parameter, digest type to use for Data
                                            Bit 0 - CRC32C
                                            Bit 7 - if set digest use will be mandatory
                                            remaining bits - for future usage
                                            default - CRC32C
                                        */
    UINT8  authMethod;
                                        /*  an iSCSI parameter, for authentication method
                                            Bit 0 - CHAP,
                                            Bit 7 - if set authentication is must
                                            remaining bits - for future usage like SRP , KERB
                                            default - CHAP
                                        */
    UINT32  mtuSize;                    /*Maximum Transfer Unit for jumbo frames
                                       ** default 1500
                                       **
                                       */
    UINT8   tgtAlias[32];               /*an iSCSI parameter, target alias
                                       ** default (NULL)
                                       ** valid - 256 bytes string
                                       */
    UINT32  maxSendDataSegmentLength;   /*initiator side maxRecvDataSegmentLength */
    UINT32     numUsers;                /*Total number of initiators configured for CHAP authentication */
    CHAPINFO  *chapInfo;                /* pointer to CHAP information*/
    UINT8      rsvd1[2];
}I_TGD;

#define NUM_ISCSI_CONFIG_PARAM 26

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _DEF_iSCSI_H_ */
/* @} */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
