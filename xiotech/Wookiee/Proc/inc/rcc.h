/* $Id: rcc.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       rcc.h
**
**  @brief      Retry Change Configuration request Definitions
**
**  To provide a common means of defining the format of the NVRAM P6 area
**  to be used for configuration saving.
**
**  Copyright (c) 2004-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#ifndef _RCC_H_
#define _RCC_H_

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/
#define RCC_G0 ilt_normal.w1    /* General Register 0                   */
#define RCC_G1 ilt_normal.w2    /* General Register 1                   */

#define RCC_G2 ilt_normal.w3    /* General Register 2                   */
#define RCC_G3 ilt_normal.w4    /* General Register 3                   */
#define RCC_G4 ilt_normal.w5    /* General Register 4                   */
#define RCC_G5 ilt_normal.w6    /* General Register 5                   */

#endif /* _RCC_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
