/* $Id: rm_val.h 144191 2010-07-15 20:23:53Z steve_wirtz $*/
/**
******************************************************************************
**
**  @file       rm_val.h
**
**  @brief      Resource Manager Group Redundancy Validation
**
**  Copyright (c) 2002-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#ifndef __RM_VAL_H__
#define __RM_VAL_H__

#include "PacketInterface.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/**
** Bit flags for validation of system "components".
**/
#define VAL_TYPE_HW         0x00000001
#define VAL_TYPE_STORAGE    0x00000002
#define VAL_TYPE_SERVER     0x00000004
#define VAL_TYPE_COMM       0x00000008

#define VAL_TYPE_BE_LOOP    0x00000010
#define VAL_TYPE_SHELF_ID   0x00000020
#define VAL_TYPE_SYS_REL    0x00000040

#define DONT_USE_RSVD_B7    0x00000080

#define VAL_TYPE_RSVD_B8    0x00000100
#define VAL_TYPE_RSVD_B9    0x00000200
#define VAL_TYPE_RSVD_B10   0x00000400
#define VAL_TYPE_RSVD_B11   0x00000800

#define VAL_TYPE_RSVD_B13   0x00001000
#define VAL_TYPE_RSVD_B14   0x00002000
#define VAL_TYPE_ALL        0x00004000
#define VAL_RUN_IMMED       0x00008000

/**
** "Compound" validation types.  These include multiple tests using
** the individual test flags above.
**/
#define VAL_TYPE_BACK_END   (VAL_TYPE_STORAGE   | VAL_TYPE_BE_LOOP |  \
                             VAL_TYPE_SHELF_ID                          )

#define VAL_TYPE_NORMAL     (VAL_TYPE_HW        | VAL_TYPE_STORAGE  | \
                             VAL_TYPE_SERVER    | VAL_TYPE_COMM     | \
                             VAL_TYPE_BE_LOOP   | VAL_TYPE_SHELF_ID     )

#define VAL_TYPE_DAILY      (VAL_TYPE_NORMAL    | VAL_TYPE_SYS_REL)

/**
** Maximum length of a validation message.  Used to allocate message
** buffer space.
**/
#define VAL_MSG_LENGTH      256

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/
#define GetCNIDFromSN(x)        (((UINT8)(x)) & 0x0F)

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

extern void RM_StartDailyGroupValidation(void);
extern void RM_StartGroupValidation(UINT32 validationFlags);

extern PI_PORT_LIST_RSP *PortList(UINT32 controllerSN, UINT32 processor, UINT16 type);

extern UINT32 GetNextRemoteControllerSN(INT16 *pIndex);

extern PI_STATS_LOOPS_RSP *StatsLoops(UINT32 controllerSN, UINT32 processor);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __RM_VAL_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
