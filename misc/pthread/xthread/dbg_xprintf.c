/* $URL: svn://cvs.xiotech.com/trunk/common/debug/dbg_xprintf.c $ */
/**
******************************************************************************
**
**  @file       dbg_xprintf.c
**
**  @version    $Revision: 117 $
**
**  @brief      Debug print functionality
**
**  @author     Chris Nigbur
**
**  modified    $Author: bbd $
**
**  @date       $Date: 2006-01-03 10:02:13 -0600 (Tue, 03 Jan 2006) $
**
**  Functionality to print debug information based on a defined set
**  of debug levels.
**
**  Copyright (c) 2005 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "xiostd.h"
#include "dbg_xprintf.h"


/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
#define XPRINTF_BUF_LENGTH          2048 /* TODO: use (SIZE_2K)??? */

/**
**  SYSLOG_LEVEL The syslog warning level to use for syslog() messages.
**                If defined in ~/.xiomake, that overrides.
**  SYSLOG_FACILITY The syslog facility to use for syslog() messages.
**                If defined in ~/.xiomake, that overrides.
**/
#ifndef SYSLOG_LEVEL
#define SYSLOG_LEVEL    LOG_DEBUG
#endif  /* SYSLOG_LEVEL */

#ifndef SYSLOG_FACILITY
#define SYSLOG_FACILITY LOG_LOCAL0
#endif  /* SYSLOG_FACILITY */


/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/
#define xprintf_check_level(a)      ((a & g_xprintf_level) == 0)


/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/


/*
******************************************************************************
** Private variables
******************************************************************************
*/
uint32_t    g_xprintf_level     = XPRINTF_LEVEL_DEFAULT;

/* If 0, openlog() has not been done yet. */
static int  syslog_started      = 0;


/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/


/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/


/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Get the current debug print level bits.
**
**              Gets the debug print level bits used by the xprintf()
**              function.
**
**  @return     uint32_t - Current debug print level bits.
**
**  @attention  none
**
******************************************************************************
**/
uint32_t dbg_xprintf_get_level(void)
{
    return g_xprintf_level;
}


/**
******************************************************************************
**
**  @brief      Set the current debug print level bits.
**
**              Sets the debug print level bits to be used by the xprintf()
**              function for all subsequent calls.
**
**  @param      level - New level bits.
**
**  @return     none
**
**  @attention  none
**
******************************************************************************
**/
void dbg_xprintf_set_level(uint32_t level)
{
    g_xprintf_level = level;
}


/**
******************************************************************************
**
**  @brief      Initialize the xprintf functionality.
**
**              Sets the debug print to use the given file for the output.
**              This function will open the file with the correct permissions
**              and make it available for output.
**
**  @param      programname - Name of the program initializing the
**                            xprintf functionality.
**
**  @return     none
**
******************************************************************************
**/
void dbg_xprintf_initialize(const char* programname)
{
    /*
    ** Open the syslog connection only once.
    */
    if (syslog_started == 0)
    {
        /*
        ** Open the syslog() connection "now" (don't delay until it is needed),
        ** log the process ID, and to the facility compiled in.
        */
        openlog(programname, LOG_NDELAY | LOG_PID, SYSLOG_FACILITY);
        syslog_started = 1;
    }
}


/**
******************************************************************************
**
**  @brief      Selectively print messages based on debug print level.
**
**              More details on this function go here.  The blank line above
**              is required to separate the brief description from the
**              detailed description
**
**  @param      filename    - Name of file that wrote this message
**  @param      function    - Function name within the file
**  @param      linenum     - Line number within the file
**  @param      level       - Level to use for this message
**  @param      fmt         - Format string to use to build message
**  @param      ...         - Format string replacement arguments
**
**  @return     none
**
**  @attention  none
**
******************************************************************************
**/
void _dbg_xprintf(
    const char*             filename,
    const char*             function,
    uint32_t                linenum,
    uint32_t                level,
    const char*             fmt, ...)
{
    const char* name;
    va_list     args;
    char        newfmt[BUFSIZ];

    /*
    ** If the syslog has not been started it means the user never called
    ** the initialize function, early exit.
    */
    if (syslog_started == 0)
    {
        return;
    }

    /*
    ** Check if this message should be printed based on the level of
    ** the message and the current xprintf level.
    */
    if (xprintf_check_level(level))
    {
        /*
        ** Early exit if not enabled
        */
        return;
    }

    /*
    ** Change filename to not have leading directory path.
    */
    name = strrchr(filename, '/');
    if (name != NULL)
    {
        name++;
    }
    else
    {
        name = filename;
    }

    snprintf(newfmt,
                (size_t)BUFSIZ,
                "[%s %s:%-4u] %s",
                name,
                function,
                linenum,
                fmt);

    /*
    ** Add on the actual message text
    */
    va_start(args, fmt);
    vfprintf(stderr, newfmt, args);
    /* vsyslog(SYSLOG_LEVEL, newfmt, args); */
    va_end(args);
}

/***
** Modelines
** vi:sw=4 ts=4 expandtab
***/
