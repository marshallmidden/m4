/* $Id: scsi.h 161041 2013-05-08 15:16:49Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       scsi.h
**
**  @brief      SCSI Information
**
**  To provide a common definition of SCSI sense data and other SCSI
**  constants.
**
**  Copyright (c) 1996-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _SCSI_H_
#define _SCSI_H_

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
/* SCSI CDB codes */
#define    SCC_TESTUNR    0x00      /* 000 - TEST UNIT READY */
#define    SCC_REWIND     0x01      /* 001 - RECALIBRATE or REWIND */
#define    SCC_REQSENSE   0x03      /* 003 - REQUEST SENSE */
#define    SCC_FORMAT     0x04      /* 004 - FORMAT UNIT */
#define    SCC_READBLKLMT 0x05      /* 005 - READ BLOCK LIMITS */
#define    SCC_REASSIGNBK 0x07      /* 007 - REASSIGN BLOCKS */
#define    SCC_READ_6     0x08      /* 008 - READ (6) */
#define    SCC_WRITE_6    0x0A      /* 010 - WRITE (6) */
#define    SCC_SEEK_6     0x0B      /* 011 - SEEK (6) */
#define    SCC_READ_RVS_6 0x0F      /* 015 - READ REVERSE (6) */
#define    SCC_WRITEFLM_6 0x10      /* 016 - WRITE FILEMARKS (6) */
#define    SCC_SPACE_6    0x11      /* 017 - SPACE (6) */
#define    SCC_INQUIRY    0x12      /* 018 - INQUIRY */
#define    SCC_VERIFY_6   0x13      /* 019 - VERIFY (6) */
#define    SCC_RCVRBUFDAT 0x14      /* 020 - RECOVER BUFFERED DATA */
#define    SCC_MODESLC    0x15      /* 021 - MODE SELECT */
#define    SCC_RESERVE_6  0x16      /* 022 - RESERVE (6) */
#define    SCC_RELEASE_6  0x17      /* 023 - RELEASE (6) */
#define    SCC_COPY       0x18      /* 024 - COPY */
#define    SCC_ERASE_6    0x19      /* 025 - ERASE (6) */
#define    SCC_MODESNS    0x1A      /* 026 - MODE SENSE (6) */
#define    SCC_SSU        0x1B      /* 027 - Start/Stop Unit or Load/Unload */
#define    SCC_RCVDIAG    0x1C      /* 028 - RECEIVE DIAGNOSTIC RESULTS */
#define    SCC_SNDDIAG    0x1D      /* 029 - SEND DIAGNOSTIC */
#define    SCC_MEDIUM_PAR 0x1E      /* 030 - PREVENT/ALLOW MEDIUM REMOVAL */
#define    SCC_READFORMAT 0x23      /* 035 - READ FORMAT CAPACITIES (MMC) */
#define    SCC_SET_WINDOW 0x24      /* 036 - SET WINDOW */
#define    SCC_READCAP    0x25      /* 037 - READ CAPACITY (10) */
#define    SCC_READEXT    0x28      /* 040 - READ (10) */
#define    SCC_READGEN    0x29      /* 041 - READ GENERATION */
#define    SCC_WRITEXT    0x2A      /* 042 - WRITE (10) */
#define    SCC_SEEK_10    0x2B      /* 043 - SEEK (10) */
#define    SCC_ERASE_10   0x2C      /* 044 - ERASE (10) */
#define    SCC_READUPDBLK 0x2D      /* 045 - READ UPDATED BLOCK */
#define    SCC_WRTVRFY_10 0x2E      /* 046 - WRITE AND VERIFY (10) */
#define    SCC_VERIMED    0x2F      /* 047 - VERIFY MEDIA (10) */
#define    SCC_SCHHIGH_10 0x30      /* 048 - SEARCH DATA HIGH (10) */
#define    SCC_SCHEQ_10   0x31      /* 049 - SEARCH DATA EQUAL (10) */
#define    SCC_SCHLOW_10  0x32      /* 050 - SEARCH DATA LOW (10) */
#define    SCC_SETLMTS_10 0x33      /* 051 - SET LIMITS (10) */
#define    SCC_PRFETCH_10 0x34      /* 052 - PRE-FETCH (10) */
#define    SCC_SYNCCAC_10 0x35      /* 053 - SYNCHRONIZE CACHE (10) */
#define    SCC_LKCACHE_10 0x36      /* 054 - LOCK/UNLOCK CACHE (10) */
#define    SCC_RDDFTDT_10 0x37      /* 055 - READ DEFECT DATA (10) */
#define    SCC_MEDIUMSCAN 0x38      /* 056 - MEDIUM SCAN */
#define    SCC_COMPARE    0x39      /* 057 - COMPARE */
#define    SCC_COPYVERIFY 0x3A      /* 058 - COPY AND VERIFY */
#define    SCC_WRITEBUF   0x3B      /* 059 - WRITE BUFFER */
#define    SCC_READBUF    0x3C      /* 060 - READ BUFFER */
#define    SCC_UPDATEBLK  0x3D      /* 061 - UPDATE BLOCK */
#define    SCC_READLONG   0x3E      /* 062 - READ LONG */
#define    SCC_WRITELONG  0x3F      /* 063 - WRITE LONG */
#define    SCC_CHANGEDEF  0x40      /* 064 - CHANGE DEFINITION */
#define    SCC_WRTSAME_10 0x41      /* 065 - WRITE SAME (10) */
#define    SCC_GETDENSITY 0x44      /* 066 - REPORT DENSITY SUPPORT */
#define    SCC_PLAYAUDIO  0x45      /* 067 - PLAY AUDIO (10) */
#define    SCC_GETCONFIG  0x46      /* 068 - GET CONFIGURATION */
#define    SCC_PLAYAUDMSF 0x47      /* 069 - PLAY AUDIO MSF */
#define    SCC_EVENTNOTFY 0x4A      /* 074 - GET EVENT STATUS NOTIFICATION */
#define    SCC_PAUSERSUME 0x4B      /* 075 - PAUSE / RESUME */
#define    SCC_LOGSELECT  0x4C      /* 076 - LOG SELECT */
#define    SCC_LOGSENSE   0x4D      /* 077 - LOG SENSE */
#define    SCC_XDWRITE_10 0x50      /* 080 - XDWRITE (10) */
#define    SCC_XPWRITE_10 0x51      /* 081 - XPWRITE (10) */
#define    SCC_XDREAD_10  0x52      /* 082 - XDREAD (10) */
#define    SCC_XDWRTRD_10 0x53      /* 083 - XDWRITEREAD (10) */
#define    SCC_SNDOPCINFO 0x54      /* 084 - SEND OPC INFORMATION */
#define    SCC_MDSELCT_10 0x55      /* 085 - MODE SELECT (10) */
#define    SCC_RESERVE_10 0x56      /* 086 - RESERVE (10) */
#define    SCC_RELEASE_10 0x57      /* 087 - RELEASE (10) */
#define    SCC_REPAIRTRCK 0x58      /* 088 - REPAIR TRACK */
#define    SCC_MODESNS_10 0x5A      /* 090 - MODE SENSE (10) */
#define    SCC_CLOSETRACK 0x5B      /* 091 - CLOSE TRACK / SESSION */
#define    SCC_RDBUFCAP   0x5C      /* 092 - READ BUFFER CAPACITY */
#define    SCC_SENDCUESHT 0x5D      /* 093 - SEND CUE SHEET */
#define    SCC_PRRSERVIN  0x5E      /* 094 - PERSISTENT RESERVE IN */
#define    SCC_PRSERVOUT  0x5F      /* 095 - PERSISTENT RESERVE OUT */
#define    SCC_EXTCDB     0x7E      /* 126 - EXTENDED CDB */
#define    SCC_VARLTHCDB  0x7F      /* 127 - VARIABLE LENGTH CDB */
#define    SCC_XDWRT_16   0x80      /* 128 - XDWRITE EXTENDED (16) */
#define    SCC_REBUILD_16 0x81      /* 129 - REBUILD (16) */
#define    SCC_REGENER_16 0x82      /* 130 - REGENERATE (16) */
#define    SCC_EXTENDCOPY 0x83      /* 131 - EXTENDED COPY */
#define    SCC_RECCOPYRLT 0x84      /* 132 - RECEIVE COPY RESULTS */
#define    SCC_ATAPTHR_16 0x85      /* 133 - ATA COMMAND PASS THROUGH (16) */
#define    SCC_A_CTL_IN   0x86      /* 134 - ACCESS CONTROL IN */
#define    SCC_A_CTL_OUT  0x87      /* 135 - ACCESS CONTROL OUT */
#define    SCC_READ_16    0x88      /* 136 - READ (16) */
#define    SCC_WRITE_16   0x8A      /* 138 - WRITE (16) */
#define    SCC_ORWRITE    0x8B      /* 139 - ORWRITE */
#define    SCC_READ_ATTR  0x8C      /* 140 - READ ATTRIBUTE */
#define    SCC_WRITE_ATTR 0x8D      /* 141 - WRITE ATTRIBUTE */
#define    SCC_WRTVRFY_16 0x8E      /* 142 - WRITE AND VERIFY (16) */
#define    SCC_VERIFY_16  0x8F      /* 143 - VERIFY (16) */
#define    SCC_PRFETCH_16 0x90      /* 144 - PRE-FETCH (16) */
#define    SCC_SYNCCAC_16 0x91      /* 145 - SYNCHRONIZE CACHE (16) */
#define    SCC_SPACE_16   0x92      /* 146 - SPACE (16) */
#define    SCC_LKCACHE_16 0x92      /* 146 - LOCK UNLOCK CACHE (16) */
#define    SCC_WRTSAME_16 0x93      /* 147 - WRITE SAME (16) */
#define    SCC_READCAP_16 0x9E      /* 158 - READ CAPACITY (16) [Service action In (16)] */
#define    SCC_SERVOUT_16 0x9F      /* 159 - SERVICE ACTION OUT (16) */
#define    SCC_REPLUNS    0xA0      /* 160 - REPORT LUNS */
#define    SCC_ATAPTHR_12 0xA1      /* 161 - ATA COMMAND PASS THROUGH (12) */
#define    SCC_SECPROTIN  0xA2      /* 162 - SECURITY PROTOCOL IN */
#define    SCC_REPORTOPCD 0xA3      /* 163 - REPORT SUPPORTED OPCODES */
#define    SCC_MAINTOUT   0xA4      /* 164 - MAINTENANCE (OUT) (REPORT_KEY) */
#define    SCC_MOVEMEDIUM 0xA5      /* 165 - MOVE MEDIUM */
#define    SCC_EXCHMEDIUM 0xA6      /* 166 - EXCHANGE MEDIUM */
#define    SCC_MVMEDATTCH 0xA7      /* 167 - MOVE MEDIUM ATTACHED */
#define    SCC_READ_12    0xA8      /* 168 - READ (12) */
#define    SCC_SERVICEOUT 0xA9      /* 169 - SERVICE ACTION OUT (12) */
#define    SCC_WRITE_12   0xAA      /* 170 - WRITE (12) */
#define    SCC_SERVICEIN  0xAB      /* 171 - SERVICE ACTION IN (12) */
#define    SCC_ERASE_12   0xAC      /* 172 - ERASE (12) */
#define    SCC_READDVDSTR 0xAD      /* 173 - READ DVD STRUCTURE */
#define    SCC_WRTVRFY_12 0xAE      /* 174 - WRITE AND VERIFY (12) */
#define    SCC_VERIFY_12  0xAF      /* 175 - VERIFY (12) */
#define    SCC_SCHHIGH_12 0xB0      /* 176 - SEARCH DATA HIGH (12) */
#define    SCC_SCHEQ_12   0xB1      /* 177 - SEARCH DATA EQUAL (12) */
#define    SCC_SCHLOW_12  0xB2      /* 178 - SEARCH DATA LOW (12) */
#define    SCC_SETLMTS_12 0xB3      /* 179 - SET LIMITS (12) */
#define    SCC_READSTATCH 0xB4      /* 180 - READ ELEMENT STATUS ATTACHED */
#define    SCC_SECPROTOUT 0xB5      /* 181 - SECURITY PROTOCOL OUT */
#define    SCC_SENDVOLTAG 0xB6      /* 182 - SEND VOLUME TAG */
#define    SCC_RDDFTDT_12 0xB7      /* 183 - READ DEFECT DATA (12) */
#define    SCC_READSTATUS 0xB8      /* 184 - READ ELEMENT STATUS */
#define    SCC_READCDMSF  0xB9      /* 185 - READ CD MSF */
#define    SCC_REDGRPIN   0xBA      /* 186 - REDUNDANCY GROUP (IN) */
#define    SCC_REDGRPOUT  0xBB      /* 187 - REDUNDANCY GROUP (OUT) */
#define    SCC_SPARE_IN   0xBC      /* 188 - SPARE (IN) */
#define    SCC_SPARE_OUT  0xBD      /* 189 - SPARE (OUT) */
#define    SCC_VOLUME_IN  0xBE      /* 190 - VOLUME SET (IN) */
#define    SCC_VOLUME_OUT 0xBF      /* 191 - VOLUME SET (OUT) */

/* SCSI Status codes */
#define    SCS_NORM       0x00      /* Normal status */
#define    SCS_ECHK       0x02      /* Check Condition */
#define    SCS_NCMG       0x04      /* Condition Met/Good */
#define    SCS_BUSY       0x08      /* Busy */
#define    SCS_NING       0x10      /* Intermediate/Good */
#define    SCS_NICM       0x14      /* Intermediate/Condition Met */
#define    SCS_RESC       0x18      /* Reservation Conflict */
#define    SCS_CMDT       0x22      /* Command Terminated */
#define    SCS_QUEF       0x28      /* Queue Full */
#define    SCS_ACAC       0x30      /* ACA Active */

/* SCSI Return codes from the SCSI command execution. */
#define    SCR_OK         0x00      /* No error returned */
#define    SCR_CC         0x43      /* Check condition from cmd exec */

/* SCSI Key */
#define    SCK_MASK       0x0F      /* Mask for sense key byte 2 */

#define    SCK_NONE       0x00      /* No sense */
#define    SCK_RECOVERED  0x01      /* Recovered */
#define    SCK_NOTRDY     0x02      /* Not ready */
#define    SCK_MEDIUM     0x03      /* Medium */
#define    SCK_HARDWARE   0x04      /* Hardware */
#define    SCK_ILLEGAL    0x05      /* Illegal */
#define    SCK_UNITATT    0x06      /* Unit attention */
#define    SCK_PROTECT    0x07      /* Data protect */
#define    SCK_BLANK      0x08      /* Blank check */
#define    SCK_VENDOR     0x09      /* Vendor specific */
#define    SCK_COPYAB     0x0A      /* Copy abort */
#define    SCK_ABORT      0x0B      /* Abort command */
#define    SCK_OBSOLETE   0x0C      /* Obsolete */
#define    SCK_OVERFLOW   0x0D      /* Volume overflow */
#define    SCK_MISCOMPARE 0x0E      /* Miscompare */
#define    SCK_RESERVED   0x0F      /* Reserved */

/* SCSI sense definitions */
#define SSERR       0x00            /* Error code */
#define SSSEGMENT   0x01            /* Segment */
#define SSKEY       0x02            /* Key */
#define SSLBA3      0x03            /* LBA MSB */
#define SSLBA2      0x04
#define SSLBA1      0x05
#define SSLBA0      0x06            /* LBA LSB */
#define SSADDLENGTH 0x07            /* Additional sense length */
#define SSCS3       0x08            /* Command specific MSB */
#define SSCS2       0x09
#define SSCS1       0x0A
#define SSCS0       0x0B            /* Command specific LSB */
#define SSCODE      0x0C            /* Additional sense code */
#define SSQUAL      0x0D            /* Additional sense qualifier */
#define SSRES       0x0E            /* Reserved */
#define SSSKS2      0x0F            /* Sense key specific MSB */
#define SSSKS1      0x10
#define SSSKS0      0x11            /* Sense key specific LSB */
#define SENSESIZE   0x12            /* Size of SCSI sense data */

/* SCSI ASC */
#define    ASC_SES_FAIL        0x35 /* SES Enclosure failure */

/* SCSI ASCQ */
#define    ASCQ_SES_FAILURE    0x00 /* SES service failure */
#define    ASCQ_SES_UNSUPP     0x01 /* SES service unsupported */
#define    ASCQ_SES_UNAVAIL    0x02 /* SES service unavailable */
#define    ASCQ_SES_XFER_FAIL  0x03 /* SES service transfer failure */
#define    ASCQ_SES_XFER_REF   0x04 /* SES service transfer refused */

/* SCSI Mode Sense Page Codes */
#define    MSP_UNITATT          0x00 /* Unit Attention page */
#define    MSC_ERROR_RECOVERY   0x01 /* Error Recovery page */
#define    MSP_DISC_REC         0x02 /* Disconnect/Reconnect page */
#define    MSP_FORMATPARAMS     0x03 /* Format Parameters page */
#define    MSC_VERIFY_RECOVERY  0x07 /* Verify Error Recovery page */

/* Page Control Field Mask */
#define    MSD_PARAMS           0    /* Mode Sense Data Parameters offset */
#define    MSD_RECOVERYTIME     8    /* Recovery time offset (UINT16) */

#define    MS_PAGECODE          0x02 /* PCF offset in the cmd */
#define    MS_PAGECODE_MASK     0x3F /* PCF mask in the cmd */

#define    PCF_CURRENT          0x00 /* Return Current Values */
#define    PCF_CHANGEABLE       0x40 /* Return Changeable Values */
#define    PCF_DEFAULTS         0x80 /* Return Defaul Values */
#define    PCF_SAVED            0xc0 /* Return Saved Values */

#define    MS10_PS_OFSET        16   /* Offset to PS in Mode Sense Page */
#define    MS10_PARAMS_OFSET    18   /* Offset to Params in Mode Sense Page */
#define    MS10_PS_OFSET        16   /* Offset to PS in Mode Sense Page */
#define    SENSE_DATA_OFSET     0    /* Sense data length offset */
#define    DEV_PARAM_OFSET      3    /* Device Specific params offset */
#define    BLK_CNT_OFSET        8    /* Number of blocks offset */

#define    MSV_RECOVERYTIME     0xB80B  /* 3 seconds (3000 byte swapped */

#define    MSD_PS_BIT           0x80 /* PS Bit */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/* Mode page structure. Note that all fields are Motorola order. */
typedef struct MODE_PAGE_HEADER
{
    UINT16      dataLength;         /* Length of data to follow */
    UINT8       mediumType;         /* Medium type */
    UINT8       devSpecParm;        /* Device specific parameters */
    UINT8       longLBA;            /* >2TB support */
    UINT8       rsvd5;              /* Reserved */
    UINT16      blockDescLength;    /* Block descriptor length */
} MODE_PAGE_HEADER;

typedef struct MODE_PAGE_BLOCK_DESC
{
    UINT8       densityCode;        /* Density Code */
    UINT8       numBlocks[3];       /* Number of blocks */
    UINT8       rsvd4;              /* Reserved */
    UINT8       blockSize[3];       /* Block size */
} MODE_PAGE_BLOCK_DESC;

typedef struct MODE_PAGE
{
    UINT8       pageCode;           /* Page num (MSBit is savable indicator) */
    UINT8       length;             /* Page length to follow */
    ZeroArray(UINT8,data);          /* Page data */
} MODE_PAGE;

/* Inquiry pages. */
typedef struct INQ_HDR
{
    UINT8       periphQualType;     /* Peripheral qualifier and type */
    UINT8       pageCode;           /* Page code */
    UINT8       rsvd;               /* Reserved */
    UINT8       length;             /* Page length (bytes to follow) */
} INQ_HDR;

typedef struct INQ_P83_ID_DESC
{
    UINT8       set;                /* Code set */
    UINT8       type;               /* Association and ID type */
    UINT8       rsvd;               /* Reserved */
    UINT8       length;             /* Descriptor length */
    ZeroArray(UINT8, desc);         /* Descriptor */
} INQ_P83_ID_DESC;

#if defined(MODEL_7000) || defined(MODEL_4700)
typedef struct INQ_P85_MGMT_DESC
{
    UINT8       service_type;       /* Sevice + association */
    UINT8       rsvd;               /* Reserved */
    UINT8       length;             /* Descriptor length */
    ZeroArray(UINT8, desc);         /* Descriptor */
} INQ_P85_MGMT_DESC;
#endif /* MODEL_7000 || MODEL_4700 */

typedef struct INQ_PAGE_83
{
    INQ_HDR     header;             /* Page header */
    ZeroArray(UINT8, id);           /* Identification descriptors */
} INQ_PAGE_83;

/*
******************************************************************************
** Public variables
******************************************************************************
*/

typedef struct SNS
{
    UINT8       rspCode;            /* SCSI response code */
    UINT8       segment;            /* Segment number */
    UINT8       snsKey;             /* Sense key */
    UINT32      iBytes;             /* Information bytes */
    UINT8       addSnsLen;          /* Additional sense length */
    UINT32      snsKeySpec;         /* Sense key specific information */
    UINT8       asc;                /* Additional sense code */
    UINT8       ascq;               /* Additional sense code qualifier */
    UINT8       fru;                /* Field replacable unit */
    UINT8       sks[3];             /* Sense key specific information */
    UINT8       addSns[14];         /* Additional sense information */
} SNS;

/* ------------------------------------------------------------------------ */
/* SCSI Command structures for various SCSI commands */

/* SCSI command fields specific for Test Unit Ready command - (cmd:0x00) */
typedef struct SCSI_TEST_UNIT_READY
{
    UINT8   opCode;      /* SCSI command code */
    UINT8   reserved[4];
    UINT8   control;
    UINT8   padTo16[10];
} SCSI_TEST_UNIT_READY;

/* SCSI command fields specific for inquiry command - standard & SCSI (cmd:0x12)
 * SCSI command fields for gTemplateInquiry  & gTemplateInqSN */
typedef struct SCSI_INQUIRY_STANDARD
{
    UINT8   opCode;      /* SCSI command code */
    UINT8   flags;       /* Flags/options for this command */
    UINT8   pageCode;    /* Page of vital product data information that disk drive returns */
    UINT8   reserved;
    UINT8   numBytes;    /* Number of bytes  allocated for return data */
    UINT8   control;
    UINT8   padTo16[10];
} SCSI_INQUIRY_STANDARD;

/* SCSI command fields specific for mode sense(6) command (cmd:0x1A) */
typedef struct SCSI_MODE_SENSE_6
{
    UINT8   opCode;      /* SCSI command code */
    UINT8   DBD;         /* Bit for disable block descriptors */
    UINT8   pagecode;
    UINT8   reserved1;
    UINT8   length;
    UINT8   control;
    UINT8   padTo16[6];
} SCSI_MODE_SENSE_6;

/* SCSI command fields specific for SCSI start/stop - unit (cmd:0x1b)
 * SCSI command fields for gTemplateStartUnit  & gTemplateStopUnit */
typedef struct SCSI_START_STOP_UNIT
{
    UINT8   opCode;      /* SCSI command code */
    UINT8   immed:1;     /* Immediate status return value after operation - part of flags */
    UINT8   flags1:7;    /* Rest of the flags */
    UINT8   reserve1[2];
    UINT8   startStop:1; /* 1 - start unit; 0 - stop unit */
    UINT8   reserve2:7;
    UINT8   control;
    UINT8   padTo16[10];
} SCSI_START_STOP_UNIT;

/* SCSI command fields specific for SCSI Send Diagnostic (cmd:0x1d)
 * SCSI command fields for gTemplateSendDiag & gTemplateSESP4WWN */
typedef struct SCSI_SEND_DIAGNOSTIC
{
    UINT8   opCode;      /* SCSI command code */
    UINT8   flags;       /* Flags/options */
    UINT8   reserved;
    UINT16  allocLength; /* Length of parameter list transfered during data transfer - MSB - LSB */
    UINT8   control;
    UINT8   padTo16[10];
} SCSI_SEND_DIAGNOSTIC;

/* SCSI command fields specific for read capacity (10) command (cmd:0x25) */
typedef struct SCSI_READ_CAPACITY_10
{
    UINT8   opCode;      /* SCSI command code */
    UINT8   LUN:3;       /* 3/lun, 4/reserved, 1/reladr */
    UINT8   reserved1:4;
    UINT8   RelAdr:1;
    UINT32  LBA;
    UINT8   reserved[2];
    UINT8   reserved2:7;
    UINT8   PMI:1;       /* 0 = return value for last LBA. */
    UINT8   control;
    UINT8   padTo16[6];
} SCSI_READ_CAPACITY_10;

/* SCSI command fields specific for  read command (cmd:0x28)
 * SCSI command fields for gTemplateRead,gTemplateReadExt, gTemplateReadRsvd */
typedef struct SCSI_READ_EXTENDED   /* read 10 */
{
    UINT8   opCode;      /* SCSI command code */
    UINT8   flags;       /* Flags/options for this command */
    UINT32  lba;         /* Logical block at which read operation begins */
    UINT8   reserved;
    UINT16  numBlocks;   /* Transfer length */
    UINT8   control;
    UINT8   padTo16[6];
} SCSI_READ_EXTENDED;               /* read 10 */

/* SCSI command fields specific for write command (cmd:0x2a)
 * SCSI command fields for gTemplateWrite, gTemplateWriteExt */
typedef struct SCSI_WRITE_EXTENDED  /* write 10 */
{
    UINT8   opCode;      /* SCSI command code */
    UINT8   flags;       /* Flags/options for this command */
    UINT32  lba;         /* Logical block at which write operation begins */
    UINT8   reserved;
    UINT16  numBlocks;   /* Transfer length */
    UINT8   control;
    UINT8   padTo16[6];
} SCSI_WRITE_EXTENDED;   /* write 10 */

/* SCSI command fields  for check words (reserved area & 1st MB) (cmd:0x2f)
 * SCSI command fields for gTemplateVerify, gTemplateVerify1 */
typedef struct SCSI_VERIFY_10
{
    UINT8   opCode;      /* SCSI command code */
    UINT8   flags;       /* Flags/options for this command */
    UINT32  lba;         /* Logical block address verify operation begins - MSBs -- LSBs */
    UINT8   reserved;
    UINT16  numBlocks;   /* No of blocks that are verified. MSB,LSB */
    UINT8   control;
    UINT8   padTo16[6];
} SCSI_VERIFY_10;

/* SCSI command fields specific for write command (cmd:0x41)
 * SCSI command fields for gTemplateWriteSame */
typedef struct SCSI_WRITE_SAME
{
    UINT8   opCode;      /* SCSI command code */
    UINT8   flags;       /* Flags/options for this command */
    UINT32  lba;         /* Logical block address */
    UINT8   reserved;
    UINT16  numBlocks;   /* Number of blocks to be written */
    UINT8   control;
    UINT8   padTo16[6];
} SCSI_WRITE_SAME;

/* SCSI command fields specific for mode select command (cmd:0x55) */
typedef struct SCSI_MODE_SELECT
{
    UINT8   opCode;      /* SCSI command code */
    UINT8   flags;       /* Flags/options for this command */
    UINT8   reserved[5];
    UINT16  parmListLen; /* Number of bytes in parm list */
    UINT8   control;
    UINT8   padTo16[6];
} SCSI_MODE_SELECT;

/* SCSI command fields specific for mode sense command (cmd:0x5A) */
typedef struct SCSI_MODE_SENSE
{
    UINT8   opCode;      /* SCSI command code */
    UINT8   flags;       /* Flags/options for this command */
    UINT8   page;        /* Page to be selected */
    UINT8   reserved[4];
    UINT16  parmListLen; /* Number of bytes in parm list */
    UINT8   control;
    UINT8   padTo16[6];
} SCSI_MODE_SENSE;

/* SCSI command fields specific for  read 16 command (cmd:0x88) */
typedef struct SCSI_READ_16 /* read 16 */
{
    UINT8   opCode;      /* SCSI command code */
    UINT8   flags;       /* Flags/options for this command */
    UINT64  lba;         /* Logical block address verify operation begins - MSBs -- LSBs */
    UINT32  numBlocks;   /* Number of blocks that are verified. MSB,LSB */
    UINT8   reserved;
    UINT8   control;
} SCSI_READ_16;          /* read 16 */

/* SCSI command fields for write (16) (cmd:0x8a) */
typedef struct SCSI_WRITE_16
{
    UINT8   opCode;      /* SCSI command code */
    UINT8   flags;       /* Flags/options for this command */
    UINT64  lba;         /* Logical block address verify operation begins - MSBs -- LSBs */
    UINT32  numBlocks;   /* Number of blocks that are verified. MSB,LSB */
    UINT8   reserved;
    UINT8   control;
} SCSI_WRITE_16;

/* SCSI command fields for writesame (16) (cmd:0x93) */
typedef struct SCSI_WRITESAME_16
{
    UINT8   opCode;      /* SCSI command code */
    UINT8   flags;       /* Flags/options for this command */
    UINT64  lba;         /* Logical block address verify operation begins - MSBs -- LSBs */
    UINT32  numBlocks;   /* Number of blocks that are verified. MSB,LSB */
    UINT8   reserved;
    UINT8   control;
} SCSI_WRITESAME_16;

/* SCSI command fields for verify (16) (cmd:0x8f) */
typedef struct SCSI_VERIFY_16
{
    UINT8   opCode;      /* SCSI command code */
    UINT8   flags;       /* Flags/options for this command */
    UINT64  lba;         /* Logical block address verify operation begins - MSBs -- LSBs */
    UINT32  numBlocks;   /* Number of blocks that are verified. MSB,LSB */
    UINT8   reserved;
    UINT8   control;
} SCSI_VERIFY_16;

/* Generic SCSI command structure */
typedef union SCSI_COMMAND_FORMAT
{
        UINT8                   opCode;         /* SCSI command code. */
        UINT8                   cmd[16];        /* Accessible via the possible 16 bytes. */
        SCSI_TEST_UNIT_READY    testunitready;  /* 0x00 */
        SCSI_INQUIRY_STANDARD   inqStandard;    /* 0x12 */
        SCSI_MODE_SENSE_6       modeSense6;     /* 0x1a */
        SCSI_START_STOP_UNIT    scsiStartStop;  /* 0x1b */
        SCSI_SEND_DIAGNOSTIC    sendDiag;       /* 0x1d */
        SCSI_READ_CAPACITY_10   readcap_10;     /* 0x25 */
        SCSI_READ_EXTENDED      readExt;        /* 0x28 read 10 */
        SCSI_WRITE_EXTENDED     writeExt;       /* 0x2a write 10 */
        SCSI_VERIFY_10          verify_10;      /* 0x2f */
        SCSI_WRITE_SAME         writeSame;      /* 0x41 */
        SCSI_MODE_SELECT        modeSelect;     /* 0x55 */
        SCSI_MODE_SENSE         modeSense;      /* 0x5a */
        SCSI_VERIFY_16          verify_16;      /* 0x8f */
        SCSI_READ_16            read_16;        /* 0x88 */
        SCSI_WRITE_16           write_16;       /* 0x8a */
        SCSI_WRITESAME_16       writesame_16;   /* 0x93 */
} SCSI_COMMAND_FORMAT;

#endif /* _SCSI_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
