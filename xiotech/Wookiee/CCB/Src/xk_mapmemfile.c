/* $Id: xk_mapmemfile.c 142341 2010-06-10 20:26:58Z mdr $ */
/**
******************************************************************************
**
**  @file       xk_mapmemfile.c
**
**  @brief      This module allows us to map memory to files
**
**  Copyright (c) 2004-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

/* XIOtech includes */
#include "xk_mapmemfile.h"

#include "debug_files.h"
#include "error_handler.h"
#include "heap.h"
#include "XIO_Const.h"
#include "XIO_Types.h"

/* Linux includes */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/*
******************************************************************************
** Private defines
******************************************************************************
*/
#define PAGE_SIZE   4096

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

#undef  close   /* Close() can't work here, so allow close */

/**
******************************************************************************
**
**  @brief      Initialize mapped memory to file
**
**  Initialize mapped memory to file. If the file does not exist
**  it is created at the length specified. If the file does exist
**  it is retrieved. The pointer returned will be mapped to the
**  start of the file.
**
**  @param      fName   - Name of file to use.
**  @param      length  - Length of Mapped region.
**  @param      fill    - Character pattern to fill if new.
**
**  @return     SUCCESS - pointer to the beginning of the mapped memory
**  @return     FAILURE - NULL
**
**  @attention  All writes to the memory will be written to the file by the OS.
**              If you want to immediately flush call MEM_FlushMapFile
**
******************************************************************************
**/
void *MEM_InitMapFile(const char *fName, INT32 length, UINT8 fill, void *start)
{
    void    *mapMem     = NULL;
    UINT8   new         = 1;
    int     fd;
    INT32   fileLength  = (length + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    char    pageSetBfr[PAGE_SIZE];

    if (!fName)
    {
        return NULL;
    }

    if (getpagesize() != PAGE_SIZE)
    {
        dprintf(DPRINTF_DEFAULT, "%s: Wrong page size = %d\n",
                __func__, getpagesize());
        abort();
    }

    /*
     * Open the file, if the file does not exist this will error
     * and we will take the new file path.
     */
    fd = open(fName, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd >= 0)
    {
        UINT32      statRetries = 20;
        struct stat fInfo;

        /* We just successfully opened the file, so this is not a new file */

        new = 0;

        /*
         * We will look at the size of the existing file,
         * if it is different than the size being requested,
         * blow it away, set new to 1 so it gets created below.
         */
        if (fstat(fd, &fInfo) < 0)
        {
            /*
             * If we cannot read the file statistics, something
             * is wrong.  Close the file and retreat.
             */
            dprintf(DPRINTF_DEFAULT, "%s - Unable to get file size of %s\n",
                    __func__, fName);

            Close(fd);
            return NULL;
        }

        while (fInfo.st_size != fileLength || fInfo.st_size == 0)
        {
            dprintf(DPRINTF_DEFAULT, "%s - File %s different size than req\n"
                    "                     ... req = 0x%08X, size = 0x%08X\n",
                    __func__, fName, fileLength, (UINT32)fInfo.st_size);

            TaskSleepMS(1000);
            fstat(fd, &fInfo);

            if (--statRetries == 0)
            {
                break;
            }
        }

        /*
         * We need to check the length of the existing file against
         * the requested length.  If they are not equal, than we
         * have to destroy the current file, and replace it with
         * a brand new file of the correct length,
         */
        if (fInfo.st_size != fileLength)
        {
            dprintf(DPRINTF_DEFAULT, "%s - File %s different size than req\n"
                    "                     ... req = 0x%08X, size = 0x%08X\n",
                    __func__, fName, fileLength, (UINT32)fInfo.st_size);

            /*
             * We have contradicting lengths, so close the current file
             * and delete it.  Set new to 1 so it will be created below.
             */
            close(fd);
            fd = -1;
            unlink(fName);

            new = 1;
        }
    }

    /* If the file does not already exist, create it */

    if (new)
    {
        int     pageCnt;

        fd = open(fName, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
        if (fd < 0)
        {
            dprintf(DPRINTF_DEFAULT, "%s: Open of %s failed with %d (%s)\n",
                    __func__, fName, errno, strerror(errno));
            return NULL;
        }

        memset(pageSetBfr, fill, sizeof(pageSetBfr));

        for (pageCnt = 0; pageCnt < fileLength; pageCnt += sizeof(pageSetBfr))
        {
            int rc;

            rc = write(fd, pageSetBfr, sizeof(pageSetBfr));
            if (rc != sizeof(pageSetBfr))
            {
                dprintf(DPRINTF_DEFAULT, "%s: write returned %d with %d (%s)\n",
                        __func__, rc, errno, strerror(errno));
                goto err;
            }
        }
    }

    if (fd >= 0)
    {
        int     mmapFlags;

        /* Create the memory map of the file */

        mmapFlags = MAP_SHARED;

        if (start)
        {
            mmapFlags |= MAP_FIXED;
        }

        mapMem = mmap(start, fileLength, PROT_READ | PROT_WRITE, mmapFlags,
                    fd, 0);

        /* ERROR - This is a system failure at this point */

        if (mapMem == MAP_FAILED)
        {
            dprintf(DPRINTF_DEFAULT, "%s: Error mapping %d bytes "
                   "mem to file %s, %d (%s)\n", __func__, length, fName,
                    errno, strerror(errno));
            mapMem = NULL;
        }

        close(fd);
    }

    return mapMem;

err:
    close(fd);
    unlink(fName);
    return NULL;
}


/**
******************************************************************************
**
**  @brief      Flush mapped memory to file
**
**  @param      pMemToFlush - pointer to start of mem segment to flush.
**  @param      length      - Length of region to flush.
**
**  @return     none
**
******************************************************************************
**/
void MEM_FlushMapFile(void *pMemToFlush, UINT32 length)
{
    int     rc;
    UINT32  pageAdjust;

    /* Asynchronous flush, make the call. */

    /* Calculate the page boundary */

    pageAdjust = (UINT32)pMemToFlush % getpagesize();

    /* Simple, if the mem is not NULL flush it. */

    if (!pMemToFlush)
    {
        return;
    }

    /* Adjust the input to align with a page boundary */

    rc = msync((char *)pMemToFlush - pageAdjust, (size_t)length + pageAdjust,
                MS_ASYNC);
    if (rc)
    {
        dprintf(DPRINTF_DEFAULT, "msync addr=%p, length=0x%08x, errno=%d\n",
                pMemToFlush, length, errno);
        dprintf(DPRINTF_DEFAULT, "msync adjusted addr=0x%08x, length=0x%08x\n",
                (UINT32)pMemToFlush - pageAdjust, length + pageAdjust);
        dprintf(DPRINTF_DEFAULT, "pagesize = 0x%08x\n", getpagesize());
    }
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
