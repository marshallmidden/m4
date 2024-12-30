/* $Id: HWM.c 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       HWM.c
** MODULE TITLE:    Hardware monitor task impementation
**
** DESCRIPTION:     Hardware monitor task functions
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "HWM.h"

#include "debug_files.h"
#include "logging.h"
#include "timer.h"
#include "XIO_Std.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
#define XIO_HW_3000 "SMX6DH8-XG2"
#define XIO_HW_7000 "SMX7DWE"

/*****************************************************************************
** Private variables
*****************************************************************************/
static IPMI_BMC *pHardwareMonitorBMC = NULL;

/*****************************************************************************
** Public variables - externed in the header file or elsewise
*****************************************************************************/
UINT8       gMonitorEnabledFlag = TRUE;
HWM_DATA    currentMonitorData;

/*****************************************************************************
** Code Start
*****************************************************************************/

/**
******************************************************************************
**
**  @brief      Tell the hardware monitor to reconfigure the BMC Ethernet
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void HWM_ConfigureEthernet(void)
{
    if (pHardwareMonitorBMC && (hwm_platform->flags & PLATFORM_FLAG_CONFIG_BMC_NET))
    {
        IPMI_BMCConfigureEthernet(pHardwareMonitorBMC);
    }
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_MonitorTask
**
**  Parameters: None
**
**  Returns:    Nothing
**--------------------------------------------------------------------------*/
void HWM_MonitorTask(UNUSED TASK_PARMS *parms)
{
    IPMI_INTERFACE *pInterface = NULL;

    /*
     * Debugging?
     modeData.ccb.bitsDPrintf |= MD_DPRINTF_IPMI_VERBOSE;
     modeData.ccb.bitsDPrintf |= MD_DPRINTF_IPMI;
     */

    dprintf(DPRINTF_IPMI, "HWM: Main starting\n");

    while (!hwm_platform)
    {
        fprintf(stderr, "%s: Waiting for hwm_platform\n", __func__);
        TaskSleepMS(HWM_PERIOD);
    }

    /* Initialize and start the IPMI interface */

    if (IPMI_InterfaceCreate(&pInterface) == GOOD && pInterface)
    {
        dprintf(DPRINTF_IPMI, "HWM: IPMI interface started\n");

        /* Verify the BMC is functioning correctly */

        if (IPMI_BMCCreate(&pHardwareMonitorBMC, pInterface) == GOOD)
        {
            dprintf(DPRINTF_IPMI, "HWM: Domain created\n");
        }
        else
        {
            dprintf(DPRINTF_IPMI, "HWM: Domain creation failed\n");
        }
    }
    else
    {
        dprintf(DPRINTF_IPMI, "HWM: Failed to open IPMI interface\n");
    }

    dprintf(DPRINTF_HWM, "MT: Hardware monitor forked\n");

    /* Initialize the monitor data structures */

    currentMonitorData = updateMonitorData;

    for (;;)
    {
        TaskSleepMS(HWM_PERIOD);

        if (IPMI_BMCConfigureEthernet(pHardwareMonitorBMC) == GOOD)
        {
            fprintf(stderr, "%s: BMC Ethernet configured\n", __func__);
            break;
        }
    }
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetMonitorStatus
**
**  Parameters:    monitorStatus - pointer to store the data
**
**--------------------------------------------------------------------------*/
UINT32 HWM_GetMonitorStatus(HWM_STATUS *monitorStatus)
{
    if (!monitorStatus)
    {
        fprintf(stderr, "%s: monitorStatus is NULL\n", __func__);
        return ERROR;
    }

    /*
     * Copy the current monitor status into the location pointed
     * to by monitorStatus.  This gives the caller a copy of the
     * data, so we don't risk them messing up our structure.
     */
    *monitorStatus = currentMonitorData.monitorStatus;

    return GOOD;
}

/*----------------------------------------------------------------------------
**  Function Name:  HWM_MonitorDisable
**
**  Parameters:     none
**
**  Returns:        GOOD or ERROR
**--------------------------------------------------------------------------*/
UINT32 HWM_MonitorDisable(void)
{
    dprintf(DPRINTF_DEFAULT, "HWM: Monitor disabled\n");
    gMonitorEnabledFlag = FALSE;

    return GOOD;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
