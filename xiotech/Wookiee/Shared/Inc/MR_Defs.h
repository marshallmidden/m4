/* $Id: MR_Defs.h 161368 2013-07-29 14:53:10Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       MR_Defs.h
**
**  @brief      Maintenance Request Packet constants and data structures.
**
**  Maintenance request packet command codes, input packet and output packet
**  definitions used between the CCB and PROC.
**
**  Copyright (c) 2003-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _MR_DEFS_H_
#define _MR_DEFS_H_

#include "CA_CI.h"
#include "cor.h"
#include "XIO_Const.h"
#include "DEF_iSCSI.h"
#include "DEF_SOS.h"
#include "DEF_Workset.h"
#include "FW_Header.h"
#include "globalOptions.h"
#include "ISP_Defs.h"
#include "MP_Proc.h"
#include "OS_II.h"
#include "pdd.h"
#include "RL_RDD.h"
#include "SES_Structs.h"
#include "vdd.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/* ------------------------------------------------------------------------ */
/* The following are orphaned because someone didn't know doxygen. */
/** @defgroup MRCLEARGEOLOCATION */
/** @defgroup MRCONFIGUREOPTIONS */
/** @defgroup MRENABLEISNS */
/** @defgroup MRGETDVLIST */
/** @defgroup MRGETIDDINFO */
/** @defgroup MRGETMLIST */
/** @defgroup MRGETPORTLIST */
/** @defgroup MRGETPORTTYPE */
/** @defgroup MRHBEAT */
/** @defgroup MRINVWC */
/** @defgroup MRMMINFO */
/** @defgroup MRPORT */
/** @defgroup MRPRREQ */
/** @defgroup MRP_NAME_LENGTH_CONSTANTS */
/** @defgroup MRRESETPORT */
/** @defgroup MRSETGEOLOCATION */
/** @defgroup MRUPDTGINFO */
/** @defgroup MRVPRI_ENABLE */
/** @defgroup MR_BEFE_GENERIC */
/** @defgroup MR_CODE_BURN */
/** @defgroup MR_DEVID_REQ */
/** @defgroup MR_FSYS_OP_RSP */
/** @defgroup MR_FW_HEADER_RSP */
/** @defgroup MR_GENERIC_RSP */
/** @defgroup MR_GET_PE_INFO_RSP */
/** @defgroup MR_HDR_RSP */
/** @defgroup MR_LINK_RSP */
/** @defgroup MR_LIST_RSP */
/** @defgroup MR_LOOP_PRIMITIVE */
/** @defgroup MR_MODE_SET */
/** @defgroup MR_PKT */
/** @defgroup MR_RESET */
/** @defgroup MR_RW_MEMORY */

/* ------------------------------------------------------------------------ */

/**
**  @defgroup MRP_CCB_TO_BE     CCB to BE MRP
**  @{
**/
/** @defgroup MRCREXP           MRCREXP - Create or expand a virtual device             */
/** @defgroup MRGETELIST        MRGETELIST - Get SES device list                        */
/** @defgroup MRLABEL           MRLABEL - Label a physical device                       */
/** @defgroup MRFAIL            MRFAIL - Fail a device                                  */
/** @defgroup MRSCSIIO          MRSCSIIO - Raw SCSI IO                                  */
/** @defgroup MRREADWRITEIO     MRREADWRITEIO - READ/WRITE a VDISK or PDISK             */
/** @defgroup MRINITRAID        MRINITRAID - Initialize a RAID device                   */
/** @defgroup MROBSOLETE106     MROBSOLETE106 - OBSOLETE                                */
/** @defgroup MRDELVIRT         MRDELVIRT - Delete a virtual device                     */
/** @defgroup MRSETCACHE        MRSETCACHE - Set caching mode                           */
/** @defgroup MRSERVERPROP      MRSERVERPROP - Set server properties                    */
/** @defgroup MRRESET           MRRESET - Reset NVRAM, clear devices                    */
/** @defgroup MRRESTORE         MRRESTORE - Restore from NVRAM                          */
/** @defgroup MRAWAKE           MRAWAKE - Awake (ping)                                  */
/** @defgroup MRWWNLOOKUP       MRWWNLOOKUP - WWN/LUN lookup                            */
/** @defgroup MRBEGENERIC       MRBEGENERIC - Generic                                   */
/** @defgroup MRSTARTSTOP       MRSTARTSTOP - Start or stop a device                    */
/** @defgroup MRSCRUBCTRL       MRSCRUBCTRL - Enable/disable scrubbing                  */
/** @defgroup MRDEFAULT         MRDEFAULT - Set default label behavior                  */
/** @defgroup MRGETBEDEVPATHS   MRGETBEDEVPATHS - Get BE Device Paths                   */
/** @defgroup MRRESTOREDEV      MRRESTOREDEV - Restore device                           */
/** @defgroup MRDEFRAGMENT      MRDEFRAGMENT - Defragment device                        */
/** @defgroup MRSETATTR         MRSETATTR - Set attribute                               */
/** @defgroup MRBELOOP          MRBELOOP - Get back end loop information                */
/** @defgroup MRGETSLIST        MRGETSLIST - Get server list                            */
/** @defgroup MRGETVLIST        MRGETVLIST - Get virtual device list                    */
/** @defgroup MRGETRLIST        MRGETRLIST - Get RAID list                              */
/** @defgroup MRGETPLIST        MRGETPLIST - Get physical device list                   */
/** @defgroup MRGETVINFO        MRGETVINFO - Get virtual device information             */
/** @defgroup MRGETRINFO        MRGETRINFO - Get RAID information                       */
/** @defgroup MRGETPINFO        MRGETPINFO - Get physical device information            */
/** @defgroup MRMAPLUN          MRMAPLUN - Map a LUN to a VDisk                         */
/** @defgroup MRUNMAPLUN        MRUNMAPLUN - Unmap a LUN from a VDisk                   */
/** @defgroup MRGETEINFO        MRGETEINFO - Get SES device information                 */
/** @defgroup MRCREATESERVER    MRCREATESERVER - Create a server                        */
/** @defgroup MRDELETESERVER    MRDELETESERVER - Delete a server                        */
/** @defgroup MRGETMINFO        MRGETMINFO - Get misc device info                       */
/** @defgroup MRVDISKCONTROL    MRVDISKCONTROL - Virtual disk control                   */
/** @defgroup MRASSIGNSYSINFO   MRASSIGNSYSINFO - Assign system information             */
/** @defgroup MRBEII            MRBEII - Get back end II information                    */
/** @defgroup MRBELINK          MRBELINK - Get back end link information                */
/** @defgroup MRBEBOOT          MRBEBOOT - Get back end boot code header                */
/** @defgroup MRBEDIAG          MRBEDIAG - Get back end diag code header                */
/** @defgroup MRBEPROC          MRBEPROC - Get back end proc code header                */
/** @defgroup MRBECODEBURN      MRBECODEBURN - Burn back end flash code                 */
/** @defgroup MRBRWMEM          MRBRWMEM - R/W back end memory                          */
/** @defgroup MRCONFIGTARG      MRCONFIGTARG - Configure target                         */
/** @defgroup MRGETMPLIST       MRGETMPLIST - Get Mirror partner list                   */
/** @defgroup MRGLOBALPRI       MRGLOBALPRI - Set background priority                   */
/** @defgroup MRGETTLIST        MRGETTLIST - Get target list                            */
/** @defgroup MRRESETBEPORT     MRRESETBEPORT - Reset BE Port                           */
/** @defgroup MRNAMECHANGE      MRNAMECHANGE - VDisk or ctrl name changed               */
/** @defgroup MRREMOTECTRLCNT   MRREMOTECTRLCNT - Get remote ctrl count                 */
/** @defgroup MRREMOTECTRLINFO  MRREMOTECTRLINFO - Get remote ctrl info                 */
/** @defgroup MRREMOTEVDISKINFO MRREMOTEVDISKINFO - Get remote vdisk info               */
/** @defgroup MRFOREIGNTARGETS  MRFOREIGNTARGETS - Set foreign targets                  */
/** @defgroup MRCREATEVLINK     MRCREATEVLINK - Create a vlink                          */
/** @defgroup MRVLINKINFO       MRVLINKINFO - Get virtual link disk info                */
/** @defgroup MRCREATECTRLR     MRCREATECTRLR - Crt dflt servers/targets for a ctrlr    */
/** @defgroup MRRESCANDEVICE    MRRESCANDEVICE - Rescan physical Devices                */
/** @defgroup MRRESYNC          MRRESYNC - Resync RAIDs or stripes                      */
/** @defgroup MRGETLCLIMAGE     MRGETLCLIMAGE - Get local NVRAM image                   */
/** @defgroup MRPUTLCLIMAGE     MRPUTLCLIMAGE - Put local NVRAM image                   */
/** @defgroup MRDELETEDEVICE    MRDELETEDEVICE - Delete SES or disk drive device        */
/** @defgroup MRBEMODEPAGE      MRBEMODEPAGE - Mode page                                */
/** @defgroup MRDEVICECOUNT     MRDEVICECOUNT - Device count for a specific serial num  */
/** @defgroup MRGETVIDOWNER     MRGETVIDOWNER - Get Virtual Disk Owner                  */
/** @defgroup MRHOTSPAREINFO    MRHOTSPAREINFO - Get hot spare info for pid             */
/** @defgroup MRFILECOPY        MRFILECOPY - File system file to file copy              */
/** @defgroup MRBEGETDVLIST     MRBEGETDVLIST - Get back end device info                */
/** @defgroup MRBEGETPORTLIST   MRBEGETPORTLIST - Get BE Port List                      */
/** @defgroup MRBREAKVLOCK      MRBREAKVLOCK - Break vlink lock                         */
/** @defgroup MRGETSOS          MRGETSOS - Get a SOS entry from the BEP                 */
/** @defgroup MRPUTSOS          MRPUTSOS - Put a SOS entry to the BEP                   */
/** @defgroup MRFORCEBEETRAP    MRFORCEBEETRAP - Force a BE error trap                  */
/** @defgroup MRPUTSCMT         MRPUTSCMT - Update a SCMT structure                     */
/** @defgroup MRBELOOPPRIMITIVE MRBELOOPPRIMITIVE - Loop primitive                      */
/** @defgroup MRTARGETCONTROL   MRTARGETCONTROL - Target move control                   */
/** @defgroup MRFAILCTRL        MRFAILCTRL - Fail / Unfail controller                   */
/** @defgroup MRNAMEDEVICE      MRNAMEDEVICE - Name a device                            */
/** @defgroup MRPUTDG           MRPUTDG - Update VLINK information                      */
/** @defgroup MRNOPBE           MRNOPBE - BE no-op                                      */
/** @defgroup MRPUTFSYS         MRPUTFSYS - Put a file system report                    */
/** @defgroup MRGETDLINK        MRGETDLINK - Get DLink information                      */
/** @defgroup MRGETDLOCK        MRGETDLOCK - Get DLock information                      */
/** @defgroup MRDEGRADEPORT     MRDEGRADEPORT - Degrade / restore port                  */
/** @defgroup MRGETWSINFO       MRGETWSINFO - Get Workset Info                          */
/** @defgroup MRSETWSINFO       MRSETWSINFO - Set Workset Info                          */
/** @defgroup MRCHGRAIDNOTMIRRORING MRCHGRAIDNOTMIRRORING - Modify RAID AStatus Field   */
/** @defgroup MRPUTLDD          MRPUTLDD - Put the LDD                                  */
/** @defgroup MRRAIDRECOVER     MRRAIDRECOVER - Recover an inoperable raid              */
/** @defgroup MRNOPFSYS         MRNOPFSYS - Internal file system no op                  */
/** @defgroup MRFSYSOP          MRFSYSOP - Internal file system operation               */
/** @defgroup MRBEHBEAT         MRBEHBEAT - BE Heartbeat                                */
/** @defgroup MRPUTDEVCONFIG    MRPUTDEVCONFIG - Send drive info to BE                  */
/** @defgroup MRRESYNCDATA      MRRESYNCDATA - Get copy/resync data                     */
/** @defgroup MRRESYNCCTL       MRRESYNCCTL - Copy/resync control                       */
/** @defgroup MRSETVPRI         MRSETVPRI - Set VDisk Priority                          */
/** @defgroup MRPDISKSPINDOWN   MRPDISKSPINDOWN - Spin down physical disk               */
/** @defgroup MRSETGLINFO       MRSETGLINFO - Set the Geo location for bay & Disks */
/** @defgroup MRCLEARGLINFO     MRCLEARGLINFO - Clear  the Geo location for bay & Disks */
/** @defgroup MRCFGOPTION       MRCFGOPTION - Configure Options                         */
/** @defgroup MRSETTGINFO       MRSETTGINFO - Configure ISCSI Target Parameter          */
/** @defgroup MRGETTGINFO       MRGETTGINFO - Get ISCSI Target Parameter Info           */
/** @defgroup MRUPDSID          MRUPDSID    - Update Server SID                         */
/** @defgroup MRSETCHAP         MRSETCHAP   - Configure CHAP User Info                  */
/** @defgroup MRGETCHAP         MRGETCHAP   - Get CHAP User Info                        */
/** @defgroup MRVIRTREDUNDANCY  MRVIRTREDUNDANCY - VDisk Redundancy                     */
/** @defgroup MRASYNCREPLICATION MRASYNCREPLICATION - Information for async replication */
/** @defgroup MRGETDLINK_GT2TB  MRGETDLINK_GT2TB - Get DLink information for vlink>2TB  */
/* @} */

/**
**  @defgroup MRP_BE_TO_FE      BE to FE MRP
**  @{
**/
/** @defgroup MRREPORTSCONFIG   MRREPORTSCONFIG - Report server config                  */
/** @defgroup MRSCONFIGCOMPLETE MRSCONFIGCOMPLETE - Server config complete              */
/** @defgroup MRREPORTCCONFIG   MRREPORTCCONFIG - Report caching config                 */
/** @defgroup MRCCONFIGCOMPLETE MRCCONFIGCOMPLETE - Caching config complete             */
/** @defgroup MRSPARE204        MRSPARE204 - Spare                                      */
/** @defgroup MRSTOPCACHE       MRSTOPCACHE - Stop caching                              */
/** @defgroup MRCONTINUECACHE   MRCONTINUECACHE - Continue caching                      */
/** @defgroup MRSETSYSINFO      MRSETSYSINFO - Set sys serial number, etc.              */
/** @defgroup MRVCHANGE         MRVCHANGE - Virtual disk config has changed             */
/** @defgroup MRSCHANGE         MRSCHANGE - Server config has changed                   */
/** @defgroup MRREPORTTARG      MRREPORTTARG - Report target config                     */
/** @defgroup MRRESETCONFIG     MRRESETCONFIG - Reset config, NVRAM, etc                */
/* @} */

/**
**  @defgroup MRP_LOG_ENTRY     Log Entry MRP
**  @{
**/
/** @defgroup MRLOGFE           MRLOGFE - Create log entry (FE to CCB)                  */
/** @defgroup MRLOGBE           MRLOGBE - Create log entry (BE to CCB)                  */
/* @} */

/**
**  @defgroup MRP_FE_TO_BE      FE to BE MRP
**  @{
**/
/** @defgroup MRFEGETVINFO      MRFEGETVINFO - Get VDisk info to FEP                    */
/** @defgroup MRFESETSEQ        MRFESETSEQ - Set Sequence Number                        */
/** @defgroup MRFESETMP         MRFESETMP - Set Mirror Partner                          */
/* @} */

/**
**  @defgroup MRP_CCB_TO_FE     CCB to FE MRP
**  @{
**/
/** @defgroup MRFELOOP          MRFELOOP - Get front end loop information               */
/** @defgroup MRGETSINFO        MRGETSINFO - Get server information                     */
/** @defgroup MRGETCINFO        MRGETCINFO - Get cache information                      */
/** @defgroup MRFELINK          MRFELINK - Get front end link information               */
/** @defgroup MRFEII            MRFEII - Get front end II information                   */
/** @defgroup MRGETCDINFO       MRGETCDINFO - Get cache device information              */
/** @defgroup MRGETSSTATS       MRGETSSTATS - Get server statistics                     */
/** @defgroup MRSETBATHEALTH    MRSETBATHEALTH - Set battery health                     */
/** @defgroup MRRESUMECACHE     MRRESUMECACHE - Resume Cache Initialization             */
/** @defgroup MRFEBOOT          MRFEBOOT - Get front end boot code header               */
/** @defgroup MRFEDIAG          MRFEDIAG - Get front end diag code header               */
/** @defgroup MRFEPROC          MRFEPROC - Get front end proc code header               */
/** @defgroup MRFECODEBURN      MRFECODEBURN - Burn front end flash code                */
/** @defgroup MRFRWMEM          MRFRWMEM - R/W front end memory                         */
/** @defgroup MRRESETFEPORT     MRRESETFEPORT - Reset FE Port                           */
/** @defgroup MRSERVERLOOKUP    MRSERVERLOOKUP - Server lookup MRP                      */
/** @defgroup MRFEGENERIC       MRFEGENERIC - Generic                                   */
/** @defgroup MRFEASSIGNMP      MRFEASSIGNMP - Assign a Mirror Partner (0x511 non-linux)*/
/** @defgroup MRSETMPCONFIGFE   MRSETMPCONFIGFE - Set the MP configuration (0x511 linux)*/
/** @defgroup MRFEFIBREHLIST    MRFEFIBREHLIST - FE Fibre Heartbeat List for DLM        */
/** @defgroup MRFECONTWOMP      MRFECONTWOMP - Continue Cache Init w/o Mirror Partner   */
/** @defgroup MRFEFLUSHWOMP     MRFEFLUSHWOMP - Flush FE Cache w/o Mirror Partner       */
/** @defgroup MRINVFEWC         MRINVFEWC - Invalidate the FE Write Cache               */
/** @defgroup MRFLUSHBEWC       MRFLUSHBEWC - Flush the BE Write Cache                  */
/** @defgroup MRINVBEWC         MRINVBEWC - Invalidate the BE Write Cache               */
/** @defgroup MRFEMODEPAGE      MRFEMODEPAGE - Mode page                                */
/** @defgroup MRFEGETDVLIST     MRFEGETDVLIST - Get front end device info               */
/** @defgroup MRFEGETPORTLIST   MRFEGETPORTLIST - Get FE port list                      */
/** @defgroup MRGETTRLIST       MRGETTRLIST - Get target resource list                  */
/** @defgroup MRSTOPIO          MRSTOPIO - Stop I/O                                     */
/** @defgroup MRSTARTIO         MRSTARTIO - Start I/O                                   */
/** @defgroup MRFEPORTNOTIFY    MRFEPORTNOTIFY - Set FE port event notification         */
/** @defgroup MRFORCEFEETRAP    MRFORCEFEETRAP - Force a FE error trap                  */
/** @defgroup MRFELOOPPRIMITIVE MRFELOOPPRIMITIVE - Loop primitive                      */
/** @defgroup MRGETTARG         MRGETTARG - Get target information                      */
/** @defgroup MRFAILPORT        MRFAILPORT - Fail / Unfail port                         */
/** @defgroup MRNOPFE           MRNOPFE - FE no-op (0x523)                              */
/** @defgroup MRQFECC           MRQFECC - Query FE Controller Communications (0x524)    */
/** @defgroup MRQSC             MRQSC - Query Stop Complete (0x525)                     */
/** @defgroup MRQMPC            MRQMPC - Query if a MP Change will Succeed (0x526)      */
/** @defgroup MRGETHABSTATS     MRGETHABSTATS - Get HAB statistics (0x527)              */
/** @defgroup MRMMCARDGETBATTERYSTATUS  MRMMCARDGETBATTERYSTATUS - Get MM status (0x528)*/
/** @defgroup MRGETMPCONFIGFE   MRGETMPCONFIGFE - Get MP configuration (0x529)          */
/** @defgroup MRFEPORTGO        MRFEPORTGO - Put Regular Ports on FE Fabric (0x052A)    */
/** @defgroup MRSETTDISCACHE    MRSETTDISCACHE - Set Temp Disable Cache (0x052B)        */
/** @defgroup MRCLRTDISCACHE    MRCLRTDISCACHE - Set Temp Disable Cache (0x052C)        */
/** @defgroup MRQTDISABLEDONE   MRQTDISABLEDONE - Query Temp Disable Done (0x052D)      */
/** @defgroup MRMMTEST          MRMMTEST - Test driver for NV Card (0x52E)              */
/** @defgroup MRFEFLERRORWOMP   MRFEFLERRORWOMP - Flush FE Cache -> Error to MP (0x52F) */
/** @defgroup MRGETSESSIONS     MRGETSESSIONS - Get ISCSI session info (0x0530)         */
/** @defgroup MRDLMPATHSTATS    MRDLMPATHSTATS - DLM Path Stats(0x533)                  */
/** @defgroup MRDLMPATHSELECTIONALGO  MRDLMPATHSELECTIONALGO - Set the Load Balance Algm (0x534)    */
/** @defgroup MRPRCONFIGCOMPLETE MRPRCONFIGCOMPLETE - Informs FE that CCB completed the config change (0x537)    */
/** @defgroup MRFEHBEAT         MRFEHBEAT - FE heartbeat (0x5FF)                        */
/* @} */

/**
**  @defgroup MRP_CCB_TO_DLM    CCB to DLM MRP
**  @{
**/
/** @defgroup MRCCBTODLM        MRCCBTODLM - CCB to DLM message                         */
/* @} */

/**
**  @defgroup MRP_RETURN_CODES  MRP return codes
**  @{
**/
/* @} */

/**
**  @defgroup MR_APOOL_STATUS   APOOL status values in fidread 355.
**  @{
**/
/* @} */

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/**
** Use the MSB of the command code to indicate that the logging of a
** command error for this MRP should be supressed.
*/
#define MRP_SUPRESS_ERROR_LOG_FLAG      0x8000

/**
**  Codes reserved for other communications methods (VRPs)
**
**  0x0000 - 0x00FF
**/

/**
**  CCB to BE MRP command codes
**  @addtogroup MRP_CCB_TO_BE
**  @{
**/
#define MREBFUNCBASE                    0x0100
#define MRCREXP                         0x0100
#define MRGETELIST                      0x0101
#define MRLABEL                         0x0102
#define MRFAIL                          0x0103
#define MRSCSIIO                        0x0104
#define MRINITRAID                      0x0105
#define MROBSOLETE106                   0x0106
#define MRDELVIRT                       0x0107
#define MRSETCACHE                      0x0108
#define MRSERVERPROP                    0x0109
#define MRRESET                         0x010A
#define MRRESTORE                       0x010B
#define MRAWAKE                         0x010C
#define MRWWNLOOKUP                     0x010D
#define MRBEGENERIC                     0x010E
#define MRSTARTSTOP                     0x010F
#define MRSCRUBCTRL                     0x0110
#define MRDEFAULT                       0x0111
#define MRGETBEDEVPATHS                 0x0112
#define MRRESTOREDEV                    0x0113
#define MRDEFRAGMENT                    0x0114
#define MRSETATTR                       0x0115
#define MRBELOOP                        0x0116
#define MRGETSLIST                      0x0117
#define MRGETVLIST                      0x0118
#define MRGETRLIST                      0x0119
#define MRGETPLIST                      0x011A
#define MRGETMLIST                      0x011B
#define MRGETVINFO                      0x011C
#define MRGETRINFO                      0x011D
#define MRGETPINFO                      0x011E
#define MRMAPLUN                        0x011F
#define MRUNMAPLUN                      0x0120
#define MRGETEINFO                      0x0121
#define MRCREATESERVER                  0x0122
#define MRDELETESERVER                  0x0123
#define MRGETMINFO                      0x0124
#define MRVDISKCONTROL                  0x0125
#define MRASSIGNSYSINFO                 0x0126
#define MRBEII                          0x0127
#define MRBELINK                        0x0128
//#define MRBEBOOT                        0x0129    /* Obsolete */
//#define MRBEDIAG                        0x012A    /* Obsolete */
#define MRBEPROC                        0x012B
#define MRBECODEBURN                    0x012C
#define MRBRWMEM                        0x012D
#define MRCONFIGTARG                    0x012E
#define MRGETMPLIST                     0x012F
#define MRGLOBALPRI                     0x0130
#define MRGETTLIST                      0x0131
#define MRRESETBEPORT                   0x0132
#define MRNAMECHANGE                    0x0133
#define MRREMOTECTRLCNT                 0x0134
#define MRREMOTECTRLINFO                0x0135
#define MRREMOTEVDISKINFO               0x0136
#define MRFOREIGNTARGETS                0x0137
#define MRCREATEVLINK                   0x0138
#define MRVLINKINFO                     0x0139
#define MRCREATECTRLR                   0x013A
#define MRRESCANDEVICE                  0x013B
#define MRRESYNC                        0x013C
#define MRGETLCLIMAGE                   0x013D
#define MRPUTLCLIMAGE                   0x013E
#define MRDELETEDEVICE                  0x013F
#define MRBEMODEPAGE                    0x0140
#define MRDEVICECOUNT                   0x0141
#define MRGETVIDOWNER                   0x0142
#define MRHOTSPAREINFO                  0x0143
#define MRFILECOPY                      0x0144
#define MRBEGETDVLIST                   0x0145
#define MRBEGETPORTLIST                 0x0146
#define MRBREAKVLOCK                    0x0147
#define MRGETSOS                        0x0148
#define MRPUTSOS                        0x0149
#define MRFORCEBEETRAP                  0x014A
#define MRPUTSCMT                       0x014B
#define MRBELOOPPRIMITIVE               0x014C
#define MRTARGETCONTROL                 0x014D
#define MRFAILCTRL                      0x014E
#define MRNAMEDEVICE                    0x014F
#define MRPUTDG                         0x0150
#define MRNOPBE                         0x0151
#define MRPUTFSYS                       0x0152
#define MRGETDLINK                      0x0153
#define MRGETDLOCK                      0x0154
#define MRDEGRADEPORT                   0x0155
#define MRGETWSINFO                     0x0156
#define MRSETWSINFO                     0x0157
#define MRSETBEPORTCONFIG               0x0158
#define MRSAVEASYNCNV                   0x0159
#define MRCHGRAIDNOTMIRRORING           0x015A
#define MRPUTLDD                        0x015B
#define MRRAIDRECOVER                   0x015C
#define MRPUTDEVCONFIG                  0x015D
#define MRRESYNCDATA                    0x015E
#define MRRESYNCCTL                     0x015F
#define MRREFRESH                       0x0160
#define MRSETVPRI                       0x0161
#define MRVPRI_ENABLE                   0x0162
#define MRPDISKSPINDOWN                 0x0163
#define MRPDISKFAILBACK                 0x0164
#define MRPDISKAUTOFAILBACKENABLEDISABLE 0x0165

#define MRCFGOPTION                     0x0166
#define MRSETTGINFO                     0x0167
#define MRGETTGINFO                     0x0168
#define MRUPDSID                        0x0169
#define MRSETCHAP                       0x016A
#define MRGETCHAP                       0x016B

#define MRGETGLINFO                     0x016C
#define MRSETGLINFO                     0x016D
#define MRCLEARGLINFO                   0x016E
#define MRGETISNSINFO                   0x016F
#define MRSETISNSINFO                   0x0170
#define MRSETPR                         0x0171
#define MRVIRTREDUNDANCY                0x0172
#define MRGETASYNC                      0x0173
#define MRGETISEIP                      0x0174
#define MRALLDEVMISS                    0x0175
#define MRGETEXTENDVINFO                0x0176
#define MRPDISKQLTIMEOUT                0x0177
#define MRGETDLINK_GT2TB                0x0178
#define MRREADWRITEIO                   0x0179
#define MRSWAPPIDS                      0x017A
#define MREMULATEPAB                    0x017B
#define MREBFUNCMAX                     0x017B      /* Max of above. */

#define MRNOPFSYS                       0x01FD
#define MRFSYSOP                        0x01FE
#define MRBEHBEAT                       0x01FF
/* @} */

/**
**  BE to FE MRP command codes
**  @addtogroup MRP_BE_TO_FE
**  @{
**/
#define MRBFFUNCBASE                    0x0200
#define MRREPORTSCONFIG                 0x0200
#define MRSCONFIGCOMPLETE               0x0201
#define MRREPORTCCONFIG                 0x0202
#define MRCCONFIGCOMPLETE               0x0203
#define MRSPARE204                      0x0204
#define MRSTOPCACHE                     0x0205
#define MRCONTINUECACHE                 0x0206
#define MRSETSYSINFO                    0x0207
#define MRVCHANGE                       0x0208
#define MRSCHANGE                       0x0209
#define MRREPORTTARG                    0x020A
#define MRRESETCONFIG                   0x020B
#define MRSETCNTLSNFE                   0x020C
#define MRMMINFO                        0x020D  /* MM_INFO struct BE => FE  */
#define MRUPDTGINFO                     0x020F
#define MRGETPORTTYPE                   0x0210
#define MRSETCHAPFE                     0x0211
#define MRSETISNSINFOFE                 0x0212
#define MRSETPRES                       0x0213
#define MRSETFT                         0x0214
#define MRBFFUNCMAX                     0x0214      /* Max of above. */
/* @} */

/**
**  Log Entry MRP command codes
**  @addtogroup MRP_LOG_ENTRY
**  @{
**/
#define MRLOGFE                         0x0300
#define MRLOGBE                         0x0301
/* @} */

/**
**  FE to BE MRP command codes
**  @addtogroup MRP_FE_TO_BE
**  @{
**/
#define MRFBFUNCBASE                    0x0400
#define MRFEGETVINFO                    0x0400
#define MRFESETSEQ                      0x0401
#define MRSETMPCONFIGBE                 0x0402  /* MRFESETMP in Bigfoot */
#define MRRETRIEVEPR                    0x0403
#define MRFBFUNCMAX                     0x0403      /* Max of above. */
/* @} */

/**
**  CCB to FE MRP command codes
**  @addtogroup MRP_CCB_TO_FE
**  @{
**/
#define MREFFUNCBASE                    0x0500
#define MRFELOOP                        0x0500
#define MRGETSINFO                      0x0501
#define MRGETCINFO                      0x0502
#define MRFELINK                        0x0503
#define MRFEII                          0x0504
#define MRGETCDINFO                     0x0505
#define MRGETSSTATS                     0x0506
#define MRSETBATHEALTH                  0x0507
#define MRRESUMECACHE                   0x0508
//#define MRFEBOOT                        0x0509    /* Obsolete */
//#define MRFEDIAG                        0x050A    /* Obsolete */
#define MRFEPROC                        0x050B
#define MRFECODEBURN                    0x050C
#define MRFRWMEM                        0x050D
#define MRRESETFEPORT                   0x050E
#define MRSERVERLOOKUP                  0x050F
#define MRFEGENERIC                     0x0510
#define MRSETMPCONFIGFE                 0x0511  /* MRFEASSIGNMP in Bigfoot */
#define MRFEFIBREHLIST                  0x0512
#define MRFECONTWOMP                    0x0513
#define MRFEFLUSHWOMP                   0x0514
#define MRINVFEWC                       0x0515
#define MRFLUSHBEWC                     0x0516
#define MRINVBEWC                       0x0517
#define MRFEMODEPAGE                    0x0518
#define MRFEGETDVLIST                   0x0519
#define MRFEGETPORTLIST                 0x051A
#define MRGETTRLIST                     0x051B
#define MRSTOPIO                        0x051C
#define MRSTARTIO                       0x051D
#define MRFEPORTNOTIFY                  0x051E
#define MRFORCEFEETRAP                  0x051F
#define MRFELOOPPRIMITIVE               0x0520
#define MRGETTARG                       0x0521
#define MRFAILPORT                      0x0522
#define MRNOPFE                         0x0523
#define MRQFECC                         0x0524  /* Query FE Controller Communications   */
#define MRQSC                           0x0525  /* Query Stop Completion                */
#define MRQMPC                          0x0526  /* Query Mirror Partner Change OK?      */
#define MRGETHABSTATS                   0x0527  /* Get HAB statistics                   */
#define MRMMCARDGETBATTERYSTATUS        0x0528  /* Get MM status                        */
#define MRGETBATTSTS                    0x0528  /* Get MM status                        */
#define MRGETMPCONFIGFE                 0x0529  /* Get MP configuration                 */
#define MRFEPORTGO                      0x052A  /* Put Regular Ports on FE Fabric       */
#define MRSETTDISCACHE                  0x052B  /* Set Temp Disable Cache               */
#define MRCLRTDISCACHE                  0x052C  /* Clear Temp Disable Cache             */
#define MRQTDISABLEDONE                 0x052D  /* Query WC Temp Disable Flush Done     */
#define MRMMTEST                        0x052E  /* Test driver for MM/NV Card           */
#define MRFEFLERRORWOMP                 0x052F  /* Flush FE Cache due to Error to MP    */
#define MRGETSESSIONS                   0x0530  /* Get Info of ISCSI Sessions           */
#define MRGETSESSIONSPERSERVER          0x0531  /* Get ISCSI Sessions on a server       */
#define MRGETIDDINFO                    0x0532  /* Get IDD Info */
#define MRDLMPATHSTATS                  0x0533  /* DLM Path Stats */
#define MRDLMPATHSELECTIONALGO          0x0534  /* DLM path options set                 */
#define MRPRGET                         0x0535  /* Persistent reserve get info          */
#define MRPRCLR                         0x0536  /* Persistent reserve clear info        */
#define MRPRCONFIGCOMPLETE              0x0537  /* Persistent reservation config completed */
#define MRUPDPRR                        0x0538  /* Persistent reservation config update */
#define MRSETFEPORTCONFIG               0x0539  /* Set FE FC port configuration         */
#define MREFFUNCMAX                     0x0539  /* Max of above. */

#define MRFEHBEAT                       0x05FF
/* @} */

/**
**  Codes reserved for other communications methods (BESRP and WRPs)
**
**  0x0600-0x06FF
**/

/* CCB to DLM MRP command codes */
#define MRCCBTODLM                      0x0700

/* MRP return codes */
#define DEOK                0x00    /* 0 - OK status                             */
#define DEINVPKTTYP         0x01    /* 1 - Invalid packet type                   */
#define DEINVPKTSIZ         0x02    /* 2 - Invalid packet size                   */
#define DEBADDAM            0x03    /* 3 - Bad DAM entry                         */
#define DELISTERROR         0x04    /* 4 - error getting list                    */
#define DE2TBLIMIT          0x05    /* 5 - 2TB limit exceeded                    */
#define DENONXDEV           0x06    /* 6 - Non-existent device                   */
#define DEINOPDEV           0x07    /* 7 - Inoperative device                    */
#define DEINVLABTYP         0x08    /* 8 - Invalid labtype                       */
#define DEDEVUSED           0x09    /* 9 - Specified device in use               */
#define DEINITINPROG        0x0A    /* 10 - Initialization in progress            */
#define DEINVWWNAME         0x0B    /* 11 - Invalid world wide name               */
#define DEIOERR             0x0C    /* 12 - I/O error                             */
#define DEINVRTYPE          0x0D    /* 13 - Invalid RAID type                     */
#define DECODEBURN          0x0E    /* 14 - Code burn failed                      */
#define DEDEFNRDY           0x0F    /* 15 - Define is not ready for full cmd set  */
#define DEINSDEVCAP         0x10    /* 16 - Insufficient device capacity          */
#define DESESINPROGRESS     0x11    /* 17 - SES discovery in progress             */
#define DEINVDRVCNT         0x12    /* 18 - Invalid drive count                   */
#define DEACTIVEDEF         0x13    /* 19 - Active Defrag                         */
#define DENOTDATALAB        0x14    /* 20 - Not labelled data device              */
#define DEINVDEPTH          0x15    /* 21 - Invalid depth                         */
#define DEINVSTRIPE         0x16    /* 22 - Invalid stripe size                   */
#define DEINVPARITY         0x17    /* 23 - Invalid parity specified              */
#define DEINVVIRTID         0x18    /* 24 - Invalid virtual ID                    */
#define DEINVOP             0x19    /* 25 - Invalid operation                     */
#define DEINVSESSLOT        0x1A    /* 26 - Invalid SES or slot information       */
#define DEBUSY              0x1B    /* 27 - Busy                                  */
#define DEMAXLUNS           0x1C    /* 28 - Max Luns                              */
#define DENOTOPRID          0x1D    /* 29 - RAID device not operative             */
#define DEMAXSEGS           0x1E    /* 30 - Maximum segments exist                */
#define DEBADNVREC          0x1F    /* 31 - nvram p2 record on disk is not valid  */
#define DEINSUFFREDUND      0x20    /* 32 - insufficent redundancy exists         */
#define DEPIDNOTUSED        0x21    /* 33 - PID not used by RAIDs on this controller */
#define DENOHOTSPARE        0x22    /* 34 - No hotspare drive or insufficient capacity */
#define DEINVPRI            0x23    /* 35 - Invalid priority                      */
#define DENOTSYNC           0x24    /* 36 - Secondary copy no synchronized        */
#define DEINSNVRAM          0x25    /* 37 - Insufficient NVRAM                    */
#define DEFOREIGNDEV        0x26    /* 38 - Foreign device                        */
#define DEREBUILDNOTREQ     0x27    /* 39 - Rebuild Not required                 */
#define DETYPEMISMATCH      0x28    /* 40 - Device type mismatch on create/expand */
#define DEINVOPT            0x29    /* 41 - Invalid option                        */
#define DEINVTID            0x2A    /* 42 - Invalid target ID                     */
#define DEINVCTRL           0x2B    /* 43 - Invalid controller ID                 */
#define DERETLENBAD         0x2C    /* 44 - Invalid return data space allowance   */
#define DEACTREBUILD        0x2D    /* 45 - Active rebuild                        */
#define DETOOMUCHDATA       0x2E    /* 46 - Too much data for return              */
#define DEINVSID            0x2F    /* 47 - Invalid server                        */
#define DEINVVID            0x30    /* 48 - Invalid VID number                    */
#define DEINVRID            0x31    /* 49 - Invalid RID number                    */
#define DEINVPID            0x32    /* 50 - Invalid PID number                    */
#define DEINSTABLE          0x33    /* 51 - Insufficient table space              */
#define DEINSRES            0x34    /* 52 - Insufficient resources                */
#define DELUNMAPPED         0x35    /* 53 - LUN is already mapped                 */
#define DEINVCHAN           0x36    /* 54 - Invalid channel number                */
#define DELUNNOTMAPPED      0x37    /* 55 - LUN is not mapped                     */
#define DEWCRECVRYFAILED    0x38    /* 56 - Cache recovery failures occurred      */
#define DEQRESETFAILED      0x39    /* 57 - Qlogic port reset failed              */
#define DEUNASSPATH         0x3A    /* 58 - Unassigned Path                       */
#define DEOUTOPS            0x3B    /* 59 - Outstanding ops prevent completion    */
#define DECHECKSUM          0x3C    /* 60 - Bad checksum                          */
#define DEFAILED            0x3D    /* 61 - Operation failed                      */
#define DECODESAME          0x3E    /* 62 - Code loads are identical              */
#define DESTOPZERO          0x3F    /* 63 - Stop Count is already zero            */
#define DEINSMEM            0x40    /* 64 - Insufficient memory for file copy     */
#define DENOTARGET          0x41    /* 65 - No target for ports                   */
#define DENOPORT            0x42    /* 66 - No spare ports                        */
#define DEINVWSID           0x43    /* 67 - Invalid work set ID                   */
#define DEMAXSNAPSHOTS      0x44    /* 68 - Max Snapshot count exceeds            */
#define DEMAXCOPIES         0x45    /* 69 - Max copies count exceeds              */
/* UNUSED                   0x46 */ /* 70 */
#define DE_IP_OR_SUBNET_ZERO 0x47   /* 71 - IP or subnet address are 0        */
#define DE_SET_IP_ERROR     0x48    /* 72 - Error setting IP address              */
#define DE_VDISK_IN_USE     0x49    /* 73 - Vdisk in use on another controller    */
/* UNUSED                   0x4A */ /* 74 */
#define DEBADDEVTYPE        0x4B    /* 75 - Unknown type used for create/expand   */
#define DEEMPTYCPYLIST      0x50    /* 76 - The copy list is empty                */
#define DENOCPYMATCH        0x51    /* 77 - no copy matches the requirments       */
#define DENOCOMM            0x52    /* 78 - no communication to the new mirror partner */
#define DEPIDUSED           0x53    /* 79 - PID used by any RAIDs                 */
#define DEPIDSPUNDOWN       0x54    /* 80 - Designted PID has already been spundown */
#define DENOHSDNAME         0x55    /* 81 - HSDNAME is not available */
#define DENOFAILBACKPDD     0x56    /* 82 - No [drive or valid-drive found for failback */
#define DENOTHOTLAB         0x57    /* 83 - Designated device is not a hotspare   */
#define DEDISKNOTINIT       0x58    /* 84 - Unable to initialize the drive        */
#define DEINVHSPTYPE        0x59    /* 85 - Hotspare not same devType as faling drive   */
#define DEINVSETMAP         0x60    /* 86 - Trying to configure a read only parameter */
#define DESETMTUFAIL        0x61    /* 87 - Failure setting MTU size on a port */
#define DEINVISCSIPARAM     0x62    /* 88 - Invalid iSCSI Parameter/Value      */
#define DEASWAPSTATE        0x63    /* 89 - aswap/aswapback in progress or src vdisk autoswapped */
#define DEASWAPINPROGRESS   0x64    /* 90 - Auto Swap in progress                 */
#define DEINVPKTVERSION     0x65    /* 91 - Incompatible packet version           */
#define DEINVALIDTARGET     0x66    /* 92 - For async_rep feature - to reject mapping to one controller's targets*/
#define DEBAYNOTFOUND       0x67    /* 93 - Bay/ISE not found  */
#define DEDESTNOTHOTLAB     0x68    /* 94 - Designated Destination device is not a hotspare   */
#define DE64TBLIMIT         0x69    /* 95 - 64TB limit exceeded                   */

/**
**  MRP Name Length Constants
**  @addtogroup MRP_NAME_LENGTH_CONSTANTS
**  @{
**/
#define NAME_DEVICE_NAME_LEN        16
#define MAX_SERVER_NAME_LEN         NAME_DEVICE_NAME_LEN
#define MAX_VDISK_NAME_LEN          NAME_DEVICE_NAME_LEN
/* @} */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
/**
**  @addtogroup MR_PKT Maintenance Request Packet
**  @{
**/
/** Maintenance Request Response Packet                                     */
typedef struct MR_RSP_PKT
{
    UINT8       rsvd0[3];   /**< RESERVED                                   */
    UINT8       status;     /**< Response status                            */
    UINT32      rspLen;     /**< Response length                            */
    UINT16      nDevs;      /**< Number of devices                          */
    UINT16      size;       /**< Size of entry                              */
} MR_RSP_PKT;

/** Maintenance Request Packet                                              */
typedef struct MR_PKT
{
    UINT16      function;   /**< Function                                   */
    UINT8       version;    /**< Command Version                            */
    UINT8       rsvd3[1];   /**< RESERVED                                   */
    UINT8       rsvd4[8];   /**< RESERVED                                   */
    void       *pReqPCI;    /**< MRP pointer across the PCI bus             */
    UINT32      rspLen;     /**< Response data allocation length            */
    MR_RSP_PKT *pRsp;       /**< Response data pointer                      */
    void       *pReq;       /**< Request packet pointer                     */
    UINT32      reqLen;     /**< Request packet length                      */
} MR_PKT;
/* @} */

/**
******************************************************************************
**  GENERIC REQUEST and RESPONSE PACKETS
**  @addtogroup MRP_GENERIC
**  @{
******************************************************************************
**/
/**
**  @addtogroup MR_DEVID_REQ Device ID request
**  @{
**/
/** Device ID request packet                                                */
typedef struct MR_DEVID_REQ
{
    UINT16  id;             /**< Device identifier                          */
    UINT16  option;         /**< Option                                     */
} MR_DEVID_REQ;
/* @} */

/**
**  @addtogroup MR_HDR_RSP Maintenance request response header
**  @{
**/
/** Maintenance request response header                                     */
typedef struct MR_HDR_RSP
{
    UINT8   rsvd0[3];       /**< RESERVED                                   */
    UINT8   status;         /**< MRP Completion Status                      */
    UINT32  len;            /**< Length of this return packet in bytes      */
} MR_HDR_RSP;
/* @} */

/**
**  @addtogroup MR_GENERIC_RSP Generic response
**  @{
**/
/** Generic MRP response packet                                             */
typedef struct MR_GENERIC_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MR_GENERIC_RSP;
/* @} */

/**
**  @addtogroup MRINVWC
**  FE & BE Invalidate Write Cache
**  @{
**/
/** FE & BE Invalidate Write Cache - Request **/
typedef struct MRINVWC_REQ
{
    UINT8   option;
    UINT8   rsvd;
    UINT16  numVids;
    ZeroArray(UINT16, vidList);
} MRINVWC_REQ;

/** FE & BE Invalidate Write Cache - Response **/
typedef struct MRINVWC_RSP
{
    MR_HDR_RSP  header;         /**< MRP Response Header - 8 bytes          */
    UINT16      invalidVid;     /**< Invalid VID, if any                    */
    UINT8       rsvd[2];        /*   Reserved                               */
} MRINVWC_RSP;
/* @} */

/**
**  @addtogroup MRHBEAT
**  FE & BE Heartbeat
**  @{
**/
/** FE & BE Heartbeat - Request **/
typedef struct MRHBEAT_REQ
{
    TIMESTAMP   ts;         /**< Signal Type                                */
    UINT8       rsvd[4];    /* RESERVED                                     */
} MRHBEAT_REQ;

/** BE Heartbeat - Response **/
typedef struct MRHBEAT_RSP
{
    MR_HDR_RSP  header;         /**< MRP Response Header - 8 bytes          */
    UINT64      ioPerSec;       /**< Aggregated I/O per second              */
    UINT64      bytesPerSec;    /**< Aggregated bytes per second            */
} MRHBEAT_RSP;
/* @} */

/**
**  @addtogroup MR_LIST_RSP Generic list response
**  @{
**/

/** List MRP response packet                                                */
typedef struct MR_LIST_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT16      ndevs;      /**< Number of items in the list                */
    UINT8       rsvd10[2];  /**< RESERVED                                   */
    UINT16      list[1];    /**< List of IDs                                */
} MR_LIST_RSP;
/* @} */

/**
**  @addtogroup MR_GET_PE_INFO_RSP Physical or Enclosure Information Response
**  @{
**/
/** Get physical device information - Response **/
typedef struct MR_GET_PE_INFO_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    PDD         pdd;        /**< Physical Device Descriptor                 */
} MR_GET_PE_INFO_RSP;
/* @} */

/**
**  @addtogroup MR_RESET Generic packet for MRRESET and MRRESETCONFIG
**  @{
**/
/* Values for MR_RESET type                                                 */
#define MXNALL      0x0         /**< Clear everything                       */
#define MXNFENVA    0x1         /**< Clear FE NVA records                   */
#define MXNNMI      0x2         /**< Clear NMI counts                       */
#define MXNBENVA    0x4         /**< Clear BE NVA records                   */

/** Reset request   */
typedef struct MR_RESET_REQ
{
    UINT32  type;           /**< Reset type                                 */
} MR_RESET_REQ;
/* @} */
/* @} */

/**
**  @addtogroup MRRESETPORT  Reset FE/BE Port
**  @{
**/

/**
**  @name MRRESETPORT - port values
**  @{
**/
#define RESET_PORT_NEEDED               0xF0    /**< Reset necessary ports  */
#define RESET_PORT_ALL                  0xFF    /**< Reset all ports        */
/* @} */

/**
**  @name MRRESETPORT - option values
**  @{
**/
#define RESET_PORT_INIT                 0x00    /**<                        */
#define RESET_PORT_NO_INIT              0x01    /**<                        */
#define RESET_PORT_INIT_IF_OFFLINE      0x02    /**<                        */
#define RESET_PORT_NO_INIT_IF_OFFLINE   0x03    /**<                        */
#define RESET_PORT_INIT_LOG             0x04    /**<                        */
#define RESET_PORT_NO_INIT_LOG          0x05    /**<                        */
#define RESET_PORT_FORCE_SYS_ERR        0xFE    /**< Force System Error     */
#define RESET_PORT_MEMDUMP              0xFF    /**< Memory Dump            */
/* @} */

/** Reset port request (used for BE and FE)                                 */
typedef struct MRRESETPORT_REQ
{
    UINT8   port;           /**< Port number                                */
    UINT8   rsvd1;          /**< RESERVED                                   */
    UINT8   option;         /**< Reset Option                               */
    UINT8   rsvd3;          /**< RESERVED                                   */
} MRRESETPORT_REQ;

/** Reset port response (used for BE and FE)                                */
#define MRRESETPORT_RSP     MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRPORT Port information
**  @{
**/

/**
**  @name MRPORT - option values
**  @{
**/
#define PORT_STATS_RLS  0x01    /**< Show RLS ELS data in return data       */
/* @} */

/** Port information request                                                */
typedef struct MRPORT_REQ
{
    UINT8   port;           /**< Port number                                */
    UINT8   rsvd1;          /**< RESERVED                                   */
    UINT8   option;         /**< Options                                    */
    UINT8   rsvd3;          /**< RESERVED                                   */
} MRPORT_REQ;


/** Revision Information                                                    */
typedef struct MRPORT_REV
{
    UINT32  vendid;         /**< Vendor ID                                  */
    UINT32  model;          /**< Vendor model                               */
    UINT16  revlvl;         /**< Revision Level of ISP                      */
    UINT16  rsclvl;         /**< RISC revision Level                        */
    UINT16  fpmlvl;         /**< FB & FPM revision levels                   */
    UINT16  romlvl;         /**< RISC ROM revision level                    */
    UINT64  type;           /**< Firmware type (ef/efm)                     */
    UINT16  fwmajor;        /**< ISP firmware major revision                */
    UINT16  fwminor;        /**< ISP firmware minor revision                */
    UINT16  fwsub;          /**< ISP firmware subminor revision             */
    UINT16  fwattrib;       /**< ISP firmware attribute                     */
    UINT16  dataRate;       /**< Data Rate (1G/2G)                          */
} MRPORT_REV;

/* VendorID values                                                          */
#define VEND_QLOGIC         0x1077      /**< QLogic Vendor ID               */
#define VEND_MM             0x1332      /**< Micro Memory Vendor ID         */
#define VEND_ISCSI          0x8086      /**< Intel Corporation Vendor ID    */

/* Values for MRPORT_RSP state                                              */
#define PORT_UNINT          0x0001      /**< Uninitialized                  */
#define PORT_FAIL           0x0002      /**< Failed                         */
#define PORT_OFFLINE        0x0004      /**< Offline                        */
#define PORT_OFFLINEFAIL    0x0008      /**< Offline failed                 */
#define PORT_TAGGED         0x0010      /**< Fail-mark.                     */

/**
** NOTE: PORT_NOTINSTALLED is not returned by the proc.
** It is used by the CCB to indicate that a given port does not
** have a card installed.
**/
#define PORT_NOTINSTALLED   0x8000

/* Mask definitions for GPIOD field in MRPORT_RSP.                          */
#define PORT_GPIOD_J2_JUMPER_MASK   0x04

/** Port information response                                               */
typedef struct MRPORT_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT8       numhosts;   /**< Number of active hosts                     */
    UINT8       lid;        /**< Loop ID                                    */
    UINT16      rsvd10;     /**< RESERVED                                   */
    ISP_RLS     rls;        /**< ISP Read Link Status                       */
    MRPORT_REV  rev;        /**< Revision Information                       */
    UINT16      state;      /**< port state                                 */
    UINT8       numtarg;    /**< Number of targets                          */
    UINT8       GPIOD;      /**< ISP GPIOD register content (low byte only) */
    UINT8       rsvd2[2];   /**< RESERVED                                   */
    ZeroArray(UINT16, target); /**< Targets                                 */
} MRPORT_RSP;
/* @} */

/**
**  @addtogroup MRCONFIG Port information
**  @{
**/

/** Port configuration request */
typedef struct MRCONFIG_REQ
{
    ISP_CONFIG  config;     /**< ISP configuration */
} MRCONFIG_REQ;

/** Port configuration response */
#define MRCONFIG_RSP    MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRGETDVLIST Get device list
**  @{
**/
/** Get device list information (used in device list response)                  */
typedef struct MRGETDVLIST_INFO
{
    UINT16  lid;            /**< Loop ID                                    */
    UINT8   mst;            /**< Master State                               */
    UINT8   sst;            /**< Slave State                                */
    UINT32  portID;         /**< Port ID                                    */
    UINT64  wwnPort;        /**< Port wwn                                   */
    UINT64  wwnNode;        /**< Node wwn                                   */
    UINT8   rsvd24[8];      /**< RESERVED                                   */
} MRGETDVLIST_INFO;

/** Get device list request                                                     */
typedef struct MRGETDVLIST_REQ
{
    UINT8   port;           /**< port to retrieve list                      */
    UINT8   rsvd1[3];       /**< RESERVED                                   */
} MRGETDVLIST_REQ;

/** Get device list response                                                    */
typedef struct MRGETDVLIST_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT16      ndevs;      /**< Number of virtual disk owners in the list  */
    UINT8       rsvd10[2];  /**< RESERVED                                   */
    MRGETDVLIST_INFO list[256]; /**< Device information list                */
} MRGETDVLIST_RSP;
/* @} */

/**
**  @addtogroup MRGETPORTLIST Get port list
**  @{
**/
/**
**  @name Get Port List Request Types
**  @{
**/
#define PORTS_ALL           0x00 /**< All ports                             */
#define PORTS_INITIALIZED   0x01 /**< Initialized ports                     */
#define PORTS_FAILED        0x02 /**< Failed ports                          */
#define PORTS_INITIALIZING  0x03 /**< Ports in process of initializing      */
#define PORTS_GOOD          0x04 /**< Ports not failed nor offine failed    */
#define PORTS_ONLINE        0x05 /**< Ports Online (with the loop up)       */
#define PORTS_OFFLINE       0x06 /**< Ports Offline (with the loop down)    */
#define PORTS_FAILMARK      0x07 /**< Ports tagged as failed                */
#define PORTS_STATUS        0x08 /**< All Port with status                  */
#define PORTS_WITH_TARGETS  0x10 /**< Ports with targets assigned           */
#define PORTS_NO_TARGETS    0x11 /**< Ports without targets assigned        */
/* @} */

/** Get Port List request                                                   */
typedef struct MRGETPORTLIST_REQ
{
    UINT16  type;           /**< Request type                               */
    UINT8   rsvd2[2];       /**< RESERVED                                   */
} MRGETPORTLIST_REQ;
/* @} */

/**
**  @addtogroup MR_LOOP_PRIMITIVE Loop primitive
**  @{
**/
#define MLPRESLOOP     0        /**< LIP Reset an entire loop               */
#define MLPRESLIDPORT  1        /**< LIP Reset a LID port                   */
#define MLPSIDPIDRES   2        /**< FE: SID LIP Reset, BE: PID LIP reset   */
#define MLPINITLIP     3        /**< Initiate Loop Initialization (LIP)     */

#define MLPLOGINLID    0x11     /**< Login a port specified by a LID        */
#define MLPLOGINPID    0x12     /**< Login a port specified by a PID        */

#define MLPLOGOUTLID   0x21     /**< Logout a port specified by a LID       */
#define MLPLOGOUTPID   0x22     /**< Logout a port specified by a PID       */

#define MLPTRSTLID     0x31     /**< Target Reset a port specified by a LID */
#define MLPTRSTPID     0x32     /**< Target Reset a port specified by a PID */

#define MLPLPBLID      0x41     /**< Loop port bypass specified by a LID    */
#define MLPLPBPID      0x42     /**< Loop port bypass specified by a PID    */

#define MLPLPELID      0x51     /**< Loop port enable specified by a LID    */
#define MLPLPEPID      0x52     /**< Loop port enable specified by a PID    */

#define MLPDRPLID      0xF1     /**< Drop redundant path by a LID           */
#define MLPDRPPID      0xF2     /**< Drop redundant path by a PID           */

/** Loop primitive request                                                  */
typedef struct MRLOOPPRIMITIVE_REQ
{
    UINT16      option;     /**< Option - see below                         */
    UINT16      id;         /**< SID or PID to use                          */
    UINT8       port;       /**< Port                                       */
    UINT8       rsvd5[3];   /**< RESERVED                                   */
    UINT32      lid;        /**< LID                                        */
} MRLOOPPRIMITIVE_REQ;

/** Loop primitive response                                                 */
#define MRLOOPPRIMITIVE_RSP     MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MR_BEFE_GENERIC Generic MRP
**  @{
**/
/** Generic MRP request                                                     */
typedef struct MR_BEFE_GENERIC_REQ
{
    UINT32      parm0;      /**< Input parameter 0                          */
    UINT32      parm1;      /**< Input parameter 1                          */
    UINT32      parm2;      /**< Input parameter 2                          */
    UINT32      parm3;      /**< Input parameter 3                          */
} MR_BEFE_GENERIC_REQ;

/** Generic MRP response                                                    */
typedef struct MR_BEFE_GENERIC_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT32      rparm0;     /**< Return parameter 0                         */
    UINT32      rparm1;     /**< Return parameter 1                         */
    UINT32      rparm2;     /**< Return parameter 2                         */
    UINT32      rparm3;     /**< Return parameter 3                         */
} MR_BEFE_GENERIC_RSP;
/* @} */

/**
**  @addtogroup MR_MODE_SET Mode Set MRP
**  @{
**/
/**
**  @name BE/FE Bit Definitions for modeData[0]
**  @{
**/
#define MMP_DISABLE_HEARTBEAT       0   /**< Heartbeat Monitor              */
#define MMP_DISABLE_BOOT_ERRTRAP    1   /**< Boot code Error Trap handling  */
#define MMP_DISABLE_CNTRL_SHUTDOWN  2   /**< Ignore Heartbeat failure       */
/* @} */

/** Mode Set MRP request                                                    */
typedef struct MR_MODE_SET_REQ
{
    UINT32      modeData[4];    /**< Processor Mode Data                    */
    UINT32      modeMask[4];    /**< Processor Mode Mask                    */
} MR_MODE_SET_REQ;
/* @} */

/**
**  @addtogroup MR_CODE_BURN Code Burn MRP
**  @{
**/
/** Code Burn MRP request                                                   */
typedef struct MR_CODE_BURN_REQ
{
    UINT32      addr;       /**< PCI address of the data to burn            */
} MR_CODE_BURN_REQ;
/* @} */

/**
**  @addtogroup MR_FSYS_OP_RSP File System Operation MRP
**  @{
**/
/** File System Operation Disk Map                                          */
typedef UINT8 FIO_DISK_MAP[(MAX_PHYSICAL_DISKS + 7) / 8];

/** File System Operation MRP response                                      */
typedef struct MR_FSYS_OP_RSP
{
    MR_HDR_RSP      header;     /**< MRP Response Header - 8 bytes          */
    UINT32          good;       /**< Number devices written without error   */
    UINT32          error;      /**< Number devices written with an error   */
    FIO_DISK_MAP    goodMap;    /**< Good read device map                   */
} MR_FSYS_OP_RSP;
/* @} */

/**
**  @addtogroup MR_RW_MEMORY Read/Write Memory MRP
**  @{
**/
/** Read/Write Memory MRP request                                           */
typedef struct MR_RW_MEMORY_REQ
{
    void       *srcAddr;        /**< Source Address                         */
    void       *dstAddr;        /**< Destination PCI Address                */
    UINT32      length;         /**< Read/Write request size                */
} MR_RW_MEMORY_REQ;

/** Read/Write Memory MRP response                                          */
#define MR_RW_MEMORY_RSP        MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MR_FW_HEADER_RSP Firmware Header MRP Response
**  @{
**/
/** Firmware Header MRP response                                            */
typedef struct MR_FW_HEADER_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    FW_HEADER   fw;         /**< Firmware header                            */
} MR_FW_HEADER_RSP;
/* @} */

/**
**  @addtogroup MR_LINK_RSP Link MRP Response
**  @{
**/
/** Link MRP response                                            */
typedef struct MR_LINK_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT16      vrpocount;  /**< Outstanding outbound VRP count             */
    UINT16      vrpicount;  /**< Outstanding inbound VRP count              */
    UINT32      vrpotcount; /**< Total outbound VRP count                   */
    UINT32      vrpitcount; /**< Total inbound VRP count                    */
    UINT8       rsvd1[4];   /**< RESERVED                                   */
} MR_LINK_RSP;
/* @} */

/**
******************************************************************************
**  CCB to BE MRP REQUEST and RESPONSE PACKETS
**  @addtogroup MRP_CCB_TO_BE
**  @{
******************************************************************************
**/
/**
**  @addtogroup MRCREXP
**  Create or expand a virtual device
**  @{
**/
/**
**  @name MRCREXP Raid Types
**  @{
**/
#define MCR_STD                     0   /**< Standard (non-RAID)            */
#define MCR_RAID0                   1   /**< RAID 0                         */
#define MCR_RAID1                   2   /**< RAID 1                         */
#define MCR_RAID5                   3   /**< RAID 5                         */
#define MCR_RAID10                  4   /**< RAID 10 (0/1)                  */
/* @} */

/**
**  @name MRCREXP Operations
**  @{
**/
#define MCR_OP_CREATE               0   /**< Create device                  */
#define MCR_OP_EXPAND               1   /**< Expand device                  */
#define MCR_OP_TEST                 2   /**< Test device                    */
/* @} */

/**
**  @name MRCREXP_REQ flag field definitions
**  @{
**/
#define MCR_FLAGS_BITS_MINPD        0 /**< 1=exactly minPD, 0= at least minPD*/
#define MCR_FLAGS_BITS_REDUN        1 /**< Enforce bay & bus redundancy     */
#define MCR_FLAGS_BITS_GEO          2 /**< Geo RAID enabled                 */
#define MCR_FLAGS_BITS_7000MIN      3 /**< MinPD, up to 16 looping till works */
#define MCR_FLAGS_BITS_SET_SIZE     4 /**< Reserved for MMC                 */
#define MCR_FLAGS_BITS_RSVD_5       5 /**< Reserved for MMC                 */
#define MCR_FLAGS_BITS_RSVD_6       6 /**< Reserved for MMC                 */
#define MCR_FLAGS_BITS_RSVD_7       7 /**< Reserved for MMC                 */

#define MRCREXP_EXACT_MIN_PD        0x01    /* 1=exactly minPD, 0= at least minPD */
#define MRCREXP_FORCE_REDUNDANCY    0x02    /* Enforce bay & bus redundancy */
#define MRCREXP_GEO_RAID            0x04    /* Geo RAID enabled             */
#define MRCREXP_7000MIN             0x08    /* MinPD, up to 16 looping till works */
#define MRCREXP_SET_SIZE            0x10    /* Set vdisk size */
#define MRCREXP_RSVD_MMC_5          0x20
#define MRCREXP_RSVD_MMC_6          0x40
#define MRCREXP_RSVD_MMC_7          0x80
/* @} */

/**
**  @name MRCREXP_REQ default depth value
**  @{
**/
#define MRCREXP_DEPTH_DEFAULT       2
/* @} */

/** Create or expand a virtual device - Request **/
typedef struct MRCREXP_REQ
{
    UINT8   rtype;          /**< RAID type                                  */
    UINT8   op;             /**< Operation 0x00=create, 0x01=expand         */
    UINT16  vid;            /**< Virtual ID to expand. Ignored for create   */
    UINT16  drives;         /**< Number of drives in the RAID               */
    UINT16  stripe;         /**< Number of sectors in the stripe            */
    UINT64  devcap;         /**< RAID capacity in 512 byte blocks           */
    UINT8   depth;          /**< Mirror depth (2 or more)                   */
    UINT8   parity;         /**< Parity level for RAID 5                    */
    UINT16  maxraids;       /**< Max RAIDs in this op                       */
    UINT16  thresh;         /**< Min space on each disk                     */
    UINT8   flags;          /**< See definitions above                      */
    UINT8   minPD;          /**< If !=0, min. # of PDisks per RAID create   */
    ZeroArray(UINT16, dmap); /**< List of physical drive IDs                */
} MRCREXP_REQ;

/** Create or expand a virtual device - Response **/
typedef struct MRCREXP_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT16      vid;        /**< Virtual ID for the virtual device created  */
    UINT8       clFlag;     /**< Multiple GEO location flag                 */
    UINT8       rsvd10;     /**< RESERVED                                   */
    UINT64      devcap;     /**< RAID capacity in 512 byte blocks           */
} MRCREXP_RSP;
/* @} */

/**
**  @addtogroup MRGETELIST
**  Get SES device list
**  @{
**/
/** Get SES device list - Request **/
#define MRGETELIST_REQ      MR_DEVID_REQ

/** Get SES device list - Response **/
#define MRGETELIST_RSP      MR_LIST_RSP
/* @} */

/**
**  @addtogroup MRLABEL
**  Label a physical device
**  @{
**/
/**
**  @name Label Types
**  @{
**/
#define MLDNOLABEL      0x00    /**< Label Type - Unlabelled device         */
#define MLDDATALABEL    0x01    /**< Label Type - Data device               */
#define MLDSPARELABEL   0x02    /**< Label Type - Hotspare device           */
#define MLDNDATALABEL   0x03    /**< Label Type - Non-redundant data device */
#define MLDFIXDNAME     0xFE    /**< Label Type - Fixup device DNAME        */
#define MLDRELABEL      0xFF    /**< Label Type - Relabel device            */
/* @} */

/**
**  @name Label Options
**  @{
**/
#define MLDFULL             0   /**< Option - Full init prior to label      */
#define MLDFSYS             1   /**< Option - Init file sytem               */
#define MLDDUPLICATE        2   /**< Option - Duplicate the file sys off good disk*/

#define MLD_OPT_FULL        (1 << MLDFULL)
#define MLD_OPT_FSYS        (1 << MLDFSYS)
#define MLD_OPT_DUPLICATE   (1 << MLDDUPLICATE)
/* @} */

/** Label a physical device - Request **/
typedef struct MRLABEL_REQ
{
    UINT16  pid;            /**< PID of the drive to be labelled            */
    UINT8   labtype;        /**< Label type                                 */
    UINT8   option;         /**< Option                                     */
    UINT8   dname[4];       /**< Drive name                                 */
} MRLABEL_REQ;

/** Label a physical device - Response **/
#define MRLABEL_RSP         MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRFAIL
**  Fail a device
**  @{
**/
/**
**  @name MRFAIL Options - bit masks
**  @{
**/
#define FP_FORCE        0x01    /**< Force hotspare, ignore redundancy req. */
#define FP_USE_HSPID    0x02    /**< Use given hspid                        */
#define FP_FAILLABEL    0x04    /**< Write a fail label on the drive        */
#define FP_SPIN_DOWN    0x08    /**< Spin the drive down                    */
#define FP_LOG_RAID_ERROR   0x10    /**< Log a raid error                   */
#define FP_CHECK_SYS    0x20    /**< Check system                           */
/* @} */

/** Fail a device - Request **/
typedef struct MRFAIL_REQ
{
    UINT16  pid;            /**< Physical device ID                         */
    UINT16  hspid;          /**< Hotspare physical device ID                */
    UINT8   options;        /**< Options                                    */
    UINT8   rsvd6[3];       /**< RESERVED                                   */
} MRFAIL_REQ;

/** Fail a device - Response **/
#define MRFAIL_RSP          MR_GENERIC_RSP
/* @} */

/**
**  @name MRPDISKFAILBACK Options - bit masks
**  @{
**/

#define FBP_FAILBACK_CANCEL  0x01       /**< failback cancel
                                         This device is cancelled from failback */
/* @} */

/** unfail a device - Request **/
typedef struct MRPDISKFAILBACK_REQ
{
    UINT16  hspid;          /**< Hotspare physical device ID to be failed back */
    UINT8   options;        /**< Options                                       */
    UINT8   rsvd6[1];       /**< RESERVED                                      */
} MRPDISKFAILBACK_REQ;

/** unfail a device - Response **/
#define MRPDISKFAILBACK_RSP          MR_GENERIC_RSP

/**
**  @name MRPDISKAUTOFAILBACKENABLEDISABLE Options - bit masks
**  @{
**/
#define MR_RB_AUTOFAILBACK_DISABLE 0x00   /* Auto failback feature is disabled */
#define MR_RB_AUTOFAILBACK_ENABLE  0x01   /* Auto failback feature is enabled  */
#define MR_RB_AUTOFAILBACK_DISPLAY 0x02   /* Displays current state of auto failback
                                             feature (enabled or disabled)     */

/* @} */

/** Auto FailBack Enable/Disable - Request **/
typedef struct MRPDISKAUTOFAILBACKENABLEDISABLE_REQ
{
    UINT8   options;        /**< Options                                    */
    UINT8   rsvd6[3];       /**< RESERVED                                   */
} MRPDISKAUTOFAILBACKENABLEDISABLE_REQ;

/** Auto FailBack Enable/Disable - Response **/
typedef struct MRPDISKAUTOFAILBACKENABLEDISABLE_RSP
{
    MR_HDR_RSP      header;     /**< MRP Response Header - 8 bytes          */
    UINT8           mode;       /**< Mode ON/OFF                            */
    UINT8           rsvd[3];    /**< Reserved                               */
} MRPDISKAUTOFAILBACKENABLEDISABLE_RSP;

/**
**  @addtogroup MRSCSIIO
**  Raw SCSI IO
**  @{
**/

/**
**  @name MRSCSIIO
**  @{
**/
#define MRSCSIIO_USE_CH_ID      1
#define MRSCSIIO_USE_WWN        2
/* @} */

/**
**  @name MRSCSIIO - Function codes
**  @{
**/
#define MRSCSIIO_CTL        0x08    /**< SCSI IO w/o data           */
#define MRSCSIIO_INPUT      0x09    /**< SCSI IO w/ input data      */
#define MRSCSIIO_OUTPUT     0x0A    /**< SCSI IO w/ output data     */
/* @} */

/**
**  @name MRSCSIIO - Strategy codes
**  @{
**/
#define MRSCSIIO_LOW        0x00    /**< Low non-optimized priority   */
#define MRSCSIIO_NORM       0x01    /**< Normal optimized priority    */
#define MRSCSIIO_HIGH       0x02    /**< High optimized priority      */
/* @} */

/**
**  @name MRSCSIIO - Flags codes
**  @{
**/
#define MRSCSIIO_FLAGS_SLI  0x01    /**< Suppress length indication (bit)   */
#define MRSCSIIO_FLAGS_SPS  0x02    /**< Suppress pre-spin sense data       */
#define MRSCSIIO_FLAGS_SNX  0x04    /**< Suppress nonexist. dvc log events  */
#define MRSCSIIO_FLAGS_BCC  0x08    /**< Bypass Check Condition recovery    */
#define MRSCSIIO_FLAGS_BLP  0x10    /**< Bypass LIP Reset                   */
#define MRSCSIIO_FLAGS_ORT  0x20    /**< Ordered Tag                        */
/* @} */

/** Raw SCSI IO - Request **/
typedef struct MRSCSIIO_REQ
{
    UINT32  rsn;            /* Request serial number                        */
    UINT8   channel;        /* Channel                                      */
    UINT8   cmdlen;         /* Command length                               */
    UINT8   func;           /* Function                                     */
    UINT8   strat;          /* Strategy                                     */
    UINT8   idchc;          /* 1=use ch/id, 2=use WWN                       */
    UINT8   rsvd[3];        /* RESERVED                                     */
    UINT64  wwn;            /* World Wide Name                              */
    UINT32  id;             /* ID                                           */
    UINT8   retry;          /* Retry count                                  */
    UINT8   flags;          /* Flags                                        */
    UINT16  lun;            /* LUN                                          */
    UINT64  sda;            /* Starting disk address                        */
    UINT64  eda;            /* Ending disk address                          */
    UINT32  timeout;        /* Timeout                                      */
    UINT32  rsvd1;          /* RESERVED                                     */
    void    *bptr;          /* Data pointer                                 */
    UINT32  blen;           /* Data length                                  */
    UINT8   cdb[16];        /* Command                                      */
} MRSCSIIO_REQ;

/** Raw SCSI IO - Response **/
typedef struct MRSCSIIO_RSP
{
//  NOTE: MR_HDR_RSP => 3 reserved bytes, then byte of status, then UINT32 len.
    UINT8   ascq;           /**< Addional sense code qualifier              */
    UINT8   asc;            /**< Addional sense code                        */
    UINT8   sense;          /**< Sense key                                  */
    UINT8   status;         /**< Completion status of the request           */
    UINT32  retLen;         /**< Return data length                         */
    UINT32  rsn;            /**< Request serial number                      */
} MRSCSIIO_RSP;
/* @} */


/** Raw READWRITE IO - Request **/
typedef struct MRREADWRITEIO_REQ
{
    UINT8       pv;             /* 'p'disk or 'v'disk           */
    UINT8       rw;             /* 'r'ead or 'w'rite            */
    UINT16      id;             /* pid or vid                   */
    UINT64      block;          /* Block number to start with   */
    void       *bptr;           /* Data pointer                 */
    UINT32      dataInLen;      /* Data Input length            */
} MRREADWRITEIO_REQ;

/** Raw READWRITE IO - Response **/
typedef struct MRREADWRITEIO_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MRREADWRITEIO_RSP;

/**
**  @addtogroup MRINITRAID
**  Initialize a RAID device
**  @{
**/
/** Initialize a RAID device - Request **/
#define MRINITRAID_REQ      MR_DEVID_REQ

/** Initialize a RAID device - Response **/
#define MRINITRAID_RSP      MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MROBSOLETE106
**  OBSOLETE
**  @{
**/
/* @} */

/**
**  @addtogroup MRDELVIRT
**  Delete a virtual device
**  @{
**/
/** Delete a virtual device - Request **/
#define MRDELVIRT_REQ       MR_DEVID_REQ

/** Delete a virtual device - Response **/
#define MRDELVIRT_RSP       MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRSETCACHE
**  Set caching mode
**  @{
**/
/**
**  @name MRSETCACHE Set Cache Mode
**  @{
**/
#define MSCGOFF         0x80    /**< Global Off                             */
#define MSCGON          0x81    /**< Global On                              */
/* @} */

/** Set caching mode - Request **/
typedef struct MRSETCACHE_REQ
{
    UINT8  rsvd0[3];        /**< RESERVED                                   */
    UINT8  mode;            /**< See doc for bit definitions                */
} MRSETCACHE_REQ;

/** Set caching mode - Response **/
#define MRSETCACHE_RSP      MR_GENERIC_RSP
/* @} */

/** Emulate PAB on a physical device - Request **/
#define MREMULATEPAB_REQ   MR_DEVID_REQ

/** Emulate PAB on a physical device - Response **/
typedef struct MREMULATEPAB_RSP
{
    UINT8   cnt;            /**< Count -- used for option 2                 */
    UINT8   rsvd1;          /**< RESERVED                                   */
    UINT8   rsvd2;          /**< RESERVED                                   */
    UINT8   status;         /**< MRP Completion Status                      */
    UINT32  len;            /**< Length of this return packet in bytes      */
} MREMULATEPAB_RSP;

/** SWAP two PIDs - Request **/
typedef struct MRSWAPPIDS_REQ
{
    UINT16  pid1;           /**< First PID to swap                          */
    UINT16  pid2;           /**< Second PID to swap                         */
} MRSWAPPIDS_REQ;

/** Swap two PIDs - Response **/
#define MRSWAPPIDS_RSP      MR_GENERIC_RSP

/**
**  @addtogroup MRSERVERPROP
**  Set server properties
**  @{
**/

/**
**  @name MRSERVERPROP_REQ - attr bit flags
**  @{
**/
#define MRSERVERPROP_MANAGE         0x00000001  /**< 1= server managed         */
#define MRSERVERPROP_SELECT_TARGET  0x00000002  /**< 1= use target for mapping */
#define MRSERVERPROP_XIO_CTRL       0x40000000  /**< 1 = XIOtech controller    */
#define MRSERVERPROP_DEFAULT_MAP    0x80000000  /**< 1 = Default LUN mappings  */
/* @} */

/** Set server properties - Request **/
typedef struct MRSERVERPROP_REQ
{
    UINT16  sid;            /**< ID of the server                           */
    UINT8   rsvd1;          /**< RESERVED                                   */
    UINT8   priority;       /**< Priority                                   */
    UINT32  attr;           /**< Attributes of the server                   */
} MRSERVERPROP_REQ;

/** Set server properties - Response **/
#define MRSERVERPROP_RSP    MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRRESET
**  Reset NVRAM, clear devices
**  @{
**/
/** Reset NVRAM, clear devices - Request **/
#define MRRESET_REQ         MR_RESET_REQ

/** Reset NVRAM, clear devices - Response **/
#define MRRESET_RSP         MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRRESTORE
**  Restore from NVRAM
**  @{
**/
/**
**  @name Restore from NVRAM options
**  @{
**/
#define MRNOBDISK       0       /**< 0x01 Option bit read from disk (c/l/i)      */
#define MRNOBWWN        1       /**< 0x02 Option bit read from disk (wwn)        */
#define MRNOBFSYS       2       /**< 0x04 Option bit read from fsys              */
#define MRNOBPCI        3       /**< 0x08 Option bit get from PCI                */
#define MRNOBFID        4       /**< 0x10 Option bit get from specific FID       */
#define MRNOBREFRESH    5       /**< 0x20 Option bit refresh                     */
#define MRNOBRESTORE    6       /**< 0x40 Option bit restore (clear and restore) */
#define MRNOBOVERLAY    7       /**< 0x80 Option bit overlay                     */

#define MRNODISK        (1 << MRNOBDISK)    /* 0x01 */
#define MRNOWWN         (1 << MRNOBWWN)     /* 0x02 */
#define MRNOFSYS        (1 << MRNOBFSYS)    /* 0x04 */
#define MRNOPCI         (1 << MRNOBPCI)     /* 0x08 */
#define MRNOFID         (1 << MRNOBFID)     /* 0x10 */
#define MRNOREFRESH     (1 << MRNOBREFRESH) /* 0x20 */
#define MRNORESTORE     (1 << MRNOBRESTORE) /* 0x40 */
#define MRNOOVERLAY     (1 << MRNOBOVERLAY) /* 0x80 */
/* @} */

/** Restore from NVRAM - Request **/
typedef struct MRRESTORE_REQ
{
    UINT8   channel;        /**< FC channel of device to get NVRAM from     */
    UINT8   opt;            /**< restore option - see flags above           */
    UINT16  lun;            /**< LUN of device to get NVRAM data from       */
    void   *addr;           /**< FC ID/PCI addr/FID to get NVRAM from       */
    UINT8   rsvd8[4];       /**< RESERVED                                   */
    UINT64  wwn;            /**< World Wide Name                            */
} MRRESTORE_REQ;

/** Restore from NVRAM - Response **/
#define MRRESTORE_RSP       MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRAWAKE
**  Awake (ping)
**  @{
**/
/**
**  @name Awake Bit/Value Definitions
**  @{
**/
#define MRAWAKE_BIT_NVRAMRDY    0   /**< NVRAM can be read up               */
#define MRAWAKE_BIT_MASTER      1   /**< Make this controller master        */
#define MRAWAKE_BIT_SLAVE       2   /**< Make this controller slave         */
#define MRAWAKE_BIT_CCBREQ      3   /**< CCB is required                    */
#define MRAWAKE_BIT_P2INIT      4   /**< P2 init may now run                */
#define MRAWAKE_BIT_FEINIT      5   /**< FE init may now run                */
#define MRAWAKE_BIT_REPLACEMENT 6   /**< Replacement, load from FSYS        */

#define MRAWAKE_NVRAMRDY        (1 << MRAWAKE_BIT_NVRAMRDY)
#define MRAWAKE_MASTER          (1 << MRAWAKE_BIT_MASTER)
#define MRAWAKE_SLAVE           (1 << MRAWAKE_BIT_SLAVE)
#define MRAWAKE_CCBREQ          (1 << MRAWAKE_BIT_CCBREQ)
#define MRAWAKE_P2INIT          (1 << MRAWAKE_BIT_P2INIT)
#define MRAWAKE_FEINIT          (1 << MRAWAKE_BIT_FEINIT)
#define MRAWAKE_REPLACEMENT     (1 << MRAWAKE_BIT_REPLACEMENT)
/* @} */

/** Awake (ping) - Request **/
typedef struct MRAWAKE_REQ
{
    UINT16  step;           /**< Signal Type                                */
    UINT8   rsvd2[2];       /**< RESERVED                                   */
} MRAWAKE_REQ;

/** Awake (ping) - Response **/
typedef struct MRAWAKE_RSP
{
//  NOTE: MR_HDR_RSP => 3 reserved bytes, then byte of status, then UINT32 len.
    UINT8   who;            /**< Who is responding                          */
    UINT8   rsvd1[2];       /**< RESERVED                                   */
    UINT8   status;         /**< Completion status of the request           */
    UINT32  len;            /**< Length of this return packet in bytes      */
} MRAWAKE_RSP;
/* @} */

/**
**  @addtogroup MRWWNLOOKUP
**  WWN/LUN lookup
**  @{
**/
#define MWLNONE     0x00    /**< No match                                   */
#define MWLDISK     0x01    /**< Disk drive                                 */
#define MWLMISC     0x02    /**< Miscellaneous device                       */
#define MWLSES      0x03    /**< SES enclosure                              */

/** WWN/LUN lookup - Request **/
typedef struct MRWWNLOOKUP_REQ
{
    UINT16  lun;            /**< LUN value                                  */
    UINT8   rsvd2[6];       /**< RESERVED                                   */
    UINT64  wwn;            /**< World Wide Name                            */
} MRWWNLOOKUP_REQ;

/** WWN/LUN lookup - Response **/
typedef struct MRWWNLOOKUP_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT8       rsvd8[4];   /**< RESERVED                                   */
    UINT16      id;         /**< Device ID                                  */
    UINT8       rsvd14;     /**< RESERVED                                   */
    UINT8       type;       /**< Type of device - defined below             */
} MRWWNLOOKUP_RSP;
/* @} */

/**
**  @addtogroup MRBEGENERIC
**  Generic
**  @{
**/
/** Generic - Request **/
#define MRBEGENERIC_REQ     MR_BEFE_GENERIC_REQ

/** Generic - Response **/
#define MRBEGENERIC_RSP     MR_BEFE_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRSTARTSTOP
**  Start or stop a device
**  @{
**/
/** Start or stop a device - Request **/
typedef struct MRSTARTSTOP_REQ
{
    UINT8       tempPlaceHolder;    /**< Temporary place holder             */
} MRSTARTSTOP_REQ;

/** Start or stop a device - Response **/
typedef struct MRSTARTSTOP_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MRSTARTSTOP_RSP;
/* @} */

/**
**  @addtogroup MRSCRUBCTRL
**  Enable/disable scrubbing
**  @{
**/

/**
**  @name Scrub Control Bit Definitions
**  @{
**/
#define SCRUB_POLL              0x00000000  /**< Poll current state         */
#define SCRUB_ENABLE            0x00000001  /**< Enable scrubbing           */
#define SCRUB_CHANGE            0x80000000  /**< Change request             */

#define PARITY_SCAN_ENABLE      0x80000001  /**< Parity scan enable         */
#define PARITY_SCAN_DISABLE     0x80000000  /**< Parity scan disable        */
#define PARITY_CHANGE           0x80000000  /**< Change request             */
/* @} */

/**
**  @name Parity Control Bit Masks
**  @{
**/
#define PC_DEFAULT              0x40        /**< Set default parms          */
#define PC_MARKED               0x20        /**< Check marked PSDs          */
#define PC_CORRUPT_MASK         0x40        /**< Corrupt parity             */
#define PC_SPECIFIC_MASK        0x10        /**< Check specific RAID        */
#define PC_CLEARLOG_MASK        0x08        /**< Clear log                  */
#define PC_1PASS_MASK           0x04        /**< Single pass                */
#define PC_CORRECT_MASK         0x02        /**< Correct errors             */
#define PC_ENABLE_MASK          0x01        /**< Enable parity checking     */
/* @} */

/** Enable/disable scrubbing - Request **/
typedef struct MRSCRUBCTRL_REQ
{
    UINT32  scrubcontrol;   /**< See below                                  */
    UINT32  paritycontrol;  /**< See below                                  */
    UINT16  raidid;         /**< RAID ID to use if raidmode=one             */
    UINT8   rsvd10[2];      /**< RESERVED                                   */
} MRSCRUBCTRL_REQ;

/** Enable/disable scrubbing - Response **/
typedef struct MRSCRUBCTRL_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT32      sstate;     /**< Current state of scrubbing (on or off)     */
    UINT32      pstate;     /**< Current state of parity (bit mask)         */
    UINT32      passes;     /**< Number of completed scan passes            */
    UINT16      scrubp;     /**< Current PID getting scrubbed               */
    UINT16      scanr;      /**< Current RID getting scanned                */
    UINT32      scrubb;     /**< Current block getting scrubbed             */
    UINT32      scanb;      /**< Current block getting scanned              */
} MRSCRUBCTRL_RSP;
/* @} */

/**
**  @addtogroup MRDEFAULT
**  Set default label behavior
**  @{
**/
/** Set default label behavior - Request **/
typedef struct MRDEFAULT_REQ
{
    UINT8       tempPlaceHolder;    /**< Temporary place holder             */
} MRDEFAULT_REQ;

/** Set default label behavior - Response **/
typedef struct MRDEFAULT_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MRDEFAULT_RSP;
/* @} */

/**
**  @addtogroup MRGETBEDEVPATHS
**  Get BE Device Paths
**  @{
**/
/**
**  @name MRGETBEDEVPATHS constants
**  @{
**/
#define DEV_PATH_MAX        4       /**< Max # of paths in path[] below     */
#define DEV_PATH_NO_CONN    0xFF    /**< No connection for a path           */

/**
**  Formats for MRGETBEDEVPATHS_REQ
**/
#define FORMAT_PID_BITPATH      0x00    /**< Bit set for each path          */
#define FORMAT_PID_PATH_ARRAY   0x01    /**< Array of driver ports          */
#define FORMAT_PID_LID_ARRAY    0x02    /**< Arrary of LIDs                 */

/**
**  Request types for MRGETBEDEVPATHS_REQ
**/
#define PATH_PHYSICAL_DISK  0x01        /**< Physical disk                  */
#define PATH_MISC_DEVICE    0x02        /**< Miscellaneaous Device          */
#define PATH_ENCLOSURES     0x03        /**< Enclosures                     */
/* @} */

/** Get BE Device Paths bit information                                     */
typedef struct MRGETBEDEVPATHS_RSP_BIT
{
    UINT16  pid;            /**< Physical disk id                           */
    UINT16  bitPath;        /**< Bit map of port with path to the device    */
}MRGETBEDEVPATHS_RSP_BIT;

/** Get BE Device Paths array information                                   */
typedef struct MRGETBEDEVPATHS_RSP_ARRAY
{
    UINT16  pid;                /**< Physical disk id                       */
    UINT16  pathCount;          /**< Number of paths to the device          */
    UINT8   path[DEV_PATH_MAX]; /**< Array of LID value for each path       */
}MRGETBEDEVPATHS_RSP_ARRAY;

/** Get BE Device Paths - Request **/
typedef struct MRGETBEDEVPATHS_REQ
{
    UINT16  type;           /**< Request type                               */
    UINT16  format;         /**< Format of return                           */
} MRGETBEDEVPATHS_REQ;

/** Get BE Device Paths - Response **/
typedef struct MRGETBEDEVPATHS_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT16      ndevs;      /**< Number of devices in the list              */
    UINT16      size;       /**< Size in bytes of each list entry           */
    ZeroArray(UINT8, list); /**< One of MRGETBEDEVPATHS_RSP_BIT
                             **  or MRGETBEDEVPATHS_RSP_ARRAY               */
} MRGETBEDEVPATHS_RSP;
/* @} */

/**
**  @addtogroup MRRESTOREDEV
**  Restore device
**  @{
**/
/** Restore device - Request **/
typedef struct MRRESTOREDEV_REQ
{
    UINT16  id;             /**< ID                                         */
    UINT16  option;         /**< Options - not currently used               */
} MRRESTOREDEV_REQ;

/** Restore device - Response **/
#define MRRESTOREDEV_RSP    MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRDEFRAGMENT
**  Defragment device
**  @{
**/
/** Defragment device - Request **/
#define MRDEFRAGMENT_ALL_PID    0xFFFF  /**< PID to indicate defrag all     */
#define MRDEFRAGMENT_STOP_PID   0xFFFE  /**< Stop defragment                */
#define MRDEFRAGMENT_ORPHANS    0xFFFD  /**< Orphan deletion code           */

/** control byte values         **/
#define MRDEFRAGMENT_PREPARE    0x0001  /**< Prepare (find PID/RID combo)   */
#define MRDEFRAGMENT_VERIFY     0x0002  /**< Start verification step        */
#define MRDEFRAGMENT_MOVE       0x0004  /**< Start the movement             */
#define MRDEFRAGMENT_STOP       0x0008  /**< Stop defragment all drives     */

typedef struct MRDEFRAGMENT_REQ
{
    UINT16  control;                    /**< Control from above             */
    UINT16  pdiskID;                    /**< PID to perform step on         */
    UINT16  raidID;                     /**< RID to perform step on         */
    UINT16  rsvd6;                      /**< reserved                       */
    UINT64  sda;                        /**< Starting disk address          */
} MRDEFRAGMENT_REQ;

/** Defragment device - Response **/
/** status byte values         **/
#define MRDEFRAGMENT_OK         0x0000  /**< Status is OK                   */
#define MRDEFRAGMENT_MISMATCH   0x0001  /**< Operation mismatch             */
#define MRDEFRAGMENT_NOMOVE     0x0002  /**< Nothing to move                */
#define MRDEFRAGMENT_CLEAR      0xFFFF  /**< Invalid response               */

typedef struct MRDEFRAGMENT_RSP
{
    MR_HDR_RSP  header;                 /**< MRP Response Header - 8 bytes  */
    UINT16      status;                 /**< Status of operation            */
    UINT16      pdiskID;                /**< PID to perform step on         */
    UINT16      raidID;                 /**< RID to perform step on         */
    UINT16      rsvd6;                  /**< reserved                       */
    UINT64      sda;                    /**< Starting disk address          */
} MRDEFRAGMENT_RSP;
/* @} */

/**
**  @addtogroup MRSETATTR
**  Set attribute
**  @{
**/
/** Set attribute - Request **/
typedef struct MRSETATTR_REQ
{
    UINT16  vid;            /**< ID of virtual device                       */
    UINT16  attr;           /**< See doc for bit definitions                */
} MRSETATTR_REQ;

/** Set attribute - Response **/
#define MRSETATTR_RSP       MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRBELOOP
**  Get back end loop information
**  @{
**/
/** Get back end loop information - Request **/
#define MRBELOOP_REQ        MRPORT_REQ

/** Get back end loop information - Response **/
#define MRBELOOP_RSP        MRPORT_RSP
/* @} */

/**
**  @addtogroup MRGETSLIST
**  Get server list
**  @{
**/
/** Get server list - Request **/
#define MRGETSLIST_REQ      MR_DEVID_REQ

/** Get server list - Response **/
#define MRGETSLIST_RSP      MR_LIST_RSP
/* @} */

/**
**  @addtogroup MRGETVLIST
**  Get virtual device list
**  @{
**/
/** Get virtual device list - Request **/
#define MRGETVLIST_REQ      MR_DEVID_REQ

/** Get virtual device list - Response **/
#define MRGETVLIST_RSP      MR_LIST_RSP
/* @} */

/**
**  @addtogroup MRGETRLIST
**  Get RAID list
**  @{
**/
/** Get RAID list - Request **/
#define MRGETRLIST_REQ      MR_DEVID_REQ

/** Get RAID list - Response **/
#define MRGETRLIST_RSP      MR_LIST_RSP
/* @} */

/**
**  @addtogroup MRGETPLIST
**  Get physical device list
**  @{
**/
/** Get physical device list - Request **/
#define MRGETPLIST_REQ      MR_DEVID_REQ

/** Get physical device list - Response **/
#define MRGETPLIST_RSP      MR_LIST_RSP
/* @} */

/**
**  @addtogroup MRGETMLIST
**  Get misc device list
**  @{
**/
/** Get misc device list - Request **/
#define MRGETMLIST_REQ      MR_DEVID_REQ

/** Get misc device list - Response **/
#define MRGETMLIST_RSP      MR_LIST_RSP
/* @} */

/**
**  @addtogroup MRGETVINFO
**  Get virtual device information
**  @{
**/
/** Get virtual device information - Request **/
#define MRGETVINFO_REQ      MR_DEVID_REQ

/**
**  @todo Convert the MRGETVINFO_RSP structure to use the VDD structure
**        when the appropriate code changes have been made.
**        NOTE: The VDD structure contains additional fields compared to
**        the MRP response structure.
**/
/** Get virtual device information - Response **/
typedef struct MRGETVINFO_RSP
{
    MR_HDR_RSP      header;     /**< MRP Response Header - 8 bytes          */
    UINT16          vid;        /**< Virtual device ID                      */
    UINT8           mirror;     /**< Mirror status                          */
    UINT8           status;     /**< VDisk status                           */
    UINT16          scorvid;    /**< Secondary copy orig. VID               */
    UINT8           scpComp;    /**< Secondary copy percent complete        */
    UINT8           raidCnt;    /**< Number of RAIDs in this VDisk          */
    UINT64          devCap;     /**< Device capacity                        */
    UINT32          error;      /**< Error count                            */
    UINT32          qd;         /**< Queue depth                            */
    UINT32          rps;        /**< Avg req/sec (last second)              */
    UINT32          avgSC;      /**< Avg sector count (last second)         */
    UINT64          rReq;       /**< Read request count                     */
    UINT64          wReq;       /**< Write request count                    */
    UINT16          attr;       /**< VDisk attribute                        */
    UINT8           draidCnt;   /**< Deferred RAID count                    */
    UINT8           owner;      /**< Owner                                  */
    UINT8           priority;   /**< Priority                               */
    GR_GeoRaidVdiskInfo grInfo;  /**< Geo-RAID services related info        */
    UINT32          sprCnt;     /**< Sample period request count            */
    UINT32          spsCnt;     /**< Sample period sector  count            */
    void           *pSCHead;    /**< Original vd scmt list head pointer     */
    void           *pSCTail;    /**< Original vd scmt list tail pointer     */
    void           *pCPScmt;    /**< Copy vd scmt pointer                   */
    struct VLAR    *pVLinks;    /**< VLinks assoc. records                  */
    UINT8           name[MAX_VDISK_NAME_LEN]; /**< VDisk name               */
    UINT32          createTime; /**< VDisk create time stamp                */
    INT32           lastAccess; /**< VDisk last access time stamp           */
    UINT32          lastHrAvgIOPerSec; /**< Average IO/sec over last hour   */
    UINT32          lastHrAvgSCPerSec; /**< Average SC/sec over last hour   */
    ZeroArray(UINT16, rid);     /**< RAID ID list                           */
} MRGETVINFO_RSP;
/* @} */

#define MRGETEXTEND_VINFO_REQ           MRGETVINFO_REQ

typedef struct MREXTENDED_VINFO_RSP_PKT
{
    UINT32 breakTime;
    UINT32 resvd[5];
} MREXTENDED_VINFO_RSP_PKT;

typedef struct MREXTEXTEND_VINFO_RSP
{
    MR_HDR_RSP               header;     /**< MRP Response Header - 8 bytes          */
    MREXTENDED_VINFO_RSP_PKT data;
} MREXTEXTEND_VINFO_RSP;

/**
**  @addtogroup MRGETRINFO
**  Get RAID information
**  @{
**/
/** Get RAID Information Extensions                                         */
typedef struct MRGETRINFO_RSP_EXT
{
    UINT16  pid;            /**< PID                                        */
    UINT8   pidstat;        /**< PID status                                 */
    UINT8   rpc;            /**< Rebuild percent complete                   */
    UINT8   pidastat;       /**< PID astatus                                */
    UINT8   rsvd2[3];       /**< RESERVED                                   */
} MRGETRINFO_RSP_EXT;

/** Get RAID information - Request **/
#define MRGETRINFO_REQ      MR_DEVID_REQ

/**
**  @todo Convert the MRGETRINFO_RSP structure to use the RDD structure
**        when the appropriate code changes have been made.
**        NOTE: The RDD structure contains additional fields compared to
**        the MRP response structure.
**/
/** Get RAID information - Response **/
typedef struct MRGETRINFO_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT16      rid;        /**< RAID ID                                    */
    UINT8       type;       /**< RAID type                                  */
    UINT8       status;     /**< Device status                              */
    UINT8       depth;      /**< Mirror depth/stripe width
                             **     For RAID 1/10 it contains
                             **     the mirror depth.
                             **     \n
                             **     For RAID 5 it contains the
                             **     stripe width of 3, 5 or 9.
                             **                                             */
    UINT8       pctRem;     /**< Initialize percent remaining               */
    UINT16      psdCnt;     /**< Count of PSD entries for this device       */
    UINT32      sps;        /**< Sectors per stripe for RAID 0, 5, 10       */
    UINT32      spu;        /**< Sectors per unit
                             **     For RAID 10 it contains
                             **     (psdcnt*sps).
                             **     \n
                             **     For RAID 5 it contains
                             **     (depth-1)*sps.
                             **                                             */
    UINT64      devCap;     /**< Device capacity                            */
    struct RDD *pNRDD;      /**< Next RDD in this VDisk                     */
    UINT16      vid;        /**< Virtual ID of owner                        */
    UINT16      frCnt;      /**< Failed/rebuild count                       */
    UINT32      error;      /**< Error count                                */
    UINT32      qd;         /**< Queue depth                                */
    UINT32      rps;        /**< Avg req/sec (last second)                  */
    UINT32      avgSC;      /**< Avg sector count (last second)             */
    UINT64      rReq;       /**< Read request count                         */
    UINT64      wReq;       /**< Write request count                        */
    UINT64      llsda;      /**< Locked LSDA (RAID 1,10)                    */
    UINT64      lleda;      /**< Locked LEDA (RAID 1,10)                    */
    UINT32      iprocs;     /**< Init RAID processes                        */
    UINT32      ierrors;    /**< Init RAID errors                           */
    UINT64      isectors;   /**< Init RAID sectors                          */
    UINT32      misComp;    /**< Parity errors in this RAID                 */
    UINT32      pardrv;     /**< Parity checks with missing drive           */
    UINT16      defLock;    /**< Defragmentation lock count                 */
    UINT8       aStatus;    /**< Additional status                          */
    UINT8       r5SROut;    /**< RAID 5 Stripe Resync Outstanding Count     */
    UINT32      notMirroringCSN; /**< Controller SN of RAID Not Mirroring Info*/
    UINT32      sprCnt;     /**< Sample period request count                */
    UINT32      spsCnt;     /**< Sample period sector  count                */
    void       *pRPNHead;   /**< RPN thread head (RAID 5)                   */
    ZeroArray(MRGETRINFO_RSP_EXT, psdExt); /**< PSD Extentions - 8 bytes    */
} MRGETRINFO_RSP;
/* @} */

/**
**  @addtogroup MRGETPINFO
**  Get physical device information
**  @{
**/
/**
**  @name MRGETPINFO Option Definitions
**  @{
**/
#define MIP_BIT_REFRESH 1       /**< Refresh device status (physical devs)  */
#define MIP_BIT_RLSELS  2       /**< Do a RLS ELS before returning data     */
/* @} */

/** Get physical device information - Request **/
typedef struct MRGETPINFO_REQ
{
    UINT16  id;             /**< PID or 0xFFFF for all                      */
    UINT8   options;        /**< Options for the request                    */
    UINT8   rsvd3[1];       /**< RESERVED                                   */
} MRGETPINFO_REQ;

/** Get physical device information - Response **/
#define MRGETPINFO_RSP      MR_GET_PE_INFO_RSP
/* @} */

/**
**  @addtogroup MRMAPLUN
**  Map a LUN to a VDisk
**  @{
**/
/**
**  @name Map LUN to VDisk Options
**  @{
**/
#define MMLMAP      0x0000  /**< Regular map - nuthin fancy                 */
#define MMLSWAP     0x0001  /**< SID to DSID swap of mappings               */
#define MMLCOPY     0x0002  /**< SID to DSID copy (DSID mapping remain)     */
#define MMLMOVE     0x0003  /**< SID to DSID copy (DSID mapping deleted)    */
#define MMLLINK     0x0004  /**< SID to DSID link (DSID mapping deleted)    */
/* @} */

/** Map a LUN to a VDisk - Request **/
typedef struct MRMAPLUN_REQ
{
    UINT16  sid;            /**< Server ID                                  */
    UINT16  option;         /**< Options                                    */
    UINT16  lun;            /**< LUN being mapped                           */
    UINT16  dsid;           /**< Destination SID                            */
    UINT16  vid;            /**< Virtual disk ID being mapped               */
    UINT16  rsvd10;         /**< RESERVED                                   */
} MRMAPLUN_REQ;

/** Map a LUN to a VDisk - Response **/
#define MRMAPLUN_RSP        MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRUNMAPLUN
**  Unmap a LUN from a VDisk
**  @{
**/
/** Unmap a LUN from a VDisk - Request **/
typedef struct MRUNMAPLUN_REQ
{
    UINT16  sid;            /**< Server ID                                  */
    UINT8   rsvd2[2];       /**< RESERVED                                   */
    UINT16  lun;            /**< LUN being mapped                           */
    UINT16  vid;            /**< Virtual disk ID being unmapped             */
} MRUNMAPLUN_REQ;

/** Unmap a LUN from a VDisk - Response **/
#define MRUNMAPLUN_RSP      MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRGETEINFO
**  Get SES device information
**  @{
**/
/** Get SES device information - Request **/
#define MRGETEINFO_REQ      MR_DEVID_REQ

/** Get SES device information - Response **/
#define MRGETEINFO_RSP      MR_GET_PE_INFO_RSP
/* @} */

/**
**  @addtogroup MRCREATESERVER
**  Create a server
**  @{
**/
/** Create a server - Request **/
typedef struct MRCREATESERVER_REQ
{
    UINT16  targetId;       /**< Target ID                                  */
    UINT8   rsvd2[2];       /**< RESERVED                                   */
    UINT32  owner;          /**< Owner                                      */
    UINT64  wwn;            /**< World wide name                            */
    UINT8   i_name[256];    /**< iSCSI Server name                          */
} MRCREATESERVER_REQ;

/** Create a server - Response **/
typedef struct MRCREATESERVER_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT16      sid;        /**< Server ID created                          */
    UINT16      flags;      /**< Server Creation Flags
                             **     - TRUE (1) if the server already existed
                             **     - FALSE (0) if the server is new
                             **                                             */
} MRCREATESERVER_RSP;
/* @} */

/**
**  @addtogroup MRDELETESERVER
**  Delete a server
**  @{
**/
#define DS_DELETE      0    /**< delete server                              */
#define DS_MAPPINGS    1    /**< delete mapping (including mapping on linked servers) */
#define DS_DELETEALL   2    /**< delete server and all linked servers       */
#define DS_LINKS       3    /**< Remove server from linked server list      */

/** Delete a server - Request **/
#define MRDELETESERVER_REQ  MR_DEVID_REQ

/** Delete a server - Response **/
#define MRDELETESERVER_RSP  MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRGETMINFO
**  Get misc device info
**  @{
**/
/** Get misc device info - Request **/
#define MRGETMINFO_REQ      MR_DEVID_REQ

/** Get misc device info - Response **/
typedef struct MRGETMINFO_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    PDD         pdd;        /**< Physical Device Descriptor                 */
} MRGETMINFO_RSP;
/* @} */

/**
**  @addtogroup MRVDISKCONTROL
**  Virtual disk control
**  @{
**/
/**
**  @name Subtype operations
**  @{
**/
#define MMLMAP      0x0000  /**< Regular map - nuthin fancy                 */

#define MVCMOVEVD       0x00    /**< Move virtual device                    */
#define MVCCOPYBRK      0x01    /**< Start copy and break                   */
#define MVCCOPYSWAP     0x02    /**< Start copy and swap                    */
#define MVCCOPYCONT     0x03    /**< Start continuous copy                  */
#define MVCSWAPVD       0x04    /**< Test function (swap vdisks)            */
#define MVCXSPECCOPY    0x05    /**< Break off specified copy               */
#define MVCPAUSECOPY    0x06    /**< Pause a copy                           */
#define MVCRESUMECOPY   0x07    /**< Resume a copy                          */
#define MVCABORTCOPY    0x08    /**< Abort a copy                           */
#define MVCUPDATE       0x09    /**< Update the status                      */
#define MVCSLINK        0x0A    /**< Create a Snapshot                      */
#define MVCXCOPY        0x0E    /**< Break off all copies                   */
/* @} */

/** Virtual disk control - Request **/
typedef struct MRVDISKCONTROL_REQ
{
    UINT8   subtype;        /**< Operation to be completed                  */
    UINT8   rsvd1[3];       /**< RESERVED                                   */
    UINT16  svid;           /**< Source VID                                 */
    UINT16  rsvd2;          /**< RESERVED                                   */
    UINT16  dvid;           /**< Destination VID                            */
    UINT16  rsvd10;         /**< RESERVED                                   */
} MRVDISKCONTROL_REQ;

#if 0
/** Virtual disk control - Response **/
#define MRVDISKCONTROL_RSP  MR_GENERIC_RSP
#else /* GEORAID */
/** Set Geo location  - Response **/
typedef struct MRVDISKCONTROL_RSP
{
    MR_HDR_RSP  header;      /**< MRP Response Header - 8 bytes              */
    UINT8       mirrorSetType; /**< Partner Type                               */
    UINT8       rsvd[3];     /**< Reserved for Quad Boundary                 */
} MRVDISKCONTROL_RSP;
#endif /* GEORAID */
/* @} */

/**
**  @addtogroup MRASSIGNSYSINFO
**  Assign system information
**  @{
**/

/**
**  @name MRASSIGNSYSINFO operations
**  @{
**/
#define ASSIGNSYSINFO_POLL      0x00
#define ASSIGNSYSINFO_SERIAL    0x01
#define ASSIGNSYSINFO_IPADDR    0x02
#define ASSIGNSYSINFO_CLEARMP   0x04
#define ASSIGNSYSINFO_CNTL_SN   0x08   /* set controller S/N            */
/* @} */

/** Assign system information - Request **/
typedef struct MRASSIGNSYSINFO_REQ
{
    UINT8   op;             /**< Operation to be completed                  */
    UINT8   rsvd1[3];       /**< RESERVED                                   */
    UINT32  sserial;        /**< System Serial Number                       */
    UINT32  ipaddr;         /**< IP Address                                 */
} MRASSIGNSYSINFO_REQ;

/** Assign system information - Response **/
typedef struct MRASSIGNSYSINFO_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT32      sserial;    /**< System serial number (set or polled)       */
    UINT32      cserial;    /**< Controller serial number (polled)          */
    UINT32      ipaddr;     /**< IP Address                                 */
    UINT32      mp;         /**< Mirror Partner                             */
} MRASSIGNSYSINFO_RSP;
/* @} */

/**
**  @addtogroup MRBEII
**  Get back end II information
**  @{
**/
/**  Get back end II information - Request **/
#define MRBEII_REQ          void

/**  Get back end II information - Response **/
typedef struct MRBEII_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    II          ii;         /**< Internal Information                       */
} MRBEII_RSP;
/* @} */

/**
**  @addtogroup MRBELINK
**  Get back end link information
**  @{
**/
/** Get back end link information - Request **/
#define MRBELINK_REQ        void

/** Get back end link information - Response **/
#define MRBELINK_RSP        MR_LINK_RSP
/* @} */

/**
**  @addtogroup MRBEBOOT
**  Get back end boot code header
**  @{
**/
/** Get back end boot code header - Request **/
#define MRBEBOOT_REQ        void

/** Get back end boot code header - Response **/
#define MRBEBOOT_RSP        MR_FW_HEADER_RSP
/* @} */

/**
**  @addtogroup MRBEDIAG
**  Get back end diagnostic code header
**  @{
**/
/** Get back end diagnostic code header - Request **/
#define MRBEDIAG_REQ        void

/** Get back end diagnostic code header - Response **/
#define MRBEDIAG_RSP        MR_FW_HEADER_RSP
/* @} */

/**
**  @addtogroup MRBEPROC
**  Get back end proc code header
**  @{
**/
/** Get back end proc code header - Request **/
#define MRBEPROC_REQ        void

/** Get back end proc code header - Response **/
#define MRBEPROC_RSP        MR_FW_HEADER_RSP
/* @} */

/**
**  @addtogroup MRBRWMEM
**  R/W back end memory
**  @{
**/
/** R/W back end memory - Request **/
#define MRBRWMEM_REQ        MR_RW_MEMORY_REQ

/** R/W back end memory - Response **/
#define MRBRWMEM_RSP        MR_RW_MEMORY_RSP
/* @} */

/**
**  @addtogroup MRCONFIGTARG
**  Configure target
**  @{
**/

/**
**  @name Configure Target Options
**  @{
**/
#define CFGTARG_HARD_ID         0x01
#define CFGTARG_SOFT_ID         0x00
#define CFGTARG_ADD_ID          0xFFFF

#define CFGTARG_MOD_ALL         0x0000  /**< Modify all fields              */
#define CFGTARG_MOD_PORT        0x0001  /**< Modify port                    */
#define CFGTARG_MOD_OPTION      0x0002  /**< Modify hard/soft option        */
#define CFGTARG_MOD_LOCK        0x0004  /**< Modify lock flag               */
#define CFGTARG_MOD_OWNER       0x0008  /**< Modify controller (owner)      */
#define CFGTARG_MOD_CLUSTER     0x0010  /**< Modify cluster                 */
/* @} */

/** Configure target - Request **/
typedef struct MRCONFIGTARG_REQ
{
    UINT16      tid;        /**< Target ID                                  */
    UINT8       port;       /**< port number                                */
    UINT8       opt;        /**< Options hard or soft ID, active or inact   */
    UINT8       fcid;       /**< Fibre Channel ID                           */
    UINT8       rsvd13;     /**< RESERVED                                   */
    UINT8       lock;       /**< Target lock bits                           */
#if ISCSI_CODE
    UINT8       ipPrefix;   /**< IP prefix for a classless IP address       */
#else
    UINT8       rsvd15;     /**< RESERVED                                   */
#endif
    /* QUAD */
    union {
    UINT64      pname;      /**< Port name                                  */
#if ISCSI_CODE
        struct {
            UINT32  ipAddr; /**< Target IP Address                          */
            UINT32  ipGw;   /**< Default Gateway IP Address                 */
            };
#endif
    };
    UINT64      nname;      /**< Node name                                  */
    /* QUAD */
    UINT32      powner;     /**< Previous owning controller serial number   */
    UINT32      owner;      /**< Owning controller serial number            */
    UINT16      cluster;    /**< Clustered target ID                        */
    UINT8       rsvd42[2];  /**< RESERVED                                   */
    UINT8       pport;      /**< Primary port                               */
    UINT8       aport;      /**< Alternate port                             */
    UINT8       rsvd46[2];  /**< RESERVED                                   */
    /* QUAD */
    UINT8       rsvd48[4];  /**< RESERVED                                   */
    UINT32      modMask;    /**< Define which fields to modify 0 = all      */
} MRCONFIGTARG_REQ;

/** Configure target - Response **/
typedef struct MRCONFIGTARG_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MRCONFIGTARG_RSP;
/* @} */

/**
**  @addtogroup MRGETMPLIST
**  Get Mirror partner list
**  @{
**/
/**
**  Get List of Mirror Partners Information (used in response)
**/
typedef struct MRGETMPLIST_RSP_INFO
{
    UINT32  source;         /**< Source mirror partner                      */
    UINT32  dest;           /**< Destination mirror partner                 */
} MRGETMPLIST_RSP_INFO;

/** Get Mirror partner list - Request **/
#define MRGETMPLIST_REQ     MR_DEVID_REQ

/** Get Mirror partner list - Response **/
typedef struct MRGETMPLIST_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT16      count;      /**< Number of devices in the dynamic list      */
    UINT8       rsvd10[2];  /**< RESERVED                                   */
    ZeroArray(MRGETMPLIST_RSP_INFO, list); /* Mirror partner list           */
} MRGETMPLIST_RSP;
/* @} */

/**
**  @addtogroup MRGLOBALPRI
**  Set background priority
**  @{
**/
/**
**  @name Global Priority Definitions
**  @{
**/
#define MBPMAXPRI           7   /**< Maximum value allowed                  */
/* @} */

/** Set background priority - Request **/
typedef struct MRGLOBALPRI_REQ
{
    UINT8   gpri;           /**< Global priority value                      */
    UINT8   rsvd1[3];       /**< RESERVED                                   */
} MRGLOBALPRI_REQ;

/** Set background priority - Response **/
#define MRGLOBALPRI_RSP     MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRGETTLIST
**  Get target list
**  @{
**/
/** Get target list - Request **/
#define MRGETTLIST_REQ      MR_DEVID_REQ

/** Get target list - Response **/
#define MRGETTLIST_RSP      MR_LIST_RSP
/* @} */

/**
**  @addtogroup MRRESETBEPORT
**  Reset BE Port
**  @{
**/
/** Reset BE port - Request **/
#define MRRESETBEPORT_REQ   MRRESETPORT_REQ

/** Reset BE port - Response **/
#define MRRESETBEPORT_RSP   MRRESETPORT_RSP
/* @} */

/**
**  @addtogroup MRNAMECHANGE
**  Virtual disk or controller name changed
**  @{
**/
/** Virtual disk or controller name changed - Request **/
#define MRNAMECHANGE_REQ    MR_DEVID_REQ

/** Virtual disk or controller name changed - Response **/
#define MRNAMECHANGE_RSP    MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRREMOTECTRLCNT
**  Get VLink Remote Controller Count
**  @{
**/
/** Get VLink Remote Controller Count - Request **/
#define MRREMOTECTRLCNT_REQ     void

/** Get VLink Remote Controller Count - Response **/
typedef struct MRREMOTECTRLCNT_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT16      count;      /**< Controller Count                           */
    UINT8       rsvd10[2];  /**< RESERVED                                   */
} MRREMOTECTRLCNT_RSP;
/* @} */

/**
**  @addtogroup MRREMOTECTRLINFO
**  Get VLink Remote Controller Information
**  @{
**/
/** Get VLink Remote Controller Information - Request **/
#define MRREMOTECTRLINFO_REQ    MR_DEVID_REQ

/** Get VLink Remote Controller Information - Response **/
typedef struct MRREMOTECTRLINFO_RSP
{
    MR_HDR_RSP  header;         /**< MRP Response Header - 8 bytes         */
    UINT64      wwn;            /**< World Wide Name                       */
    UINT8       ctrlName[20];   /**< Controller (storage unit) name        */
    UINT8       luns;           /**< Number of LUNs on this controller     */
    UINT8       scType;         /**< Storage Controller Type               */
    UINT8       cluster;        /**< Magnitude Cluster Number              */
    UINT8       rsvd2;          /**< RESERVED                              */
    UINT32      ipAddr;         /**< IP Address                            */
    UINT32      serialNum;      /**< Controller serial number              */
} MRREMOTECTRLINFO_RSP;
/* @} */

/**
**  @addtogroup MRREMOTEVDISKINFO
**  Get remote virtual disk information
**  @{
**/
/** Get VLink Remote Virtual Disk Information - Request **/
#define MRREMOTEVDISKINFO_REQ   MR_DEVID_REQ

/* VLink Remote Virtual Disk Information */
typedef struct RMT_VDISK_INFO
{
    UINT16  lun;        /**< LUNumber of VDisk on Remote Controller         */
    UINT8   rtype;      /**< RAID type of first RAID on VDisk               */
    UINT8   cluster;    /**< Cluster on Remote Controller                   */
    UINT8   attr;       /**< Attribute on Remote Controller                 */
    UINT8   rsvd1[3];   /**< RESERVED                                       */
    UINT64  devCap;     /**< Device capacity in sectors                     */
    UINT32  sserial;    /**< System serial number of Remote Controller      */
    UINT16  vid1;       /**< Virtual Disk ID on Remote Controller           */
    UINT16  vid2;       /**< Virtual Disk ID on Remote Controller           */
    UINT16  scnt;       /**< Server count on Remote Controller              */
    UINT16  vlCnt;      /**< Virtual link count on Remote Controller        */
    UINT8   rsvd2[4];   /**< RESERVED                                       */
    UINT8   vdName[52]; /**< Name on Remote Controller                      */
    UINT8   rsvd3[12];  /**< RESERVED                                       */
}RMT_VDISK_INFO;

/** Get VLink Remote Virtual Disk Information - Response **/
typedef struct MRREMOTEVDISKINFO_RSP
{
//  NOTE: MR_HDR_RSP => 3 reserved bytes, then byte of status, then UINT32 len.
    UINT8           count;
    UINT8           rsvd1[2];   /**< RESERVED                               */
    UINT8           status;     /**< Status                                 */
    UINT32          len;        /**< Length                                 */
// NOTE: MAX FE LUNS change needed below.
    RMT_VDISK_INFO  data[64];   /**< Record of each VDisk available         */
} MRREMOTEVDISKINFO_RSP;
/* @} */

/**
**  @addtogroup MRFOREIGNTARGETS
**  Set foreign targets
**  @{
**/
/** Set foreign targets - Request **/
typedef struct MRFOREIGNTARGETS_REQ
{
    UINT8   bmap;           /**< Bitmap for the ENABLE of each Controller   */
    UINT8   rsvd1[3];       /**< RESERVED                                   */
} MRFOREIGNTARGETS_REQ;

/** Set foreign targets - Response **/
#define MRFOREIGNTARGETS_RSP    MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRCREATEVLINK
**  Create a vlink
**  @{
**/
/** Create a vlink - Request **/
typedef struct MRCREATEVLINK_REQ
{
    UINT16  ctrlIndex;  /**< Controller Index - from Get Remote Controller  */
    UINT8   vdiskOrd;   /**< Virtual Disk Ordinal - from Get VLink Info     */
    UINT8   rsvd3;      /* RESERVED                                         */
    UINT16  vid;        /**< VID to use for VLink VDD                       */
    UINT8   rsvd6[2];   /* RESERVED                                         */
} MRCREATEVLINK_REQ;

/** Create a vlink - Response **/
typedef struct MRCREATEVLINK_RSP
{
    MR_HDR_RSP  header;         /**< MRP Response Header - 8 bytes          */
    UINT16      vid;            /**< Virtual Link ID                        */
    UINT8       rsvd[2];        /*   RESERVED                               */
    UINT8       ctrlName[20];   /* Remote Controller Name                   */
    UINT8       vdName[52];     /* Remote Virtual Disk Name                 */
} MRCREATEVLINK_RSP;
/* @} */

/**
**  @addtogroup MRVLINKINFO
**  Get virtual link disk information
**  @{
**/
/** Get virtual link disk information - Request **/
#define MRVLINKINFO_REQ     MR_DEVID_REQ

/** Get virtual link disk information - Response **/
typedef struct MRVLINKINFO_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    /* VDD - 6 Quads */
    UINT16  vdVid;          /* Virtual device ID                            */
    UINT8   vdAttr;         /* Attribute                                    */
    UINT8   vdDevStat;      /* Device status                                */
    UINT16  vdScorVid;      /* Second copy original VID                     */
    UINT8   vdScpComp;      /* Second copy percent complete                 */
    UINT8   vdRaidCnt;      /* Number of RAIDs in this device               */
    UINT64  vdDevCap;       /* Device capacity                              */
                            /* QUAD                                         */
    UINT32  vdError;        /* # errors this device since last power up     */
    UINT32  vdQd;           /* Curent queue depth                           */
    UINT32  vdRps;          /* Avg requests/sec (last second)               */
    UINT32  vdAvgSc;        /* Avg sector count (last second)               */
                            /* QUAD                                         */
    UINT64  vdRreq;         /* Read request count                           */
    UINT64  vdWreq;         /* Write request count                          */
                            /* QUAD                                         */
    UINT8   vdCacheEn;      /* Cache enabled (TRUE/FALSE)                   */
    UINT8   vdMirror;       /* Mirror status                                */
    UINT16  vdRsvdS1;       /* RESERVED                                     */
    UINT32  vdRsvdL1;       /* RESERVED                                     */
    UINT32  vdSprc;         /* Sample period request count                  */
    UINT32  vdSpsc;         /* Sample period sector count                   */
                            /* QUAD                                         */
    UINT32  vdScHead;       /* Original vdscmt list head pointer            */
    UINT32  vdScTail;       /* Original vdscmt list tail pointer            */
    UINT32  vdCpscmt;       /* Copy vd scmt pointer                         */
    UINT32  vdVlinks;       /* VLink Association Records pointer            */
                            /* QUAD                                         */
    UINT8   vdName[16];     /* VDisk name                                   */

    /* RDD - 8 Quads */
    UINT16  rdRid;          /* RAID ID                                      */
    UINT8   rdType;         /* RAID level                                   */
    UINT8   rdDevStatus;    /* Device status                                */
    UINT8   rdDepth;        /* Mirror depth / stripe width                  */
    UINT8   rdPctRem;       /* % storage remaining to be initialized        */
    UINT16  rdPsdCnt;       /* # PSD entries used                           */
    UINT32  rdSps;          /* Sectors/stripe on a single device            */
    UINT32  rdSpu;          /* Sectors per unit                             */
                            /* QUAD                                         */
    UINT64  rdDevCap;       /* Device capacity                              */
    UINT32  rdNvrdd;        /* Next RAID in this VDisk                      */
    UINT16  rdVid;          /* Virtual Dev ID associated w/ this RAID       */
    UINT16  rdFrCnt;        /* # of rebuilding or failed devices            */
                            /* QUAD                                         */
    UINT32  rdError;        /* Error count                                  */
    UINT32  rdQd;           /* Queue depth                                  */
    UINT32  rdRps;          /* Avg requests/sec (last second)               */
    UINT32  rdAvgsc;        /* Avg sector count (last second)               */
                            /* QUAD                                         */
    UINT64  rdRreq;         /* Read request count                           */
    UINT64  rdWreq;         /* Write request count                          */
                            /* QUAD                                         */
    UINT64  rdLlsda;        /* Locked LSDA                                  */
    UINT64  rdLleda;        /* Locked LEDA                                  */
                            /* QUAD                                         */
    UINT32  rdIprocs;       /* # of initialization processes                */
    UINT32  rdIerrors;      /* # of initialization errors                   */
    UINT64  rdIsectors;     /* # of sectors initialized                     */
                            /* QUAD                                         */

    UINT32  rdMiscomp;      /* Parity errors in this raid                   */
    UINT32  rdParDrv;       /* Parity checks w/ missing drive               */

    UINT16  rdDefLock;      /* Defrag lock count                            */
    UINT8   rdAStatus;      /* ???                                          */
    UINT8   rdRsvd1[5];     /* Reserved - 5 bytes                           */
                            /* QUAD                                         */

    UINT32  rdSprc;         /* Sample period req count                      */
    UINT32  rdSpsc;         /* Sample period sec count                      */
    UINT32  rdRpnHead;      /* RPN thread head                              */
    UINT8   rsvd00[4];      /* Reserved                                     */
                            /* QUAD                                         */

    /* LDD - 7 Quads */
    UINT8   ldClass;        /* Device Class                                 */
    UINT8   ldState;        /* Device State                                 */
    UINT8   ldPmask;        /* Path Mask                                    */
    UINT8   ldPpri;         /* Path Priority                                */
    UINT32  ldOwner;        /* Owning controller                            */
    UINT64  ldDevCap;       /* Device capacity                              */
                            /* QUAD                                         */
    INT8    ldVendId[8];    /* Vendor ID                                    */
    INT8    ldRev[4];       /* Product Revision                             */
    UINT8   ldRsvd2[4];     /* RESERVED                                     */
                            /* QUAD                                         */
    UINT8   ldProdId[16];   /* Product ID                                   */
                            /* QUAD                                         */
    UINT8   ldSerial[12];   /* Product Serial Number                        */
    UINT8   ldRsvd3[4];     /* RESERVED                                     */
                            /* QUAD                                         */
    UINT16  ldLun;          /* Foreign Target LUN Number                    */
    UINT16  ldBaseVd;       /* Base Magnitude Virtual Disk Number           */
    UINT8   ldBaseCl;       /* Base Magnitude Cluster Number                */
    UINT8   ldRsvd4[3];     /* RESERVED                                     */
    UINT32  ldBaseSn;       /* Base Magnitude Serial Number                 */
    UINT8   ldRsvd5[4];     /* RESERVED                                     */
                            /* QUAD                                         */
    UINT8   ldBaseName[16]; /* Base Name                                    */
                            /* QUAD                                         */
    UINT64  ldBaseNode;     /* Base Magnitude Node Name (WWN)               */
    UINT8   ldRsvd6[8];     /* RESERVED                                     */
                            /* QUAD                                         */
} MRVLINKINFO_RSP;
/* @} */

/**
**  @addtogroup MRCREATECTRLR
**  Create dflt servers/targets for a ctrlr
**  @{
**/
/** Create dflt servers/targets for a ctrlr - Request **/
typedef struct MRCREATECTRLR_REQ
{
    UINT8   numControllers; /**< Number of Controllers to create            */
    UINT8   rsvd1[3];       /**< RESERVED                                   */
} MRCREATECTRLR_REQ;

/** Create dflt servers/targets for a ctrlr - Response **/
#define MRCREATECTRLR_RSP   MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRRESCANDEVICE
**  Rescan physical Devices
**  @{
**/
/**
**  @name Rescan Device Scan Types
**  @{
**/
#define RESCAN_EXISTING     0x00 /**< Rescan existing devices               */
#define RESCAN_LUNS         0x01 /**< Rescan LUNS                           */
#define RESCAN_REDISCOVER   0x02 /**< Rediscover devices                    */
/* @} */

/** Rescan physical Devices - Request **/
typedef struct MRRESCANDEVICE_REQ
{
    UINT8   scanType;       /**< Rescan type to execute                     */
    UINT8   rsvd1[3];       /**< RESERVED                                   */
} MRRESCANDEVICE_REQ;

/** Rescan physical Devices - Response **/
#define MRRESCANDEVICE_RSP  MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRRESYNC
**  Resync RAIDs or stripes
**  @{
**/
/**
**  @name Resync Types
**  @{
**/
#define MRBSTRIPE           1   /**< Sync stripes given NVA table           */
#define MRBONERAID          2   /**< Sync given RAID ID                     */
#define MRBALLRAIDS         3   /**< Sync all RAIDs on this controller      */
#define MRBALLNOTMIRROR     4   /**< Sync all RAIDs with "Not Mirroring" on
                                     a different controller but now owned by
                                     this controller                        */
#define MRBLISTRAIDS        5   /**< Sync all RAIDs in the given list       */
/* @} */

/** Resync RAIDs or stripes - Request **/
typedef struct MRRESYNC_REQ
{
    UINT8   type;           /**< Type of Resync                             */
    UINT8   rsvd1;          /**< RESERVED                                   */
    union
    {
        UINT16  rid;        /**< RAID ID                                    */
        UINT16  numRids;    /**< Number of RIDs in the following list       */
    } r;
    union
    {
        void   *pNVA;       /**< Address of Part 4 NVA buffer               */
        UINT16 *pRIDList;   /**< Address of the list of RIDs to Resync      */
    } p;
} MRRESYNC_REQ;

/** Resync RAIDs or stripes - Response **/
#define MRRESYNC_RSP        MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRGETLCLIMAGE
**  Get local NVRAM image
**  @{
**/
/** Get local NVRAM image - Request **/
typedef struct MRGETLCLIMAGE_REQ
{
    UINT32  length;         /**< Size of the buffer allocated below         */
    void   *address;        /**< PCI address of memory for local image      */
} MRGETLCLIMAGE_REQ;

/** Get local NVRAM image - Response **/
typedef struct MRGETLCLIMAGE_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT32      imageSize;  /**< Size of the image                          */
} MRGETLCLIMAGE_RSP;
/* @} */

/**
**  @addtogroup MRPUTLCLIMAGE
**  Put local NVRAM image
**  @{
**/
/** Put local NVRAM image - Request **/
typedef struct MRPUTLCLIMAGE_REQ
{
    void *address;          /**< PCI address of the image to be updated     */
} MRPUTLCLIMAGE_REQ;

/** Put local NVRAM image - Response **/
#define MRPUTLCLIMAGE_RSP   MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRDELETEDEVICE
**  Delete SES or disk drive device
**  @{
**/
/**
**  @name Delete Device Delete Types
**  @{
**/
#define DELETE_DEVICE_DRIVE 0x00 /**< Delete a physical disk                */
#define DELETE_DEVICE_BAY   0x01 /**< Delete a drive bay                    */
/* @} */

/** Delete SES or disk drive device - Request **/
typedef struct MRDELETEDEVICE_REQ
{
    UINT8   type;           /**< type of device to delete                   */
    UINT8   rsvd1;          /**< RESERVED                                   */
    UINT16  did;            /**< device id                                  */
} MRDELETEDEVICE_REQ;

/** Delete SES or disk drive device - Response **/
#define MRDELETEDEVICE_RSP  MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRBEMODEPAGE
**  Mode page
**  @{
**/
/** Mode page - Request **/
#define MRBEMODEPAGE_REQ    MR_MODE_SET_REQ

/** Mode page - Response **/
#define MRBEMODEPAGE_RSP    MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRDEVICECOUNT
**  Device count for a specific serial num
**  @{
**/
/** Device count for a specific serial num - Request **/
typedef struct MRDEVICECOUNT_REQ
{
    UINT32  serialNumber;   /**< Search serial number                       */
} MRDEVICECOUNT_REQ;

/** Device count for a specific serial num - Response **/
typedef struct MRDEVICECOUNT_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT32      count;      /**< Count of the devices                       */
} MRDEVICECOUNT_RSP;
/* @} */

/**
**  @addtogroup MRGETVIDOWNER
**  Get Virtual Disk Owner
**  @{
**/
/** Virtual disk owner information (used in response packet) */
typedef struct MRGETVIDOWNER_RSP_INFO
{
    UINT16  tid;            /**< Target ID of owner                         */
    UINT16  port;           /**< Port of owner                              */
    UINT16  sid;            /**< Server ID of owner                         */
    UINT16  lun;            /**< LUN assigned to VDISK on this server       */
} MRGETVIDOWNER_RSP_INFO;

/** Get Virtual Disk Owner - Request **/
#define MRGETVIDOWNER_REQ   MR_DEVID_REQ

/** Get Virtual Disk Owner - Response **/
typedef struct MRGETVIDOWNER_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT16      ndevs;      /**< Number of virtual disk owners in the list  */
    UINT8       rsvd10[2];  /**< RESERVED                                   */
    ZeroArray(MRGETVIDOWNER_RSP_INFO, list); /**< VDisk owner information   */
} MRGETVIDOWNER_RSP;
/* @} */

/**
**  @addtogroup MRVDISKREDUNDANCY
**  Get Virtual Disk Redundancy
**  @{
**/
/** Virtual disk Redundancy Request */
typedef struct MRVIRTREDUNDANCY_REQ
{
    UINT16  vid;            /**< VDISK ID                                   */
    UINT8   rsvd1[2];       /**< Reserved                                   */
} MRVIRTREDUNDANCY_REQ;

/** Virtual Disk Redundancy - Response **/
#define VDISK_REDUNDANT     0x00
#define VDISK_NOT_REDUNDANT 0x01

typedef struct MRVIRTREDUNDANCY_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes             */
    UINT32      status;     /**< Status of the redundancy                  */
                            /**< status = VDISK_REDUNDANT = 0x00           */
                            /**<        = VDISK_NOT_REDUNDANT = 0x01       */
} MRVIRTREDUNDANCY_RSP;
/* @} */

/**
**  @addtogroup MRHOTSPAREINFO
**  Get hot spare info for pid
**  @{
**/
/** Get hot spare info for pid - Request **/
typedef struct MRHOTSPAREINFO_REQ
{
    UINT16       pid;       /**< Pid                                        */
} MRHOTSPAREINFO_REQ;

/** Get hot spare info for pid - Response **/
typedef struct MRHOTSPAREINFO_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT64      capacity;   /**< Capacity of spare required                 */
} MRHOTSPAREINFO_RSP;
/* @} */

/**
**  @addtogroup MRFILECOPY
**  File system file to file copy
**  @{
**/
/** File system file to file copy - Request **/
typedef struct MRFILECOPY_REQ
{
    UINT16      srcFID;     /**< Source FID number                          */
    UINT16      srcOffset;  /**< Offset into source FID to read from        */
    UINT16      length;     /**< Number of blocks to copy                   */
    UINT16      destFID;    /**< Destination FID number                     */
    UINT16      destOffset; /**< Offset into destination FID to write to    */
    UINT8       rsvd10[2];  /**< RESERVED                                   */
} MRFILECOPY_REQ;

/** File system file to file copy - Response **/
#define MRFILECOPY_RSP      MR_FSYS_OP_RSP
/* @} */

/**
**  @addtogroup MRBEGETDVLIST
**  Get back end device information
**  @{
**/
/** Get back end device information - Request **/
#define MRBEGETDVLIST_REQ   MRGETDVLIST_REQ

/** Get back end device information - Response **/
#define MRBEGETDVLIST_RSP   MRGETDVLIST_RSP
/* @} */

/**
**  @addtogroup MRBEGETPORTLIST
**  Get BE Port List
**  @{
**/
/** Get BE Port List - Request **/
#define MRBEGETPORTLIST_REQ     MRGETPORTLIST_REQ

/** Get BE Port List - Response **/
#define MRBEGETPORTLIST_RSP     MR_LIST_RSP
/* @} */

/**
**  @addtogroup MRBREAKVLOCK
**  Break vlink lock
**  @{
**/
/** Break vlink lock - Request **/
#define MRBREAKVLOCK_REQ    MR_DEVID_REQ

/** Break vlink lock - Response **/
#define MRBREAKVLOCK_RSP    MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRGETSOS
**  Get a SOS entry from the BEP
**  @{
**/
/** SOS Table Entry (used in response packet) **/
typedef struct MRGETSOS_RSP_ENTRY
{
    UINT32      gap;        /**< Size between last segment and this one     */
    UINT32      sda;        /**< Starting disk address                      */
    UINT32      segLen;     /**< Length of segment                          */
    UINT32      rid;        /**< Raid id of segment                         */
} MRGETSOS_RSP_ENTRY;

/** Get a SOS entry from the BEP - Request **/
#define MRGETSOS_REQ        MR_DEVID_REQ

/** Get a SOS entry from the BEP - Response **/
typedef struct MRGETSOS_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT8       rsvd8[8];   /**< RESERVED                                   */
    SOS         sos;        /**< Segment Optimization Structure             */
    ZeroArray(MRGETSOS_RSP_ENTRY, list);
} MRGETSOS_RSP;
/* @} */

/**
**  @addtogroup MRPUTSOS
**  Put a SOS entry to the BEP
**  @{
**/
/** Put a SOS entry to the BEP - Request **/
typedef struct MRPUTSOS_REQ
{
    UINT16      pid;            /**< PID for drive being defragmented       */
    UINT16      flags;          /**< Flags for defragmention progress       */
    UINT32      remain;         /**< Blocks remaining in defragmentation    */
    UINT32      total;          /**< Total blocks in defragmentation        */
                                /**< QUAD BOUNDARY                      *****/
    UINT16      count;          /**< Count of entries in SOS table          */
    UINT16      current;        /**< Current entry in SOS table             */
    UINT32      asda;           /**< Alternate starting disk address        */
    UINT32      serial;         /**< For prepare, who should do operation   */
} MRPUTSOS_REQ;

/** Put a SOS entry to the BEP - Response **/
#define MRPUTSOS_RSP        MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRFORCEBEETRAP
**  Force a BE error trap
**  @{
**/
/** Force a BE error trap - Request **/
#define MRFORCEBEETRAP_REQ  void

/** Force a BE error trap - Response **/
#define MRFORCEBEETRAP_RSP  MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRPUTSCMT
**  Update a SCMT structure
**  @{
**/
/** Update a SCMT structure - Request **/
typedef struct MRPUTSCMT_REQ
{
    UINT8       tempPlaceHolder;    /**< Temporary place holder             */
} MRPUTSCMT_REQ;

/** Update a SCMT structure - Response **/
#define MRPUTSCMT_RSP           MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRBELOOPPRIMITIVE
**  Loop primitive
**  @{
**/
/** Loop primitive - Request **/
#define MRBELOOPPRIMITIVE_REQ   MRLOOPPRIMITIVE_REQ

/** Loop primitive - Response **/
#define MRBELOOPPRIMITIVE_RSP   MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRTARGETCONTROL
**  Target move control
**  The prepare to move option will stop all background operations on
**  the controller - initializations, scrubbing, parity/mirror scan,
**  rebuilds, defrags, and copies.
**
**  The move complete option causes the controller to restart the background
**  operations after targets are all moved and the RAIDs are owned by the
**  correct controller.
**  @{
**/
/**
**  @name Target Control Options
**  @{
**/
#define TC_PREP_MOVE        0 /**< Prepare to move targets                  */
#define TC_COMP_MOVE        1 /**< All target movements are complete        */
/* @} */

/** Target move control - Request **/
typedef struct MRTARGETCONTROL_REQ
{
    UINT16  option;         /**< Option                                     */
    UINT8   rsvd2[2];       /**< RESERVED                                   */
} MRTARGETCONTROL_REQ;

/** Target move control - Response **/
#define MRTARGETCONTROL_RSP     MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRFAILCTRL
**  Fail / Unfail controller
**  @{
**/
/* Option values for Fail / Unfail Controller */
#define FC_PORTS                0x0001  /**< Use qualifier to specify ports */
#define FC_FB                   0x0002  /**< Fail-Back option */

/** Fail / Unfail controller - Request **/
typedef struct MRFAILCTRL_REQ
{
    UINT16  option;         /**< Options                                    */
    UINT8   rsvd2[2];       /**< RESERVED                                   */
    UINT16  qualifier;      /**< Qualifier                                  */
    UINT8   rsvd6[2];       /**< RESERVED                                   */
    UINT32  oldOwner;       /**< Current Owner (CSN)                        */
    UINT32  newOwner;       /**< New Owner (CSN)                            */
} MRFAILCTRL_REQ;

/** Fail / Unfail controller - Response **/
#define MRFAILCTRL_RSP      MR_GENERIC_RSP
/* @} */

/** Set Foreign Target (on/off) - Request **/
typedef struct MRSFT_REQ
{
    UINT8   option;         /**< Value                                     */
} MRSFT_REQ;

/** Set Foreign Target (on/off) - Response **/
#define MRSFT_RSP           MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRNAMEDEVICE
**  Name a device
**  @{
**/
/**
**  @name Name Device Options
**  @{
**/
#define MNDSERVER           0x0000 /**< Set the server name                 */
#define MNDVDISK            0x0001 /**< Set the virtual disk name           */
#define MNDVCG              0x0002 /**< Set the vcg name                    */
#define MNDRETVCG           0x0003 /**< Retrieve the vcg name               */
/* @} */

/** Name a device - Request **/
typedef struct MRNAMEDEVICE_REQ
{
    UINT16  option;         /**< Option                                     */
    UINT16  id;             /**< Device identifier                          */
    UINT8   rsvd4[12];      /**< RESERVED                                   */
    UINT8   name[16];       /**< Name for the device            */
} MRNAMEDEVICE_REQ;

/** Name a device - Response **/
typedef struct MRNAMEDEVICE_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT8       name[16];   /**< Name for the device        */
} MRNAMEDEVICE_RSP;
/* @} */

/**
**  @addtogroup MRPUTDG
**  Update VLINK information
**  @{
**/
/** Update VLINK information - Request **/
/* vishu*/
/** Broadcast IPC message **********    */

#define PUTDG 3

#define DGBTMASTER   0x0001          /** Broadcast to master */
#define DGBTSLAVE    0x0002          /** Broadcast to slaves other than self*/
#define DGBTSELF     0x0004          /** Broadcast to self if slave*/
#define DGBTSPEC     0x0008          /** Broadcast to specific controller*/
#define DGBTOTHERS   0x0010          /** Send to all but myself*/
#define DGBTALL      0x0007          /** Send to all controllers*/
#define DGBTALLSLAVES 0x0014         /** Send to all slaves including self if slave*/

typedef struct MRPUTDG_REQ
{
    UINT8       tempPlaceHolder;    /**< Temporary place holder             */
} MRPUTDG_REQ;

/** Update VLINK information - Response **/
#define MRPUTDG_RSP         MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRNOPBE
**  BE no-op
**  @{
**/
/** BE no-op - Request **/
#define MRNOPBE_REQ         void

/** BE no-op - Response **/
#define MRNOPBE_RSP         MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRPUTFSYS
**  Put a file system report
**  @{
**/
/** Put a file system report - Request **/
typedef struct MRPUTFSYS_REQ
{
    UINT8       tempPlaceHolder;    /**< Temporary place holder             */
} MRPUTFSYS_REQ;

/** Put a file system report - Response **/
#define MRPUTFSYS_RSP       MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRGETDLINK
**  Get DLink information
**  @{
**/
/** Get DLink information - Request **/
#define MRGETDLINK_REQ      MR_DEVID_REQ

/** Get DLink information - Response **/
typedef struct MRGETDLINK_RSP
{
    MR_HDR_RSP  header;         /**< MRP Response Header - 8 bytes          */
    UINT16      dIndex;
    UINT8       linkStatus;     /**< 0=unused, 1=OK, 2=INOP, 3=missing  VERIFY THIS */
    UINT8       type;           /**< 0=Mag/Alt, 1=foreign                   */
    UINT8       rsvd12[1];      /**< RESERVED                               */
/* NOTE, devCap should be 64 bits, not 32 bits -- VLINK problem -- use GT2TB version. */
    UINT32      devCap;         /**< Size of base VDisk                     */
    UINT8       srcVName[8];    /**< VDisk name                             */
    UINT32      srcSerial;      /**< SN of remote device linked to          */
    UINT8       srcVBlock;      /**< Cluster or VBlock of linked to remote device */
    UINT8       srcVid;         /**< VDisk ID of linked to remote device    */
    UINT8       srcVPort;       /**< VPort ID of linked to remote device    */
    UINT8       srcNumConn;     /**< Number of connections (paths)          */
    UINT64      srcNodeWwn;     /**< Node WWN                               */
    UINT64      srcPortWwn1;    /**< Port WWN 1                             */
    UINT64      srcPortWwn2;    /**< Port WWN 2                             */
    UINT32      baseSerial;     /**< Local serial number                    */
    UINT8       baseVBlock;     /**< VBlock of linked to device on local system */
    UINT8       baseVid;        /**< Base Virtual Identifier                */
    UINT8       rsvd63[10];     /**< RESERVED                               */
    UINT8       prodId[16];     /**< Product ID string                      */
    UINT8       vendId[8];      /**< Vendor ID string                       */
    UINT8       srcName[8];     /**< Source ID string                       */
    UINT8       rev[4];         /**< Revision string                        */
    UINT8       serial[12];     /**< Serial number string                   */
} MRGETDLINK_RSP;
/* @} */

/**
**  @addtogroup MRGETDLINK_GT2TB
**  Get DLink information for vlink greater than 2TB
**  @{
**/
/** Get DLink information - Request **/
#define MRGETDLINK_GT2TB_REQ    MR_DEVID_REQ

/** Get DLink information - Response **/
typedef struct MRGETDLINK_GT2TB_RSP
{
    MR_HDR_RSP  header;         /**< MRP Response Header - 8 bytes          */
    UINT16      dIndex;
    UINT8       linkStatus;     /**< 0=unused, 1=OK, 2=INOP, 3=missing  VERIFY THIS */
    UINT8       type;           /**< 0=Mag/Alt, 1=foreign                   */
    UINT8       rsvd12[1];      /**< RESERVED                               */
    UINT64      devCap;         /**< Size of base VDisk                     */
    UINT8       srcVName[8];    /**< VDisk name                             */
    UINT32      srcSerial;      /**< SN of remote device linked to          */
    UINT8       srcVBlock;      /**< Cluster or VBlock of linked to remote device */
    UINT8       srcVid;         /**< VDisk ID of linked to remote device    */
    UINT8       srcVPort;       /**< VPort ID of linked to remote device    */
    UINT8       srcNumConn;     /**< Number of connections (paths)          */
    UINT64      srcNodeWwn;     /**< Node WWN                               */
    UINT64      srcPortWwn1;    /**< Port WWN 1                             */
    UINT64      srcPortWwn2;    /**< Port WWN 2                             */
    UINT32      baseSerial;     /**< Local serial number                    */
    UINT8       baseVBlock;     /**< VBlock of linked to device on local system */
    UINT8       baseVid;        /**< Base Virtual Identifier                */
    UINT8       rsvd63[10];     /**< RESERVED                               */
    UINT8       prodId[16];     /**< Product ID string                      */
    UINT8       vendId[8];      /**< Vendor ID string                       */
    UINT8       srcName[8];     /**< Source ID string                       */
    UINT8       rev[4];         /**< Revision string                        */
    UINT8       serial[12];     /**< Serial number string                   */
} MRGETDLINK_GT2TB_RSP;
/* @} */

/**
**  @addtogroup MRGETDLOCK
**  Get DLock information
**  @{
**/
/** Get DLock information - Request **/
#define MRGETDLOCK_REQ      MR_DEVID_REQ

/** Get DLock information - Response **/
typedef struct MRGETDLOCK_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT16      vid;        /**< Virtual Disk Identifier                    */
    UINT32      lockSN;     /**< Lock Serial Number                         */
    UINT8       lockVBlock; /**< Lock VBlock                                */
    UINT8       lockVid;    /**< Lock Virtual Disk Identifier               */
    UINT8       rsvd16[2];  /**< RESERVED                                   */
    UINT8       lockVDiskName[8];   /**< Lock Virtual Disk Name             */
    UINT8       lockSUName[8];  /**< Lock Storage Unit Name                 */
} MRGETDLOCK_RSP;
/* @} */

/**
**  @addtogroup MRDEGRADEPORT
**  Degrade / restore port
**  @{
**/
/**
**  @name Option values for Degrade/Restore Port
**  @{
**/
#define DRP_ACTION_RESTORE      0x00
#define DRP_ACTION_DEGRADE      0x01
/* @} */

/** Degrade / restore port - Request **/
typedef struct MRDEGRADEPORT_REQ
{
    UINT16  option;         /**< Option                                     */
    UINT8   action;         /**< Action                                     */
    UINT8   rsvd3;          /**< RESERVED                                   */
    UINT16  qualifier;      /**< Qualifier                                  */
    UINT16  rsvd6;          /**< RESERVED                                   */
    UINT32  port;           /**< Port to degrade or restore                 */
    UINT8  rsvd12[4];       /**< RESERVED                                   */
} MRDEGRADEPORT_REQ;

/** Degrade / restore port - Response **/
#define MRDEGRADEPORT_RSP   MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRGETWSINFO
**  Get Workset Information
**  @{
**/
#define GET_ALL_WORKSETS        0xFFFF  /**< ID value to get all worksets   */

/** Get Workset Information - Request **/
#define MRGETWSINFO_REQ         MR_DEVID_REQ

/** Get Workset Information - Response **/
typedef struct MRGETWSINFO_RSP
{
    MR_HDR_RSP  header;             /**< MRP Response Header - 8 bytes      */
    ZeroArray(DEF_WORKSET, workset);/**< Workset return data - 50 bytes per */
} MRGETWSINFO_RSP;
/* @} */

/**
**  @addtogroup MRSETWSINFO
**  Set Workset Information
**  @{
**/
/** Set Workset Information - Request **/
typedef struct MRSETWSINFO_REQ
{
    UINT16      id;                 /**< Workset ID                         */
    DEF_WORKSET  workset;           /**< Workset data - 50 bytes            */
} MRSETWSINFO_REQ;

/** Set Workset Information - Response **/
#define MRSETWSINFO_RSP         MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRCHGRAIDNOTMIRRORING
**  Change RAID Not Mirroring State
**  @{
**/
/**
**  @name Change RAID Not Mirroring State Types
**  @{
**/
#define MRCCLEARNOTMIRRORING    0       /**< Clear the Not Mirroring State  */
#define MRCSETNOTMIRRORING      1       /**< Set the Not Mirroring State    */
/* @} */

/** Change RAID Not Mirroring State - Request **/
typedef struct MRCHGRAIDNOTMIRRORING_REQ
{
    UINT8   type;           /**< Type of change - Set or Clear state        */
    UINT8   rsvd1[3];       /**< RESERVED                                   */
} MRCHGRAIDNOTMIRRORING_REQ;

/** Change RAID Not Mirroring State - Response **/
#define MRCHGRAIDNOTMIRRORING_RSP       MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRPUTLDD
**  Put the LDD
**  @{
**/
/** Put the LDD - Request **/
typedef struct MRPUTLDD_REQ
{
    UINT8       tempPlaceHolder;    /**< Temporary place holder             */
} MRPUTLDD_REQ;

/** Put the LDD - Response **/
#define MRPUTLDD_RSP            MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRRAIDRECOVER
**  Start recovery on an inoperable raid
**  @{
**/
/** raid recover - Request **/
#define MRRAIDRECOVER_REQ    MR_DEVID_REQ

/** raid_recover - Response **/
#define MRRAIDRECOVER_RSP    MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRPUTDEVCONFIG
**  Put the device configuration to the back end
**  @{
**/
/** Put the Device Config - Request **/
typedef struct MRPUTDEVCONFIG_REQ
{
    UINT16              numEntries;         /**< Number of entries in list  */
    UINT16              rsvd;               /**< Reserved                   */
    SES_DEV_INFO_MAP   *pEntries;           /**< PCI address to array of entries*/
} MRPUTDEVCONFIG_REQ;

/** Put the Device Config - Response **/
#define MRPUTDEVCONFIG_RSP            MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRRESYNCDATA
**  Get copy/resync data
**  @{
**/
/** Get copy/resync data - Request **/
typedef struct MRRESYNCDATA_REQ
{
    UINT8               format;     /**< format requested                   */
    UINT8               rsvd1[7];   /**< RESERVED                           */
} MRRESYNCDATA_REQ;

/** Get copy/resync data - Response **/
typedef struct MRRESYNCDATA_RSP
{
    MR_HDR_RSP          header;     /**< MRP Response Header - 8 bytes      */
    UINT16              strctCnt;   /**< Count of devices in the array      */
    UINT8               format;     /**< Returned format                    */
    UINT8               strctSiz;   /**< structure size (0 if > 255)        */
    ZeroArray(UINT8, data);         /**< Copy operation registration        */
                                    /**< NOTE: The preceeding array supports*/
                                    /**<       multiple data formats        */
} MRRESYNCDATA_RSP;

/**
 ** Format Types Definitions
 **/
#define MRDRCORRSP      0           /**< complete COR response              */
#define MRDTLCPYRSP     1           /**< detailed copy response             */
#define MRDRTRACES      2           /**< CCSM traces response               */
#define MRDRIOSTATUSMAP     3       /**< IO status map                      */

/**
 **     The following describes where the definitions of the following field
 **     can be found:
 **
 **         field               location of definition
 **         -----               ----------------------
 **         CORcstate           shared\inc\cor.h
 **         CORrstate           shared\inc\cor.h
 **         CORmstate           shared\inc\cor.h
 **         CMcstate            proc\inc\cm.h
 **         sp2hdlr             proc\scr\p6.c
 **         dp2hdlr             proc\scr\p6.c
 **         stype               proc\inc\scd.h
 **         dtype               proc\inc\dcd.h
 **         vmirror             shared\inc\vdd.h
 **         vattr               shared\inc\vdd.h
 **/

/**
**  Detained Copy Information (used in response packet)
**/
typedef struct MRCOPYDETAIL_INFO
{
    UINT32              rcsn;       /**< Copy ctrl sn                       */
    UINT32              rid;        /**< Copy registration id               */
    UINT8               rcscl;      /**< Copy reg. copy MAG source cl num   */
    UINT8               rcsdv;      /**< Copy reg. copy MAG source dv num   */
    UINT8               rcdcl;      /**< Copy reg. copy MAG dest. cl num    */
    UINT8               rcddv;      /**< Copy reg. copy MAG dest dv num     */

    UINT8               CORcstate;  /**< COR copy state                     */
    UINT8               CORrstate;  /**< COR copy registration state        */
    UINT8               CORmstate;  /**< COR region/segment map state       */
    UINT8               CMcstate;   /**< CM copy state code                 */
    UINT8               sp2hdlr;    /**< SCD p2 handler ordinal             */
    UINT8               stype;      /**< SCD type code                      */
    UINT8               dp2hdlr;    /**< DCD p2 handler ordinal             */
    UINT8               dtype;      /**< DCD type code                      */
    UINT8               vmirror;    /**< VDD mirror state                   */
    UINT8               vattr;      /**< VDD attributes                     */
    UINT8               owner;      /**< copy owner                         */
    UINT8               ctype;      /**< copy type                          */
    UINT8               rsvd[4];    /**< reserved                           */
}MRCOPYDETAIL_INFO;

/**
** Copy IO Status Information (used in response packet)
**/
typedef struct MRCOPYIOSTATUS_INFO
{
    UINT8               bitmap[(MAX_VIRTUAL_DISKS + 7) / 8];
}MRCOPYIOSTATUS_INFO;
/* @} */

/**
**  @addtogroup MRRESYNCCTL
**  Copy/resync control
**  @{
**/
/**
**  @name Copy/Resync control function codes
**  @{
**/
#define MRRESYNCCTL_DEL     0x00 /**< Delete the resync record              */
#define MRRESYNCCTL_NAME    0x01 /**< Update the label on a resync record   */
#define MRRESYNCCTL_SWAP    0x02 /**< Swap the raids involved               */
/* @} */

/** Copy/resync control - Request **/
typedef struct MRRESYNCCTL_REQ
{
    UINT8               fc;                 /**< Function Code              */
    UINT8               rsvd1[3];           /**< RESERVED                   */
    UINT32              rid;                /**< Copy registration ID       */
    UINT32              csn;                /**< Copy manager serial #      */
    UINT32              gid;                /**< GID number                 */
    UINT8               name[16];           /**< Copy user defined name     */
} MRRESYNCCTL_REQ;

/** Copy/resync control - Response **/
#define MRRESYNCCTL_RSP            MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRNOPFSYS
**  Internal file system no op
**  @{
**/
/** Internal file system no op - Request **/
#define MRNOPFSYS_REQ       void

/** Internal file system no op - Response **/
#define MRNOPFSYS_RSP       MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRFSYSOP
**  Internal file system operation
**  @{
**/
/**
**  @name File System Operation Types
**  @{
**/
#define MFSOPRD             0x00    /**< Read Operation                     */
#define MFSOPWR             0x01    /**< Write Operation                    */
/* @} */

/** Internal file system operation - Request **/
typedef struct MRFSYSOP_REQ
{
    UINT8           fid;        /**< File ID of the file to read/written    */
    UINT8           confirm;    /**< Confirm on a Read, 1 = yes             */
    UINT8           op;         /**< Operation to be performed              */
    UINT8           rsvd4[1];   /**< RESERVED                               */
    void           *buffptr;    /**< Buffer pointer to data space which
                                 **  contains write data or will receive
                                 **  read data                              */
    UINT16          bcount;     /**< Number of blocks to read/write         */
    UINT16          offset;     /**< Block offset at which to start in file.
                                 ** Note: sector 0 is the file header.      */
    FIO_DISK_MAP   *pGoodMap;   /**< PCI address to pDisk read map          */
} MRFSYSOP_REQ;

/** Internal file system operation - Response **/
#define MRFSYSOP_RSP        MR_FSYS_OP_RSP
/* @} */

/**
**  @addtogroup MRBEHBEAT
**  BE Heartbeat
**  @{
**/
/** BE Heartbeat - Request **/
#define MRBEHBEAT_REQ       MRHBEAT_REQ

/** BE Heartbeat - Response **/
#define MRBEHBEAT_RSP       MRHBEAT_RSP
/* @} */


/**
**  @addtogroup MRSETVPRI
**  Set priorities for virtual disks
**  @{
**/

/**
**  @name MRSETVPRI Option Values
**  @{
**/
#define MSPNOCHANGE     0     /**< Leave priorities of all the remaining VDisks unchanged */
#define MSPSETLOALL     1     /**< Set priority of all the remaining VDisks to low        */
/* @} */

/**
**  @name MRSETVPRI Status Values
**  @{
**/
#define MSPOK           0     /**< Set Priority Successful */
#define MSPBADOPT       1     /**< Invalid Option Value    */
#define MSPNOVIDS       2     /**< No Vid List             */
#define MSPBADCNT       3     /**< VID Count Too Big       */
/* @} */

/**
**  @name MRSETVPRI Response Values
**  @{
**/
#define MSPDONE         0     /**< OK               */
#define MSPNOVDD        1     /**< VDD NULL         */
#define MSPBADPRI       2     /**< Invalid Priority */
/* @} */

/**
**  @name MRSETVPRI VDisk Priority Levels
**  @{
**/
#define MSPSETLO        0    /**< Low Priority    */
#define MSPSETMED       1    /**< Medium Priority */
#define MSPSETHI        2    /**< High Priority   */
/* @} */

/** Set Priorities for virtual devices - VID-Priority pairs **/
typedef struct MRSETVPRI_VIDPRI
{
    UINT16 vid;                         /**< VID                                */
    UINT8  pri;                         /**< Priority in case of request        */
                                        /**< Response Value in case of response */
    UINT8  rsvd[1];                     /**< Reserved                           */
}MRSETVPRI_VIDPRI;

/** Set Priorities for virtual devices - Request **/
typedef struct MRSETVPRI_REQ
{
    UINT16 count;                        /**< No. of VID-Priority pairs         */
    UINT16 respcount;                    /**< No. of VIDs in response structure */
    UINT16 opt;                          /**< Options                           */
    UINT8  rsvd[2];                      /**< Reserved                           */
    ZeroArray(MRSETVPRI_VIDPRI,lst);     /**< VID-Priority List                 */
}MRSETVPRI_REQ;

/** Set Priorities for virtual devices - Response **/
typedef struct MRSETVPRI_RSP
{
    MR_HDR_RSP  header;                  /**< MRP Response Header - 8 bytes     */
    UINT16      count;                   /**< No. of VIDs                       */
    UINT8       rsvd[6];                 /**< Reserved                           */
    ZeroArray(MRSETVPRI_VIDPRI,lst);     /**< VID-Response Value List           */
} MRSETVPRI_RSP;
/* @} */

/**
**  @addtogroup MRSETTGINFO
**  Config info for iSCSI target
**  @{
**/

/** Set Target Info - Request **/
typedef struct MRSETTGINFO_REQ
{
    UINT32      setmap;
    I_TGD       i_tgd;
}MRSETTGINFO_REQ;

/** Set Target Info - Response **/
#define MRSETTGINFO_RSP   MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRGETTGINFO
**  Config info for iSCSI target
**  @{
**/

/**  iSCSI Target Info - Structure **/
typedef struct MRITGINFO
{
    I_TGD       i_tgd;
    UINT32      configmap;
} MRITGINFO;

/** Get Target Info - Request **/
typedef struct MRGETTGINFO_REQ
{
    UINT16      tid;
    UINT16      rsvd;
}MRGETTGINFO_REQ;

typedef struct MRGETTGINFO_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes  */
    UINT16      count;
    UINT16      rsvd;
    ZeroArray(MRITGINFO,ITGInfo);
}MRGETTGINFO_RSP;
/* @} */

/**
**  @addtogroup MRUPDSID
**  Update Server SID
**  @{
**/
/** Update Server SID - Request **/
typedef struct MRUPDSID_REQ
{
    UINT64      wwn;                /**< Server ISID                        */
    UINT16      sid;                /**< Server ID                          */
}MRUPDSID_REQ;

/** Update Server SID - Response **/
#define MRUPDSID_RSP   MR_GENERIC_RSP
/* @} */
/**
**  @addtogroup MRSETCHAP
**  Config chap user for iSCSI target
**  @{
**/

/** Config CHAP Info - Request **/
typedef struct MRCHAPCONFIG
{
    UINT16     tid;
    UINT8      opt;         /* Initiator/Target/Both */
    UINT8      sname[256];
    UINT8      secret1[32];
    UINT8      secret2[32];
    UINT8      rsvd;
}MRCHAPCONFIG;

typedef struct MRSETCHAP_REQ
{
    UINT16     count;
    UINT16     option;
    ZeroArray(MRCHAPCONFIG,userInfo);
}MRSETCHAP_REQ;

/** Set Target Info - Response **/
#define MRSETCHAP_RSP   MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRGETCHAP
**  Get chap user for iSCSI target
**  @{
**/

/** CHAP Info - Request **/
typedef struct MRGETCHAP_REQ
{
    UINT16      tid;
    UINT16      rsvd;
}MRGETCHAP_REQ;

/** Get CHAP User Info - Response **/
typedef struct MRCHAPINFO
{
    UINT16     tid;
    UINT16     rsvd;
    UINT8      sname[256];
}MRCHAPINFO;

typedef struct MRGETCHAP_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes  */
    UINT16 count;
    UINT16 rsvd;
    ZeroArray(MRCHAPINFO,userInfo);
} MRGETCHAP_RSP;
/* @} */

/**
**  @addtogroup MRGETSESSIONS
**  Get info of iSCSI Session
**  @{
**/

/** Connection Info Structure **/
typedef struct MRCONNECTION
{
    INT32    cid;                           /**< Connection ID               */
    INT32    state;                         /**< Connection State            */
    UINT32   numPduRcvd;                    /**< No. of PDUs received so far */
    UINT32   numPduSent;                    /**< No. of PDUs sent so far     */
    UINT64   totalReads;                    /**< No. of Read requests        */
    UINT64   totalWrites;                   /**< No. of Write requess        */

    UINT8    headerDigest[256];             /**< Header digest               */
    UINT8    dataDigest[256];               /**< Data digest                 */
    UINT8    authMethod[256];               /**< Authentication Protocol     */
    UINT8    chap_A;                        /**< CHAP Algorithm              */
    UINT8    ifMarker;                      /**< IF Marker                   */
    UINT8    ofMarker;                      /**< OF Marker                   */
    UINT32   maxRecvDataSegmentLength;      /**< Max recv data segment len   */
    UINT32   maxSendDataSegmentLength;      /**< Max recv data segment len   */
    UINT32   paramMap;                      /**< Bitmap of parameters already
                                                   negotiated (by initiator) */
    UINT32   paramSentMap;                  /**< Bitmap of parameters already
                                                   negotiated by the target  */
    UINT8    rsvd;                          /**< Reserved                    */
} MRCONNECTION;

/** Session Info Structure **/
typedef struct MRSESSION
{
    UINT16 tid;
    UINT16 rsvd;
    union{
        UINT64 sid;
        struct{
            UINT8  isid[6];
            UINT16 tsih;
        };
    };                                      /**< Session ID - ISID + TSIH    */
    UINT32 state;                           /**< Session State               */

    INT16  maxConnections;                  /**< Max Connections per Session */
    UINT8  targetName[256];                 /**< Target Name                 */
    UINT8  initiatorName[256];              /**< Initiator Name              */
    UINT8  targetAlias[256];                /**< Target Alias                */
    UINT8  initiatorAlias[256];             /**< Initiator Alias             */
    UINT8  targetAddress[256];              /**< Target Address              */
    UINT16 targetPortalGroupTag;            /**< Target Portal Group tag     */
    UINT8  initialR2T;                      /**< Initial R2T                 */
    UINT8  immediateData;                   /**< Immediate Data              */
    UINT32 maxBurstLength;                  /**< Max Burst Length            */
    UINT32 firstBurstLength;                /**< First Burst Length          */
    UINT16 defaultTime2Wait;                /**< Default time to wait        */
    UINT16 defaultTime2Retain;              /**< Default time to retain      */
    UINT16 maxOutstandingR2T;               /**< Max outstanding R2T         */
    UINT8  dataPDUInOrder;                  /**< Data PDU in order           */
    UINT8  dataSequenceInOrder;             /**< Data seq. in order          */
    UINT8  errorRecoveryLevel;              /**< Error recovery level        */
    UINT8  sessionType;                     /**< Session Type                */
    UINT32 paramMap;                        /**< Bitmap of parameters already
                                                   negotiated (by initiator) */
    UINT32 paramSentMap;                    /**< Bitmap of parameters already
                                                   negotiated by the target  */

    UINT32 numConnections;                  /**< Num. Connections in  Session */
/*    ZeroArray(MRCONNECTION,connInfo);*/
    MRCONNECTION connInfo[2];               /**< Info of all Connections      */
} MRSESSION;

/** Session Info - Request **/
typedef struct MRGETSESSIONS_REQ
{
    UINT16      tid;                        /**< Target ID                    */
    UINT16      rsvd;                       /**< Reserved                     */
}MRGETSESSIONS_REQ;

/** Session Info - Response **/
typedef struct MRGETSESSIONS_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes  */
    UINT16      numSessions;
    UINT16      rsvd;
    ZeroArray(MRSESSION,sessionInfo);
} MRGETSESSIONS_RSP;

/** Session Info on a Server - Request **/
typedef struct MRGETSESSIONSPERSERVER_REQ
{
    UINT8      sname[256];                  /**< iSCSI Name of the server  */
}MRGETSESSIONSPERSERVER_REQ;

#define MRGETSESSIONSPERSERVER_RSP     MRGETSESSIONS_RSP

/**
**  @addtogroup MRGETIDDINFO
**  Get IDD Info
**  @{
**/

/** Session IDD Info - Request **/

/** Session IDD Info Structure **/
typedef struct _MRIDD
{
    UINT16  flags;
    UINT8   rsvd1[2];
    UINT8   lid;
    UINT8   ptg;
    UINT16  port;
    INT32   sg_fd;
    UINT8   sg_name[16];
    UINT64  i_name;
    UINT64  t_name;
    UINT64  t_pname;
    UINT32  t_ip;
    UINT32  i_ip;
    UINT8   rsvd2[12];
} MRIDD;

/** Session IDD Info - Response **/
typedef struct _MRGETIDDINFO_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes  */
    UINT32 count;
    ZeroArray(MRIDD, iddInfo);
} MRGETIDDINFO_RSP;
/* @} */

/** DLM Path Stats - Response **/
typedef struct MRDLMPATHSTATS_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes  */
    UINT32 pathStats[25];   /**< Max ports for each ctrl = 5    */
                            /**< Max Possible paths = 5*5       */
} MRDLMPATHSTATS_RSP;

typedef struct MRDLMPATHSELECTIONALGO_REQ
{
    UINT8 algoType;
    UINT8 rsvd1[3];
} MRDLMPATHSELECTIONALGO_REQ;

typedef struct MRDLMPATHSELECTIONALGO_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes  */
    UINT8       algoType;   /**< Current DLM algorithm set      */
}MRDLMPATHSELECTIONALGO_RSP;
/* @} */

/**
**  @addtogroup MRENABLEISNS
**  set isns server ip
**  @{
**/

#define MAX_ISNS_SERVERS        5

#define ISNS_DISCOVERY_MANUAL   0
#define ISNS_DISCOVERY_AUTO     1

#define ISNS_SF_UDPTCP          0   /** < set - UDP; else TCP>  **/

/** isns server information data **/
typedef struct MRISNS_SERVER_INFO
{
    UINT32   ip;            /** <ip address isns server>        **/
    UINT16   port;          /** <tcp/udp port of isns server>   **/
    UINT16   flags;         /** <flags defined as ISNS_SF_* >   **/
} MRISNS_SERVER_INFO;

#define ISNS_CF_ENABLE          0   /** < set = Enable >        **/
#define ISNS_CF_AUTO            1   /** < set = Enable >        **/
#define ISNS_CF_RSVD            31  /** < internal use only>    **/

/** iSNS set info **/
typedef struct MRISNSINFO
{
    UINT32    flags;      /** <flags indicating auto & enable>  **/
    UINT32    nset;        /** <number of isns servers to configure> **/
    ZeroArray (MRISNS_SERVER_INFO, serverdata); /** <number of servers information> **/
}MRISNSINFO;

/** iSNS set info  - Request **/

#define MRSETISNSINFO_REQ          MRISNSINFO

/** iSNS set info - Response **/
#define MRSETISNSINFO_RSP          MR_GENERIC_RSP

/** Get isns information - response **/
typedef struct MRGETISNSINFO_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes  */
    MRISNSINFO rspdata;
}MRGETISNSINFO_RSP;

/* @} */

/**
**  @addtogroup MRVPRI_ENABLE
**  Enable/Disable VDisk Priority Feature
**  @{
**/
/**
**  @name MRVPRI_ENABLE Mode Values
**  @{
**/
#define MSPDISABLE      0    /**< Disable       */
#define MSPENABLE       1    /**< Enable        */
#define MSPSTATUS       2    /**< Display State */
/* @} */

/** Enable VDisk Priority - Request **/
typedef struct MRVPRI_ENABLE_REQ
{
    UINT8           mode;       /**< Mode ON/OFF                            */
    UINT8           rsvd[3];    /**< Reserved                               */
} MRVPRI_ENABLE_REQ;

/** Enable VDisk Priority - Response **/
typedef struct MRVPRI_ENABLE_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT8       mode;       /**< Current Feature Status (ON/OFF)            */
    UINT8       rsvd[3];    /**< Reserved for Quad Boundary                 */
} MRVPRI_ENABLE_RSP;
/* @} */

/**
**  @addtogroup MRPDISKSPINDOWN
**  PDisk Spindown
**  @{
**/

/** PDisk spin down - Request **/
typedef struct MRPDISK_SPINDOWN_REQ
{
    UINT16          pid;        /**< ID of the physical disk                */
    UINT16          rsvd;       /**< Reserved                               */
} MRPDISK_SPINDOWN_REQ;

/** Internal file system operation - Response **/
#define MRPDISK_SPINDOWN_RSP            MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRSETGEOLOCATION
**  Set Geo location for bay and drives
**  @{
**/

/** Set Geo location - Request **/
typedef struct MR_SET_GEO_LOCATION_REQ
{
    UINT16         bayId;        /**< ID of the disk bay                    */
    UINT8          locationId;   /**< ID of the location                    */
    UINT8          rsvd;         /**< Reserved                              */
} MR_SET_GEO_LOCATION_REQ;

/** Set Geo location  - Response **/
typedef struct MR_SET_GEO_LOCATION_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT8       anyHotSpares;/**< HotSpares present info                    */
    UINT64      wwn;        /**< WWN of the Bay                             */
    UINT8       rsvd[3];    /**< Reserved for Quad Boundary                 */
} MR_SET_GEO_LOCATION_RSP;
/* @} */

/**
**  @addtogroup MRCLEARGEOLOCATION
**  Clear Geo location of all the Bays and associated disks
**  @{
**/

/** Clear Geo location - Request **/
typedef struct MR_CLEAR_GEO_LOCATION_REQ
{
    UINT8          rsvd[4];      /**< Reserved                              */
} MR_CLEAR_GEO_LOCATION_REQ;

typedef struct MR_CLEAR_GEO_LOCATION_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MR_CLEAR_GEO_LOCATION_RSP;
/* @} */

/**
**  @addtogroup MRCONFIGUREOPTIONS
** Configure Options request and response structures.
**  @{
**/

/** Configure Options - Request */
typedef struct MRCFGOPTION_REQ
{
    UINT32          data;       /**< Option Data                            */
    UINT32          mask;       /**< Option Mask                            */
} MRCFGOPTION_REQ;

/** Configure Options - Response */
typedef struct MRCFGOPTION_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT32      option;     /**< Current Options                            */
} MRCFGOPTION_RSP;
/* @} */

/**
**  @addtogroup MRASYNCREPLICATION
** Async replication request and response.
**  @{
**/
/** Async replication - Request **/
typedef struct MRASYNCREP_REQ
{
    void   *bptr;           /**< Data is returned at this location.         */
    UINT32  blen;           /**< Amount of memory at above location.        */
} MRASYNCREP_REQ;

/** Async replication - Response **/
typedef struct MRASYNCREP_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT8       status;     /**< Completion status of the request.          */
    UINT32      count;      /**< Number of apool entries.                   */
    UINT32      retLen;     /**< Return data length.                        */
} MRASYNCREP_RSP;
/* @} */

/**
 ** ISE get ip addresses request and response
 ** structures.
 **/
/** ISE get ip address request */
typedef struct MRGETISEIP_REQ
{
    UINT16 bayid;           /** <ISE bay id               */
    UINT16 resvd;           /** <RESERVED                 */
} MRGETISEIP_REQ;

/** ISE get ip address response **/
typedef struct MRGETISEIP_RSP
{
    MR_HDR_RSP header;          /** < MRP Response Header                       */
    UINT16     bayid;           /** < ISE bay id alloted by proc                */
    UINT16     resvd;           /** <RESERVED                                   */
    UINT32     ip1;             /** < 1st IP address of ISE                     */
    UINT32     ip2;             /** < 2nd IP address of ISE                     */
    UINT64     wwn;
} MRGETISEIP_RSP;
/* @} */ /* End of CCB to BE MRP REQUEST and RESPONSE PACKETS */

/**
******************************************************************************
**  BE to FE MRP REQUEST and RESPONSE PACKETS
**  @addtogroup MRP_BE_TO_FE
**  @{
******************************************************************************
**/
/**
**  @addtogroup MRREPORTSCONFIG
**  Report server configuration
**  @{
**/
/** Report server configuration - Request **/
typedef struct MRLVM
{
    UINT16      lun;                /**< Associated LUN                     */
    UINT16      vid;                /**< Virtual Device ID                  */
} MRLVM;

/** Report server configuration - Request **/
typedef struct MRREPORTSCONFIG_REQ
{
    UINT16    sid;                  /**< Server ID                          */
    UINT8     del;                  /**< Delete this server                 */
    UINT8     rsvd;                 /**< Continue flag                      */
    UINT16    lsid;                 /**< Linked SID                         */
    UINT16    nluns;                /**< Number of LUNs in map              */
    UINT16    tid;                  /**< Target ID                          */
    UINT8     status;               /**< Status                             */
    UINT8     pri;                  /**< Priority                           */
    UINT32    attrib;               /**< Server attributes                  */
    UINT32    owner;                /**< Owner                              */
    UINT64    wwn;                  /**< World wide name                    */
    UINT8     name[16];              /**< Name                               */
#if 1
    UINT8     i_name[256];          /**< iSCSI Name                         */
#endif
    ZeroArray(MRLVM,lst);           /**< LVM Value List                     */
} MRREPORTSCONFIG_REQ;

/** Report server configuration - Response **/
typedef struct MRREPORTSCONFIG_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MRREPORTSCONFIG_RSP;
/* @} */

/**
**  @addtogroup MRUPDTGINFO
**  Change Target Type
**  @{
**/
/** Update Target Info - Response **/
#define MRUPDTGINFO_REQ   I_TGD

/** Update Target Info - Response **/
#define MRUPDTGINFO_RSP   MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRGETPORTTYPE
**  Get the FE Port type
**  @{
**/
/**
**  @name FE Port Type Values
**  @{
**/
#define PT_INVAL        0           /**< Invalid - port mismatch            */
#define PT_FC           1           /**< FC Port type                       */
#define PT_ISCSI        2           /**< iSCSI port type                    */
/* @} */
/** Get FE Port Type - Request **/
typedef struct MRGETPORTTYPE_REQ
{
    UINT8       pport;
    UINT8       aport;
}MRGETPORTTYPE_REQ;

/** Get FE Port Type - Response **/
typedef struct MRGETPORTTYPE_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes  */
    UINT8       type;
}MRGETPORTTYPE_RSP;
/* @} */

/**
**  @addtogroup MRSCONFIGCOMPLETE
**  Server configuration complete
**  @{
**/
/** Server configuration complete - Request **/
typedef struct MRSCONFIGCOMPLETE_REQ
{
    UINT8       tempPlaceHolder;    /**< Temporary place holder             */
} MRSCONFIGCOMPLETE_REQ;

/** Server configuration complete - Response **/
typedef struct MRSCONFIGCOMPLETE_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MRSCONFIGCOMPLETE_RSP;
/* @} */

/**
**  @addtogroup MRREPORTCCONFIG
**  Report caching configuration
**  @{
**/
/** Report caching configuration - Request **/
typedef struct MRREPORTCCONFIG_REQ
{
    UINT8       tempPlaceHolder;    /**< Temporary place holder             */
} MRREPORTCCONFIG_REQ;

/** Report caching configuration - Response **/
typedef struct MRREPORTCCONFIG_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MRREPORTCCONFIG_RSP;
/* @} */

/**
**  @addtogroup MRCCONFIGCOMPLETE
**  Caching configuration complete
**  @{
**/
/** Caching configuration complete - Request **/
typedef struct MRCCONFIGCOMPLETE_REQ
{
    UINT8       tempPlaceHolder;    /**< Temporary place holder             */
} MRCCONFIGCOMPLETE_REQ;

/** Caching configuration complete - Response **/
typedef struct MRCCONFIGCOMPLETE_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MRCCONFIGCOMPLETE_RSP;
/* @} */

/**
**  @addtogroup MRSPARE204
**  Spare
**  @{
**/
/** Spare - Request **/
typedef struct MRSPARE204_REQ
{
    UINT8       tempPlaceHolder;    /**< Temporary place holder             */
} MRSPARE204_REQ;

/** Spare - Response **/
typedef struct MRSPARE204_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MRSPARE204_RSP;
/* @} */


/**
**  @addtogroup MRSTOPCACHE
**  Stop caching
**  @{
**/
/** Stop caching - Request **/
typedef struct MRSTOPCACHE_REQ
{
    UINT8       wait;       /**< TRUE = Wait for completion                 */
    UINT8       rsvd1[3];   /**< RESERVED                                   */
} MRSTOPCACHE_REQ;

/** Stop caching - Response **/
#define MRSTOPCACHE_RSP     MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRCONTINUECACHE
**  Continue caching
**  @{
**/
/** Continue caching - Request **/
typedef struct MRCONTINUECACHE_REQ
{
    UINT8   option;         /**< Options byte                               */
    UINT8   rsvd1;          /**< Reserved                                   */
    UINT8   user;           /**< User ID to continue                        */
    UINT8   rsvd3;          /**< Reserved                                   */
} MRCONTINUECACHE_REQ;

/** Continue caching - Response **/
#define MRCONTINUECACHE_RSP     MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRSETSYSINFO
**  Set system serial number, etc
**  @{
**/
/** Set system serial number, etc - Request **/
typedef struct MRSETSYSINFO_REQ
{
    UINT8       tempPlaceHolder;    /**< Temporary place holder             */
} MRSETSYSINFO_REQ;

/** Set system serial number, etc - Response **/
typedef struct MRSETSYSINFO_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MRSETSYSINFO_RSP;
/* @} */

/**
**  @addtogroup MRVCHANGE
**  Virtual disk config has changed
**  @{
**/
/** Virtual disk config has changed - Request **/
typedef struct MRVCHANGE_REQ
{
    UINT8       tempPlaceHolder;    /**< Temporary place holder             */
} MRVCHANGE_REQ;

/** Virtual disk config has changed - Response **/
typedef struct MRVCHANGE_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MRVCHANGE_RSP;
/* @} */

/**
**  @addtogroup MRSCHANGE
**  Server config has changed
**  @{
**/
/** Server config has changed - Request **/
typedef struct MRSCHANGE_REQ
{
    UINT8       tempPlaceHolder;    /**< Temporary place holder             */
} MRSCHANGE_REQ;

/** Server config has changed - Response **/
typedef struct MRSCHANGE_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MRSCHANGE_RSP;
/* @} */

/**
**  @addtogroup MRREPORTTARG
**  Report target config
**  @{
**/
/** Report target config - Request **/
typedef struct MRREPORTTARG_REQ
{
    UINT8       tempPlaceHolder;    /**< Temporary place holder             */
} MRREPORTTARG_REQ;

/** Report target config - Response **/
typedef struct MRREPORTTARG_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MRREPORTTARG_RSP;
/* @} */

/**
**  @addtogroup MRRESETCONFIG
**  Reset configuration, NVRAM, etc
**  @{
**/
/** Reset configuration, NVRAM, etc - Request **/
#define MRRESETCONFIG_REQ   MR_RESET_REQ

/** Reset configuration, NVRAM, etc - Response **/
#define MRRESETCONFIG_RSP   MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRMMINFO
**  MM_INFO structure
**  @{
**/
/** Send MM_INFO - Request **/
typedef struct MRMMINFO_REQ
{
    void *pMMInfo;                  /**< pointer to MM Info struct          */
} MRMMINFO_REQ;

/** Send MM_INFO - Response **/
#define MRMMINFO_RSP MR_GENERIC_RSP
/* @} */
/* @} */ /* BE to FE MRP REQUEST and RESPONSE PACKETS */

/**
******************************************************************************
**  LOG ENTRY REQUEST and RESPONSE PACKETS
**  @addtogroup MRP_LOG_ENTRY
**  @{
******************************************************************************
**/
/**
**  @addtogroup MRLOGFE
**  Create log entry (Front end to CCB)
**  @{
**/
/** Create log entry (Front end to CCB) - Request **/
typedef struct MRLOGFE_REQ
{
    UINT8       tempPlaceHolder;    /**< Temporary place holder             */
} MRLOGFE_REQ;

/** Create log entry (Front end to CCB) - Response **/
typedef struct MRLOGFE_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MRLOGFE_RSP;
/* @} */

/**
**  @addtogroup MRLOGBE
**  Create log entry (Back end to CCB)
**  @{
**/
/** Create log entry (Back end to CCB) - Request **/
typedef struct MRLOGBE_REQ
{
    UINT8       tempPlaceHolder;    /**< Temporary place holder             */
} MRLOGBE_REQ;

/** Create log entry (Back end to CCB) - Response **/
typedef struct MRLOGBE_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MRLOGBE_RSP;
/* @} */
/* @} */ /* LOG ENTRY REQUEST and RESPONSE PACKETS */

/**
******************************************************************************
**  FE to BE MRP REQUEST and RESPONSE PACKETS
**  @addtogroup MRP_FE_TO_BE
**  @{
******************************************************************************
**/
/**
**  @addtogroup MRFEGETVINFO
**  Get VDisk info to FEP
**  @{
**/
/** Get VDisk info to FEP - Request **/
typedef struct MRFEGETVINFO_REQ
{
    UINT8       tempPlaceHolder;    /**< Temporary place holder             */
} MRFEGETVINFO_REQ;

/** Get VDisk info to FEP - Response **/
typedef struct MRFEGETVINFO_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MRFEGETVINFO_RSP;
/* @} */

/**
**  @addtogroup MRFESETSEQ
**  Set Sequence Number
**  @{
**/
/** Set Sequence Number - Request **/
typedef struct MRFESETSEQ_REQ
{
    UINT8       tempPlaceHolder;    /**< Temporary place holder             */
} MRFESETSEQ_REQ;

/** Set Sequence Number - Response **/
typedef struct MRFESETSEQ_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MRFESETSEQ_RSP;
/* @} */

/**
**  @addtogroup MRFESETMP
**  Set Mirror Partner
**  @{
**/
/** Set Mirror Partner - Request **/
typedef struct MRFESETMP_REQ
{
    UINT32      serialNumber;       /**< New Mirror Partner Serial Number -
                                         if zero use currently set one      */
} MRFESETMP_REQ;

/** Set Mirror Partner - Response **/
#define MRFESETMP_RSP   MR_GENERIC_RSP
/* @} */

/** Set Mirror Partner ConfigBE - Request  **/
#define MRSETMPCONFIGBE_REQ  MP_MIRROR_PARTNER_INFO

/** Set Mirror Partner ConfigBE - Response **/
#define MRSETMPCONFIGBE_RSP   MR_GENERIC_RSP
/* @} */ /* FE to BE MRP REQUEST and RESPONSE PACKETS */

typedef struct MRREGKEY
{
    UINT16    sid;                /* Server ID                            */
    UINT8     tid;                /* Target ID                            */
    UINT8     lun;                /* Lun #                                */
    UINT8     key[8];             /* Registration key                     */
} MRREGKEY;

/*
** flag  bit definitions
*/

#define MRPRR_REQORIGINATOR    0 /* Req originator                        */
                                 /* bit 1-7 reserved for future use       */

typedef struct MRSETPRES_REQ
{
    UINT16 vid;                  /* vdisk ID                   */
    UINT16 sid;                  /* sid holding reservation    */
    UINT8  scope;                /* Scope of Reservation       */
    UINT8  type;                 /* Type of reservation        */
    UINT8  regCount;             /* Total # of keys registered */
    UINT8  flags;                /* flag bits for process ctrl */
    ZeroArray(MRREGKEY, keyList);   /* List of registered keys    */
} MRSETPRES_REQ;

#define   MRSETPRES_RSP  MR_GENERIC_RSP

typedef struct MRRETRIEVEPR_REQ
{
    UINT16 vid;                  /* vdisk ID                   */
    UINT16 dummy;
} MRRETRIEVEPR_REQ;

#define MRPRR_SERVERDEL    0x00  /* Server Delete update       */
#define MRPRR_ASSOCDEL     0x01  /* Assoc Delete Update        */
                                 /* 2-15 are reserved          */

typedef struct MRUPDPRR_REQ
{
    UINT16 vid;                  /* vdisk ID                   */
    UINT16 sid;                  /* Server ID                  */
    UINT16 dummy;                /* Reserved field             */
    UINT16 flags;                /* flags                      */
} MRUPDPRR_REQ;

#define MRUPDPRR_RSP   MR_GENERIC_RSP

/*
** It is IMPORTANT to keep the MRSETPRES_REQ atruct consistent
** with the fields in MRRETRIEVE_RSP below for proper functioning!
*/
typedef struct MRRETRIEVEPR_RSP
{
    MR_HDR_RSP  header;          /**< MRP Response Header - 8 bytes              */
    UINT16 vid;                  /* vdisk ID                   */
    UINT16 sid;                  /* sid holding reservation    */
    UINT8  scope;                /* Scope of Reservation       */
    UINT8  type;                 /* Type of reservation        */
    UINT8  regCount;             /* Total # of keys registered */
    UINT8  flags;                /* flag bits for process ctrl */
    ZeroArray(MRREGKEY, keyList);   /* List of registered keys    */
} MRRETRIEVEPR_RSP;

/**
******************************************************************************
**  CCB to FE MRP REQUEST and RESPONSE PACKETS
**  @addtogroup MRP_CCB_TO_FE CCB to FE MRP command codes
**  @{
******************************************************************************
**/
/**
**  @addtogroup MRFELOOP
**  Get front end loop information
**  @{
**/
/** Get front end loop information - Request **/
#define MRFELOOP_REQ        MRPORT_REQ

/** Get front end loop information - Response **/
#define MRFELOOP_RSP        MRPORT_RSP
/* @} */

/**
**  @addtogroup MRFECONFIG
**  Set front end configuration
**  @{
**/
/** Get front end loop information - Request **/
#define MRFECONFIG_REQ      MRCONFIG_REQ

/** Get front end loop information - Response **/
#define MRFECONFIG_RSP      MRCONFIG_RSP
/* @} */

/**
**  @addtogroup MRPRREQ
**  Get/clear PR information
**  @{
**/
/** Clear vdisk persistent reserve data - Request **/
#define MRPRCLR_REQ         MR_DEVID_REQ

/** Clear vdisk persistent reserve data - Response **/
#define MRPRCLR_RSP         MR_GENERIC_RSP

/** Get vdisk persistent reserve data - Request **/
#define MRPRGET_REQ         MR_DEVID_REQ

/** Get vdisk persistent reserve data - Response **/
#define PR_KEY_SIZE            8

typedef struct MRPRGET_RSP
{
    MR_HDR_RSP      header;                 /**< MRP Response Header - 8 bytes          */
    UINT16          vid;                    /**< Virtual device ID                      */
    UINT16          sid;                    /**< Sid of reserve holder                  */
    UINT8           tid;                    /**< Target id                              */
    UINT8           lun;                    /**< Lun number                             */
    UINT8           scope;                  /**< Scope of the reservation               */
    UINT8           keyCnt;
    UINT8           keyList[PR_KEY_SIZE * MAX_TARGETS];
} MRPRGET_RSP;

/**
**  @addtogroup MRPRCONFIGCOMPLETE
**  PR config complete notification from CCB to FE
**  @{
**/

/** Persistent reserve config complete request structure **/
typedef struct MRPRCONFIGCOMPLETE_REQ
{
   UINT16       vid;                        /**< Vdisk ID                               */
   UINT16       sid;                        /**< Server ID                              */
   UINT8        tid;                        /**< Target ID                              */
   UINT8        lun;                        /**< Lun number                             */
   UINT16       dummy;                      /**< reserved space of 2 bytes              */
   INT32        rc;                         /**< status code                            */
   UINT32       dataSize;                   /**< Data size from original request        */
   void         *data;                      /**< Data pointer from original request     */
   UINT32       rsvd[3];                    /**< reserved for future purpose            */
}MRPRCONFIGCOMPLETE_REQ;
/* @} */

/** Persistent reserve config complete response **/
#define MRPRCONFIGCOMPLETE_RSP   MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRGETSINFO
**  Get server information
**  @{
**/
/** Get Server Information Lun Map                                          */
typedef struct MRGETSINFO_RSP_LM
{
    UINT16  vid;            /**< VDisk ID                                   */
    UINT16  lun;            /**< LUN number                                 */
} MRGETSINFO_RSP_LM;

/** Get server information - Request **/
#define MRGETSINFO_REQ      MR_DEVID_REQ

/**
**  @todo Convert the MRGETSINFO_RSP structure to use the SDD structure
**        when the appropriate code changes have been made.
**        NOTE: The SDD structure contains additional fields compared to
**        the MRP response structure.
**/
/** Get server information - Response **/
typedef struct MRGETSINFO_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT16      sid;        /**< Server ID                                  */
    UINT16      nluns;      /**< # of LUNS mapped to this server            */
    UINT16      targetId;   /**< Target ID                                  */
    UINT8       sstatus;    /**< Status of the server                       */
    UINT8       pri;        /**< HAB priority                               */
    UINT32      attrib;     /**< Server attributes                          */
    UINT32      session;    /**< Session identifier                         */
    UINT64      reqcnt;     /**< Request count for the server               */
    UINT8       rsvd32[4];  /**< RESERVED                                   */
    UINT32      owner;      /**< Serial number of owning controller         */
    UINT64      wwn;        /**< World wide name of the server              */
    UINT8       rsvd48[8];  /**< RESERVED                                   */
    UINT8       name[MAX_SERVER_NAME_LEN];  /**< Server Name                */
#if ISCSI_CODE
    UINT8       i_name[256];        /**< iSCSI Server name                  */
#endif
    ZeroArray(MRGETSINFO_RSP_LM, lunMap);   /**< LUN Map - see struct above */
} MRGETSINFO_RSP;
/* @} */

/**
**  @addtogroup MRGETCINFO
**  Get cache information
**  @{
**/
/** Get cache information - Request **/
#define MRGETCINFO_REQ      void

/** Get cache information - Response **/
typedef struct MRGETCINFO_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    CA          ca;         /**< Cache Information                          */
} MRGETCINFO_RSP;
/* @} */

/**
**  @addtogroup MRFELINK
**  Get front end link information
**  @{
**/
/** Get front end link information - Request **/
#define MRFELINK_REQ        void

/** Get front end link information - Response **/
#define MRFELINK_RSP        MR_LINK_RSP
/* @} */

/**
**  @addtogroup MRFEII
**  Get front end II information
**  @{
**/
/** Get front end II information - Request **/
#define MRFEII_REQ          void

/** Get front end II information - Response **/
typedef struct MRFEII_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    II          ii;         /**< Internal Information                       */
} MRFEII_RSP;
/* @} */

/**
**  @addtogroup MRGETCDINFO
**  Get cache device information
**  @{
**/
/** Get cache device information - Request **/
#define MRGETCDINFO_REQ     MR_DEVID_REQ

/**
**  @todo Convert the MRGETCDINFO_RSP structure to use a real structure
**        for the details when it is available.
**/
/** Get cache device information - Response **/
typedef struct MRGETCDINFO_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT16      vid;        /**< Virtual ID                                 */
    UINT8       errFlushCnt;/**< Count until flush                          */
    UINT8       stat;       /**< Status                                     */
    UINT8       rsvd12[4];  /**< RESERVED                                   */
    /* QUAD */
    UINT32      cache;      /**< Data in cache tree pointer                 */
    UINT32      dirty;      /**< Dirty data tree pointer                    */
    UINT32      io;         /**< Interval I/O tree pointer                  */
    UINT32      wrt_cnt;    /**< # of outstanding host cached wrt cmds      */
    /* QUAD */
    UINT64      flushLBA;   /**< Flush LBA                                  */
    UINT64      rdhits;     /**< Cache read hits                            */
    /* QUAD */
    UINT64      rdpart;     /**< Cache partial read hits                    */
    UINT64      rdmiss;     /**< Cache read misses                          */
    /* QUAD */
    UINT64      wrhits;     /**< Cache write hits                           */
    UINT64      wrpart;     /**< Cache partial write hits                   */
    /* QUAD */
    UINT64      wrmiss;     /**< Cache write misses                         */
    UINT64      wrtbyres;   /**< Bypassed writes - resources                */
    /* QUAD */
    UINT64      wrtbylen;   /**< Bypassed writes - length                   */
    UINT64      capacity;   /**< Capacity                                   */
    /* QUAD */
    UINT8       rsvd112[4]; /**< RESERVED                                   */
    UINT32      vtv;        /**< VDisk Throttle Value                       */
    void       *pTHead;     /**< Throttle Head of Queue                     */
    void       *pTTail;     /**< Throttle Tail of Queue                     */
    /* QUAD */
    void       *pFwdWait;   /**< VCD Wait que Forwared Pointer              */
    void       *pBwdWait;   /**< VCD Wait que Forwared Pointer              */
} MRGETCDINFO_RSP;
/* @} */

/**
**  @addtogroup MRGETSSTATS
**  Get server statistics
**  @{
**/
/**
**  @name Server Statistics Request Option
**  @{
**/
#define SS_TARGETS      1   /**< Check for WWN on specified target          */
#define SS_PORTS        2   /**< Check for WWN on specified port            */
/* @} */

/** Get server statistics - Request **/
typedef struct MRGETSSTATS_REQ
{
    UINT16      sid;        /**< Server Identification                      */
    UINT16      option;     /**< Server Statistics Request Option           */

    /*
    ** The following members are not used but have been partially
    ** coded in the PROC code.  These should be revisited and
    ** potentially added back in (another option to add would be
    ** a filter or format capability).
    */
  #if 0
    UINT16      tid;        /**< Target Identification                      */
    UINT8       port;       /**< Port Number                                */
    UINT8       rsvd7[1];   /**< RESERVED                                   */
  #endif
} MRGETSSTATS_REQ;

/** Get server statistics - Response **/
typedef struct MRGETSSTATS_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT64      ag_cmds;    /**< Aggregate total # commands                 */
    /* QUAD */
    UINT64      ag_bytes;   /**< Aggregate total # bytes                    */
    UINT64      ag_writes;  /**< Aggregate total # write commands           */
    /* QUAD */
    UINT64      ag_wbytes;  /**< Aggregate total # write bytes              */
    UINT64      ag_reads;   /**< Aggregate total # read commands            */
    /* QUAD */
    UINT64      ag_rbytes;  /**< Aggregate total # read bytes               */
    UINT64      per_cmds;   /**< Periodic total # commands                  */
    /* QUAD */
    UINT64      per_bytes;  /**< Periodic total # bytes                     */
    UINT64      per_writes; /**< Periodic total # write commands            */
    /* QUAD */
    UINT64      per_wbytes; /**< Periodic total # write bytes               */
    UINT64      per_reads;  /**< Periodic total # read commands             */
    /* QUAD */
    UINT64      per_rbytes; /**< Periodic total # read bytes                */
    /* FCAL Link status */
    UINT32      fl_lifcnt;  /**< Link Failure Count                         */
    UINT32      fl_lsscnt;  /**< Loss of Sync Count                         */
    /* QUAD */
    UINT32      fl_lsgcnt;  /**< Loss of Signal Count                       */
    UINT32      fl_pspec;   /**< Primitive Seq error count                  */
    UINT32      fl_ivtqc;   /**< Inv. Xmission Word Count                   */
    UINT32      fl_ivcrc;   /**< Invalid CRC count                          */
    /* QUAD */
    UINT16      qdepth;     /**< Queue depth                                */
    UINT8       rsvd130[2]; /**< RESERVED                                   */
} MRGETSSTATS_RSP;
/* @} */

/**
**  @addtogroup MRSETBATHEALTH
**  Set battery health
**  @{
**/

/**
**  @name Board Definitions
**
**  The board definitions have different meanings depending on the
**  current system configuration.
**  - SINGLE CONTROLLER: Both FE and BE boards are local
**  - 2-WAY with Mirror Partner: FE is local board and BE is partners board
**  - 2-WAY w/o Mirror Partner: FE and BE boards are local
**  - 2-WAY examples extend for N-Way, if a controller does not have a
**    mirror partner then both boards are local, otherwise the FE board is
**    local and the BE board is the partners board.
**
**  @{
**/
#define BATTERY_BOARD_FE        0x00
#define BATTERY_BOARD_BE        0x01
/* @} */

/**
**  @name Battery health
**  @{
**/
#define BATTERY_HEALTH_GOOD     0x00
#define BATTERY_HEALTH_FAIL     0x01
/* @} */

/** Set battery health - Request **/
typedef struct MRSETBATHEALTH_REQ
{
    UINT8  board;
    UINT8  rsvd1;
    UINT8  state;
    UINT8  rsvd2;
} MRSETBATHEALTH_REQ;

/** Set battery health - Response **/
#define MRSETBATHEALTH_RSP      MR_GENERIC_RSP
/* @} */


/**
**  @addtogroup MRRESUMECACHE
**  Resume Cache Initialization
**  @{
**/
/** Resume Cache Initialization - Request **/
typedef struct MRRESUMECACHE_REQ
{
    UINT8   userResp;       /**< User response                              */
    UINT8   rsvd1[3];       /**< RESERVED                                   */
} MRRESUMECACHE_REQ;

/** Resume Cache Initialization - Response **/
#define MRRESUMECACHE_RSP   MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRFEBOOT
**  Get front end boot code header
**  @{
**/
/** Get front end boot code header - Request **/
#define MRFEBOOT_REQ        void

/** Get front end boot code header - Response **/
#define MRFEBOOT_RSP        MR_FW_HEADER_RSP
/* @} */

/**
**  @addtogroup MRFEDIAG
**  Get front end diagnostic code header
**  @{
**/
/** Get front end diagnostic code header - Request **/
#define MRFEDIAG_REQ        void

/** Get front end diagnostic code header - Response **/
#define MRFEDIAG_RSP        MR_FW_HEADER_RSP
/* @} */

/**
**  @addtogroup MRFEPROC
**  Get front end proc code header
**  @{
**/
/** Get front end proc code header - Request **/
#define MRFEPROC_REQ        void

/** Get front end proc code header - Response **/
#define MRFEPROC_RSP        MR_FW_HEADER_RSP
/* @} */

/**
**  @addtogroup MRFRWMEM
**  R/W front end memory
**  @{
**/
/** R/W front end memory - Request **/
#define MRFRWMEM_REQ        MR_RW_MEMORY_REQ

/** R/W front end memory - Response **/
#define MRFRWMEM_RSP        MR_RW_MEMORY_RSP
/* @} */

/**
**  @addtogroup MRRESETFEPORT
**  Reset FE Port
**  @{
**/
/** Reset FE Port - Request **/
#define MRRESETFEPORT_REQ   MRRESETPORT_REQ

/** Reset FE Port - Response **/
#define MRRESETFEPORT_RSP   MRRESETPORT_RSP
/* @} */

/**
**  @addtogroup MRSERVERLOOKUP
**  Server lookup MRP
**  @{
**/
/** Server lookup MRP - Request **/
typedef struct MRSERVERLOOKUP_REQ
{
    UINT64  wwn;            /**< World Wide Name                            */
    UINT16  targetId;       /**< Target ID                                  */
    UINT8   rsvd10[6];      /**< RESERVED                                   */
} MRSERVERLOOKUP_REQ;

/** Server lookup MRP - Response **/
typedef struct MRSERVERLOOKUP_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT16      serverId;   /**< Server ID                                  */
    UINT8       rsvd10[2];  /**< RESERVED                                   */
} MRSERVERLOOKUP_RSP;
/* @} */

/**
**  @addtogroup MRFEGENERIC
**  Generic
**  @{
**/
/** Generic - Request **/
#define MRFEGENERIC_REQ     MR_BEFE_GENERIC_REQ

/** Generic - Response **/
#define MRFEGENERIC_RSP     MR_BEFE_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRFEASSIGNMP
**  Assign a Mirror Partner
**  @{
**/
/** Assign a Mirror Partner - Request **/
typedef struct MRFEASSIGNMP_REQ
{
    UINT32  serialNumber;   /**< New mirror partner serial number           */
} MRFEASSIGNMP_REQ;

/** Assign a Mirror Partner - Response **/
typedef struct MRFEASSIGNMP_RSP
{
    MR_HDR_RSP  header;         /**< MRP Response Header - 8 bytes          */
    UINT32      serialNumber;   /**< Old mirror partner serial number       */
} MRFEASSIGNMP_RSP;
/* @} */

/**
**  @addtogroup MRSETMPCONFIGFE
**  Set Mirror Partner Configuration
**  @{
**/
/** Set Mirror Partner Configuration - Request **/
#define MRSETMPCONFIGFE_REQ     MP_MIRROR_PARTNER_INFO

/** Set Mirror Partner Configuration - Response **/
typedef struct MRSETMPCONFIGFE_RSP
{
    MR_HDR_RSP          header;    /**< MRP Response Header - 8 bytes      */
    UINT32              oldSerialNumber;/**< Old mirror partner serial number */
} MRSETMPCONFIGFE_RSP;
/* @} */

/**
**  @addtogroup MRGETMPCONFIGFE
**  Get Mirror Partner Configuration
**  @{
**/
/* MRGETMPCONFIGFE request and response structs */
/* There is no request data for this MRP */

/** Get Mirror Partner Configuration - Request **/
#define MRGETMPCONFIGFE_REQ     void

/** Get Mirror Partner Configuration - Response **/
typedef struct MRGETMPCONFIGFE_RSP
{
    MR_HDR_RSP          header;    /**< MRP Response Header - 8 bytes      */
    MP_MIRROR_PARTNER_INFO    mirrorPartnerInfo;/**< MP return data - 32 bytes */
} MRGETMPCONFIGFE_RSP;
/* @} */

/**
**  @addtogroup MRFEFIBREHLIST
**  FE Fibre Heartbeat List for DLM
**  @{
**/
/** FE Fibre Heartbeat List for DLM - Request **/
typedef struct MRFEFIBREHLIST_REQ
{
    UINT8       numControllers;     /**< Number of controllers in list     */
    UINT8       rsvd1[3];           /**< RESERVED                          */
    ZeroArray(UINT32, controllers); /**< List of controllers               */
} MRFEFIBREHLIST_REQ;

/** FE Fibre Heartbeat List for DLM - Response **/
typedef struct MRFEFIBREHLIST_RSP
{
    MR_HDR_RSP  header;             /**< MRP Response Header - 8 bytes      */
    UINT32      invalidController;  /**< Invalid controller, if any         */
} MRFEFIBREHLIST_RSP;
/* @} */

/**
**  @addtogroup MRFECONTWOMP
**  Continue Cache Init w/o Mirror Partner
**  @{
**/
/** Continue Cache Init w/o Mirror Partner - Request **/
#define MRFECONTWOMP_REQ    void

/** Continue Cache Init w/o Mirror Partner - Response **/
#define MRFECONTWOMP_RSP    MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRFEFLUSHWOMP
**  Flush FE Cache w/o Mirror Partner
**  @{
**/
/** Flush FE Cache w/o Mirror Partner - Request **/
#define MRFEFLUSHWOMP_REQ   void

/** Flush FE Cache w/o Mirror Partner - Response **/
#define MRFEFLUSHWOMP_RSP   MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRINVFEWC
**  Invalidate the FE Write Cache
**  @{
**/

/**
**  @name MRINVFEWC - option values
**  @{
**/
#define MIFOINVALVID    0   /* Invalidate the list of VIDs provided */
#define MIFOGLOBALINVAL 1   /* Global invalidate */
/* @} */

/** Invalidate the FE Write Cache - Request **/
#define MRINVFEWC_REQ       MRINVWC_REQ

/** Invalidate the FE Write Cache - Response **/
#define MRINVFEWC_RSP       MRINVWC_RSP
/* @} */

/**
**  @addtogroup MRFLUSHBEWC
**  Flush the BE Write Cache
**  @{
**/
/**
**  @name MRFLUSHBEWC Flush Options
**  @{
**/
#define MFBOINVALVID        0x00    /**< Flush for these VIDS               */
#define MFBOGLOBALINVAL     0x01    /**< Global Invalidate                  */
/* @} */

/** Flush the BE Write Cache - Request **/
typedef struct MRFLUSHBEWC_REQ
{
    UINT8       option;         /**< Options (see values below)             */
    UINT8       rsvd1[1];       /**< RESERVED                               */
    UINT16      numVids;        /**< if flushing VID's, number of VIDS      */
    ZeroArray(UINT16, vidList); /**< If flushing VID's, 1st element of list */
} MRFLUSHBEWC_REQ;

/** Flush the BE Write Cache - Response **/
typedef struct MRFLUSHBEWC_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT16      badVID;     /**< Bad VID                                    */
    UINT8       rsvd10[2];  /**< RESERVED                                   */
} MRFLUSHBEWC_RSP;
/* @} */

/**
**  @addtogroup MRINVBEWC
**  Invalidate the BE Write Cache
**  @{
**/

/**
**  @name MRINVFEWC - option values
**  @{
**/
#define MIBOINVALVID    0   /* Invalidate the list of VIDs provided */
#define MIBOGLOBALINVAL 1   /* global invalidate */
/* @} */

/** Invalidate the BE Write Cache - Request **/
#define MRINVBEWC_REQ       MRINVWC_REQ

/** Invalidate the BE Write Cache - Response **/
#define MRINVBEWC_RSP       MRINVWC_RSP
/* @} */

/**
**  @addtogroup MRFEMODEPAGE
**  Mode page
**  @{
**/
/** Mode page - Request **/
#define MRFEMODEPAGE_REQ    MR_MODE_SET_REQ

/** Mode page - Response **/
#define MRFEMODEPAGE_RSP    MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRFEGETDVLIST
**  Get front end device information
**  @{
**/
/** Get front end device information - Request **/
#define MRFEGETDVLIST_REQ   MRGETDVLIST_REQ

/** Get front end device information - Response **/
#define MRFEGETDVLIST_RSP   MRGETDVLIST_RSP
/* @} */

/**
**  @addtogroup MRFEGETPORTLIST
**  Get FE port list
**  @{
**/
/** Get FE port list - Request **/
#define MRFEGETPORTLIST_REQ     MRGETPORTLIST_REQ

/** Get FE Port List - Response **/
#define MRFEGETPORTLIST_RSP     MR_LIST_RSP
/* @} */

/**
**  @addtogroup MRGETTRLIST
**  Get target resource list types
**  @{
**/
#define SERVERS_W_STATS   0
#define SERVERS           1
#define SERVERS_DEFAULT   2
#define SERVERS_UNMANAGED 3
#define SERVERS_MANAGED   4
#define SERVERS_XIO       5
#define SERVERS_LOGON     6
#define SERVERS_ACTIVE    7
#define VDISKS            8
#define VDISKS_CACHEN     9
#define VDISKS_CACHDIS    10
#define VDISK_UNMAPPED    11
#define LUNMAP            12
#define LUNMAP_CACHEN     13
#define LUNMAP_CACHDIS    14
#define LUNMAP_DEFAULT    15

#define WWN_FORMAT        0x20  /**< Add to constants above for WWN format */

/**
**  Get Target Resource List WWN format
**
**  Use the struct below to derive the Target Resource info in sid/WWN
**  format.  Rather than complicate matters creating a union, etc. just
**  cast the pointer to list in MRGETTRLIST_RSP to this struct.
*/
typedef struct MRGETTRLIST_RSP_WWNFMT
{
    UINT16  sid;            /**< Server ID                                  */
    UINT16  tid;            /**< Target ID                                  */
    UINT64  wwn;            /**< WWN                                        */
    UINT8   rsvd12[4];      /**< RESERVED                                   */
} MRGETTRLIST_RSP_WWNFMT;

/** Get target resource list - Request **/
typedef struct MRGETTRLIST_REQ
{
    UINT16  sid;            /**< Starting device ID                         */
    UINT16  tid;            /**< Target ID                                  */
    UINT8   listType;       /**< List requested: 1 = Servers, 2 = VDisks    */
    UINT8   rsvd5[3];       /**< RESERVED                                   */
} MRGETTRLIST_REQ;

/** Get target resource list - Response **/
typedef struct MRGETTRLIST_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT16      ndevs;      /**< Number of items in the list                */
    UINT16      entrySize;  /**< Size in bytes of each list entry           */
    ZeroArray(UINT16, list);/**< List of IDs                                */
} MRGETTRLIST_RSP;

/*
** Constant to define the largest entry size of the various entry types
** returned by Target Resource List.
*/
#define MRGETTRLIST_MAX_ENTRY_SIZE      (sizeof(MRGETTRLIST_RSP_WWNFMT))
/* @} */

/**
**  @addtogroup MRSTOPIO
**  Stop I/O
**  @{
**/
/** Stop I/O - Request **/

/* STOP IO constants:                               */
/* OR together options for "operation" parameter    */
#define STOP_WAIT_FOR_FLUSH     0x01
#define STOP_NO_WAIT_FOR_FLUSH  0x00

#define STOP_FLUSH              0x00
#define STOP_NO_FLUSH           0x02

#define STOP_BACKGROUND         0x00
#define STOP_NO_BACKGROUND      0x04

#define STOP_WAIT_FOR_HOST_IO   0x00
#define STOP_NO_WAIT_FOR_HOST   0x08

/* "intent" parameter options                       */
#define STOP_SHUTDOWN           0x01
#define STOP_NO_SHUTDOWN        0x00

/* START IO "option" constants                      */
#define START_IO_OPTION_CLEAR_ONE_STOP_COUNT    0x00
#define START_IO_OPTION_CLEAR_ALL_STOPS         0x01

/*
** START/STOP IO "user" constants
**
**  0x00      = Generic User (all Stop/Resumes must be paired)
**
**  0x01-0x3F = PROC users
**      0x01 = Define FE
**      0x02 = Battery Health
**      0x03 = Write Cache Recovery
**          Note: wcrcvry.c has constant for
**              0x04-0x1F   = Reserved for FE Stops
**              0x20        = Define BE
**              0x21        = Target Control Update
**              0x22        = BE Rebuild Update
**              0x23-0x3F   = Reserved for BE Stops
**
**  0x40-0x7F = CCB users
**      0x40 = Election
**      0x41 = Sequence Manager
**
**  0x80-0xBF = XSSA users
**      0x80 = Generic
**
**  0xC0-0xFF = Reserved
*/
#define START_STOP_IO_USER_BE_REBUILD           0x22
#define START_STOP_IO_USER_CCB_ELECTION         0x40
#define START_STOP_IO_USER_CCB_SM               0x41
#define START_STOP_IO_USER_CCB_DEFRAG           0x42
#define START_STOP_IO_USER_CCB_ORPHANS          0x43

typedef struct MRSTOPIO_REQ
{
    UINT8   operation;          /**< Flush operation requested              */
    UINT8   intent;             /**< Intent                                 */
    UINT8   user;               /**< User                                   */
    UINT8   rsvd3;              /* RESERVED                                 */
} MRSTOPIO_REQ;

/** Stop I/O - Response **/
#define MRSTOPIO_RSP        MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRSTARTIO
**  Start I/O
**  @{
**/
/** Start I/O - Request **/
typedef struct MRSTARTIO_REQ
{
    UINT8   option;             /**< Start option value                     */
    UINT8   rsvd1;              /*   RESERVED                               */
    UINT8   user;               /**< User                                   */
    UINT8   rsvd3;              /*   RESERVED                               */
} MRSTARTIO_REQ;

/** Start I/O - Response **/
#define MRSTARTIO_RSP       MR_GENERIC_RSP
/* @} */

/*
** MRP to handle All devices missing
**
**
*/
typedef struct MRALLDEVMISS_REQ
{
#if 1
    UINT8      syncVdMap[(MAX_VIRTUAL_DISKS+7)/8];
    UINT8      srcVdMap[(MAX_VIRTUAL_DISKS+7)/8];
#else
    UINT8      syncVdMap[32];
    UINT8      srcVdMap[32];
#endif
} MRALLDEVMISS_REQ;

/*
** Response
**
*/
typedef struct MRALLDEVMISS_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
} MRALLDEVMISS_RSP;

/**
**  @addtogroup MRFEPORTNOTIFY
**  Set FE port event notification
**  @{
**/
/** Set FE port event notification - Request **/
typedef struct MRFEPORTNOTIFY_REQ
{
    UINT32  loopDownToNotify;   /**< Loop down to notify                    */
    UINT32  resetToNotify;      /**< Reset to notify                        */
    UINT32  loopDownToReset;    /**< Loop down to reset                     */
    UINT32  resetRetries;       /**< Reset retry count                      */
    UINT32  softResetPeriod;    /**< Soft reset period                      */
    UINT32  errorLimit;         /**< Error Limit before notify              */
    UINT8   rsvd24[8];          /**< RESERVED                               */
} MRFEPORTNOTIFY_REQ;

/** Set FE port event notification - Response **/
#define MRFEPORTNOTIFY_RSP  MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRFORCEFEETRAP
**  Force a FE error trap
**  @{
**/
/** Force a FE error trap - Request **/
#define MRFORCEFEETRAP_REQ  void

/** Force a FE error trap - Response **/
#define MRFORCEFEETRAP_RSP  MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRFELOOPPRIMITIVE
**  Loop primitive
**  @{
**/
/** Loop primitive - Request **/
#define MRFELOOPPRIMITIVE_REQ   MRLOOPPRIMITIVE_REQ

/** Loop primitive - Response **/
#define MRFELOOPPRIMITIVE_RSP   MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRGETTARG
**  Get target information
**  @{
**/

/** Get target information - Request **/
#define MRGETTARG_REQ       MR_DEVID_REQ

/** Get target information - Response **/
typedef struct MRGETTARG_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT16      tid;        /**< Target ID                                  */
    UINT8       port;       /**< port number                                */
    UINT8       opt;        /**< Options hard or soft ID, active or inact   */
    UINT8       fcid;       /**< Fibre Channel ID                           */
    UINT8       rsvd13;     /**< RESERVED                                   */
    UINT8       lock;       /**< Target lock bits                           */
    UINT8       rsvd15;     /**< RESERVED                                   */
    /* QUAD */
    UINT64      pname;      /**< Port name                                  */
    UINT64      nname;      /**< Node name                                  */
    /* QUAD */
    UINT32      powner;     /**< Previous owning controller serial number   */
    UINT32      owner;      /**< Owning controller serial number            */
    UINT16      cluster;    /**< Clustered target ID                        */
    UINT8       rsvd42[2];  /**< RESERVED                                   */
    UINT8       pport;      /**< Primary port                               */
    UINT8       aport;      /**< Alternate port                             */
    UINT8       rsvd46[2];  /**< RESERVED                                   */
    /* QUAD */
    UINT8       rsvd48[4];  /**< RESERVED                                   */
    UINT32      modMask;    /**< Define which fields to modify 0 = all      */
} MRGETTARG_RSP;
/* @} */

/**
**  @addtogroup MRFAILPORT
**  Fail / Unfail port
**  @{
**/
/**
**  @name MRFAILPORT_REQ Option Definitions
**  @{
**/
#define FP_NO_INIT          0x0001  /**< Do not initialize source ports     */
/* @} */

/** Fail / Unfail port - Request **/
typedef struct MRFAILPORT_REQ
{
    UINT16  option;         /**< Option                                     */
    UINT8   fail;           /**< Fail it                                    */
    UINT8   rsvd3;          /**< RESERVED                                   */
    UINT16  qualifier;      /**< Qualifier                                  */
    UINT8   rsvd6[2];       /**< RESERVED                                   */
    UINT32  port;           /**< port number (Chip Instance)                */
    UINT8   rsvd12[4];      /**< RESERVED                                   */
} MRFAILPORT_REQ;

/** Fail / Unfail port - Response **/
#define MRFAILPORT_RSP      MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRNOPFE
**  FE no-op
**  @{
**/
/** FE no-op - Request **/
#define MRNOPFE_REQ         void

/** FE no-op - Response **/
#define MRNOPFE_RSP         MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRFEHBEAT
**  FE heartbeat
**  @{
**/
/** FE heartbeat - Request **/
#define MRFEHBEAT_REQ       MRHBEAT_REQ

/** FE heartbeat - Response **/
#define MRFEHBEAT_RSP       MRHBEAT_RSP
/* @} */

/**
**  @addtogroup MRQFECC
**  Query FE Controller Communications
**  @{
**/
/** Query FE Controller Communications - Request **/
typedef struct MRQFECC_REQ
{
    UINT32      serial;     /**< Controller Serial Number to Queury         */
} MRQFECC_REQ;

/** Query FE Controller Communications - Response **/
#define MRQFECC_RSP         MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRQSC
**  Query Stop Completion
**  @{
**/
/** Query Stop Completion - Request **/
#define MRQSC_REQ     void

/** Query Stop Completion - Response **/
typedef struct MRQSC_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT32      numOut;     /**< Number of Outstanding Host Ops             */
} MRQSC_RSP;
/* @} */

/**
**  @addtogroup MRQMPC
**  Query if a Mirror Partner Change will Succeed
**  @{
**/
/**
**  @name MRQMPC BIT - mpResponse bit values
**  @{
**/
#define MRQMPCNOSTOP    0   /**< Bit 0 = I/O Is not stopped                 */
#define MRQMPCIOOUTS    1   /**< Bit 1 = I/O are still outstanding          */
#define MRQMPCCACHEDATA 2   /**< Bit 2 = Data Still Cached for old MP       */
#define MRQMPCNOCOMM    3   /**< Bit 3 = No Communications Path to new MP   */
/* @} */

/**
**  @name MRQMPC MASK - mpResponse masks
**  @{
**/
#define MRQMPC_MASK_ALL     0xFF    /**< All bits need to be checked        */
#define MRQMPC_MASK_NO_COMM 0xF8    /**< Check all but the first three bits */
/* @} */

/** Query Mirror Partner Change Possible - Request **/
typedef struct MRQMPC_REQ
{
    UINT32      serial;     /**< Possible new Mirror Partner                */
} MRQMPC_REQ;

/** Query Mirror Partner Change Possible - Response **/
typedef struct MRQMPC_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT8       mpResponse; /**< Mirror Partner Change Response bits        */
    UINT8       rsvd5[3];   /**< Reserved                                   */
} MRQMPC_RSP;
/* @} */

/**
**  @addtogroup MRGETHABSTATS
**  Get hab statistics
**  @{
**/

/** Get HAB statistics - Request **/
#define MRGETHABSTATS_REQ       MR_DEVID_REQ

/** Get HAB statistics - Response **/
typedef struct MRGETHABSTATS_RSP
{
    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
    UINT64      perCmds;    /**< Periodic total # commands                  */
                            /* QUAD */
    UINT16      qdepth;     /**< Queue depth                                */
    UINT8       rsvd18[6];  /**< RESERVED                                   */
    UINT64      avgReqSize; /**< average request size (in sectors)          */
                            /* QUAD */
    UINT64      writeReqs;  /**< total # of write requests                  */
    UINT64      readReqs;   /**< total # of read requests                   */
                            /* QUAD */
    UINT8       rsvd48[16]; /**< RESERVED (16 bytes)                        */
                            /* QUAD */
} MRGETHABSTATS_RSP;
/* @} */

/**
**  @addtogroup MRMMCARDGETBATTERYSTATUS
**  Get MM Card Status
**  @{
**/
typedef UINT16 VOLTAGE;                 /**< In millivolts                  */
typedef UINT16 PERCENTAGE;              /**< In tenths of a percent         */
#define NVRAM_BOARD_MAX_BATTERIES       4

/**
**  @name   NVRAM battery status definitions
**  @{
**/
#define NVRAM_BATTERY_INFO_STATUS_UNKNOWN               0   /**< unknown status, e.g. battery not present   */
#define NVRAM_BATTERY_INFO_STATUS_GOOD                  1   /**< GOOD and operational                       */
#define NVRAM_BATTERY_INFO_STATUS_DISABLED_HW           2   /**< Battery disabled throughHW                 */
#define NVRAM_BATTERY_INFO_STATUS_DISABLED_SW           3   /**< Battery disabled throughSW                 */
#define NVRAM_BATTERY_INFO_STATUS_DISABLED_BOTH_HW_SW   4   /**< Battery disabled through both HW and SW    */
#define NVRAM_BATTERY_INFO_STATUS_FAILURE               5   /**< Battery failed                             */
/* @} */

/** NVRAM Battery Information **/
typedef struct
{
    UINT32      status;
    VOLTAGE     voltage;
    PERCENTAGE  chargePercent;
} NVRAM_BATTERY_INFO;

/** NVRAM board revision information **/
typedef struct
{
    UINT32 major;
    UINT32 minor;
} NVRAM_BOARD_REVISION;

/**
**  @name NVRAM board status definitions
**  @{
**/
/* The duplicates below are redefinitions to shorter names                      */
#define NVRAM_BOARD_STATUS_UNKNOWN          0   /**< unknown status, e.g. board not present */
#define NVRAM_BOARD_STATUS_GOOD             1   /**< GOOD and operational       */
#define NVRAM_BOARD_STATUS_SHUT_DOWN        2   /**< GOOD, but in shipping mode */
#define NVRAM_BOARD_STATUS_LOW_BATTERY      3   /**< Battery charging, but low  */
#define NVRAM_BOARD_STATUS_BATTERY_FAILURE  4   /**< Battery malfunction        */
#define NVRAM_BOARD_STATUS_MEMORY_FAILURE   5   /**< General memory failure     */
/* Following are duplicates of above, shorter names. */
#define NV_STS_UNKNOWN                      0   /**< unknown status, e.g. board not present */
#define NV_STS_GOOD                         1   /**< GOOD and operational       */
#define NV_STS_SHUTDOWN                     2   /**< GOOD, but in shipping mode */
#define NV_STS_LOW_BATT                     3   /**< Battery charging, but low  */
#define NV_STS_BATT_FAIL                    4   /**< Battery malfunction        */
#define NV_STS_MEM_FAIL                     5   /**< General memory failure     */
#define NV_STS_SN_MISMATCH                  6   /**< Controller S/N mismatch    */
#define NV_STS_NO_BOARD                     7   /**< No MM board in the system  */
/* @} */

/**
**  @name NVRAM board fatal event definitions
**  @{
**/
#define NV_FATAL_ECC_MULTI      0x0001  /**< Multi-bit ECC error detected       */
#define NV_FATAL_ECC_THRESH     0x0002  /**< Too many single-bit ECC errors det */
#define NV_FATAL_BATT           0x0003  /**< Battery failure - break out?       */
#define NV_FATAL_NO_CARD        0x0004  /**< No MM card found                   */
#define NV_FATAL_POST           0x0005  /**< POST test failed                   */
#define NV_FATAL_ECC_UNCORR     0x0006  /**< Uncorrectable ECC error detected   */
#define NV_FATAL_ASSERT         0x0007  /**< Software sanity error detected     */
#define NV_FATAL_SN_MISMATCH    0x0008  /**< Card belongs to a different ctrl   */
#define NV_FATAL_USER_FAILED    0x1000  /**< User failed board                  */
/* @} */

/**
**  @name NVRAM board command definitions
**  @{
**/
#define NVRAM_BOARD_COMMAND_INFO_ONLY   0   /**< No command is being sent   */
#define NV_CMD_INFO                     0   /**< No command is being sent   */
#define NVRAM_BOARD_COMMAND_SHUT_DOWN   1   /**< Place into low power mode  */
#define NV_CMD_SHUTDOWN                 1   /**< Place into low power mode  */
#define NV_CMD_CLEAR                    2   /**< Clear board for wipe clean */
/* @} */

/** NVRAM board information - returned via MRP **/
typedef struct
{
    UINT32                  boardStatus;        /**< Status, see defs       */
    NVRAM_BOARD_REVISION    revision;           /**< Revision information   */
    UINT32                  memorySize;         /**< Memory size            */
    UINT32                  memoryErrorCount;   /**< ECC error count        */
    UINT32                  batteryCount;       /**< Number of batteries on board */
    NVRAM_BATTERY_INFO      batteryInformation[NVRAM_BOARD_MAX_BATTERIES];
} NVRAM_BOARD_INFO;

/** Get Battery Status - Request **/
typedef struct
{
    UINT32                  commandCode;    /**< Get status command option  */
} MRMMCARDGETBATTERYSTATUS_REQ;


/** Get Battery Status - Response **/
typedef struct
{
    MR_HDR_RSP              header;     /**< MRP Response Header - 8 bytes  */
    NVRAM_BOARD_INFO        boardInfo;  /**< NVRAM board information        */
} MRMMCARDGETBATTERYSTATUS_RSP;
/* @} */

/**
**  @addtogroup MRFEPORTGO
**  Put Regular Ports on the FE Fabric since the Cache has completed init
**  @{
**/
/** Put Regular Ports on FE Fabric - Request **/
#define MRFEPORTGO_REQ      void

/** Put Regular Ports on FE Fabric - Response **/
#define MRFEPORTGO_RSP      MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRSETTDISCACHE
**  Set the Write Cache into a Temporary Disable state
**  @{
**/

/**
** @name Set Temporary Disable "user" constants
**
**       NOTE: DO NO USE TEMP_DISABLE_GENUSER it is here only as a
**             placeholder.
**  @{
**/
#define TEMP_DISABLE_GENUSER        0   /**< Generic User                   */
#define TEMP_DISABLE_MP             1   /**< Mirror partner change user     */
#define TEMP_DISABLE_CONFIG_PROP    2   /**< Config Propagation user        */
#define TEMP_DISABLE_BROADCAST      3   /**< Broadcast User                 */
#define TEMP_DISABLE_DEFRAG         4   /**< Defrag User                    */
#define TEMP_DISABLE_INACTIVATE     5   /**< Controller inactivation user   */
#define MAX_TEMP_DISABLE_USERS      256 /**< Max Number of Users in Array   */
/* @} */

/** Set the Write Cache into a Temporary Disable state - Request **/
typedef struct MRSETTDISCACHE_REQ
{
    UINT8   user;               /**< User                                   */
    UINT8   rsvd1[3];           /* RESERVED                                 */
} MRSETTDISCACHE_REQ;

/** Set the Write Cache into a Temporary Disable state - Response **/
#define MRSETTDISCACHE_RSP      MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRCLRTDISCACHE
**  Clear the Write Cache from a Temporary Disable state
**  @{
**/

/**
** @name Option for the Clearing of the Temporary Disable State
**  @{
**/
#define T_DISABLE_CLEAR_ONE 0   /**< Clear only one count for the User      */
#define T_DISABLE_CLEAR_ALL 1   /**< Clear all counts for the User          */
/* @} */

/** Clear the Write Cache from a Temporary Disable state - Request **/
typedef struct MRCLRTDISCACHE_REQ
{
    UINT8   user;               /**< User (must match Set User)             */
    UINT8   option;             /**< Option - Clear one or Clear All        */
    UINT8   rsvd2[2];           /* RESERVED                                 */
} MRCLRTDISCACHE_REQ;

/** Clear the Write Cache from a Temporary Disable state - Response **/
#define MRCLRTDISCACHE_RSP      MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRQTDISABLEDONE
**  Query if the Write Cache Temporary Disable is done
**  @{
**/

/**
** @name Status of the Temporary Disable constants
**  @{
**/
#define MQTD_DONE                   0   /**< WC Temp Disable Flush is Done  */
#define MQTD_VDISK_IN_ERROR         1   /**< As much flush is complete as
                                             can be but a VDisk is in Error
                                             State and could not be flushed */
#define MQTD_IN_PROGRESS            2   /**< Temporary Disable Flush is
                                             still in Progress              */
/* @} */

/** Status of the Temporary Disable Flush structure - returned via MRP **/
typedef struct
{
    UINT8                   status;     /**< Status of the Flush            */
    UINT8                   rsvd[3];    /* RESERVED                         */
} QUERY_TDIS_FLUSH_DONE_INFO;

/** Query if the Write Cache Temporary Disable is done - Request **/
#define MRQTDISABLEDONE_REQ    void

/** Query if the Write Cache Temporary Disable is done - Response **/
typedef struct
{
    MR_HDR_RSP                  header; /**< MRP Response Header - 8 bytes  */
    QUERY_TDIS_FLUSH_DONE_INFO  data;   /**< Response Data                  */
}  MRQTDISABLEDONE_RSP;
/* @} */

/**
**  @addtogroup MRMMTEST
**  Test driver for MM/NV Card
**  @{
**/
/**
**  @name MRMMTEST Options
**  @{
**/
#define MMTEST_ECC_SINGLE       0   /**< Inject a single bit ECC error      */
#define MMTEST_ECC_MULTI        1   /**< Inject a multi-bit ECC error       */
#define MMTEST_FAIL             2   /**< Fail the MM/NV card                */
#define MMTEST_WCSIG            3   /**< Corrupt the WC signature value     */
#define MMTEST_WCSIG_SN         4   /**< Corrupt the Ctrl S/N in WC sig     */
#define MMTEST_WCSIG_SEQNO      5   /**< Corrupt the Seq No in WC signature */
/* @} */

/** Test driver for MM/NV Card - Request **/
typedef struct
{
    UINT8           option;     /**< Test option                            */
    UINT8           rsvd[3];    /**< RESERVED                               */
    UINT32          offset;     /**< Offset to inject the error             */
} MRMMTEST_REQ;

/** Test driver for MM/NV Card - Response **/
#define MRMMTEST_RSP        MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MRFEFLERRORWOMP
**  Flush FE Cache w/o Mirror Partner due to Error To Mirror Info to the MP
**  @{
**/
/** Flush FE Cache Error w/o Mirror Partner - Request **/
#define MRFEFLERRORWOMP_REQ void

/** Flush FE Cache Error w/o Mirror Partner - Response **/
#define MRFEFLERRORWOMP_RSP MR_GENERIC_RSP
/* @} */

/**
**  @addtogroup MR_APOOL_STATUS
**  Status bits for apool.status value.
**  @{
**/
// apool/element status bits.
#define MR_APOOL_BAD_APOOL              0       // 0x01
#define MR_APOOL_NO_REMOTE_CONNECTION   1       // 0x02
#define MR_APOOL_FULL                   2       // 0x04
#define MR_APOOL_ELEMENT_FULL           2       // Overloaded on purpose.
#define MR_APOOL_FLUSH_ERROR            3       // 0x08
#define MR_APOOL_MOVER_PAUSED           4       // 0x10
#define MR_APOOL_FILL_ERROR             5       // 0x20
#define MR_APOOL_KILL_MOVER             6       // 0x40
#define MR_REJECT_PUTS_AP               7       // 0x80
#define MR_REJECT_PUTS_INIT             8       // 0x100
#define MR_REJECT_PUTS_ERR              9       // 0x200
#define MR_REJECT_PUTS_DIS              10      // 0x400
#define MR_REJECT_PUTS_OWN              11      // 0x800
/* @} */

/** PdiskQLTimeout - Request **/
typedef struct MRPDISKQLTIMEOUT_REQ
{
    UINT16  pid;            /**< Physical disk ID                           */
    UINT8   flag;           /**< Off = 0, On = 1                            */
} MRPDISKQLTIMEOUT_REQ;

#define MRPDISKQLTIMEOUT_RSP       MR_GENERIC_RSP

/*@}*/ /* CCB to FE MRP REQUEST and RESPONSE PACKETS */

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
#ifdef FRONTEND
extern UINT8 DEF_GetSessions(MR_PKT *pMRP);
extern UINT8 DEF_GetSessionsOnServer(MR_PKT *pMRP);
extern UINT8 DEF_GetIDDInfo(MR_PKT *pMRP);
#endif  /* FRONTEND */

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _MR_DEFS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/

