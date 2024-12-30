/* $Id: NV_Memory.c 161678 2013-09-18 19:25:16Z marshall_midden $ */
/**
******************************************************************************
**  @file       NV_Memory.c
**
**  @brief      Non-Volatile Memory support
**              (Specifically: MicroMemory MM-5425CN Card Access Routines)
**
**  Provides support for the MicroMemory card and its battery-backed memory.
**
**  Copyright 2004-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "options.h"
#include <stdint.h>
#include "L_Misc.h"
#include "ecodes.h"
#include "li_pci.h"
#include "LL_LinuxLinkLayer.h"
#include "L_Misc.h"
#include "LOG_Defs.h"
#include "misc.h"
#include "NV_Memory.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/pci.h>
#include <string.h>
#include <stdlib.h>
#include "wcache.h"
#include "xio3d.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "CT_defines.h"

/* #define STANDALONE              1 */
/* #define CHAINED_DMA_TEST_CODE   1 */
#ifdef STANDALONE
#include <sys/mman.h>
#ifdef CHAINED_DMA_TEST_CODE
#include <LKM_layout.h>
#endif /* CHAINED_DMA_TEST_CODE */
#endif /* STANDALONE */

extern void CT_LC_MM_TestTask(int);
extern void CT_LC_nv_DMAExec(int);
extern void CT_LC_nv_DMAComplete(int);
extern void CT_LC_MM_MonitorTask(int);
extern void CT_LC_NV_ScrubTask(int);
extern void LL_SendPacket(void *, UINT32, UINT32, void *, UINT32, void *, UINT32);
extern INT32 MM_Write(UINT32, UINT8 *, UINT32);
extern INT32 MM_Read(UINT32, UINT8 *, UINT32);
extern void LI_RegisterIRQ(UINT32, void *, UINT32);

#ifdef FRONTEND
extern MP_MIRROR_PARTNER_INFO gMPControllerOriginalConfig;
extern MP_MIRROR_PARTNER_INFO bfDefaults;
#else   /* FRONTEND */
extern int      g_apool_initialized;
#endif /* FRONTEND */

/*
******************************************************************************
** Private defines
******************************************************************************
*/
#define DEVFILE "/dev/xio3d0"

/*
** Compiler flags
*/
/* #define MM_TEST_DEBUG    1 */
/* #define MM_DEBUG         1 */
/* #define DMA_TEST_CODE    1 */
/* #define MEMCPY_VERSION   1 */
/* #define DEBUG_MMINFO     1 */
/* #define MM_FULL_DIAGS    1 */
/* #define MM_VERBOSE       1 */
/* #define NV_DMA_DRVR      1 */
#define DMA_CHAIN_ENABLE 1
/* #define PCI_DMA_ADDR     1 */

#ifdef MM_VERBOSE
#define get_tsc() ({ unsigned long long __scr; __asm__ __volatile__("rdtsc" : "=A" (__scr)); __scr;})
#endif /* MM_VERBOSE */

/*
** The current threshold is 7 hours = 7 * 60 * 60 = 19800.
** Since the timer is in eighth second increments this value must
** be multiplied by 8 to get the real value = 8 * 19800 = 158400
**
** For coding the "8" is actually calculated using QUANTUM, there are
** 8 QUANTUMs in a second and QUANTUM is specified in milliseconds,
** the calculation is then 1000 / QUANTUM to get the number of
** QUANTUMs in a second.
*/
#define MM_CHARGE_THRESHOLD     (7 * 60 * 60 * (1000 / QUANTUM))

/**
**  @SCRUB_CONSTANTS
**/
/**
** The time it takes to scrub the entire memory module is:
**  ((module size)/(scrub size))*(number of seconds between cycles) =
**                                      Total time to scrub the entire memory
**  (1GB/16K)*60 = 3932160 seconds ~ 1093 hours ~ 46 days
**  (1GB/32K)*30 = 983040 seconds ~ 273 hours ~ 12 days
**  (1GB/64K)*15 = 245760 seconds ~ 68 hours ~ 3 days
**  (1GB/64K)*5 = 81920 seconds ~  23 hours
**  (1GB/16K)*1 = 65536 seconds ~ 18 hours
**/
#define SCRUB_TRANSFER_LENGTH   SIZE_16K    /**< Scrub transfer size (must
                                             **  be a multiple of the Card
                                             **  Size)
                                             **/

#define SCRUB_DELAY_TIME        2000        /**< Time delay between DMA
                                             **  Transfers in milliseconds
                                             **/
/*
 * The following macro is to document required flush of pci bridge data to
 * make sure that it really makes it to the MicroMemory card.
 * There are two addresses ranges, a read from the correct address range will
 * force the flush. It does not matter which address, except perhaps if you
 * wish to use the same data you just wrote -- then there appears to be an
 * errata suggesting that you have to read it twice, otherwise the first read
 * which flushes the pci bridge will return invalid data. This works, because
 * on a volatile variable, the read must be done.
 */
#define MM_FORCE_WRITE_TO_FLUSH(a)       a


/*
******************************************************************************
** Private structures
******************************************************************************
*/
extern struct pci_devs  gPCIdevs[XIO3D_NIO_MAX];
extern INT32            pci_devs_index;
extern UINT32           gMMCFound;

#ifdef STANDALONE
#ifdef BACKEND
static unsigned long    PAGE_MASK;
#endif /* BACKEND */
static unsigned long    PAGE_SIZE;
#endif


/*
******************************************************************************
** Private variables
******************************************************************************
*/

MM_REGS                 *pMM = NULL;
MM_INFO                 *mmInfo = NULL;
static volatile UINT8   *pMMemory = NULL;
static UINT32           mem_size = 0;
NV_DMA_QUEUE            *pDMAQueue = NULL;
#ifdef FRONTEND
PCB                     *pNV_DMAExecPCB = NULL;
PCB                     *pNV_DMACompPCB = NULL;
#ifdef NV_DMA_DRVR
PCB                     *pNV_DMADrvrPCB = NULL;
volatile bool           gNVDMAExecEn = TRUE;
volatile bool           gNVDMAInjECCErr = FALSE;
#endif /* NV_DMA_DRVR */
#endif /* FRONTEND */

#ifdef STANDALONE
static UINT8            *map_shared[XIO3D_NSHM_MAX];
#endif

#if (defined(BACKEND) || defined(STANDALONE))
static struct xio3d_drvinfo xio3d_drvinfo;
static int    xiofd = -1;
#endif

/*
** The following array defines the various 64-bit test patterns used to
** validate the data lines of the memory. Certain individual patterns are
** handy in other situations, such as zeroing memory or setting some other
** constant value.
*/
#define TEST_PATTERN_BOUNDS 8
static const UINT64 testPattern[TEST_PATTERN_BOUNDS] =
{
    0xddddddddddddddddLL,0xffffffffffffffffLL,
    0xffff0000ffff0000LL,0x0000ffff0000ffffLL,
    0xaaaaaaaaaaaaaaaaLL,0x5555555555555555LL,
    0x3333333333333333LL,0x0000000000000000LL
};
static const UINT64 *zeroPattern = &testPattern[7];

#ifdef BACKEND
static const UINT64 *testPatternEnd = &testPattern[TEST_PATTERN_BOUNDS]; /* beyond end of array */

/* Forward references. */
UINT32 NV_VerifySkip(void);
UINT32 NV_VerifyTest(void);
INT32 mm_Debug(void);
/*
** The following structure defines the NV Memory Directory, which describes the
** extent of the memory segments and their owners.
*/
#define NV_DIRECTORY_BOUNDS 7
static const struct NV_DIRECTORY
{
    UINT32 start;
    UINT32 end;
    UINT32 (*vfyContents)(void);    /* Prototype:  UINT32 vfyContents(void); */
    UINT8  name[4];
} nvDirectory[NV_DIRECTORY_BOUNDS] =
    {
        {NV_ADMIN_START,    NV_ADMIN_END,   NV_VerifySkip,  "Admn"},
        {NV_P3BE_START,     NV_P3BE_END,    NV_VerifySkip,  "P3BE"},
        {NV_P4BE_START,     NV_P4BE_END,    NV_VerifySkip,  "P4BE"},
        {NV_P4FE_START,     NV_P4FE_END,    NV_VerifySkip,  "P4FE"},
        {NV_P6BE_START,     NV_P6BE_END,    NV_VerifySkip,  "P6BE"},
        {NV_P7BE_START,     NV_P7BE_END,    NV_VerifySkip,  "P7BE"},
        {NV_FREE_START,     NV_FREE_END,    NV_VerifySkip,  "Free"}
    };
/*static const struct NV_DIRECTORY *nvDirectoryEnd = &nvDirectory[NV_DIRECTORY_BOUNDS]; beyond end of array */
static const struct NV_DIRECTORY *nvDirectoryFree = &nvDirectory[NV_DIRECTORY_BOUNDS-1]; /* free/unassigned space */
static const struct NV_DIRECTORY *nvDirectoryAdmin = &nvDirectory[0];
#endif /* BACKEND */

MRMMINFO_REQ *pMMInfoReq;
MRMMINFO_RSP *pMMInfoRsp;

#ifdef FRONTEND
void*               pMMMonitorPCB = NULL;
NVRAM_BOARD_INFO    gNVBoardInfo;
#endif

#ifdef FRONTEND
void*               gpScrubPCB = NULL;
NV_DMA_RSP_PKT*     gpScrubCompletionRsp;
#endif


/*****************************************************************************
** Public function prototypes not in header files.
*****************************************************************************/
extern void WC_batHealth(UINT8 board, UINT8 state);
extern void WC_RestoreData(void);

/*****************************************************************************
** Private function prototypes
*****************************************************************************/

#ifdef BACKEND /* LSW kludge */
UINT32 NV_VerifySkip(void) /* temp - skip destructive tests on this region */
{
    return 0;
}

UINT32 NV_VerifyTest(void) /* temp - perform destructive tests on this region */
{
    return 1;
}

/*extern UINT32 M$p3chksumchk();  Actually takes a UINT32 parm... */

UINT32 nv_ScrubECC(UINT32 startAddr, UINT32 length);
#endif /* BACKEND */

void MM_Interrupt(UINT32 value UNUSED);

#ifdef FRONTEND
void MM_MonitorTask(UINT32 dummy1, UINT32 dummy2);
void MM_MonitorGatherData(NVRAM_BOARD_INFO* pInfo);
void MM_MonitorAnalyze(NVRAM_BOARD_INFO* pCurrent, NVRAM_BOARD_INFO* pNew);
void MM_MonitorLogEvent(UINT32 event);
UINT8 MM_MonitorCheckBoardReadiness(void);
#endif

#ifdef FRONTEND
void NV_ScrubTask(UINT32 dummy1, UINT32 dummy2);
void NV_ScrubCompletion(NV_DMA_RSP_PKT* pRsp);
#endif

#ifdef FRONTEND
void MM_TestTask(
    UINT32                      pPCB UNUSED,
    UINT32                      pri UNUSED,
    MRMMTEST_REQ*               pReq);
#endif

#ifdef PCI_DMA_ADDR
INT32   mm_DMA_Write(UINT64 PCI_Addr, UINT64 Local_Addr, UINT32 length, UINT8 block);
#ifdef BACKEND
INT32   mm_DMA_Read(UINT64 PCI_Addr, UINT64 Local_Addr, UINT32 length, UINT8 block);
#endif /* BACKEND */
#if (defined(STANDALONE) && defined(CHAINED_DMA_TEST_CODE))
INT32   mm_Chain_DMA(UINT64 Desc_Addr, UINT8 block);
#endif /* STANDALONE && CHAINED_DMA_TEST_CODE */

#else /* PCI_DMA_ADDR */
INT32   mm_DMA_Write(UINT32 sharedAddr, UINT64 Local_Addr, UINT32 length, UINT8 block);
#ifdef BACKEND
INT32   mm_DMA_Read(UINT32 sharedAddr, UINT64 Local_Addr, UINT32 length, UINT8 block);
#endif /* BACKEND */
#if (defined(STANDALONE) && defined(CHAINED_DMA_TEST_CODE))
INT32   mm_Chain_DMA(UINT32 sharedDescAddr, UINT8 block);
#endif /* STANDALONE && CHAINED_DMA_TEST_CODE */
#endif /* PCI_DMA_ADDR */

#ifdef FRONTEND
INT32   mm_DMA(MM_DMA_DESC *pDesc, bool block);
UINT32  bcd_to_bin(UINT32 val);
void    nv_SetupDescriptor(MM_DMA_DESC *pDesc, NV_DMA_DD *pDDList);
#endif /* FRONTEND */

#ifdef BACKEND
void nv_InitAdminRegion(void);
UINT32 nv_TestDataLines(UINT32 testAddr, UINT32 testAddrCmpl);
UINT32 nv_TestAddrLines(UINT32 startAddr, UINT32 endAddr);
UINT32 nv_TestCells(UINT64 *startAddr, UINT64 *endAddr);
void nv_Diags(void);
void nv_SendMMInfo(void);
void    nv_InitSNAdminRegion(UINT32);
INT32   MM_init(void);

/* #ifndef PERF */
void mm_DumpRegs(void);
/* #endif */ /* !PERF */
#endif /* BACKEND */

void nv_ProcessFatalEvent(UINT32 event, UINT32 data);
void mm_ClearErrorCounter(void);
UINT8 mm_FixECCError(UINT32 errorAddr);
void mm_ProcessECCErrors(void);


/*
******************************************************************************
** Code Start
******************************************************************************
*/

/*
 * But first an errata on the MicroMemory card.
 *
 * <Start quote.>
 * "Writing to LED Register and then immediately reading gives old value: If
 *  a read of the LED Register follows a write to the register by only a few
 *  clock cycles, some bits from the old value may be returned.
 *
 *  Expected resolution and workaround:  Future revisions will reduce the time
 *  interval before the new value takes effect.
 *
 *  Note that any time any value is written over the PCI bus, it may not reach
 *  the device until some value is read from that device. Good programming
 *  practice with any PCI device is to read a value after it is written to
 *  force the write to complete.
 *  In this case, the result from that read should simply be discarded. The
 *  result from a second read will always produce the correct result."
 * <End quote.>
 *
 * What this means according to folks here: If you write to a PCI address
 * range, you must do a read from the same address range to make sure that the
 * write has completed. And, as the MM has two address ranges (one for the
 * registers, and the 16MB window), you must do a read after writing to either
 * of the address ranges.
 *
 * Furthermore: Having to discard the value is a MicroMemory only problem.
 */

#ifdef STANDALONE
#ifdef BACKEND
void dumpsome(void *mem, UINT32 amount)
{
    UINT32 offset;
    UINT16 *s;

    for (offset = 0; offset < amount; offset += 16)
    {
        s = (UINT16 *)((UINT8 *)mem + offset);
        fprintf(stderr, "%04X: %04X %04X %04X %04X  %04X %04X %04X %04X\n",
                offset, *s, *(s + 1), *(s + 2), *(s + 3),
                *(s + 4), *(s + 5), *(s + 6), *(s + 7));
    }
}
#endif /* BACKEND */
#endif  /* STANDALONE */


/*
******************************************************************************
**  nv_ProcessFatalEvent
**
**  @brief  This routine processes board failure events.
**
**  A failure event is sent to the CCB to alert it of the board failure.
**  Other processes are alerted to the failure of NV memory (e.g. Write Cache).
**
**  @param  UINT32 - event
**  @param  UINT32 - data, meaning varies with event
**
**  @return None
**
******************************************************************************
*/
void nv_ProcessFatalEvent(UINT32 event, UINT32 data)
{
    bool fatal = TRUE;
    bool logMsg = TRUE;

    /* Event processing is based on the event received. */
    switch (event)
    {
        case NV_FATAL_ECC_MULTI:
            if (mmInfo->status & NV_STS_DIAG)
            {
                /* Diagnostic tests are running - ignore this error */
                fatal = FALSE;
                logMsg = FALSE;
            }
            else
            {
                /* Not running diagnostics - process this error */
                fprintf(stderr, "NV Memory Fatal Error (%d): Multi-bit ECC error.\n", event);
                mmInfo->status = NV_STS_MEM_FAIL;
            }
            break;

        case NV_FATAL_BATT:
            fprintf(stderr, "NV Memory Fatal Error (%d): Failed to charge.\n", event);
            mmInfo->status = NV_STS_BATT_FAIL;
            break;

        case NV_FATAL_NO_CARD:
            fprintf(stderr, "NV Memory Fatal Error (%d): Card not present or inaccessible.\n", event);
            mmInfo->status = NV_STS_NO_BOARD;

            /* Don't consider this error fatal at this time */
            fatal = FALSE;
            break;

        case NV_FATAL_POST:
            fprintf(stderr, "NV Memory Fatal Error (%d): POST test (%d) failed.\n", event, data);
            mmInfo->status = NV_STS_MEM_FAIL;
            break;

        case NV_FATAL_ECC_UNCORR:
            fprintf(stderr, "NV Memory Fatal Error (%d): Uncorrectable single-bit ECC error encountered at offset 0x%X.\n", event, data);
            mmInfo->status = NV_STS_MEM_FAIL;
            break;

        case NV_FATAL_ASSERT:
            fprintf(stderr, "NV Memory Fatal Error (%d): Software sanity error (%d) detected.\n", event, data);
            /* mmInfo->status is not changed here */
            break;

        case NV_FATAL_SN_MISMATCH:
            fprintf(stderr, "NV Memory Fatal Error (%d): Controller S/N preserved in memory (0x%X) does not match our S/N (0x%X)! Access will not be allowed!\n",
                    event, data, K_ficb->cSerial);
            mmInfo->status = NV_STS_SN_MISMATCH;

            /*
            ** Flash OK-to-remove LED at 3.5Hz rate.
            ** Don't consider this error fatal at this time
            */
            pMM->ledCtrlBits.okToRmv = MM_LED_FLASH_3_5HZ;
            MM_FORCE_WRITE_TO_FLUSH(pMM->ledCtrlBits);
            fatal = FALSE;
            break;

        case NV_FATAL_USER_FAILED:
            fprintf(stderr, "NV Memory Fatal Error (%d): User inititiated board failure.\n",
                    event);
            mmInfo->status = NV_STS_MEM_FAIL;
            break;

        default:
            fprintf(stderr, "UNKNOWN NV Memory Fatal event received (%d)\n", event);
    } /* end of switch */

    if (logMsg)
    {
        /* Send a log message to CCB */
        LOG_NV_MEM_EVENT_PKT    gNVLogMsg;
        gNVLogMsg.header.event = LOG_NV_MEM_EVENT;
        gNVLogMsg.data.event = event;
        /* Note: message is short, and L$send_packet copies into the MRP. */

#ifdef BACKEND
  #if defined(MODEL_4700) || defined(MODEL_7000)
          if (g_apool_initialized == TRUE || event != NV_FATAL_NO_CARD)
          {
              MSC_LogMessageStack(&gNVLogMsg, sizeof(LOG_NV_MEM_EVENT_PKT));
          }
  #else   /* 4700 || 7000 */
          MSC_LogMessageStack(&gNVLogMsg, sizeof(LOG_NV_MEM_EVENT_PKT));
  #endif /* 4700 || 7000 */
#else   /* BACKEND (i.e. FRONTEND below) */
  #if defined(MODEL_4700) || defined(MODEL_7000)
          if (event != NV_FATAL_NO_CARD)
          {
              MSC_LogMessageStack(&gNVLogMsg, sizeof(LOG_NV_MEM_EVENT_PKT));
          }
  #else   /* !FRONTEND && (4700 || 7000) */
          MSC_LogMessageStack(&gNVLogMsg, sizeof(LOG_NV_MEM_EVENT_PKT));
  #endif /* 4700 || 7000 */
#endif /* BACKEND */

    }

    if (fatal)
    {
        gMMCFound = FALSE;

        if (pMM != NULL)
        {
            /*
            ** Set Fault LED to solid ON
            ** Invalidate the card contents, in case we pass the power-on diags
            ** at the next boot, so we won't attempt to use this stale data
            */
            pMM->ledCtrl = ((pMM->ledCtrl & ~MM_LED_FLT_MASK) |
                            (MM_LED_FLT_ON << MM_LED_FLT)) &
                           ~(1 << MM_MEM_INIT);
            MM_FORCE_WRITE_TO_FLUSH(pMM->ledCtrl);
        }

        /* Disable Write Cache - dirty data will be flushed */
        /* tbd - done elsewhere upon receipt of the fatal event */

        /* Awaken DMA Completor to fail any pending requests on the DMA queue. */
        TaskReadyByState(PCB_NV_DMA_COMP_WAIT);

        /* Fail this controller, if N>1. */
        if (BIT_TEST(K_ii.status, II_CCBREQ))
        {
            if (logMsg)
            {
                /* Delay 3s to give CCB time to process the log msg. */
                TaskSleepMS(3000);
            }
            abort();
        }
    }
} /* nv_ProcessFatalEvent */


/*
******************************************************************************
**  mm_ClearErrorCounter
**
**  @brief  This subroutine properly clears the Error Counter & Error Log regs
**
**  The various error registers are cleared via the process described in the
**  Micro Memory User's Guide.
**
**  @param  None
**
**  @return None
**
******************************************************************************
*/
void mm_ClearErrorCounter(void)
{
    UINT8 errCtrlSave = pMM->errCtrl;

    /*
    ** Set the Clear bit in the Control field (clears the first/last error logs)
    ** and also clear the Error Counter in the same store operation.
    ** Then turn off the Clear bit & preserve the Int Mask & EDC mode bits.
    */
    *(volatile UINT16 *)&pMM->errCtrl = errCtrlSave | MM_EDC_CLR;
    pMM->errCtrl = errCtrlSave & ~MM_EDC_CLR;
    MM_FORCE_WRITE_TO_FLUSH(pMM->errCtrl);
} /* mm_ClearErrorCounter */


/*
******************************************************************************
**  mm_FixECCError
**
**  @brief  This subroutine attempts to fix any ECC errors at the specified site
**
**  The site is re-read to determine whether an error still exists. If an error
**  is reproduced, the data is written back to the location in an attempt to
**  correct the error. If the error persists after the rewrite, this routine
**  returns an error.
**
**  @param  UINT32 - errorAddr
**
**  @return UINT32 - GOOD = success, otherwise indicate single- or multi-bit
**                          error (per Error Status register)
**
******************************************************************************
*/
UINT8 mm_FixECCError(UINT32 errorAddr)
{
    UINT64 data;
    UINT8  errStatus = 0;
    UINT8  oldWindow;
    UINT8  window = (UINT8)(errorAddr / SIZE_16MEG);
    UINT32 windowOffset = errorAddr % SIZE_16MEG;

    /*
    ** Clear the error counter and read the site to check for errors.
    ** Ensure we don't attempt to correct a multi-bit error, which could result
    ** in corruption of the data.
    */
    mm_ClearErrorCounter();

    /* Update window register if necessary */
    if ((oldWindow = pMM->windowMap) != window)
    {
        pMM->windowMap = window;
        MM_FORCE_WRITE_TO_FLUSH(pMM->windowMap);
#ifdef MM_VERBOSE
        fprintf(stderr, "mm_FixECCError:  Setting Window Map reg to 0x%X\n", window);
#endif
    }
    data = *(volatile UINT64 *)(&pMMemory[windowOffset]);

    if ((pMM->errCnt != 0) &&
        !((errStatus = (pMM->errSts & MMCARD_ERR_STATUS)) & MULTI_BIT_ERROR))
    {
        /* Error persists; re-write the site using the returned "correct" data. */
        *(volatile UINT64 *)(&pMMemory[windowOffset]) = data;

        /* Clear the error counter & read back to check for errors. */
        mm_ClearErrorCounter();
        data = *(volatile UINT64*)(&pMMemory[windowOffset]);

        if ((errStatus = pMM->errSts & MMCARD_ERR_STATUS) == 0)
        {
            fprintf(stderr, "mm_FixECCError:  Re-wrote error site 0x%X, error was fixed\n", errorAddr);
        }
        else
        {
            fprintf(stderr, "mm_FixECCError:  Re-wrote error site 0x%X, error was NOT fixed!\n", errorAddr);
        }
    }
    else
    {
        fprintf(stderr, "mm_FixECCError:  No attempt to fix error at 0x%X, site was OK on re-read\n", errorAddr);
    }

    /* Restore the window register value if necessary */
    if (oldWindow != window)
    {
        pMM->windowMap = oldWindow;
        MM_FORCE_WRITE_TO_FLUSH(pMM->windowMap);
#ifdef MM_VERBOSE
        fprintf(stderr, "mm_FixECCError:  Setting Window Map reg back to 0x%X\n", oldWindow);
#endif
    }

    return (errStatus);
} /* mm_FixECCError */


/*
******************************************************************************
**  mm_ProcessECCErrors
**
**  @brief  This subroutine attempts to fix ECC errors which have been logged
**
**  The first & last log registers are used to identify the known error sites,
**  which are then rewritted to correct them. If the error persists after the
**  rewrite, the memory (and thus the board) is considered bad.
**
**  NOTE:  The current implementation ASSUMES a 32-bit address (offset) is
**         sufficient for the error sites, i.e. that the size of the MicroMemory
**         board is 4GB or less. The hardware allocates 5 bytes for the error
**         log address, but we're only using the lower 4 bytes.
**
**  @param  None
**
**  @return None
**
******************************************************************************
*/
void mm_ProcessECCErrors(void)
{
    UINT32 firstAddr;
    UINT32 lastAddr;
    INT8   firstSynBits;
    INT8   lastSynBits;

    /*
    ** If memory contents are valid, attempt to fix the ECC errors.
    ** Otherwise the memory isn't valid, so don't correct invalid data.
    */
    if ((pMM->ledCtrl & MM_MEM_VALID_MASK) != 0)
    {
        /*
        ** Record the sites of the first & last (latest) errors, & syndromes.
        ** NOTE:  A syndrome value of 0 indicates no errors.
        */
        firstAddr    = pMM->errFirst.addrLo;
        firstSynBits = pMM->errFirst.synBits;
        lastAddr     = pMM->errLast.addrLo;
        lastSynBits  = pMM->errLast.synBits;

        /* Attempt to fix the latest error site, if the error still exists. */
        if ((lastSynBits != 0) && (mm_FixECCError(lastAddr) != 0))
        {
            nv_ProcessFatalEvent(NV_FATAL_ECC_UNCORR, lastAddr);
        }

        /*
        ** Check whether the first error log information is unique - if only one
        ** unique error site has been found, then the first log info will be set
        ** the same as the last log info.
        */
        if (firstAddr != lastAddr)
        {
            /* Attempt to fix the first error site, if the error still exists. */
            if ((firstSynBits != 0) && (mm_FixECCError(firstAddr) != 0))
            {
                nv_ProcessFatalEvent(NV_FATAL_ECC_UNCORR, firstAddr);
            }
        }
    } /* memory is valid */
} /* mm_ProcessECCErrors */


#ifdef BACKEND
/*
******************************************************************************
**  nv_InitAdminRegion
**
**  @brief  This routine initializes the Administrative region in NV Memory.
**
**  The Administrative region contains such information as the controller S/N,
**  the NV Directory, etc. which is internal bookkeeping for the NV driver.
**
**  @param  none
**
**  @return none
**
******************************************************************************
*/
void nv_InitAdminRegion(void)
{
    NV_ADMIN    adminRecord;
    NV_MGR      mgrRecord = { 0,            /* offset   */
                              0,            /* size     */
                              0,            /* name     */
                              {0},          /* flags    */
                              {0, 0, 0}     /* rsvd[3]  */
                              };
    UINT32      i;

    /* Setup the fields in the NV Admin structure. */
    adminRecord.cSerial = K_ficb->cSerial;
    adminRecord.version = NV_ADMIN_VERSION;
    adminRecord.rsvd1   = 0;
    adminRecord.rsvd2   = 0;
    adminRecord.rsvd3   = 0;
    adminRecord.rsvd4   = 0;
    adminRecord.toc[NV_ADMIN_TOC_NV_DIR].size = sizeof(nvDirectory);
    adminRecord.toc[NV_ADMIN_TOC_NV_DIR].offset = NV_ADMIN_OFFSET_FIRST_ENTRY;

    adminRecord.toc[NV_ADMIN_TOC_NV_MGR].size = (sizeof(mgrRecord)) * NV_MGR_NUM_ENTRIES;
    adminRecord.toc[NV_ADMIN_TOC_NV_MGR].offset = adminRecord.toc[NV_ADMIN_TOC_NV_DIR].size +
                                                  adminRecord.toc[NV_ADMIN_TOC_NV_DIR].offset;

    /* Clear the remaining unallocated TOC entries, if any. */
    for (i = NV_ADMIN_TOC_FREE; i < NV_ADMIN_TOC_NUM_ENTRIES; ++i)
    {
        adminRecord.toc[i].size = 0;
        adminRecord.toc[i].offset = 0;
    }

    /* Set the offset of the first free TOC entry, if it exists (size = 0). */
    if (NV_ADMIN_TOC_FREE != NV_ADMIN_TOC_NUM_ENTRIES)
    {
        adminRecord.toc[NV_ADMIN_TOC_FREE].offset = adminRecord.toc[NV_ADMIN_TOC_NV_MGR].size +
                                                    adminRecord.toc[NV_ADMIN_TOC_NV_MGR].offset;

        /* Clear the remainder (unused) space in the Admin region */
        fprintf(stderr, "nv_InitAdminRegion:  Clearing unused area (0x%08X - 0x%08X).\n",
                adminRecord.toc[NV_ADMIN_TOC_FREE].offset, NV_ADMIN_END);
        NV_ZeroMemory(adminRecord.toc[NV_ADMIN_TOC_FREE].offset, NV_ADMIN_END);
    }

    /* Write the NV Admin structure to the Admin region. */
    MM_Write(NV_ADMIN_START, (UINT8*)&adminRecord, sizeof(adminRecord));

    /* Write the NV Directory structure copy to its offset in the Admin region. */
    MM_Write(adminRecord.toc[NV_ADMIN_TOC_NV_DIR].offset,
             (UINT8*)nvDirectory,
             adminRecord.toc[NV_ADMIN_TOC_NV_DIR].size);

    /*
    ** Write/Init the NV Manager entries to their offset in the Admin region.
    ** (Loop through each entry.)
    */
    for (i = 0; i < NV_MGR_NUM_ENTRIES; ++i)
    {
        MM_Write((UINT32)&(((NV_MGR*)(adminRecord.toc[NV_ADMIN_TOC_NV_MGR].offset))[i]),
                 (UINT8*)&mgrRecord,
                 sizeof(mgrRecord));
    }
} /* nv_InitAdminRegion */


/*
******************************************************************************
**  nv_InitSNAdminRegion
**
**  @brief  This routine initializes only the S/N member of the Administrative
**          region in NV Memory.
**
**  @param  UINT32 Controller Serial Number
**
**  @return none
**
******************************************************************************
*/
void nv_InitSNAdminRegion(UINT32 serial)
{
    NV_ADMIN    adminRecord;

    /*
    ** Read the current controller S/N value in the Admin region, and update it
    ** if the S/N is changing (or the read was unsuccessful for some reason).
    */
    if ((MM_Read(NV_ADMIN_START +
                        ((UINT8 *)&adminRecord.cSerial - (UINT8 *)&adminRecord),
                 (UINT8 *)&adminRecord.cSerial,
                 sizeof(adminRecord.cSerial)) != 0) ||
        (adminRecord.cSerial != serial))
    {
        /* Set the S/N field in the NV Admin structure. */
        adminRecord.cSerial = serial;
        MM_Write(NV_ADMIN_START +
                        ((UINT8 *)&adminRecord.cSerial - (UINT8 *)&adminRecord),
                 (UINT8*)&adminRecord.cSerial,
                 sizeof(adminRecord.cSerial));
    }
} /* nv_InitSNAdminRegion */

/*
******************************************************************************
**  nv_TestDataLines
**
**  @brief  This routine verifies the data lines of the Non-Volatile Memory.
**
**  This routine does NOT validate the address lines - that task is left to the
**  address lines test. 64-bit data patterns and their bitwise complements are
**  in turn written to the device, causing the data lines to flip. The contents
**  are then read back and verified against the expected values.
**
**  @param  UINT32 - first test address for writing patterns
**  @param  UINT32 - second test address for writing pattern complements
**
**  @return 0 if success, nonzero reason code for failure
**
******************************************************************************
*/
UINT32 nv_TestDataLines(UINT32 testAddr, UINT32 testAddrCmpl)
{
    REG64        expData;
    REG64        expDataCmpl;
    REG64        recData = {0};
    REG64        recDataCmpl = {0};
    const UINT64 *pTestPattern = testPattern;
    UINT32       rc = 0;

    fprintf(stderr, "@@@@@@@@@@@@@@@@@@@@ nv_TestDataLines(0x%08X, 0x%08X) @@@@@@@@@@@@@@@@@@@@\n",
            testAddr, testAddrCmpl);

    do
    {
        /* Load 64-bit test pattern and its complement */
        expData.dw = *pTestPattern;
        expDataCmpl.dw = ~*pTestPattern;

        /* Store 64-bit test pattern to 1st location, complement to 2nd location */
        if ((rc = MM_Write(testAddr, (UINT8 *)&expData, sizeof(expData))) != 0)
        {
            break;
        }
        if ((rc = MM_Write(testAddrCmpl, (UINT8 *)&expDataCmpl, sizeof(expDataCmpl))) != 0)
        {
            break;
        }

        /* Read back patterns from 1st and 2nd locations */
        if ((rc = MM_Read(testAddr, (UINT8 *)&recData, sizeof(recData))) != 0)
        {
            break;
        }
        if ((rc = MM_Read(testAddrCmpl, (UINT8 *)&recDataCmpl, sizeof(recDataCmpl))) != 0)
        {
            break;
        }

        /* Validate the patterns, and return an error if a mismatch is found. */
        if ((expData.dw != recData.dw) || (expDataCmpl.dw != recDataCmpl.dw))
        {
            fprintf(stderr, "nv_TestDataLines:  Expected 0x%08X %08X, received 0x%08X %08X\n",
                    expData.hi, expData.lo, recData.hi, recData.lo);
            fprintf(stderr, "                 Complement 0x%08X %08X, received 0x%08X %08X\n",
                    expDataCmpl.hi, expDataCmpl.lo, recDataCmpl.hi, recDataCmpl.lo);
            rc = 5;
            break;
        }
    } while (++pTestPattern < testPatternEnd);

#ifdef MM_TEST_DEBUG
    if (rc != 0)
    {
        fprintf(stderr, "nv_TestDataLines(0x%08X, 0x%08X) failed with return code 0x%X\n",
                testAddr, testAddrCmpl, rc);
    }
    else
    {
        fprintf(stderr, "nv_TestDataLines(0x%08X, 0x%08X) successful!\n",
                testAddr, testAddrCmpl);
    }
#endif
    return (rc);
} /* nv_TestDataLines */


#ifdef MM_FULL_DIAGS
/*
******************************************************************************
**  nv_TestAddrLines
**
**  @brief  This routine verifies the address lines of the Non-Volatile Memory.
**
**  This routine writes each 32-bit word address location with a data value
**  equal to the complement & XOR of its own address. When the specified
**  address range has been initialized, the values are verified. Next, each
**  location is written with its own address, and again verified.
**
**  WARNING:  The length of the region is assumed to be a multiple of 8 bytes.
**
**  @param  UINT32 starting address (offset in device) for test
**  @param  UINT32 ending address (offset in device) for test
**
**  @return 0 if success, nonzero reason code for failure
**
******************************************************************************
*/
UINT32 nv_TestAddrLines(UINT32 startAddr, UINT32 endAddr)
{
    REG64  data;
    REG64  data2;
    UINT32 testAddr;
    UINT32 fillAddr;
    UINT32 rc = 0;
    INT32  length = endAddr - startAddr;
    INT32  xferSize;
    INT32  chunkSize;
    UINT64 *bfr = NULL;
    INT32  bfrSize = SIZE_256K;
    INT32  i;
    INT32  testNum;

    fprintf(stderr, "@@@@@@@@@@@@@@@@@@@@ nv_TestAddrLines(0x%08X, 0x%08X) @@@@@@@@@@@@@@@@@@@@\n",
            startAddr, endAddr);

    /*
    ** Allocate temp DRAM space to contain the data pattern, 256kB max.
    ** NOTE:  We can't wait on the Malloc, since the multi-tasking kernel
    **        isn't up yet. I could check K_xpcb, but didn't.
    */
    if (length >= SIZE_256K)
    {
        bfr = s_Malloc(bfrSize, __FILE__, __LINE__);
    }
    else if (length > 16)
    {
        bfrSize = length;
        bfr = s_Malloc(bfrSize, __FILE__, __LINE__);
    }

    /* Loop through 2 test passes */
    for (testNum = 1; testNum <= 2; ++testNum)
    {
        /* Write the data via DMA if possible */
        fprintf(stderr, "nv_TestAddrLines:  Beginning address lines test %d...\n", testNum);
        testAddr = startAddr;
        xferSize = length;
        if (bfr != NULL)
        {
            while (xferSize > 0)
            {
                /*
                ** Set the specified doubleword patterns in the buffer.
                ** 1st pass:  fill each location with complement/XOR of address
                ** 2nd pass:  fill each location with address
                */
                chunkSize = (xferSize >= bfrSize) ? bfrSize : xferSize;
                for (i = 0, fillAddr = testAddr; i < chunkSize/8; ++i, fillAddr+=8)
                {
                    data.lo = (testNum == 1) ? (~fillAddr ^ 0x12345678) : fillAddr;
                    data.hi = (testNum == 1) ? (~(fillAddr+4) ^ 0x12345678) : (fillAddr+4);
                    bfr[i]  = data.dw;
                }

                if ((rc = mm_DMA_Write((UINT32)bfr, testAddr, chunkSize, TRUE)) == 0)
                {
                    xferSize -= chunkSize;
                    testAddr += chunkSize;
                }
                else
                {
                    /* The DMA failed for some reason */
                    fprintf(stderr, "  ...address lines test %d FAILED! Write DMA completed with bad status 0x%08X\n", testNum, rc);
                    return rc;
                }
            }
        } /* bfr != NULL */
        else
        {
            /*
            ** Malloc failed - do Programmed I/O writes of 8-byte chunks instead.
            ** 1st pass:  fill each location with complement/XOR of address
            ** 2nd pass:  fill each location with address
            */
            do
            {
                data.lo = (testNum == 1) ? (~testAddr ^ 0x12345678) : testAddr;
                data.hi = (testNum == 1) ? (~(testAddr+4) ^ 0x12345678) : (testAddr+4);
                if ((rc = MM_Write(testAddr, (UINT8 *)&data, sizeof(data))) != 0)
                {
                    /* The write failed for some reason */
                    fprintf(stderr, "  ...address lines test %d FAILED!  Address = 0x%08X, rc = 0x%X\n",
                            testNum, testAddr, rc);
                    return rc;
                }
            } while ((testAddr+=8) < endAddr);
        }

        /* Read back and verify the data */
        testAddr = startAddr;
        xferSize = length;
        if (bfr != NULL)
        {
            /* Read data via DMA (quicker) */
            while (xferSize > 0)
            {
                chunkSize = (xferSize >= bfrSize) ? bfrSize : xferSize;
                if ((rc = mm_DMA_Read((UINT32)bfr, testAddr, chunkSize, TRUE)) == 0)
                {
                    /*
                    ** Verify the data
                    ** 1st pass:  complement/XOR of address
                    ** 2nd pass:  address
                    */
                    for (i = 0, fillAddr = testAddr; i < chunkSize/8; ++i, fillAddr+=8)
                    {
                        data.lo = (testNum == 1) ? (~fillAddr ^ 0x12345678) : fillAddr;
                        data.hi = (testNum == 1) ? (~(fillAddr+4) ^ 0x12345678) : (fillAddr+4);
                        if (bfr[i] != data.dw)
                        {
                            fprintf(stderr, "  ...address lines test %d FAILED!\n"
                                    "  Address:  0x%08X, expected data:  0x%08X %08X, actual data 0x%08X %08X\n",
                                    testNum, fillAddr, data.lo, data.hi, ((UINT32 *)bfr)[i/2], ((UINT32 *)bfr)[i/2]);
                            return 4;
                        }
                    }
                    xferSize -= chunkSize;
                    testAddr += chunkSize;
                }
                else
                {
                    /* The DMA failed for some reason */
                    fprintf(stderr, "  ...address lines test %d FAILED!  "
                            "Read DMA completed with bad status 0x%08X\n", testNum, rc);
                    return rc;
                }
            }
        } /* bfr != NULL */
        else
        {
            /* Read data via Programmed I/O (slower) */
            do
            {
                /*
                ** Verify the data
                ** 1st pass:  complement/XOR of address
                ** 2nd pass:  address
                */
                data.lo = (testNum == 1) ? (~testAddr ^ 0x12345678) :
                                           testAddr;
                data.hi = (testNum == 1) ? (~(testAddr+4) ^ 0x12345678) :
                                           (testAddr+4);
                if ((rc = MM_Read(testAddr, (UINT8 *)&data2, sizeof(data2))) != 0)
                {
                    /* The read failed for some reason */
                    fprintf(stderr, "  ...address lines test %d FAILED!  Address = 0x%08X, rc = 0x%X\n",
                            testNum, testAddr, rc);
                    return rc;
                }
                if (data.dw != data2.dw)
                {
                    fprintf(stderr, "  ...address lines test %d FAILED!\n"
                            "  Address:  0x%08X, expected data:  0x%08X %08X, actual data:  0x%08X %08X\n",
                            testNum, testAddr, data.lo, data.hi, data2.lo, data2.hi);
                    return 5;
                }
            } while ((testAddr+=8) < endAddr);
        }
        fprintf(stderr, "  ...address lines test %d completed successfully.\n", testNum);
    } /* iterate through 2 test passes */

    /* Free the temp buffer now, if necessary. */
    if (bfr != NULL)
    {
        s_Free(bfr, bfrSize, __FILE__, __LINE__);
    }

    return (rc);
} /* nv_TestAddrLines */

/*
******************************************************************************
**  nv_TestCells
**
**  @brief  This routine verifies the individual Non-Volatile Memory cells.
**
**  This routine performs a test on every 32-bit location (cell) in the memory.
**
**  WARNING:  The length of the region is assumed to be a multiple of 8 bytes.
**
**  @param  UINT64 * - pointer to starting location of test range
**  @param  UINT64 * - pointer to ending location of test range
**
**  @return 0 if success, nonzero reason code for failure
**
******************************************************************************
*/
UINT32 nv_TestCells(UINT64 *startAddr, UINT64 *endAddr)
{
    const UINT64 *pTestPattern = testPattern;
    UINT64       expData;
    UINT64       recData;
    UINT64       *testAddr;
    UINT32       rc = 0;
    UINT32       srcAddr;
    INT32        length = (UINT32)endAddr - (UINT32)startAddr;
    INT32        xferSize;
    INT32        chunkSize;
    INT32        i;
    UINT64       *bfr = NULL;
    INT32        bfrSize = SIZE_256K;

    fprintf(stderr, "@@@@@@@@@@@@@@@@@@@@ nv_TestCells(0x...%08X, 0x...%08X) @@@@@@@@@@@@@@@@@@@@\n",
            (UINT32)startAddr, (UINT32)endAddr);

    /*
    ** Allocate temp DRAM space to contain the data pattern, 256kB max.
    ** NOTE:  We can't wait on the Malloc, since the multi-tasking kernel
    **        isn't up yet. I could check K_xpcb, but didn't.
    */
    if (length >= SIZE_256K)
    {
        bfr = s_Malloc(bfrSize, __FILE__, __LINE__);
    }
    else if (length > 16)
    {
        bfrSize = length;
        bfr = s_Malloc(bfrSize, __FILE__, __LINE__);
    }

    /* Outer loop on test patterns */
    do
    {
        /* Load 64-bit test pattern */
        expData = *pTestPattern;

        fprintf(stderr, "nv_TestCells:  Testing with pattern 0x%08X...\n",
                (UINT32)expData);

        /* Initialize specified address range with 64-bit pattern */
        NV_MemSet((UINT32)startAddr, (UINT32)endAddr, &expData);

        /*
        ** Inner loop to verify pattern across specified address range.
        ** Use a DMA into the buffer if the Malloc was successful.
        */
        if (bfr != NULL)
        {
            /*
            ** Initialize the DRAM buffer, and setup the transfer length and
            ** NV memory offset prior to each pass.
            */
            memset(bfr, 0, bfrSize);
            srcAddr = (UINT32)startAddr;
            xferSize = length;

            /* Read the data, in "bfrSize" increments if necessary */
            while ((xferSize > 0) && (rc == 0))
            {
                chunkSize = (xferSize >= bfrSize) ? bfrSize : xferSize;

                /* Perform the DMA, blocking until it completes. */
                if ((rc = mm_DMA_Read((UINT32)bfr, srcAddr, chunkSize, TRUE)) == 0)
                {
                    /* Validate the data. */
                    for (i = 0; i < bfrSize/8; ++i)
                    {
                        if (bfr[i] != expData)
                        {
                            fprintf(stderr, "nv_TestCells:  Address = 0x...%08X, "
                                    "expected 0x...%08X, received 0x...%08X\n",
                                    srcAddr+(i*8), (UINT32)expData, (UINT32)bfr[i]);
                            rc = 4;
                            break;
                        }
                    }
                    xferSize -= chunkSize;
                    srcAddr += chunkSize;
                } /* rc == 0 after call */
                else
                {
                    fprintf(stderr, "nv_TestCells:  DMA completed with bad status 0x%08X\n", rc);
                }
            } /* while */
        }
        else
        {
            testAddr = startAddr;
            do
            {
                if ((rc = MM_Read((UINT32)testAddr, (UINT8 *)&recData, sizeof(recData))) != 0)
                {
                    fprintf(stderr, "nv_TestCells:  Read FAILED!  Address = 0x...%08X, rc = 0x%X\n",
                            (UINT32)testAddr, rc);
                }
                else if (expData != recData)
                {
                    fprintf(stderr, "nv_TestCells:  Address = 0x...%08X, expected 0x...%08X, received 0x...%08X\n",
                            (UINT32)testAddr, (UINT32)expData, (UINT32)recData);
                    rc = 3;
                }
            } while ((++testAddr < endAddr) && (rc == 0));
        }
    } while ((++pTestPattern < testPatternEnd) && (rc == 0));

    if (rc != 0)
    {
        fprintf(stderr, "@@@@@@ nv_TestCells(0x...%08X, 0x...%08X) failed with return code 0x%X @@@@@@\n",
                (UINT32)startAddr, (UINT32)endAddr, rc);
    }
    else
    {
        fprintf(stderr, "@@@@@@@@@@@@@@@ nv_TestCells(0x...%08X, 0x...%08X) successful @@@@@@@@@@@@@@\n",
                (UINT32)startAddr, (UINT32)endAddr);
    }

    /* Free the temp buffer now, if necessary. */
    if (bfr != NULL)
    {
        s_Free(bfr, bfrSize, __FILE__, __LINE__);
    }

    return (rc);
} /* nv_TestCells */
#endif /* MM_FULL_DIAGS */


/*
******************************************************************************
**  nv_Diags
**
**  @brief  This routine coordinates the diagnostic testing of the NV Memory
**
**  @param  None
**
**  @return None
**
******************************************************************************
*/
void nv_Diags(void)
{
    const struct NV_DIRECTORY   *nvDir;
    const struct NV_DIRECTORY   *nvDirEnd;
    const struct NV_DIRECTORY   *pnvRecord;
    NV_ADMIN                    adminRecord;

#ifdef MM_VERBOSE
    REG64 tscBegin, tscEnd, tscDelta_uS;
#endif

    /*
    ** Flash the Power LED at 3.5Hz to indicate diag tests are running.
    ** Ensure the fault LED is off. Preserve other bits.
    **
    ** NOTE:  The LED Control register is updated just once via masking,
    **        instead of multiple updates via the bit fields, since the card
    **        has a tendancy to "lose" intermediate register updates if they
    **        occur too quickly...
    */
    pMM->ledCtrl = (pMM->ledCtrl & (MM_MEM_VALID_MASK | MM_LED_RMV_MASK)) |
                   (MM_LED_FLASH_3_5HZ << MM_LED_PWR);
    MM_FORCE_WRITE_TO_FLUSH(pMM->ledCtrl);

    /*
    ** Card has been successfully initialized - set status to GOOD.
    ** If testing fails below, status will be updated accordingly.
    */
    mmInfo->status = NV_STS_GOOD | NV_STS_DIAG;

    /* Test the data lines (in the free area of the 16MB window). */
    if (nv_TestDataLines(nvDirectoryFree->start, nvDirectoryFree->start+0x10) != 0)
    {
        /* Test failed - Surface Fatal Error */
        nv_ProcessFatalEvent(NV_FATAL_POST, 0);
    }
    else if ((pMM->ledCtrl & MM_MEM_VALID_MASK) == 0)
    {
#ifdef MM_FULL_DIAGS
        /* Contents not valid - perform DESTRUCTIVE tests on entire window. */
        fprintf(stderr, "nv_Diags:  Memory Contents Invalid - Destructively Test Full Window\n");

#ifdef MM_VERBOSE
        tscBegin.dw = get_tsc();
#endif /* MM_VERBOSE */
        if (nv_TestAddrLines(0, nvDirectoryFree->end) != 0)
        {
            /* Surface Fatal Error */
            nv_ProcessFatalEvent(NV_FATAL_POST, 1);
        }
        else if (nv_TestCells((UINT64*)0, (UINT64*)nvDirectoryFree->end) != 0)
        {
            /* Surface Fatal Error */
            nv_ProcessFatalEvent(NV_FATAL_POST, 2);
        }
        else if (GetCleanShutdown() == FALSE)
        {
            /*
            ** Memory tests were successful.
            ** Prior shutdown was NOT graceful, so initialize entire window
            ** to all 'F's (this will invalidate any/all CRCs in the regions,
            ** and e.g. force a resync of all RAID-5s).
            */
            fprintf(stderr, "nv_Diags:  Memory Contents Invalid (after fault/shutdown) - "
                    "Initializing Full Window to 'F's\n");
            NV_MemSet(0, nvDirectoryFree->end, &testPattern[1]);
        }
#ifdef MM_VERBOSE
        tscEnd.dw = get_tsc();
        tscDelta_uS.dw = (tscEnd.dw - tscBegin.dw)/3200000;
        fprintf(stderr, "nv_Diags:  Full destructive test of 16MB window took %dms\n",
                tscDelta_uS.lo);
#endif /* MM_VERBOSE */
#else /* Not MM_FULL_DIAGS */
        /*
        ** Contents not valid - initialize entire window.
        ** If prior shutdown was graceful, zero the window; otherwise,
        ** set it to all 'F's (this will invalidate any/all CRCs in
        ** the regions, and e.g. force a resync of all RAID-5s).
        */
#ifdef MM_VERBOSE
        tscBegin.dw = get_tsc();
#endif /* MM_VERBOSE */
        if (GetCleanShutdown() == TRUE)
        {
            fprintf(stderr, "nv_Diags:  Memory Contents Invalid (after graceful shutdown) - "
                    "Initializing Full Window to 0's\n");
            NV_ZeroMemory(0, nvDirectoryFree->end);
        }
        else
        {
            fprintf(stderr, "nv_Diags:  Memory Contents Invalid (after fault/shutdown) - "
                    "Initializing Full Window to 'F's\n");
            NV_MemSet(0, nvDirectoryFree->end, &testPattern[1]);
        }

#ifdef MM_VERBOSE
        tscEnd.dw = get_tsc();
        tscDelta_uS.dw = (tscEnd.dw - tscBegin.dw)/3200000;
        fprintf(stderr, "nv_Diags:  Initialization of 16MB window took %dms\n",
                tscDelta_uS.lo);
#endif /* MM_VERBOSE */

        /* Verify the ECC (i.e. part is healthy) */
        if (nv_ScrubECC(0, nvDirectoryFree->end) != 0)
        {
            fprintf(stderr, "ECC errors persist after initializing window!\n");
            /* Scrub failed - Surface Fatal Error */
            nv_ProcessFatalEvent(NV_FATAL_POST, 3);
        }
#endif /* Not MM_FULL_DIAGS */

        /* If the window was initialized successfully, setup the admin area. */
        if ((mmInfo->status & ~NV_STS_DIAG) == NV_STS_GOOD)
        {
            fprintf(stderr, "nv_Diags:  Re-initialize \"Admn\" region\n");
            nv_InitAdminRegion();
        }

        /*
        ** Clear the Memory Manager area (> 16MB window), then verify the ECC
        ** to determine if this region is healthy
        */
#ifdef MM_VERBOSE
        tscBegin.dw = get_tsc();
#endif /* MM_VERBOSE */
        NV_ZeroMemory(nvDirectoryFree->end, mem_size);
        if (nv_ScrubECC(nvDirectoryFree->end, (mem_size-nvDirectoryFree->end)) != 0)
        {
            /* Scrub failed - Surface Fatal Error */
            fprintf(stderr, "ECC errors persist after initializing memory beyond window!\n");
            nv_ProcessFatalEvent(NV_FATAL_POST, 8);
        }
#ifdef MM_VERBOSE
        tscEnd.dw = get_tsc();
        tscDelta_uS.dw = (tscEnd.dw - tscBegin.dw)/3200000;
        fprintf(stderr, "nv_Diags:  Initialization of memory beyond 16MB window took %dms\n",
                tscDelta_uS.lo);
#endif /* MM_VERBOSE */

    } /* if NV Memory invalid / uninitialized */
    else
    {
        /* Some of the memory contents may be valid. Validate the NV Directory. */
#ifdef MM_VERBOSE
        tscBegin.dw = get_tsc();
#endif /* MM_VERBOSE */

        if (nv_ScrubECC(NV_ADMIN_START, NV_ADMIN_SIZE) != 0)
        {
            /*
            ** The region is invalid (ECC errors, or contents not valid).
            ** Wipe it out.
            */
            fprintf(stderr, "nv_Diags:  Content of \"Admn\" region, "
                    "offset 0x%X->0x%X, is invalid or has bad ECC "
                    "and will be re-initialized.\n",
                    NV_ADMIN_START, nvDirectoryAdmin->end);
            NV_ZeroMemory(NV_ADMIN_START, NV_ADMIN_END);

            /* Verify the region has correct ECC (i.e. part is healthy) */
            if (nv_ScrubECC(NV_ADMIN_START, NV_ADMIN_SIZE) != 0)
            {
                fprintf(stderr, "ECC errors persist after zeroing \"Admn\" region!\n");

                /* Scrub failed - Surface Fatal Error */
                nv_ProcessFatalEvent(NV_FATAL_POST, 6);
            }

            /* If the testing has been successful, re-initialize the region. */
            if ((mmInfo->status & ~NV_STS_DIAG) == NV_STS_GOOD)
            {
                fprintf(stderr, "nv_Diags:  Re-initialize \"Admn\" region\n");
                nv_InitAdminRegion();
            }
        }
#ifdef MM_VERBOSE
        tscEnd.dw = get_tsc();
        tscDelta_uS.dw = (tscEnd.dw - tscBegin.dw)/3200000;
        fprintf(stderr, "          Test/scrub of \"Admn\" region took %dms\n",
                tscDelta_uS.lo);
#endif /* MM_VERBOSE */

        /* Read the NV Directory from the board. */
        fprintf(stderr, "nv_Diags:  Read the NV Admin record from the board...\n");
        MM_Read(NV_ADMIN_START, (UINT8*)&adminRecord, sizeof(adminRecord));

        /* Attempt to validate the content before we go too far... */
        if (adminRecord.toc[NV_ADMIN_TOC_NV_DIR].offset == NV_ADMIN_OFFSET_FIRST_ENTRY)
        {
            nvDir = s_Malloc(adminRecord.toc[NV_ADMIN_TOC_NV_DIR].size, __FILE__, __LINE__);
            fprintf(stderr, "nv_Diags:  Read the NV Directory from the board...\n");
        }
        else
        {
            fprintf(stderr, "nv_Diags:  Invalid NV Admin record offset = 0x%X, expected 0x%X\n",
                    adminRecord.toc[NV_ADMIN_TOC_NV_DIR].offset, NV_ADMIN_OFFSET_FIRST_ENTRY);
            nvDir = NULL;
        }

        if ((nvDir == NULL) ||
            (MM_Read(NV_ADMIN_START + adminRecord.toc[NV_ADMIN_TOC_NV_DIR].offset,
                     (UINT8*)nvDir, adminRecord.toc[NV_ADMIN_TOC_NV_DIR].size) != 0))
        {
            /* Read failed, or data was invalid. Re-initialize and re-read. */
            if (nvDir != NULL)
            {
                s_Free((void *)nvDir, adminRecord.toc[NV_ADMIN_TOC_NV_DIR].size, __FILE__, __LINE__);
            }
            fprintf(stderr, "nv_Diags:  Re-initialize \"Admn\" region\n");
            nv_InitAdminRegion();

            MM_Read(NV_ADMIN_START, (UINT8*)&adminRecord, sizeof(adminRecord));
            if (adminRecord.toc[NV_ADMIN_TOC_NV_DIR].offset == NV_ADMIN_OFFSET_FIRST_ENTRY)
            {
                nvDir = s_Malloc(adminRecord.toc[NV_ADMIN_TOC_NV_DIR].size, __FILE__, __LINE__);
                fprintf(stderr, "nv_Diags:  Re-read the NV Directory from the board...\n");
                if (MM_Read(NV_ADMIN_START + adminRecord.toc[NV_ADMIN_TOC_NV_DIR].offset,
                            (UINT8*)nvDir,
                            adminRecord.toc[NV_ADMIN_TOC_NV_DIR].size) != 0)
                {
                    /* Retry failed. */
                    abort();
                }
            }
            else
            {
                fprintf(stderr, "nv_Diags:  Cannot read admin record\n");
                abort();
            }
        }

        /* Determine endpoint based on version */
        if (adminRecord.version == NV_ADMIN_VERSION)
        {
            nvDirEnd = &nvDir[NV_DIRECTORY_BOUNDS];
        }
        else
        {
            nvDirEnd = nvDir + adminRecord.toc[NV_ADMIN_TOC_NV_DIR].size;
        }

        /* Skip over the Admin Record, which was handled separately above */
        pnvRecord = &nvDir[1];

        /*
        ** Walk through the NV memory allocation table and validate the regions.
        ** Destructively test any areas whose contents are found to be invalid.
        */
        do
        {
            /*
            ** Scrub the entire region for ECC errors. If the scrub passes,
            ** invoke the verification function for the region.
            */
#ifdef MM_VERBOSE
            tscBegin.dw = get_tsc();
#endif /* MM_VERBOSE */
            if (nv_ScrubECC(pnvRecord->start,
                            (pnvRecord->end - pnvRecord->start)) != 0)
            {
                /*
                ** The region is invalid (ECC errors, or contents not valid).
                ** Wipe it out.
                */
                fprintf(stderr, "nv_Diags:  Content of \"%.4s\" region, "
                        "offset 0x%X->0x%X, is invalid or has bad ECC "
                        "and will be initialized to 'F's.\n",
                        pnvRecord->name, pnvRecord->start, pnvRecord->end);
                NV_MemSet(pnvRecord->start,
                          pnvRecord->end,
                          &testPattern[1]);

                /* Verify the region has correct ECC (i.e. part is healthy) */
                if (nv_ScrubECC(pnvRecord->start,
                                (pnvRecord->end - pnvRecord->start)) != 0)
                {
                    fprintf(stderr, "ECC errors persist after zeroing "
                            "\"%.4s\" region!\n", pnvRecord->name);

                    /* Scrub failed - Surface Fatal Error */
                    nv_ProcessFatalEvent(NV_FATAL_POST, 7);
                }
            }
            else
            {
                fprintf(stderr, "nv_Diags:  Content of \"%.4s\" region, "
                        "offset 0x%X->0x%X, is valid and has correct ECC.\n",
                        pnvRecord->name, pnvRecord->start, pnvRecord->end);
            }
#ifdef MM_VERBOSE
            tscEnd.dw = get_tsc();
            tscDelta_uS.dw = (tscEnd.dw - tscBegin.dw)/3200000;
            fprintf(stderr, "          Test/scrub of \"%.4s\" region took %dms\n",
                    pnvRecord->name, tscDelta_uS.lo);
#endif /* MM_VERBOSE */
        } while ((++pnvRecord < nvDirEnd) &&
                 ((mmInfo->status & ~NV_STS_DIAG) == NV_STS_GOOD));

        /*
        ** Scrub the entire memory region >16MB window for ECC errors.
        ** NOTE:  This may be replaced with a more granular test, once the full
        **        Memory Manager concept is implemented (e.g. verify/clear the
        **        region under NV_Malloc()).
        */
        --pnvRecord;
#ifdef MM_VERBOSE
        tscBegin.dw = get_tsc();
#endif /* MM_VERBOSE */
        if (nv_ScrubECC(pnvRecord->end, (mem_size-pnvRecord->end)) != 0)
        {
            /* The region is invalid (ECC errors). Wipe it out. */
            fprintf(stderr, "nv_Diags:  Content of memory region, "
                    "offset 0x%X->0x%X, has bad ECC and will be zeroed.\n",
                    pnvRecord->end, mem_size);
            NV_ZeroMemory(pnvRecord->end, mem_size);

            /* Verify the region has correct ECC now (i.e. part is healthy) */
            if (nv_ScrubECC(pnvRecord->end, (mem_size-pnvRecord->end)) != 0)
            {
                /* Scrub failed - Surface Fatal Error */
                fprintf(stderr, "ECC errors persist after zeroing memory region!\n");
                nv_ProcessFatalEvent(NV_FATAL_POST, 9);
            }
        }
        else
        {
            fprintf(stderr, "nv_Diags:  Content of memory region, "
                    "offset 0x%X->0x%X, has correct ECC.\n",
                    pnvRecord->end, mem_size);
        }
#ifdef MM_VERBOSE
        tscEnd.dw = get_tsc();
        tscDelta_uS.dw = (tscEnd.dw - tscBegin.dw)/3200000;
        fprintf(stderr, "          Test/scrub of memory region took %dms\n",
                tscDelta_uS.lo);
#endif /* MM_VERBOSE */

        /* Free the space allocated for the directory */
        s_Free((void *)nvDir, adminRecord.toc[NV_ADMIN_TOC_NV_DIR].size, __FILE__, __LINE__);
    }

    /*
    ** Turn off DIAG status indicator in MM info.
    ** Set the Power LED back to "on" state.
    ** Ensure the OK-to-remove LED is off.
    ** If status is good, set the bit indicating the memory is initialized.
    **
    ** NOTE:  The LED Control register is updated just once via masking,
    **        instead of multiple updates via the bit fields, since the card
    **        has a tendancy to "lose" intermediate register updates if they
    **        occur too quickly...
    */
    mmInfo->status &= ~NV_STS_DIAG;
    if (mmInfo->status == NV_STS_GOOD)
    {
        pMM->ledCtrl = (pMM->ledCtrl & MM_MEM_VALID_MASK) |
                       (MM_LED_PWR_ON << MM_LED_PWR) |
                       (MM_LED_RMV_OFF << MM_LED_RMV) |
                       (1 << MM_MEM_INIT);
        MM_FORCE_WRITE_TO_FLUSH(pMM->ledCtrl);
        fprintf(stderr, "nv_Diags:  Memory has been validated / initialized\n");
    }
    else
    {
        pMM->ledCtrl = (MM_LED_PWR_ON << MM_LED_PWR) |
                       (MM_LED_RMV_OFF << MM_LED_RMV);
        MM_FORCE_WRITE_TO_FLUSH(pMM->ledCtrl);
    }
} /* nv_Diags */


/*
******************************************************************************
**  nv_ScrubECC
**
**  @brief  This routine verifies the ECC in the specified region.
**
**  This routine reads the specified memory range to check for ECC errors.
**  If too many single-bit errors are detected, or if any multi-bit ECC errors
**  are detected, failure status returned.
**
**  NOTE:  Currently, only multi-bit ECC errors will cause a failure.
**
**  @param  UINT32 - starting location of test range
**  @param  UINT32 - length of test range
**
**  @return 0 if success, nonzero reason code for failure
**
******************************************************************************
*/
UINT32 nv_ScrubECC(UINT32 startAddr, UINT32 length)
{
    UINT32 rc = 0;
    UINT32 srcAddr = startAddr;
    UINT32 xferSize = length;
    UINT8  *scrubBfr = NULL;
    UINT8  stackBfr[256];
    UINT32 chunkSize = SIZE_256K;
    UINT32 mallocSize = SIZE_256K;
    bool   mallocBfr = true;

    /*
    ** Allocate temp DRAM space to receive the data, 256kB max.
    ** If length is small, or the malloc fails, just use the stack buffer.
    */
    if (length >= SIZE_256K)
    {
        scrubBfr = s_Malloc(mallocSize, __FILE__, __LINE__);
    }
    else if (length > sizeof(stackBfr))
    {
        mallocSize = length;
        scrubBfr = s_Malloc(mallocSize, __FILE__, __LINE__);
    }

    if (scrubBfr == NULL)
    {
        chunkSize = sizeof(stackBfr);
        scrubBfr = stackBfr;
        mallocBfr = false;
    }

    /* Read the data, in "chunkSize" increments if necessary */
    while ((xferSize >= chunkSize) && (rc == 0))
    {
        if ((rc = mm_DMA_Read((UINT32)scrubBfr, srcAddr, chunkSize, TRUE)) == 0)
        {
            xferSize -= chunkSize;
            srcAddr += chunkSize;
        } /* rc == 0 after call */
    }
    if ((xferSize > 0) && (rc == 0))
    {
        if ((rc = mm_DMA_Read((UINT32)scrubBfr, srcAddr, xferSize, TRUE)) != 0)
        {
            fprintf(stderr, "nv_ScrubECC:  DMA completed with bad status 0x%08X\n", rc);
        }
    }

    /* Free the temp buffer now, if necessary. */
    if (mallocBfr)
    {
        s_Free(scrubBfr, mallocSize, __FILE__, __LINE__);
    }
    return (rc);
} /* nv_ScrubECC */
#endif /* BACKEND */


/*
******************************************************************************
**  NV_MemSet
**
**  @brief  This routine sets all cells in the region to a constant value.
**
**  WARNING:  The length of the region is assumed to be a multiple of 8 bytes.
**
**  @param  UINT32  - starting address of range to clear
**  @param  UINT32  - ending address of range to clear
**  @param  UINT64* - pointer to 64-bit data pattern
**
**  @return None
**
******************************************************************************
*/
void NV_MemSet(UINT32 startAddr, UINT32 endAddr, const UINT64 *pattern)
{
    UINT8  *testAddr = (UINT8 *)startAddr;
    UINT32 rc = 0;
    INT32  length = endAddr - startAddr;
    INT32  i;
    UINT64 *bfr = NULL;
    INT32  bfrSize = SIZE_256K;

    /* Allocate temp DRAM space to contain the data pattern, 256kB max. */
    if (length >= SIZE_256K)
    {
        bfr = s_Malloc(bfrSize, __FILE__, __LINE__);
    }
    else if (length > 16)
    {
        bfrSize = length;
        bfr = s_Malloc(bfrSize, __FILE__, __LINE__);
    }

    /* Write the data via DMA if possible */
    if (bfr != NULL)
    {
        /* Set the specified doubleword pattern in the buffer. */
        for (i = 0; i < bfrSize/8; ++i)
        {
            bfr[i] = *pattern;
        }

        do
        {
            if ((rc = mm_DMA_Write((UINT32)bfr, (UINT32)testAddr, bfrSize, TRUE)) == 0)
            {
                length -= bfrSize;
                testAddr += bfrSize;
            }
        } while ((length >= bfrSize) && (rc == 0));

        if ((length > 0) && (rc == 0))
        {
            if ((rc = mm_DMA_Write((UINT32)bfr, (UINT32)testAddr, length, TRUE)) == 0)
            {
                testAddr += length;
                length = 0;
            }
        }
    }

    /* Loop to write specified address range with specified pattern */
    rc = 0;
    while (((UINT64 *)testAddr < (UINT64*)endAddr) && (rc == 0))
    {
        rc = MM_Write((UINT32)testAddr, (UINT8*)pattern, sizeof(*pattern));
        testAddr += sizeof(*pattern);
    }

    /* Free the temp buffer now, if necessary. */
    if (bfr != NULL)
    {
        s_Free(bfr, bfrSize, __FILE__, __LINE__);
    }
} /* NV_MemSet */


/*
******************************************************************************
**  NV_ZeroMemory
**
**  @brief  This routine clears (zeroes) all cells in the specified region.
**
**  WARNING:  The length of the region is assumed to be a multiple of 8 bytes.
**
**  @param  UINT32 - starting address of range to clear
**  @param  UINT32 - ending address of range to clear
**
**  @return None
**
******************************************************************************
*/
void NV_ZeroMemory(UINT32 startAddr, UINT32 endAddr)
{
    NV_MemSet(startAddr, endAddr, zeroPattern);
} /* NV_ZeroMemory */


#ifdef FRONTEND
/*
******************************************************************************
**  MM_Interrupt
**
**  @brief  MicroMemory Card Interrupt Handler
**
**  This function serves as the interrupt handler for the MM card.
**
**  @param  UINT32 - unused parameter
**
**  @return None
**
******************************************************************************
**/
void MM_Interrupt(UINT32 value UNUSED)
{
    UINT8   err_status;
    UINT8   err_count;
    UINT8   comp_status;
    UINT32  errAddr = 0;

    /* Determine interrupt source */
    if ((err_count = pMM->errCnt) != 0)
    {
        /* ECC error(s) occurred. Fix the error, if it's a single bit. */
        mmInfo->errCount += err_count;
        err_status = pMM->errSts;
#ifdef MM_VERBOSE
        fprintf(stderr, "@@@@@@@@@@@@@@@@@@@@ MicroMemory Interrupt @@@@@@@@@@@@@@@@@@@@\n");
        fprintf(stderr, "@@  EDC Error Status = 0x%02X\n@@   EDC Error Count = 0x%02X\n",
                err_status, err_count);
#endif
        if (err_status & MULTI_BIT_ERROR)
        {
            /* Surface a fatal event - report address of latest error. */
            fprintf(stderr, "MM_Interrupt:  A multi-bit error has occurred\n");
            if (pMM->errLast.synBits != 0)
            {
                errAddr = pMM->errLast.addrLo;
                fprintf(stderr, "MM_Interrupt:  Last error at 0x%08X\n", errAddr);
            }
            if (pMM->errFirst.synBits != 0)
            {
                errAddr = pMM->errFirst.addrLo;
                fprintf(stderr, "MM_Interrupt:  First error at 0x%08X\n", errAddr);
            }
            nv_ProcessFatalEvent(NV_FATAL_ECC_MULTI, errAddr);
        }
        else
        {
            /* Attempt to fix the ECC error(s) */
            mm_ProcessECCErrors();
        }

        /* Clear the error counter */
        mm_ClearErrorCounter();
    } /* err_count */

    /* Check for DMA completion */
    if ((comp_status = pMM->dmaDesc.stsCtrl.compSts) != 0)
    {
        /*
        ** Clear the completion status bits by writing 1 to those that are set.
        ** This clears the source of the interrupt also, for DMA complete.
        ** NOTE:  Don't blindly write both bits, else this seems to impact the
        **        Status/Control results found in the semaphore.
        */
        pMM->dmaDesc.stsCtrl.compSts = comp_status;

        /* The following forces possible write to dmaDesc (same address range). */
        MM_FORCE_WRITE_TO_FLUSH(pMM->ledCtrl);
#ifdef NV_DMA_DRVR
        /* LSW */ if (!(comp_status & 0x02))
            fprintf(stderr, "MM_Interrupt:  Completion Status = 0x%X\n", comp_status);
#endif
        /* Initiate the next ready DMA request */
        if ((pDMAQueue->nextSub != pDMAQueue->in)/* && (pDMAQueue->active == NULL)*/)
        {
#if 0 /* LSW - Don't initiate next request from interrupt handler yet... */
            /* Submit the request to the hardware */
            pDMAQueue->active = pDMAQueue->nextSub;
            mm_DMA(&pDMAQueue->nextSub->dmaDesc, FALSE);

            /* Advance the "nextSub" pointer */
            if (&pDMAQueue->nextSub[1] != pDMAQueue->end)
            {
                /* no wrap - advance "nextSub" pointer */
                ++pDMAQueue->nextSub;
            }
            else
            {
                /* wrap */
                pDMAQueue->nextSub = pDMAQueue->begin;
            }
#endif

            /* Awaken the DMA Executor, if necessary, to start the next DMA */
            if (TaskGetState(pNV_DMAExecPCB) == PCB_NV_DMA_EXEC_WAIT)
            {
#ifdef HISTORY_KEEP
CT_history_pcb("MM_Interrupt setting ready pcb", (UINT32)pNV_DMAExecPCB);
#endif
                TaskSetState(pNV_DMAExecPCB, PCB_READY);
            }
        } /* if requests pending on DMA queue */

        /*
        ** Awaken the DMA Completor to process the completed request(s)
        ** NOTE:  This runs at a higher priority than the Executor task
        */
        TaskReadyByState(PCB_NV_DMA_COMP_WAIT);
    } /* DMA completion */
} /* MM Interrupt */
#endif /* FRONTEND */


/*
******************************************************************************
** MM_Write
**
**  @brief  Writes to MM-5425CN Micro Memory memory
**
**  This function returns error status after the write depending
**  on whether error status reporting is enabled in NV_Memory.h
**  There are 2 versions of the Reads and Writes, one being the MEMCPY_VERSION
**  where we try to memcpy data in chunks of Micro Memory Window Size (16MB).
**  The other version of Read/Write uses a custom algorithm.
**  Passed in parameters are checked for validity.
**
**  Note: It is assumed that the calling function has allocated a buffer
**  of size length bytes and passes in a valid pointer to the buf from
**  which data is copied into MM memory.
**
**  @param  UINT32  - 32-bit destination address
**  @param  UINT8 * - pointer to source data buffer
**  @param  UINT32  - length of data to copy (bytes)
**
**  @return -1 if failure, 0 if success
**
******************************************************************************
**/
INT32 MM_Write(UINT32 destAddr, UINT8 *srcBfr, UINT32 length)
{
    UINT32 windowOffset = destAddr;
    UINT32 j;
    UINT8  window = 0;
#ifdef MEMCPY_VERSION
    UINT32 i;
    UINT32 total;
    UINT32 remainder;
    UINT32 offset;
#endif

    if (mmInfo == NULL)
    {
        fprintf(stderr, "MM_Write(0x%08X, %p, 0x%X):  Card not initialized "
                "or is inaccessible!\n", destAddr, srcBfr, length);
#ifdef BACKEND
        return(-1);
#else /* FRONTEND */
        /*
        ** Set task status to wait for MM Info MRP to be received, then
        ** give up CPU until re-awakened.
        */
        TaskSetMyState(PCB_MM_WAIT);
        TaskSwitch();
        fprintf(stderr, "MM_Write(0x%08X, %p, 0x%X):  Awakened from "
                "MM_WAIT state.\n", destAddr, srcBfr, length);
#endif
    }

    if ((mmInfo->status & ~NV_STS_DIAG) != NV_STS_GOOD)
    {
#ifdef MM_VERBOSE
        fprintf(stderr, "MM_Write(0x%08X, %p, 0x%X):  Card is inaccessible "
                "due to bad status = %d\n", destAddr, srcBfr, length, mmInfo->status);
#endif
        return(-1);
    }

#ifdef MM_DEBUG
    fprintf(stderr, "MM_Write(0x%08X, %p, 0x%X),"
            " Data First Word=0x%08X\n\n\n----",
            destAddr, srcBfr, length, *(UINT32 *)srcBfr);
#endif

    if ((length == 0) || (length > mem_size))
    {
        fprintf(stderr, "MM_Write:  Invalid length of 0x%X bytes!\n", length);
        nv_ProcessFatalEvent(NV_FATAL_ASSERT, 0);
        return -1;
    }

    if ((windowOffset + length) > mem_size)
    {
        fprintf(stderr, "MM_Write:  Cannot write to an address > memory size "
                "of 0x%08X bytes.\n", mem_size);
        nv_ProcessFatalEvent(NV_FATAL_ASSERT, 1);
        return -1;
    }

    if (!srcBfr)
    {
        fprintf(stderr, "MM_Write:  Source data pointer is NULL!");
        nv_ProcessFatalEvent(NV_FATAL_ASSERT, 2);
        return -1;
    }

    if (windowOffset >= SIZE_16MEG)
    {
        window = (UINT8)(windowOffset / SIZE_16MEG);
        windowOffset = windowOffset % SIZE_16MEG;
    }

    /* Update window register if necessary */
    if (pMM->windowMap != window)
    {
        pMM->windowMap = window;
        /* MM_FORCE_WRITE_TO_FLUSH done by pMM->errCnt read below, ERROR_STATUS_ENABLE. */
#ifdef MM_DEBUG
        fprintf(stderr, "MM_Write:  Setting Window Map reg to 0x%X\n", window);
#endif
    }

#ifdef ERROR_STATUS_ENABLE
    /* Clear error counter if necessary */
    if (pMM->errCnt != 0)
    {
        mm_ClearErrorCounter();
    }
#endif
#ifdef MEMCPY_VERSION
    total = windowOffset + length;
    if (total <= SIZE_16MEG)
    {
        memcpy((UINT8*)pMMemory + windowOffset, srcBfr, length);
        return 0;
    }

    remainder = length;
    offset = 0;
    memcpy((UINT8*)pMMemory + windowOffset, srcBfr, (SIZE_16MEG - windowOffset));

    offset += (SIZE_16MEG - windowOffset);
    remainder -= (SIZE_16MEG  - windowOffset);
    if (!remainder)
    {
        return 0;
    }
    j = remainder / SIZE_16MEG;

    pMM->windowMap = ++window;

    for (i = 0; i < j; ++i)
    {
        memcpy((UINT8*)pMMemory, srcBfr + offset, SIZE_16MEG);
        remainder -= SIZE_16MEG;
        offset += SIZE_16MEG;
        pMM->windowMap = ++window;
    }

    if (remainder)
    {
        memcpy((UINT8*)pMMemory, srcBfr + offset, remainder);
    }
#else /* Not MEMCPY_VERSION */
    j = 0;

    /* Write any initial bytes to get to a short boundary (destination) */
    if (windowOffset & 0x01)
    {
        pMMemory[windowOffset++] = srcBfr[j++];
    }

    /*
    ** Write any initial shorts to get to a word boundary (destination)
    ** (i.e. ensure length is still large enough, too)
    */
    if ((windowOffset & 0x03) && ((j + sizeof(UINT16)) <= length))
    {
        /* Check for window wrap, and advance the window register if necessary */
        if (windowOffset >= SIZE_16MEG)
        {
            pMM->windowMap = ++window;
            MM_FORCE_WRITE_TO_FLUSH(pMM->windowMap);
            windowOffset = 0;
        }
        *(volatile UINT16*)&pMMemory[windowOffset] = *(UINT16*)&srcBfr[j];
        windowOffset += sizeof(UINT16);
        j += sizeof(UINT16);
    }

    /*
    ** Write any initial words to get to a doubleword boundary (destination)
    ** (i.e. ensure length is still large enough, too)
    */
    if ((windowOffset & 0x07) && ((j + sizeof(UINT32)) <= length))
    {
        /* Check for window wrap, and advance the window register if necessary */
        if (windowOffset >= SIZE_16MEG)
        {
            pMM->windowMap = ++window;
            MM_FORCE_WRITE_TO_FLUSH(pMM->windowMap);
            windowOffset = 0;
        }
        *(volatile UINT32*)&pMMemory[windowOffset] = *(UINT32*)&srcBfr[j];
        windowOffset += sizeof(UINT32);
        j += sizeof(UINT32);
    }

    /*
    ** Destination address is now on a doubleword boundary.
    ** Write multiples of doublewords (64 bits), if any
    */
    while ((j + sizeof(UINT64)) <= length)
    {
        /* Check for window wrap, and advance the window reg if necessary */
        if (windowOffset >= SIZE_16MEG)
        {
            pMM->windowMap = ++window;
            MM_FORCE_WRITE_TO_FLUSH(pMM->windowMap);
            windowOffset = 0;
        }
        *(volatile UINT64*)&pMMemory[windowOffset] = *(UINT64*)&srcBfr[j];
        windowOffset += sizeof(UINT64);
        j += sizeof(UINT64);
    } /* doublewords */

    /* Write a full word (32 bits), if any */
    if ((j + sizeof(UINT32)) <= length)
    {
        /* Check for window wrap, and advance the window reg if necessary */
        if (windowOffset >= SIZE_16MEG)
        {
            pMM->windowMap = ++window;
            MM_FORCE_WRITE_TO_FLUSH(pMM->windowMap);
            windowOffset = 0;
        }
        *(volatile UINT32*)&pMMemory[windowOffset] = *(UINT32*)&srcBfr[j];
        j += sizeof(UINT32);
        windowOffset += sizeof(UINT32);
    } /* words */

    /* Write a short (16 bits), if any */
    if ((j + sizeof(UINT16)) <= length)
    {
        /* Check for window wrap, and advance the window reg if necessary */
        if (windowOffset >= SIZE_16MEG)
        {
            pMM->windowMap = ++window;
            MM_FORCE_WRITE_TO_FLUSH(pMM->windowMap);
            windowOffset = 0;
        }
        *(volatile UINT16*)&pMMemory[windowOffset] = *(UINT16*)&srcBfr[j];
        j += sizeof(UINT16);
        windowOffset += sizeof(UINT16);
    } /* shorts */

    /* Write remaining bytes, if any */
    while (j < length)
    {
        /* Check for window wrap, and advance the window register if necessary */
        if (windowOffset >= SIZE_16MEG)
        {
            pMM->windowMap = ++window;
            MM_FORCE_WRITE_TO_FLUSH(pMM->windowMap);
            windowOffset = 0;
        }
        pMMemory[windowOffset++] = srcBfr[j++];
    }
#endif /* Not MEMCPY_VERSION */

    /*
    ** Clear the error counter - this also flushes the previous PCI writes.
    ** Comment WRONG - 2007/04/25 - Only for the one address range, not the other.
    */
    if (pMM->errCnt != 0)
    {
        mm_ClearErrorCounter();
    }
/* LSW - Ignore error counter on writes, since it appears we can get errors
         from the read/modify/write process - which also doesn't seem to
         correct the write-back data?!
#ifdef ERROR_STATUS_ENABLE
    if (pMM->errCnt != 0)
    {
        err_count += pMM->errCnt;
        fprintf(stderr, "MM_Write:  Error occurred during write, Error Counter = 0x%X\n",
                pMM->errCnt);
        return -1;
    }
#endif */
    /*
    ** 2007/04/25 - Force read of last byte written, to force flush to memory
    ** of data in second mapped pci address range.
    */
    {
        UINT8 ignore;
        MM_Read(destAddr+length-1, &ignore, 1);
    }

    return 0;
} /* MM_Write */


/*
******************************************************************************
** MM_Read
**
**  @brief  Reads from MM-5425CN Micro Memory memory
**
**  This function returns error status after the read depending
**  on whether error status reporting is enabled in NV_Memory.h
**  There are 2 versions of the Reads and Writes, one being the MEMCPY_VERSION
**  where we try to memcpy data in chunks of Micro Memory Window Size (16MB).
**  The other version of Read/Write uses a custom algorithm.
**  Passed in parameters are checked for validity.
**
**  Note: It is assumed that the calling function has allocated a buffer
**  of size length bytes and passes in a valid pointer to the buf.
**
**  @param  UINT32  - 32-bit source address
**  @param  UINT8 * - pointer to destination data buffer
**  @param  UINT32  - length of transfer (bytes)
**
**  @return -1 if failure, 0 if success
**
******************************************************************************
**/

INT32 MM_Read(UINT32 srcAddr, UINT8 *destBfr, UINT32 length)
{
    UINT32 windowOffset = srcAddr;
    UINT32 j;
    UINT8 window = 0;
#ifdef ERROR_STATUS_ENABLE
    UINT8 err_status;
#endif
#ifdef MEMCPY_VERSION
    UINT32 i;
    UINT32 total;
    UINT32 offset;
    UINT32 remainder;
#endif

    if (mmInfo == NULL)
    {
        if (gMMCFound){         /* Only print out message once. */
            gMMCFound = FALSE;
            fprintf(stderr, "MM_Read(0x%08X, %p, 0x%X):  Card not initialized "
                    "or is inaccessible!\n", srcAddr, destBfr, length);
        }
#ifdef BACKEND
        return(-1);
#else /* FRONTEND */
        /*
        ** Set task status to wait for MM Info MRP to be received, then
        ** give up CPU until re-awakened.
        */
        TaskSetMyState(PCB_MM_WAIT);
        TaskSwitch();
        fprintf(stderr, "MM_Read(0x%08X, %p, 0x%X):  Awakened from MM_WAIT state.\n",
                srcAddr, destBfr, length);
#endif
    }

    if ((mmInfo->status & ~NV_STS_DIAG) != NV_STS_GOOD)
    {
#ifdef MM_VERBOSE
        fprintf(stderr, "MM_Read:  Card is inaccessible due to bad status = %d\n",
                mmInfo->status);
#endif
        return(-1);
    }

    if ((length == 0) || (length > mem_size))
    {
        fprintf(stderr, "MM_Read:  Invalid length of 0x%X bytes!\n", length);
        nv_ProcessFatalEvent(NV_FATAL_ASSERT, 3);
        return -1;
    }

    if ((windowOffset + length) > mem_size)
    {
        fprintf(stderr, "MM_Read:  Cannot read from an address > memory size of"
                " 0x%08X bytes.\n", mem_size);
        nv_ProcessFatalEvent(NV_FATAL_ASSERT, 4);
        return -1;
    }

    if (!destBfr)
    {
        fprintf(stderr, "MM_Read:  Destination Pointer is NULL!");
        nv_ProcessFatalEvent(NV_FATAL_ASSERT, 5);
        return -1;
    }

    if (windowOffset >= SIZE_16MEG)
    {
        window = (UINT8)(windowOffset / SIZE_16MEG);
        windowOffset = windowOffset % SIZE_16MEG;
    }

    /* Update window register if necessary */
    if (pMM->windowMap != window)
    {
        pMM->windowMap = window;
        /* MM_FORCE_WRITE_TO_FLUSH done by pMM->errCnt read below, ERROR_STATUS_ENABLE. */
#ifdef MM_DEBUG
        fprintf(stderr, "MM_Read:  Setting Window Map reg to 0x%X\n", window);
#endif
    }

#ifdef ERROR_STATUS_ENABLE
    /* Clear error counter if necessary */
    if (pMM->errCnt != 0)
    {
        mm_ClearErrorCounter();
    }
#endif
#ifdef MEMCPY_VERSION
    total = windowOffset + length;
    if (total <= SIZE_16MEG)
    {
        memcpy(destBfr, (UINT8*)pMMemory + windowOffset, length);
        return 0;
    }

    remainder = length;
    offset = 0;
    memcpy(destBfr, (UINT8*)pMMemory + windowOffset,(SIZE_16MEG - windowOffset));

    offset += (SIZE_16MEG - windowOffset);
    remainder -= (SIZE_16MEG - windowOffset);
    if (!remainder)
    {
        return 0;
    }
    j = remainder / SIZE_16MEG;

    pMM->windowMap = ++window;

    for (i = 0; i < j; ++i)
    {
        memcpy(destBfr + offset, (UINT8*)pMMemory, SIZE_16MEG);
        remainder -= SIZE_16MEG;
        offset += SIZE_16MEG;
        pMM->windowMap = ++window;
    }

    if (remainder)
    {
        memcpy(destBfr + offset, (UINT8*)pMMemory, remainder);
    }
#else /* Not MEMCPY_VERSION */
    j = 0;

    /* Read any initial bytes to get to a short boundary (destination) */
    if (windowOffset & 0x01)
    {
        destBfr[j++] = pMMemory[windowOffset++];
    }

    /*
    ** Read any initial shorts to get to a word boundary (destination)
    ** (i.e. ensure length is still large enough, too)
    */
    if ((windowOffset & 0x03) && ((j + sizeof(UINT16)) <= length))
    {
        /* Check for window wrap, and advance the window register if necessary */
        if (windowOffset >= SIZE_16MEG)
        {
            pMM->windowMap = ++window;
            MM_FORCE_WRITE_TO_FLUSH(pMM->windowMap);
            windowOffset = 0;
        }
        *(UINT16*)&destBfr[j] = *(volatile UINT16*)&pMMemory[windowOffset];
        windowOffset += sizeof(UINT16);
        j += sizeof(UINT16);
    }

    /*
    ** Read any initial words to get to a doubleword boundary (destination)
    ** (i.e. ensure length is still large enough, too)
    */
    if ((windowOffset & 0x07) && ((j + sizeof(UINT32)) <= length))
    {
        /* Check for window wrap, and advance the window register if necessary */
        if (windowOffset >= SIZE_16MEG)
        {
            pMM->windowMap = ++window;
            MM_FORCE_WRITE_TO_FLUSH(pMM->windowMap);
            windowOffset = 0;
        }
        *(UINT32*)&destBfr[j] = *(volatile UINT32*)&pMMemory[windowOffset];
        windowOffset += sizeof(UINT32);
        j += sizeof(UINT32);
    }

    /*
    ** Destination address is now on a doubleword boundary.
    ** Read multiples of doublewords (64 bits), if any
    */
    while ((j + sizeof(UINT64)) <= length)
    {
        /* Check for window wrap, and advance the window reg if necessary */
        if (windowOffset >= SIZE_16MEG)
        {
            pMM->windowMap = ++window;
            MM_FORCE_WRITE_TO_FLUSH(pMM->windowMap);
            windowOffset = 0;
        }
        *(UINT64*)&destBfr[j] = *(volatile UINT64*)&pMMemory[windowOffset];
        windowOffset += sizeof(UINT64);
        j += sizeof(UINT64);
    } /* doublewords */

    /* Read a full word (32 bits), if any */
    if ((j + sizeof(UINT32)) <= length)
    {
        /* Check for window wrap, and advance the window reg if necessary */
        if (windowOffset >= SIZE_16MEG)
        {
            pMM->windowMap = ++window;
            MM_FORCE_WRITE_TO_FLUSH(pMM->windowMap);
            windowOffset = 0;
        }
        *(UINT32*)&destBfr[j] = *(volatile UINT32*)&pMMemory[windowOffset];
        j += sizeof(UINT32);
        windowOffset += sizeof(UINT32);
    } /* words */

    /* Read a short (16 bits), if any */
    if ((j + sizeof(UINT16)) <= length)
    {
        /* Check for window wrap, and advance the window reg if necessary */
        if (windowOffset >= SIZE_16MEG)
        {
            pMM->windowMap = ++window;
            MM_FORCE_WRITE_TO_FLUSH(pMM->windowMap);
            windowOffset = 0;
        }
        *(UINT16*)&destBfr[j] = *(volatile UINT16*)&pMMemory[windowOffset];
        j += sizeof(UINT16);
        windowOffset += sizeof(UINT16);
    } /* shorts */

    /* Read remaining bytes, if any */
    while (j < length)
    {
        /* Check for window wrap, and advance the window register if necessary */
        if (windowOffset >= SIZE_16MEG)
        {
            pMM->windowMap = ++window;
            MM_FORCE_WRITE_TO_FLUSH(pMM->windowMap);
            windowOffset = 0;
        }
        destBfr[j++] = pMMemory[windowOffset++];
    }
#endif /* Not MEMCPY_VERSION */

#ifdef ERROR_STATUS_ENABLE
    /*
    ** Flush the above PCI writes by performing a read.
    ** Comment wrong - 2007/04/25 - this is a read, not a write, and since we
    ** needed the write to first address range flushed before the read, they
    ** are done at that time. But, since handling memory errors is good at
    ** this point, do so.
    */
    if (pMM->errCnt)
    {
        mmInfo->errCount += pMM->errCnt;
        fprintf(stderr, "MM_Read:  Error(s) during read of 0x%X bytes from"
                " offset 0x%X, error counter = 0x%X\n", length, srcAddr, pMM->errCnt);

        err_status = pMM->errSts;
        if (err_status & MULTI_BIT_ERROR)
        {
            fprintf(stderr, "  Multi-bit error during read, error status = 0x%X\n",
                    err_status);
            /* Surface a fatal event */
            nv_ProcessFatalEvent(NV_FATAL_ECC_MULTI, 0);
        }
        else
        {
            fprintf(stderr, "  Error status = 0x%X\n", err_status);
            /* Attempt to fix the ECC error(s) */
            mm_ProcessECCErrors();
        }

        /* Clear the error counter */
        mm_ClearErrorCounter();
        return err_status;
    }
#endif /* ERROR_STATUS_ENABLE */
    return 0;
} /* MM_Read */


/*
******************************************************************************
** mm_DMA_Write
**
**  @brief  Writes to MM-5425CN Micro Memory memory using a simple DMA transfer.
**
**  A 64 bit PCI Address is passed in as the source of the DMA transfer,
**  the second parameter is the target address on the board (ranges from 0 to
**  mem_size, or the max available memory on the MM Board.) The third parameter
**  is the transfer size, or amount of data to be transferred.
**
**  Note: It is assumed that the calling function passes in a valid DMA'able
**  PCI Bus address as the first parameter PCI_Addr. An invalid PCI_Addr value
**  will very likely end up in a segmentation fault after invoking the function.
**  It is assumed that the source address range is DMA'able memory, whose PCI
**  address is passed in.
**  Also PCI_Addr and Local_Addr must be quad word aligned.
**  The fourth parameter, block (8 bit value), will result in the function call
**  blocking till the end of the DMA transfer if non-zero.
**  If block equals zero, the function will return immediately without
**  waiting for DMA completion.
**
**  @return 0 if success, non-zero value if failure.
**
******************************************************************************
*/
#ifdef PCI_DMA_ADDR
INT32 mm_DMA_Write(UINT64 PCI_Addr, UINT64 Local_Addr, UINT32 length, UINT8 block)
{
#else /* PCI_DMA_ADDR */
INT32 mm_DMA_Write(UINT32 sharedAddr, UINT64 Local_Addr, UINT32 length, UINT8 block)
{
    UINT32 PCI_Addr = LI_GetPhysicalAddr(sharedAddr);
#endif /* PCI_DMA_ADDR */
    INT32  rc = 0;
    UINT8  err_status;

    if ((mmInfo == NULL) || ((mmInfo->status & ~NV_STS_DIAG) != NV_STS_GOOD))
    {
#ifdef MM_VERBOSE
        fprintf(stderr, "mm_DMA_Write:  Card not initialized or inaccessible!\n");
#endif
        return(-1);
    }

    if ((Local_Addr + length) > mem_size)
    {
        fprintf(stderr, "mm_DMA_Write:  Local_Addr + length cannot exceed"
                " %d bytes\n", mem_size);
        nv_ProcessFatalEvent(NV_FATAL_ASSERT, 6);
        return(-1);
    }

    if ((Local_Addr % 8) || (PCI_Addr % 8) || (length % 8))
    {
        fprintf(stderr, "mm_DMA_Write:  PCI_Addr, Local_Addr and length"
                " must be quad word aligned\n");
        nv_ProcessFatalEvent(NV_FATAL_ASSERT, 7);
        return(-1);
    }

    pMM->dmaDesc.pciDataAddr = PCI_Addr;
    pMM->dmaDesc.localAddr   = Local_Addr;
    pMM->dmaDesc.xferSize    = length;
    pMM->dmaDesc.pciDescAddr = 0;
    pMM->dmaDesc.pciSemAddr  = 0;
    pMM->dmaDesc.stsCtrlLo   = MM_DMA_ALLINT_EN | MM_DMA_GO | MM_DMA_WRT |
                               MM_DMA_COMP | MM_DMA_CHN_COMP |
                               MM_DMA_PCI_MEM_RD_MULT | MM_DMA_PCI_MEM_WRT;

    /* Flush the prior register writes out of the bridge & start the DMA. */
    MM_FORCE_WRITE_TO_FLUSH(pMM->dmaDesc.stsCtrlLo);

    if (block)
    {
        while (pMM->dmaDesc.stsCtrl.go)
        {
            /*
            ** Loop here waiting for DMA completion.
            ** NOTE:  This needs to be replaced with an interrupt & semaphore.
            */
#ifdef MM_DEBUG
            fprintf(stderr,"mm_DMA_Write:  DMA Status/Control Reg = 0x%X\n",
                    pMM->dmaDesc.stsCtrlLo);
#endif
            if (K_xpcb != NULL)
            {
                /* Kernel is up and running, so switch context. */
                TaskSwitch();
            }
            else
            {
                /* No PCB exists - kernel isn't up & running yet. Sleep 100us. */
                TaskSleepNoSwap(100);
            }
        } /* while GO */

        /* Check for errors */
        if ((err_status = pMM->dmaDesc.stsCtrl.errSts) != 0)
        {
            if (err_status & ~MM_ECC_ANY_ERR)
            {
                fprintf(stderr, "mm_DMA_Write:  DMA completed with status 0x%X\n",
                        err_status);

                /* Surface a fatal event */
                nv_ProcessFatalEvent(NV_FATAL_ECC_MULTI, err_status);
            }
            else
            {
                /* Single-bit ECC error - attempt to fix the error(s) */
                mm_ProcessECCErrors();
            }

            /* Clear the error counter */
            mm_ClearErrorCounter();
            rc = (err_status & ~MM_ECC_ANY_ERR);
        } /* DMA error */

        /* Clear completion bits */
        pMM->dmaDesc.stsCtrl.compSts = MM_DMA_CLR_COMP;

        /* Any read in the area will flush. */
        MM_FORCE_WRITE_TO_FLUSH(pMM->ledCtrl);
    } /* block */

    return rc;
} /* mm_DMA_Write */


#ifdef BACKEND
/*
******************************************************************************
** mm_DMA_Read
**
**  @brief  Reads from MM-5425CN Micro Memory memory using a simple DMA transfer.
**
**  A 64 bit PCI Address is passed in as the destination of the DMA transfer,
**  the second parameter is the source address on the board (ranges from 0 to
**  mem_size, or the max available memory on the MM Board.) The third parameter
**  is the transfer size, or amount of data to be transferred.
**  Note: It is assumed that the calling function passes in a valid DMA'able
**  PCI Bus address as the first parameter PCI_Addr. An invalid PCI_Addr value
**  will very likely end up in a segmentation fault after invoking the function.
**  It is assumed that the target address range is DMA'able memory, whose PCI
**  address is passed in.
**  Also PCI_Addr and Local_Addr must be quad word aligned.
**  The fourth parameter, block (8 bit value), will result in the function call
**  blocking till the end of the DMA transfer if non-zero.
**  If block equals zero, the function will return immediately without
**  waiting for DMA completion.
**
**  @return non-zero if failure, 0 if success
**
******************************************************************************
*/
#ifdef PCI_DMA_ADDR
INT32 mm_DMA_Read(UINT64 PCI_Addr, UINT64 Local_Addr, UINT32 length, UINT8 block)
{
#else /* PCI_DMA_ADDR */
INT32 mm_DMA_Read(UINT32 sharedAddr, UINT64 Local_Addr, UINT32 length, UINT8 block)
{
    UINT32 PCI_Addr = LI_GetPhysicalAddr(sharedAddr);
#endif /* PCI_DMA_ADDR */
    INT32  rc = 0;
    UINT8  err_status;

    if ((mmInfo == NULL) || ((mmInfo->status & ~NV_STS_DIAG) != NV_STS_GOOD))
    {
#ifdef MM_VERBOSE
        fprintf(stderr, "mm_DMA_Read:  Card not initialized or is inaccessible!\n");
#endif
        return(-1);
    }

    if ((Local_Addr + length) > mem_size)
    {
        fprintf(stderr, "mm_DMA_Read(0x%08X, 0x%08X, 0x%X, %d):  Local_Addr + length "
                "cannot exceed 0x%X bytes\n", (UINT32)PCI_Addr, (UINT32)Local_Addr, length, block, mem_size);
        nv_ProcessFatalEvent(NV_FATAL_ASSERT, 8);
        return(-1);
    }

    if ((Local_Addr % 8) || (PCI_Addr % 8) || (length % 8))
    {
        fprintf(stderr, "mm_DMA_Read(0x%08X, 0x%08X, 0x%X, %d):  PCI_Addr, Local_Addr "
                "and length must be quad word aligned\n", (UINT32)PCI_Addr, (UINT32)Local_Addr, length, block);
        nv_ProcessFatalEvent(NV_FATAL_ASSERT, 9);
        return(-1);
    }

/*    memset((void*)&pMM->dmaDesc.pciDataAddr, 0, DMA_CSR_SPACE);*/
    pMM->dmaDesc.pciDataAddr = PCI_Addr;
    pMM->dmaDesc.localAddr   = Local_Addr;
    pMM->dmaDesc.xferSize    = length;
    pMM->dmaDesc.pciDescAddr = 0;
    pMM->dmaDesc.pciSemAddr  = 0;
#ifdef MEM_WRITE_INVALIDATE
    pMM->dmaDesc.stsCtrlLo   = MM_DMA_ALLINT_EN | MM_DMA_GO | MM_DMA_RD |
                               MM_DMA_COMP | MM_DMA_CHN_COMP |
                               MM_DMA_PCI_MEM_RD_MULT | MM_DMA_PCI_MEM_WRT_INV;
#else
    pMM->dmaDesc.stsCtrlLo   = MM_DMA_ALLINT_EN | MM_DMA_GO | MM_DMA_RD |
                               MM_DMA_COMP | MM_DMA_CHN_COMP |
                               MM_DMA_PCI_MEM_RD_MULT | MM_DMA_PCI_MEM_WRT;
#endif

    /* Flush the prior register writes out of the bridge & start the DMA */
    MM_FORCE_WRITE_TO_FLUSH(pMM->dmaDesc.stsCtrlLo);

    if (block)
    {
        while (pMM->dmaDesc.stsCtrl.go)
        {
            /*
            ** Loop here waiting for DMA completion.
            ** NOTE:  This needs to be replaced with an interrupt & semaphore.
            */
#ifdef MM_DEBUG
            fprintf(stderr,"mm_DMA_Read:  DMA Status/Control Reg = 0x%X\n",
                    pMM->dmaDesc.stsCtrlLo);
#endif
            if (K_xpcb != NULL)
            {
                /* Kernel is up and running, so switch context. */
                TaskSwitch();
            }
            else
            {
                /* No PCB exists - kernel isn't up & running yet. Sleep 100us. */
                TaskSleepNoSwap(100);
            }
        } /* while GO */

        /* Check for errors */
        if ((err_status = pMM->dmaDesc.stsCtrl.errSts) != 0)
        {
            if (err_status & ~MM_ECC_ANY_ERR)
            {
                fprintf(stderr, "mm_DMA_Read:  DMA completed with status 0x%X\n",
                        err_status);
                /* Surface a fatal event */
                nv_ProcessFatalEvent(NV_FATAL_ECC_MULTI, err_status);
            }
            else
            {
                /* Single-bit ECC error - attempt to fix the error(s) */
                mm_ProcessECCErrors();
            }

            /* Clear the error counter */
            mm_ClearErrorCounter();
            rc = (err_status & ~MM_ECC_ANY_ERR);
        } /* DMA error */

        /* Clear completion bits */
        pMM->dmaDesc.stsCtrl.compSts = MM_DMA_CLR_COMP;

        /* Any read in the area will flush. */
        MM_FORCE_WRITE_TO_FLUSH(pMM->ledCtrl);
    } /* block */

    return rc;
} /* mm_DMA_Read */
#endif /* BACKEND */


#if (defined(STANDALONE) && defined(CHAINED_DMA_TEST_CODE))
/*
******************************************************************************
** mm_Chain_DMA
**
**  @brief  MM-5425CN Micro Memory memory chained DMA transfer implementation.
**
**  A 64 bit PCI Address is passed in as the address of the first DMA
**  descriptor in the chain. The chain should be built as specified on pages
**  39-40 of the MM users guide Rev L.
**
**  Note: It is assumed that the calling function passes in a valid
**  PCI Bus address as the first parameter Desc_Addr. An invalid Desc_Addr value
**  will very likely end up in a segmentation fault after invoking the function.
**  Also parameter Desc_Addr must be quad word aligned.
**  The fourth parameter, block (8 bit value), will result in the function call
**  blocking till the end of the DMA transfer if non-zero.
**  If block equals zero, the function will return immediately without
**  waiting for DMA completion.
**
**  @return 0 if success, non-zero value if failure. In case of failure,
**  the return code is set to the value of bits 11-17 of the control/
**  status register of the DMA engine.
**
******************************************************************************
*/
#ifdef PCI_DMA_ADDR
INT32 mm_Chain_DMA(UINT64 Desc_Addr, UINT8 block)
{
#else /* PCI_DMA_ADDR */
INT32 mm_Chain_DMA(UINT32 sharedDescAddr, UINT8 block)
{
    UINT32 Desc_Addr = LI_GetPhysicalAddr(sharedDescAddr);
#endif /* PCI_DMA_ADDR */
    INT32  rc = 0;
    UINT8  err_status;

    if ((mmInfo == NULL) || ((mmInfo->status & ~NV_STS_DIAG) != NV_STS_GOOD))
    {
#ifdef MM_VERBOSE
        fprintf(stderr, "mm_Chain_DMA:  Card not initialized or is inaccessible!\n");
#endif
        return(-1);
    }

    if (Desc_Addr % 8)
    {
        fprintf(stderr, "mm_Chain_DMA:  Desc_Addr must be quad word aligned\n");
        return(-1);
    }

    memset((void*)&pMM->dmaDesc.pciDataAddr, 0, DMA_CSR_SPACE);
    /* pMM->dmaDesc.pciDataAddr = PCI_Addr;    */
    /* pMM->dmaDesc.localAddr = Local_Addr; */
    pMM->dmaDesc.xferSize = 0;
    pMM->dmaDesc.pciDescAddr = Desc_Addr;
    /* pMM->dmaDesc.pciSemAddr = Sema_Addr; */
    pMM->dmaDesc.stsCtrlLo = MM_DMA_ALLINT_EN | MM_DMA_GO | MM_DMA_CHN_EN |
                             MM_DMA_COMP | MM_DMA_CHN_COMP;
    if (block)
    {
        while (pMM->dmaDesc.stsCtrl.go)
        {
            /*
            ** Loop here waiting for DMA completion.
            ** NOTE:  This needs to be replaced with an interrupt & semaphore.
            */
#ifdef MM_DEBUG
            fprintf(stderr,"mm_Chain_DMA:  DMA Status/Control Reg = 0x%X\n",
                    pMM->dmaDesc.stsCtrlLo);
#endif
            if (K_xpcb != NULL)
            {
                /* Kernel is up and running, so switch context.  */
                TaskSwitch();
            }
            else
            {
                /* No PCB exists - kernel isn't up & running yet. Sleep 100us. */
                TaskSleepNoSwap(100);
            }
        } /* while GO */

        while (!pMM->dmaDesc.stsCtrl.chainComp)
        {
            /*
            ** Loop here waiting for DMA completion.
            ** NOTE: This needs to be replaced with an interrupt & semaphore.
            */
#ifdef MM_DEBUG
            fprintf(stderr,"mm_Chain_DMA:  DMA Status/Control Reg = 0x%X\n",
                    pMM->dmaDesc.stsCtrlLo);
#endif
            if (K_xpcb != NULL)
            {
                /* Kernel is up and running, so switch context. */
                TaskSwitch();
            }
            else
            {
                /* No PCB exists - kernel isn't up & running yet. Sleep 100us. */
                TaskSleepNoSwap(100);
            }
        }

        /* Check for errors */
        if ((err_status = pMM->dmaDesc.stsCtrl.errSts) != 0)
        {
            if (err_status & ~MM_ECC_ANY_ERR)
            {
                fprintf(stderr, "mm_Chain_DMA:  DMA completed with status 0x%X\n",
                        err_status);
                /* Surface a fatal event */
                nv_ProcessFatalEvent(NV_FATAL_ECC_MULTI, err_status);
            }
            else
            {
                /* Single-bit ECC error - attempt to fix the error(s) */
                mm_ProcessECCErrors();
            }

            /* Clear the error counter */
            mm_ClearErrorCounter();
            rc = (err_status & ~MM_ECC_ANY_ERR);
        } /* DMA error */

        /* Clear completion bits */
        pMM->dmaDesc.stsCtrl.compSts = MM_DMA_CLR_COMP;

    } /* block */

    return rc;
} /* mm_Chain_DMA */
#endif /* STANDALONE && CHAINED_DMA_TEST_CODE */


#ifdef FRONTEND
/*
******************************************************************************
**  mm_DMA
**
**  @brief  MicroMemory DMA support routine
**
**  This function sets up the MicroMemory DMA engine using the DMA descriptor
**  passed as a parameter. The descriptor can specify additional descriptors
**  which are chained together.
**
**  @param  MM_DMA_DESC *   - pointer to DMA descriptor(s)
**
**  @return INT32           - status
**
******************************************************************************
**/
INT32 mm_DMA(MM_DMA_DESC *pDesc, bool block)
{
    INT32 rc = 0;
    UINT8 err_status;

    if ((mmInfo == NULL) || ((mmInfo->status & ~NV_STS_DIAG) != NV_STS_GOOD))
    {
#ifdef MM_VERBOSE
        fprintf(stderr, "mm_DMA(%p, block = %x):  Card not initialized or is inaccessible!\n",
                pDesc, block);
#endif
        return(-1);
    }

#if (defined(FRONTEND) && defined(NV_DMA_DRVR))
    /*
    ** Check whether we're to inject a single-bit ECC error on this read.
    ** NOTE:  This will also corrupt the 8 data bytes!
    */
    if ((gNVDMAInjECCErr) && !pDesc->stsCtrl.direction)
    {
        pMM->errDiag.data = 0;
        pMM->errDiag.addrLo = pDesc->localAddr;
        pMM->errDiag.chkBits = 0x04;
        MM_FORCE_WRITE_TO_FLUSH(pMM->errDiag.chkBits);
        gNVDMAInjECCErr = FALSE;
    }
#endif /* FRONTEND && NV_DMA_DRVR */

    /*
    ** Copy the fields from the first descriptor into the hardware.
    ** (Setup is in the NV_DMARequest(...) function.)
    */
    pMM->dmaDesc.pciDataAddr    = pDesc->pciDataAddr;
    pMM->dmaDesc.pciDataAddrHi  = 0;
    pMM->dmaDesc.localAddr      = pDesc->localAddr;
    pMM->dmaDesc.localAddrHi    = 0;
    pMM->dmaDesc.xferSize       = pDesc->xferSize;
    pMM->dmaDesc.pciDescAddr    = pDesc->pciDescAddr;
    pMM->dmaDesc.pciDescAddrHi  = 0;
    pMM->dmaDesc.pciSemAddr     = pDesc->pciSemAddr;
    pMM->dmaDesc.pciSemAddrHi   = 0;

    if ((pDesc->localAddr < SIZE_16MEG) && (pDesc->stsCtrl.direction == 1)) abort(); /* abort if DMA write attempted in window */
    if ((pDesc->localAddr + pDesc->xferSize) > mem_size) abort(); /* abort if DMA attempted beyond range */
    /*
    ** Set the control/status register. This will start the DMA, since the
    ** GO bit should be set in the passed parameter.
    */
    pMM->dmaDesc.stsCtrlLo = pDesc->stsCtrlLo;

    /* Flush the prior register writes out of the bridge & start the DMA */
    MM_FORCE_WRITE_TO_FLUSH(pMM->dmaDesc.stsCtrlLo);

    if (block)
    {
        while (pMM->dmaDesc.stsCtrl.go)
        {
            /* Block requested - loop here waiting for DMA completion. */
#ifdef MM_DEBUG
            fprintf(stderr,"mm_DMA:  DMA Status/Control Reg = 0x%X\n",
                    pMM->dmaDesc.stsCtrlLo);
#endif
            if (K_xpcb != NULL)
            {
                /* Kernel is up and running, so switch context. */
                TaskSwitch();
            }
            else
            {
                /* No PCB exists - kernel isn't up & running yet. Sleep 100us. */
                TaskSleepNoSwap(100);
            }
        } /* while GO */

        /* Check for errors */
        if ((err_status = pMM->dmaDesc.stsCtrl.errSts) != 0)
        {
            if (err_status & ~MM_ECC_ANY_ERR)
            {
                fprintf(stderr, "mm_DMA:  DMA completed with status 0x%X\n",
                        err_status);

                /* Surface a fatal event */
                nv_ProcessFatalEvent(NV_FATAL_ECC_MULTI, err_status);
            }
            else
            {
                /* Single-bit ECC error - attempt to fix the error(s) */
                mm_ProcessECCErrors();
            }

            /* Clear the error counter */
            mm_ClearErrorCounter();
            rc = (err_status & ~MM_ECC_ANY_ERR);
        } /* DMA error */

        /* Clear completion bits */
        pMM->dmaDesc.stsCtrl.compSts = MM_DMA_CLR_COMP;

        /* Any read in the area will flush. */
        MM_FORCE_WRITE_TO_FLUSH(pMM->ledCtrl);

    } /* block */

    return rc;
} /* mm_DMA */
#endif /* FRONTEND */


#if (defined(FRONTEND) && defined(NV_DMA_DRVR))
void dummyCompFn(NV_DMA_RSP_PKT *pRsp);
/*
******************************************************************************
**  dummyCompFn
**
**  @brief  Completion Function for DMA Test Driver use only
**
**  @param  NV_DMA_RSP_PKT *pRsp - DMA Response Packet Pointer
**
**  @return None
**
******************************************************************************
**/
void dummyCompFn(NV_DMA_RSP_PKT *pRsp)
{
    UINT32 *pBuffer = (UINT32*)pRsp->pILT;
    INT32  i, j;
    if (pRsp->dmaStsCtrl.errSts != 0)
    {
        fprintf(stderr, "@@@@@@@@@@ DMA COMPLETION FUNCTION (error status = 0x%X) @@@@@@@@@@\n",
                pRsp->dmaStsCtrl.errSts);
    }
    else
    {
        /* On GOOD completion of chained descriptors, compare the buffers */
        if (pBuffer[1] == 0xCCCCCCCC)
        {
            for (i = 0, j = SIZE_16K/4; i < SIZE_16K/4; ++i, ++j)
            {
                if (pBuffer[i] != pBuffer[j])
                {
                    fprintf(stderr, "X X X X X X X X   M I S C O M P A R E   X X X X X X X X\n"
                                    "X X X X X X  pBuffer (word 0) = 0x%08X  X X X X X X\n"
                                    "X X X X X X  Offset = 0x%04X                X X X X X X\n"
                                    "X X X X X X  [0x%08X] != [0x%08X]   X X X X X X\n"
                                    "X X X X X X X X X X X X X X X X X X X X X X X X X X X X\n",
                            pBuffer[0], i, pBuffer[i], pBuffer[j]);
                    fprintf(stderr, "dummyCompFn:  pBuffer = %p\n", pBuffer);
                    break;
                }
            }
        } /* chained descriptors, data compare indicated */
    }

    if (pBuffer[1] == 0xCCCCCCCC)
    {
        /* Free the buffers */
        s_Free(pBuffer, 2*SIZE_16K, __FILE__, __LINE__);
    }

    /* Free the response packet */
    s_Free(pRsp, sizeof(*pRsp), __FILE__, __LINE__);
}

void nv_DMATestDriver(void);
/*
******************************************************************************
**  nv_DMATestDriver
**
**  @brief  Test driver for DMA handling
**
**  @param  None
**
**  @return None
**
******************************************************************************
**/
NORETURN
void nv_DMATestDriver(void)
{
    NV_DMA_DD   *pDD;
    NV_DMA_DD   *pNextDD;
    UINT32      *pBuffer;
    INT32       i;
    UINT8       pattern = 0;
    UINT32      *pBuffer2;
    bool        wrtNotRd = 0;
#ifdef MM_VERBOSE
    UINT16      counter = 0;
    REG64       tscBegin, tscEnd, tscDelta_uS;
#endif /* MM_VERBOSE */

#ifdef MM_VERBOSE
    fprintf(stderr, "nv_DMATestDriver:  Alive, PCB* = %p\n", K_xpcb);
#endif
    /* Sleep for 30 seconds before beginning */
    TaskSleepMS(30000);

    /* Sleep until enabled */
    while (!gNVDMAExecEn)
    {
        TaskSleepMS(1000);
    }

    pDD     = s_MallocW(sizeof(*pDD), __FILE__, __LINE__);
    pNextDD = s_MallocW(sizeof(*pNextDD), __FILE__, __LINE__);
    pBuffer2= s_MallocC(SIZE_32K, __FILE__, __LINE__);
    fprintf(stderr, "nv_DMATestDriver:  Beginning request generation...\n");

#ifdef MM_VERBOSE
    tscBegin.dw = get_tsc();
#endif /* MM_VERBOSE */

    do
    {
        /* Set up buffers (one buffer cut into halves) & pattern for write */
        pBuffer = (UINT32*)s_MallocW(2*SIZE_16K, __FILE__, __LINE__);
        for (i = 0; i < SIZE_16K/4; ++i)
        {
            pBuffer[i] = (const UINT32)testPattern[pattern];
        }
        pBuffer[0] = (UINT32)pBuffer; /* save address too */
        pBuffer[1] = 0xCCCCCCCC; /* indicate data compare needed */

        /* Generate another chained (W + R) DMA request */
        pDD->pNextDD  = pNextDD;
        pDD->sysAddr  = (UINT32)pBuffer;
        pDD->nvAddr   = SIZE_512MEG;                    /* Midpoint of card */
        pDD->xferSize = SIZE_16K;
        pDD->wrtNotRd = (wrtNotRd = !wrtNotRd);         /* flip direction */

        pNextDD->pNextDD  = 0;
        pNextDD->sysAddr  = (UINT32)pBuffer + SIZE_16K; /* Midpoint of buffer */
        pNextDD->nvAddr   = SIZE_512MEG;                /* Midpoint of card */
        pNextDD->xferSize = SIZE_16K;
        pNextDD->wrtNotRd = (wrtNotRd = !wrtNotRd);     /* flip direction */

        if (NV_DMARequest(pDD, dummyCompFn, (ILT*)pBuffer) != 0)
        {
            /* Free the buffers & response packet */
            s_Free(pBuffer, 2*SIZE_16K, __FILE__, __LINE__);

            /* Sleep 1 second before continuing to see if the "issue" clears up */
            TaskSleepMS(1000);
        }

        /* Generate non-chained write and read requests */
        pDD->pNextDD  = 0;
        pDD->sysAddr  = (UINT32)pBuffer2;
        pDD->nvAddr   = SIZE_512MEG + SIZE_32K;          /* Midpoint of card */
        pDD->xferSize = SIZE_32K;
        pDD->wrtNotRd = (wrtNotRd = !wrtNotRd); /* flip direction */
        if (NV_DMARequest(pDD, dummyCompFn, (ILT*)pBuffer2) != 0)
        {
            /*
            ** Sleep 1 second before continuing to see if the "issue" clears up
            */
            TaskSleepMS(1000);
        }

        pDD->pNextDD  = 0;
        pDD->sysAddr  = (UINT32)pBuffer2;
        pDD->nvAddr   = SIZE_512MEG + SIZE_32K;          /* Midpoint of card */
        pDD->xferSize = SIZE_32K;
        pDD->wrtNotRd = (wrtNotRd = !wrtNotRd);         /* flip direction */
        if (NV_DMARequest(pDD, dummyCompFn, (ILT*)pBuffer2) != 0)
        {
            /* Sleep 1 second before continuing to see if the "issue" clears up */
            TaskSleepMS(1000);
        }

#ifdef MM_VERBOSE
        counter += 4;
        if (counter == 0)
        {
            tscEnd.dw = get_tsc();
            tscDelta_uS.dw = (tscEnd.dw - tscBegin.dw)/3200000;
            fprintf(stderr, "nv_DMATestDriver:  Another 64k requests submitted!  "
                    "Time delta = %dms\n", tscDelta_uS.lo);
            tscBegin.dw = get_tsc();
        }
#endif /* MM_VERBOSE */

        /* Increment pattern for next pass */
        if (++pattern >= TEST_PATTERN_BOUNDS)
        {
            pattern = 0;
        }

        /* If we were disabled, sleep until enabled */
        while (!gNVDMAExecEn)
        {
            TaskSleepMS(1000);
        }
    } while (TRUE);
} /* nv_DMATestDriver */
#endif /* FRONTEND && NV_DMA_DRVR */


#ifdef FRONTEND
void nv_DMAExec(void);
/*
******************************************************************************
**  nv_DMAExec
**
**  @brief  Executor for initiating DMA requests
**
**  This function is spawned during startup as a separate task; it then waits
**  to be awakened (after the arrival of new work on the queue). Upon waking,
**  the next enqueued DMA request will be initiated with the MicroMemory card.
**  This task then goes back to sleep, until another item is ready to process.
**
**  @param  None
**
**  @return None
**
******************************************************************************
**/
NORETURN
void nv_DMAExec(void)
{
#ifdef MM_VERBOSE
    fprintf(stderr, "NV Memory DMA Executor is alive - PCB* = %p\n", K_xpcb);
#endif

    do
    {
        /* Handle next item on queue (at "nextSub") if no active DMA, else sleep */
        while ((pDMAQueue->nextSub != pDMAQueue->in) && (pDMAQueue->active == NULL))
        {
            /*
            ** Submit the request to the hardware.
            ** If submission was successful, advance the queue pointers.
            */
            if (mm_DMA(&pDMAQueue->nextSub->dmaDesc, FALSE) != -1)
            {
                pDMAQueue->active = pDMAQueue->nextSub;

                /* Advance the "nextSub" pointer */
                if (&pDMAQueue->nextSub[1] != pDMAQueue->end)
                {
                    /* no wrap - advance "nextSub" pointer */
                    ++pDMAQueue->nextSub;
                }
                else
                {
                    /* wrap */
                    pDMAQueue->nextSub = pDMAQueue->begin;
                }
                continue;
            }
            else
            {
                fprintf(stderr, "nv_DMAExec:  mm_DMA next request submission (= %p) FAILED!\n",
                        pDMAQueue->nextSub);
            }
        }

        /* Sleep until awakened by arrival of new work */
        TaskSetMyState(PCB_NV_DMA_EXEC_WAIT);
        TaskSwitch();

        /* We have been awakened by the arrival of new work.  */
    } while (TRUE);
} /* nv_DMAExec */
#endif /* FRONTEND */


#ifdef FRONTEND
void nv_DMAComplete(void);
/*
******************************************************************************
**  nv_DMAExec
**
**  @brief  Executor for initiating DMA requests
**
**  This function is spawned during startup as a separate task; it then waits
**  to be awakened (after the arrival of new work on the queue). Upon waking,
**  the next enqueued DMA request will be initiated with the MicroMemory card.
**  This task then goes back to sleep, until another item is ready to process.
**
**  @param  None
**
**  @return None
**
******************************************************************************
**/
NORETURN
void nv_DMAComplete(void)
{
    NV_DMA_REQ  *pReq;
    MM_DMA_DESC *pDesc;
    UINT32       nextPCIDesc;
    UINT8        compSts;
    UINT32       tmp_addr;

#ifdef MM_VERBOSE
    fprintf(stderr, "NV Memory DMA Completor is alive - PCB* = %p\n", K_xpcb);
#endif

    do
    {
        /*
        ** Handle next completed item on queue (at "out"), else go back to sleep
        ** if queue is empty or a completed request is otherwise not found.
        */
        while (pDMAQueue->in != pDMAQueue->out)
        {
            /* Get the next completed MM request from the queue */
            pReq = pDMAQueue->out;

            /* Check for a completed request, or bad MM card status */
            if (((compSts = pReq->dmaStsCtrl.compSts) != 0) ||
                ((mmInfo->status & ~NV_STS_DIAG) != NV_STS_GOOD))
            {
                /* If completed request, copy completion status to response pkt */
                if (compSts != 0)
                {
#ifdef NV_DMA_DRVR
                    if (compSts != 3)
                        fprintf(stderr, "nv_DMAComplete:  compSts = 0x%X\n", compSts);
#endif
                    /*
                    ** Set error status results in the response packet, then
                    ** clear the field in the request packet
                    */
                    pReq->pRspPkt->dmaStsCtrl.errSts = pReq->dmaStsCtrl.errSts;
                    pReq->dmaStsCtrl.stsCtrl = 0x00F00000;
                }
                else
                {
                    /* MM card status is bad or unavailable at the moment */
                    pReq->pRspPkt->dmaStsCtrl.stsCtrl = 0xFFFFFFFF;
#ifdef NV_DMA_DRVR
                    fprintf(stderr, "nv_DMAComplete:  set compSts = 0xFFFFFFFF in req = %p\n", pReq);
#endif
                    if (pReq == pDMAQueue->nextSub)
                    {
                        /*
                        ** Advance the "nextSub" pointer. This is a case where
                        ** the "out" pointer has passed the "nextSub" pointer.
                        ** NOTE:  "out" should never pass up the "in" pointer.
                        */
                        if (&pDMAQueue->nextSub[1] != pDMAQueue->end)
                        {
                            /* no wrap - advance "nextSub" pointer */
                            ++pDMAQueue->nextSub;
                        }
                        else
                        {
                            /* wrap */
                            pDMAQueue->nextSub = pDMAQueue->begin;
                        }
                    }
                }

                /* Call the completion routine w/ rsp pkt as parameter */
                (*pReq->pCompFn)(pReq->pRspPkt);

                /*
                ** Free the descriptors in the chain, if any, which were
                ** allocated by NV_DMARequest(...)
                */
                nextPCIDesc = pReq->dmaDesc.pciDescAddr;
                while (nextPCIDesc != 0)
                {
                    /*
                    ** Convert the address from PCI back to a system address,
                    ** and get the next link in the chain.
                    */
                    tmp_addr = LI_GetSharedAddr(nextPCIDesc);
                    pDesc = (MM_DMA_DESC *)tmp_addr;
                    nextPCIDesc = pDesc->pciDescAddr;

                    /* Free the storage for the descriptor */
                    s_Free(pDesc, sizeof(*pDesc), __FILE__, __LINE__);
                }

                /* Advance the out pointer, checking for wrap */
                if (&pDMAQueue->out[1] != pDMAQueue->end)
                {
                    /* no wrap - advance "out" pointer */
                    ++pDMAQueue->out;
                }
                else
                {
                    /* wrap */
                    pDMAQueue->out = pDMAQueue->begin;
                }

                /*
                ** Clear the active request, so the next pending request
                ** can be started
                */
                if (pDMAQueue->active == pReq)
                {
                    pDMAQueue->active = NULL;
                }
                else if (pDMAQueue->active != NULL)
                {
                    fprintf(stderr, "nv_DMAComplete:  WARNING - active request "
                            "(%p) doesn't match completed request (%p)\n",
                            pDMAQueue->active, pReq);
                }

                /*
                ** If due to priority reordering (round robin) the DMA
                ** Exec executed before this exec, may need to wake it up
                ** if it is asleep and work is to be done.
                */
                if ((pDMAQueue->nextSub != pDMAQueue->in) &&
                    (TaskGetState(pNV_DMAExecPCB) == PCB_NV_DMA_EXEC_WAIT))
                {
#ifdef HISTORY_KEEP
CT_history_pcb("MM_Interrupt setting ready pcb", (UINT32)pNV_DMAExecPCB);
#endif
                    TaskSetState(pNV_DMAExecPCB, PCB_READY);
                }

                /* Awaken any task(s) waiting on DMA requests to free up */
                TaskReadyByState(PCB_NV_DMA_QFULL_WAIT);
            }
            else
            {
                break;
            }
        } /* Queue not empty */

        /* Sleep until awakened by arrival of new work */
        TaskSetMyState(PCB_NV_DMA_COMP_WAIT);
        TaskSwitch();

        /* We have been awakened by the completion of a request. */
    } while (TRUE);
} /* nv_DMAComplete */
#endif /* FRONTEND */


#ifdef FRONTEND
/*
******************************************************************************
**  nv_SetupDescriptor
**
**  @brief  Subroutine to fill out an MM descriptor using a user DD list entry
**
**  @param  MM_DMA_DESC *   - pointer to MM descriptor to be setup
**  @param  NV_DMA_DD *     - pointer to user DD list entry
**
**  @return None
**
******************************************************************************
**/
void nv_SetupDescriptor(MM_DMA_DESC *pDesc, NV_DMA_DD *pDDList)
{
    pDesc->pciDataAddr      = LI_GetPhysicalAddr(pDDList->sysAddr);
    pDesc->localAddr        = pDDList->nvAddr;
    pDesc->xferSize         = pDDList->xferSize;
    pDesc->pciSemAddr       = LI_GetPhysicalAddr((UINT32)&pDesc->pciSemAddr);
    pDesc->stsCtrlLo        = MM_DMA_GO |
                              MM_DMA_CHN_EN |
                              MM_DMA_SEM_EN |
                              MM_DMA_ALLINT_EN |
                              MM_DMA_COMP |
                              MM_DMA_CHN_COMP |
                              MM_DMA_PCI_MEM_RD_MULT;
    pDesc->stsCtrl.dmaCompIntEn = 0;
    pDesc->stsCtrl.direction= pDDList->wrtNotRd;
#ifdef MEM_WRITE_INVALIDATE
    pDesc->stsCtrl.wrtCmd   = 0xF;
#else
    pDesc->stsCtrl.wrtCmd   = 7;
#endif
    if ((pDesc->localAddr < SIZE_16MEG) && (pDesc->stsCtrl.direction == 1))
    {
        abort(); /* abort if DMA write attempted in window */
    }
    if ((pDesc->localAddr + pDesc->xferSize) > mem_size)
    {
        abort(); /* abort if DMA attempted beyond range */
    }
} /* nv_SetupDescriptor */


/*
******************************************************************************
**  NV_DMARequest
**
**  @brief  External I/F for submission of an NV Memory DMA request
**
**  The "handler" portion of this routine could be split into a separate
**  function in the presence of a Link Layer protocol, where this routine would
**  provide the setup & allocation of request/response packets, which are then
**  passed to the handler, running on a separate process, via the Link Layer.
**
**  @param  NV_DMA_DD *                       - pointer to DD list
**  @param  void (*pCompFn)(NV_DMA_RSP_PKT *) - pointer to completion function,
**                                  prototype:  void pCompFn(NV_DMA_RSP_PKT *);
**  @param  ILT *                             - pointer to ILT (handle)
**
**  @return INT32 - integer value, 0 = GOOD
**
******************************************************************************
**/
INT32 NV_DMARequest(NV_DMA_DD *pDDList, void (*pCompFn)(NV_DMA_RSP_PKT *pRsp), ILT *pILT)
{
    NV_DMA_REQ  *pReq;
    MM_DMA_DESC *pDesc;
    MM_DMA_DESC *pPrevDesc;
    bool        qFull = FALSE;

    /* Check whether MicroMemory card & driver are available & accessible. */
    if (mmInfo == NULL)
    {
#ifdef MM_VERBOSE
        fprintf(stderr, "NV_DMARequest(%p, %p, %p):  Card not initialized "
                "or is inaccessible!\n", pDDList, pCompFn, pILT);
#endif
#ifdef BACKEND
        return(-1);
#else /* FRONTEND */
        /*
         * Set task status to wait for MM Info MRP to be received, then
         * give up CPU until re-awakened.
         */
        TaskSetMyState(PCB_MM_WAIT);
        TaskSwitch();
        fprintf(stderr, "NV_DMARequest(%p, %p, %p):  Awakened from MM_WAIT state.\n",
                pDDList, pCompFn, pILT);
#endif
    }

    if ((mmInfo->status & ~NV_STS_DIAG) != NV_STS_GOOD)
    {
#ifdef MM_VERBOSE
        fprintf(stderr, "NV_DMARequest:  Card is inaccessible due to bad status = %d\n",
                mmInfo->status);
#endif
        return(-1);
    }

    /* Check for queue full condition */
    do
    {
        if (qFull)
        {
            /* Queue Full condition detected - sleep until entries free up */
            TaskSetMyState(PCB_NV_DMA_QFULL_WAIT);
            TaskSwitch();

            /* We've been re-awakened. Clear the flag & check for free entries */
            qFull = FALSE;
        }
        if (&pDMAQueue->in[1] != pDMAQueue->end)
        {
            if (&pDMAQueue->in[1] == pDMAQueue->out)
            {
                /* queue full */
                qFull = TRUE;
            }
        }
        else if (pDMAQueue->begin == pDMAQueue->out)
        {
            /* queue full */
            qFull = TRUE;
        }
    } while (qFull);


    /* Get a request from the queue and advance the "in" ptr, checking for wrap */
    pReq = pDMAQueue->in;

    if (&pDMAQueue->in[1] != pDMAQueue->end)
    {
        /* no wrap - advance "in" pointer */
        ++pDMAQueue->in;
    }
    else
    {
        /* wrap */
        pDMAQueue->in = pDMAQueue->begin;
    }

    /*
    ** Walk/convert the DD list and create a descriptor list.
    ** Set the semaphore addr to overwrite the semaphore addr field itself,
    ** and setup the status / control bits, including setting the GO bit.
    */
    pDesc = &pReq->dmaDesc;
    nv_SetupDescriptor(pDesc, pDDList);

    while ((pDDList = pDDList->pNextDD) != NULL)
    {
        /* Preserve previous descriptor pointer, for linking */
        pPrevDesc = pDesc;

        /* Get space to create the next MM descriptor, and link with previous */
        pDesc = s_MallocC(sizeof(*pDesc), __FILE__, __LINE__);
        pPrevDesc->pciDescAddr  = LI_GetPhysicalAddr((UINT32)pDesc);

        /* Setup this descriptor, as above */
        nv_SetupDescriptor(pDesc, pDDList);
    } /* while walking DD list */

    /*
    ** Disable chaining in the last descriptor to mark the end of the chain,
    ** and ensure the chain/link pointer is cleared also.
    */
    pDesc->stsCtrl.chainEn = 0;
    pDesc->pciDescAddr = 0;

    /*
    ** Set the semaphore in the last descriptor in the chain to point to the
    ** Status/Control field in the request itself (and not within the MM desc).
    ** Ensure the Status / Control field is initially cleared also.
    */
    pReq->dmaStsCtrl.stsCtrl = 0x00B00000;
    pDesc->pciSemAddr = LI_GetPhysicalAddr((UINT32)&pReq->dmaStsCtrl);

    /* Preserve the completion function pointer */
    pReq->pCompFn = pCompFn;

    /*
    ** Create space for the response packet, and save the ILT* in there.
    ** NOTE:  Completion function needs to free this space!
    */
    pReq->pRspPkt = s_MallocC(sizeof(*pReq->pRspPkt), __FILE__, __LINE__);
    pReq->pRspPkt->pILT = pILT;

    /* Awaken the DMA Executor, if necessary */
    if (TaskGetState(pNV_DMAExecPCB) == PCB_NV_DMA_EXEC_WAIT)
    {
#ifdef HISTORY_KEEP
CT_history_pcb("MM_Interrupt setting ready pcb", (UINT32)pNV_DMAExecPCB);
#endif
        TaskSetState(pNV_DMAExecPCB, PCB_READY);
    }

    return EC_OK;
} /* NV_DMARequest */
#endif /* FRONTEND */


#ifdef FRONTEND
/*
******************************************************************************
**  bcd_to_bin
**
**  @brief  Subroutine to convert a bcd-encoded value to its integer equivalent
**
**  NOTE:  This should either be moved to the Shared file XIO_Std, or replaced
**         with an equivalent function which may already exist.
**
**  @param  UINT32  - bcd-encoded value
**
**  @return UINT32  - integer value
**
******************************************************************************
**/
UINT32 bcd_to_bin(UINT32 val)
{
    return ((val & 0x000f) +
            ((val & 0x00f0) >> 4) * 10 +
            ((val & 0x0f00) >> 8) * 100 +
            ((val & 0xf000) >> 12) * 1000);
} /* bcd_to_bin */
#endif


#ifdef FRONTEND
/*
******************************************************************************
** MM_ProcessMRP
**
**  @brief  This is called from deffe.c when the front end receives MRP
**  MRMMCARDGETBATTERYSTATUS from CCB to FE.
**
**  This function queries the MM card and reads the status registers
**  to get battery/board status. Also executes a command to put the
**  MM card in low power standby mode.
**
**  @param  value - 32 bit command passed in MRP
**  MRMMCARDGETBATTERYSTATUS_REQ, pointer to Response data in MR_PKT.
**
**  @return returns completion codes as given in MR_Defs.h
**
******************************************************************************
**/
UINT32 MM_ProcessMRP(UINT32 command, MRMMCARDGETBATTERYSTATUS_RSP *pMMRsp)
{
    NVRAM_BOARD_INFO*   pInfo = NULL;
    UINT8               i;

    if (!pMMRsp)
    {
        return DEINVPKTTYP;
    }

    if (mmInfo == NULL)
    {
        fprintf(stderr, "MM_ProcessMRP:  Card not initialized or is inaccessible!\n");
        pMMRsp->header.status = DEINOPDEV;
        pMMRsp->header.len    = sizeof(MR_HDR_RSP);
        return(DEINOPDEV);
    }

    if (pMM != NULL)
    {
        pInfo = &pMMRsp->boardInfo;

        /*
        ** If the MM monitor has not yet been started set the response to all
        ** zeros, this will give an UNKNOWN state back in the MRP.
        **
        ** Otherwise, copy the cached board information into the MRP response.
        */
        if (pMMMonitorPCB == NULL)
        {
            memset(pInfo, 0x00, sizeof(NVRAM_BOARD_INFO));
        }
        else
        {
            memcpy(pInfo, &gNVBoardInfo, sizeof(NVRAM_BOARD_INFO));
        }

        pMMRsp->header.status     = DEOK;
        pMMRsp->header.len        = sizeof(*pMMRsp);

        switch(command)
        {
            case NV_CMD_SHUTDOWN:
                /* Check whether the controller S/N in NV memory mismatches our S/N. */
                if (mmInfo->status != NV_STS_SN_MISMATCH)
                {
                    /* Clear the Memory Valid and Memory Initialized bits (non-volatile) */
                    pMM->ledCtrl &= MM_LED_CTRL_MASK;

                    /* Disable the batteries - must cycle +5V to re-enable */
                    fprintf(stderr, "Shipping power down and disabling batteries\n");
                    pMM->shipPwr = MM_SHIP_PWR_DN;
                    for (i = 0; i < pInfo->batteryCount; i++)
                    {
                        /* return status of disabled by s/w */
                        pInfo->batteryInformation[i].status = NVRAM_BATTERY_INFO_STATUS_DISABLED_SW;
                    }

                    /* Set status to Shutdown */
                    pInfo->boardStatus = NV_STS_SHUTDOWN;
                    mmInfo->status = NV_STS_SHUTDOWN;

                    /* Turn on OK-to-remove LED */
                    pMM->ledCtrlBits.okToRmv = MM_LED_RMV_ON;
                    MM_FORCE_WRITE_TO_FLUSH(pMM->ledCtrl);
                }
                else
                {
                    /*
                    ** This board belongs to a different controller, and its
                    ** memory contents were found to be valid, so DO NOT disable
                    ** the batteries!
                    **
                    ** Flash the OK-to-remove LED to identify this case.
                    */
                    pMM->ledCtrlBits.okToRmv = MM_LED_FLASH_7HZ;
                    MM_FORCE_WRITE_TO_FLUSH(pMM->ledCtrl);

                    /* Do we want to set bad pMMRsp->header.status ? */
                }
                break;

            case NV_CMD_INFO:
                /* If controller S/N in NV memory mismatches our S/N, set bad sts. */
                if (mmInfo->status == NV_STS_SN_MISMATCH)
                {
                    pInfo->boardStatus = NV_STS_SN_MISMATCH;
                }
                break;

            case NV_CMD_CLEAR:
                /* Clear the Memory Valid and Memory Initialized bits (non-volatile) */
                pMM->ledCtrl &= MM_LED_CTRL_MASK;
                MM_FORCE_WRITE_TO_FLUSH(pMM->ledCtrl);
                break;

            default:
                /* illegal command code */
                fprintf(stderr, "In MM_ProcessMRP(), received an illegal command code = %d\n", command);
                pMMRsp->header.status = DEINVPKTTYP;
                pMMRsp->header.len    = sizeof(MR_HDR_RSP);
        }
    } /* pMM != NULL */
    else
    {
        /* No operational card exists */
        pMMRsp->boardInfo.boardStatus = mmInfo->status;
        pMMRsp->header.status = DEOK;
        pMMRsp->header.len = sizeof(*pMMRsp);
    }

    return pMMRsp->header.status;
} /* MM_ProcessMRP */
#endif


#ifdef FRONTEND
/**
******************************************************************************
**
**  @brief      Monitors the MicroMemory card battery status and determines
**              when the battery/card should be declared good and/or bad.
**
**              This code also monitors the board to see if it passes a
**              readiness check within 5 hours of starting. If it does
**              not pass this check...
**
**  @param      UINT32 dummy1 - Dummy parameter required for fork.
**
**  @param      UINT32 dummy2 - Dummy parameter required for fork.
**
**  @return     none
**
**  @attention  none
**
******************************************************************************
**/
NORETURN
void MM_MonitorTask(
    UINT32                          dummy1 UNUSED,
    UINT32                          dummy2 UNUSED)
{
    NVRAM_BOARD_INFO    biNew;
    UINT8               bCheckBoard = true;
    UINT32              timerEnd;

    /*
    ** Get the current time and add our board ready threshold to it.
    ** This will be used to detect if the board does not become ready
    ** within the period of time.
    */
    timerEnd = K_ii.time + MM_CHARGE_THRESHOLD;

    /* Initialize the board information structures to zero. */
    memset(&gNVBoardInfo, 0x00, sizeof(NVRAM_BOARD_INFO));
    memset(&biNew, 0x00, sizeof(NVRAM_BOARD_INFO));

    while (FOREVER)
    {
        /* The monitor code will delay for 5 seconds before continuing. */
        TaskSleepMS(5000);

        /*
        ** If the board has not passed its initial readiness test it
        ** needs to be checked again to see if has not come ready
        ** within our time threshold.
        */
        if (bCheckBoard && K_ii.time > timerEnd)
        {
            fprintf(stderr, "MM_MonitorTask: Battery readiness check failed!\n");

            /* Fatal Event - Failed to Charge */
            nv_ProcessFatalEvent(NV_FATAL_BATT, 0);

            /*
            ** The board has been logged as readiness failed so stop
            ** checking it (we don't want to log more than once).
            */
            bCheckBoard = false;
        }

        /* Get updated board information */
        MM_MonitorGatherData(&biNew);

        /*
        ** Analyze the new data to see if a log event is required and if
        ** the write cache battery health needs to be updated.
        **
        ** This function will also update the cached value with the
        ** information in the new board information.
        */
        MM_MonitorAnalyze(&gNVBoardInfo, &biNew);

        /*
        ** Check if the board has not yet passed its readiness test
        ** and it is now ready.
        */
        if (bCheckBoard)
        {
            /*
            ** Update the check board flag based on the current readiness
            ** of the board.
            */
            bCheckBoard = MM_MonitorCheckBoardReadiness();
        }
    }
}
#endif


#ifdef FRONTEND
/**
******************************************************************************
**
**  @brief      Updates the information in the given pointer to a board
**              information with the current state of the MM board.
**
**  @param      NVRAM_BOARD_INFO* pInfo - pointer to a board info structure
**                                        to receive the updated information.
**
**  @return     none
**
**  @attention  none
**
******************************************************************************
**/
void MM_MonitorGatherData(NVRAM_BOARD_INFO* pInfo)
{
    UINT8   battery_status;
    UINT16  battery_charge[NVRAM_BOARD_MAX_BATTERIES];
    UINT16  battery_voltage[NVRAM_BOARD_MAX_BATTERIES];
    UINT16  cur_batteryVolt;
    UINT8   ch;
    static  int limit_messages = 0;

    /* Get a local copy of the battery status */
    battery_status = pMM->battSts & 0x0f;

    /* Check for battery failure */
    if (!(battery_status & (MM_BATT_1_FAIL_MASK | MM_BATT_2_FAIL_MASK)))
    {
        /* Both batteries are good. Set good status for board and batteries. */
        pInfo->boardStatus = NV_STS_GOOD;
        pInfo->batteryInformation[0].status = NVRAM_BATTERY_INFO_STATUS_GOOD;
        pInfo->batteryInformation[1].status = NVRAM_BATTERY_INFO_STATUS_GOOD;
    }
    else
    {
        /*
        ** At least one battery is bad - set Low Battery status.
        ** Identify which batteries are bad.
        */
        pInfo->boardStatus = NV_STS_LOW_BATT;
        if (battery_status & MM_BATT_1_FAIL_MASK)
        {
            pInfo->batteryInformation[0].status = NVRAM_BATTERY_INFO_STATUS_FAILURE;
        }
        else
        {
            pInfo->batteryInformation[0].status = NVRAM_BATTERY_INFO_STATUS_GOOD;
        }

        if(battery_status & MM_BATT_2_FAIL_MASK)
        {
            pInfo->batteryInformation[1].status = NVRAM_BATTERY_INFO_STATUS_FAILURE;
        }
        else
        {
            pInfo->batteryInformation[1].status = NVRAM_BATTERY_INFO_STATUS_GOOD;
        }
    }

    cur_batteryVolt = pMM->battery[0].voltage;
    for (ch = 0; (ch < NVRAM_BOARD_MAX_BATTERIES) && (cur_batteryVolt != MM_BATT_END_MRK); ++ch)
    {
        battery_charge[ch] = pMM->battery[ch].charge;
        battery_voltage[ch] = pMM->battery[ch].voltage;

        /* Check for battery disabled by jumper */
        if (battery_voltage[ch] == MM_BATT_DIS_JMP)
        {
            limit_messages++;
            if (limit_messages < 128 || (limit_messages & 0xff) == 0)
            {
                fprintf(stderr, "MM_MonitorGatherData: Battery %d disabled by jumper, or fuse open.\n", ch);
            }

            pInfo->batteryInformation[ch].status = NVRAM_BATTERY_INFO_STATUS_DISABLED_HW;
            pInfo->boardStatus = NV_STS_SHUTDOWN;
            mmInfo->status = NV_STS_SHUTDOWN;
            pInfo->batteryInformation[ch].voltage = 0;
        }

        /* Check for battery disabled by software */
        else if ((battery_voltage[ch] & MM_BATT_DIS_MASK) == MM_BATT_DIS_SW)
        {
            limit_messages++;
            if (limit_messages < 128 || (limit_messages & 0xff) == 0)
            {
                fprintf(stderr, "MM_MonitorGatherData: Battery %d has been disabled through software.\n", ch);
            }

            /* return status of disabled by s/w */
            pInfo->batteryInformation[ch].status = NVRAM_BATTERY_INFO_STATUS_DISABLED_SW;

            pInfo->boardStatus = NV_STS_SHUTDOWN;
            mmInfo->status = NV_STS_SHUTDOWN;
            pInfo->batteryInformation[ch].voltage = bcd_to_bin(battery_voltage[ch] & ~MM_BATT_DIS_MASK);
        }
        else
        {
            pInfo->batteryInformation[ch].voltage = bcd_to_bin(battery_voltage[ch]);
        }
        pInfo->batteryInformation[ch].chargePercent = bcd_to_bin(battery_charge[ch]);
        cur_batteryVolt = pMM->battery[ch+1].voltage;
    }

    /* Update the basic board/battery information */
    pInfo->batteryCount     = ch;
    pInfo->revision.major   = mmInfo->revision.major;
    pInfo->revision.minor   = mmInfo->revision.minor;
    pInfo->memorySize       = mmInfo->memSize;
    pInfo->memoryErrorCount = mmInfo->errCount;

    /*
    ** Update board status to include memory health, if the battery status
    ** was good (i.e. don't overwrite bad status, if set).
    */
    if ((pInfo->boardStatus == NV_STS_GOOD) && (mmInfo->status != NV_STS_GOOD))
    {
        pInfo->boardStatus = mmInfo->status;
    }
}
#endif


#ifdef FRONTEND
/**
******************************************************************************
**
**  @brief      Analyze the current board state as compared to the new
**              board state and determine if something has changed. If
**              something has changed this code will log the correct event
**              to the CCB and handle updating the write cache battery
**              health.
**
**  @param      NVRAM_BOARD_INFO* pCurrent - Current board information
**
**  @param      NVRAM_BOARD_INFO* pNew - New board information
**
**  @return     none
**
**  @attention  none
**
******************************************************************************
**/
void MM_MonitorAnalyze(NVRAM_BOARD_INFO* pCurrent, NVRAM_BOARD_INFO* pNew)
{
    UINT8   bChanged = false;
    UINT32  logEvent;

    /*
    ** Check if the current board state is UNKNOWN, it will be in that
    ** state if this is the first time in the monitor or if the board
    ** status was somehow unavailable (which should not happen since
    ** we wait for the board to be ready before letting the monitor
    ** code really start).
    */
    if (pCurrent->boardStatus == NV_STS_UNKNOWN)
    {
        /*
        ** Since the current state is unknown, any new value is considered
        ** a change.
        */
        bChanged = true;
    }
    else
    {
        /*
        ** If the board status has changed there is more work to do so
        ** set the changed flag.
        */
        if (pCurrent->boardStatus != pNew->boardStatus)
        {
            bChanged = true;
        }
    }

    /* If the state of the board has changed, log an event. */
    if (bChanged)
    {
        fprintf(stderr, "MM_MonitorAnalyze: cur is 0x%x, new is 0x%x\n",
                pCurrent->boardStatus,
                pNew->boardStatus);

        /* Based on the board status get the correct log event code. */
        if (pNew->boardStatus == NV_STS_GOOD)
        {
            logEvent = LOG_BUFFER_BOARDS_ENABLED;

            /*
            ** Awaken the DMA Executor, if it's waiting, as well as
            ** any task(s) waiting on DMA requests to free up.
            */
            if (TaskGetState(pNV_DMAExecPCB) == PCB_NV_DMA_EXEC_WAIT)
            {
#ifdef MM_VERBOSE
                fprintf(stderr, "MM_MonitorAnalyze: NV Memory DMA Executor "
                        "awakened by battery healing\n");
#endif
#ifdef HISTORY_KEEP
CT_history_pcb("MM_Interrupt setting ready pcb", (UINT32)pNV_DMAExecPCB);
#endif
                TaskSetState(pNV_DMAExecPCB, PCB_READY);
            }
            TaskReadyByState(PCB_NV_DMA_QFULL_WAIT);
        }
        else if (pNew->boardStatus == NV_STS_SHUTDOWN)
        {
            logEvent = LOG_BUFFER_BOARDS_DISABLED_INFO;

            /*
            ** If the board is now disabled and it was previously unknown
            ** then the log message should be an error.
            */
            if (pCurrent->boardStatus == NV_STS_UNKNOWN)
            {
                LOG_SetError(logEvent);
            }
        }
        else if (pNew->boardStatus == NV_STS_LOW_BATT)
        {
            logEvent = LOG_BUFFER_BOARDS_DISABLED_WARN;
        }
        else
        {
            logEvent = LOG_BUFFER_BOARDS_DISABLED_ERROR;
        }

        MM_MonitorLogEvent(logEvent);

        /*
        ** Update the write cache battery health with the current
        ** state of the board.
        */
        if (pNew->boardStatus == NV_STS_GOOD)
        {
            WC_batHealth(BATTERY_BOARD_FE, BATTERY_HEALTH_GOOD);
        }
        else
        {
            WC_batHealth(BATTERY_BOARD_FE, BATTERY_HEALTH_FAIL);
        }
    }

    /*
    ** Make the new board information the current board information.
    **
    ** These pointers are held by the monitor task so this is making
    ** the contents ready for the next time the monitor code gets run.
    */
    memcpy(pCurrent, pNew, sizeof(NVRAM_BOARD_INFO));
}
#endif


#ifdef FRONTEND
/**
******************************************************************************
**
**  @brief      Send the correct log event to the CCB based on the current
**              battery state.
**
**  @param      UINT32 event - Log event code to send, this log event must
**                             not require additional data besides the log
**                             header.
**
**  @attention  This code assumes that all of the LOG_BUFFER_BOARDS_...
**              log events have the same basic structure (just a header)
**              so if that changes this code will need to be changed
**              to generate the correct structures for the different
**              log events.
**
******************************************************************************
**/
void MM_MonitorLogEvent(UINT32 event)
{
    LOG_HEADER_PKT  header;

    /* Setup the log event packet with the event code. */
    header.event = event;

    /*
    ** All of the log messages contains no data so the length is just
    ** the header.
    */
    header.length = sizeof(LOG_HEADER_PKT);

    fprintf(stderr, "MM_MonitorLogEvent: Send Event (0x%x)\n",
            header.event);

    /* Send the packet of data */
    LL_SendPacket(&header, sizeof(LOG_HEADER_PKT), MRLOGFE, 0, 0, 0, 0);
}
#endif


#ifdef FRONTEND
/**
******************************************************************************
**
**  @brief      Check if the board is ready or not.
**
**  @param      none
**
**  @return     UINT8 - true if the board is NOT ready and still needs
**                      to be check, false otherwise.
**
**  @attention  This code uses the cached board information as its values
**              to determine board readiness.
**
******************************************************************************
**/
UINT8 MM_MonitorCheckBoardReadiness(void)
{
    UINT8   bCheckBoard = false;
    UINT32  status0;
    UINT32  status1;

    status0 = gNVBoardInfo.batteryInformation[0].status;
    status1 = gNVBoardInfo.batteryInformation[1].status;

    if (status0 == NVRAM_BATTERY_INFO_STATUS_FAILURE ||
        status1 == NVRAM_BATTERY_INFO_STATUS_FAILURE)
    {
        /* The board is not ready, continue checking. */
        bCheckBoard = true;
    }

    return bCheckBoard;
}
#endif


#ifdef BACKEND
#ifndef PERF
/*
******************************************************************************
**  mm_DumpRegs
**
**  @brief  This subroutine dumps the MicroMemory card registers
**
**  @param  none
**
**  @return none
**
******************************************************************************
*/
void mm_DumpRegs( )
{
    REG64  reg;
    UINT16 chargePct;
    UINT16 bcdVolts;

    /* Print contents of Memory Controller Status Register, bytes 0-7h */
    fprintf(stderr, "===== Dumping MicroMemory Card Registers       7 6 5 4  3 2 1 0\n");
    fprintf(stderr, "                                              -------- --------\n");
    reg.dw = pMM->mcStsReg;
    fprintf(stderr, " Memory controller Status register (0x00):  0x%08X %08X\n",
            reg.hi, reg.lo);

    /* Print contents of Memory Controller Command Register, bytes 8-Fh */
    reg.dw = pMM->mcCmdReg;
    fprintf(stderr, "Memory controller command register (0x08):  0x%08X %08X\n",
            reg.hi, reg.lo);

    /* Print contents of First Error Data Log Register, bytes 10-17h */
    reg.dw = pMM->errFirst.data;
    fprintf(stderr, "     First error data log register (0x10):  0x%08X %08X\n",
            reg.hi, reg.lo);

    /* Print contents of First Error Info/Address Log Register, bytes 18-1Fh */
    reg.dw = pMM->errFirst.infoAddrReg;
    fprintf(stderr, "     First error info log register (0x18):  0x%08X %08X\n",
            reg.hi, reg.lo);

    /* Print contents of Last Error Data Log Register, bytes 20-27h */
    reg.dw = pMM->errLast.data;
    fprintf(stderr, "      Last error data log register (0x20):  0x%08X %08X\n",
            reg.hi, reg.lo);

    /* Print contents of Last Error Info/Address Log Register, bytes 28-2Fh */
    reg.dw = pMM->errLast.infoAddrReg;
    fprintf(stderr, "      Last error info log register (0x28):  0x%08X %08X\n",
            reg.hi, reg.lo);

    /* Print contents of Diagnostic Data Register, bytes 30-37h */
    reg.dw = pMM->errDiag.data;
    fprintf(stderr, "          Diagnostic data register (0x30):  0x%08X %08X\n",
            reg.hi, reg.lo);

    /*
    ** Print contents of Diagnostic Memory Address, bytes 38-3Ch,
    **                   Reserved, byte 3Dh,
    **                   Syndrome Bits, byte 3Eh,
    **               and Check Bits, byte 3Fh
    */
    reg.dw = pMM->errDiag.infoAddrReg;
    fprintf(stderr, "          Diagnostic info register (0x38):  0x%08X %08X\n",
            reg.hi, reg.lo);

    /*
    ** Print contents of 64-bit PCI Address Register, bytes 40-47h,
    **                   64-bit Local Memory Address Register, bytes 48-4Fh,
    **                   Transfer Size Register, bytes 50-57h,
    **                   Descriptor Address Register, bytes 58-5Fh,
    **                   Semaphore Address Register, bytes 60-67h
    **               and Status/Control Register, bytes 68-6Fh
    */
    reg.dw = pMM->dmaDesc.pciDataAddr;
    fprintf(stderr, "DMA Engine:   PCI address register (0x40):  0x%08X %08X\n",
            reg.hi, reg.lo);
    reg.dw = pMM->dmaDesc.localAddr;
    fprintf(stderr, "DMA Engine:Local mem addr register (0x48):  0x%08X %08X\n",
            reg.hi, reg.lo);
    reg.dw = pMM->dmaDesc.xferSizeReg;
    fprintf(stderr, "DMA Engine: Transfer size register (0x50):  0x%08X %08X\n",
            reg.hi, reg.lo);
    reg.dw = pMM->dmaDesc.pciDescAddr;
    fprintf(stderr, "DMA Engine:Descriptr addr register (0x58):  0x%08X %08X\n",
            reg.hi, reg.lo);
    reg.dw = pMM->dmaDesc.pciSemAddr;
    fprintf(stderr, "DMA Engine:Semaphore addr register (0x60):  0x%08X %08X\n",
            reg.hi, reg.lo);
    reg.dw = pMM->dmaDesc.stsCtrlReg;
    fprintf(stderr, "DMA Engine:Status/Control register (0x68):  0x%08X %08X\n",
            reg.hi, reg.lo);

    /* Print contents of Window Map Register, bytes 78-7Fh */
    reg.dw = pMM->windowMapReg;
    fprintf(stderr, "               Window map register (0x78):  0x%08X %08X\n",
            reg.hi, reg.lo);
    fprintf(stderr, "===============================================================\n");

    /* Print contents of Battery Status Registers, bytes A0-ABh */
    chargePct = pMM->battery[0].charge;
    bcdVolts  = pMM->battery[0].voltage;
    fprintf(stderr, " Battery Status:  Battery 1 Charge (0xA0):  0x%02X = %X.%X%%\n",
            chargePct, chargePct>>4, chargePct & 0x000F);
    fprintf(stderr, "                           Voltage (0xA2):  0x%02X = %d.%03XV\n",
            bcdVolts, bcdVolts>>12, bcdVolts & 0x0FFF);
    chargePct = pMM->battery[1].charge;
    bcdVolts  = pMM->battery[1].voltage;
    fprintf(stderr, "                  Battery 2 Charge (0xA4):  0x%02X = %X.%X%%\n",
            chargePct, chargePct>>4, chargePct & 0x000F);
    fprintf(stderr, "                           Voltage (0xA6):  0x%02X = %d.%03XV\n",
            bcdVolts, bcdVolts>>12, bcdVolts & 0x0FFF);
} /* mm_DumpRegs */
#else /* PERF below */
void mm_DumpRegs( )
{
    UINT16 bcdVolts0;
    UINT16 bcdVolts1;

    bcdVolts0  = pMM->battery[0].voltage;
    bcdVolts1  = pMM->battery[1].voltage;
    fprintf(stderr, "MicroMemory Card: Battery 1 Voltage %d.%03XV  Battery 2 Voltage %d.%03XV\n",
                    bcdVolts0 >> 12, bcdVolts0 & 0x0FFF,
                    bcdVolts1 >> 12, bcdVolts1 & 0x0FFF);
} /* mm_DumpRegs */
#endif /* Not PERF */

#ifdef MM_DEBUG
/*
******************************************************************************
**  mm_Debug
**
**  @brief  Debug routine called from MM_init
**
**  @param  none
**
**  @return INT32 0 = success, nonzero if failure
**
******************************************************************************
*/
INT32 mm_Debug(void)
{
    volatile UINT8 *uc = pMMemory;
    UINT32 data[64];
    UINT32 count;
    UINT8  dataNV[2048];
    UINT8  h;

    fprintf(stderr,"writing value 0x10 to mmaped address 0x%X\n", (UINT32)uc+2);
    uc[2] = 0x10;
    fprintf(stderr,"reading value from mmaped address 0x%X = 0x%X\n",
            (UINT32)uc+2, uc[2]);
    fprintf(stderr,"writing value 0x23 to mmaped address 0x%X\n", (UINT32)uc+5);
    uc[5] = 0x23;
    fprintf(stderr,"reading value from mmaped address 0x%X = 0x%X\n",
            (UINT32)uc+5, uc[5]);
    {
        for (count = 0; count < sizeof(dataNV); ++count)
        {
            dataNV[count] = 'B';
        }
        for (h = 'A', count = 0; count < 512; ++h, ++count)
        {
            if (h > 'Z')
            {
                h = 'A';
            }
            uc[6+count] = h;
        }
        for (count = 0; count<27; ++count)
        {
            fprintf(stderr, "reading value from mmaped address 0x%X = %c\n",
                    (UINT32)uc+6+count, uc[6+count]);
        }

        fprintf(stderr, "writing %d number of bytes starting from offset address 0x%X\n",
                sizeof(dataNV), 0x00f010);
        count = MM_Write((unsigned long)0x00f010, dataNV, sizeof(dataNV));
        if (count)
        {
            fprintf(stderr, "call to MM_Write failed\n");
            return(-1);
        }

        memset(dataNV, 0, sizeof(dataNV));
        fprintf(stderr, "reading %d number of bytes starting from offset address 0x%X\n",
                sizeof(dataNV), 0xf010);
        count = MM_Read((unsigned long)0xF010, dataNV, sizeof(dataNV));
        if (count)
        {
            fprintf(stderr, "call to MM_Read failed\n");
            return(-1);
        }
        for (count = 0; count< 16; ++count)
        {
            fprintf(stderr, "dataNV[%d] = %c\n", count, dataNV[count]);
        }
        for (count = 0; count < sizeof(dataNV); ++count)
        {
            dataNV[count] = 'C';
        }

        fprintf(stderr, "writing %d number of bytes starting from offset address 0x%X\n",
                sizeof(dataNV), SIZE_16MEG + 0x00f000);
        count = MM_Write((unsigned long)(SIZE_16MEG + 0x00f000), dataNV, sizeof(dataNV));
        if (count)
        {
            fprintf(stderr, "call to MM_Write failed\n");
            return(-1);
        }
        fprintf(stderr, "reading %d number of bytes starting from offset address 0x%X\n",
                sizeof(data), SIZE_16MEG + 61440);
        memset(dataNV, 0, sizeof(dataNV));
        count = MM_Read((unsigned long)(SIZE_16MEG + 61440), dataNV, sizeof(dataNV));
        if (count)
        {
            fprintf(stderr, "call to MM_Read failed\n");
            return(-1);
        }
        for (count = 0; count < 16; ++count)
        {
            fprintf(stderr, "dataNV[%d] = %c\n", count, dataNV[count]);
        }
    }
    return 0;
} /* mm_Debug */
#endif /* MM_DEBUG */


/**
******************************************************************************
**  NV_SendMMInfoCmplt
**
**  @brief      Frees the memory used by MMINFO MRP handler
**
**  @param      Unused
**
**  @return     none
**
******************************************************************************
**/
void NV_SendMMInfoCmplt(UINT32 rc UNUSED, ILT *pILT UNUSED,
                        MR_PKT *pMRP UNUSED, UINT32 parm UNUSED)
{
    s_Free(pMMInfoReq, sizeof(*pMMInfoReq), __FILE__, __LINE__);
    s_Free(pMMInfoRsp, sizeof(*pMMInfoRsp), __FILE__, __LINE__);

#ifdef DEBUG_MMINFO
    fprintf(stderr,"NV_SendMMInfoCmplt:  Exit\n");
#endif
} /* NV_SendMMInfoCmplt */


/*
******************************************************************************
**  nv_SendMMInfo
**
**  @brief  Send MM Info pointer to FE processor via an MRP
**
**  @param  None
**
**  @return None
**
******************************************************************************
*/
void nv_SendMMInfo( )
{
    /* Build MRP & send mmInfo structure pointer to FE processor */
    pMMInfoReq = (MRMMINFO_REQ*)s_Malloc(sizeof(MRMMINFO_REQ), __FILE__, __LINE__);
    pMMInfoRsp = (MRMMINFO_RSP*)s_Malloc(sizeof(MRMMINFO_RSP), __FILE__, __LINE__);

    if ((pMMInfoReq != NULL) && (pMMInfoRsp != NULL))
    {
        pMMInfoReq->pMMInfo = mmInfo;

#ifdef MM_VERBOSE
        fprintf(stderr,"nv_SendMMInfo:  Send mmInfo (%p) to FE\n", mmInfo);
        fprintf(stderr, "                              pMM = %p\n"
                        "                         pMMemory = %p\n"
                        "                         mem_size = 0x%08X\n"
                        "                    error counter = 0x%X\n"
                        "                        pDMAQueue = %p\n",
                        pMM, pMMemory, mem_size, mmInfo->errCount, mmInfo->pDMAQueue);
#endif
        LL_SendPacket(pMMInfoReq,                   /* Packet address       */
                      sizeof(*pMMInfoReq),          /* MM Info packet size  */
                      MRMMINFO,                     /* function code        */
                      pMMInfoRsp,                   /* return data address  */
                      sizeof(*pMMInfoRsp),          /* Return data size     */
                      (void*)&NV_SendMMInfoCmplt,   /* Completer function   */
                      0);                           /* user parameter       */
    }
    else
    {
        /* It is an error if the Mallocs fail. */
        fprintf(stderr, "nv_SendMMInfo:  Fatal error, Malloc failed\n");
        nv_ProcessFatalEvent(NV_FATAL_ASSERT, 15);
    }
} /* nv_SendMMInfo */
#endif /* BACKEND */


#ifdef FRONTEND
/*
******************************************************************************
**  NV_ProcessMMInfo
**
**  @brief  Process a received MM Info packet
**
**  @param  MR_PKT* - MRP pointer, contains pointer to MM_INFO structure from BE
**
**  @return UINT32  - MRP completion status
**
******************************************************************************
*/
UINT32 NV_ProcessMMInfo(MR_PKT *pMRP)
{
    void *ptr;

    /*
    ** Extract information from the MM_INFO structure.
    ** If the card doesn't exist, the mmInfo pointer will be NULL.
    */
    pMMInfoReq = (MRMMINFO_REQ*)(pMRP->pReq);
    mmInfo = pMMInfoReq->pMMInfo;
    fprintf(stderr, "NV_ProcessMMInfo:  Setting mmInfo = %p\n", mmInfo);
    if ((mmInfo != NULL) && (mmInfo->pMM != NULL))
    {
        UINT32  tmp_addr;

        pMMemory  = mmInfo->pMMemory;
        pMM       = mmInfo->pMM;
        mem_size  = mmInfo->memSize;

        /*
        ** Record the size of the MicroMemory device.
        ** It is an error if we sense an invalid size.
        */

        switch(mem_size)
        {
            case SIZE_256MEG:
                gMPControllerOriginalConfig.wCacheSize = WC_SIZE_256MEG;
                break;
            case SIZE_512MEG:
                gMPControllerOriginalConfig.wCacheSize = WC_SIZE_512MEG;
                break;
            case SIZE_1GIG:
                gMPControllerOriginalConfig.wCacheSize = WC_SIZE_1GIG;
                break;
            case SIZE_2GIG:
                gMPControllerOriginalConfig.wCacheSize = WC_SIZE_2GIG;
                break;
            default:
                /* should never get here */
                fprintf(stderr, "%s:%u - Invalid value of mem present register: 0x%X\n",
                        __FILE__, __LINE__, mem_size);
                nv_ProcessFatalEvent(NV_FATAL_ASSERT, 12);
                return(DEFAILED);
        }
        gMPControllerCurrentConfig.wCacheSize = gMPControllerOriginalConfig.wCacheSize;
        bfDefaults.wCacheSize = gMPControllerOriginalConfig.wCacheSize;
        gMMCFound = TRUE;
        pDMAQueue = mmInfo->pDMAQueue;
#ifdef MM_VERBOSE
        fprintf(stderr, "                              pMM = %p\n"
                        "                         pMMemory = %p\n"
                        "                         mem_size = 0x%08X\n"
                        "                    error counter = 0x%X\n"
                        "                        pDMAQueue = %p\n",
                        pMM, pMMemory, mem_size, mmInfo->errCount, mmInfo->pDMAQueue);
#endif

        /* Map the MicroMemory registers to PCI address space (256 bytes). */
/*         if ((ptr = (void*)LI_AccessDevice(mmInfo->id + PCIBASE, 0)) != pMM) */
        tmp_addr = LI_AccessDevice(pci_dev_micro_memory + PCIBASE, 0);
        if ((ptr = (void*)tmp_addr) != pMM)
        {
            fprintf(stderr, "%s:%u - Memory map failed thru LI_AccessDevice:  "
                    "old pMM = %p, new pMM = %p\n", __FILE__, __LINE__, pMM, ptr);
            nv_ProcessFatalEvent(NV_FATAL_ASSERT, 16);

            /* Setup and send the response packet */
            pMMInfoRsp = (MRMMINFO_RSP*)(pMRP->pRsp);
            pMMInfoRsp->header.len = sizeof(*pMMInfoRsp);
            pMMInfoRsp->header.status = DEFAILED;

            return(DEFAILED);
        }
        fprintf(stderr, "NV_ProcessMMInfo:  mapped 256 bytes config space "
                "on BE device %d (FE %d) to address %p\n", mmInfo->id, pci_dev_micro_memory, pMM);

        /* Map the MicroMemory device 16MB window to a PCI address space */
/*         if ((ptr = (void*)LI_AccessDevice(mmInfo->id + PCIBASE, 1)) != pMMemory) */
        tmp_addr = LI_AccessDevice(pci_dev_micro_memory + PCIBASE, 1);
        if ((ptr = (void*)tmp_addr) != pMMemory)
        {
            fprintf(stderr, "%s:%u - Memory map failed thru LI_AccessDevice:  "
                    "old pMMemory = %p, new pMMemory = %p\n",
                    __FILE__, __LINE__, pMMemory, ptr);
            nv_ProcessFatalEvent(NV_FATAL_ASSERT, 17);

            /* Setup and send the response packet */
            pMMInfoRsp = (MRMMINFO_RSP*)(pMRP->pRsp);
            pMMInfoRsp->header.len = sizeof(*pMMInfoRsp);
            pMMInfoRsp->header.status = DEFAILED;

            return(DEFAILED);
        }
        fprintf(stderr, "NV_ProcessMMInfo:  Mapped 16MB window space "
                "on BE device %d (FE %d) to address %p\n", mmInfo->id, pci_dev_micro_memory, pMMemory);

        /* Register the interrupt handler, and enable interrupts */
/*         LI_RegisterIRQ(mmInfo->id, MM_Interrupt, (UINT32)NULL); */
        LI_RegisterIRQ(pci_dev_micro_memory, MM_Interrupt, (UINT32)NULL);
        pMM->errCtrlBits.intMask = 0;
        /* Do not need to flush the above, done in tasks and otherwise. */

        /* Create/fork the NV Memory DMA Executor task */
        if (pNV_DMAExecPCB == NULL)
        {
            CT_fork_tmp = (unsigned long)"nv_DMAExec";
            pNV_DMAExecPCB = (PCB *)-1;         // Flag process being created.
            pNV_DMAExecPCB = TaskCreatePerm2(C_label_referenced_in_i960asm(nv_DMAExec), NV_DMA_EXEC_PRIORITY);
        }
        else
        {
            while (pNV_DMAExecPCB == (PCB *)-1)
            {
                TaskSleepMS(50);
            }
        }

        /* Create/fork the NV Memory DMA Completor task */
        if (pNV_DMACompPCB == NULL)
        {
            CT_fork_tmp = (unsigned long)"nv_DMAComplete";
            pNV_DMACompPCB = (PCB *)-1;         // Flag task being created.
            pNV_DMACompPCB = TaskCreatePerm2(C_label_referenced_in_i960asm(nv_DMAComplete), NV_DMA_COMP_PRIORITY);
        }

        /* Create the MM monitor task if has not already been created. */
        if (pMMMonitorPCB == NULL)
        {
            /*
            ** Initialize the cached board information, 0 is UNKNOWN for
            ** status values.
            */
            memset(&gNVBoardInfo, 0x00, sizeof(NVRAM_BOARD_INFO));

            /* Fork the task to get the monitor going. */
            CT_fork_tmp = (unsigned long)"MM_MonitorTask";
            pMMMonitorPCB = (PCB *)-1;          // Flag that task is being created.
            pMMMonitorPCB = TaskCreate2(C_label_referenced_in_i960asm(MM_MonitorTask), MM_MONITOR_PRIORITY);
        }

        /* Create the Scrub task if has not already been created. */
        if (gpScrubPCB == NULL)
        {
            /* Fork the task to get the NV Scrub going. */
            CT_fork_tmp = (unsigned long)"NV_ScrubTask";
            gpScrubPCB = (PCB *)-1;         // Flag that we are in process of creating it.
            gpScrubPCB = TaskCreate2(C_label_referenced_in_i960asm(NV_ScrubTask), NV_SCRUB_PRIORITY);
        }

#ifdef NV_DMA_DRVR
        /* Create/fork the test driver task for NV Memory DMA */
        CT_fork_tmp = (unsigned long)"nv_DMATestDriver";
        pNV_DMADrvrPCB = TaskCreatePerm2(C_label_referenced_in_i960asm(nv_DMATestDriver), NV_DMA_COMP_PRIORITY+10);
#endif
        /* Initialize the WC_NV structure */
        WC_RestoreData();
    }
    else
    {
        /* No card found by BE. */
        if (gMMCFound)
        {
            fprintf(stderr, "NV_ProcessMMInfo:  No card available - mmInfo = %p, pMM = %p\n",
                    mmInfo, pMM);
        }
    }

    /*
    ** Awaken any tasks waiting for MM to complete initialization,
    ** regardless of whether an MM card is available.
    */
    TaskReadyByState(PCB_MM_WAIT);

    /* Setup and send the response packet */
    pMMInfoRsp = (MRMMINFO_RSP*)(pMRP->pRsp);
    pMMInfoRsp->header.len = sizeof(*pMMInfoRsp);
    pMMInfoRsp->header.status = DEOK;

    return(DEOK);
} /* NV_ProcessMMInfo */
#endif /* FRONTEND */


#ifdef BACKEND
/*
******************************************************************************
** MM_init
**
**  @brief  Initializes MM-5425CN Micro Memory Card, sets up CSR and memory
**  address spaces, maps CSR and Memory address spaces to User space.
**  The function searches for Micro Memory cards, and returns -1 if no MM
**  Card found. MM_init() needs to be called before calling MM_Read and
**  MM_Write and the DMA functions, since the CSR registers are mmaped in
**  MM_init.
**
**  Note: A separate environment variable $MICROMEMORY must be set
**  to point to the Micro Memory bus device function number,  e.g. 05/04.0
**  This has to be created and "exported" in addition to the regular
**  $FEDEVS and $BEDEVS environment variables say in .bashrc script in
**  your root dir.
**
**  Code #defined under STANDALONE is meant for stand alone testing of the
**  driver code in the absence of Wookiee environment.
**
**  @param  none, no input parameters
**
**  @return -1 if failure, 0 if success
**
******************************************************************************
*/
INT32 MM_init(void)
{
    UINT32          i;
    UINT32          j;
    struct pci_devs *dev;
    UINT8           errDetMode;
    UINT32          data[64];
    UINT32          result;
    UINT8           bytes[2];
    unsigned long   tmp_addr;

#ifdef STANDALONE
    pcidevtbl       pcitbl[XIO3D_NDEVS_MAX];
    UINT32          bitmap;
    void            *mem;

    memset((void *)pcitbl, 0, sizeof(pcitbl));
    PAGE_SIZE = getpagesize();
    PAGE_MASK = ~(PAGE_SIZE - 1);
    fprintf(stderr, "PAGE_MASK=%#08lX\n", PAGE_MASK);
    fprintf(stderr, "PAGE_SIZE=%d\n", (UINT32)PAGE_SIZE);
#endif /* STANDALONE */

#ifdef MM_VERBOSE
    REG64 tscBegin, tscEnd;
#endif /* MM_VERBOSE */

    fprintf(stderr, "MM_init entry...\n");
    xiofd = open(DEVFILE, O_RDWR);
    if (xiofd < 0)
    {
        fprintf(stderr, "%s:%u - Open of %s failed with %d\n",
                __FILE__, __LINE__, DEVFILE, xiofd);
        perror("open failed");
        return xiofd;
    }

    /* Get PCI device information from /proc/bus/pci/devices */
    if (pci_devs_index == 0)
    {
        fprintf(stderr, "MM_init:  Get PCI device info\n");
        if (get_device_info())
        {
            fprintf(stderr, "%s:%u - get_device_info failed\n",
                    __FILE__, __LINE__);
            return -1;
        }
    }

    /* Get information from xio3d module. */
    if (ioctl(xiofd, XIO3D_GETINF, &xio3d_drvinfo) == -1)
    {
        int save_errno = errno;

        fprintf(stderr, "ioctl failed with %d\n", errno);
        errno = save_errno;         /* restore, fprintf might change errno. */
        perror("ioctl failed");
        return save_errno;          /* perror() might change it. */
    }
    fprintf(stderr, "MM_init:  .id=0x%08llX\n", xio3d_drvinfo.id);
    for (i = 0; i < XIO3D_NSHM_MAX; ++i)
    {
        if (xio3d_drvinfo.mem_regions[i].phys == 0)
        {
            fprintf(stderr, "MM_init:  mem_region %d has phys = 0!!\n", i);
            continue;
        }
        fprintf(stderr, "MM_init:  .mem_regions[%d].offset=0x%08llX\n", i,
                xio3d_drvinfo.mem_regions[i].offset);
        fprintf(stderr, "MM_init:  .mem_regions[%d].size=0x%08llX\n", i,
                xio3d_drvinfo.mem_regions[i].size);
        fprintf(stderr, "MM_init:  .mem_regions[%d].phys=0x%08llX\n", i,
                xio3d_drvinfo.mem_regions[i].phys);
    }

#ifdef STANDALONE
    /* Map shared memory segments using xio3d module and print a bit. */
    for (i = 0; i < XIO3D_NSHM_MAX; ++i)
    {
        if (xio3d_drvinfo.mem_regions[i].offset && xio3d_drvinfo.mem_regions[i].size)
        {
            UINT32 *l;

            mem = mmap((void *)((i + 1) << 28),
                       xio3d_drvinfo.mem_regions[i].size,
                       PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED,
                       xiofd,
                       xio3d_drvinfo.mem_regions[i].offset);

            if (mem == MAP_FAILED)
            {
                int save_errno = errno;

                fprintf(stderr, "mmap failed with %d\n", errno);
                errno = save_errno;     /* fprintf() might change errno. */
                perror("mmap failed");
                return save_errno;      /* perror() might change errno. */
            }
            else
            {
                fprintf(stderr, "mmap succeeded for shared region[%d], address: %p\n",
                        i, mem);
            }

            l = mem;
            map_shared[i] = mem;
            fprintf(stderr, "map_shared[%d]=%p, .offset=%08llX\n", i, mem,
                    xio3d_drvinfo.mem_regions[i].offset);
            dumpsome(mem, 128);
            bzero(mem,xio3d_drvinfo.mem_regions[i].size);
            fprintf(stderr, "zero?\n");
            dumpsome(mem, 128);
        }
    }

    bitmap = LI_ScanBus(pcitbl, XIO3D_NDEVS_MAX);
    fprintf(stderr, "bitmap=%04X\n", bitmap);
    for (i = PCIOFFSET; i < XIO3D_NDEVS_MAX + PCIOFFSET; ++i)
    {
        fprintf(stderr, "vendev[%d] = %04X\n", i, pcitbl[i].vendev);
    }
#endif  /* STANDALONE */

    for (i = PCIBASE; i < XIO3D_NIO_MAX+PCIBASE; ++i)
    {
#ifdef STANDALONE
        if ((bitmap & (1 << i)) == 0)
        {
            continue;
        }
#endif
        dev = &gPCIdevs[i - PCIBASE];
        if (dev->vendor == 0)
        {
            continue;       /* not a device */
        }

        /* Get PCI device ID, and check for MicroMemory card. */
        result = LI_GetConfig(i, PCI_DEVICE_ID, 2, (UINT8 *)data);
        if (result != TRUE)
        {
            fprintf(stderr, "%s:%u - Did not get PCI device ID from device %d\n",
                    __FILE__, __LINE__, i - PCIBASE);
            /* return -1; */
            continue;
        }
        if (*(UINT16 *)data != MMCARDID)
        {
            continue; /* if not MM-5425CN, continue */
        }
        else
        {
            fprintf(stderr,"MM_init:  Found MM-5425CN card, device = %d\n",
                    i - PCIBASE);
        }

        /* Get 64 bytes of PCI configuration data. */
        result = LI_GetConfig(i, 0, sizeof(data), (UINT8 *)data);
        if (result != TRUE)
        {
            fprintf(stderr, "%s:%u - Did not get PCI config from device %d\n",
                    __FILE__, __LINE__, i - PCIBASE);
            return -1;
        }
        fprintf(stderr, "MM_init:  Device %d PCI config data:\n", i - PCIBASE);
        fprintf(stderr, "  offset 0x00:  0x%08X 0x%08X 0x%08X 0x%08X\n",
                data[0], data[1], data[2], data[3]);
        fprintf(stderr, "  offset 0x10:  0x%08X 0x%08X 0x%08X 0x%08X\n",
                data[4], data[5], data[6], data[7]);
        fprintf(stderr, "  offset 0x20:  0x%08X 0x%08X 0x%08X 0x%08X\n",
                data[8], data[9], data[10], data[11]);
        fprintf(stderr, "  offset 0x30:  0x%08X 0x%08X 0x%08X 0x%08X\n",
                data[12], data[13], data[14], data[15]);

        /* Get/Set PCI latency timer. */
        result = LI_GetConfig(i, PCI_LATENCY_TIMER, 1, bytes);
        if (result != TRUE)
        {
            fprintf(stderr, "%s:%u - Did not get PCI latency timer from device %d\n",
                    __FILE__, __LINE__, i - PCIBASE);
            return -1;
        }
        fprintf(stderr, "MM_init:  Got PCI latency timer 0x%X from device %d\n",
                bytes[0], i - PCIBASE);

        if (bytes[0] != MM_LATENCY)
        {
            bytes[0] = MM_LATENCY;
            result = LI_SetConfig(i, PCI_LATENCY_TIMER, 1, bytes);
            if (result != TRUE)
            {
                fprintf(stderr, "%s:%u - Did not set PCI latency timer for device %d\n",
                        __FILE__, __LINE__, i - PCIBASE);
                return -1;
            }
            fprintf(stderr, "MM_init:  Set PCI latency timer for device %d to 0x%X\n",
                    i - PCIBASE, bytes[0]);
        }

        /* Get/Set PCI Command Register (offset 04h). */
        result = LI_GetConfig(i, PCI_COMMAND, 2, bytes);
        if (result != TRUE)
        {
            fprintf(stderr, "%s:%u - Did not get PCI command data from device %d\n",
                    __FILE__, __LINE__, i - PCIBASE);
            return -1;
        }
        fprintf(stderr, "MM_init:  Got PCI command data from device %d of 0x%04X\n",
                i - PCIBASE, *(UINT16*)bytes);

        /*
        ** Disable fast back-to-back cycles
        ** Enable system error (SERR) assertion (parity error during addr phase)
        ** Enable parity error (PERR) assertion (parity error during data phase)
        ** Enable bus master (during DMAs)
        ** Enable memory space (allows access of memory blocks)
        */
        *(UINT16*)bytes &= ~PCI_COMMAND_FAST_BACK;
        *(UINT16*)bytes |= (PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER |
                            PCI_COMMAND_PARITY | PCI_COMMAND_SERR);
#ifdef MEM_WRITE_INVALIDATE
        /* Enable memory write and invalidate */
        bytes[0] |= PCI_COMMAND_INVALIDATE;
#endif

        result = LI_SetConfig(i, PCI_COMMAND, 2, bytes);
        if (result != TRUE)
        {
            fprintf(stderr, "%s:%u - Did not set PCI command data to device %d\n",
                    __FILE__, __LINE__, i - PCIBASE);
            return -1;
        }
        fprintf(stderr, "MM_init:  Set PCI command data to device %d of 0x%04X\n",
                i - PCIBASE, *(UINT16*)bytes);

        result = LI_GetConfig(i, PCI_COMMAND, 2, bytes);
        if (result != TRUE)
        {
            fprintf(stderr, "%s:%u - Did not get PCI command data from device %d\n",
                    __FILE__, __LINE__, i - PCIBASE);
            return -1;
        }
        fprintf(stderr, "MM_init:  Got PCI command data from device %d of 0x%04X\n",
                i - PCIBASE, *(UINT16*)bytes);

        /* Get 64-bit PCI base address. */
        result = LI_GetConfig(i, PCI_BASE_ADDRESS_0, 4, (UINT8 *)data);
        if (result != TRUE)
        {
            fprintf(stderr, "%s:%u - Did not get PCI base address 0 from device %d\n",
                    __FILE__, __LINE__, i - PCIBASE);
            return -1;
        }
        fprintf(stderr, "MM_init:  Got PCI base address 0 from device %d of 0x%X\n",
                i - PCIBASE, *data);

        result = LI_GetConfig(i, PCI_BASE_ADDRESS_1, 4, (UINT8 *)data);
        if (result != TRUE)
        {
            fprintf(stderr, "%s:%u - Did not get PCI base address 1 from device %d\n",
                    __FILE__, __LINE__, i - PCIBASE);
            return -1;
        }
        fprintf(stderr, "MM_init:  Got PCI base address 1 from device %d of 0x%X\n",
                i - PCIBASE, *data);

        /*
        ** Map the MicroMemory registers to PCI address space (256 bytes).
        ** It is an error if this mapping fails.
        */
        tmp_addr = LI_AccessDevice(i, 0);
        pMM = (MM_REGS *)tmp_addr;
        if (pMM == NULL)
        {
            fprintf(stderr, "%s:%u - Memory map failed thru LI_AccessDevice:"
                    " pMM = NULL\n", __FILE__, __LINE__);
            nv_ProcessFatalEvent(NV_FATAL_ASSERT, 10);
            return -1;
        }
        fprintf(stderr, "MM_init:  mapped 256 bytes config space on device %d"
                " to address 0x%X\n", i - PCIBASE, (UINT32)pMM);

#ifdef ERROR_STATUS_ENABLE
        /*
        ** Ensure single-bit correction & double-bit detection are enabled.
        ** Also clear the error counter (do this first).
        */
        errDetMode = pMM->errCtrl;
        fprintf(stderr,"  Error Control = 0x%X\n", errDetMode);
        errDetMode &= MMCARD_ERR_STATUS;
        if (errDetMode != MMCARD_ERR_STATUS)
        {
            fprintf(stderr,"Writing value 0x%X to Error Control\n", MMCARD_ERR_STATUS);
            mm_ClearErrorCounter();
            pMM->errCtrl = MMCARD_ERR_STATUS;
            fprintf(stderr,"  Error Control = 0x%X\n", pMM->errCtrl); /* flush */
        }
#else
        fprintf(stderr,"Writing value 0 to Error Control (error detection & correction DISABLED!)\n");
        pMM->errCtrl = 0;
        fprintf(stderr,"  Error Control = 0x%X\n", pMM->errCtrl);
#endif
        /* Flush of errCtl done with read of memPres below. */

        /*
        ** Allocate an MM Info object and start filling in the fields.
        ** (Note:  Check for previous allocation first, in case the system
        **         contains more than one card for some reason. The last card
        **         discovered will be used.)
        ** It is an error if the Malloc fails.
        */
        if (mmInfo == NULL)
        {
            mmInfo = s_Malloc(sizeof(*mmInfo), __FILE__, __LINE__);
            if (mmInfo == NULL)
            {
                nv_ProcessFatalEvent(NV_FATAL_ASSERT, 11);
                return -1;
            }
        }
        mmInfo->status          = NV_STS_UNKNOWN;
        mmInfo->errCount        = 0;
        mmInfo->revision.major  = pMM->majorRev;
        mmInfo->revision.minor  = pMM->minorRev;
        for (j = 0;
             (j < NVRAM_BOARD_MAX_BATTERIES) && (pMM->battery[j].voltage != MM_BATT_END_MRK);
             ++j);
        mmInfo->battCount       = j;
        mmInfo->pMM             = pMM;
        mmInfo->id              = (i - PCIBASE);

        /*
        ** Record the size of the MicroMemory device.
        ** It is an error if we sense an invalid size.
        */
        switch(pMM->memPres)
        {
            case 0xfc:
                mem_size = SIZE_256MEG; /* 256 MB */
                gMPControllerCurrentConfig.wCacheSize = WC_SIZE_256MEG;
                break;
            case 0xf8:
                mem_size = SIZE_512MEG; /* 512 MB */
                gMPControllerCurrentConfig.wCacheSize = WC_SIZE_512MEG;
                break;
            case 0xf0:
                mem_size = SIZE_1GIG; /* 1GB */
                gMPControllerCurrentConfig.wCacheSize = WC_SIZE_1GIG;
                break;
            case 0xe0:
                mem_size = SIZE_2GIG; /* 2GB */
                gMPControllerCurrentConfig.wCacheSize = WC_SIZE_2GIG;
                break;
            default:
                /* should never get here */
                fprintf(stderr, "%s:%u - Invalid value of mem present register: 0x%X\n",
                        __FILE__, __LINE__, pMM->memPres);
                nv_ProcessFatalEvent(NV_FATAL_ASSERT, 12);
                return -1;
        }
        fprintf(stderr, "MM_init:  MM-5425CN card has 0x%X bytes of memory (%d GB)\n",
                mem_size, (mem_size/SIZE_1GIG));
        mmInfo->memSize = mem_size;

        /*
        ** Map the MicroMemory device 16MB window to a PCI address space.
        ** It is an error if this mapping fails.
        */
        tmp_addr = LI_AccessDevice(i, 1);
        pMMemory = (volatile UINT8 *)tmp_addr;
        if (pMMemory == NULL)
        {
            fprintf(stderr, "%s:%u - Memory map failed thru LI_AccessDevice!\n",
                    __FILE__, __LINE__);
            pMM = NULL;
            nv_ProcessFatalEvent(NV_FATAL_ASSERT, 13);
            return -1;
        }
        fprintf(stderr, "MM_init:  Mapped 16MB window space on device %d to address 0x%X\n",
                i - PCIBASE, (UINT32)pMMemory);
        mmInfo->pMMemory = pMMemory;

/* #ifndef PERF */
        mm_DumpRegs();
/* #endif */ /* Not PERF */

        /*
        ** Mask interrupts - Front End will enable interrupts
        ** and register the interrupt handler
        */
        pMM->errCtrlBits.intMask = 1;
        /* Any read in the area will flush. */
        MM_FORCE_WRITE_TO_FLUSH(pMM->ledCtrl);

        /*
        ** Delay to ensure memory is accessible (after setting Error Control Reg
        ** to some value other than 0x01). A delay of approx. 10ms is used.
        */
#ifdef MM_VERBOSE
        tscBegin.dw = get_tsc();
#endif /* MM_VERBOSE */

        TaskSleepNoSwap(10000);

#ifdef MM_VERBOSE
        tscEnd.dw = get_tsc();
        fprintf(stderr, "MM_init:  10ms delay loop began at 0x%X %08X,\n"
                        "                          ended at 0x%X %08X\n",
                tscBegin.hi, tscBegin.lo, tscEnd.hi, tscEnd.lo);
#endif /* MM_VERBOSE */

#ifdef MM_DEBUG
        if (mm_Debug() != 0)
        {
            return -1;
        }
#endif /* MM_DEBUG */

        /*
        ** MicroMemory Card was found (but is not necessarily accessible).
        ** Break out of discovery loop.
        */
        break;
    } /* for (i = PCIBASE; i < XIO3D_NIO_MAX+PCIBASE; ++i) */

    if ((i == XIO3D_NIO_MAX+PCIBASE) || (pMM == NULL))
    {
        /*
        ** Allocate an MM Info object and clear the fields. Set UNKNOWN status.
        ** It is an error if the Malloc fails.
        */
        if (mmInfo == NULL)
        {
            mmInfo = s_Malloc(sizeof(*mmInfo), __FILE__, __LINE__);
            if (mmInfo == NULL)
            {
                nv_ProcessFatalEvent(NV_FATAL_ASSERT, 14);
                return -1;
            }
        }
        mmInfo->status      = NV_STS_NO_BOARD;
        mmInfo->errCount    = 0;
        mmInfo->memSize     = 0;
        mmInfo->battCount   = 0;
        mmInfo->pMMemory    = NULL;
        mmInfo->pMM         = NULL;
        mmInfo->id          = 0;
        mmInfo->pDMAQueue   = NULL;

        fprintf(stderr, "%s:%u - MM-5425CN Micro Memory card not found, "
                "or is inaccessible!!\n", __FILE__, __LINE__);
        nv_ProcessFatalEvent(NV_FATAL_NO_CARD, 0);
    }
    else
    {
        /*
        ** Perform diag tests on NV Memory.
        ** mmInfo->status is updated as necessary.
        */
        nv_Diags();

        /*
        ** Verify that the controller S/N in the NV Memory matches ours.
        ** If not, set appropriately bad status in mmInfo record.
        ** (Note:  cSerial is the first field in this region)
        **
        ** NOTE:  If the read fails, this will either be a fatal error or the
        **        card will otherwise be marked as unusable, so there is no
        **        real recourse here.
        */
        if ((MM_Read(NV_ADMIN_START, (UINT8*)data, sizeof(K_ficb->cSerial)) == 0) &&
            (data[0] != K_ficb->cSerial))
        {
            /*
            ** Check for migration issue - Controller S/N of zero isn't valid,
            ** so initialize the Admin region.
            */
            if ((data[0] == 0) || (data[0] == 0xFFFFFFFF))
            {
                fprintf(stderr, "MM_init:  Controller S/N preserved in MM Card (0x%X) "
                        "was invalid, so the Admin region will be initialized\n"
                        "          with our current controller S/N (0x%X)\n",
                        data[0], K_ficb->cSerial);
                nv_InitAdminRegion();
            }
            else
            {
                nv_ProcessFatalEvent(NV_FATAL_SN_MISMATCH, data[0]);
            }
        }

        /* Allocate the DMA Request Queue, including header and entries */
        pDMAQueue = s_Malloc(NV_DMA_QUEUE_SIZE, __FILE__, __LINE__);
        if (pDMAQueue != NULL)
        {
            /*
            ** Initialize the begin, in, and out pointers to the first entry,
            ** and initialize the end pointer to the [end+1] entry.
            */
            pDMAQueue->begin = (void *)((UINT32)pDMAQueue + sizeof(*pDMAQueue));
            pDMAQueue->in = pDMAQueue->begin;
            pDMAQueue->out = pDMAQueue->begin;
            pDMAQueue->nextSub = pDMAQueue->begin;
            pDMAQueue->end = &(pDMAQueue->in[NV_DMA_MAX_REQ]);
            pDMAQueue->active = NULL;
            pDMAQueue->rsvd[0] = pDMAQueue->rsvd[1] = pDMAQueue->rsvd[2] = 0;
            memset(pDMAQueue->begin, 0, (NV_DMA_MAX_REQ * sizeof(*pDMAQueue->begin)));
        }
        mmInfo->pDMAQueue = pDMAQueue;
    } /* Card found - diag tests */

    /*
    ** Clear the Clean Shutdown flag (calling GetCleanShutdown() clears it) in
    ** case it wasn't called above. Its return value is cached internally, so
    ** it could be called later if neccessary, but it must be called at least
    ** once!
    */
    GetCleanShutdown();

    /*
    ** Notify the FE that the card, if found, has been initialized.
    ** Whether it's ready for use depends on the value of mmInfo->status.
    */
    nv_SendMMInfo();

    return 0;
} /* MM_init */
#endif /* BACKEND */


#ifdef FRONTEND
/**
******************************************************************************
**
**  @brief      NV_ScrubTask - Non-Volatile Memory Scrub Task
**
**              This is a separate task that is spawned off to read all the
**              regions of the non-volitale memory card over a specified time
**              to ensure the memory card is operating correctly (able to read
**              the data even though most accesses are writes). The task will
**              DMA data from the beginning of the memory regions to the end
**              to see if any problems are detected over the specified time.
**              This process will run forever continually testing the memory.
**              If any errors are found during the test, the card will be
**              flagged as "Bad" with an error message sent to the user to
**              inform them of the problems.
**
**  @param      UINT32 dummy1 - Dummy parameter required for fork.
**
**  @param      UINT32 dummy2 - Dummy parameter required for fork.
**
**  @return     none
**
**  @attention  Note - This function is expected to be spawned off to run
**              forever after the diagnostics have completed and all memory has
**              valid ECC.
**
**  @attention  Warning - This task should not be run too often to cause
**              performance problems with the normal data flow.
**
******************************************************************************
**/
NORETURN void NV_ScrubTask(UINT32 dummy1 UNUSED, UINT32 dummy2 UNUSED)
{
    NV_DMA_DD*  pDD;
    UINT32*     pBuffer;
    UINT64      memoryOffset = 0;
    INT32       rc = 0;

    /*
    ** Allocate the DMA structure, the buffer for the transfer and the
    ** response packet buffer.
    */
    pDD = s_MallocW(sizeof(*pDD), __FILE__, __LINE__);
    pBuffer = s_MallocW(SCRUB_TRANSFER_LENGTH, __FILE__, __LINE__);
    gpScrubCompletionRsp = s_MallocC(sizeof(NV_DMA_RSP_PKT), __FILE__, __LINE__);

    /* Loop forever, checking the memory on the Non-Volatile memory card */
    while(1)
    {
        /* Delay for the required time before doing the DMA */
        TaskSleepMS(SCRUB_DELAY_TIME);

        /*
        ** If the board is not initialized or not accessible, loop again
        ** and wait for the scrub delay.
        */
        if (mmInfo == NULL || (mmInfo->status & ~NV_STS_DIAG) != NV_STS_GOOD)
        {
            continue;
        }

        /*
        ** If we are at the zero memory starting point, the return code is
        ** still zero and there is no error in the errsts field, log a
        ** starting scrub message just so we have a reference point.
        **
        ** NOTE: This will work for the first time through since the
        **       variables used are initialized to zeros either in the
        **       variable declaration or by the MallocWC.
        */
        if (memoryOffset == 0 &&
            rc == 0 &&
            gpScrubCompletionRsp->dmaStsCtrl.errSts == 0)
        {
            fprintf(stderr, "NV_ScrubTask-Scrub Starting (0x%8.8x%8.8x)\n",
                    (UINT32)(memoryOffset>>32),
                    (UINT32)memoryOffset);
        }

        /* Clear out the scrub completion response packet. */
        memset(gpScrubCompletionRsp, 0x00, sizeof(NV_DMA_RSP_PKT));

        /* Setup the DMA structure for this request. */
        pDD->pNextDD = 0;
        pDD->sysAddr = (UINT32)pBuffer;
        pDD->nvAddr = memoryOffset;
        pDD->xferSize = SCRUB_TRANSFER_LENGTH;
        pDD->wrtNotRd = 0; /* READ */

        /*
        ** Submit the DMA request and tell it to use the scrub completion
        ** routine.
        */
        rc = NV_DMARequest(pDD, NV_ScrubCompletion, (ILT*)pBuffer);

        if (rc == 0)
        {
            /*
            ** Put the task to sleep and wait for the DMA Completion
            ** routine to wake it back up.
            */
            TaskSetMyState(PCB_NV_SCRUB_WAIT);
            TaskSwitch();
        }

        /*
        ** DMA completed OK (rc == 0 from NV_DMARequest and the response
        ** packet has errSts == 0). Point to the next region to do the
        ** next scrub cycle on (watch out for end of memory increment).
        */
        if (rc == 0 && gpScrubCompletionRsp->dmaStsCtrl.errSts == 0)
        {
            /* Update the memory region to check. */
            memoryOffset += SCRUB_TRANSFER_LENGTH;

            /* Check if the end of the memory was reached. */
            if (memoryOffset >= mem_size)
            {
                fprintf(stderr, "NV_ScrubTask-Scrub Complete (0x%8.8x%8.8x)\n",
                        (UINT32)(memoryOffset>>32),
                        (UINT32)memoryOffset);

                /* Reached the end, so restart from zero.  */
                memoryOffset = 0;
            }
        }
        else
        {
            fprintf(stderr, "NV_ScrubTask-DMA Request failed: offset 0x%8.8x%8.8x, rc 0x%x, errSts 0x%x\n",
                    (UINT32)(memoryOffset>>32), (UINT32)memoryOffset,
                    rc, gpScrubCompletionRsp->dmaStsCtrl.errSts);
        }
    }
}


/*
******************************************************************************
**
**  @brief  Completion Function for NV_ScrubTask DMA requests.
**
**  @param  NV_DMA_RSP_PKT *pRsp - DMA Response Packet Pointer
**
**  @return None
**
******************************************************************************
**/
void NV_ScrubCompletion(NV_DMA_RSP_PKT* pRsp)
{
    if (pRsp->dmaStsCtrl.errSts != 0)
    {
        fprintf(stderr, "@@@@@@@@@@ NV_ScrubCompletion DMA COMPLETION FUNCTION (error status = 0x%X) @@@@@@@@@@\n",
                pRsp->dmaStsCtrl.errSts);
    }

    /* Copy the response packet so the scrub task can use it. */
    memcpy(gpScrubCompletionRsp, pRsp, sizeof(NV_DMA_RSP_PKT));

    /* Free the response packet */
    s_Free(pRsp, sizeof(*pRsp), __FILE__, __LINE__);

    /* Awaken the NV Scrub Task */
    TaskReadyByState(PCB_NV_SCRUB_WAIT);
}
#endif /* FRONTEND */


#ifdef FRONTEND
/**
******************************************************************************
**
**  @brief      MM_TestTaskStart - Function to start the MM Test task.
**
**  @param      MRMMTEST_REQ* pReq - MRP request packet.
**
**  @return     NONE
**
******************************************************************************
**/
void MM_TestTaskStart(MRMMTEST_REQ* pReq)
{
    MRMMTEST_REQ*   pLocalReq;

    /*
    ** Make a copy of the request packet to give to the
    ** handler task. The handler task will free
    ** this memory.
    */
    pLocalReq = s_MallocW(sizeof(MRMMTEST_REQ), __FILE__, __LINE__);
    memcpy(pLocalReq, pReq, sizeof(MRMMTEST_REQ));

    CT_fork_tmp = (unsigned long)"MM_TestTask";
    TaskCreate3(C_label_referenced_in_i960asm(MM_TestTask),
                MM_TEST_PRIORITY,
                (UINT32)pLocalReq);
}


/**
******************************************************************************
**
**  @brief      MM_TestTask - Task to handle the requests from the MM/NV
**                            test driver. Some of these operations will
**                            cause controller faults so it is better to
**                            use a task to do the work.
**
**  @param      UINT32 pPCB - pointer to the PCB (UNUSED)
**
**  @param      UINT32 pri - priority for this task (UNUSED)
**
**  @param      MRMMTEST_REQ* pReq - Copy of the MRP request packet, this
**                                   task will free the memory.
**
**  @return     NONE
**
******************************************************************************
**/
void MM_TestTask(UINT32 pPCB UNUSED, UINT32 pri UNUSED, MRMMTEST_REQ *pReq)
{
    UINT8 invValue[4] = {0xEF, 0xBE, 0xAD, 0xDE};
    union {
        UINT32  word;
        UINT8   bytes[4];
    } sig;

    switch (pReq->option)
    {
        case MMTEST_ECC_SINGLE:
            fprintf(stderr, "MM_TestMRP-Inject single-bit ECC error (NOT YET AVAILABLE)\n");
            break;

        case MMTEST_ECC_MULTI:
            fprintf(stderr, "MM_TestMRP-Inject multi-bit ECC error (NOT YET AVAILABLE)\n");
            break;

        case MMTEST_FAIL:
            fprintf(stderr, "MM_TestMRP-Fail MM card\n");
            nv_ProcessFatalEvent(NV_FATAL_USER_FAILED, 0);
            break;

        case MMTEST_WCSIG:
            fprintf(stderr, "MM_TestMRP-Invalidate WC signature (\n");
            sig.word = DATA_FLUSHED;
            MM_Write((UINT32)&((WT*)gWC_NV_Mirror.wcctFEHandle)->signature1,
                     sig.bytes,
                     sizeof(((WT*)gWC_NV_Mirror.wcctFEHandle)->signature1));
            break;

        case MMTEST_WCSIG_SN:
            fprintf(stderr, "MM_TestMRP-Invalidate controller S/N in WC signature\n");
            MM_Write((UINT32)&((WT*)gWC_NV_Mirror.wcctFEHandle)->cSerial,
                     invValue,
                     sizeof(((WT*)gWC_NV_Mirror.wcctFEHandle)->cSerial));
            break;

        case MMTEST_WCSIG_SEQNO:
            fprintf(stderr, "MM_TestMRP-Invalidate sequence number in WC signature\n");
            MM_Write((UINT32)&((WT*)gWC_NV_Mirror.wcctFEHandle)->seq,
                     invValue,
                     sizeof(((WT*)gWC_NV_Mirror.wcctFEHandle)->seq));
            break;

        default:
            break;
    }

    s_Free(pReq, sizeof(MRMMTEST_REQ), __FILE__, __LINE__);
}
#endif


/*
******************************************************************************
**  NV_GetStatus
**
**  @brief  This routine returns the NV_Memory status (from mmInfo->status)
**
**  @param  None
**
**  @return status
**
******************************************************************************
*/
UINT32 NV_GetStatus(void)
{
    /* Check whether MicroMemory card & driver are available & accessible. */
    if (mmInfo == NULL)
    {
#ifdef BACKEND
        return(-1);
#else /* FRONTEND */
        fprintf(stderr, "NV_GetStatus:  Card not yet initialized or is inaccessible - waiting...\n");
        /*
        ** Set task status to wait for MM Info MRP to be received, then
        ** give up CPU until re-awakened.
        */
        TaskSetMyState(PCB_MM_WAIT);
        TaskSwitch();
        fprintf(stderr, "NV_GetStatus:  Awakened from MM_WAIT state.\n");
#endif
    }

    return mmInfo->status;
}


#ifdef  STANDALONE
/*
******************************************************************************
**  main()
**
**  @brief  This is sample code for stand alone testing of the MM_ driver
**  code defined above. Of particular interest is the way the DMA functionality
**  has been tested, including chained DMA of the MM-5425CN. DMA'able memory
**  is allocated from shared memory segments from the xio3d kernel module.
**  Also of interest is the way the mmap'ed CSR and MM-memory is unmapped,
**  for orderly shutdown.
**  @param  value - none, no input parameters
**
**  @return -1 if failure, 0 if success
**
******************************************************************************
*/
int main(void)
{
    int i, ret;
    ret = MM_init();
    fprintf(stderr, "return value of MM_init() = %d\n", ret);

#ifdef CHAINED_DMA_TEST_CODE
    /* try chained DMA transfer     */
    {
        unsigned long long *pull;
        unsigned long uli;
        memset(map_shared[XIO_FE], 0, (SIZE_4MEG*2));
        memset(map_shared[XIO_BE], 0, (SIZE_4MEG*2));
        memset(map_shared[XIO_CCB], 0, (SIZE_4MEG*2));
        fprintf(stderr, "Try chained DMA transfer......\n");
        pull = (unsigned long long *)map_shared[XIO_FE];
        /* memset((char *)pull, 0, 1024); */
        for (uli = 0; uli< SIZE_4MEG; uli++)
        {
            *(map_shared[XIO_BE] + uli) = 'H';
        }

        fprintf(stderr, "First, direction from BE->Card......\n");
        pull[0] = xio3d_drvinfo.mem_regions[XIO_BE].phys;
        pull[1] = SIZE_4K; /* 4194304 */
        pull[2] = SIZE_4MEG;
        pull[3] = (unsigned long)(xio3d_drvinfo.mem_regions[XIO_FE].phys + (8*6));
        pull[4] = xio3d_drvinfo.mem_regions[XIO_FE].phys + SIZE_4MEG + 2048;
        pull[5] = (MM_DMA_GO | MM_DMA_WRT | MM_DMA_SEM_EN | MM_DMA_CHN_EN | MM_DMA_PCI_MEM_RD_MULT | MM_DMA_COMP | MM_DMA_CHN_COMP);

        fprintf(stderr, "Second, direction from Card->CCB......\n");
        pull[6] = xio3d_drvinfo.mem_regions[XIO_CCB].phys;
        pull[7] = SIZE_4K;
        pull[8] = SIZE_4MEG;
        pull[9] = (unsigned long)(xio3d_drvinfo.mem_regions[XIO_FE].phys + (8*12));
        pull[10] = xio3d_drvinfo.mem_regions[XIO_FE].phys + SIZE_4MEG + 2048 + 8;
        pull[11] = (MM_DMA_GO | MM_DMA_RD | MM_DMA_SEM_EN | MM_DMA_CHN_EN | MM_DMA_PCI_MEM_WRT | MM_DMA_COMP | MM_DMA_CHN_COMP);
        /* fprintf(stderr, "using shared memory segment of BE for DMA transfer, address 0x%X\n",
        map_shared[XIO_BE]); */
        fprintf(stderr, "Third, direction from Card->FE......\n");
        pull[12] = xio3d_drvinfo.mem_regions[XIO_FE].phys + 1024;
        pull[13] = SIZE_4K;
        pull[14] = SIZE_4MEG;
        /* pull[15] = (unsigned long)(pull + 12); */
        pull[16] = xio3d_drvinfo.mem_regions[XIO_FE].phys + SIZE_4MEG + 2048 + 8 + 8;
        pull[17] = (MM_DMA_GO | MM_DMA_RD | MM_DMA_SEM_EN | MM_DMA_PCI_MEM_WRT | MM_DMA_COMP | MM_DMA_CHN_COMP);
        ret = mm_Chain_DMA(xio3d_drvinfo.mem_regions[XIO_FE].phys , 1);
        fprintf(stderr, "return value of function MM_CHAIN_DMA() ret = %d\n\n", ret);
        for (i=16; i>0; i--)
        fprintf(stderr, "last 16 bytes : pMMemory[%d] = %c\n",
                (SIZE_4K + SIZE_4MEG)-i, pMMemory[(SIZE_4K + SIZE_4MEG)-i]);
        fprintf(stderr, "Done with dump of MM Card Memory....\n\n");
        for (i=16; i>0; i--)
        fprintf(stderr, "last 16 bytes : map_shared[XIO_CCB][%d] = %c\n",SIZE_4MEG-i, map_shared[XIO_CCB][SIZE_4MEG-i]);
        fprintf(stderr, "Done with dump of CCB Memory....\n\n");
        for (i=16; i>0; i--)
        fprintf(stderr, "last 16 bytes : map_shared[XIO_FE][%d] = %c\n",1024 + SIZE_4MEG - i, map_shared[XIO_FE][1024+SIZE_4MEG-i]);
        fprintf(stderr, "Done with dump of FE Memory....\n\n");
        fprintf(stderr, "semaphore register1 0x%X = 0x%X\n",
                (UINT32)map_shared[XIO_FE] + SIZE_4MEG + 2048,
                *(UINT32 *)(map_shared[XIO_FE]+SIZE_4MEG + 2048));
        fprintf(stderr, "semaphore register2 0x%X = 0x%X\n",
                (UINT32)map_shared[XIO_FE] + SIZE_4MEG + 2056,
                *(UINT32 *)(map_shared[XIO_FE]+SIZE_4MEG + 2056));
        fprintf(stderr, "semaphore register3 0x%X = 0x%X\n",
                (UINT32)map_shared[XIO_FE] + SIZE_4MEG + 2064,
                *(UINT32 *)(map_shared[XIO_FE]+SIZE_4MEG + 2064));
    }
#endif /* CHAINED_DMA_TEST_CODE */

#ifdef DMA_TEST_CODE
{
    unsigned long uli;
    fprintf(stderr, "using shared memory segment of CCB for DMA transfer, address 0x%X\n",
            (UINT32)map_shared[XIO_CCB]);
    for (uli = 0; uli< 4194304; uli++)
    {
        *(map_shared[XIO_CCB] + uli) = 'Q';
    }
    pMM->dmaDesc.pciDataAddr = xio3d_drvinfo.mem_regions[XIO_CCB].phys;
    pMM->dmaDesc.localAddr = 0x40;
    pMM->dmaDesc.xferSize = SIZE_4MEG;
    pMM->dmaDesc.stsCtrlLo = 0x3 | 1<<30 | 1<<31;
/*    pMM->dmaDesc.stsCtrlLo = 0x3; */
    fprintf(stderr, "reading after DMA transfer, from mmaped memory 0x%X\n",
            (UINT32)pMMemory + 0x40);
    pMM->windowMap = 0;
    for (uli = 0; uli< 4194304; uli++)
    {
        fprintf(stderr, "data : %c\n", pMMemory[0x40 + uli]);
    }

    fprintf(stderr, "using shared memory segment of BE for DMA transfer, address 0x%X\n",
            (UINT32)map_shared[XIO_BE]);
    pMM->dmaDesc.pciDataAddr = xio3d_drvinfo.mem_regions[XIO_BE].phys;
    pMM->dmaDesc.localAddr = 0x40;
    pMM->dmaDesc.xferSize = SIZE_4MEG;
    pMM->dmaDesc.stsCtrlLo = 0x1;
    fprintf(stderr, "reading after DMA transfer, from BE mmaped shared memory 0x%X\n",
            (UINT32)map_shared[XIO_BE]);

    /* pMM->windowMap = 0;  */
    for (uli = 0; uli< 4194304; uli++)
    {
        fprintf(stderr, "data : %c\n", *(map_shared[XIO_BE]+uli));
    }

    /* try chained DMA transfer     */
    {
        unsigned long long *pull;
        fprintf(stderr, "Try chained DMA transfer......\n");
        pull = (unsigned long long *)map_shared[XIO_FE];
        memset((char *)pull, 0, 1024);
        /*
        pull[0] = xio3d_drvinfo.mem_regions[XIO_BE].phys;
        pull[1] = 4194304 + 0x40;
        pull[2] = SIZE_4MEG;
        pull[3] = (unsigned long)(pull + 6);
        pull[4] = xio3d_drvinfo.mem_regions[XIO_FE].phys + 512;
        pull[5] = 0x17;
        */
        pull[0] = xio3d_drvinfo.mem_regions[XIO_CCB].phys;
        pull[1] = 4194304 + 0x40;
        pull[2] = SIZE_4MEG;
        pull[3] = (unsigned long)(pull + 12);
        pull[4] = xio3d_drvinfo.mem_regions[XIO_FE].phys + 512 + 8;
        pull[5] = 0x11;
        fprintf(stderr, "using shared memory segment of BE for DMA transfer, address 0x%X\n",
                (UINT32)map_shared[XIO_BE]);
        for (uli = 0; uli< SIZE_4MEG; uli++)
        {
            *(map_shared[XIO_BE] + uli) = 'V';
        }
        pMM->dmaDesc.pciDataAddr = xio3d_drvinfo.mem_regions[XIO_BE].phys;
        pMM->dmaDesc.localAddr = 4194304 + 0x40;
        pMM->dmaDesc.xferSize = SIZE_4MEG;
        pMM->dmaDesc.pciDescAddr = xio3d_drvinfo.mem_regions[XIO_FE].phys;
        pMM->dmaDesc.stsCtrlLo = 0x17;
        pMM->windowMap = 0;
        fprintf(stderr, "reading after DMA transfer, from mmaped memory 0x%X\n",
                (UINT32)pMMemory + 4194304 + 0x40);
        for (uli = 0; uli< 4194304; uli++)
        {
            fprintf(stderr, "data : %c\n", pMMemory[0x40 + SIZE_4MEG + uli]);
        }
        fprintf(stderr, "reading after DMA transfer, from mmaped CCB memory 0x%X\n",
                (UINT32)map_shared[XIO_CCB]);
        for (uli = 0; uli< SIZE_4MEG; uli++)
        {
            fprintf(stderr, "data : %c\n", map_shared[XIO_CCB][uli]);
        }
        fprintf(stderr, "semaphore register1 0x%X = 0x%X\n",
                (UINT32)map_shared[XIO_FE] + 512,
                (UINT32)*((unsigned long *)map_shared[XIO_FE]+512));
        fprintf(stderr, "semaphore register2 0x%X = 0x%X\n",
                (UINT32)map_shared[XIO_FE] + 520,
                (UINT32)*((unsigned long *)map_shared[XIO_FE]+520));
    }
}
#endif /* DMA_TEST_CODE */

    /* Close the file descriptor, then unmap all the segments.  */
    if (close(xiofd) != 0)
    {
        int save_errno = errno;

        xiofd = -1;
        perror("close failed");
        return save_errno;          /* perror() might change it. */
    }
    xiofd = -1;

    fprintf(stderr, "close done\n");

    for (i = 0; i < XIO3D_NSHM_MAX; ++i)
    {
        if (map_shared[i])
        {
            ret = munmap(map_shared[i], xio3d_drvinfo.mem_regions[i].size);
            if (ret)
            {
                int save_errno = errno;

                perror("munmap failed");
                /* perror() might change errno. */
                fprintf(stderr,"errno = %d\n",save_errno);
            }
            fprintf(stderr, "munmap[%d] returned %d\n", i, ret);
        }
    }
    {
        struct pci_devs *dev;
        unsigned long   rem;
        unsigned long   total;
        for (i = 0, dev = &gPCIdevs[0]; i < XIO3D_NIO_MAX; ++i, ++dev)
        {
            if (dev->device == MMCARDID)
            {
                break;
            }
        }
        if (pMMemory)
        {
            rem = dev->start[1] % PAGE_SIZE;
            total = (dev->len[1] + rem);
            if (total > PAGE_SIZE)
            {
                if (total % PAGE_SIZE)
                {
                    total += PAGE_SIZE - (total % PAGE_SIZE);
                }
            }
            ret = munmap((UINT8*)pMMemory-rem,
                         (dev->len[1] > PAGE_SIZE) ? total : PAGE_SIZE);
            if (ret)
            {
                int save_errno = errno;

                perror("munmap failed");
                fprintf(stderr,"errno = %d\n",save_errno);
            }
            fprintf(stderr, "munmap of 16MB card memory returned %d\n", ret);
        }
        if (pMM)
        {
            rem = dev->start[0] % PAGE_SIZE;
            total = (dev->len[0] + rem);
            if (total > PAGE_SIZE)
            {
                if (total % PAGE_SIZE)
                {
                    total += PAGE_SIZE - (total % PAGE_SIZE);
                }
            }
            ret = munmap((UINT8*)pMM - rem,
                         (dev->len[0] > PAGE_SIZE) ? total : PAGE_SIZE);
            if (ret)
            {
                int save_errno = errno;

                perror("munmap failed");
                fprintf(stderr,"errno = %d\n", save_errno);
                fprintf(stderr,"pMM = 0x%X\n", (UINT32)pMM);
            }
            fprintf(stderr, "munmap of CSR 256 byte config space returned %d\n", ret);
        }
    }
    return 0;
} /* main */
#endif /* STANDALONE */

#ifdef DIAGNOSTICS
#ifdef ERROR_STATUS_ENABLE
UINT64 FirstErrorDataLog(void)
{
    if (!pMM)
    {
        fprintf(stderr, "FirstErrorDataLog:  Card not initialized or is inaccessible!\n");
        return(-1);
    }
    fprintf(stderr, "Error Counter = 0x%X\n", pMM->errCnt);
    return (pMM->errFirst.data);
}

UINT64 FirstErrorAddrLog(void)
{
    if (!pMM)
    {
        fprintf(stderr, "FirstErrorAddrLog:  Card not initialized or is inaccessible!\n");
        return(-1);
    }
    fprintf(stderr, "Error Counter = 0x%X\n", pMM->errCnt);
    return (pMM->errFirst.infoAddrReg);
}

UINT64 FirstErrorAddr(void)
{
    if (!pMM)
    {
        fprintf(stderr, "FirstErrorAddr:  Card not initialized or is inaccessible!\n");
        return(-1);
    }
    fprintf(stderr, "Error Counter = 0x%X\n", pMM->errCnt);
    return (pMM->errFirst.infoAddrReg & 0xFFFFFFFFFFLL);
}

UINT8 FirstErrorSyndrome(void)
{
    if (!pMM)
    {
        fprintf(stderr, "FirstErrorSyndrome:  Card not initialized or is inaccessible!\n");
        return(-1);
    }
    fprintf(stderr, "Error Counter = 0x%X\n", pMM->errCnt);
    return (pMM->errFirst.synBits);
}

UINT8 FirstErrorCheckBits(void)
{
    if (!pMM)
    {
        fprintf(stderr, "FirstErrorCheckBits:  Card not initialized or is inaccessible!\n");
        return(-1);
    }
    fprintf(stderr, "Error Counter = 0x%X\n", pMM->errCnt);
    return (pMM->errFirst.chkBits);
}

UINT64 LastErrorDataLog(void)
{
    if (!pMM)
    {
        fprintf(stderr, "LastErrorDataLog:  Card not initialized or is inaccessible!\n");
        return(-1);
    }
    fprintf(stderr, "Error Counter = 0x%X\n", pMM->errCnt);
    return (pMM->errLast.data);
}

UINT64 LastErrorAddrLog(void)
{
    if (!pMM)
    {
        fprintf(stderr, "LastErrorAddrLog:  Card not initialized or is inaccessible!\n");
        return(-1);
    }
    fprintf(stderr, "Error Counter = 0x%X\n", pMM->errCnt);
    return (pMM->errLast.infoAddrReg);
}

UINT64 LastErrorAddr(void)
{
    if (!pMM)
    {
        fprintf(stderr, "LastErrorAddr:  Card not initialized or is inaccessible!\n");
        return(-1);
    }
    fprintf(stderr, "Error Counter = 0x%X\n", pMM->errCnt);
    return (pMM->errLast.infoAddrReg & 0xFFFFFFFFFFLL);
}

UINT8 LastErrorSyndrome(void)
{
    if (!pMM)
    {
        fprintf(stderr, "LastErrorSyndrome:  Card not initialized or is inaccessible!\n");
        return(-1);
    }
    fprintf(stderr, "Error Counter = 0x%X\n", pMM->errCnt);
    return (pMM->errLast.synBits);
}

UINT8 LastErrorCheckBits(void)
{
    if (!pMM)
    {
        fprintf(stderr, "LastErrorCheckBits:  Card not initialized or is inaccessible!\n");
        return(-1);
    }
    fprintf(stderr, "Error Counter = 0x%X\n", pMM->errCnt);
    return (pMM->errLast.chkBits);
}
#endif /* ERROR_STATUS_ENABLE */
#endif /* DIAGNOSTICS */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
