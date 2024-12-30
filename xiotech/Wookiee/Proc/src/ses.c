/* $Id: ses.c 159663 2012-08-22 15:36:42Z marshall_midden $ */
/**
 ******************************************************************************
 **
 **  @file       ses.c
 **
 **  @brief      Enclosure services functions
 **
 **  To provide a common means of handling the enclosure services for the
 **  back end processor.
 **
 **  Copyright (c) 1996-2010 Xiotech Corporation. All rights reserved.
 **
 ******************************************************************************
 **/

#include "options.h"
#include "ses.h"

#include "chn.h"
#include "def.h"
#include "defbe.h"
#include "def_con.h"
#include "dev.h"

#include "ficb.h"
#include "GR_LocationManager.h"
#include "ilt.h"
#include "fabric.h"
#include "isp.h"
#include "kernel.h"
#include "LOG_Defs.h"
#include "misc.h"
#include "nvram.h"
#include "online.h"
#include "pcb.h"
#include "pdd.h"
#include "portdb.h"
#include "rebuild.h"
#include "scsi.h"
#include "system.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"

#include <string.h>
extern size_t strnlen(const char *s, size_t maxlen);
#include <stdlib.h>
#include <unistd.h>

#include <ctype.h>

#include <stdio.h>
#include "CT_change_routines.h"

#ifdef HISTORY_KEEP
extern void CT_history_pcb(const char *str, UINT32 pcb);
#endif

/* The following are for created tasks. */
extern unsigned long CT_fork_tmp;
extern void CT_LC_SES_PollDriveForEnclosure(int);
extern void CT_LC_SES_GetSlotWithInq(int);
extern void CT_LC_SES_GetDirectEnclosure(int);
extern void CT_LC_ses_SendBypass(int);
extern void CT_LC_ses_BypassDoubleCheck(int);

/*
 ******************************************************************************
 ** Private defines - constants
 ******************************************************************************
 */

#define tm_const_ntohs(x) (UINT16)((UINT16)(((x)>>8)&0xffU)\
                                  |(UINT16)(((x)<<8)&0xff00U))

/*
 ******************************************************************************
 ** Private defines - macros
 ******************************************************************************
 */
#define XYRATEX_VEND_ID         "XYRATEX "
#define XYRATEX_PROD_ID0        "RS1600-FCF4     "
#define XYRATEX_PROD_ID1        "RS1602-FCF4     "
#define XYRATEX_PROD_ID2        "RS1600-FS44     "
#define XYRATEX_PROD_ID3        "RS1602-FS44     "
#define XYRATEX_VEND_ID_REP     "XIOtech "
#define XYRATEX_PROD_ID0_REP    "SBOD FC Bay     "
#define XYRATEX_PROD_ID1_REP    "SBOD FC 4Gb Bay "

/*
 ******************************************************************************
 ** Public variables - externed in the header file
 ******************************************************************************
 */
static UINT8 S_bgprun = FALSE;
PCB *S_bgppcb = NULL;
SES_DEV_INFO_MAP* SES_DevInfoMaps = NULL;
UINT16 SES_DIMEntries = 0;
PSES_P82_XTEX SES_Page82[MAX_DISK_BAYS];
PSES_P82_XTEX SES_Page83[MAX_DISK_BAYS];

/*
 ******************************************************************************
 ** Public variables - not in the header file
 ******************************************************************************
 */
UINT8 SBOD_Trunking[MAX_PORTS];

/*
 ******************************************************************************
 ** Private function prototypes
 ******************************************************************************
 */
void ses_XyratexLookup(PDD*, PDD*);
void ses_CheckPaths(void);
void ses_CheckMissingEncl(void);
void ses_LocateEnclosures(void);
INT32 ses_BayAttributeTest(UINT8*,UINT8*, UINT32);
INT32 ses_EuroLogicBay(UINT8*,UINT8*);
INT32 ses_AICBay(UINT8 *vendID,UINT8 *prodID);
UINT8 my_atob(UINT8 *, void *, UINT32);
UINT32 Get_SES_Page(PRP_TEMPLATE *tp, void *output, UINT32 size, PDD *pdd);
void ses_SendBypass(UINT32, UINT32, PRP_TEMPLATE *tp, void *output, PDD *baypdd, UINT8 channel, UINT8 lid);
void ses_BypassDoubleCheck(UINT32, UINT32, PDD *pdd, UINT16 ses, UINT8 slot);

/*
 ******************************************************************************
 ** Code Start
 ******************************************************************************
 */

/**
******************************************************************************
**
**  @brief      ASCII to byte conversion
**
**              More details on this function go here.
**
**  @param      *instr  - String to convert
**  @param      *foo    - Not used
**  @param      base    - Base to use for conversion
**
**  @return     8 bit converted value
**
******************************************************************************
**/
UINT8 my_atob(UINT8 *instr, void *foo UNUSED, UINT32 base)
{
    UINT8   tmpval;
    UINT8   cnt = 0;

    while (isspace(instr[cnt]))
    {
        cnt++;
    }

    for (tmpval = 0; cnt < 2; cnt++)
    {
        tmpval = tmpval * base;

        if ((instr[cnt] >= '0') && (instr[cnt] <= '9'))
        {
            tmpval += instr[cnt] - '0';
        }
        else if ((instr[cnt] >= 'A') && (instr[cnt] <= 'F'))
        {
            tmpval += instr[cnt] - 'A' + 10;
        }
        else if ((instr[cnt] >= 'a') && (instr[cnt] <= 'f'))
        {
            tmpval += instr[cnt] - 'a' + 10;
        }
        else
        {
            break;
        }
    }

    return(tmpval);
}

/**
******************************************************************************
**
**  @brief      Start the processing of SES information in background.
**
**              This function start the SES processing task if it
**              was asleep or set the processing flag otherwise.
**              on and in hotswap operations.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void SES_StartBGProcess_c(void)
{
    S_bgprun = TRUE;

    if (TaskGetState(S_bgppcb) == PCB_NOT_READY)
    {
#ifdef HISTORY_KEEP
CT_history_pcb("SES_StartBGProcess_c setting ready pcb", (UINT32)(S_bgppcb));
#endif
        TaskSetState(S_bgppcb, PCB_READY);
    }
}


/**
******************************************************************************
**
**  @brief      Adaptec / Eurologic bay type check
**
**              This function will eventually return the index into the
**              SES_DIMEntries table if a match, otherwise a -1.
**
**  @param      vendor ID, product ID, and mask.
**
**  @return     table index >= 0 if match, -1 if no match
**
******************************************************************************
**/
INT32 ses_EuroLogicBay(UINT8 *vendID,UINT8 *prodID)
{
    /* If nonzero return, no match, bail. */
    if (strncmp((char *)"XYRATEX ", (char *)vendID, 8) != 0)
    {
        return ses_BayAttributeTest(vendID, prodID, SES_DI_MISC_EURO_PAGE1);
    }
    return (-1);
}

/**
 ******************************************************************************
 **
 **  @brief      AIC - Vitesse SAS bay type check
 **
 **              This function will eventually return the index into the
 **              SES_DIMEntries table if a match, otherwise a -1.
 **
 **  @param      vendor ID, product ID, and mask.
 **
 **  @return     table index >= 0 if match, -1 if no match
 **
 ******************************************************************************
 **/
INT32 ses_AICBay(UINT8 *vendID,UINT8 *prodID)
{
    /* If nonzero return, no match, bail. */
    if ((strstr((char *)vendID,(char *)"AIC") != NULL) ||  (strstr((char *)vendID,(char *)"Xiotec") != NULL) )
    {
        return ses_BayAttributeTest(vendID, prodID, SES_DI_MISC_VITESSE);
    }
    return (-1);
}

/**
******************************************************************************
**
**  @brief      adapter code for external references for Directly Addressable check
**
**              This function looks for a directly addressable bay. created to
**              avoid changes to external modules. crude, but effective.
**
**  @param      pdd pointer
**
**  @return     1 if found, 0 if match
**
******************************************************************************
**/
UINT8 SES_DirectlyAddressable(PDD * pdd)
{
    return(ses_BayAttributeTest(pdd->vendID, pdd->prodID, SES_DI_MISC_DIRECT) >= 0);
}


/**
******************************************************************************
**
**  @brief      Enclosure Miscellaneous attribute check
**
**              This function will return the index into the SES_DIMEntries
**              table if match, otherwise a -1.
**
**  @param      vendor ID, product ID, and mask.
**
**  @return     table index >= 0 if match, -1 if no match
**
******************************************************************************
**/
INT32 ses_BayAttributeTest(UINT8 *vendID, UINT8 *prodID, UINT32 miscMask)
{
    UINT16  indx;
    UINT32  vendIDLen = strnlen((char *)vendID, 8);
    UINT32  prodIDLen = strnlen((char *)prodID, 16);

    if ((prodIDLen > 0) && (vendIDLen > 0) )
    {
        for (indx = 0; indx <= SES_DIMEntries; ++indx)
        {
            if ((SES_DevInfoMaps[indx].devFlags[SES_DI_MISC] & miscMask) &&
                (!strncmp((char *)SES_DevInfoMaps[indx].devProdID, (char *)prodID, (signed)prodIDLen)) &&
                (!strncmp((char *)SES_DevInfoMaps[indx].devVendor, (char *)vendID, (signed)vendIDLen)))
            {
                return indx;
            }
        }
    }

    return (-1);
}


/**
******************************************************************************
**
**  @brief      Get SES page.
**
**  @param      tp      - Template of scsi command.
**  @param      output  - Address to store output of scsi command.
**  @param      size    - Length of output buffer.
**  @param      pdd     - the PDD to use.
**
**  @return     return from ON_SCSICmd
**
******************************************************************************
**/

UINT32 Get_SES_Page(PRP_TEMPLATE *tp, void *output, UINT32 size, PDD *pdd)
{
    UINT32 retCode;
    SNS*   snsData = NULL;     /* Error data                       */
    UINT8  errCnt = 0;         /* Error counter                    */

    do
    {
        if (snsData)
        {
            s_Free(snsData, sizeof(SNS), __FILE__, __LINE__);
            snsData = NULL;
            errCnt++;
            TaskSleepMS((unsigned)(errCnt*100));
        }

        retCode = ON_SCSICmd(tp, output, size, pdd, &snsData);

    } while ((retCode == SCR_CC) &&
             (errCnt < 5) &&
             (snsData->asc == ASC_SES_FAIL) &&
             ((snsData->ascq == ASCQ_SES_XFER_FAIL) ||
              (snsData->ascq == ASCQ_SES_XFER_REF)));

    if (snsData)
    {
        s_Free(snsData, sizeof(SNS), __FILE__, __LINE__);
    }

    return (retCode);
}


/**
******************************************************************************
**
**  @brief      Determine the device type based upon the table entries
**              containing the vendor ID and product ID.
**
**  @param      pPDD - pointer to the device record.
**
**  @return     UINT8 devType
**
******************************************************************************
**/
UINT8 SES_GetDeviceType(PDD* pPDD)
{
    UINT16  indx;
    UINT16  retValue = PD_DT_UNKNOWN;


    UINT32  vendIDLen = strnlen((char *)pPDD->vendID, 8);
    UINT32  prodIDLen = strnlen((char *)pPDD->prodID, 16);
// fprintf(stderr, "SES_GetDeviceType PID=%2d vendID=0x%02x%02x%02x%02x %02x%02x%02x%02x\n", pPDD->pid,
// pPDD->vendID[0], pPDD->vendID[1], pPDD->vendID[2], pPDD->vendID[3],
// pPDD->vendID[4], pPDD->vendID[5], pPDD->vendID[6], pPDD->vendID[7]);
// fprintf(stderr, "                         prodID=0x%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n",
// pPDD->prodID[0], pPDD->prodID[1], pPDD->prodID[2], pPDD->prodID[3],
// pPDD->prodID[4], pPDD->prodID[5], pPDD->prodID[6], pPDD->prodID[7],
// pPDD->prodID[8], pPDD->prodID[9], pPDD->prodID[10], pPDD->prodID[11],
// pPDD->prodID[12], pPDD->prodID[13], pPDD->prodID[14], pPDD->prodID[15]);

    if ( (vendIDLen > 0) && (prodIDLen > 0) )
    {
        for (indx = 0; indx <= SES_DIMEntries; ++indx)
        {
            if (!strncmp((char *)SES_DevInfoMaps[indx].devVendor, (char *)pPDD->vendID, (signed)vendIDLen) &&
               !strncmp((char *)SES_DevInfoMaps[indx].devProdID, (char *)pPDD->prodID, (signed)prodIDLen))
            {
                retValue = SES_DevInfoMaps[indx].devFlags[SES_DI_DEV_TYPE];
                break;
            }
        }
    }
    /*
    ** We want to be careful not to make an unknown bay into a drive.
    ** If the type is unknown, check the devtype that came from the drive.
    ** If it is a 0x0D (bay type from SCSI) or was previously set to the
    ** unknown type of bay, leave it as an unknown bay.
    */
    if (retValue == PD_DT_UNKNOWN)
    {
        if (pPDD->devType == PD_DT_BAY_UNKNOWN)
        {
            retValue = PD_DT_BAY_UNKNOWN;
        }
    }

    return(retValue);
}


/**
******************************************************************************
**
**  @brief      Process ports and enclosures to see if SBOD trunking is ok.
**
**              After getting enclosures, see if port 80/81 stats show that
**              SBOD trunking is done, and no JBOD/SATA, so we can use both
**              ports of both controllers to send/receive data to SBODs.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void ses_SBOD_trunking_flags(void)
{
    UINT8       port;
    UINT16      walkencl;
    PDD*        encl;
    UINT16      n_encl;
    UINT16      n_ff;                 /* all ports active */
    UINT16      n_99;                 /* ports 0, 3 active */
    UINT16      n_db;                 /* ports 0, 2, 3 active */
    UINT16      n_bd;                 /* ports 0, 1, 3 active */
    UINT16      n_bad;                /* number of sbods not above */
    UINT16      n_nonsbod;            /* number of non-sbods */

    for (port = 0; port < MAX_PORTS; port+=2)
    {
        n_encl = 0;
        n_ff = 0;
        n_99 = 0;
        n_db = 0;
        n_bd = 0;
        n_bad = 0;
        n_nonsbod = 0;
        for (walkencl = 0; walkencl < MAX_DISK_BAYS; walkencl++)
        {
            /*
             * Walk the enclosures checking for SBOD/JBOD/SATA and the cabling
             * done on the SBODs.
             */
            /* fprintf(stderr, "encl %d  encl=%p\n", walkencl, gEDX.pdd[walkencl]); */
            encl = gEDX.pdd[walkencl];
            if (encl != NULL)
            {
                /* fprintf(stderr, "    ->channel=%d  port=%d  (wwn=%qx)  pid=%d\n", encl->channel, port, encl->wwn, encl->pid); */

                /* Only those enclosures on port(s) we are checking. */
                if ((encl->channel & 0xfe) != port)        /* channel is UINT8 */
                {
                    /* fprintf(stderr, "    nomatch.\n"); */
                    continue;
                }

                /* fprintf(stderr, "    ->devType=%x  PD_DT_SBOD_SES=%x\n", encl->devType, PD_DT_SBOD_SES); */

                if (encl->devType != PD_DT_SBOD_SES)
                {
                    /* fprintf(stderr, "ses_SBOD_trunking_flags: not sbod = 0x%x (%d)\n", encl->devType, encl->devType); */
                    n_nonsbod++;
                }
                else
                {
                    n_encl++;
                    switch (encl->sbod_p_active)
                    {
                        case 0xff:
                            n_ff++;
                            break;

                        case 0x99:
                            n_99++;
                            break;

                        case 0xdb:
                            n_db++;
                            break;

                        case 0xbd:
                            n_bd++;
                            break;

                        default:
                            n_bad++;
                            fprintf(stderr, "Trunking - port %d active=0x%x, n_encl=%d\n", port, encl->sbod_p_active, n_encl);
                            break;
                    }
                }
            }
        }

        /* fprintf(stderr, "ports %d & %d, n_bad=%d, n_encl=%d, n_ff=%d, n_99=%d, n_db=%d, n_bd=%d\n", port, port+1, n_bad, n_encl, n_ff, n_99, n_db, n_bd); */

        /* We got our statistics, determine if trunking matches desired cabling plan. */
        if (n_nonsbod != 0 || n_bad > 0 || n_encl == 0)
        {
            if (n_encl != 0)
            {
                fprintf(stderr, "Trunking - port %d nonsbod=%d, bad=%d, 99=%d, db=%d, bd=%d, ff=%d\n", port, n_nonsbod,  n_bad, n_99, n_db, n_bd, n_ff);
            }
            SBOD_Trunking[port] = FALSE;
        }
        else
        {
            /* Check number and type of cablings. */
            if (n_encl == 1)
            {
                if (n_99 == 1)
                {
                    SBOD_Trunking[port] = TRUE;
                }
                else
                {
                    fprintf(stderr, "Trunking - port %d nonsbod=%d, bad=%d, 99=%d, db=%d, bd=%d, ff=%d\n", port, n_nonsbod,  n_bad, n_99, n_db, n_bd, n_ff);
                    SBOD_Trunking[port] = FALSE;
                }
            }
            else
            {
                if (n_99 == 0 &&
                    n_db == 1 &&
                    n_bd == 1 &&
                    n_ff == n_encl - 2)
                {
                    SBOD_Trunking[port] = TRUE;
                }
                else
                {
                    fprintf(stderr, "Trunking - port %d nonsbod=%d, bad=%d, 99=%d, db=%d, bd=%d, ff=%d\n", port, n_nonsbod,  n_bad, n_99, n_db, n_bd, n_ff);
                    SBOD_Trunking[port] = FALSE;
                }
            }
        }

        SBOD_Trunking[port+1] = SBOD_Trunking[port];
        fprintf(stderr, "SBOD_Trunking[%d and %d]=%s\n",
                port, port + 1, SBOD_Trunking[port] ? "TRUE" : "FALSE");
    }
}

/**
******************************************************************************
**
**  @brief      Process SES enclosure discovery in background.
**
**              This is a task spawned off during initial power
**              on and in hotswap operations.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
NORETURN
void SES_BackGroundProcess()
{
    UINT32     target;     /* One were looking for                 */
    UINT8      targetSlot; /* One were looking for                 */
    PDD*       targetPDD;  /* One were looking for                 */
    UINT32     victim;     /* One were looking at                  */
    PDD*       victimPDD;  /* One were looking at                  */

    UINT16     targetSES;  /* One were looking for                 */
    UINT8      lookAgain = FALSE;  /* Do another check                     */

    while (1)
    {
        /*
         * Go to sleep waiting for a signal to run. If the signal is already
         * present, then run.
         */
        if (!S_bgprun)
        {
            fprintf(stderr,"SES_BackGroundProcess sleeping\n");
            TaskSetMyState(PCB_NOT_READY);
            TaskSwitch();
        }

        fprintf(stderr,"SES_BackGroundProcess running\n");

        /* Set the flag to indicate that the processing ran. */
        S_bgprun = FALSE;

        /*
         * Assume all enclosures are gone. Check to see if this enclosure
         * is one that presents itself as an addressable device in the bus
         * and if so, do not nullify the devstat pointer or set to nonexistent.
         */
        for (targetSES = 0; targetSES < MAX_DISK_BAYS; targetSES++)
        {
            PDD* E_pdd = gEDX.pdd[targetSES];

            if (E_pdd != NULL)
            {
                E_pdd->slot = 0xFF;
                E_pdd->ses = 0xFFFF;

                if (ses_BayAttributeTest(E_pdd->vendID,E_pdd->prodID,SES_DI_MISC_DIRECT) < 0)
                {
                    E_pdd->devStat = PD_NONX;
                    E_pdd->pDev = NULL;
                }
            }
        }

        /*
         * This fragment of code creating problems when we are connected to ISE (Brick)
         * we are losing the ses/slot. We don't require this piece of code in ISE case.
         */
        for (targetSES = 0; targetSES < MAX_PHYSICAL_DISKS; targetSES++)
        {
            PDD* P_pdd = gPDX.pdd[targetSES];

            if (P_pdd != NULL)
            {
                P_pdd->slot = 0xFF;
                P_pdd->ses = 0xFFFF;
            }
        }

        /*
         * Locate the enclosures that are not directly addressable. The ones
         * that are directly addressable are already identified by the online
         * process. This will also determine which slots the drives are in.
         */
        ses_LocateEnclosures();

        /*
         * Make sure that there are no drives in the same slots. Check
         * through the entire list looking for overlaps. If there are
         * any, reset the enclosure information for the overlapping ones
         * and call the locate function again.
         */
        for (target = 0, lookAgain = FALSE; target < MAX_PHYSICAL_DISKS-1; target++)
        {
            if ((targetPDD = gPDX.pdd[target]) != NULL)
            {
                targetSlot = targetPDD->slot;
                targetSES = targetPDD->ses;

                if ((targetPDD->devStat != PD_NONX) &&
                   ((targetSlot == 0xFF) || (targetSES == 0xFFFF)))
                {
                    lookAgain = TRUE;
                }
                else
                {
                    for (victim = target + 1; victim < MAX_PHYSICAL_DISKS; victim++)
                    {
                        if (((victimPDD = gPDX.pdd[victim]) != NULL) &&
                            (victimPDD->ses == targetSES) &&
                            (victimPDD->slot == targetSlot))
                        {
                            victimPDD->ses = targetPDD->ses = 0xFFFF;
                            victimPDD->slot = targetPDD->slot = 0xFF;
                            lookAgain = TRUE;
                        }
                    }
                }
            }
        }

        if (lookAgain)
        {
            ses_LocateEnclosures();
        }

        /* Check for path violations (less than two paths to each device). */
        ses_CheckPaths();

        /* Look for missing enclosures. */
        ses_CheckMissingEncl();


        /* Update all bay locations before checking for cross insertion */
        for (targetSES = 0; targetSES < MAX_DISK_BAYS; targetSES++)
        {
            PDD* E_pdd = gEDX.pdd[targetSES];
            if (E_pdd != NULL)
            {
                GR_SetBayLocationCode(E_pdd);
            }
        }

        /*
         * Find if any of the drives is being inserted across the geo
         * location. If yes, send a log event to CCB informing that the
         * drive is being inserted across geo location and modify the
         * location ID of the drive being inserted to the location ID of
         * drive bay into which the insertion happend
         */
        GR_CheckForCrossLocationInsertion();

        /*
         * Log a configuration changed message and save to NVRAM if there
         * are no outstanding requests.
         */
        if (!S_bgprun)
        {
            NV_P2Update();
            ON_LogError(LOG_CONFIG_CHANGED);
        }

        /* Set SBOD trunking flags. */
        ses_SBOD_trunking_flags();

        /* Readjust load balancing (SBOD trunking check may allow it). */
        FAB_BalanceLoad();
    } /* while loop*/
}

/**
******************************************************************************
**
**  @brief      Check the disk drives to see if they have sufficient
**              paths to them.
**
**              Each disk drive is checked to see if there is at least
**              two paths to it. If there is only one, then a log message
**              is generated.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void ses_CheckPaths()
{
    UINT32 indx;               /* Pointer into drive list          */
    UINT32 paths;              /* Number of paths to the drive     */
    UINT32 channel;            /* Channel number                   */

    /* Make the check for disk drives. */
    for (indx = 0; indx < MAX_PHYSICAL_DISKS; indx++)
    {
        if ((gPDX.pdd[indx] != NULL) && (gPDX.pdd[indx]->devStat != PD_NONX))
        {
            /* Get the path count. */
            for (paths = 0, channel = 0; channel < MAX_PORTS; channel++)
            {
                if (gPDX.pdd[indx]->pDev->pLid[channel] != NO_CONNECT)
                {
                    paths++;
                }
            }

            if (paths == 1)
            {
                ON_LogDeviceSPath(gPDX.pdd[indx]);
            }
        }
    }
}


/**
******************************************************************************
**
**  @brief      Find any enclosures which are not directly addressable devices.
**
**              Each disk drive which is not in an enclosure is polled to see
**              if there is an enclosure controller behind it. For now, we
**              will onlysupport the Eurologic enclosure, so there is code
**              that is specific to it and may or may not work for other
**              enclosures.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void ses_LocateEnclosures()
{
    UINT16 driveIndex;         /* Pointer into drive list          */
    UINT16 enclIndex;          /* Pointer into enclosure list      */
    PDD*   pdd;                /* Disk drive pointer               */
    UINT32 tmpCnt;             /* Count of drives to do            */
    PDD*   *drvs;              /* Array of drives to do            */
    UINT8  *encCnt;            /* Array if enclosures being done   */
    UINT8  drivesToDo = FALSE; /* Are there drives to check?       */
    UINT8  enclToCheck = FALSE;/* Are there enclosures to check?   */
    UINT16 currEncl;           /* Best guess at current enclosure  */
    INT32  inqCnt = 0;         /* Count of inquiry tasks running   */
    INT32  directCnt = 0;      /* Cnt of direct addr tasks running */

    drvs = (PDD* *)s_MallocC(sizeof(PDD*) * (MAX_PHYSICAL_DISKS + MAX_DISK_BAYS), __FILE__, __LINE__);
    encCnt = (UINT8 *)s_MallocC(sizeof(UINT8) * (MAX_DISK_BAYS + 1), __FILE__, __LINE__);

    /*
     * Put each drive in the list of drives to do. We will pluck out each
     * one for a specified enclosure and run it in parallel.
     */
    for (driveIndex = tmpCnt = 0; driveIndex < MAX_PHYSICAL_DISKS; driveIndex++)
    {
        if (((pdd = gPDX.pdd[driveIndex]) != NULL) &&
           (pdd->ses == 0xFFFF) && (pdd->devStat != PD_NONX))
        {
            if (ses_BayAttributeTest(pdd->vendID, pdd->prodID, SES_DI_MISC_SLOT_INQ) < 0)
            {
                /*
                 * The drive slot is not determined by an Inquiry command, so
                 * add it into the table for checking through SES.
                 */
                drvs[tmpCnt++] = pdd;
                drivesToDo = TRUE;
            }
            else
            {
                /*
                 * The drive slot is determined via inquiry (SATA drive).
                 * Go ahead an fork the task to fetch the slot number.
                 */
                ++inqCnt;

                CT_fork_tmp = (ulong)"SES_GetSlotWithInq_task";
                TaskCreate4(C_label_referenced_in_i960asm(SES_GetSlotWithInq),
                            K_xpcb->pc_pri,
                            (UINT32)pdd,
                            (UINT32)&inqCnt);
             }
        }
    }

    /*
     * Also process bays that are directly addressable. These also can be
     * set off in a task directly since there is no contention for the SES
     * bus.
     */
    for (driveIndex = 0; driveIndex < MAX_DISK_BAYS; driveIndex++)
    {
        if (((pdd = gEDX.pdd[driveIndex]) != NULL) &&
           (pdd->ses == 0xFFFF) &&
           (pdd->devStat != PD_NONX) &&
           (ses_BayAttributeTest(pdd->vendID,pdd->prodID,SES_DI_MISC_DIRECT) >= 0))
        {
                /*
                 * Increment the direct addressable task count and fork
                 * the task.
                 */
                ++directCnt;

                CT_fork_tmp = (ulong)"SES_GetDirectEnclosure_task";
                TaskCreate5(C_label_referenced_in_i960asm(SES_GetDirectEnclosure),
                            K_xpcb->pc_pri,
                            (UINT32)pdd,
                            (UINT32)&directCnt,
                            (UINT32)FALSE);
        }
    }

    /*
     * Now do a drive for each known enclosure. If the previous enclosure
     * is not know, then put the drive in the last slot (all unknowns).
     */
    while (drivesToDo)
    {
        for (driveIndex = 0, drivesToDo = FALSE; driveIndex < tmpCnt; driveIndex++)
        {
            if ((pdd = drvs[driveIndex]) != NULL)
            {
                currEncl = pdd->devName[PD_DNAME_CSES];

                if (currEncl > MAX_DISK_BAYS)
                {
                    enclIndex = MAX_DISK_BAYS;
                }
                else
                {
                    enclIndex = currEncl;
                }

                if (encCnt[enclIndex] == 0)
                {
                    enclToCheck = TRUE;
                    encCnt[enclIndex] += 1;
                    drvs[driveIndex] = NULL;

                    CT_fork_tmp = (ulong)"SES_PollDriveForEnclosure_task";
                    TaskCreate5(C_label_referenced_in_i960asm(SES_PollDriveForEnclosure),
                            K_xpcb->pc_pri,
                            (UINT32)pdd,
                            (UINT32)&encCnt[enclIndex],
                            (UINT32)TRUE);
                }
                else
                {
                    drivesToDo = TRUE;
                }
            }
        }

        /* Sleep briefly to allow some completions. */
        TaskSleepMS(100);
    }

    /*
     * Now that all the drives have been queued up, wait for them all to
     * complete.
     */
    while (enclToCheck || (inqCnt > 0) || (directCnt > 0))
    {
        TaskSleepMS(100);
        enclToCheck = FALSE;
        for (enclIndex = 0; enclIndex < (MAX_DISK_BAYS + 1); enclIndex++)
        {
            if (encCnt[enclIndex] != 0)
            {
                enclToCheck = TRUE;
            }
        }
    }

    /* Release the memory used. */
    s_Free(drvs, sizeof(PDD*) * (MAX_PHYSICAL_DISKS + MAX_DISK_BAYS), __FILE__, __LINE__);
    s_Free(encCnt, sizeof(UINT8) * (MAX_DISK_BAYS + 1), __FILE__, __LINE__);
}

/**
******************************************************************************
**
**  @brief      Poll a SATA drive to see which enclosure contains the device.
**
**              The disk drive passed into the function is checked for the
**              slot information via Inquiry page 0x83. The counter of the
**              number of spawned tasks is decremented when complete so that
**              the parent task can complete.
**
**  @param      pdd     - the PDD to use.
**  @param      * cnt   - pointer to the count of drives being polled.
**
**  @return     none
**
******************************************************************************
**/
void SES_GetSlotWithInq(UINT32 foo UNUSED, UINT32 bar UNUSED,
                        PDD* pdd, volatile UINT32* cnt)
{
    INQ_PAGE_83*        page83 = NULL;
    INQ_P83_ID_DESC*    idDesc = NULL;
    SNS*                snsData = NULL;
    UINT16              enclIndex;
    PDD*                encl;

    /* Allocate a page to be used for the inquiry command. */
    page83 = (INQ_PAGE_83*)s_MallocW(0x100, __FILE__, __LINE__);


    if (SCR_OK == ON_SCSICmd(&gTemplateInqDevID, page83, 0xFF, pdd, &snsData))
    {
        /* Pull the data from the page. */
        for (idDesc = (INQ_P83_ID_DESC*)&page83->id[0];
            (UINT8*)idDesc < (UINT8*)page83 + page83->header.length + sizeof(INQ_HDR);
            idDesc = (INQ_P83_ID_DESC*)((UINT8 *)idDesc + idDesc->length + sizeof(INQ_P83_ID_DESC)))
        {
            if (idDesc->type == 0x10)
            {
                pdd->slot = idDesc->desc[7];

                for (enclIndex = 0; enclIndex < MAX_DISK_BAYS; enclIndex++)
                {
                    encl = gEDX.pdd[enclIndex];

                    if ((encl != NULL) &&
                       (encl->wwn == *(UINT64*)&(idDesc->desc[20])))
                    {
                        /* Found it in the current list. */
                        pdd->ses = enclIndex;
                        break;
                    }
                }
            }
        }
    }

    s_Free(page83, 0x100, __FILE__, __LINE__);

    /* Decrement the count of drives being checked. */
    if (cnt != NULL)
    {
        *cnt = *cnt - 1;
    }
}


/**
******************************************************************************
**
**  @brief      Poll a drive bay that is directly addressable to get the
**              pertinent SES information.
**
**  @param      pdd     - the PDD to use.
**  @param      * cnt   - pointer to the count of bays being polled.
**  @param      logEntry- Boolean, log or do not log new entry
**
**  @return     none
**
******************************************************************************
**/
void SES_GetDirectEnclosure(UINT32 foo UNUSED, UINT32 bar UNUSED,
                            PDD* pdd, volatile UINT32* cnt, UINT32 logEntry)
{
    UINT16          enclIndex;          /* Index into enclosure list        */
    PSES_PAGE_01    page1;              /* Page from enclosure              */
    UINT32          retCode;            /* Error code                       */
    UINT16          firstFree;          /* First free enclosure entry       */
    UINT16          newEnclID;          /* ID to use for new enclosure entry*/
    PSES_ENCL_DESC  enclDesc;           /* Pointer to enclosure descriptor  */

    /* Allocate a page to use throughout the function. */
    page1 = (PSES_PAGE_01)s_MallocW(0x800, __FILE__, __LINE__);

    /*
     * We have a drive bay that is directly addressable. Parse the pages
     * to get the select ID for drive bay name determination.
     */
    retCode = Get_SES_Page(&gTemplateSESP1Rd, (void *)page1, 0x800, pdd);

    if ((retCode == SCR_OK) && (page1->PageCode == 1) && (page1->Length != 0))
    {
        /*
         * We got a valid page one from the enclosure. Get the information
         * out of page one.
         */
        enclDesc = (PSES_ENCL_DESC)(page1 + 1);
        pdd->ses = 0xFFFF;
        if (ses_EuroLogicBay(enclDesc->VendorID, enclDesc->ProductID) >= 0)
        {
            PSES_EURO_ENCL_DESC_VU euroDesc;

            /* Grab the shelfID. */
            euroDesc = (PSES_EURO_ENCL_DESC_VU)(enclDesc + 1);
            pdd->slot = euroDesc->ShelfID;
        }

        if ( ses_AICBay(enclDesc->VendorID, enclDesc->ProductID) >= 0)
        {
            /*
             * Local bay for 750 is 0 external is 1, how do i tell them apart,
             * most likely will have different vendor product ids. SMW
             */
            pdd->slot = 0;//page1->SubEnclosureID;
        }

        for (firstFree = 0xFFFF, enclIndex = 0; enclIndex < MAX_DISK_BAYS; enclIndex++)
        {
            if (gEDX.pdd[enclIndex] == NULL)
            {
                if (firstFree == 0xFFFF)
                {
                    firstFree = enclIndex;
                }
            }
            else
            {
                if (gEDX.pdd[enclIndex] == pdd)
                {
                    /* Found it in the current list. */
                    break;
                }
            }
        }

        /*
         * When allocating an enslosure and assigning an ID, select
         * the enclosure based upon the channel and the slot number.
         * We assume that cards 0 and 1, 2 and 3 are wired to the
         * same enclosures, so pick the number based upon the slot
         * plus the second least significant bit of channel times the
         * number of unique shelf IDs (eight). If the ID is already
         * used by someone else (probably conflicting enclosure thumb
         * wheels) then take the one that is available that was
         * selected above.
         */
        fprintf(stderr,"SES_GetDirectEnclosure: %hd %04hX\n",enclIndex,firstFree);
        if ((enclIndex == MAX_DISK_BAYS) && (firstFree != 0xFFFF))
        {
            fprintf(stderr,"SES_GetDirectEnclosure: adding to gedx\n");

            /* Determine ID. */
            newEnclID = pdd->slot + ((pdd->channel >> 1) * MAX_BAYS_PORT);

            if ((newEnclID >MAX_DISK_BAYS) ||(gEDX.pdd[newEnclID] != NULL))
            {
                newEnclID = firstFree;
            }

            gEDX.count++;
            gEDX.pdd[newEnclID] = pdd;
            pdd->pid = newEnclID;
            pdd->devType = SES_GetDeviceType(pdd);

            /*
             * If the caller was requesting that this enclosure be logged
             * if new, then log it.
             */
            if (logEntry)
            {
                ON_LogBayInserted(pdd);
            }
        }
    }

    /* Free the memory used. */
    s_Free(page1, 0x800, __FILE__, __LINE__);

    /* Decrement the count of drives being checked. */
    if (cnt != NULL)
    {
        *cnt = *cnt - 1;
    }
}

/**
******************************************************************************
**
**  @brief      Poll a single drive to see which enclosure contains the device.
**
**              The drives passed into this function will be drives that are
**              in an enclosure that requires SES to talk to the bay, i.e. it
**              is not a directly addressable bay. Any drive that are in a
**              directly addressable bay are parsed out before we get to this
**              function.
**
**  @param      pdd     - the PDD to use.
**  @param      * cnt   - pointer to the count of drives being polled.
**  @param      logEntry- Boolean, log or do not log new entry
**
**  @return     none
**
******************************************************************************
**/

void SES_PollDriveForEnclosure(UINT32 foo UNUSED, UINT32 bar UNUSED,
                               PDD* pdd, volatile UINT32* cnt, UINT32 logEntry)
{
    UINT16          enclIndex;          /* Index into enclosure list        */
    PDD*            encl = NULL;        /* Enclosure pointer                */
    PSES_PAGE_01    page1;              /* Page from enclosure              */
    PSES_PAGE_04    page4 = NULL;       /* Page from enclosure              */
    UINT32          retCode;            /* Error code                       */
    UINT16          firstFree;          /* First free enclosure entry       */
    UINT16          newEnclID;          /* ID to use for new enclosure entry*/
    UINT8           newEncl = FALSE;    /* Is this a new enclosure          */
    PSES_ENCL_DESC  enclDesc;           /* Pointer to enclosure descriptor  */

    UINT64          WWN;
    UINT8           ShelfID;
    UINT8           BackPlaneSerial[12];/* lower 12 back plane serial number*/

    /* Allocate a page to use throughout the function. */
    page1 = (PSES_PAGE_01)s_MallocW(0x800, __FILE__, __LINE__);

    /*
     * We have a drive that is not in an enclosure which is addressable. We
     * must query page 1 of the SES information. Drives in a Xyratex bay
     * will only be able to repsond on two of the slots, so we may get a lot
     * of messages from the other drives indicating an error.
     *
     * Once we have the page, we can tell how to continue the processing of
     * the drives. It will either be through page 4 for Adaptec/Eurologic
     * boxes or through a VU page for Xyratex. (for now it will be using
     * hard addressing and page 2 for Xyratex).
     */
    retCode = Get_SES_Page(&gTemplateSESP1Rd, (void *)page1, 0x800, pdd);

    if ((retCode == SCR_OK) && (page1->PageCode == 1) && (page1->Length != 0))
    {
        /*
         * We got a valid page one from the drive, indicating there's an enclosure
         * behind it. Get the information out of page one to determine if the page
         * is from a defined, supported enclosure by matching the type against the
         * predefined enclosure list. If a match is found, use it. If not, create
         * one and continue.
         */
        enclDesc = (PSES_ENCL_DESC)(page1 + 1);

        if (ses_EuroLogicBay(enclDesc->VendorID, enclDesc->ProductID) >= 0)
        {
            PSES_EURO_ENCL_DESC_VU euroDesc;

            euroDesc = (PSES_EURO_ENCL_DESC_VU)(enclDesc + 1);
            WWN = euroDesc->WWN;
            ShelfID = euroDesc->ShelfID;

            /*
             * The serial number for the box is the backplane serial
             * number. It is the last eight/twelve characters of the 16.
             */
            memset(BackPlaneSerial, ' ', 12);
            strncpy((char *)BackPlaneSerial, (char *)&euroDesc->BackPlaneSerial[8], 8);

#ifdef PRINT_SERIAL_NUMBER
            fprintf(stderr,
                    "Eurologic Back Plane Serial Number (%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c)\n",
                    euroDesc->BackPlaneSerial[0],
                    euroDesc->BackPlaneSerial[1],
                    euroDesc->BackPlaneSerial[2],
                    euroDesc->BackPlaneSerial[3],
                    euroDesc->BackPlaneSerial[4],
                    euroDesc->BackPlaneSerial[5],
                    euroDesc->BackPlaneSerial[6],
                    euroDesc->BackPlaneSerial[7],
                    euroDesc->BackPlaneSerial[8],
                    euroDesc->BackPlaneSerial[9],
                    euroDesc->BackPlaneSerial[10],
                    euroDesc->BackPlaneSerial[11],
                    euroDesc->BackPlaneSerial[12],
                    euroDesc->BackPlaneSerial[13],
                    euroDesc->BackPlaneSerial[14],
                    euroDesc->BackPlaneSerial[15]);
#endif
        }
        else
        {
            PSES_XTEX_ENCL_DESC_VU xtexDesc;

            xtexDesc = (PSES_XTEX_ENCL_DESC_VU)(enclDesc + 1);
            WWN = *(UINT64 *)(&enclDesc->WWN[0]);
            ShelfID = xtexDesc->EnclConfig >> 2;

            /*
             * The serial number for the box is the backplane serial
             * number. It is the last 12 characters of the 15.
             */
/*             memset(BackPlaneSerial, ' ', 12); */
            strncpy((char *)BackPlaneSerial, (char *)&xtexDesc->ProductSer[3], 12);

#ifdef PRINT_SERIAL_NUMBER
            fprintf(stderr,
                    "Xyratex Back Plane Serial Number (%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c)\n",
                    xtexDesc->ProductSer[0],
                    xtexDesc->ProductSer[1],
                    xtexDesc->ProductSer[2],
                    xtexDesc->ProductSer[3],
                    xtexDesc->ProductSer[4],
                    xtexDesc->ProductSer[5],
                    xtexDesc->ProductSer[6],
                    xtexDesc->ProductSer[7],
                    xtexDesc->ProductSer[8],
                    xtexDesc->ProductSer[9],
                    xtexDesc->ProductSer[10],
                    xtexDesc->ProductSer[11],
                    xtexDesc->ProductSer[12],
                    xtexDesc->ProductSer[13],
                    xtexDesc->ProductSer[14]);
#endif
        }

        /*
         * We now have the WWN of the bay. See if it is already in the
         * list of bays we have already seen. If not, enter it into the
         * tables.
         */
        for (firstFree = 0xFFFF, enclIndex = 0;
            enclIndex < MAX_DISK_BAYS;
            enclIndex++, encl = NULL)
        {
            if ((encl = gEDX.pdd[enclIndex]) == NULL)
            {
                if (firstFree == 0xFFFF)
                {
                    firstFree = enclIndex;
                }
            }
            else
            {
                if (encl->wwn == WWN)
                {
                    /*
                    ** Found it in the current list.
                    */
                    break;
                }
            }
        }

        /*
         * Now, if the device was found, record this information
         * in the PDD that we were examining. If it was not found,
         * then allocate a PDD and put the information in the PDD.
         *
         * When allocating an enslosure and assigning an ID, select
         * the enclosure based upon the channel and the slot number.
         * We assume that cards 0 and 1, 2 and 3 are wired to the
         * same enclosures, so pick the number based upon the slot
         * plus the second least significant bit of channel times the
         * number of unique shelf IDs (eight). If the ID is already
         * used by someone else (probably conflicting enclosure thumb
         * wheels) then take the one that is available that was
         * selected above.
         */
        if ((enclIndex == MAX_DISK_BAYS) && (firstFree != 0xFFFF))
        {
            /* Determine ID. */
            newEnclID = ShelfID + ((pdd->channel >> 1) * MAX_BAYS_PORT);

            if (gEDX.pdd[newEnclID] != NULL)
            {
                newEnclID = firstFree;
            }

            gEDX.count++;

            /*
             * Since we cannot talk directly to the device (bay), allocate
             * a new pdd.
             */
            encl = DC_AllocPDD();
            gEDX.pdd[newEnclID] = encl;
            encl->pid = newEnclID;
            encl->devType = SES_GetDeviceType(pdd);
            newEncl = TRUE;
        }

        /*
         * Update the enclosure structure if it is not NULL. It will
         * only be NULL if we have exceeded the max number of enclosures
         * and there weren't any slots available (really at MAX).
         */
        if (encl != NULL)
        {
            encl->slot = ShelfID;
            encl->ses = 0xFFFF;

            /* Now fill in the common fields. */
            encl->miscStat = 0;
            encl->postStat = encl->devStat = PD_OP;
            encl->channel = pdd->channel;
            encl->lun = pdd->lun;
            encl->id = pdd->id;
            encl->pDev = pdd->pDev;
            encl->wwn = WWN;

            strncpy((char *)encl->serial, (char *)&BackPlaneSerial[0], 12);

            memcpy(encl->prodRev, enclDesc->ProductRev, 4);

            /*
             * Hack the vendor ID and product ID in the case of a Xyratex
             * bay. This is done since we did not get the name changed
             * natively by Xyratex.
             */
            if ((strncmp((char *)enclDesc->VendorID, XYRATEX_VEND_ID, 8) == 0) &&
               ((strncmp((char *)enclDesc->ProductID, XYRATEX_PROD_ID0, 16) == 0) ||
                (strncmp((char *)enclDesc->ProductID, XYRATEX_PROD_ID1, 16) == 0)) )
            {
                strncpy((char *)encl->vendID, (void*)XYRATEX_VEND_ID_REP, 8);
                strncpy((char *)encl->prodID, (void*)XYRATEX_PROD_ID0_REP, 16);
            }
            else if ((strncmp((char *)enclDesc->VendorID, XYRATEX_VEND_ID, 8) == 0) &&
                    ((strncmp((char *)enclDesc->ProductID, XYRATEX_PROD_ID2, 16) == 0) ||
                     (strncmp((char *)enclDesc->ProductID, XYRATEX_PROD_ID3, 16) == 0)) )
            {
                strncpy((char *)encl->vendID, (void*)XYRATEX_VEND_ID_REP, 8);
                strncpy((char *)encl->prodID, (void*)XYRATEX_PROD_ID1_REP, 16);
            }
            else
            {
                strncpy((char *)encl->vendID, (char *)enclDesc->VendorID, 8);
                strncpy((char *)encl->prodID, (char *)enclDesc->ProductID, 16);
            }

            encl->devType = SES_GetDeviceType(encl);

            /*
             * If this was a new enclosure, log it. Also set the flag to
             * cause the background processing to run again to pick up this
             * new enclosure.
             */
            if (newEncl)
            {
                if (logEntry)
                {
                    ON_LogBayInserted(encl);
                }
                S_bgprun = TRUE;
            }
        }

        /*
         * Now we have the new bay entered into the tables or the old bay
         * was already there. From this point, we now have to figure out
         * the slot number for the drive that was passed into the function.
         */
        if (encl->devType == PD_DT_SBOD_SES)
        {
            ses_XyratexLookup(pdd, encl);
        }
        else
        {
            /* Adaptec/Eurologic bay. Use page 4 to find the slot. */
            page4 = (PSES_PAGE_04)page1;

            retCode = Get_SES_Page(&gTemplateSESP4Rd, page4, 0x800, pdd);

            if ((retCode == SCR_OK) && (page4->PageCode == 4) && (page4->Length != 0))
            {
                pdd->ses = encl->pid;

                /*
                 * The format of the string returned is "Drive DD" followed
                 * by a bunch of junk. Look for the DD and convert to a
                 * short.
                 */
                pdd->slot = my_atob((UINT8*)((UINT32)(page4 + 1) + 6), NULL, 10);
            }
        }
    }

    /* Free the memory used. */
    s_Free(page1, 0x800, __FILE__, __LINE__);

    /* Decrement the count of drives being checked. */
    if (cnt != NULL)
    {
        *cnt = *cnt - 1;
    }
}


/**
******************************************************************************
**
**  @brief      Access and check mode page.
**
**  @param      pdd     - the PDD being checked at the time
**  @param      prp     - the PRP for the command
**  @param      pageno  - the number of the page being accessed
**  @param      page    - storage provided for the page
**  @param      delay   - delay to retry in milliseconds
**
**  @return     Error count, 5 if unrecovered error
**
******************************************************************************
**/

static int get_xtex_page(PDD *pdd, PRP_TEMPLATE *prp, UINT32 pageno,
        SES_P86_XTEX *page, int delay)
{
    int     errCnt = 0;
    bool    invchk = pageno == 0x86 || pageno == 0x87;

    while (errCnt < 5)
    {
        if (SCR_OK != Get_SES_Page(prp, page, 0x800, pdd))
        {
            ++errCnt;
            continue;
        }

        /* A couple simple checks on data returned. */
        if (page->PageCode != pageno || page->Length == 0 ||
            (invchk && (page->Flags & SES_XTEX_P86_INV) != 0))
        {
            if (delay)
            {
                TaskSleepMS(delay);
                ++errCnt;
                continue;
            }
            return 5;               /* Flag error */
        }
        /* else, everything is ok. */
        return errCnt;
    }

    return 5;
}


/**
******************************************************************************
**
**  @brief      Handle old Xyratex bays without page 86 support
**
**  @param      pdd     - the PDD being checked at the time
**  @param      encl    - the enclosure the drive is in
**  @param      page    - buffer to use to access page
**
**  @return     none
**
******************************************************************************
**/

static void xtex_no_page86(PDD *pdd, PDD *encl, SES_P86_XTEX *page)
{
    UINT32  outer_indx, inner_indx, drv_sel_id;
    int     errCnt;
    UINT16  drv_ses;
    UINT8   drv_chan;
    SES_P2_CTRL_XTEX    *page2;

    /* fprintf(stderr, "%s: page->Length=%d, page->Flags=0x%x, SES_XTEX_P86_INV=0x%x\n", __func__, page->Length, page->Flags, SES_XTEX_P86_INV); */

    errCnt = get_xtex_page(pdd, &gTemplateSESP2Rd, 2, page, 0);
    if (errCnt >= 5)
    {
        return;
    }

    page2 = (SES_P2_CTRL_XTEX *)page;

    /* Get the channel of the current PDD. */
    drv_chan = pdd->channel;
    drv_ses = encl->pid;

    /*
     * For each of the drives in this enclosure find
     * the corresponding PDD struct and set SES and slot.
     */
    for (outer_indx = 0; outer_indx < 16; ++outer_indx)
    {
        /* Extract the SCSI select id. */
        drv_sel_id = (page2->DevCtrl[outer_indx] & 0x0000ff00) >> 8;

        /*
         * Now search for the PDD for this drive, if found
         * set the ses and slot.
         */
        for (inner_indx = 0; inner_indx < MAX_PHYSICAL_DISKS; ++inner_indx)
        {
            if (gPDX.pdd[inner_indx] == NULL)
            {
                continue;
            }

            if (gPDX.pdd[inner_indx]->id == drv_sel_id)
            {
                /*
                 * This is to make cabling of SBODs known
                 * by which port (channel) on the BE an
                 * SBOD is located on. If on 0 and 1, then
                 * those are the same. 2 & 3 same.
                 * Make 0 and 1 match, 2 and 3 match.
                 */
                if ((gPDX.pdd[inner_indx]->channel >> 1) == (drv_chan >> 1))
                {
                    /*
                     * The channel and select ID match so
                     * update SES and slot
                     */
                    gPDX.pdd[inner_indx]->slot = outer_indx;
                    gPDX.pdd[inner_indx]->ses = drv_ses;
                    break;
                }
            }
        }
    }
}


/**
******************************************************************************
**
**  @brief      Lookup the slots for a Xyratex bay.
**
**              Parse through the list of enclosures and log any
**              missing enclosures.
**
**  @param      pdd     - the PDD being checked at the time
**  @param      encl    - the enclosure the drive is in
**
**  @return     modifies a number of PDDs for slot information
**
******************************************************************************
**/

void ses_XyratexLookup(PDD* pdd, PDD* encl)
{
    SES_P86_XTEX    *route;
    SES_P86_XTEX    *routeP;
    SES_P86_XTEX    *routeS;
    SES_P86_XTEX    *route1;
    SES_P86_XTEX    *route2;
    UINT32          errCnt, indx;
    PDB             *db;
    UINT32          drv_da, drv_daP, drv_daS;
    UINT8           drv_chan, drv_chanP, drv_chanS;
    UINT32          tmp_da;
    UINT32          slot, endSlot;
    bool            page86Supported;
    bool            secondarySet = false;
    UINT8           i;
    UINT8           sbod_ports;

    /*
     * Xyratex bay. Set up all the addresses for the bay if they
     * have not been set up yet.
     */
    if (pdd->slot != 0xFF)
    {
        return;
    }

    /* fprintf(stderr, "entering encl->sbod_p_active=%x, pid=%d, encl=%p\n", encl->sbod_p_active, encl->pid, encl); */

    /*
     * Preset to no ports ok. Note: do not change memory value until scsi
     * commands are done.
     */
    sbod_ports = 0;

    route1 = (SES_P86_XTEX *)s_MallocW(0x800, __FILE__, __LINE__);
    route2 = (SES_P86_XTEX *)s_MallocW(0x800, __FILE__, __LINE__);

    /*
     * We may get Flags & SES_XTEX_P86_INV == 1 -- if the SBOD
     * Modules are passing information between themselves after
     * a LIP has occurred, so retry after 3 second delay.
     */
    errCnt = get_xtex_page(pdd, &gTemplateSESP86Rd, 0x86, route1, 3000);
    page86Supported = errCnt < 5;

    if (!page86Supported)
    {
        xtex_no_page86(pdd, encl, route1);
        goto out;
    }

    /* fprintf(stderr, "ses_XyratexLookup: page86Supported\n"); */

    /*
     * Fetch the second route page. We only need one of them unless
     * we run into a bypassed port. In that case we will need the
     * other page, so it is easier to just grab it right away.
     */

    errCnt = get_xtex_page(pdd, &gTemplateSESP87Rd, 0x87, route2, 3000);

    /* If we can no longer talk to the device, exit. */
    if (pdd->pDev->port >= MAX_PORTS)
    {
        fprintf(stderr,"<INVDEVPORT>-ses_XyratexLookup:port in dev structure out of range %d\n", pdd->pDev->port);
        goto out;
    }

    /*
     * Look at the PDD port information. If the port we are using to
     * talk to the enclosure is a 1, then we are going to fetch route
     * information from the A port of the enclosure. If the port is
     * a 2, then it is the B port.
     */
    if (pdd->pDev->dvPort[pdd->pDev->port] == 1)
    {
        routeP = route1;
        routeS = route2;
    }
    else
    {
        routeP = route2;
        routeS = route1;
    }

    /*
     * If the pages could not be fetched, then just exit. This should not
     * be the case, but.
     */
    if (errCnt == 5)
    {
        fprintf(stderr, "%s: SES P87 fetch routing information failed\n",
            __func__);
        goto out;
    }

    /*
     * We have the route information given the channel we are using
     * for this PDD. Find out if this drive is in the first or the
     * last slot and record it. Then parse out the rest in this bay.
     */
    pdd->ses = encl->pid;

    if (routeP->Drives[0].Flags & SES_XTEX_P86_REP)
    {
        pdd->slot = 0;

        slot = 1;
        endSlot = SES_XTEX_SLOTS - 1;
    }
    else
    {
        pdd->slot = SES_XTEX_SLOTS - 1;

        slot = 0;
        endSlot = SES_XTEX_SLOTS - 2;
    }

    /*
     * Now search the port data base for the channel of the PDD
     * being used to talk to the enclosure. Find all drives that
     * match the area and domain (zeros for non-fabric) and that
     * have the same AL_PA as in the route map.
     */
    drv_chanP = pdd->pDev->port;
    drv_daP   = ((portdb[drv_chanP][pdd->id].pid & 0xff000000) >> 24) +
                ((portdb[drv_chanP][pdd->id].pid & 0x0000ffff) << 8);

    drv_chanS = drv_chanP;      /* Set to avoid stupid compiler message */
    drv_daS = drv_daP;          /* Set to avoid stupid compiler message */

    /* Get the secondary channel and DA. */
    for (indx = 0; indx < MAX_PORTS; indx++)
    {
        if (indx != pdd->pDev->port && pdd->pDev->dvPort[indx] != 0)
        {
            drv_chanS = indx;
            drv_daS   = ((portdb[drv_chanS][pdd->id].pid & 0xff000000) >> 24) +
                        ((portdb[drv_chanS][pdd->id].pid & 0x0000ffff) << 8);
            secondarySet = true;
        }
    }

    for ( ; slot != endSlot + 1; ++slot)
    {
        if (routeP->Drives[slot].ALPA != 0xFF)
        {
            drv_chan = drv_chanP;
            drv_da   = drv_daP;
            route    = routeP;
        }
        else if (!secondarySet)
        {
            continue;
        }
        else
        {
            drv_chan = drv_chanS;
            drv_da   = drv_daS;
            route    = routeS;
        }

        for (indx = 0; indx < MAX_PHYSICAL_DISKS; ++indx)
        {
            if (gPDX.pdd[indx] == NULL || gPDX.pdd[indx]->pDev == NULL)
            {
                continue;
            }

            db = portdb[drv_chan] + gPDX.pdd[indx]->pDev->pLid[drv_chan];
            if (!db)
            {
                fprintf(stderr, "%s: db==0, portdb[%d]=%p, indx=%d, "
                    "offset=%d\n",
                    __func__, drv_chan, portdb[drv_chan], indx,
                    gPDX.pdd[indx]->pDev->pLid[drv_chan]);
                continue;
            }

            tmp_da = ((db->pid & 0xff000000) >> 24) +
                     ((db->pid & 0x0000ffff) << 8);

            if (tmp_da == drv_da)
            {
                if (((db->pid & 0xFF0000) >> 16) == route->Drives[slot].ALPA)
                {
                    /*
                     * This is the drive in the slot. Now find it
                     * in the PDD list and assign the slot and SES
                     * to it.
                     */
                    gPDX.pdd[indx]->ses = encl->pid;
                    gPDX.pdd[indx]->slot = slot;
                    break;
                }
            }
        }
    }

    {
        SES_P80_XTEX    *page80;
        SES_P80_XTEX    *page81;

        errCnt = get_xtex_page(pdd, &gTemplateSESP80Rd, 0x80, route1, 0);
        if (errCnt >= 5)
        {
            fprintf(stderr, "%s: SES P80 fetch routing information failed\n",
                __func__);
            goto out;
        }
        page80 = (SES_P80_XTEX *)route1;

        errCnt = get_xtex_page(pdd, &gTemplateSESP81Rd, 0x81, route2, 0);
        if (errCnt >= 5)
        {
            fprintf(stderr, "%s: SES P81 fetch routing information failed\n",
                __func__);
            goto out;
        }
        page81 = (SES_P80_XTEX *)route2;

        /* Save the port bits for SBOD in encl->sbod_p_active. */
        for (i = 0; i < 4; ++i)
        {
            if (page80->Port[i].StateCode == 0)
            {
                sbod_ports |= 1 << i;
            }
            if (page81->Port[i].StateCode == 0)
            {
                sbod_ports |= 1 << (i + 4);
            }
        }
    }

out:
    encl->sbod_p_active = sbod_ports;
/* fprintf(stderr, "ses_XyratexLookup: sbod_ports=%x, pid=%d, encl=%p, errCnt=%d\n", sbod_ports, encl->pid, encl, errCnt); */

    /* Free the memory used. */
    s_Free(route1, 0x800, __FILE__, __LINE__);
    s_Free(route2, 0x800, __FILE__, __LINE__);
}

/**
******************************************************************************
**
**  @brief      Check for any enclosures which are non-existent.
**
**              Parse through the list of enclosures and log any
**              missing enclosures.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void ses_CheckMissingEncl(void)
{
    PDD*   enc;                 /* Enclosure being checked                  */
    PDD*   moveEnc;             /* Enclosure being moved                    */
    PDD*   chkDrv;              /* Drive being checked                      */
    UINT16 encIndx;             /* Pointer into enclosure table             */
    UINT16 moveIndx;            /* Pointer into enclosure table             */
    UINT16 chkIndx;             /* Pointer into enclosure table             */
    UINT16 drvCnt;              /* Count of drives in bay                   */
    UINT8  substitute;          /* Was there a substitution                 */

    for (encIndx = 0; encIndx < MAX_DISK_BAYS; encIndx++)
    {
        if (((enc = gEDX.pdd[encIndx]) != NULL) &&
            (enc->devStat == PD_NONX) &&
            !FAB_IsDevInUse(enc->pDev))
        {
            /* Set to indicate that was no switch. */
            substitute = FALSE;

            /* Log missing device. */
            ON_LogBayMissing(enc);

            /*
             * Do a search for an enclosure that may have been intended to
             * replace this one. Look for an enclosure that meets the following
             * criteria.
             *
             *  1)  Channel pair (0/1 or 2/3) matches the bay being removed.
             *  2)  Shelf ID matches the bay being removed.
             *  3)  Any drive in the bay has the devName of PDxy where x is
             *      the bay ID being removed.
             *
             *  If all of these are true, then log the removal of the other
             *  bay and it's addition back in this slot.
             */
            for (moveIndx = 0; moveIndx < MAX_DISK_BAYS; moveIndx++)
            {
                if (((moveEnc = gEDX.pdd[moveIndx]) != NULL) &&
                    (moveIndx != encIndx))
                {
                    /* Check if they are on the same channel pair. */
                    if ((moveEnc->channel >> 1) == (enc->channel >> 1))
                    {

                        /*
                         * Check if they are on the same shelf ID. If they
                         * are, then see if the drive being used to communicate
                         * with the enclosure is labelled to the old address.
                         * If so, swap them.
                         */
                        if (moveEnc->slot == (encIndx % MAX_BAYS_PORT))
                        {
                            /*
                             * Now check how many drives in the bay are supposed
                             * to be in the bay that was removed. If the count
                             * is non-zero, then we will move this bay into the
                             * spot for the other one.
                             */
                            drvCnt = 0;

                            for (chkIndx = 0; chkIndx < MAX_PHYSICAL_DISKS; chkIndx++)
                            {
                                chkDrv = gPDX.pdd[chkIndx];
                                if ((chkDrv != NULL) &&
                                   (chkDrv->devName[PD_DNAME_CSES] == encIndx))
                                {
                                    drvCnt++;
                                }
                            }

                            if (drvCnt > 0)
                            {
                                S_bgprun = TRUE;
                                gEDX.pdd[encIndx] = moveEnc;
                                gEDX.pdd[moveIndx] = NULL;
                                moveEnc->pid = encIndx;
                                ON_LogBayMoved(moveEnc);
                                substitute = TRUE;
                                gEDX.count--;
                                if (enc->pDev != NULL)
                                {
                                    s_Free(enc->pDev, sizeof(DEV), __FILE__, __LINE__);
                                }
                                DC_RelPDD(enc);
                            }
                        }
                    }
                }
            }

            /*
             * If there was no substitution for this bay, then check to
             * see if there are any drives in the bay that are still in
             * the system. If so, then do not delete the bay.
             */
            if (!substitute)
            {
                drvCnt = 0;

                for (chkIndx = 0; chkIndx < MAX_PHYSICAL_DISKS; chkIndx++)
                {
                    chkDrv = gPDX.pdd[chkIndx];

                    if ((chkDrv != NULL) && (chkDrv->ses == encIndx))
                    {
                        drvCnt++;
                    }
                }

                if ((drvCnt == 0) && !FAB_IsDevInUse(enc->pDev))
                {
                    /* Free the slot for this bay. */
                    gEDX.pdd[encIndx] = NULL;
                    gEDX.count--;

                    /* Delete it since it is not reporting. */
                    if (enc->pDev != NULL)
                    {
                        s_Free(enc->pDev, sizeof(DEV), __FILE__, __LINE__);
                    }

                    DC_RelPDD(enc);
                }
            }
        }
    }
}

/*----------------------------------------------------------------------------
** Function Name: SES_BypassCtrl()
**
** Comments:
**  This function will send a 'drive delay' log message to the CCB. In later
**  releases, this function will attempt to bypass a drive.
**
** Input:
**  pdd       - PDD of the drive to bypass
**
**--------------------------------------------------------------------------*/
void SES_BypassCtrl(struct PDD *pdd)
{
    PSD              *psd;
    PDD              *baypdd;
    UINT16            drvindex;
    UINT32            lowcnt = 0;     /* Number of drives with a low hangcnt */
    UINT32            hicnt = 0;      /* Number of drives with a high hangcnt */

    /*
     * Look through each PDisk, if any has a hangcount of >= 5, log a message
     * in the apps log.
     */
    for (drvindex = 0; drvindex < MAX_PHYSICAL_DISKS; drvindex++)
    {
        /* Check for a valid PDD */
        if (gPDX.pdd[drvindex] != 0)
        {
            /* Check for hangcount of >= low hang thresold */
            if (gPDX.pdd[drvindex]->hangCount >= LOW_HANG_THRESHOLD)
            {
                fprintf(stderr,"SES_BypassCtrl: PDisk %c%02d, WWN %qx, HangCount = %d\n", ('A' + gPDX.pdd[drvindex]->ses),
                                                                                            gPDX.pdd[drvindex]->slot,
                                                                                            gPDX.pdd[drvindex]->wwn,
                                                                                            gPDX.pdd[drvindex]->hangCount);
                lowcnt++;

                /*
                 * If there is more than 1 drive with a > high hangcnt, then
                 * don't bypass any drives.
                 */
                if (gPDX.pdd[drvindex]->hangCount > HIGH_HANG_THRESHOLD)
                {
                    hicnt++;
                }
            }
        }
    }

    /*
     * Check to see if the drive is a 750 Gb SATA drive, if it is don't log
     * a message. Also, if there are more than 1 drive with a hangcnt of 15
     * or more, don't bypass any drives.
     */
    if ((SES_GetDeviceType(pdd) != PD_DT_SATA) && (hicnt <= 1))
    {

        /* Log drive delay message */
        ON_LogDriveDelay(pdd);

        /* Check to see if bypassing this drive will cause a raid to go inoperative. */
        if ((psd = DC_ConvPDD2PSD(pdd)) != 0)
        {
            /*
             * Now that we know that the drive is part of at least one raid, lets
             * check for redundancy.
             */
            if (RB_CanSpare(psd) != DEOK)
            {
                /*
                 * There is not enough redundancy to bypass this drive.
                 * Log the Disk Delay INOP error message.
                 */
                ON_LogError(LOG_DELAY_INOP);

                /* Simply return */
                return;
            }
        }

        /*
         * Only bypass the drive if it's in an SBOD drivebay and we are the master
         * controller.
         */
        if ((pdd->ses < MAX_DISK_BAYS) && ((baypdd = gEDX.pdd[pdd->ses]) != NULL) &&
            (baypdd->devStat == PD_OP) && (BIT_TEST(K_ii.status, II_MASTER)))
        {
            if (baypdd->devType == PD_DT_SBOD_SES)
            {
                /*
                 * Fork off ses_BypassDoubleCheck which will wait 2 secs and then
                 * check if there are any other drives with midcnt.
                 */
                CT_fork_tmp = (ulong)"ses_BypassDoubleCheck";
                TaskCreate5(C_label_referenced_in_i960asm(ses_BypassDoubleCheck),
                                                          K_xpcb->pc_pri,
                                                          (UINT32)pdd,
                                                          (UINT32)pdd->ses,
                                                          (UINT32)pdd->slot);
            }
        }
    }
}

/**
******************************************************************************
** Function Name: SES_UpdatePages()
**
**  @brief      A routine running in the background that retrieves the latest
**              contents of SES pages 82 and 83 from each drivebay.
**
**              This is a task spawned off during initial power
**              on.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
NORETURN
void SES_UpdatePages(void)
{
    UINT8       bayIndex;
    UINT8       first_loops = 0;
    PDD         *baypdd;

fprintf(stderr, "Begin SES_UpdatePages\n");
    for (bayIndex = 0; bayIndex < MAX_DISK_BAYS; bayIndex++)
    {
        SES_Page82[bayIndex] = NULL;
        SES_Page83[bayIndex] = NULL;
    }

    while (1)
    {
        if (first_loops < 3)
        {
            TaskSleepMS(60000);
            ++first_loops;
        }
        else
        {
            TaskSleepMS(240000);
        }

        for (bayIndex = 0; bayIndex < MAX_DISK_BAYS; bayIndex++)
        {
            if ((baypdd = gEDX.pdd[bayIndex]) != NULL)
            {
                if (SES_Page82[bayIndex] == NULL)
                {
                    /* Allocate the memory for a page82 read */
                    SES_Page82[bayIndex] = (PSES_P82_XTEX)s_MallocC(0x800, __FILE__, __LINE__);
                }

                Get_SES_Page(&gTemplateSESP82Rd, (void *)SES_Page82[bayIndex], 0x800, baypdd);

                if (SES_Page83[bayIndex] == NULL)
                {
                    /* Allocate the memory for a page83 read */
                    SES_Page83[bayIndex] = (PSES_P82_XTEX)s_MallocC(0x800, __FILE__, __LINE__);
                }

                Get_SES_Page(&gTemplateSESP83Rd, (void *)SES_Page83[bayIndex], 0x800, baypdd);
            }
        }
    }
}

/**
******************************************************************************
**
**  @brief      Sends page 82 or 83 to a drivebay to bypass a drive
**
**  @param      tp      - Template of scsi command.
**  @param      output  - Address of scsi page info.
**  @param      baypdd  - the PDD to use.
**  @param      channel - fibre channel loop id
**  @param      lid     - LID of drive, slot 0 or 15
**
**  @return     None
**
******************************************************************************
**/
void ses_SendBypass(UINT32 a UNUSED, UINT32 b UNUSED, PRP_TEMPLATE *tp, void *output, PDD *baypdd, UINT8 channel, UINT8 lid)
{
    SNS*   snsData = NULL;     /* Error data                       */
    UINT8  errCnt = 0;         /* Error counter                    */
    UINT32 retCode;

    do
    {
        if (snsData)
        {
            s_Free(snsData, sizeof(SNS), __FILE__, __LINE__);
            snsData = NULL;
            errCnt++;
            TaskSleepMS((unsigned)(errCnt*100));
        }
        retCode = ON_BypassCmd(tp, output, 0x800, baypdd, &snsData, channel, lid);
    } while ((retCode == SCR_CC) &&
             (errCnt < 5) &&
             (snsData->asc == ASC_SES_FAIL) &&
             ((snsData->ascq == ASCQ_SES_XFER_FAIL) ||
              (snsData->ascq == ASCQ_SES_XFER_REF)));

    if (snsData)
    {
        s_Free(snsData, sizeof(SNS), __FILE__, __LINE__);
    }
}

/**
******************************************************************************
**
**  @brief      ses_BypassDoubleCheck
**
**  @param      pdd     - PDD address.
**  @param      ses     - Drivebay of drive to bypass
**  @param      slot    - Slot of drive to bypass
**  @param      wwn     - WWN of drive to bypass
**
**  @return     None
**
******************************************************************************
**/
void ses_BypassDoubleCheck(UINT32 a UNUSED, UINT32 b UNUSED, struct PDD *pdd, UINT16 ses, UINT8 slot)
{
    PDD               *sesdrive;
    PSES_P82_XTEX     page82 = NULL;
    PSES_P82_XTEX     page83 = NULL;
    UINT16            drvindex;
    DEV               *dev;
    UINT8             port;

    /* Wait 2 seconds. */
    TaskSleepMS(HANG_WAIT);

    /* Check to see if the PDD of the bypass drive has not changed. */
    if ((pdd != NULL) &&
        (pdd->ses == ses) &&
        (pdd->slot == slot))
    {
        /*
         * Check for any other drive that has a high-level hangcount. If
         * there is, then do not bypass the drive.
         */
        for (drvindex = 0; drvindex < MAX_PHYSICAL_DISKS; drvindex++)
        {
            /* Check for a valid PDD and drives that are not the bypass drive. */
            if ((gPDX.pdd[drvindex] != NULL) && (gPDX.pdd[drvindex] != pdd))
            {
                /*
                 * If there is more than 1 drive with a high hangcnt, then
                 * don't bypass any drives.
                 */
                if (gPDX.pdd[drvindex]->hangCount >= HIGH_HANG_THRESHOLD)
                {
                    fprintf(stderr,"ses_BypassDblChk: No drive bypass due to other drive with high hangcounts, PDisk %c%02d, WWN %qx, HangCount = %d\n",
                                                                              ('A' + gPDX.pdd[drvindex]->ses),
                                                                              gPDX.pdd[drvindex]->slot,
                                                                              gPDX.pdd[drvindex]->wwn,
                                                                              gPDX.pdd[drvindex]->hangCount);
                    return;
                }
            }
        }

        /*
         * Send the 82 and 83 pages to both SES drives, slot 0 and slot 15,
         * on both channels. Start by copying the latest contents of page
         * 82 and 83 into memory. Set the contents of page 82 and 83 to
         * bypass the drive.
         */
        if (SES_Page82[pdd->ses] != NULL)
        {
            /* Allocate the memory for a page 82 read */
            page82 = (PSES_P82_XTEX)s_MallocC(0x800, __FILE__, __LINE__);

            /* Copy contents from the latest read of page 82 */
            memcpy(page82, SES_Page82[pdd->ses], 0x800);

            /* This is the port A page. Bypass the drive. */
            page82->Control[0].Action = 0x02;

            /*
             * Set the slot number and the length. The length is the
             * size of the base structure plus one extension minus the
             * four byte header for a standard SES page. Note that the
             * slot is off by 4 since the host ports are counted in the
             * set of ports.
             */
            page82->Control[0].Port = pdd->slot + 4;
            page82->Length = tm_const_ntohs(sizeof(SES_P82_XTEX) + sizeof(SES_P82XTEXCtl) - 4);
        }

        if (SES_Page83[pdd->ses] != NULL)
        {
            /* Allocate the memory for a page 83 read */
            page83 = (PSES_P82_XTEX)s_MallocC(0x800, __FILE__, __LINE__);

            /* Copy contents from the latest read of page 83 */
            memcpy(page83, SES_Page83[pdd->ses], 0x800);

            /* This is the port B page. Bypass the drive. */
            page83->Control[0].Action = 0x02;

            /*
             * Set the slot number and the length. The length is the
             * size of the base structure plus one extension minus the
             * four byte header for a standard SES page. Note that the
             * slot is off by 4 since the host ports are counted in the
             * set of ports.
             */
            page83->Control[0].Port = pdd->slot + 4;
            page83->Length = tm_const_ntohs(sizeof(SES_P82_XTEX) + sizeof(SES_P82XTEXCtl) - 4);
        }

        fprintf(stderr, "SESByp, bypassing ses = %d, slot = %d\n", pdd->ses, pdd->slot);

        /*
         * Find the drive in slot 0 and slot 15 of the SES that needs to
         * bypass the drive.
         */
        for (drvindex = 0; drvindex < MAX_PHYSICAL_DISKS; drvindex++)
        {
            if (((sesdrive = gPDX.pdd[drvindex]) != NULL) && (sesdrive->ses == pdd->ses) &&
                ((sesdrive->slot == SES_SLOT0) || (sesdrive->slot == SES_SLOT15)))
            {
                if ((dev = sesdrive->pDev) != NULL)
                {
                    for (port = 0; port < MAX_BE_PORTS; port++)
                    {
                        if (dev->pLid[port] != NO_CONNECT)
                        {
                            /*
                             * Fork off processes to send Send Diagnostic for pages 82 & 83
                             * on this channel for LID
                             */
                            if (page82 != NULL)
                            {
                                CT_fork_tmp = (ulong)"ses_SendBypass";
                                TaskCreate7(C_label_referenced_in_i960asm(ses_SendBypass),
                                            K_xpcb->pc_pri,
                                            (UINT32)&gTemplateSESP82Wr,
                                            (UINT32)page82,
                                            (UINT32)sesdrive,
                                            (UINT32)port,
                                            (UINT32)dev->pLid[port]);
                            }
                            if (page83 != NULL)
                            {
                                CT_fork_tmp = (ulong)"ses_SendBypass";
                                TaskCreate7(C_label_referenced_in_i960asm(ses_SendBypass),
                                            K_xpcb->pc_pri,
                                            (UINT32)&gTemplateSESP83Wr,
                                            (UINT32)page83,
                                            (UINT32)sesdrive,
                                            (UINT32)port,
                                            (UINT32)dev->pLid[port]);
                            }
                        }
                    }
                }
            }
        }
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
