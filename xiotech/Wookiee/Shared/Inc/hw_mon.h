/* $Id: hw_mon.h 145021 2010-08-03 14:16:38Z m4 $ */
/**
******************************************************************************
**
**  @file       hw_mon.h
**
**  @brief      Hardware monitoring interface and definitions.
**
**  Copyright (c) 2006,2009-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#ifndef __HW_MON__
#define __HW_MON__

#ifndef LOG_SIMULATOR

#include <stdint.h>
#include <uuid/uuid.h>
#include <limits.h>

#ifndef LLONG_MAX
  #define LLONG_MAX  9223372036854775807LL
#endif
#ifndef LLONG_MIN
  #define LLONG_MIN  (-LLONG_MAX - 1LL)
#endif

/* #pragma pack(push,1) */

/**
**  @ingroup _HW_MODULES_
**  @defgroup _HW_MODULE_HWMON_ Hardware Monitor Module
**
**  @brief  Hardware monitoring interface and definitions.
**
**    The thought behind the hardware monitor and environmental reporting is
**  to keep the interface as simple and as expandable as possible.  This will
**  allow higher level clients the capability of monitoring distinct hardware
**  platforms running this monitor while retaining the ability to separate
**  interface from guts.
**
**    The management perspective is hierarchical with the controller being the
**  top-level device (struct _env_device) containing sub-devices and measures
**  (struct _env_measure).  Each device status will be a collective
**  representation of itself and any sub-devices or measures it may parent.
** <PRE>
**    A controller may contain 0 or more sub-devices:
**      fans
**      diskbays
**      cpu(s)
**      powersupply(s)
**      etc...
**
**    A device will have 0 or more measures:
**      rpm
**      voltage
**      temp
**      etc...
** </PRE>
**
**    Each measure will have a corresponding data structure representing the
**  details of the measurement (i.e. struct _env_generic_measure_data).
**  \ref _HW_MODULE_HWMON_STRUCT_MEASURE_GEN_DATA.
**
**    All devices and measures will have a name string.  By following this
**  name string reverse from the root parent, a description of the device
**  can be represented.
** <PRE>
**   device
**     name = CN1
**     device
**       name = fan 0
**       measure
**         name = rpm
**         status = warning
**         data
**           value      = 500
**           min_warn   = 515
**           min_err    = 475
**           max_warn   = 575
**           max_err    = 615
**
**
**    As we can see from this, we can generate a string for the measure in
**  the warning state by starting at the root device and concatenating the
**  names of the devices and measure through the path.
**
**   "CN1 fan 0 rpm warning"
**
**  The data could even be analyzed to produce the reason for the warning.
** </PRE> **/

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/**
**  @ingroup _HW_MODULE_HWMON_
**  @defgroup _HW_MODULE_HWMON_CONSTANTS Hardware Monitor Constants
**
**  @brief  Constants used in the Hardware Monitor Module.
**/

/**
**  @ingroup _HW_MODULE_HWMON_CONSTANTS
**  @defgroup _HW_MODULE_HWMON_CONSTANTS_MISC Miscellaneous Constants
**
**  @brief      Statuses reported on a device and/or measure.
**
**  @{
**/
#define ENV_MAX_NAME_STR        64  /**< Max string name for device/measure */
#define ENV_MAX_LOG_STR         128 /**< Max string name for device/measure */
/* @} */

/**
**  @ingroup _HW_MODULE_HWMON_CONSTANTS
**  @defgroup _HW_MODULE_HWMON_CONSTANTS_INIT_FLAGS Init Flag Constants
**
**  @brief      Initialization Flags.
**
**  @{
**/
#define HWMON_BIN_LOGS     0x00000001  /**< Generate Binary Log    */
/* @} */

/**
**  @ingroup _HW_MODULE_HWMON_CONSTANTS
**  @defgroup _HW_MODULE_HWMON_CONSTANTS_STATUS Status Constants
**
**  @brief      Statuses reported on a device and/or measure.
**              struct _env_device or struct _env_measure.
**
**  @{
**/
enum env_status
{
    ES_NA           = 0,    /**< non-applicable (status has no bearing)     */
    ES_GOOD,                /**< device/measure is good                     */
    ES_WARNING,             /**< device/measure violates warning threshold  */
    ES_ERROR,               /**< device/measure is bad                      */
    ES_NOACCESS,            /**< device/measure is unaccessible             */
};
/* @} */

/**
**  @ingroup _HW_MODULE_HWMON_CONSTANTS
**  @defgroup _HW_MODULE_HWMON_CONSTANTS_EDT Device Type Constants
**
**  @brief      These are the device types as reported in struct _env_device.
**
**  @{
**/
enum env_device_type
{
    EDT_CONTROLLER  = 0,    /**< device is a controller     */
    EDT_BAY,                /**< device is a disk bay       */
    EDT_POWERSUPPLY,        /**< device is a power supply   */
    EDT_FAN,                /**< device is a fan            */
    EDT_CPU,                /**< device is a cpu            */
    EDT_MEMORY,             /**< device is memory           */
    EDT_BUS,                /**< device is a bus            */
};
/* @} */

/**
**  @ingroup _HW_MODULE_HWMON_CONSTANTS
**  @defgroup _HW_MODULE_HWMON_CONSTANTS_EDF Device Flag Constants
**
**  @brief      These are the device flags as reported
**              in struct env_device.
**
**  @{
**/
enum env_device_flags
{
    EDF_FRU         = 0x00000001,   /**< device is a field replaceable unit */
    EDF_RESERVED    = 0x80000000,   /**< Reserved                           */
};
/* @} */

/**
**  @ingroup _HW_MODULE_HWMON_CONSTANTS
**  @defgroup _HW_MODULE_HWMON_CONSTANTS_EMT Measure Type Constants
**
**  @brief      These are the device types as reported in struct _env_measure.
**
**  @{
**/
enum env_measure_type
{
    EMT_TEMP    = 0,    /**< temperature (degrees celsius * 1000)   */
    EMT_VOLTAGE,        /**< voltage ( millivolts )                 */
    EMT_RPM,            /**< revolutions ( per minute )             */
    EMT_STATE,          /**< state (1 = good, 0 = bad)              */
    EMT_COUNT,          /**< event count                            */
};
/* @} */

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/

/**
**  @ingroup _HW_MODULE_HWMON_
**  @defgroup _HW_MODULE_HWMON_MACROS Hardware Monitor Macros
**
**  @brief      Macros used with the Hardware Monitor Module.
**/

/**
**  @ingroup _HW_MODULE_HWMON_MACROS
**  @defgroup _HW_MODULE_HWMON_MACROS_MISC Hardware Monitor Misc Macros
**
**  @brief      Miscellaneous Macros
**
**  @{
**/
/** If I can think of 1, it will go here :-)  */
#define HWMON_THIS_IS_A_FAKE_MACRO_SO_I_HOPE_YOU_LIKE_READING_IT ;
/* @} */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/**
**  @ingroup _HW_MODULE_HWMON_
**  @defgroup _HW_MODULE_HWMON_STRUCT Hardware Monitor Structures
**
**  @brief      Structures used in the Hardware Monitor Module.
**/

/**
**  @ingroup _HW_MODULE_HWMON_STRUCT
**  @defgroup _HW_MODULE_HWMON_STRUCT_MEASURE Measure Structure
**
**  @brief      This structure represents a measurement of a device.
**              (i.e. temp,rpm,volt).  Each measurement will have a name,
**              id, status, and data representing the measurement in detail.
**
**  @{
**/
/** Measure Structure */
typedef struct env_measure
{
    uint32_t    length;     /**< length of static+dynamic data */
    uint32_t    id;         /**< measurement id */
    uint32_t    type;       /**< measurement type ( see env_measure_type )  */
    uint32_t    status;     /**< measurement status ( see env_status )      */
    char        name[ENV_MAX_NAME_STR]; /**< name of measurement            */
    uint32_t    rsvd1;      /**< reserved                                   */
    uint32_t    dyn_off;    /**< offset from struct static to dynamic       */
    uint8_t     data[0];    /**< data for this measurement                  */
} env_measure;
/* @} */

/**
**  @ingroup _HW_MODULE_HWMON_STRUCT
**  @defgroup _HW_MODULE_HWMON_STRUCT_DEVICE Device Structure
**
**  @brief      This structure represents a device. A device may have 0+ measures
**              and 0+ sub-devices (devices it reports on).
**
**  @{
**/
/** Device Structure */
typedef struct env_device
{
    uint32_t    length;     /**< length of static+dynamic data */
    uint32_t    id;         /**< device id */
    uuid_t      uuid;       /**< unique identifier */
    uint16_t    type;       /**< device type ( see env_device_type ) */
    uint16_t    status;     /**< device status ( see env_status ) */
    uint16_t    num_msr;    /**< number of measurements */
    uint16_t    num_dev;    /**< number of sub-devices */
    char        name[ENV_MAX_NAME_STR]; /**< name of device */
    uint32_t    flags;      /**< device flags ( see env_device_flags ) */
    uint32_t    dyn_off;    /**< offset from struct static to dynamic */
    uint8_t     measures[0];/**< measurements (struct env_measure * num_msr) */
    uint8_t     devices[0]; /**< sub-devices  (struct env_device * num_dev) */
} env_device;
/* @} */

/**
**  @ingroup _HW_MODULE_HWMON_STRUCT
**  @defgroup _HW_MODULE_HWMON_STRUCT_MEASURE_GEN_DATA Generic Measure Data Structure
**
**  @brief      This structure is generic representation of the data portion
**              of struct _env_measure.
**
**  @{
**/
/** Generic Measure Data Structure */
typedef struct env_generic_measure_data
{
    int64_t data;           /**< measurement value */

    /* The max thresholds will be LLONG_MAX if they are unused */
    /* The min thresholds will be LLONG_MIN if they are unused */
    int64_t max_error;      /**< measurement maximum ( error threshold ) */
    int64_t max_warn;       /**< measurement maximum ( warning threshold ) */
    int64_t min_error;      /**< measurement minimum ( error threshold ) */
    int64_t min_warn;       /**< measurement minimum ( warning threshold ) */
} env_generic_measure_data;
/* @} */

/**
**  @ingroup _HW_MODULE_HWMON_STRUCT
**  @defgroup _HW_MODULE_HWMON_STRUCT_REGISTER_DATA Initialization Registration Structure
**
**  @brief  This structure is passed in the initialization function and provides
**          the necessary information for hw_mon to initialize internally as
**          well as pointers to some bridging functions for logging purposes.
**
**  @{
**/
/** Log Function Prototype */
typedef struct hwenv_ldata
{
    char        logstr[ENV_MAX_LOG_STR];    /**< String message */
    uint32_t    flags;                      /**< flags */
    uint32_t    length;                     /**< length if env_device */
    env_device  dev;                        /**< dev with problem */
} hwenv_ldata;

typedef void (*hwilf)(hwenv_ldata *);

/** Initialization Registration Structure */
typedef struct hw_mon_register_data
{
    const char  *top_level_name;    /**< Top level name to use for device */
    const char  *conf_file; /**< Configuration file (NULL if statically linked)*/
    uint32_t    reserved;   /**< reserved */
    uint32_t    flags;      /**< flags for hw_mon (see Initialization Flags ) */
    hwilf       log_debug;  /**< Pointer To debug log function */
    hwilf       log_info;   /**< Pointer To information log function */
    hwilf       log_warn;   /**< Pointer To warning log function */
    hwilf       log_error;  /**< Pointer To error log function */
} hw_mon_register_data;
/* @} */

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

/**
**  @ingroup _HW_MODULE_HWMON_
**  @defgroup _HW_MODULE_HWMON_FUNCTIONS Hardware Monitor Functions
**
**  @brief      This is a complete list of function within this library.
**              Public functions are available for use in the Interface.
**              Private functions are internally used within the library.
**/

/**
**  @ingroup _HW_MODULE_HWMON_FUNCTIONS
**  @defgroup _HW_MODULE_HWMON_PUBLIC_FUNCTIONS Hardware Monitor Public Functions
**
**  @brief      These are the public functions available for this interface.
**
**  @{
**/
int32_t hw_hwmon_init(hw_mon_register_data *init_data);
uint32_t hw_hwmon_get_clean_data(env_device **pdevice, void *rdevice);
/* @} */

/*****************************************************************************
**  Internal Layout Section ( for creating layout structures for hw_mon)
*****************************************************************************/

/**
**  @ingroup _HW_MODULE_HWMON_
**  @defgroup _HW_MODULE_HWMON_INTERNAL_ Hardware Monitor Internal Interface/Structures
**
**  @brief  Hardware monitoring internal interface and definitions.
**
**    This module defines the internal interface for the hardware monitor.
**
** <PRE>
**   hw_mon_envdevice_internal *linked_root_dev = (hw_mon_envdevice_internal*) & some_data;
**
**   The layout is essentially a single device, defined within the
**   initialization function, which contains measures and subdevices.
**   Each measure is an endpoint, but a subdevice must contain at least
**   one measure and possibly other subdevices.
**
**   Each measure and device within the same set must have unique id's.
**     i.e. s1 s2 s3 s4
**
**          d0
**             m0
**             m1
**             d0
**                m0
**                d0
**                   m0
**             d1
**                m0
**                m1
**
**   A.  Create a structure.
**     typedef struct hw_some_hw_structure
**     {
**       hw_mon_envdevice_internal             cn;
**         hw_mon_envmeasure_internal            cn_tempc;
**         hw_mon_envmeasure_internal            cn_volt_vcc5;
**         ...
**
**         hw_mon_envdevice_internal           cpu0;
**           hw_mon_envmeasure_internal          cpu0_tempc;
**           hw_mon_envmeasure_internal          cpu0_voltage;
**                                             ...
**
**         hw_mon_envdevice_internal           fan0;
**           hw_mon_envmeasure_internal          fan0_rpm;
**
**         hw_mon_envdevice_internal           fan1;
**           hw_mon_envmeasure_internal          fan1_rpm;
**                                             ...
**
**         hw_mon_envdevice_internal           ps0;
**           hw_mon_envmeasure_internal          ps0_temp;
**           hw_mon_envmeasure_internal          ps0_volt_3_3;
**           ...
**                                             ...
**     } hw_some_hw_structure;
**
**
**
**   B.  Initialize the structure.
**
**     struct hw_some_hw_structure some_data = {
**
**         .cn = {                  # Init hw_mon_envdevice_internal
**           .parent_device = NULL, # Parent device for top level must be NULL
**           .dev = {               # Init env_device
**             .id         = 0,     # Unique 0 id for device in this set
**             .type       = EDT_CONTROLLER,    # device type is a controller
**             .status     = ES_NA, # status = ES_NA  (status will be established in monitor)
**             .num_msr    = 2,     # number of measurements attached
**             .num_dev    = 4,     # number of subdevices attached
**             .length     = sizeof(hw_some_hw_structure),  # length of device (static + dynamic)
**             .name       = "controller",  # string device name (top level overide by init function)
**             .flags      = EDF_FRU,   # Set the FRU bit. (this device is field repleacable)
**             .dyn_off    = HWMON_INT_DV_DYN,  # offset from start of device to dynamic data
**           },
**         },
**
**           .cn_tempc = {          # Init hw_mon_envmeasure_internal
**             .msr = {             # Init env_measure
**               .id         = 0,   # Unique 0 id for measure in this set
**               .status     = ES_NA,   # status = ES_NA  (status will be established in monitor)
**               .type       = EMT_TEMP,    # temperature measurement
**               .length     = HWMON_INT_MS_LN, # length of measure (static + dynamic)
**               .name       = "temperature",   # string measure name
**               .dyn_off    = HWMON_INT_MS_DYN,    # offset from start of measure to dynamic data
**             },
**
**    # The access and data play together hand in hand for initial setup.
**    # Here are the rules
**    #
**    # 1. If there is no sys filesystem paths for the hysteresis (.access.X)
**    #    and the corresponding hysteresis (.data.X) is EGDV_UNKNOWN, The
**    #    corresponding hysteresis will not be monitored.
**    #
**    # 2. If there is no sys filesystem paths for the hysteresis (.access.X)
**    #    and the corresponding hysteresis (.data.X) is not EGDV_UNKNOWN, the
**    #    hysteresis (.data.X) will be used to monitor the input read
**    #    (.data.data).
**    #
**    # 3. If there is a sys filesystem paths for the hysteresis (.access.X)
**    #    and the corresponding hysteresis (.data.X) is EGDV_UNKNOWN, the
**    #    hysteresis (.access.X) will be read from the sys filesystem and
**    #    used to monitor the input read (.data.data).
**    #
**    # 4. If there is a sys filesystem paths for the hysteresis (.access.X) and the corresponding
**    #    hysteresis (.data.X) is not EGDV_UNKNOWN, the hysteresis (.access.X) will be read from the
**    #    sys filesystem and compared with the hysteresis (.data.X).  If the two are not equal, hysteresis
**    #    (.access.X) will be set to the value of hysteresis (.data.X) and
**    #    used to monitor (.data.data).
**
**             .access = {                      # Init hw_mon_envmeasure_access
**               .parent_device = &ed_750_data.cn,  # set parent device to device owning measure
**               .data          = "/sys/bus/i2c/devices/0-002f/temp3_input",     # path to sys fs to read input data
**               .max_error     = "/sys/bus/i2c/devices/0-002f/temp3_max",       # path to sys fs to read max_error
**               .max_warn      = "/sys/bus/i2c/devices/0-002f/temp3_max_hyst",  # path to sys fs to read max_warn
**               .min_error     = "\0", # path to sys fs to read min_error
**               .min_warn      = "\0", # path to sys fs to read min_error
**               .calc_wrt_fmt  = "%ld * 10",   # calculation to apply before writing
**               .calc_rd_fmt   = "%ld / 10",   # calculation to apply after reading
**             },
**             .data = {                        # Init env_generic_measure_data
**               .data       = EGDV_UNKNOWN,    # input data, must have corresponding access
**               .max_error  = 15,              # write 15 to .access.max_error
**               .max_warn   = EGDV_UNKNOWN,    # read .access.max_warn for threshold
**               .min_error  = 5,               # monitor at 5 ( no .access.min_error to write)
**               .min_warn   = EGDV_UNKNOWN,    # Do not monitor this threshold (no .access.min_warn)
**             },
**           },
**
**           .cn_volt_vcc5 = {
**             .msr = {
**               .id         = 1,
**               .status     = ES_NA,
**               .type       = EMT_VOLTAGE,
**               .length     = HWMON_INT_MS_LN,
**               .name       = "vcc 5v voltage",
**               .dyn_off    = HWMON_INT_MS_DYN,
**             },
**             .access = {
**               .parent_device    = &ed_750_data.cn,
**               .data             = "/sys/bus/i2c/devices/0-002f/in6_input",
**               .max_error        = "/sys/bus/i2c/devices/0-002f/in6_max",
**               .min_error        = "/sys/bus/i2c/devices/0-002f/in6_min",
**             },
**             .data = {
**               .data       = EGDV_UNKNOWN,
**               .max_error  = EGDV_UNKNOWN,
**               .max_warn   = EGDV_UNKNOWN,
**               .min_error  = EGDV_UNKNOWN,
**               .min_warn   = EGDV_UNKNOWN,
**             },
**           },
**         .cpu0 = {
**           .parent_device = &ed_750_data.cn,
**           .dev = {
**             .id         = 0,
**             .type       = EDT_CPU,
**             .status     = ES_NA,
**             .num_msr    = 2,
**             .num_dev    = 0,
**             .length     = HWMON_INT_DV_LN(0,2),
**             .name       = "cpu 0",
**             .flags      = 0,
**             .dyn_off    = HWMON_INT_DV_DYN,
**           },
**         },
**
**           .cpu0_tempc = {
**             .msr = {
**               .id         = 0,
**               .status     = ES_NA,
**               .type       = EMT_TEMP,
**               .length     = HWMON_INT_MS_LN,
**               .name       = "temperature",
**               .dyn_off    = HWMON_INT_MS_DYN,
**             },
**             .access = {
**               .parent_device = &ed_750_data.cpu0,
**               .data          = "/sys/bus/i2c/devices/0-002f/temp1_input",
**               .max_error     = "/sys/bus/i2c/devices/0-002f/temp1_max",
**               .max_warn      = "/sys/bus/i2c/devices/0-002f/temp1_max_hyst",
**             },
**             .data = {
**               .data       = EGDV_UNKNOWN,
**               .max_error  = EGDV_UNKNOWN,
**               .max_warn   = EGDV_UNKNOWN,
**               .min_error  = EGDV_UNKNOWN,
**               .min_warn   = EGDV_UNKNOWN,
**             },
**           },
**
**           .cpu0_voltage = {
**             .msr = {
**               .id         = 1,
**               .status     = ES_NA,
**               .type       = EMT_VOLTAGE,
**               .length     = HWMON_INT_MS_LN,
**               .name       = "voltage",
**               .dyn_off    = HWMON_INT_MS_DYN,
**             },
**             .access = {
**               .parent_device    = &ed_750_data.cpu0,
**               .data             = "/sys/bus/i2c/devices/0-002f/in0_input",
**               .max_error        = "/sys/bus/i2c/devices/0-002f/in0_max",
**               .min_error        = "/sys/bus/i2c/devices/0-002f/in0_min",
**             },
**             .data = {
**               .data       = EGDV_UNKNOWN,
**               .max_error  = EGDV_UNKNOWN,
**               .max_warn   = EGDV_UNKNOWN,
**               .min_error  = EGDV_UNKNOWN,
**               .min_warn   = EGDV_UNKNOWN,
**             },
**           },
**
**         .fan0 = {
**           .parent_device = &ed_750_data.cn,
**           .dev = {
**             .id         = 0,
**             .type       = EDT_FAN,
**             .status     = ES_NA,
**             .num_msr    = 1,
**             .num_dev    = 0,
**             .length     = HWMON_INT_DV_LN(0,1),
**             .name       = "fan 0",
**             .flags      = 0,
**             .dyn_off    = HWMON_INT_DV_DYN,
**           },
**         },
**
**           .fan0_rpm = {
**             .msr = {
**               .id         = 0,
**               .status     = ES_NA,
**               .type       = EMT_RPM,
**               .length     = HWMON_INT_MS_LN,
**               .name       = "rpm",
**               .dyn_off    = HWMON_INT_MS_DYN,
**             },
**             .access = {
**               .parent_device    = &ed_750_data.fan0,
**               .data             = "/sys/bus/i2c/devices/0-002f/fan1_input",
**               .min_error        = "/sys/bus/i2c/devices/0-002f/fan1_min",
**             },
**             .data = {
**               .data       = EGDV_UNKNOWN,
**               .max_error  = EGDV_UNKNOWN,
**               .max_warn   = EGDV_UNKNOWN,
**               .min_error  = EGDV_UNKNOWN,
**               .min_warn   = EGDV_UNKNOWN,
**             },
**           },
**
**         .fan1 = {
**           .parent_device = &ed_750_data.cn,
**           .dev = {
**             .id         = 1,
**             .type       = EDT_FAN,
**             .status     = ES_NA,
**             .num_msr    = 1,
**             .num_dev    = 0,
**             .length     = HWMON_INT_DV_LN(0,1),
**             .name       = "fan 1",
**             .flags      = EDF_FRU,
**             .dyn_off    = HWMON_INT_DV_DYN,
**           },
**         },
**
**           .fan1_rpm = {
**             .msr = {
**               .id         = 0,
**               .status     = ES_NA,
**               .type       = EMT_RPM,
**               .length     = HWMON_INT_MS_LN,
**               .name       = "rpm",
**               .dyn_off    = HWMON_INT_MS_DYN,
**             },
**             .access = {
**               .parent_device    = &ed_750_data.fan1,
**               .data             = "/sys/bus/i2c/devices/0-002f/fan2_input",
**               .min_error        = "/sys/bus/i2c/devices/0-002f/fan2_min",
**             },
**             .data = {
**               .data       = EGDV_UNKNOWN,
**               .max_error  = EGDV_UNKNOWN,
**               .max_warn   = EGDV_UNKNOWN,
**               .min_error  = EGDV_UNKNOWN,
**               .min_warn   = EGDV_UNKNOWN,
**             },
**           },
**
**
**         .ps0 = {
**           .parent_device = &ed_750_data.cn,
**           .dev = {
**             .id         = 0,
**             .type       = EDT_POWERSUPPLY,
**             .status     = ES_NA,
**             .num_msr    = 2,
**             .num_dev    = 0,
**             .length     = HWMON_INT_DV_LN(0,2),
**             .name       = "power supply 0",
**             .flags      = EDF_FRU,
**             .dyn_off    = HWMON_INT_DV_DYN,
**           },
**         },
**
**           .ps0_temp = {
**             .msr = {
**               .id         = 0,
**               .status     = ES_NA,
**               .type       = EMT_TEMP,
**               .length     = HWMON_INT_MS_LN,
**               .name       = "temperature",
**               .dyn_off    = HWMON_INT_MS_DYN,
**             },
**             .access = {
**               .parent_device    = &ed_750_data.ps0,
**               .calc_rd_fmt      = "1000 * %ld",
**               .data             = "/sys/bus/i2c/devices/0-002d/temperature",
**             },
**             .data = {
**               .data       = EGDV_UNKNOWN,
**               .max_error  = EGDV_UNKNOWN,
**               .max_warn   = EGDV_UNKNOWN,
**               .min_error  = EGDV_UNKNOWN,
**               .min_warn   = EGDV_UNKNOWN,
**             },
**           },
**
**           .ps0_volt_3_3 = {
**             .msr = {
**               .id         = 1,
**               .status     = ES_NA,
**               .type       = EMT_VOLTAGE,
**               .length     = HWMON_INT_MS_LN,
**               .name       = "3.3v voltage",
**               .dyn_off    = HWMON_INT_MS_DYN,
**             },
**             .access = {
**               .parent_device    = &ed_750_data.ps0,
**               .calc_rd_fmt      = "7.8125 * %ld * 2",
**               .data             = "/sys/bus/i2c/devices/0-002d/voltage_3_3",
**             },
**             .data = {
**               .data       = EGDV_UNKNOWN,
**               .max_error  = EGDV_UNKNOWN,
**               .max_warn   = EGDV_UNKNOWN,
**               .min_error  = EGDV_UNKNOWN,
**               .min_warn   = EGDV_UNKNOWN,
**             },
**           },
** </PRE>
**/

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/**
**  @ingroup _HW_MODULE_HWMON_INTERNAL_
**  @defgroup _HW_MODULE_HWMON_INTERNAL_CONSTANTS Hardware Monitor Constants
**
**  @brief  Constants used in the Hardware Monitor Module.
**/

/**
**  @ingroup _HW_MODULE_HWMON_INTERNAL_CONSTANTS
**  @defgroup _HW_MODULE_HWMON_INTERNAL_CONSTANTS_MISC Miscellaneous Constants
**
**  @brief      Miscellaneous Constants.
**
**  @{
**/
#define HW_HWMON_FREQ               10  /**< monitor frequency in seconds   */
#define HW_HWMON_MAX_FPATH          128 /**< max path and file name.    */
#define HW_HWMON_MAX_EQUAT          64  /**< max equation format string.    */
/* @} */

/**
**  @ingroup _HW_MODULE_HWMON_INTERNAL_CONSTANTS
**  @defgroup _HW_MODULE_HWMON_INTERNAL_CONSTANTS_EMF Measure Flag Constants
**
**  @brief      These are the measure flags as reported
**              in struct hw_mon_envmeasure_access.
**
**  @{
**/
enum env_measure_flags
{
    EMF_IGN_FFAIL   = 0x00000001,   /**< do not log failure until a good status is reported */
    EMF_STATUS_ONLY = 0x00000002,   /**< report a GOOD/ERROR status only for device/measure */
    EMF_DEBUG_ONLY  = 0x80000000,   /**< Device/Measure Debug only flag (do not report/log debug */
};
/* @} */

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/

/**
**  @ingroup _HW_MODULE_HWMON_INTERNAL_
**  @defgroup _HW_MODULE_HWMON_INTERNAL_MACROS Hardware Monitor Macros
**
**  @brief      Macros used with the Hardware Monitor Module.
**/

/**
**  @ingroup _HW_MODULE_HWMON_INTERNAL_MACROS
**  @defgroup _HW_MODULE_HWMON_INTERNAL_MACROS_MISC Hardware Monitor Misc Macros
**
**  @brief      Miscellaneous Macros
**
**  @{
**/
/** Macro for calculating an internal measure length */
#define HWMON_INT_MS_LN         (sizeof(hw_mon_envmeasure_internal))

/** Macro for calculating an internal measure dynamic offset */
#define HWMON_INT_MS_DYN        (sizeof(hw_mon_envmeasure_internal)-sizeof(env_generic_measure_data))

/** Macro for calculating an internal device length */
#define HWMON_INT_DV_LN(d,m)    ((sizeof(hw_mon_envdevice_internal)) + \
                                 (sizeof(hw_mon_envmeasure_internal) * (m)) + \
                                 (sizeof(hw_mon_envdevice_internal) * (d)))

/** Macro for calculating an internal device dynamic offset */
#define HWMON_INT_DV_DYN        (sizeof(hw_mon_envdevice_internal))


/** Macros to aid in filling out data structure */

#define MSR_DEF(i,n,t)   .id = i, .status = ES_NA, .type = t,     \
    .length = HWMON_INT_MS_LN, .name = n, .dyn_off = HWMON_INT_MS_DYN

#define MSR_TEMP(i,n)   { MSR_DEF(i, n, EMT_TEMP) }
#define MSR_VOLTAGE(i,n)    { MSR_DEF(i, n, EMT_VOLTAGE) }
#define MSR_RPM(i,n)    { MSR_DEF(i, n, EMT_RPM) }
#define MSR_COUNT(i,n)  { MSR_DEF(i, n, EMT_COUNT) }
#define MSR_STATE(i,n)  { MSR_DEF(i, n, EMT_STATE) }

#define DEV(i,t,m,n,f)  .parent_device = &HW_DATA.cn,               \
    .dev = { .id = i, .type = t, .status = ES_NA, .num_msr = m,     \
        .num_dev = 0, .length = HWMON_INT_DV_LN(0,m), .name = n,    \
        .flags = f, .dyn_off = HWMON_INT_DV_DYN }

#define DATA_NOERROR    { .max_error = LLONG_MAX, .max_warn = LLONG_MAX,    \
    .min_error = LLONG_MIN, .min_warn = LLONG_MIN }

#define DATA_COUNT_ERROR    { .max_error = 1, .max_warn = LLONG_MAX, \
    .min_error = LLONG_MIN, .min_warn = LLONG_MIN }

#define DATA_COUNT_WARN { .max_error = LLONG_MAX, .max_warn = 1, \
    .min_error = LLONG_MIN, .min_warn = LLONG_MIN }

#define DATA_STATE_ERROR_FALSE { .max_error = LLONG_MAX, .min_error = 0,    \
    .max_warn = LLONG_MAX, .min_warn = LLONG_MIN }

#define CPU_DECL(n) hw_mon_envdevice_internal cpu##n;   \
    hw_mon_envmeasure_internal cpu##n##_tempc;

#define CPU_DEF(n,i,t)  .cpu##n = { DEV(i, EDT_CPU, 1, "cpu " #n, 0) }, \
    /* CPU Temp Celsius */                          \
    .cpu##n##_tempc = { .msr = MSR_TEMP(0, "temperature"),  \
        .access = { .parent_device = &HW_DATA.cpu##n,   \
            CPU_TEMP(t),                                \
        }, .data = DATA_NOERROR,                        \
    }

#define FAN_DECL(n) hw_mon_envdevice_internal fan##n;   \
    hw_mon_envmeasure_internal fan##n##_rpm;

#define FAN_DEF(n,i)  .fan##n = { DEV(i, EDT_FAN, 1, "fan " #n, EDF_FRU) }, \
    /* FAN measurements - rpm */                        \
    .fan##n##_rpm = { .msr = MSR_RPM(0, "rpm"),         \
        .access = { .parent_device = &HW_DATA.fan##n,   \
            .data       = SYS_FAN "/fan" #n "_input",   \
            .min_error  = SYS_FAN "/fan" #n "_min",     \
        }, .data = DATA_NOERROR,                        \
    }

#define EDAC_DECL(n)   hw_mon_envdevice_internal   mem##n;  \
    hw_mon_envmeasure_internal  ce##n;          \
    hw_mon_envmeasure_internal  ce_noinfo##n;   \
    hw_mon_envmeasure_internal  ue##n;          \
    hw_mon_envmeasure_internal  ue_noinfo##n;   \
    hw_mon_envmeasure_internal  size##n

#define EDAC_DEF(n,i)   .mem##n =                       \
    { DEV(i, EDT_MEMORY, 5, "memory", EMF_DEBUG_ONLY) },\
    .ce##n = { .msr = MSR_COUNT(0, "ce"),               \
        .access = { .parent_device = &HW_DATA.mem##n,   \
            .data   = SYS_EDAC "/mc/mc" #n "/ce_count", \
        }, .data = DATA_COUNT_WARN,                     \
    },                                                  \
    .ce_noinfo##n = { .msr = MSR_COUNT(1, "ce noinfo"), \
        .access = { .parent_device = &HW_DATA.mem##n,   \
            .data   = SYS_EDAC "/mc/mc" #n "/ce_noinfo_count",  \
        }, .data = DATA_COUNT_WARN,                     \
    },                                                  \
    .ue##n = { .msr = MSR_COUNT(2, "ue"),               \
        .access = { .parent_device = &HW_DATA.mem##n,   \
            .data   = SYS_EDAC "/mc/mc" #n "/ue_count", \
        }, .data = DATA_COUNT_WARN,                     \
    },                                                  \
    .ue_noinfo##n = { .msr = MSR_COUNT(3, "ue noinfo"), \
        .access = { .parent_device = &HW_DATA.mem##n,   \
            .data   = SYS_EDAC "/mc/mc" #n "/ue_noinfo_count",  \
        }, .data = DATA_COUNT_WARN,                     \
    },                                                  \
    .size##n = { .msr = MSR_COUNT(4, "size"),           \
        .access = { .parent_device = &HW_DATA.mem##n,   \
            .data   = SYS_EDAC "/mc/mc" #n "/size_mb",  \
        }, .data = DATA_NOERROR,                        \
    }

#define BUS_DECL(n) hw_mon_envdevice_internal   bus##n; \
    hw_mon_envmeasure_internal      parity##n

#define BUS_DEF(n,i)    .bus##n = { DEV(i, EDT_BUS, 1, "bus", 0) }, \
    .parity##n = { .msr = MSR_COUNT(0, "parity"),       \
        .access = { .parent_device = &HW_DATA.bus##n,   \
            .data   = SYS_EDAC "/pci/pci_parity_count", \
        }, .data = DATA_COUNT_WARN,                     \
    }

/* @} */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/**
**  @ingroup _HW_MODULE_HWMON_INTERNAL_
**  @defgroup _HW_MODULE_HWMON_INTERNAL_STRUCT Hardware Monitor Structures
**
**  @brief      Structures used in the Hardware Monitor Module.
**/

/**
**  @ingroup _HW_MODULE_HWMON_INTERNAL_STRUCT
**  @defgroup _HW_MODULE_HWMON_INTERNAL_STRUCT_DEVICE Internal Device Structure
**
**  @brief  This is what a device looks like internally to hw_mon.  This is what
**          a monitoring layout needs to use to represent a device.
**
**  @{
**/
/** Internal Device Structure */
typedef struct hw_mon_envdevice_internal
{
  /* Client Access - device dynamic offsets to "Client measures and subdevices" below   */
    env_device      dev;        /**< device structure               */

  /* Internal Access */
    struct hw_mon_envdevice_internal    *parent_device; /**< Pointer to parent device   */
    void            *dlock;     /**< Device lock                    */

  /* Client measures and subdevices     */
  /*   measures     */
  /*   subdevices   */
} hw_mon_envdevice_internal;
/* @} */

/**
**  @ingroup _HW_MODULE_HWMON_INTERNAL_STRUCT
**  @defgroup _HW_MODULE_HWMON_INTERNAL_STRUCT_MEASURE_ACCESS  Internal Measure Access Structure
**
**  @brief  This structure is a partial mirror of the env_generic_measure_data
**          structure, providing sys fs paths to the data to be read or written
**
**  @{
**/

typedef int64_t (*calcfn)(int64_t);

/** Internal Measure Access Structure */
typedef struct hw_mon_envmeasure_access
{
    const char *data;       /**< sys fs string to input data */
    const char *max_error;  /**< sys fs string to max_error */
    const char *max_warn;   /**< sys fs string to max_warn  */
    const char *min_error;  /**< sys fs string to min_error */
    char    *min_warn;      /**< sys fs string to min_warn  */
    calcfn  calc_wr;
    calcfn  calc_rd;
    hw_mon_envdevice_internal   *parent_device; /**< Pointer to owning parent */
    uint32_t    flags;      /**< Internal flags             */
    uint32_t    count;      /**< Status change count for deglitching */
} hw_mon_envmeasure_access;
/* @} */

/**
**  @ingroup _HW_MODULE_HWMON_INTERNAL_STRUCT
**  @defgroup _HW_MODULE_HWMON_INTERNAL_STRUCT_MEASURE Internal Measure Structure
**
**  @brief  This is what a measure looks like internally to hw_mon.  This is what
**          a monitoring layout needs to use to represent a measure.
**
**  @{
**/
/** Internal Measure Structure */
typedef struct hw_mon_envmeasure_internal
{
  /* Client Access - measure dynamic offsets to "Client data" below         */
    env_measure                 msr;        /**< measure structure          */

  /* Internal Access                                                        */
    hw_mon_envmeasure_access    access;     /**< internal access structure  */

  /* Client data                                                            */
    env_generic_measure_data    data;       /**< client data                */
} hw_mon_envmeasure_internal;
/* @} */

/*
******************************************************************************
** Public variables
******************************************************************************
*/


/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
/* #pragma pack(pop) */
struct VOLTAGE_INPUT_READING_STRUCT;
struct TEMPERATURE_STATUS_STRUCT;

void update_voltage_reading(struct VOLTAGE_INPUT_READING_STRUCT *r,
                env_generic_measure_data *m);
void update_cpu_temperature(struct TEMPERATURE_STATUS_STRUCT *ts,
                env_generic_measure_data *m);
int sensor_valid(hw_mon_envmeasure_internal *s);

#endif /* LOG_SIMULATOR */

#endif /* __HW_MON_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
