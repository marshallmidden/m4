/* $Id: SerConTime.c 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       SerConTime.c
** MODULE TITLE:    Serial Console Set-time Frames
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "SerConTime.h"

#include "convert.h"
#include "cps_init.h"
#include "EL_Strings.h"
#include "kernel.h"
#include "logview.h"
#include "SerCon.h"
#include "SerConNetwork.h"
#include "serial.h"
#include "serial_num.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "rtc.h"
#include "XIO_Std.h"
#include "nvram.h"

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static unsigned char ShowSerialNumbers(unsigned char LineNumber);
static unsigned char ShowFWVersion(unsigned char LineNumber);
static unsigned char ShowTimeDate(unsigned char LineNumber);

/*****************************************************************************
** Private variables
*****************************************************************************/
static TIMESTAMP consoleTS;
static char TimeDateStrPtr[25];

/*****************************************************************************
** Code Start
*****************************************************************************/

void FirstFrameDisplayFunction(void)
{
    unsigned char CurrentLine = 0;

    sprintf(currentFramePtr->line[CurrentLine++], "\r\n************************************");
    sprintf(currentFramePtr->line[CurrentLine++], "\r\n");
    CurrentLine = ShowFWVersion(CurrentLine);
    CurrentLine = ShowEthernet(CurrentLine);
    CurrentLine = ShowSerialNumbers(CurrentLine);
    CurrentLine = ShowTimeDate(CurrentLine);
    sprintf(currentFramePtr->line[CurrentLine++], "\r\n");
    sprintf(currentFramePtr->line[CurrentLine++], "\r\n**** CONSOLE APPLICATION ***********\r\n");
#ifdef EXTRAOPTIONSDISPLAY
    sprintf(currentFramePtr->line[CurrentLine++], "(L)ogs display\r\n");
    sprintf(currentFramePtr->line[CurrentLine++], "(N)etstat display\r\n");
    sprintf(currentFramePtr->line[CurrentLine++], "(%) toggle diag port\r\n");
    sprintf(currentFramePtr->line[CurrentLine++], "(J)ournal choice\r\n");
#endif /* EXTRAOPTIONSDISPLAY */
    sprintf(currentFramePtr->line[CurrentLine++], "(C)ontroller Setup \r\n");
    sprintf(currentFramePtr->line[CurrentLine++], "(Q)uit\r\n");
    sprintf(currentFramePtr->line[CurrentLine], "\r\nSelect?: ");

    /*
     * end the display
     */
    CurrentLine++;
    sprintf(currentFramePtr->line[CurrentLine], "$");
}

static unsigned char ShowSerialNumbers(unsigned char LineNumber)
{
    char        strPowerUpState[MAX_POWER_UP_STATE_STR];
    char        strFailureState[30];
    char       *pTmp = strFailureState;

    memset(strPowerUpState, 0x00, MAX_POWER_UP_STATE_STR);

    sprintf(currentFramePtr->line[LineNumber++], "\r\nDSC ID:           %u (0x%x)",
            CntlSetup_GetSystemSN(), CntlSetup_GetSystemSN());

    sprintf(currentFramePtr->line[LineNumber++], "\r\nCN ID:            %u",
            Qm_SlotFromSerial(CntlSetup_GetControllerSN()));

    /*
     * Get the power-up state as a string for display
     */
    GetPowerUpStateString(strPowerUpState);

    /*
     * Get the failure state as a string for display.
     * Replace the '_'s with spaces.
     */
    EL_GetFailureDataStateString(strFailureState, GetControllerFailureState(),
                                 sizeof(strFailureState));
    strFailureState[sizeof(strFailureState) - 1] = 0;
    while (*pTmp)
    {
        if (*pTmp == '_')
        {
            *pTmp = ' ';
        }
        pTmp++;
    }

    /*
     * Master or Slave controller, powerup and failure states
     */
    sprintf(currentFramePtr->line[LineNumber++], "\r\nStates:           %s / %s / %s",
            TestforMaster(CntlSetup_GetControllerSN())? "PRINCIPLE" : "SECONDARY",
            strPowerUpState, strFailureState);

    return (LineNumber);
}

static unsigned char ShowFWVersion(unsigned char LineNumber)
{
    sprintf(currentFramePtr->line[LineNumber++], "\r\nFW Version:       %s-%s",
            gFWSystemRelease, gFWInternalRelease);
    return (LineNumber);
}

static unsigned char ShowTimeDate(unsigned char LineNumber)
{
    /*
     * send out a spacer line
     */
    sprintf(currentFramePtr->line[LineNumber++], "\r\n");

    /*
     * display day
     * display time and date
     */
    RTC_GetTimeStamp(&consoleTS);
    GetLogTimeString((LOGTIME *)&consoleTS, TimeDateStrPtr, false);
    sprintf(currentFramePtr->line[LineNumber++], "\r\n%s", TimeDateStrPtr);
    return (LineNumber);
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
