/* $Id: datagram.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       datagram.h
**
**  @brief      Cache Information
**
**  Copyright (c) 1996-2010 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _DATAGRAM_H_
#define _DATAGRAM_H_

#include "XIO_Types.h"                  /* Xiotech Standard Types           */

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define DG_CPU_INTERFACE 0              /* Interface processor              */
#define CAC0_FC_WRTMEM   0              /* Write Memory                     */
#define CAC0_FC_RDMEM    1              /* Read Memory                      */
#define DG_PATH_ANY      0xFF           /* any available path code          */
#define CAC0_NAME        0x30434143     /* CAC0 server name in ASCII        */

#define CAC0_WCCT        0              /* Write Cache Control Table        */
#define CAC0_WCC         1              /* Write Cache Configuration        */
#define CAC0_BBD         2              /* Battery Backup (diagnostic) Data */
#define CAC0_TAG         4              /* Write Cache Tag                  */
#define CAC0_DATA        5              /* Write Cache Data                 */

/*
** --- Request completion status (Datagram Response status) definitions ----
*/
#define DG_ST_OK         0              /* Successful                       */

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/
#define DATAGRAM_REQ_SIZE   (sizeof(struct DATAGRAM_REQ))
#define DATAGRAM_RSP_SIZE   (sizeof(struct DATAGRAM_RSP))
#define CAC0_ENTRY_LEN      (sizeof(struct CAC0))

/*
# --- DLM1 Server Function Code Definitions ----------------------------------
*/
#define    DLM1_fc_polldgp  0       /* Poll a Datagram Path */
/*
#       DLM1 server name
*/

#define DLM1name 0x314d4c44     /*# DLM1 server name in ASCII (little end) */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/*
** Begin request message header structure
*/
typedef struct DATAGRAM_REQ
{
    UINT8  srvCPU;                      /* server processor code        <b> */
    UINT8  hdrLen;                      /* request header length        <b> */
    UINT16 seq;                         /* message sequence #           <s> */
    UINT8  fc;                          /* request function code        <b> */
    UINT8  path;                        /* path #                       <b> */
    UINT8  gpr0;                        /* general purpose reg. #0      <b> */
    UINT8  gpr1;                        /* general purpose reg. #1      <b> */
    UINT32 srcSN;                       /* source serial #              <w> */
    UINT32 dstSN;                       /* destination serial #         <w> */
    UINT32 srvName;                     /* dest. server name            <w> */
    UINT32 reqLen;                      /* remaining message length     <w> */
    UINT8  gpr2;                        /* general purpose reg. #2      <b> */
    UINT8  gpr3;                        /* general purpose reg. #3      <b> */
    UINT8  gpr4;                        /* general purpose reg. #4      <b> */
    UINT8  gpr5;                        /* general purpose reg. #5      <b> */
    UINT32 crc;                         /* request message header CRC   <w> */
} DATAGRAM_REQ;

/*
** Begin response message header structure
*/
typedef struct DATAGRAM_RSP
{
    UINT8  status;                      /* request completion status    <b> */
    UINT8  hdrLen;                      /* response header length       <b> */
    UINT16 seq;                         /* message sequence #           <s> */
    UINT8  ec1;                         /* error code byte #1           <b> */
    UINT8  ec2;                         /* error code byte #2           <b> */
    UINT8  gpr0;                        /* general purpose reg. #0      <b> */
    UINT8  gpr1;                        /* general purpose reg. #1      <b> */
    UINT32 respLen;                     /* remaining message length     <w> */
    UINT32 crc;                         /* response message header CRC  <w> */
} DATAGRAM_RSP;

typedef struct CAC0
{
    void * rqMemAddr;                   /* Memory Address to write/read <w> */
    UINT32 rqMemLen;                    /* Memory Length to write/read  <w> */
} CAC0;

#endif /* _DATAGRAM_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
