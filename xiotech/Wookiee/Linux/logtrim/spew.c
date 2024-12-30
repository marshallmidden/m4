/* $Header$ */
/**
******************************************************************************
**
**  @file       spew.c
**
**  @version    $Revision: 11794 $
**
**  @brief      Log trimming utility
**
**  @author     Michael McMaster
**
**  @date       02/09/2005
**
**  This utility is responsible for writing all data that comes into stdin
**  to a user-specified file, and assuring that the file does not grow
**  beyond a predetermined size.
**
**  Copyright (c) 2005-2006 Xiotech Corporation. All rights reserved.
**     
******************************************************************************
**/
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>


/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/


/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/


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
**  @brief      Main start of program
**
**              Main start of program
**
**  @param      argc    - number of args to prgram
**  @param      argv    - array of string arguments
**
**  @return     Should not return
**
**  @attention  none
**
******************************************************************************
**/
int main(int argc, char *argv[])
{
    int returnCode = 0;
    unsigned long long lineCounter = 1;
    struct timeval  startTime;      /* Uninitialized */
    struct timeval  endTime;        /* Uninitialized */

    setvbuf(stdout, (char *)NULL, _IOLBF, 0);
    gettimeofday(&startTime, NULL);

    while (lineCounter < 10000000)
    {
        int byteCounter = 0;
        char outputByte = '!';

        printf("STDOUT - Line %llu: ", lineCounter);

        while (byteCounter < 80)
        {
            putchar(outputByte);

            byteCounter++;
            outputByte++;

            if (outputByte > 'z')
            {
                outputByte = '!';
            }
        }
        putchar('\n');

        lineCounter++;
    }

    gettimeofday(&endTime, NULL);

    fprintf(stderr, "%llu lines\n", lineCounter);
    fprintf(stderr, "%u seconds\n",
        (unsigned int)(endTime.tv_sec - startTime.tv_sec));
    fprintf(stderr, "%u lines per second\n",
        (unsigned int)(lineCounter / ((endTime.tv_sec - startTime.tv_sec) + 1)));

    return returnCode;
}

/**
** vi:sw=4 ts=4 expandtab
*/
