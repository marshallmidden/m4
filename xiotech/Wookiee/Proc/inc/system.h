/* $Id: system.h 161041 2013-05-08 15:16:49Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       system.h
**
**  @brief      System constants
**
**  Global constant definitions.
**
**  Copyright (c) 1996-2010 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "globalOptions.h"
#include "XIO_Const.h"                  /* XIOtech Constant definitions     */

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

#define OSPINDOWNPRIO  150
/*
** Kernel/misc layer assigments
*/
#define QUANTUM         125             /* Time quantum in ms - must be     */

#define SYS_TSC_ONE_MSEC_DELAY          ((UINT64)(ct_cpu_speed * 1000ULL))
#define SYS_TSC_ONE_SEC_DELAY           ((UINT64)(ct_cpu_speed * (1000ULL * 1000ULL)))

/*
**  Initial packet allocations
*/
#define INITIAL_ILTS    4095            /* Initial ILT allocation           */

/*
** NVRAM Addresses
*/
#define NVRAM_ONLY_SIZE 0x00200000      /* Size of just the NVRAM           */
#define NVRAM_STARTNMI  0x7800          /* Location of NMI area             */
#define NVRAM_NMISIZ    0x0800          /* Size of NMI area                 */
#define NVRAM_NMI_STATS 0x7F40          /* Location of NMI Stats            */
#define MVRAM_NMI_STATS_ADDR (NVRAM_BASE + NVRAM_NMI_STATS)/* Addr of NMI area*/
#define NVRAM_STARTSN   0x6200          /* Start addr of serial number area */
#define NVRAM_SNSIZ     0x0200          /* Size of serial number area       */
#define NVRAM_OLDCSER   0x7F70          /* Old location of the controller
                                            serial number                   */
#define NVRAM_CSER      0x6210          /* Location of the controller
                                            serial number                   */
#define NVRAM_RES       0x8000          /* Diagnostics area (32k)           */

/* Size of Write Record Area MUST be a multiple of 32*nvasiz (0x200)        */
    #define    NVRAM_WRITE_RECORD_AREA_SIZE SIZE_256K   /* 256kB */

    #define    SIZE_OF_PART           NVRAM_WRITE_RECORD_AREA_SIZE /* size of part-3 or 4 is 256kB here */
/*    #define    LOCAL_NVRAM_SIZE       3*SIZE_OF_PART         BE p-3 & p-4 and FE p-4
    #define    LOCAL_BE_NVRAMSIZ      2*SIZE_OF_PART
    #define    LOCAL_FE_NVRAMSIZ      SIZE_OF_PART*/

    #define    NV_ADMIN_START         0
    #define    NV_ADMIN_SIZE          SIZE_32K
    #define    MICRO_MEM_BE_P3_START  (NV_ADMIN_START+NV_ADMIN_SIZE) /* BE part-3 is the 2nd block in the NV memory */
    #define    MICRO_MEM_BE_P4_START  (MICRO_MEM_BE_P3_START+SIZE_OF_PART) /* BE part-4 is next to BE part-3 */
    #define    MICRO_MEM_FE_P4_START  (MICRO_MEM_BE_P4_START+SIZE_OF_PART) /* FE part-4 is next to BE part-4 */
    #define    MICRO_MEM_FE_P4_END    (MICRO_MEM_FE_P4_START+SIZE_OF_PART)
    #define    MICRO_MEM_BE_P6_START  MICRO_MEM_FE_P4_END

#ifdef  FRONTEND
    #define NVRAM_P3_SIZE   NVRAM_WRITE_RECORD_AREA_SIZE
    #define NVRAM_P4_START_BF 0xFE808000 /* Starting address of P4 NVRAM in Bigfoot,
        this value should be changed whenever NVRAM_BASE and NVRAM_RES are changed for Bigfoot*/
/*  Part 4 NVRAM for RAID 5 Write Records at the FE Mirror Partner          */
    #define NVRAM_P4_START  (NVRAM_BASE + NVRAM_RES)  /* FE808000           */
    #define NVRAM_P4_SIZE   NVRAM_WRITE_RECORD_AREA_SIZE

/*  Part 5 NVRAM for Diagnostic Data Area (1968K)                           */
    #define NVRAM_P5_START  (NVRAM_P4_START + NVRAM_P4_SIZE) /* FE814000    */
    #define NVRAM_P5_SIZE   0x1EC000

#else   /* BACKEND */

/*  Part 2 NVRAM for Configuration records (1472K)                          */
    #define NVRAM_P2_START  (NVRAM_BASE + NVRAM_RES)  /* FE808000           */
    #define NVRAM_P2_SIZE   0x170000
    #define NVRAM_P2_SIZE_SECTORS   (NVRAM_P2_SIZE / BYTES_PER_SECTOR)

/*  Part 3 NVRAM for RAID 5 Write Records in progress at BE                 */
    #define NVRAM_P3_START  (NVRAM_P2_START + NVRAM_P2_SIZE) /* FE978000    */
    #define NVRAM_P3_SIZE   NVRAM_WRITE_RECORD_AREA_SIZE

/*  Part 4 NVRAM for RAID 5 Write Records from FE P4 Area                   */
    #define NVRAM_P4_START  (NVRAM_P3_START + NVRAM_P3_SIZE) /* FE984000    */
    #define NVRAM_P4_SIZE   NVRAM_WRITE_RECORD_AREA_SIZE

/*  Part 5 NVRAM for Diagnostic Data Area  (300K)                           */
    #define NVRAM_P5_START  (NVRAM_P4_START + NVRAM_P4_SIZE) /* FE990000    */
    #define NVRAM_P5_SIZE   0x4B000

/*  Part 6 NVRAM for Copy/Mirror Resync Area (64K)                          */
    #define NVRAM_P6_START  (NVRAM_P5_START + NVRAM_P5_SIZE) /* FE9DB000    */
    #define NVRAM_P6_SIZE   0x10000
    #define NVRAM_P6_SIZE_SECTORS (((NVRAM_P6_SIZE - 1 ) / BYTES_PER_SECTOR) + 1)

#endif /* FRONTEND and else BACKEND */

/*
** Base memory addresses
*/
#include "LKM_layout.h"
#define CCB_COMM_ADDR   (SHARELOC+0x1000)   /* CCB/Proc communication area - 0x1000 */
#define DDR_TABLE_ADDR  (SHARELOC+0x1000)   /* Debug Data Retrieval table - 0x800 */
#ifdef FRONTEND
    /* FE CCB Communications area (outside of the DDR) needs to start at
        0xA0001FFFF and works backwards (DDR will work forward).  Each entry
        must be like a DDR entry (8 byte ID, 4 Byte Address 4 Byte Length)  */
    #define FE_NVRAM_P4_ID      (SHARELOC+0x1FF0)   /* The FE NVRAM Part 4 ID        */
    #define FE_NVRAM_P4_ID_ASCII PROC_ADDRESS_TABLE_ENTRY_ID_FE_NVRAM_P4
    #define FE_NVRAM_P4_ADDR    (SHARELOC+0x1FF8)   /* The FE NVRAM Part 4 Address   */
    #define FE_NVRAM_P4_LENGTH  (SHARELOC+0x1FFC)   /* The FE NVRAM Part 4 Length    */
#endif   /* FRONTEND */

/*
** Misc
*/
#define VERS            1
#define REV             0

/*
** MAX values
*/
#define MAX_VIRTUALS            MAX_VIRTUAL_DISKS
//#if defined(MODEL_3000) || defined(MODEL_7000)
//#define MAX_VIRTUALS            2048
//#else
//#error "Unknown model"
//#endif

#define MAX_LDDS                512
#define MAX_TAG                 31      /* 63 is the absolute max, ISE has trouble over 31. */
#define MIN_TAG                 1
#define MAX_TAG_SATA            8       /* Max tagged commands for SATA Drive */
#define MIN_TAG                 1
/* #define MAX_PORTS               4 */       /* Must be a multiple of 4 for arrays in structures */
#ifdef BACKEND
#define MAX_PORTS               MAX_BE_PORTS /* Must be a multiple of 4 for arrays in structures */
#else   /* BACKEND */
#define MAX_PORTS               MAX_FE_PORTS /* Must be a multiple of 4 for arrays in structures */
#endif  /* BACKEND */
#define MAX_DEV                 2048     /* Maximum number of FC targets     */
                                         /* (LIDs) per channel               */
#define LOOP_MAP_SIZE           128
#ifdef FE_ICL
#define  MAX_ICL_PORTS          1
#else
#define  MAX_ICL_PORTS          0
#endif

/*
** CDriver layer assignments.
*/
#define MAX_CIMT                (MAX_FE_PORTS + MAX_ICL_PORTS)

/*
** IDriver layer assignments.
*/
#define MAX_ICIMT               (MAX_FE_PORTS + MAX_ICL_PORTS)

/*
** Multi-controller assignments
*/
#define MAX_CTRL                16
/* #define TARGETS_PER_CTRL        4 */
#define TARGETS_PER_CTRL        MAX_PORTS

/*
** Task Priorities
*/
#define VCDTHROTTLE_PRIORITY    146
#define REBUILD_PRIORITY        150
#define SCRUB_PRIORITY          160
#define SCAN_PRIORITY            95
#define ISPREGVP_PRIORITY       168     /**< isp_registerVports priority    */
#define ISPPONPRI_PRIORITY       23     /**< isp_portOnlineHandler priority */
#define REBLD_WRITE_PRIORITY    105     /**< Rebuild Write Task priority    */
#define NV_SCRUB_PRIORITY       220     /**< NVRAM Scrub Priority           */
#define NV_DMA_EXEC_PRIORITY     26     /**< NV Memory DMA Executor priority*/
#define NV_DMA_COMP_PRIORITY     25     /**< NV Memory DMA Completor pri    */
#define MM_MONITOR_PRIORITY     130     /**< MicroMemory Monitor priority   */
#define MM_TEST_PRIORITY        250     /**< MicroMemory Test Driver pri    */
#define VUPDFESTATUS_PRIORITY   120     /**< V_updFEStatus priority         */
#define PEXECPRI                 50     /**< Physical exec priority         */

/* The priorities of these tasks are greater than priority of copy/manager
   related tasks.
*/
#define GR_AUTOSWAP_PRIORITY                     151
#define GR_AUTOSWAPBACK_WAITTIMER_PRIORITY       157
#define GR_AUTOSWAPBACK_HYSTERESISTIMER_PRIORITY 158
#define GR_VDISK_ERRORHANDLER_PRIORITY           155

/*
** Physical layer assignments
*/
#define MAX_SIMPLE              16

/*
** ISP (Back end) layer assignments
*/
#define ONLINE_DELAY            (2500 / QUANTUM)
#define OFFLINE_DELAY           (10000/QUANTUM)

#define ISP_PMON_PRI            18      /* F_portMonitor priority           */
#define ISP_DISC_PRI            19      /* F_Discovery priority             */
#define PHY_SETUP4RETRY_PRI     21      /* PHY_StartSetup4Retry priority    */
#define ON_INIT_PRI             150     /* online drive init priority       */
#define F_INIT_DRV_PRI          149     /* P$init_drv priority              */

#define DPOWRITE    TRUE            /* Disable Page Out (DPO) for SCSI writes */
#define DPOVERIFY   TRUE            /* Disable Page Out (DPO) for SCSI verifies */

#if defined(MODEL_7000) || defined(MODEL_4700)
#define BTIMEOUT    25              /* Base I/O timeout in seconds */
#define IORETRY      3              /* task retry counter (3 retries plus original I/O) */
#else  /* MODEL_7000 || MODEL_4700 */
#define BTIMEOUT     5              /* Base I/O timeout in seconds */
#define IORETRY      5              /* task retry counter (5 retries plus original I/O) */
#endif /* MODEL_7000 || MODEL_4700 */

#define SECSIZE     512             /* Disk sector size */
#define DSKBALLOC   1048576         /* Disk allocation unit (bytes) */

/*
 * Snapshot write completion priority.
 */
#define SS_WRITE_COMPLETION_PRIORITY    20

/*
** Global Priority
*/
#define MAX_GLOBAL_PRIORITY     7       /* Maximum global prior             */

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
UINT32 ISE_IsISEUpgrading(void);

#endif /* _SYSTEM_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
