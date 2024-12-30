/* $Id: daml.c 161041 2013-05-08 15:16:49Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       daml.c
**
**  @brief      Drive allocation map functions
**
**  To provide a common means of handling the drive allocation maps for the
**  back end processor.
**
**  Copyright (c) 1996-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "daml.h"

#include "cev.h"
#include "chn.h"
#include <ctype.h>
#include "def.h"
#include "defbe.h"
#include "dev.h"
#include "error.h"
#include "ficb.h"
#include "ilt.h"
#include "kernel.h"
#include "ldd.h"
#include "lvm.h"
#include "nvr.h"
#include "nvram.h"
#include "OS_II.h"
#include "pcb.h"
#include "pdd.h"
#include "RL_PSD.h"
#include "RL_RDD.h"
#include "sdd.h"
#include <stdlib.h>
#include <string.h>
#include "system.h"
#include "target.h"
#include "vdd.h"
#include "vlar.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "ddr.h"
#include "misc.h"
#include <stdio.h>
#include "CT_defines.h"

#include "CT_change_routines.h"

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
UINT32      DA_damfind(DAML *, UINT32);
void        DA_checkdaml(DAML *);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      To provide a consistent means of calculating the space that is
**              available on a disk. This will include the TAS and LAS.
**
**  A number of checks are made prior to the building of a DAM and the release
**  (which calculates the space values). These checks are designed to ferret
**  out inoperable devices or devices without capacity.
**
**  @param      pid - PID of drive for which the capacities are to be updated
**  @param      force - Boolean (T = force DAM build first, F = do not force)
**
**  @return     none
**
**  @attention  PDD for this PID modified to reflect current capacities
**
******************************************************************************
**/
void DA_CalcSpace(UINT16 pid, UINT8 force)
{
    PDD        *pdd = P_pddindx[pid];

    /* If the PDD exists, update the sizes. */
    if (pdd == NULL)
    {
        return;
    }

    /* If force is TRUE, dirty the DAM to make sure it gets rebuilt. */
    if (force)
    {
        DA_DAMDirty(pid);
    }

    /*
     * If the device capacity is zero, then set the total available space
     * and the largest available space to zero. Otherwise call the dam
     * build code that fills in the fields.
     */
    if (pdd->devCap == 0)
    {
        pdd->tas = pdd->las = 0;
    }
    else
    {
        DA_DAMBuild(pid);
    }
}   /* End of DA_CalcSpace */

/**
******************************************************************************
**
**  @brief      To provide a common means of setting the dirty bit in a DAML.
**
**  @param      pid - the PDisk ID of the drive to be set into a dirty DAM mode
**
**  @return     none
**
******************************************************************************
**/
void DA_DAMDirty(UINT16 pid)
{
    PDD        *pdd = P_pddindx[pid];

    /* Get the DAML and if it is non-NULL, then set the dirty flag. */
    if (pdd != NULL && pdd->pDAML != NULL)
    {
        BIT_SET(pdd->pDAML->flags, DAB_DIRTY);
    }
}   /* End of DA_DAMDirty */

/**
******************************************************************************
**
**  @brief      To provide a consistent means of allocating and initializing
**              the DAM based upon current RAID definitions from corresponding
**              PSD structures.
**
**  If the disk does not have a drive allocation map, or it is dirty, then the
**  map is created and an pointer to it is placed into the PDD.
**
**  The area corresponding to the raw device capacity (minus the reserved area)
**  is then set to unassigned. Each corresponding PSD is used to determine
**  which areas are to be assigned.
**
**  @param      pid - the PID of the drive for which the DAML is to be built.
**
**  @return     DAML* - pointer to the DAML built. NULL if not built.
**
******************************************************************************
**/
DAML       *DA_DAMBuild(UINT16 pid)
{
    PDD        *pdd;            /* PDD for PID being updated            */
    RDD        *rdd;            /* RDD for build loop                   */
    UINT32      i;              /* Index for build loop                 */
    DAML       *daml;           /* DAML to be returned                  */
    DAMLX      *damlx;          /* Extension                            */

    pdd = P_pddindx[pid];
    if (pdd == NULL)
    {
        return NULL;
    }

    daml = pdd->pDAML;
    if (daml == NULL || BIT_TEST(daml->flags, DAB_DIRTY))
    {
        /*
         * We need to build the DAM. First allocate one if needed and then
         * start to fill it all in.
         */
        if (daml == NULL)
        {
            daml = pdd->pDAML = (DAML *)s_MallocC(DAML_SIZE, __FILE__, __LINE__);
            pdd->pDAML->pdd = pdd;
        }
        else
        {
            /*
             * Clear out the maps. Clear out as large as the map says it is
             * rather than the entire memory area.
             */
            daml->firstGap = 0;
            memset(daml + 1, 0, (signed)(daml->count * DAMLX_SIZE));
            BIT_CLEAR(daml->flags, DAB_DIRTY);
        }

        /*
         * Start by allocating an area for the reserved area. Then go through
         * each RAID and allocate any space used in that RAID.
         */
        damlx = (DAMLX *)(daml + 1);
        damlx->auSda = 0;
        damlx->auSLen = RESERVED_AREA_SIZE / DISK_SECTOR_ALLOC;
        damlx->auGap = (pdd->devCap / DISK_SECTOR_ALLOC) - damlx->auSLen;
        damlx->auRID = 0xFFFF;
        daml->count = 1;

        /*
         * Now loop through each RDD and PSD looking for this drive. If it is
         * found, allocate the space for that PSD.
         */
        for (i = 0; i < MAX_RAIDS; i++)
        {
            rdd = R_rddindx[i];
            if (rdd != NULL && rdd->type != RD_LINKDEV && rdd->type != RD_SLINKDEV)
            {
                UINT8       found_once = 0;
                PSD        *psd;            /* PSD for build loop */
                PSD        *s_psd;          /* Starting PSD for build loop */

                s_psd = psd = *((PSD **)(rdd + 1));

                /*
                 * Check the PID throughout the RAID. If we find the PID we are
                 * updating, then allocate the space.
                 */
                do
                {
                    if (psd->pid == pid)
                    {
                        if (found_once++ != 0)
                        {
                            fprintf(stderr,"%s%s:%u %s Found pid %d in psd list twice, aborting\n",
                                    FEBEMESSAGE, __FILE__, __LINE__, __func__, pid);
                            abort();
                        }
                        DA_DAMAsg(daml, psd->sda / DISK_SECTOR_ALLOC,
                                  psd->sLen / DISK_SECTOR_ALLOC, psd);
                    }
                    psd = psd->npsd;
                } while (psd != s_psd);
            }
        }
    }

    /*
     * Now that the DAML is built or was not dirty to start with, summarize it
     * and exit.
     */
    DA_DAMSum(daml);
    return (daml);
}   /* End of DA_DAMBuild */

/**
******************************************************************************
**
**  @brief      To provide a standard means of allocating storage space from a
**              specified data labelled device.
**
**  The CEV structure is passed in which has a list of DAMLs. The appropriate
**  DAML is found in this list and the amount of data requested is allocated
**  from that DAML.
**
**  The DAM is assumed to be clean when it comes into this function.
**
**  @param      cev - the create/expand variables structure
**  @param      psd - the PSD for which data space is to be allocated
**  @param      au  - the number of allocation units to be allocated
**
**  @return     none
**
******************************************************************************
**/
void DA_Alloc(CEV *cev, PSD *psd, UINT32 au)
{
    DAML       *daml;           /* DAML pointer                         */
    UINT32      i;              /* Pointer into DAML list in CEV        */
    UINT32      startau;        /* Starting AU to be used               */

    /*
     * There is an array of pointers to DAMLs at the end of the CEV structure.
     * Find the one for which allocation is being requested and do allocation.
     */
    for (i = 0; i < cev->numDAML; i++)
    {
        daml = cev->daml[i];

        if (daml->pdd->pid == psd->pid)
        {
            /*
             * Now go find the segment to use and allocate it. Set PSD fields
             * to indicate the amount of space used and the starting address.
             */
            startau = DA_damfind(daml, au);
            if (startau != 0)
            {
                DA_DAMAsg(daml, startau, au, psd);
                BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                psd->sda = (UINT64)startau * DISK_SECTOR_ALLOC;
                psd->sLen = (UINT64)au * DISK_SECTOR_ALLOC;
                DA_DAMSum(daml);
            }
            break;
        }
    }
}   /* End of DA_Alloc */

/**
******************************************************************************
**
**  @brief      To provide a standard means of releasing storage space from
**              a specified data labelled device.
**
**              The psd is used to find the DAML within the PDD and the segment
**              starting at sda for sLen blocks is deallocated from the DAML.
**              The DAML is then resummed to get the right counts.
**
**  @param      psd - the PSD for which data space is to be allocated
**
**  @return     psd - Return the PSD passed in so g0 still holds the PSD.
**
******************************************************************************
**/
PSD        *DA_Release(PSD *psd)
{
    DAML       *daml = NULL;    /* DAML pointer                         */
    DAMLX      *damlx;          /* Extensions                           */
    PDD        *pdd;            /* PDD having space deallocated         */
    UINT32      i = 0;          /* Pointer into DAML list in CEV        */
    UINT32      sdaau;          /* Starting AU to be used               */
    UINT32      slenau;         /* Length in AU                         */
    UINT32      upper;
    UINT32      lower;          /* Search parms for binary seach        */
    UINT8       found = FALSE;  /* Did we find the entry                */
    UINT64      temp;

    /* Get the PDD and then the DAML. */
    pdd = P_pddindx[psd->pid];
    if (pdd == NULL || pdd->pDAML == NULL)
    {
        return psd;
    }

    daml = pdd->pDAML;

    /* If PSD was dirty, leave since it is complicated to clean it up. */
    if (BIT_TEST(daml->flags, DAB_DIRTY))
    {
        return psd;
    }

    /*
     * Now go find the segment and deallocate it. Note that we cannot remove
     * the 0th entry so a check of upper and lower equal at the start is OK.
     */
    lower = 0;
    upper = daml->count - 1;

    sdaau = psd->sda / DISK_SECTOR_ALLOC;
    slenau = psd->sLen / DISK_SECTOR_ALLOC;

    damlx = (DAMLX *)(daml + 1) + upper;

    /* Search until found or until upper and lower are equal. */
    if (damlx->auSda > sdaau)
    {
        /*
         * The element may be in the table. It is at least in the range of the
         * table entries.
         */
        while (lower < upper)
        {
            i = (upper + lower) / 2;
            damlx = (DAMLX *)(daml + 1) + i;
            if (sdaau == damlx->auSda)
            {
                found = TRUE;
                break;
            }
            else if (sdaau < damlx->auSda)
            {
                if (upper == i)
                {
                    break;
                }
                else
                {
                    upper = i;
                }
            }
            else if (lower == i)
            {
                break;
            }
            else
            {
                lower = i;
            }
        }
    }
    else if (damlx->auSda == sdaau)
    {
        found = TRUE;
        i = upper;
    }

    /*
     * If we found the entry, update the previous entry and remove the current
     * one. The variable damlx will point to the one to be removed and i is
     * the index into the table.
     */
    if (found)
    {
        /*
         * This case should never happen. If it does, just dirty the DAM and
         * continue. This will cause a clean up.
         */
        if (damlx->auSLen != slenau)
        {
            DA_DAMDirty(pdd->pid);
        }

        /* Found it, remove the space. */
        temp = (UINT64)((damlx - 1)->auGap) + (UINT64)(damlx->auGap) + (UINT64)slenau;
        if (temp > (pdd->devCap / DISK_SECTOR_ALLOC))
        {
            DA_DAMDirty(pdd->pid);
        }

        (damlx - 1)->auGap += damlx->auGap + slenau;
        memmove(damlx, damlx + 1, (signed)((daml->count - i) * DAMLX_SIZE));
        daml->count--;

        if ((i - 1) < daml->firstGap)
        {
            daml->firstGap = i - 1;
        }

        /* Do a quick and dirty update of counts if they need to change. */
        daml->total += slenau;
        if (daml->largest < (damlx - 1)->auGap)
        {
            daml->largest = (damlx - 1)->auGap;
        }
        pdd->tas = daml->total * DISK_SECTOR_ALLOC;
        pdd->las = daml->largest * DISK_SECTOR_ALLOC;
    }
    else
    {
        /*
         * We did not find the record. This is not good. To handle this case,
         * just dirty the DAM. This makes the DAM unusable until rebuilt.
         */
        DA_DAMDirty(pdd->pid);
    }

    /* Return the PSD passed in so g0 still holds the PSD. */
    return (psd);
}   /* End of DA_Release */

/**
******************************************************************************
**
**  @brief      To provide a common means of finding an appropriate allocation
**              from a DAM (Disk Allocation Map).
**
**              Whenever possible, a trial allocation is returned based upon the
**              calling parameters. DA_DAMAsg must be utilized to finalize the
**              allocation within the bitmap. This trial allocation is found
**              utilizing a best fit algorithm.
**
**  @param      daml - the DAML in which the data space is being requested
**  @param      au  - the number of allocation units to be allocated
**
**  @return     startau - the AU where the best allocation lies.
**
******************************************************************************
**/
UINT32 DA_damfind(DAML *daml, UINT32 au)
{
    UINT32      startau = 0;    /* Starting AU to be used               */
    DAMLX      *damlx;          /* Pointer into DAML list               */
    UINT32      newgap;         /* Best fit space so far                */
    UINT32      index_D;        /* Index into extensions                */

    /*
     * Search through DAML. DAML contains starting AU, AU counts, and number
     * of AUs until next start. For this function, we will find the gap that
     * is the closest fit to the number of allocation units being requested.
     * When we find that gap, the starting AU for that gap is returned.
     */
    index_D = daml->firstGap;
    damlx = (DAMLX *)(daml + 1) + index_D;
    newgap = 0xFFFFFFFF;

    while (index_D < daml->count)
    {
        /* Do a quick check for exact match. */
        if (damlx->auGap == au)
        {
            startau = damlx->auSda + damlx->auSLen;
            break;
        }
        else if ((damlx->auGap > au) && (newgap > damlx->auGap))
        {
            startau = damlx->auSda + damlx->auSLen;
            newgap = damlx->auGap;
        }
        index_D++;
        damlx++;
    }

    /* Return the starting AU to be used. Zero indicates no match. */
    return (startau);
}   /* End of DA_damfind */

/**
******************************************************************************
**
**  @brief      To provide a common means of setting the appropriate
**              allocation in a DAM (Disk Allocation Map).
**
**  The DAML is adjusted to allocate segment described by the input parameters.
**  A new entry is created in the DAML extension area for this allocation and
**  the gaps are adjusted accordingly.
**
**  Note that overlaps are also detected in this code. Overlaps may occur if
**  there is defragmentation taking place. In this case, starting address is
**  set, number of AUs is truncated to reflect gap size and gap is set to zero.
**
**  @verbatim
**              ++++++++++++++
**              ++++++++++++++
**                       +++++++++++++++
**                       +++++++++++++++
**              10       19  23        32
**  @endverbatim
**  In this case, rather than a 10 for 13 and 19 for 13, there would be a 10
**  for 9 (gap 0) and a 19 for 13 (gap x) in the table.
**
**  @param      daml    - the DAML in which the data space is being requested
**  @param      startau - the starting allocation unit address
**  @param      au      - the number of allocation units to be allocated
**  @param      psd     - PSD being assigned
**
**  @return     none
**
******************************************************************************
**/
void DA_DAMAsg(DAML *daml, UINT32 startau, UINT32 au, PSD *psd)
{
    DAMLX      *damlx;          /* Pointer into DAML list               */
    DAMLX      *shiftdamlx;     /* Pointer into DAML list               */
    UINT32      entrynum;       /* Entry where we are placing this one  */
    UINT32      shiftcnt;       /* Bytes to shift down                  */

    /*
     * The first step is to find the location in the DAML extension area where
     * the starting AU lies. We assume that the table has at least one entry
     * in it since we build it with the reserved area intact.
     */
    damlx = (DAMLX *)(daml + 1) + daml->firstGap;
    entrynum = daml->firstGap;

    /*
     * Search until we find that there is an exact match or we have a gap.
     * Note that we do not want to roll off the end of the list. If we are
     * checking the last entry, then we will go after the last one so stop
     * before we lose the last pointer.
     */
    while ((entrynum < (unsigned)(daml->count - 1)) &&
           ((damlx->auSda + damlx->auSLen + damlx->auGap) < startau))
    {
        damlx++;
        entrynum++;
    }

    /*
     * Now shift everything down one slot. damlx points to the entry above the
     * entry location. That is, everything below this position must be shifted
     * down. The entry count is the entry after which we will place the new one.
     */
    shiftcnt = (daml->count - ++entrynum) * DAMLX_SIZE;
    if (shiftcnt != 0)
    {
        memmove(damlx + 2, damlx + 1, (signed)shiftcnt);
    }

    /*
     * damlx points to the extension after which this extension will be placed.
     * A gap in the table has been created for insertion. Set the gap on the
     * current location and move to the next location to fill in the newly
     * created extension.
     */
    damlx->auGap = startau - (damlx->auSda + damlx->auSLen);

    damlx++;
    damlx->auSda = startau;
    damlx->auRID = psd->rid;

    shiftdamlx = damlx + 1;

    /*
     * Make sure overlap for defragmentation is checked here. If number of AUs
     * being inserted is greater than gap available (this will happen in a
     * defrag movement that overlaps), then set gap to zero and set the size
     * of the allocation to the space available rather than requested size.
     */
    if (entrynum == daml->count)
    {
        /* Last entry in the table case. */
        damlx->auSLen = au;
        damlx->auGap = (daml->pdd->devCap / DISK_SECTOR_ALLOC) - (startau + au);
    }
    else if (startau + au >= shiftdamlx->auSda)
    {
        /* Overlap case. */
        damlx->auGap = 0;
        damlx->auSLen = shiftdamlx->auSda - startau;
    }
    else
    {
        damlx->auSLen = au;
        damlx->auGap = shiftdamlx->auSda - (startau + au);
    }

    /* Bump the count since we put one in. */
    daml->count++;
}   /* End of DA_DAMAsg */

/**
******************************************************************************
**
**  @brief      To provide a common means of summarizing the allocations
**              in a DAM (Disk Allocation Map).
**
**  The DAML is examined to find the largest available gap and the total
**  accumulated gaps. The DAML structure is updated with this information.
**
**  @param      daml - the DAML being summarized
**
**  @return     none
**
******************************************************************************
**/
void DA_DAMSum(DAML *daml)
{
    DAMLX      *damlx;          /* Pointer into DAML list               */
    UINT32      biggap = 0;     /* Biggest space so far                 */
    UINT32      totalgap = 0;   /* Total space                          */
    UINT32      i;              /* Counter                              */
    UINT16      firstGap = 0;   /* First gap location                   */
    UINT8       found = FALSE;  /* Gap found                            */

    for (i = 0, damlx = (DAMLX *)(daml + 1); i < daml->count; i++, damlx++)
    {
        if (!found && damlx->auGap != 0)
        {
            found = TRUE;
            firstGap = i;
        }
        if (damlx->auGap > biggap)
        {
            biggap = damlx->auGap;
        }
        totalgap += damlx->auGap;
    }

    daml->largest = biggap;
    daml->total = totalgap;
    daml->firstGap = firstGap;

    if (daml->pdd->devCap == 0)
    {
        daml->pdd->tas = 0;
        daml->pdd->las = 0;
    }
    else
    {
        daml->pdd->tas = totalgap * DISK_SECTOR_ALLOC;
        daml->pdd->las = biggap * DISK_SECTOR_ALLOC;
    }
}   /* End of DA_DAMSum */


#if 0
/**
******************************************************************************
**
**  @brief      To provide a common means of checking a DAML to make
**              sure it is consistent.
**
**  DAML is checked for consistency by looking at the summaries in the header,
**  checking for negative numbers, and checking the SDAs for increasing values.
**
**  @param      daml - the DAML being checked
**
**  @return     none
**
******************************************************************************
**/
void DA_checkdaml(DAML *daml)
{
    DAMLX      *damlx;          /* Pointer into DAML list               */
    UINT32      biggap = 0;     /* Biggest space so far                 */
    UINT32      totalgap = 0;   /* Total space                          */
    UINT32      i;              /* Counter                              */
    UINT16      firstGap = 0;   /* First gap location                   */
    UINT32      lastSDA = 0;    /* Last SDA value                       */
    UINT32      nextSDA = 0;    /* Last SDA value                       */
    UINT8       found = FALSE;  /* Found the first gap                  */

    if (!BIT_TEST(daml->flags, DAB_DIRTY))
    {
        for (i = 0, damlx = (DAMLX *)(daml + 1); i < daml->count; i++, damlx++)
        {
            if (nextSDA != damlx->auSda)
            {
                i960_fault(nextSDA, damlx->auSda, 5, __FILE__, __LINE__);
                i960_fault(nextSDA, damlx->auSda, 3, __FILE__, __LINE__);
            }
            else
            {
                nextSDA = damlx->auSda + damlx->auSLen + damlx->auGap;
            }

            if ((lastSDA != 0) && (damlx->auSda <= lastSDA))
            {
                i960_fault(damlx->auSda, lastSDA, 5, __FILE__, __LINE__);
                i960_fault(damlx->auSda, lastSDA, 3, __FILE__, __LINE__);
            }
            else
            {
                lastSDA = damlx->auSda;
            }

            if (!found && damlx->auGap != 0)
            {
                found = TRUE;
                firstGap = i;
            }

            if (damlx->auGap > biggap)
            {
                biggap = damlx->auGap;
            }

            totalgap += damlx->auGap;
        }

        /* Now check the values. */
        if ((daml->largest != biggap) ||
            (daml->total != totalgap) || (daml->firstGap != firstGap))
        {
            i960_fault(daml->largest, biggap, 5, __FILE__, __LINE__);
            i960_fault(daml->largest, biggap, 3, __FILE__, __LINE__);
        }
    }
}   /* End of DA_checkdaml */
#endif  /* 0 */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
