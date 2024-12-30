/* $Id: XIO_Const.h 162888 2014-03-18 15:20:25Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       XIO_Const.h
**
**  @brief      Common constant values
**
**  Common constant values shared between PROC and CCB
**
**  Copyright (c) 2003-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _XIO_CONST_H_
#define _XIO_CONST_H_

#ifdef __cplusplus
#pragma pack(push, 1)
#endif

/*
******************************************************************************
** Public defines - Constants
******************************************************************************
*/

/**
**  @name   System object constants
**  @{
**/
#define MAX_CONTROLLERS             2
#define BYTES_PER_SECTOR            512L
#define MAX_MISC_DEVICES            64
#define MAX_DISK_BAYS               64
#define MAX_ISE                     64
#define MAX_REPORTLUN_RETRIES       10
#define MAX_PAGE85_RETRIES          30

#ifdef PERF
 #define MAX_CONFIGURABLE_NODE_ID   1
#else
 #define MAX_CONFIGURABLE_NODE_ID   (MAX_CONTROLLERS-1)
#endif /* PERF */

#if defined(MODEL_7000) || defined(MODEL_4700)
#define MAX_DISK_BAY_SLOTS          65
#else   /* MODEL_7000 || MODEL_4700 */
#define MAX_DISK_BAY_SLOTS          16
#endif  /* MODEL_7000 || MODEL_4700 */

/**
** EWok need to work with both 750 & 3000 Controller,
** So we define the following additional macro symbols
**/


#define SYM_MAX2(a, b, c) (a ## b > a ## c ? a ## b : a ## c)
#define SYM_MAX(a) SYM_MAX2(a, _3000, _7000)

/* Macro Symbols for 7000 Controller -- Also change system.inc */
#define MAX_PHYSICAL_DISKS_7000     512
#define MAX_VIRTUAL_DISKS_7000      4000
#define MAX_RAIDS_7000              8000
#define MAX_SERVERS_7000            1024
#define MAX_SERVERS_UNIQUE_7000     256
#define MAX_PDISKS_PER_RAID_7000    MAX_PHYSICAL_DISKS_7000
#define MAX_BAYS_PORT_7000          32
#if defined(MODEL_7000) || defined(MODEL_4700)
// NOTE: MAX FE LUNS change needed below (add one for lun 0).
#define MAX_ISE_LUNS                65
#else   /* MODEL_7000 || MODEL_4700 */
// NOTE: MAX FE LUNS change needed below.
#define MAX_ISE_LUNS                16
#endif  /* MODEL_7000 || MODEL_4700 */

/* Macro Symbols for 3000 Controller -- Also change system.inc */
#define MAX_PHYSICAL_DISKS_3000     512
#define MAX_VIRTUAL_DISKS_3000      4000
#define MAX_RAIDS_3000              8000
#define MAX_SERVERS_3000            1024
#define MAX_SERVERS_UNIQUE_3000     256
#define MAX_PDISKS_PER_RAID_3000    MAX_PHYSICAL_DISKS_3000
#define MAX_BAYS_PORT_3000          8

/* Macro symbols for 750 Controller -- these remain for apps compiles*/
#define MAX_PHYSICAL_DISKS_750      192
#define MAX_VIRTUAL_DISKS_750       96
#define MAX_RAIDS_750               192
#define MAX_SERVERS_750             192
#define MAX_SERVERS_UNIQUE_750      96
#define MAX_PDISKS_PER_RAID_750     MAX_PHYSICAL_DISKS_750
#define MAX_BAYS_PORT_750           8

#if defined(MODEL_7000)
#define MAX_PHYSICAL_DISKS         MAX_PHYSICAL_DISKS_7000
#define MAX_VIRTUAL_DISKS          MAX_VIRTUAL_DISKS_7000
#define MAX_RAIDS                  MAX_RAIDS_7000
#define MAX_SERVERS                MAX_SERVERS_7000
#define MAX_SERVERS_UNIQUE         MAX_SERVERS_UNIQUE_7000
#define MAX_PDISKS_PER_RAID        MAX_PDISKS_PER_RAID_7000
#define MAX_BAYS_PORT              MAX_BAYS_PORT_7000
#elif defined(MODEL_3000)
#define MAX_PHYSICAL_DISKS         MAX_PHYSICAL_DISKS_3000
#define MAX_VIRTUAL_DISKS          MAX_VIRTUAL_DISKS_3000
#define MAX_RAIDS                  MAX_RAIDS_3000
#define MAX_SERVERS                MAX_SERVERS_3000
#define MAX_SERVERS_UNIQUE         MAX_SERVERS_UNIQUE_3000
#define MAX_PDISKS_PER_RAID        MAX_PDISKS_PER_RAID_3000
#define MAX_BAYS_PORT              MAX_BAYS_PORT_3000
#elif defined(MODEL_4700)
#define MAX_PHYSICAL_DISKS         MAX_PHYSICAL_DISKS_7000      // 4000 looking like 7000
#define MAX_VIRTUAL_DISKS          MAX_VIRTUAL_DISKS_7000       // 4000 looking like 7000
#define MAX_RAIDS                  MAX_RAIDS_7000               // 4000 looking like 7000
#define MAX_SERVERS                MAX_SERVERS_7000             // 4000 looking like 7000
#define MAX_SERVERS_UNIQUE         MAX_SERVERS_UNIQUE_7000      // 4000 looking like 7000
#define MAX_PDISKS_PER_RAID        MAX_PDISKS_PER_RAID_7000     // 4000 looking like 7000
#define MAX_BAYS_PORT              MAX_BAYS_PORT_7000           // 4000 looking like 7000
#elif defined(MODEL_7400)
#define MAX_PHYSICAL_DISKS         MAX_PHYSICAL_DISKS_3000      // 7000 looking like 4000
#define MAX_VIRTUAL_DISKS          MAX_VIRTUAL_DISKS_3000       // 7000 looking like 4000
#define MAX_RAIDS                  MAX_RAIDS_3000               // 7000 looking like 4000
#define MAX_SERVERS                MAX_SERVERS_3000             // 7000 looking like 4000
#define MAX_SERVERS_UNIQUE         MAX_SERVERS_UNIQUE_3000      // 7000 looking like 4000
#define MAX_PDISKS_PER_RAID        MAX_PDISKS_PER_RAID_3000     // 7000 looking like 4000
#define MAX_BAYS_PORT              MAX_BAYS_PORT_3000           // 7000 looking like 4000
#else
#ifdef XIO_XWS  /* This header is also used by Icon Services (aka EWOK) */
                /* Values must be the max possible */
#define MAX_PHYSICAL_DISKS         SYM_MAX(MAX_PHYSICAL_DISKS)
#define MAX_VIRTUAL_DISKS          SYM_MAX(MAX_VIRTUAL_DISKS)
#define MAX_RAIDS                  SYM_MAX(MAX_RAIDS)
#define MAX_SERVERS                SYM_MAX(MAX_SERVERS)
#define MAX_SERVERS_UNIQUE         SYM_MAX(MAX_SERVERS_UNIQUE)
#define MAX_PDISKS_PER_RAID        SYM_MAX(MAX_PDISKS_PER_RAID)
#define MAX_BAYS_PORT              SYM_MAX(MAX_BAYS_PORT)
#else
#error "MODEL unknown"
#endif /* XIO_XWS*/
#endif /* MODEL */

#define MAX_SEGMENTS                16          /* The raids per vdisk (add one to get 17!) */
// NOTE: MAX FE LUNS change needed below.
#define MAX_VLINKS                  64          /* The virtual link devices */
#define MAX_PHYSICAL_DISKS_X1       1024
#define AVG_RAIDS_PER_VDISK         2
// NOTE: MAX FE LUNS change needed below.
#define MAX_TARGETS                 64
// NOTE: MAX FE LUNS change needed below.
#define MAX_LUNS                    64          /* Must be an even number for struct IMT */

#ifdef PORT_NUMBER
#define MAX_FE_PORTS                PORT_NUMBER
#define MAX_BE_PORTS                PORT_NUMBER
#else  /* PORT_NUMBER */
/* Following two defined in Proc/inc/system.inc and Proc/inc/chn.inc. */
#define MAX_FE_PORTS                4
#define MAX_BE_PORTS                4
#endif /* PORT_NUMBER */

#define MAX_HABS                    (MAX_FE_PORTS + MAX_BE_PORTS)
#define MAX_LOCATION_BAYS           64
#define MAX_OPDDLIST_COUNT          (MAX_LOCATION_BAYS + MAX_PHYSICAL_DISKS + MAX_MISC_DEVICES)
#define MAX_LOCATION_DISKS_PER_BAY  32
#define MAX_FW_HDR_TYPES            3
#define VLINK_NAME_LEN              8
#define MAX_WORKSETS                16
#define MAX_VBLOCKS                 16
#define MAX_LDDS                    512

#if defined(MODEL_7000) || defined(MODEL_4700)
#define MAX_CORS                    512         /* Maximum # CORs supported             */
#else  /* MODEL_7000 || MODEL_4700 */
#define MAX_CORS                    256         /* Maximum # CORs supported             */
#endif /* MODEL_7000 || MODEL_4700 */
/* @} */

/**
**  @name   X1 interface related constants
**  @{
**/
#define X1_NAME_LEN                 16      /**< Max name length in bytes   */
#define X1_ID_FOR_VCG               0xFFFF  /**< This ID is used for a VCG  */
/* @} */

/**
**  @name Disk Allocation Units (bytes and sectors)
**  @{
**/
#define DISK_BYTE_ALLOC             1048576
#define DISK_SECTOR_ALLOC           (DISK_BYTE_ALLOC / BYTES_PER_SECTOR)
/* @} */

/**
**  @name File System Reserved Area (bytes)
**  @{
**/
#define RESERVED_AREA_SIZE          (128 * DISK_SECTOR_ALLOC)
/* @} */

#define FALSE                       0
#define TRUE                        1

#define FAIL                        1   /* failure = non zero value         */
#define PASS                        0   /* test passed = zero               */

/**
**  @name NULL Definitions (Windows and Bigfoot)
**  @{
**/
#ifdef _WIN32
    /*
    ** If the standard MFC header has not been included, NULL is not
    ** defined so define it here.
    */
    #ifndef __AFX_H__
        #define NULL                ((void *)0)
    #endif
#else
    #ifndef NULL
        #define NULL                    ((void *)0)
    #endif       /* NULL */
#endif
/* @} */

/**
**  @name Good and Error Definitions (Windows and Bigfoot)
**  @{
**/
#ifdef _WIN32
    #if (defined(GOOD) && !defined(ERROR))
        #if (GOOD == 1)
            #define ERROR           0
        #else
            #define ERROR           1
        #endif
    #endif

    #if (!defined(GOOD) && defined(ERROR))
        #if (ERROR == 1)
            #define GOOD            0
        #else
            #define GOOD            1
        #endif
    #endif

    #if (!defined(GOOD) && !defined(ERROR))
        #define GOOD                0
        #define ERROR               1
    #endif
#else
    #define GOOD                    0
    #define ERROR                   1
#endif
/* @} */

#define FAILURE                     (FALSE)
#define SUCCESS                     (TRUE)

#define FOREVER                     (TRUE)

#define BIT31                       0x80000000

#define STOP_ON_ERROR               0 /* stop on error mode value */
#define LOOP_ON_ERROR               1 /* loop on error mode value */
#define CONT_ON_ERROR               2 /* continue on error mode value */
#define RESTART_ON_ERROR            3 /* start testing over on 1st fail */
#define SKIP_ON_ERROR               4 /* skip any test that detects a fail */

#define UINT64_ALL_1S               0xFFFFFFFFFFFFFFFF  /* skip any test that detects a fail */
#define UINT64_ALL_0S               0x0000000000000000  /* skip any test that detects a fail */

/**
**  @name Memory Sizes
**  @{
**/
#define SIZE_1                      0x00000001
#define SIZE_2                      0x00000002
#define SIZE_4                      0x00000004
#define SIZE_8                      0x00000008
#define SIZE_16                     0x00000010
#define SIZE_32                     0x00000020
#define SIZE_64                     0x00000040
#define SIZE_128                    0x00000080
#define SIZE_256                    0x00000100
#define SIZE_512                    0x00000200
#define SIZE_1K                     0x00000400
#define SIZE_2K                     0x00000800
#define SIZE_4K                     0x00001000
#define SIZE_6K                     0x00001800
#define SIZE_8K                     0x00002000
#define SIZE_16K                    0x00004000
#define SIZE_32K                    0x00008000
#define SIZE_64K                    0x00010000
#define SIZE_128K                   0x00020000
#define SIZE_256K                   0x00040000
#define SIZE_512K                   0x00080000
#define SIZE_1MEG                   0x00100000
#define SIZE_2MEG                   0x00200000
#define SIZE_4MEG                   0x00400000
/* #define SIZE_5MEG                   0x00500000 -- Unused, would be for TXBUFFER in PortServer.h */
#define SIZE_8MEG                   0x00800000
#define SIZE_16MEG                  0x01000000
#define SIZE_32MEG                  0x02000000
#define SIZE_64MEG                  0x04000000
#define SIZE_128MEG                 0x08000000
#define SIZE_256MEG                 0x10000000
#define SIZE_512MEG                 0x20000000
#define SIZE_1GIG                   0x40000000
#define SIZE_2GIG                   0x80000000
/* @} */

/**
**  @name Memory Sizes
**  @{
**/
#define WC_SIZE_256MEG              0x07400000
#if defined(MODEL_7000) || defined(MODEL_4700)
#define WC_SIZE_512MEG              WC_SIZE_256MEG
#define WC_SIZE_1GIG                WC_SIZE_256MEG
#define WC_SIZE_2GIG                WC_SIZE_256MEG
#else  /* MODEL_7000 || MODEL_4700 */
#define WC_SIZE_512MEG              0x09000000
#define WC_SIZE_1GIG                WC_SIZE_512MEG
#define WC_SIZE_2GIG                WC_SIZE_512MEG
#endif /* MODEL_7000 || MODEL_4700 */
/* @} */

/**
**  @name CCB Communications Area Constants
**  @{
**/
#define PROC_ADDRESS_TABLE_ADDR             CCB_PCI_START+0x1000

#define PROC_ADDRESS_TABLE_MAX_DDR_ENTRIES  127 /* Max number of DDR entries */
#define PROC_ADDRESS_TABLE_MAX_CCB_COM_ENTRIES 128 /* Max CCB Comm entries  */
#define PROC_ADDRESS_TABLE_MAX_ENTRIES \
  (PROC_ADDRESS_TABLE_MAX_DDR_ENTRIES + PROC_ADDRESS_TABLE_MAX_CCB_COM_ENTRIES)
                                        /* Number of entries beyond DDR Header*/
#define PROC_ADDRESS_TABLE_ENTRY_ID_II          "procK_ii"
#define PROC_ADDRESS_TABLE_ENTRY_ID_FE_NVRAM_P4 "FE_NV_P4"
/* @} */

#include "L_Signal.h"
#ifndef LOG_SIMULATOR
    #include <unistd.h>
#endif
/*
** Exit Error Codes
*/
#define errExit(a)              L_SignalProcess(getppid(), XIO_PLATFORM_SIGNAL, a); \
                                L_SignalProcess(getpid(), XIO_PLATFORM_SIGNAL, a); \
                                usleep(1000000); abort();

/* Error Codes  (Linux Realtime Signals) */
    /*
    ** NOTE: If you add a new signal, you must also update file
    **       Shared/Src/L_Misc.c routine LL_LinuxSignalToString.
    **       This will help us to determine what signals we receive
    **       from the serial buffer, and the log messages.
    **
    ** NOTE: Make sure to bump ERR_EXIT_MAX if you add a new signal.
    **
    ** NOTE: Form man 7 signal:
    **          LinuxThreads implementation uses the first three
    **          real-time signals. Start at +5
    */
/* GLOBAL RT SIGNAL */
#define XIO_PLATFORM_SIGNAL     __SIGRTMIN + 0x05

/* CCB HB */
#define CCB_2_PAM_HB            0x00000001

#define ERR_EXIT_SHUTDOWN       0x00000010
// #define ERR_EXIT_BE_MISS_HB     0x00000011       UNUSED
// #define ERR_EXIT_FE_MISS_HB     0x00000012       UNUSED
#define ERR_EXIT_RESET_CCB      0x00000013
#define ERR_EXIT_RESET_ALL      0x00000014
#define ERR_EXIT_DEADLOOP       0x00000015
#define ERR_EXIT_REBOOT         0x00000016
#define ERR_EXIT_FIRMWARE       0x00000017
#define ERR_EXIT_BVM_RESTART    0x00000018
#define ERR_EXIT_BIOS_REBOOT    0x00000019
#define ERR_PAM_DIRECTED_DOWN   0x00000020

/*
** Prioritys to set.
*/
#if 0
#if defined(PAM)
    #define XIO_LINUX_PRIORITY  ((sched_get_priority_max (SCHED_FIFO) * 9) / 10)
#elif defined(BACKEND) || defined(FRONTEND)
    #define XIO_LINUX_PRIORITY  ((sched_get_priority_max (SCHED_FIFO) * 8) / 10)
#elif defined (CCB_RUNTIME_CODE)
    #define XIO_LINUX_PRIORITY  ((sched_get_priority_max (SCHED_FIFO) * 8) / 10 + 5)
#else
#endif
#else
    #define XIO_LINUX_PRIORITY  0
#endif

#if 0
#define XIO_LL_LINUX_PRIORITY  ((sched_get_priority_max (SCHED_FIFO) * 8) / 10)
#else
#define XIO_LL_LINUX_PRIORITY  0
#endif

/* Flag to indicate whql is disabled */
#define DISABLE 0xff

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _XIO_CONST_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
