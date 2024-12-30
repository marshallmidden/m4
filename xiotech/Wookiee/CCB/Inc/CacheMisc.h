/* $Id: CacheMisc.h 146064 2010-08-20 21:15:33Z m4 $*/
/**
******************************************************************************
**
**  @file       CacheMisc.h
**
**  @brief      CCB Cache - Miscellaneous
**
**  Cached data for the left over stuff that doesn't fit anywhere else.
**
**  Copyright (c) 2002 - 2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef __CACHEMISC_H__
#define __CACHEMISC_H__

#include "CacheManager.h"
#include "CacheSize.h"
#include "PacketInterface.h"
#include "XIO_Const.h"
#include "XIO_Std.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/**
** Replace usages with MAX_CONTROLLERS when it is changed to 16
**/
#define HACK_MAX_CONTROLLERS    16

/**
** Indicates an invalid FE processor Server ID value
**/
#define INVALID_SID         0xFFFF

/**
** remotePortId value indicating that no remote port exists.
**/
#define NO_REMOTE_PORT      0xFFFF

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

typedef struct _PORT_INFO
{
    UINT16      localPortState;     /**< PI_LOOP_STATE_ for this port       */
    UINT16      remotePortId[HACK_MAX_CONTROLLERS - 1]; /**< MSB=remote port number
                                                         **  LSB=remote Controller
                                                         **  Node ID            */
} PORT_INFO;

typedef struct CACHE_FREE_SPACE
{
    UINT64      maxRaid0;
    UINT64      maxRaid5;
    UINT64      maxRaid10;
} CACHE_FREE_SPACE;

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
extern X1_CONFIG_ACCOUNT cacheAccountInfo;
extern CACHE_FREE_SPACE cacheFreeSpace;

#ifndef NO_VCG_CACHE
extern UINT8 cacheVcgInfo1[CACHE_SIZE_VCG_INFO];
extern UINT8 cacheVcgInfo2[CACHE_SIZE_VCG_INFO];
extern UINT8 *cacheVcgInfo;
extern UINT8 *cacheTempVcgInfo;
#endif  /* NO_VCG_CACHE */

extern PORT_INFO cacheBEPortInfo1[MAX_BE_PORTS];
extern PORT_INFO cacheBEPortInfo2[MAX_BE_PORTS];
extern PORT_INFO *cacheBEPortInfo;
extern PORT_INFO *cacheTempBEPortInfo;

extern UINT8 cacheWorksetInfo1[CACHE_SIZE_WORKSET_INFO];
extern UINT8 cacheWorksetInfo2[CACHE_SIZE_WORKSET_INFO];
extern UINT8 *cacheWorksetInfo;
extern UINT8 *cacheTempWorksetInfo;

/*****************************************************************************
** Function prototypes
*****************************************************************************/

extern void EnvironmentalGetStats(UINT8 *buf);

extern void GetFirmwareHeader(UINT16 fwType, MR_FW_HEADER_RSP * pFWHeader);
extern void GetFirmwareData(FW_DATA * pFWData);
extern UINT8 GetFirmwareCompatibilityIndex(void);
extern void GetQLogicFWVersion(X1_VERSION_ENTRY *fwVer);
extern INT32 RefreshEnvironmentals(void);
extern INT32 RefreshVcgInfo(void);
extern void RefreshFirmwareHeaders(void);
extern void RefreshBEDevicePaths(UINT16 type);
extern INT32 RefreshBEPortInfo(void);
extern INT32 RefreshWorksets(void);
extern void GetTenEnv(UINT8 *, UINT8 *, UINT32 *, UINT32 *);

extern void CacheRefreshAllRmtCtrl(UINT32 cacheMask, bool waitForCmpl);

extern INT32 CacheFreeSpaceRefresh(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __CACHEMISC_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
