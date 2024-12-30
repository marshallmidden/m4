/* $Id: ses.h 160556 2013-02-07 22:51:41Z marshall_midden $ */
/**
******************************************************************************
**
**  @file   ses.h
**
**  @brief  SCSI Enclosure Services
**
**  Copyright (c) 2008-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _SES_H_
#define _SES_H_

#include "ipc_packets.h"
#include "MR_Defs.h"
#include "SES_Structs.h"
#include "XIO_Types.h"
#include "mutex.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public variables
*****************************************************************************/
extern UINT16 DiscoveringSES;
extern MUTEX sesMutex;

#if defined(MODEL_7000) || defined(MODEL_4700)

typedef struct _power_supply_info
{
    UINT32      status;         // powersupply_status_details           // 704 and 804
    UINT8       general_status; // powersupply_status                   // 708 and 808
    UINT8       temp;           // powersupply_temperature              // 709 and 809
    UINT8       beacon;         // powersupply_beacon                   // 710 and 810
    UINT8       pad;                                                    // 711 and 811
    UINT8       model[32];      // powersupply_model.16                 // 712 and 812
    UINT8       serial_number[32];      // powersupply_serial_number.12 // 744 and 844
    UINT8       part_number[16];        // powersupply_part_number.16   // 776 and 876
    UINT16      fan1_status;    // powersupply_fan1_status              // 792 and 892
    UINT16      fan2_status;    // powersupply_fan2_status              // 794 and 894
    UINT32      fan1_speed;     // powersupply_fan1_speed               // 796 and 896
    UINT32      fan2_speed;     // powersupply_fan2_speed               // 800 and 900
} __attribute__ ((packed)) POWER_SUPPLY_INFO_t;                         // ends just before 904

typedef struct _battery_info
{
    UINT32      battery_status; // battery_status_details               // 496 and 600
    UINT8       general_status; // battery_status                       // 500 and 604
    UINT8       beacon;         // battery_beacon                       // 501 and 605
    UINT8       pad;            //                                      // 502 and 606
    UINT8       charger_status; // battery_charger_state                // 503 and 607
    UINT32      general_charger_status; // battery_charger_state_details// 504 and 608
    UINT8       model[32];      // battery_model.16                     // 508 and 612
    UINT8       serial_number[32];      // battery_serial_number.12     // 540 and 644
    UINT8       part_number[16];        // battery_part_number.16       // 572 and 676
    UINT16      capacity;       //                                      // 588 and 692
    UINT16      max_charge;     // battery_max_charge                   // 590 and 694
    UINT16      max_capacity;   // battery_max_charge_capacity          // 592 and 696
    UINT16      min_holdtime;   // battery_min_holdup_time              // 594 and 698
    UINT32      remaining_charge;       // battery_remaining_charge     // 596 and 700
} __attribute__ ((packed)) BATTERY_INFO_t;                              // ends just before 704

typedef struct _datapac_info
{
    UINT32      status;         // datapac_status_details               // 904 and 1012 in packet   0x0388 0x03F4
    UINT8       general_status; // datapac_status                       // 908 and 1016             0x038C 0x03F8
    UINT8       datapac_number; //                                      // 909 and 1017             0x038D 0x03F9
    UINT8       health;         // datapac_health                       // 910 and 1018             0x038E 0x03FA
    UINT8       temp;           // datapac_temperature                  // 911 and 1019             0x038F 0x03FB
    UINT8       type;           // datapac_type                         // 912 and 1020             0x0390 0x03FC
    UINT8       beacon;         // datapac_beacon                       // 913 and 1021             0x0391 0x03FD
    UINT8       pad[2];                                                 // 914 and 1022             0x0392 0x03FE
    UINT8       model[32];      // datapac_model.16                     // 916 and 1024             0x0394 0x0400
    UINT8       serial_number[32];      // datapac_serial_number.12     // 948 and 1056             0x03B4 0x0420
    UINT8       part_number[16];        // datapac_part_number.16       // 980 and 1088             0x03D4 0x0440
    UINT8       fw_version[8];  // datapac_fw_version.16                // 998 and 1104             0x03E6 0x0450
    UINT32      number_drives;  //                                      // 1004 and 1112            0x03EC 0x0458
    UINT32      number_bad_drives;      //                              // 1008 and 1116            0x03F0 0x045C
} __attribute__ ((packed)) DATAPAC_INFO_t;                              // ends just before 1120 (0x0460)

typedef struct MRC_INFO_t
{
    UINT32      status;         // controller_status_details -- modified            // 192 and 344
    UINT8       general_status; // controller_status -- modified                    // 196 and 348
    UINT8       pad1[3];        //                                                  // 197 and 349
    UINT8       model[32];      // controller_model                                 // 200 and 352
    UINT8       serial_number[32];      // controller_serial_number                 // 232 and 384
    UINT8       part_number[16];        // controller_part_number                   // 264 and 416
    UINT8       hw_version[32]; // controller_hw_version                            // 280 and 432
    UINT32      ip_address;     // ip                                               // 312 and 464
    UINT32      subnet_mask;    // gateway                                          // 316 and 468
    UINT32      gateway;        // subnet_mask                                      // 320 and 472
    UINT8       temp;           // controller_temperature                           // 324 and 476
    UINT8       fc_port_status; // controller_fc_port_status                        // 325 and 477
    UINT8       fc_port_speed;  // controller_fc_port_speed                         // 326 and 478
    UINT8       fc_port_set;    // controller_fc_port_speed_setting                 // 327 and 479
    UINT32      wwn[2];         // controller_wwn                                   // 328 and 480
    UINT8       mac_address[6]; // controller_mac_address                           // 336 and 488
    UINT8       beacon;         // controller_beacon                                // 342 and 494
    UINT8       pad;            //                                                  // 343 and 495
} __attribute__ ((packed)) MRC_INFO_t;                                              // ends just before 496

typedef struct _DRIVE_INFO
{
    UINT32      status;
    UINT8       spot;
    UINT8       reserved;
    UINT16      temperature;
    UINT32      block_size;
    UINT32      block_count;
    UINT8       product_id[16];
    UINT8       serial_number[8];
    UINT32      wwn[2];
    UINT8       revision[4];
    UINT32      iops;
    UINT32      queue_depth;
    UINT32      total_write_iops;
    UINT32      total_read_iops;
    UINT32      avg_req_size;
    UINT32      pd_tf;          // average seek distance
    UINT32      pd_path_count;  // Number of paths to the drive
    UINT32      link_fail_count;
    UINT32      loss_sync_count;
    UINT32      invalid_word_count;
    UINT32      invalid_crc_count_a;
    UINT32      lip_f7_init_count_a;
    UINT32      lip_f7_recv_count_a;
    UINT32      lip_f8_init_count_a;
    UINT32      lip_f8_recv_count_a;
    UINT32      lip_f7_init_count_b;
    UINT32      lip_f7_recv_count_b;
    UINT32      lip_f8_init_count_b;
    UINT32      lip_f8_recv_count_b;
    UINT32      writes_corrected_no_delay;
    UINT32      writes_corrected_delay;
    UINT32      writes_corrected_retry;
    UINT32      writes_corrected_count;
    UINT32      reads_corrected_no_delay;
    UINT32      reads_corrected_delay;
    UINT32      reads_corrected_retry;
    UINT32      reads_corrected_count;
    UINT32      verify_corrected_no_delay;
    UINT32      verify_corrected_delay;
    UINT32      verify_corrected_retry;
    UINT32      verify_correctved_count;
} __attribute__ ((packed)) DRIVE_INFO_t;

typedef struct LOG_ISE_PAGE
{
    UINT8       PageCode;       /* Page code (0x00)         */                      // 0
    UINT8       Rsvd;           /* Reserved                 */                      // 1
    UINT16      Length;         /* Page length              */                      // 2
    UINT32      version;        /* Recently added version field */                  // 4
    UINT32      status;         // chassis_status_details                           // 8
    UINT32      spare_level;    // spare_level / 10                                 // 12
    UINT32      wwn[2];         // chassis_wwn                                      // 16
    UINT32      temp;           // chassis_temperature_sensor                       // 24
    UINT32      hw_id;          // iws_ise_id                                       // 28
    UINT8       vendor[8];      // chassis_vendor                                   // 32
    UINT8       serial_number[32];      // chassis_serial_number                    // 40
    UINT8       part_number[16];        // chassis_part_number                      // 72
    UINT8       product_version[4];     // chassis_product_version                  // 88
    UINT8       fw_version[32]; // (MRC firmware version, both of them the same)    // 92
    UINT64      uptime;         // chassis_uptime                                   // 124
    UINT32      perf_valid;     // chassis_performance_valid                        // 132
    UINT32      iops_total;     // chassis_total_iops                               // 136
    UINT32      iops_read;      // chassis_read_iops                                // 140
    UINT32      iops_write;     // chassis_write_iops                               // 144
    UINT32      kbps_total;     // chassis_total_kbps                               // 148
    UINT32      kbps_read;      // chassis_read_kbps                                // 152
    UINT32      kbps_write;     // chassis_write_kbps                               // 156
    UINT32      read_latency;   // chassis_read_latency                             // 160
    UINT32      write_latency;  // chassis_write_latency                            // 164
    UINT32      queue_depth;    // chassis_queue_depth                              // 168
    UINT32      read_percentage;        // chassis_read_percent                     // 172
    UINT32      avg_bytes;      // chassis_avg_bytes_transferred                    // 176
    UINT8       num_mrcs;       //                                                  // 180
    UINT8       num_data_pacs;  //                                                  // 181
    UINT8       beacon;         // chassis_beacon                                   // 182
    UINT8       general_status; // chassis_status                                   // 183
    UINT32      num_events;     //                                                  // 184
    UINT8       cache_dirty;    //                                                  // 188
    UINT8       pad[3];                                                             // 189,190,191
    MRC_INFO_t  mrc[2];         // ctrlr[2]                                         // 192  (304 in size)   0x00C0
    BATTERY_INFO_t battery[2];  // battery[2]                                       // 496  (208 in size)   0x01F0
    POWER_SUPPLY_INFO_t ps[2];  // powersupply[2]                                   // 704  (200 in size)   0x02C0
    DATAPAC_INFO_t data_pac[2]; // datapac[2]                                       // 904  (216 in size)   0x0388
    DRIVE_INFO_t drives[40];    //                                                  // 1120 (max of 7040 in size);
} __attribute__ ((packed)) LOG_ISE_PAGE, *PLOG_ISE_PAGE;                            // 8160 for total length.

#endif /* MODEL_7000 || MODEL_4700 */

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void InitSES(TASK_PARMS *parms);

#if defined(MODEL_3000) || defined(MODEL_7400)
extern void DiscoverSES_StartTask(void);
extern void DiscoverSES(void);
#else  /* MODEL_3000 || MODEL_7400 */
extern void DiscoverSES(TASK_PARMS *parms);
extern void IseUpdateIP(UINT16 bayid, IP_ADDRESS ip1, IP_ADDRESS ip2, UINT64 wwn);
#endif /* MODEL_3000 || MODEL_7400 */
extern void SESAlarmCtrl(PSES_DEVICE enclosure, UINT8 setting);
extern void SESBypassCtrl(UINT16 searchSES, UINT8 slot, UINT8 setting);
extern PSES_DEVICE GetSESList(void);
extern void *GetSESPageByWWN(UINT16 page, UINT64 wwn, UINT16 lun);
extern INT32 SendSESPageByWWN(UINT64 wwn, UINT16 lun, void *data);
extern UINT32 GetDeviceMap(PSES_DEVICE pSES, SES_WWN_TO_SES ** pDevMap);
extern INT32 GetSESControlElement(UINT16 pid, UINT8 type, UINT8,
                                  SES_ELEM_CTRL * pElement);
extern void EnqueueToLedControlQueue(IPC_LED_CHANGE *pEvent);
extern IPC_PACKET *HandleLedControlRequest(IPC_PACKET *ptrPacket);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _SES_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
