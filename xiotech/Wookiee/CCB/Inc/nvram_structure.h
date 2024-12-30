/* $Id: nvram_structure.h 143007 2010-06-22 14:48:58Z m4 $ */
/*============================================================================
** FILE NAME:       nvram_structure.h
** MODULE TITLE:    Header file for nvram_structure.c
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef NVRAM_STRUCTURE_H
#define NVRAM_STRUCTURE_H

#include "AsyncClient.h"
#include "ccb_hw.h"
#include "ccb_statistics.h"
#include "EL_Disaster.h"
#include "FW_Header.h"
#include "mach.h"
#include "nvram.h"
#include "quorum.h"
#include "rtc_structure.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
extern UINT32 XSSA_PERS_DATA_BASE;

#define XSSA_PERS_DATA_SIZE     (SIZE_64K)

extern UINT32 CCB_NVRAM_BASE;

#define CCB_NVRAM_SIZE          (SIZE_64K-8)    /* minus rtc registers */

#define TRACEEVENT_SIZE         ( SIZE_16K+SIZE_4K )    /* 20K */
#define SERIALDATA_SIZE         ( SIZE_16K+SIZE_4K )    /* 20K */
#define CALLSTACK_SIZE          ( SIZE_1K )

#define CCB_NVRAM_SCHEMA        (0x3152564E)    /* NVR1 in ascii */

// #define STACK_FRAME_SIZE        0x0040
// #define MAX_STACK_LEVELS        0x0010
#define I960_NUM_R_REGS         0x0010
#define I960_NUM_G_REGS         0x0010

#define NVR_RSVD_SNMP           0x064C
#define NVR_RSVD_I2C            0x0080
#define NVR_RSVD_IPMI           0x0040

typedef struct
{
    /* 8-bit registers */
    UINT8       statusGpodReg;  /* GPOD Output Data Register */
    UINT8       reserved0[15];  /* Align to 16 byte boundary */

    /* 16-bit registers */
    UINT16      nmiPatusrReg;   /* Primary ATU Status Register      */
    UINT16      nmiSatusrReg;   /* Secondary ATU Status Register    */

    UINT16      nmiPcrReg;      /* Primary Command Register         */
    UINT16      nmiBcrReg;      /* Bridge Control Register          */
    UINT16      nmiPsrReg;      /* Primary Status Register          */
    UINT16      nmiSsrReg;      /* Secondary Status Register        */
    UINT16      nmiSderReg;     /* Secondary Decode Enable Register */
    UINT16      nmiPatucmdReg;  /* Primary ATU Command Register     */
    UINT16      nmiSatucmdReg;  /* Secondary ATU Command Register   */
    UINT8       reserved1[14];  /* Align to 16 byte boundary        */

    /* 32-bit registers */
    UINT32      nmiNisrReg;     /* nmi NMI Interrupt Status Register            */

    UINT32      nmiPatuimrReg;  /* nmi Primary ATU Interrupt Mask Register      */
    UINT32      nmiSatuimrReg;  /* nmi Secondary ATU Interrupt Mask Register    */
    UINT32      nmiAtucrReg;    /* nmi ATU Configuration Register               */

    UINT32      nmiMcisrReg;    /* nmi bit 0, Memory Controller Interrupt Status Register   */
    UINT32      nmiPatuisrReg;  /* nmi bit 1, Primary ATU Interrupt Status Register         */
    UINT32      nmiSatuisrReg;  /* nmi bit 2, Secondary ATU Interrupt Status Register       */
    UINT32      nmiPbisrReg;    /* nmi bit 3, Primary Bridge Interrupt Status Register      */
    UINT32      nmiSbisrReg;    /* nmi bit 4, Secondary Bridge Interrupt Status Register    */
    UINT32      nmiCsr0Reg;     /* nmi bit 5, Channel Status Register           */
    UINT32      nmiCsr1Reg;     /* nmi bit 6, Channel Status Register           */
    UINT32      nmiCsr2Reg;     /* nmi bit 7, Channel Status Register           */
    UINT32      nmiIisrReg;     /* nmi bit 8 / 9 is external, Inbound Interrupt Status Register */
    UINT32      nmiAsrReg;      /* nmi bit 10, Accelerator Status Register      */
    UINT32      nmiBiuisrReg;   /* nmi bit 11, BIU Interrupt Status Register    */

    UINT32      nmiEccrReg;     /* ECC Control Register     */
    UINT32      nmiEctstReg;    /* ECC Test Register        */

    UINT32      nmiElog0Reg;    /* ECC Log Registers        */
    UINT32      nmiElog1Reg;    /* ECC Log Registers        */
    UINT32      nmiEcar0Reg;    /* ECC Address Registers    */
    UINT32      nmiEcar1Reg;    /* ECC Address Registers    */

    UINT8       reserved2[12];  /* Align to 16 byte boundary */
} ERRORTRAP_DATA_I960_TRACE_REGISTERS;

typedef struct
{
    UINT32      nmiFwFaultCnt;  /* totala NMI count          */

    UINT32      nmiBrkCnt;      /*                           */
    UINT32      nmiUnexpCnt;    /*                           */

    UINT32      nmiMceCnt;      /* bit0, MCU                 */
    UINT32      nmiPaeCnt;      /* bit1                      */
    UINT32      nmiSaeCnt;      /* bit2                      */
    UINT32      nmiPbieCnt;     /* bit3                      */

    UINT32      nmiSbeCnt;      /* bit4                      */
    UINT32      nmiDmac0eCnt;   /* bit5                      */
    UINT32      nmiDmac1eCnt;   /* bit6                      */
    UINT32      nmiDmac2eCnt;   /* bit7                      */

    UINT32      nmiMuiCnt;      /* bit8                      */
    UINT32      nmiEniCnt;      /* bit9                      */
    UINT32      nmiAaueCnt;     /* bit10                     */
    UINT32      nmiBiueCnt;     /* bit11, bus interface err. */

    UINT32      nmiEccSglCnt;   /* counts single ecc         */
    UINT32      nmiEccMulCnt;   /* counts multi ecc          */
    UINT32      nmiEccNotCnt;   /* counts non logged ecc     */

    UINT8       reserved[8];    /* Align to 16 byte boundary */
} ERRORTRAP_DATA_ERROR_COUNTERS;

typedef struct
{
    UINT32      rRegisters[I960_NUM_R_REGS];
    UINT32      gRegisters[I960_NUM_G_REGS];
} ERRORTRAP_DATA_CPU_REGISTERS;

typedef struct
{
    /* Timestamp of error */
    TIMESTAMP   timestamp;

    /* Important parts of the firmware header of code that's in error */
    FW_DATA     firmwareRevisionData;

    /* G and R registers at time of error */
    ERRORTRAP_DATA_CPU_REGISTERS cpuRegisters;

    /* MACH registers for Zion CCB at time of error */
    MACH        machRegisters;
    UINT8       machRegistersPad[8];    /* Pad to align back out to 16 bytes */

    /* Important registers inside the i960 we'd like to trace */
    ERRORTRAP_DATA_I960_TRACE_REGISTERS traceRegisters;

    /* Call stack at time of error */
    INT8        callStack[CALLSTACK_SIZE];

    /* CRC to validate snapshot data */
    UINT8       reserved1[12];  /* Pad to align back out to 16 bytes */
    UINT32      snapshotDataCRC;
} ERRORTRAP_DATA_ERROR_SNAPSHOT;

typedef struct
{
    /* Counters for tracking the quantity and type of errors */
    ERRORTRAP_DATA_ERROR_COUNTERS errorCounters;
    /* Snapshot of error */
    ERRORTRAP_DATA_ERROR_SNAPSHOT errorSnapshot;
    /* Boot code serial data */
    UINT32      serialDataIndex;
    INT8        serialPortData[SIZE_1K - sizeof(UINT32)];
    /* Reserved space (6K total struct size) */
    INT8        reservedSpace[SIZE_6K - sizeof(ERRORTRAP_DATA_ERROR_COUNTERS) - sizeof(ERRORTRAP_DATA_ERROR_SNAPSHOT) - SIZE_1K -       /* serialPortData */
                              sizeof      (RTC_STRUCTURE)];     /* See declaration in NVRAM_STRUCTURE */
}
ERRORTRAP_DATA_BOOT;

typedef struct
{
    /* Counters for tracking the quantity and type of errors */
    ERRORTRAP_DATA_ERROR_COUNTERS errorCounters;
    /* Snapshot of error */
    ERRORTRAP_DATA_ERROR_SNAPSHOT errorSnapshot;
    /* Runtime code trace data */
    INT8        traceEvData[TRACEEVENT_SIZE];
    /* Runtime code trace data */
    INT8        serialPortData[SERIALDATA_SIZE];
    /* Runtime CCB statistics (heap, link layer, ethernet etc) */
    CCB_STATS_STRUCTURE ccbStatistics;
    /* Runtime Heap Statistics */
    HEAP_STATS_IN_NVRAM heapStatsNV;            // No longer used -- but must remain in structure.
} ERRORTRAP_DATA_RUN;

typedef struct
{
    UINT32      ipcSequenceNumber;
    UINT16      stkDepthCnt;    /* See Note 1 below! */
    UINT16      reserved1;
    UINT32      backtraceNum;   /* incremented before NVRAM is stored to flash */
    UINT32      reserved2[29];
} MISC_NON_CRC_DATA;

/*
** Note 1: This variable (stkDepthCnt) is referenced by a hardcoded address
**         in kernel.as.  If the NVRAM moves or this variable moves, be sure
**         to update that section of code (the stack monitoring code.
*/

typedef struct _NVRAM_STRUCTURE
{
    /*
     ** Standard defines for the 128kB NVRAM device
     */
    /* 8 kB chunk */
    UINT32      schema;         /* Overall CCB NVRAM schema        */
    UINT32      reservedSpace0[3];      /* Pad back to 16-byte boundary    */
    QM_MASTER_CONFIG masterConfigRecord;        /* NVRAM copy of MasterConfig      */
    MISC_NON_CRC_DATA miscData; /* Misc non-crc'd data             */
    CONTROLLER_SETUP cntlSetup; /* NVRAM copy of ip configuration  */
    ASYNC_EVENT_QUEUE asyncQueue;       /* NVRAM copy of log queue         */
    UINT8       reservedSpace1[NVR_RSVD_SNMP];  /* (Used to be snmp addresses)     */
    UINT8       reservedoldi2c[NVR_RSVD_I2C];   /* (old NVRAM storage for I2C code) */
    DISASTER_DATA disasterData; /* Disaster data for elections     */
    UINT8       ipmiNVRAMData[NVR_RSVD_IPMI];   /* Used to be IPMI NVRAM space     */
    UINT8       reservedSpace2[SIZE_8K -        /* Space for future growth         */
                                           (4 * sizeof(UINT32)) -       /* schema and reserved0 */
                               sizeof      (QM_MASTER_CONFIG) -
                               sizeof(MISC_NON_CRC_DATA) -
                               sizeof(CONTROLLER_SETUP) -
                               sizeof(ASYNC_EVENT_QUEUE) -
                               NVR_RSVD_SNMP -
                               NVR_RSVD_I2C - sizeof(DISASTER_DATA) - NVR_RSVD_IPMI];

    /* 8 kB chunk */
    UINT8       reservedSpace3[SIZE_8K];
    /* 42 kB chunk */
    ERRORTRAP_DATA_RUN errortrapDataRun;
    /* 6 kB chunk - minus space dedicated to the RTC */
    ERRORTRAP_DATA_BOOT errortrapDataBoot;
} NVRAM_STRUCTURE;

/*****************************************************************************
** Public variables
*****************************************************************************/
#define NVRAMData (*nvramData)
extern NVRAM_STRUCTURE *nvramData;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* NVRAM_STRUCTURE_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
