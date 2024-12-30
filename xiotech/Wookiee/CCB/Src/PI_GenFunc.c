/* $Id: PI_GenFunc.c 143020 2010-06-22 18:35:56Z m4 $*/
/**
******************************************************************************
**
**  @file   PI_GenFunc.c
**
**  @brief  Packet Interface for the user Generic Function
**
**  Handler function for the "generic" function.
**  This feature allows a developer to write his own function
**  and call it through the packet interface.
**
** Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "PI_CmdHandlers.h"

#include "AsyncClient.h"
#include "ccb_flash.h"
#include "debug_files.h"
#include "ipc_sendpacket.h"
#include "mach.h"
#include "nvram.h"
#include "nvram_structure.h"
#include "quorum.h"
#include "X1_Structs.h"
#include "X1_Workset.h"
#include "XIO_Std.h"
#include "PortServerUtils.h"
#include "xk_kernel.h"

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void PingTask(TASK_PARMS *parms);

/*****************************************************************************
** Function prototypes defined in other files, but not header files.
*****************************************************************************/
extern void DisplayAddress(const char *name, const UINT32 addr);
extern INT32 NetworkSetup(void);
extern void MM_PrintStats(void);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    PI_GenericFunc
**
** Description: User modifiable function that can be called through the
**              packet interface.
**
** Inputs:      p0-p7 - User input parms. All Unsigned ints.
**
** Returns:     User specified;
**
**--------------------------------------------------------------------------*/

#define PrintSizeof(S) dprintf(DPRINTF_DEFAULT, "sizeof(" #S ") = 0x%X/%u bytes\n", sizeof(S), sizeof(S))

/*
** NOTE:  This is all strictly test / debug stuff.
** Feel free to add case statements if you wish.
*/
INT32 PI_GenericFunc(UINT32 p0, UINT32 p1, UINT32 p2, UINT32 p3,
                     UINT32 p4, UINT32 p5, UINT32 p6, UINT32 p7)
{
    INT32       rc = 0;
    TASK_PARMS  parms;

    switch (p0)
    {

            /*
             * Print out the size of various NVRAM structures
             */
        case 0:
            dprintf(DPRINTF_DEFAULT,
                    "\nGENFUNC menu options:\n"
                    " 0) Print this menu\n"
                    " 1 n) Force a CCB memory free error trap:\n"
                    " 1 1) double free\n"
                    " 1 2) stomp fence\n"
                    " 1 3) stomp hdr\n"
                    " 2) <sock#> Close socket\n"
                    " 3) Print sizeof and location of NVRAM structures\n"
                    " 4 0) Copy Serial & Trace data to NVRAM\n"
                    " 4 1) Copy CCB NVRAM to FLASH\n"
                    " 5) Clear CCB NVRAM - the whole thing!\n"
                    " 6) Reload Controller Configuration from NVRAM\n"
                    " 7) Unused\n"
                    " 8) Display cache structure sizes\n"
                    " 9) Display MAX stack depth (\"9 1\" resets the total)\n"
                    " 10) Initialize default VPort in all Worksets\n"
                    " Flash Routines:\n"
                    " 11 0 Address Dataword) Write Flash Word\n"
                    " 12 0) Get Time\n"
                    " 12 1 mm dd yyyy hh mm ss) Set Time\n"
                    " 13) Gather heap stats in 'heapStats' (DOES NOT DO ANYTHING)\n"
                    " 14) IPC Ping (SN,Path,[times to fork ping task])\n"
                    " 15) Display Heap Control Block\n"
                    " 16) Call network accessor functions\n"
                    " 17) Set the IP/SN/GW addresses in NVRAM\n"
                    " 18) Test fork/exec 0==system(), 1==XK_System()\n"
                    " 19) Enable SSH and create ccb.cfg (overwrite if exists, SUICIDE DISABLED)\n"
                    " 20) Enable SSH and create ccb.cfg (overwrite if exists, SUICIDE ENABLED)\n"
                    " 21) Lock up the CCB with a nice while(1) loop\n"
                    " 22) Sleep in this request for <# milliseconds>\n");
            break;

        case 1:
            switch (p1)
            {
                case 1:
                    /*
                     * Force a CCB memory free error trap (double free)
                     */
                    {
                        UINT8      *q;
                        UINT8      *p = MallocWC(100);

                        q = p;
                        Free(p);
                        Free(q);
                    }
                    break;

                case 2:
                    /*
                     * Force a CCB memory free error trap (stomp fence)
                     */
                    {
                        UINT8      *p = MallocWC(10);

                        memset(p, 0, 20);
                        Free(p);
                    }
                    break;

                case 3:
                    /*
                     * Force a CCB memory free error trap (stomp header)
                     */
                    {
                        UINT8      *a;
                        UINT8      *b;
                        UINT8      *c;
                        UINT8      *p;

                        a = MallocWC(1000);
                        b = MallocWC(1000);
                        c = MallocWC(1000);

                        Free(a);
                        Free(b);

                        p = MallocWC(1000);

                        memset(p - 16, '#', 16);
                        Free(p);
                    }
                    break;

                default:
                    dprintf(DPRINTF_DEFAULT, "Must enter 1 for \"double free\" 2 for \"stomp fence\" 3 for \"stomp header\", ...\n");
                    break;
            }
            break;

            /*
             * Close specified socket
             */
        case 2:
            dprintf(DPRINTF_DEFAULT, "Closing socket %d\n", p1);
            rc = Close(p1);
            break;

            /*
             * Print out the size of various NVRAM structures
             */
        case 3:
            dprintf(DPRINTF_DEFAULT, "=== NVRAM Base @ %08X ===\n", (UINT32)&NVRAMData);
            PrintSizeof(NVRAM_STRUCTURE);

            dprintf(DPRINTF_DEFAULT, "=== masterConfigRecord @ %08X ===\n",
                    (UINT32)&NVRAMData.masterConfigRecord);
            PrintSizeof(QM_MASTER_CONFIG);

            dprintf(DPRINTF_DEFAULT, "=== miscData @ %08X ===\n",
                    (UINT32)&NVRAMData.miscData);
            PrintSizeof(MISC_NON_CRC_DATA);

            dprintf(DPRINTF_DEFAULT, "=== cntlSetup @ %08X ===\n",
                    (UINT32)&NVRAMData.cntlSetup);
            PrintSizeof(CONTROLLER_SETUP);

            dprintf(DPRINTF_DEFAULT, "=== asyncQueue @ %08X ===\n",
                    (UINT32)&NVRAMData.asyncQueue);
            PrintSizeof(ASYNC_EVENT_QUEUE);

            dprintf(DPRINTF_DEFAULT, "=== reservedSpace1 @ %08X ===\n",
                    (UINT32)&NVRAMData.reservedSpace1);
            PrintSizeof(NVRAMData.reservedSpace1);

            dprintf(DPRINTF_DEFAULT, "=== disasterData @ %08X ===\n",
                    (UINT32)&NVRAMData.disasterData);
            PrintSizeof(NVRAMData.disasterData);

            dprintf(DPRINTF_DEFAULT, "=== ipmiNVRAMData @ %08X ===\n",
                    (UINT32)&NVRAMData.ipmiNVRAMData);
            PrintSizeof(NVRAMData.ipmiNVRAMData);

            dprintf(DPRINTF_DEFAULT, "=== reservedSpace2 @ %08X ===\n",
                    (UINT32)&NVRAMData.reservedSpace2);
            PrintSizeof(NVRAMData.reservedSpace2);

            dprintf(DPRINTF_DEFAULT, "=== errortrapDataRun @ %08X ===\n",
                    (UINT32)&NVRAMData.errortrapDataRun);
            PrintSizeof(ERRORTRAP_DATA_RUN);

            dprintf(DPRINTF_DEFAULT, "=== errortrapDataRun.errorCounters @ %08X ===\n",
                    (UINT32)&NVRAMData.errortrapDataRun.errorCounters);
            PrintSizeof(ERRORTRAP_DATA_ERROR_COUNTERS);

            dprintf(DPRINTF_DEFAULT, "=== errortrapDataRun.errorSnapshot @ %08X ===\n",
                    (UINT32)&NVRAMData.errortrapDataRun.errorSnapshot);
            PrintSizeof(ERRORTRAP_DATA_ERROR_SNAPSHOT);

            dprintf(DPRINTF_DEFAULT, "=== errortrapDataRun.errorSnapshot.timestamp @ %08X ===\n",
                    (UINT32)&NVRAMData.errortrapDataRun.errorSnapshot.timestamp);
            PrintSizeof(NVRAMData.errortrapDataRun.errorSnapshot.timestamp);

            dprintf(DPRINTF_DEFAULT, "=== errortrapDataRun.errorSnapshot.firmwareRevisionData @ %08X ===\n",
                    (UINT32)&NVRAMData.errortrapDataRun.errorSnapshot.firmwareRevisionData);
            PrintSizeof(NVRAMData.errortrapDataRun.errorSnapshot.firmwareRevisionData);

            dprintf(DPRINTF_DEFAULT, "=== errortrapDataRun.errorSnapshot.cpuRegisters @ %08X ===\n",
                    (UINT32)&NVRAMData.errortrapDataRun.errorSnapshot.cpuRegisters);
            PrintSizeof(ERRORTRAP_DATA_CPU_REGISTERS);

            dprintf(DPRINTF_DEFAULT, "=== errortrapDataRun.errorSnapshot.machRegisters @ %08X ===\n",
                    (UINT32)&NVRAMData.errortrapDataRun.errorSnapshot.machRegisters);
            PrintSizeof(NVRAMData.errortrapDataRun.errorSnapshot.machRegisters);

            dprintf(DPRINTF_DEFAULT, "=== errortrapDataRun.errorSnapshot.callStack @ %08X ===\n",
                    (UINT32)NVRAMData.errortrapDataRun.errorSnapshot.callStack);
            PrintSizeof(NVRAMData.errortrapDataRun.errorSnapshot.callStack);

            dprintf(DPRINTF_DEFAULT, "=== errortrapDataRun.traceEvData @ %08X ===\n",
                    (UINT32)NVRAMData.errortrapDataRun.traceEvData);
            PrintSizeof(NVRAMData.errortrapDataRun.traceEvData);

            dprintf(DPRINTF_DEFAULT, "=== errortrapDataRun.serialPortData @ %08X ===\n",
                    (UINT32)NVRAMData.errortrapDataRun.serialPortData);
            PrintSizeof(NVRAMData.errortrapDataRun.serialPortData);

            dprintf(DPRINTF_DEFAULT, "=== errortrapDataRun.ccbStatistics @ %08X ===\n",
                    (UINT32)&NVRAMData.errortrapDataRun.ccbStatistics);
            PrintSizeof(NVRAMData.errortrapDataRun.ccbStatistics);

            dprintf(DPRINTF_DEFAULT, "=== errortrapDataRun.heapStatsNV @ %08X ===\n",
                    (UINT32)&NVRAMData.errortrapDataRun.heapStatsNV);
            PrintSizeof(NVRAMData.errortrapDataRun.heapStatsNV);

            dprintf(DPRINTF_DEFAULT, "=== errortrapDataBoot @ %08X ===\n",
                    (UINT32)&NVRAMData.errortrapDataBoot);
            PrintSizeof(ERRORTRAP_DATA_BOOT);
            break;

            /*
             * Copy Trace & Serial port data to NVRAM
             */
        case 4:
            switch (p1)
            {
                case 0:
                    CopyBacktraceDataToNVRAM();
                    break;
                case 1:
                    CopyNVRAMBacktraceDataToFlash();
                    break;
                default:
                    dprintf(DPRINTF_DEFAULT, "Huh?\n");
            }
            break;

            /*
             * Clear the entire CCB NVRAM
             */
        case 5:
            MemSetBytes((void *)0xFEEB0000, 0, (SIZE_128K - 8));
            break;

            /*
             * Reload Controller Configuration
             */
        case 6:
            LoadControllerSetup();
            break;

            /*
             * Unused
             */
        case 7:
            dprintf(DPRINTF_DEFAULT, "Genfunc 7 unused\n");
            break;

            /*
             * Print various cacheing items
             */
        case 8:
            dprintf(DPRINTF_DEFAULT, "CACHE_SIZE_DISK_BAYS       = %u / 0x%X bytes\n",
                    CACHE_SIZE_DISK_BAYS, CACHE_SIZE_DISK_BAYS);
            dprintf(DPRINTF_DEFAULT, "CACHE_SIZE_PHYSICAL_DISKS  = %u / 0x%X bytes\n",
                    CACHE_SIZE_PHYSICAL_DISKS, CACHE_SIZE_PHYSICAL_DISKS);
            dprintf(DPRINTF_DEFAULT, "CACHE_SIZE_PDISK_PATHS     = %u / 0x%X bytes\n",
                    CACHE_SIZE_PDISK_PATHS, CACHE_SIZE_PDISK_PATHS);
            dprintf(DPRINTF_DEFAULT, "CACHE_SIZE_DISK_BAY_PATHS  = %u / 0x%X bytes\n",
                    CACHE_SIZE_DISK_BAY_PATHS, CACHE_SIZE_DISK_BAY_PATHS);
            dprintf(DPRINTF_DEFAULT, "CACHE_SIZE_VIRTUAL_DISKS   = %u / 0x%X bytes\n",
                    CACHE_SIZE_VIRTUAL_DISKS, CACHE_SIZE_VIRTUAL_DISKS);
            dprintf(DPRINTF_DEFAULT, "CACHE_SIZE_RAIDS           = %u / 0x%X bytes\n",
                    CACHE_SIZE_RAIDS, CACHE_SIZE_RAIDS);
            dprintf(DPRINTF_DEFAULT, "CACHE_SIZE_SERVERS         = %u / 0x%X bytes\n",
                    CACHE_SIZE_SERVERS, CACHE_SIZE_SERVERS);
            dprintf(DPRINTF_DEFAULT, "CACHE_SIZE_TARGETS         = %u / 0x%X bytes\n",
                    CACHE_SIZE_TARGETS, CACHE_SIZE_TARGETS);
            dprintf(DPRINTF_DEFAULT, "CACHE_SIZE_DISK_BAY_MAP    = %u / 0x%X bytes\n",
                    CACHE_SIZE_DISK_BAY_MAP, CACHE_SIZE_DISK_BAY_MAP);
            dprintf(DPRINTF_DEFAULT, "CACHE_SIZE_VDISK_MAP       = %u / 0x%X bytes\n",
                    CACHE_SIZE_VDISK_MAP, CACHE_SIZE_VDISK_MAP);
            dprintf(DPRINTF_DEFAULT, "CACHE_SIZE_RAID_MAP        = %u / 0x%X bytes\n",
                    CACHE_SIZE_RAID_MAP, CACHE_SIZE_RAID_MAP);
            dprintf(DPRINTF_DEFAULT, "CACHE_SIZE_SERVER_MAP      = %u / 0x%X bytes\n",
                    CACHE_SIZE_SERVER_MAP, CACHE_SIZE_SERVER_MAP);
            dprintf(DPRINTF_DEFAULT, "CACHE_SIZE_TARGET_MAP      = %u / 0x%X bytes\n",
                    CACHE_SIZE_TARGET_MAP, CACHE_SIZE_TARGET_MAP);
            dprintf(DPRINTF_DEFAULT, "CACHE_SIZE_HAB_MAP         = %u / 0x%X bytes\n",
                    CACHE_SIZE_HAB_MAP, CACHE_SIZE_HAB_MAP);
            dprintf(DPRINTF_DEFAULT, "CACHE_SIZE_FE_LOOP_STATS   = %u / 0x%X bytes\n",
                    CACHE_SIZE_FE_LOOP_STATS, CACHE_SIZE_FE_LOOP_STATS);
            dprintf(DPRINTF_DEFAULT, "CACHE_SIZE_BE_LOOP_STATS   = %u / 0x%X bytes\n",
                    CACHE_SIZE_BE_LOOP_STATS, CACHE_SIZE_BE_LOOP_STATS);
            dprintf(DPRINTF_DEFAULT, "CACHE_SIZE_BE_PORT_INFO    = %u / 0x%X bytes\n",
                    CACHE_SIZE_BE_PORT_INFO, CACHE_SIZE_BE_PORT_INFO);
            break;

            /*
             * Display cache structure sizes
             */
        case 9:
            {
                UINT32      tmp = NVRAMData.miscData.stkDepthCnt;

                tmp *= 128;     /* Convert free count to bytes */
                tmp = TOTAL_STACK_SIZE - tmp;   /* subtract from total */
                tmp = (tmp + 1023) / 1024;      /* round up to nearest K */

                dprintf(DPRINTF_DEFAULT, "Max Stack Depth = %dK (found at 0x%08X)\n",
                        tmp, (UINT32)&NVRAMData.miscData.stkDepthCnt);

                if (p1)
                {
                    NVRAMData.miscData.stkDepthCnt = 0xFFFF;
                    dprintf(DPRINTF_DEFAULT, "Max Stack Depth RESET\n");
                }
                break;
            }

            /*
             * Initialize default VPort in all worksets.
             */
        case 10:
            {
                rc = InitWorksetDefaultVPort();
                break;
            }

            /*
             * Flash routines
             */
        case 11:
            switch (p1)
            {
            case 0:
                rc = CCBFlashProgramData((CCB_FLASH *)p2, (CCB_FLASH *)&p3, 1);
                break;

            default:
                dprintf(DPRINTF_DEFAULT, "Huh?\n");
                break;
            }
            break;

            /*
             * Set Clock
             */
        case 12:
            {
                TIMESTAMP   ts;

                switch (p1)
                {
                    case 1:
                        ts.month = Binary2BCD(p2);
                        ts.date = Binary2BCD(p3);
                        ts.year = Binary2BCD(p4);
                        ts.day = 0;
                        ts.hours = Binary2BCD(p5);
                        ts.minutes = Binary2BCD(p6);
                        ts.seconds = Binary2BCD(p7);
                        ts.systemSeconds = 0;

                        RTC_SetTime(&ts);

                        /* fall through */

                    case 0:
                        RTC_GetTimeStamp(&ts);

                        dprintf(DPRINTF_DEFAULT, "Date/Time: %02X/%02X/%04X %02X:%02X:%02X\n",
                                ts.month, ts.date, ts.year,
                                ts.hours, ts.minutes, ts.seconds);
                        break;

                    default:
                        dprintf(DPRINTF_DEFAULT, "Huh?\n");
                        break;
                }
                break;
            }

            /* Gather heap stats in 'heapStats'. (DOES NOT DO ANYTHING -- left-over stuff.) */
        case 13:
            break;

        case 14:
            {
                if (p3)
                {
                    UINT32      i;

                    for (i = 0; i < p3; ++i)
                    {
                        parms.p1 = (UINT32)p1;
                        parms.p2 = (UINT32)p2;
                        TaskCreate(PingTask, &parms);
                    }
                    rc = PI_GOOD;
                }
                else
                {
                    rc = IpcSendPing(p1, p2);
                }
                break;
            }

            /*
             * Gather heap stats in 'heapStats'.
             */
        case 15:
            {
                MM_PrintStats();
                break;
            }

            /*
             * Call network accessor functions
             */
        case 16:
            {
                UINT32      addr;
                char        buf[20];
                char        intf[20] = { 0 };
                void       *pWas = NULL;
                ETHERNET_MAC_ADDRESS mac;

                if (p1)
                {
                    sprintf(intf, "eth%u", p1);
                    pWas = ethernetDriver.interfaceHandle;
                    ethernetDriver.interfaceHandle = (void *)intf;
                }

                dprintf(DPRINTF_DEFAULT, "Interface = %s\n",
                        (char *)ethernetDriver.interfaceHandle);

                addr = GetIpAddressFromInterface(ethernetDriver.interfaceHandle);
                InetToAscii(addr, buf);
                dprintf(DPRINTF_DEFAULT, "IpAddr = %s\n", buf);

                addr = GetSubnetFromInterface(ethernetDriver.interfaceHandle);
                InetToAscii(addr, buf);
                dprintf(DPRINTF_DEFAULT, "Subnet = %s\n", buf);

                addr = GetGatewayFromInterface(ethernetDriver.interfaceHandle);
                InetToAscii(addr, buf);
                dprintf(DPRINTF_DEFAULT, "Gateway = %s\n", buf);

                addr = GetLinkStatusFromInterface(ethernetDriver.interfaceHandle);
                dprintf(DPRINTF_DEFAULT, "Link Stat = 0x%04X\n", addr);

                mac = GetMacAddrFromInterface(ethernetDriver.interfaceHandle);
                dprintf(DPRINTF_DEFAULT, "Mac = %02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX\n",
                        mac.macByte[0], mac.macByte[1], mac.macByte[2], mac.macByte[3],
                        mac.macByte[4], mac.macByte[5]);

                if (intf[0])
                {
                    ethernetDriver.interfaceHandle = pWas;
                    dprintf(DPRINTF_DEFAULT, "Interface set back to = %s\n",
                            (char *)ethernetDriver.interfaceHandle);
                }

                NetworkSetup();
                break;
            }

            /*
             * Set the IP/SN/GW addresses in NVRAM bypassing all other
             * checks and stuff with the normal method of changing the IP.
             */
        case 17:
            {
                /*
                 * 0x5F64400A = 10.64.100.95
                 * 0x00F0FFFF = 255.255.240.0
                 * 0x0160400A = 10.64.96.1
                 */
                DisplayAddress("IP ", p1);
                SetIPAddress(p1);

                DisplayAddress("SN ", p2);
                SetSubnetMask(p2);

                DisplayAddress("GW ", p3);
                SetGatewayAddress(p3);

                break;
            }

            /*
             * Test fork/exec
             */
        case 18:
            {
                switch (p1)
                {
                    case 0:
                        dprintf(DPRINTF_DEFAULT, "system:");
                        rc = system("/bin/ls -lt");
                        break;

                    case 1:
                        dprintf(DPRINTF_DEFAULT, "XK_System:");
                        rc = XK_System("/bin/ls -l -t");
                        break;

                    default:
                        dprintf(DPRINTF_DEFAULT, "huh?");
                        break;
                }
                break;
            }

            /*
             * Enable SSH and create ccb.cfg (overwrite if exists)
             */
        case 19:
            {
                XK_System("/sbin/chkconfig -s sshd 35");
                XK_System("/etc/init.d/sshd start");

                XK_System("echo -e "
                          "\"######################################################################\n"
                          "#\n"
                          "# This file takes the place of the \"dip switches\" on the Bigfoot CCB.\n"
                          "# if this file does not exist in /opt/xiotech/ccbdata, (it won't on\n"
                          "# customer systems), all options default to OFF / FALSE (the same\n"
                          "# as if the option is commentoud out).\n"
                          "#\n"
                          "# To enable an option, uncomment it; to disable it, comment it out.\n"
                          "#\n"
                          "######################################################################\n"
                          "\n"
                          "# Comment out to ENABLE controller suicide (default).\n"
                          "# Uncomment to DISABLE controller suicide.\n"
                          "SUICIDE_DISABLE\n"
                          "\n"
                          "# Comment out to DISABLE diag ports 3100 & 3200 (default).\n"
                          "# Uncomment to ENABLE diag ports 3100 & 3200.\n"
                          "DIAG_PORTS_ENABLE\n"
                          "\n"
                          "# Comment out to ENABLE PAM heartbeat actions (default).\n"
                          "# Uncomment to DISABLE PAM heartbeat actions for debug.\n"
                          "#PAM_HEARTBEAT_DISABLE\n\"" " > /opt/xiotech/ccbdata/ccb.cfg");
                break;
            }


        case 20:
            {
                XK_System("/sbin/chkconfig -s sshd 35");
                XK_System("/etc/init.d/sshd start");

                XK_System("echo -e "
                          "\"######################################################################\n"
                          "#\n"
                          "# This file takes the place of the \"dip switches\" on the Bigfoot CCB.\n"
                          "# if this file does not exist in /opt/xiotech/ccbdata, (it won't on\n"
                          "# customer systems), all options default to OFF / FALSE (the same\n"
                          "# as if the option is commentoud out).\n"
                          "#\n"
                          "# To enable an option, uncomment it; to disable it, comment it out.\n"
                          "#\n"
                          "######################################################################\n"
                          "\n"
                          "# Comment out to ENABLE controller suicide (default).\n"
                          "# Uncomment to DISABLE controller suicide.\n"
                          "#SUICIDE_DISABLE\n"
                          "\n"
                          "# Comment out to DISABLE diag ports 3100 & 3200 (default).\n"
                          "# Uncomment to ENABLE diag ports 3100 & 3200.\n"
                          "DIAG_PORTS_ENABLE\n"
                          "\n"
                          "# Comment out to ENABLE PAM heartbeat actions (default).\n"
                          "# Uncomment to DISABLE PAM heartbeat actions for debug.\n"
                          "#PAM_HEARTBEAT_DISABLE\n\"" " > /opt/xiotech/ccbdata/ccb.cfg");
                break;
            }

            /*
             * Lock her up :-)
             */
        case 21:
            while (1)
            {
                sleep(1);
            }

        case 22:
            TaskSleepMS(p1);
            break;
    }
    return rc;
}

static void PingTask(TASK_PARMS *parms)
{
    UINT32      sn = parms->p1;
    UINT32      type = parms->p2;

    IpcSendPing(sn, type);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
