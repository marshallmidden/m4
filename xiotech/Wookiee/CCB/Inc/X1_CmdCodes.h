/* $Id: X1_CmdCodes.h 122127 2010-01-06 14:04:36Z m4 $*/
/*===========================================================================
** FILE NAME:       X1_CmdCodes.h
** MODULE TITLE:    Xiotech Storage Platform Packet Command Codes
**
** DESCRIPTION:     Xiotech Storage Platform packet types.  This module
**                  is shared with Magnitude MMC code and needs to be
**                  kept in sync.
**
** Copyright (c) 2002-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _X1_CMD_CODES_H
#define _X1_CMD_CODES_H

#ifdef __cplusplus
#pragma pack(push, 1)
#endif

/*
** ----- Request packet codes -----
*/
#define X1PKT_GET_DLINK                     0x10
#define X1PKT_GET_DLOCK_INFO                0x11
#define X1PKT_GET_SU_LIST                   0x12
#define X1PKT_GET_SU_LUN_INFO               0x13

#define X1PKT_GET_ENVIRON                   0x20
#define X1PKT_GET_PMAP                      0x21
#define X1PKT_GET_RMAP                      0x22
#define X1PKT_GET_VMAP                      0x23
#define X1PKT_GET_HMAP                      0x24
#define X1PKT_GET_PSTATS                    0x25
#define X1PKT_GET_BSTATS                    0x26
#define X1PKT_GET_RSTATS                    0x27
#define X1PKT_GET_VSTATS                    0x28
#define X1PKT_GET_PINFO                     0x29
#define X1PKT_GET_RINFO                     0x2A
#define X1PKT_GET_VINFO                     0x2B
#define X1PKT_GET_HINFO                     0x2C
#define X1PKT_GET_ISALIVE                   0x2D
#define X1PKT_GET_PSTATE                    0x2F

#define X1PKT_GET_STATES                    0x30
#define X1PKT_GET_DNAME                     0x31
#define X1PKT_GET_CPU_LOADS                 0x32
#define X1PKT_GET_FREE                      0x35
#define X1PKT_WHO_INCOPY                    0x38
#define X1PKT_GET_COPY_INFO                 0x39
#define X1PKT_SERVER_MAP                    0x3A
#define X1PKT_GET_MASK_INFO                 0x3B
#define X1PKT_GET_LUNS_INFO                 0x3C
#define X1PKT_GET_COPY_STATUS               0x3D
#define X1PKT_GET_MIRROR_STATUS             0x3E

#define X1PKT_PCHANGED                      0x40
#define X1PKT_RCHANGED                      0x41
#define X1PKT_VCHANGED                      0x42
#define X1PKT_HCHANGED                      0x43
#define X1PKT_GET_CONFIG                    0x44
#define X1PKT_VCG_CHANGED                   0x45
#define X1PKT_ACHANGED                      0x46
#define X1PKT_ZCHANGED                      0x47
#define X1PKT_READ_MEM                      0x48
#define X1PKT_READ_DPRAM                    0x49
#define X1PKT_VLINKED_TO                    0x4A
#define X1PKT_GET_PERF                      0x4B
#define X1PKT_ACCT_SET                      0x4C
#define X1PKT_ACCT_GET                      0x4D
#define X1PKT_ACCT_SELECT                   0x4E
#define X1PKT_ACCT_LOGIN                    0x4F

#define X1RPKT_LOG_ENTRY                    0x50
#define X1PKT_SET_LOG                       0x51
#define X1RPKT_CONFIG_VDISK_ACK             0x57
#define X1PKT_CONFIG_VDISK                  0x58
#define X1PKT_CONFIG_HAB                    0x59
#define X1PKT_PROBE                         0x5A
#define X1RPKT_PROBE                        0x5A
#define X1PKT_DISCONNECT                    0x5C
#define X1PKT_LOG_ENTRY                     0x5D
#define X1PKT_SECURE_LOGIN                  0x5E
#define X1PKT_GET_MS                        0x5F

#define X1PKT_GET_VCG_INFO                  0x60
#define X1PKT_GET_VERSION_INFO              0x61
#define X1PKT_GET_BAY_MAP                   0x62
#define X1PKT_GET_BAY_INFO                  0x63
#define X1PKT_GET_VPORT_MAP                 0x64
#define X1PKT_GET_VPORT_INFO                0x65
#define X1PKT_UNUSED_66                     0x66
#define X1PKT_GET_BE_LOOP_INFO              0x67
#define X1PKT_GET_WORKSET_INFO              0x68

/*
** The commands 69 and 6A can be reused by any application.
*/
#define X1PKT_UNUSED_69                     0x69
#define X1PKT_UNUSED_6A                     0x6A

#define X1PKT_PUT_DEV_CONFIG                0x6B
#define X1PKT_GET_DEV_CONFIG                0x6C
#define X1PKT_GET_SERVER_STATS              0x6D
#define X1PKT_GET_HAB_STATS                 0x6E
#define X1PKT_GET_DEFRAG_STATUS             0x6F
#define X1PKT_GET_RESYNC_INFO               0x70
#define X1PKT_RESYNC_CONTROL                0x71
#define X1PKT_LICENSE_CONFIG                0x72
#define X1PKT_GET_MIRROR_IO_STATUS          0x73

#define X1PKT_BIGFOOT_CMD                   0xBF
#define X1RPKT_BIGFOOT_CMD                  0xBF

/*
** ----- Response packet codes -----
*/
#define X1RPKT_GET_DLINK                    0x90
#define X1RPKT_GET_DLOCK_INFO               0x91
#define X1PKT_REPLY_SU_CNT                  0x9A
#define X1PKT_REPLY_SU_ITEM                 0x9B
#define X1PKT_REPLY_LUN_CNT                 0x9C
#define X1PKT_REPLY_LUN_ITEM                0x9D

#define X1RPKT_GET_ENVIRON                  0xA0
#define X1RPKT_GET_PMAP                     0xA1
#define X1RPKT_GET_RMAP                     0xA2
#define X1RPKT_GET_VMAP                     0xA3
#define X1RPKT_GET_HMAP                     0xA4
#define X1RPKT_GET_PSTATS                   0xA5
#define X1RPKT_GET_BSTATS                   0xA6
#define X1RPKT_GET_RSTATS                   0xA7
#define X1RPKT_GET_VSTATS                   0xA8
#define X1RPKT_GET_PINFO                    0xA9
#define X1RPKT_GET_RINFO                    0xAA
#define X1RPKT_GET_VINFO                    0xAB
#define X1RPKT_GET_HINFO                    0xAC
#define X1RPKT_GET_ISALIVE                  0xAD
#define X1RPKT_GET_RAIDPER                  0xAE
#define X1RPKT_GET_PSTATE                   0xAF

#define X1RPKT_GET_STATES                   0xB0
#define X1RPKT_GET_DNAME                    0xB1
#define X1RPKT_GET_CPU_LOADS                0xB2
#define X1RPKT_GET_FREE                     0xB5
#define X1RPKT_WHO_INCOPY                   0xB8
#define X1RPKT_GET_COPY_INFO                0xB9
#define X1RPKT_SERVER_MAP                   0xBA
#define X1RPKT_GET_MASK_INFO                0xBB
#define X1RPKT_GET_LUNS_INFO                0xBC
#define X1RPKT_GET_COPY_STATUS              0xBD
#define X1RPKT_GET_MIRROR_STATUS            0xBE

#define X1RPKT_GET_CONFIG                   0xC4
#define X1RPKT_READ_MEM                     0xC8
#define X1RPKT_READ_DPRAM                   0xC9
#define X1RPKT_VLINKED_TO                   0xCA
#define X1RPKT_GET_PERF                     0xCB
#define X1RPKT_ACCT_SET                     0xCC
#define X1RPKT_ACCT_GET                     0xCD
#define X1RPKT_ACCT_SELECT                  0xCE
#define X1RPKT_ACCT_LOGIN                   0xCF

#define X1RPKT_SET_LOG                      0xD1
#define X1RPKT_CONFIG_VDISK                 0xD8
#define X1RPKT_CONFIG_HAB                   0xD9
#define X1RPKT_SECURE_LOGIN                 0xDE
#define X1RPKT_GET_MS                       0xDF

#define X1RPKT_VCG_INFO                     0xE0
#define X1RPKT_GET_VERSION_INFO             0xE1
#define X1RPKT_GET_BAY_MAP                  0xE2
#define X1RPKT_GET_BAY_INFO                 0xE3
#define X1RPKT_GET_VPORT_MAP                0xE4
#define X1RPKT_GET_VPORT_INFO               0xE5
#define X1RPKT_UNUSED_E6                    0xE6
#define X1RPKT_GET_BE_LOOP_INFO             0xE7
#define X1RPKT_GET_WORKSET_INFO             0xE8

/*
** The commands E9 and EA can be reused by any application.
*/
#define X1RPKT_UNUSED_E9                    0xE9
#define X1RPKT_UNUSED_EA                    0xEA
#define X1RPKT_PUT_DEV_CONFIG               0xEB
#define X1RPKT_GET_DEV_CONFIG               0xEC
#define X1RPKT_GET_SERVER_STATS             0xED
#define X1RPKT_GET_HAB_STATS                0xEE
#define X1RPKT_GET_DEFRAG_STATUS            0xEF
#define X1RPKT_GET_RESYNC_INFO              0xF0
#define X1RPKT_RESYNC_CONTROL               0xF1
#define X1RPKT_LICENSE_CONFIG               0xF2
#define X1RPKT_GET_MIRROR_IO_STATUS         0xF3

/*
** Operation codes used with X1PKT_CONFIG_VDISK
*/
#define CFG_VDISK_DELETE                    0x01

#define CFG_VDISK_ADD_RAID_0                0x10
#define CFG_VDISK_ADD_RAID_5                0x11
#define CFG_VDISK_ADD_RAID_10               0x12

#define CFG_VDISK_EXP_RAID_0                0x20
#define CFG_VDISK_EXP_RAID_5                0x21
#define CFG_VDISK_EXP_RAID_10               0x22

#define CFG_VDISK_VDISK_MOVE                0x80
#define CFG_VDISK_VDISK_COPY                0x81
#define CFG_VDISK_VDISK_SWAP                0x82
#define CFG_VDISK_VDISK_MIRROR              0x83
#define CFG_VDISK_VDISK_BREAK_MIRROR        0x84
#define CFG_VDISK_VDISK_COPY_PAUSE          0x85
#define CFG_VDISK_VDISK_COPY_RESUME         0x86
#define CFG_VDISK_VDISK_COPY_ABORT          0x87

#define CFG_VDISK_SET_ATTRIBUTE             0x90
#define CFG_VDISK_SET_LOCK                  0x91

/*
** The commands 92 can be reused by any application.
*/
#define CFG_UNUSED                          0x92

#define CFG_VDISK_ERASE_VDISK               0x99

#define CFG_VDISK_SET_SERVER_NAME           0xA0
#define CFG_VDISK_SET_MASK                  0xA1
#define CFG_VDISK_SET_LUN                   0xA2
#define CFG_VDISK_SET_DEFMODE               0xA3
#define CFG_VDISK_VDISK_SET_NAME            0xA4
#define CFG_VDISK_MAG_RSVD_1                0xA5
#define CFG_VDISK_VDISK_SET_CACHE_OFF       0xA6
#define CFG_VDISK_SELECT_HAB_FOR_SERVER     0xA7
#define CFG_VDISK_VLINK_BREAK               0xA8
#define CFG_VDISK_VLINK_CREATE              0xA9
#define CFG_VDISK_VDISK_SET_CACHE_ON        0xAA
#define CFG_VDISK_SELECT_TARGET             0xAB
#define CFG_VDISK_ASSIGN_VBLOCK             0xAC
#define CFG_VDISK_SET_WORKSET_NAME          0xAD
#define CFG_VDISK_SET_DEFAULT_VPORT         0xAE
#define CFG_VDISK_ADD_SERVER_TO_WORKSET     0xAF

#define CFG_VDISK_PDISK_LABEL               0xB0
#define CFG_VDISK_PDISK_SPINDOWN            0xB1
#define CFG_VDISK_PDISK_DEFRAG              0xB2
#define CFG_VDISK_PDISK_SCRUB               0xB3
#define CFG_VDISK_PDISK_FAIL                0xB4
#define CFG_VDISK_PDISK_UNFAIL              0xB5
#define CFG_VDISK_PDISK_BEACON              0xB6
#define CFG_VDISK_PDISK_DELETE              0xB7
#define CFG_VDISK_LOG_ACKNOWLEDGE           0xB8
#define CFG_VDISK_DELETE_SERVER             0xB9
#define CFG_VDISK_OP_SET_PRIORITY           0xBA
#define CFG_VDISK_RAID_RECOVER              0xBB
#define CFG_VDISK_SET_GLOBAL_CACHE_ON       0xBC
#define CFG_VDISK_SET_GLOBAL_CACHE_OFF      0xBD
#define CFG_VDISK_PDISK_AUTOFAILBACK        0xBE

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _X1_CMD_CODES_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
