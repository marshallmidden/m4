/* $Id: rm_val.c 161259 2013-06-26 20:16:02Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       rm_val.c
**
**  @brief      Resource Manager Group Redundancy Validation
**
**  Definition of the different types of validation
**  ===============================================
**  1.) CheckHardware() is run for all validation types. It checks that valid
**  BE and FE port pairs exist on all controllers. It also compares the ports
**  on the master to all other controllers to verify that they are the same.
**
**  - Error information: Port(s) missing on a given controller. Assumes that
**  if one port of a pair is installed that the user intended that both be
**  installed.
**
**
**  2.) VAL_TYPE_STORAGE - Verify that all bays and all PDisks can be seen on
**  a valid port pair.
**
**  - Error information: Bay ID, PDisk ID / WWN and missing port
**
**
**  3.) VAL_TYPE_SERVER -
**   a.) Check that all ACTIVE servers on the master can be seen on a valid
**       port pair.
**   b.) Check that each controller sees the same number of servers as the
**       master.
**   c.) Check that the master and slave see each server on the same ports.
**
**  - Error information: Server WWN, missing port, server counts
**
**
**  4.) VAL_TYPE_COMM - Send a ping across ethernet and fibre from the master
**  to each slave.
**
**  - Error information: Ping type and controllers
**
**
**  5.) VAL_TYPE_BE_LOOP - Verify that a given BE port on the master is
**  connected to the same BE port on all slaves. This is done by checking
**  that all controllers see a drive bay on the same ports.
**
**  - Error information: Controller and port number
**
**
**  6.) VAL_TYPE_SHELF_ID - Check that no two bays on a given path have the
**  same shelf ID.
**
**  - Error information: Bay ID (letter) and shelf ID
**
**
**  7.) VAL_TYPE_SYS_REL - Verify that the System Release Level of all slaves
**  matches the release level of the master.
**
**  - Error information: System release level, controller ID
**
**
**  Copyright (c) 2002-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "rm_val.h"

#include "AsyncEventHandler.h"
#include "CacheBay.h"
#include "CacheLoop.h"
#include "CachePDisk.h"
#include "CacheServer.h"
#include "CmdLayers.h"
#include "cps_init.h"
#include "debug_files.h"
#include "EL.h"
#include "ipc_sendpacket.h"
#include "LOG_Defs.h"
#include "logdef.h"
#include "logview.h"
#include "misc.h"
#include "MR_Defs.h"
#include "PacketInterface.h"
#include "PktCmdHdl.h"
#include "PI_Utils.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "rm.h"
#include "RMCmdHdl.h"
#include "RM_headers.h"
#include "rtc.h"
#include "XIOPacket.h"
#include "XIO_Std.h"
#include "X1_Structs.h"

#if defined(MODEL_7000) || defined(MODEL_4700)
#define  BAY  "ISE"
#else  /* MODEL_7000 || MODEL_4700 */
#define  BAY  "DISKBAY"
#endif /* MODEL_7000 || MODEL_4700 */

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/

/**
** Delay (in seconds) between creating the validation task and when the
** task actually runs. This delay allows the syatem to (hopefully) come
** to a stable state before running the validation tests.
**/
#define GROUP_VALIDATION_TASK_DELAY     90

/**
** Each time a validation trigger event is received the validation delay
** timer is reset to GROUP_VALIDATION_TASK_DELAY. Events that occur on
** a periodic basis may cause the timer to be reset and never expire.
** After VALIDATION_DELAY_RESET_MAX resets, the code will force
** validation to run.
**/
#define VALIDATION_DELAY_RESET_MAX      100

/**
** VAL_TYPE_ALL_TESTS is a private define. It is used to translate the
** user flag VAL_TYPE_ALL into the actual tests that will run. This way
** the actual tests inside validation can change but the user still gets
** all the tests available by using VAL_TYPE_ALL.
**/
#define VAL_TYPE_ALL_TESTS     (VAL_TYPE_HW        | VAL_TYPE_STORAGE  | \
                                VAL_TYPE_SERVER    | VAL_TYPE_COMM     | \
                                VAL_TYPE_BE_LOOP   | VAL_TYPE_SHELF_ID | \
                                VAL_TYPE_SYS_REL                            )

/**
** Port map constants. These are used to indicate that a "port pair"
** is present.
** "Low Cost Cluster" system will have only 1 FE card in slot 0.
**/
#define VAL_PORTS_0             0x01
#define VAL_PORTS_0_1           0x03
#define VAL_PORTS_2_3           0x0C
#define VAL_PORTS_ALL           0x0F
// #define PORT_MAP_INVALID        0x00

#define LOCAL_CTRL_INDEX        0

/**
** Bit flags indicating failure of port validation.
**/
#define VAL_PORTS_ALL_GOOD      0x00
#define VAL_PORTS_FE_FAILED     0x01
#define VAL_PORTS_BE_FAILED     0x02

/**
** Error codes returned by ValidateStorageBay(). Used to indicate
** path errors.
*/
#define PATH_COUNT_LT_2             0x01        /* Path count < 2               */
#define PATH_COUNT_GT_2             0x02        /* Path count > 2               */
#define PATHS_NOT_UNIQUE            0x03        /* Paths are not unique         */
#define PATHS_PDISK_UNKNOWN_WWN     0x04        /* OUI of WWN not recognized    */

/*
** Message length template - 40 chars
**  0123456789012345678901234567890123456789
*/

/*
** Note on Validation Log Entries:
**
** VAL_MSG_LENGTH defines the largest message that can be created by the
** Validation code. The GUI will truncate all messages at 40 characters
** so best effort is made to get the most important information into the
** first 40 characters.
*/

/*
** Validation error codes
** ----------------------
** The original design was to return a code which would contain some
** information about the error and point the user to a more detailed
** service procedure. These codes are not currently used but have
** been maintained in the code for possible future use.
*/

/*
** For the Hardware warnings below, the bit fields within the error code
** indicate the following -
**
**  b31-28  27-24   23-20   19-16   15-12   11-8    7-4     3-0
**                  Ctrl SN (LSB)   RSVD    RSVD    RSVD    Proc
**
*/
#define VAL_HW_NO_PORT_LIST             0x10000000      /* Can't get port list  */
#define VAL_HW_NO_PORT_PAIR             0x11000000      /* Valid pair not found */
#define VAL_HW_BE_PORT_MISMATCH         0x12000000      /* Master and slave port */
#define VAL_HW_FE_PORT_MISMATCH         0x13000000      /*  lists not identical */

#define LOCAL_BE_PROC                   0x00000001
#define REMOTE_BE_PROC                  0x00000002
#define LOCAL_FE_PROC                   0x00000003
#define REMOTE_FE_PROC                  0x00000004

/*
** For the Storage warnings below, the bit fields within the error code
** indicate the following -
**
**  b31-28  27-24   23-20   19-16   15-12   11-8    7-4     3-0
**                  Ctrl SN (LSB)   PDisk ID-------------------
**
*/
#define VAL_STORAGE_LOCAL               0x20000000
#define VAL_STORAGE_REMOTE              0x30000000

/*
** For the Back End Loop warnings below, the bit fields within the error code
** indicate the following -
**
**  b31-28  27-24   23-20   19-16   15-12   11-8    7-4     3-0
**          Port    Ctrl SN (LSB)   RmtPort-----    Rmt CN ID--
**
**  Port        - local port number
**  Ctrl SN     - local controller serial number (LSB)
**  RmtPort     - port number on remote controller
**  Rmt CN ID   - remote controller node ID
*/
#define VAL_BACK_END_LOOP               0x40000000

/*
** For the Shelf ID warnings below, the bit fields within the error code
** indicate the following -
**
**  b31-28  27-24   23-20   19-16   15-12   11-8    7-4     3-0
**                  Ctrl SN (LSB)   ShelfID-----    BayID------
**
**  Port        - port (loop) having non-unique shelf IDs
**  ShelfID     - shelf ID duplicated on port above
*/
#define VAL_SHELF_ID                    0x50000000

/*
** For the Server warnings below, the bit fields within the error code
** indicate the following -
**
**  b31-28  27-24   23-20   19-16   15-12   11-8    7-4     3-0
**          PortMap Ctrl SN (LSB)   Server WWN (low 16 bits)---
**
**  PortMap - bit map of ports this server is visible on
*/
#define VAL_SERVERS_LOCAL               0x70000000
#define VAL_SERVERS_REMOTE              0x80000000

/*
** For the Communication warnings below, the bit fields within the error code
** indicate the following -
**
**  b31-28  27-24   23-20   19-16   15-12   11-8    7-4     3-0
**                  Ctrl SN (LSB)   Reserved-------------------
**
*/
#define VAL_COMM_IPC_ETH_FAIL           0xA1000000      /* Ethernet comm failed */
#define VAL_COMM_IPC_FIBRE_FAIL         0xA2000000      /* Fibre comm failed    */

#define VAL_SYSTEM_RELEASE              0xB0000000

/*
** For the QLogic validation warnings below, the bit fields within the error
** code indicate the following -
**
**  b31-28  27-24   23-20   19-16   15-12   11-8    7-4     3-0
**          Port    RSVD    RSVD    RSVD    RSVD    RSVD    Proc
*/
#define VAL_QLOGIC_BASE                 0xC0000000
#define VAL_QLOGIC_NO_INFO              0xC1000000

/*
** The top nibble of the validation code indicates the type of validation
** event - i.e. Storage, Server, Comm, etc.
*/
#define VAL_TYPE_MASK                   0xF0000000

/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/
#define GetBEPortMap(index1)             (bePortMaps[index1])
#define GetFEPortMap(index1)             (fePortMaps[index1])

/*
******************************************************************************
** Private variables
******************************************************************************
*/
static UINT8 gValidationTaskActive = 0;
static UINT32 gValidationType = 0;
static UINT8 gValidationTaskDelayCounter = GROUP_VALIDATION_TASK_DELAY;
static UINT8 gValidationDelayResetCount = VALIDATION_DELAY_RESET_MAX;
static bool gValidationError = false;

static UINT8 bePortMaps[MAX_CONTROLLERS];
static UINT8 fePortMaps[MAX_CONTROLLERS];

#if defined(MODEL_3000) || defined(MODEL_7400)
static UINT8 bayName[] =                    /* This must go up to 64 for MAX_DISK_BAYS index. */
{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',   /* Assume 8 per loop for readability. */
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',   /* 8-15 */
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',   /* 16-23 */
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',   /* 24-31 */
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',   /* 32-49 */
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',   /* 40-47 */
  'w', 'x', 'y', 'z', '0', '1', '2', '3',   /* 48-55 */
  '4', '5', '6', '7', '8', '9', '.', ':',   /* 56-63 */
};
#endif /* MODEL_3000 || MODEL_7400 */

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static void DailyGroupValidationTask(TASK_PARMS *parms);
static void GroupValidationTask(TASK_PARMS *parms);

static void RunValidationTests(void);
static void ValidationRequest(UINT32 controllerSN);
static UINT8 CheckHardware(void);

static void CheckQLogicSettings(void);
static bool ValidateQLogicSettings(UINT32 controllerSN, UINT8 processor,
                                   PI_STATS_LOOPS_RSP *pStatsLoops);

static int CheckPathsToFibreStorage(void);
static void CheckBELoops(void);

#if defined(MODEL_3000) || defined(MODEL_7400)
static void CheckShelfId(void);
#endif /* MODEL_3000 || MODEL_7400 */
static void CheckPathsToServers(void);
static void CheckConnectivity(void);
static void CheckFWSystemRelease(void);

static UINT8 ValidatePorts(UINT32 type, UINT32 controllerSN, UINT8 *pPortBitMap);
static INT32 ValidateStorage(XIO_PACKET *pXIORsp, UINT32 type, UINT32 controllerSN, int *seen_bays, int *seen_disks);
static INT32 ValidateStorageBays(XIO_PACKET *pXIORsp, UINT32 type, UINT32 controllerSN, int *seen_bays);
static UINT8 ValidateStorageBay(XIO_PACKET *pXIORsp, UINT16 bayID, UINT8 *pBitPath);
static void ValidateStorageRemoveBay(XIO_PACKET *pXIORsp, UINT16 bayID);
static UINT8 ValidateStoragePath(MRGETBEDEVPATHS_RSP_ARRAY *pDevPath, UINT8 *pBitPath);

static void LogValidationError(UINT32 errorCode, UINT32 controllerSN, char *text);

static UINT8 BitMapToString(UINT8 bitMap, char *pStr, UINT8 flag);
static bool CheckFWUpdateInProgress(void);
static UINT8 FindMissingPorts(UINT8 installedPortMap);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Starts the daily validation task.
**
**  @param      none
**
**  @return     none
**
**  @attention  Creates DailyGroupValidationTask
**
******************************************************************************
**/
void RM_StartDailyGroupValidation(void)
{
    TaskCreate(DailyGroupValidationTask, NULL);
}

/**
******************************************************************************
**
**  @brief      Task to start a group validation once per day.
**
**  @param      dummy   Required parameter for forking a task.
**
**  @return     none
**
******************************************************************************
**/
static NORETURN void DailyGroupValidationTask(UNUSED TASK_PARMS *parms)
{
    TIMESTAMP   ts;
    UINT32      delay;

    while (1)
    {
        RTC_GetTimeStamp(&ts);

        /*
         * Calculate the initial number of milliseconds to wait. To
         * get valid values, convert from BCD to Short.
         *
         * - # of hours between current hour and midnight
         * - Multiply by 60 to convert to minutes
         * - Add # of minutes between current minute and next hour
         * - Multiply by 60 to convert to seconds
         * - Multiply by 1000 to convert to milliseconds
         */
        delay = (HOURS_IN_ONE_DAY - BCD2Short((UINT16)ts.hours));
        delay *= MINUTES_IN_ONE_HOUR;
        delay += (MINUTES_IN_ONE_HOUR - BCD2Short((UINT16)ts.minutes));
        delay *= SECONDS_IN_ONE_MINUTE;
        delay *= 1000;

        LogMessage(LOG_TYPE_DEBUG, "Delay %u ms before daily validation", delay);

        TaskSleepMS(delay);

        LogMessage(LOG_TYPE_DEBUG, "Starting daily validation");

        RM_StartGroupValidation(VAL_TYPE_DAILY);
    }
}

/**
******************************************************************************
**
**  @brief      Validation entry point
**
**  @param      validationFlags     Define which validation types to run.
**
**  @return     none
**
******************************************************************************
**/
void RM_StartGroupValidation(UINT32 validationFlags)
{
#if 0
    dprintf(DPRINTF_RMVAL, "RM_StartGroupValidation - ENTER (validationFlags=0x%X)\n",
            validationFlags);
#endif  /* 0 */

    /* Only start the Validation Task if Power up is complete */
    if (PowerUpComplete())
    {
        /*
         * If the run immediate flag is set zero out the counters.
         * This will cause validation to start without a delay but
         * still preserve the rest of the checks required.
         */
        if (validationFlags & VAL_RUN_IMMED)
        {
            gValidationTaskDelayCounter = 0;
            gValidationDelayResetCount = 0;
        }
        else if (gValidationDelayResetCount > 0)
        {
            /*
             * Initialize the validation delay counter. This is used to
             * "buffer up" validation requests so validation is run at
             * reasonable intervals. Repeated calls to this function at
             * intervals less than GROUP_VALIDATION_TASK_DELAY would prevent
             * validation from running. Therefore when the reset count
             * is exhausted the delay counter will not be reloaded.
             * Since the delay reset count also has sleep interval, the
             * time before validation actually starts is variable.
             *
             * 1.   If gValidationTaskDelayCounter is not reset by another call
             *      to RM_StartGroupValidation then validation will run in
             *      GROUP_VALIDATION_TASK_DELAY + 1 seconds.
             * 2.   If calls to RM_StartGroupValidation are coming continuously
             *      then validatiuon will run in approximately
             *      GROUP_VALIDATION_TASK_DELAY + VALIDATION_DELAY_RESET_MAX
             *      seconds.
             * 3.   If gValidationTaskDelayCounter has almost exprired before
             *      being reset and this condition happens repeatedly, the
             *      worstcase delay before validation runs will be
             *      GROUP_VALIDATION_TASK_DELAY * VALIDATION_DELAY_RESET_MAX
             */
            gValidationTaskDelayCounter = GROUP_VALIDATION_TASK_DELAY;
            gValidationDelayResetCount--;

#if 0
            dprintf(DPRINTF_RMVAL, "RM_StartGroupValidation - gValidationDelayResetCount=%d\n",
                    gValidationDelayResetCount);
#endif  /* 0 */
        }

        /*
         * OR the input flags with the global validation type flags.
         * If we were waiting to run validation and another request came
         * in, both will run when the timer expires.
         */
        gValidationType |= validationFlags;

        /* If the validation task is not already active, create it now. */
        if (!gValidationTaskActive)
        {
            gValidationTaskActive = TRUE;
            TaskCreate(GroupValidationTask, NULL);
        }
    }
}

/**
******************************************************************************
**
**  @brief      Task created to run validation
**
**              Task to check and report on the group's current redundancy
**              state. This includes checking for multiple paths to the
**              enclosures, physical disks, servers and other things of
**              that nature.
**
**  @param      dummy   Required parameter for a forked task.
**
**  @return     none
**
******************************************************************************
**/
static void GroupValidationTask(UNUSED TASK_PARMS *parms)
{
    dprintf(DPRINTF_DEFAULT, "GroupValidationTask - Task started.\n");

    /*
     * Loop if there is an election in progress or if RM is running.
     * The frequency that validation runs at is controlled by
     * gValidationTaskDelayCounter.
     */
    while ((gValidationTaskDelayCounter > 0) ||
           (EL_TestInProgress() == TRUE) ||
           (CheckFWUpdateInProgress() == TRUE) ||
           ((RMGetState() != RMRUNNING) && (RMGetState() != RMDOWN)))
    {
        /* Wait until the delay counter expires before running validation. */
        if (gValidationTaskDelayCounter > 0)
        {
            gValidationTaskDelayCounter--;
        }
        TaskSleepMS(1000);
    }

    LogMessage(LOG_TYPE_DEBUG, "GroupValidationTask - processing request type=0x%04X",
               gValidationType);

    /* If we are the master controller, handle the validation request. */
    if (TestforMaster(GetMyControllerSN()))
    {
        RunValidationTests();
    }
    else
    {
        /* Tunnel the validation request to the master. */
        ValidationRequest(Qm_GetMasterControllerSN());
    }

    /*
     * The group validation is complete so reset the flag that indicates
     * the task is active or not.
     */
    gValidationTaskActive = FALSE;
    gValidationType = 0;
    gValidationError = false;
    gValidationDelayResetCount = VALIDATION_DELAY_RESET_MAX;
}

/**
******************************************************************************
**
**  @brief      Do a ccbCL.pl a: resetqlogic be all
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void do_a_reset_qlogic_be_all(void)
{
    XIO_PACKET  reqPkt = { NULL, NULL };
    XIO_PACKET  rspPkt = { NULL, NULL };
    PI_RESET_PROC_QLOGIC_REQ *req;
    INT32       rc = PI_GOOD;
    UINT32      slaveSN = 0;
    INT16       slaveIndex = 0;

    reqPkt.pHeader = MallocWC(sizeof(*reqPkt.pHeader));
    reqPkt.pHeader->commandCode = PI_PROC_RESET_BE_QLOGIC_CMD;
    reqPkt.pHeader->length = sizeof(*req);
    reqPkt.pHeader->packetVersion = 1;

    req = MallocWC(sizeof(*req));
    reqPkt.pPacket = (UINT8 *)req;

    rspPkt.pHeader = MallocWC(sizeof(*rspPkt.pHeader));
    rspPkt.pPacket = NULL;
    rspPkt.pHeader->packetVersion = 1;


    /* Fill in the request parms. */
    req->port = RESET_PORT_ALL;
    req->option = RESET_PORT_INIT;

    rc = PortServerCommandHandler(&reqPkt, &rspPkt);

    /* Issue command on slave. */
    while ((slaveSN = GetNextRemoteControllerSN(&slaveIndex)))
    {
        UINT8   retries = 2;                /* Ethernet, Fiber(1), Disk Quorum(2) */

        /*
         * Memory for the response data for the previous request was allocated
         * by PortServerCommandHandler(). Free it before making tunnel request.
         */
        do
        {
            if (rc != PI_TIMEOUT)
            {
                Free(rspPkt.pPacket);
            }
            else
            {
                rspPkt.pPacket = NULL;
            }
            rc = TunnelRequest(slaveSN, &reqPkt, &rspPkt);
        } while (rc != GOOD && (retries--) > 0);
    }

    /*
     * Free all memory. Keep the response packet memory around if the request
     * timed out. Tunnel requests will not return a timeout error.
     */
    Free(reqPkt.pHeader);
    Free(reqPkt.pPacket);
    Free(rspPkt.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPkt.pPacket);
    }
}   /* End of do_a_reset_qlogic_be_all */

/**
******************************************************************************
**
**  @brief      Run the individual validation tests on the master controller
**
**              Call the individual functions which make up the set
**              of validation activities done on the master controller.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void RunValidationTests(void)
{
    UINT8       portStatus = VAL_PORTS_ALL_GOOD;
    int         flag_resetqlogic_needed = 0;

    /*
     * The input flag VAL_TYPE_ALL must be "translated" to the actual set
     * of available tests.
     */
    if (gValidationType & VAL_TYPE_ALL)
    {
        gValidationType |= VAL_TYPE_ALL_TESTS;
    }

    /*
     * Check and report on the current hardware configuration
     * on the active controllers - inter and intra controller.
     */
    portStatus = CheckHardware();

    /*
     * Check and report on the current QLogic settings on
     * the active controllers.
     */
    CheckQLogicSettings();

    /*
     * Check and report on the paths available to the fibre
     * storage bays on the active controllers - inter and
     * intra controller.
     */
    if ((gValidationType & VAL_TYPE_STORAGE) && !(portStatus & VAL_PORTS_BE_FAILED))
    {
        flag_resetqlogic_needed |= CheckPathsToFibreStorage();
    }

#if defined(MODEL_3000) || defined(MODEL_7400)
    /*
     * Validate proper Disk Bay Shelf IDs on the back end loops.
     * (Don't validate if this is a Nitrogen system)
     */
    if (gValidationType & VAL_TYPE_SHELF_ID)
    {
        CheckShelfId();
    }
#endif /* MODEL_3000 || MODEL_7400 */

    /* The checks below are done only for an n > 1 system. */
    if (ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()) > 1)
    {
        /* Validate proper connections on the back end loops. */
        if ((gValidationType & VAL_TYPE_BE_LOOP) && !(portStatus & VAL_PORTS_BE_FAILED))
        {
            CheckBELoops();
        }

        /*
         * Check and report on the paths available to the servers
         * on the active controllers - inter and intra controller.
         */
        if ((gValidationType & VAL_TYPE_SERVER) && !(portStatus & VAL_PORTS_FE_FAILED))
        {
            CheckPathsToServers();
        }

        /*
         * Check and report on the connectivity between active
         * controllers - Ethernet and Fibre.
         */
        if (gValidationType & VAL_TYPE_COMM)
        {
            CheckConnectivity();
        }

        /* Validate firmware system release levels on all controllers. */
        if (gValidationType & VAL_TYPE_SYS_REL)
        {
            CheckFWSystemRelease();
        }
    }

    if (flag_resetqlogic_needed != 0)
    {
        LogMessage(LOG_TYPE_DEBUG, "DSC validation resetqlogic be all needed.");
        do_a_reset_qlogic_be_all();
        LogMessage(LOG_TYPE_DEBUG, "DSC validation resetqlogic be all finished.");
    }

    /* Log a validation completion message */
    if (gValidationError)
    {
        LogMessage(LOG_TYPE_DEBUG, "DSC validation completed with ERROR(s)");
    }
    else
    {
        LogMessage(LOG_TYPE_DEBUG, "DSC validation completed GOOD");
    }
}

/**
******************************************************************************
**
**  @brief      Handle a validation request based on the input controller
**              serial number
**
**  @param      controllerSN    Controller that will execute the validation
**                              request.
**
**  @return     none
**
******************************************************************************
**/
static void ValidationRequest(UINT32 controllerSN)
{
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };
    INT32       rc = PI_GOOD;

    /*
     * Allocate memory for the request and response headers and the
     * request data. Response data space is allocated at a lower level.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_VCG_VALIDATION_REQ));
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /*
     * Fill in the request structure and issue the request through
     * the command layers.
     */
    reqPacket.pHeader->commandCode = PI_VCG_VALIDATION_CMD;
    reqPacket.pHeader->length = sizeof(PI_VCG_VALIDATION_REQ);

    ((PI_VCG_VALIDATION_REQ *)(reqPacket.pPacket))->validationFlags = gValidationType;

    /*
     * If the validation request is for this controller make
     * the request to the port server directly. If it is for
     * one of the slave controllers then tunnel the request to
     * that controller.
     */
    if (controllerSN == GetMyControllerSN())
    {
        /*
         * Issue the command through the top-level command handler.
         * Validate the ports and generate a port bit map to be used later.
         */
        rc = PortServerCommandHandler(&reqPacket, &rspPacket);
    }
    else
    {
        UINT8   retries = 2;                /* Ethernet, Fiber(1), Disk Quorum(2) */

        do
        {
            if (rc != PI_TIMEOUT)
            {
                Free(rspPacket.pPacket);
            }
            else
            {
                rspPacket.pPacket = NULL;
            }
            rc = TunnelRequest(controllerSN, &reqPacket, &rspPacket);
        } while (rc != GOOD && (retries--) > 0);
    }

    /* Print a debug message based on the results of the request. */
    if (rc == PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "ValidationRequest completed GOOD.\n");
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "ValidationRequest FAILED: rc=0x%X  status=0x%X  errorCode: 0x%X\n",
                rc,
                ((PI_PACKET_HEADER *)(rspPacket.pHeader))->status,
                ((PI_PACKET_HEADER *)(rspPacket.pHeader))->errorCode);
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }
}

/**
******************************************************************************
**
**  @brief      Check and report on the current hardware configuration -
**              inter and intra controller.
**
**              1. INTRA - Verify BE and FE ports exist in the proper pairs
**                 i.e. 0&1, 2&3
**              2. INTER - Verify that the ports listed above also exist
**                 on the other controller in the group.
**
**  @param      none
**
**  @return     portStatus  Flags indicating general status of BE and FE ports.
**
******************************************************************************
**/
static UINT8 CheckHardware(void)
{
    UINT32      slaveSN = 0;
    INT16       slaveIndex = 0;
    INT16       mapIndex = LOCAL_CTRL_INDEX;
    UINT8       portStatus = VAL_PORTS_ALL_GOOD;
    char        debugStr[16];
    char       *pMsg;
    char       *pStr;

    dprintf(DPRINTF_RMVAL, "CheckHardware - ENTER\n");

    /*
     * Allocate memory for a text message. Initialize another pointer
     * to this string.
     */
    pMsg = MallocWC(VAL_MSG_LENGTH);
    pStr = pMsg;

    /* Validate the local BE ports. */
    portStatus = ValidatePorts(LOCAL_BE_PROC, GetMyControllerSN(), &bePortMaps[LOCAL_CTRL_INDEX]);

    /*
     * Convert port bit map to a string and NULL terminate it. Since
     * BitMapToString() will return ALL if no ports are installed, handle
     * that case here for the debug message.
     */
    if (GetBEPortMap(LOCAL_CTRL_INDEX) == 0)
    {
        strcpy(debugStr, "NONE");
    }
    else
    {
        BitMapToString(GetBEPortMap(LOCAL_CTRL_INDEX), debugStr, TRUE);
    }

    dprintf(DPRINTF_RMVAL, "  local BE Port(s) present (with listType=PORTS_GOOD): %s\n",
            debugStr);

    /* Checks below are only done if n > 1 */
    if (ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()) > 1)
    {
        /* Validate the local FE ports. */
        portStatus |= ValidatePorts(LOCAL_FE_PROC, GetMyControllerSN(),
                                    &fePortMaps[LOCAL_CTRL_INDEX]);

        /*
         * Convert port bit map to a string and NULL terminate it. Since
         * BitMapToString() will return ALL if no ports are installed, handle
         * that case here for the debug message.
         */
        if (GetFEPortMap(LOCAL_CTRL_INDEX) == 0)
        {
            strcpy(debugStr, "NONE");
        }
        else
        {
            BitMapToString(GetFEPortMap(LOCAL_CTRL_INDEX), debugStr, TRUE);
        }

        dprintf(DPRINTF_RMVAL, "  local FE Port(s) present (with listType=PORTS_GOOD): %s\n",
                debugStr);

        /*
         * Loop through all the slave controllers. BE and FE port maps are
         * saved for each controller.
         */
        mapIndex = LOCAL_CTRL_INDEX + 1;

        while ((slaveSN = GetNextRemoteControllerSN(&slaveIndex)) != 0)
        {
            portStatus |= ValidatePorts(REMOTE_BE_PROC, slaveSN, &bePortMaps[mapIndex]);

            /*
             * Convert port bit map to a string and NULL terminate it. Since
             * BitMapToString() will return ALL if no ports are installed,
             * handle that case here for the debug message.
             */
            if (GetBEPortMap(mapIndex) == 0)
            {
                strcpy(debugStr, "NONE");
            }
            else
            {
                BitMapToString(GetBEPortMap(mapIndex), debugStr, TRUE);
            }

            dprintf(DPRINTF_RMVAL, "  remote BE Port(s) present (with listType=PORTS_GOOD): %s\n",
                    debugStr);

            /*
             * Compare the port list on the slave with the list on the
             * local controller. They must be identical. It is possible
             * for the code above to detect valid port pairs on all
             * controllers. However if they are not the SAME pairs
             * the configuration is not valid. Don't compare local and
             * remote port maps unless port status is good to this point.
             */
            if (!(portStatus & VAL_PORTS_BE_FAILED) &&
                (GetBEPortMap(mapIndex) != GetBEPortMap(LOCAL_CTRL_INDEX)))
            {
                /*
                 * Log an error and clear the BE port map that is used
                 * for Storage Validation.
                 */
                pStr += sprintf(pStr, "STORAGE ports CN%d(",
                                GetCNIDFromSN(GetMyControllerSN()));

                pStr += BitMapToString(GetBEPortMap(LOCAL_CTRL_INDEX), pStr, FALSE);

                pStr += sprintf(pStr, ") CN%d(", GetCNIDFromSN(slaveSN));

                pStr += BitMapToString(GetBEPortMap(mapIndex), pStr, FALSE);

                pStr += sprintf(pStr, ")");

                LogValidationError(VAL_HW_BE_PORT_MISMATCH, slaveSN, pMsg);

                /*
                 * Clear the message buffer and reset the string pointer
                 * before building the next message.
                 */
                memset(pMsg, 0, VAL_MSG_LENGTH);
                pStr = pMsg;
            }

            /* Repeat for the remote front end port list. */
            portStatus |= ValidatePorts(REMOTE_FE_PROC, slaveSN, &fePortMaps[mapIndex]);

            /*
             * Convert port bit map to a string and NULL terminate it. Since
             * BitMapToString() will return ALL if no ports are installed,
             * handle that case here for the debug message.
             */
            if (GetFEPortMap(mapIndex) == 0)
            {
                strcpy(debugStr, "NONE");
            }
            else
            {
                BitMapToString(GetFEPortMap(mapIndex), debugStr, TRUE);
            }

            dprintf(DPRINTF_RMVAL, "  remote FE Port(s) present (with listType=PORTS_GOOD): %s\n",
                    debugStr);

            /*
             * Compare the port list on the slave with the list on the
             * master. They must be identical. FE Port status on both
             * controllers must be good.
             */
            if (!(portStatus & VAL_PORTS_FE_FAILED) &&
                (GetFEPortMap(mapIndex) != GetFEPortMap(LOCAL_CTRL_INDEX)))
            {
                /*
                 * Clear the message buffer and reset the string pointer
                 * before building the next message.
                 */
                memset(pMsg, 0x00, VAL_MSG_LENGTH);
                pStr = pMsg;

                pStr += sprintf(pStr, "HOST ports CN%d(",
                                GetCNIDFromSN(GetMyControllerSN()));

                pStr += BitMapToString(GetFEPortMap(LOCAL_CTRL_INDEX), pStr, FALSE);

                pStr += sprintf(pStr, ") CN%d(", GetCNIDFromSN(slaveSN));

                pStr += BitMapToString(GetFEPortMap(mapIndex), pStr, FALSE);

                pStr += sprintf(pStr, ")");

                LogValidationError(VAL_HW_FE_PORT_MISMATCH, slaveSN, pMsg);

                /*
                 * Clear the message buffer and reset the string pointer
                 * before building the next message.
                 */
                memset(pMsg, 0, VAL_MSG_LENGTH);
                pStr = pMsg;
            }
            mapIndex++;
        }
    }

    /*
     * Free all memory. Keep the response packet memory around if the
     * request timed out. Tunnel requests will not return a timeout error.
     */
    Free(pMsg);

    dprintf(DPRINTF_RMVAL, "CheckHardware - EXIT (portStatus=%s)\n",
            (portStatus == GOOD ? "GOOD" : "ERROR"));

    return (portStatus);
}


/**
******************************************************************************
**
**  @brief      Check and report on the current QLogic card settings.
**
**              1. Verify the J2 jumper is set for 2310 cards.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CheckQLogicSettings(void)
{
    UINT32      index1;
    UINT32      configIndex;
    UINT32      controllerSN;
    PI_STATS_LOOPS_RSP *pStatsLoops;
    bool        bError = false;

    dprintf(DPRINTF_RMVAL, "CheckQLogicSettings - ENTER\n");
    /*
     * Loop through the active controllers and check their
     * FE and BE QLogic settings.
     */
    for (index1 = 0; index1 < Qm_GetNumControllersAllowed(); index1++)
    {
        configIndex = Qm_GetActiveCntlMap(index1);

        if (configIndex != ACM_NODE_UNDEFINED)
        {
            controllerSN = CCM_ControllerSN(configIndex);

            /*
             * If the entry in the controller configuration map is not
             * valid, skip it.
             */
            if (controllerSN == 0)
            {
                continue;
            }

            /* Get the BE loop statistics information */
            pStatsLoops = StatsLoops(controllerSN, PROCESS_BE);

            /*
             * Validate the BE settings, this function handles the case
             * where the information could not be retrieved and a NULL
             * pointer is passed for pStatsLoops.
             */
            if (!ValidateQLogicSettings(controllerSN, PROCESS_BE, pStatsLoops))
            {
                bError = true;
            }

            /* Free the BE loop statistics information. */
            Free(pStatsLoops);

            /* Get the FE loop statistics information */
            pStatsLoops = StatsLoops(controllerSN, PROCESS_FE);

            /*
             * Validate the FE settings, this function handles the case
             * where the information could not be retrieved and a NULL
             * pointer is passed for pStatsLoops.
             */
            if (!ValidateQLogicSettings(controllerSN, PROCESS_FE, pStatsLoops))
            {
                bError = true;
            }

            /* Free the FE loop statistics information. */
            Free(pStatsLoops);
        }
    }

    dprintf(DPRINTF_RMVAL,"CheckQLogicSettings - EXIT (J2 Jumpers=%s)\n",
            bError ? "ERROR" : "GOOD");
}


/**
******************************************************************************
**
**  @brief      Given the loop statistics information, validate that
**              the QLogic settings are correct.
**
**  @param      UINT32 controllerSN - Controller whose cards are being validated
**  @param      UINT8 processor - FE or BE processor being validated
**  @param      PI_STATS_LOOPS_RSP* pStatsLoops - The loop statistics
**                                                information used for the
**                                                validation.
**
**  @return     true if the settings are correct, false otherwise
**
******************************************************************************
**/
static bool ValidateQLogicSettings(UINT32 controllerSN,
                                   UINT8 processor, PI_STATS_LOOPS_RSP *pStatsLoops)
{
    UINT32      iCard;
    PI_STATS_LOOP *pStatsLoop;
    UINT32      port32;
    UINT32      errorCode;
    char       *pMsg;
    bool        bNoError = true;

    /*
     * Allocate memory for a text message. Initialize another pointer
     * to this string.
     */
    pMsg = MallocWC(VAL_MSG_LENGTH);

    /*
     * If the information was successfully retrieved interrogate
     * it to find out if the QLogic settings are correct.
     */
    if (pStatsLoops)
    {
        /* Get a pointer to the start of the loop statistics information. */
        pStatsLoop = pStatsLoops->stats;

        /*
         * Loop through each of the cards available in the loop
         * statistics information and check if each and every
         * card has the J2 jumper in the correct location.
         */
        for (iCard = 0; iCard < pStatsLoops->count; ++iCard)
        {
            if (pStatsLoop->stats[0].rev.vendid == VEND_QLOGIC &&
                (pStatsLoop->stats[0].GPIOD & PORT_GPIOD_J2_JUMPER_MASK) == 0 &&
                (pStatsLoop->stats[0].rev.model & 0xFF00) != 0x2400)

            {
                /*
                 * Clear out the message buffer so it available for the
                 * next message.
                 */
                memset(pMsg, 0, VAL_MSG_LENGTH);

                /*
                 * Create the log message text for the J2 jumper setting
                 * being incorrect for this port on the given processor
                 * for this controller.
                 */
                sprintf(pMsg, "%s-CN%d port %d J2 jumper incorrect",
                        (processor == PROCESS_BE ? "STORAGE" : "HOST"),
                        GetCNIDFromSN(controllerSN), pStatsLoop->port);

                /*
                 * port32 is created to put the 4 bit port value in the
                 * proper spot for the error message. Yes its ugly but
                 * it works. If you prefer, ignore the numeric warning
                 * and just read the text message (if you can).
                 */
                port32 = (((UINT32)pStatsLoop->port) << 24) & 0x0F000000;

                /*
                 * The error code is a combination of the QLogic base
                 * code, the processor (HOST or STORAGE) and the port
                 * that is incorrect.
                 */
                errorCode = VAL_QLOGIC_BASE + processor + port32;

                /*
                 * Log the validation error with the correct error code,
                 * controller and message text.
                 */
                LogValidationError(errorCode, controllerSN, pMsg);

                /*
                 * Change the flag to indicate there were errors in
                 * the validation.
                 */
                bNoError = false;
            }

            /*
             * Move to the next card's information in the response
             * packet. This movement must basically skip over the
             * information contained in one PI_STATS_LOOP structure,
             * however, that structure is dynamically sized so it
             * is not so simple. Thankfully we put the length of
             * that structure (the true dynamic length) in itself
             * to allow just this type of pointer movement. So,
             * bottom line is that to move the pointer just add on
             * the length in the current loop information and add
             * two since we did not include the LENGTH variable in
             * the length calculation.
             */
            pStatsLoop = (PI_STATS_LOOP *)((UINT8 *)pStatsLoop + pStatsLoop->length + 2);
        }
    }
    else
    {
        /*
         * Clear out the message buffer so it available for the
         * next message.
         */
        memset(pMsg, 0, VAL_MSG_LENGTH);

        /*
         * Create the log message text for the unsuccessful attempt
         * to retrieve the loop statistic information.
         */
        sprintf(pMsg,
                "%s-unable to get QLogic info on CN%d",
                (processor == PROCESS_BE ? "STORAGE" : "HOST"),
                GetCNIDFromSN(controllerSN));
        /*
         * Log the validation error with the correct error code,
         * controller and message text.
         */
        LogValidationError(VAL_QLOGIC_NO_INFO + processor, controllerSN, pMsg);

        /*
         * Change the flag to indicate there were errors in
         * the validation.
         */
        bNoError = false;
    }

    /* Free the message buffer since we allocated it. */
    Free(pMsg);

    /* Return the error status. */
    return bNoError;
}


/**
******************************************************************************
**
**  @brief      Check paths to Fibre STorage
**
**              Check and report on the paths available to the fibre
**              storage bays on the active controllers - inter and intra
**              controller. Validate both bays and individual PDisks.
**
**  @param      none
**
**  @return     1 if a resetqlogic is needed due to master pdisks != slave pdisks.
**
******************************************************************************
**/
static int CheckPathsToFibreStorage(void)
{
    XIO_PACKET  reqPkt = { NULL, NULL };
    XIO_PACKET  rspPkt = { NULL, NULL };
    INT32       rc = PI_GOOD;
    UINT32      slaveSN = 0;
    INT16       slaveIndex = 0;
    int         master_bays;
    int         slave_bays;
    int         master_disks;
    int         slave_disks;
    int         flag_resetqlogic_needed = 0;

    dprintf(DPRINTF_RMVAL, "%s - ENTER\n", __func__);

    /*
     * Allocate memory for the request (header and data) and the
     * response header. Memory for the response data is allocated
     * in TunnelRequest().
     */
    reqPkt.pHeader = MallocWC(sizeof(*reqPkt.pHeader));
    reqPkt.pPacket = MallocWC(sizeof(PI_PROC_BE_DEVICE_PATH_REQ));
    rspPkt.pHeader = MallocWC(sizeof(*rspPkt.pHeader));
    rspPkt.pPacket = NULL;
    reqPkt.pHeader->packetVersion = 1;
    rspPkt.pHeader->packetVersion = 1;

    /* Fill in the request header */
    reqPkt.pHeader->commandCode = PI_PROC_BE_DEVICE_PATH_CMD;
    reqPkt.pHeader->length = sizeof(PI_PROC_BE_DEVICE_PATH_REQ);

    /* Fill in the request parms. */
    ((PI_PROC_BE_DEVICE_PATH_REQ *)(reqPkt.pPacket))->type = PATH_PHYSICAL_DISK;
    ((PI_PROC_BE_DEVICE_PATH_REQ *)(reqPkt.pPacket))->format = FORMAT_PID_PATH_ARRAY;

    /*
     * Issue the command through the top-level command handler.
     * Validate the ports and generate a port bit map to be used later.
     */
    rc = PortServerCommandHandler(&reqPkt, &rspPkt);

    if (rc == GOOD)
    {
        dprintf(DPRINTF_RMVAL, "  Validate local PDisks\n");

        rc = ValidateStorage(&rspPkt, VAL_STORAGE_LOCAL, GetMyControllerSN(), &master_bays, &master_disks);
    }

    /*
     * Validate the storage on the slave if we have a valid slave serial
     * number and if the local (master) validation was successful.
     * Loop through all the slave controllers.
     */

    while (rc == GOOD && (slaveSN = GetNextRemoteControllerSN(&slaveIndex)))
    {
        UINT8   retries = 2;                /* Ethernet, Fiber(1), Disk Quorum(2) */

        /*
         * Memory for the response data for the previous request was allocated
         * by PortServerCommandHandler(). Free it before making tunnel request.
         */

        do
        {
            if (rc != PI_TIMEOUT)
            {
                Free(rspPkt.pPacket);
            }
            else
            {
                rspPkt.pPacket = NULL;
            }
            rc = TunnelRequest(slaveSN, &reqPkt, &rspPkt);
        } while (rc != GOOD && (retries--) > 0);

        dprintf(DPRINTF_RMVAL, "  Validate remote PDisks\n");

        (void)ValidateStorage(&rspPkt, VAL_STORAGE_REMOTE, slaveSN, &slave_bays, &slave_disks);

        if (master_disks != slave_disks || master_bays != slave_bays)
        {
            flag_resetqlogic_needed = 1;
            dprintf(DPRINTF_RMVAL, "  Reset Qlogic needed, master bays/disks (%d/%d) != slave bays/disks (%d/%d)\n",
                    master_bays, master_disks, slave_bays, slave_disks);
        }
    }

    /*
     * Free all memory. Keep the response packet memory around if request timed
     * out. Tunnel requests will not return a timeout error.
     */
    Free(reqPkt.pHeader);
    Free(reqPkt.pPacket);
    Free(rspPkt.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPkt.pPacket);
    }

    dprintf(DPRINTF_RMVAL, "%s - EXIT\n", __func__);
    return(flag_resetqlogic_needed);
}

/**
******************************************************************************
**
**  @brief      Check Back End Loops
**
**              Check that port 0 on controller 0 is connected to port 0
**              on controller1, port 1 to port 1, etc.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CheckBELoops(void)
{
    X1_BE_LOOP_INFO_RSP *pBELoopInfoRsp;
    UINT32      controllerSN;
    UINT32      port32 = 0;
    UINT16      remotePortId = 0;
    UINT8       port;
    UINT8       ctrl;
    UINT8       loopValCount = 0;
    char       *pMsg;

    dprintf(DPRINTF_RMVAL, "CheckBELoops - ENTER\n");

    /* Allocate memory for a text message. Get our serial number. */
    pMsg = MallocWC(VAL_MSG_LENGTH);
    controllerSN = GetMyControllerSN();

    /*
     * CacheMgr has a function to determine BE loop connectivity.
     * Get the data from cache manager and check for proper connectivity.
     */
    pBELoopInfoRsp = MallocWC(sizeof(*pBELoopInfoRsp));
    GetBELoopInfo(pBELoopInfoRsp);

    /* Loop through all possible BE ports. */
    for (port = 0; port < MAX_BE_PORTS; port++)
    {
        /*
         * If the local port is present, check that the remote controllers
         * are all connected on the same port.
         */
        if (pBELoopInfoRsp->portInfo[port].localPortState == PORT_NOTINSTALLED)
        {
            continue;
        }

        loopValCount++;         /* Count how many BE loops we validate */

        /* Loop through the remote port ID array. */
        for (ctrl = 0; ctrl < HACK_MAX_CONTROLLERS - 1; ctrl++)
        {
            /* Get the remote port ID for this controller from the array. */
            remotePortId = pBELoopInfoRsp->portInfo[port].remotePortId[ctrl];

            /*
             * Generate a warning if the remote port exists but does
             * not match the local port.
             */
            if (remotePortId != NO_REMOTE_PORT &&
                RemotePortNumber(remotePortId) != port)
            {
                sprintf(pMsg, "STORAGE-CN%d port %d to CN%d port %d",
                        GetCNIDFromSN(controllerSN), port,
                        RemotePortCNID(remotePortId),
                        RemotePortNumber(remotePortId));

                /*
                 * port32 is created to put the 4 bit port value in the
                 * proper spot for the error message. Yes its ugly but
                 * it works. If you prefer, ignore the numeric warning
                 * and just read the text message (if you can).
                 */
                port32 = (((UINT32)port) << 24) & 0x0F000000;

                LogValidationError(VAL_BACK_END_LOOP + port32 + remotePortId,
                                   controllerSN, pMsg);
            }
        }
    }

    /* Free memory */
    Free(pMsg);
    Free(pBELoopInfoRsp);

    dprintf(DPRINTF_RMVAL, "CheckBELoops - validated %d loops - EXIT\n", loopValCount);
}

#if defined(MODEL_3000) || defined(MODEL_7400)
/**
******************************************************************************
**
**  @brief      Verify that Disk Bay Shelf IDs are unique on a given loop.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CheckShelfId(void)
{
    X1_BAY_INFO_RSP *pBayInfoRsp;
    UINT8      *pBayMap;
    UINT64      shelfIdMap[DEV_PATH_MAX];   /* NOTE MAX_BAYS_PORT=8 (Allow to 64) */
    UINT32      controllerSN;
    UINT8       bayId;
    UINT8       i;
    UINT8       j;
    UINT8       port;
    UINT8       loop;
    UINT8       bay;
    UINT8       bayCount;
    char       *pMsg;

    dprintf(DPRINTF_RMVAL, "CheckShelfId - ENTER\n");

    for (i = 0 ; i < DEV_PATH_MAX; i++)
    {
        shelfIdMap[i] = 0;
    }

    /* Allocate memory for a text message. Get our serial number. */
    pMsg = MallocWC(VAL_MSG_LENGTH);
    controllerSN = GetMyControllerSN();

    /* Allocate memory for Disk Bay map and Disk Bay info. Get the Disk Bay map. */
    pBayMap = MallocWC(CACHE_SIZE_DISK_BAY_MAP);
    pBayInfoRsp = MallocWC(sizeof(*pBayInfoRsp));

    GetCachedBayMap(pBayMap);

    /* Walk the map, derive the bay ID and get the info for this bay. */
    bayCount = 0;
    for (loop = 0; loop < 2; loop++)
    {
        /* Look at each bit in this byte in the map. */
        for (bay = 0; bay < MAX_BAYS_PORT; bay++)
        {
            bayId = loop * MAX_BAYS_PORT + bay;
            j = bayId & 7;                      /* Get bit in byte. */
            i = bayId / 8;                      /* Get byte. */

            /* If a disk bay exists, determine its bayId. */
            if (!BIT_TEST(pBayMap[i], j))
            {
                continue;
            }

            bayCount++;

            /* Get the Bay Info for this disk bay. */
            BayGetInfo(bayId, pBayInfoRsp);

            /* Loop through the path array for each bay. */
            for (port = 0; port < DEV_PATH_MAX; port++)
            {
                /* See if a connection exists on this path */
                if (pBayInfoRsp->path[port] == DEV_PATH_NO_CONN)
                {
                    continue;   /* If no connection, continue */
                }

                /* If the shelf ID of this bay is already in use flag an error. */
                if (BIT_TEST(shelfIdMap[port], pBayInfoRsp->shelfId))
                {
                    sprintf(pMsg, "BAY-%c ID #%d already used in loop %d-%d",
                            bayName[bayId % MAX_DISK_BAYS], pBayInfoRsp->shelfId, loop*2, loop*2+1);
                    LogValidationError(VAL_SHELF_ID +
                            (((UINT16)pBayInfoRsp->shelfId) << 8) + bayId,
                            controllerSN, pMsg);
                }
                else
                {
                    /*
                     * Set a bit in the shelf ID map for this port
                     * indicating that the shelf ID is in use.
                     */
                    BIT_SET(shelfIdMap[port], pBayInfoRsp->shelfId);
                }
            }
        }
    }

    /* Free memory */
    Free(pMsg);
    Free(pBayMap);
    Free(pBayInfoRsp);

    dprintf(DPRINTF_RMVAL, "CheckShelfId - validated %d bays - EXIT\n", bayCount);
}
#endif /* MODEL_3000 || MODEL_7400 */

/**
******************************************************************************
**
**  @brief      Check paths to servers
**
**              Check and report on the paths available to the servers
**              on the active controllers - inter and intra controller.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CheckPathsToServers(void)
{
#ifdef RM_ENABLE_SERVER_VALIDATION

    XIO_PACKET  reqPkt = { NULL, NULL };
    XIO_PACKET  rspPktMaster = { NULL, NULL };
    XIO_PACKET  rspPktSlave = { NULL, NULL };
    PI_WWN_TO_TARGET_MAP_RSP *pDevPathListMaster = NULL;
    WWN_TO_TARGET_MAP *pDevPathMaster = NULL;
    PI_WWN_TO_TARGET_MAP_RSP *pDevPathListSlave = NULL;
    WWN_TO_TARGET_MAP *pDevPathSlave = NULL;
    MRGETSINFO_RSP serverInfoOut;
    char       *pMsg;
    char       *pStr;
    char        debugStr[16];
    UINT32      temp32;
    INT32       rc = PI_GOOD;
    UINT32      slaveSN = 0;
    INT16       slaveIndex = 0;
    UINT16      i;
    UINT16      j;
    UINT8       missingPortMap;

    dprintf(DPRINTF_RMVAL, "CheckPathsToServers - ENTER\n");

    /*
     * Allocate memory for a text message. Initialize another pointer
     * to this string.
     */
    pMsg = MallocWC(VAL_MSG_LENGTH);
    pStr = pMsg;

    /*
     * Allocate memory for the request (header and data) and the
     * response header. Memory for the response data is allocated
     * at a lower level.
     */
    reqPkt.pHeader = MallocWC(sizeof(*reqPkt.pHeader));
    rspPktMaster.pHeader = MallocWC(sizeof(*rspPktMaster.pHeader));
    rspPktSlave.pHeader = MallocWC(sizeof(*rspPktSlave.pHeader));
    reqPkt.pHeader->packetVersion = 1;
    rspPktMaster.pHeader->packetVersion = 1;
    rspPktSlave.pHeader->packetVersion = 1;

    /* Fill in the request header */
    reqPkt.pHeader->commandCode = PI_SERVER_WWN_TO_TARGET_MAP_CMD;
    reqPkt.pHeader->length = 0;

    /*
     * Issue the command through the top-level command handler.
     * This is for the local (master) controller.
     */
    rc = PortServerCommandHandler(&reqPkt, &rspPktMaster);

    if (rc == PI_GOOD)
    {
        /*
         * Start by checking that each local server can been seen on
         * a valid port pair.
         * Get the device path list from the response packet.
         */
        pDevPathListMaster = (PI_WWN_TO_TARGET_MAP_RSP *)(rspPktMaster.pPacket);

        /*
         * If the device path list is available loop through all devices
         * to insure that each has a valid path.
         */
        if (pDevPathListMaster != NULL)
        {
            /* Get a pointer to the start of the device path list */
            pDevPathMaster = (WWN_TO_TARGET_MAP *)(pDevPathListMaster->map);

            dprintf(DPRINTF_RMVAL, "  Validate %d local servers\n",
                    pDevPathListMaster->count);

            /* Loop through each server attached to the master controller. */
            for (i = 0; i < pDevPathListMaster->count; i++)
            {
                /* Convert port bit map to a string and NULL terminate it. */
                BitMapToString(pDevPathMaster[i].targetBitmap, debugStr, TRUE);

                dprintf(DPRINTF_RMVAL, "    local server -  wwn=%08X %08X on port(s) %s\n",
                        bswap_32((UINT32)(pDevPathMaster[i].wwn)),
                        bswap_32((UINT32)((pDevPathMaster[i].wwn) >> 32)),
                        debugStr);

                /*
                 * Check that the server is visible on a valid port pair.
                 * Could change this code to report port(s) missing but
                 * looks like the message would then exceed 40 characters.
                 */
                if (!((pDevPathMaster[i].targetBitmap == VAL_PORTS_0_1) ||
                      (pDevPathMaster[i].targetBitmap == VAL_PORTS_2_3) ||
                      (pDevPathMaster[i].targetBitmap == VAL_PORTS_ALL)))
                {
                    pStr += sprintf(pStr, "HOST-%08X%08X missing on port ",
                            bswap_32((UINT32)(pDevPathMaster[i].wwn)),
                            bswap_32((UINT32)((pDevPathMaster[i].wwn) >> 32)));

                    missingPortMap = FindMissingPorts(pDevPathMaster[i].targetBitmap);

                    pStr += BitMapToString(missingPortMap, pStr, TRUE);

                    /*
                     * Get the portBitmap in the right spot for the
                     * warning code and generate a log message.
                     */
                    temp32 = (((UINT32)pDevPathMaster[i].targetBitmap) << 24) & 0x0F000000;

                    LogValidationError(VAL_SERVERS_LOCAL + temp32, GetMyControllerSN(), pMsg);

                    /*
                     * Clear the message buffer and reset the string pointer
                     * before building the next message.
                     */
                    memset(pMsg, 0, VAL_MSG_LENGTH);
                    pStr = pMsg;

                    /*
                     * Indicate an error condition that will cause slave
                     * server validation to be skipped.
                     */
                    rc = PI_ERROR;
                }
            }
        }
    }

    /*
     * Validate the servers on the slaves if we have a valid slave serial
     * number and if the local (master) validation was successful.
     * Loop through all the slave controllers.
     */
    while (rc == PI_GOOD && (slaveSN = GetNextRemoteControllerSN(&slaveIndex)) != 0)
    {
        UINT8   retries = 2;      /* Ethernet, Fiber(1), Disk Quorum(2) */

        /*
         * A request packet was allocated above for the local packet request.
         * This packet will be reused to tunnel a request to a slave
         * controller. Since the response data from the master controller
         * must be compared to the slave data a different response packet is
         * used for the tunnel request to the slave.
         */
        do
        {
            if (rc != PI_TIMEOUT)
            {
                Free(rspPktSlave.pPacket);
            }
            else
            {
                rspPktSlave.pPacket = NULL;
            }
            rc = TunnelRequest(slaveSN, &reqPkt, &rspPktSlave);
        } while (rc != GOOD && (retries--) > 0);

        if (rc == GOOD)
        {
            /* Get the device path list from the response packet. */
            pDevPathListSlave = (PI_WWN_TO_TARGET_MAP_RSP *)(rspPktSlave.pPacket);

            /*
             * If the device path list is available loop through all devices
             * and compare the path map to the map from the master.
             */
            if (pDevPathListSlave != NULL)
            {
                /* Get a pointer to the start of the device path list */
                pDevPathSlave = (WWN_TO_TARGET_MAP *)(pDevPathListSlave->map);

                dprintf(DPRINTF_RMVAL, "  Validate %d remote servers\n",
                        pDevPathListSlave->count);

                /* Print list of servers on the slave for debug purposes. */
                for (i = 0; i < pDevPathListSlave->count; i++)
                {
                    /* Convert port bit map to a string and NULL terminate it. */
                    BitMapToString(pDevPathSlave[i].targetBitmap, debugStr, TRUE);

                    dprintf(DPRINTF_RMVAL, "    remote server - wwn=%08X %08X on port(s) %s\n",
                            bswap_32((UINT32)(pDevPathSlave[i].wwn)),
                            bswap_32((UINT32)((pDevPathSlave[i].wwn) >> 32)),
                            debugStr);
                }

                /*
                 * First check that the number of servers visible on the slave
                 * is the same as on the master.
                 */
                if (pDevPathListSlave->count != pDevPathListMaster->count)
                {
                    /*
                     * Log an error indicating that the master and slave see
                     * a different number of servers. This is all that
                     * goes in the log.
                     */
                    pStr += sprintf(pStr, "Server path count: CN%d=%d, "
                                    "CN%d=%d",
                                    GetCNIDFromSN(GetMyControllerSN()),
                                    pDevPathListMaster->count,
                                    GetCNIDFromSN(slaveSN), pDevPathListSlave->count);

                    LogValidationError(VAL_SERVERS_REMOTE, slaveSN, pMsg);

                    /*
                     * Clear the message buffer and reset the string pointer
                     * before building the next message.
                     */
                    memset(pMsg, 0x00, VAL_MSG_LENGTH);
                    pStr = pMsg;

                    /*
                     * Go to the start of the while to check the next
                     * controller.
                     */
                    continue;
                }

                /* Loop through each server attached to the slave controller. */
                for (i = 0; i < pDevPathListSlave->count; i++)
                {
                    /*
                     * Loop through the master records looking for data
                     * on this server.
                     */
                    for (j = 0; j < pDevPathListMaster->count; j++)
                    {
                        /*
                         * If the server is found but the port bit maps
                         * do not match log a warning.
                         */
                        if ((pDevPathSlave[i].wwn == pDevPathMaster[j].wwn) &&
                            (pDevPathSlave[i].targetBitmap != pDevPathMaster[j].targetBitmap))
                        {
                            if (GetServerInfoFromWwn(serverInfoOut.wwn, &serverInfoOut) == GOOD)
                            {
                                pStr += sprintf(pStr, "HOST-");

                                if (serverInfoOut.name[0] != 0)
                                {
                                    memcpy(pStr, serverInfoOut.name, MAX_SERVER_NAME_LEN);
                                    pStr[MAX_SERVER_NAME_LEN] = '\0';
                                    pStr += strlen(pStr);
                                }
                                else
                                {
                                    pStr += sprintf(pStr, "NONAME%d", serverInfoOut.sid);
                                }
                            }
                            else
                            {
                                pStr += sprintf(pStr, "HOST%08X%08X",
                                                bswap_32((UINT32)(pDevPathSlave[i].wwn)),
                                                bswap_32((UINT32)((pDevPathSlave[i].wwn) >> 32)));
                            }

                            /* HOST-SERVER123456 CN0=0,1,2/CN1=0,1,2,3 */


                            pStr += sprintf(pStr, " CN%d=",
                                            GetCNIDFromSN(GetSerialNumber(CONTROLLER_SN)));

                            pStr += BitMapToString(pDevPathMaster[j].targetBitmap, pStr, FALSE);

                            pStr += sprintf(pStr, "/CN%d=", GetCNIDFromSN(slaveSN));

                            pStr += BitMapToString(pDevPathSlave[i].targetBitmap, pStr, TRUE);

                            /*
                             * Get the portBitmap in the right spot for the
                             * warning code and generate a log message.
                             */
                            temp32 = (((UINT32)pDevPathSlave[i].targetBitmap) << 24) & 0x0F000000;

                            LogValidationError(VAL_SERVERS_REMOTE + temp32, slaveSN, pMsg);

                            /*
                             * Clear the message buffer and reset the string
                             * pointer before building the next message.
                             */
                            memset(pMsg, 0x00, VAL_MSG_LENGTH);
                            pStr = pMsg;

                            /*
                             * Break out and process the next server
                             * in the slave list.
                             */
                            break;
                        }
                    }
                }
            }
        }

        /*
         * Free the response packet each time through the loop.
         * A new one is allocated by TunnelRequest().
         */
        if (rc != PI_TIMEOUT)
        {
            Free(rspPktSlave.pPacket);
            rspPktSlave.pPacket = NULL;
        }
    }

    /*
     * Free all memory. Keep the response packet memory around if the
     * request timed out. Tunnel requests will not return a timeout error.
     */
    Free(reqPkt.pHeader);
    Free(rspPktMaster.pHeader);
    Free(rspPktSlave.pHeader);
    Free(pMsg);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPktMaster.pPacket);
    }

    dprintf(DPRINTF_RMVAL, "CheckPathsToServers - EXIT\n");

#endif  /* RM_ENABLE_SERVER_VALIDATION */
}


/**
******************************************************************************
**
**  @brief  Check the connectivity between controllers - Ethernet and Fibre
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CheckConnectivity(void)
{
    UINT32      slaveSN = 0;
    INT32       rc = GOOD;
    INT16       slaveIndex = 0;
    INT16       mapIndex = LOCAL_CTRL_INDEX;
    char       *pMsg;

    dprintf(DPRINTF_RMVAL, "CheckConnectivity - ENTER\n");

    /* Allocate memory for a text message. */
    pMsg = MallocWC(VAL_MSG_LENGTH);

    mapIndex = LOCAL_CTRL_INDEX + 1;

    while ((slaveSN = GetNextRemoteControllerSN(&slaveIndex)) != 0)
    {
        rc = IpcSendPing(slaveSN, SENDPACKET_ETHERNET);

        if (rc != GOOD)
        {
            sprintf(pMsg, "Ethernet ping failed CN%d to CN%d",
                    GetCNIDFromSN(GetMyControllerSN()), GetCNIDFromSN(slaveSN));

            LogValidationError(VAL_COMM_IPC_ETH_FAIL, slaveSN, pMsg);
        }
        else
        {
            rc = IpcSendPing(slaveSN, SENDPACKET_FIBRE);
            if (rc != GOOD)
            {
                sprintf(pMsg, "Front-end Path ping failed CN%d to CN%d",
                        GetCNIDFromSN(GetMyControllerSN()), GetCNIDFromSN(slaveSN));
                LogValidationError(VAL_COMM_IPC_FIBRE_FAIL, slaveSN, pMsg);
            }
        }
        mapIndex++;
    }

    Free(pMsg);

    dprintf(DPRINTF_RMVAL, "CheckConnectivity - EXIT\n");
}


/**
******************************************************************************
**
**  @brief      Check firmware system release
**
**              Verify that all controllers are running the same system
**              release level. This test applies to controller firmware ONLY.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CheckFWSystemRelease(void)
{
    XIO_PACKET  reqPkt = { NULL, NULL };
    XIO_PACKET  rspPktMaster = { NULL, NULL };
    XIO_PACKET  rspPktSlave = { NULL, NULL };
    PI_FW_SYS_REL_LEVEL_RSP *pSysRelMaster = NULL;
    PI_FW_SYS_REL_LEVEL_RSP *pSysRelSlave = NULL;
    char       *pMsg;
    INT32       rc = PI_GOOD;
    UINT32      slaveSN = 0;
    INT16       slaveIndex = 0;

    dprintf(DPRINTF_RMVAL, "CheckFWSystemRelease - ENTER\n");

    /*
     * Allocate memory for a text message. Initialize another pointer
     * to this string.
     */
    pMsg = MallocWC(VAL_MSG_LENGTH);

    /*
     * Allocate memory for the request (header and data) and the
     * response header. Memory for the response data is allocated
     * at a lower level.
     */
    reqPkt.pHeader = MallocWC(sizeof(*reqPkt.pHeader));
    rspPktMaster.pHeader = MallocWC(sizeof(*rspPktMaster.pHeader));
    rspPktSlave.pHeader = MallocWC(sizeof(*rspPktSlave.pHeader));
    reqPkt.pHeader->packetVersion = 1;
    rspPktMaster.pHeader->packetVersion = 1;
    rspPktSlave.pHeader->packetVersion = 1;

    /* Fill in the request header */
    reqPkt.pHeader->commandCode = PI_ADMIN_FW_SYS_REL_LEVEL_CMD;
    reqPkt.pHeader->length = 0;

    /*
     * Issue the command through the top-level command handler.
     * This is for the local (master) controller.
     */
    rc = PortServerCommandHandler(&reqPkt, &rspPktMaster);

    if (rc == PI_GOOD)
    {
        /* Get a pointer to the response data. */
        pSysRelMaster = (PI_FW_SYS_REL_LEVEL_RSP *)(rspPktMaster.pPacket);

        /* Print a debug message. */
        dprintf(DPRINTF_RMVAL, "  Local system release level: 0x%08X (%s)\n",
                pSysRelMaster->systemRelease, pSysRelMaster->tag);
    }
    else
    {
        dprintf(DPRINTF_RMVAL, "  Local system release level: NOT AVAILABLE\n");
    }

    /*
     * Validate the System Release level on the slaves if we have a valid
     * slave serial number and if the local (master) validation was successful.
     * Loop through all the slave controllers.
     */
    while (rc == PI_GOOD && (slaveSN = GetNextRemoteControllerSN(&slaveIndex)) != 0)
    {
        UINT8   retries = 2;            /* Ethernet, Fiber(1), Disk Quorum(2) */

        /*
         * A request packet was allocated above for the local packet request.
         * This packet will be reused to tunnel a request to a slave
         * controller. Since the response data from the master controller
         * must be compared to the slave data a different response packet is
         * used for the tunnel request to the slave.
         */
        do
        {
            if (rc != PI_TIMEOUT)
            {
                Free(rspPktSlave.pPacket);
            }
            else
            {
                rspPktSlave.pPacket = NULL;
            }
            rc = TunnelRequest(slaveSN, &reqPkt, &rspPktSlave);
        } while (rc != GOOD && (retries--) > 0);

        if (rc == GOOD)
        {
            /* Get the system release level from the response packet. */
            pSysRelSlave = (PI_FW_SYS_REL_LEVEL_RSP *)(rspPktSlave.pPacket);

            dprintf(DPRINTF_RMVAL, "  Remote system release level: 0x%08X (%s)\n",
                    pSysRelSlave->systemRelease, pSysRelSlave->tag);

            /*
             * Compare the levels between the master and slave. Log an
             * error if they are not the same.
             */
            if (pSysRelMaster->systemRelease != pSysRelSlave->systemRelease)
            {
                sprintf(pMsg, "Sys rel %s-CN%d %s-CN%d",
                        pSysRelMaster->tag,
                        GetCNIDFromSN(GetMyControllerSN()),
                        pSysRelSlave->tag, GetCNIDFromSN(slaveSN));

                LogValidationError(VAL_SYSTEM_RELEASE, slaveSN, pMsg);
            }
        }

        /*
         * Free the response packet each time through the loop.
         * A new one is allocated by TunnelRequest().
         */
        if (rc != PI_TIMEOUT)
        {
            Free(rspPktSlave.pPacket);
            rspPktSlave.pPacket = NULL;
        }
    }

    /*
     * Free all memory. Keep the response packet memory around if the
     * request timed out. Tunnel requests will not return a timeout error.
     */
    Free(reqPkt.pHeader);
    Free(rspPktMaster.pHeader);
    Free(rspPktSlave.pHeader);
    Free(pMsg);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPktMaster.pPacket);
    }

    dprintf(DPRINTF_RMVAL, "CheckFWSystemRelease - EXIT\n");
}


/**
******************************************************************************
**
**  @brief  Create and log a validation error
**
**  @param  errorCode - Code defined in this file (not currently used)
**  @param  controllerSN - SN of controler where error occurred
**  @param  pText - Text error message to be logged
**
**  @return     none
**
******************************************************************************
**/
static void LogValidationError(UINT32 errorCode, UINT32 controllerSN, char *pText)
{
    LOG_MRP     logValError;    /* Log event struct for validation error */

    /*
     * Set up log event data for the Validation event. Server events
     * are treated as warnings, the rest as errors.
     */
    if ((errorCode & VAL_TYPE_MASK) == VAL_SERVERS_LOCAL ||
        (errorCode & VAL_TYPE_MASK) == VAL_SERVERS_REMOTE)
    {
        logValError.mleEvent = LOG_Warning(LOG_VALIDATION);
    }
    else
    {
        logValError.mleEvent = LOG_Error(LOG_VALIDATION);
    }

    logValError.mleLength = sizeof(LOG_VALIDATION_MSG_PKT);

    ((LOG_VALIDATION_MSG_PKT *)(logValError.mleData))->code = errorCode |
        (((UINT8)controllerSN) << 16);

    memcpy(((LOG_VALIDATION_MSG_PKT *)(logValError.mleData))->text,
           pText, VAL_MSG_LENGTH);

    AsyncEventHandler(NULL, &logValError);  /* Create the log entry */

    gValidationError = true;    /* Indicate validation error */
}


/**
******************************************************************************
**
**  @brief  Get the next remote controller serial number from the config map
**
**  @param  pIndex - Pointer to starting index value for the next SN
**
**  @return remoteControllerSN  Or 0 if no valid SN is found
**
******************************************************************************
**/
UINT32 GetNextRemoteControllerSN(INT16 *pIndex)
{
    UINT32      configIndex;
    UINT32      controllerSN = 0;
    INT16       i;

    /*
     * Loop through the controllers in the active controller map and send
     * the tunnel event.
     */
    for (i = *pIndex; i < ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()); i++)
    {
        controllerSN = 0;
        configIndex = Qm_GetActiveCntlMap(i);

        /*
         * If the configIndex == ACM_NODE_UNDEFINED then there is no
         * controller at this position and since the active controller
         * map is packed there are no more controller to look at.
         */
        if (configIndex == ACM_NODE_UNDEFINED)
        {
            break;
        }

        controllerSN = cntlConfigMap.cntlConfigInfo[configIndex].controllerSN;

        /*
         * If the controller serial number is zero it is not a valid
         * controller. There is something wrong with the controller
         * configuration map and we do not want to try to do anything but
         * continue to the next entry in the map.
         *
         * Also skip our serial number.
         */
        if (controllerSN == 0 || controllerSN == GetMyControllerSN())
        {
            controllerSN = 0;   /* Current controllerSN is invalid */
            continue;
        }

        /*
         * This entry indicates a valid slave controller. Set the
         * index value to the next item in the list. The caller uses
         * this as the starting point to locate the next slave.
         */
        *pIndex = ++i;

        break;
    }

    return controllerSN;
}


/**
******************************************************************************
**
**  @brief      Validate ports
**
**              Create a bit map from the port list contained in the
**              input XIO_PACKET. Validate the ports based on current
**              rules.
**
**  @param      type            Type code used for error reporting
**  @param      controllerSN    Serial number of controller being validated
**  @param      pPortBitMap     Pointer used to return the port bit map.
**
**  @return     portStatus  VAL_PORTS_ALL_GOOD
**                          VAL_PORTS_FE_FAILED
**                          VAL_PORTS_BE_FAILED
**
**  @attention  THIS FUNCTION ASSUMES A MAXIMUM OF 8 PORTS
**
******************************************************************************
**/
static UINT8 ValidatePorts(UINT32 type, UINT32 controllerSN, UINT8 *pPortBitMap)
{
    PI_PORT_LIST_RSP *pPortList;
    UINT32      processor = 0;
    UINT8       portStatus = VAL_PORTS_ALL_GOOD;
    UINT8       i;
    UINT8       missingPortMap;
    char       *pMsg;
    char       *pStr;

    /*
     * Allocate memory for a text message. Initialize another pointer
     * to this string.
     */
    pMsg = MallocWC(VAL_MSG_LENGTH);
    pStr = pMsg;

    *pPortBitMap = 0;       /* Initialize the port bit map */

    /* Determine the processor on which to get the port list. */
    if (type == LOCAL_FE_PROC || type == REMOTE_FE_PROC)
    {
        processor = PROCESS_FE;
    }
    else
    {
        processor = PROCESS_BE;
    }

    /* Get the port list for the processor on the controller. */
    pPortList = PortList(controllerSN, processor, PORTS_GOOD);

    /*
     * If the port list is available verify that at least one port
     * pair exists.
     */
    if (pPortList)
    {
        /*
         * Generate a bit map for the available ports, than check the
         * appropriate values for the port pairs.
         */
        for (i = 0; i < pPortList->count; i++)
        {
            BIT_SET(*pPortBitMap, pPortList->list[i] & 7);
        }

#ifdef IGNORE_SINGLE_PATH
        if (processor == PROCESS_BE)
        {
            /*
             * If Path count is < 2 and we are not
             * missing all paths, set GOOD status.
             */
            missingPortMap = FindMissingPorts(*pPortBitMap);
            if (missingPortMap != 0)
            {
                Free(pPortList);
                Free(pMsg);
                return portStatus;
            }
        }
#endif  /* IGNORE_SINGLE_PATH */
        /*
         * Ports must be present in pairs and only in pairs - that is, no
         * odd cards are allowed. With 4 ports max the following
         * combinations are allowed: 0 & 1 OR 2 & 3 OR 0 & 1 & 2 & 3.
         * Any other configuration is an error.
         * For "Low Cost Cluster" systems, a single FE card in slot 0
         * is allowed.
         */
        if (!(*pPortBitMap == VAL_PORTS_0_1 ||
              *pPortBitMap == VAL_PORTS_2_3 ||
              *pPortBitMap == VAL_PORTS_ALL ||
              (*pPortBitMap == VAL_PORTS_0 && processor == PROCESS_FE)))
        {
            /*
             * Handle the special case of only 1 FE card installed in
             * the wrong place.
             */
            if (processor == PROCESS_FE && pPortList->count == 1)
            {
                sprintf(pStr, "HOST single port must be 0");
                LogValidationError(VAL_HW_NO_PORT_PAIR + type, controllerSN, pMsg);
            }
            else
            {
                /*
                 * If the test above failed then we must not have a valid port
                 * pair. Rather than reporting what is installed we will try
                 * to report what is missing.
                 */
                missingPortMap = FindMissingPorts(*pPortBitMap);

                /* Generate the message string and create the log message. */
                pStr += sprintf(pStr, "%s port(s) ",
                                (type < LOCAL_FE_PROC) ? "STORAGE" : "HOST");

                pStr += BitMapToString(missingPortMap, pStr, FALSE);

                sprintf(pStr, " missing on CN%d", GetCNIDFromSN(controllerSN));

                LogValidationError(VAL_HW_NO_PORT_PAIR + type, controllerSN, pMsg);
            }

            /* Set the appropriate bit to indicate a validation error. */
            if (processor == PROCESS_FE)
            {
                portStatus = VAL_PORTS_FE_FAILED;
            }
            else
            {
                portStatus = VAL_PORTS_BE_FAILED;
            }
        }
    }
    else
    {
        sprintf(pMsg, "%s-unable to get port list on CN%d",
                (type < LOCAL_FE_PROC) ? "STORAGE" : "HOST", GetCNIDFromSN(controllerSN));

        LogValidationError(VAL_HW_NO_PORT_LIST + type, controllerSN, pMsg);

        /* Set the appropriate bit to indicate a validation error. */
        if (processor == PROCESS_FE)
        {
            portStatus = VAL_PORTS_FE_FAILED;
        }
        else
        {
            portStatus = VAL_PORTS_BE_FAILED;
        }
    }

    Free(pPortList);
    Free(pMsg);

    return portStatus;
}

/**
******************************************************************************
**
**  @brief      Validate storage
**
**              Compare the bit map for the back end ports to the
**              path bit map for each PDisk
**
**  @param      pXIORsp         Pointer to a XIO_PACKET response packet from
**                              a Get BE Device Paths request.
**  @param      type            Type code used for error reporting.
**  @param      controllerSN    Serial number of controller being validated
**
**  @return     GOOD                If any pdisk in the bay has a good path.
**              PATH_COUNT_LT_2     Less than 2 paths to all PDisks
**              PATH_COUNT_GT_2     Greater than 2 paths to all PDisks
**              PATHS_NOT_UNIQUE    Paths to PDisks not unique
**
******************************************************************************
**/
static INT32 ValidateStorage(XIO_PACKET *pXIORsp, UINT32 type, UINT32 controllerSN, int *seen_bays, int *seen_disks)
{
    PI_PROC_BE_DEVICE_PATH_RSP *pDevPathList;
    MRGETBEDEVPATHS_RSP_ARRAY *pDevPath = NULL;
    MRGETPINFO_RSP *pPdiskInfo = NULL;
    UINT16      i;
    INT32       rc = GOOD;
    UINT8       missingPathMap;
    char        pdiskName[17];
    char       *pMsg;
    char       *pStr;
    UINT8       bitPath = 0;

    /*
     * Allocate memory for a text message and for PDisk Info. PDisk Info
     * is needed to find the WWN associated with the PDisk id.
     */
    pMsg = MallocWC(VAL_MSG_LENGTH);
    pPdiskInfo = MallocWC(sizeof(*pPdiskInfo));

    pStr = pMsg;    /* Set pointer to start of message string area */

    /* Get the device path list from the response packet. */
    pDevPathList = (PI_PROC_BE_DEVICE_PATH_RSP *)(pXIORsp->pPacket);

    /*
     * If the device path list is available loop through all devices
     * to insure that each has the correct path.
     */
    if (!pDevPathList)
    {
        sprintf(pMsg, "Unable to get PDisk path map on CN%d",
                GetCNIDFromSN(controllerSN));

        LogValidationError(type, controllerSN, pMsg);
        goto out;
    }

    rc = ValidateStorageBays(pXIORsp, type, controllerSN, seen_bays);

    /* Get a pointer to the start of the device path list */
    pDevPath = (MRGETBEDEVPATHS_RSP_ARRAY *)(pDevPathList->list);

    for (i = 0; i < pDevPathList->ndevs; i++)
    {
        /*
         * Check the path to the PDisk. If it is not good log the
         * appropriate error mesasage.
         */
        rc = ValidateStoragePath(&pDevPath[i], &bitPath);
#ifdef IGNORE_SINGLE_PATH
        /*
         * If Path count is < 2 and we are not
         * missing all paths, set GOOD status.
         */
        missingPathMap = FindMissingPorts(bitPath);
        if (rc == PATH_COUNT_LT_2 && missingPathMap != 0)
        {
            rc = GOOD;
        }
#endif  /* IGNORE_SINGLE_PATH */
        if (rc == GOOD)     /* If good, no more to do for this one */
        {
            continue;
        }

        /*
         * Use the pid to find the wwn of the PDisk that is
         * needed to determine the name. If the pid is not
         * found, wwn=0 and the default name "UNKNOWN" is returned
         * by WWNToShortNameString().
         */
        GetPDiskInfoFromPid(pDevPath[i].pid, pPdiskInfo);

        WWNToShortNameString(pPdiskInfo->pdd.wwn, &pdiskName[0], 0);

        /* Build the error based on the error type. */
        if (rc == PATH_COUNT_LT_2)
        {
            /*
             * Build up the message string. The bit map path is
             * converted to a comma separated list of path numbers.
             * The path reported is what is missing, not what is
             * connected.
             */
            pStr += sprintf(pStr, "Physical Disk-%s path ", pdiskName);

            missingPathMap = FindMissingPorts(bitPath);

            pStr += BitMapToString(missingPathMap, pStr, FALSE);

            pStr += sprintf(pStr, " missing (CN%d)", GetCNIDFromSN(controllerSN));
        }
        else if (rc == PATH_COUNT_GT_2)
        {
            pStr += sprintf(pStr, "Physical Disk-%s too many paths (CN%d)",
                            pdiskName, GetCNIDFromSN(controllerSN));
        }
        else if (rc == PATHS_NOT_UNIQUE)
        {
            pStr += sprintf(pStr, "Physical Disk-%s paths not unique (CN%d)",
                            pdiskName, GetCNIDFromSN(controllerSN));
        }
        else if (rc == PATHS_PDISK_UNKNOWN_WWN)
        {
            pStr += sprintf(pStr, "Physical Disk-%s bad OUI in WWN (CN%d)",
                            pdiskName, GetCNIDFromSN(controllerSN));
        }

        /* Log the Error. */
        LogValidationError(type + (UINT16)(pDevPath[i].pid), controllerSN, pMsg);

        /*
         * Clear the message buffer and reset the string pointer
         * before building the next message.
         */
        memset(pMsg, 0x00, VAL_MSG_LENGTH);
        pStr = pMsg;
    }

    dprintf(DPRINTF_RMVAL, "    %s - processed %d PDisks\n",
            __func__, pDevPathList->ndevs);
    *seen_disks =  pDevPathList->ndevs;

out:
    /* Free memory before returning. */
    Free(pMsg);
    Free(pPdiskInfo);

    return rc;
}


/**
******************************************************************************
**
**  @brief      Validate storage bays
**
**              Checks the Disk Bays to determine if all the pdisks
**              in a bay have bad paths.
**
**  @param      pXIORsp         Pointer to a XIO_PACKET response packet from
**                              a Get BE Device Paths request.
**  @param      type            Type code used for error reporting.
**  @param      controllerSN    Serial number of controller being validated
**
**  @return     GOOD                If any pdisk in the bay has a good path.
**              PATH_COUNT_LT_2     Less than 2 paths to all PDisks
**              PATH_COUNT_GT_2     Greater than 2 paths to all PDisks
**              PATHS_NOT_UNIQUE    Paths to PDisks not unique
**
******************************************************************************
**/
static INT32 ValidateStorageBays(XIO_PACKET *pXIORsp, UINT32 type, UINT32 controllerSN, int *seen_bays)
{
    MRGETEINFO_RSP *pDiskBayInfo;
    UINT16      i;
    INT32       rc = GOOD;
    UINT8       bitPath = 0;
    UINT8       missingPathMap;
    char        pBayName[17];
    char       *pMsg;
    char       *pStr;

    *seen_bays = 0;                 /* Count of good bays. */
    /*
     * Allocate memory for a text message and for PDisk Info. PDisk Info
     * is needed to find the WWN associated wit the PDisk id.
     */
    pMsg = MallocWC(VAL_MSG_LENGTH);
    pDiskBayInfo = MallocWC(sizeof(*pDiskBayInfo));

    pStr = pMsg;    /* Set pointer to start of message string area */

    /* Loop through and look for disk bays to check for. */
    for (i = 0; i < MAX_DISK_BAYS; i++)
    {
        /*
         * If we get a good disk bay, Validate the bay to see if all
         * the paths in the particular bay are bad.
         */
        if (GetDiskBayInfoFromBid(i, pDiskBayInfo) != GOOD)
        {
            continue;
        }

        *seen_bays = *seen_bays + 1;
//        dprintf(DPRINTF_RMVAL, "    %s - found bay %d\n", __func__, i);

        /*
         * If all the drives in the bay are bad, log the message and remove
         * the drives from the list.
         */
        rc = ValidateStorageBay(pXIORsp, i, &bitPath);

#ifdef IGNORE_SINGLE_PATH
        /* If Path count is < 2 and we are not missing all paths, set GOOD status. */
        missingPathMap = FindMissingPorts(bitPath);
        if (rc == PATH_COUNT_LT_2 && missingPathMap != 0)
        {
            rc = GOOD;
        }
#endif  /* IGNORE_SINGLE_PATH */
        /* Create string version of bay WWN */
        WWNToShortNameString(pDiskBayInfo->pdd.wwn, &pBayName[0], 0);

        if (rc != GOOD)
        {
            /* Build the error based on the error type. */
            if (rc == PATH_COUNT_LT_2)
            {
                /*
                 * Build up the message string. The bit map path is
                 * converted to a comma separated list of path numbers.
                 * The path reported is what is missing, not what is
                 * connected.
                 */
                pStr += sprintf(pStr, BAY "-%s path ", pBayName);

                missingPathMap = FindMissingPorts(bitPath);

                pStr += BitMapToString(missingPathMap, pStr, FALSE);

                pStr += sprintf(pStr, " missing (CN%d)", GetCNIDFromSN(controllerSN));
            }
            else if (rc == PATH_COUNT_GT_2)
            {
                pStr += sprintf(pStr, BAY "-%s too many paths (CN%d)",
                                pBayName, GetCNIDFromSN(controllerSN));
            }
            else if (rc == PATHS_NOT_UNIQUE)
            {
                pStr += sprintf(pStr, BAY "-%s paths not unique (CN%d)",
                                pBayName, GetCNIDFromSN(controllerSN));
            }
            else if (rc == PATHS_PDISK_UNKNOWN_WWN)
            {
                pStr += sprintf(pStr, BAY "-%s bad OUI in WWN (CN%d)",
                                pBayName, GetCNIDFromSN(controllerSN));
            }

            /* Log the Error. */
            LogValidationError(type + (UINT16)(pDiskBayInfo->pdd.pid),
                               controllerSN, pMsg);

            /*
             * Clear the message buffer and reset the string pointer
             * before building the next message.
             */
            memset(pMsg, 0, VAL_MSG_LENGTH);
            pStr = pMsg;

            /* Remove the bay from the path list. */
            ValidateStorageRemoveBay(pXIORsp, i);
        }
        else
        {
            /*
             * If ValidateStoragePath() returns GOOD then we have 2 valid
             * paths. Make sure they are also a valid path pair.
             */
            if (!(bitPath == VAL_PORTS_0_1 || bitPath == VAL_PORTS_2_3))
            {
                /* Generate the message string and create the log message. */
                pStr += sprintf(pStr, BAY "-%s invalid port pair ", pBayName);

                pStr += BitMapToString(bitPath, pStr, FALSE);

                pStr += sprintf(pStr, " (CN%d)", GetCNIDFromSN(controllerSN));

                LogValidationError(VAL_HW_NO_PORT_PAIR + type, controllerSN, pMsg);

                /*
                 * Clear the message buffer and reset the string pointer
                 * before building the next message.
                 */
                memset(pMsg, 0, VAL_MSG_LENGTH);
                pStr = pMsg;

                /* Remove the bay from the path list. */
                ValidateStorageRemoveBay(pXIORsp, i);
            }
            else
            {
                dprintf(DPRINTF_RMVAL, "    %s - bay %d is OK\n", __func__, i);
            }
        }
    }

    /* Free memory before returning. */
    Free(pMsg);
    Free(pDiskBayInfo);

    return (rc);
}

/**
******************************************************************************
**
**  @brief  Validate paths to a storage bay
**
**          A bay is defined to have a valid path if at least 1 PDisk
**          in the bay has a valid path.
**
**  @param  pXIORsp - Pointer to a XIO_PACKET response packet from
**                          a Get BE Device Paths request.
**  @param  bayID - BayID to check against
**  @param  pBitPath - Returns the bit path for the bay
**
**  @return GOOD                If any pdisk in the bay has a good path.
**          PATH_COUNT_LT_2     Less than 2 paths to all PDisks
**          PATH_COUNT_GT_2     Greater than 2 paths to all PDisks
**          PATHS_NOT_UNIQUE    Paths to PDisks not unique
**
******************************************************************************
**/
static UINT8 ValidateStorageBay(XIO_PACKET *pXIORsp, UINT16 bayID, UINT8 *pBitPath)
{
    MRGETBEDEVPATHS_RSP *pDevPathList;
    MRGETBEDEVPATHS_RSP_ARRAY *pDevPath;
    MRGETPINFO_RSP *pPdiskInfo;
    UINT16      i;
    UINT8       rc = GOOD;
    UINT8       goodBitPath = 0;        /* Default to no path */
    bool        oneGoodDrive = false;

    *pBitPath = 0;      /* Default the paths to none */

    /* Get the device path list from the response packet. */
    pDevPathList = (MRGETBEDEVPATHS_RSP *)(pXIORsp->pPacket);

    /*
     * If the device path list is available loop through all devices
     * to insure that each has the correct path.
     */
    if (!pDevPathList || pDevPathList->ndevs <= 0)
    {
        return GOOD;
    }

    /*
     * Allocate memory for PDisk Info. PDisk Info
     * is needed to find the the ses of the drive.
     */
    pPdiskInfo = MallocWC(sizeof(*pPdiskInfo));

    /* Get a pointer to the start of the device path list */
    pDevPath = (MRGETBEDEVPATHS_RSP_ARRAY *)(pDevPathList->list);

    /*
     * Loop through the path list and look for drives
     * that are in bayID.
     */
    for (i = 0; i < pDevPathList->ndevs; i++)
    {
        /* Get the pdisk information from the cache. */
        GetPDiskInfoFromPid(pDevPath[i].pid, pPdiskInfo);

#if 0
        /*
         * If we have an invalid ses id, break out.
         * We could not prove that every pdisk in
         * the bay was bad, so set the return
         * accordingly.
         */
        if (pPdiskInfo->pdd.ses >= MAX_DISK_BAYS)
        {
            rc = GOOD;
            break;
        }
#endif  /* 0 */

        /* If the ses id is the one we are looking for, check the paths. */
        if (pPdiskInfo->pdd.ses == bayID)
        {
            rc = ValidateStoragePath(&pDevPath[i], pBitPath);

            /*
             * If the path for this PDisk is good, set the flag to
             * indicate that at least one PDisk in the bay has
             * a valid path
             */
            if (rc == GOOD)
            {
                oneGoodDrive = true;
                goodBitPath = *pBitPath;
            }
        }
    }

    /* Free memory before returning. */
    Free(pPdiskInfo);

    /*
     * If at least 1 good PDisk path was found return GOOD. Otherwise
     * return the error code for the last PDisk.
     */
    if (oneGoodDrive)
    {
        rc = GOOD;
        *pBitPath = goodBitPath;    /* Indicate paths for the good drive */
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief  Validate the paths to a device
**
**  @param  pDevPath - Pointer to the path struct for a device.
**  @param  pBitPath - Bit map of paths for the device
**
**  @return GOOD                If any pdisk in the bay has a good path
**          PATH_COUNT_LT_2     Less than 2 paths to all PDisks
**          PATH_COUNT_GT_2     Greater than 2 paths to all PDisks
**          PATHS_NOT_UNIQUE    Paths to PDisks not unique
**
******************************************************************************
**/
static UINT8 ValidateStoragePath(MRGETBEDEVPATHS_RSP_ARRAY *pDevPath, UINT8 *pBitPath)
{
    UINT8   pathAValue = DEV_PATH_NO_CONN;
    UINT8   pathBValue = DEV_PATH_NO_CONN;
    UINT8   validPathCount = 0;
    UINT8   i;

    *pBitPath = 0;              /* Start with no paths */

    /* If there are not 2 paths, return map and specific return code */
    if (pDevPath->pathCount != 2)
    {
        /* Build a path bit map based on the actual paths present */
        for (i = 0; i < DEV_PATH_MAX; i++)
        {
            if (pDevPath->path[i] < 0x0F)
            {
                /* Valid path found. Create a bit map version of the path. */
                BIT_SET(*pBitPath, i);
            }
        }

        /* Set the return code based on the actual paths present */
        if (pDevPath->pathCount < 2)
        {
            return PATH_COUNT_LT_2;
        }
        return PATH_COUNT_GT_2;
    }

    /*
     * There are 2 paths, so check that they are unique.
     *
     * Loop through the path list and get the valid path(s).
     * path. A valid path has a value < 0x0F. A value of
     * 0x0F indicates a device with an unknown WWN format.
     *
     * Note that the value in the array element (pDevPath->path[i])
     * is used to determine if we have unique paths. The index is the path.
     */
    for (i = 0; i < DEV_PATH_MAX; i++)
    {
        if (pDevPath->path[i] < 0x0F)   /* If the path is valid */
        {
            /*
             * If this is the first valid path found, save the value in
             * pathAValue.
             */
            if (validPathCount == 0)
            {
                pathAValue = pDevPath->path[i];
            }
            else
            {
                /* Must be the second valid path. It goes in pathBValue */
                pathBValue = pDevPath->path[i];
            }

            /*
             * Create a bit map version of the path and increment
             * the path count.
             */
            BIT_SET(*pBitPath, i);
            ++validPathCount;
        }
        else if (pDevPath->path[i] == 0x0F)
        {
            /*
             * A path value of 0x0F indicates a device is present but
             * the OID of the WWN is not one we recognize.
             * In this case break out and report this condition.
             */
            pathAValue = pDevPath->path[i];
            break;
        }
    }

    /* Now look at the results and set the returns accordingly. */
    if (pathAValue == 0x0F)
    {
        return PATHS_PDISK_UNKNOWN_WWN;
    }

    if (validPathCount < 2)
    {
        return PATH_COUNT_LT_2;
    }

    if (validPathCount == 2 && pathAValue == pathBValue)
    {
        return PATHS_NOT_UNIQUE;
    }

    return GOOD;
}


/**
******************************************************************************
**
**  @brief  Removes the bay from the bitmap.
**
**  @param  pXIORsp - Pointer to a XIO_PACKET response packet from
**                              a Get BE Device Paths request.
**  @param  bayID - BayID to check against
**
**  @return none
**
******************************************************************************
**/
static void ValidateStorageRemoveBay(XIO_PACKET *pXIORsp, UINT16 bayID)
{
    PI_PROC_BE_DEVICE_PATH_RSP *pDevPathList;
    MRGETBEDEVPATHS_RSP_ARRAY *pDevPath;
    MRGETPINFO_RSP *pPdiskInfo;
    UINT16      index1 = 0;
    UINT16      index2;

    dprintf(DPRINTF_RMVAL, "    %s - Removing bay %d\n", __func__, bayID);

    /* Get the device path list from the response packet. */
    pDevPathList = (PI_PROC_BE_DEVICE_PATH_RSP *)(pXIORsp->pPacket);

    /*
     * If the device path list is available loop through all devices
     * to insure that each has the correct path.
     */
    if (!pDevPathList || pDevPathList->ndevs <= 0)
    {
        return;     /* If no devices, just leave */
    }

    /*
     * Allocate memory for PDisk Info. PDisk Info
     * is needed to find the the ses of the drive.
     */
    pPdiskInfo = MallocWC(sizeof(*pPdiskInfo));

    /* Get a pointer to the start of the device path list */
    pDevPath = (MRGETBEDEVPATHS_RSP_ARRAY *)(pDevPathList->list);

    /*
     * Loop through the path list and remove entries
     * that are associated with bayID.
     */
    while (index1 < pDevPathList->ndevs)
    {
        /* Get the pdisk information from the cache. */
        GetPDiskInfoFromPid(pDevPath[index1].pid, pPdiskInfo);

        /*
         * If this pdisk is in bay bayID, remove it from the path list.
         * Decrement the number of devices.
         */
        if (pPdiskInfo->pdd.ses != bayID)
        {
            ++index1;   /* Not in BayID, increment and continue */
            continue;
        }

        --pDevPathList->ndevs;

        for (index2 = index1; index2 < pDevPathList->ndevs; index2++)
        {
            pDevPath[index2] = pDevPath[index2 + 1];
        }
    }

    Free(pPdiskInfo);   /* Free memory before returning */
}


/**
******************************************************************************
**
**  @brief  Get list of BE or FE ports on a given controller
**
**  @param  controllerSN - Controller to get the port list from
**  @param  processor - BE or FE list
**  @param  type - Option on port list command
**
**  @return Pointer to buffer containing port list response
**
**  @attention  The pointer returned from this function must be freed
**              the caller.
**
******************************************************************************
**/
PI_PORT_LIST_RSP *PortList(UINT32 controllerSN, UINT32 processor, UINT16 type)
{
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };
    INT32       rc = GOOD;
    PI_PORT_LIST_RSP *pResponse = NULL;
    UINT32            pi_command_code;

    /* Fill in the request header */
    if (processor == PROCESS_BE)
    {
        pi_command_code = PI_PROC_BE_PORT_LIST_CMD;
    }
    else if (processor == PROCESS_FE)
    {
        pi_command_code = PI_PROC_FE_PORT_LIST_CMD;
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "PortList - Invalid processor type (0x%x).\n",
                processor);

        /* Invalid processor type was passed as a parameter. */
        return NULL;
    }

    /*
     * Allocate memory for the request (header and data) and the
     * response header. Memory for the response data is allocated
     * in TunnelRequest().
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_PORT_LIST_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pHeader->packetVersion = 1;
    reqPacket.pHeader->packetVersion = 1;
    reqPacket.pHeader->commandCode = pi_command_code;
    reqPacket.pHeader->length = sizeof(PI_PORT_LIST_REQ);

    /* Fill in the request parms. */
    ((PI_PORT_LIST_REQ *)(reqPacket.pPacket))->type = type;

    /*
     * If the port list request is for this controller make the
     * request to the port server directly. If it is for one
     * of the slave controllers then tunnel the request to
     * that controller.
     */
    if (controllerSN == GetMyControllerSN())
    {
        /* Issue the command through the top-level command handler. */
        rc = PortServerCommandHandler(&reqPacket, &rspPacket);
    }
    else
    {
        UINT8   retries = 2;            /* Ethernet, Fiber(1), Disk Quorum(2) */

        do
        {
            if (rc != PI_TIMEOUT)
            {
                Free(rspPacket.pPacket);
            }
            else
            {
                rspPacket.pPacket = NULL;
            }
            rc = TunnelRequest(controllerSN, &reqPacket, &rspPacket);
        } while (rc != GOOD && (retries--) > 0);
    }

    if (rc == PI_GOOD)
    {
        pResponse = (PI_PORT_LIST_RSP *)rspPacket.pPacket;
        rspPacket.pPacket = NULL;
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return (pResponse);
}


/**
******************************************************************************
**
**  @brief  Get BE or FE loop statistics information on a given controller
**
**  @param  controllerSN - Controller to get the information from
**  @param  processor - BE or FE
**
**  @return Pointer to buffer containing loop statistics response
**
**  @attention  The pointer returned from this function must be freed by
**              the caller.
**
******************************************************************************
**/
PI_STATS_LOOPS_RSP *StatsLoops(UINT32 controllerSN, UINT32 processor)
{
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    INT32       rc = GOOD;
    PI_STATS_LOOPS_RSP *pResponse = NULL;
    UINT32            pi_command_code;

    /* Fill in the request header */
    if (processor == PROCESS_BE)
    {
        pi_command_code = PI_STATS_BACK_END_LOOP_CMD;
    }
    else if (processor == PROCESS_FE)
    {
        pi_command_code = PI_STATS_FRONT_END_LOOP_CMD;
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "PortList - Invalid processor type (0x%x).\n",
                processor);

        /* Invalid processor type was passed as a parameter. */
        return NULL;
    }

    /*
     * Allocate memory for the request (header and data) and the
     * response header. Memory for the response data is allocated
     * in TunnelRequest().
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pHeader->commandCode = pi_command_code;
    reqPacket.pHeader->packetVersion = 1;
    reqPacket.pHeader->length = sizeof(PI_STATS_LOOPS_REQ);
    reqPacket.pPacket = MallocWC(sizeof(PI_STATS_LOOPS_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    rspPacket.pHeader->packetVersion = 1;

    /* Fill in the request parms. */
    ((PI_STATS_LOOPS_REQ *)(reqPacket.pPacket))->option = 0;

    /*
     * If the port list request is for this controller make the
     * request to the port server directly. If it is for one
     * of the slave controllers then tunnel the request to
     * that controller.
     */
    if (controllerSN == GetMyControllerSN())
    {
        /* Issue the command through the top-level command handler. */
        rc = PortServerCommandHandler(&reqPacket, &rspPacket);
    }
    else
    {
        UINT8   retries = 2;        /* Ethernet, Fiber(1), Disk Quorum(2) */

        do
        {
            if (rc != PI_TIMEOUT)
            {
                Free(rspPacket.pPacket);
            }
            else
            {
                rspPacket.pPacket = NULL;
            }
            rc = TunnelRequest(controllerSN, &reqPacket, &rspPacket);
        } while (rc != GOOD && (retries--) > 0);
    }

    if (rc == PI_GOOD)
    {
        pResponse = (PI_STATS_LOOPS_RSP *)rspPacket.pPacket;
        rspPacket.pPacket = NULL;
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return (pResponse);
}


/**
******************************************************************************
**
**  @brief      Convert the input bit map into a string representation.
**
**  @param      bitMap  Bit map to be converted
**  @param      pStr    Pointer to string buffer where output string
**                      will be copied. Allocated by caller.
**  @param      flag    TRUE = add a NULL terminator at the end of the string.
**
**  @return     String length
**
******************************************************************************
**/
static UINT8 BitMapToString(UINT8 bitMap, char *pStr, UINT8 flag)
{
    char        bitMapStr[16];
    UINT8       bitIndex = 0;
    UINT8       strIndex = 0;

    /*
     * If the bitMap is empty (i.e. 0) indicate this in the string.
     * Changed string from NONE to ALL since we are reporting what
     * is missing, not what is installed.
     */
    if (bitMap == 0)
    {
        strcpy(bitMapStr, "ALL");
        strIndex = 3;
    }
    else
    {
        /*
         * Process the bit map and create a comma separate string
         * representation of the values. This function will only
         * handle 8 bit maps.
         */
        for (bitIndex = 0; bitIndex < 8; bitIndex++)
        {
            /*
             * Check each bit position. If the bit is set append
             * to the string.
             */
            if ((0x01 << bitIndex) & bitMap)
            {
                /*
                 * The first entry in the string is a special case.
                 * Remaining entries are preceeded by a ,
                 */
                if (strIndex == 0)
                {
                    strIndex += sprintf(&(bitMapStr[strIndex]), "%d", bitIndex);
                }
                else
                {
                    strIndex += sprintf(&(bitMapStr[strIndex]), ",%d", bitIndex);
                }
            }
        }
    }

    /* NULL terminate the string if the flag is set. */
    if (flag == TRUE)
    {
        bitMapStr[strIndex++] = '\0';
    }

    /* Copy the string representation of the bit map into the caller's buffer. */
    memcpy(pStr, bitMapStr, strIndex);

    return (strIndex);
}

/**
******************************************************************************
**
**  @brief      Check firmware update in progress
**
**              See if the controller is in the process of updating
**              its firmware. Firmware update causes movement of
**              system resources which most likely will result in
**              validation errors.
**
**  @param      none
**
**  @return     TRUE if a firmware update is in progress
**
******************************************************************************
**/
static bool CheckFWUpdateInProgress(void)
{
    bool        updateInProgress = FALSE;

    /*
     * Check this controller's failure data to make sure a firmware update
     * is not in progress.
     */
    if ((GetControllerFailureState() == FD_STATE_FIRMWARE_UPDATE_INACTIVE) ||
        (GetControllerFailureState() == FD_STATE_FIRMWARE_UPDATE_ACTIVE))
    {
        updateInProgress = TRUE;
    }

    return (updateInProgress);
}

/**
******************************************************************************
**
**  @brief      Find missing ports
**
**              Based on the map of installed ports, return a map of
**              the ports that are missing. This function assumes that if
**              one port in a port pair is present that the user intended
**              both to be installed.
**
**  @param      installedPortMap    Ports installed
**
**  @return     missingPortMap      Ports missing
**
******************************************************************************
**/
static UINT8 FindMissingPorts(UINT8 installedPortMap)
{
    UINT8       missingPortMap;

    /*
     * XOR the installed port map with a map of all ports to get a map
     * of the ports NOT installed.
     */
    missingPortMap = installedPortMap ^ VAL_PORTS_ALL;

    /*
     * If both ports in a pair are missing, assume this is what
     * the user intended. Yes this is an assumption but it is
     * the only way we can report what "should be" instead of
     * "what is".
     */
    if ((missingPortMap & VAL_PORTS_0_1) == VAL_PORTS_0_1)
    {
        /*
         * Ports 0 & 1 are both missing. Don't report either
         * as missing.
         */
        missingPortMap &= (~VAL_PORTS_0_1);
    }

    if ((missingPortMap & VAL_PORTS_2_3) == VAL_PORTS_2_3)
    {
        /*
         * Ports 2 & 3 are both missing. Don't report either
         * as missing.
         */
        missingPortMap &= (~VAL_PORTS_2_3);
    }

    return (missingPortMap);
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
