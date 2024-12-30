/* $Id: quorum.c 147854 2010-09-20 14:49:35Z m4 $ */
/*============================================================================
**  FILE NAME:      quorum.c
**  MODULE TITLE:   quorum implementation
**
**  DESCRIPTION:  The quorum manager is responsible for two main functions:
**                  1. Providing an interface to allow access to quorum area
**                     data
**                  2. Providing a mechanism to transfer and monitor packets
**                     through the quorum communications area, both for a slave
**                     controller and the master controller.
**
** Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "quorum.h"

#include "crc32.h"
#include "debug_files.h"
#include "misc.h"
#include "nvram.h"
#include "quorum_utils.h"
#include "XIO_Std.h"
#include "XIO_Types.h"

#ifdef WIN32
#include "my_stubs.h"
#include ".\testing\file_io.h"
#else   /* WIN32 */
#include "FIO.h"
#endif  /* WIN32 */

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
QM_CONTROLLER_CONFIG_MAP cntlConfigMap LOCATE_IN_SHMEM; /* Controller configuration array */
QM_MASTER_CONFIG masterConfig LOCATE_IN_SHMEM;  /* Master Configuration record */
FAILURE_DATA_STATE cachedControllerFailureState = FD_STATE_UNUSED;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static UINT32 GetCommSectorOffset(UINT16 commSlot);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: ReadMasterConfiguration
**
**  Description:
**      This function allow access to the master configuration area.
**      The master configuration sector is read into a buffer. The master
**      configuration structure is then copied from the buffer to the location
**      provided.
**
**  Inputs:   configPtr - pointer location to store the master configuration
**                        record
**
**  Returns:  0  = good completion
**            !0 = file i/o error
**--------------------------------------------------------------------------*/
INT32 ReadMasterConfiguration(QM_MASTER_CONFIG *configPtr)
{
    INT32       rc = 0;

    /*
     * Ensure the we got good inputs, if not return now.
     */
    if (configPtr != NULL)
    {
        rc = ReadFile(FS_FID_QM_MASTER_CONFIG, configPtr, sizeof(*configPtr));
    }

    return rc;
}


/*----------------------------------------------------------------------------
**  Function Name: WriteMasterConfiguration
**
**  Description:
**      This function allows modifying the master configuration area in the
**      quorum. On writes, the master configuration is copied to a sector
**      buffer and then written to the quorum area.
**
**  Inputs:   configPtr - pointer location to retrieve the master configuration
**                        record
**            fid       - the file system ID to write
**
**  Returns:  0  = good completion
**            !0 = file i/o error
**--------------------------------------------------------------------------*/
void WriteMasterConfiguration(QM_MASTER_CONFIG *configPtr, INT32 fid)
{
    /*
     * If we are not the master controller, we are not permitted to update
     * this portion of the quorum. Return now, with out modifying.
     */
    if (!TestforMaster(GetMyControllerSN()))
    {
        dprintf(DPRINTF_QUORUM, "**Quorum Update attempted by non master -- Master Config\n");
        return;
    }

    /* Ensure the we got good inputs. */
    if (configPtr != NULL)
    {
        dprintf(DPRINTF_QUORUM, "**Writing Master Config, SCHEMA = %u\n",
                configPtr->schema);

        (void)WriteFile(fid, configPtr, sizeof(*configPtr));
    }
}


/*----------------------------------------------------------------------------
**  Function Name: ReadControllerMap
**
**  Description:
**      This function allow access to the Controller Map  area.
**      The Controller map sectors are read into a buffer. The controller
**      map structure is then copied from the buffer to the location
**      provided.
**
**  Inputs:   mapPtr - pointer location to store the controller map record
**
**  Returns:  0  = good completion
**            !0 = file i/o error
**--------------------------------------------------------------------------*/
INT32 ReadControllerMap(QM_CONTROLLER_CONFIG_MAP *mapPtr)
{
    INT32       rc = 0;

    /*
     * Ensure the we got good inputs, if not return now.
     */
    if (mapPtr != NULL)
    {
        rc = ReadFile(FS_FID_QM_CONTROLLER_MAP, mapPtr, sizeof(*mapPtr));
    }

    return rc;
}


/*----------------------------------------------------------------------------
**  Function Name: WriteControllerMap
**
**  Description:
**      This function allow access to the Controller Map  area.
**      On writes, the controller map is copied to an 8 sector
**      buffer and then written to the quorum area.
**
**  Inputs:   mapPtr - pointer location to retrieve the controller map record
**
**  Returns:  0  = good completion
**            !0 = file i/o error
**--------------------------------------------------------------------------*/
INT32 WriteControllerMap(QM_CONTROLLER_CONFIG_MAP *mapPtr)
{
    INT32       rc = 0;

    /*
     * If we are not the master controller, we are not permitted to update
     * this portion of the quorum. Return now, with out modifying.
     */
    if (!TestforMaster(GetMyControllerSN()))
    {
        dprintf(DPRINTF_QUORUM, "**Quorum Update attempted by non master -- Controller Map\n");

        return rc;
    }

    /*
     * Ensure the we got good inputs, if not return now.
     */
    if (mapPtr != NULL)
    {
        dprintf(DPRINTF_QUORUM, "**Writing Controller Map\n");

        rc = WriteFile(FS_FID_QM_CONTROLLER_MAP, mapPtr, sizeof(*mapPtr));
    }

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    SetControllerMapAddresses
**
** Description: Set the controllers ip address,
**              subnet mask, and gateway address in the controller map
**
** Inputs:      UINT32 sn           - serial number of controller to change
**              UINT32 ip           - Ip address to set
**              UINT32 subnet       - Subnet mask address to set
**              UINT32 gateway      - Gateway address to set
**              UINT8  bChangeIP    - Reason for calling this function.
**
**                                    true means that the IP address of
**                                    this controller is being changed.
**
**                                    false meanse that the IP address
**                                    not being changed.
**
**                                    If the IP address is being changed
**                                    the actual and new network settings
**                                    are updated while only the new
**                                    network settings are updated in the
**                                    other cases.
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
UINT8 SetControllerMapAddresses(UINT32 sn, UINT32 ip, UINT32 subnet, UINT32 gateway, bool bChangeIP)
{
    UINT8       rc = GOOD;
    UINT32      cont = 0;

    /*
     * Load the Controller Map.
     */
    if (LoadControllerMap() != 0)
    {
        dprintf(DPRINTF_DEFAULT, "SetControllerMapAddresses: Failed to load controller config map\n");

        rc = ERROR;
    }

    if (rc == GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "SetControllerMapAddresses: Looking for controller\n");

        /*
         * Find the contoller being modified in the controller config map
         */
        cont = GetCommunicationsSlot(sn);

        /*
         * If we could not find ourselves set the error
         */
        if (cont == MAX_CONTROLLERS)
        {
            dprintf(DPRINTF_DEFAULT, "SetControllerMapAddresses: Failed to find controller\n");

            rc = ERROR;
        }
    }

    if (rc == GOOD)
    {
        /*
         * Set the new IP Address.
         */
        if (ip != 0)
        {
            /*
             * Set the ip address in the controller config map
             */
            cntlConfigMap.cntlConfigInfo[cont].newIpEthernetAddress = ip;
        }

        /*
         * Set the new Subnet Mask
         */
        if (subnet != 0)
        {
            /*
             * Set the subnet mask address in the controller config map
             */
            cntlConfigMap.cntlConfigInfo[cont].newSubnetMask = subnet;
        }

        /*
         * Set the new Gateway Address.
         */
        if (gateway != 0)
        {
            /*
             * Set the gateway address in the controller config map
             */
            cntlConfigMap.cntlConfigInfo[cont].newGatewayAddress = gateway;
        }

        if (bChangeIP)
        {
            cntlConfigMap.cntlConfigInfo[cont].ipEthernetAddress = cntlConfigMap.cntlConfigInfo[cont].newIpEthernetAddress;

            cntlConfigMap.cntlConfigInfo[cont].subnetMask = cntlConfigMap.cntlConfigInfo[cont].newSubnetMask;

            cntlConfigMap.cntlConfigInfo[cont].gatewayAddress = cntlConfigMap.cntlConfigInfo[cont].newGatewayAddress;
        }

        if (WriteControllerMap(&cntlConfigMap) != 0)
        {
            rc = ERROR;
        }
    }

    return (rc);
}


/*----------------------------------------------------------------------------
**  Function Name: WriteElectionData
**
**  Description:
**      This function allow writes to the election data sector.
**      The election data is copied to a sector buffera and then the buffer
**      is written to the election sector in the quorum.
**
**  Inputs: controllerSN - controller s/n of election sector to read
**          electionPtr - pointer location to store the election record
**
**  Returns:  0  = good completion
**            !0 = file i/o error
**--------------------------------------------------------------------------*/
INT32 WriteElectionData(UINT32 controllerSN, QM_ELECTION_DATA *electionPtr)
{
    INT32       rc = 0;
    UINT32      offset;
    UINT8       slot;

    /*
     * Determine the position of the conntroller in the controller map, to
     * determime it's communications slot
     */
    slot = GetCommunicationsSlot(controllerSN);

    /*
     * Determine the Sector offset within the communications file based on the
     * controller slot position and the sector requested. The offset is 1 based
     * since the file header is at offset 0.
     */
    offset = GetCommSectorOffset(slot) + (offsetof(QM_CONTROLLER_COMM_AREA,
                                                   electionStateSector) / BYTES_PER_SECTOR);

    /*
     * Ensure the we got good inputs, if not return now.
     */
    if (electionPtr != NULL)
    {
        dprintf(DPRINTF_QUORUM, "**Writing Election Data, SLOT = %u\n", slot);

        rc = WriteFileAtOffset(FS_FID_QM_COMM_AREA, offset, electionPtr,
                               sizeof(*electionPtr));
    }

    return rc;
}


/*----------------------------------------------------------------------------
**  Function Name: ReadFailureData
**
**  Description:
**      This function allow access to the failure data record.
**      The failure sector is read into a buffer. The failure data
**      structure is then copied from the buffer to the location
**      provided.
**
**  Inputs: controllerSN - controller s/n of election sector to read
**          failurePtr - pointer location to store the failure state record
**
**  Returns:  0  = good completion
**            !0 = file i/o error
**--------------------------------------------------------------------------*/
INT32 ReadFailureData(UINT32 controllerSN, QM_FAILURE_DATA *failurePtr)
{
    INT32       rc;
    UINT32      offset;
    UINT8       slot;

    /* Ensure the we got good pointer input, if not return now. */
    if (failurePtr == NULL)
    {
        return 0;
    }

    /*
     * Determine the position of the conntroller in the controller map, to
     * determime it's communications slot
     */
    slot = GetCommunicationsSlot(controllerSN);

    /*
     * Determine the Sector offset within the communications file based on the
     * controller slot position and the sector requested. The offset is 1 based
     * since the file header is at offset 0.
     */
    offset = GetCommSectorOffset(slot) +
             (offsetof(QM_CONTROLLER_COMM_AREA, failStateSector) / BYTES_PER_SECTOR);

    rc = ReadFileAtOffset(FS_FID_QM_COMM_AREA, offset, failurePtr, sizeof(*failurePtr));

    /* Update the cached failure state for this controller */
    if (rc == 0 && controllerSN == GetMyControllerSN())
    {
        SetControllerFailureState(failurePtr->state);
    }

    return rc;
}


/*----------------------------------------------------------------------------
**  Function Name: WriteFailureData
**
**  Description:
**      This function allow writes to the failure data sector.
**      The failure data is copied to a sector buffera and then the buffer
**      is written to the failure state sector in the quorum.
**
**  Inputs: controllerSN - controller s/n of election sector to read
**          failurePtr - pointer location to store the failure state record
**
**  Returns:  0  = good completion
**            !0 = file i/o error
**--------------------------------------------------------------------------*/
INT32 WriteFailureData(UINT32 controllerSN, QM_FAILURE_DATA *failurePtr)
{
    INT32       rc = 0;
    UINT32      offset;
    UINT8       slot;

    /*
     * Determine the position of the conntroller in the controller map, to
     * determime it's communications slot
     */
    slot = GetCommunicationsSlot(controllerSN);

    /*
     * Determine the Sector offset within the communications file based on the
     * controller slot position and the sector requested. The offset is 1 based
     * since the file header is at offset 0.
     */
    offset = GetCommSectorOffset(slot) + (offsetof(QM_CONTROLLER_COMM_AREA,
                                                   failStateSector) / BYTES_PER_SECTOR);

    /*
     * Ensure the we got good inputs, if not return now.
     */
    if (failurePtr != NULL)
    {
        dprintf(DPRINTF_QUORUM, "**Writing Failure Data, SLOT = %u\n", slot);

        rc = WriteFileAtOffset(FS_FID_QM_COMM_AREA, offset, failurePtr,
                               sizeof(*failurePtr));

        /*
         * Update the cached failure state for this controller
         */
        if (rc == 0 && controllerSN == GetMyControllerSN())
        {
            SetControllerFailureState(failurePtr->state);
        }
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      This function writes the failure data state value for
**              a given controller.
**
**  @param      UINT32 controllerSN - Controller Serial Number
**              UINT32 state - State for the controller
**
**  @return     0 = good completion, !0 = file i/o error
**
******************************************************************************
**/
INT32 WriteFailureDataState(UINT32 controllerSN, UINT32 state)
{
    QM_FAILURE_DATA *qmFailureData;
    INT32       rc;

    qmFailureData = MallocSharedWC(sizeof(*qmFailureData));
    memset(qmFailureData, 0x00, sizeof(*qmFailureData));
    qmFailureData->state = state;

    rc = WriteFailureData(controllerSN, qmFailureData);
    Free(qmFailureData);
    return rc;
}


/*----------------------------------------------------------------------------
**  Function Name: ReadAllCommunications
**
**  Description:
**      This function reads the entire quorum communications area for all
**      controllers
**
**  Inputs: commPtr - pointer location to store the communications sectors
**
**  Returns:  0  = good completion
**            !0 = file i/o error
**--------------------------------------------------------------------------*/
INT32 ReadAllCommunications(QM_CONTROLLER_COMM_AREA commPtr[])
{
    INT32       rc = 0;

    /*
     * Ensure the we got good inputs, if not return now.
     */
    if (commPtr != NULL)
    {
        rc = ReadFileAtOffset(FS_FID_QM_COMM_AREA, 1, commPtr, SIZE_ENTIRE_COMM_AREA);
    }

    return rc;
}


/*----------------------------------------------------------------------------
**  Function Name: ReadCommArea
**
**  Description:
**      This function reads the entire quorum communications area for a
**      given controller.
**
**  Inputs: slotID  - controller slot ID in the Quorum
**          commPtr - pointer location to store the communications sectors
**
**  Returns:  0  = good completion
**            !0 = file i/o error
**--------------------------------------------------------------------------*/
INT32 ReadCommArea(UINT16 slotID, QM_CONTROLLER_COMM_AREA *commPtr)
{
    INT32       rc = 0;
    UINT32      offset;

    /*
     * Determine the Sector offset within the communications file based on the
     * controller slot position and the sector requested. The offset is 1 based
     * since the file header is at offset 0.
     */
    offset = GetCommSectorOffset(slotID);

    /*
     * Ensure the we got good inputs, if not return now.
     */
    if (commPtr != NULL)
    {
        rc = ReadFileAtOffset(FS_FID_QM_COMM_AREA, offset, commPtr, sizeof(*commPtr));
    }

    return rc;
}


/*----------------------------------------------------------------------------
**  Function Name: WriteMailbox
**
**  Description:
**      This function allows writes to the inbound and outbound communications
**      mailboxes in the CCB communications area of the quorum.
**
**  Inputs: slotID       - controller slot ID in the Quorum
**          direction    - direction of the requested transfer
**          sectorPtr    - pointer location to retrieve the sector
**
**  Returns:  0  = good completion
**            !0 = file i/o error
**--------------------------------------------------------------------------*/
INT32 WriteMailbox(UINT16 slotID, MessageDirection direction, QM_COMM_SECTOR *sectorPtr)
{
    INT32       rc = 0;
    UINT32      offset;
    UINT16      mySlot = GetCommunicationsSlot(GetMyControllerSN());

    /*
     * Determine the Sector offset within the communications file based on the
     * controller slot position and the sector requested. The offset is 1 based
     * since the file header is at offset 0.  The offset is also dependent on
     * the direction of the communications.
     */
    if (direction == INBOUND)
    {
        offset = (mySlot * QM_MAX_CONTROLLERS) + slotID + IPC_MAILBOXES_SECTOR_OFFSET;
    }
    else
    {
        offset = (slotID * QM_MAX_CONTROLLERS) + mySlot + IPC_MAILBOXES_SECTOR_OFFSET;
    }

    /*
     * Ensure the we got good inputs, if not return now.
     */
    if (sectorPtr != NULL)
    {
        if (direction == INBOUND)
        {
            dprintf(DPRINTF_QUORUM, "**Writing Mailbox Sector, SLOT = [%u][%u]\n",
                    mySlot, slotID);
        }
        else
        {
            dprintf(DPRINTF_QUORUM, "**Writing Mailbox Sector, SLOT = [%u][%u]\n",
                    slotID, mySlot);
        }
        rc = WriteFileAtOffset(FS_FID_QM_COMM_AREA, offset, sectorPtr,
                               sizeof(*sectorPtr));
    }

    return rc;
}


/*----------------------------------------------------------------------------
**  Function Name: GetCommSectorOffset
**
**  Description:
**      This function returns the sector offset of the selected communications
**      slot within the communications file in the quorum area. Each file
**      in the file system has a 1 sector header, followed by the file data.
**
**  Inputs:   commSlot - Communications slot index
**
**  Returns:  sector offset within the communications file
**--------------------------------------------------------------------------*/
static UINT32 GetCommSectorOffset(UINT16 commSlot)
{
    /*
     * Skip over the header and determine the sector offset by multiplying
     * the slot index by the size of a slot (multiple of a sector size) and
     * then dividing by the size of a sector.
     */
    return (1 + (commSlot * sizeof(QM_CONTROLLER_COMM_AREA) / BYTES_PER_SECTOR));
}


/*----------------------------------------------------------------------------
**  Function Name: ACM_GetActiveControllerCount
**
**  Description:
**
**  Inputs:     acmPtr - pointer to ACM structure
**
**  Modifies:   Nothing
**
**  Returns:    Number of active controllers
**--------------------------------------------------------------------------*/
UINT8 ACM_GetActiveControllerCount(QM_ACTIVE_CONTROLLER_MAP *acmPtr)
{
    UINT8       activeMapCounter = 0;

    ccb_assert(acmPtr != NULL, acmPtr);

    if (acmPtr != NULL)
    {
        while ((acmPtr->node[activeMapCounter] != ACM_NODE_UNDEFINED)&&
               (activeMapCounter < MAX_CONTROLLERS))
        {
            activeMapCounter++;
        }
    }

    return (activeMapCounter);
}


/*----------------------------------------------------------------------------
**  Function Name: ACM_GetNodeBySN
**
**  Description:
**      Determine our controllers position within the active controller map
**
**  Inputs:  acmPtr - pointer to ACM structure
**           acmNodePtr - pointer to location to store position within the ACM
**           controllerSN - Serial number of which to find the node for
**
**  Outputs: *acmNodePtr - ACM postion of this controller
**
**  Returns: GOOD - ACM position found
**           ERROR - controller not found in ACM
**
**--------------------------------------------------------------------------*/
UINT32 ACM_GetNodeBySN(QM_ACTIVE_CONTROLLER_MAP *acmPtr, UINT16 *acmNodePtr,
                       UINT32 serialNumber)
{
    UINT32      returnCode = ERROR;
    UINT16      slotNumber = 0;

    ccb_assert(acmPtr != NULL, acmPtr);
    ccb_assert(acmNodePtr != NULL, acmNodePtr);

    if ((acmPtr != NULL) && (acmNodePtr != NULL))
    {
        *acmNodePtr = ACM_NODE_UNDEFINED;

        /*
         * Get our communications slot number
         */
        slotNumber = GetCommunicationsSlot(serialNumber);

        if (ACM_GetNodeBySlot(acmPtr, acmNodePtr, slotNumber) == GOOD)
        {
            returnCode = GOOD;
        }
        else
        {
            *acmNodePtr = ACM_NODE_UNDEFINED;
        }
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      ACM_GetNodeBySlot
**
**  @param      acmPtr - pointer to ACM structure
**  @param      acmNodePtr - pointer to location to store position within the ACM
**  @param      slotNumber - Slot number to find the node for
**
**  @return     GOOD - ACM position found (returned in *acmNodePtr)
**  @return     ERROR - controller not found in ACM
**
******************************************************************************
**/
UINT32 ACM_GetNodeBySlot(QM_ACTIVE_CONTROLLER_MAP *acmPtr, UINT16 *acmNodePtr,
                         UINT16 slotNumber)
{
    UINT32      returnCode = ERROR;
    UINT16      index1 = 0;

    ccb_assert(acmPtr != NULL, acmPtr);
    ccb_assert(acmNodePtr != NULL, acmNodePtr);
    ccb_assert(slotNumber < MAX_CONTROLLERS, slotNumber);

    if ((acmPtr != NULL) && (acmNodePtr != NULL))
    {
        /*
         * Set initial return value
         */
        *acmNodePtr = ACM_NODE_UNDEFINED;

        /*
         * Ensure that our slot is within the range of acceptable controllers. It
         * should always be, unless the system is not properly configured.
         */
        if (slotNumber < MAX_CONTROLLERS)
        {
            /*
             * Scan through the ACM, trying to find the position of the slot.
             * If the node is found, set the output and change the return
             * code to indicate success.  If we step passed the end of the
             * ACM, then exit out (return an error).  Otherwise, bump the
             * index and look in the next position.
             */
            for (index1 = 0;
                 ((index1 < MAX_CONTROLLERS) &&
                  (acmPtr->node[index1] < Qm_GetNumControllersAllowed()) &&
                  (returnCode == ERROR));
                 index1++)
            {
                if (acmPtr->node[index1] == slotNumber)
                {
                    *acmNodePtr = index1;
                    returnCode = GOOD;
                }
            }
        }
    }

    return (returnCode);
}


/**
******************************************************************************
**  @brief      ACM_GetParent
**
**  @param      currentNode - pointer to ACM structure
**  @param      parentNodePtr - pointer to where parent node is returned
**
**  @return     GOOD - ACM position found (returned in *parentNodePtr)
**  @return     ERROR - Error finding parent in ACM
******************************************************************************
**/
UINT32 ACM_GetParent(UINT16 currentNode, UINT16 *parentNodePtr)
{
    UINT32      returnCode = ERROR;

    ccb_assert(currentNode < MAX_CONTROLLERS, currentNode);
    ccb_assert(parentNodePtr != NULL, parentNodePtr);

    /*
     * Bounds check the input before finding parent node
     */
    if ((currentNode < MAX_CONTROLLERS) && (parentNodePtr != NULL))
    {
        if (currentNode != 0)
        {
            *parentNodePtr = (currentNode - 1) / 2;
        }
        else
        {
            *parentNodePtr = ACM_NODE_UNDEFINED;
        }

        returnCode = GOOD;
    }

    return (returnCode);
}


/**
******************************************************************************
**  @brief      ACM_GetChildren
**
**  @param      currentNode - pointer to ACM structure
**  @param      leftChildNodePtr - pointer to where leftChild node is returned
**  @param      rightChildNodePtr - pointer to where rightChild node is returned
**
**  @return     GOOD - Children positions found
**  @return     ERROR - Error finding children in ACM
******************************************************************************
**/
UINT32 ACM_GetChildren(UINT16 currentNode, UINT16 *leftChildNodePtr,
                       UINT16 *rightChildNodePtr)
{
    UINT32      returnCode = ERROR;

    ccb_assert(currentNode < MAX_CONTROLLERS, currentNode);
    ccb_assert(leftChildNodePtr != NULL, leftChildNodePtr);
    ccb_assert(rightChildNodePtr != NULL, rightChildNodePtr);

    /*
     * Bounds check the input before finding children nodes
     */
    if ((currentNode < MAX_CONTROLLERS) &&
        (leftChildNodePtr != NULL) && (rightChildNodePtr != NULL))
    {
        if (currentNode < (MAX_CONTROLLERS / 2))
        {
            *leftChildNodePtr = (currentNode * 2) + 1;
        }
        else
        {
            *leftChildNodePtr = ACM_NODE_UNDEFINED;
        }

#if (MAX_CONTROLLERS > 2)
        if (currentNode < ((MAX_CONTROLLERS - 1) / 2))
        {
            *rightChildNodePtr = (currentNode * 2) + 2;
        }
        else
#endif  /* (MAX_CONTROLLERS > 2) */
        {
            *rightChildNodePtr = ACM_NODE_UNDEFINED;
        }

        returnCode = GOOD;
    }

    return (returnCode);
}

/*----------------------------------------------------------------------------
**  Function Name: ReadAllMailboxes
**
**  Description:
**      This function reads all IPC mailboxes
**
**  Inputs: commPtr - pointer location to store the communications sectors
**
**  Returns:  0  = good completion
**            !0 = file i/o error
**--------------------------------------------------------------------------*/
INT32 ReadAllMailboxes(QM_IPC_MAILBOX pComm[])
{
    INT32       rc = 0;

    /*
     * Ensure the we got good inputs, if not return now.
     */
    if (pComm != NULL)
    {
        rc = ReadFileAtOffset(FS_FID_QM_COMM_AREA, IPC_MAILBOXES_SECTOR_OFFSET,
                              pComm, NUM_IPC_MAILBOXES * sizeof(QM_COMM_SECTOR));
    }

    return rc;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
