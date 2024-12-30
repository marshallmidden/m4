/* $Id: convert.c 143007 2010-06-22 14:48:58Z m4 $ */
/*============================================================================
** FILE NAME:       convert.c
** MODULE TITLE:    Data type conversion routines
**
** Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "convert.h"

#ifdef LOG_SIMULATOR
#include "LogSimFuncs.h"
#endif  /* LOG_SIMULATOR */

#include "string.h"
#include "XIO_Const.h"

/*****************************************************************************
** Private functions
*****************************************************************************/
static UINT32 hexval(INT8, INT32 *);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*
 * Use toupper()?
 */
unsigned char upper_case(unsigned char ascii_code)
{
    /*
     * ascii_code is lower case
     */
    if ((ascii_code >= 0x61) && (ascii_code <= 0x7a))
    {
        return (ascii_code - 0x20);
    }
    else
    {
        return (ascii_code);
    }
}


unsigned short Binary2BCD(unsigned short num)
{
    unsigned short digit;
    unsigned short leftover;

    digit = (num / 1000) << 12;
    leftover = num % 1000;

    digit = digit | ((leftover / 100) << 8);
    leftover = leftover % 100;

    digit = digit | ((leftover / 10) << 4);

    return (digit | (leftover % 10));
}


unsigned short BCD2Binary(unsigned short num)
{
    unsigned short thousands;
    unsigned short hundreds;
    unsigned short tens;
    unsigned short ones;

    thousands = ((num & 0xF000) >> 12) * 1000;
    hundreds = ((num & 0x0F00) >> 8) * 100;
    tens = ((num & 0x00F0) >> 4) * 10;
    ones = num & 0x000F;

    return (thousands + hundreds + tens + ones);
}


/**********************************************************************
*  Name:        BCD2Short()                                           *
*                                                                     *
*  Description: Convert a BCD to a short.                             *
*                                                                     *
*  Input:       short bcd - the bcd num to convert                    *
*                                                                     *
*  Returns:     The converted short.                                  *
**********************************************************************/
UINT16 BCD2Short(UINT16 bcd)
{
    UINT16      num = 0;
    UINT16      mul = 1;
    UINT16      i;

    for (i = 0; i < 4; i++)
    {
        num += (bcd & 0xF) * mul;
        mul *= 10;
        bcd >>= 4;
    }

    return num;
}


/**********************************************************************
*  Name:        ShortToBCD()                                          *
*                                                                     *
*  Description: Convert a short to a BCD.                             *
*                                                                     *
*  Input:       short  - number to convert to BCD                     *
*                                                                     *
*  Returns:     The converted BCD.                                    *
**********************************************************************/
UINT16 ShortToBCD(UINT16 num)
{
    UINT16      bcd = 0;
    UINT16      sh;

    for (sh = 0; sh < 16; sh += 4)
    {
        bcd |= (num % 10) << sh;
        num /= 10;
    }

    return bcd;
}


/************************************************
** Ascii to Hex Converter
************************************************/
UINT32 atoh(char *s, INT32 *n)
{
    INT32       val = 0;
    INT32       h = 0;

    if ((s[0] == '0') && ((s[1] == 'x') || (s[1] == 'X')))
    {
        s += 2;
    }

    if (*s == '\0')
    {
        return ERROR;
    }

    for (val = 0; *s; s++)
    {
        if ((hexval(*s, &h)) == ERROR)
        {
            return ERROR;
        }

        val = (val << 4) + h;
    }

    *n = val;
    return GOOD;
}


/************************************************
** Hex value of character passed
************************************************/
static UINT32 hexval(INT8 c, INT32 *returnVal)
{
    switch (c)
    {
        case '0':              /* 048 */
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            *returnVal = ((INT32)c - 48);
            break;

        case 'A':              /* 065 */
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            *returnVal = ((INT32)c - 55);
            break;

        case 'a':              /* 097 */
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
            *returnVal = ((INT32)c - 87);
            break;

        default:
            *returnVal = 0;
            return ERROR;
    }

    return GOOD;
}


/************************************************
** Character of hex value passed
************************************************/
UINT32 charval(INT8 hexValue, char *charValue)
{
    if ((hexValue >= 0) && (hexValue <= 9))
    {
        *charValue = hexValue + '0';
    }
    else if ((hexValue >= 10) && (hexValue <= 15))
    {
        *charValue = hexValue - 10 + 'a';
    }
    else
    {
        return ERROR;
    }

    return GOOD;
}


/*----------------------------------------------------------------------------
**  Function:   strtoupper
**
**  Comments:   Converts a NULL terminated string to upper case.
**
**  Parameters: str - String to convert.
**
**  Returns:    pointer to str.
**
**  NOTES:      NONE.
**
**--------------------------------------------------------------------------*/
char       *strtoupper(char *str)
{
    UINT32      count = 0;
    UINT8       convert = ('A' - 'a');

    if (str != NULL)
    {
        /*
         * Loop through the string, only converting
         * characters a - z.
         */
        while (str[count] != '\0')
        {
            if ((str[count] >= 'a') && (str[count] <= 'z'))
            {
                str[count] += convert;
            }

            ++count;
        }
    }

    return str;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
