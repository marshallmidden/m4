/* $Id: iscsi_text.c 161368 2013-07-29 14:53:10Z marshall_midden $ */
/**
 ******************************************************************************
 **
 **  @file       iscsi_text.c
 **
 **  @brief      Text param negotiation
 **
 **  To provide text parameter negotiation of iSCSI.
 **
 **  Copyright (c) 2005-2010 Xiotech Corporation.  All rights reserved.
 **
 ******************************************************************************
 **/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "XIO_Std.h"
#include "XIO_Types.h"
#include "iscsi_common.h"
#include "iscsi_pdu.h"
#include "MR_Defs.h"
#include "DEF_iSCSI.h"
#include "XIO_Macros.h"
#include "target.h"
#include "fsl.h"
#include "icl.h"
#include "isp.h"

extern TDX gTDX;
extern TGD * T_tgdindx[MAX_TARGETS];
/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/

#define HEX_CONSTANT                        1
#define DECIMAL_CONSTANT                    2
#define BASE64_CONSTANT                     3
#define NUMERIC                             4
#define NATURAL_NUMBER                      5
#define LARGE_NUMERIC                       6
#define NUMERIC_RANGE                       7
#define TEXT_VALUE                          8
#define TEXT_VALUE_LIST                     9
#define BOOLEAN                             10

#define MAX_INT                             0xFFFFFFFF
#define MAX_INT                             0xFFFFFFFF
#define MIN_NATURAL                         1
#define MAX_NATNUM                          65535
#define NATNUM_LEN                          5
#define NUM_LEN                             20

#define YES                                 "Yes"
#define NO                                  "No"

#define DISCOVERY                           "Discovery"
#define NORMALSESSION                       "Normal"

#define getPortFromTid(tid)  ((T_tgdindx[(tid)] == NULL) ? 0xFF : T_tgdindx[(tid)]->port)

/*
******************************************************************************
** Private structure definitions
******************************************************************************
*/

typedef struct TGTDATA
{
    UINT8  tgtName[256];              /* Target Name */
    UINT8  tgtIP[4];                  /* Target IP Address */
    UINT8  status;
    UINT8  tid;
    UINT16 tgtPort;                   /* Port Id */
    UINT16 tgtPGT;                    /* Target Portal Group Tag */
    UINT8  tgtAlias[32];              /* Target Alias */
} TGTDATA;

/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/
#define ISSTTEND(x) ((x)== 0x0)
#define ISKEYEND(x) ((x)=='=')
#define ISDELIM(x)  ((x)==',')
#define ISSPACE(x) ((x)==' ' || (x)=='\t' || (x)=='\n' || (x)=='\r')
#define ISRANGEDELIM(x) ((x)=='~')
#define ISCHARACTER(x) (((x) >= '0' && (x) <= '9') \
                        || ((x) >= 'A' && (x) <= 'Z') \
                        || ((x) >= 'a' && (x) <= 'z') \
                        || ((x) == '.' ) || ((x) == '-' ) || ((x) == '+' ) \
                        || ((x) == '@' ) || ((x) == '_' ) || ((x) == '/' ) \
                        || ((x) == '(' ) || ((x) == ')' ) || ((x) == ':' ))
#define ISHEX(x) (isdigit(x) || (toupper((x)) >='A' && toupper((x)) <= 'F'))

#define ISBASE64(x) (isdigit((x)) || (toupper((x)) >='A' && toupper((x)) <= 'z') \
                        || (x)=='+' || (x)=='/' || (x)=='=')
#define ISVALIDBOOLEAN(x) ((x)==XIO_TRUE || (x)==XIO_FALSE)

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
void DestroyList (MSGLIST** msghead);
UINT32 stringLength(UINT8* s);
void stringCopy(const UINT8* src, UINT8* dest);

UINT8* stringReverse (UINT8* strVal);
INT32 a2i (UINT8* str);
UINT8* i2a (UINT32 n, UINT8* strVal);
void stringAdd(UINT8* s1, UINT8* s2, UINT32 pos);
UINT8* setBoolean (UINT8 val);
UINT8 getSessinType(UINT8* str);
INT8 FindKeyName(UINT8 *keyname);
UINT8 FindParamVal(UINT8 keyPos, UINT8* keyVal, UINT16 tid);
UINT8* GetParamVal(UINT8 keyPos, UINT16 tid);
UINT8* makeRangeVal(UINT32 lo, UINT32 hi, UINT8* strVal);
UINT32 fitRange(RANGEVALUE *r1, RANGEVALUE *r2);
MSGLIST* ParseTextMsg(UINT8* msg, UINT32 msglen);
UINT32 AddTextMsg(UINT8* msg, UINT32 pos, STT* pSTT);
UINT8* MakeTextMsg(MSGLIST* msghead, UINT8* msg);
void ValidateRange(UINT8* keyVal, RANGEVALUE *range);
UINT8 ValidateData(UINT8* keyVal, UINT32 opt);
UINT8 ValidateRelevance(INT8 keyPos, SESSION* pSESSION, CONNECTION* pCONN);
void UpdateParams(STT* pSTT, UINT8 keyPos, UINT8 valPos, SESSION* pSESSION, CONNECTION* pCONN);
INT8 RFMIN (STT* pSTT, UINT8 keyPos, UINT32 sessVal, SESSION* pSESSION, CONNECTION* pCONN);
INT8 RFMAX (STT* pSTT, UINT8 keyPos, UINT32 sessVal, SESSION* pSESSION, CONNECTION* pCONN);
INT8 RFOR (STT* pSTT, UINT8 keyPos, UINT8 sessVal, SESSION* pSESSION, CONNECTION* pCONN);
INT8 RFAND (STT* pSTT, UINT8 keyPos, UINT8 sessVal, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateHeaderDigest (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateDataDigest (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateMaxConnections (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateTargetName (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateInitiatorName (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateTargetAlias (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateInitiatorAlias (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateTargetAddress (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateTargetPortalGroupTag (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateInitialR2T (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateImmediateData (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateMaxRecvDataSegmentLength (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateMaxBurstLength (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateFirstBurstLength (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateDefaultTime2Wait (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateDefaultTime2Retain (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateMaxOutstandingR2T (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateDataPDUInOrder (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateDataSequenceInOrder (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateErrorRecoveryLevel (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateSessionType (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateAuthMethod (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateIFMarker (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateOFMarker (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateIFMarkInt (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateOFMarkInt (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateChap_A (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateChap_N (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateChap_R (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
INT8 NegotiateChap_C (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
UINT32 NegotiateSendTargets (SESSION *pSsn, STT* pSTT, UINT8* resp, UINT32 pos);
INT32 ProcessTxtMsg(MSGLIST* msghead, UINT8* resp, UINT32* respLen, SESSION* pSESSION, CONNECTION* pCONN);
UINT32 BuildMsg(INT8 keyPos, UINT8* resp, UINT32 pos, UINT16 tid);

/*UINT8 SetParamVal(UINT8 keyPos, UINT8* keyVal, PARAMVALS *paramVal);*/
UINT8 SetParamVal(UINT8 keyPos, UINT8* keyVal, PARAM_TABLE *pParamTable);
bool isValidParamValueRange(INT32 paramIndex, INT32 intVal);
UINT8  EnableParamNegotiation(INT32 paramIndex, PARAM_TABLE *paramTable, UINT8 bitmap, UINT16 );
void AddValueInParamTable(INT32 paramIndex, PARAM_TABLE *paramTable, const UINT8 *val);
UINT8  DisableParamNegotiation(INT32 paramIndex, PARAM_TABLE *paramTable, UINT8 *val );
UINT8  MakeParamNegotiationMandatory(INT32 paramIndex, PARAM_TABLE *paramTable, UINT8 *val );
bool isSupportedNumericParamValue(INT32 paramIndex, INT32 intVal);
void RemoveAllListValuesForIndex(INT32 paramIndex, PARAM_TABLE *paramTable);

INT8 NegotiateChap_I (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN);
UINT8 iscsiGenerateParameters(I_TGD *pParamSrc);
UINT8 iSCSIAddUser(MRCHAPCONFIG userInfo);
UINT8 iSCSIRemoveUser(MRCHAPCONFIG userInfo);

/*
******************************************************************************
** Private prototypes for iSCSI target
******************************************************************************
*/

int iscsiGetAllActiveTargets(UINT16 port, TGTDATA *pTgtData);
static INT32 getHexNumber(UINT8 ch);
INT32 convertTargetNameInTo64bit(UINT8 *string, INT32 size, UINT64 *pTargetName);
void convertIntoReadableData(UINT64 nodeName, UINT8 *readableFormat);
int iscsiGetAddressForTargetName(UINT8 *pTargetName, TGTDATA *pTgtData);

UINT64 iscsiGetNodeNameForTid(UINT16 tid);
UINT8* hex2a (UINT64 n, UINT8* strVal);
void freeMsgNode (MSGLIST* node);
MSGLIST* getNewMsgNode (void);
void printResponse (UINT8* resp, UINT32 pos);
void FreeList (MSGLIST** msghead);
UINT32 ProcessDeclarative(SESSION *pSsn, STT* pSTT, INT8 keyPos, UINT8* resp, UINT32 pos);

/*UINT8 iscsiSetParameterValue(INT32 paramIndex, UINT8 *paramVal, PARAMVALS *configParamVal);*/
UINT8 iscsiSetParameterValue(INT32 paramIndex, UINT8 *paramVal, PARAM_TABLE *pParamTable);
UINT8 iscsiSetListParameterValue(INT32 paramIndex, UINT8 *paramVal, INT32 option, PARAM_TABLE *ptrTable, UINT8 bitmap, UINT16);
void printiscsiParams(PARAM_TABLE *ptrTable);
UINT8 getBoolean (UINT8* str);

bool isNegotiableTwice(INT32 keyPos);
bool isConnectionParam(INT32 keyPos);
bool isSessionParam(INT32 keyPos);

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
#define ISCSI_INVALID_PARAM     0x1
#define ISCSI_INVALID_VALUE     0x2
#define ISCSI_OUT_OF_RANGE      0x3
#define ISCSI_NO_SUPPORT        0x4

static TGTDATA TGTINFO[MAX_TARGETS];

/*
******************************************************************************
** Private variables
******************************************************************************
*/

bool isNegotiableTwice(INT32 keyPos)
{
     switch(keyPos)
     {
       case HEADERDIGESTINDEX:
       case  DATADIGESTINDEX:
       case  AUTHMETHODINDEX:
       case  OFMARKERINDEX:
       case  IFMARKERINDEX:
       case  OFMARKINTINDEX:
       case  IFMARKINTINDEX:
       case MAXCONNECTIONSINDEX:
        case TARGETALIASINDEX:
        case TARGETADDRESSINDEX:
        case TARGETPORTALGROUPTAGINDEX:
        case INITIALR2TINDEX:
        case IMMEDIATEDATAINDEX:
        case MAXBURSTLENGTHINDEX:
        case FIRSTBURSTLENGTHINDEX:
        case DEFAULTTIME2WAITINDEX:
        case DEFAULTTIME2RETAININDEX:
        case MAXOUTSTANDINGR2TINDEX:
        case DATAPDUINORDERINDEX:
        case DATASEQUENCEINORDERINDEX:
        case ERRORRECOVERYLEVELINDEX:
            return false;
       case  MAXRECVDATASEGMENTLENGTHINDEX:
       case SENDTARGETSINDEX:
       case SESSIONTYPEINDEX:
       case INITIATORALIASINDEX:
       case  CHAP_RINDEX:
       case  CHAP_CINDEX:
       case  CHAP_IINDEX:
       case  CHAP_NINDEX:
       case TARGETNAMEINDEX:
       case INITIATORNAMEINDEX:
           return true;
       default:
           return false;
     }
}

bool isConnectionParam(INT32 keyPos)
{
  switch(keyPos)
  {
       case HEADERDIGESTINDEX:
       case DATADIGESTINDEX:
       case AUTHMETHODINDEX:
       case CHAP_RINDEX:
       case CHAP_CINDEX:
       case CHAP_IINDEX:
       case CHAP_NINDEX:
       case OFMARKERINDEX:
       case IFMARKERINDEX:
       case OFMARKINTINDEX:
       case IFMARKINTINDEX:
       case MAXRECVDATASEGMENTLENGTHINDEX:
            return true;
       default:
            return false;
  }
}

bool isSessionParam(INT32 keyPos)
{
    switch(keyPos)
    {
        case MAXCONNECTIONSINDEX:
        case SENDTARGETSINDEX:
        case TARGETNAMEINDEX:
        case INITIATORNAMEINDEX:
        case TARGETALIASINDEX:
        case INITIATORALIASINDEX:
        case TARGETADDRESSINDEX:
        case TARGETPORTALGROUPTAGINDEX:
        case INITIALR2TINDEX:
        case IMMEDIATEDATAINDEX:
        case MAXRECVDATASEGMENTLENGTHINDEX:
        case MAXBURSTLENGTHINDEX:
        case FIRSTBURSTLENGTHINDEX:
        case DEFAULTTIME2WAITINDEX:
        case DEFAULTTIME2RETAININDEX:
        case MAXOUTSTANDINGR2TINDEX:
        case DATAPDUINORDERINDEX:
        case DATASEQUENCEINORDERINDEX:
        case ERRORRECOVERYLEVELINDEX:
        case SESSIONTYPEINDEX:
            return true;
        default:
            return false;
    }
}
/*
** Parameter Names
*/
UINT8 KEYNAMES[][NUM_PARAMS] =
{
    "HeaderDigest",
    "DataDigest",
    "MaxConnections",
    "SendTargets",
    "TargetName",
    "InitiatorName",
    "TargetAlias",
    "InitiatorAlias",
    "TargetAddress",
    "TargetPortalGroupTag",
    "InitialR2T",
    "ImmediateData",
    "MaxRecvDataSegmentLength",
    "MaxBurstLength",
    "FirstBurstLength",
    "DefaultTime2Wait",
    "DefaultTime2Retain",
    "MaxOutstandingR2T",
    "DataPDUInOrder",
    "DataSequenceInOrder",
    "ErrorRecoveryLevel",
    "SessionType",
    "AuthMethod",
    "IFMarker",
    "OFMarker",
    "IFMarkInt",
    "OFMarkInt",
    "CHAP_A",
    "CHAP_N",
    "CHAP_R",
    "CHAP_C",
    "CHAP_I"
};

/*
******************************************************************************
** Public variables
******************************************************************************
*/

extern INT32 tsl_set_mtu(UINT8 port, INT32 mtu_size);

PARAM_TABLE *gTgtParams[MAX_TARGETS];

/*
** Parameter Negotiation Routines
*/
INT8 (*ParamNegotiateTable[])(STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN) =
{
    NegotiateHeaderDigest,
    NegotiateDataDigest,
    NegotiateMaxConnections,
    NULL,
    NegotiateTargetName,
    NegotiateInitiatorName,
    NegotiateTargetAlias,
    NegotiateInitiatorAlias,
    NegotiateTargetAddress,
    NegotiateTargetPortalGroupTag,
    NegotiateInitialR2T,
    NegotiateImmediateData,
    NegotiateMaxRecvDataSegmentLength,
    NegotiateMaxBurstLength,
    NegotiateFirstBurstLength,
    NegotiateDefaultTime2Wait,
    NegotiateDefaultTime2Retain,
    NegotiateMaxOutstandingR2T,
    NegotiateDataPDUInOrder,
    NegotiateDataSequenceInOrder,
    NegotiateErrorRecoveryLevel,
    NegotiateSessionType,
    NegotiateAuthMethod,
    NegotiateIFMarker,
    NegotiateOFMarker,
    NegotiateIFMarkInt,
    NegotiateOFMarkInt,
    NegotiateChap_A,
    NegotiateChap_N,
    NegotiateChap_R,
    NegotiateChap_C,
    NegotiateChap_I
};

MSGLIST* freeMsgList = NULL;

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Move Node to Free List
**
**  @param      Pointer to memory
**
**  @return     none
**
******************************************************************************
**/
void freeMsgNode (MSGLIST* node)
{
    node->decl.numvals = 0;
    node->decl.keyname[0] = '\0';
    if (freeMsgList == NULL)
    {
        freeMsgList = node;
        freeMsgList->next = NULL;
    }
    else
    {
        node->next = freeMsgList;
        freeMsgList = node;
    }
}

/**
******************************************************************************
**
**  @brief      Create New Node
**
**  @param      none
**
**  @return     Pointer to memory
**
******************************************************************************
**/
MSGLIST* getNewMsgNode (void)
{
    MSGLIST* msg;

    if (freeMsgList == NULL)
    {
        /* Allocate new node */
        msg = (MSGLIST *)s_MallocC(sizeof(MSGLIST), __FILE__, __LINE__);
        msg->next = NULL;
        return (msg);
    }
    else
    {
        /* Memory already available, Reuse */
        msg = freeMsgList;
        if (freeMsgList->next == NULL)
        {
            freeMsgList = NULL;
        }
        else
        {
            freeMsgList = freeMsgList->next;
            msg->next = NULL;
        }
    }
    return (msg);
}

/**
******************************************************************************
**
**  @brief      Free all nodes of message list
**
**  @param      pointer to head of list
**
**  @return     none
**
******************************************************************************
**/
void DestroyList (MSGLIST** msghead)
{
    MSGLIST* msg;

    while ((*msghead) != NULL)
    {
        msg = *msghead;
        if (msg->next == NULL)
        {
            *msghead = NULL;
        }
        else
        {
            *msghead = msg->next;
        }
        s_Free(msg, sizeof(MSGLIST), __FILE__, __LINE__);
    }
    *msghead = NULL;
}

/**
******************************************************************************
**
**  @brief      Free all nodes of message list
**
**  @param      pointer to head of list
**
**  @return     none
**
******************************************************************************
**/
void FreeList (MSGLIST** msghead)
{
    MSGLIST* msg;

    while ((*msghead) != NULL)
    {
        msg = *msghead;
        if (msg->next == NULL)
        {
            *msghead = NULL;
        }
        else
        {
            *msghead = msg->next;
        }
        freeMsgNode (msg);
    }
    *msghead = NULL;
}

/**
******************************************************************************
**
**  @brief      Find the length of a given string
**
**  @param      string
**
**  @return     String Length
**
******************************************************************************
**/
UINT32 stringLength(UINT8* s)
{
    UINT32 len=0;
    if (s==NULL) return (0);
    while (XIO_TRUE)
    {
        if (*s == '\0')
            return (len);
        len++;
    s++;
    }
}

/**
******************************************************************************
**
**  @brief      Copy strings
**
**  @param      Source, destinatiom strings
**
**  @return     none
**
******************************************************************************
**/
void stringCopy(const UINT8* src, UINT8* dest)
{
    UINT32 i = 0;
    if (src == NULL || dest == NULL)
    {
        return;
    }

    while (XIO_TRUE)
    {
        dest[i] = src[i];
        if (src[i] == '\0')
        {
            break;
        }
        i++;
    }
}

/**
******************************************************************************
**
**  @brief      Compare Strings
**
**  @param      string1, string2
**
**  @return     0 if equal, 1 if string1 greater, -1 if string2 greater
**
******************************************************************************
**/
INT32 stringCompare(UINT8* s1, UINT8* s2)
{
    if (s1==NULL)
    {
        if (s2==NULL)
            return(0);
        return(-1);
    }
    else if (s2==NULL)
    {
        return(1);
    }

    while (XIO_TRUE)
    {
        if (*s1 == '\0')
        {
            if (*s2 == '\0')
                return (0);
            return (-1);
        }
        else if (*s2 == '\0')
        {
            return (1);
        }
        else if (*s1 == *s2)
        {
            s1++;
            s2++;
        }
        else if (*s1 > *s2)
        {
            return(1);
        }
        else
        {
            return(-1);
        }
    }
}

/**
******************************************************************************
**
**  @brief      Reverse string
**
**  @param      string
**
**  @return     string
**
******************************************************************************
**/
UINT8* stringReverse (UINT8* strVal)
{
    UINT32 i;
    UINT32 l = stringLength(strVal);
    UINT8 c;

    for (i = 0; i <= l/2; i++)
    {
        c = strVal[i];
        strVal[i] = strVal[l-1-i];
        strVal[l-1-i] = c;
    }

    return (strVal);
}

/**
******************************************************************************
**
**  @brief      Convert string to integer
**
**  @param      string
**
**  @return     integer
**
******************************************************************************
**/
INT32 a2i (UINT8* str)
{
    INT32 numval = 0;
    UINT8 digval = 0;
    INT8 l = strlen((char *)str);
    INT8 i;

    if(l==0)
    {
        return(0);
    }
    if(l>2)
    {
        /*
        ** Hex
        */
        if(str[0]=='0' && (str[1]=='x' || str[1]=='X'))
        {
            for (i = 2; i < l; i++)
            {
                if (str[i] >= '0' && str[i] <= '9')
                {
                    digval = str[i] - '0';
                }
                else if (tolower(str[i]) >= 'a' && tolower(str[i]) <= 'f')
                {
                    digval = str[i] - 'a' + 10;
                }
                else
                {
                    return(-1);
                }
                numval = (numval*16) + digval;
            }
            return (numval);
        }
    }

    for (i = 0; i < l; i++)
    {
        if (str[i] >= '0' && str[i] <= '9')
        {
            digval = str[i] - '0';
        }
        else
        {
            return(-1);
        }
        numval = (numval*10) + digval;
    }

    return(numval);
}

/**
******************************************************************************
**
**  @brief      Convert hex to string
**
**  @param      Numeric
**
**  @return     string
**
******************************************************************************
**/
UINT8* hex2a (UINT64 n, UINT8* strVal)
{
    UINT8 i = 0;
    UINT8 l;
    UINT8 c;
    char hex[] = "0123456789abcdef";

    while (n)
    {
        strVal[i] = hex[n&0xF];
        n>>=4;
        i++;
    }
    strVal[i] = '\0';
    l = i;

    for (i = 0; i <= (l-1)/2; i++)
    {
        c = strVal[i];
        strVal[i] = strVal[l-1-i];
        strVal[l-1-i] = c;
    }
    return (strVal);
}
/**
******************************************************************************
**
**  @brief      Convert number to string
**
**  @param      Numeric
**
**  @return     string
**
******************************************************************************
**/
UINT8* i2a (UINT32 n, UINT8* strVal)
{
    UINT8 i = 0;
    UINT8 l;
    UINT8 c;
    if( n == 0)
    {
      strVal[i++] = '0';
      strVal[i]=0;
      return strVal;
    }
    while (n)
    {
        strVal[i] = (n%10) + '0';
        n/=10;
        i++;
    }
    strVal[i] = '\0';
    l = i;

    for (i = 0; i <= (l-1)/2; i++)
    {
        c = strVal[i];
        strVal[i] = strVal[l-1-i];
        strVal[l-1-i] = c;
    }
    return (strVal);
}

/**
******************************************************************************
**
**  @brief      Append a string to another
**              expansion operation or a create.
**
**  @param      strings, position from where new string should be appended
**
**  @return     none
**
******************************************************************************
**/
void stringAdd(UINT8* s1, UINT8* s2, UINT32 pos)
{
    UINT32 i;
    UINT32 l=strlen((char *)s2);
    for (i=pos; i<pos+l; i++)
    {
        s1[i] = s2[i-pos];
    }
    s1[i] = '\0';
}

/**
******************************************************************************
**
**  @brief      Decode boolean
**
**  @param      string ("Yes"/"No")
**
**  @return     Boolean value 0/1
**
******************************************************************************
**/
UINT8 getBoolean (UINT8* str)
{
    if (stringCompare(str, (UINT8*)"Yes") == 0)
    {
        return (XIO_TRUE);
    }
    return (XIO_FALSE);
}

/**
******************************************************************************
**
**  @brief      Encode boolean
**
**  @param      boolean (0/1)
**
**  @return     string("Yes"/"No")
**
******************************************************************************
**/
UINT8* setBoolean (UINT8 val)
{
    if (val == XIO_TRUE)
    {
        return ((UINT8*)"Yes");
    }
    else
    {
        return ((UINT8*)"No");
    }
}

/**
******************************************************************************
**
**  @brief      Decode Session Type
**
**  @param      Session type
**
**  @return     0 if normal, 1 if discovery session
**
******************************************************************************
**/
UINT8 getSessinType(UINT8* str)
{
    if (stringCompare(str, (UINT8*)DISCOVERY) == 0)
    {
        return (DISCOVERY_SESSION);
    }
    else
    {
        return (NORMAL_SESSION);
    }
}

/**
******************************************************************************
**
**  @brief      Check if a given keyname is valid
**
**  @param      keyname
**
**  @return     1 if found, -1 otherwise
**
******************************************************************************
**/
INT8 FindKeyName(UINT8 *keyname)
{
    INT32 i;

    for (i=0; i<NUM_PARAMS; i++)
    {
        if (stringCompare(keyname, KEYNAMES[i])==0)
            return (i);
    }
    return(-1);
}

/**
******************************************************************************
**
**  @brief      Compare a keyvalue against default parameter value
**
**  @param      keyname index, key value
**
**  @return     XIO_TRUE if found, XIO_FALSE otherwise
**
******************************************************************************
**/
UINT8 FindParamVal(UINT8 keyPos, UINT8* keyVal, UINT16 tid)
{
    INT8  retVal = XIO_FALSE;
    UINT8 i;
    if(gTgtParams[tid] != NULL)
    {
        for (i=0; i<gTgtParams[tid]->index; i++)
        {
            if (gTgtParams[tid]->ptrParams[i].keyName == keyPos &&
               stringCompare(keyVal, gTgtParams[tid]->ptrParams[i].keyVal) == 0)
            {
                retVal = XIO_TRUE;
                break;
            }
        }
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Get default parameter value
**
**  @param      keyname index
**
**  @return     Default parameter value
**
******************************************************************************
**/
UINT8* GetParamVal(UINT8 keyPos, UINT16 tid)
{
    UINT8 i;
    if(gTgtParams[tid] != NULL)
    {
        for (i=0; i<gTgtParams[tid]->index; i++)
        {
            if (gTgtParams[tid]->ptrParams[i].keyName == keyPos)
            {
                return (gTgtParams[tid]->ptrParams[i].keyVal);
            }
        }
    }
    return (NULL);
}

/**
******************************************************************************
**
**  @brief      Convert a range value to string
**
**  @param      lo value, hi value, source string
**
**  @return     string
**
******************************************************************************
**/
UINT8* makeRangeVal(UINT32 lo, UINT32 hi, UINT8* strVal)
{
    UINT32 pos;
    UINT8 str[20];

    stringAdd(strVal, i2a(lo, str), 0);
    pos = stringLength(strVal);
    stringAdd(strVal, (UINT8*)"~", pos);
    pos++;
    stringAdd(strVal, i2a(hi, str), pos);
    return (strVal);
}

/**
******************************************************************************
**
**  @brief      Compare two range values and check if a value fits in both
**
**  @param      range1, range2
**
**  @return     Matching value, MAX_INT if no overlap
**
******************************************************************************
**/
UINT32 fitRange(RANGEVALUE *r1, RANGEVALUE *r2)
{
    if (r1->lo <= r2->hi && r1->lo >= r2->lo)
    {
        return (r1->lo);
    }
    if (r1->hi <= r2->hi && r1->hi >= r2->lo)
    {
        return (r1->hi);
    }
    if (r2->lo >= r1->lo && r2->lo <= r1->hi)
    {
        return (r2->lo);
    }
    if (r2->hi >= r1->lo && r2->hi <= r1->hi)
    {
        return (r2->hi);
    }
    return (MAX_INT);
}

/**
******************************************************************************
**
**  @brief      Parse text message
**
**              Text message is a sequence of key=value pairs. Every key=value
**              pair, including the last or only pair, is followed by '0x00'.
**              The 'value' can be a list of text-values separated by comma.
**
**              The text is split into key-val pairs and the head of list is
**              returned.
**
**  @param      Text message string, length of the string
**
**  @return     Head of list of key-value pairs
**
******************************************************************************
**/
MSGLIST* ParseTextMsg(UINT8* msg, UINT32 msglen)
{
    UINT32 st, end, i, j, k, l, m;
    UINT32 valcnt;
    UINT8 state;
    UINT8 keyname[MAX_KEYNAME_LEN + 1];
    MSGLIST* msghead = NULL;
    MSGLIST* curmsg=NULL;
    MSGLIST* newmsg=NULL;

    i=0;
    st=0;
    state=0;
    while(i<msglen)
    {
        if(ISSTTEND(msg[i]) || i>msglen-1)
        {
            end = i;
            /* Now we have a statement, parse this */
            j=st;
            while (ISSPACE(msg[j])  && j<= end )     j++;

            for (st=j; !ISKEYEND(msg[j]) && j<=end; j++);

            if (ISKEYEND(msg[j]))
            {
                for (k=j-1;ISSPACE(msg[k]);k--);
                /* st to k is the keyname */
                for (l=0, m=st; m<=k; m++, l++)
                {
                    keyname[l] = msg[m];
                }
                keyname[l] = '\0';
                /* j+1 to end are the values */
                j++;
                while (ISSPACE(msg[j]) && j<= end)     j++;
                if (j>end)
                {
                    /* invalid message with no KEYVALUE */
                }
                else
                {
                    st=j;

                    /* go through the msg */
                    for (valcnt = 0; j <= end; j++)
                    {
                        if (ISDELIM(msg[j]) || j == end)
                        {
                            valcnt++;
                        }
                    }

                    if (valcnt == 0)
                    {
                        /* invalid message with no KEYVALUE */
                    }
                    else
                    {
                        newmsg = getNewMsgNode();
                        newmsg->next = NULL;

                        /* add the new node to the list */
                        if (msghead == NULL)
                        {
                            msghead = newmsg;
                            curmsg = newmsg;
                        }
                        else
                        {
                            curmsg->next = newmsg;
                            curmsg = newmsg;
                        }

                        newmsg->decl.numvals = valcnt;
                        stringCopy(keyname, newmsg->decl.keyname);
                        j=st;
                        l=0;
                        while (j<=end)
                        {
                            if (ISDELIM(msg[j]) || j == end)
                            {
                                for (k=j-1;ISSPACE(msg[k]);k--);
                                /* st to k is value */
                                for (m=0; st<=k; st++, m++)
                                {
                                    newmsg->decl.data[l].strval[m] = msg[st];
                                }
                                newmsg->decl.data[l].strval[m] = '\0';
                                j++;
                                l++;
                                while (ISSPACE(msg[j]) && j<= end)     j++;
                                st=j;
                            }
                            j++;
                        }
                    }
                }

            }
            else
            {
                /* message with KEYNAME but no RHS */
                for (k=j-1;ISSPACE(msg[k]);k--);
                /* st to k is the keyname */
                for (l=0, m=st; m<=k; m++, l++)
                {
                    keyname[l] = msg[m];
                }
                keyname[l] = '\0';

                newmsg = getNewMsgNode();
                newmsg->next = NULL;

                /* add the new node to the list */
                if (msghead == NULL)
                {
                    msghead = newmsg;
                    curmsg = newmsg;
                }
                else
                {
                    curmsg->next = newmsg;
                    curmsg = newmsg;
                }

                newmsg->decl.numvals = 0;
                stringCopy(keyname, newmsg->decl.keyname);
            }
            while (ISSTTEND(msg[i]) && i<msglen) i++;
            st = i;
        }
        i++;
    }
    return(msghead);
}

/*
    CONVERT LIST TO TEXT
*/

/**
******************************************************************************
**
**  @brief      Add keyvalues to string
**
**  @param      string, position where value should be appended,
**              key-value pointer
**
**  @return     New end of string
**
******************************************************************************
**/
UINT32 AddTextMsg(UINT8* msg, UINT32 pos, STT* pSTT)
{
    UINT32 i, j, l;

    l = stringLength(pSTT->keyname);
    for (i = 0; i < l; i++, pos++)
    {
        msg[pos] = pSTT->keyname[i];
    }
    /* append '=' */
    if (pSTT->numvals > 0)
    {
        msg[pos] = '=';
        pos++;
    }
    /* append keyvalues */
    for (i = 0; i < pSTT->numvals; i++)
    {
        /* append ith keyvalue */
        l = stringLength(pSTT->data[i].strval);
        for (j = 0; j < l; j++, pos++)
        {
            msg[pos] = pSTT->data[i].strval[j];
        }
        /* append delimiter */
        if (i==pSTT->numvals - 1)
        {
            msg[pos] = '\0';
        }
        else
        {
            msg[pos] = ',';
        }
        pos++;
    }
    return (pos);
}

/**
******************************************************************************
**
**  @brief      Convert key-val pairs to string
**
**  @param      head of list of key-val pairs, string
**
**  @return     string
**
******************************************************************************
**/
UINT8* MakeTextMsg(MSGLIST* msghead, UINT8* msg)
{
    /* UINT32 l; */
    UINT32 i, pos;
    MSGLIST *pMSG;
    STT* pSTT;

    pos = 0;

    for (pMSG = msghead; pMSG != NULL; pMSG = pMSG->next)
    {
        if (pMSG == NULL)
            break;
        pSTT = &(pMSG->decl);
        pos = AddTextMsg(msg, pos, pSTT);
    }

    pos++;

    /* print the string */

    for (i = 0; i < pos; i++)
    {
        if (msg[i] == '\0')
            printf ("\n");
        else
        {
            printf ("%c", msg[i]);
        }
    }

    return (msg);
}

/**
******************************************************************************
**
**  @brief      Validate numeric range value
**
**  @param      range value
**
**  @return     range value. if range is invalid lo and hi values have MAX_INT
**
******************************************************************************
**/
void ValidateRange(UINT8* keyVal, RANGEVALUE *range)
{
    UINT32 i, j, k;
    UINT32 l = stringLength(keyVal);
    UINT8  str[MAX_KEYVAL_LEN+1], str1[MAX_KEYVAL_LEN+1];

    range->lo=0;
    range->hi=0;

    for (i=0; !ISRANGEDELIM(keyVal[i]) && i<l; i++);
    if (i >= l)
    {
        /* No Range Delimiter found */
        range->lo = MAX_INT;
        range->hi = MAX_INT;
    }
    else
    {
        k = i;
        for (i=k-1; ISSPACE(keyVal[i]); i--);
        for (j=0; ISSPACE(keyVal[j]); j++);
        /* Validate First Number in the Range */
        if (i-j > NATNUM_LEN)
        {
            range->lo = MAX_INT;
            range->hi = MAX_INT;
        }
        else
        {
            for (j=0; j<=i; j++)
            {
                str[j] = keyVal[j];
            }
            str[j] = '\0';
            if (!ValidateData (str, NATURAL_NUMBER))
            {
                range->lo = MAX_INT;
                range->hi = MAX_INT;
            }
            else
            {
                /* Validate Second Number in the Range */
                i = k + 1;
                for (i=k+1; ISSPACE(keyVal[i]); i++);
                for (; ISSPACE(keyVal[l-1]); l--);
                if (l-i > NATNUM_LEN)
                {
                    range->lo = MAX_INT;
                    range->hi = MAX_INT;
                }
                else
                {
                    for (j=0; i<l; i++, j++)
                    {
                        str1[j] = keyVal[i];
                    }
                    str1[j] = '\0';
                    if (!ValidateData (str1, NATURAL_NUMBER))
                    {
                        range->lo = MAX_INT;
                        range->hi = MAX_INT;
                    }
                    else
                    {
                        range->lo = a2i(str);
                        range->hi = a2i(str1);
                        /* Second number should not be less than the first */
                        if (range->lo > range->hi)
                        {
                            range->lo = MAX_INT;
                            range->hi = MAX_INT;
                        }
                    }
                }
            }
        }
    }
}

/**
******************************************************************************
**
**  @brief      Validate data type of given data
**
**              Data types supported are:
**
**              HEX_CONSTANT
**              DECIMAL_CONSTANT
**              BASE64_CONSTANT
**              NUMERIC
**              NATURAL_NUMBER
**              LARGE_NUMERIC
**              NUMERIC_RANGE
**              TEXT_VALUE
**              BOOLEAN
**
**  @param      data, data type
**
**  @return     XIO_TRUE if valid, XIO_FALSE if invalid
**
******************************************************************************
**/
UINT8 ValidateData(UINT8* keyVal, UINT32 opt)
{
    UINT8 retVal = XIO_TRUE;
    UINT32 l = stringLength(keyVal);
    UINT32 i, j, k;
    INT64  val;
    UINT8  str[MAX_KEYVAL_LEN+1], str1[MAX_KEYVAL_LEN+1];

    switch (opt)
    {
        case HEX_CONSTANT:
            if (l > MAX_KEYVAL_LEN || l < 3)
            {
                retVal = XIO_FALSE;
            }
            else if (keyVal[0] != '0' || toupper(keyVal[1]) !='X')
            {
                retVal = XIO_FALSE;
            }
            else
            {
                for (i=2; i<l; i++)
                {
                    if (!ISHEX(keyVal[i]))
                    {
                        retVal = XIO_FALSE;
                    }
                }
            }
            break;
        case BASE64_CONSTANT:

            if (l > MAX_KEYVAL_LEN || l < 3)
            {
                retVal = XIO_FALSE;
            }
            else if (keyVal[0] != '0' || toupper(keyVal[1]) !='B')
            {
                retVal = XIO_FALSE;
            }
            else
            {
                for (i=2; i<l; i++)
                {
                    if (!ISBASE64(keyVal[i]))
                    {
                        retVal = XIO_FALSE;
                    }
                }
            }
            break;
        case NATURAL_NUMBER:
            /* Number from 1 to 65535 */
            if (l > NATNUM_LEN)
            {
                retVal = XIO_FALSE;
            }
            else
            {
                for (i=0; i<l; i++)
                {
                    if (!isdigit(keyVal[i]))
                    {
                        retVal = XIO_FALSE;
                        break;
                    }
                }
                if (retVal == XIO_TRUE)
                {
                    val = a2i(keyVal);
                    if (val > MAX_NATNUM || val < 1)
                    {
                        retVal = XIO_FALSE;
                    }
                }
            }
            break;
        case DECIMAL_CONSTANT:
        case NUMERIC:
            if (l > NUM_LEN || l < 1)
            {
                retVal = XIO_FALSE;
            }
            else
            {
                for (i=0; i<l; i++)
                {
                    if (!isdigit(keyVal[i]))
                    {
                        retVal = XIO_FALSE;
                        break;
                    }
                }
                if (retVal == XIO_TRUE)
                {
                    val = a2i(keyVal);
                    if (val < 0)
                    {
                        retVal = XIO_FALSE;
                    }
                }
            }
            break;
        case LARGE_NUMERIC:
            if (l > MAX_KEYVAL_LEN || l < NUM_LEN)
            {
                retVal = XIO_FALSE;
            }
            else
            {
                for (i=0; i<l; i++)
                {
                    if (!isdigit(keyVal[i]))
                    {
                        retVal = XIO_FALSE;
                        break;
                    }
                }
            }
            break;
        case NUMERIC_RANGE:
            for (i=0; !ISRANGEDELIM(keyVal[i]) && i<l; i++);
            if (i >= l)
            {
                /* No Range Delimiter found */
                retVal = XIO_FALSE;
            }
            else
            {
                k = i;
                for (i=k-1; ISSPACE(keyVal[i]); i--);
                /* Validate First Number in the Range */
                for (j=0; j<=i; j++)
                {
                    str[j] = keyVal[j];
                }
                str[j] = '\0';
                if (!ValidateData (str, NUMERIC))
                {
                    retVal = XIO_FALSE;
                }
                else
                {
                    /* Validate Second Number in the Range */
                    i = k + 1;
                    for (i=k+1; ISSPACE(keyVal[i]); i++);
                    for (j=0; i<l; i++, j++)
                    {
                        str1[j] = keyVal[i];
                    }
                    str1[j] = '\0';
                    if (!ValidateData (str, NUMERIC))
                    {
                        retVal = XIO_FALSE;
                    }
                    else
                    {
                        /* Second number should not be less than the first */
                        if (atol((char *)str)>atol((char *)str1))
                        {
                            retVal = XIO_FALSE;
                        }
                    }
                }
            }
            break;
        case TEXT_VALUE:
            if (l > MAX_KEYVAL_LEN || l < 1)
            {
                retVal = XIO_FALSE;
            }
            else if (!isupper(keyVal[0]))
            {
                retVal = XIO_FALSE;
            }
            else
            {
                for (i=0; i<l; i++)
                {
                    if (!ISCHARACTER(keyVal[i]))
                    {
                        retVal = XIO_FALSE;
                        break;
                    }
                }
            }
            break;
        case BOOLEAN:
            if (strcmp((char *)keyVal, YES) != 0 && strcmp((char *)keyVal, NO) != 0)
            {
                retVal = XIO_FALSE;
            }
            break;
        default:
            break;
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Check if given parameter can be negotiated in the current
**              state of session/connection
**
**  @param      keyname index, SESSION pointer, CONNECTION pointer
**
**  @return
**              SENDIRRELEVANT if parameter is Irrelevant
**              SENDREJECT     if value is to be Rejected
**              SENDNEGOTIATE  if value is to be Negotiated
**              SENDOK         if value is to be Accepted without negotiating
**
******************************************************************************
**/
UINT8 ValidateRelevance(INT8 keyPos, SESSION* pSESSION, CONNECTION* pCONN)
{
    UINT8 retVal = SENDOK;
    switch (keyPos)
    {
        case HEADERDIGESTINDEX:
        case DATADIGESTINDEX:
            if (pCONN == NULL)
            {
                return(SENDNEGOTIATE);
            }
            if (pCONN->state == CONNST_LOGGED_IN)
            {
                return(SENDREJECT);
            }
            else
            {
                return(SENDNEGOTIATE);
            }
//            break;
        case MAXCONNECTIONSINDEX:
            if (pSESSION == NULL)
            {
                return(SENDNEGOTIATE);
            }

            if (pSESSION->params.sessionType == DISCOVERY_SESSION)
            {
                return(SENDIRRELEVANT);
            } else
            {
                return(SENDNEGOTIATE);
            }
//            break;

        case SENDTARGETSINDEX:
        case TARGETNAMEINDEX:
        case INITIATORNAMEINDEX:
        case TARGETALIASINDEX:
        case INITIATORALIASINDEX:
        case TARGETADDRESSINDEX:
        case TARGETPORTALGROUPTAGINDEX:
            break;

        case INITIALR2TINDEX:
            if (pSESSION == NULL)
            {
                return(SENDNEGOTIATE);
            }

            if (pSESSION->params.sessionType == DISCOVERY_SESSION)
            {
                return(SENDIRRELEVANT);
            }else
            {
                return(SENDNEGOTIATE);
            }
//            break;
        case IMMEDIATEDATAINDEX:
            if (pSESSION == NULL)
            {
                return(SENDNEGOTIATE);
            }else if (pSESSION->params.sessionType == DISCOVERY_SESSION)
            {
                return(SENDIRRELEVANT);
            }
            else
            {
                return(SENDNEGOTIATE);
            }
//            break;
        case MAXRECVDATASEGMENTLENGTHINDEX:
            break;
        case MAXBURSTLENGTHINDEX:
            if (pSESSION == NULL)
            {
                return(SENDNEGOTIATE);
            }else if (pSESSION->params.sessionType == DISCOVERY_SESSION)
            {
                return(SENDIRRELEVANT);
            }
            else
            {
                return(SENDNEGOTIATE);
            }
//            break;
        case FIRSTBURSTLENGTHINDEX:
            if (pSESSION == NULL)
            {
                return(SENDNEGOTIATE);
            } else if (pSESSION->params.sessionType == DISCOVERY_SESSION)
            {
                return(SENDIRRELEVANT);
            }
            /*
            ** There is a possibility in which initiator had requested initialR2T = NO &
            ** ImmediateData = Yes, but we are responding reversely so let us not send Irrelevant
            */
            /*
            else if (pSESSION->params.initialR2T == XIO_TRUE &&
                        pSESSION->params.immediateData == XIO_FALSE)
            {
                return(SENDIRRELEVANT);
            }
            */
            else
            {
                return(SENDNEGOTIATE);
            }
//            break;
        case DEFAULTTIME2WAITINDEX:
//            if (pSESSION == NULL)
//            {
                return(SENDNEGOTIATE);
//            } else
//            {
//                retVal = SENDNEGOTIATE;
//            }
//            break;
        case DEFAULTTIME2RETAININDEX:
//            if (pSESSION == NULL)
//            {
                return(SENDNEGOTIATE);
//            }else
//            {
//                retVal = SENDNEGOTIATE;
//            }
//            break;
        case MAXOUTSTANDINGR2TINDEX:
//            if (pSESSION == NULL)
//            {
                return(SENDNEGOTIATE);
//            }else
//            {
//                return(SENDNEGOTIATE);
//            }
//            break;
        case DATAPDUINORDERINDEX:
//            if (pSESSION == NULL)
//            {
                return(SENDNEGOTIATE);
//            }else
//            {
//                return(SENDNEGOTIATE);
//            }
//            break;
        case DATASEQUENCEINORDERINDEX:
//            if (pSESSION == NULL)
//            {
                return(SENDNEGOTIATE);
//            }else
//            {
//                return(SENDNEGOTIATE);
//            }
//            break;
        case ERRORRECOVERYLEVELINDEX:
//            if (pSESSION == NULL)
//            {
                return(SENDNEGOTIATE);
//            }else
//            {
//                return(SENDNEGOTIATE);
//            }
//            break;
        case SESSIONTYPEINDEX:
            break;
        case AUTHMETHODINDEX:
            if (pCONN == NULL)
            {
                return(SENDNEGOTIATE);
            }

            if (pCONN->state != CONNST_XPT_UP)
            {
                return(SENDREJECT);
            }
            else
            {
                return(SENDNEGOTIATE);
            }
//            break;
        case IFMARKERINDEX:
//            if (pCONN == NULL)
//            {
                return(SENDNEGOTIATE);
//            }
//            return(SENDNEGOTIATE);
//            break;
        case OFMARKERINDEX:
//            if (pCONN == NULL)
//            {
                return(SENDNEGOTIATE);
//            }
//            return(SENDNEGOTIATE);
//            break;
        case IFMARKINTINDEX:
            if (pCONN == NULL)
            {
                return(SENDNEGOTIATE);
            } else if (pCONN->params.ifMarker == XIO_FALSE)
            {
                return(SENDIRRELEVANT);
            }
            else
            {
                return(SENDNEGOTIATE);
            }
//            break;
        case OFMARKINTINDEX:
            if (pCONN == NULL)
            {
                return(SENDNEGOTIATE);
            } else if (pCONN->params.ofMarker == XIO_FALSE)
            {
                return(SENDIRRELEVANT);
            }
            else
            {
                return(SENDNEGOTIATE);
            }
//            break;
        case CHAP_AINDEX:
            if (pCONN == NULL)
            {
                return(SENDNEGOTIATE);
            }
            else if (pCONN->isChap == XIO_FALSE)
            {
                return(SENDIRRELEVANT);
            }
            else
            {
                return(SENDNEGOTIATE);
            }
//            break;
        case CHAP_NINDEX:
            if (pCONN == NULL)
            {
                return(SENDNEGOTIATE);
            }
            else if (pCONN->isChap == XIO_FALSE)
            {
                return(SENDIRRELEVANT);
            }
            else
            {
                return(SENDNEGOTIATE);
            }
//            break;
        case CHAP_RINDEX:
            if (pCONN == NULL)
            {
                return(SENDNEGOTIATE);
            }
            else if (pCONN->isChap == XIO_FALSE)
            {
                return(SENDIRRELEVANT);
            }
            else
            {
                return(SENDNEGOTIATE);
            }
//            break;
         case CHAP_CINDEX:
            if (pCONN == NULL)
            {
                return(SENDNEGOTIATE);
            }
            else if (pCONN->isChap == XIO_FALSE)
            {
                return(SENDIRRELEVANT);
            }
            else
            {
                return(SENDNEGOTIATE);
            }
//            break;
          case CHAP_IINDEX:
            if (pCONN == NULL)
            {
                return(SENDNEGOTIATE);
            }
            else if (pCONN->isChap == XIO_FALSE)
            {
                return(SENDIRRELEVANT);
            }
            else
            {
                return(SENDNEGOTIATE);
            }
//            break;

        default:
            break;
    }

    return(retVal);
}

/**
******************************************************************************
**
**  @brief      Update Connection/Session Parameters
**
**  @param      key-val pair
**              keyname index
**              index to key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return     none
**
******************************************************************************
**/
void UpdateParams (STT* pSTT, UINT8 keyPos, UINT8 valPos, SESSION* pSESSION, CONNECTION* pCONN)
{

    if (pCONN == NULL || pSESSION == NULL)
    {
        return;
    }

    switch (keyPos)
    {
        case HEADERDIGESTINDEX:
            stringCopy(pSTT->data[valPos].strval, pCONN->params.headerDigest.strval);
            break;
        case DATADIGESTINDEX:
            stringCopy(pSTT->data[valPos].strval, pCONN->params.dataDigest.strval);
            break;
        case MAXCONNECTIONSINDEX:
            pSESSION->params.maxConnections = a2i(pSTT->data[valPos].strval);
            break;
        case SENDTARGETSINDEX:
            /*pSESSION*/
            break;
        case TARGETNAMEINDEX:
            stringCopy(pSTT->data[valPos].strval, pSESSION->params.targetName);
            break;
        case INITIATORNAMEINDEX:
            stringCopy(pSTT->data[valPos].strval, pSESSION->params.initiatorName);
            break;
        case TARGETALIASINDEX:
            stringCopy(pSTT->data[valPos].strval, pSESSION->params.targetAlias);
            break;
        case INITIATORALIASINDEX:
            stringCopy(pSTT->data[valPos].strval, pSESSION->params.initiatorAlias);
            break;
        case TARGETADDRESSINDEX:
            stringCopy(pSTT->data[valPos].strval, pSESSION->params.targetAddress);
            break;
        case TARGETPORTALGROUPTAGINDEX:
            pSESSION->params.targetPortalGroupTag = a2i(pSTT->data[valPos].strval);
            break;
        case INITIALR2TINDEX:
            pSESSION->params.initialR2T = getBoolean(pSTT->data[valPos].strval);
            break;
        case IMMEDIATEDATAINDEX:
            pSESSION->params.immediateData = getBoolean(pSTT->data[valPos].strval);
            break;
        case MAXRECVDATASEGMENTLENGTHINDEX:
            pCONN->params.maxSendDataSegmentLength = a2i(pSTT->data[valPos].strval);
            break;
        case MAXBURSTLENGTHINDEX:
            pSESSION->params.maxBurstLength = a2i(pSTT->data[valPos].strval);
            break;
        case FIRSTBURSTLENGTHINDEX:
            pSESSION->params.firstBurstLength = a2i(pSTT->data[valPos].strval);
            break;
        case DEFAULTTIME2WAITINDEX:
            pSESSION->params.defaultTime2Wait = a2i(pSTT->data[valPos].strval);
            break;
        case DEFAULTTIME2RETAININDEX:
            pSESSION->params.defaultTime2Retain = a2i(pSTT->data[valPos].strval);
            break;
        case MAXOUTSTANDINGR2TINDEX:
            pSESSION->params.maxOutstandingR2T = a2i(pSTT->data[valPos].strval);
            break;
        case DATAPDUINORDERINDEX:
            pSESSION->params.dataPDUInOrder = getBoolean(pSTT->data[valPos].strval);
            break;
        case DATASEQUENCEINORDERINDEX:
            pSESSION->params.dataSequenceInOrder = getBoolean(pSTT->data[valPos].strval);
            break;
        case ERRORRECOVERYLEVELINDEX:
            pSESSION->params.errorRecoveryLevel = a2i(pSTT->data[valPos].strval);
            break;
        case SESSIONTYPEINDEX:
            pSESSION->params.sessionType = getSessinType(pSTT->data[valPos].strval);
            break;
        case AUTHMETHODINDEX:
            stringCopy(pSTT->data[valPos].strval, pCONN->params.authMethod.strval);
            break;
        case IFMARKERINDEX:
            pCONN->params.ifMarker = getBoolean(pSTT->data[valPos].strval);
            break;
        case OFMARKERINDEX:
            pCONN->params.ofMarker = getBoolean(pSTT->data[valPos].strval);
            break;
        case IFMARKINTINDEX:
            /* pCONN->params.ifMarkInt = a2i(pSTT->data[valPos].strval); */
            ValidateRange(pSTT->data[valPos].strval, &(pCONN->params.ifMarkInt));
            break;
        case OFMARKINTINDEX:
            /* pCONN->params.ofMarkInt = a2i(pSTT->data[valPos].strval); */
            ValidateRange(pSTT->data[valPos].strval, &(pCONN->params.ofMarkInt));
            break;
        case CHAP_AINDEX:
            pCONN->params.chap_A = a2i(pSTT->data[valPos].strval);
            break;
/*
** Assume that CHAP name and response are null-terminating strings
*/
        case CHAP_NINDEX:
            stringCopy(pSTT->data[valPos].strval, pCONN->cc->name);
            pCONN->cc->name_len = stringLength(pCONN->cc->name);
            break;
        case CHAP_RINDEX:
            stringCopy(pSTT->data[valPos].strval, pCONN->cc->resp_recvd_encoded);
            pCONN->cc->resp_recvd_len = stringLength(pCONN->cc->resp_recvd_encoded);
            break;
        case CHAP_CINDEX:
             /*
             ** Decode the challenge here, Encoded challenge could be of huge length
             ** and we don't want another copy
             */
            decode_string(pSTT->data[valPos].strval, strlen((char *)pSTT->data[valPos].strval),
                        pCONN->cc->chal_recvd_decoded, &(pCONN->cc->chal_recvd_len));
            break;
         case CHAP_IINDEX:
             /*
             ** CHAP_I should go in cc->id_recvd   id is UINT8
             **
             */
             pCONN->cc->id_recvd = a2i(pSTT->data[valPos].strval);
        default:
            break;
    }
}


/**
******************************************************************************
**
**  @brief      Generic function to compute minimum among proposed,
**              session/connection and default values of a parameter
**
**  @param
**              key-val pair
**              keyname index
**              session/connection parameter value
**              SESSION pointer
**              CONNECTION pointer
**
**  @return
**              Minimum of values from initiator and target.
**
******************************************************************************
**/
INT8 RFMIN (STT* pSTT, UINT8 keyPos, UINT32 sessVal, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNEGOTIATE;
    UINT32 myMax, urMax;
    INT32  val;
    UINT8* pVal;
    UINT8  cFlag  = XIO_TRUE;

    val = a2i(pSTT->data[0].strval);
    if (val == (UINT8)(-1))
    {
        urMax =  MAX_INT;
    }
    else
    {
        urMax =  val;
    }

    if (sessVal != MAX_INT)
    {
        if (sessVal >= urMax)
        {
            /* Value acceptable, send back the same */
            UpdateParams(pSTT, keyPos, 0, pSESSION, pCONN);
            retVal = 0;
            cFlag = XIO_FALSE;
        }
    }

    if (cFlag == XIO_TRUE)
    {
        pVal = GetParamVal(keyPos, pSESSION->tid);
        if (pVal == NULL)
        {
            retVal = VALREJECT;
        }
        else
        {
            myMax = a2i(pVal);
            if (myMax >= urMax)
            {
                /* Value acceptable, send back the same */
                if (sessVal != MAX_INT)
                {
                    UpdateParams(pSTT, keyPos, 0, pSESSION, pCONN);
                }
                retVal = 0;
            }
            else
            {
                /* suggest value */
                /* pSTT->data[0].strval = pVal;*/
                stringCopy(pVal, pSTT->data[0].strval);
                pSTT->numvals = 1;
                retVal = VALNEGOTIATE;
            }
        }
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Generic function to compute maximum among proposed,
**              session/connection and default values of a parameter
**
**  @param
**              key-val pair
**              keyname index
**              session/connection parameter value
**              SESSION pointer
**              CONNECTION pointer
**
**  @return
**              Maximum of values from initiator and target.
**
******************************************************************************
**/
INT8 RFMAX (STT* pSTT, UINT8 keyPos, UINT32 sessVal, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNEGOTIATE;
    UINT32 myMin, urMin;
    INT32  val;
    UINT8* pVal;
    UINT8  cFlag  = XIO_TRUE;

    val = a2i(pSTT->data[0].strval);
    if (val == (UINT8)(-1))
    {
        urMin = MIN_NATURAL;
    }
    else
    {
        urMin = val;
    }

    if (sessVal != MAX_INT)
    {
        if (sessVal <= urMin)
        {
            /* Value acceptable, send back the same */
            UpdateParams(pSTT, keyPos, 0, pSESSION, pCONN);
            retVal = 0;
            cFlag = XIO_FALSE;
        }
    }

    if (cFlag == XIO_TRUE)
    {
        pVal = GetParamVal(keyPos, pSESSION->tid);
        if (pVal == NULL)
        {
            retVal = VALREJECT;
        }
        else
        {
            myMin = a2i(pVal);
            if (myMin <= urMin)
            {
                /* Value acceptable, send back the same */
                if (sessVal != MAX_INT)
                {
                    UpdateParams(pSTT, keyPos, 0, pSESSION, pCONN);
                }
                retVal = 0;
            }
            else
            {
                /* suggest value */
                /* pSTT->data[0].strval = pVal; */
                stringCopy(pVal, pSTT->data[0].strval);
                pSTT->numvals = 1;
                retVal = VALNEGOTIATE;
            }
        }
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Generic function to compute OR of proposed,
**              session/connection and default values of a parameter
**
**  @param
**              key-val pair
**              keyname index
**              session/connection parameter value
**              SESSION pointer
**              CONNECTION pointer
**
**  @return
**              OR of values from initiator and target.
**
******************************************************************************
**/
INT8 RFOR (STT* pSTT, UINT8 keyPos, UINT8 sessVal, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNEGOTIATE;
    UINT8  myVal, urVal, defVal;

    if (ValidateData(pSTT->data[0].strval, BOOLEAN))
    {
        urVal = getBoolean(pSTT->data[0].strval);
    }
    else
    {
        urVal = XIO_FALSE;
    }

    if (ISVALIDBOOLEAN(sessVal))
    {
        myVal = sessVal;
    }
    else
    {
        myVal = XIO_FALSE;
    }

    defVal = getBoolean(GetParamVal(keyPos, pSESSION->tid));

    defVal = myVal | urVal | defVal;

    /* pSTT->data[0].strval = setBoolean(defVal); */
    stringCopy(setBoolean(defVal), pSTT->data[0].strval);

    if (defVal == myVal)
    {
        /* Same Value */
        retVal = VALOK;
    }
    else
    {
        /* New Value, accept */
        pSTT->numvals = 1;
        UpdateParams(pSTT, keyPos, 0, pSESSION, pCONN);
        retVal = VALNEGOTIATE;
    }

    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Generic function to compute AND of proposed,
**              session/connection and default values of a parameter
**
**  @param
**              key-val pair
**              keyname index
**              session/connection parameter value
**              SESSION pointer
**              CONNECTION pointer
**
**  @return
**              AND of values from initiator and target.
**
******************************************************************************
**/
INT8 RFAND (STT* pSTT, UINT8 keyPos, UINT8 sessVal, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNEGOTIATE;
    UINT8  myVal, urVal, defVal;

    if (ValidateData(pSTT->data[0].strval, BOOLEAN))
    {
        urVal = getBoolean(pSTT->data[0].strval);
    }
    else
    {
        urVal = XIO_FALSE;
    }

    if (ISVALIDBOOLEAN(sessVal))
    {
        myVal = sessVal;
    }
    else
    {
        myVal = XIO_TRUE;
    }

    defVal = getBoolean(GetParamVal(keyPos, pSESSION->tid));

    defVal = myVal & urVal & defVal;

    /* pSTT->data[0].strval = setBoolean(defVal); */
    stringCopy(setBoolean(defVal), pSTT->data[0].strval);

    if (defVal == myVal)
    {
        /* Same Value */
        retVal = VALOK;
    }
    else
    {
        /* New Value, accept */
        pSTT->numvals = 1;
        UpdateParams(pSTT, keyPos, 0, pSESSION, pCONN);
        retVal = VALNEGOTIATE;
    }

    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Validate and Negotiate Header Digest Values
**
** Negotiation: A declaration is a one-way textual exchange while a negotiation
** is a two-way exchange.
**
** In list negotiation, the originator sends a list of values (which may include
** in its "None") order of preference.
** The responding party MUST respond with the same key and the first value that
** it supports (and is allowed to use for the specific originator) selected from
**  the originator list. The constant "None" MUST always be used to indicate a
** missing function. However, "None" is only**  a valid selection if it is
** explicitly proposed.If an acceptor does not understand any particular value in
** a list, it MUST ignore it. If an acceptor does not support, does not
** understand, or is not allowed to use any of the proposed options with a
** specific originator, it may use the constant "Reject" or terminate the
** negotiation. The selection of a value not proposed MUST be handled as a
** protocol error.
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return
**
**              -2 if Reject
**              -1 if Negotiate
**              Index to key-val to be sent as response. The same return codes
**              hold for all the parameters
**
******************************************************************************
**/
INT8 NegotiateHeaderDigest (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    UINT32 i;
    INT8   retVal = VALREJECT;

    if (pCONN == NULL)
    {
        return (retVal);
    }
    /*
    ** If Xiotech Initiator, donot validate. None will be sent
    */
    if(fsl_is_xioInit(pSESSION->params.initiatorName) == TRUE)
    {
        stringCopy((UINT8 *)"None", pCONN->params.headerDigest.strval);
        retVal = 0;
        return (retVal);
    }

    /*
    ** Value not present, see if it can be added
    */
    for (i=0; i<pSTT->numvals; i++)
    {
        if (FindParamVal(HEADERDIGESTINDEX, pSTT->data[i].strval, pSESSION->tid))
        {
            /*
            ** Acceptable value, add
            */
            UpdateParams(pSTT, HEADERDIGESTINDEX, i, pSESSION, pCONN);
            retVal = i;
        }
    }
    if(retVal == VALREJECT)
    {
        /*
        ** header digest is mandatory but not being negotiated by initiator
        */
        char tmp_buff[300] = {0};
        sprintf(tmp_buff, "INITIATOR ERROR HDR Digest Mandatory/Disabled  on Target %d ", pSESSION->tid);
        iscsi_dprintf(tmp_buff);
        stringCopy((UINT8 *)"HDIGEST_MANDATORY_ERR", pCONN->params.headerDigest.strval);
    }

    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Validate and Negotiate Data Digest Values
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return     error code/index to key-val to be sent as response
**
******************************************************************************
**/
INT8 NegotiateDataDigest (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    UINT32 i;
    INT8   retVal = VALREJECT;

    if (pCONN == NULL)
    {
        return (retVal);
    }

    /*
    ** If Xiotech Initiator, donot validate. None will be sent
    */
    if(fsl_is_xioInit(pSESSION->params.initiatorName) == TRUE)
    {
        stringCopy((UINT8 *)"None", pCONN->params.dataDigest.strval);
        retVal = 0;
        return (retVal);
    }

    for (i=0; i<pSTT->numvals; i++)
    {
        if (FindParamVal(DATADIGESTINDEX, pSTT->data[i].strval, pSESSION->tid))
        {
            /* Acceptable value, add */
            UpdateParams(pSTT, DATADIGESTINDEX, i, pSESSION, pCONN);
            retVal = i;
        }
    }
    if(retVal == VALREJECT)
    {
        /*
        ** data digest is mandatory but not being negotiated by initiator
        */
        char tmp_buff[300] = {0};
        sprintf(tmp_buff, "INITIATOR ERROR  Data Digest Mandatory/Disabled on Target %d ", pSESSION->tid);
        iscsi_dprintf(tmp_buff);
        stringCopy((UINT8 *)"DDIGEST_MANDATORY_ERR", pCONN->params.dataDigest.strval);
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Validate and Negotiate Max Connections
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return     error code/index to key-val to be sent as response
**
******************************************************************************
**/
INT8 NegotiateMaxConnections (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNEGOTIATE;

    if (pSESSION == NULL)
    {
        retVal = RFMIN(pSTT, MAXCONNECTIONSINDEX, MAX_INT, pSESSION, pCONN);
    }
    else
    {
        retVal = RFMIN(pSTT, MAXCONNECTIONSINDEX,
                       pSESSION->params.maxConnections, pSESSION, pCONN);
    }
    return (retVal);
}

UINT32 NegotiateSendTargets(SESSION *pSsn, STT* pSTT, UINT8* resp, UINT32 pos)
{

    /*
    ** In a discovery session, a target MUST return all path information
    ** (target name and IP address-port pairs and portal group tags)
    ** for the targets on the target network entity which the requesting initiator is
    ** authorized to access.
    */
    UINT16 i = 0;
    UINT16 port = 0;
    UINT8  str[MAX_KEYVAL_LEN+1];
    INT32 count=0;

    for(count = 0; count < MAX_TARGETS; count++)
    {
        TGTINFO[count].status = 0;
    }

    port = ispPortAssignment[pSsn->tid];


    if (pSTT->numvals == 0)
    {
        /*
         ** Send info of the Target on which request arrived
         */
    }
    else if (stringCompare(pSTT->data[0].strval, (UINT8*)SENDTARGETS_ALL) == XIO_ZERO)
    {
        /*
         ** Send info of all the Targets
         */
        /* get the TGTINFO array by looking into Network portals */
        iscsiGetAllActiveTargets(port, TGTINFO);
        for (i=0; i<MAX_TARGETS; i++)
        {
            if(TGTINFO[i].status == 1)
            {
                sprintf((char *)str, "TargetName=%s", TGTINFO[i].tgtName);
                stringAdd(resp, str, pos);
                pos += stringLength(str) + 1;
                sprintf((char *)str, "TargetAddress=%d.%d.%d.%d:%d,%d", TGTINFO[i].tgtIP[0], TGTINFO[i].tgtIP[1],
                          TGTINFO[i].tgtIP[2], TGTINFO[i].tgtIP[3], TGTINFO[i].tgtPort, TGTINFO[i].tgtPGT);
                stringAdd(resp, str, pos);
                pos += stringLength(str) + 1;
            }
        }
    }
    else if (stringCompare(pSTT->data[0].strval, (UINT8*)"Nothing") == 0)
    {
        /* target address for which session loggeg in */
        iscsiGetAllActiveTargets(port, TGTINFO);
        i = pSsn->tid;
        if(TGTINFO[i].status == 1)
        {
            sprintf((char *)str, "TargetAddress=%d.%d.%d.%d:%d,%d", TGTINFO[i].tgtIP[0], TGTINFO[i].tgtIP[1], TGTINFO[i].tgtIP[2], TGTINFO[i].tgtIP[3], TGTINFO[i].tgtPort, TGTINFO[i].tgtPGT);
            stringAdd(resp, str, pos);
            pos += stringLength(str) + 1;

        }
    }
    else
    {
        /* get ip and port corresponding to target name */
        iscsiGetAddressForTargetName(pSTT->data[0].strval, TGTINFO);
        /* copy port and ip address */
        if(TGTINFO[0].status == 1)
        {
            sprintf((char *)str, "TargetAddress=%d.%d.%d.%d:%d,%d", TGTINFO[0].tgtIP[0], TGTINFO[0].tgtIP[1], TGTINFO[0].tgtIP[2], TGTINFO[0].tgtIP[3], TGTINFO[0].tgtPort, TGTINFO[0].tgtPGT);
            stringAdd(resp, str, pos);
            pos += stringLength(str) + 1;

        }
    }

    return (pos);
}

INT8 NegotiateTargetName (STT* pSTT UNUSED, SESSION* pSESSION UNUSED, CONNECTION* pCONN UNUSED)
{
    return 0;
}

INT8 NegotiateInitiatorName (STT* pSTT UNUSED, SESSION* pSESSION UNUSED, CONNECTION* pCONN UNUSED)
{
    return 0;
}

INT8 NegotiateTargetAlias (STT* pSTT UNUSED, SESSION* pSESSION UNUSED, CONNECTION* pCONN UNUSED)
{
    return 0;
}

INT8 NegotiateInitiatorAlias (STT* pSTT UNUSED, SESSION* pSESSION UNUSED, CONNECTION* pCONN UNUSED)
{
    return 0;
}

INT8 NegotiateTargetAddress (STT* pSTT UNUSED, SESSION* pSESSION UNUSED, CONNECTION* pCONN UNUSED)
{
    return 0;
}

INT8 NegotiateTargetPortalGroupTag (STT* pSTT UNUSED, SESSION* pSESSION UNUSED, CONNECTION* pCONN UNUSED)
{
    return 0;
}

/**
******************************************************************************
**
**  @brief      Validate and Negotiate InitialR2T
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return     error code/index to key-val to be sent as response
**
******************************************************************************
**/
INT8 NegotiateInitialR2T (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNEGOTIATE;

    if (pSESSION == NULL)
    {
        retVal = RFOR(pSTT, INITIALR2TINDEX, 2, pSESSION, pCONN);
    }
    else
    {
        retVal = RFOR(pSTT, INITIALR2TINDEX,
                       pSESSION->params.initialR2T, pSESSION, pCONN);
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Validate and Negotiate Immediate Data
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return     error code/index to key-val to be sent as response
**
******************************************************************************
**/
INT8 NegotiateImmediateData (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNEGOTIATE;

    if (pSESSION == NULL)
    {
        retVal = RFAND(pSTT, IMMEDIATEDATAINDEX, 2, pSESSION, pCONN);
    }
    else
    {
        retVal = RFAND(pSTT, IMMEDIATEDATAINDEX,
                       pSESSION->params.immediateData, pSESSION, pCONN);
    }
    return (retVal);
}

INT8 NegotiateMaxRecvDataSegmentLength (STT* pSTT UNUSED, SESSION* pSESSION UNUSED, CONNECTION* pCONN UNUSED)
{
    return 0;
}

/**
******************************************************************************
**
**  @brief      Validate and Negotiate Max Burst Length
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return     error code/index to key-val to be sent as response
**
******************************************************************************
**/
INT8 NegotiateMaxBurstLength (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNEGOTIATE;

    if (pSESSION == NULL)
    {
        retVal = RFMIN(pSTT, MAXBURSTLENGTHINDEX, MAX_INT, pSESSION, pCONN);
    }
    else
    {
        retVal = RFMIN(pSTT, MAXBURSTLENGTHINDEX,
                       pSESSION->params.maxBurstLength, pSESSION, pCONN);
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Validate and Negotiate First Burst Length
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return     error code/index to key-val to be sent as response
**
******************************************************************************
**/
INT8 NegotiateFirstBurstLength (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNEGOTIATE;

    if (pSESSION == NULL)
    {
        retVal = RFMIN(pSTT, FIRSTBURSTLENGTHINDEX, MAX_INT, pSESSION, pCONN);
    }
    else
    {
        retVal = RFMIN(pSTT, FIRSTBURSTLENGTHINDEX,
                       pSESSION->params.firstBurstLength, pSESSION, pCONN);
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Validate and Negotiate Default Time To Wait
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return     error code/index to key-val to be sent as response
**
******************************************************************************
**/
INT8 NegotiateDefaultTime2Wait (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNEGOTIATE;

    if (pSESSION == NULL)
    {
        retVal = RFMAX(pSTT, DEFAULTTIME2WAITINDEX, MAX_INT, pSESSION, pCONN);
    }
    else
    {
        retVal = RFMAX(pSTT, DEFAULTTIME2WAITINDEX,
                       pSESSION->params.defaultTime2Wait, pSESSION, pCONN);
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Validate and Negotiate Default Time To Retain
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return     error code/index to key-val to be sent as response
**
******************************************************************************
**/
INT8 NegotiateDefaultTime2Retain (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNEGOTIATE;

    if (pSESSION == NULL)
    {
        retVal = RFMIN(pSTT, DEFAULTTIME2RETAININDEX, MAX_INT, pSESSION, pCONN);
    }
    else
    {
        retVal = RFMIN(pSTT, DEFAULTTIME2RETAININDEX,
                       pSESSION->params.defaultTime2Retain, pSESSION, pCONN);
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Validate and Negotiate Max Outstanding R2T
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return     error code/index to key-val to be sent as response
**
******************************************************************************
**/
INT8 NegotiateMaxOutstandingR2T (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNEGOTIATE;

    if (pSESSION == NULL)
    {
        retVal = RFMIN(pSTT, MAXOUTSTANDINGR2TINDEX, MAX_INT, pSESSION, pCONN);
    }
    else
    {
        retVal = RFMIN(pSTT, MAXOUTSTANDINGR2TINDEX,
                       pSESSION->params.maxOutstandingR2T, pSESSION, pCONN);
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Validate and Negotiate Data PDU In Order
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return     error code/index to key-val to be sent as response
**
******************************************************************************
**/
INT8 NegotiateDataPDUInOrder (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNEGOTIATE;

    if (pSESSION == NULL)
    {
        retVal = RFOR(pSTT, DATAPDUINORDERINDEX, 2, pSESSION, pCONN);
    }
    else
    {
        retVal = RFOR(pSTT, DATAPDUINORDERINDEX,
                       pSESSION->params.dataPDUInOrder, pSESSION, pCONN);
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Validate and Negotiate Data Sequence In Order
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return     error code/index to key-val to be sent as response
**
******************************************************************************
**/
INT8 NegotiateDataSequenceInOrder (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNEGOTIATE;

    if (pSESSION == NULL)
    {
        retVal = RFOR(pSTT, DATASEQUENCEINORDERINDEX, 2, pSESSION, pCONN);
    }
    else
    {
        retVal = RFOR(pSTT, DATASEQUENCEINORDERINDEX,
                       pSESSION->params.dataSequenceInOrder, pSESSION, pCONN);
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Validate and Negotiate Error Recovery Level
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return     error code/index to key-val to be sent as response
**
******************************************************************************
**/
INT8 NegotiateErrorRecoveryLevel (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNEGOTIATE;

    if (pSESSION == NULL)
    {
        retVal = RFMIN(pSTT, ERRORRECOVERYLEVELINDEX, MAX_INT, pSESSION, pCONN);
    }
    else
    {
        retVal = RFMIN(pSTT, ERRORRECOVERYLEVELINDEX,
                       pSESSION->params.errorRecoveryLevel, pSESSION, pCONN);
    }
    return (retVal);
}

INT8 NegotiateSessionType (STT* pSTT UNUSED, SESSION* pSESSION UNUSED, CONNECTION* pCONN UNUSED)
{
    return 0;
}

/**
******************************************************************************
**
**  @brief      Validate and Negotiate Authentication Method
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return     error code/index to key-val to be sent as response
**
******************************************************************************
**/
INT8 NegotiateAuthMethod (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    UINT32 i;
    INT8   retVal = VALREJECT;

    if (pCONN == NULL)
    {
        return (retVal);
    }

    /*
    ** If Xiotech Initiator, donot validate. None will be sent
    */
    if(fsl_is_xioInit(pSESSION->params.initiatorName) == TRUE)
    {
        stringCopy((UINT8 *)"None", pCONN->params.authMethod.strval);
        retVal = 0;
        return (retVal);
    }

    /*
    ** Value not present, see if it can be added
    */
    for (i=0; i<pSTT->numvals; i++)
    {
        if (FindParamVal(AUTHMETHODINDEX, pSTT->data[i].strval, pSESSION->tid))
        {
            /* Acceptable value, add */
            UpdateParams(pSTT, AUTHMETHODINDEX, i, pSESSION, pCONN);
            retVal = i;
        }
    }
    if(retVal == VALREJECT)
    {
        /*
        ** authmethod is mandatory but not being negotiated by initiator
        */
        char tmp_buff[300] = {0};
        sprintf(tmp_buff, "AUTHORIZATION FAILED CHAP Mandatory/Disabled on Target %d ", pSESSION->tid);
        iscsi_dprintf(tmp_buff);
        stringCopy((UINT8 *)"AUTH_MANDATORY_ERR", pCONN->params.authMethod.strval);
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Validate and Negotiate IFMarker
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return     error code/index to key-val to be sent as response
**
******************************************************************************
**/
INT8 NegotiateIFMarker (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNEGOTIATE;

    if (pCONN == NULL)
    {
        retVal = RFAND(pSTT, IFMARKERINDEX, 2, pSESSION, pCONN);
    }
    else
    {
        retVal = RFAND(pSTT, IFMARKERINDEX,
                       pCONN->params.ifMarker, pSESSION, pCONN);
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Validate and Negotiate OFMarker
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return     error code/index to key-val to be sent as response
**
******************************************************************************
**/
INT8 NegotiateOFMarker (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNEGOTIATE;

    if (pCONN == NULL)
    {
        retVal = RFAND(pSTT, OFMARKERINDEX, 2, pSESSION, pCONN);
    }
    else
    {
        retVal = RFAND(pSTT, OFMARKERINDEX,
                       pCONN->params.ofMarker, pSESSION, pCONN);
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Validate and Negotiate IFMarkInt
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return     error code/index to key-val to be sent as response
**
******************************************************************************
**/
INT8 NegotiateIFMarkInt (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    UINT32     resVal = 0;
    INT8       retVal = VALREJECT;
    RANGEVALUE range;
    RANGEVALUE myRange;

    ValidateRange(pSTT->data[0].strval, &range);

    if (range.lo != MAX_INT || range.hi != MAX_INT)
    {
        if (pCONN == NULL)
        {
            ValidateRange(GetParamVal(IFMARKINTINDEX, pSESSION->tid), &myRange);
            resVal = fitRange(&myRange, &range);
            if (resVal != MAX_INT)
            {
                /* acceptable value, add */
                pSTT->data[0].strval[0] = '\0';
                i2a(resVal, pSTT->data[0].strval);
                retVal = VALOK;
            }
        }
        else
        {
            resVal = fitRange(&(pCONN->params.ifMarkInt), &range);

            if (fitRange(&(pCONN->params.ifMarkInt), &range) != MAX_INT)
            {
                /* acceptable value, add */
                pSTT->data[0].strval[0] = '\0';
                i2a(resVal, pSTT->data[0].strval);
                UpdateParams(pSTT, IFMARKINTINDEX, 0, pSESSION, pCONN);
                retVal = VALOK;
            }
        }
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Validate and Negotiate OFMarkInt
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return     error code/index to key-val to be sent as response
**
******************************************************************************
**/
INT8 NegotiateOFMarkInt (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    UINT32     resVal = 0;
    INT8       retVal = VALREJECT;
    RANGEVALUE range;
    RANGEVALUE myRange;

    ValidateRange(pSTT->data[0].strval, &range);

    if (range.lo != MAX_INT || range.hi != MAX_INT)
    {
        if (pCONN == NULL)
        {
            ValidateRange(GetParamVal(OFMARKINTINDEX, pSESSION->tid), &myRange);
            resVal = fitRange(&myRange, &range);
            if (resVal != MAX_INT)
            {
                /* acceptable value, add */
                pSTT->data[0].strval[0] = '\0';
                i2a(resVal, pSTT->data[0].strval);
                retVal = VALOK;
            }
        }
        else
        {
            resVal = fitRange(&(pCONN->params.ifMarkInt), &range);

            if (fitRange(&(pCONN->params.ifMarkInt), &range) != MAX_INT)
            {
                /* acceptable value, add */
                pSTT->data[0].strval[0] = '\0';
                i2a(resVal, pSTT->data[0].strval);
                UpdateParams(pSTT, OFMARKINTINDEX, 0, pSESSION, pCONN);
                retVal = VALOK;
            }
        }
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Validate and Negotiate CHAP_A Values
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return
**              -2 if Reject
**              -1 if Negotiate
**              Index to key-val to be sent as response. The same return codes
**              hold for all the parameters
**
******************************************************************************
**/
INT8 NegotiateChap_A (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    UINT32 i;
    INT8   retVal = VALREJECT;
    UINT8  cFlag = XIO_TRUE;

    if (pCONN == NULL)
    {
        return (retVal);
    }
    for (i=0; i<pSTT->numvals && cFlag == XIO_TRUE; i++)
    {
        if (FindParamVal(CHAP_AINDEX, pSTT->data[i].strval, pSESSION->tid))
        {
            /* Acceptable value, add */
            UpdateParams(pSTT, CHAP_AINDEX, i, pSESSION, pCONN);
            retVal = VALNONE;
            cFlag = XIO_FALSE;
        }
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Update CHAP_N Value
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return
**              -2 if Reject
**              -1 if Negotiate
**              Index to key-val to be sent as response. The same return codes
**              hold for all the parameters
**
******************************************************************************
**/
INT8 NegotiateChap_N (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNONE;

    if (pCONN == NULL)
    {
        return (retVal);
    }

    /* Acceptable value, add */
    UpdateParams(pSTT, CHAP_NINDEX, 0, pSESSION, pCONN);
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Update CHAP_R Value
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return
**              -2 if Reject
**              -1 if Negotiate
**              Index to key-val to be sent as response. The same return codes
**              hold for all the parameters
**
******************************************************************************
**/
INT8 NegotiateChap_R (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNONE;

    if (pCONN == NULL)
    {
        return (retVal);
    }

    /* Acceptable value, add */
    UpdateParams(pSTT, CHAP_RINDEX, 0, pSESSION, pCONN);
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Update CHAP_C Value
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return
**              -2 if Reject
**              -1 if Negotiate
**              Index to key-val to be sent as response. The same return codes
**              hold for all the parameters
**
******************************************************************************
**/
INT8 NegotiateChap_C (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNONE;

    if (pCONN == NULL)
    {
        return (retVal);
    }

    /* Acceptable value, add */
    UpdateParams(pSTT, CHAP_CINDEX, 0, pSESSION, pCONN);
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Update CHAP_I Value
**
**  @param      key-val pair
**              SESSION pointer
**              CONNECTION pointer
**
**  @return
**              -2 if Reject
**              -1 if Negotiate
**              Index to key-val to be sent as response. The same return codes
**              hold for all the parameters
**
******************************************************************************
**/
INT8 NegotiateChap_I (STT* pSTT, SESSION* pSESSION, CONNECTION* pCONN)
{
    INT8   retVal = VALNONE;

    if (pCONN == NULL)
    {
        return (retVal);
    }

    /* Acceptable value, add */
    UpdateParams(pSTT, CHAP_IINDEX, 0, pSESSION, pCONN);
    return (retVal);
}


/**
******************************************************************************
**
**  @brief      Routine to search for a keyname in the initiator's message
**
******************************************************************************
**/
MSGLIST* SearchKey(MSGLIST* msghead, UINT8* keyname)
{
    MSGLIST* msg;
    for (msg = msghead; msg != NULL; msg=msg->next)
    {
        if (stringCompare(msg->decl.keyname, keyname)==0)
        {
            return (msg);
        }
    }
    return (NULL);
}

/**
******************************************************************************
**
**  @brief      Routine to build and send text messages
**
******************************************************************************
**/
UINT32 BuildMsg(INT8 keyPos, UINT8* resp, UINT32 pos, UINT16 tid)
{
    UINT8  str[MAX_KEYVAL_LEN+1];
    I_TGD* itgd;

    if (keyPos == TARGETALIASINDEX)
    {
        if ((itgd = T_tgdindx[tid]->itgd) != NULL)
        {
            if (strlen((char *)itgd->tgtAlias) > 0)
            {
                sprintf((char *)str, "TargetAlias=%s", itgd->tgtAlias);
                stringAdd(resp, str, pos);
                pos += stringLength(str) + 1;
            }
        }
        return (pos);
    }
    if (keyPos == TARGETPORTALGROUPTAGINDEX)
    {
        UINT8 value[10]= {0};
        i2a(tid, value);
        sprintf((char *)str, "TargetPortalGroupTag=%s", value);
        stringAdd(resp, str, pos);
        pos += stringLength(str) + 1;
        return (pos);
    }
    sprintf((char *)str, "%s=%s", KEYNAMES[keyPos], GetParamVal(keyPos, tid));
    stringAdd(resp, str, pos);
    pos += stringLength(str) + 1;
    return (pos);
}

/**
******************************************************************************
**
**  @brief      Routine to send response to declaratives
**
******************************************************************************
**/
UINT32 ProcessDeclarative(SESSION *pSsn, STT* pSTT, INT8 keyPos, UINT8* resp, UINT32 pos)
{
    switch (keyPos)
    {
        case SENDTARGETSINDEX:
            pos = NegotiateSendTargets (pSsn, pSTT, resp, pos);
            break;
        default:
            break;
    }
    return (pos);
}

/**
******************************************************************************
**
**  @brief      Routine to validate and negotiate parameters and creating
**              text responses
**
**  @param
**              pointer to the head key-val pairs list
**              response buffer
**              buffer length
**              SESSION pointer
**              CONNECTION pointer
**
******************************************************************************
**/
INT32 ProcessTxtMsg(MSGLIST* msghead, UINT8* resp, UINT32* respLen, SESSION* pSESSION, CONNECTION* pCONN)
{
    MSGLIST* pMSG;
    STT*     pSTT;
    INT32     keyPos;
    UINT8    keynull;
    UINT8    keyname[MAX_KEYNAME_LEN+1];
    UINT32   pos = 0;
    INT32    res;
    UINT8    i;
    INT32 result = XIO_SUCCESS;

    if(msghead != NULL)
    {
        for (pMSG = msghead; pMSG != NULL; pMSG = pMSG->next)
        {
            /* Process Each Message */
            pSTT = &(pMSG->decl);
            keynull = XIO_FALSE;
            stringCopy(pSTT->keyname, keyname);
            if (keyname[0] == '\0')
            {
                fprintf(stderr, "Txt, key name is NULL, continue for next key\n");
                continue;
            }
            keyPos = FindKeyName(keyname);
            if (keyPos == -1)
            {
                fprintf(stderr, "************************* key name %s not found\n", keyname);
                /*
                ** Keyname not found, send response NOTUNDERSTOOD
                */
                stringAdd(resp, keyname, pos);
                pos += stringLength(pSTT->keyname);
                stringAdd(resp, (UINT8*)"=", pos);
                pos++;
                stringAdd(resp, (UINT8*)NOTUNDERSTOOD, pos);
                pos += stringLength((UINT8*)NOTUNDERSTOOD);
            }
            else
            {
                res = ValidateRelevance(keyPos, pSESSION, pCONN);
                /*
                ** Negotiate the parameter
                */
                if(isSessionParam(keyPos) == true && isNegotiableTwice(keyPos) == false )
                {
                    if(BIT_TEST(pSESSION->params.paramSentMap, keyPos))
                    {
                        /*
                        ** Here we should see if the value sent to us are accepted to us or not
                        ** TODO?
                        */
                    }else if(BIT_TEST(pSESSION->params.paramMap, keyPos))
                    {
                        fprintf(stderr, "tid %d initiatorName %s parameter %s is negotiated twice\n", pSESSION->tid,
                                pSESSION->params.initiatorName, KEYNAMES[keyPos]);
                        return XIO_FAILURE;
                    }
                    BIT_SET(pSESSION->params.paramMap, keyPos);
                    pSESSION->params.paramSentMap =  BIT_UNSET(pSESSION->params.paramSentMap, keyPos);
                }else if(isConnectionParam(keyPos)== true && isNegotiableTwice(keyPos) == false)
                {
                    if(BIT_TEST(pCONN->params.paramSentMap, keyPos))
                    {
                        /*
                        ** Here we should see if the value sent to us are accepted to us or not
                        **TODO?
                        */
                    }else if(BIT_TEST(pCONN->params.paramMap, keyPos))
                    {
                        fprintf(stderr, "tid %d initiatorName %s parameter %s is negotiated twice\n", pSESSION->tid,
                                pSESSION->params.initiatorName, KEYNAMES[keyPos]);
                        return XIO_FAILURE;
                    }
                    BIT_SET(pCONN->params.paramMap, keyPos);
                    pCONN->params.paramSentMap =  BIT_UNSET(pCONN->params.paramSentMap, keyPos);
                }
                switch(res)
                {
                    case SENDIRRELEVANT:
                        /*
                        ** Parameter Not Relevant, send response IRRELEVANT
                        */
                        stringAdd(resp, pSTT->keyname, pos);
                        pos += stringLength(pSTT->keyname);
                        stringAdd(resp, (UINT8*)"=", pos);
                        pos++;
                        stringAdd(resp, (UINT8*)IRRELEVANT, pos);
                        pos += stringLength((UINT8*)IRRELEVANT);
                        break;
                    case SENDREJECT:
                        /*
                        ** Reject
                        */
                        stringAdd(resp, (UINT8*)pSTT->keyname, pos);
                        pos += stringLength(pSTT->keyname);
                        stringAdd(resp, (UINT8*)"=", pos);
                        pos++;
                        stringAdd(resp, (UINT8*)REJECT, pos);
                        pos += stringLength((UINT8*)REJECT);
                        break;
                    case SENDNEGOTIATE:
                        /*
                        ** Negotiate the parameter
                        */
                        res = ParamNegotiateTable[keyPos](pSTT, pSESSION, pCONN);
                        if (res == VALNONE)
                        {
                            /*
                            ** Do Nothing
                            */
                            keynull = XIO_TRUE;
                        }
                        else if (res == VALREJECT)
                        {
                            /* Reject */
                            stringAdd(resp, (UINT8*)pSTT->keyname, pos);
                            pos += stringLength(pSTT->keyname);
                            stringAdd(resp, (UINT8*)"=", pos);
                            pos++;
                            stringAdd(resp, (UINT8*)REJECT, pos);
                            pos += stringLength((UINT8*)REJECT);
                        }
                        else if (res == VALNEGOTIATE)
                        {
                            stringAdd(resp, (UINT8*)pSTT->keyname, pos);
                            pos += stringLength(pSTT->keyname);
                            stringAdd(resp, (UINT8*)"=", pos);
                            pos++;
                            for (i=0; i<pSTT->numvals; i++)
                            {
                                stringAdd(resp, pSTT->data[i].strval, pos);
                                pos += stringLength(pSTT->data[i].strval);
                                resp[pos] = ',';
                                pos++;
                            }
                            pos--;
                        }
                        else if (res >= 0)
                        {
                            /* Send Back New/Accepted Value */
                            stringAdd(resp, (UINT8*)pSTT->keyname, pos);
                            pos += stringLength(pSTT->keyname);
                            stringAdd(resp, (UINT8*)"=", pos);
                            pos++;
                            stringAdd(resp, pSTT->data[res].strval, pos);
                            pos += stringLength(pSTT->data[res].strval);
                        }
                        break;
                    case SENDOK:
                        /* Declarative */
                        pos = ProcessDeclarative(pSESSION, pSTT, keyPos, resp, pos);
                        UpdateParams(pSTT, keyPos, 0, pSESSION, pCONN);
                        keynull = XIO_TRUE;
                        break;
                    default:
                        break;
                }
            }
            if (keynull == XIO_FALSE)
            {
                resp[pos] = '\0';
                pos++;
            }
        }
    }
    if (pCONN != NULL && pCONN->state == CONNST_XPT_UP)
    {
       if (stringCompare(pCONN->params.authMethod.strval, (UINT8 *)"CHAP") == 0)
       {
           pCONN->isChap = XIO_TRUE;
       }

    }
    if (pCONN != NULL && ( pCONN->state == CONNST_IN_LOGIN || pCONN->state == CONNST_XPT_UP))
    {
        ISCSI_LOGIN_REQ_HDR *pLoginReq = (ISCSI_LOGIN_REQ_HDR *)(pCONN->pHdr);
        UINT8 flag = GET_CSG(pLoginReq->flags);
        if(flag == CSG_SECURITY && pSESSION->params.sessionType == NORMAL_SESSION)
        {
            if(!BIT_TEST(pCONN->params.paramMap, TARGETPORTALGROUPTAGINDEX ))
            {
                pos = BuildMsg(TARGETPORTALGROUPTAGINDEX, resp, pos, pSESSION->tid);
                BIT_SET(pCONN->params.paramMap, TARGETPORTALGROUPTAGINDEX);
            }
        }
        if(flag == CSG_LOGIN_OP_NEG)
        {
            if(!BIT_TEST(pCONN->params.paramMap, MAXRECVDATASEGMENTLENGTHINDEX))
            {
                pos = BuildMsg(MAXRECVDATASEGMENTLENGTHINDEX, resp, pos, pSESSION->tid);
                BIT_SET(pCONN->params.paramMap, MAXRECVDATASEGMENTLENGTHINDEX);

             }
            if(pSESSION->params.sessionType == NORMAL_SESSION)
            {
                if(!BIT_TEST(pSESSION->params.paramMap, TARGETALIASINDEX))
                {
                    pos = BuildMsg(TARGETALIASINDEX, resp, pos, pSESSION->tid);
                    BIT_SET(pSESSION->params.paramMap, TARGETALIASINDEX);
                    BIT_SET(pSESSION->params.paramSentMap, TARGETALIASINDEX);
                }
                /*
                ** Followoing parameters will be negotiated by initiator
                */

                if (!BIT_TEST(pSESSION->params.paramMap, MAXBURSTLENGTHINDEX))
                {
                    pos = BuildMsg(MAXBURSTLENGTHINDEX, resp, pos, pSESSION->tid);
                    BIT_SET(pSESSION->params.paramMap, MAXBURSTLENGTHINDEX);
                    BIT_SET(pSESSION->params.paramSentMap, MAXBURSTLENGTHINDEX);
                    result = UNSET_TR_BIT;
                }
                if (!BIT_TEST(pSESSION->params.paramMap, FIRSTBURSTLENGTHINDEX))
                {
                    pos = BuildMsg(FIRSTBURSTLENGTHINDEX, resp, pos, pSESSION->tid);
                    BIT_SET(pSESSION->params.paramMap, FIRSTBURSTLENGTHINDEX);
                    BIT_SET(pSESSION->params.paramSentMap, FIRSTBURSTLENGTHINDEX);
                    result = UNSET_TR_BIT;

                }
                if (!BIT_TEST(pSESSION->params.paramMap, IMMEDIATEDATAINDEX))
                {
                    pos = BuildMsg(IMMEDIATEDATAINDEX, resp, pos, pSESSION->tid);
                    BIT_SET(pSESSION->params.paramMap, IMMEDIATEDATAINDEX);
                    BIT_SET(pSESSION->params.paramSentMap, IMMEDIATEDATAINDEX);
                    result = UNSET_TR_BIT;
                }
                if (!BIT_TEST(pSESSION->params.paramMap, INITIALR2TINDEX))
                {
                    pos = BuildMsg(INITIALR2TINDEX, resp, pos, pSESSION->tid);
                    BIT_SET(pSESSION->params.paramMap, INITIALR2TINDEX);
                    BIT_SET(pSESSION->params.paramSentMap, INITIALR2TINDEX);
                    result = UNSET_TR_BIT;
                }
            }

        }
    }

    *respLen = pos;
    return (result);
}

/**
******************************************************************************
**
**  @brief      Process a Text Message
**
**  @param
**              request buffer
**              response buffer
**              buffer length
**              SESSION pointer
**              CONNECTION pointer
**
******************************************************************************
**/
UINT8  iscsiProcTxtReq(UINT8 *pReqBuff, UINT8 *pRespBuff, UINT32* length,
                                             SESSION* pSESSION, CONNECTION* pCONN)
{
    INT32 result;
    MSGLIST* msghead=NULL;
    msghead = ParseTextMsg(pReqBuff, *length);
    result = ProcessTxtMsg(msghead, pRespBuff, length, pSESSION, pCONN);
    DestroyList(&msghead);
    return(result);
}

/**
******************************************************************************
**
**  @brief      Initialize Session Parameters
**
**  @param
**              SESSION pointer
**
**  @return     none
**
******************************************************************************
**/
void InitSessionParams (SESSION* pSESSION)
{
    if (pSESSION == NULL)
    {
        return;
    }
    memset(&(pSESSION->params), 0, sizeof(pSESSION->params));
    pSESSION->params.maxConnections = a2i(GetParamVal(MAXCONNECTIONSINDEX, pSESSION->tid));
    pSESSION->params.initialR2T = getBoolean(GetParamVal(INITIALR2TINDEX, pSESSION->tid));
    pSESSION->params.immediateData = getBoolean(GetParamVal(IMMEDIATEDATAINDEX, pSESSION->tid));
    pSESSION->params.maxBurstLength = a2i(GetParamVal(MAXBURSTLENGTHINDEX, pSESSION->tid));
    pSESSION->params.firstBurstLength = a2i(GetParamVal(FIRSTBURSTLENGTHINDEX, pSESSION->tid));
    pSESSION->params.targetPortalGroupTag = a2i(GetParamVal(TARGETPORTALGROUPTAGINDEX, pSESSION->tid));
    pSESSION->params.defaultTime2Wait = a2i(GetParamVal(DEFAULTTIME2WAITINDEX, pSESSION->tid));
    pSESSION->params.defaultTime2Retain = a2i(GetParamVal(DEFAULTTIME2RETAININDEX, pSESSION->tid));
    pSESSION->params.maxOutstandingR2T = a2i(GetParamVal(MAXOUTSTANDINGR2TINDEX, pSESSION->tid));
    pSESSION->params.dataPDUInOrder = getBoolean(GetParamVal(DATAPDUINORDERINDEX, pSESSION->tid));
    pSESSION->params.dataSequenceInOrder = getBoolean(GetParamVal(DATASEQUENCEINORDERINDEX, pSESSION->tid));
    pSESSION->params.errorRecoveryLevel = a2i(GetParamVal(ERRORRECOVERYLEVELINDEX, pSESSION->tid));
}

/**
******************************************************************************
**
**  @brief      Initialize Connection Parameters
**
**  @param
**              CONNECTION pointer
**
**  @return     none
**
******************************************************************************
**/
void InitConnectionParams (CONNECTION* pCONN)
{
    UINT16 tid;

    if (pCONN == NULL)
    {
        return;
    }
    tid = pCONN->pTPD->tid;
    memset(&(pCONN->params), 0, sizeof(pCONN->params));

    stringCopy((UINT8 *)"None", pCONN->params.headerDigest.strval);
    stringCopy((UINT8 *)"None", pCONN->params.dataDigest.strval);

    pCONN->params.chap_A = 0;
    pCONN->isChap = XIO_FALSE;

    pCONN->params.maxRecvDataSegmentLength =
                              a2i(GetParamVal(MAXRECVDATASEGMENTLENGTHINDEX, tid));
    pCONN->params.maxSendDataSegmentLength =
                 a2i(GetParamVal(MAXSENDDATASEGMENTLENGTHINDEX, tid));

    stringCopy((UINT8 *)"None", pCONN->params.authMethod.strval);
    pCONN->params.ifMarker = getBoolean(GetParamVal(IFMARKERINDEX, tid));
    pCONN->params.ofMarker = getBoolean(GetParamVal(OFMARKERINDEX, tid));
}

/**
******************************************************************************
**
**  @brief      Set a  keyvalue against the position
**
**  @param      keyname index, key value
**
**  @return     XIO_TRUE if set XIO_FALSE otherwise
**
******************************************************************************
**/
/*UINT8 SetParamVal(UINT8 keyPos, UINT8* keyVal, PARAMVALS *paramVal)*/
UINT8 SetParamVal(UINT8 keyPos, UINT8* keyVal, PARAM_TABLE *pParamTable)
{
    INT8  retVal = XIO_FALSE;
    UINT8 i;

    for (i=0; i<pParamTable->index; i++)
    {
        if (pParamTable->ptrParams[i].keyName == keyPos )
        {
            stringCopy(keyVal, pParamTable->ptrParams[i].keyVal);
            retVal = XIO_TRUE;
            break;
        }
    }
    if (retVal == XIO_FALSE)
    {
        i = pParamTable->index;
        pParamTable->ptrParams[i].keyName = keyPos;
        stringCopy(keyVal, pParamTable->ptrParams[i].keyVal);
        pParamTable->index++;
    }
    return (retVal);
}
#define TWO_RAISE_TO_24 16777216

bool isSupportedNumericParamValue(INT32 paramIndex UNUSED, INT32 intVal UNUSED)
{

    return TRUE; /*TEST code*/
#if 0
    bool retVal = FALSE;

    switch(paramIndex)
    {
        case MAXCONNECTIONSINDEX:
            if(intVal == 1)
            {
                retVal = TRUE;
            }
            break;
        case MAXRECVDATASEGMENTLENGTHINDEX:
        case MAXSENDDATASEGMENTLENGTHINDEX:
        case MAXBURSTLENGTHINDEX:
            retVal = TRUE;
            break;
        case FIRSTBURSTLENGTHINDEX:
            break;
        case DEFAULTTIME2WAITINDEX:
        case DEFAULTTIME2RETAININDEX:
            break;
        case MAXOUTSTANDINGR2TINDEX:
            if(intVal == 1)
            {
                retVal = TRUE;
            }
            break;

        case ERRORRECOVERYLEVELINDEX:
            if(intVal == 0)
            {
                retVal = TRUE;
            }
            break;
    }
    return retVal;
#endif  /* 0 */
}
bool isValidParamValueRange(INT32 paramIndex UNUSED, INT32 intVal UNUSED)
{
    return TRUE;  /*TEST CODE*/
#if 0
    bool retVal = FALSE;

    switch(paramIndex)
    {

        case MAXCONNECTIONSINDEX:
            if(intVal >= 1 && intVal <= 65535)
            {
                retVal = TRUE;
            }
            break;
        case MAXSENDDATASEGMENTLENGTHINDEX:
            retVal = TRUE;
            break;
        case MAXRECVDATASEGMENTLENGTHINDEX:
        case MAXBURSTLENGTHINDEX:
        case FIRSTBURSTLENGTHINDEX:
            if(intVal >= 512  && intVal <= TWO_RAISE_TO_24 -1)
            {
                retVal = TRUE;
            }
            break;
        case DEFAULTTIME2WAITINDEX:
        case DEFAULTTIME2RETAININDEX:
            if(intVal >= 0  && intVal <= 3600)
            {
                retVal = TRUE;
            }
            break;
        case MAXOUTSTANDINGR2TINDEX:
            if(intVal >= 0  && intVal <= 65535)
            {
                retVal = TRUE;
            }
            break;
        case ERRORRECOVERYLEVELINDEX:
            if(intVal >= 0  && intVal <= 2)
            {
                retVal = TRUE;
            }
            break;

    }
    return retVal;
#endif  /* 0 */
}

/*
@name    iscsiSetParameterValue

@brief    This function validates the value and then set the parameter table with the new value.

@param

@return    ISCSI_INVALID_PARAM
    ISCSI_INVALID_VALUE
    ISCSI_OUT_OF_RANGE
    ISCSI_NO_SUPPORT
    DEOK
*/
/*UINT8 iscsiSetParameterValue(INT32 paramIndex, UINT8 *paramVal, PARAMVALS *configParamVal)*/
UINT8 iscsiSetParameterValue(INT32 paramIndex, UINT8 *paramVal, PARAM_TABLE *pParamTable)
{
    INT32 intVal = -1;
    UINT8 status = DEOK;
    if(paramIndex < 0 || paramIndex >= NUM_PARAMS)
    {
        return ISCSI_INVALID_PARAM;
    }

    switch(paramIndex)
    {
        /*integer values*/

        case MAXSENDDATASEGMENTLENGTHINDEX:
             break;
        case MAXCONNECTIONSINDEX:
        case MAXRECVDATASEGMENTLENGTHINDEX:
        case MAXBURSTLENGTHINDEX:
        case FIRSTBURSTLENGTHINDEX:
        case DEFAULTTIME2WAITINDEX:
        case DEFAULTTIME2RETAININDEX:
        case MAXOUTSTANDINGR2TINDEX:
        case ERRORRECOVERYLEVELINDEX:
        case TARGETPORTALGROUPTAGINDEX:

            intVal = a2i(paramVal);

            if(isValidParamValueRange(paramIndex, intVal) == TRUE)
            {
                if(isSupportedNumericParamValue(paramIndex, intVal)== TRUE)
                {
                    break;
                }
//                else
//                {
//                    status = ISCSI_NO_SUPPORT;
//                }
            }
            /*reaching here means wrong value*/
            status = ISCSI_INVALID_VALUE;
            fprintf(stderr, "status = ISCSI_INVALID_VALUE  %d\n", status);

            break;

        /* range value*/
        case IFMARKINTINDEX:
        case OFMARKINTINDEX:
            status = ISCSI_NO_SUPPORT;
            break;

        /*boolean value*/
        case INITIALR2TINDEX:
        case IMMEDIATEDATAINDEX:
        case DATAPDUINORDERINDEX:
        case DATASEQUENCEINORDERINDEX:
        case IFMARKERINDEX:
        case OFMARKERINDEX:

            break;

            /*all other indices are not supported through this function*/
        default:
            status = ISCSI_NO_SUPPORT;
    }

    if(status == DEOK)
    {
        SetParamVal(paramIndex, paramVal, pParamTable);
    }
    return status;
}
/*
@name    DisableParamNegotiation

@brief    This function disables param negotiation for Digest and AuthMethod.

@param

@return     XIO_SUCCESS
         XIO_FAILURE
*/

UINT8  DisableParamNegotiation(INT32 paramIndex, PARAM_TABLE *paramTable, UINT8 *val )
{
    UINT8 retVal = XIO_SUCCESS;
    INT32 i = 0;

    switch(paramIndex)
    {
        case HEADERDIGESTINDEX:
        case DATADIGESTINDEX:
            /*Find out in parameter table where the value is stored and make the first entry as "None"
                The second entry will not be there
            */
        case AUTHMETHODINDEX:
            for (i=0; i<paramTable->index; i++)
            {
                if(paramTable->ptrParams[i].keyName ==  paramIndex)
                {
                    if(strcmp((char *)paramTable->ptrParams[i].keyVal, (char *)val) == 0)
                    {
                        paramTable->ptrParams[i] = paramTable->ptrParams[paramTable->index -1];
                        paramTable->index--;
                        retVal = XIO_SUCCESS;        /*This entry is already there so we can return success*/
                        break;
                    }
                }
            }
        default:
            break;
    }
    return retVal;

}
/*
@name    AddValueInParamTable

@brief    This function adds the given string against paramIndex
@param   INT32 paramIndex, PARAM_TABLE *paramTable, UINT8 bitmap

@return     XIO_SUCCESS
         XIO_FAILURE
*/

void AddValueInParamTable(INT32 paramIndex, PARAM_TABLE *paramTable, const UINT8 *val)
{
    if(val != NULL)
    {
       paramTable->ptrParams[paramTable->index].keyName = paramIndex;
       stringCopy(val, paramTable->ptrParams[paramTable->index++].keyVal); /*increment the index*/
    }

}
/*
@name    EnableParamNegotiation

@brief    This function insert entries according to bitmap. This function assumes that there is no value other than None is in the table
            for paramIndex.
@param   INT32 paramIndex, PARAM_TABLE *paramTable, UINT8 bitmap

@return     XIO_SUCCESS
         XIO_FAILURE
*/


#define ENABLE_CRC32C 0x01
#define PARAM_MANDATORY 0x80
#define ENABLE_CHAP 0x01

UINT8  EnableParamNegotiation(INT32 paramIndex, PARAM_TABLE *paramTable, UINT8 bitmap, UINT16 tid)
{
    UINT8 retVal = XIO_FAILURE;
    switch(paramIndex)
    {
        case HEADERDIGESTINDEX:
        case DATADIGESTINDEX:
             if( bitmap & ENABLE_CRC32C)
             {
                AddValueInParamTable(paramIndex, paramTable, (UINT8 *)"CRC32C");
             }
             break;
        case AUTHMETHODINDEX:
             if( bitmap & ENABLE_CHAP)
             {
                AddValueInParamTable(paramIndex, paramTable, (UINT8 *)"CHAP");
                RemoveAllListValuesForIndex(CHAP_AINDEX, paramTable);
                AddValueInParamTable(CHAP_AINDEX, paramTable, (UINT8 *)"5");
             }
             /*
             ** uncomment when we support SRP
             */

             /*
             if( bitmap & 0x2)
             {
                AddValueInParamTable(paramIndex, paramTable, "SRP");
             }
             */
            break;
        default:
            break;
    }
    if (!(bitmap & PARAM_MANDATORY))
    {
        /*
        ** add None also
        */
        AddValueInParamTable(paramIndex, paramTable, (UINT8 *)"None");
    }
    else
    {
        /*
        ** send log message, header digest is mandatory
        */
        char tmp_buff[300] = {0};
        sprintf(tmp_buff, "iscsi Param %s set Mandatory Target id %d ", KEYNAMES[paramIndex], tid);
        iscsi_dprintf(tmp_buff);

    }
    return retVal;
}

/**
**  @name   RemoveAllListValuesForIndex
**  @brief  This function removes all list values  for a parameter
**  @param  INT32 paramIndex, PARAM_TABLE *paramTable
**
*/

void RemoveAllListValuesForIndex(INT32 paramIndex, PARAM_TABLE *paramTable)
{
    INT32 i=0;

    for (i=0; i<paramTable->index; i++)
    {
        if(paramTable->ptrParams[i].keyName == paramIndex)
        {
               /*
               ** This value should be removed from the list so we copy the last index content here and remove the last entry
               */
               paramTable->ptrParams[i].keyName = paramTable->ptrParams[paramTable->index-1].keyName;
               strcpy((char *)paramTable->ptrParams[i].keyVal, (char *)paramTable->ptrParams[paramTable->index-1].keyVal);
               paramTable->index--;
        }
    }

}

/*
@name    MakeParamNegotiationMandatory

@brief    Mandatory means, we don't have any value like "None", So remove None from the paramTable for the index

@param    INT32 paramIndex

@return     XIO_SUCCESS
         XIO_FAILURE
*/
UINT8  MakeParamNegotiationMandatory(INT32 paramIndex, PARAM_TABLE *paramTable, UINT8 *val UNUSED)
{
    UINT8 retVal = XIO_FAILURE;
    INT32 i = 0;
    for (i=0; i<paramTable->index; i++)
    {
        if(paramTable->ptrParams[i].keyName == paramIndex)
        {
            if(strcmp((char *)paramTable->ptrParams[i].keyVal, "None") == 0)
            {
                paramTable->ptrParams[i] = paramTable->ptrParams[paramTable->index -1];
                stringCopy(paramTable->ptrParams[paramTable->index -1].keyVal, paramTable->ptrParams[i].keyVal);
                paramTable->index--;
                retVal = XIO_SUCCESS;
            }

        }

    }
    return retVal;
}

/*
@name    iscsiSetListParameterValue

@brief    This function validates the value and then set the parameter table with the new value.
        This function sets value only for Digest and AuthMethod

@param

@return    ISCSI_INVALID_PARAM
    ISCSI_INVALID_VALUE
    ISCSI_OUT_OF_RANGE
    ISCSI_NO_SUPPORT
    DEOK
*/


UINT8 iscsiSetListParameterValue(INT32 paramIndex, UINT8 *paramVal UNUSED, INT32 option UNUSED, PARAM_TABLE *paramTable, UINT8 bitmap, UINT16 tid)
{
    UINT8 status = DEOK;
    if(paramIndex < 0 || paramIndex >= NUM_PARAMS)
    {
        return ISCSI_INVALID_PARAM;
    }

    RemoveAllListValuesForIndex(paramIndex, paramTable);
    status = EnableParamNegotiation(paramIndex, paramTable, bitmap, tid);

    return status;
}



/**
******************************************************************************
**              Local routines for testing the Code
******************************************************************************
**/
void printResponse (UINT8* resp, UINT32 pos)
{
    UINT32 i;
    UINT32 c = 0;
    fprintf(stderr, "\n");
    for (i = 0; i < pos; i++)
    {
        if (resp[i] == '\0')
        {
            fprintf (stderr, "\n");
            c++;
        }
        else
        {
            fprintf (stderr, "%c", resp[i]);
        }
    }
    fprintf(stderr, "%d values\n", c);
}

/**
******************************************************************************
** @name    iscsiGetLoggedInTargets
** @brief   This function gets the IP address of all Targets which are logged in
**          that means a session exist for the target
**
** @param   TGTDATA *
**
** @return  number of TgtData which contain valid information
**
******************************************************************************
**/

void printiscsiParams(PARAM_TABLE *ptrTable)
{
    int i=0;
    PARAMVALS *ptrParams = (ptrTable->ptrParams);
    for(i=0; i<ptrTable->index; i++)
    {
        fprintf(stderr, " %d %s=%s\n", ptrParams[i].keyName, KEYNAMES[ptrParams[i].keyName], ptrParams[i].keyVal);
    }
    fprintf(stderr, "\n========================iSCSI parameters end ======================================\n\n");
}

UINT64 iscsiGetNodeNameForTid(UINT16 tid)
{
    UINT64 retVal = 0xffffffff;
    TGD *pTgd = NULL;
    INT32 i=0;

    for(i=0; i< MAX_TARGETS; i++)
    {
        if(T_tgdindx[i] && T_tgdindx[i]->tid == tid)
        {
            pTgd = T_tgdindx[i];
#if defined(MODEL_7000) || defined(MODEL_4700)
            retVal = T_tgdindx[i]->nodeName;
            break;
#else  /* MODEL_7000 || MODEL_4700 */
            if (pTgd->cluster >= MAX_TARGETS || T_tgdindx[pTgd->cluster] == NULL)
            {
                if (T_tgdindx[i | 0x02] != NULL)
                {
                    retVal = T_tgdindx[i | 0x02]->nodeName;
                    break;
                }
            }
            else
            {
                retVal = T_tgdindx[pTgd->cluster]->nodeName;
                break;
            }
#endif /* MODEL_7000 || MODEL_4700 */
        }
    }

    return retVal;
}

void convertIntoReadableData(UINT64 nodeName, UINT8 *readableFormat)
{
    INT32 i;
    UINT8 *byteNodeName = (UINT8*)&nodeName;
    UINT8 number;
    UINT8 len = 16;


    for(i=0; i< len/2; i++)
    {
        number = ((byteNodeName[i] & 0xf0) >> 4);
        if(number > 9)
        {
            readableFormat[i*2] = 'A' + (number - 10);
        }else
        {
            readableFormat[i*2] = '0' + number;
        }
        number= byteNodeName[i] & 0x0f;
        if(number > 9)
        {
            readableFormat[i*2+1] = 'A' + (number - 10);
        }else
        {
            readableFormat[i*2+1] = '0' + number;
        }

    }
}
static INT32 getHexNumber(UINT8 ch)
{
    if(ch >= '0' && ch <= '9') return ch - '0';
    if(ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch - 'a' + 10;

    return 16;
}

INT32 convertTargetNameInTo64bit(UINT8 *string, INT32 size, UINT64 *pTargetName)
{
    INT32 i;
    UINT8 tmp = 0, nibble1, nibble2;
    UINT8 *tmpTarget = (UINT8*)pTargetName;
    /*first 4 bytes are naa.*/
    if(size != 20)
    {
        return -1;
    }

    for(i=4; i<size; i =i+2 )
    {
        if((nibble1=getHexNumber(string[i])) > 0xf
                || (nibble2=getHexNumber(string[i+1])) > 0xf)
        {
            return -1;
        }

        tmp = nibble2 | (nibble1<< 4);
        tmpTarget[(i/2) -2 ] = tmp;
    }
    return 1;
}


/*
   @name getiscsiNameForTarget
   @brief copies iscsi target name into given buffer for a target id
   @param UINT8 *pName, UINT32 tid

 */
void getiscsiNameForTarget(UINT8 *pName, UINT32 tid)
{
    UINT64 nodeName;
    UINT8 readableFormat[16]; /* 2 * sizeof UINT64*/

    strcpy((char *)pName, "naa.");
    /*find target name for this tid*/
    nodeName = iscsiGetNodeNameForTid(tid);
    convertIntoReadableData(nodeName, readableFormat);
    memcpy(pName + 4, readableFormat, 16);
    pName[20]=0;
}

/**
******************************************************************************
** @name    isTargetNameMatching
** @brief
**
**
** @param
**
** @return
**
******************************************************************************
**/
bool isTargetNameMatching(UINT8 *targetName, UINT16 targetPortalGroupTag UNUSED, UINT16 tid)
{
  UINT8 tmpTargetName[256];
  getiscsiNameForTarget(tmpTargetName, tid);
  if(strcasecmp((char *)targetName, (char *)tmpTargetName)== 0 )
  {
    return TRUE;
  }
  fprintf(stderr, "isTargetNameMatching: initiator sends %s we have %s for tid = 0x%x\n", targetName, tmpTargetName, tid);
  return FALSE;
}
/**
******************************************************************************
** @name    iscsiGetAllActiveTargets
** @brief   This function gets the IP address of all Targets which are currently
**          waiting for new connections
**
** @param   TGTDATA *
**
** @return  number of TgtData which contains valid information
**
******************************************************************************
**/

int iscsiGetAllActiveTargets(UINT16 port, TGTDATA *pTgtData)
{
    INT32 i;
    INT32 retVal=0;
    TGTDATA *pData = pTgtData;
    TGD    *pTGD;

    for(i=0; i<MAX_TARGETS; i++)
    {
        if(ICL_TARGET(i))
        {
          continue;
        }
        pTGD = T_tgdindx[i];
        pData = pTgtData + i;

        if(pTGD && (BIT_TEST(pTGD->opt, TARGET_ISCSI))
                    && (BIT_TEST(pTGD->opt, TARGET_ENABLE))
                    && ((pTGD->prefPort == port)
                    || (pTGD->altPort == port)))
        {
            getiscsiNameForTarget(pData->tgtName, pTGD->tid);
            strcpy((char *)pData->tgtAlias, (char *)pTGD->itgd->tgtAlias);
            pData->tgtIP[0] =  pTGD->ipAddr & 0x000000ff;
            pData->tgtIP[1] = (pTGD->ipAddr & 0x0000ff00) >> 8;
            pData->tgtIP[2] = (pTGD->ipAddr & 0x00ff0000) >> 16;
            pData->tgtIP[3] = (pTGD->ipAddr & 0xff000000) >> 24;
            pData->tgtPort =  ISCSI_DEFAULT_PORT;
            pData->tid =  pTGD->tid;
            pData->tgtPGT = pTGD->tid;
            pData->status = 1;
            retVal++;
        }
    }
    return retVal;
}
/**
******************************************************************************
** @name    iscsiGetAddressForTargetName
** @brief   This function gets the IP address of the target name given
**
** @param   UINT8 *pTargetName, TGTDATA *
**
** @return  0   TGTDATA contains invalid information
**          1 TGTDATA contains valid information
**
******************************************************************************
**/

int iscsiGetAddressForTargetName(UINT8 *pTargetName, TGTDATA *pTgtData)
{
    UINT64 nodeName;
    TAR *pTar;
    UINT8 ifaceNo = 0xff;
    INT32 status = 0;
    INT32 i=0;

    if(pTgtData == NULL)
    {
        return XIO_ZERO;
    }
    /* extract node name which is 8 byte long
    ** + 4 because of  "naa."
    **/

    nodeName = *((UINT64 *)(pTargetName + 4));
    /*
    **   Find if the name matches any target
    */

    for(i=0; i< MAX_TARGETS; i++)
    {
        pTar = tar[i];
        if(BIT_TEST(pTar->opt, TARGET_ISCSI))
        {
            if (T_tgdindx[i] && T_tgdindx[i]->nodeName == nodeName)
            {
                /*
                ** find the preferred port
                */
                ifaceNo = T_tgdindx[i]->prefPort;
                if(pTar->tid != T_tgdindx[i]->tid)
                {
                    /*
                    ** probably preferred port is failed
                    */
                    ifaceNo = T_tgdindx[i]->altPort;
                    /*
                    ** go to alternate port and try to find it out if the port is active for this target
                    */
                    pTar = pTar->fthd; /*because control port is never given to other Target*/
                    while(pTar != NULL)
                    {
                        if(pTar->tid == T_tgdindx[i]->tid)
                        {
                            status = 1;
                            break;
                        }
                        pTar = pTar->fthd;
                    }
                }else
                {
                    status = 1;
                }

                if(status > 0)
                {
                    pTgtData->tgtIP[0] =  pTar->ipAddr & 0x000000ff;
                    pTgtData->tgtIP[1] = (pTar->ipAddr & 0x0000ff00) >> 8;
                    pTgtData->tgtIP[2] = (pTar->ipAddr & 0x00ff0000) >> 16;
                    pTgtData->tgtIP[3] = (pTar->ipAddr & 0xff000000) >> 24;
                    pTgtData->tgtPort =  3260;
                    pTgtData->tid =  pTar->tid;
                    pTgtData->tgtPGT = 0;   /* it should be the interface no TODO */
                    fprintf(stderr, "IP=%d.%d.%d.%d\n", pTgtData->tgtIP[0], pTgtData->tgtIP[1], pTgtData->tgtIP[2], pTgtData->tgtIP[3]);
                    fprintf(stderr, "port=%d\n", pTgtData->tgtPort);
                    pTgtData->status = 1;
                    return status;
                }
            }
        }
    }
    return status;
}
/**
******************************************************************************
** @name    iscsiGenerateParameters
** @brief   This function generates text parameter from MRP,
**            Negotiation code use diffrent format other than stored in MRp so this conversion is
**            required
**
** @param   MRISCSITGT *pParamSrc, UINT32 setmap, PARAM_TABLE *paramTable
**
** @return  DEOK
**
******************************************************************************
**/


UINT8 iscsiGenerateParameters(I_TGD *pParamSrc)
{
    INT32 i=0;
    UINT8 string[32];
    INT32 intVal = -1;
    INT32 boolVal = -1;
    INT32 listVal = -1;
    INT32 paramIndex = 0;
    UINT16 tid;
    UINT8 bitmap=0;
    PARAMVALS *pParamDest = NULL;
    PARAM_TABLE *paramTable;
    UINT8 retCode = DEOK;
    INT32 status = 0;
    UINT8 port;

    tid = pParamSrc->tid;

    /*
    ** If parameter table doesnt exist already, allocate it
    */
    if(gTgtParams[tid] == NULL)
    {
        gTgtParams[tid] = (PARAM_TABLE *)s_MallocC(sizeof(PARAM_TABLE), __FILE__, __LINE__);
    }

    paramTable = (gTgtParams[tid]);

    pParamDest = paramTable->ptrParams;
    /*start from 3*/
    for(i = 3; i<= NUM_ISCSI_CONFIG_PARAM; i++)
    {
        intVal = -1;
        boolVal = -1;
        listVal = -1;
        switch(i)
        {
            /*
             ** integer value
             */
            case  KEY3_MAX_CONNECTIONS:
                paramIndex = MAXCONNECTIONSINDEX;
                intVal = pParamSrc->maxConnections;
                break;
            case  KEY10_ERROR_RECOVERY_LEVEL:
                paramIndex = ERRORRECOVERYLEVELINDEX;
                intVal = pParamSrc->errorRecoveryLevel;
                break;
            case  KEY12_MAX_BURST_LENGTH:
                paramIndex = MAXBURSTLENGTHINDEX;
                intVal = pParamSrc->maxBurstLength;
                break;
            case  KEY13_FIRST_BURST_LENGTH:
                paramIndex = FIRSTBURSTLENGTHINDEX;
                intVal = pParamSrc->firstBurstLength;
                break;
            case  KEY14_DEFAULT_TIME2_WAIT:
                paramIndex = DEFAULTTIME2WAITINDEX;
                intVal = pParamSrc->defaultTime2Wait;
                break;
            case  KEY15_DEFAULT_TIME2_RETAIN:
                paramIndex = DEFAULTTIME2RETAININDEX;
                intVal = pParamSrc->defaultTime2Retain;
                break;
            case  KEY11_TARGET_PORTAL_GROUP_TAG:
                paramIndex = TARGETPORTALGROUPTAGINDEX;
                intVal = pParamSrc->targetPortalGroupTag;
                intVal = tid;
                break;
            case  KEY16_MAX_OUTSTANDING_R2T:
                paramIndex = MAXOUTSTANDINGR2TINDEX;
                intVal = pParamSrc->maxOutstandingR2T;
                break;
            case  KEY17_MAX_RECV_DATASEGMENT_LENGTH:
                paramIndex = MAXRECVDATASEGMENTLENGTHINDEX;
                intVal = pParamSrc->maxRecvDataSegmentLength;
                break;
            case  KEY18_IFMARK_INT:
                paramIndex = IFMARKINTINDEX;
                intVal = pParamSrc->ifMarkInt;
                break;
            case  KEY19_OFMARK_INT:
                paramIndex = OFMARKINTINDEX;
                intVal = pParamSrc->ofMarkInt;
                break;
                /*
                 ** boolean value
                 */
            case  KEY4_INITIAL_R2T:
                paramIndex = INITIALR2TINDEX;
                boolVal = pParamSrc->initialR2T;
                break;
            case  KEY5_IMMEDIATE_DATA:
                paramIndex = IMMEDIATEDATAINDEX;
                boolVal = pParamSrc->immediateData;
                break;
            case  KEY6_DATA_SEQUENCE_IN_ORDER:
                paramIndex = DATASEQUENCEINORDERINDEX;
                boolVal = pParamSrc->dataSequenceInOrder;
                break;
            case  KEY7_DATA_PDU_IN_ORDER:
                paramIndex = DATAPDUINORDERINDEX;
                boolVal = pParamSrc->dataPDUInOrder;

                break;
            case  KEY8_IF_MARKER:
                paramIndex = IFMARKERINDEX;
                boolVal = pParamSrc->ifMarker;
                break;
            case  KEY9_OF_MARKER:
                paramIndex = OFMARKERINDEX;
                boolVal = pParamSrc->ofMarker;
                break;
                /*
                 ** list case
                 */
            case  KEY20_HEADER_DIGEST:
                paramIndex = HEADERDIGESTINDEX;
                bitmap = pParamSrc->headerDigest;
                listVal=1;
                break;
            case  KEY21_DATA_DIGEST:
                paramIndex = DATADIGESTINDEX;
                bitmap = pParamSrc->dataDigest;
                listVal=1;
                break;

            case  KEY22_AUTHMETHOD:
                paramIndex = AUTHMETHODINDEX;
                bitmap = pParamSrc->authMethod;
                listVal=1;
                break;
            case  KEY25_MAX_SEND_DATASEGMENT_LENGTH:
                paramIndex = MAXSENDDATASEGMENTLENGTHINDEX;
                intVal = pParamSrc->maxSendDataSegmentLength;
                break;
            case KEY23_MTUSIZE:
            /*
            ** Call tsl routine for setting MTU size over a port
            */
                port = getPortFromTid(tid);
                status = tsl_set_mtu(port, pParamSrc->mtuSize);
                if(status == -1)
                {
                    retCode = DESETMTUFAIL;
                }
                break;
            case KEY24_TGTALIAS:
                break;

            default:
                break;
        }
        if(listVal == 1)
        {
             iscsiSetListParameterValue(paramIndex, NULL, 1, paramTable, bitmap, tid);
        }
        else
        {
            if(intVal != -1)
            {
                i2a(intVal, string);
            }
            if(boolVal == 0)
            {
                strcpy((char *)string, "No");
            }
            if(boolVal > 0)
            {
                strcpy((char *)string, "Yes");
            }
            /*iscsiSetParameterValue(paramIndex, string, pParamDest); */
            iscsiSetParameterValue(paramIndex, string, paramTable);
        }

    }

    return (retCode);
}/*end of iscsiGenerateParameters*/

/**
******************************************************************************
** @name    iSCSIAddUser
** @brief   This function configures CHAP user info for a target
**
** @param   MRISCSITGT *pParamSrc, UINT32 setmap, PARAM_TABLE *paramTable
**
** @return  DEOK
**
******************************************************************************
**/
UINT8 iSCSIAddUser(MRCHAPCONFIG userInfo)
{
    UINT8 status = DEOK;
    CHAPINFO* pUserInfo;

    if(gTgtParams[userInfo.tid] == NULL)
    {
        return(DEINVTID);
    }

    if (gTgtParams[userInfo.tid]->userCount == 0)
    {
        pUserInfo = (CHAPINFO *)s_MallocC(sizeof(CHAPINFO), __FILE__, __LINE__);
        pUserInfo->fthd = NULL;
        stringCopy(userInfo.sname, pUserInfo->sname);
        if (userInfo.opt == 0)
        {
        stringCopy(userInfo.secret1, pUserInfo->secret1);
        }
        else if (userInfo.opt == 1)
        {
            stringCopy(userInfo.secret1, pUserInfo->secret2);
        }
        else if (userInfo.opt == 2)
        {
        stringCopy(userInfo.secret2, pUserInfo->secret2);
            stringCopy(userInfo.secret1, pUserInfo->secret1);
        }
        gTgtParams[userInfo.tid]->pUserInfo = pUserInfo;
        gTgtParams[userInfo.tid]->userCount = 1;
        return (status);
    }

    pUserInfo = gTgtParams[userInfo.tid]->pUserInfo;
    while(1)
    {
        if(stringCompare(pUserInfo->sname, userInfo.sname) == 0)
        {
            /*
            ** Modify
            */
            if (userInfo.opt == 0)
            {
            stringCopy(userInfo.secret1, pUserInfo->secret1);
            }
            else if (userInfo.opt == 1)
            {
                stringCopy(userInfo.secret1, pUserInfo->secret2);
            }
            else if (userInfo.opt == 2)
            {
            stringCopy(userInfo.secret2, pUserInfo->secret2);
                stringCopy(userInfo.secret1, pUserInfo->secret1);
            }
            break;
        }
        if(pUserInfo->fthd == NULL)
        {
            /*
            ** Add
            */
            pUserInfo->fthd = (CHAPINFO *)s_MallocC(sizeof(CHAPINFO), __FILE__, __LINE__);
            pUserInfo = pUserInfo->fthd;
            pUserInfo->fthd = NULL;
            stringCopy(userInfo.sname, pUserInfo->sname);
            if (userInfo.opt == 0)
            {
                stringCopy(userInfo.secret1, pUserInfo->secret1);
            }
            else if (userInfo.opt == 1)
            {
                stringCopy(userInfo.secret1, pUserInfo->secret2);
            }
            else if (userInfo.opt == 2)
            {
                stringCopy(userInfo.secret2, pUserInfo->secret2);
                stringCopy(userInfo.secret1, pUserInfo->secret1);
            }
            gTgtParams[userInfo.tid]->userCount++;
            break;
        }
        pUserInfo = pUserInfo->fthd;
    }

    return (status);
}

/**
******************************************************************************
** @name    iSCSIRemoveUser
** @brief   This function removes CHAP user info for a target
**
** @param   MRISCSITGT *pParamSrc, UINT32 setmap, PARAM_TABLE *paramTable
**
** @return  DEOK
**
******************************************************************************
**/
UINT8 iSCSIRemoveUser(MRCHAPCONFIG userInfo)
{
    UINT8 status = DEOK;
    CHAPINFO* pUserInfo;
    CHAPINFO* pUserInfoLoc;

    if (gTgtParams[userInfo.tid] == NULL)
    {
        return(DEINVTID);
    }

    if (gTgtParams[userInfo.tid]->userCount == 0)
    {
        return(status);
    }

    pUserInfo = gTgtParams[userInfo.tid]->pUserInfo;
    if(stringCompare(pUserInfo->sname, userInfo.sname) == 0)
    {
        gTgtParams[userInfo.tid]->pUserInfo = pUserInfo->fthd;
        gTgtParams[userInfo.tid]->userCount--;

        s_Free((void *)pUserInfo, sizeof(CHAPINFO), __FILE__, __LINE__);
        if(gTgtParams[userInfo.tid]->userCount == 0)
        {
            gTgtParams[userInfo.tid]->pUserInfo = NULL;
        }
        return(status);
    }

    while(1)
    {
        if(pUserInfo->fthd == NULL)
        {
            break;
        }
        if(stringCompare(pUserInfo->fthd->sname, userInfo.sname) == 0)
        {
            pUserInfoLoc = pUserInfo->fthd;
            /*
            ** Remove
            */
            if(pUserInfo->fthd->fthd == NULL)
            {
                pUserInfo->fthd = NULL;
            }
            else
            {
                pUserInfo->fthd = pUserInfo->fthd->fthd;
            }
            s_Free((void *)pUserInfoLoc, sizeof(CHAPINFO), __FILE__, __LINE__);
            gTgtParams[userInfo.tid]->userCount--;
            if(gTgtParams[userInfo.tid]->userCount == 0)
            {
                gTgtParams[userInfo.tid]->pUserInfo = NULL;
            }
            break;
        }
        pUserInfo = pUserInfo->fthd;
    }

    return (status);
}

/**
******************************************************************************
** @name    chapGetSecret
** @brief   This function gets the initiator and target secret for the given tid
**
** @param   UINT16 tid,
**          UINT8 *name, - initiator name
**          UINT8 *secret1, -secret used by target to compute response while authenticating initiator
**          UINT8 *secret2 - secret used by target to compute response while sending response
**
** @return  1 if found
**          -1 if not found
**
******************************************************************************
**/
int chapGetSecret (UINT16 tid, UINT8 *name, UINT8 *secret1, UINT8* secret2)
{
    INT32 status = -1;
    CHAPINFO* ptmp;

    if (gTgtParams[tid] == NULL)
    {
        return(status);
    }

    if (gTgtParams[tid]->userCount == 0)
    {
        return(status);
    }

    ptmp = gTgtParams[tid]->pUserInfo;

    while(ptmp != NULL)
    {
        if(strcasecmp((char *)ptmp->sname, (char *)name) == 0)
        {
             if(secret1)
             {
                strcpy((char *)secret1, (char *)ptmp->secret1);
             }
             if(secret2)
             {
                strcpy((char *)secret2, (char *)ptmp->secret2);
             }

             status = 1;
             break;
        }
        ptmp = ptmp->fthd;
    }
    return status;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
