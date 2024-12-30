/* $Id: li_pci.c 144191 2010-07-15 20:23:53Z steve_wirtz $ */
/**
******************************************************************************
**  @file       li_pci.c
**
**  @brief      Linux Interface for PCI operations
**
**  Linux interface code for PCI operations. Aside from initial device
**  discovery, the important operations provided are the config space
**  accesses, because they are used for configuration and also when adapters
**  have trouble.
**
**  Copyright 2004-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "options.h"
#include "kernel.h"
#include <stdint.h>
#include "li_pci.h"
#include "li_evt.h"
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/pci.h>
#ifndef PCI_VENDOR_ID_QLOGIC
  #define PCI_VENDOR_ID_QLOGIC          0x1077
#endif  /* PCI_VENDOR_ID_QLOGIC */
#include <string.h>
#include <stdlib.h>
#include "LKM_layout.h"
#include "xio3d.h"
#include "isp.h"
#ifdef GCOV
extern int CT_ioctl(int, unsigned long, void *);
#else
#define CT_ioctl ioctl
#endif

/*
******************************************************************************
** Private defines
******************************************************************************
*/
#define DEVFILE "/dev/xio3d0"
#define MEMFILE "/dev/mem"
#define PCIFILE "/proc/bus/pci/devices"

#ifndef FALSE
#define FALSE   0
#define TRUE    1
#endif  /* FALSE */

#define PCI_COMMAND 0x04            /* Offset to PCI Command register */

#define IORESOURCE_IO   0x0100
#define IORESOURCE_MEM  0x0200

#define INTEL_ID    0x8086      /* Intel vendor ID as kludge for enet */

/*
******************************************************************************
** Private variables
******************************************************************************
*/
struct pci_devs             gPCIdevs[XIO3D_NIO_MAX];
INT32                       pci_devs_index = 0;
UINT32                      pci_dev_micro_memory = 0;
#ifndef PAM
static INT32                gGotInfo;       /* If gxio3d_drvinfo valid */
static struct xio3d_drvinfo gxio3d_drvinfo;
#endif /* PAM */

extern INT32                CT_xiofd;       /* xio3d file descriptor */
#ifndef NOAFFINITY
extern UINT32               gCpuAffinity;   /* Cpu affinity for this process */
#endif /* NOAFFINITY */

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/

extern  void isp$r23qx(void *);

int     resetQlogic(int);

UINT32  getDMAAddr(int);
void    setdevices(const char *);
void    LI_RegisterIRQ(UINT32 id, void *func, UINT32 value);


/*
******************************************************************************
** Code Start
******************************************************************************
*/

/*
******************************************************************************
** getDMAAddr
**
**  @brief  Get DMA address for shared memory segment
**
**  This function returns the physical memory address for a given shared
**  memory segment.
**
**  @param  i   - Shared memory segment to get physical address for
**
******************************************************************************
**/

#ifndef PAM

UINT32 getDMAAddr(int i)
{
    if (!gGotInfo)
    {
        if (CT_ioctl(CT_xiofd, XIO3D_GETINF, &gxio3d_drvinfo) == -1)
        {
            perror("getDMAAddr: ioctl");
            fprintf(stderr, "getDMAAddr: ioctl of %s with %lu failed\n",
                    DEVFILE, XIO3D_GETINF);
            exit(1);
        }
        gGotInfo = 1;
    }
    if (gxio3d_drvinfo.mem_regions[i].phys == 0)
    {
        fprintf(stderr, "getDMAAddr: phys 0\n");
        exit(1);
    }
    return gxio3d_drvinfo.mem_regions[i].phys;
}

#endif /* PAM */

/*
******************************************************************************
** setdevices
**
**  @brief  Set PCI device numbers from environment variable
**
**  This function extracts bus numbers from the given environment
**  variable and adds them to the table devices.
**
**  @param  name    - Environment variable to parse
**
******************************************************************************
**/

void setdevices(const char *name)
{
    char    *value;

    value = getenv(name);
    if (value == 0 || *value == 0)
    {
        fprintf(stderr, "setdevices: Bus environment not set (%s)\n", name);
        return;
    }
    while (*value != 0)
    {
        int bus;
        int dev;
        int func;
        struct pci_devs *pcidev;

        bus = 0;
        dev = 0;
        func = 0;
        pcidev = &gPCIdevs[pci_devs_index];

        if (pci_devs_index >= XIO3D_NIO_MAX)
        {
            fprintf(stderr, "setdevices: Too many devices specified (%s)\n",
                name);
            return;
        }

        if (isalpha(*value))
        {
            int len = 0;

            while (*value && *value != ':' && len < ENET_NAME_SIZE - 1)
            {
                pcidev->enetname[len] = *value;
                ++len;
                ++value;
            }
            pcidev->enetname[len] = 0;
            pcidev->busdevfn = ENET_BUSDEVFN;
            fprintf(stderr, "setdevices: #%d (%s) Using device %s\n",
                pci_devs_index, name, pcidev->enetname);
        }
        else
        {
            int nfield;

            nfield = sscanf(value, "%x/%x.%x:", &bus, &dev, &func);
            if (nfield != 3)
            {
                fprintf(stderr, "setdevices: Parsing %s variable failed, "
                    "nfield=%d, value=%s\n", name, nfield, value);
                return;
            }

            pcidev->busdevfn = (bus << 8) | (dev << 3) | func;

            fprintf(stderr, "setdevices: #%d (%s) Using device %02x/%02x.%x\n",
                        pci_devs_index, name, bus, dev, func);
        }

        ++pci_devs_index;

        while (*value != 0 && *value != ':')
            ++value;

        if (*value == ':')
            ++value;
    }
}


/*
******************************************************************************
** get_device_info
**
**  @brief  Look for supported devices in /proc/bus/pci/devices
**
**  This function reads /proc/bus/pci/devices, looking for all QLogic
**  devices in the interesting busses. The bus numbers are defined in
**  environment variables. The appropriate variable is parsed depending
**  on whether this is built into the front end or back end.
**
******************************************************************************
**/

INT32 get_device_info(void)
{
    int         i;
    FILE        *pcifd;
    static char called = 0;
    char        line[512];
    struct pci_devs *dev;

    if (called)     /* If previously called */
    {
        return 0;   /* Return success */
    }
    called = 1;     /* Indicate that this has been called */

    pcifd = fopen(PCIFILE, "r");
    if (pcifd == 0)
    {
        int save_errno = errno;

        perror("open failed");
        fprintf(stderr, "open of %s failed with %d\n", PCIFILE, save_errno);
        return save_errno;
    }

#if defined(FRONTEND)
    setdevices("FEDEVS");
#endif
#if defined(BACKEND)
    setdevices("BEDEVS");
#endif
    /* Save which device is the micro memory card for LI_AccessDevice. */
    pci_dev_micro_memory = pci_devs_index;
    setdevices("MICROMEMORY");
    if (pci_dev_micro_memory == (UINT32)pci_devs_index)
    {
        pci_dev_micro_memory = 0;               /* No micro_memory card present. */
    }

    while (fgets(line, sizeof(line), pcifd) != 0)
    {
        int             ret;
        unsigned long   busdevfn;
        UINT16          vendor;
        UINT16          device;
        unsigned long   irq;
        unsigned long   start[7];
        unsigned long   len[7];

        ret = sscanf(line, "%4lx %4hx%4hx %lx "
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
#ifdef MM_DEBUG
        if (busdevfn == 0x0520)
        {
            fprintf(stderr, "busdevfn:%lx, vendor:%hx, device:%hx, irq:%lx\n",
                   busdevfn, vendor, device, irq);
            {
            int j;
                for(j=0; j<7; j++)
                {
                     fprintf(stderr, "start[%d] = %lx  ", j, start[j]);
                }
                fprintf(stderr, "\n");
                for(j=0; j<7; j++)
                {
                     fprintf(stderr, "len[%d] = %lx  ", j, len[j]);
                }
                fprintf(stderr, "\n");
            }
       }
#endif


        for (i = 0, dev = &gPCIdevs[0]; i < XIO3D_NIO_MAX; ++i, ++dev)
        {
            if (busdevfn == dev->busdevfn)
            {
                break;
            }
        }
        if (i >= XIO3D_NIO_MAX)   /* If not one of our devices */
        {
            continue;       /* Continue looking for our devices */
        }
        if (busdevfn == 0)  /* Ignore root device */
        {
            continue;       /* Continue looking for our devices */
        }

        fprintf(stderr, "Found device #%d, busdevfn:%lx, "
                "vendor=%04X, device=%04X\n",
                i, busdevfn, vendor, device);
        dev->busdevfn = busdevfn;
        dev->vendor = vendor;
        dev->device = device;

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
        {
            char    path[128];

            snprintf(path, sizeof path, "/proc/bus/pci/%02lx/%02lx.%ld",
                    dev->busdevfn >> 8, (dev->busdevfn >> 3) & 0x1F,
                    dev->busdevfn & 0x07);
            fprintf(stderr, "Opening %s\n", path);
            dev->cfd = open(path, O_RDWR);
            if (dev->cfd == -1)
            {
                int save_errno = errno;

                perror("Open of /proc/bus/pci... failed");
                fprintf(stderr, "Failed to open %s, errno=%d\n", path, save_errno);
                return -1;
            }
        }
    }
    fclose(pcifd);

    /* Process iscsi devices */
    for (i = 0, dev = &gPCIdevs[0]; i < XIO3D_NIO_MAX; ++i, ++dev)
    {
        if (dev->busdevfn != ENET_BUSDEVFN)
        {
            continue;
        }
        fprintf(stderr, "Found enet device #%d, name=%s\n",
                i, dev->enetname);
        dev->vendor = INTEL_ID;     /* Kludge for now */
        dev->device = 0xFFFF;
    }

    return 0;
}


/*
******************************************************************************
** resetQlogic
**
**  @brief  Reset Qlogic device
**
**  @param  i   - index to device
**
**  @return 0 = success, else failure
**
**  This function resets the QLogic device indicated by the parameter.
**
******************************************************************************
**/

int resetQlogic(int i)
{
    struct pci_devs *dev;
    UINT16          data;
    volatile UINT16 *saddr;
    volatile UINT32 *ispregs;
    int             rc;
    int             memFd;          /* /dev/mem file descriptor */
    void            *maddr;
    size_t          len;
    UINT32          page_size;

    dev = &gPCIdevs[i];
    fprintf(stderr, "resetQlogic: [%d] = %X, resetting... ", i,
            dev->vendor | (dev->device << 16));
    rc = LI_GetConfig((unsigned)(i + PCIBASE), PCI_COMMAND, 2, (unsigned char *)&data);
    if (!rc)
    {
        fprintf(stderr, "failed\n");
        return 1;
    }

    data &= ~PCI_COMMAND_MASTER;    /* Disable bus master */
    rc = LI_SetConfig((unsigned)(i + PCIBASE), PCI_COMMAND, 2, (unsigned char *)&data);
    if (!rc)
    {
        fprintf(stderr, "failed.\n");
        return 2;
    }
    if (dev->start[1] == 0)
    {
        fprintf(stderr, "resetQlogic: Unknown space 1 for device %d\n",
                i + PCIBASE);
        return 3;
    }
    memFd = open(MEMFILE, O_RDWR);
    if (memFd == -1)
    {
        int save_errno = errno;

        fprintf(stderr, "resetQlogic: Open of %s failed with %d\n",
                MEMFILE, errno);
        errno = save_errno;             /* fprintf() might change errno. */
        perror("resetQlogic: Open of /dev/mem file failed");
        return 4;
    }
    page_size = getpagesize();
    len = dev->len[1] < page_size ? page_size : dev->len[1];
    maddr = mmap(0, len,
            PROT_READ | PROT_WRITE, MAP_SHARED, memFd,
            (signed)(dev->start[1]));
    if (maddr == MAP_FAILED)
    {
        fprintf(stderr,
                "resetQlogic: mmap of device %d, space 1 failed with %d\n",
                i + PCIBASE, errno);
        close(memFd);
        return 5;
    }
    fprintf(stderr, "mmaped maddr=%p (%zu) ...", maddr,len);
    close(memFd);
    saddr = maddr;
    if (dev->device > 0x2400)
    {
        ispregs = (UINT32*)maddr;
        ispregs[2] = ISP2400CSR_SOFT_RESET | ISP2400CSR_DMA_CONTROL;
        usleep(100);
        while (((ispregs[2])&(ISP2400CSR_SOFT_RESET)) == 1)   /* Wait for reset to finish */
        ;
    ispregs[18]= ISP2400HCCR_SET_RISC_RESET; /*set RISC reset*/
    ispregs[18]= ISP2400HCCR_REL_RISC_PAUSE;/* Release RISC pause */
    ispregs[18]= ISP2400HCCR_CLEAR_RISC_RESET; /* clear RISC reset */
    }

    else
    {
        saddr[3] = 1;           /* Initiate reset */
        while (saddr[3] == 1)   /* Wait for reset to finish */
        ;
    }
    rc = munmap(maddr, len);
    if (rc == -1)
    {
        int save_errno = errno;

        perror("resetQlogic: munmap failed");
        fprintf(stderr, "resetQlogic: munmap failed with %d\n", save_errno);
        return 6;
    }
    fprintf(stderr, " done\n");

    return 0;
}


/*
******************************************************************************
** LI_ScanBus
**
**  @brief  Scans the PCI bus for devices on Linux.
**
**  @param  pcitbl - address of PCI table in proc
**  @param  tblsize - Number of entries in PCI table
**
**  @return Bitmap of found devices
**
******************************************************************************
**/

UINT32 LI_ScanBus(pcidevtbl pcitbl[], UINT32 tblsize)
{
    UINT32          bitmap = 0;
    struct pci_devs *dev;
    UINT32          i;

    /*
    ** Get PCI device information from /proc/bus/pci/devices
    */
    fprintf(stderr, "%s:%u - Get PCI device info\n", __FILE__, __LINE__);
    if (get_device_info())
    {
        fprintf(stderr, "%s:%u - get_device_info failed\n",
                __FILE__, __LINE__);
        return 0;
    }

    for (i = 0, dev = &gPCIdevs[0]; i < XIO3D_NIO_MAX; ++i, ++dev)
    {
        if (dev->vendor == 0)   /* If no vendor, must not be a device */
        {
            continue;
        }
        if (i >= tblsize)
        {
            fprintf(stderr, "%s:%u - pcitbl overflow, i=0x%X\n",
                    __FILE__, __LINE__, i);
            break;
        }
        pcitbl[i + PCIOFFSET].vendev = dev->vendor | (dev->device << 16);
        bitmap |= 1 << (PCIBASE + i);
        if (dev->vendor == PCI_VENDOR_ID_QLOGIC)
        {
#ifndef PAM
            resetQlogic((int)i);
#endif
        }
    }

    return bitmap;
}


/*
******************************************************************************
** LI_GetConfig
**
**  @brief  Reads PCI configuration space on Linux.
**
**  This function uses /proc/bus/pci to access the PCI configuration space.
**
**  @param  id - Device ID (11-31) to interrogate
**  @param  off - PCI offset of header to read
**  @param  nbytes - Number of bytes to read
**  @param  data - Location to receive config data
**
**  @return FALSE if no data returned, TRUE if data returned
**
******************************************************************************
**/

INT32 LI_GetConfig(unsigned long id, unsigned long off, INT32 nbytes, UINT8 *data)
{
    struct pci_devs *dev;
    off_t   sresult;
    ssize_t rresult;

    if (id < PCIBASE)
    {
        fprintf(stderr, "LI_GetConfig: id=%ld\n", id);
        return FALSE;
    }
    id -= PCIBASE;
    if (id >= XIO3D_NIO_MAX)
    {
        fprintf(stderr, "LI_GetConfig: id=%ld > XIO3D_NIO_MAX\n", id);
        return FALSE;
    }
    dev = &gPCIdevs[id];
    if (dev->vendor == 0)
    {
        fprintf(stderr, "LI_GetConfig: Unknown device: id=%ld\n", id);
        return FALSE;
    }
    sresult = lseek(dev->cfd, (signed)off, SEEK_SET);
    if (sresult == (off_t)-1)
    {
        int save_errno = errno;

        perror("LI_GetConfig: lseek on config fd failed");
        fprintf(stderr, "LI_GetConfig: lseek on config fd failed with %d\n", save_errno);
        return FALSE;
    }
    rresult = read(dev->cfd, data, (unsigned)nbytes);
    if (rresult <= 0)
    {
        int save_errno = errno;

        perror("LI_GetConfig: read of config fd failed");
        fprintf(stderr, "LI_GetConfig: read of config fd failed with %d\n", save_errno);
        return FALSE;
    }

    return TRUE;
}


/*
******************************************************************************
** LI_SetConfig
**
**  @brief  Writes to PCI configuration space on Linux.
**
**  This function uses /proc/bus/pci to access PCI configuration
**  space. One must be running as root to write to PCI configuration
**  space with this function.
**
**  @param  id - Device ID (11-31) to set
**  @param  off - PCI offset of header to write
**  @param  nbytes - Number of bytes to write
**  @param  data - Location of config data to write
**
**  @return FALSE if no data set, TRUE if data set
**
******************************************************************************
**/

INT32 LI_SetConfig(unsigned long id, UINT32 off, INT32 nbytes, UINT8 *data)
{
    struct pci_devs *dev;
    off_t   sresult;
    ssize_t wresult;

    if (id < PCIBASE)
    {
        fprintf(stderr, "LI_SetConfig: id=%ld\n", id);
        return FALSE;
    }
    id -= PCIBASE;
    if (id >= XIO3D_NIO_MAX)
    {
        fprintf(stderr, "LI_SetConfig: id=%ld > XIO3D_NIO_MAX\n", id);
        return FALSE;
    }
    dev = &gPCIdevs[id];
    if (dev->vendor == 0)
    {
        fprintf(stderr, "LI_SetConfig: Unknown device: id=%ld\n", id);
        return FALSE;
    }
    if (dev->busdevfn == ENET_BUSDEVFN)
    {
        fprintf(stderr, "LI_SetConfig: Cannot touch enet device: id=%ld\n",
                id);
        return FALSE;
    }
    sresult = lseek(dev->cfd, (signed)off, SEEK_SET);
    if (sresult == (off_t)-1)
    {
        int save_errno = errno;

        perror("LI_SetConfig: lseek on config fd failed");
        fprintf(stderr, "LI_SetConfig: lseek on config fd failed with %d\n",
                save_errno);
        return FALSE;
    }
    wresult = write(dev->cfd, data, (unsigned)nbytes);
    if (wresult <= 0)
    {
        int save_errno = errno;

        perror("LI_SetConfig: read of config fd failed");
        fprintf(stderr, "LI_SetConfig: read of config fd failed with %d\n",
                save_errno);
        return FALSE;
    }

    return TRUE;
}


#ifndef PAM
/*
******************************************************************************
** LI_AccessDevice
**
**  @brief  Prepares PCI device for access.
**
**  This function prepares to access PCI registers by mmap()ing the PCI
**  registers into user space. One must be running as root to map PCI
**  space with this function.
**
**  @param  id - Device ID (11-31) to set
**  @param  space - Device address space to map
**
**  @return Base address to access registers or zero if failure
**
******************************************************************************
**/

/* Micromemory will use         PGSZ   256    PGSZ   2^24       PGSZ -- with */
/* 256 rounded to PGSZ.  The 3 PGSZ's are for space between the areas. */
#define MAX_MICRO_MEMORY_DEFINE 4096 + 4096 + 4096 + 16777216 + 4096

static unsigned long LI_memory_next = PCI_MMIO_BASE + MAX_MICRO_MEMORY_DEFINE;

static unsigned long LI_micro_memory_next = PCI_MMIO_BASE;
static struct already_allocated {
    unsigned long id;
    unsigned long space;
    void *addr;
    unsigned long lth;
} is_space_address[XIO3D_NIO_MAX];
static int used_already = 0;

#if defined(FRONTEND)
#define WHEREWHAT        "FE_LI_AccessDevice"
#else
#define WHEREWHAT        "BE_LI_AccessDevice"
#endif

unsigned long LI_AccessDevice(unsigned long id, unsigned long space)
{
    int             memFd;         /* /dev/mem file descriptor */
    struct pci_devs *dev;
    void            *maddr;
    void            *daddr;
    unsigned long   remainder,total;
    UINT32          page_size;
    int             i;

    if (id < PCIBASE || space > 6)
    {
        fprintf(stderr, "%s: error: id=%ld, space=%ld\n",
            WHEREWHAT, id, space);
        return 0;
    }
    id -= PCIBASE;
    if (id >= XIO3D_NIO_MAX)
    {
        fprintf(stderr, "%s: id=%ld > XIO3D_NIO_MAX\n", WHEREWHAT, id);
        return FALSE;
    }
    dev = &gPCIdevs[id];
    if (dev->vendor == 0)
    {
        fprintf(stderr, "%s: Unknown device: id=%ld\n", WHEREWHAT, id);
        return 0;
    }
    if (dev->busdevfn == ENET_BUSDEVFN)
    {
        fprintf(stderr, "%s: Cannot touch enet device: id=%ld\n",
            WHEREWHAT, id);
        return 0;
    }
    memFd = open(MEMFILE, O_RDWR | O_SYNC); /* O_SYNC treats as uncacheable */
    if (memFd == -1)
    {
        int save_errno = errno;

        perror("LI_AccessDevice: Open of /dev/mem file failed");
        fprintf(stderr, "%s: Open of %s failed with %d\n",
                WHEREWHAT, MEMFILE, save_errno);
        return 0;
    }
    if (dev->start[space] == 0)
    {
        fprintf(stderr, "%s: Unknown space %ld for device %ld\n",
                WHEREWHAT, space, id);
        close(memFd);
        return 0;
    }
/* Check if already allocated, if so return that address. */
    i = 0;
    while (i < used_already)
    {
        if (is_space_address[i].id == id &&
            is_space_address[i].space == space)
            {
                if (is_space_address[i].lth != dev->len[space]) {
                    fprintf(stderr, "%s: id %lu space %lu old lth=%lu != new=%lu, continue?\n",
                                    WHEREWHAT, id, space, is_space_address[i].lth, dev->len[space]);
                }
                else
                {
                    maddr = is_space_address[i].addr;
                    fprintf(stderr, "%s: Successful previouw return, mmaped address: %lX\n",
                                    WHEREWHAT, (unsigned long)maddr);
                    return((unsigned long)maddr);
                }
          }
          i++;
    }

    page_size = getpagesize();

/* If starts "not on a page boundary", get offset into page. */
    remainder = (dev->start[space]) % page_size;
/* Get total space needed. */
    total = (dev->len[space] + remainder);
    if (total > page_size)
    {
        if ((total % page_size) != 0)
        {
            /* Round up to page size. */
            total += page_size - (total % page_size);
        }
    }
    else if (total < page_size)
    {
        total = page_size;
    }

/* Leave space before this address that is unallocated. */
/* NOTE: THIS PROBABLY SHOULD BE MMAPPED AS UNREADABLE AND UNWRITEABLE. */
    if (id == pci_dev_micro_memory)
    {
        daddr = (void *)(LI_micro_memory_next + page_size);
        if (((unsigned int)daddr+total) > (PCI_MMIO_BASE + MAX_MICRO_MEMORY_DEFINE))
        {
            fprintf(stderr, "%s: Error with micromemory card mmap, Not enough space reserved for %lu, %lu\n",
                            WHEREWHAT, id, space);
            close(memFd);
            return 0;
        }
    }
    else
    {
        daddr = (void *)(LI_memory_next + page_size);
    }

#ifdef MM_DEBUG
    fprintf(stderr, "%s: id = %lu, space= %lu\n", WHEREWHAT, id, space);
    fprintf(stderr, "%s: dev->len[space] = %lX dev->start[space] = %lX\n",
                            WHEREWHAT, dev->len[space], dev->start[space]);
    fprintf(stderr, "%s: dev->len[space] = %lu dev->start[space] = %lu\n",
                            WHEREWHAT, dev->len[space], dev->start[space]);
    fprintf(stderr, "%s: pagesize as returned by getpagesize: %X\n", WHEREWHAT, page_size);
#endif

    maddr = mmap(daddr, total, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED,
            memFd,
            (signed)((remainder)? (dev->start[space] - remainder) : dev->start[space]));
    if (maddr == MAP_FAILED)
    {
        int save_errno = errno;

        perror("LI_AccessDevice: mmap failed");
        fprintf(stderr,
            "%s: mmap of device %ld, space %ld failed with %d\n",
                WHEREWHAT, id, space, save_errno);
        close(memFd);
        return 0;
    }
    else if (maddr != daddr)
    {
        fprintf(stderr, "%s: Error with mmap, got address: %lX (wanted %lX)\n",
                            WHEREWHAT, (unsigned long)maddr, (unsigned long)daddr);
        close(memFd);
        return 0;
    }
    if (remainder != 0)
    {
        maddr = (void *)((u_char *)maddr + remainder);
    }

    /* Save for possible next time with same id and space. */
    is_space_address[used_already].id = id;
    is_space_address[used_already].space = space;
    is_space_address[used_already].addr = maddr;
    is_space_address[used_already].lth = dev->len[space];
    used_already++;

    /* Next time get starting memory here. */
    if (id == pci_dev_micro_memory)
    {
        LI_micro_memory_next += page_size + total;
    }
    else
    {
        LI_memory_next += page_size + total;
    }

#ifdef MM_DEBUG
    fprintf(stderr, "%s: Successful return, mmaped address: %lX (wanted %lX)\n",
            WHEREWHAT, (unsigned long)maddr, (unsigned long)daddr);

#endif
    close(memFd);
    return (unsigned long)maddr;
}


/*
******************************************************************************
** LI_RegisterIRQ
**
**  @brief  Register for PCI interrupt.
**
**  This function registers for a PCI interrupt.
**
**  @param  id - Device ID (0-3) to register for
**  @param  func - Pointer to function to call
**  @param  value - Long value to pass to func
**
**  @return None
**
******************************************************************************
**/

void LI_RegisterIRQ(UINT32 id, void *func, UINT32 value)
{
    struct pci_devs *dev;
    int32_t pcidev;
    int rc;

    if (id >= PCIBASE)
    {
        fprintf(stderr, "LI_RegisterIRQ: id=%d > PCIBASE\n", id);
        return;
    }
    dev = &gPCIdevs[id];
    if (dev->vendor == 0)
    {
        fprintf(stderr, "LI_RegisterIRQ: Unknown device: id=%d\n", id);
        return;
    }

    pcidev = dev->busdevfn;
    fprintf(stderr, "LI_RegisterIRQ: registering for device %04x...", pcidev);
    rc = CT_ioctl(CT_xiofd, XIO3D_PCIINT, &pcidev);
    if (rc == -1)
    {
        fprintf(stderr, "LI_RegisterIRQ: ioctl failed %d for device %04lx\n",
                errno, dev->busdevfn);
        return;
    }
    fprintf(stderr, "got event no. %d\n", pcidev);
    LI_RegisterEvent(pcidev, func, value);
}

/*
******************************************************************************
** LI_IRQCGlue
**
**  @brief  C Glue code for PCI interrupt handler.
**
**  This function serves as a transition to translated assembly code.
**
**  @param  value - Value to pass
**
**  @return None
**
******************************************************************************
**/

void LI_IRQCGlue(UINT32 value)
{
    isp$r23qx((void *)value);
}


/*
******************************************************************************
** LI_GetDMAOffset
**
**  @brief  Return DMA address offset.
**
**  This function returns a value to offset shared memory addresses with
**  to get physical DMA addresses.
**
**  @return Offset for physical DMA addresses.
**
******************************************************************************
**/

unsigned long   LI_GetDMAOffset(void)
{
    struct xio3d_drvinfo xio3d_drvinfo;
    unsigned long   localAddr;
    unsigned long   tmp;
    int i;

    if (CT_ioctl(CT_xiofd, XIO3D_GETINF, &xio3d_drvinfo) == -1)
    {
        perror("ioctl");
        fprintf(stderr, "LI_GetDMAOffset: XIO3D_GETINF ioctl failed\n");
        exit(1);
    }

#if defined(FRONTEND)
    i = XIO_FE, localAddr = FE_BASE_ADDR;
#elif   defined(BACKEND)
    i = XIO_BE, localAddr = BE_BASE_ADDR;
#elif   defined(BUILD_CCB)
    i = XIO_CCB, localAddr = CCB_BASE_ADDR;
#else
#error  "Unknown compilation environment"
#endif
    if (xio3d_drvinfo.mem_regions[i].phys == 0)
    {
        fprintf(stderr, "LI_GetDMAOffset: phys 0\n");
        exit(1);
    }
    tmp = xio3d_drvinfo.mem_regions[i].phys - localAddr;
    fprintf(stderr, "Phys=%08llX, localAddr=%08lX, DMAoffset=%08lX\n",
            xio3d_drvinfo.mem_regions[i].phys, localAddr, tmp);
    return tmp;
}

/*
******************************************************************************
** LI_GetDeviceID
**
**  @brief  Accessor function for getting device ID.
**
**  @param  port - The chip instance
**
**  @return device ID
**
******************************************************************************
**/

unsigned long   LI_GetDeviceID(unsigned int port)
{
    struct pci_devs *dev;

    if (port >= XIO3D_NIO_MAX)
    {
        fprintf(stderr, "LI_GetDeviceID: port %d out of range\n", port);
        return 0;
    }

    dev = &gPCIdevs[port];

    return dev->device;
}
#endif /* PAM */

/*
******************************************************************************
** LI_GetPCIdev
**
**  @brief  Accessor function for getting pci_devs structure.
**
**  @param  port - The chip instance
**
**  @return device ID
**
******************************************************************************
**/

pci_devs    *LI_GetPCIdev(unsigned int port)
{
    if (port >= XIO3D_NIO_MAX)
    {
        fprintf(stderr, "LI_GetPCIdev: port %d out of range\n", port);
        return 0;
    }

    return &gPCIdevs[port];
}

#ifndef PAM
/*
******************************************************************************
** LI_GetPhysicalAddr
**
**  @brief  Get physical addresses for PCI DMA.
**
**  This function gets physical addresses corresponding to shared memory
**  addresses. Addresses are checked for validity.
**
**  Note that the order of the code in this file is dependent on the
**  ordering of the process address spaces. Furthermore, it assumes that
**  the multiple shared memory segments are adjacent. If these conditions
**  change, this function has to be changed as well.
**
**  @param  value - Shared memory address to translate to physical
**
**  @return Physical address, or 0 if checks fail
**
******************************************************************************
**/

unsigned long   LI_GetPhysicalAddr(unsigned long value)
{
    if (value >= FE_BASE_ADDR)
    {
        if (value < CCB_BASE_ADDR)
        {
            return value - FE_BASE_ADDR + getDMAAddr(XIO_FE);
        }
        if (value < BE_BASE_ADDR)
        {
            return value - CCB_BASE_ADDR + getDMAAddr(XIO_CCB);
        }
        if (value < NVRAM_BASE)
        {
            return value - BE_BASE_ADDR + getDMAAddr(XIO_BE);
        }
    }
    fprintf(stderr, "LI_GetPhysicalAddr: Bad address, value=%08lX\n", value);
    abort();        /* Abort the system - it is broken */
} /* LI_GetPhysicalAddr */


/*
******************************************************************************
** LI_GetSharedAddr
**
**  @brief  Get shared addresses for a specified segment from PCI DMA address.
**
**  This function gets shared addresses (in the current segment) corresponding
**  to physical memory addresses.
**
**  @param  UINT32 dmaAddr  - Physical memory address to translate to shared
**
**  @return UINT32 shared address, or 0 if checks fail
**
******************************************************************************
**/

UINT32  LI_GetSharedAddr(UINT32 dmaAddr)
{
#ifdef FRONTEND
    return (dmaAddr - getDMAAddr(XIO_FE) + FE_BASE_ADDR);
#else
    return (dmaAddr - getDMAAddr(XIO_BE) + BE_BASE_ADDR);
#endif
}
#endif /* PAM */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
