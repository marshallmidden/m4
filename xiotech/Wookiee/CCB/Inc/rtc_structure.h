/* $Id: rtc_structure.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       rtc_structure.h
** MODULE TITLE:    Header file for rtc_structure.c
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _RTC_STRUCTURE_H_
#define _RTC_STRUCTURE_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
typedef volatile struct _RTC_REGISTERS
{
    UINT8       century;        /* century register                 */
    UINT8       seconds;        /* seconds register                 */
    UINT8       minutes;        /* minutes register                 */
    UINT8       hours;          /* hours register                   */
    UINT8       day;            /* day register                     */
    UINT8       date;           /* date register                    */
    UINT8       month;          /* month register                   */
    UINT8       year;           /* year register                    */
} RTC_REGISTERS;

typedef struct _RTC_STRUCTURE
{
    UINT8       reserved[8];    /* pad to place registers correctly */
    RTC_REGISTERS registers;
} RTC_STRUCTURE;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _RTC_STRUCTURE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
