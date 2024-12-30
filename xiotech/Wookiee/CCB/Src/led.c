/* $Id: led.c 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       led.c
** MODULE TITLE:    Front panel LED routines
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "led.h"

#include "mach.h"
#include "XIO_Const.h"

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
UINT32      GPOD = 0;

/*****************************************************************************
** Code Start
*****************************************************************************/

/*****************************************************************************
** FUNCTION NAME: LEDSetBit
**
** PARAMETERS:  whichBit - (0 to 7)
**
** RETURNS:     Nothing
******************************************************************************/
void LEDSetBit(UINT8 whichBit)
{
    GPOD = ((GPOD) & (~(1 << whichBit)));
}

/*****************************************************************************
** FUNCTION NAME: LEDClearBit
**
** PARAMETERS:  whichBit - (0 to 7)
**
** RETURNS:     Nothing
******************************************************************************/
void LEDClearBit(UINT8 whichBit)
{
    GPOD = ((GPOD) | (1 << whichBit));
}


/*****************************************************************************
** FUNCTION NAME: LEDSetCode
**
** PARAMETERS:  newCode -
**
** DESCRIPTION:  Zion - Inverts the write to the GPOD register to show value
**
** RETURNS:     Nothing
******************************************************************************/
void LEDClearCode(void)
{
    GPOD = ~0;
}


/*****************************************************************************
** FUNCTION NAME: LEDSetAttention
**
** PARAMETERS:  ledSetting - (TRUE = Turn LED on, FALSE = Turn LED off)
**
** RETURNS:     Nothing
******************************************************************************/
void LEDSetAttention(UINT8 ledSetting)
{
    switch (ledSetting)
    {
        case TRUE:
            mach->frontPanelControl |= REG_FPLEDS_ATTENTION;
            break;

        case FALSE:
        default:
            mach->frontPanelControl &= ~REG_FPLEDS_ATTENTION;
            break;
    }
}


/*****************************************************************************
** FUNCTION NAME: LEDSetHeartbeat
**
** PARAMETERS:  ledSetting
**                TRUE  - Turn LED on
**                FALSE - Turn LED off
**
** RETURNS:     Nothing
******************************************************************************/
void LEDSetHeartbeat(UINT8 ledSetting)
{
    switch (ledSetting)
    {
        case TRUE:
            if (!mach->frontPanelControl & REG_FPLEDS_HEARTBEAT)
            {
                mach->heartbeatToggleControl = 0;
            }
            break;

        case FALSE:
        default:
            if (mach->frontPanelControl & REG_FPLEDS_HEARTBEAT)
            {
                mach->heartbeatToggleControl = 0;
            }
            break;
    }
}


/*****************************************************************************
** FUNCTION NAME: LEDSetCommFault
**
** PARAMETERS:  ledSetting - (TRUE = Turn LED on, FALSE = Turn LED off)
**
** RETURNS:     Nothing
******************************************************************************/
void LEDSetCommFault(UINT8 ledSetting)
{
    switch (ledSetting)
    {
        case TRUE:
            mach->frontPanelControl |= REG_FPLEDS_COMM_FAULT;
            break;

        case FALSE:
        default:
            mach->frontPanelControl &= ~REG_FPLEDS_COMM_FAULT;
            break;
    }
}


/*****************************************************************************
** FUNCTION NAME: LEDSetOffline
**
** PARAMETERS:  ledSetting
**                TRUE  - Turn LED on
**                FALSE - Turn LED off
**
** RETURNS:     Nothing
******************************************************************************/
void LEDSetOffline(UINT8 ledSetting)
{
    switch (ledSetting)
    {
        case TRUE:
            mach->frontPanelControl |= REG_FPLEDS_OFFLINE;
            break;

        case FALSE:
        default:
            mach->frontPanelControl &= ~REG_FPLEDS_OFFLINE;
            break;
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
