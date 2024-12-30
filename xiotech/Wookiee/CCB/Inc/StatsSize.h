/* $Id: StatsSize.h 122127 2010-01-06 14:04:36Z m4 $*/
/**
******************************************************************************
**
**  @file       StatsSize.h
**
**  @brief      Statistics Cache Size
**
**  Constants for the Stats Manager.
**
**  Copyright (c) 2004-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef __STATSSIZE_H__
#define __STATSSIZE_H__

#include "MR_Defs.h"
#include "XIO_Const.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define STATS_SIZE_SERVERS          (MAX_SERVERS * (sizeof(STATS_SERVER_ITEM)))
#define STATS_SIZE_HABS             (MAX_HABS * (sizeof(MRGETHABSTATS_RSP)))

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __STATSSIZE_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
