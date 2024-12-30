/* $Id: ISP_Defs.h 158811 2011-12-20 20:42:56Z m4 $ */
/**
******************************************************************************
**
**  @file       ISP_Defs.h
**
**  @brief      ISP Definitions
**
**  To provide a common means of defining the ISP common structures.
**
**  Copyright (c) 2003-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _ISP_DEFS_H_
#define _ISP_DEFS_H_

#include "XIO_Macros.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

#define ISP_CONFIG_AUTO 0       /* Autonegotiate */
#define ISP_CONFIG_1    1       /* 1Gb */
#define ISP_CONFIG_2    2       /* 2Gb */
#define ISP_CONFIG_4    3       /* 4Gb */
#define ISP_CONFIG_8    4       /* 8Gb */
#define ISP_CONFIG_DEF  0xFF    /* Unconfigured default */

#define ISP_MAX_CONFIG_PORTS    8

#ifdef  PORT_NUMBER
CASSERT(PORT_NUMBER <= ISP_MAX_CONFIG_PORTS);
#endif /* PORT_NUMBER */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
/** ISP Read Link Status                                                    */
typedef struct
{
    UINT32  linkFailureCnt; /**< Link Failure Count                         */
    UINT32  lossSyncCnt;    /**< Loss of Sync Count                         */
    UINT32  lossSignalCnt;  /**< Loss of Signal Count                       */
    UINT32  primSeqErrCnt;  /**< Primitive Seq error count                  */
                            /* QUAD */
    UINT32  invTWCnt;       /**< Invalid Transmission Word Count            */
    UINT32  invCRCCnt;      /**< Invalid CRC count                          */
} ISP_RLS;

/** FC Speed configuration */
typedef struct
{
    UINT8   count;          /**< Port count */
    UINT8   config[4];      /**< Port pair configuration */
} ISP_CONFIG;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

#ifndef CCB_RUNTIME_CODE
UINT8 GetPortConfig(UINT8 port);
#endif  /* CCB_RUNTIME_CODE */

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _ISP_DEFS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
