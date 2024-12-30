/* $Id: errorCodes.h 143766 2010-07-06 12:06:32Z m4 $ */
/**
******************************************************************************
**
**  @file   errorCodes.h
**
**  @brief  Header file for CCB error Codes
**
**  Copyright (c) 2001, 2009-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _ERROR_CODES_H_
#define _ERROR_CODES_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
extern      "C"
{
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/*
** VCG Error Codes
**
** NOTE: Changes made here need to be reflected in the CCBE and XSSA code.
**          XSSA - UMC_Errors.properties
**          CCBE - errorCodes.pm
**
** EC_VCG_NO_DRIVES                 - No drives owned by this VCG
** EC_VCG_AS_INVALID_CTRL           - Add Slave - Invalid Contoller
** EC_VCG_AS_FAILED_PING            - Add Slave - Failed to send an IPC_PING
**                                      request to the controller being added
**                                      to the virtual controller group.
** EC_VCG_AS_FAILED_CREATE_CTRL     - Add Slave - Failed to successfully
**                                      execute the create controller MRP.
** EC_VCG_AS_IPC_ADD_CONTROLLER     - Add Slave - Failed to send the add
**                                      controller IPC request to the slave
**                                      or the request failed.
**
** EC_VCG_IC_BAD_MIRROR_PARTNER     - Inactivate Controller - Mirror partner
**                                      of the controller being inactivated
**                                      is not set or is itself.
**
** EC_VCG_IC_NOT_INACTIVATED        - Inactivate Controller - Unable to set
**                                      failure state to inactivated or
**                                      controller was not inactivated after
**                                      election.
**
** EC_VCG_IC_EF                     - Inactivate Controller - Election failed.
**
** EC_VCG_UC_INVALID_CTRL           - Unfail Controller - Failed to read
**                                      the failure data for the controller
**                                      or the failure data showed the
**                                      controller as something other than
**                                      failed.
**
** EC_VCG_IC_INVALID_CTRL           - Inactivate Controller - Invalid Controller
**
** EC_VCG_AL_INVALID_STATE          - Apply License - Invalid power-up state,
**                                      cannot apply license.
**
** EC_VCG_AL_IL_CHG_VCGID           - Apply License - Invalid license, cannot
**                                      change the VCGID.
**
** EC_VCG_AL_IL_RMV_CFG_CTRL        - Apply License - Invalid license, cannot
**                                      remove a configured controller.
**
** EC_VCG_IC_HOLD_IO                - Inactivate Controller - Unable to reset
**                                      Qlogics to Hold off server IO.
**
** EC_VCG_UC_FAILED_PING            - Unfail Controller - Failed to send an
**                                      IPC_PING request to the controller
**                                      being unfailed.
**
** EC_VCG_UC_WRITE_FAILURE          - Unfail Controller - Failed to write
**                                      the failure data for the controller.
**
** EC_VCG_UC_TIMEOUT                - Unfail Controller - Timout waiting for
**                                      the contoller to start power up.
**
** EC_VCG_IC_FAILED_PING            - Inactivate Controller - Failed to send
**                                      an IPC_PING request to the mirror
**                                      partner of the controller being
**                                      inactivated.
**
** EC_VCG_UC_TDISCACHE              - Unfail Controller - Failed to temporarily
**                                      disable cache.
**
** EC_VCG_AC_TDISCACHE              - Add Controller - Failed to temporarily
**                                      disable cache.
**
** EC_VCG_UC_BAD_MIRROR_PARTNER     - Unfail Controller - Master controller
**                                      is not mirroring to itself.  This
**                                      can happen if there are issues flushing
**                                      cache data which is preventing the
**                                      completion of the reallocation processing
**                                      which includes changing the mirror
**                                      partners.  This was detected in defect
**                                      19877 when there were inoperable RAIDs
**                                      during a failover.
**
** NOTE: Changes made here need to be reflected in the CCBE and XSSA code.
**          XSSA - UMC_Errors.properties
**          CCBE - errorCodes.pm
*/
#define EC_VCG_NO_DRIVES                                0x1000
#define EC_VCG_AS_INVALID_CTRL                          0x1001
#define EC_VCG_AS_FAILED_PING                           0x1002
#define EC_VCG_AS_FAILED_CREATE_CTRL                    0x1003
#define EC_VCG_AS_IPC_ADD_CONTROLLER                    0x1004
#define EC_VCG_IC_BAD_MIRROR_PARTNER                    0x1005
#define EC_VCG_IC_NOT_INACTIVATED                       0x1006
#define EC_VCG_IC_EF                                    0x1007
#define EC_VCG_UC_INVALID_CTRL                          0x1008
#define EC_VCG_IC_INVALID_CTRL                          0x1009
#define EC_VCG_AL_INVALID_STATE                         0x1010
#define EC_VCG_AL_IL_CHG_VCGID                          0x1011
#define EC_VCG_AL_IL_RMV_CFG_CTRL                       0x1012
#define EC_VCG_IC_HOLD_IO                               0x1013
#define EC_VCG_UC_FAILED_PING                           0x1014
#define EC_VCG_UC_WRITE_FAILURE                         0x1015
#define EC_VCG_UC_TIMEOUT                               0x1016
#define EC_VCG_IC_FAILED_PING                           0x1017
#define EC_VCG_UC_TDISCACHE                             0x1018
#define EC_VCG_AC_TDISCACHE                             0x1019
#define EC_VCG_IC_TDISCACHE                             0x101A
#define EC_VCG_INVALID_ADDR                             0x101B
#define EC_VCG_ALREADY_CONFIGURED                       0x101C
#define EC_VCG_UC_BAD_MIRROR_PARTNER                    0x101D

/*
** Code Burn Error Codes
*/
#define FWHEADER_VERIFY_ERROR                           0x2001
#define PROGRAM_VERIFY_ERROR                            0x2003
#define ALLOCATION_ERROR                                0x2004
#define PARM_ERROR                                      0x2005
#define MULTI_MAX_EXCEEDED                              0x2006
#define MALLOC_FAILURE                                  0x2007
#define CCB_FLASH_CORRUPTED                             0x2008  /* likely fatal... */
#define UPDATE_PROC_TIMEOUT                             0x2009
#define NVRAM_UPDATE_FAILURE                            0x200A
#define PROBABLY_NO_HEADER                              0x200B
#define CCB_FLASH_ADDRESS_ERROR                         0x200C
#define FILE_TOO_LONG                                   0x200D
#define ILLEGAL_TRY_NON_CCB_FW                          0x200E
#define DISKBAY_STATE_BAD                               0x200F

/*
** File System Error Codes
*/
#define FS_ERROR_WRITE_NULL_BUFFER                      0x2051
#define FS_ERROR_WRITE_DIRECT_INIT                      0x2052
#define FS_ERROR_WRITE_RANGE_LENGTH                     0x2053
#define FS_ERROR_WRITE_HEADER                           0x2054
#define FS_ERROR_WRITE_NO_WRITES_HEADER                 0x2055
#define FS_ERROR_WRITE_HEADER_DATA_SINGLE               0x2056
#define FS_ERROR_WRITE_NO_WRITES_HEADER_DATA_SINGLE     0x2057
#define FS_ERROR_WRITE_HEADER_DATA_LOOP                 0x2058
#define FS_ERROR_WRITE_NO_WRITES_HEADER_DATA_LOOP       0x2059
#define FS_ERROR_READ_NULL_BUFFER                       0x205A
#define FS_ERROR_READ_DIRECT_INIT                       0x205B
#define FS_ERROR_READ_HEADER                            0x205C
#define FS_ERROR_READ_CRC_CHECK_HEADER                  0x205D
#define FS_ERROR_READ_MALLOC_DATA                       0x205E
#define FS_ERROR_READ_DATA                              0x205F
#define FS_ERROR_READ_CRC_CHECK_DATA                    0x2060
#define FS_ERROR_NVRAM_READ_PI_TIMEOUT                  0x2061
#define FS_ERROR_NVRAM_READ                             0x2062
#define FS_ERROR_NVRAM_WRITE_FID_BE_NVRAM_NO_RESPONSE   0x2063
#define FS_ERROR_NVRAM_WRITE_FID_BE_NVRAM_PI_TIMEOUT    0x2064
#define FS_ERROR_NVRAM_WRITE_FID_BE_NVRAM               0x2065
#define FS_ERROR_NVRAM_WRITE_NO_RESPONSE                0x2066
#define FS_ERROR_NVRAM_WRITE_PI_TIMEOUT                 0x2067
#define FS_ERROR_NVRAM_WRITE                            0x2068
#define FS_ERROR_WRITE_HEADER_PI_TIMEOUT                0x2069
#define FS_ERROR_WRITE_HEADER_DATA_SINGLE_PI_TIMEOUT    0x206A
#define FS_ERROR_WRITE_HEADER_DATA_LOOP_PI_TIMEOUT      0x206B
#define FS_ERROR_READ_HEADER_PI_TIMEOUT                 0x206C
#define FS_ERROR_READ_DATA_PI_TIMEOUT                   0x206D
#define FS_ERROR_WRITE_FILE_FULL                        0x206E
#define FS_ERROR_FID2FID_PI_TIMEOUT                     0x2070
#define FS_ERROR_FID2FID_PI_ERROR                       0x2071

/*
** PI_RollingUpdatePhase() Error Codes
*/
#define PHASE_STATE_SET_FAILURE                         0x2100
#define PHASE_FAIL_CONTROLLER_FAILURE                   0x2101
#define PHASE_ELECTION_FAILURE                          0x2102
#define PHASE_BAD_MIRROR_PARTNER                        0x2103
#define PHASE_NON_MIRRORED_RAID5S                       0x2104
#define PHASE_PING_FAILURE                              0x2105
#define PHASE_RAIDS_NOT_READY                           0x2106
#define PHASE_STOP_IO_FAILED                            0x2107
#define PHASE_RESET_QLOGIC_FAILED                       0x2108
#define PHASE_TDISCACHE_FAILED                          0x2109

/*
** Persistent Data Error Codes
*/
#define PDATA_TOO_MUCH_DATA                             0x2210
#define PDATA_OUT_OF_RANGE                              0x2211
#define PDATA_INVALID_OPTION                            0x2212

#define PDATA_RECORD_NOT_FOUND                          0x2213
#define PDATA_DUPLICATE_RECORD                          0x2214
#define PDATA_FILESYSTEM_ERROR                          0x2215
#define PDATA_CANNOT_ALLOCATE_BUFFER                    0x2216
#define PDATA_EOF_REACHED                               0x2217
#define PDATA_INVALID_RECORD                            0x2218
#define PDATA_LOCKED_ERROR                              0x2219
#define PDATA_CANNOT_BACKUP                             0x221A
#define PDATA_SLAVE_DEAD                                0x221B


/*
** PI_VCGShutdown Error Codes
*/
#define VCG_SHUTDOWN_ERROR_WRITE_STATE_TO_QUORUM        0x2301
#define VCG_SHUTDOWN_ERROR_FE_SDIMM_SHUTDOWN            0x2302
#define VCG_SHUTDOWN_ERROR_BE_SDIMM_SHUTDOWN            0x2304
#define VCG_SHUTDOWN_ERROR_CHANGE_NET_ADDRESES          0x2308
#define VCG_SHUTDOWN_ERROR_RESET_FE_INTERFACES          0x2310

/*
** 'Config Journal' Error Codes
*/
#define SNAPSHOT_LOAD_INDEX_OUT_OF_RANGE                0x2401
#define SNAPSHOT_LOAD_NOT_LOADABLE                      0x2402
#define SNAPSHOT_LOAD_NOT_ALL_FIDS_AVAILABLE            0x2403
#define SNAPSHOT_LOAD_ERROR_LOADING_A_FID               0x2404
#define SNAPSHOT_ERROR_NO_EMPTY_SLOTS                   0x2405
#define SNAPSHOT_ERROR_RESTORING_FIDS                   0x2406
#define SNAPSHOT_ERROR_WRITING_DIR                      0x2407
#define SNAPSHOT_REFRESH_DIR_FAILED                     0x2408
#define SNAPSHOT_ERROR_READING_FW_VERSION               0x2409
#define SNAPSHOT_ERROR_RESTORING_BE_NVRAM               0x240A
#define SNAPSHOT_ERROR_RESETTING_CONTROLLERS            0x240B
#define SNAPSHOT_FILESYSTEM_NOT_READY                   0x240C
#define SNAPSHOT_RECOVER_DIR_FAILED                     0x240D

/*
** Miscellaneous Error Codes
*/
#define PI_MD5_ERROR                                    0x3000
#define EC_UNLABEL_ALL_OWNED                            0x3001

/*
** CCB Deadloop Exit Error Codes
** Upper Nibble = Number of Active controllers.
** Rest = Error Code.
*/
#define deadErrCode(a)          (0xA0 | (a))

/* Error Codes */
#define ERR_EXIT_KERNEL         0x00
#define ERR_EXIT_INVALID_USER   0x01
// #define ERR_EXIT_MEM_MAP        0x02     UNUSED
#define ERR_EXIT_SHARED_MEM     0x03

/*----------------------------------------------------------------------------
** Function:    StatusErrorToString
**
** Description: Takes a status code and an error code
**              and converts it to a string.
**
** Inputs:      status      -   Status code.
**              errorCode   -   Error code.
**              strPtr      -   Buffer to place the String.
**
** Outputs:     NONE
**
** Returns:     NONE
**
** NOTE:        THIS FUNCTION SHOULD AT MOST USE (15) CHARACTERS.
**--------------------------------------------------------------------------*/
extern void StatusErrorToString(UINT8 status, UINT32 errorCode, char *strPtr);

#ifdef __cplusplus
}                               /* extern "C" */
#pragma pack(pop)
#endif

#endif                          /* _ERROR_CODES_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
