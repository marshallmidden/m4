/* $Id: ses.c 163153 2014-04-11 23:33:11Z marshall_midden $*/
/**
******************************************************************************
**
**  @file   ses.c
**
**  @brief  SCSI Enclosure Services
**
**  Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include <sys/un.h>

#include "CacheSize.h"
#include "cps_init.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "ipc_packets.h"
#include "ipc_sendpacket.h"
#include "kernel.h"
#include "misc.h"
#include "MR_Defs.h"
#include "PacketInterface.h"
#include "PI_DiskBay.h"
#include "PI_PDisk.h"
#include "PI_Utils.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "ses.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "PI_Stats.h"
#include "X1_AsyncEventHandler.h"
#include <byteswap.h>
#include "CT_history.h"
#include "EL.h"

/*****************************************************************************
 ** Private defines
 *****************************************************************************/

/*#define DEBUG_SES_LED_CONTROL*/

#define tm_const_ntohs(x) (UINT16)((UINT16)(((x)>>8)&0xffU)\
                                  |(UINT16)(((x)<<8)&0xff00U))

#define LEDCONTROL_PACKET_TIMEOUT   5000
#define PRESENT_LED_STATE           MAX_CONTROLLERS
#define LED_MAP_EMPTY               0
#define LED_MAP_INPROGRESS          1
#define LED_MAP_INITIALIZED         2
#define LED_MAP_END                 0xFF

#if defined(MODEL_7000) || defined(MODEL_4700)
#define MAX_ISE_CTRLR    2
#define MAX_ISE_CTRLR    2
#define MAX_ISE_DATAPAC  2
#define MAX_ISE_PS       2
#define MAX_ISE_BATTERY  2
#endif /* MODEL_7000 || MODEL_4700 */

/*****************************************************************************
 ** Private variables
 *****************************************************************************/
#if defined(MODEL_3000) || defined(MODEL_7400)
static UINT8 gTaskActiveDiscoverSES = FALSE;
#endif /* MODEL_3000 || MODEL_7400 */

static SES_DEVICE *SESList = NULL;

#if defined(MODEL_7000) || defined(MODEL_4700)
static LOG_ISE_PAGE *page30_static = NULL;          /* Used for debugging page 30 after the fact. */

enum temperature_states
{
    ISE_TEMP_UNINITIALIZED,
    ISE_TEMP_OK_STATE,
    ISE_TEMP_WARNING_STATE,
    ISE_TEMP_ERROR_STATE
};
static ISE_SES_DEVICE *ISESESList = NULL;
static UINT8 ise_chassis_temp_state[MAX_DISK_BAYS];
static struct ise_stats_version_2 ise_old_data[MAX_DISK_BAYS];
#endif /* MODEL_7000 || MODEL_4700 */
static UINT8 bTaskActiveLED = 0;
static S_LIST *ptrAsyncEventLED = NULL;

static UINT8 ledControlMap[MAX_PHYSICAL_DISKS][MAX_CONTROLLERS + 1];
static UINT8 bLedControlTaskActive = 0;
static UINT8 ledControlMapState = LED_MAP_EMPTY;
static S_LIST *pLedControlQueue = NULL;
static UINT16 PerformingSES = 0;
static SES_WWN_TO_SES WWNMap[SES_WWN_MAP_SIZE];
static UINT16 SesReferenceCount = 0;

/*****************************************************************************
 ** Public variables - externed in the header file
 *****************************************************************************/
MUTEX       sesMutex;
UINT16      DiscoveringSES = 1;

#if defined(MODEL_7000) || defined(MODEL_4700)
extern UINT32 ISE_Number_Bays_Used;
struct ise_stats_version_2 ise_data[MAX_DISK_BAYS];

#if 0
static void print_ise_info_version_2(struct ise_info_version_2 *ise);
#endif /* 0 */
#endif /* MODEL_7000 || MODEL_4700 */

/*****************************************************************************
 ** Private function prototypes
 *****************************************************************************/
#if defined(MODEL_3000) || defined(MODEL_7400)
static void DiscoverSES_Task(TASK_PARMS *parms);
static void SESGangLEDCtrl(SES_DEVICE *pSES);
#else  /* MODEL_3000 || MODEL_7400 */
static void BuildIseInfoFromPage30(LOG_ISE_PAGE *page30, struct ise_info_version_2 *ise);
static void *GetIseLogPageByWWN(UINT16 page, UINT64 wwn, UINT16 lun);
static UINT16 SendIseCmd(UINT64 wwn, UINT8 value);
#endif /* MODEL_3000 || MODEL_7400 */

static int  compareChangeLED(void *e1, void *e2);
static LOG_MRP *removeEventAndIterate(LOG_MRP *pEvent);
static void ProcessEventChangeLED(TASK_PARMS *parms);

static void SendLedControlToMaster(IPC_LED_CHANGE *pEvent);
static void LedStateChange(IPC_LED_CHANGE *pEvent);
static void UpdateLeds(void);

/*****************************************************************************
 ** Public function prototypes
 *****************************************************************************/

/*****************************************************************************
 ** Private data structures
 *****************************************************************************/

/* Structure to convert from a 32 bit status bit field gotten via page 0x30,
   to a 64 bit field that ASGSES returned. */

struct status_32_64_convert
{
    UINT32      from_bit;
    UINT64      to_bit;
};

/* ------------------------------------------------------------------------ */

/*****************************************************************************
 ** Code Start
 *****************************************************************************/

/*----------------------------------------------------------------------------
** Function Name: GetSESList()
**
** Comments:
**  This function returns a pointer to the list of SES devices (SESList).
**--------------------------------------------------------------------------*/
SES_DEVICE *GetSESList(void)
{
    return SESList;
}


#if defined(MODEL_3000) || defined(MODEL_7400)

/*----------------------------------------------------------------------------
**  Function Name: SES_page2_Handle
**
**  Comments:   Handle the SES page two voltage, temperature, and current.
**
**  Parameters: pSES     - Pointer to SESList entry working with.
**              page2    - Buffer containing SES page 2 information.
**              pStdLog  - Standard log buffer.
**              pTempLog - Temperature log buffer.
**              pVoltLog - Voltage  log buffer.
**              pCurrLog - Current log buffer.
**
**  Returns:    none.
**--------------------------------------------------------------------------*/
static void SES_page2_Handle(SES_DEVICE *pSES,
                             SES_PAGE_02 *page2,
                             LOG_SES_WWN_SLOT_DAT *pStdLog,
                             LOG_SES_WWN_TEMP_DAT *pTempLog,
                             LOG_SES_WWN_VOLT_DAT *pVoltLog,
                             LOG_SES_WWN_CURR_DAT *pCurrLog,
                             LOG_SES_WWN_SBOD_EXT_DAT *pSBODExtLog)
{
    SES_ELEM_CTRL *pElem;
    UINT32      elemNum;
    UINT32      slot;
    UINT8       deltaC1;
    UINT8       deltaC2;
    UINT8       deltaVC;
    UINT8       deltaC;
    UINT8       oldT;
    UINT32      tempCode;
    LOG_SES_ELEM_CHANGE_DAT elemChangeLog;

    /*
     * Set all the WWNs in the log messages.
     */
    pSBODExtLog->wwn = pStdLog->wwn = pTempLog->wwn = pVoltLog->wwn = pCurrLog->wwn =
        elemChangeLog.wwn = pSES->WWN;

    /*
     * First, clean up the temperature sensors to normalize
     * the temperature.  The temperature returned from the
     * controllers is offset by +20 C.  Subtract off 20 from
     * each sensor to get the real temperature.  If the value
     * is less than 20, set it to zero (we will not report
     * temperatures below zero.)
     */
    if (pSES->Map[SES_ET_TEMP_SENSOR] != SES_ET_INVALID)
    {
        slot = pSES->Map[SES_ET_TEMP_SENSOR] + 1;

        for (elemNum = 0; elemNum < pSES->Slots[SES_ET_TEMP_SENSOR]; elemNum++, slot++)
        {
            pElem = &(page2->Control[slot]);

            if (pElem->Ctrl.Temp.Temp <= 20)
            {
                pElem->Ctrl.Temp.Temp = 0;
            }
            else
            {
                pElem->Ctrl.Temp.Temp -= 20;
            }
        }
    }

    /*
     * Examine each element descriptor.
     */
    for (elemNum = 0; elemNum < SES_ET_MAX_VAL; elemNum++)
    {
        if (pSES->Map[elemNum] == SES_ET_INVALID)
        {
            continue;
        }

        /*
         * Check each slot starting at the first slot plus
         * one.  This is done in order to skip the overall
         * status and get to the status of each of the
         * elements within that set.  We skip the overall
         * in order to prevent logging two messages for
         * each change.  Note that we also do not stop
         * until we hit the slot count since there are
         * really slot plus one entries.
         */
        for (slot = pSES->Map[elemNum] + 1;
             slot <= (UINT32)pSES->Map[elemNum] + pSES->Slots[elemNum]; slot++)
        {
            /*
             * Set the slot number in the log messages.
             */
            pSBODExtLog->slot = pStdLog->slot = pTempLog->slot = pVoltLog->slot =
                pCurrLog->slot = elemChangeLog.slot = slot - pSES->Map[elemNum];

            /*
             * Set the pointer to the element.
             */
            pElem = &page2->Control[slot];

            if (pSES->OldPage2 != NULL)
            {
                /*
                 * Get the deltas for the control 1 and
                 * control 2 fields.
                 */
                deltaC1 = pElem->Ctrl.Generic.Ctrl1 ^ pSES->OldPage2->Control[slot].Ctrl.Generic.Ctrl1;
                deltaC2 = pElem->Ctrl.Generic.Ctrl2 ^ pSES->OldPage2->Control[slot].Ctrl.Generic.Ctrl2;
                deltaVC = pElem->Ctrl.Volt.Ctrl ^ pSES->OldPage2->Control[slot].Ctrl.Volt.Ctrl;
                deltaC = pElem->Ctrl.Current.Ctrl ^ pSES->OldPage2->Control[slot].Ctrl.Current.Ctrl;
            }
            else
            {
                deltaC1 = pElem->Ctrl.Generic.Ctrl1;
                deltaC2 = pElem->Ctrl.Generic.Ctrl2;
                deltaVC = pElem->Ctrl.Volt.Ctrl;
                deltaC = pElem->Ctrl.Current.Ctrl;
            }

            /*
             * Do a check of the common status to see if the element
             * has been removed or reinserted.  This will be logged in a
             * specific message containing the WWN of the enclosure,
             * the element type, the slot, the old state and the new.
             * It will then be up to the logging code to ferret out
             * what should be displayed.
             */
            if (pSES->OldPage2 == NULL)
            {
                /*
                 * This page is new, then check if the value in the
                 * status is not OK.  If not, log it.
                 *
                 * Also check for some items that should not be logged
                 * regardless.  Do not log drives missing since this is
                 * not that uncommon for drive slots to be empty.  Do
                 * not log SES elements or controllers missing for SATA
                 * since we do not support the multiple controller unit.
                 */
                if ((pElem->CommonCtrl & SES_CC_STAT_MASK) != SES_CC_STAT_OK)
                {
                    if ((elemNum != SES_ET_DEVICE) &&
                        ((pSES->devType != PD_DT_SATA_SES) ||
                         ((elemNum != SES_ET_LOOP_STAT) &&
                          (elemNum != SES_ET_CTRL_STAT))))
                    {
                        elemChangeLog.oldState = 0xFF;
                        elemChangeLog.newState = pElem->CommonCtrl & SES_CC_STAT_MASK;
                        elemChangeLog.elemType = elemNum;

                        SendAsyncEvent(LOG_SES_ELEM_CHANGE, sizeof(elemChangeLog),
                                       &elemChangeLog);
                    }
                }
            }
            else if ((pElem->CommonCtrl ^ pSES->OldPage2->Control[slot].CommonCtrl) & SES_CC_STAT_MASK)
            {
                INT32       logcode = LOG_SES_ELEM_CHANGE;

                elemChangeLog.oldState = pSES->OldPage2->Control[slot].CommonCtrl & SES_CC_STAT_MASK;
                elemChangeLog.newState = pElem->CommonCtrl & SES_CC_STAT_MASK;
                elemChangeLog.elemType = elemNum;

                if (elemChangeLog.newState == SES_CC_STAT_OK)
                {
                    logcode = LOG_Info(LOG_SES_ELEM_CHANGE);
                }
                else if (elemNum == SES_ET_AUD_ALARM)
                {
                    logcode = LOG_Warning(LOG_SES_ELEM_CHANGE);
                }
                SendAsyncEvent(logcode, sizeof(elemChangeLog), &elemChangeLog);
            }

            /*
             * Now check the element type and log any deltas
             * to the last page 2 received.
             */
            switch (elemNum)
            {
            case SES_ET_DEVICE:
                /*
                 * Post fault if it changed.
                 */
                if (deltaC2 & SES_C2DEV_FAULT)
                {
                    pStdLog->direction = pElem->Ctrl.Generic.Ctrl2 & SES_C2DEV_FAULT;
                    SendAsyncEvent(pStdLog->direction ? LOG_SES_DEV_FLT :
                                   LOG_Info(LOG_SES_DEV_FLT),
                                   sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                }

                /*
                 * If the device is bypassed, report it.
                 */
                if (deltaC2 & SES_C2DEV_BYPASSA)
                {
                    pStdLog->direction = pElem->Ctrl.Generic.Ctrl2 & SES_C2DEV_BYPASSA;
                    SendAsyncEvent(pStdLog->direction ? LOG_SES_DEV_BYPA :
                                   LOG_Info(LOG_SES_DEV_BYPA),
                                   sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                }

                /*
                 * If the device is bypassed, report it.
                 */
                if (deltaC2 & SES_C2DEV_BYPASSB)
                {
                    pStdLog->direction = pElem->Ctrl.Generic.Ctrl2 & SES_C2DEV_BYPASSB;
                    SendAsyncEvent(pStdLog->direction ? LOG_SES_DEV_BYPB :
                                   LOG_Info(LOG_SES_DEV_BYPB),
                                   sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                }

                /*
                 * If the device is turned off, report it.
                 */
                if (deltaC2 & SES_C2DEV_DEVOFF)
                {
                    pStdLog->direction = pElem->Ctrl.Generic.Ctrl2 & SES_C2DEV_DEVOFF;
                    SendAsyncEvent(pStdLog->direction ? LOG_SES_DEV_OFF :
                                   LOG_Info(LOG_SES_DEV_OFF),
                                   sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                }
                break;


            case SES_ET_POWER:
                /*
                 * Check for failure of the power supply.
                 */
                if (deltaC1 & SES_C1PS_DCOVER)
                {
                    pStdLog->direction = pElem->Ctrl.Generic.Ctrl1 & SES_C1PS_DCOVER;
                    SendAsyncEvent(pStdLog->direction ? LOG_SES_PS_DC_OVERVOLT :
                                   LOG_Info(LOG_SES_PS_DC_OVERVOLT),
                                   sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                }

                /*
                 * Post DC under voltage.
                 */
                if (deltaC1 & SES_C1PS_DCUNDER)
                {
                    pStdLog->direction = pElem->Ctrl.Generic.Ctrl1 & SES_C1PS_DCUNDER;
                    SendAsyncEvent(pStdLog->direction ? LOG_SES_PS_DC_UNDERVOLT :
                                   LOG_Info(LOG_SES_PS_DC_UNDERVOLT),
                                   sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                }

                /*
                 * Post DC under current.
                 */
                if (deltaC1 & SES_C1PS_DCCURRENT)
                {
                    pStdLog->direction = pElem->Ctrl.Generic.Ctrl1 & SES_C1PS_DCCURRENT;
                    SendAsyncEvent(pStdLog->direction ? LOG_SES_PS_DC_OVERCURR :
                                   LOG_Info(LOG_SES_PS_DC_OVERCURR),
                                   sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                }

                /*
                 * Now check the control 2 byte.
                 */

                /*
                 * Post DC fail fault.
                 */
                if (deltaC2 & SES_C2PS_DCFAIL)
                {
                    pStdLog->direction = pElem->Ctrl.Generic.Ctrl2 & SES_C2PS_DCFAIL;
                    SendAsyncEvent(pStdLog->direction ? LOG_SES_PS_DC_FAIL :
                                   LOG_Info(LOG_SES_PS_DC_FAIL),
                                   sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                }

                /*
                 * Post off fault.
                 */
                if (deltaC2 & SES_C2PS_OFF)
                {
                    pStdLog->direction = pElem->Ctrl.Generic.Ctrl2 & SES_C2PS_OFF;
                    SendAsyncEvent(pStdLog->direction ?
                                   LOG_SES_PS_OFF :
                                   LOG_Info(LOG_SES_PS_OFF),
                                   sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                }

                /*
                 * Post AC fault.
                 */
                if (deltaC2 & SES_C2PS_TMPFAIL)
                {
                    pStdLog->direction = pElem->Ctrl.Generic.Ctrl2 & SES_C2PS_TMPFAIL;
                    SendAsyncEvent(pStdLog->direction ? LOG_SES_PS_OVER_TEMP :
                                   LOG_Info(LOG_SES_PS_OVER_TEMP),
                                   sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                }

                /*
                 * Post AC fault.
                 */
                if (deltaC2 & SES_C2PS_TMPWARN)
                {
                    pStdLog->direction = pElem->Ctrl.Generic.Ctrl2 & SES_C2PS_TMPWARN;
                    SendAsyncEvent(pStdLog->direction ? LOG_SES_PS_TEMP_WARN :
                                   LOG_Info(LOG_SES_PS_TEMP_WARN),
                                   sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                }

                /*
                 * Post AC fault.
                 */
                if (deltaC2 & SES_C2PS_ACFAIL)
                {
                    pStdLog->direction = pElem->Ctrl.Generic.Ctrl2 & SES_C2PS_ACFAIL;
                    SendAsyncEvent(pStdLog->direction ? LOG_SES_PS_AC_FAIL :
                                   LOG_Info(LOG_SES_PS_AC_FAIL),
                                   sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                }

                /*
                 * Post power supply failure
                 */
                if (deltaC2 & SES_C2PS_FAIL)
                {
                    pStdLog->direction = pElem->Ctrl.Generic.Ctrl2 & SES_C2PS_FAIL;
                    SendAsyncEvent(pStdLog->direction ? LOG_SES_PS_FAIL :
                                   LOG_Info(LOG_SES_PS_FAIL),
                                   sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                }
                break;


            case SES_ET_COOLING:
                /* Fans are not plug into so avoid bogus error messages */
                if (pSES->devType == PD_DT_SAS_SES)
                {
                    break;
                }

                /*
                 * Post fan failure if there was a change.
                 */
                if (deltaC2 & SES_C2FAN_FAIL)
                {
                    pStdLog->direction = pElem->Ctrl.Generic.Ctrl2 & SES_C2FAN_FAIL;
                    SendAsyncEvent(pStdLog->direction ? LOG_SES_FAN_FAIL :
                                   LOG_Info(LOG_SES_FAN_FAIL),
                                   sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                }

                /*
                 * For fan on and off indicators, the
                 * protocol is a bit different since
                 * they both can change at the same time
                 * for one event.  Only log the change
                 * if the bit is turning on.
                 */
                if (deltaC2 & SES_C2FAN_OFF)
                {
                    pStdLog->direction = pElem->Ctrl.Generic.Ctrl2 & SES_C2FAN_OFF;
                    if (pStdLog->direction)
                    {
                        SendAsyncEvent(LOG_SES_FAN_OFF,
                                       sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                    }
                }

                /*
                 * Post fan turned on if it changed.
                 * For this one, make an additional
                 * check to make sure that this does
                 * not get logged at power on.
                 */
                if ((deltaC2 & SES_C2FAN_ON) && (pSES->OldPage2 != NULL))
                {
                    pStdLog->direction = pElem->Ctrl.Generic.Ctrl2 & SES_C2FAN_ON;

                    if (pStdLog->direction)
                    {
                        SendAsyncEvent(LOG_SES_FAN_ON,
                                       sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                    }
                }
                break;


            case SES_ET_TEMP_SENSOR:

                pTempLog->temp = pElem->Ctrl.Temp.Temp;

                /*
                 * If this is a SATA or SBOD bay, then just
                 * use the temperature indications from
                 * the enclosure.
                 */
                if (pSES->devType == PD_DT_SATA_SES ||
                    pSES->devType == PD_DT_SBOD_SES ||
                    pSES->devType == PD_DT_SAS_SES)
                {
                    /*
                     * Adjust the slot number to make
                     * it look like an internal slot.
                     */
                    if (pSES->devType == PD_DT_SATA_SES ||
                        pSES->devType == PD_DT_SAS_SES)
                    {
                        pTempLog->slot += SES_T_INT_SLOT0 - SES_T_AMB_SLOT0;
                    }

                    /*
                     * Check for over temperature fault.
                     */
                    if (deltaC2 & SES_C2TS_OTFAIL)
                    {
                        pStdLog->direction = pElem->Ctrl.Temp.Ctrl2 & SES_C2TS_OTFAIL;
                        SendAsyncEvent(pStdLog->direction ? LOG_SES_TEMP_FAIL :
                                       LOG_Info(LOG_SES_TEMP_FAIL),
                                       sizeof(LOG_SES_WWN_TEMP_DAT), pTempLog);
                    }

                    /*
                     * Check for under temperature fault.
                     */
                    if (deltaC2 & SES_C2TS_UTFAIL)
                    {
                        pStdLog->direction = pElem->Ctrl.Temp.Ctrl2 & SES_C2TS_UTFAIL;
                        SendAsyncEvent(pStdLog->direction ? LOG_SES_TEMP_FAIL :
                                       LOG_Info(LOG_SES_TEMP_FAIL),
                                       sizeof(LOG_SES_WWN_TEMP_DAT), pTempLog);
                    }

                    /*
                     * Check for over temperature warning.
                     */
                    if (deltaC2 & SES_C2TS_OTWARN)
                    {
                        pStdLog->direction = pElem->Ctrl.Temp.Ctrl2 & SES_C2TS_OTWARN;
                        SendAsyncEvent(pStdLog->direction ? LOG_SES_TEMP_WARN :
                                       LOG_Info(LOG_SES_TEMP_WARN),
                                       sizeof(LOG_SES_WWN_TEMP_DAT), pTempLog);
                    }

                    /*
                     * Check for under temperature warning.
                     */
                    if (deltaC2 & SES_C2TS_UTWARN)
                    {
                        pStdLog->direction = pElem->Ctrl.Temp.Ctrl2 & SES_C2TS_UTWARN;
                        SendAsyncEvent(pStdLog->direction ? LOG_SES_TEMP_WARN :
                                       LOG_Info(LOG_SES_TEMP_WARN),
                                       sizeof(LOG_SES_WWN_TEMP_DAT), pTempLog);
                    }
                }
                else
                {
                    /*
                     * Preset to indicate no error to
                     * be logged.
                     */
                    tempCode = 0xFFFFFFFF;

                    /*
                     * Disable alarm for under temperature warning.
                     */
                    if (deltaC2 & SES_C2TS_UTWARN)
                    {
                        if (pElem->Ctrl.Temp.Ctrl2 & SES_C2TS_UTWARN)
                        {
                            SESAlarmCtrl(pSES, SES_C2AA_MUTED);
                        }
                        else if (!(pElem->Ctrl.Temp.Ctrl2 & SES_C2TS_UTFAIL))
                        {
                            SESAlarmCtrl(pSES, 0);
                        }
                    }

                    /*
                     * Disable alarm for over temperature warning.
                     */
                    if (deltaC2 & SES_C2TS_OTWARN)
                    {
                        if (pElem->Ctrl.Temp.Ctrl2 & SES_C2TS_OTWARN)
                        {
                            SESAlarmCtrl(pSES, SES_C2AA_MUTED);
                        }
                        else if (!(pElem->Ctrl.Temp.Ctrl2 & SES_C2TS_OTFAIL))
                        {
                            SESAlarmCtrl(pSES, 0);
                        }
                    }

                    if (pSES->OldPage2 != NULL)
                    {
                        oldT = pSES->OldPage2->Control[slot].Ctrl.Temp.Temp;
                    }
                    else
                    {
                        oldT = 0xFF;
                    }

                    if (oldT != pTempLog->temp)
                    {
                        if (pTempLog->slot == SES_T_AMB_SLOT0 ||
                            pTempLog->slot == SES_T_AMB_SLOT1)
                        {
                            /*
                             * Check for the temperature in
                             * the coldest range.  If we are
                             * in this range and previously
                             * were not in this range or had
                             * no reading, log the error.
                             */
                            if ((oldT > SES_T_AMB_L_ERR || oldT == 0xFF) &&
                                pTempLog->temp <= SES_T_AMB_L_ERR)
                            {
                                pTempLog->direction = TRUE;
                                tempCode = LOG_SES_TEMP_FAIL;
                            }

                            /*
                             * Check for the cold warning
                             * range.  If we are in this
                             * range and previsously we were
                             * not in this range or had no
                             * reading, then log it based
                             * upon how we entered into the
                             * range (from OK is warning,
                             * from cold is informative).
                             */
                            if ((oldT <= SES_T_AMB_L_ERR ||
                                 oldT > SES_T_AMB_L_WARN ||
                                 oldT == 0xFF) &&
                                pTempLog->temp > SES_T_AMB_L_ERR &&
                                pTempLog->temp <= SES_T_AMB_L_WARN)
                            {
                                pTempLog->direction = TRUE;
                                tempCode = LOG_SES_TEMP_WARN;
                            }

                            /*
                             * Check for the "IS OK NOW"
                             * range.  If we are in this
                             * range and previsously we were
                             * not in this range, then log
                             * it.
                             */
                            if ((oldT <= SES_T_AMB_L_WARN ||
                                 oldT >= SES_T_AMB_H_WARN) &&
                                oldT != 0xFF &&
                                pTempLog->temp > SES_T_AMB_L_WARN &&
                                pTempLog->temp < SES_T_AMB_H_WARN)
                            {
                                pTempLog->direction = FALSE;
                                tempCode = LOG_Info(LOG_SES_TEMP_WARN);
                            }

                            /*
                             * Check for the warm warning
                             * range.  If we are in this
                             * range and previsously we were
                             * not in this range or had no
                             * reading, then log it based
                             * upon how we entered into the
                             * range (from OK is warning,
                             * from hot is informative).
                             */
                            if ((oldT < SES_T_AMB_H_WARN ||
                                 oldT >= SES_T_AMB_H_ERR ||
                                 oldT == 0xFF) &&
                                pTempLog->temp < SES_T_AMB_H_ERR &&
                                pTempLog->temp >= SES_T_AMB_H_WARN)
                            {
                                pTempLog->direction = TRUE;
                                tempCode = LOG_SES_TEMP_WARN;
                            }

                            /*
                             * Check for the warm error
                             * range.  If we are in this
                             * range and previsously we were
                             * not in this range or had no
                             * reading, then log the error.
                             */
                            if ((oldT < SES_T_AMB_H_ERR || oldT == 0xFF) &&
                                pTempLog->temp > SES_T_AMB_H_ERR)
                            {
                                pTempLog->direction = TRUE;
                                tempCode = LOG_SES_TEMP_FAIL;
                            }
                        }
                        else
                        {
                            /*
                             * Check for the temperature in
                             * the coldest range.  If we are
                             * in this range and previously
                             * were not in this range or had
                             * no reading, log the error.
                             */
                            if ((oldT > SES_T_INT_L_ERR || oldT == 0xFF) &&
                                pTempLog->temp <= SES_T_INT_L_ERR)
                            {
                                pTempLog->direction = TRUE;
                                tempCode = LOG_SES_TEMP_FAIL;
                            }

                            /*
                             * Check for the cold warning
                             * range.  If we are in this
                             * range and previsously we were
                             * not in this range or had no
                             * reading, then log it based
                             * upon how we entered into the
                             * range (from OK is warning,
                             * from cold is informative).
                             */
                            if ((oldT <= SES_T_INT_L_ERR ||
                                 oldT > SES_T_INT_L_WARN || oldT == 0xFF) &&
                                pTempLog->temp > SES_T_INT_L_ERR &&
                                pTempLog->temp <= SES_T_INT_L_WARN)
                            {
                                pTempLog->direction = TRUE;
                                tempCode = LOG_SES_TEMP_WARN;
                            }

                            /*
                             * Check for the "IS OK NOW"
                             * range.  If we are in this
                             * range and previsously we were
                             * not in this range, then log
                             * it.
                             */
                            if ((oldT <= SES_T_INT_L_WARN ||
                                 oldT >= SES_T_INT_H_WARN) && oldT != 0xFF &&
                                pTempLog->temp > SES_T_INT_L_WARN &&
                                pTempLog->temp < SES_T_INT_H_WARN)
                            {
                                pTempLog->direction = FALSE;
                                tempCode = LOG_Info(LOG_SES_TEMP_WARN);
                            }

                            /*
                             * Check for the warm warning
                             * range.  If we are in this
                             * range and previsously we were
                             * not in this range or had no
                             * reading, then log it based
                             * upon how we entered into the
                             * range (from OK is warning,
                             * from hot is informative).
                             */
                            if ((oldT < SES_T_INT_H_WARN ||
                                 oldT >= SES_T_INT_H_ERR || oldT == 0xFF) &&
                                pTempLog->temp < SES_T_INT_H_ERR &&
                                pTempLog->temp >= SES_T_INT_H_WARN)
                            {
                                pTempLog->direction = TRUE;
                                tempCode = LOG_SES_TEMP_WARN;
                            }

                            /*
                             * Check for the warm error
                             * range.  If we are in this
                             * range and previsously we were
                             * not in this range or had no
                             * reading, then log the error.
                             */
                            if ((oldT < SES_T_INT_H_ERR || oldT == 0xFF) &&
                                pTempLog->temp > SES_T_INT_H_ERR)
                            {
                                pTempLog->direction = TRUE;
                                tempCode = LOG_SES_TEMP_FAIL;
                            }
                        }

                        /*
                         * If we set up a log code, then send the
                         * log message.
                         */
                        if (tempCode != 0xFFFFFFFF)
                        {
                            SendAsyncEvent(tempCode,
                                           sizeof(LOG_SES_WWN_TEMP_DAT),
                                           pTempLog);
                        }
                    }
                }
                break;


            case SES_ET_SES_ELEC:
                /*
                 * Post failure.
                 */
                if (deltaC1 & SES_C1ES_FAIL)
                {
                    pStdLog->direction = pElem->Ctrl.Generic.Ctrl1 &
                        SES_C1ES_FAIL;
                    SendAsyncEvent(pStdLog->direction ?
                                   LOG_SES_EL_FAIL :
                                   LOG_Info(LOG_SES_EL_FAIL),
                                   sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                }

                /*
                 * Post present change.
                 */
                if ((deltaC1 & SES_C1ES_PRESENT) && pSES->OldPage2)
                {
                    pStdLog->direction = pElem->Ctrl.Generic.Ctrl1 &
                        SES_C1ES_PRESENT;
                    SendAsyncEvent(pStdLog->direction ?
                                   LOG_Info(LOG_SES_EL_PRESENT) :
                                   LOG_SES_EL_PRESENT,
                                   sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                }

                /*
                 * Post reporting change.
                 */
                if ((deltaC1 & SES_C1ES_REPORT) && pSES->OldPage2)
                {
                    pStdLog->direction = pElem->Ctrl.Generic.Ctrl1 &
                        SES_C1ES_REPORT;
                    SendAsyncEvent(pStdLog->direction ?
                                   LOG_Info(LOG_SES_EL_REPORT) :
                                   LOG_SES_EL_REPORT,
                                   sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                }
                break;

            case SES_ET_VOLT_SENSOR:
                pVoltLog->voltage = tm_const_ntohs(pElem->Ctrl.Volt.Voltage);

                /*
                 * Post over voltage critical fault.
                 */
                if (deltaVC & SES_VOLT_OCRIT)
                {
                    pVoltLog->direction = pElem->Ctrl.Volt.Ctrl & SES_VOLT_OCRIT;
                    SendAsyncEvent(pVoltLog->direction ?
                                   LOG_SES_VOLTAGE_HI :
                                   LOG_Info(LOG_SES_VOLTAGE_HI),
                                   sizeof(LOG_SES_WWN_VOLT_DAT), pVoltLog);
                }

                /*
                 * Post over voltage critical warning.
                 */
                if (deltaVC & SES_VOLT_OWARN)
                {
                    pVoltLog->direction = pElem->Ctrl.Volt.Ctrl & SES_VOLT_OWARN;
                    SendAsyncEvent(pVoltLog->direction ?
                                   LOG_SES_VOLTAGE_HI_WARN :
                                   LOG_Info(LOG_SES_VOLTAGE_HI_WARN),
                                   sizeof(LOG_SES_WWN_VOLT_DAT), pVoltLog);
                }

                /*
                 * Post under voltage critical fault.
                 */
                if (deltaVC & SES_VOLT_UCRIT)
                {
                    pVoltLog->direction = pElem->Ctrl.Volt.Ctrl & SES_VOLT_UCRIT;
                    SendAsyncEvent(pVoltLog->direction ?
                                   LOG_SES_VOLTAGE_LO :
                                   LOG_Info(LOG_SES_VOLTAGE_LO),
                                   sizeof(LOG_SES_WWN_VOLT_DAT), pVoltLog);
                }

                /*
                 * Post under voltage critical fault.
                 */
                if (deltaVC & SES_VOLT_UWARN)
                {
                    pVoltLog->direction = pElem->Ctrl.Volt.Ctrl & SES_VOLT_UWARN;
                    SendAsyncEvent(pVoltLog->direction ?
                                   LOG_SES_VOLTAGE_LO_WARN :
                                   LOG_Info(LOG_SES_VOLTAGE_LO_WARN),
                                   sizeof(LOG_SES_WWN_VOLT_DAT), pVoltLog);
                }
                break;


            case SES_ET_CURR_SENSOR:
                pCurrLog->current = tm_const_ntohs(pElem->Ctrl.Current.Current);

                /*
                 * Post over current critical fault.
                 */
                if (deltaC & SES_CURR_OCRIT)
                {
                    pCurrLog->direction = pElem->Ctrl.Current.Ctrl &
                        SES_CURR_OCRIT;
                    SendAsyncEvent(pCurrLog->direction ?
                                   LOG_SES_CURRENT_HI :
                                   LOG_Info(LOG_SES_CURRENT_HI),
                                   sizeof(LOG_SES_WWN_CURR_DAT), pCurrLog);
                }

                /*
                 * Post over voltage critical warning.
                 */
                if (deltaC & SES_CURR_OWARN)
                {
                    pCurrLog->direction = pElem->Ctrl.Current.Ctrl &
                        SES_CURR_OWARN;
                    SendAsyncEvent(pCurrLog->direction ?
                                   LOG_SES_CURRENT_HI_WARN :
                                   LOG_Info(LOG_SES_CURRENT_HI_WARN),
                                   sizeof(LOG_SES_WWN_CURR_DAT), pCurrLog);
                }
                break;

                /*
                 * This constant happens to be the same for
                 * Adaptec/Eurologic as it is for Xyratex
                 * since it is a vendor unique page.
                 */
                /*case SES_ET_SBOD_STAT: */
            case SES_ET_LOOP_STAT:
                /*
                 * Check the vendor and if this is an
                 * Eurologic bay, check the page.
                 */
                if (pSES->devType == PD_DT_SBOD_SES)
                {
                    /*
                     * Check for a change in the page
                     * value or a bad value in the case
                     * of first time seeing the page.
                     */
                    pSBODExtLog->newStat = pElem->Ctrl.Current.Ctrl & SES_C0XS_ES_MASK;

                    if (pSES->OldPage2 == NULL)
                    {
                        if (pSBODExtLog->newStat != SES_C0XS_ES_OK)
                        {
                            SendAsyncEvent(LOG_SES_SBOD_EXT,
                                           sizeof(LOG_SES_WWN_SBOD_EXT_DAT),
                                           pSBODExtLog);
                        }
                    }
                    else
                    {
                        if (deltaC & SES_C0XS_ES_MASK)
                        {
                            /*
                             * The change was in the status field.  Log it.
                             */
                            SendAsyncEvent(pSBODExtLog->newStat ==
                                           SES_C0XS_ES_OK ?
                                           LOG_Info(LOG_SES_SBOD_EXT) :
                                           LOG_SES_SBOD_EXT,
                                           sizeof(LOG_SES_WWN_SBOD_EXT_DAT),
                                           pSBODExtLog);
                        }

                    }
                }
                else if (pSES->devType != PD_DT_FC_SES)
                {
                    /*
                     * Eurologic FC bay.
                     *
                     * If loop A failed, report it.
                     */
                    if (deltaC2 & SES_C2EL_LOOPAFAIL)
                    {
                        pStdLog->direction = pElem->Ctrl.Generic.Ctrl2 &
                            SES_C2EL_LOOPAFAIL;
                        SendAsyncEvent(pStdLog->direction ?
                                       LOG_SES_LOOPAFAIL :
                                       LOG_Info(LOG_SES_LOOPAFAIL),
                                       sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                    }

                    /*
                     * If loop B failed, report it.
                     */
                    if (deltaC2 & SES_C2EL_LOOPBFAIL)
                    {
                        pStdLog->direction = pElem->Ctrl.Generic.Ctrl2 &
                            SES_C2EL_LOOPBFAIL;
                        SendAsyncEvent(pStdLog->direction ?
                                       LOG_SES_LOOPBFAIL :
                                       LOG_Info(LOG_SES_LOOPBFAIL),
                                       sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                    }

                    /*
                     * If speed mismatch, report it.
                     */
                    if (deltaC2 & SES_C2EL_SPEEDMIS)
                    {
                        pStdLog->direction = pElem->Ctrl.Generic.Ctrl2 &
                            SES_C2EL_SPEEDMIS;
                        SendAsyncEvent(pStdLog->direction ?
                                       LOG_SES_SPEEDMIS :
                                       LOG_Info(LOG_SES_SPEEDMIS),
                                       sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                    }

                    /*
                     * If firmware mismatch, report it.
                     */
                    if (deltaC2 & SES_C2EL_FWMISMATCH)
                    {
                        pStdLog->direction = pElem->Ctrl.Generic.Ctrl2 &
                            SES_C2EL_FWMISMATCH;
                        SendAsyncEvent(pStdLog->direction ?
                                       LOG_SES_FWMISMATCH :
                                       LOG_Info(LOG_SES_FWMISMATCH),
                                       sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                    }
                }
                break;

            case SES_ET_CTRL_STAT:
                if (pSES->devType == PD_DT_FC_SES)
                {
                    /*
                     * If I/O module, report it.
                     * Extra check for powerup.
                     */

                    if ((deltaC2 & SES_C2EI_PRESENT))
                    {
                        pStdLog->direction = !(pElem->Ctrl.Generic.Ctrl2 & SES_C2EI_PRESENT);

                        if (pStdLog->direction ||
                            (!pStdLog->direction && pSES->OldPage2))
                        {
                            SendAsyncEvent(pStdLog->direction ?
                                           LOG_SES_IO_MOD_PULLED :
                                           LOG_Info(LOG_SES_IO_MOD_PULLED),
                                           sizeof(LOG_SES_WWN_SLOT_DAT), pStdLog);
                        }
                    }
                }
                break;

            default:
                break;
            }
        }
    }

    /*
     * Free the page if there was an old one.  Save the
     * current one for comparisons next time through the
     * loop..
     */
    if (pSES->OldPage2)
    {
        Free(pSES->OldPage2);
    }
    pSES->OldPage2 = page2;
}


/* ------------------------------------------------------------------------ */


/*----------------------------------------------------------------------------
**  Function Name: SES_page80_Handle
**
**  Comments:   Handle the SES page 80 and 81 switch port status.
**
**  Parameters: pSES        - Pointer to SESList entry working with.
**              page80      - Buffer containing SES page 2 information.
**              pSBODExtLog - Current SBOD extended log buffer.
**
**  Returns:    none.
**--------------------------------------------------------------------------*/
static void SES_page80_Handle(SES_DEVICE *pSES, SES_P80_XTEX *page80,
                              LOG_SES_WWN_SBOD_EXT_DAT *pSBODExtLog)
{
    SES_P80XTEXPort *p;
    UINT32      i;
    UINT16      count;
    int         flag = FALSE;
    SES_P80_XTEX **OldPage;

/* dprintf(DPRINTF_DEFAULT, "WWN=0x%qx, devType=0x%x, bayId=%d, page=%x\n", pSES->WWN, pSES->devType, pSES->PID, page80->PageCode); */
    /*
     * A few simple checks.
     */
    if (page80->PageCode == 0x80)
    {
        OldPage = &pSES->OldPage80;
    }
    else if (page80->PageCode == 0x81)
    {
        OldPage = &pSES->OldPage81;
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "PageCode (0x%x) != 0x80 or 0x81\n", page80->PageCode);
        return;
    }
    count = bswap_16(page80->Length);
    if (count == 0)
    {
        dprintf(DPRINTF_DEFAULT, "Length (0x%x) == 0\n", page80->Length);
        return;
    }
    count = (count - 10) / 8;
    if (count != 20)
    {
        dprintf(DPRINTF_DEFAULT, "Count (%d) != 20\n", count);
        return;
    }

    /*
     * Do not print anything for first time up.  Assume that cabling is still
     * happening.  Only report changes -- usually "good" after that.  Else
     * they are really bad and need to be reported.
     */
    if (*OldPage)
    {
        /*
         * Examine each port status.
         */
        for (i = 0; i < count; i++)
        {
            /*
             * Check for a change in the State Code.
             */
            p = &page80->Port[i];
            flag = false;
            if ((*OldPage)->Port[i].StateCode != p->StateCode)
            {
                /*
                 * The intention in these if's is to put the message out only
                 * once for both page 80 and 81 processing.
                 */
                if (page80->PageCode == 0x80)
                {
                    if (pSES->OldPage81 == NULL ||
                        pSES->OldPage81->Port[i].StateCode != p->StateCode)
                    {
                        flag = TRUE;
                    }
                }
                else
                {
                    if (pSES->OldPage80 == NULL ||
                        pSES->OldPage80->Port[i].StateCode != p->StateCode)
                    {
                        flag = TRUE;
                    }
                }
            }
            if (flag == TRUE)
            {
                /*
                 * Set the WWN in the log message.
                 */
                pSBODExtLog->wwn = pSES->WWN;
                /*
                 * Set the slot number in the log messages.
                 */
                pSBODExtLog->slot = i;

/* dprintf(DPRINTF_DEFAULT, " #%d WWN=%qx StateCode=0x%x, %s\n", i, pSES->WWN, p->StateCode, SES_page80_String(p->StateCode, page80->PageCode)); */
                if (page80->PageCode == 0x80)
                {
                    pSBODExtLog->newStat = 0;
                }
                else if (page80->PageCode == 0x81)
                {
                    pSBODExtLog->newStat = 1 << 31;     /* Top Bit set = 81 */
                }
                pSBODExtLog->newStat |= p->StateCode;
                SendAsyncEvent(p->StateCode == SES_P80_SC_INSERTED ?
                               LOG_Info(LOG_SES_SBOD_STATECODE) :
                               LOG_SES_SBOD_STATECODE, sizeof(*pSBODExtLog), pSBODExtLog);
            }
        }
        /*
         * Free the old page.
         */
        Free(*OldPage);
    }
    /*
     * Save the current one for comparison next time through the loop.
     */
    *OldPage = page80;
}


#else  /* MODEL_3000 || MODEL_7400 */

/* ------------------------------------------------------------------------ */
static void DeleteStatsData(UINT16 bayid)
{
    memset(&ise_data[bayid], 0, sizeof(struct ise_stats_version_2));
}

/* ------------------------------------------------------------------------ */
#if 0
static void print_page30_debug(void)
{
    LOG_ISE_PAGE *page30 = page30_static;
    int i;

//    for (i = 0 ; i < (int)sizeof(LOG_ISE_PAGE); i += 4)
    for (i = 0 ; i < 1120; i += 4)          // Only print out the part up to the DRIVE information.
    {
        if ((i % 48) == 0) { fprintf(stderr, "%4d", i); }
        fprintf(stderr, " %8.8x", *((UINT32*)((UINT32)page30+i)));
        if ((i % 48) == 48-4) { fprintf(stderr, "\n"); }
    }
    fprintf(stderr, "\n");
    dprintf(DPRINTF_DEFAULT, "PageCode=0x%02x Rsvd=0x%02x Length=%d version=%d status=0x%08x\n",
        page30->PageCode, page30->Rsvd, bswap_16(page30->Length), bswap_32(page30->version), bswap_32(page30->status));
    dprintf(DPRINTF_DEFAULT, "spare_level=%d wwn=%08x%08x temp=%d hw_id=0x%08x\n",
        bswap_32(page30->spare_level), bswap_32(page30->wwn[0]), bswap_32(page30->wwn[1]), bswap_32(page30->temp),
        bswap_32(page30->hw_id));
    dprintf(DPRINTF_DEFAULT, "num_mrcs=%d num_data_pacs=%d num_events=%d\n",
        page30->num_mrcs, page30->num_data_pacs, bswap_32(page30->num_events));
    dprintf(DPRINTF_DEFAULT, "DataPac0 &%d  status=0x%08x general_status=0x%02x datapac_number=%d health=%d temp=%d\n",
        (UINT32)page30 - (UINT32)(&page30->data_pac[0].status), bswap_32(page30->data_pac[0].status),
        page30->data_pac[0].general_status, page30->data_pac[0].datapac_number, page30->data_pac[0].health,
        page30->data_pac[0].temp);
    dprintf(DPRINTF_DEFAULT, "type=0x%02x beacon=0x%02x number_drives=%d number_bad_drives=%d\n",
        page30->data_pac[0].type, page30->data_pac[0].beacon, bswap_32(page30->data_pac[0].number_drives),
        bswap_32(page30->data_pac[0].number_bad_drives));

    dprintf(DPRINTF_DEFAULT, "MRC0 general_status=%02x status=%08x serial_number=%24.24s\n",
        page30->mrc[0].general_status, bswap_32(page30->mrc[0].status), page30->mrc[0].serial_number);
    dprintf(DPRINTF_DEFAULT, "MRC1 general_status=%02x status=%08x serial_number=%24.24s\n",
        page30->mrc[1].general_status, bswap_32(page30->mrc[1].status), page30->mrc[1].serial_number);
}   /* end of print_page30_debug */
#endif  /* 0 */

/* ------------------------------------------------------------------------ */
static INT32 ISEHandleEvents(ISE_SES_DEVICE *pise, struct ise_info_version_2 *ise)
{
    LOG_ISE_ELEM_CHANGE_DAT iseLog;
    struct ise_info_version_2 *poldinfo;
    struct ise_stats_version_2 *poldstats;
    int         ip_change = 0;
    UINT16      envmap = 0;
    int         i;
    UINT8       discover_ses = FALSE;
    UINT8       something_changed = 0;
    int         ise_temp_hysteresis = 2;
    int         ise_warning_temperature = 55;
    int         ise_error_temperature = 60;

    poldstats = &ise_old_data[pise->PID];
    poldinfo = &poldstats->ise_bay_info;

    /*
     * Compare with old ise infor values to the new and
     * if there is any change just send the asynchrnous event
     * to the clients
     */
    for (i = 0; i < 2; i++)
    {
        struct ise_controller_version_2 *nc = &ise->ctrlr[i];
        struct ise_controller_version_2 *oc = &poldinfo->ctrlr[i];
        int         ctrlr_change;

        ctrlr_change = nc->ip != oc->ip ||
            nc->gateway != oc->gateway ||
            nc->subnet_mask != oc->subnet_mask ||
            nc->controller_fc_port_status != oc->controller_fc_port_status ||
            nc->controller_fc_port_speed != oc->controller_fc_port_speed ||
            nc->controller_ethernet_link_up != oc->controller_ethernet_link_up ||
            nc->controller_status != oc->controller_status ||
            nc->controller_status_details != oc->controller_status_details ||
            strncmp(nc->controller_fw_version,
                        oc->controller_fw_version, 28);

        if (!ctrlr_change)
        {
            continue;
        }

        UINT8       newStatus = nc->controller_status;
        UINT8       oldStatus = oc->controller_status;

        if (newStatus != oldStatus)
        {
            iseLog.component = LOG_ISE_MRC;
            iseLog.mrc_data.bayid = pise->PID;
            iseLog.mrc_data.mrc_no = i + 1;

            strncpy(iseLog.mrc_data.sn, nc->controller_serial_number, 12);
            iseLog.mrc_data.sn[12] = '\0';

            strncpy(iseLog.mrc_data.pn, nc->controller_part_number, 16);
            iseLog.mrc_data.pn[16] = '\0';

            strncpy(iseLog.mrc_data.hw_vers, nc->controller_hw_version, 4);
            iseLog.mrc_data.hw_vers[4] = '\0';

            strncpy(iseLog.mrc_data.fw_vers, nc->controller_fw_version, 16);
            iseLog.mrc_data.fw_vers[16] = '\0';

            iseLog.mrc_data.ip = nc->ip;
            iseLog.mrc_data.reason_code = nc->controller_status_details;

            dprintf(DPRINTF_DEFAULT, "MRC%d new/old Status=%d/%d details=%08llx/%08llx fcspeed=%d/%d fcstatus=%d/%d\n",
                    i,
                    newStatus, oldStatus,
                    nc->controller_status_details, oc->controller_status_details,
                    nc->controller_fc_port_speed, oc->controller_fc_port_speed,
                    nc->controller_fc_port_status, oc->controller_fc_port_status);
            dprintf(DPRINTF_DEFAULT, "Current Page30 MRC%d general_status=0x%02x status=0x%08x fcspeed=0x%02x fcstatus=0x%02x\n",
                    i,
                    page30_static->mrc[i].general_status, bswap_32(page30_static->mrc[i].general_status),
                    page30_static->mrc[i].fc_port_speed, page30_static->mrc[i].fc_port_status);

            switch (newStatus)
            {
            case OPERATIONAL:
                SendAsyncEvent(LOG_ISE_ELEM_CHANGE_I,
                               sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);
                break;

            case NONOPERATIONAL:
            case UNINITIALIZED:
                SendAsyncEvent(LOG_ISE_ELEM_CHANGE_E,
                               sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);
                break;

            case WARNING:
                SendAsyncEvent(LOG_ISE_ELEM_CHANGE_W,
                               sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);
                break;

            default:
                SendAsyncEvent(LOG_ISE_ELEM_CHANGE_W,
                               sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);
                break;
            }
        }

        envmap |= 1 << ISE_ENV_CTRLR_CHANGED;
        something_changed = 1;
    }

#if 0   // ATS-153  Do not put out datapac failed messages. The ISE MRC's die and you can't get the info.
    for (i = 0; i < 2; i++)
    {
        struct ise_datapac_version_2    *nd = &ise->datapac[i];
        struct ise_datapac_version_2    *od = &poldinfo->datapac[i];
        int         dpac_change;

        dpac_change = nd->datapac_status != od->datapac_status ||
             nd->datapac_status_details != od->datapac_status_details ||
             nd->datapac_capacity != od->datapac_capacity ||
             strncmp(nd->datapac_fw_version, od->datapac_fw_version, 16));

        if (!dpac_change)
        {
            continue;
        }

        UINT8       newStatus = nd->datapac_status;
        UINT8       oldStatus = od->datapac_status;

        if (newStatus != oldStatus)
        {
            iseLog.component = LOG_ISE_DPAC;
            iseLog.dpac_data.bayid = pise->PID;
            iseLog.dpac_data.dpac_no = i + 1;

            strncpy(iseLog.dpac_data.sn, nd->datapac_serial_number, 12);
            iseLog.dpac_data.sn[12] = '\0';

            strncpy(iseLog.dpac_data.pn, nd->datapac_part_number, 16);
            iseLog.dpac_data.pn[16] = '\0';

            strncpy(iseLog.dpac_data.fw_vers, nd->datapac_fw_version, 16);
            iseLog.dpac_data.fw_vers[16] = '\0';

            iseLog.dpac_data.health = nd->datapac_health;

            dprintf(DPRINTF_DEFAULT, "DataPac%d new/old Status=%d/%d details=%08x/%08x capacity=%d/%d\n",
                    i,
                    newStatus, oldStatus,
                    nd->datapac_status_details, od->datapac_status_details,
                    nd->datapac_capacity, od->datapac_capacity);
            dprintf(DPRINTF_DEFAULT, "Current Page30 DataPac%d general_status=0x%02x status=0x%08x\n",
                    i,
                    page30_static->data_pac[i].general_status, bswap_32(page30_static->data_pac[i].status));

            switch (newStatus)
            {
            case OPERATIONAL:
                SendAsyncEvent(LOG_ISE_ELEM_CHANGE_I,
                               sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);
                break;

            case NONOPERATIONAL:
            case UNINITIALIZED:
                SendAsyncEvent(LOG_ISE_ELEM_CHANGE_E,
                               sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);
                break;

            default:
                SendAsyncEvent(LOG_ISE_ELEM_CHANGE_W,
                               sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);
                break;
            }
        }
        envmap |= 1 << ISE_ENV_DPAC_CHANGED;
        something_changed = 1;
    }
#endif  /* 0 */

    for (i = 0; i < 2; i++)
    {
        struct ise_powersupply_version_2    *nps = &ise->powersupply[i];
        struct ise_powersupply_version_2    *ops = &poldinfo->powersupply[i];
        int         ps_change;

        ps_change = nps->powersupply_status != ops->powersupply_status ||
             nps->powersupply_status_details != ops->powersupply_status_details;
        if (!ps_change)
        {
            continue;
        }

        UINT8       newStatus = nps->powersupply_status;
        UINT8       oldStatus = ops->powersupply_status;

        if (newStatus != oldStatus)
        {
            iseLog.component = LOG_ISE_PS;
            iseLog.ps_data.bayid = pise->PID;
            iseLog.ps_data.ps_no = i + 1;

            strncpy(iseLog.ps_data.sn, nps->powersupply_serial_number, 12);
            iseLog.ps_data.sn[12] = '\0';

            strncpy(iseLog.ps_data.pn, nps->powersupply_part_number, 16);
            iseLog.ps_data.pn[16] = '\0';

            dprintf(DPRINTF_DEFAULT, "PowerSupply%d new/old Status=%d/%d details=%08llx/%08llx\n",
                    i,
                    newStatus, oldStatus,
                    nps->powersupply_status_details, ops->powersupply_status_details);
            dprintf(DPRINTF_DEFAULT, "Current Page30 PowerSupply%d general_status=0x%02x status=0x%08x\n",
                    i,
                    page30_static->ps[i].general_status, bswap_32(page30_static->ps[i].status));

            switch (newStatus)
            {
            case OPERATIONAL:
                SendAsyncEvent(LOG_ISE_ELEM_CHANGE_I,
                               sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);
                break;

            case NONOPERATIONAL:
            case UNINITIALIZED:
                SendAsyncEvent(LOG_ISE_ELEM_CHANGE_E,
                               sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);
                break;

            default:
                SendAsyncEvent(LOG_ISE_ELEM_CHANGE_W,
                               sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);
                break;
            }
        }
        envmap |= 1 << ISE_ENV_PS_CHANGED;
        something_changed = 1;
    }

    for (i = 0; i < 2; i++)
    {
        struct ise_battery_version_2    *nb = &ise->battery[i];
        struct ise_battery_version_2    *ob = &poldinfo->battery[i];
        int         battery_change;

        battery_change = nb->battery_status != ob->battery_status ||
             nb->battery_status_details != ob->battery_status_details ||
             nb->battery_remaining_charge != ob->battery_remaining_charge ||
             nb->battery_max_charge != ob->battery_max_charge;

        if (!battery_change)
        {
            continue;
        }

        UINT8       newStatus = nb->battery_status;
        UINT8       oldStatus = ob->battery_status;

        if (newStatus != oldStatus)
        {
            iseLog.component = LOG_ISE_BATTERY;
            iseLog.bat_data.bayid = pise->PID;
            iseLog.bat_data.bat_no = i + 1;

            strncpy(iseLog.bat_data.sn, nb->battery_serial_number, 12);
            iseLog.bat_data.sn[12] = '\0';

            strncpy(iseLog.bat_data.pn, nb->battery_part_number, 16);
            iseLog.bat_data.pn[16] = '\0';

            dprintf(DPRINTF_DEFAULT, "Battery%d new/old Status=%d/%d details=%08llx/%08llx\n",
                    i,
                    newStatus, oldStatus,
                    nb->battery_status_details, ob->battery_status_details);
            dprintf(DPRINTF_DEFAULT, "Current Page30 Battery%d general_status=0x%02x status=0x%08x\n",
                    i,
                    page30_static->battery[i].general_status, bswap_32(page30_static->battery[i].battery_status));

            switch (newStatus)
            {
            case OPERATIONAL:
                SendAsyncEvent(LOG_ISE_ELEM_CHANGE_I,
                               sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);
                break;

            case NONOPERATIONAL:
            case UNINITIALIZED:
                SendAsyncEvent(LOG_ISE_ELEM_CHANGE_E,
                               sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);
                break;

            default:
                SendAsyncEvent(LOG_ISE_ELEM_CHANGE_W,
                               sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);
                break;
            }
        }
        envmap |= 1 << ISE_ENV_BATTERY_CHANGED;
        something_changed = 1;
    }

    /*
     * Find out any change in the IP addresses
     */
    if (ise->ip1 != poldinfo->ip1 || ise->ip2 != poldinfo->ip2)
    {
        /* Add or remove MRC has happend */
        ip_change = 1;
        discover_ses = TRUE;
        something_changed = 1;
    }

/*    chassis_change = !( (ise->Protocol_Version_Level == poldinfo->Protocol_Version_Level) && */
/*                        (ise->ip1 == poldinfo->ip1) && */
/*                        (ise->ip2 == poldinfo->ip2) && */
/*                        (ise->iws_ise_id == poldinfo->iws_ise_id) && */
/*                        (ise->chassis_wwn == poldinfo->chassis_wwn) && */
/*                        (ise->spare_level ==  poldinfo->spare_level) && */
/*                        (ise->chassis_auto_connect_enable == poldinfo->chassis_auto_connect_enable) && */
/*                        (ise->chassis_beacon == poldinfo->chassis_beacon) && */
/*                        (ise->chassis_status == poldinfo->chassis_status) && */
/*                        (ise->chassis_status_details == poldinfo->chassis_status_details) && */
/*                        !strncmp(ise->chassis_serial_number, poldinfo->chassis_serial_number, 12) && */
/*                        !strncmp(ise->chassis_model, poldinfo->chassis_model, 16) && */
/*                        !strncmp(ise->chassis_part_number, poldinfo->chassis_part_number, 16) && */
/*                        !strncmp(ise->chassis_vendor, poldinfo->chassis_vendor, 8) && */
/*                        !strncmp(ise->chassis_manufacturer, poldinfo->chassis_manufacturer, 8) && */
/*                        !strncmp(ise->chassis_product_version, poldinfo->chassis_product_version, 4) ); */

    if (ise->chassis_temperature_sensor != poldinfo->chassis_temperature_sensor)
    {
        if (ise_chassis_temp_state[pise->PID] == ISE_TEMP_UNINITIALIZED)
        {
            /*
             * Not yet initialized the state
             */
            /*
             * Generate the ok log message
             */
            iseLog.component = LOG_ISE_TEMP;
            iseLog.temp_data.bayid = pise->PID;
            iseLog.temp_data.ise_temperature = ise->chassis_temperature_sensor;

            SendAsyncEvent(LOG_ISE_ELEM_CHANGE_I,
                           sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);
            ise_chassis_temp_state[pise->PID] = ISE_TEMP_OK_STATE;
        }

        switch (ise_chassis_temp_state[pise->PID])
        {
        case ISE_TEMP_OK_STATE:
            if (ise->chassis_temperature_sensor >= ise_warning_temperature &&
                ise->chassis_temperature_sensor < ise_error_temperature)
            {
                /*
                 * Generate the Warning log message
                 */
                iseLog.component = LOG_ISE_TEMP;
                iseLog.temp_data.bayid = pise->PID;
                iseLog.temp_data.ise_temperature = ise->chassis_temperature_sensor;

                SendAsyncEvent(LOG_ISE_ELEM_CHANGE_W,
                               sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);

                ise_chassis_temp_state[pise->PID] = ISE_TEMP_WARNING_STATE;
            }
            else if (ise->chassis_temperature_sensor >= ise_error_temperature)
            {
                /*
                 * Generate the Error log message
                 */
                iseLog.component = LOG_ISE_TEMP;
                iseLog.temp_data.bayid = pise->PID;
                iseLog.temp_data.ise_temperature = ise->chassis_temperature_sensor;

                SendAsyncEvent(LOG_ISE_ELEM_CHANGE_E,
                               sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);
                ise_chassis_temp_state[pise->PID] = ISE_TEMP_ERROR_STATE;
            }
            break;

        case ISE_TEMP_WARNING_STATE:
            if (ise->chassis_temperature_sensor >= ise_error_temperature)
            {
                /*
                 * Generate the Error log message
                 */
                iseLog.component = LOG_ISE_TEMP;
                iseLog.temp_data.bayid = pise->PID;
                iseLog.temp_data.ise_temperature = ise->chassis_temperature_sensor;

                SendAsyncEvent(LOG_ISE_ELEM_CHANGE_E,
                               sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);
                ise_chassis_temp_state[pise->PID] = ISE_TEMP_ERROR_STATE;
            }
            else if (ise->chassis_temperature_sensor <
                     (ise_warning_temperature - ise_temp_hysteresis))
            {
                /*
                 * Generate the ok log message
                 */
                iseLog.component = LOG_ISE_TEMP;
                iseLog.temp_data.bayid = pise->PID;
                iseLog.temp_data.ise_temperature = ise->chassis_temperature_sensor;

                SendAsyncEvent(LOG_ISE_ELEM_CHANGE_I,
                               sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);
                ise_chassis_temp_state[pise->PID] = ISE_TEMP_OK_STATE;
            }
            break;

        case ISE_TEMP_ERROR_STATE:
            if (ise->chassis_temperature_sensor <
                (ise_error_temperature - ise_temp_hysteresis))
            {
                /*
                 * Generate the warning log message
                 */
                iseLog.component = LOG_ISE_TEMP;
                iseLog.temp_data.bayid = pise->PID;
                iseLog.temp_data.ise_temperature = ise->chassis_temperature_sensor;

                SendAsyncEvent(LOG_ISE_ELEM_CHANGE_W,
                               sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);
                ise_chassis_temp_state[pise->PID] = ISE_TEMP_WARNING_STATE;
            }
            break;
        }
    }

    if (something_changed)
    {
        if (envmap)
        {
            /*
             * There is a change log the environment change
             * and send async event to the clients
             */
            iseLog.map_data.bayid = pise->PID;

            iseLog.map_data.envmap = envmap;

            SendAsyncEvent(LOG_ISE_ELEM_CHANGE,
                           sizeof(LOG_ISE_ELEM_CHANGE_DAT), &iseLog);
            EnqueuePIAsyncEvent(envmap, PIASYNC_EVENT_SECOND32_MAP);
            /*
             * copy the new ise info to oldinfo
             */
        }
        memcpy(poldinfo, ise, sizeof(struct ise_info_version_2));
    }
    return discover_ses;
}


/* ------------------------------------------------------------------------ */
static UINT8 ise_page30_status8_convert(UINT8 status)
{
    /* page 30, is zero based, iws is 1 based. Add one. */
    return (status + 1);
// enum xwstype__ObjStatus1 {
// xwstype__ObjStatus1__NEWER_PROTOCOL_VERSION_VALUE = 0,
// xwstype__ObjStatus1__OPERATIONAL = 1,
// xwstype__ObjStatus1__WARNING = 2,
// xwstype__ObjStatus1__CRITICAL = 3,
// xwstype__ObjStatus1__UNINITIALIZED = 4,
// xwstype__ObjStatus1__STATE_CHANGING = 5,
// xwstype__ObjStatus1__NON_OPERATIONAL = 6};
}                               /* End of ise_page30_status8_convert */

/* ------------------------------------------------------------------------ */

/*
 * Go through a two field table and convert bits set from one value to another.
 */
static UINT64 bit_table_convert(UINT32 status, struct status_32_64_convert t[],
                                unsigned int array_lth)
{
    unsigned int i;
    UINT64      r = 0;

    for (i = 0; i < array_lth; i++)
    {
        if (status & t[i].from_bit)
        {
            r |= t[i].to_bit;
        }
    }
    return (r);
}

/* ------------------------------------------------------------------------ */
static UINT64 ise_page30_status32_convert(UINT32 status)
{
    UINT64      r = 0;          // The return is a bit field.

// 2009-04-03   The left field is from LogSense.doc of ISE, right ASGSES.
// NOTE: Not in page0x30 status, 1 << 20     Degraded Battery
// NOTE: Not in page0x30 status, 1 << 28     Cache Inconsistency
// NOTE: Need to add: Cache Protect Mode (Wait for Auto-Reboot) -- 33
// NOTE: Need to add: Unused04000000                            -- 34

    static struct status_32_64_convert ise_convert[] = {
        {0x00000001, 1LL << 10},        // Maintenance Mode
        {0x00000002, 1LL << 11},        // Component Degraded
        {0x00000004, 1LL << 12},        // Temperature Out of Range
        {0x00000008, 1LL << 13},        // Uninitialized Not Ready ?????????????
        {0x00000010, 1LL << 13},        // Uninitialized Not Ready ?????????????
        {0x00000020, 1LL << 14},        // Initialized Not Operational
        {0x00000040, 1LL << 15},        // Display Card Not Present
        {0x00000080, 1LL << 16},        // Display Card Failure
        {0x00000100, 1LL << 2}, // Component Not Present
        {0x00000200, 1LL << 3}, // Component Offline
        {0x00000400, 1LL << 4}, // Vital Product Data Corrupt
        {0x00000800, 1LL << 5}, // Vital Product Data Unknown Type
        {0x00001000, 1LL << 6}, // Vital Product Data Unknown Version
        {0x00002000, 1LL << 7}, // Vital Product Data in Bad State
        {0x00004000, 1LL << 8}, // Vital Product Data is loaded
        {0x00008000, 1LL << 9}, // Vital Product Data in Persistent Fault
        {0x00010000, 1LL << 17},        // System Metadata Error
        {0x00020000, 1LL << 18},        // Cache Memory Error
        {0x00040000, 1LL << 19},        // System Metadata Conversion Error
        {0x00080000, 1LL << 33},        // Cache Protect Mode (Wait for Auto-Reboot)
        {0x00100000, 1LL << 21},        // All Datapacs Degraded
        {0x00200000, 1LL << 23},        // System Metadata Mismatch (code 1)
        {0x00400000, 1LL << 24},        // System Metadata Mismatch (code 2)
        {0x00800000, 1LL << 25},        // System Metadata Mismatch (code 3)
        {0x01000000, 1LL << 26},        // System Metadata Mismatch (code 4)
        {0x02000000, 1LL << 27},        // Inoperative MRC
        {0x04000000, 1LL << 34},        // Unused04000000
        {0x08000000, 1LL << 29},        // Diagnostic Mode
        {0x10000000, 1LL << 30},        // Datapac Configuration Conflict
        {0x20000000, 1LL << 22},        // System Metadata Mismatch (code 0)
        {0x40000000, 1LL << 31},        // MRC Configuration Conflict
        {0x80000000, 1LL << 32} // Datapac Configuration Conflict Recoverable
    };

    if (status == 0)
    {
        r = (1LL << 1);         // NONE or Operational
    }
    else
    {
        r = bit_table_convert(status, ise_convert,
                              sizeof(ise_convert) / sizeof(struct status_32_64_convert));
    }
    return (r);
// enum xwstype__IseDetails1 {
// xwstype__IseDetails1__NEWER_PROTOCOL_VERSION_VALUE = 0,
// xwstype__IseDetails1__NONE = 1,
// xwstype__IseDetails1__COMPONENT_NOT_PRESENT = 2,
// xwstype__IseDetails1__COMPONENT_OFFLINE = 3,
// xwstype__IseDetails1__VITAL_PRODUCT_DATA_CORRUPT = 4,
// xwstype__IseDetails1__VITAL_PRODUCT_DATA_UNKNOWN_TYPE = 5,
// xwstype__IseDetails1__VITAL_PRODUCT_DATA_UNKNOWN_VERSION = 6,
// xwstype__IseDetails1__VITAL_PRODUCT_DATA_BAD_STATE = 7,
// xwstype__IseDetails1__VITAL_PRODUCT_DATA_LOADED = 8,
// xwstype__IseDetails1__VITAL_PRODUCT_DATA_PERSISTENT_FAULT = 9,
// xwstype__IseDetails1__MAINTENANCE_MODE = 10,
// xwstype__IseDetails1__COMPONENT_DEGRADED = 11,
// xwstype__IseDetails1__TEMPERATURE_OUT_OF_RANGE = 12,
// xwstype__IseDetails1__UNINITIALIZED_NOT_READY = 13,
// xwstype__IseDetails1__INITIALIZED_NOT_OPERATIONAL = 14,
// xwstype__IseDetails1__DISPLAY_CARD_NOT_PRESENT = 15,
// xwstype__IseDetails1__DISPLAY_CARD_FAILURE = 16,
// xwstype__IseDetails1__SYSTEM_METADATA_ERROR = 17,
// xwstype__IseDetails1__CACHE_MEMORY_ERROR = 18,
// xwstype__IseDetails1__SYSTEM_METADATA_CONVERSION_ERROR = 19,
// xwstype__IseDetails1__DEGRAGED_BATTERY = 20,
// xwstype__IseDetails1__ALL_DATAPACS_DEGRADED = 21,
// xwstype__IseDetails1__SYSTEM_METADATA_MISMATCH_CODE_0 = 22,
// xwstype__IseDetails1__SYSTEM_METADATA_MISMATCH_CODE_1 = 23,
// xwstype__IseDetails1__SYSTEM_METADATA_MISMATCH_CODE_2 = 24,
// xwstype__IseDetails1__SYSTEM_METADATA_MISMATCH_CODE_3 = 25,
// xwstype__IseDetails1__SYSTEM_METADATA_MISMATCH_CODE_4 = 26,
// xwstype__IseDetails1__MRC_INOPERATIVE = 27,
// xwstype__IseDetails1__CACHE_INCONSISTENCY = 28,
// xwstype__IseDetails1__DIAGNOSTIC_MODE = 29,
// xwstype__IseDetails1__DATAPAC_CONFIGURATION_CONFLICT = 30,
// xwstype__IseDetails1__MRC_CONFIGURATION_CONFLICT = 31,
// xwstype__IseDetails1__DATAPAC_CONFIGURATION_CONFLICT_RECOVERABLE = 32};
}                               /* End of ise_page30_status32_convert */


/* ------------------------------------------------------------------------ */
static UINT64 mrc_page30_status32_convert(UINT32 status)
{
    UINT64      r = 0;          // The return is a bit field.

// 2009-04-03   The left field is from LogSense.doc of ISE, right ASGSES.
// NOTE: Not in current status, 1 <<  3     No Heartbeat
// NOTE: Not in current status, 1 << 13     Vital Product Data Unavailable
// NOTE: Not in current status, 1 << 14     Temperature Unavailable
// NOTE: Not in current status, 1 << 15     Meltdown
// NOTE: Not in current status, 1 << 16     Degraded
// NOTE: Not in current status, 1 << 17     FW Version Mismatch
// NOTE: Not in current status, 1 << 19     Unable to Load FW
// NOTE: Not in current status, 1 << 20     Unknown FW Version
// NOTE: Need to add: Unused0002                                -- 23
// NOTE: Need to add: Unused0004                                -- 24
// NOTE: Need to add: Unsupported VPD version found             -- 25
// NOTE: Need to add: Bad VPD                                   -- 26
// NOTE: Need to add: Unused00004000                            -- 27
// NOTE: Need to add: Unused00010000                            -- 28
// NOTE: Need to add: Unused00020000                            -- 29
// NOTE: Need to add: Unused00040000                            -- 30
// NOTE: Need to add: Unused00080000                            -- 31
// NOTE: Need to add: Unused00100000                            -- 32
// NOTE: Need to add: Unused00200000                            -- 33
// NOTE: Need to add: Unused00400000                            -- 34
// NOTE: Need to add: Unused00800000                            -- 35
// NOTE: Need to add: Unused04000000                            -- 36
// NOTE: Need to add: Unused08000000                            -- 37
// NOTE: Need to add: Unused10000000                            -- 38
// NOTE: Need to add: Unused40000000                            -- 39
// NOTE: Need to add: Unused80000000                            -- 40

    static struct status_32_64_convert mrc_convert[] = {
        {0x00000001, 1LL << 2},         // RTC monitor detected real time clock not running
        {0x00000002, 1LL << 23},        // Unused0002
        {0x00000004, 1LL << 24},        // Unused0004
        {0x00000008, 1LL << 25},        // Unsupported VPD version found
        {0x00000010, 1LL << 4},         // ABL/Post fault
        {0x00000020, 1LL << 5},         // high temperature
        {0x00000040, 1LL << 6},         // other MRC in reset
        {0x00000080, 1LL << 21},        // Error in battery backup of cache
        {0x00000100, 1LL << 7},         // Not Present
        {0x00000200, 1LL << 8},         // Offline, but present
        {0x00000400, 1LL << 9},         // Bad VPD CRC
        {0x00000800, 1LL << 10},        // Bad VPD Type
        {0x00001000, 1LL << 11},        // Bad VPD Version
        {0x00002000, 1LL << 26},        // Bad VPD
        {0x00004000, 1LL << 27},        // Unused00004000
        {0x00008000, 1LL << 12},        // Persistent Fault Detected
        {0x00010000, 1LL << 28},        // Unused00010000
        {0x00020000, 1LL << 29},        // Unused00020000
        {0x00040000, 1LL << 30},        // Unused00040000
        {0x00080000, 1LL << 31},        // Unused00080000
        {0x00100000, 1LL << 32},        // Unused00100000
        {0x00200000, 1LL << 33},        // Unused00200000
        {0x00400000, 1LL << 34},        // Unused00400000
        {0x00800000, 1LL << 35},        // Unused00800000
        {0x01000000, 1LL << 18},        // FC port is Down
        {0x02000000, 1LL << 19},        // MRC is Unbootable - guessed "19" 2010-06-24
        {0x04000000, 1LL << 36},        // Unused04000000
        {0x08000000, 1LL << 37},        // Unused08000000
        {0x10000000, 1LL << 38},        // Unused10000000
        {0x20000000, 1LL << 22},        // FW Upgrade is in Progress
        {0x40000000, 1LL << 39},        // Unused40000000
        {0x80000000, 1LL << 40},        // Unused80000000
    };

    if (status == 0)
    {
        r = (1LL << 1);         // NONE or Operational
    }
    else
    {
        r = bit_table_convert(status, mrc_convert,
                              sizeof(mrc_convert) / sizeof(struct status_32_64_convert));
    }
    return (r);
// enum xwstype__CtrlrDetails1 {
// xwstype__CtrlrDetails1__NEWER_PROTOCOL_VERSION_VALUE = 0,
// xwstype__CtrlrDetails1__NONE = 1,
// xwstype__CtrlrDetails1__REAL_TIME_CLOCK_ERROR = 2,
// xwstype__CtrlrDetails1__NO_HEARTBEAT = 3,
// xwstype__CtrlrDetails1__SELF_TEST_FAILURE = 4,
// xwstype__CtrlrDetails1__TEMPERATURE_OUT_OF_RANGE = 5,
// xwstype__CtrlrDetails1__OTHER_CONTROLLER_RESET = 6,
// xwstype__CtrlrDetails1__NOT_PRESENT = 7,
// xwstype__CtrlrDetails1__OFFLINE = 8,
// xwstype__CtrlrDetails1__VITAL_PRODUCT_DATA_CORRUPT = 9,
// xwstype__CtrlrDetails1__VITAL_PRODUCT_DATA_UNKNOWN_TYPE = 10,
// xwstype__CtrlrDetails1__VITAL_PRODUCT_DATA_UNKNOWN_VERSION = 11,
// xwstype__CtrlrDetails1__PERSISTENT_FAULT = 12,
// xwstype__CtrlrDetails1__VITAL_PRODUCT_DATA_UNAVAILABLE = 13,
// xwstype__CtrlrDetails1__TEMPERATURE_UNAVAILABLE = 14,
// xwstype__CtrlrDetails1__MELTDOWN = 15,
// xwstype__CtrlrDetails1__DEGRADED = 16,
// xwstype__CtrlrDetails1__FW_VERSION_MISMATCH = 17,
// xwstype__CtrlrDetails1__FC_PORT_UNAVAILABLE = 18,
// xwstype__CtrlrDetails1__UNABLE_TO_LOAD_FW = 19,
// xwstype__CtrlrDetails1__UNKNOWN_FW_VERSION = 20,
// xwstype__CtrlrDetails1__STALE_CACHE = 21,
// xwstype__CtrlrDetails1__FW_UPDATE_IN_PROGRESS = 22};
}                               /* End of mrc_page30_status32_convert */


/* ------------------------------------------------------------------------ */
static UINT64 bat_page30_status32_convert(UINT32 status)
{
    UINT64      r = 0;          // The return is a bit field.

// 2009-04-06   The left field is from LogSense.doc of ISE, right ASGSES.
// NOTE - bat_page30_status32_convert, Added value 19 (temperature out of range).
// NOTE - bat_page30_status32_convert, Added value 20 (UNUSED0020).
// NOTE - bat_page30_status32_convert, Added value 21 (UNUSED4000).

    static struct status_32_64_convert bat_convert[] = {
        {0x00000001, 1LL << 2}, // Battery degraded due to overdischarge
        {0x00000002, 1LL << 19},        // Temperature out of range
        {0x00000004, 1LL << 9}, // Charger reports fault -> CHARGING_FAILURE
        {0x00000008, 1LL << 5}, // Constant current circuit fault
        {0x00000010, 1LL << 6}, // Maintenance Charge not obtained (under voltage)
        {0x00000020, 1LL << 20},        // UNUSED0020
        {0x00000040, 1LL << 8}, // Calibration mode test failed
        {0x00000080, 1LL << 9}, // Charging system HW failure
        {0x00000100, 1LL << 10},        // Not Present
        {0x00000200, 1LL << 11},        // Offline, but present
        {0x00000400, 1LL << 12},        // Bad VPD CRC
        {0x00000800, 1LL << 13},        // Bad VPD Type
        {0x00001000, 1LL << 14},        // Bad VPD Version
        {0x00002000, 1LL << 15},        // Bad VPD
        {0x00004000, 1LL << 21},        // Unused4000
        {0x00008000, 1LL << 16} // Persistent Fault Detected
    };

    if (status == 0)
    {
        r = (1LL << 1);         // NONE or Operational
    }
    else
    {
        r = bit_table_convert(status, bat_convert,
                              sizeof(bat_convert) / sizeof(struct status_32_64_convert));
    }
    return (r);
// enum xwstype__BatteryDetails1 {
// xwstype__BatteryDetails1__NEWER_PROTOCOL_VERSION_VALUE = 0,
// xwstype__BatteryDetails1__NONE = 1,
// xwstype__BatteryDetails1__DEGRADED = 2,
// xwstype__BatteryDetails1__CHARGE_PENDING = 3,
// xwstype__BatteryDetails1__CHARGING_FAULT = 4,
// xwstype__BatteryDetails1__CONSTANT_CURRENT_CIRCUIT_FAILURE = 5,
// xwstype__BatteryDetails1__CHARGE_FAILURE = 6,
// xwstype__BatteryDetails1__BACKUP_HW_FAILURE = 7,
// xwstype__BatteryDetails1__LOW_CAPACITY = 8,
// xwstype__BatteryDetails1__CHARGING_FAILURE = 9,
// xwstype__BatteryDetails1__NOT_PRESENT = 10,
// xwstype__BatteryDetails1__OFFLINE = 11,
// xwstype__BatteryDetails1__VITAL_PRODUCT_DATA_CORRUPT = 12,
// xwstype__BatteryDetails1__VITAL_PRODUCT_DATA_UNKNOWN_TYPE = 13,
// xwstype__BatteryDetails1__VITAL_PRODUCT_DATA_UNKNOWN_VERSION = 14,
// xwstype__BatteryDetails1__VITAL_PRODUCT_DATA_ACCESS_ERROR = 15,
// xwstype__BatteryDetails1__PERSISTENT_FAULT = 16,
// xwstype__BatteryDetails1__VITAL_PRODUCT_DATA_UNAVAILABLE = 17,
// xwstype__BatteryDetails1__BATTERY_INFORMATION_UNAVAILABLE = 18};
}                               /* End of bat_page30_status32_convert */


/* ------------------------------------------------------------------------ */
static UINT64 ps_page30_status32_convert(UINT32 status)
{
    UINT64      r = 0;          // The return is a bit field.

// 2009-04-07   The left field is from LogSense.doc of ISE, right ASGSES.
// NOTE: Not in current status, 1 <<  1     NONE
// NOTE: Not in current status, 1 << 12     Vital Product Data Access Error
// NOTE: Not in current status, 1 << 15     Temperature unavailable
// NOTE: Not in current status, 1 << 16     Fan status unavailable
// NOTE: Not in current status, 1 << 17     Fan speed unavailable
// NOTE: Need to add: Unused0001                                -- 18
// NOTE: Need to add: true if ADT7467 (new chip) found          -- 19
// NOTE: Need to add: true if failed to init ADT746x            -- 20
// NOTE: Need to add: Unused4000                                -- 21

    static struct status_32_64_convert ps_convert[] = {
        {0x0001, 1LL << 18},    // UNUSED001
        {0x0002, 1LL << 2},     // Bad AC
        {0x0004, 1LL << 3},     // Bad DC
        {0x0008, 1LL << 19},    // true if ADT7467 (new chip) found
        {0x0010, 1LL << 20},    // true if failed to init ADT746x
        {0x0020, 1LL << 4},     // Fan speed does not match RPM
        {0x0040, 1LL << 5},     // No power off that supply
        {0x0080, 1LL << 6},     // Power Supply is hot
        {0x0100, 1LL << 7},     // Not Present
        {0x0200, 1LL << 8},     // Offline, but present
        {0x0400, 1LL << 9},     // Bad VPD CRC
        {0x0800, 1LL << 10},    // Bad VPD Type
        {0x1000, 1LL << 11},    // Bad VPD Version
        {0x2000, 1LL << 14},    // Bad VPD
        {0x4000, 1LL << 21},    // UNUSED4000
        {0x8000, 1LL << 13}     // Persistent Fault  Detected
    };

    if (status == 0)
    {
        r = (1LL << 1);         // NONE or Operational
    }
    else
    {
        r = bit_table_convert(status, ps_convert,
                              sizeof(ps_convert) / sizeof(struct status_32_64_convert));
    }
    return (r);
// enum xwstype__PowerSupplyDetails1 {
// xwstype__PowerSupplyDetails1__NEWER_PROTOCOL_VERSION_VALUE = 0,
// xwstype__PowerSupplyDetails1__NONE = 1,
// xwstype__PowerSupplyDetails1__AC_POWER_FAILURE = 2,
// xwstype__PowerSupplyDetails1__DC_POWER_FAILURE = 3,
// xwstype__PowerSupplyDetails1__FAN_FAILURE = 4,
// xwstype__PowerSupplyDetails1__FAULT_DC_OUTPUT = 5,
// xwstype__PowerSupplyDetails1__TEMPERATURE_OUT_OF_RANGE = 6,
// xwstype__PowerSupplyDetails1__NOT_PRESENT = 7,
// xwstype__PowerSupplyDetails1__OFFLINE = 8,
// xwstype__PowerSupplyDetails1__VITAL_PRODUCT_DATA_CORRUPT = 9,
// xwstype__PowerSupplyDetails1__VITAL_PRODUCT_DATA_UNKNOWN_TYPE = 10,
// xwstype__PowerSupplyDetails1__VITAL_PRODUCT_DATA_UNKNOWN_VERSION = 11,
// xwstype__PowerSupplyDetails1__VITAL_PRODUCT_DATA_ACCESS_ERROR = 12,
// xwstype__PowerSupplyDetails1__PERSISTENT_FAULT = 13,
// xwstype__PowerSupplyDetails1__VITAL_PRODUCT_DATA_UNAVAILABLE = 14,
// xwstype__PowerSupplyDetails1__TEMPERATURE_UNAVAILABLE = 15,
// xwstype__PowerSupplyDetails1__FAN_STATUS_UNAVAILABLE = 16,
// xwstype__PowerSupplyDetails1__FAN_SPEED_UNAVAILABLE = 17};
}                               /* End of ps_page30_status32_convert */


/* ------------------------------------------------------------------------ */
static UINT64 dp_page30_status32_convert(UINT32 status)
{
    UINT64      r = 0;          // The return is a bit field.

// 2009-04-07   The left field is from LogSense.doc of ISE, right ASGSES.
// NOTE: Not in current status, 1 <<  1     NONE
// NOTE: Not in current status, 1 <<  2     VITAL_PRODUCT_DATA_ACCESS_ERROR
// NOTE: Not in current status, 1 << 12     Storage Pool Uninitialized
// NOTE: Not in current status, 1 << 13     FW Upgrade In Progress
// NOTE: Not in current status, 1 << 15     Meltdown
// NOTE: Not in current status, 1 << 16     Spare Capacity Lost
// NOTE: Not in current status, 1 << 17     Secure Erase in Progress
// NOTE: Not in current status, 1 << 18     Removal in Progress
// NOTE: Not in current status, 1 << 19     Add in Progress
// NOTE: Need to add: Unused0002                                -- 20
// NOTE: Need to add: Unused0004                                -- 21
// NOTE: Need to add: Unused0008                                -- 22
// NOTE: Need to add: Load Error                                -- 23
// NOTE: Need to add: Unknown DataPac type                      -- 24
// NOTE: Need to add: Unused4000                                -- 25

    static struct status_32_64_convert dp_convert[] = {
        {0x0001, 1LL << 14},    // DataPac degraded
        {0x0002, 1LL << 20},    // UNUSED0002
        {0x0004, 1LL << 21},    // UNUSED0004
        {0x0008, 1LL << 22},    // UNUSED0008
        {0x0010, 1LL << 23},    // Load Error
        {0x0020, 1LL << 3},     // DataPac online but handle not secure
        {0x0040, 1LL << 5},     // DataPac is hot
        {0x0080, 1LL << 24},    // Unknown DataPac type
        {0x0100, 1LL << 4},     // Not Present
        {0x0200, 1LL << 6},     // Offline, but present
        {0x0400, 1LL << 7},     // Bad VPD CRC
        {0x0800, 1LL << 8},     // Bad VPD Type
        {0x1000, 1LL << 9},     // Bad VPD Version
        {0x2000, 1LL << 11},    // Bad VPD
        {0x4000, 1LL << 25},    // UNUSED4000
        {0x8000, 1LL << 10}     // Persistent Fault  Detected
    };

    if (status == 0)
    {
        r = (1LL << 1);         // NONE or Operational
    }
    else
    {
        r = bit_table_convert(status, dp_convert,
                              sizeof(dp_convert) / sizeof(struct status_32_64_convert));
    }
    return (r);
// enum xwstype__DataPacDetails1 {
// xwstype__DataPacDetails1__NEWER_PROTOCOL_VERSION_VALUE = 0,
// xwstype__DataPacDetails1__NONE = 1,
// xwstype__DataPacDetails1__VITAL_PRODUCT_DATA_ACCESS_ERROR = 2,
// xwstype__DataPacDetails1__HANDLE_OPEN = 3,
// xwstype__DataPacDetails1__NOT_PRESENT = 4,
// xwstype__DataPacDetails1__TEMPERATURE_OUT_OF_RANGE = 5,
// xwstype__DataPacDetails1__OFFLINE = 6,
// xwstype__DataPacDetails1__VITAL_PRODUCT_DATA_CORRUPT = 7,
// xwstype__DataPacDetails1__VITAL_PRODUCT_DATA_UNKNOWN_TYPE = 8,
// xwstype__DataPacDetails1__VITAL_PRODUCT_DATA_UNKNOWN_VERSION = 9,
// xwstype__DataPacDetails1__PERSISTENT_FAULT = 10,
// xwstype__DataPacDetails1__VITAL_PRODUCT_DATA_UNAVAILABLE = 11,
// xwstype__DataPacDetails1__STORAGE_POOL_UNINITIALIZED = 12,
// xwstype__DataPacDetails1__FW_UPGRADE_INPROGRESS = 13,
// xwstype__DataPacDetails1__DEGRADED = 14,
// xwstype__DataPacDetails1__MELTDOWN = 15,
// xwstype__DataPacDetails1__SPARE_CAPACITY_LOST = 16,
// xwstype__DataPacDetails1__SECURE_ERASE_IN_PROGRESS = 17,
// xwstype__DataPacDetails1__REMOVAL_IN_PROGRESS = 18,
// xwstype__DataPacDetails1__ADD_IN_PROGRESS = 19};
}                               /* End of dp_page30_status32_convert */


/* ------------------------------------------------------------------------ */
// charger state (charger_status comes here).
static UINT64 charger_page30_status32_convert(UINT8 status)
{
// Input of 7, output of 8.
    UINT64      r = 1 + status; // The return is one more than the input.

    return (r);
// xwstype__BatteryChargerDetails1__NEWER_PROTOCOL_VERSION_VALUE = 0,
// xwstype__BatteryChargerDetails1__UNKNOWN = 1,
// xwstype__BatteryChargerDetails1__CHARGER_IN_RESET = 2,
// xwstype__BatteryChargerDetails1__CHARGER_INITIALIZATION = 3,
// xwstype__BatteryChargerDetails1__PRE_CHARGE_QUALIFICATION = 4,
// xwstype__BatteryChargerDetails1__FAST_CHARGE_CURRENT_REG = 5,
// xwstype__BatteryChargerDetails1__FAST_CHARGE_VOLTAGE_REG = 6,
// xwstype__BatteryChargerDetails1__CHARGING_OFFLINE = 7,
// xwstype__BatteryChargerDetails1__MAINTENANCE_CHARGING = 8,
// xwstype__BatteryChargerDetails1__CALIBRATION_TEST = 9,
// xwstype__BatteryChargerDetails1__CHARGE_PENDING = 10,
// xwstype__BatteryChargerDetails1__CHARGER_FAULT = 11,
// xwstype__BatteryChargerDetails1__OVER_VOLTAGE_FAULT = 12,
// xwstype__BatteryChargerDetails1__OK_TO_REPLACE_BATTERY = 13};
}                               /* End of charger_page30_status32_convert */


/* ------------------------------------------------------------------------ */
// general charger status comes here, even though it is a 32 bit value.
static UINT8 charger_page30_status8_convert(UINT32 status)
{
    switch (status)
    {
        case 0:
            return (2);         // UNKNOWN.
        case 1:
            return (1);         // Other
        case 2:
            return (2);         // Unknown
        case 3:
            return (3);         // Fully Charged
        case 4:
            return (4);         // Low Charge
        case 5:
            return (5);         // Critical Charge
        case 6:
            return (6);         // Charging
        case 7:
            return (7);         // Charging - High
        case 8:
            return (8);         // Charging - Low
        case 9:
            return (9);         // Charging - Critical
        case 10:
            return (10);        // Undefined
        case 11:
            return (11);        // Partially Charged
    }
    return (2);                 // UNKNOWN.
// enum xwstype__BatteryChargerStatus1 {
// xwstype__BatteryChargerStatus1__NEWER_PROTOCOL_VERSION_VALUE = 0,
// xwstype__BatteryChargerStatus1__OTHER = 1,
// xwstype__BatteryChargerStatus1__UNKNOWN = 2,
// xwstype__BatteryChargerStatus1__FULLY_CHARGED = 3,
// xwstype__BatteryChargerStatus1__LOW_CHARGE = 4,
// xwstype__BatteryChargerStatus1__CRITICAL_CHARGE = 5,
// xwstype__BatteryChargerStatus1__CHARGING = 6,
// xwstype__BatteryChargerStatus1__CHARGING_HIGH = 7,
// xwstype__BatteryChargerStatus1__CHARGING_LOW = 8,
// xwstype__BatteryChargerStatus1__CHARGING_CRITICAL = 9,
// xwstype__BatteryChargerStatus1__UNDEFINED = 10,
// xwstype__BatteryChargerStatus1__PARTIALLY_CHARGED = 11,
// xwstype__BatteryChargerStatus1__DEGRADED = 12};
}                               /* End of charger_page30_status8_convert */

/* ------------------------------------------------------------------------ */
static void BuildIseInfoFromPage30(LOG_ISE_PAGE *page30, struct ise_info_version_2 *ise)
{
    int         i;

// for (i = 0 ; i < sizeof(LOG_ISE_PAGE); i += 4)
// {
//   if ((i % 32) == 0) { fprintf(stderr, "%3d ", i/4); }
//   fprintf(stderr, "%8.8x", *((UINT32*)((UINT32)page30+i)));
//   if ((i % 32) == 32-4) { fprintf(stderr, "\n"); } else { fprintf(stderr, " "); }
// }
// fprintf(stderr, "\n");
//
//     fprintf(stderr, "ISE temp=%ld, uptime=%lld ms, iops=%ld, MRC0 Temp=%ld MRC1 Temp=%ld PS0 Temp=%ld PS1 Temp=%ld, DP0 Temp=%ld, DP1 Temp=%ld, BAT0 MAXC=%ld, BAT1 MAXC=%ld, PD0/9/10/19 sts=%ld/%ld/%ld/%ld\n",
//             (unsigned long)bswap_32(page30->temp),
//             bswap_64(page30->uptime),
//             (unsigned long)bswap_32(page30->iops_total),
//             (unsigned long)page30->mrc[0].temp,
//             (unsigned long)page30->mrc[1].temp,
//             (unsigned long)page30->ps[0].temp,
//             (unsigned long)page30->ps[1].temp,
//             (unsigned long)page30->data_pac[0].temp,
//             (unsigned long)page30->data_pac[1].temp,
//             (unsigned long)bswap_16(page30->battery[0].max_charge),
//             (unsigned long)bswap_16(page30->battery[1].max_charge),
//             (unsigned long)bswap_32(page30->drives[0].status),
//             (unsigned long)bswap_32(page30->drives[9].status),
//             (unsigned long)bswap_32(page30->drives[10].status),
//             (unsigned long)bswap_32(page30->drives[19].status));

    ise->Protocol_Version_Level = 1;    // Might need to be 2.
    ise->which_tcp_connections = 0;     // Do not know if these are active/reachable.
    ise->which_fw_index_type = 1;       // Numbering of MRC's are 1 and 2. (Not 0 and 1)
    ise->which_controllers[0] = 1;
    ise->which_controllers[1] = 2;
    ise->which_datapacs[0] = 1;
    ise->which_datapacs[1] = 2;
    ise->which_powersupplies[0] = 1;
    ise->which_powersupplies[1] = 2;
    ise->which_batteries[0] = 1;
    ise->which_batteries[1] = 2;

    page30_static = page30;             // We need this for figuring out why ISE firmware is giving us junk.

    ise->ip1 = (page30->mrc[0].ip_address);     // ???????????????????????????????
    ise->ip2 = (page30->mrc[1].ip_address);     // ???????????????????????????????
    ise->iws_ise_id = page30->hw_id;    // Not initialized (i.e. 0).
    ise->chassis_wwn = (((UINT64)page30->wwn[1]) << 32) | (page30->wwn[0]);

    memcpy(ise->chassis_serial_number, page30->serial_number,
                sizeof(ise->chassis_serial_number));
    memset(ise->chassis_model, 0, sizeof(ise->chassis_model));  // Not present?
    strncpy(ise->chassis_model, "ISE1400         ", 16);        // Make chassis model be ISE1400.
    memcpy(ise->chassis_part_number, page30->part_number,
                sizeof(ise->chassis_part_number));
    memcpy(ise->chassis_vendor, page30->vendor,
                sizeof(ise->chassis_vendor));
    memset(ise->chassis_manufacturer, 0,    // Not in page 0x30, zero anyways
                sizeof(ise->chassis_manufacturer));
    memcpy(ise->chassis_product_version, page30->product_version, 4);

    ise->spare_level = bswap_32(page30->spare_level) * 10;      // Need to multiple by 10.
    ise->chassis_beacon = page30->beacon;

    ise->chassis_status = ise_page30_status8_convert(page30->general_status);
    ise->chassis_status_details = ise_page30_status32_convert(bswap_32(page30->status));
// NOTE - chassis_uptime is not what you think. It increments by seconds.
    ise->chassis_uptime = bswap_64(page30->uptime);
    ise->chassis_current_date_time = 0; // Not right anyway.
    ise->chassis_performance_valid = bswap_32(page30->perf_valid);
    if (ise->chassis_performance_valid != 0)
    {
// NOTE - perf_valid is coming back as 3, make it 1.
        ise->chassis_performance_valid = 1;
    }
    ise->chassis_total_iops = bswap_32(page30->iops_total);
    ise->chassis_read_iops = bswap_32(page30->iops_read);
    ise->chassis_write_iops = bswap_32(page30->iops_write);
    ise->chassis_total_kbps = bswap_32(page30->kbps_total);
    ise->chassis_read_kbps = bswap_32(page30->kbps_read);
    ise->chassis_write_kbps = bswap_32(page30->kbps_write);
    ise->chassis_read_latency = bswap_32(page30->read_latency);
    ise->chassis_write_latency = bswap_32(page30->write_latency);
    ise->chassis_queue_depth = bswap_32(page30->queue_depth);
    ise->chassis_read_percent = bswap_32(page30->read_percentage);
    ise->chassis_avg_bytes_transferred = bswap_32(page30->avg_bytes);
    ise->chassis_temperature_sensor = bswap_32(page30->temp);
// Moved from MRC to ISE chassis location.
    bzero(ise->ctrlr[0].controller_fw_version, 28);
    sprintf(ise->ctrlr[0].controller_fw_version, "v%d.%d.%d-%d",
            page30->fw_version[0], page30->fw_version[1], page30->fw_version[2],
            page30->fw_version[3]);
    memcpy(ise->ctrlr[1].controller_fw_version, ise->ctrlr[0].controller_fw_version, 28);

// NOTE - Fields not used: num_data_pacs, num_mrcs, cache_dirty, num_events.

    /*
     * MRC fields
     */
    for (i = 0; i < 2; i++)
    {
        struct ise_controller_version_2 *c = &ise->ctrlr[i];

        memcpy(c->controller_model, page30->mrc[i].model,
                sizeof(c->controller_model));
        memcpy(c->controller_serial_number, page30->mrc[i].serial_number,
                sizeof(c->controller_serial_number));
        memcpy(c->controller_part_number, page30->mrc[i].part_number,
                sizeof(c->controller_part_number));
        memcpy(c->controller_hw_version, page30->mrc[i].hw_version,
                sizeof(c->controller_hw_version));

        c->controller_x_position = i + 1;    // constant location
        c->controller_y_position = 0;        // don't need y and z.
        c->controller_z_position = 0;
        c->controller_wwn = ((UINT64)page30->mrc[i].wwn[1] << 32) | page30->mrc[i].wwn[0];

        c->ip = page30->mrc[i].ip_address;
// NOTDONEYET - gateway returned zero from page 0x30.
        c->gateway = page30->mrc[i].gateway;
        c->subnet_mask = page30->mrc[i].subnet_mask;
        c->controller_fc_port_speed_setting = page30->mrc[i].fc_port_set;
        c->controller_beacon = page30->mrc[i].beacon;
// NOTDONEYET -- rank is not in the MRC_INFO_t field.
        c->controller_rank = i + 1;  // This is wrong. We don't have the field.

        c->controller_status = ise_page30_status8_convert(page30->mrc[i].general_status);
        c->controller_status_details = mrc_page30_status32_convert(bswap_32(page30->mrc[i].status));

// NOTDONEYET - controller_fc_port_status wrong, zero returned from page 0x30, should be 1 or 2.
        c->controller_fc_port_status = ise_page30_status8_convert(page30->mrc[i].fc_port_status);
        c->controller_fc_port_speed = page30->mrc[i].fc_port_speed;
// NOTE - controller_ethernet_link_up not provided in page 0x30.
        c->controller_ethernet_link_up = 0;  // Not provided        ?????????

        sprintf(c->controller_mac_address, "%02X:%02X:%02X:%02X:%02X:%02X",
                page30->mrc[i].mac_address[0], page30->mrc[i].mac_address[1],
                page30->mrc[i].mac_address[2], page30->mrc[i].mac_address[3],
                page30->mrc[i].mac_address[4], page30->mrc[i].mac_address[5]);

        c->controller_temperature = page30->mrc[i].temp;
    }

    /*
     * Battery pack fields
     */
    for (i = 0; i < 2; i++)
    {
        struct ise_battery_version_2    *b = &ise->battery[i];

        b->battery_beacon = page30->battery[i].beacon;

        memcpy(b->battery_model, page30->battery[i].model,
                sizeof(b->battery_model));
        memcpy(b->battery_serial_number, page30->battery[i].serial_number,
                sizeof(b->battery_serial_number));
        memcpy(b->battery_part_number, page30->battery[i].part_number,
                sizeof(b->battery_part_number));
        memset(b->battery_type, 0, sizeof(b->battery_type));
        strcpy(b->battery_type, "Lead Acid");   /* Set as constant */

        b->battery_x_position = i + 1;
        b->battery_y_position = 0;
        b->battery_z_position = 0;

        b->battery_status = ise_page30_status8_convert(page30->battery[i].general_status);
        b->battery_status_details = bat_page30_status32_convert(bswap_32(page30->battery[i].battery_status));

        b->battery_remaining_charge = bswap_32(page30->battery[i].remaining_charge);
        b->battery_max_charge = bswap_16(page30->battery[i].max_charge);
        b->battery_max_charge_capacity = bswap_16(page30->battery[i].max_capacity);
        b->battery_min_holdup_time = bswap_16(page30->battery[i].min_holdtime);
// NOTE: the state and details are backwards in page 0x30.
//        b->battery_charger_state = charger_page30_status8_convert(page30->battery[i].charger_status);
//        b->battery_charger_state_details = charger_page30_status32_convert(bswap_32(page30->battery[i].general_charger_status));
        b->battery_charger_state = charger_page30_status8_convert(bswap_32
                                           (page30->battery[i].general_charger_status));
        b->battery_charger_state_details = charger_page30_status32_convert(page30->battery[i].charger_status);
// NOTE - Fields not used: capacity
    }

    /*
     * Power supply fields
     */
    for (i = 0; i < 2; i++)
    {
        struct ise_powersupply_version_2    *ps = &ise->powersupply[i];

        ps->powersupply_beacon = page30->ps[i].beacon;

        memcpy(ps->powersupply_model, page30->ps[i].model,
                sizeof(ps->powersupply_model));
        memcpy(ps->powersupply_serial_number, page30->ps[i].serial_number,
                sizeof(ps->powersupply_serial_number));
// NOTE -- powersupply_part_number is wrong.
        memcpy(ps->powersupply_part_number, page30->ps[i].part_number,
                sizeof(ps->powersupply_serial_number));

        ps->powersupply_x_position = i + 1;
        ps->powersupply_y_position = 0;
        ps->powersupply_z_position = 0;

        ps->powersupply_status = ise_page30_status8_convert(page30->ps[i].general_status);
        ps->powersupply_status_details = ps_page30_status32_convert(bswap_32(page30->ps[i].status));

        ps->powersupply_fan1_status = ise_page30_status8_convert(bswap_16(page30->ps[i].fan1_status));
        ps->powersupply_fan1_speed = bswap_32(page30->ps[i].fan1_speed);
        ps->powersupply_fan2_status = ise_page30_status8_convert(bswap_16(page30->ps[i].fan2_status));
        ps->powersupply_fan2_speed = bswap_32(page30->ps[i].fan2_speed);
        ps->powersupply_temperature = page30->ps[i].temp;
    }


// The size of all disk drives in a datapac are the same. Use the first non-zero for size.
    INT64       disksize[2] = { 0, 0 }; // Size of disk in each DataPac.
    int         n_d[2];         // Number of Drives in each DataPac.
    n_d[0] = bswap_32(page30->data_pac[0].number_drives);       // DataPac 1 has this number of drives
    n_d[1] = bswap_32(page30->data_pac[1].number_drives);       // DataPac 2 has this number of drives

    /* It appears that this value is not just 10 or 20, sometimes less depending on ... who knows what. */
    if (n_d[0] > 10) {
        n_d[0] = 20;
    } else {
        n_d[0] = 10;
    }

    if (n_d[1] > 10) {
        n_d[1] = 20;
    } else {
        n_d[1] = 10;
    }

    /* Get the largest disk size for both datapacs. */
    for (i = 0; i < n_d[0]; i++)
    {
        INT64       b_c = 512LL * bswap_32(page30->drives[i].block_count);
        if (b_c > disksize[0])
        {
            disksize[0] = b_c;
        }
    }

    for (i = n_d[0]; i < n_d[0] + n_d[1]; i++)
    {
        INT64       b_c = 512LL * bswap_32(page30->drives[i].block_count);
        if (b_c > disksize[1])
        {
            disksize[1] = b_c;
        }
    }

    /*
     * Datapack fields
     */
    for (i = 0; i < 2; i++)
    {
        struct ise_datapac_version_2    *dp = &ise->datapac[i];

        dp->datapac_beacon = page30->data_pac[i].beacon;

// NOTDONEYET -- data_pac type balanced is not in enum.
// 0 - Unknown
// 1 - Balanced
// 2 - Capacity
// 3 - Performance
// enum xwstype__DataPacType1 {
// xwstype__DataPacType1__NEWER_PROTOCOL_VERSION_VALUE = 0,
// xwstype__DataPacType1__UNKNOWN = 1,
// xwstype__DataPacType1__RESERVED = 2,
// xwstype__DataPacType1__HIGH_CAPACITY = 3,
// xwstype__DataPacType1__HIGH_PERFORMANCE = 4};
        switch (page30->data_pac[i].type)
        {
        default:
        case 0:
            dp->datapac_type = 1;   // UNKNOWN
            break;

        case 1:
            dp->datapac_type = 2;   // Balanced
            break;

        case 2:
            dp->datapac_type = 3;   // Capacity
            break;

        case 3:
            dp->datapac_type = 4;   // Performance
            break;
        }

        memcpy(dp->datapac_serial_number, page30->data_pac[i].serial_number,
                sizeof(dp->datapac_serial_number));
        memcpy(dp->datapac_model, page30->data_pac[i].model,
                sizeof(dp->datapac_model));
        memcpy(dp->datapac_part_number, page30->data_pac[i].part_number,
                sizeof(dp->datapac_part_number));

        dp->datapac_x_position = i + 1;
        dp->datapac_y_position = 0;
        dp->datapac_z_position = 0;

        dp->datapac_status = ise_page30_status8_convert(page30->data_pac[i].general_status);
        dp->datapac_status_details = dp_page30_status32_convert(bswap_32(page30->data_pac[i].status));

// There is automatically reserved 3gb of disk space.
        dp->datapac_capacity = disksize[i] - (3LL * 1024 * 1024 * 1024);
// There is 2MB at beginning and 76MB at end of disk for diagnostics.
// There is 128MB for "per segment reserved" on the disk.
        dp->datapac_capacity -= ((2LL + 76 + 128) * 1024 * 1024);
// We want usable space in GB -- and datapac_capacity is INT64.
// This means, one drive's usable space, times number of drives,
// times percent not in sparing, rounded by half of 1GB(512MB),
// then converted from bytes to GB.
        dp->datapac_capacity = (((dp->datapac_capacity * n_d[i] *
                                  (100 - ise->spare_level)) / 100) +
                                    512 * 1024 * 1024) / (1024 * 1024 * 1024);

        memcpy(dp->datapac_fw_version, page30->data_pac[i].fw_version, 8);

        dp->datapac_temperature = page30->data_pac[i].temp;
        dp->datapac_health = page30->data_pac[i].health;

// NOTE - Fields not used: number_bad_drives, datapac_number, and number_drives sort of.
    }
// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#if 0
    int         n_bd[2];        // Number of Bad Drives in each DataPac.
    n_bd[0] = bswap_32(page30->data_pac[0].number_bad_drives);  // DataPac 1 has this number of bad drives
    n_bd[1] = bswap_32(page30->data_pac[1].number_bad_drives);  // DataPac 2 has this number of bad drives

    UINT32 d0;
    int cnt;
    char line[256];
    line[0] = line[1] = line[2] = line[3] = ' ';
    line[4] = '\0';
    cnt = 0;
    dprintf(DPRINTF_DEFAULT, "datapac 0 sparing %lld -- %d drives, %d bad drives, capacity %lld\n",
            ise->spare_level, bswap_32(page30->data_pac[0].number_drives), n_bd[0], ise->datapac[0].datapac_capacity);
    for (i = 0; i < n_d[0]; i++)
    {
        d0 = bswap_32(page30->drives[i].block_count);
        sprintf(line, "%s %9d", line, d0);
        if ((cnt++ % 6) == 5) {
            sprintf(line, "%s\n    ", line);
        }
    }
    dprintf(DPRINTF_DEFAULT, "%s\n", line);

    line[0] = line[1] = line[2] = line[3] = ' ';
    line[4] = '\0';
    cnt = 0;
    dprintf(DPRINTF_DEFAULT, "datapac 1 sparing %lld -- %d drives, %d bad drives, capacity %lld\n",
            ise->spare_level, bswap_32(page30->data_pac[1].number_drives), n_bd[1], ise->datapac[0].datapac_capacity);
    for (i = n_d[0]; i < n_d[0] + n_d[1]; i++)
    {
        d0 = bswap_32(page30->drives[i].block_count);
        sprintf(line, "%s %9d", line, d0);
        if ((cnt++ % 6) == 5) {
            sprintf(line, "%s\n    ", line);
        }
    }
    dprintf(DPRINTF_DEFAULT, "%s\n", line);
#endif  /* 0 */
// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
}

#endif /* MODEL_3000 || MODEL_7400 */

/*----------------------------------------------------------------------------
** Function Name: IniSES()
**
** Comments:
**  This function will initialize the SES process which examines the SES
**  devices to see if there are any warning or error conditions to report
**  to the user interface.  This function relies on discovery of SES
**  devices to be complete before it can examine any devices.
**
**--------------------------------------------------------------------------*/
NORETURN void InitSES(UNUSED TASK_PARMS *parms)
{
    LOG_SES_WWN_SLOT_DAT *pStdLog;
    LOG_SES_WWN_TEMP_DAT *pTempLog;
    LOG_SES_WWN_VOLT_DAT *pVoltLog;
    LOG_SES_WWN_CURR_DAT *pCurrLog;
    LOG_SES_WWN_SBOD_EXT_DAT *pSBODExtLog;
    UINT8       first_loops = 0;

    /*
     * Allocate a log message buffer for the function.  All log messages
     * can go through the same buffers since they are posted before going
     * to the next function.  Allocate one for each type of log message.
     */
    pStdLog = MallocWC(sizeof(*pStdLog));
    pTempLog = MallocWC(sizeof(*pTempLog));
    pVoltLog = MallocWC(sizeof(*pVoltLog));
    pCurrLog = MallocWC(sizeof(*pCurrLog));
    pSBODExtLog = MallocWC(sizeof(*pSBODExtLog));

    InitMutex(&sesMutex);

#if defined(MODEL_7000) || defined(MODEL_4700)
    /*
     * Initialize the ise temperature state array
     */
    {
        int         i;

        for (i = 0; i < MAX_DISK_BAYS; i++)
        {
            ise_chassis_temp_state[i] = ISE_TEMP_UNINITIALIZED;
        }
    }
#endif /* MODEL_7000 || MODEL_4700 */

    for (;;)
    {
        PerformingSES = FALSE;      /* Indicate that no SES in progress */

#if defined(MODEL_7000) || defined(MODEL_4700)
        UINT8       discover_flag = FALSE;

        if (discover_flag)
        {
            DiscoverSES(NULL);
            discover_flag = FALSE;
        }
#endif /* MODEL_7000 || MODEL_4700 */

        /*
         * Synchronize with bay fw downloads
         */
        UnlockMutex(&sesMutex);
        if (first_loops < 3)
        {
           TaskSleepMS(SES_POLL_INTERVAL1);
            ++first_loops;
        }
        else
        {
            TaskSleepMS(SES_POLL_INTERVAL);
        }
        (void)LockMutex(&sesMutex, MUTEX_WAIT);

        if (DiscoveringSES)
        {
            continue;
        }

        PerformingSES = TRUE;   /* Set variable to indicate SES in progress */

#if defined(MODEL_3000) || defined(MODEL_7400)
        SES_DEVICE *pSES;

        /*
         * Examine each SES device and see if there are any warnings or
         * critical conditions that need to be posted to the user interface.
         */
        for (pSES = SESList; pSES != NULL; pSES = pSES->NextSES)
        {
            SES_PAGE_02     *page2;
            SES_P80_XTEX    *page80;    /* page 80 and 81 same structure */

            if (pSES->devStat != PD_OP)
            {
                continue;
            }

            while (EL_TestInProgress())
            {
                TaskSleepMS(1000);      /* Wait for a second before continuing. */
            }

            /*
             * Get page 2 (device control and status) and examine
             * each element descriptor.  Post interesting events to
             * the user interface.
             */
            page2 = GetSESPageByWWN(SES_CMD_STATUS, pSES->WWN, pSES->LUN);
            if (page2)
            {
                SES_page2_Handle(pSES, page2, pStdLog, pTempLog, pVoltLog,
                            pCurrLog, pSBODExtLog);
            }

            if (pSES->devType == PD_DT_SBOD_SES)
            {
                /*
                 * Get page 80 (Xyratex switch control and status) and
                 * examine each port to see if any change that requires
                 * log message posting.
                 */
                page80 = GetSESPageByWWN(SES_CMD_LOOP_STAT_A, pSES->WWN, pSES->LUN);
                if (page80)
                {
                    SES_page80_Handle(pSES, page80, pSBODExtLog);
                }

                page80 = GetSESPageByWWN(SES_CMD_LOOP_STAT_B, pSES->WWN, pSES->LUN);
                if (page80)
                {
                    SES_page80_Handle(pSES, page80, pSBODExtLog);
                }
            }
        }
#else   /* MODEL_3000 || MODEL_7400 */
        /*
         * Examine each ISESES device and see if there are any warnings or
         * critical conditions that need to be posted to the user interface.
         */
        UINT32      local_nbu = 0;
        ISE_SES_DEVICE *pise;

        for (pise = ISESESList; pise; pise = pise->NextISE)
        {
            LOG_ISE_PAGE *page30;

            if (!pise->ip1 && !pise->ip2)
            {
                discover_flag = TRUE;
                continue;
            }

            while (EL_TestInProgress())
            {
                TaskSleepMS(1000);      /* Wait for a second before continuing. */
            }

// dprintf(DPRINTF_DEFAULT, "getting page30 from wwn=%llx LUN 0\n", pise->wwn);
            page30 = GetIseLogPageByWWN(0x30, pise->wwn, 0);
            if (page30)
            {
// dprintf(DPRINTF_DEFAULT, "processing page30 from wwn=%llx LUN 0\n", pise->wwn);
                /* Do the decoding of the packet and distribute */
                struct ise_info_version_2 ise_copy;

                local_nbu++;

                /*
                 * Build the Ise info version 2 packet from ISE page 30 data
                 */
                BuildIseInfoFromPage30(page30, &ise_copy);
                ISESESStatusHandle(pise, &ise_copy);
                ISEHandleEvents(pise, &ise_copy);
            }
            if (page30)
            {
                Free(page30);
            }
        }
        ISE_Number_Bays_Used = local_nbu;
#endif /* MODEL_3000 || MODEL_7400 */
    }
}


/**
******************************************************************************
**
**  @brief      Task to handle the running of SES discovery.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
#if defined(MODEL_3000) || defined(MODEL_7400)
void DiscoverSES_StartTask(void)
{
    if (!gTaskActiveDiscoverSES)
    {
        gTaskActiveDiscoverSES = TRUE;
        TaskCreate(DiscoverSES_Task, NULL);
    }
}
#endif /* MODEL_3000 || MODEL_7400 */

/**
******************************************************************************
**
**  @brief      Task to handle the running of SES discovery.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
#if defined(MODEL_3000) || defined(MODEL_7400)
static void DiscoverSES_Task(UNUSED TASK_PARMS *parms)
{
    DiscoverSES();
    SetDiscoveryComplete(true);
    gTaskActiveDiscoverSES = FALSE;
}
#endif /* MODEL_3000 || MODEL_7400 */

#if defined(MODEL_7000) || defined(MODEL_4700)

/**
******************************************************************************
**
**  @brief      Search ISE list for ISE with matching ses.
**
**  @return     Pointer to entry found, or NULL if not found.
**
******************************************************************************
**/
static ISE_SES_DEVICE *FindISE(UINT16 ses)
{
    ISE_SES_DEVICE *pise;

    for (pise = ISESESList; pise; pise = pise->NextISE)
    {
        if (ses == pise->PID)
        {
            return pise;
        }
    }

    return NULL;
}


/**
******************************************************************************
**
**  @brief      Delete entry from ISE list.
**
**  @return     none
**
******************************************************************************
**/
static void DeleteISE(ISE_SES_DEVICE *pise)
{
    ISE_SES_DEVICE *pisetmp;
    ISE_SES_DEVICE *piseprev;

    if (pise == ISESESList)
    {
        ISESESList = pise->NextISE;
        Free(pise);
        return;
    }

    piseprev = ISESESList;
    for (pisetmp = piseprev->NextISE; pisetmp; pisetmp = pisetmp->NextISE)
    {
        if (pisetmp == pise)
        {
            piseprev->NextISE = pise->NextISE;
            Free(pise);
            return;
        }
    }
}


/**
******************************************************************************
**
**  @brief      Add entry to ISE list.
**
**  @return     none
**
******************************************************************************
**/
static void AddISE(ISE_SES_DEVICE *pise)
{
    /*
     * Insert the new one at the head of the queue.
     */

    if (ISESESList == NULL)
    {
        pise->NextISE = NULL;
        ISESESList = pise;
    }
    else
    {
        pise->NextISE = ISESESList;
        ISESESList = pise;
    }
}


/**
******************************************************************************
**
**  @brief      Get the IP addresses of ISE
**
**  @param      Bay id
**
**  @return     ip address1, ip address2
**
******************************************************************************
**/

static void GetISEBayIPs(UINT16 bayid, UINT32 *ip1, UINT32 *ip2, UINT64 *wwn)
{
    MRGETISEIP_REQ *inPkt;
    MRGETISEIP_RSP *outPkt;
    UINT16      rc;

    inPkt = MallocWC(sizeof(*inPkt));
    outPkt = MallocSharedWC(sizeof(*outPkt));

    /*
     * Set input parm from the input.
     */
    inPkt->bayid = bayid;

    /*
     * Send the request to BE.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(inPkt, sizeof(*inPkt), MRGETISEIP, outPkt, sizeof(*outPkt), 35000);

    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "GetISEBayIP: MRP failed: bay id = %d\n", bayid);
    }

    DelayedFree(MRGETISEIP, inPkt);

    if (rc != PI_TIMEOUT)
    {
        *ip1 = outPkt->ip1;
        *ip2 = outPkt->ip2;
        *wwn = outPkt->wwn;
        Free(outPkt);
    }
}


/*----------------------------------------------------------------------------
** Function Name: SendBeaconIseCommand()
**
** Comments:
**   This function sends the beacon ise component command to ASGSES program
**--------------------------------------------------------------------------*/

INT32 SendBeaconIseCommand(UINT16 bayid, UINT16 command, UINT8 light_on_off)
{
    struct ip_light
    {
        UINT32      ip;
        UINT8       on_off;
    } __attribute__ ((packed)) ip_light;
    UINT8       cmdflags = 0;
    ISE_SES_DEVICE *pise;
    UINT32      rc = PI_ERROR;

    pise = FindISE(bayid);

    if (!pise)
    {
        return DEBAYNOTFOUND;
    }

    if (pise->ip1)
    {
        ip_light.ip = pise->ip1;
    }
    else
    {
        ip_light.ip = pise->ip2;
    }

    if (!ip_light.ip)
    {
        return PI_ERROR;
    }

    ip_light.on_off = light_on_off;
    switch (command)
    {
        case ASGSES_cmd_ISE_Beacon:
            cmdflags = 0x03 + 0x40 * light_on_off;
            break;
        case ASGSES_cmd_MRC_Beacon0:
            cmdflags = 0x02 + 0x40 * light_on_off;
            break;
        case ASGSES_cmd_MRC_Beacon1:
            cmdflags = 0x82 + 0x40 * light_on_off;
            break;
        case ASGSES_cmd_PS_Beacon0:
            cmdflags = 0x01 + 0x40 * light_on_off;
            break;
        case ASGSES_cmd_PS_Beacon1:
            cmdflags = 0x81 + 0x40 * light_on_off;
            break;
        case ASGSES_cmd_Bat_Beacon0:
            cmdflags = 0x06 + 0x40 * light_on_off;
            break;
        case ASGSES_cmd_Bat_Beacon1:
            cmdflags = 0x86 + 0x40 * light_on_off;
            break;
        case ASGSES_cmd_DP_Beacon0:
            cmdflags = 0x05 + 0x40 * light_on_off;
            break;
        case ASGSES_cmd_DP_Beacon1:
            cmdflags = 0x85 + 0x40 * light_on_off;
            break;
        case ASGSES_cmd_SFP_Beacon0:
            cmdflags = 0x07 + 0x40 * light_on_off;
            break;
        case ASGSES_cmd_SFP_Beacon1:
            cmdflags = 0x87 + 0x40 * light_on_off;
            break;
        case ASGSES_cmd_CAP_Beacon0:
            cmdflags = 0x08 + 0x40 * light_on_off;
            break;
        case ASGSES_cmd_CAP_Beacon1:
            cmdflags = 0x88 + 0x40 * light_on_off;
            break;
        case ASGSES_cmd_BEZEL_Beacon:
            cmdflags = 0x0A + 0x40 * light_on_off;
            break;
    }
    rc = SendIseCmd(pise->wwn, cmdflags);
    if (rc != GOOD)
    {
        return PI_ERROR;
    }
    return PI_GOOD;
}

/* ------------------------------------------------------------------------ */
void IseUpdateIP(UINT16 bayid, IP_ADDRESS ip1, IP_ADDRESS ip2, UINT64 wwn)
{
    ISE_SES_DEVICE *pise;

    pise = FindISE(bayid);

    if (!pise)
    {
        pise = MallocWC(sizeof(*pise));
        pise->PID = bayid;
        AddISE(pise);
    }
    pise->ip1 = ip1;
    pise->ip2 = ip2;
    pise->wwn = wwn;
}


#endif /* MODEL_7000 || MODEL_4700 */

/*----------------------------------------------------------------------------
** Function Name: DiscoverSES()
**
** Comments:
**  This function will initialize all of the SES structures by going out
**  to the BEP and getting the list of SES devices and then individually
**  polling each of them for their construction.  As each is polled, a
**  strcuture will be allocated and linked to a list of SES devices.
**
**--------------------------------------------------------------------------*/
#if defined(MODEL_7000) || defined(MODEL_4700)
void DiscoverSES(UNUSED TASK_PARMS *parms)
#else  /* MODEL_7000 || MODEL_4700 */
void DiscoverSES(void)
#endif /* MODEL_7000 || MODEL_4700 */
{
    MRGETPINFO_RSP *devInfo;
    UINT16      i;
    UINT16      nextWWN;
    SES_DEVICE *SESPtr;
    SES_DEVICE *SESPrevPtr;
    PI_DISK_BAYS_RSP *pDiskBays = NULL;
    PI_PDISKS_RSP *pPDisks = NULL;
    UINT8       found = FALSE;

#if defined(MODEL_7000) || defined(MODEL_4700)
    ISE_SES_DEVICE *ISESESPtr;
    ISE_SES_DEVICE *pISESESTmp;
#else  /* MODEL_7000 || MODEL_4700 */
    UINT16      eCnt;
    UINT16      pElem;
    UINT16      pPtr;
    UINT16      cummOffset;
    SES_TYPE_DESC *pTDesc;
    SES_PAGE_00 *page0;
    SES_PAGE_01 *page1;
#endif /* MODEL_7000 || MODEL_4700 */

    /*
     * Increment the reference count
     */
    ++SesReferenceCount;

#if defined(MODEL_7000) || defined(MODEL_4700)
    while (SesReferenceCount > 1)
    {
        TaskSleepMS(100);
    }
#else  /* MODEL_7000 || MODEL_4700 */
    /*
     * If we are currently discovering, hold other processes here... */
    if (SesReferenceCount > 1)
    {
        /*
         * Try to run the least amount of times necessary
         */
        SesReferenceCount = 2;
        while (SesReferenceCount != 0)
        {
            TaskSleepMS(100);
        }
    }
#endif /* MODEL_7000 || MODEL_4700 */
    /*
     * Prevent the periodic poll by setting the
     * discovery in progress indicator.
     */
    DiscoveringSES = TRUE;

    while (SesReferenceCount > 0)
    {
        /*
         * If doing a periodic poll, do not do a
         * discovery operation because the linked
         * list will change during the operation.
         */
        while (PerformingSES)
        {
            TaskSleepMS(1000);
        }
        /*
         * Get the list of SES devices and physical devices from the BEP.
         * Wait here until we get valid lists since it does not make sense
         * to continue without them.
         */
        while (pDiskBays == NULL || pPDisks == NULL)
        {
            /*
             * The DiskBays and PhysicalDisks commands return NULL pointers if
             * something goes wrong within the calls.  This includes timeouts
             * and errors.  If timeouts occur then the BE blocked will be and
             * we will want to wait before trying additional requests.  If an
             * error occurred we are safe to try the requests immediately.
             */

            if (BEBlocked())
            {
                TaskSleepMS(100);
                continue;
            }

            pDiskBays = DiskBays();

            if (pDiskBays != NULL)
            {
                /*
                 * At this point we have a valid disk bays list so lets try
                 * and get the list of physical disks.
                 */
                pPDisks = PhysicalDisks();

                /*
                 * If we were unable to get the information for the physical
                 * disks then free up the disk bays.  We will then have to
                 * retry both requests again.
                 */
                if (pPDisks == NULL)
                {
                    /*
                     * Free the memory
                     */
                    Free(pDiskBays);
//                    pDiskBays = NULL;
                }
            }
        }

        /*
         * Zero out the WWN maps for devices used for LED control.
         */
        for (nextWWN = 0; nextWWN < SES_WWN_MAP_SIZE; nextWWN++)
        {
            WWNMap[nextWWN].WWN = 0;
            WWNMap[nextWWN].SES = NULL;
            WWNMap[nextWWN].Slot = 0;
            WWNMap[nextWWN].PID = 0;
        }

        /*
         * Go through the list of currently known SES controllers and delete
         * the ones that did not show up in the new list.  Leave the ones that
         * were in the list so we can prevent losing the page 2 data on a loop
         * event.
         */
        for (SESPtr = SESList, SESPrevPtr = NULL; SESPtr != NULL;)
        {
            /*
             * Search the list of new devices.
             */
            for (i = 0, found = FALSE; i < pDiskBays->count; i++)
            {
                devInfo = &pDiskBays->bayInfo[i];

                if (devInfo->pdd.wwn == SESPtr->WWN)
                {
                    found = TRUE;
                    break;
                }
            }

            if (((SESPtr != NULL) && !found))
            {
                if (SESPtr == SESList)
                {
                    SESList = SESPtr->NextSES;
                    Free(SESPtr);
                    SESPtr = SESList;
                }
                else
                {
                    SESPrevPtr->NextSES = SESPtr->NextSES;
                    Free(SESPtr);
                    SESPtr = SESPrevPtr->NextSES;
                }
            }
            else
            {
                SESPrevPtr = SESPtr;
                SESPtr = SESPtr->NextSES;
            }
        }

#if defined(MODEL_7000) || defined(MODEL_4700)
        /*
         * Go through the list of currently known ISE SES controllers and delete
         * the ones that did not show up in the new list.
         */
        for (ISESESPtr = ISESESList; ISESESPtr; ISESESPtr = pISESESTmp)
        {
            UINT8       foundise;

            pISESESTmp = ISESESPtr->NextISE;

            /*
             * Search the list of new devices.
             */
            foundise = FALSE;
            for (i = 0; i < pDiskBays->count; i++)
            {
                devInfo = &pDiskBays->bayInfo[i];
                if (devInfo->pdd.ses >= MAX_DISK_BAYS)
                {
                    continue;
                }

                if (devInfo->pdd.ses == ISESESPtr->PID)
                {
                    foundise = TRUE;
                    break;
                }
            }

            if (!foundise)
            {
                // Stop monitoring this bay's IP addresses.
                DeleteStatsData(ISESESPtr->PID);
                DeleteISE(ISESESPtr);
            }
        }

        for (i = 0; i < pDiskBays->count; i++)
        {
            UINT32      ip1;
            UINT32      ip2;
            UINT64      wwn;

            devInfo = &pDiskBays->bayInfo[i];
            if (devInfo->pdd.ses >= MAX_DISK_BAYS)
            {
                continue;
            }

            /*
             * Allocate an ISE SES tracking structure if one is needed.
             */

            ISESESPtr = FindISE(devInfo->pdd.ses);

            if (!ISESESPtr)
            {
                ISESESPtr = MallocWC(sizeof(*ISESESPtr));
                ISESESPtr->PID = devInfo->pdd.ses;

                AddISE(ISESESPtr);
                /*
                 * Get the IP addresses from Proc to store in
                 * ISESES linked list
                 */

                GetISEBayIPs(ISESESPtr->PID, &ip1, &ip2, &wwn);

                /*
                 * Fill in the data from the device info.
                 */
                ISESESPtr->ip1 = ip1;
                ISESESPtr->ip2 = ip2;
                ISESESPtr->wwn = wwn;
            }

//         fprintf(stderr, "%s: bay id %d, IP addrs %d.%d.%d.%d, %d.%d.%d.%d\n",
//                 __FUNCTION__, ISESESPtr->PID,
//                 (ip1 >> 24) & 0xFF, (ip1 >> 16) & 0xFF, (ip1 >> 8) & 0xFF,
//                 ip1 & 0xFF,
//                 (ip2 >> 24) & 0xFF, (ip2 >> 16) & 0xFF, (ip2 >> 8) & 0xFF,
//                 ip2 & 0xFF);
        }
#endif /* MODEL_7000 || MODEL_4700 */

        for (i = 0; i < pDiskBays->count; i++)
        {
            devInfo = &pDiskBays->bayInfo[i];

            /*
             * Allocate an SES tracking structure if one is needed.
             */
            found = FALSE;

            for (SESPtr = SESList; SESPtr != NULL; SESPtr = SESPtr->NextSES)
            {
                if (devInfo->pdd.wwn == SESPtr->WWN)
                {
                    /*
                     * Mark found and save the pointer.
                     */
                    found = TRUE;
                    break;
                }
            }

            if (!found)
            {
                if (SESList == NULL)
                {
                    SESList = SESPtr = MallocWC(sizeof(*SESPtr));
                }
                else
                {
                    /*
                     * Insert the new one at the head of the queue.
                     */
                    SESPtr = SESList;
                    SESList = MallocWC(sizeof(*SESList));
                    SESList->NextSES = SESPtr;
                    SESPtr = SESList;
                }

                SESPtr->OldPage2 = NULL;
            }

            /*
             * Fill in the data from the device info.
             */
            SESPtr->Channel = devInfo->pdd.channel;
            SESPtr->FCID = devInfo->pdd.id;
            SESPtr->WWN = devInfo->pdd.wwn;
            SESPtr->PID = devInfo->pdd.pid;

            SESPtr->LUN = devInfo->pdd.lun;
            SESPtr->devStat = devInfo->pdd.devStat;
            SESPtr->devType = devInfo->pdd.devType;
            SESPtr->SupportedPages = 0;

            memcpy(SESPtr->pd_rev, devInfo->pdd.prodRev, 4);

#if defined(MODEL_3000) || defined(MODEL_7400)
            /*
             * Now go issue a receive diags command on page 0 to see
             * what pages are supported.  Record this in the supported
             * pages field.
             */
            if (SESPtr->devStat == PD_OP)
            {
                while (EL_TestInProgress())
                {
                    TaskSleepMS(1000);      /* Wait for a second before continuing. */
                }

                page0 = GetSESPageByWWN(SES_CMD_SUPPORTED, SESPtr->WWN, SESPtr->LUN);

                if (page0 != NULL)
                {
                    for (pPtr = 0; pPtr < tm_const_ntohs(page0->Length); pPtr++)
                    {
                        switch (page0->Pages[pPtr])
                        {
                            case SES_CMD_SUPPORTED:
                            case SES_CMD_CONFIG:
                            case SES_CMD_CONTROL:
                            case SES_CMD_HELP_TEXT:
                            case SES_CMD_STRING_OUT:
                            case SES_CMD_THRESH_IN:
                            case SES_CMD_ARRAY_STAT:
                            case SES_CMD_ELEMENT:
                            case SES_CMD_SHORT_STAT:
                                SESPtr->SupportedPages |= (1 << page0->Pages[pPtr]);
                                break;

                            default:
                                break;
                        }
                    }

                    /*
                     * Free the page.
                     */
                    Free(page0);
                }
            }

            /*
             * If the device supports the config page, get it and
             * create a mapping of the element types to offsets
             * within the page.  If the device is inoperable, then
             * none of the supported pages bits will be set other
             * than the enclosure type, so this will get skipped.
             */
            if ((SESPtr->devStat == PD_OP) &&
                (SESPtr->SupportedPages & (1 << SES_CMD_CONFIG)))
            {
                while (EL_TestInProgress())
                {
                    TaskSleepMS(1000);      /* Wait for a second before continuing. */
                }

                page1 = GetSESPageByWWN(SES_CMD_CONFIG, SESPtr->WWN, SESPtr->LUN);

                if (page1 != NULL)
                {
                    /*
                     * Clear out the offsets.  A huge number indicated no
                     * support for the device type.
                     */
                    for (pElem = 0; pElem <= SES_ET_MAX_VAL; pElem++)
                    {
                        SESPtr->Map[pElem] = SES_ET_INVALID;
                    }

                    /*
                     * Parse through the element descriptors and create
                     * the map of the elements for the status and control
                     * page.
                     */
                    for (pTDesc = (PSES_TYPE_DESC)(&page1->EnclDescLength + page1->EnclDescLength + 1), cummOffset = eCnt = 0;
                         eCnt < page1->NumElementTypes; eCnt++)
                    {
                        /*
                         * Get the element type value.  Truncate down in the
                         * case of values outside the range of the lookup
                         * table.  This will effectively place our VU page
                         * at the last entry.
                         */
                        if (pTDesc->ElementType >= SES_ET_MAX_VAL)
                        {
                            pElem = SES_ET_MAX_VAL - 1;
                        }
                        else
                        {
                            pElem = pTDesc->ElementType;
                        }

                        /*
                         * Place the cummulative offset into the map and
                         * then add in the number of slots plus one for the
                         * cummulative offset.  Plus one is done in order to
                         * account for the overall control entry for each
                         * element type.
                         */
                        SESPtr->Map[pElem] = cummOffset;
                        SESPtr->Slots[pElem] = pTDesc->NumPossibleElem;
                        cummOffset += pTDesc->NumPossibleElem + 1;
                        pTDesc += 1;
                    }

                    /*
                     * Update the total number of slots and free the page.
                     */
                    SESPtr->TotalSlots = cummOffset;
                    Free(page1);
                }
            }

            if (SESPtr->devStat == PD_OP)
            {
                /*
                 * Now gang write the LEDs
                 */
                SESGangLEDCtrl(SESPtr);
            }
#else  /* MODEL_3000 || MODEL_7400 */
            SESPtr->SupportedPages = 0;
#endif /* MODEL_3000 || MODEL_7400 */
        }

        /*
         * Get each drive information records, look up the SES enclosure
         * to make sure it exists, and  if it does, record it in the WWN
         * map table.
         */
        nextWWN = 0;

        for (i = 0; i < pPDisks->count; i++)
        {
            /*
             * Look up the SES device.
             */
            for (SESPtr = SESList; SESPtr != NULL; SESPtr = SESPtr->NextSES)
            {
                if (SESPtr->PID == pPDisks->pdiskInfo[i].pdd.ses)
                {
                    /*
                     * Found it.  Record it in the table and exit the loop.
                     */
                    WWNMap[nextWWN].PID = pPDisks->pdiskInfo[i].pdd.pid;
                    WWNMap[nextWWN].WWN = pPDisks->pdiskInfo[i].pdd.wwn;
                    WWNMap[nextWWN].Slot = pPDisks->pdiskInfo[i].pdd.slot;
                    WWNMap[nextWWN++].SES = SESPtr;
                    break;
                }
            }
        }

        /*
         * Free the memory and set to NULL.
         */
        Free(pDiskBays);
//        pDiskBays = NULL;

        Free(pPDisks);
//        pPDisks = NULL;

        /*
         * Decrement our reference count
         */
        --SesReferenceCount;

        /*
         * Give someone else a chance
         */
        TaskSwitch();
    }                           /* while loop */

    /*
     * Update the LED's
     */
    UpdateLeds();

    DiscoveringSES = FALSE;
#if defined(MODEL_7000) || defined(MODEL_4700)
    SetDiscoveryComplete(true);
#endif /* MODEL_7000 || MODEL_4700 */
}

#if defined(MODEL_7000) || defined(MODEL_4700)

/*----------------------------------------------------------------------------
** Function Name: SendIseCmd()
**
** Comments:
**  This function sends a command to an ISE using the vendor specific page 0xF9
**
**--------------------------------------------------------------------------*/
static UINT16 SendIseCmd(UINT64 wwn, UINT8 value)
{
    MRSCSIIO_REQ *inPkt = NULL;
    MRSCSIIO_RSP *outPkt = NULL;
    UINT16      rc = PI_GOOD;

    inPkt = MallocSharedWC(sizeof(*inPkt));
    outPkt = MallocSharedWC(sizeof(*outPkt));

    /*
     * Set input parm from the input.
     */
    inPkt->idchc = MRSCSIIO_USE_WWN;
    inPkt->wwn = wwn;
    inPkt->cmdlen = 12;
    inPkt->func = MRSCSIIO_CTL;
    inPkt->strat = MRSCSIIO_NORM;
    inPkt->timeout = 14;
    inPkt->flags = 1;
    inPkt->retry = 0;
    inPkt->lun = 0;             /* use lun=0 on ISE */
    inPkt->blen = 0;
    inPkt->cdb[0] = 0xF9;       /* Opcode                       */
    inPkt->cdb[1] = 0x02;       /* Command                      */
    inPkt->cdb[2] = 0;          /* */
    inPkt->cdb[3] = 0x00;       /* */
    inPkt->cdb[4] = 0x00;       /* */
    inPkt->cdb[5] = 0x00;       /* */
    inPkt->cdb[6] = 0x00;       /* */
    inPkt->cdb[7] = 0;          /* */
    inPkt->cdb[8] = 0;          /* */
    inPkt->cdb[9] = 0x00;       /* */
    inPkt->cdb[10] = 0x00;      /* */
    inPkt->cdb[11] = value;     /* Flags Bit7=which side, Bit6=on/off, bits 0-3=FRU */

//     fprintf(stderr, "SendIseCmd: wwn=%llx, CDB = %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n",
//             wwn, inPkt->cdb[0], inPkt->cdb[1], inPkt->cdb[2], inPkt->cdb[3],
//             inPkt->cdb[4], inPkt->cdb[5], inPkt->cdb[6], inPkt->cdb[7], inPkt->cdb[8],
//             inPkt->cdb[9], inPkt->cdb[10], inPkt->cdb[11]);

    /*
     * Send the request to the ISE.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(inPkt, sizeof(*inPkt), MRSCSIIO, outPkt, sizeof(*outPkt), 10000);

    if (rc == PI_ERROR)
    {
        dprintf(DPRINTF_DEFAULT, "SendIseCmd: MRP failed: SenseKey 0x%02hhX, ASC 0x%02hhX ASCQ 0x%02hhX\n",
                outPkt->sense, outPkt->asc, outPkt->ascq);
    }

    if (rc != PI_TIMEOUT)
    {
        Free(inPkt);
        Free(outPkt);
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function Name: GetIseLogPageByWWN()
**
** Comments:
**  This function will grab a log page from an ISE device.
**
**--------------------------------------------------------------------------*/
static void *GetIseLogPageByWWN(UINT16 page, UINT64 wwn, UINT16 lun)
{
    MRSCSIIO_REQ *inPkt = NULL;
    MRSCSIIO_RSP *outPkt = NULL;
    MRSCSIIO_RSP *retPkt = NULL;
    UINT16      rc = PI_GOOD;
    int         sz = 0;

    sz = sizeof(LOG_ISE_PAGE);

    inPkt = MallocSharedWC(sizeof(*inPkt));
    outPkt = MallocSharedWC(sizeof(*outPkt));

    /*
     * Set input parm from the input.
     */
    inPkt->idchc = MRSCSIIO_USE_WWN;
    inPkt->wwn = wwn;
    inPkt->cmdlen = 10;
    inPkt->func = MRSCSIIO_INPUT;
    inPkt->strat = MRSCSIIO_NORM;
    inPkt->timeout = 14;
    inPkt->flags = 1;
    inPkt->retry = 0;
    inPkt->lun = lun;
    inPkt->blen = sizeof(LOG_ISE_PAGE);
    retPkt = inPkt->bptr = MallocSharedWC(sizeof(LOG_ISE_PAGE));
    inPkt->cdb[0] = 0x4D;       /* Request log page 30          */
    inPkt->cdb[1] = 0x00;       /* Page code valid              */
    inPkt->cdb[2] = page;       /* Page code                    */
    inPkt->cdb[3] = 0x00;       /* */
    inPkt->cdb[4] = 0x00;       /* */
    inPkt->cdb[5] = 0x00;       /* */
    inPkt->cdb[6] = 0x00;       /* */
    inPkt->cdb[7] = sz / 256;   /* Data length                  */
    inPkt->cdb[8] = sz % 256;   /* Data length                  */
    inPkt->cdb[9] = 0x00;       /* Control                      */

#if 0
fprintf(stderr, "GetIseLogPage: wwn=%llx, CDB = %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X #bytesreq=%d\n",
        wwn, inPkt->cdb[0], inPkt->cdb[1], inPkt->cdb[2], inPkt->cdb[3],
        inPkt->cdb[4], inPkt->cdb[5], inPkt->cdb[6], inPkt->cdb[7], inPkt->cdb[8],
       inPkt->cdb[9], sizeof(LOG_ISE_PAGE));
#endif /* 0 */
    /*
     * Send the request to the ISE.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(inPkt, sizeof(*inPkt), MRSCSIIO, outPkt, sizeof(*outPkt), 10000);

    if (rc == PI_ERROR)
    {
        dprintf(DPRINTF_DEFAULT, "GetIseLogPage: MRP failed: SenseKey 0x%02hhX, ASC 0x%02hhX ASCQ 0x%02hhX\n",
                outPkt->sense, outPkt->asc, outPkt->ascq);
    }

    if (rc != PI_TIMEOUT)
    {
        Free(inPkt);
        Free(outPkt);
    }

    /*
     * If there was an error, free the buffer allocated and return
     * a NULL pointer.  If there was no error, return the buffer and
     * leave it up to the caller to return it.  Also check the page
     * code and length to make sure it was a good page.
     */
    if (rc != PI_GOOD ||
        ((LOG_ISE_PAGE *)retPkt)->PageCode != page ||
        ((LOG_ISE_PAGE *)retPkt)->Length == 0)
    {
        DelayedFree(MRSCSIIO, retPkt);
        return NULL;
    }

    return retPkt;
}
#endif /* MODEL_7000 || MODEL_4700 */


/*----------------------------------------------------------------------------
** Function Name: GetSESPageByWWN()
**
** Comments:
**  This function will grab a diag page from an SES device.
**
**--------------------------------------------------------------------------*/
void       *GetSESPageByWWN(UINT16 page, UINT64 wwn, UINT16 lun)
{
    MRSCSIIO_REQ *inPkt = NULL;
    MRSCSIIO_RSP *outPkt = NULL;
    MRSCSIIO_RSP *retPkt = NULL;
    UINT16      rc = PI_GOOD;

    inPkt = MallocSharedWC(sizeof(*inPkt));
    outPkt = MallocSharedWC(sizeof(*outPkt));

    /*
     * Set input parm from the input.
     */
    inPkt->idchc = MRSCSIIO_USE_WWN;
    inPkt->wwn = wwn;
    inPkt->cmdlen = 6;
    inPkt->func = MRSCSIIO_INPUT;
    inPkt->strat = MRSCSIIO_NORM;
    inPkt->timeout = 8;
    inPkt->flags = 1;
    inPkt->retry = 1;
    inPkt->lun = lun;
    inPkt->blen = 0x800;
    retPkt = inPkt->bptr = MallocSharedWC(0x800);
    inPkt->cdb[0] = 0x1C;       /* Receive diagnostic results   */
    inPkt->cdb[1] = 0x01;       /* Page code valid              */
    inPkt->cdb[2] = page;       /* Page code                    */
    inPkt->cdb[3] = 0x08;       /* Data length                  */
    inPkt->cdb[4] = 0x00;       /* Data length                  */
    inPkt->cdb[5] = 0x00;       /* Control                      */

//    dprintf(DPRINTF_SES, "GetSESPage: CDB = %02hhX%02hhX%02hhX%02hhX%02hhX%02hhX\n",
//            inPkt->cdb[0], inPkt->cdb[1], inPkt->cdb[2], inPkt->cdb[3],
//            inPkt->cdb[4], inPkt->cdb[5]);

    /*
     * Send the request to Thunderbolt.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(inPkt, sizeof(*inPkt), MRSCSIIO, outPkt, sizeof(*outPkt), 35000);

    if (rc == PI_ERROR)
    {
        dprintf(DPRINTF_DEFAULT, "GetSESPage: MRP failed: SenseKey 0x%02hhX, ASC 0x%02hhX ASCQ 0x%02hhX\n",
                outPkt->sense, outPkt->asc, outPkt->ascq);
    }

    if (rc != PI_TIMEOUT)
    {
        Free(inPkt);
        Free(outPkt);
    }

    /*
     * If there was an error, free the buffer allocated and return
     * a NULL pointer.  If there was no error, return the buffer and
     * leave it up to the caller to return it.  Also check the page
     * code and length to make sure it was a good page.
     */
    if ((rc != PI_GOOD) ||
        (((PSES_PAGE_00)retPkt)->PageCode != page) ||
        (((PSES_PAGE_00)retPkt)->Length == 0))
    {
        DelayedFree(MRSCSIIO, retPkt);
        return NULL;
    }

//    UINT16      i,
//                j;
//
//    j = (((UINT8 *)retPkt)[2] << 8) + ((UINT8 *)retPkt)[3];
//    dprintf(DPRINTF_SES, "GetSESPage: Length = %04X\n", j);

//    for (i = 0; i < j; i += 16)
//    {
//        dprintf(DPRINTF_SES, "GetSESPage: data = %02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX "
//                "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX\n",
//                ((UINT8 *)retPkt)[i + 0], ((UINT8 *)retPkt)[i + 1],
//                ((UINT8 *)retPkt)[i + 2], ((UINT8 *)retPkt)[i + 3],
//                ((UINT8 *)retPkt)[i + 4], ((UINT8 *)retPkt)[i + 5],
//                ((UINT8 *)retPkt)[i + 6], ((UINT8 *)retPkt)[i + 7],
//                ((UINT8 *)retPkt)[i + 8], ((UINT8 *)retPkt)[i + 9],
//                ((UINT8 *)retPkt)[i + 10], ((UINT8 *)retPkt)[i + 11],
//                ((UINT8 *)retPkt)[i + 12], ((UINT8 *)retPkt)[i + 13],
//                ((UINT8 *)retPkt)[i + 14], ((UINT8 *)retPkt)[i + 15]);
//    }

    return (retPkt);
}

/*----------------------------------------------------------------------------
** Function Name: SendSESPageByWWN()
**
** Comments:
**  This function will send a diag page to an SES device.
**
**--------------------------------------------------------------------------*/
INT32 SendSESPageByWWN(UINT64 wwn, UINT16 lun, void *data)
{
    MRSCSIIO_REQ *inPkt = NULL;
    MRSCSIIO_RSP *outPkt = NULL;
    INT32       rc = PI_GOOD;
    INT32       retry = 0;

    inPkt = MallocSharedWC(sizeof(*inPkt));
    outPkt = MallocSharedWC(sizeof(*outPkt));

    /*
     * Set input parm from the input.
     */
    inPkt->idchc = MRSCSIIO_USE_WWN;
    inPkt->wwn = wwn;
    inPkt->cmdlen = 6;
    inPkt->func = MRSCSIIO_OUTPUT;
    inPkt->strat = MRSCSIIO_NORM;
    inPkt->timeout = 8;
    inPkt->flags = 1;
    inPkt->retry = 1;
    inPkt->lun = lun;
    inPkt->blen = tm_const_ntohs(((PSES_PAGE_00)data)->Length) + 4;
    inPkt->bptr = data;
    inPkt->cdb[0] = 0x1D;       /* Send diagnostic              */
    inPkt->cdb[1] = 0x10;       /* Page code valid              */
    inPkt->cdb[2] = 0x00;       /* Reserved                     */
    inPkt->cdb[3] = (inPkt->blen & 0xFF00) >> 8;        /* Data length     */
    inPkt->cdb[4] = (inPkt->blen & 0x00FF);
    inPkt->cdb[5] = 0x00;       /* Control                      */

//    dprintf(DPRINTF_SES, "SendSESPage: CDB =  %02hhX%02hhX%02hhX %02hhX%02hhX %02hhX\n",
//            inPkt->cdb[0], inPkt->cdb[1], inPkt->cdb[2], inPkt->cdb[3],
//            inPkt->cdb[4], inPkt->cdb[5]);
//
//    UINT16      i;
//#if 0
//    for (i = 0; i < inPkt->blen; i += 16)
//#else
//    i = 0;
//#endif
//    {
//        dprintf(DPRINTF_SES, "SendSESPage: data = %02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX "
//                "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX\n",
//                ((UINT8 *)data)[i + 0], ((UINT8 *)data)[i + 1], ((UINT8 *)data)[i + 2],
//                ((UINT8 *)data)[i + 3], ((UINT8 *)data)[i + 4], ((UINT8 *)data)[i + 5],
//                ((UINT8 *)data)[i + 6], ((UINT8 *)data)[i + 7], ((UINT8 *)data)[i + 8],
//                ((UINT8 *)data)[i + 9], ((UINT8 *)data)[i + 10], ((UINT8 *)data)[i + 11],
//                ((UINT8 *)data)[i + 12], ((UINT8 *)data)[i + 13], ((UINT8 *)data)[i + 14],
//                ((UINT8 *)data)[i + 15]);
//    }

    /*
     * Send the request to Thunderbolt.  This function handles timeout
     * conditions and task switches while waiting.
     */
    do
    {
        if (retry > 0)
        {
            dprintf(DPRINTF_DEFAULT, "SendSESPage:-retrying the send diag command -- retry count =%x\n",
                    retry);
            memset(outPkt, 0, sizeof(*outPkt));
        }

        rc = PI_ExecMRP(inPkt, sizeof(*inPkt), MRSCSIIO, outPkt, sizeof(*outPkt), 35000);

        if ((rc != PI_TIMEOUT) && (outPkt->asc == 0x35) && (outPkt->ascq == 0x2))
        {
            dprintf(DPRINTF_DEFAULT, "SendSESPage:Checkcondition>-SenseKey 0x%02hhX, ASC 0x%02hhX ASCQ 0x%02hhX,status 0x%02hhX ret=%x\n",
                    outPkt->sense, outPkt->asc, outPkt->ascq, outPkt->status, rc);
            if (retry++ == 5)
            {
                /*
                 * Exit from retry loop
                 */
                retry = 0;
            }
            else
            {
                /*
                 * Wait for 1000 msec and then retry again
                 */
                TaskSleepMS(1000);
            }
        }
        else
        {
            /*
             * Exit from retry loop
             */
            retry = 0;
        }
    } while (retry);

    if (rc == PI_ERROR)
    {
        dprintf(DPRINTF_DEFAULT, "SendSESPage: MRP failed: SenseKey 0x%02hhX, ASC 0x%02hhX ASCQ 0x%02hhX\n",
                outPkt->sense, outPkt->asc, outPkt->ascq);
    }

    /*
     * Free the allocated memory, use DelayedFree for the input packet
     * since it contains a PCI address.
     */
    DelayedFree(MRSCSIIO, inPkt);

    if (rc != PI_TIMEOUT)
    {
        Free(outPkt);
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function Name: SESAlarmCtrl()
**
** Comments:
**  This function will send a diag page to an SES device in order to modify
**  the alarm state.
**
**--------------------------------------------------------------------------*/
void SESAlarmCtrl(SES_DEVICE *enclosure, UINT8 setting)
{
    PSES_PAGE_02 page2;
    PSES_ELEM_CTRL alarm_ptr = NULL;

    if ((enclosure == NULL) || (enclosure->Map[SES_ET_AUD_ALARM] == SES_ET_INVALID))
    {
        return;
    }

    /*
     * Get the configuration page, modify it and then send it back out
     * with the appropriate bit settings for the alarm control.
     */
    if ((enclosure->devStat == PD_OP) &&
        (NULL != (page2 = GetSESPageByWWN(SES_CMD_STATUS, enclosure->WWN, enclosure->LUN))))
    {
        /*
         * Get the slot number in page 2.  Adjust for the default slot.
         */
        alarm_ptr = &(page2->Control[enclosure->Map[SES_ET_AUD_ALARM] + 1]);
        alarm_ptr->CommonCtrl |= SES_CC_SELECT;
        alarm_ptr->Ctrl.Generic.Ctrl2 = setting;

        /*
         * Clear the status byte to prevent latching an error.
         */
        page2->Status = 0;

        /*
         * Now write the page back out.
         */
        SendSESPageByWWN(enclosure->WWN, enclosure->LUN, page2);

        Free(page2);

    }
}

/*----------------------------------------------------------------------------
** Function Name: SESBypassCtrl()
**
** Comments:
**  This function will send a diag page to an SES device in order to modify
**  the bypass state.
**
** Input:
**  searchSES - SES ID containing drive to alter
**  slot      - slot in enclosure
**  setting   - SES_C2DEV_RBYPASSA  0x08
**              SES_C2DEV_RBYPASSB  0x04
**
**--------------------------------------------------------------------------*/
void SESBypassCtrl(UINT16 searchSES, UINT8 slot, UINT8 setting)
{
    SES_DEVICE *pSES = NULL;
    UINT16      mapNum;
    PSES_PAGE_02 page2;
    PSES_ELEM_CTRL pCtrl;
    PSES_P82_XTEX page8x;

    /*
     * Find the enclosure in the SES list.  If it does not exist, it
     * means that we cannot control the device.
     */
    for (pSES = SESList; pSES != NULL; pSES = pSES->NextSES)
    {
        if (pSES->PID == searchSES)
        {
            break;
        }
    }

    /*
     * There are two method for bypass control.  One is the standard method
     * that is supported by the Adaptec/Eurologic enclosures and the other
     * is a non-standard method supported by the Xyratex enclosures.  Use
     * the device type field in the SES information to determine which one
     * to use.
     */
    if ((pSES != NULL) && (pSES->devStat == PD_OP))
    {
        if (pSES->devType == PD_DT_SBOD_SES)
        {
            /*
             * This is a Xyratex bay.  To bypass or unbypass it, we need to
             * modify the port in two commands, one to each port.  We will
             * assume that we are already in the opposite state, that is, we
             * will assume that if the port is being bypassed, that it is
             * unbypassed at the time.
             */
            if ((NULL != (page8x = GetSESPageByWWN(SES_CMD_PORT_CFG_A, pSES->WWN, pSES->LUN))))
            {
                /*
                 * This is the port A page.  Check the setting of the input
                 * to this function and then send the command to either turn
                 * the bypass on or off.
                 */
                if (setting & SES_C2DEV_RBYPASSA)
                {
                    page8x->Control[0].Action = 0x02;
                }
                else
                {
                    page8x->Control[0].Action = 0x01;
                }

                /*
                 * Set the slot number and the length.  The length is the
                 * size of the base structure plus one extension minus the
                 * four byte header for a standard SES page.  Note that the
                 * slot is off by 4 since the host ports are counted in the
                 * set of ports.
                 */
                page8x->Control[0].Port = slot + 4;
                page8x->Length = tm_const_ntohs(sizeof(SES_P82_XTEX) + sizeof(SES_P82XTEXCtl) - 4);

                SendSESPageByWWN(pSES->WWN, pSES->LUN, (void *)page8x);

                Free(page8x);
            }

            if ((NULL != (page8x = GetSESPageByWWN(SES_CMD_PORT_CFG_B, pSES->WWN, pSES->LUN))))
            {
                /*
                 * This is the port A page.  Check the setting of the input
                 * to this function and then send the command to either turn
                 * the bypass on or off.
                 */
                if (setting & SES_C2DEV_RBYPASSB)
                {
                    page8x->Control[0].Action = 0x02;
                }
                else
                {
                    page8x->Control[0].Action = 0x01;
                }

                /*
                 * Set the slot number and the length.  The length is the
                 * size of the base structure plus one extension minus the
                 * four byte header for a standard SES page.  Note that the
                 * slot is off by 4 since the host ports are counted in the
                 * set of ports.
                 */
                page8x->Control[0].Port = slot + 4;
                page8x->Length = tm_const_ntohs(sizeof(SES_P82_XTEX) + sizeof(SES_P82XTEXCtl) - 4);

                SendSESPageByWWN(pSES->WWN, pSES->LUN, (void *)page8x);

                Free(page8x);
            }
        }
        else
        {
            /*
             * Get the configuration page, modify it and then send it back out
             * with the appropriate bit settings for the LED control.
             */
            if ((NULL != (page2 = GetSESPageByWWN(SES_CMD_STATUS, pSES->WWN, pSES->LUN)))
                && ((mapNum = pSES->Map[SES_ET_DEVICE]) != SES_ET_INVALID))
            {
                /*
                 * Adjust for the default slot.
                 */
                slot += mapNum + 1;

                /*
                 * Clear the system status byte to make sure we do not latch up an
                 * old value.
                 */
                page2->Status = 0;

                pCtrl = &(page2->Control[slot]);

                /*
                 * Set the control to the value passed in.
                 */
                pCtrl->Ctrl.Generic.Ctrl2 &= ~(SES_C2DEV_RBYPASSA | SES_C2DEV_RBYPASSB);
                pCtrl->Ctrl.Generic.Ctrl2 |= setting;

                /*
                 * Turn on the select.
                 */
                pCtrl->CommonCtrl |= SES_CC_SELECT;

                /*
                 * Now write the page back out.
                 */
                SendSESPageByWWN(pSES->WWN, pSES->LUN, page2);

                Free(page2);

            }
        }
    }
}


#if defined(MODEL_3000) || defined(MODEL_7400)

/*----------------------------------------------------------------------------
** Function Name: SESGangLEDCtrl()
**
** Comments:
**  This function will send a diag page to an SES device in order to modify
**  the LED state for all devices contained within the SES device.
**
**--------------------------------------------------------------------------*/
static void SESGangLEDCtrl(SES_DEVICE *pSES)
{
    UINT16      wwnLoc;
    UINT16      slot;
    PSES_PAGE_02 page2;
    PSES_ELEM_CTRL pCtrl;
    UINT8       newValue;
    MRGETPINFO_REQ *ptrInPkt = NULL;
    MRGETPINFO_RSP *ptrOutPkt = NULL;
    UINT16      rc;

    /*
     * First get the control page.  If it cannot be fetched, then there
     * is no reason to continue.
     */
    if ((pSES->devStat == PD_OP) &&
        (NULL != (page2 = GetSESPageByWWN(SES_CMD_STATUS, pSES->WWN, pSES->LUN))))
    {

        /*
         * Find the device in the WWN to SES map.  If it does not exist, it
         * means that we cannot control the device.
         */
        for (wwnLoc = 0; wwnLoc < SES_WWN_MAP_SIZE; wwnLoc++)
        {
            if (WWNMap[wwnLoc].SES == pSES)
            {
                /*
                 * We found a device in this enclosure.  Record the slot
                 * number, fetch the fled value and set it.
                 */
                slot = WWNMap[wwnLoc].Slot + 1;
                pCtrl = &(page2->Control[slot]);

                /*
                 * Turn off all control bits for the LED.
                 */
                pCtrl->Ctrl.Generic.Ctrl1 &= ~SES_C1DEV_IDENT;
                pCtrl->Ctrl.Generic.Ctrl2 &= ~SES_C2DEV_RFAULT;

                /*
                 * Check for the bay type.  In a Xyratex bay, we have to
                 * make sure all bypass bits are turned off also.
                 */
                if (pSES->devType == PD_DT_SBOD_SES)
                {
                    pCtrl->Ctrl.Generic.Ctrl2 &= ~(SES_C2DEV_RBYPASSA | SES_C2DEV_RBYPASSB);
                }

                /*
                 * Allocate memory for MRP input and output packets.
                 */
                ptrInPkt = MallocWC(sizeof(*ptrInPkt));
                ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

                /*
                 * Get input parm from the WWN map.
                 */
                ptrInPkt->id = WWNMap[wwnLoc].PID;

                /*
                 * Send the request to Thunderbolt.  This function handles timeout
                 * conditions and task switches while waiting.
                 */
                rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRGETPINFO,
                                ptrOutPkt, sizeof(*ptrOutPkt), MRP_STD_TIMEOUT);

                /*
                 * At this point, we should have received the return packet
                 * of gotten a timeout.  Check for timeout and if not a timeout
                 * then grab the value, adjust the page and free the packet.
                 */
                if (rc != PI_TIMEOUT)
                {
                    newValue = ptrOutPkt->pdd.failedLED;

                    Free(ptrInPkt);
                    Free(ptrOutPkt);

                    /*
                     * Turn on the appropriate bit (if any).
                     */
                    if (newValue == SES_LED_ID)
                    {
                        pCtrl->Ctrl.Generic.Ctrl1 |= SES_C1DEV_IDENT;
                    }

                    if (newValue == SES_LED_FAIL)
                    {
                        pCtrl->Ctrl.Generic.Ctrl2 |= SES_C2DEV_RFAULT;
                    }

                    /*
                     * Turn on the select.
                     */
                    pCtrl->CommonCtrl |= SES_CC_SELECT;
                }
            }
        }

        /*
         * Clear the status byte to prevent latching an error.
         */
        page2->Status = 0;

        /*
         * Now write the page back out.
         */
        SendSESPageByWWN(pSES->WWN, pSES->LUN, page2);

        Free(page2);

    }
}
#endif /* MODEL_3000 || MODEL_7400 */


/*----------------------------------------------------------------------------
** Name:    enqueueEventChangeLED
**
** Desc:    This method will enqueue the Change LED asyncronous
**          event and fork off a task to process the event.
**
** Inputs:  UINT32 dummy - placeholder required for forked task
**
**--------------------------------------------------------------------------*/
static void enqueueEventChangeLED(LOG_MRP *pEvent)
{
    dprintf(DPRINTF_SES_LED_CONTROL, "enqueueEventChangeLED - Enqueuing Event.\n");

    /*
     * If the change LED event list has not yet been created, create it.
     */
    if (ptrAsyncEventLED == NULL)
    {
        ptrAsyncEventLED = CreateList();
    }

    /*
     * Enqueue this change LED event on the event list so it will be
     * processed by the ProcessEventChangeLED task.
     */
    Enqueue(ptrAsyncEventLED, pEvent);

    /*
     * if the task that processes LED changes is not active, activate it.
     */
    if (!bTaskActiveLED)
    {
        dprintf(DPRINTF_SES_LED_CONTROL, "enqueueEventChangeLED - Starting ProcessEventChangeLED.\n");
        bTaskActiveLED = 1;
        TaskCreate(ProcessEventChangeLED, NULL);
    }
}

/*----------------------------------------------------------------------------
** Name:    compareChangeLED
**
** Desc:    This method will compare two change LED events and determine
**          if they are the same (just check pointer values, not content).
**
** Inputs:  void* e1 - First change LED event
**          void* e2 - Second change LED event
**
** Returns: 0 if the events are the same
**          1 if the events are different
**
**--------------------------------------------------------------------------*/
static int compareChangeLED(void *e1, void *e2)
{
    return ((e1 == e2) ? 0 : 1);
}

/*----------------------------------------------------------------------------
** Name:    removeEventAndIterate
**
** Desc:    This method remove the current event and iterate to the next
**          event in the list.  If the iterator has seen the last event and
**          there are still more items in the list, restart the iterator.
**
** Inputs:  LOG_MRP* pEvent - Event to be removed from the list.
**
** Returns: Next item in the list or first item if the iterator has been
**          restarted.
**
**--------------------------------------------------------------------------*/
static LOG_MRP *removeEventAndIterate(LOG_MRP *pEvent)
{
    LOG_MRP    *pRemove = pEvent;
    LOG_MRP    *pNextEvent = NULL;

    /*
     * Get the next item from the iterator, this may be NULL if the iterator
     * has finished with the last item in the list.
     */
    pNextEvent = (LOG_MRP *)Iterate(ptrAsyncEventLED);

    /*
     * Remove the elemente from the queue.  It uses the comparison function
     * above (address == address) to find it in the list.
     */
    pRemove = RemoveElement(ptrAsyncEventLED, pRemove, (void *)&compareChangeLED);
    Free(pRemove);

    /*
     * There are times when the iterator has completed what it knows
     * about as events but there are still items on the list.  If
     * that is the case we want to reset the iterator and get the
     * first item.
     */
    if (pNextEvent == NULL && NumberOfItems(ptrAsyncEventLED) > 0)
    {
        SetIterator(ptrAsyncEventLED);
        pNextEvent = (LOG_MRP *)Iterate(ptrAsyncEventLED);
    }

    return pNextEvent;
}

/*----------------------------------------------------------------------------
** Name:    ProcessEventChangeLED
**
** Desc:    This method will handle the Change LED asyncronous
**          event.
**
** Inputs:  UINT32 dummy - placeholder required for forked task
**
**--------------------------------------------------------------------------*/
static void ProcessEventChangeLED(UNUSED TASK_PARMS *parms)
{
    LOG_MRP    *logMRP = NULL;
    SES_DEVICE *pCurrentDevice = NULL;
    PSES_PAGE_02 pCurrentPage = NULL;
    PSES_ELEM_CTRL pCtrl;
    UINT64      srchWWN;
    UINT16      wwnLoc;
    UINT16      slot = 0;
    UINT16      mapNum = 0;
    UINT8       newValue;
    UINT16      i;

    dprintf(DPRINTF_SES_LED_CONTROL, "ProcessEventChangeLED - ENTER.\n");

    ccb_assert(ptrAsyncEventLED != NULL, ptrAsyncEventLED);

    while (NumberOfItems(ptrAsyncEventLED) > 0)
    {
        dprintf(DPRINTF_SES_LED_CONTROL, "ProcessEventChangeLED - Items on queue: %d.\n",
                NumberOfItems(ptrAsyncEventLED));

        /*
         * Exchange once here to make sure all events that have been sent are
         * accounted for before starting the loop.
         */
        TaskSwitch();

        pCurrentDevice = NULL;

        /*
         * Setup the iterator for the event list.
         */
        SetIterator(ptrAsyncEventLED);

        /*
         * Get the first event in the list so we can process it.
         */
        logMRP = (LOG_MRP *)Iterate(ptrAsyncEventLED);

        /*
         * While we have a valid event we are going to loop
         */
        while (logMRP != NULL)
        {
            srchWWN = (UINT64)(*((UINT64 *)(logMRP->mleData)));

            for (wwnLoc = 0; wwnLoc < SES_WWN_MAP_SIZE; wwnLoc++)
            {
                if (WWNMap[wwnLoc].WWN == srchWWN)
                {
                    break;
                }
            }

            /*
             * If we did not find the WWN in our map we cannot process
             * the event.  Remove the event from our list and continue
             * with the next event.
             */
            if (wwnLoc == SES_WWN_MAP_SIZE)
            {
                dprintf(DPRINTF_DEFAULT, "ProcessEventChangeLED - Failed to find the event in our SES-WWN map.  Moving to the next item in the list.\n");

                logMRP = removeEventAndIterate(logMRP);

                /*
                 * Go back to the beginning of the WHILE loop iterating
                 * through the event list.
                 */
                continue;
            }

            /*
             * If we have not yet set our current device, do so now with the
             * device for the current event.
             */
            if (pCurrentDevice == NULL)
            {
                ccb_assert(WWNMap[wwnLoc].SES != NULL, WWNMap[wwnLoc].SES);
                ccb_assert(pCurrentPage == NULL, pCurrentPage);

                pCurrentDevice = WWNMap[wwnLoc].SES;

                /*
                 * If this event is trying to use an invalid device, remove
                 * the event from our list of items to process.
                 */
                mapNum = pCurrentDevice->Map[SES_ET_DEVICE];

                if ((pCurrentDevice->devStat != PD_OP) || (mapNum == SES_ET_INVALID))
                {
                    dprintf(DPRINTF_DEFAULT, "ProcessEventChangeLED - SES Device for this event is invalid.  Moving to the next item in the list.\n");

                    pCurrentDevice = NULL;
                    mapNum = 0;

                    logMRP = removeEventAndIterate(logMRP);

                    /*
                     * Go back to the beginning of the WHILE loop iterating
                     * through the event list.
                     */
                    continue;
                }

#if defined(MODEL_7000) || defined(MODEL_4700)
                pCurrentPage = NULL;
                pCurrentDevice = NULL;
                mapNum = 0;
                logMRP = removeEventAndIterate(logMRP);
                continue;
#else  /* MODEL_7000 || MODEL_4700 */
                pCurrentPage = GetSESPageByWWN(SES_CMD_STATUS,
                                               pCurrentDevice->WWN, pCurrentDevice->LUN);

                /*
                 * If for some reason we were not able to get the SES page
                 * we need to reset the current device and get the next
                 * event in the list.
                 */
                if (pCurrentPage == NULL)
                {

                    dprintf(DPRINTF_DEFAULT, "ProcessEventChangeLED - Failed to get the SES page for Bay WWN=%qx LUN=%d for this event.  Moving to the next item in the list.\n",
                            pCurrentDevice->WWN, WWNMap[wwnLoc].Slot);

                    pCurrentDevice = NULL;
                    mapNum = 0;

                    logMRP = removeEventAndIterate(logMRP);

                    /*
                     * Go back to the beginning of the WHILE loop iterating
                     * through the event list.
                     */
                    continue;
                }
#endif /* MODEL_7000 || MODEL_4700 */
            }

            /*
             * We are trying to batch up requests for the same device so if
             * this event is not for our current device, skip it for now.
             */
            if (pCurrentDevice != WWNMap[wwnLoc].SES)
            {
                logMRP = (LOG_MRP *)Iterate(ptrAsyncEventLED);

                /*
                 * Go back to the beginning of the WHILE loop iterating
                 * through the event list.
                 */
                continue;
            }

            /*
             * Get the slot number for this event
             */
            slot = WWNMap[wwnLoc].Slot;

            /*
             * Adjust for the default slot.
             */
            slot += mapNum + 1;

            pCtrl = &(pCurrentPage->Control[slot]);

            ccb_assert(pCtrl != NULL, pCtrl);

            /*
             * Turn off all control bits for the LED.
             */
            pCtrl->Ctrl.Generic.Ctrl1 &= ~SES_C1DEV_IDENT;
            pCtrl->Ctrl.Generic.Ctrl2 &= ~SES_C2DEV_RFAULT;

            /*
             * Check for the bay type.  In a Xyratex bay, we have to
             * make sure all bypass bits are turned off also.
             */
            if (pCurrentDevice->devType == PD_DT_SBOD_SES)
            {
                pCtrl->Ctrl.Generic.Ctrl2 &= ~(SES_C2DEV_RBYPASSA | SES_C2DEV_RBYPASSB);
            }

            newValue = logMRP->mleData[2] & 0xFF;

            /*
             * Turn on the appropriate bit (if any).
             */
            if (newValue & SES_LED_ID)
            {
                pCtrl->Ctrl.Generic.Ctrl1 |= SES_C1DEV_IDENT;
            }

            if (newValue & SES_LED_FAIL)
            {
                pCtrl->Ctrl.Generic.Ctrl2 |= SES_C2DEV_RFAULT;
            }

            /*
             * Turn on the select.
             */
            pCtrl->CommonCtrl |= SES_CC_SELECT;

            /*
             * We have finished processing this event, move to the next one.
             */
            logMRP = removeEventAndIterate(logMRP);
        }

        /*
         * Now write the page back out.
         */
        if (pCurrentPage != NULL)
        {
            dprintf(DPRINTF_SES_LED_CONTROL, "ProcessEventChangeLED - Sending SES Page\n");

            /*
             * Clear the status byte to prevent latching an error.
             */
            pCurrentPage->Status = 0;

            SendSESPageByWWN(pCurrentDevice->WWN, pCurrentDevice->LUN, pCurrentPage);
            Free(pCurrentPage);
        }

        /*
         * If there are no more items on the list, wait 10 times for 1
         * second(s) in case more items are added to the list.  This is
         * done to hopefully reduce the number of times the task is
         * created and destroyed.
         */
        i = 0;
        while (NumberOfItems(ptrAsyncEventLED) == 0 && i < 10)
        {
            TaskSleepMS(1000);
            i++;
        }

        /*
         * Go back to the start of the WHILE loop that checks if there are
         * more items in the list to be processed.
         */
    }

    /*
     * There are no more items to be processed at this time, shutdown the
     * task (flag it as inactive).
     */
    bTaskActiveLED = 0;
}

/*----------------------------------------------------------------------------
** Function:    GetDeviceMap
**
** Description: Get a map of the PDisks in the disk bay referenced
**              by the input pointer
**
** Inputs:      SES_DEVICE *pSES - pointer to the disk bay struct that
**                                 containing the PDisks
**              SES_WWN_TO_SES **pdevMap - used to return a pointer to the
**                                         map of PDisks for this disk bay.
**
** Returns:     UINT32 - length in bytes of the device map.
**
** WARNING:     MEMORY ALLOCATED IN THIS FUNCTION MUST BE FREED BY THE CALLER.
**
**--------------------------------------------------------------------------*/
UINT32 GetDeviceMap(SES_DEVICE *pSES, SES_WWN_TO_SES **pDevMap)
{
    UINT32      i;
    UINT32      mapSize = 0;
    SES_WWN_TO_SES *pMap = NULL;
    SES_WWN_TO_SES *pCurrentEntry = NULL;

    /*
     * Walk the WWNMap once to determine how many entries match the input
     * criteria.
     */
    for (i = 0; i < SES_WWN_MAP_SIZE; i++)
    {
        if (WWNMap[i].SES == pSES)
        {
            mapSize += sizeof(*pMap);
        }
    }

    /*
     * Allocate memory based on the number of matching entries found.
     * Walk the WWNMap again and copy the appropriate entries into the
     * return buffer.
     */
    pMap = MallocWC(mapSize);

    if (pMap != NULL)
    {
        pCurrentEntry = pMap;

        for (i = 0; i < SES_WWN_MAP_SIZE; i++)
        {
            if (WWNMap[i].SES == pSES)
            {
                memcpy(pCurrentEntry, &WWNMap[i], sizeof(*pCurrentEntry));
                pCurrentEntry++;
            }
        }
    }
    else
    {
        /*
         * If a memory allocation error occurred change mapSize to
         * 0 to reflect that no data is being returned.
         */
        mapSize = 0;
    }

    /*
     * Return the pointer to the dvice map to the caller.
     */
    *pDevMap = pMap;

    return (mapSize);
}

/*----------------------------------------------------------------------------
** Function:    GetSESControlElement
**
** Description: Get the SES Control Element specified by the input parms
**              by the input pointer
**
** Inputs:      pid         - ID for the SES device
**              type        - SES element type
**              index       - which element of the type above to get.
**                            index=0 is the overall control element.
**              pElement    - pointer to buffer for element data.  This
**                            buffer is allocated by the caller.
**
** Returns:     INT32       - GOOD or ERROR
**
**--------------------------------------------------------------------------*/
INT32 GetSESControlElement(UINT16 pid, UINT8 type, UINT8 index1, SES_ELEM_CTRL *pElement)
{
    SES_DEVICE *pSES;
    INT32       rc = ERROR;
    UINT8       firstElementOffset = 0;
    UINT8       numElementsThisType = 0;

    /*
     * Get a pointer to the list of fibre bays (SES enclosures).
     */
    pSES = GetSESList();

    /*
     * Search the list for the requested SES Device data.
     */
    while (pSES != NULL)
    {
        /* dprintf(DPRINTF_DEFAULT, "GetSESControlElement - pSES->pid=%d\n", pSES->PID); */

        /* Proceed if we have the correct bay info. */
        if (pSES->PID == pid)
        {
            /*
             * We found the info for the requested SES device.
             * The SES Map and Slot arrays hold the number and
             * type of each element.  The index into Map[] is the element
             * type.  The value of Map[] (at this index) is the offset of
             * the first element of that type in Page2 Control Element data.
             * Slot[] gives the number of elements of the indexed type NOT
             * including the Overall Control Element.
             *
             * Yes this is all confusing.  Consult
             * SCSI Enclosure Services docs for more information.
             *
             * Get the offset of the first element of the requested type
             * and the number of elements of this type.
             */
            firstElementOffset = pSES->Map[type];
            numElementsThisType = pSES->Slots[type] + 1;

#if 0
            dprintf(DPRINTF_DEFAULT, "  firstElementOffset=0x%02hhX  numElementsThisType=%d\n",
                    firstElementOffset, numElementsThisType);
#endif /* 0 */

            /*
             * Check that the firstElementOffset is valid and the
             * requested element index is in range.
             */
            if ((firstElementOffset != SES_ET_INVALID) &&
                (index1 < numElementsThisType) && (pSES->OldPage2 != NULL))
            {
                /*
                 * Copy the requested element into the output buffer.
                 */
                memcpy(pElement, &pSES->OldPage2->Control[firstElementOffset + index1],
                       sizeof(*pElement));

#if 0
                dprintf(DPRINTF_DEFAULT, "  Control Element: 0x%02hhX  0x%02hhX  0x%02hhX  0x%02hhX\n",
                        pSES->OldPage2->Control[firstElementOffset + index1].CommonCtrl,
                        pSES->OldPage2->Control[firstElementOffset + index1].Ctrl.Generic.Slot,
                        pSES->OldPage2->Control[firstElementOffset + index1].Ctrl.Generic.Ctrl1,
                        pSES->OldPage2->Control[firstElementOffset + index1].Ctrl.Generic.Ctrl2);
#endif /* 0 */

                /*
                 * Return GOOD status.
                 */
                rc = GOOD;
            }

            /*
             * We found the device we were looking for so we're done.
             */
            break;
        }
        else
        {
            pSES = pSES->NextSES;
        }
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    InitLedControlMap
**
** Description: Initialize the LED control Map, which is used to coordinate
**              the setting of LEDs on physical drives shared between
**              controllers.
**
**--------------------------------------------------------------------------*/
static void InitLedControlMap(UNUSED TASK_PARMS *parms)
{
    PI_PDISKS_RSP *pPDisks = NULL;
    UINT32      i = 0;
    UINT16      highPID = 0;

    dprintf(DPRINTF_SES_LED_CONTROL, "InitLedControlMap - Initializing led control map.\n");
    ledControlMapState = LED_MAP_INPROGRESS;

    pPDisks = PhysicalDisks();

    /*
     * Check the state to make sure nothing changed while we were
     * away gathering the Physical disk information. If it did, get out
     * and let the table be initialized later.
     */
    if (ledControlMapState != LED_MAP_INPROGRESS)
    {
        /*
         * Do nothing
         */
    }
    else
    {
        /*
         * Clear the ledControlMap.
         */
        memset(ledControlMap, 0x00, (MAX_PHYSICAL_DISKS * (MAX_CONTROLLERS + 1)));

        /*
         * Loop through the physical disks and set the current state for each
         * controller,
         */
        if (pPDisks != NULL)
        {
            /*
             * Load the state of each device into the table
             */
            for (i = 0; i < pPDisks->count; i++)
            {
                /*
                 * Set our controller state of each device into the table.
                 */
                ledControlMap[pPDisks->pdiskInfo[i].pdd.pid][GetCommunicationsSlot(GetMyControllerSN())] =
                    pPDisks->pdiskInfo[i].pdd.failedLED;

                /*
                 * Set the current state of each device into the table.
                 */
                ledControlMap[pPDisks->pdiskInfo[i].pdd.pid][PRESENT_LED_STATE] =
                    pPDisks->pdiskInfo[i].pdd.failedLED;

                /*
                 * Store the highest pid we are saving.
                 */
                highPID = MAX(pPDisks->pdiskInfo[i].pdd.pid, highPID);
            }

            /*
             * If we do not have MAX_PHYSICAL_DISKS, place a marker
             * in the Map so we do not have to process the entire
             * map looking for led states.
             */
            if (highPID != MAX_PHYSICAL_DISKS)
            {
                ledControlMap[highPID + 1][PRESENT_LED_STATE] = LED_MAP_END;
            }
        }

        /*
         * Signal that the Led Control Map is now initialized.
         */
        ledControlMapState = LED_MAP_INITIALIZED;
    }

    /*
     * Free memory allocated in getting the pdisk info.
     */
    Free(pPDisks);
}

/*----------------------------------------------------------------------------
** Function:    ProcessLedControl
**
** Description: Handle queue of Led change requests and forward to the master
**              controller.
**
** NOTE:        Runs on both master and slave controllers
**--------------------------------------------------------------------------*/
static void ProcessLedControl(UNUSED TASK_PARMS *parms)
{
    IPC_LED_CHANGE *pLedControl = NULL;
    UINT8       i = 0;

    while (NumberOfItems(pLedControlQueue) > 0)
    {
        dprintf(DPRINTF_SES_LED_CONTROL, "SESLedControl - Items on Queue: 0x%x\n",
                NumberOfItems(pLedControlQueue));


        /*
         * If we are a Master controller, process the LED state change
         * directly. Otherwise send it to the master via IPC.
         */
        if (TestforMaster(GetMyControllerSN()))
        {
            /*
             * If the LED control map has not been initialized, don't
             * dequeue anything yet. We will wait until it is ready.
             */

            if (ledControlMapState != LED_MAP_INITIALIZED &&
                ledControlMapState != LED_MAP_INPROGRESS)
            {
                /*
                 * Fork off the task to initialize the led contol map
                 */
                TaskCreate(InitLedControlMap, NULL);
                TaskSwitch();
            }

            while (ledControlMapState == LED_MAP_INPROGRESS)
            {
                TaskSleepMS(1000);
            }

            if (ledControlMapState == LED_MAP_INITIALIZED)
            {
                /*
                 * Get the first event in the list so we can process it.
                 */
                pLedControl = (IPC_LED_CHANGE *)Dequeue(pLedControlQueue);
                LedStateChange(pLedControl);
                Free(pLedControl);

            }
        }
        else
        {
            /*
             * Get the first event in the list so we can process it.
             */
            pLedControl = (IPC_LED_CHANGE *)Dequeue(pLedControlQueue);
            if (pLedControl)
            {
                SendLedControlToMaster(pLedControl);
                Free(pLedControl);

            }
        }

        /*
         * If there are no more items on the list, wait 10 times for 1
         * second(s) in case more items are added to the list.  This is
         * done to hopefully reduce the number of times the task is
         * created and destroyed.
         */
        i = 0;
        while (NumberOfItems(pLedControlQueue) == 0 && i < 10)
        {
            TaskSleepMS(1000);
            i++;
        }

        /*
         * Go back to the start of the WHILE loop that checks if there are
         * more items in the list to be processed.
         */
    }

    /*
     * The task has finished, set the flag, so we will restart
     */
    bLedControlTaskActive = 0;
}

/*----------------------------------------------------------------------------
** Function Name: EnqueueToLedControlQueue()
**
** Comments: Places LED control event on a queue to be processed in the
**           order they were received.
**
** Inputs:  IPC_LED_CHANGE* pEvent - Event to be placed on the queue.
**
**--------------------------------------------------------------------------*/
void EnqueueToLedControlQueue(IPC_LED_CHANGE *pEvent)
{
    dprintf(DPRINTF_SES_LED_CONTROL, "EnqueueToLedControlQueue - Enqueuing Event.\n");

    /*
     * If the LED Change event list has not yet been created, create it.
     */
    if (pLedControlQueue == NULL)
    {
        pLedControlQueue = CreateList();
    }

    /*
     * Enqueue this led change on the list so it will be
     * processed by the ProcessLedControl task.
     */
    Enqueue(pLedControlQueue, pEvent);

    /*
     * if the task that processes led changes from the queue is not
     * active, activate it.
     */
    if (!bLedControlTaskActive)
    {
        dprintf(DPRINTF_SES_LED_CONTROL, "EnqueueToLedControlQueue - Starting ProcessLedControl.\n");
        bLedControlTaskActive = 1;
        TaskCreate(ProcessLedControl, NULL);
    }
}


/*----------------------------------------------------------------------------
** Function Name: SendLedControlToMaster()
**
** Comments: Send LED change event to the Master Controller via IPC.
**
** Inputs:  IPC_LED_CHANGE* pEvent - Event to be sent.
**
**--------------------------------------------------------------------------*/
static void SendLedControlToMaster(IPC_LED_CHANGE *pEvent)
{
    IPC_PACKET *rx;
    IPC_PACKET *ptrPacket;
    PATH_TYPE   pathType;
    UINT8       retries = 2;                /* Ethernet, Fiber(1), Disk Quorum(2) */

    /*
     * Allocate the response packet, setting the header and data
     * pointers to null.
     */
    rx = MallocWC(sizeof(IPC_PACKET));

    /* Create the transmit packet and send it. */
    ptrPacket = CreatePacket(PACKET_IPC_LED_CHANGE, sizeof(*pEvent), __FILE__, __LINE__);

    /* Fill in the data packet */
    memcpy(ptrPacket->data, pEvent, sizeof(*pEvent));

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call IpcSendPacketBySN with rxPacket of %p\n", __FILE__, __LINE__, __func__, rx);
#endif  /* HISTORY_KEEP */

    do
    {
        Free(rx->data);

        /* Sending packet to the other controller using any IPC path possible */
        pathType = IpcSendPacketBySN(Qm_GetMasterControllerSN(), SENDPACKET_ANY_PATH,
                                     ptrPacket, rx, NULL, NULL, NULL, LEDCONTROL_PACKET_TIMEOUT);
    } while (pathType == SENDPACKET_NO_PATH && (retries--) > 0);

//    if (!IpcSuccessfulXfer(pathType))
//    {
//        /* error */
//    }
//    else
//    {
//        /* Worked */
//    }

    FreePacket(&ptrPacket, __FILE__, __LINE__);
    FreePacket(&rx, __FILE__, __LINE__);
}

/*----------------------------------------------------------------------------
** Function Name: HandleLedControlRequest()
**
** Inputs:  IPC_PACKET *ptrPacket -  IPC led control packet.
**
**--------------------------------------------------------------------------*/
IPC_PACKET *HandleLedControlRequest(IPC_PACKET *ptrPacket)
{
    dprintf(DPRINTF_SES_LED_CONTROL, "HandleLedControlRequest - Received IPC LED State Change.\n");

    /* Queue the LED change request for processing.  */
    EnqueueToLedControlQueue((IPC_LED_CHANGE *)ptrPacket->data);

    /*
     * Set the length to 0 and data to NULL to prevent the data packet
     * from being freed by IPC.  This memory will be freed later by
     * ProcessLedControl.
     */
    ptrPacket->header->length = 0;
    ptrPacket->data = NULL;

    return (ptrPacket);
}

/*----------------------------------------------------------------------------
** Function Name: LedStateChange()
**
** Comments: Add the Led change request to the LED control table. Compare the
**           requested state to the current state. If a state change is required,
**           return a TRUE indication
**
** Inputs:  LOG_MRP* pEvent - Event to be placed on the queue.
**          controllerSN    - Controller SN requesting LED change
**
**--------------------------------------------------------------------------*/
static void LedStateChange(IPC_LED_CHANGE *pEvent)
{
    UINT32      controllerSN = pEvent->serialNum;
    LOG_MRP    *event = NULL;
    UINT16      cntlIndex;
    UINT16      pidIndex;
    UINT8       newState = 0;
    UINT8       i;

    dprintf(DPRINTF_SES_LED_CONTROL, "LedStateChange - ENTER.\n");

    /*
     * Determine controller location in Led Controller table, based on the
     * controller map.
     */
    cntlIndex = GetCommunicationsSlot(controllerSN);

    /*
     * Ensure the index is valid. If not, don't change anything.
     */
    if (cntlIndex < MAX_CONTROLLERS)
    {
        /*
         * Convert Drive WWN to a PID index for the Led Control table
         */
        pidIndex = PhysicalDisk_PIDFromWWN(pEvent->wwn, 0);

        dprintf(DPRINTF_SES_LED_CONTROL, "LedStateChange - cntlIndex: %d, pidIndex: %d.\n",
                cntlIndex, pidIndex);

        /*
         * Ensure the index is valid. If not, don't change anything.
         */
        if (pidIndex < MAX_PHYSICAL_DISKS)
        {
            /*
             * Determine the new led state for this controller, based on the
             * requested change.
             */
            switch (pEvent->ledReq)
            {
                case LED_BOTH_OFF:
                    ledControlMap[pidIndex][cntlIndex] &= ~(SES_LED_FAIL + SES_LED_ID);
                    break;

                case LED_FAIL_ON:
                    ledControlMap[pidIndex][cntlIndex] |= SES_LED_FAIL;
                    break;

                case LED_FAIL_OFF:
                    ledControlMap[pidIndex][cntlIndex] &= ~(SES_LED_FAIL);
                    break;

                case LED_ID_ON:
                    ledControlMap[pidIndex][cntlIndex] |= SES_LED_ID;
                    break;

                case LED_ID_OFF:
                    ledControlMap[pidIndex][cntlIndex] &= ~(SES_LED_ID);
                    break;

                default:
                    break;
            }

            /*
             * Walk through the LED state for this PID, for all
             * controllers. Save the highest state. If this state is
             * not equal to the current LED state, save the current state
             * and pass the change onto the SES controller.
             */
            for (i = 0; i < MAX_CONTROLLERS; ++i)
            {
                if (newState < ledControlMap[pidIndex][i])
                {
                    newState = ledControlMap[pidIndex][i];
                }
            }

            dprintf(DPRINTF_SES_LED_CONTROL, "LedStateChange - State %s Change...newState %d\n",
                    (newState != ledControlMap[pidIndex][PRESENT_LED_STATE]) ? "DID" : "DID NOT", newState);
            /*
             * If a state change is necessary, save the new state and
             * send it to the SES controller.
             */
            if (newState != ledControlMap[pidIndex][PRESENT_LED_STATE])
            {
                ledControlMap[pidIndex][PRESENT_LED_STATE] = newState;

                /*
                 * The change LED event is slow due to SES calls being
                 * slow so we are going to pull this event off the queue
                 * and process it using a separate task.  To do that we
                 * need to make a copy of the event data since the original
                 * data is owned by the PROC and if we return before
                 * completing our work the memory would have been freed on
                 * us.  So, make a copy of the data and put it on the
                 * queue of events to be handled by the change LED task
                 * (the change LED task is coded in SES.C).
                 */
                event = MallocWC(sizeof(*event));

                if (event != NULL)
                {
                    event->mleEvent = LOG_CHANGE_LED;
                    event->mleLength = 0x0C;
                    (*((UINT64 *)(event->mleData))) = pEvent->wwn;
                    event->mleData[2] = newState;
                    enqueueEventChangeLED(event);
                }
            }
        }
    }

}

/*----------------------------------------------------------------------------
** Function Name: UpdateLeds()
**
** Comments: Updates the LED's to what we have in store for them
**
** Inputs:  NONE
**
** Returns: NONE
**
** WARNING: Only run on Master, (Function will check and do nothing if
**          we are not the master.
**--------------------------------------------------------------------------*/
static void UpdateLeds(void)
{
    LOG_MRP    *event = NULL;
    UINT16      i = 0;

    dprintf(DPRINTF_SES_LED_CONTROL, "UpdateLeds - ENTER.\n");

    if (TestforMaster(GetMyControllerSN()))
    {
        /*
         * Get the most recent values of the pdisks
         */
        InitLedControlMap(NULL);        /* dummy value */

#ifdef HISTORY_KEEP
        CT_HISTORY_OFF();               // Turn history mechanism off to speed this up.
#endif  /* HISTORY_KEEP */

        /* Loop through the WWNMap and set the LED state of the drives we find */
        for (i = 0; i < MAX_PHYSICAL_DISKS; i++)
        {
            if (ledControlMap[i][PRESENT_LED_STATE] == LED_MAP_END)
            {
                dprintf(DPRINTF_SES_LED_CONTROL, "UpdateLeds - End of list.\n");
                break;
            }

            if ((WWNMap[i].SES == NULL) || (WWNMap[i].WWN == 0))
            {
                dprintf(DPRINTF_SES_LED_CONTROL, "UpdateLeds - Invalid..PID: %d, WWN: %8.8x%8.8x, Slot: %d, Led: %d\n",
                        WWNMap[i].PID, (UINT32)WWNMap[i].WWN,
                        (UINT32)(WWNMap[i].WWN >> 32), WWNMap[i].Slot,
                        ledControlMap[WWNMap[i].PID][PRESENT_LED_STATE]);
                continue;
            }

            dprintf(DPRINTF_SES_LED_CONTROL, "UpdateLeds - Sending..PID: %d, WWN: %8.8x%8.8x, Slot: %d, Led: %d\n",
                    WWNMap[i].PID, (UINT32)WWNMap[i].WWN, (UINT32)(WWNMap[i].WWN >> 32),
                    WWNMap[i].Slot, ledControlMap[WWNMap[i].PID][PRESENT_LED_STATE]);


            /*
             * Create an event and place it on the queue to be processed.
             * We do not need to worry about freeing the memory, as it
             * will be destroyed by the process we are handing it over to.
             */
            event = MallocWC(sizeof(*event));

            event->mleEvent = LOG_CHANGE_LED;
            event->mleLength = 0x0C;
            (*((UINT64 *)(event->mleData))) = WWNMap[i].WWN;
            event->mleData[2] = ledControlMap[WWNMap[i].PID][PRESENT_LED_STATE];
            enqueueEventChangeLED(event);

            /*
             * Set event to NULL
             */
            event = NULL;
        }

#ifdef HISTORY_KEEP
        CT_HISTORY_ON();                // Turn history mechanism back.
#endif  /* HISTORY_KEEP */

    }
}

/* ------------------------------------------------------------------------ */

#if 0
static void print_ise_info_version_2(struct ise_info_version_2 *ise)
{
    int         i;

    dprintf(DPRINTF_DEFAULT, "CHASSIS_DETAILS\n ");
    dprintf(DPRINTF_DEFAULT, " Protocol_Version_Level = %d\n ",
            ise->Protocol_Version_Level);
    dprintf(DPRINTF_DEFAULT, " which_tcp_connections = %d\n ",
            ise->which_tcp_connections);
    dprintf(DPRINTF_DEFAULT, " which_fw_index_type = %d\n", ise->which_fw_index_type);
    dprintf(DPRINTF_DEFAULT, " which_controllers[0] =   %d which_controllers[1] =   %d\n",
            ise->which_controllers[0], ise->which_controllers[1]);
    dprintf(DPRINTF_DEFAULT, " which_datapacs[0] =      %d which_datapacs[1] =      %d\n",
            ise->which_datapacs[0], ise->which_datapacs[1]);
    dprintf(DPRINTF_DEFAULT, " which_powersupplies[0] = %d which_powersupplies[1] = %d\n",
            ise->which_powersupplies[0], ise->which_powersupplies[1]);
    dprintf(DPRINTF_DEFAULT, " which_batteries[0] =     %d which_batteries[1] =     %d\n",
            ise->which_batteries[0], ise->which_batteries[1]);
    dprintf(DPRINTF_DEFAULT, " ip1 = %08x  ip2 = %08x\n", ise->ip1, ise->ip2);
    dprintf(DPRINTF_DEFAULT, " iws_ise_id = %016llx\n", ise->iws_ise_id);
    dprintf(DPRINTF_DEFAULT, " chassis_wwn = %016llx\n", ise->chassis_wwn);
    dprintf(DPRINTF_DEFAULT, " chassis_serial_number = %.12s\n",
            ise->chassis_serial_number);
    dprintf(DPRINTF_DEFAULT, " chassis_model = %.16s\n", ise->chassis_model);
    dprintf(DPRINTF_DEFAULT, " chassis_part_number = %.16s\n", ise->chassis_part_number);
    dprintf(DPRINTF_DEFAULT, " chassis_vendor = %.8s\n", ise->chassis_vendor);
    dprintf(DPRINTF_DEFAULT, " chassis_manufacturer = %.8s\n", ise->chassis_manufacturer);
    dprintf(DPRINTF_DEFAULT, " chassis_product_version = %.4s\n",
            ise->chassis_product_version);
    dprintf(DPRINTF_DEFAULT, " spare_level = %llu\n", ise->spare_level);
    dprintf(DPRINTF_DEFAULT, " chassis_beacon = %d\n", ise->chassis_beacon);
    dprintf(DPRINTF_DEFAULT, " chassis_status = 0x%08x chassis_status_details= 0x%016llx\n",
            ise->chassis_status, ise->chassis_status_details);
    dprintf(DPRINTF_DEFAULT, " chassis_uptime = %u\n", ise->chassis_uptime);
    dprintf(DPRINTF_DEFAULT, " chassis_current_date_time = %u\n",
            ise->chassis_current_date_time);
    dprintf(DPRINTF_DEFAULT, " chassis_performance_valid = %d\n",
            ise->chassis_performance_valid);
    dprintf(DPRINTF_DEFAULT, " chassis_total_iops = %lld\n", ise->chassis_total_iops);
    dprintf(DPRINTF_DEFAULT, " chassis_read_iops = %lld\n", ise->chassis_read_iops);
    dprintf(DPRINTF_DEFAULT, " chassis_write_iops =%lld\n", ise->chassis_write_iops);
    dprintf(DPRINTF_DEFAULT, " chassis_total_kbps =%lld\n", ise->chassis_total_kbps);
    dprintf(DPRINTF_DEFAULT, " chassis_read_kbps = %lld\n", ise->chassis_read_kbps);
    dprintf(DPRINTF_DEFAULT, " chassis_write_kbps = %lld\n", ise->chassis_write_kbps);
    dprintf(DPRINTF_DEFAULT, " chassis_read_latency = %lld\n",
            ise->chassis_read_latency);
    dprintf(DPRINTF_DEFAULT, " chassis_write_latency = %lld\n",
            ise->chassis_write_latency);
    dprintf(DPRINTF_DEFAULT, " chassis_queue_depth = %lld\n", ise->chassis_queue_depth);
    dprintf(DPRINTF_DEFAULT, " chassis_read_percent = %lld\n", ise->chassis_read_percent);
    dprintf(DPRINTF_DEFAULT, " chassis_avg_bytes_transferred = %lld\n",
            ise->chassis_avg_bytes_transferred);
    dprintf(DPRINTF_DEFAULT, " chassis_temperature_sensor = %d\n",
            ise->chassis_temperature_sensor);

    for (i = 0; i < 2; i++)
    {
        dprintf(DPRINTF_DEFAULT, " ------------ISE MRC %d DETAILS------------\n", i);
        dprintf(DPRINTF_DEFAULT, " controller_model[16] = %.16s\n",
                ise->ctrlr[i].controller_model);
        dprintf(DPRINTF_DEFAULT, " controller_serial_number = %.12s\n",
                ise->ctrlr[i].controller_serial_number);
        dprintf(DPRINTF_DEFAULT, " controller_part_number[16] = %.16s\n",
                ise->ctrlr[i].controller_part_number);
        dprintf(DPRINTF_DEFAULT, " controller_hw_version[4]=%.4s\n",
                ise->ctrlr[i].controller_hw_version);
        dprintf(DPRINTF_DEFAULT, " controller_x_position = %lld\n",
                ise->ctrlr[i].controller_x_position);
        dprintf(DPRINTF_DEFAULT, " controller_y_position = %lld\n ",
                ise->ctrlr[i].controller_y_position);
        dprintf(DPRINTF_DEFAULT, " controller_z_position = %lld\n ",
                ise->ctrlr[i].controller_z_position);
        dprintf(DPRINTF_DEFAULT, " controller_wwn = %llx\n",
                ise->ctrlr[i].controller_wwn);
        dprintf(DPRINTF_DEFAULT, " ip= %x\n", ise->ctrlr[i].ip);
        dprintf(DPRINTF_DEFAULT, " gateway= %x\n", ise->ctrlr[i].gateway);
        dprintf(DPRINTF_DEFAULT, " subnet_mask = %x\n", ise->ctrlr[i].subnet_mask);
        dprintf(DPRINTF_DEFAULT, " controller_fc_port_speed_setting = %d\n ",
                ise->ctrlr[i].controller_fc_port_speed_setting);
        dprintf(DPRINTF_DEFAULT, " controller_beacon = %d\n",
                ise->ctrlr[i].controller_beacon);
        dprintf(DPRINTF_DEFAULT, " controller_rank = %d\n ",
                ise->ctrlr[i].controller_rank);
        dprintf(DPRINTF_DEFAULT, " %d controller_status = 0x%08x controller_status_details = 0x%016llx\n",
                i, ise->ctrlr[i].controller_status,
                ise->ctrlr[i].controller_status_details);
        dprintf(DPRINTF_DEFAULT, " controller_fw_version = %.28s\n",
                ise->ctrlr[i].controller_fw_version);
        dprintf(DPRINTF_DEFAULT, " controller_fc_port_status = %d\n",
                ise->ctrlr[i].controller_fc_port_status);
        dprintf(DPRINTF_DEFAULT, " controller_fc_port_speed = %d\n",
                ise->ctrlr[i].controller_fc_port_speed);
        dprintf(DPRINTF_DEFAULT, " controller_ethernet_link_up = %d\n",
                ise->ctrlr[i].controller_ethernet_link_up);
        dprintf(DPRINTF_DEFAULT, " controller_mac_address = %.18s\n",
                ise->ctrlr[i].controller_mac_address);
        dprintf(DPRINTF_DEFAULT, " controller_temperature = %d\n",
                ise->ctrlr[i].controller_temperature);
    }

    for (i = 0; i < 2; i++)
    {
        dprintf(DPRINTF_DEFAULT, " ----------ISE BATTERY %d DETAILS---------\n ", i);
        dprintf(DPRINTF_DEFAULT, " battery_beacon = %d\n ",
                ise->battery[i].battery_beacon);
        dprintf(DPRINTF_DEFAULT, " battery_model = %.16s\n ",
                ise->battery[i].battery_model);
        dprintf(DPRINTF_DEFAULT, " battery_serial_number = %.12s\n ",
                ise->battery[i].battery_serial_number);
        dprintf(DPRINTF_DEFAULT, " battery_part_number = %.16s\n",
                ise->battery[i].battery_part_number);
        dprintf(DPRINTF_DEFAULT, " battery_type[16] = %16.s\n",
                ise->battery[i].battery_type);
        dprintf(DPRINTF_DEFAULT, " battery_x_position = %lld\n",
                ise->battery[i].battery_x_position);
        dprintf(DPRINTF_DEFAULT, " battery_y_position = %lld\n",
                ise->battery[i].battery_y_position);
        dprintf(DPRINTF_DEFAULT, " battery_z_position = %lld\n",
                ise->battery[i].battery_z_position);
        dprintf(DPRINTF_DEFAULT, " %d battery_status = 0x%08x battery_status_details = 0x%016llx\n",
                i, ise->battery[i].battery_status, ise->battery[i].battery_status_details);
        dprintf(DPRINTF_DEFAULT, " battery_remaining_charge = %lld\n ",
                ise->battery[i].battery_remaining_charge);
        dprintf(DPRINTF_DEFAULT, " battery_max_charge = %lld\n",
                ise->battery[i].battery_max_charge);
        dprintf(DPRINTF_DEFAULT, " battery_max_charge_capacity = %lld\n ",
                ise->battery[i].battery_max_charge_capacity);
        dprintf(DPRINTF_DEFAULT, " battery_min_holdup_time = %lld\n",
                ise->battery[i].battery_min_holdup_time);
        dprintf(DPRINTF_DEFAULT, " battery_charger_state = %d\n",
                ise->battery[i].battery_charger_state);
        dprintf(DPRINTF_DEFAULT, " battery_charger_state_details= %d\n ",
                ise->battery[i].battery_charger_state_details);
    }

    for (i = 0; i < 2; i++)
    {
        dprintf(DPRINTF_DEFAULT, " -----------ISE POWER SUPPLY %d DETAILS------\n ", i);
        dprintf(DPRINTF_DEFAULT, " powersupply_beacon = %d\n",
                ise->powersupply[i].powersupply_beacon);
        dprintf(DPRINTF_DEFAULT, " powersupply_model = %.16s\n",
                ise->powersupply[i].powersupply_model);
        dprintf(DPRINTF_DEFAULT, " powersupply_serial_number = %.12s\n ",
                ise->powersupply[i].powersupply_serial_number);
        dprintf(DPRINTF_DEFAULT, " powersupply_part_number = %.16s\n ",
                ise->powersupply[i].powersupply_part_number);
        dprintf(DPRINTF_DEFAULT, " powersupply_x_position = %lld\n",
                ise->powersupply[i].powersupply_x_position);
        dprintf(DPRINTF_DEFAULT, " powersupply_y_position = %lld\n ",
                ise->powersupply[i].powersupply_y_position);
        dprintf(DPRINTF_DEFAULT, " powersupply_z_position = %lld\n",
                ise->powersupply[i].powersupply_z_position);
        dprintf(DPRINTF_DEFAULT, " %d powersupply_status = 0x%08x  powersupply_status_details = 0x%016llx\n",
                ise->powersupply[i].powersupply_status,
                ise->powersupply[i].powersupply_status_details);
        dprintf(DPRINTF_DEFAULT, " powersupply_fan1_status = %d\n",
                ise->powersupply[i].powersupply_fan1_status);
        dprintf(DPRINTF_DEFAULT, " powersupply_fan1_speed = %lld\n",
                ise->powersupply[i].powersupply_fan1_speed);
        dprintf(DPRINTF_DEFAULT, " powersupply_fan2_status = %d\n",
                ise->powersupply[i].powersupply_fan2_status);
        dprintf(DPRINTF_DEFAULT, " powersupply_fan2_speed = %lld\n",
                ise->powersupply[i].powersupply_fan2_speed);
        dprintf(DPRINTF_DEFAULT, " powersupply_temperature = %d\n",
                ise->powersupply[i].powersupply_temperature);
    }

    for (i = 0; i < 2; i++)
    {
        dprintf(DPRINTF_DEFAULT, "----------ISE DATAPAC %d DETAILS----------\n ", i);
        dprintf(DPRINTF_DEFAULT, " datapac_beacon = %d\n",
                ise->datapac[i].datapac_beacon);
        dprintf(DPRINTF_DEFAULT, " datapac_type = %d\n", ise->datapac[i].datapac_type);
        dprintf(DPRINTF_DEFAULT, " datapac_serial_number[12]= %.12s\n",
                ise->datapac[i].datapac_serial_number);
        dprintf(DPRINTF_DEFAULT, " datapac_model[16] = %.16s\n ",
                ise->datapac[i].datapac_model);
        dprintf(DPRINTF_DEFAULT, " datapac_part_number= %.16s\n",
                ise->datapac[i].datapac_part_number);
        dprintf(DPRINTF_DEFAULT, " datapac_spare_level = %lld\n",
                ise->datapac[i].datapac_spare_level);
        dprintf(DPRINTF_DEFAULT, " datapac_x_position= %lld\n",
                ise->datapac[i].datapac_x_position);
        dprintf(DPRINTF_DEFAULT, " datapac_y_position= %lld\n",
                ise->datapac[i].datapac_y_position);
        dprintf(DPRINTF_DEFAULT, " datapac_z_position = %lld\n",
                ise->datapac[i].datapac_z_position);
        dprintf(DPRINTF_DEFAULT, " %d datapac_status = 0x%08x  datapac_status_details = 0x%016llx\n",
                i, ise->datapac[i].datapac_status, ise->datapac[i].datapac_status_details);
        dprintf(DPRINTF_DEFAULT, " datapac_capacity=%lld\n ",
                ise->datapac[i].datapac_capacity);
        dprintf(DPRINTF_DEFAULT, " datapac_fw_version= %.16s\n ",
                ise->datapac[i].datapac_fw_version);
        dprintf(DPRINTF_DEFAULT, " datapac_temperature = %d\n  ",
                ise->datapac[i].datapac_temperature);
        dprintf(DPRINTF_DEFAULT, " datapac_health = %d\n",
                ise->datapac[i].datapac_health);
    }

}
#endif /* 0 */

/***
 ** Modelines:
 ** Local Variables:
 ** tab-width: 4
 ** indent-tabs-mode: nil
 ** End:
 ** vi:sw=4 ts=4 expandtab
 **/
