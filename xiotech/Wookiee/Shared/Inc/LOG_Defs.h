/* $Id: LOG_Defs.h 148621 2010-10-05 17:49:17Z m4 $ */
/**
******************************************************************************
**
**  @file       LOG_Defs.h
**
**  @brief      Log Event constants and data structures.
**
**  Logging constants, data structures and defitions used between
**  the CCB and PROC.
**
**  Copyright (c) 2003-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
/**
**  @defgroup _LOG_DEFS_H_      Log Event Information
**  @{
**/
#ifndef _LOG_DEFS_H_
#define _LOG_DEFS_H_

#include "globalOptions.h"
#include "MR_Defs.h"
#include "XIO_Types.h"
#include "SES_Structs.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
/**
**  @defgroup LOG_EVENT_LEVELS Log Event Levels
**  @{
**/
#define LOG_INFO            0x0000
#define LOG_WARNING         0x4000
#define LOG_ERROR           0x8000
#define LOG_FATAL           0xC000
/* @} */

/**
**  @defgroup LOG_LEGACY_ETYPES Log Event Error Types
**  @{
**/
#define LOG_AS_INFO             0
#define LOG_AS_WARNING          1
#define LOG_AS_ERROR            2
#define LOG_AS_FATAL            3
#define LOG_AS_DEBUG            4
/* @} */

/**
**  @defgroup LOG_EVENT_TYPES Log Event Types
**  @{
**/
#define LOG_HIDDEN          0x2000
#define LOG_DEBUG           0x1000
#define LOG_CUSTOMER        0x0000
/* @} */


/**
**  @defgroup LOG_EVENT_MASKS Log Event Masks
**  @{
**/
#define LOG_SEV_MASK        0xC000
#define LOG_PROP_MASK       0x3000
#define LOG_CODE_MASK       0x0FFF
/* @} */


/**
**  @defgroup LOG_EVENT_CODES Log Event Codes
**  @{
**/
#define LOG_SCRUB_DONE                                  LOG_Debug(0x0000)   /* Scrub complete log event         */
#define LOG_NVRAM_RESTORE                               LOG_Info(0x0001)    /* NVRAM restore log event          */
#define LOG_BOOT_CODE_EVENT_INFO                        LOG_Info(0x0002)    /* Boot code generated event        */
#define LOG_BOOT_COMPLETE                               LOG_Info(0x0003)    /* Basic boot complete log event    */
#define LOG_DEVICE_REMOVED                              LOG_Error(0x0004)   /* Device removed log event         */
#define LOG_DEVICE_INSERTED                             LOG_Info(0x0005)    /* Device inserted log event        */
#define LOG_DEVICE_REATTACED                            LOG_Info(0x0006)    /* Device reattached log event      */
#define LOG_COPY_COMPLETE                               LOG_Info(0x0007)    /* Copy complete log event          */
#define LOG_HOST_OFFLINE                                LOG_Info(0x0008)    /* Host offline                     */
#define LOG_LOG_TEXT_MESSAGE_INFO                       LOG_Info(0x0009)    /* Log a text message               */
#define LOG_BUFFER_BOARDS_ENABLED                       LOG_Info(0x000A)    /* Cache boards are ready           */
#define LOG_CONFIG_CHANGED                              LOG_Debug(0x000B)   /* Configuration Changed            */
#define LOG_CACHE_FLUSH_RECOVER                         LOG_Info(0x000C)    /* Cache Flush recovered            */
#define LOG_FIRMWARE_UPDATE                             LOG_Info(0x000D)    /* Firmware Updated                 */
#define LOG_SERVER_CREATE_OP                            LOG_Debug(0x000E)   /* Create Server                    */
#define LOG_SERVER_DELETE_OP                            LOG_Info(0x000F)    /* Delete Server                    */

#define LOG_SERVER_ASSOC_OP                             LOG_Info(0x0010)    /* Associate Server and VDisk       */
#define LOG_SERVER_DISASSOC_OP                          LOG_Info(0x0011)    /* Disassociate Server and VDisk    */
#define LOG_VDISK_CREATE_OP                             LOG_Info(0x0012)    /* Create Virtual Disk              */
#define LOG_VDISK_DELETE_OP                             LOG_Info(0x0013)    /* Delete Virtual Disk              */
#define LOG_VDISK_EXPAND_OP                             LOG_Info(0x0014)    /* Expand Virtual Disk              */
#define LOG_VDISK_PREPARE_OP                            LOG_Info(0x0015)    /* Prepare Virtual Disk             */
#define LOG_VDISK_CONTROL_OP                            LOG_Info(0x0016)    /* VDisk copy, copy/swap, etc       */
#define LOG_VCG_SET_CACHE_OP                            LOG_Info(0x0017)    /* Vcg Set Cache op                 */
#define LOG_VDISK_SET_ATTRIBUTE_OP                      LOG_Info(0x0018)    /* VDisk Set Attribute op           */
#define LOG_RAID_INIT_OP                                LOG_Info(0x0019)    /* RAID Initialization Op           */
#define LOG_RAID_CONTROL_OP                             LOG_Info(0x001A)    /* RAID Control Op                  */
#define LOG_DEVICE_DELETE_OP                            LOG_Info(0x001B)    /* Delete Device                    */
#define LOG_VLINK_BREAK_LOCK_OP                         LOG_Info(0x001C)    /* VLink Break Lock op              */
#define LOG_PROC_ASSIGN_MIRROR_PARTNER_OP               LOG_Info(0x001D)    /* Assign cache mirror partner      */
#define LOG_DRIVE_BAY_FW_UPDATE                         LOG_Info(0x001E)    /* Drive Bay Firmware Update Rcvd   */
#define LOG_PDISK_LABEL_OP                              LOG_Info(0x001F)    /* Physical Disk Label              */

#define LOG_PDISK_FAIL_OP                               LOG_Info(0x0020)    /* Fail Physical Disk               */
#define LOG_PDISK_DEFRAG_OP                             LOG_Info(0x0021)    /* Physical Disk Defrag             */
#define LOG_PSD_REBUILD_DONE                            LOG_Info(0x0022)    /* PSD rebuild was compeleted       */
#define LOG_PDD_REBUILD_DONE                            LOG_Info(0x0023)    /* Full PDD rebuild was completed   */
#define LOG_HSPARE_DONE                                 LOG_Info(0x0024)    /* Hot spare operation was cmpltd   */
#define LOG_RAID_INIT_DONE                              LOG_Info(0x0025)    /* RAID initialization was cmpltd   */
#define LOG_LOOPUP                                      LOG_Info(0x0026)    /* FC Loop Up                       */
#define LOG_DLOOPUP                                     LOG_Debug(0x0026)   /* FC Loop Up                       */
#define LOG_PORTDBCHANGED                               LOG_Debug(0x0027)   /* Port database changed            */
#define LOG_CONFIGURATION_SESSION_START                 LOG_Info(0x0028)    /* Configuration session connected  */
#define LOG_CONFIGURATION_SESSION_END                   LOG_Info(0x0029)    /* Config session disconnected      */
#define LOG_TARGET_SET_PROPERTIES_OP                    LOG_Info(0x002A)    /* Set Target Properties            */
#define LOG_SES_FAN_ON                                  LOG_Info(0x002B)    /* SES fan turned back on           */
#define LOG_PSD_REBUILD_START                           LOG_Debug(0x002C)   /* PSD rebuild was started          */
#define LOG_EST_VLINK                                   LOG_Info(0x002D)    /* Establish VLink Dg               */
#define LOG_TERM_VLINK                                  LOG_Info(0x002E)    /* Terminate VLink Dg               */
#define LOG_SWP_VLINK                                   LOG_Info(0x002F)    /* Swap a VLink DG                  */

#define LOG_NEW_PATH                                    LOG_Info(0x0030)    /* New Path to XIOtech              */
#define LOG_FW_VERSIONS                                 LOG_Debug(0x0031)   /* Log all firmware versions        */
#define LOG_VLINK_SIZE_CHANGED                          LOG_Info(0x0032)    /* VLink Size Changed               */
#define LOG_PARITY_CHECK_DONE                           LOG_Info(0x0033)    /* Parity scan pass completed       */
#define LOG_DEFRAG_DONE                                 LOG_Info(0x0034)    /* Defrag on a RAID complete        */
#define LOG_I2C_BUS_GOOD                                LOG_Info(0x0035)    /* I2C Bus Good                     */
#define LOG_VCG_INACTIVATE_CONTROLLER_OP                LOG_Info(0x0036)    /* VCG Inactivate Controller Op     */
#define LOG_VCG_ACTIVATE_CONTROLLER_OP                  LOG_Info(0x0037)    /* VCG Activate Controller Op       */
#define LOG_VLINK_CREATE_OP                             LOG_Info(0x0038)    /* Create Virtual Link              */
#define LOG_SERVER_SET_PROPERTIES_OP                    LOG_Info(0x0039)    /* Set Server Properties            */
#define LOG_ETHERNET_LINK_UP                            LOG_Info(0x003A)    /* Ethernet link up or chg state    */
#define LOG_WRITE_FLUSH_COMPLETE                        LOG_Debug(0x003B)   /* Write Cache Flush Complete       */
#define LOG_NVA_RESYNC_FAILED                           LOG_Debug(0x003C)   /* Can't resync the NVA record      */
#define LOG_FS_UPDATE                                   LOG_Debug(0x003D)   /* Drive updated with file system   */
#define LOG_PULSE                                       LOG_Debug(0x003E)   /* System Pulse from Proc           */
#define LOG_CACHE_TAGS_RECOVERED                        LOG_Info(0x003F)    /* Cache tag recovery complete      */

#define LOG_NVRAM_RELOAD                                LOG_Debug(0x0040)   /* NVRAM reload log event           */
#define LOG_IPC_LINK_UP                                 LOG_Info(0x0041)    /* IPC Link up                      */
#define LOG_IPC_BROADCAST                               LOG_Info(0x0042)    /* Broadcasted via IPC to ctrlrs    */
#define LOG_WORKSET_CHANGED                             LOG_Info(0x0043)    /* Change to and X1 Workset         */
#define LOG_INIT_PROC_NVRAM_OP                          LOG_Debug(0x0044)   /* Init Proc NVRAM Op               */
#define LOG_INIT_CCB_NVRAM_OP                           LOG_Info(0x0045)    /* Init CCB NVRAM Op                */
#define LOG_SET_SYSTEM_SERIAL_NUM_OP                    LOG_Info(0x0046)    /* Set System Serial Number Op      */
#define LOG_VCG_PREPARE_SLAVE_OP                        LOG_Info(0x0047)    /* VCG Prepare Slave Op             */
#define LOG_VCG_ADD_SLAVE_OP                            LOG_Info(0x0048)    /* VCG Add Slave Op                 */
#define LOG_VCG_SET_MIRROR_PARTNERS_OP                  LOG_Info(0x0049)    /* VCG Set Mirror Partners Op       */
#define LOG_VCG_START_IO_OP                             LOG_Info(0x004A)    /* VCG Start IO Op                  */
#define LOG_VCG_STOP_IO_OP                              LOG_Info(0x004B)    /* VCG Stop IO Op                   */
#define LOG_VCG_GLOBAL_CACHE_OP                         LOG_Info(0x004C)    /* VCG Global Cache Op              */
#define LOG_VCG_APPLY_LICENSE_OP                        LOG_Info(0x004D)    /* VCG Apply License Op             */
#define LOG_VCG_UNFAIL_CONTROLLER_OP                    LOG_Info(0x004E)    /* VCG Unfail Controller Op         */
#define LOG_VCG_FAIL_CONTROLLER_OP                      LOG_Info(0x004F)    /* VCG Fail Controller Op           */

#define LOG_VCG_REMOVE_CONTROLLER_OP                    LOG_Info(0x0050)    /* VCG Remove Controller Op         */
#define LOG_VCG_SHUTDOWN_OP                             LOG_Info(0x0051)    /* VCG Shutdown Op                  */
#define LOG_FE_QLOGIC_RESET_OP                          LOG_Debug(0x0052)   /* Reset Front End QLogic           */
#define LOG_BE_QLOGIC_RESET_OP                          LOG_Debug(0x0053)   /* Reset Back End QLogic            */
#define LOG_PROC_START_IO_OP                            LOG_Info(0x0054)    /* Proc Start IO                    */
#define LOG_PROC_STOP_IO_OP                             LOG_Info(0x0055)    /* Proc Stop IO                     */
#define LOG_PROC_SYSTEM_NMI                             LOG_Info(0x0056)    /* Proc SystemNMI event             */
#define LOG_ADMIN_SETTIME_OP                            LOG_Debug(0x0057)   /* Set System Time                  */
#define LOG_LOG_FAILURE_EVENT_INFO                      LOG_Info(0x0058)    /* Failure management message       */
#define LOG_SNAPSHOT_RESTORED                           LOG_Info(0x0059)    /* Restore successful               */
#define LOG_CONTROLLERS_READY                           LOG_Info(0x005A)    /* Power-up all controllers ready   */
#define LOG_POWER_UP_COMPLETE                           LOG_Info(0x005B)    /* Power-up sequencing completed    */
#define LOG_RM_EVENT_INFO                               LOG_Info(0x005C)    /* Resource Manager message         */
#define LOG_ADMIN_SET_IP_OP                             LOG_Info(0x005D)    /* Resource Manager message         */
#define LOG_SNAPSHOT_TAKEN                              LOG_Info(0x005E)    /* Capture taken                    */
#define LOG_COPY_SYNC                                   LOG_Info(0x005F)    /* Copy has entered sync state      */

#define LOG_ALL_DEV_MISSING                             LOG_Error(0x0060)   /* All devs missing or inoperable   */
#define LOG_XSSA_LOG_MESSAGE                            LOG_Info(0x0061)    /* A log Message sent by the XSSA   */
#define LOG_DEVICE_FAIL_HS                              LOG_Error(0x0062)   /* Device fail (w/ hot spare)       */
#define LOG_SMART_EVENT                                 LOG_Error(0x0063)   /* SMART enunciation log event      */
#define LOG_SMART_EVENT_WARNING                         LOG_Warning(0x0063) /* SMART enunciation log event      */
#define LOG_SMART_EVENT_INFO                            LOG_Info(0x0063)    /* SMART enunciation log event      */
#define LOG_SPINUP_FAILED                               LOG_Warning(0x0064) /* Drive did not spin up            */
#define LOG_LOG_TEXT_MESSAGE_WARNING                    LOG_Warning(0x0065) /* Log a text message               */
#define LOG_VALIDATION                                  LOG_Error(0x0066)   /* Validation warning               */
#define LOG_CACHE_FLUSH_FAILED                          LOG_Warning(0x0067) /* Cache Flush Failed               */
#define LOG_FIRMWARE_ALERT                              LOG_Warning(0x0068) /* Firmware Alert                   */
#define LOG_LOOP_DOWN                                   LOG_Warning(0x0069) /* Loop Down                        */
#define LOG_CCB_NVRAM_RESTORED                          LOG_Warning(0x006A) /* CCB NVRAM restored from disk     */
#define LOG_SES_DEV_BYPA                                LOG_Warning(0x006B) /* SES dev is bypassed on port A    */
#define LOG_SES_DEV_BYPB                                LOG_Warning(0x006C) /* SES dev is bypassed on port B    */
#define LOG_SES_DEV_OFF                                 LOG_Warning(0x006D) /* SES dev is turned off            */
#define LOG_SES_PS_TEMP_WARN                            LOG_Warning(0x006E) /* SES power supply temp warning    */
#define LOG_SES_VOLTAGE_HI_WARN                         LOG_Warning(0x006F) /* SES volt over high limit warn    */

#define LOG_SES_VOLTAGE_LO_WARN                         LOG_Warning(0x0070) /* SES volt under low limit warn    */
#define LOG_SES_TEMP_WARN                               LOG_Warning(0x0071) /* SES temp over hi limit warn      */
/*
** Following logevent ID (72) is used for ISE Busy/Upgrade. It was used by old Georaid code
** that is removed..Also refer def.inc for its equivalent.
*/
#define LOG_ISE_SPECIAL                                 LOG_Info(0x0072)   /* ISE special events(PAB/Upgrade)   */
#define LOG_LOST_PATH_DEBUG                             LOG_Debug(0x0073)   /* Lost a path to a XIOtech box     */
#define LOG_LOST_PATH                                   LOG_Warning(0x0073) /* Lost a path to a XIOtech box     */
#define LOG_ISP_CHIP_RESET                              LOG_Warning(0x0074) /* ISP Chip reset                   */
#define LOG_CACHE_MIRROR_FAILED                         LOG_Error(0x0075)   /* Cache N-Way mirror failed        */
#define LOG_SINGLE_PATH                                 LOG_Warning(0x0076) /* Only a single path to the dev    */
#define LOG_HOTSPARE_INOP                               LOG_Warning(0x0077) /* Hotspare inoperative             */
#define LOG_ETHERNET_LINK_DOWN                          LOG_Warning(0x0078) /* Ethernet link down               */
#define LOG_MSG_DELETED                                 LOG_Warning(0x0079) /* Dnload deleted - not processed   */
#define LOG_VCG_SHUTDOWN_WARN                           LOG_Warning(0x007A) /* Shutdown Warning                 */
#define LOG_SES_CURRENT_HI_WARN                         LOG_Warning(0x007B) /* SES curr over high limit warn    */
#define LOG_SES_EL_REPORT                               LOG_Warning(0x007C) /* SES elec active ctrl change      */
#define LOG_SES_EL_PRESENT                              LOG_Warning(0x007D) /* SES elec present                 */
#define LOG_LOG_FAILURE_EVENT_WARN                      LOG_Warning(0x007E) /* Failure management message       */
#define LOG_RM_WARN                                     LOG_Warning(0x007F) /* RM warning messages              */

#define LOG_SES_IO_MOD_PULLED                           LOG_Warning(0x0080) /*  SES IO Module Pulled            */
#define LOG_BUFFER_BOARDS_DISABLED_INFO                 LOG_Info(0x0081)    /*  Cache boards are ready          */
#define LOG_BUFFER_BOARDS_DISABLED_WARN                 LOG_Warning(0x0082) /*  Cache boards are ready - warn   */
#define LOG_BUFFER_BOARDS_DISABLED_ERROR                LOG_Error(0x0083)   /*  Cache boards are not ready      */
#define LOG_BOOT_CODE_EVENT_WARN                        LOG_Warning(0x0084) /*  Boot code generated event       */
#define LOG_DEVICE_FAIL_NO_HS                           LOG_Error(0x0085)   /*  Device fail (w/o hot spare)     */
#define LOG_DEVICE_MISSING                              LOG_Error(0x0086)   /*  Device missing log event        */
#define LOG_SERIAL_WRONG                                LOG_Error(0x0087)   /*  Serial Number mismatch          */
#define LOG_NVA_BAD                                     LOG_Error(0x0088)   /*  NVA Bad                         */
#define LOG_LOG_TEXT_MESSAGE_ERROR                      LOG_Error(0x0089)   /*  Log a text message              */
#define LOG_CACHE_DRAM_FAIL                             LOG_Error(0x008A)   /*  Cache DRAM failed log event     */
#define LOG_CACHE_RECOVER_FAIL                          LOG_Error(0x008B)   /*  Cache recovery fault            */
#define LOG_COPY_FAILED                                 LOG_Error(0x008C)   /*  Copy failed log event           */
#define LOG_SES_DEV_FLT                                 LOG_Error(0x008D)   /*  SES device element fault        */
#define LOG_SES_PS_OVER_TEMP                            LOG_Error(0x008E)   /*  SES pwr supply over temp fault  */
#define LOG_SES_PS_AC_FAIL                              LOG_Error(0x008F)   /*  SES pwr supply AC failure       */

#define LOG_SES_PS_DC_FAIL                              LOG_Error(0x0090)   /*  SES power supply DC failure     */
#define LOG_SES_FAN_FAIL                                LOG_Error(0x0091)   /*  SES fan failure                 */
#define LOG_SES_TEMP_FAIL                               LOG_Error(0x0092)   /*  SES temperature failure         */
#define LOG_SES_VOLTAGE_HI                              LOG_Error(0x0093)   /*  SES voltage over high limit     */
#define LOG_SES_VOLTAGE_LO                              LOG_Error(0x0094)   /*  SES voltage under low limits    */
#define LOG_ERR_TRAP                                    LOG_Fatal(0x0095)   /*  FE or BE Error trap             */
#define LOG_FOREIGN_PCI                                 LOG_Error(0x0096)   /*  Foreign PCI device              */
#define LOG_CCB_NVRAM_RESET                             LOG_Error(0x0097)   /*  CCB NVRAM reset to defaults     */
#define LOG_WC_SEQNO_BAD                                LOG_Error(0x0098)   /*  Write cache seq number bad      */
#define LOG_VID_RECOVERY_FAIL                           LOG_Error(0x0099)   /*  Cache recovery for VID failed   */
#define LOG_INVALID_TAG                                 LOG_Error(0x009A)   /*  Invalid tag during Cache recv   */
#define LOG_SES_FAN_OFF                                 LOG_Error(0x009B)   /*  SES fan is turned off           */
#define LOG_SES_PS_DC_OVERVOLT                          LOG_Error(0x009C)   /*  SES pwr supply DC overvolt      */
#define LOG_SES_PS_DC_UNDERVOLT                         LOG_Error(0x009D)   /*  SES pwr supply DC undervolt     */
#define LOG_SES_PS_DC_OVERCURR                          LOG_Error(0x009E)   /*  SES pwr supply DC over curr     */
#define LOG_SES_PS_OFF                                  LOG_Error(0x009F)   /*  SES pwr supply off              */

#define LOG_SES_PS_FAIL                                 LOG_Error(0x00A0)   /* SES power supply failed          */
#define LOG_RSCN                                        LOG_Debug(0x00A1)   /* RSCN received                    */
#define LOG_FILEIO_ERR                                  LOG_Warning(0x00A2) /* Internal File I/O error          */
#define LOG_PROC_NOT_READY                              LOG_Error(0x00A3)   /* Proccessor did not come rdy      */
#define LOG_I2C_BUS_FAIL                                LOG_Error(0x00A4)   /* I2C Bus Hang Failure             */
#define LOG_ILLEGAL_ELECTION_STATE                      LOG_Error(0x00A5)   /* Illegal election state change    */
#define LOG_RAID_ERROR                                  LOG_Error(0x00A6)   /* RAID detected drive error        */
#define LOG_SERIAL_MISMATCH                             LOG_Error(0x00A7)   /* FE/BE serial number mismatch     */
#define LOG_PORT_INIT_FAILED                            LOG_Error(0x00A8)   /* Port initialization failure      */
#define LOG_PORT_EVENT                                  LOG_Debug(0x00A9)   /* Port Event Notification          */
#define LOG_NVRAM_CHKSUM_ERR                            LOG_Error(0x00AA)   /* NVRAM checksum error             */
#define LOG_BATTERY_ALERT                               LOG_Error(0x00AB)   /* SDRAM Battery Not Found          */
#define LOG_FW_UPDATE_FAILED                            LOG_Error(0x00AC)   /* Firmware Updated Failed          */
#define LOG_BE_INITIATOR                                LOG_Error(0x00AD)   /* Back End Initiator Detected      */
#define LOG_LOST_ALL_PATHS                              LOG_Debug(0x00AE)   /* Lost all paths to XIOtech box    */
#define LOG_FS_UPDATE_FAIL                              LOG_Error(0x00AF)   /* File Sys update fail on drive    */

#define LOG_DEVICE_TIMEOUT                              LOG_Error(0x00B0)   /* Device timeout log event         */
#define LOG_SES_EL_FAIL                                 LOG_Error(0x00B1)   /* SES electronics failed           */
#define LOG_SES_LOOPAFAIL                               LOG_Error(0x00B2)   /* SES loop A failed                */
#define LOG_SES_LOOPBFAIL                               LOG_Error(0x00B3)   /* SES loop B failed                */
#define LOG_SES_SPEEDMIS                                LOG_Error(0x00B4)   /* SES speed mismatch               */
#define LOG_SES_FWMISMATCH                              LOG_Error(0x00B5)   /* SES firmware mismatch            */
#define LOG_SES_CURRENT_HI                              LOG_Error(0x00B6)   /* SES current over high limit      */
#define LOG_IPC_LINK_DOWN                               LOG_Error(0x00B7)   /* IPC Link down                    */
#define LOG_PROC_COMM_NOT_READY                         LOG_Error(0x00B8)   /* Processor communication not ready*/
#define LOG_NO_OWNED_DRIVES                             LOG_Error(0x00B9)   /* No owned drives available        */
#define LOG_CTRL_FAILED                                 LOG_Error(0x00BA)   /* Controller is failed             */
#define LOG_CTRL_UNUSED                                 LOG_Error(0x00BB)   /* Controller is not used in the group */
#define LOG_FWV_INCOMPATIBLE                            LOG_Error(0x00BC)   /* Controller firmware versions are incompatible */
#define LOG_LOG_FAILURE_EVENT                           LOG_Error(0x00BD)   /* Failure management message       */
#define LOG_SNAPSHOT_RESTORE_FAILED                     LOG_Error(0x00BE)   /* Restore attempt failed           */
#define LOG_MISSING_DISK_BAY                            LOG_Error(0x00BF)   /* One or more disk bays are missing*/

#define LOG_WAIT_CORRUPT_BE_NVRAM                       LOG_Error(0x00C0)   /* Inital load of BE NVRAM was corrupted */
#define LOG_MISSING_CONTROLLER                          LOG_Error(0x00C1)   /* One or more controllers are missing */
#define LOG_RM_ERROR                                    LOG_Error(0x00C2)   /* RM ERROR class messages          */
#define LOG_BOOT_CODE_EVENT_ERROR                       LOG_Error(0x00C3)   /* Boot code generated event        */
#define LOG_CHANGE_LED                                  LOG_Debug(0x00C4)   /* LED change log event             */
#define LOG_WAIT_DISASTER                               LOG_Error(0x00C5)   /* Controller is in a disaster mode */
#define LOG_CHANGE_NAME                                 LOG_Debug(0x00C6)   /* Async Event Change Name          */
#define LOG_GET_LIST_ERROR                              LOG_Debug(0x00C7)   /* GetList() call failed            */
#define LOG_ELECTION_STATE_CHANGE                       LOG_Debug(0x00C8)   /* Changed election state           */
#define LOG_LOCAL_IMAGE_READY                           LOG_Debug(0x00C9)   /* Local image ready                */
#define LOG_REFRESH_NVRAM                               LOG_Debug(0x00CA)   /* Refresh NVRAM on other ctrlrs    */
#define LOG_MAG_DRIVER_ERR                              LOG_Debug(0x00CB)   /* Error rep by Mag driver layer    */
#define LOG_HOST_NONSENSE                               LOG_Debug(0x00CC)   /* Host Error with no sense data    */
#define LOG_HOST_QLOGIC_ERR                             LOG_Debug(0x00CD)   /* Host Error from Qlogic device    */
#define LOG_RAID_EVENT                                  LOG_Debug(0x00CE)   /* RAID errors, rebuild, hotspare   */
#define LOG_IOCB                                        LOG_Debug(0x00CF)   /* IOCB log event                   */

#define LOG_SINGLE_BIT_ECC                              LOG_Debug(0x00D0)   /* Single bit ecc error detected    */
#define LOG_RESYNC_DONE                                 LOG_Debug(0x00D1)   /* Stripe resync completed          */
#define LOG_SHORT_SCSI_EVENT                            LOG_Debug(0x00D2)   /* Short SCSI log event             */
#define LOG_LONG_SCSI_EVENT                             LOG_Debug(0x00D3)   /* Long SCSI log event              */
#define LOG_HOST_IMED_NOTIFY                            LOG_Debug(0x00D4)   /* Host immediate notify            */
#define LOG_HOST_SENSE_DATA                             LOG_Debug(0x00D5)   /* Host error with sense data       */
#define LOG_ZONE_INQUIRY                                LOG_Debug(0x00D6)   /* Zoning Inquiry                   */
#define LOG_FRAMEDROPPED                                LOG_Debug(0x00D7)   /* Frame Dropped                    */
#define LOG_SOCKET_ERROR                                LOG_Debug(0x00D8)   /* Socket error in user socket      */
#define LOG_NVRAM_WRITTEN                               LOG_Debug(0x00D9)   /* NVRAM was written                */
#define LOG_HBEAT_STOP                                  LOG_Debug(0x00DA)   /* PROC detect missed CCB hrtbeat   */
#define LOG_PARITY_SCAN_REQUIRED                        LOG_Debug(0x00DB)   /* Parity scan is required          */
#define LOG_PDISK_UNFAIL_OP                             LOG_Debug(0x00DC)   /* Un-Fail Physical Disk            */
#define LOG_MB_FAILED                                   LOG_Debug(0x00DD)   /* Mailbox command failed           */
#define LOG_LIP                                         LOG_Debug(0x00DE)   /* LIP Event                        */
#define LOG_IOCBTO                                      LOG_Debug(0x00DF)   /* IOCB Timeout                     */

#define LOG_TASKTOOLONG                                 LOG_Debug(0x00E0)   /* Too long between exchanges       */
#define LOG_VLINK_NAME_CHANGED                          LOG_Debug(0x00E1)   /* Name chg notify - CCB to BEP     */
#define LOG_FILEIO_DEBUG                                LOG_Debug(0x00E2)   /* Internal File I/O *warning*      */
#define LOG_LOG_TEXT_MESSAGE_DEBUG                      LOG_Debug(0x00E3)   /* Log a text message               */
#define LOG_PHY_RETRY                                   LOG_Debug(0x00E4)   /* Physical retry                   */
#define LOG_DVLIST                                      LOG_Debug(0x00E5)   /* Device List sent to online       */
#define LOG_PORT_UP                                     LOG_Debug(0x00E6)   /* Interface up                     */
#define LOG_PHY_ACTION                                  LOG_Debug(0x00E7)   /* Physical retry                   */
#define LOG_NO_LICENSE                                  LOG_Debug(0x00E8)   /* No license applied to this controller */
#define LOG_NO_MIRROR_PARTNER                           LOG_Debug(0x00E9)   /* No Mirror Partner set up         */
#define LOG_LPDN_RETRY                                  LOG_Debug(0x00EA)   /* Loop Down reset retry            */
#define LOG_BOOT_CODE_EVENT_DEBUG                       LOG_Debug(0x00EB)   /* Boot code generated event        */
#define LOG_VALIDATION_DEBUG                            LOG_Debug(0x00EC)   /* Validation debug                 */
#define LOG_LOOP_PRIMITIVE_DEBUG                        LOG_Debug(0x00ED)   /* Loop primitive debug             */
#define LOG_PROC_NAME_DEVICE_OP                         LOG_Info(0x00EE)    /* Change device name               */
#define LOG_HARDWARE_MONITOR_DEBUG                      LOG_Debug(0x00EF)   /* HW Monitor Debug Message         */

#define LOG_HARDWARE_MONITOR_STATUS_INFO                LOG_Info(0x00F0)
#define LOG_CCB_STATUS_INFO                             LOG_Info(0x00F1)
#define LOG_PROC_BOARD_STATUS_INFO                      LOG_Info(0x00F2)
#define LOG_FE_BUFFER_BOARD_STATUS_INFO                 LOG_Info(0x00F3)
#define LOG_BE_BUFFER_BOARD_STATUS_INFO                 LOG_Info(0x00F4)
#define LOG_FE_POWER_SUPPLY_STATUS_INFO                 LOG_Info(0x00F5)
#define LOG_BE_POWER_SUPPLY_STATUS_INFO                 LOG_Info(0x00F6)
/*** 0x00F7 - 0x00FF no longer used ***/

#define LOG_HARDWARE_MONITOR_STATUS_WARN                LOG_Warning(0x0100)
#define LOG_CCB_STATUS_WARN                             LOG_Warning(0x0101)
#define LOG_PROC_BOARD_STATUS_WARN                      LOG_Warning(0x0102)
#define LOG_FE_BUFFER_BOARD_STATUS_WARN                 LOG_Warning(0x0103)
#define LOG_BE_BUFFER_BOARD_STATUS_WARN                 LOG_Warning(0x0104)
#define LOG_FE_POWER_SUPPLY_STATUS_WARN                 LOG_Warning(0x0105)
#define LOG_BE_POWER_SUPPLY_STATUS_WARN                 LOG_Warning(0x0106)
/*** 0x0107 - 0x010F no longer used ***/

#define LOG_HARDWARE_MONITOR_STATUS_ERROR               LOG_Error(0x0110)
#define LOG_CCB_STATUS_ERROR                            LOG_Error(0x0111)
#define LOG_PROC_BOARD_STATUS_ERROR                     LOG_Error(0x0112)
#define LOG_FE_BUFFER_BOARD_STATUS_ERROR                LOG_Error(0x0113)
#define LOG_BE_BUFFER_BOARD_STATUS_ERROR                LOG_Error(0x0114)
#define LOG_FE_POWER_SUPPLY_STATUS_ERROR                LOG_Error(0x0115)
#define LOG_BE_POWER_SUPPLY_STATUS_ERROR                LOG_Error(0x0116)
/*** 0x0117 - 0x011F no longer used ***/

#define LOG_HARDWARE_MONITOR_INFO                       LOG_Info(0x0120)
#define LOG_CCB_PROCESSOR_HW_INFO                       LOG_Info(0x0121)
#define LOG_CCB_EEPROM_HW_INFO                          LOG_Info(0x0122)
#define LOG_CCB_MEMORY_MODULE_HW_INFO                   LOG_Info(0x0123)
/*** 0x0124 - 0x013F no longer used ***/

#define LOG_HARDWARE_MONITOR_WARN                       LOG_Warning(0x0140)
#define LOG_CCB_PROCESSOR_HW_WARN                       LOG_Warning(0x0141)
#define LOG_CCB_EEPROM_HW_WARN                          LOG_Warning(0x0142)
#define LOG_CCB_MEMORY_MODULE_HW_WARN                   LOG_Warning(0x0143)
/*** 0x0144 - 0x015F no longer used ***/

#define LOG_HARDWARE_MONITOR_ERROR                      LOG_Error(0x0160)
#define LOG_CCB_PROCESSOR_HW_ERROR                      LOG_Error(0x0161)
#define LOG_CCB_EEPROM_HW_ERROR                         LOG_Error(0x0162)
#define LOG_CCB_MEMORY_MODULE_HW_ERROR                  LOG_Error(0x0163)
/*** 0x0164 - 0x017f no longer used ***/

#define LOG_DIAG_RESULT                                 LOG_Info(0x0180)    /* Diagnostic Result Record log     */
#define LOG_CCB_MEMORY_HEALTH_ALERT                     LOG_Error(0x0181)   /* CCB memory ECC count is high     */
#define LOG_DISKBAY_REMOVED                             LOG_Warning(0x0182) /* Diskbay removed log event        */
#define LOG_DISKBAY_INSERTED                            LOG_Info(0x0183)    /* Diskbay inserted log event       */
#define LOG_PROC_MEMORY_HEALTH_ALERT                    LOG_Error(0x0184)   /* Proc memory ECC count is high    */
#define LOG_DISKBAY_MOVED                               LOG_Info(0x0185)    /* Diskbay moved log event          */
#define LOG_PCI_CFG_ERR                                 LOG_Error(0x0186)   /* PCI set config error             */
#define LOG_FCM                                         LOG_Warning(0x0187) /* FCAL monitor log event           */
#define LOG_HOTSPARE_DEPLETED                           LOG_Error(0x0188)   /* Hotspares depleted               */
#define LOG_CSTOP_LOG                                   LOG_Debug(0x0189)   /* C$Stop recovery step log         */
#define LOG_CONTROLLER_FAIL                             LOG_Error(0x018A)   /* Controller failed log message    */
#define LOG_DISASTER                                    LOG_Error(0x018B)   /* Controller disaster log message  */
#define LOG_MIRROR_CAPABLE                              LOG_Info(0x018C)    /* Now capable to mirror to MP      */
/* #define LOG_MOVING_TARGETS                              LOG_Debug(0x018D) */   /* Moving Targets log message       */
/* #define LOG_PROC_PRINTF                                 LOG_Debug(0x018E) */   /* Proc code debug printf message   */
#define LOG_DRV_FLT                                     LOG_Debug(0x018F)   /* Drive fault/reset FRU code 0xCC  */

#define LOG_BYPASS_DEVICE                               LOG_Debug(0x0190)   /* Bypass Device action             */
#define LOG_VLINK_OPEN_BEGIN                            LOG_Debug(0x0191)   /* VLink Open Process Beginning     */
#define LOG_VLINK_OPEN_END                              LOG_Debug(0x0192)   /* VLink Open Process Completed     */
#define LOG_RAID5_INOPERATIVE                           LOG_Error(0x0193)   /* RAID 5 Inoperative Log message   */
#define LOG_NVRAM_WRITE_FAIL                            LOG_Error(0x0194)   /* NVRAM write verification failed  */
#define LOG_PARITY_CHECK_RAID                           LOG_Debug(0x0195)   /* Parity Chk RAID Begin, end, term */
#define LOG_ISP_J2                                      LOG_Warning(0x0196) /* ISP 23xx J2 jumper incorrect     */
#define LOG_ORPHAN_DETECTED                             LOG_Error(0x0197)   /* Orphaned RAID detected           */
#define LOG_DEFRAG_VER_DONE                             LOG_Debug(0x0198)   /* Defrag verification step done    */
#define LOG_DEFRAG_OP_COMPLETE                          LOG_Info(0x0199)    /* Defrag verification step done    */
#define LOG_NV_MEM_EVENT                                LOG_Error(0x019A)   /* MM Fatal Event detected          */
#define LOG_WC_FLUSH                                    LOG_Debug(0x019B)   /* Flush Taking Long Log            */
#define LOG_WC_SN_VCG_BAD                               LOG_Error(0x019C)   /* WC - swapped with other CN in VCG*/
#define LOG_WC_SN_BAD                                   LOG_Error(0x019D)   /* WC - swapped with foreign CN     */
#define LOG_WC_NVMEM_BAD                                LOG_Error(0x019E)   /* WC - NV Memory unavailable       */
#define LOG_SES_SBOD_EXT                                LOG_Error(0x019F)   /* SES SBOD extended status change  */

#define LOG_IPMI_EVENT                                  LOG_Info(0x01A0)    /* IPMI Event                       */
#define LOG_WAIT_CACHE_ERROR                            LOG_Error(0x01A1)   /* Cache Error detected at powerup  */
#define LOG_SES_SBOD_STATECODE                          LOG_Error(0x01A2)   /* SES SBOD page80 status code      */
#define LOG_SES_ELEM_CHANGE                             LOG_Error(0x01A3)   /* SES element changed states       */

#define LOG_VDISK_SET_PRIORITY                          LOG_Info(0x01A4)    /* Set VDisk Priority               */

#define LOG_PDISK_SPINDOWN_OP                           LOG_Info(0x01A5)    /* Spindown PDisk                   */
#define LOG_PDISK_FAILBACK_OP                           LOG_Info(0x01A6)    /* PDisk Failback                   */
#define LOG_PDISK_AUTO_FAILBACK_OP                      LOG_Info(0x01A7)    /* PDisk AutoFailback               */

#define LOG_DEVICE_RESET                                LOG_Warning(0x01A8) /* Device reset log event           */

#if ISCSI_CODE
#define LOG_TARGET_UP_OP                                LOG_Info(0x01A9)    /* iSCSI Target came up/went down   */
#define LOG_ISCSI_SET_INFO                              LOG_Info(0x01AA)    /* iSCSI Target Set Info            */
#define LOG_ISCSI_SET_CHAP                              LOG_Info(0x01AB)    /* iSCSI CHAP Set Info              */
#define LOG_SERVER_LOGGED_IN_OP                         LOG_Info(0x01AC)    /* iSCSI Server has Logged in       */
#define LOG_ISCSI_GENERIC                               LOG_Info(0x01AD)    /* iSCSI Generic Log                */
#endif

#define LOG_SET_GEO_LOCATION                            LOG_Info(0x01AE)   /* Set Geo location Code              */
#define LOG_NO_HS_LOCATION_CODE_MATCH                   LOG_Warning(0x01AF) /* No hotspare matching location code */
#define LOG_CLEAR_GEO_LOCATION                          LOG_Info(0x01B0)    /* Clear Geo location Code              */

#define LOG_SCSI_TIMEOUT                                LOG_Error(0x01B2)   /* SCSI command timed out     */
#define LOG_PDATA_CREATE                                LOG_Info(0x01B3)    /* Client Persistent Data Record Created */
#define LOG_PDATA_REMOVE                                LOG_Info(0x01B4)    /* Client Persistent Data Record Deleted */
#define LOG_PDATA_WRITE                                 LOG_Info(0x01B5)    /* Client Persistent Data Record Written */
#define LOG_NO_CONFIGURATION                            LOG_Debug(0x01B6)   /* Controller not configured             */

#define LOG_RB_FAILBACK_REINIT_DRIVE                    LOG_Info(0x01B7)    /* Drive Reinitialization as part of failback operation */
#define LOG_RB_FAILBACK_CTLR_MISMATCH                   LOG_Info(0x01B8)    /* Failback not undertaken since the disk is from a different controller */
#define LOG_AUTO_FAILBACK                               LOG_Info(0x01B9)    /* Failback on the disk completed*/
#define LOG_WRONG_SLOT                                  LOG_Error(0x01BA)   /* Controller in wrong slot */
#define LOG_BAD_CHASIS                                  LOG_Error(0x01BB)   /* Controller hardware error */
#define LOG_ICL_PORT_EVENT                              LOG_Info(0x01BC)    /* ICL port event           */
#define LOG_ISNS_CHANGED                                LOG_Info(0x01BD)    /* ISNS Configuration Changed */
#define LOG_PRES_EVENT                                  LOG_Info(0x01BE)    /* Persistent Reserve Event */
#define LOG_PRES_CHANGE                                 LOG_Info(0x01BF)    /* Persistent Reserve Config Change */

#define LOG_ERROR_GENERIC                               LOG_Error(0x01C0)   /* Generic Error Log                */
#define LOG_WARN_GENERIC                                LOG_Warning(0x01C1) /* Generic Warn Log                 */
#define LOG_INFO_GENERIC                                LOG_Info(0x01C2)    /* Generic Info Log                 */
#define LOG_VDISK_PR_CLR                                LOG_Info(0x01C3)    /* Clear PR info for a VID          */
#define LOG_APOOL_CHANGE                                LOG_Info(0x01C4)    /* Log message about APOOL change.  */
#define LOG_APOOL_CHANGE_I                              LOG_Info(0x01C5)    /* Info message about APOOL change.  */
#define LOG_APOOL_CHANGE_W                              LOG_Warning(0x01C6) /* warn message about APOOL change.  */
#define LOG_APOOL_CHANGE_E                              LOG_Error(0x01C7)   /* error message about APOOL change.  */
#define LOG_APOOL_CHANGE_D                              LOG_Debug(0x01C8)   /* debug message about APOOL change.  */
#define LOG_ISP_FATAL                                   LOG_Error(0x01C9)   /* error qlogic fatal error. */

#define LOG_COPY_LABEL                                  LOG_Info(0x01CA)    /* Info message about copy label/description setting */

#define LOG_SPOOL_CHANGE_I                              LOG_Info(0x01CB)    /* Info message about APOOL change.  */
#define LOG_SPOOL_CHANGE_W                              LOG_Warning(0x01CC) /* warn message about APOOL change.  */
#define LOG_SPOOL_CHANGE_E                              LOG_Error(0x01CD)   /* error message about APOOL change.  */
#define LOG_SPOOL_CHANGE_D                              LOG_Debug(0x01CE)   /* debug message about APOOL change.  */
#define LOG_DRIVE_DELAY                                 LOG_Warning(0x01CF) /* warn  message about failing drive. */

#define LOG_DELAY_INOP                                  LOG_Error(0x01D0)   /* Bypassing this delaying drive will cause inop raids. */
#define LOG_ISE_IP_DISCOVER                             LOG_Info(0x01D1)    /* Info message about the ISE ip discovery */
#define LOG_ISE_ELEM_CHANGE                             LOG_Warning(0x01D2) /* ISE environment change           */
#define LOG_RAID_INOPERATIVE                            LOG_Error(0x01D3)   /* RAID Inoperative Log message     */
#define LOG_ISE_ELEM_CHANGE_I                           LOG_Info(0x01D4)    /* ISE environment change           */
#define LOG_ISE_ELEM_CHANGE_W                           LOG_Warning(0x01D5) /* ISE environment change           */
#define LOG_ISE_ELEM_CHANGE_E                           LOG_Error(0x01D6)   /* ISE environment change           */
#define LOG_AF_ENABLE_DISABLE                           LOG_Info(0x01D7)    /* AutoFailback Enabled or Disabled */
#define LOG_GLOBAL_CACHE_MODE                           LOG_Info(0x01D8)    /* Gloal cache mode                 */

#define LOG_GR_EVENT                                    LOG_Info(0x01D9)    /* GeoRaid related event            */
#define LOG_CM_EVENT                                    LOG_Info(0x01DA)    /* CopyManager related event--IM    */

#define LOG_SCRAMBLE_INFO                               LOG_Info(0x01DB)    /* Mark log scramble */
#define LOG_SCRAMBLE_DEBUG                              LOG_Debug(0x01DB)   /* Mark log scramble */
#define LOG_FILL_INFO                                   LOG_Info(0x01DC)    /* Log filling message */
#define LOG_FILL_DEBUG                                  LOG_Debug(0x01DC)   /* Log filling message */

#define LOG_CONFIGURATION_SESSION_START_DEBUG           LOG_Debug(0x0228)   /* Configuration session connected  */
#define LOG_CONFIGURATION_SESSION_END_DEBUG             LOG_Debug(0x0229)   /* Config session disconnected      */

/* @} */

/**
**  @defgroup AP_EVENT_CODES Log Event Code subtypes for APOOL Change notification
**  @{
**/

#define AP_ASYNC_BUFFER_OK_NOW   1          /* INFO  Message: "Async Buffer #x (VID=y) is now operational" */
#define AP_ASYNC_BUFFER_BAD      0x81       /* ERROR Message: "Async Buffer #x (VID=y) is inoperable" */
#define AP_MOVER_STARTED         2          /* INFO  Message: "Async Remote Data Mover has started" */
#define AP_MOVER_STOPPED         0x82       /* ERROR Message: "Async Remote Data Mover has stopped" */
#define AP_ASYNC_LINKS_OK        3          /* INFO  Message: "Async Links are operational" */
#define AP_ASYNC_LINKS_DOWN      0x83       /* ERROR Message: "Async Links are down" */
#define AP_ASYNC_BUFFER_FULL_OK  0x4        /* INFO  Message: "Async Buffer is <30% full" */
#define AP_ASYNC_BUFFER_FULL_WARN 0x44      /* WARN  Message: "Async Buffer is >50% full" */
#define AP_ASYNC_BUFFER_FULL     0x84       /* ERROR Message: "Async Buffer is 100% full" */
#define AP_ASYNC_BUFFER_EMPTY    5          /* DEBUG Message: "Async Buffer is empty" */
#define AP_ASYNC_BUFFER_SET      6          /* INFO  Message: "Async Buffer %x (VID=%y) is defined" */
#define AP_ASYNC_BUFFER_UNSET    7          /* INFO  Message: "Async Buffer %x (VID=%y) is unset" */
#define AP_ALINK_SET             8          /* INFO  Message: "Async Link %y is defined" */
#define AP_ALINK_UNSET           9          /* INFO  Message: "Async Link %y is unset" */
#define AP_FATAL_ERROR           0xA0       /* ERROR Message: "Unexpected Async Error: Errorcode=%xx" */
#define AP_ASYNC_BUFFER_OWNER_SET     0xa   /* INFO  Message: "Async Buffer %d Ownership Set" */
#define AP_ASYNC_BUFFER_OWNER_UNSET   0xb   /* INFO  Message: "Async Buffer %d Ownership Released" */
#define AP_ASYNC_NVRAM_RESTORED_OK    0xc   /* INFO  Message: "Async NVRAM Restored OK" */
#define AP_ASYNC_NVRAM_RESTORED_BAD   0xd   /* ERROR Message: "Async NVRAM Restore Failed. EC=%d.%d.%d" */
#define AP_ASYNC_NVRAM_UNCONFIGURED   0xe   /* INFO  Message: "Async NVRAM is unconfigured." */
#define AP_ASYNC_NVRAM_INITIALIZED    0xf   /* INFO  Message: "Async NVRAM Initialized." */
#define AP_ASYNC_BUFFER_EXPANDED      0x10  /* INFO  Message: "Async Buffer %d expanded by %ldMB" */
#define AP_ASYNC_BUFFER_NOEXPAND      0x91  /* ERROR Message: "Async Buffer %d at max expands of %d" */
#define AP_BREAK_ALL_ASYNC_MIRRORS    0x92  /* ERROR Message: "Async Buffer %d EC=%d. Breaking Mirrors" */
#define AP_ASYNC_BUFFER_IO_BAD        0x15  /* DEBUG Message: "Async Buffer %d I/O Failed. EC=%d.%d" */

#define SS_BUFFER_OK_NOW   1          /* INFO  Message: "SS Buffer #x (VID=y) is now operational" */
#define SS_BUFFER_BAD      0x81       /* ERROR Message: "SS Buffer #x (VID=y) is inoperable" */
#define SS_BUFFER_FULL_OK  0x4        /* INFO  Message: "SS Buffer is <30% full" */
#define SS_BUFFER_FULL_WARN 0x44      /* WARN  Message: "SS Buffer is >50% full" */
#define SS_BUFFER_FULL     0x84       /* ERROR Message: "SS Buffer is 100% full" */
#define SS_BUFFER_EMPTY    5          /* DEBUG Message: "SS Buffer is empty" */
#define SS_BUFFER_SET      6          /* INFO  Message: "SS Buffer %x (VID=%y) is defined" */
#define SS_BUFFER_UNSET    7          /* INFO  Message: "SS Buffer %x (VID=%y) is unset" */
#define SS_CREATE_GOOD     0x3C       /* INFO  Message: "Batch SS Created (Src=%d, SS=%d)" */
#define SS_FATAL_ERROR          0xA0  /* ERROR Message: "Unexpected SS Error: Errorcode=%xx" */
#define SS_BUFFER_OWNER_SET     0xa   /* INFO  Message: "SS Buffer %d Ownership Set" */
#define SS_BUFFER_OWNER_UNSET   0xb   /* INFO  Message: "SS Buffer %d Ownership Released" */
#define SS_NVRAM_RESTORED_OK    0xc   /* INFO  Message: "SS NVRAM Restored OK" */
#define SS_NVRAM_RESTORED_BAD   0xd   /* ERROR Message: "SS NVRAM Restore Failed. EC=%d.%d.%d" */
#define SS_NVRAM_UNCONFIGURED   0xe   /* INFO  Message: "SS NVRAM is unconfigured." */
#define SS_NVRAM_INITIALIZED    0xf   /* INFO  Message: "SS NVRAM Initialized." */
#define SS_READ_SRC_BAD         0x90  /* ERROR Message: "SS SRC (VID=%y) failed (ec=%d)" */
#define SS_OGER_NV_UPDATE_BAD   0x91  /* ERROR Message: "SS ICL update failed (ec=%d)" */
#define SS_BUFFER_OVERFLOW      0xC1  /* DEBUG Message: "SS BUFFER OVERFLOW (ec=%d)" */
#define SS_CREATE_FAIL          0xCC  /* ERROR Message: "Batch SS Create Failed (Src=%d, SS=%d) (ec=%d)" */
/* @} */

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/
/**
**  @defgroup LOG_EVENT_MACROS Log Event Macros
**  @{
**/
#define LOG_GetCode(a)          ((a) & LOG_CODE_MASK)
#define LOG_GetSev(a)           ((a) & LOG_SEV_MASK)
#define LOG_GetProperties(a)    ((a) & LOG_PROP_MASK)

#define LOG_SetCode(a, b)       ((a) = (LOG_GetSev(a) | \
                                      LOG_GetProperties(a) | \
                                      LOG_GetCode(b)))

#define LOG_Info(a)             (((a) & ~LOG_SEV_MASK) | LOG_INFO)
#define LOG_Warning(a)          (((a) & ~LOG_SEV_MASK) | LOG_WARNING)
#define LOG_Error(a)            (((a) & ~LOG_SEV_MASK) | LOG_ERROR)
#define LOG_Fatal(a)            (((a) & ~LOG_SEV_MASK) | LOG_FATAL)
#define LOG_Hidden(a)           ((a) | LOG_HIDDEN)
#define LOG_Debug(a)            ((a) | LOG_DEBUG)
#define LOG_NotHidden(a)        ((a) & ~LOG_HIDDEN)
#define LOG_NotDebug(a)         ((a) & ~LOG_DEBUG)

#define LOG_SetInfo(a)          (a = ((a & ~LOG_SEV_MASK) | LOG_INFO))
#define LOG_SetWarning(a)       (a = ((a & ~LOG_SEV_MASK) | LOG_WARNING))
#define LOG_SetError(a)         (a = ((a & ~LOG_SEV_MASK) | LOG_ERROR))
#define LOG_SetFatal(a)         (a = ((a & ~LOG_SEV_MASK) | LOG_FATAL))
#define LOG_SetHidden(a)        (a = (a | LOG_HIDDEN))
#define LOG_SetDebug(a)         (a = (a | LOG_DEBUG))
#define LOG_SetNotHidden(a)     (a = (a & ~LOG_HIDDEN))
#define LOG_SetNotDebug(a)      (a = (a & ~LOG_DEBUG))

#define LOG_IsInfo(a)           (LOG_GetSev(a) == LOG_INFO)
#define LOG_IsWarning(a)        (LOG_GetSev(a) == LOG_WARNING)
#define LOG_IsError(a)          (LOG_GetSev(a) == LOG_ERROR)
#define LOG_IsFatal(a)          (LOG_GetSev(a) == LOG_FATAL)
#define LOG_IsHidden(a)         ((LOG_GetProperties(a) & LOG_HIDDEN) > 0)
#define LOG_IsDebug(a)          ((LOG_GetProperties(a) & LOG_DEBUG) > 0)
#define LOG_IsNotHidden(a)      ((LOG_GetProperties(a) & LOG_HIDDEN) == 0)
#define LOG_IsNotDebug(a)       ((LOG_GetProperties(a) & LOG_DEBUG) == 0)
/* @} */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
/**
**  @addtogroup LOG_HEADER_PKT Log Event Header
**  @{
**/
/** Log Event header                                                        */
typedef struct LOG_HEADER_PKT
{
    UINT32  event;                  /**< Event code                         */
    UINT32  length;                 /**< Length                             */
} LOG_HEADER_PKT;
/* @} */

typedef struct LOG_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    UINT8           data[1024];     /**< Log Event Data                     */
} LOG_PKT;

/**
**  @addtogroup LOG_COPY_COMPLETE
**  @{
**/
#define CMCC_OK             0x00    /**< Successful                         */
#define CMCC_ABORT          0x01    /**< Copy Aborted                       */
#define CMCC_COPYERR        0x02    /**< Copy Error                         */
#define CMCC_DESTUPERR      0x03    /**< Destination VDD Update Error       */
#define CMCC_SRCUERR        0x04    /**< Source VDD Update Error            */
#define CMCC_DESTDEL        0x05    /**< Destination VDD Deleted            */
#define CMCC_SRCDEL         0x06    /**< Source VDD Deleted                 */

#define CMCC_MIRROREND      0x11    /**< Mirror Ended                       */
#define CMCC_USRTERM        0x12    /**< Copy Terminated by User            */
#define CMCC_AUTOTERM       0x13    /**< Copy Terminated by CM              */
#define CMCC_CPYSTART       0x14    /**< Copy Started                       */
#define CMCC_CPYMIRROR      0x15    /**< Copy Mirrored                      */
#define CMCC_RAIDSWAP       0x16    /**< RAIDs Swapped                      */
#define CMCC_USRSPND        0x17    /**< User Suspended                     */
#define CMCC_USRRSM         0x18    /**< User Resume                        */
#define CMCC_AUTOSPND       0x19    /**< Auto Suspend                       */
#define CMCC_CPYRSM         0x1A    /**< Copy Resume                        */
#define CMCC_CMSPND         0x1B    /**< Mirror Suspendes by Swap Operation */

/* RSD == RAID Swap Denied */
#define CMCC_RSD_2VL        0x1C    /**< RSD - 2 Vlinks to same MAGNITUDE   */
#define CMCC_RSD_NODV       0x1D    /**< RSD - Device Not Defined           */
#define CMCC_RSD_VL2VL      0x1E    /**< RSD - VLink to VLink Copy          */
#define CMCC_RSD_RSVD       0x1F    /**< RSD - reserved                     */

#define CMCC_AQRDOWNRSHP    0x20    /**< Ownership of copy acquired          */
#define CMCC_OWNRSHPTERM    0x21    /**< Ownership of copy terminated       */
#define CMCC_FORCEOWNRSHP   0x22    /**< Force Ownership of copy            */

typedef struct LOG_COPY_COMPLETE_DAT
{
    UINT8   compstat;
    UINT8   function;
    UINT8   percent;
    UINT8   rsvd;
    UINT16  srcVid;
    UINT16  destVid;
} LOG_COPY_COMPLETE_DAT;

typedef struct LOG_COPY_COMPLETE_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_COPY_COMPLETE_DAT   data;   /**< Copy completion information        */
} LOG_COPY_COMPLETE_PKT;
/* @} */

/*
** Structure used to generate a log event when the label/description copy
** operation record is changed
*/

typedef struct LOG_COPY_LABEL_DAT
{
    UINT16  svid;                /**< Source VID of this copy */
    UINT16  dvid;                /**< Destination VID of this copy */
}LOG_COPY_LABEL_DAT;

typedef struct  LOG_COPY_LABEL_EVENT_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_COPY_LABEL_DAT data;
}LOG_COPY_LABEL_EVENT_PKT;

/**
**  @addtogroup LOG_PDD_REBUILD_DONE_DAT
**  @{
**/
typedef struct LOG_PDD_REBUILD_DONE_DAT
{
    UINT8           rsvd0[4];       /**< RESERVED                           */
    UINT16          lun;            /**< LUN of physical device rebuilt     */
    UINT16          pid;            /**< ID of physical device rebuilt      */
    UINT64          wwn;            /**< WWN of physical device rebuilt     */
} LOG_PDD_REBUILD_DONE_DAT;
/* @} */

/**
**  @addtogroup LOG_PDD_REBUILD_DONE
**  @{
**/
typedef struct LOG_PDD_REBUILD_DONE_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_PDD_REBUILD_DONE_DAT data;  /**< PDD rebuild done information       */
} LOG_PDD_REBUILD_DONE_PKT;
/* @} */

/**
**  @addtogroup LOG_LOOPUP
**  @{
**/
typedef struct LOG_LOOPUP_DAT
{
    UINT8           port;           /**< Port Number                        */
    UINT8           flags;          /**< Flags                              */
    UINT8           proc;           /**< FE (0) or BE (1) indicator         */
    UINT8           failed;         /**< Failed indicator                   */
    UINT16          lid;            /**< Loop ID                            */
    UINT16          state;          /**< login state                        */
    UINT8           lpmap[56];      /**< Loop Map                           */
} LOG_LOOPUP_DAT;
#define PORT_FABRIC_MODE_BIT 0      /* flags field, bit 0 */
/* For the future.  #define PORT_PID_MODE_BIT 1 */         /* flags field, bit 1 - lid is pid */

typedef struct LOG_LOOPUP_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_LOOPUP_DAT  data;           /**< Log Event Data                     */
} LOG_LOOPUP_PKT;
/* @} */

#define LOG_PORTDBCHANGED_DAT       LOG_LOOPUP_DAT
#define LOG_PORTDBCHANGED_PKT       LOG_LOOPUP_PKT


/* Note that these values are also used in the RBR record status field. */
#define RB_ECOK         0           /**< No error (value, not bit)          */
#define RB_ECSPARE      0           /**< 0x0001 - Not used                           */
#define RB_ECINOP       1           /**< 0x0002 - Drive being rebuilt inoperable     */
#define RB_ECBADTYPE    2           /**< 0x0004 - RAID type is not rebuildable       */
#define RB_ECWRFAULT    3           /**< 0x0008 - Write fault on R10                 */
#define RB_ECNOTREADY   4           /**< 0x0010 - R5 not operative                   */
#define RB_ECR5INOP     5           /**< 0x0020 - R5 became inoperable               */
#define RB_ECDELETED    6           /**< 0x0040 - RBR deleted                        */
#define RB_ECCANCEL     7           /**< 0x0080 - RBR cancelled                      */
#define RB_ECDELAY      8           /**< 0x0100 - RBR delayed too long               */
#define RB_ECOWNER      9           /**< 0x0200 - VID ownership changed              */
#define RB_ECDONE       10          /**< 0x0400 - PSD already done rebuilding        */
// NOT USED ANYWHERE -- #define RB_ECDEFRAG     11          /**< 0x0800 - PSD defragging                     */

#define RB_LOGDEBUGMASK ((1<<RB_ECBADTYPE) | \
                         (1<<RB_ECDELETED) | \
                         (1<<RB_ECCANCEL) | \
                         (1<<RB_ECOWNER) | \
                         (1<<RB_ECDONE))

/**
**  @addtogroup LOG_PSD_REBUILD_INFO_DAT
**  @{
**/
typedef struct LOG_PSD_REBUILD_INFO_DAT
{
    UINT16          rid;            /**< RAID ID of RAID being rebuilt      */
    UINT16          vid;            /**< Virtual Disk ID being rebuilt      */
    UINT16          lun;            /**< LUN of physical device             */
    UINT16          errorCode;      /**< Error code of rebuild complete     */
    UINT64          wwn;            /**< World wide name of physical device */
} LOG_PSD_REBUILD_INFO_DAT;
/* @} */


/**
**  @addtogroup LOG_PSD_REBUILD_START
**  @{
**/
typedef struct LOG_PSD_REBUILD_START_PKT
{
    LOG_HEADER_PKT              header;/**< Log Event Header - 8 bytes      */
    LOG_PSD_REBUILD_INFO_DAT    data;  /**< Rebuild PSD information         */
} LOG_PSD_REBUILD_START_PKT;
/* @} */


/**
**  @addtogroup LOG_PSD_REBUILD_DONE
**  @{
**/
#define LOG_PSD_REBUILD_DONE_PKT LOG_PSD_REBUILD_START_PKT
/* @} */

/**
**  @addtogroup LOG_DEFRAG_DONE
**  @{
**/
typedef struct LOG_DEFRAG_DONE_DAT
{
    UINT16  rid;                    /**< RID of defrag completed            */
    UINT16  errCode;                /**< Reserved                           */
    UINT64  wwnPDisk;               /**< WWN of device defragmented         */
} LOG_DEFRAG_DONE_DAT;

typedef struct LOG_DEFRAG_DONE_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_DEFRAG_DONE_DAT data;       /**< Log Event Data                     */
} LOG_DEFRAG_DONE_PKT;
/* @} */

/**
**  @addtogroup LOG_WRITE_FLUSH_COMPLETE
**  @{
**/
typedef struct LOG_WRITE_FLUSH_COMPLETE_DAT
{
    UINT32          vid;            /**< VID = 0xFFFFFFFF if Global         */
} LOG_WRITE_FLUSH_COMPLETE_DAT;

typedef struct LOG_WRITE_FLUSH_COMPLETE_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_WRITE_FLUSH_COMPLETE_DAT data; /**< Log Event Data                  */
} LOG_WRITE_FLUSH_COMPLETE_PKT;
/* @} */

/**
**  @addtogroup LOG_CACHE_TAGS_RECOVERED
**  @{
**/
typedef struct LOG_CACHE_TAGS_RECOVERED_DAT
{
    UINT32          totalDirty;     /**< Total dirty tags flushed           */
    UINT32          totalFailed;    /**< Total bad tags detected            */
} LOG_CACHE_TAGS_RECOVERED_DAT;

typedef struct LOG_CACHE_TAGS_RECOVERED_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_CACHE_TAGS_RECOVERED_DAT data; /**< Log Event Data                  */
} LOG_CACHE_TAGS_RECOVERED_PKT;
/* @} */

/**
**  @addtogroup LOG_ALL_DEV_MISSING
**  @{
**/
typedef struct LOG_ALL_DEV_MISSING_DAT
{
    UINT32   cSerial;
    UINT8    syncVdMap[(MAX_VIRTUAL_DISKS+7)/8];
    UINT8    srcVdMap[(MAX_VIRTUAL_DISKS+7)/8];
}LOG_ALL_DEV_MISSING_DAT;

typedef struct LOG_ALL_DEV_MISSING_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    ZeroArray(LOG_ALL_DEV_MISSING_DAT, data);
} LOG_ALL_DEV_MISSING_PKT;
typedef struct LOG_ISE_SPECIAL_DAT
{
    UINT16  bayID;
    UINT8   busyFlag;              /**< TRUE =  ISE busy
                                        FALSE = ISE back to operational     */
}LOG_ISE_SPECIAL_DAT;

typedef struct LOG_ISE_SPECIAL_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_ISE_SPECIAL_DAT data;
}LOG_ISE_SPECIAL_PKT;
/* @} */

/**
**  @addtogroup LOG_DEVICE_FAIL_HS
**  @{
**/
typedef struct
{
    UINT8   channel;
    UINT8   rsvd1;
    UINT16  lun;
    UINT16  pid;
    UINT16  rsvd2;
    UINT32  lid;
    UINT64  wwn;

    UINT8   hschannel;
    UINT8   rsvd3;
    UINT16  hslun;
    UINT16  hspid;
    UINT16  rsvd4;
    UINT32  hsid;
    UINT64  hswwn;
} LOG_DEVICE_FAIL_HS_DAT;       /*  Device fail (w/ hot spare)          */

typedef struct LOG_DEVICE_FAIL_HS_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_DEVICE_FAIL_HS_DAT data;
} LOG_DEVICE_FAIL_HS_PKT;
/* @} */


/**
 **  @addtogroup LOG_SMART_EVENT_DAT
 **  @{
**/
typedef struct LOG_SMART_EVENT_DAT
{
    UINT8 port;
    UINT8 rsv;
    UINT16 lun;
    UINT32 id;
    UINT64 wwn;
    UINT8  cdb[16];
    UINT8 sense[32];

} LOG_SMART_EVENT_DAT;
/* @} */
/**
**  @addtogroup LOG_SMART_EVENT
**  @{
**/

typedef struct LOG_SMART_EV_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_SMART_EVENT_DAT data;
} LOG_SMART_EV_PKT;
/* @} */

/**
**  @addtogroup LOG_FIRMWARE_ALERT
**  @{
**/
/*
** Firmware Alert Error Code Definitions (unique for every location called)
**
**  CCB is assigned 0x00000000 to 0x7FFFFFFF
*/
#define PROCSFT     0x80000000          /**< Beginning of PROC Error Codes  */

#define DLM_SFT1    PROCSFT+0x00        /**< DLM device capacity too large #1 */
#define DLM_SFT2    PROCSFT+0x01        /**< DLM device capacity too large #2 */
#define DLM_SFT3    PROCSFT+0x02        /**< DLM device capacity too large #3 */
#define DLM_SFT4    PROCSFT+0x03        /**< DLM device capacity too large #4 */
#define DLM_SFT5    PROCSFT+0x04        /**< DLM device capacity too large #5 */
#define DLM_SFT6    PROCSFT+0x05        /**< DLM device capacity too large #6 */
#define DLM_SFT7    PROCSFT+0x06        /**< DLM device capacity too large #7 */
#define DLM_SFT8    PROCSFT+0x07        /**< DLM device capacity too large #8 */
#define DLM_SFT9    PROCSFT+0x08        /**< DLM LDD/VID not corelated #1   */
#define DLM_SFT10   PROCSFT+0x09        /**< DLM LDD/VID not corelated #2   */
#define DLM_SFT11   PROCSFT+0x0A        /**< DLM SDA too large              */
#define DLM_SFT12   PROCSFT+0x0B        /**< DLM VID/LDD not corelated      */
#define DLM_SFT13   PROCSFT+0x0C        /**< DLM device capacity too large #9 */
#define DLM_SFT14   PROCSFT+0x0D        /**< DLM Poll Unexpected Error      */
#define DLM_SFT15   PROCSFT+0x0E        /**< DLM extended SGL looks bad     */
                                        /**< # DLM reserved 15-31           */
#define MAGD_SFT1   PROCSFT+0x20        /**< Magdrvr inquiry VIDs not match */
#define MAGD_SFT2   PROCSFT+0x21        /**< Magdrvr capacity too large #1  */
#define MAGD_SFT3   PROCSFT+0x22        /**< Magdrvr capacity too large #2  */
#define MAGD_SFT4   PROCSFT+0x23        /**< Magdrvr capacity too large #3  */
#define MAGD_SFT5   PROCSFT+0x24        /**< Magdrvr unsupported Receive Diag */
#define MAGD_SFT6   PROCSFT+0x25        /**< Magdrvr unsupported Log Sense  */
#define MAGD_SFT7   PROCSFT+0x26        /**< Magdrvr unsupported Log Select */
#define MAGD_SFT8   PROCSFT+0x27        /**< Magdrvr - build_maci - max VID */
#define MAGD_SFT9   PROCSFT+0x28        /**< Magdrvr - build_maci - max LUN */
#define MAGD_SFT10  PROCSFT+0x29        /**< Magdrvr - build_maci - no VDMT */
#define MAGD_SFT11  PROCSFT+0x2A        /**< Magdrvr - build_maci - used LUN */
                                        /**< Magdrvr reserved 11-31         */
#define CAC_SFT1    PROCSFT+0x40        /**< Cache - Flush all took > 2 min */
#define CAC_SFT2    PROCSFT+0x41        /**< Cache - Invalid VID in BE Tags */
#define CAC_SFT3    PROCSFT+0x42        /**< Cache - More than one Bad BE Tag */
#define CAC_SFT4    PROCSFT+0x43        /**< Cache - SWI ILT not found in list*/
#define CAC_SFT5    PROCSFT+0x44        /**< Cache - SWI too many ILTs      */
#define CAC_SFT6    PROCSFT+0x45        /**< Cache - BE Tag overlaps other Tag*/
#define CAC_SFT7    PROCSFT+0x46        /**< Cache - Rebuild Check Unexp Error*/
#define CAC_SFT8    PROCSFT+0x47        /**< Cache - Mirror BE NV Card Wrong */
#define CAC_SFT9    PROCSFT+0x48        /**< Cache - WRP invalid function   */
#define CAC_SFT10   PROCSFT+0x49        /**< Cache - WRP invalid VID        */
#define CAC_SFT11   PROCSFT+0x4A        /**< Cache - ILT not on Queue       */
#define CAC_SFT12   PROCSFT+0x4B        /**< Cache - Invalid WRP VID        */
                                        /**<  Cache reserved 12-31          */
#define ISP_SFT1    PROCSFT+0x60        /**< ISP - Chip reset               */
                                        /**<  ISP reserved 1-31             */
#define NVA_SFT1    PROCSFT+0x80        /**< NVA - Len too large to handle  */
                                        /**<  NVA reserved 1-31             */

typedef struct LOG_FIRMWARE_ALERT_DAT
{
    UINT32          errorCode;      /**< Error Code                         */
    UINT32          data[15];       /**< Data specific to the error         */
} LOG_FIRMWARE_ALERT_DAT;

typedef struct LOG_FIRMWARE_ALERT_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_FIRMWARE_ALERT_DAT data;    /**< Log Event Data                     */
} LOG_FIRMWARE_ALERT_PKT;
/* @} */

/**
**  @addtogroup LOG_CACHE_MIRROR_FAILED
**  @{
**/
typedef struct LOG_CACHE_MIRROR_FAILED_DAT
{
    UINT32          controllerSN;   /**< Controller Serial Number           */
    UINT32          ilt_status;     /**< The g0 status value                */
    UINT8           drp_status;     /**< DRP status code                    */
    UINT8           dg_status;      /**< Datagram status code               */
    UINT8           dg_ec1;         /**< Datagram Error Code #1             */
    UINT8           dg_ec2;         /**< Datagram Error Code #2             */
} LOG_CACHE_MIRROR_FAILED_DAT;

typedef struct LOG_CACHE_MIRROR_FAILED_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_CACHE_MIRROR_FAILED_DAT data; /**< Log Event Data                   */
} LOG_CACHE_MIRROR_FAILED_PKT;
/* @} */

/**
**  @addtogroup LOG_CACHE_MIRROR_FAILED
**  @{
**/
typedef struct
{
    UINT32  controllerSN;
} LOG_CACHE_MIRROR_CAPABLE_DAT;

typedef struct LOG_CACHE_MIRROR_CAPABLE_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_CACHE_MIRROR_CAPABLE_DAT data; /**< Log Event Data                  */
} LOG_CACHE_MIRROR_CAPABLE_PKT;
/* @} */

/**
**  @addtogroup LOG_DEVICE_FAIL_NO_HS
**  @{
**/
typedef struct
{
    UINT8   channel;
    UINT8   rsvd1;
    UINT16  lun;
    UINT16  pid;
    UINT16  rsvd2;
    UINT16  rid;
    UINT16  vid;
    UINT32  lid;
    UINT64  wwn;
} LOG_DEVICE_FAIL_NO_HS_DAT;        /*  Device fail (w/o hot spare)         */


typedef struct LOG_DEVICE_FAIL_NO_HS_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_DEVICE_FAIL_NO_HS_DAT data;
} LOG_DEVICE_FAIL_NO_HS_PKT;
/* @} */

/**
**  @addtogroup LOG_WC_SEQNO_BAD
**  @{
**/
typedef struct LOG_WC_SEQNO_BAD_DAT
{
    UINT32          cacheId;        /**< Cache identifier                   */
    UINT32          sysSeq;         /**< System sequence number             */
    UINT32          seq;            /**< Cache sequence                     */
} LOG_WC_SEQNO_BAD_DAT;

typedef struct LOG_WC_SEQNO_BAD_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_WC_SEQNO_BAD_DAT data;      /**< Log Event Data                     */
} LOG_WC_SEQNO_BAD_PKT;
/* @} */


/**
**  @addtogroup LOG_VID_RECOVERY_FAIL
**  @{
**/
typedef struct LOG_VID_RECOVERY_FAIL_DAT
{
    UINT32          vid;            /**< Vid Number                         */
    UINT32          count;          /**< Error count                        */
} LOG_VID_RECOVERY_FAIL_DAT;

typedef struct LOG_VID_RECOVERY_FAIL_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_VID_RECOVERY_FAIL_DAT data; /**< Log Event Data                     */
} LOG_VID_RECOVERY_FAIL_PKT;
/* @} */

/**
**  @addtogroup LOG_RSCN
**  @{
**/
typedef struct LOG_RSCN_DAT
{
    UINT8           port;           /**< Port Number                        */
    UINT8           rsvd9[1];       /**< RESERVED                           */
    UINT8           proc;           /**< FE (0) or BE (1) indicator         */
    UINT8           addressFormat;  /**< address format                     */
    UINT32          portId;         /**< N_Port ID                          */
} LOG_RSCN_DAT;

typedef struct LOG_RSCN_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_RSCN_DAT    data;           /**< Log Event Data                     */
} LOG_RSCN_PKT;
/* @} */

/**
**  @addtogroup LOG_RAID_ERROR
**  @{
**/
typedef struct LOG_RAID_ERROR_DAT
{
    UINT16  lun;
    UINT16  vid;
    UINT16  rid;
    UINT16  pid;        /* PID of this device (good for this boot only)  */
    UINT64  wwnPdisk;   /* Device with error                             */
} LOG_RAID_ERROR_DAT;

typedef struct LOG_RAID_ERROR_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_RAID_ERROR_DAT data;
} LOG_RAID_ERROR_PKT;
/* @} */

/**
**  @addtogroup LOG_PORT_EVENT
**  @{
**/
typedef struct LOG_PORT_EVENT_DAT
{
    UINT8           port;           /**< Port Number                        */
    UINT8           rsvd9[1];       /**< RESERVED                           */
    UINT8           proc;           /**< FE (0) or BE (1) indicator         */
    UINT8           rsvd11[1];      /**< RESERVED                           */
    UINT32          reason;         /**< Reason Code                        */
    UINT32          count;          /**< Count                              */
} LOG_PORT_EVENT_DAT;

typedef struct LOG_PORT_EVENT_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_PORT_EVENT_DAT data;        /**< Log Event Data                     */
} LOG_PORT_EVENT_PKT;
/* @} */

/***** GEORAID  ****START*/
/**
**  @addtogroup LOG_GR_EVENT
**  @{
**/

// Event types
#define GR_VLINK_ASSOCIATE 1       /**< when associating Georaid to xiotech server  */
#define GR_VLINK_CPYMIRROR 2       /**< when copy-mirror vlink with GeoRaid Vdisk   */
#define GR_ASWAP_STATE_EVENT 3     /**< Autoswap state transisition event           */

typedef struct LOG_GR_EVENT_DATA
{
    UINT8   eventType;
    UINT16  svid;
    UINT16  dvid;
    UINT8   aswapState;
}LOG_GR_EVENT_DAT;

typedef struct LOG_GR_EVENT_PKT
{
    LOG_HEADER_PKT        header; /**< Log Event Header - 8 bytes         */
    LOG_GR_EVENT_DAT      data;   /**< Log Event Data                     */
}LOG_GR_EVENT_PKT;
/* @} */

/***** GEORAID  ****END*/

/***** Copy Manager events -- START*/
/**
**  @addtogroup LOG_CM_EVENT
**  @{
**/

// Event types -- this facilitates to add more type of events related
//                to copy manager in future.
#define CM_INSTANT_MIRROR_DISABLE  1   /**< Instant Mirror related event  */

typedef struct LOG_CM_EVENT_DATA
{
    UINT8   eventType;
    UINT16  svid;
    UINT8   owner;
} LOG_CM_EVENT_DAT;

typedef struct LOG_CM_EVENT_PKT
{
    LOG_HEADER_PKT        header; /**< Log Event Header - 8 bytes         */
    LOG_CM_EVENT_DAT      data;   /**< Log Event Data                     */
} LOG_CM_EVENT_PKT;

/* @} */
/***** Copy Manager events  -- END*/

/**
**  @addtogroup LOG_ICL_PORT_EVENT
**  @{
**/

/*
** ICL port events/states
*/
#define LOG_ICLPORT_INIT_OK     1
#define LOG_ICLPORT_INIT_FAILED 2
#define LOG_ICLPORT_LOOPUP      3
#define LOG_ICLPORT_LOOPDOWN    4
#define LOG_ICLPORT_CHIPRESET   5

typedef struct LOG_ICL_EVENT_DATA
{
    UINT8   portState;             /**< ICL port state                     */
    UINT8   rsvd2[3];              /**< Reserved                           */
}LOG_ICL_EVENT_DATA;

typedef struct LOG_ICL_EVENT_PKT
{
    LOG_HEADER_PKT         header; /**< Log Event Header - 8 bytes         */
    LOG_ICL_EVENT_DATA     data;   /**< Log Event Date                     */
}LOG_ICL_EVENT_PKT;
/* @} */

/**
**  @addtogroup LOG_PRES_EVENT
**  @{
**/
typedef struct LOG_PRES_EVENT_PKT
{
    LOG_HEADER_PKT         header;    /**< Log Event Header - 8 bytes       */
    UINT16                 vid;       /**< VID for completion cb            */
    UINT16                 sid;       /**< SID for completion cb            */
    UINT8                  lun;       /**< lun for completion cb            */
    UINT8                  dummy[7];  /**< dummy for future use             */
    UINT32                 dataSize;  /**< size of data to follow           */
    MRSETPRES_REQ          *data;     /**< data pointer                     */
} LOG_PRES_EVENT_PKT;
/* @} */

/**
**  @addtogroup LOG_PRES_CHANGE_PKT
**  @{
**/
typedef struct LOG_PRES_CHANGE_PKT
{
    LOG_HEADER_PKT         header; /**< Log Event Header - 8 bytes         */
} LOG_PRES_CHANGE_PKT;
/* @} */

/**
 **  @addtogroup LOG_IOCB
 **  @{
 **/
typedef struct LOG_IOCB_DATA
{
  UINT8   port;
  UINT8   feorbe;
  UINT32  reason;
  UINT8   iocb[64];
} LOG_IOCB_DATA;

typedef struct LOG_IOCB_PKT
{
  LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
  LOG_IOCB_DATA   data;
} LOG_IOCB_PKT;
/*@}*/

/**
**  @addtogroup LOG_MB_FAILED
**  @{
**/
typedef struct LOG_MB_FAILED_DAT
{
    UINT8           port;           /**< Port Number                        */
    UINT8           rsvd9[1];       /**< RESERVED                           */
    UINT8           proc;           /**< FE (0) or BE (1) indicator         */
    UINT8           rsvd11[1];      /**< RESERVED                           */
    UINT16          iregs;
    UINT16          oregs;
    UINT16          imbr[12];       /**< Input mailbox registers - see ispc.c */
    UINT16          ombr[12];       /**< Output mailbox registers - see ispc.c */
} LOG_MB_FAILED_DAT;

typedef struct LOG_MB_FAILED_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_MB_FAILED_DAT data;         /**< Log Event Data                     */
} LOG_MB_FAILED_PKT;
/* @} */

typedef struct LOG_PHYRETRY_DAT
{
    UINT8   prpstat;
    UINT8   scsistat;
    UINT8   qstatus;
    UINT8   retry;
    UINT8   port;
    UINT8   lun;
    UINT16  pid;
    UINT32  id;
    UINT64  wwn;
    UINT8   cdb[16];
    UINT8   sense[32];
} LOG_PHYRETRY_DAT;

typedef struct LOG_PHY_RETRY_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_PHYRETRY_DAT data;
} LOG_PHY_RETRY_PKT;

/**
**  @addtogroup LOG_DVLIST
**  @{
**/
typedef struct LOG_DVLIST_DAT
{
    UINT32 devcnt;                  /* Device Count                         */
    UINT16 pid[1];                  /* List of PIDs                         */
} LOG_DVLIST_DAT;

typedef struct LOG_DVLIST_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_DVLIST_DAT  data;           /**< Log Event Data                     */
} LOG_DVLIST_PKT;
/* @} */

/**
**  @addtogroup LOG_PHY_ACTION
**  @{
**/
typedef struct LOG_PHY_ACTION_DAT
{
    UINT8   port;
    UINT8   rsv0;
    UINT8   proc;
    UINT8   recovery;
    UINT16  handle;
    UINT8   lun;
    UINT8   action;
    UINT16  pid;
    UINT16  rsv16;
}LOG_PHY_ACTION_DAT;
typedef struct LOG_PHY_ACTION_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_PHY_ACTION_DAT data;
} LOG_PHY_ACTION_PKT;
/* @} */

/**
**  @addtogroup LOG_HOTSPARE_DEPLETED
**  @{
**/
#define     HSD_CNC         0       /**< CNC is depleted                    */
#define     HSD_GEO         1       /**< GEO-RAID is depleted               */
#define     HSD_TOO_SMALL   2       /**< Not big enough for biggest drive   */
#define     HSD_CNC_OK      3       /**< GEO_RAID is OK now                 */
#define     HSD_GEO_OK      4       /**< GEO_RAID is OK now                 */
#define     HSD_JUST_RIGHT  5       /**< Now big enough for biggest drive   */

/**
**  @addtogroup LOG_MIRROR_CAPABLE
**  @{
**/
typedef struct LOG_MIRROR_CAPABLE_DAT
{
    UINT32          mirrorPartnerSN;/**< Mirror Partner Serial Number       */
} LOG_MIRROR_CAPABLE_DAT;

typedef struct LOG_MIRROR_CAPABLE_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_MIRROR_CAPABLE_DAT data; /**< Log Event Data                   */
} LOG_MIRROR_CAPABLE_PKT;
/* @} */

/**
**  @addtogroup LOG_VLINK_OPEN_BEGIN
**  @{
**/
typedef struct LOG_VLINK_OPEN_BEGIN_DAT
{
    UINT16 inVID;
    UINT16 raidVID;
    UINT16 raidID;
    UINT16 reserved;
    UINT32 vlop;
} LOG_VLINK_OPEN_BEGIN_DAT;

typedef struct LOG_VLINK_OPEN_BEGIN_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_VLINK_OPEN_BEGIN_DAT data;  /**< Log Event Data                     */
} LOG_VLINK_OPEN_BEGIN_PKT;
/* @} */

/**
**  @addtogroup LOG_VLINK_OPEN_END
**  @{
**/
typedef struct LOG_VLINK_OPEN_END_DAT
{
    UINT16 vlopVID;
    UINT16 raidVID;
    UINT16 raidID;
    UINT8  vlopState;
    UINT8  reserved;
    UINT32 vlop;
} LOG_VLINK_OPEN_END_DAT;

typedef struct LOG_VLINK_OPEN_END_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_VLINK_OPEN_END_DAT data;    /**< Log Event Data                     */
} LOG_VLINK_OPEN_END_PKT;
/* @} */

/**
**  @addtogroup LOG_RAID5_INOPERATIVE
**  @{
**/
typedef struct LOG_RAID5_INOPERATIVE_DAT
{
    UINT16 vdiskID;
    UINT16 raidID;
} LOG_RAID5_INOPERATIVE_DAT;

typedef struct LOG_RAID5_INOPERATIVE_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_RAID5_INOPERATIVE_DAT data; /**< Log Event Data                     */
} LOG_RAID5_INOPERATIVE_PKT;
/* @} */

/**
**  @addtogroup LOG_RAID_INOPERATIVE
**  @{
**/
typedef struct LOG_RAID_INOPERATIVE_DAT
{
    UINT16 vdiskID;
    UINT16 raidID;
} LOG_RAID_INOPERATIVE_DAT;

typedef struct LOG_RAID_INOPERATIVE_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_RAID_INOPERATIVE_DAT data;  /**< Log Event Data                     */
} LOG_RAID_INOPERATIVE_PKT;
/* @} */

/**
**  @addtogroup LOG_NVRAM_WRITE_FAIL
**  @{
**/
typedef struct LOG_NVRAM_WRITE_FAIL_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    /*UINT8           proc;*/           /**< FE (0) or BE (1) indicator         */
} LOG_NVRAM_WRITE_FAIL_PKT;
/* @} */

/*
** Define Types of Parity Check RAID Log Message
*/
#define PARITY_CHECK_RAID_START     0 /**< Parity Check for RAID x Started  */
#define PARITY_CHECK_RAID_END       1 /**< Parity Check for RAID x Complete */
#define PARITY_CHECK_RAID_TERM      2 /**< Parity Check for RAID x Terminated */

/**
**  @addtogroup LOG_PARITY_CHECK_RAID
**  @{
**/
typedef struct LOG_PARITY_CHECK_RAID_DAT
{
    UINT16 raidID;                  /**< RAID ID associated with the log    */
    UINT16 type;                    /**< Type of message - begin, end, term */
} LOG_PARITY_CHECK_RAID_DAT;

typedef struct LOG_PARITY_CHECK_RAID_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_PARITY_CHECK_RAID_DAT data; /**< Log Event Data                     */
} LOG_PARITY_CHECK_RAID_PKT;
/* @} */

/**
**  @addtogroup LOG_ORPHAN_DETECTED
**  @{
**/
typedef struct LOG_ORPHAN_DETECTED_DAT
{
    UINT16 raidID;                  /**< RAID ID associated with the log    */
    UINT16 vdiskID;                 /**< Virtual ID associated with the log */
} LOG_ORPHAN_DETECTED_DAT;

typedef struct LOG_ORPHAN_DETECTED_PKT
{
    LOG_HEADER_PKT          header; /**< Log Event Header - 8 bytes         */
    LOG_ORPHAN_DETECTED_DAT data;   /**< Log Event Data                     */
} LOG_ORPHAN_DETECTED_PKT;
/* @} */

/**
**  @addtogroup LOG_DEFRAG_VER_DONE
**  @{
**/
typedef struct LOG_DEFRAG_VER_DONE_DAT
{
    UINT16 pdiskID;                 /**< PID of verification completed      */
    UINT16 raidID;                  /**< RID of verification completed      */
    UINT16 success;                 /**< Boolean indicator of success       */
    UINT16 failedPID;               /**< If not successful, which PID failed*/
    UINT64 sda;                     /**< Starting disk address of RID on PID*/
} LOG_DEFRAG_VER_DONE_DAT;

typedef struct LOG_DEFRAG_VER_DONE_PKT
{
    LOG_HEADER_PKT          header; /**< Log Event Header - 8 bytes         */
    LOG_DEFRAG_VER_DONE_DAT data;   /**< Log Event Data                     */
} LOG_DEFRAG_VER_DONE_PKT;
/* @} */

/**
**  @addtogroup LOG_PDISK_DEFRAG_OP_COMPLETE
**  @{
**/
/*
** Define types of defrag completions
*/
#define DEFRAG_OP_COMPLETE_OK       0 /**< Defrag OP completed OK           */
#define DEFRAG_OP_COMPLETE_ERROR    1 /**< Defrag OP failed                 */
#define DEFRAG_OP_COMPLETE_STOPPED  2 /**< Defrag OP stopped                */

typedef struct LOG_DEFRAG_OP_COMPLETE_DAT
{
    UINT8   status;                 /**< Completion status                  */
    UINT8   rsvd[1];                /**< RESERVED                           */
    UINT16  pdiskID;                /**< PID of verification completed      */
} LOG_DEFRAG_OP_COMPLETE_DAT;

typedef struct LOG_DEFRAG_OP_COMPLETE_PKT
{
    LOG_HEADER_PKT              header; /**< Log Event Header - 8 bytes     */
    LOG_DEFRAG_OP_COMPLETE_DAT  data;   /**< Log Event Data                 */
} LOG_DEFRAG_OP_COMPLETE_PKT;
/* @} */


/**
**  @addtogroup LOG_NV_MEM_EVENT
**  @{
**/
typedef struct LOG_NV_MEM_EVENT_DAT
{
    UINT32 event;                   /**< Event code                         */
} LOG_NV_MEM_EVENT_DAT;

typedef struct LOG_NV_MEM_EVENT_PKT
{
    LOG_HEADER_PKT          header; /**< Log Event Header - 8 bytes         */
    LOG_NV_MEM_EVENT_DAT    data;   /**< Log Event Data                     */
} LOG_NV_MEM_EVENT_PKT;
/* @} */

/**
**  @addtogroup LOG_SES_SBOD_EXT
**  @{
**/
typedef struct LOG_SES_WWN_SBOD_EXT_DAT
{
    UINT64          wwn;                /* World wide name          */
    UINT32          slot;               /* Slot number              */
    UINT32          newStat;            /* New status               */

} LOG_SES_WWN_SBOD_EXT_DAT, *PLOG_SES_WWN_SBOD_EXT_DAT;

typedef struct LOG_SES_SBOD_EXT_PKT
{
    LOG_HEADER_PKT           header; /**< Log Event Header - 8 bytes        */
    LOG_SES_WWN_SBOD_EXT_DAT data;   /**< Log Event Data                    */
} LOG_SES_SBOD_EXT_PKT;
/* @} */

/**
**  @addtogroup LOG_SES_WWN_SLOT
**  @{
**/
typedef struct LOG_SES_WWN_SLOT_DAT
{
    UINT64          wwn;                /* World wide name          */
    UINT32          slot;               /* Slot number              */
    UINT8           direction;          /* Change type (on or off)  */
} LOG_SES_WWN_SLOT_DAT;

typedef struct LOG_SES_WWN_SLOT_PKT
{
    LOG_HEADER_PKT          header; /**< Log Event Header - 8 bytes        */
    LOG_SES_WWN_SLOT_DAT    data;   /**< Log Event Data                    */
} LOG_SES_WWN_SLOT_PKT;
/* @} */

/**
**  @addtogroup LOG_SES_WWN_TEMP
**  @{
**/
typedef struct LOG_SES_WWN_TEMP_DAT
{
    UINT64          wwn;                /* World wide name          */
    UINT32          slot;               /* Slot number              */
    UINT32          temp;               /* temperature              */
    UINT8           direction;          /* Change type (on or off)  */

} LOG_SES_WWN_TEMP_DAT, *PLOG_SES_WWN_TEMP_DAT;

typedef struct LOG_SES_WWN_TEMP_PKT
{
    LOG_HEADER_PKT          header; /**< Log Event Header - 8 bytes        */
    LOG_SES_WWN_TEMP_DAT    data;   /**< Log Event Data                    */
} LOG_SES_WWN_TEMP_PKT;
/* @} */

/**
**  @addtogroup LOG_SES_WWN_VOLT
**  @{
**/
typedef struct LOG_SES_WWN_VOLT_DAT
{
    UINT64          wwn;                /* World wide name          */
    UINT32          slot;               /* Slot number              */
    UINT32          voltage;            /* Voltage                  */
    UINT8           direction;          /* Change type (on or off)  */

} LOG_SES_WWN_VOLT_DAT, *PLOG_SES_WWN_VOLT_DAT;

typedef struct LOG_SES_WWN_VOLT_PKT
{
    LOG_HEADER_PKT          header; /**< Log Event Header - 8 bytes        */
    LOG_SES_WWN_VOLT_DAT    data;   /**< Log Event Data                    */
} LOG_SES_WWN_VOLT_PKT;
/* @} */

/**
**  @addtogroup LOG_SES_WWN_CURR
**  @{
**/
typedef struct LOG_SES_WWN_CURR_DAT
{
    UINT64          wwn;                /* World wide name          */
    UINT32          slot;               /* Slot number              */
    UINT32          current;            /* Current                  */
    UINT8           direction;          /* Change type (on or off)  */

} LOG_SES_WWN_CURR_DAT, *PLOG_SES_WWN_CURR_DAT;

typedef struct LOG_SES_WWN_CURR_PKT
{
    LOG_HEADER_PKT          header; /**< Log Event Header - 8 bytes        */
    LOG_SES_WWN_CURR_DAT    data;   /**< Log Event Data                    */
} LOG_SES_WWN_CURR_PKT;
/* @} */

/**
**  @addtogroup LOG_SES_ELEM_CHANGE
**  @{
**/
typedef struct LOG_SES_ELEM_CHANGE_DAT
{
    UINT64          wwn;                /* World wide name          */
    UINT32          slot;               /* Slot number              */
    UINT8           elemType;           /* Element type             */
    UINT8           oldState;           /* Old state = 0xff if none */
    UINT8           newState;           /* New state                */

} LOG_SES_ELEM_CHANGE_DAT, *PLOG_SES_ELEM_CHANGE_DAT;

typedef struct LOG_SES_ELEM_CHANGE_PKT
{
    LOG_HEADER_PKT          header; /**< Log Event Header - 8 bytes        */
    LOG_SES_ELEM_CHANGE_DAT data;   /**< Log Event Data                    */
} LOG_SES_ELEM_CHANGE_PKT;
/* @} */

typedef struct LOG_ISE_IP_DISCOVER_DAT
{
    UINT16     bayid;                  /* Bay id of ISE */
    UINT16     resvd;                  /* reserved for byte boundary */
    IP_ADDRESS ip1;                    /* 1st ip address of ISE */
    IP_ADDRESS ip2;                    /* 2nd ip address of ISE */
    UINT64     wwn;                    /* wwn of ISE             */
} LOG_ISE_IP_DISCOVER_DAT;

typedef struct LOG_ISE_IP_DISCOVER_PKT
{
    LOG_HEADER_PKT            header;          /* Log event header */
    LOG_ISE_IP_DISCOVER_DAT   data;            /* ise environment data */
} LOG_ISE_IP_DISCOVER_PKT;

/**
**  @addtogroup LOG_ISE_ELEM_CHANGE
**  @{
**/
enum ISE_ENV_ELEMTYPE_BITPOS
{
   ISE_ENV_CTRLR_CHANGED,
   ISE_ENV_DPAC_CHANGED,
   ISE_ENV_PS_CHANGED,
   ISE_ENV_BATTERY_CHANGED,
   ISE_ENV_CHASSIS_CHANGED
};

enum LOG_ISE_COMPONENTS
{
    LOG_ISE_MAP,
    LOG_ISE_MRC,
    LOG_ISE_DPAC,
    LOG_ISE_PS,
    LOG_ISE_BATTERY,
    LOG_ISE_TEMP
};

typedef struct LOG_ISE_PS_ELEM_DATA
{
    UINT16        bayid;       /* bay id or ise number */
    UINT16        ps_no;       /* power supply number 1 or 2 */
    char          sn[20];      /* serial number string */
    char          pn[20];      /* part number string    */
} LOG_ISE_PS_ELEM_DATA;

typedef struct LOG_ISE_BAT_ELEM_DATA
{
    UINT16        bayid;         /* bay id or ise number */
    UINT16        bat_no;        /* battery number 1 or 2 */
    char          sn[20];        /* serial number string */
    char          pn[20];        /* part number string    */
} LOG_ISE_BAT_ELEM_DATA;

typedef struct LOG_ISE_DPAC_ELEM_DATA
{
    UINT16      bayid;           /* bay id or ise number */
    UINT16      dpac_no;         /* datapack number 1 or 2 */
    char        sn[20];          /* serial number string */
    char        pn[20];          /* part number string    */
    char        fw_vers[20];     /* firmware version */
    UINT16      health;          /* health in percentage */
} LOG_ISE_DPAC_ELEM_DATA;

typedef struct LOG_ISE_MRC_ELEM_DATA
{
    UINT16      bayid;           /* bay id or ise number */
    UINT16      mrc_no;          /* mrc controller number 1 or 2 */
    char        sn[20];          /* serial number string */
    char        pn[20];          /* part number string    */
    char        hw_vers[20];     /* hardware version */
    char        fw_vers[20];     /* firmware version */
    UINT32      ip;              /* ip addrress of mrc */
    UINT64      reason_code;     /* this is equal to detailed status value */
} LOG_ISE_MRC_ELEM_DATA;

typedef struct LOG_ISE_ELEM_MAP_CHANGE_DAT
{
    UINT16          bayid;              /* Bay Id of ISE          */
    UINT16          envmap;             /* Bit position of environments effected  */
} LOG_ISE_ELEM_MAP_CHANGE_DAT;

typedef struct LOG_ISE_ELEM_TEMP_CHANGE_DAT
{
    UINT16          bayid;              /* Bay id of ISE            */
    INT16           ise_temperature;    /* ise chassis temperature */
} LOG_ISE_ELEM_TEMP_CHANGE_DAT;

typedef struct LOG_ISE_ELEM_CHANGE_DAT
{
    UINT16                    component;              /* component enum LOG_ISE_COMPONENTS */
    union
    {
        LOG_ISE_ELEM_MAP_CHANGE_DAT     map_data;    /* internal elements map data ISE_ENV_ELEMTYPE_BITPOS */
        LOG_ISE_MRC_ELEM_DATA           mrc_data;    /* mrc data to be logged */
        LOG_ISE_DPAC_ELEM_DATA          dpac_data;   /* data pack data to be logged */
        LOG_ISE_BAT_ELEM_DATA           bat_data;    /* battery data to be logged */
        LOG_ISE_PS_ELEM_DATA            ps_data;     /* log information of power supply */
        LOG_ISE_ELEM_TEMP_CHANGE_DAT    temp_data;   /* temperature data of ISE */
    };
} LOG_ISE_ELEM_CHANGE_DAT;

typedef struct LOG_ISE_ELEM_CHANGE_PKT
{
    LOG_HEADER_PKT          header; /**< Log Event Header - 8 bytes        */
    LOG_ISE_ELEM_CHANGE_DAT data;   /**< Log Event Data                    */
} LOG_ISE_ELEM_CHANGE_PKT;
/* @} */

/**
**  @addtogroup LOG_WAIT_CACHE_ERROR
**  @{
**/
typedef struct LOG_WAIT_CACHE_ERROR_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
} LOG_WAIT_CACHE_ERROR_PKT;
/* @} */

/**
**  @addtogroup LOG_WC_FLUSH
**  @{
**/
typedef struct LOG_WC_FLUSH_DAT
{
    UINT32 time;                    /**< Time from beginning of Flush       */
    UINT32 tagDirty;                /**< Tags Dirty Count                   */
    UINT32 tagResident;             /**< Tags Resident Count                */
    UINT32 tagFlushInProgress;      /**< Tags Flush in Progress Count       */
    UINT32 blksDirty;               /**< Blocks Dirty Count                 */
    UINT32 blksResident;            /**< Blocks Resident Count              */
    UINT32 blksFlushInProgress;     /**< Blocks Flush in Progress Count     */
    UINT32 flushInvalidates;        /**< Flush and Invalidate Count         */
    UINT32 cacheORC;                /**< Cache Outstanding Request Count    */
    UINT32 cacheStatus;             /**< Cache Status, Status2, Stop Count  */
    UINT32 outstandingVRPs;         /**< Outstanding VRP Count to BE        */
    UINT32 procUtilization;         /**< Processor Utilization              */
} LOG_WC_FLUSH_DAT;

typedef struct LOG_WC_FLUSH_PKT
{
    LOG_HEADER_PKT          header; /**< Log Event Header - 8 bytes         */
    LOG_WC_FLUSH_DAT        data;   /**< Log Event Data                     */
} LOG_WC_FLUSH_PKT;
/* @} */


/**
**  @addtogroup LOG_WC_SN_VCG_BAD
**  @{
**/
typedef struct LOG_WC_SN_VCG_BAD_DAT
{
    UINT8       rsvd;               /**< RESERVED                           */
} LOG_WC_SN_VCG_BAD_DAT;

typedef struct LOG_WC_SN_VCG_BAD_PKT
{
    LOG_HEADER_PKT          header; /**< Log Event Header - 8 bytes         */
    LOG_WC_SN_VCG_BAD_DAT   data;   /**< Log Event Data                     */
} LOG_WC_SN_VCG_BAD_PKT;
/* @} */

/**
**  @addtogroup LOG_WC_SN_BAD
**  @{
**/
typedef struct LOG_WC_SN_BAD_DAT
{
    UINT8       rsvd;               /**< RESERVED                           */
} LOG_WC_SN_BAD_DAT;

typedef struct LOG_WC_SN_BAD_PKT
{
    LOG_HEADER_PKT          header; /**< Log Event Header - 8 bytes         */
    LOG_WC_SN_BAD_DAT       data;   /**< Log Event Data                     */
} LOG_WC_SN_BAD_PKT;
/* @} */

/**
**  @addtogroup LOG_WC_NVMEM_BAD
**  @{
**/
typedef struct LOG_WC_NVMEM_BAD_DAT
{
    UINT8       rsvd;               /**< RESERVED                           */
} LOG_WC_NVMEM_BAD_DAT;

typedef struct LOG_WC_NVMEM_BAD_PKT
{
    LOG_HEADER_PKT          header; /**< Log Event Header - 8 bytes         */
    LOG_WC_NVMEM_BAD_DAT    data;   /**< Log Event Data                     */
} LOG_WC_NVMEM_BAD_PKT;
/* @} */

#if ISCSI_CODE
/**
**  @addtogroup LOG_SERVER_LOGGED_IN_OP_PKT
**  @{
**/
typedef struct LOG_SERVER_LOGGED_IN_OP_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    UINT16 tid;
    UINT16 loginState;
    UINT64 wwn;
    UINT8  i_name[256];
} LOG_SERVER_LOGGED_IN_OP_PKT;
/* @} */

/**
**  @addtogroup LOG_TARGET_UP_OP_PKT
**  @{
**/
typedef struct LOG_TARGET_UP_OP_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    UINT16 tid;
    UINT8  type;
    UINT8  state;
    union
    {
      struct {
          UINT32 ip;
          UINT32 mask;
          UINT32 gateway;
      }
#ifdef S_SPLINT_S
        aha1
#endif  /* S_SPLINT_S */
      ;
      struct {
          UINT64 wwn;
          UINT32 rsvd;
      }
#ifdef S_SPLINT_S
        aha2
#endif  /* S_SPLINT_S */
      ;
    }
#ifdef S_SPLINT_S
      aha3
#endif  /* S_SPLINT_S */
     ;
} LOG_TARGET_UP_OP_PKT;
/* @} */

/**
**  @addtogroup LOG_ISCSI_GENERIC_PKT
**  @{
**/
typedef struct LOG_ISCSI_GENERIC_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    UINT8           msgStr[4096];
} LOG_ISCSI_GENERIC_PKT;
/* @} */

#endif

/**
 ** APOOL log message, which triggers an async event to icon.
 **/
typedef struct LOG_APOOL_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    UINT8     msgStr[1024];         /**< Message, see LOG_GENERIC_OP_PKT    */
} LOG_APOOL_PKT;

typedef struct LOG_APOOLEVENT_DAT
{
    UINT8    sub_event_code;        /**< extended event codes (see AP_EVENT_CODES list) */
    UINT8    severity;              /**< severity: 0-info, 1-warning, 2=error, 4=debug */
    UINT8    notify_on_message;     /**< future use to restrict events if too many for ewok */
    UINT8    rsvd;
    INT32    errorCode;             /**< extended error code if needed */
    INT32    value1;                /**< misc value depending on sub event code */
    INT32    value2;                /**< misc value depending on sub event code */
} LOG_APOOLEVENT_DAT;

typedef struct LOG_APOOLEVENT_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_APOOLEVENT_DAT data;
} LOG_APOOLEVENT_PKT;

typedef struct LOG_SPOOLEVENT_DAT
{
    UINT8    sub_event_code;        /**< extended event codes (see SS_EVENT_CODES list) */
    UINT8    severity;              /**< severity: 0-info, 1-warning, 2=error, 4=debug */
    UINT8    skip_notify;           /**< future use to restrict events if too many for ewok */
    UINT8    rsvd;
    INT32    errorCode;             /**< extended error code if needed */
    INT32    value1;                /**< misc value depending on sub event code */
    INT32    value2;                /**< misc value depending on sub event code */
} LOG_SPOOLEVENT_DAT;

typedef struct LOG_SPOOLEVENT_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    LOG_SPOOLEVENT_DAT data;
} LOG_SPOOLEVENT_PKT;


/**
**  @addtogroup LOG_GENERIC_PKT
**  @{
**/
typedef struct LOG_GENERIC_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    UINT8           msgStr[1024];
} LOG_GENERIC_PKT;
/* @} */

/**
**  @addtogroup LOG_AUTO_FAILBACK_INFO_PKT
**  @{
**/
typedef struct LOG_AUTO_FAILBACK_INFO_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    UINT8           status;
    UINT64          hswwn;
    UINT64          dstwwn;
} LOG_AUTO_FAILBACK_INFO_PKT;
/* @} */

/**
**  @addtogroup LOG_AF_ENABLE_DISABLE
**  @{
**/
typedef struct LOG_AF_ENABLE_DISABLE_INFO_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    UINT8           status;
} LOG_AF_ENABLE_DISABLE_INFO_PKT;
/* @} */

/**
**  @addtogroup LOG_GLOBAL_CACHE_MODE
**  @{
**/
typedef struct LOG_GLOBAL_CACHE_MODE_INFO_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    UINT8           status;
} LOG_GLOBAL_CACHE_MODE_INFO_PKT;
/* @} */

/**
**  @addtogroup LOG_REINIT_DRIVE_INFO_PKT
**  @{
**/
typedef struct LOG_REINIT_DRIVE_INFO_PKT
{
    LOG_HEADER_PKT  header;         /**< Log Event Header - 8 bytes         */
    UINT64          dstwwn;
} LOG_REINIT_DRIVE_INFO_PKT;
/* @} */

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _LOG_DEFS_H_ */
/* @} */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
