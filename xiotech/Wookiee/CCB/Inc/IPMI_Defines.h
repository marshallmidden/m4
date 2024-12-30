/* $Id: IPMI_Defines.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       IPMI_Defines.h
**
**  @brief      Header file for generic IPMI definitions - no source file
**
**  Copyright (c) 2004-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _IPMI_DEFINES_H_
#define _IPMI_DEFINES_H_

#include "XIO_Const.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define IPMI_MAX_MESSAGE_SIZE           80

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/* IPMI interface structures */
typedef struct IPMI_INTERFACE_PRIVATE IPMI_INTERFACE;
typedef struct IPMI_BMC_PRIVATE IPMI_BMC;

typedef struct
{
    UINT32      eventSeverity;
    const UINT32 bufferSize;
    char       *const pBuffer;
} IPMI_EVENT;

/* Command Header */
typedef struct
{
    UINT32      commandComplete:1;
    UINT32      sendCommandError:1;
    UINT32      receiveResponseError:1;
    UINT32      reserved:29;
} COMMAND_HEADER_FLAGS_BITS;

typedef volatile union
{
    UINT32      value;
    COMMAND_HEADER_FLAGS_BITS bits;
} COMMAND_HEADER_FLAGS;

typedef struct
{
    UINT8       netFn;
    UINT8       command;
    UINT16      dataLength;
    UINT8      *pData;
} IPMI_MESSAGE;

typedef struct
{
    COMMAND_HEADER_FLAGS flags;
    IPMI_MESSAGE message;
    UINT32      responseLength;
    UINT8      *pResponse;
} COMMAND_HEADER;


/* Command Completion Codes - IPMI v2.0 - Table 5.1 - p38 */
enum IPMI_COMPLETION_CODE_ENUM
{
    IPMI_COMPLETION_CODE_NORMAL = 0x00,
    IPMI_COMPLETION_CODE_OEM_RANGE_LOW = 0x01,
    IPMI_COMPLETION_CODE_OEM_RANGE_HIGH = 0x7E,
    IPMI_COMPLETION_CODE_COMMAND_SPECIFIC_RANGE_LOW = 0x80,
    IPMI_COMPLETION_CODE_COMMAND_SPECIFIC_RANGE_HIGH = 0xBE,
    IPMI_COMPLETION_CODE_NODE_BUSY = 0xC0,
    IPMI_COMPLETION_CODE_INVALID_COMMAND = 0xC1,
    IPMI_COMPLETION_CODE_COMMAND_INVALID_FOR_LUN = 0xC2,
    IPMI_COMPLETION_CODE_COMMAND_TIMEOUT = 0xC3,
    IPMI_COMPLETION_CODE_OUT_OF_SPACE = 0xC4,
    IPMI_COMPLETION_CODE_RESERVATION_CANCELLED = 0xC5,
    IPMI_COMPLETION_CODE_REQUEST_DATA_TRUNCATED = 0xC6,
    IPMI_COMPLETION_CODE_REQUEST_DATA_LENGTH_INVALID = 0xC7,
    IPMI_COMPLETION_CODE_REQUEST_DATA_LENGTH_EXCEEDED = 0xC8,
    IPMI_COMPLETION_CODE_PARAMETER_OUT_OF_RANGE = 0xC9,
    IPMI_COMPLETION_CODE_CANNOT_RETURN_NUMBER_OF_BYTES = 0xCA,
    IPMI_COMPLETION_CODE_REQUESTED_ITEM_NOT_PRESENT = 0xCB,
    IPMI_COMPLETION_CODE_INVALID_DATA_FIELD_IN_REQUEST = 0xCC,
    IPMI_COMPLETION_CODE_ILLEGAL_COMMAND = 0xCD,
    IPMI_COMPLETION_CODE_NO_COMMAND_RESPONSE = 0xCE,
    IPMI_COMPLETION_CODE_CANNOT_EXECUTE_PRIOR_REQUEST = 0xCF,
    IPMI_COMPLETION_CODE_SDR_IN_UPDATE_MODE = 0xD0,
    IPMI_COMPLETION_CODE_DEVICE_IN_FIRMWARE_UPDATE_MODE = 0xD1,
    IPMI_COMPLETION_CODE_BMC_INITIALIZATION_IN_PROGRESS = 0xD2,
    IPMI_COMPLETION_CODE_DESTINATION_UNAVAILABLE = 0xD3,
    IPMI_COMPLETION_CODE_SECURITY_RESTRICTION = 0xD4,
    IPMI_COMPLETION_CODE_COMMAND_NOT_SUPPORTED = 0xD5,
    IPMI_COMPLETION_CODE_COMMAND_DISABLED = 0xD6,
    IPMI_COMPLETION_CODE_UNSPECIFIED = 0xFF,
};

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern UINT32 IPMI_BMCCreate(IPMI_BMC **pBMCPtr, IPMI_INTERFACE *);
extern UINT32 IPMI_BMCConfigureEthernet(IPMI_BMC *);
extern UINT32 IPMI_InterfaceCreate(IPMI_INTERFACE **);
extern UINT32 IPMI_InterfaceSend(IPMI_INTERFACE *, COMMAND_HEADER *);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _IPMI_DEFINES_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
