/* $Id: EL_KeepAlive.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       EL_KeepAlive.h
**
**  @brief      Election disaster detection and recovery code
**
**  Copyright (c) 2003-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _EL_KEEP_ALIVE_H_
#define _EL_KEEP_ALIVE_H_

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
#define KEEP_ALIVE_SCHEMA               3

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/*
** Controller keepAliveList definitions
*/
typedef struct
{
    UINT16      slotValid:1;    /* TRUE when slot is valid                         */
    UINT16      reserved:15;    /* Room for future expansion                       */
} KEEP_ALIVE_LIST_ITEM_BITS;

typedef union
{
    UINT16      value;
    KEEP_ALIVE_LIST_ITEM_BITS bits;
} KEEP_ALIVE_LIST_ITEM;

/*
** Controller keep-alive structure header definitions
*/
typedef struct
{
    UINT16      systemEnabled:1;        /* TRUE when keepAlive feature is enabled          */
    UINT16      reserved:15;    /* Room for future expansion                       */
} KEEP_ALIVE_HEADER_FLAGS_BITS;

typedef union
{
    UINT16      value;
    KEEP_ALIVE_HEADER_FLAGS_BITS bits;
} KEEP_ALIVE_HEADER_FLAGS;

typedef struct
{
    UINT16      schema;         /* keepAlive structure compatability schema        */
    KEEP_ALIVE_HEADER_FLAGS flags;
} KEEP_ALIVE_HEADER;

/* NOTE: Changes to the size of KEEP_ALIVE structure will impact the masterConfig */
typedef struct
{
    KEEP_ALIVE_HEADER header;
    KEEP_ALIVE_LIST_ITEM keepAliveList[MAX_CONTROLLERS];

    UINT8       pad[SIZE_128 -  /* Pad to 128 bytes */
                    sizeof      (KEEP_ALIVE_HEADER) -   /* header           */
                                (sizeof(KEEP_ALIVE_LIST_ITEM) * MAX_CONTROLLERS)];      /* keepAliveList    */
} KEEP_ALIVE;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void EL_KeepAliveSystemReset(void);
extern void EL_KeepAliveSystemEnable(void);
extern void EL_KeepAliveSystemDisable(void);
extern UINT32 EL_KeepAliveSetSlotValid(UINT16 slotNumber, UINT32 validFlag);
extern UINT32 EL_KeepAliveTestSlot(UINT16 slotNumber);
extern UINT32 EL_KeepAliveSetUnfail(UINT32 unfailFlag);
extern UINT32 EL_KeepAliveSystemTestEnabled(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _EL_KEEP_ALIVE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
