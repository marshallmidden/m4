/* $Id: L_Misc.c 157460 2011-08-03 14:52:28Z m4 $ */
/**
******************************************************************************
**
**  @file       L_Misc.c
**
**  @brief      Miscellaneous Linux Usability Functions.
**
**  Copyright (c) 2004-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "L_Misc.h"

#include "XIO_Const.h"

#ifdef LOG_SIMULATOR
    #include "LogSimFuncs.h"
#endif

#include <signal.h>
#include <stdio.h>

#define __USE_GNU
#include <string.h>
#undef __USE_GNU

#ifndef LOG_SIMULATOR
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <errno.h>
    #include <time.h>
    #include <sys/time.h>
#endif /* LOG_SIMULATOR */

/*
******************************************************************************
** Private variables
******************************************************************************
*/
static char sigRtStr[128];             /* String return L_LinuxSignalToString      */

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Translate Signal numbers into strings.
**
**              Translate Signal numbers into strings.
**
**  @inputs     sigNo   - Signal number of string.
**
**  @return     String of Signal.
**
******************************************************************************
**/
volatile char* L_LinuxSignalToString(INT32 sigNo, UINT32 data)
{
    /*
    ** Get the string.
    */
    switch(sigNo)
    {
        case XIO_PLATFORM_SIGNAL:
            switch (data)
            {
                case ERR_EXIT_SHUTDOWN:
                    sprintf(sigRtStr, "(%d) ERR_EXIT_SHUTDOWN", data);
                    break;

//                case ERR_EXIT_BE_MISS_HB:         UNUSED
//                    sprintf(sigRtStr, "(%d) ERR_EXIT_BE_MISS_HB", data);    UNUSED
//                    break;

//                 case ERR_EXIT_FE_MISS_HB:                        UNUSED
//                     sprintf(sigRtStr, "(%d) ERR_EXIT_FE_MISS_HB", data);
//                     break;

                case ERR_EXIT_DEADLOOP:
                    sprintf(sigRtStr, "(%d) ERR_EXIT_DEADLOOP", data);
                    break;

                case ERR_EXIT_REBOOT:
                    sprintf(sigRtStr, "(%d) ERR_EXIT_REBOOT", data);
                    break;

                case ERR_EXIT_RESET_CCB:
                    sprintf(sigRtStr, "(%d) ERR_EXIT_RESET_CCB", data);
                    break;

                case ERR_EXIT_RESET_ALL:
                    sprintf(sigRtStr, "(%d) ERR_EXIT_RESET_ALL", data);
                    break;

                case ERR_EXIT_FIRMWARE:
                    sprintf(sigRtStr, "(%d) ERR_EXIT_FIRMWARE", data);
                    break;

                case ERR_EXIT_BVM_RESTART:
                    sprintf(sigRtStr, "(%d) ERR_EXIT_BVM_RESTART", data);
                    break;

                case ERR_EXIT_BIOS_REBOOT:
                    sprintf(sigRtStr, "(%d) ERR_EXIT_BIOS_REBOOT", data);
                    break;

                case ERR_PAM_DIRECTED_DOWN:
                    sprintf(sigRtStr, "(%d) ERR_PAM_DIRECTED_DOWN", data);
                    break;

                default:
                    sprintf(sigRtStr, "(%d) Unknown Platform SIGNAL", data);
                    break;
            }
            break;

        default:
#ifdef LOG_SIMULATOR
            sprintf(sigRtStr, "(%d) Signal", sigNo);
#else
            sprintf(sigRtStr, "(%d) %s", sigNo, strsignal(sigNo));
#endif

            break;
    }

    return sigRtStr;
}

#ifndef LOG_SIMULATOR
/**
******************************************************************************
**
**  @brief      Set the clean shutdown flag
**
**              Put a marker in the filesystem indicating a clean shutdown.
**
**  @return     GOOD if marker placed successfully, ERROR otherwise.
**
******************************************************************************
**/
#define CLEAN_SHUTDOWN_FLAG_FILE "/opt/xiotech/ccbdata/clean_shutdown.flag"
INT32 SetCleanShutdown(void)
{
    FILE *pF;
    INT32 rc = ERROR;

    pF = fopen(CLEAN_SHUTDOWN_FLAG_FILE, "w");
    if (pF)
    {
        fclose(pF);
        rc = GOOD;
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Read the clean shutdown flag
**
**              Read the filesystem marker that indicates a clean shutdown or
**              not. Also deletes the file if it exists.
**
**              Note: A static flag is kept with the result of the initial
**                    check -- but the static flag exists ONLY in the process
**                    that made the initial call!
**
**  @return     TRUE if Clean Shutdown, FALSE otherwise.
**
******************************************************************************
**/
INT32 GetCleanShutdown(void)
{
    struct stat statInfo;
    static INT32 cachedRC = FALSE;

    /*
    ** If stat() succeeds, the file exists, so unlink it.
    */
    if ( stat(CLEAN_SHUTDOWN_FLAG_FILE, &statInfo) == 0 )
    {
        unlink(CLEAN_SHUTDOWN_FLAG_FILE);
        cachedRC = TRUE;
    }

    return cachedRC;
}
#endif /* LOG_SIMULATOR */

#ifndef LOG_SIMULATOR
/**
******************************************************************************
**
**  @brief      Get current time in milliseconds and return character string.
**
**  @return     Pointer to start of ascii string representing the time.
**
******************************************************************************
**/

static char millitime_buf[1024];    /* Static character string to return. */

char *millitime_string(void);       /* Forward reference (and prototype). */

char *millitime_string(void)
{
  struct timeval tv;
  int chars;

  gettimeofday(&tv, NULL);
  chars = strftime(millitime_buf, sizeof(millitime_buf), "%Y-%m-%d@%H:%M:%S", localtime(&tv.tv_sec));
  (void)sprintf(millitime_buf + chars, ".%06d", (int)tv.tv_usec);
  return(millitime_buf);
}

#endif /* LOG_SIMULATOR */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
