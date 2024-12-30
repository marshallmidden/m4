/* $Id: nvram.c 162912 2014-03-20 22:47:47Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       nvram.c
**
**  @brief      NVRAM configuration
**
**  To provide a common means of handling the NVRAM configuration.
**
**  Copyright (c) 1996-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "nvram.h"

#include "XIO_Types.h"
#include "cev.h"
#include "chn.h"
#include "daml.h"
#include "def.h"
#include "defbe.h"
#include "def_con.h"
#include "DEF_iSCSI.h"
#include "def_lun.h"
#include "dev.h"
#include "dlmbe.h"
#include "fabric.h"
#include "ficb.h"
#include "fr.h"
#include "globalOptions.h"
#include "GR_Error.h"
#include "GR_LocationManager.h"
#include "ilt.h"
#include "kernel.h"
#include "ldd.h"
#include "lvm.h"
#include "MR_Defs.h"
#include "misc.h"
#include "nvr.h"
#include "online.h"
#include "options.h"
#include "p6.h"
#include "pcb.h"
#include "pr.h"
#include "RL_PSD.h"
#include "RL_RDD.h"
#include "rebuild.h"
#include "scrub.h"
#include "sdd.h"
#include "ses.h"
#include "system.h"
#include <sys/mman.h>
#include "target.h"
#include "virtual.h"
#include "vdd.h"
#include "vlar.h"

#include "LOG_Defs.h"
#include "OS_II.h"
#include "pdd.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "stdio.h"
#include "CT_defines.h"         /* Need the registers ... */
#include "CT_change_routines.h"

#include "mem_pool.h"           /* VLAR is in local memory. */
#include "ddr.h"
#include "misc.h"

/*
******************************************************************************
** Private variables
******************************************************************************
*/
UINT32      imageSeq = 0;
UINT32      CCSM_fresh_count = 0;
/* WHQL SCSI compliance enable (T/F)*/

UINT8       NV_scsi_whql = TRUE;

UINT32      locationPlaceHolder = 0;    /* Temp variable                   */

/*
 * NVRAM part 2's header with description (initialization).
 */

static NVRII nvram_p2_Init = {
    .cSum2 = 0x004f7012,
    .rsvd4 = {0, 0, 0, 0},
    .vers = 0,                  /* This should be VERS, but isn't checked. */
    .rev = REV,
    .length = sizeof(NVRII) + sizeof(NVRH),     /* NVRII+nvram_p2_nvrh */

    .magic = NR_MAGIC,
    .revision = NR_REVISION,
    .defLabel = 0x00,
    .seq = 0x00000000,
    .gPri = MAX_GLOBAL_PRIORITY,        /* See D_gpri. */
    .rsvd25 = 0x00,
    .ften = 0x00,
    .rsvd27 = 0x00,
    .vcgID = 0x00000000,

    .whql = TRUE,               /* Disable whql compliance. See NV_scsi_whql. */
    .rsvd33 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    .scrubOpt = FALSE,          /* Disable scrubbing. See gScrubOpt. */
    .glCache = FALSE,           /* Disable global cache. See D_glcache. */
    .glVdPriority = 0x40,       /* This has always been this value. */
    .glPdAutoFailback = 0x00,
    .name = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

/* The record following the NVRII structure. (An end of file.) */
static NVRH nvram_p2_nvrh = {
    .recLen = sizeof(NVRH),
    .recType = NRT_EOF,
    .status = 0,
};

/* The amount of NV RAM Part 2 for percentage display. */
static struct DMC_nvramp2percentused BE_nvramp2percentused;

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
UINT8 gOrphanLogged[MAX_RAIDS];

/* NOTE: There must always be enough NVRAM for the following. */
#define MAX_P2_DEVICES  ((MAX_PHYSICAL_DISKS + MAX_DISK_BAYS + MAX_MISC_DEVICES) * \
                         (sizeof(NVRH) + sizeof(NVRP)))
//                          (512 + 64 + 64) * (4 + 76) => 51200
#define MAX_P2_LLDS     (MAX_LDDS * (sizeof(NVRH) + MAX(sizeof(NVRX), sizeof(NVRF))))
//                        512 * (4 + max(76, 76)) => 40960
#define MAX_P2_MPR      (MAX_CTRL * (sizeof(NVRH) + sizeof(NVRM)))
//                       16 * 16 => 256
#define MAX_P2_P6_CFG   ((sizeof(NVRH) + sizeof(NVDMCR)) + MAX_CORS * (sizeof(NVRH) + sizeof(NVCOPY)))
//                        (4 + 16) + 512 * (4 + 96) => 51220

/* In addition, an MRP may add the max of the following. */
#define MAX_P2_V_CREATE (sizeof(NVRH) + sizeof(NVRV) + MAX_SEGMENTS * (sizeof(NVRVX1) + sizeof(NVRVX2)))
// VLINK create is one vdisk and 1 raid, and above is MAX_SEGMENTS times larger.
#define MAX_P2_SERVER   ((sizeof(NVRH) + sizeof(NVRS) + sizeof(NVRSX) + 256 + 15) & ~0xF)
#define MAX_P2_ISCSI_USER_GUESS 10
#define MAX_P2_TARGET   (sizeof(NVRH) + sizeof(NVRT) + sizeof(NVRX) + MAX_P2_ISCSI_USER_GUESS * sizeof(NVRTX1))
#define M_INT_MED_1     MAX(MAX_P2_V_CREATE, MAX_P2_SERVER)
#define MAX_P2_MRP_ADD  MAX(M_INT_MED_1, MAX_P2_TARGET)

/* Routine for allowing P2 NVRAM to expand, but reserve for essentials. */
UINT32 max_nvram_reserve(void);

/* External queue for running. */
extern COR *CM_cor_act_que;

/*
******************************************************************************
** Public functions - defined in other files
******************************************************************************
*/
extern UINT8 CCSM_fresh_flag;
extern GRS_PDD gGRSpdd[MAX_PHYSICAL_DISKS];
extern ISD  gISD;
extern void DEF_NewRsvNode(MRSETPRES_REQ *nvrec);
extern void DEF_ClearRsvAll(void);
extern UINT32 check_nvram_p2_available(void);

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
void        NV_P2ChkSum(NVRII *nvramImage);
void        NV_P2GenerateImage(NVRII *);
UINT32      NV_GetLocalImageSize(NVRL *);
UINT16      NV_ClearRemainingLDD(UINT16 currLID);
UINT16      NV_ClearRemainingSDD(UINT16 currSID);
UINT16      NV_ClearRemainingTGD(UINT16 currTID);
UINT16      NV_ClearRemainingRDD(UINT16 currRID);
UINT16      NV_ClearRemainingVDD(UINT16 currVID);
void        NV_RecordRemainingPID(PDX *pTable);

void        NV_P2WriteImage(NVRII *p2, UINT32 len);
UINT8       NV_P2VerifyNvram(NVRII *nvramImage);
void        DEF_update_spool_percent_used(UINT16 vid);

extern void D$SndFTOO(void);
/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Calculate and record the checksum for PART II of the NVRAM.
**
**              Each word within the PART II area (excluding the checksum word)
**              is summed up and stored. Interrupts are disabled during this
**              operation.
**
**  @param      nvramImage  - Address of the data for the NVRAM.
**
**  @return     none
**
******************************************************************************
**/
void NV_P2ChkSum(NVRII *nvramImage)
{
    UINT32      checksum;       /* Value for checksum                   */
    UINT32     *value;          /* Ptr to the image                     */
    UINT32      words;          /* Words to checksum                    */

    /*
     * Calculate the checksum starting at the field following the
     * checksum. This skips the checksum value itself.
     */
    for (checksum = NR_MAGIC,
         value = (UINT32 *)&nvramImage->rsvd4[0],
         words = (nvramImage->length - 4 + 3) / 4;
         words > 0; words--, checksum += *value, value++) ;

    nvramImage->cSum2 = checksum;

    /*
     * Also, calculate the amount of space available for define to add more
     * items to the configuration.
     */
    BE_nvramp2percentused.totalnvramp2 = NVRAM_P2_SIZE;
    BE_nvramp2percentused.nvram_unused = NVRAM_P2_SIZE - nvramImage->length;
    BE_nvramp2percentused.must_reserve_nvram = max_nvram_reserve();

    /* Update CCB memory copy of the information. */
    Copy_2_DMC(CCB_DMC_nvramp2percent, (void *)&BE_nvramp2percentused);
}   /* End 0f NV_P2ChkSum */

/**
******************************************************************************
**
**  @brief      Validate the checksum for PART II of the NVRAM.
**
**              Each word within the PART II area (excluding the checksum word)
**              is summed up and compared to the checksum word. The result of
**              this comparison is returned.
**
**  @param      nvramImage  - Address of the data for the NVRAM.
**
**  @return     TRUE if checksum is valid, FALSE otherwise.
**
******************************************************************************
**/
UINT8 NV_P2ChkSumChk(NVRII *nvramImage)
{
    UINT32      checksum;       /* Value for checksum                   */
    UINT32     *value;          /* Ptr to the image                     */
    UINT32      words;          /* Words to checksum                    */
    UINT32      magicMatch;     /* Did the magic number match           */

    BE_nvramp2percentused.totalnvramp2 = NVRAM_P2_SIZE;
    /*
     * First check for the magic, baby. If there is no magic number
     * match, then we assume that we have an uninitialized buffer and
     * auto-magically fail.
     */
    if (nvramImage->magic != NR_MAGIC)
    {
        magicMatch = FALSE;
        BE_nvramp2percentused.nvram_unused = NVRAM_P2_SIZE;
        checksum = 0;
    }
    else
    {
        magicMatch = TRUE;

        /*
         * Calculate the checksum starting at the field following the
         * checksum. This skips the checksum value itself.
         */
        for (checksum = NR_MAGIC,
             value = (UINT32 *)&nvramImage->rsvd4[0],
             words = (nvramImage->length - 4 + 3) / 4;
             words > 0; words--, checksum += *value, value++) ;

        /*
         * Also, caclculate the amount of space available for define to add more
         * items to the configuration. This is done in the check function in
         * addition to the generate function since on power on, there is only a
         * check which will then cause the seeding og the nvram available value.
         */
        BE_nvramp2percentused.nvram_unused = NVRAM_P2_SIZE - nvramImage->length;

        /* Update CCB memory copy of the information. */
        Copy_2_DMC(CCB_DMC_nvramp2percent, (void *)&BE_nvramp2percentused);
    }

    /*
     * Now return true or false based upon the checksum matching the
     * value in the image.
     */
    return (magicMatch && (checksum == nvramImage->cSum2) ? TRUE : FALSE);
}   /* End of NV_P2ChkSumChk */

/**
******************************************************************************
**
**  @brief      Fetch the VCG ID from part II of the NVRAM.
**
**              Just go in and grab the VCG ID and put it into the
**              FICB structure.
**
**  @param      nvramImage  - Address of the data for the NVRAM.
**
**  @return     none
**
******************************************************************************
**/
void NV_GetVCGID(NVRII *nvramImage)
{
    K_ficb->vcgID = nvramImage->vcgID;
}   /* End of NV_GetVCGID */

/**
******************************************************************************
**
**  @brief      Reorder the PDDs within the PDD list based upon temp NVRAM.
**
**              The temporary NVRAM image is checksum checked and then just
**              the PDD records are examined to pull the proper PDD and place
**              it into the PDD index table to attempt to get a best guess
**              ordered list of the PDDs
**
**  @param      nvramImage  - Address of the data for the NVRAM.
**              gPDX - global PDD list
**
**  @return     reordered PDD list
**
******************************************************************************
**/
void NV_ReorderPDDs(NVRII *nvramImage)
{
    NVR        *pNvRec;
    PDD        *pPDD;
    UINT16      i;

    /*
     * If the checksum is OK, then use the PDD records in the table to move
     * all of the PDDs to the best location. This is only a best guess since
     * the NVRAM image passed in may not be the latest. But, it is the best
     * we can do.
     */
    if (NV_P2ChkSumChk(nvramImage))
    {
        for (pNvRec = (NVR *)(nvramImage + 1);
             pNvRec->hdr.recType != NRT_EOF;
             pNvRec = (NVR *)((UINT32)pNvRec + pNvRec->hdr.recLen))
        {
            /*
             * Now check the type. If this is a physical drive, handle it,
             * otherwise skip to the next one.
             */
            if (pNvRec->hdr.recType == NRT_PHYS)
            {
                for (i = 0; i < MAX_PHYSICAL_DISKS; i++)
                {
                    /* Grab the PDD from the PDD table. */
                    pPDD = gPDX.pdd[i];

                    if ((pPDD != NULL) &&
                        (pPDD->wwn == pNvRec->u.phys.wwn) &&
                        (pPDD->lun == pNvRec->u.phys.lun))
                    {
                        /*
                         * Drive found. Swap it into the table in the
                         * right slot and change the PID. A simple swap
                         * of the two locations (where it was and where it
                         * goes) should guarantee that all the drives are
                         * placed in the right spot.
                         */
                        gPDX.pdd[i] = gPDX.pdd[pNvRec->u.phys.pid];

                        if (gPDX.pdd[i] != NULL)
                        {
                            gPDX.pdd[i]->pid = i;
                        }

                        gPDX.pdd[pNvRec->u.phys.pid] = pPDD;
                        pPDD->pid = pNvRec->u.phys.pid;

                        break;
                    }
                }
            }
        }
    }
}   /* End of NV_ReorderPDDs */

/*
******************************************************************************
**
**  @brief      Physically write the NVRAM image to the P2 NVRAM.
**
**  @param      p2  - Address of the data to be written to the NVRAM.
**  @param      len - Length of the data to write to the P2 NVRAM.
**
**  @return     none
**
******************************************************************************
*/
void NV_P2WriteImage(NVRII *p2, UINT32 len)
{
    /* Truncate length to max of NVRAM_P2_SIZE. */
    len = MIN(NVRAM_P2_SIZE, len);
    memcpy((void *)NVRAM_P2_START, (void *)p2, len);

    UINT32      rc;
    UINT8      *dst = (UINT8 *)NVRAM_P2_START;
    UINT32      pageAdjust = ((UINT32)dst % getpagesize());

    /* Adjust the input to align with a page boundary */
    rc = msync((UINT8 *)(dst - pageAdjust), (size_t)(len + pageAdjust), MS_SYNC);
    if (rc)
    {
        int         save_errno = errno;

        fprintf(stderr, "msync() addr = 0x%08x, length = 0x%08x\n", (UINT32)dst, len);
        fprintf(stderr, "msync() adjusted addr = 0x%08x, length = 0x%08x\n",
               (UINT32)dst - pageAdjust, len + pageAdjust);
        fprintf(stderr, "@@@@@@ msync() returned an error = %s @@@@\n", strerror(save_errno));
        fprintf(stderr, "pagesize = 0x%08x\n", getpagesize());
    }
}   /* End of NV_P2WriteImage */

/*
******************************************************************************
**
**  @brief      Verify that the NVRAM image in the P2 NVRAM area has a valid
**              checksum. Also check that the checksum is the same as that
**              of the image passsed in.
**
**  @param      nvramImage - Address of the NVRAM Image to verify the NVRAM
**              against.
**
**  @return     boolean indicating success/failure.
**
******************************************************************************
*/
UINT8 NV_P2VerifyNvram(NVRII *nvramImage)
{

    NVRII      *p2;             /* Pointer to the DRAM version of part 2 */
    UINT32      saveCSum;       /* saved checksum for write verification */
    UINT32      rc;

    saveCSum = nvramImage->cSum2;
    p2 = (NVRII *)NVRAM_P2_START;
    rc = NV_P2ChkSumChk(p2);

    /* Are the checksums matching? */
    if (rc != TRUE || p2->cSum2 != saveCSum)
    {
        /* Fail - report the error (critical?) */
        ON_LogError(LOG_NVRAM_WRITE_FAIL);
        D_p2writefail_flag = TRUE;
        return FALSE;
    }

    D_p2writefail_flag = FALSE;
    return TRUE;
}   /* End of NV_P2VerifyNvram */

/**
******************************************************************************
**
**  @brief      Update the PART II NVRAM structure with current
**              configuration information.
**
**              This function will create a copy of the NVRAM in memory.
**
**  @param      p2  - Address of the data for the NVRAM.
**
**  @return     none
**
******************************************************************************
**/
void NV_P2GenerateImage(NVRII *p2)
{
    NVR        *nvrec;          /* NVRAM record pointer (union of types)   */
    NVX        *nvxrec;         /* NVRAM record ext ptr (union of types)   */
    SDD        *sdd;            /* SDD pointer to move through lists       */
    VDD        *vdd;            /* VDD pointer to move through lists       */
    VLAR       *vlar;           /* VLAR pointer to move through VDD        */
    RDD        *rdd;            /* RDD pointer to move through lists       */
    PSD        *startpsd;       /* PSD pointers to move through lists      */
    PSD        *psd;            /* PSD pointers to move through lists      */
    PDD        *pdd;            /* PDD pointer to move through lists       */
    TGD        *tgd;            /* TGD pointer to move through lists       */
    LDD        *ldd;            /* LDD pointer to move through lists       */
    LVM        *lvm;            /* LVM pointer to move through lists       */
    UINT16      i;
    UINT16      k;
    UINT16      rectype;        /* Temp variables                          */
    UINT8      *dst;            /* Copy pointers                           */
    UINT32      copyIndx;       /* Copy index                              */
    LVM        *lvmPtr;         /* LVM pointer to move through lists       */
    LVM        *ilvmPtr;        /* LVM pointer to move through lists       */
    UINT16      vid;            /* Temp VID for RDD ownership check        */
    UINT8       owner;          /* Raid owner                              */

#if ISCSI_CODE
    CHAPINFO   *pUserInfo;
#endif
    MRSETPRES_REQ *prsv;

#ifdef DEBUG_FLIGHTREC_D
    MSC_FlightRec(FRT_H_MISCD, 0, 0, 0);
#endif

    /* Global priority is ALWAYS set to max since the UI no longer does that. */
    p2->vers = VERS;
    p2->rev = REV;
    p2->magic = NR_MAGIC;
    p2->revision = NR_REVISION;
    p2->defLabel = 0;
    p2->seq = K_ficb->seq;
    p2->gPri = D_gpri = MAX_GLOBAL_PRIORITY;
    p2->ften = D_ften;
    p2->vcgID = K_ficb->vcgID;
    p2->scrubOpt = gScrubOpt;

    /* Always make global cache be false on a 7000. */
#if defined(MODEL_7000) || defined(MODEL_4700)
    p2->glCache = FALSE;
    D_glcache = FALSE;
#else  /* MODEL_7000 || MODEL_4700 */
    p2->glCache = D_glcache;
#endif /* MODEL_7000 || MODEL_4700 */
    p2->whql = NV_scsi_whql;
    p2->glVdPriority = VPri_enable;
    p2->glPdAutoFailback = gAutoFailBackEnable;

    memcpy(p2->name, K_ficb->vcgName, 16);

    /*
     * For the NVRAM records to follow, the size of the main records have
     * been kept to a number of quads. Line up the sizes to match this if
     * possible to make NVRAM reading much easier. This should not be a
     * big issue since we have 1.4 of NVRAM.
     */

    /* Set the ownership values in the VDDs. */
    DL_SetVDiskOwnership();

    /*
     * Now that the header is filled in (short of the checksum and length),
     * fill in the fields for other devices. Start with the physical devices,
     * miscellaneous devices and the SES devices. They are all in the same
     * format so just do them all at once.
     */
    nvrec = (NVR *)(p2 + 1);

    for (i = 0; i < MAX_PHYSICAL_DISKS + MAX_DISK_BAYS + MAX_MISC_DEVICES; i++)
    {
        if (i < MAX_PHYSICAL_DISKS)
        {
            pdd = P_pddindx[i];
            if (pdd != 0 && pdd->pid != i)
            {
                fprintf(stderr, "%s%s:%u %s P_pddindx[%d]->pid = %d, ERROR, ignoring!\n",
                                FEBEMESSAGE, __FILE__, __LINE__, __func__, i, pdd->pid);
                continue;
            }
            rectype = NRT_PHYS;
        }
        else if (i < MAX_PHYSICAL_DISKS + MAX_DISK_BAYS)
        {
            pdd = E_pddindx[i - MAX_PHYSICAL_DISKS];
            if (pdd != 0 && pdd->pid != (i-MAX_PHYSICAL_DISKS))
            {
                fprintf(stderr, "%s%s:%u %s E_pddindx[%d]->pid = %d, ERROR, ignoring!\n",
                                FEBEMESSAGE, __FILE__, __LINE__, __func__, i-MAX_PHYSICAL_DISKS, pdd->pid);
                continue;
            }
            rectype = NRT_SES;
        }
        else
        {
            pdd = M_pddindx[i - MAX_PHYSICAL_DISKS - MAX_DISK_BAYS];
            if (pdd != 0 && pdd->pid != (i-MAX_PHYSICAL_DISKS - MAX_DISK_BAYS))
            {
                fprintf(stderr, "%s%s:%u %s M_pddindx[%d]->pid = %d, ERROR, ignoring!\n",
                                FEBEMESSAGE, __FILE__, __LINE__, __func__, i-MAX_PHYSICAL_DISKS - MAX_DISK_BAYS, pdd->pid);
                continue;
            }
            rectype = NRT_MISC;
        }

        if (pdd != 0)
        {
            nvrec->hdr.recLen = sizeof(NVRP) + sizeof(NVRH);
            nvrec->hdr.recType = rectype;
            nvrec->hdr.status = pdd->devStat;

            nvrec->u.phys.pid = pdd->pid;
            nvrec->u.phys.class = pdd->devClass;
            nvrec->u.phys.channel = pdd->channel;
            nvrec->u.phys.lun = pdd->lun;
            nvrec->u.phys.fcID = pdd->id;
            nvrec->u.phys.sSerial = pdd->ssn;
            nvrec->u.phys.wwn = pdd->wwn;
            nvrec->u.phys.geoLocation = pdd->geoLocationId;
            if (rectype == NRT_PHYS)
            {
                if (BIT_TEST(pdd->flags, PD_USER_FAILED))
                {
                    BIT_SET((nvrec->u.phys.flags), PD_USER_FAILED);
                }
            }

            memcpy(nvrec->u.phys.dName, pdd->devName, sizeof(pdd->devName));
            memcpy(nvrec->u.phys.hsDName, pdd->hsDevName, sizeof(pdd->hsDevName));

            nvrec->u.phys.miscStat = pdd->miscStat;

            *(u_qword *)&nvrec->u.phys.prodID = *(u_qword *)&pdd->prodID;
            *(u_tword *)&nvrec->u.phys.vendID = *(u_tword *)&pdd->vendID;
            *(u_tword *)&nvrec->u.phys.serial = *(u_tword *)&pdd->serial;

            nvrec = (NVR *)((UINT32)nvrec + sizeof(NVRH) + sizeof(NVRP));
        }
    }

    /* Now fill in the target records. */
    for (i = 0; i < MAX_TARGETS; i++)
    {
        tgd = T_tgdindx[i];

        if (tgd != 0)
        {
            nvrec->hdr.recLen = sizeof(NVRT) + sizeof(NVRH);
            nvrec->hdr.recType = NRT_TARGET;
            nvrec->hdr.status = 0;

            nvrec->u.targ.tid = i;
            nvrec->u.targ.port = tgd->port;
            nvrec->u.targ.opt = tgd->opt;
            nvrec->u.targ.fcid = tgd->fcid;
            nvrec->u.targ.lock = tgd->lock;
            nvrec->u.targ.owner = tgd->owner;
#if ISCSI_CODE
            nvrec->u.targ.ipPrefix = tgd->ipPrefix;
#endif
            nvrec->u.targ.portName = tgd->portName;
            nvrec->u.targ.nodeName = tgd->nodeName;
            nvrec->u.targ.prefOwner = tgd->prefOwner;
            nvrec->u.targ.cluster = tgd->cluster;
            nvrec->u.targ.prefPort = tgd->prefPort;
            nvrec->u.targ.altPort = tgd->altPort;
            nvrec->u.targ.i_mask = tgd->i_mask;

            /* Save the target extension record if the target type is iSCSI (fix for CQ #23451) */
            if (BIT_TEST(tgd->opt, TARGET_ISCSI) && (tgd->itgd != NULL))
            {
                /* Process the params structure. */
                nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRT) + sizeof(NVRH));

                nvxrec->u.targ.i_tgd.tid = tgd->itgd->tid;

                nvxrec->u.targ.i_tgd.ipAddr = tgd->itgd->ipAddr;
                nvxrec->u.targ.i_tgd.ipMask = tgd->itgd->ipMask;
                nvxrec->u.targ.i_tgd.ipGw = tgd->itgd->ipGw;
                nvxrec->u.targ.i_tgd.maxConnections = tgd->itgd->maxConnections;
                nvxrec->u.targ.i_tgd.initialR2T = tgd->itgd->initialR2T;
                nvxrec->u.targ.i_tgd.immediateData = tgd->itgd->immediateData;
                nvxrec->u.targ.i_tgd.dataSequenceInOrder = tgd->itgd->dataSequenceInOrder;
                nvxrec->u.targ.i_tgd.dataPDUInOrder = tgd->itgd->dataPDUInOrder;
                nvxrec->u.targ.i_tgd.ifMarker = tgd->itgd->ifMarker;
                nvxrec->u.targ.i_tgd.ofMarker = tgd->itgd->ofMarker;
                nvxrec->u.targ.i_tgd.errorRecoveryLevel = tgd->itgd->errorRecoveryLevel;
                nvxrec->u.targ.i_tgd.targetPortalGroupTag = tgd->itgd->targetPortalGroupTag;
                nvxrec->u.targ.i_tgd.maxBurstLength = tgd->itgd->maxBurstLength;
                nvxrec->u.targ.i_tgd.firstBurstLength = tgd->itgd->firstBurstLength;
                nvxrec->u.targ.i_tgd.defaultTime2Wait = tgd->itgd->defaultTime2Wait;
                nvxrec->u.targ.i_tgd.defaultTime2Retain = tgd->itgd->defaultTime2Retain;
                nvxrec->u.targ.i_tgd.maxOutstandingR2T = tgd->itgd->maxOutstandingR2T;
                nvxrec->u.targ.i_tgd.maxRecvDataSegmentLength = tgd->itgd->maxRecvDataSegmentLength;
                nvxrec->u.targ.i_tgd.maxSendDataSegmentLength = tgd->itgd->maxSendDataSegmentLength;
                nvxrec->u.targ.i_tgd.ifMarkInt = tgd->itgd->ifMarkInt;
                nvxrec->u.targ.i_tgd.ofMarkInt = tgd->itgd->ofMarkInt;
                nvxrec->u.targ.i_tgd.headerDigest = tgd->itgd->headerDigest;
                nvxrec->u.targ.i_tgd.dataDigest = tgd->itgd->dataDigest;
                nvxrec->u.targ.i_tgd.authMethod = tgd->itgd->authMethod;
                nvxrec->u.targ.i_tgd.mtuSize = tgd->itgd->mtuSize;

                memcpy(nvxrec->u.targ.i_tgd.tgtAlias, tgd->itgd->tgtAlias, sizeof(tgd->itgd->tgtAlias));

                nvxrec->u.targ.i_tgd.numUsers = tgd->itgd->numUsers;
                nvrec->hdr.recLen = sizeof(NVRT) + sizeof(NVRH) + sizeof(NVRTX) +
                    (tgd->itgd->numUsers) * sizeof(NVRTX1);
                nvrec = (NVR *)((UINT32)nvrec + sizeof(NVRH) + sizeof(NVRT) + sizeof(NVRTX));
                nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRTX));
                if (tgd->itgd->numUsers > 0)
                {
                    for (pUserInfo = tgd->itgd->chapInfo; pUserInfo != NULL; pUserInfo = pUserInfo->fthd)
                    {
                        memcpy(nvxrec->u.chapInfo.sname, pUserInfo->sname, 254);
                        memcpy(nvxrec->u.chapInfo.secret1, pUserInfo->secret1, 31);
                        memcpy(nvxrec->u.chapInfo.secret2, pUserInfo->secret2, 31);
                        nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRTX1));
                        nvrec = (NVR *)((UINT32)nvrec + sizeof(NVRTX1));
                    }
                }
            }
            else
            {
                nvrec = (NVR *)((UINT32)nvrec + sizeof(NVRH) + sizeof(NVRT));
            }
        }
    }

    /*
     * Now fill in the LDD records. The LDD table contains pointers to one
     * of two types of LDD records. The record is either a XIOtech LDD or
     * it is a foreign LDD.
     */
    for (i = 0; i < MAX_LDDS; i++)
    {
        ldd = DLM_lddindx[i];

        if (ldd != 0)
        {
            /* Check for XIOtech or foreign target. */
            if (ldd->class == LD_MLD)
            {
                nvrec->hdr.recLen = sizeof(NVRX) + sizeof(NVRH);
                nvrec->hdr.recType = NRT_XLDD;
                nvrec->hdr.status = 0;

                nvrec->u.lddx.lid = i;
                nvrec->u.lddx.pathMask = ldd->pathMask;
                nvrec->u.lddx.pathPri = ldd->pathPri;
                nvrec->u.lddx.devCap = ldd->devCap;
                nvrec->u.lddx.baseVDisk = ldd->baseVDisk;
                nvrec->u.lddx.baseCluster = ldd->baseCluster;
                nvrec->u.lddx.state = ldd->state;
                nvrec->u.lddx.baseNode = ldd->baseNode;
                memcpy(nvrec->u.lddx.baseName, ldd->baseName, 16);
                nvrec->u.lddx.baseSN = ldd->baseSN;
                nvrec->u.lddx.lun = ldd->lun;
                nvrec->u.lddx.owner = ldd->owner;
                *(u_tword *)&nvrec->u.lddx.serial = *(u_tword *)&ldd->serial;

                nvrec = (NVR *)((UINT32)nvrec + sizeof(NVRH) + sizeof(NVRX));
            }
            else
            {
                nvrec->hdr.recLen = sizeof(NVRF) + sizeof(NVRH);
                nvrec->hdr.recType = NRT_FLDD;
                nvrec->hdr.status = 0;

                nvrec->u.lddf.lid = i;
                nvrec->u.lddf.pathMask = ldd->pathMask;
                nvrec->u.lddf.pathPri = ldd->pathPri;
                nvrec->u.lddf.devCap = ldd->devCap;
                nvrec->u.lddf.lun = ldd->lun;
                nvrec->u.lddf.owner = ldd->owner;

                *(u_qword *)&nvrec->u.lddf.prodID = *(u_qword *)&ldd->prodID;
                *(u_tword *)&nvrec->u.lddf.vendID = *(u_tword *)&ldd->vendID;
                *(u_tword *)&nvrec->u.lddf.serial = *(u_tword *)&ldd->serial;

                nvrec = (NVR *)((UINT32)nvrec + sizeof(NVRH) + sizeof(NVRF));
            }
        }
    }

    /* Now fill in the RAID records. */
    for (i = 0; i < MAX_RAIDS; i++)
    {
        rdd = R_rddindx[i];

        if (rdd != 0)
        {
            UINT8 flagGT2TB = 0;        /* 0=32 bit version, 1=64 bit version. */

            /* Determine 32 or 64 bit versions of NVRAM to use. */
            startpsd = psd = *((PSD **)(rdd + 1));
            if ((psd->sLen & ~0xffffffffULL) != 0ULL)
            {
                flagGT2TB = 1;
            }
            else
            {
                do
                {
                    if ((psd->sda & ~0xffffffffULL) != 0ULL)
                    {
                        flagGT2TB = 1;
                        break;
                    }
                    psd = psd->npsd;
                }
                while (startpsd != psd);
            }
            if (flagGT2TB == 0)
            {
                nvrec->hdr.recLen = ((sizeof(NVRR) + sizeof(NVRH) +
                                      (rdd->psdCnt * sizeof(NVRRX))) + 15) & 0xFFF0;
                nvrec->hdr.recType = NRT_RAID;
                nvrec->hdr.status = rdd->status;

                nvrec->u.raid.rid = i;
                nvrec->u.raid.type = rdd->type;
                nvrec->u.raid.depth = rdd->depth;
                nvrec->u.raid.vid = rdd->vid;
                nvrec->u.raid.sps = rdd->sps;
                nvrec->u.raid.devCount = rdd->psdCnt;
                nvrec->u.raid.devCap = rdd->devCap;
                nvrec->u.raid.spu = rdd->spu;
                nvrec->u.raid.aStatus = rdd->aStatus;
                nvrec->u.raid.notMirrorCSN = rdd->notMirroringCSN;

                /*
                 * Assume we own the RAID. Check the mappings of non-master
                 * controllers to see who really owns it.
                 */
                nvrec->u.raid.owner = K_ficb->cSerial;
                vid = rdd->vid;
                vdd = V_vddindx[vid];
                if (vdd != NULL)
                {
                    owner = vdd->owner;
                    if (owner != 0xff)
                    {
                        nvrec->u.raid.owner &= 0xfffffff0;
                        nvrec->u.raid.owner |= owner;
                    }
                }

                /*
                 * The PSDs form a circular list. Grab the first one (it follows
                 * the RDD structure in memory) and run around the list.
                 */
                startpsd = psd = *((PSD **)(rdd + 1));
                nvrec->u.raid.sLen = psd->sLen;

                nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRR) + sizeof(NVRH));

                /* Fill in the PSD extensions to the RAID. */
                do
                {
                    nvxrec->u.raid.status = psd->status;
                    nvxrec->u.raid.aStatus = psd->aStatus & ~(1 << PSDA_ERROR);
                    nvxrec->u.raid.pid = psd->pid;
                    nvxrec->u.raid.sda = psd->sda;

                    nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRRX));
                    psd = psd->npsd;
                }
                while (startpsd != psd);
            }
            else
            {
                nvrec->hdr.recLen = ((sizeof(NVRR_GT2TB) + sizeof(NVRH) +
                                      (rdd->psdCnt * sizeof(NVRRX_GT2TB))) + 15) & 0xFFF0;
                nvrec->hdr.recType = NRT_RAID_GT2TB;
                nvrec->hdr.status = rdd->status;

                nvrec->u.raidGT2TB.rid = i;
                nvrec->u.raidGT2TB.type = rdd->type;
                nvrec->u.raidGT2TB.depth = rdd->depth;
                nvrec->u.raidGT2TB.vid = rdd->vid;
                nvrec->u.raidGT2TB.sps = rdd->sps;
                nvrec->u.raidGT2TB.devCount = rdd->psdCnt;
                nvrec->u.raidGT2TB.devCap = rdd->devCap;
                nvrec->u.raidGT2TB.spu = rdd->spu;
                nvrec->u.raidGT2TB.aStatus = rdd->aStatus;
                nvrec->u.raidGT2TB.notMirrorCSN = rdd->notMirroringCSN;

                /*
                 * Assume we own the RAID. Check the mappings of non-master
                 * controllers to see who really owns it.
                 */
                nvrec->u.raidGT2TB.owner = K_ficb->cSerial;
                vid = rdd->vid;
                vdd = V_vddindx[vid];
                if (vdd != NULL)
                {
                    owner = vdd->owner;
                    if (owner != 0xff)
                    {
                        nvrec->u.raidGT2TB.owner &= 0xfffffff0;
                        nvrec->u.raidGT2TB.owner |= owner;
                    }
                }

                /*
                 * The PSDs form a circular list. Grab the first one (it follows
                 * the RDD structure in memory) and run around the list.
                 */
                startpsd = psd = *((PSD **)(rdd + 1));
                nvrec->u.raidGT2TB.sLen = psd->sLen;

                nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRR_GT2TB) + sizeof(NVRH));

                /* Fill in the PSD extensions to the RAID. */
                do
                {
                    nvxrec->u.raidGT2TB.status = psd->status;
                    nvxrec->u.raidGT2TB.aStatus = psd->aStatus & ~(1 << PSDA_ERROR);
                    nvxrec->u.raidGT2TB.pid = psd->pid;
                    nvxrec->u.raidGT2TB.sda = psd->sda;

                    nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRRX_GT2TB));
                    psd = psd->npsd;
                }
                while (startpsd != psd);
            }
            nvrec = (NVR *)((UINT32)nvrec + nvrec->hdr.recLen);
        }
    }

    /*
     * Fill in the virtual devices. For virtual devices we need to save
     * the virtual device base record and three different extensions. The
     * first is the RAIDs which are attached. The second is the deferred
     * RAIDs (those undergoing initialization). And the third is VLOPs.
     * The first two use the same extension record type. The third uses
     * a different one.
     */
    for (i = 0; i < MAX_VIRTUAL_DISKS; i++)
    {
        vdd = V_vddindx[i];

        if (vdd != 0)
        {
            nvrec->hdr.recType = NRT_VIRT;
            /* Do not propagate the snapshot IO suspended, nor ISE busy states. */
            if (vdd->status != SS_IOSUSPEND)
            {
                nvrec->hdr.status = vdd->status;
            }
            else
            {
                /* Must set the state to something, pretend it is operational. */
                nvrec->hdr.status = VD_OP;
            }

            nvrec->u.virt.vid = i;
            nvrec->u.virt.attr = (vdd->attr & ~VD_BEBUSY);
            nvrec->u.virt.raidCnt = vdd->raidCnt;
            nvrec->u.virt.dRaidCnt = vdd->draidCnt;
            nvrec->u.virt.devCap = vdd->devCap;
            nvrec->u.virt.breakTime = vdd->breakTime;
            nvrec->u.virt.createTime = vdd->createTime;
            nvrec->u.virt.priority = vdd->priority;

            if (vdd->grInfo.vdOpState != GR_VD_IOSUSPEND)
            {
                nvrec->u.virt.grInfo.vdOpState = vdd->grInfo.vdOpState;
            }
            nvrec->u.virt.grInfo.permFlags = vdd->grInfo.permFlags;

            memcpy(nvrec->u.virt.name, vdd->name, 16);

            /*
             * The RDDs form a linked list. Grab the first one and follow
             * it to the end of the list. These are the regular RAIDs. Do
             * the same for the deferred RAIDs after the regular ones are done.
             */
            for (nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRV) + sizeof(NVRH)),
                 rdd = vdd->pRDD;
                 rdd != 0;
                 rdd = rdd->pNRDD, nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRVX1)))
            {
                nvxrec->u.virt1.rid = rdd->rid;
            }

            for (rdd = vdd->pDRDD;
                 rdd != 0;
                 rdd = rdd->pNRDD, nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRVX2)))
            {
                nvxrec->u.virt2.rid = rdd->rid;
            }

            /* Now do the vlar structures. */
            for (vlar = vdd->pVLinks, k = 0;
                 vlar != 0;
                 vlar = vlar->link, k++,
                 nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRVX3)))
            {
                nvxrec->u.virt3.srcSN = vlar->srcSN;
                nvxrec->u.virt3.srcCluster = vlar->srcCluster;
                nvxrec->u.virt3.srcVDisk = vlar->srcVDisk;
                nvxrec->u.virt3.attr = vlar->attr;
                nvxrec->u.virt3.poll = vlar->poll;
                nvxrec->u.virt3.repVID = vlar->repVDisk;
                nvxrec->u.virt3.agnt = vlar->agent;

//                for (j = 0; j < 52; j++)
//                {
//                    nvxrec->u.virt3.name[j] = vlar->name[j];
//                }
                memcpy(nvxrec->u.virt3.name, vlar->name, 52);
            }

            /*
             * Now fill in the number of VLAR's and the VDD extention length.
             * This could not be done until we knew the number of VLARs in
             * the vdd.
             */
            nvrec->u.virt.vlarCnt = k;
            nvrec->hdr.recLen = (((UINT32)nvxrec - (UINT32)nvrec) + 15) & 0xFFF0;
            nvrec = (NVR *)((UINT32)nvrec + nvrec->hdr.recLen);
        }
    }

    /* Do the servers. */
    for (i = 0; i < MAX_SERVERS; i++)
    {
        sdd = S_sddindx[i];

        if (sdd != 0)
        {
            nvrec->hdr.recLen = ((sizeof(NVRS) + sizeof(NVRH) +
                                  (sdd->numLuns * sizeof(NVRSX))) + 15) & 0xFFF0;
            nvrec->hdr.recType = NRT_SERVER;
            nvrec->hdr.status = 0;

            nvrec->u.serv.sid = i;
            nvrec->u.serv.nLuns = sdd->numLuns;
            nvrec->u.serv.tid = sdd->tid;
            nvrec->u.serv.stat = sdd->status;
            nvrec->u.serv.pri = sdd->pri;
            nvrec->u.serv.attrib = sdd->attrib;
            nvrec->u.serv.linkedSID = sdd->linkedSID;
            nvrec->u.serv.owner = sdd->owner;
            nvrec->u.serv.wwn = sdd->wwn;

            memcpy(nvrec->u.serv.name, sdd->name, 16);

            /*
             * Process the hash list for the LUNs. There are 16 hash entries
             * which are each a linked list of LVM records. Go through each
             * list until a NULL pointer is found.
             */
            nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRS) + sizeof(NVRH));

            /* Get the list of vdisks for this server. */
            lvmPtr = sdd->lvm;
            ilvmPtr = sdd->ilvm;

            while ((lvmPtr != NULL) || (ilvmPtr != NULL))
            {
                /*
                 * Get the list of all Vdisks mapped to this
                 * server and set the indicator in the idMapped
                 * for each Vdisk.
                 */

                /*
                 * Set the pointer for the one we will examine in
                 * this iteration of the loop. Also move the ptr
                 * onto the next one for the list we used.
                 */
                if (lvmPtr != NULL)
                {
                    lvm = lvmPtr;
                    lvmPtr = lvmPtr->nlvm;
                }
                else
                {
                    lvm = ilvmPtr;
                    ilvmPtr = ilvmPtr->nlvm;
                }

                nvxrec->u.serv.vid = lvm->vid;
                nvxrec->u.serv.lun = lvm->lun;
                nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRSX));
            }
            if (T_tgdindx[sdd->tid] && BIT_TEST(T_tgdindx[sdd->tid]->opt, TARGET_ISCSI))
            {
                memcpy(nvxrec->u.serv2.i_name, sdd->i_name, 254);
                nvrec->hdr.recLen += 256;
            }

            nvrec->hdr.recLen = (nvrec->hdr.recLen + 15) & 0xFFF0;

            nvrec = (NVR *)((UINT32)nvrec + nvrec->hdr.recLen);
        }
    }

    /* Now fill in the persistent reservation records. */
    for (i = 0; i < MAX_VIRTUAL_DISKS; i++)
    {
        prsv = gRsvData[i];
        if (prsv != 0)
        {
            if (V_vddindx[i] != 0)
            {
                nvrec->hdr.recLen = sizeof(NVRPR) + sizeof(NVRH) + prsv->regCount * sizeof(NVRPRX);
                nvrec->hdr.recType = NRT_PRES;
                nvrec->hdr.status = 0;
                memcpy(&(nvrec->u.prsv), prsv, sizeof(NVRPR) + prsv->regCount * sizeof(NVRPRX));
                nvrec->hdr.recLen = (nvrec->hdr.recLen + 15) & 0xFFF0;
                nvrec = (NVR *)((UINT32)nvrec + nvrec->hdr.recLen);
            }
            else
            {
                s_Free(prsv, (sizeof(MRSETPRES_REQ) + prsv->regCount * sizeof(MRREGKEY)),
                       __FILE__, __LINE__);
                gRsvData[i] = NULL;
            }
        }
    }

    /* Save the iSNS Servers configuration */
    nvrec->hdr.recType = NRT_ISNS;
    nvrec->hdr.recLen = (sizeof(NVRISNS) + sizeof(NVRH) + 15) & 0xFFF0;
    nvrec->hdr.status = 0;
    memcpy((void *)&nvrec->u.isns, (void *)&gISD, sizeof(ISD));
    BIT_CLEAR(nvrec->u.isns.gflags, ISNS_GF_MASTER);

    nvrec = (NVR *)((UINT32)nvrec + nvrec->hdr.recLen);

    /*
     * save the configurations of the current copys. This will save the configuration of
     * the copy even if their state cannot be maintained. This will cause the copies
     * to be restarted.
     */
    nvrec = P6_SaveCpyCfg(nvrec);

    /* Save the mirror partner records. */
    for (i = 0; i < D_mpcnt; i++)
    {
        nvrec->hdr.recLen = sizeof(NVRH) + sizeof(NVRM);
        nvrec->hdr.recType = NRT_MIRROR;
        nvrec->hdr.status = 0;

        nvrec->u.mirror.mySerial = D_mpmaps[i].source;
        nvrec->u.mirror.myPartner = D_mpmaps[i].dest;

        nvrec = (NVR *)((UINT32)nvrec + nvrec->hdr.recLen);
    }

    /*
     * Save the workset information. This is done in one block copy since
     * the proc code has no insight into the contents of this area.
     */
    nvrec->hdr.recType = NRT_WORKSET;
    nvrec->hdr.recLen = (sizeof(NVRW) + sizeof(NVRH) + 15) & 0xFFF0;
    nvrec->hdr.status = 0;

    memcpy((void *)(&(nvrec->u.workset)), (void *)&(gWorksetTable[0]),
           DEF_MAX_WORKSETS * sizeof(DEF_WORKSET));

    nvrec = (NVR *)((UINT32)nvrec + nvrec->hdr.recLen);

    /* Put the end of file record into the image. */
    nvrec->hdr.recLen = sizeof(NVRH);
    nvrec->hdr.recType = NRT_EOF;
    nvrec->hdr.status = 0;

    /* Place the length indicator into the NVRAM image. */
    p2->length = (UINT32)(nvrec) - (UINT32)p2 + sizeof(NVRH);
//    fprintf(stderr, "%s: Length of PII nvram image = %d\n", __func__, p2->length);

    /* Calculate the checksum value. */
    NV_P2ChkSum(p2);

    /*
     * Put a couple of quads of 0xff's into the image to make it readable.
     * Follow this with a few quads of zeros.
     */
    for (copyIndx = 0, dst = (UINT8 *)((UINT32)nvrec + sizeof(NVRH));
         copyIndx < 124 && copyIndx < BE_nvramp2percentused.nvram_unused;
         dst[copyIndx] = 0xFF, copyIndx++) ;

    for (;
         copyIndx < 256 && copyIndx < BE_nvramp2percentused.nvram_unused;
         dst[copyIndx] = 0x00, copyIndx++) ;
}   /* End of NV_P2GenerateImage */

/**
******************************************************************************
**
**  @brief      Calculate how much NVRAM part 2 must be reserved, "just in case".
**
**  @param      none
**
**  @return     NVRAM P2 bytes that must be reserved.
**
******************************************************************************
**/
UINT32 max_nvram_reserve(void)
{
    UINT32 bytes_must_reserve;
    UINT16 i;
    UINT32 bytes_devices = 0;       // MAX_P2_DEVICES
    UINT32 bytes_llds = 0;          // MAX_P2_LLDS
    UINT32 bytes_p6 = 0;            // MAX_P2_P6_CFG
    UINT32 bytes_mpr = 0;           // MAX_P2_MPR
    COR   *cor;                     /* COR pointer to move through lists. */

    /* Devices */
    for (i = 0; i < MAX_PHYSICAL_DISKS; i++)
    {
        if (P_pddindx[i] != 0)
        {
            bytes_devices += sizeof(NVRH) + sizeof(NVRP);
        }
    }
    for (i = 0; i < MAX_DISK_BAYS; i++)
    {
        if (E_pddindx[i] != 0)
        {
            bytes_devices += sizeof(NVRH) + sizeof(NVRP);
        }
    }
    for (i = 0; i < MAX_MISC_DEVICES; i++)
    {
        if (M_pddindx[i] != 0)
        {
            bytes_devices += sizeof(NVRH) + sizeof(NVRP);
        }
    }

    bytes_must_reserve = MAX_P2_DEVICES - bytes_devices;

    /* LDDs */
    for (i = 0; i < MAX_LDDS; i++)
    {
        if (DLM_lddindx[i] != 0)
        {
            /* Check for XIOtech or foreign target. */
            if (DLM_lddindx[i]->class == LD_MLD)
            {
                bytes_llds += sizeof(NVRH) + sizeof(NVRX);
            }
            else
            {
                bytes_llds += sizeof(NVRH) + sizeof(NVRF);
            }
        }
    }
    bytes_must_reserve += (MAX_P2_LLDS - bytes_llds);

    /* Part 6 */
    bytes_p6 = sizeof(NVRH) + sizeof(NVDMCR);
    for (cor = (COR*) CM_cor_act_que; cor != NULL; cor = cor->link)
    {
        bytes_p6  += sizeof(NVRH) + sizeof(NVCOPY);
    }
    bytes_must_reserve += (MAX_P2_P6_CFG - bytes_p6);


    /* Max Mirror Partner mappings. */
    for (i = 0; i < D_mpcnt; i++)
    {
        bytes_mpr += sizeof(NVRH) + sizeof(NVRM);
    }
    bytes_must_reserve += (MAX_P2_MPR - bytes_mpr);

    /* Add in the size that an MRP can change the nvram, to that which must be available. */
    bytes_must_reserve += MAX_P2_MRP_ADD;

    /* Add one disk block, might as well be safe! */
    bytes_must_reserve += 512;

    return(bytes_must_reserve);
}   /* End of max_nvram_reserve */

/**
******************************************************************************
**
**  @brief      Calculate how much NVRAM part 2 must be reserved, "just in case".
**
**  @param      none
**
**  @return     NVRAM P2 bytes that must be reserved.
**
******************************************************************************
**/
UINT32 check_nvram_p2_available(void)
{
    if (BE_nvramp2percentused.totalnvramp2 == 0)
    {
        /* Initialize first time. */
        BE_nvramp2percentused.totalnvramp2 = NVRAM_P2_SIZE;
        BE_nvramp2percentused.nvram_unused = NVRAM_P2_SIZE;
        BE_nvramp2percentused.must_reserve_nvram = NVRAM_P2_SIZE;

        /* Update CCB memory copy of the information. */
        Copy_2_DMC(CCB_DMC_nvramp2percent, (void *)&BE_nvramp2percentused);
        return FALSE;
    }

    if (BE_nvramp2percentused.nvram_unused < BE_nvramp2percentused.must_reserve_nvram)
    {
        return FALSE;
    }
    return TRUE;
}   /* End of check_nvram_p2_available */

/**
******************************************************************************
**
**  @brief      Update the PART II NVRAM structure with current
**              configuration information.
**
**              This function will create a copy of the NVRAM in memory.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/

static NVRII *p2_buffer = NULL;        /* Buffer for writing NVRAM P2. */
/* NOTE: This can task switch, so limit it to only one thing writing at a time. */
static int    p2_accessing = 0; /* Lock to prohibit multiple writes. */

void NV_P2UpdateNvram(void)
{
    UINT32      saveCSum;       /* saved checksum for write verification   */
    UINT32      p2Len;          /* Length of data to write out             */
    UINT16      i;              /* Temp variables                          */

    /* Get p2_buffer update lock. */
    while (p2_accessing != 0)
    {
        TaskSleepMS(10);
    }
    p2_accessing++;

    /* Allocate the DRAM to use for the temporary storage of the NVRAM image. */
    if (p2_buffer == NULL)
    {
        p2_buffer = s_MallocC(NVRAM_P2_SIZE, __FILE__, __LINE__);
    }
    else
    {
        memset(p2_buffer, 0, NVRAM_P2_SIZE);
    }

    /* Generate the NVRAM Image... */
    NV_P2GenerateImage(p2_buffer);

    saveCSum = p2_buffer->cSum2;       /* save the checksum for later verification */
    p2Len = p2_buffer->length;         /* save the length of the NVRAM data. */

    /* Now physically write the NVRAM image to the NVRAM. */
    NV_P2WriteImage(p2_buffer, MAX(p2Len + 256, NVRAM_P2_SIZE));

    /* Next, read back and verify that the NVRAM was updated. */

    /* Read NVRAM into the buffer */
    memcpy((void *)p2_buffer, (void *)NVRAM_P2_START, p2Len);

    /* Run a checksum verification on the newly read NVRAM */
    i = NV_P2ChkSumChk(p2_buffer);

    if (TRUE == i)
    {
        /* Compare the checkums to make sure they match */
        if (p2_buffer->cSum2 != saveCSum)
        {
            i = FALSE;
        }
    }

    /* Fail - report critical error */
    if (FALSE == i)
    {
        /* report the error */
        D_p2writefail_flag = TRUE;
        ON_LogError(LOG_NVRAM_WRITE_FAIL);
    }
    else
    {
        D_p2writefail_flag = FALSE;
    }

    /* The memory is never freed, we do this a lot on busy systems. */
    p2_accessing--;                     /* Clear the locking flag. */

    /*
     * Do a check for orphaned RAID devices. If the orphans were logged
     * already and one is still there, do not log. If they were logged and
     * none exist, clear the log indicator.
     */
    for (i = 0; i < MAX_RAIDS; ++i)
    {
        if (DEF_IsOrphanRAID(i))
        {
            if (!gOrphanLogged[i])
            {
                /* Log it. */
                gOrphanLogged[i] = TRUE;
                DEF_LogOrphanRAID(i);
            }
        }
        else
        {
            gOrphanLogged[i] = FALSE;
        }
    }
}   /* End of NV_P2UpdateNvram */

/**
******************************************************************************
**
**  @brief      Restore the previous system configuration stored in NVRAM.
**
**              The system configuration is regenerated from the information
**              stored in part II of NVRAM and device label information
**              read from available drives.
**
**              If the reload input is set, then the NVRAM is being used to
**              update the configuration already in the data structures.
**              This could be as simple as deleting a virtual disk or adding
**              a LUN mapping. In this case, the strucrtures are being checked
**              for consistency rather than being loaded into new structures.
**
**              Note that the drives have already been spun up and have been
**              recorded in a temporary PDX table.
**
**  @param      p2      - Image of the NVRAM
**  @param      pTable  - table of PDDs seen by the fibre off the back end
**  @param      initiaLoad - BOOLEAN, if TRUE, we are loading from power on
**  @param      restartCopies - BOOLEAN, if TRUE restart any copies in NVRAM
**  @param      caller -- Which module/function is calling
**                        (1 -- online module, 0 -- defbe module)
**
**  @return     none
**
******************************************************************************
**/
void NV_RestoreNvram(NVRII *p2, PDX *pTable, UINT8 initialLoad,
                     UINT8 restartCopies UNUSED, UINT32 caller)
{
    NVR        *nvrec;          /* NVRAM record pointer                     */
    NVX        *nvxrec;         /* NVRAM extension record pointer           */
    PDD        *pdd;            /* PDD pointer to move through lists        */
    LDD        *ldd;            /* LDD pointer to move through lists        */
    SDD        *sdd;            /* SDD pointer to move through lists        */
    TGD        *tgd;            /* TGD pointer to move through lists        */
    RDD        *rdd = NULL;     /* RDD pointer to move through lists        */
    PSD        *psd;            /* PSD pointer                              */
    PSD        *opsd;           /* PSD pointer                              */
    PSD        *fpsd = NULL;    /* PSD pointers                             */
    PSD       **ppsd;           /* Pointer to PSD pointers for RDD          */
    VDD        *vdd;            /* VDD pointer to move through lists        */
    VLAR       *vlar = NULL;    /* VLAR pointer to move through VDD         */
    VLAR       *pvlar = NULL;   /* Previous VLAR pointer to move through VDD */
    UINT32      i;              /* Temp variables                           */
    UINT32      j;
    UINT32      k;
    INT16       currLID = -1;   /* Current LDD index (deletion processing)  */
    UINT16      lid;            /* Current LID being loaded                 */
    INT16       currSID = 0;    /* Current server index-deletion processing */
    INT16       currTID = -1;   /* Current target index-deletion processing */
    INT16       currRID = -1;   /* Current RAID index (deletion processing) */
    UINT8       newXDD;         /* New or existing RAID or VDisk            */
    INT16       currVID = -1;   /* Current VDisk index (deletion processing) */
    UINT32      spool_expanded = FALSE; /* Snap pool expanded flag          */

#ifdef DEBUG_FLIGHTREC_D
    MSC_FlightRec(FRT_H_MISCE, (UINT32)p2, (UINT32)pTable, initialLoad);
#endif

    /*
     * Grab all of the header information and place it in the appropriate
     * global variables.
     * Global priority is ALWAYS set to max since the UI no longer does that.
     */
    K_ficb->seq = p2->seq;
    K_ficb->vcgID = p2->vcgID;
    K_ii.globalPri = D_gpri = p2->gPri = MAX_GLOBAL_PRIORITY;
    K_ii.scrub = gScrubOpt = p2->scrubOpt;
    if (D_ften != p2->ften)
    {
        D_ften = p2->ften;
        D$SndFTOO();
    }
#if defined(MODEL_7000) || defined(MODEL_4700)
    p2->glCache = FALSE;
    D_glcache = FALSE;
#else  /* MODEL_7000 || MODEL_4700 */
    D_glcache = p2->glCache;
#endif /* MODEL_7000 || MODEL_4700 */
    /*
     * If whql is set to DISABLE (user turned off SCSI 3), set NV_scsi_whql
     * to FALSE. If whql is FALSE (upgrade from a previous version) or whql
     * is TRUE, set NV_scsi_whql to TRUE.
     */
    if (p2->whql == DISABLE)
    {
        NV_scsi_whql = FALSE;
    }
    else
    {
        NV_scsi_whql = TRUE;
    }

    /* Always force priority on, if compiled with it on. */
    VPri_enable = 1;            /*p2->glVdPriority; */
    gAutoFailBackEnable = p2->glPdAutoFailback;
    memcpy(K_ficb->vcgName, p2->name, 16);

    /* Clear the mirror partner mappings. */
    for (i = 0, D_mpcnt = 0; i < MAX_CTRL; i++)
    {
        D_mpmaps[i].source = D_mpmaps[i].dest = 0;
    }

    /* Clear all PRR data in the BE */
    DEF_ClearRsvAll();

    /*
     * Process the physical device records. There exists a table of discovered
     * disks which is checked against the records. At the end of the check,
     * each physical device that matched with the records, will be in the
     * permanent table. All devices which were not found will have been
     * reported to the CCB. All devices which are new to the system will be
     * left in the temporary table of discovered devices and will then be
     * reported to the CCB.
     *
     * Keep count of the number of drives handled as they are processed.
     */
    for (nvrec = (NVR *)(p2 + 1), j = 0;
         nvrec->hdr.recType != NRT_EOF;
         nvrec = (NVR *)((UINT32)nvrec + nvrec->hdr.recLen))
    {
        /*
         * Now check the type. If this is a physical drive, handle it,
         * otherwise skip to the next one.
         */
        if ((nvrec->hdr.recType == NRT_PHYS) ||
            (nvrec->hdr.recType == NRT_SES) || (nvrec->hdr.recType == NRT_MISC))
        {
            if (pTable != NULL)
            {
                for (i = 0; i < MAX_PHYSICAL_DISKS + MAX_DISK_BAYS + MAX_MISC_DEVICES; i++)
                {
                    if (((pdd = pTable->pdd[i]) != 0) &&
                        (pTable->pdd[i]->wwn == nvrec->u.phys.wwn) &&
                        (pTable->pdd[i]->lun == nvrec->u.phys.lun))
                    {
                        /*
                         * Drive found. Enter it into the appropriate PDD table,
                         * NULL the pointer in the temp table, fill in the
                         * values from the NVRAM image and perform a number of
                         * checks on the device.
                         */
                        if (pdd->devType <= PD_DT_MAX_DISK)
                        {
                            if (nvrec->u.phys.pid >= MAX_PHYSICAL_DISKS)
                            {
                                fprintf(stderr, "%s%s:%u %s nvram PDISK pid(%d) >= %d, ERROR, ignoring!\n",
                                    FEBEMESSAGE, __FILE__, __LINE__, __func__, nvrec->u.phys.pid, MAX_PHYSICAL_DISKS);
                                goto ignoreentry;
                            }
                            P_pddindx[nvrec->u.phys.pid] = pdd;
                            *(UINT16 *)((UINT32 *)P_pddindx - 1) += 1;
                        }
                        else if ((pdd->devType <= PD_DT_MAX_SES) && (caller != 1))
                        {
                            if (nvrec->u.phys.pid > MAX_DISK_BAYS)
                            {
                                fprintf(stderr, "%s%s:%u %s nvram SES pid(%d) >= %d, ERROR, ignoring!\n",
                                    FEBEMESSAGE, __FILE__, __LINE__, __func__, nvrec->u.phys.pid, MAX_DISK_BAYS);
                                goto ignoreentry;
                            }
#if defined(MODEL_7000) || defined(MODEL_4700)
                            if (nvrec->u.phys.pid != pdd->ses &&
                                !(pdd->ses >= MAX_DISK_BAYS))
                            {
                                fprintf(stderr, "<ATS-54>nvrec pid=%d pdd-ses=%d devType=%d lun=%d\n",
                                        nvrec->u.phys.pid, pdd->ses, pdd->devType,
                                        nvrec->u.phys.lun);
                                nvrec->u.phys.pid = pdd->ses;
                            }
#endif /* MODEL_7000 || MODEL_4700 */
                            E_pddindx[nvrec->u.phys.pid] = pdd;
                            *(UINT16 *)((UINT32 *)E_pddindx - 1) += 1;
                        }
                        else
                        {
                            if (nvrec->u.phys.pid >= MAX_MISC_DEVICES)
                            {
                                fprintf(stderr, "%s%s:%u %s nvram MISC pid(%d) >= %d, ERROR, ignoring!\n",
                                    FEBEMESSAGE, __FILE__, __LINE__, __func__, nvrec->u.phys.pid, MAX_MISC_DEVICES);
                                goto ignoreentry;
                            }
                            M_pddindx[nvrec->u.phys.pid] = pdd;
                            *(UINT16 *)((UINT32 *)M_pddindx - 1) += 1;
                        }

                        /*
                         * Check for a change from an unlabelled drive to a
                         * labelled drive. If this is the case, then modify
                         * the device status to be the same as the status
                         * on the other controller.
                         */
                        if ((pdd->devClass != nvrec->u.phys.class) &&
                            (pdd->devClass == PD_UNLAB) &&
                            (pdd->devStat == PD_INOP) && (pdd->postStat == PD_FDIR))
                        {
                            pdd->devStat = nvrec->hdr.status;
                        }

                        if (BIT_TEST(nvrec->u.phys.flags, PD_USER_FAILED))
                        {
                            pdd->devStat = nvrec->hdr.status;
                            pdd->miscStat = nvrec->u.phys.miscStat;
                            BIT_SET(pdd->flags, PD_USER_FAILED);
                        }
                        else if (BIT_TEST(pdd->flags, PD_USER_FAILED))
                        {
                            pdd->devStat = nvrec->hdr.status;
                            pdd->miscStat = nvrec->u.phys.miscStat;
                            BIT_CLEAR(pdd->flags, PD_USER_FAILED);
                        }

                        /*
                         * Set the PID since the drive may have moved on a
                         * new controller.
                         */
                        pdd->pid = nvrec->u.phys.pid;
                        pdd->devClass = nvrec->u.phys.class;
                        pdd->ssn = nvrec->u.phys.sSerial;
                        pdd->geoLocationId = nvrec->u.phys.geoLocation;

                        /*
                         * Following is to aid when failover occurs for GEORAID:
                         * We are maintaining a local copy of WWN and its
                         * associated location code of all the physical
                         * disks. This is useful to log a message indicating
                         * that the drive is inserted either across GEO location
                         * or same location. This is one of items for GEORAID.
                         * If we don't maintain this information as a local copy
                         * we loose the PDD of the drive once it is removed and
                         * therefore loose its previous location code to know
                         * that the insertion happend within the bay(s) with the
                         * same location code or into a bay with a different
                         * location code from its previous location code.
                         */
                        gGRSpdd[locationPlaceHolder].wwn = nvrec->u.phys.wwn;
                        gGRSpdd[locationPlaceHolder++].locationCode = nvrec->u.phys.geoLocation;

                        memcpy(pdd->devName, nvrec->u.phys.dName, sizeof(pdd->devName));

                        memcpy(pdd->hsDevName, nvrec->u.phys.hsDName, sizeof(pdd->hsDevName));

                        /* Clear the table to show the drive has been processed. */
                        pTable->pdd[i] = 0;

                        /*
                         * If we are doing an initial load, update
                         * the settings in the PDD.
                         */
                        if (initialLoad)
                        {
                            pdd->miscStat = nvrec->u.phys.miscStat;

                            if ((pdd->devType <= PD_DT_MAX_DISK) &&
                                (pdd->postStat == PD_OP) &&
                                (pdd->devClass != PD_UNLAB) &&
                                (pdd->ssn != K_ficb->vcgID))
                            {
                                /* Log the drive as foreign. */
                                ON_LogSerialChanged(pdd);
                                pdd->failedLED = PD_LED_FAIL;
                                BIT_SET(pdd->miscStat, PD_MB_SERIALCHANGE);
                            }
                            else
                            {
                                /* Device is normal. Load up items from NVRAM. */
                                BIT_CLEAR(pdd->miscStat, PD_MB_SERIALCHANGE);
                            }

                            /*
                             * The device is here, so make sure any lingering misc
                             * status of missing is corrected. This catches a
                             * power on case where the drive was set to missing
                             * but now exists.
                             */
                            if (pdd->devStat != PD_NONX)
                            {
                                BIT_CLEAR(pdd->miscStat, PD_MB_MISSING);
                            }
                        }

                        /* The device is always inoperative if the replace bit is on. */
                        if (BIT_TEST(pdd->miscStat, PD_MB_REPLACE) && (pdd->devStat != PD_NONX))
                        {
                            pdd->devStat = PD_INOP;
                        }
                        /* Found the one we were looking for. Break out of the search. */
                        break;
                    }
                }               /* End looping i from 0 to MAX_PHYSICAL_DISKS */
            }                   /* End pTable == NULL */

            /*
             * Handle drive not found case.
             * If found the index would not be at the maximum.
             */
            if ((i == MAX_PHYSICAL_DISKS + MAX_DISK_BAYS + MAX_MISC_DEVICES) || (pTable == NULL))
            {
                /*
                 * Drive is missing from the drives seen on the physical
                 * interfaces. Log this and record the drive in the drive
                 * table since it is a part of a valid configuration and
                 * must be kept in place.
                 */
                if (nvrec->hdr.recType == NRT_PHYS)
                {
                    if (nvrec->u.phys.pid >= MAX_PHYSICAL_DISKS)
                    {
                        fprintf(stderr, "%s%s:%u %s nvram PDISK pid(%d) >= %d, ERROR, ignoring!\n",
                            FEBEMESSAGE, __FILE__, __LINE__, __func__, nvrec->u.phys.pid, MAX_DISK_BAYS);
                        goto ignoreentry;
                    }
                    pdd = DC_AllocPDD();
                    pdd->devType = PD_DT_UNKNOWN;
                    P_pddindx[nvrec->u.phys.pid] = pdd;
                    *(UINT16 *)((UINT32 *)P_pddindx - 1) += 1;
                }
                else if (nvrec->hdr.recType == NRT_SES)
                {
                    if (nvrec->u.phys.pid >= MAX_DISK_BAYS)
                    {
                        fprintf(stderr, "%s%s:%u %s nvram SES pid(%d) >= %d, ERROR, ignoring!\n",
                            FEBEMESSAGE, __FILE__, __LINE__, __func__, nvrec->u.phys.pid, MAX_DISK_BAYS);
                        goto ignoreentry;
                    }
                    pdd = DC_AllocPDD();
                    pdd->devType = PD_DT_BAY_UNKNOWN;
                    E_pddindx[nvrec->u.phys.pid] = pdd;
                    *(UINT16 *)((UINT32 *)E_pddindx - 1) += 1;
                }
                else
                {
                    if (nvrec->u.phys.pid >= MAX_MISC_DEVICES)
                    {
                        fprintf(stderr, "%s%s:%u %s nvram MISC pid(%d) >= %d, ERROR, ignoring!\n",
                            FEBEMESSAGE, __FILE__, __LINE__, __func__, nvrec->u.phys.pid, MAX_MISC_DEVICES);
                        goto ignoreentry;
                    }
                    pdd = DC_AllocPDD();
                    pdd->devType = PD_DT_UNKNOWN;
                    M_pddindx[nvrec->u.phys.pid] = pdd;
                    *(UINT16 *)((UINT32 *)M_pddindx - 1) += 1;
                }

                pdd->pid = nvrec->u.phys.pid;
                pdd->devClass = nvrec->u.phys.class;
                pdd->channel = nvrec->u.phys.channel;
                pdd->id = nvrec->u.phys.fcID;
                pdd->ssn = nvrec->u.phys.sSerial;
                *(u_qword *)&pdd->prodID = *(u_qword *)&nvrec->u.phys.prodID;
                *(u_tword *)&pdd->vendID = *(u_tword *)&nvrec->u.phys.vendID;
                *(u_tword *)&pdd->serial = *(u_tword *)&nvrec->u.phys.serial;
                pdd->wwn = nvrec->u.phys.wwn;
                pdd->lun = nvrec->u.phys.lun;
                pdd->geoLocationId = nvrec->u.phys.geoLocation;

                /*
                 * Following is to aid when failover occurs for GEORAID:
                 * We are maintaining a local copy of WWN and its
                 * associated location code of all the physical
                 * disks. This is useful to log a message indicating
                 * that the drive is inserted either across GEO location
                 * or same location. This is one of items for GEORAID.
                 * If we don't maintain this information as a local copy
                 * we loose the PDD of the drive once it is removed and
                 * therefore loose its previous location code to know
                 * that the insertion happend within the bay(s) with the
                 * same location code or into a bay with a different
                 * location code from its previous location code.
                 */
                gGRSpdd[locationPlaceHolder].wwn = nvrec->u.phys.wwn;
                gGRSpdd[locationPlaceHolder++].locationCode = nvrec->u.phys.geoLocation;
                pdd->devStat = PD_NONX;
                pdd->postStat = PD_NONX;
                pdd->ses = SES_NO_BAY_ID;
#if defined(MODEL_7000) || defined(MODEL_4700)
                fprintf(stderr, "NVRAM ISE DEBUG: Device(devstat postStat) moved to PDNONX\n");
#endif /* MODEL_7000 || MODEL_4700 */
                pdd->failedLED = PD_LED_FAIL;

                memcpy(pdd->devName, nvrec->u.phys.dName, 4);

                /*
                 * Fill in the device type based upon the product ID and the
                 * vendor ID information. Make sure the drive detune bit is
                 * masked off. Do not worry about dev structure since there
                 * isn't one for this drive.
                 */
                pdd->devType = SES_GetDeviceType(pdd);
                BIT_CLEAR(pdd->devType, PD_DT_KLUDGE_ECON);

                /* Set the error flags since we cannot see the device. */
                BIT_SET(pdd->miscStat, PD_MB_MISSING);

                /*
                 * Log missing drive. If the missing device is not a drive,
                 * do not log it.
                 */
                if (nvrec->hdr.recType == NRT_PHYS)
                {
                    ON_LogDriveMissing(pdd);
                }
            }

            /*
             * Check if this is the last record of this type. If so, process
             * the remaining PDDs in the PDD temporary list.
             */
            if ((((NVR *)((UINT32)nvrec + nvrec->hdr.recLen))->hdr.recType != NRT_PHYS) &&
                (((NVR *)((UINT32)nvrec + nvrec->hdr.recLen))->hdr.recType != NRT_SES) &&
                (((NVR *)((UINT32)nvrec + nvrec->hdr.recLen))->hdr.recType != NRT_MISC))
            {
                NV_RecordRemainingPID(pTable);
            }
    ignoreentry: ;
        }
        else if ((nvrec->hdr.recType == NRT_XLDD) || (nvrec->hdr.recType == NRT_FLDD))
        {
            /*
             * Process MAGNITUDE or foreign target link linked device records.
             *
             * Delete any missing device records.
             */
            if (nvrec->hdr.recType == NRT_XLDD)
            {
                lid = nvrec->u.lddx.lid;
            }
            else
            {
                lid = nvrec->u.lddf.lid;
            }

            for (currLID++; currLID < lid; currLID++)
            {
                if (DLM_lddindx[currLID] != NULL)
                {
                    /* Delete the LDD and propagate accordingly. */
                    DLM_ClrLDD(DLM_lddindx[currLID]);
                    DLM_PutLDD(DLM_lddindx[currLID]);
                    DLM_lddindx[currLID] = NULL;
                }
            }

            /*
             * If the LDD is NULL, allocate one, otherwise use
             * the one that is there.
             */
            if ((ldd = DLM_lddindx[lid]) == NULL)
            {
                DLM_lddindx[lid] = ldd = DLM_GetLDD();
            }

            /*
             * Now load it up. We do not have to worry about it being a
             * overlay or a restore in this case.
             */
            if (nvrec->hdr.recType == NRT_XLDD)
            {
                ldd->ord = lid;
                ldd->pathMask = nvrec->u.lddx.pathMask;
                ldd->pathPri = nvrec->u.lddx.pathPri;
                ldd->devCap = nvrec->u.lddx.devCap;
                ldd->baseVDisk = nvrec->u.lddx.baseVDisk;
                ldd->baseCluster = nvrec->u.lddx.baseCluster;
                ldd->baseSN = nvrec->u.lddx.baseSN;
                memcpy(ldd->baseName, nvrec->u.lddx.baseName, 16);
                ldd->lun = (nvrec->u.lddx.lun >= MAX_LUNS) ? 0 : nvrec->u.lddx.lun;
                ldd->owner = nvrec->u.lddx.owner;
                ldd->class = LD_MLD;
                ldd->state = nvrec->u.lddx.state;
                ldd->baseNode = nvrec->u.lddx.baseNode;
                *(u_tword *)&ldd->serial = *(u_tword *)&nvrec->u.lddx.serial;
            }
            else
            {
                ldd->ord = lid;
                ldd->pathMask = nvrec->u.lddf.pathMask;
                ldd->pathPri = nvrec->u.lddf.pathPri;
                ldd->devCap = nvrec->u.lddf.devCap;
                ldd->lun = (nvrec->u.lddf.lun >= MAX_LUNS) ? 0 : nvrec->u.lddf.lun;
                ldd->owner = nvrec->u.lddf.owner;
                ldd->rev = nvrec->u.lddf.rev;
                ldd->class = LD_FTD;
                ldd->state = LDD_ST_UNINIT;
                *(u_tword *)&ldd->serial = *(u_tword *)&nvrec->u.lddf.serial;
                *(u_tword *)&ldd->vendID = *(u_tword *)&nvrec->u.lddf.vendID;
                *(u_qword *)&ldd->prodID = *(u_qword *)&nvrec->u.lddf.prodID;
            }

            /*
             * Check if this is the last record of this type. If so, delete
             * all records past the index.
             */
            if ((((NVR *)((UINT32)nvrec + nvrec->hdr.recLen))->hdr.recType != NRT_XLDD) &&
                (((NVR *)((UINT32)nvrec + nvrec->hdr.recLen))->hdr.recType != NRT_FLDD))
            {
                currLID = NV_ClearRemainingLDD(currLID);
            }
        }
        else if (nvrec->hdr.recType == NRT_SERVER)
        {
            /*
             * Process the server records. For servers it is easier to just
             * blow them away and then recreate them. The structures on the
             * back end are only used to hold the information and are not in
             * active use during normal operations, so a clear and reuse is
             * acceptable and easier to do than a surgical strike.
             */
            for (; currSID <= nvrec->u.serv.sid; currSID++)
            {
                if (S_sddindx[currSID] != NULL)
                {
                    DEF_RelSDDLVM(S_sddindx[currSID]);
                    S_sddindx[currSID] = NULL;
                    *(UINT16 *)((UINT32 *)S_sddindx - 1) -= 1;

                    if (currSID != nvrec->u.serv.sid)
                    {
                        /*
                         * Only update to remove the server if it really is
                         * being removed. Otherwise, we will temporarily
                         * foul up the ownerships.
                         */
                        DEF_UpdRmtServer(currSID, TRUE);
                    }
                }
            }

            sdd = DEF_AllocServer();

            sdd->sid = nvrec->u.serv.sid;
            S_sddindx[sdd->sid] = sdd;
            *(UINT16 *)((UINT32 *)S_sddindx - 1) += 1;
            sdd->tid = nvrec->u.serv.tid;
            sdd->status = nvrec->u.serv.stat;
            sdd->pri = nvrec->u.serv.pri;
            sdd->attrib = nvrec->u.serv.attrib;
            sdd->linkedSID = nvrec->u.serv.linkedSID;
            sdd->owner = nvrec->u.serv.owner;
            sdd->wwn = nvrec->u.serv.wwn;

            memcpy(sdd->name, nvrec->u.serv.name, 16);

            for (nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRH) + sizeof(NVRS)),
                 i = nvrec->u.serv.nLuns;
                 i > 0; i--, nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRSX)))
            {
                DEF_HashLUN(sdd,
                            (UINT32)(nvxrec->u.serv.lun > MAX_LUNS ? 0 : nvxrec->u.serv.lun),
                            (UINT32)(nvxrec->u.serv.vid));
            }

            /* If iSCSI Server name exists, restore it */
            if (nvrec->hdr.recLen >
                ((sizeof(NVRS) + (nvrec->u.serv.nLuns * sizeof(NVRSX)) + 15) & 0xfff0))
            {
                nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRH) + sizeof(NVRS)
                                  + (nvrec->u.serv.nLuns * sizeof(NVRSX)));
                memcpy(sdd->i_name, nvxrec->u.serv2.i_name, 254);
            }

            DEF_UpdRmtServer(sdd->sid, FALSE);

            /*
             * Check if this is the last record of this type. If so, delete
             * all records past the index.
             */
            if (((NVR *)((UINT32)nvrec + nvrec->hdr.recLen))->hdr.recType != NRT_SERVER)
            {
                NV_ClearRemainingSDD(currSID);
            }
        }
        else if (nvrec->hdr.recType == NRT_TARGET)
        {
            /* Delete any missing device records. */
            for (currTID++; currTID < nvrec->u.targ.tid; currTID++)
            {
                if (T_tgdindx[currTID] != NULL)
                {
                    s_Free(T_tgdindx[currTID], sizeof(TGD), __FILE__, __LINE__);
                    DEF_UpdRmtTarg(currTID, TRUE);
                    T_tgdindx[currTID] = NULL;
                    *(UINT16 *)((UINT32 *)T_tgdindx - 1) -= 1;
                }
            }

            if ((tgd = T_tgdindx[nvrec->u.targ.tid]) == NULL)
            {
                T_tgdindx[nvrec->u.targ.tid] = tgd = DEF_AllocTarg();
                *(UINT16 *)((UINT32 *)T_tgdindx - 1) += 1;
            }

            tgd->tid = nvrec->u.targ.tid;
            tgd->port = nvrec->u.targ.port;
            tgd->opt = nvrec->u.targ.opt;
            tgd->fcid = nvrec->u.targ.fcid;
            tgd->lock = nvrec->u.targ.lock;
#if ISCSI_CODE
            tgd->ipPrefix = nvrec->u.targ.ipPrefix;
#endif
            tgd->portName = nvrec->u.targ.portName;
            tgd->nodeName = nvrec->u.targ.nodeName;
            tgd->owner = nvrec->u.targ.owner;
            tgd->prefOwner = nvrec->u.targ.prefOwner;
            tgd->cluster = nvrec->u.targ.cluster;
            tgd->prefPort = nvrec->u.targ.prefPort;
            tgd->altPort = nvrec->u.targ.altPort;
            tgd->i_mask = nvrec->u.targ.i_mask;

            /* Check the FE interfaces and accordingly update the TGD */
            if (initialLoad)
            {
                DEF_ValidateTarget(tgd, II_MASTER);
            }

            DEF_UpdRmtTarg(tgd->tid, FALSE);

            /*
             * Restore the Target Extension Records only if the target is of type iSCSI (fix for 23451).
             * If the param struct (I_TGD) exists for non-iSCSI target, then clean it up.
             */
            if (!BIT_TEST(tgd->opt, TARGET_ISCSI))
            {
                if (tgd->itgd != NULL)
                {
                    CHAPINFO   *pInfo;

                    /* Free existing userinfo */
                    while (tgd->itgd->chapInfo)
                    {
                        pInfo = tgd->itgd->chapInfo;
                        tgd->itgd->chapInfo = pInfo->fthd;
                        s_Free((void *)pInfo, sizeof(CHAPINFO), __FILE__, __LINE__);
                    }
                    s_Free((void *)tgd->itgd, sizeof(I_TGD), __FILE__, __LINE__);
                    tgd->itgd = NULL;
                }
            }
            else if (nvrec->hdr.recLen > sizeof(NVRH) + sizeof(NVRT))
            {
                nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRH) + sizeof(NVRT));
                DEF_CreateTargetInfo(tgd, &(nvxrec->u.targ.i_tgd));
                if (nvxrec->u.targ.i_tgd.numUsers > 0)
                {
                    k = nvxrec->u.targ.i_tgd.numUsers;
                    /* Restore CHAP User Info */
                    nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRTX));
                    for (j = 0; j < k; j++)
                    {
                        DEF_CreateChapUserInfo(tgd, (CHAPINFO *)(&(nvxrec->u.chapInfo)));
                        nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRTX1));
                    }
                }
                DEF_UpdRmtTgInfo(tgd);
                DEF_UpdChapInfo(tgd->itgd);
            }

            DEF_UpdRmtTarg(tgd->tid, FALSE);

            /*
             * Check if this is the last record of this type. If so, delete
             * all records past the index.
             */
            if (((NVR *)((UINT32)nvrec + nvrec->hdr.recLen))->hdr.recType != NRT_TARGET)
            {
                currTID = NV_ClearRemainingTGD(currTID);
            }
        }
        else if (nvrec->hdr.recType == NRT_RAID)
        {
            /* Delete any missing device records. */
            for (currRID++; currRID < nvrec->u.raid.rid; currRID++)
            {
                if (R_rddindx[currRID] != NULL)
                {
                    /*
                     * Delete the RAID. This function will terminate
                     * any RAID scans, defragmentations or rebuilds on
                     * this RAID. It will also handle the bookkeeping
                     * of the list (delete from RDD table).
                     */
                    DC_DeleteRAID(R_rddindx[currRID]);
                }
            }

            if ((rdd = R_rddindx[nvrec->u.raid.rid]) == NULL)
            {
                if (nvrec->u.raid.type == RD_SLINKDEV)
                {
                    nvrec->u.raid.devCount = 1;
                }

                R_rddindx[nvrec->u.raid.rid] = rdd = DC_AllocRDD(nvrec->u.raid.devCount);
                BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                *(UINT16 *)((UINT32 *)R_rddindx - 1) += 1;
                newXDD = TRUE;
            }
            else
            {
                newXDD = FALSE;
            }

            if (newXDD)
            {
                rdd->rid = nvrec->u.raid.rid;
                rdd->type = nvrec->u.raid.type;
                rdd->depth = nvrec->u.raid.depth;
                rdd->psdCnt = nvrec->u.raid.devCount;
                rdd->devCap = nvrec->u.raid.devCap;
                rdd->sps = nvrec->u.raid.sps;
                rdd->spu = nvrec->u.raid.spu;
                rdd->aStatus = nvrec->u.raid.aStatus;
                rdd->notMirroringCSN = nvrec->u.raid.notMirrorCSN;
            }

            rdd->vid = nvrec->u.raid.vid;
            if (rdd->type == RD_SLINKDEV)
            {
                rdd->psdCnt = 1;
                nvrec->u.raid.devCount = 1;
            }

            /*
             * Fill in the PSDs for this RAID. The PSDs are kept in the
             * RDD as an array of PSD pointers. The PSDs themselves are
             * also in a circularly linked list. Both linkages must be
             * set up here.
             */
            for (i = 0, opsd = NULL, ppsd = (PSD **)((UINT32)rdd + sizeof(RDD)),
                 nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRH) + sizeof(NVRR));
                 i < nvrec->u.raid.devCount;
                 i++, ppsd++, opsd = psd,
                 nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRRX)))
            {
                if (newXDD)
                {
                    *ppsd = psd = DC_AllocPSD();

                    BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                    psd->status = nvxrec->u.raid.status;
                    psd->aStatus = nvxrec->u.raid.aStatus;
                    psd->rid = nvrec->u.raid.rid;
                    psd->sLen = nvrec->u.raid.sLen;
                    psd->sda = nvxrec->u.raid.sda;
                    psd->pid = nvxrec->u.raid.pid;

                    /* Assign the space. */
                    pdd = P_pddindx[psd->pid];

                    if ((pdd != NULL) &&
                        (pdd->pDAML != NULL) &&
                        (!BIT_TEST(pdd->pDAML->flags, DAB_DIRTY)) &&
                        (rdd->type != RD_LINKDEV) && (rdd->type != RD_SLINKDEV))
                    {
                        /*
                         * If this is the initial load of NVRAM just
                         * dirty the PDD. Otherwise, set the DAM
                         * allocation and summarize all the allocations
                         * for the PDD.
                         */
                        if (initialLoad)
                        {
                            DA_DAMDirty(pdd->pid);
                        }
                        else
                        {
                            DA_DAMAsg(pdd->pDAML,
                                      psd->sda / DISK_SECTOR_ALLOC,
                                      psd->sLen / DISK_SECTOR_ALLOC, psd);
                            DA_DAMSum(pdd->pDAML);
                        }
                    }
                }
                else
                {
                    psd = *ppsd;

                    BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                    psd->rid = nvrec->u.raid.rid;
                    psd->sLen = nvrec->u.raid.sLen;

                    /*
                     * Check the SDA for a change. If it did change, then
                     * this is either a defrag starting or a hot spare has
                     * occurred. Also, check the PID. If it changed, then
                     * it is a hot spare. In either case, set up for a
                     * rebuild.
                     */
                    if ((psd->sda != nvxrec->u.raid.sda) ||
                        (psd->pid != nvxrec->u.raid.pid))
                    {
                        BIT_SET(psd->aStatus, PSDA_REBUILD);
                        BIT_CLEAR(psd->aStatus, PSDA_HOT_SPARE_REQD);

                        DA_DAMDirty(psd->pid);
                        DA_DAMDirty(nvxrec->u.raid.pid);

                        /*
                         * If the PID did not change, but the starting disk
                         * address moved, this is indicating a defragmentation
                         * is taking place. Set the defrag alternate status
                         * in this case.
                         */
                        if ((psd->sda != nvxrec->u.raid.sda) &&
                            (psd->pid == nvxrec->u.raid.pid))
                        {
                            BIT_SET(psd->aStatus, PSDA_DEFRAG);
                        }

                        psd->pid = nvxrec->u.raid.pid;
                    }

                    psd->sda = nvxrec->u.raid.sda;
                }

                /*
                 * Set the linkages up if we are not doing a overlay.
                 * We already have the PSD pointer in the RDD.
                 * Now set the next pointer of the previously
                 * allocated PSD to point to this PSD. If we are on the
                 * first one, save it for the circular linkage and do not
                 * set the next pointer yet. Otherwise, set the next pointer.
                 */
                if (newXDD)
                {
                    if (opsd != 0)
                    {
                        opsd->npsd = psd;
                    }
                    else
                    {
                        fpsd = psd;
                    }
                }
            }

            /* Complete the circular linkage. */
            if (newXDD)
            {
                opsd->npsd = fpsd;
            }

            /*
             * Check if this is the last record of this type. If so, delete
             * all records past the index.
             */
            if (((NVR *)((UINT32)nvrec + nvrec->hdr.recLen))->hdr.recType != NRT_RAID_GT2TB &&
                ((NVR *)((UINT32)nvrec + nvrec->hdr.recLen))->hdr.recType != NRT_RAID)
            {
                currRID = NV_ClearRemainingRDD(currRID);
            }
        }
        else if (nvrec->hdr.recType == NRT_RAID_GT2TB)
        {
            /* Delete any missing device records. */
            for (currRID++; currRID < nvrec->u.raidGT2TB.rid; currRID++)
            {
                if (R_rddindx[currRID] != NULL)
                {
                    /*
                     * Delete the RAID. This function will terminate
                     * any RAID scans, defragmentations or rebuilds on
                     * this RAID. It will also handle the bookkeeping
                     * of the list (delete from RDD table).
                     */
                    DC_DeleteRAID(R_rddindx[currRID]);
                }
            }

            if ((rdd = R_rddindx[nvrec->u.raidGT2TB.rid]) == NULL)
            {
                if (nvrec->u.raidGT2TB.type == RD_SLINKDEV)
                {
                    nvrec->u.raidGT2TB.devCount = 1;
                }

                R_rddindx[nvrec->u.raidGT2TB.rid] = rdd = DC_AllocRDD(nvrec->u.raidGT2TB.devCount);
                BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                *(UINT16 *)((UINT32 *)R_rddindx - 1) += 1;
                newXDD = TRUE;
            }
            else
            {
                newXDD = FALSE;
            }

            if (newXDD)
            {
                rdd->rid = nvrec->u.raidGT2TB.rid;
                rdd->type = nvrec->u.raidGT2TB.type;
                rdd->depth = nvrec->u.raidGT2TB.depth;
                rdd->psdCnt = nvrec->u.raidGT2TB.devCount;
                rdd->devCap = nvrec->u.raidGT2TB.devCap;
                rdd->sps = nvrec->u.raidGT2TB.sps;
                rdd->spu = nvrec->u.raidGT2TB.spu;
                rdd->aStatus = nvrec->u.raidGT2TB.aStatus;
                rdd->notMirroringCSN = nvrec->u.raidGT2TB.notMirrorCSN;
            }

            rdd->vid = nvrec->u.raidGT2TB.vid;
            if (rdd->type == RD_SLINKDEV)
            {
                rdd->psdCnt = 1;
                nvrec->u.raidGT2TB.devCount = 1;
            }

            /*
             * Fill in the PSDs for this RAID. The PSDs are kept in the
             * RDD as an array of PSD pointers. The PSDs themselves are
             * also in a circularly linked list. Both linkages must be
             * set up here.
             */
            for (i = 0, opsd = NULL, ppsd = (PSD **)((UINT32)rdd + sizeof(RDD)),
                 nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRH) + sizeof(NVRR_GT2TB));
                 i < nvrec->u.raidGT2TB.devCount;
                 i++, ppsd++, opsd = psd,
                 nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRRX_GT2TB)))
            {
                if (newXDD)
                {
                    *ppsd = psd = DC_AllocPSD();

                    BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                    psd->status = nvxrec->u.raidGT2TB.status;
                    psd->aStatus = nvxrec->u.raidGT2TB.aStatus;
                    psd->rid = nvrec->u.raidGT2TB.rid;
                    psd->sLen = nvrec->u.raidGT2TB.sLen;
                    psd->sda = nvxrec->u.raidGT2TB.sda;
                    psd->pid = nvxrec->u.raidGT2TB.pid;

                    /* Assign the space. */
                    pdd = P_pddindx[psd->pid];

                    if ((pdd != NULL) &&
                        (pdd->pDAML != NULL) &&
                        (!BIT_TEST(pdd->pDAML->flags, DAB_DIRTY)) &&
                        (rdd->type != RD_LINKDEV) && (rdd->type != RD_SLINKDEV))
                    {
                        /*
                         * If this is the initial load of NVRAM just
                         * dirty the PDD. Otherwise, set the DAM
                         * allocation and summarize all the allocations
                         * for the PDD.
                         */
                        if (initialLoad)
                        {
                            DA_DAMDirty(pdd->pid);
                        }
                        else
                        {
                            DA_DAMAsg(pdd->pDAML,           /* NOTE 22tb max physical disk. */
                                      psd->sda / DISK_SECTOR_ALLOC,
                                      psd->sLen / DISK_SECTOR_ALLOC, psd);
                            DA_DAMSum(pdd->pDAML);
                        }
                    }
                }
                else
                {
                    psd = *ppsd;

                    BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                    psd->rid = nvrec->u.raidGT2TB.rid;
                    psd->sLen = nvrec->u.raidGT2TB.sLen;

                    /*
                     * Check the SDA for a change. If it did change, then
                     * this is either a defrag starting or a hot spare has
                     * occurred. Also, check the PID. If it changed, then
                     * it is a hot spare. In either case, set up for a
                     * rebuild.
                     */
                    if ((psd->sda != nvxrec->u.raidGT2TB.sda) ||
                        (psd->pid != nvxrec->u.raidGT2TB.pid))
                    {
                        BIT_SET(psd->aStatus, PSDA_REBUILD);
                        BIT_CLEAR(psd->aStatus, PSDA_HOT_SPARE_REQD);

                        DA_DAMDirty(psd->pid);
                        DA_DAMDirty(nvxrec->u.raidGT2TB.pid);

                        /*
                         * If the PID did not change, but the starting disk
                         * address moved, this is indicating a defragmentation
                         * is taking place. Set the defrag alternate status
                         * in this case.
                         */
                        if ((psd->sda != nvxrec->u.raidGT2TB.sda) &&
                            (psd->pid == nvxrec->u.raidGT2TB.pid))
                        {
                            BIT_SET(psd->aStatus, PSDA_DEFRAG);
                        }

                        psd->pid = nvxrec->u.raidGT2TB.pid;
                    }

                    psd->sda = nvxrec->u.raidGT2TB.sda;
                }

                /*
                 * Set the linkages up if we are not doing a overlay.
                 * We already have the PSD pointer in the RDD.
                 * Now set the next pointer of the previously
                 * allocated PSD to point to this PSD. If we are on the
                 * first one, save it for the circular linkage and do not
                 * set the next pointer yet. Otherwise, set the next pointer.
                 */
                if (newXDD)
                {
                    if (opsd != 0)
                    {
                        opsd->npsd = psd;
                    }
                    else
                    {
                        fpsd = psd;
                    }
                }
            }

            /* Complete the circular linkage. */
            if (newXDD)
            {
                opsd->npsd = fpsd;
            }

            /*
             * Check if this is the last record of this type. If so, delete
             * all records past the index.
             */
            if (((NVR *)((UINT32)nvrec + nvrec->hdr.recLen))->hdr.recType != NRT_RAID_GT2TB &&
                ((NVR *)((UINT32)nvrec + nvrec->hdr.recLen))->hdr.recType != NRT_RAID)
            {
                currRID = NV_ClearRemainingRDD(currRID);
            }
        }
        else if (nvrec->hdr.recType == NRT_VIRT)
        {
            /* Delete any missing device records. */
            for (currVID++; currVID < nvrec->u.virt.vid; currVID++)
            {
                if (V_vddindx[currVID] != NULL)
                {
                    vlar = V_vddindx[currVID]->pVLinks;

                    while (vlar != NULL)
                    {
                        pvlar = vlar->link;
#ifdef M4_DEBUG_VLAR
                        fprintf(stderr, "put_vlar 0x%08x\n", (UINT32)vlar);
#endif /* M4_DEBUG_VLAR */
                        put_vlar(vlar);
                        vlar = pvlar;
                    }

                    P6_Process_VDD_Loss(V_vddindx[currVID]);

                    DEF_DeallocVDStatsMemory(V_vddindx[currVID]);

                    s_Free(V_vddindx[currVID], sizeof(VDD), __FILE__, __LINE__);
                    V_vddindx[currVID] = NULL;
                    *(UINT16 *)((UINT32 *)V_vddindx - 1) -= 1;
                    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                }
            }

            if ((vdd = V_vddindx[nvrec->u.virt.vid]) == NULL)
            {
                V_vddindx[nvrec->u.virt.vid] = vdd = DC_AllocVDD();
                *(UINT16 *)((UINT32 *)V_vddindx - 1) += 1;
                newXDD = TRUE;
                BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            }
            else
            {
                newXDD = FALSE;
                if (BIT_TEST(vdd->attr, VD_BSNAPPOOL) &&
                    (vdd->devCap != nvrec->u.virt.devCap))
                {
                    spool_expanded = TRUE;
                }
            }

            /*
             * Since the records were saved in an order that placed the
             * virtual devices at the end of the NVRAM image, we know that
             * all of the disk drives and RAIDs are already defined so we
             * can make a single pass through the virtual device records
             * and process them.
             */
            vdd->vid = nvrec->u.virt.vid;
            vdd->raidCnt = nvrec->u.virt.raidCnt;
            vdd->draidCnt = nvrec->u.virt.dRaidCnt;
            /* If we are not an inop-ed vdisk that is a snapshot, then ... */
            if (!((vdd->status == VD_INOP) && vdd->vd_incssms))
            {
                /* Do not change to or from SS_IOSUSPEND state */
                if ((nvrec->hdr.status != SS_IOSUSPEND) && (vdd->status != SS_IOSUSPEND))
                {
                    vdd->status = nvrec->hdr.status;
                }
            }
            vdd->devCap = nvrec->u.virt.devCap;

            if (spool_expanded)
            {
                DEF_update_spool_percent_used(vdd->vid);
                spool_expanded = FALSE;
            }

            /* If there are no SCDs or DCD, clear there associated attribute flags */
            vdd->attr = ~(VD_SCD | VD_DCD | VD_SUSPEND | VD_BEBUSY) & nvrec->u.virt.attr;

            if (vdd->pSCDHead != NULL)
            {
                vdd->attr |= (VD_SCD);
            }

            if (vdd->pDCD != NULL)
            {
                vdd->attr |= (VD_DCD);
            }

#if defined(MODEL_7000) || defined(MODEL_4700)
            if (!BIT_TEST(vdd->attr, VD_BVLINK))        /* If not a VLink */
            {
                BIT_SET(vdd->attr, VD_BCACHEEN);        /* Set WC on */
            }
#endif /* MODEL_7000 || MODEL_4700 */

            vdd->priority = nvrec->u.virt.priority;
            vdd->breakTime = nvrec->u.virt.breakTime;
            vdd->createTime = nvrec->u.virt.createTime;
            vdd->grInfo.vdOpState = nvrec->u.virt.grInfo.vdOpState;
            vdd->grInfo.permFlags = nvrec->u.virt.grInfo.permFlags;
#if GR_GEORAID15_DEBUG
            fprintf(stderr, "<GR><NvRestore>vid=%x permflags=%x devmiss flag=%x opstate=%x\n",
                    vdd->vid, vdd->grInfo.permFlags, vdd->grInfo.allDevMissSyncFlag,
                    vdd->grInfo.vdOpState);
#endif
            memcpy(vdd->name, nvrec->u.virt.name, 16);
            BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */

            /*
             * Insert the RAIDs into the VDD. There may be more
             * than one, but there is at least one.
             */
            for (i = 0, nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRH) + sizeof(NVRV));
                 i < nvrec->u.virt.raidCnt;
                 i++, nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRVX1)))
            {
                if (i == 0)
                {
                    rdd = vdd->pRDD = R_rddindx[nvxrec->u.virt1.rid];
                }
                else
                {
                    rdd = rdd->pNRDD = R_rddindx[nvxrec->u.virt1.rid];
                }

                BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                rdd->pNRDD = 0;

                /*
                 * Set up cache layer to mirror RAID 5 cache tags.
                 if (rdd->type == rdraid5)
                 {
                     vdd->cacheen |= (1<<vc_write_tags);
                 }
                 */

                /* If the RAID was initializing, restart it. */
                if (initialLoad && (BIT_TEST(rdd->aStatus, RD_A_UNINIT)) &&
                    !D_moveinprogress)
                {
                    DEF_QueRInit(rdd);
                }
            }

            /*
             * Insert the deferred RAIDs into the VDD. There may be zero
             * or more of these. If there are none, then make sure that
             * the pointer is set to NULL.
             */
            for (i = 0, vdd->pDRDD = NULL;
                 i < nvrec->u.virt.dRaidCnt;
                 i++, nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRVX2)))
            {
                if (i == 0)
                {
                    rdd = vdd->pDRDD = R_rddindx[nvxrec->u.virt2.rid];
                }
                else
                {
                    rdd = rdd->pNRDD = R_rddindx[nvxrec->u.virt2.rid];
                }

                BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                rdd->pNRDD = 0;

                /*
                 * Set up cache layer to mirror RAID 5 cache tags.
                 if (rdd->type == rdraid5)
                 {
                     vdd->cacheen |= (1<<vc_write_tags);
                 }
                 */

                /* If the RAID was initializing, restart it. */
                if (initialLoad && (BIT_TEST(rdd->aStatus, RD_A_UNINIT)) &&
                    !D_moveinprogress)
                {
                    DEF_QueRInit(rdd);
                }
            }

            /* Now set up the VLARs. */
            for (i = 0;
                 i < nvrec->u.virt.vlarCnt;
                 i++, nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRVX3)))
            {
                /*
                 * Determine if there is a VLAR present. If so, use that VLAR.
                 * Otherwise allocate a VLAR.
                 */
                if (i == 0)
                {
                    vlar = vdd->pVLinks;
                    if (vlar == NULL)
                    {
                        vlar = vdd->pVLinks = get_vlar();
#ifdef M4_DEBUG_VLAR
                        fprintf(stderr, "get_vlar 0x%08x\n", (UINT32)vlar);
#endif /* M4_DEBUG_VLAR */
                        vlar->link = NULL;
                    }
                }
                else
                {
                    vlar = vlar->link;
                    if (vlar == NULL)
                    {
                        vlar = vlar->link = get_vlar();
#ifdef M4_DEBUG_VLAR
                        fprintf(stderr, "get_vlar 0x%08x\n", (UINT32)vlar);
#endif /* M4_DEBUG_VLAR */
                        vlar->link = NULL;
                    }
                }
                /* Fill in the contents of the VLAR. */
                vlar->srcSN = nvxrec->u.virt3.srcSN;
                vlar->srcCluster = nvxrec->u.virt3.srcCluster;
                vlar->srcVDisk = nvxrec->u.virt3.srcVDisk;
                vlar->attr = nvxrec->u.virt3.attr;
                vlar->poll = nvxrec->u.virt3.poll;
                vlar->repVDisk = nvxrec->u.virt3.repVID;
                vlar->agent = nvxrec->u.virt3.agnt;

                for (j = 0; j < 52; j++)
                {
                    vlar->name[j] = nvxrec->u.virt3.name[j];
                }
            }

            /*
             * Check if this is the last record of this type. If so, delete
             * all records past the index.
             */
            if (((NVR *)((UINT32)nvrec + nvrec->hdr.recLen))->hdr.recType != NRT_VIRT)
            {
#ifdef USE_TO_FIX_ORPHANS
                int         index_n;
#endif
                currVID = NV_ClearRemainingVDD(currVID);
                /* And also delete any unnassigned raid ids (DO NOT CHECK INTO MAINLINE. YET) */
#ifdef USE_TO_FIX_ORPHANS
                for (index_n = 0; index_n < MAX_RAIDS; index_n++)
                {
                    if (((rdd = R_rddindx[index_n]) != 0))
                    {
                        if (V_vddindx[rdd->vid] == NULL)
                        {
                            /*
                             * Delete the RAID. This function will terminate
                             * any RAID scans, defragmentations or rebuilds on
                             * this RAID. It will also handle the bookkeeping
                             * of the list (delete from RDD table).
                             */
                            DC_DeleteRAID(R_rddindx[index_n]);
                        }
                    }
                }
#endif
            }
        }
        else if (nvrec->hdr.recType == NRT_MIRROR)
        {
            /*
             * If this is the initial load of NVRAM and it is our mirror
             * partner record, get the mirror partner into the FICB.
             */
            if (initialLoad && nvrec->u.mirror.mySerial == K_ficb->cSerial)
            {
                K_ficb->mirrorPartner = nvrec->u.mirror.myPartner;
            }

            /* Load up the mirror partner information into the tables. */
            D_mpmaps[D_mpcnt].source = nvrec->u.mirror.mySerial;
            D_mpmaps[D_mpcnt++].dest = nvrec->u.mirror.myPartner;
        }
        else if ((nvrec->hdr.recType == NRT_DMCR) || (nvrec->hdr.recType == NRT_COPYCFG))
        {
            /*
             * Restore the configurations of the current copys. The configuration is restore in a
             * manner that will cause the copies to be restarted.
             * FINISH The test for content on the COR active queue allows config restore to
             *        act simulay to config refresh until configuration restore for a booting
             *        controller can be tested
             */
            /* if (CM_cor_act_que != NULL)
             * { */

            p6NvrecBase = NVRAM_P2_START + ((UINT32)nvrec - (UINT32)p2);
#if GR_GEORAID15_DEBUG
            fprintf(stderr, "<GR><NV_RestoreNvram- nvram.c>Calling P6_RstCpyCfg\n");
#endif

            nvrec = P6_RstCpyCfg(nvrec);
            /*} */
        }
        else if (nvrec->hdr.recType == NRT_WORKSET)
        {
            /*
             * Restore the workset information. This is done in one block
             * copy since the proc code has no insight into the contents of
             * this area.
             */
            memcpy((void *)&(gWorksetTable[0]), (void *)(&(nvrec->u.workset)),
                   DEF_MAX_WORKSETS * sizeof(DEF_WORKSET));
        }
        else if (nvrec->hdr.recType == NRT_ISNS)
        {
            /* Restore the iSNS Servers configuration */
            memcpy((void *)&gISD, (void *)&nvrec->u.isns, sizeof(ISD));
            DEF_iSNSUpdateFE();
        }
        else if (nvrec->hdr.recType == NRT_PRES)
        {
            /* Restore Persistent Reservation records */
            DEF_NewRsvNode((MRSETPRES_REQ *)&(nvrec->u.prsv));
        }
        else
        {
            fprintf(stderr, "%s%s:%u %s unknown nvram recType %d, ERROR, ignoring!\n",
                FEBEMESSAGE, __FILE__, __LINE__, __func__, nvrec->hdr.recType);
        }
    }

    /*
     * Now that all of the servers, targets, vdisks, RAIDs, etc have
     * been processed, go through the remaining portion of the tables
     * and delete any devices left. This is done to catch all of the
     * devices with IDs above the one last entered from NVRAM.
     */
    NV_ClearRemainingLDD(currLID);
    NV_ClearRemainingSDD(currSID);
    NV_ClearRemainingTGD(currTID);
    NV_ClearRemainingRDD(currRID);
    NV_ClearRemainingVDD(currVID);
    NV_RecordRemainingPID(pTable);
    locationPlaceHolder = 0;

    /*
     * Update the PSD, RAID, and Virtual status for all devices.
     * Attempt to hotspare any failed devices. Do this only if
     * the full define is running.
     */
    DL_SetVDiskOwnership();
    RB_setpsdstat();
    RB_SetVirtStat();
    RB_UpdateRebuildWriteState(false, false);
    RB_SearchForFailedPSDs();
    DEF_UMiscStat();

    DEF_SndConfigOpt();

    TaskReadyByState(PCB_FILE_SYS_CLEANUP);

    /*
     * Update all PDDs. Do not force the updates since the PDDs will
     * be marked in the restoration if an update was required.
     */
    for (i = 0; i < MAX_PHYSICAL_DISKS; i++)
    {
        if (P_pddindx[i] != 0)
        {
            DA_CalcSpace(i, FALSE);
        }
    }

    i = MIN(NVRAM_P2_SIZE, p2->length + 256);
    memcpy((void *)NVRAM_P2_START, (void *)p2, i);

    {
        UINT32      rc;
        UINT8      *dst = (UINT8 *)NVRAM_P2_START;
        UINT32      pageAdjust = ((UINT32)dst % getpagesize());

        /* Adjust the input to align with a page boundary */
        rc = msync((UINT8 *)(dst - pageAdjust), (size_t)(i + pageAdjust), MS_SYNC);
        if (rc)
        {
            int         save_errno = errno;

            fprintf(stderr, "msync() addr = 0x%08x, length = 0x%08x\n", (UINT32)dst, i);
            fprintf(stderr, "msync() adjusted addr = 0x%08x, length = 0x%08x\n",
                   (UINT32)dst - pageAdjust, i + pageAdjust);
            fprintf(stderr, "@@@@@@ msync() returned an error = %s @@@@\n", strerror(save_errno));
            fprintf(stderr, "pagesize = 0x%08x\n", getpagesize());
        }
    }
    /* Log NVRAM restored message. */
    if (initialLoad)
    {
        ON_LogError(LOG_NVRAM_RESTORE);
    }
//    else
//    {
//        ON_LogError(LOG_NVRAM_RELOAD);
//    }

    /* Next, verify that the data written out to NVRAM is good. */
    NV_P2VerifyNvram(p2);
}   /* End of NV_RestoreNvram */

/**
******************************************************************************
**
**  @brief      Clear remaining LDD.
**
**  @param      currLID - the last LID updated
**
**  @return     new current LID
**
******************************************************************************
**/
UINT16 NV_ClearRemainingLDD(UINT16 currLID)
{
    for (currLID++; currLID < MAX_LDDS; currLID++)
    {
        if (DLM_lddindx[currLID] != NULL)
        {
            /* Delete the LDD and propagate accordingly. */
            DLM_ClrLDD(DLM_lddindx[currLID]);
            DLM_PutLDD(DLM_lddindx[currLID]);
            DLM_lddindx[currLID] = NULL;
        }
    }
    return (currLID);
}   /* End of NV_ClearRemainingLDD */

/**
******************************************************************************
**
**  @brief      Clear remaining SDDs.
**
**  @param      currSID - the last SID updated
**
**  @return     the mast SID updated
**
******************************************************************************
**/
UINT16 NV_ClearRemainingSDD(UINT16 currSID)
{
    for (; currSID < MAX_SERVERS; currSID++)
    {
        if (gSDX.sdd[currSID] != NULL)
        {
            DEF_RelSDDLVM(gSDX.sdd[currSID]);
            gSDX.sdd[currSID] = NULL;
            --gSDX.count;
            DEF_UpdRmtServer(currSID, TRUE);
        }
    }
    return (currSID);
}   /* End of NV_ClearRemainingSDD */

/**
******************************************************************************
**
**  @brief      Clear remaining TGDs.
**
**  @param      currTID - the last TGD updated
**
**  @return     the last TID updated
**
******************************************************************************
**/
UINT16 NV_ClearRemainingTGD(UINT16 currTID)
{
    for (currTID++; currTID < MAX_TARGETS; currTID++)
    {
        if (gTDX.tgd[currTID] != NULL)
        {
            s_Free(gTDX.tgd[currTID], sizeof(TGD), __FILE__, __LINE__);
            gTDX.tgd[currTID] = NULL;
            --gTDX.count;
            DEF_UpdRmtTarg(currTID, TRUE);
        }
    }
    return (currTID);
}   /* End of NV_ClearRemainingTGD */

/**
******************************************************************************
**
**  @brief      Clear remaining RDDs.
**
**  @param      currRID - the last RDD updated
**
**  @return     the last RID updated
**
******************************************************************************
**/
UINT16 NV_ClearRemainingRDD(UINT16 currRID)
{
    for (currRID++; currRID < MAX_RAIDS; currRID++)
    {
        if (gRDX.rdd[currRID] != NULL)
        {
            /*
             * Delete the RAID. This function will terminate
             * any RAID scans, defragmentations or rebuilds on
             * this RAID. It will also handle the bookkeeping
             * of the list (delete from RDD table).
             */
            DC_DeleteRAID(gRDX.rdd[currRID]);
        }
    }
    return (currRID);
}   /* End of NV_ClearRemainingRDD */

/**
******************************************************************************
**
**  @brief      Clear remaining VDDs.
**
**  @param      currVID - the last VDD updated
**
**  @return     the last VID updated
**
******************************************************************************
**/
UINT16 NV_ClearRemainingVDD(UINT16 currVID)
{
    VLAR       *vlar = NULL;
    VLAR       *pvlar = NULL;

    for (currVID++; currVID < MAX_VIRTUAL_DISKS; currVID++)
    {
        if (gVDX.vdd[currVID] != NULL)
        {
            vlar = gVDX.vdd[currVID]->pVLinks;

            while (vlar != NULL)
            {
                pvlar = vlar->link;
#ifdef M4_DEBUG_VLAR
                fprintf(stderr, "put_vlar 0x%08x\n", (UINT32)vlar);
#endif /* M4_DEBUG_VLAR */
                put_vlar(vlar);
                vlar = pvlar;
            }

            P6_Process_VDD_Loss(gVDX.vdd[currVID]);
            DEF_DeallocVDStatsMemory(V_vddindx[currVID]);

            s_Free(gVDX.vdd[currVID], sizeof(VDD), __FILE__, __LINE__);
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            gVDX.vdd[currVID] = NULL;
            --gVDX.count;
        }
    }
    return (currVID);
}   /* End of NV_ClearRemainingVDD */

/**
******************************************************************************
**
**  @brief      Add remaining PDDs to index table.
**
**  @param      pTable - temp pdd table.
**
**  @return     none
**
******************************************************************************
**/
void NV_RecordRemainingPID(PDX *pTable)
{
    PDD        *pdd;
    UINT16      i;

    /*
     * Now that the list was processed, all physical devices (drives,
     * enclosures, miscellaneous) should be in the proper lists. If the
     * temp list has anything in it at this point, then it is previously
     * unknown or is previously known but not participating in any config.
     * In this case, just add it into the appropriate table and log its
     * entry into the sytem.
     */
    if (pTable != NULL)
    {
        for (i = 0; i < MAX_PHYSICAL_DISKS; i++)
        {
            if ((pdd = pTable->pdd[i]) != 0)
            {
                pTable->pdd[i] = NULL;

                /* Insert and log this device. */
                pdd->miscStat = 0;

                /*
                 * If this is a bay, then it must be directly addressable, so
                 * go ahead and enter it according to the slot number.
                 */
                if ((pdd->devType > PD_DT_MAX_DISK) && (pdd->devType <= PD_DT_MAX_SES))
                {
                    /*
                     * Check for the bay information. Do not send in a counter
                     * and log the entry of a new bay.
                     */
                    SES_GetDirectEnclosure(0, 0, pdd, NULL, TRUE);
                }
                else
                {
                    BIT_SET(pdd->miscStat, PD_MB_FSERROR);
                    DEF_InsertPDD(pdd);
                    ON_LogDriveInserted(pdd);
                    NV_SendFSys(FALSE);
                }
            }
        }
    }
}   /* End of NV_RecordRemainingPID */

/**
******************************************************************************
**
**  @brief      Return the local image size from a local image.
**
**              The size is simply pulled from the local image structure.
**
**  @param      lcl - Location of the local image.
**
**  @return     Size of the local image.
**
******************************************************************************
**/
UINT32 NV_GetLocalImageSize(NVRL *lcl)
{
    return (lcl->length);
}   /* End of NV_GetLocalImageSize */

/**
******************************************************************************
**
**  @brief      Create a local image for NVRAM.
**
**              The RAIDs and physical devices are examined and a local image
**              is built from this information.
**
**  @param      lcl - Location of the local image.
**
**  @return     none
**
******************************************************************************
**/
void NV_BuildLocalImage(NVRL *lcl)
{
    NVR        *nvrec;          /* NVRAM record pointer                 */
    NVX        *nvxrec;         /* NVRAM extension record pointer       */
    RDD        *rdd;            /* RDD pointer to move through lists    */
    PSD        *psd;            /* PSD pointers to move through lists   */
    PDD        *pdd;            /* PDD pointer to move through lists    */
    UINT16      index_n;        /* Array index                          */

    /* First build up the header. */
    lcl->ctrlID = K_ficb->cSerial;
    lcl->mirrorPartner = K_ficb->mirrorPartner;
    lcl->seqNumber = imageSeq++;

    /* Start out the nvrec pointer at the end of the local image header. */
    nvrec = (NVR *)(lcl + 1);

    /*
     * Now parse the RAIDs. Only the RAIDs which we own are recorded in
     * the table. Make sure the nvx record address is also set up in case
     * there are no RAIDs. If it is not set up, then the drive loop
     * below will fail.
     */
    for (index_n = 0; index_n < MAX_RAIDS; index_n++)
    {
        if (((rdd = R_rddindx[index_n]) != 0) &&
            (DL_AmIOwner(rdd->vid) == TRUE) && !BIT_TEST(rdd->aStatus, RD_A_UNINIT))
        {
            /* Fill in a new entry for this RAID (version 2) */
            nvrec->hdr.recType = NRL_RAID2;
            nvrec->u.lraid2.rid = index_n;
            nvrec->u.lraid2.psdCnt = rdd->psdCnt;
            nvrec->u.lraid2.aStatus = rdd->aStatus;
            nvrec->u.lraid2.notMirroringCSN = rdd->notMirroringCSN;

            nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRH) + sizeof(NVRLRDD2));

            /*
             * The PSDs form a circular list. Grab the first one (it follows
             * the RDD structure in memory) and run around the list.
             */
            psd = *((PSD **)(rdd + 1));

            /* Fill in the PSD extensions to the RAID. */
            do
            {
                nvxrec->u.lpsd.pid = psd->pid;
                nvxrec->u.lpsd.status = psd->status;
                nvxrec->u.lpsd.aStatus = psd->aStatus & ~(1 << PSDA_ERROR);

                nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRLXPSD));
                psd = psd->npsd;
            }
            while (psd != *((PSD **)(rdd + 1)));

            /* Set the length */
            nvrec->hdr.recLen = (UINT32)nvxrec - (UINT32)nvrec;
            nvrec = (NVR *)((UINT32)nvrec + nvrec->hdr.recLen);
        }
    }

    /*
     * Now parse the drives. Each drive is recorded in the table regardless
     * of status. The status' are gathered by each controller to determine
     * the list of available drives to use for the file system.
     */
    for (index_n = 0, nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRH)),
         nvrec->hdr.recType = NRL_PHYS; index_n < MAX_PHYSICAL_DISKS; index_n++)
    {
        if ((pdd = P_pddindx[index_n]) != 0)
        {
            /* Fill in a new entry for this drive. */
            nvxrec->u.lphys.pid = index_n;

            /*
             * If the device is non-existent and in process of rebuilding,
             * change the status until the rebuild is done.
             */
            if ((pdd->devStat == PD_NONX) &&
                ((BIT_TEST(pdd->miscStat, PD_MB_SCHEDREBUILD)) ||
                 (BIT_TEST(pdd->miscStat, PD_MB_REBUILDING))))
            {
                nvxrec->u.lphys.status = PD_NONX_REBUILDING;
            }
            else
            {
                nvxrec->u.lphys.status = pdd->devStat;
            }

            nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRLXPDD));
        }
    }

    /*
     * The following movement of the pointers is required since the
     * drives are placed in one record rather than one per drive. Even
     * if there are no drives, the pointers will be moved out by one,
     * mostly empty record.
     */
    nvrec->hdr.recLen = (UINT32)nvxrec - (UINT32)nvrec;
    nvrec = (NVR *)((UINT32)nvrec + nvrec->hdr.recLen);

    /* Add an end of file record. */
    nvrec->hdr.recType = NRT_EOF;
    nvrec->hdr.recLen = sizeof(NVRH);
    nvrec = (NVR *)((UINT32)nvrec + sizeof(NVRH));

    /* Set length. */
    lcl->length = ((UINT32)nvrec - (UINT32)lcl + 15) & 0xFFFFFFF0;
}   /* End of NV_BuildLocalImage */

/**
******************************************************************************
**
**  @brief      Add or replace a local image for NVRAM.
**
**              The local image passed in is placed into the NVRAM image and
**              is put into the NVRAM itself.
**
**  @param      lcl - Location of the local image.
**
**  @return     none
**
******************************************************************************
**/
void NV_UpdateLocalImage(NVRL *lcl)
{
    NVX        *nvxrec;         /* Pointer to NVRAM extension record        */
    NVR        *nvrec;          /* Pointer to NVRAM record                  */
    UINT8       psdcnt;         /* Temporary counter                        */
    PSD        *currpsd;        /* Current PSD in RDD PSD traversal         */
    UINT8       index_n;        /* Array index                              */
    bool        nvramUpdateNeeded = false;      /* NVRAM Update Needed Flag */

    /*
     * Parse through the local image being put into the NVRAM. For most
     * fields, we will ignore the data if it came from us.
     */
    for (nvrec = (NVR *)((UINT32)lcl + sizeof(NVRL));
         nvrec->hdr.recType != NRT_EOF;
         nvrec = (NVR *)((UINT32)nvrec + nvrec->hdr.recLen))
    {
        if (nvrec->hdr.recType == NRL_PHYS)
        {
            /*
             * Loop through the extensions. There is one extension
             * for each drive. The ending point is determined by
             * the length in the record header.
             */
            for (nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRH));
                 (UINT32)nvxrec < ((UINT32)nvrec + nvrec->hdr.recLen);
                 nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRLXPDD)))
            {
                if (gPDX.pdd[nvxrec->u.lphys.pid] != NULL)
                {
                    if (lcl->ctrlID != K_ficb->cSerial)
                    {
                        /*
                         * First check for non-existent. If the drive does not
                         * appear as existent on the controller putting the local
                         * image and it is not present on the master (this
                         * controller) and if there are no active definitions
                         * on the drive, then delete it. The refresh of NVRAM
                         * on the slave will cause the PDD to be deleted.
                         * Note that this does not work well for N > 2 since the
                         * drive will be deleted.
                         */
                        if ((nvxrec->u.lphys.status == PD_NONX) &&
                            (gPDX.pdd[nvxrec->u.lphys.pid]->devStat == PD_NONX))
                        {
                            DA_DAMDirty(nvxrec->u.lphys.pid);

                            if (!RB_ActDefCheck(gPDX.pdd[nvxrec->u.lphys.pid]) &&
                                !FAB_IsDevInUse(gPDX.pdd[nvxrec->u.lphys.pid]->pDev) &&
                                !(BIT_TEST
                                  (gPDX.pdd[nvxrec->u.lphys.pid]->miscStat,
                                   PD_MB_SCHEDREBUILD)) &&
                                !(BIT_TEST
                                  (gPDX.pdd[nvxrec->u.lphys.pid]->miscStat,
                                   PD_MB_REBUILDING)) &&
                                (TaskGetState(S_bgppcb) == PCB_NOT_READY))
                            {
                                if (gPDX.pdd[nvxrec->u.lphys.pid]->pDev != NULL)
                                {
                                    s_Free(gPDX.pdd[nvxrec->u.lphys.pid]->pDev,
                                           sizeof(DEV), __FILE__, __LINE__);
                                }

                                DC_RelPDD(gPDX.pdd[nvxrec->u.lphys.pid]);
                                gPDX.pdd[nvxrec->u.lphys.pid] = NULL;
                                --gPDX.count;
                            }
                        }
                    }
                    else if (!BIT_TEST(K_ii.status, II_CCBREQ))
                    {
                        /*
                         * We are the only controller left in the config, so do the
                         * deletions if the drives are nonexistent and they have no
                         * data defined on them.
                         */
                        if (gPDX.pdd[nvxrec->u.lphys.pid]->devStat == PD_NONX)
                        {
                            DA_DAMDirty(nvxrec->u.lphys.pid);

                            if (!RB_ActDefCheck(gPDX.pdd[nvxrec->u.lphys.pid]) &&
                                !FAB_IsDevInUse(gPDX.pdd[nvxrec->u.lphys.pid]->pDev) &&
                                !(BIT_TEST
                                  (gPDX.pdd[nvxrec->u.lphys.pid]->miscStat,
                                   PD_MB_SCHEDREBUILD)) &&
                                !(BIT_TEST
                                  (gPDX.pdd[nvxrec->u.lphys.pid]->miscStat,
                                   PD_MB_REBUILDING)) &&
                                (TaskGetState(S_bgppcb) == PCB_NOT_READY))
                            {
                                if (gPDX.pdd[nvxrec->u.lphys.pid]->pDev != NULL)
                                {
                                    s_Free(gPDX.pdd[nvxrec->u.lphys.pid]->pDev,
                                           sizeof(DEV), __FILE__, __LINE__);
                                }

                                DC_RelPDD(gPDX.pdd[nvxrec->u.lphys.pid]);
                                gPDX.pdd[nvxrec->u.lphys.pid] = NULL;
                                --gPDX.count;
                            }
                        }
                    }
                }
            }
        }
        else if ((lcl->ctrlID == K_ficb->cSerial) &&
                 (nvrec->hdr.recType == NRL_RAID2) &&
                 (R_rddindx[nvrec->u.lraid2.rid] != NULL))
        {
            /*
             * This record is for a RAID owned by this controller. Determine
             * if the Local Image in Progress bit is on in this RAIDs status
             * and the Images RAID status. If so, then clear the bit and set
             * a flag to do another P2Update to refresh everyone else.
             */
            if ((BIT_TEST(R_rddindx[nvrec->u.lraid2.rid]->aStatus,
                          RD_A_LOCAL_IMAGE_IP)) &&
                (BIT_TEST(nvrec->u.lraid2.aStatus, RD_A_LOCAL_IMAGE_IP)))
            {
                /* The new image has the bit on, clear the bit and set the flag */
                BIT_CLEAR(R_rddindx[nvrec->u.lraid2.rid]->aStatus, RD_A_LOCAL_IMAGE_IP);
                BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
#if GR_GEORAID15_DEBUG
                fprintf(stderr, "<GR><NV_UpdateLocalImage-nvram.c>Clear LCL IMAGE IN PROGRSS flag %x\n",
                        (UINT32)(nvrec->u.lraid2.rid));
#endif
                nvramUpdateNeeded = true;
            }
        }
        else if ((lcl->ctrlID != K_ficb->cSerial) &&
                 (((nvrec->hdr.recType == NRL_RAID) &&
                   (R_rddindx[nvrec->u.lraid.rid] != NULL)) ||
                  ((nvrec->hdr.recType == NRL_RAID2) &&
                   (R_rddindx[nvrec->u.lraid2.rid] != NULL))))
        {
            /*
             * The extension record must be examined and the
             * status of each PSD in the RDD table must be updated
             * with the new status. Each of the local images will
             * only have the records for the RAIDs that it owns,
             * so ownership does not have to be tested. In addition,
             * the PSDs will be in the correct order, so we should
             * only have to take the new status and place it into
             * the PSD without validating the PSD PID.
             */
            if (nvrec->hdr.recType == NRL_RAID) /* Version 1 of RAID Lcl Image */
            {
                psdcnt = nvrec->u.lraid.psdCnt;
                currpsd = *(PSD **)(R_rddindx[nvrec->u.lraid.rid] + 1);
                BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                R_rddindx[nvrec->u.lraid.rid]->aStatus = nvrec->u.lraid.aStatus;
                nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRH) + sizeof(NVRLRDD));
            }
            else                /* Version 2 of the RAID Local Image                       */
            {
                psdcnt = nvrec->u.lraid2.psdCnt;
                currpsd = *(PSD **)(R_rddindx[nvrec->u.lraid2.rid] + 1);

                /*
                 * If the RAID Status goes from Inoperative Resync Required to
                 * Degraded Rebuild Required, then the slave has fixed up a
                 * previous problem. Let rebuild handle a rebuild or hotswap.
                 */
                if ((R_rddindx[nvrec->u.lraid2.rid]->status == RD_INOP) &&
                    (BIT_TEST(R_rddindx[nvrec->u.lraid2.rid]->aStatus, RD_A_PARITY)) &&
                    (!BIT_TEST(nvrec->u.lraid2.aStatus, RD_A_PARITY)))
                {
                    RB_AcceptIOError();
                }

                BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                R_rddindx[nvrec->u.lraid2.rid]->aStatus = nvrec->u.lraid2.aStatus;
                R_rddindx[nvrec->u.lraid2.rid]->notMirroringCSN = nvrec->u.lraid2.notMirroringCSN;
                nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRH) + sizeof(NVRLRDD2));
            }

            for (;
                 psdcnt > 0;
                 nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRLXPSD)),
                 currpsd = currpsd->npsd, psdcnt--)
            {
                /*
                 * Only change the PSD status and astatus if the PIDs still
                 * match. Otherwise the local image has stale data and do not
                 * want to clobber the more recent stuff.
                 */
                if (currpsd->pid == nvxrec->u.lpsd.pid)
                {
                    /*
                     * If the rebuild or hotspare bits are now on, then the
                     * slave has detected an IO error. Let rebuild handle it.
                     */
                    if ((!BIT_TEST(currpsd->aStatus, PSDA_HOT_SPARE) &&
                         BIT_TEST(nvxrec->u.lpsd.aStatus, PSDA_HOT_SPARE)) ||
                        (!BIT_TEST(currpsd->aStatus, PSDA_REBUILD) &&
                         BIT_TEST(nvxrec->u.lpsd.aStatus, PSDA_REBUILD)))
                    {
                        RB_AcceptIOError();
                    }

                    BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                    currpsd->status = nvxrec->u.lpsd.status;
                    currpsd->aStatus = nvxrec->u.lpsd.aStatus;
                    /*
                     * If the PSD requires a hotspare, then that drive must be
                     * replaced. Force that PDD inoperable & set the replace bit
                     * in it's misc status.
                     */
                    if (BIT_TEST(currpsd->aStatus, PSDA_HOT_SPARE_REQD))
                    {
                        if (P_pddindx[currpsd->pid]->devStat != PD_NONX)
                        {
                            P_pddindx[currpsd->pid]->devStat = PD_INOP;
                        }
                        BIT_SET(P_pddindx[currpsd->pid]->miscStat, PD_MB_REPLACE);
                    }
                }
            }

            /* Set the RAID status based upon the new values. */
            if (nvrec->hdr.recType == NRL_RAID) /* Version 1 of RAID Lcl Image */
            {
                RB_setraidstat(R_rddindx[nvrec->u.lraid.rid]);
            }
            else                /* Version 2 of RAID Local Image                          */
            {
                RB_setraidstat(R_rddindx[nvrec->u.lraid2.rid]);
            }
        }
    }

    /*
     * Update the mirror partners. First, look to see if the one we are
     * updating is already in the mappings. If so, just replace it. If
     * not, create a new entry.
     */
    for (index_n = 0; index_n < MAX_CTRL; index_n++)
    {
        if (D_mpmaps[index_n].source == lcl->ctrlID)
        {
            D_mpmaps[index_n].source = lcl->ctrlID;
            D_mpmaps[index_n].dest = lcl->mirrorPartner;
            break;
        }
    }

    /* Make sure that the controller map doesn't overflow. */
    if ((index_n == MAX_CTRL) && (D_mpcnt < MAX_CTRL))
    {
        D_mpmaps[D_mpcnt].source = lcl->ctrlID;
        D_mpmaps[D_mpcnt++].dest = lcl->mirrorPartner;
    }

    /*
     * Now set the virtual disk and physical disk statuses based upon any
     * changes that occurred. Also update the FE with any Rebuild Write
     * Status changes.
     */
    DEF_UMiscStat();
    RB_setpsdstat();
    RB_UpdateRebuildWriteState(true, false);
    RB_SetVirtStat();

    /* Now update the NVRAM. */
    NV_P2UpdateNvram();

    /*
     * Determine if changes to this controllers owned items has occurred. If
     * so, do another P2 Update to get it refreshed again.
     */
    if (nvramUpdateNeeded == true)
    {
        NV_P2Update();
    }
}   /* End of NV_UpdateLocalImage */

/**
******************************************************************************
**
**  @brief      Refresh the NVRAM for PART II from the copy sent in.
**
**              The NVRAM is written with the copy passed in. In addition, the
**              PDDs are updated with the aggregate status of all the local
**              images in the NVRAM. This is done to allow the file system to
**              have a more accurate image of the drives that are accessible
**              to all of the controllers.
**
**
**  @param      nvramImage  - Pointer to a view of the PII NVRAM.
**
**  @return     none
**
******************************************************************************
**/
void NV_RefreshNvram(NVRII *nvramImage)
{
    NVR        *nvrec;          /* Pointer into the NVRAM records           */
    NVX        *nvxrec;         /* Pointer into the NVRAM extensions        */
    VDD        *vdd;            /* VDD pointer to move through lists        */
    RDD        *rdd = NULL;     /* RDD pointer to move through lists        */
    PSD        *psd = NULL;     /* PSD pointer                              */
    VLAR       *vlar = 0;       /* VLAR pointer to move through VDD         */
    VLAR       *pvlar = 0;      /* Previous VLAR pointer to move through VDD */
    LDD        *ldd = NULL;     /* LDD pointer to move through lists        */
    UINT32      j;              /* Temp variables                           */
    UINT32      i;              /* Counter for loops                        */
    PDD        *pdd;            /* Pointer to a PDD                         */
    PSD        *currpsd;        /* Pointer to a PSD                         */
    UINT16      lid = 0;        /* Current LID being loaded                 */
    UINT32      psdcnt;         /* Number of PSDs to do                     */
    UINT16      currPID = -1;   /* PID for deletion processing              */
    bool        nvramUpdateNeeded = false;      /* P2 Update needed flag     */

    /* Ignore refresh if CCSM fresh flag is set */
    if (CCSM_fresh_flag)
    {
        ++CCSM_fresh_count;
    }
    else
    {
        /*
         * Clear all PDD flags for the file system before pulling in all the
         * individual status.
         */
        for (i = 0; i < MAX_PHYSICAL_DISKS; i++)
        {
            if ((pdd = P_pddindx[i]) != 0)
            {
                BIT_CLEAR(pdd->miscStat, PD_MB_REPLACE);
            }
        }

        /* Clear the mirror partner mappings. */
        for (i = 0, D_mpcnt = 0; i < MAX_CTRL; i++)
        {
            D_mpmaps[i].source = D_mpmaps[i].dest = 0;
        }

        /*
         * Update the PDDs based upon the status from all controllers and the
         * Vlars.
         */
        for (nvrec = (NVR *)(nvramImage + 1);
             nvrec->hdr.recType != NRT_EOF;
             nvrec = (NVR *)((UINT32)nvrec + nvrec->hdr.recLen))
        {
            if (nvrec->hdr.recType == NRT_PHYS)
            {
                /*
                 * Process the previous records. Delete all records prior to
                 * this one and since the last one. This will cause a deletion
                 * of the records that the master blew away.
                 */
                for (currPID++; currPID < nvrec->u.phys.pid; currPID++)
                {
                    if (P_pddindx[currPID] != NULL)
                    {
                        DA_DAMDirty(currPID);

                        if (!RB_ActDefCheck(P_pddindx[currPID]) &&
                            !FAB_IsDevInUse(P_pddindx[currPID]->pDev) &&
                            !(BIT_TEST(P_pddindx[currPID]->miscStat,
                                       PD_MB_SCHEDREBUILD)) &&
                            !(BIT_TEST(P_pddindx[currPID]->miscStat, PD_MB_REBUILDING)) &&
                            (TaskGetState(S_bgppcb) == PCB_NOT_READY))
                        {
                            if (P_pddindx[currPID]->pDev != NULL)
                            {
                                s_Free(P_pddindx[currPID]->pDev, sizeof(DEV), __FILE__,
                                       __LINE__);
                            }

                            DC_RelPDD(P_pddindx[currPID]);
                            P_pddindx[currPID] = NULL;
                            *(UINT16 *)((UINT32 *)P_pddindx - 1) -= 1;
                        }
                    }
                }

                if (P_pddindx[nvrec->u.phys.pid] != NULL)
                {
                    /*
                     * Grab the fs error bit field from the PDD records. This was
                     * set by the master as an accumulation of the status fields of
                     * all the controllers.
                     */
                    P_pddindx[nvrec->u.phys.pid]->miscStat |=
                        (nvrec->u.phys.miscStat & (1 << PD_MB_FSERROR));
                    P_pddindx[nvrec->u.phys.pid]->miscStat |=
                        (nvrec->u.phys.miscStat & (1 << PD_MB_REPLACE));
                }
            }
            else if (nvrec->hdr.recType == NRT_MIRROR)
            {
                /* Load up the mirror partner information into the tables. */
                D_mpmaps[D_mpcnt].source = nvrec->u.mirror.mySerial;
                D_mpmaps[D_mpcnt++].dest = nvrec->u.mirror.myPartner;
            }
            else if ((nvrec->hdr.recType == NRT_RAID) &&
                     (R_rddindx[nvrec->u.raid.rid] != NULL))
            {
                if (nvrec->u.raid.owner == K_ficb->cSerial)
                {
                    /*
                     * This record is for a RAID owned by this controller. Determine
                     * if the Local Image in Progress bit is on in this RAIDs status
                     * and the Images RAID status. If so, then clear the bit, and set
                     * a flag to do another P2Update to refresh everyone else.
                     */
                    if ((BIT_TEST(R_rddindx[nvrec->u.raid.rid]->aStatus, RD_A_LOCAL_IMAGE_IP)) &&
                        (BIT_TEST(nvrec->u.raid.aStatus, RD_A_LOCAL_IMAGE_IP)))
                    {
                        /* The new image has the bit on, clear the bit and set the flag */
#if GR_GEORAID15_DEBUG
                        fprintf(stderr, "<GR><NV_RefreshNvram-nvram.c.TODO>Clearing LOCAL IMAGE IN PROGRESS flag.. rid=%x\n",
                                (UINT32)(nvrec->u.raid.rid));
#endif
                        BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                        BIT_CLEAR(R_rddindx[nvrec->u.raid.rid]->aStatus, RD_A_LOCAL_IMAGE_IP);
                        nvramUpdateNeeded = true;
                    }
                }

                if ((nvrec->u.raid.owner != K_ficb->cSerial) ||
                    (BIT_TEST(R_rddindx[nvrec->u.raid.rid]->aStatus, RD_A_UNINIT)))
                {

                    /*
                     * The extension record must be examined and the
                     * status of each PSD in the RDD table must be updated
                     * with the new status. This is done to keep the RAID
                     * rebuild status up to date so that the checking for
                     * can spare is accurate.
                     */
                    BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                    for (psdcnt = nvrec->u.raid.devCount,
                         currpsd = *(PSD **)(R_rddindx[nvrec->u.raid.rid] + 1),
                         R_rddindx[nvrec->u.raid.rid]->aStatus = nvrec->u.raid.aStatus,
                         R_rddindx[nvrec->u.raid.rid]->notMirroringCSN = nvrec->u.raid.notMirrorCSN,
                         nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRH) + sizeof(NVRR));
                         psdcnt > 0;
                         nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRRX)),
                         currpsd = currpsd->npsd, psdcnt--)
                    {
                        BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                        currpsd->status = nvxrec->u.raid.status;
                        currpsd->aStatus = nvxrec->u.raid.aStatus;

                        /*
                         * If the PSD requires a hotspare, then that drive must be
                         * replaced. Force that PDD inoperable & set the replace bit in
                         * it's misc status.
                         */
                        if (BIT_TEST(currpsd->aStatus, PSDA_HOT_SPARE_REQD) &&
                            (currpsd->pid == nvxrec->u.raid.pid))
                        {
                            if (P_pddindx[currpsd->pid]->devStat != PD_NONX)
                            {
                                P_pddindx[currpsd->pid]->devStat = PD_INOP;
                            }
                            BIT_SET(P_pddindx[currpsd->pid]->miscStat, PD_MB_REPLACE);
                        }
                    }

                    /* Set the RAID status based upon the new values. */
                    RB_setraidstat(R_rddindx[nvrec->u.raid.rid]);
                }
            }
            else if ((nvrec->hdr.recType == NRT_RAID_GT2TB) &&
                     (R_rddindx[nvrec->u.raid.rid] != NULL))
            {
                if (nvrec->u.raidGT2TB.owner == K_ficb->cSerial)
                {
                    /*
                     * This record is for a RAID owned by this controller. Determine
                     * if the Local Image in Progress bit is on in this RAIDs status
                     * and the Images RAID status. If so, then clear the bit, and set
                     * a flag to do another P2Update to refresh everyone else.
                     */
                    if ((BIT_TEST(R_rddindx[nvrec->u.raidGT2TB.rid]->aStatus, RD_A_LOCAL_IMAGE_IP)) &&
                        (BIT_TEST(nvrec->u.raidGT2TB.aStatus, RD_A_LOCAL_IMAGE_IP)))
                    {
                        /* The new image has the bit on, clear the bit and set the flag */
#if GR_GEORAID15_DEBUG
                        fprintf(stderr, "<GR><NV_RefreshNvram-nvram.c.TODO>Clearing LOCAL IMAGE IN PROGRESS flag.. rid=%x\n",
                                (UINT32)(nvrec->u.raidGT2TB.rid));
#endif
                        BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                        BIT_CLEAR(R_rddindx[nvrec->u.raidGT2TB.rid]->aStatus, RD_A_LOCAL_IMAGE_IP);
                        nvramUpdateNeeded = true;
                    }
                }

                if ((nvrec->u.raidGT2TB.owner != K_ficb->cSerial) ||
                    (BIT_TEST(R_rddindx[nvrec->u.raidGT2TB.rid]->aStatus, RD_A_UNINIT)))
                {

                    /*
                     * The extension record must be examined and the
                     * status of each PSD in the RDD table must be updated
                     * with the new status. This is done to keep the RAID
                     * rebuild status up to date so that the checking for
                     * can spare is accurate.
                     */
                    BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                    for (psdcnt = nvrec->u.raidGT2TB.devCount,
                         currpsd = *(PSD **)(R_rddindx[nvrec->u.raidGT2TB.rid] + 1),
                         R_rddindx[nvrec->u.raidGT2TB.rid]->aStatus = nvrec->u.raidGT2TB.aStatus,
                         R_rddindx[nvrec->u.raidGT2TB.rid]->notMirroringCSN = nvrec->u.raidGT2TB.notMirrorCSN,
                         nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRH) + sizeof(NVRR_GT2TB));
                         psdcnt > 0;
                         nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRRX_GT2TB)),
                         currpsd = currpsd->npsd, psdcnt--)
                    {
                        BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                        currpsd->status = nvxrec->u.raidGT2TB.status;
                        currpsd->aStatus = nvxrec->u.raidGT2TB.aStatus;

                        /*
                         * If the PSD requires a hotspare, then that drive must be
                         * replaced. Force that PDD inoperable & set the replace bit in
                         * it's misc status.
                         */
                        if (BIT_TEST(currpsd->aStatus, PSDA_HOT_SPARE_REQD) &&
                            (currpsd->pid == nvxrec->u.raidGT2TB.pid))
                        {
                            if (P_pddindx[currpsd->pid]->devStat != PD_NONX)
                            {
                                P_pddindx[currpsd->pid]->devStat = PD_INOP;
                            }
                            BIT_SET(P_pddindx[currpsd->pid]->miscStat, PD_MB_REPLACE);
                        }
                    }

                    /* Set the RAID status based upon the new values. */
                    RB_setraidstat(R_rddindx[nvrec->u.raidGT2TB.rid]);
                }
            }
            else if (nvrec->hdr.recType == NRT_VIRT)
            {
                if ((vdd = V_vddindx[nvrec->u.virt.vid]) != NULL)
                {
                    /*
                     * Restore possible new attributes. If there are no SCDs or DCD,
                     * clear there associated attribute flags.
                     */
                    vdd->attr = ~(VD_SCD | VD_DCD | VD_SUSPEND) & nvrec->u.virt.attr;

                    if (vdd->pSCDHead != NULL)
                    {
                        vdd->attr |= VD_SCD;
                    }

                    if (vdd->pDCD != NULL)
                    {
                        vdd->attr |= VD_DCD;
                    }
#if defined(MODEL_7000) || defined(MODEL_4700)
                    if (!BIT_TEST(vdd->attr, VD_BVLINK))        /* If not a VLink */
                    {
                        BIT_SET(vdd->attr, VD_BCACHEEN);        /* Set WC on */
                    }
#endif /* MODEL_7000 || MODEL_4700 */
                    /*
                     * Determine if the device capacity has changed and
                     * if it has update the VDD. if the VDD is a VLINK,
                     * the RDD and PSD structures must also be modified.
                     * Because a Vlink only has a single PSD, only the
                     * first requires modification.
                     *
                     * In the case of a regular vdisk, the regular and
                     * the deferred list are to be rebuilt.
                     */
                    if (vdd->devCap != nvrec->u.virt.devCap)
                    {
                        vdd->devCap = nvrec->u.virt.devCap;

                        rdd = vdd->pRDD;

                        if (rdd != NULL)
                        {
                            if (rdd->type == RD_LINKDEV)
                            {
                                BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                                rdd->devCap = vdd->devCap;
                                psd = *((PSD **)(rdd + 1));
                                psd->sLen = rdd->devCap;
                            }
                            else
                            {
                                vdd->raidCnt = nvrec->u.virt.raidCnt;

                                for (i = 0, nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRH) + sizeof(NVRV));
                                     i < nvrec->u.virt.raidCnt;
                                     i++, nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRVX1)))
                                {
                                    if (i == 0)
                                    {
                                        rdd = vdd->pRDD = R_rddindx[nvxrec->u.virt1.rid];
                                    }
                                    else
                                    {
                                        rdd = rdd->pNRDD = R_rddindx[nvxrec->u.virt1.rid];
                                    }

                                    BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                                    rdd->pNRDD = 0;
                                }

                                vdd->draidCnt = nvrec->u.virt.dRaidCnt;

                                for (i = 0, vdd->pDRDD = NULL;
                                     i < nvrec->u.virt.dRaidCnt;
                                     i++, nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRVX2)))
                                {
                                    if (i == 0)
                                    {
                                        rdd = vdd->pDRDD = R_rddindx[nvxrec->u.virt2.rid];
                                    }
                                    else
                                    {
                                        rdd = rdd->pNRDD = R_rddindx[nvxrec->u.virt2.rid];
                                    }

                                    rdd->pNRDD = 0;
                                    BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                                }
                            }
                        }
                        if (BIT_TEST(vdd->attr, VD_BSNAPPOOL))
                        {
                            DEF_update_spool_percent_used(vdd->vid);
                        }
                    }

                    nvxrec = (NVX *)((UINT32)nvrec + sizeof(NVRH) + sizeof(NVRV) +
                                      (nvrec->u.virt.raidCnt * sizeof(NVRVX1)) +
                                      (nvrec->u.virt.dRaidCnt * sizeof(NVRVX1)));

                    for (i = 0;
                         i < nvrec->u.virt.vlarCnt;
                         i++, nvxrec = (NVX *)((UINT32)nvxrec + sizeof(NVRVX3)))
                    {
                        if (i == 0)
                        {
                            if ((vlar = vdd->pVLinks) == NULL)
                            {
                                vlar = vdd->pVLinks = get_vlar();
#ifdef M4_DEBUG_VLAR
                                fprintf(stderr, "get_vlar 0x%08x\n", (UINT32)vlar);
#endif /* M4_DEBUG_VLAR */
                                vlar->link = NULL;
                            }
                        }
                        else
                        {
                            if ((vlar = vlar->link) == NULL)
                            {
                                vlar = vlar->link = get_vlar();
#ifdef M4_DEBUG_VLAR
                                fprintf(stderr, "get_vlar 0x%08x\n", (UINT32)vlar);
#endif /* M4_DEBUG_VLAR */
                                vlar->link = NULL;
                            }
                        }
                        /* Fill in the contents. */
                        vlar->srcSN = nvxrec->u.virt3.srcSN;
                        vlar->srcCluster = nvxrec->u.virt3.srcCluster;
                        vlar->srcVDisk = nvxrec->u.virt3.srcVDisk;
                        vlar->attr = nvxrec->u.virt3.attr;
                        vlar->poll = nvxrec->u.virt3.poll;
                        vlar->repVDisk = nvxrec->u.virt3.repVID;
                        vlar->agent = nvxrec->u.virt3.agnt;

                        for (j = 0; j < 52; j++)
                        {
                            vlar->name[j] = nvxrec->u.virt3.name[j];
                        }
                    }
                    /*
                     *  Determine if there are any extra VLAR's in the chain. If
                     *  there are, remove them from the cain and return them to
                     *  the free pool.
                     */
                    if (i == 0)
                    {
                        if (vdd->pVLinks != NULL)
                        {
                            pvlar = vdd->pVLinks;
                            vdd->pVLinks = NULL;
                        }
                    }
                    else
                    {
                        if (vlar->link != NULL)
                        {
                            pvlar = vlar->link;
                            vlar->link = NULL;
                        }
                    }
                    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */

                    for (vlar = pvlar; vlar != NULL; vlar = pvlar)
                    {
                        pvlar = vlar->link;
#ifdef M4_DEBUG_VLAR
                        fprintf(stderr, "put_vlar 0x%08x\n", (UINT32)vlar);
#endif /* M4_DEBUG_VLAR */
                        put_vlar(vlar);
                    }
                }
            }
            else if (nvrec->hdr.recType == NRT_XLDD)
            {
                lid = nvrec->u.lddx.lid;
                ldd = DLM_lddindx[lid];
                if (ldd != NULL)
                {
                    ldd->baseVDisk = nvrec->u.lddx.baseVDisk;
                    ldd->baseCluster = nvrec->u.lddx.baseCluster;
                    memcpy(ldd->baseName, nvrec->u.lddx.baseName, 16);
                    ldd->baseSN = nvrec->u.lddx.baseSN;
                    ldd->devCap = nvrec->u.lddx.devCap;
                }
            }
            else if ((nvrec->hdr.recType == NRT_DMCR) ||
                     (nvrec->hdr.recType == NRT_COPYCFG))
            {
                /*
                 * Restore the configurations of the current copies. The configuration is restore in a
                 * manner that will cause the copies to be restarted.
                 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                 */
#if GR_GEORAID15_DEBUG
                fprintf(stderr, "<GR><NV_RefreshNvram-nvram.c>Calling P6_RstCpyCfg\n");
#endif
                p6NvrecBase = NVRAM_P2_START + ((UINT32)nvrec - (UINT32)nvramImage);
                nvrec = P6_RstCpyCfg(nvrec);
            }
        }

        /* Now set the virtual disk statuses based upon any changes that occurred. */
        RB_SetVirtStat();

        /*
         * Update the RAID Rebuild Write AStatus based on these updates and inform
         * the Frontend of any change.
         */
        RB_UpdateRebuildWriteState(true, false);

        /*
         * Update the ownership. This is done since a copy state could have
         * changed, forcing the ownership to move from one controller to another.
         */
        DL_SetVDiskOwnership();

        /* Process the remaining records. Delete all records since the last update. */
        for (currPID++; currPID < MAX_PHYSICAL_DISKS; currPID++)
        {
            if (P_pddindx[currPID] != NULL)
            {
                DA_DAMDirty(currPID);

                if (!RB_ActDefCheck(P_pddindx[currPID]) &&
                    !FAB_IsDevInUse(P_pddindx[currPID]->pDev) &&
                    !(BIT_TEST(P_pddindx[currPID]->miscStat, PD_MB_SCHEDREBUILD)) &&
                    !(BIT_TEST(P_pddindx[currPID]->miscStat, PD_MB_REBUILDING)) &&
                    (TaskGetState(S_bgppcb) == PCB_NOT_READY))
                {
                    if (P_pddindx[currPID]->pDev != NULL)
                    {
                        s_Free(P_pddindx[currPID]->pDev, sizeof(DEV), __FILE__, __LINE__);
                    }

                    DC_RelPDD(P_pddindx[currPID]);
                    P_pddindx[currPID] = NULL;
                    *(UINT16 *)((UINT32 *)P_pddindx - 1) -= 1;
                }
            }
        }

        memcpy((void *)NVRAM_P2_START, (void *)nvramImage, NVRAM_P2_SIZE);

        {
            UINT32      rc;
            UINT8      *dst = (UINT8 *)NVRAM_P2_START;
            UINT32      pageAdjust = ((UINT32)dst % getpagesize());

            /* Adjust the input to align with a page boundary */
            rc = msync((UINT8 *)(dst - pageAdjust), (size_t)(i + pageAdjust), MS_SYNC);
            if (rc)
            {
                int         save_errno = errno;

                fprintf(stderr, "msync() addr = 0x%08x, length = 0x%08x\n", (UINT32)dst, i);
                fprintf(stderr, "msync() adjusted addr = 0x%08x, length = 0x%08x\n",
                       (UINT32)dst - pageAdjust, i + pageAdjust);
                fprintf(stderr, "@@@@@@ msync() returned an error = %s @@@@\n",
                       strerror(save_errno));
                fprintf(stderr, "pagesize = 0x%08x\n", getpagesize());
            }
        }
        /* Next, verify that the data written out to NVRAM is good. */
        NV_P2VerifyNvram(nvramImage);

        /*
         * Determine if changes to this controllers owned items has occurred. If
         * so, do another P2 Update to get it refreshed again.
         */
        if (nvramUpdateNeeded == true)
        {
            NV_P2Update();
        }
    }
}   /* End of NV_RefreshNvram */

/**
******************************************************************************
**
**  @brief  Process a FSYS report from a broadcast message
**
**  @param  report  - Pointer to a broadcast report for a FSYS change
**  @param  master  - Boolean - TRUE for master
**
**  @return none
**
******************************************************************************
**/
void NV_ProcessFSys(NVRFSYS * report, UINT8 master)
{
    UINT16      pid;            /* Index into the PID table */
    UINT16      num;            /* Number to process in this packet */

    /*
     * Process the map. There will be a starting PID value in the header
     * to use as the base PID. Process 256 PIDs from that PID on. If the
     * bit is set, then set the fs error bit in the PDD. If it is cleared,
     * then clear the bit if we are not the master.
     */
    num = report->firstPID + report->lastIndex + 1;
    for (pid = report->firstPID; pid < num; ++pid)
    {
        PDD        *pdd = P_pddindx[pid];
        UINT16      mapix;

        if (!pdd)
        {
            continue;
        }

        mapix = pid % 256;
        if (report->map[mapix / 32] & (1 << (mapix % 32)))
        {
            if (master && !BIT_TEST(pdd->miscStat, PD_MB_FSERROR))
            {
                TaskReadyByState(PCB_FILE_SYS_CLEANUP);
            }

            BIT_SET(pdd->miscStat, PD_MB_FSERROR);
        }
        else if (report->flags)
        {
            BIT_CLEAR(pdd->miscStat, PD_MB_FSERROR);
        }
    }
}   /* End of NV_ProcessFSys */

/**
******************************************************************************
**
**  @brief      Create and send an Fsys report
**
**  The list of PDDs is examined and the reports generated and sent.
**  If the miscstat bit is set to indicate a file system is bad,
**  then the bit in the map is set. If not, then the bit is clear.
**
**  @param      master  - Boolean - TRUE to indicate master update
**
**  @return     none
**
******************************************************************************
**/
void NV_SendFSys(UINT16 master)
{
    NVRFSYS    *report;             /* File system report structure         */
    UINT16      pid;                /* Index into the PID table             */
    UINT16      pid_send = FALSE;   /* Indicator of any PIDs in map         */

    /*
     * Build the Fsys reports and then send them. We will do this by creating
     * maps of 256 drives at a time. If there are any drives in that group
     * the map is sent. If there are none, we just go on to the next set of
     * drives.
     */
    report = s_MallocC(sizeof(*report), __FILE__, __LINE__);

    for (pid = 0; pid < MAX_PHYSICAL_DISKS; ++pid)
    {
        PDD        *pdd;
        UINT16      mapix = pid % 256;

        /* Indicate no PIDs valid yet in this section */

        if (mapix == 0)
        {
            pid_send = FALSE;
            report->firstPID = pid;
            report->flags = master;
            memset(report->map, 0, sizeof(report->map));
        }

        pdd = P_pddindx[pid];
        if (pdd)
        {
            pid_send = TRUE;

            if (BIT_TEST(pdd->miscStat, PD_MB_FSERROR))
            {
                report->map[mapix / 32] |= 1 << (mapix % 32);
            }
        }

        /*
         * If we just processed the 256th entry, send the record if the
         * pid_send flag indicates to do so.
         */
        if (mapix == 255 && pid_send)
        {
            report->lastIndex = mapix;
            DEF_ReportEvent(report, EB_FILE_SYS_SIZE, EB_FILE_SYS_SUB, EB_TO_OTHERS, 0);
            pid_send = FALSE;
        }
    }

    if (pid_send)
    {
        report->lastIndex = (pid - 1) % 256;
        DEF_ReportEvent(report, EB_FILE_SYS_SIZE, EB_FILE_SYS_SUB, EB_TO_OTHERS, 0);
    }

    s_Free(report, sizeof(*report), __FILE__, __LINE__);
}   /* End of NV_SendFSys */

/**
******************************************************************************
**
**  @brief      Calculate the size requirement for a local image for NVRAM.
**
**              The RAIDs and physical devices are examined and the size of
**              the local image is calculated from this information.
**
**  @param      none
**
**  @return     Local image size
**
******************************************************************************
**/
UINT32 NV_CalcLocalImageSize(void)
{
    RDD        *rdd;            /* RDD pointer to move through lists            */
    UINT32      size;           /* Cummulative size                             */
    UINT32      index_n;        /* Index into RDD list                          */

    /* First add in the local image header and the end of file marker. */
    size = sizeof(NVRL) + sizeof(NVRH);

    /*
     * Now parse the RAIDs. The size of each RAID is added in if the
     * RAID is owned by this controller. The size of the RAID is based
     * upon the number of drives in the RAID.
     */
    for (index_n = 0; index_n < MAX_RAIDS; index_n++)
    {
        if (((rdd = R_rddindx[index_n]) != 0) &&
            (DL_AmIOwner(rdd->vid) == TRUE) && !BIT_TEST(rdd->aStatus, RD_A_UNINIT))
        {
            /*
             * Calculate the size of the area added to the local image for
             * this RAID. First add in a header for the RAID, then add in
             * the space for each drive.
             */
            size += sizeof(NVRH) + sizeof(NVRLRDD2);
            size += (rdd->psdCnt * sizeof(NVRLXPSD));
        }
    }

    /*
     * Now parse the drives. Each drive is recorded in the table regardless
     * of status. The status' are gathered by each controller to determine
     * the list of available drives to use for the file system.
     */
    size += sizeof(NVRH);
    for (index_n = 0; index_n < MAX_PHYSICAL_DISKS; index_n++)
    {
        if (P_pddindx[index_n] != 0)    /* Not NULL pointer. */
        {
            size += sizeof(NVRLXPDD);
        }
    }

    /* Set length, rounding up. */
    size = (size + 15) & 0xFFFFFFF0;

    return (size);
}   /* End of NV_CalcLocalImageSize */

/**
******************************************************************************
**
**  @brief      Check if part II NVRAM is initialized (i.e. non-zero)
**
**  If NVRAM part II is all zeros, initialize it to a fixed pattern.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void is_nvram_p2_initialized(void)
{
    NVRII      *q = (NVRII *)NVRAM_P2_START;
    UINT32      i = 0;

    /* Quick check for mfgclean (all zeros, but length). */
    if (q->magic == NR_MAGIC && q->length == 0x44 &&
        q->gPri == MAX_GLOBAL_PRIORITY && q->revision == 0)
    {
        ;                       /* From mfgclean, not initialized. */
    }
    else
    {
        UINT8      *p = (UINT8 *)NVRAM_P2_START;

        /* Check for start of p2 section all zeroes. */
        i = sizeof(NVRII) + sizeof(nvram_p2_nvrh);
        while (i > 0)
        {
            if (*p != 0)
            {
                return;
            }
            i--;
            p++;
        }
    }
    /* It is not initialized, initialize NVRII section. */
    memcpy((void *)NVRAM_P2_START, (void *)&nvram_p2_Init, sizeof(nvram_p2_Init));

    /* Nvram part 2 has NVR's following it. */
    memcpy((void *)(q + 1), (void *)&nvram_p2_nvrh, sizeof(nvram_p2_nvrh));
}   /* End of nvram_initialize */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
