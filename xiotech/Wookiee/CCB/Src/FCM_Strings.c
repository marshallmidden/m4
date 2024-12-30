/* $Id: FCM_Strings.c 143007 2010-06-22 14:48:58Z m4 $ */
/*============================================================================
** FILE NAME:       FCM_Strings.c
** MODULE TITLE:    Fibre Channel Health Monitor string functions
**
** Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "FCM_Strings.h"

#ifdef LOG_SIMULATOR
#include "LogSimFuncs.h"
#else   /* LOG_SIMULATOR */
#include "XIO_Std.h"
#include "XIO_Const.h"
#endif  /* LOG_SIMULATOR */

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: FCM_GetEventTypeString
**
**  Comments:   Creates string for the specified eventType
**
**  Parameters: stringPtr
**              eventType
**              stringLength
**
**  Returns:    Nothing
**--------------------------------------------------------------------------*/
void FCM_GetEventTypeString(char *stringPtr, FCM_EVENT_TYPE eventType, UINT8 stringLength)
{
    if (stringPtr != NULL)
    {
        switch (eventType)
        {
            case FCM_ET_RESTORED:
                strncpy(stringPtr, "Restored", stringLength);
                break;

            case FCM_ET_DROPPED_FRAME:
                strncpy(stringPtr, "DF", stringLength);
                break;

            case FCM_ET_LOOP_RESET:
                strncpy(stringPtr, "LIP", stringLength);
                break;

            case FCM_ET_RSCN:
                strncpy(stringPtr, "RSCN", stringLength);
                break;

            case FCM_ET_UNDEFINED:
            default:
                strncpy(stringPtr, "Unknown", stringLength);
                break;
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_GetConditionCodeString
**
**  Comments:   Creates string for the specified eventType
**
**  Parameters: stringPtr
**              eventType
**              stringLength
**
**  Returns:    Nothing
**--------------------------------------------------------------------------*/
void FCM_GetConditionCodeString(char *stringPtr, FCM_CONDITION_CODE eventType,
                                UINT8 stringLength)
{
    if (stringPtr != NULL)
    {
        switch (eventType)
        {
            case FCM_CONDITION_CODE_GOOD:
                strncpy(stringPtr, "Good", stringLength);
                break;

            case FCM_CONDITION_CODE_BURST:
                strncpy(stringPtr, "Burst", stringLength);
                break;

            case FCM_CONDITION_CODE_RATE:
                strncpy(stringPtr, "Rate", stringLength);
                break;

            case FCM_CONDITION_CODE_UNDEFINED:
            default:
                strncpy(stringPtr, "Unknown", stringLength);
                break;
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
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
