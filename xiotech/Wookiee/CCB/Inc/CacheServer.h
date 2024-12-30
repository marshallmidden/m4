/* $Id: CacheServer.h 143845 2010-07-07 20:51:58Z mdr $*/
/***
**
******************************************************************************
**
**  @file   CacheServer.h
**
**  @brief  CCB Cache - Cached data for servers
**
**  Copyright (c) 2002-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef __CACHESERVER_H__
#define __CACHESERVER_H__

#include "CacheSize.h"
#include "PacketInterface.h"
#include "XIO_Std.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Private defines
*****************************************************************************/

/* An entry in the list contained in the structure below */
typedef struct wwnToTarget_t
{
    UINT16      sid;            /* Server ID used to ID this physical server */
    UINT16      targetBitmap;   /* Bit map of targets                        */
    UINT16      selectedTarget; /* Target selected for mapping               */
    UINT8       rsvd6[2];       /* Reserved                                  */
    UINT64      wwn;            /* Server WWN                                */
#if ISCSI_CODE
    UINT8       i_name[256];    /* iSCSI Server name                         */
#endif
} WWN_TO_TARGET;

/* --- Output structure for ServerWwnToTargetList() --- */
typedef struct _WWN_TO_TARGET_LIST
{
    UINT16      count;          /* Number of map entries                 */
    UINT8       rsvd[2];        /* RESERVED                              */
                ZeroArray(WWN_TO_TARGET, list); /* WWN to target list - see struct above */
} WWN_TO_TARGET_LIST;

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
extern UINT8 cacheServerMap[CACHE_SIZE_SERVER_MAP];
extern UINT8 *cacheServers;
extern UINT8 *cacheTempServers;
extern UINT32 cacheHABMapFE;
extern UINT32 cacheHABMapBE;

/*****************************************************************************
** Function prototypes
*****************************************************************************/

extern INT32 GetServerInfoFromWwn(UINT64 wwn, MRGETSINFO_RSP * pServerInfoOut);
extern INT32 GetServerInfoFromWwnNOW(UINT64 wwn, MRGETSINFO_RSP *);
extern INT32 GetServerInfoFromSid(UINT16 sid, MRGETSINFO_RSP * pServerInfoOut);
extern INT32 RefreshServers(void);
extern INT32 RefreshHABs(void);
extern PI_SERVERS_RSP *CachedServers(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __CACHESERVER_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
