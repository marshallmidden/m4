/* $Id: hw_common.c 149941 2010-11-03 21:38:18Z m4 $ */
/**
******************************************************************************
**
**  @file       hw_common.c
**
**  @brief      Common Hardware functions.
**
**  Common Hardware functions.
**
**  Copyright (c) 2006-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <stdint.h>
#include "hw_common.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/

/**
**  @ingroup _HW_MODULES_COMMON_FUNCTIONS
**  @defgroup HW_MODULES_COMMON_PRIVATE_FUNCTIONS Common Hardware Private Functions
**
**  @brief      These are the private functions available for this interface.
**
**  @{
**/

static int32_t hw_convert_signed_and_write(int64_t value, const char *filename);
static int32_t hw_read_and_convert_signed(int64_t *value, const char *filename);

/* @} */

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Write 8 bytes to filename.
**
**  @param      value    - value to write to filename
**  @param      filename - filename of device to write.
**
**  @return     0 on Success
**  @return     -1 on Failure
**
******************************************************************************
**/
inline int32_t hw_write_int64_as_str(int64_t value, const char *filename)
{
    return hw_convert_signed_and_write((int64_t)value, filename);
}

/**
******************************************************************************
**
**  @brief      Read 8 byte from filename.
**
**  @param      value    - where to store read value
**  @param      filename - filename of device to read.
**
**  @return     0 on Success
**  @return     -1 on Failure
**
******************************************************************************
**/
inline int32_t hw_read_int64_from_str(int64_t *value, const char *filename)
{
    int32_t     rc;
    int64_t     val;

    rc = hw_read_and_convert_signed(&val, filename);
    if (rc == 0)
    {
        *value = (int64_t)val;
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Write Conversion to signed value and write.
**
**  @param      value    - value to write to filename
**  @param      filename - filename of device to write.
**
**  @return     0 on Success
**  @return     -1 on Failure
**
******************************************************************************
**/
static int32_t hw_convert_signed_and_write(int64_t value, const char *filename)
{
    char        strbuffer[32];
    uint32_t    strlength;

    strlength = sprintf(strbuffer, fmt64i, value);
    if (!strlength)
    {
        return -1;
    }

    strlength += 1;
    return hw_write_buffer((uint8_t *)strbuffer, strlength, filename);
}

/**
******************************************************************************
**
**  @brief      Read and convert signed value.
**
**  @param      value    - where to store value read from filename
**  @param      filename - filename of device to read.
**
**  @return     0 on Success
**  @return     -1 on Failure
**
******************************************************************************
**/
static int32_t hw_read_and_convert_signed(int64_t *value, const char *filename)
{
    int32_t     rc;
    char        strbuffer[32];

    rc = hw_read_buffer((uint8_t *)strbuffer, 32, filename);
    if (rc)
    {
        return -1;
    }

    strbuffer[31] = '\0';
    *value = atoi(strbuffer);

    return 0;
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
