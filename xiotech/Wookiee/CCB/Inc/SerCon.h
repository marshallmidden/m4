/* $Id: SerCon.h 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       SerCon.h
** MODULE TITLE:    Header file for serial_console.c
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _SERIAL_CONSOLE_H_
#define _SERIAL_CONSOLE_H_

#include "pcb.h"
#include "XIO_Types.h"
#include "xk_kernel.h"

#ifdef __cplusplus
#pragma pack(push, 1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
#define CONSOLE_ROWS                     20
#define CONSOLE_COLUMNS                  80
#define CONSOLE_COMMAND_LINE_LENGTH      (CONSOLE_COLUMNS + 6)
#define NUMCHARS                         (CONSOLE_COMMAND_LINE_LENGTH - 1)
#define NUMBER_OF_FRAMECHOICES           10

typedef struct _CMD_RECORD
{
    unsigned char flags;
    char        line[CONSOLE_COMMAND_LINE_LENGTH];
} CMD_RECORD;

#define CR_FLAGS_WAITING                (1 << 0)
#define CR_FLAGS_IN_PROGRESS            (1 << 1)

typedef struct _FRAMECHOICES
{
    char        letter;
    void        (*DoChoice)(void);
} FRAMECHOICES;

/*
** Note:1. if there are no actions the 1st frame choice should be NULL, NULL
**      2. if there is only 1 action then it must be first
*/
typedef struct _CONSOLEFRAME
{
    void        (*Display)(void);       /* Function to call to display text */
    void        (*AfterDisplay)(void);  /* Function to call after the text is displayed */
    UINT8       numChoices;             /* Number of choices available */
    FRAMECHOICES choice[NUMBER_OF_FRAMECHOICES];        /* Choice frames */
    char        line[CONSOLE_ROWS][CONSOLE_COLUMNS];    /* Text strings */
} CONSOLEFRAME;

/*****************************************************************************
** Public variables
*****************************************************************************/
extern CONSOLEFRAME *currentFramePtr;
extern CONSOLEFRAME FirstFrame;
extern CMD_RECORD command;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void SerialConsoleMainTask(TASK_PARMS *parms);
extern void SerialConsoleHandleReceiveChar(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _SERIAL_CONSOLE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
