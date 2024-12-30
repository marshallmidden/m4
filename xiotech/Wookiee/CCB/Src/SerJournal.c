/* $Id: SerJournal.c 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       SerJournal.c
** MODULE TITLE:    Serial Console Log Frames
**
** DESCRIPTION:     Interface to config journal through the serial console
**
** Copyright (c) 2002-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "SerJournal.h"

#include "convert.h"
#include "errorCodes.h"
#include "SerBuff.h"
#include "SerCon.h"
#include "serial.h"
#include "Snapshot.h"
#include "XIO_Std.h"
#include "XIO_Const.h"

extern void GotoFirstFrame(void);
extern void BadInput(void);

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void JournalMenuFrameDisplayFunction(void);
static void JournalMenuFrameResponseFunction(void);
static void RestorePromptFunction(void);
static void RestoreResponseFunction(void);
static void DisplayJournalDirectory(void);
static void DisplayJournalEntry(UINT32 count);

/*****************************************************************************
** Private variables
*****************************************************************************/
static unsigned char J_badInput = FALSE;
static UINT32 gRestoreIndex = 0;

static CONSOLEFRAME JournalMenuFrame = {
    JournalMenuFrameDisplayFunction,    /* Start function   */
    NULL,                       /* Finish function  */
    4,                          /* # of choices     */
    /* Choice  */
    {
      {'Q', GotoFirstFrame},
      {'D', DisplayJournalDirectory},
      {CRLF, BadInput},
      /* E#, R# below */
      {'\0', JournalMenuFrameResponseFunction}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

/* Controller Node Cluster ID FRAME */
static CONSOLEFRAME RestoreConfirmationFrame = {
    RestorePromptFunction,      /* Start function   */
    NULL,                       /* Finish function  */
    5,                          /* # of choices     */
    /* Choice */
    {
      {'Q', GotoFirstFrame},
      {'Y', RestoreResponseFunction},
      {'N', RestoreResponseFunction},
      {CRLF, RestoreResponseFunction},
      {'\0', BadInput}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

/*****************************************************************************
** Code Start
*****************************************************************************/

void JournalFrameChoiceFunction(void)
{
    J_badInput = FALSE;
    currentFramePtr = &JournalMenuFrame;
}

/*----------------------------------------------------------------------------
** Function:    JournalMenuFrameDisplayFunction
**
** Description: Display journal menu
**
** Inputs:      None
**
**--------------------------------------------------------------------------*/
static void JournalMenuFrameDisplayFunction(void)
{
    unsigned char LineNumber = 0;

    if (J_badInput == TRUE)
    {
        sprintf(currentFramePtr->line[LineNumber++], "\r\n\nInvalid option; try again.");
        J_badInput = FALSE;
    }
    else
    {
        sprintf(currentFramePtr->line[LineNumber++], "\r\n******** Configuration Journaling Menu ********\r\n");
    }

    sprintf(currentFramePtr->line[LineNumber++], "\r\n(D)   Display list of Journaled Configurations (Entries)");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n(E #) Extended Info on Journal Entry #");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n(R #) Restore Journal Entry #");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n\nChoose Option (or Q to quit): ");
    sprintf(currentFramePtr->line[LineNumber], "$");
}


/* ------------------------------------------------------------------------ */
static void JournalMenuFrameResponseFunction(void)
{
    UINT32      count;

    switch (upper_case(command.line[0]))
    {
        case ('E'):
            if (sscanf(&command.line[1], "%u", &count) != 1)
            {
                J_badInput = TRUE;
            }
            else
            {
                DisplayJournalEntry(count);
            }
            break;

        case ('R'):
            if (sscanf(&command.line[1], "%u", &gRestoreIndex) != 1)
            {
                J_badInput = TRUE;
            }
            else
            {
                currentFramePtr = &RestoreConfirmationFrame;
            }
            break;

        default:
            J_badInput = TRUE;
            break;
    }
}


/* ------------------------------------------------------------------------ */
static void DisplayJournalDirectory(void)
{
    char        heading[] = { "\r\n\n************** Journal Directory **************\r\n\n" };
    char       *entries = NULL;
    UINT32      len;

    /* Get a pointer to directory entries. */

    len = DisplaySnapshotDirectory(&entries);

    if (len && entries)
    {
        /* Send the string to the serial console to be displayed. */
        SerialBufferedWriteString(heading, strlen(heading));

        SerialBufferedWriteString(entries, len);

        /* Flush the string to the console. */
        SerialBufferedWriteFlush(TRUE);
    }

    if (entries)
    {
        Free(entries);
    }
}


/* ------------------------------------------------------------------------ */
static void DisplayJournalEntry(UINT32 count)
{
    char        heading[] = { "\r\n\n********* Journal Entry Extended Info *********\r\n\n" };
    char       *entry = NULL;
    UINT32      len;

    /*
     * Get a pointer to directory entry.
     */
    len = DisplaySnapshotDirectoryEntry(&entry, count);

    if (len && entry)
    {
        /* Send the string to the serial console to be displayed. */
        SerialBufferedWriteString(heading, strlen(heading));

        SerialBufferedWriteString(entry, len);

        /* Flush the string to the console. */
        SerialBufferedWriteFlush(TRUE);
    }

    if (entry)
    {
        Free(entry);
    }
}


/* ------------------------------------------------------------------------ */
static void RestorePromptFunction(void)
{
    unsigned char LineNumber = 0;

    if (J_badInput == TRUE)
    {
        sprintf(currentFramePtr->line[LineNumber++],
                "\r\n\nInvalid Entry Number; try again.");
        J_badInput = FALSE;
    }

    /*
     * Display the controller node cluster
     */
    sprintf(currentFramePtr->line[LineNumber++], "\r\n");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nWARNING: THIS WILL CHANGE YOUR CONTROLLERS' CONFIGURATION");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nAND RESTART THEM!  MAKE SURE ALL I/O TO YOUR CONTROLLERS");
    sprintf(currentFramePtr->line[LineNumber++], "\r\nHAS BEEN STOPPED BEFORE PROCEEDING.");
    sprintf(currentFramePtr->line[LineNumber++], "\r\n\r\nContinue Y/N ?: [N] ");
    sprintf(currentFramePtr->line[LineNumber++], "$");
}


/* ------------------------------------------------------------------------ */
static void RestoreResponseFunction(void)
{
    INT32       rc = 0;
    const char *msg = NULL;

    /*
     * If Yes
     */
    if (upper_case(command.line[0]) == 'Y')
    {
        /*
         * Load the snaphot
         * NOTE: IF SUCCESSFUL, THIS WILL RESET THE CONTROLLERS
         */
        rc = LoadSnapshot(gRestoreIndex, SNAPSHOT_FLAG_ALL);

        switch (rc)
        {
            case 0:
                msg = "The restoration was successful. The controllers will now restart automatically.\r\n";
                break;

            case SNAPSHOT_LOAD_INDEX_OUT_OF_RANGE:
                msg = "The requested journal entry number is out-of-range.\r\n";
                break;

            case SNAPSHOT_LOAD_NOT_LOADABLE:
                msg = "The requested journal entry is not restorable.\r\n";
                break;

            case SNAPSHOT_LOAD_NOT_ALL_FIDS_AVAILABLE:
                msg = "Some or all of the FIDs requested cannot be restored.\r\n";
                break;

            case SNAPSHOT_LOAD_ERROR_LOADING_A_FID:
                msg = "An error was encountered loading a FID. Restoration aborted.\r\n";
                break;

            default:
                msg = "The restoration encountered an unknown error. Restoration aborted.\r\n";
                break;
        }

        /* Flush the string to the console. */
        SerialBufferedWriteString("\r\n\r\n", 4);
        SerialBufferedWriteString(msg, strlen(msg));
        SerialBufferedWriteFlush(TRUE);

        /*
         * Wait here 30 seconds -- the reset should blow us out of
         * here.  If for some reason the reset fails, we will
         * end up back at the main menu.
         */
        if (rc == 0)
        {
            TaskSleepMS(30 * 1000);

            /* If we get here, the reset failed for some reason. */
            msg = "The controller reset failed for an unknown reason.\r\n";
            SerialBufferedWriteString("\r\n\r\n", 4);
            SerialBufferedWriteString(msg, strlen(msg));
            SerialBufferedWriteFlush(TRUE);
        }
    }
    /* else NO or carriage return -- falls through. */
    currentFramePtr = &JournalMenuFrame;
}

/* ------------------------------------------------------------------------ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
