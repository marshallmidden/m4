/* $Id: error_handler.c 146565 2010-08-29 00:22:26Z m4 $ */
/*============================================================================
** FILE NAME:       error_handler.c
** MODULE TITLE:    Error handling routines
**
** DESCRIPTION:     The functions in this module are used for handling
**                  unexpected hardware errors.
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "error_handler.h"

#include "cps_init.h"
#include "debug_files.h"
#include "errorCodes.h"
#include "idr_structure.h"
#include "L_Misc.h"
#include "mode.h"
#include "SerBuff.h"
#include "timer.h"
#include "xk_kernel.h"

#include "signal.h"
#include "asm/bitops.h"
#include "L_CCBCrashDump.h"
#include "XIO_Macros.h"
#include "led_codes.h"

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
ERRORTRAP_DATA_CPU_REGISTERS cpuRegisters = {
    {                           /* UINT32 gRegisters[I960_NUM_G_REGS] */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {                           /* UINT32 rRegisters[I960_NUM_R_REGS] */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

/*****************************************************************************
** Private variables
*****************************************************************************/
#define LIN_ERRTRAP_CCB         1

static volatile unsigned long errTrapEntered = 0;

/*****************************************************************************
** Code Start
*****************************************************************************/

/*****************************************************************************
** FUNCTION NAME: error08
**
** PARAMETERS:  none
**
** DESCRIPTION: Handle error condition err08:
**              LL_QueMessageToSend: invalid function number
**
** RETURNS:     Does not return - function calls DeadLoop, which does
**              not return.
******************************************************************************/

NORETURN void error08(void)
{
    DeadLoop(LED_CODE_ERR08, TRUE);
}


/*****************************************************************************
** FUNCTION NAME: error11
**
** PARAMETERS:  none
**
** DESCRIPTION: Handle error condition err11:
**              LL_NormalTargetProcessing: invalid function number
**
** RETURNS:     Does not return - function calls DeadLoop, which does
**              not return.
******************************************************************************/

NORETURN void error11(void)
{
    DeadLoop(LED_CODE_ERR11, TRUE);
}

/*****************************************************************************
** FUNCTION NAME: CaptureRegs
**
** PARAMETERS:  None
**
** DESCRIPTION: Capture x86 regs and stick them in i960 structure.
**
** RETURNS:     Nothing
******************************************************************************/
static void CaptureRegs(void)
{
    /*
     * Copy x86 registers into i960 g registers
     * g0  = esp    - 32bit Stack Pointer (in SS segment).
     * g1  = ebp    - 32bit Pointer to data on the stack (in SS segment).
     * g2  = edi    - 32bit Pointer to data (or destination) in the segment
     *                      pointed to by the ES register; destination pointer
     *                      for string operations.
     * g3  = esi    - 32bit Pointer to data in the segment pointed to by the
     *                      DS register; source pointer for string operations.
     * g4  = edx    - 32bit I/O pointer.
     * g5  = ecx    - 32bit Counter for string and loop operations.
     * g6  = ebx    - 32bit Pointer to data in DS segment.
     * g7  = eax    - 32bit Accumulator for operands and results data.
     * g8  = cs     - 16bit Segment selector.
     * g9  = ds     - 16bit Segment selector.
     * g10 = ss     - 16bit Segment selector.
     * g11 = es     - 16bit Segment selector.
     * g12 = fs     - 16bit Segment selector.
     * g13 = gs     - 16bit Segment selector.
     * g14 = eflags - 32bit Program status and control register.
     */
    cpuRegisters.gRegisters[0] = get_esp();
    cpuRegisters.gRegisters[1] = get_ebp();
    cpuRegisters.gRegisters[2] = get_edi();
    cpuRegisters.gRegisters[3] = get_esi();
    cpuRegisters.gRegisters[4] = get_edx();
    cpuRegisters.gRegisters[5] = get_ecx();
    cpuRegisters.gRegisters[6] = get_ebx();
    cpuRegisters.gRegisters[7] = get_eax();
    cpuRegisters.gRegisters[8] = get_cs();
    cpuRegisters.gRegisters[9] = get_ds();
    cpuRegisters.gRegisters[10] = get_ss();
    cpuRegisters.gRegisters[11] = get_es();
    cpuRegisters.gRegisters[12] = get_fs();
    cpuRegisters.gRegisters[13] = get_gs();
    cpuRegisters.gRegisters[14] = get_eflags();
}

/*****************************************************************************
** FUNCTION NAME: DeadLoopInterrupt
**
** PARAMETERS:  signal - Passed in by Signal handler.
**              data - Data provided with signal.
**              sinfo - Pointer to siginfo_t structure from signal.
**              ebp - ebp register value from signal.
**
** DESCRIPTION: Passes through to DeadLoop.
**
** RETURNS:     Does not return.
******************************************************************************/
void DeadLoopInterrupt(INT32 sig, UINT32 data, siginfo_t *sinfo, UINT32 ebp)
{
    /* Set the process state. */
    if (test_and_set_bit(LIN_ERRTRAP_CCB, &errTrapEntered) == 0)
    {
        fprintf(stderr, "%s+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n",
                L_MsgPrefix);
        fprintf(stderr, "%s+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n",
                L_MsgPrefix);
        fprintf(stderr, "%s+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n",
                L_MsgPrefix);
        fprintf(stderr, "%s DeadLoopInterrupt ( %s )\n",
                L_MsgPrefix, L_LinuxSignalToString(sig, data));
        dprintf(DPRINTF_DEFAULT, "CCB DeadLoopInterrupt ( %s )\n",
                L_LinuxSignalToString(sig, data));

        /* Check for a platform signal. */
        if (sig == XIO_PLATFORM_SIGNAL)
        {
            switch (data)
            {
                    /* If this is a shutdown, sit here and wait until PAM knocks us off. */
                case ERR_EXIT_SHUTDOWN:
                    dprintf(DPRINTF_DEFAULT, "CCB DeadLoopInterrupt ( %s ) Waiting on PAM to knock off CCB\n",
                            L_LinuxSignalToString(sig, data));
                    while (1)
                    {
                        sleep(10);
                    }

                default:
                    break;
            }
        }

        /* Set the processor state. */
        SetProcessorState(IDR_PROCESSOR_STATE_NMI);

        /* Take CCB crash dump. */
        L_CCBCrashDump(sig, sinfo, ebp);

        /* Call DeadLoop. */
        DeadLoop(sig, TRUE);
    }
    else
    {
        /* Prevent deadly loop */
        signal(sig, SIG_DFL);
        dprintf(DPRINTF_DEFAULT, "CCB DeadLoopInterrupt ( %s ) DEADLOOP in progress\n",
                L_LinuxSignalToString(sig, data));
    }
}

/*****************************************************************************
** FUNCTION NAME: DeadLoop
**
** PARAMETERS:  newErrorCode - new error code to flash on the LEDs
**              ForceControllerFastReboot - (TRUE or FALSE)
**
** RETURNS:     Does not return - function loops internally
**
** WARNING:     DeadLoop can be called from Fault, Interrupt, NMI, and mainline
**              code threads.  *** DO NOT *** do anything or call any function
**              that relys upon interrupts or exchanges to occur.
******************************************************************************/
NORETURN void DeadLoop(UINT8 newErrorCode, UINT8 ForceControllerFastReboot)
{
    UINT8 processReset = FALSE;
    UINT8 processDeadLoop = FALSE;

    /*
     * Setting these bits is equivalent to disabling intterrupts.
     * Also disable external signals, we are going down.
     */
    test_and_set_bit(LIN_ERRTRAP_CCB, &errTrapEntered);

    /*
     * Set the processor state to indicate we're in DeadLoop. We can
     * use this flag to determine if we have (or are) handling a fatal
     * error. We need to make sure that the CCB handles the error, otherwise
     * we may end up with front end QLogics that are still online.
     */
    SetProcessorState(IDR_PROCESSOR_STATE_DEAD_LOOP);

    TimerDisable();     /* Disable the timer task. (our only interrupt) */

#ifdef M4_ABORT
    abort();            /* Stop after timer killed, but before we kill all pthreads. */
#endif /* M4_ABORT */

    /*
     * Ignore any following "XIO_PLATFORM_SIGNAL" and
     * and segmentation faults. We need to get down now.
     */
    signal(XIO_PLATFORM_SIGNAL, SIG_IGN);
    signal(SIGSEGV, SIG_IGN);

    /*
     * Print the error code to the serial port so it gets saved in NVRAM
     */
    dprintf(DPRINTF_DEFAULT, "\n\n  DeadLoop: ErrorCode: 0x%x\n", newErrorCode);

    /*
     * Don't capture registers for the error types that already capture
     * their own copy of the event.  These mainly include NMI events and
     * processor fault events.
     */
    CaptureRegs();

    /*
     * Determine the correct course of action for the runtime death.
     * We have four options:
     *   1) Suicide - (normal customer mode - 2 way)
     *   2) Reboot  - CCB does a self-reset, leaves the FE/BE processes alone
     *                (normal customer mode - 1 way)
     *   3) Debug A - Reset FE QLogic cards and go into a tight loop
     *
     * NOTE: Don't take any action at this point - We print messages
     *       here so that it makes it into the NVRAM backtrace data.
     *       Action is taken later on the value in blinkCounter.
     */
    if (ForceControllerFastReboot || PowerUpComplete() == true)
    {
        dprintf(DPRINTF_DEFAULT, "Powerup is complete and/or suicide is forced\n");

        /*
         * Check for single controller condition. This impacts how the CCB
         * handles failure. We might just need to reset only the CCB.
         */
        if (!ForceControllerFastReboot && ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()) == 1)
        {
            dprintf(DPRINTF_DEFAULT, "This is the only operational controller (allow restart)\n");

            if (SuicideDisableSwitch())
            {
                dprintf(DPRINTF_DEFAULT, "  *** CCB restart is disabled via diag switch ***\n");
                processDeadLoop = TRUE;
            }
            else if (TestModeBit(MD_FM_RESTART_DISABLE))
            {
                dprintf(DPRINTF_DEFAULT, "  *** CCB restart is disabled via mode bit ***\n");
                processDeadLoop = TRUE;
            }
            else
            {
                dprintf(DPRINTF_DEFAULT, "  Resetting all processes\n");
                processReset = TRUE;
            }
        }
        else
        {
            if (!ForceControllerFastReboot)
            {
                dprintf(DPRINTF_DEFAULT, "Active controller count != one (allow suicide)\n");
            }
            else
            {
                dprintf(DPRINTF_DEFAULT, "Force controller fast restart\n");
//                processDeadLoop = TRUE;       // NOT ANY MORE!
            }

            if (SuicideDisableSwitch())
            {
                dprintf(DPRINTF_DEFAULT, "  *** ControllerSuicide is disabled via diag switch ***\n");
                processDeadLoop = TRUE;
            }
            else if (TestModeBit(MD_CONTROLLER_SUICIDE_DISABLE))
            {
                dprintf(DPRINTF_DEFAULT, "  *** ControllerSuicide is disabled via mode bit ***\n");
                processDeadLoop = TRUE;
            }
            else
            {
                dprintf(DPRINTF_DEFAULT, "  Fast reboot of controller\n");
            }
        }
    }
    else
    {
        /*
         * Scenario:
         *     Controller is up and running in a normal 2-way configuration
         * The other controller falls out (suicides) leaving this controller
         * as the only operational controller. At some point in time later,
         * this controller's CCB runs into some software error, which comes
         * through this function and the CCB is reset but not the processor
         * board. The CCB restarts and runs into another fatal error before
         * power up is complete.
         *     In this case, the CCB does not have the knowledge necessary to
         * determine if it should only reset itself or the entire controller.
         * Power up is not complete, so it can't look at the ACM information.
         * The best alternative in this case is to let the CCB enter the
         * LED blink code. Doing this, we end up with a controller without
         * an operational CCB, which means that unfailing the other controller
         * can't be done (or shouldn't, since the CCB controls the unfail).
         * In order to get back to a working 2-way system, this controller will
         * need to be power cycled... causing a disruption to the servers.
         */
        dprintf(DPRINTF_DEFAULT, "Powerup is not complete (do not allow suicide or restart)\n");
    }

    /*
     * Save the backtrace information, but ONLY if the soft reset pattern
     * indicates that we're not handling an error in powerup after
     * restarting the CCB in a 1-way configuration. Preserve the backtrace
     * information from the initial error.
     */
    if ((PowerUpComplete() == true) ||
        (IDRData.resetStatus != IDR_SOFT_RUNTIME_ERROR_PATTERN))
    {
        /*
         * Copy critical data to NVRAM
         * This also saves off the serial trace, so all DebugPrintf's below
         * this point will not be saved in the NVRAM backtrace.
         */
        CopyBacktraceDataToNVRAM();

        /* Print callstack to serial port. */
        dprintf(DPRINTF_DEFAULT, "%s",
                (char *)NVRAMData.errortrapDataRun.errorSnapshot.callStack);
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "Not saving backtrace due to previous runtime error\n");
    }

    sprintf(stackDmp,
            "\n  Registers:\n"
            "  esp:     %08X\n"
            "  ebp:     %08X\n"
            "  edi:     %08X\n"
            "  esi:     %08X\n"
            "  edx:     %08X\n"
            "  ecx:     %08X\n"
            "  ebx:     %08X\n"
            "  eax:     %08X\n"
            "  cs:      %04X\n"
            "  ds:      %04X\n"
            "  ss:      %04X\n"
            "  es:      %04X\n"
            "  fs:      %04X\n"
            "  gs:      %04X\n"
            "  eflags:  %08X\n",
            cpuRegisters.gRegisters[0],
            cpuRegisters.gRegisters[1],
            cpuRegisters.gRegisters[2],
            cpuRegisters.gRegisters[3],
            cpuRegisters.gRegisters[4],
            cpuRegisters.gRegisters[5],
            cpuRegisters.gRegisters[6],
            cpuRegisters.gRegisters[7],
            cpuRegisters.gRegisters[8],
            cpuRegisters.gRegisters[9],
            cpuRegisters.gRegisters[10],
            cpuRegisters.gRegisters[11],
            cpuRegisters.gRegisters[12],
            cpuRegisters.gRegisters[13], cpuRegisters.gRegisters[14]);

    dprintf(DPRINTF_DEFAULT, "%s", stackDmp);

    if (processDeadLoop == TRUE)
    {
        fprintf(stderr, "DeadLoop: processDeadLoop exit 0x%x\n", ERR_EXIT_DEADLOOP);
        errExit(ERR_EXIT_DEADLOOP);
    }
    else if (processReset==TRUE)
    {
        /* Exit informing PAM of the situation. */
        fprintf(stderr, "DeadLoop: all processReset exit 0x%x\n", ERR_EXIT_RESET_ALL);
        errExit(ERR_EXIT_RESET_ALL);
    }
    else
    {
        /* We only get here if we are NOT directed by mode bit or by diag switch. */
// NOTDONEYET -- auto-activate        fprintf(stderr, "DeadLoop: default exit 0x%x\n", ERR_EXIT_BVM_RESTART);
// NOTDONEYET -- auto-activate        errExit(ERR_EXIT_BVM_RESTART);
        fprintf(stderr, "DeadLoop: default exit 0x%x\n", ERR_EXIT_DEADLOOP);
        errExit(ERR_EXIT_DEADLOOP);
    }

    /* Core on Abort. */
    signal(SIGABRT, SIG_DFL);

    /* Sleep for a quarter of a second, to allow signals to be processed. */
    usleep(250);

    /* Abort */
    abort();
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
