/* $Id: sg_fwupdate.c 14323 2006-10-05 16:41:54Z DavisB $ */

/***
 * sg_fwupdate.c - Update Vitesse expander firmware in AIC chassis.
 *
 * Steve Wirtz
 *
 * Copyright 2006 Xiotech Corporation. All rights reserved.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <error.h>
#include <errno.h>
#include <unistd.h>
#include <scsi/sg.h>
#include <scsi/scsi.h>
#include <string.h>
#include "sg_iosup.h"

#define CHUNKSIZE       0x800   /* 2k bytes per WRITE_BUFFER */


/**
 * @brief store_be_n - Store big-endian number in "n" bytes
 *
 * @param val - Value to store
 *
 * @param addr - First of the byte range to store into
 *
 * @param len - Number of bytes to store
 */

static void store_be_n(unsigned long val, unsigned char *addr, int len)
{
    if (len <= 0)
        return;
    while (len) {
        addr[--len] = (unsigned char)(val & 0xFF);
        val >>= 8;
    }
}


/**
 * @brief usage - Print command usage information.
 */

static void usage(void)
{
    fprintf(stderr, "  Usage sg_fwupdate /dev/sgn fwfile\n");
}


/**
 * @brief main - Main function for sg_fwupdate
 *
 * @param argc - Number of arguments in argv
 *
 * @param argv - Array of pointers to arguments
 */

int main(int argc, char *argv[])
{
    unsigned char *page;
    int handle;
    int ret;
    FILE *fwfile;
    int imagesize;
    int offset;
    int i;
    const char *fwpath;
    const char *devpath;
    unsigned char inqdata[0xff];

    if (argc < 3) {
        usage();
        exit(1);
    }
    devpath = argv[1];
    fwpath = argv[2];
    handle = open(devpath, O_RDWR | O_NONBLOCK);
    if (handle < 0) {
        fprintf(stderr, "Open of %s failed with errno %d, %s\n",
                devpath, errno, strerror(errno));
        usage();
        exit(1);
    }

    fwfile = fopen(fwpath, "rb");
    if (fwfile == NULL) {
        fprintf(stderr, "Could not open fw image file %s\n", fwpath);
        usage();
        exit(1);
    }

    memset(inqdata, 0, sizeof inqdata);
    ret = inquiry(handle, 0, 0, inqdata);
    if ((ret <= 0) || ((inqdata[0] & 0x1F) != TYPE_ENCLOSURE)) {
        fprintf(stderr, "Device %s not an enclosure\n", devpath);
        usage();
        exit(1);
    }
    printf("Updating expander\n");
    page = malloc(CHUNKSIZE);
    memset(page, 0, CHUNKSIZE);

    fseek(fwfile, 0, SEEK_END);
    imagesize = ftell(fwfile);
    rewind(fwfile);

    if (imagesize <= 0) {
        fprintf(stderr, "Bad image size\n");
        usage();
        exit(1);
    }

    fflush(stdout);
    for (i = 0, offset = 0; offset < imagesize; ++i) {
        int chunksize;
        unsigned char cdb[10];

        if ((imagesize - offset) < CHUNKSIZE)
            chunksize = imagesize - offset;
        else
            chunksize = CHUNKSIZE;

        cdb[0] = WRITE_BUFFER;
        cdb[1] = 0x07;
        cdb[2] = 0;
        store_be_n(offset, &cdb[3], 3);
        store_be_n(chunksize, &cdb[6], 3);
        cdb[9] = 0;
        ret = fread(page, 1, chunksize, fwfile);
        if (ret != chunksize) {
            printf("Fread returned %d, expected %d\n",
                   ret, chunksize);
            ret = -2;
            break;
        }
        ret = do_scsi_io_wait(handle, cdb, sizeof cdb, page,
                              SG_DXFER_TO_DEV, chunksize);
        if (ret < 0)
            break;
        offset += chunksize;

        printf("%d    \r", i);
        fflush(stdout);
    }
    printf("\n");
    if (ret < 0) {
        fprintf(stderr, "XFER FAILED WITH %d, EXITING\n", ret);
        exit(1);
    }

    printf("done\n");
    free(page);

    close(handle);
    fclose(fwfile);
    return 0;
}

/***
 ** vi:se sw=8 ts=8 noexpandtab
 */
