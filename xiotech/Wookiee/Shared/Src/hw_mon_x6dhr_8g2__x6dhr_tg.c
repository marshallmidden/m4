/* $URL: file:///media/m4/svn-repo/eng/trunk/storage/Wookiee/Shared/Src/hw_mon_x6dhr_8g2__x6dhr_tg.c $ */
/**
******************************************************************************
**
**  @file       hw_mon_x6dhr_8g2__x6dhr_tg.c
**
**  @brief      Hardware monitoring layout for supermicro x6dhr_8g2__x6dhr_tg.
**
**  Hardware monitoring layout for supermicro x6dhr_8g2__x6dhr_tg.
**
**  Copyright (c) 2006-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

/***********************************************
** This include must be first in all hw modules.
***********************************************/
#include <stdint.h>
#include "hw_common.h"

/*
** Rest of includes.
*/
#include "hw_mon.h"

/*
******************************************************************************
** Private variables
******************************************************************************
*/

/** Layout defining hw monitor for supermicro x6dhr_8g2__x6dhr_tg */
typedef struct hw_sm_x6dhr_8g2__x6dhr_tg_750
{
  hw_mon_envdevice_internal             cn;
    hw_mon_envmeasure_internal            cn_tempc;
    hw_mon_envmeasure_internal            cn_volt_vcc5;
    hw_mon_envmeasure_internal            cn_volt_vsb;
    hw_mon_envmeasure_internal            cn_volt_vbat;
    hw_mon_envmeasure_internal            cn_volt_3_3;
    hw_mon_envmeasure_internal            cn_volt_12;
    hw_mon_envmeasure_internal            cn_volt_neg_12;
    hw_mon_envmeasure_internal            cn_volt_5;

    hw_mon_envdevice_internal           cpu0;
      hw_mon_envmeasure_internal          cpu0_tempc;
      hw_mon_envmeasure_internal          cpu0_voltage;

    hw_mon_envdevice_internal           fan0;
      hw_mon_envmeasure_internal          fan0_rpm;

            hw_mon_envdevice_internal           fan1;
      hw_mon_envmeasure_internal          fan1_rpm;

    hw_mon_envdevice_internal           fan2;
      hw_mon_envmeasure_internal          fan2_rpm;

    hw_mon_envdevice_internal           fan3;
      hw_mon_envmeasure_internal          fan3_rpm;

    hw_mon_envdevice_internal           fan4;
      hw_mon_envmeasure_internal          fan4_rpm;

    hw_mon_envdevice_internal           ps;
      hw_mon_envmeasure_internal          ps_status;

/*
** Debug Only
*/

    hw_mon_envdevice_internal           ps0;
      hw_mon_envmeasure_internal          ps0_temp;
      hw_mon_envmeasure_internal          ps0_volt_3_3;
      hw_mon_envmeasure_internal          ps0_volt_5;
      hw_mon_envmeasure_internal          ps0_volt_12;
      hw_mon_envmeasure_internal          ps0_fan_rpm;

    hw_mon_envdevice_internal           ps1;
      hw_mon_envmeasure_internal          ps1_temp;
      hw_mon_envmeasure_internal          ps1_volt_3_3;
      hw_mon_envmeasure_internal          ps1_volt_5;
      hw_mon_envmeasure_internal          ps1_volt_12;
      hw_mon_envmeasure_internal          ps1_fan_rpm;

    hw_mon_envdevice_internal           ps2;
      hw_mon_envmeasure_internal          ps2_temp;
      hw_mon_envmeasure_internal          ps2_volt_3_3;
      hw_mon_envmeasure_internal          ps2_volt_5;
      hw_mon_envmeasure_internal          ps2_volt_12;
      hw_mon_envmeasure_internal          ps2_fan_rpm;

} hw_sm_x6dhr_8g2__x6dhr_tg_750;

struct hw_sm_x6dhr_8g2__x6dhr_tg_750 ed_750_data = {

    /* Controller device */
    .cn = {
      .parent_device = NULL,
      .dev = {
        .id         = 0,
        .type       = EDT_CONTROLLER,
        .status     = ES_NA,
        .num_msr    = 8,
        .num_dev    = 10,
        .length     = sizeof(hw_sm_x6dhr_8g2__x6dhr_tg_750),
        .name       = "controller",
        .flags      = EDF_FRU,
        .dyn_off    = HWMON_INT_DV_DYN,
      },
    },

    /* Controller measurements. */
      /* Temp Celcius */
      .cn_tempc = {
        .msr = {
          .id         = 0,
          .status     = ES_NA,
          .type       = EMT_TEMP,
          .length     = HWMON_INT_MS_LN,
          .name       = "temperature",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.cn,
          .data             = "/sys/bus/i2c/devices/0-002f/temp3_input",
          .max_error        = "/sys/bus/i2c/devices/0-002f/temp3_max",
          .max_warn         = "/sys/bus/i2c/devices/0-002f/temp3_max_hyst",
        },
        .data = {
          .max_error  = LLONG_MAX,
          .max_warn   = LLONG_MAX,
          .min_error  = LLONG_MIN,
          .min_warn   = LLONG_MIN,
        },
      },

      /* VCC 5v */
      .cn_volt_vcc5 = {
        .msr = {
          .id         = 1,
          .status     = ES_NA,
          .type       = EMT_VOLTAGE,
          .length     = HWMON_INT_MS_LN,
          .name       = "vcc 5v voltage",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.cn,
          .data             = "/sys/bus/i2c/devices/0-002f/in6_input",
          .max_error        = "/sys/bus/i2c/devices/0-002f/in6_max",
          .min_error        = "/sys/bus/i2c/devices/0-002f/in6_min",
        },
        .data = {
          .max_error  = 5250,
          .max_warn   = LLONG_MAX,
          .min_error  = 4750,
          .min_warn   = LLONG_MIN,
        },
      },

      /* VSB 5V */
      .cn_volt_vsb = {
        .msr = {
          .id         = 2,
          .status     = ES_NA,
          .type       = EMT_VOLTAGE,
          .length     = HWMON_INT_MS_LN,
          .name       = "vsb standby 5v voltage",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.cn,
          .data             = "/sys/bus/i2c/devices/0-002f/in7_input",
          .max_error        = "/sys/bus/i2c/devices/0-002f/in7_max",
          .min_error        = "/sys/bus/i2c/devices/0-002f/in7_min",
        },
        .data = {
          .max_error  = 5300,
          .max_warn   = LLONG_MAX,
          .min_error  = 4700,
          .min_warn   = LLONG_MIN,
        },
      },

      /* VBAT 3.3V */
      .cn_volt_vbat = {
        .msr = {
          .id         = 3,
          .status     = ES_NA,
          .type       = EMT_VOLTAGE,
          .length     = HWMON_INT_MS_LN,
          .name       = "vbat 3.3v voltage",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.cn,
          .data             = "/sys/bus/i2c/devices/0-002f/in8_input",
          .max_error        = "/sys/bus/i2c/devices/0-002f/in8_max",
          .min_error        = "/sys/bus/i2c/devices/0-002f/in8_min",
        },
        .data = {
          .max_error  = 3600,
          .max_warn   = LLONG_MAX,
          .min_error  = 2800,
          .min_warn   = LLONG_MIN,
        },
      },

      /* 3.3 voltage */
      .cn_volt_3_3 = {
        .msr = {
          .id         = 4,
          .status     = ES_NA,
          .type       = EMT_VOLTAGE,
          .length     = HWMON_INT_MS_LN,
          .name       = "3.3v voltage",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.cn,
          .data             = "/sys/bus/i2c/devices/0-002f/in2_input",
          .max_error        = "/sys/bus/i2c/devices/0-002f/in2_max",
          .min_error        = "/sys/bus/i2c/devices/0-002f/in2_min",
        },
        .data = {
          .max_error  = 3450,
          .max_warn   = LLONG_MAX,
          .min_error  = 3150,
          .min_warn   = LLONG_MIN,
        },
      },

      /* 12 volt */
      .cn_volt_12 = {
        .msr = {
          .id         = 5,
          .status     = ES_NA,
          .type       = EMT_VOLTAGE,
          .length     = HWMON_INT_MS_LN,
          .name       = "12v voltage",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.cn,
          .calc_rd_fmt      = "%ld / (10/38)",  /* from supermicro Data* 0.016/ (10/38) */
          .calc_wr_fmt      = "%ld * (10/38)",
          .data             = "/sys/bus/i2c/devices/0-002f/in5_input",
          .max_error        = "/sys/bus/i2c/devices/0-002f/in5_max",
          .min_error        = "/sys/bus/i2c/devices/0-002f/in5_min",
        },
        .data = {
          .max_error  = 13200,
          .max_warn   = LLONG_MAX,
          .min_error  = 10800,
          .min_warn   = LLONG_MIN,
        },
      },

      /* -12 volt */
      .cn_volt_neg_12 = {
        .msr = {
          .id         = 6,
          .status     = ES_NA,
          .type       = EMT_VOLTAGE,
          .length     = HWMON_INT_MS_LN,
          .name       = "-12v voltage",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device = &ed_750_data.cn,
          .calc_rd_fmt      = "((%ld - 2048) * ((232+28)/28)) + 2048",  /* (Data-2.048) * ((232+28)/28) + 2.048 */
          .calc_wr_fmt      = "((%ld - 2048) / ((232+28)/28)) + 2048",
          .data             = "/sys/bus/i2c/devices/0-002f/in4_input",
          .max_error        = "/sys/bus/i2c/devices/0-002f/in4_max",
          .min_error        = "/sys/bus/i2c/devices/0-002f/in4_min",
        },
        .data = {
          .max_error  = -10200,
          .max_warn   = LLONG_MAX,
          .min_error  = -13800,
          .min_warn   = LLONG_MIN,
        },
      },

      /* +5 volt */
      .cn_volt_5 = {
        .msr = {
          .id         = 7,
          .status     = ES_NA,
          .type       = EMT_VOLTAGE,
          .length     = HWMON_INT_MS_LN,
          .name       = "5v voltage",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.cn,
          .calc_rd_fmt      = "%ld / (50/84)",  /* from supermicro Data* 0.016/ (50/84) */
          .calc_wr_fmt      = "%ld * (50/84)",
          .data             = "/sys/bus/i2c/devices/0-002f/in3_input",
          .max_error        = "/sys/bus/i2c/devices/0-002f/in3_max",
          .min_error        = "/sys/bus/i2c/devices/0-002f/in3_min",
        },
        .data = {
          .max_error  = 5250,
          .max_warn   = LLONG_MAX,
          .min_error  = 4750,
          .min_warn   = LLONG_MIN,
        },
      },




    /* CPU 0 */
    .cpu0 = {
      .parent_device = &ed_750_data.cn,
      .dev = {
        .id         = 1,
        .type       = EDT_CPU,
        .status     = ES_NA,
        .num_msr    = 2,
        .num_dev    = 0,
        .length     = HWMON_INT_DV_LN(0,2),
        .name       = "cpu 0",
        .dyn_off    = HWMON_INT_DV_DYN,
      },
    },

    /* CPU measurements. */
      /* Temp Celcius */
      .cpu0_tempc = {
        .msr = {
          .id         = 0,
          .status     = ES_NA,
          .type       = EMT_TEMP,
          .length     = HWMON_INT_MS_LN,
          .name       = "temperature",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.cpu0,
          .data             = "/sys/bus/i2c/devices/0-002f/temp1_input",
          .max_error        = "/sys/bus/i2c/devices/0-002f/temp1_max",
          .max_warn         = "/sys/bus/i2c/devices/0-002f/temp1_max_hyst",
        },
        .data = {
          .max_error  = LLONG_MAX,
          .max_warn   = LLONG_MAX,
          .min_error  = LLONG_MIN,
          .min_warn   = LLONG_MIN,
        },
      },

      /* voltage */
      .cpu0_voltage = {
        .msr = {
          .id         = 1,
          .status     = ES_NA,
          .type       = EMT_VOLTAGE,
          .length     = HWMON_INT_MS_LN,
          .name       = "voltage",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.cpu0,
          .data             = "/sys/bus/i2c/devices/0-002f/in0_input",
          .max_error        = "/sys/bus/i2c/devices/0-002f/in0_max",
          .min_error        = "/sys/bus/i2c/devices/0-002f/in0_min",
        },
        .data = {
          .max_error  = LLONG_MAX,
          .max_warn   = LLONG_MAX,
          .min_error  = LLONG_MIN,
          .min_warn   = LLONG_MIN,
        },
      },




    /* FAN 0 */
    .fan0 = {
      .parent_device = &ed_750_data.cn,
      .dev = {
        .id         = 2,
        .type       = EDT_FAN,
        .status     = ES_NA,
        .num_msr    = 1,
        .num_dev    = 0,
        .length     = HWMON_INT_DV_LN(0,1),
        .name       = "fan 1",
        .flags      = EDF_FRU,
        .dyn_off    = HWMON_INT_DV_DYN,
      },
    },

    /* FAN measurements. */
      /* rpm */
      .fan0_rpm = {
        .msr = {
          .id         = 0,
          .status     = ES_NA,
          .type       = EMT_RPM,
          .length     = HWMON_INT_MS_LN,
          .name       = "rpm",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.fan0,
          .data             = "/sys/bus/i2c/devices/0-002f/fan1_input",
          .min_error        = "/sys/bus/i2c/devices/0-002f/fan1_min",
        },
        .data = {
          .max_error  = LLONG_MAX,
          .max_warn   = LLONG_MAX,
          .min_error  = LLONG_MIN,
          .min_warn   = LLONG_MIN,
        },
      },

    /* FAN 1 */
    .fan1 = {
      .parent_device = &ed_750_data.cn,
      .dev = {
        .id         = 3,
        .type       = EDT_FAN,
        .status     = ES_NA,
        .num_msr    = 1,
        .num_dev    = 0,
        .length     = HWMON_INT_DV_LN(0,1),
        .name       = "fan 2",
        .flags      = EDF_FRU,
        .dyn_off    = HWMON_INT_DV_DYN,
      },
    },

    /* FAN measurements. */
      /* rpm */
      .fan1_rpm = {
        .msr = {
          .id         = 0,
          .status     = ES_NA,
          .type       = EMT_RPM,
          .length     = HWMON_INT_MS_LN,
          .name       = "rpm",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.fan1,
          .data             = "/sys/bus/i2c/devices/0-002f/fan2_input",
          .min_error        = "/sys/bus/i2c/devices/0-002f/fan2_min",
        },
        .data = {
          .max_error  = LLONG_MAX,
          .max_warn   = LLONG_MAX,
          .min_error  = LLONG_MIN,
          .min_warn   = LLONG_MIN,
        },
      },

    /* FAN 2 */
    .fan2 = {
      .parent_device = &ed_750_data.cn,
      .dev = {
        .id         = 4,
        .type       = EDT_FAN,
        .status     = ES_NA,
        .num_msr    = 1,
        .num_dev    = 0,
        .length     = HWMON_INT_DV_LN(0,1),
        .name       = "fan 3",
        .flags      = EDF_FRU,
        .dyn_off    = HWMON_INT_DV_DYN,
      },
    },

    /* FAN measurements. */
      /* rpm */
      .fan2_rpm = {
        .msr = {
          .id         = 0,
          .status     = ES_NA,
          .type       = EMT_RPM,
          .length     = HWMON_INT_MS_LN,
          .name       = "rpm",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.fan2,
          .data             = "/sys/bus/i2c/devices/0-002f/fan3_input",
          .min_error        = "/sys/bus/i2c/devices/0-002f/fan3_min",
        },
        .data = {
          .max_error  = LLONG_MAX,
          .max_warn   = LLONG_MAX,
          .min_error  = LLONG_MIN,
          .min_warn   = LLONG_MIN,
        },
      },

    /* FAN 3 */
    .fan3 = {
      .parent_device = &ed_750_data.cn,
      .dev = {
        .id         = 5,
        .type       = EDT_FAN,
        .status     = ES_NA,
        .num_msr    = 1,
        .num_dev    = 0,
        .length     = HWMON_INT_DV_LN(0,1),
        .name       = "controller blank fan 1",
        .flags      = EDF_FRU,
        .dyn_off    = HWMON_INT_DV_DYN,
      },
    },

    /* FAN measurements. */
      /* rpm */
      .fan3_rpm = {
        .msr = {
          .id         = 0,
          .status     = ES_NA,
          .type       = EMT_RPM,
          .length     = HWMON_INT_MS_LN,
          .name       = "rpm",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.fan3,
          .data             = "/sys/bus/i2c/devices/0-002f/fan4_input",
          .min_error        = "/sys/bus/i2c/devices/0-002f/fan4_min",
          .flags            = EMF_IGN_FFAIL,
        },
        .data = {
          .max_error  = LLONG_MAX,
          .max_warn   = LLONG_MAX,
          .min_error  = LLONG_MIN,
          .min_warn   = LLONG_MIN,
        },
      },

    /* FAN 4 */
    .fan4 = {
      .parent_device = &ed_750_data.cn,
      .dev = {
        .id         = 6,
        .type       = EDT_FAN,
        .status     = ES_NA,
        .num_msr    = 1,
        .num_dev    = 0,
        .length     = HWMON_INT_DV_LN(0,1),
        .name       = "controller blank fan 2",
        .flags      = EDF_FRU,
        .dyn_off    = HWMON_INT_DV_DYN,
      },
    },

    /* FAN measurements. */
      /* rpm */
      .fan4_rpm = {
        .msr = {
          .id         = 0,
          .status     = ES_NA,
          .type       = EMT_RPM,
          .length     = HWMON_INT_MS_LN,
          .name       = "rpm",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.fan4,
          .data             = "/sys/bus/i2c/devices/0-002f/fan5_input",
          .min_error        = "/sys/bus/i2c/devices/0-002f/fan5_min",
          .flags            = EMF_IGN_FFAIL,
        },
        .data = {
          .max_error  = LLONG_MAX,
          .max_warn   = LLONG_MAX,
          .min_error  = LLONG_MIN,
          .min_warn   = LLONG_MIN,
        },
      },



    /* Power Supplies */
    .ps = {
      .parent_device = &ed_750_data.cn,
      .dev = {
        .id         = 7,
        .type       = EDT_POWERSUPPLY,
        .status     = ES_NA,
        .num_msr    = 1,
        .num_dev    = 0,
        .length     = HWMON_INT_DV_LN(0,1),
        .name       = "power supply",
        .flags      = EDF_FRU,
        .dyn_off    = HWMON_INT_DV_DYN,
      },
    },

      /* status through fan */
      .ps_status = {
        .msr = {
          .id         = 0,
          .status     = ES_NA,
          .type       = EMT_RPM,
          .length     = HWMON_INT_MS_LN,
          .name       = "status",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.ps,
          .data             = "/sys/bus/i2c/devices/9191-0290/powergood",
          .flags            = EMF_STATUS_ONLY,
        },
        .data = {
          .max_error  = LLONG_MAX,
          .max_warn   = LLONG_MAX,
          .min_error  = 0,
          .min_warn   = LLONG_MIN,
        },
      },



/*********************************
** DEBUG ONLY PS monitoring of measures.
*********************************/

    /* PS 0 */
    .ps0 = {
      .parent_device = &ed_750_data.cn,
      .dev = {
        .id         = 8,
        .type       = EDT_POWERSUPPLY,
        .status     = ES_NA,
        .num_msr    = 5,
        .num_dev    = 0,
        .length     = HWMON_INT_DV_LN(0,5),
        .name       = "power supply (upper)",
        .flags      = EMF_DEBUG_ONLY,
        .dyn_off    = HWMON_INT_DV_DYN,
      },
    },

    /* PS 0 measurements. */
      /* temp */
      .ps0_temp = {
        .msr = {
          .id         = 0,
          .status     = ES_NA,
          .type       = EMT_TEMP,
          .length     = HWMON_INT_MS_LN,
          .name       = "temperature",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.ps0,
          .calc_rd_fmt      = "1000 * %ld",
          .data             = "/sys/bus/i2c/devices/0-002d/temperature",
          .flags            = EMF_DEBUG_ONLY,
        },
        .data = {
          .max_error  = LLONG_MAX,
          .max_warn   = LLONG_MAX,
          .min_error  = LLONG_MIN,
          .min_warn   = LLONG_MIN,
        },
      },

      /* 3.3 voltage */
      .ps0_volt_3_3 = {
        .msr = {
          .id         = 1,
          .status     = ES_NA,
          .type       = EMT_VOLTAGE,
          .length     = HWMON_INT_MS_LN,
          .name       = "3.3v voltage",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.ps0,
          .calc_rd_fmt      = "7.8125 * %ld * 2",
          .data             = "/sys/bus/i2c/devices/0-002d/voltage_3_3",
          .flags            = EMF_DEBUG_ONLY,
        },
        .data = {
          .max_error  = 3600,
          .max_warn   = LLONG_MAX,
          .min_error  = 3150,
          .min_warn   = LLONG_MIN,
        },
      },

      /* 5 voltage */
      .ps0_volt_5 = {
        .msr = {
          .id         = 2,
          .status     = ES_NA,
          .type       = EMT_VOLTAGE,
          .length     = HWMON_INT_MS_LN,
          .name       = "5v voltage",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.ps0,
          .calc_rd_fmt      = "7.8125 * %ld * 6",
          .data             = "/sys/bus/i2c/devices/0-002d/voltage_5",
          .flags            = EMF_DEBUG_ONLY,
        },
        .data = {
          .max_error  = 5500,
          .max_warn   = LLONG_MAX,
          .min_error  = 4750,
          .min_warn   = LLONG_MIN,
        },
      },

      /* 12 voltage */
      .ps0_volt_12 = {
        .msr = {
          .id         = 3,
          .status     = ES_NA,
          .type       = EMT_VOLTAGE,
          .length     = HWMON_INT_MS_LN,
          .name       = "12v voltage",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.ps0,
          .calc_rd_fmt      = "7.8125 * %ld * 11",
          .data             = "/sys/bus/i2c/devices/0-002d/voltage_12",
          .flags            = EMF_DEBUG_ONLY,
        },
        .data = {
          .max_error  = 14000,
          .max_warn   = LLONG_MAX,
          .min_error  = 10800,
          .min_warn   = LLONG_MIN,
        },
      },

      /* status through fan */
      .ps0_fan_rpm = {
        .msr = {
          .id         = 4,
          .status     = ES_NA,
          .type       = EMT_RPM,
          .length     = HWMON_INT_MS_LN,
          .name       = "fan",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.ps0,
          .data             = "/sys/bus/i2c/devices/0-002d/status0",
          .flags            = EMF_DEBUG_ONLY,
        },
        .data = {
          .max_error  = 1,
          .max_warn   = LLONG_MAX,
          .min_error  = LLONG_MIN,
          .min_warn   = LLONG_MIN,
        },
      },


    /* PS 1 */
    .ps1 = {
      .parent_device = &ed_750_data.cn,
      .dev = {
        .id         = 9,
        .type       = EDT_POWERSUPPLY,
        .status     = ES_NA,
        .num_msr    = 5,
        .num_dev    = 0,
        .length     = HWMON_INT_DV_LN(0,5),
        .name       = "power supply (middle)",
        .flags      = EMF_DEBUG_ONLY,
        .dyn_off    = HWMON_INT_DV_DYN,
      },
    },

    /* PS 1 measurements. */
      /* temp */
      .ps1_temp = {
        .msr = {
          .id         = 0,
          .status     = ES_NA,
          .type       = EMT_TEMP,
          .length     = HWMON_INT_MS_LN,
          .name       = "temperature",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.ps1,
          .calc_rd_fmt      = "1000 * %ld",
          .data             = "/sys/bus/i2c/devices/0-002d/temperature",
          .flags            = EMF_DEBUG_ONLY,
        },
        .data = {
          .max_error  = LLONG_MAX,
          .max_warn   = LLONG_MAX,
          .min_error  = LLONG_MIN,
          .min_warn   = LLONG_MIN,
        },
      },

      /* 3.3 voltage */
      .ps1_volt_3_3 = {
        .msr = {
          .id         = 1,
          .status     = ES_NA,
          .type       = EMT_VOLTAGE,
          .length     = HWMON_INT_MS_LN,
          .name       = "3.3v voltage",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.ps1,
          .calc_rd_fmt      = "7.8125 * %ld * 2",
          .data             = "/sys/bus/i2c/devices/0-002d/voltage_3_3",
          .flags            = EMF_DEBUG_ONLY,
        },
        .data = {
          .max_error  = 3600,
          .max_warn   = LLONG_MAX,
          .min_error  = 3150,
          .min_warn   = LLONG_MIN,
        },
      },

      /* 5 voltage */
      .ps1_volt_5 = {
        .msr = {
          .id         = 2,
          .status     = ES_NA,
          .type       = EMT_VOLTAGE,
          .length     = HWMON_INT_MS_LN,
          .name       = "5v voltage",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.ps1,
          .calc_rd_fmt      = "7.8125 * %ld * 6",
          .data             = "/sys/bus/i2c/devices/0-002d/voltage_5",
          .flags            = EMF_DEBUG_ONLY,
        },
        .data = {
          .max_error  = 5500,
          .max_warn   = LLONG_MAX,
          .min_error  = 4750,
          .min_warn   = LLONG_MIN,
        },
      },

      /* 12 voltage */
      .ps1_volt_12 = {
        .msr = {
          .id         = 3,
          .status     = ES_NA,
          .type       = EMT_VOLTAGE,
          .length     = HWMON_INT_MS_LN,
          .name       = "12v voltage",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.ps1,
          .calc_rd_fmt      = "7.8125 * %ld * 11",
          .data             = "/sys/bus/i2c/devices/0-002d/voltage_12",
          .flags            = EMF_DEBUG_ONLY,
        },
        .data = {
          .max_error  = 14000,
          .max_warn   = LLONG_MAX,
          .min_error  = 10800,
          .min_warn   = LLONG_MIN,
        },
      },

      /* status through fan */
      .ps1_fan_rpm = {
        .msr = {
          .id         = 4,
          .status     = ES_NA,
          .type       = EMT_RPM,
          .length     = HWMON_INT_MS_LN,
          .name       = "fan",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.ps1,
          .data             = "/sys/bus/i2c/devices/0-002d/status1",
          .flags            = EMF_DEBUG_ONLY,
        },
        .data = {
          .max_error  = 1,
          .max_warn   = LLONG_MAX,
          .min_error  = LLONG_MIN,
          .min_warn   = LLONG_MIN,
        },
      },


    /* PS 2 */
    .ps2 = {
      .parent_device = &ed_750_data.cn,
      .dev = {
        .id         = 10,
        .type       = EDT_POWERSUPPLY,
        .status     = ES_NA,
        .num_msr    = 5,
        .num_dev    = 0,
        .length     = HWMON_INT_DV_LN(0,5),
        .name       = "power supply (lower)",
        .flags      = EMF_DEBUG_ONLY,
        .dyn_off    = HWMON_INT_DV_DYN,
      },
    },

    /* PS 2 measurements. */
      /* temp */
      .ps2_temp = {
        .msr = {
          .id         = 0,
          .status     = ES_NA,
          .type       = EMT_TEMP,
          .length     = HWMON_INT_MS_LN,
          .name       = "temperature",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.ps2,
          .calc_rd_fmt      = "1000 * %ld",
          .data             = "/sys/bus/i2c/devices/0-002d/temperature",
          .flags            = EMF_DEBUG_ONLY,
        },
        .data = {
          .max_error  = LLONG_MAX,
          .max_warn   = LLONG_MAX,
          .min_error  = LLONG_MIN,
          .min_warn   = LLONG_MIN,
        },
      },

      /* 3.3 voltage */
      .ps2_volt_3_3 = {
        .msr = {
          .id         = 1,
          .status     = ES_NA,
          .type       = EMT_VOLTAGE,
          .length     = HWMON_INT_MS_LN,
          .name       = "3.3v voltage",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.ps2,
          .calc_rd_fmt      = "7.8125 * %ld * 2",
          .data             = "/sys/bus/i2c/devices/0-002d/voltage_3_3",
          .flags            = EMF_DEBUG_ONLY,
        },
        .data = {
          .max_error  = 3600,
          .max_warn   = LLONG_MAX,
          .min_error  = 3150,
          .min_warn   = LLONG_MIN,
        },
      },

      /* 5 voltage */
      .ps2_volt_5 = {
        .msr = {
          .id         = 2,
          .status     = ES_NA,
          .type       = EMT_VOLTAGE,
          .length     = HWMON_INT_MS_LN,
          .name       = "5v voltage",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.ps2,
          .calc_rd_fmt      = "7.8125 * %ld * 6",
          .data             = "/sys/bus/i2c/devices/0-002d/voltage_5",
          .flags            = EMF_DEBUG_ONLY,
        },
        .data = {
          .max_error  = 5500,
          .max_warn   = LLONG_MAX,
          .min_error  = 4750,
          .min_warn   = LLONG_MIN,
        },
      },

      /* 12 voltage */
      .ps2_volt_12 = {
        .msr = {
          .id         = 3,
          .status     = ES_NA,
          .type       = EMT_VOLTAGE,
          .length     = HWMON_INT_MS_LN,
          .name       = "12v voltage",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.ps2,
          .calc_rd_fmt      = "7.8125 * %ld * 11",
          .data             = "/sys/bus/i2c/devices/0-002d/voltage_12",
          .flags            = EMF_DEBUG_ONLY,
        },
        .data = {
          .max_error  = 14000,
          .max_warn   = LLONG_MAX,
          .min_error  = 10800,
          .min_warn   = LLONG_MIN,
        },
      },

      /* status through fan */
      .ps2_fan_rpm = {
        .msr = {
          .id         = 4,
          .status     = ES_NA,
          .type       = EMT_RPM,
          .length     = HWMON_INT_MS_LN,
          .name       = "fan",
          .dyn_off    = HWMON_INT_MS_DYN,
        },
        .access = {
          .parent_device    = &ed_750_data.ps2,
          .data             = "/sys/bus/i2c/devices/0-002d/status2",
          .flags            = EMF_DEBUG_ONLY,
        },
        .data = {
          .max_error  = 1,
          .max_warn   = LLONG_MAX,
          .min_error  = LLONG_MIN,
          .min_warn   = LLONG_MIN,
        },
      },
};

/**
** Link in.
**/
hw_mon_envdevice_internal*  linked_root_dev = (hw_mon_envdevice_internal*)&ed_750_data;

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
