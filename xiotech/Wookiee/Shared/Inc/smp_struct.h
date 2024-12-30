/* $Id: smp_struct.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
 ******************************************************************************
 **
 **  @file       smp_structs.h
 **
 **  @brief      smp strcutures and defines
 **
 **  Copyright (c) 2006-2009 XIOtech Corporation. All rights reserved.
 **
 ******************************************************************************
 **/
#ifndef _SMP_STRUCTS_H_
#define _SMP_STRUCTS_H_

#include "system.h"
#include "XIO_Types.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define SMP_REPORT_GENERAL_SIZE              8
#define SMP_REPORT_GENERAL_RSP_SIZE         32

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
typedef struct SMP_REPORT_GENERAL
{
    UINT8   frametype;
    UINT8   function;
    UINT16  rsv;
    UINT32  crcplaceholder;          /*need space in buf but has no real use*/
} SMP_REPORT_GENERAL, *PSMP_REPORT_GENERAL;

#define CONFIGUABLE_ROUTE_TABLE 0x01
#define CONFIGURING 0x02
typedef struct SMP_REPORT_GENERAL_RSP
{
    UINT8   frametype;
    UINT8   function;
    UINT8   result;
    UINT8   rsv0;
    UINT16  changecount;
    UINT16  routeindexes;
    UINT8   rsv1;
    UINT8   numberofphys;
    UINT8   configflags;
    UINT8   rsv2;
    UINT64  enclosureidentifier;
    UINT8   rsv3[8];
    UINT32  crcplaceholder;          /*need space in buf but has no real use*/
} SMP_REPORT_GENERAL_RSP, *PSMP_REPORT_GENERAL_RSP;

typedef struct SMP_REPORT_MAN_INFO
{
    UINT8   frametype;
    UINT8   function;
    UINT8   rsv[2];
    UINT32  crcplaceholder;          /*need space in buf but has no real use*/
} SMP_REPORT_MAN_INFO, *PSMP_REPORT_MAN_INFO;

#define SAS_11_FORMAT
typedef struct SMP_REPORT_MAN_INFO_RSP
{
    UINT8   frametype;
    UINT8   function;
    UINT8   result;
    UINT8   rsv0[6];
    UINT8   formatflags;
    UINT8   rsv1[3];
    char    vendorid[8];
    char    productid[16];
    char    productrev[4];
    char    componentvendorid[8];
    UINT16  componentid;
    UINT8   componentrevid;
    UINT8   rsv2;
    UINT8   vendorspecific[8];
    UINT32  crcplaceholder;          /*need space in buf but has no real use*/
} SMP_REPORT_MAN_INFO_RSP, *PSMP_REPORT_MAN_INFO_RSP;

typedef struct SMP_DISCOVER
{
    UINT8   frametype;
    UINT8   function;
    UINT8   rsv[7];
    UINT8   phyident;
    UINT8   rsv1[2];
    UINT32  crcplaceholder;          /*need space in buf but has no real use*/
} SMP_DISCOVER, *PSMP_DISCOVER;

#define SAS_DEV_TYPE_MASK       0x70
#define SAS_DEV_TYPE_NONE       0x00
#define SAS_DEV_TYPE_END_DEVICE 0x01
#define SAS_DEV_TYPE_EDGE_EXP   0x02
#define SAS_DEV_TYPE_FAN_EXP    0x03
#define SAS_LINK_RATE_MASK      0x0F
#define SAS_LINK_RATE_UNKNOWN   0x00
#define SAS_LINK_RATE_DISABLED  0X01
#define SAS_LINK_RATE_RESET_PROB    0X02
#define SAS_LINK_RATE_SPINUP_HOLD   0X03
#define SAS_LINK_RATE_PORT_SEL      0X04
#define SAS_LINK_RATE_G1            0X08
#define SAS_LINK_RATE_G2            0X09
#define SAS_SSP_INITIATOR           0x08
#define SAS_STP_INITIATOR           0x04
#define SAS_SMP_INITIATOR           0x02
#define SAS_SATA_HOST               0x01
#define SAS_SSP_TARGET              0x08
#define SAS_STP_TARGET              0x04
#define SAS_SMP_TARGET              0x02
#define SAS_SATA_DEVICE             0x01
#define SATA_ATTACHED_PORT_SEL      0x80
typedef struct SMP_DISCOVER_RSP
{
    UINT8   frametype;
    UINT8   function;
    UINT8   result;
    UINT8   rsv0[6];
    UINT8   phyident;
    UINT8   rsv1[2];
    UINT8   devtype;
    UINT8   negotiatedlinkrate;
    UINT8   initiatorflags;
    UINT8   targetflags;
    UINT64  SASaddress;
    UINT64  attachedSASaddress;
    UINT8   attachedphyident;
    UINT8   rsv2[7];
    UINT8   minlinkrate;
    UINT8   maxlinkrate;
    UINT8   phychangecount;
    UINT8   patialpathtov;
    UINT8   routingattribute;
    UINT8   connectortype;
    UINT8   connectorelementindex;
    UINT8   connectorphysicallink;
    UINT8   rsv3[2];
    UINT8   vendorspecific[2];
    UINT32  crcplaceholder;          /*need space in buf but has no real use*/
} SMP_DISCOVER_RSP, *PSMP_DISCOVER_RSP;

typedef struct SMP_PHY_ERROR_LOG
{
    UINT8   frametype;
    UINT8   function;
    UINT8   rsv[7];
    UINT8   phyident;
    UINT8   rsv1[2];
    UINT32  crcplaceholder;          /*need space in buf but has no real use*/
} SMP_PHY_ERROR_LOG, *PSMP_PHY_ERROR_LOG;

typedef struct SMP_PHY_ERROR_LOG_RSP
{
    UINT8   frametype;
    UINT8   function;
    UINT8   rsv[6];
    UINT8   phyident;
    UINT8   rsv1[2];
    UINT32  invaliddwordcount;
    UINT32  runningdisparitycount;
    UINT32  lossofsynchcount;
    UINT32  resetproblemcount;
    UINT32  crcplaceholder;          /*need space in buf but has no real use*/
} SMP_PHY_ERROR_LOG_RSP, *PSMP_PHY_ERROR_LOG_RSP;

#endif

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
