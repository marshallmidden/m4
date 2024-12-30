/* $Id: L_Clone.c 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       L_Clone.c
**
**  @brief      Support functions to make a clone of a running task,
**              primarily for supporting the KME debug tool.
**
**  Copyright (c) 2002-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "L_Clone.h"

#ifdef CLONE
#include "XIO_Macros.h"

#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
******************************************************************************
** Private variables
******************************************************************************
*/
pid_t gCloneParentPID = 0;
UINT8 gCloneStack[SIZE_8K] = { 0 };

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
INT32 CloneTask(void *pParm);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      CloneCurrentThread
**
**              More details on this function go here.  The blank line above
**              is required to separate the brief description from the
**              detailed description
**
**  @param      none
**
**  @return     none
**
**  @attention  We use globals to pass information to the child, since we
**              aren't guaranteed that any locals will be in scope when
**              the child executes.  This function might end before the
**              new child is even allowed to run.
**
******************************************************************************
**/
void CloneCurrentThread( )
{
    pid_t newpid = 0;
    gCloneParentPID = getpid( );

    /*
    ** The stack on x86 grows downward, so we need to set the stack
    ** pointer at the end of the array we've set aside for the new thread.
    */
    fprintf( stdout, "In function CloneCurrentThread\n" );
    fprintf( stdout, "parent pid = %d\n", gCloneParentPID );
    fprintf( stdout, "gCloneStack range: %p to %p\n",
        gCloneStack, &gCloneStack[dimension_of(gCloneStack) - 1] );

    /*
    ** Set the parent to ignore the SIGCHLD to prevent a defunct process.
    */
    signal(SIGCHLD, SIG_IGN);

    /*
    ** Clone the current thread, but allow the new thread (child) to use
    ** the same virtual memory space as the parent.
    */
    newpid = clone( CloneTask,
        &gCloneStack[dimension_of(gCloneStack) - 1],
        CLONE_VM | SIGCHLD, NULL );

    if (newpid == -1)
    {
        fprintf( stderr, "Clone system call failed!\n");
        fprintf( stderr, "  errno: %d (%s)\n", errno, strerror(errno) );
    }
    else
    {
        fprintf( stdout, "Clone system call passed, child pid = %d\n", newpid );
    }
}


/**
******************************************************************************
**
**  @brief      CloneTask
**
**              More details on this function go here.  The blank line above
**              is required to separate the brief description from the
**              detailed description
**
**  @param      pParm - unused, but needed for clone prototype
**
**  @return     always returns ERROR
**
**  @attention  Don't make any calls to other CCB functions here, as it's
**              not expecting to be called from threads outside the
**              standard CCB kernel space.
**              Also, this thread has limited stack space, so don't
**              make it do too much.
**
******************************************************************************
**/
INT32 CloneTask( void *pParm )
{
    INT32 returnCode = GOOD;
    pid_t parentPID = gCloneParentPID;

    nice(17);
    fprintf( stdout, "CloneTask running\n" );

    while( returnCode == GOOD )
    {
        sleep( 5 );

        /*
        ** Periodically verify that the parent thread is still running
        */
        if( kill(parentPID, 0) == -1 )
        {
            fprintf( stderr, "CloneTask: errno=%d (%s)\n", errno, strerror(errno) );

            if( (errno == ESRCH) || (errno== EPERM) )
            {
                returnCode = ERROR;
            }
        }
    }

    fprintf( stderr, "CloneTask exiting\n" );
    return( returnCode );
}

#endif /* CLONE */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
