/* $Id: test.c 4412 2005-06-01 17:22:27Z MiddenM $ */
/*
 * test.c: test xio3d kernel module.
 *
 * Copyright 2004 Xiotech Corporation
 *
 * Mark Rustad (Mark_Rustad@Xiotech.com)
 */

#define RUNON64

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/pci.h>

/* #define __KERNEL__ */
# define __deprecated           /* unimplemented */
#include <linux/ioport.h>

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

/* struct xio3d_drvinfo    xio3d_drvinfo; */
struct xio3d_drvinfo    *p_xio3d_drvinfo;
struct xio3d_devs       xio3d_devs[XIO3D_NIO_MAX];

int ndevs;
unsigned long   PAGE_MASK;
unsigned long   PAGE_SIZE;

unsigned long   *mr[XIO3D_NSHM_MAX];
void    *mmios[XIO3D_NIO_MAX][7];

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

const struct
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


/*
 * get_device_info - Look for supported devices in /proc/bus/pci/devices
 */

int get_device_info(void)
{
    FILE    *pcifd;
    char    line[512];

    pcifd = fopen(PCIFILE, "r");
    if (pcifd == 0)
    {
        fprintf(stderr, "open of %s failed with %d\n", MEMFILE, errno);
        perror("open failed");
        return errno;
    }

    while (fgets(line, sizeof(line), pcifd) != 0)
    {
        int ret;
        int i;
        int found;
        unsigned short  busdevfn;
        unsigned short  vendor;
        unsigned short  device;
        unsigned long   irq;
        unsigned long   start[7];
        unsigned long   len[7];
        struct xio3d_devs   *dev = &xio3d_devs[ndevs];

        ret = sscanf(line, "%4hx %4hx%4hx %lx "
            "%lx %lx %lx %lx %lx %lx %lx "
            "%lx %lx %lx %lx %lx %lx %lx ",
            &busdevfn, &vendor, &device, &irq,
            &start[0], &start[1], &start[2], &start[3], &start[4],
            &start[5], &start[6],
            &len[0], &len[1], &len[2], &len[3], &len[4], &len[5],
            &len[6]);
        if (ret != 18)
        {
            fprintf(stderr, "sscanf failure, ret=%d\n", ret);
            fclose(pcifd);
            return -1;
        }

        found = 0;
        for (i = 0; xio3d_pcidevs[i].vendor != 0; ++i)
        {
            if (vendor == xio3d_pcidevs[i].vendor &&
                    device == xio3d_pcidevs[i].device)
            {
                found = 1;
                break;
            }
        }

        if (found)
        {
            fprintf(stdout, "Found device #%d, busdevfn:%04x\n",
                    ndevs, busdevfn);
            if (ndevs >= XIO3D_NIO_MAX)
            {
                fprintf(stderr, "Too many devices\n");
                fclose(pcifd);
                return -1;
            }
            dev->busdevfn = busdevfn;
            dev->vendor = vendor;
            dev->device = device;
            dev->irq = irq;
            for (i = 0; i < 7; ++i)
            {
                unsigned long   amask;

                if (start[i] == 0)
                {
                    dev->flags[i] = 0;
                    dev->start[i] = 0;
                    dev->len[i] = 0;
                    continue;
                }
                if ((start[i] & PCI_BASE_ADDRESS_SPACE))
                {
                    amask = PCI_BASE_ADDRESS_IO_MASK;
                    dev->flags[i] = IORESOURCE_IO;
                }
                else
                {
                    amask = PCI_BASE_ADDRESS_MEM_MASK;
                    dev->flags[i] = IORESOURCE_MEM;
                }
                dev->start[i] = start[i] & amask;
                dev->len[i] = len[i];
            }
            ++ndevs;
        }
    }
    fclose(pcifd);
    return 0;
}


void    dumpsome(void *mem, int amount)
{
    int offset;
    unsigned short  *s;

    for (offset = 0; offset < amount; offset += 16)
    {
        s = (unsigned short *)(mem + offset);
        fprintf(stdout, "%08X: %04X %04X %04X %04X  "
                "%04X %04X %04X %04X\n",
                (ulong) mem + offset,
                s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]);
    }
}


void printinfo(char* string, struct xio3d_drvinfo *info)
{
    int i;

    fprintf(stdout, "%s  (%lx)\n", string, (ulong)info);

    fprintf(stdout, ".id=%08lX\n", info->id);
    for (i = 0; i < XIO3D_NSHM_MAX; ++i)
    {
        if (info->mem_regions[i].offset == 0)
        {
            continue;
        }

        fprintf(stdout, ".mem_regions[%d].offset=%08lX\n", i,
                info->mem_regions[i].offset);
        fprintf(stdout, ".mem_regions[%d].size=%08lX\n", i,
                info->mem_regions[i].size);
        fprintf(stdout, ".mem_regions[%d].phys=%08lX\n", i,
                info->mem_regions[i].phys);
        fprintf(stdout, ".mem_regions[%d].kmem=%08lX\n", i,
                info->mem_regions[i].kmem);
    }

    fprintf(stdout, ".info_region.offset=%08lX\n",
            info->info_region.offset);
    fprintf(stdout, ".info_region.size=%08lX\n",
            info->info_region.size);
    fprintf(stdout, ".info_region.phys=%08lX\n",
            info->info_region.phys);
    fprintf(stdout, ".info_region.kmem=%08lX\n",
            info->info_region.kmem);
}


int main(int argc, char *argv[])
{
    int i;
    int xiofd;
    int memfd;
    unsigned int    irqmask;
    unsigned int    data;
    int err;
    void    *mem;
    struct xio3d_drvinfo *info;

    PAGE_SIZE = getpagesize();
    PAGE_MASK = ~(PAGE_SIZE - 1);
    fprintf(stdout, "PAGE_MASK=%#08lX\n", PAGE_MASK);

    xiofd = open(DEVFILE, O_RDWR);
    if (xiofd < 0)
    {
        fprintf(stderr, "open of %s failed with %d\n", DEVFILE, xiofd);
        perror("open failed");
        return xiofd;
    }

    memfd = open(MEMFILE, O_RDONLY);
    if (memfd < 0)
    {
        fprintf(stderr, "open of %s failed with %d\n", MEMFILE, memfd);
        perror("open failed");
        return memfd;
    }

    /*
     * Get PCI device information from /proc/bus/pci/devices
     */

    fprintf(stdout, "get PCI device info\n");
    if (get_device_info())
    {
        fprintf(stderr, "get_device_info failed\n");
        return -1;
    }

    /*
     * Get information from xio3d module.
     */

    p_xio3d_drvinfo = (struct xio3d_drvinfo *)mmap(0, XIO3D_INFO_SIZE,
						   PROT_READ, MAP_SHARED | MAP_LOCKED,
						   xiofd, XIO3D_INFO_OFFSET);

fprintf(stderr, "after mmap, p_xio3d_drvinfo=%p\n", p_xio3d_drvinfo);

#if 0
    p_xio3d_drvinfo = &xio3d_drvinfo;
    if (ioctl(xiofd, XIO3D_GETINF, p_xio3d_drvinfo) == -1)
    {
        fprintf(stderr, "ioctl failed with %d\n", errno);
        perror("ioctl failed");
        return errno;
    }

    /*
     * Print memory region information.
     */

    printinfo("Info retrieved with ioctl", p_xio3d_drvinfo);

#endif /* 0 */
    /*
     * Print PCI device information.
     */

    for (i = 0; i < XIO3D_NIO_MAX; ++i)
    {
        int j;
        struct xio3d_devs   *dev = &xio3d_devs[i];

        if (dev->vendor == 0)
        {
            continue;
        }

        fprintf(stdout, "device #%d: busdevfn=%04X, vendor=%04X, "
                "device=%04X, irq=%lu\n",
                i, dev->busdevfn, dev->vendor, dev->device, dev->irq);

        for (j = 0; j < 7; ++j)
        {
            if (dev->start[j] == 0)
            {
                continue;
            }
            fprintf(stdout, ".io_devs[%d].start[%d]=%08lx,"
                    "len=%08lx, flags=%08lx\n",
                    i, j, dev->start[j], dev->len[j], dev->flags[j]);
        }
    }

#if 0
    /*
     * Map info region and print it again.
     */

    mem = mmap((void*)p_xio3d_drvinfo->info_region.offset,
               p_xio3d_drvinfo->info_region.size,
               PROT_READ, MAP_SHARED | MAP_LOCKED, xiofd,
               p_xio3d_drvinfo->info_region.offset);

    if (mem == MAP_FAILED)
    {
        fprintf(stderr, "info mmap failed with %d\n", errno);
        perror("mmap failed");
        return errno;
    }
#else /* 0 */
    mem= (void *)p_xio3d_drvinfo;
#endif /* 0 */

    fprintf(stdout, "Info region mapped at: %lx\n", mem);

    printinfo("Info in mapped memory", (struct xio3d_drvinfo*) mem);

    dumpsome(mem,128);

    info = mem;

#if 0
    fprintf(stdout, "Hoping to get a segmentation fault...\n");
    *(ulong*)0x50000f00 = 12;

    mprotect(mem, 4096, PROT_READ);

    fprintf(stdout, "Still trying...\n");
    *(ulong*)0x50000f00 = 12;
#endif

    /*
     * Map shared memory segments using xio3d module and print a bit.
     */

    for (i = 0; i < XIO3D_NSHM_MAX; ++i)
    {
        if (p_xio3d_drvinfo->mem_regions[i].offset)
        {
            mem = mmap((void *)((i + 1) << 28),
                p_xio3d_drvinfo->mem_regions[i].size,
                PROT_READ, MAP_SHARED | MAP_LOCKED | MAP_DENYWRITE, xiofd,
                p_xio3d_drvinfo->mem_regions[i].offset);

            if (mem == MAP_FAILED)
            {
                fprintf(stderr, "mmap failed with %d\n", errno);
                perror("mmap failed");
                return errno;
            }

            mr[i] = mem;
            fprintf(stdout, "mr[%d]=%p, .offset=%08lX\n", i, mem,
                p_xio3d_drvinfo->mem_regions[i].offset);
            if (p_xio3d_drvinfo->mem_regions[i].size > 1) {
	        dumpsome(mem, 128);
	    } else {
	        fprintf(stdout, "no memory region.\n");
	    }
        }
    }

    /*
     * Map PCI device registers using /dev/mem device and print a bit.
     */

#ifdef redhat
    fprintf(stdout, "mmapping devices through /dev/mem\n");
    for (i = 0; i < XIO3D_NIO_MAX; ++i)
    {
        struct xio3d_devs   *dev = &xio3d_devs[i];
        int j;
        unsigned long   addr;
        void    *mmio;

        if (dev->vendor == 0)
        {
            continue;
        }
        for (j = 0; j < 6; ++j)
        {
            if ((dev->flags[j] & IORESOURCE_MEM) == 0)
            {
                continue;
            }
            addr = 0x70000000 + (i << 24) + (j << 20);
            fprintf(stdout, "mapping %08lx to %08lx\n", addr,
                    dev->start[j]);
            mmio = mmap((void *)addr, dev->len[j] < 4096 ? 4096 : dev->len[j],
                    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED,
                    memfd, dev->start[j]);
            if (mmio == MAP_FAILED)
            {
                fprintf(stderr, "mmap failed, %d\n", errno);
                perror("mmap failed");
                return errno;
            }
            if (mmio != (void *)addr)
            {
                fprintf(stderr, "different address, %p != %08lx\n",
                        mmio, addr);
                return -1;
            }
            fprintf(stdout, "Got addr=%p\n", mmio);
            mmios[i][j] = mmio;
            dumpsome(mmio, 128);
        }
    }

    fprintf(stdout, "mmaps done>");
    getchar();
#endif

    /*
     * Register for device interrupts as xio3d events.
     */

    fprintf(stdout, "About to grab interrupts\n");
    irqmask = 0;
    for (i = 0; i < XIO3D_NIO_MAX; ++i)
    {
        struct xio3d_devs   *dev = &xio3d_devs[i];
        int irq;
        int err;

        if (dev->vendor == 0)
        {
            continue;
        }
        irq = dev->irq;
        err = ioctl(xiofd, XIO3D_REGINT, &irq);
        if (err)
        {
	    perror("ioctl");
            fprintf(stderr, "ioctl failed for device #%d, err=%d\n", i, err);
            return err;
        }
        irqmask |= (1 << irq);
        fprintf(stdout, "ioctl returned irq #%d for device #%d\n", irq, i);
    }
    fprintf(stdout, "Final irqmask=%04x\n", irqmask);

    fprintf(stdout,
            "After register four irqs: reg=%08x, mask=%08x, active=%08x\n",
            info->reg, info->mask, info->active);

    /*
     * Register for two xio3d events.
     */

    err = ioctl(xiofd, XIO3D_REGEVT, XIO3D_EVT_BASE + 0);
    if (err != 0)
    {
        fprintf(stderr, "event ioctl returned %d, errno=%d\n", err, errno);
        perror("event ioctl failed");
        return -1;
    }

    err = ioctl(xiofd, XIO3D_REGEVT, XIO3D_EVT_BASE + 1);
    if (err != 0)
    {
        fprintf(stderr, "event ioctl returned %d, errno=%d\n", err, errno);
        perror("event ioctl failed");
        return -1;
    }

    fprintf(stdout,
            "After reg 01: reg=%08x, mask=%08x, active=%08x\n",
            info->reg, info->mask, info->active);

    /*
     * Test xio3d reading device.
     */

    err = read(xiofd, &data, 1); /* This should error */
    if (err != -1)
    {
        fprintf(stderr, "read did not return error, returned %d\n", err);
        return -1;
    }

    /*
     * Send ourselves an event.
     */

    err = ioctl(xiofd, XIO3D_SENDEVT, XIO3D_EVT_BASE + 0);
    if (err != 0)
    {
        fprintf(stderr, "send event ioctl returned %d, errno=%d\n",
                err, errno);
        perror("send event ioctl failed");
        return -1;
    }

    fprintf(stdout,
            "After send 0: reg=%08x, mask=%08x, active=%08x\n",
            info->reg, info->mask, info->active);

    /*
     * Receive the event.
     */

    err = read(xiofd, &data, sizeof data);
    if (err == -1)
    {
        fprintf(stderr, "read returned error, errno=%d\n", errno);
        return -1;
    }
    fprintf(stdout, "data=%08x\n", data);

    fprintf(stdout,
            "After read: reg=%08x, mask=%08x, active=%08x\n",
            info->reg, info->mask, info->active);

    /*
     * Test irq/event masking.
     *
     * Initially, get event mask, setting it to 0. Using the returned mask,
     * turn off one event and set that mask.
     */

    irqmask = 0;
    err = ioctl(xiofd, XIO3D_MASKEVT, &irqmask);
    if (err)
    {
        fprintf(stdout, "irqmask failed with %d, errno=%d\n", err, errno);
        return -1;
    }

    fprintf(stdout, "irqmask=%04X\n", irqmask);

    fprintf(stdout,
            "After mask all: reg=%08x, mask=%08x, active=%08x\n",
            info->reg, info->mask, info->active);

    data = irqmask;
    irqmask &= ~(1 << (XIO3D_EVT_BASE + 0));    /* Mask off one event */

    err = ioctl(xiofd, XIO3D_MASKEVT, &irqmask);
    if (err)
    {
        fprintf(stderr, "irqmask failed with %d, errno=%d\n", err, errno);
        return -1;
    }

    irqmask = data;

    fprintf(stdout,
            "After mask 0: reg=%08x, mask=%08x, active=%08x\n",
            info->reg, info->mask, info->active);

    /*
     * Send ourselves an event - the masked one.
     */

    err = ioctl(xiofd, XIO3D_SENDEVT, XIO3D_EVT_BASE + 0);
    if (err != 0)
    {
        fprintf(stderr, "send event ioctl returned %d, errno=%d\n",
                err, errno);
        perror("send event ioctl failed");
        return -1;
    }

    fprintf(stdout,
            "After send 0: reg=%08x, mask=%08x, active=%08x\n",
            info->reg, info->mask, info->active);

    /*
     * Send ourselves another event - the unmasked one.
     */

    err = ioctl(xiofd, XIO3D_SENDEVT, XIO3D_EVT_BASE + 1);
    if (err != 0)
    {
        fprintf(stderr, "send event ioctl returned %d, errno=%d\n",
                err, errno);
        perror("send event ioctl failed");
        return -1;
    }

    fprintf(stdout,
            "After send 1: reg=%08x, mask=%08x, active=%08x\n",
            info->reg, info->mask, info->active);

    /*
     * Receive events - should receive only the unmasked event.
     */

    err = read(xiofd, &data, sizeof data);
    if (err == -1)
    {
        fprintf(stderr, "read returned error, errno=%d\n", errno);
        return -1;
    }
    fprintf(stdout, "read returned %04x\n", data);

    fprintf(stdout,
            "After read: reg=%08x, mask=%08x, active=%08x\n",
            info->reg, info->mask, info->active);

    /*
     * Re-enable the previously disabled event.
     */

    data = irqmask;
    irqmask |= 1 << (XIO3D_EVT_BASE + 0);   /* Enable event again */
    irqmask = -1;
    err = ioctl(xiofd, XIO3D_MASKEVT, &irqmask);
    if (err)
    {
        fprintf(stderr, "mask ioctl returned %d, errno=%d\n",
                err, errno);
        return -1;
    }

    fprintf(stdout, "old irqmask=%04X, new irqmask=%04X\n",
            irqmask, data);

    fprintf(stdout,
            "After unmask 0: reg=%08x, mask=%08x, active=%08x\n",
            info->reg, info->mask, info->active);

    /*
     * Receive events again - see what we get.
     */

    err = read(xiofd, &data, sizeof data);
    if (err == -1)
    {
        fprintf(stderr, "read returned error, errno=%d\n", errno);
        return -1;
    }

    fprintf(stdout, "read returned %04x\n", data);

    fprintf(stdout,
            "After read: reg=%08x, mask=%08x, active=%08x\n",
            info->reg, info->mask, info->active);

#if 0
    data = 0;
    for (i = 0; i < 10000000; i++) {
        write(xiofd, &data, 4);
    }
#endif

#if 0
    data = 0;
    for (i = 0; i < 10000000; i++) {
        ioctl(xiofd, XIO3D_SENDEVT, XIO3D_EVT_BASE + 1);
    }
#endif
    
    /*
     * Close the file descriptor, then unmap all the segments.
     */

    if (close(xiofd) != 0)
    {
        perror("close failed");
        return errno;
    }

    fprintf(stdout, "close done\n");

    fprintf(stdout,
            "After close: reg=%08x, mask=%08x, active=%08x\n",
            info->reg, info->mask, info->active);

    for (i = 0; i < XIO3D_NSHM_MAX; ++i)
    {
        if (mr[i])
        {
            int ret;

            ret = munmap(mr[i], p_xio3d_drvinfo->mem_regions[i].size);
            if (ret)
            {
                perror("munmap failed");
            }
            fprintf(stdout, "munmap[%d] returned %d\n", i, ret);
        }
    }

    err = munmap(info, info->info_region.size);
    fprintf(stdout, "munmap(info) returned %d\n", err);

    return 0;
}
