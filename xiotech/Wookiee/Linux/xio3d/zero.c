/* $Id: zero.c 4298 2005-05-04 18:53:47Z RysavyR $ */
/*
 * test.c: test xio3d kernel module.
 *
 * Copyright 2004 Xiotech Corporation
 *
 * Mark Rustad (Mark_Rustad@Xiotech.com)
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/pci.h>
#include <linux/ioport.h>
#include <string.h>

#include "xio3d.h"

#define DEVFILE "/dev/xio3d0"
#define MEMFILE "/dev/mem"
#define PCIFILE "/proc/bus/pci/devices"

#define XIO3D_NIO_MAX   16  /* Maximum number of I/O devices */
#define MEG     (1024 * 1024)

struct xio3d_devs
{
    unsigned short  busdevfn;
    unsigned short  vendor;
    unsigned short  device;
    unsigned long   irq;
    unsigned long   start[7];
    unsigned long   len[7];
    unsigned long   flags[7];
};

static struct xio3d_drvinfo    xio3d_drvinfo;
static struct xio3d_devs       xio3d_devs[XIO3D_NIO_MAX];

static unsigned long   PAGE_MASK;
static unsigned long   PAGE_SIZE;

static unsigned long   *mr[XIO3D_NSHM_MAX];

#ifndef PCI_DEVICE_ID_QLOGIC_ISP2300
#define PCI_DEVICE_ID_QLOGIC_ISP2300    0x2300
#endif
#ifndef PCI_DEVICE_ID_QLOGIC_ISP2312
#define PCI_DEVICE_ID_QLOGIC_ISP2312    0x2312
#endif
#ifndef PCI_DEVICE_ID_QLOGIC_ISP2322
#define PCI_DEVICE_ID_QLOGIC_ISP2322    0x2322
#endif

#ifndef PCI_VENDOR_ID_MICRO_MEMORY
#define PCI_VENDOR_ID_MICRO_MEMORY      0x1332
#endif
#ifndef PCI_DEVICE_ID_MICRO_MEMORY_5425CN
#define PCI_DEVICE_ID_MICRO_MEMORY_5425CN   0x5425
#endif

/*
 * xio3d_pcidevs holds the identities of the PCI HBAs that we support.
 *
 * This table is used by get_device_info to identify interesting devices.
 */

static const struct
{
    unsigned short  vendor;
    unsigned short  device;
} xio3d_pcidevs[] =
{
    {PCI_VENDOR_ID_QLOGIC, PCI_DEVICE_ID_QLOGIC_ISP2100},
    {PCI_VENDOR_ID_QLOGIC, PCI_DEVICE_ID_QLOGIC_ISP2200},
    {PCI_VENDOR_ID_QLOGIC, PCI_DEVICE_ID_QLOGIC_ISP2300},
    {PCI_VENDOR_ID_QLOGIC, PCI_DEVICE_ID_QLOGIC_ISP2312},
    {PCI_VENDOR_ID_QLOGIC, PCI_DEVICE_ID_QLOGIC_ISP2322},
//    {PCI_VENDOR_ID_MICRO_MEMORY, PCI_DEVICE_ID_MICRO_MEMORY_5425CN},
    {0, 0}      /* Terminate list */
};


static void    dumpsome(void *mem, int amount)
{
    int offset;
    unsigned short  *s;

    for (offset = 0; offset < amount; offset += 16)
    {
        s = (unsigned short *)(mem + offset);
        fprintf(stderr, "%04X: %04X %04X %04X %04X  "
            "%04X %04X %04X %04X\n",
            offset, *s, *(s + 1), *(s + 2),
            *(s + 3), *(s + 4), *(s + 5),
            *(s + 6), *(s + 7));
    }
}


int main(int argc, char *argv[])
{
    int i;
    int xiofd;
    void    *mem;

    PAGE_SIZE = getpagesize();
    PAGE_MASK = ~(PAGE_SIZE - 1);
    fprintf(stderr, "PAGE_MASK=%#08lX\n", PAGE_MASK);
    xiofd = open(DEVFILE, O_RDONLY);
    if (xiofd < 0)
    {
        fprintf(stderr, "open of %s failed with %d\n", DEVFILE, xiofd);
        perror("open failed");
        return xiofd;
    }
    if (ioctl(xiofd, XIO3D_GETINF, &xio3d_drvinfo) == -1)
    {
        fprintf(stderr, "ioctl failed with %d\n", errno);
        perror("ioctl failed");
        return errno;
    }

    fprintf(stderr, ".id=%08lX\n", xio3d_drvinfo.id);
    for (i = 0; i < XIO3D_NSHM_MAX; ++i)
    {
        if (xio3d_drvinfo.mem_regions[i].offset == 0)
        {
            continue;
        }
        fprintf(stderr, ".mem_regions[%d].offset=%08lX\n", i,
            xio3d_drvinfo.mem_regions[i].offset);
        fprintf(stderr, ".mem_regions[%d].size=%08lX\n", i,
            xio3d_drvinfo.mem_regions[i].size);
        fprintf(stderr, ".mem_regions[%d].phys=%08lX\n", i,
            xio3d_drvinfo.mem_regions[i].phys);
    }

    for (i = 0; i < XIO3D_NIO_MAX; ++i)
    {
        int j;
        struct xio3d_devs   *dev = &xio3d_devs[i];

        if (dev->vendor == 0)
        {
            continue;
        }
        fprintf(stderr, "device #%d: busdevfn=%04X, vendor=%04X, "
                "device=%04X, irq=%lu\n",
                i, dev->busdevfn, dev->vendor, dev->device, dev->irq);
        for (j = 0; j < 7; ++j)
        {
            if (dev->start[j] == 0)
            {
                continue;
            }
            fprintf(stderr, ".io_devs[%d].start[%d]=%08lx,"
                    "len=%08lx, flags=%08lx\n",
                    i, j, dev->start[j], dev->len[j], dev->flags[j]);
        }
    }

    for (i = 0; i < XIO3D_NSHM_MAX; ++i)
    {
        if (xio3d_drvinfo.mem_regions[i].offset)
        {
            unsigned long   *l;

            mem = mmap((void *)((i + 1) << 28),
                xio3d_drvinfo.mem_regions[i].size,
                PROT_READ, MAP_SHARED | MAP_LOCKED, xiofd,
                xio3d_drvinfo.mem_regions[i].offset);

            if (mem == MAP_FAILED)
            {
                fprintf(stderr, "mmap failed with %d\n", errno);
                perror("mmap failed");
                return errno;
            }

            l = mem;
            mr[i] = mem;
            fprintf(stderr, "mr[%d]=%p, .offset=%08lX\n", i, mem,
                xio3d_drvinfo.mem_regions[i].offset);
            dumpsome(mem, 128);
            bzero(mem,xio3d_drvinfo.mem_regions[i].size);
            fprintf(stderr, "zero?\n");
            dumpsome(mem, 128);
        }
    }


    return 0;
}
