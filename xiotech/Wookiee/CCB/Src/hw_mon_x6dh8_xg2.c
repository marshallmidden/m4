/* $Id: hw_mon_x6dh8_xg2.c 125754 2010-02-01 17:48:51Z mdr $ */
/**
******************************************************************************
**
**  @file       hw_mon_x6dh8_xg2.c
**
**  @brief      Hardware monitoring layout for Supermicro x6dh8_xg2.
**
**  Hardware monitoring layout for Supermicro x6dh8_xg2.
**
**  Copyright (c) 2008-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

/***********************************************
** This include must be first in all hw modules.
***********************************************/
#include <stdint.h>
#include "hw_common.h"

/* Rest of includes. */

#include "hw_mon.h"
#include "HWM.h"

/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/

#define HW_DATA ed_4000_data

#define SYS_SENSORS "/sys/bus/i2c/devices/0-002e"
#define SYS_FAN     "/sys/devices/platform/pc87427.2112"
#define SYS_EDAC    "/sys/devices/system/edac"

#define CPU_TEMP(t) .data = SYS_SENSORS "/temp" #t "_input"

/*
******************************************************************************
** Private conversion functions
******************************************************************************
*/

/*
******************************************************************************
**
**  @brief  Convert 3.3 volt reading read to millivolts
**
**  @param  value
**
**  @return converted value
**
******************************************************************************
*/
static int64_t volt_rd3(int64_t value)
{
    double      flt = value;
    int64_t     ret;

    flt *= 0.996818181;
    flt += 0.5;

    ret = flt;

    return ret;
}

/*
******************************************************************************
**
**  @brief  Convert 3.3 volt reading to write to millivolts
**
**  @param  value
**
**  @return converted value
**
******************************************************************************
*/
static int64_t volt_wr3(int64_t value)
{
    double      flt = value;
    int64_t     ret;

    flt *= 1.003191974;
    flt += 0.5;

    ret = flt;

    return ret;
}

/*
******************************************************************************
**
**  @brief  Convert 12 volt reading read to millivolts
**
**  @param  value
**
**  @return converted value
**
******************************************************************************
*/
static int64_t volt_rd12(int64_t value)
{
    double      flt = value;
    int64_t     ret;

    flt *= 12.89441747;
    flt += 0.5;

    ret = flt;

    return ret;
}

/*
******************************************************************************
**
**  @brief  Convert 12 volt reading to write to millivolts
**
**  @param  value
**
**  @return converted value
**
******************************************************************************
*/
static int64_t volt_wr12(int64_t value)
{
    double      flt = value;
    int64_t     ret;

    flt *= 0.077552941;
    flt += 0.5;

    ret = flt;

    return ret;
}

/*
******************************************************************************
**
**  @brief  Convert -12 volt reading read to millivolts
**
**  @param  value
**
**  @return converted value
**
******************************************************************************
*/
static int64_t volt_rdn12(int64_t value)
{
    double      flt = value;
    int64_t     ret;

    flt *= 5.006953;
    flt -= 13299.5;

    ret = flt;

    return ret;
}

/*
******************************************************************************
**
**  @brief  Convert -12 volt reading to write to millivolts
**
**  @param  value
**
**  @return converted value
**
******************************************************************************
*/
static int64_t volt_wrn12(int64_t value)
{
    double      flt = value;
    int64_t     ret;

    flt += 13299.5;
    flt *= 0.199722266;

    ret = flt;

    return ret;
}

/*
******************************************************************************
**
**  @brief  Convert power supply status to state
**
**  @param  value
**
**  @return converted value TRUE if good
**
******************************************************************************
*/
static int64_t ps_state_rd(int64_t value)
{
    return (value & 0x80) != 0;
}


/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/

/** Layout defining hw monitor for Supermicro x6dh8_xg2 */
typedef struct hw_sm_x6dh8_xg2
{
    hw_mon_envdevice_internal cn;
    hw_mon_envmeasure_internal cn_tempc;
    hw_mon_envmeasure_internal cn_volt_vcc5;
    hw_mon_envmeasure_internal cn_volt_3_3;
    hw_mon_envmeasure_internal cn_volt_12;
    hw_mon_envmeasure_internal cn_volt_neg_12;

                CPU_DECL(0);
                CPU_DECL(1);

#ifndef S_SPLINT_S
                FAN_DECL(1);
                FAN_DECL(2);
                FAN_DECL(3);
                FAN_DECL(4);
#endif                          /* S_SPLINT_S */

    hw_mon_envdevice_internal ps;
    hw_mon_envmeasure_internal ps_state;

                EDAC_DECL(0);

                BUS_DECL(0);
} hw_sm_x6dh8_xg2;

/*
******************************************************************************
** Private variables
******************************************************************************
*/

static struct hw_sm_x6dh8_xg2 HW_DATA =
{
    /* Controller device */
    .cn = {.parent_device = NULL,
           .dev = {.id = 0,
                   .type = EDT_CONTROLLER,
                   .status = ES_NA,
                   .num_msr = 5,.num_dev = 9,
                   .length = sizeof(hw_sm_x6dh8_xg2),
                   .name = "controller",
                   .flags = EDF_FRU,
                   .dyn_off = HWMON_INT_DV_DYN,
                   },
           },

    /* Controller measurements. */
    /* Temp Celsius */
    .cn_tempc = {.msr = MSR_TEMP(0, "temperature"),
                 .access = {.parent_device = &HW_DATA.cn,
                            .data = SYS_SENSORS "/temp3_input",
                            .max_error = SYS_SENSORS "/temp3_max",
                            },
                 .data = DATA_NOERROR,
                 },

    /* VCC 5v */
    .cn_volt_vcc5 = {.msr = MSR_VOLTAGE(1, "vcc 5v voltage"),
                     .access = {.parent_device = &HW_DATA.cn,
                                .data = SYS_SENSORS "/in10_input",
                                .max_error = SYS_SENSORS "/in10_max",
                                .min_error = SYS_SENSORS "/in10_min",
                                },
                     .data = DATA_NOERROR,
                     },

    /* 3.3 voltage */
    .cn_volt_3_3 = {.msr = MSR_VOLTAGE(2, "3.3v voltage"),
                    .access = {.parent_device = &HW_DATA.cn,
                               .data = SYS_SENSORS "/in9_input",
                               .max_error = SYS_SENSORS "/in9_max",
                               .min_error = SYS_SENSORS "/in9_min",
                               .calc_rd = volt_rd3,
                               .calc_wr = volt_wr3,
                               },
                    .data = DATA_NOERROR,
                    },

    /* 12 volt */
    .cn_volt_12 = {.msr = MSR_VOLTAGE(3, "12v voltage"),
                   .access = {.parent_device = &HW_DATA.cn,
                              .data = SYS_SENSORS "/in1_input",
                              .max_error = SYS_SENSORS "/in1_max",
                              .min_error = SYS_SENSORS "/in1_min",
                              .calc_rd = volt_rd12,
                              .calc_wr = volt_wr12,
                              },
                   .data = DATA_NOERROR,
                   },

    /* -12 volt */
    .cn_volt_neg_12 = {.msr = MSR_VOLTAGE(4, "-12v voltage"),
                       .access = {.parent_device = &HW_DATA.cn,
                                  .data = SYS_SENSORS "/in15_input",
                                  .max_error = SYS_SENSORS "/in15_max",
                                  .min_error = SYS_SENSORS "/in15_min",
                                  .calc_rd = volt_rdn12,
                                  .calc_wr = volt_wrn12,
                                  },
                       .data = DATA_NOERROR,
                       },

    CPU_DEF(0, 0, 1),       /* CPU 0 */
    CPU_DEF(1, 1, 2),       /* CPU 1 */

    FAN_DEF(1, 2),              /* FAN 1 */
    FAN_DEF(2, 3),              /* FAN 2 */
    FAN_DEF(3, 4),              /* FAN 3 */
    FAN_DEF(4, 5),              /* FAN 4 */

    /* Power supply */
    .ps = {DEV(6, EDT_POWERSUPPLY, 1, "power supply", EDF_FRU)},

    /* Power supply measurements */
    /* state */
    .ps_state = {.msr = MSR_STATE(0, "ps_state"),
                 .access = {.parent_device = &HW_DATA.ps,
                            .data = SYS_FAN "/gpio6_input",
                            //.flags      = EMF_STATUS_ONLY,
                            .calc_rd = ps_state_rd,
                            },
                 .data = DATA_STATE_ERROR_FALSE,
                 },

    EDAC_DEF(0, 7),

    BUS_DEF(0, 8),
};

/*
******************************************************************************
** Public variables not in any header file
******************************************************************************
*/
hw_mon_envdevice_internal *root_dev_x6dh8_xg2 = &HW_DATA.cn;

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/*
******************************************************************************
**
**  @brief  Update power supply voltages for X6DH8-XG2
**
**  @param  s - Pointer to HWM_STATUS to update
**
**  @return none
**
******************************************************************************
**/
static void update_power_supply_voltages(HWM_STATUS *s)
{
    struct hw_sm_x6dh8_xg2 *h = &HW_DATA;
    POWER_SUPPLY_VOLTAGES_STATUS *vs;

    vs = &s->procBoardStatus.powerSupplyVoltagesStatus;

    update_voltage_reading(&vs->twelveVoltReading, &h->cn_volt_12.data);
    update_voltage_reading(&vs->fiveVoltReading, &h->cn_volt_vcc5.data);
    update_voltage_reading(&vs->threePointThreeVoltReading, &h->cn_volt_3_3.data);
    update_voltage_reading(&vs->standbyVoltageReading, &h->cn_volt_3_3.data);
}


/*
******************************************************************************
**
**  @brief  Update CPU stats for X6DH8-XG2
**
**  @param  s - Pointer to HWM_STATUS to update
**
**  @return none
**
******************************************************************************
**/
static void update_cpu_stats(HWM_STATUS *s)
{
    struct hw_sm_x6dh8_xg2 *h = &HW_DATA;
    PROC_BOARD_PROCESSOR_STATUS *ps;
    TEMPERATURE_STATUS  *ts;
    hw_mon_envmeasure_internal  *m;

    ps = &s->procBoardStatus.backEndProcessorStatus;
    ts = &ps->temperatureStatus;
    m = NULL;

#define CPU_TEMP_SELECT(i, p) do {     \
    if (!(p) || (sensor_valid(&h->cpu##i##_tempc) &&    \
        h->cpu##i##_tempc.data.data > 0 &&  \
        h->cpu##i##_tempc.data.data > (p)->data.data))  \
    { p = &h->cpu##i##_tempc; } } while (0)

    CPU_TEMP_SELECT(0, m);
    CPU_TEMP_SELECT(1, m);

    if (m)
    {
        update_cpu_temperature(ts, &m->data);
    }

    ps = &s->procBoardStatus.frontEndProcessorStatus;
    ts = &ps->temperatureStatus;

    m = &h->cn_tempc;
    if (sensor_valid(m))
    {
        update_cpu_temperature(ts, &m->data);
    }
}


/*
******************************************************************************
**
**  @brief  Update power supply stats for X6DH8-XG2
**
**  @param  s - Pointer to HWM_STATUS to update
**
**  @return none
**
******************************************************************************
**/
static void update_power_supply_stats(HWM_STATUS *s)
{
    struct hw_sm_x6dh8_xg2 *h = &HW_DATA;
    POWER_SUPPLY_STATUS *p;
    COOLING_FAN_CONDITION_VALUE cond;
    hw_mon_envmeasure_internal *f;
    hw_mon_envmeasure_internal *f2;

#define FAN_SELECT(i, x, p) do { if ((x) != &h->fan##i##_rpm &&     \
        (!(p) || h->fan##i##_rpm.data.data < (p)->data.data))   \
    { p = &h->fan##i##_rpm; } } while (0)

    f = NULL;
    p = &s->frontEndPowerSupply;

    FAN_SELECT(1, NULL, f);
    FAN_SELECT(2, NULL, f);
    FAN_SELECT(3, NULL, f);
    FAN_SELECT(4, NULL, f);

    cond = COOLING_FAN_CONDITION_GOOD;
    if (f->data.data < f->data.min_warn ||
        f->data.data < f->data.min_error ||
        f->data.data > f->data.max_warn || f->data.data > f->data.max_error)
    {
        cond = COOLING_FAN_CONDITION_FAILED;
    }
    p->coolingFanConditionValue = cond;

    f2 = NULL;
    p = &s->backEndPowerSupply;

    FAN_SELECT(1, f, f2);
    FAN_SELECT(2, f, f2);
    FAN_SELECT(3, f, f2);
    FAN_SELECT(4, f, f2);

    cond = COOLING_FAN_CONDITION_GOOD;
    if (f2->data.data < f2->data.min_warn ||
        f2->data.data < f2->data.min_error ||
        f2->data.data > f2->data.max_warn || f2->data.data > f2->data.max_error)
    {
        cond = COOLING_FAN_CONDITION_FAILED;
    }
    p->coolingFanConditionValue = cond;

    /* Check power supply state */

    switch (h->ps_state.msr.status)
    {
        case ES_NA:
            p->powerSupplyCondition.value = POWER_SUPPLY_CONDITION_UNKNOWN;
            break;

        case ES_GOOD:
            p->powerSupplyCondition.value = POWER_SUPPLY_CONDITION_GOOD;
            break;

        case ES_WARNING:
        case ES_ERROR:
            p->powerSupplyCondition.value = POWER_SUPPLY_CONDITION_AC_FAILED;
            break;
    }
}


/*
******************************************************************************
**
**  @brief  Analyze sensor data and generate any needed events
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
void analyze_x6dh8_xg2(void)
{
    HWM_STATUS *s = &updateMonitorData.monitorStatus;

    update_power_supply_voltages(s);

    update_cpu_stats(s);

    update_power_supply_stats(s);

    HWM_AnalyzeMonitorData(&updateMonitorData, &currentMonitorData);

    currentMonitorData = updateMonitorData;
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
