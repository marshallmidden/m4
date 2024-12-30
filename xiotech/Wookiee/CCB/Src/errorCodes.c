/* $Id: errorCodes.c 159966 2012-10-01 23:20:49Z marshall_midden $ */
/**
******************************************************************************
**
**  @file   errorCodes.C
**
**  @brief  Source file for CCB error Codes
**
**  Functions to access the ccb error codes.
**
** Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "errorCodes.h"

#ifdef LOG_SIMULATOR
#include "LogSimFuncs.h"
#else   /* LOG_SIMULATOR */
#include "globalOptions.h"
#include "LOG_Defs.h"
#include "MR_Defs.h"
#include "PortServer.h"
#include "XIO_Std.h"
#include "XIO_Const.h"
#include "XIO_Types.h"
#endif  /* LOG_SIMULATOR */

/*****************************************************************************
** Private defines
*****************************************************************************/

#if defined(MODEL_7000) || defined(MODEL_4700)
#define  BAY  "ISE"
#else  /* MODEL_7000 || MODEL_4700 */
#define  BAY  "BAY"
#endif /* MODEL_7000 || MODEL_4700 */

/*****************************************************************************
** Code Start
*****************************************************************************/

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
** Returns:     NONE
**
** NOTE:        THIS FUNCTION SHOULD AT MOST USE (16) CHARACTERS.
**--------------------------------------------------------------------------*/
void StatusErrorToString(UINT8 status, UINT32 errorCode, char *strPtr)
{
    char        errStr[20];

    switch (status)
    {
        case PI_GOOD:
            strcpy(strPtr, "OK");
            break;

        case PI_ERROR:
            {
                switch (errorCode)
                {
                    case EC_VCG_NO_DRIVES:
                        strcpy(errStr, "NO OWN DRV");
                        break;
                    case EC_VCG_AS_INVALID_CTRL:
                    case EC_VCG_UC_INVALID_CTRL:
                    case EC_VCG_IC_INVALID_CTRL:
                        strcpy(errStr, "INV CNTRLR");
                        break;
                    case EC_VCG_AS_FAILED_PING:
                    case EC_VCG_IC_FAILED_PING:
                        strcpy(errStr, "PING");
                        break;
                    case EC_VCG_AS_FAILED_CREATE_CTRL:
                        strcpy(errStr, "ERROR");
                        break;
                    case EC_VCG_AS_IPC_ADD_CONTROLLER:
                        strcpy(errStr, "IPC");
                        break;
                    case EC_VCG_AL_INVALID_STATE:
                    case EC_VCG_IC_NOT_INACTIVATED:
                        strcpy(errStr, "INV STATE");
                        break;
                    case EC_VCG_IC_EF:
                        strcpy(errStr, "ELECTION");
                        break;
                    case EC_VCG_AL_IL_CHG_VCGID:
                    case EC_VCG_AL_IL_RMV_CFG_CTRL:
                        strcpy(errStr, "INV LCNSE");
                        break;
                    case EC_VCG_IC_BAD_MIRROR_PARTNER:
                    case EC_VCG_UC_BAD_MIRROR_PARTNER:
                        strcpy(errStr, "BAD MP");
                        break;
                    case EC_VCG_UC_TDISCACHE:
                    case EC_VCG_AC_TDISCACHE:
                    case EC_VCG_IC_TDISCACHE:
                        strcpy(errStr, "TDISCACHE");
                        break;

                        /*
                         * Code Burn Error Codes
                         */
                    case FWHEADER_VERIFY_ERROR:
                    case PROBABLY_NO_HEADER:
                        strcpy(errStr, "INV HEADER");
                        break;
                    case PROGRAM_VERIFY_ERROR:
                        strcpy(errStr, "PRGRM ERR");
                        break;
                    case ALLOCATION_ERROR:
                        strcpy(errStr, "ALLOCATION");
                        break;
                    case PARM_ERROR:
                        strcpy(errStr, "INV PARAM");
                        break;
                    case MULTI_MAX_EXCEEDED:
                        strcpy(errStr, "MULTI MAX");
                        break;
                    case MALLOC_FAILURE:
                        strcpy(errStr, "MEMORY");
                        break;
                    case CCB_FLASH_ADDRESS_ERROR:
                    case CCB_FLASH_CORRUPTED:
                        strcpy(errStr, "FLASH ERR");
                        break;
                    case UPDATE_PROC_TIMEOUT:
                        strcpy(errStr, "TIMEOUT");
                        break;
                    case NVRAM_UPDATE_FAILURE:
                        strcpy(errStr, "NVR ERR");
                        break;
                    case FILE_TOO_LONG:
                        strcpy(errStr, "MEM ERR");
                        break;
                    case ILLEGAL_TRY_NON_CCB_FW:
                        strcpy(errStr, "INV FW");
                        break;

                        /*
                         * File System Error Codes
                         */
                    case FS_ERROR_WRITE_NULL_BUFFER:
                    case FS_ERROR_WRITE_DIRECT_INIT:
                    case FS_ERROR_WRITE_RANGE_LENGTH:
                    case FS_ERROR_WRITE_HEADER:
                    case FS_ERROR_WRITE_NO_WRITES_HEADER:
                    case FS_ERROR_WRITE_HEADER_DATA_SINGLE:
                    case FS_ERROR_WRITE_NO_WRITES_HEADER_DATA_SINGLE:
                    case FS_ERROR_WRITE_HEADER_DATA_LOOP:
                    case FS_ERROR_WRITE_NO_WRITES_HEADER_DATA_LOOP:
                        strcpy(errStr, "WRITE ERR");
                        break;
                    case FS_ERROR_READ_NULL_BUFFER:
                    case FS_ERROR_READ_DIRECT_INIT:
                    case FS_ERROR_READ_DATA:
                    case FS_ERROR_READ_HEADER:
                        strcpy(errStr, "READ ERR");
                        break;
                    case FS_ERROR_READ_CRC_CHECK_DATA:
                    case FS_ERROR_READ_CRC_CHECK_HEADER:
                        strcpy(errStr, "INV CRC");
                        break;
                    case FS_ERROR_READ_MALLOC_DATA:
                        strcpy(errStr, "MEM ERR");
                        break;
                    case FS_ERROR_NVRAM_READ:
                    case FS_ERROR_NVRAM_WRITE_FID_BE_NVRAM_NO_RESPONSE:
                        strcpy(errStr, "NVR RD ERR");
                        break;
                    case FS_ERROR_NVRAM_WRITE_FID_BE_NVRAM_PI_TIMEOUT:
                    case FS_ERROR_NVRAM_WRITE_NO_RESPONSE:
                    case FS_ERROR_NVRAM_WRITE:
                    case FS_ERROR_NVRAM_WRITE_FID_BE_NVRAM:
                        strcpy(errStr, "NVR WT ERR");
                        break;
                    case FS_ERROR_NVRAM_READ_PI_TIMEOUT:
                    case FS_ERROR_NVRAM_WRITE_PI_TIMEOUT:
                    case FS_ERROR_WRITE_HEADER_PI_TIMEOUT:
                    case FS_ERROR_WRITE_HEADER_DATA_SINGLE_PI_TIMEOUT:
                    case FS_ERROR_READ_DATA_PI_TIMEOUT:
                    case FS_ERROR_FID2FID_PI_TIMEOUT:
                    case FS_ERROR_WRITE_HEADER_DATA_LOOP_PI_TIMEOUT:
                    case FS_ERROR_READ_HEADER_PI_TIMEOUT:
                        strcpy(errStr, "TIMEOUT");
                        break;
                    case FS_ERROR_WRITE_FILE_FULL:
                        strcpy(errStr, "FILE FULL");
                        break;
                    case FS_ERROR_FID2FID_PI_ERROR:
                        strcpy(errStr, "ERROR");
                        break;

                        /*
                         * PI_RollingUpdatePhase() Error Codes
                         */
                    case PHASE_STATE_SET_FAILURE:
                        strcpy(errStr, "SET STATE");
                        break;
                    case PHASE_FAIL_CONTROLLER_FAILURE:
                        strcpy(errStr, "FAIL CN");
                        break;
                    case PHASE_ELECTION_FAILURE:
                        strcpy(errStr, "ELECTION");
                        break;
                    case PHASE_BAD_MIRROR_PARTNER:
                        strcpy(errStr, "BAD MP");
                        break;
                    case PHASE_NON_MIRRORED_RAID5S:
                        strcpy(errStr, "NONMIR R5");
                        break;
                    case PHASE_PING_FAILURE:
                        strcpy(errStr, "PING");
                        break;
                    case PHASE_RAIDS_NOT_READY:
                        strcpy(errStr, "RD NOT RDY");
                        break;
                    case PHASE_STOP_IO_FAILED:
                        strcpy(errStr, "STOP IO");
                        break;
                    case PHASE_RESET_QLOGIC_FAILED:
                        strcpy(errStr, "RESET QL");
                        break;
                    case PHASE_TDISCACHE_FAILED:
                        strcpy(errStr, "TDISCACHE");
                        break;

                        /*
                         * Persistent Data Error Codes
                         */
                    case PDATA_TOO_MUCH_DATA:
                        strcpy(errStr, "TO MCH DT");
                        break;
                    case PDATA_OUT_OF_RANGE:
                        strcpy(errStr, "ILEG RANGE");
                        break;
                    case PDATA_INVALID_OPTION:
                        strcpy(errStr, "INV OPTION");
                        break;

                        /*
                         * PI_VCGShutdown Error Codes
                         */
                    case VCG_SHUTDOWN_ERROR_CHANGE_NET_ADDRESES:
                        strcpy(errStr, "CHNG ADDRS");
                        break;
                    case VCG_SHUTDOWN_ERROR_WRITE_STATE_TO_QUORUM:
                        strcpy(errStr, "CHNG STATE");
                        break;
                    case VCG_SHUTDOWN_ERROR_FE_SDIMM_SHUTDOWN:
                        strcpy(errStr, "SHTDN FEBF");
                        break;
                    case VCG_SHUTDOWN_ERROR_BE_SDIMM_SHUTDOWN:
                        strcpy(errStr, "SHTDN BEBF");
                        break;

                        /*
                         * Miscellaneous Error Codes
                         */
                    case PI_MD5_ERROR:
                        strcpy(errStr, "MD5 ERR");
                        break;
                    case EC_UNLABEL_ALL_OWNED:
                        strcpy(errStr, "ALL OWND");
                        break;
                    case DEINVPKTTYP:
                        strcpy(errStr, "INV PKT TP");
                        break;
                    case DEINVPKTSIZ:
                        strcpy(errStr, "INV PKT SZ");
                        break;
                    case DEBADDAM:
                        strcpy(errStr, "BAD DAM");
                        break;
                    case DELISTERROR:
                        strcpy(errStr, "LST ERR");
                        break;
                    case DE2TBLIMIT:
                        strcpy(errStr, "2TB LMT");
                        break;
                    case DENONXDEV:
                        strcpy(errStr, "DEV N/EXST");
                        break;
                    case DEINOPDEV:
                        strcpy(errStr, "DEV INOP");
                        break;
                    case DEINVLABTYP:
                        strcpy(errStr, "INV LBL TP");
                        break;
                    case DEDEVUSED:
                        strcpy(errStr, "DEV IN USE");
                        break;
                    case DEINITINPROG:
                        strcpy(errStr, "INIT/PRGRS");
                        break;
                    case DEINVWWNAME:
                        strcpy(errStr, "INV WWN");
                        break;
                    case DEIOERR:
                        strcpy(errStr, "I/O ERR");
                        break;
                    case DEINVRTYPE:
                        strcpy(errStr, "INV RTYPE");
                        break;
                    case DECODEBURN:
                        strcpy(errStr, "CODE BURN");
                        break;
                    case DEDEFNRDY:
                        strcpy(errStr, "NOT READY");
                        break;
                    case DEINSDEVCAP:
                        strcpy(errStr, "INSUF CAP");
                        break;
                    case DEINVDRVCNT:
                        strcpy(errStr, "INV DRV CT");
                        break;
                    case DENOTDATALAB:
                        strcpy(errStr, "INV LABEL");
                        break;
                    case DEINVDEPTH:
                        strcpy(errStr, "INV DEPTH");
                        break;
                    case DEINVSTRIPE:
                        strcpy(errStr, "INV STRIPE");
                        break;
                    case DEINVPARITY:
                        strcpy(errStr, "INV PARITY");
                        break;
                    case DEINVVIRTID:
                        strcpy(errStr, "INV VID");
                        break;
                    case DEINVOP:
                        strcpy(errStr, "INV OP");
                        break;
                    case DEINVSESSLOT:
                        strcpy(errStr, "INV SLOT");
                        break;
                    case DEMAXLUNS:
                        strcpy(errStr, "MAX LUNS");
                        break;
                    case DENOTOPRID:
                        strcpy(errStr, "DEV INOP");
                        break;
                    case DEMAXSEGS:
                        strcpy(errStr, "MAX SEG");
                        break;
                    case DEBADNVREC:
                        strcpy(errStr, "INV NVR P2");
                        break;
                    case DEINSUFFREDUND:
                        strcpy(errStr, "INSUF REDN");
                        break;
                    case DEPIDNOTUSED:
                        strcpy(errStr, "UNUSED PID");
                        break;
                    case DENOHOTSPARE:
                        strcpy(errStr, "NO SPARE");
                        break;
                    case DEINVPRI:
                        strcpy(errStr, "INV PRI");
                        break;
                    case DENOTSYNC:
                        strcpy(errStr, "NO SYNCH");
                        break;
                    case DEINSNVRAM:
                        strcpy(errStr, "OUT OF NVR");
                        break;
                    case DEFOREIGNDEV:
                        strcpy(errStr, "DEV FORGN");
                        break;
                    case DETYPEMISMATCH:
                        strcpy(errStr, "TYPE MSMCH");
                        break;
                    case DEBADDEVTYPE:
                        strcpy(errStr, "UNK TYPE");
                        break;
                    case DEINVOPT:
                        strcpy(errStr, "INV OPTION");
                        break;
                    case DEINVTID:
                        strcpy(errStr, "INV TID");
                        break;
                    case DEINVCTRL:
                        strcpy(errStr, "INV CN ID");
                        break;
                    case DERETLENBAD:
                        strcpy(errStr, "INV RT SZ");
                        break;
                    case DEACTREBUILD:
                        strcpy(errStr, "ACTV RBLD");
                        break;
                    case DETOOMUCHDATA:
                        strcpy(errStr, "INV RT SZ");
                        break;
                    case DEINVSID:
                        strcpy(errStr, "INV SERVER");
                        break;
                    case DEINVVID:
                        strcpy(errStr, "INV VID");
                        break;
                    case DEINVRID:
                        strcpy(errStr, "INV RID");
                        break;
                    case DEINVPID:
                        strcpy(errStr, "INV PID");
                        break;
                    case DEINSTABLE:
                        strcpy(errStr, "INSUF SPC");
                        break;
                    case DEINSRES:
                        strcpy(errStr, "INSUF RESC");
                        break;
                    case DELUNMAPPED:
                        strcpy(errStr, "LUN MAPD");
                        break;
                    case DEINVCHAN:
                        strcpy(errStr, "INV CHANL");
                        break;
                    case DELUNNOTMAPPED:
                        strcpy(errStr, "LUN N/MAPD");
                        break;
                    case DEWCRECVRYFAILED:
                        strcpy(errStr, "CACHE REC");
                        break;
                    case DEQRESETFAILED:
                        strcpy(errStr, "CHIP RESET");
                        break;
                    case DEUNASSPATH:
                        strcpy(errStr, "INV PATH");
                        break;
                    case DEOUTOPS:
                        strcpy(errStr, "OUTSTND OPS");
                        break;
                    case DECHECKSUM:
                        strcpy(errStr, "BAD CHCKSM");
                        break;
                    case DECODESAME:
                        strcpy(errStr, "IDENT CODE");
                        break;
                    case DESTOPZERO:
                        strcpy(errStr, "COUNT 0");
                        break;
                    case DEINSMEM:
                        strcpy(errStr, "INSUF MEM");
                        break;
                    case DENOTARGET:
                        strcpy(errStr, "NO TGT/PRT");
                        break;
                    case DENOPORT:
                        strcpy(errStr, "NO SPR PRT");
                        break;
                    case DEINVWSID:
                        strcpy(errStr, "INV WID");
                        break;
                    case DE_IP_OR_SUBNET_ZERO:
                        strcpy(errStr, "IP/SBNT 0");
                        break;
                    case DE_SET_IP_ERROR:
                        strcpy(errStr, "SET IP");
                        break;
                    case DE_VDISK_IN_USE:
                        strcpy(errStr, "OTHER CN");
                        break;
                    case DEEMPTYCPYLIST:
                        strcpy(errStr, "EMPTY CPY LIST");
                        break;
                    case DENOCPYMATCH:
                        strcpy(errStr, "NO CPY MATCH");
                        break;
                    case DEINVHSPTYPE:
                        strcpy(errStr, "INV HSP DEV TYPE");
                        break;
                    case DEASWAPINPROGRESS:
                        strcpy(errStr, "AUTO SWAP IN PROGRESS");
                        break;
                    case DEASWAPSTATE:
                        strcpy(errStr, "IN AUTO SWAP STATE");
                        break;
                    case DE64TBLIMIT:
                        strcpy(errStr, "64TB LIMIT");
                        break;

                    default:
                        strcpy(errStr, "FAILED");
                        break;
                }
                sprintf(strPtr, "%s", errStr);
                break;
            }

        case PI_IN_PROGRESS:
            strcpy(strPtr, "IN PROGRESS");
            break;

        case PI_TIMEOUT:
            strcpy(strPtr, "TIMEOUT");
            break;

        case PI_INVALID_CMD_CODE:
            strcpy(strPtr, "INV CMD");
            break;

        case PI_MALLOC_ERROR:
            strcpy(strPtr, "MEM ERR");
            break;

        case PI_PARAMETER_ERROR:
            strcpy(strPtr, "PARAM ERR");
            break;

        case PI_MASTER_CNT_ERROR:
            strcpy(strPtr, "NOT MASTER");
            break;

        case PI_POWER_UP_REQ_ERROR:
            strcpy(strPtr, "POWER UP");
            break;

        case PI_ELECTION_ERROR:
            strcpy(strPtr, "ELECTION");
            break;

        default:
            strcpy(strPtr, "UNKNOWN");
            break;
    }
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
