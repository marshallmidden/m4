/* $Id: chap_base.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       chap_base.h
**
**  @brief      To convert from binary to base16 and base64, and vice versa.
**
**  Copyright (c) 2005 - 2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#ifndef _BASE_H_
#define _BASE_H_

#include <stdio.h>
#include <assert.h>

enum
{
     BIN_GRP_SZ    = 3,
     BASE64_GRP_SZ = 4,
     SIZE = 15,
};

/*
** takes and returns valid_ascii_len
*/
extern int binary_to_base64 (UINT8 *bin, int bin_len, char *ascii, int *p_valid_ascii_len);

/*
** takes and returns valid_bin_len
*/
extern int base64_to_binary (char *ascii, int ascii_len, UINT8 *bin, int *p_valid_bin_len);

/*
** takes and returns valid_ascii_len
*/
extern int binary_to_base16 (UINT8 *bin, int bin_len, char *ascii, int *p_valid_ascii_len);

/*
** takes and returns valid_bin_len
*/
extern int base16_to_binary (char *ascii, int ascii_len, UINT8 *bin, int *p_valid_bin_len);

#endif /* _BASE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
