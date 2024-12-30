/* $Id: SerCon.c 156020 2011-05-27 16:18:33Z m4 $ */
/*============================================================================
** FILE NAME:       SerCon.c
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "SerCon.h"

#include "ccb_hw.h"
#include "convert.h"
#include "kernel.h"
#include "led.h"
#include "SerBuff.h"
#include "serial.h"
#include "SerConLogs.h"
#include "SerConNetwork.h"
#include "SerConTime.h"
#include "SerJournal.h"
#include "XIO_Std.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "led_codes.h"
#include "debug_files.h"

/*****************************************************************************
** Private variables
*****************************************************************************/
static char goodbyeString[] = "\n\n-- Serial Console Closed --\n\n";
static char hitEnterString[] = "\n\n-- Hit <ENTER> to open serial console --\n\n";
static UINT8 consoleRunning = TRUE;
static UINT8 closeSerialConsole = TRUE;
static UINT8 lineLength = 0;
static UINT8 promptZero = 0;
static UINT8 txCharPosition = 0;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void AddToCommandInput(UINT8 *length, UINT8 data);
static void SendFrameOut(CONSOLEFRAME *framePtr);
static void DisplayConsoleFrame(CONSOLEFRAME *framePtr);
static void HandleUserResponse(CONSOLEFRAME *framePtr, char *input);
static void SerialConsoleSleep(void);

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
static PCB *serialConsoleMainTaskPCBPtr = NULL;
CONSOLEFRAME *currentFramePtr = &FirstFrame;
CMD_RECORD  command = {
    0,                          /* flags    */
    {0}                         /* line     */
};

/* FIRST FRAME */
CONSOLEFRAME FirstFrame = {
    FirstFrameDisplayFunction,  /* Start function   */
    NULL,                       /* Finish function  */
    6,                          /* # of choices     */

    /* Choice */
    {
     {'C', ControllerSetupFrameChoiceFunction},

/*        { 'S', ControllerSetupFrameChoiceFunction }, */
     {'L', SerialLogsFrameChoiceFunction},
     {'J', JournalFrameChoiceFunction},
     {'N', NetstatDisplayFunction},
     {'%', ToggleDiagPortsChoiceFunction},
     {'Q', SerialConsoleSleep},
     },

    /* Strings */
    {
     {'$', 0}
     }
};

/*****************************************************************************
** Code Start
*****************************************************************************/

/******************************************************************************
** NAME:        SerialConsoleMainTask
*******************************************************************************/
NORETURN void SerialConsoleMainTask(UNUSED TASK_PARMS *parms)
{
    UINT32      queueDepth = 0;

    /*
     * Initialize the global variables for this function
     */
    serialConsoleMainTaskPCBPtr = XK_GetPcb();
    currentFramePtr = &FirstFrame;
    consoleRunning = TRUE;
    closeSerialConsole = TRUE;

    for (;;)
    {
        /*
         * Check that the console menu has been transmitted
         * before indicating that the command has been processed.
         */
        if (SerialBufferedGetQueueDepth(TRANSMIT, &queueDepth) == GOOD)
        {
            /*
             * Check to see if we have anything left to send out
             * the user serial port.
             */
            if (queueDepth == 0)
            {
                /*
                 * Indicate that the command has fully transmitted the queued
                 * up response to the user's command.
                 */
                command.flags &= ~CR_FLAGS_IN_PROGRESS;

                /*
                 * Process the command if one is waiting
                 */
                if (command.flags & CR_FLAGS_WAITING)
                {
                    if (!(command.flags & CR_FLAGS_IN_PROGRESS))
                    {
                        /*
                         * Process the user command
                         */
                        HandleUserResponse(currentFramePtr, command.line);

                        /*
                         * Indicate that we've queued up the response in the
                         * serial output buffer.
                         */
                        command.flags |= CR_FLAGS_IN_PROGRESS;

                        /*
                         * Indicate that the command has been processed.
                         */
                        command.flags &= ~CR_FLAGS_WAITING;

                        /*
                         * Display the current frame.  Some commands will
                         * change the current frame pointer, but they will
                         * not display the frame, so we do it here.  However
                         * we'll only display something if the console
                         * is still open.
                         */
                        if (closeSerialConsole != TRUE)
                        {
                            DisplayConsoleFrame(currentFramePtr);
                        }
                    }
                }
                else
                {
                    /*
                     * Sleep until command is received, but only if there's no
                     * command waiting to be processed.
                     */
                    if (closeSerialConsole == TRUE)
                    {
                        /*
                         * Clear the closeSerialCosole variable so that
                         * it doesn't close next time.
                         */
                        closeSerialConsole = FALSE;

                        /*
                         * Let other tasks know console is sleeping
                         */
                        consoleRunning = FALSE;

                        /*
                         * Turn off the front panel indicator
                         */
                        LEDClearBit(LED_CONSOLE_CONNECTION);
                        /* Display "Hit <ENTER>" statement */
                        SerialBufferedWriteString(hitEnterString, sizeof(hitEnterString));
                    }

                    TaskSetState(serialConsoleMainTaskPCBPtr, PCB_NOT_READY);
                }
            }
        }
        else
        {
            /*
             * There's a problem with the user port transmit buffer.
             * Reset the command and hope that it fixes itself.
             */
            command.flags &= ~(CR_FLAGS_WAITING | CR_FLAGS_IN_PROGRESS);
        }

        /*
         * Exchange so that other tasks can run
         */
        TaskSwitch();

        /*
         * Indicate that the serial console is running
         */
        consoleRunning = TRUE;

        /*
         * Turn on the front panel indicator
         */
        LEDSetBit(LED_CONSOLE_CONNECTION);
    }
}


/******************************************************************************
** NAME:        AddToCommandInput
**
** INPUT:       length
**              data
**
** OUTPUT:      None
*******************************************************************************/
static void AddToCommandInput(UINT8 *length, UINT8 data)
{
    command.line[*length] = data;
    *length = *length + 1;

    /*
     * @ EOL
     */
    if (*length >= CONSOLE_COLUMNS)
    {
        *length = CONSOLE_COLUMNS;
    }

    /*
     * save string length
     */
    command.line[NUMCHARS] = *length;
}


/******************************************************************************
** NAME:        SendFrameOut
**
** INPUT:       framePtr
**
** OUTPUT:      None
*******************************************************************************/
static void SendFrameOut(CONSOLEFRAME *framePtr)
{
    UINT8       row = 0;
    UINT8       numchars;

    do
    {
        /*
         * test for end of current display
         */
        if (framePtr->line[row][0] == '$')
        {
            /*
             * if @ first row then it's zero
             */
            if (row <= 1)
            {
                row = 0;
                promptZero = 0;
            }
            else
            {
                /*
                 * determine where the cusor should stop
                 */
                row = row - 1;
                promptZero = strlen(framePtr->line[row]);

                /*
                 * get rid of \n\r in text line count
                 */
                if (promptZero <= 2)
                {
                    promptZero = 0;
                }
                else
                {
                    promptZero = promptZero - 2;
                }
            }

            /*
             * done, go back
             */
            txCharPosition = promptZero;
            break;
        }

        /*
         * clear out column counter
         */
        numchars = 0;
        do
        {
            if (SerialBufferedWriteChar(&framePtr->line[row][numchars]) == GOOD)
            {
                numchars++;
            }
        } while (numchars < strlen(framePtr->line[row]));

        /*
         * go to next text line in the display
         */
        row++;
    } while (row < CONSOLE_ROWS);
}


/******************************************************************************
** NAME:        DisplayConsoleFrame
**
** INPUT:       framePtr
**
** OUTPUT:      None
*******************************************************************************/
static void DisplayConsoleFrame(CONSOLEFRAME *framePtr)
{
    /*
     * set up the defined display for this frame, if there is one
     */
    if (framePtr->Display != NULL)
    {
        framePtr->Display();
    }

    /*
     * get it out
     */
    SendFrameOut(framePtr);

    /*
     * go do the defined action after display, if there is one
     */
    if (framePtr->AfterDisplay != NULL)
    {
        framePtr->AfterDisplay();
    }
}


/******************************************************************************
** NAME:        HandleUserResponse
**
** INPUT:       framePtr
**              input
**
** OUTPUT:      None
*******************************************************************************/
static void HandleUserResponse(CONSOLEFRAME *framePtr, char *input)
{
    UINT8       count;

    /*
     * go through the possible inputs for the current frame
     */
    for (count = 0; count < framePtr->numChoices; count++)
    {
        /*
         * test for match
         */
        if ((upper_case(*input) == framePtr->choice[count].letter) ||
            (framePtr->choice[count].letter == '\0'))
        {
            if (framePtr->choice[count].DoChoice != NULL)
            {
                /*
                 * go do defined action for that match
                 */
                framePtr->choice[count].DoChoice();
            }

            /*
             * get out of loop
             */
            break;
        }
    }
}


/******************************************************************************
** NAME:        SerialConsoleSleep
**
** OUTPUT:      None
*******************************************************************************/
static void SerialConsoleSleep(void)
{
    closeSerialConsole = TRUE;

    /* Display closing statement */
    SerialBufferedWriteString(goodbyeString, sizeof(goodbyeString));
}


/******************************************************************************
** NAME:        SerialConsoleHandleReceiveChar
**
** OUTPUT:      None
*******************************************************************************/
void SerialConsoleHandleReceiveChar(void)
{
    char        input = 0;
    char        output = 0;

    if (SerialBufferedReadChar(&input) == GOOD &&
        (!(command.flags & CR_FLAGS_WAITING)) &&
        (!(command.flags & CR_FLAGS_IN_PROGRESS)))
    {
        /*
         * If the serial console isn't running, force any incoming
         * characters to a CRLF, so that the console will start running.
         */
        if (consoleRunning == FALSE)
        {
            input = CRLF;
        }

        /*
         * Parse for backspace
         */
        if (input == BKSP || input == DEL)
        {
            /*
             * Test for begining of prompt
             */
            if (txCharPosition <= promptZero)
            {
                lineLength = 0;
                command.line[0] = 0;

                output = BELL;
                SerialBufferedWriteChar(&output);
            }
            else
            {
                /*
                 * Move back in command line
                 * Note: line length isn't committed until a CR
                 */
                lineLength = lineLength - 1;

                /*
                 * Test for inside display line
                 */
                if (txCharPosition < CONSOLE_COLUMNS)
                {
                    /* Destructive backspace */
                    output = BKSP;
                    SerialBufferedWriteChar(&output);

                    output = SPACE;
                    SerialBufferedWriteChar(&output);

                    output = BKSP;
                    SerialBufferedWriteChar(&output);
                }
                else
                {
                    /*
                     * @ EOL
                     */
                    output = SPACE;
                    SerialBufferedWriteChar(&output);
                }

                /*
                 * Move backwards one space on the display
                 */
                txCharPosition = txCharPosition - 1;
            }
        }
        else if (input == CRLF)
        {
            /* Echo the character back to the display */
            SerialBufferedWriteChar(&input);
            txCharPosition = txCharPosition + 1;

            /*
             * Check if a command is already due to be processed
             */
            if (txCharPosition >= CONSOLE_COLUMNS)
            {
                txCharPosition = CONSOLE_COLUMNS;
            }

            /*
             * save it
             */
            AddToCommandInput(&lineLength, input);

            /*
             * NULL terminate command line string
             */
            AddToCommandInput(&lineLength, '\0');

            /*
             * Indicate that the command is ready to be processed
             */
            command.flags |= CR_FLAGS_WAITING;

            /*
             * Make sure the serial console task is awake
             */
            TaskReadyState(serialConsoleMainTaskPCBPtr);

            /*
             * Reset variables o be prepared for the next command
             */
            txCharPosition = 0;
            lineLength = 0;
        }
        else if ((input >= SPACE) && (input < TILDE))
        {
            /*
             * We've received a 'good' character
             * Echo the character back to the display
             */
            SerialBufferedWriteChar(&input);
            txCharPosition = txCharPosition + 1;

            if (txCharPosition >= CONSOLE_COLUMNS)
            {
                txCharPosition = CONSOLE_COLUMNS;
            }

            /*
             * Save it to the command line
             */
            command.line[lineLength] = input;
            lineLength = lineLength + 1;

            /*
             * @ EOL
             */
            if (lineLength >= CONSOLE_COLUMNS)
            {
                lineLength = CONSOLE_COLUMNS;
            }
        }
        else
        {
            /* Not good character */
            output = BELL;
            SerialBufferedWriteChar(&output);
        }
    }
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
