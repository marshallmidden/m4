/* $Id: deffe.c 161678 2013-09-18 19:25:16Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       deffe.c
**
**  @brief      Define C functions
**
**  To provide support of configuration definition requests.
**
**  Copyright (c) 2002-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "deffe.h"

#include "fabric.h"
#include "cache.h"
#include "CA_CacheFE.h"
#include "vcd.h"
#include "cdriver.h"
#include "cimt.h"
#include "def.h"
#include "def_isp.h"
#include "ficb.h"
#if FE_ISCSI_CODE
#include "fsl.h"
#endif  /* FE_ISCSI_CODE */
#include "icl.h"
#include "imt.h"
#include "isp.h"
#include "kernel.h"
#include "lvm.h"
#include "magdrvr.h"
#include "MR_Defs.h"
#include "pcb.h"
#include "pr.h"
#include "RL_RDD.h"
#include "sdd.h"
#include "system.h"
#include "vlar.h"
#include "target.h"
#include "wcache.h"
#include "NV_Memory.h"

#include <string.h>
#include "misc.h"
#include "pr.h"
#include "XIO_Std.h"
#include "XIO_Const.h"
#include "XIO_Types.h"
#include "XIO_Macros.h"
#include "CT_defines.h"

#include "MP_ProcProto.h"
#include <stdio.h>
#include <byteswap.h>

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
#define ALL             0xFFFF
#define MANAGED         0

/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/
#define MASK        ( (1<<SD_DEFAULT) | (1<<SD_UNMANAGED) )
#define UNMANAGED   (1<<SD_UNMANAGED)

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
extern UINT16 hba_q_cnt[MAX_PORTS];  /* defined in isp.as */
extern UINT8  FT_DEFINED;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
void DEF_GetServersActiveList(UINT16 targetId, UINT8 * list);
UINT8 DEF_GetTargetResource(MR_PKT* pMRP);
UINT8 DEF_GetTargetInfo(MR_PKT* pMRP);
UINT8 DEF_ResumeCacheInit(MR_PKT* pMRP);
UINT8 DEF_ServerStats(MR_PKT* pMRP);
UINT8 DEF_FailPort(MR_PKT* pMRP);
UINT8 DEF_Generic(MR_PKT* pMRP);

UINT32 DEF_GetServersWithStatsList(UINT16 targetId, UINT8 * list);
UINT8 DEF_MicroMemory(MR_PKT* pMRP);
UINT8 DEF_GetHBAStats(MR_PKT* pMRP);
UINT8 DEF_MMTest(MR_PKT* pMRP);
UINT8 DEF_ConfigTar(MR_PKT* pMRP);
UINT8 DEF_ConfigServer(MR_PKT *pMRP);
UINT8 DEF_GetPortType(MR_PKT* pMRP);

void CopyHBAStats( UINT32 hbaNum, MRGETHABSTATS_RSP * mrghs);

UINT8 DEF_CBridge(MR_PKT*);
void CalcHBAStats(void);
void InitHBAStats(void);
UINT8 DEF_PRClr(MR_PKT* pMRP);
UINT8 DEF_PRCfgComp(MR_PKT* pMRP);
UINT8 DEF_UpdPRR(MR_PKT* pMRP);
UINT8 DEF_PRGet(MR_PKT* pMRP);
#if ISCSI_CODE
extern UINT8* hex2a (UINT64 n,UINT8* strVal);
static UINT8 DEF_GetSData(MR_PKT* pMRP);
extern UINT8 ICL_GetDlmPathStats(MR_PKT* pMRP);
extern UINT8 ICL_DlmPathSelectionAlgorithm(MR_PKT* pMRP);
#endif  /* ISCSI_CODE */

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Gets a list of activer servers associated with the
**              specified target.
**
**              This function return a list of servers
**              that are associated with the specified target.
**              The IMT records are examined to
**              determine the server IDs.
**
**  @param      targetId    - Target ID
**  @param      * list      - List pointer
**
**  @return     none
**
******************************************************************************
**/
void DEF_GetServersActiveList(UINT16 targetId, UINT8 * list)
{
    IMT * imt;
    UINT32 sid;
    UINT8 port;

    /*
    ** Find the port number for this target.
    */
    port = ispPortAssignment[targetId];

    /*
    ** Is the port number valid?
    */
    if (port < MAX_PORTS)
    {
        /*
        ** Get the first IMT.
        */
        imt = cimtDir[port]->imtHead;

        /*
        ** Walk the IMT linked list.
        */
        while (imt != NULL)
        {
            if (imt->tid == targetId)
            {
                /*
                ** Get the server ID.
                */
                sid = imt->sid;

                if (sid < MAX_SERVERS && S_sddindx[sid] != NULL)
                {
                    /*
                    ** Check if this is the default mappings.  The default
                    ** is not the real server number for this WWN.
                    */
                    if (S_sddindx[sid]->attrib & (1<<SD_DEFAULT))
                    {
                        /*
                        ** Look up the server ID for this WWN and target.
                        ** Include new server in the lookup.
                        */
                        sid = DEF_WWNLookup(imt->mac, targetId, TRUE, imt->i_name);
                    }

                    /*
                    ** Check for a valid server ID
                    */
                    if (sid < MAX_SERVERS)
                    {
                        /*
                        ** Record this server ID.
                        */
                        list[sid] = TRUE;
                    }
                }
            }

            /*
            ** Increment to the next imt.
            */
            imt = imt->link;
        }
    }
}

/**
******************************************************************************
**
**  @brief      Gets a list of active or inactive servers associated with the
**              specified target.  Stats are available for these servers.
**
**              This function return a list of servers
**              that are associated with the specified target.
**              The IMT records are examined to
**              determine the server IDs.
**
**  @param      targetId    - Target ID
**  @param      * list      - List pointer
**
**  @return     none
**
******************************************************************************
**/
UINT32 DEF_GetServersWithStatsList(UINT16 targetId, UINT8 * list)
{
    IMT * imt;
    UINT32 sid;
    UINT32 sidCount = 0;

    /*
    ** Get the first IMT.
    */
    imt = mag_imt_head;

    /*
    ** Walk the IMT linked list.
    */
    while (imt != NULL)
    {
        if (imt->tid == targetId || targetId == ALL)
        {
            /*
            ** Get the server ID.
            */
            sid = imt->sid;

            if (sid < MAX_SERVERS && S_sddindx[sid] != NULL)
            {
                /*
                ** Check if this is the default mappings.  The default
                ** is not the real server number for this WWN.
                */
                if (S_sddindx[sid]->attrib & (1<<SD_DEFAULT))
                {
                    /*
                    ** Look up the server ID for this WWN and target.
                    ** Include new server in the lookup.
                    */
                    sid = DEF_WWNLookup(imt->mac, targetId, TRUE, imt->i_name);
                }

                /*
                ** Check for a valid server ID
                */
                if (sid < MAX_SERVERS)
                {
                    /*
                    ** Record this server ID.
                    */
                    list[sid] = TRUE;
                    ++sidCount;
                }
            }
        }

        /*
        ** Increment to the next imt.
        */
        imt = imt->link2;
    }

    return sidCount;
}

/**
******************************************************************************
**
**  @brief      Gets a list of resource that are associated with the
**              specified target.
**
**              This function return a list of either servers or
**              vdisks that are associated with the specified target.
**              The target ID of server records are examined to
**              determine the target association.
**
**  @param      pMRP - Pointer to MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_GetTargetResource(MR_PKT* pMRP)
{
    SDD * sdd;                      /* SDD pointer to move through lists    */
    LVM * lvm;                      /* LVM pointer to move through lists    */
    VCD * vcd;                      /* VCD pointer to move through lists    */
    UINT8 * idMapped;               /* SID/VID mapped indicato              */
    UINT16 count = 0;               /* Count of devices found               */
    UINT16 i;                       /* Loop variables                       */
    UINT16 targetId;                /* Target ID                            */
    UINT32 list;                    /* List option                          */
    UINT16 * pList;                 /* pointer to returned list             */
    UINT16 maxCount;                /* Maximum number of devices in list    */
    UINT16 size;                    /* Maximum number of devices in list    */
    MRGETTRLIST_REQ * mrtr;         /* Pointer to input MRP                 */
    UINT8   retStatus = DEOK;       /* Return status, prep good status      */
    UINT16 cacheFilter = 0xFFFF;
    UINT32 defaultFilter = 0;
    UINT32 defaultMapping;
    UINT16 start;
    UINT16 end;
    LVM* lvmPtr;                    /* LVM pointer to move through lists    */
    LVM* ilvmPtr;                   /* LVM pointer to move through lists    */

    /*
    ** Get pointer to Parm block address
    */
    mrtr = (MRGETTRLIST_REQ *) pMRP->pReq;

    /*
    ** First, grab the return data address and length allowed.
    */
    pList = (UINT16 *) (pMRP->pRsp + 1);
    pMRP->pRsp->rspLen = pMRP->rspLen;
    size = sizeof(UINT16);
    maxCount = ((UINT16) pMRP->rspLen - sizeof(MR_RSP_PKT)) / size;

    /*
    ** Get list option and target ID from the input packet.
    */
    list = mrtr->listType;
    targetId = mrtr->tid;

    /*
    ** Check for the SID/WWN format
    */
    if ((list & WWN_FORMAT) != 0)
    {
        /*
        ** Remove the WWN_FORMAT from the list type.  The size variable
        ** is now used to create the list in the SID/WWN format
        */
        list &= ~WWN_FORMAT;

        /*
        ** The SID/WWN is not supported for VDISK Lists.
        */
        if (list != VDISKS && list != VDISKS_CACHEN && list != VDISKS_CACHDIS)
        {
            /*
            ** Since more data per resource is sent back for the SID/WWN,
            ** the maximum count of resources needs to be recalculated.
            */
            size = 8*sizeof(UINT16);
            maxCount = ((UINT16) pMRP->rspLen - sizeof(MR_RSP_PKT)) / size;
        }
        else
        {
            /*
            ** Change it back.
            */
            list |= WWN_FORMAT;
        }
    }

    /*
    ** Check if target exists.
    */
    if (targetId != ALL &&
       (targetId >= MAX_TARGETS || T_tgdindx[targetId] == NULL))
    {
        /*
        ** Invalid target ID.
        */
        retStatus = DEINVTID;
    }
    else if (list == SERVERS || list == SERVERS_DEFAULT ||
             list == SERVERS_MANAGED || list == SERVERS_UNMANAGED ||
             list == SERVERS_XIO)
    {
        if (mrtr->sid >= MAX_SERVERS)
        {
            /*
            ** Invalid Server ID.
            */
            retStatus = DEINVSID;
        }
        else
        {
            if (list == SERVERS_DEFAULT)
            {
                /*
                ** include only the default server.
                */
                defaultFilter = (1<<SD_DEFAULT);
            }

            /*
            ** Get the list of servers for this target.
            */
            for (i = mrtr->sid; i < MAX_SERVERS; ++i)
            {
                /*
                ** Get a pointer to the next server record.
                */
                sdd = S_sddindx[i];

                /*
                ** Check if this server is associated with the
                ** specified target.
                */
                if (sdd != NULL && (sdd->tid == targetId || targetId == ALL))
                {
                    /*
                    ** Check if this server has the correct attributes.
                    */
                    if ((sdd->attrib & (1<<SD_DEFAULT)) != defaultFilter)
                    {
                        continue;
                    }
                    else if (list == SERVERS_MANAGED)
                    {
                        /*
                        ** Skip it this is not a managed server.
                        */
                        if ((sdd->attrib & MASK) != MANAGED)
                        {
                            continue;
                        }
                    }
                    else if (list == SERVERS_UNMANAGED)
                    {
                        /*
                        ** Skip it this is not an unmanaged server.
                        */
                        if ((sdd->attrib & MASK) != UNMANAGED)
                        {
                            continue;
                        }
                    }
                    else if (list == SERVERS_XIO)
                    {
                        /*
                        ** Skip it this is not a XIOtech controller.
                        */
                        if ((sdd->attrib & (1<<SD_XIO)) == 0)
                        {
                            continue;
                        }
                    }

                    /*
                    ** Check if the count exceeds that amount of data that
                    ** is allowed to be returned.
                    */
                    if (count >= maxCount)
                    {
                        /*
                        ** Set the return status to "too much data".
                        */
                        retStatus = DETOOMUCHDATA;
                    }
                    else if (size == 8*sizeof(UINT16))
                    {
                        /*
                        ** Store the SID, TID, WWN in the return packet.
                        */
                        pList[0] = sdd->sid;
                        pList[1] = sdd->tid;
                        pList[2] = (UINT16) sdd->wwn;
                        pList[3] = (UINT16) (sdd->wwn >> 16);
                        pList[4] = (UINT16) (sdd->wwn >> 32);
                        pList[5] = (UINT16) (sdd->wwn >> 48);
                        pList[6] = 0;
                        pList[7] = 0;

                        /*
                        ** Increment list pointer.
                        */
                        pList += 8;
                    }
                    else
                    {
                        /*
                        ** Store the Server ID in the return packet.
                        */
                        pList[count] = sdd->sid;
                    }

                    /*
                    ** Increment the server count.
                    */
                    ++count;
                }
            }
        }
    }
    else if (list == SERVERS_ACTIVE || list == SERVERS_LOGON ||
             list == SERVERS_W_STATS)
    {
        /*
        ** Allocate and clear an array to store a indicator
        ** of which servers exist on this target.
        */
        idMapped = s_MallocC(MAX_SERVERS, __FILE__, __LINE__);

        /*
        ** Check if the request is for all targets or for
        ** just a single target.
        */
        if (targetId == ALL)
        {
            /*
            ** Prep return value for no targets found.
            */
            retStatus = DEINVTID;

            /*
            ** Check if a target exists.
            */
            for (i = 0; i < MAX_TARGETS; ++i)
            {
                /*
                ** Check that this target is valid.
                */
                if (T_tgdindx[i] != NULL &&
                    T_tgdindx[i]->owner == K_ficb->cSerial)
                {
                    /*
                    ** A target was found.
                    */
                    retStatus = DEOK;
                    break;
                }
            }

            /*
            ** Check for server with stats request.
            */
            if (list == SERVERS_W_STATS)
            {
                /*
                ** Get the servers for the all targets
                */
                DEF_GetServersWithStatsList(ALL, idMapped);
            }
            else
            {
                /*
                ** Process all existing targets
                */
                for (i = 0; i < MAX_TARGETS; ++i)
                {
                    /*
                    ** Check that this target is valid.
                    */
                    if (T_tgdindx[i] != NULL &&
                        T_tgdindx[i]->owner == K_ficb->cSerial)
                    {
                        if (list == SERVERS_LOGON)
                        {
                            /*
                            ** Get the logged on servers for the specified target
                            */
                            ISP_GetServersLogonList(i, idMapped);
                        }
                        else
                        {
                            /*
                            ** Get the active servers for the specified target
                            */
                            DEF_GetServersActiveList(i, idMapped);
                        }
                    }
                }
            }
        }
        else if (T_tgdindx[targetId]->owner != K_ficb->cSerial)
        {
            /*
            ** The target ID is not on this controller.
            */
            retStatus = DEINVTID;
        }
        else if (list == SERVERS_LOGON)
        {
            /*
            ** Get the logged on servers for the specified target
            */
            ISP_GetServersLogonList(targetId, idMapped);
        }
        else if (list == SERVERS_ACTIVE)
        {
            /*
            ** Get the active servers for the specified target
            */
            DEF_GetServersActiveList(targetId, idMapped);
        }
        else
        {
            /*
            ** Get the active servers for the specified target
            */
            DEF_GetServersWithStatsList(targetId, idMapped);
        }

        /*
        ** Give another process a chance.
        */
        TaskSwitch();

        /*
        ** Get the list of Servers for this target.
        */
        for (i = mrtr->sid; i < MAX_SERVERS; ++i)
        {
            /*
            ** The flag is set if the Vdisk is
            ** associated with this target.
            */
            if (idMapped[i] == TRUE)
            {
                /*
                ** Check if the count exceeds that amount of data that
                ** is allowed to be returned.
                */
                if (count >= maxCount)
                {
                    /*
                    ** Set the return status to "too much data".
                    */
                    retStatus = DETOOMUCHDATA;
                }
                else if (size == 8*sizeof(UINT16))
                {
                    /*
                    ** Get a pointer to the next server record.
                    */
                    sdd = S_sddindx[i];

                    /*
                    ** Store the SID, TID, WWN in the return packet.
                    */
                    pList[0] = i;
                    pList[1] = sdd->tid;
                    pList[2] = (UINT16) sdd->wwn;
                    pList[3] = (UINT16) (sdd->wwn >> 16);
                    pList[4] = (UINT16) (sdd->wwn >> 32);
                    pList[5] = (UINT16) (sdd->wwn >> 48);
                    pList[6] = 0;
                    pList[7] = 0;

                    /*
                    ** Increment list pointer.
                    */
                    pList += 8;
                }
                else
                {
                    /*
                    ** Store the Server ID in the return packet.
                    */
                    pList[count] = i;
                }

                /*
                ** Increment the server count.
                */
                ++count;
            }
        }

        /*
        ** Release the idMapped.
        */
        s_Free(idMapped, MAX_SERVERS, __FILE__, __LINE__);
    }
    else if (list == VDISKS || list == VDISKS_CACHEN ||
             list == VDISKS_CACHDIS)
    {
        /*
        ** Determine if vdisks are filtered based on cache enabled status.
        */
        if (list == VDISKS_CACHEN)
        {
            /*
            ** Skip vdisk that are not cached.
            */
            cacheFilter = (1 << VC_CACHED);
        }
        else if (list == VDISKS_CACHDIS)
        {
            /*
            ** Skip vdisk that are cached.
            */
            cacheFilter = 0;
        }

        if (mrtr->sid >= MAX_VIRTUAL_DISKS)
        {
            /*
            ** Invalid Virtual Device ID.
            */
            retStatus = DEINVVID;
        }
        else
        {
            /*
            ** Allocate and clear an array to store a indicator
            ** of which virtual device are owned by this target.
            */
            idMapped = s_MallocC(MAX_VIRTUAL_DISKS, __FILE__, __LINE__);

            /*
            ** Prep return value for no targets found.
            */
            retStatus = DEINVTID;

            /*
            ** Get the list of servers for this target.
            */
            for (i = 0; i < MAX_SERVERS; ++i)
            {
                /*
                ** Get a pointer to the next server record.
                */
                sdd = S_sddindx[i];

                /*
                ** Check if this server is associated with
                ** the specified target.
                */
                if (sdd != 0 && (sdd->tid == targetId || targetId == ALL))
                {
                    /*
                    ** Check that this target is valid.
                    */
                    if (targetId == ALL &&
                       (T_tgdindx[sdd->tid] == NULL ||
                        T_tgdindx[sdd->tid]->owner != K_ficb->cSerial))
                    {
                        continue;
                    }

                    /*
                    ** A target was found.
                    */
                    retStatus = DEOK;

                    /*
                    ** Get the list of vdisks for this server.
                    */
                    lvmPtr = sdd->lvm;
                    ilvmPtr = sdd->ilvm;

                    while((lvmPtr != NULL) || (ilvmPtr != NULL))
                    {
                        /*
                        ** Get the list of all Vdisks mapped to this
                        ** server and set the indicator in the idMapped
                        ** for each Vdisk.
                        */

                        /*
                        ** Set the pointer for the one we will examine in
                        ** this iteration of the loop.  Also move the ptr
                        ** onto the next one for the list we used.
                        */
                        if(lvmPtr != NULL)
                        {
                            lvm = lvmPtr;
                            lvmPtr = lvmPtr->nlvm;
                        }
                        else
                        {
                            lvm = ilvmPtr;
                            ilvmPtr = ilvmPtr->nlvm;
                        }

                        /*
                        ** Get the vdd pointer for the Vdisk.
                        */
                        vcd = vcdIndex[lvm->vid];

                        /*
                        ** Check if this is a valid Vdisk ID.
                        ** Check if either return all Vdisk
                        ** or just those with caching enabled.
                        */
                        if (lvm->vid < MAX_VIRTUAL_DISKS && vcd != NULL &&
                            (cacheFilter == 0xFFFF ||
                             (vcd->stat & (1 << VC_CACHED)) == cacheFilter))
                        {
                            /*
                            ** This Vdisk is mapped to a server
                            ** on this target, set the flag.
                            */
                            idMapped[lvm->vid] = TRUE;
                        }
                    }
                }
            }

            /*
            ** Give another process a chance.
            */
            TaskSwitch();

            /*
            ** Get the list of Vdisks for this target.
            */
            for (i = mrtr->sid; i < MAX_VIRTUAL_DISKS; ++i)
            {
                /*
                ** The flag is set if the Vdisk is
                ** associated with this target.
                */
                if (idMapped[i] == TRUE)
                {
                    /*
                    ** Check if the count exceeds that amount of data that
                    ** is allowed to be returned.
                    */
                    if (count >= maxCount)
                    {
                        /*
                        ** Set the return status to "too much data".
                        */
                        retStatus = DETOOMUCHDATA;
                    }
                    else
                    {
                        /*
                        ** Store the Vdisk ID in the return packet.
                        */
                        pList[count] = i;
                    }

                    /*
                    ** Increment the vdisk count.
                    */
                    ++count;
                }
            }

            /*
            ** Release the idMapped.
            */
            s_Free(idMapped, MAX_VIRTUAL_DISKS, __FILE__, __LINE__);
        }
    }
    else if (list == VDISK_UNMAPPED)
    {
        if (mrtr->sid >= MAX_VIRTUAL_DISKS)
        {
            /*
            ** Invalid Virtual Device ID.
            */
            retStatus = DEINVVID;
        }
        else
        {
            /*
            ** Allocate and clear an array to store a indicator
            ** of which virtual device are owned by this target.
            */
            idMapped = s_MallocC(MAX_VIRTUAL_DISKS, __FILE__, __LINE__);

            /*
            ** Get the list of servers for this target.
            */
            for (i = 0; i < MAX_SERVERS; ++i)
            {
                /*
                ** Get a pointer to the next server record
                */
                sdd = S_sddindx[i];

                /*
                ** Check if this server is associated with
                ** the specified target.
                */
                if (sdd != 0)
                {
                    /*
                    ** Get the list of vdisks for this server.
                    */
                    lvmPtr = sdd->lvm;
                    ilvmPtr = sdd->ilvm;

                    while((lvmPtr != NULL) || (ilvmPtr != NULL))
                    {
                        /*
                        ** Get the list of all Vdisks mapped to this
                        ** server and set the indicator in the idMapped
                        ** for each Vdisk.
                        */

                        /*
                        ** Set the pointer for the one we will examine in
                        ** this iteration of the loop.  Also move the ptr
                        ** onto the next one for the list we used.
                        */
                        if(lvmPtr != NULL)
                        {
                            lvm = lvmPtr;
                            lvmPtr = lvmPtr->nlvm;
                        }
                        else
                        {
                            lvm = ilvmPtr;
                            ilvmPtr = ilvmPtr->nlvm;
                        }

                        /*
                        ** Check if this is a valid Vdisk ID.
                        ** Check if either return all Vdisk
                        ** or just those with caching enabled.
                        */
                        if (lvm->vid < MAX_VIRTUAL_DISKS)
                        {
                            /*
                            ** This Vdisk is mapped to a server
                            ** on this target, set the flag.
                            */
                            idMapped[lvm->vid] = TRUE;
                        }
                    }
                }
            }

            /*
            ** Give another process a chance.
            */
            TaskSwitch();

            /*
            ** Get the list of Vdisks for this target.
            */
            for (i = mrtr->sid; i < MAX_VIRTUAL_DISKS; ++i)
            {
                /*
                ** The flag is not set if the VDisk is unassociated.
                */
                if (vcdIndex[i] != NULL && idMapped[i] == FALSE)
                {
                    /*
                    ** Check if the count exceeds that amount of data that
                    ** is allowed to be returned.
                    */
                    if (count >= maxCount)
                    {
                        /*
                        ** Set the return status to "too much data".
                        */
                        retStatus = DETOOMUCHDATA;
                    }
                    else
                    {
                        /*
                        ** Store the Vdisk ID in the return packet.
                        */
                        pList[count] = i;
                    }

                    /*
                    ** Increment the vdisk count.
                    */
                    ++count;
                }
            }

            /*
            ** Release the idMapped.
            */
            s_Free(idMapped, MAX_VIRTUAL_DISKS, __FILE__, __LINE__);
        }
    }
    else if (list == LUNMAP || list == LUNMAP_CACHEN ||
             list == LUNMAP_CACHDIS || list == LUNMAP_DEFAULT)
    {
        /*
        ** Since more data per resource is sent back for the mapping,
        ** the maximum count of resources needs to be recalculated.
        */
        size = 4*sizeof(UINT16);
        maxCount = ((UINT16) pMRP->rspLen - sizeof(MR_RSP_PKT)) / size;

        /*
        ** Determine how to filter vdisks mappings.
        */
        if (list == LUNMAP_CACHDIS)
        {
            /*
            ** Skip vdisk mappings that are cached.
            */
            cacheFilter = 0;
        }
        else if (list == LUNMAP_CACHEN)
        {
            /*
            ** Skip vdisk mappings that are not cached.
            */
            cacheFilter = (1 << VC_CACHED);
        }
        else if (list == LUNMAP_DEFAULT)
        {
            /*
            ** Skip vdisk mappings that are not the default mapping.
            */
            defaultFilter = (1<<SD_DEFAULT);
        }

        if (mrtr->sid >= MAX_SERVERS)
        {
            /*
            ** Invalid Server ID.
            */
            retStatus = DEINVSID;
        }
        else
        {
            /*
            ** Check if reporting all targets
            */
            if (targetId == ALL)
            {
                /*
                ** Set the starting and end for all targets.
                */
                start = 0;
                end = MAX_TARGETS-1;

                /*
                ** Prep return value for no targets found.
                */
                retStatus = DEINVTID;
            }
            else
            {
                /*
                ** Set the starting and end for one specific target.
                */
                start = targetId;
                end = targetId;
            }

            /*
            ** Scan the specified targets
            */
            for (targetId = start; targetId <= end; targetId++)
            {
                /*
                ** Check that this target is valid.
                */
                if (start != end &&
                    (T_tgdindx[targetId] == NULL ||
                     T_tgdindx[targetId]->owner != K_ficb->cSerial))
                {
                    continue;
                }

                /*
                ** A target was found.
                */
                retStatus = DEOK;

                /*
                ** Find the default mappings.
                */
                defaultMapping = DEF_WWNLookup(0, targetId, TRUE, NULL);

                /*
                ** Get the list of servers for this target.
                */
                for (i = mrtr->sid; i < MAX_SERVERS; ++i)
                {
                    /*
                    ** Get a pointer to the next server record.
                    */
                    sdd = S_sddindx[i];

                    /*
                    ** Check if this server is associated with
                    ** the specified target.
                    */
                    if (sdd != 0 && (sdd->tid == targetId || targetId == ALL))
                    {
                        /*
                        ** Check if this server has the correct attributes.
                        */
                        if ((sdd->attrib & (1<<SD_DEFAULT)) != defaultFilter)
                        {
                            continue;
                        }

                        /*
                        ** Check if this server is using the
                        ** default LUN mapping.
                        */
                        if ((sdd->attrib & UNMANAGED) != 0)
                        {
                            /*
                            ** If a default mapping was not found,
                            ** skip this server.
                            */
                            if (defaultMapping > MAX_SERVERS)
                            {
                                continue;
                            }

                            /*
                            ** Use the sdd for the default mapping.
                            */
                            sdd = S_sddindx[defaultMapping];
                        }

                        /*
                        ** Get the list of vdisks for this server.
                        */
                        lvmPtr = sdd->lvm;
                        ilvmPtr = sdd->ilvm;

                        while((lvmPtr != NULL) || (ilvmPtr != NULL))
                        {
                            /*
                            ** Get the list of all Vdisks mapped to this
                            ** server and set the indicator in the idMapped
                            ** for each Vdisk.
                            */

                            /*
                            ** Set the pointer for the one we will examine in
                            ** this iteration of the loop.  Also move the ptr
                            ** onto the next one for the list we used.
                            */
                            if(lvmPtr != NULL)
                            {
                                lvm = lvmPtr;
                                lvmPtr = lvmPtr->nlvm;
                            }
                            else
                            {
                                lvm = ilvmPtr;
                                ilvmPtr = ilvmPtr->nlvm;
                            }

                            /*
                            ** Get the vdd pointer for the Vdisk.
                            */
                            vcd = vcdIndex[lvm->vid];

                            /*
                            ** Check if this is a valid Vdisk ID.
                            ** Check if either return all Vdisk
                            ** or just those with caching enabled.
                            */
                            if (lvm->vid < MAX_VIRTUAL_DISKS && vcd != NULL &&
                                (cacheFilter == 0xFFFF ||
                                 (vcd->stat & (1 << VC_CACHED)) == cacheFilter))
                            {
                                if (count >= maxCount)
                                {
                                    /*
                                    ** Set return status to "too much data".
                                    */
                                    retStatus = DETOOMUCHDATA;
                                }
                                else
                                {
                                    /*
                                    ** Store Target ID, Server ID
                                    ** LUN, and Vdisk ID.
                                    */
                                    pList[0] = sdd->tid;
                                    pList[1] = i;
                                    pList[2] = lvm->lun;
                                    pList[3] = lvm->vid;

                                    /*
                                    ** Increment list pointer.
                                    */
                                    pList += 4;
                                }

                                /*
                                ** Increment resource count
                                */
                                ++count;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        /*
        ** The option requested is not valid.
        */
        retStatus = DEINVOPT;
    }

    /*
    ** Store the number of devices found in the return packet.
    */
    pMRP->pRsp->nDevs = count;
    pMRP->pRsp->size  = size;
    pMRP->pRsp->status = retStatus;

    return retStatus;
}

/**
******************************************************************************
**
**  @brief      Retrieve the configuration data for a target.
**
**              This function will dump the configuration data for a target
**              record.
**              Assumes all space needed for return data is available.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_GetTargetInfo(MR_PKT* pMRP)
{
    UINT16         targetId;
    TGD           *target;
    UINT32         port;
    struct TAR    *pTar;
    MRGETTARG_REQ *mrgt;                /* Pointer to input MRP */
    MRGETTARG_RSP *mrgto;               /* Pointer to output MRP */

    /* Get pointer to Parm block address */
    mrgt = (MRGETTARG_REQ *) pMRP->pReq;

    /* Get list option and target ID from the input packet. */
    targetId = mrgt->id;
    mrgto = (MRGETTARG_RSP *) pMRP->pRsp;

    /* Set the return length */
    mrgto->header.len = sizeof (MRGETTARG_RSP);

    /* Check if target exists. */
    if (targetId >= MAX_TARGETS || T_tgdindx[targetId] == NULL)
    {
        /* Invalid target ID. */
        return DEINVTID;
    }

    /* Get a pointer to the target record. */
    target = T_tgdindx[targetId];

    /* Copy fields from target record to output MRP */
    mrgto->tid     = target->tid;
    mrgto->opt     = target->opt;
    mrgto->fcid    = target->fcid;
    mrgto->lock    = target->lock;
    mrgto->pname   = target->portName;
    mrgto->powner  = target->prefOwner;
    mrgto->owner   = target->owner;
    mrgto->cluster = target->cluster;
    mrgto->pport   = target->prefPort;
    mrgto->aport   = target->altPort;

    /* Indicate port information is not available */
    mrgto->port = 0xE0;

    /* Examine each port and search for this target. */
    for (port = 0; port < MAX_PORTS && mrgto->port == 0xE0; ++port)
    {
        /* Get pointer to TARget record. */
        pTar = tar[port];

        /* Traverse the target list for this port. */
        while (pTar != NULL)
        {
            /* Check for this target number and the target being enabled. */
            if (pTar->tid == targetId &&
               (pTar->opt & (1 << TARGET_ENABLE)) != 0)
            {
                mrgto->port = port;
                break;
            }
            /* Advance to next TARget record. */
            pTar = pTar->fthd;
        }
    }

    /* Was the port information found? */
    if (mrgto->port == 0xE0)
    {
        /* Is the target owned by this controller? */
        if (target->owner != K_ficb->cSerial)
        {
            /* Indicate port information is on another controller. */
            mrgto->port = 0xF0 | (target->owner & 0xF);
        }
        else if (isprev[target->prefPort] == NULL && isprev[target->altPort] == NULL)
        {
            /* Indicate no ports exist for this target. */
            mrgto->port = 0xE1;
        }
    }

    /* Check if this target is not clustered to any other targets. */
    if (target->cluster >= MAX_TARGETS || T_tgdindx[target->cluster] == NULL)
    {
        /* The node name for this target is used. */
#if defined(MODEL_3000) || defined(MODEL_7400)
        if (BIT_TEST(iscsimap, target->port))
        {
            if (T_tgdindx[target->tid | 0x02])
            {
                mrgto->nname = T_tgdindx[target->tid | 0x02]->nodeName;
            }
            else
            {
                fprintf(stderr, "FE %s:%u target->tid=%d, but T_tgdindx[%d] pointer is NULL\n",
                        __func__, __LINE__, target->tid, target->tid | 0x02);
                mrgto->nname = target->nodeName;
            }
        }
        else
#endif /* MODEL_3000 || MODEL_7400 */
        {
            mrgto->nname = target->nodeName;
        }
    }
    else
    {
        /* The node name for the clustered target is used. */
#if defined(MODEL_7000) || defined(MODEL_4700)
        if(BIT_TEST(iscsimap, target->port))
        {
            mrgto->nname = T_tgdindx[target->tid]->nodeName;
        }
        else
#endif /* MODEL_7000 || MODEL_4700 */
        {
            mrgto->nname = T_tgdindx[target->cluster]->nodeName;
        }
    }

    /* Good completion status. */
    return DEOK;
}

/**
******************************************************************************
**
**  @brief      Receives User Response from the CCB.
**
**              This function reports the user response to the request
**              to complete cache initialization.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_ResumeCacheInit(MR_PKT* pMRP)
{
    /*
    ** Set the return data length.
    */
    pMRP->pRsp->rspLen = pMRP->rspLen;

    return WC_resumeCacheInit(((MRRESUMECACHE_REQ *) pMRP->pReq)->userResp);
}

#if ISCSI_CODE
/**
******************************************************************************
**
**  @brief      Get Server Info
**
**  @param      pMRP  - pointer to the input MRP from the CCB
**
**  @return     return status
**
******************************************************************************
**/
static UINT8 DEF_GetSData(MR_PKT* pMRP)
{
    register UINT16    id;
    register UINT8     retCode = DEOK;
    SDD               *pSDD;
    MRGETSINFO_RSP    *pRsp = (MRGETSINFO_RSP *)((MRGETSINFO_REQ*)pMRP->pRsp);
    struct LVM        *lvm;
    UINT32             i = 0;

    ((MRSETWSINFO_RSP*)pMRP->pRsp)->header.len = sizeof(MRSETWSINFO_RSP);

    id = ((MRGETSINFO_REQ *)pMRP->pReq)->id;

    pSDD = S_sddindx[id];

    if (id >= MAX_SERVERS || pSDD == NULL || T_tgdindx[pSDD->tid] == NULL)
    {
        /* Invalid SID */
        retCode = DEINVSID;
    }
    else
    {
        /* Copy SDD structure leaving lvm and ilvm pointers */
        memcpy(&(pRsp->sid),pSDD,sizeof(SDD)-8);

        if (((T_tgdindx[pSDD->tid]->opt) & 0x80) == 0)
        {
            /* FC */
            hex2a(bswap_64(pSDD->wwn), pRsp->i_name);
        }

        for (i = 0, lvm = pSDD->lvm; lvm != NULL; lvm = lvm->nlvm, i++)
        {
            pRsp->lunMap[i].lun = lvm->lun;
            pRsp->lunMap[i].vid = lvm->vid;
        }

        for (lvm = pSDD->ilvm; lvm != NULL; lvm = lvm->nlvm, i++)
        {
            pRsp->lunMap[i].lun = lvm->lun;
            pRsp->lunMap[i].vid = lvm->vid;
        }
    }

    pRsp->header.len = sizeof(MRGETSINFO_RSP) + (i * sizeof(MRGETSINFO_RSP_LM));

    return(retCode);
}
#endif  /* ISCSI_CODE */

/**
******************************************************************************
**
**  @brief      Retrieve the statistic for a specific server.
**
**              This function will dump the configuration and statistical data
**              from the server definition.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_ServerStats(MR_PKT* pMRP)
{
    IMT*               imt = NULL;
    UINT32             sid;
    SDD*               sdd;
    MRGETSSTATS_REQ*   mgs;
    MRGETSSTATS_RSP*   mrgs;
    UINT8              retStatus = DEOK;
    TGD*               p_target;

    /*
    ** CJN
    ** These variables are used when the option in the request packet is
    ** something other than zero.  Any option other than 0 is not currently
    ** available so these variables are unused.
    */
#if 0
    UINT32             tid;
    UINT8              port = 0xFF;
#endif  /* 0 */

    /*
    ** Get pointer to Parm block address
    */
    mgs = (MRGETSSTATS_REQ *) pMRP->pReq;
    mrgs = (MRGETSSTATS_RSP *) pMRP->pRsp;

    /*
    ** Get Server ID from input packet
    */
    sid = mgs->sid;
    sdd = S_sddindx[sid];

    /*
    ** Set length in return packet.
    */
    mrgs->header.len = sizeof(MR_HDR_RSP);

    if (pMRP->rspLen < sizeof(MRGETSSTATS_RSP))
    {
        /*
        ** Set return status to "too much data".
        */
        retStatus = DETOOMUCHDATA;
    }
    /*
    ** Check if Server ID is valid.
    */
    else if (sid > MAX_SERVERS || sdd == NULL)
    {
        retStatus = DEINVSID;
    }
    /*
    ** Check if this is a "default" server mapping.
    */
    else if ((sdd->attrib & (1<<SD_DEFAULT)) != 0)
    {
        /*
        ** Statistics are not available for "default" servers.
        */
        retStatus = DEINVSID;
    }
    else
    {
        if (mgs->option == 0)
        {
            /*
            ** Get the first IMT.
            */
            imt = mag_imt_head;

            if ((sdd->attrib & UNMANAGED) == 0)
            {
                /*
                ** Scan the allocated IMT chain for a matching server ID
                */
                while (imt != NULL)
                {
                    /*
                    ** Check for a matching Server ID
                    */
                    if (imt->sid == sid)
                    {
                        break;
                    }

                    /*
                    ** Increment to the next imt.
                    */
                    imt = imt->link2;
                }
            }
            else
            {
                /*
                ** Walk the IMT linked list.
                */
                while (imt != NULL)
                {
                    /*
                    ** Check for a matching WWN and target number
                    */
                    if (imt->mac == sdd->wwn && imt->tid == sdd->tid)
                    {
                        break;
                    }

                    /*
                    ** Increment to the next imt.
                    */
                    imt = imt->link2;
                }
            }
        }

        /*
        ** CJN
        ** Any option other than 0 is not currently available.
        */
#if 0
        else
        {
            if (mgs->option == SS_TARGETS)
            {
                /*
                ** Get Target ID from input packet.
                */
                tid = mgs->tid;

                if (tid > MAX_TARGETS)
                {
                    /*
                    ** Invalid target ID.
                    */
                    retStatus = DEINVTID;
                }
                /*
                ** Is the target that this server is association with
                ** located on this controller?
                */
                else if (T_tgdindx[tid] != NULL &&
                    T_tgdindx[tid]->owner == K_ficb->cSerial)
                {
                    /*
                    ** Find the port number for this target.
                    */
                    port = ispPortAssignment[tid];
                }
                else
                {
                    port = 0xFF;
                }
            }
            else if (mgs->option == SS_PORTS)
            {
                /*
                ** Get Target ID from Server record.
                */
                port = mgs->port;
                tid = 0xFF;
            }

            if (retStatus == DEOK)
            {
                /*
                ** Is the port valid?  The port not being valid
                ** indicates the target associated with this
                ** server is not assigned to this controller.
                */
                if (port >= MAX_PORTS)
                {
                    /*
                    ** Invalid port.
                    */
                    retStatus = DEINVCHAN;
                }
                else
                {
                    /*
                    ** Get the first IMT.
                    */
                    imt = cimtDir[port]->imtHead;

                    if ((sdd->attrib & UNMANAGED) == 0)
                    {
                        /*
                        ** Walk the IMT linked list.
                        */
                        while (imt != NULL)
                        {
                            /*
                            ** Check for a matching SID
                            */
                            if (imt->sid == sid)
                            {
                                break;
                            }

                            /*
                            ** Increment to the next imt.
                            */
                            imt = imt->link;
                        }
                    }
                    else
                    {
                        /*
                        ** Walk the IMT linked list.
                        */
                        while (imt != NULL)
                        {
                            /*
                            ** Check for a matching WWN and target number
                            */
                            if (imt->mac == sdd->wwn &&
                               (tid = 0xFF || imt->tid == tid))
                            {
                                break;
                            }

                            /*
                            ** Increment to the next imt.
                            */
                            imt = imt->link;
                        }
                    }
                }
            }
        }
#endif  /* 0 */

        /*
        ** Was the IMT for the server found?
        */
        if (imt != NULL)
        {
            memcpy(&mrgs->ag_cmds, &imt->agg,
                        sizeof(MRGETSSTATS_RSP) - sizeof(MR_HDR_RSP));

            /*
            ** Set length in return packet.
            */
            mrgs->header.len = sizeof(MRGETSSTATS_RSP);
        }
        else if (retStatus == DEOK)
        {
            if(sdd != NULL && T_tgdindx[sdd->tid] != NULL)
            {
                p_target = T_tgdindx[sdd->tid];
                if(p_target->owner != K_ficb->cSerial)
                {
                    retStatus = DEINVSID;
                }
                else
                {
                    /*
                    ** Fill the packet with zeroes and send back along with
                    ** good status
                    */
                    memset(&mrgs->ag_cmds, 0,
                        sizeof(MRGETSSTATS_RSP) - sizeof(MR_HDR_RSP));
                    mrgs->header.len = sizeof(MRGETSSTATS_RSP);
                }

            }
            else
            {
                /*
                ** Statistics are not available for offline servers.
                */
                retStatus = DEINVSID;
            }
        }
    }

    return retStatus;
}

/**
******************************************************************************
**
**  @brief      Fails or Unfail the specified port(s).
**
**              This function marks a port as failed or not failed.
**
**              This function marks the specified port as failed.
**              Any target currently assigned to this port are
**              removed from this port and if a alternate
**              port is available, the targets are moved
**              to the alternate port.
**
**              This also marks the specified port as not failed.
**              Any target that have this port the preferred port
**              are moved back to this port.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_FailPort(MR_PKT* pMRP)
{
    UINT8  retStatus = DEOK;
    UINT8  port;
    UINT32  i;
    TGD *   tgd;
    MRFAILPORT_REQ * mfp;
    UINT32 portFound = FALSE;

    /*
    ** Set the return data length.
    */
    pMRP->pRsp->rspLen = pMRP->rspLen;

    /*
    ** Get pointer to Parm block address
    */
    mfp = (MRFAILPORT_REQ *) pMRP->pReq;

    /*
    ** Get the port number from the input packet.
    */
    port = mfp->port;

    /*
    ** Check of the port is valid.
    */
    if (port >= MAX_PORTS)
    {
        /*
        ** Invalid port.
        */
        retStatus = DEINVCHAN;
    }

    /*
    ** Is the port being failed?
    */
    else if (mfp->fail == TRUE)
    {
        /*
        ** Check if the specified port is not already failed.
        */
        if (ispFailedPort[port] == FALSE)
        {
            /*
            ** Indicate the port is failed.
            */
            ispFailedPort[port] = TRUE;

            for (i = 0; i < MAX_TARGETS; ++i)
            {
                /*
                ** Point to the target record.
                */
                tgd = T_tgdindx[i];

                /*
                ** Check if the target is owned by this controller
                ** and if the primary port is being failed.
                */
                if (tgd != NULL && tgd->owner == K_ficb->cSerial &&
                    tgd->prefPort == port &&
                    ispFailedPort[tgd->altPort] == FALSE &&
                    isprev[tgd->altPort] != NULL &&
                    (ispfail & (1<<tgd->altPort)) == 0)
                {
                    /*
                    ** The target will be moved to the alternate port.
                    */
                    portFound = TRUE;
                }
            }

            /*
            ** Is no alternate port available?
            */
            if (portFound == FALSE)
            {
                /*
                ** No Spare port available.
                */
                retStatus = DENOPORT;
            }
        }
    }
    else
    {
        /*
        ** Check if the specified port is marked as failed.
        */
        if (ispFailedPort[port] != FALSE)
        {
            /*
            ** Indicate the port is not failed.
            */
            ispFailedPort[port] = FALSE;

            for (i = 0; i < MAX_TARGETS; ++i)
            {
                /*
                ** Point to the target record.
                */
                tgd = T_tgdindx[i];
#if FE_ICL

                if(tgd && (ICL_TARGET(tgd->tid)) && iclPortExists)
                {
                    /*
                    ** Ignore ICL target, as we don't handle anything here.
                    */
                    continue;
                }
#endif  /* FE_ICL */

                /*
                ** Check if the target is owned by this controller
                ** Also, check for a target who primary port is failed,
                ** but the alternate port is currently being unfailed.
                */
                if (tgd != NULL && tgd->owner == K_ficb->cSerial)
                {
                    /*
                    ** Check if the primary port is being unfailed.
                    */
                    if (tgd->prefPort == port ||
                        (ispFailedPort[tgd->prefPort] != FALSE &&
                        tgd->altPort == port))
                    {
                        portFound = TRUE;
                    }
                }
            }

            /*
            ** Was a target to be moved found?
            */
            if (portFound == FALSE)
            {
                /*
                ** No target to move found.
                */
                retStatus = DENOTARGET;
            }
        }
        else if (tar[port] == NULL || tar[port]->tid >= MAX_TARGETS)
        {
            /*
            ** Either the tar record does not exist of the
            ** tar port is configured with a control port.
            ** This port has no targets.
            */
            retStatus = DENOTARGET;
        }
    }

    /*
    ** Reset any port as a result of the change in port status.
    */
    ISP_FindPortToReset((UINT32)(mfp->fail == FALSE || (mfp->option & 1) == 0));

    return retStatus;
}

/**
******************************************************************************
**
**  @brief      Generic MRP for debug.
**
**              This function is for debug only.
**              There is no code in this function - it is provided as a
**              spot to put debug code.
**
**  @param      pMRP - MRP structure
**
**  @return     DEOK (always)
**
******************************************************************************
**/
UINT8 DEF_Generic(MR_PKT* pMRP UNUSED)
{
    return DEOK;
}

/**
******************************************************************************
**
**  @brief      Gets the battery status, returns MM board status,
**              sets up the MM board into shipping mode.
**
**              This function returns battery status from MM card
**              when CCB polls for battery and board status info
**              from the front end processor. Also can execute a
**              command to place the MM board into low power shipping
**              mode.
**
**  @param      pMRP - MRP structure
**
**  @return     return status returned by MM_ProcessMRP.
**
******************************************************************************
**/
UINT8 DEF_MicroMemory(MR_PKT* pMRP)
{
    /* Get pointer to Parm block address */
    MRMMCARDGETBATTERYSTATUS_REQ *pMM;
    MRMMCARDGETBATTERYSTATUS_RSP *pMMRsp;
    unsigned int command;
    UINT8 rc;

    pMM  = (MRMMCARDGETBATTERYSTATUS_REQ *) pMRP->pReq;
    pMMRsp  = (MRMMCARDGETBATTERYSTATUS_RSP *) pMRP->pRsp;

    /* First, grab the return data address and length allowed. */
    pMMRsp->header.len = pMRP->rspLen;

    /* Get command code from input packet. */
    if (pMRP->rspLen < sizeof(MRMMCARDGETBATTERYSTATUS_RSP))
    {
        /* Set return status to "too much data". */
        return (DETOOMUCHDATA);
    }
    command = pMM->commandCode;

    rc = MM_ProcessMRP(command, pMMRsp);

#if defined(MODEL_4700) || defined(MODEL_7000)
    if (rc == DEINOPDEV || (rc == DEOK && pMMRsp->boardInfo.boardStatus == NV_STS_NO_BOARD))
    {
        /* Fake up response for GUI. */
        pMMRsp->boardInfo.boardStatus      = 0x00000001;
        pMMRsp->boardInfo.revision.major   = 0x00000063;      /* Make it be 63 hex, 99 decimal */
        pMMRsp->boardInfo.revision.minor   = 0x00000063;      /* Make it be 63 hex, 99 decimal */
        pMMRsp->boardInfo.memorySize       = 0x10000000;      /* 256mb MM board. */
        pMMRsp->boardInfo.memoryErrorCount = 0x00000000;
        pMMRsp->boardInfo.batteryCount     = 0x00000002;
        pMMRsp->boardInfo.batteryInformation[0].chargePercent = 0x03e8;
        pMMRsp->boardInfo.batteryInformation[0].status        = 0x00000001;
        pMMRsp->boardInfo.batteryInformation[0].voltage       = 0x1067;
        pMMRsp->boardInfo.batteryInformation[1].chargePercent = 0x03e8;
        pMMRsp->boardInfo.batteryInformation[1].status        = 0x00000001;
        pMMRsp->boardInfo.batteryInformation[1].voltage       = 0x1067;
        pMMRsp->boardInfo.batteryInformation[2].chargePercent = 0x0000;
        pMMRsp->boardInfo.batteryInformation[2].status        = 0x00000000;
        pMMRsp->boardInfo.batteryInformation[2].voltage       = 0x0000;
        pMMRsp->boardInfo.batteryInformation[3].chargePercent = 0x0000;
        pMMRsp->boardInfo.batteryInformation[3].status        = 0x00000000;
        pMMRsp->boardInfo.batteryInformation[3].voltage       = 0x0000;
        rc = DEOK;
    }
#endif  /* MODEL_4700 || MODEL_7000 */
    return(rc);
}

/**
******************************************************************************
**
**  @brief      Retrieve the statistics for an HBA.
**
**              This function will return statistical data for an HBA
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_GetHBAStats(MR_PKT* pMRP)
{
    UINT8  retStatus = DEOK;
    UINT8  port;
    MR_DEVID_REQ * mghs;
    MRGETHABSTATS_RSP * mrghs;

    /*
    ** Get pointer to Parm block address
    */
    mghs = (MR_DEVID_REQ *) pMRP->pReq;
    mrghs = (MRGETHABSTATS_RSP *) pMRP->pRsp;

    /*
    ** Get Port Number from input packet
    */
    port = mghs->id;

    /*
    ** Set length in return packet.
    */
    mrghs->header.len = sizeof(MRGETHABSTATS_RSP);

    if (pMRP->rspLen < sizeof(MRGETHABSTATS_RSP))
    {
        /*
        ** Set return status to "too much data".
        */
        retStatus = DETOOMUCHDATA;
    }
    /*
    ** Check if Port Number is valid.
    */
    else if (port >= MAX_PORTS)
    {
        retStatus = DEINVCHAN;
    }
    else
    {
        /* Generate a bit mask for this port and check against ispmap */
        if (ispmap & (1 << port))
        {
            CopyHBAStats( port, mrghs);
        }
        else
        {
            retStatus = DEINVCHAN;
        }
    }

    return retStatus;
}  /* DEF_HBAStats */


/**
******************************************************************************
**
**  @brief      Submit a request to the MM Test driver.
**
**  @param      pMRP - MRP structure
**
**  @return     return status returned by MM_TestMRP.
**
******************************************************************************
**/
UINT8 DEF_MMTest(MR_PKT* pMRP)
{
    UINT8           retStatus = DEOK;
    MRMMTEST_REQ*   pReq;
    MRMMTEST_RSP*   pRsp;

    pReq = (MRMMTEST_REQ*) pMRP->pReq;
    pRsp = (MRMMTEST_RSP*) pMRP->pRsp;

    /*
    ** First, grab the return data address and length allowed.
    */
    pRsp->header.len = sizeof(MRMMTEST_RSP);

    if (pMRP->rspLen < sizeof(MRMMTEST_RSP))
    {
        /*
        ** Set return status to "too much data".
        */
        retStatus = DETOOMUCHDATA;
    }
    else
    {
            if (pReq->option == MMTEST_ECC_SINGLE ||
                pReq->option == MMTEST_ECC_MULTI ||
                pReq->option == MMTEST_FAIL ||
                pReq->option == MMTEST_WCSIG ||
                pReq->option == MMTEST_WCSIG_SN ||
                pReq->option == MMTEST_WCSIG_SEQNO)
            {
                MM_TestTaskStart(pReq);
            }
            else
            {
                retStatus = DEINVOPT;
            }

            pRsp->header.status = retStatus;
            pRsp->header.len = sizeof(MRMMTEST_RSP);
    }

    return retStatus;
}

/**
******************************************************************************
**
**  @brief      Clear persistent reserve data for a given VID.
**
**  @param      pMRP - MRP structure
**
**  @return     return status GOOD.
**
******************************************************************************
**/
UINT8 DEF_PRClr(MR_PKT* pMRP)
{
    MRPRCLR_REQ*   pReq;
    MRPRCLR_RSP*   pRsp;

    pReq = (MRPRCLR_REQ*) pMRP->pReq;
    pRsp = (MRPRCLR_RSP*) pMRP->pRsp;

    pRsp->header.status = pr_cfgClear(pReq->id);
    pRsp->header.len = sizeof(MRPRCLR_RSP);

    return pRsp->header.status;
}

/**
******************************************************************************
**
**  @brief      Set Foreign Target (on/off)
**
**  @param      pMRP - MRP structure
**
**  @return     return status GOOD.
**
******************************************************************************
**/
static UINT8 DEF_SetFT(MR_PKT* pMRP)
{
    MRSFT_REQ*   pReq;
    MRSFT_RSP*   pRsp;

    pReq = (MRSFT_REQ*) pMRP->pReq;
    pRsp = (MRSFT_RSP*) pMRP->pRsp;

    FT_DEFINED = pReq->option;
fprintf(stderr, "DEF_SetFT: Set FT_DEFINED value to %d\n", FT_DEFINED);
    pRsp->header.status = DEOK;
    pRsp->header.len = sizeof(MRSFT_RSP);

    return pRsp->header.status;
}

/**
******************************************************************************
**
**  @brief      persistent reserve config update completion from CCB
**
**  @param      pMRP - MRP structure
**
**  @return     return status GOOD.
**
******************************************************************************
**/
UINT8 DEF_PRCfgComp(MR_PKT* pMRP)
{
    UINT8                     retStatus;
    MRPRCONFIGCOMPLETE_REQ*   pReq;
    MRPRCONFIGCOMPLETE_RSP*   pRsp;

    pReq = (MRPRCONFIGCOMPLETE_REQ*) pMRP->pReq;
    pRsp = (MRPRCONFIGCOMPLETE_RSP*) pMRP->pRsp;

    /*
    ** Free the memory allocated in the original request
    */
    if((pReq->dataSize > 0) && (pReq->data != NULL))
    {
        s_Free(pReq->data, pReq->dataSize, __FILE__, __LINE__);
    }
    if (pMRP->rspLen < sizeof(MRPRCONFIGCOMPLETE_RSP))
    {
        /*
        ** Set return status to "too much data".
        */
        retStatus = DETOOMUCHDATA;
    }
    else
    {
        retStatus = pr_cfgcomp(pReq->vid, pReq->sid, pReq->rc);
    }
    pRsp->header.status = retStatus;
    pRsp->header.len = sizeof(MRPRCONFIGCOMPLETE_RSP);

    return retStatus;
}

/**
******************************************************************************
**
**  @brief      Get persistent reserve data for a given VID.
**
**  @param      pMRP - MRP structure
**
**  @return     return status good or bad.
**
******************************************************************************
**/
UINT8 DEF_PRGet(MR_PKT* pMRP)
{
    UINT8           retStatus;
    MRPRGET_REQ*   pReq;
    MRPRGET_RSP*   pRsp;

    pReq = (MRPRGET_REQ*) pMRP->pReq;
    pRsp = (MRPRGET_RSP*) pMRP->pRsp;

    //TODO
    #define get_tsc()       ({ unsigned long long __scr; \
            __asm__ __volatile__("rdtsc" : "=A" (__scr)); __scr;})
    long long tsc;
    UINT32 tsc_h;
    tsc = get_tsc();
    tsc_h = (UINT32)(tsc >> 32);


    /*
    ** First, grab the return data address and length allowed.
    */
    pRsp->header.len = sizeof(MRPRGET_RSP);

    if (pMRP->rspLen < sizeof(MRPRGET_RSP))
    {
        /*
        ** Set return status to "too much data".
        */
        retStatus = DETOOMUCHDATA;
    }
    else
    {
        retStatus = get_pr_data(pReq->id,pRsp);
    }
    pRsp->header.status = retStatus;

    return retStatus;
}

/**
******************************************************************************
**
**  @brief      Update persistent reserve data in the FE structs
**
**  @param      pMRP - MRP structure
**
**  @return     return status GOOD.
**
******************************************************************************
**/
UINT8 DEF_UpdPRR(MR_PKT* pMRP)
{
    MRUPDPRR_REQ*   pReq;
    MRUPDPRR_RSP*   pRsp;

    pReq = (MRUPDPRR_REQ*) pMRP->pReq;
    pRsp = (MRUPDPRR_RSP*) pMRP->pRsp;

    pRsp->header.status = pr_cfgChange(pReq->vid, pReq->sid, pReq->flags);
    pRsp->header.len = sizeof(MRUPDPRR_RSP);

    return pRsp->header.status;
}

/**
******************************************************************************
**
**  @brief      To provide a standard means of configuring data
**              for a target.
**
**              This function will take the information in the input parameters
**              and either create, delete or update a target record.
**
**              If the WWNs are zero, then the target will be deleted.
**
**              On the front end, there may not be a record for the target,
**              so a target record will have to be created and recorded in
**              the table.  This will happen when a create is done on the
**              BEP and reflected forward to the FEP.
**
**  @param      pMRP - MRP structure
**
**  @return     UINT8 status
**
******************************************************************************
**/
UINT8 DEF_ConfigTar(MR_PKT* pMRP)
{
    UINT8 retVal = DEOK;
    TGD *pTGD;
    MRCONFIGTARG_REQ*   pReq = NULL;
//    MRCONFIGTARG_RSP*   pRsp = NULL;

    pReq = (MRCONFIGTARG_REQ*) pMRP->pReq;
//    pRsp = (MRCONFIGTARG_RSP*) pMRP->pRsp;

    if(pReq->tid > MAX_TARGETS)
    {
        retVal = DEINVTID;
    }
    else if((pTGD = gTDX.tgd[pReq->tid]) == NULL)
    {
        if(pReq->nname != 0x0)
        {
            /*
            ** Create a new Target
            */
            pTGD = DEF_AllocTarg();
            memcpy(pTGD, pReq, sizeof(TGD));
            gTDX.tgd[pReq->tid] = pTGD;
            gTDX.count++;
        }
    }
    else if(pReq->nname == 0x0)
    {
        /*
        ** Delete the Target
        */
        s_Free(pTGD, sizeof(TGD), __FILE__, __LINE__);
        gTDX.tgd[pReq->tid] = NULL;
        gTDX.count--;
    }
    else
    {
        /*
        ** Update the Target Configuration
        */
        memcpy(pTGD, pReq, sizeof(TGD));
#if FE_ISCSI_CODE
        if(BIT_TEST(pTGD->opt, TARGET_ISCSI))
        {
            TAR *pTar;
            UINT8 port;

            port =  ispPortAssignment[pReq->tid];

            if((port < MAX_PORTS) && (ispFailedPort[port] == FALSE))
            {
                for(pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
                {
                    if(pTar->tid == pReq->tid)
                    {
                        if((pTar->ipPrefix != pTGD->ipPrefix)
                                || (pTar->ipAddr != pTGD->ipAddr)
                                || (pTar->ipGw != pTGD->ipGw))
                        {
                            /*
                            ** Check if the iSCSI default files exists
                            ** Just delete these files. These are unnecessary
                            */
                            fsl_ResetPort(port, ISP_RESET_AND_INIT);
                        }
                        break;
                    }
                }
            }
            /*
            ** For any target update, invoke the iSNS update. It will
            ** take care of checking to see if target params changed and
            ** if yes, will update the iSNS server with new information
            */
            iSNS_Update();
        }
#endif  /* FE_ISCSI_CODE */
    }
    fsl_updatePaths();
    return retVal;
}

/**
******************************************************************************
**
**  @brief      To provide a standard means of accepting and processing a
**              server configuration request.
**
**              This function will parse a server configuration request MRP
**              and appropriately configure a SDD.
**
**  @param      pMRP - MRP structure
**
**  @return     UINT8 status
**
******************************************************************************
**/
UINT8 DEF_ConfigServer(MR_PKT *pMRP)
{
    UINT32 i = 0;
    SDD *pSDD = NULL;
    MRLVM *pMRL = NULL;
    MRREPORTSCONFIG_REQ *pReq;
//    MRREPORTSCONFIG_RSP *pRsp;

    /*
    ** First, grab the params & return data address
    */
    pReq = (MRREPORTSCONFIG_REQ*) pMRP->pReq;
//    pRsp = (MRREPORTSCONFIG_RSP*) pMRP->pRsp;

    /*
    ** First, check for a deletion.
    */
    if(pReq->del == TRUE)
    {
        /*
        ** Delete Operations: if SDD exists, free it and the associated
        ** LVM structures
        */
        if((pSDD = gSDX.sdd[pReq->sid]) != NULL)
        {
            gSDX.sdd[pReq->sid] = NULL;
            DEF_RelSDDLVM(pSDD);
            gSDX.count--;
        }
    }
    else
    {
        /*
        ** Add Operation: delete any old SDD and allocate a new one, fill it in,
        ** and then go about processing the LUN records.
        */
        if((pSDD = gSDX.sdd[pReq->sid]) != NULL)
        {
            DEF_RelSDDLVM(pSDD);
            gSDX.sdd[pReq->sid] = NULL;
            gSDX.count--;
        }
        /*
        ** Allocate a server structure
        */
        gSDX.sdd[pReq->sid] = pSDD = DEF_AllocServer();
        gSDX.count++;

        pSDD->sid = pReq->sid;
        pSDD->linkedSID = pReq->lsid;
        pSDD->tid = pReq->tid;
        pSDD->status = pReq->status;
        pSDD->pri = pReq->pri;
        pSDD->attrib = pReq->attrib;
        pSDD->owner = pReq->owner;
        pSDD->wwn = pReq->wwn;

        /*
        ** Copy the alias name & the iSCSI name
        */
        memcpy(pSDD->name, pReq->name, 16);

        if (T_tgdindx[pSDD->tid] == 0 || ((T_tgdindx[pSDD->tid]->opt) & 0x80) == 0)
        {
            /* FC */
            memset(pSDD->i_name, 0, 256);
        }
        else
        {
            memcpy(pSDD->i_name, pReq->i_name, 254);
        }

        /*
        ** Process the LUN records if any
        */
        pMRL = (MRLVM *)(pReq + 1);
        for(i = 0; i < pReq->nluns; i++)
        {
            DEF_HashLUN(pSDD, (UINT32)pMRL->lun, (UINT32)pMRL->vid);
            pMRL = pMRL + 1;
        }
    }

    return (DEOK);
}

/**
******************************************************************************
**
**  @brief      To provide a standard means of processing the get port type
**              request from the BE
**
**  @param      mrp - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_GetPortType(MR_PKT* pMRP)
{
    UINT8  retCode = DEOK;
    MRGETPORTTYPE_REQ *pReq;
    MRGETPORTTYPE_RSP *pRsp;

    /*
    ** Get pointer to Parm block address
    */
    pReq = (MRGETPORTTYPE_REQ *)pMRP->pReq;
    pRsp = (MRGETPORTTYPE_RSP *)pMRP->pRsp;

    if((BIT_TEST(ispmap, pReq->pport)) && (BIT_TEST(ispmap, pReq->aport)))
    {
        if((BIT_TEST(iscsimap, pReq->pport)) !=
                    (BIT_TEST(iscsimap, pReq->aport)))
        {
            pRsp->type = PT_INVAL;
        }
        else if((BIT_TEST(iscsimap, pReq->pport)) &&
                    (BIT_TEST(iscsimap, pReq->aport)))
        {
            pRsp->type = PT_ISCSI;
        }
        else
        {
            pRsp->type = PT_FC;
        }
    }
    else if(BIT_TEST(ispmap, pReq->pport))
    {
        pRsp->type = (BIT_TEST(iscsimap, pReq->pport)) ? PT_ISCSI : PT_FC;
    }
    else if(BIT_TEST(ispmap, pReq->aport))
    {
        pRsp->type = (BIT_TEST(iscsimap, pReq->aport)) ? PT_ISCSI : PT_FC;
    }
    else
    {
        /*
        ** Both the ports are missing
        */
        retCode = DEFAILED;
    }

    return (retCode);
}   /* DEF_GetPortType */

/**
******************************************************************************
**
**  @brief      Decode the function code to the appropriate function.
**
**              The function code is extracted from the MRP and
**              used to determine which function to execute.
**
**  @param      pMRP - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_CBridge(MR_PKT* pMRP)
{
    UINT8 retStatus = DEINVPKTTYP; /* Return value, prep status    */

    /* Check if the MRP was from the CCB. */
    if ((pMRP->function & 0xFF00) == MREFFUNCBASE)
    {
        /* Decode the function */
        switch (pMRP->function)
        {
            case MRRESETFEPORT:
                retStatus = DI_ResetPort(pMRP);
                break;

            case MRFELOOP:
                retStatus = DI_PortStats(pMRP);
                break;

#if ISCSI_CODE
            case MRGETSINFO:
                retStatus = DEF_GetSData(pMRP);
                break;

            case MRGETSESSIONS:
                retStatus = DEF_GetSessions(pMRP);
                break;

            case MRGETSESSIONSPERSERVER:
                retStatus = DEF_GetSessionsOnServer(pMRP);
                break;

            case MRGETIDDINFO:
                retStatus = DEF_GetIDDInfo(pMRP);
                break;

            case MRDLMPATHSTATS:
                retStatus = ICL_GetDlmPathStats(pMRP);
                break;

            case MRDLMPATHSELECTIONALGO:
                retStatus = ICL_DlmPathSelectionAlgorithm(pMRP);
                break;
#endif  /* ISCSI_CODE */

            case MRGETSSTATS:
                retStatus = DEF_ServerStats(pMRP);
                break;

            case MRFEGETDVLIST:
                retStatus = DI_GetDeviceList(pMRP);
                break;

            case MRRESUMECACHE:
                retStatus = DEF_ResumeCacheInit(pMRP);
                break;

            case MRGETTRLIST:
                retStatus = DEF_GetTargetResource(pMRP);
                break;

            case MRFEGETPORTLIST:
                retStatus = DI_GetPortList(pMRP);
                break;

            case MRFEPORTNOTIFY:
                retStatus = DI_SetPortEventNotification(pMRP);
                break;

            case MRFEGENERIC:
                retStatus = DEF_Generic(pMRP);
                break;

            case MRFELOOPPRIMITIVE:
                retStatus = DI_LoopPrimitive(pMRP);
                break;

            case MRGETTARG:
                retStatus = DEF_GetTargetInfo(pMRP);
                break;

            case MRFAILPORT:
                retStatus = DEF_FailPort(pMRP);
                break;

            case MRGETHABSTATS:
                retStatus = DEF_GetHBAStats(pMRP);
                break;

            case MRMMCARDGETBATTERYSTATUS:  /*  Get MM status  */
                retStatus = DEF_MicroMemory(pMRP);
                break;

            case MRGETMPCONFIGFE:       /* Get resync rec and config */
               retStatus = MP_GetMPConfigFE(pMRP);
                break;

            case MRFEPORTGO:                /* Put Regular Port on FE Fabric */
                retStatus = DI_FEPortGo(pMRP);
                break;

            case MRSETTDISCACHE:           /* Set Temp Disable Write Cache */
                retStatus = CA_SetTempDisableWC(pMRP);
                break;

            case MRCLRTDISCACHE:           /* Clear Temp Disable Write Cache */
                retStatus = CA_ClearTempDisableWC(pMRP);
                break;

            case MRQTDISABLEDONE:         /* Query WC Temp Disable Flush  */
                retStatus = CA_QueryTDisableDone(pMRP);
                break;

            case MRMMTEST:
                retStatus = DEF_MMTest(pMRP);
                break;

            case MRPRGET:
                retStatus = DEF_PRGet(pMRP);
                break;

            case MRPRCLR:
                retStatus = DEF_PRClr(pMRP);
                break;

            case MRPRCONFIGCOMPLETE:
                retStatus =  DEF_PRCfgComp(pMRP);
                break;

            case MRUPDPRR:
                retStatus =  DEF_UpdPRR(pMRP);
                break;

            case MRSETFEPORTCONFIG:
                retStatus = DI_SetPortConfig(pMRP);
                break;

            default:
                fprintf(stderr, "deffe.c: Unknown MRP %d\n", pMRP->function);
        }
    }
    else if ((pMRP->function & 0xFF00) == MRBFFUNCBASE)
    {
        /* Decode the function */
        switch (pMRP->function)
        {
            case MRREPORTTARG:
                retStatus = DEF_ConfigTar(pMRP);
                break;

            case MRREPORTSCONFIG:
                retStatus = DEF_ConfigServer(pMRP);
                break;

            case MRUPDTGINFO:
                retStatus = DEF_UpdateTgInfo(pMRP);
                break;

            case MRGETPORTTYPE:
                retStatus = DEF_GetPortType(pMRP);
                break;

            case MRSETCHAPFE:
                retStatus = DEF_SetChapInfo(pMRP);
                break;

            case MRSETISNSINFOFE:
                retStatus = DEF_iSNSConfig(pMRP);
                break;

            case MRSETFT:
                retStatus = DEF_SetFT(pMRP);
                break;

            default:
                fprintf(stderr, "deffe.c: Unknown MRP %d\n", pMRP->function);
        }
    }

    return retStatus;
}



/*----------------------------------------------------------------------
  HBA Stats tracking code

  This is the C code used to keep track of HBA stats.
  There may be a better home for this code.

----------------------------------------------------------------------*/

/* This is what I should have done:
typedef struct HBA_STATS
{
    UINT64 aggRdCmds;              * total # of read commands     <l> *
    UINT64 aggWrCmds;              * total # of write commands    <l> *
    UINT32 curRdCmds;              * current read cmd count       <w> *
    UINT32 curWrCmds;              * current write cmd count      <w> *
    UINT32 perRdCmds;              * periodic read cmd count      <w> *
    UINT32 perWrCmds;              * periodic write cmd count     <w> *
    UINT32 curRdBlks;              * current read block count     <w> *
    UINT32 curWrBlks;              * current write block count    <w> *
    UINT32 perRdBlks;              * periodic read block count    <w> *
    UINT32 perWrBlks;              * periodic write block count   <w> *
} HBA_STATS;

HBA_STATS  hbaStats[MAX_PORTS];

But the following definitions were easier to interface with in the asm code.
If it becomes necessary, some of the following arrays that get accessed
from asm code can be moved to internal RAM to improve performance.

*/

UINT64 hbaAggRdCmds[MAX_PORTS];      /* track aggregate read stats here */
UINT64 hbaAggWrCmds[MAX_PORTS];      /* track aggregate write stats here */
UINT64 hbaPerCmds[MAX_PORTS];        /* track periodic (R+W) stats here */
UINT32 hbaPerRdCmds[MAX_PORTS];      /* track periodic read stats here */
UINT32 hbaPerWrCmds[MAX_PORTS];      /* track periodic write stats here */
UINT64 hbaPerWrBlocks[MAX_PORTS];    /* track periodic write block count */
UINT64 hbaPerRdBlocks[MAX_PORTS];    /* track periodic read block count */
UINT64 hbaPerBlockCount[MAX_PORTS];  /* used to calcualte avg request size */

/*
There used to be two functions:
void UpdateHBAWriteStats( UINT32 hbaNum) to increment hba write counts
and
void UpdateHBAReadStats( UINT32 hbaNum) to increment hba read counts

The increments are now done in assembly in magdrvr.as in the read_com
and write_com  sections.
*/

HBA_PERF_STATS  totalHBAStats;

/**
******************************************************************************
**
**  @brief      Calculate the periodic statistics for all HBAs.
**
**              This function will be called once a second from cd$exec in
**              cdriver.asm.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void CalcHBAStats(void);

void CalcHBAStats(void)
{
    INT32 i;

    /*
    ** Initialize the total periodic HBA stats
    */
    totalHBAStats.activeHBAs = 0;
    totalHBAStats.perQDepth = 0;
    totalHBAStats.perRdCmds = 0;
    totalHBAStats.perWrCmds = 0;
    totalHBAStats.perRdBlocks = 0;
    totalHBAStats.perWrBlocks = 0;


    for ( i = 0; i < MAX_PORTS; i++)
    {
        /*
        ** If there were periodic reads or writes, call the HBA active and
        ** bump the total periodic counts and the aggregrate counts.
        */
        if (hbaPerRdCmds[i] || hbaPerWrCmds[i])
        {
            ++totalHBAStats.activeHBAs;

            totalHBAStats.perRdCmds += hbaPerRdCmds[i];
            totalHBAStats.perRdBlocks += hbaPerRdBlocks[i];
            totalHBAStats.perWrCmds += hbaPerWrCmds[i];
            totalHBAStats.perWrBlocks += hbaPerWrBlocks[i];

            /*
            ** Aggregate Read and Write commands
            */
            hbaAggRdCmds[i] += hbaPerRdCmds[i];
            hbaAggWrCmds[i] += hbaPerWrCmds[i];

        }

        /*
        ** calculate hba ops count since last call
        */
        hbaPerCmds[i] = hbaPerWrCmds[i] + hbaPerRdCmds[i];

        /*
        ** restart hba read and write counts
        */
        hbaPerWrCmds[i] = 0;
        hbaPerRdCmds[i] = 0;

        /*
        ** save block count to use for average block size calculation
        */
        hbaPerBlockCount[i] = hbaPerWrBlocks[i] + hbaPerRdBlocks[i];

        /*
        ** restart the block counts
        */
        hbaPerWrBlocks[i] = 0;
        hbaPerRdBlocks[i] = 0;

        /*
        ** Bump the total HBA qdepth and available HBA
        */
        if (hba_q_cnt[i])
        {
            totalHBAStats.perQDepth += hba_q_cnt[i];
        }
    }




    /*
    ** Update the Write cache performacne bypass flag
    */
    WC_ComputeWCBypassState(&totalHBAStats);

}  /* CalcHBAStats */

/**
******************************************************************************
**
**  @brief      Copy the stats for an HBA into a response message.
**
**  @param      hbaNum - HBA for which to return stats
**  @param      mrghs - response message to copy stats into
**
**  @return     none
**
**  @attention  hbaNum gets checked for validity in the calling function
**
******************************************************************************
**/

void CopyHBAStats( UINT32 hbaNum, MRGETHABSTATS_RSP * mrghs)
{
    /* hbaNum gets checked for validity in the calling function */
    mrghs->perCmds = hbaPerCmds[hbaNum];
    /*mrghs->qdepth = ISP_GetFWQueueDepth( hbaNum);*/
    mrghs->qdepth = hba_q_cnt[hbaNum];
    mrghs->writeReqs = hbaAggWrCmds[hbaNum];
    mrghs->readReqs  = hbaAggRdCmds[hbaNum];
    /* calculate average request size */
    if ( mrghs->perCmds > 0 && hbaPerBlockCount[hbaNum] > 0)
    {
        mrghs->avgReqSize = hbaPerBlockCount[hbaNum] / mrghs->perCmds;
    }
    else
    {
        mrghs->avgReqSize = 0;
    }
}  /* CopyHBAStats */

/**
******************************************************************************
**
**  @brief      Initialize the structures used to track statistics for all HBAs.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void InitHBAStats(void);

void InitHBAStats(void)
{
    INT32 i;
    for ( i = 0; i < MAX_PORTS; i++)
    {
        hbaAggRdCmds[i] = 0;
        hbaAggWrCmds[i] = 0;
        hbaPerCmds[i] = 0;
        hbaPerRdCmds[i] = 0;
        hbaPerWrCmds[i] = 0;
        hbaPerBlockCount[i] = 0;
        hbaPerWrBlocks[i] = 0;
        hbaPerRdBlocks[i] = 0;
    }
}  /* InitHBAStats */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
