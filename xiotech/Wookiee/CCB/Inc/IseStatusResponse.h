/* $Id: IseStatusResponse.h 163153 2014-04-11 23:33:11Z marshall_midden $*/
#ifndef _ISESTATUSRESPONSE_H_
#define _ISESTATUSRESPONSE_H_

/* We want:      MAX_DISK_BAYS */
#include "XIO_Const.h"

/* We want:     IP_ADDRESS typedef. */
#include "XIO_Types.h"

//----------------------------------------------------------------------------
#define ASGSES_cmd_ISE_Beacon   10      /* Set Chassis beacon light for IP. */
#define ASGSES_cmd_MRC_Beacon0  11      /* Set MRC 0 beacon light for IP. */
#define ASGSES_cmd_MRC_Beacon1  12      /* Set MRC 1 beacon light for IP. */
#define ASGSES_cmd_PS_Beacon0   13      /* Set PowerSupply 0 beacon light for IP. */
#define ASGSES_cmd_PS_Beacon1   14      /* Set PowerSupply 1 beacon light for IP. */
#define ASGSES_cmd_Bat_Beacon0  15      /* Set Battery 0 beacon light for IP. */
#define ASGSES_cmd_Bat_Beacon1  16      /* Set Battery 1 beacon light for IP. */
#define ASGSES_cmd_DP_Beacon0   17      /* Set DataPac 0 beacon light for IP. */
#define ASGSES_cmd_DP_Beacon1   18      /* Set DataPac 1 beacon light for IP. */
#define ASGSES_cmd_SFP_Beacon0  19      /* Set SFP 0 beacon light for IP. */
#define ASGSES_cmd_SFP_Beacon1  20      /* Set SFP 1 beacon light for IP. */
#define ASGSES_cmd_CAP_Beacon0  21      /* Set CAP 0 beacon light for IP. */
#define ASGSES_cmd_CAP_Beacon1  22      /* Set CAP 1 beacon light for IP. */
#define ASGSES_cmd_BEZEL_Beacon 23      /* Set BEZEL beacon light for IP. */

//----------------------------------------------------------------------------
// This is for Version 0/1 of the PI ISE status, and must not change.

/*
 * The statistics data structure for an ISE Controller.
 */

enum status_ise_component       // Possibilites for controller_status.
{
    OPERATIONAL = 1,
    WARNING = 2,
    CRITICAL = 3,
    UNINITIALIZED = 4,
    CHANGING = 5,
    NONOPERATIONAL = 6
};

struct ise_controller           // LOCKED, must never change.
{
    char        controller_model[16];   /* Model.               'STFRU13CC' */
    char        controller_serial_number[12];   /* Serial number.       '2BBC0247' */
    char        controller_part_number[16];     /* Part number.         'STFRU13CC' */
    char        controller_hw_version[4];       /* Revision.            'A' */
//  char                manufacture_date[]; Lets not do this. Long, semi-unknown size.
//  xwstype__DevicePosition1 *position; Lets not do this, x,y,z positioning - yuck.
//  enum xwstype__CtrlrPortType1 fc_USCOREport_USCOREtype; Lets not do this.
    INT64       controller_wwn; /* fc_port_id.  WWN of controller. */

/* Controller configuration. */
    IP_ADDRESS  ip;             /* IP of controller. */
    IP_ADDRESS  gateway;        /* IP of gateway. */
    IP_ADDRESS  subnet_mask;    /* Subnet mask. */
    UINT8       controller_fc_port_speed_setting;       /* Speed of fc for this controller. */
    UINT8       controller_beacon;      /* Is beacon light on? */

/* Controller Status. */
    UINT8       controller_rank;        /* rank 1 or 2, primary/secondary. */
    UINT8       controller_status;      /* Status of MRC (controller). */
    /* I propose a bit field for the array of enums. */
    UINT64      controller_status_details;
    char        controller_fw_version[16];      /* firmware version.    'V1.0 (RC1.7)' */
    UINT8       controller_fc_port_status;      /* Status of fibre channel port. */
    UINT8       controller_fc_port_speed;       /* Speed of fibre channel. */
    UINT8       controller_ethernet_link_up;    /* True if ethernet link active. */
    char        controller_mac_address[18];     /* Ethernet mac address. */

/* Controller Stats1. */
// Does not seem to return anything useful.

/* Controller Environmental. */
// eeproms is empty.
    INT16       controller_temperature; /* Convert from float to signed short. */
} __attribute__ ((packed));


//----------------------------------------------------------------------------
// This is for Version 0/1 of the PI ISE status, and must not change.

/*
 * The statistics data structure for an ISE DataPac.
 */
struct ise_datapac              // LOCKED, must never change.
{

/* DataPac Configuration. */
    UINT8       datapac_beacon; /* Beacon light on/off. */

/* DataPac Information. */
    UINT8       datapac_type;   /* Type of pack, 1=unknown, 2= reserved,
                                 * 3=10pack, 4=20pack. */
    char        datapac_serial_number[12];      /* Serial number. */
    char        datapac_model[16];      /* Model. */
    char        datapac_part_number[16];        /* Part number. */
    INT64       datapac_spare_level;    /* Spare level. */
//  time_t      datapac_manufacture_date;
//  xwstype__DevicePosition1 *position;

/* DataPac Status. */
    UINT8       datapac_status; /* Status of datapac. */
    /* I propose a bit field for the array of enums. */
    UINT64      datapac_status_details;
    INT64       datapac_capacity;
    char        datapac_fw_version[16]; /* firmware version. */

/* DataPac Environmental. */
    INT16       datapac_temperature;    /* Convert from float to signed short. */
    UINT8       datapac_health; /* Percent. */
} __attribute__ ((packed));


//----------------------------------------------------------------------------
// This is for Version 0/1 of the PI ISE status, and must not change.

/*
 * The statistics data structure for an ISE PowerSypply.
 */
struct ise_powersupply          // LOCKED, must never change.
{

/* PowerSupply Configuration. */
    UINT8       powersupply_beacon;     /* Beacon light on/off. */

/* PowerSupply Information. */
    char        powersupply_model[16];  /* Model. */
    char        powersupply_serial_number[12];  /* Serial number. */
    char        powersupply_part_number[16];    /* Part number. */
//  time_t      powersupply_manufacture_date;
//  xwstype__DevicePosition1 *position;

/* PowerSupply Status. */
    UINT8       powersupply_status;     /* Status of powersupply. */
    /* I propose a bit field for the array of enums. */
    UINT64      powersupply_status_details;

/* PowerSupply Environmental. */
    UINT8       powersupply_fan1_status;        /* Status of powersupply fan. */
    INT64       powersupply_fan1_speed; /* Speed of powersupply fan. */
    UINT8       powersupply_fan2_status;        /* Status of powersupply fan. */
    INT64       powersupply_fan2_speed; /* Speed of powersupply fan. */
    INT16       powersupply_temperature;        /* Convert from float to signed short. */
} __attribute__ ((packed));


//----------------------------------------------------------------------------
// This is for Version 0/1 of the PI ISE status, and must not change.

/*
 * The statistics data structure for an ISE Battery.
 */
struct ise_battery              // LOCKED, must never change.
{

/* Battery Configuration. */
    UINT8       battery_beacon; /* Beacon light on/off. */

/* Battery Information. */
    char        battery_model[16];      /* Model. */
    char        battery_serial_number[12];      /* Serial number. */
    char        battery_part_number[16];        /* Part number. */
//  time_t      battery_manufacture_date;
    char        battery_type[16];       /* Type of battery. (Lead Acid) */
//  xwstype__DevicePosition1 *position;

/* Battery Status. */
    UINT8       battery_status; /* Status of battery. */
    /* I propose a bit field for the array of enums. */
    UINT64      battery_status_details;
// bool ups_USCOREmode;
    INT64       battery_remaining_charge;       /* Remaining charge. */
    INT64       battery_max_charge;     /* Maximum charge. */
    INT64       battery_max_charge_capacity;    /* Maximum charge capacity. */
    INT64       battery_min_holdup_time;        /* Min holdup time. */
// xwstype__BatteryCalibration1 *calibration;

/* Battery Environmental. */
    UINT8       battery_charger_state;  /* Status of battery charger. */
    UINT8       battery_charger_state_details;  /* Detailed Status of battery charger. */
} __attribute__ ((packed));


//----------------------------------------------------------------------------
// This is for Version 0/1 of the PI ISE status, and must not change.

/*
 * The statistics data structure for an ISE.
 */

typedef struct ise_info         // LOCKED, must never change.
{
    UINT8       Protocol_Version_Level; /* Which form of this data structure. */
    // which_tcp_connections has a bit set for each MRC that we can connect to
    // ethernet and tcp/ip. If a position is disconnected, check MRC and cabling.
    // This is created on the platform controller by program ASGSES.
    UINT8       which_tcp_connections;  /* bit field, set bit(s) 0 and/or 1 if present. */
    // which_controllers has a bit set for each MRC that has information available
    // via IWS on the ISE (brick). This is ISE generated. Same for rest below.
    // Note: The array structures for each will contain zeros, or whatever the
    // last state set -- if the device is no longer up via following bit fields.
    UINT8       which_controllers;      /* bit field, set bit(s) 0 and/or 1 if present. */
    UINT8       which_datapacs; /* bit field, set bit(s) 0 and/or 1 if present. */
    UINT8       which_powersupplies;    /* bit field, set bit(s) 0 and/or 1 if present. */
    UINT8       which_batteries;        /* bit field, set bit(s) 0 and/or 1 if present. */
    IP_ADDRESS  ip1;            /* IP of first controllers in chassis. */
    IP_ADDRESS  ip2;            /* IP of second controllers in chassis. */
    INT64       iws_ise_id;     /* The identifier of the chassis. */

/* Chassis data. */
    INT64       chassis_wwn;    /* identifier (wwn)     '20000014C3671E50' */
    char        chassis_serial_number[12];      /* Serial number.       '1BC1004a'  */
    char        chassis_model[16];      /* model.               'ST000000FC      ' */
    char        chassis_part_number[16];        /* Part number.         ''  --i.e. empty. */
    char        chassis_vendor[8];      /* Vendor               'SEAGATE ' */
    char        chassis_manufacturer[8];        /* Manufacturer         'Seagate' */
    char        chassis_product_version[4];     /* Product_version.     'A' */
    INT64       spare_level;    /* Spare level.         20 */

/* Chassis Configuration data that may change follows. */
    UINT8       chassis_auto_connect_enable;    /* ??? Unknown what this means! Do we need/want it? */
    UINT8       chassis_beacon; /* Is beacon light on? */

/* Chassis status data. */
    UINT8       chassis_status; /* enum: Operational=1, Warning=2, Critical=3,
                                 * Uninitialized=4, Changing=5, NON-operational=6 */
    /* I propose a bit field for the array of enums. 32 implies a 64 bit field. */
    UINT64      chassis_status_details; /* enum: NONE=1, COMPONENT_NOT_PRESENT = 2,
                                         * COMPONENT_OFFLINE = 3, VITAL_PRODUCT_DATA_CORRUPT = 4, VITAL_PRODUCT_DATA_UNKNOWN_TYPE = 5,
                                         * VITAL_PRODUCT_DATA_UNKNOWN_VERSION = 6, VITAL_PRODUCT_DATA_BAD_STATE = 7,
                                         * VITAL_PRODUCT_DATA_LOADED = 8, VITAL_PRODUCT_DATA_PERSISTENT_FAULT = 9,
                                         * MAINTENANCE_MODE = 10, COMPONENT_DEGRADED = 11, TEMPERATURE_OUT_OF_RANGE = 12,
                                         * UNINITIALIZED_NOT_READY = 13, INITIALIZED_NOT_OPERATIONAL = 14,
                                         * DISPLAY_CARD_NOT_PRESENT = 15, DISPLAY_CARD_FAILURE = 16, SYSTEM_METADATA_ERROR = 17,
                                         * CACHE_MEMORY_ERROR = 18, SYSTEM_METADATA_CONVERSION_ERROR = 19, DEGRAGED_BATTERY = 20,
                                         * ALL_DATAPACS_DEGRADED = 21, SYSTEM_METADATA_MISMATCH_CODE_0 = 22,
                                         * SYSTEM_METADATA_MISMATCH_CODE_1 = 23, SYSTEM_METADATA_MISMATCH_CODE_2 = 24,
                                         * SYSTEM_METADATA_MISMATCH_CODE_3 = 25, SYSTEM_METADATA_MISMATCH_CODE_4 = 26,
                                         * MRC_INOPERATIVE = 27, CACHE_INCONSISTENCY = 28, DIAGNOSTIC_MODE = 29,
                                         * DATAPAC_CONFIGURATION_CONFLICT = 30, MRC_CONFIGURATION_CONFLICT = 31,
                                         * DATAPAC_CONFIGURATION_CONFLICT_RECOVERABLE = 32 */

/* Chassis stat1 data. */
    time_t      chassis_uptime;
    time_t      chassis_current_date_time;
#if 0
// 2008-02-05 -- the ISE is not returning performance data -- NULL pointer.
    bool        chassis_performance_valid;      /* Indicates performance information is valid. */
    INT64       chassis_total_iops;
    INT64       chassis_read_iops;
    INT64       chassis_write_iops;
    INT64       chassis_total_kbps;
    INT64       chassis_read_kbps;
    INT64       chassis_write_kbps;
    INT64       chassis_read_latency;
    INT64       chassis_write_latency;
    INT64       chassis_queue_depth;
    INT64       chassis_read_percent;
    INT64       chassis_avg_bytes_transferred;
#endif                          /* 0 */

/* Chassis environmental data. */
    INT16       chassis_temperature_sensor;     /* Convert from float to signed short. */

/* Statistics for each controller. */
    struct ise_controller ctrlr[2];

/* Statistics for each DataPac. */
    struct ise_datapac datapac[2];

/* Statistics for each PowerSypply. */
    struct ise_powersupply powersupply[2];

/* Statistics for each Battery. */
    struct ise_battery battery[2];
} __attribute__ ((packed)) ise_info;                       // LOCKED, must never change.

//----------------------------------------------------------------------------
// This is for Version 0/1 of the PI ISE status, and must not change.

/*
* The max array we are caching.
*/
typedef struct ise_stats        // LOCKED, must never change.
{
    UINT16      bayid;          /* ise Bay id */
    UINT16      rsvd;           /* reserved   */
    ise_info    ise_bay_info;   /* ise_info   */
} __attribute__ ((packed)) ise_stats;                      // LOCKED, must never change.



//============================================================================
// Version 2 of the PI ISE status follows. It is not locked with PI.
//============================================================================

/*
 * The statistics data structure for an ISE Controller.
 */
struct ise_controller_version_2
{
    char        controller_model[16];   /* Model.               'STFRU13CC' */
    char        controller_serial_number[12];   /* Serial number.       '2BBC0247' */
    char        controller_part_number[16];     /* Part number.         'STFRU13CC' */
    char        controller_hw_version[4];       /* Revision.            'A' */
//  char                manufacture_date[]; Lets not do this. Long, semi-unknown size.
    INT64       controller_x_position;  /* Position is x,y,z coordinates. */
    INT64       controller_y_position;
    INT64       controller_z_position;
//  enum xwstype__CtrlrPortType1 fc_USCOREport_USCOREtype; Lets not do this.
//  enum xwstype__CtrlrPortType1 fc_USCOREport_USCOREtype; Lets not do this.
    INT64       controller_wwn; /* fc_port_id.  WWN of controller. */

/* Controller configuration. */
    IP_ADDRESS  ip;             /* IP of controller. */
    IP_ADDRESS  gateway;        /* IP of gateway. */
    IP_ADDRESS  subnet_mask;    /* Subnet mask. */
    UINT8       controller_fc_port_speed_setting;       /* Speed of fc for this controller. */
    UINT8       controller_beacon;      /* Is beacon light on? */

/* Controller Status. */
    UINT8       controller_rank;        /* rank 1 or 2, primary/secondary. */
    UINT8       controller_status;      /* Status of MRC (controller). */
    /* I propose a bit field for the array of enums. */
    UINT64      controller_status_details;
    char        controller_fw_version[28];      /* firmware version.    'V1.0 (RC1.7)' */
    UINT8       controller_fc_port_status;      /* Status of fibre channel port. */
    UINT8       controller_fc_port_speed;       /* Speed of fibre channel. */
    UINT8       controller_ethernet_link_up;    /* True if ethernet link active. */
    char        controller_mac_address[18];     /* Ethernet mac address. */

/* Controller Stats1. */
// Does not seem to return anything useful.

/* Controller Environmental. */
// eeproms is empty.
    INT16       controller_temperature; /* Convert from float to signed short. */
} __attribute__ ((packed));


/*
 * The statistics data structure for an ISE DataPac.
 */
struct ise_datapac_version_2
{

/* DataPac Configuration. */
    UINT8       datapac_beacon; /* Beacon light on/off. */

/* DataPac Information. */
    UINT8       datapac_type;   /* Type of pack, 1=unknown, 2= reserved,
                                 * 3=10pack, 4=20pack. */
    char        datapac_serial_number[12];      /* Serial number. */
    char        datapac_model[16];      /* Model. */
    char        datapac_part_number[16];        /* Part number. */
    INT64       datapac_spare_level;    /* Spare level. */
//  time_t      datapac_manufacture_date;
    INT64       datapac_x_position;     /* Position is x,y,z coordinates. */
    INT64       datapac_y_position;
    INT64       datapac_z_position;

/* DataPac Status. */
    UINT8       datapac_status; /* Status of datapac. */
    /* I propose a bit field for the array of enums. */
    UINT64      datapac_status_details;
    INT64       datapac_capacity;
    char        datapac_fw_version[16]; /* firmware version. */

/* DataPac Environmental. */
    INT16       datapac_temperature;    /* Convert from float to signed short. */
    UINT8       datapac_health; /* Percent. */
} __attribute__ ((packed));


/*
 * The statistics data structure for an ISE PowerSypply.
 */
struct ise_powersupply_version_2
{

/* PowerSupply Configuration. */
    UINT8       powersupply_beacon;     /* Beacon light on/off. */

/* PowerSupply Information. */
    char        powersupply_model[16];  /* Model. */
    char        powersupply_serial_number[12];  /* Serial number. */
    char        powersupply_part_number[16];    /* Part number. */
//  time_t      powersupply_manufacture_date;
    INT64       powersupply_x_position; /* Position is x,y,z coordinates. */
    INT64       powersupply_y_position;
    INT64       powersupply_z_position;

/* PowerSupply Status. */
    UINT8       powersupply_status;     /* Status of powersupply. */
    /* I propose a bit field for the array of enums. */
    UINT64      powersupply_status_details;

/* PowerSupply Environmental. */
    UINT8       powersupply_fan1_status;        /* Status of powersupply fan. */
    INT64       powersupply_fan1_speed; /* Speed of powersupply fan. */
    UINT8       powersupply_fan2_status;        /* Status of powersupply fan. */
    INT64       powersupply_fan2_speed; /* Speed of powersupply fan. */
    INT16       powersupply_temperature;        /* Convert from float to signed short. */
} __attribute__ ((packed));


/*
 * The statistics data structure for an ISE Battery.
 */
struct ise_battery_version_2
{

/* Battery Configuration. */
    UINT8       battery_beacon; /* Beacon light on/off. */

/* Battery Information. */
    char        battery_model[16];      /* Model. */
    char        battery_serial_number[12];      /* Serial number. */
    char        battery_part_number[16];        /* Part number. */
//  time_t      battery_manufacture_date;
    char        battery_type[16];       /* Type of battery. (Lead Acid) */
    INT64       battery_x_position;     /* Position is x,y,z coordinates. */
    INT64       battery_y_position;
    INT64       battery_z_position;

/* Battery Status. */
    UINT8       battery_status; /* Status of battery. */
    /* I propose a bit field for the array of enums. */
    UINT64      battery_status_details;
// bool ups_USCOREmode;
    INT64       battery_remaining_charge;       /* Remaining charge. */
    INT64       battery_max_charge;     /* Maximum charge. */
    INT64       battery_max_charge_capacity;    /* Maximum charge capacity. */
    INT64       battery_min_holdup_time;        /* Min holdup time. */
// xwstype__BatteryCalibration1 *calibration;

/* Battery Environmental. */
    UINT8       battery_charger_state;  /* Status of battery charger. */
    UINT8       battery_charger_state_details;  /* Detailed Status of battery charger. */
} __attribute__ ((packed));


/*
 * The statistics data structure for an ISE.
 */

struct ise_info_version_2
{
    UINT8       Protocol_Version_Level; /* Which form of this data structure. */
    // which_tcp_connections has a bit set for each MRC that we can connect to
    // ethernet and tcp/ip. If a position is disconnected, check MRC and cabling.
    // This is created on the platform controller by program ASGSES.
    UINT8       which_tcp_connections;  /* bit field, set bit(s) 0 and/or 1 if present. */
    UINT8       which_fw_index_type;    /* 0 = 0/1 index, 1 = 1/2 index type. */
    // Following four are for display purposes: such as MRC2, or MRC1. Possibly
    // MRC0 if there is really old firmware on the ISE.
    INT8        which_controllers[2];   /* index of two controllers, 0/1 or 1/2. */
    INT8        which_datapacs[2];      /* index of two datapacs, 0/1 or 1/2. */
    INT8        which_powersupplies[2]; /* index of two powersupplies, 0/1 or 1/2. */
    INT8        which_batteries[2];     /* index of two batteries, 0/1 or 1/2. */
    IP_ADDRESS  ip1;            /* IP of first controllers in chassis. */
    IP_ADDRESS  ip2;            /* IP of second controllers in chassis. */
    INT64       iws_ise_id;     /* The identifier of the chassis. */

/* Chassis data. */
    INT64       chassis_wwn;    /* identifier (wwn)     '20000014C3671E50' */
    char        chassis_serial_number[12];      /* Serial number.       '1BC1004a'  */
    char        chassis_model[16];      /* model.               'ST000000FC      ' */
    char        chassis_part_number[16];        /* Part number.         ''  --i.e. empty. */
    char        chassis_vendor[8];      /* Vendor               'SEAGATE ' */
    char        chassis_manufacturer[8];        /* Manufacturer         'Seagate' */
    char        chassis_product_version[4];     /* Product_version.     'A' */
    INT64       spare_level;    /* Spare level.         20 */

/* Chassis Configuration data that may change follows. */
    UINT8       chassis_beacon; /* Is beacon light on? */

/* Chassis status data. */
    UINT8       chassis_status; /* enum: Operational=1, Warning=2, Critical=3,
                                 * Uninitialized=4, Changing=5, NON-operational=6 */
    /* I propose a bit field for the array of enums. 32 implies a 64 bit field. */
    UINT64      chassis_status_details; /* enum: NONE=1, COMPONENT_NOT_PRESENT = 2,
                                         * COMPONENT_OFFLINE = 3, VITAL_PRODUCT_DATA_CORRUPT = 4, VITAL_PRODUCT_DATA_UNKNOWN_TYPE = 5,
                                         * VITAL_PRODUCT_DATA_UNKNOWN_VERSION = 6, VITAL_PRODUCT_DATA_BAD_STATE = 7,
                                         * VITAL_PRODUCT_DATA_LOADED = 8, VITAL_PRODUCT_DATA_PERSISTENT_FAULT = 9,
                                         * MAINTENANCE_MODE = 10, COMPONENT_DEGRADED = 11, TEMPERATURE_OUT_OF_RANGE = 12,
                                         * UNINITIALIZED_NOT_READY = 13, INITIALIZED_NOT_OPERATIONAL = 14,
                                         * DISPLAY_CARD_NOT_PRESENT = 15, DISPLAY_CARD_FAILURE = 16, SYSTEM_METADATA_ERROR = 17,
                                         * CACHE_MEMORY_ERROR = 18, SYSTEM_METADATA_CONVERSION_ERROR = 19, DEGRAGED_BATTERY = 20,
                                         * ALL_DATAPACS_DEGRADED = 21, SYSTEM_METADATA_MISMATCH_CODE_0 = 22,
                                         * SYSTEM_METADATA_MISMATCH_CODE_1 = 23, SYSTEM_METADATA_MISMATCH_CODE_2 = 24,
                                         * SYSTEM_METADATA_MISMATCH_CODE_3 = 25, SYSTEM_METADATA_MISMATCH_CODE_4 = 26,
                                         * MRC_INOPERATIVE = 27, CACHE_INCONSISTENCY = 28, DIAGNOSTIC_MODE = 29,
                                         * DATAPAC_CONFIGURATION_CONFLICT = 30, MRC_CONFIGURATION_CONFLICT = 31,
                                         * DATAPAC_CONFIGURATION_CONFLICT_RECOVERABLE = 32 */

/* Chassis stat1 data. */
    time_t      chassis_uptime;
    time_t      chassis_current_date_time;
    UINT8       chassis_performance_valid;      /* Indicates performance information is valid. */
    INT64       chassis_total_iops;
    INT64       chassis_read_iops;
    INT64       chassis_write_iops;
    INT64       chassis_total_kbps;
    INT64       chassis_read_kbps;
    INT64       chassis_write_kbps;
    INT64       chassis_read_latency;
    INT64       chassis_write_latency;
    INT64       chassis_queue_depth;
    INT64       chassis_read_percent;
    INT64       chassis_avg_bytes_transferred;

/* Chassis environmental data. */
    INT16       chassis_temperature_sensor;     /* Convert from float to signed short. */

/* Statistics for each controller. */
    struct ise_controller_version_2 ctrlr[2];

/* Statistics for each DataPac. */
    struct ise_datapac_version_2 datapac[2];

/* Statistics for each PowerSypply. */
    struct ise_powersupply_version_2 powersupply[2];

/* Statistics for each Battery. */
    struct ise_battery_version_2 battery[2];
} __attribute__ ((packed));

/*
* The max array we are caching.
*/
struct ise_stats_version_2
{
    UINT16      bayid;          /* ise Bay id */
    UINT16      rsvd;           /* reserved   */
    struct ise_info_version_2 ise_bay_info;     /* ise_info   */
} __attribute__ ((packed));

//----------------------------------------------------------------------------
#endif /* _ISESTATUSRESPONSE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/

/* End of file IseStatusResponse.h */
