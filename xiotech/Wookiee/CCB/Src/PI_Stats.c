/* $Id: PI_Stats.c 163153 2014-04-11 23:33:11Z marshall_midden $ */
/*===========================================================================
** FILE NAME:       PI_Stats.c
** MODULE TITLE:    Packet Interface for Stats and Enviromental commands
**
** DESCRIPTION:     Handler functions log request packets
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/

#include "CmdLayers.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "hw_mon.h"
#include "HWM.h"
#include "ipc_sendpacket.h"
#include "kernel.h"
#include "misc.h"
#include "MR_Defs.h"
#include "OS_II.h"
#include "PacketInterface.h"
#include "PI_CmdHandlers.h"
#include "PI_Stats.h"
#include "PI_Target.h"
#include "PI_Utils.h"
#include "PI_VDisk.h"
#include "quorum_utils.h"
#include "XIO_Std.h"
#include "X1_Structs.h"
#include "PktCmdHdl.h"

/*****************************************************************************
** Private defines
*****************************************************************************/

// The limited_copy() macro copies the minimum of src or dst into dst.
#define limited_copy(dst,src)   memcpy(&dst, &src, MIN(sizeof(dst), sizeof(src)))

/*****************************************************************************
** Public variables.
*****************************************************************************/

#if defined(MODEL_7000) || defined(MODEL_4700)
/* The number of ISE Bays gathering status for. */
UINT32      ISE_Number_Bays_Used = 0;
#endif /* MODEL_7000 || MODEL_4700 */

/*****************************************************************************
** Code Start
*****************************************************************************/

#ifdef ENABLE_NG_HWMON

/*----------------------------------------------------------------------------
** Function:    PI_EnvIIRequest
**
** Description: Request for Environmental data
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_EnvIIRequest(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    PI_ENV_II_GET_DATA_RSP *pdevice = NULL;

    /*
     * Call the function to get the env data.  The function will allocate the
     * memory and return the length that was allocated, and that needs to be
     * returned to the caller.
     */
    pRspPacket->pHeader->length = hw_hwmon_get_clean_data(&pdevice, NULL);

    if ((pRspPacket->pHeader->length > 0) && pdevice)
    {
        pRspPacket->pPacket = (UINT8 *)pdevice;
    }
    else
    {
        rc = PI_ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pPacket = NULL;
    }

    pRspPacket->pHeader->errorCode = 0;
    pRspPacket->pHeader->status = rc;

    return (rc);
}
#endif /* ENABLE_NG_HWMON */

/*----------------------------------------------------------------------------
** Function:    PI_EnvStatsRequest
**
** Description: Request for Environmental data
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_EnvStatsRequest(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    PI_STATS_ENVIRONMENTAL_RSP *rspDataPtr;

    /*
     * Allocate memory for the response packet data and fill in the data.
     */
    rspDataPtr = MallocWC(sizeof(*rspDataPtr));

    if (HWM_GetMonitorStatus(rspDataPtr) != GOOD)
    {
        rc = PI_ERROR;
    }

    /*
     * Copy the data into the transmit packet. Set the length of the data
     * packet and fill in the status and error code. Set the pointer to
     * the response data.
     * If an error occured, return no data.
     */
    if (rc == PI_GOOD)
    {
        pRspPacket->pHeader->length = sizeof(*rspDataPtr);
        pRspPacket->pPacket = (UINT8 *)rspDataPtr;
    }
    else
    {
        pRspPacket->pHeader->length = 0;
        pRspPacket->pPacket = NULL;
    }

    pRspPacket->pHeader->errorCode = 0;
    pRspPacket->pHeader->status = rc;

    return (rc);
}

#if defined(MODEL_7000) || defined(MODEL_4700)

/*---------------------------------------------------------------------------- */
void ISESESStatusHandle(ISE_SES_DEVICE *pise, struct ise_info_version_2 *ise)
{
    struct ise_stats_version_2 *pstats;

// fprintf(stderr, "handle status for bay[%d] ip: %x | %x\n", pise->PID, pise->ip1, pise->ip2);
// fprintf(stderr, "memcpy(&ise_data[%d], ise, sizeof(struct ise_info_version_2))\n", pise->PID);
    pstats = &(ise_data[pise->PID]);
    pstats->bayid = pise->PID;
    memcpy(&(pstats->ise_bay_info), ise, sizeof(struct ise_info_version_2));
}

/*----------------------------------------------------------------------------
** Function:    PI_ISEStatus_Copy_Version_0
**
** Description: Copy ISE status data into version 0 PI packet response.
**
** Inputs:      dest  - address to put ise data in response packet.
**              src   - pointer to ISE data from ASGSES program.
**
** Returns:     None.
**
**--------------------------------------------------------------------------*/
static void PI_ISEStatus_Copy_Version_0(UINT8 *dest, struct ise_stats_version_2 *src)
{
    UINT8       i;              // Simple 0, 1 array loop counter.
    UINT8       bf_c;           // Generate version 0 bit field for controllers.
    UINT8       bf_d;           // Generate version 0 bit field for datapacs.
    UINT8       bf_p;           // Generate version 0 bit field for powersupplies.
    UINT8       bf_b;           // Generate version 0 bit field for batteries.

    // dst is the location in the response packet for ISE data.
    ISE_BAY_STATUS_ITEM_0 *dst = (ISE_BAY_STATUS_ITEM_0 *)dest;

    /* Bay ID */
    dst->bayid = src->bayid;

    /* ISE chassis */
    // r_chassis is the response packet chassis information.
    struct ise_info *r_chassis;

    // i_chassis is the input ISE chassis information.
    struct ise_info_version_2 *i_chassis;

    r_chassis = &(dst->ise_bay_info);
    i_chassis = &(src->ise_bay_info);

    r_chassis->Protocol_Version_Level = i_chassis->Protocol_Version_Level;
    r_chassis->which_tcp_connections = i_chassis->which_tcp_connections;

    // Convert to bitfield. Account for 1/2 verses 0/1 index type.
    bf_c = (1 << i_chassis->which_controllers[0]) | (1 << i_chassis->which_controllers[1]);
    bf_d = (1 << i_chassis->which_datapacs[0]) | (1 << i_chassis->which_datapacs[1]);
    bf_p = (1 << i_chassis->which_powersupplies[0]) | (1 << i_chassis->which_powersupplies[1]);
    bf_b = (1 << i_chassis->which_batteries[0]) | (1 << i_chassis->which_batteries[1]);
    if (i_chassis->which_fw_index_type == 0)
    {
        bf_c = bf_c >> 1;
        bf_d = bf_d >> 1;
        bf_p = bf_p >> 1;
        bf_b = bf_b >> 1;
    }
    r_chassis->which_controllers = bf_c;
    r_chassis->which_datapacs = bf_d;
    r_chassis->which_powersupplies = bf_p;
    r_chassis->which_batteries = bf_b;

    r_chassis->ip1 = i_chassis->ip1;
    r_chassis->ip2 = i_chassis->ip2;
    r_chassis->iws_ise_id = i_chassis->iws_ise_id;
    r_chassis->chassis_wwn = i_chassis->chassis_wwn;
    limited_copy(r_chassis->chassis_serial_number, i_chassis->chassis_serial_number);
    limited_copy(r_chassis->chassis_model, i_chassis->chassis_model);
    limited_copy(r_chassis->chassis_part_number, i_chassis->chassis_part_number);
    limited_copy(r_chassis->chassis_vendor, i_chassis->chassis_vendor);
    limited_copy(r_chassis->chassis_manufacturer, i_chassis->chassis_manufacturer);
    limited_copy(r_chassis->chassis_product_version, i_chassis->chassis_product_version);
    r_chassis->spare_level = i_chassis->spare_level;
    r_chassis->chassis_auto_connect_enable = 0; // it was always 0 and undefined.
    r_chassis->chassis_beacon = i_chassis->chassis_beacon;
    r_chassis->chassis_status = i_chassis->chassis_status;
    r_chassis->chassis_status_details = i_chassis->chassis_status_details;
    r_chassis->chassis_uptime = i_chassis->chassis_uptime;
    r_chassis->chassis_current_date_time = i_chassis->chassis_current_date_time;
    r_chassis->chassis_temperature_sensor = i_chassis->chassis_temperature_sensor;

    /* MRC */
    // r_mrc is the response packet MRC information.
    struct ise_controller *r_mrc;

    // i_mrc is the input ISE MRC information.
    struct ise_controller_version_2 *i_mrc;

    for (i = 0; i < 2; i++)
    {
        r_mrc = &(dst->ise_bay_info.ctrlr[i]);
        i_mrc = &(src->ise_bay_info.ctrlr[i]);

        limited_copy(r_mrc->controller_model, i_mrc->controller_model);
        limited_copy(r_mrc->controller_serial_number, i_mrc->controller_serial_number);
        limited_copy(r_mrc->controller_part_number, i_mrc->controller_part_number);
        limited_copy(r_mrc->controller_hw_version, i_mrc->controller_hw_version);
        r_mrc->controller_wwn = i_mrc->controller_wwn;
        r_mrc->ip = i_mrc->ip;
        r_mrc->gateway = i_mrc->gateway;
        r_mrc->subnet_mask = i_mrc->subnet_mask;
        r_mrc->controller_fc_port_speed_setting = i_mrc->controller_fc_port_speed_setting;
        r_mrc->controller_beacon = i_mrc->controller_beacon;
        r_mrc->controller_rank = i_mrc->controller_rank;
        r_mrc->controller_status = i_mrc->controller_status;
        r_mrc->controller_status_details = i_mrc->controller_status_details;
        limited_copy(r_mrc->controller_fw_version, i_mrc->controller_fw_version);
        r_mrc->controller_fc_port_status = i_mrc->controller_fc_port_status;
        r_mrc->controller_fc_port_speed = i_mrc->controller_fc_port_speed;
        r_mrc->controller_ethernet_link_up = i_mrc->controller_ethernet_link_up;
        limited_copy(r_mrc->controller_mac_address, i_mrc->controller_mac_address);
        r_mrc->controller_temperature = i_mrc->controller_temperature;
    }

    /* DataPac */
    // r_dp is the response packet DataPac information.
    struct ise_datapac *r_dp;

    // i_dp is the input ISE DataPac information.
    struct ise_datapac_version_2 *i_dp;

    for (i = 0; i < 2; i++)
    {
        r_dp = &(dst->ise_bay_info.datapac[i]);
        i_dp = &(src->ise_bay_info.datapac[i]);

        r_dp->datapac_beacon = i_dp->datapac_beacon;
        r_dp->datapac_type = i_dp->datapac_type;
        limited_copy(r_dp->datapac_serial_number, i_dp->datapac_serial_number);
        limited_copy(r_dp->datapac_model, i_dp->datapac_model);
        limited_copy(r_dp->datapac_part_number, i_dp->datapac_part_number);
        r_dp->datapac_spare_level = i_dp->datapac_spare_level;
        r_dp->datapac_status = i_dp->datapac_status;
        r_dp->datapac_status_details = i_dp->datapac_status_details;
        r_dp->datapac_capacity = i_dp->datapac_capacity;
        limited_copy(r_dp->datapac_fw_version, i_dp->datapac_fw_version);
        r_dp->datapac_temperature = i_dp->datapac_temperature;
        r_dp->datapac_health = i_dp->datapac_health;
    }

    /* PowerSupply */
    // r_ps is the response packet PowerSupply information.
    struct ise_powersupply *r_ps;

    // i_ps is the input ISE PowerSupply information.
    struct ise_powersupply_version_2 *i_ps;

    for (i = 0; i < 2; i++)
    {
        r_ps = &(dst->ise_bay_info.powersupply[i]);
        i_ps = &(src->ise_bay_info.powersupply[i]);

        r_ps->powersupply_beacon = i_ps->powersupply_beacon;
        limited_copy(r_ps->powersupply_model, i_ps->powersupply_model);
        limited_copy(r_ps->powersupply_serial_number, i_ps->powersupply_serial_number);
        limited_copy(r_ps->powersupply_part_number, i_ps->powersupply_part_number);
        r_ps->powersupply_status = i_ps->powersupply_status;
        r_ps->powersupply_status_details = i_ps->powersupply_status_details;
        r_ps->powersupply_fan1_status = i_ps->powersupply_fan1_status;
        r_ps->powersupply_fan1_speed = i_ps->powersupply_fan1_speed;
        r_ps->powersupply_fan2_status = i_ps->powersupply_fan2_status;
        r_ps->powersupply_fan2_speed = i_ps->powersupply_fan2_speed;
        r_ps->powersupply_temperature = i_ps->powersupply_temperature;
    }

    /* Battery */
    // r_bat is the response packet Battery information.
    struct ise_battery *r_bat;

    // i_bat is the input ISE Battery information.
    struct ise_battery_version_2 *i_bat;

    for (i = 0; i < 2; i++)
    {
        r_bat = &(dst->ise_bay_info.battery[i]);
        i_bat = &(src->ise_bay_info.battery[i]);

        r_bat->battery_beacon = i_bat->battery_beacon;
        limited_copy(r_bat->battery_model, i_bat->battery_model);
        limited_copy(r_bat->battery_serial_number, i_bat->battery_serial_number);
        limited_copy(r_bat->battery_part_number, i_bat->battery_part_number);
        limited_copy(r_bat->battery_type, i_bat->battery_type);
        r_bat->battery_status = i_bat->battery_status;
        r_bat->battery_status_details = i_bat->battery_status_details;
        r_bat->battery_remaining_charge = i_bat->battery_remaining_charge;
        r_bat->battery_max_charge = i_bat->battery_max_charge;
        r_bat->battery_max_charge_capacity = i_bat->battery_max_charge_capacity;
        r_bat->battery_min_holdup_time = i_bat->battery_min_holdup_time;
        r_bat->battery_charger_state = i_bat->battery_charger_state;
        r_bat->battery_charger_state_details = i_bat->battery_charger_state_details;
    }
}

/*----------------------------------------------------------------------------
** Function:    PI_ISEStatus_Copy_Version_2
**
** Description: Copy ISE status data into version 2 PI packet response.
**
** Inputs:      dest  - address to put ise data in response packet.
**              src   - pointer to ISE data from ASGSES program.
**
** Returns:     None.
**
**--------------------------------------------------------------------------*/
static void PI_ISEStatus_Copy_Version_2(UINT8 *dest, struct ise_stats_version_2 *src)
{
    UINT8       i;              // Simple 0, 1 array loop counter.
    UINT8       j;
    // dst is the location in the response packet for ISE data.
    ISE_BAY_STATUS_ITEM_2 *dst = (ISE_BAY_STATUS_ITEM_2 *)dest;

    /* Bay ID */
    dst->bayid = src->bayid;

    /* ISE chassis */
    // r_chassis is the response packet chassis information.
    struct PI_ISE_INFO_VERSION_2 *r_chassis;

    // i_chassis is the input ISE chassis information.
    struct ise_info_version_2 *i_chassis;

    r_chassis = &(dst->ise_bay_info);
    i_chassis = &(src->ise_bay_info);

    r_chassis->Protocol_Version_Level = i_chassis->Protocol_Version_Level;
    r_chassis->which_tcp_connections = i_chassis->which_tcp_connections;

    // Convert to 1 based index (instead of 0).
    j = 0;
    if (i_chassis->which_fw_index_type == 0)
    {
        j = 1;
    }
    for (i = 0; i < 2; i++)
    {
        r_chassis->which_controllers[i] = i_chassis->which_controllers[i] + j;
        r_chassis->which_datapacs[i] = i_chassis->which_datapacs[i] + j;
        r_chassis->which_powersupplies[i] = i_chassis->which_powersupplies[i] + j;
        r_chassis->which_batteries[i] = i_chassis->which_batteries[i] + j;
    }
    r_chassis->ip1 = i_chassis->ip1;
    r_chassis->ip2 = i_chassis->ip2;
    r_chassis->iws_ise_id = i_chassis->iws_ise_id;
    r_chassis->chassis_wwn = i_chassis->chassis_wwn;
    limited_copy(r_chassis->chassis_serial_number, i_chassis->chassis_serial_number);
    limited_copy(r_chassis->chassis_model, i_chassis->chassis_model);
    limited_copy(r_chassis->chassis_part_number, i_chassis->chassis_part_number);
    limited_copy(r_chassis->chassis_vendor, i_chassis->chassis_vendor);
    limited_copy(r_chassis->chassis_manufacturer, i_chassis->chassis_manufacturer);
    limited_copy(r_chassis->chassis_product_version, i_chassis->chassis_product_version);
    r_chassis->spare_level = i_chassis->spare_level;
    r_chassis->chassis_beacon = i_chassis->chassis_beacon;
    r_chassis->chassis_status = i_chassis->chassis_status;
    r_chassis->chassis_status_details = i_chassis->chassis_status_details;
    r_chassis->chassis_uptime = i_chassis->chassis_uptime;
    r_chassis->chassis_current_date_time = i_chassis->chassis_current_date_time;
    r_chassis->chassis_performance_valid = i_chassis->chassis_performance_valid;
    r_chassis->chassis_total_iops = i_chassis->chassis_total_iops;
    r_chassis->chassis_read_iops = i_chassis->chassis_read_iops;
    r_chassis->chassis_write_iops = i_chassis->chassis_write_iops;
    r_chassis->chassis_total_kbps = i_chassis->chassis_total_kbps;
    r_chassis->chassis_read_kbps = i_chassis->chassis_read_kbps;
    r_chassis->chassis_write_kbps = i_chassis->chassis_write_kbps;
    r_chassis->chassis_read_latency = i_chassis->chassis_read_latency;
    r_chassis->chassis_write_latency = i_chassis->chassis_write_latency;
    r_chassis->chassis_queue_depth = i_chassis->chassis_queue_depth;
    r_chassis->chassis_read_percent = i_chassis->chassis_read_percent;
    r_chassis->chassis_avg_bytes_transferred = i_chassis->chassis_avg_bytes_transferred;
    r_chassis->chassis_temperature_sensor = i_chassis->chassis_temperature_sensor;

    /* MRC */
    // r_mrc is the response packet MRC information.
    struct PI_ISE_CONTROLLER_VERSION_2 *r_mrc;

    // i_mrc is the input ISE MRC information.
    struct ise_controller_version_2 *i_mrc;

    for (i = 0; i < 2; i++)
    {
        r_mrc = &(dst->ise_bay_info.ctrlr[i]);
        i_mrc = &(src->ise_bay_info.ctrlr[i]);

        limited_copy(r_mrc->controller_model, i_mrc->controller_model);
        limited_copy(r_mrc->controller_serial_number, i_mrc->controller_serial_number);
        limited_copy(r_mrc->controller_part_number, i_mrc->controller_part_number);
        limited_copy(r_mrc->controller_hw_version, i_mrc->controller_hw_version);
        r_mrc->controller_wwn = i_mrc->controller_wwn;
        r_mrc->ip = i_mrc->ip;
        r_mrc->gateway = i_mrc->gateway;
        r_mrc->subnet_mask = i_mrc->subnet_mask;
        r_mrc->controller_fc_port_speed_setting = i_mrc->controller_fc_port_speed_setting;
        r_mrc->controller_beacon = i_mrc->controller_beacon;
        r_mrc->controller_rank = i_mrc->controller_rank;
        r_mrc->controller_status = i_mrc->controller_status;
        r_mrc->controller_status_details = i_mrc->controller_status_details;
        limited_copy(r_mrc->controller_fw_version, i_mrc->controller_fw_version);
        r_mrc->controller_fc_port_status = i_mrc->controller_fc_port_status;
        r_mrc->controller_fc_port_speed = i_mrc->controller_fc_port_speed;
        r_mrc->controller_ethernet_link_up = i_mrc->controller_ethernet_link_up;
        limited_copy(r_mrc->controller_mac_address, i_mrc->controller_mac_address);
        r_mrc->controller_temperature = i_mrc->controller_temperature;
        // calculate position from x,y,z position gotten from ISE and ASGSES.
        if (i_chassis->which_fw_index_type == 0)
        {
            r_mrc->controller_position = i + 1;
        }
        else
        {
            r_mrc->controller_position = i_mrc->controller_x_position;
        }
    }

    /* DataPac */
    // r_dp is the response packet DataPac information.
    struct PI_ISE_DATAPAC_VERSION_2 *r_dp;

    // i_dp is the input ISE DataPac information.
    struct ise_datapac_version_2 *i_dp;

    for (i = 0; i < 2; i++)
    {
        r_dp = &(dst->ise_bay_info.datapac[i]);
        i_dp = &(src->ise_bay_info.datapac[i]);

        r_dp->datapac_beacon = i_dp->datapac_beacon;
        r_dp->datapac_type = i_dp->datapac_type;
        limited_copy(r_dp->datapac_serial_number, i_dp->datapac_serial_number);
        limited_copy(r_dp->datapac_model, i_dp->datapac_model);
        limited_copy(r_dp->datapac_part_number, i_dp->datapac_part_number);
        r_dp->datapac_spare_level = i_dp->datapac_spare_level;
        r_dp->datapac_status = i_dp->datapac_status;
        r_dp->datapac_status_details = i_dp->datapac_status_details;
        r_dp->datapac_capacity = i_dp->datapac_capacity;
        limited_copy(r_dp->datapac_fw_version, i_dp->datapac_fw_version);
        r_dp->datapac_temperature = i_dp->datapac_temperature;
        r_dp->datapac_health = i_dp->datapac_health;
        // calculate position from x,y,z position gotten from ISE and ASGSES.
        if (i_chassis->which_fw_index_type == 0)
        {
            r_dp->datapac_position = i + 1;
        }
        else
        {
            r_dp->datapac_position = i_dp->datapac_x_position;
        }
    }

    /* PowerSupply */
    // r_ps is the response packet PowerSupply information.
    struct PI_ISE_POWERSUPPLY_VERSION_2 *r_ps;

    // i_ps is the input ISE PowerSupply information.
    struct ise_powersupply_version_2 *i_ps;

    for (i = 0; i < 2; i++)
    {
        r_ps = &(dst->ise_bay_info.powersupply[i]);
        i_ps = &(src->ise_bay_info.powersupply[i]);

        r_ps->powersupply_beacon = i_ps->powersupply_beacon;
        limited_copy(r_ps->powersupply_model, i_ps->powersupply_model);
        limited_copy(r_ps->powersupply_serial_number, i_ps->powersupply_serial_number);
        limited_copy(r_ps->powersupply_part_number, i_ps->powersupply_part_number);
        r_ps->powersupply_status = i_ps->powersupply_status;
        r_ps->powersupply_status_details = i_ps->powersupply_status_details;
        r_ps->powersupply_fan1_status = i_ps->powersupply_fan1_status;
        r_ps->powersupply_fan1_speed = i_ps->powersupply_fan1_speed;
        r_ps->powersupply_fan2_status = i_ps->powersupply_fan2_status;
        r_ps->powersupply_fan2_speed = i_ps->powersupply_fan2_speed;
        r_ps->powersupply_temperature = i_ps->powersupply_temperature;
        // calculate position from x,y,z position gotten from ISE and ASGSES.
        if (i_chassis->which_fw_index_type == 0)
        {
            r_ps->powersupply_position = i + 1;
        }
        else
        {
            r_ps->powersupply_position = i_ps->powersupply_x_position;
        }
    }

    /* Battery */
    // r_bat is the response packet Battery information.
    struct PI_ISE_BATTERY_VERSION_2 *r_bat;

    // i_bat is the input ISE Battery information.
    struct ise_battery_version_2 *i_bat;

    for (i = 0; i < 2; i++)
    {
        r_bat = &(dst->ise_bay_info.battery[i]);
        i_bat = &(src->ise_bay_info.battery[i]);

        r_bat->battery_beacon = i_bat->battery_beacon;
        limited_copy(r_bat->battery_model, i_bat->battery_model);
        limited_copy(r_bat->battery_serial_number, i_bat->battery_serial_number);
        limited_copy(r_bat->battery_part_number, i_bat->battery_part_number);
        limited_copy(r_bat->battery_type, i_bat->battery_type);
        r_bat->battery_status = i_bat->battery_status;
        r_bat->battery_status_details = i_bat->battery_status_details;
        r_bat->battery_remaining_charge = i_bat->battery_remaining_charge;
        r_bat->battery_max_charge = i_bat->battery_max_charge;
        r_bat->battery_max_charge_capacity = i_bat->battery_max_charge_capacity;
        r_bat->battery_min_holdup_time = i_bat->battery_min_holdup_time;
        r_bat->battery_charger_state = i_bat->battery_charger_state;
        r_bat->battery_charger_state_details = i_bat->battery_charger_state_details;
        // calculate position from x,y,z position gotten from ISE and ASGSES.
        if (i_chassis->which_fw_index_type == 0)
        {
            r_bat->battery_position = i + 1;
        }
        else
        {
            r_bat->battery_position = i_bat->battery_x_position;
        }
    }
}

/*----------------------------------------------------------------------------
** Function:    PI_ISEStatus
**
** Description: Request for ISE Status
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_ISEStatus(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    UINT32      outPktSize = 0;
    PI_ISE_BAY_STATUS_RSP *ptrOutPkt = NULL;
    PI_ISE_BAY_STATUS_2_RSP *ptrOutPkt2 = NULL;
    UINT8      *currptr = NULL;
    UINT32      local_nbu = ISE_Number_Bays_Used;
    UINT32      i = 0;
    UINT32      j = 0;
    INT32       errorCode = 0;

    /*
     * Get length of memory and memory for response data.
     */

    if (pReqPacket->pHeader->packetVersion == 0 ||
        pReqPacket->pHeader->packetVersion == 1)
    {
        outPktSize = (sizeof(*ptrOutPkt) + (local_nbu * sizeof(ISE_BAY_STATUS_ITEM_0)));

        ptrOutPkt = MallocWC(outPktSize);
        ptrOutPkt->baycount = local_nbu;
        pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
        currptr = (UINT8 *)(&ptrOutPkt->iseBayStatus);
    }
    else if (pReqPacket->pHeader->packetVersion == 2)
    {
        outPktSize = (sizeof(*ptrOutPkt2) + (local_nbu * sizeof(ISE_BAY_STATUS_ITEM_2)));

        ptrOutPkt2 = MallocWC(outPktSize);
        ptrOutPkt2->baycount = local_nbu;
        pRspPacket->pPacket = (UINT8 *)ptrOutPkt2;
        currptr = (UINT8 *)(&ptrOutPkt2->iseBayStatus);
    }

    pRspPacket->pHeader->length = outPktSize;

    /*
     * If we have information from any ISE device(s) to copy, do so.
     * Specifically, if we "had" no devices (before MallocWC), do not copy.
     */
    while (i < MAX_DISK_BAYS && j < local_nbu)
    {
        if (ise_data[i].ise_bay_info.ip1 != 0 || ise_data[i].ise_bay_info.ip2 != 0)
        {
            // Copy the data correctly for version 0/1 or version 2.
            if (pReqPacket->pHeader->packetVersion == 0 ||
                pReqPacket->pHeader->packetVersion == 1)
            {
                PI_ISEStatus_Copy_Version_0(currptr, &ise_data[i]);
                currptr += sizeof(ISE_BAY_STATUS_ITEM_0);
            }
            else if (pReqPacket->pHeader->packetVersion == 2)
            {
                PI_ISEStatus_Copy_Version_2(currptr, &ise_data[i]);
                currptr += sizeof(ISE_BAY_STATUS_ITEM_2);
            }
            j++;
        }
        i++;
    }

    if ((j == 0) && (i == MAX_DISK_BAYS))
    {
        /*
         * There are no ISEs to report, so return error.
         */
        rc = PI_ERROR;
        errorCode = DESESINPROGRESS;
    }

    pRspPacket->pHeader->errorCode = errorCode;
    pRspPacket->pHeader->status = rc;

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_BeaconIseComponent
**
** Description: Issue to beacon ise component
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_BeaconIseComponent(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    UINT16      bayid;
    UINT16      command;
    PI_BEACON_ISE_COMPONENT_REQ *inPkt;
    UINT8       light_on_off;
    UINT16      subcomp;
    UINT32      errorCode = 0;

    inPkt = (PI_BEACON_ISE_COMPONENT_REQ *)pReqPacket->pPacket;
    bayid = inPkt->bayid;
    light_on_off = inPkt->light_on_off;
    subcomp = inPkt->subcomponent;

    if (subcomp == 1 || subcomp == 2)
    {
        switch (inPkt->component)
        {
            case PI_BEACON_ISE_CHASSIS:
                command = ASGSES_cmd_ISE_Beacon;
                break;

            case PI_BEACON_ISE_MRC:
                if (1 == subcomp)
                    command = ASGSES_cmd_MRC_Beacon0;
                else
                    command = ASGSES_cmd_MRC_Beacon1;
                break;

            case PI_BEACON_ISE_PS:
                if (1 == subcomp)
                    command = ASGSES_cmd_PS_Beacon0;
                else
                    command = ASGSES_cmd_PS_Beacon1;
                break;

            case PI_BEACON_ISE_BAT:
                if (1 == subcomp)
                    command = ASGSES_cmd_Bat_Beacon0;
                else
                    command = ASGSES_cmd_Bat_Beacon1;
                break;

            case PI_BEACON_ISE_DP:
                if (1 == subcomp)
                    command = ASGSES_cmd_DP_Beacon0;
                else
                    command = ASGSES_cmd_DP_Beacon1;
                break;

            case PI_BEACON_ISE_SFP:
                if (1 == subcomp)
                    command = ASGSES_cmd_SFP_Beacon0;
                else
                    command = ASGSES_cmd_SFP_Beacon1;
                break;

            case PI_BEACON_ISE_CAP:
                if (1 == subcomp)
                    command = ASGSES_cmd_CAP_Beacon0;
                else
                    command = ASGSES_cmd_CAP_Beacon1;
                break;

            case PI_BEACON_ISE_BEZEL:
                command = ASGSES_cmd_BEZEL_Beacon;
                break;

            default:
                rc = PI_ERROR;
                errorCode = DEINVOPT;
                goto out;
        }
        rc = SendBeaconIseCommand(bayid, command, light_on_off);
    }
    else
    {
        rc = DEINVOPT;
    }

    if (rc != GOOD)
    {
        errorCode = rc;
        rc = PI_ERROR;
    }
    else
    {
        rc = PI_GOOD;
    }

  out:
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = errorCode;

    return rc;
}
#endif /* MODEL_7000 || MODEL_4700 */

/*----------------------------------------------------------------------------
** Function:    PI_StatsLoop - MRP Requests
**
** Description: Loop Statistics
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_StatsLoop(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    MR_LIST_RSP *ptrPList = NULL;
    MR_LIST_RSP *ptrTList = NULL;
    UINT16      count = 0;
    PI_STATS_LOOPS_RSP *ptrOutPkt = NULL;
    UINT32      outPktSize = 0;
    UINT32      maxPktSize = 0;

    MRPORT_REQ *pInPkt = NULL;
    MRPORT_RSP *pOutPkt = NULL;
    UINT8      *pBuf = NULL;
    UINT32      bufSize = 0;
    INT32       errorCode = PI_GOOD;
    UINT16      statsMRP = 0;
    UINT16      listMRP = 0;

    dprintf(DPRINTF_PI_COMMANDS, "PI_StatsLoop - MRPs issued\n");

    /*
     * Translate the packet command code into an MRP command code.
     * Determine which port list applies for this stats command.
     */
    switch (pReqPacket->pHeader->commandCode)
    {
        case PI_STATS_FRONT_END_LOOP_CMD:
            statsMRP = MRFELOOP;
            listMRP = MRFEGETPORTLIST;
            break;

        case PI_STATS_BACK_END_LOOP_CMD:
            statsMRP = MRBELOOP;
            listMRP = MRBEGETPORTLIST;
            break;

        default:
            rc = PI_INVALID_CMD_CODE;
            break;
    }

    if (rc == PI_GOOD)
    {
        /*
         * Get the list of objects.  Always start at the beginning and return
         * the entire list.
         */
        ptrPList = PI_GetList(0, (listMRP | GET_LIST));
        ptrTList = PI_GetList(0, (MRGETTLIST | GET_NUMBER_ONLY));

        /* If we could get the lists use them, otherwise signal an error */
        if (ptrPList != NULL && ptrTList != NULL)
        {
            /*
             * Add the static data size for each LOOP STATISTIC.
             */
            outPktSize += ((sizeof(PI_STATS_LOOP) + sizeof(*pOutPkt)) * ptrPList->ndevs);

            /*
             * Add the size of a UINT16 for each of the targets (we only
             * retrieve the ID values when getting loop statistics).
             *
             * For buffer space, multiply the number of targets configured
             * by 2.  This will allow for the loop statistics to return
             * information for all targets in the system if they happen
             * to be on one loop.
             */
            outPktSize += (sizeof(UINT16) * ptrTList->ndevs);

            /*
             * Calculate the maximum size for a given LOOP STATS request packet.
             *
             * For buffer space, multiply the number of targets configured
             * by 2.  This will allow for the loop statistics to return
             * information for all targets in the system if they happen
             * to be on one loop.
             */
            maxPktSize = (sizeof(*pOutPkt) + (sizeof(UINT16) * ptrTList->ndevs));

            /*
             * The outPktSize variable currently contains the size (in bytes)
             * of the loop statistics information (including target list)
             * for all the loops.  For our response we need to include the
             * size of the PI_STATS_LOOPS_RSP packet.
             */
            outPktSize += sizeof(*ptrOutPkt);

            /*
             * The input packet will be used in the calls to the Loop Stats
             * MRP.
             *
             * The output packet will be used in the calls to the Loop Stats
             * MRP.
             *
             * The second output packet will contain the stats for all ports
             * found.
             */
            pInPkt = MallocWC(sizeof(*pInPkt));
            pOutPkt = MallocSharedWC(maxPktSize);
            ptrOutPkt = MallocWC(outPktSize);

            /* Save the number of devices in the output packet */
            ptrOutPkt->count = ptrPList->ndevs;

            /*
             * The pBuf variable holds the location in the output packet
             * buffer to where the next Loop Stats should be placed.
             * This pointer moves through the output packet buffer for
             * each request to the LOOP STATS MRP.
             */
            pBuf = (UINT8 *)ptrOutPkt->stats;

            /*
             * At this time the outPktSize contains the entire output packet
             * buffer size, including the PI_STATS_LOOPS_RSP structure size.
             * We need a variable that contains the size of the buffer
             * remaining for retrieving STATS LOOP information.  After each
             * call to the STATS LOOP MRP the buffer size will be reduced
             * by the size of the information returned by the MRP.
             */
            bufSize = outPktSize - sizeof(*ptrOutPkt);

            /*
             * Loop through each of the IDs in the PORT ID LIST and make
             * a call to the LOOP STATS MRP.
             */
            for (count = 0; count < ptrPList->ndevs; count++)
            {
                /*
                 * Setup the input packet to have the correct PORT ID.
                 * Get the option from the request packet.
                 */
                pInPkt->port = ptrPList->list[count];

                if (pReqPacket->pPacket == NULL)
                {
                    pInPkt->option = 0;

                    dprintf(DPRINTF_PI_COMMANDS, "PI_StatsLoop: pPacket == NULL, option set to 0\n");
                }
                else
                {
                    pInPkt->option = ((PI_STATS_LOOPS_REQ *)(pReqPacket->pPacket))->option;

                    dprintf(DPRINTF_PI_COMMANDS, "PI_StatsLoop: pPacket != NULL, option set to %d\n",
                            pInPkt->option);
                }

                rc = PI_ExecMRP(pInPkt, sizeof(*pInPkt), statsMRP,
                                pOutPkt, maxPktSize, GetGlobalMRPTimeout());

                if (rc == PI_GOOD)
                {
                    if (pOutPkt->header.len <= bufSize)
                    {
                        /*
                         * The PI_STATS_LOOP structure contains the data length and
                         * port ID for this set of data.  Fill those values in now
                         * and move the pointer to the actual start of the port stats.
                         * The length field includes the size of the port field.
                         */
                        ((PI_STATS_LOOP *)pBuf)->length = pOutPkt->header.len +
                            sizeof(((PI_STATS_LOOP *)pBuf)->port);

                        ((PI_STATS_LOOP *)pBuf)->port = ptrPList->list[count];

                        pBuf += sizeof((PI_STATS_LOOP *)pBuf)->length +
                            sizeof((PI_STATS_LOOP *)pBuf)->port;

                        /*
                         * Copy the data retrieved throught the MRP into the
                         * output packet that contains the information for
                         * all the LOOPS STATS combined.
                         */
                        memcpy(pBuf, pOutPkt, pOutPkt->header.len);

                        /*
                         * Decrement the remaining buffer size to not include
                         * this stats loop information.
                         */
                        bufSize -= (pOutPkt->header.len +
                                    sizeof((PI_STATS_LOOP *)pBuf)->length +
                                    sizeof((PI_STATS_LOOP *)pBuf)->port);

                        /*
                         * Move the output packet pointer to be past the STATS
                         * LOOP info we just retrieved.
                         */
                        pBuf += pOutPkt->header.len;
                    }
                    else
                    {
                        /*
                         * The amount of data returned in this request would
                         * exceed the buffer space so return the too much
                         * data error code.
                         */
                        rc = PI_ERROR;
                        errorCode = DETOOMUCHDATA;
                    }
                }
                else
                {
                    /*
                     * Some sort of error occurred.  We cannot even tolerate a
                     "" "too much data" error in this case since we should have
                     * done the calcualation correctly before starting to retrieve
                     * the stats loop information.
                     */
                    errorCode = pOutPkt->header.status;
                    break;
                }
            }
        }
        else
        {
            /*
             * One or both of the lists could not be retrieved so
             * an error must be returned.
             */
            rc = PI_ERROR;
            errorCode = DELISTERROR;
        }
    }

    /*
     * Free the allocated memory
     */
    Free(ptrPList);
    Free(ptrTList);
    Free(pInPkt);

    if (rc != PI_TIMEOUT)
    {
        Free(pOutPkt);
    }

    if (rc == PI_GOOD)
    {
        /*
         * Attach the MRP return data packet to the main response packet.
         * Fill in the header length and status fields.
         */
        pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
        pRspPacket->pHeader->length = outPktSize;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = errorCode;
    }
    else
    {
        Free(ptrOutPkt);

        /*
         * Indicate an error condition and no return data in the header.
         */
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = errorCode;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_StatsVDisk
**
** Description: Virtual Disk Statistics
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_StatsVDisk(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    PI_VDISKS_RSP *pVDisks = NULL;
    UINT16      count = 0;
    PI_STATS_VDISK_RSP *ptrOutPkt = NULL;
    PI_STATS_VDISK2_RSP *ptrOutPkt2 = NULL;
    UINT32      outPktSize = 0;
    UINT8      *pVDiskInfo = NULL;

    pVDisks = VirtualDisks();

    if (pVDisks != NULL)
    {
        if (pReqPacket->pHeader->packetVersion == 0 ||
            pReqPacket->pHeader->packetVersion == 1)
        {
            /*
             * Calculate the size of the output packet.  This will be the size
             * of the cache devices (multiple devices) response packet plus the
             * size of a cache device (single device) response packet for each
             * device.
             */
            outPktSize = sizeof(*ptrOutPkt) + (pVDisks->count * sizeof(PI_VDISK_INFO_RSP));

            ptrOutPkt = MallocWC(outPktSize);

            /* Save the number of devices in the output packet */
            ptrOutPkt->count = pVDisks->count;

            pVDiskInfo = pVDisks->vdisks;

            /*
             * Loop through the devices in the list and get the device
             * information.  When we get the information save it in
             * the output packet.
             */
            for (count = 0; count < pVDisks->count; count++)
            {
                memcpy(&ptrOutPkt->vdiskInfo[count],
                       pVDiskInfo, sizeof(PI_VDISK_INFO_RSP));

                ptrOutPkt->vdiskInfo[count].header.len = sizeof(PI_VDISK_INFO_RSP);

                /*
                 * Are there more items in the list to be processed?
                 */
                if (count < pVDisks->count)
                {
                    /*
                     * Increment past this virtual disk information response
                     * to the next one in the list.
                     */
                    pVDiskInfo += ((PI_VDISK_INFO2_RSP *)pVDiskInfo)->header.len;
                }
            }
        }
        else if (pReqPacket->pHeader->packetVersion == 2)
        {
            /*
             * Calculate the size of the output packet.  This will be the size
             * of the cache devices (multiple devices) response packet plus the
             * size of a cache device (single device) response packet for each
             * device.
             */
            outPktSize = sizeof(*ptrOutPkt2) + (pVDisks->count * sizeof(PI_VDISK_INFO2_RSP));

            ptrOutPkt2 = MallocWC(outPktSize);

            /* Save the number of devices in the output packet */
            ptrOutPkt2->count = pVDisks->count;

            pVDiskInfo = pVDisks->vdisks;

            /*
             * Loop through the devices in the list and get the device
             * information.  When we get the information save it in
             * the output packet.
             */
            for (count = 0; count < pVDisks->count; count++)
            {
                memcpy(&ptrOutPkt2->vdiskInfo[count],
                       pVDiskInfo, sizeof(PI_VDISK_INFO2_RSP));

                ptrOutPkt2->vdiskInfo[count].header.len = sizeof(PI_VDISK_INFO2_RSP);

                /*
                 * Are there more items in the list to be processed?
                 */
                if (count < pVDisks->count)
                {
                    /*
                     * Increment past this virtual disk information response
                     * to the next one in the list.
                     */
                    pVDiskInfo += ((PI_VDISK_INFO2_RSP *)pVDiskInfo)->header.len;
                }
            }
        }
    }
    else
    {
        rc = PI_ERROR;
    }

    /*
     * Free the allocated memory
     */
    Free(pVDisks);

    if (rc == PI_GOOD)
    {
        /*
         * Attach the MRP return data packet to the main response packet.
         * Fill in the header length and status fields.
         */
        if (pReqPacket->pHeader->packetVersion == 0 ||
            pReqPacket->pHeader->packetVersion == 1)
        {
            pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
        }
        else if (pReqPacket->pHeader->packetVersion == 2)
        {
            pRspPacket->pPacket = (UINT8 *)ptrOutPkt2;
        }

        pRspPacket->pHeader->length = outPktSize;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = rc;
    }
    else
    {
        Free(ptrOutPkt);

        /*
         * Indicate an error condition and no return data in the header.
         */
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = rc;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_StatsProc
**
** Description: Processor Statistics
**              Handle requests for FE, BE or combined processor stats
**              in one function.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_StatsProc(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    MR_HDR_RSP  header;
    UINT8      *pOutPkt = NULL;
    UINT32      outPktLength = 0;
    INT32       rc = PI_GOOD;
    INT32       errorCode = DEOK;

    /*
     * Fill in the MRP header.  Even though we didn't get the data by
     * issuing an MRP this information may be used by others.
     */
    memset(&(header.rsvd0), 0, 3);
    header.status = errorCode;
    header.len = sizeof(MRFEII_RSP);

    /*
     * Allocate the response packet and copy the data based on the
     * request command code.
     */
    switch (pReqPacket->pHeader->commandCode)
    {
        case PI_STATS_FRONT_END_PROC_CMD:
            outPktLength = sizeof(PI_STATS_FRONT_END_PROC_RSP);
            pOutPkt = MallocWC(outPktLength);

            memcpy(&(((PI_STATS_FRONT_END_PROC_RSP *)pOutPkt)->header), &header, sizeof(header));
            if (GetProcAddress_FEII() != 0)
            {
                memcpy(&(((PI_STATS_FRONT_END_PROC_RSP *)pOutPkt)->ii), GetProcAddress_FEII(), sizeof(II));
            }
            else
            {
                memset(&(((PI_STATS_FRONT_END_PROC_RSP *)pOutPkt)->ii), 0, sizeof(II));
            }
            break;

        case PI_STATS_BACK_END_PROC_CMD:
            outPktLength = sizeof(PI_STATS_BACK_END_PROC_RSP);
            pOutPkt = MallocWC(outPktLength);

            memcpy(&(((PI_STATS_BACK_END_PROC_RSP *)pOutPkt)->header), &header, sizeof(MR_HDR_RSP));
            if (GetProcAddress_BEII() != 0)
            {
                memcpy(&(((PI_STATS_BACK_END_PROC_RSP *)pOutPkt)->ii), GetProcAddress_BEII(), sizeof(II));
            }
            else
            {
                memset(&(((PI_STATS_BACK_END_PROC_RSP *)pOutPkt)->ii), 0, sizeof(II));
            }
            break;

        case PI_STATS_PROC_CMD:
            outPktLength = sizeof(PI_STATS_PROC_RSP);
            pOutPkt = MallocWC(outPktLength);

            memcpy(&(((PI_STATS_PROC_RSP *)pOutPkt)->fe.header), &header, sizeof(header));
            if (GetProcAddress_FEII() != 0)
            {
                memcpy(&(((PI_STATS_PROC_RSP *)pOutPkt)->fe.ii), GetProcAddress_FEII(), sizeof(II));
            }
            else
            {
                memset(&(((PI_STATS_PROC_RSP *)pOutPkt)->fe.ii), 0, sizeof(II));
            }

            memcpy(&(((PI_STATS_PROC_RSP *)pOutPkt)->be.header), &header, sizeof(header));
            if (GetProcAddress_BEII() != 0)
            {
                memcpy(&(((PI_STATS_PROC_RSP *)pOutPkt)->be.ii), GetProcAddress_BEII(), sizeof(II));
            }
            else
            {
                memset(&(((PI_STATS_PROC_RSP *)pOutPkt)->be.ii), 0, sizeof(II));
            }
            break;

        default:
            rc = PI_INVALID_CMD_CODE;
            errorCode = DEINVPKTTYP;
    }

    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = pOutPkt;
    pRspPacket->pHeader->length = outPktLength;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = errorCode;

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_StatsPCI
**
** Description: PCI Statistics
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_StatsPCI(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    PI_STATS_PCI_RSP *ptrOutPkt = NULL;
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };

    /* Allocate the response packet */
    ptrOutPkt = MallocWC(sizeof(*ptrOutPkt));

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /*
     * If everything is good so far, retrieve the front-end PCI
     * statistics and copy the results into the output packet.
     */
    if (rc == PI_GOOD)
    {
        /*
         * Fill in the Header
         */
        reqPacket.pHeader->commandCode = PI_STATS_FRONT_END_PCI_CMD;
        reqPacket.pHeader->length = 0;

        /*
         * Issue the command through the packet command handler
         */
        rc = PortServerCommandHandler(&reqPacket, &rspPacket);

        if (rc == PI_GOOD)
        {
            memcpy(&ptrOutPkt->fe, rspPacket.pPacket, sizeof(PI_STATS_FRONT_END_PCI_RSP));
        }

        if (rc != PI_TIMEOUT)
        {
            Free(rspPacket.pPacket);
        }
    }

    /*
     * If everything is good so far, retrieve the back-end PCI
     * statistics and copy the results into the output packet.
     */
    if (rc == PI_GOOD)
    {
        /*
         * Fill in the Header
         */
        reqPacket.pHeader->commandCode = PI_STATS_BACK_END_PCI_CMD;
        reqPacket.pHeader->length = 0;

        /*
         * Issue the command through the packet command handler
         */
        rc = PortServerCommandHandler(&reqPacket, &rspPacket);

        if (rc == PI_GOOD)
        {
            memcpy(&ptrOutPkt->be, rspPacket.pPacket, sizeof(PI_STATS_BACK_END_PCI_RSP));
        }

        if ((rspPacket.pPacket != NULL) && (rc != PI_TIMEOUT))
        {
            Free(rspPacket.pPacket);
        }
    }

    /*
     * Free the allocated memory
     */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    if (rc == PI_GOOD)
    {
        /*
         * Attach the MRP return data packet to the main response packet.
         * Fill in the header length and status fields.
         */
        pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
        pRspPacket->pHeader->length = sizeof(*ptrOutPkt);
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = rc;
    }
    else
    {
        Free(ptrOutPkt);

        /*
         * Indicate an error condition and no return data in the header.
         */
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = rc;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_StatsCacheDevices
**
** Description: Cache Devices Statistics
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_StatsCacheDevices(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    MR_LIST_RSP *ptrList = NULL;
    UINT16      count = 0;
    PI_STATS_CACHE_DEVICES_RSP *ptrOutPkt = NULL;
    UINT32      outPktSize = 0;
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };

    /*
     * Get the list of objects.  Always start at the beginning and return
     * the entire list.
     */
    ptrList = PI_GetList(0, (MRGETVLIST | GET_LIST));

    /* If we could not get the list, signal an error */
    if (ptrList == NULL)
    {
        rc = PI_ERROR;
    }

    if (rc == PI_GOOD)
    {
        /*
         * Calculate the size of the output packet.  This will be the size
         * of the cache devices (multiple devices) response packet plus the
         * size of a cache device (single device) response packet for each
         * device.
         */
        outPktSize = sizeof(*ptrOutPkt) + (ptrList->ndevs * sizeof(PI_STATS_CACHE_DEV_RSP));

        ptrOutPkt = MallocWC(outPktSize);

        /* Save the number of devices in the output packet */
        ptrOutPkt->count = ptrList->ndevs;

        /*
         * Allocate memory for the request (header and data) and the
         * response header. The response data will be allocated in the called
         * function.
         */
        reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
        reqPacket.pPacket = MallocWC(sizeof(PI_STATS_CACHE_DEV_REQ));
        rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
        reqPacket.pHeader->packetVersion = 1;
        rspPacket.pHeader->packetVersion = 1;

        /*
         * Fill in the Header
         */
        reqPacket.pHeader->commandCode = PI_STATS_CACHE_DEVICE_CMD;
        reqPacket.pHeader->length = sizeof(PI_STATS_CACHE_DEV_REQ);

        /*
         * Loop through the devices in the list and get the cache device
         * information.  When we get the cache information save it in
         * the output packet.
         */
        for (count = 0; count < ptrList->ndevs; count++)
        {
            rspPacket.pPacket = NULL;

            /* Setup the ID for this Cache Device Information Request */
            ((PI_STATS_CACHE_DEV_REQ *)reqPacket.pPacket)->id = ptrList->list[count];

            /*
             * Issue the command through the packet command handler
             */
            rc = PortServerCommandHandler(&reqPacket, &rspPacket);

            if (rc == PI_GOOD)
            {
                /*
                 * Copy the cache device information for this device
                 * into the output packet.
                 */
                memcpy(&ptrOutPkt->cacheDev[count],
                       rspPacket.pPacket, sizeof(*ptrOutPkt));
            }
            else
            {
                /*
                 * Since an error occurred retrieving the cache device
                 * statistics for one of the devices we do not want
                 * to continue processing, so break out of the for loop.
                 */
                break;
            }

            if (rc != PI_TIMEOUT)
            {
                Free(rspPacket.pPacket);
            }
        }
    }

    /*
     * Free the allocated memory
     */
    Free(ptrList);
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    if (rc == PI_GOOD)
    {
        /*
         * Attach the MRP return data packet to the main response packet.
         * Fill in the header length and status fields.
         */
        pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
        pRspPacket->pHeader->length = outPktSize;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = rc;
    }
    else
    {
        Free(ptrOutPkt);

        /*
         * Indicate an error condition and no return data in the header.
         */
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = rc;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    StatsCacheDevices
**
** Desc:        This method will retrieve cache statistics for all devices
**
** Inputs:      none
**
** Returns:     Stats Cache Devices response packet.
**
** WARNING:     The caller of this method will need to free the response packet
**              after they have finished using it.
**--------------------------------------------------------------------------*/
PI_STATS_CACHE_DEVICES_RSP *StatsCacheDevices(void)
{
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    PI_STATS_CACHE_DEVICES_RSP *pResponse = NULL;
    UINT32      rc = PI_GOOD;

    /*
     * Set up the request and response packets.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = NULL;
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    reqPacket.pHeader->commandCode = PI_STATS_CACHE_DEVICES_CMD;
    reqPacket.pHeader->length = 0;

    /*
     * Issue the command through the top-level command handler.
     */
    rc = PortServerCommandHandler(&reqPacket, &rspPacket);

    if (rc == PI_GOOD)
    {
        pResponse = (PI_STATS_CACHE_DEVICES_RSP *)(rspPacket.pPacket);
        rspPacket.pPacket = NULL;
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "  StatsCacheDevices - ERROR: rc=0x%X\n", rc);
    }

    /*
     * Free the allocated memory
     */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return pResponse;
}

/*----------------------------------------------------------------------------
** Function:    PI_StatsServers
**
** Description: Statistics for all Servers that are valid on this controller
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_StatsServers(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_TARGET_RESOURCE_LIST_RSP *pTargetResList = NULL;
    MRGETSSTATS_REQ *pMRPInPkt = NULL;
    UINT8      *pRspPkt = NULL;
    UINT8      *pCurrentStats = NULL;
    UINT32      rspPktSize = 0;
    UINT32      ndevs = 0;
    UINT32      i;
    INT32       rc = PI_GOOD;

    /*
     * Get the list of servers that have statistics and are owned
     * by this controller using Target Resource List.
     * 0xFFFF is used to get the complete list.
     * NOTE: pTargetResList is allocated in TargetResourceList()
     * and must be freed here.
     */
    pTargetResList = TargetResourceList(0xFFFF, SERVERS_W_STATS);

    /*
     * Bail out early if we get a NULL pointer.  Nothing else we
     * can do.
     */
    if (pTargetResList != NULL)
    {
        /*
         * Set the ndevs.
         */
        ndevs = pTargetResList->ndevs;
    }

    /*
     * Allocate memory for the MRP request packet and the response packet.
     */
    pMRPInPkt = MallocWC(sizeof(*pMRPInPkt));

    rspPktSize = sizeof(PI_STATS_SERVERS_RSP) + (ndevs * sizeof(STATS_SERVER_ITEM));

    pRspPkt = MallocSharedWC(rspPktSize);

    /*
     * Get the server count and copy to the response packet.
     */
    ((PI_STATS_SERVERS_RSP *)pRspPkt)->count = ndevs;

    /*
     * Stats for the first server are copied to the response packet
     * after the header information.
     */
    pCurrentStats = pRspPkt + sizeof(PI_STATS_SERVERS_RSP);

    /*
     * Loop through the list of servers with statistics and get the stats.
     */
    for (i = 0; i < ndevs; i++)
    {
        /*
         * Determine the requested sid from the TargetResourceList
         * response.
         */
        pMRPInPkt->sid = pTargetResList->list[i];
        pMRPInPkt->option = 0;

        /*
         * Place the requested server ID in the response packet and advance
         * the pointer to the start of the stats for this server.
         * This will skip past the sid and a reserved field.
         * Refer to PacketInterface.h for details
         *
         */
        ((STATS_SERVER_ITEM *)pCurrentStats)->sid = pTargetResList->list[i];
        pCurrentStats += 4;     /* Skip sid and rsvd fields */

        /*
         * Execute the MRP.
         */
        rc = PI_ExecuteMRP(pMRPInPkt, sizeof(*pMRPInPkt), MRGETSSTATS,
                           pCurrentStats, sizeof(MRGETSSTATS_RSP), GetGlobalMRPTimeout(),
                           PI_COMMAND_RECORD_TIMEOUT_ACTION_OUTPUT_NONE);

        /*
         * If the request completed successfully, advance the pointer for
         * the next stats record.
         */
        if (rc == PI_GOOD)
        {
            pCurrentStats += sizeof(MRGETSSTATS_RSP);
        }
        else
        {
            /*
             * Some sort of error occurred. Bail out now.
             */
            break;
        }
    }

    /*
     * If the request was successful, set up the response info.
     */
    if (rc == PI_GOOD)
    {
        pRspPacket->pPacket = pRspPkt;
        pRspPacket->pHeader->length = rspPktSize;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = rc;
    }
    else
    {
        /*
         * An error occurred.  Set the response accordingly.
         */
        pRspPacket->pPacket = NULL;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = ((MRGETSSTATS_RSP *)pCurrentStats)->header.status;

        /*
         * Free the response memory.  A DelayedFree() is used because we
         * have allocated a large buffer to hold the output from multiple
         * requests.  If any request times out, DelayedFree() will free
         * the entire buffer in the proper manner.
         */
        DelayedFree(MRGETSSTATS, pRspPkt);
    }

    /*
     * Free other allocated memory.
     */
    Free(pTargetResList);
    Free(pMRPInPkt);

    return (rc);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
