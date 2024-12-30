/* $Id: main.c 145021 2010-08-03 14:16:38Z m4 $ */
/*============================================================================
** FILE NAME:       main.c
** MODULE TITLE:    Runtime code startup routine
**
** Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "main.h"

#include "AsyncClient.h"
#include "AsyncQueue.h"
#include "CacheManager.h"
#include "ccb_flash.h"
#include "ccb_hw.h"
#include "cps_init.h"
#include "ddr.h"
#include "debug_files.h"
#include "EL_Disaster.h"
#include "error_handler.h"
#include "FCM.h"
#include "fibreshim.h"
#include "FIO.h"
#include "fm.h"
#include "hw_mon.h"
#include "X1_AsyncEventHandler.h"
#include "HWM.h"
#include "i82559.h"
#include "idr_structure.h"
#include "ipc_heartbeat.h"
#include "ipc_sendpacket.h"
#include "kernel.h"
#include "LargeArrays.h"
#include "led.h"
#include "LL_LinuxLinkLayer.h"
#include "LL_Stats.h"
#include "PI_ClientPersistent.h"
#include "mach.h"
#include "misc.h"
#include "mode.h"
#include "mutex.h"
#include "nvram.h"
#include "nvram_structure.h"
#include "PI_ClientPersistent.h"
#include "PI_Clients.h"
#include "PI_Utils.h"
#include "PortServer.h"
#include "PortServerUtils.h"
#include "quorum_comm.h"
#include "quorum_utils.h"
#include "realtime.h"
#include "RMCmdHdl.h"
#include "rtc.h"
#include "SerBuff.h"
#include "SerCon.h"
#include "ses.h"
#include "slink.h"
#include "sm.h"
#include "Snapshot.h"
#include "timer.h"
#include "trace.h"

#include "xk_raidmon.h"

extern void LL_Init(void);

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
#ifdef ENABLE_NG_HWMON
static void ng_hw_log(hwenv_ldata *, uint32_t level);
static void ng_hw_log_debug(hwenv_ldata *p) { ng_hw_log(p, LOG_TYPE_DEBUG); }
static void ng_hw_log_info(hwenv_ldata *p) { ng_hw_log(p, LOG_TYPE_INFO); }
static void ng_hw_log_warn(hwenv_ldata *p) { ng_hw_log(p, LOG_TYPE_WARNING); }
static void ng_hw_log_err(hwenv_ldata *p) { ng_hw_log(p, LOG_TYPE_ERROR); }

static struct hw_mon_register_data linit_data =
{
    .top_level_name = NULL,         /**< Top level name to use for device */
    .conf_file = NULL,              /**< Configuration file (NULL if statically linked)*/
    .reserved = 0,                  /**< reserved */
    .flags = 0,                     /**< flags for hw_mon (see Initialization Flags ) */
    .log_debug = ng_hw_log_debug,   /**< Pointer To debug log function      */
    .log_info = ng_hw_log_info,     /**< Pointer To information log function*/
    .log_warn = ng_hw_log_warn,     /**< Pointer To warning log function    */
    .log_error = ng_hw_log_err,     /**< Pointer To error log function      */
};

/*****************************************************************************
** FUNCTION NAME: ng_hw_log
**
** PARAMETERS:  hw_data, level
**
** DESCRIPTION: Log hw_data at level
**
** RETURNS: None
******************************************************************************/
void ng_hw_log(hwenv_ldata *hw_data, uint32_t level)
{
    /* Log and send async event.  */

    LogMessage(level, "%s", hw_data->logstr);
    if (level != LOG_TYPE_DEBUG)
    {
        EnqueuePIAsyncEvent(ASYNC_ENV_CHANGE, PIASYNC_EVENT_FIRST32_MAP);
    }
}
#endif /* ENABLE_NG_HWMON */

/*****************************************************************************
** Code Start
*****************************************************************************/

/*****************************************************************************
** FUNCTION NAME: Start
**
** PARAMETERS:  None
**
** DESCRIPTION: Misc system level task initialization
**
******************************************************************************/
void Start(UNUSED TASK_PARMS *startParms)
{
    TASK_PARMS  parms;

    dprintf(DPRINTF_DEFAULT, "Executing Start()\n");

    /* Seed random number generator */
    srandom(RTC_GetLongTimeStamp());

    /* Indicate that we're executing on mainline code (non-interrupt) */
    SetProcessorState(IDR_PROCESSOR_STATE_NORMAL);

    /* Initialize the CCB/Proc mode data (dprintf control) */
    InitModeData();

    /* Set the flag to indicate that the code is running from DRAM.  */
    SetRunningFromDRAMFlag(TRUE);

    /* Configure the serial ports - (interrupt disabled mode) */
    SerialInit();

    dprintf(DPRINTF_DEFAULT, "CCB runtime code starting...\n");

    /*
     * Turn on the front-bezel 'CCB' LED to indicate the runtime
     * code is active.
     */
    LEDSetHeartbeat(TRUE);

    /* Turn on the CommFault LED, since the Ethernet is not yet ready. */
    LEDSetCommFault(TRUE);

    LEDClearCode();             /* Turn off all LEDs */

    /* Emulate the boot setting its status flag. */
    IDRData.bootStatus |= IDR_COORDINATED_BOOT_GOOD;

    InitModeData();             /* Initialize Mode Data */

    /* Initialize mutexes */
    dprintf(DPRINTF_DEFAULT, "Initialize mutexes\n");

    InitLogMutex();
    InitMutex(&fileIOMutex);
    InitMutex(&fileSystemMutex);
    InitMutex(&bigBufferMutex);
    InitMutex(&configUpdateMutex);
    InitMutex(&sendMutex);
    InitMutex(&configJournalMutex);
    InitMutex(&backtraceMutex);
    InitMutex(&SM_mpMutex);
    InitMutex(&gPICommandMutex);
    InitMutex(&gClientsMutex);
    InitMutex(&gMgtListMutex);
    /* Following are the session manager locks (ipc_session_manager.c). */
    InitMutex(&sessionListMutex);
    InitMutex(&packetQueueMutex);
    InitMutex(&outStandingMutex);
    InitMutex(&inProgressMutex);
    InitMutex(&sequenceNumberMutex);
    InitMutex(&historyMutex);
    InitMutex(&runningTasksMutex);

    dprintf(DPRINTF_DEFAULT, "InitDdrTable\n");
    InitDdrTable();             /* Initialize the DDR table */

    dprintf(DPRINTF_DEFAULT, "InitCCBDMCtable\n");
    InitCCBDMCtable();             /* Initialize the DMC table */

    dprintf(DPRINTF_DEFAULT, "Initializing logs...\n");
    InitLogs(INIT_ON_FIRST_EVENT);      /* Initialize Logging */

    /* Enable code tracing (on always by default for now). */

    if (evQueue.evControl == 0)
    {
        if (evQueue.evBaseP == 0)
        {
            TraceInit();
        }

        /* Allow all trace types for now */
        evQueue.evControl = TRACE_MRP |
            TRACE_PACKET |
            TRACE_IPC | TRACE_LOG | TRACE_X1 | TRACE_X1_VDC | TRACE_X1_BF | TRACE_SIGNAL;
    }

    /* Fork the support tasks */
    dprintf(DPRINTF_DEFAULT, "Starting to fork support tasks...\n");

    /* Copy NVRAM backtrace data to flash */
    dprintf(DPRINTF_DEFAULT, "  Copying backtrace data to flash\n");
    CopyNVRAMBacktraceDataToFlash();

    /* Fork the fibre channel health monitor task */
    dprintf(DPRINTF_DEFAULT, "  Fork FCM_MonitorTask\n");
    TaskCreate(FCM_MonitorTask, NULL);
    TaskSwitch();

    /* Fork the Async Event Executive (logs) */
    dprintf(DPRINTF_DEFAULT, "  Fork AsyncExecTask(log)\n");
    parms.p1 = (UINT32)&logEventQ;
    parms.p2 = (UINT32)AsyncEventHandler;
    logEventQ.pcb = (PCB *)TaskCreate(AsyncExecTask, &parms);
    TaskSwitch();

    /* Fork the Async Event Executive (dlm) */
    dprintf(DPRINTF_DEFAULT, "  Fork AsyncExecTask(dlm)\n");
    parms.p1 = (UINT32)&dlmEventQ;
    parms.p2 = (UINT32)FibreShimHandler;
    dlmEventQ.pcb = (PCB *)TaskCreate(AsyncExecTask, &parms);
    TaskSwitch();

    /* Do the old L$init stuff - from link960.as */

    /*
     * Connect the Message Unit interrupt handler before calling the
     * the link layer init code.
     */
    dprintf(DPRINTF_DEFAULT, "  Fork LL_Init\n");
    LL_Init();

    TaskSwitch();

    /* Start the CCB statistics task */
    dprintf(DPRINTF_DEFAULT, "  Fork CCBStatsTask\n");
    TaskCreate(CCBStatsTask, NULL);
    TaskSwitch();

    /*********************************************************************
    * Start everything here that doesn't need the backend running.
    * Get as much out of the way here, since we reach this point
    * long in advance of the backend coming ready.
    ********************************************************************/

    /* Start up the serial console interface */
    dprintf(DPRINTF_DEFAULT, "  Fork the buffered serial port tasks\n");

    TaskCreate(SerialUserPortTransmitTask, NULL);
    TaskCreate(SerialUserPortReceiveTask, NULL);
    TaskCreate(SerialConsoleMainTask, NULL);
    TaskSwitch();

    /* Get controller setup infromation */
    dprintf(DPRINTF_DEFAULT, "  Getting Controller Setup information from NVRAM\n");
    LoadControllerSetup();

    /* Start the I2C monitor task */
    dprintf(DPRINTF_DEFAULT, "  Fork HWM_MonitorTask\n");
#ifdef ENABLE_NG_HWMON
    {
        static char cnNodeStr[32];

        /* Set initial data. */
        linit_data.top_level_name = cnNodeStr;

        if (CntlSetup_GetControllerSN())
        {
            sprintf(cnNodeStr, "CN%u Board", (CntlSetup_GetControllerSN() & 0x0F));
        }
        else
        {
            sprintf(cnNodeStr, "CN Board");
        }

        if (hw_hwmon_init(&linit_data) != 0)
        {
            dprintf(DPRINTF_DEFAULT, "  Fork HWM_MonitorTask FAILED\n");
        }
    }
#endif /* ENABLE_NG_HWMON */

    TaskCreate(HWM_MonitorTask, NULL);
    TaskSwitch();

    /*
     * McMaster GeoRaid debug - This safeguard will get cleared after the first
     * successful election.  Until then, each time the controller is restarted,
     * print a debug message to remind us of the disaster condition.
     */
    if (EL_DisasterCheckSafeguard() != GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "*** Disaster safeguard in place ***\n");
    }

    /*********************************************************************
    * Wait for FE/BE to come up.
    * Everything after this point NEEDS to have the FE/BE operational
    * (or timed out) in order to run.
    ********************************************************************/
    dprintf(DPRINTF_DEFAULT, "  InitProcessorComm\n");
    InitProcessorComm();

    /*
     * If the controller is not configured, wait for it to be
     * configured.
     */
    dprintf(DPRINTF_DEFAULT, "  Checking if controller is setup\n");
    WaitForControllerSetup();

    /* Initialize cache variables */
    dprintf(DPRINTF_DEFAULT, "  Initialize CCB Cache Variables\n");
    CacheMgr_Init();

    /* Start Port Servers */
    dprintf(DPRINTF_DEFAULT, "  Fork PortServer(s)\n");

    /* Initialize the platform interface clients information. */
    pi_clients_initialize();

    /*
     * If the system is configured, start the PI Server on all ports.
     * Otherwise, just open the 3000 port and wait for configuration packet.
     */
    if (IsConfigured())
    {
        /* Start the EWOK Port Server (3000) */
        parms.p1 = EWOK_PORT_NUMBER;
        TaskCreate(DebugServer, &parms);

        /* Start the TEST Port Server (3100) */
        parms.p1 = TEST_PORT_NUMBER;
        TaskCreate(DebugServer, &parms);

        /* Start the DEBUG Port Server (3200) */
        parms.p1 = DEBUG_PORT_NUMBER;
        TaskCreate(DebugServer, &parms);
    }
    else
    {
        /* Start the EWOK Port Server (3000) */
        parms.p1 = EWOK_PORT_NUMBER;
        TaskCreate(DebugServer, &parms);
    }

    TaskSwitch();

    /* Start SES task */
    dprintf(DPRINTF_DEFAULT, "  Fork InitSES\n");
    TaskCreate(InitSES, NULL);
    TaskSwitch();

    /* Start up slave failure manager initialization */
    dprintf(DPRINTF_DEFAULT, "  Fork SlaveFailureManagerInit\n");
    TaskCreate(SlaveFailureManagerInit, NULL);
    TaskSwitch();

    /* Start the Quorum communications task */
    dprintf(DPRINTF_DEFAULT, "  Fork QuorumManager\n");
    TaskCreate(QuorumManager, NULL);
    TaskSwitch();

    /* Start the heartbeat and statistics task */
    dprintf(DPRINTF_DEFAULT, "  Fork IpcHeartbeatAndStatsTask\n");
    TaskCreate(IpcHeartbeatAndStatsTask, NULL);
    TaskSwitch();

#ifndef DISABLE_LOCAL_RAID_MONITORING
    /* Start the local raid monitor, if needed. */
    if (hwm_platform->flags & PLATFORM_FLAG_LOCAL_RAID)
    {
        dprintf(DPRINTF_DEFAULT, "  Fork XK_RaidMonitorTask\n");
        TaskCreate(XK_RaidMonitorTask, NULL);
        TaskSwitch();
    }
#endif  /* DISABLE_LOCAL_RAID_MONITORING */

    InitClientPersistent();     /* Initialize EwokPersistant memory */

    dprintf(DPRINTF_DEFAULT, "  CPSInitController\n");
    CPSInitController();        /* Call CPSInitController */

    /* Force the controller to take a baseline measurement of FCAL counters */
    dprintf(DPRINTF_DEFAULT, "  FCM_CountersMajorStorageEvent\n");
    FCM_CountersMajorStorageEvent();

    /*
     * It's time for Start to end gracefully... by doing a return here,
     * TaskEndNoRelease is called (see the first couple of line of this
     * function) and the Start task is pulled from the active task list.
     */
    dprintf(DPRINTF_DEFAULT, "Startup completed...\n");

    /*
     * This is work around fix to create a task which will force the
     * cache to refresh for every 15secs. This is just to support the cache
     * feature for VDISKS and RAIDS to apps. This is required because cache
     * is refreshed only event based. But Ewok is dependent vdiskscache command
     * for stats of vdisks. So this is not updated in cache until event happens.
     * We are creating another task which will run and inject a event internally
     * and make the refresh happens
     */
    TaskCreate(VdisksCacheRefreshInjectTask, NULL);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
