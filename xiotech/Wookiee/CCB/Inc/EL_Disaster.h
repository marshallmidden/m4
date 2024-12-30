/* $Id: EL_Disaster.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       EL_Disaster.h
**
**  @brief      Election disaster detection and recovery code
**
**  Copyright (c) 2003-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _EL_DISASTER_H_
#define _EL_DISASTER_H_

#include "EL_BayMap.h"
#include "logging.h"
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
#define DISASTER_DATA_SCHEMA    0

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/**
** Flags for the election code to track and remember disaster mode conditions
**/
typedef struct
{
    UINT32      disasterDetected:1;
    UINT32      reserved:31;
} DISASTER_DATA_FLAGS_BITS;

typedef union
{
    UINT32      value;
    DISASTER_DATA_FLAGS_BITS bits;
} DISASTER_DATA_FLAGS;

typedef struct
{
    UINT32      schema;                 /**< Schema for disaster data version   */
    DISASTER_DATA_FLAGS flags;          /**< Flags used in election logic       */
    char        reasonString[MMC_MESSAGE_SIZE]; /**< String to remember disaster reason */
    /* Pad so structure fits into 16 byte alignment (see nvram_structure.h) */
    UINT8       pad[SIZE_256 -          /**< Future growth - pad to 256 bytes   */
                    sizeof      (UINT32) -      /* schema                               */
                    sizeof      (DISASTER_DATA_FLAGS) - /* flags                                */
// MISTAKE ON NEXT LINE FOLLOWS, sizeof(40)=4
//                  sizeof      (MMC_MESSAGE_SIZE) -    /* reasonString                         */
                    4 -                 /**< MISTAKE can never be corrected, file CCB_NVRAM.mmf */
                    sizeof      (UINT32)];      /* crc (below)                          */
    UINT32      crc;                    /**< CRC to validate disaster data      */
} DISASTER_DATA;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern UINT32 EL_DisasterSetString(const char *newStringPtr);
extern UINT32 EL_DisasterGetFlags(DISASTER_DATA_FLAGS *returnFlagsPtr);
extern UINT32 EL_DisasterSetFlags(DISASTER_DATA_FLAGS *newFlagsPtr);
extern UINT32 EL_DisasterCheck(void);
extern UINT32 EL_DisasterCheckSafeguard(void);
extern UINT32 EL_DisasterBypassSafeguard(void);
extern UINT32 EL_DisasterTakeAction(const char *reasonStringPtr);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _EL_DISASTER_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
