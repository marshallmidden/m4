/* $Id: iscsi_pdu.h 161368 2013-07-29 14:53:10Z marshall_midden $ */
/**
 ******************************************************************************
 **
 **  @file       iscsi_pdu.h
 **
 **  @brief      iSCSI PDU functions header file
 **
 **  This provides structures for iSCSI PDU(s)
 **
 **  Copyright (c) 2005-2010 Xiotech Corporation.  All rights reserved.
 **
 ******************************************************************************
 **/

#ifndef __ISCSI_PDU_H
#define __ISCSI_PDU_H

#include "sgl.h"
#include "ilt.h"
#include "misc.h"
#include "scsi.h"
#include "chap_new.h"
#include "DEF_iSCSI.h"
#include "target.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

#define ISCSI_MAX_LUN                       64          // NOTE: MAX FE LUNS change needed here.
#define ISCSI_NO_TID                        0xffff

#define MAX_COMMANDS                        64
#define NORMAL_SESSION                      0
#define DISCOVERY_SESSION                   1

/* target definitions and structures */

#define NUM_PARAMS                          33
#define NUM_PARAM_VALS                      40
#define NUMLISTVALS                         32
#define MAX_KEYNAME_LEN                     63
#define MAX_KEYVAL_LEN                      4096
#define ISCSI_ISID_LEN                      6


#define ISCSI_NNAME_LEN                     254   /* max length is 255 bytes (0-254) */
#define DATA_IN_FLAG_S                      0x01   /* Flag S     */
/*
** AHS defines
*/
#define ISCSI_AHS_CDB                       0x01
#define ISCSI_AHS_RD_LEN                    0x02

/*
** SCSI Read/Write commands
*/
#define ISCSI_SCSI_FIN                      0x80
#define ISCSI_SCSI_RD                       0x40
#define ISCSI_SCSI_WR                       0x20

/*
** SCSI Command task attributes
*/
#define ISCSI_SCSI_UNTAGGED                 0x00
#define ISCSI_SCSI_SIMPLE                   0x01
#define ISCSI_SCSI_ORDERED                  0x02
#define ISCSI_SCSI_QUEUE_HEAD               0x03
#define ISCSI_SCSI_ACA                      0x04

/*
** SCSI response defines
*/
#define SCSI_CMD_COMPLETED                  0x00
#define SCSI_CMD_TRGT_FAILURE               0x01

/*
** SCSI status types
*/
#define SCSI_CMD_GOOD                       0x00
#define SCSI_CMD_CHK_CONDITION              0x02
#define SCSI_CMD_BUSY                       0x08
#define SCSI_CMD_RSVN_CONFLICT              0x18
#define SCSI_CMD_TASK_SET_FULL              0x28
#define SCSI_CMD_ACA_ACTIVE                 0x30
#define SCSI_CMD_TASK_ABORTED               0x40

/*
** SCSI Response flab byte
*/
#define SCSI_RESP_FLAG_BRRO                 0x10 /* (o) Bidirectional Read Residual Overflow  */
#define SCSI_RESP_FLAG_BRRU                 0x08 /* (u) Bidirectional Read Residual Underflow */
#define SCSI_RESP_FLAG_RO                   0x04 /* (O) Residual Overflow                     */
#define SCSI_RESP_FLAG_RU                   0x02 /* (O) Residual Underflow                    */

/*
** ISCSI SNACK defines
*/
#define DATA_R2T_SNACK                      0x0
#define STATUS_SNACK                        0x1
#define DATAACK_SNACK                       0x2
#define R_DATA_SNACK                        0x3

/*
 ** Login/Text Parameters
 */
#define SENDTARGETS_ALL                     "All"
#define HEADERDIGEST                        "HeaderDigest"
#define DATADIGEST                          "DataDigest"
#define MAXCONNECTIONS                      "MaxConnections"
#define SENDTARGETS                         "SendTargets"
#define TARGETNAME                          "TargetName"
#define INITIATORNAME                       "InitiatorName"
#define TARGETALIAS                         "TargetAlias"
#define INITIATORALIAS                      "InitiatorAlias"
#define TARGETADDRESS                       "TargetAddress"
#define TARGETPORTALGROUPTAG                "TargetPortalGroupTag"
#define INITIALR2T                          "InitialR2T"
#define IMMEDIATEDATA                       "ImmediateData"
#define MAXRECVDATASEGMENTLENGTH            "MaxRecvDataSegmentLength"
#define MAXBURSTLENGTH                      "MaxBurstLength"
#define FIRSTBURSTLENGTH                    "FirstBurstLength"
#define DEFAULTTIME2WAIT                    "DefaultTime2Wait"
#define DEFAULTTIME2RETAIN                  "DefaultTime2Retain"
#define MAXOUTSTANDINGR2T                   "MaxOutstandingR2T"
#define DATAPDUINORDER                      "DataPDUInOrder"
#define DATASEQUENCEINORDER                 "DataSequenceInOrder"
#define ERRORRECOVERYLEVEL                  "ErrorRecoveryLevel"
#define SESSIONTYPE                         "SessionType"

#define AUTHMETHOD                          "AuthMethod"
#define IFMARKER                            "IFMarker"
#define OFMARKER                            "OFMarker"
#define IFMARKINT                           "IFMarkInt"
#define OFMARKINT                           "OFMarkInt"
#define CHAP_A                              "CHAP_A"
#define CHAP_N                              "CHAP_N"
#define CHAP_R                              "CHAP_R"
#define CHAP_C                              "CHAP_C"
#define CHAP_I                              "CHAP_I"

/*
 ** Parameter Indices
 */
#define HEADERDIGESTINDEX                   0
#define DATADIGESTINDEX                     1
#define MAXCONNECTIONSINDEX                 2
#define SENDTARGETSINDEX                    3
#define TARGETNAMEINDEX                     4
#define INITIATORNAMEINDEX                  5
#define TARGETALIASINDEX                    6
#define INITIATORALIASINDEX                 7
#define TARGETADDRESSINDEX                  8
#define TARGETPORTALGROUPTAGINDEX           9
#define INITIALR2TINDEX                     10
#define IMMEDIATEDATAINDEX                  11
#define MAXRECVDATASEGMENTLENGTHINDEX       12
#define MAXBURSTLENGTHINDEX                 13
#define FIRSTBURSTLENGTHINDEX               14
#define DEFAULTTIME2WAITINDEX               15
#define DEFAULTTIME2RETAININDEX             16
#define MAXOUTSTANDINGR2TINDEX              17
#define DATAPDUINORDERINDEX                 18
#define DATASEQUENCEINORDERINDEX            19
#define ERRORRECOVERYLEVELINDEX             20

#define SESSIONTYPEINDEX                    21

#define AUTHMETHODINDEX                     22
#define IFMARKERINDEX                       23
#define OFMARKERINDEX                       24
#define IFMARKINTINDEX                      25
#define OFMARKINTINDEX                      26
#define CHAP_AINDEX                         27
#define CHAP_NINDEX                         28
#define CHAP_RINDEX                         29
#define CHAP_CINDEX                         30
#define CHAP_IINDEX                         31

/* Put these after all parameters */
#define MAXSENDDATASEGMENTLENGTHINDEX       32

/*
 ** Login/Text Responses
 */
#define NOTUNDERSTOOD                       "NotUnderstood"
#define IRRELEVANT                          "Irrelevant"
#define REJECT                              "Reject"
#define NONE                                "None"

/*
 ** Key Value Validation Levels
 */
#define SENDNOTUNDERSTOOD                   0
#define SENDIRRELEVANT                      1
#define SENDREJECT                          2
#define SENDNEGOTIATE                       3
#define SENDOK                              4

#define VALNONE                             -3
#define VALREJECT                           -2
#define VALNEGOTIATE                        -1
#define VALOK                               0

#define ISCSI_MAX_PORTS                     4
#define ISCSI_DEFAULT_PORT                  3260

/*
** NOP timer related defines
** RFC 3720 section 9.3 iSCSI recovery actions are dependent on iSCSI time-outs being
** recognized and acted upon before SCSI time-outs: We assume SCSI timeouts to be 90 so
** idle connection cleanup should happen before that.
** turn-around time *  multiplied by a safery factor 4 is good enough.
** turn around time is assumed to be 4 seconds (because of H/W, links, TCP/IP stack) * 4 = 16 Secs
*/
#define NOPIN_TIMER_VALUE                   16 /* 16 seconds NOPIN/NOPOUT Timer */

/*
** Max nopins the traget can send before
** tearing down the connection
*/
#define MAX_OUTSTANDING_NOPINS              2 /*
                                              **   try 2 NOPIN(s); 16*2 + 16(sec initial idle time)=> 48 sec
                                              **   this is good enough before we cleanup connection
                                              */

/* target NOP In  */
#define ISCSI_TARG_NOPIN_REQ                1
#define ISCSI_TARG_NOPIN_RESP               2
#define ISCSI_TARG_NOPIN                    3 /* target doen't expect a response for this */

#define ISCSI_HDR_RSVDFF                    0xffffffff
#define MAX_INT                             0xFFFFFFFF
#define SSN_REINST                          10

/*
** stage codes for CSG and NSG as
*/
#define CSG_SECURITY             0x00
#define CSG_LOGIN_OP_NEG         0x01 /* Login operational negotiation */
#define CSG_FFP                  0x03 /* Full feature phase */
#define CSG_INVALID              0x02 /* Invalid stage */

#define CSG_OPCODE               0x0c /* CSG OPCODE */
#define NSG_OPCODE               0x03 /* NSG OPCODE */

#define UNSET_TR_BIT            2  /*return value for ProcTxtMsg. This means Target is sending some parameter and it expects
                                    a login request so it can't transit to next phase*/

/*
 ** iSCSI Connection states for target as per RFC 3720
 ** connection state machine(CSM).
 */
typedef enum connState4Trgt
{
    CONNST_FREE,                    /* S0 */
    CONNST_XPT_WAIT,                /* S1 */
    CONNST_XPT_UP,                  /* S2 */
    CONNST_IN_LOGIN,                /* S3 */
    CONNST_LOGGED_IN,               /* S4 */
    CONNST_IN_LOGOUT,               /* S5 */
    CONNST_LOGOUT_REQUESTED,        /* S6 */
    CONNST_CLEANUP_WAIT,            /* S7 */
} connState4Trgt;

/*
 ** connection Events for target as per RFC 3720
 */
typedef enum connEvent4Trgt
{
    CONNEVT_ILL,                /* T0 - illegal           */
    CONNEVT_ILL1,               /* T1 - illegal           */
    CONNEVT_VALID_CONN,         /* T2 - csmtRcvdValidConn */
    CONNEVT_INIT_LOGIN,         /* T3 - initLoginReqRecvd */
    CONNEVT_FIN_LOGIN,          /* T4 - finLoginReqRecvd  */
    CONNEVT_LOGIN_TIMEOUT,      /* T5 - LoginTimedOut     */
    CONNEVT_CONN_CLOSE,         /* T6 - connCloseEvts     */
    CONNEVT_CONN_CLEAN,         /* T7 - connClean         */
    CONNEVT_LOGOUT_RECVD,       /* T8 - logoutRecvd       */
    CONNEVT_LOGOUT_RECVD1,      /* T9 - logoutRecvd       */
    CONNEVT_ASYNC_EVT2SEND,     /* T10 - asyncEvt2Send    */
    CONNEVT_ASYNC_EVT2SEND1,    /* T11 - asyncEvt2Send    */
    CONNEVT_TP_CLOSE,           /* T12 - connClean        */
    CONNEVT_ILL2,               /* T13 - illegal          */
    CONN_TP_EVTS,               /* T14 - tpEvts           */
    CONN_TP_EVTS1,              /* T15 - tpEvts           */
    CONNEVT_LOGOUT_FAIL,        /* T16 - logoutProcFailure */
    CONNEVT_LOGOUT_SUCCESS,     /* T17 - logoutProcSuccess */
} connEvent4Trgt;

/*
 ** connection cleanup states for target/initiator
 */
typedef enum connCleanupState
{
    CSMC_CLEANUP_WAIT,      /* R1 */
    CSMC_IN_CLEANUP,        /* R2 */
    CSMC_FREE               /* R3 */
} connCleanupState;

/*
 ** connection cleanup events for target/initiator
 */
typedef enum connCleanupEvent
{
    CSMC_CONN_TIMEOUT,               /* M1,connection timeout                 */

    CSMC_LOGOUT_SSN_ON_DIFF_CONN,    /* M1, success logout resp on a different
                                     ** connection with "ssn close" reqeues
                                     */

    CSMC_I_REINST_IN_XPTUP,          /*
                                     ** M2,A connection/session reinstatement
                                     ** Login was received while in state XPT_UP
                                     */

    CSMC_E_LOGOUT_IN_LOGGEDIN,       /* M2, An explicit logout was received for
                                     ** this CID in state LOGGED_IN
                                     */

    CSMC_I_LOGOUT_FAILURE,          /* M3,CSM-I failed to reach LOGGED_IN and
                                     ** arrived into FREE instead
                                     */

    CSMC_E_LOGOUT_FAILURE,           /* M3,CSM-E either moved out of LOGGED_IN,
                                     ** Logout timed out and/or aborted, or an
                                     ** internal event that indicates a failed
                                     ** Logout processing was received. A Logout
                                     ** response (failure) was sent
                                     */

    CSMC_I_LOGOUT_SUCCESS,           /* M4, CSM-I reached state LOGGED_IN, or an
                                     ** internal event of sending a Logout response
                                     ** (success) on a different connection for a
                                     ** "close the session" Logout request was
                                     ** received
                                     */

    CSMC_E_LOGOUT_SUCCESS,           /* M4, CSM-E stayed in LOGGED_IN and an internal
                                     ** event indicating a successful Logout processing
                                     ** was received, or an internal event of sending
                                     ** a Logout response (success) on a different
                                     ** connection for a "close the session" Logout
                                     ** request was received
                                     */
} connCleanupEvent;

/*
 ** iSCSI Session states for target as per RFC 3720
 ** Session state machine.
 */
typedef enum ssnStates4Trgt
{
    SSN_FREE,               /* Q1 */
    SSN_ACTIVE,             /* Q2 */
    SSN_LOGGED_IN,          /* Q3 */
    SSN_FAILED,             /* Q4 */
    SSN_IN_CONTINUE         /* Q5 */
} ssnStates4Trgt;

/*
 ** session transitions
 */
typedef enum ssnTrn
{
    FIRST_CONN_INLOGIN,     /* The first iSCSI connection in the session had reached the IN_LOGIN state  */
    ONE_CONN_INLOGIN,       /* At least one iSCSI connection reached the LOGGED_IN state                 */
    GRACEFUL_CLOSE,         /* Graceful closing of the session via session closure                       */
    ILLEGAL,                /* Illegal                                                                   */
    SSN_FAILURE,            /* Session failure                                                           */
    SSN_TIMEOUT,            /* Session state timeout occurred                                            */
    SSN_CONT,               /* A session continuation attempt is initiated                               */
    LAST_SSN_CONT,          /* The last session continuation attempt failed                              */
    LOGIN_ON_LEADCONN,      /* Login attempt on the leading connection failed                            */
    SSN_CONT_SUCCESS,       /* A session continuation attempt succeeded                                  */
    SSN_CLOSE_SUCCESS       /* A successful session reinstatement cleanly closed the session             */
} ssnTrn;

/*
 ** STRUCTURE FOR KEY VALUES
 */
typedef union KEYVALUE
{
    UINT32 intval;
    UINT8  strval[MAX_KEYVAL_LEN + 1];
} KEYVALUE;

/*
 ** STRUCTURE FOR RANGE VALUES
 */
typedef struct RANGEVALUE
{
    UINT32 lo;
    UINT32 hi;
} RANGEVALUE;

/*
 ** STRUCTURE FOR KEY-VAL PAIRS
 */
typedef struct STT
{
    UINT8       keyname[MAX_KEYNAME_LEN + 1];   /* key              */
    UINT32      numvals;                        /* number of values */
    KEYVALUE    data[NUMLISTVALS];              /* values           */
} STT;

/*
 ** STRUCTURE FOR LIST OF KEY-VAL PAIRS (FOR STORING ENTIRE TEXT MESSAGE)
 */
typedef struct msglist
{
    STT decl;
    struct msglist* next;
} MSGLIST;

/*
 ** STRUCTURE FOR DEFAULT PARAMETER VALUES
 */
typedef struct PARAMVALS
{
    UINT8  keyName;
    UINT8  keyVal[32]; /* was 4k - now changed to 32 bytes */
} PARAMVALS;

typedef struct PARAM_TABLE
{
    INT32 index;
    INT32 userCount;
    PARAMVALS ptrParams[NUM_PARAM_VALS];
    CHAPINFO* pUserInfo;
} PARAM_TABLE;

/*
 ** iSCSI Session id
 */
typedef union ISCSI_SID
{
    UINT64 sid;
    struct {
        UINT8    isid[ISCSI_ISID_LEN];    /* Initiator Session ID */
        UINT16   tsih;                    /* Target Session ID    */
    };
} ISCSI_SID;

/**
 ******************************************************************************
 **
 **  iSCSI PDU(s) structures
 **
 ******************************************************************************
 **/
/*
 ** ISCSI Generic Header structure
 */
typedef struct ISCSI_GENERIC_HDR
{
    UINT8   opcode;     /* opcode                                       */
    UINT8   flags;      /* T|C|NSG|CSG                                  */
    UINT16  rsvd1;      /* Val 1 to be geniric                          */
    UINT32  ahsDataLen; /* 1 byte - AHS length, 3 bytes DataSegment len */
    UINT64  lun;        /* LUN                                          */
    UINT32  itt;        /* Initiator Task Tag                           */
    UINT32  ttt;        /* TTT                                          */
    UINT32  Sn;         /* SN                                           */
    UINT32  expStatSn;  /* ExpSN                                        */
    UINT8   cdb[16];    /* CDB                                          */
    UINT8    headerDigest[4];/* header digest                            */
} ISCSI_GENERIC_HDR;

/*
 ** ISCSI PDU
 */
typedef struct ISCSI_PDU
{
    struct ISCSI_GENERIC_HDR bhs;/* Basic header portion */
    UINT32 ahsLen;               /* ahs segment length   */
    void   *ahs;                 /* ahs data pointer     */
    INT32  position;             /* position             */
    UINT8  *extCdb;
    UINT32  cdbLen;
} ISCSI_PDU;

/*
** ISCSI Login Request Header
*/
typedef struct ISCSI_LOGIN_REQ_HDR
{
    UINT8   opcode;         /* opcode                                       */
    UINT8   flags;          /* T|C|NSG|CSG                                  */
    UINT8   verMax;         /* Version Max                                  */
    UINT8   verMin;         /* Version Min                                  */
    UINT32  ahsDataLen;     /* 1 byte - AHS length, 3 bytes DataSegment len */
    ISCSI_SID sid;  /* ISID                                         */
    UINT32  initTaskTag;    /* Initiator Task Tag                           */
    UINT16  cid;            /* connection Identifier                        */
    UINT16  rsvd1;          /* Reserved 1                                   */
    UINT32  cmdSn;          /* command SN                                   */
    UINT32  expStatSnOrRsvd;/* ExpStatSN or Reserved                        */
    UINT32 rsvd2;           /* Reserved  2                                  */
    UINT32 rsvd3;           /* Reserved  3                                  */
    UINT8   rsvd4[8];       /* Reserved  2                                  */
} ISCSI_LOGIN_REQ_HDR;

/*
** ISCSI Login Response Header
*/
typedef struct ISCSI_LOGIN_RESP_HDR
{
    UINT8   opcode;        /* opcode                                       */
    UINT8   flags;         /* T|C|NSG|CSG                                  */
    UINT8   verMax;        /* Version Max                                  */
    UINT8   verActive;     /* Version Active                               */
    UINT32  ahsDataLen;    /* 1 byte - AHS length, 3 bytes DataSegment len */
    ISCSI_SID sid; /* ISID                                         */
    UINT32  initTaskTag;   /* Initiator Task Tag                           */
    UINT32  rsvd1;         /* Reserved 1                                   */
    UINT32  statSn;        /* Status Sequence Number                       */
    UINT32  expCmdSn;      /* Expected command SN                          */
    UINT32  maxCmdSn;      /* Max command SN                               */
    UINT8   statClass;     /* Status Class                                 */
    UINT8   statDetail;    /* Status Details                               */
    UINT8  rsvd2[2];       /* Reserved 2                                   */
    UINT8  rsvd3[8];       /* Reserved 3                                   */
} ISCSI_LOGIN_RESP_HDR;

/*
** ISCSI Logout Request Header
*/
typedef struct ISCSI_LOGOUT_REQ_HDR
{
    UINT8   opcode;        /* opcode                                       */
    UINT8   flags;         /* Reason Code                                  */
    UINT16  rsvd1;         /* Reserved 1                                   */
    UINT32  ahsDataLen;    /* 1 byte - AHS length, 3 bytes DataSegment len */
    UINT8   rsvd2[8];      /* Reserved 2                                   */
    UINT32  initTaskTag;   /* Initiator Task Tag                           */
    UINT16  cidOrResvd;    /* connection Identifier or Rerserved           */
    UINT16  rsvd3;         /* Reserved 3                                   */
    UINT32  cmdSn;         /* command SN                                   */
    UINT32  expStatSn;     /* ExpStatSN                                    */
    UINT8  rsvd4[16];      /* Reserved 4                                   */
} ISCSI_LOGOUT_REQ_HDR;

/*
** ISCSI Logout Response Reason
*/
typedef struct ISCSI_LOGOUT_RESP_REASON
{
    bool     connSsnSuccess; /* conn or ssn closed successfully     */
    bool    isValidCid;      /* CID i                               */
    bool     isRecoverySupp; /* is recovery supp or not             */
    bool    CleanupFailed;   /* clean up failed for various reasons */
} ISCSI_LOGOUT_RESP_REASON;

/*
** ISCSI Logout Response Header
*/
typedef struct ISCSI_LOGOUT_RESP_HDR
{
    UINT8   opcode;        /* opcode                                       */
    UINT8   flags;         /* flags                                        */
    UINT16  resp;          /* Responce                                     */
    UINT32  ahsDataLen;    /* 1 byte - AHS length, 3 bytes DataSegment len */
    UINT8   rsvd1[8];      /* Reserved 1                                   */
    UINT32  initTaskTag;   /* Initiator Task Tag                           */
    UINT32  rsvd2;         /* Reserved 2                                   */
    UINT32  statSn;        /* Status Sequence Number                       */
    UINT32  expCmdSn;      /* Expected command SN                          */
    UINT32  maxCmdSn;      /* Max command SN                               */
    UINT8   rsvd3[4];      /* Reserved 3                                   */
    UINT16  time2Wait;     /* Time to wait                                 */
    UINT16  time2Retain;   /* Time to Retain                               */
    UINT8   rsvd4[4];      /* Reserved 4                                   */
} ISCSI_LOGOUT_RESP_HDR;

/*
** ISCSI Logout Reject Header
*/
typedef struct ISCSI_REJECT_HDR
{
    UINT8   opcode;        /* opcode                                       */
    UINT8   flags;         /* flags                                        */
    UINT8   reason;        /* reason                                       */
    UINT8   rsvd1;         /* Reserved  1                                  */
    UINT32  ahsDataLen;    /* 1 byte - AHS length, 3 bytes DataSegment len */
    UINT8   rsvd2[8];      /* Reserved  2                                  */
    UINT32  val;           /* 0xffffffff                                   */
    UINT32  resvd3;        /* Reserved 3                                   */
    UINT32  statSn;        /* Status Sequence Number                       */
    UINT32  expCmdSn;      /* Expected command SN                          */
    UINT32  maxCmdSn;      /* Max command SN                               */
    UINT32  dataSNorR2tsnorRsvd; /* Data SN/R2TSN/Reserved      */
    UINT8   rsvd4[8];            /* Reserved  4                 */
} ISCSI_REJECT_HDR;

/*
** ISCSI SNACK Request Header
*/
typedef struct ISCSI_SNACK_HDR
{
    UINT8   opcode;        /* opcode                                        */
    UINT8   flags;         /* flags                                         */
    UINT16  rsvd1;         /* Reserved  1                                   */
    UINT32  ahsDataLen;    /* 1 byte - AHS length, 3 bytes DataSegment len  */
    UINT8   rsvd2[8];      /* Reserved  2                                   */
    UINT32  initTaskTag;   /* intiator task tag or 0xffffffff               */
    UINT32  ttgOrSnackTag; /* Target transfer Tag or Snack Tag or 0xffffffff*/
    UINT32  rsvd3;         /* reserved 2                                    */
    UINT32  expCmdSn;      /* Expected command SN                           */
    UINT8   rsvd4[8];      /* reserved 3                                    */
    UINT32  begRun;
    UINT32  runLength;
} ISCSI_SNACK_HDR;

/*
** ISCSI SNACK defines
*/
#define DATA_R2T_SNACK                      0x0
#define STATUS_SNACK                        0x1
#define DATAACK_SNACK                       0x2
#define R_DATA_SNACK                        0x3

#define GET_SNACK_TYPE(flag)                ((flag) & 0xf)

/*
** ISCSI NOPIN Header
*/
typedef struct ISCSI_NOPIN_HDR
{
    UINT8   opcode;      /* opcode                                       */
    UINT8   rsvd1;       /* Reserved  1                                  */
    UINT16  resvd2;      /* Reserved 2                                   */
    UINT32  ahsDataLen;  /* 1 byte - AHS length, 3 bytes DataSegment len */
    UINT64  lunOrRsvd;   /* LUN or Reserved                              */
    UINT32  initTaskTag; /* Initiator Task Tag or 0xffffffff             */
    UINT32  ttt;         /* Target transfer tag or 0xffffffff            */
    UINT32  statSn;      /* status SN                                    */
    UINT32  expCmdSn;    /* ExpCmdSN                                     */
    UINT32  maxCmdSn;    /* maxCmdSN                                     */
    UINT8   rsvd3[12];   /* Reserved  3                                  */
} ISCSI_NOPIN_HDR;

/*
** ISCSI NOP out Header
*/
typedef struct ISCSI_NOPOUT_HDR
{
    UINT8   opcode;      /* opcode                                       */
    UINT8   rsvd1;       /* Reserved  1                                  */
    UINT16  resvd2;      /* Reserved 2                                   */
    UINT32  ahsDataLen;  /* 1 byte - AHS length, 3 bytes DataSegment len */
    UINT64  lunOrRsvd;   /* LUN or Reserved                              */
    UINT32  initTaskTag; /* Initiator Task Tag or 0xffffffff             */
    UINT32  ttt;         /* Target transfer tag or 0xffffffff            */
    UINT32  cmdSn;       /* command SN                                   */
    UINT32  expStatSn;   /* ExpStatSN                                    */
    UINT8   rsvd3[16];   /* Reserved  3                                  */
} ISCSI_NOPOUT_HDR;

/*
** ISCSI Assync Header
*/
typedef struct ISCSI_ASYNC_HDR
{
    UINT8   opcode;      /* opcode                                       */
    UINT8   rsvd1;       /* Reserved  1                                  */
    UINT16  resvd2;      /* Reserved 2                                   */
    UINT32  ahsDataLen;  /* 1 byte - AHS length, 3 bytes DataSegment len */
    UINT64  lunOrRsvd;   /* LUN or Reserved                              */
    UINT32  val;         /*  0xffffffff                                  */
    UINT32  resvd3;      /* Reserved 3                                   */
    UINT32  statSn;      /* status SN                                    */
    UINT32  expCmdSn;    /* ExpCmdSN                                     */
    UINT32  maxCmdSn;    /* maxCmdSN                                     */
    UINT8   asyncEvt;    /* Async event                                  */
    UINT8   asyncVCode;  /* Async Vcode                                  */
    UINT16  parm1orRsvd; /* Parameter1 or reserved                       */
    UINT16  parm2orRsvd; /* Parameter2 or reserved                       */
    UINT16  parm3orRsvd; /* Parameter3 or reserved                       */
    UINT32  rsvd4;       /* Reserved 4                                   */
} ISCSI_ASYNC_HDR;

/*
** ISCSI SCSI command Header
*/
typedef struct ISCSI_SCSI_CMD_HDR
{
    UINT8   opcode;      /* opcode                                       */
    UINT8   flags;       /* F|R|W|.|.|ATTR                               */
    UINT16  rsvd1;       /* reserved 1                                   */
    UINT32  ahsDataLen;  /* 1 byte - AHS length, 3 bytes DataSegment len */
    UINT64  LUN;         /* logical unit number                          */
    UINT32  initTaskTag; /* Initiator Task Tag                           */
    UINT32  expDataTxLen;/* Expected data transfer length                */
    UINT32  cmdSn;       /* command SN                                   */
    UINT32  expStatSn;   /* ExpStatSN                                    */
    UINT8   cdb[16];     /* SCSI command descriptor block                */
} ISCSI_SCSI_CMD_HDR;

/*
** ISCSI SCSI CBD AHS Header
*/
typedef struct ISCSI_SCSI_CDB_AHS_HDR
{
    UINT16 ahsLen;        /* 2 bytes - AHS length                         */
    UINT8  ahsType;       /* AHS type                                     */
    UINT8  rsvd;          /* Reserved val = 0x01                          */
    UINT8  extCdb;        /* Extended CDB                                 */
} ISCSI_SCSI_CDB_AHS_HDR;

/*
** ISCSI SCSI Read Data Length AHS Header
*/
typedef struct ISCSI_SCSIRD_DLEN_AHS_HDR
{
    UINT16 ahsLen;         /* 2 bytes - AHS length                        */
    UINT8  type;           /* AHS type, val = 0x02                        */
    UINT8  rsvd1;          /* Reserved 2                                  */
    UINT8  extRdDataLen;   /* Extended READ data length                   */
} ISCSI_SCSIRD_DLEN_AHS_HDR;

/*
** ISCSI Text Request Header
*/
typedef struct ISCSI_SCSI_RESP_HDR
{
    UINT8   opcode;              /* opcode                                       */
    UINT8   flags;               /* 1|.|.|o|u|O|U|.|                             */
    UINT8   resp;                /* resp                                         */
    UINT8   status;              /* status                                       */
    UINT32  ahsDataLen;          /* 1 byte -AHS length,3 bytes DataSegment len   */
    UINT8   rsvd1[8];            /* Reserved  2                                  */
    UINT32  initTaskTag;         /* Initiator Task Tag                           */
    UINT32  SNACKTagOrRsvd;      /* SNACK Tag or Reserved                        */
    UINT32  statSn;              /* Status Sequence Number                       */
    UINT32  expCmdSn;            /* Expected command SN                          */
    UINT32  maxCmdSn;            /* Max command SN                               */
    UINT32  expDataOrRsvd;       /* Expected data or reserved                    */
    UINT32  biReadResidCntOrRsvd;/*Bidirectional Read Residual Count or Reserved */
    UINT32  residCntOrRsvd;      /* Residual Count or Reserved                   */
} ISCSI_SCSI_RESP_HDR;

#define ISCSI_SCSI_SENSE_DATA      (sizeof(SNS))
#define SENSE_DATA_PRESENT         2
#define SENSE_DATA_LEN_FLD         2
#define ISCSI_SCSI_SENSE_SEG_SIZE  (sizeof(ISCSI_SCSI_SENSE_SEG))
/*
** SCSI Sense data segment
*/
typedef struct ISCSI_SCSI_SENSE_SEG
{
    UINT16 snsLen;                          /* sense length            */
    UINT8  snsData[ISCSI_SCSI_SENSE_DATA];  /* sense data              */

} ISCSI_SCSI_SENSE_SEG;

/*
** ISCSI Text Response Header
*/
typedef struct ISCSI_TEXT_RESP_HDR
{
    UINT8   opcode;      /* opcode                                       */
    UINT8   flags;       /* F|C                                          */
    UINT16  rsvd1;       /* reserved 1                                   */
    UINT32  ahsDataLen;  /* 1 byte - AHS length, 3 bytes DataSegment len */
    UINT64  LUN;         /* logical unit number                          */
    UINT32  itt;         /* Initiator Task Tag                           */
    UINT32  ttt;         /* Target Task Tag                              */
    UINT32  statSn;      /* Stat SN                                      */
    UINT32  expCmdSn;    /* expected command SN                          */
    UINT32  maxCmdSn;    /* Max cmd SN                                   */
    UINT8   rsvd[12];    /* reserved                                     */
} ISCSI_TEXT_RESP_HDR;

/*
** ISCSI Text Request Header
*/
typedef struct ISCSI_TEXT_REQ_HDR
{
    UINT8   opcode;      /* opcode                                       */
    UINT8   flags;       /* F|C                                          */
    UINT16  rsvd1;       /* reserved 1                                   */
    UINT32  ahsDataLen;  /* 1 byte - AHS length, 3 bytes DataSegment len */
    UINT64  LUN;         /* logical unit number                          */
    UINT32  itt;         /* Initiator Task Tag                           */
    UINT32  ttt;         /* Target Task Tag                              */
    UINT32  cmdSn;       /* command SN                                   */
    UINT32  expStatSn;   /* ExpStatSN                                    */
    UINT8   rsvd[16];    /* reserved                                     */
} ISCSI_TEXT_REQ_HDR;

/*
** ISCSI Data out Header
*/
typedef struct ISCSI_DATA_OUT_HDR
{
    UINT8   opcode;      /* opcode                                       */
    UINT8   flags;       /* F | reserved                                 */
    UINT16  rsvd1;       /* reserved 1                                   */
    UINT32  ahsDataLen;  /* 1 byte - AHS length, 3 bytes DataSegment len */
    UINT64  LUN;         /* logical unit number                          */
    UINT32  itt;         /* Initiator Task Tag                           */
    UINT32  ttt;         /* Target Task Tag                              */
    UINT32  rsvd2;       /* reserved 2                                   */
    UINT32  expStatSn;   /* Exp Stat SN                                  */
    UINT32  rsvd3;       /* reserved 3                                   */
    UINT32  dataSn;      /* Data SN                                      */
    UINT32  buffOffset;  /* Buffer offset                                */
    UINT32  rsvd4;       /* reserved 4                                   */
} ISCSI_DATA_OUT_HDR;

/*
** ISCSI Data in Header
*/
typedef struct ISCSI_DATA_IN_HDR
{
    UINT8   opcode;      /* opcode                                       */
    UINT8   flags;       /* |F|A|0 0 0|O|U|S                             */
    UINT8   rsvd1;       /* reserved 1                                   */
    UINT8   status;      /* status or reserved                           */
    UINT32  ahsDataLen;  /* 1 byte - AHS length, 3 bytes DataSegment len */
    UINT64  LUN;         /* logical unit number                          */
    UINT32  itt;         /* Initiator Task Tag                           */
    UINT32  ttt;         /* Target Task Tag                              */
    UINT32  statSn;      /* Stat SN                                      */
    UINT32  expCmdSn;    /* Exp Cmd SN                                   */
    UINT32  maxCmdSn;    /* Max cmd sn                                   */
    UINT32  dataSn;      /* Data SN                                      */
    UINT32  buffOffset;  /* Buffer offset                                */
    UINT32  resCnt;      /* residual count                               */
} ISCSI_DATA_IN_HDR;

/*
** ISCSI R2T Header
*/
typedef struct ISCSI_R2T_HDR
{
    UINT8   opcode;      /* opcode                                       */
    UINT8   flags;       /* F | reserved                                 */
    UINT16  rsvd1;       /* reserved 1                                   */
    UINT32  ahsDataLen;  /* 1 byte - AHS length, 3 bytes DataSegment len */
    UINT64  LUN;         /* logical unit number                          */
    UINT32  itt;         /* Initiator Task Tag                           */
    UINT32  ttt;         /* Target Task Tag                              */
    UINT32  statSn;      /* Status SN                                    */
    UINT32  expCmdSn;    /* Exp Cmd SN                                   */
    UINT32  maxCmdSn;    /* Max cmd sn                                   */
    UINT32  r2tSn;       /* r2t SN                                       */
    UINT32  buffOffset;  /* Buffer offset                                */
    UINT32  dataTxLen;   /* Desired data transfer length                 */
} ISCSI_R2T_HDR;
/*
** ISCSI TMF Request Header
*/
typedef struct ISCSI_TMF_REQ_HDR
{
    UINT8   opcode;         /* opcode                                       */
    UINT8   function;       /* 0x81-0x88                                    */
    UINT16  rsvd1;          /* reserved                                     */
    UINT32  ahsDataLen;     /* 1 byte - AHS length, 3 bytes DataSegment len */
    UINT64  LUN;            /* logical unit number                          */
    UINT32  itt;            /* Initiator Task Tag                           */
    UINT32  rtt;            /* Reference Task Tag or 0xffffffff             */
    UINT32  cmdSn;          /* CMD SN                                       */
    UINT32  expStatSn;      /*  expected stat sn                            */
    UINT32  refCmdSnorRsvd; /* reference Cmd SN  or 0xffffffff              */
    UINT32  expDataSnorRsvd;/* expected Data SN  or rsvd                    */
    UINT32  rsvd2;          /* Buffer offset                                */
    UINT32  rsvd3;          /* residual count                               */
} ISCSI_TMF_REQ_HDR;

/*
** ISCSI TMF Response Header
*/
typedef struct ISCSI_TMF_RES_HDR
{
    UINT8   opcode;         /* opcode                                       */
    UINT8   rsvd1;          /* reserved                                     */
    UINT8   response;       /* response                                     */
    UINT8   rsvd2;          /* reserved                                     */
    UINT32  ahsDataLen;     /* 1 byte - AHS length, 3 bytes DataSegment len */
    UINT64  rsvd3;          /* reserved                                     */
    UINT32  itt;            /* Initiator Task Tag                           */
    UINT32  rsvd4;          /* reserved                                     */
    UINT32  statSn;         /* stat SN                                      */
    UINT32  expCmdSn;       /*  expected cmd sn                             */
    UINT32  maxCmdSn;       /* Maximum Cmd SN                               */
    UINT32  rsvd5;          /* reserved                                     */
    UINT32  rsvd6;          /* reserved                                     */
    UINT32  rsvd7;          /* reserved                                     */
} ISCSI_TMF_RES_HDR;

/*
** ISCSI TMF Requests
*/
typedef enum TMF_REQUEST
{
    TMF_ABORT_TASK = 1,
    TMF_ABORT_TASK_SET,
    TMF_CLEAR_ACA,
    TMF_CLEAR_TASK_SET,
    TMF_LOGICAL_UNIT_RESET,
    TMF_WARM_RESET,
    TMF_COLD_RESET,
    TMF_TASK_REASSIGN    /*must be 8*/
} TMF_REQUEST;

/*
** ISCSI TMF Responses
*/
typedef enum TMF_RESPONSE
{
    TMF_FUNCTION_COMPLETE                   = 0,
    TMF_TASK_NOT_EXIST                      = 1,
    TMF_LUN_NOT_EXIST                       = 2,
    TMF_TASK_ALLEGIANT                      = 3,
    TMF_ALLEGIANCE_REASSIGN_NOT_SUPPORTED   = 4,
    TMF_FUNCTION_NOT_SUPPORTED              = 5,
    TMF_AUTHORIZATION_FAILED                = 6,
    TMF_FUNCTION_REJECTED                   = 255
} TMF_RESPONSE;

/*
** ISCSI Command states
*/
typedef enum COMMAND_STATE
{
    COMMAND_EMPTY = 0,      /* intial state                                    */
    COMMAND_ARRIVED,          /* command is arrived and the pointer pdu is valid */
    COMMAND_DELIEVERED_ISCSI, /* command is being processed by iSCSI layer       */
    COMMAND_DELIEVERED_SCSI,  /* command is delieverd to SCSI layer              */
    COMMAND_ABORTED
} COMMAND_STATE;

/*
** ISCSI Command
*/
typedef struct ISCSI_COMMAND
{
    ISCSI_PDU       pdu;   /* iSCSI PDU        */
    UINT8           *data;  /* Data             */
    COMMAND_STATE   state;  /* command state    */
    struct CONNECTION *pConn;
} ISCSI_COMMAND;

/*
** ISCSI Command queue
*/
typedef struct COMMAND_QUEUE
{
    ISCSI_COMMAND    cmdArray[MAX_COMMANDS];
    INT32 currentIndex;
    INT32  submittedToProc;
} COMMAND_QUEUE;

/*
** ISCSI Transport descriptor
*/
typedef struct ISCSI_TPD
{
    struct CONNECTION *pConn;
    INT32   socket_fd;
    UINT16  tid;
    UINT8   send_indicator;
    UINT8   write_event_enabled;
} ISCSI_TPD;

/*
** SESSION-WIDE PARAMETERS
*/
typedef struct SESSION_PARAMS
{
    UINT16 maxConnections;
    UINT8  targetName[ISCSI_NNAME_LEN];
    UINT8  initiatorName[ISCSI_NNAME_LEN];
    UINT8  targetAlias[ISCSI_NNAME_LEN];
    UINT8  initiatorAlias[ISCSI_NNAME_LEN];
    UINT8  targetAddress[ISCSI_NNAME_LEN];
    UINT16 targetPortalGroupTag;
    UINT32 maxBurstLength;
    UINT32 firstBurstLength;
    UINT16 defaultTime2Wait;
    UINT16 defaultTime2Retain;
    UINT16 maxOutstandingR2T;
    UINT8  initialR2T:1;
    UINT8  immediateData:1;
    UINT8  dataPDUInOrder:1;
    UINT8  dataSequenceInOrder:1;
    UINT8  errorRecoveryLevel:2;
    UINT8  sessionType:1;
    UINT32 paramMap;
    UINT32 paramSentMap;
    UINT8  rsvd[248];
} SESSION_PARAMS;

/*
** SESSION structure
*/
typedef struct SESSION
{
    struct SESSION *pNext;
    UINT8   ssnState;    /* session state machine statess     */
    UINT8   activeConns; /* number of active (FFP) connection */
    UINT8   version;     /* supported version                 */
    UINT8   rsvd1;       /* reserved                          */
    UINT32  cmdSN;       /* command SN                        */
    UINT32  expCmdSN;    /* expected command SN               */
    UINT32  maxCmdSN;    /* max command SN                    */
    union
    {
        UINT64 sid;
        struct
        {
            UINT8  isid[6];
            UINT16 tsih;
        };
    };
    UINT32  itt;         /* initiator task tag                */
    UINT32  ttt;         /* target transfer tag               */
    UINT16  tid;         /* target Id, to which it belongs    */
    UINT16  ssnRsvd1;    /* Reserved                          */
    struct CONNECTION *pCONN;
    SESSION_PARAMS      params;   /* Session parameters       */
    UINT32  firstCmdSN;          /* for testing purpose to wrap around of cmdSN */
    ISCSI_PDU         immediatePdu;
    COMMAND_QUEUE       commandQ; /* command queue            */
} SESSION;

/*
**  Connection params
*/
typedef struct CONN_PARAMS
{
    union    KEYVALUE headerDigest;
    union    KEYVALUE dataDigest;
    union    KEYVALUE authMethod;
    UINT8    chap_A;
    UINT8    ofMarker:1;
    UINT8    ifMarker:1;
    RANGEVALUE   ofMarkInt;
    RANGEVALUE   ifMarkInt;
    UINT32   maxRecvDataSegmentLength;
    UINT32   maxSendDataSegmentLength;
    UINT32   paramMap;
    UINT32   paramSentMap;
} CONN_PARAMS;

typedef enum connRecvState4Trgt
{
    IR_FREE,                /* Iscsi Recv Free */
    IR_BHS,
    IR_AHS,
    IR_DATA,
    IR_RECV_DATA,
    IR_RECV_DIGEST_0,
    IR_RECV_DIGEST_1,
    IR_RECV_DIGEST_2,
    IR_COMP
} connRecvState4Trgt;

typedef struct send_iovec
{
    UINT32  addr1;
    UINT32  len1;
    UINT32  addr2;
    UINT32  len2;
    UINT32  addr3;
    UINT32  len3;
    UINT32  addr4;
    UINT32  len4;
} SEND_IOVEC;

#define HASH_TABLE_SIZE 157

typedef struct hash_node
{
    struct hash_node *pNext;   /* pNext node */
    UINT32 key;                /* key        */
    UINT32 ttt;                /* ttt        */
    ILT *rec;                  /* user data  */
} hash_node;

typedef UINT16 hashIndexType;

typedef struct CONNECTION
{
    struct CONNECTION *pNext;
    bool   connCleanup;  /* for cleaning up connection              */
    bool   isChap;       /* is chap being negotiated or not         */
    UINT8  csg;          /* CSG state during login */
    UINT8  pHdr[ISCSI_HDR_LEN + 4];/* used when recv doesn't return 48 bytes first . +4 is to hold header digest*/
    UINT8  *ahs;
    UINT8  state;        /* Connection state */

    INT32  recvLen;      /* length of bytes recvd so fat            */
    UINT32 statSN;       /* status SN that target generates, incr by 1 for every resp -except retries */
    UINT32 expStatSN;    /* exp status SN   */
    UINT16 cid;          /* connection id   */
    UINT8  recvState;    /* recv state      */
    UINT8  rsvd1;        /* reserved 1      */

    CHAP_CONTEXT_ST *cc;  /* chap context scope is connection level  */

    ILT    *pSendIltHead;  /* sending ILT Head pointer */
    ILT    *pSendIltTail;  /* Sending ILT Tail pointer */
    UINT32 SendCount;      /* count                    */
    UINT8  send_state;     /* Send state               */

    struct SESSION     *pSession;/* Pointer to parent session */
    struct CONN_PARAMS params;   /* connection params         */
    struct ISCSI_TPD   *pTPD;    /* Transport descriptor      */

    UINT32 outstanding_nopins; /* Max nopins that the target can send before closing the connection */
    UINT32 timerid;            /* flag to check whether some activity is going on the connection */
    UINT8  timer_created;      /* This flag specifies that nopin timer task is created for this conn */
    UINT8  rsvd2[3];           /* Reserved */
    INT32  itt;                /* used to read remaining data  */
    INT32  offset;             /* same as above                */
    struct hash_node   *hashTable[HASH_TABLE_SIZE];
    UINT32 nopin_ttt[MAX_OUTSTANDING_NOPINS];
    UINT8  recvBuff[8192];    /* used when data receiving data portion   */

    UINT32 numPduRcvd;                    /**< No. of PDUs received so far */
    UINT32 numPduSent;                    /**< No. of PDUs sent so far     */
    UINT64 totalReads;                    /**< No. of Bytes read           */
    UINT64 totalWrites;                   /**< No. of bytes Writeen        */
    UINT32 ttt;                           /* used to read remaining data, retrieving r2tILT  */
} CONNECTION;

/*
**  Command Descriptor
*/
typedef struct COMMAND
{
    struct COMMAND*          pCommand;           /* Pointer to the Parent command*/
} COMMAND;

typedef struct CmdDX
{
    UINT32      count;
    COMMAND     *pCmdD[MAX_COMMANDS];
} CmdDX;

/*
** Reject Reason code
*/
typedef enum REJECT_REASON_CODE
{
    REJECT_DIGEST_ERROR = 0x02,
    REJECT_SNACK,
    REJECT_PROTOCOL_ERROR,
    REJECT_CMD_NOT_SUPPORTED,
    REJECT_IMMEDIATE_COMMAND,
    REJECT_TASK_IN_PROGESS,
    REJECT_INVALID_DATA_ACK,
    REJECT_INVALID_PDU_FIELD,
    REJECT_LONG_OPERATION,
    REJECT_NEGOTIATION_RESET,
    REJECT_WAITING_FOR_LOGOUT
} REJECT_REASON_CODE;

#define GET_SNACK_TYPE(flag)                ((flag) & 0xf)

/*
** sequence number check returns these  code
*/
#define EXPECTED_CMD 1
#define OUT_OF_ORDER_CMD 2
#define OUT_OF_WINDOW_CMD 3
#define IMMEDIATE_CMD 4
#define NO_CMD 5

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern UINT32 sessionSize;
extern UINT32 connectionSize;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern UINT8 iscsiHandleTpEvt(ISCSI_TPD*, UINT8);
extern UINT8  iscsiProcTxtReq(UINT8 *pReqBuff, UINT8 *pRespBuff, UINT32* length, SESSION* pSESSION,CONNECTION* pCONN);
extern INT32 iscsiSeqChkCmdSn(ISCSI_GENERIC_HDR *pCmd, SESSION *pSsn, UINT8 update);

extern UINT8 iscsiProcRecvdPdu(UINT8 *pRecvdHdr, ISCSI_TPD *pTPD, ISCSI_PDU *pPdu);
extern ILT *iscsiAddInboundPdu(ISCSI_GENERIC_HDR *pReq, CONNECTION *pConn, ISCSI_PDU *pPdu);
extern UINT32 iscsiGetLun(UINT8 *pLun);

extern UINT32 getPosition(UINT32 seqNo);
extern void iscsiSeqSetCmdSn(ISCSI_GENERIC_HDR *pCmd, UINT8 statSn, CONNECTION *pConn);

extern void iscsiSsmtInitState(SESSION *pSsn);
extern UINT8 iscsiSsmtUpdateState(SESSION *pSsn, UINT8 trEvent);

extern UINT8 iscsiSsnInit(SESSION **pSession);
extern SESSION *iscsiNormalSsnMatchISID(UINT64 sid, UINT16 tid,UINT8 *initName);
extern SESSION *iscsiSsnMatchTSIH(UINT16 tsih,UINT16 tid);
extern UINT8 iscsiSsnGetCurrState(SESSION *pSsn);
extern void iscsiSsnRemove(SESSION *pSsn);

extern UINT8 iscsiConnInit(CONNECTION **pConn);
extern CONNECTION *iscsiConnMatchCID(CONNECTION *pConn, UINT32 cid);
extern UINT8 iscsiConnGetCurrState(CONNECTION *pConn);

extern UINT8 iscsiFeProcMsg(ILT *pSecILT);

extern MSGLIST* SearchKey(MSGLIST* msghead, UINT8* keyname);
extern void InitSessionParams (SESSION* pSESSION);
extern void InitConnectionParams (CONNECTION* pCONN);
extern void iscsiCr(ILT *pILT);

extern int iscsi_send (ISCSI_TPD *pTPD);

extern UINT8 iscsiBuildNopInHdr(ISCSI_NOPOUT_HDR *pReqHdr, ISCSI_NOPIN_HDR *pRespHdr, UINT8 isResp);

extern UINT8 iscsiCloseConn(CONNECTION *pConn);
extern INT32 iscsiCloseSession(SESSION *pSession);
extern void iscsi_cleanupILT (CONNECTION* pConn, UINT16 lun);
extern bool isTargetNameMatching(UINT8 *targetName, UINT16 targetPortalGroupTag,UINT16 tid);

extern INT32  processImmediateCmdForCmdSn(SESSION *pSession,UINT32 cmdSN);

extern UINT8 hash_insert(hash_node **hashTable,UINT32 key, ILT *rec,UINT32 ttt);
extern UINT8 hash_delete(hash_node **hashTable,UINT32 key,UINT32 ttt);
extern ILT *hash_find(hash_node **hashTable,UINT32 key,UINT32 ttt);
extern hash_node *hash_lookup(hash_node **hashTable,UINT32 key, UINT32 ttt);
extern void iscsi_updateSend (CONNECTION *pConn, ILT* pSecILT, char* hdr, UINT32 hdrLen, char* data, UINT32 dataLen,
                       UINT8, int (*cr)(ILT *), UINT16 lun);
extern int iscsi_crRelILT(ILT* pILT);
extern void getiscsiNameForTarget(UINT8 *pName, UINT32 tid);

extern int iscsi_crTmfWarm(ILT* pILT);
extern int iscsi_crTmfCold(ILT* pILT);
extern int chapGetSecret (UINT16 tid,UINT8 *name,UINT8 *secret1, UINT8* secret2);
extern INT32 iscsiBuildAndSendReject(CONNECTION *pConn, REJECT_REASON_CODE reason, ISCSI_GENERIC_HDR *pRejectedHdr);
extern UINT8 iscsiProcScsiData(CONNECTION *pConn, SGL_DESC *pSgl,ISCSI_DATA_OUT_HDR *pHdr,ILT *pILT, ILT *r2tILT, ISCSI_SCSI_CMD_HDR *pCmd, UINT8 isLastDataOut, UINT8 sglCount);

extern void iscsi_unthreadILT(CONNECTION* pConn, ILT *pILT);
extern void iscsi_ins_conn(CONNECTION *pStart, CONNECTION *pIns);
extern void iscsi_del_conn(CONNECTION *pStart, CONNECTION *pDel);
extern CONNECTION *iscsi_lookup_conn(CONNECTION *pStart, CONNECTION *pLookup);
extern void iscsi_ins_ssn(SESSION *pStart, SESSION *pIns);
extern UINT8 iscsi_del_ssn(UINT32 *head, SESSION *pDel);
extern SESSION *iscsi_lookup_ssn(SESSION *pStart, SESSION *pLookup);
extern void iscsi_SessionCleanup (TAR* pTar);
extern UINT32 getScsiexpDataTxLen(ISCSI_PDU *pPdu);

#endif  /* __ISCSI_PDU_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
