/* $Id: i82559.h 126080 2010-02-03 22:59:28Z mdr $ */
/*============================================================================
** FILE NAME:       i82559.h
** MODULE TITLE:    Header file for i82559.c
**
** Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#ifndef _I82559_H_
#define _I82559_H_

#include "convert.h"
#include "pcb.h"
#include "XIO_Const.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

typedef struct ETHERNET_DRIVER_STRUCT
{
    PCB     *pcbPtr;
    void    *interfaceHandle;
} ETHERNET_DRIVER;

typedef struct ETHERNET_MAC_ADDRESS_STRUCT
{
    UINT8       macByte[6];
} ETHERNET_MAC_ADDRESS;

/* General Status Register byte */
typedef struct GENERAL_STATUS_BYTE_BITS_STRUCT
{
    UINT8       linkStatus:1;   /* Bit 0    */
    UINT8       wireSpeed:1;    /* Bit 1    */
    UINT8       duplexMode:1;   /* Bit 2    */
    UINT8       reserved:5;     /* Bits 3-7 */
} GENERAL_STATUS_BYTE_BITS;

typedef enum _LINK_STATUS
{
    LINK_STATUS_DOWN = 0,
    LINK_STATUS_UP = 1
} LINK_STATUS;

typedef enum _WIRE_SPEED
{
    WIRE_SPEED_10_MBPS = 0,
    WIRE_SPEED_100_MBPS = 1
} WIRE_SPEED;

typedef enum _DUPLEX_MODE
{
    DUPLEX_MODE_HALF = 0,
    DUPLEX_MODE_FULL = 1
} DUPLEX_MODE;

typedef union GENERAL_STATUS_BYTE_UNION
{
    GENERAL_STATUS_BYTE_BITS bits;
    UINT8       value;
} GENERAL_STATUS_BYTE;

/*
** Ethernet link status structure - hardware independent
*/
typedef struct _ETHERNET_LINK_STATUS_BITS_STRUCT
{
    UINT8       linkStatus:1;   /* Bit 0    */
    UINT8       wireSpeed:1;    /* Bit 1    */
    UINT8       duplexMode:1;   /* Bit 2    */
    UINT8       reserved:4;     /* Bits 3-6 */
    UINT8       linkStatusChange:1;     /* Bit 7    */
} ETHERNET_LINK_STATUS_BITS;

typedef union _ETHERNET_LINK_STATUS_UNION
{
    ETHERNET_LINK_STATUS_BITS bits;
    UINT8       value;
} ETHERNET_LINK_STATUS;

/* Statistical Counters structure */
typedef struct ETHERNET_STATISTIC_COUNTERS_STRUCT
{
    /*
     ** These statistics are gotten directly from /proc/net/dev.
     */
    char        eyecatcher[8];
    char        interface[8];

    UINT32      rxBytes;
    UINT32      rxPackets;
    UINT32      rxErrs;
    UINT32      rxDrop;

    UINT32      rxFifo;
    UINT32      rxFrame;
    UINT32      rxCompressed;
    UINT32      rxMulticast;

    UINT32      txBytes;
    UINT32      txPackets;
    UINT32      txErrs;
    UINT32      txDrop;

    UINT32      txFifo;
    UINT32      txColls;
    UINT32      txCarrier;
    UINT32      txCompressed;
} ETHERNET_STATISTIC_COUNTERS;


/*****************************************************************************
** Public variables
*****************************************************************************/
extern ETHERNET_DRIVER ethernetDriver;
extern UINT32 stackDmpFull;
extern char stackDmp[];

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

/*----------------------------------------------------------------------------
** FUNCTION NAME:   EthernetGetStatistics
**
** DESCRIPTION:
**
** INPUTS:          ethernetStatisticsPtr
**                  resetCountersFlag: TRUE = clear internal counters after fetch
**
** RETURNS:         GOOD  -
**                  ERROR -
**--------------------------------------------------------------------------*/
UINT32 EthernetGetStatistics(ETHERNET_STATISTIC_COUNTERS *ethernetStatistics,
                    UINT32 resetCountersFlag);

/*****************************************************************************
** FUNCTION NAME:   EthernetLinkMonitor
**
** PARAMETERS:      None
**
** DESCRIPTION:     This function monitors the state of the i82559's link
**                  indicators.
**
** RETURNS:         GOOD  - Link values read and copied into
**                          structure pointed to by ethernetLinkStatusPtr
**                  ERROR - Error reading/saving link properties and the
**                          link values returned are not accurate
******************************************************************************/
extern UINT32 EthernetLinkMonitor(ETHERNET_LINK_STATUS *ethernetLinkStatus);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _I82559_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
