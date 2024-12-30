/* $Id: p6.c 157710 2011-08-23 14:32:06Z m4 $ */
/**
******************************************************************************
**
**  @file       p6.c
**
**  @brief      p6 NVRAM configuration
**
**      Provides routines to facilitate the insertion and extraction
**      of NVRAM part 6 information pursuant to re-synchronization.
**
**  Copyright (c) 2002-2011 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "cm.h"
#include "ccsm.h"
#include "copymap.h"
#include "cor.h"
#include "dcd.h"
#include "def.h"
#include "defbe.h"
#include "ficb.h"
#include "fsys.h"
#include "globalOptions.h"
#include "GR_Error.h"
#include "ilt.h"
#include "misc.h"
#include "nvr.h"
#include "nvram.h"
#include "options.h"
#ifdef LINUX_VER_NVP6_MM
#include "NV_Memory.h"
#endif
#include "p6.h"
#include "pcb.h"
#include "pcp.h"
#include "pdd.h"
#include "qu.h"
#include "rcc.h"
#include "RL_RDD.h"
#include "scd.h"
#include "system.h"
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include "vdd.h"
#include "vlar.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "ddr.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "mem_pool.h"           /* Needed for get_rm() and put_rm() */
#include "CT_defines.h"

extern void CM_deact_dcd(DCD* pDCD);
extern void CM_wp2_null(void);
extern void CM_wp2_suspend(void);
extern void CM_wp2_copy(void);
extern void CM_wp2_mirror(void);
extern void CM_wp2_inactive(void);
extern void CM_pexec(void);

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/

/*
** NOTE: These are temporary defines until changes are made in the def.h
**       modual
*/
#define DELOG   0xa0                    /* Log event                        */
#define ECP6MSG 0xe1                    /* message ID                       */

/* EPC error message                                                        */
typedef struct EPCEM
{
    UINT8 type;                         /* epc error type                   */
    UINT8 length;                       /* Message length                   */
    UINT8 id;                           /* message ID                       */
    UINT8 rc;                           /* reason code                      */
} EPCEM;

EPCEM epcErrorMsg = { DELOG, 2, ECP6MSG, 0};

#define P6RE_INVALIDSVID    0           /* invalid source VID               */
#define P6RE_INVALIDDVID    1           /* invalid destination VID          */
#define P6RE_NOSVDD         2           /* no source source VDD             */
#define P6RE_NODVDD         3           /* no destination VDD               */

UINT16 p6RestoreErrors[] = {
    0,0,0,0,0,0,0,0
};

/*
**  Phase 1 and 2 update handler routine enumeration table
**  NOTE: These can NEVER be deleted, only add new ones. The table MUST
**        end with a word of nulls
*/
#define P6_UH_NULL      1           /* null update handler                  */
#define P6_UH_SUSPEND   2           /* copy suspended update handler        */
#define P6_UH_COPY      3           /* copy resyncing update handler        */
#define P6_UH_MIRROR    4           /* copy mirrored update handler         */
#define P6_UH_INACTIVE  5           /* copy inactive update handler         */

void (* p6UpdhndTbl[8])(void) =
{
    NULL,
    CM_wp2_null,
    CM_wp2_suspend,
    CM_wp2_copy,
    CM_wp2_mirror,
    CM_wp2_inactive,
    NULL,
    NULL
};

#ifdef LINUX_VER_NVP6_MM
/*
** Block of local memory(DRAM) representing the NVRAM - P6 in Micro Memory Card.
*/
UINT8 gNvramP6Image[NVRAM_P6_SIZE];
#endif

/*
******************************************************************************
** Private variables
******************************************************************************
*/
extern CM* CM_cm_act_que;
extern UINT8 CM_cm_pri;
extern UINT32 CM_cor_rid;
extern UINT8 CM_proc_pri;
extern UINT8 D_pauseinit;
extern void* CCtbl;
extern void* OCtbl;

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
UINT32 p6NvrecBase = 0;              /* p6 nvrec base address               */
UINT32 p6Base = 0;                   /* p6  base address               */

UINT32 p6err_nullptr = 0;           /* null pointer to record               */
UINT32 p6err_regnum  = 0;           /* region number out of range           */
UINT32 p6setcnt  = 0;               /* set bit count                        */

/*
******************************************************************************
** Public function prototypes - in other files
******************************************************************************
*/
extern INT32   MM_Write(UINT32 destAddr, UINT8 *srcBfr, UINT32 length);
extern INT32   MM_Read(UINT32 srcAddr, UINT8 *destBfr, UINT32 length);
extern void CM_cnt_smap(SM* sm);
extern SM* CM_setsmtbl(void);
extern void CM_dealloc_transRM(COR* cor);
extern void CM_act_cor(COR* pCOR);
extern void CM_act_cm(struct CM* pCM);
extern void CM_act_scd(SCD* pSCD);
extern void CM_act_dcd(DCD* pDCD);
extern void CCSM_start_copy(COR* cor);
extern void CM_mmc_sflag(COR* cor);
extern void CCSM_cosc(UINT8 crstate, UINT8 unused1, UINT8 unused2, COR* cor);
extern void CCSM_term_copy(COR* cor);
extern void CCSM_cco(void);
extern void CM_deact_scd(SCD* pSCD);

/*
******************************************************************************
** Public variables - defined in other files
******************************************************************************
*/
extern COR*    CM_cor_act_que;

/*
******************************************************************************
** Function prototypes
******************************************************************************
*/
void P6_CMSave(P6AR* pP6AR);
void P6_DirtyRegion(UINT32, COR* pCOR);
UINT32 P6_save2fs(void);
void P6_GenCpyCfgRcrd(NVR* nvrec, COR* pCOR);
void P6_WriteToNVRAM( void* dst, void* src, UINT32 length);
UINT32 P6_ValidateP6( P6AR* p6_header);
COR* P6_Build_Base_Copy(NVR* nvrec);
void P6_Refresh_Local_Base_Copy(NVR* nvrec, COR* cor);
void P6_Refresh_Remote_Base_Copy(NVR* nvrec, COR* cor);
CM* p6_Build_CM(NVR* nvrec, COR* cor);
P6ST* P6_FindStRec(COR* cor);
void P6_RemoveInactiveStRec(void);
UINT32 p6_Validate_Vaddr(NVR* nvrec);
void P6_DeallocStRec(UINT32, UINT32, UINT32, COR*);
void P6_Update_Config(COR*);
void P6_LockAllCor(void);
void P6_UnlockAllCor(void);

/**
******************************************************************************
**
**  @brief      To initialize the control structure for the p6 area.
**
**              This routine will initialize the control structure within p6
**              area.
**
**  @param      none
**
**  @return     rc == NULL - no p6 area
**                 != NULL - base address of p6 area
**
******************************************************************************
**/
void P6_Init( void )
{
    UINT32      init_p6 = FALSE;    /* initialize p6 area (default = FALSE) */
    UINT32      crc;                /* tempory crc area                     */
    UINT32      i;                  /* index                                */
    P6AR*       p6_header;          /* pointer to p6 header area            */
    P6AR*       p6_headerTmp;       /* pointer to p6 header area            */
    P6ST*       P6stPtr;            /* tempory pointer to p6 record area    */
    P6ST*       p6defPtr;           /* default pointer to default p6 record */
    P6ST*       p6tmpPtr;           /* tempory pointer to default p6 record */



/*
**
**  Determine if the P6 header CRC is correct:
**      If it is correct
**          Check the CRC of all the record. if a record is
**          found with an incorrect CRC, Set the record to
**          default.
**      If it is incorrect.
**          Reinitialize the p6 header and all records to
**          default.
*/

#ifndef LINUX_VER_NVP6_MM

    p6_header = (P6AR*) NVRAM_P6_START;
    p6Base = NVRAM_P6_START;

#else

    p6_header = (P6AR *)gNvramP6Image;
    p6Base    = (UINT32)gNvramP6Image;
#ifdef DEBUG_P6MM
    fprintf(stderr,"<P6_Init>\nBase address of local P6 buffer %x\n",p6Base);
#endif /*DEBUG_P6MM*/
    /*
    ** Get NVRAM P6 contents from Micro Memory into the local memory.
    **
    ** Note: Local Memory address is mapped to NVRAM_P6_START.
    */
    MM_Read((UINT32)MICRO_MEM_BE_P6_START, (UINT8*)gNvramP6Image, NVRAM_P6_SIZE);

#endif /* LINUX_VER_NVP6_MM */

    crc = MSC_CRC32(p6_header, sizeof(P6AR) - sizeof(p6_header->crc));

    if ((p6_header->crc    == crc)                 &&
        (p6_header->bnr    == *(UINT32 *) "P6ar"))
    {
        P6stPtr = (P6ST*) ((UINT32) p6_header + sizeof(P6AR));

        /*
        **  allocate memory for and setup the default state record. Also
        **  setup pointer to tempory state record mirror.
        */
        p6defPtr = s_MallocC(sizeof(P6ST) * 2, __FILE__, __LINE__);
        p6tmpPtr = p6defPtr + 1;
        p6defPtr->bnr  = *(UINT32 *) "    ";

        /*
        **  check each state record to determine if it is valid. If the record
        **  is not valid, overwrite it with the default record. If the record is
        **  valid, NULL the mirror pointer , crc the record, and write it back
        **  to the P6 area.
        */
        for (i = 0; i < MAX_CORS; ++P6stPtr, ++i)
        {
            memcpy(p6tmpPtr, P6stPtr, sizeof(P6ST));

            crc  = MSC_CRC32(p6tmpPtr, sizeof(P6ST) - sizeof(p6tmpPtr->crc));
            if (crc != p6tmpPtr->crc)
            {
                p6defPtr->cfgrec_nbr = i;
                p6defPtr->crc  = MSC_CRC32(p6defPtr, sizeof(P6ST) - sizeof(p6defPtr->crc));
                P6_WriteToNVRAM(P6stPtr, p6defPtr, sizeof(P6ST));
            }
            else
            {
                if ((P6stPtr->bnr  == *(UINT32 *) "    ")  ||
                    (P6stPtr->bnr  == *(UINT32 *) "P6st"))
                {
                    p6tmpPtr->strcmirror = NULL;
                    p6tmpPtr->crc = MSC_CRC32(p6tmpPtr, sizeof(P6ST) - sizeof(p6tmpPtr->crc));
                    P6_WriteToNVRAM(P6stPtr, p6tmpPtr, sizeof(P6ST));
                }
                else
                {
                    p6defPtr->cfgrec_nbr = i;
                    p6defPtr->crc  = MSC_CRC32(p6defPtr, sizeof(P6ST) - sizeof(p6defPtr->crc));
                    P6_WriteToNVRAM(P6stPtr, p6defPtr, sizeof(P6ST));
                }
            }
        }
        /*
        **  deallocate default record memory
        */
        s_Free(p6defPtr, sizeof(P6ST) * 2, __FILE__, __LINE__);
    }
    else
    {
        init_p6 = TRUE;
    }

    /*
    **  if the p6 area requires initialization, do it now
    */
    if (init_p6 == TRUE)
    {
        /*
        ** Allocate the DRAM to use for the temporary storage of the
        ** NVRAM image.  This will later get copied byte by byte into
        ** the real NVRAM.
        */
        p6_headerTmp = s_MallocC(NVRAM_P6_SIZE, __FILE__, __LINE__);

        /*
        **  Initialize header for the P6 area
        */

        p6_headerTmp->bnr      = *(UINT32 *) "P6ar";
        p6_headerTmp->len      = sizeof(P6AR);
        p6_headerTmp->vers     = VERS;
        p6_headerTmp->rev      = REV;
        p6_headerTmp->stnum    = MAX_CORS;
        p6_headerTmp->stlen    = sizeof(P6ST);
        p6_headerTmp->staddr   = 0;

        p6_headerTmp->crc = MSC_CRC32(p6_headerTmp, sizeof(P6AR) - sizeof(p6_headerTmp->crc));

        /*
        **  Initialize all the state records
        */

        P6stPtr = (P6ST*) ((UINT32) p6_headerTmp + sizeof(P6AR));
        for (i = 0; i < MAX_CORS; ++P6stPtr, ++i)
        {
            P6stPtr->bnr        = *(UINT32 *) "    ";
            P6stPtr->cfgrec_nbr = i;
            P6stPtr->crc        = MSC_CRC32(P6stPtr, sizeof(P6ST) - sizeof(P6stPtr->crc));
        }

        /*
        **  crc the p6 header and write header to the P6 area
        */
        P6_WriteToNVRAM(p6_header, p6_headerTmp, sizeof(P6AR) + (sizeof(P6ST) * MAX_CORS));

        /*
        ** Deallocate the RAM for the image.
        */
        s_Free(p6_headerTmp, NVRAM_P6_SIZE, __FILE__, __LINE__);

    }
}

/**
******************************************************************************
**
**  @brief      To provide a common setting a dirty region in the
**              transfer region map of the p6 copy state record.
**
**  @param      cor     -   COR pointer
**              regNum  -   Region Number
**
**  @return     none
**
******************************************************************************
**/
void P6_SetMainRM(UINT32 regNum, UINT32 x UNUSED, UINT32 y UNUSED, COR* cor)
{
    P6ST*       P6stPtr;            /* tempory pointer to p6 record area    */
    P6ST*       p6tmpPtr;           /* tempory pointer to default p6 record */

    if (regNum < MAXRMCNT)
    {
#if GR_GEORAID15_DEBUG
        fprintf(stderr,"<GR><p6.c>- P6_SetMainRM>\n");
#endif

        P6stPtr = cor->stnvram;
        if (P6stPtr != NULL)
        {
#if GR_GEORAID15_DEBUG
            fprintf(stderr,"<GR>< p6.c>- P6_SetMainRM> P6stPtr is not null\n");
#endif

            p6tmpPtr = P6stPtr->strcmirror;
            if (p6tmpPtr != NULL)
            {
                BIT_SET(p6tmpPtr->rm[regNum / 8], (regNum % 8));
                p6tmpPtr->crc  = MSC_CRC32(p6tmpPtr, sizeof(P6ST) - sizeof(p6tmpPtr->crc));
#if GR_GEORAID15_DEBUG
                fprintf(stderr,"<GR><p6.c>- P6_SetMainRM>p6tmpPtr not NULL-calling P6_WriteToNVRAM\n");
#endif
                P6_WriteToNVRAM(P6stPtr, p6tmpPtr, sizeof(P6ST));
            }
            else
            {
#if GR_GEORAID15_DEBUG
                fprintf(stderr,"<GR><p6.c>- P6_SetMainRM> p6tmpPtr is null\n");
#endif
                ++p6err_nullptr;
            }
        }
        else
        {
#if GR_GEORAID15_DEBUG
            fprintf(stderr,"<GR><p6.c>- P6_SetMainRM> P6stPtr is null\n");
#endif
            /*
            ** bump null pointer count
            */
            ++p6err_nullptr;
        }
    }
    else
    {
#if GR_GEORAID15_DEBUG
        fprintf(stderr,"<GR><p6.c>- P6_SetMainRM> regNum >= maxrmcnt\n");
#endif
        /*
        ** bump region number out of range
        */
        ++p6err_regnum;
    }
}

/**
******************************************************************************
**
**  @brief      To provide a common setting a dirty region in the
**              the transfer region map of the p6 copy state record.
**
**  @param      cor     -   COR pointer
**              regNum  -   Region Number
**
**  @return     none
**
******************************************************************************
**/
void P6_SetTransRM(UINT32 regNum, UINT32 x UNUSED, UINT32 y UNUSED, COR* cor)
{
    P6ST*       P6stPtr;            /* tempory pointer to p6 record area    */
    P6ST*       p6tmpPtr;           /* tempory pointer to default p6 record */
    if (regNum < MAXRMCNT)
    {
        P6stPtr = cor->stnvram;
        if (P6stPtr != NULL)
        {
            p6tmpPtr = P6stPtr->strcmirror;
            if (p6tmpPtr != NULL)
            {
                BIT_SET(p6tmpPtr->trans_rm[regNum / 8], (regNum % 8));
                p6tmpPtr->crc  = MSC_CRC32(p6tmpPtr, sizeof(P6ST) - sizeof(p6tmpPtr->crc));
                P6_WriteToNVRAM(P6stPtr, p6tmpPtr, sizeof(P6ST));
            }
            else
            {
                ++p6err_nullptr;
            }
        }
        else
        {
            /*
            ** bump null pointer count
            */
            ++p6err_nullptr;
        }
    }
    else
    {
        /*
        ** bump region number out of range
        */
        ++p6err_regnum;
    }
}

/**
******************************************************************************
**
**  @brief      To provide a common clearing a dirty region in the
**              region map of the p6 copy state record.
**
**  @param      cor     -   COR pointer
**              regNum  -   Region Number
**
**  @return     none
**
******************************************************************************
**/
void P6_ClrMainRM(UINT32 regNum, UINT32 x UNUSED, UINT32 y UNUSED, COR* cor)
{
    P6ST*       P6stPtr;            /* tempory pointer to p6 record area    */
    P6ST*       p6tmpPtr;           /* tempory pointer to default p6 record */
    if (regNum < MAXRMCNT)
    {
#if GR_GEORAID15_DEBUG
//       fprintf(stderr,"<GR><p6.c- P6_ClrMainRM> regNum < maxrmcnt\n");
#endif

        P6stPtr = cor->stnvram;
        if (P6stPtr != NULL)
        {
#if GR_GEORAID15_DEBUG
//           fprintf(stderr,"<GR><p6.c- P6_ClrMainRM> P6stPtr is not null\n");
#endif

            p6tmpPtr = P6stPtr->strcmirror;
            if (p6tmpPtr != NULL)
            {
                BIT_CLEAR(p6tmpPtr->rm[regNum / 8], (regNum % 8));
                p6tmpPtr->crc  = MSC_CRC32(p6tmpPtr, sizeof(P6ST) - sizeof(p6tmpPtr->crc));
#if GR_GEORAID15_DEBUG
                fprintf(stderr,"<GR><p6.c-P6_ClrMainRM>p6tmpptr not NULL-cMMalling P6_WriteToNVRAM\n");
#endif
                P6_WriteToNVRAM(P6stPtr, p6tmpPtr, sizeof(P6ST));
            }
            else
            {
#if GR_GEORAID15_DEBUG
                fprintf(stderr,"<GR><p6.c- P6_ClrMainRM> p6tmpPtr is null\n");
#endif
                ++p6err_nullptr;
            }
        }
        else
        {
#if GR_GEORAID15_DEBUG
            fprintf(stderr,"<GR><p6.c- P6_ClrMainRM> P6stPtris null\n");
#endif
            /*
            ** bump null pointer count
            */
            ++p6err_nullptr;
        }
    }
    else
    {
#if GR_GEORAID15_DEBUG
        fprintf(stderr,"<GR>p6.c- P6_ClrMainRM> regNum is greater than maxrmcnt\n");
#endif
        /*
        ** bump region number out of range
        */
        ++p6err_regnum;
    }
}

/**
******************************************************************************
**
**  @brief      To provide a common clearing a dirty region in the
**              the transfer region map of the p6 copy state record.
**
**  @param      cor     -   COR pointer
**              regNum  -   Region Number
**
**  @return     none
**
******************************************************************************
**/
void P6_ClrTransRM(UINT32 regNum, UINT32 x UNUSED, UINT32 y UNUSED, COR* cor)
{
    P6ST*       P6stPtr;            /* tempory pointer to p6 record area    */
    P6ST*       p6tmpPtr;           /* tempory pointer to default p6 record */
    if (regNum < MAXRMCNT)
    {
        P6stPtr = cor->stnvram;
        if (P6stPtr != NULL)
        {
            p6tmpPtr = P6stPtr->strcmirror;
            if (p6tmpPtr != NULL)
            {
                BIT_CLEAR(p6tmpPtr->trans_rm[regNum / 8], (regNum % 8));
                p6tmpPtr->crc  = MSC_CRC32(p6tmpPtr, sizeof(P6ST) - sizeof(p6tmpPtr->crc));
                P6_WriteToNVRAM(P6stPtr, p6tmpPtr, sizeof(P6ST));
            }
            else
            {
                ++p6err_nullptr;
            }
        }
        else
        {
            /*
            ** bump null pointer count
            */
            ++p6err_nullptr;
        }
    }
    else
    {
        /*
        ** bump region number out of range
        */
        ++p6err_regnum;
    }
}

/**
******************************************************************************
**
**  @brief      To provide a common means of testing for a dirty region
**              in the region map of the p6 copy state record.
**
**  @param      cor     -   COR pointer
**              regNum  -   Region Number
**
**  @return     rc - TRUE  = region set
**                   FALSE = region clear
**
******************************************************************************
**/
UINT32 P6_TestMainRM(UINT32 regNum, UINT32 x UNUSED, UINT32 y UNUSED, COR* cor)
{
    P6ST*       P6stPtr;            /* tempory pointer to p6 record area    */
    P6ST*       p6tmpPtr;           /* tempory pointer to default p6 record */
    UINT32      rc = FALSE;         /* return code (default region clear)   */

    if (regNum < MAXRMCNT)
    {
        P6stPtr = cor->stnvram;
        if (P6stPtr != NULL)
        {
            p6tmpPtr = P6stPtr->strcmirror;
            if (p6tmpPtr != NULL)
            {
                rc = BIT_TEST(p6tmpPtr->rm[regNum / 8], (regNum % 8));
            }
            else
            {
                ++p6err_nullptr;
            }
        }
        else
        {
            /*
            ** bump null pointer count
            */
            ++p6err_nullptr;
        }
    }
    else
    {
        /*
        ** bump region number out of range
        */
        ++p6err_regnum;
    }
    return(rc);
}

/**
******************************************************************************
**
**  @brief      To provide a common means of testing for a dirty region
**              in the transfer region map of the p6 copy state record.
**
**  @param      cor     -   COR pointer
**              regNum  -   Region Number
**
**  @return     rc - TRUE  = region set
**                   FALSE = region clear
**
******************************************************************************
**/
UINT32 P6_TestTransRM(UINT32 regNum, UINT32 x UNUSED, UINT32 y UNUSED, COR* cor)
{
    P6ST*       P6stPtr;            /* tempory pointer to p6 record area    */
    P6ST*       p6tmpPtr;           /* tempory pointer to default p6 record */
    UINT32      rc = FALSE;         /* return code (default region clear)   */

    if (regNum < MAXRMCNT)
    {
        P6stPtr = cor->stnvram;
        if (P6stPtr != NULL)
        {
            p6tmpPtr = P6stPtr->strcmirror;
            if (p6tmpPtr != NULL)
            {
                rc = BIT_TEST(p6tmpPtr->trans_rm[regNum / 8], (regNum % 8));
            }
            else
            {
                ++p6err_nullptr;
            }
        }
        else
        {
            /*
            ** bump null pointer count
            */
            ++p6err_nullptr;
        }
    }
    else
    {
        /*
        ** bump region number out of range
        */
        ++p6err_regnum;
    }
    return(rc);
}

/**
******************************************************************************
**
**  @brief      To provide a common setting all regions in the
**              region map of the p6 copy state record.
**
**  @param      cor     -   COR pointer
**
**  @return     none
**
******************************************************************************
**/
void P6_SetAllMainRM(UINT32 w UNUSED, UINT32 x UNUSED, UINT32 y UNUSED, COR* cor)
{
    P6ST*       P6stPtr;            /* tempory pointer to p6 record area    */
    P6ST*       p6tmpPtr;           /* tempory pointer to default p6 record */
    UINT32      numReg;             /* number of regions to set             */
    UINT32      i;                  /* index                                */

    P6stPtr = cor->stnvram;
    if (P6stPtr != NULL)
    {
        p6tmpPtr = P6stPtr->strcmirror;
        if (p6tmpPtr != NULL)
        {
            numReg = (cor->totalsegs / REGSIZE_SEG) + ((cor->totalsegs % REGSIZE_SEG) ? 1 : 0);

            for (i = 0; i < numReg; ++i)
            {
                BIT_SET(p6tmpPtr->rm[i / 8], (i % 8));
            }

            p6tmpPtr->crc  = MSC_CRC32(p6tmpPtr, sizeof(P6ST) - sizeof(p6tmpPtr->crc));
            P6_WriteToNVRAM(P6stPtr, p6tmpPtr, sizeof(P6ST));
        }
        else
        {
            ++p6err_nullptr;
        }
    }
    else
    {
        /*
        ** bump null pointer count
        */
        ++p6err_nullptr;
    }
}

/**
******************************************************************************
**
**  @brief      To provide a common setting all regions in the
**              transfer region map of the p6 copy state record.
**
**  @param      cor     -   COR pointer
**
**  @return     none
**
******************************************************************************
**/
void P6_SetAllTransRM(UINT32 w UNUSED, UINT32 x UNUSED, UINT32 y UNUSED, COR* cor)
{
    P6ST*       P6stPtr;            /* tempory pointer to p6 record area    */
    P6ST*       p6tmpPtr;           /* tempory pointer to default p6 record */
    UINT32      numReg;             /* number of regions to set             */
    UINT32      i;                  /* index                                */

    P6stPtr = cor->stnvram;
    if (P6stPtr != NULL)
    {
#if GR_GEORAID15_DEBUG
        fprintf(stderr,"<GR><p6.c-P6_SetAllTransRM>P6stPtr is not NULL\n");
#endif
        p6tmpPtr = P6stPtr->strcmirror;
        if (p6tmpPtr != NULL)
        {
#if GR_GEORAID15_DEBUG
            fprintf(stderr,"<GR><p6.c-P6_SetAllTransRM>P6tmpPtr is not NULL\n");
#endif
            numReg = (cor->totalsegs / REGSIZE_SEG) + ((cor->totalsegs % REGSIZE_SEG) ? 1 : 0);

            for (i = 0; i < numReg; ++i)
            {
#if GR_GEORAID15_DEBUG
                fprintf(stderr,"<GR><p6.c-P6_SetAllTransRM>setting bit in transRM\n");
#endif
                BIT_SET(p6tmpPtr->trans_rm[i / 8], (i % 8));
            }

            p6tmpPtr->crc  = MSC_CRC32(p6tmpPtr, sizeof(P6ST) - sizeof(p6tmpPtr->crc));
#if GR_GEORAID15_DEBUG
            fprintf(stderr,"<GR><p6.c-P6_SetAllTransRM>calling P6_WriteToNVRAM\n");
#endif
            P6_WriteToNVRAM(P6stPtr, p6tmpPtr, sizeof(P6ST));
        }
        else
        {
#if GR_GEORAID15_DEBUG
            fprintf(stderr,"<GR><p6.c-P6_SetAllTransRM>P6tmpPtr is NULL\n");
#endif
            ++p6err_nullptr;
        }
    }
    else
    {
#if GR_GEORAID15_DEBUG
        fprintf(stderr,"<GR><p6.c-P6_SetAllTransRM>P6stPtr is  NULL\n");
#endif
        /*
        ** bump null pointer count
        */
        ++p6err_nullptr;
    }
}

/**
******************************************************************************
**
**  @brief      To provide a common clearing all regions in the
**              region map of the p6 copy state record.
**
**  @param      cor     -   COR pointer
**
**  @return     none
**
******************************************************************************
**/
void P6_ClrAllMainRM(UINT32 w UNUSED, UINT32 x UNUSED, UINT32 y UNUSED, COR* cor)
{
    P6ST*       P6stPtr;            /* tempory pointer to p6 record area    */
    P6ST*       p6tmpPtr;           /* tempory pointer to default p6 record */
    UINT32      i;                  /* index                                */

    P6stPtr = cor->stnvram;
    if (P6stPtr != NULL)
    {
        p6tmpPtr = P6stPtr->strcmirror;
        if (p6tmpPtr != NULL)
        {
            for (i = 0; i < (MAXRMCNT / 8); ++i)
            {
                p6tmpPtr->rm[i] = 0;
            }

            p6tmpPtr->crc  = MSC_CRC32(p6tmpPtr, sizeof(P6ST) - sizeof(p6tmpPtr->crc));
            P6_WriteToNVRAM(P6stPtr, p6tmpPtr, sizeof(P6ST));
        }
        else
        {
            ++p6err_nullptr;
        }
    }
    else
    {
        /*
        ** bump null pointer count
        */
        ++p6err_nullptr;
    }
}

/**
******************************************************************************
**
**  @brief      To provide a common clearing all regions in the
**              transfer region map of the p6 copy state record.
**
**  @param      cor     -   COR pointer
**
**  @return     none
**
******************************************************************************
**/
void P6_ClrAllTransRM(UINT32 w UNUSED, UINT32 x UNUSED, UINT32 y UNUSED, COR* cor)
{
    P6ST*       P6stPtr;            /* tempory pointer to p6 record area    */
    P6ST*       p6tmpPtr;           /* tempory pointer to default p6 record */
    UINT32      i;                  /* index                                */

    P6stPtr = cor->stnvram;
    if (P6stPtr != NULL)
    {
#if GR_GEORAID15_DEBUG
        fprintf(stderr, "<GR><p6.c- P6_ClrAllTransRM> P6stPtr is not null\n");
#endif
        p6tmpPtr = P6stPtr->strcmirror;
        if (p6tmpPtr != NULL)
        {
#if GR_GEORAID15_DEBUG
            fprintf(stderr, "<GR><p6.c-P6_ClrAllTransRM> p6tmpPtr is not null\n");
#endif

            for (i = 0; i < (MAXRMCNT / 8); ++i)
            {
                p6tmpPtr->trans_rm[i] = 0;
            }

            p6tmpPtr->crc  = MSC_CRC32(p6tmpPtr, sizeof(P6ST) - sizeof(p6tmpPtr->crc));
#if GR_GEORAID15_DEBUG
            fprintf(stderr, "<GR><p6.c-P6_ClrAllTransRM> Calling P6_WriteToNVRAM\n");
#endif
            P6_WriteToNVRAM(P6stPtr, p6tmpPtr, sizeof(P6ST));
        }
        else
        {
#if GR_GEORAID15_DEBUG
            fprintf(stderr, "<GR><p6.c-P6_ClrAllTransRM> p6tmpPtr is null\n");
#endif
            ++p6err_nullptr;
        }
    }
    else
    {
#if GR_GEORAID15_DEBUG
       fprintf(stderr, "<GR><p6.c-P6_ClrAllTransRM>  P6stPtr is null\n");
#endif
        /*
        ** bump null pointer count
        */
        ++p6err_nullptr;
    }
}
/**
******************************************************************************
**
**  @brief      To provide a common clearing all regions in the
**              transfer region map of the p6 copy state record.
**
**  @param      ptr     -   pointer to region bit map
**              cor     -   COR pointer
**
**  @return     none
**
******************************************************************************
**/
void P6_CopyTransRM(UINT8* rbm, UINT32 x UNUSED, UINT32 y UNUSED, COR* cor)
{
    P6ST*       P6stPtr;            /* tempory pointer to p6 record area    */
    P6ST*       p6tmpPtr;           /* tempory pointer to default p6 record */
    UINT32      i;                  /* index                                */

    if (rbm != NULL)
    {
        P6stPtr = cor->stnvram;
        if (P6stPtr != NULL)
        {
            p6tmpPtr = P6stPtr->strcmirror;
            if (p6tmpPtr != NULL)
            {

                for (i = 0; i < (MAXRMCNT / 8); ++i)
                {
                    p6tmpPtr->trans_rm[i] = rbm[i];
                }

                p6tmpPtr->crc  = MSC_CRC32(p6tmpPtr, sizeof(P6ST) - sizeof(p6tmpPtr->crc));
                P6_WriteToNVRAM(P6stPtr, p6tmpPtr, sizeof(P6ST));
            }
            else
            {
            /*
            ** bump null pointer count
            */
                ++p6err_nullptr;
            }
        }
        else
        {
            /*
            ** bump null pointer count
            */
            ++p6err_nullptr;
        }
    }
    else
    {
        /*
        ** bump null pointer count
        */
        ++p6err_nullptr;
    }
}
/**
******************************************************************************
**
**  @brief      To provide a common means of syncchronizing the main region
**              bit map to the main region map.
**
**  @param      cor     -   COR pointer
**
**  @return     none
**
******************************************************************************
**/
void P6_SyncMainRM(COR* cor)
{
    RM*         mainRM;             /* pointer to main region map           */
    UINT32      i;                  /* index                                */
    UINT32      numReg;             /* number of regions                    */

#if GR_GEORAID15_DEBUG
    fprintf(stderr,"<GR><p6.c- P6_SyncMainRM> Entering P6_SyncMainRM\n");
#endif

    mainRM  = cor->rmaptbl;

    numReg = (cor->totalsegs / REGSIZE_SEG) + ((cor->totalsegs % REGSIZE_SEG) ? 1 : 0);

    if (mainRM != NULL)
    {
#if GR_GEORAID15_DEBUG
        fprintf(stderr,"<GR><p6.c- P6_SyncMainRM> mainRM is not null\n");
#endif

        for (i = 0; i < numReg; ++i)
        {
            if (mainRM->regions[i] != NULL)
            {
#if GR_GEORAID15_DEBUG
                fprintf(stderr,"<GR><p6.c- P6_SyncMainRM> calling P6_SetMainRM\n");
#endif
                P6_SetMainRM( i, 0, 0, cor);
            }
            else
            {
#if GR_GEORAID15_DEBUG
                fprintf(stderr,"<GR><p6.c- P6_SyncMainRM> calling P6_ClrMainRM\n");
#endif
                P6_ClrMainRM( i, 0, 0, cor);
            }
        }
    }
}


/**
******************************************************************************
**
**  @brief      To provide a common mergeing the transfer and main
**              region maps.
**
**  @param      cor     -   COR pointer
**
**  @return     none
**
******************************************************************************
**/
void P6_MergeRM(UINT32 w UNUSED, UINT32 x UNUSED, UINT32 y UNUSED, COR* cor)
{
    RM*         mainRM;             /* pointer to main region map           */
    RM*         transRM;            /* pointer to transfer region map       */

    SM*         mainSM;             /* pointer to main segment map          */
    SM*         transSM;            /* pointer to transfer segment map      */
    P6ST*       p6stPtr;            /* tempory pointer to p6 record area    */

    UINT32      i,j;                /* indexs                               */
    UINT32      numReg;             /* number of regions                    */

    mainRM  = cor->rmaptbl;
    transRM = cor->transrmap;
    p6stPtr = cor->stnvram;
    numReg = (cor->totalsegs / REGSIZE_SEG) + ((cor->totalsegs % REGSIZE_SEG) ? 1 : 0);
#if GR_GEORAID15_DEBUG
    fprintf(stderr,"<GR><p6.c- MergeRM> number of regions = %u\n",numReg);
#endif

    if (mainRM != NULL)
    {
#if GR_GEORAID15_DEBUG
        fprintf(stderr,"<GR><p6.c- MergeRM> mainRM is existing\n");
#endif
        if (transRM != NULL)
        {
#if GR_GEORAID15_DEBUG
            fprintf(stderr,"<GR><p6.c- MergeRM> transRM is existing\n");
#endif
            for (i = 0; i < numReg; ++i)
            {
                if (mainRM->regions[i] != NULL)
                {
#if GR_GEORAID15_DEBUG
                    fprintf(stderr,"<GR><p6.c- MergeRM> mainRM-regions is not null \n");
#endif
                    /*
                    **  There is a main segment map associated with the current region.
                    */
                    mainSM = mainRM->regions[i];

                    if (transRM->regions[i] != NULL)
                    {
#if GR_GEORAID15_DEBUG
                        fprintf(stderr,"<GR><p6.c- MergeRM> transRM-regions is not null \n");
#endif
                        transSM = transRM->regions[i];
                        /*
                        **  There is a transfer segment map associated with the
                        **  current region. Merge the two maps together.
                        */
                        for (j = 0; j < ((REGSIZE_SEG >> SMBITS2WRD_SF)); j += sizeof(UINT64))
                        {
                            *(UINT64 *)&mainSM->segments[j] |= *(UINT64 *)&transSM->segments[j];
                        }
#ifdef M4_DEBUG_SM
fprintf(stderr, "%s%s:%u put_sm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)transSM);
#endif /* M4_DEBUG_SM */
                        put_sm(transSM);
                        transRM->regions[i] = NULL;
                        CM_cnt_smap(mainSM);
                    }
                    else
                    {
#if GR_GEORAID15_DEBUG
                        fprintf(stderr,"<GR><p6.c- MergeRM> transRM-regions is null \n");
#endif
                        /*
                        **  There is no transfer segment map associated with the
                        **  current region. Determine if the associated region
                        **  bit is set in the Transfer Region Bit Map. If it is,
                        **  set ALL bits in the main segment map.
                        */
                        if (p6stPtr != NULL)
                        {
#if GR_GEORAID15_DEBUG
                            fprintf(stderr,"<GR><p6.c- MergeRM> p6stPtr is not null \n");
#endif
                            if (BIT_TEST(p6stPtr->trans_rm[i / 8], (i % 8)))
                            {
#if GR_GEORAID15_DEBUG
                                fprintf(stderr,"<GR><p6.c- MergeRM> Dirty the region = %x\n",i);
#endif
                                P6_DirtyRegion(i, cor);
                            }
                        }
                    }
                }
                else
                {
#if GR_GEORAID15_DEBUG
                    fprintf(stderr,"<GR><p6.c- MergeRM> mainRM-regions is null \n");
#endif
                    /*
                    **  There is no main segment map associated with the current region.
                    */
                    if (transRM->regions[i] != NULL)
                    {
#if GR_GEORAID15_DEBUG
                        fprintf(stderr,"<GR><p6.c- MergeRM> transRM is not null\n");
#endif
                        /*
                        **  There is a transfer segment map associated with the
                        **  current region. Move the pointer from the transfer
                        **  region table to the main region table. Clear
                        **  accociated pointer in the transfer region table.
                        */
                        mainRM->regions[i] = transRM->regions[i];
                        transRM->regions[i] = NULL;
                    }
                    else
                    {
#if GR_GEORAID15_DEBUG
                        fprintf(stderr,"<GR><p6.c- MergeRM> transRM is null\n");
#endif
                        /*
                        **  There is no transfer segment map associated with the
                        **  current region. Determine if the associated region
                        **  bit is set in the Main or Transfer Region Bit Maps.
                        **  If either is set, allocate a Main segment map and
                        **  set ALL bits.
                        */
                        if (p6stPtr != NULL)
                        {
#if GR_GEORAID15_DEBUG
                            fprintf(stderr,"<GR><p6.c- MergeRM> p6stPtr not null\n");
#endif
                            if ((BIT_TEST(p6stPtr->trans_rm[i / 8], (i % 8)))  ||
                                (BIT_TEST(p6stPtr->rm[i / 8], (i % 8))))

                            {
#if GR_GEORAID15_DEBUG
                                fprintf(stderr,"<GR><p6.c- MergeRM> Allocate segment map\n");
#endif
                                mainRM->regions[i] = CM_setsmtbl();
                            }
                        }
                    }
                }
            }

            /*
            **  All regions have been merged. Release Transfer region table
            */
            if  (transRM != NULL)
            {
#if GR_GEORAID15_DEBUG
                fprintf(stderr,"<GR><p6.c- MergeRM> transRM is existing. All regions merged\n");
#endif
#ifdef M4_DEBUG_RM
fprintf(stderr, "%s%s:%u put_rm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)transRM);
#endif /* M4_DEBUG_RM */
                put_rm(transRM);
                cor->transrmap = NULL;
            }
        }
        else
        {
#if GR_GEORAID15_DEBUG
            fprintf(stderr,"<GR><p6.c- MergeRM> transRM is not existing\n");
#endif
        /*
        **  There is no Main or Transfer region maps. Check the main
        **  and transfer region bit maps to determine if any bits are
        **  set. If set, dirty that region.
        */
            for (i = 0; i < numReg; ++i)
            {
                if (BIT_TEST(p6stPtr->trans_rm[i / 8], (i % 8)))
                {
#if GR_GEORAID15_DEBUG
                    fprintf(stderr,"<GR><p6.c- MergeRM> Dirty the region = %u\n",i);
#endif
                    P6_DirtyRegion(i, cor);
                }
            }
        }
    }
    else
    {
#if GR_GEORAID15_DEBUG
        fprintf(stderr,"<GR><p6.c- MergeRM> mainRM is not existing\n");
#endif
        /*
        **  There is no main RM defined
        */
        if (transRM != NULL)
        {
#if GR_GEORAID15_DEBUG
            fprintf(stderr,"<GR><p6.c- MergeRM> transRM is existing\n");
#endif
            cor->rmaptbl   = transRM;
            cor->transrmap = NULL;
        }
        else
        {
#if GR_GEORAID15_DEBUG
            fprintf(stderr,"<GR><p6.c- MergeRM> transRM is not existing\n");
#endif
        /*
        **  There is no main or transfer region maps. Check the main
        **  and transfer region bit maps to determine if any bits are
        **  set. If set, dirty that region.
        */
            if ((cor->destvdd) && (GR_IsAllDevMissSyncFlagSet(cor->destvdd) == FALSE))
            {
#if GR_GEORAID15_DEBUG
                fprintf(stderr,"<GR><p6.c- MergeRM> dev miss sync flag is not set\n");
#endif
                for (i = 0; i < numReg; ++i)
                {
                    if ((BIT_TEST(p6stPtr->rm[i / 8], (i % 8)))   ||
                        (BIT_TEST(p6stPtr->trans_rm[i / 8], (i % 8))))
                    {
#if GR_GEORAID15_DEBUG
                        fprintf(stderr,"<GR><p6.c- MergeRM> Dirty the region = %u\n",i);
#endif
                        P6_DirtyRegion(i, cor);
                    }
                }
            }
            else
            {
#if 1 /*GR_GEORAID15_DEBUG */
                if(cor->srcvdd && cor->destvdd)
                {
                   fprintf(stderr,"<GR><p6.c-MergeRM>devmiss sync bit set-svid=%x dvid=%x-bypass dirty region call\n",
cor->srcvdd->vid,cor->destvdd->vid);
                }
#endif
            }
        }
    }
    /*
    **  deallocate the transRM, clear out the transfer region
    **  bit map, and set the appropriate bits in the main
    **  region bit map.
    */
    CM_dealloc_transRM(cor);
    P6_SyncMainRM(cor);
}

/**
******************************************************************************
**
**  @brief      To provide a common means of allocating a state record
**              in the P6 NVRAM area.
**
**  @param      cor - COR pointer
**
**  @return     Pointer to P6 state record
**
******************************************************************************
**/
P6ST* P6_AllocStRec(UINT32 w UNUSED, UINT32 x UNUSED, UINT32 y UNUSED, COR* cor)
{
    P6AR*       p6_header;          /* pointer to p6 header area            */
    P6ST*   P6stPtr = NULL;         /* pointer to P6 state record           */
    P6ST*   p6tmpPtr = NULL;        /* pointer to memory state record       */
    UINT32  i;                      /* index                                */

    cor->stnvram = NULL;

#ifdef LINUX_VER_NVP6_MM
    p6_header = (P6AR*) gNvramP6Image;
#else
    p6_header = (P6AR*) NVRAM_P6_START;
#endif

    for (P6stPtr = (P6ST*) ((UINT32) p6_header + sizeof(P6AR)), i = 0;
          i < MAX_CORS;
          ++P6stPtr, ++i)
    {
        if (P6stPtr->bnr == *(UINT32 *) "    ")
        {
            p6tmpPtr = s_MallocC(sizeof(P6ST), __FILE__, __LINE__);

            p6tmpPtr->strcmirror    = p6tmpPtr;
            p6tmpPtr->rid           = cor->rid;
            p6tmpPtr->cmsn          = cor->rcsn;
            p6tmpPtr->bnr           = *(UINT32 *) "P6st";
            p6tmpPtr->cfgrec_nbr    = i;

            p6tmpPtr->crc  = MSC_CRC32(p6tmpPtr, sizeof(P6ST) - sizeof(p6tmpPtr->crc));
            P6_WriteToNVRAM(P6stPtr, p6tmpPtr, sizeof(P6ST));

            cor->stnvram = P6stPtr;
            break;
        }
    }
    return(cor->stnvram);
}

/**
******************************************************************************
**
**  @brief      To provide a common deallocating a state record in the
**              the P6 NVRAM area.
**
**  @param      cor - COR pointer
**
**  @return     Pointer to P6 state record
**
******************************************************************************
**/
void P6_DeallocStRec(UINT32 w UNUSED, UINT32 x UNUSED, UINT32 y UNUSED, COR* cor)
{
    P6ST*   P6stPtr = NULL;         /* pointer to P6 state record           */
    P6ST*   p6tmpPtr = NULL;        /* pointer to memory state record       */

    P6stPtr = cor->stnvram;
    if (P6stPtr != NULL)
    {
        p6tmpPtr = P6stPtr->strcmirror;
        if (p6tmpPtr != NULL)
        {
            cor->stnvram = NULL;

            p6tmpPtr->bnr           = *(UINT32 *) "    ";
            p6tmpPtr->strcmirror    = NULL;

            p6tmpPtr->crc  = MSC_CRC32(p6tmpPtr, sizeof(P6ST) - sizeof(p6tmpPtr->crc));
            P6_WriteToNVRAM(P6stPtr, p6tmpPtr, sizeof(P6ST));

            cor->stnvram = NULL;

            s_Free(p6tmpPtr, sizeof(P6ST), __FILE__, __LINE__);
        }
    }
}

/**
******************************************************************************
**
**  @brief      To provide a common means of finding a state record in the
**              the P6 NVRAM area based on the RID and DSC serial numbers in
**              COR. If a record is found, a new mirror record is allocated
**              and the address of the P6 state record is returned.
**
**  @param      rid  - copy registration ID
**              rcsn - copy controller serial number
**
**  @return     Pointer to P6 state record
**
******************************************************************************
**/
P6ST* P6_FindStRec(COR* cor)
{
    P6AR*   p6_header;              /* pointer to p6 header area            */
    P6ST*   P6stPtr  = NULL;        /* pointer to P6 state record           */
    P6ST*   P6rtnPtr = NULL;        /* return pointer to P6 state record    */
    P6ST*   p6tmpPtr = NULL;        /* pointer to memory state record       */
    UINT32  i;                      /* index                                */

#ifdef LINUX_VER_NVP6_MM
    p6_header = (P6AR*) gNvramP6Image;
#else
    p6_header = (P6AR*) NVRAM_P6_START;
#endif

    for (P6stPtr = (P6ST*) ((UINT32) p6_header + sizeof(P6AR)), i = 0;
         i < MAX_CORS;
         ++P6stPtr, ++i)
    {
        if ((P6stPtr->bnr  == *(UINT32 *) "P6st")  &&
            (P6stPtr->rid  == cor->rid)            &&
            (P6stPtr->cmsn == cor->rcsn))
        {
            P6rtnPtr = P6stPtr;

            p6tmpPtr = s_MallocC(sizeof(P6ST), __FILE__, __LINE__);

            memcpy(p6tmpPtr, P6stPtr, sizeof(P6ST));

            p6tmpPtr->strcmirror    = p6tmpPtr;

            p6tmpPtr->crc  = MSC_CRC32(p6tmpPtr, sizeof(P6ST) - sizeof(p6tmpPtr->crc));
            P6_WriteToNVRAM(P6stPtr, p6tmpPtr, sizeof(P6ST));

            break;
        }
    }
    return(P6rtnPtr);
}
/**
******************************************************************************
**
**  @brief      To provide a common means of finding and removing inactive
**              state records in the P6 NVRAM area. This routine assumes that
**              a state record without a mirror pointer is no longer used and
**              therefor should be removed.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void P6_RemoveInactiveStRec()
{
    P6AR*   p6_header;              /* pointer to p6 header area            */
    P6ST*   P6stPtr  = NULL;        /* pointer to P6 state record           */
    P6ST*   p6tmpPtr = NULL;        /* pointer to memory state record       */
    UINT32  i;                      /* index                                */

#ifdef LINUX_VER_NVP6_MM
    p6_header = (P6AR*) gNvramP6Image;
#else
    p6_header = (P6AR*) NVRAM_P6_START;
#endif

    p6tmpPtr = s_MallocC(sizeof(P6ST), __FILE__, __LINE__);
    p6tmpPtr->bnr = *(UINT32 *) "    ";

    for (P6stPtr = (P6ST*) ((UINT32) p6_header + sizeof(P6AR)), i = 0;
         i < MAX_CORS;
         ++P6stPtr, ++i)
    {
        if ((P6stPtr->bnr  == *(UINT32 *) "P6st")  &&
            (P6stPtr->strcmirror  == NULL))
        {
            p6tmpPtr->cfgrec_nbr    = i;
            p6tmpPtr->crc  = MSC_CRC32(p6tmpPtr, sizeof(P6ST) - sizeof(p6tmpPtr->crc));
            P6_WriteToNVRAM(P6stPtr, p6tmpPtr, sizeof(P6ST));
        }
    }
    s_Free(p6tmpPtr, sizeof(P6ST), __FILE__, __LINE__);
}

/**
******************************************************************************
**
**  @brief      To provide a common means of saving the configuration of a
**              copy operation to NVRAM.
**
**  @param      g0 = start of NVRAM part 6
**
**  @return     none
**
******************************************************************************
**/
NVR* P6_SaveCpyCfg(NVR* nvrec)
{
    COR*    cor;            /* COR pointer to move through lists            */
    UINT32  i;              /* Temp variables                               */

    /* --- create and save a Default Manager Copy Record (DMCR) */
    nvrec->hdr.recLen  = sizeof(NVRH) + sizeof(NVDMCR);
    nvrec->hdr.recType = NRT_DMCR;
    nvrec->hdr.status  = 0;

    nvrec->u.dmcr.rid    = (UINT32) CM_cor_rid;
    nvrec->u.dmcr.cr_pri = (UINT8)  CM_proc_pri;
    nvrec->u.dmcr.pr_pri = (UINT8)  CM_cm_pri;

    for(i = 0; i < 10; nvrec->u.dmcr.rsvd06[i] = 0x00, i++);

    nvrec = (NVR*)((UINT32)nvrec + nvrec->hdr.recLen);

    /* determine if there are any CORs to save */
    for (cor = (COR*) CM_cor_act_que;
         cor != NULL;
         cor = cor->link)
    {
        P6_GenCpyCfgRcrd(nvrec, cor);

        nvrec = (NVR*)((UINT32)nvrec + nvrec->hdr.recLen);
    }
    return(nvrec);
}

/**
******************************************************************************
**
**  @brief      To provide a common means of constructing a copy configuration
**              record in the provided memory.
**
**  @param      p6_record = pointer to place copy record
**              cor       = pointer to COR structure
**
**  @return     none
**
******************************************************************************
**/
void P6_GenCpyCfgRcrd(NVR *nvrec, COR *cor)
{
    CM     *cm;             /* CM pointer to move through lists             */
    SCD    *scd;            /* SCD pointer to move through lists            */
    DCD    *dcd;            /* DCD pointer to move through lists            */
    VDD    *vdd;            /* VDD pointer to move through lists            */
    ILT    *rcc;            /* RCC pointer                                  */
    UINT32  i,j;            /* Temp variables                               */
    void (*pfv)(void);      /* pointer to void function                     */

    /* Save current configuration of COR */
    nvrec->hdr.recLen  = ((sizeof(NVRH) + sizeof(NVCOPY)));
    nvrec->hdr.recType = NRT_COPYCFG;
    nvrec->hdr.status  = 0;

    nvrec->u.copycfg.rid          = cor->rid;
    nvrec->u.copycfg.gid          = cor->gid;
    nvrec->u.copycfg.cr_crstate   = cor->crstate;
    nvrec->u.copycfg.cr_cstate    = cor->copystate;
    nvrec->u.copycfg.tsegs        = cor->totalsegs;
    nvrec->u.copycfg.powner       = cor->powner;
    nvrec->u.copycfg.sowner       = cor->sowner;

    memcpy(nvrec->u.copycfg.label, cor->label, 16);
    *(u_qword *)&nvrec->u.copycfg.rcsn  = *(u_qword *)&cor->rcsn;
    *(UINT32 *)&nvrec->u.copycfg.rscl   = *(UINT32 *)&cor->rscl;

    for ( i = 0; i < 12; nvrec->u.copycfg.rsvd34[i++] = 0);

    /* save current configuration of CM */
    cm = cor->cm;
    if(cm != NULL)
    {
        nvrec->u.copycfg.cm_type   = cm->copytype;
        nvrec->u.copycfg.cm_pri    = cm->priority;
        nvrec->u.copycfg.cm_mtype  = cm->mirrortype;
    }
    else
    {
        nvrec->u.copycfg.cm_type   = 0;
        nvrec->u.copycfg.cm_pri    = 0;
        nvrec->u.copycfg.cm_mtype  = 0;
    }

    /* save current configuration of SCD */
    i = j = 0;
    scd = cor->scd;
    if (scd != NULL)
    {
        pfv = scd->p1handler;
        if (pfv != NULL)
        {
            for (i = 1; (p6UpdhndTbl[i] != NULL) && (pfv != p6UpdhndTbl[i]); i++);
        }
        pfv = scd->p2handler;
        if (pfv != NULL)
        {
            for (j = 1; (p6UpdhndTbl[j] != NULL) && (pfv != p6UpdhndTbl[j]); j++);
        }
    }
    nvrec->u.copycfg.shidx = ((i << 4) | j);
    if (nvrec->u.copycfg.shidx)
    {
        nvrec->u.copycfg.stype = scd->type;
        vdd = scd->vdd;
        if (vdd != NULL)
        {
            nvrec->u.copycfg.svid = vdd->vid;
        }
        else
        {
            nvrec->u.copycfg.svid = 0xffff;
        }
    }
    else
    {
        nvrec->u.copycfg.svid  = 0xffff;
        nvrec->u.copycfg.stype = 0;
    }

    /* save current configuration of DCD */
    i = j = 0;
    dcd = cor->dcd;
    if (dcd != NULL)
    {
        pfv = dcd->p1handler;
        if (pfv != NULL)
        {
            for (i=1; (p6UpdhndTbl[i] != NULL) && (pfv != p6UpdhndTbl[i]); i++);
        }
        pfv = dcd->p2handler;
        if (pfv != NULL)
        {
            for (j=1; (p6UpdhndTbl[j] != NULL) && (pfv != p6UpdhndTbl[j]); j++);
        }
    }
    nvrec->u.copycfg.dhidx = ((i << 4) | j);
    if (nvrec->u.copycfg.dhidx)
    {
        nvrec->u.copycfg.dtype = dcd->type;
        vdd = dcd->vdd;
        if (vdd != NULL)
        {
            nvrec->u.copycfg.dvid = vdd->vid;
        }
        else
        {
            nvrec->u.copycfg.dvid = 0xffff;
        }
    }
    else
    {
        nvrec->u.copycfg.dvid  = 0xffff;
        nvrec->u.copycfg.dtype = 0;
    }

    /*
    **  --------------------------------------------------------------
    **  --- determine if there is a outstanding RCC and if there is
    **      save it's values.
    */

    rcc = cor->rcc;
    if  (rcc != NULL)
    {
        nvrec->u.copycfg.nssn = (rcc + 1)->RCC_G1;
        nvrec->u.copycfg.ndsn = (rcc + 1)->RCC_G2;
        nvrec->u.copycfg.cssn = (rcc + 1)->RCC_G4;
        nvrec->u.copycfg.cdsn = (rcc + 1)->RCC_G5;
    }
    else
    {
        nvrec->u.copycfg.nssn = 0;
        nvrec->u.copycfg.ndsn = 0;
        nvrec->u.copycfg.cssn = 0;
        nvrec->u.copycfg.cdsn = 0;
    }
}

/**
******************************************************************************
**
**  @brief      P6_DirtyRegion
**
**              To provide a common means of setting the appropriate number ot
**              segment bit in a region.
**
**  @param      region - the region to make dirty
**  @param      COR    - cor address
**
**  @return     none
**
******************************************************************************
**/
void P6_DirtyRegion( UINT32 region ,COR* cor )
{
    RM*     rm;                         /* pointer to region map            */
    SM*     sm;                         /* pointer to segment map           */
    UINT32  i = 0;                      /* Temp variables                   */
    UINT32  total_segments;
    UINT32  region_starting_segment;
    UINT32  region_ending_segment;
    UINT32  segments_to_set;
    UINT32  tempword = 0;               /* temporary  working word          */
    UINT8   bit = 31;                   /* bit loop count                   */
    u_qword allsetquad = {-1, -1, -1, -1}; /* quadword fill                 */

    /*
    **  determine if request region is within range
    */
    if (region <= MAXRMCNT)
    {

        /*
        **  determine if a RM is allocated. If not, allocate one
        */
        rm = cor->rmaptbl;
        if (rm == NULL)
        {
            rm = get_rm();
#ifdef M4_DEBUG_RM
fprintf(stderr, "%s%s:%u get_rm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)rm);
#endif /* M4_DEBUG_RM */
            cor->rmaptbl = rm;
        }

        /*
        **  determine if a full segment map is required
        */
        total_segments = cor->totalsegs;
        region_starting_segment = region << (SMWRD2REG_SF+SMBITS2WRD_SF);
        region_ending_segment   = region_starting_segment + (REGSIZE_SEG-1);
#if GR_GEORAID15_DEBUG
        fprintf(stderr,"<GR>p6.c- DirtyRegion> totalsegs = %x start-seg=%x, end_segment=%x\n",
                total_segments, region_starting_segment, region_ending_segment);
#endif

        if (region_ending_segment >= total_segments)
        {
            segments_to_set = total_segments - region_starting_segment;

            /*
            **  partial segment map required
            **
            **       - allocate clean segment map
            */
            sm = rm->regions[region];
            if (sm == NULL)
            {
                sm = get_sm();
#ifdef M4_DEBUG_SM
fprintf(stderr, "%s%s:%u get_sm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)sm);
#endif /* M4_DEBUG_SM */
                rm->regions[region] = sm;
            }
            sm->cnt = segments_to_set;

            /*
            **  fill in the quad portions of the segment map
            */
            if (segments_to_set != 0)
            {
                i = 0;
                while (segments_to_set >= (sizeof(u_qword) * 8))
                {
                    *(u_qword *)&sm->segments[i] = allsetquad;
                    i += sizeof(UINT32);
                    segments_to_set -= (sizeof(u_qword) * 8);
                }
            }

            /*
            **  setup remaining portion of segment map
            */
            if (segments_to_set != 0)
            {
                while (segments_to_set >= 32)
                {
                    sm->segments[i] = 0xffffffff;
                    ++i;
                    segments_to_set -= 32;
                }
                while (segments_to_set != 0)
                {
                    BIT_SET(tempword, bit);
                    --bit;
                    --segments_to_set;
                }
                sm->segments[i] = tempword;
            }
        }
        /*
        **  full segment map is required
        */
        else
        {
            if (rm->regions[region] == NULL)
            {
                sm = CM_setsmtbl();
                rm->regions[region] = sm;
            }
            else
            {
                sm = rm->regions[region];

                for (i = 0; i < ((REGSIZE_SEG >> SMBITS2WRD_SF) / 4); i += 4)
                {
                    *(u_qword *)&sm->segments[i] = allsetquad;
                }
            }
        }
    }
}

/**
******************************************************************************
**
**  @brief      Write the P6 NVRAM information out to the all drive.
**
**              This routine allocates a buffer and copies the P6 NVRAM into.
**              It the builds up a physical request packet to write the NVRAM
**              data to all drives.
**
**  @param      none
**
**  @return     Return code  0 = success
**                          -1 = failure
**
******************************************************************************
**/
UINT32 P6_save2fs(void)
{
    P6AR* p6_header;             /* pointer to the P6 header             */
    P6AR* p6Ptr;                 /* tempory pointer to p6 header area    */
    UINT32   rc = 0;                /* return code  (Default = success)     */
    UINT32   GoodCount = 0;         /* good write count                     */
    UINT32   ErrCount = 0;          /* write error count                    */

    /*
    **  Determine if there is a P6 area
    */
#ifdef LINUX_VER_NVP6_MM
    p6_header =  (P6AR*) gNvramP6Image;
#else
    p6_header =  (P6AR*) NVRAM_P6_START;
#endif
    if (p6_header != NULL)
    {
        /*
        **  Allocate a buffer and copy NVRAM PART 6 in to it....
        */
        p6Ptr = (P6AR*) s_MallocC(NVRAM_P6_SIZE_SECTORS * BYTES_PER_SECTOR, __FILE__, __LINE__);
        memcpy(p6Ptr, p6_header, sizeof(P6AR) + (sizeof(P6ST) * MAX_CORS));

        /*
        **  Write p6 area to all drives
        */
        rc = FS_MultiWrite(FID_COPY ,p6Ptr ,NVRAM_P6_SIZE_SECTORS ,(UINT32)NULL ,1 ,&GoodCount ,&ErrCount);

        /*
        **  Release the allocated buffer
        */
        s_Free(p6Ptr, NVRAM_P6_SIZE_SECTORS * BYTES_PER_SECTOR, __FILE__, __LINE__);
    }
    return (rc);
}

/**
******************************************************************************
**
**
**  @brief      Provide a means of saving changes of the copy configuration
**
**              This routine routine determines if the current node is a slave
**              or a master controller. If it is a slave, the changed copy
**              configuration record is sent to the master controller. If it
**              is a master, the configuration is saved with a p2update call.
**
**  @param      cor = pointer to COR structure
**
**  @return     none
**
******************************************************************************
**/
void P6_Update_Config(COR* cor UNUSED)
{
    if (BIT_TEST(K_ii.status, II_MASTER))
    {
        NV_P2UpdateConfig();
    }
}

/**
******************************************************************************
**
**  @brief      P6_WriteToNVRAM
**
**              This routine provides a means of writing a buffer to NVRAM
**
**  @param      dst = NVRAM destination pointer
**              src = copy source pointer
**              length = amount of data (in bytes) to copy
**
**  @return     none
**
******************************************************************************
**/
void P6_WriteToNVRAM( void* dst, void* src, UINT32 length)
{
    UINT8*  d = (UINT8 *) dst;
    UINT8*  s = (UINT8 *) src;
    UINT32  i;
#ifdef LINUX_VER_NVP6_MM
    UINT32  mmDestAddr;
    UINT32  localAddrStart = (UINT32)gNvramP6Image;
#endif

    /*
    ** Now physically write the NVRAM image to the NVRAM. Then the data must
    ** be copied a byte at a time into the NVRAM. This data path is only a
    ** byte wide, so it cannot be done any differently.
    */

    for (i = 0;
         i < length;
         d[i] = s[i], i++);

#ifdef LINUX_VER_NVP6_MM
    /*
    ** Copied to the  local memory(of NVRAM p6)
    ** Now write into the Micro Memory Card in Part - 6
    */
    mmDestAddr = ((UINT32)d - localAddrStart+(UINT32)MICRO_MEM_BE_P6_START);
#ifdef DEBUG_P6MM
    fprintf(stderr,"<<P6_WriteToNVRAM>> %d bytes written at Addr(%x)of MM\n",length, mmDestAddr);
#endif /*DEBUG_P6MM*/
    MM_Write(mmDestAddr,s, length);
#endif

/*      msync((void*)d, length, MS_SYNC); */
#ifndef LINUX_VER_NVP6_MM
       /*
       ** MSYNC is valid only when writing to file from shared memory
       ** For micro memory card it is not needed
       */
  {
        UINT32 rc;
        UINT32 pageAdjust = ((UINT32)dst % getpagesize());
        /*
        ** Adjust the input to align with a page boundary
        */
        rc = msync((UINT8*)(d - pageAdjust), (size_t)(length + pageAdjust), MS_SYNC);
        if (rc)
        {
            int save_errno = errno;

            fprintf(stderr, "msync() addr = 0x%08x, length = 0x%08x\n",
                                                  (UINT32)d, length);
            fprintf(stderr, "msync() adjusted addr = 0x%08x, length = 0x%08x\n",
                            (UINT32)d-pageAdjust, length+pageAdjust);
            fprintf(stderr, "@@@@@@ msync() returned an error = %s @@@@\n",strerror(save_errno));
            fprintf(stderr, "pagesize = 0x%08x\n", getpagesize());
        }
  }
#endif /* LINUX_VER_NVP6_MM */
}

/**
******************************************************************************
**
**  @brief      P6_LockAllCor
**              Sets STATE and GENERAL locks in all COR structures.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void P6_LockAllCor(void)
{
    COR    *cor = NULL;         /* COR pointer                              */

    /* Set all COR Locks */
    for (cor = CM_cor_act_que; cor != NULL; cor = cor->link)
    {
        BIT_SET(cor->flags, CFLG_B_DIS_STATE);
        BIT_SET(cor->flags, CFLG_B_DIS_GENERAL);
        // BIT_SET(cor->flags, CFLG_B_DIS_DEVICE);
    }
}

/**
******************************************************************************
**
**  @brief      P6_LockAllCor
**              Clears STATE, GENERAL, and DEVICE locks in all COR structures.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void P6_UnlockAllCor(void)
{
    COR    *cor = NULL;         /* COR pointer                              */

    /* Clear all COR Locks */
    for (cor = CM_cor_act_que; cor != NULL; cor = cor->link)
    {
        BIT_CLEAR(cor->flags, CFLG_B_DIS_STATE);
        BIT_CLEAR(cor->flags, CFLG_B_DIS_GENERAL);
        BIT_CLEAR(cor->flags, CFLG_B_DIS_DEVICE);
    }
}

/**
******************************************************************************
**
**  @brief      P6_RstCpyCfg
**
**              To provide a common means of restoring the configuration of a
**              copy operation from NVRAM
**
**  @param      nvrec - pointer configuration record
**
**  @return     returncode
**
******************************************************************************
**/
NVR* P6_RstCpyCfg(NVR* nvrec)
{
    COR*    cor = NULL;         /* COR pointer                              */
    CM*     cm  = NULL;         /* CM pointer                               */
    DCD*    dcd = NULL;         /* pointer to dcd                           */
    SCD*    scd = NULL;         /* pointer to scd                           */
    PCB*    pcb = NULL;         /* PCB pointer                              */
    P6ST*   p6tmpPtr = NULL;    /* tempory pointer to default p6 record     */
    NVR*    preNvrec = NULL;    /* Previous NVRAM record                    */
    UINT32  rc = PASS;          /* return code (default = FAIL)             */

    /*
    **  Set all CORs inactive
    */
    for (cor = CM_cor_act_que; cor != NULL; cor = cor->link)
    {
        BIT_SET(cor->flags, CFLG_B_INACTIVE);
    }

    for(preNvrec = nvrec;;
        preNvrec = nvrec, nvrec = (NVR*)((UINT32)nvrec + nvrec->hdr.recLen))
    {
        if (nvrec->hdr.recType == NRT_DMCR)
        {
            CM_cor_rid  = nvrec->u.dmcr.rid;
            CM_proc_pri = nvrec->u.dmcr.cr_pri;
            CM_cm_pri   = nvrec->u.dmcr.pr_pri;
        }
        else if (nvrec->hdr.recType == NRT_COPYCFG)
        {
            /*
            **  Determine if a COR already is defined for this copy
            **  ---------------------------------------------------
            */
            for (cor = CM_cor_act_que; cor != NULL; cor = cor->link)
            {
                if ((cor->rid  == nvrec->u.copycfg.rid) &&
                    (cor->rcsn == nvrec->u.copycfg.rcsn))
                {
                    break;
                }
            }

            /*
            ** if a COR does not exist for this copy. build one.
            */
            if (cor == NULL)
            {
                /*
                ** determine if the svid and dvid are valid
                */
#if GR_GEORAID15_DEBUG
                fprintf(stderr,"<GR><P6_RstCpyCfg-p6.c>COR is not existing..creating COR....\n");
#endif
                rc = p6_Validate_Vaddr(nvrec);
                if (rc == PASS)
                {
                    /*
                    **  Determine if this is a local or remote copy coperation
                    */
                    if (K_ficb->vcgID == nvrec->u.copycfg.rcsn)
                    {
#if GR_GEORAID15_DEBUG
                        fprintf(stderr,"<GR><P6_RstCpyCfg>local copy create/build COR and CM- svid=%x crstate=%x\n",
                               nvrec->u.copycfg.svid,nvrec->u.copycfg.cr_crstate );
#endif
                        /*
                        ** Create local copy structures
                        */
                        cor = P6_Build_Base_Copy(nvrec);
                        cm  = p6_Build_CM(nvrec, cor);
#if GR_GEORAID15_DEBUG
                        fprintf(stderr,"<GR><P6_RstCpyCfg-p6.c>cstate=%u,crstate=%u\n",
                                (UINT32)(cor->copystate),(UINT32)(cor->crstate));
#endif
                        cor->stnvram = P6_FindStRec(cor);

                        scd = cor->scd;
                        dcd = cor->dcd;

                        if (cor->stnvram != NULL)
                        {
#if GR_GEORAID15_DEBUG
                            fprintf(stderr,"<GR><P6_RstCpyCfg-p6.c>stnvram is existing\n");
#endif
                            if (scd != NULL)
                            {
#if GR_GEORAID15_DEBUG
                                fprintf(stderr,"<GR><P6_RstCpyCfg-p6.c>scd exists cor_crstate=%x\n",
                                        cor->crstate);
#endif
                                switch(cor->crstate)
                                {
                                    case(CRST_UNDEF):
                                    case(CRST_INIT):
                                        scd->p2handler = p6UpdhndTbl[P6_UH_NULL];
                                        break;
                                    case(CRST_ACTIVE):
                                    case(CRST_AUTOSUSP):
                                    case(CRST_USERSUSP):
                                    case(CRST_REMSUSP):
                                        scd->p2handler = p6UpdhndTbl[P6_UH_SUSPEND];
                                    default:
                                        break;
                                }
                            }

                            if (dcd != NULL)
                            {
#if GR_GEORAID15_DEBUG
                                fprintf(stderr,"<GR><P6_RstCpyCfg-p6.c>dcd exists cor_crstate=%x\n",
                                        cor->crstate);
#endif
                                switch(cor->crstate)
                                {
                                    case(CRST_UNDEF):
                                    case(CRST_INIT):
                                        dcd->p2handler = p6UpdhndTbl[P6_UH_NULL];
                                        break;
                                    case(CRST_ACTIVE):
                                    case(CRST_AUTOSUSP):
                                    case(CRST_USERSUSP):
                                    case(CRST_REMSUSP):
                                        dcd->p2handler = p6UpdhndTbl[P6_UH_SUSPEND];
                                    default:
                                        break;
                                }
                            }
                        }

                        /*
                        ** Activate the known components of the copy
                        */
                        CM_act_cor(cor);
                        CM_act_cm(cm);
                        if (scd != NULL)
                        {
                            CM_act_scd(scd);
                        }
                        if (dcd != NULL)
                        {
                            CM_act_dcd(dcd);
                        }

                        /*
                        **  Fork and start the copy task
                        */
                        CT_fork_tmp = (unsigned long)"CM_pexec";
                        pcb = TaskCreate3(&CM_pexec, CM_proc_pri, (UINT32)cm);
                        cm->pcb = pcb;
                        /* Stop the copy task before any taskswitches might allow it to start. */
                        TaskSetState(pcb, PCB_NOT_READY);
#if GR_GEORAID15_DEBUG
                        fprintf(stderr,"<GR><P6_RstCpyCfg-p6.c>call CCSM_start_copy().svid=%x dvid=%x\n",
                         (UINT32)(cor->srcvdd->vid), (UINT32)(cor->destvdd->vid));
#endif
                        CCSM_start_copy(cor);
                    }
                    else
                    {
                        /*
                        ** Create remote copy structures
                        */
                        cor = P6_Build_Base_Copy(nvrec);

                        p6tmpPtr = P6_FindStRec(cor);
                        if (p6tmpPtr == NULL)
                        {
                            p6tmpPtr = P6_AllocStRec(0, 0, 0, cor);
                        }
                        cor->stnvram = p6tmpPtr;

                        /*
                        ** Activate the known components of the copy
                        */
                        if (p6tmpPtr != NULL)
                        {
                            CM_act_cor(cor);
                        }
                        if (cor->scd != NULL)
                        {
                            CM_act_scd(cor->scd);
                        }
                        if (cor->dcd != NULL)
                        {
                            CM_act_dcd(cor->dcd);
                            CM_mmc_sflag(cor);
                        }

                        /*
                        **  Start the copy task
                        */
                        CCSM_start_copy(cor);
                    }
                }
            }
            else
            {
#if GR_GEORAID15_DEBUG
                fprintf(stderr,"<GR><P6_RstCpyCfg>COR exists..srcvid=%x destvid=%x....\n",
                         (cor->srcvdd->vid), (cor->destvdd->vid));
#endif
                /*
                **  There is already a copy operation active. Determine if the
                **  copy operation is local or remote.
                */
                if (K_ficb->vcgID == nvrec->u.copycfg.rcsn)
                {
#if GR_GEORAID15_DEBUG
                    fprintf(stderr,"<GR><P6_RstCpyCfg>COR- local copy svid=%x dvid=%x.\n",
                            (cor->srcvdd->vid), (cor->destvdd->vid));
#endif
                    /*
                    ** Refresh local copy structures
                    */
                    P6_Refresh_Local_Base_Copy(nvrec, cor);
#if GR_GEORAID15_DEBUG
                    if(cor->srcvdd)
                    {
                        fprintf(stderr,"<GR><P6_RstCpyCfg>cstate=%x,crstate=%x  crstate(nvr) =%x svid=%x\n",
                       (cor->copystate), (cor->crstate),(nvrec->u.copycfg.cr_crstate),(cor->srcvdd->vid));
                    }
#endif

                    /*
                    **  Determine if there has been a change in the COR
                    **  crstate. If there was a change, send indication
                    **  to CCSM.
                    */
                    if (nvrec->u.copycfg.cr_crstate != cor->crstate)
                    {
                        CCSM_cosc(nvrec->u.copycfg.cr_crstate, 0, 0, cor);
                    }
                }
                else
                {

                    /*
                    ** Refresh remote copy structures
                    */
                    P6_Refresh_Remote_Base_Copy(nvrec, cor);
                }
            }
        }
        else
        {
            break;
        }
    }

    /*
    **  Terminate any inactive CORs that are still inactive
    */
    for (cor = CM_cor_act_que; cor != NULL; cor = cor->link)
    {
        if (BIT_TEST(cor->flags, CFLG_B_INACTIVE))
        {
// NOTE: get_ilt can task switch, thus above loop is technically wrong.
            CCSM_term_copy(cor);
        }
    }
    /*
    **  Remove any inactive P6 entries
    */

    P6_RemoveInactiveStRec();

    /*
    **  Issue a Configuration Changed Occurred to CCSM
    */

    CCSM_cco();

    return(preNvrec);
}


/**
******************************************************************************
**
**  @brief      P6_Build_Base_Copy
**
**              This routine provides a means of building a base copy operation.
**              This base copy places all components of the copy in suspended
**              state. This routine expects that the requested copy does not
**              already have a COR.
**
**  @param      nvrec = pointer to configuration record describing copy
**
**  @return     cor = pointer to COR (NULL if no COR created)
**
******************************************************************************
**/
COR *P6_Build_Base_Copy(NVR *nvrec)
{
    COR    *cor = NULL;         /* pointer to cor                           */
    DCD    *dcd = NULL;         /* pointer to dcd                           */
    SCD    *scd = NULL;         /* pointer to scd                           */
    VDD    *vdd = NULL;         /* pointer to vdd                           */
    UINT16  vid;                /* vdisk ID                                 */
    UINT8   mstate = MST_TERM;  /* cor_mstate                               */

    /*
     *  Allocate and set up a COR
     *  -------------------------
     */
    cor = get_cor();
#ifdef M4_DEBUG_COR
fprintf(stderr, "%s%s:%d get_cor 0x%08x\n", "BE ", __FILE__, __LINE__, (UINT32)cor);
#endif /* M4_DEBUG_COR */

    if (cor != NULL)
    {
        cor->rid         = nvrec->u.copycfg.rid;
        cor->gid         = nvrec->u.copycfg.gid;
        cor->crstate     = nvrec->u.copycfg.cr_crstate;
        cor->copystate   = nvrec->u.copycfg.cr_cstate;
        cor->totalsegs   = nvrec->u.copycfg.tsegs;
#if GR_GEORAID15_DEBUG
        fprintf(stderr, "<GR-%s>svid=%d dvid=%d cstate=%d crstate=%d\n", __FUNCTION__, nvrec->u.copycfg.svid,
                nvrec->u.copycfg.dvid, cor->copystate, cor->crstate);
#endif  /* GR_GEORAID15_DEBUG */
// if (cor->totalsegs > (REGSIZE_SEG * 256))
// {
//   cor->totalsegs = (REGSIZE_SEG * 256);
// }
        cor->powner      = nvrec->u.copycfg.powner;
        cor->sowner      = nvrec->u.copycfg.sowner;

        memcpy(cor->label, nvrec->u.copycfg.label, 16);
        *(u_qword *)&cor->rcsn  = *(u_qword *)&nvrec->u.copycfg.rcsn;
        *(UINT32 *)&cor->rscl   = *(UINT32 *)&nvrec->u.copycfg.rscl;

        /*
        **  Determine if a remote copy being initialized and setup
        **  cor->mstate value based on cor->crstate value.
        */

        if (K_ficb->vcgID != nvrec->u.copycfg.rcsn)
        {

            /*
            **  Set mirror state active if remote copy in any form of
            **  suspension.
            */

            if (cor->crstate == CRST_USERSUSP || cor->crstate == CRST_REMSUSP)
            {
                mstate = MST_ACT;
            }
        }

        cor->mirrorstate = mstate;

        /*
        **  Set up the OCstate and CCstate tables
        */
        cor->ocseptr    = &OCtbl;
        cor->ccseptr    = &CCtbl;

        cor->ocsecst    = 0;
        cor->ccsecst    = 0;
        cor->ostindx    = 0;
        cor->cstindx    = 0;

        /*
        **  Determine if a SCD is required and if so set it up.
        */
        if (nvrec->u.copycfg.shidx)
        {
            scd = get_scd();
#ifdef M4_DEBUG_SCD
fprintf(stderr, "%s%s:%d get_scd 0x%08x\n", "BE ", __FILE__, __LINE__, (UINT32)scd);
#endif /* M4_DEBUG_SCD */
            scd->cor = cor;
            cor->scd = scd;

            /*
            **  Determine if a local or remote copy being initialized.
            **  New local copies have the p2handler set to NULL.
            **  New remote copies have the p2handler based on the NVRAM index value.
            */
            if (K_ficb->vcgID == nvrec->u.copycfg.rcsn)
            {
                scd->p2handler = p6UpdhndTbl[P6_UH_NULL];
            }
            else
            {
                scd->p2handler = p6UpdhndTbl[nvrec->u.copycfg.shidx & 0x0f];
            }

            scd->p1handler = NULL;

            scd->type = nvrec->u.copycfg.stype;

            vid = nvrec->u.copycfg.svid;
            if (vid != 0xffff)
            {
                vdd = V_vddindx[vid];
#if GR_GEORAID15_DEBUG
                if (vdd && vdd->grInfo.vdOpState == GR_VD_IOSUSPEND)
                {
                    fprintf(stderr, "<GR>p6.c--WARNING..COR miss for the copy of svid=%x...\n", vdd->vid);
                }
#endif  /* GR_GEORAID15_DEBUG */
                scd->vdd    = vdd;
                cor->srcvdd = vdd;
            }
        }

        /* Determine if a DCD is required and if so, set it up. */
        if (nvrec->u.copycfg.dhidx)
        {
            dcd = get_dcd();
#ifdef M4_DEBUG_DCD
fprintf(stderr, "%s%s:%d get_dcd 0x%08x\n", "BE ", __FILE__, __LINE__, (UINT32)dcd);
#endif /* M4_DEBUG_DCD */
            dcd->cor = cor;
            cor->dcd = dcd;

            /*
            **  Determine if a local or remote copy being initialized.
            **  New local copies have the p2handler set to NULL.
            **  New remote copies have the p2handler based on the NVRAM index value.
            */
            if (K_ficb->vcgID == nvrec->u.copycfg.rcsn)
            {
                dcd->p2handler = p6UpdhndTbl[P6_UH_NULL];
            }
            else
            {
                dcd->p2handler = p6UpdhndTbl[nvrec->u.copycfg.dhidx & 0x0f];
            }

            dcd->p1handler = NULL;

            dcd->type = nvrec->u.copycfg.dtype;

            vid = nvrec->u.copycfg.dvid;
            if (vid != 0xffff)
            {
                vdd = V_vddindx[vid];
#if GR_GEORAID15_DEBUG
                if (vdd) fprintf(stderr,"<GR>Build_base_copy p6.c>dvdd is existing\n");
#endif
                dcd->vdd     = vdd;
                cor->destvdd = vdd;

                /* Update the source vid field in the destination device VDD. */
                if ((scd != NULL)       &&
                    (scd->vdd != NULL))
                {
                    vdd->scorVID = scd->vdd->vid;
                }
                else
                {
                    vdd->scorVID = 0xffff;
                }
                BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            }
        }

#if GR_GEORAID15_DEBUG
    fprintf(stderr,"<GR>P6_Build_BaseCopy.svid=%x dvid=%x....\n",
            (UINT32)(cor->srcvdd->vid), (UINT32)(cor->destvdd->vid));
#endif
    }
    return(cor);
}
/**
******************************************************************************
**
**  @brief      P6_Refresh_Local_Base_Copy
**
**              This routine provides a means of building a base copy operation.
**              This base copy places all components of the copy in suspended
**              state. This routine expects that the requested copy does not
**              already have a COR.
**
**  @param      nvrec = pointer to configuration record describing copy
**              cor   = pointer to COR
**
**  @return     none
**
******************************************************************************
**/
void P6_Refresh_Local_Base_Copy(NVR *nvrec, COR *cor)
{
    DCD    *dcd = NULL;         /* pointer to dcd                           */
    SCD    *scd = NULL;         /* pointer to scd                           */
    VDD    *vdd = NULL;         /* pointer to vdd                           */
    UINT32  i;                  /* indet var                                */
    UINT32 *corCheck;           /* pointer to word for checking cor         */
    UINT32 *NVCheck;            /* pointer to word for checking cor         */
    UINT32  check = TRUE;       /* cor inhibit refresh check word           */
    UINT32  vid;                /* vdisk ID                                 */

    /*
    **  set COR active
    */
    BIT_CLEAR(cor->flags, CFLG_B_INACTIVE);

    /*
    **  restore primary and secondary owners
    */
    cor->powner  = nvrec->u.copycfg.powner;
    cor->sowner  = nvrec->u.copycfg.sowner;

    /*
    **********************************************************************
    **  determine if the disable copy state update flag is set. If not,
    **  update copy state. Otherwise, determine if the disable copy state
    **  update flage should be cleared
    **********************************************************************
    */
    if ( !(BIT_TEST(cor->flags, CFLG_B_DIS_STATE)))
    {
        /*
        ******************************************************************
        ** ME-547:
        ** Do not update the current crstate of the COR with the crstate
        ** from the NV record if the crstate of the NV record specifies it
        ** as either user-pause and copy resume
        ******************************************************************
        */
        if ( (nvrec->u.copycfg.cr_crstate != CRST_USERSUSP) &&
             (nvrec->u.copycfg.cr_crstate != CRST_ACTIVE) )
        {
#if GR_GEORAID15_DEBUG
             fprintf(stderr, "<GR-%s> svid=%d dvid=%d cr_crstate(nvr)=%d cstate(nvr)=%d\n", __FUNCTION__, nvrec->u.copycfg.svid, nvrec->u.copycfg.dvid, nvrec->u.copycfg.cr_crstate, nvrec->u.copycfg.cr_cstate);
#endif  /* GR_GEORAID15_DEBUG */

             cor->crstate = nvrec->u.copycfg.cr_crstate;
        }
        cor->copystate = nvrec->u.copycfg.cr_cstate;

#if GR_GEORAID15_DEBUG
        fprintf(stderr, "<GR-%s>svid=%d dvid=%d cstate=%d crstate=%d\n", __FUNCTION__, nvrec->u.copycfg.svid,
                nvrec->u.copycfg.dvid, cor->copystate, cor->crstate);
#endif  /* GR_GEORAID15_DEBUG */
    }
    else
    {
        if ((cor->crstate == nvrec->u.copycfg.cr_crstate) &&
            (cor->copystate == nvrec->u.copycfg.cr_cstate))
        {
            BIT_CLEAR(cor->flags, CFLG_B_DIS_STATE);
        }
    }
    /*
    **********************************************************************
    **  Determine if the disable General information update flag is
    **  set. If not, update the general copy information. Otherwise,
    **  determine if the disable general copy information update flag
    **  should be cleared.
    **********************************************************************
    */
    if ( !(BIT_TEST(cor->flags, CFLG_B_DIS_GENERAL)))
    {
        memcpy(cor->label, nvrec->u.copycfg.label, 16);
        cor->gid = nvrec->u.copycfg.gid;
    }
    else
    {
        if (!((memcmp(cor->label, nvrec->u.copycfg.label, 16)) &&
            (cor->gid == nvrec->u.copycfg.gid)))
        {
            BIT_CLEAR(cor->flags, CFLG_B_DIS_GENERAL);
        }
    }

    /*
    **********************************************************************
    **  determine if the disable copy device update flag is set. If not,
    **  update copy device. Otherwise, determine if the disable copy device
    **  update flag should be cleared
    **********************************************************************
    */
    if ( !(BIT_TEST(cor->flags, CFLG_B_DIS_DEVICE)))
    {
        cor->copystate = nvrec->u.copycfg.cr_cstate;
#if GR_GEORAID15_DEBUG
        fprintf(stderr, "<GR-%s2>svid=%d dvid=%d cstate=%d crstate=%d\n", __FUNCTION__, nvrec->u.copycfg.svid,
                nvrec->u.copycfg.dvid, cor->copystate, cor->crstate);
#endif  /* GR_GEORAID15_DEBUG */

        /*
        **  reconfigure possible DCD
        */
        i = (UINT32)(nvrec->u.copycfg.dhidx);

        if (i)
        {
            dcd = cor->dcd;
            if (dcd == NULL)
            {
                /*
                **  A New DCD is required.
                */
                dcd = get_dcd();
#ifdef M4_DEBUG_DCD
fprintf(stderr, "%s%s:%d get_dcd 0x%08x\n", "BE ", __FILE__, __LINE__, (UINT32)dcd);
#endif /* M4_DEBUG_DCD */
                dcd->cor = cor;
                cor->dcd = dcd;

                if (cor->cm == NULL)
                {
                    dcd->p2handler = p6UpdhndTbl[P6_UH_NULL];
                    dcd->p1handler = NULL;
                }
                else
                {
                    dcd->p2handler = p6UpdhndTbl[P6_UH_SUSPEND];
                    dcd->p1handler = NULL;
                }

                dcd->type = nvrec->u.copycfg.dtype;

                if (nvrec->u.copycfg.dvid != 0xffff)
                {
                    vdd = V_vddindx[nvrec->u.copycfg.dvid];
                    dcd->vdd     = vdd;
                    cor->destvdd = vdd;
                }

                CM_act_dcd(dcd);
            }

            /*
            **  Update the DCD according to the configuration provided
            */

            if ( !(BIT_TEST(cor->flags, CFLG_B_DIS_DEVICE)))
            {
                if (nvrec->u.copycfg.dvid == 0xffff)
                {
                    cor->destvdd = NULL;
                    dcd->vdd     = NULL;
                    cor->rcdvd   = 0xff;
                    cor->rcdcl   = 0xff;
                }
                else
                {
                    if ((dcd->vdd     == NULL)   ||
                        (cor->destvdd == NULL))
                    {
                        vid = nvrec->u.copycfg.dvid;
                        vdd = V_vddindx[vid];
                        dcd->vdd     = vdd;
                        cor->destvdd = vdd;
                        cor->rcdvd   = (UINT8) vid;
                        cor->rcdcl   = (UINT8) (vid >> 8);

                        CM_act_dcd(dcd);
                        CM_mmc_sflag(cor);

                    }
                    else
                    {
                        vdd = cor->destvdd;
                    }
                }
            }

            /* Update the VDD mirror state */
            if (vdd != NULL)
            {
                switch(cor->crstate){
                    case(CRST_AUTOSUSP):
                        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                        vdd->mirror = VD_COPYAUTOPAUSE;
                        vdd->attr |= VD_SUSPEND;
#if GR_GEORAID15_DEBUG
                fprintf(stderr,"<GR>P6_Refresh_Local_Base_Copy>Set vd attrb/mirror=autosuspend vid=%x\n",
                        (UINT32)(vdd->vid));
#endif
                        /* DO We need to clear sync bit in VDD.... ?????    TBD.....(GR_GEORAID15) */
                        break;

                    case(CRST_USERSUSP):
                    case(CRST_REMSUSP):
                        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                        vdd->mirror = VD_COPYUSERPAUSE;
                        vdd->attr |= VD_SUSPEND;
#if GR_GEORAID15_DEBUG
                fprintf(stderr,"<GR>P6_Refresh_Local_Base_Copy>Set vd attrb/mirror=usersuspend vid=%x\n",
                                     (UINT32)(vdd->vid));
#endif
                        /* DO We need to clear sync bit in VDD.... ?????    TBD.....(GR_GEORAID15) */
                        break;

                    case(CRST_UNDEF):
                    case(CRST_INIT):
                    case(CRST_ACTIVE):
                        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                        vdd->mirror = (cor->rmaptbl != NULL) ? VD_COPYTO : VD_COPYMIRROR;
                        vdd->attr &= ~(VD_SUSPEND);

                        /* DO We need to set sync bit in VDD.... ?????    TBD.....(GR_GEORAID15) */
#if GR_GEORAID15_DEBUG
                fprintf(stderr,"<GR>P6_Refresh_Local_Base_Copy>clear suspndbit set mirror/copy vid=%x\n",
                                     (UINT32)(vdd->vid));
#endif

                    default:
                        break;
                }
            }
        }
        else
        {
            /* A dcd is no longer configured. remove it from the VDD structure */
            if (cor->dcd != NULL)
            {
                cor->dcd     = NULL;
                cor->destvdd = NULL;

                vid = (cor->rdcl << 8) | cor->rdvd;

                if (vid != 0xffff)
                {
                    vdd = V_vddindx[vid];

                    if (vdd == NULL)
                    {
                        dcd->vdd = NULL;
                    }
                }

                CM_deact_dcd(dcd);
#ifdef M4_DEBUG_DCD
fprintf(stderr, "%s%s:%d put_dcd 0x%08x\n", "BE ", __FILE__, __LINE__, (UINT32)dcd);
#endif /* M4_DEBUG_DCD */
                put_dcd(dcd);
            }
        }

        /*
        **  reconfigure possible SCD
        */
        i = (UINT32)(nvrec->u.copycfg.shidx);

        if (i)
        {
            scd = cor->scd;
            if (scd == NULL)
            {
                /*
                **  A New SCD is required.
                */
                scd = get_scd();
#ifdef M4_DEBUG_SCD
fprintf(stderr, "%s%s:%d get_scd 0x%08x\n", "BE ", __FILE__, __LINE__, (UINT32)scd);
#endif /* M4_DEBUG_SCD */
                scd->cor = cor;
                cor->scd = scd;

                if (cor->cm == NULL)
                {
                    scd->p2handler = p6UpdhndTbl[P6_UH_NULL];
                    scd->p1handler = NULL;
                }
                else
                {
                    scd->p2handler = p6UpdhndTbl[P6_UH_SUSPEND];
                    scd->p1handler = NULL;
                }

                scd->type = nvrec->u.copycfg.stype;

                if (nvrec->u.copycfg.svid != 0xffff)
                {
                    vdd = V_vddindx[nvrec->u.copycfg.svid];
                    scd->vdd    = vdd;
                    cor->srcvdd = vdd;
                }

                CM_act_scd(scd);
            }

            /* Update the SCD according to the configuration provided */

            if (nvrec->u.copycfg.svid == 0xffff)
            {
                cor->srcvdd  = NULL;
                scd->vdd     = NULL;
                cor->rcsvd   = 0xff;
                cor->rcscl   = 0xff;
            }
            else
            {
                if ((scd->vdd    == NULL)   ||
                    (cor->srcvdd == NULL))
                {
                    vid = nvrec->u.copycfg.svid;
                    vdd = V_vddindx[vid];
                    scd->vdd     = vdd;
                    cor->srcvdd  = vdd;
                    cor->rcsvd   = (UINT8) vid;
                    cor->rcscl   = (UINT8) (vid >> 8);

                    CM_act_scd(scd);
                }
            }
        }
        else
        {
            /* A scd is no longer configured. remove it from the VDD structure */
            if (scd != NULL)
            {
                cor->scd    = NULL;
                cor->srcvdd = NULL;

                vid = (cor->rscl << 8) | cor->rsvd;

                if (vid != 0xffff)
                {
                    vdd = V_vddindx[vid];

                    if (vdd == NULL)
                    {
                        scd->vdd = NULL;
                    }
                }

                CM_deact_scd(scd);
#ifdef M4_DEBUG_SCD
fprintf(stderr, "%s%s:%d put_scd 0x%08x\n", "BE ", __FILE__, __LINE__, (UINT32)scd);
#endif /* M4_DEBUG_SCD */
                put_scd(scd);
            }
        }
        /*
        **  set up the new COR destination values
        */
        cor->rdsn    = nvrec->u.copycfg.rdsn;
        cor->rdcl    = nvrec->u.copycfg.rdcl;
        cor->rdvd    = nvrec->u.copycfg.rdvd;

        /*
        **  set up the new COR source values
        */
        cor->rssn    = nvrec->u.copycfg.rssn;
        cor->rscl    = nvrec->u.copycfg.rscl;
        cor->rsvd    = nvrec->u.copycfg.rsvd;
    }
    else
    {
        /*
        **  Determine if the CFLG_B_DIS_DEVICE flag should be cleared. The
        **  flag is cleared if the provided configuration is  the same as
        **  the current COR.
        */
        for (corCheck = (UINT32 *)&cor->rid, NVCheck = (UINT32 *)&nvrec->u.copycfg.rid;
             corCheck < (UINT32 *)&cor->gid;
             ++corCheck, ++NVCheck)
        {
            if (*corCheck != *NVCheck)
            {
                check = FALSE;
                break;
            }
        }
        if (check == TRUE)
        {
            BIT_CLEAR(cor->flags, CFLG_B_DIS_DEVICE);
        }
    }
}

/**
******************************************************************************
**
**  @brief      P6_Refresh_Remote_Base_Copy
**
**              This routine provides a means of building a base copy operation.
**              This base copy places all components of the copy in suspended
**              state. This routine expects that the requested copy does not
**              already have a COR.
**
**  @param      nvrec = pointer to configuration record describing copy
**              cor   = pointer to COR
**
**  @return     none
**
******************************************************************************
**/
void P6_Refresh_Remote_Base_Copy(NVR *nvrec, COR *cor)
{
    DCD    *dcd = NULL;         /* pointer to dcd                           */
    SCD    *scd = NULL;         /* pointer to scd                           */
    VDD    *vdd = NULL;         /* pointer to vdd                           */
    UINT32  i;                  /* indet var                                */
    UINT32 *corCheck;           /* pointer to word for checking cor         */
    UINT32 *NVCheck;            /* pointer to word for checking cor         */
    UINT32  check = TRUE;       /* cor inhibit refresh check word           */
    UINT32  vid;                /* vdisk ID                                 */
    UINT8   mstate = MST_TERM;  /* cor_mstate                               */

    /*
    **  set COR active
    */
    BIT_CLEAR(cor->flags, CFLG_B_INACTIVE);

    /*
    **  restore primary and secondary owners
    */
    cor->powner  = nvrec->u.copycfg.powner;
    cor->sowner  = nvrec->u.copycfg.sowner;

    /*
    **********************************************************************
    **  Determine if the disable General information update flag is
    **  set. If not, update the general copy information. Otherwise,
    **  determine if the disable general copy information update flag
    **  should be cleared.
    **********************************************************************
    */
    if ( !(BIT_TEST(cor->flags, CFLG_B_DIS_GENERAL)))
    {
        memcpy(cor->label, nvrec->u.copycfg.label, 16);
        cor->gid = nvrec->u.copycfg.gid;
    }
    else
    {
        if (!((memcmp(cor->label, nvrec->u.copycfg.label, 16)) &&
            (cor->gid == nvrec->u.copycfg.gid)))
        {
            BIT_CLEAR(cor->flags, CFLG_B_DIS_GENERAL);
        }
    }

    /*
    **********************************************************************
    **  Determine if the disable copy state update flag is set. If not,
    **  update copy state. Otherwise, determine if the disable copy state
    **  update flag should be cleared
    **********************************************************************
    */
    if ( !(BIT_TEST(cor->flags, CFLG_B_DIS_STATE)))
    {
        /*
        **  save new copy registration state (crstate)
        */
        cor->crstate = nvrec->u.copycfg.cr_crstate;

        if (cor->crstate == CRST_USERSUSP || cor->crstate == CRST_REMSUSP)
        {
            mstate = MST_ACT;
        }

        cor->mirrorstate = mstate;

        /*
        **  Set the DCD p2 handler
        */
        dcd = cor->dcd;
        if (dcd != NULL)
        {
            dcd->p2handler = p6UpdhndTbl[nvrec->u.copycfg.dhidx & 0x0f];
        }

        /*
        **  Set the SCD p2 handler
        */
        scd = cor->scd;
        if (scd != NULL)
        {
            scd->p2handler = p6UpdhndTbl[nvrec->u.copycfg.shidx & 0x0f];
        }
    }
    else
    {
        if (cor->crstate == nvrec->u.copycfg.cr_crstate)
        {
            BIT_CLEAR(cor->flags, CFLG_B_DIS_STATE);
        }
    }

    /*
    **********************************************************************
    **  Determine if the disable copy device update flag is set. If not,
    **  update copy devices. Otherwise, determine if the disable copy device
    **  update flag should be cleared
    **********************************************************************
    */
    if ( !(BIT_TEST(cor->flags, CFLG_B_DIS_DEVICE)))
    {
        /*
        **  reconfigure possible DCD
        */
        i = (UINT32)(nvrec->u.copycfg.dhidx);

        if (i)
        {
            dcd = cor->dcd;
            if (dcd == NULL)
            {
                /*
                **  A New DCD is required.
                */
                dcd = get_dcd();
#ifdef M4_DEBUG_DCD
fprintf(stderr, "%s%s:%d get_dcd 0x%08x\n", "BE ", __FILE__, __LINE__, (UINT32)dcd);
#endif /* M4_DEBUG_DCD */
                dcd->cor = cor;
                cor->dcd = dcd;

                if (cor->cm == NULL)
                {
                    dcd->p2handler = p6UpdhndTbl[P6_UH_NULL];
                    dcd->p1handler = NULL;
                }
                else
                {
                    dcd->p2handler = p6UpdhndTbl[P6_UH_SUSPEND];
                    dcd->p1handler = NULL;
                }

                dcd->type = nvrec->u.copycfg.dtype;

                if (nvrec->u.copycfg.dvid != 0xffff)
                {
                    vdd = V_vddindx[nvrec->u.copycfg.dvid];
                    dcd->vdd     = vdd;
                    cor->destvdd = vdd;
                }

                CM_act_dcd(dcd);
                CM_mmc_sflag(cor);
            }

            /*
            **  Update the DCD according to the configuration provided
            */
            if (nvrec->u.copycfg.dvid == 0xffff)
            {
                cor->destvdd = NULL;
                dcd->vdd     = NULL;
                cor->rcdvd   = 0xff;
                cor->rcdcl   = 0xff;
            }
            else
            {
                if ((dcd->vdd     == NULL)   ||
                    (cor->destvdd == NULL))
                {
                    vid = nvrec->u.copycfg.dvid;
                    vdd = V_vddindx[vid];
                    dcd->vdd     = vdd;
                    cor->destvdd = vdd;
                    cor->rcdvd   = (UINT8) vid;
                    cor->rcdcl   = (UINT8) (vid >> 8);

                    CM_act_dcd(dcd);
                    CM_mmc_sflag(cor);
                }
            }

            /*
            **  Update the VDD mirror state
            */
            if (vdd != NULL)
            {
                switch(cor->crstate)
                {
                    case(CRST_AUTOSUSP):
                        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                        vdd->mirror = VD_COPYAUTOPAUSE;
                        if (dcd->vdd != NULL)
                        {
                            vdd->attr |= VD_SUSPEND;
                        }
                        break;

                    case(CRST_USERSUSP):
                    case(CRST_REMSUSP):
                        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                        vdd->mirror = VD_COPYUSERPAUSE;
                        if (dcd->vdd != NULL)
                        {
                            vdd->attr |= VD_SUSPEND;
                        }
                        break;

                    case(CRST_UNDEF):
                    case(CRST_INIT):
                    case(CRST_ACTIVE):
                        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                        vdd->mirror = VD_COPYTO;
                        vdd->attr &= ~(VD_SUSPEND);
                        break;

                    default:
                        break;
                }
            }
        }
        else
        {
            /* A dcd is no longer configured. remove it from the VDD structure */
            if (cor->dcd != NULL)
            {
                cor->dcd     = NULL;
                cor->destvdd = NULL;

                vid = (cor->rdcl << 8) | cor->rdvd;

                if (vid != 0xffff)
                {
                    vdd = V_vddindx[vid];

                    if (vdd == NULL)
                    {
                        dcd->vdd = NULL;
                    }
                }

                CM_deact_dcd(dcd);
#ifdef M4_DEBUG_DCD
fprintf(stderr, "%s%s:%d put_dcd 0x%08x\n", "BE ", __FILE__, __LINE__, (UINT32)dcd);
#endif /* M4_DEBUG_DCD */
                put_dcd(dcd);
            }
        }

        /*
        **********************************************************************
        **
        **  reconfigure possible SCD
        **
        **********************************************************************
        */
        i = (UINT32)(nvrec->u.copycfg.shidx);

        if (i)
        {
            scd = cor->scd;
            if (scd == NULL)
            {
                /*
                **  A New SCD is required.
                */
                scd = get_scd();
#ifdef M4_DEBUG_SCD
fprintf(stderr, "%s%s:%d get_scd 0x%08x\n", "BE ", __FILE__, __LINE__, (UINT32)scd);
#endif /* M4_DEBUG_SCD */
                scd->cor = cor;
                cor->scd = scd;

                if (cor->cm == NULL)
                {
                    scd->p2handler = p6UpdhndTbl[P6_UH_NULL];
                    scd->p1handler = NULL;
                }
                else
                {
                    scd->p2handler = p6UpdhndTbl[P6_UH_SUSPEND];
                    scd->p1handler = NULL;
                }

                scd->type = nvrec->u.copycfg.stype;

                if (nvrec->u.copycfg.svid != 0xffff)
                {
                    vdd = V_vddindx[nvrec->u.copycfg.svid];
                    scd->vdd    = vdd;
                    cor->srcvdd = vdd;
                }

                CM_act_scd(scd);
            }

            /* Update the SCD according to the configuration provided */
            if (nvrec->u.copycfg.svid == 0xffff)
            {
                cor->srcvdd  = NULL;
                scd->vdd     = NULL;
                cor->rcsvd   = 0xff;
                cor->rcscl   = 0xff;

            }
            else
            {
                if ((scd->vdd    == NULL)   ||
                    (cor->srcvdd == NULL))
                {
                    vid = nvrec->u.copycfg.svid;
                    vdd = V_vddindx[vid];
                    scd->vdd     = vdd;
                    cor->srcvdd  = vdd;
                    cor->rcsvd   = (UINT8) vid;
                    cor->rcscl   = (UINT8) (vid >> 8);

                    CM_act_scd(scd);
                }
            }
        }
        else
        {
            /* A scd is no longer configured. remove it from the VDD structure */
            if (scd != NULL)
            {
                cor->scd    = NULL;
                cor->srcvdd = NULL;

                vid = (cor->rscl << 8) | cor->rsvd;

                if (vid != 0xffff)
                {
                    vdd = V_vddindx[vid];

                    if (vdd == NULL)
                    {
                        scd->vdd = NULL;
                    }
                }

                CM_deact_scd(scd);
#ifdef M4_DEBUG_SCD
fprintf(stderr, "%s%s:%d put_scd 0x%08x\n", "BE ", __FILE__, __LINE__, (UINT32)scd);
#endif /* M4_DEBUG_SCD */
                put_scd(scd);
            }
        }

        /* set up the new COR destination values */
        cor->rdsn    = nvrec->u.copycfg.rdsn;
        cor->rdcl    = nvrec->u.copycfg.rdcl;
        cor->rdvd    = nvrec->u.copycfg.rdvd;

        /* set up the new COR source values */
        cor->rssn    = nvrec->u.copycfg.rssn;
        cor->rscl    = nvrec->u.copycfg.rscl;
        cor->rsvd    = nvrec->u.copycfg.rsvd;

    }
    else
    {
        /*
         *  Determine if the CFLG_B_DIS_DEVICE flag should be cleared. The
         *  flag is cleared if the provided configuration is  the same as
         *  the current COR.
         */
        for (corCheck = (UINT32 *)&cor->rid, NVCheck = (UINT32 *)&nvrec->u.copycfg.rid;
             corCheck < (UINT32 *)&cor->gid;
             ++corCheck, ++NVCheck)
        {
            if ( *corCheck != *NVCheck)
            {
                check = FALSE;
                break;
            }
        }
        if (check == TRUE)
        {
            BIT_CLEAR(cor->flags, CFLG_B_DIS_DEVICE);
        }
    }
}

/**
******************************************************************************
**
**  @brief      P6_Process_VDD_Loss
**
**              This routine provides a means adjusting the SCD, DCD, and COR
**              structures after the lose or deletion of a VDD struture.
**
**  @param      VDD   = pointer to VDD
**
**  @return     none
**
******************************************************************************
**/
void P6_Process_VDD_Loss(VDD *vdd)
{
    DCD    *dcd = NULL;         /* pointer to dcd                           */
    SCD    *scd = NULL;         /* pointer to scd                           */
    COR    *cor = NULL;         /* pointer to cor                           */

    for (scd = vdd->pSCDHead; scd != NULL; scd = scd->link)
    {
        cor = scd->cor;
        if (cor != NULL && cor->srcvdd == scd->vdd)
        {
            cor->srcvdd  = NULL;
            scd->vdd     = NULL;
            cor->rcsvd   = 0xff;
            cor->rcscl   = 0xff;
        }
    }

    dcd = vdd->pDCD;
    if (dcd != NULL)
    {
        cor = dcd->cor;
        if (cor != NULL && cor->destvdd == dcd->vdd)
        {
            cor->destvdd  = NULL;
            dcd->vdd      = NULL;
            cor->rcdvd    = 0xff;
            cor->rcdcl    = 0xff;
        }
    }
    DEF_Slink_Delete(vdd);
}

/**
******************************************************************************
**
**  @brief      P6_Build_CM
**
**              This routine provides a means of building a CM structure.
**              This routine expects that the requested copy does not already
**              have a CM structure.
**
**  @param      nvrec = pointer to configuration record describing copy
**              cor   = pointer to COR
**
**  @return     none
**
******************************************************************************
**/
CM *p6_Build_CM(NVR *nvrec, COR *cor)
{
    CM *cm = NULL;              /* pointer to cm                            */

    cm = get_cm();
#ifdef M4_DEBUG_CM
fprintf(stderr, "%s%s:%d get_cm 0x%08x\n", "BE ", __FILE__, __LINE__, (UINT32)cm);
#endif /* M4_DEBUG_CM */
    cm->cor = cor;
    cor->cm = cm;

    cm->copystate  = CSTO_NRS;
    cm->priority   = nvrec->u.copycfg.cm_pri;
    cm->copytype   = nvrec->u.copycfg.cm_type;
    cm->mirrortype = nvrec->u.copycfg.cm_mtype;
// if (cor->totalsegs > (REGSIZE_SEG * 256))
// {
//   cor->totalsegs = (REGSIZE_SEG * 256);
// }
    cm->totalsegs  = cor->totalsegs;
    return(cm);
}

/**
******************************************************************************
**
**  @brief      P6_Validate_Vaddr
**
**              This routine provides a means of validating the virtual device
**              addresses.
**
**  @param      nvrec = pointer to configuration record describing copy
**
**  @return     rc = PASS - addresses valid
**                   FAIL - addresses not valid
**
******************************************************************************
**/
UINT32 p6_Validate_Vaddr(NVR *nvrec)
{
    UINT32  rc = PASS;          /* return code (default = FAIL)             */
    UINT16  vid;                /* vdisk ID                                 */
    VDD    *vdd = NULL;         /* pointer to vdd                           */

    /* Determine if the source vid is within range and is valid */
    vid = nvrec->u.copycfg.svid;
    if (vid != 0xffff)
    {
        if (vid >= MAX_VIRTUAL_DISKS)
        {
            rc = FAIL;
            ++p6RestoreErrors[P6RE_INVALIDSVID];
        }
        else
        {
            vdd = V_vddindx[vid];
            if (vdd == NULL)
            {
                rc = FAIL;
                ++p6RestoreErrors[P6RE_NOSVDD];
            }
        }
    }

    /* Determine if the destination vid is within range and is valid */
    vid = nvrec->u.copycfg.dvid;
    if (vid != 0xffff)
    {
        if (vid >= MAX_VIRTUAL_DISKS)
        {
            rc = FAIL;
            ++p6RestoreErrors[P6RE_INVALIDDVID];
        }
        else
        {
            vdd = V_vddindx[vid];
            if (vdd == NULL)
            {
                rc = FAIL;
                ++p6RestoreErrors[P6RE_NODVDD];
            }
        }
    }
    return(rc);
}

#ifdef LINUX_VER_NVP6_MM
/**
******************************************************************************
**
**  @brief      p6_Set_CopyWorkInProcess_Bit
**
**              This routine sets the 5th bit in the header of nvram P6, that
**              represents the copy work in process. Changes in the DRAM buffer
**              as well as in Micro Memory Card.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/

void p6_Set_CopyWorkInProcess_Bit (void)
{
    /*
    ** Set the "Copy Work In process" bit
    */
    BIT_SET(gNvramP6Image[0], 5);

    /*
    ** Update it Micro Memory Card (writing Ist byte is enough, as we have changed the
    ** info in the Ist byte only.
    */
    MM_Write((UINT32)MICRO_MEM_BE_P6_START,gNvramP6Image,1);
}

/**
******************************************************************************
**
**  @brief      p6_Clear_CopyWorkInProcess_Bit
**
**              This routine clears the 5th bit in the header of nvram P6, that
**              represents the copy work in process. Changes in the DRAM buffer
**              as well as in Micro Memory Card.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/

void p6_Clear_CopyWorkInProcess_Bit (void)
{
    /*
    ** Set the "Copy Work In process" bit
    */
    BIT_CLEAR(gNvramP6Image[0], 5);

    /*
    ** Update it Micro Memory Card (writing Ist byte is enough, as we have changed the
    ** info in the Ist byte only.
    */
    MM_Write((UINT32)MICRO_MEM_BE_P6_START,gNvramP6Image,1);
}
#endif /* LINUX_VER_NVP6_MM */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/


