/* $Id: IPMI_Commands.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       IPMI_Commands.h
**
**  @brief      Header file for IPMI_Commands.c
**
**  Copyright (c) 2004,2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _IPMI_COMMANDS_H_
#define _IPMI_COMMANDS_H_

#include "IPMI_Defines.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/* Network Function Codes - IPMI v2.0 - Table 5.1 - p38 */
#define IPMI_NETFN_SENSOR_EVENT                                 0x04
#define IPMI_NETFN_APP                                          0x06
#define IPMI_NETFN_FIRMWARE                                     0x08
#define IPMI_NETFN_STORAGE                                      0x0A
#define IPMI_NETFN_TRANSPORT                                    0x0C

/* Network Function Codes - IPMI v2.0 - Table 44.11 - p536 */

/* SENSOR_EVENT commands */
#define IPMI_CMD_EVENT_SET_PEF_CONFIGURATION_PARAMETERS         0x12
#define IPMI_CMD_EVENT_GET_PEF_CONFIGURATION_PARAMETERS         0x13

/* APP commands */
#define IPMI_CMD_APP_SET_CHANNEL_ACCESS                         0x40
#define IPMI_CMD_APP_GET_CHANNEL_ACCESS                         0x41

/* TRANSPORT commands */
#define IPMI_CMD_TRANSPORT_SET_LAN_CONFIGURATION_PARAMETERS     0x01
#define IPMI_CMD_TRANSPORT_GET_LAN_CONFIGURATION_PARAMETERS     0x02

/*
******************************************************************************
** Public_defines - data_structures
******************************************************************************
*/

/* Set Channel Access - IPMI v2.0 - Section 22.22 - p288 */

/* Request */
typedef struct
{
    UINT8       channelNumber:4;
    UINT8       reserved:4;
} SCA_CHANNEL_BITS;

typedef union
{
    UINT8       value;
    SCA_CHANNEL_BITS bits;
} SCA_CHANNEL;

typedef struct
{
    UINT8       accessMode:3;
#define ACCESS_MODE_DISABLED            0
#define ACCESS_MODE_PRE_BOOT            1
#define ACCESS_MODE_ALWAYS_OPEN         2
#define ACCESS_MODE_SHARED              3

    UINT8       userLevelAuthDisable:1;
    UINT8       perMessageAuthDisable:1;
    UINT8       pefAlertingDisable:1;
    UINT8       changeType:2;
#define CHANGE_TYPE_NONE                0
#define CHANGE_TYPE_NON_VOLATILE        1
#define CHANGE_TYPE_VOLATILE            2
#define CHANGE_TYPE_RESERVED            3
} SCA_ACCESS_BITS;

typedef union
{
    UINT8       value;
    SCA_ACCESS_BITS bits;
} SCA_ACCESS;

typedef struct
{
    UINT8       levelLimit:4;
#define LEVEL_LIMIT_RESERVED            0
#define LEVEL_LIMIT_CALLBACK            1
#define LEVEL_LIMIT_USER                2
#define LEVEL_LIMIT_OPERATOR            3
#define LEVEL_LIMIT_ADMINISTRATOR       4
#define LEVEL_LIMIT_OEM                 5

    UINT8       reserved:2;
    UINT8       changeType:2;
} SCA_PRIVILEGE_BITS;

typedef union
{
    UINT8       value;
    SCA_PRIVILEGE_BITS bits;
} SCA_PRIVILEGE;

typedef struct
{
    SCA_CHANNEL channel;
    SCA_ACCESS  access;
    SCA_PRIVILEGE privilege;
} SET_CHANNEL_ACCESS_REQUEST;

/* Response */
typedef struct
{
    UINT8       completionCode;
} SET_CHANNEL_ACCESS_RESPONSE;

typedef struct
{
    COMMAND_HEADER header;
    SET_CHANNEL_ACCESS_REQUEST request;
    SET_CHANNEL_ACCESS_RESPONSE response;
} COMMAND_SET_CHANNEL_ACCESS;


/* Get Channel Access - IPMI v2.0 - Section 22.23 - p290 */

/* Request */
typedef struct
{
    UINT8       channelNumber:4;
    UINT8       reserved:4;
} GCA_CHANNEL_BITS;

typedef union
{
    UINT8       value;
    GCA_CHANNEL_BITS bits;
} GCA_CHANNEL;

typedef struct
{
    UINT8       reserved:6;
    UINT8       changeType:2;
} GCA_SETTING_BITS;

typedef union
{
    UINT8       value;
    GCA_SETTING_BITS bits;
} GCA_SETTING;

typedef struct
{
    GCA_CHANNEL channel;
    GCA_SETTING setting;
} GET_CHANNEL_ACCESS_REQUEST;

/* Response */
typedef struct
{
    UINT8       accessMode:3;
    UINT8       userLevelAuthDisable:1;
    UINT8       perMessageAuthDisable:1;
    UINT8       pefAlertingDisable:1;
    UINT8       reserved:2;
} GCA_ACCESS_BITS;

typedef union
{
    UINT8       value;
    GCA_ACCESS_BITS bits;
} GCA_ACCESS;

typedef struct
{
    UINT8       levelLimit:4;
    UINT8       reserved:4;
} GCA_PRIVILEGE_BITS;

typedef union
{
    UINT8       value;
    GCA_PRIVILEGE_BITS bits;
} GCA_PRIVILEGE;

typedef struct
{
    UINT8       completionCode;
    GCA_ACCESS  access;
    GCA_PRIVILEGE privilege;
} GET_CHANNEL_ACCESS_RESPONSE;

typedef struct
{
    COMMAND_HEADER header;
    GET_CHANNEL_ACCESS_REQUEST request;
    GET_CHANNEL_ACCESS_RESPONSE response;
} COMMAND_GET_CHANNEL_ACCESS;


/* Set LAN Configuration - IPMI v2.0 - Section 23.1 - p299 */
#define LCP_PARAMETER_SET_IN_PROGRESS               0
#define LCP_PARAMETER_AUTHENTICATION_TYPE_SUPPORT   1
#define LCP_PARAMETER_AUTHENTICATION_TYPE_ENABLES   2
#define LCP_PARAMETER_IP_ADDRESS                    3
#define LCP_PARAMETER_IP_ADDRESS_SOURCE             4
#define LCP_PARAMETER_MAC_ADDRESS                   5
#define LCP_PARAMETER_SUBNET_MASK                   6
#define LCP_PARAMETER_IPV4_HEADER_PARAMETERS        7
#define LCP_PARAMETER_PRIMARY_RMCP_PORT             8
#define LCP_PARAMETER_SECONDARY_RMCP_PORT           9
#define LCP_PARAMETER_BMC_ARP_CONTROL               10
#define LCP_PARAMETER_GRATUITOUS_ARP_INTERVAL       11
#define LCP_PARAMETER_DEFAULT_GATEWAY_ADDRESS       12
#define LCP_PARAMETER_DEFAULT_GATEWAY_MAC_ADDRESS   13
#define LCP_PARAMETER_BACKUP_GATEWAY_ADDRESS        14
#define LCP_PARAMETER_BACKUP_GATEWAY_MAC_ADDRESS    15
#define LCP_PARAMETER_COMMUNITY_STRING              16
#define LCP_PARAMETER_NUMBER_OF_DESTINATIONS        17
#define LCP_PARAMETER_DESTINATION_TYPE              18
#define LCP_PARAMETER_DESTINATION_ADDRESSES         19
#define LCP_PARAMETER_VLAN_ID                       20
#define LCP_PARAMETER_VLAN_PRIORITY                 21
#define LCP_PARAMETER_RMCP_MESSAGING_SUPPORT        22
#define LCP_PARAMETER_RMCP_MESSAGING_ENTRIES        23
#define LCP_PARAMETER_RMCP_MESSAGING_PRIVILEGE      24
#define LCP_PARAMETER_VLAN_TAGS                     25
#define LCP_PARAMETER_OEM_RANGE_LOW                 192
#define LCP_PARAMETER_OEM_RANGE_HIGH                255

/* Parameter number 0 */
typedef struct
{
    UINT8       actionFlag:2;
#define LCP_ACTION_FLAG_COMPLETE        0
#define LCP_ACTION_FLAG_IN_PROGRESS     1
#define LCP_ACTION_FLAG_COMMIT_WRITE    2
#define LCP_ACTION_FLAG_RESERVED        3

    UINT8       reserved:6;
} LCP_SET_IN_PROGRESS;

/* Parameter number 1 */
typedef struct
{
    UINT8       none:1;
    UINT8       MD2:1;
    UINT8       MD5:1;
    UINT8       reserved_3:1;
    UINT8       passwordKey:1;
    UINT8       oem:1;
    UINT8       reserved_6:2;
} LCP_AUTHENTICATION_TYPE_BITS;

typedef union
{
    UINT8       value;
    LCP_AUTHENTICATION_TYPE_BITS bits;
} LCP_AUTHENTICATION_TYPE;

typedef struct
{
    LCP_AUTHENTICATION_TYPE authenticationType;
} LCP_AUTHENTICATION_TYPE_SUPPORT;

/* Parameter number 2 */
typedef struct
{
    LCP_AUTHENTICATION_TYPE callbackLevel;
    LCP_AUTHENTICATION_TYPE userLevel;
    LCP_AUTHENTICATION_TYPE operatorLevel;
    LCP_AUTHENTICATION_TYPE administratorLevel;
    LCP_AUTHENTICATION_TYPE oemLevel;
} LCP_AUTHENTICATION_TYPE_ENABLES;

/* Parameter number 3 */
typedef struct
{
    UINT8       ipAddress1_MSB;
    UINT8       ipAddress2;
    UINT8       ipAddress3;
    UINT8       ipAddress4_LSB;
} LCP_IP_ADDRESS;

/* Parameter number 4 */
typedef struct
{
    UINT8       source:4;
#define LCP_IP_ADDRESS_SOURCE_UNDEFINED     0
#define LCP_IP_ADDRESS_SOURCE_STATIC        1
#define LCP_IP_ADDRESS_SOURCE_DHCP          2
#define LCP_IP_ADDRESS_SOURCE_SYSTEM        3
#define LCP_IP_ADDRESS_SOURCE_OTHER         4

    UINT8       reserved_4:4;
} LCP_IP_ADDRESS_SOURCE_BITS;

typedef union
{
    UINT8       value;
    LCP_IP_ADDRESS_SOURCE_BITS bits;
} LCP_IP_ADDRESS_SOURCE;

/* Parameter number 5 */
typedef struct
{
    UINT8       macAddress1_MSB;
    UINT8       macAddress2;
    UINT8       macAddress3;
    UINT8       macAddress4;
    UINT8       macAddress5;
    UINT8       macAddress6_LSB;
} LCP_MAC_ADDRESS;

/* Parameter number 6 */
typedef struct
{
    UINT8       subnetMask1_MSB;
    UINT8       subnetMask2;
    UINT8       subnetMask3;
    UINT8       subnetMask4_LSB;
} LCP_SUBNET_MASK;

/* Parameter number 7 */
typedef struct
{
    UINT8       data[3];
} LCP_IPV4_HEADER_PARAMETERS;

/* Parameter number 8 */
typedef struct
{
    UINT8       data[2];
} LCP_PRIMARY_RMCP_PORT;

/* Parameter number 9 */
typedef struct
{
    UINT8       data[2];
} LCP_SECONDARY_RMCP_PORT;

/* Parameter number 10 */
typedef struct
{
    UINT8       arpResponseEnabled:1;
    UINT8       gratuitousARPEnabled:1;
    UINT8       reserved:6;
} LCP_BMC_ARP_CONTROL;

/* Parameter number 11 */
typedef struct
{
    UINT8       data[1];
} LCP_GRATUITOUS_ARP_INTERVAL;

/* Parameter number 12 */
typedef struct
{
    UINT8       ipAddress1_MSB;
    UINT8       ipAddress2;
    UINT8       ipAddress3;
    UINT8       ipAddress4_LSB;
} LCP_DEFAULT_GATEWAY_ADDRESS;

/* Parameter number 13 */
typedef struct
{
    UINT8       macAddress1_MSB;
    UINT8       macAddress2;
    UINT8       macAddress3;
    UINT8       macAddress4;
    UINT8       macAddress5;
    UINT8       macAddress6_LSB;
} LCP_DEFAULT_GATEWAY_MAC_ADDRESS;

/* Parameter number 14 */
typedef struct
{
    UINT8       ipAddress1_MSB;
    UINT8       ipAddress2;
    UINT8       ipAddress3;
    UINT8       ipAddress4_LSB;
} LCP_BACKUP_GATEWAY_ADDRESS;

/* Parameter number 15 */
typedef struct
{
    UINT8       macAddress1_MSB;
    UINT8       macAddress2;
    UINT8       macAddress3;
    UINT8       macAddress4;
    UINT8       macAddress5;
    UINT8       macAddress6_LSB;
} LCP_BACKUP_GATEWAY_MAC_ADDRESS;

/* Parameter number 16 */
typedef struct
{
    UINT8       data[18];
} LCP_COMMUNITY_STRING;

/* Parameter number 17 */
typedef struct
{
    UINT8       data[1];
} LCP_NUMBER_OF_DESTINATIONS;

/* Parameter number 18 */
typedef struct
{
    UINT8       data[4];
} LCP_DESTINATION_TYPE;

/* Parameter number 19 */
typedef struct
{
    UINT8       data[13];
} LCP_DESTINATION_ADDRESSES;

/* Parameter number 20 */
typedef struct
{
    UINT8       data[1];
} LCP_VLAN_ID;

/* Parameter number 21 */
typedef struct
{
    UINT8       data[1];
} LCP_VLAN_PRIORITY;

/* Parameter number 22 */
typedef struct
{
    UINT8       data[1];
} LCP_RMCP_MESSAGING_SUPPORT;

/* Parameter number 23 */
typedef struct
{
    UINT8       data[17];
} LCP_RMCP_MESSAGING_ENTRIES;

/* Parameter number 24 */
typedef struct
{
    UINT8       data[9];
} LCP_RMCP_MESSAGING_PRIVILEGE;

/* Parameter number 25 */
typedef struct
{
    UINT8       data[4];
} LCP_VLAN_TAGS;

/* Parameter Collection (union) */
typedef union
{
    LCP_SET_IN_PROGRESS setInProgress;  /*  0 */
    LCP_AUTHENTICATION_TYPE_SUPPORT authenticationTypeSupport;  /*  1 */
    LCP_AUTHENTICATION_TYPE_ENABLES authenticationTypeEnables;  /*  2 */
    LCP_IP_ADDRESS ipAddress;   /*  3 */
    LCP_IP_ADDRESS_SOURCE ipAddressSource;      /*  4 */
    LCP_MAC_ADDRESS macAddress; /*  5 */
    LCP_SUBNET_MASK subnetMask; /*  6 */
    LCP_IPV4_HEADER_PARAMETERS ipv4HeaderParameters;    /*  7 */
    LCP_PRIMARY_RMCP_PORT primaryRMCPPort;      /*  8 */
    LCP_SECONDARY_RMCP_PORT secondaryRMCPPort;  /*  9 */
    LCP_BMC_ARP_CONTROL bmcARPControl;  /* 10 */
    LCP_GRATUITOUS_ARP_INTERVAL gratuitousARPInterval;  /* 11 */
    LCP_DEFAULT_GATEWAY_ADDRESS defaultGatewayAddress;  /* 12 */
    LCP_DEFAULT_GATEWAY_MAC_ADDRESS defaultGatewayMACAddress;   /* 13 */
    LCP_BACKUP_GATEWAY_ADDRESS backupGatewayAddress;    /* 14 */
    LCP_BACKUP_GATEWAY_MAC_ADDRESS backupGatewayMACAddress;     /* 15 */
    LCP_COMMUNITY_STRING communityString;       /* 16 */
    LCP_NUMBER_OF_DESTINATIONS numberOfDestinations;    /* 17 */
    LCP_DESTINATION_TYPE destinationType;       /* 18 */
    LCP_DESTINATION_ADDRESSES destinationAddress;       /* 19 */
    LCP_VLAN_ID vlanID;         /* 20 */
    LCP_VLAN_PRIORITY vlanPriority;     /* 21 */
    LCP_RMCP_MESSAGING_SUPPORT rmcpMessagingSupport;    /* 22 */
    LCP_RMCP_MESSAGING_ENTRIES rmcpMessagingEntries;    /* 23 */
    LCP_RMCP_MESSAGING_PRIVILEGE rmcpMessagingPrivilege;        /* 24 */
    LCP_VLAN_TAGS vlanTags;     /* 25 */
} LAN_CONFIG_PARAMETER_UNION;

/* Request */
typedef struct
{
    UINT8       number:4;
    UINT8       reserved:4;
} SET_LAN_CONFIG_CHANNEL;

typedef struct
{
    SET_LAN_CONFIG_CHANNEL channel;
    UINT8       parameterSelector;
    LAN_CONFIG_PARAMETER_UNION parameterUnion;
} SET_LAN_CONFIG_REQUEST;

/* Response */
typedef struct
{
    UINT8       completionCode;
} SET_LAN_CONFIG_RESPONSE;

typedef struct
{
    COMMAND_HEADER header;
    SET_LAN_CONFIG_REQUEST request;
    SET_LAN_CONFIG_RESPONSE response;
} COMMAND_SET_LAN_CONFIG;


/* Set PEF Configuration - IPMI v2.0 - Section 30.4 - p377 */
#define PEFCP_PARAMETER_SET_IN_PROGRESS                 0
#define PEFCP_PARAMETER_PEF_CONTROL                     1
#define PEFCP_PARAMETER_PEF_ACTION_GLOBAL_CONTROL       2
#define PEFCP_PARAMETER_PEF_STARTUP_DELAY               3
#define PEFCP_PARAMETER_PEF_ALERT_STARTUP_DELAY         4
#define PEFCP_PARAMETER_NUMBER_OF_EVENT_FILTERS         5
#define PEFCP_PARAMETER_EVENT_FILTER_TABLE              6
#define PEFCP_PARAMETER_EVENT_FILTER_TABLE_DATA         7
#define PEFCP_PARAMETER_NUMBER_OF_ALERT_POLICY_ENTRIES  8
#define PEFCP_PARAMETER_ALERT_POLICY_TABLE              9
#define PEFCP_PARAMETER_SYSTEM_GUID                     10
#define PEFCP_PARAMETER_NUMBER_OF_ALERT_STRINGS         11
#define PEFCP_PARAMETER_ALERT_STRING_KEYS               12
#define PEFCP_PARAMETER_ALERT_STRINGS                   13
#define PEFCP_PARAMETER_NUMBER_OF_GROUP_CONTROL_ENTRIES 14
#define PEFCP_PARAMETER_GROUP_CONTROL_TABLE             15
#define PEFCP_PARAMETER_OEM_RANGE_LOW                   192
#define PEFCP_PARAMETER_OEM_RANGE_HIGH                  255

/* Parameter number 0 */
typedef struct
{
    UINT8       actionFlag:2;
#define PEFCP_ACTION_FLAG_COMPLETE      0
#define PEFCP_ACTION_FLAG_IN_PROGRESS   1
#define PEFCP_ACTION_FLAG_COMMIT_WRITE  2
#define PEFCP_ACTION_FLAG_RESERVED      3

    UINT8       reserved:6;
} PEFCP_SET_IN_PROGRESS;

/* Parameter number 1 */
typedef struct
{
    UINT8       pefEnabled:1;
    UINT8       eventMessagesEnabled:1;
    UINT8       startupDelayDisabled:1;
    UINT8       alertStartupDelayEnabled:1;
    UINT8       reserved:4;
} PEFCP_PEF_CONTROL_BITS;

typedef union
{
    UINT8       value;
    PEFCP_PEF_CONTROL_BITS bits;
} PEFCP_PEF_CONTROL;

/* Parameter number 2 */
typedef struct
{
    UINT8       data[1];
} PEFCP_PEF_ACTION_GLOBAL_CONTROL;

/* Parameter number 3 */
typedef struct
{
    UINT8       data[1];
} PEFCP_PEF_STARTUP_DELAY;

/* Parameter number 4 */
typedef union
{
    UINT8       data[1];
} PEFCP_PEF_ALERT_STARTUP_DELAY;

/* Parameter number 5 */
typedef struct
{
    UINT8       data[1];
} PEFCP_NUMBER_OF_EVENT_FILTERS;

/* Parameter number 6 */
typedef struct
{
    UINT8       data[21];
} PEFCP_EVENT_FILTER_TABLE;

/* Parameter number 7 */
typedef struct
{
    UINT8       data[2];
} PEFCP_EVENT_FILTER_TABLE_DATA;

/* Parameter number 8 */
typedef struct
{
    UINT8       data[1];
} PEFCP_NUMBER_OF_ALERT_POLICY_ENTRIES;

/* Parameter number 9 */
typedef struct
{
    UINT8       data[4];
} PEFCP_ALERT_POLICY_TABLE;

/* Parameter number 10 */
typedef struct
{
    UINT8       data[17];
} PEFCP_SYSTEM_GUID;

/* Parameter number 11 */
typedef struct
{
    UINT8       data[1];
} PEFCP_NUMBER_OF_ALERT_STRINGS;

/* Parameter number 12 */
typedef struct
{
    UINT8       data[3];
} PEFCP_ALERT_STRING_KEYS;

/* Parameter number 13 */
typedef struct
{
    UINT8       data[18];
} PEFCP_ALERT_STRINGS;

/* Parameter number 14 */
typedef struct
{
    UINT8       data[1];
} PEFCP_NUMBER_OF_GROUP_CONTROL_ENTRIES;

/* Parameter number 15 */
typedef struct
{
    UINT8       data[11];
} PEFCP_GROUP_CONTROL_TABLE;

/* Parameter Collection (union) */
typedef union
{
    PEFCP_SET_IN_PROGRESS setInProgress;
    PEFCP_PEF_CONTROL pefControl;
    PEFCP_PEF_ACTION_GLOBAL_CONTROL actionGlobalControl;
    PEFCP_PEF_STARTUP_DELAY startupDelay;
    PEFCP_PEF_ALERT_STARTUP_DELAY alertStartupDelay;
    PEFCP_NUMBER_OF_EVENT_FILTERS numberOfEventFilters;
    PEFCP_EVENT_FILTER_TABLE eventFilterTable;
    PEFCP_EVENT_FILTER_TABLE_DATA eventFilterTableData;
    PEFCP_NUMBER_OF_ALERT_POLICY_ENTRIES numberOfAlertPolicyEntries;
    PEFCP_ALERT_POLICY_TABLE alertPolicyTable;
    PEFCP_SYSTEM_GUID systemGUID;
    PEFCP_NUMBER_OF_ALERT_STRINGS numberOfAlertStrings;
    PEFCP_ALERT_STRING_KEYS alertStringKeys;
    PEFCP_ALERT_STRINGS alertStrings;
    PEFCP_NUMBER_OF_GROUP_CONTROL_ENTRIES numberOfGroupControlEntries;
    PEFCP_GROUP_CONTROL_TABLE groupControlTable;
} PEF_CONFIG_PARAMETER_UNION;

/* Request */
typedef struct
{
    UINT8       parameter:7;
    UINT8       reserved:1;
} SET_PEF_CONFIG_PARAMETER_SELECTOR;

typedef struct
{
    SET_PEF_CONFIG_PARAMETER_SELECTOR parameterSelector;
    PEF_CONFIG_PARAMETER_UNION parameterUnion;
} SET_PEF_CONFIG_REQUEST;

/* Response */
typedef struct
{
    UINT8       completionCode;
} SET_PEF_CONFIG_RESPONSE;

typedef struct
{
    COMMAND_HEADER header;
    SET_PEF_CONFIG_REQUEST request;
    SET_PEF_CONFIG_RESPONSE response;
} COMMAND_SET_PEF_CONFIG;


/* Get PEF Configuration - IPMI v2.0 - Section 30.4 - p377 */

/* Request */
typedef struct
{
    UINT8       parameter:7;
    UINT8       getRevisionOnly:1;
} GET_PEF_CONFIG_PARAMETER_SELECTOR;

typedef struct
{
    GET_PEF_CONFIG_PARAMETER_SELECTOR parameterSelector;
    UINT8       setSelector;
    UINT8       blockSelector;
} GET_PEF_CONFIG_REQUEST;

/* Response */
typedef struct
{
    UINT8       completionCode;
    UINT8       parameterRevision;
    PEF_CONFIG_PARAMETER_UNION parameterUnion;
} GET_PEF_CONFIG_RESPONSE;

typedef struct
{
    COMMAND_HEADER header;
    GET_PEF_CONFIG_REQUEST request;
    GET_PEF_CONFIG_RESPONSE response;
} COMMAND_GET_PEF_CONFIG;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern UINT32 CommandCheckGoodCompletion(COMMAND_HEADER *);

extern UINT32 CommandSetChannelAccess(IPMI_INTERFACE *, COMMAND_SET_CHANNEL_ACCESS *);

extern UINT32 CommandGetChannelAccess(IPMI_INTERFACE *, COMMAND_GET_CHANNEL_ACCESS *);

extern UINT32 CommandSetLANConfig(IPMI_INTERFACE *, COMMAND_SET_LAN_CONFIG *,
                                  UINT32 requestLength);

extern UINT32 CommandSetPEFConfig(IPMI_INTERFACE *, COMMAND_SET_PEF_CONFIG *,
                                  UINT32 requestLength);

extern UINT32 CommandGetPEFConfig(IPMI_INTERFACE *, COMMAND_GET_PEF_CONFIG *,
                                  UINT32 responseLength);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _IPMI_COMMANDS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
