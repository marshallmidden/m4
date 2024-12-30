/* $Id: sendevt.c 4298 2005-05-04 18:53:47Z RysavyR $ */
/*
 * sendevt.c: Send event.
 *
 * Copyright 2004 Xiotech Corporation
 *
 * Mark Rustad (Mark_Rustad@Xiotech.com)
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "xio3d.h"

#define DEVFILE "/dev/xio3d0"


int main(int argc, char *argv[])
{
    int i;
    int err;
    int xiofd;

    xiofd = open(DEVFILE, O_RDONLY);
    if (xiofd < 0)
    {
        fprintf(stderr, "open of %s failed with %d\n", DEVFILE, xiofd);
        perror("open failed");
        return xiofd;
    }

    /*
     * Send events.
     */

    for (i = 1; i < argc; ++i)
    {
        unsigned long   evt;

        evt = strtoul(argv[i], 0, 0);
        if (errno)
        {
            fprintf(stderr, "conversion of [%s] failed, errno=%d\n",
                    argv[i], errno);
            return -1;
        }
        fprintf(stderr, "sending event %lu\n", evt);
        err = ioctl(xiofd, XIO3D_SENDEVT, evt);
        if (err != 0)
        {
            fprintf(stderr, "send event ioctl returned %d, errno=%d\n",
                    err, errno);
            perror("send event ioctl failed");
            return -1;
        }
    }

    fprintf(stderr, "Done sending events\n");

    if (close(xiofd) != 0)
    {
        perror("close failed");
        return errno;
    }

    return 0;
}
