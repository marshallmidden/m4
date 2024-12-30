/* $Id: beep1.c 16047 2006-12-21 22:34:30Z holtyb $ */
/**
******************************************************************************
**
**  @file       beep1.c
**
**  @version    $Revision: 16047 $
**
**  @brief      Make 1 second noise.
**
**  @author     Michael McMaster
**
**  @date       09/27/2004
**
**  Make noise.
**
**  Copyright (c) 2004-2005 Xiotech Corporation. All rights reserved.
**     
******************************************************************************
**/

#include <dos.h>

int main(int argc, char *argv[])
{
    int     returnCode = -1;

    /* Start beep */

    sound(800);

    /* Stop beep */

    delay(1000);
    nosound();
    delay(3000);

    return returnCode;
}

/*
 * vi:ts=4 sw=4 expandtab
 */
