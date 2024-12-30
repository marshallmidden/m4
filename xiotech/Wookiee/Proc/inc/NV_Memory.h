/* $Id: NV_Memory.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**  @file       NV_Memory.h
**
**  @brief      Non-Volatile Memory support
**              (Specifically:  MicroMemory MM-5425CN Card Access Routines)
**
**  Provides support for the MicroMemory card and its battery-backed memory.
**
**  Copyright 2004-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#ifndef __NV_MEMORY_H__
#define __NV_MEMORY_H__
#include <options.h>
#include <XIO_Types.h>
#include <XIO_Const.h>
#include <system.h>
#include <MR_Defs.h>

/*
******************************************************************************
**  Public definitions
******************************************************************************
*/

/*
** Compiler flags
*/
#define ERROR_STATUS_ENABLE 1
/*#define DIAGNOSTICS         1 */

/*
** NV Memory region definitions.
** NOTE:  See also the NV Directory in NV_Memory.c and system.h|.inc
*/
#define NV_ADMIN_END            (NV_ADMIN_START + NV_ADMIN_SIZE)

#define NV_P3BE_START           MICRO_MEM_BE_P3_START
#define NV_P3BE_SIZE            NVRAM_WRITE_RECORD_AREA_SIZE
#define NV_P3BE_END             (NV_P3BE_START + NV_P3BE_SIZE)

#define NV_P4BE_START           NV_P3BE_END
#define NV_P4BE_SIZE            NVRAM_WRITE_RECORD_AREA_SIZE
#define NV_P4BE_END             (NV_P4BE_START + NV_P4BE_SIZE)

#define NV_P4FE_START           NV_P4BE_END
#define NV_P4FE_SIZE            NVRAM_WRITE_RECORD_AREA_SIZE
#define NV_P4FE_END             (NV_P4FE_START + NV_P4FE_SIZE)

#define NV_P6BE_START           NV_P4FE_END
#define NV_P6BE_SIZE            NVRAM_P6_SIZE
#define NV_P6BE_END             (NV_P6BE_START + NV_P6BE_SIZE)

#define NV_P7BE_START           NV_P6BE_END
#define NV_P7BE_SIZE            SIZE_1K
#define NV_P7BE_END             (NV_P7BE_START + NV_P7BE_SIZE)

#define NV_FREE_START           NV_P7BE_END
#define NV_FREE_END             SIZE_16MEG

/*
** MicroMemory card definitions
*/
#define MMCARDID                0x5425
#define MMCARD_ERR_STATUS       0x3
#define MM_ECC_ANY_ERR          1
#define MULTI_BIT_ERROR         2
#define MM_LATENCY              0xF8

/*
******************************************************************************
**  Public structure definitions
******************************************************************************
*/

/*
** MicroMemory DMA Descriptor's Status / Control register
** - Hardware register layout matches this structure
*/
typedef union {
    UINT32 stsCtrl;
    /* Bit breakout */
    struct {
        UINT32 go : 1;                                              /* 0    */
        UINT32 direction : 1;                                       /* 1    */
        UINT32 chainEn : 1;                                         /* 2    */
        UINT32 dmaError : 1;                                        /* 3    */
        UINT32 semaphoreEn : 1;                                     /* 4    */
        UINT32 dmaCompIntEn : 1;                                    /* 5    */
        UINT32 chainCompIntEn : 1;                                  /* 6    */
        UINT32 sysErrIntEn : 1;                                     /* 7    */
        UINT32 parErrIntEn : 1;                                     /* 8    */
        UINT32 eccErrIntEn : 1;                                     /* 9    */
        UINT32 abtIntEn : 1;                                        /* 10   */

        UINT32 eccAnyErr : 1;                                       /* 11   */
        UINT32 eccMultiErr : 1;                                     /* 12   */
        UINT32 parErrRpt : 1;                                       /* 13   */
        UINT32 parErrDet : 1;                                       /* 14   */
        UINT32 sysErrRpt : 1;                                       /* 15   */
        UINT32 tgtAbtRec : 1;                                       /* 16   */
        UINT32 mstAbtRec : 1;                                       /* 17   */

        UINT32 dmaComp : 1;                                         /* 18   */
        UINT32 chainComp : 1;                                       /* 19   */

        UINT32 : 4; /* reserved */                                  /* 20-23 */
        UINT32 /*dmaWrtCmd*/ : 4;                                   /* 24-27 */
        UINT32 /*dmaRdCmd*/ : 4;                                    /* 28-31 */
    };
    /* Bit group breakout */
    struct {
        UINT32 ctrl: 11;                                            /*  0-10 */
        UINT32 errSts : 7;                                          /* 11-17 */
        UINT32 compSts : 2;                                         /* 18-19 */
        UINT32 : 4;                                                 /* 20-23 */
        UINT32 wrtCmd : 4;                                          /* 24-27 */
        UINT32 rdCmd : 4;                                           /* 28-31 */
    };
} MM_DMA_DESC_STSCTRL;

/*
** MicroMemory DMA Descriptor
** - Hardware register layout matches this structure
** - Total size is 0x30 bytes
** NOTE:  Definition ASSUMES 32-bit addressing for system (PCI) & local addr
*/
typedef struct MM_DMA_DESC
{
    UINT32 pciDataAddr;                 /* System address (PCI)             */
    UINT32 pciDataAddrHi;               /* (unused) upper 32-bits           */

    UINT32 localAddr;                   /* MicroMemory Card local address   */
    UINT32 localAddrHi;                 /* (unused) upper 32-bits           */

    union {                             /* Transfer Size (low 32-bits)      */
        UINT64 xferSizeReg;
        struct {
            UINT32 xferSize;
            UINT32 rsvd54h;
        };
    };

    UINT32 pciDescAddr;                 /* Next descriptor address (PCI)    */
    UINT32 pciDescAddrHi;               /* (unused) upper 32-bits           */

    UINT32 pciSemAddr;                  /* Semaphore address (PCI)          */
    UINT32 pciSemAddrHi;                /* (unused) upper 32-bits           */

    union {
        UINT64 stsCtrlReg;
        UINT32 stsCtrlLo;
        struct {
            MM_DMA_DESC_STSCTRL stsCtrl; /* Status & Control Bits           */
            UINT32 rsvd6Ch;
        };
    };
} MM_DMA_DESC;

/*
** MicroMemory 5425/5428 Registers - 256 byte range - entire struct is volatile
*/
typedef struct MM_5425CN
{
    /*
    ** Memory Controller Status Register - offset 00h
    */
    union {
        UINT64 mcStsReg;
        struct {
            UINT8 magicNum;
            UINT8 rsvd01h;
            UINT8 majorRev;
            UINT8 minorRev;
            union {
                UINT8 battSts;
                struct {
                    UINT8 disabled1 : 1;                            /* 0    */
                    UINT8 failure1 : 1;                             /* 1    */
                    UINT8 disabled2 : 1;                            /* 2    */
                    UINT8 failure2 : 1;                             /* 3    */
                    UINT8 : 4; /* not used */                       /* 4-7  */
                } battStsBits;
            };
            UINT8 rsvd05h;
            UINT8 rsvd06h;
            UINT8 memPres;
        };
    };

    /*
    ** Memory Controller Command Register - offset 08h
    */
    union {
        UINT64 mcCmdReg;
        struct {
            union {
                UINT8 ledCtrl;
                struct {
                    UINT8 memValid : 1;                 /* 0 (non-volatile) */
                    UINT8 memInit : 1;                  /* 1 (non-volatile) */
                    UINT8 okToRmv : 2;                  /* 2-3 (non-vol)    */
                    UINT8 fault : 2;                    /* 4-5 (non-vol)    */
                    UINT8 power : 2;                    /* 6-7              */
                } ledCtrlBits;
            };
            UINT8 shipPwr;
            UINT8 rsvd0Ah;
            union {
                UINT8 dmaOpt;
                struct {
                    UINT8 : 5; /* Reserved */
                    UINT8 descRdCmd : 2;
                    UINT8 : 1; /* Reserved */
                } dmaOptBits;
            };
            union {
                UINT8 errCtrl;
                struct {
                    UINT8 edcMode : 2;
                    UINT8 : 1; /* Reserved */
                    UINT8 clear : 1;
                    UINT8 : 1; /* Reserved */
                    UINT8 intMask : 1;
                    UINT8 : 2; /* Reserved */
                } errCtrlBits;
            };
            UINT8 errCnt;
            UINT8 errSts;
            UINT8 rsvd0Fh;
        };
    };

    /*
    ** First Error Data Log Register - offset 10h (16)
    ** First Error Information/Address Log Register - offset 18h (24)
    ** Last Error Data Log Register - offset 20h (32)
    ** Last Error Information/Address Log Register - offset 28h (40)
    ** Diagnostic Error Data Register - offset 30h (48)
    ** Diagnostic Error Information/Address Register - offset 38h (56)
    */
    struct {
        union {
            UINT64 data;
            struct {
                UINT32 dataLo;
                UINT32 dataHi;
            };
        };
        union {
            UINT64 infoAddrReg;
            struct {
                UINT32 addrLo;
                UINT8  addrHi;
                UINT8  rsvd1Dh;
                UINT8  synBits;
                UINT8  chkBits;
            };
        };
    } errFirst, errLast, errDiag;

    /*
    ** DMA Descriptor Registers - offsets 40h - 68h (64 - 104)
    */
    MM_DMA_DESC dmaDesc;

    /*
    ** RESERVED - offset 70h (112)
    */
    UINT64 rsvd70h;

    /*
    ** Window Map Register - offset 78h (120)
    */
    union {
        UINT64 windowMapReg;
        struct {
            UINT8  rsvd70ha;
            UINT8  rsvd79h;
            UINT8  rsvd7Ah;
            UINT8  windowMap;
            UINT32 rsvd7Ch;
        };
    };

    /*
    ** RESERVED - offsets 80h - 9Fh (128 - 159)
    */
    UINT64 rsvd80h;
    UINT64 rsvd88h;
    UINT64 rsvd90h;
    UINT64 rsvd98h;

    /*
    ** Battery Status Registers - offset A0h (160)
    */
    struct
    {
        UINT16 charge;
        UINT16 voltage;
    } battery[4];

    /*
    ** RESERVED - offsets B0h - FFh (176 - 255)
    */
    UINT64 rsvdB0h;
    UINT64 rsvdB8h;
    UINT64 rsvdC0h;
    UINT64 rsvdC8h;
    UINT64 rsvdD0h;
    UINT64 rsvdD8h;
    UINT64 rsvdE0h;
    UINT64 rsvdE8h;
    UINT64 rsvdF0h;
    UINT64 rsvdF8h;
} volatile MM_REGS;


/*
** Constants for pMM->battSts & battStsBits subfields
*/
#define MM_BATT_1_FAIL_MASK     0x02
#define MM_BATT_2_FAIL_MASK     0x08

/*
** Constants for pMM->ledCtrl & ledCtrlBits subfields
*/
#define MM_MEM_INIT             1       /* offset for shifting */

#define MM_LED_RMV              2       /* offset for shifting */
#define MM_LED_RMV_MASK         0x0C
#define MM_LED_RMV_OFF          0
#define MM_LED_RMV_ON           1

#define MM_LED_FLT              4       /* offset for shifting */
#define MM_LED_FLT_MASK         0x30
#define MM_LED_FLT_OFF          0
#define MM_LED_FLT_ON           1

#define MM_LED_PWR              6       /* offset for shifting */
#define MM_LED_PWR_MASK         0xC0
#define MM_LED_PWR_OFF          1 /* Note: polarity is reversed for the power */
#define MM_LED_PWR_ON           0 /*       LED vs. the remove & fault LEDs... */

#define MM_LED_FLASH_3_5HZ      2
#define MM_LED_FLASH_7HZ        3

#define MM_LED_CTRL_MASK        0xFC
#define MM_MEM_VALID_MASK       0x03

/*
** Constants for pMM->shipPwr
*/
#define MM_SHIP_PWR_DN          0x77

/*
** Constants for pMM->errCtrl & errCtrlBits subfields
*/
#define MM_EDC_CLR              0x08

/*
** Constants for pMM->battery[].voltage
*/
#define MM_BATT_END_MRK         0xFFFF
#define MM_BATT_DIS_JMP         0xFFFD
#define MM_BATT_DIS_SW          0xC
#define MM_BATT_DIS_MASK        0x000F

/*
** Constants for pMM->dmaDesc.stsCtrl
*/
#define MM_DMA_GO               0x1
#define MM_DMA_RD               0
#define MM_DMA_WRT              0x2
#define MM_DMA_CHN_EN           0x4
#define MM_DMA_ERR              0x8
#define MM_DMA_SEM_EN           0x10
#define MM_DMA_COMP_EN          0x20
#define MM_DMA_CHN_COMP_EN      0x40
#define MM_DMA_SERR_EN          0x80
#define MM_DMA_PERR_EN          0x100
#define MM_DMA_ECCERR_EN        0x200
#define MM_DMA_ALLINT_EN        0x7E0
#define MM_DMA_COMP             0x40000
#define MM_DMA_CHN_COMP         0x80000
#define MM_DMA_CLR_COMP         3       /* Use with pMM->dmaDesc.compSts    */

#define MM_DMA_PCI_MEM_RD       (1<<29 | 1<<30)
#define MM_DMA_PCI_MEM_RD_MULT  (1<<30 | 1<<31)
#define MM_DMA_PCI_MEM_RD_LINE  (MM_DMA_PCI_MEM_RD | 1<<31)
#define MM_DMA_PCI_MEM_WRT      (1<<24 | 1<<25 | 1<<26)
#define MM_DMA_PCI_MEM_WRT_INV  (MM_DMA_PCI_MEM_WRT | 1<<27)
#define DMA_CSR_SPACE           48

/*
** Generic DD List structure for input to NV DMA Request
*/
typedef struct NV_DMA_DD {
    struct NV_DMA_DD *pNextDD;      /* Link to next DD in list              */
    UINT32           sysAddr;       /* System (DRAM) address                */
    UINT32           nvAddr;        /* NV Memory address/offset             */
    UINT32           xferSize;      /* Transfer length (bytes)              */
    bool             wrtNotRd;      /* Direction:  0 = Read, 1 = Write      */
} NV_DMA_DD;

/*
** NV Memory DMA Response Packet
*/
typedef struct NV_DMA_RSP_PKT {
    MM_DMA_DESC_STSCTRL dmaStsCtrl;
    struct ILT          *pILT;
} NV_DMA_RSP_PKT;

/*
** NV Memory DMA Request - total size 40h
*/
typedef struct {
    MM_DMA_DESC         dmaDesc;    /* DMA descriptor (chain)               */
    void                (*pCompFn)(NV_DMA_RSP_PKT *pRsp); /* Comp fn ptr    */
                                    /* proto: void pCompFn(NV_DMA_RSP_PKT *);*/
    NV_DMA_RSP_PKT      *pRspPkt;   /* Response pkt pointer                 */
    UINT32              rsvd;       /* Reserved                             */
    MM_DMA_DESC_STSCTRL dmaStsCtrl; /* Status / Control field for DMA       */
} NV_DMA_REQ;

/*
** NV Memory DMA Request Queue (similar to QCB, but more strongly typed)
*/
typedef struct {
    NV_DMA_REQ *begin;
    NV_DMA_REQ * volatile in;       /* volatile ptr, updated in mult. tasks */
    NV_DMA_REQ * volatile out;      /* volatile ptr, updated in mult. tasks */
    NV_DMA_REQ *end;
    NV_DMA_REQ * volatile nextSub;  /* volatile ptr, updated in mult. tasks */
    NV_DMA_REQ * volatile active;   /* volatile ptr, updated in mult. tasks */
    UINT32     rsvd[2];
} NV_DMA_QUEUE;

#define NV_DMA_MAX_REQ          512
#define NV_DMA_QUEUE_SIZE       sizeof(NV_DMA_QUEUE) + (NV_DMA_MAX_REQ * sizeof(NV_DMA_REQ))

/*
** MicroMemory Information Structure - common status between FE & BE
*/
typedef struct
{
    volatile UINT32      status;
    volatile UINT32      errCount;
    NVRAM_BOARD_REVISION revision;
    UINT32               memSize;
    UINT32               battCount;
    volatile UINT8       *pMMemory;
    MM_REGS              *pMM;
    UINT32               id;
    NV_DMA_QUEUE         *pDMAQueue;
    UINT32               rsvd3Words[2];
} MM_INFO;

/*
** MM_INFO status settings - see also MR_Defs.h
*/
#define NV_STS_DIAG 0x80000000      /* Set upper bit */

/*
** Generic 64-bit "register" structure, with 32-bit hi/lo halves
*/
typedef union {
    UINT64      dw;
    struct {
        UINT32  lo;
        UINT32  hi;
    };
} REG64;

/*
** Structure defining the layout of the NV Administrative region in NV Memory
*/
#define NV_ADMIN_TOC_NUM_ENTRIES 16 /* Number of TOC entries allowed        */
typedef struct NV_ADMIN_TOC_ENTRY {
    UINT32  size;
    UINT32  offset;
} NV_ADMIN_TOC_ENTRY;

typedef struct {
    UINT32  cSerial;                /* Controller S/N (from FICB)           */
    UINT8   version;                /* Version/compatibility of NV_ADMIN    */
    UINT8   rsvd1;                  /* Rsvd fields...                       */
    UINT16  rsvd2;
    UINT32  rsvd3;
    UINT32  rsvd4;

    NV_ADMIN_TOC_ENTRY toc[NV_ADMIN_TOC_NUM_ENTRIES]; /* Table of contents  */

} NV_ADMIN;

#define NV_MGR_NUM_ENTRIES 16
typedef struct {
    UINT32      offset;             /* Starting offset of memory region     */
    UINT32      size;               /* Size of region (bytes)               */
    UINT32      name;               /* 4 character region name              */
    union {
        UINT8   flags;              /* Flags byte                           */
        struct {
            UINT8   allocNotFree : 1;   /* 1 = allocated, 0 = free          */
        };
    };
    UINT8       rsvd[3];
} NV_MGR;

#define NV_ADMIN_VERSION        0   /* Current Admin structure version      */

#define NV_ADMIN_TOC_NV_DIR     0   /* TOC entry for the NV Directory       */
#define NV_ADMIN_TOC_NV_MGR     1   /* Memory Mgr area, for region >16MB    */
#define NV_ADMIN_TOC_FREE       2   /* First unallocated TOC entry          */

#define NV_ADMIN_OFFSET_FIRST_ENTRY sizeof(NV_ADMIN)

/*
******************************************************************************
**  Public function prototypes
******************************************************************************
*/
extern UINT32  MM_ProcessMRP(UINT32 command, MRMMCARDGETBATTERYSTATUS_RSP *pMMRsp);
extern void    MM_TestTaskStart(MRMMTEST_REQ* pReq);

extern INT32   NV_DMARequest(NV_DMA_DD *pDDList,
                             void (*pCompFn)(NV_DMA_RSP_PKT *pRsp), struct ILT *pILT);
extern UINT32  NV_GetStatus(void);
extern void    NV_MemSet(UINT32 startAddr, UINT32 endAddr, const UINT64 *pattern);
extern UINT32  NV_ProcessMMInfo(MR_PKT *pMRP);
extern void    NV_SendMMInfoCmplt(UINT32 rc, struct ILT *pILT,
                                  MR_PKT *pMRP, UINT32 parm);
extern void    NV_ZeroMemory(UINT32 startAddr, UINT32 endAddr);

#ifdef DIAGNOSTICS
#ifdef ERROR_STATUS_ENABLE
extern UINT64  FirstErrorDataLog(void);
extern UINT64  FirstErrorAddrLog(void);
extern UINT64  FirstErrorAddr(void);
extern UINT8   FirstErrorSyndrome(void);
extern UINT8   FirstErrorCheckBits(void);
extern UINT64  LastErrorDataLog(void);
extern UINT64  LastErrorAddrLog(void);
extern UINT64  LastErrorAddr(void);
extern UINT8   LastErrorSyndrome(void);
extern UINT8   LastErrorCheckBits(void);
#endif /* ERROR_STATUS_ENABLE */
#endif /* DIAGNOSTICS */

#endif  /* __NV_MEMORY_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
