/* $Id: CacheSize.h 131756 2010-03-12 20:06:11Z mdr $*/
/**
******************************************************************************
**
**  @file   CacheSize.h
**
**  @brief  Constants for CCB Cache Manager
**
**  Copyright (c) 2002-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#ifndef __CACHESIZE_H__
#define __CACHESIZE_H__

#include "MR_Defs.h"
#include "PacketInterface.h"
#include "quorum.h"
#include "XIO_Std.h"
#include "XIO_Const.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines - Constants
*****************************************************************************/
#define VLINK_NAME_LEN                  8

/* Cache sizes for each system component */
#define CACHE_SIZE_DISK_BAYS        (MAX_DISK_BAYS * sizeof(MRGETPINFO_RSP))

#define CACHE_SIZE_DISK_BAYS_ADDR   (MAX_DISK_BAYS * sizeof(UINT8*))

#define CACHE_SIZE_PHYSICAL_DISKS   (MAX_PHYSICAL_DISKS * sizeof(MRGETPINFO_RSP))

#define CACHE_SIZE_PDISK_PATHS      (sizeof(MRGETBEDEVPATHS_RSP) + \
                                    (MAX_PHYSICAL_DISKS * \
                                    sizeof(MRGETBEDEVPATHS_RSP_ARRAY)))

#define CACHE_SIZE_DISK_BAY_PATHS   (sizeof(MRGETBEDEVPATHS_RSP) + \
                                    (MAX_DISK_BAYS * \
                                    sizeof(MRGETBEDEVPATHS_RSP_ARRAY)))

#define CACHE_SIZE_VIRTUAL_DISKS    (MAX_VIRTUAL_DISKS * \
                                    (sizeof(MRGETVINFO_RSP) + \
                                    (AVG_RAIDS_PER_VDISK * sizeof(UINT16))))

#define CACHE_SIZE_VDISK_ADDR       (MAX_VIRTUAL_DISKS * sizeof(UINT8*))

#define CACHE_SIZE_VDISK_CACHE_INFO (MAX_VIRTUAL_DISKS * \
                                     sizeof(CACHE_VDISK_CACHE_INFO))

#define CACHE_SIZE_RAIDS            (MAX_RAIDS * \
                                    (sizeof(MRGETRINFO_RSP) + \
                                    (MAX_PDISKS_PER_RAID * sizeof(MRGETRINFO_RSP_EXT))))

#define CACHE_SIZE_RAID_ADDR        (MAX_RAIDS * sizeof(UINT8*))

#define CACHE_SIZE_SERVERS          (MAX_SERVERS * \
                                    (sizeof(MRGETSINFO_RSP) + \
                                    (MAX_LUNS * sizeof(MRGETSINFO_RSP_LM))))

#define CACHE_SIZE_SERVER_ADDR      (MAX_SERVERS * sizeof(UINT8*))

#define CACHE_SIZE_HAB_ADDR         (MAX_HABS * sizeof(UINT8*))

#define CACHE_SIZE_TARGETS          (MAX_TARGETS * sizeof(MRGETTARG_RSP))

#define CACHE_SIZE_TARGET_ADDR      (MAX_TARGETS * sizeof(UINT8*))

#define CACHE_SIZE_PDISK_MAP        ((MAX_PHYSICAL_DISKS + 7) / 8)

#define CACHE_SIZE_PDISK_MAP_X1     ((MAX_PHYSICAL_DISKS_X1 + 7) / 8)

#define CACHE_SIZE_DISK_BAY_MAP     ((MAX_DISK_BAYS + 7) / 8)

#define CACHE_SIZE_VDISK_MAP        ((MAX_VIRTUAL_DISKS + 7) / 8)

#define CACHE_SIZE_RAID_MAP         ((MAX_RAIDS + 7) / 8)

#define CACHE_SIZE_SERVER_MAP       ((MAX_SERVERS_UNIQUE + 7) / 8)

#define CACHE_SIZE_TARGET_MAP       ((MAX_TARGETS + 7) / 8)

#define CACHE_SIZE_HAB_MAP          (sizeof(UINT8))

#define CACHE_SIZE_FE_LOOP_STATS    (MAX_FE_PORTS * (sizeof(PI_STATS_LOOP) + \
                                    sizeof(MRFELOOP_RSP) + \
                                    (MAX_TARGETS * sizeof(UINT16))))

#define CACHE_SIZE_FE_LOOP_STATS_ADDR   (MAX_FE_PORTS * sizeof(UINT8*))

#define CACHE_SIZE_BE_LOOP_STATS    (MAX_BE_PORTS * (sizeof(PI_STATS_LOOP) + \
                                    sizeof(MRBELOOP_RSP) + \
                                    (MAX_TARGETS * sizeof(UINT16))))

#define CACHE_SIZE_BE_PORT_INFO     (MAX_BE_PORTS * sizeof(PORT_INFO))

#define CACHE_SIZE_BE_LOOP_STATS_ADDR   (MAX_BE_PORTS * sizeof(UINT8*))

#define CACHE_SIZE_VCG_INFO         ((MAX_CONTROLLERS * sizeof(PI_VCG_CTRL_INFO)) + \
                                     sizeof(PI_VCG_INFO_RSP))

#define CACHE_SIZE_WORKSET_INFO     (sizeof(MRGETWSINFO_RSP) + \
                                     (MAX_WORKSETS * sizeof(DEF_WORKSET)))

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __CACHESIZE_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
