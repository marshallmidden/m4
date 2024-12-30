/* $Id: nvram.h 143845 2010-07-07 20:51:58Z mdr $ */
/*============================================================================
** FILE NAME:       nvram.h
** MODULE TITLE:    Header file for nvram.c
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _NVRAM_H_
#define _NVRAM_H_

#include "XIO_Macros.h"
#include "EL_Disaster.h"
#include "mutex.h"
#include "quorum.h"
#include "XIO_Types.h"
#include "ISP_Defs.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
#define CONTROLLER_SETUP_SCHEMA             1

#define CntlSetup_GetSchema()               ( cntlSetup.schema )

#define CntlSetup_GetIPAddress()            ( cntlSetup.ipAddress )
#define CntlSetup_SetIPAddress(addr)        { \
                                                cntlSetup.ipAddress = addr; \
                                                SaveControllerSetup(); \
                                            }

#define CntlSetup_GetGatewayAddress()       ( cntlSetup.gatewayAddress )
#define CntlSetup_SetGatewayAddress(addr)   { \
                                                cntlSetup.gatewayAddress = addr; \
                                                SaveControllerSetup(); \
                                            }

#define CntlSetup_GetSubnetMask()           ( cntlSetup.subnetMask )
#define CntlSetup_SetSubnetMask(mask)       { \
                                                cntlSetup.subnetMask = mask; \
                                                SaveControllerSetup(); \
                                            }

#define CntlSetup_GetSystemSN()             ( cntlSetup.systemSN )
#define CntlSetup_SetSystemSN(sn)           { \
                                                cntlSetup.systemSN = sn; \
                                                SaveControllerSetup(); \
                                            }

#define CntlSetup_GetControllerSN()         ( cntlSetup.controllerSN )
#define CntlSetup_SetControllerSN(sn)       { \
                                                cntlSetup.controllerSN = sn; \
                                                SaveControllerSetup(); \
                                            }

#define GetDebugConsoleIPAddr()             ( cntlSetup.debugConsoleIPAddr )
#define SetDebugConsoleIPAddr(addr)         { \
                                                cntlSetup.debugConsoleIPAddr = (addr); \
                                                SaveControllerSetup(); \
                                            }

#define GetDebugConsoleChannel()            ( cntlSetup.debugConsoleChannel )
#define SetDebugConsoleChannel(chan)        { \
                                                cntlSetup.debugConsoleChannel = (chan); \
                                                SaveControllerSetup(); \
                                            }

/* Config Flags - Bit Definitions */
#define CF_LICENSE_APPLIED                  0
#define CF_INIT_LOGS                        1
#define CF_REPLACEMENT_CONTROLLER           2
#define CF_INIT_LOGS_MASK                   0x02

/* Config Flags - Helper Macros */
#define ConfigFlagsBitTst(a)                BIT_TEST(cntlSetup.configFlags, a)
#define ConfigFlagsBitSet(a)                { \
                                                BIT_SET(cntlSetup.configFlags, a); \
                                                SaveControllerSetup(); \
                                            }
#define ConfigFlagsBitClr(a)                { \
                                                BIT_CLEAR(cntlSetup.configFlags, a); \
                                                SaveControllerSetup(); \
                                            }

#define IsLicenseApplied()                  ConfigFlagsBitTst(CF_LICENSE_APPLIED)
#define SetLicenseApplied()                 ConfigFlagsBitSet(CF_LICENSE_APPLIED)
#define ClearLicenseApplied()               ConfigFlagsBitClr(CF_LICENSE_APPLIED)

#define IsReplacementController()           ConfigFlagsBitTst(CF_REPLACEMENT_CONTROLLER)
#define SetReplacementController()          ConfigFlagsBitSet(CF_REPLACEMENT_CONTROLLER)
#define ClearReplacementController()        ConfigFlagsBitClr(CF_REPLACEMENT_CONTROLLER)

#define GetIPAddress()                      CntlSetup_GetIPAddress()
#define SetIPAddress(addr)                  CntlSetup_SetIPAddress(addr)

#define GetSubnetMask()                     CntlSetup_GetSubnetMask()
#define SetSubnetMask(mask)                 CntlSetup_SetSubnetMask(mask)

#define GetGatewayAddress()                 CntlSetup_GetGatewayAddress()
#define SetGatewayAddress(addr)             CntlSetup_SetGatewayAddress(addr)

#define GetEthernetConfigured()             ( cntlSetup.ethernetConfigured )
#define SetEthernetConfigured(flag)         { \
                                                cntlSetup.ethernetConfigured = flag; \
                                                SaveControllerSetup(); \
                                            }

/*****************************************************************************
** Public structures
*****************************************************************************/

/*
 * Port Configuration structure
 */

typedef struct
{
    ISP_CONFIG  be;
    ISP_CONFIG  fe;
} PORT_CONFIG;

/*
** Controller Configuration structure
**
** This structure must remain 476 bytes.  It is a copy of information
** stored in the NVRAM.  This can change but requires the NVRAM image
** to change which is a headache.
*/
typedef struct
{
    UINT32      schema;         /* schema version for setup       */
    UINT8       rsvd1[4];       /* RESERVED                       */
    IP_ADDRESS  ipAddress;
    IP_ADDRESS  gatewayAddress;
    IP_ADDRESS  subnetMask;

    UINT32      systemSN;       /* System Serial Number           */
    UINT32      controllerSN;   /* Controller Serial Number       */

    UINT8       configFlags;    /* Configuration Flags - Bits     */
    /* Bit 0 - License Applied        */
    /* Bit 1 - Init Logs if 0         */
    /* Bit 2 - Replacement Controller */
    /* Bit 3 - RESERVED               */
    /* Bit 4 - RESERVED               */
    /* Bit 5 - RESERVED               */
    /* Bit 6 - RESERVED               */
    /* Bit 7 - RESERVED               */

    PORT_CONFIG config;         /* FC port configuration */
    UINT8       rsvd2[189];     /* RESERVED */

    UINT8       useDHCP;        /* Flag 1= Use DHCP               */
    UINT8       ethernetConfigured;     /* Flag 1= Ethernet configured    */

    UINT8       rsvd3[18];      /* RESERVED                       */

    IP_ADDRESS  debugConsoleIPAddr;     /* The address EtherWrite sends   */
    /*   to, in network byte order    */

    UINT8       debugConsoleChannel;    /* This is the port offset for    */
    /* EtherWrite to send data to     */
    /* DEBUG_PORT_NUMBER +            */
    /* debugConsoleChannel            */
    UINT8       UNUSED_X1S_Enable;      /* UNUSED old X1s enable bit      */

    UINT8       reserved[218];  /* RESERVED */
    UINT32      crc;
} CONTROLLER_SETUP;

CASSERT(sizeof(CONTROLLER_SETUP) == 476);

/*****************************************************************************
** Public variables
*****************************************************************************/

extern CONTROLLER_SETUP cntlSetup;
extern MUTEX backtraceMutex;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void LoadMasterConfigFromNVRAM(void);
extern void StoreMasterConfigToNVRAM(void);
extern void ResetMasterConfigNVRAM(void);
extern UINT32 NVRAM_DisasterDataLoad(DISASTER_DATA *disasterDataPtr);
extern UINT32 NVRAM_DisasterDataSave(DISASTER_DATA *disasterDataPtr);
extern UINT32 NVRAM_DisasterDataReset(DISASTER_DATA *disasterDataPtr);

extern bool IsControllerSetup(void);
extern void WaitForControllerSetup(void);
extern void LoadControllerSetup(void);
extern void SaveControllerSetup(void);
extern void SendPortConfig(UINT32 proc);

extern UINT32 MemSetBytes(void *dest, UINT8 val, UINT32 length);
extern UINT32 MemCpyBytes(void *dest, void *src, UINT32 length);
extern UINT32 MemSetNVRAMBytes(void *dest, UINT8 val, UINT32 length);

extern void CopyBacktraceDataToNVRAM(void);
extern void CopyNVRAMBacktraceDataToFlash(void);

extern INT32 UpdateIfcfgScript(UINT32 ipAddr, UINT32 snMask, UINT32 gwAddr);

extern bool IsConfigured(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _NVRAM_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
