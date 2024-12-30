/* $Id: i82559_Strings.c 143007 2010-06-22 14:48:58Z m4 $ */
/*============================================================================
** FILE NAME:       i82559_Strings.c
** MODULE TITLE:    Intel 82559 Ethernet
**
** DESCRIPTION:     To provide configuration and other miscellaneous
**                  hardware-related services for the Intel 82559 Ethernet
**                  chip.
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "i82559_Strings.h"

#ifdef LOG_SIMULATOR
#include "LogSimFuncs.h"
#else   /* LOG_SIMULATOR */
#include "convert.h"
#include "logging.h"
#include "XIO_Std.h"
#include "XIO_Const.h"
#endif  /* LOG_SIMULATOR */

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: EthernetGetLinkStatusString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 stateNumber - state number to decode
**                 stringLength - size of the space allocated for stringPtr
**
**--------------------------------------------------------------------------*/
void EthernetGetLinkStatusString(char *stringPtr, LINK_STATUS stateNumber,
                                 UINT8 stringLength)
{
    switch (stateNumber)
    {
        case LINK_STATUS_DOWN:
            strncpy(stringPtr, "LINK-DOWN", stringLength);
            break;

        case LINK_STATUS_UP:
            strncpy(stringPtr, "LINK-UP", stringLength);
            break;

        default:
            strncpy(stringPtr, "LINK-????", stringLength);
            break;
    }
}


/*----------------------------------------------------------------------------
**  Function Name: EthernetGetWireSpeedString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 stateNumber - state number to decode
**                 stringLength - size of the space allocated for stringPtr
**
**--------------------------------------------------------------------------*/
void EthernetGetWireSpeedString(char *stringPtr, WIRE_SPEED stateNumber,
                                UINT8 stringLength)
{
    switch (stateNumber)
    {
        case WIRE_SPEED_10_MBPS:
            strncpy(stringPtr, "10 Mbps", stringLength);
            break;

        case WIRE_SPEED_100_MBPS:
            strncpy(stringPtr, "100 Mbps", stringLength);
            break;

        default:
            strncpy(stringPtr, "??? Mbps", stringLength);
            break;
    }
}


/*----------------------------------------------------------------------------
**  Function Name: EthernetGetDuplexModeString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 stateNumber - state number to decode
**                 stringLength - size of the space allocated for stringPtr
**
**--------------------------------------------------------------------------*/
void EthernetGetDuplexModeString(char *stringPtr, DUPLEX_MODE stateNumber,
                                 UINT8 stringLength)
{
    switch (stateNumber)
    {
        case DUPLEX_MODE_HALF:
            strncpy(stringPtr, "Half Duplex", stringLength);
            break;

        case DUPLEX_MODE_FULL:
            strncpy(stringPtr, "Full Duplex", stringLength);
            break;

        default:
            strncpy(stringPtr, "???? Duplex", stringLength);
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
