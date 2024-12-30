/* $Id: iscsi_digest.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
 ******************************************************************************
 **
 **  @file       iscsi_digest.h
 **
 **  @brief      iSCSI Digest functions header file
 **
 **  This file provides API's for Header and data digest in iSCSI
 **
 **  Copyright (c) 2005-2010 XIOtech Corporation.  All rights reserved.
 **
 ******************************************************************************
 **/
#ifndef ISCSI_DIGEST_H
#define ISCSI_DIGEST_H

#define DIGEST_LENGTH    4

/**
 * @name iscsi_CalculateCRC32
 * @params
 *     char *message - message
 *     unsigned length - length of message
 * @brief  The function calculates the digest and returns.
 *             before sending it on connection it is needed to
 *             be converted in network byte order
 *
 * @return
 *    -   digest
 */

extern unsigned long iscsi_CalculateCRC32(const unsigned char *message,unsigned long length);

extern int iscsi_isDigestError(const unsigned char *message, unsigned long length,unsigned long digest);

extern UINT32 isDigestCheckValid(ISCSI_TPD *pTPD);
extern int    ReadDataDigestAndCheck(CONNECTION *pConn, SGL_DESC* data, UINT8 sglCount, UINT32 start, UINT32 end);
extern INT32  HandleDataDigestError(CONNECTION *pConn,ISCSI_GENERIC_HDR *pRejectedHdr);
extern void   AppendHeaderDigest(ISCSI_TPD *pTPD, char* buffer, int *totalLength);

extern unsigned long iscsi_CalculateCRC32_sgl(SGL_DESC *pSgl, UINT32 sglCount, UINT32 start, UINT32 end);

#endif  /* ISCSI_DIGEST_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
