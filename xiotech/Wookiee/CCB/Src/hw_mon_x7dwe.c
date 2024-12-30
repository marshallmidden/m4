/* $Id: hw_mon_x7dwe.c 125754 2010-02-01 17:48:51Z mdr $ */
/**
******************************************************************************
**
**  @file       hw_mon_x7dwe.c
**
**  @brief      Hardware monitoring layout for Supermicro x7dwe.
**
**  Copyright (c) 2008-2010 XIOtech Corporation. All rights reserved.
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

#define HW_DATA ed_7000_data

#define SYS_SENSORS "/sys/bus/i2c/devices/0-002f"
#define SYS_FAN     SYS_SENSORS
#define SYS_EDAC    "/sys/devices/system/edac"

#define CPU_SENSORS "/sys/devices/platform/coretemp."
#define CPU_TEMP(t) .data = CPU_SENSORS #t "/temp1_input",  \
            .max_error = CPU_SENSORS #t "/temp1_crit",      \
            .max_warn = CPU_SENSORS #t "/temp1_max"

/*
******************************************************************************
** Private conversion functions
******************************************************************************
*/

/*
******************************************************************************
**
**  @brief  Convert 12v read value
**
**  @param  value - value to convert to millivolts
**
**  @return millivots
**
******************************************************************************
*/
static int64_t volt12_rd(int64_t value)
{
    return value * 12;
}

/*
******************************************************************************
**
**  @brief  Convert 12v value to write
**
**  @param  mv - millivolts to convert to value
**
**  @return converted value
**
******************************************************************************
*/
static int64_t volt12_wr(int64_t mv)
{
    return mv / 12;
}

/*
******************************************************************************
**
**  @brief  Convert -12v read value to millivolts
**
**  @param  value - value to convert to millivolts
**
**  @return millivolts
**
******************************************************************************
*/
static int64_t voltn12_rd(int64_t value)
{
    return ((65 * value) - 118784) / 7;
}

/*
******************************************************************************
**
**  @brief  Convert -12v value to write
**
**  @param  mv - millivolts to convert to value
**
**  @return converted value
**
******************************************************************************
*/
static int64_t voltn12_wr(int64_t mv)
{
    return ((7 * mv) + 118784) / 65;
}


/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/

/** Layout defining hw monitor for Supermicro x7dwe */
typedef struct hw_sm_x7dwe_7000
{
    hw_mon_envdevice_internal cn;
    hw_mon_envmeasure_internal cn_tempc;
    hw_mon_envmeasure_internal cn_volt_vcc5;
    hw_mon_envmeasure_internal cn_volt_vsb;
    hw_mon_envmeasure_internal cn_volt_vbat;
    hw_mon_envmeasure_internal cn_volt_3_3;
    hw_mon_envmeasure_internal cn_volt_12;
    hw_mon_envmeasure_internal cn_volt_neg_12;
    hw_mon_envmeasure_internal cn_volt_vtt;

                CPU_DECL(0);
                CPU_DECL(1);
                CPU_DECL(2);
                CPU_DECL(3);
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
} hw_sm_x7dwe_7000;

/*
******************************************************************************
** Private variables
******************************************************************************
*/

static struct hw_sm_x7dwe_7000 HW_DATA =
{
    /* Controller device */
    .cn = {.parent_device = NULL,
           .dev = {.id = 0,
                   .type = EDT_CONTROLLER,
                   .status = ES_NA,
                   .num_msr = 8,.num_dev = 11,
                   .length = sizeof(hw_sm_x7dwe_7000),
                   .name = "controller",
                   .flags = EDF_FRU,
                   .dyn_off = HWMON_INT_DV_DYN,
                   },
           },

    /* Controller measurements. */
    /* Temp Celsius */
    .cn_tempc = {.msr = MSR_TEMP(0, "temperature"),
                 .access = {.parent_device = &HW_DATA.cn,
                            .data = SYS_SENSORS "/temp5_input",
                            .max_error = SYS_SENSORS "/temp5_max",
                            .max_warn = SYS_SENSORS "/temp5_max_hyst",
                            },
                 .data = DATA_NOERROR
                },

    /* VCC 5v */
    .cn_volt_vcc5 = {.msr = MSR_VOLTAGE(1, "vcc 5v voltage"),
                     .access = {.parent_device = &HW_DATA.cn,
                                .data = SYS_SENSORS "/in7_input",
                                .max_error = SYS_SENSORS "/in7_max",
                                .min_error = SYS_SENSORS "/in7_min",
                                },
                     .data = DATA_NOERROR,
#if 0
                     .data = {
                              .max_error = 5310,
                              .max_warn = LLONG_MAX,
                              .min_error = 4710,
                              .min_warn = LLONG_MIN,
                              },
#endif /* 0 */
                     },

    /* VSB 5V */
    .cn_volt_vsb = {.msr = MSR_VOLTAGE(2, "vsb standby 5v voltage"),
                    .access = {.parent_device = &HW_DATA.cn,
                               .data = SYS_SENSORS "/in8_input",
                               .max_error = SYS_SENSORS "/in8_max",
                               .min_error = SYS_SENSORS "/in8_min",
                               },
                    .data = DATA_NOERROR,
#if 0
                    .data = {
                             .max_error = 5300,
                             .max_warn = LLONG_MAX,
                             .min_error = 4700,
                             .min_warn = LLONG_MIN,
                             },
#endif /* 0 */
                    },

    /* VBAT 3.3V */
    .cn_volt_vbat = {.msr = MSR_VOLTAGE(3, "vbat 3.3v voltage"),
                     .access = {.parent_device = &HW_DATA.cn,
                                .data = SYS_SENSORS "/in9_input",
                                .max_error = SYS_SENSORS "/in9_max",
                                .min_error = SYS_SENSORS "/in9_min",
                                },
                     .data = DATA_NOERROR,
#if 0
                     .data = {
                              .max_error = 3664,
                              .max_warn = LLONG_MAX,
                              .min_error = 2992,
                              .min_warn = LLONG_MIN,
                              },
#endif /* 0 */
                     },

    /* 3.3 voltage */
    .cn_volt_3_3 = {
                    .msr = MSR_VOLTAGE(4, "3.3v voltage"),
                    .access = {.parent_device = &HW_DATA.cn,
                               .data = SYS_SENSORS "/in5_input",
                               },
                    .data = DATA_NOERROR,
#if 0
                    .data = {
                             .max_error = 3360,
                             .max_warn = LLONG_MAX,
                             .min_error = 2845,
                             .min_warn = LLONG_MIN,
                             },
#endif /* 0 */
                    },

    /* 12 volt */
    .cn_volt_12 = {.msr = MSR_VOLTAGE(5, "12v voltage"),
                   .access = {.parent_device = &HW_DATA.cn,
                              .data = SYS_SENSORS "/in6_input",
                              .max_error = SYS_SENSORS "/in6_max",
                              .min_error = SYS_SENSORS "/in6_min",
                              .calc_rd = volt12_rd,
                              .calc_wr = volt12_wr,
                              },
                   .data = DATA_NOERROR,
#if 0
                   .data = {
                            .max_error = 13200,.max_warn = LLONG_MAX,
                            .min_error = 10800,.min_warn = LLONG_MIN,
                            },
#endif /* 0 */
                   },

    /* -12 volt */
    .cn_volt_neg_12 = {.msr = MSR_VOLTAGE(6, "-12v voltage"),
                       .access = {.parent_device = &HW_DATA.cn,
                                  .data = SYS_SENSORS "/in3_input",
                                  .max_error = SYS_SENSORS "/in3_max",
                                  .min_error = SYS_SENSORS "/in3_min",
                                  .calc_rd = voltn12_rd,
                                  .calc_wr = voltn12_wr,
                                  },
                       .data = DATA_NOERROR,
#if 0
                       .data = {
                                .max_error = -10200,.max_warn = LLONG_MAX,
                                .min_error = -13800,.min_warn = LLONG_MIN,
                                },
#endif /* 0 */
                       },

    /* Vtt volt */
    .cn_volt_vtt = {.msr = MSR_VOLTAGE(7, "Vtt voltage"),
                    .access = {.parent_device = &HW_DATA.cn,
                               .data = SYS_SENSORS "/in2_input",
                               .max_error = SYS_SENSORS "/in2_max",
                               .min_error = SYS_SENSORS "/in2_min",
                               },
                    .data = DATA_NOERROR,
#if 0
                    .data = {
                             .max_error = 1486,.max_warn = LLONG_MAX,
                             .min_error = 920,.min_warn = LLONG_MIN,
                             },
#endif /* 0 */
                    },

    CPU_DEF(0, 0, 0),       /* CPU 0 */
    CPU_DEF(1, 1, 1),       /* CPU 1 */
    CPU_DEF(2, 2, 2),       /* CPU 2 */
    CPU_DEF(3, 3, 3),       /* CPU 3 */

    FAN_DEF(1, 4),              /* FAN 1 */
    FAN_DEF(2, 5),              /* FAN 2 */
    FAN_DEF(3, 6),              /* FAN 3 */
    FAN_DEF(4, 7),              /* FAN 4 */

    /* Power supply */
    .ps = {DEV(8, EDT_POWERSUPPLY, 1, "power supply", EDF_FRU)},

    /* Power supply measurements */
    /* state */
    .ps_state = {.msr = MSR_STATE(0, "ps_state"),
                 .access = {.parent_device = &HW_DATA.ps,
                            .data = "/sys/bus/i2c/devices/9191-0290/ps_state",
                            //.flags  = EMF_STATUS_ONLY,
                            },
                 .data = DATA_STATE_ERROR_FALSE,
                 },

    EDAC_DEF(0, 9),

    BUS_DEF(0, 10),
};

/*
******************************************************************************
** Public variables not in any header file
******************************************************************************
*/
hw_mon_envdevice_internal *root_dev_x7dwe = &HW_DATA.cn;

/*
******************************************************************************
** Code Start
******************************************************************************
*/

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
void analyze_x7dwe(void)
{
    HWM_STATUS *s = &updateMonitorData.monitorStatus;
    struct hw_sm_x7dwe_7000 *h = &HW_DATA;

    {
        POWER_SUPPLY_VOLTAGES_STATUS *vs;

        vs = &s->procBoardStatus.powerSupplyVoltagesStatus;

        update_voltage_reading(&vs->twelveVoltReading, &h->cn_volt_12.data);
        update_voltage_reading(&vs->fiveVoltReading, &h->cn_volt_vcc5.data);
        update_voltage_reading(&vs->threePointThreeVoltReading, &h->cn_volt_3_3.data);
        update_voltage_reading(&vs->standbyVoltageReading, &h->cn_volt_vsb.data);
    }

    {
        PROC_BOARD_PROCESSOR_STATUS *ps;
        TEMPERATURE_STATUS  *ts;
        hw_mon_envmeasure_internal  *m;

        ps = &s->procBoardStatus.backEndProcessorStatus;
        ts = &ps->temperatureStatus;
        m = NULL;

#define CPU_TEMP_SELECT(i, p) do {         \
        if (!(p) || (sensor_valid(&h->cpu##i##_tempc) &&    \
            h->cpu##i##_tempc.data.data > 0 &&  \
            h->cpu##i##_tempc.data.data > (p)->data.data))  \
        { p = &h->cpu##i##_tempc; } } while (0)

        CPU_TEMP_SELECT(0, m);
        CPU_TEMP_SELECT(1, m);
        CPU_TEMP_SELECT(2, m);
        CPU_TEMP_SELECT(3, m);

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

    {
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
