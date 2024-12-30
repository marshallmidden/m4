/* $Id: cps_init.h 143845 2010-07-07 20:51:58Z mdr $ */
/*============================================================================
** FILE NAME:       cps_init.h
** MODULE TITLE:    Header file for cps_init.c
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _CPS_INIT_H_
#define _CPS_INIT_H_

#include "MR_Defs.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/**
**  @name Power Up States, UIR == User Input Required
**
**        These values are not bit sensitive although they look like
**        they are.
**  @{
**/
#define POWER_UP_UNKNOWN                0x0000 /**< Unknown or indeterminate*/
#define POWER_UP_START                  0x0001 /**< Power-up starting point */
#define POWER_UP_WAIT_FWV_INCOMPATIBLE  0x0002 /**< Mismatched FW, UIR      */
#define POWER_UP_WAIT_PROC_COMM         0x0004 /**< No PROC comm, UIR       */
#define POWER_UP_WAIT_CONFIGURATION     0x0005 /**< No Config, UIR          */
#define POWER_UP_WAIT_LICENSE           0x0008 /**< No license, UIR         */
#define POWER_UP_WAIT_DRIVES            0x0010 /**< No owned drives, UIR    */
#define POWER_UP_WAIT_DISASTER          0x0015 /**< Disaster Mode, UIR      */
#define POWER_UP_DISCOVER_CONTROLLERS   0x0019 /**< Discover Controllers    */
#define POWER_UP_WAIT_CONTROLLERS       0x0020 /**< Missing ctrls, UIR      */
#define POWER_UP_PROCESS_BE_INIT        0x0040 /**< Processing BE           */
#define POWER_UP_PROCESS_DISCOVERY      0x0080 /**< Processing Discovery    */
#define POWER_UP_WAIT_DISK_BAY          0x0100 /**< Missing disk bays, UIR  */
#define POWER_UP_WAIT_CORRUPT_BE_NVRAM  0x0200 /**< NVRAM image corrupt, UIR*/
#define POWER_UP_ALL_CTRL_BE_READY      0x0400 /**< All ctrls BE ready      */
#define POWER_UP_PROCESS_R5_RIP         0x0410 /**< Processing R5 Resync    */
#define POWER_UP_SIGNAL_SLAVES_RUN_FE   0x0420 /**< Signal slaves run FE    */
#define POWER_UP_PROCESS_CACHE_INIT     0x0800 /**< Process Cache Initialize*/
#define POWER_UP_WAIT_CACHE_ERROR       0x0810 /**< Cache Init Error, UIR   */
#define POWER_UP_INACTIVE               0x1000 /**< Ctrl inactive, UIR      */
#define POWER_UP_FAILED                 0x2000 /**< Ctrl failed, UIR        */
#define POWER_UP_WRONG_SLOT             0x2001 /**< Ctrl in wrong slot, UIR */
#define POWER_UP_FAILED_AUTO_NODE_CONFIG 0x2002 /**< Hardware error, UIR     */
#define POWER_UP_COMPLETE               0xFFFF /**< Power-up Complete       */
/* @} */

/**
**  @name Power Up Additional Status
**  @{
**/
#define POWER_UP_ASTATUS_UNKNOWN        0x0000 /**< Unknown, unused or indeterminate*/
#define POWER_UP_ASTATUS_WC_SEQNO_BAD   0x0001 /**< Write Cache, Bad Sequence Number */
#define POWER_UP_ASTATUS_WC_SN_VCG_BAD  0x0002 /**< Write Cache, Bad VCG Serial Number */
#define POWER_UP_ASTATUS_WC_SN_BAD      0x0003 /**< Write Cache, Bad Serial Number */
#define POWER_UP_ASTATUS_WC_NVMEM_BAD   0x0004 /**< Write Cache, Bad NVMEM  */
/* @} */

#define MAX_POWER_UP_STATE_STR          20

/**
**  @name Slave Initialization Options
**  @{
**/
#define SLAVE_INIT_FULL                 0x00    /**< Full slave init        */
#define SLAVE_INIT_RUN_BE               0x01    /**< Only allow BE to run   */
/* @} */

/*****************************************************************************
** Public variables
*****************************************************************************/
extern char gFWSystemRelease[];
extern char gFWInternalRelease[];

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern bool PowerUpBEReady(void);
extern bool PowerUpAllBEReady(void);
extern bool PowerUpComplete(void);

extern UINT16 GetPowerUpState(void);
extern void GetPowerUpStateString(char *str);
extern void SetPowerUpState(UINT16 state);
extern UINT16 GetPowerUpAStatus(void);
extern void SetPowerUpAStatus(UINT16 astatus);

extern void SetPowerUpResponse(UINT32 state, UINT8 response);
extern void SetInitialNVRAM(UINT16 eventCode);
extern void SetCacheErrorEvent(UINT32 event);
extern void CPSInitWaitForCacheInitTaskStart(void);
extern bool DiscoveryComplete(void);
extern void SetDiscoveryComplete(bool bComplete);
extern void CPSInitController(void);
extern void CPSInitSlaveController(UINT8 option);
extern UINT32 CPSInitGetOwnedDriveCount(UINT32 serialNum);
extern INT32 CreateController(UINT8 numControllers);
extern UINT16 MRP_Awake(UINT16 step);
extern void CPSInitWaitForever(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _CPS_INIT_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
