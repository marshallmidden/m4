/* $Id: code_start.c 143007 2010-06-22 14:48:58Z m4 $ */
/*============================================================================
** FILE NAME:       code_start.c
** MODULE TITLE:    Runtime code entry point
**
** Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "code_start.h"

#include "debug_files.h"
#include "xk_init.h"
#include "XIO_Std.h"

#include "idr_structure.h"
#include "main.h"
#include "XIO_Const.h"

/*****************************************************************************
** Code Start
*****************************************************************************/

/*****************************************************************************
** FUNCTION NAME: main
**
** PARAMETERS:  Standard argc and argv
******************************************************************************/
extern void XK_KernelInitTime(void);

int main(int argc, char *argv[])
{
#ifdef close
#undef close
#endif  /* close */
    close(0);                   /* No stdin needed. */
    close(3);                   /* Extra file descriptors left over from "commands" */
    close(4);
    close(5);
    close(6);
    close(7);
    close(8);
    close(9);
    close(10);

    /*
     * Set the OS output buffering mode to 'line buffer'
     */
    setvbuf(stdout, (char *)NULL, _IOLBF, 0);
    setvbuf(stderr, (char *)NULL, _IOLBF, 0);

    XK_KernelInitTime();

    /*
     * Get the pid for the Platform Application Monitor
     */
    if (argc > 1)
    {
        pamPid = atoi(argv[1]);
        dprintf(DPRINTF_DEFAULT, "PAM pid = %d\n", pamPid);
    }

    /*
     * Initialize the Hypernode System
     */
    XK_Init();

    /*
     * Fork Start
     */
    dprintf(DPRINTF_DEFAULT, "Forking Start...\n");
    TaskCreate(Start, NULL);

    /*
     * Stay Here
     */
    while (1)
    {
        usleep(1000 * 1000 * 1000);
    }

    /*
     * This line is NEVER executed, but it's here to keep the compiler happy.
     */
    return (GOOD);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
