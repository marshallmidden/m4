/* $Id: L_XIO3D.c 143007 2010-06-22 14:48:58Z m4 $ */

/**
******************************************************************************
**  @file       L_XIO3D.c
**
**  @brief      Get DMA shareable memory set up from kernel module.
**
**  Provide a way for the FE, BE, and CCB to get the same DMA-able (i.e. the
**  memory is physically contiguous) memory for the three processes, each of
**  the processes has a kernel specified amount memory for itself, but the
**  others can get at it -- to simulate the PCI mapping of the i960's.  The
**  memory is at a fixed location for each process.
**
**  Copyright (c) 2004-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sched.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "li_evt.h"
#include "LKM_layout.h"
#include "LL_LinuxLinkLayer.h"
#include "xio3d.h"
#include "XIO_Const.h"
#ifdef CCB_RUNTIME_CODE
#include "errorCodes.h"
#include "xk_kernel.h"
#include "error_handler.h"
#endif /* CCB_RUNTIME_CODE */

#ifdef BACKEND
#ifndef PAM
#include "nvram.h"
#endif
#endif

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/

/* Following will set the first few words of shared memory for each process */
/* and to each mmap-ped memory section. */
/* #define      DEBUG_PRINT_SHARED */
/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/

#define BIT(x)  (1 << (UINT32)(x))

/*
******************************************************************************
** Private variables
******************************************************************************
*/

#if !defined(PAM) && !defined(NOPRIORITY)
struct sched_param schedparam;
#endif /* !PAM && !NOPRIORITY */
#if !defined(PAM) && !defined(NOAFFINITY)
extern int  sched_setaffinity(pid_t pid, unsigned int len, unsigned long *mask);
#endif /* !PAM && !NOAFFINITY */

#ifdef PROC_CODE
const char *tokenPtr = "cpu ";

int procStatFd = -1;

UINT32 lastIdleCount, lastTotalCount, prevIdleCount, prevTotalCount;

#endif /* PROC_CODE */
#if defined(PROC_CODE) && ! defined(PAM)
extern void error31(void);
#endif /* PROC_CODE && !PAM */

#ifndef PAM
#ifdef FRONTEND
#define MSGPRE  "FE:  "
static UINT32 connection_complete_mask = BIT(FE2BE) | BIT(BE2FE) | BIT(FE2CCB) | BIT(CCB2FE);
#elif defined(BACKEND)
#define MSGPRE "BE:  "
static UINT32 connection_complete_mask = BIT(FE2BE) | BIT(BE2FE) | BIT(BE2CCB) | BIT(CCB2BE);
#else
#define MSGPRE "CCB: "
static UINT32 connection_complete_mask = BIT(FE2CCB) | BIT(CCB2FE) | BIT(BE2CCB) | BIT(CCB2BE);
#endif /* FRONTEND */

#else /* PAM */

#define MSGPRE  "PAM: "
#endif /* PAM */

const char *L_MsgPrefix = MSGPRE;

#if defined(PROC_CODE) && !defined(PAM)
static char statBuffer[2048];
#endif /* PROC_CODE && !PAM */

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/

struct xio3d_drvinfo *ptr_xio3d_drvinfo;

struct app_shm be_shm =
    { "/huge/be", "BE", PROT_READ | PROT_WRITE, 0, -1 };
struct app_shm fe_shm =
    { "/huge/fe", "FE", PROT_READ | PROT_WRITE, 1, -1 };
struct app_shm ccb_shm =
    { "/huge/ccb", "CCB", PROT_READ | PROT_WRITE, 2, -1 };
struct app_shm xio3d_shm =
    { "/dev/xio3d0", "INFO", PROT_READ, -1, -1 };

static const struct {
    const char  * const env;
    struct app_shm  * const shm;
} shms[] =
{
    { "CCBSHM", &ccb_shm },
    { "BESHM", &be_shm },
    { "FESHM", &fe_shm },
    { 0, 0 }
};

#ifdef PROC_CODE
unsigned long   gCpuAffinity = -1;  /* Cpu affinity for process */
#endif /* PROC_CODE */

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/

#ifdef CCB_RUNTIME_CODE
/* These are assembler versions of routines for non-use of shared libraries */
  #define L_special_mmap   mmap
  #define L_special_munmap munmap
#define lXio3dExitError() DeadLoop(deadErrCode(ERR_EXIT_SHARED_MEM), TRUE)
#else   /* CCB_RUNTIME_CODE */
#define L_special_mmap   mmap
#define L_special_munmap munmap
#define lXio3dExitError() exit(1)
#endif  /* CCB_RUNTIME_CODE */

#if !defined(PAM) && (defined(FRONTEND) || defined(BACKEND))
static void get_shared_memory(const char *, int, void *);
#endif  /* !defined(PAM) && (defined(FRONTEND) || defined(BACKEND)) */
int change_my_priority(LL_POSSIBLE_LINKS, UINT32);
void SETUP_linux(void);
#if !defined(CCB_RUNTIME_CODE) && !defined(PAM)
UINT8 L_CalcIdle(void);
#endif  /* !defined(CCB_RUNTIME_CODE) && !defined(PAM) */

/*
******************************************************************************
** Public variables - available via other means (linker script, special extern).
******************************************************************************
*/
#if defined(FRONTEND) || defined(BACKEND)
extern unsigned int PSTARTOFHEAP;
#endif  /* defined(FRONTEND) || defined(BACKEND) */

extern UINT32 SHMEM_START, SHMEM_END;

int CT_xiofd = -1;

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**  shm_config
**  @brief      Configure shared memory parameters.
**
**              Modify shared memory files and regions based on environment.
**
**  @return     none
**
******************************************************************************
**/

static void shm_config(void)
{
    int     i;
    int     fd;
    char    *vp;

    for (i = 0; shms[i].shm; ++i)
    {
        char    *nvp;
        unsigned long   ul;

        if (!shms[i].env)
        {
            continue;
        }
        vp = getenv(shms[i].env);
        if (!vp)
        {
            continue;
        }
        nvp = 0;
        ul = strtoul(vp, &nvp, 0);
        if ((ul == 0 && nvp == vp) || ul >= XIO3D_NSHM_MAX || nvp == 0 ||
            *nvp == 0 || nvp[1] == 0)
        {
            fprintf(stderr, MSGPRE "Environment setting for %s invalid (%s)\n",
                shms[i].env, vp);
            lXio3dExitError();
        }
        shms[i].shm->rgn = ul;
        shms[i].shm->file = &nvp[1];
        fprintf(stderr, MSGPRE
            "%s shared memory changed to region %ld file %s\n",
            shms[i].shm->name, ul, &nvp[1]);
    }
    for (i = 0; shms[i].shm; ++i)
    {
        fprintf(stderr, MSGPRE "%s shared memory is region %d file %s\n",
            shms[i].shm->name, shms[i].shm->rgn, shms[i].shm->file);
        fd = open(shms[i].shm->file, O_RDWR);
        if (fd < 0)
        {
            perror("open");
            fprintf(stderr, MSGPRE "Open of %s failed, returning %d\n",
                shms[i].shm->file, fd);
            lXio3dExitError();
        }
        shms[i].shm->fd = fd;   /* Save huge file descriptor */
    }
    vp = getenv("XIO3D");
    if (vp)
    {
        xio3d_shm.file = vp;
    }
    fd = open(xio3d_shm.file, O_RDWR);
    if (fd < 0)
    {
        perror("open");
        fprintf(stderr, MSGPRE "Open of %s failed, returning %d\n",
            xio3d_shm.file, fd);
        lXio3dExitError();
    }
    xio3d_shm.fd = fd;
}


/**
******************************************************************************
**  map_shm
**  @brief      Map shared memory.
**
**              Modify shared memory files and regions based on environment.
**
**  @param      shmp - Pointer to struct app_shm for shared memory to map.
**  @param      mr - Pointer to struct xio3d_mem_region info.
**  @param      vaddr - Virtual address to map the region.
**  @param      len - Length of region to map.
**
**  @return     none
**
******************************************************************************
**/

static void map_shm(struct app_shm *shmp, struct xio3d_mem_region *mr,
        void *vaddr, unsigned long len)
{
    int     rc;
    void    *mem;
    char    name[60];
    uint64_t    size;
    uint64_t    offset;
    struct app_shm  shm = *shmp;

    /* Copy these items onto the stack to allow bizarre CCB case to work. */
    strncpy(name, shm.name, sizeof(name) - 1);
    name[sizeof(name) - 1] = 0;
    size = mr->size;
    offset = mr->offset;

    /* Unmap the addresses where the shared memory will reside. */

    rc = L_special_munmap(vaddr, len);
    if (rc)
    {
        perror("munmap");
        fprintf(stderr, MSGPRE "%s munmap failed, returned value %d\n",
            name, rc);
    }

    /* Map the shared memory. */

    mem = L_special_mmap(vaddr, len < size ? len : size, shm.prot,
        MAP_SHARED | MAP_LOCKED | MAP_FIXED, shm.fd, offset);
    if (mem == MAP_FAILED)
    {
        perror("L_special_mmap failed");
        fprintf(stderr, MSGPRE "%s L_special_mmap failed for address %p\n",
            name, vaddr);
        lXio3dExitError();
    }
    if (len > size)
    {
        fprintf(stderr, MSGPRE
            "%s requested size 0x%lx > provided size 0x%llx\n",
            name, len, size);
        lXio3dExitError();
    }
#ifndef CCB_RUNTIME_CODE
    fprintf(stderr, MSGPRE
        "Mapping %s to %p for %08lx with prot %04x, offset %08llx\n",
        name, vaddr, len, shm.prot, offset);
#endif /* CCB_RUNTIME_CODE */
}


/**
******************************************************************************
**
**  @brief      Get shared memory through a filename.
**
**              Open/Create file, and mmap it to the desired location.
**
**  @param      filename - The file name to open.
**  @param      length   - The length of the file.
**  @param      addr     - Address in memory desired.
**
**  @return     none
**
******************************************************************************
**/

#if !defined(PAM) && (defined(FRONTEND) || defined(BACKEND))
static void get_shared_memory(const char *filename, int length, void *addr)
{
    void *start;
    int fd = -1;
    int i;
    struct stat st;
    int loop;

    length = (length + 4096 - 1) & ~(4096 - 1);  /* Page size it for i386 */
    fd = open(filename, O_RDWR | O_CREAT,
        S_IREAD | S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
    if (fd < 0)
    {
        perror("open get_shared_memory");
        lXio3dExitError();
    }
    loop = 0;
  stat_loop:
    loop++;
    if (fstat(fd, &st) < 0)
    {
        perror("fstat");
        lXio3dExitError();
    }
    if (!S_ISREG(st.st_mode))
    {
        fprintf(stderr, MSGPRE "fstat file (%s) type not regular file\n",
            filename);
        lXio3dExitError();
    }
    if (st.st_size == 0)
    {                /* must create the file */
        /* Assume page size of 4096 */
        char buf[4096];

        memset (buf, '\0', sizeof(buf));
        if (lseek(fd, length - 1, SEEK_SET) < 0)
        {
            perror("lseek");
            lXio3dExitError();
        }
        if (lseek(fd, 0, SEEK_SET) < 0)
        {
            perror("lseek 0");
            lXio3dExitError();
        }
        for (i = 0; i < length; i += 4096)
        {
            if (write(fd, buf, 4096) < 0)
            {
                perror("write");
                lXio3dExitError();
            }
        }
    }
    else if (st.st_size != length)
    {
        if (loop < 200)
        {
            sleep(1);
            goto stat_loop;
        }
        fprintf(stderr, MSGPRE "%s, wanted size %d, unexpected size (%ld)\n",
            filename, length, (long)st.st_size);
        lXio3dExitError();
    }
    start = mmap(addr, (unsigned)length, PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_FIXED, fd, 0);
    if (start == NULL)
    {
        perror("mmap null");
        fprintf(stderr, MSGPRE "mmap returned null for address %p file %s\n",
            addr, filename);
        lXio3dExitError();
    }
    if (start == MAP_FAILED)
    {
        perror("mmap failed");
        fprintf(stderr, MSGPRE "mmap failed for address %p file %s\n",
            addr, filename);
        lXio3dExitError();
    }
    if (start != addr)
    {
        fprintf(stderr, MSGPRE "mmap returned %p, expected %p for file %s\n",
            start, addr, filename);
        lXio3dExitError();
    }
}   /* end of get_shared_memory */
#endif  /* !defined(PAM) && (defined(FRONTEND) || defined(BACKEND)) */

/**
******************************************************************************
**
**  @brief      Change the priority of the calling task.
**
**              If another connection has been established, see if all of
**              the connections are now established. If so, raise the
**              priority of this task (actually the scheduling algorithm)
**              to be real-time (SCHED_FIFO).
**
**              If a connection has been broken, return the priority of
**              the task (its scheduling algorithm) to be the standard
**              Linux time-sharing (SCHED_OTHER).
**
**  @param      myLink - ID of link that has been established
**  @param      connection - 0 if connection broken, 1 of connection made
**
**  @return     0 - No priority change
**              1 - Priority has been raised.
**
******************************************************************************
**/

int change_my_priority(LL_POSSIBLE_LINKS myLink, UINT32 connection)
{
#ifndef PAM

    static UINT32 connection_mask = 0;

    schedparam.__sched_priority = XIO_LL_LINUX_PRIORITY;

    if (connection)
    {
        /*
        ** Connection has been established.
        */
        /*
        ** Don't change the priority until all of the connections have been
        ** established.
        */
        connection_mask |= BIT(myLink);

        if (connection_mask != connection_complete_mask)
        {
            return 0;
        }

#ifdef PROC_CODE
#if XIO_LL_LINUX_PRIORITY != 0
        if (XIO_LL_LINUX_PRIORITY > 0)
        {
            fprintf(stderr, MSGPRE
                "Setting task priority to %d within policy SCHED_FIFO\n",
                    schedparam.__sched_priority);
            if (sched_setscheduler(0, SCHED_FIFO, &schedparam) != 0)
            {
                fprintf(stderr, MSGPRE "FAILED to set task priority\n");
            }
        }
#endif  /* XIO_LL_LINUX_PRIORITY != 0 */
#endif  /* PROC_CODE */
    }
    else
    {
        /* Connection has been lost */
        connection_mask &= ~BIT(myLink);

#if XIO_LL_LINUX_PRIORITY != 0
        if (XIO_LL_LINUX_PRIORITY > 0)
        {
            fprintf(stderr, MSGPRE
                "Setting task priority to %d within policy SCHED_RR\n",
                    schedparam.__sched_priority);
            if (sched_setscheduler(0, SCHED_RR, &schedparam) != 0)
            {
                fprintf(stderr, MSGPRE "FAILED to set task priority\n");
            }
        }
#endif  /* XIO_LL_LINUX_PRIORITY != 0 */
    }
#endif /* !PAM */

    return 1;
}

/**
******************************************************************************
**
**  @brief      Get the shared memory from the kernel module.
**
**              Open the kernel device, get the memory map, mmap() it, and
**              make sure it is where we want it to be.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/

void SETUP_linux()
{
    struct rlimit resource_limit;
    struct xio3d_drvinfo xio3d_inf;
#if defined(PROC_CODE)
    UINT32 shmem_start = (UINT32) &SHMEM_START;
    UINT32 shmem_end = (UINT32) &SHMEM_END;
#endif  /* PROC_CODE */

#ifdef PROC_CODE
    /*
    ** Make sure we have the proper permissions to continue.
    ** We must have root permissions at startup.  We will
    ** dummy down to our wookiee user.
    */
    if ((geteuid() != WOOKIEE_ADMIN_UID) ||
        (getegid() != WOOKIEE_ADMIN_GID))
    {
        fprintf(stderr, MSGPRE
            "SETUP_linux FATAL ERROR - You must be root to start process\n");
        lXio3dExitError();
    }

#if !defined(PAM) && !defined(NOAFFINITY)

    /*
    ** Set up the CPU affinity. The front end and its associated interrupts
    ** are set to use cpu 0 and the back end and its associated interrupts
    ** are set to use cpu 1. If there are 4 cpus, we use the same values
    ** because cpus 0 and 2 are the same physical cpu, as are 1 and 3.
    */

#ifdef  FRONTEND
    gCpuAffinity = 0x1;
    tokenPtr = "cpu0";
#endif  /* FRONTEND */
#ifdef  BACKEND
    gCpuAffinity = 0x2;
    tokenPtr = "cpu1";
#endif  /* BACKEND */
    fprintf(stderr, MSGPRE "Setting cpu affinity to %04lx\n", gCpuAffinity);
    sched_setaffinity(0, sizeof(gCpuAffinity), &gCpuAffinity);

#endif /* !PAM && !NOAFFINITY */

#endif /* PROC_CODE */

    /*
    ** Set the limit value for the amount of locked memory (shared memory)
    ** and stack size for the process.
    */

    fprintf(stderr, MSGPRE
        "Setting the new process limits for locked pages and stack size\n");
    resource_limit.rlim_cur = RLIM_INFINITY;
    resource_limit.rlim_max = RLIM_INFINITY;
    if (setrlimit(RLIMIT_MEMLOCK, &resource_limit) != 0)
    {
        perror("setlimit RLIMIT_MEMLOCK");
        fprintf(stderr, MSGPRE
            "setrlimit failed for RLIMIT_MEMLOCK, error %d\n", errno);
        exit(1);
    }
    resource_limit.rlim_cur = 512 * 1024;
    resource_limit.rlim_max = 512 * 1024;
    if (setrlimit(RLIMIT_STACK, &resource_limit) != 0)
    {
        perror("setlimit RLIMIT_STACK");
        fprintf(stderr, MSGPRE
            "setrlimit failed for RLIMIT_STACK, error %d\n", errno);
        exit(1);
    }

/* ------------------------------------------------------------------------ */

    shm_config();   /* Configure shared memory regions */
    CT_xiofd = xio3d_shm.fd;

    fprintf(stderr, MSGPRE "opening kernel memory sections\n");

/* ------------------------------------------------------------------------ */
    if (ioctl(CT_xiofd, XIO3D_GETINF, &xio3d_inf) < 0)
    {
        perror("ioctl");
        fprintf(stderr, MSGPRE "ioctl of %s with %d failed\n",
            xio3d_shm.name, XIO3D_GETINF);
        lXio3dExitError();
    }
/* ------------------------------------------------------------------------ */

    map_shm(&xio3d_shm, &xio3d_inf.info_region, (void *)INFOREGION_BASE_ADDR,
        xio3d_inf.info_region.size);

    /* Set the global. */

    ptr_xio3d_drvinfo = (void *)INFOREGION_BASE_ADDR;

    map_shm(&ccb_shm, &xio3d_inf.mem_regions[ccb_shm.rgn],
            (void *)CCB_BASE_ADDR, SIZE_CCB_LTH);

#ifdef CCB_RUNTIME_CODE

    /*
    ** Save the global values, and initialize the "BSS" section of the shared memory.
    */

    startOfMySharedMem = (UINT32)CCB_BASE_ADDR;
    endOfMySharedMem = startOfMySharedMem + SIZE_CCB_LTH;
#endif /* CCB_RUNTIME_CODE */
    fprintf(stderr, MSGPRE
            "CCB memory from %p thru %p (length=0x%x [%d])\n",
            (void *)CCB_BASE_ADDR, (void *)((unsigned long)CCB_BASE_ADDR + SIZE_CCB_LTH),
            SIZE_CCB_LTH, SIZE_CCB_LTH);
    if (xio3d_inf.mem_regions[XIO_CCB].size != SIZE_CCB_LTH)
    {
        fprintf(stderr, MSGPRE "unused CCB memory of length=0x%llx [%lld]\n",
            xio3d_inf.mem_regions[XIO_CCB].size - SIZE_CCB_LTH,
            xio3d_inf.mem_regions[XIO_CCB].size - SIZE_CCB_LTH);
    }


    map_shm(&be_shm, &xio3d_inf.mem_regions[be_shm.rgn],
        (void *)BE_BASE_ADDR, SIZE_BE_LTH);

    /*
    ** Save the global values, and initialize the "BSS" section of the shared memory.
    */

    startOfBESharedMem = (UINT32)BE_BASE_ADDR;
    endOfBESharedMem = startOfBESharedMem + SIZE_BE_LTH;
#ifdef BACKEND
    startOfMySharedMem = startOfBESharedMem;
    endOfMySharedMem = endOfBESharedMem;
    pStartOfHeap = (UINT8 *)&PSTARTOFHEAP;
    memset ((void *)shmem_start, 0, shmem_end - shmem_start);
#endif /* BACKEND */
    fprintf(stderr, MSGPRE "BE memory from %p thru %p (length=0x%x [%d])\n",
        (void *)BE_BASE_ADDR,
        (void *)((unsigned int)BE_BASE_ADDR + SIZE_BE_LTH),
        SIZE_BE_LTH, SIZE_BE_LTH);
    if (xio3d_inf.mem_regions[XIO_BE].size != SIZE_BE_LTH)
    {
        fprintf(stderr, MSGPRE "unused BE memory of length=0x%llx [%lld]\n",
            xio3d_inf.mem_regions[XIO_BE].size - SIZE_BE_LTH,
            xio3d_inf.mem_regions[XIO_BE].size - SIZE_BE_LTH);
    }


    map_shm(&fe_shm, &xio3d_inf.mem_regions[fe_shm.rgn],
        (void *)FE_BASE_ADDR, SIZE_FE_LTH);

    /*
    ** Save the global values, and initialize the "BSS" section
    ** of the shared memory.
    */

#ifdef FRONTEND
    startOfMySharedMem = (UINT32)FE_BASE_ADDR;
    endOfMySharedMem = startOfMySharedMem + SIZE_FE_LTH;
    memset((void *)shmem_start, 0, shmem_end - shmem_start);
    pStartOfHeap = (UINT8 *)&PSTARTOFHEAP;
#endif /* FRONTEND */
    fprintf(stderr, MSGPRE "FE memory from %p thru %p (length=0x%x [%d])\n",
        (void *)FE_BASE_ADDR, (void *)(FE_BASE_ADDR + SIZE_FE_LTH),
        SIZE_FE_LTH, SIZE_FE_LTH);
    if (xio3d_inf.mem_regions[XIO_FE].size != SIZE_FE_LTH)
    {
        fprintf(stderr, MSGPRE "unused FE memory of length=0x%llx [%lld]\n",
            xio3d_inf.mem_regions[XIO_FE].size - SIZE_FE_LTH,
            xio3d_inf.mem_regions[XIO_FE].size - SIZE_FE_LTH);
    }


/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  */
#ifdef FRONTEND
#ifdef DEBUG_PRINT_SHARED
    *(int *)(FE_BASE_ADDR+4) = 0xfe2fe;
    *(int *)(BE_BASE_ADDR+4) = 0xfe2be;
    *(int *)(CCB_BASE_ADDR+4) = 0xfe2ccb;
#endif  /* DEBUG_PRINT_SHARED */

    /* Register for the FE event */

#ifndef PAM
    LI_RegisterEvent(XIO3D_FE_EVT, 0, 0);

    /* Get the FE NVRAM */

    get_shared_memory("/opt/xiotech/procdata/shared_memory_NVSRAM_FE",
        NVRAM_BASESIZE, (void *)NVRAM_BASE);
#endif  /* PAM */
#endif  /* FRONTEND */

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  */
#ifdef BACKEND
#ifdef DEBUG_PRINT_SHARED
    *(int *)(FE_BASE_ADDR+8) = 0xbe2fe;
    *(int *)(BE_BASE_ADDR+8) = 0xbe2be;
    *(int *)(CCB_BASE_ADDR+8) = 0xbe2ccb;
#endif  /* DEBUG_PRINT_SHARED */

    /* Register for the BE event */

#ifndef PAM
    LI_RegisterEvent(XIO3D_BE_EVT, 0, 0);

    /* Get the BE NVRAM */

    get_shared_memory("/opt/xiotech/procdata/shared_memory_NVSRAM_BE",
        NVRAM_BASESIZE, (void *)NVRAM_BASE);
    is_nvram_p2_initialized();
#endif  /* PAM */
#endif  /* BACKEND */
/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  */

#ifdef CCB_RUNTIME_CODE
#ifdef DEBUG_PRINT_SHARED
    *(int *)(FE_BASE_ADDR+12) = 0xccb2fe;
    *(int *)(BE_BASE_ADDR+12) = 0xccb2be;
    *(int *)(CCB_BASE_ADDR+12) = 0xccb2ccb;
#endif  /* DEBUG_PRINT_SHARED */

    /* Register for the CCB event */

#ifndef PAM
    LI_RegisterEvent(XIO3D_CCB_EVT, 0, 0);
#endif  /* PAM */
#endif  /* CCB_RUNTIME_CODE */
/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  */

    fprintf(stderr, MSGPRE "shared memory sections opened\n");
}   /* end of SETUP_linux */


#if defined(PROC_CODE) && !defined(PAM)

/**
******************************************************************************
**
**  @brief      Calculate the CPU idle time.
**
**              This routine reads the Linux file /proc/stat, which contains
**              information about the number of "jiffies" that the system has
**              spent idle, in user mode, in user niced mode, and in kernel
**              mode. From this information, calculate the idle percentage
**              of this CPU.
**
**  @param      none
**
**  @return     The CPU idle percentage, as an integer in the range 9 - 100,
**              returned as an 8 bit value.
**
******************************************************************************
**/

UINT8 L_CalcIdle(void)
{
    int         length;
    UINT32      idleCount = 0, totalCount = 0, idleSum, totalSum;
    char        *bufferPtr;
    UINT8       idlePercent = 0;

    /*
    ** Open the file if it hasn't already been opened. Otherwise, reposition
    ** in the file to read from the beginning.
    */

    if (procStatFd < 0)
    {
        procStatFd = open("/proc/stat", O_RDONLY);
    }
    else
    {
        if ((length = lseek(procStatFd, 0, SEEK_SET)) != 0)
        {
            close(procStatFd);
            procStatFd = -1;
            return 0;
        }
    }

    /*
    ** Open the /proc/stats file for reading.
    */

    if (procStatFd >= 0)
    {
        /* Read in the information and close the file. */

        length = read(procStatFd, statBuffer, sizeof(statBuffer) - 1);
        if (length < 0)
        {
            close(procStatFd);
            procStatFd = -1;
            return 0;
        }

        /* Make sure the buffer is zero terminated */

        statBuffer[length] = 0;

        /* Find the token in the buffer.*/

        bufferPtr = strstr(statBuffer, tokenPtr);

        /* If the token was found, skip past it. */

        if (bufferPtr != NULL)
        {
            bufferPtr += strlen(tokenPtr);

            /*
            ** The next four fields represent, respectively, the number of
            ** jiffies spent in user mode, user niced mode, kernel mode,
            ** and idle.
            */

            totalCount = strtoul(bufferPtr, &bufferPtr, 10);
            totalCount += strtoul(bufferPtr, &bufferPtr, 10);
            totalCount += strtoul(bufferPtr, &bufferPtr, 10);
            idleCount = strtoul(bufferPtr, &bufferPtr, 10);
            totalCount += idleCount;

            /*
            ** Calculate the idle percentage the same way as Bigfoot. That is,
            ** add the idle counts for the current and last iterations, divide
            ** that by the sum of the total counts for the current and last
            ** iterations, and turn that into a non-rounded unsigned integral
            ** value. Since the value is 0 - 100, save it in an unsigned char.
            */

            idleSum = idleCount - prevIdleCount;
            totalSum = totalCount - prevTotalCount;
            if (totalSum == 0) idlePercent = 0;
            else idlePercent = (UINT8)(idleSum * 100 / totalSum);
        }
    }

    /* Save the values for next time. */

    prevIdleCount = lastIdleCount;
    prevTotalCount = lastTotalCount;
    lastIdleCount = idleCount;
    lastTotalCount = totalCount;

    return idlePercent;
}

#endif /* PROC_CODE && !PAM */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
