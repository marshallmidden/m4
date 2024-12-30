/* $Id: MP_Proc.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       MP_Proc.h
**
**  @brief      Mirror Partner Information
**
**  To provide a common means of defining structures and Macros for mpi.c file
**
**  Copyright (c) 2004-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _MP_PROC_H_
#define _MP_PROC_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
/*
** The flags attribute in the mirror partner structure represents various fea-
** ---------------------------------------------------------------------------
** tures that it supports.
** -----------------------
*/

#define MP_USE_BFDEFAULTS_BIT       0      /* For CCB */
#define MP_CONTROLLER_TYPE_BIT      0      /* For Proc applications */
#define MP_CONFIG_CHANGE_BIT        1
#define MP_SERIAL_NUM_CHANGE_BIT    2

/*
** bit-0      : "Use BF Defaults" / "Controller Type"
**
**              This bit's interpretation is different for CCB and PROC processes.
**              From CCB's point of view this is called "USE BF Defaults" bit.
**              The CCB uses this bit (in its instance contained in MRP) to let
**              the FE know whether its MP is Bigfoot or not. In case of BF, the
**              CCB sets this bit and pass it to FE.
**              The FE on finding whether the MP is Bigfoot or non-Bigfoot,take
**              appropriate configuration data (BF defaults) and later uses this
**              bit in its configuration instance,  to check whether its MP is
**              BF or non-BF controller. And the same is carried to BE. The PROC
**              modules (especially NVRAM module) uses this bit to ensure whether
**              its MP is BF or non-BF thus enabling them to use either
**              addresses,or offsets for resync records processing.
**
**       Values of this bit are as follows
**
** For CCB -- While passing to FE, the CCB sets as follows, in the MRP.
*/

/*
** For PROC FE/BE - In the configuration instance, the FE will set like this.
*/
#define MP_CONTROLLER_TYPE_BF  0x00000001 /* Mirror Partner is BF */
#define MP_CONTROLLER_TYPE_WK  0x00000000 /* Mirror Partner is WK */

/*
** bit-1       :"Config_Change"
**
**              This is to let BE know , if there is any change in its current
**              configuration. This is either set or cleared in the current
**              configuration  instance of FE, before passing it to BE, based
**              on any change is there or not.
**
** bit-2       :"MP_SerialNo_Change"
**
**              This is to let BE know, if ther is any change in mirror partner
**              serial number. This is either set or cleared in the current
**              configuration instance of FE, before passing to BE, based on
**              any change in its MP serial number or not.
*/

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/
/*
** Macros used by applications
*/
#define gMPNvramP3Size              (gMPControllerCurrentConfig.nvramP3SizeBE)

#ifdef FRONTEND
#define gMPNvramP4Size              (gMPControllerCurrentConfig.nvramP4SizeFE)
#define gMPNvramP5Size              (gMPControllerCurrentConfig.nvramP5SizeFE)
#else
#define gMPNvramP4Size              (gMPControllerCurrentConfig.nvramP4SizeBE)
#define gMPNvramP5Size              (gMPControllerCurrentConfig.nvramP5SizeBE)
#endif /* FRONTEND */

#define gMPWCacheSize               (gMPControllerCurrentConfig.wCacheSize)
#define gMPFlags                    (gMPControllerCurrentConfig.flags)
#define gMPSerialNo                 (gMPControllerCurrentConfig.serialNo)
#define MP_IS_BIGFOOT               (BIT_TEST(gMPFlags,MP_CONTROLLER_TYPE_BIT)==TRUE)
#define MP_IS_WOOKIEE               (BIT_TEST(gMPFlags,MP_CONTROLLER_TYPE_BIT)==FALSE)
#define MP_IS_USE_BFDEFAULTS_SET    (BIT_TEST(gMPFlags,MP_USE_BFDEFAULTS_BIT)==TRUE)

#define MP_SET_MPSERIALNUM(sno)     (gMPSerialNo=sno)

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/**
** Mirror Partner Information structure that contain all required attributes
** i.e. resync capability and configration data of a controller.
**/
typedef struct MP_MIRROR_PARTNER_INFO
{
    UINT32  serialNo;       /**< Mirror partner's serial number             */
    UINT32  nvramP3SizeBE;  /**< BE Part-3 NVRAM Size                       */
    UINT32  nvramP4SizeFE;  /**< FE Part-4 NVRAM Size                       */
    UINT32  nvramP4SizeBE;  /**< BE Part-4 NVRAM Size                       */
    UINT32  nvramP5SizeFE;  /**< FE Part-5 NVRAM Size                       */
    UINT32  nvramP5SizeBE;  /**< BE Part-5 NVRAM Size                       */
    UINT32  wCacheSize;     /**< Write Cache  Size                          */
    UINT32  flags;          /**< Flags to indicate various features support */
    UINT8   batteryHealth;  /**< Battery health state                       */
    UINT8   rsvd[15];       /**< RESERVED                                   */
} MP_MIRROR_PARTNER_INFO;

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern MP_MIRROR_PARTNER_INFO gMPControllerCurrentConfig;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* MP_PROC_H_*/

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
