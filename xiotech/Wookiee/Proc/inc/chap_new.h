/* $Id: chap_new.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       chap_new.h
**
**  @brief      CHAP (Challenge Handshake Authentication Protocol) based on
*               RFC 1994. Intended for use with iSCSI only.
**
**  Copyright (c) 2005 - 2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#ifndef _ISCSI_CHAP_NEW_H_
#define _ISCSI_CHAP_NEW_H_

/*
** these include all sorts of headers needed for MD5.
*/
#include "chap_md5global.h"
#include "chap_md5.h"

enum
  {
    CHAP_FALSE                   =   0,
    CHAP_TRUE                    =   1,
    CHAP_ID_SIZE_BYTES      =   1,
    CHAP_MAX_ID             = 256, /* id = [0, 256) */
    CHAP_MD5_DIGEST_LEN     =  16,
    CHAP_MIN_CHAL_LEN       =  16, /* todo */
    CHAP_MAX_CHAL_LEN       =  16, /* todo */
    CHAP_MAX_NAME_LEN       =  256, /* todo */
    CHAP_MAX_SECRET_LEN     =  16, /* todo */
    CHAP_MD5_ALGORITHM_CODE =   5, /* todo */
    CHAP_MAX_CHAL_RECVD_LEN = 1024,
    /*
    ** worst case occurs for base-16, which needs twice the number of bytes
    ** and a prefix of 0x or 0X. This argument assumes that md5 len is not a
    ** very small number like 0, 1, 2 etc.
    */
    CHAP_MAX_RESP_RECVD_LEN = CHAP_MD5_DIGEST_LEN * 2 + 2,
    CHAP_MAX_RESP_EXPECTED_LEN = CHAP_MD5_DIGEST_LEN,
    CHAP_MIN_RESP_EXPECTED_LEN = CHAP_MD5_DIGEST_LEN,

    CHAP_JUNK_UINT16        = 8888,
    CHAP_JUNK_UINT8         = 77,
    CHAP_JUNK_INT           = 5555555,
    CHAP_JUNK_CHAR          = (int) 'v',
    CHAP_JUNK_POINTER       = 6666666,
    CHAP_CONTEXT_COOKIE     = 1111111,
  };

/*
** contains values in host byte order, as opposed to network byte order.
*/
typedef struct chap_context_st
{
  int    cookie;

  UINT8  id;
  int    algorithm;

  UINT8  name[CHAP_MAX_NAME_LEN];
  int    name_len;

  UINT8  chal[CHAP_MAX_CHAL_LEN];
  int    chal_len;

  UINT8  resp_recvd_encoded[CHAP_MAX_RESP_RECVD_LEN];
  int    resp_recvd_len;

  UINT8  resp_expected_bin[CHAP_MAX_RESP_EXPECTED_LEN];
  int    resp_expected_len;

  UINT8  chal_recvd_decoded[CHAP_MAX_CHAL_RECVD_LEN +1];
  int    chal_recvd_len;
  UINT8    id_recvd;
  UINT8    state;
  UINT16   tid;
} CHAP_CONTEXT_ST;

/*
** different state of CHAP
**
*/

#define CHAP_INIT 0
#define CHAP_CHAL_SENT 1
#define CHAP_RESP_RCVD 2
#define CHAP_CHAL_RCVD 3
#define CHAP_COMPLETE  4

/*
** start of logging code
** change 0 to 1 to disable logs.
*/
/* #if 1 */
/* #define chap_log_low myvoid */
/* #else */
/* #define chap_log_low printf */
/* #endif */

/* #if 0 */
/* #define chap_log_mid myvoid */
/* #else */
/* #define chap_log_mid printf */
/* #endif */

#if 0
#define chap_log_high(a)
#define chap_log_high(a,b)
#define chap_log_high(a,b,c)
#define chap_log_high(a,b,c,d)
#define chap_log_high(a,b,c,d,e)
#else
#define chap_log_high printf
#endif

#if 0
#define chap_log_mid(a)
#define chap_log_mid(a,b)
#define chap_log_mid(a,b,c)
#define chap_log_mid(a,b,c,d)
#define chap_log_mid(a,b,c,d,e)
#else
#define chap_log_mid printf
#endif

#if 0
#define chap_log_low(a)
#define chap_log_low(a,b)
#define chap_log_low(a,b,c)
#define chap_log_low(a,b,c,d)
#define chap_log_low(a,b,c,d,e)
#else
#define chap_log_low printf
#endif

#if 0
static inline void  myvoid (char *dummy, ...)
{
  char *junk = dummy;
  if (!junk)
    junk = dummy;
}
#endif /* 0 */

/*
** end of logging code
*/

/*
** global functions
*/
extern CHAP_CONTEXT_ST *chap_create_context_st(void);
extern void chap_release_context_st (CHAP_CONTEXT_ST *cc);
extern int chap_is_valid_resp (CHAP_CONTEXT_ST *cc);
extern int chap_create_chal (CHAP_CONTEXT_ST *cc
                      , UINT8 *buf
                      , int   *p_valid_len
                      , int    encoding_base);
extern int chap_test (void);

extern int decode_string(UINT8* encoded, INT32 encoded_length, UINT8* decoded,int *decoded_length );

extern int chap_create_response(CHAP_CONTEXT_ST *,UINT8* buf,int *p_valid_len,int encoding_base,UINT8 *targetName);

#endif /* _ISCSI_CHAP_NEW_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
