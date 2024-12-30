/* $Id: serial.c 149941 2010-11-03 21:38:18Z m4 $ */
/*============================================================================
** FILE NAME:       serial.c
** MODULE TITLE:    Serial port I/O implementation
**
** Copyright (c) 2001-2010 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "serial.h"

#include "ccb_hw.h"
#include "debug_files.h"
#include "idr_structure.h"
#include "kernel.h"
#include "mach.h"
#include "PortServerUtils.h"
#include "timer.h"
#include "XIO_Const.h"
#include "XIO_Std.h"
#include "XIO_Macros.h"

#include "fcntl.h"
#include "stdio.h"
#include "termios.h"

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
#define USER_PORT_HANDLE    "/dev/ttyS0"

INT32       userPortHandle = -1;

/*****************************************************************************
** Code Start
*****************************************************************************/

/*****************************************************************************
** FUNCTION NAME: SerialInit
**
** PARAMETERS:  None
**
** DESCRIPTION: Initialize and enable both channels to known state
**
** RETURNS:     GOOD
**              ERROR
******************************************************************************/
UINT32 SerialInit(void)
{
    UINT32      returnCode = GOOD;

    /* Indicate that the serial port has been initialized */
    char        userPortMessage[] = "User serial port initialized\r\n";
    struct termios newtios;

    memset(&newtios, 0, sizeof(newtios));

    if (TestSerialInitFlag() != TRUE)
    {
        if (userPortHandle < 0)
        {
            /* Configure the user serial port interface */
            userPortHandle = open(USER_PORT_HANDLE, O_RDWR | O_NOCTTY);
            SetBlockingState(userPortHandle, BLOCKING_ON);

            if (userPortHandle >= 0)
            {
                /* Load current settings */
                tcgetattr(userPortHandle, &newtios);

                /*
                 * Enable the receiver and set local mode
                 * Ignore break and parity
                 * Set 8 data bits, NO PARITY, 1 stop bit
                 */
                newtios.c_iflag = IGNBRK | IGNPAR | ISTRIP | INLCR;
                newtios.c_oflag = OPOST | ONLCR;        /* Map CR to CRLF on output */
                newtios.c_cflag = CS8 | CREAD | CLOCAL;
                newtios.c_lflag = IEXTEN;
                newtios.c_cc[VMIN] = 1;
                newtios.c_cc[VTIME] = 0;

                /* Set input and output baud rate to 115200 */
                cfsetispeed(&newtios, B115200);
                cfsetospeed(&newtios, B115200);

                /* Flush any old stuff and set the above attributes */
                tcflush(userPortHandle, TCIFLUSH);
                tcsetattr(userPortHandle, TCSANOW, &newtios);
                write(userPortHandle, userPortMessage, sizeof(userPortMessage));
            }
            else
            {
                returnCode = ERROR;
            }
        }
    }

    if (returnCode == GOOD)
    {
        /* Indicate that the serial ports have been initialized */
        SetSerialInitFlag(TRUE);
    }

    return (returnCode);
}

/*****************************************************************************
** FUNCTION NAME: SerialWrite
**
** PARAMETERS:  *buffer
**              length
**
** DESCRIPTION: Write 'length' number of bytes to the port.
**
** NOTE:        It is important that this function not make any other
**              function calls.  This is called from early in the boot process
**              and any deeper call depth could overflow the small stack.
**
** RETURNS:     GOOD
******************************************************************************/
UINT32 SerialWrite(const char *bufferPtr, UINT32 length)
{
    if (!bufferPtr)
    {
        return ERROR;
    }

    while (length > 0)
    {
        /* If we're outputting a newline, automatically insert a CR */
        if (userPortHandle >= 0)
        {
            if (*bufferPtr == '\n')
            {
                write(userPortHandle, "\r", 1);
            }
            write(userPortHandle, bufferPtr, sizeof(bufferPtr[0]));
        }

        /* Move the serial buffer pointer */
        bufferPtr++;
        length--;
    }

    return GOOD;
}


/*****************************************************************************
** FUNCTION NAME: SerialPutChar
**
** PARAMETERS:  writeChar
**
** DESCRIPTION: Transmit a single character
**
** RETURNS:     GOOD
**              ERROR
******************************************************************************/
UINT32 SerialPutChar(char writeChar)
{
    return SerialWrite(&writeChar, sizeof(writeChar));
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
