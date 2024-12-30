/* $Id: logview.h 143845 2010-07-07 20:51:58Z mdr $ */
/**
******************************************************************************
**
**  @file   logview.h
**
**  @brief  Header file for logview.c
**
**  Utility functions for viewing the flash based logs.
**
**  Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _LOGVIEW_H_
#define _LOGVIEW_H_

#include "XIO_Types.h"
#include <logging.h>

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

#define HTML_TYPE               0
#define ASCII_TYPE              1
#define JAVASCRIPT_TYPE         2
#define NO_INDENT               0
#define INDENT                  1
#define MAX_MESSAGE_LENGTH      255

/*
** Message Type definitions for length of message to display
*/
#define SHORT_MESSAGE   0
#define LONG_MESSAGE    1

/*
** Space to allocate for a standard message
*/
#define MAX_STANDARD_MESSAGE    256

/*
** Maximum size of log message
** Determine by default message displaying 128 Words at 4 words a line. This
** is 32 lines x 80 characters per line.  Plus one additional line for the
** standard message.
*/
#define MAX_LOG_MESSAGE_LINES   32
#if (((MAX_LOG_MESSAGE_LINES+1) * 80) < 4096)
  #define MAX_LOG_MESSAGE_SIZE  4096            /* ISCSI has a message this big. */
#else   /* (((MAX_LOG_MESSAGE_LINES+1) * 80) < 4096) */
  #define MAX_LOG_MESSAGE_SIZE  ((MAX_LOG_MESSAGE_LINES+1) * 80)
#endif  /* (((MAX_LOG_MESSAGE_LINES+1) * 80) < 4096) */

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: FormatLogEntry
**
**  Comments: Reads the requested log entry and stores it into a formatted
**            string.
**
**  Parameters: strPtr - pointer to where log entry string will be stored
**              logPtr - pointer to log entry to convert to string
**              messageType - short or long version of formatted message
**                          - valid values are SHORT_MESSAGE, LONG_MESSAGE
**
**--------------------------------------------------------------------------*/
extern void FormatLogEntry(char *strPtr, LOG_HDR *logPtr, char messageType);

/*----------------------------------------------------------------------------
**  Function Name: HTML_ExtendedMessage
**
**  Comments:  Parse the passed event code and create the extended message
**             associated with this event
**
**  Parameters:   logPtr - pointer to the desired log entry
**                strPtr - pointer to store the constructed string
**
**  Returns:  Length of extended message string
**
**  Notes:  Adds null termination to strPtr.
**--------------------------------------------------------------------------*/
extern unsigned int HTML_ExtendedMessage(char *strPtr, LOG_HDR *logPtr);

/*----------------------------------------------------------------------------
**  Function Name: ExtendedMessage
**
**  Comments:  Parse the passed event code and create the extended message
**             associated with this event
**
**  Parameters:   logPtr - pointer to the desired log entry
**                strPtr - pointer to store the constructed string
**                type   - used for string formatting options
**                indent - formatting option
**
**  Returns:  Length of extended message string
**
**  Notes:  Adds null termination to strPtr.
**--------------------------------------------------------------------------*/
extern void ExtendedMessage(char *strPtr, LOG_HDR *, char type, char indent);

/*----------------------------------------------------------------------------
**  Function Name: GetTimeString
**
**  Comments:  Convert the Time and date to a string of the form:
**              12:01pm 06/14/2000
**
**  Parameters:   timePtr - pointer to UINT32 time in milliseconds
**                strPtr  - pointer to a string to store the result
**
**--------------------------------------------------------------------------*/
extern void GetTimeString(UINT32 *timePtr, char *strPtr);

/*----------------------------------------------------------------------------
**  Function Name: GetLogTimeString
**
**  Comments:  Convert the Time and date to a string of the form:
**              12:01pm 06/14/2000
**
**  Parameters:   timePtr - ponter to LOGTIME timestamp
**                strPtr  - pointer to a string to store the result
**                military - use military time
**
**--------------------------------------------------------------------------*/
extern void GetLogTimeString(LOGTIME *timePtr, char *strPtr, bool military);

/*----------------------------------------------------------------------------
**  Function Name: GetEventTypeString
**
**  Comments:  Get the event type  of the log event and convert it to one of
**             the following strings:
**              "ERROR  "       - error entry
**              "INFO   "       - Informational entry
**              "WARNING"       - Warning entry
**
**  Parameters:   eventCode  - log entry event code
**                strPtr     - pointer to a string to store the result
**
**--------------------------------------------------------------------------*/
extern void GetEventTypeString(UINT16 eventCode, char *typeStr);

/*----------------------------------------------------------------------------
**  Function Name: GetMessageString
**
**  Comments:  Parse the passed event code and create the standard message
**             associated with this event
**
**  Parameters:   eventCode - event code value
**                messStr - pointer to store the constructed string
**                secToWait   - seconds to wait for cache refresh
**
**--------------------------------------------------------------------------*/
extern void GetMessageString(LOG_HDR *, char *messStr, UINT32 secToWait);

/*----------------------------------------------------------------------------
**  Function:   WWNToString
**
**  Comments:   Take an input wwn and create a string
**
**  Parameters: UINT64  wwn         - World Wide Name
**              char    *strPtr     - pointer to store the constructed string
**
**--------------------------------------------------------------------------*/
extern void WWNToString(UINT64 wwn, char *strPtr);

/*----------------------------------------------------------------------------
**  Function:   WWNToShortNameString
**
**  Comments:   Take an input wwn and create a string
**
**  Parameters: UINT64  wwn         - World Wide Name
**              char    *strPtr     - pointer to store the constructed string
**              secToWait   - seconds to wait (number of tries w/1 sec delay)
**
**--------------------------------------------------------------------------*/
extern void WWNToShortNameString(UINT64 wwn, char *strPtr, UINT32 secToWait);

/*----------------------------------------------------------------------------
**  Function:   DiskbayIdToString
**
**  Comments:   Take a diskbay misc. id and converts it to a char string.
**
**  Parameters: psId     - ID
**              strPtr   - pointer to store the constructed string
**
**--------------------------------------------------------------------------*/
extern void DiskbayIdToString(UINT32 id, char *strPtr);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _LOGVIEW_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
